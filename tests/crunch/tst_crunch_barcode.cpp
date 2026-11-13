// Tests of Insert > Barcode in JeffCrunch: a barcode is an object over the sheet (a picture that is moved, sized, edited again, and undone like any), saved as a picture other programs
// show (an SVG with a PNG beside it) that carries the marker JeffCrunch opens as the same barcode.
#include "../sheet/drawinghelpers.h"
#include "../withdialog.h"
#include "editwindow.h"
#include "savedok.h"

#include "app/crunchinfo.h"
#include "core/barcodeitem.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "io/zip.h"
#include "office/package.h"

#include <QComboBox>
#include <QElapsedTimer>
#include <QFile>
#include <QLineEdit>
#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace crunchtest;
using namespace drawtest;
using namespace jp::office::sheet;

namespace {

QByteArray plainBook() { return buildXlsx({dataSheet()}); }

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    return parts;
}

} // namespace

class CrunchBarcodeTests : public QObject {
    Q_OBJECT

    static bool save(EditWindow &e)
    {
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        return e.win.w.save() && savedOk(saved);
    }
    // Insert > Barcode with the dialog answered: a QR code of `text`.
    static void insertQr(EditWindow &e, const QString &text)
    {
        withDialog([&] { e.win.w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Insert Barcode"));
            d->findChildren<QComboBox *>().first()->setCurrentIndex(int(jp::barcode::Type::Qr));
            d->findChildren<QLineEdit *>().first()->setText(text);
        });
    }

    // A Code 128 barcode made from the text of A2 ("Q1"), linked to that cell; its id.
    static int linkedBarcode(EditWindow &e)
    {
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = range.row2 = 1;
        range.col1 = range.col2 = 0;
        const QJsonObject look{{"type", int(jp::barcode::Type::Code128)}, {"data", "sample"}, {"text", true}};
        const CrunchWindow::BarcodeColumnResult r = e.win.w.insertBarcodesFromCells(range, look);
        if (r.made != 1) return -1;
        return e.win.w.objects()->idsOf(0).front();
    }
    // The data of the one barcode of the sheet that is not taken away (empty for none).
    static QString shownData(EditWindow &e)
    {
        const std::vector<int> ids = e.win.w.objects()->idsOf(0);
        return ids.size() == 1 ? e.win.w.objects()->barcodeOf(0, ids.front()).value(QLatin1String("data")).toString() : QStringLiteral("(%1 objects)").arg(ids.size());
    }
    static void letGo()
    {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    }

private Q_SLOTS:
    void initTestCase()
    {
        setAppInfo(crunchAppInfo());
        jp::initCore();
    }

    // Insert > Barcode puts a picture object on the sheet as one step: Undo takes it off, and Redo puts it back.
    void insertUndoAndRedo()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(e.win.w.act(QStringLiteral("insert.barcode")));
        crunch::SheetObjects *o = e.win.w.objects();
        const int steps = e.win.w.undoStack()->count();
        insertQr(e, QStringLiteral("https://jeffoffice.com/crunch"));
        QCOMPARE(o->layer()->count(), 1);
        const int id = o->layer()->selectedId();
        QCOMPARE(o->kindOf(0, id), ObjectKind::Picture);
        QCOMPARE(o->barcodeOf(0, id).value(QLatin1String("data")).toString(), QStringLiteral("https://jeffoffice.com/crunch"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const jp::grid::ObjectBox box = o->layer()->boxNow(id);
        QVERIFY(box.w > 50 && qAbs(box.w - box.h) < 0.5);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(o->idsOf(0).size()), 0);
        e.win.w.undoStack()->redo();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        QVERIFY(!o->barcodeOf(0, o->idsOf(0).front()).isEmpty());
    }

