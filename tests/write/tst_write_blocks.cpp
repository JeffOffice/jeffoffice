// Tests of JeffWrite's saved blocks (core/blockops.h): a selection kept as a small document and put in again, with its styles, pictures, and tables; where it goes in; one step of Undo;
// Text Shortcuts; a block or a file that is hostile; and the glossary part of a file kept as it was.

#include "tst_write.h"
#include "core/blockops.h"
#include "core/pictureops.h"
#include "io/blocklibrary.h"
#include "io/zip.h"
#include "app/blockgallery.h"
#include "app/pagepaint.h"
#include "app/theme.h"

#include <QBuffer>
#include <QImage>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextFrame>
#include <QTextTable>

namespace {

QByteArray pngBytes()
{
    QImage img(30, 20, QImage::Format_RGB32);
    img.fill(Qt::darkGreen);
    QByteArray out;
    QBuffer b(&out);
    b.open(QIODevice::WriteOnly);
    img.save(&b, "PNG");
    return out;
}

QString allText(WriteDoc &doc) { return doc.story()->toPlainText(); }

int paragraphsOf(WriteDoc &doc) { return doc.story()->blockCount(); }

QTextCursor selecting(WriteDoc &doc, int from, int to)
{
    QTextCursor c(doc.story());
    c.setPosition(from);
    c.setPosition(to, QTextCursor::KeepAnchor);
    return c;
}

int positionAfter(WriteDoc &doc, const QString &text)
{
    const int at = allText(doc).indexOf(text);
    return at < 0 ? -1 : at + int(text.size());
}

// The tables the file it would save holds.
int tablesIn(WriteDoc &doc)
{
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(doc.toDocx(), parts)) return -1;
    return int(QString::fromUtf8(parts.value(QStringLiteral("word/document.xml"))).count(QRegularExpression(QStringLiteral("<w:tbl[ >/]"))));
}

int pictures(WriteDoc &doc) { return int(allText(doc).count(QChar::ObjectReplacementCharacter)); }

const QByteArray kPlainStyles =
    "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Tinos\" w:hAnsi=\"Tinos\"/><w:sz w:val=\"23\"/></w:rPr></w:rPrDefault></w:docDefaults>"
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>";

const QByteArray kOtherLegal =
    "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"Legal\"><w:name w:val=\"Legal Note\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/><w:rPr><w:sz w:val=\"20\"/></w:rPr></w:style>";

const QByteArray kLegal =
    "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"Legal\"><w:name w:val=\"Legal Note\"/><w:basedOn w:val=\"Normal\"/><w:qFormat/><w:rPr><w:i/><w:sz w:val=\"18\"/></w:rPr></w:style>";

// A source document: a heading, a paragraph with a bold word, and a last line.
std::unique_ptr<WriteDoc> sourceDoc()
{
    const QByteArray body =
        "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Terms</w:t></w:r></w:p>"
        "<w:p><w:pPr><w:pStyle w:val=\"Legal\"/></w:pPr><w:r><w:t xml:space=\"preserve\">Payment is due in </w:t></w:r><w:r><w:rPr><w:b/></w:rPr><w:t>thirty</w:t></w:r><w:r><w:t xml:space=\"preserve\"> days.</w:t></w:r></w:p>"
        "<w:p><w:r><w:t>Last line</w:t></w:r></w:p>";
    return open(makeDocx(body, kStyles + kLegal));
}

QByteArray savedFromSource(QString *problem = nullptr)
{
    auto src = sourceDoc();
    return jp::write::blockFromSelection(*src, selecting(*src, 0, positionAfter(*src, QStringLiteral("Last line"))), problem);
}

