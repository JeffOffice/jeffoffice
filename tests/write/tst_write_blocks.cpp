// Tests of JeffWrite's saved blocks (core/blockops.h): a selection kept as a small document and put in again, with its styles, pictures, and tables; where it goes in; one step of Undo;
// Text Shortcuts; a block or a file that is hostile; and the glossary part of a file kept as it was.

#include "tst_write.h"
#include "core/blockops.h"
#include "core/insertops.h"
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
#include <QSignalSpy>
#include <QScopeGuard>
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
    // A document that has a style of that name that is another style: its own is not changed, and the table of the copy uses the style of the copy, which comes in under a new name.
    const QByteArray greenFancy = "<w:style w:type=\"table\" w:customStyle=\"1\" w:styleId=\"Fancy\"><w:name w:val=\"Fancy Grid\"/><w:basedOn w:val=\"TableNormal\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"8\" w:color=\"00AA00\"/></w:tblBorders></w:tblPr></w:style>";
    auto sink = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblStyle w:val=\"Fancy\"/><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                              "<w:tr><w:tc><w:p><w:r><w:t>Mine</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>End</w:t></w:r></w:p>", kPlainStyles + greenFancy));
    const auto stylesOf = [](WriteDoc &doc) { return partText(doc, QStringLiteral("word/styles.xml")); };
    const int whereTop = 3;
    for (int times = 0; times < 2; ++times) {
        QTextCursor d(sink->story());
        d.setPosition(whereTop);
        pasteClip(*sink, d, copySelection(*src, selecting(*src, 0, positionAfter(*src, QStringLiteral("After")))));
    }
    const QString styles = stylesOf(*sink), xml = partText(*sink, QStringLiteral("word/document.xml"));
    const auto styleElement = [&](const char *id) {
        const int at = styles.indexOf(QStringLiteral("w:styleId=\"%1\"").arg(QLatin1String(id)));
        return at < 0 ? QString() : styles.mid(at, styles.indexOf(QStringLiteral("</w:style>"), at) - at);
    };
    // (its own style is as it was; the copy's is there once, though it was pasted twice, with the look of the source's)
    QVERIFY2(styleElement("Fancy").contains(QStringLiteral("00AA00")) && !styleElement("Fancy").contains(QStringLiteral("0000FF")), qPrintable(styles));
    QVERIFY2(styleElement("Fancy2").contains(QStringLiteral("0000FF")) && !styleElement("Fancy2").contains(QStringLiteral("00AA00")), qPrintable(styles));
    QCOMPARE(int(styles.count(QStringLiteral("w:styleId=\"Fancy\""))), 1);
    QCOMPARE(int(styles.count(QStringLiteral("w:styleId=\"Fancy2\""))), 1);
    QCOMPARE(int(styles.count(QStringLiteral("w:styleId=\"Fancy3\""))), 0);
    QVERIFY2(styles.contains(QStringLiteral("<w:name w:val=\"Fancy Grid 2\"/>")), qPrintable(styles));
    QCOMPARE(int(xml.count(QStringLiteral("<w:tblStyle w:val=\"Fancy\"/>"))), 1);    // (the document's own table)
    QCOMPARE(int(xml.count(QStringLiteral("<w:tblStyle w:val=\"Fancy2\"/>"))), 2);   // (the two copies of the table, the same style)
    QCOMPARE(int(xml.count(QStringLiteral("<w:tbl>"))), 5);                           // (its own, and an outer and an inner table in each copy)
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

