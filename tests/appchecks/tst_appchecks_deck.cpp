// The checks every app's window must pass, for JeffDeck (see appchecks.h): its first five minutes, its standard commands, and every command it has.
#include "appchecks.h"

#include "app/animationpane.h"
#include "app/deckcanvas.h"
#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/filescreen.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "core/appinfo.h"
#include "app/commentspane.h"
#include "core/comments.h"
#include "core/deck.h"
#include "core/media.h"
#include "core/sections.h"
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
        m_app.showPrint = [](AppWindow *w) -> QWidget * {
            w->act(QStringLiteral("file.print"))->trigger();
            return w->findChild<FileScreen *>();
        };
        m_app.reopen = [](const QString &path) -> QString {
            if (QFileInfo(path).suffix().toLower() != QLatin1String("pptx")) return {};
            QString err;
            return deck::importPresentationFile(path, &err) ? QString() : err.isEmpty() ? QStringLiteral("it is not read") : err;
        };
        m_app.settle = [](AppWindow *w) {
            drain();
            asDeck(w)->refreshNow();
            drain();
        };
        m_app.docState = [](AppWindow *aw) {
            auto *w = asDeck(aw);
            Editor *ed = w->editor();
            QString s = zipDigest(deck::exportPresentation(*ed->doc()));
            s += QStringLiteral("|%1|%2|%3|%4|%5|%6|%7").arg(ed->selection().join(QLatin1Char(','))).arg(ed->isEditingText()).arg(ed->undoStack()->index()).arg(ed->currentPage()).arg(w->canvas()->zoom()).arg(int(w->view())).arg(int(ed->tool()));
            if (ed->isEditingText()) s += QStringLiteral("|%1,%2").arg(ed->cursor().position()).arg(ed->cursor().anchor());
            s += QLatin1String("|comment ") + w->commentsPane()->currentComment();
            s += QStringLiteral("|view%1%2|presenter%3").arg(w->canvas()->gridlines()).arg(w->canvas()->guides()).arg(SlideShow::usePresenterView());
            const auto block = ed->cellBlock();
            s += QStringLiteral("|cells%1,%2,%3,%4").arg(block.range.valid()).arg(block.range.r0).arg(block.range.c0).arg(block.range.r1 * 100 + block.range.c1);
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
            if (d->pages[1]->items.size() > 1) QTextCursor(d->storyDoc(static_cast<TextItem *>(d->pages[1]->items[1].get())->storyId)).insertText(QStringLiteral("First point\nTeh second point\nThird point"));
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
            if (w->slideShow()) w->slideShow()->close();   // (a show or a preview still going on would play the presentation that is taken away)
            drain();
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
            if (!fromFile && type == ItemType::Table) asDeck(aw)->insertTable(2, 2);   // (a blank slide has none)
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
             }, false},
            {QStringLiteral("cells of a table selected"), [onSlide](AppWindow *aw, bool f) {
                 onSlide(aw, f, ItemType::Table);
                 Editor *ed = asDeck(aw)->editor();
                 ed->selectCells(ed->selection().first(), 0, 0, 1, 1);
             }, false},
        };
        auto addShape = [](AppWindow *aw, const QRectF &rect) {
            auto shape = std::make_shared<ShapeItem>();
            shape->shape = QStringLiteral("rect");
            shape->rect = rect;
            return asDeck(aw)->editor()->addItem(shape);
        };
        auto effectOn = [](AppWindow *aw, const QString &id) {
            asDeck(aw)->editor()->select(id);
            asDeck(aw)->refreshNow();
            aw->act(QStringLiteral("anim.add.entr.fade"))->trigger();
        };
        m_app.contexts += QList<Adapter::Context>{
            {QStringLiteral("a change to undo"), [](AppWindow *aw, bool) {
                 clickTitle(asDeck(aw));
                 typeInto(asDeck(aw)->canvas(), QStringLiteral("more"));
                 asDeck(aw)->editor()->flushTyping();
             }, false},
            {QStringLiteral("a change undone"), [](AppWindow *aw, bool) {
                 clickTitle(asDeck(aw));
                 typeInto(asDeck(aw)->canvas(), QStringLiteral("more"));
                 asDeck(aw)->editor()->flushTyping();
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("edit.undo"))->trigger();
             }, false},
            {QStringLiteral("two shapes selected"), [addShape](AppWindow *aw, bool) {
                 const QString a = addShape(aw, QRectF(60, 300, 120, 80)), b = addShape(aw, QRectF(260, 340, 120, 80));
                 asDeck(aw)->editor()->select(QStringList{a, b});
             }, false},
            {QStringLiteral("a group selected"), [addShape](AppWindow *aw, bool) {
                 const QString a = addShape(aw, QRectF(60, 300, 120, 80)), b = addShape(aw, QRectF(260, 340, 120, 80));
                 asDeck(aw)->editor()->select(QStringList{a, b});
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("arr.group"))->trigger();
             }, false},
            {QStringLiteral("the first of two effects selected"), [addShape, effectOn](AppWindow *aw, bool) {
                 const QString a = addShape(aw, QRectF(60, 300, 120, 80)), b = addShape(aw, QRectF(260, 340, 120, 80));
                 effectOn(aw, a);
                 effectOn(aw, b);
                 asDeck(aw)->editor()->select(a);
             }, false},
            {QStringLiteral("the second of two effects selected"), [addShape, effectOn](AppWindow *aw, bool) {
                 const QString a = addShape(aw, QRectF(60, 300, 120, 80)), b = addShape(aw, QRectF(260, 340, 120, 80));
                 effectOn(aw, a);
                 effectOn(aw, b);
                 asDeck(aw)->editor()->select(b);
             }, false},
            {QStringLiteral("Slide Master view on a layout"), [](AppWindow *aw, bool) { asDeck(aw)->editMasters(); }, false},
            {QStringLiteral("Slide Master view on a slide master"), [](AppWindow *aw, bool) { asDeck(aw)->editMasters(asDeck(aw)->editor()->doc()->masters.first()->id); }, false},
            {QStringLiteral("a comment, chosen"), [](AppWindow *aw, bool) {
                 auto *w = asDeck(aw);
                 Page &slide = *w->editor()->doc()->pages[w->editor()->currentPage()];
                 const QString id = deck::addComment(slide, deck::CommentAuthor{QStringLiteral("Test Writer"), QStringLiteral("TW")}, QStringLiteral("A note"), QPointF(100, 100));
                 w->showComments(true);
                 w->refreshNow();
                 w->commentsPane()->setCurrentComment(id);
                 w->refreshNow();
             }, false},
            {QStringLiteral("a section"), [](AppWindow *aw, bool) {
                 auto *w = asDeck(aw);
                 Document *d = w->editor()->doc();
                 w->editor()->change(QStringLiteral("Add Section"), [d] { deck::addSection(*d, 0, QStringLiteral("Part one")); });
                 w->refreshNow();
             }, false},
            {QStringLiteral("two slide masters"), [](AppWindow *aw, bool) {
                 auto *w = asDeck(aw);
                 w->editMasters();
                 w->insertSlideMaster();
             }, false},
            {QStringLiteral("a misspelled word in the title"), [](AppWindow *aw, bool) {
                 clickTitle(asDeck(aw));
                 typeInto(asDeck(aw)->canvas(), QStringLiteral("Teh quikc brown fox"));
                 asDeck(aw)->editor()->flushTyping();
             }, false},
            {QStringLiteral("two comments, the first chosen"), [](AppWindow *aw, bool) {
                 auto *w = asDeck(aw);
                 Page &slide = *w->editor()->doc()->pages[w->editor()->currentPage()];
                 const deck::CommentAuthor who{QStringLiteral("Test Writer"), QStringLiteral("TW")};
                 const QString first = deck::addComment(slide, who, QStringLiteral("One"), QPointF(100, 100));
                 deck::addComment(slide, who, QStringLiteral("Two"), QPointF(200, 100));
                 w->showComments(true);
                 w->refreshNow();
                 w->commentsPane()->setCurrentComment(first);
                 w->refreshNow();
             }, false},
            {QStringLiteral("a zoomed window"), [](AppWindow *aw, bool) { asDeck(aw)->canvas()->setZoom(0.3); }, false},
            {QStringLiteral("a table with uneven columns and rows"), [](AppWindow *aw, bool) {
                 auto *w = asDeck(aw);
                 w->insertTable(2, 2);
                 Editor *ed = w->editor();
                 for (const auto &it : ed->doc()->pages[ed->currentPage()]->items)
                     if (it->type() == ItemType::Table) {
                         auto *t = static_cast<TableItem *>(it.get());
                         t->colW = {80, 300};
                         t->rowH = {30, 90};
                         ed->select(it->id);
                     }
             }, false},
            {QStringLiteral("a push transition"), [](AppWindow *aw, bool) {
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("transition.push"))->trigger();
             }, false},
            {QStringLiteral("a zoom transition"), [](AppWindow *aw, bool) {
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("transition.zoom"))->trigger();
             }, false},
            {QStringLiteral("a split transition"), [](AppWindow *aw, bool) {
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("transition.split"))->trigger();
             }, false},
            {QStringLiteral("a morph transition"), [](AppWindow *aw, bool) {
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("transition.morph"))->trigger();
             }, false},
            {QStringLiteral("a transition on the slide"), [](AppWindow *aw, bool) {
                 asDeck(aw)->refreshNow();
                 aw->act(QStringLiteral("transition.fade"))->trigger();
             }, false},
            {QStringLiteral("a 4:3 slide"), [](AppWindow *aw, bool) { asDeck(aw)->setSlideSize(QSizeF(720, 540)); }, false},
            {QStringLiteral("a sound on the slide, chosen"), [](AppWindow *aw, bool) {
                 auto *w = asDeck(aw);
                 QByteArray wav("RIFF");
                 auto le = [&](quint32 v, int n) { for (int i = 0; i < n; ++i) wav.append(char((v >> (8 * i)) & 0xff)); };
                 le(36 + 200, 4); wav += "WAVEfmt "; le(16, 4); le(1, 2); le(1, 2); le(8000, 4); le(16000, 4); le(2, 2); le(16, 2); wav += "data"; le(200, 4); wav += QByteArray(200, '\0');
                 Document *d = w->editor()->doc();
                 auto pic = deck::newMediaPicture(*d, *d->pages[w->editor()->currentPage()], wav, QStringLiteral("tone.wav"), QImage(), 1000);
                 if (pic) {
                     w->editor()->surfaceItems().push_back(pic);
                     w->editor()->select(pic->id);
                 }
             }, false},
        };
        m_app.prepare = [](AppWindow *w, const QString &id, bool) {
            asDeck(w)->showFormatPane(false);   // (the pane is open or shut as the person left it, in the next window too)
            if (id == QLatin1String("edit.paste")) QGuiApplication::clipboard()->setText(QStringLiteral("Pasted words"));
        };
        m_app.skipped = {};
        m_app.quiet = {
            {QStringLiteral("design.slideSize"), QStringLiteral("the name of the Slide Size menu: its choices are the commands, and it has none of its own")},
            {QStringLiteral("anim.painter"), QStringLiteral("it arms the animation painter, and nothing shows until a shape is clicked")},
            {QStringLiteral("review.wordCount"), QStringLiteral("it tells the counts in a message and changes nothing")},
        };
        m_app.neverRun = {};
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

        // The print layout: the File screen's Print page, with the slide drawn (not a blank one).
        QVERIFY2(previewShowsTheDocument(m_app, again.get()), "the print preview shows no more than a blank slide");
        auto *screen = again->findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
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
        for (bool fromFile : {false, true}) {
            std::unique_ptr<AppWindow> w(fromFile ? m_app.launchWith(m_app.sampleFile) : m_app.launch());
            QVERIFY(w);
            m_app.settle(w.get());
            const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 3000);
            QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
        }
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

    // Choosing the transition a slide has (or the way it goes) changes nothing, and must leave its mark as it was: on (the menu showed it off, with the slide still having it).
    void choosingWhatTheSlideHasKeepsTheMark()
    {
        std::unique_ptr<DeckWindow> w(launchBlank());
        w->refreshNow();
        w->act(QStringLiteral("transition.push"))->trigger();
        w->refreshNow();
        QAction *direction = w->act(QStringLiteral("transition.dir.l"));   // (a push comes from the right)
        QVERIFY(direction->isChecked());
        direction->trigger();
        QVERIFY2(direction->isChecked(), "choosing the direction the slide has turns its mark off");
        QAction *push = w->act(QStringLiteral("transition.push"));
        push->trigger();
        QVERIFY2(push->isChecked(), "choosing the transition the slide has turns its mark off");
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
    // The commands that no test could enable in either state are named, each with the reason, and no more of them than the list says.
    void commandsNeverRun()
    {
        QStringList report;
        const QStringList fails = checkNeverRun(m_app, &report);
        qInfo("%s", qPrintable(report.first()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
};

JP_TEST_CLASS(DeckAppChecks)
#include "tst_appchecks_deck.moc"
