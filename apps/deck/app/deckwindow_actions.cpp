// JeffDeck's commands for the Home, Insert, Design, and View tabs: each calls
// the same Editor function the matching command of the other apps does.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/editor.h"
#include "app/pictureask.h"
#include "app/filedialogs.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/pagenumbergallery.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/textdialogs.h"
#include "canvas/canvas.h"
#include "core/deck.h"
#include "text/textprops.h"

#include <QAction>
#include <climits>
#include <QActionGroup>
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>
#include "app/plaintext.h"
#include "io/presentationdefaults.h"

namespace jp {

namespace {
// The text boxes and shapes a text box command applies to: the one being
// typed in, else the ones selected.
QVector<Item *> textTargets(Editor *ed)
{
    QVector<Item *> out;
    if (ed->isEditingText()) {
        if (Item *it = ed->doc()->item(ed->textTarget().itemId)) out << it;
        return out;
    }
    for (Item *it : ed->selectedItems())
        if (it->type() == ItemType::Text || it->type() == ItemType::Shape) out << it;
    return out;
}

// Whether the paragraph with the text cursor is a bulleted one, or a numbered one.
bool inList(Editor *ed, bool bullets)
{
    if (!ed->isEditingText() || !ed->cursor().block().textList()) return false;
    return isBulletList(ed->cursor().block().textList()->format().style()) == bullets;
}

// Increase and Decrease List Level: in a slide's placeholder whose layout has a list, a paragraph takes the layout's list, indent, and look for
// its new level (one step that undoes); anywhere else the editor changes the level.
void changeLevel(Editor *ed, int dir)
{
    Document *d = ed->doc();
    struct Target {
        const Item *item;
        int first, last;
    };
    QVector<Target> targets;
    bool placeholders = d != nullptr;
    auto add = [&](const Item *it, int first, int last) {
        if (!it || !deck::layoutHasList(*d, *it)) placeholders = false;
        else targets << Target{it, first, last};
    };
    if (placeholders && ed->isEditingText()) {
        const Editor::TextTarget &t = ed->textTarget();
        const QTextDocument *sd = d->storyDoc(t.storyId);
        if (t.row >= 0 || !sd) placeholders = false;
        else add(d->item(t.itemId), sd->findBlock(ed->cursor().selectionStart()).blockNumber(), sd->findBlock(ed->cursor().selectionEnd()).blockNumber());
    } else if (placeholders) {
        for (const Item *it : ed->selectedItems()) add(it, 0, INT_MAX);
        placeholders = placeholders && !targets.isEmpty();
    }
    if (!placeholders) {
        ed->changeIndent(dir);
        return;
    }
    ed->beginChange(dir > 0 ? QCoreApplication::translate("DeckWindow", "Increase List Level") : QCoreApplication::translate("DeckWindow", "Decrease List Level"));
    for (const Target &t : std::as_const(targets)) deck::changeListLevel(*d, *t.item, t.first, t.last, dir);
    ed->endChange();
    Q_EMIT ed->textCursorChanged();
}
} // namespace

void DeckWindow::createFormatActions()
{
    Editor *ed = m_ed;
    mk("edit.formatPainter", tr("Copy Formatting"), "paintbrush", QKeySequence(), [ed] {
        if (ed->tool() == Tool::FormatPainter) {
            ed->setTool(Tool::Select);
            return;
        }
        ed->copyFormatting();
        ed->painterLocked = QApplication::keyboardModifiers() & Qt::ShiftModifier;
        ed->setTool(Tool::FormatPainter);
    }, true);

    // ---------------- Font ----------------
    mk("fmt.shadow", tr("Shadow"), "layers", QKeySequence(), [ed] {
        // Each run gets or loses the shadow by itself, in its own effect list, so that it is saved and the run's other effects stay.
        ed->setTextShadow(!Editor::hasTextShadow(ed->currentCharFormat()), tr("Text Shadow"));
    }, true);
    mk("fmt.grow", tr("Increase Font Size"), "a-arrow-up", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Greater), [ed] { ed->growFont(1); });
    mk("fmt.shrink", tr("Decrease Font Size"), "a-arrow-down", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Less), [ed] { ed->growFont(-1); });
    mk("fmt.clear", tr("Clear All Formatting"), "remove-formatting", QKeySequence(Qt::CTRL | Qt::Key_Space), [ed] { ed->clearFormatting(); });
    const QString caseNames[] = {tr("Sentence case."), tr("lowercase"), tr("UPPERCASE"), tr("Capitalize Each Word"), tr("tOGGLE cASE")};
    for (int i = 0; i < 5; ++i) mk(QStringLiteral("case.%1").arg(i), caseNames[i], "", QKeySequence(), [ed, i] { ed->changeCase(i); });
    // Tracking presets, as percentages of normal letter spacing.
    struct Spacing { const char *id; QString name; double pct; };
    const Spacing spacing[] = {{"Very Tight", tr("Very Tight"), 75}, {"Tight", tr("Tight"), 87.5}, {"Normal", tr("Normal"), 100}, {"Loose", tr("Loose"), 112.5},
                               {"Very Loose", tr("Very Loose"), 125}};
    for (const auto &s : spacing) {
        const double v = s.pct;
        mk(QStringLiteral("spacing.%1").arg(QString::fromLatin1(s.id)), s.name, "", QKeySequence(), [ed, v] {
            QTextCharFormat f;
            f.setProperty(tp::Tracking, v);
            ed->mergeCharFormat(f, tr("Character Spacing"));
        });
    }

    // ---------------- Paragraph ----------------
    mk("para.bullets", tr("Bullets"), "list", QKeySequence(), [ed] { ed->setList(inList(ed, true) ? 0 : 1); }, true);
    mk("para.numbers", tr("Numbering"), "list-ordered", QKeySequence(), [ed] { ed->setList(inList(ed, false) ? 0 : 2, 1); }, true);
    ed->changeListLevelHook = [ed](int dir) { changeLevel(ed, dir); };   // (Tab and Shift+Tab at the start of a list item)
    mk("para.indentDec", tr("Decrease List Level"), "indent-decrease", QKeySequence(), [ed] { changeLevel(ed, -1); });
    mk("para.indentInc", tr("Increase List Level"), "indent-increase", QKeySequence(), [ed] { changeLevel(ed, 1); });
    for (double s : {1.0, 1.15, 1.5, 2.0, 2.5, 3.0})
        mk(QStringLiteral("ls.%1").arg(s), QString::number(s), "", QKeySequence(), [ed, s] { ed->setLineSpacing(QTextBlockFormat::ProportionalHeight, s * 100); });

    // Text Direction, Align Text, and Columns work on the whole box.
    for (const bool vertical : {false, true})
        mk(vertical ? "tb.dirVertical" : "tb.dirHorizontal", vertical ? tr("Rotate All Text 90°") : tr("Horizontal"), vertical ? "arrow-down-wide-narrow" : "text", QKeySequence(),
           [ed, vertical] {
               const QVector<Item *> boxes = textTargets(ed);
               ed->change(tr("Text Direction"), [&] {
                   for (Item *it : boxes)
                       if (it->type() == ItemType::Text) static_cast<TextItem *>(it)->vertical = vertical;
               });
           }, true);
    const QString alignNames[] = {tr("Top"), tr("Middle"), tr("Bottom")};
    for (int k = 0; k < 3; ++k)
        mk(QStringLiteral("valign.%1").arg(k), alignNames[k], k == 0 ? "align-vertical-justify-start" : k == 1 ? "align-vertical-justify-center" : "align-vertical-justify-end",
           QKeySequence(), [ed, k] {
               const QVector<Item *> boxes = textTargets(ed);
               ed->change(tr("Align Text"), [&] {
                   for (Item *it : boxes) {
                       if (it->type() == ItemType::Text) static_cast<TextItem *>(it)->valign = VAlign(k);
                       else if (it->type() == ItemType::Shape) static_cast<ShapeItem *>(it)->valign = VAlign(k);
                   }
               });
           }, true);
    for (int c = 1; c <= 3; ++c)
        mk(QStringLiteral("cols.%1").arg(c), c == 1 ? tr("One Column") : tr("%1 Columns").arg(c), c == 1 ? "square" : c == 2 ? "columns-2" : "columns-3", QKeySequence(),
           [ed, c] {
               const QVector<Item *> boxes = textTargets(ed);
               ed->change(tr("Columns"), [&] {
                   for (Item *it : boxes)
                       if (it->type() == ItemType::Text) static_cast<TextItem *>(it)->columns = c;
               });
           }, true);

    // ---------------- Drawing ----------------
    mk("arr.front", tr("Bring to Front"), "bring-to-front", QKeySequence(Qt::ALT | Qt::Key_F6), [ed] { ed->arrange(Editor::Order::Front); });
    mk("arr.forward", tr("Bring Forward"), "arrow-up-from-line", QKeySequence(), [ed] { ed->arrange(Editor::Order::Forward); });
    mk("arr.backward", tr("Send Backward"), "arrow-down-to-line", QKeySequence(), [ed] { ed->arrange(Editor::Order::Backward); });
    mk("arr.back", tr("Send to Back"), "send-to-back", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_F6), [ed] { ed->arrange(Editor::Order::Back); });
    // (the slide has no margins, so "relative to the slide" is the margin rectangle of the shared editor)
    mk("arr.alignToSlide", tr("Align to Slide"), "", QKeySequence(), [this] { refreshUi(); }, true);
    auto toSlide = [this] { return act("arr.alignToSlide")->isChecked(); };
    mk("arr.alignLeft", tr("Align Left"), "align-start-vertical", QKeySequence(), [ed, toSlide] { ed->align(Editor::Align::Left, toSlide()); });
    mk("arr.alignCenter", tr("Align Center"), "align-center-vertical", QKeySequence(), [ed, toSlide] { ed->align(Editor::Align::Center, toSlide()); });
    mk("arr.alignRight", tr("Align Right"), "align-end-vertical", QKeySequence(), [ed, toSlide] { ed->align(Editor::Align::Right, toSlide()); });
    mk("arr.alignTop", tr("Align Top"), "align-start-horizontal", QKeySequence(), [ed, toSlide] { ed->align(Editor::Align::Top, toSlide()); });
    mk("arr.alignMiddle", tr("Align Middle"), "align-center-horizontal", QKeySequence(), [ed, toSlide] { ed->align(Editor::Align::Middle, toSlide()); });
    mk("arr.alignBottom", tr("Align Bottom"), "align-end-horizontal", QKeySequence(), [ed, toSlide] { ed->align(Editor::Align::Bottom, toSlide()); });
    mk("arr.distH", tr("Distribute Horizontally"), "align-horizontal-distribute-center", QKeySequence(), [ed, toSlide] { ed->distribute(true, toSlide()); });
    mk("arr.distV", tr("Distribute Vertically"), "align-vertical-distribute-center", QKeySequence(), [ed, toSlide] { ed->distribute(false, toSlide()); });
    mk("arr.selectionPane", tr("Selection Pane"), "layers", QKeySequence(), [this] { showPageSelectionPane(this, m_ed, &m_selectionPane); });
    mk("arr.group", tr("Group"), "group", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G), [ed] { ed->groupSelection(); });
    mk("arr.ungroup", tr("Ungroup"), "ungroup", QKeySequence(), [ed] { ed->ungroupSelection(); });
    mk("arr.rotR", tr("Rotate Right 90°"), "rotate-cw", QKeySequence(), [ed] { ed->rotateSelection(90); });
    mk("arr.rotL", tr("Rotate Left 90°"), "rotate-ccw", QKeySequence(), [ed] { ed->rotateSelection(-90); });
    mk("arr.flipH", tr("Flip Horizontal"), "flip-horizontal-2", QKeySequence(), [ed] { ed->flipSelection(true); });
    mk("arr.flipV", tr("Flip Vertical"), "flip-vertical-2", QKeySequence(), [ed] { ed->flipSelection(false); });
    mk("edit.selectAll", tr("Select All"), "text-select", QKeySequence::SelectAll, [ed] {
        if (ed->isEditingText()) {
            QTextCursor c = ed->cursor();
            c.select(QTextCursor::Document);
            ed->setCursor(c);
            return;
        }
        QStringList ids;
        for (const auto &it : ed->surfaceItems())
            if (!it->hidden) ids << it->id;   // (a hidden object is reached through the Selection Pane)
        ed->select(ids);
    });

    // Types in the selected box, shape, or table; asks for the words of a Text Art.
    mk("edit.text", tr("Edit Text"), "text-cursor", QKeySequence(), [this, ed] {
        Item *it = ed->single();
        if (!it) return;
        if (it->type() == ItemType::TextArt) textArtTextDialog(this, ed, it->id);
        else if (it->type() == ItemType::Text || it->type() == ItemType::Shape || it->type() == ItemType::Table) ed->beginTextEdit(it->id);
    });
}