// Track Changes on: a pasted table is an inserted table (its rows and its words), so Reject All takes the whole table out and Accept All keeps it with no marks.
void WriteTests::aTablePastedWithChangesTrackedIsAnInsertedTable()
{
    // The words and the table (pasted at the end of the last paragraph), the table alone (at the end of the last paragraph, and in the middle of the first), and a whole table in a paragraph of its own.
    for (int how = 0; how < 3; ++how) {
        for (const bool accept : {false, true}) {
            auto doc = tableSource();
            QVERIFY(doc);
            const int cellStart = positionAfter(*doc, QStringLiteral("Intro")) + 1, tableEnd = positionAfter(*doc, QStringLiteral("Low C"));
            const Clip clip = how == 0 ? copySelection(*doc, selecting(*doc, positionAfter(*doc, QStringLiteral("Intro")) - 5, positionAfter(*doc, QStringLiteral("After"))))
                                       : copySelection(*doc, selecting(*doc, cellStart, tableEnd));
            QVERIFY(!clip.tables.isEmpty());
            QTextCursor c(doc->story());
            c.setPosition(how == 2 ? positionAfter(*doc, QStringLiteral("Intro")) - 2 : positionAfter(*doc, QStringLiteral("After")));
            const QString wasXml = partText(*doc, QStringLiteral("word/document.xml"));
            doc->setReviewer(QStringLiteral("Pat"), QStringLiteral("P"));
            doc->setTrackChanges(true);
            pasteClip(*doc, c, clip);
            QString xml = partText(*doc, QStringLiteral("word/document.xml"));
            const QByteArray where = QByteArray("case ") + QByteArray::number(how) + (accept ? ", accepted" : ", rejected");
            QVERIFY2(xml.count(QStringLiteral("<w:tbl>")) == 4, where.constData());
            // Every row made is a tracked insertion (the table has 3 rows, the one in its cell 1).
            QVERIFY2(xml.count(QRegularExpression(QStringLiteral("<w:trPr>(<w:tblHeader/>)?<w:ins w:id="))) == 4, where.constData());
            if (accept) {
                QVERIFY(acceptAll(*doc) > 0);
                xml = partText(*doc, QStringLiteral("word/document.xml"));
                QVERIFY2(!xml.contains(QStringLiteral("<w:ins ")) && !xml.contains(QStringLiteral("<w:del ")), where.constData());
                verifyTableIn(*doc, 4, where.constData());
            } else {
                QVERIFY(rejectAll(*doc) > 0);
                doc->setTrackChanges(false);
                xml = partText(*doc, QStringLiteral("word/document.xml"));
                QVERIFY2(xml.count(QStringLiteral("<w:tbl>")) == 2, where.constData());
                QVERIFY2(xml == wasXml, where.constData());
            }
        }
    }
}

// What the clipboard and Keep Text Only get of a table selection is what was selected: the empty paragraphs that the paste of a table uses stay in the copy's private form.
void WriteTests::theClipboardHoldsWhatWasSelectedOfATable()
{
    auto doc = tableSource();
    QVERIFY(doc);
    PageView view;
    view.setDocument(doc.get());
    const int cellStart = positionAfter(*doc, QStringLiteral("Intro")) + 1, tableEnd = positionAfter(*doc, QStringLiteral("Low C"));
    struct Case { int from, to; };
    // Begins in the first cell, ends at the end of the last (the same table), begins in the first and ends after it, and begins before it and ends in it.
    for (const Case &k : {Case{cellStart, tableEnd}, Case{cellStart, positionAfter(*doc, QStringLiteral("After"))}, Case{positionAfter(*doc, QStringLiteral("Intro")) - 5, tableEnd}}) {
        const QTextCursor sel = selecting(*doc, k.from, k.to);
        view.cursor() = sel;
        view.copy();
        const QMimeData *mime = QApplication::clipboard()->mimeData();
        QVERIFY(mime);
        // (what the selection holds, as Qt makes of it: the clipboard of the program before tables were kept had exactly this)
        const QTextDocumentFragment plain(sel);
        QCOMPARE(mime->html(), plain.toHtml());
        QVERIFY2(!mime->text().startsWith(QLatin1Char('\n')) && !mime->text().endsWith(QLatin1Char('\n')), qPrintable(mime->text()));
        const Clip clip = copySelection(*doc, sel);
        QCOMPARE(clip.fragment.toHtml(), plain.toHtml());
        QCOMPARE(plainTextOf(clip.fragment), mime->text());
        // The words Keep Text Only pastes are those words.
        view.cursor().setPosition(doc->story()->characterCount() - 1);
        const int before = doc->story()->blockCount();
        view.pasteAs(PageView::PasteKind::TextOnly);
        QCOMPARE(doc->story()->blockCount() - before, int(mime->text().count(QLatin1Char('\n'))));
        QVERIFY(doc->undo());
    }
    // The text of the first case, as it reads.
    view.cursor() = selecting(*doc, cellStart, tableEnd);
    view.copy();
    QCOMPARE(QApplication::clipboard()->mimeData()->text(), QStringLiteral("Head A\nHead BC\nTall\nOuter\nInner\n\nRight\n\nLow B\nLow C"));
}

