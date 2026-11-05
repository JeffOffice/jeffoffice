#include "app/writewindow.h"

#include "io/crypto.h"

#include "app/icons.h"
#include "app/keytips.h"
#include "app/writeoutlineview.h"
#include <QSplitter>
#include "app/pageview.h"
#include "app/reviewpane.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/ruler.h"
#include "app/settings.h"
#include "app/spellpane.h"
#include "core/proofops.h"
#include "app/stylespane.h"
#include "app/telemetry.h"
#include "app/widgets.h"
#include "app/writedialogs.h"
#include "app/writefilescreen.h"
#include "core/appinfo.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QTimer>
#include <QUrl>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSlider>
#include <QLocale>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <cmath>
#include <functional>
#include "app/plaintext.h"

namespace jp {

namespace {
int s_untitledCount = 0;
}

WriteWindow::WriteWindow(QWidget *parent) : AppWindow(parent)
{
    setObjectName(QStringLiteral("writeWindow"));
    setAcceptDrops(true);
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
    m_untitled = tr("Document%1").arg(++s_untitledCount);
    m_view = new write::PageView(this);
    connect(m_view, &write::PageView::documentsDropped, this, &WriteWindow::openDropped);
    m_view->setAccessibleName(tr("Document"));
    m_view->setAccessibleDescription(tr("The pages of the document. Type to write; the arrow keys move the caret."));
    m_ribbon = new Ribbon(this);
    setMenuWidget(m_ribbon);
    // The ruler is above the pages.
    auto *central = new QWidget(this);
    auto *column = new QVBoxLayout(central);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);
    m_ruler = new write::Ruler(m_view, central);
    column->addWidget(m_ruler);
    // The pages, with room below them for a second view of the same document (View > Split).
    m_splitter = new QSplitter(Qt::Vertical, central);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->addWidget(m_view);
    m_split = new write::PageView(m_splitter);
    m_split->setObjectName(QStringLiteral("splitView"));
    m_split->setAccessibleName(tr("Document, second pane"));
    m_split->setReadOnly(true);
    m_split->hide();
    m_splitter->addWidget(m_split);
    column->addWidget(m_splitter, 1);
    m_outline = new WriteOutlineView(central);
    m_outline->hide();
    column->addWidget(m_outline, 1);
    setCentralWidget(central);

    createActions();
    buildRibbon();
    buildStatusBar();
    describeForScreenReaders(m_view);
    describeForScreenReaders(statusBar());
    m_fileScreen = new WriteFileScreen(this);
    m_fileScreen->hide();
    connect(m_fileScreen, &WriteFileScreen::closeRequested, this, &WriteWindow::hideFileScreen);
    new KeyTips(this, m_ribbon, [this] { showFileScreen(); });   // Alt, F
    connect(m_ribbon, &Ribbon::fileClicked, this, [this] { showFileScreen(); });
    connect(m_ribbon, &Ribbon::leaveRequested, this, [this] { m_view->setFocus(Qt::OtherFocusReason); });

    m_reviewStateTimer.setSingleShot(true);
    m_reviewStateTimer.setInterval(400);
    connect(&m_reviewStateTimer, &QTimer::timeout, this, [this] {
        updateReviewState();
        refreshReviewUi();
    });
    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(30);
    connect(&m_refreshTimer, &QTimer::timeout, this, &WriteWindow::refreshUi);
    auto queue = [this] { m_refreshTimer.start(); };
    connect(m_view, &write::PageView::caretChanged, this, queue);
    connect(m_view, &write::PageView::contentChanged, this, queue);
    connect(m_view, &write::PageView::pagesChanged, this, queue);
    connect(m_view, &write::PageView::contextMenuWanted, this, &WriteWindow::showContextMenu);
    connect(m_view, &write::PageView::selectionMade, this, &WriteWindow::paintFormatNow);
    connect(m_view, &write::PageView::headerFooterModeChanged, this, &WriteWindow::headerFooterModeChanged);
    connect(m_view, &write::PageView::noteModeChanged, this, queue);
    connect(m_view, &write::PageView::zoomChanged, this, [this](double z) {
        m_zoomLabel->setText(QStringLiteral("%1%").arg(std::lround(z * 100)));
        const QSignalBlocker block(m_zoomSlider);
        m_zoomSlider->setValue(int(std::lround(std::log(z) / std::log(1.05))));
        act("zoom.100")->setChecked(std::abs(z - 1.0) < 0.005 && m_view->zoomMode() == write::PageView::ZoomMode::Percent);
    });
    m_wordTimer.setSingleShot(true);
    m_wordTimer.setInterval(500);
    connect(&m_wordTimer, &QTimer::timeout, this, &WriteWindow::updateWords);

    setDocument(write::WriteDoc::blank(), QString());
    applyDisplayDefaults();
    applyEditingDefaults();
    resize(1280, 800);
    updateTitle();
    refreshUi();
}

WriteWindow::~WriteWindow()
{
    // (The pane's dock is taken down with the window's other children, after this: it must not reach the window's commands then.)
    if (m_navDock) m_navDock->disconnect(this);
    // (The view lets go of the document before the document goes.)
    m_view->setDocument(nullptr);
    crypto::wipe(m_filePassword);
}

