// Tests of JeffCrunch's window with a workbook that has what-if (data) tables: opened, its tables are worked out and shown; a change to what the formula reads works them out again, and the
// file saved holds the new values; a workbook set to leave tables by hand keeps what the file stored until F9. Every file is built here.
#include "../sheet/odshelpers.h"
#include "app/crunchwindow.h"
#include "core/loader.h"
#include "grid/gridview.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace jp;

namespace {

QString cellXml(const QString &ref, const QString &inner) { return QStringLiteral("<c r=\"%1\">%2</c>").arg(ref, inner); }
QString num(const QString &ref, double v) { return cellXml(ref, QStringLiteral("<v>%1</v>").arg(v)); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

// C2 = A2*B2; the table B5:B7 puts A5:A7 in A2 (B4 holds =C2).
QByteArray book(const QString &calcPr = QString())
{
    QString rows = row(2, num(QStringLiteral("A2"), 1) + num(QStringLiteral("B2"), 2) + cellXml(QStringLiteral("C2"), QStringLiteral("<f>A2*B2</f><v>2</v>")));
    rows += row(4, cellXml(QStringLiteral("B4"), QStringLiteral("<f>C2</f><v>2</v>")));
    rows += row(5, num(QStringLiteral("A5"), 1) + cellXml(QStringLiteral("B5"), QStringLiteral("<f t=\"dataTable\" ref=\"B5:B7\" dt2D=\"0\" dtr=\"0\" r1=\"A2\"/><v>0</v>")));
    rows += row(6, num(QStringLiteral("A6"), 2) + num(QStringLiteral("B6"), 0));
    rows += row(7, num(QStringLiteral("A7"), 3) + num(QStringLiteral("B7"), 0));
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), calcPr);
}

struct Win {
    QTemporaryDir dir;
    CrunchWindow w;
    QString path;
    Win()
    {
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 640);
        w.show();
    }
    bool open(const QByteArray &bytes)
    {
        path = dir.filePath(QStringLiteral("tables.xlsx"));
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return false;
        f.write(bytes);
        f.close();
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        if (!(first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000)))) return false;
        w.activateWindow();
        return QTest::qWaitForWindowActive(&w);
    }
    bool settle() { return QTest::qWaitFor([&] { return !w.isCalculating(); }, 20000); }
    void enter(quint32 r, quint32 c, const QString &text)
    {
        w.view()->selectCell(r, c);
        QTest::keyClick(w.view(), text.at(0).toLatin1());
        if (text.size() > 1) QTest::keyClicks(w.cellEditor(), text.mid(1));
        QTest::keyClick(w.cellEditor(), Qt::Key_Return);
        settle();
    }
    QString shown(quint32 r, quint32 c) { return w.view()->sheet().textOf(r, c, *w.view()->sheet().cellAt(r, c)); }
    QString saved(const QString &ref)
    {
        QByteArray bytes;
        QString why;
        if (!w.snapshotBytesForTest(&bytes, &why)) return QStringLiteral("no save: ") + why;
        Package p;
        Workbook wb;
        Sheet s;
        if (!p.readLazy(bytes, &why) || !wb.open(p, &why) || !wb.readSheet(0, &s)) return QStringLiteral("no read: ") + why;
        return show(s, wb.sharedStrings(), ref);
    }
};

} // namespace

class CrunchDataTableTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void aTableIsWorkedOutAndFollowsAChange()
    {
        Win o;
        QVERIFY(o.open(book()));
        QCOMPARE(o.shown(5, 1), QStringLiteral("4"));   // (the file stored 0)
        QCOMPARE(o.shown(6, 1), QStringLiteral("6"));
        o.enter(1, 1, QStringLiteral("5"));   // B2 := 5
        QCOMPARE(o.shown(5, 1), QStringLiteral("10"));
        QCOMPARE(o.shown(6, 1), QStringLiteral("15"));
        // the file saved holds the new values
        QCOMPARE(o.saved(QStringLiteral("B6")), QStringLiteral("n:10"));
        QCOMPARE(o.saved(QStringLiteral("B7")), QStringLiteral("n:15"));
    }

    // The setting made before the workbook is read whole (the person changed it while the file was loading) is the one the worker works by: it is not lost for want of a workbook to give it to.
    void tablesByHandSetWhileTheFileIsLoadingStayByHand()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("tables.xlsx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(book());
        f.close();
        crunch::WorkbookLoader loader;
        loader.setTablesAutomatic(false);   // (the file says automatic; the person has said by hand)
        QSignalSpy finished(&loader, &crunch::WorkbookLoader::finished);
        loader.start(path);
        QVERIFY(finished.wait(20000));
        QVERIFY(loader.result() && loader.result()->calc);
        QVERIFY(!loader.result()->calc->dataTablesAutomatic());
        // and a setting made after the load is done is the engine's at once
        loader.setTablesAutomatic(true);
        QVERIFY(loader.result()->calc->dataTablesAutomatic());
        loader.cancel();
    }

    // The setting made while the file is being read (after start(), with the read slowed so that it is still going): the worker works by it when it has the workbook.
    void tablesByHandSetDuringTheLoadStayByHand()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("tables.xlsx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(book());
        f.close();
        crunch::WorkbookLoader loader;
        loader.setPace(-1);   // (the read is held where it is: the window says its word in the middle of it, however busy the machine is)
        QSignalSpy first(&loader, &crunch::WorkbookLoader::firstScreen), finished(&loader, &crunch::WorkbookLoader::finished);
        loader.start(path);
        QVERIFY(first.wait(20000));
        QVERIFY(finished.isEmpty());
        loader.setTablesAutomatic(false);
        loader.setPace(0);   // (let go)
        QVERIFY(finished.wait(20000));
        QVERIFY(loader.result() && loader.result()->calc);
        QVERIFY(!loader.result()->calc->dataTablesAutomatic());
        loader.cancel();
    }

    void tablesByHandWaitForTheKey()
    {
        Win o;
        QVERIFY(o.open(book(QStringLiteral("calcMode=\"autoNoTable\""))));
        QCOMPARE(o.shown(5, 1), QStringLiteral("0"));   // (as the file stored it)
        o.w.act("formulas.calculate")->trigger();
        QVERIFY(o.settle());
        QCOMPARE(o.shown(5, 1), QStringLiteral("4"));
    }
};

JP_TEST_CLASS(CrunchDataTableTests)
#include "tst_crunch_datatable.moc"
