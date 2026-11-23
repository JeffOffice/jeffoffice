// Tests of moving a section (a heading with what is under it): the move keeps every part of the saved file as if the section had been typed at the new place, and nothing else changes.

#include "../isolatesettings.h"
#include "app/navpane.h"
#include "app/pageview.h"
#include "app/theme.h"
#include "app/writewindow.h"
#include "core/blockmove.h"
#include "core/editops.h"
#include "core/sectionmove.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QFrame>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStatusBar>
#include <QTreeWidget>
#include <QImage>
#include <QRegularExpression>
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>

using namespace jp;
using namespace jp::write;

namespace {

const QByteArray kNamespaces =
    "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
    "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
    "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"";
const QByteArray kPageSize = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
const QByteArray kRel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";
const QString kMain = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

QByteArray run(const QByteArray &text, const QByteArray &props = QByteArray()) { return "<w:r>" + (props.isEmpty() ? QByteArray() : "<w:rPr>" + props + "</w:rPr>") + "<w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; }
QByteArray para(const QByteArray &text, const QByteArray &ppr = QByteArray(), const QByteArray &inner = QByteArray()) { return "<w:p>" + (ppr.isEmpty() ? QByteArray() : "<w:pPr>" + ppr + "</w:pPr>") + run(text) + inner + "</w:p>"; }
// A paragraph whose mark has formatting of its own and whose paragraph has its own alignment: what a move must leave where it is.
QByteArray marked(const QByteArray &text, const QByteArray &more = QByteArray()) { return para(text, "<w:jc w:val=\"right\"/>" + more + "<w:rPr><w:b/><w:i/><w:color w:val=\"C00000\"/></w:rPr>"); }
QByteArray head(const QByteArray &text, int level = 1, const QByteArray &more = QByteArray()) { return para(text, "<w:pStyle w:val=\"Heading" + QByteArray::number(level) + "\"/>" + more); }

QByteArray pngBytes()
{
    QImage image(20, 10, QImage::Format_RGB32);
    image.fill(Qt::black);
    for (int y = 0; y < 10; ++y)
        for (int x = 10; x < 20; ++x) image.setPixel(x, y, qRgb(255, 255, 255));
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return png;
}

QByteArray pictureGraphic()
{
    return "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"x\"/><pic:cNvPicPr/></pic:nvPicPr>"
           "<pic:blipFill><a:blip r:embed=\"rIdPic\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"952500\" cy=\"476250\"/></a:xfrm>"
           "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic>";
}

QByteArray inlinePicture(int id) { return "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"952500\" cy=\"476250\"/><wp:docPr id=\"" + QByteArray::number(id) + "\" name=\"Picture\"/>" + pictureGraphic() + "</wp:inline></w:drawing></w:r>"; }

QByteArray anchored(int id, const QByteArray &uri, const QByteArray &graphic, int x, int y)
{
    return "<w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"" + QByteArray::number(id) + "\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
           "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"page\"><wp:posOffset>" + QByteArray::number(x) + "</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"page\"><wp:posOffset>" + QByteArray::number(y) +
           "</wp:posOffset></wp:positionV><wp:extent cx=\"1270000\" cy=\"635000\"/><wp:wrapNone/><wp:docPr id=\"" + QByteArray::number(id) + "\" name=\"Object\"/><wp:cNvGraphicFramePr/>"
           "<a:graphic><a:graphicData uri=\"" + uri + "\">" + graphic + "</a:graphicData></a:graphic></wp:anchor></w:drawing></w:r>";
}

QByteArray anchoredPicture(int id) { return anchored(id, "http://schemas.openxmlformats.org/drawingml/2006/picture", pictureGraphic().mid(pictureGraphic().indexOf("<pic:pic>"), pictureGraphic().indexOf("</a:graphicData>") - pictureGraphic().indexOf("<pic:pic>")), 1270000, 2540000); }

QByteArray drawnShape(int id)
{
    return anchored(id, "http://schemas.microsoft.com/office/word/2010/wordprocessingShape",
                    "<wps:wsp><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"635000\"/></a:xfrm><a:prstGeom prst=\"ellipse\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"00AA44\"/></a:solidFill></wps:spPr><wps:bodyPr/></wps:wsp>",
                    2540000, 3810000);
}

QByteArray textBox(int id)
{
    return anchored(id, "http://schemas.microsoft.com/office/word/2010/wordprocessingShape",
                    "<wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"635000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr>"
                    "<wps:txbx><w:txbxContent><w:p><w:r><w:t>words in the box</w:t></w:r></w:p></w:txbxContent></wps:txbx><wps:bodyPr/></wps:wsp>",
                    3810000, 5080000);
}

QByteArray noteRef(const char *kind, int id) { return "<w:r><w:rPr><w:rStyle w:val=\"" + QByteArray(kind) + "Reference\"/></w:rPr><w:" + kind + "Reference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; }

QByteArray cell(int width, const QByteArray &inner, const QByteArray &extra = QByteArray()) { return "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(width) + "\" w:type=\"dxa\"/>" + extra + "</w:tcPr>" + inner + "</w:tc>"; }

QByteArray tableWithMergesAndANestedTable()
{
    const QByteArray nested = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1500\"/><w:gridCol w:w=\"1500\"/></w:tblGrid>"
                              "<w:tr>" + cell(1500, para("n1")) + cell(1500, para("n2")) + "</w:tr></w:tbl>";
    return "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
           "<w:tr>" + cell(6000, para("wide"), "<w:gridSpan w:val=\"2\"/>") + cell(3000, para("tall"), "<w:vMerge w:val=\"restart\"/>") + "</w:tr>"
           "<w:tr>" + cell(3000, para("a")) + cell(3000, nested + para("after nested")) + cell(3000, para(""), "<w:vMerge/>") + "</w:tr></w:tbl>";
}

// The package of a document: the body, and the parts its notes, comment, numbering, picture, and settings need.
QByteArray build(const QByteArray &body, const QByteArray &finalSection = "<w:sectPr>" + kPageSize + "</w:sectPr>", bool numberedHeadings = false, const QByteArray &settings = QByteArray())
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + kNamespaces + "><w:body>" + body + finalSection + "</w:body></w:document>"), kMain + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), QString::fromLatin1(kRel + "officeDocument"), QStringLiteral("/word/document.xml"));
    const QByteArray styles =
        "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Tinos\" w:hAnsi=\"Tinos\"/><w:sz w:val=\"23\"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr><w:spacing w:after=\"120\"/></w:pPr></w:pPrDefault></w:docDefaults>"
        "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:pPr>" + QByteArray(numberedHeadings ? "<w:numPr><w:numId w:val=\"1\"/></w:numPr>" : "") +
        "<w:keepNext/><w:outlineLvl w:val=\"0\"/></w:pPr><w:rPr><w:b/><w:sz w:val=\"32\"/></w:rPr></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:keepNext/><w:outlineLvl w:val=\"1\"/></w:pPr><w:rPr><w:b/></w:rPr></w:style>"
        "<w:style w:type=\"character\" w:styleId=\"FootnoteReference\"><w:name w:val=\"footnote reference\"/><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr></w:style>"
        "<w:style w:type=\"character\" w:styleId=\"EndnoteReference\"><w:name w:val=\"endnote reference\"/><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr></w:style>";
    w.addPart(QStringLiteral("/word/styles.xml"), xml("<w:styles " + kNamespaces + ">" + styles + "</w:styles>"), kMain + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel + "styles"), QStringLiteral("/word/styles.xml"));
    const QByteArray numbering =
        "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"multilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
        "<w:abstractNum w:abstractNumId=\"1\"><w:multiLevelType w:val=\"singleLevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/></w:lvl></w:abstractNum>"
        "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>";
    w.addPart(QStringLiteral("/word/numbering.xml"), xml("<w:numbering " + kNamespaces + ">" + numbering + "</w:numbering>"), kMain + QStringLiteral("numbering+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel + "numbering"), QStringLiteral("/word/numbering.xml"));
    auto notes = [&](const char *kind, const char *plural) {
        QByteArray out = "<w:" + QByteArray(plural) + " " + kNamespaces + "><w:" + kind + " w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:" + kind + ">"
                         "<w:" + kind + " w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:" + kind + ">";
        for (int id = 2; id <= 4; ++id)
            out += "<w:" + QByteArray(kind) + " w:id=\"" + QByteArray::number(id) + "\"><w:p><w:r><w:rPr><w:rStyle w:val=\"" + kind + "Reference\"/></w:rPr><w:" + kind + "Ref/></w:r>" + run(" " + QByteArray(kind) + " words " + QByteArray::number(id)) +
                   "</w:p></w:" + kind + ">";
        return out + "</w:" + plural + ">";
    };
    w.addPart(QStringLiteral("/word/footnotes.xml"), xml(notes("footnote", "footnotes")), kMain + QStringLiteral("footnotes+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel + "footnotes"), QStringLiteral("/word/footnotes.xml"));
    w.addPart(QStringLiteral("/word/endnotes.xml"), xml(notes("endnote", "endnotes")), kMain + QStringLiteral("endnotes+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel + "endnotes"), QStringLiteral("/word/endnotes.xml"));
    w.addPart(QStringLiteral("/word/comments.xml"),
              xml("<w:comments " + kNamespaces + "><w:comment w:id=\"0\" w:author=\"Ann\" w:date=\"2026-01-02T03:04:05Z\" w:initials=\"A\"><w:p><w:r><w:t>a comment</w:t></w:r></w:p></w:comment></w:comments>"), kMain + QStringLiteral("comments+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel + "comments"), QStringLiteral("/word/comments.xml"));
    w.addPart(QStringLiteral("/word/media/image1.png"), pngBytes(), QStringLiteral("image/png"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdPic"), QString::fromLatin1(kRel + "image"), QStringLiteral("/word/media/image1.png"));
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), xml("<w:settings " + kNamespaces + ">" + settings + "</w:settings>"), kMain + QStringLiteral("settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel + "settings"), QStringLiteral("/word/settings.xml"));
    }
    return w.finish();
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

QMap<QString, QByteArray> partsOf(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    QMap<QString, QByteArray> out;
    for (const QString &name : p.partNames())
        if (!name.contains(QLatin1String("docProps"))) out.insert(name, p.part(name));   // (the properties hold the time of the save)
    return out;
}

// Whether two saved files are the same in every part; the first part that differs is named.
QString differsIn(const QByteArray &a, const QByteArray &b)
{
    const auto pa = partsOf(a), pb = partsOf(b);
    if (pa.keys() != pb.keys()) return QStringLiteral("the parts: %1 against %2").arg(pa.keys().join(QLatin1Char(' ')), pb.keys().join(QLatin1Char(' ')));
    for (auto it = pa.cbegin(); it != pa.cend(); ++it)
        if (it.value() != pb.value(it.key())) {
            const QByteArray &x = it.value(), &y = pb.value(it.key());
            int at = 0;
            while (at < x.size() && at < y.size() && x[at] == y[at]) ++at;
            return QStringLiteral("%1 at %2: [%3] against [%4]").arg(it.key()).arg(at).arg(QString::fromUtf8(x.mid(std::max(0, at - 60), 160)), QString::fromUtf8(y.mid(std::max(0, at - 60), 160)));
        }
    return QString();
}

QString documentXml(const QByteArray &zip) { return QString::fromUtf8(partsOf(zip).value(QStringLiteral("/word/document.xml"))); }

// The XML of the paragraph that holds a word, as the file has it (a paragraph with no paragraph inside it).
QString paragraphXml(const QString &xml, const QString &word)
{
    const int at = xml.indexOf(word);
    if (at < 0) return QString();
    const int from = std::max(xml.lastIndexOf(QLatin1String("<w:p>"), at), xml.lastIndexOf(QLatin1String("<w:p "), at));
    return xml.mid(from, xml.indexOf(QLatin1String("</w:p>"), at) + 6 - from);
}

// What each paragraph shows as its number (the marker of its own, or Qt's count), by its first word.
QStringList numbersOf(WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        QString n;
        if (QTextList *list = b.textList()) {
            n = b.blockFormat().stringProperty(tp::BulletChar);
            if (n.isEmpty()) n = QString::number(list->format().start() + list->itemNumber(b));
            n.remove(QRegularExpression(QStringLiteral("[.)]$")));   // (a number is shown as its own or as Qt counts it, with the same punctuation)
        }
        out << b.text().section(QLatin1Char(' '), 0, 0) + QLatin1Char('=') + n;
    }
    return out;
}

int positionOf(WriteDoc &doc, const QString &start)
{
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        if (QString(b.text()).remove(QChar(0xFFFC)).startsWith(start)) return b.position();
    return -1;
}

// The four parts of a test document: what comes before the three sections (Alpha, Beta, Gamma) and after them. The sections are given in an order to put them in.
struct Pieces {
    QByteArray intro = marked("Intro words", "<w:pageBreakBefore/>"), alpha = head("Alpha") + marked("Alpha words"), beta = head("Beta") + para("Beta words"), gamma = head("Gamma") + marked("Gamma words"),
               tail = marked("Closing words");
    QByteArray finalSection = "<w:sectPr>" + kPageSize + "</w:sectPr>";
    bool numbered = false;
    QByteArray settings;
    QByteArray bodyIn(const QList<int> &order) const
    {
        QByteArray body = intro;
        for (int n : order) body += n == 1 ? alpha : n == 2 ? beta : gamma;
        return body + tail;
    }
    QByteArray docx(const QList<int> &order = {1, 2, 3}) const { return build(bodyIn(order), finalSection, numbered, settings); }
};

} // namespace

class SectionMoveTests : public QObject {
    Q_OBJECT

    // Moves Beta up before Alpha, or down past Gamma, and checks everything: the saved file is the one a document typed in the new order saves, the paragraphs round both places are the
    // same text of the file as before, the numbers are those of the new order, and Undo and Redo give back the file exactly.
    void moveAndCompare(const Pieces &p, bool up, bool hasTail = true)
    {
        auto doc = open(p.docx());
        QVERIFY(doc);
        const QByteArray before = doc->toDocx();
        QVERIFY(!before.isEmpty());
        const int betaAt = positionOf(*doc, QStringLiteral("Beta"));
        const int stepsBefore = doc->canUndo() ? 1 : 0;
        const QString textBefore = doc->story()->toPlainText();
        QString why;
        int movedTo = -1;
        const int target = up ? positionOf(*doc, QStringLiteral("Alpha")) : hasTail ? positionOf(*doc, QStringLiteral("Closing")) : -1;
        QVERIFY2(moveSection(*doc, betaAt, target, &why, &movedTo), qPrintable(why));
        QVERIFY2(doc->story()->findBlock(movedTo).text().remove(QChar(0xFFFC)) == QStringLiteral("Beta"), qPrintable(doc->story()->toPlainText().replace(QLatin1Char('\n'), QLatin1Char('|')) + QStringLiteral(" movedTo=%1 up=%2").arg(movedTo).arg(up)));
        const QByteArray after = doc->toDocx();
        auto oracle = open(p.docx(up ? QList<int>{2, 1, 3} : QList<int>{1, 3, 2}));
        QVERIFY(oracle);
        const QByteArray expected = oracle->toDocx();
        QVERIFY2(differsIn(after, expected).isEmpty(), qPrintable(differsIn(after, expected)));
        QCOMPARE(doc->story()->toPlainText(), oracle->story()->toPlainText());
        QVERIFY2(numbersOf(*doc) == numbersOf(*oracle), qPrintable(numbersOf(*doc).join(QLatin1Char(' ')) + QStringLiteral(" against ") + numbersOf(*oracle).join(QLatin1Char(' '))));
        QVERIFY(after != before);
        // The paragraphs round the two places, as the file has them.
        const QString was = documentXml(before), now = documentXml(after);
        for (const char *word : {"Intro words", "Alpha words", "Gamma words", "Closing words"}) {
            if (!hasTail && QByteArray(word) == "Closing words") continue;
            if (paragraphXml(was, QString::fromLatin1(word)).isEmpty()) {
                QVERIFY2(QByteArray(word) != "Intro words" && QByteArray(word) != "Closing words", word);   // (a case that replaces the paragraphs of a section has no such words)
                continue;
            }
            QCOMPARE(paragraphXml(now, QString::fromLatin1(word)), paragraphXml(was, QString::fromLatin1(word)));
        }
        // One step: Undo gives back the file, Redo the move.
        QVERIFY(doc->canUndo());
        doc->undo();
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        QCOMPARE(doc->story()->toPlainText(), textBefore);
        QCOMPARE(doc->canUndo(), stepsBefore != 0);
        QVERIFY(doc->canRedo());
        doc->redo();
        QVERIFY2(differsIn(doc->toDocx(), after).isEmpty(), qPrintable(differsIn(doc->toDocx(), after)));
        QCOMPARE(numbersOf(*doc), numbersOf(*oracle));
    }

    // A move that is refused leaves the text, the file, and the history as they were.
    void refused(const Pieces &p, const QByteArray &betaTarget, const QString &part, bool tracking = false)
    {
        Q_UNUSED(betaTarget);
        auto doc = open(p.docx());
        QVERIFY(doc);
        if (tracking) doc->setTrackChanges(true);
        const QByteArray before = doc->toDocx();
        const quint64 revision = doc->story()->revision();
        QString why;
        QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why));
        QVERIFY2(why.contains(part), qPrintable(why));
        QCOMPARE(doc->story()->revision(), revision);
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        QVERIFY(!doc->canUndo());
    }

private Q_SLOTS:
    void initTestCase() {}

    void aPlainSectionMovesUpAndDownLikeTextTypedThere()
    {
        Pieces p;
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void aSectionThatIsLastInTheDocumentMovesUp()
    {
        Pieces p;
        p.tail.clear();
        // Beta is made the last (Gamma above it first), then moved up and back.
        auto doc = open(p.docx({1, 3, 2}));
        QVERIFY(doc);
        const QByteArray before = doc->toDocx();
        QString why;
        QVERIFY2(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Gamma")), &why), qPrintable(why));
        auto oracle = open(p.docx({1, 2, 3}));
        QVERIFY2(differsIn(doc->toDocx(), oracle->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), oracle->toDocx())));
        // And down again to the very end, where no paragraph follows it.
        QVERIFY2(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), -1, &why), qPrintable(why));
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        doc->undo();
        doc->undo();
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        QVERIFY(!doc->canUndo());
        moveAndCompare(p, false, false);
    }

    void aTableWithMergedCellsAndANestedTableMoves()
    {
        Pieces p;
        p.beta = head("Beta") + para("Before the table") + tableWithMergesAndANestedTable() + para("Beta words");
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void inlineAndAnchoredPicturesMove()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + inlinePicture(11) + anchoredPicture(12) + "</w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void aDrawnShapeAndATextBoxMove()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + drawnShape(21) + textBox(22) + "</w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void footnoteAndEndnoteMarksFollowTheNewOrder()
    {
        Pieces p;
        p.alpha = head("Alpha") + "<w:p>" + run("Alpha words ") + noteRef("footnote", 2) + noteRef("endnote", 2) + "</w:p>";
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + noteRef("footnote", 3) + noteRef("endnote", 3) + "</w:p>";
        p.gamma = head("Gamma") + "<w:p>" + run("Gamma words ") + noteRef("footnote", 4) + noteRef("endnote", 4) + "</w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
        // The order the saved file numbers them in is the order of the text now.
        auto doc = open(p.docx());
        QVERIFY(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha"))));
        const QString now = documentXml(doc->toDocx());
        auto at = [&](const char *word) { return now.indexOf(QString::fromLatin1(word)); };
        QVERIFY(at("Beta words") < at("Alpha words") && at("Alpha words") < at("Gamma words"));
        QCOMPARE(partsOf(doc->toDocx()).value(QStringLiteral("/word/footnotes.xml")), partsOf(open(p.docx({2, 1, 3}))->toDocx()).value(QStringLiteral("/word/footnotes.xml")));
        QCOMPARE(partsOf(doc->toDocx()).value(QStringLiteral("/word/endnotes.xml")), partsOf(open(p.docx({2, 1, 3}))->toDocx()).value(QStringLiteral("/word/endnotes.xml")));
    }

    void aCommentRangeMoves()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta ") + "<w:commentRangeStart w:id=\"0\"/>" + run("commented words") + "<w:commentRangeEnd w:id=\"0\"/><w:r><w:commentReference w:id=\"0\"/></w:r></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void aBookmarkInsideTheSectionMoves()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p><w:bookmarkStart w:id=\"7\" w:name=\"inside\"/>" + run("Beta words") + "<w:bookmarkEnd w:id=\"7\"/></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    // The rule: a section that holds one end of a bookmark (or of a comment's range, a field, or a content control) whose other end is outside it is not moved; the person is told.
    void aBookmarkThatEndsOutsideTheSectionRefusesTheMove()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p><w:bookmarkStart w:id=\"7\" w:name=\"across\"/>" + run("Beta words") + "</w:p>";
        p.gamma = head("Gamma") + "<w:p>" + run("Gamma words") + "<w:bookmarkEnd w:id=\"7\"/></w:p>";
        refused(p, QByteArray(), QStringLiteral("bookmark"));
        // One that begins above the section and ends inside it.
        Pieces q;
        q.alpha = head("Alpha") + "<w:p><w:bookmarkStart w:id=\"8\" w:name=\"from\"/>" + run("Alpha words") + "</w:p>";
        q.beta = head("Beta") + "<w:p>" + run("Beta words") + "<w:bookmarkEnd w:id=\"8\"/></w:p>";
        refused(q, QByteArray(), QStringLiteral("bookmark"));
        // And a comment's range.
        Pieces r;
        r.beta = head("Beta") + "<w:p><w:commentRangeStart w:id=\"0\"/>" + run("Beta words") + "</w:p>";
        r.gamma = head("Gamma") + "<w:p>" + run("Gamma words") + "<w:commentRangeEnd w:id=\"0\"/><w:r><w:commentReference w:id=\"0\"/></w:r></w:p>";
        refused(r, QByteArray(), QStringLiteral("comment"));
    }

    void aFieldAndCrossReferencesToHeadingsInsideAndOutsideMove()
    {
        Pieces p;
        p.alpha = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"_RefAlpha\"/>" + run("Alpha") + "<w:bookmarkEnd w:id=\"1\"/></w:p>" + marked("Alpha words");
        p.beta = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"2\" w:name=\"_RefBeta\"/>" + run("Beta") + "<w:bookmarkEnd w:id=\"2\"/></w:p>"
                 "<w:p>" + run("Beta words see ") + "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> REF _RefAlpha \\h </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + run("Alpha") +
                 "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>" + run(" and ") + "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> REF _RefBeta \\h </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" +
                 run("Beta") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple></w:p>";
        p.gamma = head("Gamma") + "<w:p>" + run("Gamma words see ") + "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> REF _RefBeta \\h </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" +
                  run("Beta") + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void aNumberedHeadingAndAListThatContinuesAcrossTheBoundaryAreRenumbered()
    {
        Pieces p;
        p.numbered = true;
        const QByteArray item = "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"2\"/></w:numPr>";
        p.alpha = head("Alpha") + para("Alpha first", item) + para("Alpha second", item);
        p.beta = head("Beta") + para("Beta third", item) + para("Beta fourth", item);
        p.gamma = head("Gamma") + para("Gamma fifth", item);
        moveAndCompare(p, true);
        moveAndCompare(p, false);
        // The numbers are those of the text typed in that order (the oracle's, compared above), and they are not the ones it had.
        auto doc = open(p.docx());
        const QStringList was = numbersOf(*doc);
        QVERIFY(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha"))));
        QVERIFY(numbersOf(*doc) != was);
        QCOMPARE(doc->story()->blockCount(), open(p.docx())->story()->blockCount());
    }

    // Tracked changes in the section go with it (the section is moved, not edited); what the move records: nothing, and with Track Changes on the move is refused, since a cut and a paste would
    // be a deletion and an insertion of the whole section (every table, picture, and note in it twice), which this does not make.
    void trackedChangesInsideTheSectionMoveWithItAndTrackingRefusesTheMove()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + "<w:ins w:id=\"30\" w:author=\"Ann\" w:date=\"2026-01-02T03:04:05Z\"><w:r><w:t>added</w:t></w:r></w:ins>"
                 "<w:del w:id=\"31\" w:author=\"Bob\" w:date=\"2026-01-02T03:04:05Z\"><w:r><w:delText>removed</w:delText></w:r></w:del></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
        refused(p, QByteArray(), QStringLiteral("Track Changes"), true);
    }

    void aContentControlMoves()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + "<w:sdt><w:sdtPr><w:alias w:val=\"Name\"/><w:tag w:val=\"name\"/><w:text/></w:sdtPr><w:sdtContent>" + run("the value") + "</w:sdtContent></w:sdt></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    void aPageBreakBeforeTheHeadingStaysWithTheTextItIsIn()
    {
        Pieces p;
        // A paragraph of a page break only, at the end of Alpha (it stays with Alpha), and a heading that says its page starts there (it goes with Beta).
        p.alpha = head("Alpha") + marked("Alpha words") + "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>";
        p.beta = head("Beta", 1, "<w:pageBreakBefore/>") + para("Beta words");
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    // The rule: the page setup of a section break describes the text before it, and goes with the paragraph that has it. A run that holds breaks moves only when it is whole sections, from
    // the start of one to the break that ends one, and goes between sections; the text then keeps the page setup it had.
    void sectionBreaksInsideAndAtTheEndOfTheMovedRunKeepTheirPageSetup()
    {
        auto setup = [](const char *orientation, int w, int h) { return "<w:sectPr><w:pgSz w:w=\"" + QByteArray::number(w) + "\" w:h=\"" + QByteArray::number(h) + "\" w:orient=\"" + orientation + "\"/>"
                                                                           "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>"; };
        Pieces p;
        p.intro = para("Intro words", "<w:jc w:val=\"right\"/>" + setup("portrait", 12240, 15840));
        // Beta holds a break in its middle (landscape) and one at its end (A4); Gamma ends in one as well, and Alpha's text is in the last section (Letter, the document's final one).
        p.alpha = head("Alpha") + marked("Alpha words");
        p.beta = head("Beta") + para("Beta words") + para("Beta middle", setup("landscape", 15840, 12240)) + para("Beta more") + para("Beta end", setup("portrait", 11906, 16838));
        p.gamma = head("Gamma") + para("Gamma words", setup("portrait", 12000, 15000));
        // Gamma is moved up past Beta: both are whole sections, between sections.
        {
            auto doc = open(p.docx({2, 3, 1}));
            QVERIFY(doc);
            const QByteArray before = doc->toDocx();
            QString why;
            QVERIFY2(moveSection(*doc, positionOf(*doc, QStringLiteral("Gamma")), positionOf(*doc, QStringLiteral("Beta")), &why), qPrintable(why));
            auto oracle = open(p.docx({3, 2, 1}));
            QVERIFY2(differsIn(doc->toDocx(), oracle->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), oracle->toDocx())));
            QCOMPARE(doc->sectionCount(), oracle->sectionCount());
            for (int i = 0; i < doc->sectionCount(); ++i) QCOMPARE(doc->sectionProps(i).pageW, oracle->sectionProps(i).pageW);
            doc->undo();
            QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
            QCOMPARE(doc->sectionCount(), open(p.docx({2, 3, 1}))->sectionCount());
        }
        // Beta, with its two breaks, moves down past Gamma and up again (it starts after a break and ends in one, and goes between breaks).
        {
            auto doc = open(p.docx({2, 3, 1}));
            QString why;
            QVERIFY2(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why), qPrintable(why));
            auto oracle = open(p.docx({3, 2, 1}));
            QVERIFY2(differsIn(doc->toDocx(), oracle->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), oracle->toDocx())));
        }
        // Refused: a break inside a run that does not end in one (its tail would take the setup of the text it lands among), and a run that goes into the middle of a section.
        {
            Pieces q = p;
            q.beta = head("Beta") + para("Beta words") + para("Beta middle", setup("landscape", 15840, 12240)) + para("Beta end");
            auto doc = open(q.docx({2, 3, 1}));
            const QByteArray before = doc->toDocx();
            QString why;
            QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why));
            QVERIFY2(why.contains(QLatin1String("page setup")), qPrintable(why));
            QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
            QVERIFY(!doc->canUndo());
            // Gamma (it ends in a break) into the middle of Beta's section (before "Beta more").
            QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Gamma")), positionOf(*doc, QStringLiteral("Beta end")), &why));
            QVERIFY2(why.contains(QLatin1String("page setup")), qPrintable(why));
        }
    }

    void aSectionThatEndsInATableIsNotLeftLastAndAHeadingInATableIsNoSection()
    {
        Pieces p;
        p.tail.clear();
        p.beta = head("Beta") + tableWithMergesAndANestedTable();
        auto doc = open(p.docx());
        QVERIFY(doc);
        const QByteArray before = doc->toDocx();
        QString why;
        QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), -1, &why));   // (it would be the last thing, with no paragraph after the table)
        QVERIFY2(why.contains(QLatin1String("table")), qPrintable(why));
        QVERIFY(!doc->canUndo());
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        // A paragraph in a cell is not a heading to move, and a move whose place is inside a table is refused.
        SectionRange range;
        QVERIFY(!sectionAt(*doc, positionOf(*doc, QStringLiteral("wide")), &range));
        QVERIFY(!moveBlocks(*doc, positionOf(*doc, QStringLiteral("Alpha")), positionOf(*doc, QStringLiteral("Alpha words")), positionOf(*doc, QStringLiteral("tall")), &why));
        QVERIFY2(why.contains(QLatin1String("table")), qPrintable(why));
        QVERIFY(!doc->canUndo());
    }

    void aProtectedDocumentRefuses()
    {
        Pieces p;
        p.settings = "<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\"/>";
        auto doc = open(p.docx());
        QVERIFY(doc);
        QVERIFY(doc->protection() && doc->protection()->enforced);
        const QByteArray before = doc->toDocx();
        QString why;
        QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why));
        QVERIFY2(why.contains(QLatin1String("protected")), qPrintable(why));
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        QVERIFY(!doc->canUndo());
    }

    // ---- the Navigation pane: dragging a heading, and the commands

    // A window with the pane open on a document of three sections, the middle one with a heading of its own under it.
    static Pieces panePieces(const QByteArray &settings = QByteArray())
    {
        Pieces p;
        p.beta = head("Beta") + para("Beta words") + head("Beta sub", 2) + para("Beta sub words");
        p.tail.clear();   // (text after the last heading is in that heading's section)
        p.settings = settings;
        return p;
    }
    static QByteArray paneDocx(const QByteArray &settings = QByteArray()) { return panePieces(settings).docx(); }
    static QTreeWidgetItem *rowOf(QTreeWidget *tree, const QString &text)
    {
        for (QTreeWidgetItemIterator it(tree); *it; ++it)
            if ((*it)->text(0) == text) return *it;
        return nullptr;
    }
    // A real press, move, or release of the left button on the tree's viewport, at a place `third` thirds down the row (1 is the upper third, 5 the lower).
    static QPoint inRow(QTreeWidget *tree, QTreeWidgetItem *row, int third) { const QRect r = tree->visualItemRect(row); return QPoint(r.left() + 20, r.top() + r.height() * third / 6); }
    static void mouse(QWidget *viewport, QEvent::Type type, const QPoint &at, Qt::MouseButtons buttons)
    {
        QMouseEvent e(type, QPointF(at), QPointF(viewport->mapToGlobal(at)), type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton, buttons, Qt::NoModifier);
        QApplication::sendEvent(viewport, &e);
    }
    static void press(QTreeWidget *t, const QPoint &at) { mouse(t->viewport(), QEvent::MouseButtonPress, at, Qt::LeftButton); }
    static void move(QTreeWidget *t, const QPoint &at) { mouse(t->viewport(), QEvent::MouseMove, at, Qt::LeftButton); }
    static void release(QTreeWidget *t, const QPoint &at) { mouse(t->viewport(), QEvent::MouseButtonRelease, at, Qt::NoButton); }
    static void openPane(WriteWindow &w, const QByteArray &docx)
    {
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(docx), QString());
        w.act("view.navigationPane")->trigger();
        QVERIFY(w.navigationPane());
    }

    void aHeadingDraggedInThePaneMovesWithItsSectionUnderTheLineAndIsOneUndoStep()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        WriteDoc *doc = w.document();
        NavigationPane *pane = w.navigationPane();
        QTreeWidget *tree = pane->headings();
        const QByteArray before = doc->toDocx();
        QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *alpha = rowOf(tree, QStringLiteral("Alpha"));
        QVERIFY(beta && alpha);
        press(tree, inRow(tree, beta, 3));
        QCOMPARE(pane->draggedHeading(), -1);   // (a press is no drag)
        move(tree, inRow(tree, beta, 3) + QPoint(0, 2));
        QCOMPARE(pane->draggedHeading(), -1);   // (nor is a move of a pixel or two)
        move(tree, inRow(tree, alpha, 1));
        QCOMPARE(pane->draggedHeading(), 1);
        QCOMPARE(pane->dropBefore(), 0);
        QVERIFY(pane->dropLine()->isVisible());
        QCOMPARE(pane->dropLine()->geometry().center().y(), tree->visualItemRect(alpha).top());
        QVERIFY(!doc->canUndo());   // (nothing is changed while the pointer moves)
        release(tree, inRow(tree, alpha, 1));
        QVERIFY(!pane->dropLine()->isVisible());
        QCOMPARE(pane->draggedHeading(), -1);
        const Pieces p = panePieces();
        auto oracle = open(p.docx({2, 1, 3}));
        QVERIFY2(differsIn(doc->toDocx(), oracle->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), oracle->toDocx())));
        QCOMPARE(tree->topLevelItem(0)->text(0), QStringLiteral("Beta"));
        QCOMPARE(tree->topLevelItem(0)->childCount(), 1);   // (Beta sub went with it)
        doc->undo();
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        QVERIFY(!doc->canUndo());
        // Dragged down past the last row: the end of the text (the pane looks at the headings again a moment after the undo).
        const int scans = pane->scanCountForTest();
        pane->refresh();
        QTRY_VERIFY(pane->scanCountForTest() > scans);
        QTreeWidgetItem *gamma = rowOf(tree, QStringLiteral("Gamma"));
        beta = rowOf(tree, QStringLiteral("Beta"));
        QVERIFY(pane->headings()->visualItemRect(gamma).isValid());
        press(tree, inRow(tree, beta, 3));
        move(tree, inRow(tree, gamma, 5));
        QCOMPARE(pane->dropBefore(), 4);   // (the number of rows: the end)
        release(tree, inRow(tree, gamma, 5));
        auto down = open(p.docx({1, 3, 2}));
        QVERIFY2(differsIn(doc->toDocx(), down->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), down->toDocx())));
    }

    void droppingOnTheSectionItselfOrInsideItDoesNothing()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        WriteDoc *doc = w.document();
        QTreeWidget *tree = w.navigationPane()->headings();
        const QByteArray before = doc->toDocx();
        QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *sub = rowOf(tree, QStringLiteral("Beta sub")), *gamma = rowOf(tree, QStringLiteral("Gamma"));
        for (const auto &spot : {std::make_pair(beta, 1), std::make_pair(beta, 5), std::make_pair(sub, 1), std::make_pair(sub, 5), std::make_pair(gamma, 1)}) {   // (the last is just under the section: where it is)
            press(tree, inRow(tree, beta, 3));
            move(tree, inRow(tree, gamma, 3));   // (a drag is begun, then taken to the place)
            move(tree, inRow(tree, spot.first, spot.second));
            QCOMPARE(w.navigationPane()->draggedHeading(), 1);
            QCOMPARE(w.navigationPane()->dropBefore(), -1);
            QVERIFY(!w.navigationPane()->dropLine()->isVisible());
            release(tree, inRow(tree, spot.first, spot.second));
            QVERIFY(!doc->canUndo());
            QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        }
    }

    void escapeCancelsTheDrag()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        WriteDoc *doc = w.document();
        NavigationPane *pane = w.navigationPane();
        QTreeWidget *tree = pane->headings();
        const QByteArray before = doc->toDocx();
        QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *alpha = rowOf(tree, QStringLiteral("Alpha"));
        tree->setFocus();
        press(tree, inRow(tree, beta, 3));
        move(tree, inRow(tree, alpha, 1));
        QVERIFY(pane->dropLine()->isVisible());
        QTest::keyClick(tree, Qt::Key_Escape);
        QCOMPARE(pane->draggedHeading(), -1);
        QVERIFY(!pane->dropLine()->isVisible());
        release(tree, inRow(tree, alpha, 1));
        QVERIFY(!doc->canUndo());
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
    }

    // 2,000 headings with their text: a move of the pointer looks at the rows under it and never at the text or the layout; a drop works on the paragraphs of the section moved, wherever it goes.
    void aDragInALongDocumentLooksAtRowsOnlyAndADropWorksOnTheSectionOnly()
    {
        QByteArray body = marked("Intro words");
        for (int i = 0; i < 2000; ++i) {
            body += head("Heading " + QByteArray::number(i));
            for (int k = 0; k < 4; ++k) body += para("Words of heading " + QByteArray::number(i) + " paragraph " + QByteArray::number(k) + QByteArray(" and some more words to fill the line").repeated(6));
        }
        WriteWindow w;
        openPane(w, build(body));
        WriteDoc *doc = w.document();
        NavigationPane *pane = w.navigationPane();
        QTreeWidget *tree = pane->headings();
        QCOMPARE(tree->topLevelItemCount(), 2000);
        const int blocks = doc->story()->blockCount();
        QVERIFY(blocks > 10000);
        tree->scrollToItem(rowOf(tree, QStringLiteral("Heading 1999")));
        QTreeWidgetItem *from = rowOf(tree, QStringLiteral("Heading 1999"));
        QVERIFY(from);
        const int scans = pane->scanCountForTest(), rebuilds = pane->rebuildCountForTest();
        const quint64 revision = doc->story()->revision();
        const qint64 rows = pane->dragRowsLookedAtForTest(), moves = pane->dragMovesForTest();
        press(tree, inRow(tree, from, 3));
        const QRect view = tree->viewport()->rect();
        int made = 0;
        for (int y = tree->visualItemRect(from).center().y(); y > view.top() + 20; y -= 3, ++made) move(tree, QPoint(20, y));
        QVERIFY(made > 20);
        QCOMPARE(pane->dragMovesForTest() - moves, qint64(made));
        QVERIFY2(pane->dragRowsLookedAtForTest() - rows <= 2 * made + 2, qPrintable(QString::number(pane->dragRowsLookedAtForTest() - rows)));   // (at most the row under the pointer and the one below it)
        QCOMPARE(pane->scanCountForTest(), scans);
        QCOMPARE(pane->rebuildCountForTest(), rebuilds);
        QCOMPARE(doc->story()->revision(), revision);
        QVERIFY(pane->dropBefore() >= 0);
        const int target = pane->dropBefore();
        QVERIFY(target < 1999 - 5);   // (a long way up)
        release(tree, QPoint(20, tree->viewport()->rect().top() + 24));
        QVERIFY(doc->canUndo());
        QCOMPARE(doc->story()->blockCount(), blocks);
        QCOMPARE(doc->story()->findBlockByNumber(1 + target * 5).text(), QStringLiteral("Heading 1999"));
        QVERIFY2(pane->lastDropWorkForTest() > 0 && pane->lastDropWorkForTest() <= 40, qPrintable(QString::number(pane->lastDropWorkForTest())));   // (the section's five paragraphs, a few more at the seams: 10,000 paragraphs are not looked at)
    }

    void commandsMoveTheSectionTheCaretIsInPastItsNeighbor()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        WriteDoc *doc = w.document();
        QVERIFY(w.act("view.moveSectionUp") && w.act("view.moveSectionDown"));
        QCOMPARE(w.act("view.moveSectionUp")->shortcut().toString(QKeySequence::PortableText), QStringLiteral("Ctrl+Alt+Up"));
        const QByteArray before = doc->toDocx();
        const Pieces p = panePieces();
        QTextCursor c(doc->story());
        c.setPosition(positionOf(*doc, QStringLiteral("Beta sub words")));   // (in the text under the sub-heading: that section is the one that moves, and it has no neighbor of its level)
        w.view()->showCursor(c);
        w.act("view.moveSectionUp")->trigger();
        QVERIFY(!doc->canUndo());
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("same level")), qPrintable(w.statusBar()->currentMessage()));
        c.setPosition(positionOf(*doc, QStringLiteral("Beta words")));
        w.view()->showCursor(c);
        w.act("view.moveSectionUp")->trigger();
        QVERIFY2(differsIn(doc->toDocx(), open(p.docx({2, 1, 3}))->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), open(p.docx({2, 1, 3}))->toDocx())));
        QCOMPARE(w.view()->cursor().block().text(), QStringLiteral("Beta"));   // (the caret goes with the section)
        w.act("view.moveSectionDown")->trigger();
        w.act("view.moveSectionDown")->trigger();
        QVERIFY2(differsIn(doc->toDocx(), open(p.docx({1, 3, 2}))->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), open(p.docx({1, 3, 2}))->toDocx())));
        doc->undo();
        doc->undo();
        doc->undo();
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        // The pane's own commands (the context menu's) are those too, for the row they are on.
        QTreeWidget *tree = w.navigationPane()->headings();
        QVERIFY(w.navigationPane()->moveSectionBy(rowOf(tree, QStringLiteral("Gamma")), true));
        QVERIFY2(differsIn(doc->toDocx(), open(p.docx({1, 3, 2}))->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), open(p.docx({1, 3, 2}))->toDocx())));
        QVERIFY(!w.navigationPane()->moveSectionBy(rowOf(tree, QStringLiteral("Alpha")), true));
    }

    void aReadOnlyOrProtectedWindowRefusesTheDragAndTheCommands()
    {
        for (int kind = 0; kind < 2; ++kind) {
            WriteWindow w;
            openPane(w, paneDocx(kind ? QByteArray("<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\"/>") : QByteArray()));
            if (!kind) w.setReadOnly(true);
            QVERIFY(w.isReadOnly());
            WriteDoc *doc = w.document();
            QTreeWidget *tree = w.navigationPane()->headings();
            const QByteArray before = doc->toDocx();
            QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *alpha = rowOf(tree, QStringLiteral("Alpha"));
            press(tree, inRow(tree, beta, 3));
            move(tree, inRow(tree, alpha, 1));
            QCOMPARE(w.navigationPane()->draggedHeading(), -1);
            QVERIFY(!w.navigationPane()->dropLine()->isVisible());
            release(tree, inRow(tree, alpha, 1));
            QTextCursor c(doc->story());
            c.setPosition(positionOf(*doc, QStringLiteral("Beta words")));
            w.view()->showCursor(c);
            w.act("view.moveSectionUp")->trigger();
            QVERIFY(!w.navigationPane()->moveSectionBy(beta, true));
            QVERIFY(!doc->canUndo());
            QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        }
    }

    // Screenshots for looking at (only with JW_SHOT_DIR set): in the middle of a drag, with the line, and after the drop.
    void screenshots()
    {
        const QString folder = qEnvironmentVariable("JW_SHOT_DIR");
        if (folder.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        applyUiTheme(1);
        WriteWindow w;
        openPane(w, paneDocx());
        QTreeWidget *tree = w.navigationPane()->headings();
        QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *alpha = rowOf(tree, QStringLiteral("Alpha"));
        press(tree, inRow(tree, beta, 3));
        move(tree, inRow(tree, alpha, 1));
        QDir().mkpath(folder);
        QVERIFY(w.grab().save(folder + QStringLiteral("/section-drag-dragging.png")));
        release(tree, inRow(tree, alpha, 1));
        QVERIFY(w.grab().save(folder + QStringLiteral("/section-drag-dropped.png")));
    }

    void aDestinationInTheSectionItselfDoesNothing()
    {
        Pieces p;
        auto doc = open(p.docx());
        const int beta = positionOf(*doc, QStringLiteral("Beta"));
        QString why = QStringLiteral("x");
        QVERIFY(!moveSection(*doc, beta, beta, &why));
        QVERIFY(why.isEmpty());
        QVERIFY(!moveSection(*doc, beta, positionOf(*doc, QStringLiteral("Beta words")), &why));
        QVERIFY(!moveSection(*doc, beta, positionOf(*doc, QStringLiteral("Gamma")), &why));   // (right after it: where it is)
        QVERIFY(!doc->canUndo());
    }
};

QTEST_MAIN(SectionMoveTests)
#include "tst_write_secmove.moc"
