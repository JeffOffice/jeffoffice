// Tests of what the Layout, References, Review, and View tabs, the outline, and the right-click menus add.

#include "../isolatesettings.h"
#include "../withdialog.h"
#include "io/blocklibrary.h"
#include "app/help.h"
#include "core/version.h"
#include "app/writeoutlineview.h"
#include "app/pagepaint.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/stylearea.h"
#include "app/theme.h"
#include "app/ribbon.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/fonts.h"
#include "core/insertops.h"
#include "core/linkops.h"
#include "core/headerops.h"
#include "core/outlineops.h"
#include "core/pictureops.h"
#include "core/pagesetup.h"
#include "app/tabledialogs.h"
#include "app/writedialogs.h"
#include "core/proofops.h"
#include "core/tableops.h"
#include "core/textexport.h"
#include "core/writedoc.h"
#include "app/widgets.h"

#include "office/package.h"
#include <QAction>
#include <QListWidget>
#include <QTextBrowser>
#include <QToolButton>
#include <QUrlQuery>
#include <QApplication>
#include <QFrame>
#include <QMouseEvent>
#include <QInputMethodEvent>
#include <QBuffer>
#include <QImage>
#include <QRegularExpression>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QTimer>
#include <QDockWidget>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QSignalSpy>
#include <QMenu>
#include <QPair>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QSpinBox>
#include <QTreeWidget>

using namespace jp;
using namespace jp::write;

namespace {

// The ids (object names) of the commands in a menu and its submenus.
QStringList idsIn(const QMenu &menu)
{
    QStringList ids;
    for (QAction *a : menu.actions()) {
        if (a->menu()) ids += idsIn(*a->menu());
        else if (!a->isSeparator()) ids << a->objectName();
    }
    return ids;
}

QStringList titlesOf(const QMenu &menu)
{
    QStringList titles;
    for (QAction *a : menu.actions())
        if (a->menu()) titles << a->text();
    return titles;
}

QAction *find(const QMenu &menu, const QString &id)
{
    for (QAction *a : menu.actions()) {
        if (a->menu()) {
            if (QAction *in = find(*a->menu(), id)) return in;
        } else if (a->objectName() == id) {
            return a;
        }
    }
    return nullptr;
}

// A document of paragraphs: each text, with the heading level it is given (kBodyLevel for body text).
std::unique_ptr<WriteDoc> outlineDoc(const QList<QPair<QString, int>> &paragraphs)
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    bool first = true;
    for (const auto &p : paragraphs) {
        if (!first) insertParagraph(*doc, c);
        first = false;
        insertText(*doc, c, p.first);
        if (p.second != kBodyLevel) applyParagraphStyle(*doc, c, QStringLiteral("Heading%1").arg(p.second + 1));
    }
    return doc;
}

QStringList texts(WriteDoc &doc)
{
    QStringList out;
    for (const OutlineRow &r : outlineRows(doc)) out << r.text;
    return out;
}

QList<int> levels(WriteDoc &doc)
{
    QList<int> out;
    for (const OutlineRow &r : outlineRows(doc)) out << r.level;
    return out;
}

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

