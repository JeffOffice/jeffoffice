#pragma once
// JeffPub's identity, for the shared libraries (core/appinfo.h), and the
// settings and files it kept under an older name.

#include "core/appinfo.h"

namespace jp {

inline AppInfo pubAppInfo()
{
    return {QStringLiteral("JeffPub"), QStringLiteral("jeffpub"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice"),
            QColor(0x9E, 0x1F, 0x63), QColor(0xE0, 0x6A, 0xA5), QStringLiteral("JP"), QColor(0xC2, 0x4A, 0x8A), QColor(0xE0, 0x7A, 0xB0)};
}

// JeffPub was called JeffPub 79 through version 0.5.0, and kept its settings
// and files under that name. Brings them over the first time the renamed
// program starts, before anything reads its settings: settings it doesn't
// have yet (setup may already have written the statistics choice), and the
// folder of templates, building blocks, AutoRecover copies and downloads.
// The old settings stay behind.
void moveFromJeffPub79();

} // namespace jp
