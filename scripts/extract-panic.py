#!/usr/bin/env python3
"""Extract kernel panic/oops context from a QEMU serial log.

Usage:
  extract-panic.py SERIAL_LOG [--repro NAME] [--out REPORT.md]

Finds the last LKRT_BEGIN marker (optionally for a given repro) and the
following oops/panic block. Prints a short title to stdout; writes a full
report to --out when given. Exit 0 if a panic/oops was found, 1 otherwise.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

# Patterns that indicate a real kernel failure on the console.
PANIC_HINTS = re.compile(
    r"(?:"
    r"Kernel panic"
    r"|BUG:"
    r"|Oops:"
    r"|general protection fault"
    r"|unable to handle kernel"
    r"|Unable to handle kernel"
    r"|KASAN:"
    r"|UBSAN:"
    r"|BUG: KASAN:"
    r"|BUG: KFENCE:"
    r"|BUG: soft lockup"
    r"|WARNING: CPU:"
    r"|RIP:"
    r"|Call Trace:"
    r"|------------\[ cut here \]------------"
    r"|invalid opcode:"
    r"|divide error:"
    r"|stack segment:"
    r"|smp: softlockup"
    r")",
    re.IGNORECASE,
)

BEGIN_RE = re.compile(r"LKRT_BEGIN\s+(\S+)")
END_RE = re.compile(r"LKRT_END\s+(\S+)")


def load_lines(path: Path) -> list[str]:
    raw = path.read_bytes()
    # Serial may be latin-1 / mixed; don't crash on decode.
    text = raw.decode("utf-8", errors="replace")
    return text.splitlines()


def find_begin_index(lines: list[str], repro: str | None) -> tuple[int, str | None]:
    """Return (line_index, repro_name) for the relevant LKRT_BEGIN."""
    last: tuple[int, str] | None = None
    for i, line in enumerate(lines):
        m = BEGIN_RE.search(line)
        if not m:
            continue
        name = m.group(1)
        if repro is None or name == repro or name.rstrip(".c") == repro.rstrip(".c"):
            last = (i, name)
    if last:
        return last[0], last[1]
    return -1, repro


def extract_block(lines: list[str], start: int) -> list[str]:
    """Take lines from start through panic tail (or end of log)."""
    if start < 0:
        start = 0
    # Prefer window starting a bit before BEGIN for context.
    window_start = max(0, start - 5)
    chunk = lines[window_start:]

    # If we see another LKRT_BEGIN after the first panic, stop before it
    # (belongs to a later attempt after reboot — shouldn't happen in one serial).
    panic_idx = None
    for i, line in enumerate(chunk):
        if PANIC_HINTS.search(line):
            panic_idx = i
            break
    if panic_idx is None:
        # No classic panic string — still return tail after BEGIN for forensics.
        return chunk[: min(len(chunk), 80)]

    # Extend from a few lines before first hint through call trace / panic end.
    block_start = max(0, panic_idx - 15)
    end = panic_idx + 1
    end_markers = re.compile(
        r"Kernel panic|---\[ end trace|RIP:|Kernel Offset:|Rebooting in|---\[ end",
        re.I,
    )
    # Keep going until things go quiet or we hit login prompt after panic.
    for j in range(panic_idx, min(len(chunk), panic_idx + 200)):
        end = j + 1
        if re.search(r"login:|cloud-init|Debian GNU", chunk[j]):
            # After full panic, serial may wrap to login on reboot — stop.
            if j > panic_idx + 20:
                break
    return chunk[block_start:end]


def title_from_block(block: list[str]) -> str:
    for line in block:
        s = line.strip()
        if not s:
            continue
        if PANIC_HINTS.search(s):
            # Collapse whitespace
            return re.sub(r"\s+", " ", s)[:240]
    if block:
        return re.sub(r"\s+", " ", block[0].strip())[:240]
    return "unknown crash (no oops signature in serial)"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("serial_log", type=Path)
    ap.add_argument("--repro", default=None, help="Repro basename expected to be running")
    ap.add_argument("--out", type=Path, default=None, help="Write full markdown report here")
    ap.add_argument("--cycle", default="", help="Boot cycle id for the report header")
    args = ap.parse_args()

    if not args.serial_log.is_file():
        print("no serial log", file=sys.stderr)
        return 1

    lines = load_lines(args.serial_log)
    begin_i, repro_name = find_begin_index(lines, args.repro)
    block = extract_block(lines, begin_i if begin_i >= 0 else max(0, len(lines) - 100))
    title = title_from_block(block)
    found = any(PANIC_HINTS.search(l) for l in block)

    repro_name = repro_name or args.repro or "unknown"

    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        body = []
        body.append(f"# Guest crash report")
        body.append("")
        body.append(f"- **repro**: `{repro_name}`")
        if args.cycle != "":
            body.append(f"- **boot cycle**: {args.cycle}")
        body.append(f"- **serial log**: `{args.serial_log}`")
        body.append(f"- **panic signature found**: {found}")
        body.append(f"- **title**: {title}")
        body.append("")
        body.append("## Serial excerpt")
        body.append("")
        body.append("```")
        body.extend(block if block else ["(empty)"])
        body.append("```")
        body.append("")
        # Also note nearby BEGIN markers for confidence.
        begins = [BEGIN_RE.search(l).group(1) for l in lines if BEGIN_RE.search(l)]
        if begins:
            body.append("## LKRT_BEGIN markers in this serial (order)")
            body.append("")
            for b in begins[-20:]:
                body.append(f"- `{b}`")
            body.append("")
        args.out.write_text("\n".join(body) + "\n", encoding="utf-8")

    # Machine-readable one-liner for the host driver:
    # PANIC_TITLE<TAB>repro<TAB>found(0|1)<TAB>title
    print(f"PANIC_TITLE\t{repro_name}\t{1 if found else 0}\t{title}")
    return 0 if found else 1


if __name__ == "__main__":
    sys.exit(main())