QByteArray zipWith(const QByteArray &zip, const QMap<QString, QByteArray> &add, const QMap<QString, QPair<QByteArray, QByteArray>> &edit = {})
{
    QMap<QString, QByteArray> parts;
    jp::readZip(zip, parts);
    jp::ZipWriter z;
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        QByteArray data = it.value();
        if (edit.contains(it.key())) data.replace(edit[it.key()].first, edit[it.key()].second);
        z.add(it.key(), data, true);
    }
    for (auto it = add.cbegin(); it != add.cend(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

} // namespace

void WriteTests::aSelectionIsKeptWithItsFormatAndStylesAndPutInAgainAsOneStep()
{
    QString problem;
    const QByteArray block = savedFromSource(&problem);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    // (The block is a document of its own: it reads as one.)
    auto read = open(block);
    QVERIFY(read);
    QVERIFY(allText(*read).startsWith(QStringLiteral("Terms\nPayment is due in thirty days.")));
    QVERIFY(allText(*read).contains(QStringLiteral("Last line")));
    QCOMPARE(read->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));

    auto target = open(makeDocx("<w:p><w:r><w:t>Hello</w:t></w:r></w:p><w:p><w:r><w:t>Bye</w:t></w:r></w:p>", kPlainStyles));
    QVERIFY(!target->styleSheet().find(QStringLiteral("Legal")));
    const QString before = allText(*target);
    QTextCursor c(target->story());
    c.setPosition(positionAfter(*target, QStringLiteral("Hello")) + 1);   // the start of "Bye"
    QVERIFY2(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
    QVERIFY(allText(*target).contains(QStringLiteral("Terms\nPayment is due in thirty days.")));
    QVERIFY(target->styleSheet().find(QStringLiteral("Legal")));
    // (The first and last paragraphs of a block take the look of the paragraph they join, as in any paste; the ones between keep theirs.)
    QCOMPARE(target->story()->findBlock(allText(*target).indexOf(QStringLiteral("Payment"))).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Legal"));
    // The bold word is still bold.
    const int bold = allText(*target).indexOf(QStringLiteral("thirty"));
    QTextCursor probe(target->story());
    probe.setPosition(bold + 2);
    QCOMPARE(probe.charFormat().fontWeight(), int(QFont::Bold));
    // It saves and reads back.
    auto back = open(target->toDocx());
    QVERIFY(back);
    QVERIFY(allText(*back).contains(QStringLiteral("Payment is due in thirty days.")));
    QVERIFY(back->styleSheet().find(QStringLiteral("Legal")));
    // One Undo takes it all out.
    QVERIFY(target->undo());
    QCOMPARE(allText(*target), before);
}

void WriteTests::aStyleTheDocumentHasIsNotChanged()
{
    const QByteArray block = savedFromSource();
    auto target = open(makeDocx("<w:p><w:r><w:t>Here</w:t></w:r></w:p>", kPlainStyles + kOtherLegal));
    const auto *mine = target->styleSheet().find(QStringLiteral("Legal"));
    QVERIFY(mine);
    const int size = mine->rPr.size.value_or(0);
    QCOMPARE(size, 20);
    QTextCursor c(target->story());
    c.setPosition(positionAfter(*target, QStringLiteral("Here")));
    QVERIFY(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph));
    QCOMPARE(target->styleSheet().find(QStringLiteral("Legal"))->rPr.size.value_or(0), size);
    QCOMPARE(target->story()->findBlock(allText(*target).indexOf(QStringLiteral("Payment"))).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Legal"));
}

void WriteTests::aBlockGoesInAsItIsInAParagraphOfItsOwnOrOnAPageOfItsOwn()
{
    auto src = open(makeDocx("<w:p><w:r><w:t>Signed</w:t></w:r></w:p>", kPlainStyles));
    const QByteArray block = jp::write::blockFromSelection(*src, selecting(*src, 0, 6));
    QVERIFY(!block.isEmpty());
    auto text = [&](jp::BlockInsert how) {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Hello World</w:t></w:r></w:p>", kPlainStyles));
        QTextCursor c(doc->story());
        c.setPosition(6);
        [&] { QVERIFY(jp::write::insertBlock(*doc, c, block, how)); }();
        return doc;
    };
    auto inline_ = text(jp::BlockInsert::Content);
    QCOMPARE(paragraphsOf(*inline_), 1);
    QCOMPARE(allText(*inline_), QStringLiteral("Hello SignedWorld"));
    auto own = text(jp::BlockInsert::Paragraph);
    QCOMPARE(allText(*own), QStringLiteral("Hello \nSigned\nWorld"));
    auto page = text(jp::BlockInsert::Page);
    QVERIFY(allText(*page).contains(QStringLiteral("Signed")));
    QCOMPARE(page->pager().pageCount(), 3);
    QCOMPARE(inline_->pager().pageCount(), 1);
    QCOMPARE(own->pager().pageCount(), 1);
    // (In a paragraph of its own at the start of a paragraph: nothing is split.)
    auto start = open(makeDocx("<w:p><w:r><w:t>Hello</w:t></w:r></w:p>", kPlainStyles));
    QTextCursor c(start->story());
    QVERIFY(jp::write::insertBlock(*start, c, block, jp::BlockInsert::Paragraph));
    QCOMPARE(allText(*start), QStringLiteral("Signed\nHello"));
}

void WriteTests::aPictureAndTheWordsOfATableComeAlong()
{
    const QByteArray body =
        "<w:p><w:r><w:t>Logo </w:t></w:r></w:p>"
        "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
        "<w:tr><w:tc><w:p><w:r><w:t>Cell A</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>Cell B</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>After</w:t></w:r></w:p>";
    auto src = open(makeDocx(body, kPlainStyles));
    QTextCursor at(src->story());
    at.setPosition(4);
    QCOMPARE(jp::write::insertPicture(*src, at, pngBytes()), jp::write::PictureResult::Ok);
    QCOMPARE(pictures(*src), 1);
    QCOMPARE(tablesIn(*src), 1);   // (a table copied to another document arrives as its words, as in any paste between documents: the cells are kept, the grid is not)
    QTextCursor all(src->story());
    all.select(QTextCursor::Document);
    QString problem;
    const QByteArray block = jp::write::blockFromSelection(*src, all, &problem);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    auto made = open(block);
    QVERIFY(made);
    QCOMPARE(pictures(*made), 1);
    auto target = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p>", kPlainStyles));
    QTextCursor c(target->story());
    c.setPosition(3);
    QVERIFY2(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
    QCOMPARE(pictures(*target), 1);
    QVERIFY(allText(*target).contains(QStringLiteral("Cell B")));
    // The picture is in the file the target saves: a part of its own, with the bytes.
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(target->toDocx(), parts));
    int media = 0;
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (it.key().startsWith(QStringLiteral("word/media/")) && it.value() == pngBytes()) ++media;
    QCOMPARE(media, 1);
}

void WriteTests::aFieldInABlockShowsTheDocumentsOwnValueOnceItIsIn()
{
    auto src = open(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">Pages: </w:t></w:r><w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>9</w:t></w:r></w:fldSimple></w:p>", kPlainStyles));
    QTextCursor all(src->story());
    all.select(QTextCursor::Document);
    const QByteArray block = jp::write::blockFromSelection(*src, all);
    QVERIFY(!block.isEmpty());
    auto target = open(makeDocx("<w:p><w:r><w:t>Start</w:t></w:r></w:p>", kPlainStyles));
    QTextCursor c(target->story());
    c.setPosition(5);
    QVERIFY(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph));
    const QString text = allText(*target);
    QVERIFY(text.contains(QStringLiteral("Pages: ")));
    QVERIFY(!text.contains(QStringLiteral("Pages: 9")));
}

