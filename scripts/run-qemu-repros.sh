#!/bin/bash
# Host-side driver: boot Debian in QEMU, compile+run crepros *inside* the guest.
#
# Syz repros can panic the guest kernel. This driver streams per-file RESULT
# lines to the host, and on SSH drop (exit 255) records GUEST_CRASH for the
# next unfinished file, reboots the VM, and resumes the remainder.
#
# Usage:
#   ./scripts/run-qemu-repros.sh
#   SHARD_INDEX=3 SHARD_COUNT=32 ./scripts/run-qemu-repros.sh
#   LIMIT=50 RUN_TIMEOUT=5 ./scripts/run-qemu-repros.sh
#
# Env: SHARD_INDEX, SHARD_COUNT, RUN_TIMEOUT, COMPILE_TIMEOUT, LIMIT,
#      SSH_PORT, ARTIFACT_DIR, KEEP_VM, DISK_SIZE_G, MAX_REBOOTS (default 200)

set -euo pipefail
cd "$(dirname "$0")/.."

SHARD_INDEX="${SHARD_INDEX:-0}"
SHARD_COUNT="${SHARD_COUNT:-1}"
RUN_TIMEOUT="${RUN_TIMEOUT:-5}"
COMPILE_TIMEOUT="${COMPILE_TIMEOUT:-30}"
LIMIT="${LIMIT:-0}"
SSH_PORT="${SSH_PORT:-2222}"
ARTIFACT_DIR="${ARTIFACT_DIR:-$PWD/.qemu-run}"
KEEP_VM="${KEEP_VM:-0}"
DISK_SIZE_G="${DISK_SIZE_G:-8}"
MAX_REBOOTS="${MAX_REBOOTS:-200}"

mkdir -p "$ARTIFACT_DIR"
SERIAL_LOG="$ARTIFACT_DIR/serial.log"
QEMU_PID_FILE="$ARTIFACT_DIR/qemu.pid"
RESULTS_HOST="$ARTIFACT_DIR/repro-results.tsv"
SHARD_LIST="$ARTIFACT_DIR/shard-list.txt"
REMAINING_LIST="$ARTIFACT_DIR/remaining-list.txt"
ID_RSA="$ARTIFACT_DIR/id_rsa"
DISK_IMG="$ARTIFACT_DIR/disk.qcow2"
SEED_ISO="$ARTIFACT_DIR/seed.iso"

rm -f "$RESULTS_HOST" "$SHARD_LIST" "$REMAINING_LIST" "$QEMU_PID_FILE"

echo "Selecting shard $SHARD_INDEX/$SHARD_COUNT from crepros/ ..."
python3 - "$SHARD_INDEX" "$SHARD_COUNT" "$LIMIT" <<'PY' >"$SHARD_LIST"
import os, sys, zlib
idx, count, limit = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
files = sorted(
    f for f in os.listdir("crepros")
    if f.endswith(".c") and os.path.isfile(os.path.join("crepros", f))
)
selected = [f for f in files if zlib.crc32(f.encode()) % count == idx]
if limit > 0:
    selected = selected[:limit]
for f in selected:
    print(os.path.join("crepros", f))
print(f"shard {idx}/{count}: {len(selected)} of {len(files)} sources", file=sys.stderr)
PY

n_sources=$(wc -l <"$SHARD_LIST" | tr -d ' ')
if [[ "$n_sources" -eq 0 ]]; then
	echo "No sources in this shard; nothing to do."
	: >"$RESULTS_HOST"
	echo -e "file\tstatus\tcompile_rc\trun_rc" >"$RESULTS_HOST"
	exit 0
fi
cp "$SHARD_LIST" "$REMAINING_LIST"
printf 'file\tstatus\tcompile_rc\trun_rc\n' >"$RESULTS_HOST"

# --- image + ssh key (once) -------------------------------------------------
img_url="https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-genericcloud-amd64.qcow2"
cache="${CLOUD_IMAGE_CACHE:-/tmp/debian-12-genericcloud-amd64.qcow2}"
if [[ ! -f "$cache" ]]; then
	wget -q --show-progress -O "$cache" "$img_url"
fi

if [[ ! -f "$ID_RSA" ]]; then
	ssh-keygen -t ed25519 -N '' -f "$ID_RSA" -C "lkrt-ci" >/dev/null
fi
PUB=$(cat "${ID_RSA}.pub")