void DeckWindow::createInsertActions()
{
    mk("insert.headerFooter", tr("Header and Footer"), "panel-bottom", QKeySequence(), [this] { showHeaderFooter(); });
    // Slide Number: on or off, the place it goes, and not on the title slide.
    mk("insert.slideNumberOn", tr("Show Slide Number"), "", QKeySequence(), [this] { setSlideNumberOn(act("insert.slideNumberOn")->isChecked()); }, true);
    auto *places = new QActionGroup(this);
    struct Spot { const char *id; QString text; const char *design; deck::SlideNumberPlace place; };
    const Spot spots[] = {
        {"insert.slideNumberPlace.topLeft", tr("Top Left"), "top.plain.left", {deck::NumberAcross::Left, deck::NumberDown::Top}},
        {"insert.slideNumberPlace.topCenter", tr("Top Center"), "top.plain.center", {deck::NumberAcross::Center, deck::NumberDown::Top}},
        {"insert.slideNumberPlace.topRight", tr("Top Right"), "top.plain.right", {deck::NumberAcross::Right, deck::NumberDown::Top}},
        {"insert.slideNumberPlace.bottomLeft", tr("Bottom Left"), "bottom.plain.left", {deck::NumberAcross::Left, deck::NumberDown::Bottom}},
        {"insert.slideNumberPlace.bottomCenter", tr("Bottom Center"), "bottom.plain.center", {deck::NumberAcross::Center, deck::NumberDown::Bottom}},
        {"insert.slideNumberPlace.bottomRight", tr("Bottom Right"), "bottom.plain.right", {deck::NumberAcross::Right, deck::NumberDown::Bottom}},
    };
    for (const Spot &s : spots) {
        const int across = int(s.place.across), down = int(s.place.down);
        QAction *a = mk(s.id, s.text, "", QKeySequence(), [this, across, down] { moveSlideNumber(across, down); }, true);
        places->addAction(a);
        // The picture is the shared gallery's design for that place: a small page with the number on it.
        if (const auto design = pageNumberDesignById(QString::fromLatin1(s.design))) {
            QPixmap pm(40, 30);
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            paintPageNumberDesign(&p, QRectF(0, 0, 40, 30), *design, palette().color(QPalette::Highlight));
            p.end();
            a->setIcon(QIcon(pm));
        }
    }
    mk("insert.slideNumberSkipTitle", tr("Don't Show on Title Slide"), "", QKeySequence(), [this] { setSlideNumberSkipsTitle(act("insert.slideNumberSkipTitle")->isChecked()); }, true);
    createPictureActions();
}

