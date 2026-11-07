#!/bin/bash
# Prints the CMake targets one CI job builds (.github/workflows/build.yml, the Build step), one line, for `cmake --build build --target ...`,
# or with "run" the programs it runs (the arguments of tools/ci_tests.sh).
#
#     tools/ci_targets.sh <tests|checks> [Linux|Windows|macOS] [k/n] [run]
#
# A job builds only what its later steps use; everything they pull in on the way (a test's helper programs, the libraries) comes by
# the targets' own dependencies. A program a step needs that is not listed here fails that step (tools/ci_tests.sh stops at a missing
# test program; the packaging and the PDF check stop at a missing app), so a forgotten target fails loudly. Releases (a run started by
# hand with mac_both on) build everything instead.
#
# Without k/n, the list is the whole part (our own Linux machine runs each part in one job). Windows and macOS run each part in two
# jobs, 1/2 and 2/2, which together run every program once; the lists are balanced by the seconds each program took on the hosted
# machines (Nov 7 2026, Windows / macOS): the sums are in the comments.
#
# tests:  the test programs of the Test step (tools/ci_tests.sh). jptests pulls in jpcrashtest and jpfonttest, which it starts, and
#         crunchtests pulls in jeffcrunch, which it starts (add_dependencies in CMakeLists.txt). crunchtests is the longest program (530
#         s), so it runs in both jobs, half of its test classes each (crunchtests:k/2).
#           1/2: crunchtests:1/2 270, flowtests 214 / 268, jdtests 200 / 159, officetests 56 / 46, wmltests, odttests, writeodttests
#           2/2: crunchtests:2/2 265, writetests 249 / 189, sheettests 209 / 151, jptests 95 / 98, the small write tests
# checks: the four app-check programs of the App checks step; the four apps (the PDF check, the screenshots of the packages, the
#         installer, the .deb and AppImages, the disk image); jpubtool and jdecktool (the Windows developer-tools artifact).
#         The first job (1/2) builds the four apps and the tools and packages them, which takes it as long as the other job's checks
#         (4 apps cold: 750 s of building on Windows), so it runs only the shortest check program; the second job builds and runs the
#         other three, crunchchecks (680 s, the longest) in two parts side by side.
#           1/2: deckchecks 183 / 123
#           2/2: pubchecks 254 / 237, writechecks 308 / 291, crunchchecks:1/2 and crunchchecks:2/2 about 350 / 390 each
set -eu
part=${1:?usage: ci_targets.sh <tests|checks> [os] [k/n] [run]}
shard=${3:-}
mode=${4:-targets}

case "$part/$shard" in
tests/)
    runs="flowtests crunchtests jdtests writetests writereviewtests writeinfotests writehometests writeinserttests sheettests jptests officetests wmltests odttests writeodttests writerefstests writemailtests writefieldtests" ;;
tests/1/2)
    runs="crunchtests:1/2 flowtests jdtests officetests wmltests odttests writeodttests" ;;
tests/2/2)
    runs="crunchtests:2/2 writetests sheettests jptests writereviewtests writeinfotests writehometests writeinserttests writerefstests writemailtests writefieldtests" ;;
checks/)
    runs="crunchchecks writechecks pubchecks deckchecks"
    extra="jeffpub jeffdeck jeffwrite jeffcrunch jpubtool jdecktool" ;;
checks/1/2)
    runs="deckchecks"
    extra="jeffpub jeffdeck jeffwrite jeffcrunch jpubtool jdecktool" ;;
checks/2/2)
    runs="crunchchecks:1/2 crunchchecks:2/2 writechecks pubchecks"
    extra="" ;;
tests/*|checks/*) echo "ci_targets.sh: unknown shard '$shard'" >&2; exit 2 ;;
*) echo "ci_targets.sh: unknown part '$part'" >&2; exit 2 ;;
esac

case $mode in
run) echo $runs ;;
targets) for r in $runs; do echo "${r%%:*}"; done | awk '!seen[$0]++' | tr '\n' ' ' | sed "s/\$/${extra:-}/;s/ *\$//"; echo ;;
*) echo "ci_targets.sh: unknown mode '$mode'" >&2; exit 2 ;;
esac
