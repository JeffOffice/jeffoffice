// Tests of the sparklines in JeffCrunch: made, changed, and taken away as steps of Undo, saved into the file's extension list, and the same when the file is opened again.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "app/ribbon.h"

#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save()) return false;
    return (!saved.isEmpty() || saved.wait(20000)) && saved.last().value(1).toBool();
}

SparklineGroup groupOf(SparklineGroup::Type type, quint32 row, quint32 col, const QString &source)
{
    SparklineGroup g;
    g.type = type;
    g.series = ColorSpec::fromRgb(0x376092);
    Sparkline s;
    s.row = row;
    s.col = col;
    s.source = source;
    g.sparklines.append(s);
    return g;
}

bool sparklineTabShown(CrunchWindow &w)
{
    jp::Ribbon *r = w.ribbon();
    for (int i = 0; i < r->tabCount(); ++i)
        if (r->tabName(i) == QLatin1String("Sparkline")) return r->tabVisible(i);
    return false;
}

} // namespace

class CrunchSparklineTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    void sparklinesAreMadeUndoneRedoneAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QSignalSpy changed(s, &crunch::EditSession::sparklinesChanged);
        QVERIFY(e.win.w.calculation()->sheet(0).sparklineGroups.isEmpty());
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(s->setSparklines(0, {groupOf(SparklineGroup::Type::Column, 0, 5, QStringLiteral("Sheet1!A1:A3"))}, QStringLiteral("Create Sparklines")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Create Sparklines"));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(int(e.win.w.calculation()->sheet(0).sparklineGroups.size()), 1);
        // the same groups again is no step; a group with no sparklines is dropped
        QVERIFY(s->setSparklines(0, s->sparklinesOf(0), QStringLiteral("Nothing")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QVERIFY(s->sparklinesOf(0).isEmpty());
        QCOMPARE(changed.count(), 2);
        e.win.w.undoStack()->redo();
        QCOMPARE(int(s->sparklinesOf(0).size()), 1);
        SparklineGroup empty;
        QVERIFY(s->setSparklines(0, {s->sparklinesOf(0).first(), empty}, QStringLiteral("Edit Sparklines")).ok);
        QCOMPARE(int(s->sparklinesOf(0).size()), 1);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);   // (the empty group made no step)
        // saved, and read when the file is opened again
        QVERIFY(saveNow(e));
        Package package;
        Workbook book;
        QString why;
        QVERIFY(package.readLazyFile(e.path, &why) && book.open(package, &why));
        Sheet sheet;
        QVERIFY(book.readSheet(0, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 1);
        QCOMPARE(int(sheet.sparklineGroups[0].type), int(SparklineGroup::Type::Column));
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].source, QStringLiteral("Sheet1!A1:A3"));
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].col, quint32(5));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.calculation() != nullptr);
        QCOMPARE(int(second.w.calculation()->sheet(0).sparklineGroups.size()), 1);
    }

    void sparklinesAreMadeFromTheSelectionAndTheTabShows()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QString askedData, askedWhere = QStringLiteral("E1");
        w.setSparklineAsker([&](QString *data, QString *where) {
            askedData = *data;
            *where = askedWhere;
            return true;
        });
        QVERIFY(!sparklineTabShown(w));
        e.view()->selectRange(0, 0, 2, 0);
        w.act(QStringLiteral("spark.insert.column"))->trigger();
        QCOMPARE(askedData, QStringLiteral("A1:A3"));
        const auto &groups = w.session()->sparklinesOf(0);
        QCOMPARE(int(groups.size()), 1);
        QCOMPARE(int(groups[0].type), int(SparklineGroup::Type::Column));
        QCOMPARE(groups[0].sparklines[0].source, QStringLiteral("Sheet1!$A$1:$A$3"));
        QCOMPARE(groups[0].sparklines[0].row, quint32(0));
        QCOMPARE(groups[0].sparklines[0].col, quint32(4));
        // the cursor is on it, and the tab shows, with the kind checked
        QCOMPARE(e.selected(), QStringLiteral("E1"));
        QVERIFY(sparklineTabShown(w));
        QVERIFY(w.act(QStringLiteral("spark.type.column"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("spark.type.line"))->isChecked());
        e.select(QStringLiteral("A1"));
        QVERIFY(!sparklineTabShown(w));
        e.select(QStringLiteral("E1"));
        // one for each row of the data, in a column of cells; counts that do not fit are said
        e.view()->selectRange(0, 0, 2, 1);
        askedWhere = QStringLiteral("G1:G3");
        w.act(QStringLiteral("spark.insert.line"))->trigger();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 2);
        QCOMPARE(int(w.session()->sparklinesOf(0)[1].sparklines.size()), 3);
        QCOMPARE(w.session()->sparklinesOf(0)[1].sparklines[2].source, QStringLiteral("Sheet1!$A$3:$B$3"));
        askedWhere = QStringLiteral("I1:I5");
        e.view()->selectRange(0, 0, 2, 1);
        w.act(QStringLiteral("spark.insert.line"))->trigger();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 2);
        QVERIFY(e.win.told.last().contains(QLatin1String("3 rows")) || e.win.told.last().contains(QLatin1String("counts")));
        // a new group over cells that have sparklines takes them over
        askedWhere = QStringLiteral("E1");
        e.view()->selectRange(0, 0, 0, 2);
        w.act(QStringLiteral("spark.insert.winloss"))->trigger();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 2);   // (the column group had only E1: it is gone)
        QCOMPARE(int(w.session()->sparklinesOf(0)[1].type), int(SparklineGroup::Type::WinLoss));
        e.win.w.undoStack()->undo();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 2);
        QCOMPARE(int(w.session()->sparklinesOf(0)[0].type), int(SparklineGroup::Type::Column));
    }

    void theTabChangesGroupsAsStepsOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QString where = QStringLiteral("F1:F2");
        w.setSparklineAsker([&](QString *, QString *loc) {
            *loc = where;
            return true;
        });
        e.view()->selectRange(0, 0, 1, 1);
        w.act(QStringLiteral("spark.insert.line"))->trigger();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 1);
        e.select(QStringLiteral("F1"));
        auto group = [&]() { return w.session()->sparklinesOf(0).first(); };
        w.act(QStringLiteral("spark.show.high"))->trigger();
        QVERIFY(group().high);
        w.act(QStringLiteral("spark.show.markers"))->trigger();
        QVERIFY(group().markers);
        w.act(QStringLiteral("spark.type.winloss"))->trigger();
        QCOMPARE(int(group().type), int(SparklineGroup::Type::WinLoss));
        QVERIFY(w.act(QStringLiteral("spark.type.winloss"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("spark.show.markers"))->isEnabled());   // (markers are for lines)
        w.undoStack()->undo();
        QCOMPARE(int(group().type), int(SparklineGroup::Type::Line));
        QVERIFY(w.act(QStringLiteral("spark.type.line"))->isChecked());
        w.act(QStringLiteral("spark.empty.span"))->trigger();
        QCOMPARE(int(group().empty), int(SparklineGroup::Empty::Span));
        w.act(QStringLiteral("spark.axis.show"))->trigger();
        QVERIFY(group().showAxis);
        // the data of the group is edited: another place and other data
        where = QStringLiteral("F1:F3");
        w.setSparklineAsker([&](QString *data, QString *loc) {
            *data = QStringLiteral("A1:B3");
            *loc = where;
            return true;
        });
        w.act(QStringLiteral("spark.editGroup"))->trigger();
        QCOMPARE(int(group().sparklines.size()), 3);
        QVERIFY(group().showAxis);   // (its look is kept)
        // ungroup makes each its own, group puts them together
        e.view()->selectRange(0, 5, 2, 5);
        w.act(QStringLiteral("spark.ungroup"))->trigger();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 3);
        QVERIFY(w.session()->sparklinesOf(0)[1].showAxis);
        w.act(QStringLiteral("spark.group"))->trigger();
        QCOMPARE(int(w.session()->sparklinesOf(0).size()), 1);
        QCOMPARE(int(group().sparklines.size()), 3);
        // clearing the selected ones leaves the others; clearing the groups takes the group
        e.select(QStringLiteral("F2"));
        w.act(QStringLiteral("spark.clearSelected"))->trigger();
        QCOMPARE(int(group().sparklines.size()), 2);
        w.act(QStringLiteral("spark.show.low"))->trigger();   // (no sparkline is selected: no change)
        QVERIFY(!group().low);
        e.select(QStringLiteral("F1"));
        w.act(QStringLiteral("spark.clearGroups"))->trigger();
        QVERIFY(w.session()->sparklinesOf(0).isEmpty());
        QVERIFY(!sparklineTabShown(w));
    }

    void sparklinesFollowTheirCellsWhenRowsAreInserted()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->setSparklines(0, {groupOf(SparklineGroup::Type::Column, 1, 5, QStringLiteral("Sheet1!$A$1:$A$3")), groupOf(SparklineGroup::Type::Line, 2, 6, QStringLiteral("Sheet1!$A$3:$B$3"))}, QStringLiteral("Test")).ok);
        QVERIFY(s->insertRows(0, 1, 2).ok);   // two rows before row 2
        QCOMPARE(s->sparklinesOf(0)[0].sparklines[0].row, quint32(3));
        QCOMPARE(s->sparklinesOf(0)[0].sparklines[0].source, QStringLiteral("Sheet1!$A$1:$A$5"));
        QCOMPARE(s->sparklinesOf(0)[1].sparklines[0].row, quint32(4));
        QCOMPARE(s->sparklinesOf(0)[1].sparklines[0].source, QStringLiteral("Sheet1!$A$5:$B$5"));
        e.win.w.undoStack()->undo();
        QCOMPARE(s->sparklinesOf(0)[0].sparklines[0].row, quint32(1));
        QCOMPARE(s->sparklinesOf(0)[0].sparklines[0].source, QStringLiteral("Sheet1!$A$1:$A$3"));
        // deleting the row a sparkline is in takes it away; Undo brings it back; and deleting all the cells it reads takes it too
        QVERIFY(s->deleteRows(0, 1, 1).ok);
        QCOMPARE(int(s->sparklinesOf(0).size()), 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(s->sparklinesOf(0).size()), 2);
        // the file has the sparklines where they moved to
        QVERIFY(s->insertRows(0, 0, 1).ok);
        QVERIFY(saveNow(e));
        Package package;
        Workbook book;
        QString why;
        QVERIFY(package.readLazyFile(e.path, &why) && book.open(package, &why));
        Sheet sheet;
        QVERIFY(book.readSheet(0, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 2);
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].row, quint32(2));
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].source, QStringLiteral("Sheet1!$A$2:$A$4"));
    }
};

JP_TEST_CLASS(CrunchSparklineTests)
#include "tst_crunch_sparklines.moc"
