#!/bin/bash
# The merge gate, faster: the same builds, checks, and suites as the lead's gate, with the app-check suites split into parts that run side by side.
# Usage: tools/gate.sh <worktree> <log>        (JO_JOBS = compile jobs, JO_GATE_JOBS = suites running at once, JO_GATE_SHARDS = parts of each app-check suite)
# The biggest other suites run in JO_GATE_SPLIT parts too (their test classes, or for the two single-class ones their test functions).
# (the split of the other suites is by the times in tools/gate_times.json, tools/gate_plan.py; JO_GATE_XML=1 makes every part write QtTest XML, and tools/gate_times.py <log> --write makes that file)
# Each app-check suite (the programs named *checks) runs its three long tests in JO_GATE_SHARDS parts (JO_APPCHECK_SHARD=k/n, see tests/appchecks/appchecks.cpp);
# one more run does the rest of its tests. Every part checks its own commands, and all together check the same ones as one run does.
set -u
W=${1:?worktree}; L=${2:?log}; : > "$L"
cd "$W" || exit 1
SHARDS=${JO_GATE_SHARDS:-4}; PAR=${JO_GATE_JOBS:-16}
LONG="everyCommandOnABlankDocument everyCommandOnADocumentFromAFile commandsNeverRun"
# Longest first, so the last ones to start are the short ones.
SUITES="crunchchecks writechecks crunchtests writetests officetests pubchecks deckchecks sheettests flowtests jdtests jptests odttests wmltests writefieldtests writefiletests writehometests writeinfotests writeinserttests writemailtests writeodttests writerefstests writereviewtests writetabs2tests writedrawtests"
# (only the suites this tree has: a suite a later branch adds is skipped on trees without it)
HAVE=$(grep -rhoE 'add_executable\((\w+)' --include=CMakeLists.txt . | sed 's/add_executable(//' | sort -u)
SUITES=$(for s in $SUITES; do case $s in *checks) echo $s;; *) echo "$HAVE" | grep -qx "$s" && echo $s;; esac; done | tr '\n' ' ')
for c in check_layers check_translate_calls check_plurals check_plain_text check_third_party check_deb_copyright check_feature_counts check_ffmpeg_components; do echo "== $c: $(python3 tools/$c.py 2>&1 | tail -1)" >> "$L"; done
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
    # (output capped: a crashing test can make QtTest's crash handler print "Received signal" without end; one filled the disk on Nov 8)
    HOME=$T/$log XDG_CONFIG_HOME=$T/$log/.config QT_QPA_PLATFORM=offscreen JP_NO_TELEMETRY=1 timeout 2400 systemd-run --user --scope -p MemoryMax=12G -q ./$s ${JO_GATE_XML:+-o "$L.$log.xml,xml" -o "-,txt"} "$@" 2>&1 | head -c 200M > "$L.$log" ) &
}
wait_slot() { while [ "$(jobs -rp | wc -l)" -ge "$PAR" ]; do sleep 1; done; }
SPLIT=${JO_GATE_SPLIT:-4}
# Suites that run their test classes in JO_GATE_SPLIT parts (--class, round robin over the sorted list), and suites of one big class that run their test functions in parts.
CLASS_SPLIT="crunchtests sheettests jdtests jptests"
FUNC_SPLIT="writetests flowtests"
for s in $SUITES; do
  case $s in
    *checks)
      for k in $(seq 1 "$SHARDS"); do wait_slot; run "$s.part$k" "$s" "$k/$SHARDS" $LONG; done
      REST=$(QT_QPA_PLATFORM=offscreen ./$s -functions 2>/dev/null | sed 's/()$//' | grep -vxF "$(echo $LONG | tr ' ' '\n')" | tr '\n' ' ')
      wait_slot; run "$s.rest" "$s" "" $REST ;;
    *)
      if echo " $CLASS_SPLIT " | grep -q " $s "; then
        CL=$(QT_QPA_PLATFORM=offscreen ./$s --classes 2>/dev/null)
        for k in $(seq 1 "$SPLIT"); do
          ARGS=$(python3 "$W/tools/gate_plan.py" "$s" classes "$SPLIT" "$k" $CL | awk '{printf "--class %s ", $0}')
          [ -n "$ARGS" ] && { wait_slot; run "$s.part$k" "$s" "" $ARGS; }
        done
      elif echo " $FUNC_SPLIT " | grep -q " $s "; then
        FL=$(QT_QPA_PLATFORM=offscreen ./$s -functions 2>/dev/null | sed 's/()$//' | grep -vxE 'initTestCase|cleanupTestCase')
        for k in $(seq 1 "$SPLIT"); do
          ARGS=$(python3 "$W/tools/gate_plan.py" "$s" functions "$SPLIT" "$k" $FL | tr '\n' ' ')
          [ -n "$ARGS" ] && { wait_slot; run "$s.part$k" "$s" "" $ARGS; }
        done
      else
        wait_slot; run "$s" "$s" ""
      fi ;;
  esac
done
wait
for s in $SUITES; do
  fs=$(ls "$L.$s" "$L.$s".part* "$L.$s.rest" 2>/dev/null); n=1; extra=0
  case $s in
    *checks) n=$(echo $fs | wc -w); extra=$((2 * (n - 1) + $(echo $LONG | wc -w) * (SHARDS - 1))) ;;
    *) if echo " $FUNC_SPLIT " | grep -q " $s "; then n=$(echo $fs | wc -w); extra=$((2 * (n - 1))); fi ;;
  esac
  echo "== $s: $(grep -hE '^FAIL' $fs | head -3 | tr '\n' ' ')$(cat $fs | awk -v x=$extra -v n=$n '/^Totals/{p+=$2;f+=$4;c++} END{printf "%d classes, %d passed, %d failed", c/n,p-x,f}')" >> "$L"
done
rm -rf "${T:?}"
# (Linux: the build installed into a temporary prefix: every library resolves there, and each app starts and finds Help and its translations)
echo "== $(bash "$W/tools/check_packaged_layout.sh" "$W/build" 2>&1 | tail -4 | tr '\n' ' ')" >> "$L"
echo DONE >> "$L"
