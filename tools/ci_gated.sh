#!/bin/bash
# For our own Linux runners: says whether this exact tree already passed these Linux tests IN THIS SAME ENVIRONMENT (the runner image: its
# operating system, compiler, and package set). Prints skip=true or skip=false to the step's output file, and a line saying which, so the log
# shows why a Test step did not run. When it skips, the first line of the job summary and one fixed log line say so:
#
#     TESTS-SKIPPED already-passed <tree> <where the record came from>
#
#     tools/ci_gated.sh [list-file]        (default: $JO_GATED_TREES, else /gated/trees)
#     tools/ci_gated.sh --env              prints this machine's environment name and stops
#
# The list file's lines are: the git tree hash (of HEAD^{tree}, the files alone, so a rebase or an amended message does not matter), a date,
# a name, a commit, and ` env=<environment>` last. A record excuses the tests only when its env equals this machine's (S96, Nov 27 2026): the
# local gate on the lead's machine records env=host, another operating system and compiler than the runner image, and on Nov 27 its pass let
# the runner skip four pixel tests that failed in the image every time. A missing file, an unreadable one, a tree not in it, or a record made
# in another environment means skip=false: the tests run. The landing's linux_gated input is no longer honored for the same reason.
set -u
# (the environment name: system, operating system release, compiler, and a short hash of the installed packages; the same text wherever it is
# computed in the same image, so a script that runs this inside a container of the image writes records the job there recognizes)
envname() {
    local os cc pk
    os=$(. /etc/os-release 2>/dev/null; echo "${ID:-unknown}-${VERSION_ID:-0}")
    cc=$( (c++ --version 2>/dev/null || echo unknown) | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
    pk=$( (dpkg-query -W -f '${Package} ${Version}\n' 2>/dev/null | LC_ALL=C sort | sha1sum | cut -c1-8) || true)
    echo "$(uname -s | tr 'A-Z' 'a-z'):$os:gcc-${cc:-unknown}:pkgs-${pk:-none}"
}
if [ "${1:-}" = --env ]; then envname; exit 0; fi
f=${1:-${JO_GATED_TREES:-/gated/trees}}
out=${GITHUB_OUTPUT:-/dev/null}
sum=${GITHUB_STEP_SUMMARY:-/dev/null}
tree=$(git rev-parse 'HEAD^{tree}' 2>/dev/null) || tree=
me=$(envname)
if [ "${JO_LINUX_GATED:-false}" = true ]; then
    echo "the landing's linux_gated input is ignored (S96): the local gate runs on another environment than this one ($me)"
fi
if [ -n "$tree" ] && [ -r "$f" ] && line=$(grep -m1 "^$tree .* env=$me\$" "$f" 2>/dev/null); then
    echo "skip=true" >> "$out"
    where="record in $f from $(echo "$line" | cut -d' ' -f2- | sed 's/ env=.*//') env=$me"
    echo "TESTS-SKIPPED already-passed $tree $where"
    { echo "TESTS-SKIPPED already-passed $tree $where"; echo; echo "The Linux tests and checks of this job did not run: this exact tree passed them before, in this environment."; echo; } >> "$sum"
    echo "::notice title=TESTS-SKIPPED::already-passed $tree $where"
else
    echo "skip=false" >> "$out"
    other=""
    if [ -n "$tree" ] && [ -r "$f" ] && o=$(grep -m1 "^$tree " "$f" 2>/dev/null); then other=" (a record of it is from another environment: $(echo "$o" | sed -n 's/.* env=//p;t;s/.*/none/p'), not $me)"; fi
    if [ ! -r "$f" ]; then echo "no list of passed trees at $f: the Linux tests run"
    else echo "tree ${tree:-unknown} has no record from this environment in $f: the Linux tests run$other"; fi
fi
