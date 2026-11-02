// Pivot tables in an open workbook (core/editpivot.cpp): Refresh and Refresh All, Change Data Source, the fields and options of a table, Clear, one step of Undo each, and what a save writes: a
// table nobody changed is the file's own to the byte, and the parts of one that was changed are well formed and read back to the cells the sheet holds. The workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/pivotpane.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "io/zip.h"
#include "office/package.h"
#include "savedok.h"
#include "sheet/pivot.h"
#include "sheet/sheetreader.h"

#include <QAction>
#include <QDateTime>
#include <QElapsedTimer>
#include <QListWidget>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

#include <functional>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

using Parts = QMap<QString, QByteArray>;

const QString kRelBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kSheetPart = QStringLiteral("xl/worksheets/sheet1.xml");

QByteArray repack(const QByteArray &zip, const std::function<void(Parts &)> &change)
{
    Parts parts;
    readZip(zip, parts);
    change(parts);
    ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

void addType(Parts &p, const QString &part, const QString &contentType)
{
    p[QStringLiteral("[Content_Types].xml")].replace("</Types>", QStringLiteral("<Override PartName=\"/%1\" ContentType=\"%2\"/>").arg(part, contentType).toUtf8() + "</Types>");
}

void relate(Parts &p, const QString &relsPart, const QString &id, const QString &type, const QString &target)
{
    QByteArray &rels = p[relsPart];
    if (rels.isEmpty()) rels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"></Relationships>";
    rels.replace("</Relationships>", QStringLiteral("<Relationship Id=\"%1\" Type=\"%2%3\" Target=\"%4\"/></Relationships>").arg(id, kRelBase, type, target).toUtf8());
}

QString text(const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t.toHtmlEscaped()); }
QString num(const QString &ref, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

const char *const kNs = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

//        A       B      C        E            F       G      H
//   1  Region  Month  Amount
//   2  East    Jan    10
//   3  East    Jan    5        Sum of Amount  Month
//   4  East    Feb    20       Region         Jan     Feb    Grand Total
//   5  West    Jan    7        East           15      20     35
//   6  West    Feb    3        West           7       7      14
//   7  West    Feb    4        Grand Total    22      27     49
// The table PivotTable1 at E3:H7 (and, with `second`, PivotTable2 at K3:N7 on a cache of its own made from the same cells).
// Options of the workbook: the second table, the tables on a second sheet (Sheet2; the data stays on Sheet1), a workbook that counts days from 1904, and a change of the parts before they are packed.
struct PivotBookOptions {
    bool second = false, otherSheet = false, d1904 = false;
    std::function<void(Parts &)> tweak;
};

QByteArray pivotWorkbook(const PivotBookOptions &o);
QByteArray pivotWorkbook(bool second = false)
{
    PivotBookOptions o;
    o.second = second;
    return pivotWorkbook(o);
}

QByteArray pivotWorkbook(const PivotBookOptions &o)
{
    const bool second = o.second;
    const QString rows = row(1, text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Month")) + text(QStringLiteral("C1"), QStringLiteral("Amount")))
                         + row(2, text(QStringLiteral("A2"), QStringLiteral("East")) + text(QStringLiteral("B2"), QStringLiteral("Jan")) + num(QStringLiteral("C2"), 10))
                         + row(3, text(QStringLiteral("A3"), QStringLiteral("East")) + text(QStringLiteral("B3"), QStringLiteral("Jan")) + num(QStringLiteral("C3"), 5) + text(QStringLiteral("E3"), QStringLiteral("Sum of Amount")) + text(QStringLiteral("F3"), QStringLiteral("Month"))
                                + (second ? text(QStringLiteral("K3"), QStringLiteral("Sum of Amount")) + text(QStringLiteral("L3"), QStringLiteral("Month")) : QString()))
                         + row(4, text(QStringLiteral("A4"), QStringLiteral("East")) + text(QStringLiteral("B4"), QStringLiteral("Feb")) + num(QStringLiteral("C4"), 20) + text(QStringLiteral("E4"), QStringLiteral("Region")) + text(QStringLiteral("F4"), QStringLiteral("Jan"))
                                + text(QStringLiteral("G4"), QStringLiteral("Feb")) + text(QStringLiteral("H4"), QStringLiteral("Grand Total"))
                                + (second ? text(QStringLiteral("K4"), QStringLiteral("Region")) + text(QStringLiteral("L4"), QStringLiteral("Jan")) + text(QStringLiteral("M4"), QStringLiteral("Feb")) + text(QStringLiteral("N4"), QStringLiteral("Grand Total")) : QString()))
                         + row(5, text(QStringLiteral("A5"), QStringLiteral("West")) + text(QStringLiteral("B5"), QStringLiteral("Jan")) + num(QStringLiteral("C5"), 7) + text(QStringLiteral("E5"), QStringLiteral("East")) + num(QStringLiteral("F5"), 15) + num(QStringLiteral("G5"), 20)
                                + num(QStringLiteral("H5"), 35)
                                + (second ? text(QStringLiteral("K5"), QStringLiteral("East")) + num(QStringLiteral("L5"), 15) + num(QStringLiteral("M5"), 20) + num(QStringLiteral("N5"), 35) : QString()))
                         + row(6, text(QStringLiteral("A6"), QStringLiteral("West")) + text(QStringLiteral("B6"), QStringLiteral("Feb")) + num(QStringLiteral("C6"), 3) + text(QStringLiteral("E6"), QStringLiteral("West")) + num(QStringLiteral("F6"), 7) + num(QStringLiteral("G6"), 7)
                                + num(QStringLiteral("H6"), 14)
                                + (second ? text(QStringLiteral("K6"), QStringLiteral("West")) + num(QStringLiteral("L6"), 7) + num(QStringLiteral("M6"), 7) + num(QStringLiteral("N6"), 14) : QString()))
                         + row(7, text(QStringLiteral("A7"), QStringLiteral("West")) + text(QStringLiteral("B7"), QStringLiteral("Feb")) + num(QStringLiteral("C7"), 4) + text(QStringLiteral("E7"), QStringLiteral("Grand Total")) + num(QStringLiteral("F7"), 22)
                                + num(QStringLiteral("G7"), 27) + num(QStringLiteral("H7"), 49)
                                + (second ? text(QStringLiteral("K7"), QStringLiteral("Grand Total")) + num(QStringLiteral("L7"), 22) + num(QStringLiteral("M7"), 27) + num(QStringLiteral("N7"), 49) : QString()));
    QByteArray sheet1 = worksheet(rows), sheet2;
    if (o.otherSheet) {
        // the cells from column E on go to the second sheet
        static const QRegularExpression cell(QStringLiteral("<c r=\"([E-N])(\\d+)\"[^>]*>.*?</c>"), QRegularExpression::DotMatchesEverythingOption);
        QMap<int, QString> pivotRows;
        QString dataRows = rows;
        for (auto m = cell.globalMatch(rows); m.hasNext();) {
            const auto match = m.next();
            pivotRows[match.captured(2).toInt()] += match.captured(0);
        }
        dataRows.remove(cell);
        QString pr;
        for (auto it = pivotRows.begin(); it != pivotRows.end(); ++it) pr += row(it.key(), it.value());
        sheet1 = worksheet(dataRows);
        sheet2 = worksheet(pr);
    }
    const QString pivotSheetRels = o.otherSheet ? QStringLiteral("xl/worksheets/_rels/sheet2.xml.rels") : QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels");
    QVector<QByteArray> sheets{sheet1};
    if (o.otherSheet) sheets.append(sheet2);
    return repack(buildXlsx(sheets, QByteArray(), o.d1904 ? QStringLiteral("date1904=\"1\"") : QString()), [&](Parts &p) {
        int caches = 0;
        for (int n = 1; n <= (second ? 2 : 1); ++n) {
            ++caches;
            const QString def = QStringLiteral("xl/pivotCache/pivotCacheDefinition%1.xml").arg(n), rec = QStringLiteral("xl/pivotCache/pivotCacheRecords%1.xml").arg(n), table = QStringLiteral("xl/pivotTables/pivotTable%1.xml").arg(n);
            addType(p, def, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml"));
            addType(p, rec, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml"));
            addType(p, table, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"));
            p[def] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheDefinition xmlns=\"%1\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId1\" refreshedBy=\"x\" "
                                    "refreshedDate=\"45000\" createdVersion=\"6\" refreshedVersion=\"6\" minRefreshableVersion=\"3\" recordCount=\"6\"><cacheSource type=\"worksheet\"><worksheetSource ref=\"A1:C7\" sheet=\"Sheet1\"/></cacheSource>"
                                    "<cacheFields count=\"3\"><cacheField name=\"Region\" numFmtId=\"0\"><sharedItems count=\"2\"><s v=\"East\"/><s v=\"West\"/></sharedItems></cacheField>"
                                    "<cacheField name=\"Month\" numFmtId=\"0\"><sharedItems count=\"2\"><s v=\"Jan\"/><s v=\"Feb\"/></sharedItems></cacheField>"
                                    "<cacheField name=\"Amount\" numFmtId=\"0\"><sharedItems containsSemiMixedTypes=\"0\" containsString=\"0\" containsNumber=\"1\" containsInteger=\"1\" minValue=\"3\" maxValue=\"20\"/></cacheField>"
                                    "</cacheFields></pivotCacheDefinition>").arg(QLatin1String(kNs)).toUtf8();
            p[rec] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheRecords xmlns=\"%1\" count=\"6\">"
                                    "<r><x v=\"0\"/><x v=\"0\"/><n v=\"10\"/></r><r><x v=\"0\"/><x v=\"0\"/><n v=\"5\"/></r><r><x v=\"0\"/><x v=\"1\"/><n v=\"20\"/></r>"
                                    "<r><x v=\"1\"/><x v=\"0\"/><n v=\"7\"/></r><r><x v=\"1\"/><x v=\"1\"/><n v=\"3\"/></r><r><x v=\"1\"/><x v=\"1\"/><n v=\"4\"/></r></pivotCacheRecords>").arg(QLatin1String(kNs)).toUtf8();
            relate(p, QStringLiteral("xl/pivotCache/_rels/pivotCacheDefinition%1.xml.rels").arg(n), QStringLiteral("rId1"), QStringLiteral("pivotCacheRecords"), QStringLiteral("pivotCacheRecords%1.xml").arg(n));
            p[table] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotTableDefinition xmlns=\"%1\" name=\"PivotTable%2\" cacheId=\"%2\" dataCaption=\"Values\" compact=\"0\" compactData=\"0\">"
                                      "<location ref=\"%3\" firstHeaderRow=\"1\" firstDataRow=\"2\" firstDataCol=\"1\"/><pivotFields count=\"3\">"
                                      "<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>"
                                      "<pivotField axis=\"axisCol\" compact=\"0\" outline=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>"
                                      "<pivotField dataField=\"1\" compact=\"0\" outline=\"0\" showAll=\"0\"/></pivotFields><rowFields count=\"1\"><field x=\"0\"/></rowFields>"
                                      "<rowItems count=\"3\"><i><x/></i><i><x v=\"1\"/></i><i t=\"grand\"><x/></i></rowItems><colFields count=\"1\"><field x=\"1\"/></colFields>"
                                      "<colItems count=\"3\"><i><x/></i><i><x v=\"1\"/></i><i t=\"grand\"><x/></i></colItems>"
                                      "<dataFields count=\"1\"><dataField name=\"Sum of Amount\" fld=\"2\" baseField=\"0\" baseItem=\"0\"/></dataFields></pivotTableDefinition>")
                           .arg(QLatin1String(kNs)).arg(n).arg(n == 1 ? QStringLiteral("E3:H7") : QStringLiteral("K3:N7")).toUtf8();
            relate(p, QStringLiteral("xl/pivotTables/_rels/pivotTable%1.xml.rels").arg(n), QStringLiteral("rId1"), QStringLiteral("pivotCacheDefinition"), QStringLiteral("../pivotCache/pivotCacheDefinition%1.xml").arg(n));
            relate(p, pivotSheetRels, QStringLiteral("rIdPv%1").arg(n), QStringLiteral("pivotTable"), QStringLiteral("../pivotTables/pivotTable%1.xml").arg(n));
            relate(p, QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("rIdPc%1").arg(n), QStringLiteral("pivotCacheDefinition"), QStringLiteral("pivotCache/pivotCacheDefinition%1.xml").arg(n));
        }
        QString caches_ = QStringLiteral("<pivotCaches>");
        for (int n = 1; n <= caches; ++n) caches_ += QStringLiteral("<pivotCache cacheId=\"%1\" r:id=\"rIdPc%1\"/>").arg(n);
        p[QStringLiteral("xl/workbook.xml")].replace("</workbook>", (caches_ + QStringLiteral("</pivotCaches></workbook>")).toUtf8());
        if (o.tweak) o.tweak(p);
    });
}

quint32 rowOf(const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return r;
}

quint32 colOf(const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return c;
}

QString valueOf(EditWindow &e, const QString &ref, int sheet = 0) { return e.win.w.session()->valueAt(sheet, rowOf(ref), colOf(ref)).debug(); }

// The cells E3:H7 (or the range given) as one text, a row to a line.
QString pivotShown(EditWindow &e, const QString &range = QStringLiteral("E3:H7"))
{
    CellRange r;
    parseRangeRef(range, &r);
    QStringList lines;
    for (quint32 row = r.row1; row <= r.row2; ++row) {
        QStringList cells;
        for (quint32 col = r.col1; col <= r.col2; ++col) {
            const jp::calc::Value v = e.win.w.session()->valueAt(0, row, col);
            cells << (v.isEmpty() ? QString() : v.isNumber() ? QString::number(v.number()) : v.text());
        }
        lines << cells.join(QLatin1Char('|'));
    }
    return lines.join(QLatin1Char('\n'));
}

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    const bool started = e.win.w.save();
    return started && (!saved.isEmpty() ? saved.last().value(1).toBool() : savedOk(saved));
}

Parts readParts(const QString &path)
{
    Parts parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) readZip(f.readAll(), parts);
    return parts;
}

