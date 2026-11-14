// Tests of JeffWrite's review features (apps/write/core/reviewops.*, commentops.*): tracked changes (finding, accepting, rejecting, making), the way they are shown, and comments.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "../withdialog.h"
#include "app/writeinfo.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/reviewpane.h"
#include "app/writefindpane.h"
#include "app/writewindow.h"
#include "wml/xmltree.h"
#include "core/appinfo.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/reviewops.h"
#include "core/spellcheck.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textengine.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QTimeZone>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QDockWidget>
#include <QRegularExpression>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;
namespace wml = jp::office::wml;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

// A package of a document whose body is `body`.
QByteArray makeDocx(const QByteArray &body, const QByteArray &settings = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), xml(QByteArray("<w:settings ") + kNs + ">" + settings + "</w:settings>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    return w.finish();
}

QByteArray documentXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/document.xml"));
}

QByteArray settingsXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/settings.xml"));
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

QByteArray rev(int id, const char *author = "Ann", const char *date = "2020-01-01T00:00:00Z")
{
    return " w:id=\"" + QByteArray::number(id) + "\" w:author=\"" + author + "\" w:date=\"" + date + "\"";
}

QByteArray run(const char *text) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + text + "</w:t></w:r>"; }
QByteArray para(const QByteArray &content, const QByteArray &props = QByteArray()) { return "<w:p>" + (props.isEmpty() ? QByteArray() : "<w:pPr>" + props + "</w:pPr>") + content + "</w:p>"; }
QByteArray inserted(int id, const char *text, const char *author = "Ann") { return "<w:ins" + rev(id, author) + ">" + run(text) + "</w:ins>"; }
QByteArray deleted(int id, const char *text, const char *author = "Ann")
{
    return "<w:del" + rev(id, author) + "><w:r><w:delText xml:space=\"preserve\">" + QByteArray(text) + "</w:delText></w:r></w:del>";
}

// The look of a tracked change's run when changes are marked: its fragment's format as the text engine resolves it.
QTextCharFormat shownFormat(WriteDoc &doc, const QString &text)
{
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
            if (!it.fragment().charFormat().hasProperty(prop::Atom) && it.fragment().text() == text) return resolveCharFormat(it.fragment().charFormat(), doc.flow().env);
    return QTextCharFormat();
}

// Puts the person's choices for drawing changes back as they were when the test ends.
struct MarkupGuard {
    tp::RevisionMarkup saved = tp::revisionMarkup();
    ~MarkupGuard()
    {
        tp::revisionMarkup() = saved;
        ++tp::revisionMarkup().generation;
    }
};

QByteArray moveFromOf(const char *name, int id, const char *text) { return "<w:moveFromRangeStart w:id=\"" + QByteArray::number(id) + "\" w:name=\"" + name + "\"/><w:moveFrom" + rev(id + 1) + "><w:r><w:delText>" + text + "</w:delText></w:r></w:moveFrom><w:moveFromRangeEnd w:id=\"" + QByteArray::number(id) + "\"/>"; }
QByteArray moveToOf(const char *name, int id, const char *text) { return "<w:moveToRangeStart w:id=\"" + QByteArray::number(id) + "\" w:name=\"" + name + "\"/><w:moveTo" + rev(id + 1) + "><w:r><w:t>" + text + "</w:t></w:r></w:moveTo><w:moveToRangeEnd w:id=\"" + QByteArray::number(id) + "\"/>"; }

// The number of rows of an image that have a pixel of the strong blue the tests give the change bars, and where they are across (the mean column).
struct BarRows {
    int rows = 0;
    double meanX = 0;
};
BarRows blueBars(const QImage &img)
{
    BarRows out;
    qint64 sum = 0, n = 0;
    for (int y = 0; y < img.height(); ++y) {
        bool any = false;
        for (int x = 0; x < img.width(); ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.blue() > 200 && c.red() < 70 && c.green() < 70) {
                any = true;
                sum += x;
                ++n;
            }
        }
        if (any) ++out.rows;
    }
    out.meanX = n ? double(sum) / double(n) : 0;
    return out;
}

// What the paragraphs say when the changes are shown as the document stands: the text that is not hidden, a paragraph to a line.
QString finalText(const WriteDoc &doc)
{
    QStringList lines;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        QString line;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.hasProperty(prop::TrackedDeleted)) continue;
            QString t = it.fragment().text();
            t.remove(QChar::ObjectReplacementCharacter);
            line += t;
        }
        lines << line;
    }
    return lines.join(QLatin1Char('\n'));
}

// The same with the changes' deletions in their places.
// What the lines of the text say, each line's words, as the engine lays the story out with the way changes are shown now.
QStringList lineTexts(WriteDoc &doc)
{
    FrameSpec fs;
    fs.size = QSizeF(468, 600);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(doc.story(), {fs}, doc.flow().env);
    QStringList lines;
    for (const auto &l : layout.lineInfo(0)) lines << QString(l.text).remove(QChar(0x2060)).trimmed();
    return lines;
}

QStringList flaggedWords(SpellCheck &sc, WriteDoc &doc)
{
    const QString text = doc.story()->toPlainText();
    QStringList out;
    for (const auto &r : sc.misspelledIn(0, doc.story()->characterCount())) out << text.mid(r.first, r.second - r.first);
    return out;
}

QString comments_text(WriteDoc &doc) { return comments(doc).value(0).text; }
bool comments_done(WriteDoc &doc, int id) { return commentById(doc, id) && commentById(doc, id)->done; }

QString textWithDeletions(const WriteDoc &doc)
{
    QStringList lines;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        QString t = b.text();
        t.remove(QChar::ObjectReplacementCharacter);
        lines << t;
    }
    return lines.join(QLatin1Char('\n'));
}

