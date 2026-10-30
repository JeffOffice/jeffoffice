// The checks every app's window must pass, for JeffDeck (see appchecks.h): its first five minutes, its standard commands, and every command it has.
#include "appchecks.h"

#include "app/animationpane.h"
#include "app/deckcanvas.h"
#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/filescreen.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/slidespane.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "testrunner.h"

#include <QClipboard>
#include <QDockWidget>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QStatusBar>
#include <memory>

using namespace jp;
using namespace appchecks;

namespace {

DeckWindow *asDeck(AppWindow *w) { return static_cast<DeckWindow *>(w); }

// The window as launching the program makes it: a new presentation with its title slide.
DeckWindow *launchBlank()
{
    auto *w = new DeckWindow();
    w->resize(1100, 700);
    w->show();
    return w;
}

// A click in the title of the slide on show: it is selected, and what is typed next goes in it.
void clickTitle(DeckWindow *w)
{
    QTest::mouseClick(w->canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, w->canvas()->pageToView(w->editor()->doc()->pages[w->editor()->currentPage()]->items.front()->rect.center()).toPoint());
}

// The words of every text box on the slides, one after the other.
QString allText(DeckWindow *w)
{
    QString out;
    for (const auto &page : w->editor()->doc()->pages)
        for (const auto &item : page->items)
            if (item->type() == ItemType::Text)
                if (QTextDocument *story = w->editor()->doc()->storyDoc(static_cast<TextItem *>(item.get())->storyId); story && !story->isEmpty()) out += story->toPlainText() + QLatin1Char('|');
    return out;
}

} // namespace

class DeckAppChecks : public QObject {
    Q_OBJECT

