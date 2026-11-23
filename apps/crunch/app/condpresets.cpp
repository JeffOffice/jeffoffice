#include "app/condpresets.h"

#include "grid/condformat.h"

#include <QCoreApplication>
#include <QLinearGradient>
#include <QPainter>

#include <algorithm>

namespace jp::condpresets {

using jp::office::sheet::CellRange;
using jp::office::sheet::ColorSpec;
using jp::office::sheet::CondRule;
using jp::office::sheet::CondValue;

namespace {

CondValue value(CondValue::Type type, const QString &v = QString())
{
    CondValue c;
    c.type = type;
    c.value = v;
    return c;
}

} // namespace

const QVector<BarColor> &barColors()
{
    static const QVector<BarColor> list = {{QT_TRANSLATE_NOOP("jp::CrunchWindow", "Blue"), 0x638EC6},   {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Green"), 0x63C384},
                                           {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red"), 0xFF555A},    {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Orange"), 0xFFB628},
                                           {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Blue"), 0x008AEF}, {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Purple"), 0xD6007B}};
    return list;
}

const QVector<ScalePreset> &scalePresets()
{
    constexpr quint32 red = 0xF8696B, yellow = 0xFFEB84, green = 0x63BE7B, white = 0xFCFCFF, blue = 0x5A8AC6;
    static const QVector<ScalePreset> list = {{QT_TRANSLATE_NOOP("jp::CrunchWindow", "Green - Yellow - Red Color Scale"), {red, yellow, green}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red - Yellow - Green Color Scale"), {green, yellow, red}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Green - White - Red Color Scale"), {red, white, green}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red - White - Green Color Scale"), {green, white, red}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Blue - White - Red Color Scale"), {red, white, blue}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red - White - Blue Color Scale"), {blue, white, red}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "White - Red Color Scale"), {white, red}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red - White Color Scale"), {red, white}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Green - White Color Scale"), {white, green}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "White - Green Color Scale"), {green, white}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Green - Yellow Color Scale"), {yellow, green}},
                                              {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Yellow - Green Color Scale"), {green, yellow}}};
    return list;
}

