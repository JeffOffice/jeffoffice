// Pointing at cells while a formula is typed: a click on a cell writes its address where a reference may go, a drag writes the block, a click on a header the whole column or row, a click
// on another sheet's tab goes on pointing there, the arrow keys move a pointer from the formula's cell, and F4 changes which parts of the reference are fixed. The cells the formula names
// are outlined in the grid, each in the color its reference has in the text.

#include "app/crunchwindow.h"
#include "core/formularefs.h"
#include "core/formulaspot.h"
#include "grid/gridview.h"
#include "sheet/refshift.h"

#include <QColor>
#include <QCoreApplication>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTextCharFormat>
#include <QTimer>

namespace jp {

namespace {

QChar separatorOf(const crunch::EditSession *session) { return crunch::argumentSeparator(session ? session->locale() : QLocale()); }

} // namespace

// A formula is being typed (the entry begins with "=": a cell formatted as text reads a formula as one too).
bool CrunchWindow::formulaTyped(const QLineEdit *line) const
{
    return m_edit.active && m_session && line && line->text().startsWith(QLatin1Char('='));
}

QLineEdit *CrunchWindow::editLine() const
{
    if (m_point.line && (m_point.line != m_cellEditor || m_cellEditor->isVisible())) return m_point.line;
    return m_cellEditor->isVisible() ? static_cast<QLineEdit *>(m_cellEditor) : m_formulaBar;
}

// The reference written by pointing is the last thing done to the line: the text is as it left it and the cursor is after the reference. Anything typed or moved ends that, and the next
// click writes a new reference.
bool CrunchWindow::pointValid(const QLineEdit *line) const
{
    return m_point.active && line && line->text() == m_point.text && line->cursorPosition() == m_point.caret && !line->hasSelectedText();
}

void CrunchWindow::writePoint(QLineEdit *line, const QString &text, bool replace, bool append)
{
    int at = line->cursorPosition();
    if (replace) {
        line->setSelection(m_point.start, m_point.length);
        at = m_point.start;
        line->insert(text);
    } else {
        const QString put = append ? QString(separatorOf(m_session.get())) + text : text;
        line->deselect();
        line->insert(put);
        if (append) ++at;
    }
    m_point.active = true;
    m_point.start = at;
    m_point.length = int(text.size());
    m_point.text = line->text();
    m_point.caret = line->cursorPosition();
    Q_EMIT line->textEdited(line->text());   // (the other line shows it too)
    m_point.text = line->text();
    m_point.caret = line->cursorPosition();
    line->setFocus(Qt::OtherFocusReason);
    updatePointViews();
}

bool CrunchWindow::pointAtCells(const grid::GridView::PointSpan &span, bool first, Qt::KeyboardModifiers modifiers)
{
    if (!m_edit.active || !m_session || !m_loaded) return false;
    QLineEdit *line = editLine();
    if (!formulaTyped(line)) return false;
    const bool valid = pointValid(line);
    if (first && !valid) {
        const crunch::FormulaSpot spot = crunch::spotAt(line->text(), line->cursorPosition(), separatorOf(m_session.get()));
        if (!spot.canPoint || spot.inText) return false;
    }
    if (!first && !valid) return true;   // (a drag goes on with the reference its press wrote)
    using Kind = crunch::FormulaRef::Kind;
    const Kind kind = span.kind == grid::GridView::PointSpan::Kind::Rows ? Kind::Rows : span.kind == grid::GridView::PointSpan::Kind::Columns ? Kind::Columns : Kind::Range;
    QString ref = crunch::referenceText(kind, quint32(span.range.row1), quint32(span.range.col1), quint32(span.range.row2), quint32(span.range.col2));
    if (m_sheet != m_edit.sheet && m_sheet >= 0 && m_sheet < int(m_loaded->book->info().sheets.size())) ref = jp::office::sheet::sheetNamePrefix(m_loaded->book->info().sheets[size_t(m_sheet)].name) + ref;
    const bool ctrl = first && (modifiers & Qt::ControlModifier);
    writePoint(line, ref, valid && !ctrl, valid && ctrl);
    m_point.sheet = m_sheet;
    m_point.anchorRow = span.range.row1;
    m_point.anchorCol = span.range.col1;
    m_point.headRow = span.range.row2;
    m_point.headCol = span.range.col2;
    return true;
}

// F2 changes between pointing and editing the text; with the cursor where a reference may go, an arrow moves the pointer (Shift extends it to a block), and F4 changes the fixed parts.
bool CrunchWindow::pointKey(QLineEdit *line, QKeyEvent *key)
{
    if (!m_edit.active || !m_session) return false;
    if (key->key() == Qt::Key_F2 && key->modifiers() == Qt::NoModifier) {
        m_cellEditor->setMode(m_cellEditor->mode() == CellEditor::Mode::Typing ? CellEditor::Mode::Editing : CellEditor::Mode::Typing);
        updatePointStatus();
        return true;
    }
    if (!formulaTyped(line)) return false;
    if (key->key() == Qt::Key_F4 && !(key->modifiers() & ~Qt::KeypadModifier)) {
        cycleFixedParts(line);
        return true;
    }
    const int k = key->key();
    if (k != Qt::Key_Up && k != Qt::Key_Down && k != Qt::Key_Left && k != Qt::Key_Right) return false;
    if (m_cellEditor->mode() != CellEditor::Mode::Typing || (key->modifiers() & ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::KeypadModifier))) return false;
    const bool toEdge = key->modifiers() & Qt::ControlModifier;
    const bool valid = pointValid(line);
    if (!valid) {
        const crunch::FormulaSpot spot = crunch::spotAt(line->text(), line->cursorPosition(), separatorOf(m_session.get()));
        if (!spot.canPoint || spot.inText) return false;
    }
    const bool shift = key->modifiers() & Qt::ShiftModifier;
    qint64 anchorRow, anchorCol, headRow, headCol;
    if (valid && m_point.sheet == m_sheet) {
        anchorRow = m_point.anchorRow;
        anchorCol = m_point.anchorCol;
        headRow = m_point.headRow;
        headCol = m_point.headCol;
    } else if (m_sheet == m_edit.sheet) {
        anchorRow = headRow = m_edit.row;
        anchorCol = headCol = m_edit.col;
    } else {
        anchorRow = headRow = std::max<qint64>(0, m_view->selection().activeRow);
        anchorCol = headCol = std::max<qint64>(0, m_view->selection().activeCol);
    }
    const int dRow = k == Qt::Key_Down ? 1 : k == Qt::Key_Up ? -1 : 0, dCol = k == Qt::Key_Right ? 1 : k == Qt::Key_Left ? -1 : 0;
    if (toEdge) m_view->edgeFrom(headRow, headCol, dRow, dCol, &headRow, &headCol);
    else m_view->stepFrom(headRow, headCol, dRow, dCol, &headRow, &headCol);
    if (!shift) {
        anchorRow = headRow;
        anchorCol = headCol;
    }
    // (a merged range the pointer is on, or reaches into, is written whole)
    const grid::GridRange block = m_view->expandMerges(grid::GridRange{std::min(anchorRow, headRow), std::min(anchorCol, headCol), std::max(anchorRow, headRow), std::max(anchorCol, headCol)});
    QString ref = crunch::referenceText(crunch::FormulaRef::Kind::Range, quint32(block.row1), quint32(block.col1), quint32(block.row2), quint32(block.col2));
    if (m_sheet != m_edit.sheet) ref = jp::office::sheet::sheetNamePrefix(m_loaded->book->info().sheets[size_t(m_sheet)].name) + ref;
    writePoint(line, ref, valid, false);
    m_point.sheet = m_sheet;
    m_point.anchorRow = anchorRow;
    m_point.anchorCol = anchorCol;
    m_point.headRow = headRow;
    m_point.headCol = headCol;
    m_view->ensureVisible(headRow, headCol);
    return true;
}

