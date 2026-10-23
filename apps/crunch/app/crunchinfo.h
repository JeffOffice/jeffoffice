#pragma once
// JeffCrunch's identity, for the shared libraries (core/appinfo.h).

#include "core/appinfo.h"

#include <QKeySequence>

namespace jp {

inline AppInfo crunchAppInfo()
{
    return {QStringLiteral("JeffCrunch"), QStringLiteral("jeffcrunch"), QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice/jeffoffice"),
            QColor(0x13, 0x77, 0x3A), QColor(0x4C, 0xB8, 0x78), QStringLiteral("JC"), QColor(0x1C, 0x80, 0x43), QColor(0x8B, 0xD6, 0xA6), QStringLiteral("crunch"), true};
}

namespace crunch {

// The keys of Show Formulas: Ctrl and the grave accent. On a Mac, Command and that key is the system's own (it moves between the windows of a program), so the key that is
// called Control there, which Qt calls Meta, is the one that is used.
inline QKeySequence showFormulasKey(bool mac)
{
    return mac ? QKeySequence(Qt::META | Qt::Key_QuoteLeft) : QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft);
}
inline QKeySequence showFormulasKey()
{
#ifdef Q_OS_MACOS
    return showFormulasKey(true);
#else
    return showFormulasKey(false);
#endif
}

} // namespace crunch

} // namespace jp
