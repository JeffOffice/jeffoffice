#!/bin/bash
# For our own Linux runners: says whether this exact tree already passed the lead's local gate (tools/gate.sh, run by gatecheck.sh on the same
# machine, which records each passed tree in a file and mounts it read-only into the runner containers at /gated/trees). Prints skip=true or
# skip=false to the step's output file, and a line saying which, so the log shows why a Test step did not run.
#
#     tools/ci_gated.sh [list-file]        (default: $JO_GATED_TREES, else /gated/trees)
#
# A missing file, an unreadable one, or a tree not in it means skip=false: the tests run as they always did. The file's lines start with the
# git tree hash (of HEAD^{tree}, the files alone, so a rebase or an amended message does not matter), then a date and the commit.
set -u
f=${1:-${JO_GATED_TREES:-/gated/trees}}
out=${GITHUB_OUTPUT:-/dev/null}
tree=$(git rev-parse 'HEAD^{tree}' 2>/dev/null) || tree=
if [ -n "$tree" ] && [ -r "$f" ] && line=$(grep -m1 "^$tree " "$f" 2>/dev/null); then
    echo "skip=true" >> "$out"
    echo "tree $tree already passed the local gate ($(echo "$line" | cut -d' ' -f2-)): the Linux tests are skipped here"
else
    echo "skip=false" >> "$out"
    if [ ! -r "$f" ]; then echo "no gated-trees list at $f: the Linux tests run"
    else echo "tree ${tree:-unknown} is not in $f: the Linux tests run"; fi
fi
