#include "office/theme.h"

#include "office/xml.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QXmlStreamWriter>

namespace jp::office {

namespace {

const char *const kColorNames[Theme::ColorCount] = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4",
                                                    "accent5", "accent6", "hlink", "folHlink"};

// Six hex digits, as a:srgbClr and lastClr write a color.
QColor hexColor(const QString &s)
{
    static const QRegularExpression six(QStringLiteral("^[0-9A-Fa-f]{6}$"));
    return six.match(s).hasMatch() ? QColor(QLatin1Char('#') + s) : QColor();
}

// The color a theme slot holds: an RGB value, or a system color, which the
// file gives as the value it had when saved (lastClr).
QColor slotColor(const QDomElement &slot)
{
    const QDomElement rgb = child(slot, ns::A, QStringLiteral("srgbClr"));
    if (!rgb.isNull()) return hexColor(attr(rgb, QStringLiteral("val")));
    const QDomElement system = child(slot, ns::A, QStringLiteral("sysClr"));
    if (system.isNull()) return QColor();
    const QColor last = hexColor(attr(system, QStringLiteral("lastClr")));
    if (last.isValid()) return last;
    const QString val = attr(system, QStringLiteral("val"));
    if (val == QLatin1String("window")) return QColor(Qt::white);
    if (val == QLatin1String("windowText")) return QColor(Qt::black);
    return QColor();
}

QString typeface(const QDomElement &fonts, const QString &kind)
{
    return attr(child(fonts, ns::A, kind), QStringLiteral("typeface"));
}

QString colorHex(const QColor &c)
{
    return c.name(QColor::HexRgb).mid(1).toUpper();
}

} // namespace

QString themeColorName(Theme::Color slot)
{
    return slot >= 0 && slot < Theme::ColorCount ? QString::fromLatin1(kColorNames[slot]) : QString();
}

bool themeColorFromName(const QString &name, Theme::Color *slot)
{
    for (int i = 0; i < Theme::ColorCount; ++i)
        if (name == QLatin1String(kColorNames[i])) {
            *slot = Theme::Color(i);
            return true;
        }
    return false;
}

bool readTheme(const QByteArray &xml, Theme *theme, QString *error)
{
    auto fail = [&](const QString &message) {
        if (error) *error = message;
        return false;
    };
    QDomDocument doc;
    QString why;
    if (!parseXml(xml, &doc, &why)) return fail(why);
    const QDomElement root = doc.documentElement();
    if (!isElement(root, ns::A, QStringLiteral("theme"))) return fail(QCoreApplication::translate("Theme", "The part is not a theme."));
    const QDomElement elements = child(root, ns::A, QStringLiteral("themeElements"));
    const QDomElement colors = child(elements, ns::A, QStringLiteral("clrScheme"));
    if (colors.isNull()) return fail(QCoreApplication::translate("Theme", "The theme has no colors."));

    Theme t;
    t.name = attr(root, QStringLiteral("name"));
    t.colorSchemeName = attr(colors, QStringLiteral("name"));
    const Theme fallback = defaultTheme();
    for (int i = 0; i < Theme::ColorCount; ++i) {
        t.colors[i] = slotColor(child(colors, ns::A, QString::fromLatin1(kColorNames[i])));
        if (!t.colors[i].isValid()) t.colors[i] = fallback.colors[i];
    }
    const QDomElement fonts = child(elements, ns::A, QStringLiteral("fontScheme"));
    t.fontSchemeName = attr(fonts, QStringLiteral("name"));
    const QDomElement major = child(fonts, ns::A, QStringLiteral("majorFont")), minor = child(fonts, ns::A, QStringLiteral("minorFont"));
    t.majorLatin = typeface(major, QStringLiteral("latin"));
    t.majorEastAsian = typeface(major, QStringLiteral("ea"));
    t.majorComplex = typeface(major, QStringLiteral("cs"));
    t.minorLatin = typeface(minor, QStringLiteral("latin"));
    t.minorEastAsian = typeface(minor, QStringLiteral("ea"));
    t.minorComplex = typeface(minor, QStringLiteral("cs"));
    t.xml = xml;
    *theme = t;
    return true;
}