# Install toolchain via cloud-init packages so every boot is ready faster.
cat >"$ARTIFACT_DIR/user-data" <<EOF
#cloud-config
hostname: lkrt-ci
manage_etc_hosts: true
disable_root: false
ssh_pwauth: false
package_update: true
packages:
  - gcc
  - gcc-multilib
  - libc6-dev
  - make
  - ca-certificates
users:
  - default
  - name: root
    lock_passwd: true
    ssh_authorized_keys:
      - ${PUB}
write_files:
  - path: /etc/ssh/sshd_config.d/60-lkrt.conf
    permissions: "0644"
    content: |
      PermitRootLogin prohibit-password
      PasswordAuthentication no
      PubkeyAuthentication yes
  - path: /root/.ssh/authorized_keys
    permissions: "0600"
    owner: root:root
    content: |
      ${PUB}
runcmd:
  - [ bash, -c, "mkdir -p /home/debian/.ssh /root/.ssh && echo '${PUB}' > /root/.ssh/authorized_keys && chmod 600 /root/.ssh/authorized_keys" ]
  - [ bash, -c, "echo '${PUB}' >> /home/debian/.ssh/authorized_keys && chown -R debian:debian /home/debian/.ssh && chmod 700 /home/debian/.ssh && chmod 600 /home/debian/.ssh/authorized_keys || true" ]
  - [ systemctl, restart, ssh ]
EOF

cat >"$ARTIFACT_DIR/meta-data" <<EOF
instance-id: lkrt-ci-${SHARD_INDEX}
local-hostname: lkrt-ci
EOF
cloud-localds "$SEED_ISO" "$ARTIFACT_DIR/user-data" "$ARTIFACT_DIR/meta-data"

SSH_BASE=(
	ssh -i "$ID_RSA"
	-o StrictHostKeyChecking=no
	-o UserKnownHostsFile=/dev/null
	-o ConnectTimeout=10
	-o ConnectionAttempts=1
	-o LogLevel=ERROR
	-o ServerAliveInterval=15
	-o ServerAliveCountMax=4
	-p "$SSH_PORT"
)
SCP_BASE=(
	scp -i "$ID_RSA"
	-o StrictHostKeyChecking=no
	-o UserKnownHostsFile=/dev/null
	-o ConnectTimeout=10
	-o LogLevel=ERROR
	-P "$SSH_PORT"
)

stop_qemu() {
	if [[ -f "$QEMU_PID_FILE" ]]; then
		kill "$(cat "$QEMU_PID_FILE")" 2>/dev/null || true
		# Wait briefly for exit
		for _ in $(seq 1 20); do
			kill -0 "$(cat "$QEMU_PID_FILE" 2>/dev/null)" 2>/dev/null || break
			sleep 0.25
		done
		kill -9 "$(cat "$QEMU_PID_FILE" 2>/dev/null)" 2>/dev/null || true
		rm -f "$QEMU_PID_FILE"
	fi
}

start_qemu() {
	stop_qemu
	# Fresh disk each boot so a corrupted rootfs after panic cannot stick.
	cp -f "$cache" "$DISK_IMG"
	qemu-img resize "$DISK_IMG" "${DISK_SIZE_G}G" >/dev/null
	: >"$SERIAL_LOG"

	ACCEL=tcg
	if [[ -r /dev/kvm ]]; then ACCEL=kvm; fi
	echo "Starting QEMU accel=$ACCEL"

	qemu-system-x86_64 \
		-name "lkrt-ci-s${SHARD_INDEX}" \
		-machine "pc,accel=$ACCEL" \
		-m 2G \
		-smp 2 \
		-drive "file=$DISK_IMG,format=qcow2,if=virtio" \
		-drive "file=$SEED_ISO,format=raw,if=virtio,media=cdrom" \
		-netdev "user,id=net0,hostfwd=tcp::${SSH_PORT}-:22" \
		-device virtio-net-pci,netdev=net0 \
		-display none \
		-serial "file:$SERIAL_LOG" \
		-pidfile "$QEMU_PID_FILE" \
		-daemonize
	echo "QEMU pid $(cat "$QEMU_PID_FILE")"
}

wait_ssh() {
	# Sets global GUEST_USER to root or debian.
	GUEST_USER=""
	# package_update + gcc install can take a few minutes on first boot
	for i in $(seq 1 120); do
		if "${SSH_BASE[@]}" root@127.0.0.1 'echo up' 2>/dev/null; then
			GUEST_USER=root
			break
		fi
		if "${SSH_BASE[@]}" debian@127.0.0.1 'echo up' 2>/dev/null; then
			GUEST_USER=debian
			break
		fi
		if [[ $((i % 12)) -eq 0 ]]; then
			echo "  still waiting for SSH/cloud-init ($i/120)..."
		fi
		sleep 5
	done
	if [[ -z "$GUEST_USER" ]]; then
		echo "SSH failed to come up"
		tail -n 100 "$SERIAL_LOG" || true
		return 1
	fi
	echo "SSH as $GUEST_USER"
	return 0
}