// A table selection pasted into a cell or a header makes no paragraph of its own (the words go in as they did before tables were kept).
void WriteTests::aTableSelectionPastedIntoACellOrAHeaderStartsNoParagraph()
{
    auto src = tableSource();
    const Clip clip = copySelection(*src, selecting(*src, positionAfter(*src, QStringLiteral("Intro")) + 1, positionAfter(*src, QStringLiteral("Low C"))));
    QVERIFY(!clip.tables.isEmpty());
    // (the words of the selection as the engine copies them: one new paragraph for each paragraph mark in them)
    const int added = int(QTextDocumentFragment(selecting(*src, positionAfter(*src, QStringLiteral("Intro")) + 1, positionAfter(*src, QStringLiteral("Low C")))).toPlainText().count(QLatin1Char('\n')));
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                                 "<w:tr><w:tc><w:p><w:r><w:t>Cell ab</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>End</w:t></w:r></w:p>", kPlainStyles));
        QTextCursor c(doc->story());
        c.setPosition(positionAfter(*doc, QStringLiteral("Cell a")));
        const int before = paragraphsOf(*doc);
        pasteClip(*doc, c, clip);
        QCOMPARE(paragraphsOf(*doc) - before, added);
        QCOMPARE(tablesIn(*doc), 1);
    }
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Body</w:t></w:r></w:p>", kPlainStyles));
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        QTextDocument *header = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
        QVERIFY(header);
        QTextCursor c(header);
        c.insertText(QStringLiteral("Head ab"));
        c.setPosition(6);
        const int before = header->blockCount();
        pasteClip(*doc, c, clip);
        QCOMPARE(header->blockCount() - before, added);
    }
}

// A table pasted (in this document, and in another) is saved as its source was, apart from ids: no paragraph-mark properties are made up for a bold cell.
void WriteTests::aPastedTableIsSavedAsItsSourceWas()
{
    auto src = tableSource();
    // The outer table of a saved document, from its start to the end of its last row.
    const auto tableOf = [](const QString &xml, int which) {
        // (the which-th table that is not inside another: from its start to the end tag that closes it)
        int depth = 0, start = -1, seen = 0;
        for (int i = 0; i < xml.size();) {
            const int open = xml.indexOf(QStringLiteral("<w:tbl>"), i), close = xml.indexOf(QStringLiteral("</w:tbl>"), i);
            if (close < 0) break;
            if (open >= 0 && open < close) {
                if (depth++ == 0) start = open;
                i = open + 1;
            } else {
                if (--depth == 0 && seen++ == which) return xml.mid(start, close + 8 - start);
                i = close + 1;
            }
        }
        return QString();
    };
    // (the writer says xml:space on words with a space in them, and writes an empty paragraph in full, as a file that was never changed may not)
    const auto normalized = [](QString x) { return x.remove(QStringLiteral(" xml:space=\"preserve\"")).replace(QStringLiteral("<w:p/>"), QStringLiteral("<w:p></w:p>")); };
    const QString sourceTable = tableOf(partText(*src, QStringLiteral("word/document.xml")), 0);
    QVERIFY(sourceTable.contains(QStringLiteral("Head A")));
    for (const bool another : {false, true}) {
        const Clip clip = copySelection(*src, selecting(*src, positionAfter(*src, QStringLiteral("Intro")) + 1, positionAfter(*src, QStringLiteral("Low C"))));
        auto target = another ? open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p>", kPlainStyles)) : tableSource();
        QTextCursor c(target->story());
        c.setPosition(another ? 3 : positionAfter(*target, QStringLiteral("After")));
        pasteClip(*target, c, clip);
        const QString xml = partText(*target, QStringLiteral("word/document.xml"));
        const QString pasted = tableOf(xml, another ? 0 : 1);
        QVERIFY(!pasted.isEmpty());
        const QString a = normalized(pasted), b = normalized(sourceTable);
        int differs = 0;
        while (differs < a.size() && differs < b.size() && a[differs] == b[differs]) ++differs;
        const QByteArray said = (QByteArray(another ? "another document: " : "the same document: ") + "pasted ..." + a.mid(std::max(0, differs - 60), 160).toUtf8() + "... source ..." + b.mid(std::max(0, differs - 60), 160).toUtf8());
        QVERIFY2(a == b, said.constData());
        // (and no paragraph around it gains a property: the saved paragraphs outside the table are the plain ones)
        QVERIFY2(!xml.contains(QStringLiteral("<w:pPr><w:rPr>")), another ? "another document" : "the same document");
    }
}