    // Barcodes for a column: a range of cells makes a barcode for each cell, with the cell's own text, as one step of Undo.
    void aColumnOfCellsMakesABarcodeForEach()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        const int steps = e.win.w.undoStack()->count();
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = 1;
        range.row2 = 3;
        range.col1 = range.col2 = 0;   // A2:A4: Q1, Q2, Q3
        const QJsonObject look{{"type", int(jp::barcode::Type::Code128)}, {"data", "sample"}, {"text", true}};
        const CrunchWindow::BarcodeColumnResult r = e.win.w.insertBarcodesFromCells(range, look);
        QCOMPARE(r.made, 3);
        QVERIFY(r.problems.isEmpty());
        QCOMPARE(int(o->idsOf(0).size()), 3);
        QStringList data;
        for (int id : o->idsOf(0)) data << o->barcodeOf(0, id).value(QLatin1String("data")).toString();
        QCOMPARE(data, (QStringList{"Q1", "Q2", "Q3"}));
        // Each is drawn apart from the others, one under the next.
        QSet<qint64> tops;
        for (int id : o->idsOf(0)) tops.insert(qint64(o->layer()->boxNow(id).y));
        QCOMPARE(int(tops.size()), 3);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(o->idsOf(0).size()), 0);
        e.win.w.undoStack()->redo();
        QCOMPARE(int(o->idsOf(0).size()), 3);
        // Saved and opened again, the three are still barcodes.
        QVERIFY(save(e));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 3);
        for (int id : second.w.objects()->idsOf(0)) QVERIFY(!second.w.objects()->barcodeOf(0, id).isEmpty());
    }

    // A value the code can't hold gets a message naming its cell, and no barcode; the count made at once is limited.
    void aValueTheCodeCannotHoldIsReportedNotDrawn()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        jp::office::sheet::CellRange range;
        range.valid = true;
        range.row1 = 1;
        range.row2 = 3;
        range.col1 = range.col2 = 0;
        const int steps = e.win.w.undoStack()->count();
        const QJsonObject ean{{"type", int(jp::barcode::Type::Ean8)}, {"data", "12345670"}};
        const CrunchWindow::BarcodeColumnResult bad = e.win.w.insertBarcodesFromCells(range, ean);
        QCOMPARE(bad.made, 0);
        QCOMPARE(int(bad.problems.size()), 3);
        QVERIFY2(bad.problems.first().startsWith(QLatin1String("A2")), qPrintable(bad.problems.first()));
        QCOMPARE(int(o->idsOf(0).size()), 0);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // More cells than the limit: nothing is made.
        const QJsonObject code128{{"type", int(jp::barcode::Type::Code128)}, {"data", "x"}};
        const CrunchWindow::BarcodeColumnResult over = e.win.w.insertBarcodesFromCells(range, code128, 2);
        QVERIFY(over.tooMany);
        QCOMPARE(over.made, 0);
        QCOMPARE(int(o->idsOf(0).size()), 0);
    }

    // Ctrl+A on a sheet with a few cells in it: only the cells the sheet uses are looked at, however large the selection is.
    void aHugeSelectionLooksOnlyAtTheUsedCells()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        jp::office::sheet::CellRange all;
        all.valid = true;
        all.row1 = all.col1 = 0;
        all.row2 = 1048575;
        all.col2 = 16383;
        const QJsonObject look{{"type", int(jp::barcode::Type::Code128)}, {"data", "x"}};
        QElapsedTimer timer;
        timer.start();
        const CrunchWindow::BarcodeColumnResult r = e.win.w.insertBarcodesFromCells(all, look);
        QVERIFY2(r.visited <= 100, qPrintable(QString::number(r.visited)));
        QVERIFY(timer.elapsed() < 20000);
    }

    // A picture of the window with a QR code on the sheet, for the docs, when JP_SHOT_DIR names a folder.
    void aPictureOfTheWindowForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        e.win.w.resize(1300, 820);
        e.win.w.show();
        insertQr(e, QStringLiteral("https://jeffoffice.com"));
        // (The object is drawn on a worker: the picture is taken once the bars are on the screen.)
        QImage shot;
        QTRY_VERIFY2([&] {
            shot = e.win.w.grab().toImage();
            int black = 0;
            for (int y = 200; y < shot.height() - 60; ++y)
                for (int x = 400; x < shot.width() - 100; ++x) black += (shot.pixel(x, y) & 0xFFFFFF) == 0 ? 1 : 0;
            return black > 300;
        }(), "the barcode was not drawn");
        shot.save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/crunch-qr.png"));
    }

    // With the barcode selected, Insert > Barcode edits it: the dialog starts from its data, the new barcode is where the old one was, and one step undoes it.
    void editingReopensTheDialogWithItsData()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        insertQr(e, QStringLiteral("first"));
        o->layer()->select(o->idsOf(0).front());
        const jp::grid::ObjectBox was = o->layer()->boxNow(o->layer()->selectedId());
        const int steps = e.win.w.undoStack()->count();
        withDialog([&] { e.win.w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Barcode"));
            auto *line = d->findChildren<QLineEdit *>().first();
            QCOMPARE(line->text(), QStringLiteral("first"));
            line->setText(QStringLiteral("second"));
        });
        QCOMPARE(int(o->idsOf(0).size()), 1);
        QCOMPARE(int(o->idsOf(0).size()), 1);
        const int id = o->layer()->selectedId();
        QCOMPARE(o->barcodeOf(0, id).value(QLatin1String("data")).toString(), QStringLiteral("second"));
        const jp::grid::ObjectBox now = o->layer()->boxNow(id);
        QVERIFY(qAbs(now.x - was.x) < 0.5 && qAbs(now.y - was.y) < 0.5);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(o->idsOf(0).size()), 1);
        QCOMPARE(o->barcodeOf(0, o->idsOf(0).front()).value(QLatin1String("data")).toString(), QStringLiteral("first"));
    }

    // Saved, the barcode is a picture of the drawing (an SVG with a PNG, each with its content type), with the marker; another program shows the picture. Opened again, it is the same
    // barcode, and saved a second time it is still one.
    void aSavedBarcodeIsAPictureAndReopensAsABarcode()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        insertQr(e, QStringLiteral("https://jeffoffice.com/round-trip"));
        const QJsonObject settings = o->barcodeOf(0, o->idsOf(0).front());
        QVERIFY(!settings.isEmpty());
        QVERIFY(save(e));
        const QMap<QString, QByteArray> parts = partsOf(e.path);
        const QByteArray drawing = parts.value(QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY2(drawing.contains("<xdr:pic"), drawing.left(1500).constData());
        QVERIFY(drawing.contains("jeffoffice.com/ns/barcode"));
        QVERIFY(drawing.contains("<asvg:svgBlip "));
        QVERIFY(drawing.contains("descr=\"Barcode: QR code https://jeffoffice.com/round-trip\""));
        QString svg, png;
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            if (it.key().startsWith(QLatin1String("xl/media/")) && it.key().endsWith(QLatin1String(".svg"))) svg = it.key();
            if (it.key().startsWith(QLatin1String("xl/media/")) && it.key().endsWith(QLatin1String(".png"))) png = it.key();
        }
        QVERIFY(!svg.isEmpty() && !png.isEmpty());
        QVERIFY(parts.value(svg).contains("<svg"));
        QVERIFY(parts.value(png).startsWith("\x89PNG"));
        jp::office::Package pkg;
        QVERIFY(pkg.read(QByteArray([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }())));
        QCOMPARE(pkg.contentType(QLatin1Char('/') + svg), QStringLiteral("image/svg+xml"));
        QCOMPARE(pkg.contentType(QLatin1Char('/') + png), QStringLiteral("image/png"));
        int images = 0;
        for (const jp::office::Relationship &r : pkg.relationships(QStringLiteral("/xl/drawings/drawing1.xml")))
            if (r.type.endsWith(QLatin1String("/image")) && (r.target == QLatin1Char('/') + svg || r.target == QLatin1Char('/') + png)) ++images;
        QCOMPARE(images, 2);

        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        crunch::SheetObjects *again = second.w.objects();
        QCOMPARE(again->barcodeOf(0, again->idsOf(0).front()), settings);
    }

    // A barcode made from a cell follows it: the cell changes, and the barcode is made again from its text in the same place, at the same size, as a step after the cell's. Undo gives the
    // old barcode back.
    void aCellEditUpdatesItsBarcode()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        const int first = linkedBarcode(e);
        QVERIFY(first >= 0);
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QCOMPARE(o->barcodeOf(0, first).value(QLatin1String("cell")).toString(), QStringLiteral("Sheet1!$A$2"));
        const jp::grid::ObjectBox was = o->layer()->boxNow(first);
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("NEW-42")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("NEW-42"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);   // (the cell, and the barcode)
        const int now = o->idsOf(0).front();
        QVERIFY(now != first);
        const jp::grid::ObjectBox box = o->layer()->boxNow(now);
        QVERIFY(qAbs(box.x - was.x) < 0.5 && qAbs(box.y - was.y) < 0.5 && qAbs(box.w - was.w) < 0.5 && qAbs(box.h - was.h) < 0.5);
        QCOMPARE(o->barcodeOf(0, now).value(QLatin1String("cell")).toString(), QStringLiteral("Sheet1!$A$2"));
        // Undo takes the new barcode away and gives the old one back.
        e.win.w.undoStack()->undo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QCOMPARE(o->idsOf(0).front(), first);
        e.win.w.undoStack()->redo();
        QCOMPARE(shownData(e), QStringLiteral("NEW-42"));
        // The cell Undone as well: the barcode, now stale, is not made again behind Redo's back.
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(e.win.w.undoStack()->canRedo());
    }

    // Rows inserted above the cell move the link with it, and a barcode follows the cell at its new place.
    void aRowInsertedAboveKeepsTheLink()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QCOMPARE(o->barcodeLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$A$4"));
        QVERIFY(e.win.w.session()->type(0, 3, 0, QStringLiteral("MOVED")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("MOVED"));
        // (the cell it had before is another now: typing there changes nothing)
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("OTHER")).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("MOVED"));
    }

    // The cell's row deleted: the barcode keeps its last value, and nothing breaks (here, or when the file is saved and opened).
    void aDeletedCellLeavesTheBarcodeAsItWas()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->deleteRows(0, 1, 1).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(o->linkedBarcodes().size() == 1 && !o->linkedBarcodes().front().found);
        QVERIFY(e.win.w.session()->type(0, 1, 0, QStringLiteral("Another")).ok);
        letGo();
        QCOMPARE(shownData(e), QStringLiteral("Q1"));
        QVERIFY(save(e));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        QVERIFY(!second.w.objects()->barcodeOf(0, second.w.objects()->idsOf(0).front()).isEmpty());
    }

    // Saved and opened again, the barcode is still linked: the picture in the file is of the text the cell had when it was saved (the marker is honored only for the picture that was written), the
    // link follows rows inserted before the save, and a change of the cell in the reopened workbook updates it; saved once more, the file has that.
    void theLinkSurvivesSaveAndReopen()
    {
        EditWindow e;
        QVERIFY(e.open(plainBook()));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(linkedBarcode(e) >= 0);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(e.win.w.session()->type(0, 2, 0, QStringLiteral("SAVED-1")).ok);
        QTRY_COMPARE(shownData(e), QStringLiteral("SAVED-1"));
        QVERIFY(save(e));
        EditWindow again;
        QVERIFY(again.open([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        QTRY_VERIFY(again.win.w.objects()->shownSheet() == 0 && again.win.w.objects()->layer()->count() == 1);
        crunch::SheetObjects *o = again.win.w.objects();
        QCOMPARE(shownData(again), QStringLiteral("SAVED-1"));
        QCOMPARE(o->barcodeLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$A$3"));
        // A cell changed in this workbook: the save writes the barcode from it, whether or not the window has done it yet.
        QVERIFY(again.win.w.session()->type(0, 2, 0, QStringLiteral("SAVED-2")).ok);
        {
            jp::office::sheet::SaveChanges changes;
            o->addToSave(&changes);
            QVERIFY(!changes.objectBarcodeMarkers.empty());
            QCOMPARE(int(changes.producedParts.size()), 2);
        }
        QTRY_COMPARE(shownData(again), QStringLiteral("SAVED-2"));
        QVERIFY(save(again));
        EditWindow third;
        QVERIFY(third.open([&] {
            QFile f(again.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(third.settle());
        QTRY_VERIFY(third.win.w.objects()->shownSheet() == 0 && third.win.w.objects()->layer()->count() == 1);
        QCOMPARE(shownData(third), QStringLiteral("SAVED-2"));
        QVERIFY(third.win.w.session()->type(0, 2, 0, QStringLiteral("SAVED-3")).ok);
        QTRY_COMPARE(shownData(third), QStringLiteral("SAVED-3"));
    }
};

JP_TEST_CLASS(CrunchBarcodeTests)
#include "tst_crunch_barcode.moc"
