// Tests of the workbook's theme in JeffCrunch: a blank workbook has the default theme, a theme part is kept and written, the colors that name the theme stay references through the window's own
// save and open, the Page Layout menus change the theme in one step that the cells, tabs, fonts, shapes, and charts follow, and the pickers store a theme color as a reference.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "core/cellstyles.h"
#include "office/package.h"
#include "office/themelib.h"
#include "sheet/sheetreader.h"
#include "savedok.h"
#include "app/widgets.h"
#include "io/zip.h"
#include "../sheet/drawinghelpers.h"

#include <QAction>
#include <QFile>
#include <QMenu>
#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace drawtest;
using namespace jp::office;
using namespace jp::office::sheet;

namespace {

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

// The workbook with a theme part (as the first of the package's, related to the workbook).
QByteArray withThemePart(const QByteArray &zip, const Theme &theme)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(zip, parts);
    parts[QStringLiteral("xl/theme/theme1.xml")] = writeTheme(theme);
    parts[QStringLiteral("[Content_Types].xml")].replace("</Types>", "<Override PartName=\"/xl/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/></Types>");
    parts[QStringLiteral("xl/_rels/workbook.xml.rels")].replace("</Relationships>", "<Relationship Id=\"rIdTheme1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme\" Target=\"theme/theme1.xml\"/></Relationships>");
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    return e.win.w.save() && savedOk(saved);
}

// A theme of its own colors and fonts (not the default's), as a part.
Theme ourTheme()
{
    Theme t = defaultTheme();
    t.name = t.colorSchemeName = QStringLiteral("Ours");
    t.colors[Theme::Accent1] = QColor(0x20, 0x80, 0x40);
    t.colors[Theme::Accent2] = QColor(0xA0, 0x30, 0x30);
    t.colors[Theme::Accent4] = QColor(0x30, 0x30, 0xC0);
    t.majorLatin = QStringLiteral("Head Face");
    t.minorLatin = QStringLiteral("Body Face");
    return t;
}

// The styles the cells of the test workbook have: 1 a fill of accent 1 (theme 4) with a tint and a font of accent 2 (theme 5) that follows the body font, 2 a font that follows the heading font.
QByteArray themedStyles()
{
    return stylesXml(QString(),
                     QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
                                    "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" applyFill=\"1\" applyFont=\"1\"/>"
                                    "<xf numFmtId=\"0\" fontId=\"2\" fillId=\"0\" borderId=\"0\" applyFont=\"1\"/>"),
                     QStringLiteral("<fonts count=\"3\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                                    "<font><sz val=\"11\"/><color theme=\"5\"/><name val=\"Calibri\"/><scheme val=\"minor\"/></font>"
                                    "<font><sz val=\"11\"/><name val=\"Calibri Light\"/><scheme val=\"major\"/></font></fonts>"
                                    "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                                    "<fill><patternFill patternType=\"solid\"><fgColor theme=\"4\" tint=\"0.39997558519241921\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
                                    "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"));
}

QByteArray themedSheet()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"1\"><v>1</v></c><c r=\"B1\" s=\"1\"><v>2</v></c><c r=\"C1\" s=\"2\"><v>3</v></c></row>");
    return worksheet(rows, QString(), QStringLiteral("<sheetPr><tabColor theme=\"7\"/></sheetPr>"));
}

QByteArray themedBook(const Theme &theme)
{
    return buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), writeTheme(theme));
}

QColor fillAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    return QColor::fromRgb(gs.lookOf(r, c, gs.styleOf(r, c, gs.cellAt(r, c))).fg);
}

grid::CellLook lookAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    return gs.lookOf(r, c, gs.styleOf(r, c, gs.cellAt(r, c)));
}

QColor tinted(const QColor &base, double tint) { return QColor::fromRgb(QRgb(0xFF000000u | tintedRgb(base.rgb() & 0xFFFFFF, tint))); }

QAction *entry(QMenu *menu, const QString &text)
{
    for (QAction *a : menu->actions())
        if (a->text().startsWith(text)) return a;
    return nullptr;
}

// Every color of the workbook's fonts, fills, and borders that names the theme, by where it is: its place in the theme, its tint, and the color it comes to.
struct ThemeUse {
    qint32 index = 0;
    double tint = 0;
    quint32 rgb = 0;
};
QMap<QString, ThemeUse> themeColorsOf(const Styles &st)
{
    QMap<QString, ThemeUse> out;
    auto note = [&](const QString &key, const ColorSpec &c) {
        if (c.kind == ColorSpec::Kind::Theme) out.insert(key, {c.index, c.tint, st.resolve(c, 0)});
    };
    for (int i = 0; i < st.fonts.size(); ++i) note(QStringLiteral("font%1").arg(i), st.fonts[i].color);
    for (int i = 0; i < st.fills.size(); ++i) {
        note(QStringLiteral("fill%1fg").arg(i), st.fills[i].fg);
        note(QStringLiteral("fill%1bg").arg(i), st.fills[i].bg);
        for (int k = 0; k < st.fills[i].stops.size(); ++k) note(QStringLiteral("fill%1stop%2").arg(i).arg(k), st.fills[i].stops[k].color);
    }
    for (int i = 0; i < st.borders.size(); ++i) {
        const BorderSpec &b = st.borders[i];
        note(QStringLiteral("border%1l").arg(i), b.left.color);
        note(QStringLiteral("border%1r").arg(i), b.right.color);
        note(QStringLiteral("border%1t").arg(i), b.top.color);
        note(QStringLiteral("border%1b").arg(i), b.bottom.color);
        note(QStringLiteral("border%1d").arg(i), b.diagonal.color);
    }
    return out;
}

bool themePartBytes(const QString &path, QByteArray *bytes)
{
    const QMap<QString, QByteArray> parts = partsOf(path);
    for (auto it = parts.begin(); it != parts.end(); ++it)
        if (it.key().startsWith(QLatin1String("xl/theme/"))) {
            *bytes = it.value();
            return true;
        }
    return false;
}


// The same theme with one color and the heading font given, as a part of its own (a new buffer each time it is made).
Theme themeWith(const Theme &base, Theme::Color slot, const QColor &color, const QString &major = QString())
{
    Theme t = base;
    t.colors[slot] = color;
    if (!major.isEmpty()) t.majorLatin = major;
    t.xml.clear();
    t.xml = writeTheme(t);
    return t;
}

// A theme part of exactly `size` bytes (a comment makes up the rest).
QByteArray themePartOfSize(qint64 size)
{
    QByteArray part = writeTheme(ourTheme());
    const qint64 room = size - part.size() - 7;
    part.insert(part.indexOf("</a:theme>"), "<!--" + QByteArray(qsizetype(room), 'x') + "-->");
    return part;
}