// A table the selection holds whole that goes as its words says so, and why; a copy of a few words says nothing.
void WriteTests::aTableThatGoesAsItsWordsIsSaid()
{
    const QByteArray row = "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Kept</w:t></w:r></w:p></w:tc>";
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr>" + row + "</w:tr>"
                             "<w:tr><w:trPr><w:ins w:id=\"7\" w:author=\"Pat\" w:date=\"2026-01-01T00:00:00Z\"/></w:trPr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:ins w:id=\"8\" w:author=\"Pat\" w:date=\"2026-01-01T00:00:00Z\"><w:r><w:t>Added row</w:t></w:r></w:ins></w:p></w:tc></w:tr></w:tbl>";
    // A tracked change in it.
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>Tail</w:t></w:r></w:p>", kPlainStyles));
        QVERIFY(doc);
        PageView view;
        view.setDocument(doc.get());
        QStringList said;
        connect(&view, &PageView::notice, this, [&](const QString &t) { said << t; });
        view.cursor() = selecting(*doc, 0, positionAfter(*doc, QStringLiteral("Tail")));
        view.copy();
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QStringLiteral("tracked")) && said[0].contains(QStringLiteral("not as a table")), qPrintable(said[0]));
        QVERIFY(copySelection(*doc, view.cursor()).tables.isEmpty());
        // A few words of the table say nothing (no table was held whole).
        said.clear();
        view.cursor() = selecting(*doc, positionAfter(*doc, QStringLiteral("Kep")) - 2, positionAfter(*doc, QStringLiteral("Kep")));
        view.copy();
        QVERIFY2(said.isEmpty(), qPrintable(said.join(QLatin1Char('|'))));
    }
    // A selection that holds only a part of a content control, and a table whole.
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>a</w:t></w:r><w:sdt><w:sdtPr><w:id w:val=\"5\"/></w:sdtPr><w:sdtContent><w:r><w:t>Ctl</w:t></w:r></w:sdtContent></w:sdt></w:p>"
                                 "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr>" + row + "</w:tr></w:tbl><w:p><w:r><w:t>Tail</w:t></w:r></w:p>", kPlainStyles));
        QVERIFY(doc);
        PageView view;
        view.setDocument(doc.get());
        QStringList said;
        connect(&view, &PageView::notice, this, [&](const QString &t) { said << t; });
        view.cursor() = selecting(*doc, positionAfter(*doc, QStringLiteral("Ct")), positionAfter(*doc, QStringLiteral("Tail")));
        view.copy();
        QCOMPARE(said.size(), 1);
        QVERIFY2(said[0].contains(QStringLiteral("content control")), qPrintable(said[0]));
    }
    // Plain tables say nothing.
    {
        auto doc = tableSource();
        PageView view;
        view.setDocument(doc.get());
        QStringList said;
        connect(&view, &PageView::notice, this, [&](const QString &t) { said << t; });
        view.cursor() = selecting(*doc, 0, positionAfter(*doc, QStringLiteral("After")));
        view.copy();
        QVERIFY2(said.isEmpty(), qPrintable(said.join(QLatin1Char('|'))));
    }
}

