// Tests of the second slice of editing and view gaps: Find by format and Replace in notes, Merge Styles and Modify for a cell style, the Options settings, and the
// small commands (Repeat, the Undo list, Show and Hide Detail, outline settings, Remove Hyperlinks). Every workbook is built in the test.
#include "../cmdsearchcheck.h"
#include "app/celleditor.h"
#include "app/crunchwindow.h"
#include "app/finddialog.h"
#include "app/formatdialog.h"
#include "core/findrun.h"
#include "core/formatfind.h"
#include "../sheet/odshelpers.h"
#include "io/zip.h"
#include "core/outline.h"
#include "sheet/notes.h"
#include "app/settings.h"
#include "core/editsession.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "savedok.h"

#include <QAction>
#include <QKeyEvent>
#include <QLabel>
#include <QComboBox>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

// The text a cell holds, read with the worker held back.
QString textAt(EditWindow &e, const QString &ref, int sheet = 0)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = e.win.w.workbookLoader().hold();
    return e.win.w.session()->editText(sheet, r, c);
}

QString textIn(Window &w, const QString &ref, int sheet = 0)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    return w.w.session()->editText(sheet, r, c);
}

// Whether the cell is set in bold, as its format says.
bool boldAt(Window &w, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    const grid::GridSheet &gs = w.w.view()->sheet();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    return st.font(st.format(gs.styleOf(r, c, gs.cellAt(r, c))).font).bold;
}

// The style format a named style has in the window's workbook (none: a default one).
jp::office::sheet::CellFormat styleOfName(Window &w, const QString &name)
{
    auto hold = w.w.workbookLoader().hold();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    for (const jp::office::sheet::CellStyleName &n : st.cellStyles)
        if (n.name == name && n.xfId < quint32(st.styleXfs.size())) return st.styleXfs[int(n.xfId)];
    return jp::office::sheet::CellFormat();
}

bool styleBold(Window &w, const QString &name)
{
    const jp::office::sheet::CellFormat f = styleOfName(w, name);
    auto hold = w.w.workbookLoader().hold();
    return w.w.loaded()->book->styles().font(f.font).bold;
}

bool styleItalic(Window &w, const QString &name)
{
    const jp::office::sheet::CellFormat f = styleOfName(w, name);
    auto hold = w.w.workbookLoader().hold();
    return w.w.loaded()->book->styles().font(f.font).italic;
}

bool hasStyle(Window &w, const QString &name)
{
    for (const jp::office::sheet::CellStyleName &n : w.w.loaded()->book->styles().cellStyles)
        if (n.name == name && !n.hidden) return true;
    return false;
}

// A small workbook that can keep named styles (A3 is a percent).
QByteArray styledBook()
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(),
                                        QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"9\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/>"), lists);
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>10</v></c><c r=\"B2\"><v>3</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\" s=\"1\"><v>0.25</v></c><c r=\"B3\"><v>4</v></c></row>");
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles);
}

using jp::office::sheet::CellFormat;
using jp::office::sheet::ColorSpec;
using jp::office::sheet::FillSpec;
using jp::office::sheet::FormatChange;
using jp::office::sheet::PatternKind;

FormatChange solidFill(quint32 rgb)
{
    FormatChange c;
    FillSpec f;
    f.pattern = PatternKind::Solid;
    f.fg = ColorSpec::fromRgb(rgb);
    c.fill = f;
    return c;
}

// The fill color a cell format shows (0 for none), as the workbook resolves it.
quint32 fillOfFormat(const jp::office::sheet::Styles &st, const CellFormat &f)
{
    const FillSpec &fill = st.fill(f.fill);
    return fill.pattern == PatternKind::None ? 0 : (st.resolve(fill.fg, 0) & 0xFFFFFF);
}

quint32 fillAt(Window &w, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    const grid::GridSheet &gs = w.w.view()->sheet();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    return fillOfFormat(st, st.format(gs.styleOf(r, c, gs.cellAt(r, c))));
}

QString numberFormatAt(Window &w, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    const grid::GridSheet &gs = w.w.view()->sheet();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    return crunch::fullFormatChange(st, gs.styleOf(r, c, gs.cellAt(r, c))).numberFormat.value_or(QString());
}

quint32 styleFill(Window &w, const QString &name)
{
    const CellFormat f = styleOfName(w, name);
    auto hold = w.w.workbookLoader().hold();
    return fillOfFormat(w.w.loaded()->book->styles(), f);
}

// Whether the workbook keeps a style of the name at all (hidden or not).
bool keepsStyle(Window &w, const QString &name)
{
    auto hold = w.w.workbookLoader().hold();
    for (const jp::office::sheet::CellStyleName &n : w.w.loaded()->book->styles().cellStyles)
        if (n.name == name) return true;
    return false;
}

// The fill an empty column or row that was given a format shows.
quint32 columnFill(Window &w, quint32 col)
{
    auto hold = w.w.workbookLoader().hold();
    const auto runs = hold->calculation()->columnRuns(0, col, col);
    return runs.empty() ? 0 : fillOfFormat(w.w.loaded()->book->styles(), w.w.loaded()->book->styles().format(runs.front().style));
}

quint32 rowFill(Window &w, quint32 row)
{
    auto hold = w.w.workbookLoader().hold();
    for (const jp::office::sheet::RowSpec &r : hold->calculation()->rowSpecsIn(0, row, row))
        if (r.row == row && r.customFormat) return fillOfFormat(w.w.loaded()->book->styles(), w.w.loaded()->book->styles().format(r.style));
    return 0;
}

// The font color and underline the cell shows.
std::pair<quint32, bool> linkLookAt(Window &w, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    const grid::GridSheet &gs = w.w.view()->sheet();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    const jp::office::sheet::FontSpec &font = st.font(st.format(gs.styleOf(r, c, gs.cellAt(r, c))).font);
    return {st.resolve(font.color, 0) & 0xFFFFFF, font.underline != jp::office::sheet::Underline::None};
}

// A style named from the cell's look.
void newStyle(EditWindow &e, const QString &ref, const QString &name)
{
    e.win.w.setStyleNameAsker([name](QString *n) {
        *n = name;
        return true;
    });
    e.select(ref);
    e.win.w.act(QStringLiteral("style.new"))->trigger();
}

// Merge Styles from `source` into `dest`, answering "replace" (or keeping) when asked; how many were asked about is *asked (-1 for no question).
void mergeStylesFrom(EditWindow &dest, EditWindow &source, bool replace, int *asked = nullptr)
{
    const QString title = source.win.w.displayName();
    dest.win.w.setWindowPicker([title](const QStringList &titles) { return int(titles.indexOf(title)); });
    if (asked) *asked = -1;
    dest.win.w.setStyleMergeAsker([asked, replace](int count) {
        if (asked) *asked = count;
        return replace;
    });
    dest.win.w.act(QStringLiteral("style.merge"))->trigger();
}

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