// F4: the reference the cursor is in, or just after, goes through $A$1, A$1, $A1, and A1.
void CrunchWindow::cycleFixedParts(QLineEdit *line)
{
    const QString text = line->text();
    const int caret = line->cursorPosition();
    const auto refs = crunch::findReferences(text);
    const crunch::FormulaRef *at = nullptr;
    for (const auto &r : refs)
        if (caret >= r.start && caret <= r.end() && (!at || r.end() == caret)) at = &r;
    if (!at) return;
    const bool wasPointed = pointValid(line) && m_point.start == at->start;
    const QString body = text.mid(at->bodyStart, at->end() - at->bodyStart);
    const QString changed = crunch::nextFixedForm(body);
    const int start = at->bodyStart, length = int(body.size());
    line->setSelection(start, length);
    line->insert(changed);
    line->setCursorPosition(start + int(changed.size()));
    Q_EMIT line->textEdited(line->text());
    if (wasPointed) {
        m_point.length += int(changed.size()) - length;
        m_point.text = line->text();
        m_point.caret = line->cursorPosition();
    }
    updatePointViews();
}

// A highlight's border or corner dragged: the reference it is of is written again for the block, with its sheet's name and its dollar signs as they were.
void CrunchWindow::adjustReference(int id, const grid::GridRange &block, bool final)
{
    QLineEdit *line = editLine();
    if (final) {
        if (line) line->setFocus(Qt::OtherFocusReason);
        return;
    }
    if (!m_edit.active || !line) return;
    const auto refs = crunch::findReferences(line->text());
    if (id < 0 || id >= int(refs.size())) return;
    const crunch::FormulaRef &r = refs[size_t(id)];
    using Kind = crunch::FormulaRef::Kind;
    quint32 row1 = quint32(block.row1), col1 = quint32(block.col1), row2 = quint32(block.row2), col2 = quint32(block.col2);
    if (r.kind == Kind::Columns) row1 = row2 = 0;   // (whole columns and rows move and stretch along their own side only)
    if (r.kind == Kind::Rows) col1 = col2 = 0;
    const QString body = line->text().mid(r.bodyStart, r.end() - r.bodyStart);
    const QString text = crunch::refitBody(body, crunch::referenceText(r.kind == Kind::Cell ? Kind::Range : r.kind, row1, col1, row2, col2));
    if (text == body) return;
    const int caret = line->cursorPosition();
    line->setSelection(r.bodyStart, int(body.size()));
    line->insert(text);
    line->setCursorPosition(std::max(0, caret + int(text.size() - body.size()) * (caret >= r.end() ? 1 : 0)));
    m_point.active = false;
    Q_EMIT line->textEdited(line->text());
}

