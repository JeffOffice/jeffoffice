#include "core/cellstyles.h"

#include "sheet/builtinstyles.h"

#include <QCoreApplication>

namespace jp::crunch {

using namespace jp::office::sheet;

QString styleGroupName(int group)
{
    static const char *names[] = {QT_TRANSLATE_NOOP("jp::crunch::CellStyles", "Good, Bad and Neutral"), QT_TRANSLATE_NOOP("jp::crunch::CellStyles", "Data and Model"),
                                  QT_TRANSLATE_NOOP("jp::crunch::CellStyles", "Titles and Headings"), QT_TRANSLATE_NOOP("jp::crunch::CellStyles", "Themed Cell Styles"),
                                  QT_TRANSLATE_NOOP("jp::crunch::CellStyles", "Number Format"), QT_TRANSLATE_NOOP("jp::crunch::CellStyles", "Custom")};
    if (group < 0 || group > 5) return QString();
    return QCoreApplication::translate("jp::crunch::CellStyles", names[group]);
}

namespace {

StyleSample sampleOfFormat(const Styles &styles, const CellFormat &f, const QString &name, int group)
{
    StyleSample s;
    s.name = name;
    s.inFile = true;
    s.group = group;
    const FontSpec &font = styles.font(f.font);
    s.bold = font.bold;
    s.italic = font.italic;
    s.underline = font.underline != Underline::None;
    s.textRgb = font.color.isSet() ? styles.resolve(font.color, 0x000000) : 0x000000;
    const FillSpec &fill = styles.fill(f.fill);
    if (fill.pattern == PatternKind::Solid) {
        s.hasFill = true;
        s.fillRgb = styles.resolve(fill.fg, 0xFFFFFF);
    }
    const BorderSpec &b = styles.border(f.border);
    for (const BorderSide *side : {&b.left, &b.right, &b.top, &b.bottom})
        if (side->style != BorderStyle::None) {
            s.hasBorder = true;
            s.borderRgb = styles.resolve(side->color, 0x000000);
            break;
        }
    s.numFmtId = f.numFmtId;
    return s;
}

} // namespace

QVector<StyleSample> styleSamples(const Styles &styles)
{
    QVector<StyleSample> out;
    // the file's own (a name that is not a built-in one) first
    for (const CellStyleName &n : styles.cellStyles) {
        if (n.hidden || n.xfId >= quint32(styles.styleXfs.size())) continue;
        BuiltinStyle def;
        if (builtinStyle(n.name, &def)) continue;
        out.append(sampleOfFormat(styles, styles.styleXfs[int(n.xfId)], n.name, 5));
    }
    for (const QString &name : builtinStyleNames()) {
        BuiltinStyle def;
        if (!builtinStyle(name, &def)) continue;
        bool found = false;
        for (const CellStyleName &n : styles.cellStyles)
            if (!n.hidden && n.name.compare(name, Qt::CaseInsensitive) == 0 && n.xfId < quint32(styles.styleXfs.size())) {
                out.append(sampleOfFormat(styles, styles.styleXfs[int(n.xfId)], def.name, def.group));
                found = true;
                break;
            }
        if (found) continue;
        StyleSample s;
        s.name = def.name;
        s.group = def.group;
        s.bold = def.bold;
        s.italic = def.italic;
        s.underline = def.underline != Underline::None;
        s.textRgb = def.fontColor.isSet() ? styles.resolve(def.fontColor, 0x000000) : 0x000000;
        if (def.fill.pattern == PatternKind::Solid) {
            s.hasFill = true;
            s.fillRgb = styles.resolve(def.fill.fg, 0xFFFFFF);
        }
        for (const BorderSide *side : {&def.border.left, &def.border.right, &def.border.top, &def.border.bottom})
            if (side->style != BorderStyle::None) {
                s.hasBorder = true;
                s.borderRgb = styles.resolve(side->color, 0x000000);
                break;
            }
        s.numFmtId = def.numFmtId;
        out.append(s);
    }
    return out;
}

} // namespace jp::crunch
