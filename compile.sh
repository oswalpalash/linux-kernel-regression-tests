#!/bin/bash
# Compile C reproducers under crepros/ into compiled/.
# Usage:
#   ./compile.sh              # compile everything missing
#   ./compile.sh --force      # recompile all
#   ./compile.sh --limit N    # compile at most N missing files
#   ./compile.sh --jobs N     # parallel jobs (default: nproc)
#   ./compile.sh file.c ...   # compile specific sources (paths or basenames)

set -u

FORCE=0
LIMIT=0
JOBS="$(nproc 2>/dev/null || echo 2)"
SPECIFIC=()

while [[ $# -gt 0 ]]; do
	case "$1" in
		--force|-f) FORCE=1; shift ;;
		--limit) LIMIT="$2"; shift 2 ;;
		--jobs|-j) JOBS="$2"; shift 2 ;;
		-h|--help)
			sed -n '2,10p' "$0"
			exit 0
			;;
		*) SPECIFIC+=("$1"); shift ;;
	esac
done

mkdir -p compiled

compile_one() {
	local f="$1"
	local base out
	base="$(basename "$f" .c)"
	out="compiled/${base}"

	if [[ "${FORCE:-0}" -eq 0 && -f "$out" ]]; then
		echo "SKIP $f"
		return 0
	fi

	local -a flags=()
	if grep -q '__NR_mmap2' "$f" 2>/dev/null; then
		flags+=(-m32)
	fi

	if gcc "$f" -static -pthread "${flags[@]}" -o "$out" 2>"compiled/${base}.err"; then
		rm -f "compiled/${base}.err"
		echo "OK   $f"
		return 0
	fi
	rm -f "$out"
	echo "FAIL $f"
	return 1
}
export -f compile_one
export FORCE

list_file="$(mktemp)"
trap 'rm -f "$list_file"' EXIT

if [[ ${#SPECIFIC[@]} -gt 0 ]]; then
	for s in "${SPECIFIC[@]}"; do
		if [[ -f "$s" ]]; then
			printf '%s\n' "$s"
		elif [[ -f "crepros/$s" ]]; then
			printf '%s\n' "crepros/$s"
		elif [[ -f "crepros/${s}.c" ]]; then
			printf '%s\n' "crepros/${s}.c"
		else
			echo "WARN not found: $s" >&2
		fi
	done >"$list_file"
else
	# Avoid shell glob / ARG_MAX issues with tens of thousands of files
	find crepros -maxdepth 1 -type f -name '*.c' -print | LC_ALL=C sort >"$list_file"
fi

if [[ ! -s "$list_file" ]]; then
	echo "No source files to compile."
	exit 0
fi

if [[ "$LIMIT" -gt 0 ]]; then
	filtered="$(mktemp)"
	count=0
	while IFS= read -r f; do
		base="$(basename "$f" .c)"
		if [[ "$FORCE" -eq 0 && -f "compiled/${base}" ]]; then
			continue
		fi
		printf '%s\n' "$f" >>"$filtered"
		count=$((count + 1))
		if [[ "$count" -ge "$LIMIT" ]]; then
			break
		fi
	done <"$list_file"
	mv "$filtered" "$list_file"
fi

total=$(wc -l <"$list_file" | tr -d ' ')
echo "Compiling ${total} file(s) with ${JOBS} job(s)..."

if [[ "$total" -eq 0 ]]; then
	echo "Nothing to do (all outputs present; use --force to rebuild)."
	exit 0
fi

result="$(mktemp)"
# -d '\n' keeps paths with spaces safe; -r replaces string, -n max args per proc
xargs -a "$list_file" -d '\n' -n 1 -P "$JOBS" bash -c 'compile_one "$@"' _ | tee "$result"

ok=$(grep -c '^OK' "$result" || true)
fail=$(grep -c '^FAIL' "$result" || true)
skip=$(grep -c '^SKIP' "$result" || true)
rm -f "$result"

echo "----"
echo "OK=$ok FAIL=$fail SKIP=$skip"

if [[ "$fail" -gt 0 ]]; then
	echo "Compiler errors are under compiled/*.err"
	exit 1
fi
exit 0
