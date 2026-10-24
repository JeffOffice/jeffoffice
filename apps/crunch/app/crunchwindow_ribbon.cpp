// JeffCrunch's ribbon (its layout is resources/ribbon.json, built by ribbonbuilder.cpp): the commands it names that are on the window, and the controls built in code (none yet).

#include "app/crunchwindow.h"

#include "app/ribbon.h"
#include "app/ribbonbuilder.h"

#include <QAction>
#include <QFile>

namespace jp {

void CrunchWindow::buildRibbon()
{
    RibbonParts parts;
    parts.action = [this](const QString &id) { return m_actions.value(id); };
    ribbonParts(parts);
    QFile f(QStringLiteral(":/crunch/ribbon.json"));
    QByteArray json;
    if (f.open(QIODevice::ReadOnly)) json = f.readAll();
    jp::buildRibbon(m_ribbon, json, parts, this, &m_ribbonErrors);
}

void CrunchWindow::ribbonParts(RibbonParts &parts)
{
    // (the ribbon's commands are all actions of the window; the controls built in code are those of the Page Layout tab)
    pageRibbonParts(parts);
}

} // namespace jp
