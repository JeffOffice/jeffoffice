#pragma once
// The program's version, from the project line in CMakeLists.txt. It lives in one generated file (version.cpp), so a version bump
// recompiles that file and relinks, and not every file in the build.

#include <QString>

namespace jp {

// "0.24.0"
const char *versionString();
QString versionText();

} // namespace jp