void WriteTests::aTextShortcutBecomesItsBlockWhereTheNameWasTyped()
{
    QTemporaryDir dir;
    jp::BlockLibrary lib(dir.filePath(QStringLiteral("l.jblocks")));
    auto src = open(makeDocx("<w:p><w:r><w:t>Kind regards, Jeff</w:t></w:r></w:p>", kPlainStyles));
    const QByteArray sig = jp::write::blockFromSelection(*src, selecting(*src, 0, 18));
    jp::BlockEntry e;
    e.info.name = QStringLiteral("sig");
    e.info.gallery = jp::kShortcutsGallery;
    e.info.format = QStringLiteral("docx");
    e.data = sig;
    QString err;
    QVERIFY2(lib.add(e, nullptr, &err), qPrintable(err));
    e.info.name = QStringLiteral("my sig");
    e.data = jp::write::blockFromSelection(*src, selecting(*src, 0, 4));
    QVERIFY2(lib.add(e, nullptr, &err), qPrintable(err));
    QVERIFY(lib.load());

    auto doc = open(makeDocx("<w:p><w:r><w:t>Thanks. SIG</w:t></w:r></w:p><w:p><w:r><w:t>xsig</w:t></w:r></w:p><w:p><w:r><w:t>See my sig</w:t></w:r></w:p>", kPlainStyles));
    const QString before = allText(*doc);
    QTextCursor c(doc->story());
    c.setPosition(positionAfter(*doc, QStringLiteral("Thanks. SIG")));
    QVERIFY(jp::write::expandShortcut(*doc, c, lib));
    QVERIFY(allText(*doc).startsWith(QStringLiteral("Thanks. Kind regards, Jeff\n")));
    QVERIFY(!allText(*doc).contains(QStringLiteral("SIG")));
    QCOMPARE(c.position(), positionAfter(*doc, QStringLiteral("Thanks. Kind regards, Jeff")));
    QVERIFY(doc->undo());
    QCOMPARE(allText(*doc), before);
    // Inside a word it is not a shortcut.
    QTextCursor d(doc->story());
    d.setPosition(positionAfter(*doc, QStringLiteral("xsig")));
    QVERIFY(!jp::write::expandShortcut(*doc, d, lib));
    QCOMPARE(allText(*doc), before);
    // The longest name that fits wins.
    QTextCursor f(doc->story());
    f.setPosition(positionAfter(*doc, QStringLiteral("See my sig")));
    QVERIFY(jp::write::expandShortcut(*doc, f, lib));
    QVERIFY(allText(*doc).contains(QStringLiteral("See Kind")));
    QVERIFY(!allText(*doc).contains(QStringLiteral("my sig")));
}

