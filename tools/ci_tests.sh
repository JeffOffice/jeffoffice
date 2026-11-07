#!/bin/bash
# Runs the test programs side by side for CI, as many at once as the machine has processors, each with its own results file
# (<part>-results.txt), then prints each one's results in turn. A program that crashed (no QtTest totals line) is run again
# under the system's debugger, alone, for the stack. The exit status is 0 only when every program passed.
#
#     tools/ci_tests.sh jptests officetests sheettests ...
#     tools/ci_tests.sh crunchtests:1/2 flowtests ...     (a program named <program>:k/n runs only part k of n, see below)
#
# The longest programs go first, so the side-by-side runs finish together. With CI_SPLIT=n (a machine with many processors), the
# biggest programs run in n parts at once, as tools/gate.sh splits them: an app-check program (*checks) runs its three long checks in
# n parts (JO_APPCHECK_SHARD=k/n) and one more part for the rest of its checks; crunchtests, sheettests, jdtests, and jptests run their
# test classes in n parts; writetests and flowtests (one class each) their test functions. All the parts together run what one run does.
#
# <program>:k/n is for a program shared by two CI jobs (each job names its own part, .github/workflows/build.yml, tools/ci_targets.sh): an
# app-check program runs its three long checks for part k of n (JO_APPCHECK_SHARD=k/n), and part n also runs the rest of its checks; the
# test classes of crunchtests, sheettests, jdtests, and jptests are dealt out to the n parts by their weights below (the heaviest class
# first, to the part with the least so far), so that the parts take about the same time. Every part computes the same deal from the
# same program, so all the parts together run each class once. A class not in the table weighs 1; a stale weight only unbalances the parts.
set -u

ext=""; [ "${RUNNER_OS:-}" = "Windows" ] && ext=".exe"

# Seconds of the test classes that take the longest (Windows, Nov 7 2026), for dealing the classes of a program to its parts.
WEIGHTS="CrunchStructModelTests=111 CrunchReviewTests=65 CrunchCellsTests=47 CrunchHostileTests=31 CrunchWindowTests=27 CrunchEditTests=26
CrunchXlsTests=23 CrunchSortTests=19 CrunchPrintTests=17 CrunchChartTests=15 CrunchKeepTests=15 CrunchPivotTests=15 CrunchDrawingTests=11
CrunchTablesTests=10 CrunchFormatUiTests=9 CrunchOpsTests=7 CrunchFormulasTests=6"

# A program the build did not make (a CI part builds only the targets in tools/ci_targets.sh) stops everything here, by name.
missing=0
for A in "$@"; do N=${A%%:*}; [ -x "build/$N$ext" ] || { echo "ci_tests.sh: build/$N$ext was not built (is its target in tools/ci_targets.sh?)" >&2; missing=1; }; done
[ "$missing" -eq 0 ] || exit 2
jobs=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
split=${CI_SPLIT:-1}
LONG="everyCommandOnABlankDocument everyCommandOnADocumentFromAFile commandsNeverRun"