// A package of a document whose body is `body`, with `settings` (the inside of w:settings) in its settings part.
QByteArray makeDocx(const QByteArray &body, const QByteArray &settings)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QByteArray head("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n");
    w.addPart(QStringLiteral("/word/document.xml"), head + "<w:document " + kNs + "><w:body>" + body + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    if (!settings.isEmpty()) {
        w.addPart(QStringLiteral("/word/settings.xml"), head + "<w:settings " + kNs + ">" + settings + "</w:settings>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    return w.finish();
}

QByteArray settingsXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/settings.xml"));
}

// A narrow page of long words (so that many lines break inside a word when hyphenation is on).
std::unique_ptr<WriteDoc> narrowDoc(const QByteArray &settings)
{
    QByteArray text;
    for (int i = 0; i < 30; ++i) text += i % 3 == 0 ? "extraordinarily " : i % 3 == 1 ? "indistinguishable " : "unquestionably ";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"2400\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"720\" w:bottom=\"1440\" w:left=\"720\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    QString err;
    return WriteDoc::fromDocx(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>" + sect, settings), &err);
}

// A page wide enough for each word of a line, its words in capitals (so that a line ends inside a word only where the word is hyphenated).
std::unique_ptr<WriteDoc> capsDoc(const QByteArray &settings)
{
    QByteArray text;
    for (int i = 0; i < 30; ++i) text += i % 3 == 0 ? "EXTRAORDINARILY " : i % 3 == 1 ? "INDISTINGUISHABLE " : "UNQUESTIONABLY ";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"6000\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"720\" w:bottom=\"1440\" w:left=\"720\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    QString err;
    return WriteDoc::fromDocx(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>" + sect, settings), &err);
}

QByteArray bluePng()
{
    QImage image(40, 20, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return png;
}

// The positions of the pictures of the story, in order.
QList<int> picturePositions(WriteDoc &doc)
{
    QList<int> out;
    for (int p = 0; p + 1 < doc.story()->characterCount(); ++p)
        if (isPictureAt(doc.story(), p)) out << p;
    return out;
}

// The heights (w:relativeHeight) of the pictures the file holds, in the order of the text.
QList<qint64> pictureHeights(const QByteArray &docx)
{
    office::Package p;
    p.read(docx);
    const QString xml = QString::fromUtf8(p.part(QStringLiteral("/word/document.xml")));
    QList<qint64> out;
    auto it = QRegularExpression(QStringLiteral("relativeHeight=\"([0-9]+)\"")).globalMatch(xml);
    while (it.hasNext()) out << it.next().captured(1).toLongLong();
    return out;
}

// The longest run of lines in a row that end inside a word (a hyphenated break), and how many lines there are.
QPair<int, int> hyphenRun(WriteDoc &doc)
{
    int run = 0, most = 0;
    const auto lines = doc.pager().lines(0);
    for (const auto &l : lines) {
        const QChar last = l.end > 0 ? doc.story()->characterAt(l.end - 1) : QChar(), next = doc.story()->characterAt(l.end);
        if (last.isLetter() && next.isLetter()) most = std::max(most, ++run);
        else run = 0;
    }
    return {most, int(lines.size())};
}

} // namespace

// ---- the style area (Draft and Outline views)
namespace {
const QByteArray kAreaStyles =
    "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Quote1\"><w:name w:val=\"Quote Style\"/><w:basedOn w:val=\"Normal\"/></w:style>"
    "<w:style w:type=\"paragraph\" w:styleId=\"Long1\"><w:name w:val=\"An unusually long style name that cannot fit in a narrow strip\"/><w:basedOn w:val=\"Normal\"/></w:style>";

QByteArray areaPara(const char *style, const QString &text, const QByteArray &more = QByteArray())
{
    return "<w:p><w:pPr><w:pStyle w:val=\"" + QByteArray(style) + "\"/>" + more + "</w:pPr><w:r><w:t>" + text.toUtf8() + "</w:t></w:r></w:p>";
}

QByteArray areaCell(const QByteArray &paragraphs) { return "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>" + paragraphs + "</w:tc>"; }

std::unique_ptr<WriteDoc> areaDoc(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QString main = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.");
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>", main + QStringLiteral("document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), QByteArray("<w:styles ") + kNs + ">" + kAreaStyles + "</w:styles>", main + QStringLiteral("styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    QString err;
    return WriteDoc::fromDocx(w.finish(), &err);
}

// The window with the document, in Draft view, with the style area `width` wide.
void draftWithStyleArea(WriteWindow &w, std::unique_ptr<WriteDoc> doc, int width)
{
    w.setNotifier([](const QString &, const QString &) {});
    w.resize(900, 700);
    w.setDocument(std::move(doc), QString());
    w.show();
    w.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&w));
    w.view()->setStyleAreaWidth(width);
    w.act(QStringLiteral("view.draft"))->trigger();
}

// Answers the dialog that opens (a modal one), looking every 20 ms for up to 400 tries: its title, and the name its box has, are noted, and it is closed.
struct DialogSeen {
    QString title, name;
    int tries = 0;
    bool seen = false;
};
void closeNextDialog(QObject *owner, DialogSeen *seen)
{
    auto *timer = new QTimer(owner);
    timer->setInterval(20);
    QObject::connect(timer, &QTimer::timeout, timer, [timer, seen] {
        if (auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            seen->seen = true;
            seen->title = dlg->windowTitle();
            if (auto *name = dlg->findChild<QLineEdit *>(QStringLiteral("styleName"))) seen->name = name->text();
            timer->stop();
            dlg->reject();
            timer->deleteLater();
        } else if (++seen->tries > 400) {
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start();
}

// A row of the strip as a click on it would reach it (the middle of its first line).
QPoint areaRowPoint(const StyleRow &row) { return QPoint(8, row.top + std::max(1, row.height / 2)); }

// Settles the event loop for a counted number of turns (what the strip schedules for later is done).
// Puts the style area's width setting back to hidden when a test ends, whichever way it ends.
struct StyleWidthReset {
    ~StyleWidthReset() { Settings::get().setValue(QStringLiteral("view/styleAreaWidth"), 0); }
};

void settleTurns(int turns = 12)
{
    for (int i = 0; i < turns; ++i) QCoreApplication::processEvents();
}
} // namespace

class Tabs2Tests : public QObject {
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

    void theMenuOnTextHasPasteOptionsAndLists()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QTextCursor c = w.view()->cursor();
        insertText(*w.document(), c, QStringLiteral("plain words"));
        w.view()->setCursor(c);
        w.refreshNow();
        QMenu menu;
        w.fillContextMenu(menu);
        const QStringList ids = idsIn(menu);
        QVERIFY(titlesOf(menu).contains(QStringLiteral("Paste Options")));
        for (const char *id : {"edit.paste", "edit.pasteMerge", "edit.pasteText", "para.bullets", "para.numbers", "insert.hyperlink"}) QVERIFY2(ids.contains(QString::fromLatin1(id)), id);
        // Away from a link, a field, and a note, the menu has nothing for them.
        for (const char *id : {"ctx.openLink", "ctx.editLink", "insert.removeHyperlink", "fields.update", "fields.toggleCode", "ctx.goToNote"}) QVERIFY2(!ids.contains(QString::fromLatin1(id)), id);
    }

    void theMenuOnALinkOpensEditsAndRemovesIt()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QStringList opened;
        w.setLinkOpener([&](const QString &address) { opened << address; });
        w.setLinkAsker([](const QString &) { return true; });
        QTextCursor c = w.view()->cursor();
        QVERIFY(insertHyperlink(*w.document(), c, QStringLiteral("https://example.com/a"), QString(), QString(), QStringLiteral("the site")));
        QTextCursor in(w.document()->story());
        in.setPosition(3);
        w.view()->setCursor(in);
        w.refreshNow();
        QMenu menu;
        w.fillContextMenu(menu);
        for (const char *id : {"ctx.openLink", "ctx.editLink", "insert.removeHyperlink"}) QVERIFY2(find(menu, QString::fromLatin1(id)), id);
        QVERIFY(!find(menu, QStringLiteral("insert.hyperlink")));
        find(menu, QStringLiteral("ctx.openLink"))->trigger();
        QCOMPARE(opened, QStringList{QStringLiteral("https://example.com/a")});
        find(menu, QStringLiteral("insert.removeHyperlink"))->trigger();
        QVERIFY(!hyperlinkAt(*w.document(), 3).valid());
    }

    void theMenuOnAFieldUpdatesAndShowsItsCode()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QTextCursor c = w.view()->cursor();
        QVERIFY(insertField(*w.document(), c, QStringLiteral("= 2 + 3")));
        QTextCursor in(w.document()->story());
        in.setPosition(1);
        w.view()->setCursor(in);
        w.refreshNow();
        QMenu menu;
        w.fillContextMenu(menu);
        for (const char *id : {"fields.update", "fields.toggleCode", "fields.lock", "fields.unlink"}) QVERIFY2(find(menu, QString::fromLatin1(id)), id);
        QVERIFY(find(menu, QStringLiteral("fields.update"))->isEnabled());
    }

    void theMenuOnAFootnoteMarkGoesToItsNote()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QTextCursor c = w.view()->cursor();
        insertText(*w.document(), c, QStringLiteral("body"));
        QVERIFY(insertFootnote(*w.document(), c));
        QTextCursor on(w.document()->story());
        on.setPosition(4);
        w.view()->setCursor(on);
        w.refreshNow();
        QMenu menu;
        w.fillContextMenu(menu);
        QAction *go = find(menu, QStringLiteral("ctx.goToNote"));
        QVERIFY(go);
        go->trigger();
        QTRY_VERIFY(w.view()->inNote());
    }

    // ---------------------------------------------------------------- Outline view
    void outlineRowsListEachParagraphWithItsLevel()
    {
        auto doc = outlineDoc({{"One", 0}, {"body", kBodyLevel}, {"Two", 1}});
        const QVector<OutlineRow> rows = outlineRows(*doc);
        QCOMPARE(rows.size(), 3);
        QCOMPARE(levels(*doc), (QList<int>{0, kBodyLevel, 1}));
        QCOMPARE(texts(*doc), (QStringList{"One", "body", "Two"}));
        QCOMPARE(outlineSubtreeEnd(rows, 0), 3);
        QCOMPARE(outlineSubtreeEnd(rows, 1), 2);
        QCOMPARE(outlineSubtreeEnd(rows, 2), 3);
    }

    void promotingAndDemotingMovesAHeadingAndWhatIsUnderIt()
    {
        auto doc = outlineDoc({{"A", 0}, {"B", 1}, {"C", 2}, {"body", kBodyLevel}, {"D", 1}});
        const QVector<OutlineRow> rows = outlineRows(*doc);
        // Demote B with its subtree: C goes down too, body text stays body text, D (a sibling) is not touched.
        QVERIFY(outlineShift(*doc, rows[1].position, rows[1].position, 1, true));
        QCOMPARE(levels(*doc), (QList<int>{0, 2, 3, kBodyLevel, 1}));
        doc->undo();
        QCOMPARE(levels(*doc), (QList<int>{0, 1, 2, kBodyLevel, 1}));
        // Without the subtree only B moves.
        QVERIFY(outlineShift(*doc, rows[1].position, rows[1].position, 1, false));
        QCOMPARE(levels(*doc), (QList<int>{0, 2, 2, kBodyLevel, 1}));
        // Promote Heading 1: nothing changes; Heading 9 demoted is body text; body text promoted takes the heading above's level.
        QVERIFY(!outlineShift(*doc, rows[0].position, rows[0].position, -1, false));
        QVERIFY(outlineShift(*doc, rows[3].position, rows[3].position, -1, false));
        QCOMPARE(levels(*doc).at(3), 2);
        QVERIFY(outlineSetLevel(*doc, rows[2].position, rows[2].position, 8));
        QVERIFY(outlineShift(*doc, rows[2].position, rows[2].position, 1, false));
        QCOMPARE(levels(*doc).at(2), kBodyLevel);
        QVERIFY(outlineSetLevel(*doc, rows[0].position, rows[1].position, kBodyLevel));
        QCOMPARE(levels(*doc).mid(0, 2), (QList<int>{kBodyLevel, kBodyLevel}));
    }

    void movingASectionUpAndDownKeepsItsWholeText()
    {
        auto doc = outlineDoc({{"A", 0}, {"a body", kBodyLevel}, {"B", 0}, {"b sub", 1}, {"b body", kBodyLevel}, {"C", 0}});
        QVector<OutlineRow> rows = outlineRows(*doc);
        QVERIFY(outlineMove(*doc, rows[2].position, true, true));   // B above A
        QCOMPARE(texts(*doc), (QStringList{"B", "b sub", "b body", "A", "a body", "C"}));
        QCOMPARE(levels(*doc), (QList<int>{0, 1, kBodyLevel, 0, kBodyLevel, 0}));
        doc->undo();
        QCOMPARE(texts(*doc), (QStringList{"A", "a body", "B", "b sub", "b body", "C"}));
        rows = outlineRows(*doc);
        QVERIFY(outlineMove(*doc, rows[2].position, false, true));  // B below C, the last paragraph
        QCOMPARE(texts(*doc), (QStringList{"A", "a body", "C", "B", "b sub", "b body"}));
        QCOMPARE(levels(*doc), (QList<int>{0, kBodyLevel, 0, 0, 1, kBodyLevel}));
        rows = outlineRows(*doc);
        QVERIFY(!outlineMove(*doc, rows[5].position, false, true));  // (nothing below the last)
        QVERIFY(!outlineMove(*doc, rows[0].position, true, true));
        // The last section to the top.
        {
            auto d2 = outlineDoc({{"One", 0}, {"text under one", kBodyLevel}, {"Two", 1}, {"Three", 0}});
            QVERIFY(outlineMove(*d2, outlineRows(*d2).at(3).position, true, true));
            QCOMPARE(texts(*d2), (QStringList{"Three", "One", "text under one", "Two"}));
            QCOMPARE(levels(*d2), (QList<int>{0, 0, kBodyLevel, 1}));
        }
        // A paragraph alone moves one place.
        QVERIFY(outlineMove(*doc, rows[1].position, false, false));
        QCOMPARE(texts(*doc).mid(0, 3), (QStringList{"A", "C", "a body"}));
    }

    void theOutlineViewReplacesThePagesAndTheOutliningTabComesWithIt()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.act("view.outline") && w.act("outline.promote") && w.act("outline.moveUp"));
        QVERIFY(!w.act("outline.promote")->isEnabled());
        QVERIFY(w.act("view.printLayout")->isChecked());
        w.setDocument(outlineDoc({{"One", 0}, {"text under one", kBodyLevel}, {"Two", 1}, {"Three", 0}}), QString());
        w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w.outlineMode());
        QVERIFY(w.outlineView()->isVisible());
        QVERIFY(!w.view()->isVisible());
        QVERIFY(w.act("outline.promote")->isEnabled());
        QVERIFY(!w.act("view.printLayout")->isChecked());
        QVERIFY(w.ribbon()->tab(QStringLiteral("Outlining")));
        QCOMPARE(w.outlineView()->shownRows(), (QStringList{"One", "text under one", "Two", "Three"}));
        // Show Level 1: the headings of the first level only; a heading that has more under it is collapsed.
        w.act(QStringLiteral("outline.level.1"))->trigger();
        QCOMPARE(w.outlineView()->shownRows(), (QStringList{"One", "Three"}));
        w.act(QStringLiteral("outline.level.2"))->trigger();
        QCOMPARE(w.outlineView()->shownRows(), (QStringList{"One", "Two", "Three"}));
        w.act(QStringLiteral("outline.level.all"))->trigger();
        // Demote "Two" and move "Three" up past "One" ("One" holds the text and "Two").
        w.outlineView()->selectParagraphAt(outlineRows(*w.document()).at(3).position);
        w.act(QStringLiteral("outline.moveUp"))->trigger();
        QCOMPARE(texts(*w.document()), (QStringList{"Three", "One", "text under one", "Two"}));
        QCOMPARE(w.outlineView()->shownRows(), texts(*w.document()));
        w.document()->undo();
        QCOMPARE(w.outlineView()->shownRows(), (QStringList{"One", "text under one", "Two", "Three"}));
        w.outlineView()->selectParagraphAt(outlineRows(*w.document()).at(2).position);
        w.act(QStringLiteral("outline.demote"))->trigger();
        QCOMPARE(levels(*w.document()), (QList<int>{0, kBodyLevel, 2, 0}));
        w.act(QStringLiteral("outline.toHeading1"))->trigger();
        QCOMPARE(levels(*w.document()).at(2), 0);
        // Back to the pages: the caret is at the paragraph that was selected.
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QVERIFY(!w.outlineMode());
        QVERIFY(w.view()->isVisible() && !w.outlineView()->isVisible());
        QVERIFY(!w.act("outline.promote")->isEnabled());
        QCOMPARE(w.view()->cursor().position(), outlineRows(*w.document()).at(2).position);
    }

    void theOutlineViewFillsTheWindowWhereThePagesWere()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(outlineDoc({{"One", 0}, {"text under one", kBodyLevel}, {"Two", 1}, {"Three", 0}}), QString());
        w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w.outlineMode());
        QCoreApplication::processEvents();
        // (The outline takes all the room below the ribbon, not half of it with an empty half above it.)
        QWidget *central = w.centralWidget();
        QVERIFY2(w.outlineView()->geometry().top() < 20, qPrintable(QStringLiteral("the outline starts %1 pixels down").arg(w.outlineView()->geometry().top())));
        QVERIFY(w.outlineView()->geometry().height() > central->height() - 20);
        // And back: the pages have all of it again.
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QCoreApplication::processEvents();
        QVERIFY(w.view()->height() > central->height() - w.view()->geometry().top() - 40);
    }


    // ---------------------------------------------------------------- View
    void pagesCanBeSetSideToSideInOneRow()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        QVERIFY(w.act("view.pageMovement.vertical")->isChecked() && !w.act("view.pageMovement.sideToSide")->isChecked());
        QTextCursor c = w.view()->cursor();
        insertBlankPage(*w.document(), c);
        insertBlankPage(*w.document(), c);
        w.view()->paginateAll();
        QVERIFY(w.view()->pageCount() >= 3);
        w.view()->setZoom(0.5);
        // In a column, each page is below the one before.
        QVERIFY(w.view()->pageRect(1).top() > w.view()->pageRect(0).bottom() - 1);
        w.act(QStringLiteral("view.pageMovement.sideToSide"))->trigger();
        QVERIFY(w.view()->sideToSide());
        QCOMPARE(w.view()->pageRect(1).top(), w.view()->pageRect(0).top());
        QCOMPARE(w.view()->pageRect(2).top(), w.view()->pageRect(0).top());
        QVERIFY(w.view()->pageRect(1).left() > w.view()->pageRect(0).right());
        QVERIFY(w.view()->pageRect(2).left() > w.view()->pageRect(1).right());
        // Going to a page scrolls across to it.
        w.view()->goToPage(2);
        QVERIFY(w.view()->horizontalScrollBar()->value() > 0);
        QCOMPARE(w.view()->topPage(), 2);
        w.act(QStringLiteral("view.pageMovement.vertical"))->trigger();
        QVERIFY(!w.view()->sideToSide());
        QVERIFY(w.view()->pageRect(1).top() > w.view()->pageRect(0).bottom() - 1);
        QCOMPARE(w.view()->horizontalScrollBar()->value(), 0);
    }

    // ---------------------------------------------------------------- Help
    void theHelpTabHasHelpShortcutsWhatsNewAndFeedback()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.ribbon()->tab(QStringLiteral("Help")));
        const auto saved = help::openUrl;
        QList<QUrl> opened;
        help::openUrl = [&](const QUrl &u) {
            opened << u;
            return true;
        };
        w.act(QStringLiteral("help.feedback"))->trigger();
        w.act(QStringLiteral("help.whatsNew"))->trigger();
        help::openUrl = saved;
        QCOMPARE(opened, (QList<QUrl>{help::feedbackUrl(), help::whatsNewUrl()}));
        QDockWidget *dock = w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
        QVERIFY(!dock || !dock->isVisible());
        w.act(QStringLiteral("help.shortcuts"))->trigger();
        dock = w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
        QVERIFY(dock && dock->isVisible());
    }

    void theHelpPaneSearchesFollowsLinksAndGoesBackAndForward()
    {
        WriteWindow w;
        w.resize(1300, 800);
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.act(QStringLiteral("help.show"))->trigger();
        jp::HelpView *v = w.helpView();
        QVERIFY(v && v->isVisible());
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        auto texts = [v] {
            QStringList t;
            for (int i = 0; i < v->results()->count(); ++i) t << v->results()->item(i)->text();
            return t;
        };
        auto button = [v](const QString &name) {
            for (auto *b : v->findChildren<QToolButton *>())
                if (b->accessibleName() == name) return b;
            return static_cast<QToolButton *>(nullptr);
        };
        // The search finds topics and commands, and says so when it finds nothing.
        v->search(QStringLiteral("mail merge"));
        QVERIFY(v->results()->isVisible() && !v->browser()->isVisible());
        const QStringList found = texts();
        QVERIFY2(found.contains(QStringLiteral("Help topics")) && found.contains(QStringLiteral("Commands")), qPrintable(found.join(QStringLiteral(" / "))));
        v->search(QStringLiteral("zzqxv"));
        QCOMPARE(texts(), QStringList{QStringLiteral("Nothing found. Try other words, or open the contents.")});
        // Enter opens the first topic found, and the topic shows in place of the results.
        v->search(QStringLiteral("mail merge"));
        QTest::keyClick(v->searchBox(), Qt::Key_Return);
        QCOMPARE(v->currentTopic(), QStringLiteral("mail-merge"));
        QVERIFY(v->browser()->isVisible() && v->searchBox()->text().isEmpty());
        // A link goes to its topic; Back and Forward go between the topics; Contents goes home.
        QVERIFY(!button(QStringLiteral("Forward"))->isEnabled());
        Q_EMIT v->browser()->anchorClicked(QUrl(QStringLiteral("fields.md")));
        QCOMPARE(v->currentTopic(), QStringLiteral("fields"));
        QVERIFY(button(QStringLiteral("Back"))->isEnabled());
        button(QStringLiteral("Back"))->click();
        QCOMPARE(v->currentTopic(), QStringLiteral("mail-merge"));
        QVERIFY(button(QStringLiteral("Forward"))->isEnabled());
        button(QStringLiteral("Forward"))->click();
        QCOMPARE(v->currentTopic(), QStringLiteral("fields"));
        button(QStringLiteral("Contents"))->click();
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        // A web link goes to the browser, not to a topic. Feedback carries the program's name and version.
        const auto saved = help::openUrl;
        QList<QUrl> opened;
        help::openUrl = [&](const QUrl &u) {
            opened << u;
            return true;
        };
        Q_EMIT v->browser()->anchorClicked(QUrl(QStringLiteral("https://example.com/page")));
        w.act(QStringLiteral("help.feedback"))->trigger();
        help::openUrl = saved;
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        QCOMPARE(opened.size(), 2);
        QCOMPARE(opened[0], QUrl(QStringLiteral("https://example.com/page")));
        QCOMPARE(opened[1].scheme(), QStringLiteral("https"));
        QCOMPARE(opened[1].host(), QStringLiteral("github.com"));
        QVERIFY2(opened[1].path().endsWith(QStringLiteral("/issues/new")), qPrintable(opened[1].path()));
        QVERIFY(QUrlQuery(opened[1]).queryItemValue(QStringLiteral("body"), QUrl::FullyDecoded).contains(QStringLiteral("JeffWrite ") + jp::versionText() + QStringLiteral(" on ")));
    }

    // The Keyboard Shortcuts topic holds a table made from the commands: each row is a real command with the keys it really has.
    void theKeyboardShortcutsTableIsMadeFromTheCommands()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QSet<QString> real;
        for (const QString &id : w.actionIds())
            if (QAction *a = w.act(id); a && !a->shortcuts().isEmpty()) {
                QStringList keys;
                for (const QKeySequence &k : a->shortcuts()) keys << k.toString(QKeySequence::NativeText);
                QString name = a->text();
                name.remove(QLatin1Char('&'));
                if (name.endsWith(QStringLiteral("..."))) name.chop(3);
                real.insert(name.simplified() + QLatin1Char('|') + keys.join(QStringLiteral(", ")));
            }
        const QString md = help::displayMarkdown(help::topic(QStringLiteral("keyboard")), &w);
        QVERIFY(md.contains(QStringLiteral("|---|---|---|")));
        static const QRegularExpression bar(QStringLiteral("(?<!\\\\)\\|"));
        int rows = 0;
        QStringList wrong;
        bool inTable = false;
        for (const QString &line : md.split(QLatin1Char('\n'))) {
            if (line.startsWith(QStringLiteral("|---"))) {
                inTable = true;
                continue;
            }
            if (!inTable) continue;
            if (!line.startsWith(QLatin1Char('|'))) break;
            const QStringList cells = line.split(bar);
            QVERIFY2(cells.size() >= 4, qPrintable(line));
            ++rows;
            const QString name = QString(cells[1]).replace(QStringLiteral("\\|"), QStringLiteral("|")).trimmed(), keys = QString(cells[2]).replace(QStringLiteral("\\|"), QStringLiteral("|")).trimmed();
            if (keys.isEmpty() || !real.contains(name + QLatin1Char('|') + keys)) wrong << line;
        }
        QVERIFY2(wrong.isEmpty(), qPrintable(wrong.join(QLatin1Char('\n'))));
        QVERIFY(rows >= 20);
        QVERIFY(md.contains(QStringLiteral("| Bold | ") + w.act(QStringLiteral("fmt.bold"))->shortcut().toString(QKeySequence::NativeText) + QStringLiteral(" |")));
    }

    void aFieldsCodeCanBeReadAndChanged()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        insertText(*doc, c, QStringLiteral("before "));
        QVERIFY(insertField(*doc, c, QStringLiteral("= 2 + 3")));
        insertText(*doc, c, QStringLiteral(" after"));
        const int at = 8;
        QCOMPARE(fieldCodeAt(*doc, at).trimmed(), QStringLiteral("= 2 + 3"));
        QCOMPARE(fieldCodeAt(*doc, 2), QString());   // (no field there)
        QVERIFY(replaceFieldCode(*doc, at, QStringLiteral("= 4 * 5")));
        QCOMPARE(fieldCodeAt(*doc, at).trimmed(), QStringLiteral("= 4 * 5"));
        QCOMPARE(doc->story()->toPlainText().remove(QChar(0xFFFC)).left(7), QStringLiteral("before "));
        QVERIFY(doc->story()->toPlainText().contains(QStringLiteral(" after")));
        // One step undoes it.
        doc->undo();
        QCOMPARE(fieldCodeAt(*doc, at).trimmed(), QStringLiteral("= 2 + 3"));
        // A locked field is left as it is.
        QTextCursor all(doc->story());
        all.select(QTextCursor::Document);
        lockFields(*doc, all, true);
        QVERIFY(!replaceFieldCode(*doc, at, QStringLiteral("= 1")));
        QVERIFY(!replaceFieldCode(*doc, 2, QStringLiteral("= 1")));
    }

    void theMenuOnAFieldEditsItsCode()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QTextCursor c = w.view()->cursor();
        QVERIFY(insertField(*w.document(), c, QStringLiteral("= 2 + 3")));
        QTextCursor in(w.document()->story());
        in.setPosition(1);
        w.view()->setCursor(in);
        w.refreshNow();
        QMenu menu;
        w.fillContextMenu(menu);
        QVERIFY(find(menu, QStringLiteral("ctx.editField")));
        QString shown;
        w.setFieldCodeAsker([&](const QString &now) {
            shown = now;
            return QStringLiteral("= 7 * 6");
        });
        find(menu, QStringLiteral("ctx.editField"))->trigger();
        QCOMPARE(shown.trimmed(), QStringLiteral("= 2 + 3"));
        QCOMPARE(fieldCodeAt(*w.document(), 1).trimmed(), QStringLiteral("= 7 * 6"));
    }

    void readModeFitsTheCommandsOfFocusAndLeavesTheTextAsItWas()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        QTextCursor c = w.view()->cursor();
        insertText(*w.document(), c, QStringLiteral("some words"));
        const QString before = w.document()->story()->toPlainText();
        QVERIFY(!w.readMode() && !w.act("view.readMode")->isChecked());
        w.act(QStringLiteral("view.readMode"))->trigger();
        QVERIFY(w.readMode() && w.focusMode());
        QVERIFY(w.view()->readOnly() && w.view()->sideToSide());
        QCOMPARE(w.view()->zoomMode(), PageView::ZoomMode::OnePage);
        QTest::keyClick(w.view(), Qt::Key_X);
        QCOMPARE(w.document()->story()->toPlainText(), before);
        // Focus's own command ends it, and the window is as it was.
        w.act(QStringLiteral("view.focus"))->trigger();
        QVERIFY(!w.readMode() && !w.focusMode());
        QVERIFY(!w.view()->readOnly() && !w.view()->sideToSide());
        QVERIFY(!w.act("view.readMode")->isChecked());
        QCOMPARE(w.view()->zoomMode(), PageView::ZoomMode::Percent);
        QCOMPARE(w.view()->zoom(), 1.0);
        // Again, and ended by the command itself.
        w.act(QStringLiteral("view.readMode"))->trigger();
        QVERIFY(w.readMode());
        w.act(QStringLiteral("view.readMode"))->trigger();
        QVERIFY(!w.readMode() && !w.focusMode());
    }

    void theStatusBarShowsTheSectionWhereThereAreSeveral()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QLabel *label = w.findChild<QLabel *>(QStringLiteral("sectionNumber"));
        QVERIFY(label);
        w.refreshNow();
        QVERIFY(!label->isVisible());   // (one section: nothing to tell)
        QTextCursor c = w.view()->cursor();
        insertText(*w.document(), c, QStringLiteral("first"));
        insertSectionBreak(*w.document(), c);
        insertText(*w.document(), c, QStringLiteral("second"));
        w.view()->setCursor(c);
        w.refreshNow();
        QVERIFY(label->isVisible());
        QCOMPARE(label->text(), QStringLiteral("Section 2"));
        QTextCursor top(w.document()->story());
        top.setPosition(1);
        w.view()->setCursor(top);
        w.refreshNow();
        QCOMPARE(label->text(), QStringLiteral("Section 1"));
    }

    void movingASectionKeepsItsListAndItsTable()
    {
        auto doc = outlineDoc({{"A", 0}, {"x1", kBodyLevel}, {"x2", kBodyLevel}, {"after list", kBodyLevel}, {"B", 0}, {"b text", kBodyLevel}});
        QVector<OutlineRow> rows = outlineRows(*doc);
        QTextCursor c(doc->story());
        c.setPosition(rows[1].position);
        c.setPosition(rows[2].position, QTextCursor::KeepAnchor);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        toggleList(*doc, c, false);
        QTextCursor t(doc->story());
        t.setPosition(rows[3].position);
        QVERIFY(insertTable(*doc, t, NewTable()));
        auto look = [&] {
            QStringList out;
            int cells = 0;
            for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
                if (inTable(b)) ++cells;
                out << QStringLiteral("%1:%2").arg(b.text().remove(QChar(0xFFFC))).arg(int(listKindOf(*doc, b)));
            }
            return QStringList{out.join('|'), QString::number(cells)};
        };
        const QStringList before = look();
        const QByteArray bytes = doc->toDocx();
        QVERIFY(before[1].toInt() >= 4);
        rows = outlineRows(*doc);
        QVERIFY(outlineMove(*doc, rows[0].position, false, true));   // A, with its list and table, below B
        QStringList after = look();
        QCOMPARE(after[1], before[1]);
        QVERIFY(after[0].startsWith(QStringLiteral("B:0")));
        QVERIFY(after[0].contains(QStringLiteral("x1:1|x2:1")));
        QVERIFY(after[0].contains(QStringLiteral("A:0")));
        QCOMPARE(outlineRows(*doc).first().text, QStringLiteral("B"));
        doc->undo();
        QCOMPARE(look(), before);
        QCOMPARE(doc->toDocx(), bytes);
        // Up as well: B above everything.
        rows = outlineRows(*doc);
        QVERIFY(outlineMove(*doc, rows.at(rows.size() - 2).position, true, true));
        QCOMPARE(look()[1], before[1]);
        QVERIFY(look()[0].contains(QStringLiteral("x1:1|x2:1")));
        doc->undo();
        QCOMPARE(doc->toDocx(), bytes);
    }

    void aParagraphsWordsCanBeChangedFromTheOutline()
    {
        auto doc = outlineDoc({{"One", 0}, {"some body", kBodyLevel}, {"Two", 1}});
        QVector<OutlineRow> rows = outlineRows(*doc);
        QVERIFY(rows[0].editable && rows[1].editable);
        QVERIFY(outlineSetText(*doc, rows[0].position, QStringLiteral("First")));
        QCOMPARE(texts(*doc), (QStringList{"First", "some body", "Two"}));
        QCOMPARE(levels(*doc), (QList<int>{0, kBodyLevel, 1}));   // (the heading stays a heading)
        QVERIFY(outlineSetText(*doc, outlineRows(*doc)[1].position, QStringLiteral("")));
        QCOMPARE(texts(*doc), (QStringList{"First", "", "Two"}));
        QVERIFY(outlineSetText(*doc, outlineRows(*doc)[2].position, QStringLiteral("Second\nline")));   // (a line break is a space)
        QCOMPARE(texts(*doc).last(), QStringLiteral("Second line"));
        doc->undo();
        QCOMPARE(texts(*doc).last(), QStringLiteral("Two"));
        // A paragraph that holds a field is left as it is.
        QTextCursor c(doc->story());
        c.setPosition(outlineRows(*doc)[1].position);
        QVERIFY(insertField(*doc, c, QStringLiteral("= 2 + 3")));
        rows = outlineRows(*doc);
        QVERIFY(!rows[1].editable);
        QVERIFY(!outlineSetText(*doc, rows[1].position, QStringLiteral("x")));
    }

    void theOutlineViewTakesTypedWordsIntoTheDocument()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(outlineDoc({{"One", 0}, {"body", kBodyLevel}}), QString());
        w.setOutlineMode(true);
        QTreeWidgetItem *item = w.outlineView()->topLevelItem(0);
        QVERIFY(item && (item->flags() & Qt::ItemIsEditable));
        item->setText(0, QStringLiteral("Renamed"));   // (what the editing box does when it is accepted)
        QCOMPARE(texts(*w.document()).first(), QStringLiteral("Renamed"));
        QCOMPARE(levels(*w.document()).first(), 0);
        w.document()->undo();
        QCOMPARE(w.outlineView()->topLevelItem(0)->text(0), QStringLiteral("One"));
    }

    void aNewParagraphInTheOutlineHasTheLevelOfTheOneBefore()
    {
        auto doc = outlineDoc({{"One", 1}, {"body", kBodyLevel}, {"Two", 0}});
        QVector<OutlineRow> rows = outlineRows(*doc);
        int at = -1;
        QVERIFY(outlineInsertAfter(*doc, rows[0].position, &at));
        QCOMPARE(texts(*doc), (QStringList{"One", "", "body", "Two"}));
        QCOMPARE(levels(*doc), (QList<int>{1, 1, kBodyLevel, 0}));
        QCOMPARE(at, outlineRows(*doc)[1].position);
        QVERIFY(outlineInsertAfter(*doc, outlineRows(*doc)[2].position, &at));
        QCOMPARE(levels(*doc), (QList<int>{1, 1, kBodyLevel, kBodyLevel, 0}));
        doc->undo();
        QCOMPARE(texts(*doc).size(), 4);
        QCOMPARE(levels(*doc), (QList<int>{1, 1, kBodyLevel, 0}));
    }

    void enterTabAndShiftTabWorkInTheOutline()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        w.setDocument(outlineDoc({{"One", 0}, {"Sub", 1}, {"body", kBodyLevel}}), QString());
        w.setOutlineMode(true);
        w.outlineView()->selectParagraphAt(outlineRows(*w.document())[1].position);
        QTest::keyClick(w.outlineView(), Qt::Key_Tab);   // demote Sub (it has body text under it, which stays body text)
        QCOMPARE(levels(*w.document()), (QList<int>{0, 2, kBodyLevel}));
        QTest::keyClick(w.outlineView(), Qt::Key_Backtab, Qt::ShiftModifier);
        QCOMPARE(levels(*w.document()), (QList<int>{0, 1, kBodyLevel}));
        QTest::keyClick(w.outlineView(), Qt::Key_Return);
        QCOMPARE(texts(*w.document()), (QStringList{"One", "Sub", "", "body"}));
        QCOMPARE(levels(*w.document()).at(2), 1);
        QCOMPARE(w.outlineView()->selectedRange().first, outlineRows(*w.document())[2].position);
    }

    // Dragging the symbol of a heading moves its section (with real press, move, and release events): a line shows where it will go, and one step of Undo puts it back. A press on the words only
    // selects, and a drag that ends where it began, or inside the section itself, moves nothing.
    void draggingAHeadingSymbolMovesItsSection()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        w.setDocument(outlineDoc({{"One", 0}, {"body", kBodyLevel}, {"Two", 0}, {"Three", 0}}), QString());
        w.setOutlineMode(true);
        WriteOutlineView *view = w.outlineView();
        QWidget *port = view->viewport();
        auto item = [&](int row) {
            QTreeWidgetItem *it = view->topLevelItem(0);
            for (int i = 0; i < row && it; ++i) it = view->itemBelow(it);
            return it;
        };
        auto send = [&](QEvent::Type type, QPoint at, Qt::MouseButton button, Qt::MouseButtons held) {
            QMouseEvent e(type, at, port->mapToGlobal(at), button, held, Qt::NoModifier);
            QApplication::sendEvent(port, &e);
        };
        auto drag = [&](int fromRow, int toRow, bool below, bool onSymbol = true) {
            const QRect from = view->visualItemRect(item(fromRow));
            const QRect to = view->visualItemRect(item(toRow));
            const QPoint start(from.left() + (onSymbol ? 4 : from.width() / 2), from.center().y());
            const QPoint end(to.left() + 30, below ? to.bottom() - 2 : to.top() + 2);
            send(QEvent::MouseButtonPress, start, Qt::LeftButton, Qt::LeftButton);
            send(QEvent::MouseMove, QPoint(start.x(), start.y() + 1), Qt::NoButton, Qt::LeftButton);
            send(QEvent::MouseMove, end, Qt::NoButton, Qt::LeftButton);
            const QFrame *line = port->findChild<QFrame *>(QStringLiteral("outlineDropLine"));
            const bool shown = line && line->isVisible();
            send(QEvent::MouseButtonRelease, end, Qt::LeftButton, Qt::NoButton);
            return shown;
        };
        // "Three" to the top: before "One".
        QVERIFY2(drag(3, 0, false), "a line shows where it will go");
        QCOMPARE(texts(*w.document()), (QStringList{"Three", "One", "body", "Two"}));
        QVERIFY(!port->findChild<QFrame *>(QStringLiteral("outlineDropLine"))->isVisible());
        w.document()->undo();
        QCOMPARE(texts(*w.document()), (QStringList{"One", "body", "Two", "Three"}));
        // "One" with its body, to the end.
        QVERIFY(drag(0, 3, true));
        QCOMPARE(texts(*w.document()), (QStringList{"Two", "Three", "One", "body"}));
        w.document()->undo();
        // Dragged by its words, and let go where it began, nothing moves.
        QVERIFY2(!drag(2, 0, false, false), "no line is drawn for a drag by the words");
        QCOMPARE(texts(*w.document()), (QStringList{"One", "body", "Two", "Three"}));
        drag(0, 1, false);   // (inside its own section)
        QCOMPARE(texts(*w.document()), (QStringList{"One", "body", "Two", "Three"}));
    }

    void draftViewJoinsThePagesAndDrawsNoEdgesOrHeaders()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.show();
        QTextCursor c = w.view()->cursor();
        insertBlankPage(*w.document(), c);
        insertBlankPage(*w.document(), c);
        w.view()->paginateAll();
        w.view()->setZoom(0.5);
        const int gap = w.view()->pageRect(1).top() - w.view()->pageRect(0).bottom();
        QVERIFY(gap > 2);
        QVERIFY(w.act("view.draft") && !w.act("view.draft")->isChecked() && w.act("view.printLayout")->isChecked());
        bool drafted = false;
        setPaintObserver([&](const flow::MaterializedPage &, const PageDrawing &d) { drafted = d.draft; });
        w.act(QStringLiteral("view.draft"))->trigger();
        QVERIFY(w.view()->draft());
        QVERIFY(!w.act("view.printLayout")->isChecked() && w.act("view.draft")->isChecked());
        QVERIFY(std::abs(w.view()->pageRect(1).top() - w.view()->pageRect(0).bottom()) <= 1);   // (rounded to whole pixels)
        w.view()->grab();
        setPaintObserver(nullptr);
        QVERIFY(drafted);
        // Print Layout, and Outline, take it away again.
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QVERIFY(!w.view()->draft());
        QVERIFY(std::abs(w.view()->pageRect(1).top() - w.view()->pageRect(0).bottom() - gap) <= 1);
        w.act(QStringLiteral("view.draft"))->trigger();
        w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w.outlineMode() && !w.view()->draft() && !w.act("view.draft")->isChecked());
    }

    // ---------------------------------------------------------------- the style area
    void theStyleAreaIsHiddenByDefaultAndCostsNothing()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.setDocument(areaDoc(areaPara("Quote1", "one") + areaPara("Normal", "two")), QString());
        w.show();
        w.act(QStringLiteral("view.draft"))->trigger();
        QVERIFY(w.view()->draft());
        QCOMPARE(w.view()->styleAreaWidth(), 0);
        QVERIFY(!w.view()->styleArea());   // (not even made)
        QCOMPARE(w.view()->viewport()->x(), 0);
        const qint64 before = w.view()->styleParagraphsLooked();
        for (int i = 0; i < 5; ++i) w.view()->verticalScrollBar()->setValue(i);
        w.view()->setZoom(0.8);
        settleTurns();
        QCOMPARE(w.view()->styleParagraphsLooked(), before);
        QCOMPARE(w.view()->styleParagraphsLooked(), qint64(0));
        QVERIFY(!w.outlineView()->styleArea());
    }

    void theStyleAreaNamesEachParagraphBesideItsFirstLineAtAnyZoom()
    {
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Quote1", "one") + areaPara("Normal", "two") + areaPara("Long1", "three")), 120);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa && sa->isVisible());
        QTRY_VERIFY(sa->shownNames().size() >= 3);
        QCOMPARE(sa->shownNames().mid(0, 3), (QStringList{"Quote Style", "Normal", "An unusually long style name that cannot fit in a narrow strip"}));
        QCOMPARE(sa->width(), 120);
        QCOMPARE(w.view()->viewport()->x(), 120);   // (the text begins where the strip ends)
        for (const double zoom : {1.0, 0.5, 2.0}) {
            w.view()->setZoom(zoom);
            w.view()->verticalScrollBar()->setValue(0);
            QTRY_VERIFY2(sa->rows().size() >= 3 && sa->rows().at(1).top != sa->rows().at(0).top, qPrintable(QStringLiteral("zoom %1: %2 rows, scroll %3, page0 %4,%5 view h %6, looked %7").arg(zoom).arg(sa->rows().size()).arg(w.view()->verticalScrollBar()->value()).arg(w.view()->pageRect(0).top()).arg(w.view()->pageRect(0).bottom()).arg(w.view()->viewport()->height()).arg(w.view()->styleParagraphsLooked())));
            settleTurns();
            const QVector<StyleRow> rows = sa->rows();
            QVERIFY(rows.size() >= 3);
            for (int i = 0; i < 3; ++i) {
                QTextCursor c(w.document()->story());
                c.setPosition(rows.at(i).position);
                w.view()->setCursor(c);
                QRect r;
                QVERIFY(w.view()->caretRect(&r));
                QVERIFY2(std::abs(r.top() - rows.at(i).top) <= 2, qPrintable(QStringLiteral("zoom %1 row %2: caret %3, name %4").arg(zoom).arg(i).arg(r.top()).arg(rows.at(i).top)));
            }
        }
    }

    void aLongNameIsCutAndKeepsItsFullNameForScreenReaders()
    {
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Quote1", "one") + areaPara("Long1", "two")), 100);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 2);
        QVERIFY(!sa->nameCut(0));   // ("Quote Style" fits)
        QVERIFY(sa->nameCut(1));
        QStringList accessible;
        for (QLabel *l : sa->findChildren<QLabel *>(QStringLiteral("styleAreaName")))
            if (l->isVisible()) accessible << l->accessibleName();
        QVERIFY(accessible.contains(QStringLiteral("An unusually long style name that cannot fit in a narrow strip")));
        QVERIFY(accessible.contains(QStringLiteral("Quote Style")));
        QVERIFY(!sa->accessibleName().isEmpty());
        QCOMPARE(sa->width(), 100);
        // a wider strip does not cut it
        sa->setAreaWidth(StyleArea::kMostWidth);
        QTRY_VERIFY(!sa->nameCut(1));
    }

    void aParagraphInATableCellShowsItsOwnStyle()
    {
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                                 "<w:tr>" + areaCell(areaPara("Quote1", "a")) + areaCell(areaPara("Long1", "b")) + "</w:tr>"
                                 "<w:tr>" + areaCell(areaPara("Long1", "c")) + areaCell(areaPara("Normal", "d")) + "</w:tr></w:tbl>";
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Normal", "before") + table + areaPara("Quote1", "after")), 120);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 4);
        // (Side by side, the cells' first lines are at one place: the first cell's name is the one shown for the row.)
        QCOMPARE(sa->shownNames().mid(0, 4), (QStringList{"Normal", "Quote Style", "An unusually long style name that cannot fit in a narrow strip", "Quote Style"}));
    }

    void aTrackedStyleChangeShowsTheNewName()
    {
        const QByteArray changed = "<w:p><w:pPr><w:pStyle w:val=\"Quote1\"/><w:pPrChange w:id=\"1\" w:author=\"A\" w:date=\"2026-01-01T00:00:00Z\"><w:pPr><w:pStyle w:val=\"Long1\"/></w:pPr></w:pPrChange></w:pPr>"
                                   "<w:r><w:t>words</w:t></w:r></w:p>";
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(changed + areaPara("Normal", "plain")), 120);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 2);
        QCOMPARE(sa->shownNames().mid(0, 2), (QStringList{"Quote Style", "Normal"}));
    }

    void aClickOnANameSelectsTheParagraphAndADoubleClickOpensItsStylesBox()
    {
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Normal", "first words") + areaPara("Quote1", "second paragraph") + areaPara("Normal", "third")), 120);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 3);
        const StyleRow second = sa->rows().at(1);
        QTest::mouseClick(sa, Qt::LeftButton, Qt::NoModifier, areaRowPoint(second));
        const QTextCursor sel = w.view()->cursor();
        QVERIFY(sel.hasSelection());
        QCOMPARE(sel.selectionStart(), second.position);
        QCOMPARE(sel.selectedText(), QStringLiteral("second paragraph"));
        QVERIFY(w.view()->hasFocus());
        // (the Styles pane's own choice does not decide which style is changed)
        DialogSeen seen;
        closeNextDialog(&w, &seen);
        QTest::mouseDClick(sa, Qt::LeftButton, Qt::NoModifier, areaRowPoint(second));
        QVERIFY(seen.seen);
        QCOMPARE(seen.title, QStringLiteral("Modify Style"));
        QCOMPARE(seen.name, QStringLiteral("Quote Style"));
        QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("second paragraph"));
        // a click between the names, or past the last, selects nothing new
        QTextCursor none(w.document()->story());
        w.view()->setCursor(none);
        QTest::mouseClick(sa, Qt::LeftButton, Qt::NoModifier, QPoint(8, sa->height() - 2));
        QVERIFY(!w.view()->cursor().hasSelection());
    }

    void theStyleAreaFollowsScrollingAndWorksOnlyForTheParagraphsOnShow()
    {
        QByteArray body;
        for (int i = 0; i < 3000; ++i) body += areaPara(i % 3 == 0 ? "Quote1" : "Normal", QStringLiteral("Paragraph %1").arg(i));
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(body), 100);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        w.view()->paginateAll();
        QTRY_VERIFY(sa->rows().size() >= 10);
        settleTurns();
        QCOMPARE(w.view()->pageCount() >= 40, true);   // (the document is many pages: a pass over all of it would look at all 3,000)
        qint64 looked = w.view()->styleParagraphsLooked();
        // Scrolling: every name moves with its paragraph, exactly, and a step looks at the paragraphs of the pages on show only.
        for (int step = 0; step < 20; ++step) {
            const QVector<StyleRow> was = sa->rows();
            QVERIFY(!was.isEmpty());
            const qint64 lookedBefore = w.view()->styleParagraphsLooked();
            const int at = w.view()->verticalScrollBar()->value();
            w.view()->verticalScrollBar()->setValue(at + 37);
            const int moved = w.view()->verticalScrollBar()->value() - at;
            QCOMPARE(moved, 37);
            const QVector<StyleRow> now = sa->rows();
            QVERIFY(w.view()->styleParagraphsLooked() - lookedBefore <= 250);
            QVERIFY(w.view()->styleParagraphsLooked() - lookedBefore > 0);
            int matched = 0;
            for (const StyleRow &a : was)
                for (const StyleRow &b : now)
                    if (a.position == b.position) {
                        QCOMPARE(b.top, a.top - moved);
                        ++matched;
                    }
            QVERIFY(matched >= 3);
        }
        QVERIFY(w.view()->styleParagraphsLooked() - looked <= 20 * 250);
        // A caret move that scrolls nothing makes no rows at all.
        w.view()->verticalScrollBar()->setValue(0);
        settleTurns();
        QTextCursor c(w.document()->story());
        w.view()->setCursor(c);
        looked = w.view()->styleParagraphsLooked();
        const qint64 made = sa->refreshes();
        for (int i = 0; i < 10; ++i) QTest::keyClick(w.view(), Qt::Key_Down);
        settleTurns();
        QCOMPARE(w.view()->verticalScrollBar()->value(), 0);
        QCOMPARE(w.view()->styleParagraphsLooked(), looked);
        QCOMPARE(sa->refreshes(), made);
        // Far down the document it is the same: the names there are the paragraphs there.
        w.view()->verticalScrollBar()->setValue(w.view()->verticalScrollBar()->maximum() / 2);
        QVERIFY(!sa->rows().isEmpty());
        QVERIFY(sa->rows().first().position > 10000);
    }

    void theStyleAreaIsOnlyInDraftAndOutlineViews()
    {
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Quote1", "one") + areaPara("Normal", "two")), 100);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->isVisible() && sa->rows().size() >= 2);
        // Print Layout
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QVERIFY(!sa->isVisible());
        QCOMPARE(w.view()->viewport()->x(), 0);
        // Web Layout
        w.act(QStringLiteral("view.webLayout"))->trigger();
        QVERIFY(!sa->isVisible());
        // Read Mode
        w.act(QStringLiteral("view.draft"))->trigger();
        QVERIFY(sa->isVisible());
        w.act(QStringLiteral("view.readMode"))->trigger();
        QVERIFY(w.view()->draft());
        QVERIFY(!sa->isVisible());
        QCOMPARE(w.view()->viewport()->x(), 0);
        w.act(QStringLiteral("view.readMode"))->trigger();
        QTRY_VERIFY(sa->isVisible());
        // Outline: its own strip (the pages' is away with the pages)
        w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w.outlineMode());
        QVERIFY(!sa->isVisible());
        w.outlineView()->setStyleAreaWidth(100);
        StyleArea *os = w.outlineView()->styleArea();
        QVERIFY(os && os->isVisible());
        QTRY_VERIFY(os->rows().size() >= 2);
        QCOMPARE(os->shownNames().mid(0, 2), (QStringList{"Quote Style", "Normal"}));
        QCOMPARE(w.outlineView()->viewport()->x(), 100);
        // a double click on a name in the outline opens that style's box
        DialogSeen seen;
        closeNextDialog(&w, &seen);
        QTest::mouseDClick(os, Qt::LeftButton, Qt::NoModifier, areaRowPoint(os->rows().at(0)));
        QVERIFY(seen.seen);
        QCOMPARE(seen.name, QStringLiteral("Quote Style"));
    }

    void theStyleAreaIsNotPrintedAndChangesNoSavedByte()
    {
        QTemporaryDir dir;
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 700);
        w.setDocument(areaDoc(areaPara("Quote1", "one") + areaPara("Normal", "two") + areaPara("Long1", "three")), QString());
        w.show();
        const QString plain = dir.filePath(QStringLiteral("plain.docx"));
        QVERIFY(w.saveTo(plain, true));
        w.view()->setStyleAreaWidth(150);
        w.act(QStringLiteral("view.draft"))->trigger();
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 3);
        w.view()->verticalScrollBar()->setValue(5);
        QTest::mouseClick(sa, Qt::LeftButton, Qt::NoModifier, areaRowPoint(sa->rows().at(1)));
        const QString shown = dir.filePath(QStringLiteral("shown.docx"));
        QVERIFY(w.saveTo(shown, true));
        QFile a(plain), b(shown);
        QVERIFY(a.open(QIODevice::ReadOnly) && b.open(QIODevice::ReadOnly));
        const QByteArray plainBytes = a.readAll(), shownBytes = b.readAll();
        a.close();
        b.close();
        QVERIFY(!plainBytes.isEmpty());
        QCOMPARE(shownBytes, plainBytes);
        // The pages as printed carry no strip: a page drawn for the printer is the same with it shown and hidden.
        const QImage withStrip = w.view()->pageImage(0, 400);
        w.view()->setStyleAreaWidth(0);
        QVERIFY(!sa->isVisible());
        const QImage without = w.view()->pageImage(0, 400);
        QVERIFY(!withStrip.isNull());
        QCOMPARE(withStrip, without);
    }

    void theStyleAreaWidthIsAnOptionAndTheEdgeIsDragged()
    {
        const StyleWidthReset reset;
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            QVector<OptionsPage> pages = w.optionsPages();
            QList<QWidget *> built;
            for (const OptionsPage &p : pages) built << p.build();
            MeasureSpin *box = nullptr;
            for (QWidget *page : built)
                if (auto *f = page->findChild<MeasureSpin *>(QStringLiteral("styleAreaWidth"))) box = f;
            QVERIFY(box);
            QCOMPARE(box->value(), 0);   // (hidden unless the person chose)
            // (in the person's units: a pixel is 3/4 of a point; nothing lies between hidden and the least width the strip takes)
            box->setValue(0.5 * StyleArea::kLeastWidth * 0.75);
            QCOMPARE(box->value(), StyleArea::kLeastWidth * 0.75);
            box->setValue(0);
            box->setValue(90 * 0.75);
            QCOMPARE(box->value(), 90 * 0.75);
            box->setValue(StyleArea::kLeastWidth * 0.75 - 1);
            QCOMPARE(box->value(), 0.0);
            QCOMPARE(box->maximum(), StyleArea::kMostWidth * 0.75);
            box->setValue(90 * 0.75);
            for (const OptionsPage &p : pages)
                if (p.apply) p.apply();
            qDeleteAll(built);
            QCOMPARE(w.view()->styleAreaWidth(), 90);
            QCOMPARE(w.outlineView()->styleAreaWidth(), 90);
            QCOMPARE(Settings::get().value(QStringLiteral("view/styleAreaWidth"), 0).toInt(), 90);
        }
        // a new window starts with it, and the edge is dragged with the mouse (real press, move, and release)
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Quote1", "one") + areaPara("Normal", "two")), 90);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa && sa->areaWidth() == 90);
        QTRY_VERIFY(sa->rows().size() >= 2);
        QTest::mousePress(sa, Qt::LeftButton, Qt::NoModifier, QPoint(sa->width() - 2, 30));
        QTest::mouseMove(sa, QPoint(sa->width() + 38, 30));
        QTest::mouseRelease(sa, Qt::LeftButton, Qt::NoModifier, QPoint(sa->width() + 38, 30));
        QCOMPARE(sa->areaWidth(), 130);
        QCOMPARE(w.view()->styleAreaWidth(), 130);
        QCOMPARE(w.view()->viewport()->x(), 130);
        QCOMPARE(w.outlineView()->styleAreaWidth(), 130);
        QCOMPARE(Settings::get().value(QStringLiteral("view/styleAreaWidth"), 0).toInt(), 130);
        // (the drag has ended: a move now changes nothing)
        QTest::mouseMove(sa, QPoint(10, 30));
        QCOMPARE(sa->areaWidth(), 130);
    }

    void theStyleAreaIsReachedByTheKeyboardAndFollowsAMirroredWindow()
    {
        {
            WriteWindow w;
            draftWithStyleArea(w, areaDoc(areaPara("Normal", "one") + areaPara("Quote1", "two")), 110);
            StyleArea *sa = w.view()->styleArea();
            QVERIFY(sa);
            QTRY_VERIFY(sa->rows().size() >= 2);
            // The strip takes the keyboard's Tab focus; Down goes to the next name and Enter opens that paragraph's style box.
            QVERIFY(sa->focusPolicy() & Qt::TabFocus);
            sa->setFocus(Qt::TabFocusReason);
            QTRY_VERIFY(sa->hasFocus());
            QTest::keyClick(sa, Qt::Key_Down);
            QCOMPARE(sa->currentRow(), 1);
            QSignalSpy spy(w.view(), &PageView::styleNameActivated);
            DialogSeen seen;
            closeNextDialog(&w, &seen);
            QTest::keyClick(sa, Qt::Key_Return);
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spy.first().first().toInt(), sa->rows().at(1).position);
            QVERIFY(seen.seen);
            QCOMPARE(seen.name, QStringLiteral("Quote Style"));
            w.activateWindow();
            QVERIFY(QTest::qWaitForWindowActive(&w));
            sa->setFocus(Qt::TabFocusReason);
            QTRY_VERIFY(sa->hasFocus());
            QTest::keyClick(sa, Qt::Key_Escape);
            QTRY_VERIFY(w.view()->hasFocus());
            // The strip is on the left of the pages, for a paragraph that runs right to left too.
        }
        {
            WriteWindow w;
            draftWithStyleArea(w, areaDoc(areaPara("Normal", "one", "<w:bidi/>") + areaPara("Quote1", "two", "<w:bidi/>")), 110);
            StyleArea *sa = w.view()->styleArea();
            QVERIFY(sa);
            QTRY_VERIFY(sa->rows().size() >= 2);
            QVERIFY(sa->x() < w.view()->viewport()->x());
        }
        {
            // A mirrored window puts the strip on the other side of the pages, as it does its scroll bar.
            WriteWindow w;
            w.setLayoutDirection(Qt::RightToLeft);
            draftWithStyleArea(w, areaDoc(areaPara("Normal", "one") + areaPara("Quote1", "two")), 110);
            StyleArea *sa = w.view()->styleArea();
            QVERIFY(sa);
            QTRY_VERIFY(sa->rows().size() >= 2);
            QVERIFY2(sa->x() > w.view()->viewport()->x(), qPrintable(QStringLiteral("strip %1, viewport %2").arg(sa->x()).arg(w.view()->viewport()->x())));
            QCOMPARE(sa->width(), 110);
        }
    }

    void typingAndPastingWhileTheStripHasTheFocusDoNotChangeTheText()
    {
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Normal", "one") + areaPara("Quote1", "two")), 110);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 2);
        const QString before = w.document()->story()->toPlainText();
        QTextCursor c(w.document()->story());
        c.setPosition(2);
        w.view()->setCursor(c);
        sa->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(sa->hasFocus());
        for (Qt::Key key : {Qt::Key_Z, Qt::Key_Space, Qt::Key_Backspace, Qt::Key_Delete, Qt::Key_Left, Qt::Key_Right, Qt::Key_Home, Qt::Key_End, Qt::Key_PageDown, Qt::Key_A}) QTest::keyClick(sa, key);
        QTest::keyClicks(sa, QStringLiteral("zzz"));
        QCOMPARE(w.document()->story()->toPlainText(), before);
        QCOMPARE(w.view()->cursor().position(), 2);
        // typing by input method
        QInputMethodEvent im(QString(), {});
        im.setCommitString(QStringLiteral("yyy"));
        QCoreApplication::sendEvent(sa, &im);
        QCOMPARE(w.document()->story()->toPlainText(), before);
        // Tab leaves the strip and Escape returns to the text, as before
        QTest::keyClick(sa, Qt::Key_Escape);
        QTRY_VERIFY(w.view()->hasFocus());
    }

    void windowShortcutsStillWorkWhileTheStripHasTheFocus()
    {
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Normal", "one") + areaPara("Quote1", "two")), 110);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 2);
        QTextCursor c(w.document()->story());
        c.setPosition(2);
        insertText(*w.document(), c, QStringLiteral("XY"));
        w.view()->setCursor(c);
        w.refreshNow();
        QCOMPARE(w.document()->story()->toPlainText().left(5), QStringLiteral("onXYe"));
        QAction *undo = w.act(QStringLiteral("edit.undo"));
        QVERIFY(undo->isEnabled());
        QSignalSpy spy(undo, &QAction::triggered);
        sa->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(sa->hasFocus());
        const QKeyCombination key = QKeySequence(QKeySequence::Undo)[0];
        QTest::keyClick(sa, key.key(), key.keyboardModifiers());
        QCOMPARE(spy.count(), 1);   // (the shortcut path: the strip does not claim the key)
    }

    void leavingDraftMidDragEndsTheDragAndAMoveWithNoButtonNeverResizes()
    {
        const StyleWidthReset reset;
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Quote1", "one") + areaPara("Normal", "two")), 90);
        StyleArea *sa = w.view()->styleArea();
        QVERIFY(sa);
        QTRY_VERIFY(sa->rows().size() >= 2);
        auto move = [sa](int x, Qt::MouseButtons buttons) {
            const QPoint p(x, 30);
            QMouseEvent e(QEvent::MouseMove, p, sa->mapToGlobal(p), Qt::NoButton, buttons, Qt::NoModifier);
            QCoreApplication::sendEvent(sa, &e);
        };
        auto release = [sa](int x) {
            const QPoint p(x, 30);
            QMouseEvent e(QEvent::MouseButtonRelease, p, sa->mapToGlobal(p), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(sa, &e);
        };
        auto press = [sa](int x) {
            const QPoint p(x, 30);
            QMouseEvent e(QEvent::MouseButtonPress, p, sa->mapToGlobal(p), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(sa, &e);
        };
        // A: the pointer leaves with the button still down; the strip is hidden, then shown again.
        press(sa->width() - 2);
        move(sa->width() + 8, Qt::LeftButton);
        QCOMPARE(sa->areaWidth(), 100);
        w.act(QStringLiteral("view.printLayout"))->trigger();
        QVERIFY(!sa->isVisible());
        w.act(QStringLiteral("view.draft"))->trigger();
        QTRY_VERIFY(sa->isVisible());
        move(sa->width() + 40, Qt::LeftButton);
        QCOMPARE(sa->areaWidth(), 100);
        release(sa->width());
        // B: a move that arrives with no button down ends a drag.
        press(sa->width() - 2);
        move(sa->width() + 8, Qt::LeftButton);
        QCOMPARE(sa->areaWidth(), 110);
        move(sa->width() + 40, Qt::NoButton);
        QCOMPARE(sa->areaWidth(), 110);
        release(sa->width());
    }

    void aStyleChangeShowsInTheNamesOfBothViewsOfTheDocument()
    {
        const StyleWidthReset reset;
        WriteWindow w;
        draftWithStyleArea(w, areaDoc(areaPara("Normal", "one") + areaPara("Normal", "two")), 110);
        w.outlineView()->setStyleAreaWidth(110);
        StyleArea *draft = w.view()->styleArea();
        QVERIFY(draft);
        QTRY_COMPARE(draft->shownNames().mid(0, 2), (QStringList{"Normal", "Normal"}));
        QTextCursor c(w.document()->story());
        c.setPosition(w.document()->story()->findBlockByNumber(1).position());
        applyParagraphStyle(*w.document(), c, QStringLiteral("Quote1"));
        w.view()->setCursor(c);
        w.refreshNow();
        QTRY_COMPARE(draft->shownNames().mid(0, 2), (QStringList{"Normal", "Quote Style"}));
        w.act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w.outlineMode());
        StyleArea *outline = w.outlineView()->styleArea();
        QVERIFY(outline);
        QTRY_COMPARE(outline->shownNames().mid(0, 2), (QStringList{"Normal", "Quote Style"}));
        QTextCursor d(w.document()->story());
        d.setPosition(0);
        applyParagraphStyle(*w.document(), d, QStringLiteral("Long1"));
        w.outlineView()->rebuild();
        QTRY_COMPARE(outline->shownNames().mid(0, 2), (QStringList{"An unusually long style name that cannot fit in a narrow strip", "Quote Style"}));
        w.act(QStringLiteral("view.draft"))->trigger();
        QTRY_COMPARE(draft->shownNames().mid(0, 2), (QStringList{"An unusually long style name that cannot fit in a narrow strip", "Quote Style"}));
    }

    // Pictures of the strip for the person who asked for it (only where JW_SHOT_DIR names a folder).
    void theStyleAreaScreenshots()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\"/><w:left w:val=\"single\" w:sz=\"4\"/><w:bottom w:val=\"single\" w:sz=\"4\"/><w:right w:val=\"single\" w:sz=\"4\"/><w:insideH w:val=\"single\" w:sz=\"4\"/><w:insideV w:val=\"single\" w:sz=\"4\"/></w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                                 "<w:tr>" + areaCell(areaPara("Quote1", "Cell one")) + areaCell(areaPara("Long1", "Cell two")) + "</w:tr>"
                                 "<w:tr>" + areaCell(areaPara("Long1", "Cell three")) + areaCell(areaPara("Normal", "Cell four")) + "</w:tr></w:tbl>";
        const QByteArray body = areaPara("Quote1", "A paragraph in the quote style.") + areaPara("Normal", "Plain text in the normal style, long enough to wrap onto a second line so that the name stays level with the first line only.") + areaPara("Long1", "A paragraph with a very long style name.") + table + areaPara("Normal", "After the table.");
        const int widths[] = {140, 60};
        const char *names[] = {"stylearea-shown.png", "stylearea-narrow.png"};
        for (int i = 0; i < 2; ++i) {
            WriteWindow w;
            draftWithStyleArea(w, areaDoc(body), widths[i]);
            QTRY_VERIFY(w.view()->styleArea() && w.view()->styleArea()->rows().size() >= 6);
            settleTurns();
            QVERIFY(w.grab().save(QDir(dir).filePath(QString::fromLatin1(names[i]))));
        }
    }

    // ---------------------------------------------------------------- Layout
    void theHyphenationZoneAndLimitAreReadSetAndSaved()
    {
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", "<w:autoHyphenation/><w:consecutiveHyphenLimit w:val=\"2\"/><w:hyphenationZone w:val=\"720\"/>"), &err);
        QVERIFY(doc);
        QCOMPARE(doc->hyphenationZone(), 36.0);
        QCOMPARE(doc->hyphenLimit(), 2);
        doc->setHyphenation(10.0, 3);
        QCOMPARE(doc->hyphenationZone(), 10.0);
        QCOMPARE(doc->hyphenLimit(), 3);
        const QByteArray settings = settingsXml(doc->toDocx());
        QVERIFY2(settings.contains("<w:consecutiveHyphenLimit w:val=\"3\"/>"), settings.constData());
        QVERIFY2(settings.contains("<w:hyphenationZone w:val=\"200\"/>"), settings.constData());
        // (The schema's order: auto, limit, zone.)
        QVERIFY(settings.indexOf("autoHyphenation") < settings.indexOf("consecutiveHyphenLimit"));
        QVERIFY(settings.indexOf("consecutiveHyphenLimit") < settings.indexOf("hyphenationZone"));
        auto again = WriteDoc::fromDocx(doc->toDocx(), &err);
        QVERIFY(again);
        QCOMPARE(again->hyphenationZone(), 10.0);
        QCOMPARE(again->hyphenLimit(), 3);
        // Put back to the usual, the settings say nothing of them.
        doc->setHyphenation(18.0, 0);
        const QByteArray plain = settingsXml(doc->toDocx());
        QVERIFY(!plain.contains("consecutiveHyphenLimit") && !plain.contains("hyphenationZone"));
    }

    void wordsInCapitalsAreHyphenatedUnlessTheSettingsSayNot()
    {
        auto on = capsDoc("<w:autoHyphenation/>");
        QVERIFY(on);
        QVERIFY(on->hyphenateCaps());
        QVERIFY2(hyphenRun(*on).first >= 1, qPrintable(QString::number(hyphenRun(*on).first)));
        auto off = capsDoc("<w:autoHyphenation/><w:doNotHyphenateCaps/>");
        QVERIFY(off);
        QVERIFY(!off->hyphenateCaps());
        QCOMPARE(hyphenRun(*off).first, 0);
        // The choice is the document's: set, it lays the lines out again, undoes, and is saved in the schema's place (after the zone, before the compatibility part).
        off->setHyphenateCaps(true);
        QVERIFY(hyphenRun(*off).first >= 1);
        const QByteArray plain = settingsXml(off->toDocx());
        QVERIFY2(!plain.contains("doNotHyphenateCaps"), plain.constData());
        on->setHyphenateCaps(false);
        QCOMPARE(hyphenRun(*on).first, 0);
        const QByteArray settings = settingsXml(on->toDocx());
        QVERIFY2(settings.contains("<w:doNotHyphenateCaps/>"), settings.constData());
        QVERIFY(settings.indexOf("autoHyphenation") < settings.indexOf("doNotHyphenateCaps"));
        QString err;
        auto again = WriteDoc::fromDocx(on->toDocx(), &err);
        QVERIFY(again);
        QVERIFY(!again->hyphenateCaps());
        on->undo();
        QVERIFY(on->hyphenateCaps());
    }

    void hyphenateCapsOnTheLayoutTabFollowsAutomaticHyphenation()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.act("layout.hyphenation.caps"));
        QVERIFY(!w.act("layout.hyphenation.caps")->isEnabled());   // (nothing is hyphenated, so there is nothing to choose)
        w.act(QStringLiteral("layout.hyphenation.auto"))->trigger();
        QTRY_VERIFY(w.act("layout.hyphenation.caps")->isEnabled());
        QVERIFY(w.act("layout.hyphenation.caps")->isChecked());
        w.act(QStringLiteral("layout.hyphenation.caps"))->trigger();
        QVERIFY(!w.document()->hyphenateCaps());
        QVERIFY(!w.act("layout.hyphenation.caps")->isChecked());
    }

    void theLimitKeepsLinesFromEndingInAHyphenTooManyTimesInARow()
    {
        auto none = narrowDoc("<w:autoHyphenation/>");
        QVERIFY(none);
        const QPair<int, int> free = hyphenRun(*none);
        QVERIFY2(free.first >= 3, qPrintable(QString::number(free.first)));   // (there are long runs without a limit)
        auto one = narrowDoc("<w:autoHyphenation/><w:consecutiveHyphenLimit w:val=\"1\"/>");
        QVERIFY(one);
        const QPair<int, int> limited = hyphenRun(*one);
        QCOMPARE(limited.first, 1);
        QVERIFY(limited.second >= free.second);   // (words moved down whole take no fewer lines)
        auto two = narrowDoc("<w:autoHyphenation/><w:consecutiveHyphenLimit w:val=\"2\"/>");
        QVERIFY(hyphenRun(*two).first <= free.first);
        QVERIFY(hyphenRun(*two).first >= limited.first);
    }

    void hyphenationOptionsOnTheLayoutTabSetTheZoneAndLimit()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        double seenZone = 0;
        int seenLimit = -1;
        w.setHyphenationAsker([&](double &zone, int &limit) {
            seenZone = zone;
            seenLimit = limit;
            zone = 24;
            limit = 2;
            return true;
        });
        w.act(QStringLiteral("layout.hyphenation.options"))->trigger();
        QCOMPARE(seenZone, 18.0);
        QCOMPARE(seenLimit, 0);
        QCOMPARE(w.document()->hyphenationZone(), 24.0);
        QCOMPARE(w.document()->hyphenLimit(), 2);
        w.setHyphenationAsker([](double &, int &) { return false; });
        w.act(QStringLiteral("layout.hyphenation.options"))->trigger();
        QCOMPARE(w.document()->hyphenLimit(), 2);
    }

    // ---------------------------------------------------------------- Arrange
    void floatingPicturesMoveForwardAndBackwardInTheLayers()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        for (int i = 0; i < 3; ++i) {
            QCOMPARE(insertPicture(*doc, c, bluePng(), QStringLiteral("p%1").arg(i), PictureWrap::Square), PictureResult::Ok);
            insertText(*doc, c, QStringLiteral(" "));
        }
        QCOMPARE(insertPicture(*doc, c, bluePng(), QStringLiteral("flat")), PictureResult::Ok);   // (in line with the text: no layer)
        const QList<int> at = picturePositions(*doc);
        QCOMPARE(at.size(), 4);
        QVERIFY(!canOrderPicture(*doc, at[3], PictureOrder::BringToFront));
        QVERIFY(!setPictureOrder(*doc, at[3], PictureOrder::BringToFront));
        // Level to begin with (a new picture is at the usual height), the last of them is the one in front: none to bring to the front, a place to send to the back.
        QVERIFY(canOrderPicture(*doc, at[2], PictureOrder::SendToBack));
        QVERIFY(setPictureOrder(*doc, at[2], PictureOrder::SendToBack));
        QList<qint64> z = pictureHeights(doc->toDocx());
        QCOMPARE(z.size(), 3);
        QVERIFY2(z[2] < z[0] && z[2] < z[1], qPrintable(QString::number(z[2])));
        // The first to the front: above both.
        QVERIFY(setPictureOrder(*doc, at[0], PictureOrder::BringToFront));
        z = pictureHeights(doc->toDocx());
        QVERIFY(z[0] > z[1] && z[0] > z[2] && z[1] > z[2]);
        QVERIFY(!canOrderPicture(*doc, at[0], PictureOrder::BringToFront));
        // The one at the back goes one forward: above the middle one, below the front one.
        QVERIFY(setPictureOrder(*doc, at[2], PictureOrder::BringForward));
        z = pictureHeights(doc->toDocx());
        QVERIFY2(z[2] > z[1] && z[2] < z[0], qPrintable(QString::number(z[0]) + "," + QString::number(z[1]) + "," + QString::number(z[2])));
        // And one backward again, which is one step of the history each time.
        QVERIFY(setPictureOrder(*doc, at[2], PictureOrder::SendBackward));
        z = pictureHeights(doc->toDocx());
        QVERIFY(z[2] < z[1]);
        QVERIFY(doc->undo());
        z = pictureHeights(doc->toDocx());
        QVERIFY(z[2] > z[1] && z[2] < z[0]);
        // The lowest one sent backward goes behind the text; one more step forward brings it out in front of it again.
        QVERIFY(setPictureOrder(*doc, at[1], PictureOrder::SendBackward));
        QCOMPARE(pictureAt(*doc, doc->story(), at[1]).wrap, PictureWrap::BehindText);
        QVERIFY(!canOrderPicture(*doc, at[1], PictureOrder::SendBackward));
        QVERIFY(setPictureOrder(*doc, at[1], PictureOrder::BringForward));
        QCOMPARE(pictureAt(*doc, doc->story(), at[1]).wrap, PictureWrap::InFrontOfText);
        // The file reads back with the same layers.
        QString err;
        auto again = WriteDoc::fromDocx(doc->toDocx(), &err);
        QVERIFY(again);
        QCOMPARE(pictureHeights(again->toDocx()), pictureHeights(doc->toDocx()));
    }

    // ---------------------------------------------------------------- Insert Alignment Tab
    void anAlignmentTabInAHeaderIsSavedAsAnAbsolutePositionTab()
    {
        auto doc = WriteDoc::blank();
        QVERIFY(doc->createHeaderFooter(0, false, WriteDoc::kDefaultSlot));
        QTextDocument *header = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
        QVERIFY(header);
        QTextCursor c(header);
        insertText(*doc, c, QStringLiteral("Left"));
        QVERIFY(insertAlignmentTab(*doc, c, QStringLiteral("center"), QStringLiteral("margin"), QStringLiteral("dot")));
        insertText(*doc, c, QStringLiteral("Middle"));
        QVERIFY(insertAlignmentTab(*doc, c, QStringLiteral("right"), QStringLiteral("indent"), QStringLiteral("none")));
        insertText(*doc, c, QStringLiteral("Right"));
        QVERIFY(!insertAlignmentTab(*doc, c, QStringLiteral("sideways"), QStringLiteral("margin"), QStringLiteral("none")));   // (not one of the three)
        // Each is a step, and the tab is one character that the layout sets as a tab at the place it says.
        QCOMPARE(header->toPlainText().count(QChar::ObjectReplacementCharacter), 2);
        QVERIFY(header->toPlainText().startsWith(QStringLiteral("Left")));
        const QByteArray saved = doc->toDocx();
        office::Package p;
        p.read(saved);
        QByteArray headerXml;
        for (const QString &name : p.partNames())
            if (name.contains(QStringLiteral("header"), Qt::CaseInsensitive) && name.endsWith(QStringLiteral(".xml")) && !name.contains(QStringLiteral("_rels"))) headerXml = p.part(name);
        QVERIFY(!headerXml.isEmpty());
        QVERIFY2(headerXml.contains("w:relativeTo=\"margin\"") && headerXml.contains("w:alignment=\"center\"") && headerXml.contains("w:leader=\"dot\""), headerXml.constData());
        QVERIFY2(headerXml.contains("w:relativeTo=\"indent\"") && headerXml.contains("w:alignment=\"right\""), headerXml.constData());
        QCOMPARE(headerXml.count("<w:ptab "), 2);
        // Opened again, the tabs are still there, still in their place.
        QString err;
        auto again = WriteDoc::fromDocx(saved, &err);
        QVERIFY(again);
        QTextDocument *story = again->headerFooterStory(0, false, WriteDoc::kDefaultSlot);
        QVERIFY(story);
        QCOMPARE(story->toPlainText().count(QLatin1Char('\t')), 2);   // (read back, an absolute position tab is a tab that goes where it says)
        QStringList aligned;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().hasProperty(tp::AlignTab)) aligned << it.fragment().charFormat().stringProperty(tp::AlignTab);
        QCOMPARE(aligned, (QStringList{QStringLiteral("center:margin:dot"), QStringLiteral("right:indent:none")}));
        QVERIFY(doc->undo());
        QCOMPARE(header->toPlainText().count(QChar::ObjectReplacementCharacter), 2);   // (the last text typed is the step undone, not a tab)
    }

    void insertAlignmentTabOnTheHeaderAndFooterTabAsksForItsPlace()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.act("hf.alignmentTab"));
        QVERIFY(!w.act("hf.alignmentTab")->isEnabled());
        w.view()->paginateAll();
        QVERIFY(w.view()->enterHeaderFooter(0, false, true));
        w.refreshNow();
        QVERIFY(w.act("hf.alignmentTab")->isEnabled());
        QStringList asked;
        w.setAlignmentTabAsker([&](QString &alignment, QString &relativeTo, QString &leader) {
            asked << alignment << relativeTo << leader;
            alignment = QStringLiteral("right");
            leader = QStringLiteral("hyphen");
            return true;
        });
        w.act(QStringLiteral("hf.alignmentTab"))->trigger();
        QCOMPARE(asked, (QStringList{QStringLiteral("center"), QStringLiteral("margin"), QStringLiteral("none")}));   // (what a page number usually sits at: the middle, then offered again)
        QTextDocument *story = w.view()->headerFooterEdit().story;
        QVERIFY(story);
        QCOMPARE(story->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
        w.setAlignmentTabAsker([](QString &, QString &, QString &) { return false; });
        w.act(QStringLiteral("hf.alignmentTab"))->trigger();
        QCOMPARE(story->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
    }

    // The Header & Footer tab inserts fields (Field…) and saved blocks at the caret in the header, as the text has them; a block set on a page of its own goes in as a paragraph.
    void theHeaderTabInsertsAFieldAndASavedBlock()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.act("hf.field"));
        QVERIFY(!w.act("hf.field")->isEnabled());
        w.view()->paginateAll();
        QVERIFY(w.view()->enterHeaderFooter(0, false, true));
        w.refreshNow();
        QVERIFY(w.act("hf.field")->isEnabled());
        QTextDocument *story = w.view()->headerFooterEdit().story;
        QVERIFY(story);
        // Field…: a field of the list, in the header
        withDialog([&] { w.act("hf.field")->trigger(); }, QStringLiteral("fieldDialog"), [&](QDialog *d) { d->findChild<QLineEdit *>(QStringLiteral("fieldCode"))->setText(QStringLiteral("NUMWORDS")); });
        QVERIFY(w.document()->toDocx().size() > 0);
        office::Package pkg;
        pkg.read(w.document()->toDocx());
        bool inHeader = false;
        for (const QString &name : pkg.partNames())
            if (name.contains(QLatin1String("header")) && pkg.part(name).contains("NUMWORDS")) inHeader = true;
        QVERIFY2(inHeader, "the field is in the header part of the saved file");
        // A saved block: its words in the header; "on a page of its own" is a paragraph of its own there
        std::unique_ptr<WriteDoc> blank = WriteDoc::blank();
        QTextCursor in(blank->story());
        insertText(*blank, in, QStringLiteral("Block words"));
        jp::BlockInfo info;
        info.name = QStringLiteral("B");
        info.format = QStringLiteral("docx");
        info.insertAs = jp::BlockInsert::Page;
        QVERIFY(w.insertSavedBlock(info, blank->toDocx()));
        QVERIFY2(story->toPlainText().contains(QStringLiteral("Block words")), qPrintable(story->toPlainText()));
        QVERIFY(!story->toPlainText().contains(QChar(0x000C)));
        // (what keeps the break out is the page break command itself: a header has no pages to break)
        QTextCursor at(story);
        at.setPosition(1);
        insertPageBreak(*w.document(), at);
        QVERIFY(!story->toPlainText().contains(QChar(0x000C)));
    }

    // ---------------------------------------------------------------- Tables
    void textWrapsRoundATableThatTheTablePropertiesSetAtAPlace()
    {
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1500\"/><w:gridCol w:w=\"1500\"/></w:tblGrid>"
                                 "<w:tr><w:tc><w:p><w:r><w:t>a1</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>b1</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx(table + "<w:p><w:r><w:t>After the table, words that go round it.</w:t></w:r></w:p>", QByteArray()), &err);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->toPlainText().indexOf(QLatin1String("a1")) + 1);
        const TableProps was = tablePropsAt(*doc, c);
        QVERIFY(!was.wrapAround);
        TableProps now = was;
        now.wrapAround = true;
        setTableProps(*doc, c, was, now);
        QVERIFY(tablePropsAt(*doc, c).wrapAround);
        QVERIFY(doc->flow().tables.get(cellKeyOf(c.block()))->table->place.has_value());
        // The file says it, in the schema's place (after the style, before the width), and reads back the same.
        office::Package p;
        p.read(doc->toDocx());
        const QByteArray xml = p.part(QStringLiteral("/word/document.xml"));
        QVERIFY2(xml.contains("<w:tblpPr "), xml.constData());
        QVERIFY(xml.indexOf("<w:tblpPr ") < xml.indexOf("<w:tblW "));
        auto again = WriteDoc::fromDocx(doc->toDocx(), &err);
        QVERIFY(again);
        QTextCursor d(again->story());
        d.setPosition(again->story()->toPlainText().indexOf(QLatin1String("a1")) + 1);
        QVERIFY(tablePropsAt(*again, d).wrapAround);
        // Off again takes the element away; one step each.
        TableProps off = tablePropsAt(*doc, c);
        off.wrapAround = false;
        setTableProps(*doc, c, tablePropsAt(*doc, c), off);
        QVERIFY(!tablePropsAt(*doc, c).wrapAround);
        office::Package q;
        q.read(doc->toDocx());
        QVERIFY(!q.part(QStringLiteral("/word/document.xml")).contains("tblpPr"));
        QVERIFY(doc->undo());
        QVERIFY(tablePropsAt(*doc, c).wrapAround);
    }

    // Delete Cells, Shift Left and Shift Up work with Track Changes on as they do with it off, and the status bar says the change was not recorded.
    void deletingCellsAndShiftingWorksWhileTrackChangesIsOnAndSaysItWasNotRecorded()
    {
        auto cell = [](const char *t) { return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>") + t + "</w:t></w:r></w:p></w:tc>"; };
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid>"
                                 "<w:tr>" + cell("a1") + cell("b1") + cell("c1") + "</w:tr><w:tr>" + cell("a2") + cell("b2") + cell("c2") + "</w:tr><w:tr>" + cell("a3") + cell("b3") + cell("c3") + "</w:tr></w:tbl>";
        for (const char *command : {"table.deleteCells", "table.deleteCellsUp"})
            for (const bool tracking : {true, false}) {
                WriteWindow w;
                w.setNotifier([](const QString &, const QString &) {});
                w.resize(1100, 800);
                w.show();
                QString err;
                auto doc = WriteDoc::fromDocx(makeDocx(table + "<w:p><w:r><w:t>After</w:t></w:r></w:p>", QByteArray()), &err);
                QVERIFY(doc);
                w.setDocument(std::move(doc), QString());
                QTextCursor c(w.document()->story());
                c.setPosition(w.document()->story()->toPlainText().indexOf(QLatin1String("a2")) + 1);
                w.view()->setCursor(c);
                w.document()->setTrackChanges(tracking);
                w.refreshNow();
                QTRY_VERIFY(w.act(QString::fromLatin1(command))->isEnabled());
                const QString before = w.document()->story()->toPlainText();
                w.act(QString::fromLatin1(command))->trigger();
                QVERIFY(w.document()->story()->toPlainText() != before);
                if (tracking) QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("not recorded")), qPrintable(w.statusBar()->currentMessage()));
            }
    }

    void deletingACellCanShiftTheCellsAfterItLeft()
    {
        auto cell = [](const char *t) { return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>") + t + "</w:t></w:r></w:p></w:tc>"; };
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid>"
                                 "<w:tr>" + cell("a1") + cell("b1") + cell("c1") + "</w:tr><w:tr>" + cell("a2") + cell("b2") + cell("c2") + "</w:tr></w:tbl><w:p/>";
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx(table, QByteArray()), &err);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->toPlainText().indexOf(QLatin1String("b1")) + 1);
        QVERIFY(deleteTableCellsShiftLeft(*doc, c));
        const QString text = doc->story()->toPlainText();
        QVERIFY2(!text.contains(QLatin1String("b1")) && text.contains(QLatin1String("c1")) && text.contains(QLatin1String("b2")), qPrintable(text));
        office::Package p;
        p.read(doc->toDocx());
        const QByteArray xml = p.part(QStringLiteral("/word/document.xml"));
        const qsizetype secondRow = xml.indexOf("</w:tr>") + 7;
        const QByteArray firstRow = xml.left(secondRow);
        QVERIFY2(firstRow.count("<w:tc>") == 2, firstRow.constData());   // (the cells have no attributes in the file)
        QVERIFY2(firstRow.contains("<w:gridAfter w:val=\"1\"/>"), firstRow.constData());
        QCOMPARE(xml.mid(secondRow).count("<w:tc>"), 3);   // (the other row is as it was)
        auto again = WriteDoc::fromDocx(doc->toDocx(), &err);
        QVERIFY(again);
        QVERIFY(!again->story()->toPlainText().contains(QLatin1String("b1")));
        QVERIFY(doc->undo());
        QVERIFY(doc->story()->toPlainText().contains(QLatin1String("b1")));
        QVERIFY(!doc->canUndo());
    }

    // Delete Cells, Shift Up: the words of the cells under the ones that go move up in their columns and the last cells are empty; one step; refused where a cell down the column is joined with others.
    void deleteCellsShiftUpMovesTheWordsOfTheColumnUpAndLeavesTheLastCellsEmpty()
    {
        auto cell = [](const char *t, const char *extra = "") { return QByteArray("<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/>") + extra + "</w:tcPr><w:p><w:r><w:t>" + t + "</w:t></w:r></w:p></w:tc>"; };
        auto grid = [](const QByteArray &rows) {
            return "<w:tbl><w:tblPr><w:tblW w:w=\"2000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid>" + rows + "</w:tbl><w:p/>";
        };
        auto words = [](WriteDoc &doc) {
            QStringList out;
            for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
                if (inTable(b)) out << b.text();
            return out.join(QLatin1Char('|'));
        };
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx(grid("<w:tr>" + cell("a1") + cell("b1") + "</w:tr><w:tr>" + cell("a2") + cell("b2") + "</w:tr><w:tr>" + cell("a3") + cell("b3") + "</w:tr>"), QByteArray()), &err);
        QVERIFY(doc);
        auto caretIn = [&](const char *word) {
            QTextCursor c(doc->story());
            c.setPosition(doc->story()->toPlainText().indexOf(QLatin1String(word)) + 1);
            return c;
        };
        QCOMPARE(words(*doc), QStringLiteral("a1|b1|a2|b2|a3|b3"));
        // one cell: its column moves up by one
        QTextCursor c = caretIn("b1");
        QVERIFY(deleteTableCellsShiftUp(*doc, c, false));   // (only asked)
        QCOMPARE(words(*doc), QStringLiteral("a1|b1|a2|b2|a3|b3"));
        QVERIFY(deleteTableCellsShiftUp(*doc, c));
        QCOMPARE(words(*doc), QStringLiteral("a1|b2|a2|b3|a3|"));
        // the file keeps three rows of two cells, and says the same on reading it
        auto again = WriteDoc::fromDocx(doc->toDocx(), &err);
        QVERIFY(again);
        QCOMPARE(words(*again), QStringLiteral("a1|b2|a2|b3|a3|"));
        office::Package pkg;
        pkg.read(doc->toDocx());
        const QByteArray xml = pkg.part(QStringLiteral("/word/document.xml"));
        QCOMPARE(xml.count("<w:tr>") + xml.count("<w:tr "), 3);
        QCOMPARE(xml.count("<w:tc>"), 6);
        QVERIFY(doc->undo());
        QCOMPARE(words(*doc), QStringLiteral("a1|b1|a2|b2|a3|b3"));
        QVERIFY(!doc->canUndo());
        // two cells selected in a column: the third moves up by two
        QTextCursor two = caretIn("a1");
        two.setPosition(doc->story()->toPlainText().indexOf(QLatin1String("a2")) + 1, QTextCursor::KeepAnchor);
        QVERIFY(deleteTableCellsShiftUp(*doc, two));
        QCOMPARE(words(*doc), QStringLiteral("a3|b1||b2||b3"));
        QVERIFY(doc->undo());
        // the last cell of a column: its words go and nothing moves
        QTextCursor lastCell = caretIn("b3");
        QVERIFY(deleteTableCellsShiftUp(*doc, lastCell));
        QCOMPARE(words(*doc), QStringLiteral("a1|b1|a2|b2|a3|"));
        QVERIFY(doc->undo());
        // a cell below joined with others down is in the way, and nothing changes
        auto joined = WriteDoc::fromDocx(makeDocx(grid("<w:tr>" + cell("a1") + cell("b1") + "</w:tr><w:tr>" + cell("a2", "<w:vMerge w:val=\"restart\"/>") + cell("b2") + "</w:tr><w:tr>" + cell("", "<w:vMerge/>") + cell("b3") + "</w:tr>"), QByteArray()), &err);
        QVERIFY(joined);
        QTextCursor j(joined->story());
        j.setPosition(joined->story()->toPlainText().indexOf(QLatin1String("a1")) + 1);
        QVERIFY(!deleteTableCellsShiftUp(*joined, j, false));
        QVERIFY(!deleteTableCellsShiftUp(*joined, j));
        QVERIFY(!joined->canUndo());
    }

    void theTablePropertiesDialogHasAWrapTextAroundChoice()
    {
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>a1</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/>";
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx(table, QByteArray()), &err);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->toPlainText().indexOf(QLatin1String("a1")) + 1);
        bool found = false;
        QTimer::singleShot(0, [&found] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            if (auto *around = dlg->findChild<QCheckBox *>(QStringLiteral("wrapAround"))) {
                found = true;
                around->setChecked(true);
            }
            dlg->accept();
        });
        tablePropertiesDialog(nullptr, doc.get(), &c);
        QVERIFY(found);
        QVERIFY(tablePropsAt(*doc, c).wrapAround);
    }

    // ---------------------------------------------------------------- Paper sizes
    void envelopeSizesAreOnTheSizeMenuAndInPageSetup()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        const QList<std::pair<QString, QSizeF>> envelopes = {{QStringLiteral("envelope10"), QSizeF(297, 684)}, {QStringLiteral("envelopeMonarch"), QSizeF(279, 540)},
                                                              {QStringLiteral("envelopeDL"), QSizeF(311.81, 623.62)}, {QStringLiteral("envelopeC5"), QSizeF(459.21, 649.13)}};
        for (const auto &e : envelopes) {
            QVERIFY2(w.act(QStringLiteral("layout.size.") + e.first), qPrintable(e.first));
            w.act(QStringLiteral("layout.size.") + e.first)->trigger();
            const QSizeF paper = w.document()->flow().section(0).paper;   // (kept to a twentieth of a point)
            QVERIFY2(std::abs(paper.width() - e.second.width()) < 0.1 && std::abs(paper.height() - e.second.height()) < 0.1, qPrintable(e.first));
            QTRY_VERIFY(w.act(QStringLiteral("layout.size.") + e.first)->isChecked());
        }
        // Page Setup names the paper the same way.
        QString found;
        QTimer::singleShot(0, [&found] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            auto *size = dlg->findChild<QComboBox *>(QStringLiteral("paperSize"));
            if (size) found = size->currentText();
            dlg->reject();
        });
        pageSetupDialog(nullptr, w.document(), 0);
        QCOMPARE(found, QStringLiteral("Envelope C5"));
    }

    // ---------------------------------------------------------------- Page Setup
    void pageSetupFromThisPointForwardMakesASectionAtTheCursor()
    {
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx("<w:p><w:r><w:t>One</w:t></w:r></w:p><w:p><w:r><w:t>Two</w:t></w:r></w:p><w:p><w:r><w:t>Three</w:t></w:r></w:p>", QByteArray()), &err);
        QVERIFY(doc);
        QCOMPARE(doc->sectionCount(), 1);
        const SectionSetup before = pageSetupOf(doc->sectionProps(0), false);
        SectionSetup after = before;
        after.left = 36;
        after.right = 36;
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 1);
        QVERIFY(applyPageSetupFromCursor(*doc, c, before, after) >= 1);
        QCOMPARE(doc->sectionCount(), 2);
        QCOMPARE(pageSetupOf(doc->sectionProps(0), false).left, before.left);   // (the text before the cursor is as it was)
        QCOMPARE(pageSetupOf(doc->sectionProps(1), false).left, 36.0);
        QCOMPARE(pageSetupOf(doc->sectionProps(1), false).right, 36.0);
        QCOMPARE(doc->sectionProps(1).type, QStringLiteral("continuous"));
        QCOMPARE(doc->sectionAt(c.position()), 1);
        // One step, and the file keeps both sections.
        QByteArray body;
        {
            office::Package p;
            p.read(doc->toDocx());
            body = p.part(QStringLiteral("/word/document.xml"));
        }
        QCOMPARE(body.count("<w:sectPr"), 2);
        QVERIFY2(body.contains("w:left=\"720\""), body.constData());
        QVERIFY(doc->undo());
        QCOMPARE(doc->sectionCount(), 1);
        QVERIFY(!doc->canUndo());
        // Nothing to change puts nothing in.
        QCOMPARE(applyPageSetupFromCursor(*doc, c, before, before), 0);
        QCOMPARE(doc->sectionCount(), 1);
    }

    void thePageSetupDialogOffersThisPointForwardOnlyWithACursor()
    {
        QString err;
        auto doc = WriteDoc::fromDocx(makeDocx("<w:p><w:r><w:t>One</w:t></w:r></w:p><w:p><w:r><w:t>Two</w:t></w:r></w:p>", QByteArray()), &err);
        QVERIFY(doc);
        auto choices = [&](QTextCursor *cursor, bool pick) {
            int count = -1;
            QTimer::singleShot(0, [&count, pick] {
                auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dlg) return;
                auto *applyTo = dlg->findChild<QComboBox *>(QStringLiteral("applyTo"));
                count = applyTo ? applyTo->count() : -1;
                if (pick && applyTo && applyTo->count() > 2) {
                    applyTo->setCurrentIndex(2);
                    dlg->findChild<MeasureSpin *>(QStringLiteral("left"))->setValue(36);
                }
                dlg->accept();
            });
            pageSetupDialog(nullptr, doc.get(), 0, cursor);
            return count;
        };
        QCOMPARE(choices(nullptr, false), 2);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(1).position() + 1);
        QCOMPARE(choices(&c, true), 3);
        QCOMPARE(doc->sectionCount(), 2);
        QCOMPARE(pageSetupOf(doc->sectionProps(1), false).left, 36.0);
        QCOMPARE(pageSetupOf(doc->sectionProps(0), false).left, 72.0);
    }

    // ---------------------------------------------------------------- Header & Footer
    void theHeaderAndFooterDistancesOnTheTabSetTheSectionsPageMargins()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        w.view()->paginateAll();
        QVERIFY(w.view()->enterHeaderFooter(0, false, true));
        auto *header = w.findChild<MeasureSpin *>(QStringLiteral("headerFromTopField"));
        auto *footer = w.findChild<MeasureSpin *>(QStringLiteral("footerFromBottomField"));
        QVERIFY(header && footer);
        w.refreshNow();
        QCOMPARE(header->value(), w.document()->sectionProps(0).header / 20.0);
        QCOMPARE(footer->value(), w.document()->sectionProps(0).footer / 20.0);
        header->setValue(18);
        Q_EMIT header->editingFinished();
        footer->setValue(54);
        Q_EMIT footer->editingFinished();
        QCOMPARE(w.document()->sectionProps(0).header, 360);
        QCOMPARE(w.document()->sectionProps(0).footer, 1080);
        // Each is one step that undoes, and the file says it.
        const QByteArray saved = w.document()->toDocx();
        office::Package p;
        p.read(saved);
        const QByteArray body = p.part(QStringLiteral("/word/document.xml"));
        QVERIFY2(body.contains("w:header=\"360\"") && body.contains("w:footer=\"1080\""), body.constData());
        w.document()->undo();
        QCOMPARE(w.document()->sectionProps(0).footer, 720);
        QCOMPARE(w.document()->sectionProps(0).header, 360);
        // A value that is no change is no step.
        w.document()->redo();
        const bool could = w.document()->canUndo();
        Q_EMIT header->editingFinished();
        QCOMPARE(w.document()->sectionProps(0).header, 360);
        QVERIFY(could);
    }

    void showDocumentTextCoversThePagesTextWhileAHeaderIsEdited()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(1100, 800);
        w.show();
        QVERIFY(w.act("hf.showDocumentText"));
        QVERIFY(!w.act("hf.showDocumentText")->isEnabled());   // (only while a header or footer is edited)
        w.view()->paginateAll();
        QVERIFY(w.view()->enterHeaderFooter(0, false, true));
        w.refreshNow();
        QVERIFY(w.act("hf.showDocumentText")->isEnabled() && w.act("hf.showDocumentText")->isChecked());
        bool covered = true;
        setPaintObserver([&](const flow::MaterializedPage &, const PageDrawing &d) { covered = d.hideBody; });
        w.view()->grab();
        QVERIFY(!covered);
        w.act(QStringLiteral("hf.showDocumentText"))->trigger();
        QVERIFY(!w.view()->showBodyWhileEditingHeader());
        w.view()->grab();
        setPaintObserver(nullptr);
        QVERIFY(covered);
    }

    void wordCountHasTheLinesTheLayoutMade()
    {
        auto doc = outlineDoc({{"First line", kBodyLevel}, {"Second line", kBodyLevel}, {"Third line", kBodyLevel}});
        QTextCursor none(doc->story());
        QCOMPARE(countWords(*doc, none, false).lines, 3);
        QTextCursor second(doc->story());
        second.setPosition(outlineRows(*doc)[1].position);
        second.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QCOMPARE(countWords(*doc, second, false).lines, 1);
        QTextCursor two(doc->story());
        two.setPosition(outlineRows(*doc)[1].position + 3);
        two.setPosition(outlineRows(*doc)[2].position + 3, QTextCursor::KeepAnchor);
        QCOMPARE(countWords(*doc, two, false).lines, 2);
        // A paragraph of many lines counts them all.
        auto narrow = narrowDoc(QByteArray());
        QVERIFY(narrow);
        QTextCursor whole(narrow->story());
        QVERIFY(countWords(*narrow, whole, false).lines >= hyphenRun(*narrow).second);   // (that is the first page's lines; the count goes on over the pages)
        QVERIFY(countWords(*narrow, whole, false).lines > 10);
    }

    void splitShowsTheSameDocumentInASecondPane()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 800);
        w.show();
        QVERIFY(w.act("view.split") && !w.act("view.split")->isChecked());
        QVERIFY(!w.splitView() || !w.splitView()->isVisible());
        w.act(QStringLiteral("view.split"))->trigger();
        QVERIFY(w.splitView() && w.splitView()->isVisible() && w.view()->isVisible());
        QCOMPARE(w.splitView()->document(), w.document());
        QVERIFY(w.splitView()->readOnly());   // (the second pane is for looking: the words are written in the first)
        // What is written in the first pane is in the second: pages added there are counted there too.
        QTextCursor c = w.view()->cursor();
        insertText(*w.document(), c, QStringLiteral("shared words"));
        insertBlankPage(*w.document(), c);
        insertBlankPage(*w.document(), c);
        w.view()->paginateAll();
        w.splitView()->paginateAll();
        QCOMPARE(w.splitView()->pageCount(), w.view()->pageCount());
        QVERIFY(w.view()->pageCount() >= 3);
        // The two scroll apart.
        w.splitView()->goToPage(2);
        QVERIFY(w.splitView()->topPage() != w.view()->topPage());
        // A new document replaces the one both show.
        w.setDocument(WriteDoc::blank(), QString());
        QCOMPARE(w.splitView()->document(), w.document());
        // Closing the split leaves the one pane.
        w.act(QStringLiteral("view.split"))->trigger();
        QVERIFY(!w.splitView()->isVisible() && w.view()->isVisible());
        QVERIFY(!w.splitView()->document());
    }

    // View > Window: a window on the same file, the windows placed in rows, and two windows side by side that scroll together until the view is turned off.
    void windowsCanBeOpenedPlacedAndScrolledTogether()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("two.docx"));
        WriteWindow a;
        a.setNotifier([](const QString &, const QString &) {});
        a.resize(700, 600);
        a.show();
        QTextCursor c = a.view()->cursor();
        insertText(*a.document(), c, QStringLiteral("window words"));
        for (int i = 0; i < 6; ++i) insertBlankPage(*a.document(), c);
        a.view()->paginateAll();
        QVERIFY(a.saveTo(path));
        // View Side by Side needs a second window: with one it says so and stays off.
        QVERIFY(WriteWindow::writeWindows().size() == 1);
        a.act(QStringLiteral("view.sideBySide"))->trigger();
        QVERIFY(!a.act(QStringLiteral("view.sideBySide"))->isChecked());
        QVERIFY(!a.sideBySidePartner());
        a.refreshNow();
        QVERIFY(!a.act(QStringLiteral("view.resetWindowPosition"))->isEnabled());
        // New Window opens the same file in a window of its own.
        const int before = WriteWindow::writeWindows().size();
        a.act(QStringLiteral("win.new"))->trigger();
        QCOMPARE(WriteWindow::writeWindows().size(), before + 1);
        WriteWindow *b = nullptr;
        for (WriteWindow *w : WriteWindow::writeWindows())
            if (w != &a) b = w;
        QVERIFY(b);
        QVERIFY(b->document()->story()->toPlainText().contains(QLatin1String("window words")));
        b->setNotifier([](const QString &, const QString &) {});
        b->resize(700, 600);
        b->view()->paginateAll();
        a.act(QStringLiteral("win.arrange"))->trigger();
        QVERIFY(a.geometry().right() < b->geometry().left() || b->geometry().right() < a.geometry().left());
        // Side by side: both are marked, each is the other's partner, and a scroll in one moves the other by the same share.
        a.act(QStringLiteral("view.sideBySide"))->trigger();
        QVERIFY(a.act(QStringLiteral("view.sideBySide"))->isChecked());
        QCOMPARE(a.sideBySidePartner(), b);
        QCOMPARE(b->sideBySidePartner(), &a);
        QVERIFY(a.act(QStringLiteral("view.resetWindowPosition"))->isEnabled());
        QVERIFY(b->act(QStringLiteral("view.sideBySide"))->isChecked());
        QScrollBar *sa = a.view()->verticalScrollBar(), *sb = b->view()->verticalScrollBar();
        QVERIFY(sa->maximum() > 0 && sb->maximum() > 0);
        sa->setValue(sa->maximum() / 2);
        QVERIFY2(qAbs(sb->value() - sb->maximum() / 2) <= 2, qPrintable(QStringLiteral("%1 of %2").arg(sb->value()).arg(sb->maximum())));
        sb->setValue(0);
        QCOMPARE(sa->value(), 0);
        // Reset Window Position puts the pair back on the two halves of the screen, whatever was done to them.
        a.move(5, 5);
        b->move(7, 9);
        a.act(QStringLiteral("view.resetWindowPosition"))->trigger();
        QVERIFY(a.geometry().right() < b->geometry().left());
        QCOMPARE(a.geometry().top(), b->geometry().top());
        // Turned off, they scroll apart again, and neither is marked.
        a.act(QStringLiteral("view.sideBySide"))->trigger();
        QVERIFY(!a.sideBySidePartner() && !b->sideBySidePartner());
        QVERIFY(!a.act(QStringLiteral("view.resetWindowPosition"))->isEnabled());
        QVERIFY(!a.act(QStringLiteral("view.sideBySide"))->isChecked() && !b->act(QStringLiteral("view.sideBySide"))->isChecked());
        sa->setValue(sa->maximum());
        QCOMPARE(sb->value(), 0);
        delete b;
    }

    // Display: hidden text is left out of the lines, laid out with the rest when the view asks, and never in a file or on paper.
    void hiddenTextCanBeShownOnTheScreenAndStaysOutOfTheTextExport()
    {
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.resize(900, 800);
        w.show();
        QTextCursor c = w.view()->cursor();
        insertText(*w.document(), c, QStringLiteral("first"));
        insertParagraph(*w.document(), c);
        insertText(*w.document(), c, QStringLiteral("hidden words ").repeated(60));
        insertParagraph(*w.document(), c);
        insertText(*w.document(), c, QStringLiteral("last"));
        QTextCursor mid(w.document()->story());
        mid.setPosition(w.document()->story()->findBlockByNumber(1).position());
        mid.setPosition(w.document()->story()->findBlockByNumber(2).position() - 1, QTextCursor::KeepAnchor);
        QTextCharFormat hide;
        hide.setProperty(tp::HiddenText, true);
        mid.mergeCharFormat(hide);
        w.view()->paginateAll();
        auto lines = [&] { return int(w.document()->pager().lines(0).size()); };
        const int without = lines();
        QVERIFY(!w.document()->showsHiddenText());
        w.document()->setShowHiddenText(true);
        w.view()->paginateAll();
        QVERIFY2(lines() > without + 3, qPrintable(QStringLiteral("%1 and %2").arg(lines()).arg(without)));
        // The text export and the properties of the runs are the same either way.
        QVERIFY(!visibleText(*w.document()).contains(QLatin1String("hidden")));
        w.document()->setShowHiddenText(false);
        w.view()->paginateAll();
        QCOMPARE(lines(), without);
    }
};

QTEST_MAIN(Tabs2Tests)
#include "tst_write_tabs2.moc"