void DeckWindow::createDesignActions()
{
    // One size at a time.
    auto *sizes = new QActionGroup(this);
    sizes->addAction(mk("design.sizeStandard", tr("Standard (4:3)"), "", QKeySequence(), [this] { askSlideSize(QSizeF(deck::kStandardWidth, deck::kStandardHeight)); }, true));
    sizes->addAction(mk("design.sizeWidescreen", tr("Widescreen (16:9)"), "", QKeySequence(), [this] { askSlideSize(QSizeF(deck::kSlideWidth, deck::kSlideHeight)); }, true));
    for (const deck::SlideSizePreset &p : deck::slideSizePresets()) {
        if (p.key == QLatin1String("standard") || p.key == QLatin1String("widescreen")) continue;
        const QString text = p.key == QLatin1String("screen16x10") ? tr("On-screen Show (16:10)") : p.key == QLatin1String("letter") ? tr("Letter Paper") : p.key == QLatin1String("ledger") ? tr("Ledger Paper")
                           : p.key == QLatin1String("a3") ? tr("A3 Paper") : p.key == QLatin1String("a4") ? tr("A4 Paper") : p.key == QLatin1String("b4") ? tr("B4 (ISO) Paper")
                           : p.key == QLatin1String("b5") ? tr("B5 (ISO) Paper") : p.key == QLatin1String("slides35mm") ? tr("35mm Slides") : tr("Banner");
        const QSizeF size = p.size;
        sizes->addAction(mk(("design.size." + p.key).toLatin1().constData(), text, "", QKeySequence(), [this, size] { askSlideSize(size); }, true));
    }
    mk("design.sizeCustom", tr("Custom Slide Size…"), "", QKeySequence(), [this] {
        SlideSizeDialog dlg(m_ed->doc()->pageSize(), deck::notesArePortrait(*m_ed->doc()), this);
        if (dlg.exec() == QDialog::Accepted) askSlideSize(dlg.size(), dlg.notesPortrait() ? 1 : 0);
    });
    mk("design.slideSize", tr("Slide Size"), "ratio", QKeySequence(), [] {});
    mk("design.setDefaultSlideSize", tr("Set as Default Slide Size"), "", QKeySequence(), [this] {
        deck::setDefaultSlideSize(m_ed->doc()->pageSize());
        statusBar()->showMessage(tr("New presentations will start at this slide size."), 5000);
    });
    mk("design.setDefaultTheme", tr("Set as Default Theme"), "palette", QKeySequence(), [this] {
        QString problem;
        if (deck::setDefaultTheme(*m_ed->doc(), &problem)) statusBar()->showMessage(tr("New presentations will start with this theme."), 5000);
        else jp::plainWarning(this, tr("Set as Default Theme"), problem);
    });
    mk("design.formatBackground", tr("Format Background"), "paint-roller", QKeySequence(), [this] { showFormatBackground(); });
}

