#!/bin/bash
# Host-side driver: boot Debian in QEMU, compile+run crepros *inside* the guest.
#
# Protocol: guest streams BEGIN/RESULT lines over SSH. On guest panic (SSH drop),
# the host attributes the crash to the last BEGIN'd repro, scrapes the QEMU
# serial log for oops/panic text, writes crashes/<repro>.md, records PANIC in
# the results TSV, reboots, and resumes.
#
# Usage:
#   ./scripts/run-qemu-repros.sh
#   SHARD_INDEX=3 SHARD_COUNT=32 ./scripts/run-qemu-repros.sh
#   LIMIT=50 RUN_TIMEOUT=5 ./scripts/run-qemu-repros.sh
#
# Env: SHARD_INDEX, SHARD_COUNT, RUN_TIMEOUT, COMPILE_TIMEOUT, LIMIT,
#      SSH_PORT, ARTIFACT_DIR, KEEP_VM, DISK_SIZE_G, MAX_REBOOTS (default 200),
#      KERNEL_MODE (kasan|distro), BZIMAGE_URL, QEMU_MEM, KERNEL_APPEND_EXTRAS

set -euo pipefail
cd "$(dirname "$0")/.."

# shellcheck source=kernel-assets.conf
source scripts/kernel-assets.conf

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
KERNEL_MODE="${KERNEL_MODE:-kasan}"
QEMU_MEM="${QEMU_MEM:-4G}"

mkdir -p "$ARTIFACT_DIR/crashes" "$ARTIFACT_DIR/serial" "$ARTIFACT_DIR/kernel"
QEMU_PID_FILE="$ARTIFACT_DIR/qemu.pid"
RESULTS_HOST="$ARTIFACT_DIR/repro-results.tsv"
PANICS_INDEX="$ARTIFACT_DIR/panics.tsv"
SHARD_LIST="$ARTIFACT_DIR/shard-list.txt"
REMAINING_LIST="$ARTIFACT_DIR/remaining-list.txt"
CURRENT_REPRO_FILE="$ARTIFACT_DIR/current-repro.txt"
ID_RSA="$ARTIFACT_DIR/id_rsa"
DISK_IMG="$ARTIFACT_DIR/disk.qcow2"
SEED_ISO="$ARTIFACT_DIR/seed.iso"
BZIMAGE_PATH="$ARTIFACT_DIR/kernel/bzImage"
# Active serial for this boot cycle (also concatenated into serial/all.log)
SERIAL_LOG="$ARTIFACT_DIR/serial/current.log"
SERIAL_ALL="$ARTIFACT_DIR/serial/all.log"

rm -f "$RESULTS_HOST" "$PANICS_INDEX" "$SHARD_LIST" "$REMAINING_LIST" \
	"$QEMU_PID_FILE" "$CURRENT_REPRO_FILE"
rm -rf "$ARTIFACT_DIR/crashes"
mkdir -p "$ARTIFACT_DIR/crashes" "$ARTIFACT_DIR/serial"
: >"$SERIAL_ALL"

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
	printf 'file\tstatus\tcompile_rc\trun_rc\tpanic_title\treport\n' >"$RESULTS_HOST"
	printf 'repro\tcycle\ttitle\treport\n' >"$PANICS_INDEX"
	exit 0
fi
cp "$SHARD_LIST" "$REMAINING_LIST"
printf 'file\tstatus\tcompile_rc\trun_rc\tpanic_title\treport\n' >"$RESULTS_HOST"
printf 'repro\tcycle\ttitle\treport\n' >"$PANICS_INDEX"

# --- image + kernel + ssh key (once) ----------------------------------------
img_url="https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-genericcloud-amd64.qcow2"
cache="${CLOUD_IMAGE_CACHE:-/tmp/debian-12-genericcloud-amd64.qcow2}"
if [[ ! -f "$cache" ]]; then
	echo "Downloading Debian cloud image..."
	wget -q --show-progress -O "$cache" "$img_url"
fi