QByteArray writeTheme(const Theme &theme)
{
    if (!theme.xml.isEmpty()) return theme.xml;
    const Theme fallback = defaultTheme();
    QByteArray xml;
    QXmlStreamWriter w(&xml);
    auto open = [&](const char *name) { w.writeStartElement(QString::fromLatin1(name)); };
    auto close = [&] { w.writeEndElement(); };
    auto empty = [&](const char *name) { w.writeEmptyElement(QString::fromLatin1(name)); };
    auto attribute = [&](const char *name, const QString &value) { w.writeAttribute(QString::fromLatin1(name), value); };
    // A fill and a line that take whatever color a shape asks of the theme.
    auto placeholderFill = [&] {
        open("a:solidFill");
        empty("a:schemeClr");
        attribute("val", QStringLiteral("phClr"));
        close();
    };
    auto font = [&](const char *kind, const QString &latin, const QString &eastAsian, const QString &complex) {
        open(kind);
        empty("a:latin");
        attribute("typeface", latin);
        empty("a:ea");
        attribute("typeface", eastAsian);
        empty("a:cs");
        attribute("typeface", complex);
        close();
    };

    w.writeStartDocument(QStringLiteral("1.0"), true);
    open("a:theme");
    w.writeNamespace(ns::A, QStringLiteral("a"));
    attribute("name", theme.name);
    open("a:themeElements");

    open("a:clrScheme");
    attribute("name", theme.colorSchemeName);
    for (int i = 0; i < Theme::ColorCount; ++i) {
        w.writeStartElement(QStringLiteral("a:") + QLatin1String(kColorNames[i]));
        empty("a:srgbClr");
        attribute("val", colorHex(theme.colors[i].isValid() ? theme.colors[i] : fallback.colors[i]));
        close();
    }
    close();

    open("a:fontScheme");
    attribute("name", theme.fontSchemeName);
    font("a:majorFont", theme.majorLatin, theme.majorEastAsian, theme.majorComplex);
    font("a:minorFont", theme.minorLatin, theme.minorEastAsian, theme.minorComplex);
    close();

    open("a:fmtScheme");
    attribute("name", theme.name);
    open("a:fillStyleLst");
    for (int i = 0; i < 3; ++i) placeholderFill();
    close();
    open("a:lnStyleLst");
    for (const char *width : {"6350", "12700", "19050"}) {
        open("a:ln");
        attribute("w", QString::fromLatin1(width));
        attribute("cap", QStringLiteral("flat"));
        attribute("cmpd", QStringLiteral("sng"));
        attribute("algn", QStringLiteral("ctr"));
        placeholderFill();
        empty("a:prstDash");
        attribute("val", QStringLiteral("solid"));
        empty("a:miter");
        attribute("lim", QStringLiteral("800000"));
        close();
    }
    close();
    open("a:effectStyleLst");
    for (int i = 0; i < 3; ++i) {
        open("a:effectStyle");
        empty("a:effectLst");
        close();
    }
    close();
    open("a:bgFillStyleLst");
    for (int i = 0; i < 3; ++i) placeholderFill();
    close();
    close();   // fmtScheme

    close();   // themeElements
    empty("a:objectDefaults");
    empty("a:extraClrSchemeLst");
    close();   // theme
    w.writeEndDocument();
    return xml;
}

Theme defaultTheme()
{
    Theme t;
    t.name = t.colorSchemeName = t.fontSchemeName = QStringLiteral("JeffOffice");
    const char *const colors[Theme::ColorCount] = {"#1F2937", "#FFFFFF", "#24364F", "#E8ECF2", "#1D55C4", "#B6420F",
                                                   "#13773A", "#9E1F63", "#0A6C8C", "#5E2CA5", "#1D55C4", "#5E2CA5"};
    for (int i = 0; i < Theme::ColorCount; ++i) t.colors[i] = QColor(QLatin1String(colors[i]));
    t.majorLatin = t.minorLatin = QStringLiteral("Carlito");
    return t;
}

} // namespace jp::office
