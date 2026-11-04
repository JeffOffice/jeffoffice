// Tests of the sparklines in JeffCrunch: made, changed, and taken away as steps of Undo, saved into the file's extension list, and the same when the file is opened again.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "app/ribbon.h"

#include <QElapsedTimer>
#include <QFile>
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


// Two sheets: the first has sparklines that read the second (and a third that reads itself), the second has the numbers.
QByteArray twoSheets(const QString &secondName = QStringLiteral("Sheet2"))
{
    auto group = [](const QString &source, int row, const QString &attrs) {
        return QStringLiteral("<x14:sparklineGroup ") + attrs + QStringLiteral("><x14:colorSeries rgb=\"FF376092\"/><x14:sparklines><x14:sparkline><xm:f>") + source.toHtmlEscaped()
               + QStringLiteral("</xm:f><xm:sqref>E") + QString::number(row) + QStringLiteral("</xm:sqref></x14:sparkline></x14:sparklines></x14:sparklineGroup>");
    };
    const QString ext = QStringLiteral("<extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
                                       "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\">")
                        + group(QStringLiteral("Sheet2!A1:C1"), 1, QString()) + group(QStringLiteral("Sheet1!A1:C1"), 2, QStringLiteral("type=\"column\"")) + QStringLiteral("</x14:sparklineGroups></ext></extLst>");
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>3</v></c><c r=\"C1\"><v>2</v></c></row>");
    return buildXlsx({worksheet(rows, ext), worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList{QStringLiteral("Sheet1"), secondName});
}