// Every pivot table of a saved file is as the schema has it, with its cache and records (the tables are numbered as the fixtures number them, from 1).
void checkShapes(const Parts &parts)
{
    int checked = 0;
    for (int n = 1; n <= 2; ++n) {
        const QString table = QStringLiteral("xl/pivotTables/pivotTable%1.xml").arg(n);
        if (!parts.contains(table)) continue;
        QString why;
        QVERIFY2(pivotCheckShape(parts.value(table), parts.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition%1.xml").arg(n)), parts.value(QStringLiteral("xl/pivotCache/pivotCacheRecords%1.xml").arg(n)), &why),
                 qPrintable(QStringLiteral("table %1: %2").arg(n).arg(why)));
        ++checked;
    }
    QVERIFY(checked > 0);
}

void finish(EditWindow &e)
{
    e.win.w.session()->finishFills();
    e.settle();
}

} // namespace

class CrunchPivotTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // The tables of a workbook are found, and a cell says which table it belongs to.
    void aPivotTableIsFoundWhereTheFileHasIt()
    {
        if (qEnvironmentVariableIsSet("JC_PIVOT_DUMP")) {
            QFile dump(qEnvironmentVariable("JC_PIVOT_DUMP"));
            if (dump.open(QIODevice::WriteOnly)) dump.write(pivotWorkbook());
        }
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->workbook()->pivots().tableCount(), 1);
        QCOMPARE(s->pivotAt(0, rowOf(QStringLiteral("F5")), colOf(QStringLiteral("F5"))), 0);
        QCOMPARE(s->pivotAt(0, rowOf(QStringLiteral("A1")), colOf(QStringLiteral("A1"))), -1);
        QVERIFY(s->pivotUsable(0));
        QCOMPARE(s->pivotSourceText(0), QStringLiteral("Sheet1!A1:C7"));
    }

    // Refresh with the data as it was: no cell changes, and the table shows what the file stored.
    void refreshOfDataThatDidNotChangeChangesNoCell()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        const QString before = pivotShown(e);
        const auto o = s->refreshPivot(0);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(pivotShown(e), before);
        QCOMPARE(before, QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // Refresh after the data changed: every number is worked out again, in one step that Undo takes back and Redo does again.
    void refreshAfterTheDataChangedIsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, rowOf(QStringLiteral("C2")), colOf(QStringLiteral("C2")), QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:15"));   // (a table shows what it was made from until it is refreshed)
        const int steps = s->undoStack()->count();
        const auto o = s->refreshPivot(0);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|105|20|125\nWest|7|7|14\nGrand Total|112|27|139"));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        s->undoStack()->redo();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:105"));
    }

    // The data grows: a new row of the source with a region and a month the table has not seen adds a line and a column; the items keep their order and the new ones come after.
    void changingTheSourceGrowsTheTable()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 7, 0, QStringLiteral("North")).ok);    // A8
        QVERIFY(s->type(0, 7, 1, QStringLiteral("Mar")).ok);      // B8
        QVERIFY(s->type(0, 7, 2, QStringLiteral("9")).ok);        // C8
        QVERIFY(e.settle());
        const int steps = s->undoStack()->count();
        // (the table grows over cells that are empty)
        const auto o = s->changePivotSource(0, QStringLiteral("Sheet1!A1:C8"));
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:I8")), QStringLiteral("Sum of Amount|Month|||\nRegion|Jan|Feb|Mar|Grand Total\nEast|15|20||35\nWest|7|7||14\nNorth|||9|9\nGrand Total|22|27|9|58"));
        QCOMPARE(s->pivotSourceText(0), QStringLiteral("Sheet1!A1:C8"));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        QCOMPARE(s->pivotSourceText(0), QStringLiteral("Sheet1!A1:C7"));
        QCOMPARE(valueOf(e, QStringLiteral("I3")), QStringLiteral("-"));
    }

    // A table that would grow over cells with something in them is not refreshed: nothing changes, and the message says so.
    void aTableThatWouldCoverCellsIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 7, 0, QStringLiteral("North")).ok);
        QVERIFY(s->type(0, 7, 1, QStringLiteral("Mar")).ok);
        QVERIFY(s->type(0, 7, 2, QStringLiteral("9")).ok);
        QVERIFY(s->type(0, rowOf(QStringLiteral("F9")), colOf(QStringLiteral("F9")), QStringLiteral("mine")).ok);   // under where the new line would go
        QVERIFY(s->type(0, rowOf(QStringLiteral("F8")), colOf(QStringLiteral("F8")), QStringLiteral("mine")).ok);
        QVERIFY(e.settle());
        const int steps = s->undoStack()->count();
        const auto o = s->changePivotSource(0, QStringLiteral("Sheet1!A1:C8"));
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QLatin1String("cells that have something in them")), qPrintable(o.problem));
        QCOMPARE(s->undoStack()->count(), steps);
        QCOMPARE(s->pivotSourceText(0), QStringLiteral("Sheet1!A1:C7"));
        QCOMPARE(valueOf(e, QStringLiteral("F8")), QStringLiteral("s:mine"));
    }

    // Moving a field: Month down the side and Region across; the grand totals switched off; the function changed; each one step that Undo takes back.
    void theFieldsAndOptionsOfATableCanBeChanged()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        PivotTable t = *s->workbook()->pivots().table(0).table;
        std::swap(t.rowFields, t.colFields);
        const int steps = s->undoStack()->count();
        auto o = s->setPivot(0, t, true, QStringLiteral("Move fields"));
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H7")), QStringLiteral("Sum of Amount|Region||\nMonth|East|West|Grand Total\nJan|15|7|22\nFeb|20|7|27\nGrand Total|35|14|49"));
        // grand totals off for the columns' side and the function a count
        t = *s->workbook()->pivots().table(0).table;
        t.colGrandTotals = false;
        t.rowGrandTotals = false;
        t.dataFields[0].subtotal = QStringLiteral("count");
        t.dataFields[0].name = QStringLiteral("Count of Amount");
        o = s->setPivot(0, t, false, QStringLiteral("Change"));
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:G6")), QStringLiteral("Count of Amount|Region|\nMonth|East|West\nJan|2|1\nFeb|1|2"));
        QCOMPARE(valueOf(e, QStringLiteral("H7")), QStringLiteral("-"));   // (what the table no longer covers is cleared)
        s->undoStack()->undo();
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // Clear: filters only leaves the fields; all takes every field out and leaves an empty area.
    void clearTakesOutFiltersOrEverything()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        PivotTable t = *s->workbook()->pivots().table(0).table;
        t.fields[0].items[1].hidden = true;   // West hidden
        QVERIFY(s->setPivot(0, t, false, QStringLiteral("Hide West")).ok);
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H6")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nGrand Total|15|20|35"));
        QVERIFY(s->clearPivot(0, true).ok);
        finish(e);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        QVERIFY(s->clearPivot(0, false).ok);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("E4")), QStringLiteral("-"));
        QVERIFY(s->workbook()->pivots().table(0).table->rowFields.isEmpty());
    }

    // What is saved: the parts of a table that was refreshed are well formed, consistent with each other, and read back to the cells of the sheet; the other table is the file's own, to the byte.
    void aRefreshedTableIsSavedWellFormedAndAnotherIsKeptToTheByte()
    {
        const QByteArray book = pivotWorkbook(true);
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, rowOf(QStringLiteral("C2")), colOf(QStringLiteral("C2")), QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        QVERIFY2(s->refreshPivot(0).ok, "refresh");
        finish(e);
        QVERIFY(saveNow(e));
        Parts before;
        readZip(book, before);
        const Parts after = readParts(e.path);
        checkShapes(after);
        // table 2 and its cache: not one byte different
        for (const QString &part : {QStringLiteral("xl/pivotTables/pivotTable2.xml"), QStringLiteral("xl/pivotCache/pivotCacheDefinition2.xml"), QStringLiteral("xl/pivotCache/pivotCacheRecords2.xml")})
            QVERIFY2(after.value(part) == before.value(part), qPrintable(part));
        // table 1: changed, and read back it shows the cells of the sheet
        QVERIFY2(after.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")).contains("v=\"100\""), after.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")).constData());
        QVERIFY(after.value(QStringLiteral("xl/pivotTables/pivotTable1.xml")).contains("E3:H7"));
        PivotCache cache;
        PivotTable table;
        QString why;
        QVERIFY2(readPivotCache(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")), after.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")), jp::calc::DateSystem::Date1900, &cache, &why), qPrintable(why));
        QVERIFY2(readPivotTable(after.value(QStringLiteral("xl/pivotTables/pivotTable1.xml")), &table, &why), qPrintable(why));
        PivotGrid grid;
        QVERIFY2(computePivot(table, cache, &grid, &why), qPrintable(why));
        QCOMPARE(grid.at(2, 1).number, 105.0);
        QCOMPARE(grid.at(4, 3).number, 139.0);
        // and reopened, the sheet has what the table says
        EditWindow again;
        QVERIFY(again.open(readParts(e.path).isEmpty() ? book : [&] { QFile f(e.path); f.open(QIODevice::ReadOnly); return f.readAll(); }(), QStringLiteral("again.xlsx")));
        QCOMPARE(pivotShown(again), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|105|20|125\nWest|7|7|14\nGrand Total|112|27|139"));
    }

    // A refresh that is undone before the save leaves the file's parts as they were: nothing is rewritten.
    void aRefreshThatIsUndoneWritesNothing()
    {
        const QByteArray book = pivotWorkbook();
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        s->undoStack()->undo();
        finish(e);
        QVERIFY(s->type(0, rowOf(QStringLiteral("J1")), colOf(QStringLiteral("J1")), QStringLiteral("1")).ok);
        QVERIFY(saveNow(e));
        Parts before;
        readZip(book, before);
        const Parts after = readParts(e.path);
        checkShapes(after);
        for (const QString &part : {QStringLiteral("xl/pivotTables/pivotTable1.xml"), QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"), QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")})
            QVERIFY2(after.value(part) == before.value(part), qPrintable(part));
    }

    // Refresh All reads every table's cache again; a sheet protected against it refuses.
    void refreshAllAndAProtectedSheet()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(true)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, rowOf(QStringLiteral("C2")), colOf(QStringLiteral("C2")), QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        const int steps = s->undoStack()->count();
        QVERIFY(s->refreshAllPivots().ok);
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:105"));
        QCOMPARE(valueOf(e, QStringLiteral("L5")), QStringLiteral("n:105"));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:15"));
        QCOMPARE(valueOf(e, QStringLiteral("L5")), QStringLiteral("n:15"));
    }

    // The commands of the PivotTable tab work on the table the cursor is in: Refresh, Change Data Source (the cells asked for), and Clear; the tab shows only inside a table.
    void theTabsCommandsWorkOnTheTableTheCursorIsIn()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, rowOf(QStringLiteral("C2")), colOf(QStringLiteral("C2")), QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QCOMPARE(e.win.w.pivotAtCursor(), -1);
        e.select(QStringLiteral("F5"));
        QCOMPARE(e.win.w.pivotAtCursor(), 0);
        e.win.w.act(QStringLiteral("pivot.refresh"))->trigger();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:105"));
        QString asked;
        e.win.w.setPivotSourceAsker([&](QString *where) {
            asked = *where;
            *where = QStringLiteral("Sheet1!A1:C5");
            return true;
        });
        e.win.w.act(QStringLiteral("pivot.changeSource"))->trigger();
        finish(e);
        QCOMPARE(asked, QStringLiteral("Sheet1!A1:C7"));
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|105|20|125\nWest|7||7\nGrand Total|112|20|132"));
    }

    // The fields pane: a field checked out and in, a field moved to the filters, the settings of a value; each one step of Undo, and the pane follows the table.
    void theFieldsPaneChangesTheLayout()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        PivotFieldsPane *pane = e.win.w.pivotPane();
        QVERIFY(pane);
        QCOMPARE(pane->fieldList()->count(), 3);
        QCOMPARE(pane->box(2)->count(), 1);   // Rows: Region
        QCOMPARE(pane->box(1)->count(), 1);   // Columns: Month
        QCOMPARE(pane->box(3)->count(), 1);   // Values: Sum of Amount
        const int steps = s->undoStack()->count();
        // Month to the filters: the table loses its columns and gains a line above it (a blank line between)
        QVERIFY(pane->select(1, 0));
        pane->moveSelected(0);
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(valueOf(e, QStringLiteral("E1")), QStringLiteral("s:Month"));
        QCOMPARE(valueOf(e, QStringLiteral("F1")), QStringLiteral("s:(All)"));
        QCOMPARE(pivotShown(e, QStringLiteral("E3:F7")), QStringLiteral("Sum of Amount|\nRegion|Total\nEast|35\nWest|14\nGrand Total|49"));
        QCOMPARE(pane->box(0)->count(), 1);
        QCOMPARE(pane->box(1)->count(), 0);
        // Region checked out, then in
        pane->checkField(0, false);
        finish(e);
        QCOMPARE(pane->box(2)->count(), 0);
        pane->checkField(0, true);
        finish(e);
        QCOMPARE(pane->box(2)->count(), 1);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:F7")), QStringLiteral("Sum of Amount|\nRegion|Total\nEast|35\nWest|14\nGrand Total|49"));
        // the value shown as an average with a name of its own
        e.win.w.setValueFieldAsker([](QString *name, QString *fn) {
            *name = QStringLiteral("Mean amount");
            *fn = QStringLiteral("average");
            return true;
        });
        QVERIFY(pane->select(3, 0));
        Q_EMIT pane->valueSettingsRequested(0);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("E3")), QStringLiteral("s:Mean amount"));
        QCOMPARE(pivotShown(e, QStringLiteral("E5:F7")), QStringLiteral("East|11.6667\nWest|4.66667\nGrand Total|8.16667"));
        // everything is one step each, undone in order
        while (s->undoStack()->count() > steps && s->undoStack()->canUndo()) {
            s->undoStack()->undo();
            finish(e);
        }
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        QCOMPARE(valueOf(e, QStringLiteral("E1")), QStringLiteral("-"));
    }

    // The Design choices: grand totals, subtotals, and the layout of the report; a field moved from the data to a place it was not (a number among the rows) is saved and read back.
    void theDesignChoicesAndAFieldOfNumbersOnTheRows()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.grand.rows"))->trigger();   // a total for each row: the column at the right stays, the line at the bottom goes
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H6")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14"));
        e.win.w.act(QStringLiteral("pivot.grand.off"))->trigger();
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:G6")), QStringLiteral("Sum of Amount|Month|\nRegion|Jan|Feb\nEast|15|20\nWest|7|7"));
        e.win.w.act(QStringLiteral("pivot.grand.both"))->trigger();
        finish(e);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        // the compact form: one column of labels
        e.win.w.act(QStringLiteral("pivot.layout.compact"))->trigger();
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H7")).split(QLatin1Char('\n')).value(1), QStringLiteral("Row Labels|Jan|Feb|Grand Total"));
        e.win.w.act(QStringLiteral("pivot.layout.tabular"))->trigger();
        finish(e);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        // all of it saved and read back
        QVERIFY(saveNow(e));
        const Parts after = readParts(e.path);
        checkShapes(after);
        PivotCache cache;
        PivotTable table;
        QString why;
        QVERIFY2(readPivotCache(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")), after.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")), jp::calc::DateSystem::Date1900, &cache, &why), qPrintable(why));
        QVERIFY2(readPivotTable(after.value(QStringLiteral("xl/pivotTables/pivotTable1.xml")), &table, &why), qPrintable(why));
        PivotGrid grid;
        QVERIFY2(computePivot(table, cache, &grid, &why), qPrintable(why));
        QCOMPARE(grid.rows, 5);
        QCOMPARE(grid.at(4, 3).number, 49.0);
        Q_UNUSED(s)
    }

    // ---- the review

    // A table that can't be read whole still says where its cells are, so that an edit that would cut through it is refused (it does not shut the whole sheet).
    void aTableThatCannotBeReadStillHasItsLocation()
    {
        PivotBookOptions o;
        o.tweak = [](Parts &p) {
            p[QStringLiteral("xl/pivotTables/pivotTable1.xml")].replace("<rowFields count=\"1\"><field x=\"0\"/></rowFields>", "<rowFields count=\"2\"><field x=\"0\"/><field x=\"0\"/></rowFields>");
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        const QVector<CellRange> ranges = s->workbook()->pivotTablesOn(0);
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(int(ranges[0].row1), 2);
        QCOMPARE(int(ranges[0].col1), 4);
        QCOMPARE(int(ranges[0].row2), 6);
        QCOMPARE(int(ranges[0].col2), 7);
        QCOMPARE(s->pivotAt(0, rowOf(QStringLiteral("F5")), colOf(QStringLiteral("F5"))), 0);
        QVERIFY(!s->pivotUsable(0));
        // a row inserted above the sheet's first lines is refused for the table, but one under the table is not
        QVERIFY(s->insertRows(0, 20, 1).ok);
    }

    // A part nested a hundred thousand deep is not given to a recursive reader, and the table made from it is left as it is.
    void aHostilelyDeepCacheIsLeftAlone()
    {
        const QByteArray book = [] {
            PivotBookOptions o;
            o.tweak = [](Parts &p) {
                QByteArray deep = "<extLst>";
                for (int i = 0; i < 100000; ++i) deep += "<a>";
                for (int i = 0; i < 100000; ++i) deep += "</a>";
                deep += "</extLst>";
                p[QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")].replace("</pivotCacheDefinition>", deep + "</pivotCacheDefinition>");
            };
            return pivotWorkbook(o);
        }();
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(!s->pivotUsable(0));
        QVERIFY(!s->refreshPivot(0).ok);
        QVERIFY(s->type(0, 0, 8, QStringLiteral("1")).ok);
        QVERIFY(saveNow(e));
        Parts before;
        readZip(book, before);
        const Parts after = readParts(e.path);
        for (const QString &part : {QStringLiteral("xl/pivotTables/pivotTable1.xml"), QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"), QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")})
            QVERIFY2(after.value(part) == before.value(part), qPrintable(part));
    }

    // In a workbook that counts days from 1904 the date of a refresh is counted so.
    void theDateOfARefreshFollowsTheWorkbooksCount()
    {
        PivotBookOptions o;
        o.d1904 = true;
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        QVERIFY(saveNow(e));
        const QString def = QString::fromUtf8(readParts(e.path).value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")));
        const QRegularExpression re(QStringLiteral("refreshedDate=\"([0-9.]+)\""));
        const auto m = re.match(def);
        QVERIFY2(m.hasMatch(), qPrintable(def));
        const double expected = double(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()) / 86400.0 + 24107.0;
        QVERIFY2(std::abs(m.captured(1).toDouble() - expected) < 2, qPrintable(m.captured(1)));
    }

    // Rows put in above the data move the range the cache is made from: it is saved so (only that, the rest of the part is the file's), and a refresh of the file saved reads the right cells.
    void theSourceFollowsRowsInsertedAboveIt()
    {
        PivotBookOptions o;
        o.otherSheet = true;
        const QByteArray book = pivotWorkbook(o);
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 1, 1).ok);   // a new row 2 inside the data
        QVERIFY(s->type(0, 1, 0, QStringLiteral("East")).ok);
        QVERIFY(s->type(0, 1, 1, QStringLiteral("Jan")).ok);
        QVERIFY(s->type(0, 1, 2, QStringLiteral("1000")).ok);
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        Parts before;
        readZip(book, before);
        const Parts after = readParts(e.path);
        QVERIFY2(QString::fromUtf8(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"))).contains(QLatin1String("ref=\"A1:C8\"")), "the saved source is not A1:C8");
        QByteArray expected = before.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"));
        expected.replace("ref=\"A1:C7\"", "ref=\"A1:C8\"");
        QCOMPARE(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")), expected);
        QCOMPARE(after.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")), before.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")));
        QCOMPARE(after.value(QStringLiteral("xl/pivotTables/pivotTable1.xml")), before.value(QStringLiteral("xl/pivotTables/pivotTable1.xml")));
        // and reopened, a refresh adds the new row
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(again.open(f.readAll(), QStringLiteral("again.xlsx")));
        QVERIFY(again.win.w.session()->refreshPivot(0).ok);
        finish(again);
        QCOMPARE(valueOf(again, QStringLiteral("F5"), 1), QStringLiteral("n:1015"));
    }

    // A lone value has no place of its own among the columns: a refresh takes the stale one out.
    void aRefreshTakesTheValuesOutOfTheAxesWhenThereIsOne()
    {
        PivotBookOptions o;
        o.tweak = [](Parts &p) {
            p[QStringLiteral("xl/pivotTables/pivotTable1.xml")].replace("<colFields count=\"1\"><field x=\"1\"/></colFields>", "<colFields count=\"2\"><field x=\"-2\"/><field x=\"1\"/></colFields>");
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->workbook()->pivots().table(0).table->colFields.contains(-2));
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        QCOMPARE(s->workbook()->pivots().table(0).table->colFields, (QVector<int>{1}));
        QVERIFY(!s->workbook()->pivots().table(0).table->rowFields.contains(-2));
    }

    // Refresh All refreshes what it can, one step, and says which tables it could not.
    void refreshAllDoesWhatItCanAndSaysWhatItCouldNot()
    {
        PivotBookOptions o;
        o.second = true;
        o.tweak = [](Parts &p) {
            p[QStringLiteral("xl/pivotCache/pivotCacheDefinition2.xml")].replace("sheet=\"Sheet1\"", "sheet=\"Gone\"");
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, rowOf(QStringLiteral("C2")), colOf(QStringLiteral("C2")), QStringLiteral("100")).ok);
        QVERIFY(e.settle());
        const int steps = s->undoStack()->count();
        const auto o2 = s->refreshAllPivots();
        QVERIFY2(o2.ok, qPrintable(o2.problem));
        QVERIFY2(o2.problem.contains(QLatin1String("PivotTable2")), qPrintable(o2.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:105"));
        QCOMPARE(valueOf(e, QStringLiteral("L5")), QStringLiteral("n:15"));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:15"));
    }

    // Merged cells (and the rest that is not content) under the part a table would grow over are in the way as cells with content are.
    void mergedCellsUnderTheGrowthAreInTheWay()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 7, 0, QStringLiteral("North")).ok);
        QVERIFY(s->type(0, 7, 1, QStringLiteral("Mar")).ok);
        QVERIFY(s->type(0, 7, 2, QStringLiteral("9")).ok);
        CellRange merged;
        parseRangeRef(QStringLiteral("I4:I5"), &merged);
        QVERIFY(s->mergeCells(0, merged, crunch::EditSession::MergeKind::Merge).ok);
        QVERIFY(e.settle());
        const int steps = s->undoStack()->count();
        const auto o = s->changePivotSource(0, QStringLiteral("Sheet1!A1:C8"));
        QVERIFY(!o.ok);
        QVERIFY2(o.problem.contains(QLatin1String("something in them")), qPrintable(o.problem));
        QCOMPARE(s->undoStack()->count(), steps);
    }

    // A refresh of a big list, through the session: many different items (thirty thousand rows, three thousand regions), one step, the table right. The time is only bounded loosely (a slow machine
    // takes longer); what grows with the work is counted: the cells of the table.
    void aBigRefreshThroughTheSessionIsOneStepAndBounded()
    {
        const int rows = 30000, regions = 3000;
        QString cells = row(1, text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Month")) + text(QStringLiteral("C1"), QStringLiteral("Amount")));
        double total = 0;
        for (int r = 2; r <= rows + 1; ++r) {
            cells += row(r, text(QStringLiteral("A%1").arg(r), QStringLiteral("R%1").arg(r % regions)) + text(QStringLiteral("B%1").arg(r), QStringLiteral("M%1").arg(r % 12)) + num(QStringLiteral("C%1").arg(r), r % 100));
            total += r % 100;
        }
        PivotBookOptions o;
        o.tweak = [&](Parts &p) {
            p[kSheetPart] = worksheet(cells);
            p[QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")].replace("A1:C7", QStringLiteral("A1:C%1").arg(rows + 1).toUtf8());
            // (the cells the table was saved with are not there: it is made from the list)
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        const int steps = s->undoStack()->count();
        QElapsedTimer t;
        t.start();
        const auto out = s->refreshPivot(0);
        QVERIFY2(out.ok, qPrintable(out.problem));
        finish(e);
        const qint64 ms = t.elapsed();
        qInfo() << "refresh of" << rows << "rows with" << regions << "regions through the session:" << ms << "ms";
        QCOMPARE(s->undoStack()->count(), steps + 1);
        // the table: 2 lines of header, the regions, the grand total; the months across and the grand total
        const int tableRows = 2 + regions + 1, tableCols = 1 + 12 + 1;
        CellRange loc = s->workbook()->pivots().locationOf(0);
        QCOMPARE(int(loc.row2 - loc.row1 + 1), tableRows);
        QCOMPARE(int(loc.col2 - loc.col1 + 1), tableCols);
        QCOMPARE(s->valueAt(0, loc.row2, loc.col2).number(), total);
        QVERIFY2(ms < 120000, qPrintable(QString::number(ms)));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("E4")), QStringLiteral("-"));   // (the file had no cells there)
    }
};

JP_TEST_CLASS(CrunchPivotTests)
#include "tst_crunch_pivot.moc"
