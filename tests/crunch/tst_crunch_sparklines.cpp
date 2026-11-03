// Tests of the sparklines in JeffCrunch: made, changed, and taken away as steps of Undo, saved into the file's extension list, and the same when the file is opened again.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

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
};

JP_TEST_CLASS(CrunchSparklineTests)
#include "tst_crunch_sparklines.moc"
