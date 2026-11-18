// Tests of what JeffCrunch offers on a copy: cells as a range object and a chart as a chart object, for the other apps to paste as an object.
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "app/crunchwindow.h"
#include "app/objects.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
namespace ole = jp::office::ole;

class CrunchObjectTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    void copyingCellsOffersARangeObject()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(made->kind(), ole::Kind::Sheet);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Spreadsheet range"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }

    void copyingAChartOffersAChartObject()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=4", "B2=5", "B3=6"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        e.select(QStringLiteral("A1"));
        QVERIFY(e.win.w.insertChart(jp::office::sheet::ChartKind::Column));
        QApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Chart"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }
};

JP_TEST_CLASS(CrunchObjectTests)
#include "tst_crunch_objects.moc"