ensure_gcc() {
	local user=$1
	local ssh=("${SSH_BASE[@]}" "${user}@127.0.0.1")
	# SSH often comes up mid cloud-init. Wait for packages/gcc (quotes must be
	# embedded in the remote command string — ssh does not preserve local quotes).
	echo "Waiting for cloud-init / gcc..."
	"${ssh[@]}" "cloud-init status --wait 2>/dev/null || true" || true
	local i
	for i in $(seq 1 90); do
		if "${ssh[@]}" "command -v gcc >/dev/null 2>&1"; then
			break
		fi
		# Hold off while apt is busy (cloud-init package install).
		"${ssh[@]}" "sh -c 'while fuser /var/lib/dpkg/lock-frontend /var/lib/apt/lists/lock >/dev/null 2>&1; do sleep 2; done'" || true
		if [[ $((i % 6)) -eq 0 ]]; then
			echo "  still waiting for gcc ($i/90)..."
		fi
		sleep 5
	done
	if ! "${ssh[@]}" "command -v gcc >/dev/null 2>&1"; then
		echo "gcc still missing; installing via apt..."
		if [[ "$user" == "root" ]]; then
			"${ssh[@]}" "sh -c 'export DEBIAN_FRONTEND=noninteractive; apt-get update -qq && apt-get install -y --no-install-recommends gcc gcc-multilib libc6-dev'"
		else
			"${ssh[@]}" "sudo -n sh -c 'export DEBIAN_FRONTEND=noninteractive; apt-get update -qq && apt-get install -y --no-install-recommends gcc gcc-multilib libc6-dev'"
		fi
	fi
	"${ssh[@]}" "gcc --version | head -1"
}

append_result() {
	# $1=file $2=status $3=crc $4=rrc
	printf '%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" >>"$RESULTS_HOST"
}

done_count() {
	# lines minus header
	local n
	n=$(wc -l <"$RESULTS_HOST" | tr -d ' ')
	echo $((n > 0 ? n - 1 : 0))
}

# Rebuild remaining list from shard-list minus completed basenames in results.
refresh_remaining() {
	python3 - "$SHARD_LIST" "$RESULTS_HOST" "$REMAINING_LIST" <<'PY'
import sys
shard, results, out = sys.argv[1:4]
done=set()
with open(results) as f:
    next(f, None)
    for line in f:
        parts=line.rstrip("\n").split("\t")
        if parts and parts[0]:
            done.add(parts[0])
remain=[]
with open(shard) as f:
    for line in f:
        path=line.strip()
        if not path:
            continue
        base=path.rsplit("/",1)[-1]
        if base not in done:
            remain.append(path)
with open(out,"w") as f:
    f.write("\n".join(remain) + ("\n" if remain else ""))
print(f"remaining={len(remain)} done={len(done)}")
PY
}

