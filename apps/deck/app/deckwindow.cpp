#include "app/uistyle.h"
#include "app/commandsearch.h"
#include "app/keymap.h"
#include "app/ribboncustom.h"
#include "app/deckwindow.h"
#include "canvas/editorminitoolbar.h"
#include "core/startuptrace.h"

#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/deckfilescreen.h"
#include "app/editor.h"
#include "app/textdialogs.h"
#include "app/icons.h"
#include "app/keytips.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/slideclipboard.h"
#include "app/slideshow.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"
#include "app/telemetry.h"
#include "app/widgets.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "io/presentationdefaults.h"
#include "core/embeddedfonts.h"
#include "core/sections.h"
#include "core/themes.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
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
#include "app/plaintext.h"

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
    m_ed->setDocument(deck::newDefaultPresentation());
    m_fontsOf = m_ed->doc();
    m_openedTheme = deck::themeOf(*m_ed->doc());
    // A presentation's view: no rulers, guides, or boundaries until they are asked for (the canvas outlines empty placeholders itself).
    m_ed->view.guides = false;
    m_ed->view.boundaries = false;
    m_ed->view.rulers = false;
    m_ed->view.sharedGuides = true;
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
    createGridActions();
    createShapeFormatActions();
    startupMark("win: before ribbon");
    buildRibbon();
    startupMark("win: ribbon built");
    KeyMap::apply(this);   // (the keys the person chose in Options > Customize)
    RibbonCustom::apply(this);   // (and the tabs and groups the person shows, hides, and adds)
    buildStatusBar();
    describeForScreenReaders(m_split);
    describeForScreenReaders(statusBar());
    m_fileScreen = new DeckFileScreen(this);
    m_fileScreen->hide();
    connect(m_fileScreen, &DeckFileScreen::closeRequested, this, &DeckWindow::hideFileScreen);
    new KeyTips(this, m_ribbon, [this] { showFileScreen(); });   // Alt, F
    connect(m_ribbon, &Ribbon::fileClicked, this, [this] { showFileScreen(); });
    connect(m_ribbon, &Ribbon::leaveRequested, this, [this] { m_canvas->setFocus(Qt::OtherFocusReason); });
    new CommandSearchBox(this, QStringLiteral(":/deck/cmdsearch.json"));

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
            else if (Item *sh = m_ed->doc()->item(target.itemId); sh && sh->type() == ItemType::Shape) static_cast<ShapeItem *>(sh)->fitAsStored = false;
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
        applyGridView(true);
        m_slides->refresh();
        m_notes->reload();
        updateTitle();
        refreshUi();
    });
    connect(m_ed, &Editor::changed, this, [this] { applyGridView(false); });   // (an undo of the dialog's step changes them back)
    connect(m_ed, &Editor::modifiedChanged, this, &DeckWindow::updateTitle);
    connect(m_slides, &SlidesPane::newSlideWanted, this, [this] { newSlide(); });
    connect(m_slides, &SlidesPane::duplicateWanted, this, &DeckWindow::duplicateSlide);
    connect(m_slides, &SlidesPane::deleteWanted, this, &DeckWindow::deleteSlide);
    connect(m_slides, &SlidesPane::cutWanted, this, &DeckWindow::cutSlides);
    connect(m_slides, &SlidesPane::copyWanted, this, &DeckWindow::copySlides);
    connect(m_slides, &SlidesPane::pasteWanted, this, &DeckWindow::pasteSlides);
    connect(m_slides, &SlidesPane::hideWanted, this, &DeckWindow::toggleHidden);
    // Pictures dropped on the slide.
    connect(m_canvas, &Canvas::insertFilesWanted, this, [this](const QStringList &paths, const QPointF &at) {
        QPointF where = at;
        for (const QString &path : paths)
            if (insertPicture(path, where) && where.x() >= 0) where += QPointF(18, 18);
    });
    connect(m_canvas, &Canvas::contextMenuWanted, this, &DeckWindow::showSlideContextMenu);
    // The mini toolbar by words selected with the mouse: the ribbon's own commands, on the text being typed in.
    {
        auto target = std::make_unique<FunctionMiniTarget>();
        target->actionFor = [this](MiniTarget::Command c) {
            using C = MiniTarget::Command;
            switch (c) {
            case C::Bold: return act("fmt.bold");
            case C::Italic: return act("fmt.italic");
            case C::Underline: return act("fmt.underline");
            case C::Shrink: return act("fmt.shrink");
            case C::Grow: return act("fmt.grow");
            case C::AlignLeft: return act("fmt.alignLeft");
            case C::AlignCenter: return act("fmt.alignCenter");
            case C::AlignRight: return act("fmt.alignRight");
            case C::Bullets: return act("para.bullets");
            case C::Numbering: return act("para.numbers");
            }
            return static_cast<QAction *>(nullptr);
        };
        target->setFont = [this](const QString &f) {
            m_ed->setFontFamily(f);
            m_canvas->setFocus();
        };
        target->setColor = [this](const ColorRef &c) {
            m_ed->setTextColor(c);
            m_canvas->setFocus();
        };
        auto *mini = new EditorMiniToolbar(m_canvas, std::move(target), [this] { return isReadOnly(); }, [this] { refreshUi(); });
        m_fontCombos << mini->bar()->fontBox();
        m_colorButtons << mini->bar()->colorButton();
    }
    m_canvas->zoomToFit(Canvas::Fit::WholePage);
    resize(1280, 800);
    updateTitle();
    refreshUi();
    // The Format Shape pane is open if it was when the last window closed it.
    if (Settings::get().value(QStringLiteral("view/formatPane"), false).toBool()) showFormatPane(true);
    // (the keys go to the slide: a window that opens with the focus on its File button, the first in the tab order, would take typing as presses of it)
    m_canvas->setFocus();
}