// A package with comments: the document, and the comments part with its extensions. `parts` are (name, relationship type's end, content XML without the root).
QByteArray makeDocxWithComments(const QByteArray &body, const QByteArray &comments, const QByteArray &extended = QByteArray(), const QByteArray &ids = QByteArray(), const QByteArray &people = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:w15=\"http://schemas.microsoft.com/office/word/2012/wordml\" xmlns:w16cid=\"http://schemas.microsoft.com/office/word/2016/wordml/cid\"";
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/comments.xml"), xml("<w:comments " + ns + ">" + comments + "</w:comments>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.comments+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("comments"), QStringLiteral("/word/comments.xml"));
    if (!extended.isEmpty()) {
        w.addPart(QStringLiteral("/word/commentsExtended.xml"), xml("<w15:commentsEx " + ns + ">" + extended + "</w15:commentsEx>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.commentsExtended+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.microsoft.com/office/2011/relationships/commentsExtended"), QStringLiteral("/word/commentsExtended.xml"));
    }
    if (!ids.isEmpty()) {
        w.addPart(QStringLiteral("/word/commentsIds.xml"), xml("<w16cid:commentsIds " + ns + ">" + ids + "</w16cid:commentsIds>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.commentsIds+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.microsoft.com/office/2016/09/relationships/commentsIds"), QStringLiteral("/word/commentsIds.xml"));
    }
    if (!people.isEmpty()) {
        w.addPart(QStringLiteral("/word/people.xml"), xml("<w15:people " + ns + ">" + people + "</w15:people>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.people+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), QStringLiteral("http://schemas.microsoft.com/office/2011/relationships/people"), QStringLiteral("/word/people.xml"));
    }
    return w.finish();
}

QByteArray partOf(const QByteArray &zip, const char *name)
{
    office::Package p;
    p.read(zip);
    return p.part(QString::fromLatin1(name));
}

QByteArray commentElement(int id, const char *author, const char *text, const char *paraId, const char *date = "2020-02-02T10:00:00Z")
{
    return "<w:comment w:id=\"" + QByteArray::number(id) + "\" w:author=\"" + author + "\" w:date=\"" + date + "\" w:initials=\"AB\"><w:p w14:paraId=\"" + paraId +
           "\" w14:textId=\"77777777\"><w:r><w:annotationRef/></w:r><w:r><w:t>" + text + "</w:t></w:r></w:p></w:comment>";
}

// A paragraph with a comment on the words in it.
QByteArray commented(int id, const char *words) { return "<w:commentRangeStart w:id=\"" + QByteArray::number(id) + "\"/>" + run(words) + "<w:commentRangeEnd w:id=\"" + QByteArray::number(id) + "\"/><w:r><w:commentReference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; }

} // namespace

class ReviewTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // The run's settings are its own, and none reached the person's (checked at the end).
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    // ---------------------------------------------------------------- finding changes
    void aDocumentsChangesAreListedWithTheirAuthorsDatesAndWords()
    {
        auto doc = open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob") + run("four"))));
        QVERIFY(doc);
        const QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), 2);
        QCOMPARE(all[0].kind, Revision::Insertion);
        QCOMPARE(all[0].author, QStringLiteral("Ann"));
        QCOMPARE(all[0].date, QDateTime(QDate(2020, 1, 1), QTime(0, 0), QTimeZone::UTC));
        QCOMPARE(revisionText(doc->story(), all[0]), QStringLiteral("two "));
        QCOMPARE(all[1].kind, Revision::Deletion);
        QCOMPARE(all[1].author, QStringLiteral("Bob"));
        QCOMPARE(revisionText(doc->story(), all[1]), QStringLiteral("three "));
        QCOMPARE(revisionCount(doc->story()), 2);
        QCOMPARE(revisionAuthors(doc->story()), QStringList({QStringLiteral("Ann"), QStringLiteral("Bob")}));
        // The deleted words are the paragraph's text, hidden from the lines.
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("one two three four"));
        QCOMPARE(finalText(*doc), QStringLiteral("one two four"));
    }

    void aTrackedNumberingMarkIsWrittenBack()
    {
        // The mark of a numbering that was added while changes were tracked (w:ins in w:numPr) is part of the paragraph's numbering, and stays with it.
        const QByteArray mark = "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/><w:ins" + rev(5) + "/></w:numPr>";
        auto doc = open(makeDocx(para(run("item"), mark) + para(run("bare"), "<w:numPr><w:ins" + rev(6) + "/></w:numPr>") + para(run("plain"))));
        QVERIFY(doc);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(x.count("<w:numPr>"), 2);
        QVERIFY2(x.contains("<w:ins w:id=\"5\"") && x.contains("<w:ins w:id=\"6\""), x.constData());
        QVERIFY2(QRegularExpression("<w:numPr><w:ins w:id=\"5\"[^>]*/></w:numPr>").match(QString::fromUtf8(x)).hasMatch(), x.constData());
    }

    void anRtfsTrackedChangesAreTrackedChanges()
    {
        const QByteArray rtf = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Arial;}}{\\*\\revtbl{Unknown;}{Ann;}}\\pard Keep {\\revised\\revauth1 added}{\\deleted\\revauthdel1 gone} end\\par}";
        QString err;
        auto doc = WriteDoc::fromFile(rtf, &err);
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(revisionCount(doc->story()), 2);
        QCOMPARE(finalText(*doc), QStringLiteral("Keep added end"));
        QCOMPARE(revisionAuthors(doc->story()), QStringList{QStringLiteral("Ann")});
        QCOMPARE(acceptAll(*doc), 2);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("Keep added end"));
    }

    // ---------------------------------------------------------------- Accept and Reject
    void acceptAllKeepsInsertionsAndTakesDeletionsOut()
    {
        auto doc = open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ") + run("four"))));
        QVERIFY(doc);
        QCOMPARE(acceptAll(*doc), 2);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("one two four"));
        QCOMPARE(revisionCount(doc->story()), 0);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("<w:ins") && !x.contains("<w:del") && x.contains("two "), x.constData());
    }

    void rejectAllTakesInsertionsOutAndKeepsDeletions()
    {
        auto doc = open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ") + run("four"))));
        QVERIFY(doc);
        QCOMPARE(rejectAll(*doc), 2);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("one three four"));
        QCOMPARE(finalText(*doc), QStringLiteral("one three four"));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("<w:ins") && !x.contains("<w:del") && x.contains("three "), x.constData());
        // (The words are no longer hidden: they are text of the paragraph.)
        QVERIFY(!x.contains("vanish"));
    }

    void anInsertionThatWasDeletedIsGoneWhicheverWayItGoes()
    {
        const QByteArray both = "<w:ins" + rev(1) + "><w:del" + rev(2, "Bob") + "><w:r><w:delText>both</w:delText></w:r></w:del></w:ins>";
        for (const bool accept : {true, false}) {
            auto doc = open(makeDocx(para(run("a ") + both + run(" b"))));
            QVERIFY(doc);
            QCOMPARE(accept ? acceptAll(*doc) : rejectAll(*doc), 2);
            QCOMPARE(textWithDeletions(*doc), QStringLiteral("a  b"));
            QCOMPARE(revisionCount(doc->story()), 0);
        }
        // A deletion of someone's insertion, rejected, leaves the insertion.
        auto doc = open(makeDocx(para(run("a ") + both + run(" b"))));
        const QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), 2);
        QVector<Revision> deletions;
        for (const Revision &r : all)
            if (r.kind == Revision::Deletion) deletions << r;
        QCOMPARE(resolveRevisions(*doc, doc->story(), deletions, false), 1);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("a both b"));
        QCOMPARE(finalText(*doc), QStringLiteral("a both b"));
        QCOMPARE(revisionCount(doc->story()), 1);
    }

    void aDeletedParagraphMarkJoinsTheParagraphToTheNextWhenAccepted()
    {
        const QByteArray body = para(run("one ") + deleted(1, "gone"), "<w:rPr><w:del" + rev(2) + "/></w:rPr>") + para(run("two"));
        {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QCOMPARE(acceptAll(*doc), 2);
            QCOMPARE(textWithDeletions(*doc), QStringLiteral("one two"));
            QCOMPARE(doc->story()->blockCount(), 1);
        }
        {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QCOMPARE(rejectAll(*doc), 2);
            QCOMPARE(textWithDeletions(*doc), QStringLiteral("one gone\ntwo"));
            QCOMPARE(doc->story()->blockCount(), 2);
            const QByteArray x = documentXml(doc->toDocx());
            QVERIFY2(!x.contains("<w:del"), x.constData());
        }
    }

    void anInsertedParagraphMarkIsTakenAwayWhenRejected()
    {
        // "AAA" and "BBB" were one paragraph; Enter between them made the mark after "AAA" a new one.
        const QByteArray body = para(inserted(1, "AAA"), "<w:rPr><w:ins" + rev(2) + "/></w:rPr>") + para(run("BBB"));
        {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QCOMPARE(rejectAll(*doc), 2);
            QCOMPARE(textWithDeletions(*doc), QStringLiteral("BBB"));
            QCOMPARE(doc->story()->blockCount(), 1);
        }
        {
            auto doc = open(makeDocx(body));
            QVERIFY(doc);
            QCOMPARE(acceptAll(*doc), 2);
            QCOMPARE(textWithDeletions(*doc), QStringLiteral("AAA\nBBB"));
            const QByteArray x = documentXml(doc->toDocx());
            QVERIFY2(!x.contains("<w:ins"), x.constData());
        }
    }

    void aParagraphWhollyDeletedLeavesTheNextOneItsOwnLook()
    {
        const QByteArray body = para(run("keep")) + para(deleted(1, "all of it"), "<w:jc w:val=\"center\"/><w:rPr><w:del" + rev(2) + "/></w:rPr>") + para(run("tail"), "<w:jc w:val=\"right\"/>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QCOMPARE(acceptAll(*doc), 2);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("keep\ntail"));
        QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().alignment(), Qt::AlignRight);
    }

    void aChangeOfRunPropertiesIsAcceptedByForgettingItAndRejectedByGivingTheOldOnesBack()
    {
        const QByteArray bold = "<w:r><w:rPr><w:b/><w:rPrChange" + rev(1) + "><w:rPr><w:i/></w:rPr></w:rPrChange></w:rPr><w:t>word</w:t></w:r>";
        auto doc = open(makeDocx(para(run("a ") + bold)));
        QVERIFY(doc);
        QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), 1);
        QCOMPARE(all[0].kind, Revision::Format);
        QCOMPARE(all[0].author, QStringLiteral("Ann"));
        auto weightAt = [&](int pos) {
            QTextCursor c(doc->story());
            c.setPosition(pos + 1);
            return c.charFormat();
        };
        QVERIFY(weightAt(2).fontWeight() >= QFont::DemiBold);
        QVERIFY(!weightAt(2).fontItalic());
        {
            auto accepted = open(makeDocx(para(run("a ") + bold)));
            QCOMPARE(acceptAll(*accepted), 1);
            QCOMPARE(revisionCount(accepted->story()), 0);
            const QByteArray x = documentXml(accepted->toDocx());
            QVERIFY2(!x.contains("rPrChange") && x.contains("<w:b/>") && !x.contains("<w:i/>"), x.constData());
        }
        QCOMPARE(rejectAll(*doc), 1);
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(weightAt(2).fontWeight() < QFont::DemiBold);
        QVERIFY(weightAt(2).fontItalic());
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("rPrChange") && x.contains("<w:i/>") && !x.contains("<w:b/>"), x.constData());
    }

    void aChangeOfParagraphPropertiesIsRejectedByGivingTheOldOnesBack()
    {
        const QByteArray p = "<w:p><w:pPr><w:jc w:val=\"center\"/><w:pPrChange" + rev(1) + "><w:pPr><w:jc w:val=\"right\"/></w:pPr></w:pPrChange></w:pPr>" + run("x") + "</w:p>";
        {
            auto doc = open(makeDocx(p));
            QVERIFY(doc);
            QCOMPARE(revisionsIn(doc->story()).value(0).kind, Revision::ParagraphFormat);
            QCOMPARE(acceptAll(*doc), 1);
            QCOMPARE(doc->story()->begin().blockFormat().alignment(), Qt::AlignHCenter);
            QVERIFY(!documentXml(doc->toDocx()).contains("pPrChange"));
        }
        auto doc = open(makeDocx(p));
        QCOMPARE(rejectAll(*doc), 1);
        QCOMPARE(doc->story()->begin().blockFormat().alignment(), Qt::AlignRight);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("pPrChange") && x.contains("w:val=\"right\"") && !x.contains("w:val=\"center\""), x.constData());
    }

    void acceptAndRejectAreOneStepEachAndUndoBringsBackTheExactDocument()
    {
        const QByteArray body = para(run("one ") + inserted(1, "two ") + deleted(2, "three ") + run("four"), "<w:rPr><w:ins" + rev(5) + "/></w:rPr>") +
                                para(deleted(3, "x"), "<w:rPr><w:del" + rev(6) + "/></w:rPr>") + para(run("tail"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const QByteArray before = documentXml(doc->toDocx());
        const QString textBefore = textWithDeletions(*doc);
        QVERIFY(!doc->canUndo());
        QVERIFY(acceptAll(*doc) > 0);
        QVERIFY(doc->canUndo());
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(doc->undo());
        QVERIFY2(!doc->canUndo(), "Accept All was more than one step");
        QCOMPARE(textWithDeletions(*doc), textBefore);
        QCOMPARE(documentXml(doc->toDocx()), before);
        QVERIFY(doc->redo());
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(doc->undo());
        QVERIFY(rejectAll(*doc) > 0);
        QVERIFY(doc->undo());
        QVERIFY(!doc->canUndo());
        QCOMPARE(documentXml(doc->toDocx()), before);
    }

    void thisChangeNextAndPreviousStepThroughTheChanges()
    {
        auto doc = open(makeDocx(para(run("a ") + inserted(1, "b ") + run("c ") + deleted(2, "d ") + run("e ") + inserted(3, "f"))));
        QVERIFY(doc);
        QTextDocument *s = doc->story();
        const QVector<Revision> all = revisionsIn(s);
        QCOMPARE(all.size(), 3);
        QVERIFY(!previousRevision(s, all[0].from));
        QCOMPARE(nextRevision(s, all[0].from)->id, QStringLiteral("2"));
        QCOMPARE(nextRevision(s, all[1].from)->id, QStringLiteral("3"));
        QVERIFY(!nextRevision(s, all[2].from));
        QCOMPARE(previousRevision(s, all[2].from)->id, QStringLiteral("2"));
        QCOMPARE(revisionAt(s, all[0].from + 2)->id, QStringLiteral("1"));
        QVERIFY(!revisionAt(s, all[0].to + 1));
        // Accepting at the caret does that one change and goes on to the next.
        QTextCursor c(s);
        c.setPosition(all[0].from + 2);
        QCOMPARE(resolveAtCursor(*doc, c, true), 1);
        QCOMPARE(revisionCount(s), 2);
        QCOMPARE(c.position(), nextRevision(s, 0)->from);
        QCOMPARE(revisionAt(s, c.position())->kind, Revision::Deletion);
        QCOMPARE(resolveAtCursor(*doc, c, false), 1);   // (the deletion rejected: the words stay)
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("a b c d e f"));
        QCOMPARE(finalText(*doc), QStringLiteral("a b c d e f"));
        QCOMPARE(revisionCount(s), 1);
        // A selection takes every change it touches.
        QTextCursor all2(s);
        all2.select(QTextCursor::Document);
        QCOMPARE(resolveAtCursor(*doc, all2, false), 1);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("a b c d e "));
        QCOMPARE(revisionCount(s), 0);
    }

    // ---------------------------------------------------------------- Track Changes: making changes
    void typingWithTrackingWritesAnInsertionByTheReviewerWithTheTimeNow()
    {
        auto doc = open(makeDocx(para(run("hello"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed \"Z\" <&>"), QStringLiteral("Z"));
        doc->setTrackChanges(true);
        const qint64 before = QDateTime::currentSecsSinceEpoch();
        QTextCursor c(doc->story());
        c.setPosition(5);
        for (const QChar ch : QStringLiteral("abc")) insertText(*doc, c, QString(ch));
        QCOMPARE(finalText(*doc), QStringLiteral("helloabc"));
        const qint64 after = QDateTime::currentSecsSinceEpoch();
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(x.count("<w:ins "), 1);
        QCOMPARE(x.count("</w:ins>"), 1);
        const QRegularExpression tag(QStringLiteral("<w:ins w:id=\"(\\d+)\" w:author=\"([^\"]*)\" w:date=\"(\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\dZ)\">"));
        const QRegularExpressionMatch m = tag.match(QString::fromUtf8(x));
        QVERIFY2(m.hasMatch(), x.constData());
        QCOMPARE(m.captured(2), QStringLiteral("Zed &quot;Z&quot; &lt;&amp;&gt;"));
        QDateTime when = QDateTime::fromString(m.captured(3), Qt::ISODate);
        when.setTimeZone(QTimeZone::UTC);
        QVERIFY2(when.toSecsSinceEpoch() >= before && when.toSecsSinceEpoch() <= after, qPrintable(m.captured(3)));
        QVERIFY2(x.contains("</w:ins>") && x.indexOf("abc") > x.indexOf("<w:ins ") && x.indexOf("abc") < x.indexOf("</w:ins>"), x.constData());
        // The document says it tracks, and the one who opens it again finds the change with its author.
        QVERIFY2(settingsXml(doc->toDocx()).contains("<w:trackRevisions/>"), settingsXml(doc->toDocx()).constData());
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QVERIFY(again->trackChanges());
        QCOMPARE(revisionsIn(again->story()).value(0).author, QStringLiteral("Zed \"Z\" <&>"));
        QCOMPARE(revisionsIn(again->story()).value(0).kind, Revision::Insertion);
    }

    void eachTrackedEditIsOneStepOfTheHistoryAndUndoIsNeverCut()
    {
        auto doc = open(makeDocx(para(run("hello world"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        const QByteArray original = documentXml(doc->toDocx());
        QTextCursor c(doc->story());
        c.setPosition(5);
        insertText(*doc, c, QStringLiteral("X"));
        const QByteArray typed = documentXml(doc->toDocx());
        c.setPosition(7);
        c.setPosition(10, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        const QByteArray deleted = documentXml(doc->toDocx());
        QVERIFY(deleted != typed);
        QVERIFY(doc->canUndo());
        // The selection's deletion is one step.
        QVERIFY(doc->undo());
        QCOMPARE(documentXml(doc->toDocx()), typed);
        QVERIFY(doc->redo());
        QCOMPARE(documentXml(doc->toDocx()), deleted);
        while (doc->canUndo()) QVERIFY(doc->undo());
        QCOMPARE(documentXml(doc->toDocx()), original);
        QVERIFY(!doc->canUndo());
        QVERIFY(doc->canRedo());
        while (doc->canRedo()) QVERIFY(doc->redo());
        QCOMPARE(documentXml(doc->toDocx()), deleted);
    }

    void deletingWithTrackingMarksTheTextDeletedAndKeepsIt()
    {
        auto doc = open(makeDocx(para(run("hello"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(1);
        c.setPosition(4, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("hello"));
        QCOMPARE(finalText(*doc), QStringLiteral("ho"));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:delText xml:space=\"preserve\">ell</w:delText></w:r></w:del>") && x.count("<w:del ") == 1 && x.contains("w:author=\"Zed\""), x.constData());
        QVERIFY2(!x.contains("<w:t xml:space=\"preserve\">ell"), x.constData());
        const QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), 1);
        QCOMPARE(all[0].kind, Revision::Deletion);
        QCOMPARE(revisionText(doc->story(), all[0]), QStringLiteral("ell"));
        // The deletion can be rejected.
        QCOMPARE(rejectAll(*doc), 1);
        QCOMPARE(finalText(*doc), QStringLiteral("hello"));
        QVERIFY(!documentXml(doc->toDocx()).contains("<w:del"));
    }

    void backspaceAndDeleteStepOverWhatIsDeletedAndJoinTheirDeletions()
    {
        auto doc = open(makeDocx(para(run("abcdef"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(3);
        deleteBack(*doc, c, false);   // c
        deleteBack(*doc, c, false);   // b
        QCOMPARE(finalText(*doc), QStringLiteral("adef"));
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("abcdef"));
        QCOMPARE(documentXml(doc->toDocx()).count("<w:del "), 1);   // (one deletion of "bc")
        deleteForward(*doc, c, false);   // d
        QCOMPARE(finalText(*doc), QStringLiteral("aef"));
        QCOMPARE(documentXml(doc->toDocx()).count("<w:del "), 1);
        QCOMPARE(revisionText(doc->story(), revisionsIn(doc->story()).value(0)), QStringLiteral("bcd"));
        // A caret beside deleted text goes round it.
        insertText(*doc, c, QStringLiteral("Z"));
        QVERIFY2(finalText(*doc) == QStringLiteral("aZef") || finalText(*doc) == QStringLiteral("aeZf") || finalText(*doc) == QStringLiteral("aef") , qPrintable(finalText(*doc)));
    }

    void whatThePersonTypedWithTrackingIsTakenOutForGoodWhenDeleted()
    {
        auto doc = open(makeDocx(para(run("hello"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(5);
        insertText(*doc, c, QStringLiteral("abc"));
        deleteBack(*doc, c, false);
        QCOMPARE(finalText(*doc), QStringLiteral("helloab"));
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("helloab"));
        QVERIFY(!documentXml(doc->toDocx()).contains("<w:del "));
        deleteBack(*doc, c, false);
        deleteBack(*doc, c, false);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("hello"));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("<w:ins") && !x.contains("<w:del"), x.constData());
        QCOMPARE(revisionCount(doc->story()), 0);
        // Someone else's insertion is deleted by marking it so, inside it.
        auto other = open(makeDocx(para(run("a ") + inserted(1, "theirs", "Bob") + run(" z"))));
        other->setReviewer(QStringLiteral("Zed"), QString());
        other->setTrackChanges(true);
        QTextCursor d(other->story());
        d.setPosition(3);
        d.setPosition(9, QTextCursor::KeepAnchor);
        removeSelection(*other, d);
        QCOMPARE(finalText(*other), QStringLiteral("a  z"));
        QCOMPARE(textWithDeletions(*other), QStringLiteral("a theirs z"));
        const QByteArray y = documentXml(other->toDocx());
        QVERIFY2(y.indexOf("<w:ins ") < y.indexOf("<w:del ") && y.indexOf("<w:del ") < y.indexOf("</w:del>") && y.indexOf("</w:del>") < y.indexOf("</w:ins>"), y.constData());
    }

    void typingOverASelectionDeletesItAndInsertsAfterIt()
    {
        auto doc = open(makeDocx(para(run("hello"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(1);
        c.setPosition(4, QTextCursor::KeepAnchor);
        insertText(*doc, c, QStringLiteral("XY"));
        QCOMPARE(finalText(*doc), QStringLiteral("hXYo"));
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("hellXYo").left(1) + QStringLiteral("ell") + QStringLiteral("XYo"));
        const QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), 2);
        QCOMPARE(all[0].kind, Revision::Deletion);
        QCOMPARE(all[1].kind, Revision::Insertion);
        QVERIFY(doc->undo());
        QCOMPARE(finalText(*doc), QStringLiteral("hello"));
        QCOMPARE(revisionCount(doc->story()), 0);
    }

    void enterMakesAnInsertedParagraphMarkAndBackspaceTakesTheReviewersOwnBack()
    {
        auto doc = open(makeDocx(para(run("AAABBB"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(3);
        insertParagraph(*doc, c);
        QCOMPARE(doc->story()->blockCount(), 2);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:pPr><w:rPr><w:ins w:id=") && x.count("<w:p>") == 2, x.constData());
        QCOMPARE(revisionsIn(doc->story()).value(0).kind, Revision::MarkInserted);
        // Rejecting joins the halves again.
        {
            auto copy = open(doc->toDocx());
            QCOMPARE(rejectAll(*copy), 1);
            QCOMPARE(textWithDeletions(*copy), QStringLiteral("AAABBB"));
        }
        // Backspace at the start of the second takes the reviewer's mark away for good.
        deleteBack(*doc, c, false);
        QCOMPARE(doc->story()->blockCount(), 1);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("AAABBB"));
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(!documentXml(doc->toDocx()).contains("<w:ins"));
    }

    void backspaceAtTheStartOfAParagraphMarksTheMarkBeforeItDeletedAndAcceptJoinsThem()
    {
        auto doc = open(makeDocx(para(run("one")) + para(run("two"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(1).position());
        deleteBack(*doc, c, false);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:pPr><w:rPr><w:del w:id=") && x.count("<w:p>") == 2, x.constData());
        QCOMPARE(revisionsIn(doc->story()).value(0).kind, Revision::MarkDeleted);
        // The lines show them as one paragraph; accepting makes it so.
        QCOMPARE(acceptAll(*doc), 1);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("onetwo"));
    }

    void formattingWithTrackingRecordsTheOldLookAndRejectingBringsItBack()
    {
        auto doc = open(makeDocx(para(run("plain words"), "<w:jc w:val=\"left\"/>")));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        toggleBold(*doc, c);
        setAlignment(*doc, c, Qt::AlignHCenter);
        QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:rPrChange w:id=") && x.contains("<w:b/>") && x.contains("<w:pPrChange w:id=") && x.contains("w:val=\"center\""), x.constData());
        const QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), 2);
        QVERIFY(all[0].kind == Revision::ParagraphFormat || all[1].kind == Revision::ParagraphFormat);
        QVERIFY(all[0].kind == Revision::Format || all[1].kind == Revision::Format);
        // The file opens again with the changes, and rejecting them gives the old look back.
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(revisionCount(again->story()), 2);
        QCOMPARE(rejectAll(*again), 2);
        QTextCursor d(again->story());
        d.setPosition(3);
        QVERIFY(d.charFormat().fontWeight() < QFont::DemiBold);
        QCOMPARE(again->story()->begin().blockFormat().alignment(), Qt::AlignLeft);
        x = documentXml(again->toDocx());
        QVERIFY2(!x.contains("Change") && !x.contains("<w:b/>"), x.constData());
        // Text the reviewer inserted is formatted without a record.
        auto fresh = open(makeDocx(para(run("a"))));
        fresh->setReviewer(QStringLiteral("Zed"), QString());
        fresh->setTrackChanges(true);
        QTextCursor e(fresh->story());
        e.setPosition(1);
        insertText(*fresh, e, QStringLiteral("new"));
        e.setPosition(1);
        e.setPosition(4, QTextCursor::KeepAnchor);
        toggleItalic(*fresh, e);
        QVERIFY2(!documentXml(fresh->toDocx()).contains("rPrChange"), documentXml(fresh->toDocx()).constData());
    }

    void textPastedWithTrackingIsAnInsertionAndMovedTextIsADeletionAndAnInsertion()
    {
        auto doc = open(makeDocx(para(run("alpha beta gamma")) + para(run("second"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(0);
        pasteText(*doc, c, QStringLiteral("one\ntwo"));
        QCOMPARE(finalText(*doc), QStringLiteral("one\ntwoalpha beta gamma\nsecond"));
        QVector<Revision> all = revisionsIn(doc->story());
        int inserted = 0, marks = 0;
        for (const Revision &r : all) {
            inserted += r.kind == Revision::Insertion;
            marks += r.kind == Revision::MarkInserted;
        }
        QVERIFY2(inserted >= 1 && marks == 1, qPrintable(QStringLiteral("%1 insertions, %2 marks").arg(inserted).arg(marks)));
        // Rejecting everything gives the document back.
        QCOMPARE(rejectAll(*doc), int(all.size()));
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("alpha beta gamma\nsecond"));
        // Moving words by drag and drop: the old place is a deletion, and the new one an insertion.
        auto fresh = open(makeDocx(para(run("alpha beta gamma"))));
        fresh->setReviewer(QStringLiteral("Zed"), QString());
        fresh->setTrackChanges(true);
        QTextCursor s(fresh->story());
        s.setPosition(0);
        s.setPosition(6, QTextCursor::KeepAnchor);
        QVERIFY(moveSelectedText(*fresh, s, 16, false));
        QCOMPARE(finalText(*fresh), QStringLiteral("beta gammaalpha "));
        const QVector<Revision> moved = revisionsIn(fresh->story());
        QCOMPARE(moved.size(), 2);
        QCOMPARE(rejectAll(*fresh), 2);
        QCOMPARE(textWithDeletions(*fresh), QStringLiteral("alpha beta gamma"));
    }

    void turningTrackingOffAndOnIsASettingOfTheDocument()
    {
        auto doc = open(makeDocx(para(run("x")), "<w:trackRevisions/>"));
        QVERIFY(doc);
        QVERIFY(doc->trackChanges());
        QVERIFY2(settingsXml(doc->toDocx()).contains("<w:trackRevisions/>"), "kept as it was");
        doc->setTrackChanges(false);
        QVERIFY2(!settingsXml(doc->toDocx()).contains("trackRevisions"), settingsXml(doc->toDocx()).constData());
        QTextCursor c(doc->story());
        c.setPosition(1);
        insertText(*doc, c, QStringLiteral("y"));
        QCOMPARE(revisionCount(doc->story()), 0);   // (typed without tracking: no change)
        auto none = open(makeDocx(para(run("x"))));
        QVERIFY(!none->trackChanges());
        none->setTrackChanges(true);
        QVERIFY2(settingsXml(none->toDocx()).contains("<w:trackRevisions/>"), "a document with no settings gets them");
    }

    void everyTrackedEditSavesAsValidMarks()
    {
        auto doc = open(makeDocx(para(run("one two three")) + para(run("four five"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(4);
        c.setPosition(7, QTextCursor::KeepAnchor);
        insertText(*doc, c, QStringLiteral("2"));
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 2);
        insertParagraph(*doc, c);
        insertText(*doc, c, QStringLiteral("zz"));
        c.setPosition(0);
        c.setPosition(doc->story()->findBlockByNumber(0).position() + 2, QTextCursor::KeepAnchor);
        toggleBold(*doc, c);
        const QByteArray x = documentXml(doc->toDocx());
        // Every deletion holds w:delText and no w:t; every insertion holds w:t and no w:delText; ids are unique; the file opens again with the same changes.
        const QString s = QString::fromUtf8(x);
        static const QRegularExpression del(QStringLiteral("<w:del [^>]*>(.*?)</w:del>")), ins(QStringLiteral("<w:ins [^>]*>(.*?)</w:ins>")), ids(QStringLiteral("<w:(?:ins|del|rPrChange|pPrChange) w:id=\"(\\d+)\""));
        for (auto it = del.globalMatch(s); it.hasNext();) {
            const QString inner = it.next().captured(1);
            QVERIFY2(!inner.contains(QLatin1String("<w:t ")) && !inner.contains(QLatin1String("<w:t>")) && inner.contains(QLatin1String("delText")), qPrintable(inner));
        }
        for (auto it = ins.globalMatch(s); it.hasNext();) QVERIFY2(!it.next().captured(1).contains(QLatin1String("delText")), qPrintable(s));
        QSet<QString> seen;
        for (auto it = ids.globalMatch(s); it.hasNext();) {
            const QString id = it.next().captured(1);
            QVERIFY2(!seen.contains(id), qPrintable(QStringLiteral("id %1 twice: %2").arg(id, s)));
            seen.insert(id);
        }
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(revisionCount(again->story()), revisionCount(doc->story()));
        QCOMPARE(textWithDeletions(*again), textWithDeletions(*doc));
        QCOMPARE(documentXml(again->toDocx()), x);
    }

    // ---------------------------------------------------------------- Display for Review
    void changesAreShownAsTheViewSaysAndTheViewNeverChangesTheFile()
    {
        auto doc = open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob") + run("four"))));
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        // The document as it stands (No Markup, Simple Markup): the deleted words take no room.
        QCOMPARE(doc->revisionView(), 0);
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two four")});
        // All Markup: the deletion takes its place.
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two three four")});
        // Original: the insertion is left out and the deletion is there, unmarked.
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionHideInserted);
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one three four")});
        // Back: nothing of the document changed, not the file, not the history.
        doc->setRevisionView(0);
        QCOMPARE(documentXml(doc->toDocx()), documentXml(saved));
        QVERIFY(!doc->canUndo());
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two four")});
        QCOMPARE(revisionCount(doc->story()), 2);
    }

    void markedChangesAreDrawnInTheirAuthorsColorsUnderlinedOrStruckThrough()
    {
        auto doc = open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob"))));
        QVERIFY(doc);
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        QTextBlock b = doc->story()->begin();
        QTextCharFormat plain, ins, del;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.hasProperty(prop::Atom)) continue;
            const QString t = it.fragment().text();
            if (t == QLatin1String("one ")) plain = cf;
            else if (t == QLatin1String("two ")) ins = cf;
            else if (t == QLatin1String("three ")) del = cf;
        }
        const LayoutEnv &env = doc->flow().env;
        const QTextCharFormat p = resolveCharFormat(plain, env), i = resolveCharFormat(ins, env), d = resolveCharFormat(del, env);
        QVERIFY(!p.fontStrikeOut() && p.underlineStyle() == QTextCharFormat::NoUnderline);
        QVERIFY(i.underlineStyle() != QTextCharFormat::NoUnderline && !i.fontStrikeOut());
        QVERIFY(d.fontStrikeOut());
        // (Each person has a color of their own, the same one every time.)
        QVERIFY(i.foreground().color() != d.foreground().color());
        QCOMPARE(i.foreground().color(), tp::revisionColor(doc->reviewerIndex(QStringLiteral("Ann"))));
        QCOMPARE(d.foreground().color(), tp::revisionColor(doc->reviewerIndex(QStringLiteral("Bob"))));
        // Not marked: as the text is.
        doc->setRevisionView(tp::kRevisionShowDeleted);
        const QTextCharFormat unmarked = resolveCharFormat(del, doc->flow().env);
        QVERIFY(!unmarked.fontStrikeOut());
    }

    void eachAuthorHasAColorOfTheirOwnThatTheyKeepUnlessOneColorIsChosen()
    {
        MarkupGuard guard;
        auto doc = open(makeDocx(para(inserted(1, "aa", "Ann") + run(" ") + inserted(2, "bb", "Bob") + run(" ") + inserted(3, "cc", "Ann"))));
        QVERIFY(doc);
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        QCOMPARE(shownFormat(*doc, QStringLiteral("aa")).foreground().color(), shownFormat(*doc, QStringLiteral("cc")).foreground().color());
        QVERIFY(shownFormat(*doc, QStringLiteral("aa")).foreground().color() != shownFormat(*doc, QStringLiteral("bb")).foreground().color());
        // (The palette has eight colors, none twice; the person who comes first has the first.)
        QSet<QRgb> palette;
        for (int i = 0; i < 8; ++i) palette.insert(tp::revisionMarkColor(1, i).rgb());
        QCOMPARE(palette.size(), 8);
        QCOMPARE(tp::revisionMarkColor(1, doc->reviewerIndex(QStringLiteral("Ann"))), tp::revisionColor(0));
        // One color for everyone, chosen.
        tp::revisionMarkup().insertionColor = QColor(0, 0, 255);
        QCOMPARE(shownFormat(*doc, QStringLiteral("aa")).foreground().color(), QColor(0, 0, 255));
        QCOMPARE(shownFormat(*doc, QStringLiteral("bb")).foreground().color(), QColor(0, 0, 255));
        QCOMPARE(tp::revisionMarkColor(1, 5), QColor(0, 0, 255));
    }

    void theChosenLooksOfInsertionsAndDeletionsAreApplied()
    {
        MarkupGuard guard;
        auto doc = open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob"))));
        QVERIFY(doc);
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        tp::RevisionMarkup &m = tp::revisionMarkup();
        const QColor ann = tp::revisionColor(doc->reviewerIndex(QStringLiteral("Ann"))), bob = tp::revisionColor(doc->reviewerIndex(QStringLiteral("Bob")));
        auto ins = [&](int style) {
            m.insertion = style;
            return shownFormat(*doc, QStringLiteral("two "));
        };
        auto del = [&](int style) {
            m.deletion = style;
            return shownFormat(*doc, QStringLiteral("three "));
        };
        QTextCharFormat f = ins(tp::RevisionMarkup::InsUnderline);
        QVERIFY(f.underlineStyle() != QTextCharFormat::NoUnderline && !(f.intProperty(tp::UnderlineKind) & 1));
        f = ins(tp::RevisionMarkup::InsDoubleUnderline);
        QVERIFY(f.underlineStyle() != QTextCharFormat::NoUnderline && (f.intProperty(tp::UnderlineKind) & 1));
        f = ins(tp::RevisionMarkup::InsStrike);
        QVERIFY(f.fontStrikeOut() && f.underlineStyle() == QTextCharFormat::NoUnderline);
        f = ins(tp::RevisionMarkup::InsBold);
        QVERIFY(f.fontWeight() >= QFont::Bold && f.underlineStyle() == QTextCharFormat::NoUnderline);
        f = ins(tp::RevisionMarkup::InsItalic);
        QVERIFY(f.fontItalic() && f.underlineStyle() == QTextCharFormat::NoUnderline);
        f = ins(tp::RevisionMarkup::InsColorOnly);
        QVERIFY(f.underlineStyle() == QTextCharFormat::NoUnderline && !f.fontItalic() && !f.fontStrikeOut());
        QCOMPARE(f.foreground().color(), ann);
        f = ins(tp::RevisionMarkup::InsNone);
        QVERIFY(f.underlineStyle() == QTextCharFormat::NoUnderline && !f.fontStrikeOut() && f.foreground().color() != ann);
        f = del(tp::RevisionMarkup::DelStrike);
        QVERIFY(f.fontStrikeOut() && !f.boolProperty(tp::DoubleStrike));
        f = del(tp::RevisionMarkup::DelDoubleStrike);
        QVERIFY(f.fontStrikeOut() && f.boolProperty(tp::DoubleStrike));
        f = del(tp::RevisionMarkup::DelColorOnly);
        QVERIFY(!f.fontStrikeOut());
        QCOMPARE(f.foreground().color(), bob);
        // The colors of each kind: the author's, or the one chosen.
        m.deletion = tp::RevisionMarkup::DelStrike;
        m.insertion = tp::RevisionMarkup::InsUnderline;
        m.deletionColor = QColor(0, 0, 255);
        QCOMPARE(shownFormat(*doc, QStringLiteral("three ")).foreground().color(), QColor(0, 0, 255));
        QCOMPARE(shownFormat(*doc, QStringLiteral("two ")).foreground().color(), ann);
    }

    void aChangeBarStandsBesideEachLineThatHoldsAChangeAndNoOther()
    {
        MarkupGuard guard;
        tp::revisionMarkup().barColor = QColor(0, 0, 255);
        auto bars = [&](const QByteArray &body, int placing) {
            tp::revisionMarkup().bars = placing;
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.resize(900, 700);
            w.show();
            w.setDocument(open(makeDocx(body)), QString());
            w.act(QStringLiteral("review.view.simple"))->trigger();
            return blueBars(w.view()->pageImage(0, 600));
        };
        const QByteArray one = para(run("first line")) + para(inserted(1, "second line")) + para(run("third line"));
        const QByteArray all = para(inserted(1, "first line")) + para(inserted(2, "second line")) + para(inserted(3, "third line"));
        const BarRows mid = bars(one, tp::RevisionMarkup::BarsLeft), every = bars(all, tp::RevisionMarkup::BarsLeft);
        QVERIFY(mid.rows > 0);
        QVERIFY2(every.rows >= mid.rows * 5 / 2, qPrintable(QStringLiteral("%1 rows for one changed line, %2 for three").arg(mid.rows).arg(every.rows)));
        QVERIFY(mid.meanX < 300);
        QVERIFY(bars(one, tp::RevisionMarkup::BarsRight).meanX > 300);
        QVERIFY(bars(one, tp::RevisionMarkup::BarsOutside).meanX > 300);   // (page one is a right-hand page: its outside is at the right)
        QCOMPARE(bars(one, tp::RevisionMarkup::BarsNone).rows, 0);
    }

    void aMovedPairIsReadDrawnAsAMoveSavedAsAMoveAndAcceptedAsOne()
    {
        MarkupGuard guard;
        const QByteArray body = para(run("keep ") + moveFromOf("move9", 1, "moved")) + para(moveToOf("move9", 3, "moved") + run(" tail"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QVector<Revision::Kind> kinds;
        for (const Revision &r : revisionsIn(doc->story())) kinds << r.kind;
        QCOMPARE(kinds, (QVector<Revision::Kind>{Revision::MovedFrom, Revision::MovedTo}));
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        const QColor green(0x1A, 0x7F, 0x37);
        QTextCharFormat from, to;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().text() == QLatin1String("moved")) (it.fragment().charFormat().intProperty(tp::Revision) == 3 ? from : to) = resolveCharFormat(it.fragment().charFormat(), doc->flow().env);
        QVERIFY(from.fontStrikeOut() && from.boolProperty(tp::DoubleStrike));
        QCOMPARE(from.foreground().color(), green);
        QVERIFY(to.underlineStyle() != QTextCharFormat::NoUnderline && (to.intProperty(tp::UnderlineKind) & 1));
        QCOMPARE(to.foreground().color(), green);
        // Saved as a move, with its ranges and the name that pairs them.
        const QByteArray saved = documentXml(doc->toDocx());
        QVERIFY2(saved.contains("<w:moveFrom ") && saved.contains("<w:moveTo ") && saved.contains("<w:moveFromRangeStart") && saved.contains("<w:moveToRangeEnd") && saved.contains("w:name=\"move9\""), saved.constData());
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(revisionCount(again->story()), 2);
        // Accepting one half takes both.
        QVector<Revision> which;
        for (const Revision &r : revisionsIn(doc->story()))
            if (r.kind == Revision::MovedTo) which << r;
        QCOMPARE(resolveRevisions(*doc, doc->story(), which, true), 2);
        QCOMPARE(revisionCount(doc->story()), 0);
        // Track moves off: a deletion and an insertion, in the looks and colors of those.
        tp::revisionMarkup().trackMoves = false;
        const QTextCharFormat d = shownFormat(*again, QStringLiteral("moved"));
        again->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        for (QTextBlock b = again->story()->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().text() == QLatin1String("moved")) (it.fragment().charFormat().intProperty(tp::Revision) == 3 ? from : to) = resolveCharFormat(it.fragment().charFormat(), again->flow().env);
        Q_UNUSED(d);
        QVERIFY(from.fontStrikeOut() && !from.boolProperty(tp::DoubleStrike));
        QVERIFY(to.underlineStyle() != QTextCharFormat::NoUnderline && !(to.intProperty(tp::UnderlineKind) & 1));
        QVERIFY(from.foreground().color() != green && to.foreground().color() != green);
        QCOMPARE(from.foreground().color(), tp::revisionColor(again->reviewerIndex(QStringLiteral("Ann"))));
    }

    void aChangeOfFormattingIsMarkedAsChosen()
    {
        MarkupGuard guard;
        auto doc = open(makeDocx(para(run("plain ") + "<w:r><w:rPr><w:i/><w:rPrChange" + rev(1) + "><w:rPr/></w:rPrChange></w:rPr><w:t>styled</w:t></w:r>")));
        QVERIFY(doc);
        QCOMPARE(revisionCount(doc->story()), 1);
        tp::RevisionMarkup &m = tp::revisionMarkup();
        const QColor ann = tp::revisionColor(doc->reviewerIndex(QStringLiteral("Ann")));
        auto look = [&](int view) {
            doc->setRevisionView(view);
            return shownFormat(*doc, QStringLiteral("styled"));
        };
        const int marked = tp::kRevisionShowDeleted | tp::kRevisionMark | tp::kRevisionMarkFormat;
        QVERIFY(look(marked).fontWeight() >= QFont::Bold);
        QVERIFY(look(tp::kRevisionShowDeleted | tp::kRevisionMark).fontWeight() < QFont::DemiBold);   // (formatting not shown)
        QVERIFY(shownFormat(*doc, QStringLiteral("plain ")).fontWeight() < QFont::DemiBold);
        m.formatting = tp::RevisionMarkup::FmtColorOnly;
        QVERIFY(look(marked).fontWeight() < QFont::DemiBold);
        QCOMPARE(look(marked).foreground().color(), ann);
        m.formatting = tp::RevisionMarkup::FmtNone;
        QVERIFY(look(marked).fontWeight() < QFont::DemiBold && look(marked).foreground().color() != ann);
        m.formatting = tp::RevisionMarkup::FmtBold;
        m.trackFormatting = false;
        QVERIFY(look(marked).fontWeight() < QFont::DemiBold);
    }

    void theChoicesAreKeptAndEveryOpenWindowRedrawsWhenTheyChange()
    {
        MarkupGuard guard;
        const QByteArray body = para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob") + run("four"));
        WriteWindow a, b;
        for (WriteWindow *w : {&a, &b}) {
            w->setNotifier([](const QString &, const QString &) {});
            w->show();
            w->setDocument(open(makeDocx(body)), QString());
        }
        QCOMPARE(lineTexts(*a.document()), QStringList{QStringLiteral("one two three four")});
        tp::RevisionMarkup m = tp::revisionMarkup();
        m.deletion = tp::RevisionMarkup::DelHidden;
        WriteWindow::applyRevisionMarkup(m);
        // (Hidden and caret deletions leave the words out of the lines; the other looks keep them.)
        QCOMPARE(lineTexts(*a.document()), QStringList{QStringLiteral("one two four")});
        QCOMPARE(lineTexts(*b.document()), QStringList{QStringLiteral("one two four")});
        m.deletion = tp::RevisionMarkup::DelCaret;
        WriteWindow::applyRevisionMarkup(m);
        QCOMPARE(lineTexts(*b.document()), QStringList{QStringLiteral("one two four")});
        m.deletion = tp::RevisionMarkup::DelDoubleStrike;
        m.trackFormatting = false;
        WriteWindow::applyRevisionMarkup(m);
        QCOMPARE(lineTexts(*b.document()), QStringList{QStringLiteral("one two three four")});
        QVERIFY(!(a.document()->revisionView() & tp::kRevisionMarkFormat));
        // Kept in the settings, and read back.
        tp::revisionMarkup() = tp::RevisionMarkup();
        WriteWindow::loadRevisionMarkup();
        QCOMPARE(tp::revisionMarkup().deletion, int(tp::RevisionMarkup::DelDoubleStrike));
        QVERIFY(!tp::revisionMarkup().trackFormatting);
    }

    void theTrackingOptionsDialogChoosesTheLooksAndTheyTakeEffect()
    {
        MarkupGuard guard;
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(open(makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob") + run("four")))), QString());
        withDialog([&] { w.act(QStringLiteral("review.trackOptions"))->trigger(); }, QStringLiteral("trackOptionsDialog"), [](QDialog *d) {
            auto *deletion = d->findChild<QComboBox *>(QStringLiteral("deletionLook"));
            QVERIFY(deletion && d->findChild<QComboBox *>(QStringLiteral("insertionLook")) && d->findChild<QComboBox *>(QStringLiteral("barsLook")));
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("trackMoves")) && d->findChild<QCheckBox *>(QStringLiteral("trackFormatting")));
            deletion->setCurrentIndex(deletion->findData(int(tp::RevisionMarkup::DelHidden)));
        });
        QCOMPARE(tp::revisionMarkup().deletion, int(tp::RevisionMarkup::DelHidden));
        QCOMPARE(lineTexts(*w.document()), QStringList{QStringLiteral("one two four")});
    }

    void aParagraphWhollyDeletedIsAParagraphOfItsOwnWhereDeletionsAreShown()
    {
        const QByteArray body = para(run("keep")) + para(deleted(1, "gone"), "<w:rPr><w:del" + rev(2) + "/></w:rPr>") + para(run("tail"));
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QCOMPARE(lineTexts(*doc), (QStringList{QStringLiteral("keep"), QStringLiteral("tail")}));
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        QCOMPARE(lineTexts(*doc), (QStringList{QStringLiteral("keep"), QStringLiteral("gone"), QStringLiteral("tail")}));
        QCOMPARE(doc->pager().lines(0).size(), 3);
        doc->setRevisionView(0);
        QCOMPARE(doc->pager().lines(0).size(), 2);
    }

    void textTypedByTheReviewerShowsInThatColorAndTextTypedBesideChangesDoesNotInheritThem()
    {
        auto doc = open(makeDocx(para(run("a ") + inserted(1, "theirs", "Bob") + run(" z"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setRevisionView(tp::kRevisionShowDeleted | tp::kRevisionMark);
        // Typed inside someone's insertion with tracking off: part of it, in its color.
        QTextCursor c(doc->story());
        c.setPosition(5);
        insertText(*doc, c, QStringLiteral("X"));
        QTextCursor look(doc->story());
        look.setPosition(6);
        QCOMPARE(look.charFormat().intProperty(tp::Revision), 1);
        QCOMPARE(look.charFormat().intProperty(tp::RevisionAuthor), doc->reviewerIndex(QStringLiteral("Bob")));
        // Typed after it: no mark.
        c.setPosition(doc->story()->characterCount() - 1);
        insertText(*doc, c, QStringLiteral("Y"));
        QTextCursor end(doc->story());
        end.setPosition(doc->story()->characterCount() - 1);
        QCOMPARE(end.charFormat().intProperty(tp::Revision), 0);
        // Typed with tracking on: in the reviewer's color.
        doc->setTrackChanges(true);
        c.setPosition(1);
        insertText(*doc, c, QStringLiteral("Q"));
        QTextCursor mine(doc->story());
        mine.setPosition(c.position());
        QCOMPARE(mine.charFormat().intProperty(tp::Revision), 1);
        QCOMPARE(mine.charFormat().intProperty(tp::RevisionAuthor), doc->reviewerIndex(QStringLiteral("Zed")));
    }

    // ---------------------------------------------------------------- comments
    void aFilesCommentsAreListedWithTheirRepliesAndResolvedState()
    {
        const QByteArray cmXml = commentElement(1, "Ann", "First", "0000000A") + commentElement(2, "Bob", "A reply", "0000000B", "2020-02-03T10:00:00Z") + commentElement(3, "Ann", "Third", "0000000C");
        const QByteArray ext = "<w15:commentEx w15:paraId=\"0000000A\" w15:done=\"1\"/><w15:commentEx w15:paraId=\"0000000B\" w15:paraIdParent=\"0000000A\" w15:done=\"1\"/><w15:commentEx w15:paraId=\"0000000C\" w15:done=\"0\"/>";
        auto doc = open(makeDocxWithComments(para(run("see ") + commented(1, "these words") + commented(2, "these words") + run(" and ") + commented(3, "that")), cmXml, ext));
        QVERIFY(doc);
        const QVector<Comment> all = comments(*doc);
        QCOMPARE(all.size(), 3);
        QCOMPARE(all[0].id, 1);
        QCOMPARE(all[0].author, QStringLiteral("Ann"));
        QCOMPARE(all[0].initials, QStringLiteral("AB"));
        QCOMPARE(all[0].text, QStringLiteral("First"));
        QCOMPARE(all[0].date, QDateTime(QDate(2020, 2, 2), QTime(10, 0), QTimeZone::UTC));
        QVERIFY(all[0].done);
        QCOMPARE(all[0].parent, -1);
        QCOMPARE(all[1].parent, 1);
        QVERIFY(all[1].done);
        QCOMPARE(all[2].parent, -1);
        QVERIFY(!all[2].done);
        QVERIFY(all[0].to > all[0].from);
        QCOMPARE(doc->story()->toPlainText().mid(all[0].from + 1, 1), QStringLiteral("t"));
        QCOMPARE(repliesTo(*doc, 1).size(), 1);
        QCOMPARE(commentAt(*doc, all[2].from + 2)->id, 3);
        QCOMPARE(nextComment(*doc, all[0].from)->id, 3);   // (the reply is not another comment to step to)
        QCOMPARE(previousComment(*doc, all[2].from)->id, 1);
        // Saved without a change, the parts are the file's own bytes.
        const QByteArray saved = doc->toDocx();
        QCOMPARE(partOf(saved, "/word/comments.xml"), partOf(makeDocxWithComments(QByteArray(), cmXml, ext), "/word/comments.xml"));
        QCOMPARE(partOf(saved, "/word/commentsExtended.xml"), partOf(makeDocxWithComments(QByteArray(), cmXml, ext), "/word/commentsExtended.xml"));
    }

    void aCommentIsAddedToTheSelectionAndUndoTakesItBack()
    {
        auto doc = open(makeDocx(para(run("some words here"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed Ypsilon"), QString());
        const QByteArray original = documentXml(doc->toDocx());
        QTextCursor c(doc->story());
        c.setPosition(5);
        c.setPosition(10, QTextCursor::KeepAnchor);
        const int id = addComment(*doc, c, QStringLiteral("Why \"these\"?\nSecond line & more"));
        QVERIFY(id > 0);
        const QVector<Comment> all = comments(*doc);
        QCOMPARE(all.size(), 1);
        QCOMPARE(all[0].id, id);
        QCOMPARE(all[0].author, QStringLiteral("Zed Ypsilon"));
        QCOMPARE(all[0].initials, QStringLiteral("ZY"));
        QCOMPARE(all[0].text, QStringLiteral("Why \"these\"?\nSecond line & more"));
        QVERIFY(all[0].to > all[0].from);
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        QVERIFY2(x.indexOf("commentRangeStart") < x.indexOf("words") && x.indexOf("words") < x.indexOf("commentRangeEnd") && x.indexOf("commentRangeEnd") < x.indexOf("commentReference"), x.constData());
        const QByteArray commentsPart = partOf(saved, "/word/comments.xml");
        QVERIFY2(commentsPart.contains("w:author=\"Zed Ypsilon\"") && commentsPart.contains("w:initials=\"ZY\"") && commentsPart.contains("Why \"these\"?") && commentsPart.contains("Second line &amp; more") &&
                     commentsPart.contains("<w:annotationRef/>") && commentsPart.contains("w14:paraId="),
                 commentsPart.constData());
        // It opens again as the same comment.
        auto again = open(saved);
        QVERIFY(again);
        QCOMPARE(comments_text(*again), QStringLiteral("Why \"these\"?\nSecond line & more"));
        // Undo takes the marks and the comment out in one step; nothing of it is in the file then.
        QVERIFY(doc->undo());
        QVERIFY(comments(*doc).isEmpty());
        QVERIFY(!doc->canUndo());
        QCOMPARE(documentXml(doc->toDocx()), original);
        QVERIFY2(!partOf(doc->toDocx(), "/word/comments.xml").contains("w:comment "), "an undone comment is not written");
        QVERIFY(doc->redo());
        QCOMPARE(comments(*doc).size(), 1);
    }

    void aReplyIsInTheRangeOfItsCommentAndResolvingAndReopeningChangeTheThread()
    {
        const QByteArray cm = commentElement(1, "Ann", "Question", "0000000A");
        auto doc = open(makeDocxWithComments(para(run("a ") + commented(1, "range") + run(" b")), cm, "<w15:commentEx w15:paraId=\"0000000A\" w15:done=\"0\"/>",
                                             "<w16cid:commentId w16cid:paraId=\"0000000A\" w16cid:durableId=\"11111111\"/>",
                                             "<w15:person w15:author=\"Ann\"><w15:presenceInfo w15:providerId=\"None\" w15:userId=\"Ann\"/></w15:person>"));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Bob"), QStringLiteral("B"));
        const int reply = replyToComment(*doc, 1, QStringLiteral("Answer"));
        QVERIFY(reply > 0);
        QVector<Comment> all = comments(*doc);
        QCOMPARE(all.size(), 2);
        QCOMPARE(all[1].id, reply);
        QCOMPARE(all[1].parent, 1);
        QCOMPARE(all[1].from, all[0].from + 1);
        QVERIFY(setCommentDone(*doc, 1, true));
        all = comments(*doc);
        QVERIFY(all[0].done && all[1].done);
        QByteArray saved = doc->toDocx();
        QByteArray ext = partOf(saved, "/word/commentsExtended.xml");
        QVERIFY2(ext.count("w15:done=\"1\"") == 2 && ext.contains("w15:paraIdParent=\"0000000A\"") && ext.contains("w15:paraId=\"0000000A\""), ext.constData());
        const QByteArray commentsPart = partOf(saved, "/word/comments.xml");
        QVERIFY2(commentsPart.contains("Answer") && commentsPart.contains("Question") && commentsPart.contains("w:author=\"Bob\"") && commentsPart.indexOf("Question") < commentsPart.indexOf("Answer"), commentsPart.constData());
        QVERIFY2(commentsPart.contains(commentElement(1, "Ann", "Question", "0000000A")), "the file's own comment stays as it was written");
        const QByteArray ids = partOf(saved, "/word/commentsIds.xml");
        QVERIFY2(ids.contains("w16cid:durableId=\"11111111\"") && ids.count("w16cid:commentId ") == 2, ids.constData());
        QVERIFY2(partOf(saved, "/word/people.xml").contains("w15:author=\"Bob\"") && partOf(saved, "/word/people.xml").contains("w15:author=\"Ann\""), partOf(saved, "/word/people.xml").constData());
        // Reopening it, and the whole thing coming back from the file.
        QVERIFY(setCommentDone(*doc, 1, false));
        QVERIFY(!comments_done(*doc, 1));
        auto again = open(saved);
        QVERIFY(again);
        const QVector<Comment> back = comments(*again);
        QCOMPARE(back.size(), 2);
        QCOMPARE(back[1].parent, 1);
        QVERIFY(back[0].done && back[1].done);
        QVERIFY(setCommentDone(*again, 1, false));
        ext = partOf(again->toDocx(), "/word/commentsExtended.xml");
        QVERIFY2(ext.count("w15:done=\"0\"") == 2 && ext.count("w15:done=\"1\"") == 0, ext.constData());
    }

    void aCommentAndItsRepliesAreDeletedTogetherAndItsPartsGoWithIt()
    {
        const QByteArray cms = commentElement(1, "Ann", "One", "0000000A") + commentElement(2, "Bob", "Reply", "0000000B") + commentElement(3, "Cy", "Other", "0000000C");
        const QByteArray ext = "<w15:commentEx w15:paraId=\"0000000A\" w15:done=\"0\"/><w15:commentEx w15:paraId=\"0000000B\" w15:paraIdParent=\"0000000A\" w15:done=\"0\"/><w15:commentEx w15:paraId=\"0000000C\" w15:done=\"0\"/>";
        auto doc = open(makeDocxWithComments(para(commented(1, "x") + commented(2, "x") + commented(3, "y")), cms, ext));
        QVERIFY(doc);
        QCOMPARE(deleteComment(*doc, 1), 2);
        QCOMPARE(comments(*doc).size(), 1);
        QCOMPARE(comments(*doc)[0].id, 3);
        const QByteArray saved = doc->toDocx();
        QVERIFY2(!documentXml(saved).contains("w:id=\"1\"") && !documentXml(saved).contains("w:id=\"2\""), documentXml(saved).constData());
        const QByteArray parts = partOf(saved, "/word/comments.xml");
        QVERIFY2(!parts.contains("One") && !parts.contains("Reply") && parts.contains("Other"), parts.constData());
        const QByteArray e = partOf(saved, "/word/commentsExtended.xml");
        QVERIFY2(e.count("w15:commentEx ") == 1 && e.contains("0000000C"), e.constData());
        // One step.
        QVERIFY(doc->undo());
        QCOMPARE(comments(*doc).size(), 3);
        QVERIFY(!doc->canUndo());
        QCOMPARE(deleteAllComments(*doc), 3);
        QVERIFY(comments(*doc).isEmpty());
        QVERIFY(!documentXml(doc->toDocx()).contains("comment"));
    }

    void commentsWithMissingRangesOrLoopsOfRepliesAreKeptAndBounded()
    {
        // A reference with no range, a range with no reference, a reference to a comment the file lacks, and two comments that reply to each other.
        const QByteArray cms = commentElement(1, "Ann", "Lonely", "0000000A") + commentElement(2, "Ann", "Loop one", "0000000B") + commentElement(3, "Ann", "Loop two", "0000000C");
        const QByteArray ext = "<w15:commentEx w15:paraId=\"0000000B\" w15:paraIdParent=\"0000000C\" w15:done=\"0\"/><w15:commentEx w15:paraId=\"0000000C\" w15:paraIdParent=\"0000000B\" w15:done=\"0\"/>"
                               "<w15:commentEx w15:paraId=\"0000000A\" w15:paraIdParent=\"0000000A\" w15:done=\"0\"/>";
        const QByteArray body = para("<w:r><w:commentReference w:id=\"1\"/></w:r><w:commentRangeStart w:id=\"9\"/>" + run("x") + "<w:r><w:commentReference w:id=\"77\"/></w:r>" + commented(2, "a") + commented(3, "b"));
        auto doc = open(makeDocxWithComments(body, cms, ext));
        QVERIFY(doc);
        const QVector<Comment> all = comments(*doc);
        QCOMPARE(all.size(), 3);   // (the reference to a comment the file lacks is none)
        for (const Comment &c : all) QCOMPARE(c.parent, c.id == 3 ? 2 : -1);   // (a loop is broken at one of its two)
        QVERIFY(all[0].from >= 0 && all[0].to == all[0].from);
        // Saving keeps all three, and deleting them works.
        QCOMPARE(partOf(doc->toDocx(), "/word/comments.xml").count("<w:comment "), 3);
        QCOMPARE(deleteAllComments(*doc), 3);
    }

    // ---------------------------------------------------------------- the window
    void theReviewTabHasItsCommandsAndItsShortcutsAndBuildsCleanly()
    {
        WriteWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"review.trackChanges", "review.accept", "review.reject", "review.acceptAll", "review.rejectAll", "review.acceptShown", "review.rejectShown", "review.acceptAllStop",
                               "review.nextChange", "review.previousChange", "review.view.simple", "review.view.all", "review.view.none", "review.view.original", "review.show.insdel",
                               "review.show.formatting", "review.show.comments", "review.balloons.inline", "review.balloons.balloons", "review.balloons.comments", "review.reviewingPane",
                               "review.newComment", "review.deleteComment", "review.deleteAllComments", "review.nextComment", "review.previousComment", "review.replyComment", "review.resolveComment"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QCOMPARE(w.act(QStringLiteral("review.trackChanges"))->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E).toString(QKeySequence::NativeText));
        QCOMPARE(w.act(QStringLiteral("review.newComment"))->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_M).toString(QKeySequence::NativeText));
        QVERIFY(w.act(QStringLiteral("review.trackChanges"))->isCheckable());
        QVERIFY(w.act(QStringLiteral("review.view.all"))->isChecked());
    }

    void trackChangesTurnsOnAndOffAndTypingInTheWindowIsRecorded()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        write::WriteDoc *doc = w.document();
        QVERIFY(doc);
        QVERIFY(!doc->trackChanges());
        w.act(QStringLiteral("review.trackChanges"))->trigger();
        QVERIFY(doc->trackChanges());
        QVERIFY(w.act(QStringLiteral("review.trackChanges"))->isChecked());
        w.view()->insertText(QStringLiteral("hello"));
        QCOMPARE(revisionCount(doc->story()), 1);
        // (a computer whose account has no full name records the change as by "Author")
        const QString me = Settings::get().userName().trimmed();
        QCOMPARE(revisionsIn(doc->story()).value(0).author, me.isEmpty() ? QStringLiteral("Author") : me);
        QTRY_COMPARE(w.findChild<QToolButton *>(QStringLiteral("trackStatus"))->text(), QStringLiteral("Track Changes: On"));
        w.act(QStringLiteral("review.trackChanges"))->trigger();
        QVERIFY(!doc->trackChanges());
        w.view()->insertText(QStringLiteral("!"));
        QCOMPARE(revisionCount(doc->story()), 1);
        QTRY_COMPARE(w.findChild<QToolButton *>(QStringLiteral("trackStatus"))->text(), QStringLiteral("Track Changes: Off"));
        // Accept All through the command: one step.
        w.act(QStringLiteral("review.acceptAll"))->trigger();
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(w.act(QStringLiteral("review.accept")) != nullptr);
    }

    void aDocumentWithChangesOpensShowingTheirMarkupAndTheViewsChangeOnlyWhatIsShown()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("changes.docx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        const QByteArray original = makeDocx(para(run("one ") + inserted(1, "two ") + deleted(2, "three ", "Bob") + run("four")));
        f.write(original);
        f.close();
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY(w.openFile(path));
        write::WriteDoc *doc = w.document();
        QVERIFY(doc);
        QCOMPARE(w.reviewView(), WriteWindow::ReviewView::AllMarkup);
        QCOMPARE(doc->revisionView(), tp::kRevisionShowDeleted | tp::kRevisionMark | tp::kRevisionMarkFormat);
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two three four")});
        const QByteArray saved = documentXml(doc->toDocx());
        w.act(QStringLiteral("review.view.none"))->trigger();
        QCOMPARE(doc->revisionView(), 0);
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two four")});
        QVERIFY(!w.view()->reviewMarks().changeBars);
        w.act(QStringLiteral("review.view.simple"))->trigger();
        QCOMPARE(doc->revisionView(), 0);
        QVERIFY(w.view()->reviewMarks().changeBars);
        w.act(QStringLiteral("review.view.original"))->trigger();
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one three four")});
        w.act(QStringLiteral("review.view.all"))->trigger();
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two three four")});
        // Insertions and deletions off: the document as it stands, whatever else is on.
        w.act(QStringLiteral("review.show.insdel"))->trigger();
        QCOMPARE(doc->revisionView(), 0);
        w.act(QStringLiteral("review.show.insdel"))->trigger();
        // Revisions in balloons: the deletion leaves the line and is in the margin.
        w.act(QStringLiteral("review.balloons.balloons"))->trigger();
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two four")});
        QVERIFY(w.view()->reviewMarks().deletions);
        w.act(QStringLiteral("review.balloons.inline"))->trigger();
        QCOMPARE(lineTexts(*doc), QStringList{QStringLiteral("one two three four")});
        // None of it was an edit: the file and the history are as they were.
        QCOMPARE(documentXml(doc->toDocx()), saved);
        QVERIFY(!doc->canUndo());
        QVERIFY(!doc->isModified());
    }

    void acceptAndRejectByCommandKeepToOneStepAndMoveOn()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        auto made = open(makeDocx(para(run("a ") + inserted(1, "b ") + run("c ") + deleted(2, "d ") + run("e"))));
        w.setDocument(std::move(made), QString());
        write::WriteDoc *doc = w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        w.view()->setCursor(c);
        w.act(QStringLiteral("review.nextChange"))->trigger();
        QVERIFY(w.view()->cursor().hasSelection());
        QCOMPARE(revisionAt(doc->story(), w.view()->cursor().position())->kind, Revision::Insertion);
        w.act(QStringLiteral("review.accept"))->trigger();   // accepted, and on to the next
        QCOMPARE(revisionCount(doc->story()), 1);
        QCOMPARE(revisionAt(doc->story(), w.view()->cursor().position())->kind, Revision::Deletion);
        w.act(QStringLiteral("review.reject"))->trigger();
        QCOMPARE(revisionCount(doc->story()), 0);
        QCOMPARE(finalText(*doc), QStringLiteral("a b c d e"));
        QVERIFY(doc->undo());
        QVERIFY(doc->undo());
        QCOMPARE(revisionCount(doc->story()), 2);
        // "All shown" is those Show Markup has on.
        w.act(QStringLiteral("review.show.insdel"))->trigger();   // off
        QCOMPARE(w.act(QStringLiteral("review.acceptShown"))->isEnabled(), true);
        w.act(QStringLiteral("review.acceptShown"))->trigger();
        QCOMPARE(revisionCount(doc->story()), 2);   // (none shown)
        w.act(QStringLiteral("review.show.insdel"))->trigger();
        w.act(QStringLiteral("review.rejectShown"))->trigger();
        QCOMPARE(revisionCount(doc->story()), 0);
        // Accept All and Stop Tracking.
        QVERIFY(doc->undo());
        doc->setTrackChanges(true);
        w.act(QStringLiteral("review.acceptAllStop"))->trigger();
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(!doc->trackChanges());
    }

    void theReviewingPaneListsChangesAndCommentsAndItsButtonsWork()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        const QByteArray cm = commentElement(1, "Ann", "A question", "0000000A");
        auto made = open(makeDocxWithComments(para(run("see ") + commented(1, "this") + inserted(5, " more", "Bob") + deleted(6, " gone", "Bob")), cm,
                                              "<w15:commentEx w15:paraId=\"0000000A\" w15:done=\"0\"/>"));
        w.setDocument(std::move(made), QString());
        write::WriteDoc *doc = w.document();
        w.act(QStringLiteral("review.reviewingPane"))->trigger();
        ReviewPane *pane = w.reviewPane();
        QVERIFY(pane);
        QCOMPARE(pane->commentList()->topLevelItemCount(), 1);
        QVERIFY(pane->commentList()->topLevelItem(0)->text(0).contains(QStringLiteral("A question")));
        QVERIFY(pane->commentList()->topLevelItem(0)->text(0).contains(QStringLiteral("Ann")));
        QCOMPARE(pane->changeList()->topLevelItemCount(), 2);
        QVERIFY(pane->changeList()->topLevelItem(0)->text(0).contains(QStringLiteral("more")));
        QCOMPARE(pane->changeList()->topLevelItem(1)->text(1), QStringLiteral("Bob"));
        // Choosing a comment shows its words selected.
        pane->commentList()->topLevelItem(0)->setSelected(true);
        QCOMPARE(w.view()->cursor().selectedText().remove(QChar::ObjectReplacementCharacter), QStringLiteral("this"));
        // A reply from the pane.
        pane->replyField()->setText(QStringLiteral("An answer"));
        QVERIFY(pane->postReply());
        QCOMPARE(pane->commentList()->topLevelItem(0)->childCount(), 1);
        QVERIFY(pane->commentList()->topLevelItem(0)->child(0)->text(0).contains(QStringLiteral("An answer")));
        // Resolve, and it shows so; delete takes the thread.
        pane->commentList()->topLevelItem(0)->setSelected(true);
        auto *resolve = pane->findChild<QPushButton *>(QStringLiteral("resolveButton"));
        QVERIFY(resolve);
        QCOMPARE(resolve->text(), QStringLiteral("Resolve"));
        resolve->click();
        QVERIFY(write::commentById(*doc, 1)->done);
        QCOMPARE(pane->findChild<QPushButton *>(QStringLiteral("resolveButton"))->text(), QStringLiteral("Reopen"));
        // Accept a change from the list.
        pane->showPage(ReviewPane::Changes);
        pane->changeList()->topLevelItem(0)->setSelected(true);
        pane->findChild<QPushButton *>(QStringLiteral("acceptButton"))->click();
        QCOMPARE(revisionCount(doc->story()), 1);
        pane->commentList()->topLevelItem(0)->setSelected(true);
        pane->findChild<QPushButton *>(QStringLiteral("deleteCommentButton"))->click();
        QCOMPARE(pane->commentList()->topLevelItemCount(), 0);
        QVERIFY(write::comments(*doc).isEmpty());
    }

    void changeBarsAndCommentBalloonsAreDrawnOverThePagesAndNeverAreTheDocument()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        const QByteArray cm = commentElement(1, "Ann", "A comment in the margin", "0000000A");
        auto made = open(makeDocxWithComments(para(run("one ") + inserted(5, "two") + run(" ") + commented(1, "three")), cm));
        w.setDocument(std::move(made), QString());
        w.act(QStringLiteral("review.view.none"))->trigger();
        w.view()->viewport()->repaint();
        const QImage plain = w.view()->viewport()->grab().toImage();
        w.act(QStringLiteral("review.view.simple"))->trigger();
        w.view()->viewport()->repaint();
        const QImage marked = w.view()->viewport()->grab().toImage();
        QVERIFY(plain != marked);
        // The bar is red, in the left margin of the page; the balloon is at the right.
        int red = 0;
        for (int y = 0; y < marked.height(); ++y)
            for (int x = 0; x < marked.width(); ++x) {
                const QColor c = marked.pixelColor(x, y);
                if (c.red() > 170 && c.green() < 90 && c.blue() < 90 && plain.pixelColor(x, y) != c) ++red;
            }
        QVERIFY(red > 4);
        QVERIFY(!w.document()->isModified());
    }

    // ---------------------------------------------------------------- review fixes: cost
    void changesOfRunPropertiesAreResolvedInOnePassOverTheRuns()
    {
        const int n = 20000;
        QByteArray content;
        for (int i = 0; i < n; ++i)
            content += "<w:r><w:rPr><w:b/><w:rPrChange" + rev(i + 1) + "><w:rPr/></w:rPrChange></w:rPr><w:t>x</w:t></w:r><w:r><w:t>y</w:t></w:r>";
        for (const bool accept : {true, false}) {
            auto doc = open(makeDocx(para(content)));
            QVERIFY(doc);
            QCOMPARE(revisionCount(doc->story()), n);
            const qint64 before = revisionRunsVisitedForTest();
            QCOMPARE(accept ? acceptAll(*doc) : rejectAll(*doc), n);
            const qint64 visited = revisionRunsVisitedForTest() - before;
            QVERIFY2(visited < qint64(n) * 2 * 8, qPrintable(QStringLiteral("%1 runs looked at for %2 changes").arg(visited).arg(n)));
            QCOMPARE(revisionCount(doc->story()), 0);
            QVERIFY(doc->undo());
            QCOMPARE(revisionCount(doc->story()), n);
        }
    }

    void theWordsOfEachChangeAreFoundWithoutLookingAtTheRunsBeforeIt()
    {
        const int n = 5000;
        QByteArray content;
        for (int i = 0; i < n; ++i) content += inserted(i + 1, "ab") + run("-");
        auto doc = open(makeDocx(para(content)));
        QVERIFY(doc);
        const QVector<Revision> all = revisionsIn(doc->story());
        QCOMPARE(all.size(), n);
        // The words of the last change cost what the first's do: nothing of the thousands of runs before it is looked at.
        qint64 before = revisionRunsVisitedForTest();
        QCOMPARE(revisionText(doc->story(), all.first()), QStringLiteral("ab"));
        const qint64 first = revisionRunsVisitedForTest() - before;
        before = revisionRunsVisitedForTest();
        QCOMPARE(revisionText(doc->story(), all.last()), QStringLiteral("ab"));
        const qint64 last = revisionRunsVisitedForTest() - before;
        QVERIFY2(first > 0 && last > 0 && last <= first + 4, qPrintable(QStringLiteral("%1 runs looked at for the first change, %2 for the last").arg(first).arg(last)));
        before = revisionRunsVisitedForTest();
        for (const Revision &r : all) QCOMPARE(revisionText(doc->story(), r), QStringLiteral("ab"));
        const qint64 visited = revisionRunsVisitedForTest() - before;
        QVERIFY2(visited < qint64(n) * 20, qPrintable(QStringLiteral("%1 runs looked at for %2 changes").arg(visited).arg(n)));
    }

    void manyPeopleAreMetInTimeThatGrowsWithThem()
    {
        const int n = 20000;
        QByteArray content;
        for (int i = 0; i < n; ++i) content += "<w:ins w:id=\"" + QByteArray::number(i + 1) + "\" w:author=\"person" + QByteArray::number(i) + "\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:t>x</w:t></w:r></w:ins>";
        const QByteArray zip = makeDocx(para(content));
        const qint64 before = reviewerProbesForTest();
        auto doc = open(zip);
        QVERIFY(doc);
        const qint64 probes = reviewerProbesForTest() - before;
        QVERIFY2(probes > 0 && probes < qint64(n) * 8, qPrintable(QStringLiteral("%1 people looked at to meet %2 people").arg(probes).arg(n)));
        QCOMPARE(doc->reviewerIndex(QStringLiteral("person7")), 7);
    }

    void aChainOfRepliesIsSettledInOnePass()
    {
        const int n = 5000;
        QByteArray cms, ext, body;
        for (int i = 0; i < n; ++i) {
            const QByteArray para8 = QByteArray::number(i + 1, 16).rightJustified(8, '0').toUpper();
            cms += commentElement(i, "Ann", "c", para8);
            ext += "<w15:commentEx w15:paraId=\"" + para8 + "\" " + (i ? "w15:paraIdParent=\"" + QByteArray::number(i, 16).rightJustified(8, '0').toUpper() + "\" " : QByteArray()) + "w15:done=\"0\"/>";
            body += commented(i, "x");
        }
        auto doc = open(makeDocxWithComments(para(body), cms, ext));
        QVERIFY(doc);
        qint64 before = commentReplyStepsForTest();
        const QVector<Comment> all = comments(*doc);
        qint64 steps = commentReplyStepsForTest() - before;
        QVERIFY2(steps > 0 && steps <= qint64(n) * 3, qPrintable(QStringLiteral("%1 steps along the replies for a chain of %2").arg(steps).arg(n)));
        QCOMPARE(all.size(), n);
        QCOMPARE(all[5].parent, 4);
        // A loop through all of them is broken at one place only.
        QByteArray ext2;
        for (int i = 0; i < n; ++i) {
            const QByteArray para8 = QByteArray::number(i + 1, 16).rightJustified(8, '0').toUpper();
            ext2 += "<w15:commentEx w15:paraId=\"" + para8 + "\" w15:paraIdParent=\"" + QByteArray::number(((i + n - 1) % n) + 1, 16).rightJustified(8, '0').toUpper() + "\" w15:done=\"0\"/>";
        }
        auto loop = open(makeDocxWithComments(para(body), cms, ext2));
        before = commentReplyStepsForTest();
        const QVector<Comment> looped = comments(*loop);
        steps = commentReplyStepsForTest() - before;
        QVERIFY2(steps > 0 && steps <= qint64(n) * 3, qPrintable(QStringLiteral("%1 steps along the replies for a loop of %2").arg(steps).arg(n)));
        int roots = 0;
        for (const Comment &c : looped) roots += c.parent < 0;
        QCOMPARE(roots, 1);
    }

    void deletingAllCommentsIsOnePassAndOneStep()
    {
        const int n = 3000;
        QByteArray cms, body;
        for (int i = 0; i < n; ++i) {
            cms += commentElement(i + 1, "Ann", "text", QByteArray::number(i + 1, 16).rightJustified(8, '0').toUpper().constData());
            body += para(commented(i + 1, "word"));
        }
        auto doc = open(makeDocxWithComments(body, cms));
        QVERIFY(doc);
        QCOMPARE(int(comments(*doc).size()), n);
        const QByteArray saved = documentXml(doc->toDocx());
        const qint64 before = commentBlocksScannedForTest();
        QCOMPARE(deleteAllComments(*doc), n);
        const qint64 scanned = commentBlocksScannedForTest() - before;
        QVERIFY2(scanned < qint64(n) * 6, qPrintable(QStringLiteral("%1 paragraphs looked through for %2 comments").arg(scanned).arg(n)));
        QVERIFY(comments(*doc).isEmpty());
        QVERIFY(doc->undo());
        QVERIFY2(!doc->canUndo(), "Delete All was more than one step");
        QCOMPARE(int(comments(*doc).size()), n);
        QCOMPARE(documentXml(doc->toDocx()), saved);
        QVERIFY(doc->redo());
        QVERIFY(comments(*doc).isEmpty());
    }

    void typingInABigDocumentWithChangesCostsNothingMoreForEachKey()
    {
        QByteArray body;
        const int n = 30000;
        for (int i = 0; i < n; ++i) body += para(run("a ") + inserted(i + 1, "b"));
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        auto made = open(makeDocx(body));
        QVERIFY(made);
        w.setDocument(std::move(made), QString());
        const qint64 first = revisionBlocksScannedForTest();
        w.show();
        QTest::qWait(200);
        w.view()->viewport()->repaint();
        QVERIFY2(revisionBlocksScannedForTest() > first, "the pages were not painted with their marks");
        const qint64 blocks = revisionBlocksScannedForTest(), commentBlocks = commentBlocksScannedForTest();
        for (int i = 0; i < 10; ++i) {
            w.view()->insertText(QStringLiteral("k"));
            w.view()->viewport()->repaint();
        }
        QVERIFY2(revisionBlocksScannedForTest() - blocks < 10 * 300, qPrintable(QStringLiteral("%1 paragraphs looked through for changes").arg(revisionBlocksScannedForTest() - blocks)));
        QVERIFY2(commentBlocksScannedForTest() - commentBlocks < 10 * 300, qPrintable(QStringLiteral("%1 paragraphs looked through for comments").arg(commentBlocksScannedForTest() - commentBlocks)));
    }

    // ---------------------------------------------------------------- review fixes: behavior
    void deletingAcrossTheCellsOfATableWithTrackingMarksTheTextAndKeepsIt()
    {
        const QByteArray body = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr>"
                                "<w:tc><w:p><w:r><w:t>aaa</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>bbb</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextDocument *s = doc->story();
        QCOMPARE(s->blockCount(), 3);
        QTextCursor c(s);
        c.setPosition(s->findBlockByNumber(0).position() + 1);
        c.setPosition(s->findBlockByNumber(1).position() + 2, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("aaa\nbbb\nafter"));
        QCOMPARE(finalText(*doc), QStringLiteral("a\nb\nafter"));
        QCOMPARE(s->blockCount(), 3);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(x.count("<w:del "), 2);
        QCOMPARE(x.count("<w:tc>"), 2);
        QCOMPARE(acceptAll(*doc), 2);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("a\nb\nafter"));
        // A selection that holds the whole table: its words are marked, and the table stays.
        auto whole = open(makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + body));
        QVERIFY(whole);
        whole->setReviewer(QStringLiteral("Zed"), QString());
        whole->setTrackChanges(true);
        QTextCursor d(whole->story());
        d.setPosition(0);
        d.setPosition(whole->story()->characterCount() - 1, QTextCursor::KeepAnchor);
        removeSelection(*whole, d);
        QCOMPARE(textWithDeletions(*whole), QStringLiteral("before\naaa\nbbb\nafter"));
        QCOMPARE(finalText(*whole), QStringLiteral("\n\n\n"));
        QCOMPARE(documentXml(whole->toDocx()).count("<w:tc>"), 2);
        QVERIFY(whole->undo());
        QCOMPARE(revisionCount(whole->story()), 0);
    }

    void deletingAcrossEndnotesWithTrackingMarksTheTextOnBothSidesAndLeavesTheNotes()
    {
        // Two sections that each set their endnotes at their end: the story is First, Second, the notes of the first section, Third, and the notes of the second.
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray sect = "<w:sectPr><w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray body = para(run("First body") + "<w:r><w:endnoteReference w:id=\"2\"/></w:r>") + para(run("Second body"), sect) +
                                para(run("Third body") + "<w:r><w:endnoteReference w:id=\"3\"/></w:r>") + sect;
        const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                                 "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha body note</w:t></w:r></w:p></w:endnote>"
                                 "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Beta body note</w:t></w:r></w:p></w:endnote></w:endnotes>";
        w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/endnotes.xml"), xml(notes), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("endnotes"), QStringLiteral("/word/endnotes.xml"));
        const QByteArray zip = w.finish();
        auto doc = open(zip);
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextDocument *story = doc->story();
        int notesSeen = 0, third = -1;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            notesSeen += b.blockFormat().hasProperty(prop::EndnoteBlock);
            if (b.text().startsWith(QLatin1String("Third"))) third = b.blockNumber();
        }
        QVERIFY2(notesSeen >= 3 && third > 2, "the notes of the first section are set between its text and the next section's");
        const int blocks = story->blockCount();
        QTextCursor c(story);
        c.setPosition(story->findBlockByNumber(1).position() + 3);   // (after the first note's reference, so that the note stays)
        c.setPosition(story->findBlockByNumber(third).position() + 2, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        // The text on both sides is marked as deleted and is still there, no paragraph is joined to another, and the notes are as they were.
        const QString all = textWithDeletions(*doc);
        for (const char *text : {"First body", "Second body", "Third body", "Alpha body note"}) QVERIFY2(all.contains(QLatin1String(text)), qPrintable(all));
        QCOMPARE(story->blockCount(), blocks);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("<w:del ") && x.contains("ond body") && x.contains("Sec"), x.constData());
        QCOMPARE(x.count("<w:p>") + x.count("<w:p "), 3);
        QCOMPARE(partOf(doc->toDocx(), "/word/endnotes.xml"), partOf(zip, "/word/endnotes.xml"));
        QVERIFY(doc->undo());
        QCOMPARE(revisionCount(doc->story()), 0);
    }

    void deletingAcrossAFrameWithTrackingMarksTheTextOfBothAndJoinsNone()
    {
        const QByteArray frame = "<w:p><w:pPr><w:framePr w:w=\"3000\" w:h=\"1000\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:x=\"0\" w:y=\"0\"/></w:pPr><w:r><w:t>Frame text</w:t></w:r></w:p>";
        const QByteArray body = "<w:p><w:r><w:t>Before</w:t></w:r></w:p>" + frame + "<w:p><w:r><w:t>After</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QVERIFY(!doc->flow().frames.isEmpty());
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextDocument *s = doc->story();
        QCOMPARE(s->blockCount(), 3);
        QTextCursor c(s);
        c.setPosition(s->findBlockByNumber(0).position() + 2);
        c.setPosition(s->findBlockByNumber(2).position() + 2, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("Before\nFrame text\nAfter"));
        QCOMPARE(s->blockCount(), 3);
        QCOMPARE(finalText(*doc), QStringLiteral("Be\n\nter"));
        QVERIFY(!doc->flow().frames.isEmpty());
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.count("<w:framePr") == 1 && x.count("<w:del ") >= 3, x.constData());
        QVERIFY(doc->undo());
        QCOMPARE(revisionCount(doc->story()), 0);
    }

    void everyViewShowsWhatItsNameSaysOfAnEditedDocument()
    {
        const QByteArray nested = "<w:ins" + rev(5, "Ann") + "><w:del" + rev(6, "Bob") + "><w:r><w:delText xml:space=\"preserve\">four </w:delText></w:r></w:del></w:ins>";
        auto doc = open(makeDocx(para(run("AAABBB")) + para(run("one ") + inserted(1, "two ", "Ann") + deleted(2, "three ", "Bob") + nested + run("five")) +
                                 para(deleted(7, "whole"), "<w:rPr><w:del" + rev(8, "Bob") + "/></w:rPr>") + para(run("end"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.setPosition(3);
        insertParagraph(*doc, c);   // (an inserted mark, in the first paragraph)
        c.setPosition(doc->story()->findBlockByNumber(2).position());
        insertText(*doc, c, QStringLiteral("typed "));
        doc->setTrackChanges(false);
        const QByteArray saved = doc->toDocx();
        auto accepted = open(saved), rejected = open(saved);
        acceptAll(*accepted);
        rejectAll(*rejected);
        const QString acceptedText = textWithDeletions(*accepted), rejectedText = textWithDeletions(*rejected);
        QCOMPARE(acceptedText, QStringLiteral("AAA\nBBB\ntyped one two five\nend"));
        QCOMPARE(rejectedText, QStringLiteral("AAABBB\none three five\nwhole\nend"));
        auto shown = [&](int view) {
            doc->setRevisionView(view);
            return lineTexts(*doc).join(QLatin1Char('\n'));
        };
        QCOMPARE(shown(0), acceptedText);   // (No Markup, and Simple Markup, which differs by the bars only)
        QCOMPARE(shown(tp::kRevisionShowDeleted | tp::kRevisionHideInserted), rejectedText);   // (Original)
        QCOMPARE(shown(tp::kRevisionShowDeleted | tp::kRevisionMark), QStringLiteral("AAA\nBBB\ntyped one two three four five\nwhole\nend"));   // (All Markup)
    }

    void wordsThatAreDeletedAreNotCountedNorSpelledNorFound()
    {
        const QByteArray body = para(run("one ") + deleted(1, "badwrdz gone ") + run("two teh"));
        // The count of words, in every view.
        auto countShown = [&](const QByteArray &b, int view) {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.setDocument(open(makeDocx(b)), QString());
            w.document()->setRevisionView(view);
            w.show();
            QLabel *label = w.findChild<QLabel *>(QStringLiteral("wordCount"));
            // (the count comes from a 500 ms timer, after the view is set: wait for it, not for a fixed time; a slow runner took longer than 700 ms)
            QTest::qWaitFor([&] { return !label->text().isEmpty(); }, 15000);
            return label->text();
        };
        const QString plain = countShown(para(run("one two teh")), 0);
        QVERIFY(!plain.isEmpty());
        for (const int view : {0, tp::kRevisionShowDeleted | tp::kRevisionMark, tp::kRevisionShowDeleted | tp::kRevisionHideInserted})
            QCOMPARE(countShown(body, view), plain);
        // Spelling: the words that are deleted are not flagged (and the one that is not is).
        auto doc = open(makeDocx(body));
        SpellCheck sc(doc->story());
        QCOMPARE(flaggedWords(sc, *doc), QStringList{QStringLiteral("teh")});
        // Find: the deleted words are not found, and the words round them are one stretch.
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.setDocument(open(makeDocx(body)), QString());
        w.show();
        w.showFindPane();
        WriteFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("gone"));
        QVERIFY(!pane->findNext());
        pane->findBox()->setText(QStringLiteral("one two"));
        QVERIFY(pane->findNext());
    }

    void thePaneResolvesTheChangeOfTheRowAndNoOther()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        // A paragraph mark that was inserted is followed by a paragraph with an insertion: the mark's row is the mark.
        w.setDocument(open(makeDocx(para(inserted(1, "AAA"), "<w:rPr><w:ins" + rev(2) + "/></w:rPr>") + para(inserted(3, "BBB")))), QString());
        w.act(QStringLiteral("review.reviewingPane"))->trigger();
        ReviewPane *pane = w.reviewPane();
        QVERIFY(pane);
        auto rowOf = [&](Revision::Kind kind) -> QTreeWidgetItem * {
            for (int i = 0; i < pane->changeList()->topLevelItemCount(); ++i)
                if (pane->changeList()->topLevelItem(i)->data(0, Qt::UserRole + 2).toInt() == int(kind)) return pane->changeList()->topLevelItem(i);
            return nullptr;
        };
        pane->showPage(ReviewPane::Changes);
        QTreeWidgetItem *mark = rowOf(Revision::MarkInserted);
        QVERIFY(mark);
        mark->setSelected(true);
        pane->findChild<QPushButton *>(QStringLiteral("acceptButton"))->click();
        const QVector<Revision> left = revisionsIn(w.document()->story());
        QCOMPARE(left.size(), 2);
        for (const Revision &r : left) QCOMPARE(r.kind, Revision::Insertion);
        // An insertion that holds a deletion from its first place: its row is the insertion.
        w.setDocument(open(makeDocx(para("<w:ins" + rev(1, "Ann") + "><w:del" + rev(2, "Bob") + "><w:r><w:delText>y</w:delText></w:r></w:del>" + run("x") + "</w:ins>"))), QString());
        pane->refresh();
        QTreeWidgetItem *outer = rowOf(Revision::Insertion);
        QVERIFY(outer);
        outer->setSelected(true);
        pane->findChild<QPushButton *>(QStringLiteral("acceptButton"))->click();
        const QVector<Revision> inner = revisionsIn(w.document()->story());
        QCOMPARE(inner.size(), 1);
        QCOMPARE(inner[0].kind, Revision::Deletion);
    }

    void textThatIsFreeOfControlCharactersGoesIntoTheCommentParts()
    {
        auto doc = open(makeDocx(para(run("some words here"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed\x02 Y"), QStringLiteral("Z\x03"));
        QTextCursor c(doc->story());
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        const int id = addComment(*doc, c, QStringLiteral("bell\x07 and nul-ish \x01 text"));
        QVERIFY(id > 0);
        QVERIFY(replyToComment(*doc, id, QStringLiteral("reply\x08 text")) > 0);   // (a reply makes the extension and people parts)
        doc->setTrackChanges(true);
        QTextCursor t(doc->story());
        t.setPosition(doc->story()->characterCount() - 1);
        insertText(*doc, t, QStringLiteral("typed"));   // (a change made under the same name)
        const QByteArray saved = doc->toDocx();
        for (const char *name : {"/word/comments.xml", "/word/commentsExtended.xml", "/word/people.xml", "/word/document.xml"}) {
            const QByteArray part = partOf(saved, name);
            QVERIFY2(!part.isEmpty(), name);   // (each of the parts is written: the comment's person is in people.xml)
            wml::XmlTree tree;
            QString err;
            QVERIFY2(tree.parse(part, &err), qPrintable(QStringLiteral("%1: %2").arg(QString::fromLatin1(name), err)));
        }
        QVERIFY(partOf(saved, "/word/comments.xml").contains("bell and nul-ish  text"));
    }

    void aFieldThatIsDeletedWithTrackingHasItsCodeAsDeletedCode()
    {
        const QByteArray field = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                                 "<w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        auto doc = open(makeDocx(para(run("a ") + field + run(" b"))));
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        QTextCursor c(doc->story());
        c.select(QTextCursor::Document);
        removeSelection(*doc, c);
        const QByteArray x = documentXml(doc->toDocx());
        QRegularExpression del(QStringLiteral("<w:del [^>]*>(.*?)</w:del>"), QRegularExpression::DotMatchesEverythingOption);
        QString all;
        for (auto it = del.globalMatch(QString::fromUtf8(x)); it.hasNext();) all += it.next().captured(1);
        QVERIFY2(all.contains(QLatin1String("<w:delInstrText")) && !all.contains(QLatin1String("<w:instrText")), x.constData());
        QVERIFY2(!QString::fromUtf8(x).remove(del).contains(QLatin1String("delInstrText")), x.constData());
        // Rejected, the code is code again, outside any deletion.
        auto again = open(doc->toDocx());
        rejectAll(*again);
        const QByteArray y = documentXml(again->toDocx());
        QVERIFY2(y.contains("<w:instrText") && !y.contains("delInstrText") && !y.contains("<w:del"), y.constData());
    }

    void everyIdOfASavedDocumentIsUniqueAcrossChangesCommentsAndBookmarks()
    {
        // The body's highest id is 5, so the next one is 6: a header's change has 6, a note's change has 7, and an orphan comment (kept in the comments part, with no mark in the text) has 8.
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
        const QByteArray body = para("<w:bookmarkStart w:id=\"4\" w:name=\"mark\"/>" + run("hello world") + "<w:bookmarkEnd w:id=\"4\"/>" + inserted(5, "kept") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>") +
                                "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdH\"/></w:sectPr>";
        w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/header1.xml"), xml("<w:hdr " + ns + "><w:p><w:ins" + rev(6) + "><w:r><w:t>head</w:t></w:r></w:ins></w:p></w:hdr>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdH"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
        w.addPart(QStringLiteral("/word/footnotes.xml"), xml("<w:footnotes " + ns + "><w:footnote w:id=\"2\"><w:p><w:ins" + rev(7) + "><w:r><w:t>note</w:t></w:r></w:ins></w:p></w:footnote></w:footnotes>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
        w.addPart(QStringLiteral("/word/comments.xml"), xml("<w:comments " + ns + ">" + commentElement(8, "Ann", "orphan", "0000000A") + "</w:comments>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.comments+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("comments"), QStringLiteral("/word/comments.xml"));
        auto doc = open(w.finish());
        QVERIFY(doc);
        doc->setReviewer(QStringLiteral("Zed"), QString());
        doc->setTrackChanges(true);
        // The first new id is above every id of every part (the body's highest is 5; the header's, the note's, and the comment's are 6, 7, and 8).
        QVERIFY2(doc->nextRevisionId() > 8, "a new id must not be one the header, the notes, or the comments have");
        QTextCursor c(doc->story());
        c.setPosition(3);
        for (const QChar ch : QStringLiteral("abc")) insertText(*doc, c, QString(ch));
        // Bold on a stretch, then a paragraph split inside it and inside the insertion, and the bold text copied and pasted twice.
        c.setPosition(0);
        c.setPosition(4, QTextCursor::KeepAnchor);
        toggleBold(*doc, c);
        const Clip clip = copySelection(*doc, c);
        c.setPosition(2);
        insertParagraph(*doc, c);
        c.movePosition(QTextCursor::End);
        pasteClip(*doc, c, clip);
        pasteClip(*doc, c, clip);
        QTextCursor d(doc->story());
        d.setPosition(1);
        d.setPosition(2, QTextCursor::KeepAnchor);
        QVERIFY(addComment(*doc, d, QStringLiteral("remark")) > 0);
        const QByteArray saved = doc->toDocx();
        // The ids of every part, by what they name.
        static const QRegularExpression changeId(QStringLiteral("<w:(?:ins|del|moveFrom|moveTo|rPrChange|pPrChange)\\b[^>]*?w:id=\"(\\d+)\""));
        static const QRegularExpression commentId(QStringLiteral("<w:comment\\b[^>]*?w:id=\"(\\d+)\""));
        static const QRegularExpression rangeId(QStringLiteral("<w:commentRangeStart\\b[^>]*?w:id=\"(\\d+)\""));
        static const QRegularExpression bookmarkId(QStringLiteral("<w:bookmarkStart\\b[^>]*?w:id=\"(\\d+)\""));
        QStringList changes, comments, ranges, bookmarks;
        auto gather = [&](const QRegularExpression &re, const QString &x, QStringList *into) {
            for (auto it = re.globalMatch(x); it.hasNext();) into->push_back(it.next().captured(1));
        };
        int partsSeen = 0;
        for (const char *name : {"/word/document.xml", "/word/header1.xml", "/word/footer1.xml", "/word/footnotes.xml", "/word/endnotes.xml", "/word/comments.xml"}) {
            const QString x = QString::fromUtf8(partOf(saved, name));
            if (x.isEmpty()) continue;
            ++partsSeen;
            gather(changeId, x, &changes);
            gather(commentId, x, &comments);
            gather(rangeId, x, &ranges);
            gather(bookmarkId, x, &bookmarks);
        }
        QVERIFY2(partsSeen >= 4, "the body, the header, the notes, and the comments are all saved");
        auto unique = [](const QStringList &l) { return QSet<QString>(l.cbegin(), l.cend()).size() == l.size(); };
        const QString all = changes.join(QLatin1Char(',')) + QLatin1Char('|') + comments.join(QLatin1Char(',')) + QLatin1Char('|') + bookmarks.join(QLatin1Char(','));
        QVERIFY2(unique(changes), qPrintable(all));
        QVERIFY2(unique(comments) && unique(ranges) && unique(bookmarks), qPrintable(all));
        const QSet<QString> c1(changes.cbegin(), changes.cend()), c2(comments.cbegin(), comments.cend()), c3(bookmarks.cbegin(), bookmarks.cend());
        QVERIFY2(!c1.intersects(c2) && !c1.intersects(c3) && !c2.intersects(c3), qPrintable(all));
        for (const QString &id : ranges) QVERIFY2(c2.contains(id), qPrintable(all));
        // The ids the file had are kept, and new changes took ids above all of them.
        for (const QString &id : {QStringLiteral("5"), QStringLiteral("6"), QStringLiteral("7")}) QVERIFY2(c1.contains(id), qPrintable(all));
        QVERIFY2(c2.contains(QStringLiteral("8")) && c3.contains(QStringLiteral("4")), qPrintable(all));
        QVERIFY2(changes.size() > 3, qPrintable(all));
    }

    void aCarriageReturnLaysOutExactlyAsTheLineBreakItReplaced()
    {
        auto layout = [&](const char *mark, QStringList *out) {
            auto doc = open(makeDocx(para("<w:r><w:rPr><w:sz w:val=\"28\"/></w:rPr><w:t>one two</w:t></w:r><w:r><w:rPr><w:b/><w:sz w:val=\"36\"/></w:rPr>" + QByteArray(mark) + "</w:r><w:r><w:t>three four</w:t></w:r>") +
                                     para(run("next"))));
            if (!doc) return false;
            *out << lineTexts(*doc).join(QLatin1Char('|'));
            for (const auto &l : doc->pager().lines(0)) *out << QStringLiteral("%1,%2,%3,%4,%5,%6").arg(l.pos).arg(l.end).arg(l.rect.left()).arg(l.rect.top()).arg(l.rect.width()).arg(l.rect.height());
            return true;
        };
        QStringList cr, br;
        QVERIFY(layout("<w:cr/>", &cr) && layout("<w:br/>", &br));
        QCOMPARE(cr, br);
        QVERIFY(cr.size() >= 4);
        QVERIFY(cr[0].contains(QLatin1Char('|')));
    }

    void acceptAllAndRejectAllCoverHeadersNotesAndTextBoxesInOneStep()
    {
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:v=\"urn:schemas-microsoft-com:vml\"";
        auto build = [&]() {
            office::PackageWriter w;
            const QByteArray box = "<w:r><w:pict><v:shape style=\"width:100pt;height:40pt\"><v:textbox><w:txbxContent><w:p>" + inserted(31, "boxin ") + deleted(32, "boxout") + "</w:p></w:txbxContent></v:textbox></v:shape></w:pict></w:r>";
            const QByteArray body = para(run("body ") + inserted(30, "new ") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r>" + box) +
                                    "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdH\"/><w:footerReference w:type=\"default\" r:id=\"rIdF\"/></w:sectPr>";
            w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
            w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            w.addPart(QStringLiteral("/word/header1.xml"), xml("<w:hdr " + ns + "><w:p>" + inserted(33, "headin ") + deleted(34, "headout") + "</w:p></w:hdr>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdH"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
            w.addPart(QStringLiteral("/word/footer1.xml"), xml("<w:ftr " + ns + "><w:p>" + inserted(37, "footin ") + deleted(38, "footout") + "</w:p></w:ftr>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdF"), rel + QStringLiteral("footer"), QStringLiteral("/word/footer1.xml"));
            w.addPart(QStringLiteral("/word/footnotes.xml"), xml("<w:footnotes " + ns + "><w:footnote w:id=\"2\"><w:p>" + inserted(35, "noteIn ") + deleted(36, "noteOut") + "</w:p></w:footnote></w:footnotes>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("footnotes"), QStringLiteral("/word/footnotes.xml"));
            w.addPart(QStringLiteral("/word/endnotes.xml"), xml("<w:endnotes " + ns + "><w:endnote w:id=\"2\"><w:p>" + inserted(39, "endIn ") + deleted(40, "endOut") + "</w:p></w:endnote></w:endnotes>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml"));
            w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("endnotes"), QStringLiteral("/word/endnotes.xml"));
            return w.finish();
        };
        // What each part must hold after the change is accepted, and after it is rejected.
        struct Part { const char *name; const char *in; const char *out; };
        const Part parts[] = {{"/word/document.xml", "boxin", "boxout"}, {"/word/header1.xml", "headin", "headout"}, {"/word/footer1.xml", "footin", "footout"},
                              {"/word/footnotes.xml", "noteIn", "noteOut"}, {"/word/endnotes.xml", "endIn", "endOut"}};
        for (const bool accept : {true, false}) {
            auto doc = open(build());
            QVERIFY(doc);
            if (accept) acceptAll(*doc); else rejectAll(*doc);
            const QByteArray saved = doc->toDocx();
            for (const Part &part : parts) {
                const QByteArray x = partOf(saved, part.name);
                const QString what = QString::fromLatin1(part.name) + QLatin1Char(' ') + QString::fromUtf8(x);
                QVERIFY2(!x.isEmpty(), qPrintable(what));
                QVERIFY2(!xmlHasChanges(x), qPrintable(what));
                QVERIFY2(x.contains(accept ? part.in : part.out), qPrintable(what));
                QVERIFY2(!x.contains(accept ? part.out : part.in), qPrintable(what));
            }
            // One step brings every change back, and one redo does it all again.
            doc->undo();
            QVERIFY(!doc->canUndo());
            const QByteArray undone = doc->toDocx();
            for (const Part &part : parts) {
                const QByteArray x = partOf(undone, part.name);
                QVERIFY2(xmlHasChanges(x) && x.contains(part.in) && x.contains(part.out), part.name);
            }
            QVERIFY(doc->canRedo());
            doc->redo();
            const QByteArray again = doc->toDocx();
            for (const Part &part : parts) QVERIFY2(!xmlHasChanges(partOf(again, part.name)) && partOf(again, part.name).contains(accept ? part.in : part.out), part.name);
            QVERIFY(!doc->canRedo());
        }
    }

    void oneHalfOfAMoveResolvesItsPairToo()
    {
        const QByteArray from = "<w:moveFromRangeStart w:id=\"1\" w:name=\"move9\"/><w:moveFrom" + rev(2) + "><w:r><w:delText>moved</w:delText></w:r></w:moveFrom><w:moveFromRangeEnd w:id=\"1\"/>";
        const QByteArray to = "<w:moveToRangeStart w:id=\"3\" w:name=\"move9\"/><w:moveTo" + rev(4) + "><w:r><w:t>moved</w:t></w:r></w:moveTo><w:moveToRangeEnd w:id=\"3\"/>";
        const QByteArray body = para(run("keep ") + from) + para(to + run(" tail"));
        for (const bool accept : {true, false}) {
            for (const bool fromHalf : {true, false}) {
                auto doc = open(makeDocx(body));
                QVERIFY(doc);
                QVector<Revision> which;
                for (const Revision &r : revisionsIn(doc->story()))
                    if (r.kind == (fromHalf ? Revision::MovedFrom : Revision::MovedTo)) which << r;
                QCOMPARE(which.size(), 1);
                QCOMPARE(resolveRevisions(*doc, doc->story(), which, accept), 2);
                QCOMPARE(revisionCount(doc->story()), 0);
                QCOMPARE(textWithDeletions(*doc), accept ? QStringLiteral("keep \nmoved tail") : QStringLiteral("keep moved\n tail"));
            }
        }
    }

    void changesNestedPastTheReadersCapAreKeptAsTheyWereAndTheRestResolved()
    {
        // The reader follows 24 levels of containers (kMaxContainerDepth in wmlread.cpp); what is nested deeper is kept as the XML it is.
        const int depth = 70, cap = 24;
        QByteArray open_, close_;
        for (int i = 0; i < depth; ++i) {
            open_ += "<w:ins" + rev(i + 1) + ">";
            close_ += "</w:ins>";
        }
        auto doc = open(makeDocx(para(run("a") + open_ + run("deep") + close_ + run("b"))));
        QVERIFY(doc);
        const int found = revisionCount(doc->story());
        QCOMPARE(found, cap);
        // Every level is in the saved file.
        QCOMPARE(documentXml(doc->toDocx()).count("<w:ins "), depth);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("ab"));
        QCOMPARE(acceptAll(*doc), cap);
        QCOMPARE(revisionCount(doc->story()), 0);
        QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(x.count("<w:ins "), depth - cap);   // (the levels past the cap stay as they were)
        QVERIFY(x.contains("deep"));
        QVERIFY(doc->undo());
        QCOMPARE(revisionCount(doc->story()), cap);
        QCOMPARE(rejectAll(*doc), cap);
        x = documentXml(doc->toDocx());
        QVERIFY2(!x.contains("<w:ins ") && !x.contains("deep"), "rejecting the outermost insertion takes what is inside it, the piece too");
    }

    void changesNestedPastTheWritersCapInATextBoxAreKeptAndTheRestResolved()
    {
        // A text box is resolved on its XML, which is followed 64 levels deep (kMostDepth in reviewxml.cpp): the box (a pict, a shape, a textbox, the content, a paragraph) takes 4 of them,
        // so the insertions of the first 60 levels are accepted and the 10 inside the 61st stay as they were.
        const int depth = 70;
        QByteArray open_, close_;
        for (int i = 0; i < depth; ++i) {
            open_ += "<w:ins" + rev(i + 1) + ">";
            close_ += "</w:ins>";
        }
        const QByteArray pict = "<w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\"><v:shape style=\"width:100pt;height:40pt\"><v:textbox><w:txbxContent><w:p>" + open_ + "<w:r><w:t>deep</w:t></w:r>" + close_ + "</w:p></w:txbxContent></v:textbox></v:shape></w:pict></w:r>";
        const QByteArray resolved = resolveChangesInXml(pict.mid(pict.indexOf("<w:pict"), pict.lastIndexOf("</w:pict>") + 9 - pict.indexOf("<w:pict")), true);
        QCOMPARE(resolved.count("<w:ins "), 10);
        QVERIFY(resolved.contains("deep"));
        const QByteArray rejected = resolveChangesInXml(pict.mid(pict.indexOf("<w:pict"), pict.lastIndexOf("</w:pict>") + 9 - pict.indexOf("<w:pict")), false);
        QVERIFY2(!rejected.contains("<w:ins ") && !rejected.contains("deep"), "rejecting the outermost insertion takes what is inside it");
        // The same through Accept All of a document: the box is made again from the XML that was resolved.
        auto doc = open(makeDocx(para(run("a") + pict + run("b"))));
        QVERIFY(doc);
        QCOMPARE(documentXml(doc->toDocx()).count("<w:ins "), depth);
        acceptAll(*doc);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(x.count("<w:ins "), 10);
        QVERIFY(x.contains("deep"));
    }

    // ---------------------------------------------------------------- hostile files
    void manyChangesInOneParagraphAreResolvedWithoutTrouble()
    {
        QByteArray content;
        const int n = 20000;
        for (int i = 0; i < n; ++i) content += i % 2 ? deleted(2 * i, "d") : inserted(2 * i, "i");
        auto doc = open(makeDocx(para(content)));
        QVERIFY(doc);
        QCOMPARE(revisionCount(doc->story()), n);
        QCOMPARE(rejectAll(*doc), n);
        QCOMPARE(textWithDeletions(*doc).size(), n / 2);
        QVERIFY(doc->undo());
        QCOMPARE(revisionCount(doc->story()), n);
        QCOMPARE(acceptAll(*doc), n);
        QCOMPARE(textWithDeletions(*doc).size(), n / 2);
    }

    void changesWithNoPartnerOrNoTextAreHandled()
    {
        // A paragraph with a mark deleted at the end of the document, a deletion with nothing in it, and an insertion that was never closed in the story.
        const QByteArray body = para(run("x") + "<w:del" + rev(1) + "></w:del>", "<w:rPr><w:del" + rev(2) + "/></w:rPr>");
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QVERIFY(acceptAll(*doc) >= 0);
        QCOMPARE(textWithDeletions(*doc), QStringLiteral("x"));
        QCOMPARE(revisionCount(doc->story()), 0);
        QVERIFY(!documentXml(doc->toDocx()).isEmpty());
    }
};

QTEST_MAIN(ReviewTests)
#include "tst_write_review.moc"