void WriteTests::aBlockThatIsNoDocumentIsRefusedAndNothingChanges()
{
    auto doc = open(makeDocx("<w:p><w:r><w:t>Safe</w:t></w:r></w:p>", kPlainStyles));
    const QString before = allText(*doc);
    QTextCursor c(doc->story());
    c.setPosition(4);
    QString problem;
    QVERIFY(!jp::write::insertBlock(*doc, c, QByteArray("not a zip"), jp::BlockInsert::Content, &problem));
    QVERIFY(!problem.isEmpty());
    QVERIFY(!jp::write::insertBlock(*doc, c, QByteArray(), jp::BlockInsert::Content));
    QVERIFY(!jp::write::insertBlock(*doc, c, QByteArray(int(jp::BlockLibrary::kMostEntryBytes) + 1, 'x'), jp::BlockInsert::Content));
    jp::ZipWriter z;
    z.add(QStringLiteral("junk.txt"), "junk", true);
    QVERIFY(!jp::write::insertBlock(*doc, c, z.finish(), jp::BlockInsert::Content));
    QCOMPARE(allText(*doc), before);
    QVERIFY(!doc->undo() || allText(*doc) == before);
    // No selection: nothing to save.
    QString why;
    QVERIFY(jp::write::blockFromSelection(*doc, QTextCursor(doc->story()), &why).isEmpty());
    QVERIFY(!why.isEmpty());
}

// A glossary part of a file (the building blocks another program saved in it) is carried through a save as it was.
void WriteTests::aGlossaryPartOfAFileIsKeptByteForByte()
{
    const QByteArray glossary =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<w:glossaryDocument xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:docParts>"
        "<w:docPart><w:docPartPr><w:name w:val=\"Closing\"/><w:category><w:name w:val=\"General\"/><w:gallery w:val=\"placeholder\"/></w:category><w:behaviors><w:behavior w:val=\"content\"/></w:behaviors></w:docPartPr>"
        "<w:docPartBody><w:p><w:r><w:t>Yours truly</w:t></w:r></w:p></w:docPartBody></w:docPart></w:docParts></w:glossaryDocument>";
    const QByteArray plain = makeDocx("<w:p><w:r><w:t>Body</w:t></w:r></w:p>", kPlainStyles);
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(plain, parts));
    QVERIFY(parts.contains(QStringLiteral("[Content_Types].xml")));
    const QString gloss = QStringLiteral("word/glossary/document.xml");
    QMap<QString, QPair<QByteArray, QByteArray>> edit;
    edit.insert(QStringLiteral("[Content_Types].xml"),
                {QByteArray("</Types>"), QByteArray("<Override PartName=\"/word/glossary/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.glossary+xml\"/></Types>")});
    edit.insert(QStringLiteral("word/_rels/document.xml.rels"),
                {QByteArray("</Relationships>"), QByteArray("<Relationship Id=\"rIdGl1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/glossaryDocument\" Target=\"glossary/document.xml\"/></Relationships>")});
    const QByteArray withGloss = zipWith(plain, {{gloss, glossary}}, edit);
    auto doc = open(withGloss);
    QVERIFY(doc);
    QMap<QString, QByteArray> saved;
    QVERIFY(jp::readZip(doc->toDocx(), saved));
    QVERIFY2(saved.contains(gloss), "the glossary part is gone after a save");
    QCOMPARE(saved.value(gloss), glossary);
    // And after a block is put in and the document saved again.
    QTextCursor c(doc->story());
    c.setPosition(4);
    QVERIFY(jp::write::insertBlock(*doc, c, savedFromSource(), jp::BlockInsert::Paragraph));
    saved.clear();
    QVERIFY(jp::readZip(doc->toDocx(), saved));
    QCOMPARE(saved.value(gloss), glossary);
}

