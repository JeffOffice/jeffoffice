// Tests of moving a section (a heading with what is under it): the move keeps every part of the saved file as if the section had been typed at the new place, and nothing else changes.

#include "../isolatesettings.h"
#include "app/icons.h"
#include "app/keymap.h"
#include "app/navpane.h"
#include "app/reviewpane.h"
#include "app/ribbon.h"
#include "app/pageview.h"
#include "app/theme.h"
#include "app/writeinfo.h"
#include "app/writeoutlineview.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/blockmove.h"
#include "core/fonts.h"
#include "core/editops.h"
#include "core/items.h"
#include "core/reviewops.h"
#include "core/sectionmove.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "core/trackpolicy.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QFrame>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QScrollBar>
#include <QStatusBar>
#include <QTreeWidget>
#include <QImage>
#include <QLineEdit>
#include <QRegularExpression>
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <QToolButton>
#include <tuple>

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

const QString kBreakWhy = QStringLiteral("This section includes a section break, and moving it would change the page layout of other text, so it can't be moved.");
const QString kFrameWhy = QStringLiteral("A section that holds endnotes or a text frame can't be moved.");
const QString kEndnotesWhy = QStringLiteral("Sections can't be moved while endnotes are collected at the end of each section. Put the endnotes at the end of the document first.");

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
    // The run's settings are its own, and none reached the person's (checked at the end).
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        jp::initCore();
    }

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

    // The rule: a section that holds one end of a bookmark (or of a comment's range, a field, or a content control) whose other end is outside it is not moved; the person is told. Each case is a test
    // of its own, so that one that fails does not hide the others.
    void aBookmarkThatBeginsInTheSectionAndEndsBelowItRefusesTheMove()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p><w:bookmarkStart w:id=\"7\" w:name=\"across\"/>" + run("Beta words") + "</w:p>";
        p.gamma = head("Gamma") + "<w:p>" + run("Gamma words") + "<w:bookmarkEnd w:id=\"7\"/></w:p>";
        refused(p, QByteArray(), QStringLiteral("bookmark"));
    }
    void aBookmarkThatBeginsAboveTheSectionAndEndsInsideItRefusesTheMove()
    {
        Pieces q;
        q.alpha = head("Alpha") + "<w:p><w:bookmarkStart w:id=\"8\" w:name=\"from\"/>" + run("Alpha words") + "</w:p>";
        q.beta = head("Beta") + "<w:p>" + run("Beta words") + "<w:bookmarkEnd w:id=\"8\"/></w:p>";
        refused(q, QByteArray(), QStringLiteral("bookmark"));
    }
    void aCommentRangeThatEndsOutsideTheSectionRefusesTheMove()
    {
        Pieces r;
        r.beta = head("Beta") + "<w:p><w:commentRangeStart w:id=\"0\"/>" + run("Beta words") + "</w:p>";
        r.gamma = head("Gamma") + "<w:p>" + run("Gamma words") + "<w:commentRangeEnd w:id=\"0\"/><w:r><w:commentReference w:id=\"0\"/></w:r></w:p>";
        refused(r, QByteArray(), QStringLiteral("comment"));
    }

    static QByteArray fieldPart(const char *type, const QByteArray &instr = QByteArray())
    {
        return "<w:r><w:fldChar w:fldCharType=\"" + QByteArray(type) + "\"/></w:r>" + (instr.isEmpty() ? QByteArray() : "<w:r><w:instrText xml:space=\"preserve\"> " + instr + " </w:instrText></w:r>");
    }
    // A field that begins in the section and ends below it, and one that begins above it and ends inside it.
    void aFieldThatBeginsInTheSectionAndEndsBelowItRefusesTheMove()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + fieldPart("begin", "SEQ Figure") + fieldPart("separate") + run("1") + "</w:p>";
        p.gamma = head("Gamma") + "<w:p>" + run("Gamma words") + fieldPart("end") + "</w:p>";
        refused(p, QByteArray(), QStringLiteral("field"));
    }
    void aFieldThatBeginsAboveTheSectionAndEndsInsideItRefusesTheMove()
    {
        Pieces p;
        p.alpha = head("Alpha") + "<w:p>" + run("Alpha words ") + fieldPart("begin", "SEQ Figure") + fieldPart("separate") + run("1") + "</w:p>";
        p.beta = head("Beta") + "<w:p>" + run("Beta words") + fieldPart("end") + "</w:p>";
        refused(p, QByteArray(), QStringLiteral("field"));
    }
    void aFieldThatIsWholeInsideTheSectionMovesWithIt()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + fieldPart("begin", "SEQ Figure") + fieldPart("separate") + run("1") + fieldPart("end") + "</w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }
    // A content control with ends in two sections is a wrapper around whole paragraphs (one in a paragraph can't have an end in another): it refuses the move whichever way the section goes.
    void aContentControlWithAnEndOutsideTheSectionRefusesTheMoveUpAndDown()
    {
        Pieces p;
        p.beta = head("Beta") + wrapOpen(0) + para("Beta words");
        p.gamma = head("Gamma") + wrapClose(0) + marked("Gamma words");
        refused(p, QByteArray(), QStringLiteral("content control"));
        auto doc = open(p.docx());
        const QByteArray before = doc->toDocx();
        QString why;
        QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Closing")), &why));
        QVERIFY2(why.contains(QLatin1String("content control")), qPrintable(why));
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        QVERIFY(!doc->canUndo());
    }

    // The words of every line of every box of every page, as the pages lay them out: the numbers of the footnotes are in them, where the page puts them.
    static QStringList pageLines(WriteDoc &doc)
    {
        QStringList out;
        for (int known = -1; !doc.pager().complete() && doc.pager().pagesKnown() != known;) {
            known = doc.pager().pagesKnown();
            doc.pager().paginate(known);
        }
        for (int page = 0; page < doc.pager().pageCount(); ++page) {
            const auto made = doc.pager().materialize(page);
            if (!made) continue;
            for (int box = 0; box < made->boxes.size(); ++box) {
                const auto *item = dynamic_cast<const TextItem *>(made->doc->item(made->boxes[box]));
                if (!item) continue;
                FrameSpec fs;
                fs.size = item->rect.size();
                fs.insets = QMarginsF(0, 0, 0, 0);
                fs.hyphenate = false;
                StoryLayout layout;
                layout.build(made->doc->story(item->storyId)->doc.get(), {fs}, doc.flow().env);
                for (const auto &l : layout.lineInfo(0)) out << QStringLiteral("%1.%2: %3").arg(page).arg(box).arg(QString(l.text).remove(QChar(0x2060)).trimmed());
            }
        }
        return out;
    }
    // Footnote numbers that begin again in each section, with a heading between the section breaks: after a section moves, the numbers are those of a document typed in the new order.
    void footnoteNumbersThatRestartInEachSectionAreThoseOfTheNewOrder()
    {
        const QByteArray brk = "<w:sectPr>" + kPageSize + "</w:sectPr>";
        Pieces p;
        p.settings = "<w:footnotePr><w:numRestart w:val=\"eachSect\"/></w:footnotePr>";
        p.intro = para("Intro words", brk);
        p.alpha = head("Alpha") + "<w:p>" + run("Alpha words ") + noteRef("footnote", 2) + run(" and ") + noteRef("footnote", 3) + "</w:p>" + para("Alpha end", brk);
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + noteRef("footnote", 4) + "</w:p>" + para("Beta end", brk);
        p.gamma = head("Gamma") + para("Gamma words", brk);
        auto moved = [&](bool up, QString *why) {
            auto doc = open(p.docx());
            const bool ok = moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), up ? positionOf(*doc, QStringLiteral("Alpha")) : positionOf(*doc, QStringLiteral("Closing")), why);
            return std::make_pair(ok, pageLines(*doc));
        };
        const QStringList wasLines = pageLines(*open(p.docx()));
        QVERIFY(wasLines.size() > 6);
        for (const bool up : {true, false}) {
            QString why;
            const auto [ok, lines] = moved(up, &why);
            QVERIFY2(ok, qPrintable(why));
            const QStringList typed = pageLines(*open(p.docx(up ? QList<int>{2, 1, 3} : QList<int>{1, 3, 2})));
            QCOMPARE(lines, typed);
            QVERIFY(lines != wasLines);   // (the notes of the sections now follow the new order)
        }
        moveAndCompare(p, true);
        moveAndCompare(p, false);
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

    // Tracked changes in the section go with it (the section is moved, not edited); what the move records: nothing (a cut and a paste would be a deletion and an insertion of the whole section, every
    // table, picture, and note in it twice, which this does not make), so with Track Changes on the move is made as with it off, and the document says it was not recorded.
    void trackedChangesInsideTheSectionMoveWithItAndTrackingMovesItAndSaysSo()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + "<w:ins w:id=\"30\" w:author=\"Ann\" w:date=\"2026-01-02T03:04:05Z\"><w:r><w:t>added</w:t></w:r></w:ins>"
                 "<w:del w:id=\"31\" w:author=\"Bob\" w:date=\"2026-01-02T03:04:05Z\"><w:r><w:delText>removed</w:delText></w:r></w:del></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
        // With Track Changes on the move is made as with it off (the text goes as it is), and the document says it was not recorded.
        QByteArray result[2];
        for (const bool tracking : {true, false}) {
            auto doc = open(p.docx());
            QVERIFY(doc);
            doc->setTrackChanges(tracking);
            QSignalSpy said(doc.get(), &WriteDoc::untrackedChange);
            QString why;
            QVERIFY2(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why), qPrintable(why));
            QCOMPARE(said.count(), tracking ? 1 : 0);
            QVERIFY(doc->canUndo());
            doc->setTrackChanges(false);   // (the setting is the one thing that differs)
            result[tracking ? 0 : 1] = doc->toDocx();
        }
        QVERIFY2(differsIn(result[0], result[1]).isEmpty(), qPrintable(differsIn(result[0], result[1])));
    }

    void aContentControlMoves()
    {
        Pieces p;
        p.beta = head("Beta") + "<w:p>" + run("Beta words ") + "<w:sdt><w:sdtPr><w:alias w:val=\"Name\"/><w:tag w:val=\"name\"/><w:text/></w:sdtPr><w:sdtContent>" + run("the value") + "</w:sdtContent></w:sdt></w:p>";
        moveAndCompare(p, true);
        moveAndCompare(p, false);
    }

    // Block-level wrappers (a block content control, a custom-XML block, a block-level tracked insertion) around whole paragraphs. One that is wholly inside a run goes with its paragraphs, and
    // the saved file is the one a document typed in the new order saves; one that crosses the edge of a run refuses the move.
    static QByteArray wrapOpen(int kind)
    {
        return kind == 0 ? QByteArray("<w:sdt><w:sdtPr><w:alias w:val=\"Box\"/><w:id w:val=\"99\"/></w:sdtPr><w:sdtContent>")
             : kind == 1 ? QByteArray("<w:customXml w:element=\"x\">")
                         : QByteArray("<w:ins w:id=\"7\" w:author=\"Ann\" w:date=\"2026-01-02T03:04:05Z\">");
    }
    static QByteArray wrapClose(int kind) { return kind == 0 ? QByteArray("</w:sdtContent></w:sdt>") : kind == 1 ? QByteArray("</w:customXml>") : QByteArray("</w:ins>"); }
    static QByteArray wrapped(int kind, const QByteArray &paragraphs) { return wrapOpen(kind) + paragraphs + wrapClose(kind); }

    void wrapperKindMoves(int kind)
    {
        for (int where = 1; where <= 3; ++where)
            for (int whole = 0; whole < 2; ++whole)
                for (int tail = 0; tail < 2; ++tail) {
                    // Whole: the wrapper holds the heading and its text; not: it holds one more paragraph after them.
                    Pieces p;
                    QByteArray &section = where == 1 ? p.alpha : where == 2 ? p.beta : p.gamma;
                    section = whole ? wrapped(kind, section) : section + wrapped(kind, para("Wrapped words"));
                    if (!tail) p.tail.clear();
                    const QByteArray label = "wrapper " + QByteArray::number(kind) + " in section " + QByteArray::number(where) + (whole ? " (whole)" : " (after)") + (tail ? "" : " (no tail)");
                    if (tail) moveAndCompare(p, true);
                    if (!QTest::currentTestFailed()) moveAndCompare(p, false, tail);
                    if (QTest::currentTestFailed()) {
                        qWarning("failed for the %s", label.constData());
                        return;
                    }
                }
    }
    void wrapperKindRefuses(int kind)
    {
        // Across the edge between Alpha and Beta (above the moved run's start), across the edge between Beta and Gamma (its end), and from the text before Alpha into Alpha.
        {
            Pieces p;
            p.alpha = head("Alpha") + wrapOpen(kind) + marked("Alpha words");
            p.beta = head("Beta") + wrapClose(kind) + para("Beta words");
            refused(p, QByteArray(), QStringLiteral("content control"));
        }
        if (QTest::currentTestFailed()) return;
        {
            Pieces p;
            p.beta = head("Beta") + wrapOpen(kind) + para("Beta words");
            p.gamma = head("Gamma") + wrapClose(kind) + marked("Gamma words");
            refused(p, QByteArray(), QStringLiteral("content control"));
        }
        if (QTest::currentTestFailed()) return;
        {
            Pieces p;
            p.intro = marked("Intro words") + wrapOpen(kind) + marked("Intro two");
            p.alpha = head("Alpha") + wrapClose(kind) + marked("Alpha words");
            refused(p, QByteArray(), QStringLiteral("content control"));
        }
    }

    void aBlockContentControlAroundWholeParagraphsMovesWithThemOrRefuses() { wrapperKindMoves(0); if (!QTest::currentTestFailed()) wrapperKindRefuses(0); }
    void aCustomXmlBlockAroundWholeParagraphsMovesWithThemOrRefuses() { wrapperKindMoves(1); if (!QTest::currentTestFailed()) wrapperKindRefuses(1); }
    void aTrackedInsertionBlockAroundWholeParagraphsMovesWithThemOrRefuses() { wrapperKindMoves(2); if (!QTest::currentTestFailed()) wrapperKindRefuses(2); }

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
            QCOMPARE(why, kBreakWhy);
            QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
            QVERIFY(!doc->canUndo());
            // Gamma (it ends in a break) into the middle of Beta's section (before "Beta more").
            QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Gamma")), positionOf(*doc, QStringLiteral("Beta end")), &why));
            QCOMPARE(why, kBreakWhy);
        }
    }

    // What the person is told when a section is refused for what it holds or where the document keeps its notes: plain words that are true of that section.
    void theRefusalsSayWhyInPlainWords()
    {
        {   // A page break of the section kind in a run that does not end in one.
            Pieces p;
            p.beta = head("Beta") + para("Beta words", "<w:sectPr><w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/></w:sectPr>") + para("Beta end");
            auto doc = open(p.docx());
            QString why;
            QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why));
            QCOMPARE(why, kBreakWhy);
        }
        {   // A paragraph in a frame of text.
            Pieces p;
            p.beta = head("Beta") + "<w:p><w:pPr><w:framePr w:w=\"2000\" w:h=\"1000\" w:hRule=\"exact\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\"/></w:pPr>" + run("framed words") + "</w:p>" + para("Beta words");
            auto doc = open(p.docx());
            QString why;
            QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why));
            QCOMPARE(why, kFrameWhy);
        }
        {   // Endnotes collected at the end of each section.
            Pieces p;
            p.finalSection = "<w:sectPr><w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>" + kPageSize + "</w:sectPr>";
            p.beta = head("Beta") + "<w:p>" + run("Beta words") + noteRef("endnote", 2) + "</w:p>";
            auto doc = open(p.docx());
            QVERIFY(doc->hasSectionEndEndnotes());
            QString why;
            QVERIFY(!moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Alpha")), &why));
            QCOMPARE(why, kEndnotesWhy);
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
        QVERIFY(pane->dropLine()->geometry().top() <= tree->visualItemRect(alpha).top() && pane->dropLine()->geometry().bottom() >= tree->visualItemRect(alpha).top());   // (it lies on the edge between the rows)
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
        // The two runs that swap places are looked at for the ends of wrappers, and no paragraph more: the section and what it jumped over, not the whole text.
        const qint64 jumped = (2000 - target) * 5;
        QVERIFY2(moveBlocksWrapperScanForTest() >= jumped && moveBlocksWrapperScanForTest() <= jumped + 3, qPrintable(QString::number(moveBlocksWrapperScanForTest()) + QStringLiteral(" against ") + QString::number(jumped)));
    }

    void commandsMoveTheSectionTheCaretIsInPastItsNeighbor()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        WriteDoc *doc = w.document();
        QVERIFY(w.act("view.moveSectionUp") && w.act("view.moveSectionDown"));
        QCOMPARE(w.act("view.moveSectionUp")->shortcut().toString(QKeySequence::PortableText), QStringLiteral("Alt+Shift+Up"));
        const QByteArray before = doc->toDocx();
        const Pieces p = panePieces();
        QTextCursor c(doc->story());
        c.setPosition(positionOf(*doc, QStringLiteral("Beta sub words")));   // (in the text under the sub-heading: that section is the one that moves, and it has no neighbor of its level)
        w.view()->showCursor(c);
        w.refreshNow();
        w.act("view.moveSectionUp")->trigger();
        QVERIFY(!doc->canUndo());
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("same level")), qPrintable(w.statusBar()->currentMessage()));
        c.setPosition(positionOf(*doc, QStringLiteral("Beta words")));
        w.view()->showCursor(c);
        w.refreshNow();
        w.act("view.moveSectionUp")->trigger();
        QVERIFY2(differsIn(doc->toDocx(), open(p.docx({2, 1, 3}))->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), open(p.docx({2, 1, 3}))->toDocx())));
        QCOMPARE(w.view()->cursor().block().text(), QStringLiteral("Beta words"));   // (the caret stays in the text it was in, which went with the section)
        w.act("view.moveSectionDown")->trigger();
        w.act("view.moveSectionDown")->trigger();
        QVERIFY2(differsIn(doc->toDocx(), open(p.docx({1, 3, 2}))->toDocx()).isEmpty(), qPrintable(differsIn(doc->toDocx(), open(p.docx({1, 3, 2}))->toDocx())));
        doc->undo();
        doc->undo();
        doc->undo();
        QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        // The pane's own commands (the context menu's) are those too, for the row they are on.
        w.navigationPane()->rescan();   // (the text was edited since the pane looked: its rows are not used)
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
            w.refreshNow();
            QVERIFY(!w.act("view.moveSectionUp")->isEnabled());
            w.moveSectionAtCaret(true);   // (the key can come before the commands are brought up to date)
            QVERIFY(w.statusBar()->currentMessage().contains(QLatin1String("protected")) || w.statusBar()->currentMessage().contains(QLatin1String("reading only")));
            w.act("view.moveSectionUp")->trigger();
            QVERIFY(!w.navigationPane()->moveSectionBy(beta, true));
            QVERIFY(!doc->canUndo());
            QVERIFY2(differsIn(doc->toDocx(), before).isEmpty(), qPrintable(differsIn(doc->toDocx(), before)));
        }
    }

    // ---- where the commands work and what they leave alone

    // The caret in a heading's text, `offset` characters into the paragraph that starts with `words`.
    static void caretIn(WriteWindow &w, const QString &words, int offset)
    {
        QTextCursor c(w.document()->story());
        c.setPosition(positionOf(*w.document(), words) + offset);
        w.view()->showCursor(c);
        w.refreshNow();   // (the commands are brought up to date a moment after a change: a test does not wait for the moment)
    }
    // Where the caret and the other end of the selection are, as the paragraph they are in and how far into it.
    static QString whereIs(WriteWindow &w)
    {
        const QTextCursor &c = w.view()->cursor();
        const QTextBlock a = c.document()->findBlock(c.anchor());
        return QStringLiteral("%1:%2 to %3:%4").arg(a.text().remove(QChar(0xFFFC)), QString::number(c.anchor() - a.position()), c.block().text().remove(QChar(0xFFFC)), QString::number(c.positionInBlock()));
    }

    // A caret in a header, a footer, a note, a box of text, or a comment is not in a section of the text: the commands are off, and used all the same (the key), they change nothing and say why.
    void theSectionCommandsRefuseWhereTheCaretIsNotInTheText()
    {
        for (int story = 0; story < 6; ++story) {
            Pieces p = panePieces();
            if (story == 5) p.beta = head("Beta") + "<w:p><w:commentRangeStart w:id=\"0\"/>" + run("Beta words") + "<w:commentRangeEnd w:id=\"0\"/><w:r><w:commentReference w:id=\"0\"/></w:r></w:p>" + head("Beta sub", 2) + para("Beta sub words");
            if (story == 4) p.beta = head("Beta") + "<w:p>" + run("Beta box ") + textBox(5) + "</w:p>" + head("Beta sub", 2) + para("Beta sub words");
            WriteWindow w;
            openPane(w, p.docx());
            WriteDoc *doc = w.document();
            const QByteArray before = doc->toDocx();
            const QString which = QStringList{"header", "footer", "footnote", "endnote", "text box", "comment"}.at(story);
            caretIn(w, QStringLiteral("Beta words"), 3);
            switch (story) {
            case 0: QVERIFY(w.view()->enterHeaderFooter(0, false, true)); break;
            case 1: QVERIFY(w.view()->enterHeaderFooter(0, true, true)); break;
            case 2: w.act("refs.footnote")->trigger(); QVERIFY(w.view()->inNote()); break;
            case 3: w.act("refs.endnote")->trigger(); QVERIFY(doc->endnoteIdOf(w.view()->cursor().block(), nullptr) || write::inEndnoteWords(w.view()->cursor())); break;
            case 4: {
                const int box = doc->story()->toPlainText().indexOf(QChar(0xFFFC));
                QVERIFY(box >= 0);
                w.view()->selectPicture(box);
                QVERIFY(w.view()->beginShapeTextEdit());
                QVERIFY(w.view()->editingShapeText());
                break;
            }
            default: {
                w.showReviewPane(false);
                QTRY_VERIFY(w.reviewPane()->commentList()->topLevelItemCount() > 0);
                w.reviewPane()->commentList()->setCurrentItem(w.reviewPane()->commentList()->topLevelItem(0));
                QVERIFY(w.reviewPane()->replyField()->isEnabled());
                w.reviewPane()->replyField()->setFocus();
                QVERIFY(w.focusWidget() == w.reviewPane()->replyField());
                break;
            }
            }
            const QByteArray typed = doc->toDocx();
            w.refreshNow();
            QVERIFY2(!w.act("view.moveSectionUp")->isEnabled() && !w.act("view.moveSectionDown")->isEnabled(), qPrintable(which));
            QVERIFY2(w.act("view.moveSectionUp")->toolTip().contains(QLatin1String("main text")), qPrintable(which + QLatin1Char(' ') + w.act("view.moveSectionUp")->toolTip()));
            // (A key can arrive before the commands are brought up to date: the move itself looks too.)
            w.statusBar()->clearMessage();
            w.moveSectionAtCaret(true);
            w.moveSectionAtCaret(false);
            QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("main text")), qPrintable(which + QLatin1Char(' ') + w.statusBar()->currentMessage()));
            QVERIFY2(differsIn(doc->toDocx(), typed).isEmpty(), qPrintable(which + QLatin1Char(' ') + differsIn(doc->toDocx(), typed)));
            Q_UNUSED(before);
        }
    }

    // Where a move would be refused for a reason that does not depend on what is moved, the commands are off, and the tooltip says why (and is gone when the reason is).
    void theSectionCommandsAreOffWithTheReasonWhereTheyWouldRefuse()
    {
        auto offWith = [](WriteWindow &w, const QString &part) {
            w.refreshNow();
            for (const char *id : {"view.moveSectionUp", "view.moveSectionDown"}) {
                QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
                QVERIFY2(w.act(QString::fromLatin1(id))->toolTip().contains(part), qPrintable(w.act(QString::fromLatin1(id))->toolTip()));
                QVERIFY2(w.act(QString::fromLatin1(id))->statusTip().contains(part), qPrintable(w.act(QString::fromLatin1(id))->statusTip()));
            }
        };
        {   // No heading at or above the caret: in a text with none, and above the first one.
            WriteWindow w;
            openPane(w, build(para("Just words") + para("More words")));
            caretIn(w, QStringLiteral("More"), 1);
            offWith(w, QStringLiteral("heading"));
        }
        if (QTest::currentTestFailed()) return;
        {
            WriteWindow w;
            openPane(w, paneDocx());
            caretIn(w, QStringLiteral("Intro"), 1);
            offWith(w, QStringLiteral("heading"));
            if (QTest::currentTestFailed()) return;
            caretIn(w, QStringLiteral("Beta words"), 1);
            w.refreshNow();
            QVERIFY(w.act("view.moveSectionUp")->isEnabled() && w.act("view.moveSectionDown")->isEnabled());
            QVERIFY(w.act("view.moveSectionUp")->toolTip().contains(QLatin1String("Move Section Up")));
            // Track Changes on: the commands stay on (the move is made, and says so).
            w.document()->setTrackChanges(true);
            w.refreshNow();
            QVERIFY(w.act("view.moveSectionUp")->isEnabled() && w.act("view.moveSectionDown")->isEnabled());
        }
        {   // Read only, and protected.
            WriteWindow w;
            openPane(w, paneDocx());
            caretIn(w, QStringLiteral("Beta words"), 1);
            w.setReadOnly(true);
            offWith(w, QStringLiteral("reading only"));
        }
        if (QTest::currentTestFailed()) return;
        {
            WriteWindow w;
            openPane(w, paneDocx("<w:documentProtection w:edit=\"readOnly\" w:enforcement=\"1\"/>"));
            caretIn(w, QStringLiteral("Beta words"), 1);
            offWith(w, QStringLiteral("protected"));
        }
    }

    // The caret stays at the same place in the same text, and a selection stays on the same words, wherever the move comes from.
    void afterAMoveTheCaretAndTheSelectionStayInTheSameText()
    {
        {
            WriteWindow w;
            openPane(w, paneDocx());
            QTextCursor c(w.document()->story());
            const int at = positionOf(*w.document(), QStringLiteral("Beta words"));
            c.setPosition(at + 2);
            c.setPosition(at + 6, QTextCursor::KeepAnchor);   // ("ta w")
            w.view()->setCursor(c);
            w.refreshNow();
            QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("ta w"));
            w.act("view.moveSectionUp")->trigger();
            QCOMPARE(w.document()->story()->findBlockByNumber(1).text(), QStringLiteral("Beta"));   // (it moved)
            QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("ta w"));
            QCOMPARE(whereIs(w), QStringLiteral("Beta words:2 to Beta words:6"));
            w.act("view.moveSectionDown")->trigger();
            QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("ta w"));
            QCOMPARE(whereIs(w), QStringLiteral("Beta words:2 to Beta words:6"));
            // A caret with no selection, in the middle of a word of the heading itself.
            caretIn(w, QStringLiteral("Beta"), 2);
            w.act("view.moveSectionDown")->trigger();
            QCOMPARE(whereIs(w), QStringLiteral("Beta:2 to Beta:2"));
        }
        {   // A caret in the text the section jumps over (above it), and in text below.
            WriteWindow w;
            openPane(w, paneDocx());
            QTreeWidget *tree = w.navigationPane()->headings();
            caretIn(w, QStringLiteral("Alpha words"), 4);
            QVERIFY(w.navigationPane()->moveSectionBy(rowOf(tree, QStringLiteral("Beta")), true));   // (the menu's command: Beta goes above Alpha)
            QCOMPARE(whereIs(w), QStringLiteral("Alpha words:4 to Alpha words:4"));
            caretIn(w, QStringLiteral("Gamma words"), 3);
            QVERIFY(w.navigationPane()->moveSectionBy(rowOf(tree, QStringLiteral("Beta")), false));
            QCOMPARE(whereIs(w), QStringLiteral("Gamma words:3 to Gamma words:3"));
        }
        {   // The drop of a dragged heading.
            WriteWindow w;
            openPane(w, paneDocx());
            QTreeWidget *tree = w.navigationPane()->headings();
            caretIn(w, QStringLiteral("Gamma words"), 5);
            QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *alpha = rowOf(tree, QStringLiteral("Alpha"));
            press(tree, inRow(tree, beta, 3));
            move(tree, inRow(tree, alpha, 1));
            release(tree, inRow(tree, alpha, 1));
            QCOMPARE(w.document()->story()->findBlockByNumber(1).text(), QStringLiteral("Beta"));
            QCOMPARE(whereIs(w), QStringLiteral("Gamma words:5 to Gamma words:5"));
        }
    }

    // Rows that the text has left behind (an edit the pane has not looked at yet) are not used: the move is refused and nothing changes, from the menu's commands as from a drop.
    void rowsThatWentStaleRefuseTheMenuCommands()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        WriteDoc *doc = w.document();
        NavigationPane *pane = w.navigationPane();
        QTreeWidget *tree = pane->headings();
        QTreeWidgetItem *alpha = rowOf(tree, QStringLiteral("Alpha")), *gamma = rowOf(tree, QStringLiteral("Gamma"));
        QVERIFY(alpha && gamma);
        QVERIFY(moveSection(*doc, positionOf(*doc, QStringLiteral("Gamma")), positionOf(*doc, QStringLiteral("Alpha"))));   // (the text now has Gamma first; the rows say Alpha is the first)
        const QByteArray edited = doc->toDocx();
        const quint64 revision = doc->story()->revision();
        w.statusBar()->clearMessage();
        QVERIFY(!pane->moveSectionBy(alpha, false));
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("changed")), qPrintable(w.statusBar()->currentMessage()));
        QCOMPARE(doc->story()->revision(), revision);
        QVERIFY2(differsIn(doc->toDocx(), edited).isEmpty(), qPrintable(differsIn(doc->toDocx(), edited)));
        // (The pane has looked at the text again: the rows it has now are current, and work.)
        QTreeWidgetItem *now = rowOf(tree, QStringLiteral("Alpha"));
        QVERIFY(now);
        QVERIFY(moveSection(*doc, positionOf(*doc, QStringLiteral("Beta")), positionOf(*doc, QStringLiteral("Gamma"))));
        const QByteArray again = doc->toDocx();
        w.statusBar()->clearMessage();
        QVERIFY(!pane->moveSectionTo(now, 0));
        QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("changed")), qPrintable(w.statusBar()->currentMessage()));
        QVERIFY2(differsIn(doc->toDocx(), again).isEmpty(), qPrintable(differsIn(doc->toDocx(), again)));
        QTreeWidgetItem *fresh = rowOf(tree, QStringLiteral("Beta"));
        QVERIFY(fresh);
        QVERIFY(pane->moveSectionBy(fresh, false));
    }

    // The keys of the two commands are Alt+Shift+Up and Alt+Shift+Down, the ones Outline view uses for its own move, and no command shares them; in Outline view they move the selected heading there.
    void theKeysAreTheOnesTheOutlineUsesAndNoCommandSharesThem()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        const QKeySequence up(Qt::ALT | Qt::SHIFT | Qt::Key_Up), down(Qt::ALT | Qt::SHIFT | Qt::Key_Down);
        QCOMPARE(w.act("view.moveSectionUp")->shortcut().toString(QKeySequence::PortableText), up.toString(QKeySequence::PortableText));
        QCOMPARE(w.act("view.moveSectionDown")->shortcut().toString(QKeySequence::PortableText), down.toString(QKeySequence::PortableText));
        // (The window gives the commands the keys of the person's settings, and drops a key that two commands have: after it has, both commands still have theirs.)
        QCoreApplication::sendPostedEvents();
        QCoreApplication::processEvents();
        QCOMPARE(w.act("view.moveSectionUp")->shortcut().toString(QKeySequence::PortableText), up.toString(QKeySequence::PortableText));
        QCOMPARE(w.act("view.moveSectionDown")->shortcut().toString(QKeySequence::PortableText), down.toString(QKeySequence::PortableText));
        // (No two commands of the window have either key, so the Keyboard page shows no clash.)
        KeyMap map(&w, false);
        for (const QKeySequence &key : {up, down}) {
            QStringList owners;
            for (const QString &id : w.actionIds())
                if (w.act(id)->shortcuts().contains(key)) owners << id;
            QVERIFY2(owners.size() == 1, qPrintable(owners.join(QLatin1Char(' '))));
            QVERIFY(map.owner(key).isEmpty() || map.owner(key) == owners.first());
        }
        // In Outline view the same keys move the selected heading and what is under it, as the outline's own command does.
        QByteArray moved[2];
        for (int own = 0; own < 2; ++own) {
            WriteWindow o;
            openPane(o, paneDocx());
            o.act("view.outline")->trigger();
            QVERIFY(o.outlineView());
            o.outlineView()->selectParagraphAt(positionOf(*o.document(), QStringLiteral("Beta")));
            o.act(own ? "outline.moveUp" : "view.moveSectionUp")->trigger();
            moved[own] = o.document()->toDocx();
        }
        QVERIFY2(differsIn(moved[0], moved[1]).isEmpty(), qPrintable(differsIn(moved[0], moved[1])));
        QVERIFY(differsIn(moved[0], w.document()->toDocx()).size() > 0);   // (and it did move)
    }

    // Outline view's own move records a tracked change, while a section move from the pane or the commands is made as usual and says it was not recorded (Help says so).
    void outlineViewRecordsItsMoveAsTrackedChangesWhereTheSectionMoveIsNot()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        w.document()->setTrackChanges(true);
        w.act("view.outline")->trigger();
        w.outlineView()->selectParagraphAt(positionOf(*w.document(), QStringLiteral("Beta")));
        w.act("outline.moveUp")->trigger();
        const QString xml = documentXml(w.document()->toDocx());
        QVERIFY2(xml.contains(QLatin1String("<w:ins ")) || xml.contains(QLatin1String("<w:moveTo ")), qPrintable(xml.left(400)));
        QVERIFY(!write::revisionsIn(w.document()->story()).isEmpty());
        // (The commands, in the text, are made, and say so.)
        w.act("view.printLayout")->trigger();
        caretIn(w, QStringLiteral("Alpha words"), 1);
        const QByteArray tracked = w.document()->toDocx();
        w.statusBar()->clearMessage();
        w.act("view.moveSectionUp")->trigger();
        QCOMPARE(w.statusBar()->currentMessage(), untrackedNoticeText());
        QVERIFY2(!differsIn(w.document()->toDocx(), tracked).isEmpty(), "the section moved");
    }

    // View > Move Section Up and Move Section Down are on the ribbon, by the Navigation pane's button.
    void theCommandsAreOnTheViewTabNextToTheNavigationPane()
    {
        WriteWindow w;
        openPane(w, paneDocx());
        QStringList found;
        QTRY_VERIFY_WITH_TIMEOUT(w.isActiveWindow(), 30000);
        QTRY_VERIFY(w.ribbon()->tab(QStringLiteral("View")));
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("View")));
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() && (b->defaultAction() == w.act("view.moveSectionUp") || b->defaultAction() == w.act("view.moveSectionDown") || b->defaultAction() == w.act("view.navigationPane")))
                found << b->defaultAction()->objectName();
        found.removeDuplicates();
        found.sort();
        QCOMPARE(found, (QStringList{"view.moveSectionDown", "view.moveSectionUp", "view.navigationPane"}));
    }

    // The line that shows where a dragged heading will land is as plain to see as the line of a table that is moved: the accent color, two pixels thick.
    void theDropLineIsTheAccentColorAndTwoPixelsThick()
    {
        applyUiTheme(1);
        WriteWindow w;
        openPane(w, paneDocx());
        NavigationPane *pane = w.navigationPane();
        QTreeWidget *tree = pane->headings();
        QTreeWidgetItem *beta = rowOf(tree, QStringLiteral("Beta")), *alpha = rowOf(tree, QStringLiteral("Alpha"));
        press(tree, inRow(tree, beta, 3));
        move(tree, inRow(tree, alpha, 1));
        QVERIFY(pane->dropLine()->isVisible());
        QCOMPARE(pane->dropLine()->height(), 2);
        const QImage line = pane->dropLine()->grab().toImage();
        QCOMPARE(line.height(), 2);
        for (int y = 0; y < 2; ++y)
            for (int x : {0, line.width() / 2, line.width() - 1}) QCOMPARE(line.pixelColor(x, y).name(), uiAccent().name());
        release(tree, inRow(tree, alpha, 1));
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