// A few words of a table cost no more than the words (the places of the document's tables are made only for a selection that can hold a whole table), and the Spike counts the tables it holds.
void WriteTests::aCopyInsideATableReadsNoTablesAndTheSpikeCountsTheTablesItHolds()
{
    QByteArray rows;
    for (int i = 0; i < 40; ++i) rows += "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"FFFF00\"/></w:tcPr><w:p><w:r><w:t>c" + QByteArray::number(i) + "</w:t></w:r></w:p></w:tc></w:tr>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>" + rows + "</w:tbl><w:p><w:r><w:t>Tail</w:t></w:r></w:p>", kPlainStyles));
    QVERIFY(doc);
    const int inCell = positionAfter(*doc, QStringLiteral("c20"));
    const qint64 builds = copyTableIndexBuildsForTest();
    // A few characters of a cell; a whole cell's words (they begin the paragraph, which is not a table's first); rows from the middle to the end of the table.
    for (const auto &range : {std::pair<int, int>{inCell - 2, inCell}, std::pair<int, int>{inCell - 3, inCell}, std::pair<int, int>{inCell - 3, positionAfter(*doc, QStringLiteral("c30"))}}) {
        const Clip clip = copySelection(*doc, selecting(*doc, range.first, range.second));
        QVERIFY(clip.tables.isEmpty());
        QVERIFY(clip.tablesAsWords > 0);
    }
    QCOMPARE(copyTableIndexBuildsForTest(), builds);
    // A selection that begins at a table's first paragraph can hold the whole table: the places are made, once.
    const Clip whole = copySelection(*doc, selecting(*doc, positionAfter(*doc, QStringLiteral("Top")) + 1, positionAfter(*doc, QStringLiteral("c39"))));
    QCOMPARE(copyTableIndexBuildsForTest(), builds + 1);
    QCOMPARE(int(whole.tables.size()), 1);
    QCOMPARE(int(whole.tables[0].rows.size()), 40);
    // The Spike counts what holds the table: with room for the words and not for the table, the cut is refused and the table stays.
    PageView::emptySpike();
    const auto restore = qScopeGuard([] { PageView::emptySpike(); PageView::setSpikeByteLimit(64 * 1024 * 1024); });
    PageView view;
    view.setDocument(doc.get());
    QStringList said;
    connect(&view, &PageView::notice, this, [&](const QString &t) { said << t; });
    PageView::setSpikeByteLimit(qint64(plainTextOf(whole.fragment).size()) * 2 * 6);   // (six times the words: the words, the marks, and the second form of the words fit, and the table's own bytes do not)
    view.cursor() = selecting(*doc, positionAfter(*doc, QStringLiteral("Top")) + 1, positionAfter(*doc, QStringLiteral("c39")));
    view.cutToSpike();
    QCOMPARE(PageView::spikeCount(), 0);
    QCOMPARE(tablesIn(*doc), 1);
    QVERIFY2(said.size() == 1 && said[0].contains(QStringLiteral("Spike is full")), qPrintable(said.join(QLatin1Char('|'))));
    // (and with room, the same cut goes)
    PageView::setSpikeByteLimit(64 * 1024 * 1024);
    view.cursor() = selecting(*doc, positionAfter(*doc, QStringLiteral("Top")) + 1, positionAfter(*doc, QStringLiteral("c39")));
    view.cutToSpike();
    QCOMPARE(PageView::spikeCount(), 1);
    QCOMPARE(tablesIn(*doc), 0);
}