// The gallery draws a block when its cell comes into view, never all of a large library at once; a pick goes in at the caret as one step.
void WriteTests::theGalleryDrawsOnlyWhatIsInViewAndAPickGoesInAsOneStep()
{
    QTemporaryDir dir;
    jp::BlockLibrary::setSharedPath(dir.filePath(QStringLiteral("shared.jblocks")));
    const QByteArray block = savedFromSource();
    QVERIFY(!block.isEmpty());
    for (int i = 0; i < 40; ++i) {
        jp::BlockEntry e;
        e.info.name = QStringLiteral("Block %1").arg(i, 2, 10, QLatin1Char('0'));
        e.info.gallery = jp::kBlocksGallery;
        e.info.format = QStringLiteral("docx");
        e.data = block;
        QString err;
        QVERIFY2(jp::BlockLibrary::shared().add(e, nullptr, &err), qPrintable(err));
    }
    int drawn = 0;
    QString picked;
    jp::BlockInfo info;
    QByteArray bytes;
    QMenu *menu = jp::blocksMenu(nullptr, jp::kBlocksGallery, QStringLiteral("docx"),
                                 [&](const jp::BlockEntry &, const QSize &size, qreal) {
                                     ++drawn;
                                     QImage img(size, QImage::Format_RGB32);
                                     img.fill(Qt::white);
                                     return img;
                                 },
                                 [&](const jp::BlockInfo &i, const QByteArray &d) {
                                     info = i;
                                     bytes = d;
                                 },
                                 {});
    auto *gal = menu->findChild<jp::BlockGalleryList *>();
    QVERIFY(gal);
    gal->reload();
    QCOMPARE(gal->list()->count(), 40);
    gal->show();
    gal->grab();
    QVERIFY(drawn >= 1);
    QVERIFY(drawn < 40);
    QCOMPARE(gal->previewsMade(), drawn);
    // Search by name narrows the list.
    gal->search()->setText(QStringLiteral("block 07"));
    QCOMPARE(gal->shownIds().size(), 1);
    gal->search()->setText(QStringLiteral("nothing like this"));
    QCOMPARE(gal->shownIds().size(), 0);
    gal->search()->clear();
    QCOMPARE(gal->shownIds().size(), 40);
    // A pick is handed on with the block's bytes.
    Q_EMIT gal->picked(gal->shownIds().first());
    QCOMPARE(bytes, block);
    delete menu;

    WriteWindow win;
    win.resize(900, 700);
    win.show();
    win.setDocument(open(makeDocx("<w:p><w:r><w:t>Start</w:t></w:r></w:p>", kPlainStyles)), QString());
    win.view()->paginateAll();
    QTextCursor c(win.document()->story());
    c.setPosition(5);
    win.view()->setCursor(c);
    const QString before = allText(*win.document());
    QVERIFY(win.insertSavedBlock(info, bytes));
    QVERIFY(allText(*win.document()).contains(QStringLiteral("Payment is due in thirty days.")));
    QVERIFY(win.document()->undo());
    QCOMPARE(allText(*win.document()), before);
    jp::BlockLibrary::setSharedPath(QString());
}

