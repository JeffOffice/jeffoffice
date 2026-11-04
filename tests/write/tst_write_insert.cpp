// Tests of JeffWrite's Insert, Design, Layout, and View tabs: symbols and special characters, the breaks, page setup presets, the page's look, and the views.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/navpane.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/textdialogs.h"
#include "app/writeinfo.h"
#include "app/ribbon.h"
#include "app/ruler.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "core/insertops.h"
#include "core/layoutops.h"
#include "core/refops.h"
#include "core/linkops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "wml/wml.h"

#include <QFile>
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
        Q_EMIT win.w.view()->linkFollowed(QStringLiteral("https://example.com"));
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
};

QTEST_MAIN(InsertTests)
#include "tst_write_insert.moc"
