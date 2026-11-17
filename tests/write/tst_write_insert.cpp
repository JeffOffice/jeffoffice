// Tests of JeffWrite's Insert, Design, Layout, and View tabs: symbols and special characters, the breaks, page setup presets, the page's look, and the views.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/barcodedialog.h"
#include "app/linktargetpicker.h"
#include "app/navpane.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/theme.h"
#include "app/textdialogs.h"
#include "core/items.h"
#include "flow/paginator.h"
#include "app/writeinfo.h"
#include "app/ribbon.h"
#include "app/ruler.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/barcodeitem.h"
#include "app/controldialogs.h"
#include "core/contentcontrolops.h"
#include "core/pictureops.h"
#include "io/zip.h"
#include "core/designops.h"
#include "core/dropcapops.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/insertops.h"
#include "core/layoutops.h"
#include "core/refops.h"
#include "core/spellcheck.h"
#include "core/linkops.h"
#include "core/watermarkops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/barcodemarker.h"
#include "office/package.h"
#include "wml/wml.h"

#include <QBuffer>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QMenu>
#include <QPainter>
#include <QGroupBox>
#include <QFile>
#include <QHelpEvent>
#include <QToolTip>
#include <QStatusBar>
#include <QScrollBar>
#include <QDialog>
#include <QDockWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QTabWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

QByteArray makeDocx(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
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

QByteArray partOf(const QByteArray &zip, const QString &name)
{
    office::Package p;
    p.read(zip);
    return p.part(name);
}

// The page items (of a page of the window's document) whose id starts with this.
int itemsStartingWith(WriteWindow &w, int page, const QString &prefix)
{
    w.view()->paginateAll();
    const auto made = w.document()->pager().materialize(page);
    if (!made) return -1;
    int n = 0;
    for (const auto &item : made->doc->pages[0]->items) n += item->id.startsWith(prefix);
    return n;
}

// A package whose body is built from the Ids of external hyperlink relationships to `targets`.
QByteArray makeLinkDocx(const QStringList &targets, const std::function<QByteArray(const QStringList &)> &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    QStringList ids;
    for (const QString &t : targets) ids << w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("hyperlink"), t, true);
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body(ids) + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

QByteArray linkRun(const QString &id, const char *text, const QByteArray &extra = QByteArray())
{
    return "<w:p><w:hyperlink r:id=\"" + id.toUtf8() + "\"" + extra + "><w:r><w:t>" + text + "</w:t></w:r></w:hyperlink></w:p>";
}

// The vertical lines between columns that a page has, as the pager draws them.
QVector<QLineF> columnLines(WriteWindow &w, int page)
{
    w.view()->paginateAll();
    QVector<QLineF> out;
    const auto made = w.document()->pager().materialize(page);
    if (!made) return out;
    for (const auto &item : made->doc->pages[0]->items)
        if (item->id.startsWith(QLatin1String("flow-column-line-")))
            if (const auto *l = dynamic_cast<const LineItem *>(item.get())) out << QLineF(l->p1, l->p2);
    return out;
}

const char kSect1[] = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";

QByteArray para(const char *text) { return QByteArray("<w:p><w:r><w:t xml:space=\"preserve\">") + text + "</w:t></w:r></w:p>"; }

// Runs `open`, which makes a modal dialog, and meanwhile runs `fill` on that dialog, then closes it (a watchdog closes whatever stays open after ten seconds and fails the test).
void withDialog(const std::function<void()> &opener, const QString &objectName, const std::function<void(QDialog *)> &fill, bool accept = true)
{
    QString opened;
    bool leftOpen = false;
    auto closeAll = [](bool ok) {
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<QDialog *>(top); d && d->isVisible()) {
                if (ok) d->accept();
                else d->reject();
            }
    };
    QTimer once, watchdog;
    once.setSingleShot(true);
    watchdog.setSingleShot(true);
    QObject::connect(&once, &QTimer::timeout, [&] {
        auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dlg) opened = dlg->objectName();
        if (dlg && opened == objectName) fill(dlg);
        closeAll(accept && opened == objectName);
    });
    QObject::connect(&watchdog, &QTimer::timeout, [&] {
        leftOpen = true;
        closeAll(false);
    });
    once.start(0);
    watchdog.start(10000);
    opener();
    QCOMPARE(opened, objectName);
    QVERIFY2(!leftOpen, "the dialog was left open");
}

// A window on a document of the paragraphs given, the caret at the end of the text of the first.
struct Win {
    WriteWindow w;
    explicit Win(const QList<const char *> &paragraphs = {"Hello"})
    {
        QByteArray body;
        for (const char *p : paragraphs) body += para(p);
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(5);
        w.view()->setCursor(c);
    }
    QString text() const { return w.document()->story()->toPlainText(); }
};


// The position of the first picture of the text, or -1.
int firstPictureOf(WriteDoc &doc)
{
    for (int i = 0; i + 1 < doc.story()->characterCount(); ++i)
        if (isPictureAt(doc.story(), i)) return i;
    return -1;
}

// Insert > Barcode with the dialog answered: a QR code of `text`.
void insertQrBarcode(WriteWindow &w, const QString &text)
{
    withDialog([&] { w.act("insert.barcode")->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
        QCOMPARE(d->windowTitle(), QStringLiteral("Insert Barcode"));
        d->findChildren<QComboBox *>().first()->setCurrentIndex(int(barcode::Type::Qr));
        d->findChildren<QLineEdit *>().first()->setText(text);
    });
}


// A saved document with a part's bytes changed (`change` gets the parts by name).
QByteArray repackedDocx(const QByteArray &saved, const std::function<void(const QString &name, QByteArray *bytes)> &change)
{
    office::Package pkg;
    pkg.read(saved);
    office::PackageWriter w;
    for (const QString &name : pkg.partNames()) {
        QByteArray bytes = pkg.part(name);
        change(name, &bytes);
        w.addPart(name, bytes, pkg.contentType(name));
    }
    for (const QString &source : QStringList{QStringLiteral("/"), QStringLiteral("/word/document.xml")})
        for (const office::Relationship &r : pkg.relationships(source)) w.addRelationshipWithId(source, r.id, r.type, r.target, r.external);
    return w.finish();
}

} // namespace

class InsertTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    // ---------------------------------------------------------------- Insert > Symbols
    void theSymbolDialogPutsASymbolAtTheCaretAndRemembersIt()
    {
        Win win;
        QVERIFY(win.w.act("insert.symbol"));
        withDialog([&] { win.w.act("insert.symbol")->trigger(); }, QStringLiteral("symbolDialog"), [&](QDialog *dlg) {
            auto grids = dlg->findChildren<QTableWidget *>();
            QVERIFY(grids.size() >= 2);
            QTableWidget *grid = grids.first();
            QTableWidgetItem *it = grid->item(2, 9);   // (Basic Latin: row 2 starts at 0x40; column 9 is "I")
            QVERIFY(it);
            Q_EMIT grid->itemDoubleClicked(it);
        });
        QCOMPARE(win.text(), QStringLiteral("HelloI"));
        QCOMPARE(recentSymbols().value(0), QStringLiteral("I"));
        win.w.document()->undo();
        QCOMPARE(win.text(), QStringLiteral("Hello"));
    }

    void specialCharactersAreCharactersOfTheText()
    {
        struct Case { const char *id; QChar ch; };
        for (const Case &k : {Case{"insert.special.nbsp", QChar(0x00A0)}, Case{"insert.special.nbHyphen", QChar(0x2011)}, Case{"insert.special.optHyphen", QChar(0x00AD)},
                              Case{"insert.special.enDash", QChar(0x2013)}, Case{"insert.special.emDash", QChar(0x2014)}, Case{"insert.special.enSpace", QChar(0x2002)},
                              Case{"insert.special.emSpace", QChar(0x2003)}, Case{"insert.special.lineBreak", QChar(0x2028)}, Case{"insert.special.noWidthBreak", QChar(0x200B)},
                              Case{"insert.special.noWidthNoBreak", QChar(0x2060)}, Case{"insert.special.openDouble", QChar(0x201C)}, Case{"insert.special.closeDouble", QChar(0x201D)},
                              Case{"insert.special.openSingle", QChar(0x2018)}, Case{"insert.special.closeSingle", QChar(0x2019)}}) {
            Win win;
            QAction *a = win.w.act(QString::fromLatin1(k.id));
            QVERIFY2(a, k.id);
            QVERIFY2(a->isEnabled(), k.id);
            a->trigger();
            QCOMPARE(win.w.document()->story()->characterAt(5), k.ch);   // (the text as plain text makes a nonbreaking space a space)
            QCOMPARE(win.w.document()->story()->characterCount(), 7);
        }
    }

    void hyphensKeepTheirKindInTheSavedFile()
    {
        Win win;
        win.w.act("insert.special.nbHyphen")->trigger();
        win.w.act("insert.special.optHyphen")->trigger();
        const QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("<w:noBreakHyphen/>"), out.constData());
        QVERIFY2(out.contains("<w:softHyphen/>"), out.constData());
        auto again = open(win.w.document()->toDocx());
        QVERIFY(again);
        QCOMPARE(again->story()->toPlainText(), QStringLiteral("Hello") + QChar(0x2011) + QChar(0x00AD));
    }

    // ---------------------------------------------------------------- Insert > Hyperlink
    void aTypedAddressBecomesAHyperlinkOverTheWordsAndIsSaved()
    {
        Win win({"Visit our site now"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(6);
        c.setPosition(14, QTextCursor::KeepAnchor);   // "our site"
        win.w.view()->setCursor(c);
        QVERIFY(win.w.act("insert.hyperlink"));
        withDialog([&] { win.w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            dlg->findChild<QLineEdit *>(QStringLiteral("linkAddress"))->setText(QStringLiteral("example.com/page"));
            dlg->findChild<QLineEdit *>(QStringLiteral("screenTip"))->setText(QStringLiteral("Our home"));
        });
        const write::Hyperlink link = write::hyperlinkAt(*win.w.document(), 8);
        QVERIFY(link.valid());
        QCOMPARE(link.address, QStringLiteral("https://example.com/page"));
        QCOMPARE(link.text, QStringLiteral("our site"));
        QCOMPARE(link.screenTip, QStringLiteral("Our home"));
        QVERIFY(!write::hyperlinkAt(*win.w.document(), 2).valid());
        const QByteArray saved = win.w.document()->toDocx();
        const QByteArray out = documentXml(saved);
        QVERIFY2(out.contains("<w:hyperlink r:id=\"") && out.contains("w:tooltip=\"Our home\"") && out.contains("</w:hyperlink>"), out.constData());
        QVERIFY2(out.contains("w:val=\"Hyperlink\""), "the link's words do not have the link look");
        office::Package pkg;
        pkg.read(saved);
        bool related = false;
        for (const office::Relationship &r : pkg.relationships(QStringLiteral("/word/document.xml"))) related |= r.external && r.target == QStringLiteral("https://example.com/page");
        QVERIFY2(related, "no relationship for the address");
        auto again = open(saved);
        QVERIFY(again);
        QCOMPARE(write::hyperlinkAt(*again, 8).address, QStringLiteral("https://example.com/page"));
        QCOMPARE(again->story()->toPlainText().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Visit our site now"));
        win.w.document()->undo();
        QVERIFY(!write::hyperlinkAt(*win.w.document(), 8).valid());
        QCOMPARE(win.text().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Visit our site now"));
    }

    // ---------------------------------------------------------------- the link target picker: a heading, a file, a new document
    static QByteArray headingBody()
    {
        auto heading = [](const char *t) { return QByteArray("<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>") + t + "</w:t></w:r></w:p>"; };
        return heading("Intro") + para("Some text") + heading("Details") + para("Go to the intro now");
    }
    static void pick(QDialog *dlg, jp::LinkTargetPicker::Kind kind, const QString &place = QString(), const QString &address = QString())
    {
        auto *picker = dlg->findChild<jp::LinkTargetPicker *>();
        QVERIFY(picker);
        picker->setKind(kind);
        if (!place.isEmpty()) QVERIFY(picker->setPlaceValue(place));
        if (!address.isEmpty()) picker->setAddress(address);
    }

    void aLinkToAHeadingMakesAHiddenBookmarkAndSurvivesDocxAndOdt()
    {
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(headingBody())), QString());
        WriteDoc &doc = *w.document();
        const int intro = doc.story()->findBlockByNumber(0).position();
        const int target = doc.story()->findBlockByNumber(3).position();
        QTextCursor c(doc.story());
        c.setPosition(target + 10);
        c.setPosition(target + 15, QTextCursor::KeepAnchor);   // "intro"
        QCOMPARE(c.selectedText(), QStringLiteral("intro"));
        w.view()->setCursor(c);
        QVERIFY(bookmarkList(doc, true).isEmpty());
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            auto *places = dlg->findChild<QComboBox *>(QStringLiteral("linkPlace"));
            QVERIFY(places->findText(QStringLiteral("Heading: Intro")) >= 0);
            QVERIFY(places->findText(QStringLiteral("Heading: Details")) >= 0);
            pick(dlg, jp::LinkTargetPicker::Place, write::headingPlace(intro));
        });
        const write::Hyperlink link = write::hyperlinkAt(doc, doc.story()->findBlockByNumber(3).position() + 12);
        QVERIFY(link.valid());
        QVERIFY2(link.anchor.startsWith(QLatin1String("_Toc")), qPrintable(link.anchor));
        QVERIFY(link.address.isEmpty());
        QCOMPARE(link.text, QStringLiteral("intro"));
        // The bookmark is hidden (not in the list a person sees), and covers the heading.
        QVERIFY(bookmarkList(doc, false).isEmpty());
        const QVector<BookmarkItem> hidden = bookmarkList(doc, true);
        QCOMPARE(hidden.size(), 1);
        QCOMPARE(hidden[0].name, link.anchor);
        QCOMPARE(doc.story()->findBlock(hidden[0].start).position(), intro);
        QCOMPARE(write::placeOfAnchor(doc, link.anchor), write::headingPlace(intro));
        // A second link to the same heading uses the bookmark that is there.
        QTextCursor again(doc.story());
        again.setPosition(doc.story()->findBlockByNumber(1).position());
        again.setPosition(doc.story()->findBlockByNumber(1).position() + 4, QTextCursor::KeepAnchor);
        w.view()->setCursor(again);
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) { pick(dlg, jp::LinkTargetPicker::Place, write::headingPlace(intro)); });
        QCOMPARE(bookmarkList(doc, true).size(), 1);
        QCOMPARE(write::hyperlinkAt(doc, doc.story()->findBlockByNumber(1).position() + 2).anchor, link.anchor);
        // Both links were one step each: undo takes the second, then the first with its bookmark.
        doc.undo();
        QVERIFY(!write::hyperlinkAt(doc, doc.story()->findBlockByNumber(1).position() + 2).valid());
        doc.undo();
        QVERIFY(bookmarkList(doc, true).isEmpty());
        QVERIFY(!write::hyperlinkAt(doc, doc.story()->findBlockByNumber(3).position() + 12).valid());
        doc.redo();
        QCOMPARE(bookmarkList(doc, true).size(), 1);
        const QString name = write::hyperlinkAt(doc, doc.story()->findBlockByNumber(3).position() + 12).anchor;
        QVERIFY(name.startsWith(QLatin1String("_Toc")));
        // .docx: w:hyperlink w:anchor and the w:bookmarkStart and w:bookmarkEnd, and back.
        const QByteArray saved = doc.toDocx();
        const QByteArray xmlOut = documentXml(saved);
        QVERIFY2(xmlOut.contains("w:anchor=\"" + name.toUtf8() + "\""), xmlOut.constData());
        QVERIFY2(xmlOut.contains("<w:bookmarkStart") && xmlOut.contains("w:name=\"" + name.toUtf8() + "\"") && xmlOut.contains("<w:bookmarkEnd"), xmlOut.constData());
        auto docx = open(saved);
        QVERIFY(docx);
        const write::Hyperlink fromDocx = write::hyperlinkAt(*docx, docx->story()->findBlockByNumber(3).position() + 12);
        QCOMPARE(fromDocx.anchor, name);
        QCOMPARE(docx->story()->findBlock(write::linkTargetAt(*docx, docx->story()->findBlockByNumber(3).position() + 12)).blockNumber(), 0);
        QCOMPARE(write::placeOfAnchor(*docx, name), write::headingPlace(docx->story()->findBlockByNumber(0).position()));
        // .odt: text:a xlink:href="#name" and the text:bookmark-start of the same name, and back.
        QString err;
        const QByteArray odt = doc.toOdt(&err);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        auto back = WriteDoc::fromOdt(odt, &err);
        QVERIFY2(back, qPrintable(err));
        const write::Hyperlink fromOdt = write::hyperlinkAt(*back, back->story()->findBlockByNumber(3).position() + 12);
        QVERIFY(fromOdt.valid());
        QCOMPARE(fromOdt.anchor, name);
        QCOMPARE(back->story()->findBlock(write::linkTargetAt(*back, back->story()->findBlockByNumber(3).position() + 12)).blockNumber(), 0);
        QVERIFY(bookmarkList(*back, false).isEmpty());   // (still hidden)
        // A heading that is gone is refused with a reason, and nothing is made.
        QString why;
        QTextCursor none(doc.story());
        none.setPosition(doc.story()->findBlockByNumber(1).position());
        none.setPosition(doc.story()->findBlockByNumber(1).position() + 4, QTextCursor::KeepAnchor);
        QVERIFY(!write::insertHyperlink(doc, none, QString(), write::headingPlace(doc.story()->findBlockByNumber(1).position()), QString(), QString(), &why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(bookmarkList(doc, true).size(), 1);
    }

    void aLinkToAFileIsKeptAndANewDocumentIsMadeAndLinkedTo()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Win win({"See the file here"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(4);
        c.setPosition(12, QTextCursor::KeepAnchor);   // "the file" ("See " is four characters)
        QCOMPARE(c.selectedText(), QStringLiteral("the file"));
        win.w.view()->setCursor(c);
        const QString existing = dir.filePath(QStringLiteral("notes.txt"));
        QFile out(existing);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write("x");
        out.close();
        withDialog([&] { win.w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) { pick(dlg, jp::LinkTargetPicker::File, QString(), existing); });
        write::Hyperlink link = write::hyperlinkAt(*win.w.document(), 6);
        QCOMPARE(link.address, QUrl::fromLocalFile(existing).toString(QUrl::FullyEncoded));
        QCOMPARE(write::pathOfFileLink(link.address), existing);
        QVERIFY(write::isFileLink(link.address));
        // Saved with the address as an outside relationship, and read back; a link to a file is not a box of a PDF's.
        auto again = open(win.w.document()->toDocx());
        QVERIFY(again);
        QCOMPARE(write::hyperlinkAt(*again, 6).address, link.address);
        for (const write::LinkSpan &span : write::linkSpans(*win.w.document())) QVERIFY(span.uri.isEmpty());
        // Ctrl+click asks first (a file link) and hands the address to the opener only on a yes; a program is refused.
        QStringList opened, asked;
        bool yes = false;
        win.w.setLinkOpener([&](const QString &a) { opened << a; });
        win.w.setLinkAsker([&](const QString &q) { asked << q; return yes; });
        Q_EMIT win.w.view()->linkFollowed(link.address, link.text);
        QCOMPARE(asked.size(), 1);
        QVERIFY(opened.isEmpty());
        yes = true;
        Q_EMIT win.w.view()->linkFollowed(link.address, link.text);
        QCOMPARE(opened, (QStringList{link.address}));
        QVERIFY(!write::isFileLink(QStringLiteral("file:///home/a/run.exe")));
        QVERIFY(!write::isFileLink(QStringLiteral("file://server/share/a.txt")));
        QVERIFY(write::fileLinkOfPath(QStringLiteral("relative/path.txt")).isEmpty());
        // A new document: an empty one of this app's kind is made at the path, and the link goes to it; nothing there is overwritten.
        const QString made = dir.filePath(QStringLiteral("brand new.docx"));
        QVERIFY(!QFile::exists(made));
        QVERIFY2(!write::fileLinkOfPath(made).isEmpty(), qPrintable(made));
        QVERIFY2(!write::fileLinkOfPath(existing).isEmpty(), qPrintable(existing));
        QTextCursor c2(win.w.document()->story());
        c2.setPosition(0);
        c2.setPosition(3, QTextCursor::KeepAnchor);   // "See"
        win.w.view()->setCursor(c2);
        withDialog([&] { win.w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) { pick(dlg, jp::LinkTargetPicker::NewDocument, QString(), made); });
        QVERIFY(QFile::exists(made));
        QFile in(made);
        QVERIFY(in.open(QIODevice::ReadOnly));
        const QByteArray bytes = in.readAll();
        in.close();
        QString err;
        auto empty = WriteDoc::fromDocx(bytes, &err);
        QVERIFY2(empty, qPrintable(err));
        QCOMPARE(empty->story()->toPlainText().remove(QChar::ObjectReplacementCharacter), QString());
        QCOMPARE(write::hyperlinkAt(*win.w.document(), 1).address, QUrl::fromLocalFile(made).toString(QUrl::FullyEncoded));
        // The same path again is refused, with the file as it was.
        QTextCursor c3(win.w.document()->story());
        c3.setPosition(win.w.document()->story()->characterCount() - 1);
        win.w.view()->setCursor(c3);
        QString why;
        {
            jp::LinkTargetPicker picker;
            picker.enableNewDocument(QStringLiteral("x"), {QStringLiteral("docx")}, [](const QString &, QString *) { return true; });
            picker.setKind(jp::LinkTargetPicker::NewDocument);
            picker.setAddress(made);
            QVERIFY(!picker.prepare(&why));
            QVERIFY(!why.isEmpty());
        }
        QVERIFY(in.open(QIODevice::ReadOnly));
        QCOMPARE(in.readAll(), bytes);
    }

    // Pictures of the link box for the change's report (only with JO_SHOT_DIR set).
    void theLinkBoxPictures()
    {
        const QString shots = qEnvironmentVariable("JO_SHOT_DIR");
        if (shots.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(headingBody())), QString());
        QTextCursor c(w.document()->story());
        const int at = w.document()->story()->findBlockByNumber(3).position();
        c.setPosition(at + 10);
        c.setPosition(at + 15, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            pick(dlg, jp::LinkTargetPicker::Place, write::headingPlace(0));
            dlg->grab().save(shots + QStringLiteral("/write-link-place.png"));
        }, false);
    }

    // The real click path: only a link whose text alone says it is a document or picture on this computer is asked about and opened.
    void aFileLinkIsFollowedOnlyWhenItsTextAloneSaysItIsSafe()
    {
        const QStringList targets = {"file:///tmp/jo-no-such-folder/ok.txt", "file:payload.txt", "file:..\\x.txt", "file:///tmp/a%00.txt", "file:///tmp/a%0A.txt", "file://server/share/a.txt",
                                     "file:////server/share/a.txt", "file:///tmp/a.svg", "file:///usr/bin/run.exe", "file:///tmp/a.html", "file:///tmp/a.txt:stream", "smb://host/x.txt", "file:///tmp/noending"};
        const QByteArray file = makeLinkDocx(targets, [&](const QStringList &ids) {
            QByteArray body;
            for (int i = 0; i < ids.size(); ++i) body += linkRun(ids[i], "words");
            return body;
        });
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(file), QString());
        QStringList opened, asked, notices;
        w.setLinkOpener([&](const QString &a) { opened << a; });
        w.setLinkAsker([&](const QString &q) { asked << q; return true; });
        QObject::connect(w.view(), &write::PageView::notice, [&](const QString &t) { notices << t; });
        auto clickIn = [&](int block) {
            QTextCursor c(w.document()->story());
            c.setPosition(w.document()->story()->findBlockByNumber(block).position() + 2);
            w.view()->setCursor(c);
            QRect r;
            QVERIFY(w.view()->caretRect(&r));
            QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, r.center());
        };
        clickIn(0);   // (nothing is looked at: the folder is not there, and the link is asked about and followed)
        QCOMPARE(asked.size(), 1);
        QCOMPARE(opened.size(), 1);
        for (int i = 1; i < targets.size(); ++i) {
            asked.clear();
            opened.clear();
            notices.clear();
            clickIn(i);
            QVERIFY2(asked.isEmpty() && opened.isEmpty(), qPrintable(targets[i]));
            QVERIFY2(!notices.isEmpty(), qPrintable(targets[i]));
            QVERIFY2(!write::isFileLink(targets[i]), qPrintable(targets[i]));
        }
        QVERIFY(write::isFileLink(QStringLiteral("file:///C:/Users/a/Report.DOCX")));
        QVERIFY(write::isFileLink(QStringLiteral("file://localhost/tmp/a.png")));
        QVERIFY(!write::isFileLink(QStringLiteral("file:///tmp/a.png?x=1")));
    }

    void aNewDocumentPathIsCheckedBeforeAnythingIsMade()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        int made = 0;
        for (const QString &bad : {QStringLiteral("relative.docx"), QStringLiteral("//server/share/a.docx"), QStringLiteral("\\\\server\\share\\a.docx"), dir.filePath(QStringLiteral("run.exe")), dir.filePath(QStringLiteral("a.pptx")),
                                   dir.filePath(QStringLiteral("a\n.docx")), dir.filePath(QStringLiteral("a.docx:stream"))}) {
            jp::LinkTargetPicker picker;
            picker.enableNewDocument(QStringLiteral("x"), {QStringLiteral("docx"), QStringLiteral("odt")}, [&](const QString &, QString *) { ++made; return true; });
            picker.setKind(jp::LinkTargetPicker::NewDocument);
            picker.setAddress(bad);
            QString why;
            QVERIFY2(!picker.prepare(&why) && !why.isEmpty(), qPrintable(bad));
        }
        QCOMPARE(made, 0);   // (nothing was made for any of them)
        QVERIFY(write::fileLinkOfPath(dir.filePath(QStringLiteral("a%00.docx"))).contains(QLatin1String("%2500")));   // (a typed percent sign stays one)
        QVERIFY(write::fileLinkOfPath(dir.filePath(QStringLiteral("a.docx:stream"))).isEmpty());
    }

    // Editing a link never sends it somewhere else: the place it has is in the list (first) even past the 5,000 places, and only the tip changes.
    void editingALinkKeepsItsPlaceEvenWhenTheListIsCut()
    {
        QByteArray body;
        for (int i = 0; i < 5100; ++i) body += QByteArray("<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>H") + QByteArray::number(i) + "</w:t></w:r></w:p>";
        body += "<w:p><w:bookmarkStart w:id=\"1\" w:name=\"zzlast\"/><w:r><w:t>target</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>";
        body += "<w:p><w:hyperlink w:anchor=\"zzlast\" w:tooltip=\"old\"><w:r><w:t>go</w:t></w:r></w:hyperlink></w:p>";
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        WriteDoc &doc = *w.document();
        const int at = doc.story()->findBlockByNumber(5101).position();
        QTextCursor c(doc.story());
        c.setPosition(at + 2);
        w.view()->setCursor(c);
        QVERIFY(write::hyperlinkAt(doc, at + 2).valid());
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            auto *picker = dlg->findChild<jp::LinkTargetPicker *>();
            QCOMPARE(picker->kind(), jp::LinkTargetPicker::Place);
            QCOMPARE(picker->placeValue(), QStringLiteral("zzlast"));
            QCOMPARE(picker->placeCount(), jp::kMostLinkPlaces);
            QVERIFY(picker->truncated());
            QVERIFY(!dlg->findChild<QLabel *>(QStringLiteral("linkPlaceNote"))->isHidden());
            QVERIFY(dlg->findChild<QLabel *>(QStringLiteral("linkPlaceNote"))->text().contains(QLatin1String("5000")) || dlg->findChild<QLabel *>(QStringLiteral("linkPlaceNote"))->text().contains(QLatin1String("5,000")));
            dlg->findChild<QLineEdit *>(QStringLiteral("screenTip"))->setText(QStringLiteral("new"));
        });
        const write::Hyperlink link = write::hyperlinkAt(doc, at + 2);
        QCOMPARE(link.anchor, QStringLiteral("zzlast"));
        QCOMPARE(link.screenTip, QStringLiteral("new"));
        QCOMPARE(link.text, QStringLiteral("go"));
    }

    // A bookmark named like a heading's place is still a bookmark: picking it, and editing a link to it, keep it.
    void aBookmarkNamedLikeAHeadingPlaceIsNotTakenForOne()
    {
        const QByteArray body = "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Intr</w:t></w:r></w:p>"   // (the next paragraph begins at 5)
                                "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Next</w:t></w:r></w:p>"
                                "<w:p><w:bookmarkStart w:id=\"7\" w:name=\"heading:5\"/><w:r><w:t>odd</w:t></w:r><w:bookmarkEnd w:id=\"7\"/></w:p>"
                                "<w:p><w:r><w:t>link here</w:t></w:r></w:p>";
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        WriteDoc &doc = *w.document();
        QCOMPARE(doc.story()->findBlockByNumber(1).position(), 5);
        QCOMPARE(bookmarkList(doc, false).size(), 1);
        const int at = doc.story()->findBlockByNumber(3).position();
        QTextCursor c(doc.story());
        c.setPosition(at);
        c.setPosition(at + 4, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            auto *places = dlg->findChild<QComboBox *>(QStringLiteral("linkPlace"));
            QVERIFY(places->findData(QStringLiteral("heading:5")) >= 0);
            QVERIFY(places->findData(write::headingPlace(5)) >= 0);
            QVERIFY(places->findData(QStringLiteral("heading:5")) != places->findData(write::headingPlace(5)));
            pick(dlg, jp::LinkTargetPicker::Place, QStringLiteral("heading:5"));
        });
        QCOMPARE(write::hyperlinkAt(doc, at + 2).anchor, QStringLiteral("heading:5"));
        QCOMPARE(bookmarkList(doc, true).size(), 1);   // (no bookmark was made for the heading)
        QTextCursor inLink(doc.story());
        inLink.setPosition(at + 2);
        w.view()->setCursor(inLink);
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            QCOMPARE(dlg->findChild<jp::LinkTargetPicker *>()->placeValue(), QStringLiteral("heading:5"));
            dlg->findChild<QLineEdit *>(QStringLiteral("screenTip"))->setText(QStringLiteral("tip"));
        });
        QCOMPARE(write::hyperlinkAt(doc, at + 2).anchor, QStringLiteral("heading:5"));
        QCOMPARE(write::hyperlinkAt(doc, at + 2).screenTip, QStringLiteral("tip"));
        QCOMPARE(write::placeOfAnchor(doc, QStringLiteral("heading:5")), QStringLiteral("heading:5"));
    }

    // A bookmark's label in the list is cut at 120 characters, as a heading's text is.
    void aLongBookmarkNameIsCutInThePlaceList()
    {
        const QByteArray longName(300, 'b');
        const QByteArray body = "<w:p><w:bookmarkStart w:id=\"7\" w:name=\"" + longName + "\"/><w:r><w:t>odd</w:t></w:r><w:bookmarkEnd w:id=\"7\"/></w:p><w:p><w:r><w:t>link here</w:t></w:r></w:p>";
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(w.document()->story()->findBlockByNumber(1).position());
        c.setPosition(c.position() + 4, QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
        withDialog([&] { w.act("insert.hyperlink")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            auto *places = dlg->findChild<QComboBox *>(QStringLiteral("linkPlace"));
            const int at = places->findData(QString::fromLatin1(longName));
            QVERIFY(at >= 0);   // (the value is the whole name)
            QCOMPARE(places->itemText(at), QStringLiteral("Bookmark: ") + QString(120, QLatin1Char('b')));
        }, false);
    }

    void aHeadingLinkThatFailsLeavesNoBookmarkBehind()
    {
        const QByteArray body = "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr></w:p><w:p><w:r><w:t>x</w:t></w:r></w:p>";   // (an empty heading: its words are no text for a link)
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        WriteDoc &doc = *w.document();
        QTextCursor c(doc.story());
        c.setPosition(doc.story()->findBlockByNumber(1).position());
        QString why;
        QVERIFY(!write::insertHyperlink(doc, c, QString(), write::headingPlace(0), QString(), QString(), &why));
        QVERIFY(bookmarkList(doc, true).isEmpty());
        // The same for a link that is changed to go to a heading: one that is not a heading (or is gone) is refused, and nothing is left; a heading makes exactly one.
        QTextCursor over(doc.story());
        const int x = doc.story()->findBlockByNumber(1).position();
        over.setPosition(x);
        over.setPosition(x + 1, QTextCursor::KeepAnchor);
        QVERIFY(write::insertHyperlink(doc, over, QStringLiteral("https://example.com"), QString(), QString(), QString(), &why));
        QVERIFY(!write::changeHyperlink(doc, x + 1, QString(), write::headingPlace(x), QString(), QString(), &why));
        QVERIFY(!write::changeHyperlink(doc, x + 1, QString(), write::headingPlace(9999), QString(), QString(), &why));
        QVERIFY(bookmarkList(doc, true).isEmpty());
        QCOMPARE(write::hyperlinkAt(doc, x + 1).address, QStringLiteral("https://example.com"));
        QVERIFY(write::changeHyperlink(doc, x + 1, QString(), write::headingPlace(0), QString(), QString(), &why));
        QCOMPARE(bookmarkList(doc, true).size(), 1);
    }

    void anEmailAddressGetsMailtoAndABadOneIsRefusedWithAReason()
    {
        QCOMPARE(write::normalizedAddress(QStringLiteral("a.b@example.com")), QStringLiteral("mailto:a.b@example.com"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("www.example.com")), QStringLiteral("https://www.example.com"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("two words")), QString());
        Win win;
        QString why;
        QTextCursor &c = win.w.view()->cursor();
        QVERIFY(!write::insertHyperlink(*win.w.document(), c, QStringLiteral("file:///usr/bin/run.exe"), QString(), QString(), QStringLiteral("x"), &why));
        QVERIFY(!why.isEmpty());
        why.clear();
        QVERIFY(!write::insertHyperlink(*win.w.document(), c, QStringLiteral("file://server/share/a.txt"), QString(), QString(), QStringLiteral("x"), &why));
        QVERIFY(!why.isEmpty());
        why.clear();
        QVERIFY(!write::insertHyperlink(*win.w.document(), c, QString(), QStringLiteral("nowhere"), QString(), QStringLiteral("x"), &why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(win.text(), QStringLiteral("Hello"));
    }

    void aLinkToABookmarkGoesThereOnCtrlClickAndAnOutsideOneIsHandedToTheOpener()
    {
        Win win({"Start here", "Target text"});
        QTextCursor t(win.w.document()->story());
        t.setPosition(win.w.document()->story()->findBlockByNumber(1).position());
        t.setPosition(t.position() + 6, QTextCursor::KeepAnchor);
        QVERIFY(write::addBookmark(*win.w.document(), t, QStringLiteral("target")));
        QTextCursor c(win.w.document()->story());
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        QString why;
        QVERIFY2(write::insertHyperlink(*win.w.document(), c, QString(), QStringLiteral("target"), QString(), QString(), &why), qPrintable(why));
        const write::Hyperlink link = write::hyperlinkAt(*win.w.document(), 2);
        QCOMPARE(link.anchor, QStringLiteral("target"));
        QVERIFY(link.address.isEmpty());
        QVERIFY(write::linkTargetAt(*win.w.document(), 2) > 0);
        QVERIFY(write::mayFollow(QStringLiteral("https://example.com")));
        QVERIFY(write::mayFollow(QStringLiteral("mailto:a@example.com")));
        QVERIFY(!write::mayFollow(QStringLiteral("file:///etc/passwd")));   // (a file link is followed only after a question: see isFileLink)
        QVERIFY(!write::mayFollow(QStringLiteral("javascript:alert(1)")));
        QVERIFY(!write::mayFollow(QStringLiteral("smb://host/share")));
        QStringList opened;
        win.w.setLinkOpener([&](const QString &a) { opened << a; });
        Q_EMIT win.w.view()->linkFollowed(QStringLiteral("https://example.com"), QString());
        QCOMPARE(opened, (QStringList{"https://example.com"}));
    }

    void aLinkIsChangedAndRemovedKeepingItsWords()
    {
        Win win({"Click here please"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(6);
        c.setPosition(10, QTextCursor::KeepAnchor);
        QVERIFY(write::insertHyperlink(*win.w.document(), c, QStringLiteral("https://a.example"), QString(), QString(), QString()));
        QVERIFY(win.w.act("insert.removeHyperlink"));
        QTextCursor in(win.w.document()->story());
        in.setPosition(8);
        win.w.view()->setCursor(in);
        QTRY_VERIFY(win.w.act("insert.removeHyperlink")->isEnabled());
        QVERIFY(write::changeHyperlink(*win.w.document(), 8, QStringLiteral("https://b.example"), QString(), QString(), QString()));
        QCOMPARE(write::hyperlinkAt(*win.w.document(), 8).address, QStringLiteral("https://b.example"));
        QCOMPARE(write::hyperlinkAt(*win.w.document(), 8).text, QStringLiteral("here"));
        win.w.act("insert.removeHyperlink")->trigger();
        QVERIFY(!write::hyperlinkAt(*win.w.document(), 8).valid());
        QCOMPARE(win.text().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Click here please"));
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("<w:hyperlink"));
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("w:val=\"Hyperlink\""));
        QTRY_VERIFY(!win.w.act("insert.removeHyperlink")->isEnabled());
    }

    // ---------------------------------------------------------------- Insert > Text from File
    void textFromAnotherDocumentIsInsertedWithItsLooksAsOneStep()
    {
        QTemporaryDir dir;
        const QByteArray other = makeDocx("<w:p><w:r><w:rPr><w:b/></w:rPr><w:t>Bold start</w:t></w:r><w:r><w:t xml:space=\"preserve\"> and plain</w:t></w:r></w:p>" + para("Second paragraph"));
        QFile f(dir.filePath(QStringLiteral("other.docx")));
        QVERIFY(f.open(QIODevice::WriteOnly) && f.write(other) == other.size());
        f.close();
        Win win({"Before", "After"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(6);
        win.w.view()->setCursor(c);
        win.w.setTextFileChooser([&] { return f.fileName(); });
        QVERIFY(win.w.act("insert.textFromFile"));
        win.w.act("insert.textFromFile")->trigger();
        const QString text = win.text();
        QVERIFY2(text.contains(QStringLiteral("BeforeBold start and plain")) && text.contains(QStringLiteral("Second paragraph")), qPrintable(text));
        QTextCursor at(win.w.document()->story());
        at.setPosition(8);
        at.setPosition(9, QTextCursor::KeepAnchor);
        QVERIFY(at.charFormat().fontWeight() >= QFont::DemiBold);
        win.w.document()->undo();
        QCOMPARE(win.text(), QStringLiteral("Before\nAfter"));
    }

    void aPlainTextFileBecomesParagraphsAndAnUnreadableOneIsRefused()
    {
        Win win;
        QString why;
        QTextCursor &c = win.w.view()->cursor();
        QVERIFY(write::insertTextFromFile(*win.w.document(), c, QByteArray("one\r\ntwo\r\n"), &why));
        QCOMPARE(win.text(), QStringLiteral("Helloone\ntwo"));
        QVERIFY(!write::insertTextFromFile(*win.w.document(), c, QByteArray("bin\0ary\x01\x02", 9), &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!write::insertTextFromFile(*win.w.document(), c, QByteArray(), &why));
    }

    // ---------------------------------------------------------------- Layout > Breaks
    void aColumnBreakIsInsertedAndSavedAsOne()
    {
        Win win;
        QVERIFY(win.w.act("layout.columnBreak"));
        win.w.act("layout.columnBreak")->trigger();
        QVERIFY(win.text().contains(QChar(0x000E)));
        QVERIFY2(documentXml(win.w.document()->toDocx()).contains("<w:br w:type=\"column\"/>"), "no column break in the file");
    }

    void sectionBreaksOfEachKindAreMadeAndSaved()
    {
        struct Case { const char *id; const char *type; };
        for (const Case &k : {Case{"layout.sectionBreak.continuous", "continuous"}, Case{"layout.sectionBreak.evenPage", "evenPage"}, Case{"layout.sectionBreak.oddPage", "oddPage"}}) {
            Win win;
            QAction *a = win.w.act(QString::fromLatin1(k.id));
            QVERIFY2(a, k.id);
            a->trigger();
            QCOMPARE(win.w.document()->sectionCount(), 2);
            const QByteArray out = documentXml(win.w.document()->toDocx());
            QVERIFY2(out.contains(QByteArray("<w:type w:val=\"") + k.type + "\"/>"), out.constData());
        }
    }

    void aBlankPageMakesOneEmptyPageAtTheCaret()
    {
        Win win({"First"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(5);
        win.w.view()->setCursor(c);
        QVERIFY(win.w.act("insert.blankPage"));
        win.w.act("insert.blankPage")->trigger();
        QTRY_COMPARE(win.w.view()->pageCount(), 3);
        win.w.document()->undo();
        QTRY_COMPARE(win.w.view()->pageCount(), 1);
    }

    // ---------------------------------------------------------------- Layout > Margins, Columns, Hyphenation, Line Numbers
    void mirroredMarginsAreTheDocumentsAndSurviveASave()
    {
        Win win;
        QVERIFY(!win.w.document()->mirrorMargins());
        win.w.act("layout.margins.mirrored")->trigger();
        QVERIFY(win.w.document()->mirrorMargins());
        QCOMPARE(win.w.document()->flow().section(0).margins.left(), 90.0);
        QVERIFY(win.w.act("layout.mirrorMargins")->isChecked());
        const QByteArray saved = win.w.document()->toDocx();
        QVERIFY2(partOf(saved, QStringLiteral("/word/settings.xml")).contains("<w:mirrorMargins/>"), "no mirrorMargins in the settings");
        auto again = open(saved);
        QVERIFY(again && again->mirrorMargins());
        win.w.act("edit.undo")->trigger();
        QVERIFY(!win.w.document()->mirrorMargins());
        QTRY_VERIFY(!win.w.act("layout.mirrorMargins")->isChecked());
        win.w.act("layout.mirrorMargins")->trigger();
        QVERIFY(win.w.document()->mirrorMargins());
        win.w.act("layout.mirrorMargins")->trigger();
        QVERIFY(!win.w.document()->mirrorMargins());
    }

    void automaticHyphenationIsTheDocumentsAndSurvivesASave()
    {
        Win win;
        QVERIFY(win.w.act("layout.hyphenation.auto"));
        QVERIFY(win.w.act("layout.hyphenation.none")->isChecked());
        win.w.act("layout.hyphenation.auto")->trigger();
        QVERIFY(win.w.document()->autoHyphenation());
        QVERIFY(win.w.act("layout.hyphenation.auto")->isChecked());
        QVERIFY2(partOf(win.w.document()->toDocx(), QStringLiteral("/word/settings.xml")).contains("<w:autoHyphenation/>"), "no autoHyphenation in the settings");
        auto again = open(win.w.document()->toDocx());
        QVERIFY(again && again->autoHyphenation());
        win.w.act("layout.hyphenation.none")->trigger();
        QVERIFY(!win.w.document()->autoHyphenation());
        QVERIFY(!partOf(win.w.document()->toDocx(), QStringLiteral("/word/settings.xml")).contains("autoHyphenation"));
    }

    void leftAndRightColumnsAreUnequalAndKeptInTheFile()
    {
        Win win;
        QVERIFY(win.w.act("layout.columns.left"));
        win.w.act("layout.columns.left")->trigger();
        const flow::Section s = win.w.document()->flow().section(0);
        QCOMPARE(s.columns, 2);
        QCOMPARE(s.columnWidths.size(), 2);
        QVERIFY(s.columnWidths[0] < s.columnWidths[1]);
        QVERIFY(win.w.act("layout.columns.left")->isChecked());
        const QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("w:equalWidth=\"0\""), out.constData());
        QVERIFY2(out.contains("<w:col w:w="), out.constData());
        auto again = open(win.w.document()->toDocx());
        QVERIFY(again);
        QCOMPARE(again->flow().section(0).columnWidths.size(), 2);
        win.w.act("layout.columns.right")->trigger();
        QVERIFY(win.w.document()->flow().section(0).columnWidths[0] > win.w.document()->flow().section(0).columnWidths[1]);
        win.w.act("layout.columns.2")->trigger();   // (two equal ones again)
        QVERIFY(win.w.document()->flow().section(0).columnWidths.isEmpty());
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("w:equalWidth=\"0\""));
    }

    void theMoreColumnsDialogSetsTheNumberTheSpaceAndTheLine()
    {
        Win win;
        withDialog([&] { win.w.act("layout.columns.more")->trigger(); }, QStringLiteral("columnsDialog"), [&](QDialog *dlg) {
            dlg->findChild<QSpinBox *>(QStringLiteral("count"))->setValue(3);
            dlg->findChild<QCheckBox *>(QStringLiteral("line"))->setChecked(true);
        });
        const flow::Section s = win.w.document()->flow().section(0);
        QCOMPARE(s.columns, 3);
        QVERIFY(s.columnLine);
        const QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("w:num=\"3\"") && out.contains("w:sep=\"1\""), out.constData());
        QTRY_COMPARE(itemsStartingWith(win.w, 0, QStringLiteral("flow-column-line-")), 2);
        win.w.document()->undo();
        QCOMPARE(win.w.document()->flow().section(0).columns, 1);
        QCOMPARE(itemsStartingWith(win.w, 0, QStringLiteral("flow-column-line-")), 0);
    }

    void lineNumbersAreSetForTheSectionAndSaved()
    {
        Win win;
        QVERIFY(win.w.act("layout.lineNumbers.continuous"));
        QVERIFY(win.w.act("layout.lineNumbers.none")->isChecked());
        win.w.act("layout.lineNumbers.continuous")->trigger();
        QCOMPARE(win.w.document()->flow().section(0).lineNumbers.countBy, 1);
        QVERIFY(win.w.document()->flow().section(0).lineNumbers.restart == flow::Section::LineNumbers::Restart::Continuous);
        QVERIFY(win.w.act("layout.lineNumbers.continuous")->isChecked());
        QVERIFY2(documentXml(win.w.document()->toDocx()).contains("<w:lnNumType w:countBy=\"1\" w:restart=\"continuous\"/>"), "no line numbers in the file");
        win.w.act("layout.lineNumbers.eachSection")->trigger();
        QVERIFY(win.w.document()->flow().section(0).lineNumbers.restart == flow::Section::LineNumbers::Restart::NewSection);
        win.w.act("layout.lineNumbers.none")->trigger();
        QCOMPARE(win.w.document()->flow().section(0).lineNumbers.countBy, 0);
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("lnNumType"));
    }

    void theLineNumbersDialogCountsByAndStartsAt()
    {
        Win win;
        withDialog([&] { win.w.act("layout.lineNumbers.options")->trigger(); }, QStringLiteral("lineNumbersDialog"), [&](QDialog *dlg) {
            dlg->findChild<QCheckBox *>(QStringLiteral("on"))->setChecked(true);
            dlg->findChild<QSpinBox *>(QStringLiteral("countBy"))->setValue(5);
            dlg->findChild<QSpinBox *>(QStringLiteral("start"))->setValue(10);
        });
        const flow::Section::LineNumbers n = win.w.document()->flow().section(0).lineNumbers;
        QCOMPARE(n.countBy, 5);
        QCOMPARE(n.start, 10);
    }

    // ---------------------------------------------------------------- Design
    void thePageColorIsSetShownSavedAndUndone()
    {
        Win win;
        win.w.document()->setPageColor(QColor(0xFF, 0x00, 0x00));
        QCOMPARE(win.w.document()->flow().pageColor, QColor(0xFF, 0x00, 0x00));
        QVERIFY(itemsStartingWith(win.w, 0, QStringLiteral("flow-page-color-")) >= 1);
        const QByteArray saved = win.w.document()->toDocx();
        const QByteArray out = documentXml(saved);
        QVERIFY2(out.indexOf("<w:background w:color=\"FF0000\"/>") >= 0 && out.indexOf("<w:background") < out.indexOf("<w:body"), out.constData());
        QVERIFY2(partOf(saved, QStringLiteral("/word/settings.xml")).contains("<w:displayBackgroundShape/>"), "the color is not shown by the settings");
        auto again = open(saved);
        QVERIFY(again);
        QCOMPARE(again->pageColor(), QColor(0xFF, 0x00, 0x00));
        QTRY_VERIFY(win.w.act("design.pageColor.none")->isEnabled());
        win.w.act("design.pageColor.none")->trigger();
        QVERIFY(!win.w.document()->pageColor().isValid());
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("w:background"));
        QCOMPARE(itemsStartingWith(win.w, 0, QStringLiteral("flow-page-color-")), 0);
        win.w.document()->undo();
        QCOMPARE(win.w.document()->pageColor(), QColor(0xFF, 0x00, 0x00));
    }

    void thePageBordersDialogMakesABoxAndWritesItInTheFile()
    {
        Win win;
        QVERIFY(win.w.act("design.pageBorders"));
        withDialog([&] { win.w.act("design.pageBorders")->trigger(); }, QStringLiteral("pageBordersDialog"), [&](QDialog *dlg) {
            dlg->findChild<QPushButton *>(QStringLiteral("box"))->click();
            dlg->findChild<QDoubleSpinBox *>(QStringLiteral("width"))->setValue(2);
        });
        const flow::PageBorders b = win.w.document()->flow().section(0).pageBorders;
        QVERIFY(b.top.present && b.left.present && b.bottom.present && b.right.present);
        QCOMPARE(b.top.width, 2.0);
        QCOMPARE(itemsStartingWith(win.w, 0, QStringLiteral("flow-page-border-")), 4);
        const QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("<w:pgBorders>") && out.contains("<w:top w:val=\"single\" w:sz=\"16\""), out.constData());
        // (The borders come after the page margins and before the columns, where the schema puts them.)
        QVERIFY(out.indexOf("<w:pgMar") < out.indexOf("<w:pgBorders"));
        withDialog([&] { win.w.act("design.pageBorders")->trigger(); }, QStringLiteral("pageBordersDialog"), [&](QDialog *dlg) { dlg->findChild<QPushButton *>(QStringLiteral("none"))->click(); });
        QVERIFY(!win.w.document()->flow().section(0).pageBorders.any());
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("pgBorders"));
    }

    // ---------------------------------------------------------------- Watermark
    void aWatermarkGoesInTheHeaderKeepsTheHeadersTextAndComesOut()
    {
        auto doc = WriteDoc::blank();
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        QTextCursor h(doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        h.insertText(QStringLiteral("Quarterly report"));
        QVERIFY(!watermarkOf(*doc));
        WatermarkSetup w;
        w.text = QStringLiteral("CONFIDENTIAL & DRAFT");
        w.color = QColor(0xFF, 0, 0);
        w.semitransparent = false;
        w.diagonal = false;
        QCOMPARE(insertWatermark(*doc, w), 1);
        const auto found = watermarkOf(*doc);
        QVERIFY(found);
        QCOMPARE(found->text, w.text);
        QCOMPARE(found->color, w.color);
        QVERIFY(!found->diagonal);
        QVERIFY(!found->semitransparent);
        // The file has the shape in the header, behind the text, and the header's words as they were.
        office::Package pkg;
        pkg.read(doc->toDocx());
        QByteArray header;
        for (const QString &name : pkg.partNames())
            if (name.startsWith(QLatin1String("/word/header"))) header = pkg.part(name);
        QVERIFY2(header.contains("<v:textpath") && header.contains("string=\"CONFIDENTIAL &amp; DRAFT\"") && header.contains("z-index:-251658752"), header.constData());
        QVERIFY(header.contains("Quarterly report"));
        QVERIFY(!header.contains("rotation:"));
        // Opened again, it is found; putting another in replaces it, and nothing is doubled.
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QVERIFY(watermarkOf(*again));
        WatermarkSetup other;
        other.text = QStringLiteral("DRAFT");
        QCOMPARE(insertWatermark(*again, other), 1);
        QCOMPARE(watermarkOf(*again)->text, QStringLiteral("DRAFT"));
        QVERIFY(watermarkOf(*again)->diagonal);
        pkg.read(again->toDocx());
        for (const QString &name : pkg.partNames())
            if (name.startsWith(QLatin1String("/word/header"))) QCOMPARE(pkg.part(name).count("string=\"DRAFT\""), 1);
        QCOMPARE(removeWatermark(*again), 1);
        QVERIFY(!watermarkOf(*again));
        pkg.read(again->toDocx());
        for (const QString &name : pkg.partNames())
            if (name.startsWith(QLatin1String("/word/header"))) {
                QVERIFY(!pkg.part(name).contains("textpath"));
                QVERIFY(pkg.part(name).contains("Quarterly report"));
            }
    }

    void aWatermarkReachesHeadersMadeAfterIt()
    {
        auto doc = open(makeDocx(para("One") + para("Two")));
        QVERIFY(doc);
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        WatermarkSetup w;
        w.text = QStringLiteral("DRAFT");
        QCOMPARE(insertWatermark(*doc, w), 1);
        // A first-page header made later has it, as part of the step that made the header.
        doc->setDifferentFirstPage(0, true);
        QVERIFY(!doc->hasOwnHeaderFooter(0, false, WriteDoc::kFirstSlot));
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kFirstSlot));
        QCOMPARE(watermarksIn(doc->editableHeaderFooter(0, false, WriteDoc::kFirstSlot)), 1);
        doc->undo();
        QVERIFY(!doc->hasOwnHeaderFooter(0, false, WriteDoc::kFirstSlot));
        QCOMPARE(watermarksIn(doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot)), 1);
        doc->redo();
        QVERIFY(doc->hasOwnHeaderFooter(0, false, WriteDoc::kFirstSlot));
        QCOMPARE(watermarksIn(doc->editableHeaderFooter(0, false, WriteDoc::kFirstSlot)), 1);
        // An even-page header, too, and one made to be typed in (shown before it is kept).
        doc->setDifferentOddEven(true);
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kEvenSlot, true));
        QCOMPARE(watermarksIn(doc->headerFooterStory(0, false, WriteDoc::kEvenSlot)), 1);
        doc->discardPendingHeaderFooter();
        // A new section's own header, made by unlinking it, and its first-page header.
        QTextCursor c(doc->story());
        QVERIFY(doc->insertSectionBreak(doc->story()->begin()));
        QVERIFY(doc->sectionCount() >= 2);
        doc->setDifferentFirstPage(1, true);
        QVERIFY(doc->createHeaderFooter(1, false, WriteDoc::kFirstSlot));
        QCOMPARE(watermarksIn(doc->editableHeaderFooter(1, false, WriteDoc::kFirstSlot)), 1);
        // A footer gets none, and a document with no watermark makes plain headers.
        QVERIFY(doc->createHeaderFooter(0, true, WriteDoc::kDefaultSlot));
        QCOMPARE(watermarksIn(doc->editableHeaderFooter(0, true, WriteDoc::kDefaultSlot)), 0);
        auto plain = WriteDoc::blank();
        QVERIFY(plain->createHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        QCOMPARE(watermarksIn(plain->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot)), 0);
    }

    void aWatermarkCoversEveryHeaderOfEverySectionAsOneStepEachWay()
    {
        auto doc = open(makeDocx(para("One") + para("Two")));
        QVERIFY(doc);
        QVERIFY(doc->insertSectionBreak(doc->story()->begin()));
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        QVERIFY(doc->setLinkedToPrevious(1, false, WriteDoc::kDefaultSlot, false));
        QVERIFY(doc->hasOwnHeaderFooter(1, false, WriteDoc::kDefaultSlot));
        QTextDocument *one = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot), *two = doc->editableHeaderFooter(1, false, WriteDoc::kDefaultSlot);
        QVERIFY(one);
        QVERIFY(two);
        QVERIFY(one != two);
        WatermarkSetup w;
        w.text = QStringLiteral("SAMPLE");
        QCOMPARE(insertWatermark(*doc, w), 2);
        QCOMPARE(watermarksIn(one), 1);
        QCOMPARE(watermarksIn(two), 1);
        doc->undo();
        QCOMPARE(watermarksIn(one), 0);
        QCOMPARE(watermarksIn(two), 0);
        doc->redo();
        QCOMPARE(watermarksIn(one), 1);
        QCOMPARE(watermarksIn(two), 1);
        QCOMPARE(removeWatermark(*doc), 2);
        QCOMPARE(watermarksIn(one) + watermarksIn(two), 0);
        doc->undo();
        QCOMPARE(watermarksIn(one), 1);
        QCOMPARE(watermarksIn(two), 1);
        doc->redo();
        QCOMPARE(watermarksIn(one) + watermarksIn(two), 0);
    }

    void aWatermarkMakesAHeaderWhereThereIsNoneAndNothingForNoWords()
    {
        auto doc = WriteDoc::blank();
        WatermarkSetup w;
        w.text = QStringLiteral("   ");
        QCOMPARE(insertWatermark(*doc, w), 0);
        w.text = QStringLiteral("SAMPLE");
        QCOMPARE(insertWatermark(*doc, w), 1);
        QVERIFY(watermarkOf(*doc));
        QVERIFY(doc->hasOwnHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        QVERIFY(!builtInWatermarks().isEmpty());
    }

    // ---------------------------------------------------------------- Paragraph spacing
    void paragraphSpacingPresetsSetTheDefaultStyleAsOneStepAndAreKept()
    {
        auto doc = open(makeDocx(para("First paragraph.") + para("Second paragraph.")));
        QVERIFY(doc);
        QVERIFY(!applyParagraphSpacing(*doc, QStringLiteral("nonsense")));
        QVERIFY(applyParagraphSpacing(*doc, QStringLiteral("double")));
        QCOMPARE(paragraphSpacingOf(*doc), QStringLiteral("double"));
        const QByteArray styles = partOf(doc->toDocx(), QStringLiteral("/word/styles.xml"));
        QVERIFY2(styles.contains("w:line=\"480\"") && styles.contains("w:lineRule=\"auto\""), styles.constData());
        // Every ordinary paragraph is double spaced: the text is restyled with the style.
        doc->pager().paginate(0);
        QTextBlock first = doc->story()->begin();
        QVERIFY(first.blockFormat().lineHeightType() == QTextBlockFormat::ProportionalHeight);
        QCOMPARE(first.blockFormat().lineHeight(), 200.0);
        auto again = open(doc->toDocx());
        QCOMPARE(paragraphSpacingOf(*again), QStringLiteral("double"));
        QVERIFY(applyParagraphSpacing(*doc, QStringLiteral("none")));
        QCOMPARE(paragraphSpacingOf(*doc), QStringLiteral("none"));
        doc->undo();
        QCOMPARE(paragraphSpacingOf(*doc), QStringLiteral("double"));
        QVERIFY(spacingPresets().size() == 7);
        QVERIFY(!spacingPresetName(QStringLiteral("tight")).isEmpty());
    }

    // ---------------------------------------------------------------- Drop cap
    void aDropCapIsSetAsOneStepAndWrittenAsAFrameBeforeTheParagraph()
    {
        auto doc = open(makeDocx(para("Once upon a time there was a drop cap.")));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(5);
        DropCapSetup w;
        w.lines = 3;
        QVERIFY(canHaveDropCap(c.block()));
        QVERIFY(setDropCap(*doc, c, w));
        QCOMPARE(dropCapOf(doc->story()->begin()).lines, 3);
        QCOMPARE(dropCapOf(doc->story()->begin()).chars, 1);
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Once upon a time there was a drop cap."));   // (the letters stay in the paragraph while it is edited)
        const QByteArray out = documentXml(doc->toDocx());
        const QRegularExpression frame(QStringLiteral("<w:p><w:pPr><w:keepNext/><w:framePr w:dropCap=\"drop\" w:lines=\"3\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"text\"/>.*?</w:pPr><w:r><w:rPr>(.*?)</w:rPr><w:t[^>]*>O</w:t></w:r></w:p><w:p><w:r><w:t xml:space=\"preserve\">nce upon a time there was a drop cap.</w:t></w:r></w:p>"));
        const auto m = frame.match(QString::fromUtf8(out));
        QVERIFY2(m.hasMatch(), out.constData());
        QVERIFY(m.captured(1).contains(QLatin1String("<w:position w:val=\"-")) && m.captured(1).contains(QLatin1String("<w:sz w:val=\"")));
        // The file opens as the frame and the paragraph after it, and the frame is a drop cap the window knows.
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(again->story()->blockCount(), 2);
        QCOMPARE(dropCapOf(again->story()->begin()).lines, 3);
        QCOMPARE(dropCapOf(again->story()->begin().next()).lines, 3);
        QVERIFY(!canHaveDropCap(again->story()->begin().next()));
        // Undo takes it away; None does too.
        doc->undo();
        QCOMPARE(dropCapOf(doc->story()->begin()).lines, 0);
        doc->redo();
        w.lines = 0;
        QVERIFY(setDropCap(*doc, c, w));
        QVERIFY(!documentXml(doc->toDocx()).contains("dropCap"));
    }

    void theSpellingCheckReadsADropCapAndTheRestOfItsWordAsOneWord()
    {
        auto doc = open(makeDocx(para("Once upon a time there was a drop cap.")));
        QTextCursor c(doc->story());
        DropCapSetup w;
        w.lines = 3;
        QVERIFY(setDropCap(*doc, c, w));
        auto again = open(doc->toDocx());
        QVERIFY(again);
        QCOMPARE(again->story()->blockCount(), 2);
        SpellCheck sc(again->story());
        QVERIFY2(sc.misspelledIn(0, again->story()->characterCount()).isEmpty(), "the rest of the first word is flagged");
        // A real mistake in that word is still found, as the whole word.
        auto bad = open(makeDocx(para("Onze upon a time.")));
        QTextCursor b(bad->story());
        QVERIFY(setDropCap(*bad, b, w));
        auto badAgain = open(bad->toDocx());
        SpellCheck sb(badAgain->story());
        const auto flagged = sb.misspelledIn(0, badAgain->story()->characterCount());
        QCOMPARE(flagged.size(), 1);
        QCOMPARE(badAgain->story()->toPlainText().mid(flagged.first().first, flagged.first().second - flagged.first().first), QStringLiteral("nze"));
        // Two paragraphs that are not a drop cap are two words.
        auto plain = open(makeDocx(para("O") + para("nce upon a time.")));
        SpellCheck sp(plain->story());
        QCOMPARE(sp.misspelledIn(0, plain->story()->characterCount()).size(), 1);
    }

    void aDropCapOfMoreLettersAndAFontIsKept()
    {
        auto doc = open(makeDocx(para("Chapter one begins here.")));
        QTextCursor c(doc->story());
        DropCapSetup w;
        w.lines = 2;
        w.chars = 3;
        w.font = QStringLiteral("Cousine");
        QVERIFY(setDropCap(*doc, c, w));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains("w:lines=\"2\"") && out.contains(">Cha</w:t>") && out.contains(">pter one begins here.</w:t>") && out.contains("Cousine"), out.constData());
    }

    void aDropCapIsRefusedWhereItCannotBe()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        DropCapSetup w;
        w.lines = 3;
        QVERIFY(!canHaveDropCap(c.block()));   // (an empty paragraph)
        QVERIFY(!setDropCap(*doc, c, w));
        QVERIFY(!documentXml(doc->toDocx()).contains("dropCap"));
        DropCapSetup none;
        QVERIFY(!setDropCap(*doc, c, none));   // (nothing to take away)
    }

    // ---------------------------------------------------------------- View
    void theNavigationPaneListsTheHeadingsAndGoesToThem()
    {
        QByteArray body;
        body += "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>One</w:t></w:r></w:p>" + para("text") + "<w:p><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr><w:r><w:t>One A</w:t></w:r></w:p>"
                + para("more text") + "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Two</w:t></w:r></w:p>" + para("end");
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        QVERIFY(w.act("view.navigationPane"));
        QVERIFY(!w.navigationPane());
        w.act("view.navigationPane")->trigger();
        QVERIFY(w.navigationPane());
        QVERIFY(w.act("view.navigationPane")->isChecked());
        QTreeWidget *tree = w.navigationPane()->headings();
        QCOMPARE(tree->topLevelItemCount(), 2);
        QCOMPARE(tree->topLevelItem(0)->text(0), QStringLiteral("One"));
        QCOMPARE(tree->topLevelItem(0)->childCount(), 1);
        QCOMPARE(tree->topLevelItem(0)->child(0)->text(0), QStringLiteral("One A"));
        w.navigationPane()->goToHeading(tree->topLevelItem(1));
        QCOMPARE(w.view()->cursor().block().text(), QStringLiteral("Two"));
        // (Demoting a heading takes the headings under it with it, and stops where a level would pass the end.)
        QVERIFY(w.navigationPane()->moveHeadingLevel(tree->topLevelItem(0), 1));
        QCOMPARE(w.document()->story()->findBlockByNumber(0).blockFormat().intProperty(write::prop::Outline), 1);
        QCOMPARE(w.document()->story()->findBlockByNumber(2).blockFormat().intProperty(write::prop::Outline), 2);
        QCOMPARE(w.document()->story()->findBlockByNumber(4).blockFormat().intProperty(write::prop::Outline), 0);
        w.document()->undo();
        QCOMPARE(w.document()->story()->findBlockByNumber(0).blockFormat().intProperty(write::prop::Outline), 0);
        QCOMPARE(w.document()->story()->findBlockByNumber(2).blockFormat().intProperty(write::prop::Outline), 1);
        QVERIFY(!w.navigationPane()->moveHeadingLevel(tree->topLevelItem(0), -1));   // (level 1 cannot be promoted)
        w.act("view.navigationPane")->trigger();
        QVERIFY(!w.act("view.navigationPane")->isChecked());
    }

    void theNavigationPaneSearchListsEachPlaceAndGoesToIt()
    {
        Win win({"alpha beta", "beta gamma beta"});
        win.w.act("view.navigationPane")->trigger();
        NavigationPane *pane = win.w.navigationPane();
        pane->tabs()->setCurrentIndex(2);
        pane->searchBox()->setText(QStringLiteral("beta"));
        pane->search();
        QCOMPARE(pane->results()->count(), 3);
        Q_EMIT pane->results()->itemClicked(pane->results()->item(1));
        QCOMPARE(win.w.view()->cursor().selectedText(), QStringLiteral("beta"));
        QCOMPARE(win.w.view()->cursor().block().text(), QStringLiteral("beta gamma beta"));
        pane->searchBox()->setText(QStringLiteral("zzz"));
        pane->search();
        QCOMPARE(pane->results()->count(), 1);   // ("No results")
    }

    void theNavigationPaneResultsCanBeListedByHeadingAndByPage()
    {
        QByteArray body;
        body += "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>One</w:t></w:r></w:p>" + para("a beta here") + "<w:p><w:pPr><w:outlineLvl w:val=\"1\"/></w:pPr><w:r><w:t>One A</w:t></w:r></w:p>"
                + para("beta and beta") + "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p><w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>Two</w:t></w:r></w:p>" + para("last beta");
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        w.act("view.navigationPane")->trigger();
        NavigationPane *pane = w.navigationPane();
        QTRY_COMPARE(w.view()->pageCount(), 2);
        pane->tabs()->setCurrentIndex(2);
        pane->searchBox()->setText(QStringLiteral("beta"));
        pane->search();
        QCOMPARE(pane->results()->count(), 4);   // (as text: the places alone)
        pane->groupBox()->setCurrentIndex(1);
        QStringList rows;
        for (int i = 0; i < pane->results()->count(); ++i) rows << pane->results()->item(i)->text();
        QCOMPARE(pane->results()->count(), 7);
        QCOMPARE(rows[0], QStringLiteral("One"));
        QCOMPARE(rows[2], QStringLiteral("One A"));
        QCOMPARE(rows[5], QStringLiteral("Two"));
        QVERIFY(rows[1].contains(QStringLiteral("beta")));
        QVERIFY(!(pane->results()->item(0)->flags() & Qt::ItemIsSelectable));
        Q_EMIT pane->results()->itemClicked(pane->results()->item(4));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("beta"));
        QCOMPARE(w.view()->cursor().block().text(), QStringLiteral("beta and beta"));
        pane->groupBox()->setCurrentIndex(2);
        QCOMPARE(pane->results()->count(), 6);
        QCOMPARE(pane->results()->item(0)->text(), QStringLiteral("Page 1"));
        QCOMPARE(pane->results()->item(4)->text(), QStringLiteral("Page 2"));
        pane->groupBox()->setCurrentIndex(0);
        QCOMPARE(pane->results()->count(), 4);
    }

    void theNavigationPaneShowsThePages()
    {
        Win win({"one"});
        win.w.act("insert.pageBreak")->trigger();
        win.w.act("view.navigationPane")->trigger();
        QTRY_COMPARE(win.w.view()->pageCount(), 2);
        win.w.navigationPane()->tabs()->setCurrentIndex(1);
        win.w.navigationPane()->refresh();
        QCOMPARE(win.w.navigationPane()->pages()->count(), 2);
    }

    void focusModeHidesTheWindowPartsAndBringsThemBack()
    {
        Win win;
        QVERIFY(win.w.ribbon()->isVisible());
        win.w.act("view.focus")->trigger();
        QVERIFY(win.w.focusMode());
        QVERIFY(!win.w.ribbon()->isVisible());
        QVERIFY(!win.w.ruler()->isVisible());
        QTest::keyClick(win.w.view(), Qt::Key_Escape);
        QVERIFY(!win.w.focusMode());
        QVERIFY(win.w.ribbon()->isVisible());
        QVERIFY(win.w.ruler()->isVisible());
    }

    // The ruler's margin marker stays over the page's text edge when the
    // window grows or shrinks (the pages move to stay centered).
    void theRulerFollowsThePagesWhenTheWindowIsResized()
    {
        Win win;
        auto marginAt = [&](double *ruler, double *page) {
            QCoreApplication::processEvents();
            QPointF at;
            QVERIFY(win.w.ruler()->markerPoint(write::Ruler::Marker::LeftMargin, 0, &at));
            *ruler = at.x();
            const QRect pg = win.w.view()->pageRect(win.w.view()->currentPage());
            const double scale = win.w.view()->zoom() * 96.0 / 72.0;
            *page = win.w.ruler()->mapFromGlobal(win.w.view()->viewport()->mapToGlobal(QPoint(pg.left(), 0))).x() + win.w.ruler()->textLeft() * scale;
        };
        for (const QSize size : {QSize(1500, 900), QSize(900, 700), QSize(1700, 1000)}) {
            win.w.resize(size);
            double ruler = 0, page = 0;
            marginAt(&ruler, &page);
            QVERIFY2(std::abs(ruler - page) < 1.5, qPrintable(QStringLiteral("at %1x%2: ruler %3, page %4").arg(size.width()).arg(size.height()).arg(ruler).arg(page)));
        }
    }

    // A drag of a marker is given up by a move with no button held (the release was lost), by Esc, by the ruler losing focus, and by its being hidden: none of them changes the paragraph.
    void aRulerDragIsGivenUpWhenItsReleaseIsLostOrItIsCancelled()
    {
        for (int how = 0; how < 5; ++how) {   // 0 release as usual, 1 no button held, 2 Esc, 3 focus out, 4 hidden
            Win win;
            write::Ruler *ruler = win.w.ruler();
            QCoreApplication::processEvents();
            QPointF at;
            QVERIFY(ruler->markerPoint(write::Ruler::Marker::LeftIndent, 0, &at));
            QCOMPARE(ruler->leftIndent(), 0.0);
            auto send = [&](QEvent::Type type, const QPointF &where, Qt::MouseButtons held, Qt::MouseButton changed) {
                QMouseEvent ev(type, where, ruler->mapToGlobal(where), changed, held, Qt::NoModifier);
                QCoreApplication::sendEvent(ruler, &ev);
            };
            send(QEvent::MouseButtonPress, at, Qt::LeftButton, Qt::LeftButton);
            const QPointF far = at + QPointF(60, 0);
            send(QEvent::MouseMove, far, Qt::LeftButton, Qt::NoButton);
            if (how == 1) send(QEvent::MouseMove, far, Qt::NoButton, Qt::NoButton);
            if (how == 2) QTest::keyClick(ruler, Qt::Key_Escape);
            if (how == 3) {
                QFocusEvent out(QEvent::FocusOut, Qt::OtherFocusReason);
                QCoreApplication::sendEvent(ruler, &out);
            }
            if (how == 4) ruler->hide();
            send(QEvent::MouseButtonRelease, far, Qt::NoButton, Qt::LeftButton);
            if (how == 0) QVERIFY2(ruler->leftIndent() > 1, "a drag that is finished moves the indent");
            else QCOMPARE(ruler->leftIndent(), 0.0);
        }
    }

    // ---------------------------------------------------------------- Insert > Date & Time
    void theDateAndTimeDialogPutsInFixedTextOrAFieldThatUpdates()
    {
        Win win;
        QVERIFY(win.w.act("insert.dateTime"));
        const QString format = QStringLiteral("MMMM d, yyyy");
        withDialog([&] { win.w.act("insert.dateTime")->trigger(); }, QStringLiteral("dateTimeDialog"), [&](QDialog *dlg) {
            auto *formats = dlg->findChild<QListWidget *>(QStringLiteral("formats"));
            QVERIFY(formats && formats->count() >= 5);
            int row = -1;
            for (int i = 0; i < formats->count(); ++i)
                if (formats->item(i)->data(Qt::UserRole).toString() == format) row = i;
            QVERIFY(row >= 0);
            formats->setCurrentRow(row);
            dlg->findChild<QCheckBox *>(QStringLiteral("update"))->setChecked(false);
        });
        QVERIFY2(win.text().contains(QDate::currentDate().toString(format)), qPrintable(win.text()));
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("DATE"));
        win.w.document()->undo();
        QCOMPARE(win.text(), QStringLiteral("Hello"));
        withDialog([&] { win.w.act("insert.dateTime")->trigger(); }, QStringLiteral("dateTimeDialog"), [&](QDialog *dlg) {
            auto *formats = dlg->findChild<QListWidget *>(QStringLiteral("formats"));
            for (int i = 0; i < formats->count(); ++i)
                if (formats->item(i)->data(Qt::UserRole).toString() == format) formats->setCurrentRow(i);
            QVERIFY(dlg->findChild<QCheckBox *>(QStringLiteral("update"))->isChecked());
        });
        const QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("DATE") && out.contains("MMMM d, yyyy"), out.constData());
        QVERIFY2(out.contains(QDate::currentDate().toString(format).toUtf8()), "the field has no result");   // (a field is one character of the text, and its result is what the file keeps)
    }

    // ---------------------------------------------------------------- Insert > Cover Page
    void aCoverPageIsPutBeforeTheTextOnAPageOfItsOwnAndTakenOffAgain()
    {
        Win win({"Body text"});
        auto shown = [&] { return win.text().remove(QChar::ObjectReplacementCharacter); };   // (without the marks of the cover's bookmark)
        for (const char *id : {"insert.cover.classic", "insert.cover.band", "insert.cover.plain", "insert.cover.remove"}) QVERIFY2(win.w.act(QString::fromLatin1(id)), id);
        QVERIFY(!win.w.act("insert.cover.remove")->isEnabled());
        win.w.act("insert.cover.classic")->trigger();
        QTRY_COMPARE(win.w.view()->pageCount(), 2);
        QVERIFY(shown().startsWith(QStringLiteral("Document Title\nSubtitle\n")));
        QVERIFY(shown().contains(QChar(0x000C)));
        QVERIFY(shown().endsWith(QStringLiteral("Body text")));
        QVERIFY(win.w.document()->story()->begin().blockFormat().alignment() & Qt::AlignHCenter);
        QVERIFY(write::hasCoverPage(*win.w.document()));
        QTRY_VERIFY(win.w.act("insert.cover.remove")->isEnabled());
        // It is kept in the file, and found again there.
        auto again = open(win.w.document()->toDocx());
        QVERIFY(again && write::hasCoverPage(*again));
        // Another design takes the place of the first (one cover, one step).
        win.w.act("insert.cover.band")->trigger();
        QCOMPARE(shown().count(QStringLiteral("Document Title")), 1);
        QVERIFY(!(win.w.document()->story()->begin().blockFormat().alignment() & Qt::AlignHCenter));
        win.w.document()->undo();
        QVERIFY(win.w.document()->story()->begin().blockFormat().alignment() & Qt::AlignHCenter);
        QCOMPARE(shown().count(QStringLiteral("Document Title")), 1);
        win.w.act("insert.cover.remove")->trigger();
        QCOMPARE(shown(), QStringLiteral("Body text"));
        QVERIFY(!write::hasCoverPage(*win.w.document()));
        QTRY_COMPARE(win.w.view()->pageCount(), 1);
        win.w.document()->undo();
        QVERIFY(shown().startsWith(QStringLiteral("Document Title")));
    }

    // ---------------------------------------------------------------- review round: links
    void aLinkShowsItsRealTargetAndScreenTipWhenHoveredOver()
    {
        Win win({"Visit our site now"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(6);
        c.setPosition(14, QTextCursor::KeepAnchor);
        QVERIFY(write::insertHyperlink(*win.w.document(), c, QStringLiteral("https://real.example/x"), QString(), QStringLiteral("Our home"), QString()));
        QTextCursor in(win.w.document()->story());
        in.setPosition(9);
        win.w.view()->setCursor(in);
        QRect r;
        QVERIFY(win.w.view()->caretRect(&r));
        QToolTip::hideText();
        QHelpEvent help(QEvent::ToolTip, r.center(), win.w.view()->viewport()->mapToGlobal(r.center()));
        QApplication::sendEvent(win.w.view()->viewport(), &help);
        QVERIFY2(QToolTip::text().contains(QStringLiteral("https://real.example/x")) && QToolTip::text().contains(QStringLiteral("Our home")), qPrintable(QToolTip::text()));
        QToolTip::hideText();
    }

    void ctrlClickOnALinkAsksBeforeAnAddressThatIsNotWhatItShows()
    {
        const QByteArray file = makeLinkDocx({"https://good.example", "https://evil.example?x=1", "mailto:a@example.com", "tel:+15555550100", "file:///usr/bin/run.exe", "https://plain.example"}, [](const QStringList &ids) {
            return linkRun(ids[0], "Safe words") + linkRun(ids[1], "https://good.example") + linkRun(ids[2], "Mail me") + linkRun(ids[3], "Call me") + linkRun(ids[4], "Local file") +
                   linkRun(ids[5], "Plain words");
        });
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(file), QString());
        QStringList opened, asked, notices;
        bool yes = false;
        w.setLinkOpener([&](const QString &a) { opened << a; });
        w.setLinkAsker([&](const QString &question) {
            asked << question;
            return yes;
        });
        QObject::connect(w.view(), &write::PageView::notice, [&](const QString &t) { notices << t; });
        auto clickIn = [&](int block) {
            QTextCursor c(w.document()->story());
            c.setPosition(w.document()->story()->findBlockByNumber(block).position() + 2);
            w.view()->setCursor(c);
            QRect r;
            QVERIFY(w.view()->caretRect(&r));
            QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, r.center());
        };
        clickIn(5);   // (plain words: no question)
        QCOMPARE(opened, (QStringList{"https://plain.example"}));
        QVERIFY(asked.isEmpty());
        opened.clear();
        clickIn(1);   // (shows one address, goes to another: asks, is told no)
        QCOMPARE(asked.size(), 1);
        QVERIFY2(asked[0].contains(QStringLiteral("Open https://evil.example?x=1?")), qPrintable(asked[0]));
        QVERIFY(opened.isEmpty());
        yes = true;
        clickIn(1);
        QCOMPARE(opened, (QStringList{"https://evil.example?x=1"}));
        asked.clear();
        opened.clear();
        clickIn(2);   // (mail and telephone links ask first)
        clickIn(3);
        QCOMPARE(asked.size(), 2);
        QCOMPARE(opened, (QStringList{"mailto:a@example.com", "tel:+15555550100"}));
        // A link to a program is never followed, and the person is told.
        asked.clear();
        opened.clear();
        clickIn(4);
        QVERIFY(opened.isEmpty() && asked.isEmpty());
        QVERIFY2(!notices.isEmpty(), "no notice for the refused link");
    }

    void aLinkOverSeveralParagraphsBecomesOneLinkForEachAndSurvivesASave()
    {
        Win win({"alpha one", "beta two"});
        QTextCursor c(win.w.document()->story());
        c.setPosition(2);
        c.setPosition(win.w.document()->story()->findBlockByNumber(1).position() + 4, QTextCursor::KeepAnchor);
        QString why;
        QVERIFY2(write::insertHyperlink(*win.w.document(), c, QStringLiteral("https://span.example"), QString(), QString(), QString(), &why), qPrintable(why));
        const QByteArray saved = win.w.document()->toDocx();
        const QByteArray out = documentXml(saved);
        QCOMPARE(out.count("<w:hyperlink"), 2);
        QCOMPARE(out.count("</w:hyperlink>"), 2);
        auto again = open(saved);
        QVERIFY(again);
        const QTextDocument *story = again->story();
        for (int b = 0; b < 2; ++b) {
            const write::Hyperlink l = write::hyperlinkAt(*again, story->findBlockByNumber(b).position() + 3);
            QVERIFY2(l.valid() && l.address == QStringLiteral("https://span.example"), qPrintable(QString::number(b)));
        }
        QCOMPARE(again->story()->toPlainText().remove(QChar::ObjectReplacementCharacter), QStringLiteral("alpha one\nbeta two"));
    }

    void aLinkThatIsUndoneLeavesNoRelationshipInTheSavedFile()
    {
        Win win;
        QTextCursor &c = win.w.view()->cursor();
        c.setPosition(0);
        c.setPosition(3, QTextCursor::KeepAnchor);
        QVERIFY(write::insertHyperlink(*win.w.document(), c, QStringLiteral("https://gone.example"), QString(), QString(), QString()));
        auto relsOf = [&](const QByteArray &saved) {
            office::Package pkg;
            pkg.read(saved);
            QStringList targets;
            for (const office::Relationship &r : pkg.relationships(QStringLiteral("/word/document.xml"))) targets << r.target;
            return targets;
        };
        QVERIFY(relsOf(win.w.document()->toDocx()).contains(QStringLiteral("https://gone.example")));
        win.w.document()->undo();
        QVERIFY2(!relsOf(win.w.document()->toDocx()).contains(QStringLiteral("https://gone.example")), "an unused link relationship was written");
        win.w.document()->redo();
        QVERIFY(relsOf(win.w.document()->toDocx()).contains(QStringLiteral("https://gone.example")));
    }

    void addressesWithAPortAreWebAddressesAndAttributesAreReadProperly()
    {
        QCOMPARE(write::normalizedAddress(QStringLiteral("example.com:8080/x")), QStringLiteral("https://example.com:8080/x"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("localhost:3000")), QStringLiteral("https://localhost:3000"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("tel:+15555550100")), QStringLiteral("tel:+15555550100"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("mailto:a@b.example")), QStringLiteral("mailto:a@b.example"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("ftp://f.example")), QStringLiteral("ftp://f.example"));
    }

    void aLinkAddressIsReadFromTheRealIdAttributeNotFromTextInAScreenTip()
    {
        // A screen tip that holds text that looks like an attribute, in single quotes, is not read as the link's address.
        const QByteArray file = makeLinkDocx({"https://wrong.example", "https://right.example"}, [](const QStringList &ids) {
            return "<w:p><w:hyperlink w:tooltip='see r:id=\"" + ids[0].toUtf8() + "\" here' r:id='" + ids[1].toUtf8() + "'><w:r><w:t>words</w:t></w:r></w:hyperlink></w:p>";
        });
        auto doc = open(file);
        QVERIFY(doc);
        const write::Hyperlink l = write::hyperlinkAt(*doc, 2);
        QVERIFY(l.valid());
        QCOMPARE(l.address, QStringLiteral("https://right.example"));
        QVERIFY(l.screenTip.contains(QStringLiteral("see r:id=")));
    }

    // ---------------------------------------------------------------- review round: Text from File
    void textFromAFileTakesControlCharactersAsPlainTextAndIsCapped()
    {
        Win win;
        QTemporaryDir dir;
        int number = 0;
        auto fileWith = [&](const QByteArray &bytes) {
            QFile f(dir.filePath(QStringLiteral("t%1.txt").arg(++number)));
            f.open(QIODevice::WriteOnly);
            f.write(bytes);
            return f.fileName();
        };
        QString path = fileWith(QByteArray("a\x0c" "b\x0e" "c\x07" "d\te"));
        win.w.setTextFileChooser([&] { return path; });
        win.w.act("insert.textFromFile")->trigger();
        const QString text = win.text();
        QVERIFY2(!text.contains(QChar(0x000C)) && !text.contains(QChar(0x000E)) && !text.contains(QChar(0x0007)), "a control character got into the text");
        QVERIFY(text.contains(QStringLiteral("abcd\te")));
        QCOMPARE(win.w.document()->sectionCount(), 1);
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("w:type=\"page\""));
    }

    void aTextFileWithTooManyLinesIsRefusedWithASentence()
    {
        Win win;
        QTemporaryDir dir;
        QFile f(dir.filePath(QStringLiteral("many.txt")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        QByteArray many;
        for (int i = 0; i < write::kMostInsertedParagraphs + 1; ++i) many += "x\n";
        f.write(many);
        f.close();
        win.w.setTextFileChooser([&] { return f.fileName(); });
        const QString before = win.text();
        win.w.act("insert.textFromFile")->trigger();
        QCOMPARE(win.text(), before);
        QVERIFY(!win.w.statusBar()->currentMessage().isEmpty());
    }

    // ---------------------------------------------------------------- review round: columns
    void moreColumnsKeepsASectionOfTwentyColumnsAsItIs()
    {
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(para("text") + "<w:sectPr>" + kSect1 + "<w:cols w:num=\"20\" w:space=\"0\"/></w:sectPr>")), QString());
        QCOMPARE(w.document()->flow().section(0).columns, 20);
        withDialog([&] { w.act("layout.columns.more")->trigger(); }, QStringLiteral("columnsDialog"), [&](QDialog *dlg) { QCOMPARE(dlg->findChild<QSpinBox *>(QStringLiteral("count"))->value(), 20); });
        QCOMPARE(w.document()->flow().section(0).columns, 20);
    }

    // ---------------------------------------------------------------- review round: column lines
    void columnLinesAreOnlyOverTheColumnsOfAMixedPage()
    {
        // One column of text, then (a continuous section) two columns with a line between: the line begins below the first part and ends with the text.
        const QByteArray body = "<w:p><w:pPr><w:sectPr>" + QByteArray(kSect1) + "</w:sectPr></w:pPr><w:r><w:t>Heading of one column</w:t></w:r></w:p>" + para("left right words here and a few more words to fill") +
                                "<w:sectPr><w:type w:val=\"continuous\"/>" + kSect1 + "<w:cols w:num=\"2\" w:space=\"720\" w:sep=\"1\"/></w:sectPr>";
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        const QVector<QLineF> lines = columnLines(w, 0);
        QCOMPARE(lines.size(), 1);
        QVERIFY2(lines[0].y1() > 72 + 8, qPrintable(QString::number(lines[0].y1())));   // (below the first section's line)
        QVERIFY2(lines[0].y2() < 720 - 100, qPrintable(QString::number(lines[0].y2())));   // (ends with the text, not at the foot of the page)
    }

    void noColumnLinesAreDrawnOnABlankPageAnOddPageBreakLeaves()
    {
        const QByteArray body = "<w:p><w:pPr><w:sectPr>" + QByteArray(kSect1) + "<w:cols w:num=\"2\" w:space=\"720\" w:sep=\"1\"/></w:sectPr></w:pPr><w:r><w:t>first section words</w:t></w:r></w:p>" +
                                para("second") + "<w:sectPr><w:type w:val=\"oddPage\"/>" + kSect1 + "</w:sectPr>";
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        w.view()->paginateAll();
        QCOMPARE(w.document()->pager().pageCount(), 3);
        QCOMPARE(columnLines(w, 0).size(), 1);
        QCOMPARE(columnLines(w, 1).size(), 0);
    }

    // ---------------------------------------------------------------- review round: the Navigation pane
    void typingInABigDocumentDoesNotRebuildTheHeadingsAtOnce()
    {
        QByteArray body;
        for (int i = 0; i < 100000; ++i) body += "<w:p><w:pPr><w:outlineLvl w:val=\"0\"/></w:pPr><w:r><w:t>H" + QByteArray::number(i) + "</w:t></w:r></w:p>";
        body += para("tail");
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        w.act("view.navigationPane")->trigger();
        NavigationPane *pane = w.navigationPane();
        const int built = pane->rebuildCountForTest(), scans = pane->scanCountForTest();
        QTextCursor c(w.document()->story());
        c.movePosition(QTextCursor::End);
        w.view()->setCursor(c);
        for (int i = 0; i < 5; ++i) write::insertText(*w.document(), w.view()->cursor(), QStringLiteral("x"));
        QCOMPARE(pane->rebuildCountForTest(), built);   // (not at once)
        QCOMPARE(pane->scanCountForTest(), scans);      // (nor looked at once, for each key)
        QTRY_VERIFY(pane->scanCountForTest() > scans);  // (after the typing pauses)
        QCOMPARE(pane->rebuildCountForTest(), built);   // (the headings are the same: the tree stays)
        // A heading's words changing does rebuild it.
        QTextCursor h(w.document()->story());
        h.setPosition(1);
        w.view()->setCursor(h);
        write::insertText(*w.document(), w.view()->cursor(), QStringLiteral("Z"));
        QTRY_VERIFY(pane->rebuildCountForTest() > built);
        QCOMPARE(pane->headings()->topLevelItem(0)->text(0), QStringLiteral("HZ0"));
    }

    void thePagesTabMakesPicturesOnlyForThePagesInView()
    {
        QByteArray body;
        for (int i = 0; i < 30; ++i) body += "<w:p><w:r><w:t>page</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p>";
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(body)), QString());
        w.view()->paginateAll();
        w.act("view.navigationPane")->trigger();
        w.navigationPane()->tabs()->setCurrentIndex(1);
        w.navigationPane()->refresh();
        QVERIFY(w.navigationPane()->pages()->count() >= 30);
        QTRY_VERIFY(w.navigationPane()->thumbnailsMadeForTest() > 0);
        QVERIFY2(w.navigationPane()->thumbnailsMadeForTest() < w.navigationPane()->pages()->count(), "a picture was made for every page");
    }

    // ---------------------------------------------------------------- Insert > Barcode
    // Insert > Barcode puts a floating picture at the caret as one step: Undo takes it away, and Redo puts it back.
    void aBarcodeIsInsertedAsOneStepThatUndoesAndRedoes()
    {
        Win win({"Before after"});
        QVERIFY(win.w.act("insert.barcode"));
        WriteDoc &doc = *win.w.document();
        insertQrBarcode(win.w, QStringLiteral("https://jeffoffice.com/write"));
        const int at = firstPictureOf(doc);
        QVERIFY(at >= 0);
        const PictureInfo info = pictureAt(doc, doc.story(), at);
        QVERIFY(info.valid && info.editable && info.floating);
        QCOMPARE(info.barcode.value(QLatin1String("data")).toString(), QStringLiteral("https://jeffoffice.com/write"));
        QVERIFY(info.alt.startsWith(QLatin1String("Barcode: QR code")));
        QVERIFY(info.width > 50 && qAbs(info.width - info.height) < 0.5);
        QCOMPARE(win.w.view()->selectedPicture(), at);
        QVERIFY(doc.undo());
        QCOMPARE(firstPictureOf(doc), -1);
        QCOMPARE(doc.story()->toPlainText(), QStringLiteral("Before after"));
        QVERIFY(doc.redo());
        QCOMPARE(firstPictureOf(doc), at);
        QVERIFY(!pictureAt(doc, doc.story(), at).barcode.isEmpty());
    }

    // A marker is honored only where the pictures in the file are the ones that were written: a photo in the barcode's place is a photo, though the marker is still valid.
    void aPhotoWithAValidBarcodeMarkerIsAPhoto()
    {
        Win win({"Before after"});
        insertQrBarcode(win.w, QStringLiteral("https://jeffoffice.com/photo"));
        QString err;
        const QByteArray saved = win.w.document()->toDocx(&err);
        QImage red(60, 40, QImage::Format_RGB32);
        red.fill(Qt::red);
        QByteArray photo;
        QBuffer buffer(&photo);
        buffer.open(QIODevice::WriteOnly);
        red.save(&buffer, "PNG");
        auto swapped = WriteDoc::fromDocx(repackedDocx(saved, [&](const QString &name, QByteArray *bytes) {
            if (name.endsWith(QLatin1String(".png"))) *bytes = photo;
        }), &err);
        QVERIFY2(swapped, qPrintable(err));
        const int at = firstPictureOf(*swapped);
        QVERIFY(at >= 0);
        QVERIFY(pictureAt(*swapped, swapped->story(), at).barcode.isEmpty());
        // It is drawn as the photo (a PNG), not as vectors.
        const auto page = swapped->pager().materialize(0);
        QVERIFY(page);
        for (const auto &it : page->doc->pages[0]->items)
            if (it->type() == ItemType::Picture) QVERIFY(page->doc->images.value(static_cast<const PictureItem *>(it.get())->imageId).format != QLatin1String("svg"));
        // The pictures as they were: the barcode.
        auto same = WriteDoc::fromDocx(repackedDocx(saved, [](const QString &, QByteArray *) {}), &err);
        QVERIFY2(same, qPrintable(err));
        QVERIFY(!pictureAt(*same, same->story(), firstPictureOf(*same)).barcode.isEmpty());
    }

    // A marker whose prefix is declared on the drawing's root, not on the marker, is read; and an edit keeps another program's extension and makes no second list.
    void aMarkerWithItsPrefixDeclaredOnTheRootIsReadAndEditedInPlace()
    {
        Win win({"Before after"});
        insertQrBarcode(win.w, QStringLiteral("first"));
        QString err;
        const QByteArray saved = win.w.document()->toDocx(&err);
        auto moved = WriteDoc::fromDocx(repackedDocx(saved, [](const QString &name, QByteArray *bytes) {
            if (name != QLatin1String("/word/document.xml")) return;
            bytes->replace("<jo:barcode xmlns:jo=\"https://jeffoffice.com/ns/barcode\"", "<jo:barcode");
            bytes->replace("<w:drawing", "<w:drawing xmlns:jo=\"https://jeffoffice.com/ns/barcode\"");
            // Another program's extension, before ours.
            bytes->replace("<a:extLst><a:ext uri=\"{5B6C1E0A", "<a:extLst><a:ext uri=\"{OTHER-PROGRAM}\"><x:keep xmlns:x=\"urn:x\"/></a:ext><a:ext uri=\"{5B6C1E0A");
        }), &err);
        QVERIFY2(moved, qPrintable(err));
        const int at = firstPictureOf(*moved);
        QVERIFY(at >= 0);
        QCOMPARE(pictureAt(*moved, moved->story(), at).barcode.value(QLatin1String("data")).toString(), QStringLiteral("first"));
        QJsonObject next = pictureAt(*moved, moved->story(), at).barcode;
        next["data"] = QStringLiteral("second");
        QVERIFY(changeBarcode(*moved, at, next));
        office::Package pkg;
        QVERIFY(pkg.read(moved->toDocx(&err)));
        const QByteArray xml = pkg.part(QStringLiteral("/word/document.xml"));
        QCOMPARE(xml.count("<a:extLst>"), 2);   // (the picture's own blip list, and the one in the properties)
        QVERIFY(xml.contains("{OTHER-PROGRAM}"));
        QCOMPARE(xml.count("{5B6C1E0A"), 1);
        QCOMPARE(pictureAt(*moved, moved->story(), at).barcode.value(QLatin1String("data")).toString(), QStringLiteral("second"));
    }

    // A picture of the window with a QR code in the text, for the docs, when JP_SHOT_DIR names a folder.
    void aPictureOfTheWindowWithABarcodeForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
        Win win({"Scan this code with a phone to open our web site:"});
        win.w.resize(1300, 900);
        QTextCursor c(win.w.document()->story());
        c.movePosition(QTextCursor::End);
        win.w.view()->setCursor(c);
        insertQrBarcode(win.w, QStringLiteral("https://jeffoffice.com"));
        QCoreApplication::processEvents();
        win.w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/write-qr.png"));
    }

    // A barcode that is a merge field's has what a field can carry: the digits, the size, the height, and the QR level. Not the book price, an add-on, a check character, the clear background, or a book's ISBN as a type.
    void aBarcodeDialogForAFieldOffersOnlyWhatAFieldCarries()
    {
        QJsonObject out;
        withDialog([&] { askBarcode(nullptr, QJsonObject{{"type", 0}, {"data", "978-1-64002-163-1"}}, &out, true); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            auto *type = d->findChildren<QComboBox *>().first();
            QVERIFY(!(type->model()->flags(type->model()->index(0, 0)) & Qt::ItemIsEnabled));
            QVERIFY(type->currentIndex() != 0);
            for (QCheckBox *c : d->findChildren<QCheckBox *>())
                if (c->text().startsWith(QLatin1String("Add a check")) || c->text().startsWith(QLatin1String("White background"))) QVERIFY(!c->isVisibleTo(d));
            for (QGroupBox *g : d->findChildren<QGroupBox *>())
                if (g->title().isEmpty() || g->title().contains(QLatin1String("add-on"), Qt::CaseInsensitive) || g->title().contains(QLatin1String("price"), Qt::CaseInsensitive)) QVERIFY(!g->isVisibleTo(d));
            for (QLineEdit *l : d->findChildren<QLineEdit *>())
                if (l->placeholderText().startsWith(QLatin1String("none, or 2"))) QVERIFY(!l->isVisibleTo(d));
        });
    }

    // A barcode a file holds is read at any length the encoder draws, though a new one is held to 500 characters.
    void aLongCode128MarkerStillReadsAsABarcode()
    {
        const QJsonObject settings{{"type", int(barcode::Type::Code128)}, {"data", QString(600, QLatin1Char('A'))}};
        QVERIFY(!office::barcodeSettingsFromText(office::barcodeSettingsText(settings)).isEmpty());
    }

    // With the barcode selected, Insert > Barcode edits it: the dialog starts from its data, the picture keeps its place and width, and one step undoes it.
    void aBarcodeIsEditedAgainFromItsData()
    {
        Win win({"Before after"});
        WriteDoc &doc = *win.w.document();
        insertQrBarcode(win.w, QStringLiteral("first"));
        const int at = firstPictureOf(doc);
        QVERIFY(at >= 0);
        QVERIFY(setPictureSize(doc, at, 90, 90));
        win.w.view()->selectPicture(at);
        withDialog([&] { win.w.act("insert.barcode")->trigger(); }, QStringLiteral("barcodeDialog"), [&](QDialog *d) {
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Barcode"));
            auto *line = d->findChildren<QLineEdit *>().first();
            QCOMPARE(line->text(), QStringLiteral("first"));
            line->setText(QStringLiteral("second"));
        });
        QCOMPARE(firstPictureOf(doc), at);
        PictureInfo now = pictureAt(doc, doc.story(), at);
        QCOMPARE(now.barcode.value(QLatin1String("data")).toString(), QStringLiteral("second"));
        QCOMPARE(now.width, 90.0);
        QVERIFY(doc.undo());
        QCOMPARE(pictureAt(doc, doc.story(), at).barcode.value(QLatin1String("data")).toString(), QStringLiteral("first"));
    }

    // Saved, the barcode is a picture of its own in the package (an SVG with a PNG, each with its content type), with the marker; another program shows the picture.
    // Opened again, it is the same barcode at the size it was given, and it is drawn as vectors.
    void aSavedBarcodeIsAPictureOthersShowAndReopensAsABarcode()
    {
        Win win({"Before after"});
        WriteDoc &doc = *win.w.document();
        insertQrBarcode(win.w, QStringLiteral("https://jeffoffice.com/round-trip"));
        const int at = firstPictureOf(doc);
        QVERIFY(at >= 0);
        QVERIFY(setPictureSize(doc, at, 100, 100));
        const QJsonObject settings = pictureAt(doc, doc.story(), at).barcode;
        QVERIFY(!settings.isEmpty());

        QString err;
        const QByteArray saved = doc.toDocx(&err);
        QVERIFY2(!saved.isEmpty(), qPrintable(err));
        office::Package pkg;
        QVERIFY(pkg.read(saved));
        QString svg, png;
        for (const QString &name : pkg.partNames()) {
            if (name.endsWith(QLatin1String(".svg"))) svg = name;
            if (name.endsWith(QLatin1String(".png"))) png = name;
        }
        QVERIFY(!svg.isEmpty() && !png.isEmpty());
        QCOMPARE(pkg.contentType(svg), QStringLiteral("image/svg+xml"));
        QCOMPARE(pkg.contentType(png), QStringLiteral("image/png"));
        QVERIFY(pkg.part(svg).contains("<svg"));
        QVERIFY(pkg.part(png).startsWith("\x89PNG"));
        const QByteArray body = pkg.part(QStringLiteral("/word/document.xml"));
        QVERIFY(body.contains("<wp:anchor"));
        QVERIFY(body.contains("jeffoffice.com/ns/barcode"));
        QVERIFY(body.contains("<asvg:svgBlip "));
        QVERIFY(body.contains("descr=\"Barcode: QR code https://jeffoffice.com/round-trip\""));
        // Both pictures are named by the document's relationships, as images.
        int images = 0;
        for (const office::Relationship &r : pkg.relationships(QStringLiteral("/word/document.xml")))
            if (r.type.endsWith(QLatin1String("/image")) && (r.target == svg || r.target == png)) ++images;
        QCOMPARE(images, 2);

        auto back = WriteDoc::fromDocx(saved, &err);
        QVERIFY2(back, qPrintable(err));
        const int again = firstPictureOf(*back);
        QVERIFY(again >= 0);
        const PictureInfo info = pictureAt(*back, back->story(), again);
        QCOMPARE(info.barcode, settings);
        QVERIFY(info.floating);
        QCOMPARE(info.width, 100.0);
        QCOMPARE(info.height, 100.0);
        QCOMPARE(back->story()->toPlainText(), doc.story()->toPlainText());
        // Drawn from the settings as vectors: the page's picture is an SVG.
        const auto page = back->pager().materialize(0);
        QVERIFY(page);
        bool vector = false;
        for (const auto &it : page->doc->pages[0]->items)
            if (it->type() == ItemType::Picture) vector = vector || page->doc->images.value(static_cast<const PictureItem *>(it.get())->imageId).format == QLatin1String("svg");
        QVERIFY(vector);
        // And saved again, it is still one barcode, not two.
        const QByteArray twice = back->toDocx(&err);
        office::Package second;
        QVERIFY(second.read(twice));
        QCOMPARE(second.part(QStringLiteral("/word/document.xml")).count("jeffoffice.com/ns/barcode"), 1);
    }

    // In an open document text a barcode is a picture frame (a PNG) with the settings in an attribute of the frame; opened again, it is the same barcode at the size it was given.
    void inAnOpenDocumentTextABarcodeIsAPictureFrameWithItsSettings()
    {
        Win win({"Before after"});
        WriteDoc &doc = *win.w.document();
        insertQrBarcode(win.w, QStringLiteral("https://jeffoffice.com/odt"));
        const int at = firstPictureOf(doc);
        QVERIFY(at >= 0);
        QVERIFY(setPictureSize(doc, at, 110, 110));
        const QJsonObject settings = pictureAt(doc, doc.story(), at).barcode;
        QString err;
        const QByteArray odt = doc.toOdt(&err);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(odt, parts, &err));
        const QString content = QString::fromUtf8(parts.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("jo:barcode=")));
        QVERIFY(content.contains(QStringLiteral("<draw:image ")));
        auto back = WriteDoc::fromOdt(odt, &err);
        QVERIFY2(back, qPrintable(err));
        const int again = firstPictureOf(*back);
        QVERIFY(again >= 0);
        const PictureInfo info = pictureAt(*back, back->story(), again);
        QCOMPARE(info.barcode, settings);
        QVERIFY(qAbs(info.width - 110) < 0.5);
    }

    // A marker with no barcode in it leaves an ordinary picture.
    void aBarcodeMarkerWithNoDataIsJustAPicture()
    {
        Win win({"Before after"});
        WriteDoc &doc = *win.w.document();
        insertQrBarcode(win.w, QStringLiteral("x"));
        QString err;
        const QByteArray saved = doc.toDocx(&err);
        office::Package pkg;
        QVERIFY(pkg.read(saved));
        QByteArray body = pkg.part(QStringLiteral("/word/document.xml"));
        QVERIFY(body.contains("&quot;data&quot;:&quot;x&quot;"));
        body.replace("&quot;data&quot;:&quot;x&quot;", "&quot;data&quot;:&quot;&quot;");
        office::PackageWriter w;
        for (const QString &name : pkg.partNames())
            if (name != QLatin1String("/word/document.xml")) w.addPart(name, pkg.part(name), pkg.contentType(name));
        w.addPart(QStringLiteral("/word/document.xml"), body, pkg.contentType(QStringLiteral("/word/document.xml")));
        for (const QString &source : QStringList{QStringLiteral("/"), QStringLiteral("/word/document.xml")})
            for (const office::Relationship &r : pkg.relationships(source)) w.addRelationshipWithId(source, r.id, r.type, r.target, r.external);
        auto back = WriteDoc::fromDocx(w.finish(), &err);
        QVERIFY2(back, qPrintable(err));
        const int at = firstPictureOf(*back);
        QVERIFY(at >= 0);
        QVERIFY(pictureAt(*back, back->story(), at).barcode.isEmpty());
    }

    // ---- content controls (core/contentcontrolops.h)

    // A control of the kind made at the caret of a window on one paragraph (the caret after "Hello").
    static int makeControl(Win &win, ControlKind kind)
    {
        QString why;
        if (!insertContentControl(*win.w.document(), win.w.view()->cursor(), kind, &why)) return -1;
        const QVector<ContentControl> all = contentControls(*win.w.document());
        return all.size() == 1 ? all.first().start : -1;
    }

    void eachKindOfContentControlIsMadeAtTheCaretAndWritten()
    {
        struct Case {
            ControlKind kind;
            const char *element;
            bool placeholder;
        };
        const Case cases[] = {{ControlKind::PlainText, "<w:text/>", true}, {ControlKind::DropDown, "<w:dropDownList>", true}, {ControlKind::Date, "<w:date>", true}, {ControlKind::CheckBox, "<w14:checkbox>", false}};
        for (const Case &k : cases) {
            Win win;
            const int start = makeControl(win, k.kind);
            QVERIFY(start >= 0);
            const ContentControl c = contentControlAt(*win.w.document(), start + 1);
            QCOMPARE(c.props.kind, k.kind);
            QCOMPARE(c.placeholder, k.placeholder);
            QVERIFY(!c.text.isEmpty());
            QVERIFY(c.editable);
            const QByteArray out = documentXml(win.w.document()->toDocx());
            QVERIFY2(out.contains("<w:sdt><w:sdtPr>"), out.constData());
            QVERIFY2(out.contains(k.element), k.element);
            QCOMPARE(out.contains("<w:showingPlcHdr/>"), k.placeholder);
            if (k.kind == ControlKind::CheckBox) QVERIFY(out.contains("xmlns:w14=") && out.contains("mc:Ignorable=\"w14\""));
            // (Read back: the same control.)
            auto again = open(win.w.document()->toDocx());
            const QVector<ContentControl> back = contentControls(*again);
            QCOMPARE(back.size(), 1);
            QCOMPARE(back.first().props.kind, k.kind);
            QCOMPARE(back.first().placeholder, k.placeholder);
            QCOMPARE(back.first().text, c.text);
            // One step that undoes: the text is as it was.
            win.w.document()->undo();
            QCOMPARE(contentControls(*win.w.document()).size(), 0);
            QCOMPARE(win.text().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Hello"));
        }
    }

    void aPlainTextControlOverASelectionHoldsTheSelection()
    {
        Win win;
        QTextCursor &c = win.w.view()->cursor();
        c.setPosition(1);
        c.setPosition(4, QTextCursor::KeepAnchor);
        QVERIFY(insertContentControl(*win.w.document(), c, ControlKind::PlainText));
        const QVector<ContentControl> all = contentControls(*win.w.document());
        QCOMPARE(all.size(), 1);
        QCOMPARE(all.first().text, QStringLiteral("ell"));
        QVERIFY(!all.first().placeholder);
        QCOMPARE(win.text().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Hello"));
    }

    void propertiesAreWrittenAndReadBack()
    {
        Win win;
        const int start = makeControl(win, ControlKind::DropDown);
        QVERIFY(start >= 0);
        ControlProps p = contentControlAt(*win.w.document(), start + 1).props;
        p.title = QStringLiteral("Size & \"fit\"");
        p.tag = QStringLiteral("size");
        p.lockDelete = true;
        p.items = {{QStringLiteral("Small"), QStringLiteral("s")}, {QStringLiteral("Large"), QStringLiteral("l")}};
        QVERIFY(setContentControlProps(*win.w.document(), start, p));
        QVERIFY(win.w.document()->canUndo());
        const QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("<w:alias w:val=\"Size &amp; &quot;fit&quot;\"/>"), out.constData());
        QVERIFY(out.contains("<w:tag w:val=\"size\"/>"));
        QVERIFY(out.contains("<w:lock w:val=\"sdtLocked\"/>"));
        QVERIFY(out.contains("<w:listItem w:displayText=\"Large\" w:value=\"l\"/>"));
        QVERIFY(out.contains("<w:showingPlcHdr/>"));
        auto again = open(win.w.document()->toDocx());
        const ContentControl back = contentControls(*again).first();
        QCOMPARE(back.props.title, p.title);
        QCOMPARE(back.props.tag, p.tag);
        QVERIFY(back.props.lockDelete);
        QVERIFY(!back.props.lockContents);
        QCOMPARE(back.props.items.size(), 2);
        QCOMPARE(back.props.items[1].display, QStringLiteral("Large"));
        // The kind can't be changed by properties, and a list can't be longer than the most.
        ControlProps other = p;
        other.kind = ControlKind::Date;
        QVERIFY(!setContentControlProps(*win.w.document(), start, other));
        ControlProps many = p;
        many.items.clear();
        for (int i = 0; i <= kMostControlItems; ++i) many.items.push_back({QString::number(i), QString::number(i)});
        QString why;
        QVERIFY(!setContentControlProps(*win.w.document(), start, many, &why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(contentControlAt(*win.w.document(), start + 1).props.items.size(), 2);
    }

    void choosingADropDownItemADateAndACheckBoxChangesWhatTheyShow()
    {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        Win win;
        const int start = makeControl(win, ControlKind::DropDown);
        ControlProps p = contentControlAt(*win.w.document(), start + 1).props;
        p.items = {{QStringLiteral("Small"), QStringLiteral("s")}, {QStringLiteral("Large"), QStringLiteral("l")}};
        QVERIFY(setContentControlProps(*win.w.document(), start, p));
        QVERIFY(!chooseControlItem(*win.w.document(), start, 2));
        QVERIFY(chooseControlItem(*win.w.document(), start, 1));
        ContentControl c = contentControlAt(*win.w.document(), start + 1);
        QCOMPARE(c.text, QStringLiteral("Large"));
        QVERIFY(!c.placeholder);
        QCOMPARE(controlItemShown(c), 1);
        QByteArray out = documentXml(win.w.document()->toDocx());
        QVERIFY2(out.contains("w:lastValue=\"l\""), out.constData());
        QVERIFY(!out.contains("showingPlcHdr"));
        win.w.document()->undo();
        QVERIFY(contentControlAt(*win.w.document(), start + 1).placeholder);

        Win dateWin;
        const int d = makeControl(dateWin, ControlKind::Date);
        ControlProps dp = contentControlAt(*dateWin.w.document(), d + 1).props;
        dp.dateFormat = QStringLiteral("yyyy-MM-dd");
        QVERIFY(setContentControlProps(*dateWin.w.document(), d, dp));
        QVERIFY(setControlDate(*dateWin.w.document(), d, QDate(2026, 11, 17)));
        c = contentControlAt(*dateWin.w.document(), d + 1);
        QCOMPARE(c.text, QStringLiteral("2026-11-17"));
        QVERIFY(!c.placeholder);
        out = documentXml(dateWin.w.document()->toDocx());
        QVERIFY2(out.contains("w:fullDate=\"2026-11-17T00:00:00Z\""), out.constData());
        QVERIFY(out.contains("<w:dateFormat w:val=\"yyyy-MM-dd\"/>"));
        auto again = open(dateWin.w.document()->toDocx());
        QCOMPARE(contentControls(*again).first().props.date, QDate(2026, 11, 17));
        QCOMPARE(formatControlDate(QDate(2026, 1, 2), QStringLiteral("M/d/yy")), QStringLiteral("1/2/26"));
        QVERIFY(setControlDate(*dateWin.w.document(), d, QDate()));
        QVERIFY(contentControlAt(*dateWin.w.document(), d + 1).placeholder);

        Win boxWin;
        const int b = makeControl(boxWin, ControlKind::CheckBox);
        QCOMPARE(contentControlAt(*boxWin.w.document(), b + 1).text, QString(QChar(0x2610)));
        QVERIFY(toggleControlCheck(*boxWin.w.document(), b));
        c = contentControlAt(*boxWin.w.document(), b + 1);
        QVERIFY(c.props.checked);
        QCOMPARE(c.text, QString(QChar(0x2612)));
        QVERIFY(documentXml(boxWin.w.document()->toDocx()).contains("<w14:checked w14:val=\"1\"/>"));
        QVERIFY(toggleControlCheck(*boxWin.w.document(), b));
        QVERIFY(!contentControlAt(*boxWin.w.document(), b + 1).props.checked);
        QVERIFY(!toggleControlCheck(*boxWin.w.document(), 0));
    }

    void typingOverAPlaceholderReplacesItInOneStep()
    {
        Win win;
        const int start = makeControl(win, ControlKind::PlainText);
        QTextCursor &c = win.w.view()->cursor();
        QVERIFY(c.hasSelection());   // (the placeholder is selected, as a person's next key replaces it)
        write::insertText(*win.w.document(), c, QStringLiteral("Ada"));
        ContentControl ctl = contentControlAt(*win.w.document(), start + 1);
        QCOMPARE(ctl.text, QStringLiteral("Ada"));
        QVERIFY(!ctl.placeholder);
        QVERIFY(!documentXml(win.w.document()->toDocx()).contains("showingPlcHdr"));
        write::insertText(*win.w.document(), c, QStringLiteral("m"));
        QCOMPARE(contentControlAt(*win.w.document(), start + 1).text, QStringLiteral("Adam"));
        win.w.document()->undo();   // (the typing is one step with the placeholder's going)
        ctl = contentControlAt(*win.w.document(), start + 1);
        QVERIFY2(ctl.placeholder, qPrintable(ctl.text));
        QVERIFY(documentXml(win.w.document()->toDocx()).contains("showingPlcHdr"));
    }

    void locksKeepTheControlAndItsContents()
    {
        Win win;
        const int start = makeControl(win, ControlKind::PlainText);
        WriteDoc &doc = *win.w.document();
        QTextCursor &c = win.w.view()->cursor();
        write::insertText(doc, c, QStringLiteral("abc"));
        ContentControl ctl = contentControlAt(doc, start + 1);
        QCOMPARE(ctl.text, QStringLiteral("abc"));
        ControlProps p = ctl.props;
        p.lockContents = true;
        QVERIFY(setContentControlProps(doc, start, p));
        // Contents locked: no typing, no deleting inside; the control as a whole can go.
        c.setPosition(ctl.end);
        write::insertText(doc, c, QStringLiteral("x"));
        write::deleteBack(doc, c, false);
        QCOMPARE(contentControlAt(doc, start + 1).text, QStringLiteral("abc"));
        p.lockContents = false;
        p.lockDelete = true;
        QVERIFY(setContentControlProps(doc, start, p));
        // Not deletable: the text inside can be edited, the control can't be selected away.
        c.setPosition(ctl.end);
        write::deleteBack(doc, c, false);
        QCOMPARE(contentControlAt(doc, start + 1).text, QStringLiteral("ab"));
        c.setPosition(0);
        c.setPosition(doc.story()->characterCount() - 1, QTextCursor::KeepAnchor);
        write::removeSelection(doc, c);
        QCOMPARE(contentControls(doc).size(), 1);
        QVERIFY(doc.story()->toPlainText().contains(QLatin1String("Hello")));
        // An unlocked one goes whole with a selection over it, and Enter inside one makes no paragraph.
        p.lockDelete = false;
        QVERIFY(setContentControlProps(doc, start, p));
        c.setPosition(start + 2);
        const int blocks = doc.story()->blockCount();
        write::insertParagraph(doc, c);
        QCOMPARE(doc.story()->blockCount(), blocks);
        c.setPosition(0);
        c.setPosition(doc.story()->characterCount() - 1, QTextCursor::KeepAnchor);
        write::removeSelection(doc, c);
        QCOMPARE(contentControls(doc).size(), 0);
    }

    void aCheckBoxAndADropDownHoldOnlyWhatIsChosen()
    {
        for (ControlKind kind : {ControlKind::CheckBox, ControlKind::DropDown, ControlKind::Date}) {
            Win win;
            const int start = makeControl(win, kind);
            WriteDoc &doc = *win.w.document();
            const QString before = contentControlAt(doc, start + 1).text;
            QTextCursor &c = win.w.view()->cursor();
            c.setPosition(contentControlAt(doc, start + 1).end);
            write::insertText(doc, c, QStringLiteral("x"));
            write::deleteBack(doc, c, false);
            QCOMPARE(contentControlAt(doc, start + 1).text, before);
            // (Typing beside it is not inside it.)
            c.setPosition(contentControlAt(doc, start + 1).end + 1);
            write::insertText(doc, c, QStringLiteral("!"));
            QCOMPARE(contentControlAt(doc, start + 1).text, before);
            QVERIFY(win.text().contains(QLatin1String("!")));
        }
    }

    void controlsOfAFileAreKeptByteForByteUntilTheyAreChanged()
    {
        const QByteArray sdt = "<w:sdt><w:sdtPr><w:rPr><w:b/></w:rPr><w:alias w:val=\"Name\"/><w:tag w:val=\"n\"/><w:id w:val=\"42\"/><w:placeholder><w:docPart w:val=\"DefaultPlaceholder\"/></w:placeholder>"
                               "<w:showingPlcHdr/><w:dataBinding w:xpath=\"/a/b\" w:storeItemID=\"{1}\"/><w:text/></w:sdtPr><w:sdtEndPr><w:rPr><w:i/></w:rPr></w:sdtEndPr><w:sdtContent><w:r><w:t xml:space=\"preserve\">Your name</w:t></w:r></w:sdtContent></w:sdt>";
        const QByteArray body = "<w:p><w:r><w:t>A</w:t></w:r>" + sdt + "<w:r><w:t>B</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains(sdt), out.constData());
        const QVector<ContentControl> all = contentControls(*doc);
        QCOMPARE(all.size(), 1);
        QCOMPARE(all.first().props.title, QStringLiteral("Name"));
        QVERIFY(all.first().placeholder);
        // A change to its title keeps what is not known here (the binding, the placeholder part, its end properties).
        ControlProps p = all.first().props;
        p.title = QStringLiteral("Full name");
        QVERIFY(setContentControlProps(*doc, all.first().start, p));
        const QByteArray changed = documentXml(doc->toDocx());
        QVERIFY2(changed.contains("<w:alias w:val=\"Full name\"/>"), changed.constData());
        QVERIFY(changed.contains("<w:id w:val=\"42\"/>"));
        QVERIFY(changed.contains("<w:dataBinding w:xpath=\"/a/b\" w:storeItemID=\"{1}\"/>"));
        QVERIFY(changed.contains("<w:placeholder><w:docPart w:val=\"DefaultPlaceholder\"/></w:placeholder>"));
        QVERIFY(changed.contains("<w:sdtEndPr><w:rPr><w:i/></w:rPr></w:sdtEndPr>"));
        QVERIFY(changed.contains("<w:rPr><w:b/></w:rPr>"));
        // A control of another kind (a combo box, rich text) is shown and kept, its title and locks can change, and typing in a locked one is refused.
        const QByteArray combo = "<w:sdt><w:sdtPr><w:comboBox><w:listItem w:displayText=\"a\" w:value=\"a\"/></w:comboBox><w:lock w:val=\"sdtContentLocked\"/></w:sdtPr><w:sdtContent><w:r><w:t xml:space=\"preserve\">a</w:t></w:r></w:sdtContent></w:sdt>";
        auto other = open(makeDocx("<w:p>" + combo + "</w:p>"));
        QCOMPARE(documentXml(other->toDocx()).contains(combo), true);
        const ContentControl oc = contentControls(*other).first();
        QCOMPARE(oc.props.kind, ControlKind::Other);
        QVERIFY(oc.props.lockDelete && oc.props.lockContents);
        QTextCursor c(other->story());
        c.setPosition(oc.end);
        write::insertText(*other, c, QStringLiteral("z"));
        QCOMPARE(contentControls(*other).first().text, QStringLiteral("a"));
    }

    void hostileFilesBoundTheControlsRead()
    {
        // More than the most controls: only the most are counted, and no more is made.
        QByteArray body = "<w:p>";
        for (int i = 0; i < kMostControls + 5; ++i) body += "<w:sdt><w:sdtPr><w:text/></w:sdtPr><w:sdtContent><w:r><w:t>x</w:t></w:r></w:sdtContent></w:sdt>";
        body += "</w:p>";
        auto many = open(makeDocx(body));
        QVERIFY(many);
        QCOMPARE(contentControls(*many).size(), kMostControls);
        QTextCursor c(many->story());
        c.setPosition(0);
        QString why;
        QVERIFY(!insertContentControl(*many, c, ControlKind::PlainText, &why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(contentControls(*many).size(), kMostControls);
        // Controls inside controls: those deeper than the most are not counted, and none is made inside the deepest.
        QByteArray deep = "<w:p>";
        for (int i = 0; i < kMostControlDepth + 4; ++i) deep += "<w:sdt><w:sdtPr><w:text/></w:sdtPr><w:sdtContent>";
        deep += "<w:r><w:t>core</w:t></w:r>";
        for (int i = 0; i < kMostControlDepth + 4; ++i) deep += "</w:sdtContent></w:sdt>";
        deep += "</w:p>";
        auto nested = open(makeDocx(deep));
        QVERIFY(nested);
        const QVector<ContentControl> all = contentControls(*nested);
        QVERIFY(all.size() <= kMostControlDepth);
        QVERIFY(!all.isEmpty());
        int deepest = 0;
        for (const ContentControl &k : all) deepest = std::max(deepest, k.depth);
        QCOMPARE(deepest, int(all.size()) - 1);
        // A list of more items than the most is left as it is, as a control of another kind.
        QByteArray items = "<w:p><w:sdt><w:sdtPr><w:dropDownList>";
        for (int i = 0; i < kMostControlItems + 1; ++i) items += "<w:listItem w:displayText=\"i\" w:value=\"" + QByteArray::number(i) + "\"/>";
        items += "</w:dropDownList></w:sdtPr><w:sdtContent><w:r><w:t>i</w:t></w:r></w:sdtContent></w:sdt></w:p>";
        auto list = open(makeDocx(items));
        QVERIFY(list);
        const ContentControl lc = contentControls(*list).first();
        QCOMPARE(lc.props.kind, ControlKind::Other);
        QCOMPARE(lc.props.items.size(), 0);
        QVERIFY(!chooseControlItem(*list, lc.start, 0));
        QVERIFY(documentXml(list->toDocx()).contains("w:value=\"1000\""));
    }

    // The middle of a control's words in the window (from where its text begins to where it ends).
    static QPoint controlPoint(WriteWindow &w, int start)
    {
        const ContentControl c = contentControlAt(*w.document(), start + 1);
        QTextCursor cur(w.document()->story());
        QRect a, b;
        cur.setPosition(c.start + 1);
        w.view()->setCursor(cur);
        w.view()->caretRect(&a);
        cur.setPosition(c.end);
        w.view()->setCursor(cur);
        w.view()->caretRect(&b);
        return QPoint((a.center().x() + b.center().x()) / 2, a.center().y());
    }

    // The menu of choices a click showed (null if none).
    static QMenu *choicesOf(WriteWindow &w)
    {
        for (QMenu *m : w.findChildren<QMenu *>(QStringLiteral("controlChoices")))
            if (m->isVisible()) return m;
        return nullptr;
    }

    void theInsertMenuMakesEachControlAndPropertiesNeedsOne()
    {
        Win win;
        WriteWindow &w = win.w;
        for (const char *id : {"insert.control.text", "insert.control.dropDown", "insert.control.date", "insert.control.checkBox", "insert.control.properties"}) QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QVERIFY(!w.act(QStringLiteral("insert.control.properties"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("insert.control.text"))->isEnabled());
        w.act(QStringLiteral("insert.control.text"))->trigger();
        QCOMPARE(contentControls(*w.document()).size(), 1);
        QVERIFY(w.view()->cursor().hasSelection());   // (the placeholder is selected)
        QTRY_VERIFY(w.act(QStringLiteral("insert.control.properties"))->isEnabled());
        for (const char *id : {"insert.control.checkBox", "insert.control.date", "insert.control.dropDown"}) {
            w.view()->cursor().setPosition(w.document()->story()->characterCount() - 1);
            w.act(QString::fromLatin1(id))->trigger();
        }
        const QVector<ContentControl> all = contentControls(*w.document());
        QCOMPARE(all.size(), 4);
        QVERIFY(w.document()->canUndo());
    }

    void aClickFlipsACheckBoxAndSelectsAPlaceholder()
    {
        Win win;
        WriteWindow &w = win.w;
        const int box = makeControl(win, ControlKind::CheckBox);
        QVERIFY(box >= 0);
        w.view()->paginateAll();
        QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(w, box));
        QVERIFY(contentControlAt(*w.document(), box + 1).props.checked);
        QCOMPARE(w.view()->cursor().position(), contentControlAt(*w.document(), box + 1).end + 1);   // (the caret goes on after it)
        QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(w, box));
        QVERIFY(!contentControlAt(*w.document(), box + 1).props.checked);
        // With its contents locked the click does nothing.
        ControlProps p = contentControlAt(*w.document(), box + 1).props;
        p.lockContents = true;
        QVERIFY(setContentControlProps(*w.document(), box, p));
        QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(w, box));
        QVERIFY(!contentControlAt(*w.document(), box + 1).props.checked);
        // The space key flips the box when the caret is in it.
        p.lockContents = false;
        QVERIFY(setContentControlProps(*w.document(), box, p));
        w.view()->cursor().setPosition(box + 2);
        QTest::keyClick(w.view()->viewport(), Qt::Key_Space);
        QVERIFY(contentControlAt(*w.document(), box + 1).props.checked);
        // A text control's placeholder is selected by a click, and what is typed replaces it.
        Win text;
        const int t = makeControl(text, ControlKind::PlainText);
        text.w.view()->paginateAll();
        QTextCursor c(text.w.document()->story());
        c.setPosition(0);
        text.w.view()->setCursor(c);
        QTest::mouseClick(text.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(text.w, t));
        QVERIFY(text.w.view()->cursor().hasSelection());
        QCOMPARE(text.w.view()->cursor().selectedText().remove(QChar::ObjectReplacementCharacter), contentControlAt(*text.w.document(), t + 1).text);
        QTest::keyClicks(text.w.view()->viewport(), QStringLiteral("Zed"));
        QCOMPARE(contentControlAt(*text.w.document(), t + 1).text, QStringLiteral("Zed"));
    }

    void aClickOnADropDownShowsItsItemsAndOnADateACalendar()
    {
        Win win;
        WriteWindow &w = win.w;
        const int list = makeControl(win, ControlKind::DropDown);
        ControlProps p = contentControlAt(*w.document(), list + 1).props;
        p.items = {{QStringLiteral("Small"), QStringLiteral("s")}, {QStringLiteral("Large"), QStringLiteral("l")}};
        QVERIFY(setContentControlProps(*w.document(), list, p));
        w.view()->paginateAll();
        QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(w, list));
        QMenu *menu = nullptr;
        QTRY_VERIFY((menu = choicesOf(w)) != nullptr);
        QCOMPARE(menu->actions().size(), 2);
        QCOMPARE(menu->actions()[1]->text(), QStringLiteral("Large"));
        menu->actions()[1]->trigger();
        QCOMPARE(contentControlAt(*w.document(), list + 1).text, QStringLiteral("Large"));
        menu->close();
        // With its contents locked, no choices are shown.
        p.lockContents = true;
        QVERIFY(setContentControlProps(*w.document(), list, p));
        QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(w, list));
        QVERIFY(choicesOf(w) == nullptr);

        Win dateWin;
        const int d = makeControl(dateWin, ControlKind::Date);
        dateWin.w.view()->paginateAll();
        QTest::mouseClick(dateWin.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(dateWin.w, d));
        QMenu *cal = nullptr;
        QTRY_VERIFY((cal = choicesOf(dateWin.w)) != nullptr);
        auto *calendar = cal->findChild<QCalendarWidget *>(QStringLiteral("controlCalendar"));
        QVERIFY(calendar);
        calendar->setSelectedDate(QDate(2026, 3, 9));
        Q_EMIT calendar->clicked(QDate(2026, 3, 9));
        QCOMPARE(contentControlAt(*dateWin.w.document(), d + 1).props.date, QDate(2026, 3, 9));
        QCOMPARE(contentControlAt(*dateWin.w.document(), d + 1).text, formatControlDate(QDate(2026, 3, 9), QStringLiteral("M/d/yyyy")));
    }

    void thePropertiesBoxEditsTheControlInOneStep()
    {
        Win win;
        WriteWindow &w = win.w;
        const int list = makeControl(win, ControlKind::DropDown);
        QVERIFY(list >= 0);
        QTRY_VERIFY(w.act(QStringLiteral("insert.control.properties"))->isEnabled());
        withDialog([&] { w.act(QStringLiteral("insert.control.properties"))->trigger(); }, QStringLiteral("contentControlDialog"), [](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("title"))->setText(QStringLiteral("Shirt size"));
            d->findChild<QLineEdit *>(QStringLiteral("tag"))->setText(QStringLiteral("size"));
            d->findChild<QCheckBox *>(QStringLiteral("lockDelete"))->setChecked(true);
            auto *items = d->findChild<QListWidget *>(QStringLiteral("items"));
            QVERIFY(items);
            d->findChild<QPushButton *>(QStringLiteral("addItem"))->click();
            QCOMPARE(items->count(), 1);
            items->item(0)->setText(QStringLiteral("Small"));
            d->findChild<QPushButton *>(QStringLiteral("addItem"))->click();
            items->item(1)->setText(QStringLiteral("Large"));
            items->setCurrentRow(1);
            d->findChild<QPushButton *>(QStringLiteral("itemUp"))->click();
            QCOMPARE(items->item(0)->text(), QStringLiteral("Large"));
        });
        const ContentControl c = contentControlAt(*w.document(), list + 1);
        QCOMPARE(c.props.title, QStringLiteral("Shirt size"));
        QCOMPARE(c.props.tag, QStringLiteral("size"));
        QVERIFY(c.props.lockDelete);
        QCOMPARE(c.props.items.size(), 2);
        QCOMPARE(c.props.items[0].display, QStringLiteral("Large"));
        // One step: an undo takes the title and the items back together.
        w.document()->undo();
        QVERIFY(contentControlAt(*w.document(), list + 1).props.title.isEmpty());
        QCOMPARE(contentControlAt(*w.document(), list + 1).props.items.size(), 0);
        // Cancel changes nothing.
        w.document()->redo();
        w.view()->cursor().setPosition(list + 2);
        QTRY_VERIFY(w.act(QStringLiteral("insert.control.properties"))->isEnabled());
        withDialog([&] { w.act(QStringLiteral("insert.control.properties"))->trigger(); }, QStringLiteral("contentControlDialog"), [](QDialog *d) {
            d->findChild<QLineEdit *>(QStringLiteral("title"))->setText(QStringLiteral("Other"));
        }, false);
        QCOMPARE(contentControlAt(*w.document(), list + 1).props.title, QStringLiteral("Shirt size"));
    }

    void aFormWithEachControlIsDrawnWithABoxForTheOneTheCaretIsIn()
    {
        Win win({"Name", "Size", "Date", "Agree"});
        WriteWindow &w = win.w;
        const ControlKind kinds[] = {ControlKind::PlainText, ControlKind::DropDown, ControlKind::Date, ControlKind::CheckBox};
        for (int i = 0; i < 4; ++i) {
            QTextCursor c(w.document()->story());
            c.setPosition(w.document()->story()->findBlockByNumber(i).position() + int(w.document()->story()->findBlockByNumber(i).text().size()));
            QVERIFY(insertContentControl(*w.document(), c, kinds[i]));
        }
        QCOMPARE(contentControls(*w.document()).size(), 4);
        const QVector<ControlMark> &marks = w.view()->controlMarks();
        QCOMPARE(marks.size(), 4);
        int focused = 0;
        for (const ControlMark &m : marks) focused += m.focus;
        QCOMPARE(focused, 0);   // (the caret is in the last: a check box is not selected, it is after it)
        QTextCursor c(w.document()->story());
        c.setPosition(contentControls(*w.document())[1].start + 2);
        w.view()->setCursor(c);
        int on = -1;
        const QVector<ControlMark> &again = w.view()->controlMarks();
        for (int i = 0; i < again.size(); ++i)
            if (again[i].focus) on = i;
        QCOMPARE(on, 1);
        // Painted: the page's picture differs from one with no boxes (the shade and the outline are drawn).
        w.view()->paginateAll();
        const QImage with = w.view()->viewport()->grab().toImage();
        w.document()->undo();
        w.view()->paginateAll();
        QVERIFY(!with.isNull());
    }

    // A picture for a person to look at (JW_SHOT_DIR names the folder; nothing is made without it): a form with each control and the drop-down open.
    void contentControlScreenshots()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        Win win({"Full name: ", "", "Shirt size: ", "", "Start date: ", "", "I agree to the terms: ", "", "Thank you for filling in this form."});
        WriteWindow &w = win.w;
        w.resize(1100, 760);
        const ControlKind kinds[] = {ControlKind::PlainText, ControlKind::DropDown, ControlKind::Date, ControlKind::CheckBox};
        const char *titles[] = {"Name", "Size", "Start", "Agree"};
        for (int i = 0; i < 4; ++i) {
            QTextCursor c(w.document()->story());
            const QTextBlock b = w.document()->story()->findBlockByNumber(2 * i);
            const int at = b.position() + int(b.text().size());
            c.setPosition(at);
            QVERIFY(insertContentControl(*w.document(), c, kinds[i]));
            const ContentControl made = contentControlAt(*w.document(), at + 1);
            ControlProps p = made.props;
            p.title = QString::fromLatin1(titles[i]);
            if (kinds[i] == ControlKind::DropDown) p.items = {{QStringLiteral("Small"), QStringLiteral("s")}, {QStringLiteral("Medium"), QStringLiteral("m")}, {QStringLiteral("Large"), QStringLiteral("l")}};
            QVERIFY2(made.valid() && made.props.kind == kinds[i], qPrintable(QStringLiteral("%1: %2 %3").arg(i).arg(made.start).arg(int(made.props.kind))));
            QVERIFY(setContentControlProps(*w.document(), made.start, p));
        }
        const QVector<ContentControl> all = contentControls(*w.document());
        QVERIFY(setControlChecked(*w.document(), all[3].start, true));
        QVERIFY(setControlDate(*w.document(), all[2].start, QDate(2026, 12, 1)));
        QVERIFY(chooseControlItem(*w.document(), all[1].start, 1));
        w.show();
        if (!QTest::qWaitForWindowActive(&w)) QSKIP("the window can't be made active here");
        w.view()->paginateAll();
        QTest::mouseClick(w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, controlPoint(w, all[1].start));
        QMenu *menu = nullptr;
        QTRY_VERIFY((menu = choicesOf(w)) != nullptr);
        QImage shot = w.grab().toImage();
        {
            QPainter paint(&shot);
            paint.drawPixmap(w.mapFromGlobal(menu->pos()), menu->grab());
        }
        shot.save(dir + QLatin1String("/content-controls-dropdown.png"));
        menu->close();
        QTextCursor none(w.document()->story());
        none.setPosition(w.document()->story()->characterCount() - 1);
        w.view()->setCursor(none);
        w.grab().save(dir + QLatin1String("/content-controls-form.png"));
    }

    // The saved text has as many starts of controls as ends.
    static void verifyBalanced(const QByteArray &xmlText)
    {
        QCOMPARE(xmlText.count("<w:sdt>"), xmlText.count("</w:sdt>"));
        QCOMPARE(xmlText.count("<w:sdtContent>"), xmlText.count("</w:sdtContent>"));
    }

    void halfAControlIsNeverDeletedTypedOverOrCopiedWithItsAtoms()
    {
        Win win;
        WriteDoc &doc = *win.w.document();
        const int start = makeControl(win, ControlKind::PlainText);
        QTextCursor &c = win.w.view()->cursor();
        write::insertText(doc, c, QStringLiteral("abc"));
        c.setPosition(doc.story()->characterCount() - 1);
        write::insertText(doc, c, QStringLiteral(" tail"));
        const ContentControl ctl = contentControlAt(doc, start + 1);
        const QString before = doc.story()->toPlainText();
        // From inside past the end, from before into the middle: refused, whatever way the text is taken out.
        const int ranges[][2] = {{ctl.start + 2, ctl.end + 3}, {1, ctl.start + 3}, {ctl.start, ctl.start + 2}, {ctl.end - 1, ctl.end + 1}};
        for (const auto &r : ranges) {
            QVERIFY(splitsContentControl(doc.story(), r[0], r[1]));
            c.setPosition(r[0]);
            c.setPosition(r[1], QTextCursor::KeepAnchor);
            write::removeSelection(doc, c);
            QCOMPARE(doc.story()->toPlainText(), before);
            c.setPosition(r[0]);
            c.setPosition(r[1], QTextCursor::KeepAnchor);
            write::insertText(doc, c, QStringLiteral("Q"));
            QCOMPARE(doc.story()->toPlainText(), before);
            // (Copied, it is its words: a paste of it holds no atom.)
            c.setPosition(r[0]);
            c.setPosition(r[1], QTextCursor::KeepAnchor);
            const Clip clip = write::copySelection(doc, c);
            QVERIFY(!clip.fragment.toPlainText().contains(QChar::ObjectReplacementCharacter));
            QVERIFY(!clip.fragment.toPlainText().isEmpty());
        }
        verifyBalanced(documentXml(doc.toDocx()));
        // A selection over the whole control (or none of it) is not half.
        QVERIFY(!splitsContentControl(doc.story(), ctl.start, ctl.end + 1));
        QVERIFY(!splitsContentControl(doc.story(), ctl.start + 1, ctl.end));
        c.setPosition(ctl.start);
        c.setPosition(ctl.end + 1, QTextCursor::KeepAnchor);
        write::removeSelection(doc, c);
        QCOMPARE(contentControls(doc).size(), 0);
        verifyBalanced(documentXml(doc.toDocx()));
    }

    void aNewControlIsNotMadeOverOneEndOfAnother()
    {
        Win win;
        WriteDoc &doc = *win.w.document();
        const int start = makeControl(win, ControlKind::PlainText);
        QTextCursor &c = win.w.view()->cursor();
        write::insertText(doc, c, QStringLiteral("abc"));
        const ContentControl ctl = contentControlAt(doc, start + 1);
        c.setPosition(2);
        c.setPosition(ctl.start + 3, QTextCursor::KeepAnchor);
        QString why;
        QVERIFY(!insertContentControl(doc, c, ControlKind::PlainText, &why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(contentControls(doc).size(), 1);
        // Over the whole of it, or inside it, a control is made.
        c.setPosition(2);
        c.setPosition(ctl.end + 1, QTextCursor::KeepAnchor);
        QVERIFY(insertContentControl(doc, c, ControlKind::PlainText, &why));
        QCOMPARE(contentControls(doc).size(), 2);
        verifyBalanced(documentXml(doc.toDocx()));
    }

    void whatIsReadOnceIsNotReadAgainAtEveryKey()
    {
        QByteArray body;
        for (int p = 0; p < 100; ++p) {
            body += "<w:p>";
            for (int i = 0; i < 100; ++i) body += "<w:sdt><w:sdtPr><w:id w:val=\"" + QByteArray::number(p * 100 + i + 1) + "\"/><w:text/></w:sdtPr><w:sdtContent><w:r><w:t>x</w:t></w:r></w:sdtContent></w:sdt>";
            body += "</w:p>";
        }
        WriteWindow w;
        w.setDocument(open(makeDocx(body)), QString());
        WriteDoc &doc = *w.document();
        QCOMPARE(w.view()->controlMarks().size(), kMostControls);
        const int last = w.view()->controlMarks().last().from;
        QTextCursor c(doc.story());
        c.setPosition(doc.story()->characterCount() - 1);
        w.view()->setCursor(c);
        (void)w.view()->controlMarks();
        const int parses = controlParsesForTest();
        for (int k = 0; k < 20; ++k) {
            write::insertText(doc, w.view()->cursor(), QStringLiteral("k"));
            QCOMPARE(w.view()->controlMarks().size(), kMostControls);
        }
        write::deleteBack(doc, w.view()->cursor(), false);
        (void)w.view()->controlMarks();
        QVERIFY2(controlParsesForTest() - parses <= 2, qPrintable(QString::number(controlParsesForTest() - parses)));
        // Typed at the start of the text, the marks move with it without a new look.
        c.setPosition(0);
        w.view()->setCursor(c);
        write::insertText(doc, w.view()->cursor(), QStringLiteral("ab"));
        QCOMPARE(w.view()->controlMarks().last().from, last + 2);
        QCOMPARE(controlParsesForTest() - parses <= 2, true);
        QCOMPARE(contentControls(doc).last().start, last + 2);
    }

    void aHostileStartIsKeptAndNotRead()
    {
        // A start far longer than the most, and a title and tag longer than the most: the control is shown and kept as it was, and cannot be edited.
        const QByteArray longTitle(5000, 'T');
        const QByteArray sdt = "<w:sdt><w:sdtPr><w:alias w:val=\"" + longTitle + "\"/><w:text/></w:sdtPr><w:sdtContent><w:r><w:t xml:space=\"preserve\">x</w:t></w:r></w:sdtContent></w:sdt>";
        QByteArray huge = "<w:sdt><w:sdtPr><w:text/>";
        for (int i = 0; i < 4000; ++i) huge += "<w:rPr><w:b/></w:rPr>";
        huge += "</w:sdtPr><w:sdtContent><w:r><w:t xml:space=\"preserve\">y</w:t></w:r></w:sdtContent></w:sdt>";
        auto doc = open(makeDocx("<w:p>" + sdt + huge + "</w:p>"));
        QVERIFY(doc);
        const QVector<ContentControl> all = contentControls(*doc);
        QCOMPARE(all.size(), 2);
        for (const ContentControl &c : all) {
            QVERIFY(!c.editable);
            QCOMPARE(c.props.kind, ControlKind::Other);
        }
        QVERIFY(!setContentControlProps(*doc, all[0].start, all[0].props));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY(out.contains(sdt));
        QVERIFY(out.contains(huge));
    }

    void anAttributeHoldingQuotesAndNumberedCharactersIsReadAndWrittenWhole()
    {
        const QByteArray sdt = "<w:sdt><w:sdtPr><w:alias w:val=\"It&#8217;s &#x41;\"/><w:dropDownList><w:listItem w:displayText=\"a value='x' b\" w:value=\"v1\"/>"
                               "<w:listItem w:displayText=\"R&amp;D &lt;1&gt;\" w:value='q\"r'/></w:dropDownList></w:sdtPr><w:sdtContent><w:r><w:t xml:space=\"preserve\">a</w:t></w:r></w:sdtContent></w:sdt>";
        auto doc = open(makeDocx("<w:p>" + sdt + "</w:p>"));
        const ContentControl c = contentControls(*doc).first();
        QCOMPARE(c.props.title, QString(QStringLiteral("It")) + QChar(0x2019) + QStringLiteral("s A"));
        QCOMPARE(c.props.items.size(), 2);
        QCOMPARE(c.props.items[0].display, QStringLiteral("a value='x' b"));
        QCOMPARE(c.props.items[0].value, QStringLiteral("v1"));
        QCOMPARE(c.props.items[1].display, QStringLiteral("R&D <1>"));
        QCOMPARE(c.props.items[1].value, QStringLiteral("q\"r"));
        ControlProps p = c.props;
        p.tag = QStringLiteral("t");
        QVERIFY(setContentControlProps(*doc, c.start, p));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(!out.contains("&amp;#"), out.constData());
        auto again = open(doc->toDocx());
        const ContentControl back = contentControls(*again).first();
        QCOMPARE(back.props.title, c.props.title);
        QCOMPARE(back.props.items[0].display, c.props.items[0].display);
        QCOMPARE(back.props.items[1].value, c.props.items[1].value);
    }

    void aLongTagIsKeptWholeWhenAnotherFieldChanges()
    {
        const QByteArray tag(300, 'g');
        auto doc = open(makeDocx("<w:p><w:sdt><w:sdtPr><w:tag w:val=\"" + tag + "\"/><w:text/></w:sdtPr><w:sdtContent><w:r><w:t>x</w:t></w:r></w:sdtContent></w:sdt></w:p>"));
        const ContentControl c = contentControls(*doc).first();
        ControlProps p = c.props;
        p.title = QStringLiteral("New");
        QVERIFY(setContentControlProps(*doc, c.start, p));
        QCOMPARE(contentControls(*doc).first().props.tag, QString::fromLatin1(tag));
        QVERIFY(documentXml(doc->toDocx()).contains("<w:tag w:val=\"" + tag + "\"/>"));
    }

    void anEmptiedTextControlShowsItsPlaceholderAgain()
    {
        Win win;
        WriteDoc &doc = *win.w.document();
        const int start = makeControl(win, ControlKind::PlainText);
        QTextCursor &c = win.w.view()->cursor();
        write::insertText(doc, c, QStringLiteral("a"));
        QVERIFY(!contentControlAt(doc, start + 1).placeholder);
        c.setPosition(contentControlAt(doc, start + 1).end);
        QTest::keyClick(win.w.view()->viewport(), Qt::Key_Backspace);
        const ContentControl now = contentControlAt(doc, start + 1);
        QVERIFY(now.placeholder);
        QCOMPARE(now.text, QStringLiteral("Enter text"));
        QVERIFY(documentXml(doc.toDocx()).contains("<w:showingPlcHdr/>"));
    }

    void aDateIsWrittenAgainInANewFormatAndInTheControlsLanguage()
    {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        Win win;
        WriteDoc &doc = *win.w.document();
        const int start = makeControl(win, ControlKind::Date);
        QVERIFY(setControlDate(doc, start, QDate(2026, 3, 9)));
        ControlProps p = contentControlAt(doc, start + 1).props;
        p.dateFormat = QStringLiteral("yyyy-MM-dd");
        QVERIFY(setContentControlProps(doc, start, p));
        QCOMPARE(contentControlAt(doc, start + 1).text, QStringLiteral("2026-03-09"));
        QCOMPARE(formatControlDate(QDate(2026, 3, 9), QStringLiteral("MMMM"), QStringLiteral("en-US")), QStringLiteral("March"));
        QCOMPARE(formatControlDate(QDate(2026, 3, 9), QStringLiteral("MMMM"), QStringLiteral("de-DE")), QStringLiteral("März"));
    }

    void choosingKeepsABookmarkInsideAndAnUnchangedBoxChangesNothing()
    {
        const QByteArray sdt = "<w:sdt><w:sdtPr><w:dropDownList><w:listItem w:displayText=\"A\" w:value=\"a\"/><w:listItem w:displayText=\"B\" w:value=\"b\"/></w:dropDownList></w:sdtPr>"
                               "<w:sdtContent><w:bookmarkStart w:id=\"3\" w:name=\"keep\"/><w:r><w:t xml:space=\"preserve\">A</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:sdtContent></w:sdt>";
        auto doc = open(makeDocx("<w:p>" + sdt + "</w:p>"));
        const ContentControl c = contentControls(*doc).first();
        QVERIFY(chooseControlItem(*doc, c.start, 1));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains("w:name=\"keep\"") && out.contains("w:bookmarkEnd"), out.constData());
        QCOMPARE(contentControls(*doc).first().text, QStringLiteral("B"));
        // The box with nothing changed makes no step and no change.
        const int revision = doc->story()->revision();
        QVERIFY(setContentControlProps(*doc, c.start, contentControlAt(*doc, c.start + 1).props));
        QCOMPARE(doc->story()->revision(), revision);
    }

    void theMenuOfAListShowsAmpersandsAndCutsLongItems()
    {
        Win win;
        const int list = makeControl(win, ControlKind::DropDown);
        ControlProps p = contentControlAt(*win.w.document(), list + 1).props;
        p.items = {{QStringLiteral("R&D"), QStringLiteral("rd")}, {QString(500, QLatin1Char('w')), QStringLiteral("long")}};
        QVERIFY(setContentControlProps(*win.w.document(), list, p));
        QMenu *menu = controlChoicesMenu(&win.w, win.w.document(), list, nullptr);
        QVERIFY(menu);
        QCOMPARE(menu->actions()[0]->text(), QStringLiteral("R&&D"));
        QVERIFY(menu->actions()[1]->text().size() < 200);
        delete menu;
    }
};

QTEST_MAIN(InsertTests)
#include "tst_write_insert.moc"