DeckWindow::~DeckWindow()
{
    qApp->removeEventFilter(this);   // (the filter of Full Screen and of the File screen)
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
        a->setToolTip(jp::plainToolTip(tr("%1 (%2)").arg(QString(text).remove('&'), key.toString(QKeySequence::NativeText))));
        addAction(a);
    }
    a->setCheckable(checkable);
    // Each command counts by its id for the usage statistics (when they're on).
    connect(a, &QAction::triggered, this, [this, fn, id] {
        if (isReadOnly() && !commandAllowedReadOnly(id)) {
            refuseReadOnlyChange();
            return;
        }
        telemetry::count(id);
        // A crop in progress ends (and is kept) when another command goes to work, except the ones that belong to it or only change the view.
        if (m_ed && !m_ed->cropItem.isEmpty() && !id.startsWith(QLatin1String("pic.")) && !id.startsWith(QLatin1String("view.")) && !id.startsWith(QLatin1String("zoom.")) &&
            !id.startsWith(QLatin1String("edit.undo")) && !id.startsWith(QLatin1String("edit.copy")) && !id.startsWith(QLatin1String("help.")))
            m_ed->endCrop(true);
        fn();
    });
    m_actions.insert(id, a);
    KeyMap::applyLater(this, a);
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
    mk("file.open", tr("Open"), "folder-open", QKeySequence::Open, [this] { browseAndOpen(); });
    mk("file.close", tr("Close"), "circle-x", QKeySequence::Close, [this] { close(); });
    mk("edit.undo", tr("Undo"), "undo-2", QKeySequence::Undo, [ed] { ed->undo(); });   // while cropping, takes the crop back, as Esc does
    mk("edit.redo", tr("Redo"), "redo-2", QKeySequence::Redo, [ed] { ed->redo(); });
    mk("edit.cut", tr("Cut"), "scissors", QKeySequence::Cut, [this, ed] { if (slideListFocused()) cutSlides(); else ed->cut(); });
    mk("edit.copy", tr("Copy"), "copy", QKeySequence::Copy, [this, ed] { if (slideListFocused()) copySlides(); else ed->copy(); });
    mk("edit.paste", tr("Paste"), "clipboard-paste", QKeySequence::Paste, [this, ed] { if (slideListFocused()) pasteSlides(true); else ed->paste(); });
    mk("edit.pasteKeep", tr("Keep Source Formatting"), "clipboard-paste", QKeySequence(), [ed] { ed->paste(false); });
    mk("edit.pasteText", tr("Keep Text Only"), "clipboard-type", QKeySequence(), [ed] { ed->paste(true); });
    mk("edit.pasteCells", tr("Spreadsheet Cells as Table"), "table", QKeySequence(), [ed] { ed->pasteCells(QApplication::clipboard()->mimeData()); });
    mk("edit.pasteSpecial", tr("Paste Special…"), "clipboard-list", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V), [this, ed] { pasteSpecialDialog(this, ed); });
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
    createMasterActions();
    createNotesMasterActions();
    createSaveActions();
    createDialogActions();
    createEditingActions();
    createCommentActions();
    createSectionActions();
    createOutputActions();
    createHelpActions();
    createMediaActions();
    createTableActions();
    createCropActions();
    createChartActions();
}

