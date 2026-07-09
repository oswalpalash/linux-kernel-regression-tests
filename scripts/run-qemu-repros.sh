#!/bin/bash
# Host-side driver: boot Debian cloud image in QEMU, copy a shard of crepros
# into the guest, install a toolchain, compile+run each repro (5s default).
#
# Usage:
#   ./scripts/run-qemu-repros.sh
#   SHARD_INDEX=3 SHARD_COUNT=32 ./scripts/run-qemu-repros.sh
#   RUN_TIMEOUT=5 LIMIT=50 ./scripts/run-qemu-repros.sh   # first 50 of shard
#
# Env:
#   SHARD_INDEX / SHARD_COUNT  partition crepros (default 0/1 = all)
#   RUN_TIMEOUT               guest per-binary timeout seconds (default 5)
#   COMPILE_TIMEOUT           guest per-compile timeout (default 30)
#   LIMIT                     max sources in this run (0 = all in shard)
#   SSH_PORT                  host forward port (default 2222)
#   ARTIFACT_DIR              working dir for image/logs (default .qemu-run)
#   KEEP_VM                   if 1, leave QEMU running on exit

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

mkdir -p "$ARTIFACT_DIR"
SERIAL_LOG="$ARTIFACT_DIR/serial.log"
QEMU_PID_FILE="$ARTIFACT_DIR/qemu.pid"
RESULTS_HOST="$ARTIFACT_DIR/repro-results.tsv"
SHARD_LIST="$ARTIFACT_DIR/shard-list.txt"
SHARD_TAR="$ARTIFACT_DIR/crepros-shard.tar.gz"

rm -f "$SERIAL_LOG" "$QEMU_PID_FILE" "$RESULTS_HOST" "$SHARD_LIST" "$SHARD_TAR"

echo "Selecting shard $SHARD_INDEX/$SHARD_COUNT from crepros/ ..."
python3 - "$SHARD_INDEX" "$SHARD_COUNT" "$LIMIT" <<'PY' >"$SHARD_LIST"
import os, sys, zlib
idx, count, limit = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
files = sorted(
    f for f in os.listdir("crepros")
    if f.endswith(".c") and os.path.isfile(os.path.join("crepros", f))
)
# Stable partition (not Python's salted hash()) so shards match across hosts.
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
	exit 0
fi
echo "Packing $n_sources sources..."
# Transform crepros/foo.c -> foo.c inside the archive under crepros/
tar -czf "$SHARD_TAR" -T "$SHARD_LIST"
ls -lh "$SHARD_TAR"

img_url="https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-genericcloud-amd64.qcow2"
cache="/tmp/debian-12-genericcloud-amd64.qcow2"
if [[ ! -f "$cache" ]]; then
	wget -q --show-progress -O "$cache" "$img_url"
fi
cp -f "$cache" "$ARTIFACT_DIR/disk.qcow2"
qemu-img resize "$ARTIFACT_DIR/disk.qcow2" "${DISK_SIZE_G}G"

rm -f "$ARTIFACT_DIR/id_rsa" "$ARTIFACT_DIR/id_rsa.pub"
ssh-keygen -t ed25519 -N '' -f "$ARTIFACT_DIR/id_rsa" -C "lkrt-ci" >/dev/null
PUB=$(cat "$ARTIFACT_DIR/id_rsa.pub")

cat >"$ARTIFACT_DIR/user-data" <<EOF
#cloud-config
hostname: lkrt-ci
manage_etc_hosts: true
disable_root: false
ssh_pwauth: false
package_update: false
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
  - [ bash, -c, "echo '${PUB}' >> /home/debian/.ssh/authorized_keys && chown -R debian:debian /home/debian/.ssh && chmod 700 /home/debian/.ssh && chmod 600 /home/debian/.ssh/authorized_keys" ]
  - [ systemctl, restart, ssh ]
EOF

cat >"$ARTIFACT_DIR/meta-data" <<EOF
instance-id: lkrt-ci-$(date +%s)-${SHARD_INDEX}
local-hostname: lkrt-ci
EOF

cloud-localds "$ARTIFACT_DIR/seed.iso" "$ARTIFACT_DIR/user-data" "$ARTIFACT_DIR/meta-data"
: >"$SERIAL_LOG"

ACCEL=tcg
if [[ -r /dev/kvm ]]; then ACCEL=kvm; fi
echo "Using accel=$ACCEL"

