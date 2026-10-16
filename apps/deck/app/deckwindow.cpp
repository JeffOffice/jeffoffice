#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/deckfilescreen.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/keytips.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "app/telemetry.h"
#include "app/widgets.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "core/embeddedfonts.h"
#include "core/sections.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextList>
#include <QWindow>

namespace jp {

DeckWindow::DeckWindow(QWidget *parent) : AppWindow(parent)
{
    setObjectName(QStringLiteral("deckWindow"));
    setWindowIcon(drawnIcon([](QPainter *p, const QRectF &r) {
        p->setPen(Qt::NoPen);
        p->setBrush(appInfo().accent);
        p->drawRoundedRect(r.adjusted(1, 1, -1, -1), r.width() * 0.24, r.width() * 0.24);
        QFont f;
        f.setPixelSize(int(r.height() * 0.42));
        f.setWeight(QFont::DemiBold);
        p->setFont(f);
        p->setPen(Qt::white);
        p->drawText(r, Qt::AlignCenter, appInfo().badge);
    }));
    m_ed = new Editor(this);
    m_ed->setNewDocumentName(tr("Presentation1"));
    m_ed->setDocument(deck::newPresentation());
    m_fontsOf = m_ed->doc();
    // A presentation's view: no rulers, guides, or boundaries until they are asked for (the canvas outlines empty placeholders itself).
    m_ed->view.guides = false;
    m_ed->view.boundaries = false;
    m_ed->view.rulers = false;
    m_canvas = new DeckCanvas(m_ed, this);
    m_canvas->setRulersVisible(false);
    m_ribbon = new Ribbon(this);
    m_slides = new SlidesPane(m_ed, this);
    m_notes = new NotesPane(m_ed, this);
    setMenuWidget(m_ribbon);

    m_rightSplit = new QSplitter(Qt::Vertical, this);
    m_rightSplit->addWidget(m_canvas);
    m_rightSplit->addWidget(m_notes);
    m_rightSplit->setStretchFactor(0, 1);
    m_rightSplit->setCollapsible(0, false);
    m_rightSplit->setSizes({600, 90});
    m_split = new QSplitter(Qt::Horizontal, this);
    m_split->addWidget(m_slides);
    m_split->addWidget(m_rightSplit);
    m_split->setStretchFactor(1, 1);
    m_split->setCollapsible(1, false);
    m_split->setSizes({250, 1000});
    setCentralWidget(m_split);

    createActions();
    createFormatActions();
    createInsertActions();
    createDesignActions();
    createViewActions();
    buildRibbon();
    buildStatusBar();
    describeForScreenReaders(m_split);
    describeForScreenReaders(statusBar());
    m_fileScreen = new DeckFileScreen(this);
    m_fileScreen->hide();
    connect(m_fileScreen, &DeckFileScreen::closeRequested, this, &DeckWindow::hideFileScreen);
    new KeyTips(this, m_ribbon, [this] { showFileScreen(); });   // Alt, F
    connect(m_ribbon, &Ribbon::fileClicked, this, [this] { showFileScreen(); });
    connect(m_ribbon, &Ribbon::leaveRequested, this, [this] { m_canvas->setFocus(Qt::OtherFocusReason); });

    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(30);
    connect(&m_refreshTimer, &QTimer::timeout, this, &DeckWindow::refreshUi);
    auto queue = [this] { m_refreshTimer.start(); };
    // A box the presentation program shrank keeps the size it gave the text until the text is edited here (a click into
    // it changes nothing): then it is fitted here.
    auto watchFit = [this](bool changed) {
        if (!m_ed->isEditingText()) {
            m_fitWatch = {};
            return;
        }
        const Editor::TextTarget &target = m_ed->textTarget();
        const QTextDocument *story = m_ed->doc()->storyDoc(target.storyId);
        const int revision = story ? story->revision() : -1;
        if (m_fitWatch.first != target.itemId) {
            m_fitWatch = {target.itemId, revision};
        } else if (changed && revision != m_fitWatch.second) {
            if (Item *it = m_ed->doc()->item(target.itemId); it && it->type() == ItemType::Text) static_cast<TextItem *>(it)->fitAsStored = false;
            m_fitWatch.second = revision;
        }
    };
    connect(m_ed, &Editor::changed, this, [this, queue, watchFit] {
        watchFit(true);
        m_slides->refresh(true);
        m_notes->reload();   // an undo can change the notes
        queue();
    });
    connect(m_ed, &Editor::selectionChanged, this, queue);
    connect(m_ed, &Editor::textCursorChanged, this, [queue, watchFit] {
        watchFit(false);
        queue();
    });
    connect(m_ed, &Editor::toolChanged, this, queue);
    connect(m_ed, &Editor::pageChanged, this, [this, queue] {
        m_notes->commit();
        if (m_outline) m_outline->commit();
        m_notes->reload();
        m_slides->followCurrent();   // drawn again only when something changed (Editor::changed)
        queue();
    });
    connect(m_ed, &Editor::documentReplaced, this, [this] {
        // The fonts the presentation before it carried go (the new one's were added as it was read).
        deck::EmbeddedFonts::release(m_fontsOf);
        m_fontsOf = m_ed->doc();
        m_slides->refresh();
        m_notes->reload();
        updateTitle();
        refreshUi();
    });
    connect(m_ed, &Editor::modifiedChanged, this, &DeckWindow::updateTitle);
    connect(m_slides, &SlidesPane::newSlideWanted, this, [this] { newSlide(); });
    connect(m_slides, &SlidesPane::duplicateWanted, this, &DeckWindow::duplicateSlide);
    connect(m_slides, &SlidesPane::deleteWanted, this, &DeckWindow::deleteSlide);
    connect(m_slides, &SlidesPane::hideWanted, this, &DeckWindow::toggleHidden);
    // Pictures dropped on the slide.
    connect(m_canvas, &Canvas::insertFilesWanted, this, [this](const QStringList &paths, const QPointF &at) {
        QPointF where = at;
        for (const QString &path : paths)
            if (insertPicture(path, where) && where.x() >= 0) where += QPointF(18, 18);
    });
    m_canvas->zoomToFit(Canvas::Fit::WholePage);
    resize(1280, 800);
    updateTitle();
    refreshUi();
}

DeckWindow::~DeckWindow()
{
    if (m_show) m_show->close();
    deck::EmbeddedFonts::release(m_fontsOf);
}

QAction *DeckWindow::mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn,
                        bool checkable)
{
    auto *a = new QAction(text, this);
    a->setObjectName(id);
    if (!iconName.isEmpty()) a->setIcon(icon(iconName));
    if (!key.isEmpty()) {
        a->setShortcut(key);
        a->setShortcutContext(Qt::WindowShortcut);
        a->setToolTip(tr("%1 (%2)").arg(QString(text).remove('&'), key.toString(QKeySequence::NativeText)));
        addAction(a);
    }
    a->setCheckable(checkable);
    // Each command counts by its id for the usage statistics (when they're on).
    connect(a, &QAction::triggered, this, [fn, id] {
        telemetry::count(id);
        fn();
    });
    m_actions.insert(id, a);
    return a;
}

