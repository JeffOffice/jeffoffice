#!/bin/bash
# Runs the test programs side by side for CI, as many at once as the machine has processors, each with its own results file
# (<name>-results.txt), then prints each one's results in turn. A program that crashed (no QtTest totals line) is run again
# under the system's debugger, alone, for the stack. The exit status is 0 only when every program passed.
#
#     tools/ci_tests.sh jptests officetests sheettests ...
#
# The longest programs go first, so the side-by-side runs finish together.
set -u

ext=""; [ "${RUNNER_OS:-}" = "Windows" ] && ext=".exe"
jobs=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

printf '%s\n' "$@" | xargs -P "$jobs" -I '{}' \
    bash -c 'started=$(date +%s); "build/$1$2" -v1 -o "$1-results.txt,txt" -o -,txt > "$1.log" 2>&1; echo $? > "$1.code"; echo "$1 finished in $(( $(date +%s) - started )) s"' _ '{}' "$ext"

# (macOS has no timeout command; Homebrew's coreutils name it gtimeout; without either, the debugger runs unbounded under the step's own time limit)
T=$(command -v timeout || command -v gtimeout || true)

code=0
for N in "$@"; do
    P=build/$N$ext
    ncode=$(cat "$N.code" 2>/dev/null || echo 1)
    echo "--- $N-results.txt"; cat "$N-results.txt" 2>/dev/null || echo "(no results file: the program did not start)"
    echo "$N exit code: $ncode"
    if [ "$ncode" -ne 0 ]; then echo "--- $N.log (the end)"; tail -60 "$N.log"; fi
    # A crash: a nonzero exit with no failed test in the totals (QtTest exits with the number of failures; a program of several classes
    # prints a totals line for each class that finished, so a totals line alone doesn't say the program did). The stack of the first
    # crash is in the log; a crash can depend on the tests before it.
    failed=$(grep -oE 'Totals: [0-9]+ passed, [0-9]+ failed' "$N-results.txt" 2>/dev/null | awk '{s += $4} END {print s + 0}')
    if [ "$ncode" -ne 0 ] && [ "$failed" -eq 0 ]; then
        echo "--- stack of the crash of $N"
        if [ "${RUNNER_OS:-}" = "macOS" ]; then
            $T ${T:+900} lldb --batch -o run -k "thread backtrace all -c 30" -- "$P" -silent 2>&1 | tail -120
        else
            G=gdb
            if [ "${RUNNER_OS:-}" = "Windows" ]; then G="$(cygpath -u "$IQTA_TOOLS")/mingw1310_64/bin/gdb.exe"; fi
            $T ${T:+900} "$G" -q -batch -ex run -ex "thread apply all bt 30" --args "$P" -silent 2>&1 | grep -v '^\[' | tail -120
        fi
    fi
    if [ "$code" -eq 0 ]; then code=$ncode; fi
done
exit $code
