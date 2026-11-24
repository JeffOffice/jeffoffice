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

// A document with words around a table that has a header row, shading, a border of its own, a merged cell across and one down, bold words, a nested table, and a table style.
static std::unique_ptr<WriteDoc> tableSource(const QByteArray &fancy = QByteArray())
{
    const QByteArray style = fancy.isEmpty()
        ? QByteArray("<w:style w:type=\"table\" w:customStyle=\"1\" w:styleId=\"Fancy\"><w:name w:val=\"Fancy Grid\"/><w:basedOn w:val=\"TableNormal\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"0000FF\"/>"
                     "<w:insideH w:val=\"single\" w:sz=\"8\" w:color=\"0000FF\"/></w:tblBorders></w:tblPr></w:style>")
        : fancy;
    const QByteArray body =
        "<w:p><w:r><w:t>Intro</w:t></w:r></w:p>"
        "<w:tbl><w:tblPr><w:tblStyle w:val=\"Fancy\"/><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"1500\"/></w:tblGrid>"
        "<w:tr><w:trPr><w:tblHeader/></w:trPr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FFFF00\"/></w:tcPr><w:p><w:r><w:rPr><w:b/></w:rPr><w:t>Head A</w:t></w:r></w:p></w:tc>"
        "<w:tc><w:tcPr><w:tcW w:w=\"4500\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/></w:tcPr><w:p><w:r><w:t>Head BC</w:t></w:r></w:p></w:tc></w:tr>"
        "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge w:val=\"restart\"/><w:tcBorders><w:top w:val=\"single\" w:sz=\"12\" w:color=\"FF0000\"/></w:tcBorders></w:tcPr><w:p><w:r><w:t>Tall</w:t></w:r></w:p></w:tc>"
        "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Outer</w:t></w:r></w:p>"
        "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Inner</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:tc>"
        "<w:tc><w:tcPr><w:tcW w:w=\"1500\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Right</w:t></w:r></w:p></w:tc></w:tr>"
        "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:vMerge/></w:tcPr><w:p/></w:tc><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Low B</w:t></w:r></w:p></w:tc>"
        "<w:tc><w:tcPr><w:tcW w:w=\"1500\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Low C</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
        "<w:p><w:r><w:t>After</w:t></w:r></w:p>";
    return open(makeDocx(body, kPlainStyles + style));
}

static QString partText(WriteDoc &doc, const QString &name)
{
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(doc.toDocx(), parts)) return QString();
    return QString::fromUtf8(parts.value(name));
}

// What the saved file of a document says of the table made in tableSource(): the pieces a person sees.
static void verifyTableIn(WriteDoc &doc, int tables, const char *where)
{
    const QString xml = partText(doc, QStringLiteral("word/document.xml"));
    const auto count = [&](const QString &needle) { return int(xml.count(needle)); };
    QVERIFY2(count(QStringLiteral("<w:tbl>")) == tables, where);
    QVERIFY2(count(QStringLiteral("w:gridSpan w:val=\"2\"")) == tables / 2, where);
    QVERIFY2(count(QStringLiteral("<w:vMerge")) == 2 * (tables / 2), where);
    QVERIFY2(count(QStringLiteral("<w:tblHeader")) == tables / 2, where);
    QVERIFY2(count(QStringLiteral("w:fill=\"FFFF00\"")) == tables / 2, where);
    QVERIFY2(count(QStringLiteral("w:color=\"FF0000\"")) == tables / 2, where);
    QVERIFY2(count(QStringLiteral("<w:gridCol w:w=\"3000\"/>")) == tables / 2, where);
    QVERIFY2(count(QStringLiteral("<w:tblStyle w:val=\"Fancy\"/>")) == tables / 2, where);
    // (the nested table is the table each copy holds besides the outer one: tables counts both)
    QVERIFY2(xml.contains(QStringLiteral("Inner")) && xml.contains(QStringLiteral("Low C")) && xml.contains(QStringLiteral("Head BC")), where);
    const int head = xml.indexOf(QStringLiteral("Head A"));
    QVERIFY2(head > 0 && xml.lastIndexOf(QStringLiteral("<w:b/>"), head) > xml.lastIndexOf(QStringLiteral("<w:tc>"), head), where);   // (the bold is on the words of that cell)
}

