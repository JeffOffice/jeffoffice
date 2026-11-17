// The keyboard, the mouse, the input method, and the clipboard in the page view.

#include "app/pageview.h"
#include "app/shapetextedit.h"

#include "core/autoops.h"
#include "core/contentcontrolops.h"
#include "core/editops.h"
#include "core/linkops.h"
#include "app/settings.h"
#include "core/homeops.h"
#include "core/insertops.h"
#include <functional>
#include <optional>
#include <QMenu>
#include <QToolButton>
#include "core/refops.h"
#include "core/tableops.h"
#include "core/writeprops.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDateTime>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPointer>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <algorithm>

namespace jp::write {

namespace {

// The copy made in this program (from any of its documents): pasting it again keeps every format, and carries pictures and hyperlinks over.
Clip s_copied;
QString s_copiedText;
bool s_copiedWhole = true;   // the words last copied from here were whole words (smart paste adds spaces only for those)

} // namespace

// ---- the editing commands

void PageView::undo()
{
    if (m_shapeEditor) {
        m_shapeEditor->undo();
        return;
    }
    if (!m_doc) return;
    m_pictureDrag = PictureDrag();   // (a drag of a picture is over: what it would have done is not done)
    viewport()->update();
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();   // (an empty one shown to be typed in has no step to take back)
    QTextCursor at;
    const int picture = selectedPicture();
    if (QTextDocument *story = m_doc->undo(&at)) {
        followHistory(story, at);
        // (A picture that was selected stays selected through a step that changed it.)
        if (picture >= 0 && selectedPicture() < 0 && m_cursor.position() >= picture && m_cursor.position() <= picture + 1) selectPicture(picture);
    }
}

void PageView::redo()
{
    if (m_shapeEditor) {
        m_shapeEditor->redo();
        return;
    }
    if (!m_doc) return;
    m_pictureDrag = PictureDrag();   // (a drag of a picture is over: what it would have done is not done)
    viewport()->update();
    if (m_hf.active && m_doc->isPendingHeaderFooter(m_hf.story)) leaveHeaderFooter();
    QTextCursor at;
    const int picture = selectedPicture();
    if (QTextDocument *story = m_doc->redo(&at)) {
        followHistory(story, at);
        // (A picture that was selected stays selected through a step that changed it.)
        if (picture >= 0 && selectedPicture() < 0 && m_cursor.position() >= picture && m_cursor.position() <= picture + 1) selectPicture(picture);
    }
}

void PageView::copy()
{
    if (m_shapeEditor) {
        m_shapeEditor->copy();
        return;
    }
    if (!m_doc || !m_cursor.hasSelection()) return;
    const Clip clip = copySelection(*m_doc, m_cursor);
    const QTextDocumentFragment &f = clip.fragment;
    auto *mime = new QMimeData;
    // (A field is the text it shows on the page being edited, or the one the caret is on.)
    FieldContext shown;
    shown.doc = m_doc->flow().fields;
    {
        flow::Paginator &pg = m_doc->pager();
        const int page = std::clamp(currentPage(), 0, std::max(0, pg.pagesKnown() - 1));
        shown.pageNumber = pg.pagesKnown() > 0 ? pg.page(page).number : 1;
        shown.pageCount = std::max(1, pg.pageCount());
    }
    const QString text = plainTextOf(f, &shown);
    mime->setText(text);
    mime->setHtml(f.toHtml());
    mime->setData(QStringLiteral("application/x-jeffwrite-fragment"), text.toUtf8());
    s_copied = clip;
    s_copiedText = text;
    s_copiedWhole = selectionIsWholeWords(m_cursor);
    QApplication::clipboard()->setMimeData(mime);
}

// Smart cut and paste (Options > Advanced): on unless turned off.
bool PageView::smartCutPaste() { return Settings::get().value(QStringLiteral("edit/smartCutPaste"), true).toBool(); }

void PageView::cut()
{
    if (m_shapeEditor) {
        m_shapeEditor->cut();
        return;
    }
    if (!m_doc || !m_cursor.hasSelection()) return;
    copy();
    if (smartCutPaste()) smartRemoveSelection(*m_doc, m_cursor);
    else removeSelection(*m_doc, m_cursor);
    caretMoved();
}

static const QString &fragmentFormat()
{
    static const QString f = QStringLiteral("application/x-jeffwrite-fragment");
    return f;
}

bool PageView::PasteSource::ours() const
{
    return mime && mime->hasFormat(fragmentFormat()) && QString::fromUtf8(mime->data(fragmentFormat())) == clipText && !clip.isEmpty();
}

// What the clipboard holds now, as a paste goes by it. (Not a copy: a paste that is made note of copies what it was made of.)
PageView::PasteSource PageView::clipboardSource() const
{
    PasteSource out;
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime) return out;
    out.mime = std::shared_ptr<const QMimeData>(mime, [](const QMimeData *) {});
    out.clip = s_copied;
    out.clipText = s_copiedText;
    out.clipWhole = s_copiedWhole;
    if (!m_clipCells || m_clipCellsOf != mime) {
        m_clipCells = std::make_shared<CellsOfClip>();
        m_clipCellsOf = mime;
    }
    out.cells = m_clipCells;
    if (!m_clipFetched || m_clipFetchedOf != mime) {
        m_clipFetched = std::make_shared<Fetched>();
        m_clipFetchedOf = mime;
    }
    out.fetched = m_clipFetched;
    return out;
}

const std::optional<office::wml::ClipTable> &PageView::cellsOf(const PasteSource &from) const
{
    static const std::optional<office::wml::ClipTable> none;
    if (!from.mime || !from.cells || from.mime->hasFormat(fragmentFormat())) return none;
    if (!from.cells->parsed) {
        from.cells->parsed = true;
        from.cells->cells = office::wml::clipTable(htmlOf(from), textOf(from));
    }
    return from.cells->cells;
}

const std::optional<Clip> &PageView::rtfOf(const PasteSource &from) const
{
    static const std::optional<Clip> none;
    if (!from.mime || !from.fetched || from.mime->hasFormat(fragmentFormat()) || !hasRtfFormat(from.mime.get())) return none;
    Fetched &f = *from.fetched;
    if (!f.rtfDone) {
        f.rtfDone = true;
        f.rtfBytes = rtfOfMime(from.mime.get());   // (read once, held to kMostPastedRtf before it is parsed)
        Clip clip;
        if (!f.rtfBytes.isEmpty() && clipFromRtf(f.rtfBytes, &clip)) f.clip = clip;
    }
    return f.clip;
}

const QString &PageView::htmlOf(const PasteSource &from) const
{
    static const QString none;
    if (!from.mime || !from.fetched || !from.mime->hasHtml()) return none;
    Fetched &f = *from.fetched;
    if (!f.htmlDone) {
        f.htmlDone = true;
        // (a mime object can't say how big a format is without reading it: the size is checked on the one read, before anything is converted or parsed)
        if (from.mime->data(QStringLiteral("text/html")).size() <= kMostPastedHtml) f.html = safeClipHtml(from.mime->html());
    }
    return f.html;
}

const QString &PageView::textOf(const PasteSource &from) const
{
    static const QString none;
    if (!from.mime || !from.fetched || !from.mime->hasText()) return none;
    Fetched &f = *from.fetched;
    if (!f.textDone) {
        f.textDone = true;
        f.text = from.mime->text();
    }
    return f.text;
}

PageView::PasteSource PageView::sourceOfDrop(const QMimeData *mime) const
{
    PasteSource out;
    out.mime = std::shared_ptr<const QMimeData>(mime, [](const QMimeData *) {});
    out.cells = std::make_shared<CellsOfClip>();
    out.fetched = std::make_shared<Fetched>();
    return out;
}

// What a paste puts in, by the order: this program's own copy, then spreadsheet cells as their words (Paste Options and Paste Special make a table of them), then RTF, then HTML, then the words.
std::function<void()> PageView::putOf(const PasteSource &src)
{
    const QMimeData *mime = src.mime.get();
    PageView *self = this;
    if (src.ours()) return [self, &src] { pasteClip(*self->m_doc, self->m_cursor, src.clip); };
    const bool cells = mime->hasText() && office::wml::mayBeClipTable(htmlOf(src), textOf(src));
    if (!cells) {
        if (const auto &rtf = rtfOf(src)) {
            const Clip clip = *rtf;
            return [self, clip] { pasteClip(*self->m_doc, self->m_cursor, clip); };
        }
        if (!htmlOf(src).isEmpty()) {
            const QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(htmlOf(src));
            return [self, fragment] { pasteFragment(*self->m_doc, self->m_cursor, fragment); };
        }
    }
    if (mime->hasText()) {
        const QString text = textOf(src);
        return [self, text] { pasteText(*self->m_doc, self->m_cursor, text); };
    }
    return {};
}

