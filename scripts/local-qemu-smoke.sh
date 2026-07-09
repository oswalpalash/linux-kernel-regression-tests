#!/bin/bash
# Local mirror of .github/workflows/test.yaml (smoke subset).
set -euo pipefail
cd "$(dirname "$0")/.."

SAMPLE_SIZE="${SAMPLE_SIZE:-4}"
SSH_PORT="${SSH_PORT:-2222}"
WORKDIR="$(pwd)"
ARTIFACT_DIR="${ARTIFACT_DIR:-$WORKDIR/.qemu-run}"
mkdir -p "$ARTIFACT_DIR"
SERIAL_LOG="${SERIAL_LOG:-$ARTIFACT_DIR/serial.log}"
QEMU_PID_FILE="${QEMU_PID_FILE:-$ARTIFACT_DIR/qemu.pid}"

rm -f "$SERIAL_LOG" "$QEMU_PID_FILE"
rm -rf compiled
mkdir -p compiled
chmod +x ./compile.sh

python3 - "$SAMPLE_SIZE" <<'PY' > /tmp/sample-list.txt
import os, random, sys
n = int(sys.argv[1])
files = [os.path.join("crepros", f) for f in os.listdir("crepros") if f.endswith(".c")]
random.seed(42)
random.shuffle(files)
for f in files[:n]:
    print(f)
PY
echo "Sample ($SAMPLE_SIZE):"
cat /tmp/sample-list.txt
mapfile -t sample < /tmp/sample-list.txt
./compile.sh --jobs "$(nproc)" --force "${sample[@]}"
echo "Built $(find compiled -type f ! -name '*.err' | wc -l) binaries"

img_url="https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-genericcloud-amd64.qcow2"
cache="/tmp/debian-12-genericcloud-amd64.qcow2"
if [ ! -f "$cache" ]; then
  wget -q -O "$cache" "$img_url"
fi
cp -f "$cache" "$ARTIFACT_DIR/disk.qcow2"
qemu-img resize "$ARTIFACT_DIR/disk.qcow2" 4G

rm -f "$ARTIFACT_DIR/id_rsa" "$ARTIFACT_DIR/id_rsa.pub"
ssh-keygen -t ed25519 -N '' -f "$ARTIFACT_DIR/id_rsa" -C "lkrt-ci" >/dev/null
PUB=$(cat "$ARTIFACT_DIR/id_rsa.pub")

cat > "$ARTIFACT_DIR/user-data" <<EOF
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
  - [ bash, -c, "mkdir -p /home/debian/.ssh /root/.ssh /root/repros" ]
  - [ bash, -c, "echo '${PUB}' >> /home/debian/.ssh/authorized_keys && chown -R debian:debian /home/debian/.ssh && chmod 700 /home/debian/.ssh && chmod 600 /home/debian/.ssh/authorized_keys" ]
  - [ bash, -c, "echo '${PUB}' > /root/.ssh/authorized_keys && chmod 600 /root/.ssh/authorized_keys" ]
  - [ systemctl, restart, ssh ]
EOF

cat > "$ARTIFACT_DIR/meta-data" <<EOF
instance-id: lkrt-ci-$(date +%s)
local-hostname: lkrt-ci
EOF

cloud-localds "$ARTIFACT_DIR/seed.iso" "$ARTIFACT_DIR/user-data" "$ARTIFACT_DIR/meta-data"
: > "$SERIAL_LOG"

ACCEL=tcg
if [ -r /dev/kvm ]; then ACCEL=kvm; fi
echo "Using accel=$ACCEL"

qemu-system-x86_64 \
  -name lkrt-ci \
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
  if [ -f "$QEMU_PID_FILE" ]; then
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
  if "${SSH_BASE[@]}" debian@127.0.0.1 'echo up' 2>/dev/null; then
    USER=debian
    break
  fi
  if "${SSH_BASE[@]}" root@127.0.0.1 'echo up' 2>/dev/null; then
    USER=root
    break
  fi
  if [ $((i % 6)) -eq 0 ]; then
    echo "  still waiting ($i/90)..."
    tail -n 5 "$SERIAL_LOG" 2>/dev/null || true
  fi
  sleep 5
done

if [ -z "$USER" ]; then
  echo "SSH failed"
  tail -n 200 "$SERIAL_LOG" || true
  exit 1
fi
echo "SSH as $USER"

SSH=("${SSH_BASE[@]}" "${USER}@127.0.0.1")
# Create the upload dir as the SSH user (no sudo). sudo mkdir as debian makes
# root-owned dirs and scp then fails with Permission denied (seen on GHA).
if [ "$USER" = "root" ]; then
  REMOTE_DIR=/root/repros
else
  REMOTE_DIR=/home/debian/repros
fi

"${SSH[@]}" mkdir -p "$REMOTE_DIR"
# scp is more reliable than 9p on stock cloud kernels
"${SCP_BASE[@]}" -r compiled/. "${USER}@127.0.0.1:${REMOTE_DIR}/"
echo "copied files: $("${SSH[@]}" "ls '$REMOTE_DIR' | wc -l")"

"${SSH[@]}" env REMOTE_DIR="$REMOTE_DIR" bash -s <<'EOF'
set +e
ran=0
for test_file in "$REMOTE_DIR"/*; do
  [ -f "$test_file" ] || continue
  case "$test_file" in *.err) continue ;; esac
  chmod +x "$test_file" 2>/dev/null
  echo "=== RUN $test_file ==="
  dir=$(mktemp -d)
  (cd "$dir" && timeout -s KILL 3 "$test_file")
  rc=$?
  echo "=== EXIT $rc $test_file ==="
  rm -rf "$dir"
  ran=$((ran + 1))
done
echo "RAN=$ran"
[ "$ran" -gt 0 ]
EOF

echo "QEMU_TEST_OK"
