#include "core/appinfo.h"

namespace jp {

namespace {
AppInfo g_info;
}

const AppInfo &appInfo() { return g_info; }
void setAppInfo(const AppInfo &info) { g_info = info; }

} // namespace jp
