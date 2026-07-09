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
| `compile.sh` | Compile repros to static binaries (local / used by test CI) |

Compiled binaries are not stored in git. Build them on demand.

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
| **Fetch Repros** | Daily + changes to `fetch.py` | Runs `fetch.py`, commits new files under `crepros/`; dispatches the QEMU job when something new landed |
| **Run Tests in QEMU VM** | Weekly + changes under `crepros/` | Compiles a random sample, boots a Debian cloud image in QEMU, runs binaries via 9p |

There is no separate full-tree compile workflow: compiling ~45k static
binaries is too large for GitHub Actions, and the QEMU job already builds
whatever sample it needs.

GitHub disables scheduled workflows after ~60 days of repository inactivity.
If schedules stop firing, open the Actions tab and click **Enable workflow**,
or push any commit to the default branch.

Manual runs: **Actions → workflow → Run workflow**.

### Notes on the QEMU job

The GHA job is a **smoke test** (default 32 repros), not a full 40k+ suite.
Full regression against a custom upstream `bzImage` is intended to be run
elsewhere with more disk, KVM, and time; this workflow validates that fetch →
compile → execute still works end-to-end.
