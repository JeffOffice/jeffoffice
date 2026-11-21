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
#include <QCheckBox>
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
};

JP_TEST_CLASS(DeckLines11Tests)
#include "tst_deck_lines11.moc"
