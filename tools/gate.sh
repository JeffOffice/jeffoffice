#!/bin/bash
# The merge gate, faster: the same builds, checks, and suites as the lead's gate, with the app-check suites split into parts that run side by side.
# Usage: tools/gate.sh <worktree> <log>        (JO_JOBS = compile jobs, JO_GATE_JOBS = suites running at once, JO_GATE_SHARDS = parts of each app-check suite)
# Each app-check suite (the programs named *checks) runs its three long tests in JO_GATE_SHARDS parts (JO_APPCHECK_SHARD=k/n, see tests/appchecks/appchecks.cpp);
# one more run does the rest of its tests. Every part checks its own commands, and all together check the same ones as one run does.
set -u
W=${1:?worktree}; L=${2:?log}; : > "$L"
cd "$W" || exit 1
SHARDS=${JO_GATE_SHARDS:-4}; PAR=${JO_GATE_JOBS:-14}
LONG="everyCommandOnABlankDocument everyCommandOnADocumentFromAFile commandsNeverRun"
# Longest first, so the last ones to start are the short ones.
SUITES="crunchchecks writechecks crunchtests writetests pubchecks deckchecks sheettests flowtests jdtests jptests odttests officetests wmltests writefieldtests writefiletests writehometests writeinfotests writeinserttests writemailtests writeodttests writerefstests writereviewtests writetabs2tests writedrawtests"
# (only the suites this tree has: a suite a later branch adds is skipped on trees without it)
HAVE=$(grep -rhoE 'add_executable\((\w+)' --include=CMakeLists.txt . | sed 's/add_executable(//' | sort -u)
SUITES=$(for s in $SUITES; do case $s in *checks) echo $s;; *) echo "$HAVE" | grep -qx "$s" && echo $s;; esac; done | tr '\n' ' ')
for c in check_layers check_translate_calls check_plain_text check_third_party; do echo "== $c: $(python3 tools/$c.py 2>&1 | tail -1)" >> "$L"; done
echo "== hex escapes: $(git diff origin/main...HEAD 2>/dev/null | grep -cE '^\+.*\\x[0-9A-Fa-f]{2}[0-9A-Fa-f]')" >> "$L"
if ! JO_JOBS=${JO_JOBS:-12} sh /home/user/code/jeffpub79/.claude/agents-kit/wtbuild.sh $SUITES > "$L.build" 2>&1; then
  tail -15 "$L.build" >> "$L"; echo "BUILD FAILED" >> "$L"; exit 1
fi
cd build || exit 1
T=$(mktemp -d)
run() {  # run <log> <suite> <shard or ""> [functions...]
  local log=$1 s=$2 shard=$3; shift 3
  mkdir -p "$T/$log/.config"
  ( [ -n "$shard" ] && export JO_APPCHECK_SHARD=$shard
    HOME=$T/$log XDG_CONFIG_HOME=$T/$log/.config QT_QPA_PLATFORM=offscreen JP_NO_TELEMETRY=1 timeout 2400 systemd-run --user --scope -p MemoryMax=12G -q ./$s "$@" > "$L.$log" 2>&1 ) &
}
wait_slot() { while [ "$(jobs -rp | wc -l)" -ge "$PAR" ]; do sleep 1; done; }
PARTS=""
for s in $SUITES; do
  case $s in
    *checks)
      for k in $(seq 1 "$SHARDS"); do wait_slot; run "$s.part$k" "$s" "$k/$SHARDS" $LONG; done
      REST=$(QT_QPA_PLATFORM=offscreen ./$s -functions 2>/dev/null | sed 's/()$//' | grep -vxF "$(echo $LONG | tr ' ' '\n')" | tr '\n' ' ')
      wait_slot; run "$s.rest" "$s" "" $REST
      PARTS="$PARTS $s" ;;
    *) wait_slot; run "$s" "$s" "" ;;
  esac
done
wait
for s in $SUITES; do
  case $s in
    *checks) fs=$(ls "$L.$s".part* "$L.$s.rest"); n=$(echo $fs | wc -w); extra=$((2 * (n - 1))) ;;
    *) fs="$L.$s"; extra=0 ;;
  esac
  echo "== $s: $(grep -hE '^FAIL' $fs | head -3 | tr '\n' ' ')$(cat $fs | awk -v x=$extra '/^Totals/{p+=$2;f+=$4;c++} END{printf "%d classes, %d passed, %d failed", c/(x/2+1),p-x,f}')" >> "$L"
done
rm -rf "${T:?}"
echo DONE >> "$L"