// The paste and the drop both end here: smart paste, one step, the note for the Paste Options button.
bool PageView::pasteSource(const PasteSource &src)
{
    const int pasteFrom = m_cursor.selectionStart(), pasteSelectionEnd = m_cursor.selectionEnd();
    const bool startedInTable = inTable(m_cursor.block());
    const std::function<void()> put = putOf(src);
    if (!put) return false;
    const bool ours = src.ours();
    runPaste(smartCutPaste() && (!ours || src.clipWhole), put, pasteFrom);
    notePaste(pasteFrom, pasteSelectionEnd, src, startedInTable);
    caretMoved();
    return true;
}

void PageView::paste()
{
    if (m_shapeEditor) {
        m_shapeEditor->paste();
        return;
    }
    if (!m_doc) return;
    const int pasteFrom = m_cursor.selectionStart(), pasteSelectionEnd = m_cursor.selectionEnd();
    const PasteSource src = source();
    const QMimeData *mime = src.mime.get();
    if (!mime) return;
    const bool startedInTable = inTable(m_cursor.block());
    // A choice of cells: the clipboard's table, put where the choice says (nothing, not even a note of a paste, when it does not fit).
    if (m_choice.table >= 0) {
        const auto &cells = cellsOf(src);
        const TablePaste how = TablePaste(m_choice.table);
        if (!cells || !canPasteCells(*m_doc, m_cursor, how)) return;
        const bool done = pasteCells(*m_doc, m_cursor, *cells, how);   // (one step of its own; it looks at every limit first, and a refusal has changed nothing)
        if (!m_doc->tableNotice.isEmpty()) Q_EMIT notice(m_doc->tableNotice);
        if (!done) return;
        notePaste(pasteFrom, pasteSelectionEnd, src, startedInTable, true);
        caretMoved();
        return;
    }
    // A picture on the clipboard (a screenshot, say) goes in as a picture.
    if (!src.ours() && mime->hasImage() && !mime->hasText() && !mime->hasHtml() && !hasRtfFormat(mime)) {
        const QVariant data = mime->imageData();
        if (!data.canConvert<QImage>()) return;
        insertPictureImage(data.value<QImage>());
        return;
    }
    pasteSource(src);
}

// One step: the paste, and then, when a list choice was made, what it makes of the list items the paste put in.
void PageView::runPaste(bool smart, const std::function<void()> &put, int from)
{
    m_cursor.beginEditBlock();
    if (smart) smartPaste(*m_doc, m_cursor, put);
    else put();
    if (m_choice.list) applyListPaste(*m_doc, m_cursor, from, m_cursor.position(), *m_choice.list);
    m_cursor.endEditBlock();
}

void PageView::pasteListAs(ListPaste how)
{
    m_choice.list = how;
    paste();
    m_choice = PasteChoice();
}

bool PageView::caretBesideList() const
{
    if (!m_doc || m_cursor.document() != m_doc->story()) return false;
    const QTextBlock b = m_cursor.block();
    return listKindOf(*m_doc, b) != ListKind::None || (b.previous().isValid() && listKindOf(*m_doc, b.previous()) != ListKind::None);
}

bool PageView::clipboardMayHaveCells() const
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!m_doc || !mime || mime->hasFormat(fragmentFormat())) return false;
    const PasteSource from = clipboardSource();   // (read once for as long as the clipboard is what it is)
    return office::wml::mayBeClipTable(htmlOf(from), textOf(from));
}

bool PageView::clipboardHasCells() const
{
    return m_doc && cellsOf(clipboardSource()).has_value();
}

bool PageView::canPasteCellsAs(TablePaste how) const
{
    return m_doc && !m_shapeEditor && canPasteCells(*m_doc, m_cursor, how) && clipboardHasCells();
}

void PageView::pasteCellsAs(TablePaste how)
{
    m_choice.table = int(how);
    paste();
    m_choice = PasteChoice();
}

bool PageView::canPaste(PasteKind kind) const
{
    return m_doc && canPasteFrom(clipboardSource(), kind);
}

bool PageView::canPasteFrom(const PasteSource &from, PasteKind kind) const
{
    const QMimeData *mime = from.mime.get();
    if (!m_doc || !mime) return false;
    const bool ours = mime->hasFormat(fragmentFormat()) && !from.clip.isEmpty();
    const bool rtf = !ours && hasRtfFormat(mime);   // (the names of the formats only: nothing is read to find out)
    switch (kind) {
    case PasteKind::Keep: return ours || rtf || mime->hasText() || mime->hasHtml() || mime->hasImage();
    case PasteKind::Merge: return ours || rtf || mime->hasHtml();
    case PasteKind::TextOnly: return ours || rtf || mime->hasText() || mime->hasHtml();
    case PasteKind::Rtf: return rtf;
    case PasteKind::Picture: return mime->hasImage();
    case PasteKind::Html: return mime->hasHtml();
    }
    return false;
}

void PageView::pasteAs(PasteKind kind)
{
    const PasteSource src = source();
    if (!m_doc || !canPasteFrom(src, kind)) return;
    const int pasteFrom = m_cursor.selectionStart(), pasteSelectionEnd = m_cursor.selectionEnd();
    const bool startedInTable = inTable(m_cursor.block());
    const QMimeData *mime = src.mime.get();
    const bool ours = src.ours();
    auto put = [this, ours, pasteFrom, &src](const std::function<void()> &what) { runPaste(smartCutPaste() && (!ours || src.clipWhole), what, pasteFrom); };
    switch (kind) {
    case PasteKind::Keep:
        paste();
        return;
    case PasteKind::Merge: {
        const QTextDocumentFragment fragment = ours ? src.clip.fragment : rtfOf(src) ? rtfOf(src)->fragment : QTextDocumentFragment::fromHtml(htmlOf(src));
        put([&] { pasteMerged(*m_doc, m_cursor, fragment); });
        break;
    }
    case PasteKind::TextOnly: {
        const QString text = ours ? plainTextOf(src.clip.fragment) : (mime->hasText() ? textOf(src) : rtfOf(src) ? plainTextOf(rtfOf(src)->fragment) : QTextDocumentFragment::fromHtml(htmlOf(src)).toPlainText());
        put([&] { pasteText(*m_doc, m_cursor, text); });
        break;
    }
    case PasteKind::Picture: {
        const QVariant data = mime->imageData();
        if (!data.canConvert<QImage>()) return;
        insertPictureImage(data.value<QImage>());
        return;
    }
    case PasteKind::Html: {
        const QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(htmlOf(src));
        put([&] { pasteFragment(*m_doc, m_cursor, fragment); });
        break;
    }
    case PasteKind::Rtf: {
        if (!rtfOf(src)) return;
        const Clip clip = *rtfOf(src);
        put([&] { pasteClip(*m_doc, m_cursor, clip); });
        break;
    }
    }
    notePaste(pasteFrom, pasteSelectionEnd, src, startedInTable);
    caretMoved();
}

// Notes what was pasted (for the Paste Options button), with a copy of what it was made from: the choices that follow paste that, not what the clipboard holds then.
void PageView::notePaste(int from, int selectionEnd, const PasteSource &made, bool startedInTable, bool asCells)
{
    PasteSource kept = made;
    if (!m_pasteAgain && made.mime) {
        auto copy = std::make_shared<QMimeData>();
        const QMimeData *mime = made.mime.get();
        if (mime->hasText()) copy->setText(textOf(made));
        if (mime->hasHtml()) copy->setHtml(htmlOf(made));
        if (made.fetched && !made.fetched->rtfBytes.isEmpty()) copy->setData(QStringLiteral("text/rtf"), made.fetched->rtfBytes);
        if (mime->hasImage()) copy->setImageData(mime->imageData());
        if (mime->hasFormat(fragmentFormat())) copy->setData(fragmentFormat(), mime->data(fragmentFormat()));
        kept.mime = copy;
    }
    m_lastPaste = LastPaste{from, selectionEnd, m_cursor.position(), m_doc ? m_doc->story()->revision() : -1, m_doc ? m_doc->story() : nullptr, m_doc && listBesidePaste(*m_doc, from, m_cursor.position()), asCells, startedInTable, false, kept};
    if (kept.mime && !kept.mime->hasFormat(fragmentFormat()))
        m_lastPaste.offersCells = asCells || office::wml::mayBeClipTable(htmlOf(kept), textOf(kept));
}