bool CrunchWindow::keepEditOnSheetChange(int target) const
{
    if (!m_edit.active || !m_session) return false;
    if (target == m_edit.sheet) return true;
    const QLineEdit *line = editLine();
    if (!formulaTyped(line)) return false;
    if (pointValid(line)) return true;
    const crunch::FormulaSpot spot = crunch::spotAt(line->text(), line->cursorPosition(), separatorOf(m_session.get()));
    return spot.canPoint && !spot.inText;
}

void CrunchWindow::returnToEditSheet()
{
    if (m_edit.active && m_sheet != m_edit.sheet) setActiveSheet(m_edit.sheet);
}

// The views are made once for the changes of a turn of the event loop (a drag changes the text at each move).
void CrunchWindow::schedulePointViews()
{
    if (m_pointViewsPending) return;
    m_pointViewsPending = true;
    QTimer::singleShot(0, this, [this] {
        if (!m_pointViewsPending) return;
        updatePointViews();
    });
}

void CrunchWindow::updatePointStatus()
{
    if (!m_edit.active || m_calculating || m_loading) return;   // (what the program is busy with stays in the status bar)
    QString mode = tr("Enter");
    if (m_cellEditor->mode() == CellEditor::Mode::Editing) {
        mode = tr("Edit");
    } else if (const QLineEdit *line = editLine(); formulaTyped(line)) {
        const crunch::FormulaSpot spot = crunch::spotAt(line->text(), line->cursorPosition(), separatorOf(m_session.get()));
        if (pointValid(line) || (spot.canPoint && !spot.inText)) mode = tr("Point");
    }
    setStatus(mode);
}

// The references of the formula being typed: each outlined on the sheet that has its cells, in a color of its own, and its text in the same color in both lines.
void CrunchWindow::updatePointViews()
{
    m_pointViewsPending = false;
    std::vector<grid::PointMark> marks;
    std::vector<crunch::FormulaRef> refs;
    const QLineEdit *line = editLine();
    const bool formula = m_loaded && formulaTyped(line);
    if (formula) {
        refs = crunch::findReferences(line->text());
        const QVector<quint32> colors = crunch::referenceColors();
        const auto &sheets = m_loaded->book->info().sheets;
        for (size_t i = 0; i < refs.size(); ++i) {
            const crunch::FormulaRef &r = refs[i];
            int sheet = m_edit.sheet;
            if (!r.sheet.isEmpty()) {
                sheet = -1;
                for (size_t s = 0; s < sheets.size(); ++s)
                    if (sheets[s].name.compare(r.sheet, Qt::CaseInsensitive) == 0) sheet = int(s);
            }
            if (sheet != m_sheet) continue;
            marks.push_back({grid::GridRange{qint64(r.row1), qint64(r.col1), qint64(r.row2), qint64(r.col2)}, QColor::fromRgba(colors[int(i % size_t(colors.size()))]), int(i)});
        }
    }
    m_view->setPointMarks(std::move(marks));
    // the text in the same colors (formats of the line's input, which a line draws as it does the marks of a language's input)
    const QVector<quint32> colors = crunch::referenceColors();
    for (QLineEdit *l : {static_cast<QLineEdit *>(m_cellEditor), m_formulaBar}) {
        if (l->text().isEmpty() && refs.empty()) continue;
        if (m_coloring || m_composing.contains(l)) continue;   // (a text being composed with an input method is left alone: the formats would end it)
        QList<QInputMethodEvent::Attribute> attrs;
        const int caret = l->cursorPosition();
        if (formula && l->text() == line->text()) {
            for (size_t i = 0; i < refs.size(); ++i) {
                QTextCharFormat f;
                f.setForeground(QColor::fromRgba(colors[int(i % size_t(colors.size()))]));
                attrs.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, refs[i].start - caret, refs[i].length, f));
            }
        }
        if (attrs.isEmpty() && !m_coloredLines.contains(l)) continue;
        m_coloring = true;
        QInputMethodEvent ev(QString(), attrs);
        QCoreApplication::sendEvent(l, &ev);
        m_coloring = false;
        if (attrs.isEmpty()) m_coloredLines.remove(l);
        else m_coloredLines.insert(l);
    }
    updatePointStatus();
}

} // namespace jp
