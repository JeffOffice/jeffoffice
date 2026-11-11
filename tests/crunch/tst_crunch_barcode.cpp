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
};

JP_TEST_CLASS(CrunchBarcodeTests)
#include "tst_crunch_barcode.moc"