QAction *WriteWindow::mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable)
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
        if (isReadOnly() && !commandAllowedReadOnly(id) && !protectionAllows(id)) {
            refuseReadOnlyChange();
            return;
        }
        telemetry::count(id);
        fn();
    });
    m_actions.insert(id, a);
    return a;
}

QAction *WriteWindow::act(const QString &id) const { return m_actions.value(id); }

QStringList WriteWindow::actionIds() const
{
    QStringList ids = m_actions.keys();
    ids.sort();
    return ids;
}

// ---- the document

QString WriteWindow::displayName() const { return m_path.isEmpty() ? m_untitled : QFileInfo(m_path).completeBaseName(); }

bool WriteWindow::isModified() const { return m_doc && m_doc->isModified(); }

void WriteWindow::setDocument(std::unique_ptr<write::WriteDoc> doc, const QString &path)
{
    if (m_outlineOn) setOutlineMode(false);
    m_split->setDocument(nullptr);
    m_view->setDocument(nullptr);
    m_doc = std::move(doc);
    m_path = path;
    if (m_doc) {
        m_doc->filePath = path;
        connect(m_doc.get(), &write::WriteDoc::modifiedChanged, this, [this] { updateTitle(); });
        connect(m_doc.get(), &write::WriteDoc::historyChanged, this, [this] {
            m_refreshTimer.start();
            if (m_outlineOn && !m_outlineBusy) m_outline->rebuild();
        });
        resetEditPlaces();
        connect(m_doc->story(), &QTextDocument::contentsChange, this, [this](int position, int, int added) { recordPlace(position + added); });
    }
    m_view->setDocument(m_doc.get());
    if (m_split->isVisible()) m_split->setDocument(m_doc.get());
    m_view->setZoomMode(write::PageView::ZoomMode::Percent);
    m_view->setZoom(1.0);
    if (m_doc) {
        // Who makes changes and comments here, and how the document shows them: a document with changes or comments opens showing its markup.
        m_doc->setReviewer(Settings::get().userName(), Settings::get().value(QStringLiteral("user/initials")).toString());
        connect(m_doc.get(), &write::WriteDoc::reviewChanged, this, [this] { applyProtection(); });
        if (m_reviewView == ReviewView::NoMarkup || m_reviewView == ReviewView::Original) {
            if (write::revisionCount(m_doc->story()) > 0) setReviewView(ReviewView::AllMarkup);
        }
        applyReviewView();
        if (m_review) m_review->refresh();
        applyProtection();
    }
    if (m_spellPane && m_spellDock->isVisible()) m_spellPane->start();
    updateTitle();
    refreshUi();
    m_wordTimer.start();
}

void WriteWindow::updateTitle()
{
    // A document that came from an old binary file shows its whole name, so its kind is plain until it is saved as a .docx.
    const QString ending = QFileInfo(m_path).suffix().toLower();
    const QString shown = ending == QLatin1String("doc") || ending == QLatin1String("dot") || ending == QLatin1String("rtf") ? QFileInfo(m_path).fileName() : displayName();
    setWindowTitle(QStringLiteral("%1%2%4 - %3").arg(shown, isModified() ? QStringLiteral("*") : QString(), appInfo().name, isReadOnly() ? (m_readOnlyByProtection ? tr(" (Protected)") : tr(" (Read-Only)")) : QString()));
}

// ---- commands

void WriteWindow::createActions()
{
    createFileActions();
    createFormatActions();
    createLayoutActions();
    createViewActions();
    createReviewActions();
    createHelpActions();
    createEditingActions();
    createHeaderFooterActions();
    createNoteActions();
    createFieldActions();
    createRefsActions();
    createTableActions();
    createPictureActions();
    createShapeActions();
    createPictureAdjustActions();
    createInsertActions();
    createLayoutExtraActions();
    createViewExtraActions();
    createOutlineActions();
    createStyleActions();
    createHomeActions();
    createProtectActions();
    createMailActions();
}

namespace {

// Runs a command on the document and the view's cursor, and puts the keyboard back in the text.
struct Edit {
    WriteWindow *w;
    void operator()(const std::function<void(write::WriteDoc &, QTextCursor &)> &fn) const
    {
        if (!w->document()) return;
        fn(*w->document(), w->view()->cursor());
        w->view()->setFocus(Qt::OtherFocusReason);
        w->view()->viewport()->update();
    }
};

} // namespace

