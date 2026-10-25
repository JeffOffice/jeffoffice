// JeffCrunch's notes and links: the commands (New, Edit, Delete, Show or Hide a note; Insert, Edit, Remove, Follow a link), the note that appears when the pointer rests on a cell that has one,
// the tip a link shows, and what a click on a link does. The notes and links are kept by the session (core/anchors.h); this is how the window shows them and asks for them.

#include "app/crunchwindow.h"

#include "app/anchordialogs.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "core/anchors.h"

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QToolTip>
#include <QUndoStack>
#include <QUrl>

#include <algorithm>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::LinkKind;
using jp::office::sheet::LinkSpec;
using jp::office::sheet::NoteInfo;
using jp::office::sheet::noteKey;

namespace {

// The name the person is known by in the notes they write: the settings' own, else the computer account's.
QString authorName()
{
    QString name = qEnvironmentVariable("USERNAME");
    if (name.isEmpty()) name = qEnvironmentVariable("USER");
    return name.trimmed();
}

// A place a link names ("Sheet2!B5", "'My Sheet'!B5:C9", "Sheet2!$B$5"), as a sheet name and a range; false if it has no sheet in it.
bool splitPlace(const QString &place, QString *sheetName, CellRange *range)
{
    const int bang = int(place.lastIndexOf(QLatin1Char('!')));
    if (bang <= 0) return false;
    QString name = place.left(bang);
    if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
    QString cells = place.mid(bang + 1);
    cells.remove(QLatin1Char('$'));
    if (!jp::office::sheet::parseRangeRef(cells, range)) return false;
    *sheetName = name;
    return true;
}

} // namespace

AnchorHooks &CrunchWindow::anchorHooks() { return m_anchorHooks; }

void CrunchWindow::createNoteActions()
{
    mk("notes.edit", tr("New Note"), "message-square-plus", QKeySequence(Qt::SHIFT | Qt::Key_F2), [this] { editNote(); }, false, false);
    mk("notes.delete", tr("Delete Note"), "message-square-x", QKeySequence(), [this] { deleteSelectedNotes(); }, false, false);
    mk("notes.showHide", tr("Show or Hide Note"), "eye", QKeySequence(), [this] { toggleNoteShown(); }, true, false);
    mk("notes.showAll", tr("Show All Notes"), "sticky-note", QKeySequence(), [this] { showAllNotes(); }, false, false);
    mk("notes.prev", tr("Previous Note"), "chevron-left", QKeySequence(), [this] { goToNote(false); }, false, false);
    mk("notes.next", tr("Next Note"), "chevron-right", QKeySequence(), [this] { goToNote(true); }, false, false);
    mk("insert.link", tr("Link"), "link", QKeySequence(Qt::CTRL | Qt::Key_K), [this] { insertLink(); }, false, false);
    mk("link.open", tr("Open Link"), "external-link", QKeySequence(), [this] { followSelectedLink(); }, false, false);
    mk("link.remove", tr("Remove Link"), "unlink", QKeySequence(), [this] { removeSelectedLinks(); }, false, false);
    m_hoverTimer.setSingleShot(true);
    m_hoverTimer.setInterval(350);
    connect(&m_hoverTimer, &QTimer::timeout, this, &CrunchWindow::showHoverTip);
    connect(m_view, &grid::GridView::hoverCellChanged, this, &CrunchWindow::hoverCell);
    connect(m_view, &grid::GridView::cellClicked, this, &CrunchWindow::cellClicked);
    connect(m_view, &grid::GridView::selectionChanged, this, &CrunchWindow::updateNoteActions);
    connect(m_view, &grid::GridView::viewChanged, this, [this] { hideHoverTip(); });
}

void CrunchWindow::connectAnchors()
{
    connect(m_session.get(), &crunch::EditSession::anchorsChanged, this, [this](int sheet) {
        if (sheet == m_sheet) refreshAnchors();
    });
    connect(m_session->undoStack(), &QUndoStack::indexChanged, this, [this] { updateNoteActions(); });
}

// The grid marks the cells that have notes, and draws the notes that are shown.
void CrunchWindow::refreshAnchors()
{
    hideHoverTip();
    if (!m_session || m_sheet < 0 || m_chartSheetShown) {
        m_view->setNoteCells({});
        m_view->setShownNotes({});
        updateNoteActions();
        return;
    }
    const crunch::CellAnchors &a = m_session->anchors();
    m_view->setNoteCells(a.noteCells(m_sheet));
    std::vector<grid::GridView::ShownNote> shown;
    for (const auto &n : a.notes(m_sheet)) {
        if (!n.second.shown) continue;
        shown.push_back({qint64(n.first >> 32), qint64(n.first & 0xFFFFFFFFu), n.second.text});
        if (shown.size() >= 5000) break;
    }
    m_view->setShownNotes(std::move(shown));
    updateNoteActions();
}