bool PageView::pasteOptionsAvailable() const
{
    return m_doc && m_lastPaste.story == m_doc->story() && m_cursor.document() == m_doc->story() && !m_cursor.hasSelection() && m_cursor.position() == m_lastPaste.to &&
           m_doc->story()->revision() == m_lastPaste.revision && m_lastPaste.to > m_lastPaste.from;
}

// Takes the last paste back and pastes it again the way `again` says (from what it was made of); when that does nothing (it does not fit), the paste stands.
bool PageView::redoLastPaste(const std::function<void()> &again)
{
    const int from = m_lastPaste.from, selectionEnd = m_lastPaste.selectionEnd, to = m_lastPaste.to;
    const std::optional<PasteSource> made = m_lastPaste.source;
    undo();
    const int taken = m_doc->story()->revision();
    QTextCursor back(m_doc->story());
    back.setPosition(from);
    back.setPosition(std::max(from, selectionEnd), QTextCursor::KeepAnchor);
    m_cursor = back;
    m_pasteAgain = made;
    again();
    m_pasteAgain.reset();
    if (m_doc->story()->revision() == taken) {
        redo();
        if (m_cursor.position() == to && !m_cursor.hasSelection()) {
            m_lastPaste.revision = m_doc->story()->revision();
            m_lastPaste.story = m_doc->story();
        }
        return false;
    }
    return true;
}

bool PageView::changeLastPaste(PasteKind kind)
{
    if (!pasteOptionsAvailable() || !m_lastPaste.source || !canPasteFrom(*m_lastPaste.source, kind)) return false;
    return redoLastPaste([this, kind] { pasteAs(kind); });
}

bool PageView::changeLastPasteList(ListPaste how)
{
    if (!pasteOptionsAvailable() || !m_lastPaste.source || !canPasteFrom(*m_lastPaste.source, PasteKind::Keep) || !m_lastPaste.lists) return false;
    return redoLastPaste([this, how] { pasteListAs(how); });
}

bool PageView::changeLastPasteCells(TablePaste how)
{
    if (!pasteOptionsAvailable() || !m_lastPaste.offersCells || !m_lastPaste.source || !cellsOf(*m_lastPaste.source)) return false;
    QTextCursor there(m_doc->story());
    there.setPosition(m_lastPaste.from);
    if (!canPasteCells(*m_doc, there, how)) return false;
    return redoLastPaste([this, how] { pasteCellsAs(how); });
}

// The choices the button offers for the paste just made: how its formatting is kept, and, when it brought list items or cells, what to make of them.
void PageView::fillPasteMenu()
{
    QMenu *menu = m_pasteButton->menu();
    // (An action is let go later: this runs from the step an action of the menu took.)
    for (QAction *old : menu->actions()) {
        menu->removeAction(old);
        old->deleteLater();
    }
    auto add = [this, menu](const char *text, const char *name, int id, const std::function<void()> &run) {
        QAction *a = menu->addAction(QCoreApplication::translate("jp::write::PageView", text));
        a->setObjectName(QStringLiteral("%1%2").arg(QLatin1String(name)).arg(id));
        connect(a, &QAction::triggered, this, [this, run] {
            run();
            setFocus(Qt::OtherFocusReason);
        });
    };
    // (The cells are parsed, once, when the menu opens: what may be cells is cells only when it parses.)
    const bool cells = m_lastPaste.cells || (m_lastPaste.offersCells && m_lastPaste.source && cellsOf(*m_lastPaste.source));
    if (!m_lastPaste.cells) {
        add(QT_TRANSLATE_NOOP("jp::write::PageView", "Keep Source Formatting"), "pasteOption", int(PasteKind::Keep), [this] { changeLastPaste(PasteKind::Keep); });
        add(QT_TRANSLATE_NOOP("jp::write::PageView", "Merge Formatting"), "pasteOption", int(PasteKind::Merge), [this] { changeLastPaste(PasteKind::Merge); });
    }
    add(QT_TRANSLATE_NOOP("jp::write::PageView", "Keep Text Only"), "pasteOption", int(PasteKind::TextOnly), [this] { changeLastPaste(PasteKind::TextOnly); });
    if (m_lastPaste.lists) {
        menu->addSeparator();
        add(QT_TRANSLATE_NOOP("jp::write::PageView", "Continue List"), "pasteList", int(ListPaste::Continue), [this] { changeLastPasteList(ListPaste::Continue); });
        add(QT_TRANSLATE_NOOP("jp::write::PageView", "New List"), "pasteList", int(ListPaste::New), [this] { changeLastPasteList(ListPaste::New); });
        add(QT_TRANSLATE_NOOP("jp::write::PageView", "Merge List"), "pasteList", int(ListPaste::Merge), [this] { changeLastPasteList(ListPaste::Merge); });
    }
    if (cells) {
        menu->addSeparator();
        if (m_lastPaste.inTable) {
            add(QT_TRANSLATE_NOOP("jp::write::PageView", "Nest Table"), "pasteCells", int(TablePaste::Nest), [this] { changeLastPasteCells(TablePaste::Nest); });
            add(QT_TRANSLATE_NOOP("jp::write::PageView", "Merge Table"), "pasteCells", int(TablePaste::Merge), [this] { changeLastPasteCells(TablePaste::Merge); });
            add(QT_TRANSLATE_NOOP("jp::write::PageView", "Insert as New Rows"), "pasteCells", int(TablePaste::NewRows), [this] { changeLastPasteCells(TablePaste::NewRows); });
            add(QT_TRANSLATE_NOOP("jp::write::PageView", "Overwrite Cells"), "pasteCells", int(TablePaste::Overwrite), [this] { changeLastPasteCells(TablePaste::Overwrite); });
        } else {
            add(QT_TRANSLATE_NOOP("jp::write::PageView", "Table"), "pasteCells", int(TablePaste::NewTable), [this] { changeLastPasteCells(TablePaste::NewTable); });
        }
    }
}

void PageView::updatePasteButton()
{
    if (!m_pasteButton) {
        m_pasteButton = new QToolButton(viewport());
        m_pasteButton->setObjectName(QStringLiteral("pasteOptionsButton"));
        m_pasteButton->setText(QCoreApplication::translate("jp::write::PageView", "Paste Options"));
        m_pasteButton->setAccessibleName(QCoreApplication::translate("jp::write::PageView", "Paste Options"));
        m_pasteButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        m_pasteButton->setPopupMode(QToolButton::InstantPopup);
        m_pasteButton->setFocusPolicy(Qt::NoFocus);
        auto *menu = new QMenu(m_pasteButton);
        m_pasteButton->setMenu(menu);
        connect(menu, &QMenu::aboutToShow, this, [this] { fillPasteMenu(); });   // (built when it opens: the button moves with every scroll)
        m_pasteButton->hide();
        connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { updatePasteButton(); });
    }
    QRect r;
    if (!pasteOptionsAvailable() || !caretRect(&r)) {
        m_pasteButton->hide();
        return;
    }
    m_pasteButton->adjustSize();
    const QSize size = m_pasteButton->size();
    m_pasteButton->move(std::clamp(r.right() + 6, 0, std::max(0, viewport()->width() - size.width())), std::clamp(r.bottom() + 4, 0, std::max(0, viewport()->height() - size.height())));
    m_pasteButton->show();
    m_pasteButton->raise();
}

void PageView::selectAll()
{
    if (m_shapeEditor) {
        m_shapeEditor->selectAll();
        return;
    }
    if (!m_doc) return;
    m_cursor.select(QTextCursor::Document);
    caretMoved();
}

