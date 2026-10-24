#include "sheet/builtinstyles.h"

#include <QHash>

namespace jp::office::sheet {

namespace {

ColorSpec rgb(quint32 v) { return ColorSpec::fromRgb(v); }

ColorSpec theme(int index, double tint = 0)
{
    ColorSpec c;
    c.kind = ColorSpec::Kind::Theme;
    c.index = index;
    c.tint = tint;
    return c;
}

FillSpec solid(const ColorSpec &c)
{
    FillSpec f;
    f.pattern = PatternKind::Solid;
    f.fg = c;
    f.bg.kind = ColorSpec::Kind::Indexed;
    f.bg.index = 64;
    return f;
}

BorderSide line(BorderStyle style, const ColorSpec &color)
{
    BorderSide s;
    s.style = style;
    s.color = color;
    return s;
}

BorderSpec box(BorderStyle style, const ColorSpec &color)
{
    BorderSpec b;
    b.left = b.right = b.top = b.bottom = line(style, color);
    return b;
}

QVector<BuiltinStyle> makeAll()
{
    QVector<BuiltinStyle> all;
    auto add = [&](const char *name, int id, int group) -> BuiltinStyle & {
        BuiltinStyle s;
        s.name = QString::fromLatin1(name);
        s.builtinId = id;
        s.group = group;
        all.append(s);
        return all.last();
    };
    // good, bad, and neutral
    add("Normal", 0, 0);
    {
        BuiltinStyle &s = add("Good", 26, 0);
        s.fontColor = rgb(0x006100);
        s.fill = solid(rgb(0xC6EFCE));
    }
    {
        BuiltinStyle &s = add("Bad", 27, 0);
        s.fontColor = rgb(0x9C0006);
        s.fill = solid(rgb(0xFFC7CE));
    }
    {
        BuiltinStyle &s = add("Neutral", 28, 0);
        s.fontColor = rgb(0x9C5700);
        s.fill = solid(rgb(0xFFEB9C));
    }
    // data and model
    {
        BuiltinStyle &s = add("Calculation", 22, 1);
        s.bold = true;
        s.fontColor = rgb(0xFA7D00);
        s.fill = solid(rgb(0xF2F2F2));
        s.border = box(BorderStyle::Thin, rgb(0x7F7F7F));
    }
    {
        BuiltinStyle &s = add("Check Cell", 23, 1);
        s.bold = true;
        s.fontColor = rgb(0xFFFFFF);
        s.fill = solid(rgb(0xA5A5A5));
        s.border = box(BorderStyle::Double, rgb(0x3F3F3F));
    }
    {
        BuiltinStyle &s = add("Explanatory Text", 53, 1);
        s.italic = true;
        s.fontColor = rgb(0x7F7F7F);
    }
    {
        BuiltinStyle &s = add("Input", 20, 1);
        s.fontColor = rgb(0x3F3F76);
        s.fill = solid(rgb(0xFFCC99));
        s.border = box(BorderStyle::Thin, rgb(0x7F7F7F));
    }
    {
        BuiltinStyle &s = add("Linked Cell", 24, 1);
        s.fontColor = rgb(0xFA7D00);
        s.border.bottom = line(BorderStyle::Double, rgb(0xFF8001));
    }
    {
        BuiltinStyle &s = add("Note", 10, 1);
        s.fill = solid(rgb(0xFFFFCC));
        s.border = box(BorderStyle::Thin, rgb(0xB2B2B2));
    }
    {
        BuiltinStyle &s = add("Output", 21, 1);
        s.bold = true;
        s.fontColor = rgb(0x3F3F3F);
        s.fill = solid(rgb(0xF2F2F2));
        s.border = box(BorderStyle::Thin, rgb(0x3F3F3F));
    }
    {
        BuiltinStyle &s = add("Warning Text", 11, 1);
        s.fontColor = rgb(0xFF0000);
    }
    // titles and headings
    {
        BuiltinStyle &s = add("Title", 15, 2);
        s.size = 18;
        s.fontName = QStringLiteral("Calibri Light");
        s.scheme = QStringLiteral("major");
        s.fontColor = theme(3);
    }
    {
        BuiltinStyle &s = add("Heading 1", 16, 2);
        s.bold = true;
        s.size = 15;
        s.fontColor = theme(3);
        s.border.bottom = line(BorderStyle::Thick, theme(4));
    }
    {
        BuiltinStyle &s = add("Heading 2", 17, 2);
        s.bold = true;
        s.size = 13;
        s.fontColor = theme(3);
        s.border.bottom = line(BorderStyle::Thick, theme(4, 0.3999755851924192));
    }
    {
        BuiltinStyle &s = add("Heading 3", 18, 2);
        s.bold = true;
        s.fontColor = theme(3);
        s.border.bottom = line(BorderStyle::Medium, theme(4, 0.3999755851924192));
    }
    {
        BuiltinStyle &s = add("Heading 4", 19, 2);
        s.bold = true;
        s.fontColor = theme(3);
    }
    {
        BuiltinStyle &s = add("Total", 25, 2);
        s.bold = true;
        s.border.top = line(BorderStyle::Thin, theme(4));
        s.border.bottom = line(BorderStyle::Double, theme(4));
    }
    // themed styles: each accent in 20%, 40%, and 60% lighter, and the accent itself with white text
    const int accentIds[6][4] = {{30, 31, 32, 29}, {34, 35, 36, 33}, {38, 39, 40, 37}, {42, 43, 44, 41}, {46, 47, 48, 45}, {50, 51, 52, 49}};
    const double tints[3] = {0.7999816888943144, 0.5999938962981048, 0.3999755851924192};
    const char *percents[3] = {"20%", "40%", "60%"};
    for (int a = 0; a < 6; ++a) {
        for (int k = 0; k < 3; ++k) {
            BuiltinStyle &s = add("", accentIds[a][k], 3);
            s.name = QStringLiteral("%1 - Accent%2").arg(QLatin1String(percents[k])).arg(a + 1);
            s.fill = solid(theme(4 + a, tints[k]));
        }
        BuiltinStyle &s = add("", accentIds[a][3], 3);
        s.name = QStringLiteral("Accent%1").arg(a + 1);
        s.fontColor = theme(0);
        s.fill = solid(theme(4 + a));
    }
    // number formats
    {
        BuiltinStyle &s = add("Comma", 3, 4);
        s.numFmtId = 43;
    }
    {
        BuiltinStyle &s = add("Comma [0]", 6, 4);
        s.numFmtId = 41;
    }
    {
        BuiltinStyle &s = add("Currency", 4, 4);
        s.numFmtId = 44;
    }
    {
        BuiltinStyle &s = add("Currency [0]", 7, 4);
        s.numFmtId = 42;
    }
    {
        BuiltinStyle &s = add("Percent", 5, 4);
        s.numFmtId = 9;
    }
    // (the two for links are not in the gallery's groups of their own, but the programs have them)
    {
        BuiltinStyle &s = add("Hyperlink", 8, 1);
        s.underline = Underline::Single;
        s.fontColor = theme(10);
    }
    {
        BuiltinStyle &s = add("Followed Hyperlink", 9, 1);
        s.underline = Underline::Single;
        s.fontColor = theme(11);
    }
    return all;
}

const QVector<BuiltinStyle> &all()
{
    static const QVector<BuiltinStyle> styles = makeAll();
    return styles;
}

} // namespace

QStringList builtinStyleNames()
{
    QStringList out;
    for (int group = 0; group <= 4; ++group)
        for (const BuiltinStyle &s : all())
            if (s.group == group) out << s.name;
    return out;
}

bool builtinStyle(const QString &name, BuiltinStyle *out)
{
    for (const BuiltinStyle &s : all())
        if (s.name.compare(name, Qt::CaseInsensitive) == 0) {
            if (out) *out = s;
            return true;
        }
    return false;
}

} // namespace jp::office::sheet
