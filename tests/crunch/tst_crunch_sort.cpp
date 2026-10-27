// Tests of Sort on the edit session: the order of the kinds of value, several levels, headers, case, left to right, lists, colors, formulas that go with their rows, merged cells, the cells
// that must not be moved, one step for Undo, and the work being counted and stoppable. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/sortdialog.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "io/zipindex.h"
#include "savedok.h"

#include <QAction>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFile>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalSpy>
#include <QTableWidget>
#include <QUndoStack>
#include "../localepin.h"

#include <memory>

using namespace jp;
using namespace crunchtest;
using jp::crunch::SortLevel;
using jp::crunch::SortSpec;

namespace {

SortSpec byColumn(int key, bool descending = false, bool header = false)
{
    SortSpec spec;
    SortLevel level;
    level.key = key;
    level.descending = descending;
    spec.levels.append(level);
    spec.hasHeader = header;
    return spec;
}

crunch::EditSession::Outcome sortWith(EditWindow &e, const QString &range, const SortSpec &spec)
{
    return e.win.w.session()->sortRange(0, rangeOf(range), spec);
}

} // namespace

class CrunchSortTests : public QObject {
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
        m_localeBefore = QLocale();
        m_pin = std::make_unique<PinnedLocale>();   // (put back when the class is done)
    }

    void cleanupTestCase()
    {
        // the computer's language is back, whatever this class pinned (a class that is run after this one in the same process is not told it was English)
        m_pin.reset();
        QCOMPARE(QLocale().name(), m_localeBefore.name());
    }

    void numbersTextLogicalsAndErrorsGoInThatOrderAndNothingGoesLast()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=3", "A2=banana", "A4==1/0", "A5==TRUE", "A6=1", "A7=Apple", "A8=2"})));
        QVERIFY(e.settle());
        QVERIFY2(sortWith(e, QStringLiteral("A1:A8"), byColumn(0)).ok, "ascending");
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A8")), QStringLiteral("1|2|3|Apple|banana|TRUE|#DIV/0!|"));
        QVERIFY2(sortWith(e, QStringLiteral("A1:A8"), byColumn(0, true)).ok, "descending");
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A8")), QStringLiteral("#DIV/0!|TRUE|banana|Apple|3|2|1|"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);                                 // one step each
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A8")), QStringLiteral("1|2|3|Apple|banana|TRUE|#DIV/0!|"));
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A8")), QStringLiteral("#DIV/0!|TRUE|banana|Apple|3|2|1|"));
    }

    void severalLevelsAHeaderAndEqualKeysThatKeepTheirOrder()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Group", "B1=Name", "C1=Score", "A2=b", "B2=Zed", "C2=5", "A3=a", "B3=Amy", "C3=9", "A4=b", "B4=Bob", "C4=7", "A5=a", "B5=Cy", "C5=9", "A6=b", "B6=Dee", "C6=5"})));
        SortSpec spec = byColumn(0, false, true);
        SortLevel second;
        second.key = 2;
        second.descending = true;
        spec.levels.append(second);
        QVERIFY2(sortWith(e, QStringLiteral("A1:C6"), spec).ok, "two levels");
        // group a first, the higher score first, equal ones as they were (Amy before Cy; Zed before Dee)
        QCOMPARE(shown(e, QStringLiteral("A1:C6")), QStringLiteral("Group|Name|Score|a|Amy|9|a|Cy|9|b|Bob|7|b|Zed|5|b|Dee|5"));
        // a header that is not told is sorted like the rest
        QVERIFY(sortWith(e, QStringLiteral("A1:C6"), byColumn(0)).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A6")), QStringLiteral("a|a|b|b|b|Group"));
    }

    void upperAndLowerCaseAreTheSameUnlessAskedForAndLowerComesFirst()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=b", "A2=B", "A3=a", "A4=A", "A5=b"})));
        SortSpec spec = byColumn(0);
        QVERIFY(sortWith(e, QStringLiteral("A1:A5"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("a|A|b|B|b"));       // (equal keys keep their order)
        spec.caseSensitive = true;
        QVERIFY(sortWith(e, QStringLiteral("A1:A5"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("a|A|b|b|B"));
    }

    void formulasAreWrittenForTheRowTheyGoTo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=30", "B1==A1*2", "C1==A1+A$1", "A2=10", "B2==A2*2", "C2==A2+A$1", "A3=20", "B3==A3*2", "C3==A3+A$1"})));
        QVERIFY(e.settle());
        QVERIFY(sortWith(e, QStringLiteral("A1:C3"), byColumn(0)).ok);
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("10|20|30"));
        // a reference to the same row stays in the row; an absolute reference stays where it is
        QCOMPARE(s->editText(0, 0, 1), QStringLiteral("=A1*2"));
        QCOMPARE(s->editText(0, 2, 1), QStringLiteral("=A3*2"));
        QCOMPARE(s->editText(0, 0, 2), QStringLiteral("=A1+A$1"));
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("20|40|60"));
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("20|30|40"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(s->editText(0, 0, 1), QStringLiteral("=A1*2"));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("30|10|20"));
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("60|20|40"));
    }

    void columnsCanBePutInOrderFromLeftToRight()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Item", "B1=c", "C1=a", "D1=b", "A2=Price", "B2=3", "C2=1", "D2=2", "B3==B2*10"})));
        SortSpec spec = byColumn(0);
        spec.leftToRight = true;
        spec.hasHeader = true;
        QVERIFY2(sortWith(e, QStringLiteral("A1:D3"), spec).ok, "left to right");
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:D2")), QStringLiteral("Item|a|b|c|Price|1|2|3"));
        QCOMPARE(e.win.w.session()->editText(0, 2, 3), QStringLiteral("=D2*10"));          // (the formula went to column D with its column)
    }

    void valuesCanGoInTheOrderOfAList()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Wednesday", "A2=Monday", "A3=Sunday", "A4=Tuesday", "A5=Funday", "A6=sunday"})));
        SortSpec spec = byColumn(0);
        spec.levels[0].list = jp::crunch::builtInSortLists().first();
        QCOMPARE(spec.levels[0].list.first(), QStringLiteral("Sunday"));
        QVERIFY(sortWith(e, QStringLiteral("A1:A6"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A6")), QStringLiteral("Sunday|sunday|Monday|Tuesday|Wednesday|Funday"));      // (a value not in the list goes after)
        spec.levels[0].descending = true;
        QVERIFY(sortWith(e, QStringLiteral("A1:A6"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A6")), QStringLiteral("Wednesday|Tuesday|Monday|Sunday|sunday|Funday"));
    }

    void cellsCanBePutInOrderByTheirColorsAndTheColorCanGoLast()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=one~3", "B1=1", "A2=two", "B2=2", "A3=three~3", "B3=3", "A4=four~4", "B4=4", "A5=five", "B5=5"}})));
        SortSpec spec;
        SortLevel level;
        level.key = 0;
        level.by = SortLevel::By::CellColor;
        level.color = jp::office::sheet::ColorSpec::fromRgb(0xFF0000);
        spec.levels.append(level);
        QVERIFY(sortWith(e, QStringLiteral("A1:B5"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:B5")), QStringLiteral("one|1|three|3|two|2|four|4|five|5"));          // (red first; the rest as they were)
        spec.levels[0].onTop = false;
        QVERIFY(sortWith(e, QStringLiteral("A1:B5"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("two|four|five|one|three"));
        // the font color: blue first
        spec.levels[0].by = SortLevel::By::FontColor;
        spec.levels[0].color = jp::office::sheet::ColorSpec::fromRgb(0x0000FF);
        spec.levels[0].onTop = true;
        QVERIFY(sortWith(e, QStringLiteral("A1:B5"), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A1")), QStringLiteral("four"));
        // the colors in a column, as the dialog lists them
        bool none = false;
        const auto fills = e.win.w.session()->colorsIn(0, rangeOf(QStringLiteral("A1:A5")), false, &none);
        QCOMPARE(int(fills.size()), 1);
        QVERIFY(none);
        QCOMPARE(fills[0].rgb, quint32(0xFF0000));
        const auto fonts = e.win.w.session()->colorsIn(0, rangeOf(QStringLiteral("A1:A5")), true, &none);
        QCOMPARE(int(fonts.size()), 1);
        QCOMPARE(fonts[0].rgb, quint32(0x0000FF));
    }

    void mergedCellsOfOneSizeGoWithTheirRowsAndOtherMergesAreRefused()
    {
        auto book = [](const QString &merges) {
            return buildXlsx({worksheet(rowsOfSpecs({"A1=c", "C1=3", "A2=a", "C2=1", "A3=b", "C3=2"}), merges)}, QByteArray(), QString(), QString(), findStyles());
        };
        {
            EditWindow e;
            QVERIFY(e.open(book(QStringLiteral("<mergeCells count=\"2\"><mergeCell ref=\"A1:B1\"/><mergeCell ref=\"A3:B3\"/></mergeCells>"))));
            QVERIFY2(sortWith(e, QStringLiteral("A1:C3"), byColumn(2)).ok, "merged cells of one size");
            QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("a|b|c"));
            // (the cell at the top left of each merged range went with its row, and the range with it)
            QCOMPARE(e.win.w.session()->calculation()->mergesOf(0), (QVector<CellRange>{rangeOf(QStringLiteral("A3:B3")), rangeOf(QStringLiteral("A2:B2"))}));
            pressUndo(&e.win.w);
            QCOMPARE(e.win.w.session()->calculation()->mergesOf(0), (QVector<CellRange>{rangeOf(QStringLiteral("A1:B1")), rangeOf(QStringLiteral("A3:B3"))}));
        }
        {
            EditWindow e;
            QVERIFY(e.open(book(QStringLiteral("<mergeCells count=\"2\"><mergeCell ref=\"A1:B1\"/><mergeCell ref=\"A3:C3\"/></mergeCells>"))));
            const crunch::EditSession::Outcome o = sortWith(e, QStringLiteral("A1:C3"), byColumn(2));
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("same size")));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
        }
        {
            EditWindow e;
            QVERIFY(e.open(book(QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:A2\"/></mergeCells>"))));
            const crunch::EditSession::Outcome o = sortWith(e, QStringLiteral("A1:C3"), byColumn(2));
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("span more than one row")));
        }
        {
            EditWindow e;
            QVERIFY(e.open(book(QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A2:B3\"/></mergeCells>"))));
            const crunch::EditSession::Outcome o = sortWith(e, QStringLiteral("A1:C2"), byColumn(2));      // (reaches past the edge)
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("past the edge")));
        }
    }

    void cellsThatMustNotMoveRefuseTheSortInWords()
    {
        {
            EditWindow e;
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><f t=\"array\" ref=\"A1:A2\">{3;1}</f><v>3</v></c></row><row r=\"2\"><c r=\"A2\"><v>1</v></c></row><row r=\"3\"><c r=\"A3\"><v>2</v></c></row>");
            QVERIFY(e.open(buildXlsx({worksheet(rows)})));
            const crunch::EditSession::Outcome o = sortWith(e, QStringLiteral("A1:A3"), byColumn(0));
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("array")));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
        }
        {
            // a protected sheet with every cell locked (the usual), and one with the cells unlocked
            const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
            const QString locked = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>1</v></c></row>");
            const QString open = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\" s=\"1\"><v>1</v></c></row>");
            EditWindow a;
            QVERIFY(a.open(buildXlsx({worksheet(locked, QStringLiteral("<sheetProtection sheet=\"1\"/>"))}, QByteArray(), QString(), QString(), styles)));
            const crunch::EditSession::Outcome o = sortWith(a, QStringLiteral("A1:A2"), byColumn(0));
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("locked")));
            // a protected sheet whose protection does not allow sorting refuses even unlocked cells, and says so
            EditWindow c;
            QVERIFY(c.open(buildXlsx({worksheet(open, QStringLiteral("<sheetProtection sheet=\"1\"/>"))}, QByteArray(), QString(), QString(), styles)));
            const crunch::EditSession::Outcome refused = sortWith(c, QStringLiteral("A1:A2"), byColumn(0));
            QVERIFY(!refused.ok);
            QVERIFY2(refused.problem.contains(QLatin1String("sorting")), qPrintable(refused.problem));
            EditWindow b;
            QVERIFY(b.open(buildXlsx({worksheet(open, QStringLiteral("<sheetProtection sheet=\"1\" sort=\"0\"/>"))}, QByteArray(), QString(), QString(), styles)));
            QVERIFY2(sortWith(b, QStringLiteral("A1:A2"), byColumn(0)).ok, "unlocked cells on a protected sheet that allows sorting");
            QCOMPARE(shown(b, QStringLiteral("A1:A2")), QStringLiteral("1|2"));
        }
        {
            // the header row of a table is not moved
            EditWindow e;
            const QByteArray table = "<?xml version=\"1.0\"?><table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"T\" displayName=\"T\" ref=\"A1:A3\">"
                                     "<tableColumns count=\"1\"><tableColumn id=\"1\" name=\"N\"/></tableColumns></table>";
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>N</t></is></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>1</v></c></row>");
            QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {QString::fromUtf8(table)})));
            const crunch::EditSession::Outcome o = sortWith(e, QStringLiteral("A1:A3"), byColumn(0));
            QVERIFY(!o.ok);
            QVERIFY(o.problem.contains(QLatin1String("names of its columns")));
            QVERIFY2(sortWith(e, QStringLiteral("A1:A3"), byColumn(0, false, true)).ok, "below the header");
            QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("N|1|2"));
        }
    }

    void aSortWithNothingToSortOrABadLevelIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=2", "A2=1", "B1=x"})));
        QVERIFY(!sortWith(e, QStringLiteral("A1:A1"), byColumn(0)).ok);                // one row
        QVERIFY(!sortWith(e, QStringLiteral("A1:A2"), byColumn(0, false, true)).ok);   // only a header and one row... (one row to sort)
        QVERIFY(!sortWith(e, QStringLiteral("A1:A2"), byColumn(1)).ok);                // a column outside
        QVERIFY(!sortWith(e, QStringLiteral("A1:A2"), SortSpec()).ok);                 // no level
        SortSpec many = byColumn(0);
        for (int i = 0; i < jp::crunch::kMaxSortLevels; ++i) many.levels.append(many.levels.first());
        QVERIFY(!sortWith(e, QStringLiteral("A1:A2"), many).ok);
        QVERIFY(!sortWith(e, QStringLiteral("A1:XFD1048576"), byColumn(0)).ok);        // more cells than an edit holds
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        // a range already in order makes no step
        QVERIFY(sortWith(e, QStringLiteral("A1:A2"), byColumn(0)).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(sortWith(e, QStringLiteral("A1:A2"), byColumn(0)).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
    }

    void theKeysAreReadASliceAtATimeAndTheOrderCanBeGivenUp()
    {
        QStringList cells;
        for (int r = 1; r <= 5000; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg((r * 7919) % 5003);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        crunch::EditSession *s = e.win.w.session();
        QString why;
        const auto plan = s->planSort(0, rangeOf(QStringLiteral("A1:A5000")), byColumn(0), &why);
        QVERIFY2(plan, qPrintable(why));
        int slices = 0;
        while (!s->prepareSort(*plan, 1000)) QVERIFY(++slices < 10);
        QCOMPARE(slices, 4);                                                    // (5,000 lines, 1,000 at a time)
        std::atomic<bool> cancel(true);
        QVERIFY(!crunch::EditSession::orderSort(*plan, &cancel));               // (given up: nothing is ordered)
        QVERIFY(!s->applySort(*plan).ok);
        QVERIFY(crunch::EditSession::orderSort(*plan));
        QVERIFY(s->applySort(*plan).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // a plan is for the cells as they were: after another edit it is not applied
        const auto stale = s->planSort(0, rangeOf(QStringLiteral("A1:A5000")), byColumn(0, true), &why);
        QVERIFY(stale);
        s->prepareSort(*stale, 0);
        QVERIFY(crunch::EditSession::orderSort(*stale));
        QVERIFY(s->type(0, 0, 3, QStringLiteral("9")).ok);
        const crunch::EditSession::Outcome o = s->applySort(*stale);
        QVERIFY(!o.ok);
        QVERIFY(o.problem.contains(QLatin1String("changed")));
    }

    // ---- what is tied to a cell goes with it
    void sortMovesTheNotesAndLinksWithTheirRowsWhenTheFileIsSaved()
    {
        const QByteArray comments = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>A</author></authors><commentList>"
                                    "<comment ref=\"A1\" authorId=\"0\"><text><t>note on c</t></text></comment><comment ref=\"A3\" authorId=\"0\"><text><t>note on b</t></text></comment></commentList></comments>";
        sheettest::ExtraPart part;
        part.name = QStringLiteral("xl/comments1.xml");
        part.data = comments;
        part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
        part.sheet = 0;
        part.relId = QStringLiteral("rIdC");
        part.relType = QStringLiteral("comments");
        part.relTarget = QStringLiteral("../comments1.xml");
        const QString links = QStringLiteral("<hyperlinks><hyperlink ref=\"A2\" location=\"Sheet1!C9\" display=\"go\"/></hyperlinks>");
        EditWindow e;
        QVERIFY(e.open(sheettest::withParts(bookOfSheets({{"A1=c", "A2=a", "A3=b"}}, {{0, links}}), {part})));
        auto saved = [&](const QString &name) {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            QMap<QString, QByteArray> parts;
            jp::readZip(f.readAll(), parts);
            return QString::fromUtf8(parts.value(name));
        };
        auto save = [&] {
            QSignalSpy done(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(done, 20000), "the save failed");
        };
        QVERIFY(sortWith(e, QStringLiteral("A1:A3"), byColumn(0)).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("a|b|c"));
        save();
        // "c" was in row 1 with its note and is in row 3; "b" was in row 3 and is in row 2; the link was on "a", which is in row 1
        const QString comment = saved(QStringLiteral("xl/comments1.xml"));
        QVERIFY2(comment.contains(QRegularExpression(QStringLiteral("ref=\"A3\"[^>]*><text><t>note on c"))), qPrintable(comment));
        QVERIFY2(comment.contains(QRegularExpression(QStringLiteral("ref=\"A2\"[^>]*><text><t>note on b"))), qPrintable(comment));
        QVERIFY2(saved(QStringLiteral("xl/worksheets/sheet1.xml")).contains(QLatin1String("<hyperlink ref=\"A1\"")), qPrintable(saved(QStringLiteral("xl/worksheets/sheet1.xml"))));
        // undone and saved again, they are where they were
        pressUndo(&e.win.w);
        save();
        const QString back = saved(QStringLiteral("xl/comments1.xml"));
        QVERIFY2(back.contains(QRegularExpression(QStringLiteral("ref=\"A1\"[^>]*><text><t>note on c"))), qPrintable(back));
        QVERIFY2(saved(QStringLiteral("xl/worksheets/sheet1.xml")).contains(QLatin1String("<hyperlink ref=\"A2\"")), "link back");
    }

    // Rows and columns inserted and deleted change where cells are, not the cells: a sort, a fill, a paste, a cut and paste, and a copy of the sheet made after them give the cells at the places
    // a person sees, and the file saved after them holds the same cells at the same places (read again in a window of its own).
    void sortFillPasteCutAndSheetCopyAfterRowsAndColumnsWereInsertedAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=5", "A2=3", "A3=9", "A4=1", "B1==A1*2", "B2==A2*2", "B3==A3*2", "B4==A4*2", "C1=a", "C2=b", "C3=c", "C4=d"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 1, 2).ok);        // two blank rows after the first
        QVERIFY(s->deleteColumns(0, 2, 1).ok);     // the text goes
        QVERIFY(s->insertColumns(0, 0, 1).ok);     // a blank column in front: values in B, formulas in C
        QVERIFY(s->deleteRows(0, 2, 1).ok);        // and one of the blank rows
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("B1:C5")), QStringLiteral("5|10|||3|6|9|18|1|2"));
        // sorted: the formulas go with their rows
        QVERIFY(sortWith(e, QStringLiteral("B3:C5"), byColumn(0)).ok);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("B3:C5")), QStringLiteral("1|2|3|6|9|18"));
        QCOMPARE(s->calculation()->content(0, 2, 2).formula, QStringLiteral("B3*2"));
        // filled down: the formula of C5 into C6 and C7, which read the blank cells beside them
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C5")), crunch::EditSession::FillDirection::Down, 2).ok);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("C5:C7")), QStringLiteral("18|0|0"));
        QCOMPARE(s->calculation()->content(0, 6, 2).formula, QStringLiteral("B7*2"));
        // text pasted over cells that were never there
        QVERIFY(s->pasteText(0, rangeOf(QStringLiteral("E2")), {{QStringLiteral("x"), QStringLiteral("y")}, {QStringLiteral("z"), QStringLiteral("w")}}, crunch::EditSession::PasteOptions()).ok);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("E2:F3")), QStringLiteral("x|y|z|w"));
        // cut and pasted lower down
        QVERIFY(s->moveCells(0, rangeOf(QStringLiteral("B3:C5")), 0, 8, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("B9:C11")), QStringLiteral("1|2|3|6|9|18"));
        QCOMPARE(shown(e, QStringLiteral("B3:C5")), QStringLiteral("|||||"));
        QCOMPARE(s->calculation()->content(0, 8, 2).formula, QStringLiteral("B9*2"));
        // the sheet copied is the same cells at the same places
        int made = -1;
        QVERIFY(s->copySheet(0, 1, QStringLiteral("Copy"), &made).ok);
        QVERIFY(e.settle());
        QVERIFY(made >= 0);
        QStringList original, copied;
        s->calculation()->sheet(0).cells.forEach([&](quint32 row, quint32 col, const jp::office::sheet::Cell &c) { original << QStringLiteral("%1,%2:%3").arg(row).arg(col).arg(c.number); });
        s->calculation()->sheet(made).cells.forEach([&](quint32 row, quint32 col, const jp::office::sheet::Cell &c) { copied << QStringLiteral("%1,%2:%3").arg(row).arg(col).arg(c.number); });
        QVERIFY(!original.isEmpty());
        QCOMPARE(copied, original);
        // saved, and read again in a window of its own: the same cells at the same places, and the formulas as they read where they are
        QSignalSpy done(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(done, 20000), "the save failed");
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        EditWindow again;
        QVERIFY(again.open(f.readAll(), QStringLiteral("again.xlsx")));
        QVERIFY(again.settle());
        for (const char *range : {"A1:G13"}) QCOMPARE(shown(again, QString::fromLatin1(range)), shown(e, QString::fromLatin1(range)));
        QCOMPARE(shown(again, QStringLiteral("B9:C11")), QStringLiteral("1|2|3|6|9|18"));
        QCOMPARE(again.win.w.session()->calculation()->content(0, 8, 2).formula, QStringLiteral("B9*2"));
        QCOMPARE(again.win.w.session()->calculation()->content(0, 6, 2).formula, QStringLiteral("B7*2"));
        QCOMPARE(again.win.w.session()->calculation()->sheet(0).cells.cellCount(), s->calculation()->sheet(0).cells.cellCount());
    }

    void aBigSortIsPutInASliceAtATimeAndIsOneStep()
    {
        QStringList cells;
        for (int r = 1; r <= 5000; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg((r * 7919) % 5003);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        CrunchWindow &w = e.win.w;
        w.setFixedSlices(500);
        QSignalSpy finished(&w, &CrunchWindow::sortFinished);
        QVERIFY(w.sortRange(rangeOf(QStringLiteral("A1:A5000")), byColumn(0)));
        QTRY_COMPARE(finished.count(), 1);
        QVERIFY(finished.last().first().toBool());
        crunch::EditSession *s = w.session();
        QVERIFY2(s->longestSlice() <= 500, qPrintable(QString::number(s->longestSlice())));      // (counted: no slice put in more than the budget)
        QVERIFY2(s->slicesPushed() >= 9 && s->slicesPushed() <= 11, qPrintable(QString::number(s->slicesPushed())));
        QCOMPARE(w.undoStack()->count(), 1);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        pressUndo(&w);
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("%1|%2").arg((1 * 7919) % 5003).arg((2 * 7919) % 5003));
        pressRedo(&w);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        QVERIFY(!w.isApplying());
    }

    void aHeaderIsGuessedFromTextOverNumbersAndColorsAreListed()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Qty", "A2=x", "B2=3", "A3=y", "B3=4", "D1=3", "D2=4", "F1=a", "F2=b"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->looksLikeHeader(0, rangeOf(QStringLiteral("A1:B3"))));
        QVERIFY(!s->looksLikeHeader(0, rangeOf(QStringLiteral("A2:B3"))));        // (a row of text over a row of text and numbers is a header; numbers on top is not)
        QVERIFY(!s->looksLikeHeader(0, rangeOf(QStringLiteral("D1:D2"))));
        QVERIFY(!s->looksLikeHeader(0, rangeOf(QStringLiteral("F1:F2"))));
        QVERIFY(!s->looksLikeHeader(0, rangeOf(QStringLiteral("A1:B1"))));
        QVERIFY(jp::crunch::builtInSortLists().size() >= 4);
    }

    // ---- the window
    void sortAToZAndZToAOnACellTakeTheBlockAroundItAndKeepAHeader()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Name", "B1=Qty", "A2=x", "B2=30", "A3=y", "B3=10", "A4=z", "B4=20", "D1=other"})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("B3"));
        w.act(QStringLiteral("sort.ascending"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A1:B4")), QStringLiteral("Name|Qty|y|10|z|20|x|30"));
        QCOMPARE(w.undoStack()->count(), 1);
        w.act(QStringLiteral("sort.descending"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A1:B4")), QStringLiteral("Name|Qty|x|30|z|20|y|10"));
        e.select(QStringLiteral("A2"));
        w.act(QStringLiteral("sort.descending"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A2:A4")), QStringLiteral("z|y|x"));
        // a cell with nothing around it has nothing to sort
        e.win.told.clear();
        e.select(QStringLiteral("H9"));
        w.act(QStringLiteral("sort.ascending"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
        QCOMPARE(w.undoStack()->count(), 3);
    }

    void dataNextToASelectedColumnIsAskedAbout()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=c", "B1=1", "A2=a", "B2=2", "A3=b", "B3=3"})));
        CrunchWindow &w = e.win.w;
        int asked = 0;
        w.setExpandAsker([&] {
            ++asked;
            return true;
        });
        e.view()->selectRange(0, 0, 2, 0);
        w.act(QStringLiteral("sort.ascending"))->trigger();
        QCOMPARE(asked, 1);
        QCOMPARE(shown(e, QStringLiteral("A1:B3")), QStringLiteral("a|2|b|3|c|1"));      // (the whole block)
        w.setExpandAsker([&] {
            ++asked;
            return false;
        });
        e.view()->selectRange(0, 0, 2, 0);
        w.act(QStringLiteral("sort.descending"))->trigger();
        QCOMPARE(asked, 2);
        QCOMPARE(shown(e, QStringLiteral("A1:B3")), QStringLiteral("c|2|b|3|a|1"));      // (only what was selected)
    }

    void customSortAsksForTheLevelsAndSortsByThem()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=g", "B1=Qty", "A2=b", "B2=1", "A3=a", "B3=2", "A4=b", "B4=3", "A5=a", "B5=4"})));
        CrunchWindow &w = e.win.w;
        CellRange seen;
        bool headerGuessed = false;
        w.setSortAsker([&](const CellRange &range, SortSpec *spec) {
            seen = range;
            headerGuessed = spec->hasHeader;
            SortLevel second;
            second.key = 1;
            second.descending = true;
            spec->levels.append(second);
            return true;
        });
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("sort.custom"))->trigger();
        QCOMPARE(rangeName(seen), QStringLiteral("A1:B5"));
        QVERIFY(headerGuessed);
        QCOMPARE(shown(e, QStringLiteral("A1:B5")), QStringLiteral("g|Qty|a|4|a|2|b|3|b|1"));
        // giving it up changes nothing
        w.setSortAsker([](const CellRange &, SortSpec *) { return false; });
        w.act(QStringLiteral("edit.sort"))->trigger();
        QCOMPARE(w.undoStack()->count(), 1);
    }

    void aLargeSortIsMadeASliceAtATimeAndEscGivesItUp()
    {
        QStringList cells;
        for (int r = 1; r <= 3000; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg((r * 7) % 3001) << QStringLiteral("B%1=row %2").arg(r).arg(r);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        CrunchWindow &w = e.win.w;
        w.setFixedSlices(300);                                                      // (every slice the same size: how many turns it takes is counted, not timed)
        QSignalSpy finished(&w, &CrunchWindow::sortFinished);
        e.select(QStringLiteral("A2"));
        w.act(QStringLiteral("sort.ascending"))->trigger();
        QVERIFY(w.isSorting());                                                     // (6,000 cells at 300 a turn: not done at once)
        QTRY_COMPARE(finished.count(), 1);
        QVERIFY(finished.last().first().toBool());
        QVERIFY(!w.isSorting());
        QCOMPARE(shown(e, QStringLiteral("A1:B2")), QStringLiteral("1|row 1715|2|row 429"));
        QCOMPARE(w.undoStack()->count(), 1);
        // Esc before the work is done gives it up and changes nothing
        w.act(QStringLiteral("sort.descending"))->trigger();
        QVERIFY(w.isSorting());
        QTest::keyClick(&w, Qt::Key_Escape);
        QTRY_COMPARE(finished.count(), 2);
        QVERIFY(!finished.last().first().toBool());
        QVERIFY(!w.isSorting());
        QCOMPARE(w.undoStack()->count(), 1);
        QVERIFY(w.statusText().contains(QLatin1String("stopped")));
        // and the cells are as the first sort left them
        QCOMPARE(shown(e, QStringLiteral("A1:B1")), QStringLiteral("1|row 1715"));
    }

    void aSortWhenTheCellsCannotBeMovedSaysWhy()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rowsOfSpecs({"A1=2", "A2=1"}), QStringLiteral("<sheetProtection sheet=\"1\"/>"))})));
        e.select(QStringLiteral("A1"));
        e.win.w.act(QStringLiteral("sort.ascending"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
        QVERIFY(e.win.told.last().contains(QLatin1String("locked")));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    void theSortDialogGivesBackWhatWasSetUpInIt()
    {
        SortDialog::Provider provider;
        provider.names = [](bool header, bool leftToRight) {
            return leftToRight ? QStringList{QStringLiteral("Row 1"), QStringLiteral("Row 2")} : header ? QStringList{QStringLiteral("Name"), QStringLiteral("Qty"), QStringLiteral("Date")}
                                                                                                       : QStringList{QStringLiteral("Column A"), QStringLiteral("Column B"), QStringLiteral("Column C")};
        };
        provider.colors = [](int key, bool font, bool) {
            SortDialog::Colors c;
            if (key == 1 && !font) c.colors.append({jp::office::sheet::ColorSpec::fromRgb(0xFF0000), QColor(0xFF0000)});
            c.none = true;
            return c;
        };
        SortSpec initial = byColumn(1, true, true);
        initial.caseSensitive = true;
        SortDialog d(provider, initial, nullptr);
        {
            // a second level by color, for a picture of the dialog
            SortLevel color;
            color.key = 2;
            color.by = SortLevel::By::CellColor;
            color.color = jp::office::sheet::ColorSpec::fromRgb(0xFF0000);
            SortSpec two = initial;
            two.levels.append(color);
            SortDialog shot(provider, two, nullptr);
            shot.show();
            saveShot(&shot, QStringLiteral("sort-dialog"));
        }
        SortSpec got = d.spec();
        QCOMPARE(got.levels.size(), 1);
        QCOMPARE(got.levels[0].key, 1);
        QVERIFY(got.levels[0].descending);
        QVERIFY(got.hasHeader && got.caseSensitive && !got.leftToRight);
        QComboBox *key0 = d.findChild<QComboBox *>(QStringLiteral("sortColumn0"));
        QCOMPARE(key0->itemText(0), QStringLiteral("Name"));
        // add a level, copy it, move it, delete one
        d.findChild<QPushButton *>(QStringLiteral("sortAddLevel"))->click();
        QCOMPARE(d.findChild<QTableWidget *>(QStringLiteral("sortLevels"))->rowCount(), 2);
        d.findChild<QComboBox *>(QStringLiteral("sortColumn1"))->setCurrentIndex(2);
        d.findChild<QComboBox *>(QStringLiteral("sortOn1"))->setCurrentIndex(1);           // cell color
        QComboBox *order1 = d.findChild<QComboBox *>(QStringLiteral("sortOrder1"));
        QCOMPARE(order1->count(), 1);                                                       // (the column has no colors, and some cells none)
        d.findChild<QComboBox *>(QStringLiteral("sortColumn1"))->setCurrentIndex(1);
        QCOMPARE(order1->count(), 2);                                                       // (red, and no color)
        order1->setCurrentIndex(0);
        d.findChild<QComboBox *>(QStringLiteral("sortWhere1"))->setCurrentIndex(1);
        got = d.spec();
        QCOMPARE(got.levels.size(), 2);
        QCOMPARE(int(got.levels[1].by), int(SortLevel::By::CellColor));
        QCOMPARE(got.levels[1].color.rgb, quint32(0xFF0000));
        QVERIFY(!got.levels[1].onTop);
        d.findChild<QTableWidget *>(QStringLiteral("sortLevels"))->selectRow(1);
        d.findChild<QPushButton *>(QStringLiteral("sortMoveUp"))->click();
        got = d.spec();
        QCOMPARE(int(got.levels[0].by), int(SortLevel::By::CellColor));
        QCOMPARE(got.levels[1].key, 1);
        d.findChild<QPushButton *>(QStringLiteral("sortCopyLevel"))->click();
        QCOMPARE(d.spec().levels.size(), 3);
        d.findChild<QPushButton *>(QStringLiteral("sortDeleteLevel"))->click();
        QCOMPARE(d.spec().levels.size(), 2);
        // a list, and left to right
        d.findChild<QComboBox *>(QStringLiteral("sortOn0"))->setCurrentIndex(0);
        QComboBox *order0 = d.findChild<QComboBox *>(QStringLiteral("sortOrder0"));
        int dayList = -1;
        for (int i = 0; i < order0->count(); ++i)
            if (order0->itemText(i).startsWith(QLatin1String("Sunday"))) dayList = i;
        QVERIFY(dayList > 1);
        order0->setCurrentIndex(dayList);
        got = d.spec();
        QCOMPARE(got.levels[0].list.first(), QStringLiteral("Sunday"));
        d.findChild<QCheckBox *>(QStringLiteral("sortLeftToRight"))->setChecked(true);
        QCOMPARE(d.findChild<QComboBox *>(QStringLiteral("sortColumn0"))->itemText(0), QStringLiteral("Row 1"));
        QVERIFY(d.spec().leftToRight);
        // there is room for eight levels and no more
        for (int i = 0; i < 12; ++i) d.findChild<QPushButton *>(QStringLiteral("sortAddLevel"))->click();
        QCOMPARE(d.spec().levels.size(), jp::crunch::kMaxSortLevels);
    }

    void theSortCommandsAreOnTheRibbonAndOffWithoutAWorkbook()
    {
        Window blank;
        for (const char *id : {"edit.sort", "sort.ascending", "sort.descending", "sort.custom"}) {
            QVERIFY2(blank.w.act(QString::fromLatin1(id)), id);
            QVERIFY2(!blank.w.act(QString::fromLatin1(id))->isEnabled(), id);
        }
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        for (const char *id : {"edit.sort", "sort.ascending", "sort.descending", "sort.custom"}) QVERIFY2(e.win.w.act(QString::fromLatin1(id))->isEnabled(), id);
        QVERIFY(e.win.w.ribbonErrors().isEmpty());
    }
private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchSortTests)
#include "tst_crunch_sort.moc"
