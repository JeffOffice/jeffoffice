#pragma once
// JeffDeck's identity, for the shared libraries (core/appinfo.h).

#include "core/appinfo.h"

namespace jp {

inline AppInfo deckAppInfo()
{
    return {QStringLiteral("JeffDeck"), QStringLiteral("jeffdeck"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice"),
            QColor(0xB6, 0x42, 0x0F), QColor(0xF0, 0x8A, 0x4B), QStringLiteral("JD"), QColor(0xC4, 0x50, 0x1A), QColor(0xF4, 0xA0, 0x6A), QStringLiteral("deck"), true};
}

} // namespace jp