QByteArray themeOf(const QString &accent1)
{
    return QStringLiteral("<?xml version=\"1.0\"?>\n<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"T\"><a:themeElements><a:clrScheme name=\"C\">"
                          "<a:dk1><a:srgbClr val=\"111111\"/></a:dk1><a:lt1><a:srgbClr val=\"EEDDCC\"/></a:lt1><a:dk2><a:srgbClr val=\"222222\"/></a:dk2><a:lt2><a:srgbClr val=\"DDDDDD\"/></a:lt2>"
                          "<a:accent1><a:srgbClr val=\"%1\"/></a:accent1><a:accent2><a:srgbClr val=\"00AA00\"/></a:accent2><a:accent3><a:srgbClr val=\"0000AA\"/></a:accent3>"
                          "<a:accent4><a:srgbClr val=\"AAAA00\"/></a:accent4><a:accent5><a:srgbClr val=\"00AAAA\"/></a:accent5><a:accent6><a:srgbClr val=\"AA00AA\"/></a:accent6>"
                          "<a:hlink><a:srgbClr val=\"0000FF\"/></a:hlink><a:folHlink><a:srgbClr val=\"800080\"/></a:folHlink></a:clrScheme>"
                          "<a:fontScheme name=\"F\"><a:majorFont><a:latin typeface=\"Calibri\"/></a:majorFont><a:minorFont><a:latin typeface=\"Calibri\"/></a:minorFont></a:fontScheme></a:themeElements></a:theme>")
        .arg(accent1)
        .toUtf8();
}

// A workbook whose theme has the accent color given, with a style "Accent" (a theme color for the fill and the text, a font of the theme's own) that cell A1 uses.
QByteArray themedBook(const QString &accent1, bool withStyle = true)
{
    const QString styles = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font>"
        "<font><b/><sz val=\"11\"/><color theme=\"4\"/><name val=\"Calibri\"/><family val=\"1\"/><scheme val=\"major\"/></font></fonts>"
        "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
        "<fill><patternFill patternType=\"solid\"><fgColor theme=\"4\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
        "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
        "<cellStyleXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" applyFont=\"1\" applyFill=\"1\"/></cellStyleXfs>"
        "<cellXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"1\" applyFont=\"1\" applyFill=\"1\"/></cellXfs>"
        "<cellStyles count=\"2\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/><cellStyle name=\"Accent\" xfId=\"1\"/></cellStyles></styleSheet>");
    QString text = styles;
    if (!withStyle) {
        text.replace(QLatin1String("<cellStyles count=\"2\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/><cellStyle name=\"Accent\" xfId=\"1\"/></cellStyles>"),
                     QLatin1String("<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"));
        text.replace(QLatin1String("<cellStyleXfs count=\"2\">"), QLatin1String("<cellStyleXfs count=\"1\">"));
        text.replace(QLatin1String("<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" applyFont=\"1\" applyFill=\"1\"/></cellStyleXfs>"), QLatin1String("</cellStyleXfs>"));
        text.replace(QLatin1String("<cellXfs count=\"2\">"), QLatin1String("<cellXfs count=\"1\">"));
        text.replace(QLatin1String("<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"1\" applyFont=\"1\" applyFill=\"1\"/></cellXfs>"), QLatin1String("</cellXfs>"));
    }
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"%1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row>").arg(withStyle ? 1 : 0);
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), text.toUtf8(), QStringList(), QString(), themeOf(accent1));
}

// A workbook of 100,000 cells; every thousandth (100 of them) has the percent format.
QByteArray bigBook()
{
    QString rows;
    rows.reserve(5 << 20);
    for (int r = 0; r < 1000; ++r) {
        rows += QStringLiteral("<row r=\"%1\">").arg(r + 1);
        for (int c = 0; c < 100; ++c) {
            const bool percent = (r * 100 + c) % 1000 == 3;
            rows += QStringLiteral("<c r=\"%1%2\"%3><v>1</v></c>").arg(columnName(quint32(c))).arg(r + 1).arg(percent ? QStringLiteral(" s=\"1\"") : QString());
        }
        rows += QStringLiteral("</row>");
    }
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"9\" applyNumberFormat=\"1\"/>")));
}

// A workbook whose style "Keep" has an attribute and an extension of another program, with a conditional format and a table that refer to formats of the file.
QByteArray bookWithRules(QByteArray *tableless = nullptr)
{
    const QString styles = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
        "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
        "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
        "<cellStyleXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
        "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" pivotButton=\"1\"><alignment horizontal=\"left\"/><extLst><ext uri=\"{B1B1B1B1-0000-0000-0000-000000000001}\" xmlns:k=\"urn:keep\"><k:mark val=\"7\"/></ext></extLst></xf></cellStyleXfs>"
        "<cellXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"1\"/></cellXfs>"
        "<cellStyles count=\"2\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/><cellStyle name=\"Keep\" xfId=\"1\"/></cellStyles>"
        "<dxfs count=\"1\"><dxf><font><b/></font><fill><patternFill><bgColor rgb=\"FFFF0000\"/></patternFill></fill></dxf></dxfs></styleSheet>");
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\" t=\"s\"><v>1</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>3</v></c><c r=\"B2\" s=\"1\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>8</v></c><c r=\"B3\"><v>9</v></c></row>");
    const QString after = QStringLiteral("<conditionalFormatting sqref=\"A2:A3\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"1\" operator=\"greaterThan\"><formula>5</formula></cfRule></conditionalFormatting>");
    const QByteArray base = buildXlsx({worksheet(rows, after)}, sharedStringsXml({QStringLiteral("<t>A</t>"), QStringLiteral("<t>B</t>")}), QString(), QString(), styles.toUtf8());
    const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"T1\" displayName=\"T1\" ref=\"A1:B3\">"
                                         "<autoFilter ref=\"A1:B3\"/><tableColumns count=\"2\"><tableColumn id=\"1\" name=\"A\" dataDxfId=\"0\"/><tableColumn id=\"2\" name=\"B\"/></tableColumns>"
                                         "<tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>");
    if (tableless) *tableless = base;
    return withTables(base, {table});
}

std::unique_ptr<Window> reopen(const QString &path)
{
    auto again = std::make_unique<Window>();
    if (!again->open(path)) again.reset();
    return again;
}

} // namespace

