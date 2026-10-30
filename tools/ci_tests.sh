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
    bash -c 'started=$(date +%s); "build/$1$2" -v1 -o "$1-results.txt,txt" > "$1.log" 2>&1; echo $? > "$1.code"; echo "$1 finished in $(( $(date +%s) - started )) s"' _ '{}' "$ext"

code=0
for N in "$@"; do
    P=build/$N$ext
    ncode=$(cat "$N.code" 2>/dev/null || echo 1)
    echo "--- $N-results.txt"; cat "$N-results.txt" 2>/dev/null || echo "(no results file: the program did not start)"
    echo "$N exit code: $ncode"
    if [ "$ncode" -ne 0 ]; then echo "--- $N.log (the end)"; tail -60 "$N.log"; fi
    # (a crash is a failure with no QtTest totals line: the exit code of a native crash can reach bash cut short, so it is not what says so;
    # the stack of the first crash is in the log, and a crash can depend on the tests before it)
    if [ "$ncode" -ne 0 ] && ! grep -q "Totals:" "$N-results.txt" 2>/dev/null; then
        echo "--- stack of the crash of $N"
        if [ "${RUNNER_OS:-}" = "macOS" ]; then
            timeout 900 lldb --batch -o run -k "thread backtrace all -c 30" -- "$P" -silent 2>&1 | tail -120
        else
            G=gdb
            if [ "${RUNNER_OS:-}" = "Windows" ]; then G="$(cygpath -u "$IQTA_TOOLS")/mingw1310_64/bin/gdb.exe"; fi
            timeout 900 "$G" -q -batch -ex run -ex "thread apply all bt 30" --args "$P" -silent 2>&1 | grep -v '^\[' | tail -120
        fi
    fi
    if [ "$code" -eq 0 ]; then code=$ncode; fi
done
exit $code