    Adapter m_app;

private Q_SLOTS:
    void initTestCase()
    {
        isolateSettings();
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(deck);
        setAppInfo(deckAppInfo());
        initCore();
        m_app.name = QStringLiteral("JeffDeck");
        m_app.launch = [] { return static_cast<AppWindow *>(launchBlank()); };
        m_app.launchWith = [](const QString &path) -> AppWindow * {
            auto *w = launchBlank();
            if (!w->openFile(path)) {
                delete w;
                return nullptr;
            }
            return w;
        };
        m_app.type = [](AppWindow *w) {
            clickTitle(asDeck(w));
            typeInto(asDeck(w)->canvas(), QStringLiteral("Some words"));
            asDeck(w)->editor()->flushTyping();   // (as a pause in typing does)
        };
        m_app.documentView = [](AppWindow *w) -> QWidget * { return asDeck(w)->canvas()->viewport(); };
        m_app.settle = [](AppWindow *w) {
            drain();
            asDeck(w)->refreshNow();
            asDeck(w)->slidesPane()->refresh(false);
            drain();
        };
        m_app.docState = [](AppWindow *aw) {
            auto *w = asDeck(aw);
            Editor *ed = w->editor();
            QString s = zipDigest(deck::exportPresentation(*ed->doc()));
            s += QStringLiteral("|%1|%2|%3|%4|%5|%6|%7").arg(ed->selection().join(QLatin1Char(','))).arg(ed->isEditingText()).arg(ed->undoStack()->index()).arg(ed->currentPage()).arg(w->canvas()->zoom()).arg(int(w->view())).arg(int(ed->tool()));
            if (ed->isEditingText()) s += QStringLiteral("|%1,%2").arg(ed->cursor().position()).arg(ed->cursor().anchor());
            return s + QLatin1Char('|') + ed->filePath() + QString::number(ed->isModified());
        };
        // The sample: the title slide, and a second slide with its words, a shape, a picture, and a table, all made and saved by the program.
        {
            std::unique_ptr<DeckWindow> w(launchBlank());
            clickTitle(w.get());
            typeInto(w->canvas(), QStringLiteral("A sample deck"));
            w->editor()->flushTyping();
            w->newSlide();
            Editor *ed = w->editor();
            Document *d = ed->doc();
            ed->setCurrentPage(1);
            QTextCursor(d->storyDoc(static_cast<TextItem *>(d->pages[1]->items[0].get())->storyId)).insertText(QStringLiteral("second SLIDE here"));
            if (d->pages[1]->items.size() > 1) QTextCursor(d->storyDoc(static_cast<TextItem *>(d->pages[1]->items[1].get())->storyId)).insertText(QStringLiteral("First point\nSecond point\nThird point"));
            auto shape = std::make_shared<ShapeItem>();
            shape->shape = QStringLiteral("rect");
            shape->rect = QRectF(60, 400, 200, 80);
            shape->storyId = d->createStory(QStringLiteral("A shape"));
            d->pages[1]->items.push_back(shape);
            QImage red(40, 30, QImage::Format_RGB32);
            red.fill(Qt::red);
            QVERIFY(red.save(tempFile(QStringLiteral("red.png"))));
            QVERIFY(w->insertPicture(tempFile(QStringLiteral("red.png")), QPointF(300, 400)));
            w->insertTable(2, 2);
            ed->setCurrentPage(0);
            QVERIFY(w->saveTo(tempFile(QStringLiteral("sample.pptx"))));
        }
        m_app.sampleFile = tempFile(QStringLiteral("sample.pptx"));
        QVERIFY(QFileInfo::exists(m_app.sampleFile));
        m_app.reset = [this](AppWindow *aw, bool fromFile) {
            auto *w = asDeck(aw);
            w->showNormal();
            w->resize(1100, 700);   // (a window command may have made it another size)
            w->hideFileScreen();
            w->setView(DeckWindow::View::Normal);
            for (QDockWidget *dock : w->findChildren<QDockWidget *>()) dock->hide();
            w->editor()->endTextEdit();
            w->editor()->setTool(Tool::Select);
            w->editor()->clearSelection();
            if (fromFile) {
                w->editor()->setDocument(deck::importPresentationFile(m_app.sampleFile), m_app.sampleFile);
            } else {
                w->editor()->setDocument(deck::newPresentation());
            }
            w->editor()->setCurrentPage(0);
        };
        // The places a person is in when a command is there to use: a title selected, words selected in it, and (in the sample) a shape, a picture, or a table selected.
        auto onSlide = [](AppWindow *aw, bool fromFile, ItemType type) {
            Editor *ed = asDeck(aw)->editor();
            if (fromFile) ed->setCurrentPage(1);
            for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                if (it->type() == type) {
                    ed->select(it->id);
                    return;
                }
        };
        auto wordsSelected = [](AppWindow *aw, bool fromFile) {
            Editor *ed = asDeck(aw)->editor();
            if (fromFile) ed->setCurrentPage(1);
            const auto &first = ed->doc()->pages[ed->currentPage()]->items.front();
            if (QTextDocument *story = ed->doc()->storyDoc(static_cast<TextItem *>(first.get())->storyId); story && story->isEmpty()) QTextCursor(story).insertText(QStringLiteral("some WORDS here"));
            ed->select(first->id);
            ed->beginTextEdit(first->id, 0);
            QTextCursor c = ed->cursor();
            c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            ed->setCursor(c);
        };
        m_app.contexts = {
            {QStringLiteral("a title selected"), [onSlide](AppWindow *w, bool f) { onSlide(w, f, ItemType::Text); }, false},
            {QStringLiteral("words selected"), wordsSelected, false},
            {QStringLiteral("a shape selected"), [onSlide](AppWindow *w, bool f) { onSlide(w, f, ItemType::Shape); }, true},
            {QStringLiteral("a picture selected"), [onSlide](AppWindow *w, bool f) { onSlide(w, f, ItemType::Picture); }, true},
            {QStringLiteral("a table selected"), [onSlide](AppWindow *w, bool f) { onSlide(w, f, ItemType::Table); }, true},
            {QStringLiteral("the cursor in a table cell"), [onSlide](AppWindow *aw, bool f) {
                 onSlide(aw, f, ItemType::Table);
                 Editor *ed = asDeck(aw)->editor();
                 ed->beginTextEdit(ed->selection().first(), 0, 0, 0);
             }, true},
            {QStringLiteral("cells of a table selected"), [onSlide](AppWindow *aw, bool f) {
                 onSlide(aw, f, ItemType::Table);
                 Editor *ed = asDeck(aw)->editor();
                 ed->selectCells(ed->selection().first(), 0, 0, 1, 1);
             }, true},
        };
        m_app.prepare = [](AppWindow *, const QString &id, bool) {
            if (id == QLatin1String("edit.paste")) QGuiApplication::clipboard()->setText(QStringLiteral("Pasted words"));
        };
        m_app.skipped = {};
        // What a command may leave as it is, where what it sets is so already.
        m_app.quiet = {
            {QStringLiteral("design.slideSize"), QStringLiteral("the name of the Slide Size menu: its choices are the commands, and it has none of its own")},
            {QStringLiteral("design.sizeWidescreen"), QStringLiteral("the slides are widescreen already")},
            {QStringLiteral("show.usePresenterView"), QStringLiteral("a setting for the next show")},
            {QStringLiteral("view.fit"), QStringLiteral("a window begins with the slide fitted")},
            {QStringLiteral("view.normal"), QStringLiteral("a window begins in Normal view")},
            {QStringLiteral("transition.none"), QStringLiteral("no slide has a transition to take away")},
            {QStringLiteral("cols.1"), QStringLiteral("a box has one column already")},
            {QStringLiteral("ls.1"), QStringLiteral("the lines are single spaced already")},
            {QStringLiteral("valign.*"), QStringLiteral("a box is aligned one of these ways already, and the command for that way has nothing to change")},
            {QStringLiteral("tb.dirHorizontal"), QStringLiteral("the words run across already")},
            {QStringLiteral("arr.back@blank"), QStringLiteral("the title is at the back already")},
            {QStringLiteral("arr.backward@blank"), QStringLiteral("the title is at the back already")},
            {QStringLiteral("para.indentDec@file"), QStringLiteral("a paragraph with no indent has none to take away")},
            {QStringLiteral("pic.aspect.4x3"), QStringLiteral("the sample's picture is 4 by 3 already")},
            {QStringLiteral("pic.clearCrop"), QStringLiteral("the picture is not cropped")},
            {QStringLiteral("pic.fill"), QStringLiteral("the picture fills its frame already")},
            {QStringLiteral("pic.fit"), QStringLiteral("the picture fits its frame already")},
            {QStringLiteral("tbl.alignTop"), QStringLiteral("the cells are aligned to the top already")},
            {QStringLiteral("tbl.marginNarrow"), QStringLiteral("the cells have these margins already")},
            {QStringLiteral("tbl.distributeCols"), QStringLiteral("the sample's columns are equal already")},
            {QStringLiteral("tbl.distributeRows"), QStringLiteral("the sample's rows are equal already")},
        };
    }

