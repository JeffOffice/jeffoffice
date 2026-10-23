// A new word processing document, and the styles a person may apply that a document might lack. The looks of the styles are
// JeffOffice's own.

#include "wml/wml.h"

#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QDateTime>

namespace jp::office::wml {

namespace {

Fonts themeFonts(bool major)
{
    Fonts f;
    const QString p = major ? QStringLiteral("major") : QStringLiteral("minor");
    f.asciiTheme = f.hAnsiTheme = p + QStringLiteral("HAnsi");
    f.eastAsiaTheme = p + QStringLiteral("EastAsia");
    f.csTheme = p + QStringLiteral("Bidi");
    return f;
}

Color accent(const QString &shade = QString())
{
    const Theme t = defaultTheme();
    QColor c = t.colors[Theme::Accent1];
    if (!shade.isEmpty()) c = c.darker(135);
    return Color{c.name().mid(1).toUpper(), QStringLiteral("accent1"), QString(), shade};
}

Style paragraphStyle(const QString &id, const QString &name, int priority)
{
    Style s;
    s.id = id;
    s.name = name;
    s.type = QStringLiteral("paragraph");
    s.basedOn = QStringLiteral("Normal");
    s.next = QStringLiteral("Normal");
    s.uiPriority = priority;
    s.qFormat = true;
    return s;
}

Style heading(int level)
{
    Style s = paragraphStyle(QStringLiteral("Heading%1").arg(level), QStringLiteral("heading %1").arg(level), 9);
    s.pPr.keepNext = true;
    s.pPr.keepLines = true;
    s.pPr.outlineLvl = level - 1;
    s.rPr.fonts = themeFonts(true);
    static const int sizes[] = {32, 26, 24, 22, 22, 22, 22, 22, 22};
    s.rPr.size = sizes[level - 1];
    s.rPr.color = accent(level <= 3 ? QStringLiteral("BF") : QString());
    if (level == 1) {
        s.pPr.before = 360;
        s.pPr.after = 80;
    } else if (level <= 3) {
        s.pPr.before = 160;
        s.pPr.after = 80;
    } else {
        s.pPr.before = 80;
        s.pPr.after = 40;
    }
    if (level >= 4) s.rPr.italic = level == 4;
    if (level >= 6) s.rPr.color = Color{QStringLiteral("595959"), QString(), QString(), QString()};
    if (level > 3) s.uiPriority = 9;
    return s;
}

} // namespace

QStringList builtinStyleIds()
{
    QStringList ids{QStringLiteral("Normal"), QStringLiteral("DefaultParagraphFont"), QStringLiteral("Title"), QStringLiteral("Subtitle")};
    for (int i = 1; i <= 9; ++i) ids << QStringLiteral("Heading%1").arg(i);
    ids << QStringLiteral("NoSpacing") << QStringLiteral("ListParagraph") << QStringLiteral("Quote") << QStringLiteral("IntenseQuote") << QStringLiteral("Emphasis")
        << QStringLiteral("Strong") << QStringLiteral("Hyperlink");
    return ids;
}

Style builtinStyle(const QString &id)
{
    if (id == QLatin1String("Normal")) {
        Style s;
        s.id = QStringLiteral("Normal");
        s.name = QStringLiteral("Normal");
        s.type = QStringLiteral("paragraph");
        s.isDefault = true;
        s.qFormat = true;
        return s;
    }
    if (id == QLatin1String("DefaultParagraphFont")) {
        Style s;
        s.id = id;
        s.name = QStringLiteral("Default Paragraph Font");
        s.type = QStringLiteral("character");
        s.isDefault = true;
        s.uiPriority = 1;
        s.semiHidden = s.unhideWhenUsed = true;
        return s;
    }
    if (id.startsWith(QLatin1String("Heading")) && id.size() == 8 && id[7] >= QLatin1Char('1') && id[7] <= QLatin1Char('9')) return heading(id[7].digitValue());
    if (id == QLatin1String("Title")) {
        Style s = paragraphStyle(id, QStringLiteral("Title"), 10);
        s.pPr.contextualSpacing = true;
        s.pPr.after = 80;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        s.rPr.fonts = themeFonts(true);
        s.rPr.size = 56;
        s.rPr.spacing = -10;
        return s;
    }
    if (id == QLatin1String("Subtitle")) {
        Style s = paragraphStyle(id, QStringLiteral("Subtitle"), 11);
        s.pPr.after = 160;
        s.rPr.size = 28;
        s.rPr.spacing = 15;
        s.rPr.color = Color{QStringLiteral("595959"), QString(), QString(), QString()};
        return s;
    }
    if (id == QLatin1String("NoSpacing")) {
        Style s = paragraphStyle(id, QStringLiteral("No Spacing"), 1);
        s.next.clear();
        s.pPr.after = 0;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        return s;
    }
    if (id == QLatin1String("ListParagraph")) {
        Style s = paragraphStyle(id, QStringLiteral("List Paragraph"), 34);
        s.next.clear();
        s.pPr.indLeft = 720;
        s.pPr.contextualSpacing = true;
        return s;
    }
    if (id == QLatin1String("Quote")) {
        Style s = paragraphStyle(id, QStringLiteral("Quote"), 29);
        s.pPr.before = 200;
        s.pPr.after = 160;
        s.pPr.indLeft = 864;
        s.pPr.indRight = 864;
        s.pPr.jc = QStringLiteral("center");
        s.rPr.italic = true;
        s.rPr.color = Color{QStringLiteral("404040"), QString(), QString(), QString()};
        return s;
    }
    if (id == QLatin1String("IntenseQuote")) {
        Style s = paragraphStyle(id, QStringLiteral("Intense Quote"), 30);
        s.pPr.before = 360;
        s.pPr.after = 360;
        s.pPr.indLeft = 864;
        s.pPr.indRight = 864;
        s.pPr.jc = QStringLiteral("center");
        s.rPr.italic = true;
        s.rPr.color = accent();
        return s;
    }
    if (id == QLatin1String("Emphasis") || id == QLatin1String("Strong")) {
        Style s;
        s.id = id;
        s.name = id;
        s.type = QStringLiteral("character");
        s.uiPriority = id == QLatin1String("Strong") ? 22 : 20;
        s.qFormat = true;
        if (id == QLatin1String("Strong")) s.rPr.bold = true;
        else s.rPr.italic = true;
        return s;
    }
    if (id == QLatin1String("Header") || id == QLatin1String("Footer")) {
        // The styles of the text in a page's header and footer: a center tab and a right tab at the middle and the end of a line of the usual page's text.
        Style s = paragraphStyle(id, id, 99);
        s.next.clear();
        s.qFormat = false;
        s.unhideWhenUsed = true;
        s.pPr.after = 0;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        s.pPr.tabs = QVector<TabStop>{TabStop{4680, QStringLiteral("center"), QString()}, TabStop{9360, QStringLiteral("right"), QString()}};
        return s;
    }
    if (id == QLatin1String("Hyperlink")) {
        Style s;
        s.id = id;
        s.name = QStringLiteral("Hyperlink");
        s.type = QStringLiteral("character");
        s.uiPriority = 99;
        s.unhideWhenUsed = true;
        s.rPr.color = accent();
        s.rPr.underline = Underline{QStringLiteral("single"), QString()};
        return s;
    }
    return Style();
}

namespace {

QByteArray xmlDoc(const QByteArray &root) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + root; }

AbstractNum listDefinition(int id, bool bullets)
{
    AbstractNum a;
    a.id = id;
    a.multiLevelType = QStringLiteral("hybridMultilevel");
    static const char *const bulletText[] = {"•", "◦", "▪"};
    static const char *const numFmt[] = {"decimal", "lowerLetter", "lowerRoman"};
    for (int i = 0; i < 9; ++i) {
        Level &l = a.levels[i];
        l.present = true;
        l.start = 1;
        l.jc = QStringLiteral("left");
        if (bullets) {
            l.numFmt = QStringLiteral("bullet");
            l.text = QString::fromUtf8(bulletText[i % 3]);
        } else {
            l.numFmt = QString::fromLatin1(numFmt[i % 3]);
            l.text = QStringLiteral("%%1.").arg(i + 1);
        }
        l.pPr.indLeft = 720 * (i + 1);
        l.pPr.indFirst = -360;
    }
    return a;
}

} // namespace