if [[ "$KERNEL_MODE" == "kasan" ]]; then
	bz_cache="${BZIMAGE_CACHE:-/tmp/$(basename "$BZIMAGE_URL")}"
	if [[ ! -f "$bz_cache" ]]; then
		echo "Downloading KASAN bzImage from syzbot assets..."
		wget -q --show-progress -O "$bz_cache" "$BZIMAGE_URL"
	fi
	if [[ "$bz_cache" == *.xz ]]; then
		echo "Decompressing bzImage..."
		xz -dkc "$bz_cache" >"$BZIMAGE_PATH"
	else
		cp -f "$bz_cache" "$BZIMAGE_PATH"
	fi
	ls -lh "$BZIMAGE_PATH"
	echo "KERNEL_MODE=kasan append extras: $KERNEL_APPEND_EXTRAS"
else
	echo "KERNEL_MODE=distro (stock cloud kernel; weak signal)"
fi

if [[ ! -f "$ID_RSA" ]]; then
	ssh-keygen -t ed25519 -N '' -f "$ID_RSA" -C "lkrt-ci" >/dev/null
fi
PUB=$(cat "${ID_RSA}.pub")

cat >"$ARTIFACT_DIR/user-data" <<EOF
#cloud-config
hostname: lkrt-ci
manage_etc_hosts: true
disable_root: false
ssh_pwauth: false
# Avoid blocking boot on package installs when the guest net stack is still
# settling under a syzbot kernel (many virtual NICs). ensure_gcc() installs
# the toolchain after SSH is up.
package_update: false
packages: []
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
		for _ in $(seq 1 20); do
			kill -0 "$(cat "$QEMU_PID_FILE" 2>/dev/null)" 2>/dev/null || break
			sleep 0.25
		done
		kill -9 "$(cat "$QEMU_PID_FILE" 2>/dev/null)" 2>/dev/null || true
		rm -f "$QEMU_PID_FILE"
	fi
	# Preserve this cycle's serial into the combined log.
	if [[ -f "$SERIAL_LOG" ]]; then
		{
			echo ""
			echo "===== END CYCLE serial $(date -u +%Y-%m-%dT%H:%M:%SZ) ====="
			cat "$SERIAL_LOG"
		} >>"$SERIAL_ALL" 2>/dev/null || true
	fi
}

start_qemu() {
	local cycle=$1
	stop_qemu
	# Fresh disk each boot so a corrupted rootfs after panic cannot stick.
	cp -f "$cache" "$DISK_IMG"
	qemu-img resize "$DISK_IMG" "${DISK_SIZE_G}G" >/dev/null
	SERIAL_LOG="$ARTIFACT_DIR/serial/cycle-${cycle}.log"
	: >"$SERIAL_LOG"
	{
		echo "===== START CYCLE ${cycle} mode=${KERNEL_MODE} $(date -u +%Y-%m-%dT%H:%M:%SZ) ====="
	} >>"$SERIAL_ALL"

	ACCEL=tcg
	if [[ -r /dev/kvm ]]; then ACCEL=kvm; fi
	echo "Starting QEMU accel=$ACCEL mem=$QEMU_MEM mode=$KERNEL_MODE cycle=$cycle"

	# e1000 is more reliable than virtio-net under syzbot kernels (which
	# register dozens of dummy NICs and often leave virtio eth without carrier).
	local -a qemu_cmd=(
		qemu-system-x86_64
		-name "lkrt-ci-s${SHARD_INDEX}"
		-machine "pc,accel=$ACCEL"
		-m "$QEMU_MEM"
		-smp 2
		-drive "file=$DISK_IMG,format=qcow2,if=virtio"
		-drive "file=$SEED_ISO,format=raw,if=virtio,media=cdrom"
		-netdev "user,id=net0,hostfwd=tcp::${SSH_PORT}-:22"
		-device e1000,netdev=net0
		-display none
		-serial "file:$SERIAL_LOG"
		-pidfile "$QEMU_PID_FILE"
		-daemonize
	)

	if [[ "$KERNEL_MODE" == "kasan" ]]; then
		# Boot syzbot-style instrumented kernel on the Debian cloud rootfs.
		# -no-reboot keeps the panic frame on serial for extract-panic.py.
		# root=/dev/vda1 is the first virtio disk partition of the cloud image.
		local append="root=/dev/vda1 rootfstype=ext4 rw console=ttyS0 earlyprintk=serial"
		append+=" net.ifnames=0 ip=dhcp ${KERNEL_APPEND_EXTRAS}"
		qemu_cmd+=(
			-kernel "$BZIMAGE_PATH"
			-append "$append"
			-no-reboot
		)
		echo "  kernel=$BZIMAGE_PATH"
		echo "  append=$append"
	fi

	"${qemu_cmd[@]}"
	echo "QEMU pid $(cat "$QEMU_PID_FILE") serial=$SERIAL_LOG"
}

