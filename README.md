## Linux Kernel Regression Tests

C reproducers for upstream Linux kernel crashes (from
[syzbot](https://syzkaller.appspot.com/upstream/fixed)), for checking whether
newer kernels still hit known bugs.

### Layout

| Path | Purpose |
|------|---------|
| `crepros/` | C reproducer sources (tracked in git) |
| `compiled/` | Optional host-side build output from `./compile.sh` (**not** tracked) |
| `fetch.py` | Scrape new C repros from syzbot |
| `compile.sh` | Optional host-side compiler helper |
| `scripts/run-qemu-repros.sh` | Boot QEMU, ship sources, compile+run inside the guest |
| `scripts/guest-run-repros.sh` | In-guest compile/run loop |

### Local use

```bash
# Optional: refresh sources from syzbot
python3 -m pip install beautifulsoup4 requests
python3 fetch.py

# Full suite inside QEMU — default KERNEL_MODE=kasan boots a syzbot KASAN
# bzImage with panic_on_warn / oops=panic on a Debian cloud rootfs.
./scripts/run-qemu-repros.sh

# One shard of 32 (same partitioning as CI)
SHARD_INDEX=0 SHARD_COUNT=32 ./scripts/run-qemu-repros.sh

# Smoke: 20 sources, 5s timeout
LIMIT=20 RUN_TIMEOUT=5 ./scripts/run-qemu-repros.sh

# Stock distro kernel (weak signal, no KASAN)
KERNEL_MODE=distro LIMIT=10 ./scripts/run-qemu-repros.sh

# Optional host-side builds (not required for the QEMU path)
./compile.sh --limit 50
```

Pinned kernel URLs live in `scripts/kernel-assets.conf` (refresh from a recent
syzbot bug assets list when needed).

### CI workflows

| Workflow | Trigger | What it does |
|----------|---------|----------------|
| **Fetch Repros** | Daily + changes to `fetch.py` | Runs `fetch.py`, commits new files under `crepros/`; dispatches the QEMU job when something new landed |
| **Run Tests in QEMU VM** | Weekly + changes under `crepros/` / scripts | Partitions **all** `crepros/*.c` across parallel QEMU shards; each guest installs `gcc`, compiles every assigned repro, and runs it with a **5s** timeout |

Pull requests use a small 2×20 smoke so review stays cheap. Push, schedule, and
manual dispatch run the full sharded suite (default **32** shards).

GitHub disables scheduled workflows after ~60 days of repository inactivity.
If schedules stop firing, open the Actions tab and click **Enable workflow**,
or push any commit to the default branch.

Manual runs: **Actions → workflow → Run workflow** (optional `shard_count`,
`run_timeout`, `limit_per_shard`).

### In-guest execution model

1. Host selects a deterministic shard of `crepros/*.c` and packs a tarball.
2. QEMU boots a Debian cloud **rootfs** with either:
   - **`KERNEL_MODE=kasan` (default):** syzbot **KASAN** `bzImage`, cmdline
     `oops=panic panic_on_warn=1 panic=0`, QEMU `-no-reboot`, 4G RAM; or
   - **`KERNEL_MODE=distro`:** stock cloud kernel (smoke only).
   Cloud-init injects an SSH key and installs `gcc` / `gcc-multilib`.
3. For each source: compile (dynamic link, optional `-m32`), run under
   `timeout -s KILL $RUN_TIMEOUT` (default 5 seconds), stream a `RESULT` line
   to the host, delete the binary.
4. If the guest panics, SSH drops. The host attributes the crash to the repro
   that last emitted `BEGIN` / `LKRT_BEGIN` (also stamped on the QEMU serial
   console), scrapes the serial log for oops/panic text, and writes
   `crashes/<repro>.md` with the excerpt. Status is `PANIC` when an oops
   signature is found, otherwise `GUEST_CRASH`. The VM is rebooted and the
   suite **resumes** the remainder of the shard.
5. Results land in `repro-results.tsv` plus `panics.tsv` and per-crash
   markdown reports (uploaded as CI artifacts).

This is still a **signal** suite against a distro guest kernel, not a full
upstream `bzImage` + KASAN syzbot reproduction environment.