Document newDocument()
{
    Document d;
    d.mainContentType = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
    d.stylesName = QStringLiteral("/word/styles.xml");
    d.numberingName = QStringLiteral("/word/numbering.xml");
    d.settingsName = QStringLiteral("/word/settings.xml");
    d.themeName = QStringLiteral("/word/theme/theme1.xml");
    d.theme = defaultTheme();
    d.hasTheme = true;

    StyleSheet &s = d.styles;
    s.defaultRun.fonts = themeFonts(false);
    s.defaultRun.size = 22;
    s.defaultRun.extra.push_back({runChildRank("szCs"), "<w:szCs w:val=\"22\"/>"});
    s.defaultRun.extra.push_back({runChildRank("lang"), "<w:lang w:val=\"en-US\" w:eastAsia=\"en-US\" w:bidi=\"ar-SA\"/>"});
    s.defaultPara.after = 160;
    s.defaultPara.line = 259;
    s.defaultPara.lineRule = QStringLiteral("auto");
    for (const QString &id : builtinStyleIds()) s.styles.push_back(builtinStyle(id));
    s.changed();
    s.modified = true;

    d.numbering.abstracts << listDefinition(0, true) << listDefinition(1, false);
    d.numbering.nums << Num{1, 0, {}, QByteArray()} << Num{2, 1, {}, QByteArray()};
    d.numbering.modified = true;

    SectionProps &sec = d.lastSection;
    sec.hasPgSz = sec.hasPgMar = true;
    d.lastSectionStated = true;
    Block first;
    d.body.push_back(first);

    // The parts a new document keeps.
    const QByteArray w = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
    d.parts.push_back({d.settingsName, QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"),
                       xmlDoc("<w:settings " + w + "><w:zoom w:percent=\"100\"/><w:defaultTabStop w:val=\"720\"/><w:characterSpacingControl w:val=\"doNotCompress\"/>"
                              "<w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"15\"/></w:compat></w:settings>")});
    d.parts.push_back({d.themeName, QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"), writeTheme(d.theme)});
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    d.parts.push_back({QStringLiteral("/docProps/core.xml"), QStringLiteral("application/vnd.openxmlformats-package.core-properties+xml"),
                       xmlDoc("<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                              "xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
                              "<dcterms:created xsi:type=\"dcterms:W3CDTF\">" + now.toUtf8() + "</dcterms:created><dcterms:modified xsi:type=\"dcterms:W3CDTF\">" + now.toUtf8() +
                              "</dcterms:modified></cp:coreProperties>")});
    d.parts.push_back({QStringLiteral("/docProps/app.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.extended-properties+xml"),
                       xmlDoc("<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\"><Application>JeffWrite</Application></Properties>")});

    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    d.packageRelationships << Relationship{QStringLiteral("rId1"), rel + QStringLiteral("officeDocument"), d.mainName, false}
                           << Relationship{QStringLiteral("rId2"), QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"),
                                           QStringLiteral("/docProps/core.xml"), false}
                           << Relationship{QStringLiteral("rId3"), rel + QStringLiteral("extended-properties"), QStringLiteral("/docProps/app.xml"), false};
    d.mainRelationships << Relationship{QStringLiteral("rId1"), rel + QStringLiteral("styles"), d.stylesName, false}
                        << Relationship{QStringLiteral("rId2"), rel + QStringLiteral("numbering"), d.numberingName, false}
                        << Relationship{QStringLiteral("rId3"), rel + QStringLiteral("settings"), d.settingsName, false}
                        << Relationship{QStringLiteral("rId4"), rel + QStringLiteral("theme"), d.themeName, false};
    return d;
}

} // namespace jp::office::wml
