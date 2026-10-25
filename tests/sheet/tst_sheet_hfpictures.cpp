// Tests of the pictures in headers and footers: read from the legacy drawing a sheet names (the shapes' ids say where each goes, their styles how large), bounded in how many are read and how
// large they may be, an image shared by two sheets read once, and found by the part of the header or footer and the form of the page.
#include "hfpicturehelpers.h"

#include "io/zipindex.h"
#include "sheet/headerfooter.h"
#include "sheet/pagesetup.h"
#include "grid/hfpicturecache.h"
#include "sheet/xlsxsave.h"

using jp::office::sheet::HeaderPicture;
using drawtest::pngBytes;

namespace {

bool readFirstSheet(const QByteArray &book, Sheet *sheet, Workbook *wb, Package *package)
{
    QString why;
    return package->readLazy(book, &why) && wb->open(*package, &why) && wb->readSheet(0, sheet);
}

// The workbook saved with the first sheet's page setup as given (everything else as it was), and read again.
bool saveWith(const QByteArray &book, const PageSetup &page, QByteArray *saved, Package *package, Workbook *reopened, Sheet *sheet, SaveReport *report = nullptr)
{
    Package original;
    Workbook wb;
    QString why;
    if (!original.readLazy(book, &why) || !wb.open(original, &why)) return false;
    SaveChanges changes;
    changes.sheets[0].setPage = true;
    changes.sheets[0].page = page;
    if (!saveXlsx(wb, changes, saved, &why, report)) {
        qWarning("%s", qPrintable(why));
        return false;
    }
    return package->readLazy(*saved, &why) && reopened->open(*package, &why) && reopened->readSheet(0, sheet);
}

QByteArray plainBook(const QString &headerFooter)
{
    QString rows;
    for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
    return buildXlsx({worksheet(rows, QStringLiteral("<headerFooter>%1</headerFooter>").arg(headerFooter))});
}

HeaderPicture pictureAt(const QString &place, const QByteArray &image, double width = 50, double height = 25)
{
    HeaderPicture p;
    p.place = place;
    p.image = image;
    p.extension = QStringLiteral("png");
    p.width = width;
    p.height = height;
    p.title = QStringLiteral("made");
    return p;
}

} // namespace

class SheetHfPictureTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A picture in the center of the header and one at the left of the footer: where each is, its size in points, its image, and its name.
    void theShapesOfTheDrawingAreRead()
    {
        const QByteArray red = pngBytes(8, 4, Qt::red), blue = pngBytes(4, 8, Qt::blue);
        HfShape center{QStringLiteral("CH")};
        HfShape footer{QStringLiteral("LF"), QStringLiteral("position:absolute;width:1in;height:0.5in;z-index:2"), QStringLiteral("rId2"), QStringLiteral("seal")};
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader><oddFooter>&amp;L&amp;G</oddFooter>"), {center, footer}, {red, blue});
        Sheet sheet;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &sheet, &wb, &package));
        const HeaderFooter &hf = sheet.page.headerFooter;
        QCOMPARE(hf.pictures.size(), 2);
        QCOMPARE(hf.pictures[0].place, QStringLiteral("CH"));
        QCOMPARE(hf.pictures[0].width, 72.0);
        QCOMPARE(hf.pictures[0].height, 36.0);
        QCOMPARE(hf.pictures[0].image, red);
        QCOMPARE(hf.pictures[0].extension, QStringLiteral("png"));
        QCOMPARE(hf.pictures[0].title, QStringLiteral("logo"));
        QCOMPARE(hf.pictures[1].place, QStringLiteral("LF"));
        QCOMPARE(hf.pictures[1].width, 72.0);    // (an inch)
        QCOMPARE(hf.pictures[1].height, 36.0);   // (half an inch)
        QCOMPARE(hf.pictures[1].image, blue);
        QCOMPARE(hf.drawingRel, QStringLiteral("rIdH1"));
        QVERIFY(hf.pictureFor(QLatin1Char('C'), true, false, false) == &hf.pictures[0]);
        QVERIFY(hf.pictureFor(QLatin1Char('L'), false, false, false) == &hf.pictures[1]);
        QVERIFY(!hf.pictureFor(QLatin1Char('R'), true, false, false));
        QVERIFY(!hf.pictureFor(QLatin1Char('C'), false, false, false));
        // (the first page and the even pages use the picture the other pages do, when they have none of their own)
        QVERIFY(hf.pictureFor(QLatin1Char('C'), true, true, false) == &hf.pictures[0]);
        QVERIFY(hf.pictureFor(QLatin1Char('C'), true, false, true) == &hf.pictures[0]);
    }

    // The forms for the first page and the even pages are picked by the page.
    void theFormsOfThePageHaveTheirOwnPictures()
    {
        const QByteArray a = pngBytes(2, 2, Qt::red), b = pngBytes(3, 3, Qt::green), c = pngBytes(4, 4, Qt::blue);
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH")), hfShape(QStringLiteral("CHF"), QStringLiteral("rId2")), hfShape(QStringLiteral("CHE"), QStringLiteral("rId3"))}, {a, b, c});
        Sheet sheet;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &sheet, &wb, &package));
        const HeaderFooter &hf = sheet.page.headerFooter;
        QCOMPARE(hf.pictureFor(QLatin1Char('C'), true, false, false)->image, a);
        QCOMPARE(hf.pictureFor(QLatin1Char('C'), true, true, false)->image, b);
        QCOMPARE(hf.pictureFor(QLatin1Char('C'), true, false, true)->image, c);
    }

    // A hostile drawing: more shapes than a sheet may hold, one with no image, one whose link is not to an image, one with no size, and an image larger than a picture may be. What is bad is
    // left out; a drawing that is not well formed has none read.
    void hostileDrawingsAreBounded()
    {
        const QByteArray png = pngBytes(2, 2, Qt::red);
        QVector<HfShape> many;
        for (int i = 0; i < 40; ++i) many.append(HfShape{QStringLiteral("S%1").arg(i)});
        Sheet sheet;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), many, {png}), &sheet, &wb, &package));
        QCOMPARE(sheet.page.headerFooter.pictures.size(), jp::office::sheet::kMaxHeaderPictures);
        // (no image named, an image that is not there, no size)
        HfShape noImage{QStringLiteral("CH")}, missing = hfShape(QStringLiteral("LH"), QStringLiteral("rId9")), noSize{QStringLiteral("RH"), QStringLiteral("position:absolute")}, fine{QStringLiteral("CF")};
        noImage.relId.clear();
        Sheet s2;
        Workbook w2;
        Package p2;
        QVERIFY(readFirstSheet(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), {noImage, missing, noSize, fine}, {png}), &s2, &w2, &p2));
        QCOMPARE(s2.page.headerFooter.pictures.size(), 1);
        QCOMPARE(s2.page.headerFooter.pictures[0].place, QStringLiteral("CF"));
        // (a link that is not to an image)
        Sheet s3;
        Workbook w3;
        Package p3;
        QVERIFY(readFirstSheet(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), {HfShape{QStringLiteral("CH")}}, {png}, QStringLiteral("hyperlink")), &s3, &w3, &p3));
        QVERIFY(s3.page.headerFooter.pictures.isEmpty());
        // (an image past the most a picture may have)
        Sheet s4;
        Workbook w4;
        Package p4;
        QVERIFY(readFirstSheet(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), {HfShape{QStringLiteral("CH")}}, {QByteArray(int(jp::office::sheet::kMaxHeaderPictureBytes) + 1, 'x')}), &s4, &w4, &p4));
        QVERIFY(s4.page.headerFooter.pictures.isEmpty());
        // (the sizes are held to what a page can show)
        HfShape huge{QStringLiteral("CH"), QStringLiteral("width:99999pt;height:50000pt")};
        Sheet s5;
        Workbook w5;
        Package p5;
        QVERIFY(readFirstSheet(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), {huge}, {png}), &s5, &w5, &p5));
        QCOMPARE(s5.page.headerFooter.pictures.size(), 1);
        QCOMPARE(s5.page.headerFooter.pictures[0].width, jp::office::sheet::kMaxHeaderPicturePoints);
        QCOMPARE(s5.page.headerFooter.pictures[0].height, jp::office::sheet::kMaxHeaderPicturePoints);
        // (a drawing that is not well formed)
        QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), {HfShape{QStringLiteral("CH")}}, {png});
        Sheet s6;
        Workbook w6;
        Package p6;
        QString why;
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(book, parts));
        parts[QStringLiteral("xl/drawings/vmlDrawing1.vml")] += "<broken";
        jp::ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
        QVERIFY(readFirstSheet(z.finish(), &s6, &w6, &p6));
        QVERIFY(s6.page.headerFooter.pictures.isEmpty());
    }

    // Two sheets that name the same image share its bytes (the workbook reads it once).
    void anImageSharedBySheetsIsReadOnce()
    {
        const QByteArray png = pngBytes(2, 2, Qt::red);
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;G</oddHeader>"), {HfShape{QStringLiteral("CH")}}, {png});
        Sheet a, b;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &a, &wb, &package));
        QVERIFY(wb.readSheet(0, &b));
        QCOMPARE(a.page.headerFooter.pictures.size(), 1);
        QCOMPARE(b.page.headerFooter.pictures.size(), 1);
        QCOMPARE(static_cast<const void *>(a.page.headerFooter.pictures[0].image.constData()), static_cast<const void *>(b.page.headerFooter.pictures[0].image.constData()));
    }

    // A picture put in a sheet that had none: the file gets a drawing, its image, the links between them and the sheet, their types, and the element in the sheet; read again it is the same.
    void aPictureAddedIsWrittenWhole()
    {
        const QByteArray book = plainBook(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"));
        Sheet before;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &before, &wb, &package));
        QVERIFY(before.page.headerFooter.pictures.isEmpty());
        PageSetup page = before.page;
        const QByteArray red = pngBytes(8, 4, Qt::red);
        page.headerFooter.pictures.append(pictureAt(QStringLiteral("CH"), red, 60, 30));
        QByteArray saved;
        Package again;
        Workbook reopened;
        Sheet sheet;
        SaveReport report;
        QVERIFY(saveWith(book, page, &saved, &again, &reopened, &sheet, &report));
        const HeaderFooter &hf = sheet.page.headerFooter;
        QCOMPARE(hf.pictures.size(), 1);
        QCOMPARE(hf.pictures[0].place, QStringLiteral("CH"));
        QCOMPARE(hf.pictures[0].image, red);
        QCOMPARE(hf.pictures[0].width, 60.0);
        QCOMPARE(hf.pictures[0].height, 30.0);
        QCOMPARE(hf.pictures[0].title, QStringLiteral("made"));
        QVERIFY(!hf.drawingRel.isEmpty());
        QCOMPARE(report.headerDrawings.at(0), hf.drawingRel);
        // the links and types a program that reads the file needs
        const auto vml = again.relationshipsOfType(QStringLiteral("/xl/worksheets/sheet1.xml"), QStringLiteral("/vmlDrawing"));
        QCOMPARE(vml.size(), 1);
        QCOMPARE(vml[0].id, hf.drawingRel);
        QVERIFY(again.hasPart(vml[0].target));
        QCOMPARE(again.contentType(vml[0].target), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"));
        const auto images = again.relationshipsOfType(vml[0].target, QStringLiteral("/image"));
        QCOMPARE(images.size(), 1);
        QCOMPARE(again.part(images[0].target), red);
        QCOMPARE(again.contentType(images[0].target), QStringLiteral("image/png"));
        QVERIFY(again.part(QStringLiteral("/xl/worksheets/sheet1.xml")).contains("<legacyDrawingHF"));
        // and the headerFooter element is where it was, before the new element (the format's order)
        const QByteArray xml = again.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(xml.indexOf("<headerFooter") < xml.indexOf("<legacyDrawingHF"));
    }

    // A picture changed, one added, and one taken out of a sheet that has some: the drawing is written again under its own name, kept links keep their ids, and the sheet's other parts are as they were.
    void picturesChangedAndTakenOutAreWrittenAgain()
    {
        const QByteArray red = pngBytes(8, 4, Qt::red), blue = pngBytes(4, 8, Qt::blue), green = pngBytes(6, 6, Qt::green);
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader><oddFooter>&amp;L&amp;G</oddFooter>"),
                                                        {hfShape(QStringLiteral("CH")), hfShape(QStringLiteral("LF"), QStringLiteral("rId2"))}, {red, blue});
        Sheet before;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &before, &wb, &package));
        PageSetup page = before.page;
        QCOMPARE(page.headerFooter.pictures.size(), 2);
        page.headerFooter.pictures[0].image = green;   // (changed)
        page.headerFooter.pictures.removeAt(1);        // (taken out)
        page.headerFooter.pictures.append(pictureAt(QStringLiteral("RH"), blue));   // (added)
        QByteArray saved;
        Package again;
        Workbook reopened;
        Sheet sheet;
        QVERIFY(saveWith(book, page, &saved, &again, &reopened, &sheet));
        const HeaderFooter &hf = sheet.page.headerFooter;
        QCOMPARE(hf.pictures.size(), 2);
        QCOMPARE(hf.pictures[0].place, QStringLiteral("CH"));
        QCOMPARE(hf.pictures[0].image, green);
        QCOMPARE(hf.pictures[1].place, QStringLiteral("RH"));
        QCOMPARE(hf.pictures[1].image, blue);
        QCOMPARE(hf.drawingRel, QStringLiteral("rIdH1"));   // (the sheet's link to the drawing is the one it had)
        QCOMPARE(again.relationshipsOfType(QStringLiteral("/xl/worksheets/sheet1.xml"), QStringLiteral("/vmlDrawing")).size(), 1);
        QVERIFY(again.hasPart(QStringLiteral("/xl/drawings/vmlDrawing1.vml")));
    }

    // All the pictures taken out: the sheet loses the element and its link, and the drawing and the images only it used are gone from the file.
    void takingAllPicturesOutLeavesNothingOfThem()
    {
        const QByteArray red = pngBytes(8, 4, Qt::red);
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red});
        Sheet before;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &before, &wb, &package));
        PageSetup page = before.page;
        page.headerFooter.pictures.clear();
        QByteArray saved;
        Package again;
        Workbook reopened;
        Sheet sheet;
        SaveReport report;
        QVERIFY(saveWith(book, page, &saved, &again, &reopened, &sheet, &report));
        QVERIFY(sheet.page.headerFooter.pictures.isEmpty());
        QVERIFY(sheet.page.headerFooter.drawingRel.isEmpty());
        QCOMPARE(report.headerDrawings.at(0), QString());
        QVERIFY(!again.part(QStringLiteral("/xl/worksheets/sheet1.xml")).contains("legacyDrawingHF"));
        QVERIFY(!again.hasPart(QStringLiteral("/xl/drawings/vmlDrawing1.vml")));
        QVERIFY(!again.hasPart(QStringLiteral("/xl/media/hfimage1.png")));
        QCOMPARE(again.relationshipsOfType(QStringLiteral("/xl/worksheets/sheet1.xml"), QStringLiteral("/vmlDrawing")).size(), 0);
    }

    // A save that does not change the pictures leaves the drawing, the image, and their links as the same bytes.
    void unchangedPicturesAreNotTouched()
    {
        const QByteArray red = pngBytes(8, 4, Qt::red);
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red});
        Sheet before;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &before, &wb, &package));
        PageSetup page = before.page;
        page.orientation = PageOrientation::Landscape;
        QByteArray saved;
        Package again;
        Workbook reopened;
        Sheet sheet;
        QVERIFY(saveWith(book, page, &saved, &again, &reopened, &sheet));
        QCOMPARE(again.part(QStringLiteral("/xl/drawings/vmlDrawing1.vml")), package.part(QStringLiteral("/xl/drawings/vmlDrawing1.vml")));
        QCOMPARE(again.part(QStringLiteral("/xl/media/hfimage1.png")), red);
        QCOMPARE(sheet.page.headerFooter.pictures.size(), 1);
        QVERIFY(sheet.page.orientation == PageOrientation::Landscape);
    }

    // The ids of the forms of a page: the short forms (an F or an E after the two letters) and the long ones (FIRST and EVEN) are read alike, as the long ones, in any case; a text with no
    // code for a picture drops the picture of either form.
    void theFormsOfThePageAreReadInBothSpellings()
    {
        const QByteArray a = pngBytes(2, 2, Qt::red), b = pngBytes(3, 3, Qt::green), c = pngBytes(4, 4, Qt::blue), d = pngBytes(5, 5, Qt::yellow);
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"),
                                                       {hfShape(QStringLiteral("CH")), hfShape(QStringLiteral("CHFIRST"), QStringLiteral("rId2")), hfShape(QStringLiteral("chE"), QStringLiteral("rId3")), hfShape(QStringLiteral("LHf"), QStringLiteral("rId4"))},
                                                       {a, b, c, d});
        Sheet sheet;
        Workbook wb;
        Package package;
        QVERIFY(readFirstSheet(book, &sheet, &wb, &package));
        QCOMPARE(sheet.page.headerFooter.pictures.size(), 4);
        QCOMPARE(sheet.page.headerFooter.pictures[1].place, QStringLiteral("CHFIRST"));
        QCOMPARE(sheet.page.headerFooter.pictures[3].place, QStringLiteral("LHFIRST"));   // (the short form is read as the long one)
        QCOMPARE(canonicalHeaderPlace(QStringLiteral("rhE")), QStringLiteral("RHEVEN"));
        QCOMPARE(canonicalHeaderPlace(QStringLiteral("RHEVEN")), QStringLiteral("RHEVEN"));
        QCOMPARE(canonicalHeaderPlace(QStringLiteral("CF")), QStringLiteral("CF"));
        QCOMPARE(canonicalHeaderPlace(QStringLiteral("CFF")), QStringLiteral("CFFIRST"));
        HeaderFooter hf = sheet.page.headerFooter;
        QCOMPARE(hf.pictureFor(QLatin1Char('C'), true, true, false)->image, b);
        // a text that names the picture of one form only keeps that one
        hf.oddHeader = QStringLiteral("&C&G");
        hf.firstHeader = QStringLiteral("&Cnothing");
        hf.evenHeader = QStringLiteral("&L&G");
        dropUnusedHeaderPictures(&hf);
        QCOMPARE(hf.pictures.size(), 1);
        QCOMPARE(hf.pictures[0].place, QStringLiteral("CH"));
        hf = sheet.page.headerFooter;
        hf.oddHeader = QStringLiteral("&C&G&LL&G");
        hf.firstHeader = QStringLiteral("&C&G&L&G");
        hf.evenHeader = QStringLiteral("&C&G");
        dropUnusedHeaderPictures(&hf);
        QCOMPARE(hf.pictures.size(), 4);
    }

    // A drawing two sheets share is read once for the workbook (and a save that looks again does not read it again); a drawing of many elements is read only as far as the most elements.
    void aDrawingSharedBySheetsIsReadOnce()
    {
        const QByteArray png = pngBytes(2, 2, Qt::red);
        const QByteArray book = bookWithSharedDrawing({hfShape(QStringLiteral("CH"))}, {png});
        Package package;
        Workbook wb;
        QString why;
        QVERIFY2(package.readLazy(book, &why) && wb.open(package, &why), qPrintable(why));
        Sheet a, b, again;
        QVERIFY(wb.readSheet(0, &a));
        QVERIFY(wb.readSheet(1, &b));
        QVERIFY(wb.readSheet(0, &again));
        QCOMPARE(a.page.headerFooter.pictures.size(), 1);
        QCOMPARE(b.page.headerFooter.pictures.size(), 1);
        QCOMPARE(again.page.headerFooter.pictures.size(), 1);
        QCOMPARE(wb.headerDrawingsRead(), 1);
    }

    // Elements of any kind are counted: in a drawing of thousands of them the shape after them is not reached.
    void aDrawingOfManyElementsIsReadOnlyAsFar()
    {
        const QByteArray png = pngBytes(2, 2, Qt::red);
        QString junk;
        for (int i = 0; i < 6000; ++i) junk += QStringLiteral("<o:x/>");
        QByteArray hostile = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {png});
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(hostile, parts));
        QByteArray vml = parts[QStringLiteral("xl/drawings/vmlDrawing1.vml")];
        vml.replace("<v:shape ", junk.toUtf8() + "<v:shape ");
        parts[QStringLiteral("xl/drawings/vmlDrawing1.vml")] = vml;
        jp::ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
        Sheet s;
        Workbook w2;
        Package p2;
        QVERIFY(readFirstSheet(z.finish(), &s, &w2, &p2));
        QVERIFY(s.page.headerFooter.pictures.isEmpty());
    }

    // A change of the pictures of one sheet that shares its drawing with another leaves the other's drawing, its link, and its pictures as they were: the changed sheet is given a drawing of its own.
    void aSharedDrawingIsNotChangedForOneSheet()
    {
        const QByteArray red = pngBytes(8, 4, Qt::red), blue = pngBytes(4, 8, Qt::blue);
        const QByteArray book = bookWithSharedDrawing({hfShape(QStringLiteral("CH"))}, {red});
        Package package;
        Workbook wb;
        QString why;
        QVERIFY2(package.readLazy(book, &why) && wb.open(package, &why), qPrintable(why));
        Sheet first;
        QVERIFY(wb.readSheet(0, &first));
        SaveChanges changes;
        changes.sheets[0].setPage = true;
        changes.sheets[0].page = first.page;
        changes.sheets[0].page.headerFooter.pictures[0].image = blue;
        QByteArray saved;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, changes, &saved, &why, &report), qPrintable(why));
        Package again;
        Workbook w2;
        QVERIFY2(again.readLazy(saved, &why) && w2.open(again, &why), qPrintable(why));
        Sheet a, b;
        QVERIFY(w2.readSheet(0, &a));
        QVERIFY(w2.readSheet(1, &b));
        QCOMPARE(a.page.headerFooter.pictures.size(), 1);
        QCOMPARE(a.page.headerFooter.pictures[0].image, blue);
        QCOMPARE(b.page.headerFooter.pictures.size(), 1);
        QCOMPARE(b.page.headerFooter.pictures[0].image, red);
        const auto linkA = again.relationshipsOfType(w2.info().sheets[0].part, QStringLiteral("/vmlDrawing")), linkB = again.relationshipsOfType(w2.info().sheets[1].part, QStringLiteral("/vmlDrawing"));
        QCOMPARE(linkA.size(), 1);
        QCOMPARE(linkB.size(), 1);
        QVERIFY(linkA[0].target != linkB[0].target);
        QCOMPARE(linkB[0].target, QStringLiteral("/xl/drawings/vmlDrawing1.vml"));
        QCOMPARE(again.part(linkB[0].target), package.part(QStringLiteral("/xl/drawings/vmlDrawing1.vml")));   // (the other sheet's drawing: the same bytes)
        QCOMPARE(again.relationshipsOfType(linkB[0].target, QStringLiteral("/image")).size(), 1);
        // and taking the pictures out of one leaves the other's
        SaveChanges none;
        none.sheets[1].setPage = true;
        none.sheets[1].page = b.page;
        none.sheets[1].page.headerFooter.pictures.clear();
        QByteArray saved2;
        QVERIFY2(saveXlsx(wb, none, &saved2, &why), qPrintable(why));
        Package third;
        Workbook w3;
        QVERIFY2(third.readLazy(saved2, &why) && w3.open(third, &why), qPrintable(why));
        Sheet c, d;
        QVERIFY(w3.readSheet(0, &c));
        QVERIFY(w3.readSheet(1, &d));
        QCOMPARE(c.page.headerFooter.pictures.size(), 1);
        QVERIFY(d.page.headerFooter.pictures.isEmpty());
        QVERIFY(third.hasPart(QStringLiteral("/xl/drawings/vmlDrawing1.vml")));
    }

    // The decoded pictures are kept within a budget of bytes for the whole process, the one used last let go of first; a picture asked for with no waiting is made on a worker and the asker is told.
    void decodedPicturesAreKeptWithinABudgetOfBytes()
    {
        using jp::grid::HeaderPictureCache;
        HeaderPictureCache &cache = HeaderPictureCache::instance();
        cache.clear();
        const qint64 was = cache.budgetBytes();
        QVector<QByteArray> pictures;
        for (int i = 0; i < 6; ++i) pictures.append(pngBytes(100 + i, 100, QColor(10 * i, 0, 0)));   // (about 40 KB each decoded)
        cache.setBudgetBytes(100000);
        const int hereBefore = cache.decodesHere();
        for (const QByteArray &p : pictures) QVERIFY(!cache.get(p, true, nullptr).isNull());
        QCOMPARE(cache.decodesHere() - hereBefore, 6);
        QVERIFY(cache.bytes() <= 100000 + 100 * 106 * 4);   // (within the budget, but for the picture just made)
        QVERIFY(cache.count() < 6);
        // the last is kept; the first is made again when asked for
        const int mid = cache.decodesHere();
        QVERIFY(!cache.get(pictures.last(), true, nullptr).isNull());
        QCOMPARE(cache.decodesHere(), mid);
        QVERIFY(!cache.get(pictures.first(), true, nullptr).isNull());
        QCOMPARE(cache.decodesHere(), mid + 1);
        // no waiting: nothing comes back, and the picture is made on a worker, not here
        cache.clear();
        const int here = cache.decodesHere();
        bool pending = false;
        QVERIFY(cache.get(pictures[2], false, &pending).isNull());
        QVERIFY(pending);
        cache.waitForWorkers();
        QCOMPARE(cache.decodesHere(), here);
        QCOMPARE(cache.pendingCount(), 0);
        QVERIFY(!cache.get(pictures[2], false, &pending).isNull());
        QVERIFY(!pending);
        cache.setBudgetBytes(was);
        cache.clear();
    }
};

JP_TEST_CLASS(SheetHfPictureTests)
#include "tst_sheet_hfpictures.moc"
