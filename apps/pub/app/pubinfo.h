#pragma once
// JeffPub's identity, for the shared libraries (core/appinfo.h).

#include "core/appinfo.h"

namespace jp {

inline AppInfo pubAppInfo()
{
    return {QStringLiteral("JeffPub"), QStringLiteral("jeffpub"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffpub")};
}

} // namespace jp
