// Pivot tables in an open workbook (core/editpivot.cpp): Refresh and Refresh All, Change Data Source, the fields and options of a table, Clear, one step of Undo each, and what a save writes: a
// table nobody changed is the file's own to the byte, and the parts of one that was changed are well formed and read back to the cells the sheet holds. The workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "app/pivotpane.h"
#include "app/ribbon.h"
#include "app/slicerpane.h"
#include "sheet/chartedit.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "io/zip.h"
#include "office/package.h"
#include "savedok.h"
#include "sheet/oldsave.h"
#include "sheet/pivot.h"
#include "sheet/slicer.h"
#include "sheet/sheetreader.h"

#include <QAbstractButton>
#include <QAction>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QImage>
#include <QListWidget>
#include <QRegularExpression>
#include <QSet>
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
    QByteArray styles;
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
    return repack(buildXlsx(sheets, QByteArray(), o.d1904 ? QStringLiteral("date1904=\"1\"") : QString(), QString(), o.styles), [&](Parts &p) {
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
            p[table] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotTableDefinition xmlns=\"%1\" name=\"PivotTable%2\" cacheId=\"%2\" dataCaption=\"Values\" compact=\"0\" compactData=\"0\" gridDropZones=\"1\">"
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

// The package as the original program reads it: every link a part names is a part that is there; every relationship a workbook names is in its list; every part has a type; and the
// list of pivot caches comes where the format has it in the workbook (after the calculation settings, before anything that follows it).
void checkPackage(const Parts &parts)
{
    const QString workbook = QString::fromUtf8(parts.value(QStringLiteral("xl/workbook.xml")));
    const QString rels = QString::fromUtf8(parts.value(QStringLiteral("xl/_rels/workbook.xml.rels")));
    QSet<QString> ids;
    for (auto m = QRegularExpression(QStringLiteral("Id=\"([^\"]+)\"")).globalMatch(rels); m.hasNext();) {
        const QString id = m.next().captured(1);
        QVERIFY2(!ids.contains(id), qPrintable(QStringLiteral("the workbook's relationship %1 is there twice").arg(id)));
        ids.insert(id);
    }
    for (auto m = QRegularExpression(QStringLiteral("r:id=\"([^\"]+)\"")).globalMatch(workbook); m.hasNext();) {
        const QString id = m.next().captured(1);
        QVERIFY2(ids.contains(id), qPrintable(QStringLiteral("the workbook names %1, which its relationships do not have").arg(id)));
    }
    const QString types = QString::fromUtf8(parts.value(QStringLiteral("[Content_Types].xml")));
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        const QString name = it.key();
        if (name.endsWith(QLatin1String(".rels")) || name == QLatin1String("[Content_Types].xml") || !name.endsWith(QLatin1String(".xml"))) continue;
        QVERIFY2(types.contains(QStringLiteral("PartName=\"/%1\"").arg(name)) || types.contains(QLatin1String("Extension=\"xml\"")), qPrintable(name + QStringLiteral(" has no type")));
    }
    const qsizetype caches = workbook.indexOf(QLatin1String("<pivotCaches>"));
    if (caches >= 0) {
        for (const char *before : {"<calcPr", "<definedNames", "<sheets>"})
            if (const qsizetype at = workbook.indexOf(QLatin1String(before)); at >= 0) QVERIFY2(at < caches, before);
        for (const char *after : {"<extLst", "<webPublishing", "<fileRecoveryPr", "<smartTagPr"})
            if (const qsizetype at = workbook.indexOf(QLatin1String(after)); at >= 0) QVERIFY2(at > caches, after);
    }
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

    // The commands of the pivot table tab work on the table the cursor is in: Refresh, Change Data Source (the cells asked for), and Clear; the tab shows only inside a table.
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
        // (a table with no rows is two lines high, so the cursor on the old last line is outside it; the pane follows the cursor)
        QCOMPARE(pivotShown(e, QStringLiteral("E3:F4")), QStringLiteral("Sum of Amount|Total\nTotal|49"));
        e.select(QStringLiteral("F4"));
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

    // Value Field Settings: a value shown as a share of the total, or against another item, with a number format of its own; the cells take the format, one step of Undo each, and the saved
    // table has the choice and the format (a code that is new gets an id in the styles).
    void aValueIsShownAsAShareWithItsOwnFormat()
    {
        EditWindow e;
        PivotBookOptions o;
        o.styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"));
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        PivotFieldsPane *pane = e.win.w.pivotPane();
        QVERIFY(pane);
        const int steps = s->undoStack()->count();
        ValueFieldSettings seen;
        e.win.w.setValueSettingsAsker([&](ValueFieldSettings *v) {
            seen = *v;
            v->showAs = QStringLiteral("percentOfTotal");
            v->numFmtCode = QStringLiteral("0.0%");
            return true;
        });
        QVERIFY(pane->select(3, 0));
        Q_EMIT pane->valueSettingsRequested(0);
        finish(e);
        QCOMPARE(seen.bases.size(), 2);   // Region and Month
        QCOMPARE(seen.bases[1].items, (QStringList{QStringLiteral("Jan"), QStringLiteral("Feb")}));
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(pivotShown(e, QStringLiteral("E5:H5")), QStringLiteral("East|0.306122|0.408163|0.714286"));
        QCOMPARE(s->formatCodeOf(0, rowOf(QStringLiteral("F5")), colOf(QStringLiteral("F5"))), QStringLiteral("0.0%"));
        QCOMPARE(s->formatCodeOf(0, rowOf(QStringLiteral("H7")), colOf(QStringLiteral("H7"))), QStringLiteral("0.0%"));
        // against Jan: the difference, with no format of its own the format stays
        e.win.w.setValueSettingsAsker([&](ValueFieldSettings *v) {
            v->showAs = QStringLiteral("difference");
            v->baseField = 1;
            v->baseItem = 0;
            v->numFmtCode = QStringLiteral("0.00");
            return true;
        });
        e.select(QStringLiteral("F5"));
        Q_EMIT pane->valueSettingsRequested(0);
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E5:H6")), QStringLiteral("East||5|\nWest||0|"));
        QCOMPARE(s->formatCodeOf(0, rowOf(QStringLiteral("G5")), colOf(QStringLiteral("G5"))), QStringLiteral("0.00"));
        // saved, the part says so
        QVERIFY(saveNow(e));
        const Parts saved = readParts(e.path);
        const QByteArray table = saved.value(QStringLiteral("xl/pivotTables/pivotTable1.xml"));
        QVERIFY2(table.contains("showDataAs=\"difference\""), table.constData());
        QVERIFY(table.contains("baseField=\"1\""));
        QVERIFY(table.contains("numFmtId=\""));
        checkShapes(saved);
        // undone, the numbers are back
        while (s->undoStack()->count() > steps && s->undoStack()->canUndo()) {
            s->undoStack()->undo();
            finish(e);
        }
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // Field Settings: a custom name, a sort, a filter by a value (the items it leaves out are hidden), and the items that show; the options of the table; each one step of Undo.
    void aFieldIsSortedFilteredAndNamedAndTheOptionsAreSet()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        PivotFieldsPane *pane = e.win.w.pivotPane();
        QVERIFY(pane);
        const int steps = s->undoStack()->count();
        // Region, Z to A, with a name of its own
        e.win.w.setFieldSettingsAsker([](FieldSettings *f) {
            f->name = QStringLiteral("Area");
            f->sort = 2;
            return true;
        });
        QVERIFY(pane->select(2, 0));
        pane->fieldSettingsRequested(pane->selectedField());
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(pivotShown(e, QStringLiteral("E4:H7")), QStringLiteral("Area|Jan|Feb|Grand Total\nWest|7|7|14\nEast|15|20|35\nGrand Total|22|27|49"));
        // by a value, then only the top item
        e.win.w.setFieldSettingsAsker([](FieldSettings *f) {
            f->sort = 0;
            f->sortByData = 0;
            f->sortByDescending = true;
            f->filter.kind = jp::office::sheet::PivotFilter::TopItems;
            f->filter.a = 1;
            f->filter.dataField = 0;
            return true;
        });
        e.select(QStringLiteral("F5"));
        QVERIFY(pane->select(2, 0));
        pane->fieldSettingsRequested(pane->selectedField());
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E4:H6")), QStringLiteral("Area|Jan|Feb|Grand Total\nEast|15|20|35\nGrand Total|15|20|35"));
        // the options: a name, and no grand totals for the columns (the row of totals at the bottom)
        e.win.w.setPivotOptionsChoiceAsker([](PivotOptionsChoice *c) {
            c->name = QStringLiteral("Sales");
            c->rowGrandTotals = false;
            return true;
        });
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.options"))->trigger();
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E4:H5")), QStringLiteral("Area|Jan|Feb|Grand Total\nEast|15|20|35"));
        QCOMPARE(s->workbook()->pivots().table(0).table->name, QStringLiteral("Sales"));
        QVERIFY(saveNow(e));
        const Parts saved = readParts(e.path);
        const QByteArray table = saved.value(QStringLiteral("xl/pivotTables/pivotTable1.xml"));
        QVERIFY2(table.contains("name=\"Sales\""), table.constData());
        QVERIFY(table.contains("rowGrandTotals=\"0\""));
        QVERIFY(table.contains("<autoSortScope>"));
        checkShapes(saved);
        while (s->undoStack()->count() > steps && s->undoStack()->canUndo()) {
            s->undoStack()->undo();
            finish(e);
        }
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // Numbers are grouped in steps (the field is still added up by its own values), the group is saved and read again, and Ungroup gives the numbers back.
    void numbersAreGroupedAndUngrouped()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        // Amount in the rows, and also added up
        PivotTable t = *s->workbook()->pivots().table(0).table;
        t.rowFields = {2};
        t.colFields.clear();
        QVERIFY2(s->setPivot(0, t, true, QStringLiteral("Move fields")).ok, "layout");
        finish(e);
        const int steps = s->undoStack()->count();
        const auto o = s->groupPivotField(0, 2, QStringList(), 1, 20, 10, false, false);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        const QString shown = pivotShown(e, QStringLiteral("E3:F7"));
        QVERIFY2(shown.contains(QStringLiteral("1-10|29")), qPrintable(shown));
        QVERIFY2(shown.contains(QStringLiteral("11-20|20")), qPrintable(shown));
        QVERIFY2(shown.contains(QStringLiteral("Grand Total|49")), qPrintable(shown));
        QVERIFY(saveNow(e));
        const Parts saved = readParts(e.path);
        const QByteArray definition = saved.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"));
        QVERIFY2(definition.contains("<fieldGroup base=\"2\">") && definition.contains("startNum=\"1\"") && definition.contains("endNum=\"20\"") && definition.contains("groupInterval=\"10\""), definition.constData());
        QVERIFY(definition.contains("<s v=\"11-20\"/>"));
        checkShapes(saved);
        EditWindow again;
        QVERIFY(again.open([&] { QFile f(e.path); f.open(QIODevice::ReadOnly); return f.readAll(); }(), QStringLiteral("again.xlsx")));
        QVERIFY(again.win.w.session()->pivotUsable(0));
        QVERIFY(again.win.w.session()->refreshPivot(0).ok);
        finish(again);
        QVERIFY2(pivotShown(again, QStringLiteral("E3:F7")).contains(QStringLiteral("1-10|29")), qPrintable(pivotShown(again, QStringLiteral("E3:F7"))));
        // Ungroup
        QVERIFY2(s->ungroupPivotField(0, 2).ok, "ungroup");
        finish(e);
        QVERIFY2(pivotShown(e, QStringLiteral("E3:F9")).contains(QStringLiteral("10|10")), qPrintable(pivotShown(e, QStringLiteral("E3:F9"))));
        s->undoStack()->undo();
        finish(e);
        QVERIFY2(pivotShown(e, QStringLiteral("E3:F7")).contains(QStringLiteral("1-10|29")), "undone");
    }

    // Calculated Field: a field made from a formula over the other fields is added to the values, worked out in each cell from the totals; a bad name, formula, or duplicate is said and changes
    // nothing; it is saved with its formula, read back, kept by a refresh, changed, deleted, and each is one step of Undo.
    void aCalculatedFieldIsAddedChangedAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        const int steps = s->undoStack()->count();
        QVERIFY(!s->setPivotCalculatedField(0, -1, QStringLiteral("Bonus"), QStringLiteral("=Amount*")).ok);
        QVERIFY(!s->setPivotCalculatedField(0, -1, QStringLiteral("Bonus"), QStringLiteral("=Nothing*2")).ok);
        QVERIFY(!s->setPivotCalculatedField(0, -1, QStringLiteral("Amount"), QStringLiteral("=Amount*2")).ok);
        QVERIFY(!s->setPivotCalculatedField(0, -1, QString(), QStringLiteral("=Amount*2")).ok);
        QCOMPARE(s->undoStack()->count(), steps);
        const auto made = s->setPivotCalculatedField(0, -1, QStringLiteral("Square"), QStringLiteral("=Amount*Amount"));
        QVERIFY2(made.ok, qPrintable(made.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(s->undoStack()->undoText(), QStringLiteral("Insert Calculated Field"));
        const PivotTable *t = s->workbook()->pivots().table(0).table.get();
        QCOMPARE(t->dataFields.size(), 2);
        QCOMPARE(t->dataFields[1].name, QStringLiteral("Sum of Square"));
        // (the total of Amount, squared: 49 * 49 -- the formula works on the totals of a cell, not on each record)
        QVERIFY2(pivotShown(e, QStringLiteral("E3:K9")).contains(QStringLiteral("2401")), qPrintable(pivotShown(e, QStringLiteral("E3:K9"))));
        QVERIFY(saveNow(e));
        const Parts saved = readParts(e.path);
        const QByteArray definition = saved.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"));
        QVERIFY2(definition.contains("name=\"Square\"") && definition.contains("formula=\"Amount*Amount\"") && definition.contains("databaseField=\"0\""), definition.constData());
        const QByteArray table = saved.value(QStringLiteral("xl/pivotTables/pivotTable1.xml"));
        QVERIFY2(table.contains("<pivotFields count=\"4\">") && table.contains("dragToRow=\"0\"") && table.contains("fld=\"3\""), table.constData());
        QVERIFY(saved.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")).contains("<r><x v=\"0\"/><x v=\"0\"/><n v=\"10\"/></r>"));
        checkShapes(saved);
        // read back, and read again from the data
        EditWindow again;
        QVERIFY(again.open([&] { QFile f(e.path); f.open(QIODevice::ReadOnly); return f.readAll(); }(), QStringLiteral("again.xlsx")));
        QVERIFY(again.win.w.session()->pivotUsable(0));
        QVERIFY(again.win.w.session()->refreshPivot(0).ok);
        finish(again);
        QVERIFY2(pivotShown(again, QStringLiteral("E3:K9")).contains(QStringLiteral("2401")), qPrintable(pivotShown(again, QStringLiteral("E3:K9"))));
        // changed through the dialog's own hook, then deleted
        e.win.w.setCalculatedFieldAsker([](jp::CalculatedFieldChoice *c) {
            c->name = QStringLiteral("Square");
            c->formula = QStringLiteral("=Amount*2");
            return true;
        });
        e.win.w.act(QStringLiteral("pivot.calculatedField"))->trigger();
        finish(e);
        QCOMPARE(s->undoStack()->undoText(), QStringLiteral("Change Calculated Field"));
        QVERIFY2(pivotShown(e, QStringLiteral("E3:K9")).contains(QStringLiteral("98")), qPrintable(pivotShown(e, QStringLiteral("E3:K9"))));
        e.win.w.setCalculatedFieldAsker([](jp::CalculatedFieldChoice *c) {
            c->name = QStringLiteral("Square");
            c->action = jp::CalculatedFieldChoice::Delete;
            return true;
        });
        e.win.w.act(QStringLiteral("pivot.calculatedField"))->trigger();
        finish(e);
        QCOMPARE(s->undoStack()->undoText(), QStringLiteral("Delete Calculated Field"));
        QCOMPARE(s->workbook()->pivots().table(0).table->dataFields.size(), 1);
        QVERIFY2(pivotShown(e, QStringLiteral("E3:H7")).contains(QStringLiteral("East|15|20|35")), qPrintable(pivotShown(e)));
        s->undoStack()->undo();
        finish(e);
        QVERIFY2(pivotShown(e, QStringLiteral("E3:K9")).contains(QStringLiteral("98")), "undone");
    }

    // List Formulas puts the calculated fields and their formulas on a new sheet; a table with none says so and makes nothing.
    void theFormulasOfCalculatedFieldsAreListedOnANewSheet()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        const int sheets = int(s->workbook()->info().sheets.size());
        e.win.w.act(QStringLiteral("pivot.listFormulas"))->trigger();
        QCOMPARE(int(s->workbook()->info().sheets.size()), sheets);
        QVERIFY(e.win.told.last().contains(QLatin1String("no calculated fields")));
        QVERIFY(s->setPivotCalculatedField(0, -1, QStringLiteral("Bonus"), QStringLiteral("=Amount*0.1")).ok);
        QVERIFY(s->setPivotCalculatedField(0, -1, QStringLiteral("Share"), QStringLiteral("=Bonus/Amount")).ok);
        finish(e);
        int made = -1;
        const auto o = s->listPivotFormulas(0, &made);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(int(s->workbook()->info().sheets.size()), sheets + 1);
        QCOMPARE(s->workbook()->info().sheets[made].name, QStringLiteral("Formulas 1"));
        QCOMPARE(s->valueAt(made, 0, 0).text(), QStringLiteral("Calculated Field"));
        QCOMPARE(s->valueAt(made, 1, 0).text(), QStringLiteral("Bonus"));
        QCOMPARE(s->valueAt(made, 1, 1).text(), QStringLiteral("=Amount*0.1"));
        QCOMPARE(s->valueAt(made, 2, 1).text(), QStringLiteral("=Bonus/Amount"));
    }

    // A double click on a value shows the rows behind it on a new sheet; a label is edited as always.
    void theRowsBehindAValueAreShownOnANewSheet()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->isPivotValue(0, rowOf(QStringLiteral("F5")), colOf(QStringLiteral("F5"))));
        QVERIFY(!s->isPivotValue(0, rowOf(QStringLiteral("E5")), colOf(QStringLiteral("E5"))));   // (a label)
        const int sheets = int(s->workbook()->info().sheets.size());
        int made = -1;
        const auto o = s->showPivotDetail(0, rowOf(QStringLiteral("F5")), colOf(QStringLiteral("F5")), &made);   // East, Jan: two rows
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(int(s->workbook()->info().sheets.size()), sheets + 1);
        QVERIFY(made > 0);
        QCOMPARE(s->valueAt(made, 0, 0).debug(), QStringLiteral("s:Region"));
        QCOMPARE(s->valueAt(made, 1, 0).debug(), QStringLiteral("s:East"));
        QCOMPARE(s->valueAt(made, 1, 2).debug(), QStringLiteral("n:10"));
        QCOMPARE(s->valueAt(made, 2, 2).debug(), QStringLiteral("n:5"));
        QVERIFY(s->valueAt(made, 3, 0).isEmpty());
        // the grand total shows all six rows
        const auto all = s->showPivotDetail(0, rowOf(QStringLiteral("H7")), colOf(QStringLiteral("H7")));
        QVERIFY2(all.ok, qPrintable(all.problem));
        finish(e);
        // a label has no rows
        QVERIFY(!s->showPivotDetail(0, rowOf(QStringLiteral("E5")), colOf(QStringLiteral("E5"))).ok);
    }

    // The search box of the fields pane hides the fields that don't match; Defer Layout Update keeps changes in the boxes until Update, and then they are one step.
    void theFieldsPaneSearchesAndDefersAnUpdate()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        PivotFieldsPane *pane = e.win.w.pivotPane();
        QVERIFY(pane);
        pane->search(QStringLiteral("mON"));
        QVERIFY(pane->fieldList()->item(0)->isHidden());
        QVERIFY(!pane->fieldList()->item(1)->isHidden());
        pane->search(QString());
        QVERIFY(!pane->fieldList()->item(0)->isHidden());
        const int steps = s->undoStack()->count();
        pane->setDeferred(true);
        QVERIFY(pane->select(1, 0));
        pane->moveSelected(0);   // Month to the filters
        QVERIFY(pane->hasPending());
        QCOMPARE(pane->box(0)->count(), 1);
        QCOMPARE(pane->box(1)->count(), 0);
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps);
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        pane->applyDeferred();
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QVERIFY(!pane->hasPending());
        QCOMPARE(pivotShown(e, QStringLiteral("E3:F7")), QStringLiteral("Sum of Amount|\nRegion|Total\nEast|35\nWest|14\nGrand Total|49"));
    }

    // Changes waiting for Update belong to the table they were made on: another table picked meanwhile is not written over, and the waiting changes are dropped.
    void deferredChangesAreNotWrittenOverAnotherTable()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(true)));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        PivotFieldsPane *pane = e.win.w.pivotPane();
        QVERIFY(pane);
        pane->setDeferred(true);
        QVERIFY(pane->select(1, 0));
        pane->moveSelected(0);   // Month to the filters of the first table, waiting
        QVERIFY(pane->hasPending());
        const int steps = s->undoStack()->count();
        e.select(QStringLiteral("L5"));   // the second table
        finish(e);
        QVERIFY(!pane->hasPending());
        pane->applyDeferred();
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps);
        QCOMPARE(pivotShown(e, QStringLiteral("K3:N7")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H7")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // A table whose data says to be read again when the file opens is read again once, as one step, after the first screen; the tables of a file are held to a total of cells, and the rest
    // wait for Refresh, with a note.
    void tablesAreReadAgainOnOpenWithinABound()
    {
        PivotBookOptions o;
        o.tweak = [](Parts &p) {
            const QString def = QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml");
            p[def].replace("<pivotCacheDefinition ", "<pivotCacheDefinition refreshOnLoad=\"1\" ");
        };
        {
            EditWindow e;
            QVERIFY(e.open(pivotWorkbook(o)));
            QTRY_COMPARE(e.win.w.session()->undoStack()->count(), 1);
        }
        crunch::EditSession::setRefreshOnOpenLimit(1);
        const struct Reset { ~Reset() { crunch::EditSession::setRefreshOnOpenLimit(0); } } reset;
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        QVERIFY(e.settle());
        QTest::qWait(50);
        QCOMPARE(e.win.w.session()->undoStack()->count(), 0);
        QVERIFY2(e.win.w.statusText().contains(QStringLiteral("not read again")), qPrintable(e.win.w.statusText()));
    }

    // Value Field Settings in a workbook with no cell formats says that a number format can't be kept, instead of dropping it without a word.
    void aNumberFormatThatCantBeKeptIsSaid()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        QStringList said;
        e.win.w.setNotifier([&](const QString &, const QString &text) { said << text; });
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        e.win.w.setValueSettingsAsker([](ValueFieldSettings *v) {
            v->numFmtCode = QStringLiteral("0.0%");
            return true;
        });
        Q_EMIT e.win.w.pivotPane()->valueSettingsRequested(0);
        finish(e);
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QStringLiteral("no cell formats")), qPrintable(said[0]));
    }

    // A pivot table made in this session has no part in the file yet: a copy in the older form still says it can't hold it.
    void aNewPivotTableIsListedAsNotInACopy()
    {
        const QString rows = row(1, text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Amount")))
                             + row(2, text(QStringLiteral("A2"), QStringLiteral("East")) + num(QStringLiteral("B2"), 10)) + row(3, text(QStringLiteral("A3"), QStringLiteral("West")) + num(QStringLiteral("B3"), 5));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(!jp::office::sheet::featuresNotInCopy(*s->workbook(), jp::office::sheet::CopyKind::Ods).join(QLatin1Char('|')).contains(QStringLiteral("pivot tables")));
        QVERIFY(s->createPivot(QStringLiteral("Sheet1!A1:B3"), 0, 0, 4).ok);
        QVERIFY(jp::office::sheet::featuresNotInCopy(*s->workbook(), jp::office::sheet::CopyKind::Ods).join(QLatin1Char('|')).contains(QStringLiteral("pivot tables")));
        QVERIFY(jp::office::sheet::featuresNotInCopy(*s->workbook(), jp::office::sheet::CopyKind::Xls).join(QLatin1Char('|')).contains(QStringLiteral("pivot tables")));
    }

    // Move pivot table: the table is drawn again at the cell given on its sheet, the cells it left are empty, one step of Undo; a place that has something in it is refused.
    void aPivotTableIsMovedToAnotherPlaceOnItsSheet()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        const QString before = pivotShown(e);
        QCOMPARE(before, QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
        const int steps = s->undoStack()->count();
        QVERIFY(!s->movePivot(0, 0, 0).ok);   // (the source data is there)
        const auto o = s->movePivot(0, 19, 9);   // J20
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(int(s->workbook()->pivots().locationOf(0).row1), 19);
        QCOMPARE(int(s->workbook()->pivots().locationOf(0).col1), 9);
        QCOMPARE(pivotShown(e, QStringLiteral("J20:M24")), before);
        QCOMPARE(pivotShown(e), QStringLiteral("|||\n|||\n|||\n|||\n|||"));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(pivotShown(e), before);
        QCOMPARE(pivotShown(e, QStringLiteral("J20:M24")), QStringLiteral("|||\n|||\n|||\n|||\n|||"));
        QCOMPARE(int(s->workbook()->pivots().locationOf(0).row1), 2);
    }

    // Drill Up shuts the item the cell names (the lines under it are not shown, its total stays) and Drill Down opens it again; each is one step of Undo; the last field's items have nothing under them.
    void anItemIsShutAndOpenedFromItsLabel()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::PivotTable two = *s->workbook()->pivots().table(0).table;
        two.colFields.clear();
        two.rowFields = {0, 1};
        QVERIFY(s->setPivot(0, two, true, QStringLiteral("Two rows")).ok);
        finish(e);
        auto labels = [&] {
            QStringList out;
            for (quint32 r = 2; r < 16; ++r) {
                for (quint32 c = 4; c < 6; ++c) {
                    const jp::calc::Value v = s->valueAt(0, r, c);
                    if (!v.isEmpty()) out << (v.isNumber() ? QString::number(v.number()) : v.text());
                }
            }
            return out.join(QLatin1Char('|'));
        };
        const QString open = labels();
        QVERIFY2(open.contains(QStringLiteral("East|Jan|Feb|East Total|West|Jan|Feb")), qPrintable(open + QStringLiteral("\n") + pivotShown(e, QStringLiteral("E3:I14"))));
        const int steps = s->undoStack()->index();
        // the cell of "East" (the first line under the head)
        quint32 east = 0;
        for (quint32 r = 2; r < 16; ++r)
            if (s->valueAt(0, r, 4).isText() && s->valueAt(0, r, 4).text() == QLatin1String("East")) east = r;
        QVERIFY(east > 0);
        QVERIFY2(s->drillPivotItem(0, east, 4, false).ok, "drill up");
        finish(e);
        QCOMPARE(s->undoStack()->index(), steps + 1);
        QVERIFY2(labels().contains(QStringLiteral("East|West|Jan|Feb|West Total")), qPrintable(labels()));
        QVERIFY(s->drillPivotItem(0, east, 4, true).ok);
        finish(e);
        QCOMPARE(labels(), open);
        // an item of the last field has nothing under it
        quint32 jan = 0;
        for (quint32 r = 2; r < 16; ++r)
            if (s->valueAt(0, r, 5).isText() && s->valueAt(0, r, 5).text() == QLatin1String("Jan")) jan = r;
        QVERIFY(!s->drillPivotItem(0, jan, 5, false).ok);
        // a cell that is not a label
        QVERIFY(!s->drillPivotItem(0, east, 6, false).ok);
        s->undoStack()->undo();
        finish(e);
        QVERIFY2(labels().contains(QStringLiteral("East|West|Jan|Feb|West Total")), qPrintable(labels()));
    }

    // A pivot table with a filter above it moves with it: the old area, filter included, is empty, and a formula that pointed at the filter's cell follows.
    void aPivotTableIsMovedWithItsFilters()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::PivotTable t = *s->workbook()->pivots().table(0).table;
        t.colFields.clear();
        jp::office::sheet::PivotPageField page;
        page.fld = 1;
        t.pageFields.append(page);
        QVERIFY2(s->setPivot(0, t, true, QStringLiteral("Filter")).ok, "filter");
        finish(e);
        quint32 pageRow = 99;
        for (quint32 r = 0; r < 3; ++r)
            if (s->valueAt(0, r, 4).isText() && s->valueAt(0, r, 4).text() == QLatin1String("Month")) pageRow = r;
        QVERIFY(pageRow < 3);
        QVERIFY(s->type(0, 11, 0, QStringLiteral("=F%1").arg(pageRow + 1)).ok);
        finish(e);
        const auto o = s->movePivot(0, 19, 9);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        // the old area is empty (the page filter's cells and the table's)
        for (quint32 r = 0; r < 10; ++r)
            for (quint32 c = 4; c < 9; ++c) QVERIFY2(s->valueAt(0, r, c).isEmpty(), qPrintable(QStringLiteral("%1,%2").arg(r).arg(c)));
        QCOMPARE(s->valueAt(0, pageRow + 17, 9).text(), QStringLiteral("Month"));
        QCOMPARE(s->editText(0, 11, 0), QStringLiteral("=K%1").arg(pageRow + 17 + 1));
        QCOMPARE(int(s->workbook()->pivots().locationOf(0).row1), 19);
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(s->valueAt(0, pageRow, 4).text(), QStringLiteral("Month"));
        QCOMPARE(s->editText(0, 11, 0), QStringLiteral("=F%1").arg(pageRow + 1));
    }

    // Report Filter Pages: a copy of the table on a new sheet for each item of a filter, the sheet named for the item, the filter set to it; one step of Undo.
    void reportFilterPagesMakeASheetForEachItem()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::PivotTable t = *s->workbook()->pivots().table(0).table;
        t.colFields.clear();
        jp::office::sheet::PivotPageField page;
        page.fld = 1;
        t.pageFields.append(page);
        QVERIFY2(s->setPivot(0, t, true, QStringLiteral("Filter")).ok, "filter");
        finish(e);
        auto live = [&] {
            QStringList names;
            for (const auto &se : s->workbook()->info().sheets)
                if (!se.deleted) names << se.name;
            return names;
        };
        QCOMPARE(live().size(), 1);
        const int steps = s->undoStack()->count();
        int first = -1;
        const auto o = s->showReportFilterPages(0, 0, &first);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QStringList names = live();
        names.sort();
        QCOMPARE(names, (QStringList{QStringLiteral("Feb"), QStringLiteral("Jan"), QStringLiteral("Sheet1")}));
        QCOMPARE(int(s->workbook()->pivots().tableCount()), 3);
        auto holds = [&](int sheet, double number) {
            for (quint32 r = 0; r < 30; ++r)
                for (quint32 c = 0; c < 12; ++c) {
                    const auto v = s->valueAt(sheet, r, c);
                    if (v.isNumber() && v.number() == number) return true;
                }
            return false;
        };
        int jan = -1, feb = -1;
        for (int i = 0; i < s->workbook()->info().sheets.size(); ++i) {
            if (s->workbook()->info().sheets[i].name == QLatin1String("Jan")) jan = i;
            if (s->workbook()->info().sheets[i].name == QLatin1String("Feb")) feb = i;
        }
        QVERIFY(jan >= 0 && feb >= 0 && first >= 0);
        QVERIFY(holds(jan, 22) && !holds(jan, 27));   // (Jan: East 15 and West 7)
        QVERIFY(holds(feb, 20) && !holds(feb, 22));
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(live(), (QStringList{QStringLiteral("Sheet1")}));
        QCOMPARE(int(s->workbook()->pivots().tableCount()), 1);
        // a table with no filter has none to show pages for
        QVERIFY(!s->showReportFilterPages(0, 5).ok);
    }

    // The command is on only in a table that has a filter, and then it makes the pages.
    void theReportFilterPagesCommandNeedsAFilter()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        QVERIFY(!e.win.w.act(QStringLiteral("pivot.reportPages"))->isEnabled());
        jp::office::sheet::PivotTable t = *s->workbook()->pivots().table(0).table;
        t.colFields.clear();
        jp::office::sheet::PivotPageField page;
        page.fld = 1;
        t.pageFields.append(page);
        QVERIFY2(s->setPivot(0, t, true, QStringLiteral("Filter")).ok, "filter");
        finish(e);
        const jp::office::sheet::CellRange where = s->workbook()->pivots().locationOf(0);
        e.view()->selectRange(where.row1 + 1, where.col1 + 1, where.row1 + 1, where.col1 + 1);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("pivot.reportPages"))->isEnabled());
        e.win.w.act(QStringLiteral("pivot.reportPages"))->trigger();
        finish(e);
        QCOMPARE(int(s->workbook()->pivots().tableCount()), 3);
    }

    // What sits where the pivot table would go (merged cells that are empty, here) is found before anything is moved: nothing changes, and the reason is told.
    void aPivotTableIsNotMovedOverMergedCells()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        CellRange merged;
        parseRangeRef(QStringLiteral("K21:L22"), &merged);
        QVERIFY(s->mergeCells(0, merged, crunch::EditSession::MergeKind::Merge).ok);
        finish(e);
        const int steps = s->undoStack()->index();
        const auto o = s->movePivot(0, 19, 9);   // J20:M24 covers them
        QVERIFY(!o.ok);
        QVERIFY(o.problem.contains(QStringLiteral("merged")));
        QCOMPARE(s->undoStack()->index(), steps);
        QCOMPARE(int(s->workbook()->pivots().locationOf(0).row1), 2);
    }

    // A formula that points at a cell of the pivot table follows it when the table is moved, as it follows cells that are cut and pasted; one that points elsewhere stays, and Undo is one step.
    void formulasPointingAtAMovedPivotTableFollowIt()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 11, 0, QStringLiteral("=F5")).ok);
        QVERIFY(s->type(0, 11, 1, QStringLiteral("=SUM(F5:H5)+C2")).ok);
        QVERIFY(s->type(0, 11, 2, QStringLiteral("=C2")).ok);
        finish(e);
        QCOMPARE(s->valueAt(0, 11, 1).number(), 70.0 + 10);
        const int steps = s->undoStack()->count();
        const auto o = s->movePivot(0, 19, 9);   // J20: 17 rows down and 5 columns right
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(s->editText(0, 11, 0), QStringLiteral("=K22"));
        QCOMPARE(s->editText(0, 11, 1), QStringLiteral("=SUM(K22:M22)+C2"));
        QCOMPARE(s->editText(0, 11, 2), QStringLiteral("=C2"));
        QCOMPARE(s->valueAt(0, 11, 0).number(), 15.0);
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(s->editText(0, 11, 0), QStringLiteral("=F5"));
        QCOMPARE(s->editText(0, 11, 1), QStringLiteral("=SUM(F5:H5)+C2"));
        QCOMPARE(int(s->workbook()->pivots().locationOf(0).row1), 2);
        // a place with something in it is refused, and nothing is left half done
        const int again = s->undoStack()->index();
        QVERIFY(!s->movePivot(0, 11, 0).ok);
        finish(e);
        QCOMPARE(s->undoStack()->index(), again);
        QCOMPARE(s->editText(0, 11, 0), QStringLiteral("=F5"));
    }

    // Changes waiting for Update are dropped too when the SAME table is changed under them (an Undo, say): Update then writes nothing.
    void deferredChangesAreDroppedWhenTheSameTableChangesByUndo()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        PivotFieldsPane *pane = e.win.w.pivotPane();
        QVERIFY(pane);
        jp::office::sheet::PivotTable renamed = *s->workbook()->pivots().table(0).table;
        renamed.name = QStringLiteral("Sales");
        QVERIFY(s->setPivot(0, renamed, false, QStringLiteral("Rename")).ok);
        finish(e);
        pane->setDeferred(true);
        QVERIFY(pane->select(1, 0));
        pane->moveSelected(0);
        QVERIFY(pane->hasPending());
        s->undoStack()->undo();   // the table is the one it was before the rename
        finish(e);
        QVERIFY(!pane->hasPending());
        const int steps = s->undoStack()->count();
        pane->applyDeferred();
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps);
        QCOMPARE(s->workbook()->pivots().table(0).table->name, QStringLiteral("PivotTable1"));
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // The cells read again when a file opens are counted over all its tables: the first fits and is read, the second does not and waits, with a note.
    void theReadingOnOpenIsBoundedOverSeveralTables()
    {
        PivotBookOptions o;
        o.second = true;
        o.tweak = [](Parts &p) {
            for (int n = 1; n <= 2; ++n)
                p[QStringLiteral("xl/pivotCache/pivotCacheDefinition%1.xml").arg(n)].replace("<pivotCacheDefinition ", "<pivotCacheDefinition refreshOnLoad=\"1\" ");
        };
        crunch::EditSession::setRefreshOnOpenLimit(20);   // (a cache of the book is 6 records of 3 fields: 18 cells)
        const struct Reset { ~Reset() { crunch::EditSession::setRefreshOnOpenLimit(0); } } reset;
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        QTRY_COMPARE(e.win.w.session()->undoStack()->count(), 1);
        QTRY_VERIFY2(e.win.w.statusText().contains(QStringLiteral("1 pivot table")), qPrintable(e.win.w.statusText()));
    }

    // A workbook whose table has a slicer on Region (East chosen in the part), a timeline on Month, and the slicer on the sheet.
    static PivotBookOptions slicerBook(int copies = 1, bool both = false)
    {
        PivotBookOptions o;
        o.second = both;
        o.tweak = [copies, both](Parts &p) {
            addType(p, QStringLiteral("xl/slicerCaches/slicerCache1.xml"), QStringLiteral("application/vnd.ms-excel.slicerCache+xml"));
            addType(p, QStringLiteral("xl/timelineCaches/timelineCache1.xml"), QStringLiteral("application/vnd.ms-excel.timelineCache+xml"));
            addType(p, QStringLiteral("xl/slicers/slicer1.xml"), QStringLiteral("application/vnd.ms-excel.slicer+xml"));
            p[QStringLiteral("xl/slicerCaches/slicerCache1.xml")] =
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<slicerCacheDefinition xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" name=\"Slicer_Region\" sourceName=\"Region\">"
                "" + [&] { QByteArray c; for (int i = 0; i < copies; ++i) c += "<pivotTable tabId=\"1\" name=\"PivotTable1\"/>"; if (both) c += "<pivotTable tabId=\"1\" name=\"PivotTable2\"/>"; return QByteArray("<pivotTables>") + c + "</pivotTables>"; }() + "<data><tabular pivotCacheId=\"1\"><items count=\"2\"><i x=\"0\" s=\"1\"/><i x=\"1\"/></items></tabular></data></slicerCacheDefinition>";
            p[QStringLiteral("xl/timelineCaches/timelineCache1.xml")] =
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<timelineCacheDefinition xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2010/11/main\" name=\"NativeTimeline_Month\" sourceName=\"Month\">"
                "<pivotTables><pivotTable tabId=\"1\" name=\"PivotTable1\"/></pivotTables><state name=\"Timeline1\"><selection startDate=\"2020-01-01T00:00:00\" endDate=\"2020-02-29T00:00:00\"/></state></timelineCacheDefinition>";
            p[QStringLiteral("xl/slicers/slicer1.xml")] =
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<slicers xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\"><slicer name=\"Region\" cache=\"Slicer_Region\" caption=\"Region\" rowHeight=\"241300\"/></slicers>";
            relate(p, QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("rIdSc1"), QStringLiteral("slicerCache"), QStringLiteral("slicerCaches/slicerCache1.xml"));
            if (copies > 1) {   // (the same part named by more relationships)
                relate(p, QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("rIdSc2"), QStringLiteral("slicerCache"), QStringLiteral("slicerCaches/slicerCache1.xml"));
                relate(p, QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("rIdSc3"), QStringLiteral("slicerCache"), QStringLiteral("slicerCaches/slicerCache1.xml"));
            }
            relate(p, QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("rIdTc1"), QStringLiteral("timelineCache"), QStringLiteral("timelineCaches/timelineCache1.xml"));
            relate(p, QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels"), QStringLiteral("rIdSl1"), QStringLiteral("slicer"), QStringLiteral("../slicers/slicer1.xml"));
        };
        return o;
    }

    // The workbook names its slicer and timeline caches and the sheet its slicers: they are found from the relationships, read once, and the file keeps them as it had them when saved untouched.
    void slicersAndTimelinesAreFoundFromTheWorkbook()
    {
        PivotBookOptions o = slicerBook();
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        const auto caches = s->workbook()->slicerCaches();
        QCOMPARE(caches.size(), 2);
        QCOMPARE(caches[0].sourceName, QStringLiteral("Region"));
        QVERIFY(caches[1].timeline);
        const auto views = s->workbook()->slicerViews(0);
        QCOMPARE(views.size(), 1);
        QCOMPARE(views[0].caption, QStringLiteral("Region"));
        // an edit elsewhere keeps the three parts as they were
        const Parts before = readParts(e.path);
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(saveNow(e));
        const Parts after = readParts(e.path);
        for (const QString &name : {QStringLiteral("xl/slicerCaches/slicerCache1.xml"), QStringLiteral("xl/timelineCaches/timelineCache1.xml"), QStringLiteral("xl/slicers/slicer1.xml")})
            QVERIFY2(after.value(name) == before.value(name) && !before.value(name).isEmpty(), qPrintable(name));
    }

    // A click on a slicer's button filters the table (one step of Undo, and Undo brings the table back), the slicer part is written with the choice, and a table that was not changed leaves the part alone.
    void aSlicerClickFiltersTheTableAndIsWrittenBack()
    {
        const QString items = QStringLiteral("xl/slicerCaches/slicerCache1.xml");
        auto itemsOf = [&](const QString &path) {
            const QByteArray part = readParts(path).value(items);
            const int at = part.indexOf("<items");
            return QString::fromUtf8(part.mid(at, part.indexOf("</items>", at) + 8 - at));
        };
        const QString allShown = QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49");
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(slicerBook())));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.slicers"))->trigger();
        SlicerPane *pane = e.win.w.slicerPane();
        QVERIFY(pane);
        QCOMPARE(pane->slicerCount(), 2);   // the slicer and the timeline
        QVERIFY(pane->button(0, 1));
        QCOMPARE(pane->button(0, 1)->text(), QStringLiteral("West"));
        // (the table shows both regions: both buttons are on, though the file's part chose East)
        QVERIFY(pane->button(0, 0)->isChecked() && pane->button(0, 1)->isChecked());
        const int steps = s->undoStack()->count();
        pane->button(0, 1)->click();
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H6")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nWest|7|7|14\nGrand Total|7|7|14"));
        QVERIFY(pane->button(0, 1)->isChecked());
        QVERIFY(!pane->button(0, 0)->isChecked());
        QVERIFY(saveNow(e));
        QCOMPARE(itemsOf(e.path), QStringLiteral("<items count=\"2\"><i x=\"0\"/><i x=\"1\" s=\"1\"/></items>"));
        // Undo brings the table back; with no filter on, no item is marked
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(pivotShown(e), allShown);
        QVERIFY(pane->button(0, 0)->isChecked() && pane->button(0, 1)->isChecked());
        QVERIFY(saveNow(e));
        QCOMPARE(itemsOf(e.path), QStringLiteral("<items count=\"2\"><i x=\"0\"/><i x=\"1\"/></items>"));
        // Redo puts the filter back, and the part
        s->undoStack()->redo();
        finish(e);
        QVERIFY(saveNow(e));
        QCOMPARE(itemsOf(e.path), QStringLiteral("<items count=\"2\"><i x=\"0\"/><i x=\"1\" s=\"1\"/></items>"));
    }

    // Ctrl-click adds an item to the ones shown, or takes it away; a click on the only one shown clears the filter.
    void aCtrlClickAddsOrTakesAwayAnItem()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(slicerBook())));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.slicers"))->trigger();
        SlicerPane *pane = e.win.w.slicerPane();
        QVERIFY(pane);
        auto shownItems = [&] {
            QSet<qint32> shown;
            qint64 count = 0;
            [&] { QVERIFY(s->slicerChoice(0, &shown, &count)); }();
            return shown;
        };
        QCOMPARE(shownItems(), (QSet<qint32>{0, 1}));
        pane->clickItem(0, 0, true);   // Ctrl on East while all show: East goes
        finish(e);
        QCOMPARE(shownItems(), (QSet<qint32>{1}));
        pane->clickItem(0, 0, true);   // Ctrl on East again: East is added
        finish(e);
        QCOMPARE(shownItems(), (QSet<qint32>{0, 1}));
        pane->clickItem(0, 1, false);   // a click: only West
        finish(e);
        QCOMPARE(shownItems(), (QSet<qint32>{1}));
        pane->clickItem(0, 1, false);   // a click on the only one shown: the filter goes
        finish(e);
        QCOMPARE(shownItems(), (QSet<qint32>{0, 1}));
        QCOMPARE(pivotShown(e), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // A slicer that names a table many times filters it once; one with tables of other data does not give them its choice (its items are places in the first table's data).
    void aSlicerFiltersEachTableOnceAndOnlyThoseOfItsData()
    {
        {
            EditWindow e;
            QVERIFY(e.open(pivotWorkbook(slicerBook(5000))));
            crunch::EditSession *s = e.win.w.session();
            const auto caches = s->workbook()->slicerCaches();
            QCOMPARE(slicerTargets(*s->workbook(), caches[0]).size(), 1);
            const int steps = s->undoStack()->count();
            e.select(QStringLiteral("F5"));
            QVERIFY(s->chooseSlicerItems(0, QSet<qint32>{1}).ok);
            QCOMPARE(s->undoStack()->count(), steps + 1);
            QCOMPARE(s->undoStack()->command(steps)->childCount(), 1);   // (one change of the table, not 5,000)
        }
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(slicerBook(1, true))));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(slicerTargets(*s->workbook(), s->workbook()->slicerCaches()[0]).size(), 2);
        QVERIFY(s->chooseSlicerItems(0, QSet<qint32>{1}).ok);
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("E3:H6")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nWest|7|7|14\nGrand Total|7|7|14"));
        // (the second table is made from a cache of its own: it is not filtered by places in the first one's)
        QCOMPARE(pivotShown(e, QStringLiteral("K3:N7")), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // The slicer parts are read once, however many relationships name them and however often they are asked for; the pane is filled again when a table changed, not when the cursor moved.
    void slicersAreReadOnceAndThePaneIsFilledWhenATableChanged()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(slicerBook(3))));
        crunch::EditSession *s = e.win.w.session();
        const int before = jp::office::sheet::slicerPartsParsed();
        for (int i = 0; i < 5; ++i) {
            QCOMPARE(s->workbook()->slicerCaches().size(), 2);
            QCOMPARE(int(s->slicers().size()), 2);
        }
        QVERIFY2(jp::office::sheet::slicerPartsParsed() - before <= 3, qPrintable(QString::number(jp::office::sheet::slicerPartsParsed() - before)));   // (the two caches and the slicer list)
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.slicers"))->trigger();
        const int fills = e.win.w.slicerFills();
        for (const char *cell : {"D1", "E3", "F6", "A1", "G5"}) e.select(QString::fromLatin1(cell));
        QCOMPARE(e.win.w.slicerFills(), fills);
        e.select(QStringLiteral("F5"));
        e.win.w.slicerPane()->clickItem(0, 1, false);
        finish(e);
        QVERIFY(e.win.w.slicerFills() > fills);
    }

    // A slicer with a million items shows a thousand buttons and a note of the rest; an & in an item or a caption is shown as written.
    void aSlicerPaneIsCappedAndShowsAmpersands()
    {
        SlicerPane pane;
        SlicerPane::Entry e;
        e.caption = QStringLiteral("R&D");
        e.usable = true;
        for (int i = 0; i < 1500; ++i) e.items << QStringLiteral("item %1").arg(i);
        e.items[0] = QStringLiteral("Q&A");
        e.chosen = QVector<bool>(1500, true);
        pane.setEntries({e});
        QVERIFY(pane.button(0, 999));
        QVERIFY(!pane.button(0, 1000));
        QCOMPARE(pane.button(0, 0)->text(), QStringLiteral("Q&&A"));   // (the button shows one &)
        QVERIFY(pane.hasMoreNote(0));
        QCOMPARE(pane.moreNoteText(0), QStringLiteral("and 500 more items"));
        QCOMPARE(pane.captionOf(0), QStringLiteral("R&&D"));
    }

    // The workbook's slicer parts are not patched when they are not UTF-8 (the table's choice is not written, and nothing is damaged): see the library tests; here a deleted sheet's
    // pivot table is written when the sheet is back.
    void aPivotTableOnADeletedSheetIsNotMarkedSavedAndIsWrittenWhenTheSheetIsBack()
    {
        PivotBookOptions o;
        o.otherSheet = true;
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        // (the table is changed, and its sheet is deleted: the save writes nothing of the table, so the file's copy is still the old one)
        jp::office::sheet::PivotTable renamed = *s->workbook()->pivots().table(0).table;
        renamed.name = QStringLiteral("Sales");
        QVERIFY(s->setPivot(0, renamed, false, QStringLiteral("Rename")).ok);
        QVERIFY(s->deleteSheet(1).ok);
        finish(e);
        QVERIFY(saveNow(e));
        QVERIFY(s->workbook()->pivots().tableChanged(0));
        auto tableParts = [&] {
            int n = 0;
            const Parts parts = readParts(e.path);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it) n += it.key().startsWith(QLatin1String("xl/pivotTables/pivotTable")) && it.key().endsWith(QLatin1String(".xml"));
            return n;
        };
        QCOMPARE(tableParts(), 0);
        if (!s->undoStack()->canUndo() || s->undoStack()->undoText().isEmpty()) QSKIP("an Undo of a delete that was saved is refused: nothing to bring back");
        s->undoStack()->undo();
        finish(e);
        if (s->workbook()->info().sheets.size() < 2 || s->workbook()->info().sheets[1].deleted) QSKIP("the delete was not undone");
        QVERIFY(saveNow(e));
        QCOMPARE(tableParts(), 1);
    }

    // Insert > Pivot Chart: a chart of the table's values without its grand totals, marked as the table's chart (the saved chart has the table's name); one step of Undo.
    void aPivotChartIsMadeFromATable()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        const int steps = s->undoStack()->count();
        e.win.w.act(QStringLiteral("insert.pivotChart"))->trigger();
        finish(e);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QVERIFY(saveNow(e));
        const Parts saved = readParts(e.path);
        QByteArray chart;
        for (auto it = saved.cbegin(); it != saved.cend(); ++it)
            if (it.key().startsWith(QLatin1String("xl/charts/chart")) && it.key().endsWith(QLatin1String(".xml"))) chart = it.value();
        QVERIFY(!chart.isEmpty());
        QVERIFY2(chart.contains("<c:pivotSource>") && chart.contains("Sheet1!PivotTable1"), chart.constData());
        QCOMPARE(chart.count("<c:ser>"), 2);   // Jan and Feb: the grand total column is left out
        QVERIFY(chart.contains("$F$4:$F$4") || chart.contains("$F$4"));
        s->undoStack()->undo();
        finish(e);
        QVERIFY(saveNow(e));
        bool any = false;
        const Parts after = readParts(e.path);
        for (auto it = after.cbegin(); it != after.cend(); ++it) any = any || it.key().startsWith(QLatin1String("xl/charts/chart"));
        QVERIFY(!any);
    }

    // A new pivot chart shows its field buttons (saved as the chart's pivot options); Field Buttons in the elements list turns kinds off and on, as one step of Undo, and the file keeps each.
    void aPivotChartHasFieldButtons()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("insert.pivotChart"))->trigger();
        finish(e);
        QTRY_VERIFY(e.win.w.pickedObject() >= 0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        auto fire = [&](const QStringList &path) {
            QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
            QMenu *m = menu;
            QAction *found = nullptr;
            for (int i = 0; i < path.size(); ++i) {
                found = nullptr;
                for (QAction *a : m->actions())
                    if (a->text() == path[i]) found = a;
                if (!found) return false;
                if (i + 1 < path.size()) m = found->menu();
            }
            found->trigger();
            return true;
        };
        auto savedChart = [&] {
            const Parts saved = readParts(e.path);
            QByteArray chart;
            for (auto it = saved.cbegin(); it != saved.cend(); ++it)
                if (it.key().startsWith(QLatin1String("xl/charts/chart")) && it.key().endsWith(QLatin1String(".xml"))) chart = it.value();
            return chart;
        };
        auto zone = [&](const char *name) {
            const QByteArray chart = savedChart();
            return chart.contains(QByteArray("<c14:") + name + " val=\"1\"/>") ? 1 : chart.contains(QByteArray("<c14:") + name + " val=\"0\"/>") ? 0 : -1;
        };
        QVERIFY(saveNow(e));
        QVERIFY2(zone("dropZonesVisible") == 1 && zone("dropZoneFilter") == 1 && zone("dropZoneSeries") == 1, savedChart().constData());
        const int steps = e.win.w.session()->undoStack()->count();
        QString told;
        e.win.w.setNotifier([&told](const QString &, const QString &text) { told = text; });
        QVERIFY(fire({"Field Buttons", "Legend Field Buttons"}));
        QVERIFY2(told.isEmpty(), qPrintable(told));
        QCOMPARE(e.win.w.session()->undoStack()->count(), steps + 1);
        QVERIFY(saveNow(e));
        QVERIFY2(zone("dropZoneSeries") == 0 && zone("dropZoneCategories") == 1, savedChart().constData());
        QVERIFY(fire({"Field Buttons", "Hide All"}));
        QVERIFY(saveNow(e));
        QVERIFY2(zone("dropZoneFilter") == 0 && zone("dropZoneCategories") == 0 && zone("dropZoneData") == 0, savedChart().constData());
        pressUndo(&e.win.w);
        pressUndo(&e.win.w);
        QVERIFY(saveNow(e));
        QVERIFY2(zone("dropZoneSeries") == 1 && zone("dropZoneData") == 1, savedChart().constData());
    }

    // A pivot chart has the Chart tab of any chart: its look is changed there, the saved chart is still the table's chart, and a refresh of the table leaves what was changed.
    void aPivotChartHasTheChartTab()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("insert.pivotChart"))->trigger();
        finish(e);
        QTRY_VERIFY(e.win.w.pickedObject() >= 0);
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.changeType"))->isEnabled());
        QVERIFY(e.win.w.act(QStringLiteral("chartdesign.saveAsPicture"))->isEnabled());
        QVERIFY(e.win.w.changePickedChart(QStringLiteral("Chart Title"), [](jp::office::Chart *c) { jp::office::sheet::setChartTitleText(c, QStringLiteral("Sales by month")); }));
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        QVERIFY(saveNow(e));
        QByteArray chart;
        const Parts saved = readParts(e.path);
        for (auto it = saved.cbegin(); it != saved.cend(); ++it)
            if (it.key().startsWith(QLatin1String("xl/charts/chart")) && it.key().endsWith(QLatin1String(".xml"))) chart = it.value();
        QVERIFY2(chart.contains("<c:pivotSource>") && chart.contains("Sales by month"), chart.constData());
    }

    // The file's copy of a pivot table is the one just saved: save, change the table, save, undo the change, and save again leaves the file with the table as it is on screen (the second
    // file is read again to see it).
    void aPivotTableSavedChangedAndUndoneIsSavedAsItIs()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));   // (something to save the first time)
        QVERIFY(saveNow(e));
        jp::office::sheet::PivotTable renamed = *s->workbook()->pivots().table(0).table;
        renamed.name = QStringLiteral("Sales");
        QVERIFY(s->setPivot(0, renamed, false, QStringLiteral("Rename")).ok);
        finish(e);
        QVERIFY(saveNow(e));
        QVERIFY(readParts(e.path).value(QStringLiteral("xl/pivotTables/pivotTable1.xml")).contains("name=\"Sales\""));
        s->undoStack()->undo();   // the table is as it was before the rename
        finish(e);
        QCOMPARE(s->workbook()->pivots().table(0).table->name, QStringLiteral("PivotTable1"));
        QVERIFY(saveNow(e));
        const QByteArray part = readParts(e.path).value(QStringLiteral("xl/pivotTables/pivotTable1.xml"));
        QVERIFY2(part.contains("name=\"PivotTable1\"") && !part.contains("name=\"Sales\""), part.left(300).constData());
        // read again, the table is the one on screen
        QFile saved(e.path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(again.open(saved.readAll(), QStringLiteral("again.xlsx")));
        QCOMPARE(again.win.w.session()->workbook()->pivots().table(0).table->name, QStringLiteral("PivotTable1"));
        QCOMPARE(pivotShown(again), QStringLiteral("Sum of Amount|Month||\nRegion|Jan|Feb|Grand Total\nEast|15|20|35\nWest|7|7|14\nGrand Total|22|27|49"));
    }

    // A pivot table made in this session and saved is the file's own after that: changed and saved again, the file has one table part and one cache, holding the change, and the same after an Undo.
    void aNewPivotTableSavedThenChangedIsSavedOnce()
    {
        const QString rows = row(1, text(QStringLiteral("A1"), QStringLiteral("Region")) + text(QStringLiteral("B1"), QStringLiteral("Amount")))
                             + row(2, text(QStringLiteral("A2"), QStringLiteral("East")) + num(QStringLiteral("B2"), 10)) + row(3, text(QStringLiteral("A3"), QStringLiteral("West")) + num(QStringLiteral("B3"), 5));
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->createPivot(QStringLiteral("Sheet1!A1:B3"), 0, 0, 4).ok);
        finish(e);
        const QString was = s->workbook()->pivots().table(0).table->name;
        QVERIFY(saveNow(e));
        auto count = [&](const QString &prefix, const QString &suffix) {
            int n = 0;
            const Parts parts = readParts(e.path);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                if (it.key().startsWith(prefix) && it.key().endsWith(suffix)) ++n;
            return n;
        };
        auto tablePart = [&] {
            const Parts parts = readParts(e.path);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                if (it.key().startsWith(QLatin1String("xl/pivotTables/pivotTable")) && it.key().endsWith(QLatin1String(".xml"))) return it.value();
            return QByteArray();
        };
        QCOMPARE(count(QStringLiteral("xl/pivotTables/pivotTable"), QStringLiteral(".xml")), 1);
        QVERIFY(tablePart().contains(("name=\"" + was + "\"").toUtf8()));
        jp::office::sheet::PivotTable renamed = *s->workbook()->pivots().table(0).table;
        renamed.name = QStringLiteral("Sales");
        QVERIFY(s->setPivot(0, renamed, false, QStringLiteral("Rename")).ok);
        finish(e);
        QVERIFY(saveNow(e));
        QCOMPARE(count(QStringLiteral("xl/pivotTables/pivotTable"), QStringLiteral(".xml")), 1);
        QCOMPARE(count(QStringLiteral("xl/pivotCache/pivotCacheDefinition"), QStringLiteral(".xml")), 1);
        QVERIFY2(tablePart().contains("name=\"Sales\""), tablePart().left(300).constData());
        s->undoStack()->undo();
        finish(e);
        QVERIFY(saveNow(e));
        QCOMPARE(count(QStringLiteral("xl/pivotTables/pivotTable"), QStringLiteral(".xml")), 1);
        QCOMPARE(count(QStringLiteral("xl/pivotCache/pivotCacheDefinition"), QStringLiteral(".xml")), 1);
        QVERIFY2(tablePart().contains(("name=\"" + was + "\"").toUtf8()) && !tablePart().contains("name=\"Sales\""), tablePart().left(300).constData());
        QFile saved(e.path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(again.open(saved.readAll(), QStringLiteral("again.xlsx")));
        QCOMPARE(again.win.w.session()->workbook()->pivots().tableCount(), 1);
        QCOMPARE(again.win.w.session()->workbook()->pivots().table(0).table->name, was);
    }

    // A pivot chart's name in the file is made again at each save from the table's name, the sheet's, and the file's: a rename of the table or the sheet, and a Save As, are followed.
    void aPivotChartsNameFollowsRenamesAndSaveAs()
    {
        auto chartOf = [](const QString &path) {
            const Parts saved = readParts(path);
            QByteArray chart;
            for (auto it = saved.cbegin(); it != saved.cend(); ++it)
                if (it.key().startsWith(QLatin1String("xl/charts/chart")) && it.key().endsWith(QLatin1String(".xml"))) chart = it.value();
            const int at = chart.indexOf("<c:pivotSource>");
            return at < 0 ? QString() : QString::fromUtf8(chart.mid(at, chart.indexOf("</c:pivotSource>", at) - at));
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("insert.pivotChart"))->trigger();
        finish(e);
        QVERIFY(saveNow(e));
        QVERIFY2(chartOf(e.path).contains(QStringLiteral("[book.xlsx]Sheet1!PivotTable1")), qPrintable(chartOf(e.path)));
        // the table is renamed
        jp::office::sheet::PivotTable renamed = *s->workbook()->pivots().table(0).table;
        renamed.name = QStringLiteral("Sales");
        QVERIFY(s->setPivot(0, renamed, false, QStringLiteral("Rename")).ok);
        QVERIFY(saveNow(e));
        QVERIFY2(chartOf(e.path).contains(QStringLiteral("[book.xlsx]Sheet1!Sales")), qPrintable(chartOf(e.path)));
        // the sheet is renamed
        QVERIFY(s->renameSheet(0, QStringLiteral("Data")).ok);
        QVERIFY(saveNow(e));
        QVERIFY2(chartOf(e.path).contains(QStringLiteral("[book.xlsx]Data!Sales")), qPrintable(chartOf(e.path)));
        // the file is saved under another name
        const QString other = QFileInfo(e.path).dir().filePath(QStringLiteral("other.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(other));
        QVERIFY(savedOk(saved));
        QVERIFY2(chartOf(other).contains(QStringLiteral("[other.xlsx]Data!Sales")), qPrintable(chartOf(other)));
    }

    // A picture of the Slicers pane beside a pivot table, with one button chosen (only when JC_SHOT_DIR names a folder).
    void aPictureOfASlicerAndATimeline()
    {
        const QString dir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
        if (dir.isEmpty()) QSKIP("set JC_SHOT_DIR to take the picture");
        QDir().mkpath(dir);
        PivotBookOptions o = slicerBook();
        o.styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"));
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        e.win.w.resize(1440, 900);
        e.win.w.show();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.slicers"))->trigger();
        SlicerPane *pane = e.win.w.slicerPane();
        QVERIFY(pane);
        pane->button(0, 1)->click();
        finish(e);
        if (Ribbon *r = e.win.w.ribbon()) r->showTab(r->tab(QStringLiteral("Pivot Table")));
        QCoreApplication::processEvents();
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/slicers.png")));
    }

    // Pictures of the pivot table tab, the pane, and the dialogs, for a person to look at (only when JC_SHOT_DIR names a folder).
    void picturesOfThePivotFeatures()
    {
        const QString dir = QString::fromLocal8Bit(qgetenv("JC_SHOT_DIR"));
        if (dir.isEmpty()) QSKIP("set JC_SHOT_DIR to take the pictures");
        QDir().mkpath(dir);
        EditWindow e;
        PivotBookOptions shotOptions;
        shotOptions.styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"));
        QVERIFY(e.open(pivotWorkbook(shotOptions)));
        e.win.w.resize(1440, 900);
        e.win.w.show();
        crunch::EditSession *s = e.win.w.session();
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.fieldList"))->trigger();
        e.win.w.act(QStringLiteral("pivot.style.PivotStyleMedium2"))->trigger();
        finish(e);
        e.win.w.act(QStringLiteral("pivot.style.rowStripes"))->setChecked(true);
        e.win.w.act(QStringLiteral("pivot.style.rowStripes"))->trigger();
        finish(e);
        if (Ribbon *r = e.win.w.ribbon()) r->showTab(r->tab(QStringLiteral("Pivot Table")));
        QCoreApplication::processEvents();
        e.win.w.grab().save(dir + QStringLiteral("/pivot-tab-and-pane.png"));
        // a value shown as a percent of the grand total, the rows sorted by value, and the second table's chart
        e.win.w.setValueSettingsAsker([](ValueFieldSettings *v) {
            v->showAs = QStringLiteral("percentOfTotal");
            v->numFmtCode = QStringLiteral("0.0%");
            return true;
        });
        e.select(QStringLiteral("F5"));
        Q_EMIT e.win.w.pivotPane()->valueSettingsRequested(0);
        finish(e);
        e.win.w.act(QStringLiteral("insert.pivotChart"))->trigger();
        finish(e);
        // (the chart is drawn when the event loop gets to it: the picture waits until the chart's place on the sheet has more than the sheet's white and its lines)
        QTRY_VERIFY([&] {
            QSet<QRgb> colors;
            const QImage shot = e.win.w.grab().toImage().copy(QRect(180, 380, 440, 250));
            for (int y = 0; y < shot.height(); y += 3)
                for (int x = 0; x < shot.width(); x += 3) colors.insert(shot.pixel(x, y));
            return colors.size() > 8;
        }());
        QCoreApplication::processEvents();
        e.win.w.grab().save(dir + QStringLiteral("/pivot-percent-and-chart.png"));
        // the dialogs
        ValueFieldSettings vs;
        vs.name = QStringLiteral("Sum of Amount");
        vs.sourceName = QStringLiteral("Amount");
        vs.showAs = QStringLiteral("difference");
        vs.baseField = 1;
        vs.bases = {{0, QStringLiteral("Region"), {QStringLiteral("East"), QStringLiteral("West")}}, {1, QStringLiteral("Month"), {QStringLiteral("Jan"), QStringLiteral("Feb")}}};
        vs.numFmtCode = QStringLiteral("0.00");
        ValueFieldDialog vd(vs);
        vd.show();
        QCoreApplication::processEvents();
        vd.grab().save(dir + QStringLiteral("/pivot-value-settings.png"));
        FieldSettings fs;
        fs.sourceName = fs.name = QStringLiteral("Region");
        fs.dataNames = {QStringLiteral("Sum of Amount")};
        fs.items = {{QStringLiteral("East"), 0, true}, {QStringLiteral("West"), 1, false}};
        FieldSettingsDialog fd(fs);
        fd.show();
        QCoreApplication::processEvents();
        fd.grab().save(dir + QStringLiteral("/pivot-field-settings.png"));
        GroupChoice gc;
        gc.isDate = true;
        gc.levels = {QStringLiteral("months"), QStringLiteral("years")};
        gc.startText = QStringLiteral("1/15/2020");
        gc.endText = QStringLiteral("1/10/2021");
        GroupDialog gd(gc);
        gd.show();
        QCoreApplication::processEvents();
        gd.grab().save(dir + QStringLiteral("/pivot-group.png"));
        PivotOptionsChoice oc;
        oc.hasTable = true;
        oc.name = QStringLiteral("PivotTable1");
        PivotOptionsDialog od(oc);
        od.show();
        QCoreApplication::processEvents();
        od.grab().save(dir + QStringLiteral("/pivot-options.png"));
        Q_UNUSED(s);
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
        QVERIFY(!table.fields[0].subtotalTop);   // the tabular form has its subtotals at the bottom of a group
        // the compact form puts them on top
        e.win.w.act(QStringLiteral("pivot.layout.compact"))->trigger();
        finish(e);
        QVERIFY(saveNow(e));
        PivotTable compact;
        QVERIFY2(readPivotTable(readParts(e.path).value(QStringLiteral("xl/pivotTables/pivotTable1.xml")), &compact, &why), qPrintable(why));
        QVERIFY(compact.fields[0].subtotalTop);
        Q_UNUSED(s)
    }

    // The selection a sheet's view keeps for a table points at the lines it had: after the table changes size it is dropped (the original program will not open a file that has it outside the table).
    void theSelectionOfTheViewIsDroppedWhenTheTableChanges()
    {
        PivotBookOptions o;
        o.tweak = [](Parts &p) {
            p[kSheetPart].replace("<sheetData>", "<sheetViews><sheetView workbookViewId=\"0\"><pivotSelection pane=\"bottomRight\" axis=\"axisRow\" start=\"2\" max=\"3\" r:id=\"rIdPv1\"><pivotArea dataOnly=\"0\" outline=\"0\" fieldPosition=\"0\"/></pivotSelection></sheetView></sheetViews><sheetData>");
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        e.select(QStringLiteral("F5"));
        e.win.w.act(QStringLiteral("pivot.grand.off"))->trigger();
        finish(e);
        QVERIFY(saveNow(e));
        const Parts after = readParts(e.path);
        checkShapes(after);
        QVERIFY(!after.value(kSheetPart).contains("pivotSelection"));
    }

    // A cache whose data is in another workbook (its source names a link) is not read again from this one, is not moved when rows are put in, and is saved as the file has it.
    void aSourceInAnotherWorkbookIsLeftAlone()
    {
        PivotBookOptions o;
        o.otherSheet = true;
        o.tweak = [](Parts &p) {
            p[QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")].replace("<worksheetSource ref=\"A1:C7\" sheet=\"Sheet1\"/>", "<worksheetSource ref=\"A1:C7\" sheet=\"Sheet1\" r:id=\"rId9\"/>");
        };
        const QByteArray book = pivotWorkbook(o);
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->workbook()->pivots().cache(0).external);
        const auto refreshed = s->refreshPivot(0);
        QVERIFY(!refreshed.ok);
        QVERIFY2(refreshed.problem.contains(QLatin1String("another workbook")), qPrintable(refreshed.problem));
        QVERIFY(s->insertRows(0, 1, 1).ok);   // a row inside what would be the data
        finish(e);
        QVERIFY2(!s->workbook()->pivots().cache(0).source.valid, qPrintable(QString::number(s->workbook()->pivots().cache(0).source.row2)));
        QVERIFY(saveNow(e));
        Parts original;
        readZip(book, original);
        const Parts after = readParts(e.path);
        QVERIFY2(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")) == original.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")), after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")).constData());
    }

    // The selection a sheet's view keeps for a table is left as it is in a sheet that is written for something else (a table that was not changed has no reason to lose it).
    void anUntouchedTablesSelectionSurvivesASaveOfOtherCells()
    {
        PivotBookOptions o;
        o.tweak = [](Parts &p) {
            p[kSheetPart].replace("<sheetData>", "<sheetViews><sheetView workbookViewId=\"0\"><pivotSelection pane=\"bottomRight\" axis=\"axisRow\" start=\"2\" max=\"3\" r:id=\"rIdPv1\"><pivotArea dataOnly=\"0\" outline=\"0\" fieldPosition=\"0\"/></pivotSelection></sheetView></sheetViews><sheetData>");
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 20, 25, QStringLiteral("5")).ok);
        finish(e);
        QVERIFY(saveNow(e));
        QVERIFY(readParts(e.path).value(kSheetPart).contains("pivotSelection"));
    }

    // GETPIVOTDATA in a cell: sums, a total, the grand total, and #REF! for what the table does not show; it follows the table when the table changes.
    void getPivotDataIsWorkedOutFromTheTable()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 9, 9, QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",$E$3,\"Region\",\"East\",\"Month\",\"Jan\")")).ok);
        QVERIFY(s->type(0, 10, 9, QStringLiteral("=GETPIVOTDATA(\"Amount\",E3,\"Region\",\"West\")")).ok);
        QVERIFY(s->type(0, 11, 9, QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",F5)")).ok);
        QVERIFY(s->type(0, 12, 9, QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",$E$3,\"Region\",\"North\")")).ok);
        QVERIFY(s->type(0, 13, 9, QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",A1)")).ok);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("J10")), QStringLiteral("n:15"));
        QCOMPARE(valueOf(e, QStringLiteral("J11")), QStringLiteral("n:14"));
        QCOMPARE(valueOf(e, QStringLiteral("J12")), QStringLiteral("n:49"));
        QCOMPARE(valueOf(e, QStringLiteral("J13")), QStringLiteral("e:#REF!"));
        QCOMPARE(valueOf(e, QStringLiteral("J14")), QStringLiteral("e:#REF!"));
        // the data changes and the table is read again: the cell follows
        QVERIFY(s->type(0, rowOf(QStringLiteral("C2")), colOf(QStringLiteral("C2")), QStringLiteral("100")).ok);
        finish(e);
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("J10")), QStringLiteral("n:105"));
    }

    // A pivot table made by a person: an empty table with its cache, the layout filled in from the fields, saved with the parts the original program writes, read again, and taken back by Undo.
    void aPivotTableIsMadeSavedAndReadAgain()
    {
        // (a workbook with a calculation chain: the save takes the chain out, and the workbook's relationships, which it changes for that, must keep the link to the new cache)
        PivotBookOptions chain;
        chain.tweak = [](Parts &p) {
            p[QStringLiteral("xl/calcChain.xml")] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<calcChain xmlns=\"%1\"><c r=\"A1\" i=\"1\"/></calcChain>").arg(QLatin1String(kNs)).toUtf8();
            addType(p, QStringLiteral("xl/calcChain.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml"));
            relate(p, QStringLiteral("xl/_rels/workbook.xml.rels"), QStringLiteral("rIdChain"), QStringLiteral("calcChain"), QStringLiteral("calcChain.xml"));
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(chain)));
        crunch::EditSession *s = e.win.w.session();
        const int steps = s->undoStack()->count();
        int made = -1;
        QVERIFY(!s->createPivot(QStringLiteral("Sheet1!A1:C1"), 0, 9, 9).ok);          // a row of names and nothing under it
        QVERIFY(!s->createPivot(QStringLiteral("Sheet1!A1:C7"), 0, 2, 4).ok);          // over the table that is there
        QVERIFY(!s->createPivot(QStringLiteral("Sheet1!A1:C7"), 0, 0, 0).ok);          // over the data
        const auto o = s->createPivot(QStringLiteral("Sheet1!A1:C7"), 0, 9, 9, &made);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QCOMPARE(made, 1);
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(s->pivotAt(0, 9, 9), 1);
        QCOMPARE(s->pivotAt(0, 26, 11), 1);
        // the fields drive it as they do any table
        PivotTable t = *s->workbook()->pivots().table(1).table;
        QCOMPARE(t.fields.size(), 3);
        t.rowFields = {0};
        PivotDataField d;
        d.fld = 2;
        d.name = QStringLiteral("Sum of Amount");
        t.dataFields.append(d);
        QVERIFY2(s->setPivot(1, t, true, QStringLiteral("Add Field to Pivot Table")).ok, "layout");
        finish(e);
        QCOMPARE(pivotShown(e, QStringLiteral("J10:K13")), QStringLiteral("Row Labels|Sum of Amount\nEast|35\nWest|14\nGrand Total|49"));
        QVERIFY(saveNow(e));
        const Parts after = readParts(e.path);
        checkShapes(after);
        checkPackage(after);
        QVERIFY(!after.contains(QStringLiteral("xl/calcChain.xml")));
        QVERIFY(after.contains(QStringLiteral("xl/pivotTables/pivotTable2.xml")));
        QVERIFY(after.contains(QStringLiteral("xl/pivotCache/pivotCacheDefinition2.xml")));
        QVERIFY(after.contains(QStringLiteral("xl/pivotCache/pivotCacheRecords2.xml")));
        QVERIFY(after.value(QStringLiteral("[Content_Types].xml")).contains("pivotTable2.xml"));
        QVERIFY(after.value(QStringLiteral("xl/workbook.xml")).contains("<pivotCache cacheId=\"2\""));
        QVERIFY(after.value(QStringLiteral("xl/_rels/workbook.xml.rels")).contains("pivotCacheDefinition2.xml"));
        QVERIFY(after.value(QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")).contains("pivotTable2.xml"));
        // the file as saved opens with both tables, and the new one works out as it was
        EditWindow again;
        QFile saved(e.path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        QVERIFY(again.open(saved.readAll(), QStringLiteral("again.xlsx")));
        QCOMPARE(again.win.w.session()->workbook()->pivots().tableCount(), 2);
        QVERIFY(again.win.w.session()->refreshPivot(1).ok);
        again.settle();
        QCOMPARE(pivotShown(again, QStringLiteral("J10:K13")), QStringLiteral("Row Labels|Sum of Amount\nEast|35\nWest|14\nGrand Total|49"));
        // Undo takes the layout away, then the table
        s->undoStack()->undo();
        s->undoStack()->undo();
        QCOMPARE(s->workbook()->pivots().tableCount(), 1);
        QCOMPARE(s->pivotAt(0, 9, 9), -1);
        s->undoStack()->redo();
        QCOMPARE(s->workbook()->pivots().tableCount(), 2);
    }

    // A change that leaves no formula to work out (a table made right after cells were typed) still ends with the calculation over, not waiting for the next edit.
    void thePivotTableCommandEndsTheCalculation()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        e.win.w.setPivotCreateAsker([&](QString *source, bool *newSheet, QString *) {
            *source = QStringLiteral("Sheet1!A1:C7");
            *newSheet = true;
            return true;
        });
        e.select(QStringLiteral("F1"));
        e.enter(QStringLiteral("4"));
        e.enter(QStringLiteral("5"));
        e.enter(QStringLiteral("=F1+F2"));
        e.select(QStringLiteral("A3"));
        e.win.w.act(QStringLiteral("insert.pivot"))->trigger();
        QCOMPARE(e.win.w.session()->workbook()->pivots().tableCount(), 2);
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.w.isCalculating(), 5000);
    }

    // A step that changes a pivot table sends the worker to work the workbook out, so the window is calculating until the worker says it is done: it is not ready (and a command run at that
    // moment does not see the worker's last pass land in the middle of it, such as the Circular References command turning dim).
    void aPivotTableChangeKeepsTheWindowCalculatingUntilTheWorkerIsDone()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        PivotTable t = *s->workbook()->pivots().table(0).table;
        std::swap(t.rowFields, t.colFields);
        QVERIFY(s->setPivot(0, t, true, QStringLiteral("Move fields")).ok);
        QVERIFY(e.win.w.isCalculating());
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.w.isCalculating(), 5000);
        // (done: nothing comes after)
        const bool circular = e.win.w.act(QStringLiteral("formulas.circular"))->isEnabled();
        QTest::qWait(200);
        QCOMPARE(e.win.w.act(QStringLiteral("formulas.circular"))->isEnabled(), circular);
        QVERIFY(!e.win.w.isCalculating());
    }

    // Insert > Pivot Table: asks for the data and the place, makes the table on a sheet of its own, and shows its fields.
    void thePivotTableCommandMakesATableOnANewSheet()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        QString asked;
        e.win.w.setPivotCreateAsker([&](QString *source, bool *newSheet, QString *) {
            asked = *source;
            *source = QStringLiteral("Sheet1!A1:C7");
            *newSheet = true;
            return true;
        });
        e.select(QStringLiteral("A3"));
        e.win.w.act(QStringLiteral("insert.pivot"))->trigger();
        finish(e);
        QCOMPARE(asked, QStringLiteral("Sheet1!A1:C7"));   // (the block of data around the cursor)
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->workbook()->pivots().tableCount(), 2);
        const PivotTableEntry &made = s->workbook()->pivots().table(1);
        QCOMPARE(s->workbook()->info().sheets[made.sheet].name, QStringLiteral("Pivot Table 1"));
        QCOMPARE(int(s->workbook()->pivots().locationOf(1).row1), 2);
        QVERIFY(e.win.w.pivotPane());   // (the pane of fields is up)
        // Undo takes the table away, and then the sheet
        s->undoStack()->undo();
        QCOMPARE(s->workbook()->pivots().tableCount(), 1);
        s->undoStack()->undo();
        QCOMPARE(int(s->workbook()->info().order.size()), 1);
        // and Redo brings them back; saved then, the sheet of its own, the table, and its cache are in the file, and the file opens with both tables
        s->undoStack()->redo();
        s->undoStack()->redo();
        QCOMPARE(s->workbook()->pivots().tableCount(), 2);
        // saved: the sheet of its own, the table, and its cache are in the file, and the file opens with both tables
        QVERIFY(saveNow(e));
        const Parts after = readParts(e.path);
        checkShapes(after);
        checkPackage(after);
        QFile saved(e.path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(again.open(saved.readAll(), QStringLiteral("again.xlsx")));
        QCOMPARE(again.win.w.session()->workbook()->pivots().tableCount(), 2);
    }

    // A click on a value of a pivot table while a formula is typed writes GETPIVOTDATA with the names of its lines; the option turns that off.
    void clickingAPivotValueWhileTypingWritesGetPivotData()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        QCOMPARE(e.win.w.session()->pivotDataFormula(0, 4, 5, 0), QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",$E$3,\"Region\",\"East\",\"Month\",\"Jan\")"));
        QCOMPARE(e.win.w.session()->pivotDataFormula(0, 4, 4, 0), QString());   // (a label)
        auto typeAndClick = [&](const char *expect) {
            e.select(QStringLiteral("J20"));
            QTest::keyClick(e.view(), '=');
            QVERIFY(e.win.w.cellEditor()->isVisible());
            QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->cellRect(4, 5).center());   // F5: East, Jan
            QVERIFY2(e.win.w.cellEditor()->text() == QLatin1String(expect) || e.win.w.formulaBar()->text() == QLatin1String(expect), qPrintable(e.win.w.cellEditor()->text() + QStringLiteral(" / ") + e.win.w.formulaBar()->text()));
            QTest::keyClick(e.win.w.cellEditor(), Qt::Key_Escape);
        };
        Settings::get().setValue(QStringLiteral("crunch/generateGetPivotData"), true);
        typeAndClick("=GETPIVOTDATA(\"Sum of Amount\",$E$3,\"Region\",\"East\",\"Month\",\"Jan\")");
        e.win.w.setPivotOptionsAsker([](bool *generate) {
            *generate = false;
            return true;
        });
        e.win.w.act(QStringLiteral("pivot.options"))->trigger();   // (turned off)
        QVERIFY(!Settings::get().value(QStringLiteral("crunch/generateGetPivotData"), true).toBool());
        typeAndClick("=F5");   // (turned off: the click is a plain click, which points at the cell, as a click on any other cell)
        Settings::get().setValue(QStringLiteral("crunch/generateGetPivotData"), true);
    }

    // A GETPIVOTDATA typed before the table exists is worked out again when the table is made, and when the making is undone.
    void getPivotDataFollowsATableThatComesAndGoes()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 0, 11, QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",$J$10,\"Region\",\"East\")")).ok);   // L1
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("L1")), QStringLiteral("e:#REF!"));
        QVERIFY(s->createPivot(QStringLiteral("Sheet1!A1:C7"), 0, 9, 9).ok);
        PivotTable t = *s->workbook()->pivots().table(1).table;
        t.rowFields = {0};
        PivotDataField d;
        d.fld = 2;
        d.name = QStringLiteral("Sum of Amount");
        t.dataFields.append(d);
        QVERIFY(s->setPivot(1, t, true, QStringLiteral("Add Field to Pivot Table")).ok);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("L1")), QStringLiteral("n:35"));
        s->undoStack()->undo();   // (the layout)
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("L1")), QStringLiteral("e:#REF!"));
        s->undoStack()->undo();   // (the table)
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("L1")), QStringLiteral("e:#REF!"));
        s->undoStack()->redo();
        s->undoStack()->redo();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("L1")), QStringLiteral("n:35"));
    }

    // A refusal leaves nothing behind (no sheet of its own made first); the place may not be over the data; whole columns and rows are the cells they hold; a formula in the empty table's cells is no circle.
    void aRefusedTableLeavesNoSheetAndTheDataIsNotCovered()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        const int sheets = s->workbook()->info().sheets.size();   // (a sheet that was made and taken away stays in the list, marked deleted: the list grows if a sheet was made)
        QVERIFY(!s->createPivot(QStringLiteral("Sheet1!A1:C1"), -1, 0, 0).ok);
        QVERIFY(!s->createPivot(QStringLiteral("Nowhere!A1:C7"), -1, 0, 0).ok);
        QVERIFY(!s->createPivot(QStringLiteral("Sheet1!A1:C7"), 0, 0, 2).ok);   // over the data (C1:E18 meets A1:C7)
        QCOMPARE(int(s->workbook()->info().sheets.size()), sheets);
        QCOMPARE(s->undoStack()->count(), 0);
        // whole columns, and whole rows
        int made = -1;
        QVERIFY(s->createPivot(QStringLiteral("A:C"), 0, 20, 0, &made).ok);
        const PivotCacheEntry &one = s->workbook()->pivots().cache(s->workbook()->pivots().table(made).cache);
        QCOMPARE(int(one.source.row1), 0);
        QCOMPARE(int(one.source.row2), 6);   // as far as the sheet is used: row 7
        QCOMPARE(int(one.source.col2), 2);
        QVERIFY(s->createPivot(QStringLiteral("1:7"), 0, 20, 5, &made).ok);
        QCOMPARE(int(s->workbook()->pivots().cache(s->workbook()->pivots().table(made).cache).source.row2), 6);
        // a formula in the cells the empty table covers: #REF!, and no circle
        QVERIFY(s->type(0, 22, 0, QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",$A$21)")).ok);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("A23")), QStringLiteral("e:#REF!"));
        QCOMPARE(s->calculation()->circularCells(nullptr, 10), qint64(0));
    }

    // A table on a sheet that is deleted before the save is not written at all; a cache whose data sheet is deleted keeps naming it.
    void aTableOnADeletedSheetIsNotWritten()
    {
        PivotBookOptions o;
        o.otherSheet = true;   // (the data on sheet 1, the pivot table on sheet 2)
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        int made = -1;
        QVERIFY(s->createPivot(QStringLiteral("Sheet1!A1:C7"), -1, 0, 0, &made).ok);   // on a sheet of its own (the third)
        const int third = s->workbook()->pivots().table(made).sheet;
        PivotTable t = *s->workbook()->pivots().table(made).table;
        t.rowFields = {0};
        PivotDataField d;
        d.fld = 2;
        d.name = QStringLiteral("Sum of Amount");
        t.dataFields.append(d);
        QVERIFY(s->setPivot(made, t, true, QStringLiteral("Add Field to Pivot Table")).ok);
        // a second table on the same new sheet is not made here; this one is kept for the data-sheet case below
        QVERIFY(s->deleteSheet(third).ok);
        finish(e);
        QVERIFY(saveNow(e));
        Parts after = readParts(e.path);
        checkPackage(after);
        QVERIFY(!after.contains(QStringLiteral("xl/pivotTables/pivotTable2.xml")));
        QVERIFY(!after.contains(QStringLiteral("xl/pivotCache/pivotCacheDefinition2.xml")));
        QVERIFY(!QString::fromUtf8(after.value(QStringLiteral("[Content_Types].xml"))).contains(QLatin1String("pivotTable2.xml")));
        QVERIFY(!QString::fromUtf8(after.value(QStringLiteral("xl/workbook.xml"))).contains(QLatin1String("<pivotCache cacheId=\"2\"")));
        // the data sheet deleted after a table was made on a sheet that stays: the definition still names the sheet
        EditWindow f;
        QVERIFY(f.open(pivotWorkbook(o)));
        crunch::EditSession *s2 = f.win.w.session();
        QVERIFY(s2->createPivot(QStringLiteral("Sheet1!A1:C7"), 1, 9, 9, &made).ok);
        QVERIFY(s2->deleteSheet(0).ok);
        f.settle();
        QVERIFY(saveNow(f));
        after = readParts(f.path);
        checkPackage(after);
        QVERIFY2(QString::fromUtf8(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition2.xml"))).contains(QLatin1String("sheet=\"Sheet1\"")), "the definition names the sheet that is gone");
    }

    // A formula written for a locale with a semicolon between arguments; the dates and numbers of the items.
    void theFormulaOfAClickIsWrittenInThePersonsLocale()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->pivotDataFormula(0, 4, 5, 0), QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\",$E$3,\"Region\",\"East\",\"Month\",\"Jan\")"));
        s->setLocale(QLocale(QLocale::German, QLocale::Germany));
        QCOMPARE(s->pivotDataFormula(0, 4, 5, 0), QStringLiteral("=GETPIVOTDATA(\"Sum of Amount\";$E$3;\"Region\";\"East\";\"Month\";\"Jan\")"));
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // A created pivot table saved as an .xls copy keeps what it shows, as values.
    void aCreatedPivotTableInAnXlsCopyKeepsItsValues()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        int made = -1;
        QVERIFY(s->createPivot(QStringLiteral("Sheet1!A1:C7"), 0, 9, 9, &made).ok);
        PivotTable t = *s->workbook()->pivots().table(made).table;
        t.rowFields = {0};
        PivotDataField d;
        d.fld = 2;
        d.name = QStringLiteral("Sum of Amount");
        t.dataFields.append(d);
        QVERIFY(s->setPivot(made, t, true, QStringLiteral("Add Field to Pivot Table")).ok);
        finish(e);
        e.win.w.setLossAsker([](const QString &, const QString &) { return CrunchWindow::LossAnswer::Continue; });
        QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
        const QString target = QFileInfo(e.path).dir().filePath(QStringLiteral("copy.xls"));
        QVERIFY(e.win.w.saveAs(target));
        QVERIFY(copied.count() > 0 || copied.wait(30000));
        QVERIFY(copied.first().at(1).toBool());
        jp::office::sheet::Workbook book;
        QString why;
        QVERIFY2(book.openFile(target, &why), qPrintable(why));
        jp::office::sheet::Sheet sheet;
        book.readSheet(0, &sheet);
        const Cell *grand = sheet.cells.find(12, 10);   // K13: the grand total
        QVERIFY(grand);
        QCOMPARE(grand->number, 49.0);
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
        // (the date is the moment of the save, in this workbook's count of days: between the moment before and the moment after, to the second)
        const double before = double(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()) / 86400.0 + 24107.0;
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        QVERIFY(saveNow(e));
        const double after = double(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()) / 86400.0 + 24107.0;
        const QString def = QString::fromUtf8(readParts(e.path).value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")));
        const QRegularExpression re(QStringLiteral("refreshedDate=\"([0-9.]+)\""));
        const auto m = re.match(def);
        QVERIFY2(m.hasMatch(), qPrintable(def));
        const double written = m.captured(1).toDouble();
        QVERIFY2(written >= before - 1e-9 && written <= after + 1e-9, qPrintable(m.captured(1)));
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

    // A refresh made after rows were put in above the data reads the cells where the data is now (the cache was not read before the edit, and its range is the file's, moved).
    void aRefreshAfterRowsWerePutInReadsTheMovedSource()
    {
        PivotBookOptions o;
        o.otherSheet = true;
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 0, 2).ok);   // two rows above the whole list
        QVERIFY(e.settle());
        QCOMPARE(s->pivotSourceText(0), QStringLiteral("Sheet1!A3:C9"));
        QVERIFY(s->refreshPivot(0).ok);
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5"), 1), QStringLiteral("n:15"));
        QCOMPARE(valueOf(e, QStringLiteral("H7"), 1), QStringLiteral("n:49"));
    }

    // A subtotal's label shows a number as the cell that has it shows it: two numbers of one column, one a date and one not, are each as their own cell has it.
    void aSubtotalLabelTakesTheFormatOfItsOwnNumber()
    {
        PivotBookOptions o;
        o.styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/>"));
        o.tweak = [](Parts &p) {
            p[kSheetPart] = worksheet(row(1, text(QStringLiteral("A1"), QStringLiteral("N")) + text(QStringLiteral("B1"), QStringLiteral("Region")) + text(QStringLiteral("C1"), QStringLiteral("Amount")))
                                      + row(2, QStringLiteral("<c r=\"A2\" s=\"1\"><v>45000</v></c>") + text(QStringLiteral("B2"), QStringLiteral("East")) + num(QStringLiteral("C2"), 10))
                                      + row(3, num(QStringLiteral("A3"), 5) + text(QStringLiteral("B3"), QStringLiteral("East")) + num(QStringLiteral("C3"), 3))
                                      + row(5, QStringLiteral("<c r=\"E5\" s=\"1\"><v>45000</v></c>"))
                                      + row(7, QStringLiteral("<c r=\"E7\"><v>5</v></c>")));
            p[QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheDefinition xmlns=\"%1\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId1\" "
                                                                           "recordCount=\"2\"><cacheSource type=\"worksheet\"><worksheetSource ref=\"A1:C3\" sheet=\"Sheet1\"/></cacheSource><cacheFields count=\"3\">"
                                                                           "<cacheField name=\"N\" numFmtId=\"0\"><sharedItems containsSemiMixedTypes=\"0\" containsNonDate=\"1\" containsDate=\"1\" containsString=\"0\" containsNumber=\"1\" containsInteger=\"1\" minValue=\"5\" maxValue=\"5\" minDate=\"2023-03-15T00:00:00\" maxDate=\"2023-03-15T00:00:00\" count=\"2\"><d v=\"2023-03-15T00:00:00\"/><n v=\"5\"/></sharedItems></cacheField>"
                                                                           "<cacheField name=\"Region\" numFmtId=\"0\"><sharedItems count=\"1\"><s v=\"East\"/></sharedItems></cacheField>"
                                                                           "<cacheField name=\"Amount\" numFmtId=\"0\"><sharedItems containsSemiMixedTypes=\"0\" containsString=\"0\" containsNumber=\"1\" containsInteger=\"1\" minValue=\"3\" maxValue=\"10\"/></cacheField>"
                                                                           "</cacheFields></pivotCacheDefinition>").arg(QLatin1String(kNs)).toUtf8();
            p[QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")] = QStringLiteral("<pivotCacheRecords xmlns=\"%1\" count=\"2\"><r><x v=\"0\"/><x v=\"0\"/><n v=\"10\"/></r><r><x v=\"1\"/><x v=\"0\"/><n v=\"3\"/></r></pivotCacheRecords>").arg(QLatin1String(kNs)).toUtf8();
            p[QStringLiteral("xl/pivotTables/pivotTable1.xml")] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotTableDefinition xmlns=\"%1\" name=\"PivotTable1\" cacheId=\"1\" dataCaption=\"Values\" compact=\"0\" compactData=\"0\" gridDropZones=\"1\">"
                                                                      "<location ref=\"E3:G9\" firstHeaderRow=\"1\" firstDataRow=\"2\" firstDataCol=\"2\"/><pivotFields count=\"3\">"
                                                                      "<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" subtotalTop=\"0\" showAll=\"0\"><items count=\"3\"><item x=\"0\"/><item x=\"1\"/><item t=\"default\"/></items></pivotField>"
                                                                      "<pivotField axis=\"axisRow\" compact=\"0\" outline=\"0\" showAll=\"0\" defaultSubtotal=\"0\"><items count=\"1\"><item x=\"0\"/></items></pivotField>"
                                                                      "<pivotField dataField=\"1\" compact=\"0\" outline=\"0\" showAll=\"0\"/></pivotFields><rowFields count=\"2\"><field x=\"0\"/><field x=\"1\"/></rowFields>"
                                                                      "<dataFields count=\"1\"><dataField name=\"Sum of Amount\" fld=\"2\" baseField=\"0\" baseItem=\"0\"/></dataFields></pivotTableDefinition>").arg(QLatin1String(kNs)).toUtf8();
        };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY2(s->refreshPivot(0).ok, "refresh");
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("E5")), QStringLiteral("n:45000"));
        QCOMPARE(valueOf(e, QStringLiteral("E6")), QStringLiteral("s:3/15/2023 Total"));
        QCOMPARE(valueOf(e, QStringLiteral("E8")), QStringLiteral("s:5 Total"));
    }

    // Growth over a validation, a conditional format, a hyperlink, or a note is refused as growth over content is, and nothing is changed.
    void growthOverRulesLinksAndNotesIsRefused()
    {
        struct Case {
            const char *name;
            QByteArray sheetTail;
            std::function<void(crunch::EditSession *)> setup;
        };
        const QVector<Case> cases = {
            {"validation", "<dataValidations count=\"1\"><dataValidation type=\"whole\" sqref=\"I4:I5\"><formula1>1</formula1></dataValidation></dataValidations>", nullptr},
            {"conditional format", "<conditionalFormatting sqref=\"I4:I5\"><cfRule type=\"expression\" priority=\"1\"><formula>TRUE</formula></cfRule></conditionalFormatting>", nullptr},
            {"link", QByteArray(), [](crunch::EditSession *s) {
                 jp::office::sheet::LinkSpec link;
                 link.target = QStringLiteral("https://example.org/");
                 CellRange r;
                 parseRangeRef(QStringLiteral("I4"), &r);
                 link.range = r;
                 QVERIFY(s->setLink(0, r, link, QStringLiteral("Link")).ok);
             }},
            {"note", QByteArray(), [](crunch::EditSession *s) { QVERIFY(s->setNote(0, 3, 8, QStringLiteral("hi"), QStringLiteral("me"), false, QStringLiteral("Note")).ok); }}};
        for (const Case &c : cases) {
            PivotBookOptions o;
            const QByteArray tail = c.sheetTail;
            o.tweak = [tail](Parts &p) { p[kSheetPart].replace("</sheetData>", "</sheetData>" + tail); };
            EditWindow e;
            QVERIFY2(e.open(pivotWorkbook(o)), c.name);
            crunch::EditSession *s = e.win.w.session();
            QVERIFY(s->type(0, 7, 0, QStringLiteral("North")).ok);
            QVERIFY(s->type(0, 7, 1, QStringLiteral("Mar")).ok);
            QVERIFY(s->type(0, 7, 2, QStringLiteral("9")).ok);
            if (c.setup) c.setup(s);
            QVERIFY(e.settle());
            const int steps = s->undoStack()->count();
            const auto out = s->changePivotSource(0, QStringLiteral("Sheet1!A1:C8"));
            QVERIFY2(!out.ok, c.name);
            QVERIFY2(out.problem.contains(QLatin1String("something in them")), c.name);
            QCOMPARE(s->undoStack()->count(), steps);
        }
    }

    // A part past the limits for the parts of a pivot table is not read: the table or the cache is left as it is, and a table that is too big still shuts the cells it has.
    void partsPastTheLimitsAreLeftAlone()
    {
        const QByteArray book = pivotWorkbook();
        struct Limits {
            qint64 table, definition, records;
        };
        for (const Limits &l : {Limits{500, 0, 0}, Limits{0, 500, 0}, Limits{0, 0, 100}}) {
            jp::office::sheet::pivotSetPartLimitsForTest(l.table, l.definition, l.records);
            EditWindow e;
            const bool opened = e.open(book);
            const bool usable = opened && e.win.w.session()->pivotUsable(0);
            const int found = opened ? e.win.w.session()->workbook()->pivotTablesOn(0).size() : -1;
            jp::office::sheet::pivotSetPartLimitsForTest(0, 0, 0);
            QVERIFY(opened);
            QVERIFY2(!usable, qPrintable(QString::number(l.table) + QLatin1Char(' ') + QString::number(l.definition)));
            QCOMPARE(found, 1);
        }
        // (a table too big to read is the whole sheet's as far as edits go: it is not known where it is)
        jp::office::sheet::pivotSetPartLimitsForTest(500, 0, 0);
        EditWindow e;
        QVERIFY(e.open(book));
        const QVector<CellRange> ranges = e.win.w.session()->workbook()->pivotTablesOn(0);
        jp::office::sheet::pivotSetPartLimitsForTest(0, 0, 0);
        QVERIFY(ranges.size() == 1 && ranges[0].row2 >= 1048575);
    }

    // The sheet a cache is made from is renamed: the cache is saved naming the new name (only that of the part changes), and a refresh of the file saved reads the cells.
    void aRenamedSourceSheetIsSavedAndRefreshed()
    {
        PivotBookOptions o;
        o.otherSheet = true;
        const QByteArray book = pivotWorkbook(o);
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->pivotUsable(0));
        QVERIFY2(s->renameSheet(0, QStringLiteral("Data")).ok, "rename");
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        Parts before;
        readZip(book, before);
        const Parts after = readParts(e.path);
        QByteArray expected = before.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"));
        expected.replace("sheet=\"Sheet1\"", "sheet=\"Data\"");
        QCOMPARE(after.value(QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")), expected);
        QCOMPARE(after.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")), before.value(QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(again.open(f.readAll(), QStringLiteral("again.xlsx")));
        QCOMPARE(again.win.w.session()->pivotSourceText(0), QStringLiteral("Data!A1:C7"));
        QVERIFY(again.win.w.session()->type(1, 20, 0, QStringLiteral("x")).ok);   // (an edit elsewhere on the pivot sheet)
        QVERIFY(again.win.w.session()->refreshPivot(0).ok);
        finish(again);
        QCOMPARE(valueOf(again, QStringLiteral("F5"), 1), QStringLiteral("n:15"));
    }

    // A table part nested a hundred thousand deep is not given to a recursive reader either: the table is left as it is, and still shuts its cells.
    void aHostilelyDeepTableIsLeftAloneAndKeepsItsLocation()
    {
        const QByteArray book = [] {
            PivotBookOptions o;
            o.tweak = [](Parts &p) {
                QByteArray deep = "<extLst>";
                for (int i = 0; i < 100000; ++i) deep += "<a>";
                for (int i = 0; i < 100000; ++i) deep += "</a>";
                deep += "</extLst>";
                p[QStringLiteral("xl/pivotTables/pivotTable1.xml")].replace("</pivotTableDefinition>", deep + "</pivotTableDefinition>");
            };
            return pivotWorkbook(o);
        }();
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(!s->pivotUsable(0));
        QVERIFY(!s->refreshPivot(0).ok);
        const QVector<CellRange> ranges = s->workbook()->pivotTablesOn(0);
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(int(ranges[0].row1), 2);
        QCOMPARE(int(ranges[0].col2), 7);
        QVERIFY(s->type(0, 0, 8, QStringLiteral("1")).ok);
        QVERIFY(saveNow(e));
        Parts before;
        readZip(book, before);
        const Parts after = readParts(e.path);
        for (const QString &part : {QStringLiteral("xl/pivotTables/pivotTable1.xml"), QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"), QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")})
            QVERIFY2(after.value(part) == before.value(part), qPrintable(part));
    }

    // A table at the top of its sheet, with no filters, can be changed; filters that would need lines above the first are what is refused.
    void aTableAtTheTopOfASheetWithNoFiltersCanBeChanged()
    {
        PivotBookOptions o;
        o.tweak = [](Parts &p) { p[QStringLiteral("xl/pivotTables/pivotTable1.xml")].replace("ref=\"E3:H7\"", "ref=\"E1:H5\""); };
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook(o)));
        crunch::EditSession *s = e.win.w.session();
        const auto refreshed = s->refreshPivot(0);
        QVERIFY2(refreshed.ok, qPrintable(refreshed.problem));
        PivotTable t = *s->workbook()->pivots().table(0).table;
        PivotPageField page;
        page.fld = 1;
        t.colFields.removeAll(1);
        t.pageFields.append(page);
        const auto o2 = s->setPivot(0, t, true, QStringLiteral("Filter"));
        QVERIFY(!o2.ok);
        QVERIFY2(o2.problem.contains(QLatin1String("no room above")), qPrintable(o2.problem));
    }

    // Clear is never refused for the cells it covers: what is left claims no more than the table had, and what is beside the table is not in its way.
    void clearIsNotRefusedForCellsBesideTheTable()
    {
        EditWindow e;
        QVERIFY(e.open(pivotWorkbook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, rowOf(QStringLiteral("F12")), colOf(QStringLiteral("F12")), QStringLiteral("mine")).ok);   // (under where an empty area of 18 lines would reach)
        QVERIFY(e.settle());
        const auto o = s->clearPivot(0, false);
        QVERIFY2(o.ok, qPrintable(o.problem));
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("-"));
        QCOMPARE(valueOf(e, QStringLiteral("F12")), QStringLiteral("s:mine"));
        const CellRange loc = s->workbook()->pivots().locationOf(0);
        QVERIFY(loc.row2 <= 6 && loc.col2 <= 7);
        s->undoStack()->undo();
        finish(e);
        QCOMPARE(valueOf(e, QStringLiteral("F5")), QStringLiteral("n:15"));
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
        // and in the saved part: one value, and no place for the values among the fields
        QVERIFY(saveNow(e));
        const QString xml = QString::fromUtf8(readParts(e.path).value(QStringLiteral("xl/pivotTables/pivotTable1.xml")));
        QVERIFY2(!xml.contains(QLatin1String("x=\"-2\"")), qPrintable(xml));
        QVERIFY(xml.contains(QLatin1String("<colFields count=\"1\">")));
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