QAction *DeckWindow::act(const QString &id) const { return m_actions.value(id); }

QStringList DeckWindow::actionIds() const
{
    QStringList ids = m_actions.keys();
    ids.sort();
    return ids;
}

void DeckWindow::createActions()
{
    Editor *ed = m_ed;
    mk("file.new", tr("New"), "file-plus", QKeySequence::New, [] {
        auto *w = new DeckWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
    });
    mk("file.open", tr("Open"), "folder-open", QKeySequence::Open, [this] {
        const QString p = QFileDialog::getOpenFileName(this, tr("Open Presentation"), Settings::get().value("dirs/open", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                       tr("Presentations (*.pptx *.potx *.ppsx *.ppt *.pps *.pot *.odp *.otp)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (p.isEmpty()) return;
        Settings::get().setValue("dirs/open", QFileInfo(p).absolutePath());
        openFile(p);
    });
    mk("file.close", tr("Close"), "circle-x", QKeySequence::Close, [this] { close(); });
    mk("edit.undo", tr("Undo"), "undo-2", QKeySequence::Undo, [ed] { ed->undo(); });
    mk("edit.redo", tr("Redo"), "redo-2", QKeySequence::Redo, [ed] { ed->redo(); });
    mk("edit.cut", tr("Cut"), "scissors", QKeySequence::Cut, [ed] { ed->cut(); });
    mk("edit.copy", tr("Copy"), "copy", QKeySequence::Copy, [ed] { ed->copy(); });
    mk("edit.paste", tr("Paste"), "clipboard-paste", QKeySequence::Paste, [ed] { ed->paste(); });
    mk("edit.delete", tr("Delete"), "trash-2", QKeySequence(), [ed] { ed->deleteSelection(); });

    mk("slide.new", tr("New Slide"), "copy-plus", QKeySequence(Qt::CTRL | Qt::Key_M), [this] { newSlide(); });
    mk("slide.duplicate", tr("Duplicate Slide"), "copy", QKeySequence(), [this] { duplicateSlide(); });
    mk("slide.delete", tr("Delete Slide"), "trash-2", QKeySequence(), [this] { deleteSlide(); });
    mk("slide.hide", tr("Hide Slide"), "eye-off", QKeySequence(), [this] { toggleHidden(); }, true);
    mk("slide.reset", tr("Reset"), "rotate-ccw", QKeySequence(), [this] { resetSlide(); });

    mk("fmt.bold", tr("Bold"), "bold", QKeySequence::Bold, [ed] { ed->toggleBold(); }, true);
    mk("fmt.italic", tr("Italic"), "italic", QKeySequence::Italic, [ed] { ed->toggleItalic(); }, true);
    mk("fmt.underline", tr("Underline"), "underline", QKeySequence::Underline, [ed] { ed->toggleUnderline(); }, true);
    mk("fmt.strike", tr("Strikethrough"), "strikethrough", QKeySequence(), [ed] { ed->toggleStrike(); }, true);
    mk("fmt.alignLeft", tr("Align Left"), "align-left", QKeySequence(Qt::CTRL | Qt::Key_L), [ed] { ed->setAlignment(Qt::AlignLeft); }, true);
    mk("fmt.alignCenter", tr("Center"), "align-center", QKeySequence(Qt::CTRL | Qt::Key_E), [ed] { ed->setAlignment(Qt::AlignHCenter); }, true);
    mk("fmt.alignRight", tr("Align Right"), "align-right", QKeySequence(Qt::CTRL | Qt::Key_R), [ed] { ed->setAlignment(Qt::AlignRight); }, true);
    mk("fmt.justify", tr("Justify"), "align-justify", QKeySequence(Qt::CTRL | Qt::Key_J), [ed] { ed->setAlignment(Qt::AlignJustify); }, true);

    mk("insert.textBox", tr("Text Box"), "text-cursor-input", QKeySequence(), [ed] { ed->setTool(Tool::Text); });
    mk("insert.picture", tr("Pictures"), "image", QKeySequence(), [this] { insertPictureFromFile(); });
    mk("insert.table", tr("Insert Table…"), "table", QKeySequence(), [this] {
        TableSizeDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) insertTable(dlg.rows(), dlg.columns());
    });

    mk("show.fromStart", tr("From Beginning"), "presentation", QKeySequence(Qt::Key_F5), [this] { startShowFromBeginning(); });
    mk("show.fromCurrent", tr("From Current Slide"), "monitor-play", QKeySequence(Qt::SHIFT | Qt::Key_F5), [this] { startShow(m_ed->currentPage()); });

    mk("view.notes", tr("Notes"), "notebook-pen", QKeySequence(), [this] { m_notes->setVisible(m_notes->isHidden()); refreshUi(); }, true);
    mk("view.fit", tr("Fit to Window"), "scan", QKeySequence(), [this] { m_canvas->zoomToFit(Canvas::Fit::WholePage); });
    createShowActions();
    createAnimationActions();
    createViewModeActions();
    createSaveActions();
    createDialogActions();
    createCommentActions();
    createSectionActions();
    createOutputActions();
    createHelpActions();
    createMediaActions();
    createTableActions();
    createCropActions();
}

void DeckWindow::refreshUi()
{
    Editor *ed = m_ed;
    Document *d = ed->doc();
    const int n = d ? int(d->pages.size()) : 0;
    m_slideLabel->setText(tr("Slide %1 of %2").arg(ed->currentPage() + 1).arg(n));
    act("slide.delete")->setEnabled(n > 1);
    act("slide.hide")->setChecked(n && d->pages[std::clamp(ed->currentPage(), 0, n - 1)]->hidden);
    act("edit.undo")->setEnabled(ed->undoStack()->canUndo());
    act("edit.redo")->setEnabled(ed->undoStack()->canRedo());
    act("edit.undo")->setToolTip(tr("Undo %1 (Ctrl+Z)").arg(ed->undoStack()->undoText()));
    act("edit.redo")->setToolTip(tr("Redo %1 (Ctrl+Y)").arg(ed->undoStack()->redoText()));

    // What the selection lets the text and drawing commands do.
    const QString kind = ed->selectionKind();
    const bool sel = !kind.isEmpty(), editing = ed->isEditingText();
    const bool textish = editing || (ed->single() && ed->single()->hasText()) || kind == "multi";
    const QTextCharFormat cf = textish ? ed->currentCharFormat() : QTextCharFormat();
    const QTextBlockFormat bf = textish ? ed->currentBlockFormat() : QTextBlockFormat();
    for (const char *id : {"edit.cut", "edit.copy", "edit.delete"}) act(id)->setEnabled(sel || (editing && ed->cursor().hasSelection()));
    for (const char *id : {"arr.front", "arr.forward", "arr.backward", "arr.back", "arr.rotR", "arr.rotL", "arr.flipH", "arr.flipV"}) act(id)->setEnabled(sel);
    act("arr.group")->setEnabled(kind == "multi");
    act("arr.ungroup")->setEnabled(kind == "group");
    for (const char *id : {"fmt.bold", "fmt.italic", "fmt.underline", "fmt.strike", "fmt.shadow", "fmt.grow", "fmt.shrink", "fmt.clear", "para.bullets", "para.numbers",
                           "fmt.alignLeft", "fmt.alignCenter", "fmt.alignRight", "fmt.justify", "para.indentDec", "para.indentInc"})
        act(id)->setEnabled(textish);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("ls.")) || a->objectName().startsWith(QLatin1String("case.")) || a->objectName().startsWith(QLatin1String("spacing.")))
            a->setEnabled(textish);
    act("fmt.bold")->setChecked(cf.fontWeight() >= QFont::DemiBold);
    act("fmt.italic")->setChecked(cf.fontItalic());
    act("fmt.underline")->setChecked(cf.underlineStyle() != QTextCharFormat::NoUnderline);
    act("fmt.strike")->setChecked(cf.fontStrikeOut());
    act("fmt.shadow")->setChecked(Editor::hasTextShadow(cf));
    const Qt::Alignment al = bf.alignment() & Qt::AlignHorizontal_Mask;
    act("fmt.alignLeft")->setChecked(textish && (al == Qt::AlignLeft || al == 0 || al == Qt::AlignLeading));
    act("fmt.alignCenter")->setChecked(al == Qt::AlignHCenter);
    act("fmt.alignRight")->setChecked(al == Qt::AlignRight || al == Qt::AlignTrailing);
    act("fmt.justify")->setChecked(al == Qt::AlignJustify);
    QTextList *list = editing ? ed->cursor().block().textList() : nullptr;
    act("para.bullets")->setChecked(list && isBulletList(list->format().style()));
    act("para.numbers")->setChecked(list && !isBulletList(list->format().style()));
    act("edit.formatPainter")->setChecked(ed->tool() == Tool::FormatPainter);
    // Text Direction, Align Text, and Columns read the box being typed in or the selected one.
    Item *one = editing ? d->item(ed->textTarget().itemId) : ed->single();
    const bool box = one && (one->type() == ItemType::Text || one->type() == ItemType::Shape);
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("valign.")) || a->objectName().startsWith(QLatin1String("cols.")) || a->objectName().startsWith(QLatin1String("tb.dir")))
            a->setEnabled(box);
    if (auto *t = dynamic_cast<TextItem *>(one)) {
        act("tb.dirHorizontal")->setChecked(!t->vertical);
        act("tb.dirVertical")->setChecked(t->vertical);
        for (int k = 0; k < 3; ++k) act(QStringLiteral("valign.%1").arg(k))->setChecked(int(t->valign) == k);
        for (int c = 1; c <= 3; ++c) act(QStringLiteral("cols.%1").arg(c))->setChecked(t->columns == c);
    } else if (auto *s = dynamic_cast<ShapeItem *>(one)) {
        for (int k = 0; k < 3; ++k) act(QStringLiteral("valign.%1").arg(k))->setChecked(int(s->valign) == k);
    }

    // The font and size boxes, and the colors the pickers offer.
    QString family;
    if (textish) {
        if (cf.hasProperty(QTextFormat::FontFamilies)) family = cf.fontFamilies().toStringList().value(0);
        else family = cf.stringProperty(tp::ThemeFont) == QLatin1String("major") ? d->fonts.heading : d->fonts.body;
    }
    for (FontCombo *f : m_fontCombos) {
        f->setSchemeFonts(d->fonts.heading, d->fonts.body);
        f->setEnabled(textish);
        f->setCurrentFamily(family);
    }
    for (SizeCombo *sz : m_sizeCombos) {
        sz->setEnabled(textish);
        sz->setSize(textish ? (cf.hasProperty(QTextFormat::FontPointSize) ? cf.fontPointSize() : 11) : 0);
    }
    for (ColorButton *c : m_colorButtons) c->setScheme(d->colors);
    for (ColorButton *c : m_colorButtons) c->setEnabled(c->objectName() == QLatin1String("textColor") ? textish : sel);

    // The view, and the slide size, which the canvas fits again when it changed.
    act("view.notes")->setChecked(!m_notes->isHidden());
    act("view.rulers")->setChecked(ed->view.rulers);
    act("view.gridlines")->setChecked(m_canvas->gridlines());
    act("view.guides")->setChecked(m_canvas->guides());
    act("design.sizeStandard")->setChecked(d->pageSize() == QSizeF(deck::kStandardWidth, deck::kStandardHeight));
    act("design.sizeWidescreen")->setChecked(d->pageSize() == QSizeF(deck::kSlideWidth, deck::kSlideHeight));
    if (d->pageSize() != m_shownSize) {
        m_shownSize = d->pageSize();
        if (m_canvas->fitMode() != Canvas::Fit::None) m_canvas->zoomToFit(m_canvas->fitMode());
        m_canvas->viewport()->update();
    }
}