void DeckWindow::createViewActions()
{
    // View > Window: another window on the same file (as in JeffWrite: the file is opened again, so a change is seen in the other window once it is saved and the file opened there
    // again); a new, empty presentation when this one has no file yet.
    mk("win.new", tr("New Window"), "app-window", QKeySequence(), [this] {
        if (m_ed->filePath().isEmpty()) {
            auto *w = new DeckWindow();
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
            return;
        }
        openFile(m_ed->filePath(), OpenHow::NewWindow);
    });
    mk("view.slideShow", tr("Slide Show"), "presentation", QKeySequence(), [this] { startShow(m_ed->currentPage()); });
    mk("view.rulers", tr("Ruler"), "ruler", QKeySequence(), [this] {
        m_ed->view.rulers = !m_ed->view.rulers;
        m_canvas->setRulersVisible(m_ed->view.rulers);
        refreshUi();
    }, true);
    mk("view.gridlines", tr("Gridlines"), "grid-2x2", QKeySequence(Qt::SHIFT | Qt::Key_F9), [this] {
        m_canvas->setGridlines(!m_canvas->gridlines());
        refreshUi();
    }, true);
    mk("view.guides", tr("Guides"), "layout-grid", QKeySequence(Qt::ALT | Qt::Key_F9), [this] {
        m_canvas->setGuides(!m_canvas->guides());
        refreshUi();
    }, true);
    mk("zoom.in", tr("Zoom In"), "zoom-in", QKeySequence::ZoomIn, [this] {
        m_canvas->zoomToFit(Canvas::Fit::None);
        m_canvas->setZoom(m_canvas->zoom() * 1.2);
    });
    mk("view.fullScreen", tr("Full Screen"), "maximize", QKeySequence(Qt::Key_F11), [this] { toggleFullScreen(); }, true);
    mk("view.collapseRibbon", tr("Collapse the Ribbon"), "chevron-up", QKeySequence(Qt::CTRL | Qt::Key_F1), [this] { m_ribbon->setMinimized(!m_ribbon->isMinimized()); });
    mk("zoom.out", tr("Zoom Out"), "zoom-out", QKeySequence::ZoomOut, [this] {
        m_canvas->zoomToFit(Canvas::Fit::None);
        m_canvas->setZoom(m_canvas->zoom() / 1.2);
    });
    for (int pct : {400, 300, 200, 150, 100, 75, 66, 50, 33, 25, 10})
        mk(QStringLiteral("zoom.%1").arg(pct), QStringLiteral("%1%").arg(pct), "", QKeySequence(), [this, pct] {
            m_canvas->zoomToFit(Canvas::Fit::None);
            m_canvas->setZoom(pct / 100.0);
        });
}

