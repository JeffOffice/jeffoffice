#pragma once
// JeffWrite's identity, for the shared libraries (core/appinfo.h).

#include "core/appinfo.h"

namespace jp {

inline AppInfo writeAppInfo()
{
    return {QStringLiteral("JeffWrite"), QStringLiteral("jeffwrite"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice"),
            QColor(0x1D, 0x55, 0xC4), QColor(0x6E, 0x9B, 0xF0), QStringLiteral("JW"), QColor(0x2C, 0x66, 0xD6), QColor(0x8F, 0xB3, 0xF5), QStringLiteral("write"), true};
}

} // namespace jp
