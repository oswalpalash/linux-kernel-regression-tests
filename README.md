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

# Full suite inside QEMU (compile in guest, 5s run timeout each)
./scripts/run-qemu-repros.sh

# One shard of 32 (same partitioning as CI)
SHARD_INDEX=0 SHARD_COUNT=32 ./scripts/run-qemu-repros.sh

# Smoke: 20 sources, 5s timeout
LIMIT=20 RUN_TIMEOUT=5 ./scripts/run-qemu-repros.sh

# Optional host-side builds (not required for the QEMU path)
./compile.sh --limit 50
```

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
2. QEMU boots a Debian cloud image; cloud-init injects an SSH key.
3. Guest installs `gcc` / `gcc-multilib` over the network.
4. For each source: compile (dynamic link, optional `-m32`), run under
   `timeout -s KILL $RUN_TIMEOUT` (default 5 seconds), delete the binary.
5. Results land in `repro-results.tsv` (`OK`, `TIMEOUT`, `COMPILE_FAIL`,
   `EXIT_<n>`) and are uploaded as artifacts per shard.

This is still a **smoke / signal** suite against a distro guest kernel, not a
full upstream `bzImage` + KASAN syzbot reproduction environment.
