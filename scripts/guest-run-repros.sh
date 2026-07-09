#!/bin/bash
# Runs inside the QEMU guest: compile each C repro and execute it with a short timeout.
#
# Env:
#   CREPROS_DIR       directory of *.c sources (default: $HOME/crepros)
#   RUN_TIMEOUT       seconds per binary (default: 5)
#   COMPILE_TIMEOUT   seconds per compile (default: 30)
#   RESULTS_FILE      TSV summary path (default: $HOME/repro-results.tsv)
#   PROGRESS_EVERY    log a progress line every N files (default: 25)
#
# Exit 0 always after processing (individual failures are recorded). Non-zero
# only if zero sources were found or the toolchain is missing.

set -u

CREPROS_DIR="${CREPROS_DIR:-$HOME/crepros}"
RUN_TIMEOUT="${RUN_TIMEOUT:-5}"
COMPILE_TIMEOUT="${COMPILE_TIMEOUT:-30}"
RESULTS_FILE="${RESULTS_FILE:-$HOME/repro-results.tsv}"
PROGRESS_EVERY="${PROGRESS_EVERY:-25}"

if ! command -v gcc >/dev/null 2>&1; then
	echo "ERROR: gcc not installed" >&2
	exit 2
fi
if ! command -v timeout >/dev/null 2>&1; then
	echo "ERROR: timeout not installed" >&2
	exit 2
fi

mapfile -t sources < <(find "$CREPROS_DIR" -maxdepth 1 -type f -name '*.c' | LC_ALL=C sort)
total_sources=${#sources[@]}
if [[ "$total_sources" -eq 0 ]]; then
	echo "ERROR: no .c files under $CREPROS_DIR" >&2
	exit 1
fi

echo "guest-run-repros: $total_sources sources in $CREPROS_DIR"
echo "  RUN_TIMEOUT=${RUN_TIMEOUT}s COMPILE_TIMEOUT=${COMPILE_TIMEOUT}s"
printf 'file\tstatus\tcompile_rc\trun_rc\n' >"$RESULTS_FILE"

ok=0
compile_fail=0
timeout_n=0
run_nonzero=0
done_n=0

for f in "${sources[@]}"; do
	base="$(basename "$f")"
	done_n=$((done_n + 1))

	flags=()
	if grep -q '__NR_mmap2' "$f" 2>/dev/null; then
		flags+=(-m32)
	fi

	bin="$(mktemp /tmp/repro-XXXXXX)"
	# Dynamic link inside the guest — much faster than -static for 10k+ files.
	timeout -s KILL "$COMPILE_TIMEOUT" gcc "$f" -pthread "${flags[@]}" -o "$bin" >/tmp/repro-compile.err 2>&1
	crc=$?
	if [[ "$crc" -ne 0 ]]; then
		printf '%s\tCOMPILE_FAIL\t%s\t-\n' "$base" "$crc" >>"$RESULTS_FILE"
		compile_fail=$((compile_fail + 1))
		rm -f "$bin"
		if [[ $((done_n % PROGRESS_EVERY)) -eq 0 ]]; then
			echo "PROGRESS $done_n/$total_sources ok=$ok cfail=$compile_fail timeout=$timeout_n nonzero=$run_nonzero"
		fi
		continue
	fi

	workdir="$(mktemp -d /tmp/repro-wd-XXXXXX)"
	# 124 = timeout(1) soft kill; 137 = SIGKILL (128+9)
	(cd "$workdir" && timeout -s KILL "$RUN_TIMEOUT" "$bin") >/dev/null 2>&1
	rrc=$?
	rm -rf "$workdir" "$bin"

	if [[ "$rrc" -eq 124 || "$rrc" -eq 137 ]]; then
		status="TIMEOUT"
		timeout_n=$((timeout_n + 1))
	elif [[ "$rrc" -eq 0 ]]; then
		status="OK"
		ok=$((ok + 1))
	else
		status="EXIT_${rrc}"
		run_nonzero=$((run_nonzero + 1))
	fi
	printf '%s\t%s\t0\t%s\n' "$base" "$status" "$rrc" >>"$RESULTS_FILE"

	if [[ $((done_n % PROGRESS_EVERY)) -eq 0 || "$done_n" -eq "$total_sources" ]]; then
		echo "PROGRESS $done_n/$total_sources ok=$ok cfail=$compile_fail timeout=$timeout_n nonzero=$run_nonzero"
	fi
done

echo "SUMMARY total=$done_n ok=$ok compile_fail=$compile_fail timeout=$timeout_n run_nonzero=$run_nonzero"
echo "RESULTS $RESULTS_FILE"
# Suite-level success: we exercised sources. Per-repro failures are expected.
exit 0