// Copy and paste within one document must already carry a table as a table: the same copy goes in again after the table.
void WriteTests::aTablePastedWithinOneDocumentIsATableWithItsLook()
{
    auto doc = tableSource();
    QVERIFY(doc);
    verifyTableIn(*doc, 2, "the source");
    const Clip clip = copySelection(*doc, selecting(*doc, positionAfter(*doc, QStringLiteral("Intro")) - 5, positionAfter(*doc, QStringLiteral("After"))));
    QTextCursor c(doc->story());
    c.setPosition(positionAfter(*doc, QStringLiteral("After")));
    const QString before = allText(*doc);
    pasteClip(*doc, c, clip);
    QCOMPARE(tablesIn(*doc), 4);   // (two tables in the copy as in the source: the outer one and the one in its cell)
    verifyTableIn(*doc, 4, "the same document, after the paste");
    QVERIFY(doc->undo());
    QCOMPARE(allText(*doc), before);
    verifyTableIn(*doc, 2, "the same document, after Undo");
    QVERIFY(doc->redo());
    verifyTableIn(*doc, 4, "the same document, after Redo");
    // It saves and reads back with the same four tables.
    auto again = open(doc->toDocx());
    QVERIFY(again);
    verifyTableIn(*again, 4, "read back");
}

void WriteTests::aTableCopiedToAnotherDocumentIsATableThereWithItsStyle()
{
    auto src = tableSource();
    const Clip clip = copySelection(*src, selecting(*src, 0, positionAfter(*src, QStringLiteral("After"))));
    // A document that lacks the table style: it is added, and the table is the same table.
    auto target = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p><w:p><w:r><w:t>Bottom</w:t></w:r></w:p>", kPlainStyles));
    QTextCursor c(target->story());
    c.setPosition(positionAfter(*target, QStringLiteral("Top")));
    const QString before = allText(*target);
    pasteClip(*target, c, clip);
    verifyTableIn(*target, 2, "another document");
    QVERIFY(partText(*target, QStringLiteral("word/styles.xml")).contains(QStringLiteral("w:styleId=\"Fancy\"")));
    QVERIFY(allText(*target).startsWith(QStringLiteral("Top")) && allText(*target).contains(QStringLiteral("After")) && allText(*target).endsWith(QStringLiteral("Bottom")));
    QVERIFY(target->undo());
    QCOMPARE(allText(*target), before);
    QCOMPARE(tablesIn(*target), 0);
    QVERIFY(!partText(*target, QStringLiteral("word/styles.xml")).contains(QStringLiteral("w:styleId=\"Fancy\"")));
    // A document that has a style of that name that is another style: its own is not changed, and the table uses it.
    auto other = tableSource("<w:style w:type=\"table\" w:customStyle=\"1\" w:styleId=\"Fancy\"><w:name w:val=\"Fancy Grid\"/><w:basedOn w:val=\"TableNormal\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"00AA00\"/></w:tblBorders></w:tblPr></w:style>");
    auto sink = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p>", kPlainStyles + "<w:style w:type=\"table\" w:customStyle=\"1\" w:styleId=\"Fancy\"><w:name w:val=\"Fancy Grid\"/><w:basedOn w:val=\"TableNormal\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"00AA00\"/></w:tblBorders></w:tblPr></w:style>"));
    QTextCursor d(sink->story());
    d.setPosition(3);
    pasteClip(*sink, d, copySelection(*src, selecting(*src, 0, positionAfter(*src, QStringLiteral("After")))));
    const QString styles = partText(*sink, QStringLiteral("word/styles.xml"));
    QVERIFY2(styles.contains(QStringLiteral("00AA00")) && !styles.contains(QStringLiteral("0000FF")), qPrintable(styles));
    QCOMPARE(int(styles.count(QStringLiteral("w:styleId=\"Fancy\""))), 1);
    verifyTableIn(*sink, 2, "a document with a style of the same name");
}