// Pictures of the saved blocks' windows for the person who reviews them (only when JW_SHOT_DIR names a folder).
void WriteTests::thePicturesOfTheSavedBlocksWindows()
{
    const QString shots = qEnvironmentVariable("JW_SHOT_DIR");
    if (shots.isEmpty()) QSKIP("set JW_SHOT_DIR to take the pictures");
    jp::applyUiTheme(1);
    QTemporaryDir dir;
    jp::BlockLibrary::setSharedPath(dir.filePath(QStringLiteral("shot.jblocks")));
    const QStringList names{QStringLiteral("Payment terms"), QStringLiteral("Signature"), QStringLiteral("Confidential notice"), QStringLiteral("Letterhead"), QStringLiteral("Closing line")};
    const QStringList texts{QStringLiteral("Payment is due in thirty days."), QStringLiteral("Kind regards,\nJeff"), QStringLiteral("CONFIDENTIAL: for the named reader only."), QStringLiteral("JeffOffice LLC\n12 Main Street"), QStringLiteral("Thank you for your business.")};
    for (int i = 0; i < names.size(); ++i) {
        QByteArray body;
        for (const QString &line : texts[i].split(QLatin1Char('\n'))) body += "<w:p><w:r><w:t>" + line.toUtf8() + "</w:t></w:r></w:p>";
        auto src = open(makeDocx(body, kStyles));
        QTextCursor all(src->story());
        all.select(QTextCursor::Document);
        jp::BlockEntry e;
        e.info.name = names[i];
        e.info.gallery = i == 4 ? jp::kShortcutsGallery : jp::kBlocksGallery;
        e.info.description = QStringLiteral("Saved from a letter");
        e.info.format = QStringLiteral("docx");
        e.data = jp::write::blockFromSelection(*src, all);
        QVERIFY(!e.data.isEmpty());
        QString err;
        QVERIFY2(jp::BlockLibrary::shared().add(e, nullptr, &err), qPrintable(err));
    }
    WriteWindow win;
    win.resize(1200, 800);
    win.show();
    win.setDocument(open(makeDocx("<w:p><w:r><w:t>Dear customer,</w:t></w:r></w:p>", kStyles)), QString());
    win.view()->paginateAll();
    win.ribbon()->showTab(win.ribbon()->tab(QStringLiteral("Insert")));
    QMenu *menu = win.findChild<QMenu *>(QStringLiteral("blocksMenu"));
    QVERIFY(menu);
    menu->popup(win.mapToGlobal(QPoint(300, 120)));
    QTRY_VERIFY(menu->isVisible());
    auto *gal = menu->findChild<jp::BlockGalleryList *>();
    QVERIFY(gal);
    QTRY_VERIFY(gal->previewsMade() >= 3);
    menu->grab().save(QDir(shots).filePath(QStringLiteral("blocks-gallery.png")));
    menu->close();
    auto shoot = [&](const char *objectName, const QString &file) {
        int tries = 0;
        auto *t = new QTimer(this);
        t->setInterval(20);
        connect(t, &QTimer::timeout, this, [=, &tries] {
            QWidget *w = QApplication::activeModalWidget();
            if (w && w->objectName() == QLatin1String(objectName) && w->isVisible()) {
                w->grab().save(file);
                w->close();
            } else if (++tries > 400 && w) {
                w->close();
            }
        });
        t->start();
        return t;
    };
    jp::BlockInfo info;
    info.name = QStringLiteral("Payment terms");
    QTimer *t1 = shoot("saveBlockBox", QDir(shots).filePath(QStringLiteral("blocks-save.png")));
    jp::saveBlockBox(&win, &info);
    delete t1;
    QTimer *t2 = shoot("manageBlocksBox", QDir(shots).filePath(QStringLiteral("blocks-organizer.png")));
    jp::manageBlocksBox(&win, QStringLiteral("docx"), [](const jp::BlockEntry &e, const QSize &size, qreal dpr) {
        const auto doc = WriteDoc::fromDocx(e.data);
        QImage img(QSize(qRound(size.width() * dpr), qRound(size.height() * dpr)), QImage::Format_RGB32);
        img.fill(Qt::white);
        img.setDevicePixelRatio(dpr);
        if (!doc) return img;
        const auto page = doc->pager().materialize(0);
        if (!page) return img;
        QPainter p(&img);
        p.scale(size.width() / 250.0, size.width() / 250.0);
        p.translate(-62, -62);
        LayoutCache cache;
        paintPage(&p, *page, &cache, PageDrawing());
        return img;
    });
    delete t2;
    jp::BlockLibrary::setSharedPath(QString());
}
