// What the plans of Sort, Replace, and Filter share: a step put in a slice at a time, the notes of a sheet kept as the cells now hold them, the number formats made once, and colors compared
// as the colors they come to.
#include "core/editsession.h"

#include "sheet/notes.h"
#include "sheet/refshift.h"

#include <QElapsedTimer>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

std::shared_ptr<EditSession::SlicedPush> EditSession::beginSlicedPush(Step step, const QString &text)
{
    auto push = std::make_shared<SlicedPush>();
    push->step = std::move(step);
    push->text = text;
    m_slicing = true;
    return push;
}

bool EditSession::continueSlicedPush(SlicedPush &push, qint64 budget)
{
    if (!m_slicing) return true;
    Step &step = push.step;
    auto fail = [&] {
        m_slicing = false;
        giveUpUndo(push.text, true);
        return true;
    };
    // the tables the step changes come first (the cells of a table that grows are put in a table that has grown)
    if (!push.tablesApplied) {
        push.tablesApplied = true;
        if (!step.tables.empty()) {
            Step first;
            first.tables = step.tables;
            const bool ok = apply(&first, true);
            step.tables = std::move(first.tables);   // (what applying made of them, the names that were rewritten, is kept for undoing)
            if (!ok) return fail();
        }
    }
    const size_t total = step.cells.size();
    qint64 left = budget;
    // the cells, a slice at a time (each slice is a step of its own as far as the model is concerned: the worker is held for it, and told what changed)
    if (push.next < total) {
        const size_t to = budget > 0 ? std::min<size_t>(total, push.next + size_t(budget)) : total;
        Step part;
        part.cells.assign(std::make_move_iterator(step.cells.begin() + std::ptrdiff_t(push.next)), std::make_move_iterator(step.cells.begin() + std::ptrdiff_t(to)));
        const bool ok = apply(&part, true);
        m_longestSlice = std::max<qint64>(m_longestSlice, qint64(part.cells.size()));
        ++m_slicesPushed;
        std::move(part.cells.begin(), part.cells.end(), step.cells.begin() + std::ptrdiff_t(push.next));
        if (!ok) return fail();
        left -= qint64(to - push.next);
        push.next = to;
        if (push.next < total) return false;
        if (budget > 0 && left <= 0 && !step.fills.empty()) return false;
    }
    // the cells that formulas are filled into, rows at a time
    while (push.fillAt < step.fills.size()) {
        const FillEdit &f = step.fills[push.fillAt];
        if (push.fillRow < f.row1) push.fillRow = f.row1;
        const qint64 rowsLeft = qint64(f.row2) - push.fillRow + 1;
        const qint64 take = budget > 0 ? std::min<qint64>(rowsLeft, std::max<qint64>(left, 1)) : rowsLeft;
        Step part;
        FillEdit sub = f;
        sub.row1 = push.fillRow;
        sub.row2 = quint32(qint64(push.fillRow) + take - 1);
        part.fills.push_back(std::move(sub));
        const bool ok = apply(&part, true);
        m_longestSlice = std::max<qint64>(m_longestSlice, take);
        ++m_slicesPushed;
        if (!ok) return fail();
        left -= take;
        if (take < rowsLeft) {
            push.fillRow = quint32(qint64(push.fillRow) + take);
            return false;
        }
        ++push.fillAt;
        push.fillRow = 0;
        if (budget > 0 && left <= 0 && push.fillAt < step.fills.size()) return false;
    }
    // what goes with the cells (the merged ranges, the filter, and the like) is put in with the last
    if (!push.began) {
        push.began = true;
        Step rest;
        rest.sizes = step.sizes;
        rest.merges = step.merges;
        rest.formats = step.formats;
        rest.filters = step.filters;
        rest.rowSpecs = step.rowSpecs;
        rest.permutes = step.permutes;
        if (!rest.empty()) {
            if (!apply(&rest, true)) return fail();
        }
    }
    m_slicing = false;
    pushStep(std::move(step), push.text, true);
    return true;
}

// A step that fills many cells: put in at once if it is small, else a slice at a time by a timer of the session's own, as one step of Undo.
void EditSession::startFill(Step step, const QString &text)
{
    qint64 cells = qint64(step.cells.size());
    for (const FillEdit &f : step.fills) cells += qint64(f.row2) - f.row1 + 1;
    if (cells <= kFillAtOnce) {
        pushStep(std::move(step), text);
        return;
    }
    m_fill = beginSlicedPush(std::move(step), text);
    m_fillBudget = 2000;
    if (!m_fillTimer.isActive()) {
        m_fillTimer.setInterval(0);
        m_fillTimer.setSingleShot(false);
        disconnect(&m_fillTimer, nullptr, this, nullptr);
        connect(&m_fillTimer, &QTimer::timeout, this, [this] { stepFill(); });
    }
    m_fillTimer.start();
}

void EditSession::stepFill()
{
    if (!m_fill) {
        m_fillTimer.stop();
        return;
    }
    QElapsedTimer t;
    t.start();
    const bool done = continueSlicedPush(*m_fill, m_fillBudget);
    // (the next slice is made to take about six milliseconds of the window's thread)
    const qint64 us = std::max<qint64>(1, t.nsecsElapsed() / 1000);
    m_fillBudget = std::clamp<qint64>(m_fillBudget * 6000 / us, 200, 20000);
    if (done) {
        m_fill.reset();
        m_fillTimer.stop();
        Q_EMIT fillFinished();
    }
}

void EditSession::finishFills()
{
    while (m_fill) {
        const bool done = continueSlicedPush(*m_fill, 0);
        if (done) {
            m_fill.reset();
            m_fillTimer.stop();
            Q_EMIT fillFinished();
        }
    }
}

const jp::calc::NumberFormat &EditSession::formatOfStyle(quint32 style) const
{
    auto found = m_formats.find(style);
    if (found != m_formats.end()) return *found->second;
    // (the code is the file's own, with no language in it; the marks a number is shown with are the locale's, and are put in when it is formatted)
    auto format = std::make_shared<jp::calc::NumberFormat>(m_book->book->styles().formatCode(style, QLocale::c()));
    ++m_formatsMade;
    return *m_formats.emplace(style, std::move(format)).first->second;
}

qint64 EditSession::colorKey(const ColorSpec &color) const
{
    if (!color.isSet() || color.kind == ColorSpec::Kind::Auto) return -1;
    return qint64(m_book->book->styles().resolve(color, 0x000000) & 0xFFFFFF);
}

std::shared_ptr<const std::map<quint64, QString>> EditSession::notesOf(int sheet, bool *cannot, QString *why) const
{
    if (cannot) *cannot = false;
    auto none = std::make_shared<const std::map<quint64, QString>>();
    if (!m_book || !m_book->book) return none;
    if (m_book->book->isConverted()) {
        if (cannot) *cannot = true;
        if (why) *why = tr("A workbook opened from an older kind of file, or from a text file, has its notes in a form JeffCrunch does not search. Save it as an .xlsx workbook, and look again.");
        return none;
    }
    // (the notes as they are now, kept by the session; the texts are copied when a search asks, and again after a change: see forgetNotes)
    NotesOfSheet &c = m_notes[sheet];
    if (c.notes) return c.notes;
    std::map<quint64, QString> texts;
    for (const auto &n : m_anchors.notes(sheet)) texts[n.first] = n.second.text;
    c.notes = std::make_shared<const std::map<quint64, QString>>(std::move(texts));
    return c.notes;
}

} // namespace jp::crunch
