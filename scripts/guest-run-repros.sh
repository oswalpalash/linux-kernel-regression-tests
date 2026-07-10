#!/bin/bash
# Runs inside the QEMU guest: compile each C repro and execute it with a short timeout.
#
# Protocol on stdout (line-buffered, flushed):
#   BEGIN\t<file>          — about to compile/run this repro
#   RESULT\t<file>\t<status>\t<compile_rc>\t<run_rc>
#
# Also stamps the QEMU serial console with LKRT_BEGIN / LKRT_END so a panic
# after BEGIN can be attributed to that repro from the host serial log.
#
# Env:
#   CREPROS_DIR, RUN_TIMEOUT (default 5), COMPILE_TIMEOUT (default 30),
#   RESULTS_FILE (optional local TSV mirror), PROGRESS_EVERY (default 25)

set -u

CREPROS_DIR="${CREPROS_DIR:-$HOME/crepros}"
RUN_TIMEOUT="${RUN_TIMEOUT:-5}"
COMPILE_TIMEOUT="${COMPILE_TIMEOUT:-30}"
RESULTS_FILE="${RESULTS_FILE:-}"
PROGRESS_EVERY="${PROGRESS_EVERY:-25}"

if ! command -v gcc >/dev/null 2>&1; then
	echo "ERROR: gcc not installed" >&2
	exit 2
fi

mapfile -t sources < <(find "$CREPROS_DIR" -maxdepth 1 -type f -name '*.c' | LC_ALL=C sort)
total_sources=${#sources[@]}
if [[ "$total_sources" -eq 0 ]]; then
	echo "ERROR: no .c files under $CREPROS_DIR" >&2
	exit 1
fi

echo "guest-run-repros: $total_sources sources in $CREPROS_DIR" >&2
echo "  RUN_TIMEOUT=${RUN_TIMEOUT}s COMPILE_TIMEOUT=${COMPILE_TIMEOUT}s" >&2

if [[ -n "$RESULTS_FILE" ]]; then
	printf 'file\tstatus\tcompile_rc\trun_rc\n' >"$RESULTS_FILE"
fi

# Stamp both stdout (host SSH stream) and the physical console (QEMU serial).
console_stamp() {
	# $1=tag $2=name
	local msg="$1 $2"
	echo "$msg" >&2
	# /dev/console reaches the VGA/serial console used by QEMU -serial file:
	echo "$msg" >/dev/console 2>/dev/null || echo "$msg" >/dev/ttyS0 2>/dev/null || true
	# Nudge the log so the stamp is less likely to sit in a kernel buffer.
	echo >/dev/console 2>/dev/null || true
}

emit_begin() {
	python3 -c 'import sys; print("BEGIN\t" + sys.argv[1], flush=True)' "$1"
	console_stamp "LKRT_BEGIN" "$1"
}

emit_result() {
	# $1=file $2=status $3=compile_rc $4=run_rc
	python3 -c 'import sys; print("RESULT\t" + "\t".join(sys.argv[1:]), flush=True)' "$1" "$2" "$3" "$4"
	console_stamp "LKRT_END" "$1"
	if [[ -n "$RESULTS_FILE" ]]; then
		printf '%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" >>"$RESULTS_FILE"
		sync "$RESULTS_FILE" 2>/dev/null || true
	fi
}

ok=0
compile_fail=0
timeout_n=0
run_nonzero=0
done_n=0

for f in "${sources[@]}"; do
	base="$(basename "$f")"
	done_n=$((done_n + 1))
	emit_begin "$base"

	flags=()
	if grep -q '__NR_mmap2' "$f" 2>/dev/null; then
		flags+=(-m32)
	fi

	bin="$(mktemp /tmp/repro-XXXXXX)"
	timeout -s KILL "$COMPILE_TIMEOUT" gcc "$f" -pthread "${flags[@]}" -o "$bin" >/tmp/repro-compile.err 2>&1
	crc=$?
	if [[ "$crc" -ne 0 ]]; then
		emit_result "$base" "COMPILE_FAIL" "$crc" "-"
		compile_fail=$((compile_fail + 1))
		rm -f "$bin"
	else
		workdir="$(mktemp -d /tmp/repro-wd-XXXXXX)"
		# Drop caches so oops paths are a bit noisier / less deferred? skip — costly.
		set +e
		(cd "$workdir" && timeout -s KILL "$RUN_TIMEOUT" "$bin") >/dev/null 2>&1
		rrc=$?
		set -e
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
		emit_result "$base" "$status" "0" "$rrc"
	fi

	if [[ $((done_n % PROGRESS_EVERY)) -eq 0 || "$done_n" -eq "$total_sources" ]]; then
		echo "PROGRESS $done_n/$total_sources ok=$ok cfail=$compile_fail timeout=$timeout_n nonzero=$run_nonzero" >&2
	fi
done

echo "SUMMARY total=$done_n ok=$ok compile_fail=$compile_fail timeout=$timeout_n run_nonzero=$run_nonzero" >&2
exit 0