wait_ssh() {
	GUEST_USER=""
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
	echo "Waiting for cloud-init / gcc..."
	"${ssh[@]}" "cloud-init status --wait 2>/dev/null || true" || true
	local i
	for i in $(seq 1 90); do
		if "${ssh[@]}" "command -v gcc >/dev/null 2>&1"; then
			break
		fi
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
	# file status crc rrc [panic_title] [report_path]
	local file=$1 status=$2 crc=$3 rrc=$4
	local title=${5:-}
	local report=${6:-}
	printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$file" "$status" "$crc" "$rrc" "$title" "$report" >>"$RESULTS_HOST"
}

done_count() {
	local n
	n=$(wc -l <"$RESULTS_HOST" | tr -d ' ')
	echo $((n > 0 ? n - 1 : 0))
}

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

# Record a panic for $repro using serial log; sets globals PANIC_TITLE / PANIC_REPORT.
record_panic() {
	local repro=$1
	local cycle=$2
	local rc=$3
	local report="$ARTIFACT_DIR/crashes/${repro}.md"
	# If multiple panics for same name across cycles, uniquify.
	if [[ -f "$report" ]]; then
		report="$ARTIFACT_DIR/crashes/${repro}.cycle${cycle}.md"
	fi

	# Give the kernel a moment to flush oops text to the serial file.
	sleep 2
	sync 2>/dev/null || true

	set +e
	python3 scripts/extract-panic.py "$SERIAL_LOG" \
		--repro "$repro" \
		--cycle "$cycle" \
		--out "$report" >"$ARTIFACT_DIR/last-panic-meta.txt"
	local ext_rc=$?
	set -e

	local title="guest died (ssh rc=${rc}); see serial"
	local found=0
	if [[ -f "$ARTIFACT_DIR/last-panic-meta.txt" ]]; then
		# PANIC_TITLE\trepro\tfound\ttitle
		local meta
		meta=$(grep '^PANIC_TITLE' "$ARTIFACT_DIR/last-panic-meta.txt" | tail -n1 || true)
		if [[ -n "$meta" ]]; then
			found=$(printf '%s\n' "$meta" | cut -f3)
			title=$(printf '%s\n' "$meta" | cut -f4-)
		fi
	fi

	local status="PANIC"
	if [[ "$found" != "1" ]]; then
		status="GUEST_CRASH"
		title="no oops signature in serial (ssh rc=${rc}); last BEGIN was ${repro}"
	fi

	local rel_report
	rel_report=$(realpath --relative-to="$ARTIFACT_DIR" "$report" 2>/dev/null || echo "crashes/$(basename "$report")")
	append_result "$repro" "$status" "-" "$rc" "$title" "$rel_report"
	printf '%s\t%s\t%s\t%s\n' "$repro" "$cycle" "$title" "$rel_report" >>"$PANICS_INDEX"

	echo "!!!! $status repro=$repro"
	echo "     title: $title"
	echo "     report: $report"
	if [[ -f "$report" ]]; then
		# Show a short excerpt in the CI log.
		sed -n '/## Serial excerpt/,/^```$/p' "$report" | head -n 40 || true
	fi
}