// ---------------- what the commands do ----------------
void DeckWindow::setSlideSize(const QSizeF &size, bool scale, int notes)
{
    const bool notesChange = notes >= 0 && deck::notesArePortrait(*m_ed->doc()) != (notes == 1);
    if (size.isEmpty() || (size == m_ed->doc()->pageSize() && !notesChange)) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    const OwnEdit own(this);
    m_ed->beginChange(tr("Slide Size"));
    if (size != m_ed->doc()->pageSize()) deck::setSlideSize(*m_ed->doc(), size, scale);
    if (notesChange) deck::setNotesPortrait(*m_ed->doc(), notes == 1);
    m_ed->endChange();
}

// A new size asks whether the content scales to it or keeps its own size.
void DeckWindow::askSlideSize(const QSizeF &size, int notes)
{
    if (size == m_ed->doc()->pageSize()) {
        setSlideSize(size, true, notes);   // (only the notes pages can change)
        refreshUi();   // the check on the chosen size stays
        return;
    }
    jp::MessageBox box(QMessageBox::Question, tr("Slide Size"), tr("Scale your content to fit the new slide size?"), QMessageBox::NoButton, this);
    box.setObjectName(QStringLiteral("slideSizeAsk"));
    box.setTextFormat(Qt::PlainText);
    box.setInformativeText(tr("Scale to Fit shrinks your content when it has to, so all of it stays on the slide. Keep Size leaves every object as it is."));
    QPushButton *fit = box.addButton(tr("Scale to Fit"), QMessageBox::AcceptRole);
    QPushButton *keep = box.addButton(tr("Keep Size"), QMessageBox::DestructiveRole);
    box.addButton(QMessageBox::Cancel);
    box.setDefaultButton(fit);
    box.exec();
    if (box.clickedButton() == fit) setSlideSize(size, true, notes);
    else if (box.clickedButton() == keep) setSlideSize(size, false, notes);
    else refreshUi();
}

