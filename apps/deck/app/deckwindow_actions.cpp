// JeffDeck's commands for the Home, Insert, Design, and View tabs: each calls
// the same Editor function the matching command of the other apps does.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/editor.h"
#include "app/filedialogs.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
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
#include <QPushButton>
#include <QStandardPaths>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>

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
    mk("edit.formatPainter", tr("Format Painter"), "paintbrush", QKeySequence(), [ed] {
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
    for (const auto &[id, ch] : {std::pair{"bullet.disc", "•"}, {"bullet.circle", "◦"}, {"bullet.square", "▪"}, {"bullet.diamond", "❖"}, {"bullet.arrow", "➢"},
                                 {"bullet.check", "✔"}, {"bullet.star", "★"}, {"bullet.dash", "–"}}) {
        const QString c = QString::fromUtf8(ch);
        mk(id, c, "", QKeySequence(), [ed, c] { ed->setList(1, 0, c); });
    }
    const char *numNames[] = {"1. 2. 3.", "a. b. c.", "A. B. C.", "i. ii. iii.", "I. II. III.", "1) 2) 3)", "(1) (2) (3)"};
    for (int i = 0; i < 7; ++i) mk(QStringLiteral("number.%1").arg(i), QString::fromLatin1(numNames[i]), "", QKeySequence(), [ed, i] { ed->setList(2, i + 1); });
    mk("para.listNone", tr("None"), "", QKeySequence(), [ed] { ed->setList(0); });
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
        for (const auto &it : ed->surfaceItems()) ids << it->id;
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
    createPictureActions();
}

void DeckWindow::createDesignActions()
{
    // One size at a time.
    auto *sizes = new QActionGroup(this);
    sizes->addAction(mk("design.sizeStandard", tr("Standard (4:3)"), "", QKeySequence(), [this] { askSlideSize(QSizeF(deck::kStandardWidth, deck::kStandardHeight)); }, true));
    sizes->addAction(mk("design.sizeWidescreen", tr("Widescreen (16:9)"), "", QKeySequence(), [this] { askSlideSize(QSizeF(deck::kSlideWidth, deck::kSlideHeight)); }, true));
    mk("design.slideSize", tr("Slide Size"), "ratio", QKeySequence(), [] {});
    mk("design.formatBackground", tr("Format Background"), "paint-roller", QKeySequence(), [this] { showFormatBackground(); });
}

void DeckWindow::createViewActions()
{
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
void DeckWindow::setSlideSize(const QSizeF &size)
{
    if (size.isEmpty() || size == m_ed->doc()->pageSize()) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Slide Size"));
    deck::setSlideSize(*m_ed->doc(), size);
    m_ed->endChange();
}

void DeckWindow::askSlideSize(const QSizeF &size)
{
    if (size == m_ed->doc()->pageSize()) {
        refreshUi();   // the check on the chosen size stays
        return;
    }
    QMessageBox box(QMessageBox::Question, tr("Slide Size"), tr("Scale your content to fit the new slide size?"), QMessageBox::NoButton, this);
    box.setInformativeText(tr("Ensure Fit shrinks your content when it has to, so all of it stays on the slide."));
    QPushButton *fit = box.addButton(tr("Ensure Fit"), QMessageBox::AcceptRole);
    box.addButton(QMessageBox::Cancel);
    box.setDefaultButton(fit);
    box.exec();
    if (box.clickedButton() == fit) setSlideSize(size);
    else refreshUi();
}

void DeckWindow::setBackground(const QColor &color, bool all)
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (!color.isValid() || (!all && (i < 0 || i >= d->pages.size()))) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Format Background"));
    if (all) deck::setAllBackgrounds(*d, color);
    else deck::setSlideBackground(*d->pages[i], color);
    m_ed->endChange();
}

void DeckWindow::showFormatBackground()
{
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (i < 0 || i >= d->pages.size()) return;
    // The color the slide shows now: its own, else its layout's, else the master's.
    const Page &slide = *d->pages[i];
    Fill fill = slide.background;
    const QVector<MasterPage *> chain = d->masterChain(d->masterFor(slide));
    for (int k = int(chain.size()) - 1; k >= 0 && fill.type == Fill::NoFill; --k) fill = chain[k]->background;
    const QColor now = fill.type == Fill::Solid ? fill.color.resolve(d->colors) : QColor(Qt::white);
    BackgroundDialog dlg(now, d->colors, this);
    if (dlg.exec() == QDialog::Accepted) setBackground(dlg.color(), dlg.applyToAll());
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

void DeckWindow::showHeaderFooter()
{
    HeaderFooterDialog dlg(deck::footerOptions(*m_ed->doc()), this);
    if (dlg.exec() == QDialog::Accepted) applyFooters(dlg.options(), dlg.applyToAll());
}

bool DeckWindow::insertPicture(const QString &path, const QPointF &at, const QString &altText, const QString &credit)
{
    QByteArray bytes = readPictureBytes(path);
    QString format;
    QSize pixels;
    if (bytes.isEmpty() || !readPictureFile(path, 0, &bytes, &format, &pixels)) return false;
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
        if (!insertPicture(path)) QMessageBox::warning(this, tr("Insert Picture"), tr("%1 can't be read as a picture.").arg(QFileInfo(path).fileName()));
}

void DeckWindow::insertTable(int rows, int columns)
{
    rows = std::clamp(rows, 1, 100);
    columns = std::clamp(columns, 1, 30);
    const QSizeF ps = m_ed->doc()->pageSize();
    const double w = ps.width() * 0.8, h = std::min(ps.height() * 0.8, 36.0 * rows);
    auto table = m_ed->newTable(QRectF((ps.width() - w) / 2, (ps.height() - h) / 2, w, h), rows, columns);
    // Text that reads on a slide.
    for (const TableCell &cell : static_cast<TableItem *>(table.get())->cells) {
        QTextCursor c(m_ed->doc()->storyDoc(cell.storyId));
        QTextCharFormat f;
        f.setFontPointSize(18);
        c.mergeBlockCharFormat(f);
    }
    m_ed->endTextEdit();
    m_ed->addItem(table);
}

} // namespace jp