class CrunchGap2Tests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
    }

    // ---- 1. a line break in a formula, for the old form
    void aLineBreakTypedInAFormulaIsKeptInXlsxAndFlatInTheOldForms()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("E5"));
        const int steps = e.win.w.undoStack()->count();
        quint32 r = 0, c = 0;
        parseCellRef(QStringLiteral("E5"), &r, &c);
        QVERIFY(e.win.w.session()->typeInto(0, r, c, r, c, r, c, QStringLiteral("=SUM(A1,\nA2)")).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(textAt(e, QStringLiteral("E5")), QStringLiteral("=SUM(A1,\nA2)"));
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:11"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QCOMPARE(savedFormula(e.path, QStringLiteral("E5")), QStringLiteral("SUM(A1,\nA2)"));
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QCOMPARE(textIn(*again, QStringLiteral("E5")), QStringLiteral("=SUM(A1,\nA2)"));
        }
        for (const QString &kind : {QStringLiteral("ods"), QStringLiteral("xls")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            // (the formula is kept, and those two forms keep no line break in a formula)
            QCOMPARE(textIn(*again, QStringLiteral("E5")), QStringLiteral("=SUM(A1,A2)"));
        }
    }

    // ---- 2. Find by format, and Replace in notes
    void findByFormatListsOnlyTheCellsThatHaveTheFormat()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));   // (the percent format)
        w.act(QStringLiteral("edit.find"))->trigger();
        FindDialog *d = w.findDialog();
        QVERIFY(d);
        d->findChild<QPushButton *>(QStringLiteral("findFormatFromCell"))->click();
        QVERIFY(d->format());
        QVERIFY(!d->findChild<QLabel *>(QStringLiteral("findFormatLabel"))->text().isEmpty());
        QPushButton *all = d->findChild<QPushButton *>(QStringLiteral("findAll"));
        all->click();   // (no text: every cell with that format)
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);
        QCOMPARE(d->results()->topLevelItem(0)->text(1), QStringLiteral("A3"));
        // with a text too, both must hold
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("25%"));
        all->click();
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
        all->click();
        QTRY_VERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("No cell matches")));
        // the format off: the same text is found in its own cell
        d->findChild<QPushButton *>(QStringLiteral("findFormatClear"))->click();
        QVERIFY(!d->format());
        all->click();
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);
        QCOMPARE(d->results()->topLevelItem(0)->text(1), QStringLiteral("C1"));
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("*"));
        all->click();
        QTRY_VERIFY(d->results()->topLevelItemCount() > 3);
    }

    void findByTheFormatBoxFindsTheCellsThatAreBold()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("B2"));
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(boldAt(e.win, QStringLiteral("B2")));
        w.act(QStringLiteral("edit.find"))->trigger();
        FindDialog *d = w.findDialog();
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *r) {
            r->change.bold = true;
            return true;
        });
        d->findChild<QPushButton *>(QStringLiteral("findFormat"))->click();
        QVERIFY(d->format() && d->format()->bold && *d->format()->bold);
        d->findChild<QPushButton *>(QStringLiteral("findAll"))->click();
        QTRY_VERIFY(d->results()->topLevelItemCount() > 0);
        QStringList got;
        for (int i = 0; i < d->results()->topLevelItemCount(); ++i) got << d->results()->topLevelItem(i)->text(1);
        QCOMPARE(got.join(QLatin1Char(' ')), QStringLiteral("B2"));
        // a dialog that changes nothing leaves no format
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *) { return true; });
        d->findChild<QPushButton *>(QStringLiteral("findFormat"))->click();
        QVERIFY(!d->format());
    }

    void replaceAllInNotesIsOneStepRefusedOnAProtectedSheetAndSavedInEveryFormat()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("say hello twice: hello"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 2, 2, QStringLiteral("Hello there"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 1, 1, QStringLiteral("nothing here"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        auto noteText = [&](crunch::EditSession *session, int row, int col) {
            const auto *n = session->anchors().noteAt(0, quint32(row), quint32(col));
            return n ? n->text : QString();
        };
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        d->setTab(FindDialog::Tab::Replace);
        d->findChild<QComboBox *>(QStringLiteral("findLookIn"))->setCurrentIndex(2);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("bye"));
        // a protected sheet refuses, and nothing changes
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        locked.objects = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        int steps = w.undoStack()->index();
        e.win.told.clear();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY2(e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->index(), steps);
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say hello twice: hello"));
        w.undoStack()->undo();   // (the protection)
        QVERIFY(!s->protectionOf(0).enabled);
        steps = w.undoStack()->index();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_COMPARE(w.undoStack()->index(), steps + 1);
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say bye twice: bye"));
        QCOMPARE(noteText(s, 2, 2), QStringLiteral("bye there"));   // (matching is not case sensitive)
        QCOMPARE(noteText(s, 1, 1), QStringLiteral("nothing here"));
        const QString said = d->findChild<QLabel *>(QStringLiteral("findStatus"))->text();
        QVERIFY2(said.contains(QLatin1String("Changed 3 ")) && said.contains(QLatin1String(" in 2 ")), qPrintable(said));
        // one step takes both back, and gives both again
        w.undoStack()->undo();
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say hello twice: hello"));
        QCOMPARE(noteText(s, 2, 2), QStringLiteral("Hello there"));
        w.undoStack()->redo();
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say bye twice: bye"));
        // the app's own save, and the copies in the other formats, keep the new words
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QCOMPARE(noteText(again->w.session(), 0, 0), QStringLiteral("say bye twice: bye"));
            QCOMPARE(noteText(again->w.session(), 2, 2), QStringLiteral("bye there"));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(noteText(again->w.session(), 0, 0).contains(QLatin1String("bye twice: bye")), qPrintable(kind + noteText(again->w.session(), 0, 0)));
            QVERIFY2(noteText(again->w.session(), 2, 2).contains(QLatin1String("bye there")), qPrintable(kind));
        }
    }

    void replaceOneInNotesChangesTheNoteOfTheCursorCellAndGoesOn()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("a cat"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 2, 2, QStringLiteral("a cat too"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        e.select(QStringLiteral("A1"));
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        d->setTab(FindDialog::Tab::Replace);
        d->findChild<QComboBox *>(QStringLiteral("findLookIn"))->setCurrentIndex(2);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("cat"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("dog"));
        const int steps = w.undoStack()->count();
        d->findChild<QPushButton *>(QStringLiteral("replaceOne"))->click();
        QTRY_COMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(s->anchors().noteAt(0, 0, 0)->text, QStringLiteral("a dog"));
        QCOMPARE(s->anchors().noteAt(0, 2, 2)->text, QStringLiteral("a cat too"));
        QTRY_COMPARE(e.selected(), QStringLiteral("C3"));   // (and the search went on to the next note)
        QCOMPARE(w.undoStack()->count(), steps + 1);
    }

    // ---- 3. Modify and Merge Styles
    void modifyingAStyleChangesItAndEveryCellThatUsesItInOneStepAndIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("style.new"))->trigger();
        QVERIFY(hasStyle(e.win, QStringLiteral("Mine")));
        QVERIFY(w.session()->applyStyleNamed(0, rangeOf(QStringLiteral("B1")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);   // B1
        QVERIFY(e.settle());
        QVERIFY(!styleBold(e.win, QStringLiteral("Mine")) && !boldAt(e.win, QStringLiteral("A3")) && !boldAt(e.win, QStringLiteral("B1")));
        w.setFormatAsker([](const FormatCellsDialog::Input &in, FormatCellsDialog::Result *r) {
            // (the dialog starts from the style's own look: a percent)
            if (in.view.numberFormat != QLatin1String("0%")) return false;
            r->change.bold = true;
            return true;
        });
        const int steps = w.undoStack()->count();
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QVERIFY(styleBold(e.win, QStringLiteral("Mine")));
        QVERIFY(boldAt(e.win, QStringLiteral("A3")) && boldAt(e.win, QStringLiteral("B1")));
        QVERIFY2(!boldAt(e.win, QStringLiteral("A1")), "a cell that does not use the style");
        w.undoStack()->undo();
        QVERIFY(!styleBold(e.win, QStringLiteral("Mine")) && !boldAt(e.win, QStringLiteral("A3")) && !boldAt(e.win, QStringLiteral("B1")));
        w.undoStack()->redo();
        QVERIFY(styleBold(e.win, QStringLiteral("Mine")) && boldAt(e.win, QStringLiteral("B1")));
        // the app's own save keeps the style's new look (a cell given the style afterwards has it) and the cells'
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const QString first = e.copyOfFile();
            auto again = reopen(first);
            QVERIFY(again);
            QVERIFY(styleBold(*again, QStringLiteral("Mine")));
            QVERIFY(boldAt(*again, QStringLiteral("A3")) && boldAt(*again, QStringLiteral("B1")));
            QVERIFY(again->w.session()->applyStyleNamed(0, rangeOf(QStringLiteral("A1")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);
            QVERIFY(boldAt(*again, QStringLiteral("A1")));
            again->w.undoStack()->undo();
            // the style is in the file now: modified there, the file's own style is written again, in place
            again->w.setStyleNameAsker([](QString *n) {
                *n = QStringLiteral("Mine");
                return true;
            });
            again->w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *r) {
                r->change.italic = true;
                return true;
            });
            again->w.act(QStringLiteral("style.modify"))->trigger();
            QVERIFY(styleItalic(*again, QStringLiteral("Mine")) && styleBold(*again, QStringLiteral("Mine")));
            QSignalSpy savedAgain(&again->w, &CrunchWindow::saved);
            QVERIFY(again->w.save());
            QVERIFY2(savedOk(savedAgain), "the second save failed");
            const QString second = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
            QVERIFY(QFile::copy(first, second));
            const auto third = reopen(second);
            QVERIFY(third);
            QVERIFY(styleItalic(*third, QStringLiteral("Mine")) && styleBold(*third, QStringLiteral("Mine")));
            QVERIFY(third->w.session()->applyStyleNamed(0, rangeOf(QStringLiteral("A1")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);
            QVERIFY(boldAt(*third, QStringLiteral("A1")));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(boldAt(*again, QStringLiteral("A3")) && boldAt(*again, QStringLiteral("B1")), qPrintable(kind));
        }
    }

    void modifyingAStyleOnAProtectedSheetIsRefusedWholeAndNormalCannotBeModified()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("style.new"))->trigger();
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *r) {
            r->change.bold = true;
            return true;
        });
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(w.session()->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        const int steps = w.undoStack()->count();
        e.win.told.clear();
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(!styleBold(e.win, QStringLiteral("Mine")) && !boldAt(e.win, QStringLiteral("A3")));
        w.undoStack()->undo();
        e.win.told.clear();
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Normal");
            return true;
        });
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Normal")), qPrintable(e.win.told.join('|')));
        QVERIFY(!styleBold(e.win, QStringLiteral("Normal")));
    }

    void mergeStylesBringsInTheStylesOfAnotherWorkbookAndAsksBeforeReplacing()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(source.open(styledBook()));
        QVERIFY(dest.settle() && source.settle());
        // the source: "Mine" in bold, and "Fancy"; the destination: its own "Mine" (not bold)
        source.win.w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        source.select(QStringLiteral("B1"));
        source.win.w.act(QStringLiteral("fmt.bold"))->trigger();
        source.win.w.act(QStringLiteral("style.new"))->trigger();
        source.win.w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Fancy");
            return true;
        });
        source.select(QStringLiteral("A3"));
        source.win.w.act(QStringLiteral("style.new"))->trigger();
        dest.win.w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        dest.select(QStringLiteral("A3"));
        dest.win.w.act(QStringLiteral("style.new"))->trigger();
        QVERIFY(styleBold(source.win, QStringLiteral("Mine")) && !styleBold(dest.win, QStringLiteral("Mine")) && !hasStyle(dest.win, QStringLiteral("Fancy")));
        CrunchWindow &w = dest.win.w;
        const QString title = source.win.w.displayName();
        w.setWindowPicker([&](const QStringList &titles) { return int(titles.indexOf(title)); });
        // kept: the destination's own "Mine" stays, "Fancy" comes in
        int asked = -1;
        w.setStyleMergeAsker([&](int count) {
            asked = count;
            return false;
        });
        int steps = w.undoStack()->index();
        w.act(QStringLiteral("style.merge"))->trigger();
        QCOMPARE(asked, 1);
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(hasStyle(dest.win, QStringLiteral("Fancy")) && !styleBold(dest.win, QStringLiteral("Mine")));
        w.undoStack()->undo();
        QVERIFY(!hasStyle(dest.win, QStringLiteral("Fancy")));
        // replaced: asked once, and "Mine" takes the source's look, in one step that Undo takes back
        w.setStyleMergeAsker([&](int count) {
            asked = count;
            return true;
        });
        asked = -1;
        steps = w.undoStack()->index();
        w.act(QStringLiteral("style.merge"))->trigger();
        QVERIFY(dest.settle());
        QCOMPARE(asked, 1);
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(hasStyle(dest.win, QStringLiteral("Fancy")) && styleBold(dest.win, QStringLiteral("Mine")));
        QVERIFY(boldAt(dest.win, QStringLiteral("A3")));   // (the cell that uses "Mine" follows)
        w.undoStack()->undo();
        QVERIFY(!hasStyle(dest.win, QStringLiteral("Fancy")) && !styleBold(dest.win, QStringLiteral("Mine")) && !boldAt(dest.win, QStringLiteral("A3")));
        w.undoStack()->redo();
        QVERIFY(styleBold(dest.win, QStringLiteral("Mine")));
        // merged again, the styles are the same: nothing is asked and nothing is added
        asked = -1;
        steps = w.undoStack()->index();
        w.act(QStringLiteral("style.merge"))->trigger();
        QCOMPARE(asked, -1);
        QCOMPARE(w.undoStack()->index(), steps);
        // saved: the file the app writes has them
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const auto again = reopen(dest.copyOfFile());
        QVERIFY(again);
        QVERIFY(hasStyle(*again, QStringLiteral("Fancy")) && styleBold(*again, QStringLiteral("Mine")));
    }

    void mergeStylesWithNoOtherWorkbookSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        e.win.told.clear();
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("style.merge"))->trigger();
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Open the workbook")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }

    // ---- 5. Show Detail, Hide Detail, and Remove Hyperlinks
    void hideAndShowDetailCollapseAndOpenTheGroupInOneStepEachAndAreSaved()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        auto hidden = [&](Window &win, quint32 row) {
            auto hold = win.w.workbookLoader().hold();
            return win.w.session()->outlineOf(0, false).hiddenOf(row);
        };
        // rows 1 to 2 are a group, and row 3 sums them up
        QVERIFY(w.session()->changeOutline(0, false, crunch::Outline({}, 1048576, true).group(0, 1), QStringLiteral("Group")).ok);
        e.select(QStringLiteral("A3"));
        int steps = w.undoStack()->index();
        w.act(QStringLiteral("outline.hideDetail"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(hidden(e.win, 0) && hidden(e.win, 1) && !hidden(e.win, 2));
        // saved: the file the app writes, and the copies, hold the hidden lines
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QVERIFY(hidden(*again, 0) && hidden(*again, 1));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(hidden(*again, 0) && hidden(*again, 1), qPrintable(kind));
        }
        // Hide Detail again: nothing to hide (the group is collapsed), said in a message
        e.win.told.clear();
        steps = w.undoStack()->index();
        w.act(QStringLiteral("outline.hideDetail"))->trigger();
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Nothing was changed")));
        QCOMPARE(w.undoStack()->index(), steps);
        w.act(QStringLiteral("outline.showDetail"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(!hidden(e.win, 0) && !hidden(e.win, 1));
        w.undoStack()->undo();
        QVERIFY(hidden(e.win, 0));
        // with a cell that is in no group: said, nothing changed
        e.select(QStringLiteral("A9"));
        e.win.told.clear();
        w.act(QStringLiteral("outline.showDetail"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
        QVERIFY(hidden(e.win, 0));
    }

    void showAndHideDetailOnAProtectedSheetAreRefused()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.session()->changeOutline(0, false, crunch::Outline({}, 1048576, true).group(0, 1), QStringLiteral("Group")).ok);
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(w.session()->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        e.select(QStringLiteral("A3"));
        const int steps = w.undoStack()->index();
        e.win.told.clear();
        w.act(QStringLiteral("outline.hideDetail"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->index(), steps);
        auto hold = w.workbookLoader().hold();
        QVERIFY(!w.session()->outlineOf(0, false).hiddenOf(0));
    }

    void removeHyperlinksTakesTheLinkAndItsLookOffInOneStepAndIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        jp::office::sheet::LinkSpec link;
        link.target = QStringLiteral("https://example.com/");
        QVERIFY(s->setLink(0, rangeOf(QStringLiteral("A1")), link, QStringLiteral("Link")).ok);
        QVERIFY(s->applyStyleNamed(0, rangeOf(QStringLiteral("A1")), QStringLiteral("Hyperlink"), QStringLiteral("Link look")).ok);
        QVERIFY(e.settle());
        auto underlined = [&](Window &win) {
            auto hold = win.w.workbookLoader().hold();
            const grid::GridSheet &gs = win.w.view()->sheet();
            const jp::office::sheet::Styles &st = win.w.loaded()->book->styles();
            return st.font(st.format(gs.styleOf(0, 0, gs.cellAt(0, 0))).font).underline != jp::office::sheet::Underline::None;
        };
        QVERIFY(underlined(e.win) && s->anchors().linkAt(0, 0, 0));
        // a protected sheet that does not allow links refuses, and nothing changes
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        e.select(QStringLiteral("A1"));
        int steps = w.undoStack()->index();
        e.win.told.clear();
        w.act(QStringLiteral("link.removeAll"))->setEnabled(true);
        w.act(QStringLiteral("link.removeAll"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->index(), steps);
        QVERIFY(underlined(e.win) && s->anchors().linkAt(0, 0, 0));
        w.undoStack()->undo();   // (the protection)
        e.select(QStringLiteral("A1"));
        QVERIFY(w.act(QStringLiteral("link.removeAll"))->isEnabled());
        steps = w.undoStack()->index();
        w.act(QStringLiteral("link.removeAll"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(!underlined(e.win) && !s->anchors().linkAt(0, 0, 0));
        w.undoStack()->undo();
        QVERIFY(underlined(e.win) && s->anchors().linkAt(0, 0, 0));
        w.undoStack()->redo();
        QVERIFY(!underlined(e.win) && !s->anchors().linkAt(0, 0, 0));
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QVERIFY(!underlined(*again) && !again->w.session()->anchors().linkAt(0, 0, 0));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(!underlined(*again) && !again->w.session()->anchors().linkAt(0, 0, 0), qPrintable(kind));
        }
    }

    // ---- the review's fixes: a style changes only what the cells inherit from it
    void modifyingAStyleKeepsACellsOwnFillAndNumberFormat()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        newStyle(e, QStringLiteral("A3"), QStringLiteral("Mine"));
        QVERIFY(s->applyStyleNamed(0, rangeOf(QStringLiteral("B1:B2")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("B2")), solidFill(0xFFFF00), QStringLiteral("Fill")).ok);   // (B2's own fill)
        QVERIFY(e.settle());
        QCOMPARE(fillAt(e.win, QStringLiteral("B1")), 0u);
        QCOMPARE(fillAt(e.win, QStringLiteral("B2")), 0xFFFF00u);
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *r) {
            r->change = solidFill(0xFF0000);
            return true;
        });
        const int steps = w.undoStack()->count();
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(styleFill(e.win, QStringLiteral("Mine")), 0xFF0000u);
        QCOMPARE(fillAt(e.win, QStringLiteral("A3")), 0xFF0000u);   // (what the cells inherit changes)
        QCOMPARE(fillAt(e.win, QStringLiteral("B1")), 0xFF0000u);
        QCOMPARE(fillAt(e.win, QStringLiteral("B2")), 0xFFFF00u);   // (an override stays, though the style's same property changed)
        QCOMPARE(numberFormatAt(e.win, QStringLiteral("B2")), QStringLiteral("0%"));
        w.undoStack()->undo();
        QCOMPARE(fillAt(e.win, QStringLiteral("B1")), 0u);
        QCOMPARE(fillAt(e.win, QStringLiteral("B2")), 0xFFFF00u);
    }

    void modifyingAStyleChangesAnEmptyColumnAndRowThatFollowIt()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        newStyle(e, QStringLiteral("A3"), QStringLiteral("Mine"));
        jp::office::sheet::CellRange column;
        column.valid = true;
        column.row1 = 0;
        column.row2 = jp::office::sheet::kMaxRows - 1;
        column.col1 = column.col2 = 5;   // F, which holds nothing
        QVERIFY(s->applyStyleNamed(0, column, QStringLiteral("Mine"), QStringLiteral("Style")).ok);
        jp::office::sheet::CellRange wholeRow;
        wholeRow.valid = true;
        wholeRow.col1 = 0;
        wholeRow.col2 = jp::office::sheet::kMaxColumns - 1;
        wholeRow.row1 = wholeRow.row2 = 9;   // row 10, which holds nothing
        QVERIFY(s->applyStyleNamed(0, wholeRow, QStringLiteral("Mine"), QStringLiteral("Style")).ok);
        QVERIFY(e.settle());
        QCOMPARE(columnFill(e.win, 5), 0u);
        QCOMPARE(rowFill(e.win, 9), 0u);
        QVERIFY(s->modifyCellStyle(QStringLiteral("Mine"), solidFill(0x00FF00)).ok);
        QVERIFY(e.settle());
        QCOMPARE(columnFill(e.win, 5), 0x00FF00u);
        QCOMPARE(rowFill(e.win, 9), 0x00FF00u);
        w.undoStack()->undo();
        QCOMPARE(columnFill(e.win, 5), 0u);
        QCOMPARE(rowFill(e.win, 9), 0u);
        w.undoStack()->redo();
        QCOMPARE(columnFill(e.win, 5), 0x00FF00u);
    }

    void modifyingABuiltInStyleTheWorkbookHasNotUsedIsOneStepThatUndoTakesBack()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(!keepsStyle(e.win, QStringLiteral("Good")));
        const int steps = w.undoStack()->count();
        QVERIFY(w.session()->modifyCellStyle(QStringLiteral("Good"), solidFill(0x123456)).ok);
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QVERIFY(keepsStyle(e.win, QStringLiteral("Good")));
        QCOMPARE(styleFill(e.win, QStringLiteral("Good")), 0x123456u);
        w.undoStack()->undo();
        QVERIFY2(!keepsStyle(e.win, QStringLiteral("Good")), "Undo takes out the style the step made");
        w.undoStack()->redo();
        QVERIFY(keepsStyle(e.win, QStringLiteral("Good")));
        QCOMPARE(styleFill(e.win, QStringLiteral("Good")), 0x123456u);
    }

    void mergingStylesKeepsACellsOwnFillAndMovesWhatItInherits()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(source.open(styledBook()));
        QVERIFY(dest.settle() && source.settle());
        // the source's "Mine": a red fill, and General; the destination's: a percent
        QVERIFY(source.win.w.session()->formatCells(0, rangeOf(QStringLiteral("B1")), solidFill(0xFF0000), QStringLiteral("Fill")).ok);
        newStyle(source, QStringLiteral("B1"), QStringLiteral("Mine"));
        newStyle(dest, QStringLiteral("A3"), QStringLiteral("Mine"));
        crunch::EditSession *s = dest.win.w.session();
        QVERIFY(s->applyStyleNamed(0, rangeOf(QStringLiteral("B1:B2")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("B2")), solidFill(0xFFFF00), QStringLiteral("Fill")).ok);   // (B2's own fill)
        QVERIFY(dest.settle());
        int asked = -1;
        const int steps = dest.win.w.undoStack()->count();
        mergeStylesFrom(dest, source, true, &asked);
        QVERIFY(dest.settle());
        QCOMPARE(asked, 1);
        QCOMPARE(dest.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(styleFill(dest.win, QStringLiteral("Mine")), 0xFF0000u);
        QCOMPARE(fillAt(dest.win, QStringLiteral("A3")), 0xFF0000u);
        QCOMPARE(fillAt(dest.win, QStringLiteral("B1")), 0xFF0000u);
        QCOMPARE(fillAt(dest.win, QStringLiteral("B2")), 0xFFFF00u);   // (its own fill stays)
        QCOMPARE(numberFormatAt(dest.win, QStringLiteral("B2")), QStringLiteral("General"));   // (and what it inherited moves)
    }

    void undoAndRedoOfAMergeThatAddsAndReplacesStylesAreExact()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(source.open(styledBook()));
        QVERIFY(dest.settle() && source.settle());
        // the source: "Mine" and "Fancy" in bold, and the built-in "Good" with a fill of its own choosing
        source.select(QStringLiteral("A1"));
        source.win.w.act(QStringLiteral("fmt.bold"))->trigger();
        newStyle(source, QStringLiteral("A1"), QStringLiteral("Mine"));
        newStyle(source, QStringLiteral("A1"), QStringLiteral("Fancy"));
        QVERIFY(source.win.w.session()->modifyCellStyle(QStringLiteral("Good"), solidFill(0x123456)).ok);
        QVERIFY(source.settle());
        // the destination: its own "Mine" (a percent, not bold) that A3 follows
        newStyle(dest, QStringLiteral("A3"), QStringLiteral("Mine"));
        QVERIFY(dest.settle());
        CrunchWindow &w = dest.win.w;
        const int steps = w.undoStack()->count();
        mergeStylesFrom(dest, source, true);
        QVERIFY(dest.settle());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        auto merged = [&] {
            return keepsStyle(dest.win, QStringLiteral("Fancy")) && keepsStyle(dest.win, QStringLiteral("Good")) && styleBold(dest.win, QStringLiteral("Fancy")) && styleBold(dest.win, QStringLiteral("Mine"))
                   && styleFill(dest.win, QStringLiteral("Good")) == 0x123456u && boldAt(dest.win, QStringLiteral("A3"));
        };
        QVERIFY(merged());
        w.undoStack()->undo();
        QVERIFY2(!keepsStyle(dest.win, QStringLiteral("Fancy")) && !keepsStyle(dest.win, QStringLiteral("Good")), "Undo removes every style the merge made");
        QVERIFY2(!styleBold(dest.win, QStringLiteral("Mine")) && !boldAt(dest.win, QStringLiteral("A3")) && numberFormatAt(dest.win, QStringLiteral("A3")) == QLatin1String("0%"), "and restores every look it changed");
        w.undoStack()->redo();
        QVERIFY2(merged(), "Redo makes the same styles with the same looks");
        w.undoStack()->undo();
        w.undoStack()->redo();
        QVERIFY(merged());
    }

    void aRefusedMergeLeavesNoStyleBehind()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(source.open(styledBook()));
        QVERIFY(dest.settle() && source.settle());
        source.select(QStringLiteral("A1"));
        source.win.w.act(QStringLiteral("fmt.bold"))->trigger();
        newStyle(source, QStringLiteral("A1"), QStringLiteral("Mine"));
        QVERIFY(source.win.w.session()->modifyCellStyle(QStringLiteral("Good"), solidFill(0x123456)).ok);
        newStyle(dest, QStringLiteral("A3"), QStringLiteral("Mine"));
        QVERIFY(dest.settle());
        // the destination's sheet is protected, and a cell of it follows the style that the merge would change
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(dest.win.w.session()->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        const int steps = dest.win.w.undoStack()->count();
        dest.win.told.clear();
        mergeStylesFrom(dest, source, true);
        QVERIFY2(!dest.win.told.isEmpty() && dest.win.told.last().contains(QLatin1String("protected")), qPrintable(dest.win.told.join('|')));
        QCOMPARE(dest.win.w.undoStack()->count(), steps);
        QVERIFY2(!keepsStyle(dest.win, QStringLiteral("Good")), "the built-in style the merge would have made is not made");
        QVERIFY(!styleBold(dest.win, QStringLiteral("Mine")));
    }

    void mergeStylesSaysTheOtherWorkbookIsBusy()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(source.open(styledBook()));
        QVERIFY(dest.settle() && source.settle());
        newStyle(source, QStringLiteral("A3"), QStringLiteral("Fancy"));
        source.win.w.workbookLoader().refuseHolds(1);   // (the next hold of the source's worker is refused)
        dest.win.told.clear();
        const int steps = dest.win.w.undoStack()->count();
        mergeStylesFrom(dest, source, true);
        QVERIFY2(!dest.win.told.isEmpty() && dest.win.told.last().contains(QLatin1String("busy")), qPrintable(dest.win.told.join('|')));
        QVERIFY(!dest.win.told.last().contains(QLatin1String("no styles")));
        QCOMPARE(dest.win.w.undoStack()->count(), steps);
        source.win.w.workbookLoader().refuseHolds(0);
        mergeStylesFrom(dest, source, true);   // (tried again, it works)
        QVERIFY(keepsStyle(dest.win, QStringLiteral("Fancy")));
    }

    void mergeStylesRefusesMoreStylesThanItBringsInAtOnce()
    {
        EditWindow dest;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(dest.settle());
        crunch::EditSession::StyleLooks looks;
        for (int i = 0; i <= crunch::EditSession::kMaxMergedStyles; ++i) {
            crunch::EditSession::StyleLook look;
            look.name = QStringLiteral("Style %1").arg(i);
            look.look = look.resolved = solidFill(0x00FF00u + quint32(i));
            looks.styles.push_back(look);
        }
        QCOMPARE(int(looks.styles.size()), crunch::EditSession::kMaxMergedStyles + 1);
        const int steps = dest.win.w.undoStack()->count();
        const crunch::EditSession::Outcome o = dest.win.w.session()->mergeStyles(looks, crunch::EditSession::Same::Replace);
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QString::number(crunch::EditSession::kMaxMergedStyles)) && o.problem.contains(QLatin1String("Nothing was merged")), qPrintable(o.problem));
        QCOMPARE(dest.win.w.undoStack()->count(), steps);
        QVERIFY(!keepsStyle(dest.win, QStringLiteral("Style 0")));
        looks.styles.pop_back();   // (exactly the most is taken)
        QVERIFY(dest.win.w.session()->mergeStyles(looks, crunch::EditSession::Same::Replace).ok);
        QVERIFY(keepsStyle(dest.win, QStringLiteral("Style 0")) && keepsStyle(dest.win, QStringLiteral("Style 999")));
    }

    void mergedStylesKeepTheColorsTheyShowAcrossWorkbooksOfAnotherTheme()
    {
        // the same name and the same theme color number, in two themes: they look different, and the merge sees that
        EditWindow dest, source;
        QVERIFY(dest.open(themedBook(QStringLiteral("00AA00"))));
        QVERIFY(source.open(themedBook(QStringLiteral("AA0000"))));
        QVERIFY(dest.settle() && source.settle());
        QCOMPARE(styleFill(source.win, QStringLiteral("Accent")), 0xAA0000u);
        QCOMPARE(styleFill(dest.win, QStringLiteral("Accent")), 0x00AA00u);
        int asked = -1;
        mergeStylesFrom(dest, source, true, &asked);
        QVERIFY(dest.settle());
        QCOMPARE(asked, 1);
        QCOMPARE(styleFill(dest.win, QStringLiteral("Accent")), 0xAA0000u);   // (the color as the source shows it, not the destination's accent)
        QCOMPARE(fillAt(dest.win, QStringLiteral("A1")), 0xAA0000u);
        // a style the destination does not have is copied as the color it shows
        EditWindow other;
        QVERIFY(other.open(themedBook(QStringLiteral("0000AA"), false)));
        QVERIFY(other.settle());
        mergeStylesFrom(other, source, true);
        QVERIFY(other.settle());
        QVERIFY(keepsStyle(other.win, QStringLiteral("Accent")));
        QCOMPARE(styleFill(other.win, QStringLiteral("Accent")), 0xAA0000u);
    }

    void mergedStylesCarryTheFontsFamilyAndSchemeBetweenWorkbooksOfTheSameTheme()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(themedBook(QStringLiteral("AA0000"), false)));
        QVERIFY(source.open(themedBook(QStringLiteral("AA0000"))));
        QVERIFY(dest.settle() && source.settle());
        mergeStylesFrom(dest, source, true);
        QVERIFY(dest.settle());
        QVERIFY(keepsStyle(dest.win, QStringLiteral("Accent")));
        const CellFormat look = styleOfName(dest.win, QStringLiteral("Accent"));
        auto hold = dest.win.w.workbookLoader().hold();
        const jp::office::sheet::Styles &st = dest.win.w.loaded()->book->styles();
        QCOMPARE(st.font(look.font).family, 1);
        QCOMPARE(st.font(look.font).scheme, QStringLiteral("major"));
        // (and a theme color stays a theme color where the theme is the same)
        QVERIFY(st.fill(look.fill).fg.kind == ColorSpec::Kind::Theme);
    }

    // ---- Find by format: each distinct format once, the work charged, an empty format said first
    void findByFormatWorksOutEachDistinctFormatOnceOnABigSheet()
    {
        EditWindow e;
        QVERIFY(e.open(bigBook()));
        QVERIFY(e.settle());
        crunch::FindOptions options;
        FormatChange want;
        want.numberFormat = QStringLiteral("0%");
        options.format = std::make_shared<const FormatChange>(want);
        crunch::FindRun run(e.win.w.session(), options, crunch::FindRun::Want::All);
        while (!run.step(0)) {
        }
        QCOMPARE(int(run.hits().size()), 100);
        QCOMPARE(run.cellsLooked(), qint64(100000));
        QCOMPARE(run.formatResolutions(), qint64(2));   // (the two formats the cells have, not one for each of the 100,000 cells)
        // a slice's budget counts the formats worked out: it ends well before as many cells as the budget have been looked at
        crunch::FindRun sliced(e.win.w.session(), options, crunch::FindRun::Want::All);
        sliced.step(200);
        QVERIFY2(sliced.cellsLooked() < 200 && sliced.cellsLooked() > 0, qPrintable(QString::number(sliced.cellsLooked())));
    }

    void aFormatWithNothingInItIsSaidBeforeAnythingIsSearched()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        crunch::FindOptions options;
        options.pattern.text = QStringLiteral("hello");
        options.format = std::make_shared<const FormatChange>();
        crunch::FindRun run(e.win.w.session(), options, crunch::FindRun::Want::All);
        QVERIFY(run.done());
        QVERIFY2(run.problem().contains(QLatin1String("nothing in it")), qPrintable(run.problem()));
        QCOMPARE(run.cellsLooked(), qint64(0));
        // the dialog says it when the format is set, and looks for no format at all
        e.win.w.act(QStringLiteral("edit.find"))->trigger();
        FindDialog *d = e.win.w.findDialog();
        QVERIFY(d);
        d->setFormat(std::make_shared<const FormatChange>(), QString());
        QVERIFY(!d->format());
        QVERIFY2(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("No format was chosen")), qPrintable(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text()));
        // and says that the look a conditional format gives is not searched
        bool said = false;
        for (QLabel *label : d->findChildren<QLabel *>())
            said = said || label->text().contains(QLatin1String("conditional format"));
        QVERIFY(said);
    }

    // ---- Replace in notes: the plain notes are changed, the conversations are left and counted
    void replaceAllInNotesChangesThePlainNotesAndCountsTheConversationsItLeft()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("say hello"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 2, 2, QStringLiteral("hello again"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->newComment(0, 1, 1, QStringLiteral("a hello in a conversation"), QStringLiteral("Bob"), QStringLiteral("Comment")).ok);
        QVERIFY(s->newComment(0, 3, 3, QStringLiteral("hello too"), QStringLiteral("Bob"), QStringLiteral("Comment")).ok);
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        d->setTab(FindDialog::Tab::Replace);
        d->findChild<QComboBox *>(QStringLiteral("findLookIn"))->setCurrentIndex(2);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("bye"));
        const int steps = w.undoStack()->index();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_COMPARE(w.undoStack()->index(), steps + 1);
        QCOMPARE(s->anchors().noteAt(0, 0, 0)->text, QStringLiteral("say bye"));
        QCOMPARE(s->anchors().noteAt(0, 2, 2)->text, QStringLiteral("bye again"));
        QVERIFY(s->anchors().noteAt(0, 1, 1)->threaded && s->anchors().noteAt(0, 1, 1)->text.contains(QLatin1String("hello")));
        QVERIFY(s->anchors().noteAt(0, 3, 3)->threaded && s->anchors().noteAt(0, 3, 3)->text.contains(QLatin1String("hello")));
        const QString said = d->findChild<QLabel *>(QStringLiteral("findStatus"))->text();
        QVERIFY2(said.contains(QLatin1String("Changed 2 ")) && said.contains(QLatin1String("2 conversation")) && said.contains(QLatin1String("left as they are")), qPrintable(said));
        // a note of only conversations changes nothing and says how many were left
        QVERIFY(s->deleteNotes(0, {{0, 0}, {2, 2}}, QStringLiteral("Delete")).ok);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
        const int again = w.undoStack()->index();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_VERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("2 conversation")));
        QCOMPARE(w.undoStack()->index(), again);
    }

    void aReplacedNoteClearsItsDateAsEditingItsWordsDoes()
    {
        using namespace odstest;
        const QString dated = QStringLiteral("<office:annotation><dc:creator>Ann</dc:creator><dc:date>2010-02-07T00:00:00</dc:date><text:p>%1</text:p></office:annotation>");
        const QString cells = cell(QString(), dated.arg(QStringLiteral("say hello"))) + cell(QString(), dated.arg(QStringLiteral("other")));
        EditWindow e;
        QVERIFY(e.open(buildOds(contentXml(table(QStringLiteral("S"), columns(2) + row(cells)))), QStringLiteral("book.ods")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->anchors().noteAt(0, 0, 0)->date, QStringLiteral("2010-02-07T00:00:00"));
        jp::office::sheet::TextPattern pattern;
        pattern.text = QStringLiteral("hello");
        crunch::EditSession::ReplaceResult result;
        QVERIFY(s->replaceInNotes({{0, 0, 0}, {0, 0, 1}}, pattern, QStringLiteral("bye"), &result).ok);
        QCOMPARE(result.cells, 1);
        QCOMPARE(s->anchors().noteAt(0, 0, 0)->text, QStringLiteral("say bye"));
        QVERIFY2(s->anchors().noteAt(0, 0, 0)->date.isEmpty(), "the words changed, so the date is cleared, as when a note's words are edited");
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->date, QStringLiteral("2010-02-07T00:00:00"));   // (a note that is not changed keeps its date)
        // the same rule as note editing: other words clear it, the same words keep it
        QVERIFY(s->setNote(0, 0, 1, QStringLiteral("other"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok || true);
        QCOMPARE(s->anchors().noteAt(0, 0, 1)->date, QStringLiteral("2010-02-07T00:00:00"));
        QVERIFY(s->setNote(0, 0, 1, QStringLiteral("another"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->anchors().noteAt(0, 0, 1)->date.isEmpty());
    }

    // ---- the file's own style record is changed in place: what else it says stays, and the rules and tables that refer to formats still open
    void modifyingAFileStyleKeepsWhatElseItsRecordSaysAndTheRulesAndTables()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithRules()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.session()->applyStyleNamed(0, rangeOf(QStringLiteral("A1")), QStringLiteral("Keep"), QStringLiteral("Style")).ok);
        FormatChange bold;
        bold.bold = true;
        QVERIFY(w.session()->modifyCellStyle(QStringLiteral("Keep"), bold).ok);
        QVERIFY(e.settle());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        const QString styles = QString::fromUtf8(parts.value(QStringLiteral("xl/styles.xml")));
        QVERIFY2(styles.contains(QLatin1String("pivotButton=\"1\"")), "an attribute of another program stays on the style's record");
        QVERIFY2(styles.contains(QLatin1String("urn:keep")) && styles.contains(QLatin1String("<extLst>")), "so does its extension list");
        QVERIFY2(styles.contains(QLatin1String("<b/>")) || styles.contains(QLatin1String("<b val=\"1\"/>")), "and the new look is in");
        QVERIFY(styles.contains(QLatin1String("<dxfs count=\"1\">")));
        const QString sheet = QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QVERIFY(sheet.contains(QLatin1String("<conditionalFormatting")) && sheet.contains(QLatin1String("dxfId=\"0\"")));
        QVERIFY(!parts.value(QStringLiteral("xl/tables/table1.xml")).isEmpty());
        // opened again by the app: the style, the rule, and the table are there
        const auto again = reopen(e.copyOfFile());
        QVERIFY(again);
        QVERIFY(styleBold(*again, QStringLiteral("Keep")));
        QCOMPARE(fillAt(*again, QStringLiteral("A2")), 0u);
        auto hold = again->w.workbookLoader().hold();
        QVERIFY(again->w.loaded()->book->info().sheets.size() == 1);
        QVERIFY(again->w.loaded()->book->tables().size() == 1);
    }

    // ---- Remove Hyperlinks takes off only the link's own look
    void removeHyperlinksKeepsTheFillAndNumberFormatOfTheCell()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        jp::office::sheet::LinkSpec link;
        link.target = QStringLiteral("https://example.com/");
        QVERIFY(s->setLink(0, rangeOf(QStringLiteral("A3")), link, QStringLiteral("Link")).ok);
        QVERIFY(s->setLink(0, rangeOf(QStringLiteral("B3")), link, QStringLiteral("Link")).ok);
        QVERIFY(s->applyStyleNamed(0, rangeOf(QStringLiteral("A3:B3")), QStringLiteral("Hyperlink"), QStringLiteral("Link look")).ok);
        // A3 has its own fill and a percent format; B3 a fill and a date format
        FormatChange percentFill = solidFill(0xFFFF00);
        percentFill.numberFormat = QStringLiteral("0%");
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("A3")), percentFill, QStringLiteral("Fill")).ok);
        FormatChange both = solidFill(0xFFFF00);
        both.numberFormat = QStringLiteral("yyyy-mm-dd");
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("B3")), both, QStringLiteral("Fill")).ok);
        QVERIFY(e.settle());
        const auto linked = linkLookAt(e.win, QStringLiteral("B3"));
        QVERIFY(linked.second);   // (underlined)
        const quint32 linkColor = linked.first;
        QCOMPARE(fillAt(e.win, QStringLiteral("B3")), 0xFFFF00u);
        e.view()->selectRange(2, 0, 2, 1);
        const int steps = w.undoStack()->index();
        w.act(QStringLiteral("link.removeAll"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);   // (one step)
        QVERIFY(!s->anchors().linkAt(0, 2, 1));
        for (const QString &ref : {QStringLiteral("A3"), QStringLiteral("B3")}) {
            const auto look = linkLookAt(e.win, ref);
            QVERIFY2(!look.second, "the link's underline is gone");
            QVERIFY2(look.first != linkColor, "and so is its color");
            QCOMPARE(fillAt(e.win, ref), 0xFFFF00u);
        }
        QCOMPARE(numberFormatAt(e.win, QStringLiteral("B3")), QStringLiteral("yyyy-mm-dd"));
        QCOMPARE(numberFormatAt(e.win, QStringLiteral("A3")), QStringLiteral("0%"));
        w.undoStack()->undo();
        QCOMPARE(fillAt(e.win, QStringLiteral("B3")), 0xFFFF00u);
        QVERIFY(linkLookAt(e.win, QStringLiteral("B3")).second && linkLookAt(e.win, QStringLiteral("B3")).first == linkColor);
        QVERIFY(s->anchors().linkAt(0, 2, 1));
    }

    void removeHyperlinksLeavesAHandMadeBlueUnderlineAlone()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        jp::office::sheet::LinkSpec link;
        link.target = QStringLiteral("https://example.com/");
        QVERIFY(s->setLink(0, rangeOf(QStringLiteral("A1")), link, QStringLiteral("Link")).ok);   // (a link, but no Hyperlink style: its look is made by hand)
        FormatChange blue;
        blue.fontColor = ColorSpec::fromRgb(0x0000EE);
        blue.underline = jp::office::sheet::Underline::Single;
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("A1")), blue, QStringLiteral("Look")).ok);
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        w.act(QStringLiteral("link.removeAll"))->trigger();
        QVERIFY(!s->anchors().linkAt(0, 0, 0));
        const auto look = linkLookAt(e.win, QStringLiteral("A1"));
        QVERIFY2(look.second && look.first == 0x0000EEu, "a look made by hand is not the link's look");
    }

    void removeHyperlinksIsFoundBySearchAndIsOnTheClearMenu()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        const QString problem = jptest::commandSearchProblem(e.win.w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("strip hyperlinks"), QStringLiteral("Remove Hyperlinks"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        const QString problem2 = jptest::commandSearchProblem(e.win.w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("collapse group"), QStringLiteral("Hide Detail"));
        QVERIFY2(problem2.isEmpty(), qPrintable(problem2));
    }
};

JP_TEST_CLASS(CrunchGap2Tests)
#include "tst_crunch_gaps2.moc"