void DeckWindow::setBackground(const QColor &color, bool all)
{
    if (color.isValid()) setBackground(Fill::solid(ColorRef::rgb(color)), all);
}

void DeckWindow::setBackground(const Fill &fill, bool all)
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    // In Slide Master view the page is the slide master or layout being edited.
    MasterPage *open = m_view == View::Master ? d->master(m_ed->masterView()) : nullptr;
    if (!all && !open && (i < 0 || i >= d->pages.size())) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Format Background"));
    if (all) deck::setAllBackgrounds(*d, fill);
    else if (open) open->background = fill;
    else
        for (int slide : selectedSlides())
            if (slide >= 0 && slide < d->pages.size()) deck::setSlideBackground(*d->pages[slide], fill);
    m_ed->endChange();
}

void DeckWindow::resetBackground(bool all)
{
    Document *d = m_ed->doc();
    MasterPage *open = m_view == View::Master ? d->master(m_ed->masterView()) : nullptr;
    m_ed->endTextEdit();
    if (open && !all) {
        // A layout follows its slide master again; the slide master itself has nothing to follow.
        if (!deck::isLayout(*open)) return;
        m_ed->beginChange(tr("Reset Background"));
        open->background = Fill();
        m_ed->endChange();
        return;
    }
    m_ed->beginChange(tr("Reset Background"));
    if (all) deck::resetAllBackgrounds(*d);
    else
        for (int slide : selectedSlides())
            if (slide >= 0 && slide < d->pages.size()) deck::resetSlideBackground(*d->pages[slide]);
    m_ed->endChange();
}

// The picture a picture background is made of: asked for, read, and kept in the presentation. Empty when none is chosen or it can't be read.
QString DeckWindow::chooseBackgroundPicture()
{
    const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    const QString filter = tr("All Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg *.wmf *.emf)") + QStringLiteral(";;") + tr("All Files (*)");
    const QStringList paths = askPicturePaths(this, tr("Insert Picture"), dir, filter, false, nullptr);
    if (paths.isEmpty()) return QString();
    Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
    QByteArray bytes = readPictureBytes(paths.first());
    QString kind;
    QSize pixels;
    bool asksNotToBeCopied = false;
    if (bytes.isEmpty() || !readPictureFileAsking(this, paths.first(), 0, &bytes, &kind, &pixels, nullptr, &asksNotToBeCopied)) {
        statusBar()->showMessage(tr("%1 can't be read as a picture.").arg(QFileInfo(paths.first()).fileName()), 8000);
        return QString();
    }
    return m_ed->doc()->addImage(bytes, kind, paths.first());
}