// RTF on the clipboard and Insert Text from File keep a table as a table, within the same limits of a copy.
void WriteTests::aTableInRtfOrATextFileIsATableWithinTheLimitsOfACopy()
{
    const auto restore = qScopeGuard([] { setCopyLimitsForTest(0, 0); });
    QByteArray rtf = "{\\rtf1\\ansi\\deff0 Before\\par ";
    QByteArray body = "<w:p><w:r><w:t>Before</w:t></w:r></w:p>";
    for (int i = 0; i < 5; ++i) {
        rtf += "\\trowd\\cellx2000\\cellx4000\\intbl T" + QByteArray::number(i) + "a\\cell T" + QByteArray::number(i) + "b\\cell\\row\\pard between\\par ";
        body += "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>T" + QByteArray::number(i) + "a</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>T" + QByteArray::number(i) +
                "b</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>between</w:t></w:r></w:p>";
    }
    rtf += "}";
    const QByteArray file = makeDocx(body, kPlainStyles);
    const auto emptyTarget = [] { return open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p>", kPlainStyles)); };
    const auto fromRtf = [&](WriteDoc &target) {
        Clip clip;
        if (!clipFromRtf(rtf, &clip)) return false;
        QTextCursor c(target.story());
        c.setPosition(3);
        pasteClip(target, c, clip);
        return true;
    };
    const auto fromFile = [&](WriteDoc &target) {
        QTextCursor c(target.story());
        c.setPosition(3);
        QString problem;
        return insertTextFromFile(target, c, file, &problem);
    };
    // Without limits: five tables each way (saved as tables in the file the document saves, which says each is one with two cells).
    for (const bool rtfPath : {true, false}) {
        auto target = emptyTarget();
        QVERIFY(rtfPath ? fromRtf(*target) : fromFile(*target));
        QCOMPARE(tablesIn(*target), 5);
        QVERIFY(allText(*target).contains(QStringLiteral("T4b")));
        QVERIFY(target->undo());
        QCOMPARE(tablesIn(*target), 0);
    }
    // The most tables: the rest of them are their words (and the words are all there).
    setCopyLimitsForTest(3, 0);
    for (const bool rtfPath : {true, false}) {
        auto target = emptyTarget();
        QVERIFY(rtfPath ? fromRtf(*target) : fromFile(*target));
        QCOMPARE(tablesIn(*target), 3);
        QVERIFY(allText(*target).contains(QStringLiteral("T4b")) && allText(*target).contains(QStringLiteral("T3a")));
    }
    // The most cells (all the tables together): each table has two, so 5 hold the first two tables only.
    setCopyLimitsForTest(0, 5);
    for (const bool rtfPath : {true, false}) {
        auto target = emptyTarget();
        QVERIFY(rtfPath ? fromRtf(*target) : fromFile(*target));
        QCOMPARE(tablesIn(*target), 2);
        QVERIFY(allText(*target).contains(QStringLiteral("T4b")));
    }
}

// A shape and a text box in a block come in again, in this document and in another (they were lost without a word: a pasted shape had no picture to find, so it was not made again).
void WriteTests::aShapeAndATextBoxInABlockComeBackAsObjects()
{
    auto src = open(makeDocx("<w:p><w:r><w:t>Words</w:t></w:r></w:p><w:p><w:r><w:t>More</w:t></w:r></w:p>", kPlainStyles));
    QTextCursor at(src->story());
    at.setPosition(2);
    QCOMPARE(insertTextBox(*src, at, textBoxDesigns().first()), PictureResult::Ok);
    QTextCursor at2(src->story());
    at2.setPosition(allText(*src).size() - 1);
    QCOMPARE(insertShape(*src, at2, QStringLiteral("rect")), PictureResult::Ok);
    const auto boxes = [](WriteDoc &doc) {
        int withStory = 0;
        for (const flow::FloatingObject &o : doc.flow().objects)
            if (o.story) ++withStory;
        return withStory;
    };
    QCOMPARE(int(src->flow().objects.size()), 2);
    QCOMPARE(boxes(*src), 1);
    QTextCursor all(src->story());
    all.select(QTextCursor::Document);
    QString problem;
    QStringList said;
    const QByteArray block = jp::write::blockFromSelection(*src, all, &problem, &said);
    QVERIFY2(!block.isEmpty(), qPrintable(problem));
    QVERIFY2(said.isEmpty(), qPrintable(said.join(QLatin1Char('|'))));
    // Put in another document, a second time, and Undo takes both out.
    auto target = open(makeDocx("<w:p><w:r><w:t>Top</w:t></w:r></w:p>", kPlainStyles));
    for (int times = 1; times <= 2; ++times) {
        QTextCursor c(target->story());
        c.setPosition(3);
        QVERIFY2(jp::write::insertBlock(*target, c, block, jp::BlockInsert::Paragraph, &problem), qPrintable(problem));
        QCOMPARE(int(target->flow().objects.size()), 2 * times);
        QCOMPARE(boxes(*target), times);
    }
    QVERIFY(target->undo());
    // And they are in the file the document saves (the objects of the second putting in went with Undo; the records of them are kept for Redo).
    auto again = open(target->toDocx());
    QVERIFY(again);
    QCOMPARE(int(again->flow().objects.size()), 2);
    QCOMPARE(boxes(*again), 1);
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
    QCOMPARE(said.size(), 3);   // (the whole table is kept as a table: nothing is said of it)
    QVERIFY2(!joined.contains(QStringLiteral("table")) && joined.contains(QStringLiteral("Footnotes")) && joined.contains(QStringLiteral("Comments")) && joined.contains(QStringLiteral("Tracked")), qPrintable(joined));
    // A plain selection leaves nothing out, so nothing is said.
    auto plain = sourceDoc();
    QStringList none;
    QVERIFY(!jp::write::blockFromSelection(*plain, selecting(*plain, 0, positionAfter(*plain, QStringLiteral("Last line"))), nullptr, &none).isEmpty());
    QVERIFY2(none.isEmpty(), qPrintable(none.join(QLatin1Char('|'))));
}
