// JeffDeck's tests of the 1.0 lines closed in round 45: a window that goes while a pane is typed in, and the small commands of the lists below.
#include "deckhelpers.h"
#include "app/outlineview.h"
#include "app/notespane.h"
#include "../withdialog.h"
#include "app/propertyeditors.h"
#include "app/formatpane.h"
#include "app/filescreen.h"
#include "core/themes.h"
#include "core/printheaderfooter.h"
#include "app/deckdialogs.h"
#include "app/picturetools.h"
#include "core/blipeffects.h"
#include "core/picturefx.h"
#include <QCheckBox>
#include <QJsonDocument>
#include "core/pathops.h"
#include "core/pathpoints.h"
#include "app/shapetools.h"
#include "app/lossywarning.h"
#include <QScopeGuard>
#include <QTimer>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include "io/deckoutput.h"
#include "io/presentationdefaults.h"
#include <QSpinBox>
#include "app/pageaccesspane.h"
#include "core/pageaccess.h"
#include <QListWidget>
#include <QBuffer>
#include <QThread>
#include "app/splitmover.h"
#include <QSplitter>
#include <numeric>
#include <QStatusBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialog>
#include <QPlainTextEdit>
#include "app/commentspane.h"
#include "app/themegallery.h"
#include <QRegularExpression>
#include <QProgressDialog>
#include <QJsonObject>
#include <QFile>
#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>
#include "io/zip.h"
#include "render/renderer.h"
#include "core/blipeffects.h"
#include <QSet>
#include <QComboBox>
#include <QPointer>
#include <QTemporaryDir>

namespace {
// A shape named `name` on the window's first slide, selected.
ShapeItem *addShape(DeckWindow &w, const QString &name, const QRectF &rect)
{
    auto s = std::make_shared<ShapeItem>();
    s->name = name;
    s->rect = rect;
    w.editor()->setCurrentPage(0);
    w.editor()->addItem(s, false);
    w.editor()->select(s->id);
    return s.get();
}

// A picture named `name` on the window's first slide, selected: a red picture with a blue stripe.
PictureItem *addPicture(DeckWindow &w, const QString &name)
{
    QImage img(60, 40, QImage::Format_ARGB32);
    img.fill(QColor(200, 30, 30));
    for (int y = 0; y < 40; ++y)
        for (int x = 30; x < 60; ++x) img.setPixelColor(x, y, QColor(30, 30, 200));
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    auto pic = std::make_shared<PictureItem>();
    pic->name = name;
    pic->imageId = w.editor()->doc()->addImage(png, QStringLiteral("png"));
    pic->rect = QRectF(100, 100, 240, 160);
    pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
    w.editor()->setCurrentPage(0);
    w.editor()->addItem(pic, false);
    w.editor()->select(pic->id);
    return pic.get();
}

PictureItem *pictureNamed(DeckWindow &w, const QString &name)
{
    for (const auto &it : w.editor()->doc()->pages[0]->items)
        if (it->name == name && it->type() == ItemType::Picture) return static_cast<PictureItem *>(it.get());
    return nullptr;
}

// The shape of that name on the first slide of a window's presentation.
ShapeItem *shapeNamed(DeckWindow &w, const QString &name)
{
    for (const auto &it : w.editor()->doc()->pages[0]->items)
        if (it->name == name && it->type() == ItemType::Shape) return static_cast<ShapeItem *>(it.get());
    return nullptr;
}
// Deletes the window and says how many changes were asked of any editor while it went (a pane's commit that reached the editor counts, refused or not).
int entriesWhenDeleted(DeckWindow *w)
{
    const int before = Editor::changeEntries();
    delete w;
    return Editor::changeEntries() - before;
}

// The picture filters' worker, held: the test holds it at the start of each job (counted) so that what a window shows before the filters are done can be read.
struct HeldWorker {
    static std::atomic<bool> &hold() { static std::atomic<bool> h{false}; return h; }
    static std::atomic<int> &entered() { static std::atomic<int> n{0}; return n; }
    HeldWorker()
    {
        hold() = true;
        entered() = 0;
        jp::blipFilterWorkerHook = [](const std::atomic<bool> &stop) {
            if (!hold()) return;
            ++entered();
            for (long i = 0; hold() && !stop && i < 1'000'000'000L; ++i) QThread::yieldCurrentThread();
        };
    }
    void release() { hold() = false; }
    ~HeldWorker() { release(); }   // (the hook stays, off: the thread may still be reading it)
};

// Closes the Format pane and puts the view/formatPane setting back as it was, however the test ends (the pane stays open for later windows otherwise).
struct FormatPaneGuard {
    QPointer<DeckWindow> window;
    QVariant was = Settings::get().value(QStringLiteral("view/formatPane"));
    explicit FormatPaneGuard(DeckWindow *w = nullptr) : window(w) {}
    ~FormatPaneGuard()
    {
        if (window) window->showFormatPane(false);
        if (was.isValid()) Settings::get().setValue(QStringLiteral("view/formatPane"), was);
        else Settings::get().remove(QStringLiteral("view/formatPane"));
    }
};
} // namespace

class DeckLines11Tests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A window deleted while a child holds typing nobody saved: the focus-out and editingFinished that deleting it sends must not begin any change
    // (the count of changes asked of the editor, taken from just before the delete): the window is half gone by then. The old tests called
    // giveUpTyping themselves and proved nothing about the destructor.
    void aWindowGoingAsksNoChangeOfItsNotesPane()
    {
        auto *w = new DeckWindow;
        w->resize(1000, 700);
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        w->activateWindow();
        w->notesPane()->setFocus();
        QTRY_VERIFY(w->notesPane()->hasFocus());
        QTest::keyClicks(w->notesPane(), QStringLiteral("typed"));
        QCOMPARE(w->notesPane()->toPlainText(), QStringLiteral("typed"));
        QCOMPARE(entriesWhenDeleted(w), 0);
    }