void PageView::insertText(const QString &text)
{
    if (!m_doc) return;
    if (m_cursor.hasSelection() && !m_typingReplaces) {
        m_cursor.setPosition(m_cursor.selectionEnd());
    } else if (m_overtype && !m_cursor.hasSelection()) {
        // Overtype: the letters typed take the place of those after the caret, as far as the paragraph goes.
        const QTextBlock b = m_cursor.block();
        const int room = b.position() + b.length() - 1 - m_cursor.position();
        const int n = std::min<int>(room, int(text.size()));
        if (n > 0) m_cursor.setPosition(m_cursor.position() + n, QTextCursor::KeepAnchor);
    }
    Settings &st = Settings::get();
    write::AutoOptions auto_;
    auto_.correct = st.value(QStringLiteral("proof/autocorrect"), true).toBool();
    auto_.quotes = st.value(QStringLiteral("proof/smartQuotes"), true).toBool();
    auto_.format = st.value(QStringLiteral("proof/autoformat"), true).toBool();
    auto_.links = st.value(QStringLiteral("proof/autolink"), true).toBool();
    if (m_overtype) auto_ = write::AutoOptions{false, false, false, false};
    write::typeText(*m_doc, m_cursor, text, auto_);
    caretMoved();
}

// ---- the keyboard

void PageView::typeKey(int key, Qt::KeyboardModifiers m)
{
    if (!m_doc) return;
    m_movedFrom = m_cursor.position();
    const bool shift = m & Qt::ShiftModifier, ctrl = m & Qt::ControlModifier;
    const QTextCursor::MoveMode mode = shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    if (!shift && !ctrl && ((key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) || (key == Qt::Key_Down && (m & Qt::AltModifier)))) {
        QRect r;
        const QPoint where = caretRect(&r) ? viewport()->mapToGlobal(r.bottomLeft()) : QCursor::pos();
        if (activateControlAtCaret(key, where)) return;
    }
    switch (key) {
    case Qt::Key_Left:
        if (ctrl) m_cursor.movePosition(QTextCursor::PreviousWord, mode);
        else moveCursor(m_cursor, QTextCursor::PreviousCharacter, shift);
        caretMoved();
        break;
    case Qt::Key_Right:
        if (ctrl) m_cursor.movePosition(QTextCursor::NextWord, mode);
        else moveCursor(m_cursor, QTextCursor::NextCharacter, shift);
        caretMoved();
        break;
    case Qt::Key_Up:
    case Qt::Key_Down: {
        const int dir = key == Qt::Key_Up ? -1 : 1;
        if (ctrl) {
            m_cursor.movePosition(dir < 0 ? QTextCursor::PreviousBlock : QTextCursor::NextBlock, mode);
            caretMoved();
        } else {
            // Up from a note's first line and Down from its last leave it, for the text, just after its reference.
            if (m_note.active && !shift && !m_cursor.hasSelection() && noteLineAtEdge(dir)) {
                leaveNote(true);
                break;
            }
            const int to = moveVertical(m_cursor.position(), dir);
            // Up from the first line of an endnote and Down from its last leave it, for the text just after its reference.
            if (!m_note.active && !m_hf.active && !shift && !m_cursor.hasSelection()) {
                const EndnoteSpan span = endnoteSpanAt(m_doc->story(), m_cursor.position());
                int id = 0;
                if (span.valid && span.editable && (to < span.textFrom || to > span.textTo || to == m_cursor.position()) && m_doc->endnoteIdOf(m_doc->story()->findBlock(m_cursor.position()), &id)) {
                    leaveEndnote(id);
                    break;
                }
            }
            m_cursor.setPosition(to, mode);
            caretMoved(true);
        }
        break;
    }
    case Qt::Key_Home:
        if (ctrl) m_cursor.movePosition(QTextCursor::Start, mode);
        else m_cursor.setPosition(lineEdge(m_cursor.position(), false), mode);
        caretMoved();
        break;
    case Qt::Key_End:
        if (ctrl) m_cursor.movePosition(QTextCursor::End, mode);
        else m_cursor.setPosition(lineEdge(m_cursor.position(), true), mode);
        caretMoved();
        break;
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        const int dir = key == Qt::Key_PageUp ? -1 : 1;
        QRect r;
        if (!caretRect(&r)) break;
        const int step = std::max(40, viewport()->height() - 60);
        const int before = verticalScrollBar()->value();
        verticalScrollBar()->setValue(before + dir * step);
        const int moved = verticalScrollBar()->value() - before;
        // (The caret has moved up the window by what the window scrolled; it goes on by the rest of the step.)
        const Hit hit = hitAt(QPoint(r.x(), std::clamp(r.center().y() + dir * step - moved, 0, viewport()->height() - 1)));
        const int at = positionAt(hit);
        if (at >= 0) m_cursor.setPosition(at, mode);
        if (dir > 0 && moved == 0 && !pagesComplete()) paginateAll();
        caretMoved(true);
        break;
    }
    case Qt::Key_Backspace:
        deleteBack(*m_doc, m_cursor, ctrl);
        write::restoreEmptyPlaceholder(*m_doc, m_cursor.position());
        caretMoved();
        break;
    case Qt::Key_Delete:
        deleteForward(*m_doc, m_cursor, ctrl);
        write::restoreEmptyPlaceholder(*m_doc, m_cursor.position());
        caretMoved();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (ctrl) insertPageBreak(*m_doc, m_cursor);
        else if (shift) write::insertText(*m_doc, m_cursor, QString(QChar(QChar::LineSeparator)));
        else {
            write::AutoOptions auto_;
            auto_.format = Settings::get().value(QStringLiteral("proof/autoformat"), true).toBool();
            if (!write::enterAfterRule(*m_doc, m_cursor, auto_)) insertParagraph(*m_doc, m_cursor);
        }
        caretMoved();
        break;
    case Qt::Key_Tab:
        insertTab(*m_doc, m_cursor, false);
        caretMoved();
        break;
    case Qt::Key_Backtab:
        insertTab(*m_doc, m_cursor, true);
        caretMoved();
        break;
    case Qt::Key_Escape:
        if (m_hf.active) {
            leaveHeaderFooter();
            break;
        }
        if (m_note.active) {
            leaveNote(true);
            break;
        }
        if (int id = 0; !m_hf.active && !m_cursor.hasSelection() && inEndnoteWords(m_cursor) && m_doc->endnoteIdOf(m_doc->story()->findBlock(m_cursor.position()), &id)) {
            leaveEndnote(id);
            break;
        }
        if (m_cursor.hasSelection()) {
            m_cursor.clearSelection();
            caretMoved();
        }
        break;
    default:
        break;
    }
}

void PageView::keyPressEvent(QKeyEvent *e)
{
    if (!m_doc) {
        e->ignore();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_linkFrom >= 0) {
        m_linkFrom = -1;
        viewport()->unsetCursor();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_editWrap) {
        setEditWrap(false);
        Q_EMIT caretChanged();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_editPoints) {
        m_editPoints = false;
        viewport()->update();
        Q_EMIT caretChanged();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && !m_shapeDraw.isEmpty()) {
        // (the shape tool is put down; a shape begun is let go)
        m_shapeDraw.clear();
        m_toolDrag = ToolDrag();
        m_hoverCursorShape = -1;
        viewport()->setCursor(Qt::IBeamCursor);
        viewport()->update();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_tableTool != TableTool::None) {
        // (the pen or the eraser is put down; a line begun is let go)
        m_toolDrag = ToolDrag();
        setTableTool(TableTool::None);
        viewport()->update();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Escape && m_columnDrag.active) {
        // (a pull of a border or of the table's handle is let go: nothing is changed)
        m_columnDrag.active = false;
        viewport()->update();
        e->accept();
        return;
    }
    if (m_readOnly) {
        // (only the keys that move the caret)
        switch (e->key()) {
        case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Home: case Qt::Key_End: case Qt::Key_PageUp: case Qt::Key_PageDown:
            typeKey(e->key(), e->modifiers());
            e->accept();
            return;
        default:
            e->ignore();
            return;
        }
    }
    if (keyOnSelectedShape(e, nullptr) == SelectedShapeKey::Begin && selectedPicture() >= 0 && beginShapeTextEdit()) {
        e->accept();
        return;
    }
    switch (e->key()) {
    case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Up: case Qt::Key_Down: case Qt::Key_Home: case Qt::Key_End: case Qt::Key_PageUp: case Qt::Key_PageDown:
    case Qt::Key_Backspace: case Qt::Key_Delete: case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Tab: case Qt::Key_Backtab: case Qt::Key_Escape:
        typeKey(e->key(), e->modifiers());
        e->accept();
        return;
    default:
        break;
    }
    if (e->key() == Qt::Key_Insert && m_insertKeyOvertype && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
        m_overtype = !m_overtype;
        Q_EMIT notice(m_overtype ? tr("Overtype: typing replaces the letters after the caret. Press Insert to go back.") : tr("Insert: typing pushes the letters after the caret along."));
        e->accept();
        return;
    }
    // Text: not a shortcut (Ctrl alone), and not a control character.
    const QString text = e->text();
    const bool shortcut = (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) && !(e->modifiers() & Qt::AltModifier);
    // A shape or text box that is selected takes the words typed: they go in it, in place.
    QString typedOnShape;
    if (!shortcut && keyOnSelectedShape(e, &typedOnShape) == SelectedShapeKey::BeginTyped && selectedPicture() >= 0 && beginShapeTextEdit(typedOnShape)) {
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Space && !(e->modifiers() & ~Qt::KeypadModifier) && activateControlAtCaret(Qt::Key_Space, QPoint())) {   // (Space flips a check box the caret is in)
        e->accept();
        return;
    }
    if (!text.isEmpty() && !shortcut && text[0].isPrint()) {
        insertText(text);
        e->accept();
        return;
    }
    e->ignore();
}

