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
#include "office/themelib.h"
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
        QVERIFY(saveNow(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        const QByteArray styles = parts.value(QStringLiteral("xl/styles.xml"));
        QVERIFY2(styles.contains("theme=\"4\"") && styles.contains("tint=\"0.399") && styles.contains("theme=\"5\""), styles.constData());
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

    // A theme part larger than a theme may be is left as it is: the workbook reads its colors but takes no other theme.
    void aTooLargeThemePartIsKeptAndTakesNone()
    {
        Theme big = ourTheme();
        QByteArray part = writeTheme(big);
        part.insert(part.indexOf("</a:theme>"), "<!--" + QByteArray(kMaxThemePartBytes + 10, 'x') + "-->");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({themedSheet()}, QByteArray(), QString(), QString(), themedStyles(), QStringList(), QString(), part)));
        QVERIFY(e.settle());
        QVERIFY(!e.win.w.session()->hasTheme());
        QCOMPARE(fillAt(e, QStringLiteral("A1")), tinted(big.colors[Theme::Accent1], 0.39997558519241921));
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

    // A scheme can say how a tint is made (the shared color code): the picker of a workbook has its tints as a file does, and a scheme that says nothing mixes in white or black as before.
    void aColorSchemeCanSayHowATintIsMade()
    {
        ColorScheme plain = builtinColorSchemes().first();
        const QColor base = plain.c[Accent1];
        QCOMPARE(ColorRef::scheme(Accent1, 40).resolve(plain), mix(base, Qt::white, 0.4));
        ColorScheme own = plain;
        own.shade = [](const QColor &, int lighten, int darken) { return QColor(lighten, darken, 7); };
        QCOMPARE(ColorRef::scheme(Accent1, 40).resolve(own), QColor(40, 0, 7));
        QCOMPARE(ColorRef::scheme(Accent1, 0, 30).resolve(own), QColor(0, 30, 7));
        QCOMPARE(ColorRef::scheme(Accent1).resolve(own), base);   // (no tint: the color itself)
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