void CrunchWindow::updateNoteActions()
{
    const bool open = m_session && !m_saving && m_sheet >= 0 && !m_chartSheetShown && m_loaded && m_loaded->calc;
    QString why;
    const bool can = open && m_session->canEditNotes(m_sheet, &why);
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const NoteInfo *here = open ? m_session->anchors().noteAt(m_sheet, row, col) : nullptr;
    QAction *edit = act("notes.edit");
    edit->setText(here ? tr("Edit Note") : tr("New Note"));
    edit->setEnabled(can || (here && open));   // (a note that cannot be changed can still be read)
    if (!can && !here) edit->setToolTip(why);
    const CellRange r = selectedRange();
    bool anyInSelection = false, anyShown = false, anyHidden = false;
    if (open) {
        const auto &notes = m_session->anchors().notes(m_sheet);
        for (auto it = notes.lower_bound(noteKey(r.row1, 0)); it != notes.end() && quint32(it->first >> 32) <= r.row2; ++it) {
            const quint32 c = quint32(it->first & 0xFFFFFFFFu);
            if (c < r.col1 || c > r.col2) continue;
            anyInSelection = true;
            (it->second.shown ? anyShown : anyHidden) = true;
        }
    }
    act("notes.delete")->setEnabled(can && anyInSelection);
    QAction *showHide = act("notes.showHide");
    showHide->setEnabled(can && anyInSelection);
    {
        const QSignalBlocker block(showHide);
        showHide->setChecked(anyShown && !anyHidden);
    }
    const bool anyNotes = open && !m_session->anchors().notes(m_sheet).empty();
    bool allShown = anyNotes;
    if (anyNotes)
        for (const auto &n : m_session->anchors().notes(m_sheet))
            if (!n.second.shown) {
                allShown = false;
                break;
            }
    act("notes.showAll")->setText(allShown ? tr("Hide All Notes") : tr("Show All Notes"));
    act("notes.showAll")->setEnabled(can && anyNotes);
    act("notes.prev")->setEnabled(anyNotes);
    act("notes.next")->setEnabled(anyNotes);
    // links
    const LinkSpec *link = open ? m_session->anchors().linkAt(m_sheet, row, col) : nullptr;
    act("insert.link")->setEnabled(can);
    act("insert.link")->setText(link ? tr("Edit Link") : tr("Link"));
    act("link.open")->setEnabled(open && link);
    bool anyLinks = false;
    if (open)
        for (const LinkSpec &l : m_session->anchors().links(m_sheet))
            if (l.range.row1 <= r.row2 && l.range.row2 >= r.row1 && l.range.col1 <= r.col2 && l.range.col2 >= r.col1) {
                anyLinks = true;
                break;
            }
    act("link.remove")->setEnabled(can && anyLinks);
}

// ---- notes

void CrunchWindow::editNote()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || m_sheet < 0) return;
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const NoteInfo *was = m_session->anchors().noteAt(m_sheet, row, col);
    NoteRequest req;
    req.where = jp::office::sheet::cellName(row, col);
    req.author = was ? was->author : authorName();
    if (was) {
        req.text = was->text;
        req.readOnly = was->threaded;
    }
    if (!was) {
        QString why;
        if (!m_session->canEditNotes(m_sheet, &why)) {
            notice(tr("Note"), why);
            return;
        }
    }
    if (m_anchorHooks.noteAsker) {
        if (!m_anchorHooks.noteAsker(&req)) return;
    } else {
        NoteDialog dialog(req, this);
        if (dialog.exec() != QDialog::Accepted) return;
        req.text = dialog.text();
    }
    if (req.readOnly) return;
    if (req.text.trimmed().isEmpty()) {
        // (no words: a new note is not made, and an old one is taken away)
        if (was) deleteSelectedNotes();
        return;
    }
    const crunch::EditSession::Outcome o = m_session->setNote(m_sheet, row, col, req.text, req.author, was ? was->shown : false, was ? tr("Edit Note") : tr("New Note"));
    if (!o.ok) notice(tr("Note"), o.problem);
    m_view->setFocus();
}