void WriteWindow::createFormatActions()
{
    const Edit edit{this};
    write::PageView *v = m_view;
    mk("edit.undo", tr("Undo"), "undo-2", QKeySequence::Undo, [v] { v->undo(); });
    mk("edit.redo", tr("Redo"), "redo-2", QKeySequence::Redo, [v] { v->redo(); });
    mk("edit.cut", tr("Cut"), "scissors", QKeySequence::Cut, [v] { v->cut(); });
    mk("edit.copy", tr("Copy"), "copy", QKeySequence::Copy, [v] { v->copy(); });
    mk("edit.paste", tr("Paste"), "clipboard-paste", QKeySequence::Paste, [v] { v->paste(); });
    mk("edit.selectAll", tr("Select All"), "", QKeySequence::SelectAll, [v] { v->selectAll(); });

    mk("fmt.bold", tr("Bold"), "bold", QKeySequence::Bold, [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleBold(d, c); }); }, true);
    mk("fmt.italic", tr("Italic"), "italic", QKeySequence::Italic, [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleItalic(d, c); }); }, true);
    mk("fmt.underline", tr("Underline"), "underline", QKeySequence::Underline, [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleUnderline(d, c); }); }, true);
    mk("fmt.strike", tr("Strikethrough"), "strikethrough", QKeySequence(), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleStrike(d, c); }); }, true);
    mk("fmt.subscript", tr("Subscript"), "subscript", QKeySequence(Qt::CTRL | Qt::Key_Equal), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleSubscript(d, c); }); }, true);
    mk("fmt.superscript", tr("Superscript"), "superscript", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleSuperscript(d, c); }); }, true);
    mk("fmt.charBorder", tr("Character Border"), "square", QKeySequence(), [edit] {
        edit([](write::WriteDoc &d, QTextCursor &c) {
            const bool on = !write::allSelected(c, [](const QTextCharFormat &f) { return f.hasProperty(tp::RunBorder); });
            write::setCharBorder(d, c, on);
        });
    }, true);
    mk("fmt.grow", tr("Increase Font Size"), "a-arrow-up", QKeySequence(Qt::CTRL | Qt::Key_BracketRight), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::changeFontSize(d, c, 1); }); });
    mk("fmt.shrink", tr("Decrease Font Size"), "a-arrow-down", QKeySequence(Qt::CTRL | Qt::Key_BracketLeft), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::changeFontSize(d, c, -1); }); });
    mk("fmt.clear", tr("Clear All Formatting"), "remove-formatting", QKeySequence(Qt::CTRL | Qt::Key_Space), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::clearFormatting(d, c); }); });

    mk("fmt.alignLeft", tr("Align Left"), "align-left", QKeySequence(Qt::CTRL | Qt::Key_L), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::setAlignment(d, c, Qt::AlignLeft); }); }, true);
    mk("fmt.alignCenter", tr("Center"), "align-center", QKeySequence(Qt::CTRL | Qt::Key_E), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::setAlignment(d, c, Qt::AlignHCenter); }); }, true);
    mk("fmt.alignRight", tr("Align Right"), "align-right", QKeySequence(Qt::CTRL | Qt::Key_R), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::setAlignment(d, c, Qt::AlignRight); }); }, true);
    mk("fmt.justify", tr("Justify"), "align-justify", QKeySequence(Qt::CTRL | Qt::Key_J), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::setAlignment(d, c, Qt::AlignJustify); }); }, true);
    mk("para.bullets", tr("Bullets"), "list", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleList(d, c, false); }); }, true);
    mk("para.numbers", tr("Numbering"), "list-ordered", QKeySequence(), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::toggleList(d, c, true); }); }, true);
    mk("para.indentInc", tr("Increase Indent"), "indent-increase", QKeySequence(Qt::CTRL | Qt::Key_M), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::changeIndent(d, c, 36); }); });
    mk("para.indentDec", tr("Decrease Indent"), "indent-decrease", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::changeIndent(d, c, -36); }); });
    struct Spacing { const char *id; const char *text; double value; };
    for (const Spacing &s : {Spacing{"ls.1", "1.0", 1.0}, Spacing{"ls.1.15", "1.15", 1.15}, Spacing{"ls.1.5", "1.5", 1.5}, Spacing{"ls.2", "2.0", 2.0}, Spacing{"ls.2.5", "2.5", 2.5}, Spacing{"ls.3", "3.0", 3.0}}) {
        const double value = s.value;
        mk(QString::fromLatin1(s.id), QString::fromLatin1(s.text), "", QKeySequence(), [edit, value] {
            edit([value](write::WriteDoc &d, QTextCursor &c) { write::setLineSpacing(d, c, QTextBlockFormat::ProportionalHeight, value); });
        });
    }
    mk("fmt.fontDialog", tr("Font…"), "type", QKeySequence(Qt::CTRL | Qt::Key_D), [this] { showFontDialog(); });
    mk("para.dialog", tr("Paragraph…"), "pilcrow", QKeySequence(), [this] { showParagraphDialog(); });
    mk("insert.pageBreak", tr("Page Break"), "file-plus-2", QKeySequence(Qt::CTRL | Qt::Key_Return), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::insertPageBreak(d, c); }); });
    mk("layout.sectionBreak", tr("Section Break (Next Page)"), "section", QKeySequence(), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::insertSectionBreak(d, c); }); });

    // Styles by key: Normal, and Heading 1 to 3.
    mk("style.normal", tr("Normal"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N), [edit] { edit([](write::WriteDoc &d, QTextCursor &c) { write::applyParagraphStyle(d, c, d.defaultParagraphStyle()); }); });
    for (int level = 1; level <= 3; ++level)
        mk(QStringLiteral("style.heading%1").arg(level), tr("Heading %1").arg(level), "", QKeySequence(Qt::CTRL | Qt::ALT | (Qt::Key_0 + level)), [edit, level] {
            edit([level](write::WriteDoc &d, QTextCursor &c) { write::applyParagraphStyle(d, c, QStringLiteral("Heading%1").arg(level)); });
        });
}