const QVector<IconGroup> &iconGroups()
{
    static const QVector<IconGroup> list = {
        {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Directional"), {"3Arrows", "3ArrowsGray", "3Triangles", "4Arrows", "4ArrowsGray", "5Arrows", "5ArrowsGray"}},
        {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Shapes"), {"3TrafficLights1", "3TrafficLights2", "3Signs", "4TrafficLights", "4RedToBlack"}},
        {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Indicators"), {"3Symbols", "3Symbols2", "3Flags"}},
        {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Ratings"), {"3Stars", "4Rating", "5Rating", "5Quarters", "5Boxes"}}};
    return list;
}

QString iconSetLabel(const QString &set)
{
    struct Label {
        const char *set, *text;
    };
    static const Label labels[] = {{"3Arrows", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Arrows (Colored)")},
                                   {"3ArrowsGray", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Arrows (Gray)")},
                                   {"3Triangles", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Triangles")},
                                   {"4Arrows", QT_TRANSLATE_NOOP("jp::CrunchWindow", "4 Arrows (Colored)")},
                                   {"4ArrowsGray", QT_TRANSLATE_NOOP("jp::CrunchWindow", "4 Arrows (Gray)")},
                                   {"5Arrows", QT_TRANSLATE_NOOP("jp::CrunchWindow", "5 Arrows (Colored)")},
                                   {"5ArrowsGray", QT_TRANSLATE_NOOP("jp::CrunchWindow", "5 Arrows (Gray)")},
                                   {"3TrafficLights1", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Traffic Lights (No Border)")},
                                   {"3TrafficLights2", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Traffic Lights (with Border)")},
                                   {"3Signs", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Signs")},
                                   {"4TrafficLights", QT_TRANSLATE_NOOP("jp::CrunchWindow", "4 Traffic Lights")},
                                   {"4RedToBlack", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red to Black")},
                                   {"3Symbols", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Symbols (in Circles)")},
                                   {"3Symbols2", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Symbols (Plain)")},
                                   {"3Flags", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Flags")},
                                   {"3Stars", QT_TRANSLATE_NOOP("jp::CrunchWindow", "3 Stars")},
                                   {"4Rating", QT_TRANSLATE_NOOP("jp::CrunchWindow", "4 Ratings")},
                                   {"5Rating", QT_TRANSLATE_NOOP("jp::CrunchWindow", "5 Ratings")},
                                   {"5Quarters", QT_TRANSLATE_NOOP("jp::CrunchWindow", "5 Quarters")},
                                   {"5Boxes", QT_TRANSLATE_NOOP("jp::CrunchWindow", "5 Boxes")}};
    for (const Label &l : labels)
        if (set == QLatin1String(l.set)) return QCoreApplication::translate("jp::CrunchWindow", l.text);
    return set;
}

CondRule dataBarRule(const CellRange &range, int color, bool gradient)
{
    const QVector<BarColor> &colors = barColors();
    CondRule r;
    r.type = CondRule::Type::DataBar;
    r.ranges.append(range);
    r.values = {value(CondValue::Type::Min), value(CondValue::Type::Max)};
    const ColorSpec fill = ColorSpec::fromRgb(colors[std::clamp(color, 0, int(colors.size()) - 1)].rgb);
    r.colors = {fill};
    r.gradient = gradient;
    r.border = !gradient;
    r.borderColor = fill;
    r.negativeColor = ColorSpec::fromRgb(0xFF0000);
    r.negativeBorderColor = ColorSpec::fromRgb(0xFF0000);
    return r;
}

CondRule colorScaleRule(const CellRange &range, int preset)
{
    const QVector<ScalePreset> &presets = scalePresets();
    const ScalePreset &p = presets[std::clamp(preset, 0, int(presets.size()) - 1)];
    CondRule r;
    r.type = CondRule::Type::ColorScale;
    r.ranges.append(range);
    if (p.colors.size() == 3) r.values = {value(CondValue::Type::Min), value(CondValue::Type::Percentile, QStringLiteral("50")), value(CondValue::Type::Max)};
    else r.values = {value(CondValue::Type::Min), value(CondValue::Type::Max)};
    for (const quint32 c : p.colors) r.colors.append(ColorSpec::fromRgb(c));
    return r;
}

CondRule iconSetRule(const CellRange &range, const QString &set)
{
    CondRule r;
    r.type = CondRule::Type::IconSet;
    r.ranges.append(range);
    r.iconSet = set;
    const int n = std::max(2, jp::grid::iconSetCount(set));
    for (int i = 0; i < n; ++i) r.values.append(value(CondValue::Type::Percent, QString::number(i * 100 / n)));
    return r;
}

const QVector<Period> &periods()
{
    static const QVector<Period> list = {{"yesterday", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Yesterday")}, {"today", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Today")},
                                         {"tomorrow", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Tomorrow")},   {"last7Days", QT_TRANSLATE_NOOP("jp::CrunchWindow", "In the last 7 days")},
                                         {"lastWeek", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Last week")},  {"thisWeek", QT_TRANSLATE_NOOP("jp::CrunchWindow", "This week")},
                                         {"nextWeek", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Next week")},  {"lastMonth", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Last month")},
                                         {"thisMonth", QT_TRANSLATE_NOOP("jp::CrunchWindow", "This month")}, {"nextMonth", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Next month")}};
    return list;
}

QString describe(const CondRule &r)
{
    using T = CondRule::Type;
    using O = CondRule::Operator;
    auto op = [&] {
        switch (r.op) {
        case O::LessThan: return QStringLiteral("<");
        case O::LessThanOrEqual: return QStringLiteral("<=");
        case O::Equal: return QStringLiteral("=");
        case O::NotEqual: return QStringLiteral("<>");
        case O::GreaterThanOrEqual: return QStringLiteral(">=");
        case O::GreaterThan: return QStringLiteral(">");
        case O::Between: return QCoreApplication::translate("jp::CrunchWindow", "between");
        case O::NotBetween: return QCoreApplication::translate("jp::CrunchWindow", "not between");
        }
        return QString();
    };
    switch (r.type) {
    case T::CellIs:
        if (r.op == O::Between || r.op == O::NotBetween) return QCoreApplication::translate("jp::CrunchWindow", "Cell value %1 %2 and %3").arg(op(), r.formula1, r.formula2);
        return QCoreApplication::translate("jp::CrunchWindow", "Cell value %1 %2").arg(op(), r.formula1);
    case T::Expression: return QCoreApplication::translate("jp::CrunchWindow", "Formula: %1").arg(r.formula1);
    case T::ColorScale: return r.colors.size() == 3 ? QCoreApplication::translate("jp::CrunchWindow", "Three-color scale") : QCoreApplication::translate("jp::CrunchWindow", "Two-color scale");
    case T::DataBar: return r.gradient ? QCoreApplication::translate("jp::CrunchWindow", "Data bar (gradient fill)") : QCoreApplication::translate("jp::CrunchWindow", "Data bar (solid fill)");
    case T::IconSet: return QCoreApplication::translate("jp::CrunchWindow", "Icon set: %1").arg(iconSetLabel(jp::grid::iconSetOrDefault(r.iconSet)));
    case T::Top10:
        return r.percent ? (r.bottom ? QCoreApplication::translate("jp::CrunchWindow", "Bottom %1 percent").arg(r.rank) : QCoreApplication::translate("jp::CrunchWindow", "Top %1 percent").arg(r.rank))
                         : (r.bottom ? QCoreApplication::translate("jp::CrunchWindow", "Bottom items: %1").arg(r.rank) : QCoreApplication::translate("jp::CrunchWindow", "Top items: %1").arg(r.rank));
    case T::UniqueValues: return QCoreApplication::translate("jp::CrunchWindow", "Unique values");
    case T::DuplicateValues: return QCoreApplication::translate("jp::CrunchWindow", "Duplicate values");
    case T::ContainsText: return QCoreApplication::translate("jp::CrunchWindow", "Text contains \"%1\"").arg(r.text);
    case T::NotContainsText: return QCoreApplication::translate("jp::CrunchWindow", "Text does not contain \"%1\"").arg(r.text);
    case T::BeginsWith: return QCoreApplication::translate("jp::CrunchWindow", "Text begins with \"%1\"").arg(r.text);
    case T::EndsWith: return QCoreApplication::translate("jp::CrunchWindow", "Text ends with \"%1\"").arg(r.text);
    case T::ContainsBlanks: return QCoreApplication::translate("jp::CrunchWindow", "Blank cells");
    case T::NotContainsBlanks: return QCoreApplication::translate("jp::CrunchWindow", "Cells that are not blank");
    case T::ContainsErrors: return QCoreApplication::translate("jp::CrunchWindow", "Cells with errors");
    case T::NotContainsErrors: return QCoreApplication::translate("jp::CrunchWindow", "Cells without errors");
    case T::TimePeriod:
        for (const Period &p : periods())
            if (r.timePeriod == QLatin1String(p.name)) return QCoreApplication::translate("jp::CrunchWindow", "Dates: %1").arg(QCoreApplication::translate("jp::CrunchWindow", p.text));
        return QCoreApplication::translate("jp::CrunchWindow", "Dates");
    case T::AboveAverage: return r.above ? QCoreApplication::translate("jp::CrunchWindow", "Above average") : QCoreApplication::translate("jp::CrunchWindow", "Below average");
    }
    return QString();
}

QPixmap barPixmap(quint32 rgb, bool gradient, int size)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const QColor c = QColor::fromRgb(rgb);
    const QRectF bar(size * 0.12, size * 0.28, size * 0.76, size * 0.44);
    if (gradient) {
        QLinearGradient g(bar.left(), 0, bar.right(), 0);
        g.setColorAt(0, c);
        g.setColorAt(1, QColor(255, 255, 255));
        p.fillRect(bar, g);
    } else {
        p.fillRect(bar, c);
        p.setPen(c.darker(115));
        p.drawRect(bar);
    }
    return pm;
}

QPixmap scalePixmap(const QVector<quint32> &colors, int size)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const QRectF box(size * 0.1, size * 0.2, size * 0.8, size * 0.6);
    QLinearGradient g(box.left(), 0, box.right(), 0);
    for (int i = 0; i < colors.size(); ++i) g.setColorAt(colors.size() == 1 ? 0 : double(i) / (colors.size() - 1), QColor::fromRgb(colors[i]));
    p.fillRect(box, g);
    return pm;
}

QPixmap iconSetStrip(const QString &set, int size)
{
    const int n = std::max(1, jp::grid::iconSetCount(set));
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    const double cell = double(size) / std::max(3, n);
    for (int i = 0; i < n; ++i) jp::grid::paintIconSetIcon(&p, QRectF(i * cell, (size - cell) / 2, cell, cell), set, n - 1 - i, n, false);
    return pm;
}

} // namespace jp::condpresets

namespace jp::condpresets {

namespace {
struct Look {
    const char *name;
    quint32 text, fill;   // 0 for none
};
const Look kLooks[] = {{QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Red Fill with Dark Red Text"), 0x9C0006, 0xFFC7CE}, {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Yellow Fill with Dark Yellow Text"), 0x9C5700, 0xFFEB9C},
                       {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Green Fill with Dark Green Text"), 0x006100, 0xC6EFCE}, {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Red Fill"), 0, 0xFFC7CE},
                       {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Red Text"), 0x9C0006, 0},                               {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Bold Dark Blue Text"), 0x1F3864, 0}};
} // namespace

int lookCount() { return int(std::size(kLooks)); }
QString lookName(int preset) { return QCoreApplication::translate("jp::CrunchWindow", kLooks[std::clamp(preset, 0, lookCount() - 1)].name); }

jp::office::sheet::DifferentialFormat lookOf(int preset)
{
    using jp::office::sheet::ColorSpec;
    const Look &p = kLooks[std::clamp(preset, 0, lookCount() - 1)];
    jp::office::sheet::DifferentialFormat d;
    if (p.text) d.fontColor = ColorSpec::fromRgb(p.text);
    if (p.fill) {
        d.hasFill = true;
        d.fill.pattern = jp::office::sheet::PatternKind::Solid;
        d.fill.bg = ColorSpec::fromRgb(p.fill);
    }
    if (preset == 5) d.bold = true;
    return d;
}

} // namespace jp::condpresets