// The workbook of the part given as its theme, opened as a package would be (the inflated count is the package's).
bool openPackage(const QByteArray &zip, office::Package *package, jp::office::sheet::Workbook *wb)
{
    return package->readLazy(zip) && wb->open(*package);
}

// A workbook whose first sheet holds a shape, an equation (the theme's own dark color), and charts, each chart in a part of its own.
QByteArray equationBook()
{
    const QString kMc = QStringLiteral("http://schemas.openxmlformats.org/markup-compatibility/2006");
    const QString kA14 = QStringLiteral("http://schemas.microsoft.com/office/drawing/2010/main");
    const QString kMath = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/math");
    const QString sp = QStringLiteral("<xdr:sp macro=\"\" textlink=\"\"><xdr:nvSpPr><xdr:cNvPr id=\"2\" name=\"Equation 1\"/><xdr:cNvSpPr txBox=\"1\"/></xdr:nvSpPr><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/>"
                                      "<a:ext cx=\"1905000\" cy=\"476250\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/></xdr:spPr><xdr:txBody><a:bodyPr wrap=\"none\" anchor=\"ctr\"/><a:lstStyle/>"
                                      "<a:p><a14:m xmlns:a14=\"%1\"><m:oMathPara xmlns:m=\"%2\"><m:oMath><m:r><m:t>x</m:t></m:r></m:oMath></m:oMathPara></a14:m><a:endParaRPr lang=\"en-US\"/></a:p></xdr:txBody></xdr:sp>")
                           .arg(kA14, kMath);
    const QString anchors = QStringLiteral("<mc:AlternateContent xmlns:mc=\"%1\"><mc:Choice xmlns:a14=\"%2\" Requires=\"a14\">%3</mc:Choice><mc:Fallback>%4</mc:Fallback></mc:AlternateContent>")
                                .arg(kMc, kA14, twoCell(1, 5, 4, 8, sp), twoCell(1, 5, 4, 8, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Equation 1"))));
    return bookWithDrawing({worksheet(QString())}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(40, 12, Qt::darkGreen))});
}

// Styles whose fonts and fills name theme colors that are not there (places below 0 or past 11).
QByteArray badThemeStyles()
{
    QString fonts = QStringLiteral("<fonts count=\"5\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>");
    QString fills = QStringLiteral("<fills count=\"6\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>");
    QString xfs = QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>");
    int n = 1;
    for (const char *index : {"-1", "12", "255", "4294967295"}) {
        fonts += QStringLiteral("<font><sz val=\"11\"/><color theme=\"%1\"/><name val=\"Calibri\"/></font>").arg(QLatin1String(index));
        fills += QStringLiteral("<fill><patternFill patternType=\"solid\"><fgColor theme=\"%1\"/><bgColor indexed=\"64\"/></patternFill></fill>").arg(QLatin1String(index));
        xfs += QStringLiteral("<xf numFmtId=\"0\" fontId=\"%1\" fillId=\"%2\" borderId=\"0\" applyFill=\"1\" applyFont=\"1\"/>").arg(n).arg(n + 1);
        ++n;
    }
    return stylesXml(QString(), xfs, fonts + QStringLiteral("</fonts>") + fills + QStringLiteral("</fills><borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"));
}

} // namespace

class CrunchThemeTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // A blank workbook has the default theme (a part, as a file keeps it), which is written when it is saved and read when it is opened again.
    void aBlankWorkbookHasTheDefaultThemeAndSavesIt()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->hasTheme());
        const Theme def = defaultTheme();
        QCOMPARE(s->theme().colors[Theme::Accent1], def.colors[Theme::Accent1]);
        QCOMPARE(s->theme().majorLatin, def.majorLatin);
        const QString path = e.folder.dir.filePath(QStringLiteral("blank.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path) && savedOk(saved));
        const QMap<QString, QByteArray> parts = partsOf(path);
        QVERIFY(parts.contains(QStringLiteral("xl/theme/theme1.xml")));
        Theme inFile;
        QVERIFY(readTheme(parts.value(QStringLiteral("xl/theme/theme1.xml")), &inFile));
        QCOMPARE(inFile.colors[Theme::Accent3], def.colors[Theme::Accent3]);
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("theme+xml"));
        EditWindow again;
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(again.open(f.readAll()));
        QVERIFY(again.settle());
        QVERIFY(again.win.w.session()->hasTheme());
        QCOMPARE(again.win.w.session()->theme().colors[Theme::Accent5], def.colors[Theme::Accent5]);
    }

    // The colors that name the theme stay theme references (with their tints) through an edit, the window's save, and opening again, and the theme part is kept as it was.
    void themeReferencesSurviveTheWindowsSaveAndOpen()
    {
        const Theme ours = ourTheme();
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        const QColor expected = tinted(ours.colors[Theme::Accent1], 0.39997558519241921);
        QCOMPARE(fillAt(e, QStringLiteral("A1")), expected);
        QCOMPARE(QColor::fromRgb(lookAt(e, QStringLiteral("B1")).textColor), ours.colors[Theme::Accent2]);
        e.select(QStringLiteral("D3"));
        e.enter(QStringLiteral("7"));
        // (a cell with a font that names the theme is made bold: the font is written again, as a new one, with its color)
        FormatChange bold;
        bold.bold = true;
        QVERIFY(e.win.w.session()->formatCells(0, rangeOf(QStringLiteral("B1")), bold, QStringLiteral("Bold")).ok);
        QVERIFY(saveNow(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        const QByteArray styles = parts.value(QStringLiteral("xl/styles.xml"));
        QVERIFY2(styles.contains("theme=\"4\"") && styles.contains("tint=\"0.399") && styles.contains("theme=\"5\""), styles.constData());
        QCOMPARE(styles.count("<color theme=\"5\"/>"), 2);
        QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("tabColor theme=\"7\""));
        QCOMPARE(parts.value(QStringLiteral("xl/theme/theme1.xml")), writeTheme(ours));
        EditWindow again;
        QVERIFY(again.win.open(e.copyOfFile()));
        QVERIFY(again.settle());
        QCOMPARE(fillAt(again, QStringLiteral("A1")), expected);
        QCOMPARE(QColor::fromRgb(lookAt(again, QStringLiteral("B1")).textColor), ours.colors[Theme::Accent2]);
    }

    // Colors and Fonts in the Page Layout tab: one step each, cells (fill, text color, the fonts that follow the theme) and the tab follow, Undo and Redo give the theme back, the saved file
    // has the new theme part and the cells still name the theme, and the file opens with it.
    void theMenusChangeTheThemeInOneStepAndTheCellsFollow()
    {
        const Theme ours = ourTheme();
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        auto *colors = e.win.w.findChild<QMenu *>(QStringLiteral("themeColorsMenu"));
        auto *fonts = e.win.w.findChild<QMenu *>(QStringLiteral("themeFontsMenu"));
        auto *themes = e.win.w.findChild<QMenu *>(QStringLiteral("themesMenu"));
        QVERIFY(colors && fonts && themes);
        QCOMPARE(lookAt(e, QStringLiteral("B1")).family, QStringLiteral("Body Face"));
        const ThemeColors set = builtinThemeColors().at(1);
        QVERIFY(!(themeColorsOf(ours) == set));
        const int steps = e.win.w.undoStack()->count();
        Q_EMIT colors->aboutToShow();
        QAction *pick = entry(colors, set.name);
        QVERIFY(pick);
        pick->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Theme Colors"));
        QCOMPARE(s->theme().colors[Theme::Accent1], set.colors[Theme::Accent1]);
        const QColor now = tinted(set.colors[Theme::Accent1], 0.39997558519241921);
        QCOMPARE(fillAt(e, QStringLiteral("A1")), now);
        QCOMPARE(QColor::fromRgb(lookAt(e, QStringLiteral("B1")).textColor), set.colors[Theme::Accent2]);
        QCOMPARE(s->theme().majorLatin, ours.majorLatin);   // (the fonts are not the colors' business)
        // the same set again is no step
        pick->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        // fonts: the cells that follow the theme's heading and body fonts take the new ones
        ThemeFonts pair;
        for (const ThemeFonts &f : builtinThemeFonts())
            if (f.body != QStringLiteral("Body Face") && f.heading != f.body) pair = f;
        QVERIFY(!pair.body.isEmpty());
        Q_EMIT fonts->aboutToShow();
        QAction *font = entry(fonts, pair.name);
        QVERIFY(font);
        font->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        QCOMPARE(lookAt(e, QStringLiteral("B1")).family, pair.body);
        QCOMPARE(lookAt(e, QStringLiteral("C1")).family, pair.heading);
        QCOMPARE(fillAt(e, QStringLiteral("A1")), now);   // (nor the fonts' business)
        // the saved file: the new part, the cells still naming the theme
        QVERIFY(saveNow(e));
        QMap<QString, QByteArray> parts = partsOf(e.path);
        Theme saved;
        QVERIFY(readTheme(parts.value(QStringLiteral("xl/theme/theme1.xml")), &saved));
        QCOMPARE(saved.colors[Theme::Accent1], set.colors[Theme::Accent1]);
        QCOMPARE(saved.majorLatin, pair.heading);
        QCOMPARE(saved.minorLatin, pair.body);
        QVERIFY(parts.value(QStringLiteral("xl/styles.xml")).contains("theme=\"4\""));
        EditWindow again;
        QVERIFY(again.win.open(e.copyOfFile()));
        QVERIFY(again.settle());
        QCOMPARE(fillAt(again, QStringLiteral("A1")), now);
        QCOMPARE(lookAt(again, QStringLiteral("B1")).family, pair.body);
        // Undo twice gives the first theme back, Redo takes the second; saving then puts the first part back
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QCOMPARE(fillAt(e, QStringLiteral("A1")), tinted(ours.colors[Theme::Accent1], 0.39997558519241921));
        QCOMPARE(lookAt(e, QStringLiteral("B1")).family, QStringLiteral("Body Face"));
        QVERIFY(saveNow(e));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/theme/theme1.xml")), writeTheme(ours));
        e.win.w.undoStack()->redo();
        QCOMPARE(fillAt(e, QStringLiteral("A1")), now);
        // a whole theme from the Themes menu: colors and fonts together, one step, and Reset gives the opened one back
        Q_EMIT themes->aboutToShow();
        const ThemePack pack = builtinThemePacks().at(2);
        QAction *whole = entry(themes, pack.name);
        QVERIFY(whole);
        const int before = e.win.w.undoStack()->index();
        whole->trigger();
        QCOMPARE(e.win.w.undoStack()->index(), before + 1);
        QCOMPARE(s->theme().colors[Theme::Accent2], pack.colors.colors[Theme::Accent2]);
        QCOMPARE(s->theme().minorLatin, pack.fonts.body);
        Q_EMIT themes->aboutToShow();
        QAction *reset = entry(themes, QStringLiteral("Reset"));
        QVERIFY(reset && reset->isEnabled());
        reset->trigger();
        QCOMPARE(s->theme().colors[Theme::Accent1], ours.colors[Theme::Accent1]);
        QCOMPARE(s->theme().minorLatin, ours.minorLatin);
    }

    // The shapes and charts the file gave, which name the theme's colors, are drawn with the new ones; the parts they came from are kept as they were.
    void shapesAndChartsThatNameThemeColorsFollowATheme()
    {
        const Theme ours = ourTheme();
        QString shape = shapeElement(2, QStringLiteral("Box"));
        shape.replace(QStringLiteral("<a:srgbClr val=\"3366CC\"/>"), QStringLiteral("<a:schemeClr val=\"accent1\"/>"));
        QByteArray chart = chartPart(QStringLiteral("barChart"), {SeriesSpec{QString(), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {QStringLiteral("Q1"), QStringLiteral("Q2"), QStringLiteral("Q3")}, {10, 20, 30}}});
        const int at = chart.indexOf("<c:cat>");
        QVERIFY(at > 0);
        chart.insert(at, "<c:spPr><a:solidFill><a:schemeClr val=\"accent2\"/></a:solidFill></c:spPr>");
        const QString anchors = twoCell(6, 2, 9, 6, shape) + twoCell(6, 8, 12, 14, chartFrame(3, QStringLiteral("rIdC"), QStringLiteral("Sales")));
        QByteArray book = bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chart)});
        // (a theme part for it)
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(book, parts));
        QVERIFY(!parts.contains(QStringLiteral("xl/theme/theme1.xml")));
        const QByteArray themed = withThemePart(book, ours);
        EditWindow e;
        QVERIFY(e.open(themed));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QTRY_VERIFY(o->shownSheet() == 0 && o->layer()->count() == 2);
        const std::vector<int> ids = o->idsOf(0);
        QCOMPARE(o->shapeFillOf(0, ids.front()), ours.colors[Theme::Accent1]);
        QCOMPARE(o->chartColorOf(0, ids.back()), ours.colors[Theme::Accent2]);
        const ThemeColors set = builtinThemeColors().at(1);
        auto *colors = e.win.w.findChild<QMenu *>(QStringLiteral("themeColorsMenu"));
        QVERIFY(colors);
        Q_EMIT colors->aboutToShow();
        QVERIFY(entry(colors, set.name));
        entry(colors, set.name)->trigger();
        QCOMPARE(o->shapeFillOf(0, ids.front()), set.colors[Theme::Accent1]);
        QCOMPARE(o->chartColorOf(0, ids.back()), set.colors[Theme::Accent2]);
        e.win.w.undoStack()->undo();
        QCOMPARE(o->shapeFillOf(0, ids.front()), ours.colors[Theme::Accent1]);
        QCOMPARE(o->chartColorOf(0, ids.back()), ours.colors[Theme::Accent2]);
        e.win.w.undoStack()->redo();
        QCOMPARE(o->shapeFillOf(0, ids.front()), set.colors[Theme::Accent1]);
        QVERIFY(saveNow(e));
        const QMap<QString, QByteArray> saved = partsOf(e.path);
        QVERIFY(saved.value(QStringLiteral("xl/drawings/drawing1.xml")).contains("schemeClr val=\"accent1\""));
        QVERIFY(saved.value(QStringLiteral("xl/charts/chart1.xml")).contains("schemeClr val=\"accent2\""));
        Theme inFile;
        QVERIFY(readTheme(saved.value(QStringLiteral("xl/theme/theme1.xml")), &inFile));
        QCOMPARE(inFile.colors[Theme::Accent1], set.colors[Theme::Accent1]);
    }

    // A workbook with no theme part takes none: no step, and the menus offer nothing.
    void aWorkbookWithNoThemeTakesNone()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles())));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(!s->hasTheme());
        const int steps = e.win.w.undoStack()->count();
        const crunch::EditSession::Outcome o = s->applyTheme(ourTheme(), QStringLiteral("Theme"));
        QVERIFY(!o.ok && !o.problem.isEmpty());
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        auto *colors = e.win.w.findChild<QMenu *>(QStringLiteral("themeColorsMenu"));
        QVERIFY(colors);
        Q_EMIT colors->aboutToShow();
        QVERIFY(colors->actions().isEmpty());
    }

    // A theme part larger than a theme may be is left as it is: the workbook has the usual colors and takes no theme.
    void aTooLargeThemePartIsKeptAndTakesNone()
    {
        Theme big = ourTheme();
        QByteArray part = writeTheme(big);
        part.insert(part.indexOf("</a:theme>"), "<!--" + QByteArray(kMaxThemePartBytes + 10, 'x') + "-->");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), part)));
        QVERIFY(e.settle());
        QVERIFY(!e.win.w.session()->hasTheme());
        e.select(QStringLiteral("D3"));
        e.enter(QStringLiteral("7"));
        QVERIFY(saveNow(e));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/theme/theme1.xml")), part);
    }

    // The color pickers store a theme color as a reference (the swatch's own color and tint), which follows a later theme; a plain color stays a plain color.
    void aPickedThemeColorIsStoredAsAThemeReference()
    {
        const Theme ours = ourTheme();
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        auto *fill = e.win.w.findChild<ColorButton *>(QStringLiteral("fmt.fillColor"));
        auto *font = e.win.w.findChild<ColorButton *>(QStringLiteral("fmt.fontColor"));
        QVERIFY(fill && font);
        e.select(QStringLiteral("E2"));
        Q_EMIT fill->colorPicked(ColorRef::scheme(Accent3, 60));
        QCOMPARE(fillAt(e, QStringLiteral("E2")), tinted(ours.colors[Theme::Accent3], 0.6));
        // (the swatch shows the color the cell comes to)
        QCOMPARE(ColorRef::scheme(Accent3, 60).resolve(e.win.w.themeColors()), fillAt(e, QStringLiteral("E2")));
        QCOMPARE(ColorRef::scheme(Accent3, 0, 30).resolve(e.win.w.themeColors()), tinted(ours.colors[Theme::Accent3], -0.3));
        e.select(QStringLiteral("F2"));
        Q_EMIT font->colorPicked(ColorRef::scheme(Accent4));
        QCOMPARE(QColor::fromRgb(lookAt(e, QStringLiteral("F2")).textColor), ours.colors[Theme::Accent4]);
        e.select(QStringLiteral("G2"));
        Q_EMIT fill->colorPicked(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        // a theme change moves the picked theme colors and leaves the plain one
        const ThemeColors set = builtinThemeColors().at(1);
        auto *colors = e.win.w.findChild<QMenu *>(QStringLiteral("themeColorsMenu"));
        Q_EMIT colors->aboutToShow();
        entry(colors, set.name)->trigger();
        QCOMPARE(fillAt(e, QStringLiteral("E2")), tinted(set.colors[Theme::Accent3], 0.6));
        QCOMPARE(QColor::fromRgb(lookAt(e, QStringLiteral("F2")).textColor), set.colors[Theme::Accent4]);
        QCOMPARE(fillAt(e, QStringLiteral("G2")), QColor(0x12, 0x34, 0x56));
        QVERIFY(saveNow(e));
        const QByteArray styles = partsOf(e.path).value(QStringLiteral("xl/styles.xml"));
        QVERIFY2(styles.contains("theme=\"6\"") && styles.contains("tint=\"0.6\"") && styles.contains("theme=\"7\"") && styles.contains("FF123456"), styles.constData());
    }

    // A theme part that was not changed here is written back byte for byte, even one with a byte order mark.
    void anUnchangedThemePartIsWrittenBackAsItWas()
    {
        const QByteArray part = QByteArray("\xEF\xBB\xBF") + writeTheme(ourTheme());
        EditWindow e;
        QVERIFY(e.open(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), part)));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->hasTheme());
        e.select(QStringLiteral("D3"));
        e.enter(QStringLiteral("7"));
        QVERIFY(saveNow(e));
        QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/theme/theme1.xml")), part);
    }

    // The older open-document format has no theme: a save writes the colors as they are drawn.
    void theOpenDocumentFormatGetsTheResolvedColors()
    {
        const Theme ours = ourTheme();
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("copy.ods"));
        QSignalSpy done(&e.win.w, &CrunchWindow::copyExported);
        QVERIFY(e.win.w.saveAs(path, true));
        QVERIFY(done.wait(60000) || !done.isEmpty());
        QVERIFY(done.last().value(1).toBool());
        const QByteArray content = partsOf(path).value(QStringLiteral("content.xml"));
        const QString hex = tinted(ours.colors[Theme::Accent1], 0.39997558519241921).name();
        QVERIFY2(content.toLower().contains(hex.toLatin1()), qPrintable(hex));
        QVERIFY(!content.contains("theme=\""));
    }

    // A theme part is looked at by the size it says before it is inflated: one of exactly the most a theme part may be is kept and used, one byte more is left as it is (the usual
    // colors, no theme to change, the file's part written back as it was), and one that says it is far more is never inflated.
    void aThemePartIsSizedBeforeItIsInflated()
    {
        const Theme ours = ourTheme();
        const QByteArray atLimit = themePartOfSize(kMaxThemePartBytes), over = themePartOfSize(kMaxThemePartBytes + 1);
        QCOMPARE(qint64(atLimit.size()), kMaxThemePartBytes);
        QCOMPARE(qint64(over.size()), kMaxThemePartBytes + 1);
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), atLimit)));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->hasTheme());
            QCOMPARE(fillAt(e, QStringLiteral("A1")), tinted(ours.colors[Theme::Accent1], 0.39997558519241921));
        }
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), over)));
            QVERIFY(e.settle());
            QVERIFY(!e.win.w.session()->hasTheme());
            QVERIFY(fillAt(e, QStringLiteral("A1")) != tinted(ours.colors[Theme::Accent1], 0.39997558519241921));   // (the usual colors)
            e.select(QStringLiteral("D3"));
            e.enter(QStringLiteral("7"));
            QVERIFY(saveNow(e));
            QCOMPARE(partsOf(e.path).value(QStringLiteral("xl/theme/theme1.xml")), over);
        }
        // a small entry that inflates to many times what a theme part may be: the workbook opens, the part is not inflated
        QByteArray bomb = writeTheme(ours);
        bomb.insert(bomb.indexOf("</a:theme>"), "<!--" + QByteArray(qsizetype(kMaxThemePartBytes) * 5, 'x') + "-->");
        const QByteArray zip = buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), bomb);
        QVERIFY(zip.size() < kMaxThemePartBytes / 4);   // (it is small)
        office::Package package;
        jp::office::sheet::Workbook wb;
        QVERIFY(openPackage(zip, &package, &wb));
        QVERIFY(!wb.hasTheme());
        QVERIFY2(package.inflatedBytes() < kMaxThemePartBytes, qPrintable(QString::number(package.inflatedBytes())));
    }

    // A font of the font scheme is shown, and drawn, in the theme's font; the .xlsx keeps the name and the scheme it has; the open-document and old binary copies are written in the font that is
    // shown; Replace Fonts finds a font by what is shown, and a cell that is given a font by name stops following the theme (even by the name the file stores).
    void aThemeFontIsShownAndWrittenAsEachFormatSaysIt()
    {
        const Theme ours = ourTheme();
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        QCOMPARE(lookAt(e, QStringLiteral("B1")).family, QStringLiteral("Body Face"));
        QCOMPARE(lookAt(e, QStringLiteral("C1")).family, QStringLiteral("Head Face"));
        e.select(QStringLiteral("D3"));
        e.enter(QStringLiteral("7"));
        // the .xlsx: the stored name and the scheme, as read
        QVERIFY(saveNow(e));
        const QByteArray styles = partsOf(e.path).value(QStringLiteral("xl/styles.xml"));
        QVERIFY2(styles.contains("<name val=\"Calibri\"/>") && styles.contains("<scheme val=\"minor\"/>") && styles.contains("<name val=\"Calibri Light\"/>") && styles.contains("<scheme val=\"major\"/>"), styles.constData());
        QVERIFY(!styles.contains("Body Face") && !styles.contains("Head Face"));
        // the copies in the other formats: the font that is shown
        const QString ods = e.folder.dir.filePath(QStringLiteral("copy.ods")), xls = e.folder.dir.filePath(QStringLiteral("copy.xls"));
        for (const QString &path : {ods, xls}) {
            QSignalSpy done(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(path, true));
            QVERIFY(done.wait(60000) || !done.isEmpty());
            QVERIFY(done.last().value(1).toBool());
        }
        const QByteArray content = partsOf(ods).value(QStringLiteral("content.xml"));
        QVERIFY(content.contains("Body Face") && content.contains("Head Face"));
        QVERIFY(!content.contains("Calibri Light"));
        jp::office::sheet::Workbook old;
        QVERIFY(old.openFile(xls));
        QStringList names;
        for (const auto &f : old.styles().fonts) names << f.name;
        QVERIFY2(names.contains(QStringLiteral("Body Face")) && names.contains(QStringLiteral("Head Face")) && !names.contains(QStringLiteral("Calibri Light")), qPrintable(names.join(',')));
        // Replace Fonts goes by what is shown: the stored name finds only the cells that are set in it
        int cells = 0;
        QVERIFY(e.win.w.session()->replaceFont(QStringLiteral("Calibri Light"), QStringLiteral("Arial"), QStringLiteral("Replace Font"), &cells).ok);
        QCOMPARE(cells, 0);
        QCOMPARE(lookAt(e, QStringLiteral("C1")).family, QStringLiteral("Head Face"));
        // the shown name is replaced by the very name the file stores: the cell is that font from now on, and a later theme leaves it
        QVERIFY(e.win.w.session()->replaceFont(QStringLiteral("Body Face"), QStringLiteral("Calibri"), QStringLiteral("Replace Font"), &cells).ok);
        QCOMPARE(cells, 2);
        QCOMPARE(lookAt(e, QStringLiteral("B1")).family, QStringLiteral("Calibri"));
        const office::ThemeFonts fonts = builtinThemeFonts().at(1);
        auto *menu = e.win.w.findChild<QMenu *>(QStringLiteral("themeFontsMenu"));
        QVERIFY(menu);
        Q_EMIT menu->aboutToShow();
        QVERIFY(entry(menu, fonts.name));
        entry(menu, fonts.name)->trigger();
        QCOMPARE(lookAt(e, QStringLiteral("B1")).family, QStringLiteral("Calibri"));
        QVERIFY(lookAt(e, QStringLiteral("C1")).family != QStringLiteral("Head Face"));
        QVERIFY(saveNow(e));
        EditWindow again;
        QVERIFY(again.win.open(e.copyOfFile()));
        QVERIFY(again.settle());
        QCOMPARE(lookAt(again, QStringLiteral("B1")).family, QStringLiteral("Calibri"));
    }

    // A tinted theme color picked for a shape is kept as that color of the theme, is drawn as the swatch showed it, and follows a later theme, through the window's save and open.
    void aShapeGivenATintedThemeColorDrawsTheSwatchAndFollowsTheTheme()
    {
        const Theme ours = ourTheme();
        const QByteArray book = withThemePart(bookWithDrawing({dataSheet()}, drawingPart(QString()), {}), ours);
        EditWindow e;
        QVERIFY(e.open(book));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QTRY_VERIFY(o->shownSheet() == 0);
        QVERIFY(e.win.w.insertShape(QStringLiteral("rect")));
        const int id = o->layer()->selectedId();
        const ColorRef lighter = ColorRef::scheme(Accent1, 40), darker = ColorRef::scheme(Accent2, 0, 30);
        QVERIFY(e.win.w.setObjectFill(lighter));
        QCOMPARE(o->shapeFillOf(0, o->layer()->selectedId()), lighter.resolve(e.win.w.themeColors()));
        QCOMPARE(lighter.resolve(e.win.w.themeColors()), tinted(ours.colors[Theme::Accent1], 0.4));
        QVERIFY(e.win.w.setObjectOutline(darker));
        const ThemeColors set = builtinThemeColors().at(1);
        auto *colors = e.win.w.findChild<QMenu *>(QStringLiteral("themeColorsMenu"));
        QVERIFY(colors);
        Q_EMIT colors->aboutToShow();
        entry(colors, set.name)->trigger();
        // (a shape made here has the colors it was given until the workbook is saved and opened: it follows the theme then)
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->redo();
        QVERIFY(saveNow(e));
        EditWindow again;
        QVERIFY(again.win.open(e.copyOfFile()));
        QVERIFY(again.settle());
        crunch::SheetObjects *o2 = again.win.w.objects();
        QTRY_VERIFY(o2->shownSheet() == 0 && !o2->idsOf(0).empty());
        QCOMPARE(o2->shapeFillOf(0, o2->idsOf(0).front()), tinted(set.colors[Theme::Accent1], 0.4));
        Q_UNUSED(id);
    }

    // An equation is drawn in the workbook's dark color (not a built-in one), and in the new one after a theme change.
    void anEquationIsDrawnInTheWorkbooksColorAndFollowsTheTheme()
    {
        const Theme ours = themeWith(ourTheme(), Theme::Dark1, QColor(0x20, 0x30, 0x40));
        EditWindow e;
        QVERIFY(e.open(withThemePart(equationBook(), ours)));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QTRY_VERIFY(o->shownSheet() == 0 && !o->idsOf(0).empty());
        const int id = o->idsOf(0).front();
        QCOMPARE(o->equationColorOf(0, id), QColor(0x20, 0x30, 0x40));
        const Theme next = themeWith(ours, Theme::Dark1, QColor(0x11, 0x88, 0x33));
        QVERIFY(e.win.w.session()->applyTheme(next, QStringLiteral("Theme")).ok);
        QCOMPARE(o->equationColorOf(0, id), QColor(0x11, 0x88, 0x33));
    }

    // The gallery of cell styles shows the theme's colors as they are now (it is made as the menu opens), and the link styles take the theme's hyperlink colors.
    void theCellStyleGalleryAndTheLinkStylesFollowTheTheme()
    {
        const Theme ours = themeWith(ourTheme(), Theme::Hyperlink, QColor(0x11, 0x22, 0x33));
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
        QVERIFY(gallery);
        auto swatch = [&](const QString &name) {
            Q_EMIT gallery->aboutToShow();
            QAction *a = entry(gallery, name);
            return a ? a->icon().pixmap(44, 22).toImage().pixelColor(4, 4) : QColor();
        };
        QCOMPARE(swatch(QStringLiteral("Accent1")), ours.colors[Theme::Accent1]);
        const Theme next = themeWith(ours, Theme::Accent1, QColor(0x90, 0x20, 0x20));
        QVERIFY(e.win.w.session()->applyTheme(next, QStringLiteral("Theme")).ok);
        QCOMPARE(swatch(QStringLiteral("Accent1")), QColor(0x90, 0x20, 0x20));
        // (the link style is the hyperlink slot)
        auto linkText = [](const Theme &t) {
            office::Package package;
            jp::office::sheet::Workbook wb;
            const QByteArray zip = buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), t.xml);
            if (!openPackage(zip, &package, &wb)) return QColor();
            for (const crunch::StyleSample &s : crunch::styleSamples(wb.styles()))
                if (s.name == QLatin1String("Hyperlink")) return QColor::fromRgb(s.textRgb);
            return QColor();
        };
        QCOMPARE(linkText(ours), QColor(0x11, 0x22, 0x33));
        QCOMPARE(linkText(themeWith(ours, Theme::Hyperlink, QColor(0x44, 0x55, 0x66))), QColor(0x44, 0x55, 0x66));
    }

    // A file's charts are not read when the theme changes: each is read once, with the theme as it is then, when it is next wanted; the theme part is parsed once for each change.
    void chartsAreRecoloredWhenWantedNotWhenTheThemeChanges()
    {
        const Theme ours = ourTheme();
        QString anchors;
        QVector<DrawingTarget> targets;
        for (int i = 1; i <= 5; ++i) {
            QByteArray chart = chartPart(QStringLiteral("barChart"), {SeriesSpec{QString(), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {QStringLiteral("Q1"), QStringLiteral("Q2"), QStringLiteral("Q3")}, {10, 20, 30}}});
            chart.insert(chart.indexOf("<c:cat>"), "<c:spPr><a:solidFill><a:schemeClr val=\"accent2\"/></a:solidFill></c:spPr>");
            anchors += twoCell(6, i * 3, 9, i * 3 + 2, chartFrame(i + 1, QStringLiteral("rIdC%1").arg(i), QStringLiteral("Chart %1").arg(i)));
            targets << chartTarget(QStringLiteral("rIdC%1").arg(i), i, chart);
        }
        EditWindow e;
        QVERIFY(e.open(withThemePart(bookWithDrawing({dataSheet()}, drawingPart(anchors), targets), ours)));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QTRY_VERIFY(o->shownSheet() == 0 && o->layer()->count() == 5);
        const std::vector<int> ids = o->idsOf(0);
        for (int id : ids) QCOMPARE(o->chartColorOf(0, id), ours.colors[Theme::Accent2]);
        const int reads = o->chartPartsRead(), parses = o->themeParses();
        crunch::EditSession *s = e.win.w.session();
        Theme t = ours;
        for (int i = 0; i < 3; ++i) {
            t = themeWith(t, Theme::Accent2, QColor(0x10 * (i + 1), 0x80, 0x40));
            QVERIFY(s->applyTheme(t, QStringLiteral("Theme")).ok);
        }
        QCOMPARE(o->themeParses() - parses, 3);          // (once for each change)
        QCOMPARE(o->chartPartsRead(), reads);            // (no chart part was read by a change)
        QCOMPARE(o->chartColorOf(0, ids.front()), t.colors[Theme::Accent2]);
        QCOMPARE(o->chartPartsRead() - reads, 1);
        for (int id : ids) QCOMPARE(o->chartColorOf(0, id), t.colors[Theme::Accent2]);
        QCOMPARE(o->chartPartsRead() - reads, 5);        // (one read for each chart, for three changes)
        e.win.w.undoStack()->undo();
        QCOMPARE(o->chartPartsRead() - reads, 5);
        QCOMPARE(o->chartColorOf(0, ids.back()), QColor(0x20, 0x80, 0x40));
        QCOMPARE(o->chartPartsRead() - reads, 6);
    }

    // The steps of a theme change hold each distinct theme's part once: many steps between two themes hold two parts; Undo and Redo give the themes back.
    void manyThemeStepsBetweenTwoThemesHoldTwoParts()
    {
        const Theme a = ourTheme();
        EditWindow e;
        QVERIFY(e.open(themedBook(a)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const int steps = e.win.w.undoStack()->count();
        const QColor other(0x44, 0x88, 0xCC);
        for (int i = 0; i < 12; ++i)
            QVERIFY(s->applyTheme(i % 2 == 0 ? themeWith(a, Theme::Accent3, other) : themeWith(a, Theme::Accent3, a.colors[Theme::Accent3]), QStringLiteral("Theme")).ok);
        QCOMPARE(e.win.w.undoStack()->count() - steps, 12);
        QCOMPARE(s->themePartsHeld(), 2);
        while (e.win.w.undoStack()->canUndo() && e.win.w.undoStack()->count() > steps) e.win.w.undoStack()->undo();
        QCOMPARE(s->theme().colors[Theme::Accent3], a.colors[Theme::Accent3]);
        while (e.win.w.undoStack()->canRedo()) e.win.w.undoStack()->redo();
        QCOMPARE(s->theme().colors[Theme::Accent3], a.colors[Theme::Accent3]);
        QCOMPARE(s->themePartsHeld(), 2);
    }

    // A workbook keeps its theme in memory when a save wrote a package that has no theme part, so that a later save writes it; a copy in another format leaves the open workbook as it is.
    void aWorkbookKeepsItsThemeWhenTheFileWrittenHasNone()
    {
        const Theme ours = ourTheme();
        {
            office::Package package;
            jp::office::sheet::Workbook wb;
            QVERIFY(openPackage(themedBook(ours), &package, &wb));
            QVERIFY(wb.hasTheme());
            office::Package without;
            QVERIFY(without.readLazy(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles())));
            wb.savedTo(without, jp::office::sheet::Workbook::SavedCounts());
            QVERIFY(wb.hasTheme());
            QCOMPARE(wb.theme().colors[Theme::Accent1], ours.colors[Theme::Accent1]);
        }
        EditWindow e;
        QVERIFY(e.open(themedBook(ours)));
        QVERIFY(e.settle());
        const QString ods = e.folder.dir.filePath(QStringLiteral("copy.ods"));
        QSignalSpy done(&e.win.w, &CrunchWindow::copyExported);
        QVERIFY(e.win.w.saveAs(ods, true));
        QVERIFY(done.wait(60000) || !done.isEmpty());
        QVERIFY(e.win.w.session()->hasTheme());
        QVERIFY(e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("again.xlsx"))));
        QTRY_VERIFY(!e.win.w.isSaving());
        QVERIFY(e.win.w.session()->hasTheme());
        QVERIFY(partsOf(e.folder.dir.filePath(QStringLiteral("again.xlsx"))).contains(QStringLiteral("xl/theme/theme1.xml")));
    }

    // A color that names a theme place that is not there (below 0, or past the twelve) is the automatic color, and does not stop the workbook from opening.
    void aThemePlaceThatIsNotThereIsAutomatic()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"1\"><v>1</v></c><c r=\"B1\" s=\"2\"><v>1</v></c><c r=\"C1\" s=\"3\"><v>1</v></c><c r=\"D1\" s=\"4\"><v>1</v></c><c r=\"E1\"><v>1</v></c></row>"))},
                                 QByteArray(), QString(), QString(), badThemeStyles(), QStringList(), QString(), writeTheme(ourTheme()))));
        QVERIFY(e.settle());
        const quint32 automatic = lookAt(e, QStringLiteral("E1")).textColor;
        for (const char *ref : {"A1", "B1", "C1", "D1"}) {
            QCOMPARE(lookAt(e, QString::fromLatin1(ref)).textColor, automatic);
            QVERIFY(fillAt(e, QString::fromLatin1(ref)).isValid());
        }
        jp::office::sheet::Styles st;
        for (qint32 index : {qint32(-1), qint32(12), qint32(255), qint32(0x7FFFFFFF)}) {
            jp::office::sheet::ColorSpec c;
            c.kind = jp::office::sheet::ColorSpec::Kind::Theme;
            c.index = index;
            QCOMPARE(st.resolve(c, 0xABCDEFu), 0xABCDEFu);
        }
    }

    // The tint of a theme color, by hand (lightness moved toward white by a tint above 0, toward black below 0; hue and saturation kept):
    //  black +0.4: lightness 0 -> 0.4, gray: 0.4 * 255 = 102 = 0x66, so #666666. White -0.25: lightness 1 -> 0.75: 191.25 -> 191 = 0xBF, so #BFBFBF.
    //  Red (#FF0000, hue 0, saturation 1, lightness 0.5) +0.4: lightness 0.5 * 0.6 + 0.4 = 0.7, chroma 0.6, so the high channel is 1 and the other two 0.4 -> #FF6666; -0.4: lightness 0.3,
    //  chroma 0.6, so the high channel is 0.6 * 255 = 153 -> #990000.
    //  #003366 (hue 210, saturation 1, lightness 0.2) +0.2: lightness 0.2 * 0.8 + 0.2 = 0.36, chroma 0.72, so the channels are 0, 0.36, 0.72 -> 0, 91.8, 183.6 -> #005CB8 (mixing in white would
    //  give #335C85); -0.4: lightness 0.12, chroma 0.24, channels 0, 0.12, 0.24 -> 0, 30.6, 61.2 -> #001F3D.
    void aTintIsMadeAsTheLibraryHasIt()
    {
        using jp::office::sheet::tintedRgb;
        QCOMPARE(tintedRgb(0x000000, 0.4), 0x666666u);
        QCOMPARE(tintedRgb(0xFFFFFF, -0.25), 0xBFBFBFu);
        QCOMPARE(tintedRgb(0xFF0000, 0.4), 0xFF6666u);
        QCOMPARE(tintedRgb(0xFF0000, -0.4), 0x990000u);
        QCOMPARE(tintedRgb(0x003366, 0.2), 0x005CB8u);
        QCOMPARE(tintedRgb(0x003366, -0.4), 0x001F3Du);
        QCOMPARE(tintedRgb(0x003366, 0), 0x003366u);
    }

    // A scheme says how a tint is made (the shared color code): a workbook's picker has them as the library makes them (the constants of the test above), and a scheme that says nothing mixes in
    // white or black.
    void aColorSchemeCanSayHowATintIsMade()
    {
        ColorScheme plain = builtinColorSchemes().first();
        const QColor base = plain.c[Accent1];
        QCOMPARE(ColorRef::scheme(Accent1, 40).resolve(plain), mix(base, Qt::white, 0.4));
        QVERIFY(!plain.shade);
        jp::office::sheet::Styles st;
        st.theme[4] = 0x003366;
        const ColorScheme own = crunch::workbookColorScheme(st);
        QVERIFY(own.shade);
        QCOMPARE(ColorRef::scheme(Accent1, 20).resolve(own), QColor(0x00, 0x5C, 0xB8));
        QCOMPARE(ColorRef::scheme(Accent1, 0, 40).resolve(own), QColor(0x00, 0x1F, 0x3D));
        QCOMPARE(ColorRef::scheme(Accent1).resolve(own), QColor(0x00, 0x33, 0x66));   // (no tint: the color itself)
        QVERIFY(ColorRef::scheme(Accent1, 20).resolve(own) != mix(QColor(0x00, 0x33, 0x66), Qt::white, 0.2));
    }

    // The sweep of real workbooks (JO_THEME_SWEEP_LIST: a file of paths, JO_THEME_SWEEP_OUT: where the lines go): each is opened, edited, saved by the window, and opened again; then given another
    // set of theme colors, saved, and opened again. Skipped when the list is not given.
    void realWorkbooksKeepTheirThemeThroughTheWindow()
    {
        const QString list = qEnvironmentVariable("JO_THEME_SWEEP_LIST");
        if (list.isEmpty()) QSKIP("JO_THEME_SWEEP_LIST is not set");
        QFile in(list);
        QVERIFY(in.open(QIODevice::ReadOnly));
        const QStringList paths = QString::fromUtf8(in.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QFile out(qEnvironmentVariable("JO_THEME_SWEEP_OUT"));
        QVERIFY(out.open(QIODevice::WriteOnly | QIODevice::Truncate));
        static const Theme::Color order[kThemeColors] = {Theme::Light1, Theme::Dark1, Theme::Light2, Theme::Dark2, Theme::Accent1, Theme::Accent2,
                                                         Theme::Accent3, Theme::Accent4, Theme::Accent5, Theme::Accent6, Theme::Hyperlink, Theme::FollowedHyperlink};
        const ThemeColors set = builtinThemeColors().at(1);
        for (const QString &path : paths) {
            QString line = QFileInfo(path).fileName() + QLatin1Char(' ');
            EditWindow e;
            if (!e.win.open(path) || !e.settle()) {
                out.write((line + QStringLiteral("OPENFAIL\n")).toUtf8());
                continue;
            }
            const bool hasTheme = e.win.w.session()->hasTheme();
            const QMap<QString, ThemeUse> before = themeColorsOf(e.win.w.loaded()->book->styles());
            QByteArray themeBefore;
            const bool partBefore = themePartBytes(path, &themeBefore);
            // an edit far from the cells, and the window's own save
            e.select(QStringLiteral("XFA1"));
            e.enter(QStringLiteral("1"));
            const QString saved = e.folder.dir.filePath(QStringLiteral("saved.xlsx"));
            QSignalSpy first(&e.win.w, &CrunchWindow::saved);
            if (!e.win.w.saveAs(saved) || !savedOk(first)) {
                out.write((line + QStringLiteral("SAVEREFUSED part=%1\n").arg(partBefore ? 1 : 0)).toUtf8());
                continue;
            }
            e.path = saved;
            QByteArray themeAfter;
            const bool partAfter = themePartBytes(saved, &themeAfter);
            const QString copy = e.copyOfFile();
            EditWindow again;
            if (!again.win.open(copy) || !again.settle()) {
                out.write((line + QStringLiteral("REOPENFAIL\n")).toUtf8());
                continue;
            }
            const QMap<QString, ThemeUse> after = themeColorsOf(again.win.w.loaded()->book->styles());
            int same = 0, differ = 0;
            for (auto it = before.begin(); it != before.end(); ++it) {
                const auto found = after.constFind(it.key());
                if (found != after.constEnd() && found->index == it->index && found->tint == it->tint && found->rgb == it->rgb) ++same;
                else ++differ;
            }
            // another set of theme colors: the references stay, and come to the new colors
            int movedOk = 0, movedBad = 0;
            QString second = QStringLiteral("-");
            if (hasTheme) {
                const crunch::EditSession::Outcome o = e.win.w.session()->applyTheme(withThemeColors(e.win.w.session()->theme(), set), QStringLiteral("Theme Colors"));
                QSignalSpy again2(&e.win.w, &CrunchWindow::saved);
                if (!o.ok || !e.win.w.save() || !savedOk(again2)) {
                    second = QStringLiteral("SECONDSAVEFAIL");
                } else {
                    EditWindow third;
                    if (!third.win.open(e.copyOfFile()) || !third.settle()) {
                        second = QStringLiteral("THIRDOPENFAIL");
                    } else {
                        second = QStringLiteral("ok");
                        const QMap<QString, ThemeUse> moved = themeColorsOf(third.win.w.loaded()->book->styles());
                        for (auto it = before.begin(); it != before.end(); ++it) {
                            const auto found = moved.constFind(it.key());
                            const quint32 expect = tintedRgb(set.colors[order[it->index]].rgb() & 0xFFFFFF, it->tint);
                            if (found != moved.constEnd() && found->index == it->index && found->tint == it->tint && found->rgb == expect) ++movedOk;
                            else ++movedBad;
                        }
                    }
                }
            }
            line += QStringLiteral("theme=%1 partBefore=%2 partKept=%3 refs=%4 same=%5 differ=%6 second=%7 movedOk=%8 movedBad=%9\n")
                        .arg(hasTheme ? 1 : 0).arg(partBefore ? 1 : 0).arg(partAfter && (!partBefore || themeAfter == themeBefore) ? 1 : 0).arg(before.size()).arg(same).arg(differ).arg(second).arg(movedOk).arg(movedBad);
            out.write(line.toUtf8());
            out.flush();
        }
    }
};

JP_TEST_CLASS(CrunchThemeTests)
#include "tst_crunch_themes.moc"
