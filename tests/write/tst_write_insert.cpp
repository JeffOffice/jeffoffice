// Tests of JeffWrite's Insert, Design, Layout, and View tabs: symbols and special characters, the breaks, page setup presets, the page's look, and the views.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/navpane.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/textdialogs.h"
#include "core/items.h"
#include "flow/paginator.h"
#include "app/writeinfo.h"
#include "app/ribbon.h"
#include "app/ruler.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
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
#include "office/package.h"
#include "wml/wml.h"

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
            dlg->findChild<QLineEdit *>(QStringLiteral("address"))->setText(QStringLiteral("example.com/page"));
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

    void anEmailAddressGetsMailtoAndABadOneIsRefusedWithAReason()
    {
        QCOMPARE(write::normalizedAddress(QStringLiteral("a.b@example.com")), QStringLiteral("mailto:a.b@example.com"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("www.example.com")), QStringLiteral("https://www.example.com"));
        QCOMPARE(write::normalizedAddress(QStringLiteral("two words")), QString());
        Win win;
        QString why;
        QTextCursor &c = win.w.view()->cursor();
        QVERIFY(!write::insertHyperlink(*win.w.document(), c, QStringLiteral("file:///etc/passwd"), QString(), QString(), QStringLiteral("x"), &why));
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
        QVERIFY(!write::mayFollow(QStringLiteral("file:///etc/passwd")));
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
        const QByteArray file = makeLinkDocx({"https://good.example", "https://evil.example?x=1", "mailto:a@example.com", "tel:+15555550100", "file:///etc/passwd", "https://plain.example"}, [](const QStringList &ids) {
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
        // A link to a file is never followed, and the person is told.
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
};

QTEST_MAIN(InsertTests)
#include "tst_write_insert.moc"