void CrunchWindow::deleteSelectedNotes()
{
    if (!m_session || m_sheet < 0) return;
    std::vector<std::pair<quint32, quint32>> cells;
    const CellRange r = selectedRange();
    const auto &notes = m_session->anchors().notes(m_sheet);
    for (auto it = notes.lower_bound(noteKey(r.row1, 0)); it != notes.end() && quint32(it->first >> 32) <= r.row2; ++it) {
        const quint32 c = quint32(it->first & 0xFFFFFFFFu);
        if (c >= r.col1 && c <= r.col2) cells.emplace_back(quint32(it->first >> 32), c);
    }
    const crunch::EditSession::Outcome o = m_session->deleteNotes(m_sheet, cells, cells.size() == 1 ? tr("Delete Note") : tr("Delete Notes"));
    if (!o.ok) notice(tr("Note"), o.problem);
}

void CrunchWindow::toggleNoteShown()
{
    if (!m_session || m_sheet < 0) return;
    std::vector<std::pair<quint32, quint32>> cells;
    const CellRange r = selectedRange();
    bool anyHidden = false;
    const auto &notes = m_session->anchors().notes(m_sheet);
    for (auto it = notes.lower_bound(noteKey(r.row1, 0)); it != notes.end() && quint32(it->first >> 32) <= r.row2; ++it) {
        const quint32 c = quint32(it->first & 0xFFFFFFFFu);
        if (c < r.col1 || c > r.col2) continue;
        cells.emplace_back(quint32(it->first >> 32), c);
        anyHidden = anyHidden || !it->second.shown;
    }
    const crunch::EditSession::Outcome o = m_session->showNotes(m_sheet, cells, anyHidden, anyHidden ? tr("Show Note") : tr("Hide Note"));
    if (!o.ok) notice(tr("Note"), o.problem);
}

void CrunchWindow::showAllNotes()
{
    if (!m_session || m_sheet < 0) return;
    std::vector<std::pair<quint32, quint32>> cells;
    bool anyHidden = false;
    for (const auto &n : m_session->anchors().notes(m_sheet)) {
        cells.emplace_back(quint32(n.first >> 32), quint32(n.first & 0xFFFFFFFFu));
        anyHidden = anyHidden || !n.second.shown;
    }
    const crunch::EditSession::Outcome o = m_session->showNotes(m_sheet, cells, anyHidden, anyHidden ? tr("Show All Notes") : tr("Hide All Notes"));
    if (!o.ok) notice(tr("Note"), o.problem);
}

