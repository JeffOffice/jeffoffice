// Tests of the References tab's work (apps/write/core/tocops.*, notesops, captionops, citeops, indexops): table entry fields, footnote and endnote numbering, captions and tables of
// figures, sources and bibliographies, indexes, and tables of authorities. Each document is built here from a few lines of XML, and each feature is read back from the .docx it writes.

#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/writedialogs.h"
#include "app/writewindow.h"
#include "core/captionops.h"
#include "core/citeops.h"
#include "core/fieldcode.h"
#include "core/fieldops.h"
#include "core/indexops.h"
#include "core/refops.h"
#include "core/styleops.h"
#include "core/tocops.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "core/appinfo.h"
#include "core/document.h"
#include "flow/paginator.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "../localepin.h"
#include "office/package.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include <QDialog>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QSpinBox>
#include <QTimer>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;
using namespace jp::write::fields;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

const char kStyles[] =
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr></w:style>";

QByteArray makeDocx(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + kStyles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    return w.finish();
}

QByteArray documentXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/document.xml"));
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

QByteArray run(const QByteArray &text) { return "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; }
QByteArray para(const QByteArray &content, const QByteArray &props = QByteArray()) { return "<w:p>" + (props.isEmpty() ? QByteArray() : "<w:pPr>" + props + "</w:pPr>") + content + "</w:p>"; }
QByteArray field(const QByteArray &code, const QByteArray &result, const QByteArray &flags = QByteArray())
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"" + flags + "/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" +
           run(result) + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}
QByteArray bookmark(int id, const QByteArray &name, const QByteArray &inner)
{
    return "<w:bookmarkStart w:id=\"" + QByteArray::number(id) + "\" w:name=\"" + name + "\"/>" + inner + "<w:bookmarkEnd w:id=\"" + QByteArray::number(id) + "\"/>";
}

// A document with footnotes and endnotes parts (and settings) beside the body.
QByteArray makeDocxNotes(const QByteArray &body, const QByteArray &footnotes, const QByteArray &endnotes, const QByteArray &settings = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QString ct = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + sect + "</w:body></w:document>"), ct + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + kStyles + "</w:styles>"), ct + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!footnotes.isEmpty()) {
        w.addPart(QStringLiteral("/word/footnotes.xml"), xml(QByteArray("<w:footnotes ") + kNs + ">" + footnotes + "</w:footnotes>"), ct + QStringLiteral("footnotes+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
    }
    if (!endnotes.isEmpty()) {
        w.addPart(QStringLiteral("/word/endnotes.xml"), xml(QByteArray("<w:endnotes ") + kNs + ">" + endnotes + "</w:endnotes>"), ct + QStringLiteral("endnotes+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("endnotes"), QStringLiteral("/word/endnotes.xml"));
    }
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), xml(QByteArray("<w:settings ") + kNs + ">" + settings + "</w:settings>"), ct + QStringLiteral("settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    return w.finish();
}

QByteArray noteXml(const char *tag, int id, const QByteArray &text)
{
    return QByteArray("<w:") + tag + " w:id=\"" + QByteArray::number(id) + "\"><w:p><w:r><w:" + tag + "Ref/></w:r><w:r><w:t xml:space=\"preserve\"> " + text + "</w:t></w:r></w:p></w:" + tag + ">";
}
QByteArray styledNote(const char *tag, int id, const QByteArray &text)
{
    const QByteArray style = QByteArray(tag) == "footnote" ? "FootnoteText" : "EndnoteText";
    return QByteArray("<w:") + tag + " w:id=\"" + QByteArray::number(id) + "\"><w:p><w:pPr><w:pStyle w:val=\"" + style + "\"/></w:pPr><w:r><w:" + tag + "Ref/></w:r><w:r><w:t xml:space=\"preserve\"> " + text + "</w:t></w:r></w:p></w:" + tag + ">";
}
const char kSeparators[] = "<w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>";

// The numbers a page's text shows for its notes: the displayed text of the references (and note marks) in the body's box.
QStringList shownMarks(WriteDoc &doc, int page)
{
    QStringList out;
    const auto mat = doc.pager().materialize(page);
    for (const auto &item : mat->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const Story *s = mat->doc->story(t->storyId))
                for (QTextBlock b = s->doc->begin(); b.isValid(); b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it) {
                        const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                        if (code.startsWith(QLatin1String("raw:")) && !code.mid(4).isEmpty()) out << code.mid(4);
                    }
    return out;
}

// Acts on the next modal dialog that opens (the test's side of a dialog whose exec() waits): `act` is given it, and closes it. With JW_SHOT_DIR set, a picture of it is saved first. A dialog
// that does not open within ten seconds fails the test; one that `act` leaves open (a QVERIFY in it returns before the dialog is closed) is rejected and fails the test, so that exec() returns.
void onNextDialog(const QString &name, const std::function<void(QDialog *)> &act)
{
    auto *timer = new QTimer;
    auto *waited = new QElapsedTimer;
    waited->start();
    timer->setInterval(20);
    QObject::connect(timer, &QTimer::timeout, [timer, waited, name, act] {
        QWidget *modal = QApplication::activeModalWidget();
        if (!modal) {
            if (waited->elapsed() > 10000) {
                timer->stop();
                timer->deleteLater();
                delete waited;
                QTest::qFail(qPrintable(QStringLiteral("the dialog %1 did not open").arg(name)), __FILE__, __LINE__);
            }
            return;
        }
        timer->stop();
        timer->deleteLater();
        delete waited;
        auto *dlg = qobject_cast<QDialog *>(modal);
        if (!dlg) return;
        const QByteArray dir = qgetenv("JW_SHOT_DIR");
        if (!dir.isEmpty()) dlg->grab().save(QString::fromUtf8(dir) + QLatin1Char('/') + name + QStringLiteral(".png"));
        act(dlg);
        if (dlg->isVisible()) {
            QTest::qFail(qPrintable(QStringLiteral("the dialog %1 was left open").arg(name)), __FILE__, __LINE__);
            dlg->reject();
        }
    });
    timer->start();
}

// What the paragraphs show, a line each: the text, and the result of a field that is one character (its text is not in the story's).
QStringList lines(const WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        QString line;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const QString field = frag.charFormat().stringProperty(tp::Field);
            if (field.startsWith(QLatin1String("raw:"))) line += field.mid(4).repeated(frag.length());
            else line += QString(frag.text()).remove(QChar::ObjectReplacementCharacter);
        }
        out << line;
    }
    return out;
}

} // namespace

class RefsTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        jp::initCore();
    }

    // ---- table of contents: table entry fields
    void aTableBuiltFromTableEntryFieldsHasThoseEntriesAtTheirLevels()
    {
        const QByteArray body = para(run("Intro"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Some text ") + field("TC \"Marked one\" \\f C \\l 2", "")) +
                                para(run("More ") + field("TC \"Other table\" \\f D", "")) + para(run("And ") + field("TC \"Marked two\" \\f C", "")) +
                                para(field("TOC \\f C", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        const QStringList l = lines(*doc);
        QVERIFY2(l.contains(QStringLiteral("Marked one\t1")), qPrintable(l.join('|')));
        QVERIFY2(l.contains(QStringLiteral("Marked two\t1")), qPrintable(l.join('|')));
        QVERIFY(!l.join('|').contains(QStringLiteral("Other table")));
        QVERIFY(!l.join('|').contains(QStringLiteral("Intro\t")));   // (\f alone takes no headings)
        // The levels are the entries' own: the first is at level 2, the second at 1.
        QTextBlock b = doc->story()->begin();
        while (b.isValid() && !QString(b.text()).remove(QChar::ObjectReplacementCharacter).startsWith(QLatin1String("Marked one"))) b = b.next();
        QVERIFY(b.isValid());
        QCOMPARE(b.blockFormat().stringProperty(tp::StyleName), QStringLiteral("TOC2"));
    }

    void aTableWithBothHeadingsAndEntriesListsThemInTheOrderOfTheText()
    {
        const QByteArray body = para(run("Intro"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("Text ") + field("TC \"Marked\" \\l 2 \\n", "")) +
                                para(run("Later"), "<w:pStyle w:val=\"Heading1\"/>") + para(field("TOC \\o \"1-3\" \\f \\h", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        const QStringList l = lines(*doc);
        const int a = l.indexOf(QStringLiteral("Intro\t1")), m = l.indexOf(QStringLiteral("Marked")), z = l.indexOf(QStringLiteral("Later\t1"));
        QVERIFY2(a >= 0 && m > a && z > m, qPrintable(l.join('|')));   // (a \n entry has no page number)
        // Each is a link to a bookmark; the heading's paragraph holds the entry's, once.
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("w:anchor=\"_Toc100000001\""));
        QVERIFY(xml.contains("w:anchor=\"_Toc100000002\""));
        QVERIFY(xml.contains("w:anchor=\"_Toc100000003\""));
        QCOMPARE(xml.count("w:name=\"_Toc100000002\""), 1);
    }

    void markingATableEntryPutsTheFieldAtTheCaretAndItRoundTrips()
    {
        auto doc = open(makeDocx(para(run("Alpha beta gamma")) + para(field("TOC \\f X", "x"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(6);
        c.setPosition(10, QTextCursor::KeepAnchor);   // "beta"
        QVERIFY(markTocEntry(*doc, c, QString(), 2, QStringLiteral("X")));
        QVERIFY(doc->canUndo());
        updateFields(*doc);
        QVERIFY2(lines(*doc).contains(QStringLiteral("beta\t1")), qPrintable(lines(*doc).join('|')));
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("TC \"beta\" \\f X \\l 2"));
        auto again = open(doc->toDocx());
        QVERIFY(again);
        updateFields(*again);
        QVERIFY(lines(*again).contains(QStringLiteral("beta\t1")));
        QVERIFY(!markTocEntry(*doc, c, QStringLiteral("   "), 1));
    }

    void addTextMakesTheParagraphAHeadingOfThatLevelAndZeroUndoesIt()
    {
        auto doc = open(makeDocx(para(run("Just a line")) + para(run("Next"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(2);
        QVERIFY(addTextToToc(*doc, c, 2));
        QCOMPARE(doc->story()->begin().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading2"));
        QVERIFY(addTextToToc(*doc, c, 0));
        QCOMPARE(doc->story()->begin().blockFormat().stringProperty(tp::StyleName), doc->defaultParagraphStyle());
    }

    // ---- captions and tables of figures
    void aCaptionIsAParagraphOfTheCaptionStyleWithACountingField()
    {
        auto doc = open(makeDocx(para(run("A picture")) + para(run("After"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(3);
        CaptionOptions o;
        o.text = QStringLiteral(": First");
        QVERIFY(insertCaption(*doc, c, o));
        QCOMPARE(lines(*doc), (QStringList{QStringLiteral("A picture"), QStringLiteral("Figure 1: First"), QStringLiteral("After")}));
        QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Caption"));
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("SEQ Figure \\* ARABIC"));
        QVERIFY(xml.contains("<w:pStyle w:val=\"Caption\"/>"));
        // One step; and the style is in the file only while the caption is.
        QVERIFY(doc->undo());
        QCOMPARE(lines(*doc), (QStringList{QStringLiteral("A picture"), QStringLiteral("After")}));
    }

    void aCaptionAboveRenumbersTheOnesAfterIt()
    {
        auto doc = open(makeDocx(para(run("One")) + para(run("Two"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(1);
        CaptionOptions o;
        o.text = QStringLiteral(" later");
        QVERIFY(insertCaption(*doc, c, o));            // below "One": Figure 1
        c.setPosition(doc->story()->findBlockByNumber(0).position() + 1);
        CaptionOptions above;
        above.above = true;
        above.text = QStringLiteral(" first");
        QVERIFY(insertCaption(*doc, c, above));       // above "One": Figure 1, and the one after is 2
        QCOMPARE(lines(*doc), (QStringList{QStringLiteral("Figure 1 first"), QStringLiteral("One"), QStringLiteral("Figure 2 later"), QStringLiteral("Two")}));
        // The label can be a word of one's own, with its own count, and the number alone.
        CaptionOptions own;
        own.label = QStringLiteral("Chart");
        own.excludeLabel = true;
        c.setPosition(doc->story()->findBlockByNumber(3).position());
        QVERIFY(insertCaption(*doc, c, own));
        QCOMPARE(lines(*doc).last(), QStringLiteral("1"));
        QVERIFY(captionLabelChoices(*doc).contains(QStringLiteral("Chart")));
    }

    void aCaptionWithAChapterNumberCountsAgainFromEachHeading()
    {
        auto doc = open(makeDocx(para(run("Ch one"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("a")) + para(run("Ch two"), "<w:pStyle w:val=\"Heading1\"/>") + para(run("b"))));
        QVERIFY(doc);
        CaptionOptions o;
        o.chapterLevel = 1;
        o.text = QStringLiteral(".");
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 1);
        QVERIFY(insertCaption(*doc, c, o));
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 1);
        QVERIFY(insertCaption(*doc, c, o));
        c.setPosition(doc->story()->findBlockByNumber(4).position() + 1);
        QVERIFY(insertCaption(*doc, c, o));
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("STYLEREF 1 \\s"));
        QVERIFY(xml.contains("SEQ Figure \\* ARABIC \\s 1"));
        QStringList caps;
        for (const QString &l : lines(*doc))
            if (l.startsWith(QLatin1String("Figure"))) caps << l.section(QLatin1Char('-'), 1);
        QCOMPARE(caps, (QStringList{QStringLiteral("1."), QStringLiteral("2."), QStringLiteral("1.")}));
    }

    void aCaptionNextToATableGoesAboveOrBelowTheWholeTable()
    {
        const QByteArray table = "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>a1</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>b1</w:t></w:r></w:p></w:tc></w:tr>"
                                 "<w:tr><w:tc><w:p><w:r><w:t>a2</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>b2</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        auto doc = open(makeDocx(para(run("Before")) + table + para(run("After"))));
        QVERIFY(doc);
        const QString plain = doc->story()->toPlainText();
        QTextCursor c(doc->story());
        c.setPosition(int(plain.indexOf(QLatin1String("a2"))) + 1);
        CaptionOptions o;
        o.label = QStringLiteral("Table");
        o.above = true;
        QVERIFY(insertCaption(*doc, c, o));
        c.setPosition(int(doc->story()->toPlainText().indexOf(QLatin1String("b1"))) + 1);
        CaptionOptions below;
        below.label = QStringLiteral("Table");
        QVERIFY(insertCaption(*doc, c, below));
        QCOMPARE(lines(*doc), (QStringList{QStringLiteral("Before"), QStringLiteral("Table 1"), QStringLiteral("a1"), QStringLiteral("b1"), QStringLiteral("a2"), QStringLiteral("b2"), QStringLiteral("Table 2"), QStringLiteral("After")}));
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(lines(*again).size(), 8);
    }

    void aTableOfFiguresListsTheCaptionsOfOneLabel()
    {
        auto doc = open(makeDocx(para(run("Pic")) + para(run("Chart")) + para(run("End"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(1);
        CaptionOptions o;
        o.text = QStringLiteral(": Sunrise");
        QVERIFY(insertCaption(*doc, c, o));
        c.setPosition(doc->story()->findBlockByNumber(2).position() + 1);
        CaptionOptions t;
        t.label = QStringLiteral("Table");
        t.text = QStringLiteral(": Prices");
        QVERIFY(insertCaption(*doc, c, t));
        c.setPosition(doc->story()->characterCount() - 1);
        FiguresOptions f;
        QVERIFY(insertTableOfFigures(*doc, c, f));
        QStringList l = lines(*doc);
        QVERIFY2(l.contains(QStringLiteral("Figure 1: Sunrise\t1")), qPrintable(l.join('|')));
        QVERIFY(!l.join('|').contains(QStringLiteral("Prices\t")));
        QTextBlock b = doc->story()->begin();
        while (b.isValid() && !b.text().contains(QLatin1String("Sunrise\t"))) b = b.next();
        QVERIFY(b.isValid());
        QCOMPARE(b.blockFormat().stringProperty(tp::StyleName), QStringLiteral("TableofFigures"));
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("TOC \\h \\z \\c \"Figure\""));
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(updateFields(*again).changed, 0);
        // Without the label and number.
        FiguresOptions plain;
        plain.includeLabel = false;
        plain.pageNumbers = false;
        c = QTextCursor(doc->story());
        c.setPosition(doc->story()->characterCount() - 1);
        QVERIFY(insertTableOfFigures(*doc, c, plain));
        QVERIFY2(lines(*doc).contains(QStringLiteral("Sunrise")), qPrintable(lines(*doc).join('|')));
    }

    // ---- index
    void anIndexListsTheMarkedEntriesSortedWithTheirPages()
    {
        const QByteArray pageBreak = "<w:r><w:br w:type=\"page\"/></w:r>";
        const QByteArray body = para(run("Fruit ") + field("XE \"Banana:Cavendish\"", "") + run(" and ") + field("XE \"Apple\"", "")) + para(pageBreak + run("More ") + field("XE \"Apple:Fuji\"", "") + field("XE \"Apple\"", "")) +
                                para(pageBreak + run("Last ") + field("XE \"Apple\" \\b", "") + field("XE \"Zebra\" \\t \"See Apple\"", "")) + para(field("INDEX \\h \"A\"", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        const QStringList l = lines(*doc);
        const int i = l.indexOf(QStringLiteral("A"));
        QVERIFY2(i >= 0, qPrintable(l.join('|')));
        QCOMPARE(l.mid(i, 8), (QStringList{QStringLiteral("A"), QStringLiteral("Apple, 1, 2, 3"), QStringLiteral("Fuji, 2"), QStringLiteral("B"), QStringLiteral("Banana"), QStringLiteral("Cavendish, 1"),
                                          QStringLiteral("Z"), QStringLiteral("Zebra. See Apple")}));
        // The subentry is in the style one level down, the heading letter in the heading style.
        QTextBlock b;
        for (b = doc->story()->begin(); b.isValid() && !b.text().startsWith(QLatin1String("Fuji")); b = b.next()) {}
        QVERIFY(b.isValid());
        QCOMPARE(b.blockFormat().stringProperty(tp::StyleName), QStringLiteral("Index2"));
        // Updating again changes nothing; and the file keeps the field, written back whole.
        QCOMPARE(updateFields(*doc).changed, 0);
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("INDEX \\h \"A\""));
        QVERIFY(xml.contains("XE \"Banana:Cavendish\""));
        // A page number of a bold mark is bold.
        QTextCursor c(doc->story());
        const int at = int(doc->story()->toPlainText().indexOf(QLatin1String("Apple, 1, 2, 3"))) + 13;
        c.setPosition(at + 1);
        QVERIFY(c.charFormat().fontWeight() >= QFont::Bold);
    }

    void anIndexCanRunSubentriesOnAndFilterByTypeAndRightAlignPageNumbers()
    {
        const QByteArray body = para(run("x") + field("XE \"Cat\"", "") + field("XE \"Cat:Lion\"", "") + field("XE \"Cat:Tiger\"", "") + field("XE \"Dog\" \\f \"b\"", "")) +
                                para(field("INDEX \\r", "x")) + para(field("INDEX \\f \"b\" \\e \"\t\"", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        const QStringList l = lines(*doc);
        QVERIFY2(l.contains(QStringLiteral("Cat, 1: Lion, 1; Tiger, 1")), qPrintable(l.join('|')));
        QVERIFY(!l.contains(QStringLiteral("Dog, 1")));        // (the first index takes marks of no type)
        QVERIFY2(l.contains(QStringLiteral("Dog\t1")), qPrintable(l.join('|')));
        QVERIFY(!l.join('|').contains(QStringLiteral("Cat\t")));
    }

    void markingEntriesPutsTheFieldAtTheCaretAndInsertingAnIndexBuildsIt()
    {
        auto doc = open(makeDocx(para(run("The quick fox met a quick hare. Quick!"))));
        QVERIFY(doc);
        IndexMark m;
        m.main = QStringLiteral("quick");
        m.sub = QStringLiteral("speed");
        m.bold = true;
        QTextCursor c(doc->story());
        c.setPosition(4);
        c.setPosition(9, QTextCursor::KeepAnchor);
        QVERIFY(markIndexEntry(*doc, c, m));
        QCOMPARE(markAllIndexEntries(*doc, m), 1);   // (the other "quick" in lower case; the one marked has its field, and "Quick" is another case)
        QVERIFY(markedIndexEntries(*doc).contains(QStringLiteral("quick:speed")));
        c = QTextCursor(doc->story());
        c.setPosition(doc->story()->characterCount() - 1);
        IndexOptions o;
        o.rightAlign = false;
        QVERIFY(insertIndex(*doc, c, o));
        QVERIFY2(lines(*doc).join('|').contains(QStringLiteral("quick|speed, 1")), qPrintable(lines(*doc).join('|')));
        QVERIFY(doc->undo());
        QVERIFY(!doc->story()->toPlainText().contains(QStringLiteral("speed, 1")));
        QVERIFY(doc->undo());
        QVERIFY(doc->undo());
        QVERIFY(!documentXml(doc->toDocx()).contains("XE \"quick:speed\""));
        QVERIFY(!markIndexEntry(*doc, c, IndexMark()));
    }

    // ---- table of authorities
    void aTableOfAuthoritiesListsCitationsByCategoryWithPassim()
    {
        QByteArray body = para(run("Court held ") + field("TA \\l \"Smith v. Jones, 410 U.S. 113 (1973)\" \\s \"Smith\" \\c 1", "")) +
                          para(run("Under ") + field("TA \\l \"26 U.S.C. 501\" \\c 2", "")) + para(run("Again ") + field("TA \\l \"Adams v. Baker, 1 U.S. 1 (1790)\" \\c 1", ""));
        const QByteArray pageBreak = "<w:r><w:br w:type=\"page\"/></w:r>";
        for (int i = 0; i < 5; ++i) body += para(pageBreak + run("p ") + field("TA \\l \"Smith v. Jones, 410 U.S. 113 (1973)\" \\c 1", ""));
        body += para(field("TOA \\h \\c 1 \\p", "x")) + para(field("TOA \\h \\e \", \"", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        updateFields(*doc);
        const QStringList l = lines(*doc);
        const int a = l.indexOf(QStringLiteral("Cases"));
        QVERIFY2(a >= 0, qPrintable(l.join('|')));
        QCOMPARE(l.mid(a, 3), (QStringList{QStringLiteral("Cases"), QStringLiteral("Adams v. Baker, 1 U.S. 1 (1790)\t1"), QStringLiteral("Smith v. Jones, 410 U.S. 113 (1973)\tpassim")}));
        QVERIFY(!l.mid(a, 3).join('|').contains(QStringLiteral("U.S.C.")));   // (\c 1: cases only)
        // The second table lists every category, with page numbers one after another where there are fewer than five.
        QVERIFY2(l.contains(QStringLiteral("Statutes")), qPrintable(l.join('|')));
        QVERIFY2(l.contains(QStringLiteral("26 U.S.C. 501, 1")), qPrintable(l.join('|')));
        QVERIFY(l.contains(QStringLiteral("Smith v. Jones, 410 U.S. 113 (1973), 1, 2, 3, 4, 5, 6")));
        QCOMPARE(updateFields(*doc).changed, 0);
        QVERIFY(documentXml(doc->toDocx()).contains("TA \\l \"26 U.S.C. 501\" \\c 2"));
    }

    void markingACitationAndInsertingATableOfAuthorities()
    {
        auto doc = open(makeDocx(para(run("See Roe v. Wade, 410 U.S. 113 (1973), and again Roe."))));
        QVERIFY(doc);
        AuthorityMark m;
        m.longCitation = QStringLiteral("Roe v. Wade, 410 U.S. 113 (1973)");
        m.shortCitation = QStringLiteral("Roe");
        m.category = 1;
        QTextCursor c(doc->story());
        c.setPosition(5);
        QVERIFY(markAuthority(*doc, c, m));
        QVERIFY(documentXml(doc->toDocx()).contains("TA \\l \"Roe v. Wade, 410 U.S. 113 (1973)\" \\s \"Roe\" \\c 1"));
        c.setPosition(doc->story()->characterCount() - 1);
        AuthoritiesOptions o;
        QVERIFY(insertTableOfAuthorities(*doc, c, o));
        QVERIFY2(lines(*doc).join('|').contains(QStringLiteral("Cases|Roe v. Wade, 410 U.S. 113 (1973)\t1")), qPrintable(lines(*doc).join('|')));
        QCOMPARE(updateIndexOrAuthorities(*doc, c, true), 1);
        QVERIFY(removeIndexOrAuthorities(*doc, c, true));
        QVERIFY(!lines(*doc).join('|').contains(QStringLiteral("Cases")));
        QVERIFY(!markAuthority(*doc, c, AuthorityMark()));
    }

    // ---- sources, citations, and the bibliography
    void sourcesAreReadAndWrittenInTheFormatsOwnElements()
    {
        QVector<Source> sources;
        Source book;
        book.tag = QStringLiteral("Smi20");
        book.authors = {Person{QStringLiteral("Smith"), QStringLiteral("John"), QStringLiteral("A")}, Person{QStringLiteral("Jones"), QStringLiteral("Beth"), QString()}};
        book.fields = {{QStringLiteral("Title"), QStringLiteral("Fish & Chips")}, {QStringLiteral("Year"), QStringLiteral("2020")}, {QStringLiteral("City"), QStringLiteral("Boston")}, {QStringLiteral("Publisher"), QStringLiteral("Pub")}};
        Source site;
        site.tag = QStringLiteral("Acm19");
        site.type = QStringLiteral("InternetSite");
        site.corporateAuthor = QStringLiteral("Acme <Corp>");
        site.fields = {{QStringLiteral("Title"), QStringLiteral("Home")}, {QStringLiteral("URL"), QStringLiteral("https://example.com/?a=1&b=2")}};
        sources << book << site;
        const QByteArray xml = sourcesToXml(sources, QStringLiteral("MLA"));
        QVERIFY(xml.contains("<b:Sources SelectedStyle="));
        QVERIFY(xml.contains("StyleName=\"MLA\""));
        QVERIFY(xml.contains("<b:Tag>Smi20</b:Tag><b:SourceType>Book</b:SourceType>"));
        QVERIFY(xml.contains("<b:Corporate>Acme &lt;Corp&gt;</b:Corporate>"));
        QString style;
        const QVector<Source> again = parseSources(xml, &style);
        QCOMPARE(style, QStringLiteral("MLA"));
        QCOMPARE(again.size(), 2);
        QVERIFY(again[0] == book);
        QVERIFY(again[1] == site);
        // An element this program does not edit is kept as it was.
        QByteArray custom = xml;
        custom.replace("<b:RefOrder>1</b:RefOrder>", "<b:Thing><b:Deep>x</b:Deep></b:Thing><b:RefOrder>1</b:RefOrder>");
        const QVector<Source> kept = parseSources(custom);
        QVERIFY(kept[0].extra.contains("<b:Deep>x</b:Deep>"));
        QVERIFY(sourcesToXml(kept, QStringLiteral("MLA")).contains("<b:Thing><b:Deep>x</b:Deep></b:Thing>"));
        QCOMPARE(newSourceTag(book, {}), QStringLiteral("Smi20"));
        QCOMPARE(newSourceTag(book, again), QStringLiteral("Smi202"));
        // The master list is a file.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("deep/sources.xml"));
        QVERIFY(setMasterSources(sources, path));
        QCOMPARE(masterSources(path).size(), 2);
        QVERIFY(masterSources(dir.filePath(QStringLiteral("none.xml"))).isEmpty());
    }

    void aCitationShowsInTheStyleOfTheSourcesAndChangesWithIt()
    {
        auto doc = open(makeDocx(para(run("Text here."))));
        QVERIFY(doc);
        Source book;
        book.tag = QStringLiteral("Smi20");
        book.authors = {Person{QStringLiteral("Smith"), QStringLiteral("John"), QStringLiteral("A")}};
        book.fields = {{QStringLiteral("Title"), QStringLiteral("A Book")}, {QStringLiteral("Year"), QStringLiteral("2020")}, {QStringLiteral("City"), QStringLiteral("Boston")}, {QStringLiteral("Publisher"), QStringLiteral("Pub")}};
        setDocumentSources(*doc, {book});
        QVERIFY(doc->isModified());
        QTextCursor c(doc->story());
        c.setPosition(5);
        QVERIFY(insertCitation(*doc, c, QStringLiteral("Smi20")));
        CitationOptions pages;
        pages.pages = QStringLiteral("5-7");
        QVERIFY(insertCitation(*doc, c, QStringLiteral("Smi20"), pages));
        QVERIFY(!insertCitation(*doc, c, QStringLiteral("Nobody")));
        QCOMPARE(lines(*doc)[0], QStringLiteral("Text (Smith, 2020)(Smith, 2020, pp. 5-7)here."));
        setDocumentCitationStyle(*doc, QStringLiteral("MLA"));
        QCOMPARE(lines(*doc)[0], QStringLiteral("Text (Smith)(Smith 5-7)here."));
        setDocumentCitationStyle(*doc, QStringLiteral("Chicago"));
        QCOMPARE(lines(*doc)[0], QStringLiteral("Text (Smith 2020)(Smith 2020, 5-7)here."));
        setDocumentCitationStyle(*doc, QStringLiteral("IEEE"));
        QCOMPARE(lines(*doc)[0], QStringLiteral("Text [1][1, pp. 5-7]here."));
        QCOMPARE(documentCitationStyle(*doc), QStringLiteral("IEEE"));
        // The part is in the file, with its relationships, and the file opens again with them.
        const QByteArray zip = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(zip));
        const QByteArray item = p.part(QStringLiteral("/customXml/item1.xml"));
        QVERIFY(item.contains("<b:Tag>Smi20</b:Tag>"));
        QVERIFY(item.contains("StyleName=\"IEEE\""));
        QVERIFY(p.part(QStringLiteral("/customXml/itemProps1.xml")).contains("bibliography"));
        QVERIFY(documentXml(zip).contains("CITATION Smi20 \\p \"5-7\""));
        auto reopened = open(zip);
        QVERIFY(reopened);
        QCOMPARE(documentSources(*reopened).size(), 1);
        QCOMPARE(documentCitationStyle(*reopened), QStringLiteral("IEEE"));
        QCOMPARE(updateFields(*reopened).changed, 0);
        // Saved again unchanged, the part is as it was.
        office::Package q;
        QVERIFY(q.read(reopened->toDocx()));
        QCOMPARE(q.part(QStringLiteral("/customXml/item1.xml")), item);
    }

    void aBibliographyListsTheSourcesCitedSortedInTheStyle()
    {
        auto doc = open(makeDocx(para(run("A")) + para(run("B"))));
        QVERIFY(doc);
        Source book, article, unused;
        book.tag = QStringLiteral("Smi20");
        book.authors = {Person{QStringLiteral("Smith"), QStringLiteral("John"), QStringLiteral("A")}};
        book.fields = {{QStringLiteral("Title"), QStringLiteral("A Book")}, {QStringLiteral("Year"), QStringLiteral("2020")}, {QStringLiteral("City"), QStringLiteral("Boston")}, {QStringLiteral("Publisher"), QStringLiteral("Pub")}};
        article.tag = QStringLiteral("Ada18");
        article.type = QStringLiteral("JournalArticle");
        article.authors = {Person{QStringLiteral("Adams"), QStringLiteral("Beth"), QString()}};
        article.fields = {{QStringLiteral("Title"), QStringLiteral("On Fish")}, {QStringLiteral("JournalName"), QStringLiteral("Nature")}, {QStringLiteral("Year"), QStringLiteral("2018")},
                          {QStringLiteral("Volume"), QStringLiteral("12")}, {QStringLiteral("Issue"), QStringLiteral("3")}, {QStringLiteral("Pages"), QStringLiteral("45-67")}};
        unused.tag = QStringLiteral("Zed01");
        unused.fields = {{QStringLiteral("Title"), QStringLiteral("Never Cited")}};
        setDocumentSources(*doc, {book, article, unused});
        QTextCursor c(doc->story());
        c.setPosition(1);
        QVERIFY(insertCitation(*doc, c, QStringLiteral("Smi20")));
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 1);
        QVERIFY(insertCitation(*doc, c, QStringLiteral("Ada18")));
        c.setPosition(doc->story()->characterCount() - 1);
        QVERIFY(insertBibliography(*doc, c, BibliographyKind::WorksCited));
        QStringList l = lines(*doc);
        const int h = l.indexOf(QStringLiteral("Works Cited"));
        QVERIFY2(h >= 0, qPrintable(l.join('|')));
        QCOMPARE(l.mid(h + 1, 2), (QStringList{QStringLiteral("Adams, B. (2018). On Fish. Nature, 12(3), 45-67."), QStringLiteral("Smith, J. A. (2020). A Book. Boston: Pub.")}));
        QVERIFY(!l.join('|').contains(QStringLiteral("Never Cited")));
        QTextBlock b = doc->story()->begin();
        while (b.isValid() && !QString(b.text()).contains(QLatin1String("Works Cited"))) b = b.next();
        QCOMPARE(b.blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        // The book's title is italic; a new style makes the list again.
        QTextCursor t(doc->story());
        const int at = int(doc->story()->toPlainText().indexOf(QLatin1String("A Book")));
        t.setPosition(at + 2);
        QVERIFY(t.charFormat().fontItalic());
        setDocumentCitationStyle(*doc, QStringLiteral("IEEE"));
        l = lines(*doc);
        QVERIFY2(l.contains(QStringLiteral("[1] J. A. Smith, A Book, Boston: Pub, 2020.")), qPrintable(l.join('|')));
        QVERIFY(l.contains(QStringLiteral("[2] B. Adams, \u201COn Fish,\u201D Nature, vol. 12, no. 3, pp. 45-67, 2018.")));
        QVERIFY(documentXml(doc->toDocx()).contains("BIBLIOGRAPHY"));
    }

    void aFileSourcesPartIsKeptAsItWasAndItsCitationsAreReadFromIt()
    {
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + para(run("See ") + field("CITATION Smi20 \\l 1033", "(old)")) + "</w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        const QByteArray sources = sourcesToXml({[] {
                                                     Source s;
                                                     s.tag = QStringLiteral("Smi20");
                                                     s.authors = {Person{QStringLiteral("Smith"), QString(), QString()}};
                                                     s.fields = {{QStringLiteral("Title"), QStringLiteral("T")}, {QStringLiteral("Year"), QStringLiteral("1999")}};
                                                     return s;
                                                 }()},
                                                 QStringLiteral("APA"));
        w.addPart(QStringLiteral("/customXml/item1.xml"), sources, QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/customXml/itemProps1.xml"), "<ds:datastoreItem xmlns:ds=\"http://schemas.openxmlformats.org/officeDocument/2006/customXml\" ds:itemID=\"{1}\"/>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.customXmlProperties+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("customXml"), QStringLiteral("/customXml/item1.xml"));
        w.addRelationship(QStringLiteral("/customXml/item1.xml"), rel + QStringLiteral("customXmlProps"), QStringLiteral("/customXml/itemProps1.xml"));
        auto doc = open(w.finish());
        QVERIFY(doc);
        QCOMPARE(documentSources(*doc).size(), 1);
        QCOMPARE(updateFields(*doc).changed, 1);   // (the old result "(old)" is made again from the source)
        QCOMPARE(lines(*doc)[0], QStringLiteral("See (Smith, 1999)"));
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        QCOMPARE(p.part(QStringLiteral("/customXml/item1.xml")), sources);   // (not touched: the sources were not changed)
        QVERIFY(!p.part(QStringLiteral("/customXml/item2.xml")).size());
    }

    // ---- footnote and endnote numbering
    void footnoteNumbersShowTheFormatAndTheStartTheSettingsGive()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>" + run(" more") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + noteXml("footnote", 2, "One") + noteXml("footnote", 3, "Two");
        auto doc = open(makeDocxNotes(body, notes, QByteArray()));
        QVERIFY(doc);
        QCOMPARE(shownMarks(*doc, 0).mid(0, 2), (QStringList{QStringLiteral("1"), QStringLiteral("2")}));
        QCOMPARE(doc->noteSettings(false).format, QStringLiteral("decimal"));
        WriteDoc::NoteSettings s;
        s.format = QStringLiteral("lowerRoman");
        s.start = 3;
        doc->setNoteSettings(false, s);
        QVERIFY(doc->isModified());
        QCOMPARE(shownMarks(*doc, 0).mid(0, 2), (QStringList{QStringLiteral("iii"), QStringLiteral("iv")}));
        s.format = QStringLiteral("upperLetter");
        s.start = 1;
        doc->setNoteSettings(false, s);
        QCOMPARE(shownMarks(*doc, 0).mid(0, 2), (QStringList{QStringLiteral("A"), QStringLiteral("B")}));
        // The file says it, and a file that says it is read so.
        const QByteArray zip = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(zip));
        const QByteArray settings = p.part(QStringLiteral("/word/settings.xml"));
        QVERIFY2(settings.contains("<w:footnotePr>") && settings.contains("<w:numFmt w:val=\"upperLetter\"/>") && settings.contains("<w:numStart w:val=\"1\"/>"), settings.constData());
        auto again = open(zip);
        QVERIFY(again);
        QCOMPARE(again->noteSettings(false).format, QStringLiteral("upperLetter"));
        QCOMPARE(shownMarks(*again, 0).mid(0, 2), (QStringList{QStringLiteral("A"), QStringLiteral("B")}));
    }

    void footnoteNumbersCanBeginAgainOnEachPage()
    {
        const QByteArray pageBreak = "<w:r><w:br w:type=\"page\"/></w:r>";
        const QByteArray body = para(run("Page one") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>") + para(pageBreak + run("Page two") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + noteXml("footnote", 2, "One") + noteXml("footnote", 3, "Two");
        auto doc = open(makeDocxNotes(body, notes, QByteArray()));
        QVERIFY(doc);
        QCOMPARE(shownMarks(*doc, 1).mid(0, 1), (QStringList{QStringLiteral("2")}));
        WriteDoc::NoteSettings s;
        s.restart = QStringLiteral("eachPage");
        doc->setNoteSettings(false, s);
        QCOMPARE(shownMarks(*doc, 1).mid(0, 1), (QStringList{QStringLiteral("1")}));
        QVERIFY(documentXml(doc->toDocx()).size() > 0);
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        QVERIFY(p.part(QStringLiteral("/word/settings.xml")).contains("<w:numRestart w:val=\"eachPage\"/>"));
    }

    void endnoteSettingsAreWrittenAndMakeTheMarksAgainWhenTheFileIsRead()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:endnoteReference w:id=\"2\"/></w:r>" + run(" more") + "<w:r><w:endnoteReference w:id=\"3\"/></w:r>");
        const QByteArray notes = "<w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote>" + noteXml("endnote", 2, "One") + noteXml("endnote", 3, "Two");
        auto doc = open(makeDocxNotes(body, QByteArray(), notes));
        QVERIFY(doc);
        auto marks = [](WriteDoc &d) {
            QStringList out;
            for (QTextBlock b = d.story()->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                    if (code.startsWith(QLatin1String("raw:")) && !code.mid(4).isEmpty()) out << code.mid(4);
                }
            return out;
        };
        QVERIFY2(marks(*doc).contains(QStringLiteral("i")) && marks(*doc).contains(QStringLiteral("ii")), qPrintable(marks(*doc).join('|')));
        WriteDoc::NoteSettings s = doc->noteSettings(true);
        QCOMPARE(s.format, QStringLiteral("lowerRoman"));
        s.format = QStringLiteral("upperLetter");
        s.start = 2;
        s.position = QStringLiteral("docEnd");
        doc->setNoteSettings(true, s);
        const QByteArray zip = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(zip));
        const QByteArray settings = p.part(QStringLiteral("/word/settings.xml"));
        QVERIFY2(settings.contains("<w:endnotePr><w:pos w:val=\"docEnd\"/><w:numFmt w:val=\"upperLetter\"/><w:numStart w:val=\"2\"/>"), settings.constData());
        auto again = open(zip);
        QVERIFY(again);
        QVERIFY2(marks(*again).contains(QStringLiteral("B")) && marks(*again).contains(QStringLiteral("C")), qPrintable(marks(*again).join('|')));
        QCOMPARE(again->noteSettings(true).start, 2);
        // Each section begins the numbers again, when the settings say so.
        const QByteArray two = para(run("A") + "<w:r><w:endnoteReference w:id=\"2\"/></w:r>", "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>") + para(run("B") + "<w:r><w:endnoteReference w:id=\"3\"/></w:r>");
        auto sections = open(makeDocxNotes(two, QByteArray(), notes, "<w:endnotePr><w:numFmt w:val=\"decimal\"/><w:numRestart w:val=\"eachSect\"/></w:endnotePr>"));
        QVERIFY(sections);
        QCOMPARE(marks(*sections).mid(0, 2), (QStringList{QStringLiteral("1"), QStringLiteral("1")}));
    }

    // ---- converting notes
    void footnotesConvertToEndnotesAndBackWithTheirWords()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>" + run(" more") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + styledNote("footnote", 2, "One") + styledNote("footnote", 3, "Two");
        auto doc = open(makeDocxNotes(body, notes, QByteArray()));
        QVERIFY(doc);
        QCOMPARE(convertNotes(*doc, NoteConversion::FootnotesToEndnotes), 2);
        QVERIFY(doc->citedFootnotes().isEmpty());
        QCOMPARE(doc->citedEndnotes().size(), 2);
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        const QByteArray main = p.part(QStringLiteral("/word/document.xml")), ends = p.part(QStringLiteral("/word/endnotes.xml"));
        QCOMPARE(main.count("<w:endnoteReference"), 2);
        QVERIFY(!main.contains("footnoteReference"));
        QVERIFY2(ends.indexOf("One") < ends.indexOf("Two") && ends.contains("One") && ends.contains("Two") && ends.contains("<w:endnoteRef/>") && ends.contains("EndnoteText"), ends.constData());
        // It is one step of the history.
        doc->undo();
        QCOMPARE(doc->citedFootnotes().size(), 2);
        QVERIFY(doc->citedEndnotes().isEmpty());
        doc->redo();
        QCOMPARE(doc->citedEndnotes().size(), 2);
        // The file read again has them as endnotes, and back they go.
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(again->citedEndnotes().size(), 2);
        QCOMPARE(convertNotes(*again, NoteConversion::EndnotesToFootnotes), 2);
        QCOMPARE(again->citedFootnotes().size(), 2);
        QVERIFY(p.read(again->toDocx()));
        const QByteArray foot = p.part(QStringLiteral("/word/footnotes.xml"));
        QVERIFY2(foot.contains("One") && foot.contains("Two") && foot.contains("<w:footnoteRef/>") && foot.contains("FootnoteText"), foot.constData());
    }

    void convertingManyNotesGrowsInStepWithTheirNumber()
    {
        auto workFor = [&](int n) {
            QByteArray body, notes = kSeparators;
            for (int i = 0; i < n; ++i) {
                body += para(run("Text " + QByteArray::number(i)) + "<w:r><w:footnoteReference w:id=\"" + QByteArray::number(i + 2) + "\"/></w:r>");
                notes += noteXml("footnote", i + 2, "Note " + QByteArray::number(i));
            }
            auto doc = open(makeDocxNotes(body, notes, QByteArray()));
            if (!doc) return qint64(-1);
            resetNoteWorkForTest();
            const int done = convertNotes(*doc, NoteConversion::FootnotesToEndnotes);
            return done == n ? noteWorkForTest() : qint64(-1);
        };
        const qint64 small = workFor(250), big = workFor(1000);
        QVERIFY2(small > 0 && big > 0, "the notes did not all convert");
        // Four times the notes is about four times the work; quadratic work would be sixteen times.
        QVERIFY2(big < 8 * small, qPrintable(QStringLiteral("250 notes %1, 1000 notes %2").arg(small).arg(big)));
    }

    void swapAndOneNoteConvert()
    {
        const QByteArray body = para(run("A") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>" + run(" B") + "<w:r><w:endnoteReference w:id=\"2\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + noteXml("footnote", 2, "Foot");
        const QByteArray ends = "<w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote>" + noteXml("endnote", 2, "End");
        auto doc = open(makeDocxNotes(body, notes, ends));
        QVERIFY(doc);
        QCOMPARE(convertNotes(*doc, NoteConversion::Swap), 2);
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        const QByteArray f = p.part(QStringLiteral("/word/footnotes.xml")), e = p.part(QStringLiteral("/word/endnotes.xml"));
        QVERIFY(f.contains("End") && !f.contains("Foot"));
        QVERIFY(e.contains("Foot") && !e.contains(">End<") && !e.contains(" End"));
        // Just one: the footnote that is asked for.
        auto two = open(makeDocxNotes(para(run("A") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:footnoteReference w:id=\"3\"/></w:r>"),
                                      QByteArray(kSeparators) + noteXml("footnote", 2, "One") + noteXml("footnote", 3, "Two"), QByteArray()));
        QVERIFY(two);
        QCOMPARE(convertNotes(*two, NoteConversion::FootnotesToEndnotes, 3), 1);
        QCOMPARE(two->citedFootnotes().size(), 1);
        QCOMPARE(two->citedEndnotes().size(), 1);
    }

    // ---- separators
    void theNoteSeparatorsAreSetAndWrittenInTheirParts()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>" + noteXml("footnote", 2, "One");
        const QByteArray ends = "<w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote>" + noteXml("endnote", 2, "End");
        auto doc = open(makeDocxNotes(body, notes, ends));
        QVERIFY(doc);
        QCOMPARE(doc->noteSeparator(false, QStringLiteral("separator")).kind, WriteDoc::NoteSeparator::Line);
        QCOMPARE(doc->noteSeparator(false, QStringLiteral("continuationNotice")).kind, WriteDoc::NoteSeparator::None);
        WriteDoc::NoteSeparator notice;
        notice.kind = WriteDoc::NoteSeparator::Text;
        notice.text = QStringLiteral("Continued on the next page");
        QVERIFY(doc->setNoteSeparator(false, QStringLiteral("continuationNotice"), notice));
        WriteDoc::NoteSeparator words;
        words.kind = WriteDoc::NoteSeparator::Text;
        words.text = QStringLiteral("* * *");
        QVERIFY(doc->setNoteSeparator(false, QStringLiteral("continuationSeparator"), words));
        QVERIFY(doc->setNoteSeparator(true, QStringLiteral("separator"), words));
        QVERIFY(doc->isModified());
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        const QByteArray f = p.part(QStringLiteral("/word/footnotes.xml")), e = p.part(QStringLiteral("/word/endnotes.xml"));
        QVERIFY2(f.contains("w:type=\"continuationNotice\"") && f.contains("Continued on the next page"), f.constData());
        QVERIFY(f.contains("<w:separator/>"));   // (the separator, not changed, is as it was)
        QVERIFY(f.contains("* * *") && !f.contains("<w:continuationSeparator/>"));
        QVERIFY2(e.contains("* * *") && !e.contains("<w:separator/>"), e.constData());
        QVERIFY(e.contains("<w:endnoteRef/>"));
        // Read again, they are as set.
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(again->noteSeparator(false, QStringLiteral("continuationNotice")).text, notice.text);
        QCOMPARE(again->noteSeparator(false, QStringLiteral("continuationSeparator")).kind, WriteDoc::NoteSeparator::Text);
        QCOMPARE(again->noteSeparator(true, QStringLiteral("separator")).text, words.text);
        // A notice that was in the file is replaced where it is, with its Id.
        QVERIFY(again->setNoteSeparator(false, QStringLiteral("continuationNotice"), WriteDoc::NoteSeparator{WriteDoc::NoteSeparator::Text, QStringLiteral("More")}));
        QVERIFY(p.read(again->toDocx()));
        const QByteArray g = p.part(QStringLiteral("/word/footnotes.xml"));
        QCOMPARE(g.count("continuationNotice"), 1);
        QVERIFY(g.contains("More") && !g.contains("Continued on"));
        // A document with no notes has no separators to set.
        auto none = open(makeDocx(para(run("Text"))));
        QVERIFY(none);
        QVERIFY(!none->setNoteSeparator(false, QStringLiteral("continuationNotice"), notice));
    }

    void theSeparatorsDialogAndTheConvertDialogWork()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + styledNote("footnote", 2, "One");
        WriteWindow w;
        w.setDocument(open(makeDocxNotes(body, notes, QByteArray())), QString());
        w.show();
        onNextDialog(QStringLiteral("note-separators-dialog"), [](QDialog *d) {
            auto *kind = d->findChild<QComboBox *>(QStringLiteral("footcontinuationNoticeKind"));
            kind->setCurrentIndex(kind->findData(int(WriteDoc::NoteSeparator::Text)));
            d->findChild<QLineEdit *>(QStringLiteral("footcontinuationNoticeText"))->setText(QStringLiteral("Continued"));
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        bool endnotes = true;
        QVERIFY(noteSeparatorsDialog(&w, w.document(), &endnotes));
        QVERIFY(!endnotes);
        QCOMPARE(w.document()->noteSeparator(false, QStringLiteral("continuationNotice")).text, QStringLiteral("Continued"));
        onNextDialog(QStringLiteral("convert-notes-dialog"), [](QDialog *d) {
            QVERIFY(!d->findChild<QRadioButton *>(QStringLiteral("convertOne"))->isEnabled());
            d->findChild<QRadioButton *>(QStringLiteral("convertToEndnotes"))->setChecked(true);
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        QVERIFY(convertNotesDialog(&w, w.document(), w.view()));
        QCOMPARE(w.document()->citedEndnotes().size(), 1);
        QVERIFY(w.document()->citedFootnotes().isEmpty());
    }

    // ---- the dialogs
    void theCaptionDialogPutsTheCaptionWhereTheCaretIs()
    {
        WriteWindow w;
        w.setDocument(open(makeDocx(para(run("A picture")) + para(run("After")))), QString());
        w.show();
        QTextCursor c(w.document()->story());
        c.setPosition(3);
        w.view()->setCursor(c);
        onNextDialog(QStringLiteral("caption-dialog"), [](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("captionText"))->setText(QStringLiteral(": Sunrise"));
            d->findChild<QCheckBox *>(QStringLiteral("captionChapter"))->setChecked(false);
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        QVERIFY(captionDialog(&w, w.document(), w.view()));
        QCOMPARE(lines(*w.document())[1], QStringLiteral("Figure 1: Sunrise"));
        // A label the box has not seen is the document's own from then on.
        onNextDialog(QStringLiteral("caption-dialog-2"), [](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("captionLabel"))->setCurrentText(QStringLiteral("Chart"));
            d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        QVERIFY(captionDialog(&w, w.document(), w.view()));
        QVERIFY(captionLabelChoices(*w.document()).contains(QStringLiteral("Chart")));
    }

    void theIndexDialogsMarkAndInsert()
    {
        WriteWindow w;
        w.setDocument(open(makeDocx(para(run("Apples and apples")) + para(run("End")))), QString());
        w.show();
        QTextCursor c(w.document()->story());
        c.setPosition(0);
        c.setPosition(6, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        onNextDialog(QStringLiteral("mark-entry-dialog"), [](QDialog *d) {
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("indexMain"))->text(), QStringLiteral("Apples"));
            d->findChild<QCheckBox *>(QStringLiteral("indexBold"))->setChecked(true);
            for (auto *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Mark")) b->click();
        });
        QVERIFY(markIndexEntryDialog(&w, w.document(), w.view()));
        QVERIFY(markedIndexEntries(*w.document()).contains(QStringLiteral("Apples")));
        c = QTextCursor(w.document()->story());
        c.setPosition(w.document()->story()->characterCount() - 1);
        w.view()->setCursor(c);
        onNextDialog(QStringLiteral("index-dialog"), [](QDialog *d) {
            d->findChild<QSpinBox *>(QStringLiteral("indexColumns"))->setValue(1);
            d->accept();
        });
        QVERIFY(indexDialog(&w, w.document(), w.view()));
        QVERIFY2(lines(*w.document()).join('|').contains(QStringLiteral("Apples")), qPrintable(lines(*w.document()).join('|')));
        QVERIFY(documentXml(w.document()->toDocx()).contains("INDEX \\h \"A\" \\e"));
    }

    void theNotesOptionsDialogSetsTheNumbering()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + noteXml("footnote", 2, "One");
        WriteWindow w;
        w.setDocument(open(makeDocxNotes(body, notes, QByteArray())), QString());
        w.show();
        onNextDialog(QStringLiteral("notes-options-dialog"), [](QDialog *d) {
            auto *format = d->findChild<QComboBox *>(QStringLiteral("footnoteFormat"));
            format->setCurrentIndex(format->findData(QStringLiteral("upperRoman")));
            d->findChild<QSpinBox *>(QStringLiteral("footnoteStart"))->setValue(4);
            auto *restart = d->findChild<QComboBox *>(QStringLiteral("endnoteRestart"));
            restart->setCurrentIndex(restart->findData(QStringLiteral("eachSect")));
            d->accept();
        });
        bool endnotes = false;
        QVERIFY(notesOptionsDialog(&w, w.document(), &endnotes));
        QVERIFY(endnotes);
        QCOMPARE(w.document()->noteSettings(false).format, QStringLiteral("upperRoman"));
        QCOMPARE(w.document()->noteSettings(false).start, 4);
        QCOMPARE(w.document()->noteSettings(true).restart, QStringLiteral("eachSect"));
        QVERIFY(shownMarks(*w.document(), 0).contains(QStringLiteral("IV")));
    }

    // ---- the work a pass does is bounded (a hostile file has thousands of marks and of tables)
    void marksAreReadOncePerPassNotOncePerTable()
    {
        const int n = 400;
        QByteArray body;
        for (int i = 0; i < n; ++i) {
            body += para(run("w") + field("XE \"Term" + QByteArray::number(i) + "\"", "") + field("TA \\l \"Case " + QByteArray::number(i) + "\" \\c 1", "") + field("TC \"Entry\" \\f \"t\"", "") +
                         field("SEQ Figure \\* ARABIC", "1") + field("CITATION Smi20", ""));
        }
        // (Tables that find nothing of what they list: the cost left is reading what they look through.)
        for (int i = 0; i < n; ++i)
            body += para(field("INDEX \\f \"q\"", "x")) + para(field("TOA \\c 16", "x")) + para(field("TOC \\f \"q\"", "x")) + para(field("TOC \\c \"Nothing\"", "x")) + para(field("BIBLIOGRAPHY \\l 1033", "x"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        Source book;
        book.tag = QStringLiteral("Smi20");
        book.authors = {Person{QStringLiteral("Smith"), QStringLiteral("John"), QString()}};
        book.fields = {{QStringLiteral("Title"), QStringLiteral("A Book")}, {QStringLiteral("Year"), QStringLiteral("2020")}};
        setDocumentSources(*doc, {book});
        resetParseCodeCallsForTest();
        QElapsedTimer timer;
        timer.start();
        updateFields(*doc);
        // Each instruction is read a few times for the pass (once for itself, once for the list of its kind, at most three passes), not once for each of the 2,000 tables.
        QVERIFY2(parseCodeCallsForTest() <= 40000, qPrintable(QStringLiteral("%1 instructions read").arg(parseCodeCallsForTest())));
        QVERIFY2(timer.elapsed() < 60000, qPrintable(QStringLiteral("%1 ms").arg(timer.elapsed())));
    }

    void markAllMakesOneScanAndOneStep()
    {
        const int n = 600;
        QByteArray text;
        for (int i = 0; i < n; ++i) text += "word " + QByteArray::number(i) + " ";
        auto doc = open(makeDocx(para(run(text))));
        QVERIFY(doc);
        IndexMark m;
        m.main = QStringLiteral("word");
        resetStoryScansForTest();
        QCOMPARE(markAllIndexEntries(*doc, m), n);
        QVERIFY2(storyScansForTest() <= 3, qPrintable(QStringLiteral("%1 scans").arg(storyScansForTest())));
        QCOMPARE(markedIndexEntries(*doc), QStringList{QStringLiteral("word")});
        QVERIFY(doc->undo());
        QVERIFY(!documentXml(doc->toDocx()).contains("XE \"word\""));
    }

    void ieeeNumbersAreWorkedOutForEachTextAndNotKeptFromTheLastOne()
    {
        Source a, b;
        a.tag = QStringLiteral("Aaa");
        a.fields = {{QStringLiteral("Title"), QStringLiteral("One")}};
        b.tag = QStringLiteral("Bbb");
        b.fields = {{QStringLiteral("Title"), QStringLiteral("Two")}};
        auto numbers = [&](const QByteArray &first, const QByteArray &second) {
            auto doc = open(makeDocx(para(field("CITATION " + first, "") + field("CITATION " + second, ""))));
            if (!doc) return QString();
            setDocumentSources(*doc, {a, b});
            setDocumentCitationStyle(*doc, QStringLiteral("IEEE"));
            return lines(*doc).value(0);
        };
        QCOMPARE(numbers("Aaa", "Bbb"), QStringLiteral("[1][2]"));
        QCOMPARE(numbers("Bbb", "Aaa"), QStringLiteral("[1][2]"));   // (the numbers go in the order of the first citations: a new text is not numbered as the last one was)
        QCOMPARE(numbers("Bbb", "Bbb"), QStringLiteral("[1][1]"));
    }

    // A scan that takes the place of another (the same variable, the same number of fields) is not numbered as the one before it was.
    void ieeeNumbersOfANewScanAtTheSameAddressAreItsOwn()
    {
        Source a, b;
        a.tag = QStringLiteral("Aaa");
        a.fields = {{QStringLiteral("Title"), QStringLiteral("One")}};
        b.tag = QStringLiteral("Bbb");
        b.fields = {{QStringLiteral("Title"), QStringLiteral("Two")}};
        auto first = open(makeDocx(para(field("CITATION Aaa", "") + field("CITATION Bbb", ""))));
        auto second = open(makeDocx(para(field("CITATION Bbb", "") + field("CITATION Aaa", ""))));
        QVERIFY(first && second);
        for (auto *d : {first.get(), second.get()}) {
            setDocumentSources(*d, {a, b});
            setDocumentCitationStyle(*d, QStringLiteral("IEEE"));
        }
        StoryScan scan = scanStory(first->story());
        const fields::Code code = fields::parseCode(QStringLiteral("CITATION Bbb"));
        QCOMPARE(citationFieldText(*first, scan, QStringLiteral("Bbb"), code).value_or(QString()), QStringLiteral("[2]"));
        scan = scanStory(second->story());
        QCOMPARE(citationFieldText(*second, scan, QStringLiteral("Bbb"), code).value_or(QString()), QStringLiteral("[1]"));
    }

    void aSourcesPartTooLargeIsIgnoredAndTheFirstSourceOfATagIsTheOne()
    {
        auto doc = open(makeDocx(para(run("Text"))));
        QVERIFY(doc);
        Source one, two;
        one.tag = two.tag = QStringLiteral("Dup");
        one.fields = {{QStringLiteral("Title"), QStringLiteral("First")}};
        two.fields = {{QStringLiteral("Title"), QStringLiteral("Second")}};
        setDocumentSources(*doc, {one, two});
        QTextCursor c(doc->story());
        c.setPosition(2);
        QVERIFY(insertCitation(*doc, c, QStringLiteral("dup")));   // (a tag is found whatever its case)
        QVERIFY2(lines(*doc)[0].contains(QStringLiteral("First")) || lines(*doc)[0].contains(QStringLiteral("Dup")), qPrintable(lines(*doc)[0]));
        QVERIFY(!lines(*doc)[0].contains(QStringLiteral("Second")));
        // A part past the most that is read is left out, with a warning.
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + para(run("T")) + "</w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        QByteArray huge = "<b:Sources xmlns:b=\"http://schemas.openxmlformats.org/officeDocument/2006/bibliography\"><b:Source><b:Tag>Big</b:Tag><b:Title>" + QByteArray(17 * 1024 * 1024, 'x') + "</b:Title></b:Source></b:Sources>";
        w.addPart(QStringLiteral("/customXml/item1.xml"), huge, QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("customXml"), QStringLiteral("/customXml/item1.xml"));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("larger than 16 MB")));
        auto big = open(w.finish());
        QVERIFY(big);
        QCOMPARE(documentSources(*big).size(), 0);
    }

    void clearingTheSourcesLeavesAnEmptyListNotAnEmptyFile()
    {
        auto doc = open(makeDocx(para(run("Text"))));
        QVERIFY(doc);
        Source one;
        one.tag = QStringLiteral("One");
        one.fields = {{QStringLiteral("Title"), QStringLiteral("Title")}};
        setDocumentSources(*doc, {one});
        auto reopened = open(doc->toDocx());
        QVERIFY(reopened);
        QCOMPARE(documentSources(*reopened).size(), 1);
        reopened->setSourcesXml(QByteArray());
        const QByteArray zip = reopened->toDocx();
        office::Package p;
        QVERIFY(p.read(zip));
        const QByteArray item = p.part(QStringLiteral("/customXml/item1.xml"));
        QVERIFY2(item.contains("Sources") && item.contains("officeDocument/2006/bibliography"), item.constData());
        QVERIFY(office::wml::XmlTree().parse(item));
        auto again = open(zip);
        QVERIFY(again);
        QCOMPARE(documentSources(*again).size(), 0);
    }

    // ---- built-in styles come only from our own commands
    void aStyleNamedButNotDefinedStaysNormalUntilACommandAddsIt()
    {
        auto doc = open(makeDocx(para(run("Words"), "<w:pStyle w:val=\"Caption\"/>") + para(run("Index"), "<w:pStyle w:val=\"Index1\"/>")));
        QVERIFY(doc);
        QVERIFY(!doc->styleSheet().find(QStringLiteral("Caption")));
        const QString normal = doc->defaultParagraphStyle();
        for (const QString &id : {QStringLiteral("Caption"), QStringLiteral("Index1"), QStringLiteral("TableofFigures"), QStringLiteral("Bibliography"), QStringLiteral("TOAHeading")}) {
            QCOMPARE(doc->styleCharFormat(id).fontItalic(), doc->styleCharFormat(normal).fontItalic());
            QCOMPARE(doc->styleCharFormat(id).fontPointSize(), doc->styleCharFormat(normal).fontPointSize());
            QCOMPARE(doc->styleBlockFormat(id).bottomMargin(), doc->styleBlockFormat(normal).bottomMargin());
        }
        // Saved, the file still does not define it.
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        QVERIFY(!p.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Caption\""));
        // A caption our command makes brings the style, defined in the file.
        QTextCursor c(doc->story());
        c.setPosition(2);
        QVERIFY(insertCaption(*doc, c, CaptionOptions()));
        QVERIFY(doc->styleSheet().find(QStringLiteral("Caption")));
        QVERIFY(p.read(doc->toDocx()));
        const QByteArray styles = p.part(QStringLiteral("/word/styles.xml"));
        QVERIFY2(styles.contains("w:styleId=\"Caption\"") && styles.contains("<w:name w:val=\"caption\"/>"), styles.constData());
        QVERIFY(doc->styleCharFormat(QStringLiteral("Caption")).fontItalic());
    }

    void aStyleCreatedWithTheNameCaptionIsTheOneCaptionsUse()
    {
        auto doc = open(makeDocx(para(run("A picture"))));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        office::wml::Style def;
        def.type = QStringLiteral("paragraph");
        def.name = QStringLiteral("Caption");
        def.basedOn = QStringLiteral("Normal");
        def.rPr.bold = true;
        QString problem;
        const QString id = createStyle(*doc, c, def, &problem);
        QVERIFY2(!id.isEmpty(), qPrintable(problem));
        QCOMPARE(id, QStringLiteral("Caption"));
        c.setPosition(3);
        QVERIFY(insertCaption(*doc, c, CaptionOptions()));
        QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().stringProperty(tp::StyleName), id);
        QVERIFY(doc->styleCharFormat(id).fontWeight() >= QFont::Bold);
    }

    // ---- the separators the part lacked, named where the settings name them
    void aSeparatorThePartLackedIsAddedWithTheIdTheSettingsNameItBy()
    {
        const QByteArray body = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + noteXml("footnote", 2, "One");   // (no continuation separator)
        auto doc = open(makeDocxNotes(body, notes, QByteArray()));
        QVERIFY(doc);
        WriteDoc::NoteSeparator words, notice;
        words.kind = WriteDoc::NoteSeparator::Text;
        words.text = QStringLiteral("* * *");
        notice.kind = WriteDoc::NoteSeparator::Text;
        notice.text = QStringLiteral("More");
        QVERIFY(doc->setNoteSeparator(false, QStringLiteral("continuationSeparator"), words));
        QVERIFY(doc->setNoteSeparator(false, QStringLiteral("continuationNotice"), notice));
        const QByteArray zip = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(zip));
        const QByteArray f = p.part(QStringLiteral("/word/footnotes.xml")), settings = p.part(QStringLiteral("/word/settings.xml"));
        QVERIFY2(f.contains("w:type=\"continuationSeparator\" w:id=\"0\""), f.constData());
        QVERIFY2(settings.contains("<w:footnotePr>") && settings.contains("<w:footnote w:id=\"-1\"/>") && settings.contains("<w:footnote w:id=\"0\"/>"), settings.constData());
        // The Ids are one to a note, and the ones the settings name are in the part.
        QSet<QString> ids;
        for (int at = f.indexOf("<w:footnote "); at >= 0; at = f.indexOf("<w:footnote ", at + 1)) {
            const int idAt = f.indexOf("w:id=\"", at);
            const QString id = QString::fromLatin1(f.mid(idAt + 6, f.indexOf('"', idAt + 6) - idAt - 6));
            QVERIFY2(!ids.contains(id), qPrintable(id));
            ids.insert(id);
        }
        QVERIFY2(ids.contains(QStringLiteral("-1")) && ids.contains(QStringLiteral("0")) && ids.contains(QStringLiteral("2")) && ids.size() == 4, qPrintable(QStringList(ids.values()).join(',')));
        // Read and written again, nothing changes.
        auto again = open(zip);
        QVERIFY(again);
        QCOMPARE(again->noteSeparator(false, QStringLiteral("continuationSeparator")).text, words.text);
        QCOMPARE(again->noteSeparator(false, QStringLiteral("continuationNotice")).text, notice.text);
        QVERIFY(p.read(again->toDocx()));
        QCOMPARE(p.part(QStringLiteral("/word/footnotes.xml")), f);
        QCOMPARE(p.part(QStringLiteral("/word/settings.xml")).count("<w:footnotePr>"), 1);
    }

    void aNotesSettingsPartTheNumberingCannotBePutInIsSaidSoAtTheEdit()
    {
        auto good = open(makeDocxNotes(para(run("T")), QByteArray(), QByteArray(), "<w:zoom w:percent=\"100\"/>"));
        QVERIFY(good);
        QVERIFY(good->noteSettingsCanBeSaved());
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + para(run("T")) + "</w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/settings.xml"), QByteArray("<w:settings ") + kNs + "/>", QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
        auto bad = open(w.finish());
        QVERIFY(bad);
        QVERIFY(!bad->noteSettingsCanBeSaved());
        // Both notes dialogs say so, and change nothing.
        WriteWindow window;
        window.setDocument(std::move(bad), QString());
        window.show();
        for (const bool separators : {false, true}) {
            QString said;
            onNextDialog(QStringLiteral("settings-warning"), [&said](QDialog *d) {
                if (auto *box = qobject_cast<QMessageBox *>(d)) said = box->text();
                d->accept();
            });
            bool endnotes = false;
            QVERIFY(!(separators ? noteSeparatorsDialog(&window, window.document(), &endnotes) : notesOptionsDialog(&window, window.document(), &endnotes)));
            QVERIFY2(said.contains(QStringLiteral("can't be read")), qPrintable(said));
        }
    }

    // ---- notes on the page
    // The reference to a note in the text is raised (superscript) where it is laid out, for a note our command makes and for one a file has with the style it names.
    void aNoteReferenceInTheTextIsRaised()
    {
        auto raisedMarks = [](WriteDoc &doc, int page) {
            QList<bool> out;
            const auto mat = doc.pager().materialize(page);
            for (const auto &item : mat->doc->pages[0]->items)
                if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                    if (const Story *s = mat->doc->story(t->storyId))
                        for (QTextBlock b = s->doc->begin(); b.isValid(); b = b.next())
                            for (auto it = b.begin(); !it.atEnd(); ++it) {
                                const QTextCharFormat cf = it.fragment().charFormat();
                                if (cf.stringProperty(tp::Field).startsWith(QLatin1String("raw:")))
                                    out << (cf.verticalAlignment() == QTextCharFormat::AlignSuperScript);
                            }
            return out;
        };
        // From a file: the run names the style, and the styles part defines it.
        const QByteArray styles = QByteArray(kStyles) + "<w:style w:type=\"character\" w:styleId=\"FootnoteReference\"><w:name w:val=\"footnote reference\"/><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr></w:style>";
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString ct = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
        const QByteArray body = para(run("Text") + "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"2\"/></w:r>");
        w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"), ct + QStringLiteral("document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/styles.xml"), xml(QByteArray("<w:styles ") + kNs + ">" + styles + "</w:styles>"), ct + QStringLiteral("styles+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
        w.addPart(QStringLiteral("/word/footnotes.xml"), xml(QByteArray("<w:footnotes ") + kNs + ">" + kSeparators + noteXml("footnote", 2, "One") + "</w:footnotes>"), ct + QStringLiteral("footnotes+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
        auto fromFile = open(w.finish());
        QVERIFY(fromFile);
        QCOMPARE(raisedMarks(*fromFile, 0).value(0), true);   // (the reference; the mark that begins the note has no style in this file)
        // A run that names no style is raised too, and is saved as it was.
        const QByteArray plainBody = para(run("Text") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>");
        const QByteArray notes = QByteArray(kSeparators) + noteXml("footnote", 2, "One");
        auto plain = open(makeDocxNotes(plainBody, notes, QByteArray()));
        QVERIFY(plain);
        QCOMPARE(raisedMarks(*plain, 0).value(0), true);
        const QByteArray savedXml = documentXml(plain->toDocx());
        QVERIFY2(savedXml.contains("<w:r><w:footnoteReference w:id=\"2\"/></w:r>"), savedXml.mid(savedXml.indexOf("<w:body>"), 500).constData());
        // An endnote's reference is raised the same way, and saved as it was.
        const QByteArray endBody = para(run("Text") + "<w:r><w:endnoteReference w:id=\"2\"/></w:r>");
        auto endPlain = open(makeDocxNotes(endBody, QByteArray(), noteXml("endnote", 2, "End")));
        QVERIFY(endPlain);
        QCOMPARE(raisedMarks(*endPlain, 0).value(0), true);
        QVERIFY(documentXml(endPlain->toDocx()).contains("<w:r><w:endnoteReference w:id=\"2\"/></w:r>"));
        // From our command, in a document whose file has no such style.
        auto made = open(makeDocx(para(run("Text"))));
        QVERIFY(made);
        QTextCursor c(made->story());
        c.setPosition(5);
        QVERIFY(insertFootnote(*made, c));
        QCOMPARE(raisedMarks(*made, 0), (QList<bool>{true, true}));
        auto reopened = open(made->toDocx());
        QVERIFY(reopened);
        QCOMPARE(raisedMarks(*reopened, 0), (QList<bool>{true, true}));
    }

    // A footnote our command makes is set at the foot of the page its reference is on, under a rule, and is gone from the pages that do not cite it.
    void aFootnoteMadeByTheCommandIsAtTheFootOfItsPage()
    {
        QByteArray text;
        for (int i = 0; i < 700; ++i) text += "Line of the text number " + QByteArray::number(i) + " goes on. ";
        auto doc = open(makeDocxNotes(para(run("First paragraph.")) + para(run(text)), QByteArray(), QByteArray()));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 2);
        QTextCursor c(doc->story());
        c.setPosition(6);
        QTextDocument *note = insertFootnote(*doc, c);
        QVERIFY(note);
        QTextCursor in(note);
        in.movePosition(QTextCursor::End);
        in.insertText(QStringLiteral("MADE NOTE says something."));
        const auto page = doc->pager().materialize(0);
        const TextItem *box = nullptr;
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (const Story *s = page->doc->story(t->storyId))
                    if (s->doc->toPlainText().contains(QLatin1String("MADE NOTE"))) box = t;
        QVERIFY(box);
        QVERIFY2(std::abs(box->rect.bottom() - (792 - 72)) < 0.1, qPrintable(QString::number(box->rect.bottom())));
        bool rule = false;
        for (const auto &item : page->doc->pages[0]->items) rule |= dynamic_cast<const LineItem *>(item.get()) != nullptr;
        QVERIFY(rule);
        const auto next = doc->pager().materialize(1);
        for (const auto &item : next->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (const Story *s = next->doc->story(t->storyId)) QVERIFY(!s->doc->toPlainText().contains(QLatin1String("MADE NOTE")));
    }

    // An endnote's part of the separators dialog has the separator only: no page breaks an endnote.
    void theSeparatorsDialogOffersAnEndnoteOnlyItsSeparator()
    {
        WriteWindow w;
        w.setDocument(open(makeDocxNotes(para(run("Text") + "<w:r><w:endnoteReference w:id=\"2\"/></w:r>"), QByteArray(), noteXml("endnote", 2, "End"))), QString());
        w.show();
        bool seen = false;
        onNextDialog(QStringLiteral("note-separators-endnotes"), [&](QDialog *d) {
            seen = d->findChild<QComboBox *>(QStringLiteral("footcontinuationNoticeKind")) && d->findChild<QComboBox *>(QStringLiteral("endseparatorKind"));
            QVERIFY(!d->findChild<QComboBox *>(QStringLiteral("endcontinuationNoticeKind")));
            QVERIFY(!d->findChild<QComboBox *>(QStringLiteral("endcontinuationSeparatorKind")));
            d->reject();
        });
        noteSeparatorsDialog(&w, w.document(), nullptr);
        QVERIFY(seen);
    }

    // Each group of the References tab shows with its own heading: none is folded into one button (which has no heading above it) in a window wide enough for them, and at the usual
    // 1440 where the system's font is the one this was measured with (labels are as wide as the platform's font makes them).
    void everyGroupOfTheReferencesTabShowsItsHeading()
    {
        for (const int width : {2200, 1440}) {
#ifndef Q_OS_LINUX
            if (width < 2000) continue;
#endif
            WriteWindow w;
            w.resize(width, 900);
            w.show();
            Ribbon *ribbon = w.ribbon();
            RibbonTab *tab = ribbon->tab(QStringLiteral("References"));
            QVERIFY(tab);
            ribbon->showTab(tab);
            QStringList titles;
            for (RibbonGroup *g : tab->groups()) titles << g->title();
            QCOMPARE(titles, (QStringList{QStringLiteral("Table of Contents"), QStringLiteral("Footnotes"), QStringLiteral("Citations and Bibliography"), QStringLiteral("Captions"),
                                          QStringLiteral("Index"), QStringLiteral("Table of Authorities")}));
            QTRY_COMPARE_WITH_TIMEOUT(tab->collapsedCount(), 0, 5000);
            for (RibbonGroup *g : tab->groups()) QVERIFY2(g->isVisible(), qPrintable(g->title()));
        }
    }

    void theOtherReferenceDialogsOpenAndCancel()
    {
        WriteWindow w;
        w.setDocument(open(makeDocx(para(run("Text")))), QString());
        w.show();
        const QList<std::pair<QString, std::function<bool()>>> dialogs = {
            {QStringLiteral("toc-dialog"), [&] { return tableOfContentsDialog(&w, w.document(), w.view()); }},
            {QStringLiteral("toc-entry-dialog"), [&] { return tocEntryDialog(&w, w.document(), w.view()); }},
            {QStringLiteral("figures-dialog"), [&] { return tableOfFiguresDialog(&w, w.document(), w.view()); }},
            {QStringLiteral("mark-citation-dialog"), [&] { return markCitationDialog(&w, w.document(), w.view()); }},
            {QStringLiteral("authorities-dialog"), [&] { return tableOfAuthoritiesDialog(&w, w.document(), w.view()); }},
            {QStringLiteral("insert-citation-dialog"), [&] { return insertCitationDialog(&w, w.document(), w.view()); }},
            {QStringLiteral("sources-dialog"), [&] { return manageSourcesDialog(&w, w.document()); }},
        };
        for (const auto &d : dialogs) {
            onNextDialog(d.first, [](QDialog *dlg) { dlg->reject(); });
            QVERIFY2(!d.second(), qPrintable(d.first));
        }
        QVERIFY(!w.document()->canUndo());
    }
};

QTEST_MAIN(RefsTests)
#include "tst_write_refs.moc"