void PageView::inputMethodEvent(QInputMethodEvent *e)
{
    if (!m_doc || m_readOnly) return;
    if (!e->commitString().isEmpty()) insertText(e->commitString());
    e->accept();
}

QVariant PageView::inputMethodQuery(Qt::InputMethodQuery q) const
{
    switch (q) {
    case Qt::ImEnabled: return m_doc != nullptr;
    case Qt::ImCursorRectangle: {
        QRect r;
        if (const_cast<PageView *>(this)->caretRect(&r)) return r;
        return QRect();
    }
    case Qt::ImCursorPosition: return m_cursor.positionInBlock();
    case Qt::ImSurroundingText: return m_cursor.block().text();
    case Qt::ImCurrentSelection: return m_cursor.selectedText();
    default: return QVariant();
    }
}

// ---- the mouse

void PageView::mousePressEvent(QMouseEvent *e)
{
    if (!m_doc) return;
    setFocus(Qt::MouseFocusReason);
    m_movedFrom = -1;   // (a click has no way it went)
    if (m_linkFrom >= 0) {
        // Create Link is waiting for the box that follows: this click names it (or, on no box that can, leaves the link).
        const int from = m_linkFrom, to = pictureAtPoint(e->position().toPoint());
        m_linkFrom = -1;
        viewport()->unsetCursor();
        if (e->button() == Qt::LeftButton && to >= 0) {
            m_doc->pictureNotice.clear();
            if (linkTextBoxes(*m_doc, from, to) == PictureResult::Ok) {
                selectPicture(from);
                Q_EMIT contentChanged();
            } else {
                Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? tr("A text box can be followed only by a text box that has no words and is in no chain yet.") : m_doc->pictureNotice);
            }
            e->accept();
            viewport()->update();
            return;
        }
    }
    if (!m_shapeDraw.isEmpty() && e->button() == Qt::LeftButton && !m_hf.active && !m_note.active && !m_readOnly) {
        // Insert > Shapes is waiting for a drag: it begins here, on the page, at a place of the text.
        const Hit hit = hitAt(e->position().toPoint());
        const int at = positionAt(hit);
        if (hit.page >= 0 && at >= 0) {
            m_toolDrag = {true, hit.page, at, hit.point, hit.point};
            viewport()->update();
            e->accept();
            return;
        }
    }
    if (e->button() == Qt::LeftButton && e->type() != QEvent::MouseButtonDblClick && beginPictureDrag(e)) {
        e->accept();
        return;
    }
    m_columnDrag.pending = false;
    if (m_tableTool != TableTool::None && e->button() == Qt::LeftButton && !m_hf.active && !m_note.active && !m_readOnly) {
        const Hit hit = hitAt(e->position().toPoint());
        const int at = positionAt(hit);
        if (hit.page >= 0 && at >= 0) {
            m_toolDrag = {true, hit.page, at, hit.point, hit.point};
            viewport()->update();
            e->accept();
            return;
        }
    }
    if (e->button() == Qt::LeftButton && e->type() != QEvent::MouseButtonDblClick && !m_hf.active && !m_note.active) {
        // The little triangle by a heading collapses or opens it.
        if (const int heading = headingAt(e->position().toPoint()); heading >= 0 && collapseMark(heading).adjusted(-4, -4, 4, 4).contains(e->position().toPoint())) {
            toggleHeadingCollapsed(heading);
            e->accept();
            return;
        }
        if (tableHandleAt(e->position().toPoint(), &m_columnDrag)) {
            m_columnDrag.active = true;
            viewport()->update();
            e->accept();
            return;
        }
        // A press on a border is a click (the caret goes there) until the pointer has moved a few pixels; then it is the pull of the border.
        ColumnDrag border;
        if (columnBorderAt(e->position().toPoint(), &border)) {
            m_columnDrag = border;
            m_columnDrag.pending = true;
            m_columnDrag.startPx = e->position().toPoint();
        }
    }
    const Hit hit = hitAt(e->position().toPoint());
    if (m_hf.active) {
        // A click in the text does nothing while a header or footer is edited (a double click leaves); one in a header's or a footer's area goes to it.
        const Area area = areaOf(hit);
        if (area == Area::Body || !takeFurnitureAt(hit, area == Area::Footer)) return;
    } else {
        // A click in a footnote's text puts the caret there, in the note; a click anywhere else is in the text, and leaves the note it was in.
        QTextDocument *noteStory = nullptr;
        int noteId = 0, noteNumber = 0;
        if (noteAt(hit, &noteStory, &noteId, &noteNumber)) {
            if (!enterNote(noteStory, noteId, hit.page, noteNumber)) return;
        } else if (m_note.active) {
            leaveNote();
        }
    }
    int at = positionAt(hit);
    if (at < 0) return;
    // A click in the second or a later copy of an endnote (which the pages show, and no more) is a click at the same place of its first copy, the note's own.
    if (!m_note.active && !m_hf.active && m_cursor.document() == m_doc->story()) at = placeInFirstCopy(at);
    if (e->button() == Qt::RightButton) {
        // A right click on a picture selects it, and the menu is for it.
        if (const int picture = pictureAtPoint(e->position().toPoint()); picture >= 0 && picture != selectedPicture()) {
            selectPicture(picture);
            m_contextAt = picture;
            return;
        }
        if (!m_cursor.hasSelection() || at < m_cursor.selectionStart() || at > m_cursor.selectionEnd()) {
            m_cursor.setPosition(at);
            caretMoved();
        }
        m_contextAt = at;   // (what the menu is for, whether or not the selection stayed)
        return;
    }
    if (e->button() != Qt::LeftButton) return;
    // Ctrl+click on a link inside the text (a reference or a table of contents line with its link, or a link to a bookmark) goes to what it names.
    if (((e->modifiers() & Qt::ControlModifier) || (!m_linkNeedsCtrl && !(e->modifiers() & Qt::ShiftModifier))) && followLinkAt(at)) return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool again = now - m_lastClickTime < QApplication::doubleClickInterval() && (e->position().toPoint() - m_lastPos).manhattanLength() < 6;
    // (A double click event is the second press of a pair, whatever came before it.)
    m_clicks = again ? (m_clicks % 3) + 1 : e->type() == QEvent::MouseButtonDblClick ? 2 : 1;
    m_lastClickTime = now;
    m_lastPos = e->position().toPoint();
    if (m_clicks <= 2 && !(e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) && clickInControl(at, e->position().toPoint(), e->globalPosition().toPoint(), m_clicks)) return;
    // A press inside the selection may begin a drag of its text: the selection stays until the pointer has moved (or the button is let go, which puts the caret there).
    if (m_clicks == 1 && m_textDragAllowed && m_dragTextOption && !m_readOnly && !(e->modifiers() & Qt::ShiftModifier) && m_cursor.hasSelection() && !m_hf.active && !m_note.active && m_cursor.document() == m_doc->story() &&
        at >= m_cursor.selectionStart() && at < m_cursor.selectionEnd() && !isLocked(m_cursor.block())) {
        m_textDrag.pending = true;
        m_textDrag.start = e->position().toPoint();
        return;
    }
    if (m_clicks == 3) {
        m_cursor.setPosition(at);
        m_cursor.movePosition(QTextCursor::StartOfBlock);
        m_cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    } else if (m_clicks == 2) {
        m_cursor.setPosition(at);
        m_cursor.select(QTextCursor::WordUnderCursor);
    } else if (e->modifiers() & Qt::ShiftModifier) {
        m_cursor.setPosition(at, QTextCursor::KeepAnchor);
    } else {
        m_cursor.setPosition(at);
    }
    m_dragging = true;
    m_autoscroll.start();
    caretMoved();
}