void DeckWindow::updateTitle()
{
    setWindowTitle(QStringLiteral("%1%2 - %3").arg(m_ed->displayName(), m_ed->isModified() ? QStringLiteral("*") : QString(), appInfo().name));
}

void DeckWindow::showFileScreen(const QString &page)
{
    m_notes->commit();
    m_ed->endTextEdit();
    m_fileScreen->setGeometry(rect());
    m_fileScreen->showPage(page.isEmpty() ? QStringLiteral("open") : page);
    m_fileScreen->show();
    m_fileScreen->raise();
    m_fileScreen->setFocus();
    qApp->installEventFilter(this);
}

void DeckWindow::hideFileScreen()
{
    if (!m_fileScreen->isVisible()) return;
    qApp->removeEventFilter(this);
    m_fileScreen->hide();
    m_canvas->setFocus();
}

void DeckWindow::resizeEvent(QResizeEvent *e)
{
    AppWindow::resizeEvent(e);
    if (m_fileScreen) m_fileScreen->setGeometry(rect());
}

bool DeckWindow::eventFilter(QObject *o, QEvent *e)
{
    // Escape closes the File screen from anywhere in the window.
    if (e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape && m_fileScreen->isVisible()) {
        auto *w = qobject_cast<QWidget *>(o);
        if (w && w->window() == this) {
            hideFileScreen();
            return true;
        }
    }
    return AppWindow::eventFilter(o, e);
}