# The parts: one line each, "<part> <program> <shard or -> <arguments...>".
parts=$(mktemp)
for A in "$@"; do
    N=${A%%:*}; P=build/$N$ext
    if [ "$N" != "$A" ]; then
        # A named part: k of n.
        k=${A#*:}; n=${k#*/}; k=${k%/*}
        case $N in
        *checks)
            echo "$N.part$k $N $k/$n $LONG" >> "$parts"
            if [ "$k" -eq "$n" ]; then
                rest=$(QT_QPA_PLATFORM=offscreen "$P" -functions 2>/dev/null | sed 's/()$//' | grep -vxF "$(echo $LONG | tr ' ' '\n')" | grep -vxE 'initTestCase|cleanupTestCase' | tr '\n' ' ')
                echo "$N.rest $N - $rest" >> "$parts"
            fi ;;
        crunchtests|sheettests|jdtests|jptests)
            args=$(QT_QPA_PLATFORM=offscreen "$P" --classes 2>/dev/null | awk -v k="$k" -v n="$n" -v w="$(echo $WEIGHTS)" '
                BEGIN { m = split(w, a, /[ \n]+/); for (i = 1; i <= m; i++) { split(a[i], kv, "="); wt[kv[1]] = kv[2] } }
                NF { c[++t] = $0; v[t] = ($0 in wt) ? wt[$0] : 1 }
                END {
                    # (heaviest first, ties in the order of the list; each to the part with the least so far, the lower part number on a tie)
                    for (i = 1; i <= t; i++) { best = 0; for (j = 1; j <= t; j++) if (!done[j] && (best == 0 || v[j] > v[best])) best = j; done[best] = 1
                        p = 1; for (q = 2; q <= n; q++) if (sum[q] < sum[p]) p = q
                        sum[p] += v[best]; if (p == k) printf "--class %s ", c[best] }
                }')
            [ -n "$args" ] && echo "$N.part$k $N - $args" >> "$parts" ;;
        *) echo "ci_tests.sh: $N cannot run in parts" >&2; exit 2 ;;
        esac
        continue
    fi
    if [ "$split" -le 1 ]; then echo "$N $N -" >> "$parts"; continue; fi
    case $N in
    *checks)
        for k in $(seq 1 "$split"); do echo "$N.part$k $N $k/$split $LONG" >> "$parts"; done
        rest=$(QT_QPA_PLATFORM=offscreen "$P" -functions 2>/dev/null | sed 's/()$//' | grep -vxF "$(echo $LONG | tr ' ' '\n')" | grep -vxE 'initTestCase|cleanupTestCase' | tr '\n' ' ')
        echo "$N.rest $N - $rest" >> "$parts" ;;
    crunchtests|sheettests|jdtests|jptests)
        list=$(QT_QPA_PLATFORM=offscreen "$P" --classes 2>/dev/null)
        for k in $(seq 1 "$split"); do
            args=$(echo "$list" | awk -v k=$k -v n=$split 'NF && NR % n == k % n {printf "--class %s ", $0}')
            [ -n "$args" ] && echo "$N.part$k $N - $args" >> "$parts"
        done ;;
    writetests|flowtests)
        list=$(QT_QPA_PLATFORM=offscreen "$P" -functions 2>/dev/null | sed 's/()$//' | grep -vxE 'initTestCase|cleanupTestCase')
        for k in $(seq 1 "$split"); do
            args=$(echo "$list" | awk -v k=$k -v n=$split 'NF && NR % n == k % n {printf "%s ", $0}')
            [ -n "$args" ] && echo "$N.part$k $N - $args" >> "$parts"
        done ;;
    *) echo "$N $N -" >> "$parts" ;;
    esac
done

# (xargs -L takes a line that ends in a blank as going on to the next one)
sed -i 's/[[:space:]]*$//' "$parts"
# shellcheck disable=SC2016
xargs -P "$jobs" -L 1 bash -c 'id=$1; prog=$2; shard=$3; shift 3; started=$(date +%s)
    if [ "$shard" != "-" ]; then export JO_APPCHECK_SHARD=$shard; fi
    "build/$prog'"$ext"'" "$@" -v1 -o "$id-results.txt,txt" -o -,txt > "$id.log" 2>&1; echo $? > "$id.code"
    echo "$id finished in $(( $(date +%s) - started )) s"' _ < "$parts"

# (macOS has no timeout command; Homebrew's coreutils name it gtimeout; without either, the debugger runs unbounded under the step's own time limit)
T=$(command -v timeout || command -v gtimeout || true)

code=0
while read -r ID N SHARD ARGS; do
    P=build/$N$ext
    ncode=$(cat "$ID.code" 2>/dev/null || echo 1)
    echo "--- $ID-results.txt"; cat "$ID-results.txt" 2>/dev/null || echo "(no results file: the program did not start)"
    echo "$ID exit code: $ncode"
    if [ "$ncode" -ne 0 ]; then echo "--- $ID.log (the end)"; tail -60 "$ID.log"; fi
    # A crash: a nonzero exit with no failed test in the totals (QtTest exits with the number of failures; a program of several classes
    # prints a totals line for each class that finished, so a totals line alone doesn't say the program did). The stack of the first
    # crash is in the log; a crash can depend on the tests before it, so the part runs again as it did.
    failed=$(grep -oE 'Totals: [0-9]+ passed, [0-9]+ failed' "$ID-results.txt" 2>/dev/null | awk '{s += $4} END {print s + 0}')
    if [ "$ncode" -ne 0 ] && [ "$failed" -eq 0 ]; then
        echo "--- stack of the crash of $ID"
        if [ "$SHARD" != "-" ]; then export JO_APPCHECK_SHARD=$SHARD; else unset JO_APPCHECK_SHARD; fi
        # shellcheck disable=SC2086
        if [ "${RUNNER_OS:-}" = "macOS" ]; then
            $T ${T:+900} lldb --batch -o run -k "thread backtrace all -c 30" -- "$P" -silent $ARGS 2>&1 | tail -120
        else
            G=gdb
            if [ "${RUNNER_OS:-}" = "Windows" ]; then G="$(cygpath -u "$IQTA_TOOLS")/mingw1310_64/bin/gdb.exe"; fi
            $T ${T:+900} "$G" -q -batch -ex run -ex "thread apply all bt 30" --args "$P" -silent $ARGS 2>&1 | grep -v '^\[' | tail -120
        fi
    fi
    if [ "$code" -eq 0 ]; then code=$ncode; fi
done < "$parts"
rm -f "$parts"
exit $code