    void aWindowGoingAsksNoChangeOfItsOutlineView()
    {
        auto *w = new DeckWindow;
        w->resize(1000, 700);
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        w->act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w->outlineView() && w->outlineView()->isVisible());
        w->activateWindow();
        w->outlineView()->setFocus();
        QTRY_VERIFY(w->outlineView()->hasFocus());
        QTest::keyClicks(w->outlineView(), QStringLiteral("typed"));
        QCOMPARE(entriesWhenDeleted(w), 0);
    }

    // The same, with a combo box's list open as the window goes (the list closing sends events through the application-wide filter while the
    // window's children are freed one by one).
    void aWindowGoingWithAListOpenAsksNothingOfItsOutlineView()
    {
        auto *w = new DeckWindow;
        w->resize(1000, 700);
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        w->act(QStringLiteral("view.outline"))->trigger();
        QVERIFY(w->outlineView() && w->outlineView()->isVisible());
        w->activateWindow();
        w->outlineView()->setFocus();
        QTRY_VERIFY(w->outlineView()->hasFocus());
        QTest::keyClicks(w->outlineView(), QStringLiteral("typed"));
        QVERIFY(w->outlineView()->hasTyping());
        QComboBox *box = nullptr;
        for (auto *c : w->findChildren<QComboBox *>())
            if (c->isVisibleTo(w) && c->count() > 0) { box = c; break; }
        if (box) box->showPopup();
        QCOMPARE(entriesWhenDeleted(w), 0);
    }

    // The view's filter belongs to the view: once the editor it reads is gone, a click anywhere gives up the typing instead of reading the editor.
    void anOutlineViewWhoseEditorIsGoneIsNotAskedByAClick()
    {
        auto *ed = new Editor;
        ed->setDocument(deck::newDefaultPresentation());
        OutlineView view(ed);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        view.setFocus();
        QTest::keyClicks(&view, QStringLiteral("typed"));
        QVERIFY(view.hasTyping());
        delete ed;
        view.hide();   // (a view whose editor is gone is only ever deleted: nothing paints it)
        QWidget other;
        other.show();
        QVERIFY(QTest::qWaitForWindowExposed(&other));
        QTest::mouseClick(&other, Qt::LeftButton);
        QVERIFY(!view.hasTyping());
    }

    // A number typed in the Format pane and not yet entered: the field's editingFinished fires as the window goes.
    void aWindowGoingAsksNoChangeOfAFormatPaneNumber()
    {
        FormatPaneGuard guard;
        auto *w = new DeckWindow;
        w->resize(1200, 800);
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        addShape(*w, QStringLiteral("Spinner"), QRectF(100, 100, 200, 100));
        w->activateWindow();
        QTRY_VERIFY(w->act(QStringLiteral("arr.rotMore"))->isEnabled());
        w->act(QStringLiteral("arr.rotMore"))->trigger();
        auto *rotation = w->formatPane()->findChild<NumberField *>(QStringLiteral("sizeRotation"));
        QVERIFY(rotation);
        rotation->box()->setFocus();
        rotation->box()->selectAll();
        QTRY_VERIFY(rotation->box()->hasFocus());
        QTest::keyClicks(rotation->box(), QStringLiteral("45"));
        QCOMPARE(entriesWhenDeleted(w), 0);
    }

    // The text of a shape being edited on the slide, and a comment being drafted.
    void aWindowGoingAsksNoChangeOfAnEditedShapeOrACommentDraft()
    {
        Settings &st = Settings::get();   // (a comment asks who is writing otherwise)
        st.setValue(QStringLiteral("user/name"), QStringLiteral("Pat Doe"));
        st.setValue(QStringLiteral("user/initials"), QStringLiteral("PD"));
        auto *w = new DeckWindow;
        w->resize(1280, 800);
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        ShapeItem *s = addShape(*w, QStringLiteral("Boxed"), QRectF(100, 100, 300, 120));
        w->activateWindow();
        w->canvas()->setFocus();
        w->editor()->beginTextEdit(s->id);
        w->editor()->typeText(QStringLiteral("abc"));
        w->act(QStringLiteral("comment.new"))->trigger();
        auto *draft = w->commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"));
        QVERIFY(draft && draft->isVisible());
        draft->setFocus();
        QTest::keyClicks(draft, QStringLiteral("a draft"));
        QCOMPARE(entriesWhenDeleted(w), 0);
    }

    // The notes typed in Presenter View while the window goes (the show is the window's and closes with it): nothing is saved into the window.
    void aWindowGoingAsksNoChangeOfPresenterViewNotes()
    {
        auto *w = new DeckWindow;
        w->resize(1280, 800);
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        w->newSlide();
        w->editor()->setCurrentPage(0);
        w->act(QStringLiteral("show.presenterView"))->trigger();
        SlideShow *show = w->slideShow();
        QVERIFY(show && show->presenterView());
        auto *edit = show->presenterView()->findChild<QPlainTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit && !edit->isReadOnly());
        edit->setFocus();
        QTest::keyClicks(edit, QStringLiteral("spoken"));
        QCOMPARE(entriesWhenDeleted(w), 0);
    }

    // More Rotation Options (Arrange menu): opens the Format pane at its Size page; a rotation typed there is one step of Undo and Redo, and
    // the saved file, opened again through the window, has it.
    void moreRotationOptionsOpenTheSizePageAndTheTurnSurvivesASave()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("turn.pptx"));
        {
            DeckWindow w;
            FormatPaneGuard guard(&w);
            w.resize(1200, 800);
            w.show();
            ShapeItem *s = addShape(w, QStringLiteral("Spinner"), QRectF(100, 100, 200, 100));
            const QString id = s->id;
            QTRY_VERIFY(w.act(QStringLiteral("arr.rotMore"))->isEnabled());
            w.act(QStringLiteral("arr.rotMore"))->trigger();
            QVERIFY(!w.formatDock()->isHidden());
            QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::SizeProperties);
            auto *rotation = w.formatPane()->findChild<NumberField *>(QStringLiteral("sizeRotation"));
            QVERIFY(rotation);
            rotation->box()->setValue(37);
            QCOMPARE(w.editor()->doc()->item(id)->rotation, 37.0);
            w.editor()->undo();
            QCOMPARE(w.editor()->doc()->item(id)->rotation, 0.0);
            w.editor()->redo();
            QCOMPARE(w.editor()->doc()->item(id)->rotation, 37.0);
            QVERIFY(w.saveTo(path));
        }
        DeckWindow b;
        QVERIFY(b.openFile(path));
        ShapeItem *back = shapeNamed(b, QStringLiteral("Spinner"));
        QVERIFY(back);
        QVERIFY(std::abs(back->rotation - 37.0) < 0.05);
        // With nothing selected the command is off.
        b.editor()->clearSelection();
        QVERIFY(!b.act(QStringLiteral("arr.rotMore"))->isEnabled());
    }

    // Size and Position, from the center: the boxes show the object's center and a typed number puts the center there (one step of Undo); the
    // choice changes only what the boxes count from, so the saved file has the same frame either way.
    void positionCanBeCountedFromTheCenterOfTheObject()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("center.pptx"));
        {
            DeckWindow w;
            FormatPaneGuard guard(&w);
            w.resize(1200, 800);
            w.show();
            const QString id = addShape(w, QStringLiteral("Mover"), QRectF(100, 100, 200, 100))->id;
            w.act(QStringLiteral("shape.sizePos"))->trigger();
            auto *x = w.formatPane()->findChild<NumberField *>(QStringLiteral("positionX"));
            auto *y = w.formatPane()->findChild<NumberField *>(QStringLiteral("positionY"));
            auto *from = w.formatPane()->findChild<QComboBox *>(QStringLiteral("positionFrom"));
            QVERIFY(x && y && from);
            QCOMPARE(from->currentIndex(), 0);
            QCOMPARE(x->value(), 100.0);
            from->setCurrentIndex(1);
            QCOMPARE(x->value(), 200.0);
            QCOMPARE(y->value(), 150.0);
            x->box()->setValue(400);
            QCOMPARE(w.editor()->doc()->item(id)->rect, QRectF(300, 100, 200, 100));
            w.editor()->undo();
            QCOMPARE(w.editor()->doc()->item(id)->rect, QRectF(100, 100, 200, 100));
            w.editor()->redo();
            QCOMPARE(w.editor()->doc()->item(id)->rect, QRectF(300, 100, 200, 100));
            from->setCurrentIndex(0);
            QCOMPARE(x->value(), 300.0);
            QVERIFY(w.saveTo(path));
        }
        DeckWindow b;
        QVERIFY(b.openFile(path));
        const ShapeItem *back = shapeNamed(b, QStringLiteral("Mover"));
        QVERIFY(back);
        QVERIFY(std::abs(back->rect.x() - 300) < 0.05 && std::abs(back->rect.width() - 200) < 0.05 && std::abs(back->rect.y() - 100) < 0.05);
    }

    // New lists the built-in themes in their variants as places to start; choosing one makes the presentation in that theme and variant (in
    // this window when it is untouched, in a new one otherwise), and the saved file, opened again through the window, looks the same.
    void theNewPageStartsFromABuiltInThemeAndVariant()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("themed.pptx"));
        const int theme = 2, variant = 1;
        LayoutCache cache;
        const QImage expected = deck::slideImage(*deck::themePresentation(theme, variant, false), &cache, 0, 640);
        {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            w.showFileScreen(QStringLiteral("new"));
            auto *screen = w.findChild<FileScreen *>();
            QVERIFY(screen);
            for (int t = 0; t < deck::builtinThemeCount(); ++t)
                for (int v = 0; v < int(deck::builtinThemeVariants(t).size()); ++v)
                    QVERIFY2(screen->findChild<QAbstractButton *>(QStringLiteral("themeTile%1_%2").arg(t).arg(v)), qPrintable(QStringLiteral("%1 %2").arg(t).arg(v)));
            auto *tile = screen->findChild<QAbstractButton *>(QStringLiteral("themeTile%1_%2").arg(theme).arg(variant));
            QVERIFY(tile);
            QVERIFY(!w.editor()->isModified());
            // The tiles read apart to a screen reader: the theme's name, then "name, variant 2" and so on, and no two alike.
            QCOMPARE(tile->accessibleName(), QStringLiteral("%1, variant 2").arg(deck::builtinThemeName(theme)));
            QSet<QString> names;
            const auto tiles = screen->findChildren<QAbstractButton *>(QRegularExpression(QStringLiteral("^themeTile")));
            for (const QAbstractButton *each : tiles) names.insert(each->accessibleName());
            QCOMPARE(int(names.size()), int(tiles.size()));
            QVERIFY(!tiles.isEmpty());
            tile->click();
            QVERIFY(!screen->isVisible());
            // The new presentation starts with empty places, as a blank one does (the name is on the tile), unsaved and unchanged.
            QVERIFY(!w.editor()->isModified() && w.editor()->filePath().isEmpty());
            for (const auto &it : w.editor()->doc()->pages[0]->items)
                if (it->type() == ItemType::Text) QVERIFY(w.editor()->doc()->storyDoc(static_cast<TextItem *>(it.get())->storyId)->toPlainText().isEmpty());
            LayoutCache mine;
            QCOMPARE(deck::slideImage(*w.editor()->doc(), &mine, 0, 640), expected);
            // The default presentation looks different from the chosen theme's, so the comparison above means something.
            QVERIFY(deck::slideImage(*deck::newDefaultPresentation(), &mine, 0, 640) != expected);
            QVERIFY(w.saveTo(path));
        }
        DeckWindow b;
        QVERIFY(b.openFile(path));
        LayoutCache again;
        QCOMPARE(deck::slideImage(*b.editor()->doc(), &again, 0, 640), expected);
    }

    // The New page draws its previews once for the run of the program, and a few at a time after the page is up: the second time it is shown
    // draws none.
    void theNewPageDrawsItsPreviewsOnceAndAfterItIsUp()
    {
        forgetThemePictures();
        int total = 0;
        for (int t = 0; t < deck::builtinThemeCount(); ++t) total += int(deck::builtinThemeVariants(t).size());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        auto ready = [&w, total] {
            auto *screen = w.findChild<FileScreen *>();
            if (!screen) return false;
            int n = 0;
            for (QAbstractButton *tile : screen->findChildren<QAbstractButton *>(QRegularExpression(QStringLiteral("^themeTile")))) n += tile->property("pictureReady").toBool();
            return n == total;
        };
        const int before = themePictureRenders();
        w.showFileScreen(QStringLiteral("new"));
        // The page is up with none or few drawn (they come from the event loop), then all of them are.
        QVERIFY(themePictureRenders() - before < total);
        QTRY_VERIFY(ready());
        QCOMPARE(themePictureRenders() - before, total);
        w.showFileScreen(QStringLiteral("new"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);   // (the page that was replaced goes, so that only the new one is counted)
        QTRY_VERIFY(ready());
        QCOMPARE(themePictureRenders() - before, total);
    }

    // Header and Footer has a Notes and Handouts tab: what is set there reaches the notes pages and the handouts, with the slides' choice made in
    // the same window, as one step of Undo; and the saved file, opened again through the window, has both.
    void headerAndFooterHasANotesAndHandoutsTab()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("hf.pptx"));
        {
            DeckWindow w;
            w.show();
            QVERIFY(!deck::hasPrintHeaderFooter(*w.editor()->doc(), deck::PrintedPages::Notes));
            withDialog([&w] { w.act(QStringLiteral("insert.headerFooter"))->trigger(); }, QStringLiteral("headerFooterDialog"), [](QDialog *dlg) {
                auto *tabs = dlg->findChild<QTabWidget *>(QStringLiteral("headerFooterTabs"));
                QVERIFY(tabs);
                QCOMPARE(tabs->count(), 2);
                QCOMPARE(tabs->tabText(1), QStringLiteral("Notes and Handouts"));
                dlg->findChild<QCheckBox *>(QStringLiteral("footer"))->setChecked(true);
                dlg->findChild<QLineEdit *>(QStringLiteral("footerText"))->setText(QStringLiteral("On the slides"));
                tabs->setCurrentIndex(1);
                dlg->findChild<QCheckBox *>(QStringLiteral("hfHeader"))->setChecked(true);
                dlg->findChild<QLineEdit *>(QStringLiteral("hfHeaderText"))->setText(QStringLiteral("Quarterly"));
                dlg->findChild<QCheckBox *>(QStringLiteral("hfFooter"))->setChecked(true);
                dlg->findChild<QLineEdit *>(QStringLiteral("hfFooterText"))->setText(QStringLiteral("For the board"));
                dlg->findChild<QPushButton *>(QStringLiteral("applyToAll"))->click();
            }, false);
            const Document *d = w.editor()->doc();
            for (deck::PrintedPages pages : {deck::PrintedPages::Notes, deck::PrintedPages::Handouts}) {
                const deck::PrintHeaderFooter hf = deck::printHeaderFooter(*d, pages);
                QVERIFY(hf.header && hf.footer);
                QCOMPARE(hf.headerText, QStringLiteral("Quarterly"));
                QCOMPARE(hf.footerText, QStringLiteral("For the board"));
            }
            QVERIFY(deck::footerOptions(*d).footer);
            w.editor()->undo();
            QVERIFY(!deck::hasPrintHeaderFooter(*w.editor()->doc(), deck::PrintedPages::Notes));
            QVERIFY(!deck::footerOptions(*w.editor()->doc()).footer);
            w.editor()->redo();
            QVERIFY(deck::hasPrintHeaderFooter(*w.editor()->doc(), deck::PrintedPages::Notes) && deck::footerOptions(*w.editor()->doc()).footer);
            QVERIFY(w.saveTo(path));
        }
        DeckWindow b;
        QVERIFY(b.openFile(path));
        for (deck::PrintedPages pages : {deck::PrintedPages::Notes, deck::PrintedPages::Handouts}) {
            const deck::PrintHeaderFooter hf = deck::printHeaderFooter(*b.editor()->doc(), pages);
            QVERIFY(hf.header && hf.footer);
            QCOMPARE(hf.headerText, QStringLiteral("Quarterly"));
            QCOMPARE(hf.footerText, QStringLiteral("For the board"));
        }
        // (the slides' footer is in the slides themselves)
        bool slideFooter = false;
        for (const auto &page : b.editor()->doc()->pages)
            for (const ItemPtr &it : page->items)
                if (deck::placeholderType(*it) == QLatin1String("ftr") && it->type() == ItemType::Text &&
                    b.editor()->doc()->storyDoc(static_cast<const TextItem &>(*it).storyId)->toPlainText() == QLatin1String("On the slides"))
                    slideFooter = true;
        QVERIFY(slideFooter);
    }

    // A file whose notes pages and handouts have different footers: the Notes and Handouts tab shows both sets, and changing one leaves the other
    // as it was, through a save and an open of the window.
    void headerAndFooterKeepsDifferentNotesAndHandoutsApart()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString first = dir.filePath(QStringLiteral("apart.pptx")), second = dir.filePath(QStringLiteral("apart2.pptx"));
        auto footerOf = [](DeckWindow &w, deck::PrintedPages pages) { return deck::printHeaderFooter(*w.editor()->doc(), pages); };
        {
            DeckWindow w;
            w.show();
            deck::PrintHeaderFooter notes, handouts;
            notes.footer = true;
            notes.footerText = QStringLiteral("Notes footer");
            handouts.footer = true;
            handouts.footerText = QStringLiteral("Handout footer");
            deck::setPrintHeaderFooter(*w.editor()->doc(), deck::PrintedPages::Notes, notes);
            deck::setPrintHeaderFooter(*w.editor()->doc(), deck::PrintedPages::Handouts, handouts);
            QVERIFY(w.saveTo(first));
        }
        {
            DeckWindow b;
            QVERIFY(b.openFile(first));
            b.show();
            withDialog([&b] { b.act(QStringLiteral("insert.headerFooter"))->trigger(); }, QStringLiteral("headerFooterDialog"), [](QDialog *dlg) {
                auto *tabs = dlg->findChild<QTabWidget *>(QStringLiteral("headerFooterTabs"));
                QVERIFY(tabs);
                tabs->setCurrentIndex(1);
                auto *both = dlg->findChild<QTabWidget *>(QStringLiteral("headerFooterNotesAndHandouts"));
                QVERIFY(both);
                QCOMPARE(both->count(), 2);
                QCOMPARE(both->tabText(1), QStringLiteral("Handouts"));
                // The notes page's footer is changed; the handouts' is not touched.
                auto *text = both->widget(0)->findChild<QLineEdit *>(QStringLiteral("hfFooterText"));
                QCOMPARE(text->text(), QStringLiteral("Notes footer"));
                text->setText(QStringLiteral("Changed notes"));
                QCOMPARE(both->widget(1)->findChild<QLineEdit *>(QStringLiteral("hfFooterText"))->text(), QStringLiteral("Handout footer"));
                dlg->findChild<QPushButton *>(QStringLiteral("applyToAll"))->click();
            }, false);
            QCOMPARE(footerOf(b, deck::PrintedPages::Notes).footerText, QStringLiteral("Changed notes"));
            QCOMPARE(footerOf(b, deck::PrintedPages::Handouts).footerText, QStringLiteral("Handout footer"));
            QVERIFY(b.saveTo(second));
        }
        DeckWindow c;
        QVERIFY(c.openFile(second));
        QCOMPARE(footerOf(c, deck::PrintedPages::Notes).footerText, QStringLiteral("Changed notes"));
        QCOMPARE(footerOf(c, deck::PrintedPages::Handouts).footerText, QStringLiteral("Handout footer"));
        // Opened and accepted without a change to either set, neither is written over.
        withDialog([&c] { c.act(QStringLiteral("insert.headerFooter"))->trigger(); }, QStringLiteral("headerFooterDialog"), [](QDialog *dlg) {
            dlg->findChild<QPushButton *>(QStringLiteral("applyToAll"))->click();
        }, false);
        QCOMPARE(footerOf(c, deck::PrintedPages::Notes).footerText, QStringLiteral("Changed notes"));
        QCOMPARE(footerOf(c, deck::PrintedPages::Handouts).footerText, QStringLiteral("Handout footer"));
    }

    // Save as Presentation of Pictures: a new presentation file with one picture for each shown slide, the picture filling the slide with the
    // slide's title as its alt text, and the notes kept; opened again through the window it has the same slides. The pictures are held to a
    // total, and a presentation over it is refused whole.
    void aPresentationOfPicturesHasOnePictureForEachShownSlide()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("pictures.pptx"));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        Document *d = w.editor()->doc();
        w.newSlide();
        w.newSlide();
        QCOMPARE(int(d->pages.size()), 3);
        d->pages[2]->hidden = true;
        QTextCursor c(d->storyDoc(static_cast<TextItem *>(d->pages[0]->items[0].get())->storyId));
        c.insertText(QStringLiteral("Opening words"));
        d->pages[1]->notesStoryId = d->createStory();
        setStoryText(d->storyDoc(d->pages[1]->notesStoryId), QStringLiteral("Say this second"));
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        withDialog([&w] { w.act(QStringLiteral("file.exportPicturePresentation"))->trigger(); }, QStringLiteral("picturePresentationDialog"), [](QDialog *dlg) {
            dlg->findChild<QSpinBox *>(QStringLiteral("picturePresentationWidth"))->setValue(640);
        });
        QVERIFY(QFileInfo::exists(path));
        DeckWindow b;
        QVERIFY(b.openFile(path));
        const Document *back = b.editor()->doc();
        QCOMPARE(int(back->pages.size()), 2);
        for (int i = 0; i < 2; ++i) {
            QVector<PictureItem *> pictures;
            for (const auto &it : back->pages[i]->items)
                if (it->type() == ItemType::Picture) pictures << static_cast<PictureItem *>(it.get());
            QCOMPARE(int(back->pages[i]->items.size()), 1);
            QCOMPARE(int(pictures.size()), 1);
            QVERIFY(std::abs(pictures[0]->rect.width() - back->pageSize().width()) < 0.5 && std::abs(pictures[0]->rect.height() - back->pageSize().height()) < 0.5);
            QCOMPARE(back->imageSize(pictures[0]->imageId).width(), 640);
            if (i == 0) QVERIFY(pictures[0]->altText.contains(QStringLiteral("Opening words")));
        }
        QCOMPARE(deck::notesText(*back, *back->pages[1]), QStringLiteral("Say this second"));
        QCOMPARE(deck::notesText(*back, *back->pages[0]), QString());
        // Hidden slides come when asked for.
        LayoutCache cache;
        deck::PicturePresentationOptions all;
        all.width = 320;
        all.hidden = true;
        QCOMPARE(deck::picturePresentation(*d, &cache, all).slides, 3);
        // The total: a presentation whose pictures come to more than the limit is refused whole, after the first picture that takes it over.
        all.mostBytes = 1;
        const deck::PicturePresentationResult refused = deck::picturePresentation(*d, &cache, all);
        QVERIFY(!refused.presentation);
        QVERIFY(!refused.error.isEmpty());
    }

    // A presentation of pictures is never saved over the file of the presentation that is open (it would replace it, and the window would show the old one):
    // it is refused with a message and the file is as it was.
    void aPresentationOfPicturesIsRefusedOverTheOpenFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString open = dir.filePath(QStringLiteral("open.pptx"));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QStringList told;
        w.setNotifier([&told](const QString &, const QString &text) { told << text; });
        QVERIFY(w.saveTo(open));
        QFile f(open);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        w.setSavePathChooser([&](const QString &, const QString &) { return open; });
        deck::PicturePresentationOptions options;
        options.width = 160;
        QVERIFY(!w.exportPicturePresentation(options));
        QCOMPARE(int(told.size()), 1);
        QVERIFY(told[0].contains(QStringLiteral("open")));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll(), bytes);
    }

    // A deck of more than ten slides shows a progress box with Cancel, whatever number of them are shown; stopped from it, nothing is saved; a deck of
    // up to ten has no box.
    void aPresentationOfPicturesOfAnyLongDeckShowsAProgressBoxAndLeavesNoFileWhenStopped()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString other = dir.filePath(QStringLiteral("pictures.pptx"));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        for (int i = 0; i < 11; ++i) w.newSlide();
        Document *d = w.editor()->doc();
        // Twelve slides, ten of them hidden and left out: two are drawn, and the box is there all the same.
        for (int i = 2; i < d->pages.size(); ++i) d->pages[i]->hidden = true;
        w.setSavePathChooser([&](const QString &, const QString &) { return other; });
        deck::PicturePresentationOptions options;
        options.width = 160;
        int seen = 0;
        bool boxShown = false;
        options.progress = [&](int, int) {
            ++seen;
            boxShown = boxShown || w.findChild<QProgressDialog *>() != nullptr;
            return false;
        };
        QVERIFY(!w.exportPicturePresentation(options));
        QCOMPARE(seen, 1);
        QVERIFY(boxShown);
        QVERIFY(!QFileInfo::exists(other));
        while (d->pages.size() > 2) d->pages.removeLast();
        boxShown = false;
        QVERIFY(!w.exportPicturePresentation(options));
        QVERIFY(!boxShown);
        QVERIFY(!QFileInfo::exists(other));
    }

    // The total is looked at before a slide is drawn: with room for one picture and a half, the second slide is not drawn at all.
    void thePicturePresentationTotalIsCheckedBeforeTheNextSlideIsDrawn()
    {
        DeckWindow w;
        w.newSlide();
        LayoutCache cache;
        deck::PicturePresentationOptions big;
        big.width = 160;
        const deck::PicturePresentationResult whole = deck::picturePresentation(*w.editor()->doc(), &cache, big);
        QVERIFY(whole.presentation);
        QCOMPARE(whole.drawn, 2);
        deck::PicturePresentationOptions tight = big;
        tight.mostBytes = whole.bytes / 2 * 3 / 2 + 1;
        const deck::PicturePresentationResult stopped = deck::picturePresentation(*w.editor()->doc(), &cache, tight);
        QVERIFY(!stopped.presentation);
        QVERIFY(!stopped.error.isEmpty());
        QCOMPARE(stopped.drawn, 1);
    }

    // Check Accessibility also lists a slide whose title another slide has, a table with merged cells, and a link whose words do not say where
    // it goes (its own address, or "click here"); a link with words that name the page is not listed. The same list comes back from the saved
    // file opened through the window.
    void checkAccessibilityListsDuplicateTitlesMergedCellsAndUnclearLinks()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("access.pptx"));
        auto listed = [](DeckWindow &w) {
            w.act(QStringLiteral("review.accessibility"))->trigger();
            PageAccessibilityPane *pane = w.accessibilityPane();
            QStringList lines;
            for (int i = 0; i < pane->list()->count(); ++i) lines << pane->list()->item(i)->text();
            return lines;
        };
        auto has = [](const QStringList &lines, const QString &part) {
            return std::any_of(lines.begin(), lines.end(), [&](const QString &l) { return l.contains(part); });
        };
        {
            DeckWindow w;
            w.resize(1400, 900);
            w.show();
            w.newSlide();
            Editor *ed = w.editor();
            Document *d = ed->doc();
            for (int i = 0; i < 2; ++i) {
                QTextCursor c(d->storyDoc(static_cast<TextItem *>(d->pages[i]->items[0].get())->storyId));
                c.insertText(QStringLiteral("Quarterly   results"));
            }
            ed->setCurrentPage(0);
            auto table = ed->newTable(QRectF(72, 200, 200, 60), 2, 2);
            auto *t = static_cast<TableItem *>(table.get());
            ed->addItem(table, false);
            ed->change(QStringLiteral("merge"), [&] {
                t->cell(0, 0).colSpan = 2;
                t->cell(0, 1).covered = true;
            });
            auto link = [&](const QString &words, const QString &href, double top) {
                auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, top, 300, 40), QString()));
                QTextCharFormat f;
                f.setAnchor(true);
                f.setAnchorHref(href);
                QTextCursor c(d->storyDoc(box->storyId));
                c.insertText(words, f);
                ed->addItem(box, false);
            };
            link(QStringLiteral("click here"), QStringLiteral("https://example.org/report"), 300);
            link(QStringLiteral("https://example.org/long/address"), QStringLiteral("https://example.org/long/address"), 350);
            link(QStringLiteral("The annual report"), QStringLiteral("https://example.org/report"), 400);
            ed->clearSelection();
            const QStringList lines = listed(w);
            QVERIFY(has(lines, QStringLiteral("Slide 2 has the same title as slide 1")));
            QVERIFY(!has(lines, QStringLiteral("Slide 1 has the same title")));
            QVERIFY(has(lines, QStringLiteral("has merged cells")));
            QVERIFY(has(lines, QStringLiteral("The link \"click here\"")));
            QVERIFY(has(lines, QStringLiteral("The link \"https://example.org/long/address\"")));
            QVERIFY(!has(lines, QStringLiteral("The annual report")));
            QVERIFY(w.saveTo(path));
        }
        DeckWindow b;
        b.resize(1400, 900);
        QVERIFY(b.openFile(path));
        b.show();
        const QStringList again = listed(b);
        QVERIFY(has(again, QStringLiteral("Slide 2 has the same title as slide 1")));
        QVERIFY(has(again, QStringLiteral("has merged cells")));
        QVERIFY(has(again, QStringLiteral("The link \"click here\"")));
        QVERIFY(!has(again, QStringLiteral("The annual report")));
        // Words that say where the link goes.
        QVERIFY(jp::linkTextSaysWhere(QStringLiteral("Annual report"), QStringLiteral("https://example.org/r")));
        QVERIFY(!jp::linkTextSaysWhere(QStringLiteral("Read   more"), QStringLiteral("https://example.org/r")));
        QVERIFY(!jp::linkTextSaysWhere(QStringLiteral("www.example.org"), QStringLiteral("https://www.example.org")));
    }

    // Every link that says nothing of where it goes is listed (an empty one says it has no words), the first 50 one by one and the rest in one
    // line that counts them; two pieces of one link are one link.
    void everyUnclearLinkIsListedUpToFiftyAndTheRestAreCounted()
    {
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, 200, 600, 200), QString()));
        ed->addItem(box, false);
        QTextCursor c(d->storyDoc(box->storyId));
        auto link = [&](const QString &words, const QString &href) {
            QTextCharFormat f;
            f.setAnchor(true);
            f.setAnchorHref(href);
            c.insertText(words, f);
            c.insertText(QStringLiteral(" "), QTextCharFormat());
        };
        link(QStringLiteral("   "), QStringLiteral("https://example.org/empty"));
        for (int i = 0; i < 54; ++i) link(QStringLiteral("here"), QStringLiteral("https://example.org/%1").arg(i));
        // One link in two pieces of the same address: "Annual" and "report" together say where it goes.
        QTextCharFormat piece;
        piece.setAnchor(true);
        piece.setAnchorHref(QStringLiteral("https://example.org/annual"));
        c.insertText(QStringLiteral("Annual "), piece);
        piece.setFontWeight(QFont::Bold);
        c.insertText(QStringLiteral("report"), piece);
        const QVector<jp::PageAccessIssue> found = jp::checkPageAccessibility(*d, true);
        int listed = 0, empty = 0, counted = 0;
        for (const auto &issue : found) {
            if (issue.kind != jp::PageAccessIssue::UnclearLink) continue;
            if (issue.message.contains(QStringLiteral("more link"))) {
                QVERIFY(issue.message.startsWith(QStringLiteral("5 more link")));
                ++counted;
            } else {
                ++listed;
                empty += issue.message.contains(QStringLiteral("has no words"));
                QVERIFY(!issue.message.contains(QStringLiteral("Annual")));
            }
        }
        QCOMPARE(listed, jp::kMostUnclearLinks);
        QCOMPARE(empty, 1);
        QCOMPARE(counted, 1);
    }

    // Picture Format > Adjust has Corrections, Color, and Transparency: each choice is one step of Undo and Redo, and the saved file, opened again
    // through the window, has the brightness, contrast, gray, saturation, transparency, and the color made clear.
    void pictureAdjustMenusSetCorrectionsColorAndTransparency()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("adjust.pptx"));
        {
            DeckWindow w;
            FormatPaneGuard guard(&w);
            w.resize(1200, 800);
            w.show();
            const QString id = addPicture(w, QStringLiteral("Flag"))->id;
            auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
            for (const char *name : {"pic.brightness.20", "pic.contrast.-20", "pic.saturation.50", "pic.recolor.gray", "pic.transparency.30", "pic.transparent", "pic.transparentNone"})
                QVERIFY2(w.act(QString::fromLatin1(name))->isEnabled(), name);
            w.act(QStringLiteral("pic.brightness.20"))->trigger();
            QCOMPARE(pic()->brightness, 20.0);
            w.editor()->undo();
            QCOMPARE(pic()->brightness, 0.0);
            w.editor()->redo();
            QCOMPARE(pic()->brightness, 20.0);
            w.act(QStringLiteral("pic.contrast.-20"))->trigger();
            w.act(QStringLiteral("pic.saturation.50"))->trigger();
            w.act(QStringLiteral("pic.recolor.gray"))->trigger();
            w.act(QStringLiteral("pic.transparency.30"))->trigger();
            QCOMPARE(pic()->contrast, -20.0);
            QCOMPARE(pic()->saturationMove(), 50.0);
            QCOMPARE(int(pic()->recolor), int(PictureItem::Grayscale));
            QCOMPARE(pic()->transparency, 0.3);
            w.setPictureTransparentColor(QColor(200, 30, 30));
            QVERIFY(pic()->hasTransparentColor);
            QCOMPARE(pic()->transparentColor, QColor(200, 30, 30));
            w.editor()->undo();
            QVERIFY(!pic()->hasTransparentColor);
            w.editor()->redo();
            // The Format pane shows the saturation.
            w.act(QStringLiteral("shape.format"))->trigger();
            auto *box = w.formatPane()->findChild<NumberField *>(QStringLiteral("imageSaturation"));
            QVERIFY(box);
            QCOMPARE(box->value(), 50.0);
            box->box()->setValue(-100);
            QCOMPARE(pic()->saturationMove(), -100.0);
            w.editor()->undo();
            QCOMPARE(pic()->saturationMove(), 50.0);
            QVERIFY(w.saveTo(path));
        }
        DeckWindow b;
        QVERIFY(b.openFile(path));
        const PictureItem *back = pictureNamed(b, QStringLiteral("Flag"));
        QVERIFY(back);
        QVERIFY(std::abs(back->brightness - 20) < 0.5 && std::abs(back->contrast + 20) < 0.5);
        QVERIFY(std::abs(back->saturationMove() - 50) < 0.5);
        QCOMPARE(int(back->recolor), int(PictureItem::Grayscale));
        QVERIFY(std::abs(back->transparency - 0.3) < 0.01);
        QVERIFY(back->hasTransparentColor);
        QCOMPARE(back->transparentColor, QColor(200, 30, 30));
        // Setting the saturation back to 0 takes the change out of the picture.
        PictureItem plain;
        plain.setSaturationMove(30);
        QCOMPARE(int(plain.blipEffects.size()), 1);
        plain.setSaturationMove(0);
        QVERIFY(plain.blipEffects.isEmpty());
    }

    // Sharpen and soften, color temperature, and the artistic effects: each choice is one step of Undo, the picture's own bytes never change, Reset Picture takes the effects
    // off, a presentation file keeps the settings (and the original) while the picture in it is the finished one, and another kind of file says what it keeps.
    void pictureFiltersAreOneStepKeptInTheFileAndResetTakesThemOff()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("filters.pptx")), odp = dir.filePath(QStringLiteral("filters.odp"));
        QByteArray original, finished;
        QSize pixels;
        {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            const QString id = addPicture(w, QStringLiteral("Flag"))->id;
            auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
            original = w.editor()->doc()->images.value(pic()->imageId).bytes;
            pixels = w.editor()->doc()->imageSize(pic()->imageId);
            for (const char *name : {"pic.sharpen.-50", "pic.sharpen.50", "pic.temperature.-100", "pic.temperature.100"})
                QVERIFY2(w.act(QString::fromLatin1(name))->isEnabled(), name);
            const int steps = w.editor()->undoStack()->count();
            w.act(QStringLiteral("pic.sharpen.50"))->trigger();
            QCOMPARE(pic()->sharpenMove(), 50.0);
            QCOMPARE(w.editor()->undoStack()->count(), steps + 1);
            w.editor()->undo();
            QCOMPARE(pic()->sharpenMove(), 0.0);
            QVERIFY(pic()->blipEffects.isEmpty());
            w.editor()->redo();
            w.act(QStringLiteral("pic.temperature.50"))->trigger();
            jp::applyArtisticEffect(w.editor(), &w, int(jp::ArtisticStyle::GlowEdges));
            QCOMPARE(pic()->artisticStyle(), int(jp::ArtisticStyle::GlowEdges));
            w.editor()->undo();
            QCOMPARE(pic()->artisticStyle(), -1);
            w.editor()->redo();
            // (the gallery's thumbnails are made from the picture itself, one for each effect and one for none)
            const QVector<GalleryItem> items = jp::artisticEffectItems(w.editor()->doc()->image(pic()->imageId));
            QCOMPARE(int(items.size()), int(jp::ArtisticStyle::Last) + 2);
            QCOMPARE(items[0].id, QStringLiteral("-1"));
            // (for the docs, when JO_SHOT_DIR names a folder: the gallery's thumbnails side by side, the picture and each effect)
            if (const QString shots = qEnvironmentVariable("JO_SHOT_DIR"); !shots.isEmpty()) {
                QImage sheet(items.size() * 100, 80, QImage::Format_ARGB32);
                sheet.fill(Qt::white);
                QPainter pp(&sheet);
                QImage photo(96, 72, QImage::Format_ARGB32);
                photo.fill(QColor(120, 180, 230));
                QPainter pq(&photo);
                pq.setPen(Qt::NoPen);
                pq.setBrush(QColor(90, 160, 80));
                pq.drawEllipse(QRect(-20, 40, 140, 70));
                pq.setBrush(QColor(250, 210, 60));
                pq.drawEllipse(QRect(64, 8, 20, 20));
                pq.setBrush(QColor(200, 60, 50));
                pq.drawRect(30, 30, 28, 22);
                pq.end();
                const QVector<GalleryItem> shown = jp::artisticEffectItems(photo);
                for (int i = 0; i < shown.size(); ++i) pp.drawPixmap(i * 100 + 2, 4, shown[i].icon.pixmap(QSize(48, 36)).scaled(96, 72));
                pp.end();
                QDir().mkpath(shots);
                sheet.save(QDir(shots).filePath(QStringLiteral("picfx-gallery.png")));
            }
            QCOMPARE(w.editor()->doc()->images.value(pic()->imageId).bytes, original);   // the original is never changed
            finished = pixelBytes(jp::applyBlipEffects(w.editor()->doc()->image(pic()->imageId), jp::blipFilters(pic()->blipEffects)));
            QVERIFY(finished != pixelBytes(w.editor()->doc()->image(pic()->imageId)));
            QVERIFY(w.saveTo(path));
            QString said;
            jp::setLossyStandIn([&](const jp::LossyAsk &a, bool *) {
                said = a.lost.join(QLatin1Char('|'));
                return jp::LossyChoice::SaveAnyway;
            });
            const bool saved = w.saveTo(odp, true);
            jp::setLossyStandIn(nullptr);
            QVERIFY(saved);
            QVERIFY2(said.contains(QStringLiteral("artistic effect")), qPrintable(said));
        }
        {
            DeckWindow b;
            QVERIFY(b.openFile(path));
            PictureItem *back = pictureNamed(b, QStringLiteral("Flag"));
            QVERIFY(back);
            QCOMPARE(back->sharpenMove(), 50.0);
            QCOMPARE(back->temperatureMove(), 50.0);
            QCOMPARE(back->artisticStyle(), int(jp::ArtisticStyle::GlowEdges));
            // The original comes back (not the finished picture in the blip), and gives the same pixels, with no tolerance.
            QCOMPARE(b.editor()->doc()->imageSize(back->imageId), pixels);
            QCOMPARE(pixelBytes(jp::applyBlipEffects(b.editor()->doc()->image(back->imageId), jp::blipFilters(back->blipEffects))), finished);
            // Reset Picture takes the effects off, as one step that Undo takes back.
            b.editor()->select(back->id);
            b.act(QStringLiteral("pic.reset"))->trigger();
            QVERIFY(pictureNamed(b, QStringLiteral("Flag"))->blipEffects.isEmpty());
            b.editor()->undo();
            QCOMPARE(pictureNamed(b, QStringLiteral("Flag"))->sharpenMove(), 50.0);
        }
        // The other kind of file holds the finished picture.
        DeckWindow o;
        QVERIFY(o.openFile(odp));
        const PictureItem *flat = pictureNamed(o, QStringLiteral("Flag"));
        QVERIFY(flat);
        QVERIFY(jp::blipFilters(flat->blipEffects).isEmpty());
    }

    // Pictures that take over half a second to filter are filtered on a thread of its own (the work is counted, not timed): Cancel leaves them untouched, and without it the one step is made.
    void aBigPictureIsFilteredOnAThreadAndCancelLeavesItAlone()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *added = addPicture(w, QStringLiteral("Big"));
        const QString id = added->id;
        QImage big(2000, 2000, QImage::Format_ARGB32);
        big.fill(QColor(120, 80, 40));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        big.save(&buffer, "PNG");
        added->imageId = w.editor()->doc()->addImage(png, QStringLiteral("png"));
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        const int ran = jp::pictureFxThreadedRuns();
        // A small picture is not worked on a thread.
        {
            QImage small(40, 40, QImage::Format_ARGB32);
            small.fill(Qt::red);
            QByteArray p2;
            QBuffer b2(&p2);
            b2.open(QIODevice::WriteOnly);
            small.save(&b2, "PNG");
            PictureItem *other = addPicture(w, QStringLiteral("Small"));
            other->imageId = w.editor()->doc()->addImage(p2, QStringLiteral("png"));
            jp::applyArtisticEffect(w.editor(), &w, 0);
            QCOMPARE(jp::pictureFxThreadedRuns(), ran);
            w.editor()->undo();
        }
        // (the busy box is for work expected over half a second: one of these pictures with an artistic effect is not, three of them are)
        QStringList three{id};
        for (const char *name : {"Big two", "Big three"}) {
            PictureItem *more = addPicture(w, QString::fromLatin1(name));
            more->imageId = static_cast<PictureItem *>(w.editor()->doc()->item(id))->imageId;
            three << more->id;
        }
        w.editor()->select(three);
        // Cancel: the work stops, nothing is changed, and no step is made.
        const int steps = w.editor()->undoStack()->index();
        jp::pictureFxWorkerHook = [](const std::atomic<bool> &stop) {
            for (int i = 0; i < 20'000'000 && !stop; ++i) QThread::yieldCurrentThread();
        };
        const auto off = qScopeGuard([] { jp::pictureFxWorkerHook = nullptr; });
        QTimer::singleShot(0, &w, [&w] {
            if (auto *box = w.findChild<QProgressDialog *>(QStringLiteral("pictureEffectBusy"))) if (auto *cancel = box->findChild<QPushButton *>()) cancel->click();   // (the box's own Cancel button)
        });
        jp::applyArtisticEffect(w.editor(), &w, int(jp::ArtisticStyle::Blur));
        QCOMPARE(jp::pictureFxThreadedRuns(), ran + 1);
        QCOMPARE(pic()->artisticStyle(), -1);
        QCOMPARE(w.editor()->undoStack()->index(), steps);
        // Without it: worked out on the thread, then one step.
        jp::pictureFxWorkerHook = nullptr;
        jp::applyArtisticEffect(w.editor(), &w, int(jp::ArtisticStyle::Blur));
        QCOMPARE(jp::pictureFxThreadedRuns(), ran + 2);
        QCOMPARE(pic()->artisticStyle(), int(jp::ArtisticStyle::Blur));
        QCOMPARE(w.editor()->undoStack()->index(), steps + 1);
    }

    static QByteArray pixelBytes(const QImage &im)
    {
        const QImage a = im.convertToFormat(QImage::Format_ARGB32);
        QByteArray bytes;
        for (int y = 0; y < a.height(); ++y) bytes.append(reinterpret_cast<const char *>(a.constScanLine(y)), a.width() * 4);
        return bytes;
    }

    // A picture with a structure to it (bands and a square) of the size given, as the bytes of a PNG.
    static QByteArray structuredPng(int w, int h)
    {
        QImage im(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) im.setPixel(x, y, (x / 7 + y / 11) % 3 == 0 ? qRgb(200, 40, 40) : (x > w / 3 && x < 2 * w / 3 && y > h / 3 && y < 2 * h / 3 ? qRgb(20, 30, 200) : qRgb(230, 230, 190)));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        return png;
    }
    // The media parts of a saved file, as pictures.
    static QVector<QImage> mediaOf(const QString &path)
    {
        QVector<QImage> out;
        QFile f(path);
        QMap<QString, QByteArray> parts;
        if (!f.open(QIODevice::ReadOnly) || !readZip(f.readAll(), parts)) return out;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            if (it.key().startsWith(QLatin1String("ppt/media/"))) out << QImage::fromData(it.value());
        return out;
    }

    // A photograph of 4 million pixels with sharpen, temperature, and an artistic effect is filtered: on the screen, and in the picture the file carries for other programs (the work cap
    // used to turn all three off, so that the file said "filtered" over an unfiltered picture).
    void aBigPhotographKeepsAllItsFiltersOnScreenAndInTheFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("big.pptx"));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *added = addPicture(w, QStringLiteral("Photo"));
        const QString id = added->id;
        added->imageId = w.editor()->doc()->addImage(structuredPng(2000, 2000), QStringLiteral("png"));
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        pic()->setSharpenMove(50);
        pic()->setTemperatureMove(50);
        pic()->setArtistic(int(jp::ArtisticStyle::Blur), 0.5);
        QCOMPARE(jp::blipFilters(pic()->blipEffects).size(), qsizetype(3));
        const QImage original = w.editor()->doc()->image(pic()->imageId);
        const QImage expected = jp::applyBlipEffects(original, jp::blipFilters(pic()->blipEffects));
        QVERIFY(pixelBytes(expected) != pixelBytes(original.scaled(expected.size())));
        QCOMPARE(pixelBytes(Renderer::processedImage(*w.editor()->doc(), *pic(), QSizeF(), QSizeF())), pixelBytes(expected));
        QVERIFY(w.saveTo(path));
        bool found = false;
        for (const QImage &m : mediaOf(path)) found = found || pixelBytes(m) == pixelBytes(expected);
        QVERIFY(found);   // (the picture in the file is the finished one)
        // The settings are in an extension with a namespace of their own (not the removal's).
        QFile saved(path);
        QMap<QString, QByteArray> parts;
        QVERIFY(saved.open(QIODevice::ReadOnly) && readZip(saved.readAll(), parts));
        QByteArray slides;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            if (it.key().startsWith(QLatin1String("ppt/slides/slide")) && it.key().endsWith(QLatin1String(".xml"))) slides += it.value();
        QVERIFY(slides.contains("<jfx:filters"));
        QVERIFY(slides.contains("xmlns:jfx=\"http://jeffoffice.com/ns/picturefx/2026\""));
        QVERIFY(!slides.contains("jdr:filters"));
    }

    // A picture is drawn at once (a flat patch if a Blur or Mosaic is on it, else as it is) and filtered beside the window (the first draw never waits); drawn again at other sizes, or saved again, it works nothing out: counted, with a
    // removal in the way (it makes the picture again at each draw, which used to make the filters run again at every size).
    void aPictureIsDrawnAtOnceFilteredBesideTheWindowAndNeverWorkedOutTwice()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *added = addPicture(w, QStringLiteral("Beside"));
        const QString id = added->id;
        added->imageId = w.editor()->doc()->addImage(structuredPng(320, 240), QStringLiteral("png"));
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        pic()->removal = findBackground(w.editor()->doc()->image(pic()->imageId), {});
        QVERIFY(pic()->removal.isValid());
        pic()->setSharpenMove(60);
        pic()->setArtistic(int(jp::ArtisticStyle::Blur), 0.6);
        PictureItem plain(*pic());
        plain.blipEffects.clear();
        const Document &doc = *w.editor()->doc();
        const int made = jp::blipAdjustedMade();
        const QSizeF sizes[] = {QSizeF(160, 120), QSizeF(200, 150), QSizeF(120, 90), QSizeF(320, 240)};
        // The first draw: at once, and as the picture is without its filters.
        const QImage atOnce = Renderer::processedImage(doc, *pic(), QSizeF(), sizes[0], QColor(), false);
        // (a Blur is among the filters: until it is done a flat patch of the picture's color is drawn, never the picture it hides)
        for (const QPoint at : {QPoint(0, 0), QPoint(80, 60), QPoint(159, 119), QPoint(20, 100)}) QCOMPARE(atOnce.pixel(at), atOnce.pixel(1, 1));
        QVERIFY(pixelBytes(atOnce) != pixelBytes(Renderer::processedImage(doc, plain, QSizeF(), sizes[0])));
        // Finished beside the window: the same draw now gives the filtered picture.
        QImage drawn;
        QTRY_VERIFY(pixelBytes(drawn = Renderer::processedImage(doc, *pic(), QSizeF(), sizes[0], QColor(), false)) != pixelBytes(atOnce));
        QCOMPARE(jp::blipAdjustedMade(), made + 1);
        // Drawn at other sizes (the removal makes its picture again each time): nothing is worked out, and each is the filtered one at once.
        for (const QSizeF &size : sizes) {
            const QImage again = Renderer::processedImage(doc, *pic(), QSizeF(), size, QColor(), false);
            QVERIFY(pixelBytes(again) != pixelBytes(Renderer::processedImage(doc, plain, QSizeF(), size)));
        }
        QCOMPARE(jp::blipAdjustedMade(), made + 1);
        // Saved twice: the second save works nothing out.
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("one.pptx"))));
        const int afterFirst = jp::blipAdjustedMade();
        QVERIFY(afterFirst > made + 1);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("two.pptx"))));
        QCOMPARE(jp::blipAdjustedMade(), afterFirst);
    }

    // A picture of pure colors (nothing else on a screen has them) of the size given, as the bytes of a PNG: cells of two pixels, red, green, and blue in turn, so that any blur or mosaic
    // leaves none of them.
    static QByteArray pureColorPng(int w, int h)
    {
        QImage im(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) im.setPixel(x, y, (x / 2 + y / 2) % 3 == 0 ? qRgb(255, 0, 0) : (x / 2 + y / 2) % 3 == 1 ? qRgb(0, 255, 0) : qRgb(0, 0, 255));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        return png;
    }

    // How many pixels of `im` are one of the picture's pure colors.
    static int pureColorPixels(const QImage &im)
    {
        const QImage a = im.convertToFormat(QImage::Format_ARGB32);
        int n = 0;
        for (int y = 0; y < a.height(); ++y)
            for (int x = 0; x < a.width(); ++x) {
                const QRgb c = a.pixel(x, y);
                n += (qRed(c) > 240 && qGreen(c) < 15 && qBlue(c) < 15) || (qRed(c) < 15 && qGreen(c) > 240 && qBlue(c) < 15) || (qRed(c) < 15 && qGreen(c) < 15 && qBlue(c) > 240);
            }
        return n;
    }

    // The slide show draws its first frame once the filters are done: a Blur or Mosaic picture is never on the screen unfiltered (the show used to draw at once, with the filters still
    // running beside it, and so showed what the person had hidden).
    void theSlideShowsFirstFrameIsTheFilteredPicture()
    {
        for (const int style : {int(jp::ArtisticStyle::Blur), int(jp::ArtisticStyle::Mosaic)}) {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            PictureItem *added = addPicture(w, QStringLiteral("Hidden"));
            const QString id = added->id;
            added->imageId = w.editor()->doc()->addImage(pureColorPng(400, 300), QStringLiteral("png"));
            auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
            pic()->rect = QRectF(100, 100, 400, 300);
            pic()->imgRect = QRectF(0, 0, 400, 300);
            pic()->setArtistic(style, 0.8);
            PictureItem plain(*pic());
            plain.blipEffects.clear();
            const QImage unfiltered = jp::applyBlipEffects(w.editor()->doc()->image(pic()->imageId), {});
            QVERIFY(!unfiltered.isNull());
            HeldWorker held;
            SlideShow show(w.editor()->doc(), &w.editor()->cache(), 0);
            show.resize(960, 540);
            const SlideShow::View view{0, 0};
            const int asked = jp::blipFiltersAsked();
            const QImage first = show.renderView(view, -1, QSize(960, 540), 1);
            QCOMPARE(jp::blipFiltersAsked(), asked);   // (nothing was left for the thread beside the window: the show waited)
            // The same frame once the filters are known to the cache (worked out here, by a caller that waits): the first frame is already that one, never a patch or the unfiltered one.
            Renderer::processedImage(*w.editor()->doc(), *pic(), QSizeF(), QSizeF());
            const QImage again = show.renderView(view, -1, QSize(960, 540), 1);
            QCOMPARE(pixelBytes(first), pixelBytes(again));
            QCOMPARE(held.entered().load(), 0);
            pic()->blipEffects.clear();
            const QImage without = show.renderView(view, -1, QSize(960, 540), 1);
            QVERIFY(pureColorPixels(without) > 20000);
            QVERIFY2(pureColorPixels(first) < 2000, qPrintable(QStringLiteral("filter %1 left %2 pure pixels").arg(style).arg(pureColorPixels(first))));
        }
    }

    // A caller that draws again by itself (the editing canvas, the thumbnails) may draw before the filters are done, but never a Blur or Mosaic as the picture it hides: a flat patch of the
    // picture's average color stands in, then the filtered picture. A color temperature alone may show the unfiltered picture first. A caller that does not ask waits.
    void aWindowThatDrawsAgainShowsAFlatPatchForBlurAndMosaicUntilTheyAreDone()
    {
        for (const int style : {int(jp::ArtisticStyle::Blur), int(jp::ArtisticStyle::Mosaic), -1}) {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            PictureItem *added = addPicture(w, QStringLiteral("Later"));
            const QString id = added->id;
            added->imageId = w.editor()->doc()->addImage(pureColorPng(400, 300), QStringLiteral("png"));
            auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
            pic()->rect = QRectF(100, 100, 400, 300);
            pic()->imgRect = QRectF(0, 0, 400, 300);
            auto draw = [&](bool later) {
                PaintContext ctx;
                ctx.doc = w.editor()->doc();
                ctx.cache = &w.editor()->cache();
                ctx.opt.filtersLater = later;
                return Renderer::renderToImage(ctx, 0, 1.0);
            };
            const QImage plainPage = draw(false);   // (no filter yet: the picture as it is)
            if (style >= 0) pic()->setArtistic(style, 0.8);
            else pic()->setTemperatureMove(80);
            HeldWorker held;
            int asked = jp::blipFiltersAsked();
            const QImage first = draw(true);
            QCOMPARE(jp::blipFiltersAsked(), asked + 1);
            QTRY_COMPARE(held.entered().load(), 1);
            const int pureOnPage = pureColorPixels(plainPage);
            QVERIFY(pureOnPage > 20000);
            if (style >= 0) {
                // Nothing of the original: a flat patch.
                QVERIFY2(pureColorPixels(first) < 2000, qPrintable(QStringLiteral("filter %1 left %2 pure pixels").arg(style).arg(pureColorPixels(first))));
                QVERIFY(pixelBytes(first) != pixelBytes(plainPage));
                const QRgb centre = first.pixel(300, 250);
                QCOMPARE(first.pixel(150, 150), centre);
                QCOMPARE(first.pixel(450, 350), centre);
                QCOMPARE(first.pixel(300, 250), first.pixel(210, 330));
            } else {
                QCOMPARE(pixelBytes(first), pixelBytes(plainPage));   // (a color temperature may show the picture as it is for a moment)
            }
            // A caller that does not ask waits: the filtered picture at once, whatever the thread beside the window is doing.
            asked = jp::blipFiltersAsked();
            const QImage waited = draw(false);
            QCOMPARE(jp::blipFiltersAsked(), asked);   // (it waited: nothing was left for the thread)
            QVERIFY(pixelBytes(waited) != pixelBytes(plainPage));
            if (style >= 0) QVERIFY(pureColorPixels(waited) < 2000);
            const int finished = jp::blipFiltersFinished();
            held.release();
            QTRY_VERIFY(jp::blipFiltersFinished() > finished);   // (the held job ends)
            QCOMPARE(pixelBytes(draw(true)), pixelBytes(waited));
        }
    }

    // A picture with detail in every pixel (so that any shift or resampling of it shows), a plain border around a middle part (so that a background can be found), of the size given.
    static QByteArray detailedPng(int w, int h)
    {
        QImage im(w, h, QImage::Format_ARGB32);
        im.fill(qRgb(235, 235, 235));
        for (int y = h / 6; y < h - h / 6; ++y)
            for (int x = w / 6; x < w - w / 6; ++x) im.setPixel(x, y, qRgb((x * 37 + y * 11) % 256, (x * y + x) % 256, (y * 53 + x * 3) % 256));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        return png;
    }

    // The page of the window's presentation as a picture at `scale`.
    static QImage pageAt(DeckWindow &w, double scale)
    {
        PaintContext ctx;
        ctx.doc = w.editor()->doc();
        ctx.cache = &w.editor()->cache();
        return Renderer::renderToImage(ctx, 0, scale);
    }

    // How many pixels differ between two pictures (all of them, when they are not of one size).
    static qint64 differingPixels(const QImage &a, const QImage &b)
    {
        if (a.size() != b.size()) return qint64(a.width()) * a.height();
        const QImage x = a.convertToFormat(QImage::Format_ARGB32), y = b.convertToFormat(QImage::Format_ARGB32);
        qint64 n = 0;
        for (int row = 0; row < x.height(); ++row) n += memcmp(x.constScanLine(row), y.constScanLine(row), size_t(x.width()) * 4) != 0 ? [&] {
            qint64 d = 0;
            for (int col = 0; col < x.width(); ++col) d += x.pixel(col, row) != y.pixel(col, row);
            return d;
        }() : 0;
        return n;
    }

    // A cropped picture "Subject" on the first slide: a 120 by 90 pixel picture drawn 160 by 120 points (fewer pixels than points, as the rows cut only such pictures), the frame
    // showing a part of it that starts between two pixels, in the place and turn `setup` makes.
    static PictureItem *croppedSubject(DeckWindow &w, const std::function<void(PictureItem &)> &setup = {}, const QString &name = QStringLiteral("Subject"))
    {
        PictureItem *added = addPicture(w, name);
        const QString id = added->id;
        added->imageId = w.editor()->doc()->addImage(detailedPng(120, 90), QStringLiteral("png"));
        added->rect = QRectF(200, 150, 100, 70);
        added->imgRect = QRectF(-30.3, -20.7, 160, 120);
        if (setup) setup(*added);
        return static_cast<PictureItem *>(w.editor()->doc()->item(id));
    }

    // Takes the rows `kinds` in turn and says how many differing pixels the page then has at 1 and at 3 times, against the page before.
    static qint64 pageChangeAfterRows(DeckWindow &w, const QVector<jp::PictureKept> &kinds, int *changed = nullptr)
    {
        const QImage one = pageAt(w, 1), three = pageAt(w, 3);
        for (const jp::PictureKept kind : kinds) {
            const int n = jp::removeKeptPictureParts(w.editor(), kind);
            if (changed) *changed += n;
        }
        return differingPixels(one, pageAt(w, 1)) + differingPixels(three, pageAt(w, 3));
    }

    // The cropped-parts row leaves every pixel of the page as it was: a plain crop, a turned and a flipped one, a shape's, and every kind of effect on a cropped picture. (The first run of
    // this test found 10,414 pixels different at three times: the row cut to whole pixels and moved what showed.)
    void theCroppedPartsRowLeavesEveryPagePixelAsItWas()
    {
        struct Case {
            const char *what;
            std::function<void(PictureItem &)> setup;
        };
        const QVector<Case> cases = {
            {"plain", {}},
            {"turned", [](PictureItem &p) { p.rotation = 23; }},
            {"flipped", [](PictureItem &p) { p.flipH = true; p.flipV = true; }},
            {"shape", [](PictureItem &p) { p.maskShape = QStringLiteral("ellipse"); }},
            {"sharpen", [](PictureItem &p) { p.setSharpenMove(60); }},
            {"temperature", [](PictureItem &p) { p.setTemperatureMove(50); }},
            {"blur", [](PictureItem &p) { p.setArtistic(int(jp::ArtisticStyle::Blur), 0.6); }},
            {"pencil", [](PictureItem &p) { p.setArtistic(int(jp::ArtisticStyle::PencilGrayscale), 0.6); }},
            {"glow edges", [](PictureItem &p) { p.setArtistic(int(jp::ArtisticStyle::GlowEdges), 0.6); }},
            {"mosaic", [](PictureItem &p) { p.setArtistic(int(jp::ArtisticStyle::Mosaic), 0.6); }},
            {"sharpen and mosaic, turned", [](PictureItem &p) { p.setSharpenMove(40); p.setArtistic(int(jp::ArtisticStyle::Mosaic), 0.5); p.rotation = 10; }},
        };
        for (const Case &c : cases) {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            PictureItem *pic = croppedSubject(w, c.setup);
            const QString original = pic->imageId;
            const QImage before = w.editor()->doc()->image(original);
            QCOMPARE(qAlpha(before.pixel(2, 45)), 255);   // (the left edge, which the frame does not show)
            int changed = 0;
            const qint64 differing = pageChangeAfterRows(w, {jp::PictureKept::CroppedAreas}, &changed);
            QVERIFY2(differing == 0, qPrintable(QStringLiteral("%1: %2 pixels differ").arg(QLatin1String(c.what)).arg(differing)));
            QVERIFY2(changed == 1, c.what);
            pic = static_cast<PictureItem *>(w.editor()->doc()->item(pic->id));
            const QImage after = w.editor()->doc()->image(pic->imageId);
            QCOMPARE(after.size(), before.size());   // (the picture keeps its size and place: that is what keeps every pixel of the page)
            QVERIFY2(pic->imageId != original && after.pixel(2, 45) != before.pixel(2, 45) && after.pixel(2, 45) == after.pixel(117, 45), c.what);   // (what was cropped away is gone)
            QCOMPARE(jp::picturesKeeping(*w.editor()->doc(), jp::PictureKept::CroppedAreas), 0);
        }
    }

    // A removed background stays removed after the cut (the removal is cut to the same pixels), with and without an effect on the picture.
    void theCroppedPartsRowKeepsARemovedBackgroundRemoved()
    {
        for (const bool withEffect : {false, true}) {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            PictureItem *pic = croppedSubject(w, [&](PictureItem &p) {
                p.removal = findBackground(QImage::fromData(detailedPng(120, 90)), {});
                if (withEffect) p.setArtistic(int(jp::ArtisticStyle::Blur), 0.5);
            });
            QVERIFY(pic->removal.isValid());
            const qint64 differing = pageChangeAfterRows(w, {jp::PictureKept::CroppedAreas});
            QVERIFY2(differing == 0, qPrintable(QStringLiteral("%1 pixels differ").arg(differing)));
            pic = static_cast<PictureItem *>(w.editor()->doc()->item(pic->id));
            if (!withEffect) {
                QVERIFY(pic->removal.isValid());   // (the removal is still on the picture, cut with it)
                QCOMPARE(pic->removal.source, w.editor()->doc()->imageSize(pic->imageId));
            }
        }
    }

    // Two objects that show one picture, cropped differently, are cut to the part that either shows and each looks as before.
    void picturesThatShareOnePictureAreCutToWhatEitherShows()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *first = croppedSubject(w);
        const QString firstId = first->id, imageId = first->imageId;
        PictureItem *second = croppedSubject(w, [](PictureItem &p) { p.rect = QRectF(500, 150, 60, 50); p.imgRect = QRectF(-60, -30, 160, 120); }, QStringLiteral("Other"));
        second->imageId = imageId;
        const QString secondId = second->id;
        QCOMPARE(jp::picturesKeeping(*w.editor()->doc(), jp::PictureKept::CroppedAreas), 2);
        const qint64 differing = pageChangeAfterRows(w, {jp::PictureKept::CroppedAreas});
        QVERIFY2(differing == 0, qPrintable(QStringLiteral("%1 pixels differ").arg(differing)));
        auto *a = static_cast<PictureItem *>(w.editor()->doc()->item(firstId));
        auto *b = static_cast<PictureItem *>(w.editor()->doc()->item(secondId));
        QCOMPARE(a->imageId, b->imageId);   // (still one picture)
        QVERIFY(a->imageId != imageId);
        const QImage after = w.editor()->doc()->image(a->imageId);
        QVERIFY(after.pixel(1, 1) != qRgb(235, 235, 235) && after.pixel(1, 1) == after.pixel(2, 2));   // (what neither shows is gone: one flat color)
        QCOMPARE(qAlpha(after.pixel(60, 45)), 255);
        // (a third object that shows the whole picture keeps it whole: nothing is cut, and nothing changes)
        PictureItem *whole = croppedSubject(w, [](PictureItem &p) { p.rect = QRectF(0, 0, 160, 120); p.imgRect = QRectF(0, 0, 160, 120); }, QStringLiteral("Whole"));
        whole->imageId = a->imageId;
        a->imageId = b->imageId = whole->imageId;
        QCOMPARE(jp::picturesKeeping(*w.editor()->doc(), jp::PictureKept::CroppedAreas), 0);
    }

    // The three rows in every order on one picture that has all three kinds of hidden data (an effect's original, a removed background's whole picture, and a crop's far side): the page
    // never changes by a pixel, not after saving and opening it again, and the file no longer holds the picture it began with.
    void theThreeInspectRowsInAnyOrderChangeNoPixelAndLeaveNothingBehind()
    {
        QVector<int> order = {0, 1, 2};
        const jp::PictureKept kinds[] = {jp::PictureKept::FilterOriginals, jp::PictureKept::RemovedBackgrounds, jp::PictureKept::CroppedAreas};
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QByteArray png = detailedPng(120, 90);
        int run = 0;
        do {
            DeckWindow w;
            w.resize(1200, 800);
            w.show();
            PictureItem *pic = croppedSubject(w, [&](PictureItem &p) {
                p.removal = findBackground(QImage::fromData(png), {});
                p.setArtistic(int(jp::ArtisticStyle::Mosaic), 0.5);
            });
            const QString id = pic->id;
            QVERIFY(pic->removal.isValid());
            const QImage one = pageAt(w, 1), three = pageAt(w, 3);
            QVector<jp::PictureKept> take;
            for (const int i : order) take << kinds[i];
            const qint64 differing = pageChangeAfterRows(w, take);
            const QString what = QStringLiteral("order %1%2%3").arg(order[0]).arg(order[1]).arg(order[2]);
            QVERIFY2(differing == 0, qPrintable(what + QStringLiteral(": %1 pixels differ").arg(differing)));
            for (const jp::PictureKept kind : kinds) QVERIFY2(jp::picturesKeeping(*w.editor()->doc(), kind) == 0, qPrintable(what));
            pic = static_cast<PictureItem *>(w.editor()->doc()->item(id));
            QVERIFY2(!pic->removal.isValid() || pic->removal.source == w.editor()->doc()->imageSize(pic->imageId), qPrintable(what));
            const QString path = dir.filePath(QStringLiteral("order%1.pptx").arg(++run));
            QVERIFY(w.saveTo(path));
            // The file holds no picture of the size it began with.
            for (const QImage &m : mediaOf(path)) QVERIFY2(m.size() != QSize(120, 90) || m.pixel(2, 45) != qRgb(235, 235, 235), qPrintable(what));
            DeckWindow back;
            back.resize(1200, 800);
            QVERIFY(back.openFile(path));
            const qint64 reopened = differingPixels(one, pageAt(back, 1)) + differingPixels(three, pageAt(back, 3));
            QVERIFY2(reopened == 0, qPrintable(what + QStringLiteral(": %1 pixels differ after saving and opening").arg(reopened)));
        } while (std::next_permutation(order.begin(), order.end()));
        QCOMPARE(run, 6);
    }

    // What waits for the thread beside the window is held to a number of pixels in all, not only to a number of pictures: with the thread held, pictures of 3 million pixels are asked
    // for until the queue says no, and what waits never comes to more than the bound. A picture of more than the filters' work size is made smaller there, and comes out as the same
    // pixels the filters give on the window thread.
    void theFilterQueueIsHeldToAnAmountOfPixelsAndWorksOnTheBoundedCopy()
    {
        HeldWorker held;
        jp::BlipEffect mosaic;
        mosaic.kind = jp::BlipEffect::Artistic;
        mosaic.style = int(jp::ArtisticStyle::Mosaic);
        mosaic.amount = 0.5;
        const QVector<jp::BlipEffect> effects{mosaic};
        QVector<QImage> images;
        for (int i = 0; i < 12; ++i) {
            QImage im(1800, 1800, QImage::Format_ARGB32);
            im.fill(qRgb(i * 20, 40, 200 - i * 10));
            images << im;
        }
        int pending = 0;
        for (int i = 0; i < images.size(); ++i) {
            bool p = false;
            jp::blipAdjustedLater(images[i], effects, QStringLiteral("queue-%1").arg(i), &p);
            pending += p;
            if (i == 0) QTRY_COMPARE(held.entered().load(), 1);   // (the first is running now: the rest wait)
            QVERIFY(jp::blipFiltersQueuedPixels() <= jp::kBlipQueueMostPixels);
        }
        QCOMPARE(pending, int(images.size()));   // (each is told it is not done)
        QVERIFY2(jp::blipFiltersQueued() <= 7, qPrintable(QString::number(jp::blipFiltersQueued())));   // (24 million pixels hold seven of 3.24 million, not the sixteen jobs the queue used to)
        QVERIFY(jp::blipFiltersQueued() >= 1);
        held.release();
        QTRY_COMPARE(jp::blipFiltersQueued(), 0);
        // A picture of more than the work size: the same pixels as on the window thread.
        QImage big(2600, 2300, QImage::Format_ARGB32);
        for (int y = 0; y < big.height(); ++y)
            for (int x = 0; x < big.width(); ++x) big.setPixel(x, y, qRgb(x % 256, y % 256, (x + y) % 256));
        bool p = false;
        jp::blipAdjustedLater(big, effects, QStringLiteral("queue-big"), &p);
        QVERIFY(p);
        const QImage expected = jp::applyBlipEffects(big, effects);
        QVERIFY(expected.width() * expected.height() <= jp::kPictureFxWorkPixels);
        QImage got;
        QTRY_VERIFY((got = jp::blipAdjustedLater(big, effects, QStringLiteral("queue-big"), &p), !p));
        QCOMPARE(pixelBytes(got), pixelBytes(expected));
    }

    // The linear blur, pinned: a picture narrower than the radius gets a window of its own size (edges repeat the outermost pixel), horizontal pass then vertical, each rounded: worked out
    // here by the plain sum for each pixel. (The branch's first blur used the radius whatever the picture; this one, never released, is the definition.)
    void aBlurOnAPictureNarrowerThanItsRadiusIsThePlainWindowSum()
    {
        const int w = 250, h = 5;
        QImage im(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) im.setPixel(x, y, qRgb((x * 5 + y * 40) % 256, (x * 13) % 256, (x * x + y) % 256));
        jp::BlipEffect blur;
        blur.kind = jp::BlipEffect::Artistic;
        blur.style = int(jp::ArtisticStyle::Blur);
        blur.amount = 1.0;
        const QImage got = jp::applyBlipFilter(im, blur).convertToFormat(QImage::Format_ARGB32);
        const int r = std::max(1, int(qint64(w) * 100 / 1250));   // (20 for the full amount: more than the picture is high)
        QVERIFY(r > h);
        const int rx = std::min(r, w), ry = std::min(r, h);
        for (int c = 0; c < 3; ++c) {
            auto channel = [&](int x, int y) { const QRgb v = im.pixel(x, y); return c == 0 ? qRed(v) : c == 1 ? qGreen(v) : qBlue(v); };
            std::vector<std::vector<int>> rows(h, std::vector<int>(w));
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    int sum = 0;
                    for (int i = -rx; i <= rx; ++i) sum += channel(std::clamp(x + i, 0, w - 1), y);
                    rows[y][x] = (sum + (2 * rx + 1) / 2) / (2 * rx + 1);
                }
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    int sum = 0;
                    for (int i = -ry; i <= ry; ++i) sum += rows[std::clamp(y + i, 0, h - 1)][x];
                    const int want = (sum + (2 * ry + 1) / 2) / (2 * ry + 1);
                    const QRgb v = got.pixel(x, y);
                    QCOMPARE(c == 0 ? qRed(v) : c == 1 ? qGreen(v) : qBlue(v), want);
                }
        }
    }

    // Effects that are left out are never left out silently: a picture whose color effects cost more than the work allows (filters on it too: only a file can have both) is counted in the
    // row that says so, and a drawing with a crop or an effect, which the rows cannot change, is told in the row's own words.
    void effectsLeftOutAndPicturesTheRowsCannotChangeAreSaid()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *added = addPicture(w, QStringLiteral("Costly"));
        const QString id = added->id;
        added->imageId = w.editor()->doc()->addImage(structuredPng(1000, 1000), QStringLiteral("png"));
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        pic()->setSharpenMove(50);
        QCOMPARE(jp::picturesWithEffectsLeftOut(*w.editor()->doc()), 0);
        jp::BlipEffect hue;
        hue.kind = jp::BlipEffect::Hsl;
        hue.hue = 40;
        for (int i = 0; i < 20; ++i) pic()->blipEffects.append(hue);   // (20 hue turns on a million pixels: more than the work allows)
        QCOMPARE(jp::picturesWithEffectsLeftOut(*w.editor()->doc()), 1);
        QCOMPARE(jp::pictureInspectItems(w.editor()).size(), qsizetype(4));
        // The same work, without a filter, is the picture as it is: not a thing left out of a filtered one.
        pic()->blipEffects = QVector<jp::BlipEffect>(20, hue);
        QCOMPARE(jp::picturesWithEffectsLeftOut(*w.editor()->doc()), 0);
        // A drawing with a crop.
        const QByteArray svg = "<svg xmlns='http://www.w3.org/2000/svg' width='100' height='80'><rect width='100' height='80' fill='#c00'/></svg>";
        auto drawing = std::make_shared<PictureItem>();
        drawing->name = QStringLiteral("Drawing");
        drawing->imageId = w.editor()->doc()->addImage(svg, QStringLiteral("svg"));
        drawing->rect = QRectF(300, 300, 50, 40);
        drawing->imgRect = QRectF(-20, -20, 100, 80);
        w.editor()->addItem(drawing, false);
        const QVector<InspectItem> rows = jp::pictureInspectItems(w.editor());
        QVERIFY2(rows[2].detail.contains(QStringLiteral("drawing or a linked picture")), qPrintable(rows[2].detail));
        QVERIFY(!rows[0].detail.contains(QStringLiteral("drawing or a linked picture")));
        QCOMPARE(rows[2].count(), 0);
    }

    // A picture with many more pixels than points (a photograph) is cleared the same way and looks the same, at the zooms that matter (the margin kept around what shows covers the pixels the
    // page's making it smaller reads).
    void aPhotographDrawnSmallerThanItIsStillLooksTheSameAfterTheCroppedPartsRow()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *pic = croppedSubject(w);
        pic->imageId = w.editor()->doc()->addImage(detailedPng(480, 360), QStringLiteral("png"));   // (three pixels to the point)
        const QString id = pic->id;
        const QImage one = pageAt(w, 1), two = pageAt(w, 2), three = pageAt(w, 3);
        QCOMPARE(jp::picturesKeeping(*w.editor()->doc(), jp::PictureKept::CroppedAreas), 1);
        QCOMPARE(jp::removeKeptPictureParts(w.editor(), jp::PictureKept::CroppedAreas), 1);
        QCOMPARE(differingPixels(one, pageAt(w, 1)) + differingPixels(two, pageAt(w, 2)) + differingPixels(three, pageAt(w, 3)), qint64(0));
        pic = static_cast<PictureItem *>(w.editor()->doc()->item(id));
        QVERIFY(w.editor()->doc()->image(pic->imageId).pixel(8, 180) != qRgb(235, 235, 235));
        QCOMPARE(jp::picturesKeeping(*w.editor()->doc(), jp::PictureKept::CroppedAreas), 0);
    }

    // A window closed while its pictures' filters are queued and running: the thread beside the window finishes into nothing (no listener is left to call), and what it made is kept.
    void aWindowClosedWithFiltersQueuedLeavesNothingCalled()
    {
        HeldWorker held;
        QVector<qint64> sizes;
        {
            auto *w = new DeckWindow;
            w->resize(1200, 800);
            w->show();
            for (int i = 0; i < 3; ++i) {
                PictureItem *added = addPicture(*w, QStringLiteral("Closing %1").arg(i));
                added->imageId = w->editor()->doc()->addImage(pureColorPng(400 + 2 * i, 300), QStringLiteral("png"));
                added->rect = QRectF(50 + 250 * i, 100, 200, 150);
                added->imgRect = QRectF(0, 0, 200, 150);
                added->setArtistic(int(jp::ArtisticStyle::Mosaic), 0.5);
            }
            const int finished = jp::blipFiltersFinished();
            w->canvas()->grab();   // (the canvas draws: the filters are queued, the first one is running and held)
            QTRY_COMPARE(held.entered().load(), 1);
            QVERIFY(jp::blipFiltersQueued() >= 1);
            delete w;
            QCOMPARE(jp::blipFiltersFinished(), finished);   // (nothing had finished: the thread was held while the window went)
        }
        held.release();
        QTRY_COMPARE(jp::blipFiltersQueued(), 0);
        QTRY_VERIFY(jp::blipFiltersFinished() >= 3);
        QCoreApplication::processEvents();   // (the calls the thread posted find no listener: nothing is touched)
    }

    // The program quitting while a filter runs and others wait: the shutdown step ends the running one (it looks at its flag between rows), drops the waiting ones, and the thread ends
    // inside the time it is given.
    void quittingWhileFiltersRunEndsTheThreadAndDropsTheQueue()
    {
        HeldWorker held;
        jp::BlipEffect mosaic;
        mosaic.kind = jp::BlipEffect::Artistic;
        mosaic.style = int(jp::ArtisticStyle::Mosaic);
        mosaic.amount = 0.5;
        const QVector<jp::BlipEffect> effects{mosaic};
        QVector<QImage> images;
        for (int i = 0; i < 4; ++i) {
            QImage im(1500, 1500, QImage::Format_ARGB32);
            im.fill(qRgb(10 + i, 80, 160));
            images << im;
            bool p = false;
            jp::blipAdjustedLater(im, effects, QStringLiteral("quit-%1").arg(i), &p);
            if (i == 0) QTRY_COMPARE(held.entered().load(), 1);
        }
        QVERIFY(jp::blipFiltersQueued() >= 1);
        const int finished = jp::blipFiltersFinished();
        QCOMPARE(jp::stopBlipFilters(60000), 0);   // (0: the thread ended in the time given)
        QCOMPARE(jp::blipFiltersQueued(), 0);
        QCOMPARE(jp::blipFiltersFinished(), finished + 1);   // (the running one was cut short and counted; the waiting ones never started)
        jp::restartBlipFiltersForTest();   // (the quit is over for the tests after this one)
        bool p = false;
        jp::blipAdjustedLater(images[3], effects, QStringLiteral("quit-after"), &p);
        held.release();
        QTRY_VERIFY(jp::blipFiltersFinished() > finished + 1);
    }

    // Inspect finds what a picture keeps beside how it looks, and takes it out: the original under an effect, the whole picture under a removed background, and the cropped-away parts. Each
    // is one step of Undo, the picture looks the same after (the finished picture is all that is left of it), and the saved file no longer holds the original.
    void inspectFindsAndRemovesWhatPicturesKeepUnderTheirLook()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *added = addPicture(w, QStringLiteral("Kept"));
        const QString id = added->id;
        const QByteArray png = structuredPng(120, 90);
        added->imageId = w.editor()->doc()->addImage(png, QStringLiteral("png"));
        const QString originalId = added->imageId;
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        pic()->rect = QRectF(100, 100, 100, 75);
        pic()->imgRect = QRectF(-20, -15, 140, 105);   // (cropped: the frame shows the middle part)
        pic()->removal = findBackground(w.editor()->doc()->image(originalId), {});
        QVERIFY(pic()->removal.isValid());
        pic()->setArtistic(int(jp::ArtisticStyle::Mosaic), 0.5);
        const QVector<InspectItem> rows = jp::pictureInspectItems(w.editor());
        QCOMPARE(rows.size(), qsizetype(3));
        for (const InspectItem &row : rows) QCOMPARE(row.count(), 1);
        QVERIFY(rows[0].detail.contains(QStringLiteral("Reset Picture")));
        const QImage before = Renderer::processedImage(*w.editor()->doc(), *pic(), QSizeF(), QSizeF());
        const QString inFile = dir.filePath(QStringLiteral("before.pptx")), outFile = dir.filePath(QStringLiteral("after.pptx"));
        QVERIFY(w.saveTo(inFile));
        const QImage original = QImage::fromData(png);
        bool held = false;
        for (const QImage &m : mediaOf(inFile)) held = held || pixelBytes(m) == pixelBytes(original);
        QVERIFY(held);   // (the file holds the original)
        // Each row: one step, which Undo gives back.
        for (int i = 0; i < 3; ++i) {
            const int steps = w.editor()->undoStack()->index();
            QCOMPARE(rows[i].remove(), 1);
            QCOMPARE(w.editor()->undoStack()->index(), steps + 1);
            w.editor()->undo();
            QCOMPARE(rows[i].count(), 1);
            QCOMPARE(pic()->imageId, originalId);
        }
        // All three, the crop last (the picture's pixels are what the first two make of them).
        for (const int i : {1, 0}) QCOMPARE(rows[i].remove(), 1);
        QCOMPARE(rows[2].remove(), 0);   // (the removal already made what is cropped away clear: nothing is left to take)
        for (const InspectItem &row : rows) QCOMPARE(row.count(), 0);
        QVERIFY(pic()->removal.isValid() == false);
        QVERIFY(jp::blipFilters(pic()->blipEffects).isEmpty());
        QVERIFY(pic()->imageId != originalId);
        // What shows is the same (the rows were taken in an order that kept the finished look: the removal and the effect first, the crop last).
        QCOMPARE(w.editor()->doc()->imageSize(pic()->imageId), original.size());   // (the picture keeps its size: only what is cropped away is cleared)
        QCOMPARE(qAlpha(w.editor()->doc()->image(pic()->imageId).pixel(1, 1)), 0);
        QVERIFY(w.saveTo(outFile));
        for (const QImage &m : mediaOf(outFile)) QVERIFY(pixelBytes(m) != pixelBytes(original));   // (the original is no longer in the file)
        QVERIFY(!before.isNull());
    }

    // A file can hold more effects on a picture than can be worked out: the first are shown, and Inspect says that the rest are left out (nothing a person can make is).
    void inspectSaysWhenAPicturesEffectsAreLeftOut()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *added = addPicture(w, QStringLiteral("Many"));
        const QString id = added->id;
        added->imageId = w.editor()->doc()->addImage(structuredPng(1000, 1000), QStringLiteral("png"));
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        pic()->setSharpenMove(50);
        pic()->setTemperatureMove(50);
        pic()->setArtistic(int(jp::ArtisticStyle::Blur), 0.5);
        QCOMPARE(jp::pictureInspectItems(w.editor()).size(), qsizetype(3));
        jp::BlipEffect sharpen;
        sharpen.kind = jp::BlipEffect::Sharpen;
        sharpen.amount = 0.5;
        pic()->blipEffects = QVector<jp::BlipEffect>(jp::kMaxBlipEffects, sharpen);
        const QVector<InspectItem> rows = jp::pictureInspectItems(w.editor());
        QCOMPARE(rows.size(), qsizetype(4));
        QVERIFY(!rows[3].removable);
        QCOMPARE(rows[3].count(), 1);
    }

    // Set Transparent Color takes the color of the picture's own pixel where it is clicked (the picture as it was put in), not what the window shows there: on a
    // picture that is zoomed and recolored the window shows gray, and the color stored is the blue of the stripe, so the pixels of it are cleared.
    void setTransparentColorTakesThePicturesOwnPixel()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        PictureItem *added = addPicture(w, QStringLiteral("Flag"));
        const QString id = added->id;
        auto pic = [&] { return static_cast<PictureItem *>(w.editor()->doc()->item(id)); };
        pic()->recolor = PictureItem::Grayscale;
        pic()->setSaturationMove(80);
        w.canvas()->setZoom(2.5);
        const QPointF onStripe(300, 180);   // (the right half of the picture, 100, 100, 240 by 160, is blue)
        const QPoint at = w.canvas()->viewport()->mapTo(&w, w.canvas()->pageToView(onStripe).toPoint());
        auto cleared = [&] {
            const QImage shown = Renderer::processedImage(*w.editor()->doc(), *pic(), QSizeF(), QSizeF());
            int n = 0;
            for (int y = 0; y < shown.height(); ++y)
                for (int x = 0; x < shown.width(); ++x) n += qAlpha(shown.pixel(x, y)) == 0;
            return n;
        };
        QCOMPARE(cleared(), 0);
        w.act(QStringLiteral("pic.transparent"))->trigger();
        QWidget *overlay = nullptr;
        for (QWidget *c : w.findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
            if (c->cursor().shape() == Qt::CrossCursor) overlay = c;
        QVERIFY(overlay);
        QTest::mouseClick(overlay, Qt::LeftButton, {}, at);
        QVERIFY(pic()->hasTransparentColor);
        QCOMPARE(pic()->transparentColor, QColor(30, 30, 200));
        QVERIFY(cleared() > 0);
        // A click that is not on a selected picture changes nothing.
        w.editor()->undo();
        QVERIFY(!pic()->hasTransparentColor);
        QVERIFY(!w.setTransparentColorAt(w.canvas()->viewport()->mapTo(&w, w.canvas()->pageToView(QPointF(10, 10)).toPoint())));
        QVERIFY(!pic()->hasTransparentColor);
    }

    // A saturation that is not a number is not taken, and one out of range is held to -100..100, as set and as read from a file (here the file says "nan",
    // and 9000%, and the program's own list of changes has one past the range).
    void aSaturationThatIsNotANumberIsNotTaken()
    {
        PictureItem p;
        p.setSaturationMove(40);
        p.setSaturationMove(std::nan(""));
        QCOMPARE(p.saturationMove(), 40.0);
        p.setSaturationMove(std::numeric_limits<double>::infinity());
        QCOMPARE(p.saturationMove(), 40.0);
        p.setSaturationMove(900);
        QCOMPARE(p.saturationMove(), 100.0);
    }

    void aSaturationOutOfRangeInTheProgramsListIsHeld()
    {
        QJsonObject o;
        o[QStringLiteral("k")] = QStringLiteral("hsl");
        o[QStringLiteral("sat")] = 50.0;
        o[QStringLiteral("hue")] = 0;
        o[QStringLiteral("lum")] = -30.0;
        const BlipEffect e = BlipEffect::fromJson(o);
        QCOMPARE(e.saturation, 1.0);
        QCOMPARE(e.lightness, -1.0);
    }

    void aSaturationInAFileThatIsNotANumberOrOutOfRangeIsHeld()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("sat.pptx"));
        QString id;
        {
            DeckWindow w;
            id = addPicture(w, QStringLiteral("Flag"))->id;
            static_cast<PictureItem *>(w.editor()->doc()->item(id))->setSaturationMove(50);
            QVERIFY(w.saveTo(path));
        }
        QMap<QString, QByteArray> parts;
        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly));
        QVERIFY(readZip(saved.readAll(), parts));
        saved.close();
        auto patched = [&](const QString &sat, const QString &file) {
            ZipWriter out;
            bool changed = false;
            for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
                QByteArray data = it.value();
                if (it.key().startsWith(QLatin1String("ppt/slides/slide")) && it.key().endsWith(QLatin1String(".xml")) && data.contains("<a:hsl")) {
                    QString xml = QString::fromUtf8(data);
                    const QString was = QStringLiteral("sat=\"50000\"");
                    changed = xml.contains(was);
                    xml.replace(was, QStringLiteral("sat=\"%1\"").arg(sat));
                    data = xml.toUtf8();
                }
                out.add(it.key(), data);
            }
            if (!changed) return false;
            QFile f(file);
            return f.open(QIODevice::WriteOnly) && f.write(out.finish()) > 0;
        };
        auto read = [&](const QString &file) {
            DeckWindow b;
            if (!b.openFile(file)) return -1000.0;
            const PictureItem *back = pictureNamed(b, QStringLiteral("Flag"));
            return back ? back->saturationMove() : -2000.0;
        };
        QVERIFY(patched(QStringLiteral("nan"), dir.filePath(QStringLiteral("nan.pptx"))));
        QCOMPARE(read(dir.filePath(QStringLiteral("nan.pptx"))), 0.0);
        QVERIFY(patched(QStringLiteral("9000000"), dir.filePath(QStringLiteral("big.pptx"))));
        QCOMPARE(read(dir.filePath(QStringLiteral("big.pptx"))), 100.0);
    }

    // Move Split (View > Window): the arrow keys move the divider between the slide list and the slide; Tab goes on to the divider above the
    // notes; Enter ends it, and then the keys do nothing to the panes. Each pane keeps a share (the divider cannot be lost).
    void moveSplitMovesTheDividersWithTheArrowKeys()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        auto *vertical = qobject_cast<QSplitter *>(w.notesPane()->parentWidget());
        QVERIFY(vertical);
        auto *horizontal = qobject_cast<QSplitter *>(vertical->parentWidget());
        QVERIFY(horizontal && horizontal->orientation() == Qt::Horizontal);
        const QList<int> v0 = vertical->sizes();
        w.act(QStringLiteral("win.moveSplit"))->trigger();
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("Move Split")));
        const QList<int> start = horizontal->sizes();
        QTest::keyClick(&w, Qt::Key_Right);
        QCOMPARE(horizontal->sizes()[0], start[0] + SplitMover::kStep);
        const QList<int> now = horizontal->sizes();
        QCOMPARE(std::accumulate(now.cbegin(), now.cend(), 0), std::accumulate(start.cbegin(), start.cend(), 0));
        QTest::keyClick(&w, Qt::Key_Right, Qt::ShiftModifier);
        QCOMPARE(horizontal->sizes()[0], start[0] + SplitMover::kStep + 4 * SplitMover::kStep);
        // Up and Down do nothing to a side-by-side divider.
        QTest::keyClick(&w, Qt::Key_Up);
        QCOMPARE(vertical->sizes(), v0);
        QTest::keyClick(&w, Qt::Key_Tab);
        const QList<int> vStart = vertical->sizes();
        QTest::keyClick(&w, Qt::Key_Up);
        QCOMPARE(vertical->sizes()[1], vStart[1] + SplitMover::kStep);
        const QList<int> before = horizontal->sizes();
        QTest::keyClick(&w, Qt::Key_Left);
        QCOMPARE(horizontal->sizes(), before);
        // A divider keeps a share of the two panes it is between.
        for (int i = 0; i < 200; ++i) QTest::keyClick(&w, Qt::Key_Down);
        QVERIFY(vertical->sizes()[1] > 0);
        QTest::keyClick(&w, Qt::Key_Return);
        QVERIFY(w.statusBar()->currentMessage().isEmpty());
        QTest::keyClick(&w, Qt::Key_Right);
        QCOMPARE(horizontal->sizes(), before);
    }
    // A pane taken away while Move Split is on (the divider it had is gone): moving on to the next divider, moving, and ending do not touch it.
    void moveSplitSurvivesAPaneTakenAway()
    {
        QWidget win;
        auto *lay = new QVBoxLayout(&win);
        auto *split = new QSplitter(Qt::Horizontal, &win);
        lay->addWidget(split);
        for (int i = 0; i < 3; ++i) split->addWidget(new QLabel(QStringLiteral("pane"), &win));
        win.resize(500, 200);
        win.show();
        QVERIFY(QTest::qWaitForWindowExposed(&win));
        SplitMover mover;
        QVERIFY(mover.start({split}));
        delete split->widget(2);
        QTest::keyClick(&win, Qt::Key_Tab);
        QTest::keyClick(&win, Qt::Key_Tab);
        QTest::keyClick(&win, Qt::Key_Right);
        mover.stop();
        QVERIFY(!mover.active());
    }

    // The keys of another window are not read by Move Split, and a dialog opening over the window ends it.
    void moveSplitReadsOnlyItsOwnWindowAndEndsForADialog()
    {
        QWidget win;
        auto *lay = new QVBoxLayout(&win);
        auto *split = new QSplitter(Qt::Horizontal, &win);
        lay->addWidget(split);
        for (int i = 0; i < 2; ++i) split->addWidget(new QLabel(QStringLiteral("pane"), &win));
        win.resize(500, 200);
        win.show();
        QVERIFY(QTest::qWaitForWindowExposed(&win));
        QLineEdit other;
        other.show();
        QVERIFY(QTest::qWaitForWindowExposed(&other));
        win.activateWindow();
        SplitMover mover;
        QVERIFY(mover.start({split}));
        const QList<int> start = split->sizes();
        QTest::keyClick(&other, Qt::Key_Right);
        QCOMPARE(split->sizes(), start);
        QVERIFY(mover.active());
        QTest::keyClick(&win, Qt::Key_Right);
        QCOMPARE(split->sizes()[0], start[0] + SplitMover::kStep);
        QDialog dialog(&win);
        dialog.setModal(true);
        dialog.show();
        QTRY_VERIFY(!mover.active());
    }

    // Merge Shapes (libs/page/app/shapetools.cpp): two squares that overlap, every operation, through the window's own commands.
    static ShapeItem *square(DeckWindow &w, const QString &name, double x, double y, const QColor &color, int number)
    {
        ShapeItem *s = addShape(w, name, QRectF(x, y, 200, 200));
        s->shape = QStringLiteral("rect");
        s->fill = Fill::solid(ColorRef::rgb(color));
        s->stroke = Stroke::none();
        s->sourceId = number;
        return s;
    }
    static bool filledAt(DeckWindow &w, double x, double y)
    {
        const QImage img = slideImage(*w.editor()->doc());
        const double k = img.width() / w.editor()->doc()->pageSize().width();
        return img.pixelColor(int(x * k), int(y * k)).lightness() < 250;
    }
    static QStringList shapeJson(DeckWindow &w)
    {
        QStringList out;
        for (const ItemPtr &it : w.editor()->doc()->pages[0]->items) out << QString::fromUtf8(QJsonDocument(it->toJson()).toJson(QJsonDocument::Compact));
        return out;
    }

    void mergeShapesInTheDeckMakesOneShapeOrPiecesAndUndoesInOneStep()
    {
        struct Case { const char *id; int count; bool onlyA, overlap, onlyB; QString undo; } cases[] = {
            {"shape.union", 1, true, true, true, "Union"}, {"shape.combine", 1, true, false, true, "Combine"},
            {"shape.subtract", 1, true, false, false, "Subtract"}, {"shape.intersect", 1, false, true, false, "Intersect"},
            {"shape.fragment", 3, true, true, true, "Fragment"}};
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        for (const Case &c : cases) {
            w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
            QVERIFY(!w.act(QString::fromLatin1(c.id))->isEnabled());   // (nothing is selected)
            ShapeItem *a = square(w, QStringLiteral("A"), 100, 100, Qt::black, 4);
            square(w, QStringLiteral("B"), 200, 200, Qt::black, 5);
            w.editor()->select(QStringList{a->id, w.editor()->doc()->pages[0]->items[1]->id});
            QTRY_VERIFY(w.act(QString::fromLatin1(c.id))->isEnabled());
            const QStringList before = shapeJson(w);
            w.act(QString::fromLatin1(c.id))->trigger();
            QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), c.count);
            QCOMPARE(filledAt(w, 150, 150), c.onlyA);
            QCOMPARE(filledAt(w, 250, 250), c.overlap);
            QCOMPARE(filledAt(w, 350, 350), c.onlyB);
            QVERIFY(!filledAt(w, 150, 350));   // (a corner neither covers)
            QCOMPARE(w.editor()->undoStack()->undoText(), c.undo);
            QVERIFY(!static_cast<ShapeItem *>(w.editor()->doc()->pages[0]->items[0].get())->customPath.isEmpty());   // (an ordinary freeform shape)
            w.editor()->undo();
            QCOMPARE(shapeJson(w), before);   // one step puts both shapes back as they were
            w.editor()->redo();
            QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), c.count);
        }
    }

    void aMergedShapeKeepsTheFirstOnesLookLinkAndEffectsAndSavesInEveryFormat()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
        ShapeItem *a = square(w, QStringLiteral("A"), 100, 100, Qt::red, 4);
        a->hyperlink = QStringLiteral("https://example.com/first");
        ShapeItem *b = square(w, QStringLiteral("B"), 200, 200, Qt::blue, 5);
        const QStringList ids{a->id, b->id};
        w.editor()->doc()->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animSteps({animAppear(4), animAppear(5)}));
        QCOMPARE(int(deck::readAnimations(*w.editor()->doc()->pages[0], w.editor()->doc()).effects.size()), 2);
        w.editor()->select(ids);
        QTRY_VERIFY(w.act(QStringLiteral("shape.union"))->isEnabled());
        w.act(QStringLiteral("shape.union"))->trigger();
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 1);
        const auto *merged = static_cast<ShapeItem *>(w.editor()->doc()->pages[0]->items[0].get());
        QCOMPARE(merged->hyperlink, QStringLiteral("https://example.com/first"));
        {
            const QImage img = slideImage(*w.editor()->doc());
            const double k = img.width() / w.editor()->doc()->pageSize().width();
            const QColor c = img.pixelColor(int(350 * k), int(350 * k));   // (where only the second shape was)
            QVERIFY(c.red() > 200 && c.blue() < 80);   // the first shape's red, not the second's blue
        }
        // The effect on the shape merged away goes with it; the one on the first stays on the result.
        const deck::AnimSequence seq = deck::readAnimations(*w.editor()->doc()->pages[0], w.editor()->doc());
        QCOMPARE(int(seq.effects.size()), 1);
        QCOMPARE(seq.effects[0].shape, 4);
        // (An .odp file is saved with the box that says what it can't hold answered by the test: the list must not name the merged shape's outline, which the file holds.)
        QStringList lostInOdp;
        setLossyStandIn([&](const LossyAsk &ask, bool *) { lostInOdp = ask.lost; return LossyChoice::SaveAnyway; });
        auto reset = qScopeGuard([] { setLossyStandIn(nullptr); });
        for (const char *name : {"merged.pptx", "merged.odp"}) {
            const QString file = dir.filePath(QString::fromLatin1(name));
            QVERIFY2(w.saveTo(file), name);
            DeckWindow again;
            QStringList said;   // (anything the open says is a loss: no box may come up; the timer looks every 20 ms and closes what it finds, so that a failure does not hang)
            QTimer looker;
            looker.setInterval(20);
            connect(&looker, &QTimer::timeout, &looker, [&] {
                for (QWidget *top : QApplication::topLevelWidgets())
                    if (auto *box = qobject_cast<QMessageBox *>(top); box && box->isVisible()) {
                        said << box->text();
                        box->accept();
                    }
            });
            looker.start();
            const bool opened = again.openFile(file);
            looker.stop();
            QVERIFY2(opened, name);
            // (an .odp file does not keep animations, and the box says so, as it does for any file; nothing about the shape's outline may be lost)
            QVERIFY2(said.isEmpty() || (QLatin1String(name).endsWith(QLatin1String("odp")) && said.join(QLatin1Char(' ')).contains(QLatin1String("animations")) && !said.join(QLatin1Char(' ')).contains(QLatin1String("shape"), Qt::CaseInsensitive)), qPrintable(said.join(QLatin1Char('|'))));
            again.editor()->setCurrentPage(0);
            QCOMPARE(int(again.editor()->doc()->pages[0]->items.size()), 1);
            const auto *back = static_cast<ShapeItem *>(again.editor()->doc()->pages[0]->items[0].get());
            QVERIFY2(!back->customPath.isEmpty(), name);
            QCOMPARE(back->hyperlink, QStringLiteral("https://example.com/first"));
            QVERIFY2(filledAt(again, 150, 150) && filledAt(again, 250, 250) && filledAt(again, 350, 350) && !filledAt(again, 150, 350), name);
            if (QLatin1String(name).endsWith(QLatin1String("odp"))) QVERIFY2(!lostInOdp.join(QLatin1Char(' ')).contains(QLatin1String("shape"), Qt::CaseInsensitive), qPrintable(lostInOdp.join(QLatin1Char('|'))));
            if (QLatin1String(name).endsWith(QLatin1String("pptx")))
                QCOMPARE(int(deck::readAnimations(*again.editor()->doc()->pages[0], again.editor()->doc()).effects.size()), 1);
        }
    }

    // A shape taken away (merged into another, or deleted) that a trigger names takes only what names it: through the window's own save and open, the main
    // sequence and the other triggers are there, and one Undo puts the shapes and the timing back as they were.
    void aShapeATriggerNamesCanBeMergedOrDeletedAwayWithoutLosingTheRestOfTheTiming()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        setLossyStandIn([&](const LossyAsk &, bool *) { return LossyChoice::SaveAnyway; });
        auto reset = qScopeGuard([] { setLossyStandIn(nullptr); });
        for (const bool merge : {true, false})
            for (const char *which : {"object", "target"}) {
                const QString label = QString::fromLatin1(merge ? "merge " : "delete ") + QLatin1String(which);
                DeckWindow w;
                w.resize(1200, 800);
                w.show();
                w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
                ShapeItem *m = square(w, QStringLiteral("M"), 100, 100, Qt::red, 4);
                ShapeItem *b = square(w, QStringLiteral("B"), 200, 200, Qt::blue, 5);
                for (const auto &spec : {std::make_pair("C", 6), std::make_pair("D", 7), std::make_pair("E", 8)}) {
                    ShapeItem *x = addShape(w, QString::fromLatin1(spec.first), QRectF(600 + 80 * spec.second, 50, 60, 60));
                    x->sourceId = spec.second;
                }
                Page &page = *w.editor()->doc()->pages[0];
                const bool object = QLatin1String(which) == QLatin1String("object");
                // Main effects on M and C; a trigger on B (or on E) that shows D; another, on E, that shows C and, in the "target" case, B.
                page.props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({animFadeIn(4, 500), animFadeIn(6, 500)}),
                    object ? QVector<QPair<int, QString>>{{5, animFadeIn(7, 500)}, {8, animFadeIn(7, 500)}}
                           : QVector<QPair<int, QString>>{{8, animFadeIn(5, 500)}, {8, animFadeIn(7, 500)}, {7, animFadeIn(5, 500) + animFadeIn(6, 500, "withEffect")}});
                QCOMPARE(int(deck::readAnimations(page, w.editor()->doc()).effects.size()), 2);
                const QStringList shapesBefore = shapeJson(w);
                const QString timingBefore = page.props.value(QStringLiteral("timingXml")).toString();
                if (merge) {
                    w.editor()->select(QStringList{m->id, b->id});
                    QTRY_VERIFY2(w.act(QStringLiteral("shape.union"))->isEnabled(), qPrintable(label));
                    w.act(QStringLiteral("shape.union"))->trigger();
                } else {
                    w.editor()->select(QStringList{b->id});
                    w.editor()->deleteSelection();
                }
                for (const ItemPtr &it : w.editor()->doc()->pages[0]->items) QVERIFY2(it->sourceId != 5, qPrintable(label));
                const QString file = dir.filePath(QStringLiteral("trigger.pptx"));
                QVERIFY2(w.saveTo(file), qPrintable(label));
                DeckWindow again;
                QTimer looker;   // (a box the open brings up is closed, so that a failure does not hang)
                looker.setInterval(20);
                connect(&looker, &QTimer::timeout, &looker, [] {
                    for (QWidget *top : QApplication::topLevelWidgets())
                        if (auto *box = qobject_cast<QMessageBox *>(top); box && box->isVisible()) box->accept();
                });
                looker.start();
                const bool opened = again.openFile(file);
                looker.stop();
                QVERIFY2(opened, qPrintable(label));
                again.editor()->setCurrentPage(0);
                const Page &back = *again.editor()->doc()->pages[0];
                QHash<QString, int> numbers;
                for (const ItemPtr &it : back.items) numbers.insert(it->name, it->sourceId);
                QSet<int> mainShapes;
                for (const deck::Anim &an : deck::readAnimations(back, again.editor()->doc()).effects) mainShapes.insert(an.shape);
                // The rule of a deleted or merged-away trigger object: its effects move to the end of the main sequence; an effect aimed at the missing shape itself is taken out.
                QSet<int> wantMain{numbers.value(QStringLiteral("M")), numbers.value(QStringLiteral("C"))};
                if (object) wantMain.insert(numbers.value(QStringLiteral("D")));
                QVERIFY2(mainShapes == wantMain, qPrintable(label + QLatin1String(": main sequence")));
                QHash<int, QSet<int>> triggers;
                for (const deck::Trigger &t : deck::timelineOf(back).triggers)
                    for (const deck::Effect &ef : t.effects)
                        for (const deck::Behavior &bh : ef.behaviors) triggers[t.shape].insert(bh.target.shape);
                const int nc = numbers.value(QStringLiteral("C")), nd = numbers.value(QStringLiteral("D")), ne = numbers.value(QStringLiteral("E"));
                const QHash<int, QSet<int>> want = object ? QHash<int, QSet<int>>{{ne, {nd}}} : QHash<int, QSet<int>>{{ne, {nd}}, {nd, {nc}}};
                QVERIFY2(triggers == want, qPrintable(label + QLatin1String(": triggers")));
                // The window's own document keeps what is aimed at the missing shape until the save (the effects of an object that went moved at once), and one Undo puts everything back.
                if (!object) QCOMPARE(w.editor()->doc()->pages[0]->props.value(QStringLiteral("timingXml")).toString(), timingBefore);
                else QVERIFY(w.editor()->doc()->pages[0]->props.value(QStringLiteral("timingXml")).toString() != timingBefore);
                w.editor()->undo();
                QCOMPARE(shapeJson(w), shapesBefore);
                QCOMPARE(w.editor()->doc()->pages[0]->props.value(QStringLiteral("timingXml")).toString(), timingBefore);
            }
    }

    // The commands are off where Merge Shapes would only refuse: two shapes of a group (the whole group is what it takes), and more shapes than it merges at once.
    void mergeShapesCommandsAreOffWhereTheyWouldRefuse()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
        QStringList ids;
        for (int i = 0; i < pathops::kMostShapes + 1; ++i) {
            ShapeItem *s = addShape(w, QStringLiteral("S%1").arg(i), QRectF(10 + 5 * i, 10 + 3 * i, 40, 40));
            s->shape = QStringLiteral("rect");
            ids << s->id;
        }
        auto allCommands = [&](bool on) {
            for (const char *id : jp::kMergeShapeIds) QCOMPARE(w.act(QString::fromLatin1(id))->isEnabled(), on);
        };
        w.editor()->select(ids.mid(0, pathops::kMostShapes));
        QTRY_VERIFY(w.act(QStringLiteral("shape.union"))->isEnabled());
        allCommands(true);   // (the most there is room for)
        w.editor()->select(ids);
        QTRY_VERIFY(!w.act(QStringLiteral("shape.union"))->isEnabled());
        allCommands(false);
        // Two shapes inside a group.
        w.editor()->select(ids.mid(0, 2));
        QTRY_VERIFY(w.act(QStringLiteral("shape.union"))->isEnabled());
        w.editor()->groupSelection();
        const GroupItem *group = nullptr;
        for (const ItemPtr &it : w.editor()->doc()->pages[0]->items)
            if (!group) group = dynamic_cast<const GroupItem *>(it.get());
        QVERIFY(group && group->children.size() == 2);
        w.editor()->select(QStringList{group->children[0]->id, group->children[1]->id});
        QCOMPARE(w.editor()->selection().size(), 2);
        QTRY_VERIFY(!w.act(QStringLiteral("shape.union"))->isEnabled());
        allCommands(false);
        // One shape inside the group and one outside it.
        w.editor()->select(QStringList{group->children[0]->id, ids.last()});
        QTRY_VERIFY(!w.act(QStringLiteral("shape.union"))->isEnabled());
        allCommands(false);
    }

    // Edit Points on one shape of a rotated group: a point dragged with the mouse lands where the pointer is let go.
    void editPointsOnAShapeInARotatedGroupFollowsTheMouse()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
        ShapeItem *a = square(w, QStringLiteral("A"), 100, 100, Qt::red, 4);
        ShapeItem *b = square(w, QStringLiteral("B"), 400, 200, Qt::blue, 5);
        w.editor()->select(QStringList{a->id, b->id});
        w.editor()->groupSelection();
        GroupItem *group = nullptr;
        for (const ItemPtr &it : w.editor()->doc()->pages[0]->items)
            if (!group) group = dynamic_cast<GroupItem *>(it.get());
        QVERIFY(group);
        group->rotateAround(30, group->bounds().center());
        a = static_cast<ShapeItem *>(group->children[0].get());
        w.editor()->select(QStringList{a->id});
        QTRY_VERIFY(w.act(QStringLiteral("shape.editPoints"))->isEnabled());
        w.act(QStringLiteral("shape.editPoints"))->trigger();
        QCOMPARE(w.editor()->pointsItem, a->id);
        Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        const int idx = pts::vertices(a->customPath).first();
        auto vertex = [&] { const auto e = a->customPath.elementAt(idx); return a->transform().map(QPointF(e.x, e.y)); };
        const QPointF before = vertex();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(before));
        QTest::mouseMove(vp, view(before + QPointF(20, 15)));
        QTest::mouseMove(vp, view(before + QPointF(40, 30)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(before + QPointF(40, 30)));
        const QPointF after = vertex();
        QVERIFY2(std::abs(after.x() - before.x() - 40) < 2 && std::abs(after.y() - before.y() - 30) < 2, qPrintable(QStringLiteral("moved by %1, %2").arg(after.x() - before.x()).arg(after.y() - before.y())));
        QTest::keyClick(vp, Qt::Key_Escape);
    }

    // A show (from the start, from this slide, or Presenter View) and Reading View end Edit Points first: the points are not left on a shape that is not on the screen.
    void startingAShowOrReadingViewEndsEditPoints()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
        ShapeItem *s = square(w, QStringLiteral("S"), 100, 100, Qt::red, 4);
        for (const char *route : {"show.fromStart", "show.fromCurrent", "show.presenterView", "view.reading"}) {
            w.editor()->select(QStringList{s->id});
            QTRY_VERIFY2(w.act(QStringLiteral("shape.editPoints"))->isEnabled(), route);
            if (w.editor()->pointsItem.isEmpty()) w.act(QStringLiteral("shape.editPoints"))->trigger();
            QCOMPARE(w.editor()->pointsItem, s->id);
            w.act(QString::fromLatin1(route))->trigger();
            QVERIFY2(w.editor()->pointsItem.isEmpty(), route);
            if (w.inReadingView()) {
                w.act(QStringLiteral("view.normal"))->trigger();
                QTRY_VERIFY2(!w.inReadingView(), route);
            } else if (SlideShow *show = w.slideShow()) {
                show->close();
                QTRY_VERIFY2(!w.slideShow(), route);
            }
        }
    }

    void aRingMadeBySubtractStaysAHoleAndEditPointsReshapesWithRealMouseEvents()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setDocument(Document::blank(QSizeF(960, 540)));
        ShapeItem *outer = square(w, QStringLiteral("Outer"), 100, 100, Qt::red, 4);
        ShapeItem *inner = addShape(w, QStringLiteral("Inner"), QRectF(150, 150, 100, 100));
        inner->shape = QStringLiteral("rect");
        w.editor()->select(QStringList{outer->id, inner->id});
        w.act(QStringLiteral("shape.subtract"))->trigger();
        QCOMPARE(int(w.editor()->doc()->pages[0]->items.size()), 1);
        QVERIFY(filledAt(w, 120, 120));
        QVERIFY(!filledAt(w, 200, 200));   // the hole stays a hole
        auto *s = static_cast<ShapeItem *>(w.editor()->doc()->pages[0]->items[0].get());
        QCOMPARE(w.editor()->selectedItems().size(), 1);
        // Edit Points: the command turns it on for the one shape, the corner is dragged, Esc ends it.
        QTRY_VERIFY(w.act(QStringLiteral("shape.editPoints"))->isEnabled());
        w.act(QStringLiteral("shape.editPoints"))->trigger();
        QCOMPARE(w.editor()->pointsItem, s->id);
        Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        const QRectF before = s->transform().map(s->customPath).boundingRect();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(before.bottomRight()));
        QTest::mouseMove(vp, view(before.bottomRight() + QPointF(20, 15)));
        QTest::mouseMove(vp, view(before.bottomRight() + QPointF(40, 30)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(before.bottomRight() + QPointF(40, 30)));
        const QRectF after = s->transform().map(s->customPath).boundingRect();
        QVERIFY2(std::abs(after.right() - before.right() - 40) < 2 && std::abs(after.bottom() - before.bottom() - 30) < 2, "the corner did not follow the mouse");
        QVERIFY(!filledAt(w, 200, 200));   // the hole survives the drag
        QTest::keyClick(vp, Qt::Key_Escape);
        QVERIFY(w.editor()->pointsItem.isEmpty());
        w.editor()->undo();
        s = static_cast<ShapeItem *>(w.editor()->doc()->pages[0]->items[0].get());   // (an undo may put back another object)
        QCOMPARE(s->transform().map(s->customPath).boundingRect(), before);
    }
};

JP_TEST_CLASS(DeckLines11Tests)
#include "tst_deck_lines11.moc"