void PageView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_toolDrag.active) {
        const Hit hit = hitAt(e->position().toPoint());
        if (hit.page == m_toolDrag.page) m_toolDrag.to = hit.point;
        viewport()->update();
        return;
    }
    if (m_columnDrag.pending) {
        if (!(e->buttons() & Qt::LeftButton)) m_columnDrag.pending = false;
        else if ((e->position().toPoint() - m_columnDrag.startPx).manhattanLength() >= QApplication::startDragDistance()) {
            m_columnDrag.pending = false;
            m_columnDrag.active = true;
            m_dragging = false;
            m_autoscroll.stop();
        }
    }
    if (m_columnDrag.active) {
        const Hit hit = hitAt(e->position().toPoint());
        m_columnDrag.nowX = hit.point.x();
        m_columnDrag.nowY = hit.point.y();
        viewport()->update();
        return;
    }
    if (m_pictureDrag.active) {
        updatePictureDrag(e->position().toPoint(), e->modifiers());
        return;
    }
    if (m_textDrag.pending) {
        if (!(e->buttons() & Qt::LeftButton)) m_textDrag.pending = false;
        else if ((e->position().toPoint() - m_textDrag.start).manhattanLength() >= QApplication::startDragDistance()) startTextDrag();
        return;
    }
    if (m_doc && !(e->buttons() & Qt::LeftButton)) {
        const int heading = headingAt(e->position().toPoint());
        if (heading != m_collapseHover) {
            m_collapseHover = heading;
            viewport()->update();
        }
    }
    if (m_doc && !m_dragging && m_tableTool == TableTool::None && !(e->buttons() & Qt::LeftButton)) {
        // The pointer says what a press on the selected picture would do, or that a press on a column's border would pull it.
        const PictureHandle h = pictureHandleAt(e->position().toPoint());
        ColumnDrag border;
        const int shape = h == PictureHandle::None && tableHandleAt(e->position().toPoint(), &border) ? int(Qt::SizeFDiagCursor)
                          : h == PictureHandle::None && columnBorderAt(e->position().toPoint(), &border) ? int(border.row ? Qt::SplitVCursor : Qt::SplitHCursor)
                          : h == PictureHandle::None ? int(Qt::IBeamCursor) : h == PictureHandle::Body ? int(Qt::SizeAllCursor)
                          : (h == PictureHandle::Rotate || h == PictureHandle::Adjust) ? int(Qt::PointingHandCursor)
                          : (h == PictureHandle::TopLeft || h == PictureHandle::BottomRight) ? int(Qt::SizeFDiagCursor)
                          : (h == PictureHandle::TopRight || h == PictureHandle::BottomLeft) ? int(Qt::SizeBDiagCursor)
                          : (h == PictureHandle::Top || h == PictureHandle::Bottom) ? int(Qt::SizeVerCursor) : int(Qt::SizeHorCursor);
        if (shape != m_hoverCursorShape) {
            m_hoverCursorShape = shape;
            viewport()->setCursor(Qt::CursorShape(shape));
        }
    }
    if (m_dragging && !(e->buttons() & Qt::LeftButton)) {   // (the release was lost)
        m_dragging = false;
        m_autoscroll.stop();
    }
    if (!m_doc || !m_dragging || !(e->buttons() & Qt::LeftButton)) return;
    const int at = positionAt(hitAt(e->position().toPoint()));
    if (at < 0) return;
    if (m_clicks == 2) {
        // By words.
        QTextCursor w(m_cursor.document());
        w.setPosition(at);
        w.select(QTextCursor::WordUnderCursor);
        const int anchor = m_cursor.anchor() < m_cursor.position() ? m_cursor.selectionStart() : m_cursor.selectionEnd();
        Q_UNUSED(anchor);
        m_cursor.setPosition(at > m_cursor.anchor() ? w.selectionEnd() : w.selectionStart(), QTextCursor::KeepAnchor);
    } else {
        m_cursor.setPosition(at, QTextCursor::KeepAnchor);
    }
    m_movedFrom = m_cursor.anchor();   // (a drag goes on or back from where it began)
    caretMoved();
}

void PageView::setTableTool(TableTool tool)
{
    if (m_tableTool == tool) return;
    m_tableTool = tool;
    m_toolDrag = ToolDrag();
    m_hoverCursorShape = -1;
    viewport()->setCursor(tool == TableTool::None ? Qt::IBeamCursor : Qt::CrossCursor);
    viewport()->update();
    Q_EMIT tableToolChanged();
}

void PageView::beginShapeDraw(const QString &preset)
{
    if (!m_doc || m_readOnly) return;
    setTableTool(TableTool::None);
    m_shapeDraw = preset;
    m_toolDrag = ToolDrag();
    m_hoverCursorShape = -1;
    viewport()->setCursor(Qt::CrossCursor);
    viewport()->update();
}

void PageView::finishShapeDraw(const QPoint &viewportPos, Qt::KeyboardModifiers modifiers)
{
    const ToolDrag d = m_toolDrag;
    const QString preset = m_shapeDraw;
    m_toolDrag = ToolDrag();
    m_shapeDraw.clear();
    m_hoverCursorShape = -1;
    viewport()->setCursor(Qt::IBeamCursor);
    viewport()->update();
    if (!m_doc || d.position < 0 || d.position >= m_doc->story()->characterCount()) return;
    const Hit hit = hitAt(viewportPos);
    const QPointF to = hit.page == d.page ? hit.point : d.to;
    QTextCursor c(m_doc->story());
    c.setPosition(d.position);
    m_doc->pictureNotice.clear();
    const PictureResult result = insertShapeAt(*m_doc, c, preset, d.from, to, modifiers & Qt::ShiftModifier);
    if (result == PictureResult::Ok) {
        m_cursor = c;
        selectPicture(d.position);
        Q_EMIT contentChanged();
        return;
    }
    Q_EMIT notice(m_doc->pictureNotice.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "That shape can't be put here.") : m_doc->pictureNotice);
}

void PageView::finishTableTool(const QPoint &viewportPos)
{
    const ToolDrag d = m_toolDrag;
    m_toolDrag = ToolDrag();
    viewport()->update();
    if (!m_doc || d.position < 0 || d.position >= m_doc->story()->characterCount()) return;
    const Hit hit = hitAt(viewportPos);
    const QPointF delta = hit.page == d.page ? hit.point - d.from : QPointF();
    const int toPosition = hit.page == d.page ? positionAt(hit) : -1;
    QTextCursor c(m_doc->story());
    c.setPosition(d.position);
    m_doc->tableNotice.clear();
    bool changed = false;
    if (m_tableTool == TableTool::Draw) {
        if (!inTable(c.block())) {
            // A rectangle drawn in the text: a table of one cell that size (as wide as drawn, at least as tall).
            if (std::abs(delta.x()) < 12 || std::abs(delta.y()) < 8) return;
            write::NewTable table;
            table.rows = table.columns = 1;
            table.columnWidth = std::min(std::abs(delta.x()), write::textWidthAt(*m_doc, d.position));
            if (write::insertTable(*m_doc, c, table)) {
                write::setRowHeight(*m_doc, c, std::abs(delta.y()), false);
                changed = true;
            }
        } else if (std::abs(delta.x()) >= 8 || std::abs(delta.y()) >= 8) {
            // A line drawn across a cell divides it: a line that goes across makes two rows, one that goes down makes two columns.
            changed = write::splitCells(*m_doc, c, std::abs(delta.x()) >= std::abs(delta.y()) ? 1 : 2, std::abs(delta.x()) >= std::abs(delta.y()) ? 2 : 1);
        }
    } else if (toPosition >= 0 && toPosition != d.position) {
        // The eraser rubs out the line between the cell it began in and the one it ended in.
        c.setPosition(toPosition, QTextCursor::KeepAnchor);
        if (inTable(m_doc->story()->findBlock(d.position)) && inTable(m_doc->story()->findBlock(toPosition))) changed = write::mergeCells(*m_doc, c);
    }
    if (!changed && !m_doc->tableNotice.isEmpty()) Q_EMIT notice(m_doc->tableNotice);
    if (changed) {
        m_cursor.setPosition(std::min(c.position(), m_doc->story()->characterCount() - 1));
        refresh();
        caretMoved();
    }
}

