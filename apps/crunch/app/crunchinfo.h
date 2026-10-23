#pragma once
// JeffCrunch's identity, for the shared libraries (core/appinfo.h).

#include "core/appinfo.h"

namespace jp {

inline AppInfo crunchAppInfo()
{
    return {QStringLiteral("JeffCrunch"), QStringLiteral("jeffcrunch"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice"),
            QColor(0x13, 0x77, 0x3A), QColor(0x4C, 0xB8, 0x78), QStringLiteral("JC"), QColor(0x1C, 0x80, 0x43), QColor(0x8B, 0xD6, 0xA6), QStringLiteral("crunch"), true};
}

} // namespace jp
