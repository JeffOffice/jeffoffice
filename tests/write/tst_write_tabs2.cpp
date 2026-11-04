// Tests of what the Layout, References, Review, and View tabs, the outline, and the right-click menus add.

#include "../isolatesettings.h"
#include "app/help.h"
#include "app/outlineview.h"
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
#include "core/outlineops.h"
#include "core/writedoc.h"

#include <QAction>
#include <QDockWidget>
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
};

QTEST_MAIN(Tabs2Tests)
#include "tst_write_tabs2.moc"