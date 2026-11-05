// Tests of what the Layout, References, Review, and View tabs, the outline, and the right-click menus add.

#include "../isolatesettings.h"
#include "app/help.h"
#include "app/writeoutlineview.h"
#include "app/pagepaint.h"
#include "app/pageview.h"
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
#include "core/writedoc.h"
#include "app/widgets.h"

#include "office/package.h"
#include <QAction>
#include <QBuffer>
#include <QImage>
#include <QRegularExpression>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QTimer>
#include <QDockWidget>
#include <QLabel>
#include <QMenu>
#include <QPair>
#include <QScrollBar>
#include <QTest>
#include <QTextCursor>
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
};

QTEST_MAIN(Tabs2Tests)
#include "tst_write_tabs2.moc"