    // ---- 1. the first five minutes
    void firstFiveMinutes()
    {
        Watch watch;
        std::unique_ptr<DeckWindow> w(launchBlank());
        QVERIFY(w->windowTitle().startsWith(QLatin1String("Presentation")));
        QVERIFY(w->windowTitle().contains(QLatin1String("JeffDeck")));
        QVERIFY(!w->editor()->isModified());

        // The title of the first slide takes typing, and the title shows it.
        clickTitle(w.get());
        typeInto(w->canvas(), QStringLiteral("Hello there"));   // (a click selects the empty title, and typing begins in it)
        QVERIFY(w->editor()->isEditingText());
        QCOMPARE(allText(w.get()), QStringLiteral("Hello there|"));
        // (keystrokes are one step of Undo after a pause in typing: the title shows the change then)
        QVERIFY(waitFor([&] { return w->editor()->isModified(); }));
        QVERIFY2(waitFor([&] { return w->windowTitle().contains(QLatin1Char('*')); }), qPrintable(w->windowTitle()));
        QVERIFY2(!saysNotYet(w->statusBar()->currentMessage()), qPrintable(w->statusBar()->currentMessage()));

        // Undo takes it back, Redo puts it again.
        QAction *undo = w->act(QStringLiteral("edit.undo")), *redo = w->act(QStringLiteral("edit.redo"));
        QVERIFY(waitFor([&] { return undo->isEnabled(); }));
        for (int i = 0; i < 40 && undo->isEnabled(); ++i) {
            undo->trigger();
            drain();
        }
        QCOMPARE(allText(w.get()), QString());
        QVERIFY(waitFor([&] { return redo->isEnabled(); }));
        for (int i = 0; i < 40 && redo->isEnabled(); ++i) {
            redo->trigger();
            drain();
        }
        QCOMPARE(allText(w.get()), QStringLiteral("Hello there|"));

        // Save As in the program's own format: the title takes the file's name.
        w->act(QStringLiteral("file.saveAs"))->trigger();
        const QString path = w->editor()->filePath();
        QVERIFY(path.endsWith(QLatin1String(".pptx")));
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(!w->editor()->isModified());
        QVERIFY2(w->windowTitle().startsWith(QFileInfo(path).completeBaseName()), qPrintable(w->windowTitle()));
        QVERIFY2(!w->windowTitle().contains(QLatin1Char('*')), qPrintable(w->windowTitle()));

        // The saved file is among the recent ones, on the File screen's Open page.
        QVERIFY(Settings::get().recentFiles().contains(path));
        w->showFileScreen(QStringLiteral("open"));
        QVERIFY(listsRecentFile(w->findChild<FileScreen *>(), path));
        w->hideFileScreen();

        // Closed (nothing changed since the save: no question), and opened again: the same words.
        const CloseResult closed = closeWindow(w.get(), watch);
        QVERIFY(closed.closed);
        QCOMPARE(closed.asked, 0);
        w.reset();
        std::unique_ptr<DeckWindow> again(asDeck(m_app.launchWith(path)));
        QVERIFY(again);
        QCOMPARE(allText(again.get()), QStringLiteral("Hello there|"));
        QVERIFY(!again->editor()->isModified());
        QCOMPARE(again->editor()->filePath(), path);

        // File > New: a second window with a new presentation that takes typing.
        const int windows = AppWindow::windows().size();
        again->act(QStringLiteral("file.new"))->trigger();
        drain();
        QCOMPARE(AppWindow::windows().size(), windows + 1);
        DeckWindow *second = nullptr;
        for (AppWindow *other : AppWindow::windows())
            if (other != again.get()) second = asDeck(other);
        QVERIFY(second);
        QPointer<DeckWindow> guard(second);
        QVERIFY(second->windowTitle().startsWith(QLatin1String("Presentation")));
        clickTitle(second);
        typeInto(second->canvas(), QStringLiteral("Second"));
        QCOMPARE(allText(second), QStringLiteral("Second|"));
        // (a changed presentation asks before it goes; the answer here is no)
        const CloseResult asked = closeWindow(second, watch);
        QCOMPARE(asked.asked, 1);
        QVERIFY(!asked.closed);
        second->setAttribute(Qt::WA_DeleteOnClose, false);
        delete second;

        // The print layout: the File screen's Print page, with the slide drawn.
        again->act(QStringLiteral("file.print"))->trigger();
        auto *screen = again->findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
        QVERIFY(waitFor([&] {
            const QLabel *preview = screen->findChild<QLabel *>(QStringLiteral("printPreview"));
            return preview && !preview->pixmap().isNull();
        }));
        again->hideFileScreen();

        // An unchanged presentation goes without a question.
        const CloseResult plain = closeWindow(again.get(), watch);
        QVERIFY(plain.closed);
        QCOMPARE(plain.asked, 0);
        QVERIFY2(watch.warnings.isEmpty(), qPrintable(watch.warnings.join('\n')));
    }