void WriteTests::aTableInABlockComesBackAsATableBetweenParagraphsAndAsOneStep()
{
    auto src = tableSource();
    QString problem;
    QStringList said;
    // The whole run of paragraphs and the table between them.
    const QByteArray block = jp::write::blockFromSelection(*src, selecting(*src, 0, positionAfter(*src, QStringLiteral("After"))), &problem, &said);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    QVERIFY2(said.isEmpty(), qPrintable(said.join(QLatin1Char('|'))));   // (nothing is left out: the save box has nothing to say of the table)
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(block, parts));
    QCOMPARE(int(QString::fromUtf8(parts.value(QStringLiteral("word/document.xml"))).count(QStringLiteral("<w:tbl>"))), 2);
    for (const jp::BlockInsert how : {jp::BlockInsert::Content, jp::BlockInsert::Paragraph, jp::BlockInsert::Page}) {
        auto target = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p><w:p><w:r><w:t>Bottom</w:t></w:r></w:p>", kPlainStyles));
        QTextCursor c(target->story());
        c.setPosition(positionAfter(*target, QStringLiteral("Top")));
        const QString before = allText(*target);
        QVERIFY2(jp::write::insertBlock(*target, c, block, how, &problem), qPrintable(problem));
        verifyTableIn(*target, 2, "a block put in");
        QVERIFY(target->undo());
        QCOMPARE(allText(*target), before);
        QCOMPARE(tablesIn(*target), 0);
        QVERIFY(target->redo());
        verifyTableIn(*target, 2, "a block put in again by Redo");
    }
    // A block that is only the table: in a paragraph of its own, between the words of a paragraph, and at the start of a paragraph.
    const int from = positionAfter(*src, QStringLiteral("Intro")) + 1, to = positionAfter(*src, QStringLiteral("Low C")) + 1;
    QStringList none;
    const QByteArray only = jp::write::blockFromSelection(*src, selecting(*src, from, to), &problem, &none);
    QVERIFY2(!only.isEmpty(), qPrintable(problem));
    QVERIFY2(none.isEmpty(), qPrintable(none.join(QLatin1Char('|'))));
    for (const int where : {2, 0, 6}) {
        auto target = open(makeDocx("<w:p><w:r><w:t>Words here</w:t></w:r></w:p>", kPlainStyles));
        QTextCursor c(target->story());
        c.setPosition(where);
        QVERIFY2(jp::write::insertBlock(*target, c, only, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
        verifyTableIn(*target, 2, "the table alone");
        QVERIFY(allText(*target).contains(QStringLiteral("Words here")) || allText(*target).contains(QStringLiteral("Wo")));
        // (no empty paragraph before the table: the first paragraph is the words or the table's first cell)
        QVERIFY2(!target->story()->firstBlock().text().isEmpty(), qPrintable(allText(*target)));
    }
    // The selection ends at the end of the last cell: no words of the paragraph the block goes into are joined to it.
    {
        const QByteArray cut = jp::write::blockFromSelection(*src, selecting(*src, from, positionAfter(*src, QStringLiteral("Low C"))), &problem);
        QVERIFY2(!cut.isEmpty(), qPrintable(problem));
        auto target = open(makeDocx("<w:p><w:r><w:t>Words here</w:t></w:r></w:p>", kPlainStyles));
        QTextCursor c(target->story());
        c.setPosition(5);
        QVERIFY2(jp::write::insertBlock(*target, c, cut, jp::BlockInsert::Content, &problem), qPrintable(problem));
        verifyTableIn(*target, 2, "the table to the end of its last cell, as it is");
        QVERIFY(allText(*target).contains(QStringLiteral("Low C")) && allText(*target).endsWith(QStringLiteral(" here")));
    }
    // The block is a document that reads back as the same table.
    auto read = open(block);
    QVERIFY(read);
    verifyTableIn(*read, 2, "the block read as a document");
}

void WriteTests::aTableIsNotCarriedPartWayAndTrackedTablesAreAccepted()
{
    auto src = tableSource();
    // Part of a table (from its second row on) is its words, as a paste of part of a table always was: no half table is made.
    QStringList said;
    QString problem;
    const QByteArray part = jp::write::blockFromSelection(*src, selecting(*src, positionAfter(*src, QStringLiteral("Outer")), positionAfter(*src, QStringLiteral("Low C"))), &problem, &said);
    QVERIFY2(!part.isEmpty(), qPrintable(problem));
    auto read = open(part);
    QVERIFY(read);
    QCOMPARE(tablesIn(*read), 0);
    QVERIFY(allText(*read).contains(QStringLiteral("Low B")));
    QCOMPARE(said.size(), 1);
    // A table with a tracked row is a table in a block, with the change accepted.
    auto tracked = open(makeDocx(
        "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>"
        "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Kept</w:t></w:r></w:p></w:tc></w:tr>"
        "<w:tr><w:trPr><w:ins w:id=\"7\" w:author=\"Pat\" w:date=\"2026-01-01T00:00:00Z\"/></w:trPr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:ins w:id=\"8\" w:author=\"Pat\" w:date=\"2026-01-01T00:00:00Z\"><w:r><w:t>Added row</w:t></w:r></w:ins></w:p></w:tc></w:tr></w:tbl>"
        "<w:p><w:r><w:t>Tail</w:t></w:r></w:p>", kPlainStyles));
    QVERIFY(tracked);
    QStringList told;
    const QByteArray block = jp::write::blockFromSelection(*tracked, selecting(*tracked, 0, positionAfter(*tracked, QStringLiteral("Tail"))), &problem, &told);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    auto back = open(block);
    QVERIFY(back);
    QCOMPARE(tablesIn(*back), 1);
    const QString xml = partText(*back, QStringLiteral("word/document.xml"));
    QVERIFY2(xml.contains(QStringLiteral("Added row")) && !xml.contains(QStringLiteral("<w:ins ")), qPrintable(xml));
}

void WriteTests::aBlockPutInTwiceHasBookmarksOfItsOwnEachTime()
{
    auto src = open(makeDocx("<w:p><w:bookmarkStart w:id=\"3\" w:name=\"Mark\"/><w:r><w:t>Marked words</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p>", kPlainStyles));
    QVERIFY(src);
    QString problem;
    const QByteArray block = jp::write::blockFromSelection(*src, selecting(*src, 0, src->story()->characterCount() - 1), &problem);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    // The document already has a bookmark of that name, and the block goes in twice.
    auto target = open(makeDocx("<w:p><w:bookmarkStart w:id=\"3\" w:name=\"Mark\"/><w:r><w:t>Own</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p><w:p><w:r><w:t>End</w:t></w:r></w:p>", kPlainStyles));
    for (int i = 0; i < 2; ++i) {
        QTextCursor c(target->story());
        c.setPosition(positionAfter(*target, QStringLiteral("End")));
        QVERIFY2(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
    }
    const QString xml = partText(*target, QStringLiteral("word/document.xml"));
    QCOMPARE(int(xml.count(QStringLiteral("<w:bookmarkStart"))), 3);
    QSet<QString> names, ids;
    QRegularExpressionMatchIterator it = QRegularExpression(QStringLiteral("<w:bookmarkStart w:id=\"(\\d+)\" w:name=\"([^\"]*)\"")).globalMatch(xml);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        ids.insert(m.captured(1));
        names.insert(m.captured(2));
    }
    QCOMPARE(names.size(), 3);
    QCOMPARE(ids.size(), 3);
    QVERIFY2(names.contains(QStringLiteral("Mark")) && names.contains(QStringLiteral("Mark_2")) && names.contains(QStringLiteral("Mark_3")), qPrintable(QStringList(names.begin(), names.end()).join(QLatin1Char(' '))));
    // (Each start has its end: the ends have the same three numbers.)
    QCOMPARE(int(xml.count(QStringLiteral("<w:bookmarkEnd"))), 3);
    for (const QString &id : std::as_const(ids)) QVERIFY2(xml.contains(QStringLiteral("<w:bookmarkEnd w:id=\"%1\"/>").arg(id)), qPrintable(id));
    // One step of the history for each.
    QVERIFY(target->undo());
    QCOMPARE(int(partText(*target, QStringLiteral("word/document.xml")).count(QStringLiteral("<w:bookmarkStart"))), 2);
}

void WriteTests::aBlockInAParagraphOfItsOwnLeavesNoEmptyParagraphBeforeIt()
{
    QString problem;
    const QByteArray block = savedFromSource(&problem);   // (a heading, a paragraph of a style of its own, and a last line)
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    // At the end of the last paragraph, in the middle of a paragraph, and at the start of one: no empty paragraph before or after the block, and one step of Undo.
    for (const int at : {-1, 3, 0}) {
        auto target = open(makeDocx("<w:p><w:r><w:t>Mine</w:t></w:r></w:p><w:p><w:r><w:t>End</w:t></w:r></w:p>", kStyles));
        const QString before = allText(*target);
        QTextCursor c(target->story());
        c.setPosition(at < 0 ? target->story()->characterCount() - 1 : at);
        QVERIFY2(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
        QString dump;
        for (QTextBlock b = target->story()->begin(); b.isValid(); b = b.next()) dump += QStringLiteral("[%1]").arg(b.text());
        QVERIFY2(!dump.contains(QStringLiteral("[]")), qPrintable(dump));
        QVERIFY2(dump.contains(QStringLiteral("[Terms][Payment is due in thirty days.][Last line]")), qPrintable(dump));
        QVERIFY(target->undo());
        QCOMPARE(allText(*target), before);
    }
}

// F3 is on only where a Text Shortcut could be, and the caret moving about asks that of a count, not of a list made again from a library of thousands.
void WriteTests::aCaretThatMovesDoesNotMakeALibraryListForTheShortcutKey()
{
    QTemporaryDir dir;
    jp::BlockLibrary::setSharedPath(dir.filePath(QStringLiteral("shared.jblocks")));
    const QByteArray block = savedFromSource();
    QVERIFY(!block.isEmpty());
    WriteWindow win;
    win.resize(900, 700);
    win.show();
    win.setDocument(open(makeDocx("<w:p><w:r><w:t>Some words to move through</w:t></w:r></w:p>", kPlainStyles)), QString());
    win.view()->paginateAll();
    QTextCursor c(win.document()->story());
    c.setPosition(1);
    win.view()->setCursor(c);
    QTRY_VERIFY(!win.act(QStringLiteral("insert.expandShortcut"))->isEnabled());   // (no library yet: no Text Shortcut could be here)
    for (int i = 0; i < 30; ++i) {
        jp::BlockEntry e;
        e.info.name = QStringLiteral("sc%1").arg(i);
        e.info.gallery = jp::kShortcutsGallery;
        e.info.format = QStringLiteral("docx");
        e.data = block;
        QString err;
        QVERIFY2(jp::BlockLibrary::shared().add(e, nullptr, &err), qPrintable(err));
    }
    QVERIFY(jp::BlockLibrary::shared().load());   // (once it is read, the answer comes from the library in memory)
    QVERIFY(jp::BlockLibrary::shared().loaded());
    const qint64 before = jp::BlockLibrary::galleryListsMadeForTest();
    for (int i = 2; i < 22; ++i) {
        c.setPosition(i);
        win.view()->setCursor(c);
    }
    QTRY_VERIFY(win.act(QStringLiteral("insert.expandShortcut"))->isEnabled());   // (the window asked, after the moves)
    QCOMPARE(jp::BlockLibrary::galleryListsMadeForTest(), before);   // (and no list was made for it)
    jp::BlockLibrary::setSharedPath(QString());
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

// A document with a table, a footnote, a comment, and tracked changes, as a source for what a block does not carry.
static std::unique_ptr<WriteDoc> richSource()
{
    const QByteArray body =
        "<w:p><w:r><w:t>Intro</w:t></w:r></w:p>"
        "<w:p><w:r><w:t>Second item</w:t></w:r></w:p>"
        "<w:p><w:r><w:t>Marked</w:t></w:r>"
        "<w:ins w:id=\"1\" w:author=\"Pat\" w:date=\"2026-01-01T00:00:00Z\"><w:r><w:t xml:space=\"preserve\"> inserted</w:t></w:r></w:ins>"
        "<w:del w:id=\"2\" w:author=\"Pat\" w:date=\"2026-01-01T00:00:00Z\"><w:r><w:delText xml:space=\"preserve\"> deleted</w:delText></w:r></w:del></w:p>"
        "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
        "<w:tr><w:tc><w:p><w:r><w:t>Cell A</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>Cell B</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
        "<w:p><w:r><w:t>Tail words</w:t></w:r></w:p>";
    auto src = open(makeDocx(body, kStyles));
    if (!src) return src;
    QTextCursor c(src->story());
    c.setPosition(positionAfter(*src, QStringLiteral("Intro")) - 3);
    c.setPosition(positionAfter(*src, QStringLiteral("Intro")), QTextCursor::KeepAnchor);
    jp::write::addComment(*src, c, QStringLiteral("Look here"));
    QTextCursor f(src->story());
    f.setPosition(positionAfter(*src, QStringLiteral("Second item")));
    jp::write::insertFootnote(*src, f, QString());
    return src;
}

void WriteTests::aBlockHasNoCommentsAndItsWordsAsTheyReadWithChangesAccepted()
{
    auto src = richSource();
    QVERIFY(src);
    QTextCursor all(src->story());
    all.select(QTextCursor::Document);
    QString problem;
    const QByteArray block = jp::write::blockFromSelection(*src, all, &problem);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(block, parts));
    const QString document = QString::fromUtf8(parts.value(QStringLiteral("word/document.xml")));
    QVERIFY(document.contains(QStringLiteral("Intro")) && document.contains(QStringLiteral("inserted")));
    QVERIFY(!document.contains(QStringLiteral("deleted")));
    QVERIFY2(!document.contains(QStringLiteral("<w:ins ")) && !document.contains(QStringLiteral("<w:del ")), qPrintable(document));
    QVERIFY2(!document.contains(QStringLiteral("commentRange")) && !document.contains(QStringLiteral("commentReference")), qPrintable(document));
    QVERIFY(!QString::fromUtf8(parts.value(QStringLiteral("word/comments.xml"))).contains(QStringLiteral("Look here")));
    // Put in a document: no comment and no tracked change comes with it.
    auto target = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p>", kPlainStyles));
    QTextCursor c(target->story());
    c.setPosition(3);
    QVERIFY2(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
    QMap<QString, QByteArray> out;
    QVERIFY(jp::readZip(target->toDocx(), out));
    const QString saved = QString::fromUtf8(out.value(QStringLiteral("word/document.xml")));
    QVERIFY(saved.contains(QStringLiteral("inserted")));
    QVERIFY(!saved.contains(QStringLiteral("commentReference")) && !saved.contains(QStringLiteral("<w:ins ")) && !saved.contains(QStringLiteral("<w:del ")));
}

void WriteTests::theSaveBoxIsToldWhatABlockLeavesOut()
{
    auto src = richSource();
    QVERIFY(src);
    QTextCursor all(src->story());
    all.select(QTextCursor::Document);
    QStringList said;
    QVERIFY(!jp::write::blockFromSelection(*src, all, nullptr, &said).isEmpty());
    const QString joined = said.join(QLatin1Char('|'));
    QCOMPARE(said.size(), 4);
    QVERIFY2(joined.contains(QStringLiteral("table")) && joined.contains(QStringLiteral("Footnotes")) && joined.contains(QStringLiteral("Comments")) && joined.contains(QStringLiteral("Tracked")), qPrintable(joined));
    // A plain selection leaves nothing out, so nothing is said.
    auto plain = sourceDoc();
    QStringList none;
    QVERIFY(!jp::write::blockFromSelection(*plain, selecting(*plain, 0, positionAfter(*plain, QStringLiteral("Last line"))), nullptr, &none).isEmpty());
    QVERIFY2(none.isEmpty(), qPrintable(none.join(QLatin1Char('|'))));
}