void PageView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_toolDrag.active && !m_shapeDraw.isEmpty()) {
        finishShapeDraw(e->position().toPoint(), e->modifiers());
        return;
    }
    if (m_toolDrag.active) {
        finishTableTool(e->position().toPoint());
        return;
    }
    if (m_columnDrag.active) {
        const Hit hit = hitAt(e->position().toPoint());
        const double width = std::max(6.0, m_columnDrag.width + (m_columnDrag.row ? hit.point.y() - m_columnDrag.startY : hit.point.x() - m_columnDrag.startX));
        const int at = m_columnDrag.position;
        const bool whole = m_columnDrag.whole;
        m_columnDrag.active = false;
        viewport()->update();
        if (std::abs(width - m_columnDrag.width) >= 0.5 && at >= 0 && at < m_doc->story()->characterCount()) {
            QTextCursor c(m_doc->story());
            c.setPosition(at);
            if (whole) write::setTableWidth(*m_doc, c, width);
            else if (m_columnDrag.row) write::setRowHeight(*m_doc, c, width, false);
            else write::setColumnWidth(*m_doc, c, width);
            m_cursor.setPosition(std::min(at, m_doc->story()->characterCount() - 1));
            refresh();
            caretMoved();
        }
        return;
    }
    m_columnDrag.pending = false;
    if (m_pictureDrag.active) {
        endPictureDrag(e->position().toPoint(), e->modifiers());
        return;
    }
    if (m_textDrag.pending) {
        // A click inside the selection, without a pull, puts the caret there.
        m_textDrag.pending = false;
        const int at = positionAt(hitAt(e->position().toPoint()));
        if (at >= 0) {
            m_cursor.setPosition(at);
            caretMoved();
        }
        if (e->button() == Qt::LeftButton) Q_EMIT selectionMade();
        return;
    }
    m_dragging = false;
    m_autoscroll.stop();
    if (e->button() == Qt::LeftButton) Q_EMIT selectionMade();
}

void PageView::mouseDoubleClickEvent(QMouseEvent *e)
{
    // A barcode picture: a double click selects it and asks for its data again.
    if (m_doc && !m_readOnly && e->button() == Qt::LeftButton && !m_hf.active && !m_note.active) {
        // A barcode field (shown as its bars): the same, on the field.
        if (const int field = barcodeFieldAtPoint(e->position().toPoint()); field >= 0) {
            QTextCursor c = m_cursor;
            c.setPosition(field);
            setCursor(c);
            Q_EMIT editBarcodeFieldWanted(field);
            e->accept();
            return;
        }
        if (const int picture = pictureAtPoint(e->position().toPoint()); picture >= 0 && !pictureAt(*m_doc, m_doc->story(), picture).barcode.isEmpty()) {
            selectPicture(picture);
            Q_EMIT editBarcodeWanted();
            e->accept();
            return;
        }
    }
    if (m_doc && e->button() == Qt::LeftButton && !m_readOnly) {
        // A double click on a shape or a text box edits its words, in place.
        if (const int object = pictureAtPoint(e->position().toPoint()); object >= 0) {
            const PictureInfo info = pictureAt(*m_doc, m_doc->story(), object);
            if (info.kind == DrawnKind::Shape || info.kind == DrawnKind::TextBox) {
                selectPicture(object);
                if (beginShapeTextEdit()) {
                    e->accept();
                    return;
                }
            }
        }
    }
    if (m_doc && e->button() == Qt::LeftButton) {
        const Hit hit = hitAt(e->position().toPoint());
        const Area area = areaOf(hit);
        if (m_hf.active && area == Area::Body) {
            // In the text of a page: the header or footer is left, and the caret goes where the click was.
            leaveHeaderFooter();
            const int at = positionAt(hit);
            if (at >= 0) {
                m_cursor.setPosition(at);
                caretMoved();
            }
            e->accept();
            return;
        }
        if (!m_hf.active && !m_note.active && area == Area::Body && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
            // A footnote's mark: the note is edited where it stands.
            const int at = positionAt(hit);
            if (at >= 0 && openNoteAt(at)) {
                e->accept();
                return;
            }
        }
        if (!m_hf.active && !m_draft && !m_web && area != Area::Body && (leaveNote(), enterHeaderFooter(hit.page, area == Area::Footer))) {
            const int at = positionAt(hit);
            if (at >= 0) {
                m_cursor.setPosition(at);
                caretMoved();
            }
            e->accept();
            return;
        }
    }
    // (A double click is a second press: the press handler counts it.)
    mousePressEvent(e);
}

bool PageView::openNoteAt(int at)
{
    if (!m_doc || m_hf.active || m_cursor.document() != m_doc->story()) return false;
    const int id = footnoteReferenceAt(*m_doc, at);
    return id >= 0 && enterFootnote(id);
}

const QVector<ControlMark> &PageView::controlMarks()
{
    QTextDocument *story = m_doc ? m_doc->story() : nullptr;
    if (!story) {
        m_controlMarks.clear();
        return m_controlMarks;
    }
    if (m_controlStory != story) {
        // (The marks follow the edits: after a change that moves them they are shifted, and only a change that touches a control's atoms, or adds atoms, has them found again.)
        QObject::disconnect(m_controlConnection);
        m_controlStory = story;
        m_controlsDirty = true;
        m_controlConnection = connect(story, &QTextDocument::contentsChange, this, [this, story](int pos, int removed, int added) {
            if (m_controlsDirty) return;
            bool atoms = false;
            if (added > 100000) atoms = true;
            for (int i = 0; i < added && !atoms; ++i) atoms = story->characterAt(pos + i) == QChar::ObjectReplacementCharacter;
            if (!atoms && removed > 0)
                for (const ControlMark &m : std::as_const(m_controlMarks))
                    if ((m.from >= pos && m.from < pos + removed) || (m.to >= pos && m.to < pos + removed)) {
                        atoms = true;
                        break;
                    }
            if (atoms) {
                m_controlsDirty = true;
                return;
            }
            const int delta = added - removed;
            if (delta == 0) return;
            for (ControlMark &m : m_controlMarks) {
                if (m.from >= pos + removed) m.from += delta;
                if (m.to >= pos + removed) m.to += delta;
            }
        });
    }
    if (m_controlsDirty) {
        m_controlsDirty = false;
        m_controlMarks.clear();
        for (const write::ContentControl &c : write::contentControls(*m_doc))
            if (c.props.kind != write::ControlKind::Other) m_controlMarks.push_back({c.start, c.end, false, c.props.title});
    }
    // (The control the caret is in has the outline: the innermost that holds it. Only the caret's paragraph is looked at.)
    for (ControlMark &mark : m_controlMarks) mark.focus = false;
    if (!m_controlMarks.isEmpty() && m_cursor.document() == story) {
        const int at = m_cursor.hasSelection() ? m_cursor.selectionStart() + 1 : m_cursor.position();
        const write::ContentControl here = write::contentControlAt(*m_doc, at);
        if (here.valid())
            for (ControlMark &mark : m_controlMarks)
                if (mark.from == here.start && mark.to == here.end) {
                    mark.focus = true;
                    break;
                }
    }
    return m_controlMarks;
}

