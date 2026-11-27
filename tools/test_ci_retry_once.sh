#!/bin/bash
# Test of tools/ci_retry_once.sh with fake test programs: fails once then passes, fails twice, crashes once, and stops at the time limit.
#     bash tools/test_ci_retry_once.sh        (prints "ci_retry_once tests: N ok, M failed"; exit 1 on a failure)
W=$(cd "$(dirname "$0")" && pwd)/ci_retry_once.sh
D=$(mktemp -d "${TMPDIR:-/tmp}/ciretryXXXXXX"); cd "$D" || exit 2
ok=0; bad=0
chk() { if [ "$2" = "$3" ]; then ok=$((ok + 1)); else bad=$((bad + 1)); echo "FAIL $1: got '$2', want '$3'"; fi; }
mk() { printf '#!/bin/bash\nn=$(cat cnt_%s 2>/dev/null || echo 0); echo $((n+1)) > cnt_%s\n%s\n' "$1" "$1" "$2" > "fake_$1"; chmod +x "fake_$1"; }
FAIL='echo "FAIL!  : FooTests::bar() Compared values are not the same"; echo "   Actual   (x): 1"; echo "   Loc: [a.cpp(3)]"; echo "Totals: 3 passed, 1 failed"; exit 1'
mk once 'if [ $n = 0 ]; then '"$FAIL"'; fi; echo "Totals: 4 passed, 0 failed"'
mk twice "$FAIL"
mk crash 'if [ $n = 0 ]; then echo "Received signal 11"; exit 139; fi; echo "Totals: 4 passed, 0 failed"'
mk hang 'if [ $n = 0 ]; then exit 97; fi; sleep 30'
run() { # run <case>: the first run as ci_tests.sh does it, then the retry
    ./fake_$1 -v1 > "$1.log" 2>&1; echo $? > "$1.code"; cp "$1.log" "$1-results.txt"
    JO_RETRY_TIMEOUT=2 GITHUB_STEP_SUMMARY="$PWD/sum_$1" bash "$W" "$1" "./fake_$1" - -v1 > "$1.out" 2>&1; echo $? > "$1.rc"
}
mk badtotals 'if [ $n = 0 ]; then '"$FAIL"'; fi; echo "Totals: 3 passed, 1 failed"; exit 0'
mk nototals 'if [ $n = 0 ]; then '"$FAIL"'; fi; echo ok; exit 0'
for c in once twice crash hang badtotals nototals; do run $c; done
# no time-limit command on the path: no retry
mkdir -p nobin; for t in bash cat cp tr grep awk sed head cut wc basename mktemp rm date; do ln -sf "$(command -v $t)" nobin/$t; done
mk nl "$FAIL"
./fake_nl -v1 > nolimit.log 2>&1; echo $? > nolimit.code; cp nolimit.log nolimit-results.txt
PATH="$PWD/nobin" /bin/bash "$W" nolimit ./fake_nl - > nolimit.out 2>&1; echo $? > nolimit.rc
chk "no timeout command: step fails, not retried" "$(cat nolimit.rc) $(cat cnt_nl)" "1 1"
chk "once: step passes" "$(cat once.rc)" 0
chk "once: RETRIED-PASSED line" "$(grep -c '^RETRIED-PASSED fake_once FooTests::bar | FAIL!' once.out)" 1
chk "once: summary lists it" "$(grep -c 'FooTests::bar' sum_once)" 1
chk "once: ran twice" "$(cat cnt_once)" 2
chk "twice: step fails" "$(cat twice.rc)" 1
chk "twice: no RETRIED-PASSED" "$(grep -c '^RETRIED-PASSED' twice.out)" 0
chk "twice: both runs shown" "$(grep -c 'FAIL!  : FooTests::bar' twice.out)" 2
chk "twice: never a third run" "$(cat cnt_twice)" 2
chk "crash: passes on retry" "$(cat crash.rc)" 0
chk "crash: listed" "$(grep -c '^RETRIED-PASSED fake_crash (no-test-named) | exit code 139' crash.out)" 1
chk "hang: fails at the limit" "$(cat hang.rc)" 124
chk "hang: no RETRIED-PASSED" "$(grep -c '^RETRIED-PASSED' hang.out)" 0
chk "exit 0 with a failure in its totals: fails" "$(cat badtotals.rc)" 1
chk "exit 0 with no totals line: fails" "$(cat nototals.rc)" 1
chk "no RETRIED-PASSED for either" "$(cat badtotals.out nototals.out | grep -c '^RETRIED-PASSED')" 0
echo "ci_retry_once tests: $ok ok, $bad failed"
cd / && rm -rf "$D"
[ "$bad" = 0 ]