    // The way these checks see a window does not change by itself: a call that does nothing is seen to do nothing (or every command would pass).
    void aCallThatDoesNothingChangesNothing()
    {
        Watch watch;
        std::unique_ptr<AppWindow> w(m_app.launch());
        m_app.settle(w.get());
        const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 300);
        QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
    }

    // A preview of the effects plays the presentation in the window; when another is put in its place, the preview ends (it would play what is gone).
    void aPreviewEndsWhenThePresentationIsReplaced()
    {
        std::unique_ptr<DeckWindow> w(launchBlank());
        auto shape = std::make_shared<ShapeItem>();
        shape->shape = QStringLiteral("rect");
        shape->rect = QRectF(60, 300, 120, 80);
        w->editor()->select(w->editor()->addItem(shape));
        w->refreshNow();
        w->act(QStringLiteral("anim.add.entr.fade"))->trigger();
        w->refreshNow();
        w->act(QStringLiteral("anim.preview"))->trigger();
        QVERIFY(w->canvas()->findChild<AnimationPreview *>());
        w->editor()->setDocument(deck::newPresentation());
        drain();
        QVERIFY2(!w->canvas()->findChild<AnimationPreview *>(), "the preview goes on playing a presentation that is gone");
    }

    // ---- 2. the standard commands
    void standardCommands()
    {
        const QStringList fails = checkStandardCommands(m_app);
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    // ---- 3. every command
    void everyCommandOnABlankDocument()
    {
        QStringList report;
        const QStringList fails = runEveryAction(m_app, false, &report);
        qInfo("%s", qPrintable(report.last()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    void everyCommandOnADocumentFromAFile()
    {
        QStringList report;
        const QStringList fails = runEveryAction(m_app, true, &report);
        qInfo("%s", qPrintable(report.last()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
};

JP_TEST_CLASS(DeckAppChecks)
#include "tst_appchecks_deck.moc"
