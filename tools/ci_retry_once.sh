#!/bin/bash
# Runs a test program that has just failed ONE more time (the macOS test job only; tools/ci_tests.sh calls this when JO_RETRY_ONCE=1).
#     tools/ci_retry_once.sh <part id> <program path> <shard or -> [program arguments...]
# Needs what the first run left in the current folder: <id>-results.txt, <id>.log. Runs the program again the same way (same arguments and
# environment, a limit of JO_RETRY_TIMEOUT seconds, default 1500, where the system has timeout or gtimeout), then:
#   passed the second time: exit 0, <id>.code holds 0, and one line per failure of the first run is printed in the form
#       RETRIED-PASSED <program> <Class::function> | <first failure line>
#     (and written to the job summary, $GITHUB_STEP_SUMMARY), with the first run's failure text (at most 200 lines) printed;
#   failed again (a failed test, a crash, or the time limit): both runs' failures are printed, <id>.code holds the second code, and the exit
#     status is that code. Never a third run.
# A crash or a stop with no failed test in the first run is listed as "(no-test-named)" with its exit code and the log's last line.
# <id>-results.first.txt and <id>.first.log keep the first run. Written for bash 3.2 (macOS).
id=$1; prog=$2; shard=$3; shift 3
name=$(basename "$prog"); name=${name%.exe}
first_code=$(cat "$id.code" 2>/dev/null || echo 1)
cp "$id-results.txt" "$id-results.first.txt" 2>/dev/null || : > "$id-results.first.txt"
cp "$id.log" "$id.first.log" 2>/dev/null || : > "$id.first.log"
src="$id-results.first.txt"; grep -aq '^FAIL!' "$src" || src="$id.first.log"
failed=$(tr -d '\r' < "$src" | awk '/^FAIL! +: [A-Za-z0-9_]+::[A-Za-z0-9_]+/ {
    if (line != "") print line
    t = $0; sub(/^FAIL! +: /, "", t); n = t; sub(/[( ].*$/, "", n); line = n "\t" $0; waiting = 1; next }
    waiting && /^ +[^ ]/ { s = $0; gsub(/^ +/, "", s); line = line " / " s; print line; line = ""; waiting = 0; next }
    END { if (line != "") print line }' | awk -F'\t' '!seen[$1]++' | cut -c1-400)
if [ -z "$failed" ]; then
    last=$(tr -d '\r' < "$id.first.log" | grep -av '^[[:space:]]*$' | tail -1 | cut -c1-150)
    failed="(no-test-named)	exit code $first_code with no failed test (crash, hang, or time limit); the log ends: $last"
fi
echo "=== $id failed (exit code $first_code); running $name once more ==="
echo "--- first run's failures (at most 200 lines)"
tr -d '\r' < "$src" | grep -a -A3 '^FAIL!' | head -200
T=$(command -v timeout || command -v gtimeout || true)
if [ "$shard" != "-" ]; then export JO_APPCHECK_SHARD=$shard; else unset JO_APPCHECK_SHARD; fi
export JP_CLASS_TIMES=1
rm -f "$id.done"; export JP_DONE_FILE="$PWD/$id.done"
rm -f "$id-results.txt"
# shellcheck disable=SC2086
$T ${T:+-k 10 ${JO_RETRY_TIMEOUT:-1500}} "$prog" "$@" -v1 -o "$id-results.txt,txt" -o -,txt > "$id.log" 2>&1
code=$?
echo $code > "$id.code"
if [ "$code" -eq 0 ]; then
    echo "$failed" | while IFS= read -r l; do
        t=${l%%	*}; f=${l#*	}
        echo "RETRIED-PASSED $name $t | $f"
    done
    if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
        { echo "### Passed only on the second run: $name"; echo
          echo "$failed" | while IFS= read -r l; do echo "- \`${l%%	*}\`: ${l#*	}" | cut -c1-300; done; echo; } >> "$GITHUB_STEP_SUMMARY"
    fi
    exit 0
fi
echo "=== $id failed again (exit code $code; first run $first_code): the step fails ==="
echo "--- second run's failures (at most 200 lines)"
src2="$id-results.txt"; [ -s "$src2" ] || src2="$id.log"
tr -d '\r' < "$src2" | grep -a -A3 '^FAIL!' | head -200
if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
    { echo "### Failed twice: $name ($id)"; echo
      echo "$failed" | while IFS= read -r l; do echo "- first run: \`${l%%	*}\`"; done; echo; } >> "$GITHUB_STEP_SUMMARY"
fi
exit "$code"