void DeckWindow::refreshUi()
{
    Editor *ed = m_ed;
    Document *d = ed->doc();
    const int n = d ? int(d->pages.size()) : 0;
    const MasterPage *open = m_view == View::Master && d ? d->master(ed->masterView()) : nullptr;
    m_slideLabel->setText(open ? (deck::isSlideMaster(*open) ? tr("Slide master: %1") : tr("Layout: %1")).arg(open->name) : tr("Slide %1 of %2").arg(ed->currentPage() + 1).arg(n));
    m_slideLabel->setMinimumWidth(std::max(110, m_slideLabel->fontMetrics().horizontalAdvance(m_slideLabel->text()) + 12));
    act("slide.delete")->setEnabled(n > 1 && m_view != View::Master);
    act("slide.hide")->setChecked(n && d->pages[std::clamp(ed->currentPage(), 0, n - 1)]->hidden);
    act("edit.undo")->setEnabled(ed->undoStack()->canUndo());
    act("edit.redo")->setEnabled(ed->undoStack()->canRedo());
    // (each shows the key the command has now, which Options > Customize can change)
    act("edit.undo")->setToolTip(jp::plainToolTip(KeyMap::tipWithShownKey(tr("Undo %1 (Ctrl+Z)").arg(ed->undoStack()->undoText()), QStringLiteral("Ctrl+Z"), act("edit.undo"))));
    act("edit.redo")->setToolTip(jp::plainToolTip(KeyMap::tipWithShownKey(tr("Redo %1 (Ctrl+Y)").arg(ed->undoStack()->redoText()), QStringLiteral("Ctrl+Y"), act("edit.redo"))));

    // What the selection lets the text and drawing commands do.
    const QString kind = ed->selectionKind();
    const bool sel = !kind.isEmpty(), editing = ed->isEditingText();
    const bool textish = editing || (ed->single() && ed->single()->hasText()) || kind == "multi";
    const QTextCharFormat cf = textish ? ed->currentCharFormat() : QTextCharFormat();
    const QTextBlockFormat bf = textish ? ed->currentBlockFormat() : QTextBlockFormat();
    for (const char *id : {"edit.cut", "edit.copy", "edit.delete"}) act(id)->setEnabled(sel || (editing && ed->cursor().hasSelection()));
    for (const char *id : {"arr.front", "arr.forward", "arr.backward", "arr.back", "arr.rotR", "arr.rotL", "arr.flipH", "arr.flipV"}) act(id)->setEnabled(sel);
    for (const char *id : {"arr.alignLeft", "arr.alignCenter", "arr.alignRight", "arr.alignTop", "arr.alignMiddle", "arr.alignBottom"}) act(id)->setEnabled(sel);
    for (const char *id : {"arr.distH", "arr.distV"}) act(id)->setEnabled(ed->selection().size() > 2 || (sel && act("arr.alignToSlide")->isChecked()));
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
    for (const char *id : {"review.language", "review.noProof"}) act(id)->setEnabled(textish);
    act("review.noProof")->setChecked(textish && cf.boolProperty(tp::NoProof));
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
    {
        const deck::FooterOptions footers = deck::footerOptions(*d);
        const deck::SlideNumberPlace place = deck::slideNumberPlace(*d);
        act("insert.slideNumberOn")->setChecked(footers.slideNumber);
        act("insert.slideNumberSkipTitle")->setChecked(footers.skipTitleSlide);
        const char *down = place.down == deck::NumberDown::Top ? "top" : "bottom";
        const char *across = place.across == deck::NumberAcross::Left ? "Left" : place.across == deck::NumberAcross::Center ? "Center" : "Right";
        for (const char *id : {"topLeft", "topCenter", "topRight", "bottomLeft", "bottomCenter", "bottomRight"})
            act((QByteArray("insert.slideNumberPlace.") + id).constData())->setChecked(QByteArray(id) == QByteArray(down) + across);
    }
    act("design.sizeStandard")->setChecked(d->pageSize() == QSizeF(deck::kStandardWidth, deck::kStandardHeight));
    act("design.sizeWidescreen")->setChecked(d->pageSize() == QSizeF(deck::kSlideWidth, deck::kSlideHeight));
    for (const deck::SlideSizePreset &p : deck::slideSizePresets())
        if (p.key != QLatin1String("standard") && p.key != QLatin1String("widescreen"))
            act(("design.size." + p.key).toLatin1().constData())->setChecked(std::abs(d->pageSize().width() - p.size.width()) < 0.5 && std::abs(d->pageSize().height() - p.size.height()) < 0.5);
    if (d->pageSize() != m_shownSize) {
        m_shownSize = d->pageSize();
        if (m_canvas->fitMode() != Canvas::Fit::None) m_canvas->zoomToFit(m_canvas->fitMode());
        m_canvas->viewport()->update();
    }
    refreshMasterActions();
}