void DeckWindow::newSlide(const QString &layoutType)
{
    m_notes->commit();
    if (m_outline) m_outline->commit();
    Document *d = m_ed->doc();
    const int at = m_ed->currentPage();
    // The current slide's slide master's layouts, when the file has several.
    const QString master = deck::slideMasterOf(*d, at);
    const MasterPage *layout = layoutType.isEmpty() ? nullptr : deck::layoutOfType(*d, layoutType, master);
    if (!layout && at >= 0 && at < d->pages.size()) {
        layout = d->masterFor(*d->pages[at]);
        // After a title slide comes a slide with content, as people expect.
        if (layout && deck::layoutType(*layout) == QLatin1String("title")) layout = deck::layoutOfType(*d, QStringLiteral("obj"), master);
    }
    if (!layout) layout = deck::layoutOfType(*d, QStringLiteral("obj"), master);
    if (!layout) layout = deck::layoutOfType(*d, QStringLiteral("obj"));
    if (!layout) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("New Slide"));
    const int index = deck::addSlide(*d, *layout, at + 1);
    m_ed->endChange();
    m_ed->setCurrentPage(index);
}

void DeckWindow::duplicateSlide()
{
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Duplicate Slide"));
    const int index = deck::duplicateSlide(*m_ed->doc(), m_ed->currentPage());
    m_ed->endChange();
    if (index >= 0) m_ed->setCurrentPage(index);
}

