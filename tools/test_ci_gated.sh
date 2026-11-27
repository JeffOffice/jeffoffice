#!/bin/bash
# Test of tools/ci_gated.sh with a throwaway repository and fake lists of passed trees.
#     bash tools/test_ci_gated.sh        (prints "ci_gated tests: N ok, M failed"; exit 1 on a failure)
G=$(cd "$(dirname "$0")" && pwd)/ci_gated.sh
D=$(mktemp -d "${TMPDIR:-/tmp}/cigatedXXXXXX"); cd "$D" || exit 2
ok=0; bad=0
chk() { if [ "$2" = "$3" ]; then ok=$((ok + 1)); else bad=$((bad + 1)); echo "FAIL $1: got '$2', want '$3'"; fi; }
git init -q repo && cd repo && echo a > a && git add a && git -c user.name=t -c user.email=t@t commit -qm one
T=$(git rev-parse 'HEAD^{tree}'); ME=$(bash "$G" --env)
run() { # run <list-file> [JO_LINUX_GATED]: output in out, step outputs in gho, summary in sum
    rm -f gho sum; : > gho; : > sum
    JO_LINUX_GATED=${2:-false} GITHUB_OUTPUT=$PWD/gho GITHUB_STEP_SUMMARY=$PWD/sum bash "$G" "$1" > out 2>&1
}
chk "environment name has system, os, compiler, packages" "$(echo "$ME" | grep -cE '^[a-z]+:[^:]+:gcc-[^:]+:pkgs-[0-9a-f]+$')" 1
echo "$T Nov 27 22:41 branch abc1234 env=$ME" > same; run same
chk "same environment: skip" "$(cat gho)" "skip=true"
chk "same environment: fixed log line" "$(grep -c "^TESTS-SKIPPED already-passed $T record in same from Nov 27 22:41 branch abc1234 env=" out)" 1
chk "same environment: summary first line" "$(head -1 sum | cut -c1-53)" "TESTS-SKIPPED already-passed $(echo $T | cut -c1-24)"
echo "$T Nov 27 22:41 branch abc1234 env=linux:debian-13:gcc-14.2.0:pkgs-00000000" > other; run other
chk "another environment: tests run" "$(cat gho)" "skip=false"
chk "another environment: says so" "$(grep -c 'another environment' out)" 1
chk "another environment: no skipped line, no summary" "$(grep -c TESTS-SKIPPED out)$(wc -c < sum | tr -d ' ')" "00"
echo "$T Nov 27 22:41 branch abc1234" > legacy; run legacy
chk "a record with no environment (the local gate's, before S96): tests run" "$(cat gho)" "skip=false"
echo "0000000000000000000000000000000000000000 Nov 27 22:41 branch abc1234 env=$ME" > othertree; run othertree
chk "another tree: tests run" "$(cat gho)" "skip=false"
run missing
chk "no list: tests run" "$(cat gho)" "skip=false"
run same true
chk "same record, linux_gated true: still skip by the record only" "$(cat gho)" "skip=true"
run legacy true
chk "linux_gated alone excuses nothing" "$(cat gho)$(grep -c ignored out)" "skip=false1"
echo "ci_gated tests: $ok ok, $bad failed"; cd /; rm -rf "$D"; [ "$bad" = 0 ]