void DeckWindow::showFormatBackground()
{
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    const MasterPage *open = m_view == View::Master ? d->master(m_ed->masterView()) : nullptr;
    if (!open && (i < 0 || i >= d->pages.size())) return;
    // The fill the page shows now: its own, else its layout's, else the master's.
    Fill fill = open ? Fill() : d->pages[i]->background;
    const QVector<MasterPage *> chain = d->masterChain(open ? open : d->masterFor(*d->pages[i]));
    for (int k = int(chain.size()) - 1; k >= 0 && fill.type == Fill::NoFill; --k) fill = chain[k]->background;
    if (fill.type == Fill::NoFill) fill = Fill::solid(ColorRef::rgb(Qt::white));
    BackgroundDialog dlg(fill, d->colors, [this](const QString &id) { return m_ed->doc()->image(id); }, [this] { return chooseBackgroundPicture(); }, this);
    const bool hidden = open ? !open->showMasterItems : !d->pages[i]->showMasterItems;
    dlg.setHideGraphics(hidden);
    if (dlg.exec() != QDialog::Accepted) return;
    // The fill and the box are one step.
    m_ed->beginChange(tr("Format Background"));
    if (dlg.resetRequested()) resetBackground(false);
    else setBackground(dlg.fill(), dlg.applyToAll());
    if (dlg.hideGraphics() != hidden) hideBackgroundGraphics(dlg.hideGraphics());
    m_ed->endChange();
}

void DeckWindow::applyFooters(const deck::FooterOptions &options, bool all)
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (!all && (i < 0 || i >= d->pages.size())) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Header & Footer"));
    deck::applyFooters(*d, options, all ? nullptr : d->pages[i].get());
    m_ed->endChange();
}

// Insert > Slide Number: the number on or off on every slide (the options of Header & Footer, with the date and footer as they were).
void DeckWindow::setSlideNumberOn(bool on)
{
    deck::FooterOptions o = deck::footerOptions(*m_ed->doc());
    if (o.slideNumber == on) return;
    o.slideNumber = on;
    changeFooters(o, tr("Slide Number"));
}

void DeckWindow::setSlideNumberSkipsTitle(bool skip)
{
    deck::FooterOptions o = deck::footerOptions(*m_ed->doc());
    if (o.skipTitleSlide == skip) return;
    o.skipTitleSlide = skip;
    changeFooters(o, tr("Slide Number"));
}

void DeckWindow::changeFooters(const deck::FooterOptions &o, const QString &step)
{
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(step);
    deck::applyFooters(*m_ed->doc(), o);
    m_ed->endChange();
}

// The place of the slide number: the placeholder of the master, the layouts, and the slides moves, as one step.
void DeckWindow::moveSlideNumber(int across, int down)
{
    const deck::SlideNumberPlace place{deck::NumberAcross(across), deck::NumberDown(down)};
    if (deck::slideNumberPlace(*m_ed->doc()) == place) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Slide Number"));
    deck::setSlideNumberPlace(*m_ed->doc(), place);
    m_ed->endChange();
}

void DeckWindow::showHeaderFooter()
{
    HeaderFooterDialog dlg(deck::footerOptions(*m_ed->doc()), this);
    if (dlg.exec() == QDialog::Accepted) applyFooters(dlg.options(), dlg.applyToAll());
}

void DeckWindow::showPrintHeaderFooter()
{
    Document *d = m_ed->doc();
    PrintHeaderFooterDialog dlg(deck::printHeaderFooter(*d, deck::hasPrintHeaderFooter(*d, deck::PrintedPages::Notes) ? deck::PrintedPages::Notes : deck::PrintedPages::Handouts), this);
    if (dlg.exec() != QDialog::Accepted) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Header & Footer"));
    for (deck::PrintedPages pages : {deck::PrintedPages::Notes, deck::PrintedPages::Handouts}) deck::setPrintHeaderFooter(*d, pages, dlg.choice());
    m_ed->endChange();
}

