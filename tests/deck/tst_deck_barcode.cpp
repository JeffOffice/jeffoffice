// JeffDeck's tests of Insert > Barcode: a barcode is an object on the slide (moved, sized, edited again, undone), and saved as a picture other programs show
// that JeffDeck opens as the same barcode.
#include "deckhelpers.h"
#include "../withdialog.h"
#include "core/barcodeitem.h"
#include "office/barcodemarker.h"

namespace {

// Insert > Barcode with the dialog answered: a QR code of `text`.
void insertQr(DeckWindow &w, const QString &text)
{
    withDialog([&] { w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
        QCOMPARE(d->windowTitle(), QStringLiteral("Insert Barcode"));
        auto *type = d->findChildren<QComboBox *>().first();
        type->setCurrentIndex(int(barcode::Type::Qr));
        d->findChildren<QLineEdit *>().first()->setText(text);
    });
}

// Whether the first slide's thumbnail (drawn again a moment after a change) has dark bars in it.
bool thumbnailHasBars(DeckWindow &w)
{
    const QImage thumb = w.slidesPane()->item(0)->icon().pixmap(w.slidesPane()->iconSize()).toImage();
    int dark = 0;
    for (int y = 0; y < thumb.height(); ++y)
        for (int x = 0; x < thumb.width(); ++x) dark += qGray(thumb.pixel(x, y)) < 128 ? 1 : 0;
    return dark > 10;
}

// The exported file with the entries `change` makes.
QByteArray repacked(const Exported &e, const std::function<void(QMap<QString, QByteArray> &)> &change)
{
    QMap<QString, QByteArray> parts = e.zip;
    change(parts);
    ZipWriter zip;
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) zip.add(it.key(), it.value(), true);
    return zip.finish();
}

GroupItem *lastBarcode(Document &d)
{
    for (auto it = d.pages[0]->items.rbegin(); it != d.pages[0]->items.rend(); ++it)
        if (isBarcodeItem(it->get())) return static_cast<GroupItem *>(it->get());
    return nullptr;
}

} // namespace

class DeckBarcodeTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Insert > Barcode puts a barcode in the middle of the slide as one step: Undo takes it away and Redo puts it back.
    void insertUndoAndRedo()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[0]->items.size());
        insertQr(w, QStringLiteral("https://jeffoffice.com/deck"));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        QCOMPARE(g->barcode.value(QLatin1String("data")).toString(), QStringLiteral("https://jeffoffice.com/deck"));
        QCOMPARE(g->barcode.value(QLatin1String("type")).toInt(), int(barcode::Type::Qr));
        QVERIFY(g->altText.startsWith(QLatin1String("Barcode: QR code")));
        const QSizeF page = ed->doc()->pageSize();
        QVERIFY(QRectF(QPointF(0, 0), page).contains(g->rect));
        ed->undoStack()->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        QVERIFY(!lastBarcode(*ed->doc()));
        ed->undoStack()->redo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        QVERIFY(lastBarcode(*ed->doc()));
    }

    // A new barcode lands where nothing else is (the lower right of the slide, not on the placeholders), and the slide's thumbnail shows it once the pane has
    // drawn it again (it does that a moment after a change).
    void aNewBarcodeLeavesThePlaceholdersAloneAndShowsInTheThumbnail()
    {
        DeckWindow w;
        w.resize(1300, 820);
        w.show();
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/room"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        for (const ItemPtr &it : ed->doc()->pages[0]->items)
            if (it.get() != g) QVERIFY2(!it->bounds().intersects(g->rect), qPrintable(it->name));
        const QSizeF page = ed->doc()->pageSize();
        QVERIFY(g->rect.center().x() > page.width() / 2 && g->rect.center().y() > page.height() / 2);
        QTRY_VERIFY(thumbnailHasBars(w));
    }

    // A picture of the window with a QR code on the slide, for the docs, when JP_SHOT_DIR names a folder.
    void aPictureOfTheWindowForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
        DeckWindow w;
        w.resize(1300, 820);
        w.show();
        insertQr(w, QStringLiteral("https://jeffoffice.com"));
        QTRY_VERIFY(thumbnailHasBars(w));
        QCoreApplication::processEvents();
        w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/deck-qr.png"));
    }

    // With the barcode selected, Insert > Barcode edits it: the dialog starts from its data, and the new barcode takes its place, one step back.
    void editingReopensTheDialogWithItsData()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("first"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        g->moveBy(-100, 40);
        const QString id = g->id;
        const QPointF where = g->rect.topLeft();
        ed->select(id);
        withDialog([&] { w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Barcode"));
            auto *line = d->findChildren<QLineEdit *>().first();
            QCOMPARE(line->text(), QStringLiteral("first"));
            line->setText(QStringLiteral("second"));
        });
        GroupItem *now = lastBarcode(*ed->doc());
        QVERIFY(now);
        QCOMPARE(now->id, id);
        QCOMPARE(now->rect.topLeft(), where);
        QCOMPARE(now->barcode.value(QLatin1String("data")).toString(), QStringLiteral("second"));
        ed->undoStack()->undo();
        QCOMPARE(lastBarcode(*ed->doc())->barcode.value(QLatin1String("data")).toString(), QStringLiteral("first"));
    }

    // Saved, a barcode is a picture of its own (the SVG, with a PNG for programs that don't draw SVG) in the slide's package, with the marker that
    // carries its data. Opened again, it is the same barcode, in the same place and at the size it was given, and can be edited.
    void aSavedBarcodeIsAPictureAndReopensAsABarcode()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/round-trip"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        // Moved and sized: 100 points wide.
        g->scaleInto(g->rect, QRectF(50, 60, 100, 100));
        g->rotation = 0;
        const QJsonObject settings = g->barcode;
        const QString description = g->altText;
        const int objects = int(ed->doc()->pages[0]->items.size());

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        const QString slide = part(e, slidePart);
        // Another program sees a picture, with the alternative text, and nothing it would draw as a pile of shapes.
        QVERIFY(slide.contains(QStringLiteral("<p:pic>")));
        QVERIFY(slide.contains(QStringLiteral("descr=\"Barcode: QR code https://jeffoffice.com/round-trip\"")));
        QVERIFY(slide.contains(QStringLiteral("jeffoffice.com/ns/barcode")));
        QString svgTarget, pngTarget;
        for (const QString &target : relationshipTargets(e, slidePart, QStringLiteral("/image"))) {
            if (target.endsWith(QLatin1String(".svg"))) svgTarget = target;
            if (target.endsWith(QLatin1String(".png"))) pngTarget = target;
        }
        QVERIFY(!svgTarget.isEmpty() && !pngTarget.isEmpty());
        QCOMPARE(e.pkg.contentType(svgTarget), QStringLiteral("image/svg+xml"));
        QCOMPARE(e.pkg.contentType(pngTarget), QStringLiteral("image/png"));
        QVERIFY(e.zip.value(svgTarget.mid(1)).contains("<svg"));
        QVERIFY(e.zip.value(pngTarget.mid(1)).startsWith("\x89PNG"));
        QVERIFY(slide.contains(QStringLiteral("<asvg:svgBlip ")));

        // Opened again: the same barcode.
        auto reopened = deck::importPresentation(bytes, &err);
        QVERIFY2(reopened, qPrintable(err));
        QCOMPARE(int(reopened->pages[0]->items.size()), objects);
        GroupItem *back = lastBarcode(*reopened);
        QVERIFY(back);
        QCOMPARE(back->barcode, settings);
        QCOMPARE(back->altText, description);
        QVERIFY(qAbs(back->rect.x() - 50) < 0.2 && qAbs(back->rect.y() - 60) < 0.2 && qAbs(back->rect.width() - 100) < 0.2 && qAbs(back->rect.height() - 100) < 0.2);
        // And saved a second time, it is the same file's worth of barcode.
        const QByteArray again = deck::exportPresentation(*reopened, &err);
        QVERIFY2(!again.isEmpty(), qPrintable(err));
        Exported e2;
        QVERIFY2(unpack(again, &e2, &err), qPrintable(err));
        QCOMPARE(part(e2, slidePart).count(QStringLiteral("jeffoffice.com/ns/barcode")), 1);
    }

    // In an open document presentation a barcode is a picture frame (a PNG) with the settings in an attribute of the frame; opened again, it is the same barcode.
    void inAnOpenDocumentFileItIsAPictureWithTheSettingsInTheFrame()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/odp"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        g->scaleInto(g->rect, QRectF(40, 50, 120, 120));
        const QJsonObject settings = g->barcode;
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportOdp(*ed->doc(), false, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(bytes, parts, &err));
        const QString content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("jo:barcode=")));
        QVERIFY(content.contains(QStringLiteral("<draw:image ")));
        bool png = false;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) png = png || (it.key().startsWith(QLatin1String("Pictures/")) && it.value().startsWith("\x89PNG"));
        QVERIFY(png);
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        GroupItem *again = lastBarcode(*back);
        QVERIFY(again);
        QCOMPARE(again->barcode, settings);
        QVERIFY(qAbs(again->rect.x() - 40) < 0.5 && qAbs(again->rect.width() - 120) < 0.5);
    }

    // A marker is honored only where the picture in the file is the one that was written: a photo put in the barcode's place (by another program's Change Picture) is a photo,
    // though the marker is still a valid one for a QR code; so is a changed SVG.
    void aPhotoWithAValidMarkerIsAPhoto()
    {
        DeckWindow w;
        insertQr(w, QStringLiteral("https://jeffoffice.com/photo"));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*w.editor()->doc(), &err);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QImage red(60, 40, QImage::Format_RGB32);
        red.fill(Qt::red);
        QByteArray photo;
        QBuffer buffer(&photo);
        buffer.open(QIODevice::WriteOnly);
        red.save(&buffer, "PNG");
        QString pngPart, svgPart;
        for (const QString &target : relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/image"))) {
            if (target.endsWith(QLatin1String(".png"))) pngPart = target;
            if (target.endsWith(QLatin1String(".svg"))) svgPart = target;
        }
        QVERIFY(!pngPart.isEmpty() && !svgPart.isEmpty());
        // The picture replaced, the marker as it was.
        auto swapped = deck::importPresentation(repacked(e, [&](QMap<QString, QByteArray> &parts) { parts[pngPart.mid(1)] = photo; }), &err);
        QVERIFY2(swapped, qPrintable(err));
        QVERIFY(!lastBarcode(*swapped));
        QVERIFY(firstPicture(*swapped));
        // The SVG changed, the PNG as it was.
        auto other = deck::importPresentation(repacked(e, [&](QMap<QString, QByteArray> &parts) { parts[svgPart.mid(1)] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"9\" height=\"9\"/>"; }), &err);
        QVERIFY2(other, qPrintable(err));
        QVERIFY(!lastBarcode(*other));
        // And untouched, it is the barcode.
        auto same = deck::importPresentation(repacked(e, [](QMap<QString, QByteArray> &) {}), &err);
        QVERIFY2(same, qPrintable(err));
        QVERIFY(lastBarcode(*same));
    }

    // A barcode turned a quarter, and one that is decorative, are the same barcodes when the file is opened again, in a .pptx and in an .odp.
    void aTurnedAndADecorativeBarcodeSurviveBothFiles()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/turn"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        g->scaleInto(g->rect, QRectF(100, 100, 120, 120));
        g->rotateAround(90, g->rect.center());
        g->decorative = true;
        double turn = 0;
        bool fh = true, fv = true;
        barcodeTurn(*g, &turn, &fh, &fv);
        QCOMPARE(turn, 90.0);
        QString err;
        const QByteArray pptx = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!pptx.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(pptx, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("rot=\"5400000\"")));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("jeffoffice.com/ns/barcode")));
        auto fromPptx = deck::importPresentation(pptx, &err);
        QVERIFY2(fromPptx, qPrintable(err));
        const QByteArray odp = deck::exportOdp(*ed->doc(), false, &err);
        QVERIFY2(!odp.isEmpty(), qPrintable(err));
        auto fromOdp = deck::importOdp(odp, &err);
        QVERIFY2(fromOdp, qPrintable(err));
        for (Document *d : {fromPptx.get(), fromOdp.get()}) {
            GroupItem *back = lastBarcode(*d);
            QVERIFY(back);
            barcodeTurn(*back, &turn, &fh, &fv);
            QVERIFY2(qAbs(turn - 90) < 0.01 && !fh && !fv, qPrintable(QString::number(turn)));
            QVERIFY2(qAbs(back->rect.width() - 120) < 0.5 && qAbs(back->rect.center().x() - 160) < 0.5, qPrintable(QString::number(back->rect.center().x())));
        }
        QVERIFY(lastBarcode(*fromPptx)->decorative);
    }

    // A slide of more barcodes than are honored has the rest as plain pictures, and many copies of one barcode are one picture in the file.
    void aSlideOfManyBarcodesHonorsTheFirstOnes()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        insertQr(w, QStringLiteral("https://jeffoffice.com/many"));
        GroupItem *g = lastBarcode(*ed->doc());
        QVERIFY(g);
        const int total = jp::office::kMostBarcodesHonored + 100;
        for (int i = 1; i < total; ++i) {
            ItemPtr copy = g->clone();
            copy->id = newId();
            copy->moveBy(i % 20, i % 20);
            ed->doc()->pages[0]->items.push_back(copy);
        }
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        int pictures = 0;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) pictures += it.key().startsWith(QLatin1String("ppt/media/")) ? 1 : 0;
        QCOMPARE(pictures, 2);   // (one PNG and one SVG)
        const int encodes = barcodeEncodes();
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        int barcodes = 0, plain = 0;
        for (const ItemPtr &it : back->pages[0]->items) {
            barcodes += isBarcodeItem(it.get()) ? 1 : 0;
            plain += it->type() == ItemType::Picture ? 1 : 0;
        }
        QCOMPARE(barcodes, jp::office::kMostBarcodesHonored);
        QCOMPARE(plain, 100);
        QVERIFY(barcodeEncodes() <= encodes + 1);   // (all copies of the one barcode are laid out once)
    }

    // An EAN with a wrong check digit shows what is wrong, and puts nothing on the slide.
    void anEanWithAWrongCheckDigitIsRefused()
    {
        DeckWindow w;
        const int before = int(w.editor()->doc()->pages[0]->items.size());
        QString shown;
        withDialog([&] { w.act(QStringLiteral("insert.barcode"))->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            d->findChildren<QComboBox *>().first()->setCurrentIndex(int(barcode::Type::Ean13));
            d->findChildren<QLineEdit *>().first()->setText(QStringLiteral("5901234123458"));
            for (QLabel *l : d->findChildren<QLabel *>()) shown += l->text();
            for (QPushButton *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Insert")) QVERIFY(!b->isEnabled());
        });
        QVERIFY2(shown.contains(QStringLiteral("check digit should be 7")), qPrintable(shown));
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), before);
    }

    // A marker that holds no barcode the encoder can draw (the data is gone) leaves the picture as an ordinary picture.
    void aMarkerWithNoBarcodeInItIsJustAPicture()
    {
        DeckWindow w;
        insertQr(w, QStringLiteral("x"));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*w.editor()->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("&quot;data&quot;:&quot;x&quot;")) || slide.contains(QStringLiteral("\"data\":\"x\"")));
        slide.replace(QStringLiteral("&quot;data&quot;:&quot;x&quot;"), QStringLiteral("&quot;data&quot;:&quot;&quot;"));
        e.zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QByteArray tampered;
        {
            ZipWriter zip;
            for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) zip.add(it.key(), it.value(), true);
            tampered = zip.finish();
        }
        auto d = deck::importPresentation(tampered, &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!lastBarcode(*d));
        QVERIFY(firstPicture(*d));
    }
};

JP_TEST_CLASS(DeckBarcodeTests)
#include "tst_deck_barcode.moc"
