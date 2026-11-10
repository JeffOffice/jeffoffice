// Editing in JeffCrunch's window: typing over a cell, in it (F2, a double click), or in the formula bar; Delete; Undo and Redo. What is typed is read and put in the cell by the
// EditSession (core/editsession.h), which holds the worker back for the moment the cell is put in; the worker works out what depends on it and tells the window in batches.

#include "app/crunchwindow.h"

#include "core/autocomplete.h"
#include "text/autocorrect.h"
#include "core/fixeddecimals.h"
#include "app/functionargumentsdialog.h"
#include "app/formulahelper.h"
#include "app/settings.h"

#include "app/sheettabs.h"
#include "core/sheetobjects.h"
#include "core/editsession.h"
#include "grid/gridview.h"
#include "sheet/cellref.h"

#include <algorithm>
#include <vector>
#include <QAction>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLineEdit>
#include <QUndoStack>
#include "app/plaintext.h"

namespace jp {

QUndoStack *CrunchWindow::undoStack() const { return m_session ? m_session->undoStack() : nullptr; }

void CrunchWindow::createEditing()
{
    m_session = std::make_unique<crunch::EditSession>(&m_loader, m_loaded, &m_values, nullptr);
    applyCustomListLines();
    m_formatShown = FormatShown();   // (a workbook of its own: the buttons show its cell with the cursor)
    connect(m_session.get(), &crunch::EditSession::cellsChanged, this, &CrunchWindow::onCellsChanged);
    connect(m_session.get(), &crunch::EditSession::cellsMoved, this, &CrunchWindow::onCellsMoved);
    connect(m_session.get(), &crunch::EditSession::sizesChanged, this, &CrunchWindow::onSizesChanged);
    // the grid asks for the formatting inside a text, and draws a sheet's texts again when it changes
    m_view->sheet().setRichSource([this](quint32 row, quint32 col, jp::office::sheet::RichText *out) {
        if (!m_session || !m_session->mayHaveRich(m_sheet)) return false;
        const std::optional<jp::office::sheet::RichText> r = m_session->richOf(m_sheet, row, col);
        if (!r) return false;
        *out = *r;
        return true;
    });
    connect(m_session.get(), &crunch::EditSession::richTextChanged, this, [this](int sheet) {
        if (sheet != m_sheet) return;
        m_view->sheet().invalidateTexts();
        m_view->viewport()->update();
    });
    connect(m_session.get(), &crunch::EditSession::sparklinesChanged, this, [this](int sheet) {
        if (sheet != m_sheet) return;
        m_view->sheet().resetSparklines();
        m_view->viewport()->update();
        updateSparklineTab();
    });
    connect(m_session.get(), &crunch::EditSession::sheetsChanged, this, &CrunchWindow::onSheetsChanged);
    connect(m_session.get(), &crunch::EditSession::conditionalFormatsChanged, this, [this](int sheet) {
        if (sheet == m_sheet) m_view->conditionalFormatsChanged();
        updateCondActions();
    });
    connect(m_session.get(), &crunch::EditSession::tablesChanged, this, &CrunchWindow::onTablesChanged);
    // (every step that changes the workbook sends the worker to work it out: the window is calculating from then until the worker says it is done, whichever command made the step; the
    // pivot table commands, among others, don't say so themselves, and the window was then taken for ready while the worker still had a pass to make)
    m_changesNoted = m_loader.changesMade();
    connect(m_session->undoStack(), &QUndoStack::indexChanged, this, [this] {
        const qint64 now = m_loader.changesMade();
        if (now == m_changesNoted) return;
        m_changesNoted = now;
        if (m_haveCalc && !m_loading) m_calculating = true;
    });
    connectObjects();
    connectPageSetup();
    connectAnchors();
    connectProtection();
    connect(m_session.get(), &crunch::EditSession::calcSettingsChanged, this, [this] { sendCalcSettings(); });
    sendCalcSettings();   // (how the file says it calculates)
    connect(m_session.get(), &crunch::EditSession::stepFailed, this, [this](const QString &what) {
        notice(tr("Undo"), what);
        updateEditActions();
    });
    connect(m_session.get(), &crunch::EditSession::fillFinished, this, &CrunchWindow::updateEditActions);   // (Undo and Redo are on again when the last slice is in)
    QUndoStack *u = m_session->undoStack();
    connect(u, &QUndoStack::indexChanged, this, [this] { removeArrows(); });   // (the arrows are of the cells as they were)
    connect(u, &QUndoStack::canUndoChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::canRedoChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::indexChanged, this, &CrunchWindow::updateEditActions);   // (the oldest steps may be forgotten: Undo ends before them)
    connect(u, &QUndoStack::indexChanged, this, [this] { cancelCut(); });   // (any edit ends a cut)
    connect(u, &QUndoStack::indexChanged, this, [this] { updateFormatState(false); });   // (the buttons show the format the cell has now)
    connect(u, &QUndoStack::indexChanged, this, [this] { updatePivotTab(); });   // (the fields pane shows the table as an Undo or Redo leaves it, and drops changes waiting for Update that were made on another version)
    connect(u, &QUndoStack::undoTextChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::redoTextChanged, this, &CrunchWindow::updateEditActions);
    connect(u, &QUndoStack::cleanChanged, this, [this](bool clean) {
        setWindowModified(!clean);
        updateEditActions();
    });
    setWindowModified(false);
    setLocale(QLocale());
    updateEditActions();
}

void CrunchWindow::setLocale(const QLocale &locale)
{
    if (m_session) m_session->setLocale(locale);
    m_view->sheet().setNumberSymbols(jp::calc::NumberSymbols::of(locale));
    m_view->invalidateValues();
    m_formatShown = FormatShown();   // (the number format box shows the marks of the language)
    updateBars();
}

void CrunchWindow::updateEditActions()
{
    QUndoStack *u = undoStack();
    const bool open = m_session && !m_saving && !isReadOnly() && !m_argsDialog;
    const bool stepping = m_applying || (m_session && m_session->isSlicing());   // (Undo and Redo wait while a large change is being put in)
    act("edit.undo")->setEnabled(open && !stepping && m_session->canUndo());
    act("edit.redo")->setEnabled(open && !stepping && u->canRedo());
    act("edit.undo")->setToolTip(jp::plainToolTip(u && m_session->canUndo() ? tr("Undo %1 (%2)").arg(u->undoText(), QKeySequence(QKeySequence::Undo).toString(QKeySequence::NativeText)) : tr("Undo")));
    act("edit.redo")->setToolTip(jp::plainToolTip(u && u->canRedo() ? tr("Redo %1 (%2)").arg(u->redoText(), QKeySequence(QKeySequence::Redo).toString(QKeySequence::NativeText)) : tr("Redo")));
    // (a chart sheet has no cells to work on: none of the commands that do is on while one is in view, and they are back on a sheet of cells)
    const std::vector<const char *> &pivotOwned = pivotSelectionActions();
    for (const QString &id : m_opsActions) {
        QAction *a = act(id);
        if (!a) continue;
        const bool allowed = (open || (m_session && !m_saving && isReadOnly() && commandAllowedReadOnly(id))) && m_loaded && m_loaded->problem.isEmpty() && !m_chartSheetShown;   // (Find and Go To work in a workbook open for reading only; Replace and the rest do not)
        // (Commands another update turns on from what it knows are only turned off here: turning them on, and that update off again, at every edit made Qt 6.11 slow in proportion to the edits so far.)
        const bool owned = id == QLatin1String("pivot.reportPages") || id == QLatin1String("printarea.clear") || std::any_of(pivotOwned.begin(), pivotOwned.end(), [&](const char *own) { return id == QLatin1String(own); });
        if (!owned || !allowed) a->setEnabled(allowed);
    }
    for (const char *id : {"edit.copy", "edit.selectAll", "view.formulas", "view.gridlines", "view.headings", "view.freeze", "view.freezeRow", "view.freezeCol", "view.unfreeze", "view.split"})
        if (QAction *a = act(QString::fromLatin1(id))) a->setEnabled(!m_chartSheetShown);
    const bool canSave = m_session && !m_saving && m_loaded && m_loaded->problem.isEmpty();
    act("file.save")->setEnabled(canSave);
    act("file.saveAs")->setEnabled(canSave);
    act("file.saveCopy")->setEnabled(canSave);
    act("file.saveTemplate")->setEnabled(canSave);
    updatePivotFieldActions();
    updatePrintAreaAction();
    updateSparklineTab();
    updateFormatState(false);
}

// Whether a cell can be changed now; if not, the person is told why (as a note in the status line while the workbook is still being opened, else in a message).
bool CrunchWindow::editingAllowed()
{
    QString why;
    if (isReadOnly()) {
        setStatus(tr("This workbook is open for reading only. Use Save a Copy to make a copy you can change."));
        return false;
    }
    if (!m_session) {
        setStatus(m_blank ? tr("The new workbook is getting ready. It can be changed in a moment.") : tr("The workbook is still being opened. It can be changed when it is open."));
        return false;
    }
    if (m_applying) {
        setStatus(tr("A large change is still being made. It can be changed again when that is done."));
        return false;
    }
    if (m_argsDialog) {
        setStatus(tr("Finish the Function Arguments dialog first."));
        return false;
    }
    if (!m_session->canEdit(&why)) {
        if (m_saving) setStatus(why);
        else notice(tr("Edit"), why);
        return false;
    }
    return true;
}

void CrunchWindow::beginEdit(const QString &initial, CellEditor::Mode mode, bool fromBar)
{
    if (m_edit.active) return;
    if (m_objects && m_objects->isChartSheet(m_sheet)) return;   // (a chart sheet has no cells to type in)
    if (!editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    const int sheet = m_sheet;
    const quint32 row = quint32(std::max<qint64>(0, sel.activeRow)), col = quint32(std::max<qint64>(0, sel.activeCol));
    QString why;
    const bool forcedToBar = !fromBar && !m_editInCell;
    if (forcedToBar) fromBar = true;
    if (!m_session->canEditCell(sheet, row, col, &why)) {
        // a cell in a range that has a password is opened with it
        const QString title = m_session->passwordRangeAt(sheet, row, col);
        if (!title.isEmpty() && !m_passwordJob) {
            jp::office::sheet::PasswordHash hash;
            for (const jp::office::sheet::EditRange &r : m_session->protectionOf(sheet).editRanges)
                if (r.title == title) hash = r.password;
            askPassword(title, hash, [this, sheet, title, initial, mode, fromBar] {
                if (m_session && m_session->openEditRange(sheet, title)) beginEdit(initial, mode, fromBar);
            });
            return;
        }
        notice(tr("Edit"), why);
        return;
    }
    m_edit.active = true;
    m_autoTyped = initial;
    m_autoColumn.clear();
    m_autoColumnKnown = false;
    m_point = PointState();
    m_point.line = fromBar ? static_cast<QLineEdit *>(m_formulaBar) : static_cast<QLineEdit *>(m_cellEditor);
    m_edit.sheet = sheet;
    m_edit.row = row;
    m_edit.col = col;
    m_edit.original = m_formulaFull;
    m_edit.view = m_view->viewState();
    m_editRich = jp::office::sheet::RichText();
    if (mode == CellEditor::Mode::Editing && !fromBar) {
        const std::optional<jp::office::sheet::RichText> runs = m_session->richOf(sheet, row, col);
        if (runs && runs->text == initial) m_editRich = *runs;
    }
    m_editRichFirst = m_editRich;
    m_view->ensureVisible(row, col);
    m_syncingEditors = true;
    m_cellEditor->setMode(mode);
    m_cellEditor->setText(initial);
    if (!fromBar || forcedToBar) m_formulaBar->setText(initial);
    m_syncingEditors = false;
    m_cellEditor->show();
    m_cellEditor->raise();
    placeEditor();
    if (forcedToBar) {
        // (editing in the formula bar only: the strip over the cell is not shown)
        m_cellEditor->hide();
        m_formulaBar->setFocus(Qt::OtherFocusReason);
        m_formulaBar->setCursorPosition(int(initial.size()));
    }
    if (!fromBar) {
        m_cellEditor->setFocus(Qt::OtherFocusReason);
        m_cellEditor->setCursorPosition(int(initial.size()));
        m_formulaBar->setCursorPosition(int(initial.size()));
        // (the first key typed over a cell starts an entry that may be finished too)
        if (mode == CellEditor::Mode::Typing && !initial.isEmpty()) {
            m_autoTyped.clear();
            autoCompleteEntry(initial);
        }
    }
    updatePointViews();
}

void CrunchWindow::placeEditor()
{
    if (!m_edit.active) return;
    const bool onPages = m_viewKind == 2 && m_pageView && m_pageView->isVisible();
    // (over the pages, the strip is over the cell on its page, and the zoom is the pages')
    const QRect r = onPages ? m_pageView->cellRect(m_edit.row, m_edit.col) : m_view->cellRect(m_edit.row, m_edit.col);
    if (r.isEmpty() || m_edit.sheet != m_sheet) {
        m_cellEditor->hide();
        return;
    }
    // the font of the cell, as the grid shows it
    const grid::GridSheet &gs = m_view->sheet();
    const grid::Cell *cell = gs.cellAt(m_edit.row, m_edit.col);
    const grid::CellLook &look = gs.looks().look(gs.styleOf(m_edit.row, m_edit.col, cell));
    QFont f(look.family.isEmpty() ? m_view->font().family() : look.family);
    f.setPixelSize(std::max(9, int(std::lround(look.pointSize * 96.0 / 72.0 * (onPages ? m_pageView->zoom() : m_view->zoom())))));
    f.setBold(look.bold);
    f.setItalic(look.italic);
    m_cellEditor->setFont(f);
    // as wide as the cell, and wider as the text grows, as far as the edge of the grid
    const QRect vp = onPages ? m_pageView->viewport()->rect() : m_view->viewport()->rect();
    const int want = QFontMetrics(f).horizontalAdvance(m_cellEditor->text()) + 16;
    const int width = std::max(r.width(), std::min(want, std::max(r.width(), vp.right() - r.left())));
    m_cellEditor->setGeometry(r.left(), r.top(), width, r.height());
    m_cellEditor->show();
}

void CrunchWindow::setSpellAsYouType(bool on)
{
    jp::setSpellAsYouType(on);   // (the suite's one choice; every open editor follows)
}

void CrunchWindow::setAutoComplete(bool on)
{
    m_autoComplete = on;
    Settings::get().setValue(QStringLiteral("crunch/autoComplete"), on);
}

void CrunchWindow::setShowZeros(bool on)
{
    if (m_view->showZeros() == on) return;
    m_view->setShowZeros(on);
    syncPanes();
    if (m_session && m_session->viewsChanged()) setWindowModified(true);
}

void CrunchWindow::setEditInCell(bool on)
{
    m_editInCell = on;
    Settings::get().setValue(QStringLiteral("crunch/editInCell"), on);
}

void CrunchWindow::setGridlineColor(const QColor &color)
{
    if (!color.isValid()) return;
    m_view->setGridlineColor(color);
    Settings::get().setValue(QStringLiteral("crunch/gridColor"), color.name());
}

int CrunchWindow::defaultZoom() const { return std::clamp(Settings::get().value(QStringLiteral("crunch/defaultZoom"), 100).toInt(), 10, 400); }

void CrunchWindow::setDefaultZoom(int percent) { Settings::get().setValue(QStringLiteral("crunch/defaultZoom"), std::clamp(percent, 10, 400)); }

bool CrunchWindow::removePersonalOnSave() const { return Settings::get().value(QStringLiteral("crunch/removePersonalOnSave"), false).toBool(); }

void CrunchWindow::setRemovePersonalOnSave(bool on) { Settings::get().setValue(QStringLiteral("crunch/removePersonalOnSave"), on); }

void CrunchWindow::setStandardColumnWidth(double shown)
{
    if (!m_session || m_sheet < 0 || shown <= 0 || !editingAllowed()) return;
    const int mdw = m_view->sheet().looks().maxDigitWidth();
    const crunch::EditSession::Outcome o = m_session->setDefaultSizes(m_sheet, grid::GridSheet::widthOfShown(shown, mdw), std::nullopt, tr("Default Width"));
    if (!o.ok) notice(tr("Default Width"), o.problem);
}

void CrunchWindow::setDragEditing(bool on)
{
    m_view->setDragEditing(on);
    Settings::get().setValue(QStringLiteral("crunch/dragEditing"), on);
}

void CrunchWindow::setFixedDecimals(int places)
{
    m_fixedDecimals = std::clamp(places, -crunch::kMaxFixedDecimals, crunch::kMaxFixedDecimals);
    Settings::get().setValue(QStringLiteral("crunch/fixedDecimals"), m_fixedDecimals);
}

// After a key typed in the cell: when one text in the column starts with what is typed, the rest of it is put in and selected (the next key replaces it, Backspace takes it away, Enter keeps it).
void CrunchWindow::autoCompleteEntry(const QString &text)
{
    const bool grew = text.size() > m_autoTyped.size();
    m_autoTyped = text;
    if (!m_autoComplete || !m_edit.active || !grew || m_cellEditor->cursorPosition() != text.size() || m_edit.sheet != m_sheet) return;
    if (!m_autoColumnKnown) {
        m_autoColumn = crunch::columnTextsAround(*m_session, m_edit.sheet, m_edit.row, m_edit.col);
        m_autoColumnKnown = true;
    }
    const QString whole = crunch::autoCompleted(m_autoColumn, text);
    if (whole.isEmpty()) return;
    // (the entry's own capitals are shown, and what was typed stays before the selected rest)
    const QString shown = whole;
    m_syncingEditors = true;
    m_cellEditor->setText(shown);
    m_cellEditor->setSelection(int(shown.size()), -int(shown.size() - text.size()));
    m_formulaBar->setText(shown);
    m_syncingEditors = false;
}

void CrunchWindow::setEnterMove(EnterMove move)
{
    m_enterMove = move;
    Settings::get().setValue(QStringLiteral("crunch/enterMove"), int(move));
}

QStringList CrunchWindow::customListLines() const
{
    return Settings::get().value(QStringLiteral("crunch/customLists")).toStringList();
}

// The lists of lines of text as the session has them: items between commas, trimmed, with the limits of what one may hold; `kept` gets the lines as they are then written.
static QList<QStringList> parseCustomLists(const QStringList &lines, QStringList *kept)
{
    QList<QStringList> lists;
    for (const QString &line : lines) {
        QStringList items;
        for (const QString &item : line.split(QLatin1Char(','))) {
            const QString t = item.trimmed();
            if (!t.isEmpty() && t.size() <= crunch::EditSession::kMaxCustomItemChars && items.size() < crunch::EditSession::kMaxCustomListItems) items << t;
        }
        if (items.size() < 2 || lists.size() >= crunch::EditSession::kMaxCustomLists) continue;   // (a list of one item makes no series)
        if (kept) *kept << items.join(QStringLiteral(", "));
        lists << items;
    }
    return lists;
}

QStringList CrunchWindow::customListLinesRead() const
{
    QStringList lines;
    parseCustomLists(customListLines(), &lines);
    return lines;
}

// What was typed in Options: the lists are kept in the settings (only when they differ from what is there) and given to the session.
void CrunchWindow::setCustomListLines(const QStringList &lines)
{
    QStringList kept, was;
    const QList<QStringList> lists = parseCustomLists(lines, &kept);
    parseCustomLists(customListLines(), &was);   // (the lists as they stand are compared as they are read: only a change of the lists is written)
    if (kept != was) Settings::get().setValue(QStringLiteral("crunch/customLists"), kept);
    if (m_session) m_session->setCustomLists(lists);
}

// The lists the settings hold, given to a session as they are (nothing is written: a workbook opening does not change the person's settings).
void CrunchWindow::applyCustomListLines()
{
    if (m_session) m_session->setCustomLists(parseCustomLists(customListLines(), nullptr));
}

bool CrunchWindow::commitEdit(CellEditor::Way way)
{
    if (!m_edit.active) return true;
    returnToEditSheet();   // (an entry made while pointing at another sheet is put in its own cell, and the selection goes on from there)
    // (a number typed over a cell gets its decimal point where Options says; what was edited in the cell, or in the formula bar, is entered as it is)
    const QString text = m_cellEditor->mode() == CellEditor::Mode::Typing && !m_formulaBar->hasFocus() ? crunch::withFixedDecimals(m_cellEditor->text(), m_fixedDecimals, QLocale().decimalPoint()) : m_cellEditor->text();
    // What F2 showed, put in again as it is (Enter, or a click on another cell), is not a change: nothing is put in, there is nothing to undo, and the cell keeps what it had (a formula
    // with a function this program lacks keeps the result the file stored, and is not written again as this program reads it back)
    const bool richNow = m_editRich.text == text && !m_editRich.isEmpty();
    const bool sameRuns = m_editRich.text == m_editRichFirst.text && m_editRich.runs == m_editRichFirst.runs && m_editRich.fonts == m_editRichFirst.fonts;
    if (m_cellEditor->mode() == CellEditor::Mode::Editing && text == m_session->editText(m_edit.sheet, m_edit.row, m_edit.col)) {
        // (the text is as it was: only the formatting inside it may be new)
        if (!sameRuns || (!richNow && !m_editRichFirst.isEmpty())) {
            const crunch::EditSession::Outcome o = m_session->setRichText(m_edit.sheet, m_edit.row, m_edit.col, richNow ? m_editRich : jp::office::sheet::RichText(), tr("Format Text"));
            if (!o.ok) {
                notice(tr("Format Text"), o.problem);
                return false;
            }
        }
        finishEdit(way);
        return true;
    }
    const qint64 before = m_loader.changesMade();
    const int stackBefore = m_session->undoStack()->index();
    // Ctrl+Enter puts the entry in every selected cell
    const grid::GridRange block = m_view->selection().range;
    const bool fill = way == CellEditor::Way::Fill && m_edit.sheet == m_sheet && !m_view->selection().isOneCell();
    auto enter = [&](bool checkRule) {
        return fill ? m_session->typeInto(m_edit.sheet, block.row1, block.col1, block.row2, block.col2, m_edit.row, m_edit.col, text, checkRule)
                    : m_session->type(m_edit.sheet, m_edit.row, m_edit.col, text, checkRule, richNow ? &m_editRich : nullptr);
    };
    crunch::EditSession::Outcome o = enter(true);
    if (!o.ok && o.rule) {
        // the rule for the cell does not allow it: a rule that stops is told, the others ask whether to go on
        if (askAboutRule(o)) {
            o = enter(false);
        } else {
            QLineEdit *w = m_formulaBar->hasFocus() ? static_cast<QLineEdit *>(m_formulaBar) : static_cast<QLineEdit *>(m_cellEditor);
            w->setFocus(Qt::OtherFocusReason);
            return false;
        }
    }
    if (!o.ok) {
        // the entry stays as it was typed, with the cursor where the fault is
        QLineEdit *w = m_formulaBar->hasFocus() ? static_cast<QLineEdit *>(m_formulaBar) : static_cast<QLineEdit *>(m_cellEditor);
        notice(o.problemAt >= 0 ? tr("Formula") : tr("Edit"), o.problem);
        if (o.problemAt >= 0) w->setCursorPosition(std::min<int>(o.problemAt, int(text.size())));
        w->setFocus(Qt::OtherFocusReason);
        return false;
    }
    if (m_loader.changesMade() != before) m_calculating = true;
    if (m_session->undoStack()->index() > stackBefore) widenForEntry(m_edit.sheet, m_edit.row, m_edit.col);   // (an entry that made no step has none to widen in)
    if (!fill) linkEntry(text);
    finishEdit(way);
    return true;
}

// An entry that is a web or email address on its own becomes a link in its cell (when Options > Proofing turns web addresses into links as they are typed); never a formula.
void CrunchWindow::linkEntry(const QString &text)
{
    if (!Settings::get().value(QStringLiteral("proof/autolink"), true).toBool() || CellEditor::isFormulaEntry(text)) return;
    const std::optional<jp::autotext::Fix> address = jp::autotext::linkWord(text);
    if (!address || address->from != 0 || address->to != int(text.size())) return;
    jp::office::sheet::LinkSpec link;
    link.range = jp::office::sheet::CellRange{m_edit.row, m_edit.col, m_edit.row, m_edit.col, true};
    link.target = address->with;
    link.display = text;
    if (m_session->anchors().linkAt(m_edit.sheet, m_edit.row, m_edit.col) || !m_session->checkLink(m_edit.sheet, link.range, link).ok) return;
    m_session->setLink(m_edit.sheet, link.range, link, tr("Put In a Link"));
}

void CrunchWindow::finishEdit(CellEditor::Way way)
{
    if (m_helper) m_helper->close();
    m_cellEditor->hide();
    const quint32 row = m_edit.row, col = m_edit.col;
    m_edit.active = false;
    m_editRich = m_editRichFirst = jp::office::sheet::RichText();
    updatePointViews();   // (the outlines go, and the colors of the text in both lines)
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    m_view->setFocus(Qt::OtherFocusReason);
    using Way = CellEditor::Way;
    // Tab after Tab goes on along the row, and Enter then goes down to the column the Tabs began in
    if (way == Way::Tab || way == Way::ShiftTab) {
        if (m_tabOriginCol < 0 || m_tabRow != qint64(row)) {
            m_tabOriginCol = col;
            m_tabRow = row;
        }
    }
    const bool backToOrigin = way == Way::Enter && m_enterMove == EnterMove::Down && m_tabOriginCol >= 0 && m_tabRow == qint64(row);
    const qint64 origin = m_tabOriginCol;
    m_finishing = true;
    switch (way) {
    case Way::Stay:
    case Way::Fill: break;
    case Way::Enter:
    case Way::ShiftEnter: {
        // (Shift goes the other way)
        const bool back = way == Way::ShiftEnter;
        switch (m_enterMove) {
        case EnterMove::Down: m_view->moveAfterEnter(back); break;
        case EnterMove::Up: m_view->moveAfterEnter(!back); break;
        case EnterMove::Right: m_view->moveAfterTab(back); break;
        case EnterMove::Left: m_view->moveAfterTab(!back); break;
        case EnterMove::Stay: break;
        }
        break;
    }
    case Way::Tab: m_view->moveAfterTab(false); break;
    case Way::ShiftTab: m_view->moveAfterTab(true); break;
    case Way::Up: m_view->moveAfterArrow(-1, 0); break;
    case Way::Down: m_view->moveAfterArrow(1, 0); break;
    case Way::Left: m_view->moveAfterArrow(0, -1); break;
    case Way::Right: m_view->moveAfterArrow(0, 1); break;
    }
    if (backToOrigin && m_view->selection().isOneCell()) m_view->selectCell(m_view->selection().activeRow, origin);
    if (way != Way::Tab && way != Way::ShiftTab) m_tabOriginCol = m_tabRow = -1;
    else m_tabRow = m_view->selection().activeRow;
    m_finishing = false;
    updateBars();
}

void CrunchWindow::cancelEdit()
{
    if (!m_edit.active) return;
    returnToEditSheet();   // (Esc puts everything back, the sheet that was shown too)
    if (m_helper) m_helper->close();
    m_cellEditor->hide();
    m_edit.active = false;
    m_editRich = m_editRichFirst = jp::office::sheet::RichText();
    updatePointViews();
    if (m_edit.view.valid && m_view->viewState().valid) m_view->setViewState(m_edit.view);   // (the sheet is scrolled as it was, and the cell with the cursor is where it was)
    if (!m_calculating && !m_loading) setStatus(tr("Ready"));
    m_view->setFocus(Qt::OtherFocusReason);
    updateBars();
}

void CrunchWindow::onCellsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells)
{
    if (m_objects) m_objects->cellsChanged(cells);
    noteRowsChanged(sheet, cells);
    if (sheet == m_sheet && m_view->sheet().showFormulas() && !cells.empty()) {
        // (formulas typed while the formulas show widen their columns if they need: only the cells told of are measured)
        std::vector<std::pair<quint32, quint32>> places;
        for (size_t i = 0; i < cells.size() && i < 2000; ++i) places.emplace_back(cells[i].row, cells[i].col);
        m_view->formulaCellsChanged(places);
    }
    m_errorListValid = false;
    if (sheet == m_sheet && m_view->valuesChanged(sheet, cells)) updateBars();
    if (m_undoing && !cells.empty()) {
        // Undo and Redo show the cell they changed
        if (sheet != m_sheet) setActiveSheet(sheet);
        m_view->selectCell(cells.front().row, cells.front().col);
        m_view->ensureVisible(cells.front().row, cells.front().col);
    }
}

void CrunchWindow::clearSelection()
{
    if (m_edit.active || !editingAllowed()) return;
    const grid::Selection &sel = m_view->selection();
    const qint64 before = m_loader.changesMade();
    const crunch::EditSession::Outcome o = m_session->clear(m_sheet, sel.range.row1, sel.range.col1, sel.range.row2, sel.range.col2);
    if (!o.ok) notice(tr("Clear"), o.problem);
    else if (m_loader.changesMade() != before) m_calculating = true;
}

// Undo and Redo: a step that would take a password's protection off (the undoing of putting it on, the redoing of taking it off) asks for the password first, so that undoing is no way
// round a password.
void CrunchWindow::undoRedo(bool redo)
{
    QUndoStack *u = undoStack();
    if (!u || m_edit.active || m_saving || m_passwordJob || m_argsDialog) return;
    if (m_applying || m_session->isSlicing()) {
        setStatus(tr("A large change is still being made. It can be changed again when that is done."));
        return;
    }
    if (redo ? !u->canRedo() : !m_session->canUndo()) return;
    jp::office::sheet::PasswordHash guard;
    QString what;
    if (m_session->stepDropsPassword(redo, &guard, &what)) {
        askPassword(what, guard, [this, redo] { undoRedoNow(redo); });
        return;
    }
    undoRedoNow(redo);
}

void CrunchWindow::undoRedoNow(bool redo)
{
    QUndoStack *u = undoStack();
    if (!u || m_edit.active || m_saving || m_applying || m_argsDialog || m_session->isSlicing()) return;
    if (redo ? !u->canRedo() : !m_session->canUndo()) return;
    const qint64 before = m_loader.changesMade();
    const QString what = redo ? u->redoText() : u->undoText();
    m_undoing = true;
    if (redo) u->redo();
    else u->undo();
    m_undoing = false;
    if (m_loader.changesMade() != before) m_calculating = true;
    setStatus(redo ? tr("Redone: %1").arg(what) : tr("Undone: %1").arg(what));
}

} // namespace jp
