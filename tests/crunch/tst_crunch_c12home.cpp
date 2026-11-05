// Tests of the Home, Insert, Formulas, and Data tab items added in round 12: Icons on the sheet, Edit Links, and the rest of the small items listed with them. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "core/sheetobjects.h"
#include "sheet/drawing.h"

#include <QAction>
#include <QSettings>
#include <QTemporaryDir>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

class CrunchHomeInsertTests : public QObject {
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
    }

    // Insert > Icons: the icons the shared picker returns are put on the sheet as pictures, one step of Undo each; no choice puts nothing.
    void iconsFromTheSharedPickerAreInsertedAsPictures()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(e.win.w.act(QStringLiteral("insert.icons")));
        QVERIFY(e.win.w.act(QStringLiteral("insert.icons"))->isEnabled());
        e.win.w.setIconsPicker([] { return QStringList(); });
        QVERIFY(!e.win.w.insertIcons());
        QCOMPARE(e.win.w.objects()->layer()->count(), 0);
        const QStringList names = {QStringLiteral("smile-plus"), QStringLiteral("link")};
        e.win.w.setIconsPicker([names] { return names; });
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.insertIcons());
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->layer()->count(), 2);
        QCOMPARE(o->kindOf(0, o->idsOf(0).front()), jp::office::sheet::ObjectKind::Picture);
        const grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
        QCOMPARE(box.w, 48.0);
        QCOMPARE(box.h, 48.0);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        pressUndo(&e.win.w);
        QCOMPARE(int(o->idsOf(0).size()), 1);
        pressUndo(&e.win.w);
        QCOMPARE(int(o->idsOf(0).size()), 0);
        pressRedo(&e.win.w);
        QCOMPARE(int(o->idsOf(0).size()), 1);
    }
    // A metafile (WMF) is drawn and put on the sheet as a PNG picture of it, in proportion; a file that only looks like one is turned down.
    void aMetafileIsInsertedAsAPictureOfIt()
    {
        auto le16 = [](QByteArray &b, quint32 v) { b.append(char(v & 0xFF)).append(char((v >> 8) & 0xFF)); };
        auto le32 = [&](QByteArray &b, quint32 v) { le16(b, v & 0xFFFF); le16(b, v >> 16); };
        auto record = [&](quint32 function, const QList<quint32> &params) {
            QByteArray r;
            le32(r, quint32(3 + params.size()));
            le16(r, function);
            for (quint32 p : params) le16(r, p);
            return r;
        };
        QByteArray body = record(0x020C, {100, 200}) + record(0x020B, {0, 0});                      // window extent (height, width) and origin
        body += record(0x02FC, {0, 0x00FF, 0x0000, 0}) + record(0x012D, {0});                       // a red brush, selected
        body += record(0x041B, {100, 200, 0, 0}) + record(0x0000, {});                              // a rectangle over all of it, and the end
        QByteArray standard;
        le16(standard, 1);
        le16(standard, 9);
        le16(standard, 0x300);
        le32(standard, quint32((18 + body.size()) / 2));
        le16(standard, 1);
        le32(standard, 8);
        le16(standard, 0);
        QByteArray placeable;
        le32(placeable, 0x9AC6CDD7u);
        le16(placeable, 0);
        for (quint32 v : {0u, 0u, 200u, 100u, 96u}) le16(placeable, v);
        le32(placeable, 0);
        quint32 sum = 0;
        for (int i = 0; i < 20; i += 2) sum ^= quint32(quint8(placeable[i])) | (quint32(quint8(placeable[i + 1])) << 8);
        le16(placeable, sum);
        const QByteArray wmf = placeable + standard + body;
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(!e.win.w.insertPictureData("not a metafile at all", QStringLiteral("x.wmf")));
        QCOMPARE(e.win.w.objects()->layer()->count(), 0);
        QVERIFY(e.win.w.insertPictureData(wmf, QStringLiteral("shape.wmf")));
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->layer()->count(), 1);
        QCOMPARE(o->kindOf(0, o->layer()->selectedId()), jp::office::sheet::ObjectKind::Picture);
        const grid::ObjectBox box = o->layer()->boxNow(o->layer()->selectedId());
        QVERIFY2(box.w > box.h * 1.5 && box.w < box.h * 2.5, qPrintable(QStringLiteral("%1 x %2").arg(box.w).arg(box.h)));
    }
};

JP_TEST_CLASS(CrunchHomeInsertTests)
#include "tst_crunch_c12home.moc"
