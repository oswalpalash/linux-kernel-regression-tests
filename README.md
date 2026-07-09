## Linux Kernel Regression Tests

C reproducers for upstream Linux kernel crashes (from
[syzbot](https://syzkaller.appspot.com/upstream/fixed)), for checking whether
newer kernels still hit known bugs.

### Layout

| Path | Purpose |
|------|---------|
| `crepros/` | C reproducer sources (tracked in git) |
| `compiled/` | Build output from `./compile.sh` (**not** tracked) |
| `fetch.py` | Scrape new C repros from syzbot |
| `compile.sh` | Compile repros to static binaries |

Compiled binaries used to live in this repository (~44 GiB). They are now
produced on demand in CI or locally and are gitignored.

### Local use

```bash
# Optional: refresh sources from syzbot
python3 -m pip install beautifulsoup4 requests
python3 fetch.py

# Build (all missing, or a sample)
./compile.sh                  # compile anything not yet in compiled/
./compile.sh --limit 50       # at most 50 new binaries
./compile.sh --force file.c   # rebuild specific sources
./compile.sh --jobs 8
```

### CI workflows

| Workflow | Trigger | What it does |
|----------|---------|----------------|
| **Fetch Repros** | Daily + changes to `fetch.py` | Runs `fetch.py`, commits new files under `crepros/` |
| **Compile reproducers** | Daily + changes under `crepros/` | Compiles sources (no binary commits); uploads `*.err` on failure |
| **Run Tests in QEMU VM** | Weekly + changes under `crepros/` | Compiles a random sample, boots Debian cloud image in QEMU, runs binaries via 9p |

GitHub disables scheduled workflows after ~60 days of repository inactivity.
If schedules stop firing, open the Actions tab and click **Enable workflow**
on each workflow, or push any commit to the default branch.

Manual runs: **Actions → workflow → Run workflow**.

### Notes on the QEMU job

The GHA job is a **smoke test** (default 32 repros), not a full 40k+ suite.
Full regression against a custom upstream `bzImage` is intended to be run
elsewhere with more disk, KVM, and time; this workflow validates that fetch →
compile → execute still works end-to-end.