qemu-system-x86_64 \
	-name "lkrt-ci-s${SHARD_INDEX}" \
	-machine "pc,accel=$ACCEL" \
	-m 2G \
	-smp 2 \
	-drive "file=$ARTIFACT_DIR/disk.qcow2,format=qcow2,if=virtio" \
	-drive "file=$ARTIFACT_DIR/seed.iso,format=raw,if=virtio,media=cdrom" \
	-netdev "user,id=net0,hostfwd=tcp::${SSH_PORT}-:22" \
	-device virtio-net-pci,netdev=net0 \
	-display none \
	-serial "file:$SERIAL_LOG" \
	-pidfile "$QEMU_PID_FILE" \
	-daemonize

cleanup() {
	if [[ "$KEEP_VM" != "1" && -f "$QEMU_PID_FILE" ]]; then
		kill "$(cat "$QEMU_PID_FILE")" 2>/dev/null || true
	fi
}
trap cleanup EXIT

echo "QEMU pid $(cat "$QEMU_PID_FILE")"

SSH_BASE=(
	ssh -i "$ARTIFACT_DIR/id_rsa"
	-o StrictHostKeyChecking=no
	-o UserKnownHostsFile=/dev/null
	-o ConnectTimeout=10
	-o LogLevel=ERROR
	-o ServerAliveInterval=30
	-p "$SSH_PORT"
)
SCP_BASE=(
	scp -i "$ARTIFACT_DIR/id_rsa"
	-o StrictHostKeyChecking=no
	-o UserKnownHostsFile=/dev/null
	-o ConnectTimeout=10
	-o LogLevel=ERROR
	-P "$SSH_PORT"
)

echo "Waiting for SSH..."
USER=""
for i in $(seq 1 90); do
	# Prefer root for installs + compile.
	if "${SSH_BASE[@]}" root@127.0.0.1 'echo up' 2>/dev/null; then
		USER=root
		break
	fi
	if "${SSH_BASE[@]}" debian@127.0.0.1 'echo up' 2>/dev/null; then
		USER=debian
		break
	fi
	if [[ $((i % 6)) -eq 0 ]]; then
		echo "  still waiting ($i/90)..."
	fi
	sleep 5
done
if [[ -z "$USER" ]]; then
	echo "SSH failed"
	tail -n 200 "$SERIAL_LOG" || true
	exit 1
fi
echo "SSH as $USER"
SSH=("${SSH_BASE[@]}" "${USER}@127.0.0.1")
if [[ "$USER" != "root" ]]; then
	SUDO=(sudo -n)
else
	SUDO=()
fi

echo "Installing guest toolchain..."
"${SSH[@]}" "${SUDO[@]}" bash -s <<'EOF'
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y --no-install-recommends gcc gcc-multilib libc6-dev make ca-certificates
EOF

REMOTE_BASE="/home/debian"
[[ "$USER" == "root" ]] && REMOTE_BASE="/root"
REMOTE_CREPROS="$REMOTE_BASE/crepros"
REMOTE_RESULTS="$REMOTE_BASE/repro-results.tsv"

echo "Uploading sources + guest runner..."
"${SSH[@]}" mkdir -p "$REMOTE_CREPROS"
"${SCP_BASE[@]}" "$SHARD_TAR" "${USER}@127.0.0.1:$REMOTE_BASE/crepros-shard.tar.gz"
"${SCP_BASE[@]}" scripts/guest-run-repros.sh "${USER}@127.0.0.1:$REMOTE_BASE/guest-run-repros.sh"
"${SSH[@]}" bash -s <<EOF
set -euo pipefail
tar -xzf "$REMOTE_BASE/crepros-shard.tar.gz" -C "$REMOTE_BASE"
# tarball paths are crepros/foo.c → end up in \$REMOTE_BASE/crepros
chmod +x "$REMOTE_BASE/guest-run-repros.sh"
ls "$REMOTE_CREPROS" | wc -l
EOF

echo "Compile + run inside guest (timeout ${RUN_TIMEOUT}s each)..."
# Stream progress to host log; do not fail the host shell on individual repros.
set +e
"${SSH[@]}" \
	env CREPROS_DIR="$REMOTE_CREPROS" \
	RUN_TIMEOUT="$RUN_TIMEOUT" \
	COMPILE_TIMEOUT="$COMPILE_TIMEOUT" \
	RESULTS_FILE="$REMOTE_RESULTS" \
	bash "$REMOTE_BASE/guest-run-repros.sh"
guest_rc=$?
set -e
echo "guest exit=$guest_rc"

"${SCP_BASE[@]}" "${USER}@127.0.0.1:$REMOTE_RESULTS" "$RESULTS_HOST" || true
if [[ -f "$RESULTS_HOST" ]]; then
	echo "---- results summary ----"
	# count statuses
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
fi

if [[ "$guest_rc" -ne 0 ]]; then
	exit "$guest_rc"
fi
echo "QEMU_REPROS_OK shard=${SHARD_INDEX}/${SHARD_COUNT} n=${n_sources}"