// The next (or previous) cell with a note, going by rows and wrapping round.
void CrunchWindow::goToNote(bool next)
{
    if (!m_session || m_sheet < 0) return;
    const auto &notes = m_session->anchors().notes(m_sheet);
    if (notes.empty()) return;
    const grid::Selection &sel = m_view->selection();
    const quint64 here = noteKey(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
    quint64 key;
    if (next) {
        auto it = notes.upper_bound(here);
        key = it == notes.end() ? notes.begin()->first : it->first;
    } else {
        auto it = notes.lower_bound(here);
        key = it == notes.begin() ? notes.rbegin()->first : std::prev(it)->first;
    }
    m_view->selectCell(qint64(key >> 32), qint64(key & 0xFFFFFFFFu));
    m_view->ensureVisible(qint64(key >> 32), qint64(key & 0xFFFFFFFFu));
}

// ---- what appears when the pointer rests on a cell

void CrunchWindow::hoverCell(qint64 row, qint64 col)
{
    hideHoverTip();
    if (row < 0 || !m_session || m_sheet < 0) {
        m_hoverTimer.stop();
        return;
    }
    const crunch::CellAnchors &a = m_session->anchors();
    if (a.noteAt(m_sheet, quint32(row), quint32(col)) || a.linkAt(m_sheet, quint32(row), quint32(col))) m_hoverTimer.start();
    else m_hoverTimer.stop();
}

void CrunchWindow::hideHoverTip()
{
    if (m_notePopup) m_notePopup->hide();
    if (m_hoverTipShown) {
        QToolTip::hideText();
        m_hoverTipShown = false;
    }
}

QString CrunchWindow::hoverTipText(bool *isNote) const
{
    if (isNote) *isNote = false;
    const qint64 row = m_view->hoverRow(), col = m_view->hoverCol();
    if (row < 0 || !m_session || m_sheet < 0) return QString();
    const crunch::CellAnchors &a = m_session->anchors();
    if (const NoteInfo *n = a.noteAt(m_sheet, quint32(row), quint32(col))) {
        if (isNote) *isNote = true;
        return n->text;
    }
    if (const LinkSpec *l = a.linkAt(m_sheet, quint32(row), quint32(col))) {
        QString where = l->tip;
        if (where.isEmpty()) where = l->target.isEmpty() ? l->location : l->target;
        return where + QLatin1Char('\n') + tr("Ctrl+click to follow");
    }
    return QString();
}

void CrunchWindow::showHoverTip()
{
    bool isNote = false;
    const QString text = hoverTipText(&isNote);
    if (text.isEmpty()) return;
    const qint64 row = m_view->hoverRow(), col = m_view->hoverCol();
    const QRect cell = m_view->cellRectAt(row, col);
    const QPoint global = m_view->viewport()->mapToGlobal(QPoint(cell.right() + 4, cell.top()));
    if (!isNote) {
        QToolTip::showText(global, text, m_view->viewport(), cell);
        m_hoverTipShown = true;
        return;
    }
    // (a note shown all the time is drawn by the grid already)
    if (const NoteInfo *n = m_session->anchors().noteAt(m_sheet, quint32(row), quint32(col)); n && n->shown) return;
    if (!m_notePopup) {
        auto *label = new QLabel(this, Qt::ToolTip | Qt::FramelessWindowHint);
        label->setObjectName(QStringLiteral("notePopup"));
        label->setWordWrap(true);
        label->setMargin(5);
        label->setMaximumWidth(280);
        label->setStyleSheet(QStringLiteral("QLabel { background: #ffffe1; color: #202020; border: 1px solid #808060; }"));
        label->setAttribute(Qt::WA_ShowWithoutActivating);
        label->setFocusPolicy(Qt::NoFocus);
        m_notePopup = label;
    }
    auto *label = static_cast<QLabel *>(m_notePopup.data());
    label->setText(text.size() > 4000 ? text.left(4000) + QChar(0x2026) : text);
    label->adjustSize();
    label->move(global);
    label->show();
}

// ---- links

void CrunchWindow::insertLink()
{
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || m_sheet < 0) return;
    QString why;
    if (!m_session->canEditNotes(m_sheet, &why)) {
        notice(tr("Link"), why);
        return;
    }
    const CellRange range = selectedRange();
    const grid::Selection &sel = m_view->selection();
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    const LinkSpec *was = m_session->anchors().linkAt(m_sheet, row, col);
    LinkRequest req;
    req.where = range.cellCount() > 1 ? jp::office::sheet::rangeName(range) : jp::office::sheet::cellName(row, col);
    req.shownText = m_session->displayText(m_sheet, range.row1, range.col1);
    req.editing = was != nullptr;
    if (was) req.link = *was;
    for (const int i : m_session->sheetOrder())
        if (i >= 0 && i < m_loaded->book->info().sheets.size()) {
            if (i == m_sheet) req.sheet = int(req.sheetNames.size());
            req.sheetNames << m_loaded->book->info().sheets[i].name;
        }
    for (const auto &n : m_loaded->book->info().definedNames)
        if (!n.hidden && n.localSheet < 0 && !n.name.startsWith(QLatin1String("_xlnm.")) && req.names.size() < 500) req.names << n.name;
    if (m_anchorHooks.linkAsker) {
        if (!m_anchorHooks.linkAsker(&req)) return;
    } else {
        LinkDialog dialog(req, this);
        for (;;) {
            if (dialog.exec() != QDialog::Accepted) return;
            QString problem;
            if (dialog.result(&req, &problem)) break;
        }
    }
    const CellRange target = was && !(range.cellCount() > 1) ? was->range : range;
    crunch::EditSession::Outcome o = m_session->checkLink(m_sheet, target, req.link);
    if (!o.ok) {
        notice(tr("Link"), o.problem);
        return;
    }
    // (the link, the words the cell shows, and the look of a link are one step to a person)
    QUndoStack *u = undoStack();
    u->beginMacro(was ? tr("Change Link") : tr("Put In a Link"));
    o = m_session->setLink(m_sheet, target, req.link, was ? tr("Change Link") : tr("Put In a Link"));
    if (o.ok && range.cellCount() == 1) {
        QString words = req.shownTextOut;
        if (words.isEmpty() && req.shownText.isEmpty()) words = req.link.target.isEmpty() ? req.link.location : req.link.target;
        if (!words.isEmpty() && words != req.shownText) m_session->type(m_sheet, row, col, words);
    }
    if (o.ok && !was) m_session->applyStyleNamed(m_sheet, range, QStringLiteral("Hyperlink"), tr("Put In a Link"));
    u->endMacro();
    if (!o.ok) notice(tr("Link"), o.problem);
    updateBars();
}