run_batch_on_live_guest() {
	local user=$1
	local remote_base="/root"
	[[ "$user" != "root" ]] && remote_base="/home/debian"
	local remote_crepros="$remote_base/crepros"
	local ssh=("${SSH_BASE[@]}" "${user}@127.0.0.1")
	local scp=("${SCP_BASE[@]}")
	local n_rem
	n_rem=$(wc -l <"$REMAINING_LIST" | tr -d ' ')
	[[ "$n_rem" -gt 0 ]] || return 0

	local tarpath="$ARTIFACT_DIR/crepros-remaining.tar.gz"
	tar -czf "$tarpath" -T "$REMAINING_LIST"

	"${ssh[@]}" mkdir -p "$remote_crepros"
	"${scp[@]}" "$tarpath" "${user}@127.0.0.1:$remote_base/crepros-remaining.tar.gz"
	"${scp[@]}" scripts/guest-run-repros.sh "${user}@127.0.0.1:$remote_base/guest-run-repros.sh"
	"${ssh[@]}" bash -s <<EOF
set -euo pipefail
rm -rf "$remote_crepros"
mkdir -p "$remote_base"
tar -xzf "$remote_base/crepros-remaining.tar.gz" -C "$remote_base"
chmod +x "$remote_base/guest-run-repros.sh"
# expect crepros/ under remote_base
test -d "$remote_crepros"
echo "guest has \$(ls "$remote_crepros" | wc -l) sources"
EOF

	echo "Running $n_rem remaining repros in guest (timeout ${RUN_TIMEOUT}s)..."
	# Stream RESULT lines. Never return non-zero from this function under set -e;
	# stash status in BATCH_RC instead so the reboot loop can continue.
	BATCH_RC=0
	set +e
	# NOTE: cannot use python <<'PY' here — that steals stdin from the pipe.
	"${ssh[@]}" \
		env CREPROS_DIR="$remote_crepros" \
		RUN_TIMEOUT="$RUN_TIMEOUT" \
		COMPILE_TIMEOUT="$COMPILE_TIMEOUT" \
		PYTHONUNBUFFERED=1 \
		bash "$remote_base/guest-run-repros.sh" \
		2>"$ARTIFACT_DIR/guest-stderr.log" \
		| tee "$ARTIFACT_DIR/guest-stdout.log" \
		| python3 -u -c '
import sys
results_path = sys.argv[1]
for line in sys.stdin:
    line = line.rstrip("\n")
    if not line.startswith("RESULT\t"):
        print(line, flush=True)
        continue
    parts = line.split("\t")
    if len(parts) >= 5:
        _, fn, status, crc, rrc = parts[:5]
        with open(results_path, "a") as out:
            out.write(f"{fn}\t{status}\t{crc}\t{rrc}\n")
            out.flush()
        print(f"  {status:12} {fn}", flush=True)
' "$RESULTS_HOST"
	BATCH_RC=${PIPESTATUS[0]}
	set +e
	if [[ -f "$ARTIFACT_DIR/guest-stderr.log" ]]; then
		tail -n 40 "$ARTIFACT_DIR/guest-stderr.log" || true
	fi
	echo "guest_rc=$BATCH_RC"
	return 0
}

cleanup() {
	if [[ "$KEEP_VM" != "1" ]]; then
		stop_qemu
	fi
}
trap cleanup EXIT

reboots=0
while true; do
	refresh_remaining
	n_rem=$(wc -l <"$REMAINING_LIST" | tr -d ' ')
	if [[ "$n_rem" -eq 0 ]]; then
		echo "All sources processed."
		break
	fi
	if [[ "$reboots" -gt "$MAX_REBOOTS" ]]; then
		echo "ERROR: exceeded MAX_REBOOTS=$MAX_REBOOTS with $n_rem remaining"
		exit 1
	fi

	echo "==== boot cycle $reboots (remaining=$n_rem done=$(done_count)) ===="
	start_qemu
	if ! wait_ssh; then
		reboots=$((reboots + 1))
		continue
	fi
	ensure_gcc "$GUEST_USER"

	BATCH_RC=0
	run_batch_on_live_guest "$GUEST_USER"
	rc=${BATCH_RC:-0}

	refresh_remaining
	n_rem=$(wc -l <"$REMAINING_LIST" | tr -d ' ')
	if [[ "$n_rem" -eq 0 ]]; then
		echo "Shard complete after $reboots reboot(s)."
		break
	fi

	if [[ "$rc" -eq 0 ]]; then
		echo "WARN: guest exit 0 but remaining=$n_rem — marking next as ERROR and rebooting"
	fi

	# Guest died (panic / SSH 255) or incomplete. Mark the next unfinished file
	# as GUEST_CRASH (likely the repro that panics the kernel), then reboot.
	next=$(head -n 1 "$REMAINING_LIST" || true)
	if [[ -n "$next" ]]; then
		base=$(basename "$next")
		echo "Recording GUEST_CRASH for $base (ssh/guest rc=$rc)"
		append_result "$base" "GUEST_CRASH" "-" "$rc"
	fi
	reboots=$((reboots + 1))
	stop_qemu
done

echo "---- results summary ----"
python3 - "$RESULTS_HOST" <<'PY'
import sys, collections
p = sys.argv[1]
c = collections.Counter()
n = 0
with open(p) as f:
    next(f, None)
    for line in f:
        parts = line.rstrip("\n").split("\t")
        if len(parts) >= 2:
            c[parts[1]] += 1
            n += 1
print(f"rows={n}")
for k, v in sorted(c.items(), key=lambda kv: (-kv[1], kv[0])):
    print(f"  {k}: {v}")
PY
ls -lh "$RESULTS_HOST"
echo "QEMU_REPROS_OK shard=${SHARD_INDEX}/${SHARD_COUNT} n=${n_sources} reboots=${reboots}"
