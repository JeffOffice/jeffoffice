#pragma once
// JeffDeck's identity, for the shared libraries (core/appinfo.h).

#include "core/appinfo.h"

namespace jp {

inline AppInfo deckAppInfo()
{
    return {QStringLiteral("JeffDeck"), QStringLiteral("jeffdeck"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice")};
}

} // namespace jp