void CrunchWindow::removeSelectedLinks()
{
    if (!m_session || m_sheet < 0) return;
    const crunch::EditSession::Outcome o = m_session->removeLinks(m_sheet, selectedRange(), tr("Remove Link"));
    if (!o.ok) notice(tr("Link"), o.problem);
}

void CrunchWindow::followSelectedLink()
{
    if (!m_session || m_sheet < 0) return;
    const grid::Selection &sel = m_view->selection();
    followLinkAt(quint32(std::max<qint64>(0, sel.activeRow)), quint32(std::max<qint64>(0, sel.activeCol)));
}

// A click on a cell: with Ctrl pressed it follows the link on the cell. (Nothing is opened but by a click or a command.)
void CrunchWindow::cellClicked(qint64 row, qint64 col, Qt::KeyboardModifiers modifiers)
{
    if (!(modifiers & Qt::ControlModifier) || !m_session || m_sheet < 0) return;
    if (m_session->anchors().linkAt(m_sheet, quint32(row), quint32(col))) followLinkAt(quint32(row), quint32(col));
}

bool CrunchWindow::openAddress(const QUrl &url)
{
    if (m_anchorHooks.opener) return m_anchorHooks.opener(url);
    return QDesktopServices::openUrl(url);
}

void CrunchWindow::followLinkAt(quint32 row, quint32 col)
{
    if (!m_session) return;
    const LinkSpec *found = m_session->anchors().linkAt(m_sheet, row, col);
    if (!found) return;
    const LinkSpec link = *found;
    switch (jp::office::sheet::linkKind(link)) {
    case LinkKind::Place: {
        goToPlace(link.location);
        return;
    }
    case LinkKind::Unsafe:
        notice(tr("Link"), tr("This link runs a script or carries a page inside itself, so JeffCrunch does not follow it."));
        return;
    case LinkKind::Web:
    case LinkKind::Mail: {
        const QUrl url(link.target, QUrl::TolerantMode);
        if (!url.isValid() || url.scheme().isEmpty()) {
            notice(tr("Link"), tr("That address can't be opened."));
            return;
        }
        if (!openAddress(url)) notice(tr("Link"), tr("The address could not be opened."));
        return;
    }
    case LinkKind::File:
    case LinkKind::Other: {
        QUrl url;
        if (jp::office::sheet::linkKind(link) == LinkKind::File) {
            // (a file the link names, beside the workbook if its name is not whole)
            QString path = link.target;
            if (path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) path = QUrl(path).toLocalFile();
            path.replace(QLatin1Char('\\'), QLatin1Char('/'));
            if (!QDir::isAbsolutePath(path) && !m_path.isEmpty()) path = QFileInfo(m_path).absolutePath() + QLatin1Char('/') + path;
            url = QUrl::fromLocalFile(path);
        } else {
            url = QUrl(link.target, QUrl::TolerantMode);
        }
        if (!confirm(tr("Link"), tr("This link opens %1 with another program. Open it?").arg(link.target.left(300)))) return;
        if (!url.isValid() || !openAddress(url)) notice(tr("Link"), tr("The address could not be opened."));
        return;
    }
    }
}

// A place in the workbook: a cell or cells of a sheet, or a defined name.
void CrunchWindow::goToPlace(const QString &place)
{
    if (!m_loaded || !m_loaded->book) return;
    QString where = place.trimmed();
    QString sheetName;
    CellRange range;
    if (!splitPlace(where, &sheetName, &range)) {
        // a defined name
        for (const auto &n : m_loaded->book->info().definedNames)
            if (n.name.compare(where, Qt::CaseInsensitive) == 0 && n.localSheet < 0) {
                where = n.ref;
                break;
            }
        if (!splitPlace(where, &sheetName, &range)) {
            notice(tr("Link"), tr("That place isn't in this workbook any more."));
            return;
        }
    }
    int sheet = -1;
    for (int i = 0; i < m_loaded->book->info().sheets.size(); ++i) {
        const auto &s = m_loaded->book->info().sheets[i];
        if (!s.deleted && s.name.compare(sheetName, Qt::CaseInsensitive) == 0) {
            sheet = i;
            break;
        }
    }
    if (sheet < 0) {
        notice(tr("Link"), tr("That place isn't in this workbook any more."));
        return;
    }
    setActiveSheet(sheet);
    m_view->selectRange(range.row1, range.col1, range.row2, range.col2);
    m_view->ensureVisible(range.row1, range.col1);
    m_view->setFocus();
}

} // namespace jp
