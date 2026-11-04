#include "core/designops.h"

#include "core/styleops.h"
#include "wml/wml.h"

#include <QCoreApplication>
#include <QTextCursor>

using namespace jp::office::wml;

namespace jp::write {

const QVector<SpacingPreset> &spacingPresets()
{
    static const QVector<SpacingPreset> presets = {
        {QStringLiteral("default"), 0, 160, 259}, {QStringLiteral("none"), 0, 0, 240},  {QStringLiteral("compact"), 0, 80, 240}, {QStringLiteral("tight"), 0, 120, 264},
        {QStringLiteral("open"), 0, 200, 276},    {QStringLiteral("relaxed"), 0, 120, 360}, {QStringLiteral("double"), 0, 160, 480}};
    return presets;
}

QString spacingPresetName(const QString &id)
{
    if (id == QLatin1String("default")) return QCoreApplication::translate("jp::WriteWindow", "Default");
    if (id == QLatin1String("none")) return QCoreApplication::translate("jp::WriteWindow", "No Paragraph Space");
    if (id == QLatin1String("compact")) return QCoreApplication::translate("jp::WriteWindow", "Compact");
    if (id == QLatin1String("tight")) return QCoreApplication::translate("jp::WriteWindow", "Tight");
    if (id == QLatin1String("open")) return QCoreApplication::translate("jp::WriteWindow", "Open");
    if (id == QLatin1String("relaxed")) return QCoreApplication::translate("jp::WriteWindow", "Relaxed");
    if (id == QLatin1String("double")) return QCoreApplication::translate("jp::WriteWindow", "Double");
    return id;
}

bool applyParagraphSpacing(WriteDoc &doc, const QString &presetId)
{
    const SpacingPreset *preset = nullptr;
    for (const SpacingPreset &p : spacingPresets())
        if (p.id == presetId) preset = &p;
    if (!preset) return false;
    const QString id = doc.defaultParagraphStyle();
    std::optional<Style> found = styleOfDocument(doc, id);
    if (!found) return false;
    Style s = *found;
    s.pPr.before = preset->before;
    s.pPr.after = preset->after;
    s.pPr.beforeAuto.reset();
    s.pPr.afterAuto.reset();
    s.pPr.line = preset->line;
    s.pPr.lineRule = QStringLiteral("auto");
    QTextCursor c(doc.story());
    return modifyStyle(doc, c, id, s);
}

QString paragraphSpacingOf(const WriteDoc &doc)
{
    const ParaProps p = doc.styleSheet().paragraph(doc.defaultParagraphStyle());   // (with what the styles under it and the document's defaults give)
    for (const SpacingPreset &preset : spacingPresets())
        if (p.before.value_or(0) == preset.before && p.after.value_or(0) == preset.after && p.line.value_or(240) == preset.line &&
            (p.lineRule.isEmpty() || p.lineRule == QLatin1String("auto")))
            return preset.id;
    return QString();
}

} // namespace jp::write
