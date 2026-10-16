#!/bin/sh
# Runs every test class of a test program as a process of its own, several at a time, and prints
# one Totals line for each class and the sum of them.
#
#     tools/runtests.sh build/jdtests [jobs]
#
# jobs is how many classes run at once (default: the number of processors). The exit status is 0
# when every class passed. One class alone: build/jdtests --class DeckTableTests; the classes:
# build/jdtests --classes. The programs are jptests, officetests, and jdtests.
set -u

[ $# -ge 1 ] || { echo "usage: $0 <test program> [jobs]" >&2; exit 2; }
program=$1
jobs=${2:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}
classes=$("$program" --classes) || { echo "$program does not list its test classes" >&2; exit 2; }

results=$(mktemp -d)
trap 'rm -rf "$results"' EXIT
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}" JP_NO_TELEMETRY=1

started=$(date +%s)
printf '%s\n' $classes | xargs -P "$jobs" -I '{}' \
    sh -c '"$0" --class "$1" > "$2/$1.out" 2>&1; echo $? > "$2/$1.code"' "$program" '{}' "$results"
seconds=$(( $(date +%s) - started ))

status=0
passed=0 failed=0 skipped=0
for class in $classes; do
    code=$(cat "$results/$class.code" 2>/dev/null || echo 1)
    totals=$(grep '^Totals:' "$results/$class.out" | tail -n 1)
    if [ -z "$totals" ]; then
        echo "$class: no Totals line (exit code $code); its last output:"
        tail -n 15 "$results/$class.out" | sed 's/^/    /'
        status=1
        continue
    fi
    echo "$class: $totals"
    set -- $(echo "$totals" | sed 's/[^0-9]/ /g')
    passed=$((passed + $1)) failed=$((failed + $2)) skipped=$((skipped + $3))
    if [ "$code" -ne 0 ] || [ "$2" -ne 0 ]; then
        status=1
        awk '/^FAIL!/ { show = 1; print; next } /^ / { if (show) print; next } { show = 0 }' "$results/$class.out" | sed 's/^/    /'
    fi
done
echo "Total of $(echo $classes | wc -w | tr -d ' ') classes: $passed passed, $failed failed, $skipped skipped, ${seconds}s"
exit $status
