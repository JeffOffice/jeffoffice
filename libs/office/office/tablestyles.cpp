// The table styles a presentation names by id (tablestyles.h).

#include "office/tablestyles.h"

namespace jp::office {

namespace {

// The families, each in the same seven (or four) colors: plain, then accents 1 to 6.
enum Family { Light1, Light2, Light3, Medium1, Medium2, Medium3, Medium4, Dark1, Dark2, FamilyCount };

struct FamilyInfo {
    const char *group;
    const char *name;
    // plain, accent 1, ... accent 6; a family of two-color styles lists one id for each pair (plain, 1 and 2, 3 and 4, 5 and 6) in the first four
    const char *ids[7];
};

const FamilyInfo kFamilies[FamilyCount] = {
    {"Light", "Light Style 1", {"{9D7B26C5-4107-4FEC-AEDC-1716B250A1EF}", "{3B4B98B0-60AC-42C2-AFA5-B58CD77FA1E5}", "{0E3FDE45-AF77-4B5C-9715-49D594BDF05E}",
                                "{C083E6E3-FA7D-4D7B-A595-EF9225AFEA82}", "{D27102A9-8310-4765-A935-A1911B00CA55}", "{5FD0F851-EC5A-4D38-B0AD-8093EC10F338}",
                                "{68D230F3-CF80-4859-8CE7-A43EE81993B5}"}},
    {"Light", "Light Style 2", {"{7E9639D4-E3E2-4D34-9284-5A2195B3D0D7}", "{69012ECD-51FC-41F1-AA8D-1B2483CD663E}", "{72833802-FEF1-4C79-8D5D-14CF1EAF98D9}",
                                "{F2DE63D5-997A-4646-A377-4702673A728D}", "{17292A2E-F333-43FB-9621-5CBBE7FDCDCB}", "{5A111915-BE36-4E01-A7E5-04B1672EAD32}",
                                "{912C8C85-51F0-491E-9774-3900AFEF0FD7}"}},
    {"Light", "Light Style 3", {"{616DA210-FB5B-4158-B5E0-FEB733F419BA}", "{BC89EF96-8CEA-46FF-86C4-4CE0E7609802}", "{5DA37D80-6434-44D0-A028-1B22A696006F}",
                                "{8799B23B-EC83-4686-B30A-512413B5E67A}", "{ED083AE6-46FA-4A59-8FB0-9F97EB10719F}", "{BDBED569-4797-4DF1-A0F4-6AAB3CD982D8}",
                                "{E8B1032C-EA38-4F05-BA0D-38AFFFC7BED3}"}},
    {"Medium", "Medium Style 1", {"{793D81CF-94F2-401A-BA57-92F5A7B2D0C5}", "{B301B821-A1FF-4177-AEE7-76D212191A09}", "{9DCAF9ED-07DC-4A11-8D7F-57B35C25682E}",
                                  "{1FECB4D8-DB02-4DC6-A0A2-4F2EBAE1DC90}", "{1E171933-4619-4E11-9A3F-F7608DF75F80}", "{FABFCF23-3B69-468F-B69F-88F6DE6A72F2}",
                                  "{10A1B5D5-9B99-4C35-A422-299274C87663}"}},
    {"Medium", "Medium Style 2", {"{073A0DAA-6AF3-43AB-8588-CEC1D06C72B9}", "{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}", "{21E4AEA4-8DFA-4A89-87EB-49C32662AFE0}",
                                  "{F5AB1C69-6EDB-4FF4-983F-18BD219EF322}", "{00A15C55-8517-42AA-B614-E9B94910E393}", "{7DF18680-E054-41AD-8BC1-D1AEF772440D}",
                                  "{93296810-A885-4BE3-A3E7-6D5BEEA58F35}"}},
    {"Medium", "Medium Style 3", {"{8EC20E35-A176-4012-BC5E-935CFFF8708E}", "{6E25E649-3F16-4E02-A733-19D2CDBF48F0}", "{85BE263C-DBD7-4A20-BB59-AAB30ACAA65A}",
                                  "{EB344D84-9AFB-497E-A393-DC336BA19D2E}", "{EB9631B5-78F2-41C9-869B-9F39066F8104}", "{74C1A8A3-306A-4EB7-A6B1-4F7E0EB9C5D6}",
                                  "{2A488322-F2BA-4B5B-9748-0D474271808F}"}},
    {"Medium", "Medium Style 4", {"{D7AC3CCA-C797-4891-BE02-D94E43425B78}", "{69CF1AB2-1976-4502-BF36-3FF5EA218861}", "{8A107856-5554-42FB-B03E-39F5DBC370BA}",
                                  "{0505E3EF-67EA-436B-97B2-0124C06EBD24}", "{C4B1156A-380E-4F78-BDF5-A606A8083BF9}", "{22838BEF-8BB2-4498-84A7-C5851F593DF1}",
                                  "{16D9F66E-5EB9-4882-86FB-DCBF35E3C3E4}"}},
    {"Dark", "Dark Style 1", {"{E8034E78-7F5D-4C2E-B375-FC64B27BC917}", "{125E5076-3810-47DD-B79F-674D7AD40C01}", "{37CE84F3-28C3-443E-9E96-99CF82512B78}",
                              "{D03447BB-5D67-496B-8E87-E561075AD55C}", "{E929F9F4-4A8F-4326-A1B4-22849713DDAB}", "{8FD4443E-F989-4FC4-A0C8-D5A2AF1F390B}",
                              "{AF606853-7671-496A-8E4F-DF71F8EC918B}"}},
    {"Dark", "Dark Style 2", {"{5202B0CA-FC54-4496-8BCA-5EF66A818D29}", "{0660B408-B3CF-4A94-85FC-2B1E0A45F4A2}", "{91EBBBCC-DAD2-459C-BE2E-F6DE35CF9A28}",
                              "{46F890A9-2807-4EBB-B81D-B2AA78EC7F39}", nullptr, nullptr, nullptr}},
};

const char *const kNoGrid = "{2D5ABB26-0587-4C30-8999-92F81FD0307C}";
const char *const kTableGrid = "{5940675A-B579-460E-94D1-54222C63F5DA}";

// ---- the XML of a style

QString color(const QString &name, const QString &change = QString(), int amount = 0)
{
    if (change.isEmpty()) return QStringLiteral("<a:schemeClr val=\"%1\"/>").arg(name);
    return QStringLiteral("<a:schemeClr val=\"%1\"><a:%2 val=\"%3\"/></a:schemeClr>").arg(name, change).arg(amount);
}

QString solid(const QString &clr) { return QStringLiteral("<a:fill><a:solidFill>%1</a:solidFill></a:fill>").arg(clr); }
QString noFill() { return QStringLiteral("<a:fill><a:noFill/></a:fill>"); }

// A line `width` EMUs wide in a color (a:ln), or none.
QString line(int width, const QString &clr, const char *compound = "sng")
{
    return QStringLiteral("<a:ln w=\"%1\" cmpd=\"%2\"><a:solidFill>%3</a:solidFill></a:ln>").arg(width).arg(QLatin1String(compound), clr);
}
QString noLine() { return QStringLiteral("<a:ln><a:noFill/></a:ln>"); }

// The sides of a region's border, in the order the schema lists them; a side not given is left to the regions below.
struct Borders {
    QString left, right, top, bottom, insideH, insideV;
    QString xml() const
    {
        QString s;
        const struct { const char *name; const QString *ln; } sides[] = {{"left", &left}, {"right", &right}, {"top", &top}, {"bottom", &bottom}, {"insideH", &insideH}, {"insideV", &insideV}};
        for (const auto &side : sides)
            if (!side.ln->isEmpty()) s += QStringLiteral("<a:%1>%2</a:%1>").arg(QLatin1String(side.name), *side.ln);
        return s.isEmpty() ? s : QStringLiteral("<a:tcBdr>") + s + QStringLiteral("</a:tcBdr>");
    }
};

// One region: its text (bold, and the color when it has one) and its cells' lines and fill.
QString region(const char *name, bool bold, const QString &textColor, const Borders &borders, const QString &fill)
{
    QString text;
    if (bold || !textColor.isEmpty())
        text = QStringLiteral("<a:tcTxStyle%1><a:fontRef idx=\"minor\"/>%2</a:tcTxStyle>").arg(bold ? QStringLiteral(" b=\"on\"") : QString(), textColor);
    QString style;
    const QString edges = borders.xml();
    if (!edges.isEmpty() || !fill.isEmpty()) style = QStringLiteral("<a:tcStyle>") + edges + fill + QStringLiteral("</a:tcStyle>");
    return QStringLiteral("<a:%1>%2%3</a:%1>").arg(QLatin1String(name), text, style);
}

Borders all(const QString &ln)
{
    Borders b;
    b.left = b.right = b.top = b.bottom = b.insideH = b.insideV = ln;
    return b;
}

// The regions of one style, in the order the schema lists them. `a` is the style's color (an accent, or dark 1 for the plain style),
// `b` the second color of a two-color style.
QString regions(Family family, const QString &a, const QString &b)
{
    const QString dk = color(QStringLiteral("dk1")), lt = color(QStringLiteral("lt1")), accent = color(a);
    QString out;
    switch (family) {
    case Light1: {
        Borders whole;
        whole.top = whole.bottom = line(12700, accent);
        whole.left = whole.right = whole.insideH = whole.insideV = noLine();
        Borders head, foot;
        head.bottom = line(12700, accent);
        foot.top = line(12700, accent);
        const QString band = solid(color(a, QStringLiteral("alpha"), 20000));
        out += region("wholeTbl", false, dk, whole, noFill());
        out += region("band1H", false, QString(), Borders(), band);
        out += region("band1V", false, QString(), Borders(), band);
        out += region("lastCol", true, QString(), Borders(), QString());
        out += region("firstCol", true, QString(), Borders(), QString());
        out += region("lastRow", true, QString(), foot, noFill());
        out += region("firstRow", true, QString(), head, noFill());
        break;
    }
    case Light2: {
        Borders whole;
        whole.left = whole.right = whole.top = whole.bottom = line(12700, accent);
        Borders rows, columns, foot;
        rows.top = rows.bottom = line(12700, accent);
        columns.left = columns.right = line(12700, accent);
        foot.top = line(50800, accent, "dbl");
        out += region("wholeTbl", false, dk, whole, noFill());
        out += region("band1H", false, QString(), rows, QString());
        out += region("band1V", false, QString(), columns, QString());
        out += region("lastCol", true, QString(), Borders(), QString());
        out += region("firstCol", true, QString(), Borders(), QString());
        out += region("lastRow", true, QString(), foot, QString());
        out += region("firstRow", true, lt, Borders(), solid(accent));
        break;
    }
    case Light3: {
        Borders foot, head;
        foot.top = line(50800, accent, "dbl");
        head.bottom = line(25400, accent);
        const QString band = solid(color(a, QStringLiteral("alpha"), 20000));
        out += region("wholeTbl", false, dk, all(line(12700, accent)), noFill());
        out += region("band1H", false, QString(), Borders(), band);
        out += region("band1V", false, QString(), Borders(), band);
        out += region("lastCol", true, QString(), Borders(), QString());
        out += region("firstCol", true, QString(), Borders(), QString());
        out += region("lastRow", true, QString(), foot, noFill());
        out += region("firstRow", true, accent, head, noFill());
        break;
    }
    case Medium1: {
        Borders whole = all(line(12700, accent));
        whole.insideV = noLine();
        Borders foot;
        foot.top = line(50800, accent, "dbl");
        const QString band = solid(color(a, QStringLiteral("tint"), 20000));
        out += region("wholeTbl", false, dk, whole, solid(lt));
        out += region("band1H", false, QString(), Borders(), band);
        out += region("band1V", false, QString(), Borders(), band);
        out += region("lastCol", true, QString(), Borders(), QString());
        out += region("firstCol", true, QString(), Borders(), QString());
        out += region("lastRow", true, QString(), foot, solid(lt));
        out += region("firstRow", true, lt, Borders(), solid(accent));
        break;
    }
    case Medium2: {
        const QString white = line(12700, lt);
        Borders foot, head;
        foot.top = line(38100, lt);
        head.bottom = line(38100, lt);
        out += region("wholeTbl", false, dk, all(white), solid(color(a, QStringLiteral("tint"), 20000)));
        out += region("band1H", false, QString(), Borders(), solid(color(a, QStringLiteral("tint"), 40000)));
        out += region("band1V", false, QString(), Borders(), solid(color(a, QStringLiteral("tint"), 40000)));
        out += region("lastCol", true, lt, Borders(), solid(accent));
        out += region("firstCol", true, lt, Borders(), solid(accent));
        out += region("lastRow", true, lt, foot, solid(accent));
        out += region("firstRow", true, lt, head, solid(accent));
        break;
    }
    case Medium3: {
        Borders whole;
        whole.top = whole.bottom = line(12700, dk);
        whole.left = whole.right = whole.insideH = whole.insideV = noLine();
        Borders foot;
        foot.top = line(50800, dk);
        const QString band = solid(color(QStringLiteral("dk1"), QStringLiteral("tint"), 20000));
        out += region("wholeTbl", false, dk, whole, solid(lt));
        out += region("band1H", false, QString(), Borders(), band);
        out += region("band1V", false, QString(), Borders(), band);
        out += region("lastCol", true, QString(), Borders(), QString());
        out += region("firstCol", true, QString(), Borders(), QString());
        out += region("lastRow", true, QString(), foot, solid(lt));
        out += region("firstRow", true, lt, Borders(), solid(accent));
        break;
    }
    case Medium4: {
        Borders head;
        head.bottom = line(38100, accent);
        Borders foot;
        foot.top = line(38100, accent);
        const QString band = solid(color(a, QStringLiteral("tint"), 40000));
        out += region("wholeTbl", false, dk, all(line(12700, accent)), solid(color(a, QStringLiteral("tint"), 20000)));
        out += region("band1H", false, QString(), Borders(), band);
        out += region("band1V", false, QString(), Borders(), band);
        out += region("lastCol", true, QString(), Borders(), QString());
        out += region("firstCol", true, QString(), Borders(), QString());
        out += region("lastRow", true, QString(), foot, QString());
        out += region("firstRow", true, QString(), head, QString());
        break;
    }
    case Dark1: {
        // Light text on the color made dark (the plain style: dark 1 lightened a little); the body is darkest, then the bands, then the columns.
        const bool plain = a == QLatin1String("dk1");
        const auto tone = [&](int level) {
            return plain ? color(a, QStringLiteral("tint"), 85000 - 15000 * level) : color(a, QStringLiteral("shade"), 20000 + 20000 * level);
        };
        Borders head, foot;
        head.bottom = line(38100, lt);
        foot.top = line(38100, lt);
        out += region("wholeTbl", false, lt, Borders(), solid(tone(0)));
        out += region("band1H", false, QString(), Borders(), solid(tone(1)));
        out += region("band1V", false, QString(), Borders(), solid(tone(1)));
        out += region("lastCol", true, lt, Borders(), solid(tone(2)));
        out += region("firstCol", true, lt, Borders(), solid(tone(2)));
        out += region("lastRow", true, lt, foot, solid(dk));
        out += region("firstRow", true, lt, head, solid(dk));
        break;
    }
    case Dark2: {
        // Two colors: the first for the body and the first and last columns, the second for the first and last rows.
        Borders head, foot;
        head.bottom = line(38100, lt);
        foot.top = line(38100, lt);
        const QString second = color(b);
        out += region("wholeTbl", false, dk, Borders(), solid(color(a, QStringLiteral("tint"), 20000)));
        out += region("band1H", false, QString(), Borders(), solid(color(a, QStringLiteral("tint"), 40000)));
        out += region("band1V", false, QString(), Borders(), solid(color(a, QStringLiteral("tint"), 40000)));
        out += region("lastCol", true, lt, Borders(), solid(accent));
        out += region("firstCol", true, lt, Borders(), solid(accent));
        out += region("lastRow", true, lt, foot, solid(second));
        out += region("firstRow", true, lt, head, solid(second));
        break;
    }
    case FamilyCount: break;
    }
    return out;
}

QString accentName(int index) { return index == 0 ? QStringLiteral("dk1") : QStringLiteral("accent%1").arg(index); }

QString styleName(const FamilyInfo &family, int index)
{
    if (index == 0) return QLatin1String(family.name);
    return QStringLiteral("%1 - Accent %2").arg(QLatin1String(family.name)).arg(index);
}

struct Located {
    Family family = FamilyCount;
    int index = 0;   // 0 plain, else the accent
};

Located locate(const QString &id)
{
    for (int f = 0; f < FamilyCount; ++f)
        for (int i = 0; i < 7; ++i)
            if (kFamilies[f].ids[i] && id.compare(QLatin1String(kFamilies[f].ids[i]), Qt::CaseInsensitive) == 0) return {Family(f), i};
    return {};
}

const char *const kPlainStyleHead = "<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">";

QString noStyle(const char *id, const char *name, bool grid)
{
    const QString ln = grid ? line(12700, color(QStringLiteral("tx1"))) : noLine();
    Borders borders = all(ln);
    return QStringLiteral("<a:tblStyle styleId=\"%1\" styleName=\"%2\">").arg(QLatin1String(id), QLatin1String(name))
           + region("wholeTbl", false, color(QStringLiteral("tx1")), borders, noFill()) + QStringLiteral("</a:tblStyle>");
}

} // namespace

const QVector<TableStyleInfo> &tableStyleList()
{
    static const QVector<TableStyleInfo> list = [] {
        QVector<TableStyleInfo> v;
        // The usual style's family leads (the gallery's first strip shows the first few), then the rest of the medium ones, the light, and the dark.
        for (const Family f : {Medium2, Medium1, Medium3, Medium4, Light1, Light2, Light3, Dark1, Dark2}) {
            const int count = f == Dark2 ? 4 : 7;
            for (int i = 0; i < count; ++i) {
                QString name = styleName(kFamilies[f], i);
                QString accent = i == 0 ? QString() : QStringLiteral("accent%1").arg(f == Dark2 ? 2 * i - 1 : i);
                if (f == Dark2 && i > 0) name = QStringLiteral("%1 - Accent %2/Accent %3").arg(QLatin1String(kFamilies[f].name)).arg(2 * i - 1).arg(2 * i);
                v.append({QLatin1String(kFamilies[f].ids[i]), name, QLatin1String(kFamilies[f].group), accent});
            }
        }
        v.append({QLatin1String(kNoGrid), QStringLiteral("No Style, No Grid"), QString(), QString()});
        v.append({QLatin1String(kTableGrid), QStringLiteral("No Style, Table Grid"), QString(), QString()});
        return v;
    }();
    return list;
}

QByteArray tableStyleXml(const QString &id)
{
    QString style;
    if (id.compare(QLatin1String(kNoGrid), Qt::CaseInsensitive) == 0) {
        style = noStyle(kNoGrid, "No Style, No Grid", false);
    } else if (id.compare(QLatin1String(kTableGrid), Qt::CaseInsensitive) == 0) {
        style = noStyle(kTableGrid, "No Style, Table Grid", true);
    } else {
        const Located at = locate(id);
        if (at.family == FamilyCount) return QByteArray();
        const FamilyInfo &family = kFamilies[at.family];
        QString a = accentName(at.index), b = a;
        QString name = styleName(family, at.index);
        if (at.family == Dark2) {
            // plain: dark 1 and dark 2; the others, two accents in a row.
            a = at.index == 0 ? QStringLiteral("dk1") : QStringLiteral("accent%1").arg(2 * at.index - 1);
            b = at.index == 0 ? QStringLiteral("dk2") : QStringLiteral("accent%1").arg(2 * at.index);
            if (at.index > 0) name = QStringLiteral("%1 - Accent %2/Accent %3").arg(QLatin1String(family.name)).arg(2 * at.index - 1).arg(2 * at.index);
        }
        style = QStringLiteral("<a:tblStyle styleId=\"%1\" styleName=\"%2\">").arg(QLatin1String(family.ids[at.index]), name) + regions(at.family, a, b)
                + QStringLiteral("</a:tblStyle>");
    }
    return (QLatin1String(kPlainStyleHead) + style + QStringLiteral("</a:tblStyleLst>")).toUtf8();
}

QString defaultTableStyleId() { return QStringLiteral("{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}"); }

} // namespace jp::office