void DeckWindow::deleteSlide()
{
    if (m_ed->doc()->pages.size() < 2) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Delete Slide"));
    deck::slideRemoved(*m_ed->doc(), m_ed->currentPage());
    m_ed->deletePage(m_ed->currentPage());
    m_ed->endChange();
}

void DeckWindow::toggleHidden()
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (i < 0 || i >= d->pages.size()) return;
    m_ed->beginChange(d->pages[i]->hidden ? tr("Show Slide") : tr("Hide Slide"));
    d->pages[i]->hidden = !d->pages[i]->hidden;
    m_ed->endChange();
}

void DeckWindow::setLayout(const QString &layoutType)
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    const MasterPage *layout = deck::layoutOfType(*d, layoutType, deck::slideMasterOf(*d, i));
    if (!layout) layout = deck::layoutOfType(*d, layoutType);
    if (!layout || i < 0 || i >= d->pages.size()) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Layout"));
    deck::applyLayout(*d, *d->pages[i], *layout);
    m_ed->endChange();
}

void DeckWindow::resetSlide()
{
    Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (i < 0 || i >= d->pages.size()) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Reset Slide"));
    deck::resetSlide(*d, *d->pages[i]);
    m_ed->endChange();
}

void DeckWindow::startShow(int slide) { playShow(slide, deck::ShowPlan(), false); }

bool DeckWindow::maybeSave()
{
    m_ed->flushTyping();
    if (!m_ed->isModified()) return true;
    QMessageBox box(QMessageBox::Question, appInfo().name, tr("Do you want to save changes to %1?").arg(m_ed->displayName()),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
    box.setDefaultButton(QMessageBox::Save);
    box.button(QMessageBox::Discard)->setText(tr("Don't Save"));
    box.exec();
    if (box.clickedButton() == box.button(QMessageBox::Save)) return save();
    if (box.clickedButton() != box.button(QMessageBox::Discard)) return false;
    dropRecoveryCopy();
    return true;
}

void DeckWindow::closeEvent(QCloseEvent *e)
{
    m_notes->commit();
    if (m_outline) m_outline->commit();
    if (!maybeSave()) {
        e->ignore();
        return;
    }
    if (m_show) m_show->close();
    e->accept();
}

} // namespace jp