void WriteWindow::changeSection(const std::function<void(flow::Section &)> &change)
{
    if (!m_doc) return;
    const int index = m_view->cursorSection();
    office::wml::SectionProps props = m_doc->sectionProps(index);
    flow::Section s = flow::Section(m_doc->flow().section(index));
    const flow::Section was = s;
    change(s);
    s.margins = QMarginsF(std::max(0.0, s.margins.left()), std::max(0.0, s.margins.top()), std::max(0.0, s.margins.right()), std::max(0.0, s.margins.bottom()));
    write::updateSectionProps(&props, s);
    Q_UNUSED(was);
    m_doc->setSectionProps(index, props);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::createLayoutActions()
{
    struct Margins { const char *id; const char *text; double top, right, bottom, left; };
    for (const Margins &m : {Margins{"normal", QT_TRANSLATE_NOOP("jp::WriteWindow", "Normal"), 72, 72, 72, 72}, Margins{"narrow", QT_TRANSLATE_NOOP("jp::WriteWindow", "Narrow"), 36, 36, 36, 36}, Margins{"moderate", QT_TRANSLATE_NOOP("jp::WriteWindow", "Moderate"), 72, 54, 72, 54},
                             Margins{"wide", QT_TRANSLATE_NOOP("jp::WriteWindow", "Wide"), 72, 144, 72, 144}}) {
        const double t = m.top, r = m.right, b = m.bottom, l = m.left;
        mk(QStringLiteral("layout.margins.") + QLatin1String(m.id), QCoreApplication::translate("jp::WriteWindow", m.text), "", QKeySequence(), [this, t, r, b, l] {
            changeSection([=](flow::Section &s) { s.margins = QMarginsF(l, t, r, b); });
        });
    }
    auto orient = [this](bool landscape) {
        changeSection([landscape](flow::Section &s) {
            const bool now = s.orientation == flow::Section::Orientation::Landscape;
            if (now == landscape) return;
            const QMarginsF m = s.margins;
            // The margins turn with the page: the top becomes the left (to landscape), and back.
            s.margins = landscape ? QMarginsF(m.bottom(), m.left(), m.top(), m.right()) : QMarginsF(m.top(), m.right(), m.bottom(), m.left());
            s.orientation = landscape ? flow::Section::Orientation::Landscape : flow::Section::Orientation::Portrait;
        });
    };
    mk("layout.portrait", tr("Portrait"), "", QKeySequence(), [orient] { orient(false); }, true);
    mk("layout.landscape", tr("Landscape"), "", QKeySequence(), [orient] { orient(true); }, true);
    mk("layout.pageSetup", tr("Page Setup…"), "settings-2", QKeySequence(), [this] { showPageSetupDialog(); });
    for (int n = 1; n <= 3; ++n)
        mk(QStringLiteral("layout.columns.%1").arg(n), n == 1 ? tr("One") : n == 2 ? tr("Two") : tr("Three"), n == 1 ? "square" : n == 2 ? "columns-2" : "columns-3", QKeySequence(), [this, n] {
            changeSection([n](flow::Section &s) {
                s.columns = n;
                s.columnGap = 36;
                s.columnWidths.clear();
                s.columnSpaces.clear();
            });
        }, true);
    struct Paper { const char *id; const char *text; double w, h; };
    for (const Paper &p : {Paper{"letter", QT_TRANSLATE_NOOP("jp::WriteWindow", "Letter"), 612, 792}, Paper{"legal", QT_TRANSLATE_NOOP("jp::WriteWindow", "Legal"), 612, 1008}, Paper{"executive", QT_TRANSLATE_NOOP("jp::WriteWindow", "Executive"), 522, 756}, Paper{"a3", QT_TRANSLATE_NOOP("jp::WriteWindow", "A3"), 841.9, 1190.55},
                           Paper{"a4", QT_TRANSLATE_NOOP("jp::WriteWindow", "A4"), 595.3, 841.9}, Paper{"a5", QT_TRANSLATE_NOOP("jp::WriteWindow", "A5"), 419.55, 595.3}, Paper{"b5", QT_TRANSLATE_NOOP("jp::WriteWindow", "B5"), 498.95, 708.75},
                           Paper{"envelope10", QT_TRANSLATE_NOOP("jp::WriteWindow", "Envelope #10"), 297, 684}, Paper{"envelopeMonarch", QT_TRANSLATE_NOOP("jp::WriteWindow", "Envelope Monarch"), 279, 540},
                           Paper{"envelopeDL", QT_TRANSLATE_NOOP("jp::WriteWindow", "Envelope DL"), 311.81, 623.62}, Paper{"envelopeC5", QT_TRANSLATE_NOOP("jp::WriteWindow", "Envelope C5"), 459.21, 649.13}}) {
        const double w = p.w, h = p.h;
        mk(QStringLiteral("layout.size.") + QLatin1String(p.id), QCoreApplication::translate("jp::WriteWindow", p.text), "", QKeySequence(), [this, w, h] {
            changeSection([w, h](flow::Section &s) { s.paper = QSizeF(w, h); });
        }, true);
    }
}

void WriteWindow::createViewActions()
{
    write::PageView *v = m_view;
    mk("view.printLayout", tr("Print Layout"), "file-text", QKeySequence(), [] {}, true)->setChecked(true);
    mk("view.marks", tr("Show/Hide ¶"), "pilcrow", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_8), [this] { m_view->setShowMarks(act("view.marks")->isChecked()); }, true);
    mk("view.ruler", tr("Ruler"), "ruler", QKeySequence(), [this] { m_ruler->setVisible(act("view.ruler")->isChecked()); }, true)->setChecked(true);
    mk("zoom.in", tr("Zoom In"), "zoom-in", QKeySequence(), [v] { v->setZoom(std::round(v->zoom() * 10 + 1) / 10); });
    mk("zoom.out", tr("Zoom Out"), "zoom-out", QKeySequence(), [v] { v->setZoom(std::round(v->zoom() * 10 - 1) / 10); });
    for (int pct : {200, 150, 100, 75, 50, 25, 10}) {
        mk(QStringLiteral("zoom.%1").arg(pct), QStringLiteral("%1%").arg(pct), pct == 100 ? "zoom-in" : "", QKeySequence(), [v, pct] { v->setZoom(pct / 100.0); }, pct == 100);
    }
    act("zoom.100")->setText(tr("100%"));
    mk("zoom.onePage", tr("One Page"), "file", QKeySequence(), [v] { v->setZoomMode(write::PageView::ZoomMode::OnePage); });
    mk("zoom.multiplePages", tr("Multiple Pages"), "files", QKeySequence(), [v] { v->setZoomMode(write::PageView::ZoomMode::ManyPages, 2, 1); });
    mk("zoom.pageWidth", tr("Page Width"), "maximize-2", QKeySequence(), [v] { v->setZoomMode(write::PageView::ZoomMode::PageWidth); });
    mk("zoom.dialog", tr("Zoom…"), "zoom-in", QKeySequence(), [this] { showZoomDialog(); });
}

