#!/bin/bash
# Prints the CMake targets one CI part builds (.github/workflows/build.yml, the Build step), one line, for `cmake --build build --target ...`.
#
#     tools/ci_targets.sh <tests|checks> [Linux|Windows|macOS]
#
# A part builds only what its later steps use; everything they pull in on the way (a test's helper programs, the libraries) comes by
# the targets' own dependencies. A program a step needs that is not listed here fails that step (tools/ci_tests.sh stops at a missing
# test program; the packaging and the PDF check stop at a missing app), so a forgotten target fails loudly. Releases (a run started by
# hand with mac_both on) build everything instead.
#
# tests:  the test programs of the Test step (tools/ci_tests.sh). jptests pulls in jpcrashtest and jpfonttest, which it starts, and
#         crunchtests pulls in jeffcrunch, which it starts (add_dependencies in CMakeLists.txt).
# checks: the four app-check programs of the App checks step; the four apps (the PDF check, the screenshots of the packages, the
#         installer, the .deb and AppImages, the disk image); jpubtool and jdecktool (the Windows developer-tools artifact).
set -eu
part=${1:?usage: ci_targets.sh <tests|checks> [os]}
case $part in
tests)
    echo flowtests crunchtests jdtests writetests writereviewtests writeinfotests writehometests writeinserttests sheettests jptests officetests wmltests odttests writeodttests writerefstests writemailtests writefieldtests ;;
checks)
    echo crunchchecks writechecks pubchecks deckchecks jeffpub jeffdeck jeffwrite jeffcrunch jpubtool jdecktool ;;
*) echo "ci_targets.sh: unknown part '$part'" >&2; exit 2 ;;
esac