run_batch_on_live_guest() {
	local user=$1
	local cycle=$2
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
test -d "$remote_crepros"
echo "guest has \$(ls "$remote_crepros" | wc -l) sources"
EOF

	echo "Running $n_rem remaining repros in guest (timeout ${RUN_TIMEOUT}s)..."
	: >"$CURRENT_REPRO_FILE"
	BATCH_RC=0
	set +e
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
results_path, current_path = sys.argv[1], sys.argv[2]
for line in sys.stdin:
    line = line.rstrip("\n")
    if line.startswith("BEGIN\t"):
        fn = line.split("\t", 1)[1]
        with open(current_path, "w") as c:
            c.write(fn + "\n")
            c.flush()
        print(f"  BEGIN        {fn}", flush=True)
        continue
    if not line.startswith("RESULT\t"):
        print(line, flush=True)
        continue
    parts = line.split("\t")
    if len(parts) >= 5:
        _, fn, status, crc, rrc = parts[:5]
        with open(results_path, "a") as out:
            # panic_title/report empty for normal results
            out.write(f"{fn}\t{status}\t{crc}\t{rrc}\t\t\n")
            out.flush()
        # Clear current — finished cleanly
        open(current_path, "w").close()
        print(f"  {status:12} {fn}", flush=True)
' "$RESULTS_HOST" "$CURRENT_REPRO_FILE"
	BATCH_RC=${PIPESTATUS[0]}
	set +e
	if [[ -f "$ARTIFACT_DIR/guest-stderr.log" ]]; then
		tail -n 40 "$ARTIFACT_DIR/guest-stderr.log" || true
	fi
	echo "guest_rc=$BATCH_RC current=$(cat "$CURRENT_REPRO_FILE" 2>/dev/null || true)"
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
	start_qemu "$reboots"
	if ! wait_ssh; then
		reboots=$((reboots + 1))
		continue
	fi
	ensure_gcc "$GUEST_USER"

	: >"$CURRENT_REPRO_FILE"
	BATCH_RC=0
	run_batch_on_live_guest "$GUEST_USER" "$reboots"
	rc=${BATCH_RC:-0}

	refresh_remaining
	n_rem=$(wc -l <"$REMAINING_LIST" | tr -d ' ')
	if [[ "$n_rem" -eq 0 ]]; then
		echo "Shard complete after $reboots reboot(s)."
		break
	fi

	# Attribute crash: prefer in-flight BEGIN (host-tracked), else first remaining.
	culprit=""
	if [[ -s "$CURRENT_REPRO_FILE" ]]; then
		culprit=$(tr -d '[:space:]' <"$CURRENT_REPRO_FILE")
	fi
	if [[ -z "$culprit" ]]; then
		# Fallback: last BEGIN in guest stdout / serial
		if [[ -f "$ARTIFACT_DIR/guest-stdout.log" ]]; then
			culprit=$(grep '^BEGIN' "$ARTIFACT_DIR/guest-stdout.log" | tail -n1 | cut -f2 || true)
		fi
	fi
	if [[ -z "$culprit" ]]; then
		next=$(head -n 1 "$REMAINING_LIST" || true)
		culprit=$(basename "${next:-unknown}")
	fi

	echo "Guest stopped early (rc=$rc); attributing to repro=$culprit"
	record_panic "$culprit" "$reboots" "$rc"

	reboots=$((reboots + 1))
	stop_qemu
done

echo "---- results summary ----"
python3 - "$RESULTS_HOST" "$PANICS_INDEX" <<'PY'
import sys, collections
results, panics = sys.argv[1], sys.argv[2]
c = collections.Counter()
n = 0
with open(results) as f:
    next(f, None)
    for line in f:
        parts = line.rstrip("\n").split("\t")
        if len(parts) >= 2:
            c[parts[1]] += 1
            n += 1
print(f"rows={n}")
for k, v in sorted(c.items(), key=lambda kv: (-kv[1], kv[0])):
    print(f"  {k}: {v}")
pc = 0
with open(panics) as f:
    next(f, None)
    for line in f:
        if line.strip():
            pc += 1
            parts = line.rstrip("\n").split("\t")
            if len(parts) >= 3:
                print(f"  PANIC_DETAIL  {parts[0]}: {parts[2][:120]}")
print(f"panic_reports={pc}")
PY
ls -lh "$RESULTS_HOST" "$PANICS_INDEX"
ls -la "$ARTIFACT_DIR/crashes" 2>/dev/null | head -20 || true
echo "QEMU_REPROS_OK shard=${SHARD_INDEX}/${SHARD_COUNT} n=${n_sources} reboots=${reboots}"