void WriteWindow::showFontDialog()
{
    if (!m_doc) return;
    write::fontDialog(this, m_doc.get(), &m_view->cursor());
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::showParagraphDialog()
{
    if (!m_doc) return;
    write::paragraphDialog(this, m_doc.get(), &m_view->cursor());
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::showZoomDialog()
{
    write::zoomDialog(this, m_view);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::showContextMenu(const QPoint &globalPos)
{
    QMenu menu(this);
    fillContextMenu(menu);
    menu.exec(globalPos);
}

void WriteWindow::fillContextMenu(QMenu &menu)
{
    write::addSpellingSuggestions(menu, m_view);
    for (const char *id : {"edit.cut", "edit.copy", "edit.paste"}) menu.addAction(act(QString::fromLatin1(id)));
    // The ways to paste: with the source's formatting, merged into the text round it, or as text only.
    QMenu *paste = menu.addMenu(tr("Paste Options"));
    for (const char *id : {"edit.paste", "edit.pasteMerge", "edit.pasteText"}) paste->addAction(act(QString::fromLatin1(id)));
    menu.addSeparator();
    menu.addAction(act("fmt.fontDialog"));
    menu.addAction(act("para.dialog"));
    QMenu *lists = menu.addMenu(tr("Bullets and Numbering"));
    lists->addAction(act("para.bullets"));
    lists->addAction(act("para.numbers"));
    menu.addAction(act("edit.selectAll"));
    addLinkFieldNoteContextMenu(menu);
    // On a picture: how the text goes round it, and its alt text.
    if (m_view->selectedPicture() >= 0) addPictureContextMenu(menu);
    // In a table: its rows and columns, merging, lines, size, and properties.
    if (cursorInTable()) addTableContextMenu(menu);
    addReviewContextMenu(menu);
}

// ---- keeping the controls in step with the text

void WriteWindow::refreshUi()
{
    updateStatus();
    refreshReviewUi();
    const bool has = m_doc != nullptr;
    if (m_styles && m_stylesDock->isVisible()) m_styles->refresh();
    for (QAction *a : m_actions) {
        const QString id = a->objectName();
        if (id.startsWith(QLatin1String("fmt.")) || id.startsWith(QLatin1String("para.")) || id.startsWith(QLatin1String("ls.")) || id.startsWith(QLatin1String("layout.")) ||
            id.startsWith(QLatin1String("insert.")) || id.startsWith(QLatin1String("refs.")) || id.startsWith(QLatin1String("style.")) || id == QLatin1String("edit.formatPainter") || id == QLatin1String("edit.copyFormat") ||
            id == QLatin1String("edit.pasteFormat") || id == QLatin1String("edit.goTo"))
            a->setEnabled(has);
    }
    if (!has) {
        for (const char *id : {"refs.updateTable", "refs.removeTable", "refs.nextFootnote", "refs.previousFootnote", "refs.nextEndnote", "refs.previousEndnote", "fields.update", "fields.unlink", "fields.lock", "fields.unlock", "fields.toggleCode", "fields.toggleAllCodes"}) act(QString::fromLatin1(id))->setEnabled(false);
        refreshTableUi();
        refreshPictureUi();
        refreshMailUi();
        refreshInsertUi();
        refreshLayoutUi();
        refreshViewUi();
        return;
    }
    QTextCursor &c = m_view->cursor();
    act("edit.undo")->setEnabled(m_doc->canUndo());
    act("edit.redo")->setEnabled(m_doc->canRedo());
    for (const char *id : {"edit.cut", "edit.copy"}) act(QString::fromLatin1(id))->setEnabled(c.hasSelection());
    // The look of the selection (of its start, when it is long): checked when all of it has it.
    const bool shortSelection = c.selectionEnd() - c.selectionStart() < 20000;
    auto all = [&](const std::function<bool(const QTextCharFormat &)> &test) {
        return shortSelection ? write::allSelected(c, test) : test(c.charFormat());
    };
    act("fmt.bold")->setChecked(all([](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
    act("fmt.italic")->setChecked(all([](const QTextCharFormat &f) { return f.fontItalic(); }));
    act("fmt.underline")->setChecked(all([](const QTextCharFormat &f) { return f.underlineStyle() != QTextCharFormat::NoUnderline; }));
    act("fmt.strike")->setChecked(all([](const QTextCharFormat &f) { return f.fontStrikeOut(); }));
    act("fmt.subscript")->setChecked(all([](const QTextCharFormat &f) { return f.verticalAlignment() == QTextCharFormat::AlignSubScript; }));
    act("fmt.charBorder")->setChecked(all([](const QTextCharFormat &f) { return f.hasProperty(tp::RunBorder); }));
    act("fmt.superscript")->setChecked(all([](const QTextCharFormat &f) { return f.verticalAlignment() == QTextCharFormat::AlignSuperScript; }));
    const QTextBlock block = c.block();
    const Qt::Alignment al = block.blockFormat().alignment() & Qt::AlignHorizontal_Mask;
    act("fmt.alignLeft")->setChecked(al == Qt::AlignLeft || al == 0 || al == Qt::AlignLeading);
    act("fmt.alignCenter")->setChecked(al == Qt::AlignHCenter);
    act("fmt.alignRight")->setChecked(al == Qt::AlignRight || al == Qt::AlignTrailing);
    act("fmt.justify")->setChecked(al == Qt::AlignJustify);
    const write::ListKind kind = write::listKindOf(*m_doc, block);
    act("para.bullets")->setChecked(kind == write::ListKind::Bullets);
    act("para.numbers")->setChecked(kind == write::ListKind::Numbers);

    // The font and size boxes, and the colors the pickers offer.
    QTextCharFormat cf = c.charFormat();
    if (write::isAtom(cf) || cf.fontPointSize() <= 0) cf = block.charFormat();
    const FontScheme &fonts = m_doc->flow().env.fonts;
    QString family;
    if (cf.hasProperty(QTextFormat::FontFamilies) && !cf.fontFamilies().toStringList().isEmpty()) family = cf.fontFamilies().toStringList().first();
    else family = cf.stringProperty(tp::ThemeFont) == QLatin1String("major") ? fonts.heading : fonts.body;
    for (FontCombo *f : m_fontCombos) {
        f->setSchemeFonts(fonts.heading, fonts.body);
        f->setCurrentFamily(family);
    }
    for (SizeCombo *s : m_sizeCombos) s->setSize(cf.fontPointSize() > 0 ? cf.fontPointSize() : 11);
    for (ColorButton *b : m_colorButtons) b->setScheme(m_doc->flow().env.colors);

    // Page setup of the section the caret is in.
    const int section = m_view->cursorSection();
    const flow::Section s = m_doc->flow().section(section);
    act("layout.portrait")->setChecked(s.orientation == flow::Section::Orientation::Portrait);
    act("layout.landscape")->setChecked(s.orientation == flow::Section::Orientation::Landscape);
    for (int n = 1; n <= 3; ++n) act(QStringLiteral("layout.columns.%1").arg(n))->setChecked(s.columns == n);
    for (const char *id : {"letter", "legal", "executive", "a3", "a4", "a5", "b5", "envelope10", "envelopeMonarch", "envelopeDL", "envelopeC5"}) {
        QAction *a = act(QStringLiteral("layout.size.") + QLatin1String(id));
        const QSizeF paper = s.paper;
        // (Each size by its paper, to a point.)
        static const QHash<QString, QSizeF> sizes = {{"letter", QSizeF(612, 792)}, {"legal", QSizeF(612, 1008)}, {"executive", QSizeF(522, 756)}, {"a3", QSizeF(841.9, 1190.55)},
                                                     {"a4", QSizeF(595.3, 841.9)}, {"a5", QSizeF(419.55, 595.3)}, {"b5", QSizeF(498.95, 708.75)},
                                                     {"envelope10", QSizeF(297, 684)}, {"envelopeMonarch", QSizeF(279, 540)}, {"envelopeDL", QSizeF(311.81, 623.62)}, {"envelopeC5", QSizeF(459.21, 649.13)}};
        const QSizeF want = sizes.value(QString::fromLatin1(id));
        a->setChecked(std::abs(paper.width() - want.width()) < 2 && std::abs(paper.height() - want.height()) < 2);
    }
    m_updatingFields = true;
    if (m_indentFields.size() >= 4) {
        const QTextBlockFormat bf = block.blockFormat();
        m_indentFields[0]->setPoints(bf.leftMargin());
        m_indentFields[1]->setPoints(bf.rightMargin());
        m_indentFields[2]->setPoints(bf.topMargin());
        m_indentFields[3]->setPoints(bf.bottomMargin());
    }
    m_updatingFields = false;
    // The Header & Footer tab: its options for the section and the slot being edited.
    {
        const bool editing = m_view->inHeaderFooter();
        const write::PageView::HeaderFooterEdit &e = m_view->headerFooterEdit();
        for (const char *id : {"hf.close", "hf.pageNumber", "hf.pageNumberFormat", "hf.alignmentTab", "hf.dateTime", "hf.removeHeader", "hf.removeFooter", "hf.differentFirst", "hf.differentOddEven", "hf.showDocumentText"}) act(QString::fromLatin1(id))->setEnabled(editing);
        // (A header, a footer, or a note takes no break of a page or a section.)
        if (editing || m_view->inNote())
            for (const char *id : {"insert.pageBreak", "layout.sectionBreak"}) act(QString::fromLatin1(id))->setEnabled(false);
        // (And no note.)
        const bool inNoteWords = write::inEndnoteWords(c);
        const bool noteAllowed = !editing && !m_view->inNote() && !inNoteWords;
        // (A document with the most notes it holds: the commands are off, and the status bar says why.)
        const bool footnoteRoom = !noteAllowed || m_doc->roomForFootnote(), endnoteRoom = !noteAllowed || m_doc->roomForEndnote();
        const QString full = tr("This document has the most notes JeffWrite can hold (%1), so no more can be added.").arg(QLocale().toString(write::kMostNoteStories));
        // (The commands for the fields have a field to work on: one the caret is in, or the selection takes in; Alt+F9, a field in the text.)
        {
            const bool inText = !editing && !m_view->inNote();
            const write::FieldsNear near = inText ? write::fieldsNearSelection(m_doc->story(), c) : write::FieldsNear();
            act("fields.unlink")->setEnabled(near.any);
            act("fields.toggleCode")->setEnabled(near.any);
            act("refs.updateTable")->setEnabled(near.toc);
            act("refs.removeTable")->setEnabled(near.toc);
            act("refs.updateIndex")->setEnabled(near.index);
            act("refs.updateAuthorities")->setEnabled(near.authorities);
            for (const char *id : {"refs.tocAutomatic1", "refs.tocAutomatic2", "refs.tocManual", "refs.addText.none", "refs.addText.1", "refs.addText.2", "refs.addText.3", "refs.markTocEntry", "refs.insertCaption",
                                   "refs.tableOfFigures", "refs.insertCitation", "refs.bibliography.bibliography", "refs.bibliography.references", "refs.bibliography.worksCited", "refs.markEntry", "refs.insertIndex",
                                   "refs.markCitation", "refs.insertAuthorities"})
                act(QString::fromLatin1(id))->setEnabled(inText);
            act("fields.lock")->setEnabled(near.unlocked);
            act("fields.unlock")->setEnabled(near.locked);
            act("fields.toggleAllCodes")->setEnabled(inText && m_doc->mayHaveFields());
            act("fields.update")->setEnabled(inText && m_doc->mayHaveFields());
            // (Next and Previous Footnote and Endnote have a note to go to in that direction.)
            const int here = c.position();
            act("refs.nextFootnote")->setEnabled(inText && write::nextNoteReference(*m_doc, here, true, false) >= 0);
            act("refs.previousFootnote")->setEnabled(inText && write::nextNoteReference(*m_doc, here, false, false) >= 0);
            act("refs.nextEndnote")->setEnabled(inText && write::nextNoteReference(*m_doc, here, true, true) >= 0);
            act("refs.previousEndnote")->setEnabled(inText && write::nextNoteReference(*m_doc, here, false, true) >= 0);
        }
        act("refs.footnote")->setEnabled(noteAllowed && footnoteRoom);
        act("refs.endnote")->setEnabled(noteAllowed && endnoteRoom);
        act("refs.footnote")->setStatusTip(jp::plainToolTip(footnoteRoom ? QString() : full));
        act("refs.endnote")->setStatusTip(jp::plainToolTip(endnoteRoom ? QString() : full));
        act("refs.footnote")->setToolTip(jp::plainToolTip(footnoteRoom ? QString() : full));
        act("refs.endnote")->setToolTip(jp::plainToolTip(endnoteRoom ? QString() : full));
        if (!footnoteRoom || !endnoteRoom) {
            if (!m_noteLimitShown) statusBar()->showMessage(full, 8000);
            m_noteLimitShown = true;
        } else {
            m_noteLimitShown = false;
        }
        act("hf.differentFirst")->setChecked(editing && m_doc->differentFirstPage(e.section));
        act("hf.differentOddEven")->setChecked(m_doc->differentOddEven());
        act("hf.previousSection")->setEnabled(editing && e.section > 0);
        act("hf.nextSection")->setEnabled(editing && e.section < m_doc->sectionCount() - 1);
        act("hf.goToHeader")->setEnabled(editing && e.footer);
        act("hf.goToFooter")->setEnabled(editing && !e.footer);
        act("hf.linkPrevious")->setEnabled(editing && e.section > 0);
        act("hf.linkPrevious")->setChecked(editing && m_doc->linkedToPrevious(e.section, e.footer, e.slot));
        if (editing && m_hfFields.size() >= 2) {
            const office::wml::SectionProps sp = m_doc->sectionProps(e.section);
            m_updatingFields = true;
            m_hfFields[0]->setPoints(sp.header / 20.0);
            m_hfFields[1]->setPoints(sp.footer / 20.0);
            m_updatingFields = false;
        }
    }
    refreshTableUi();
    refreshPictureUi();
    refreshMailUi();
    refreshInsertUi();
    refreshLayoutUi();
    refreshViewUi();
}

void WriteWindow::updateStatus()
{
    if (!m_pageLabel) return;
    const int pages = m_view->pageCount();
    const QString total = m_view->pagesComplete() ? QString::number(pages) : QStringLiteral("%1+").arg(pages);
    m_pageLabel->setText(tr("Page %1 of %2").arg(m_doc ? m_view->currentPage() + 1 : 0).arg(total));
    if (m_sectionLabel) {
        const bool several = m_doc && m_doc->sectionCount() > 1;
        m_sectionLabel->setVisible(several);
        if (several) m_sectionLabel->setText(tr("Section %1").arg(m_view->cursorSection() + 1));
    }
    updateLanguage();
    if (m_doc && !m_wordTimer.isActive()) m_wordTimer.start();
}

// The words, counted after typing has paused (a very long document is not counted: its count would hold up the typing).
void WriteWindow::updateWords()
{
    if (!m_wordLabel) return;
    if (!m_doc || m_doc->story()->characterCount() > 400000) {
        m_wordLabel->clear();
        return;
    }
    // (Text that is hidden is not counted: what a tracked change deleted is hidden in every view.)
    QString text;
    for (QTextBlock b = m_doc->story()->begin(); b.isValid(); b = b.next()) {
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
            if (!it.fragment().charFormat().boolProperty(tp::HiddenText)) text += it.fragment().text();
        text += QLatin1Char('\n');
    }
    text.remove(QChar::ObjectReplacementCharacter);
    int words = 0;
    bool in = false;
    for (const QChar c : std::as_const(text)) {
        const bool space = c.isSpace();
        if (!space && !in) ++words;
        in = !space;
    }
    // With words selected: "12 of 340 words".
    const QTextCursor sel = m_view->cursor();
    if (sel.hasSelection() && sel.document() == m_doc->story()) {
        const int selected = write::countWordsIn(sel.document(), sel.selectionStart(), sel.selectionEnd(), false).words;
        if (selected > 0) {
            m_wordLabel->setText(tr("%1 of %n word(s)", nullptr, words).arg(selected));
            return;
        }
    }
    m_wordLabel->setText(tr("%n word(s)", nullptr, words));
}

// ---- the File screen

void WriteWindow::showFileScreen(const QString &page)
{
    m_view->setFocus();
    m_fileScreen->setGeometry(rect());
    m_fileScreen->showPage(page.isEmpty() ? QStringLiteral("open") : page);
    m_fileScreen->show();
    m_fileScreen->raise();
    m_fileScreen->setFocus();
    qApp->installEventFilter(this);
}

void WriteWindow::hideFileScreen()
{
    if (!m_fileScreen->isVisible()) return;
    qApp->removeEventFilter(this);
    m_fileScreen->hide();
    m_view->setFocus();
}

void WriteWindow::resizeEvent(QResizeEvent *e)
{
    AppWindow::resizeEvent(e);
    if (m_fileScreen) m_fileScreen->setGeometry(rect());
}

bool WriteWindow::eventFilter(QObject *o, QEvent *e)
{
    // A double click on the Copy Formatting button keeps it on; Esc turns it off.
    if (e->type() == QEvent::MouseButtonDblClick) {
        auto *button = qobject_cast<QToolButton *>(o);
        if (button && button->defaultAction() && button->defaultAction() == act(QStringLiteral("edit.formatPainter")) && button->window() == this) {
            setPainterMode(PainterMode::Locked);
            return true;
        }
    }
    if (e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape && m_painter != PainterMode::Off) {
        auto *w = qobject_cast<QWidget *>(o);
        if (w && w->window() == this) {
            setPainterMode(PainterMode::Off);
            return true;
        }
    }
    // Escape leaves Focus mode.
    if (e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape && m_focusMode) {
        auto *w = qobject_cast<QWidget *>(o);
        if (w && w->window() == this) {
            setFocusMode(false);
            return true;
        }
    }
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

bool WriteWindow::maybeSave()
{
    if (!isModified()) return true;
    QMessageBox box(QMessageBox::Question, appInfo().name, tr("Do you want to save changes to %1?").arg(displayName()), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
    box.setTextFormat(Qt::PlainText);
    box.setDefaultButton(QMessageBox::Save);
    box.button(QMessageBox::Discard)->setText(tr("Don't Save"));
    box.exec();
    if (box.clickedButton() == box.button(QMessageBox::Save)) return save();
    if (box.clickedButton() != box.button(QMessageBox::Discard)) return false;
    dropRecoveryCopy();
    return true;
}

void WriteWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (!write::PageView::documentFilesIn(e->mimeData()).isEmpty()) e->acceptProposedAction();
    else e->ignore();
}

void WriteWindow::dropEvent(QDropEvent *e)
{
    const QStringList paths = write::PageView::documentFilesIn(e->mimeData());
    if (paths.isEmpty()) {
        e->ignore();
        return;
    }
    e->acceptProposedAction();
    openDropped(paths);
}

void WriteWindow::openDropped(const QStringList &paths)
{
    // (After the drop is over: opening can show a message, which must not run inside the drag.)
    QTimer::singleShot(0, this, [this, paths] {
        for (const QString &p : paths) openFile(p);
    });
}

void WriteWindow::closeEvent(QCloseEvent *e)
{
    if (!maybeSave()) {
        e->ignore();
        return;
    }
    rememberPlace();
    e->accept();
}

} // namespace jp