void DeckWindow::updateTitle()
{
    setWindowTitle(QStringLiteral("%1%2%4 - %3").arg(m_ed->displayName(), m_ed->isModified() ? QStringLiteral("*") : QString(), appInfo().name, isReadOnly() ? tr(" (Read-Only)") : QString()));
}

FileScreen *DeckWindow::fileScreen() const { return m_fileScreen; }

QStringList DeckWindow::optionsPageTitles() const { return {tr("View"), tr("Save")}; }

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
    if (!isFullScreen()) qApp->removeEventFilter(this);   // (Full Screen keeps it: Esc leaves that)
    m_fileScreen->hide();
    m_canvas->setFocus();
}

// While the window is full screen it watches the keys, so that Esc can leave it (the File screen watches them too, and shares the filter).
void DeckWindow::changeEvent(QEvent *e)
{
    AppWindow::changeEvent(e);
    if (e->type() != QEvent::WindowStateChange) return;
    if (isFullScreen()) qApp->installEventFilter(this);
    else if (m_fileScreen && !m_fileScreen->isVisible()) qApp->removeEventFilter(this);
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
    // Escape leaves Full Screen, unless the slide has something to put away first.
    if (e->type() == QEvent::KeyPress && isFullScreen()) {
        const KeyTips *keyTips = nullptr;
        for (QObject *child : children())
            if ((keyTips = dynamic_cast<const KeyTips *>(child))) break;
        const bool slideWants = m_ed->isEditingText() || !m_ed->selection().isEmpty() || m_ed->tool() != Tool::Select || !m_ed->cropItem.isEmpty() || !m_ed->pointsItem.isEmpty()
                                || !m_ed->wrapItem.isEmpty() || m_ed->hasCellBlock() || (keyTips && keyTips->level() != KeyTips::Off);
        if (escapeLeavesFullScreen(o, e, slideWants)) return true;
    }
    return AppWindow::eventFilter(o, e);
}

void DeckWindow::newSlide(const QString &layoutType)
{
    if (isReadOnly()) {
        refuseReadOnlyChange();
        return;
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    Document *d = m_ed->doc();
    const int at = m_ed->currentPage();
    // The current slide's slide master's layouts, when the file has several.
    const QString master = deck::slideMasterOf(*d, at);
    const MasterPage *layout = layoutType.isEmpty() ? nullptr : deck::layoutByKey(*d, layoutType, master);
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

// The slides pane or the Slide Sorter has the keyboard: Cut, Copy, and Paste are for whole slides there.
bool DeckWindow::slideListFocused() const
{
    const QWidget *f = QApplication::focusWidget();
    return f && ((m_slides && (f == m_slides || m_slides->isAncestorOf(f))) || (m_sorter && (f == m_sorter || m_sorter->isAncestorOf(f))));
}

void DeckWindow::copySlides()
{
    if (m_sorter->isVisible()) { m_sorter->copySelected(); return; }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    deck::copySlidesToClipboard(*m_ed->doc(), {m_ed->currentPage()});
}

void DeckWindow::cutSlides()
{
    if (m_sorter->isVisible()) { m_sorter->cutSelected(); return; }
    if (m_ed->doc()->pages.size() < 2) return;
    copySlides();
    m_ed->beginChange(tr("Cut Slide"));
    deck::slideRemoved(*m_ed->doc(), m_ed->currentPage());
    m_ed->deletePage(m_ed->currentPage());
    m_ed->endChange();
}

void DeckWindow::pasteSlides(bool keepFormatting)
{
    if (m_sorter->isVisible()) { m_sorter->pasteSlides(keepFormatting); return; }
    if (!deck::slidesOnClipboard()) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Paste Slides"));
    const int at = m_ed->currentPage() + 1;
    const int made = deck::pasteSlidesFromClipboard(*m_ed->doc(), at, keepFormatting);
    m_ed->endChange();
    if (made > 0) m_ed->setCurrentPage(at + made - 1);
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
    const MasterPage *layout = deck::layoutByKey(*d, layoutType, deck::slideMasterOf(*d, i));
    if (!layout) layout = deck::layoutByKey(*d, layoutType);
    if (!layout || i < 0 || i >= d->pages.size()) return;
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Layout"));
    deck::moveToLayout(*d, *d->pages[i], *layout);
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
    m_ed->endCrop(true);   // a crop in progress is a change to ask about, and to save
    if (!m_ed->isModified()) return true;
    const QMessageBox::StandardButton answer = jp::askSaveChanges(this, m_ed->displayName());
    if (answer == QMessageBox::Save) return save();
    if (answer != QMessageBox::Discard) return false;
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