bool reread(const QString &path, Sheet *sheet, int index = 0)
{
    Package package;
    Workbook book;
    QString why;
    return package.readLazyFile(path, &why) && book.open(package, &why) && book.readSheet(index, sheet);
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
        // a date axis: the points are spaced by the dates in the cells given (as an absolute reference with the sheet), and none given turns it off; each is one step
        QString dates = QStringLiteral("A1:B1");
        w.setDateAxisAsker([&](QString *range) {
            *range = dates;
            return true;
        });
        w.act(QStringLiteral("spark.axis.date"))->trigger();
        QVERIFY(group().dateAxis);
        QCOMPARE(group().dateRange, QStringLiteral("Sheet1!$A$1:$B$1"));
        dates = QStringLiteral("not a range");
        w.act(QStringLiteral("spark.axis.date"))->trigger();
        QCOMPARE(group().dateRange, QStringLiteral("Sheet1!$A$1:$B$1"));   // (refused: nothing changed)
        dates.clear();
        w.act(QStringLiteral("spark.axis.date"))->trigger();
        QVERIFY(!group().dateAxis);
        QVERIFY(group().dateRange.isEmpty());
        w.undoStack()->undo();
        QVERIFY(group().dateAxis);
        QCOMPARE(group().dateRange, QStringLiteral("Sheet1!$A$1:$B$1"));
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

    // A sheet renamed has the sources that read it written as they now are (an emoji in its name too), and the file has them; Undo gives them back, and the file has those.
    void aRenamedSheetsSourcesAreSavedAndReadAgain()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const QString emoji = QString::fromUtf8("Data \xF0\x9F\x98\x80");
        QVERIFY(s->renameSheet(1, emoji).ok);
        QCOMPARE(s->sparklinesOf(0)[0].sparklines[0].source, QStringLiteral("'%1'!A1:C1").arg(emoji));
        QVERIFY(saveNow(e));
        Sheet sheet;
        QVERIFY(reread(e.path, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 2);
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].source, QStringLiteral("'%1'!A1:C1").arg(emoji));
        QCOMPARE(sheet.sparklineGroups[1].sparklines[0].source, QStringLiteral("Sheet1!A1:C1"));
    }

    // The rename undone before a save is not in the file either.
    void aRenameUndoneBeforeASaveLeavesTheOriginalSources()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->renameSheet(1, QStringLiteral("Other")).ok);
        e.win.w.undoStack()->undo();
        QCOMPARE(s->sparklinesOf(0)[0].sparklines[0].source, QStringLiteral("Sheet2!A1:C1"));
        QVERIFY(saveNow(e));
        Sheet sheet;
        QVERIFY(reread(e.path, &sheet));
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].source, QStringLiteral("Sheet2!A1:C1"));
    }

    // A sheet deleted takes the sparklines that read it, in the file too; Undo brings them back.
    void aDeletedSheetTakesTheSparklinesThatReadItAndSavesSo()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->deleteSheet(1).ok);
        QCOMPARE(int(s->sparklinesOf(0).size()), 1);   // (the one that reads Sheet1 stays)
        QVERIFY(saveNow(e));
        Sheet sheet;
        QVERIFY(reread(e.path, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 1);
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].source, QStringLiteral("Sheet1!A1:C1"));
    }

    void aDeleteUndoneBeforeASaveLeavesTheSparklines()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->deleteSheet(1).ok);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(s->sparklinesOf(0).size()), 2);
        QVERIFY(saveNow(e));
        Sheet sheet;
        QVERIFY(reread(e.path, &sheet));
        QCOMPARE(int(sheet.sparklineGroups.size()), 2);
        QCOMPARE(sheet.sparklineGroups[0].sparklines[0].source, QStringLiteral("Sheet2!A1:C1"));
    }

    // A sheet whose sparklines an edit did not change is not written again for it: its part is as the file had it, to the byte.
    void aSheetWhoseSparklinesDidNotChangeIsLeftAsItWas()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        auto part = [&](const QString &path) {
            QMap<QString, QByteArray> parts;
            QFile f(path);
            f.open(QIODevice::ReadOnly);
            jp::readZip(f.readAll(), parts);
            return parts.value(QStringLiteral("xl/worksheets/sheet1.xml"));
        };
        const QByteArray before = part(e.path);
        // rows inserted on the second sheet move no cell that the first sheet's sparklines read through (they read A1:C1, above the rows)
        QVERIFY(s->insertRows(1, 5, 2).ok);
        QVERIFY(saveNow(e));
        QCOMPARE(part(e.path), before);
    }

    // Sparklines the model can't hold whole (a range of cells for one sparkline) are shown but not changed, and the file keeps them whole, moved with the rows.
    void sparklinesTheModelCannotHoldAreLeftAlone()
    {
        const QString ext = QStringLiteral("<extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
                                           "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"><x14:sparklineGroup><x14:sparklines>"
                                           "<x14:sparkline><xm:f>Sheet1!A1:C1</xm:f><xm:sqref>E1:E3</xm:sqref></x14:sparkline><x14:sparkline><xm:f>Sheet1!A2:C2</xm:f><xm:sqref>F2</xm:sqref></x14:sparkline>"
                                           "</x14:sparklines></x14:sparklineGroup></x14:sparklineGroups></ext></extLst>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), ext)})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.win.w.calculation()->sheet(0).sparklinesLossy);
        const auto o = s->setSparklines(0, {groupOf(SparklineGroup::Type::Line, 0, 5, QStringLiteral("Sheet1!A1:C1"))}, QStringLiteral("Create Sparklines"));
        QVERIFY(!o.ok);
        QVERIFY(!o.problem.isEmpty());
        QVERIFY(s->insertRows(0, 0, 1).ok);
        QVERIFY(saveNow(e));
        Package package;
        Workbook book;
        QString why;
        QVERIFY(package.readLazyFile(e.path, &why) && book.open(package, &why));
        const QByteArray part = package.part(book.info().sheets[0].part);
        QVERIFY2(part.contains("<xm:sqref>E2:E4</xm:sqref>") && part.contains("Sheet1!A3:C3"), part.constData());   // (kept whole, and moved with the row inserted)
    }

    // Making sparklines for many cells over many that are there is not slow (the cells are looked up in a set, not compared one by one).
    void makingManySparklinesOverManyIsNotQuadratic()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = e.win.w.session();
        // 60,000 sparklines are there, in H1:H60000
        SparklineGroup g;
        g.series = ColorSpec::fromRgb(0x376092);
        for (quint32 r = 0; r < 60000; ++r) {
            Sparkline sp;
            sp.row = r;
            sp.col = 7;
            sp.source = QStringLiteral("Sheet1!$A$1:$C$1");
            g.sparklines.append(sp);
        }
        QVERIFY(s->setSparklines(0, {g}, QStringLiteral("Test")).ok);
        w.setSparklineAsker([&](QString *data, QString *where) {
            *data = QStringLiteral("A1:C60000");
            *where = QStringLiteral("H1:H60000");
            return true;
        });
        QElapsedTimer timer;
        timer.start();
        e.view()->selectRange(0, 0, 0, 2);
        w.act(QStringLiteral("spark.insert.line"))->trigger();
        QVERIFY2(timer.elapsed() < 15000, "bounded time");
        qint64 total = 0;
        for (const SparklineGroup &x : s->sparklinesOf(0)) total += x.sparklines.size();
        QCOMPARE(total, qint64(60000));
        QCOMPARE(int(s->sparklinesOf(0).size()), 1);   // (the old group had nothing left and is gone)
    }
};

JP_TEST_CLASS(CrunchSparklineTests)
#include "tst_crunch_sparklines.moc"