write::ContentControl PageView::controlUnder(int at, const QPoint &viewportPos)
{
    // (A click on the first or last letter of a control gives the place before its first atom or after its last: those are asked too.)
    for (int p : {at, at + 1, at - 1}) {
        const write::ContentControl c = write::contentControlAt(*m_doc, p);
        if (!c.valid()) continue;
        QRect a, b;
        if (!positionRect(c.start + 1, &a) || !positionRect(c.end, &b)) continue;
        if (std::abs(a.center().y() - b.center().y()) > 2) {
            if (p == at) return c;   // (over lines: only what the place says)
            continue;
        }
        if (viewportPos.y() >= a.top() && viewportPos.y() <= a.bottom() && viewportPos.x() >= a.left() - 1 && viewportPos.x() <= b.left() + 1) return c;
    }
    return write::ContentControl();
}

bool PageView::clickInControl(int at, const QPoint &viewportPos, const QPoint &globalPos, int clicks)
{
    if (!m_doc || m_readOnly || m_hf.active || m_note.active || m_cursor.document() != m_doc->story() || at < 0) return false;
    const write::ContentControl c = controlUnder(at, viewportPos);
    if (!c.valid() || !c.editable || (clicks > 1 && c.props.kind != write::ControlKind::CheckBox)) return false;   // (a check box flips at each click, however quick; the others are a double click's word)
    switch (c.props.kind) {
    case write::ControlKind::CheckBox:
        if (!c.props.lockContents) write::toggleControlCheck(*m_doc, c.start);
        m_cursor.setPosition(std::min(write::contentControlAt(*m_doc, c.start + 1).end + 1, m_doc->story()->characterCount() - 1));
        break;
    case write::ControlKind::DropDown:
    case write::ControlKind::Date:
        if (c.placeholder) write::selectControlContents(m_cursor, c);
        else m_cursor.setPosition(std::clamp(at, c.start + 1, c.end));
        if (!c.props.lockContents) Q_EMIT controlChoiceWanted(c.start, globalPos);
        break;
    case write::ControlKind::PlainText:
        if (!c.placeholder || c.props.lockContents) return false;
        write::selectControlContents(m_cursor, c);
        break;
    default:
        return false;
    }
    m_dragging = false;
    caretMoved();
    return true;
}

bool PageView::activateControlAtCaret(int key, const QPoint &globalPos)
{
    if (!m_doc || m_readOnly || m_hf.active || m_note.active || m_cursor.document() != m_doc->story() || m_cursor.hasSelection()) return false;
    const write::ContentControl c = write::contentControlAt(*m_doc, m_cursor.position());
    if (!c.valid() || !c.editable || c.props.lockContents) return false;
    if (c.props.kind == write::ControlKind::CheckBox && key == Qt::Key_Space) {
        write::toggleControlCheck(*m_doc, c.start);
        caretMoved();
        return true;
    }
    if ((c.props.kind == write::ControlKind::DropDown || c.props.kind == write::ControlKind::Date) && key != Qt::Key_Space) {
        Q_EMIT controlChoiceWanted(c.start, globalPos);
        return true;
    }
    return false;
}

bool PageView::followLinkAt(int at)
{
    if (!m_doc || m_hf.active || m_note.active || m_cursor.document() != m_doc->story()) return false;
    const Hyperlink link = hyperlinkAt(*m_doc, at);
    if (link.valid() && link.anchor.isEmpty() && !link.address.isEmpty()) {
        if (mayFollow(link.address) || isFileLink(link.address)) Q_EMIT linkFollowed(link.address, link.text);
        else Q_EMIT notice(tr("JeffWrite opens links to web addresses, email addresses, and documents or pictures on this computer only."));
        return true;
    }
    const int target = linkTargetAt(*m_doc, at);
    if (target < 0) return false;
    QTextCursor c(m_doc->story());
    c.setPosition(target);
    showCursor(c);
    return true;
}

void PageView::contextMenuEvent(QContextMenuEvent *e)
{
    if (!m_doc) return;
    if (e->reason() == QContextMenuEvent::Mouse) {
        e->accept();
        Q_EMIT contextMenuWanted(e->globalPos());
        m_contextAt = -1;
        return;
    }
    m_contextAt = -1;   // (from the keyboard it is the caret's)
    QRect r;
    caretRect(&r);
    e->accept();
    Q_EMIT contextMenuWanted(viewport()->mapToGlobal(r.bottomLeft()));
}

void PageView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const double steps = e->angleDelta().y() / 120.0;
        if (steps != 0) setZoom(std::round(m_zoom * 10 + steps) / 10);
        e->accept();
        return;
    }
    if (m_sideToSide && e->angleDelta().x() == 0 && e->angleDelta().y() != 0) {
        // (Pages side by side: the wheel turns them past.)
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - e->angleDelta().y());
        if (!pagesComplete() && horizontalScrollBar()->value() >= horizontalScrollBar()->maximum() - 2) startPagination();
        e->accept();
        return;
    }
    QAbstractScrollArea::wheelEvent(e);
    if (!pagesComplete() && verticalScrollBar()->value() >= verticalScrollBar()->maximum() - 2) startPagination();
}

// ---- dragging text

namespace {
constexpr char kTextDragMime[] = "application/x-jeffwrite-text";
}

QMimeData *PageView::prepareTextDrag()
{
    if (!m_doc || !m_cursor.hasSelection() || m_hf.active || m_note.active || m_cursor.document() != m_doc->story()) return nullptr;
    m_textDrag.selection = m_cursor;
    m_textDrag.active = true;
    m_textDrag.done = false;
    auto *mime = new QMimeData;
    mime->setText(plainTextOf(selectionFragment(m_cursor)));
    mime->setData(QString::fromLatin1(kTextDragMime), QByteArray::number(qulonglong(m_doc->id())));
    return mime;
}

void PageView::startTextDrag()
{
    QPointer<PageView> self(this);
    m_textDrag.pending = false;
    std::unique_ptr<QMimeData> mime(prepareTextDrag());
    if (!mime) return;
    m_dragging = false;
    m_autoscroll.stop();
    auto *drag = new QDrag(viewport());
    drag->setMimeData(mime.release());
    const Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
    if (!self) return;
    // A target outside this view took the text as a move: it leaves here.
    if (result == Qt::MoveAction && !m_textDrag.done && m_doc && m_textDrag.selection.hasSelection() && m_textDrag.selection.document() == m_doc->story()) {
        QTextCursor cut = m_textDrag.selection;
        removeSelection(*m_doc, cut);
        m_cursor = cut;
        caretMoved();
    }
    m_textDrag = TextDrag();
}

bool PageView::wantsTextDrop(const QMimeData *mime) const
{
    return m_doc && !m_readOnly && !m_hf.active && !m_note.active && mime && (mime->hasFormat(QString::fromLatin1(kTextDragMime)) || ((mime->hasText() || mime->hasHtml() || hasRtfFormat(mime)) && !mime->hasUrls()));
}

bool PageView::dropText(QDropEvent *e)
{
    const QMimeData *mime = e->mimeData();
    if (!wantsTextDrop(mime)) return false;
    const int at = positionAt(hitAt(e->position().toPoint()));
    if (at < 0) {
        e->ignore();
        return true;
    }
    if (mime->hasFormat(QString::fromLatin1(kTextDragMime)) && m_textDrag.active && m_textDrag.selection.hasSelection()) {
        Qt::KeyboardModifiers copyKeys = Qt::ControlModifier;
#ifdef Q_OS_MACOS
        copyKeys |= Qt::AltModifier;
#endif
        const bool copy = (e->modifiers() & copyKeys) || e->possibleActions() == Qt::CopyAction;
        QTextCursor moved = m_textDrag.selection;
        if (moveSelectedText(*m_doc, moved, at, copy)) {
            m_cursor = moved;
            caretMoved();
            m_textDrag.done = true;
            e->setDropAction(copy ? Qt::CopyAction : Qt::MoveAction);
            e->accept();
        } else {
            e->ignore();
        }
        return true;
    }
    // Text from another program goes in as a paste does (the same order of forms, smart paste, one step, the Paste Options button); nothing usable in it, the drop is refused.
    const QTextCursor before = m_cursor;
    m_cursor.setPosition(at);
    if (!pasteSource(sourceOfDrop(mime))) {
        m_cursor = before;
        e->ignore();
        return true;
    }
    e->acceptProposedAction();
    return true;
}

} // namespace jp::write
