#pragma once
// Words written for JeffPub that name the program ("JeffPub Options", "JeffPub
// sample text"): they are translated that way, and the program that shows
// them puts its own name where JeffPub's was.

#include "core/appinfo.h"

#include <QString>

namespace jp {

inline QString forThisApp(const QString &text)
{
    const QString &name = appInfo().name;
    return name.isEmpty() ? text : QString(text).replace(QStringLiteral("JeffPub"), name);
}

} // namespace jp