bool DeckWindow::insertPicture(const QString &path, const QPointF &at, const QString &altText, const QString &credit)
{
    QByteArray bytes = readPictureBytes(path);
    QString format;
    QSize pixels;
    bool asksNotToBeCopied = false;
    if (bytes.isEmpty() || !readPictureFileAsking(this, path, 0, &bytes, &format, &pixels, nullptr, &asksNotToBeCopied)) return false;
    if (asksNotToBeCopied) statusBar()->showMessage(tr("%1 asks not to be copied. It is placed all the same.").arg(QFileInfo(path).fileName()), 8000);
    Document *d = m_ed->doc();
    const QSizeF ps = d->pageSize();
    // Its pixels at 96 to the inch, no larger than most of the slide.
    QSizeF size(pixels.width() * 0.75, pixels.height() * 0.75);
    if (size.isEmpty()) size = QSizeF(144, 144);
    const QSizeF most = ps * 0.8;
    if (size.width() > most.width() || size.height() > most.height()) size.scale(most, Qt::KeepAspectRatio);
    auto pic = std::make_shared<PictureItem>();
    pic->rect = QRectF(at.x() < 0 ? QPointF((ps.width() - size.width()) / 2, (ps.height() - size.height()) / 2) : at, size);
    pic->imgRect = QRectF(QPointF(0, 0), size);
    pic->altText = altText;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Insert Picture"));
    pic->imageId = d->addImage(bytes, format, path);
    m_ed->surfaceItems().push_back(pic);
    if (!credit.isEmpty()) {
        // Under the picture, as wide as it is (but wide enough to read), kept on the slide.
        const double width = std::min(ps.width() * 0.9, std::max(pic->rect.width(), 320.0));
        const double left = std::clamp(pic->rect.center().x() - width / 2, 0.0, ps.width() - width);
        auto line = std::static_pointer_cast<TextItem>(m_ed->newTextBox(QRectF(left, pic->rect.bottom() + 4, width, 28), credit));
        QTextCursor c(d->storyDoc(line->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat creditText;
        creditText.setFontPointSize(12);
        c.mergeCharFormat(creditText);
        QTextBlockFormat centered;
        centered.setAlignment(Qt::AlignHCenter);
        c.mergeBlockFormat(centered);
        m_ed->surfaceItems().push_back(line);
        pic->caption = line->id;
    }
    m_ed->endChange();
    m_ed->select(pic->id);
    return true;
}

void DeckWindow::insertPictureFromFile()
{
    const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    const QString filter = tr("All Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg *.wmf *.emf *.ico *.pdf)") + QStringLiteral(";;") + tr("All Files (*)");
    const QStringList paths = askPicturePaths(this, tr("Insert Picture"), dir, filter, true, nullptr);
    if (paths.isEmpty()) return;
    Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
    for (const QString &path : paths)
        if (!insertPicture(path)) jp::plainWarning(this, tr("Insert Picture"), tr("%1 can't be read as a picture.").arg(QFileInfo(path).fileName()));
}

void DeckWindow::insertTable(int rows, int columns)
{
    rows = std::clamp(rows, 1, 100);
    columns = std::clamp(columns, 1, 30);
    const QSizeF ps = m_ed->doc()->pageSize();
    const double w = ps.width() * 0.8, h = std::min(ps.height() * 0.8, 36.0 * rows);
    auto table = m_ed->newTable(QRectF((ps.width() - w) / 2, (ps.height() - h) / 2, w, h), rows, columns, QStringLiteral("None"));
    // Text that reads on a slide.
    for (const TableCell &cell : static_cast<TableItem *>(table.get())->cells) {
        QTextCursor c(m_ed->doc()->storyDoc(cell.storyId));
        QTextCharFormat f;
        f.setFontPointSize(18);
        c.mergeBlockCharFormat(f);
    }
    styleNewTable(static_cast<TableItem *>(table.get()));
    m_ed->endTextEdit();
    m_ed->addItem(table);
}

} // namespace jp
