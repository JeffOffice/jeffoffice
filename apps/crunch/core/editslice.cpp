// What the plans of Sort, Replace, and Filter share: a step put in a slice at a time, the notes of a sheet kept as the cells now hold them, the number formats made once, and colors compared
// as the colors they come to.
#include "core/editsession.h"

#include "sheet/notes.h"
#include "sheet/refshift.h"

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
    const size_t total = step.cells.size();
    // the cells, a slice at a time (each slice is a step of its own as far as the model is concerned: the worker is held for it, and told what changed)
    if (push.next < total) {
        const size_t to = budget > 0 ? std::min<size_t>(total, push.next + size_t(budget)) : total;
        Step part;
        part.cells.assign(std::make_move_iterator(step.cells.begin() + std::ptrdiff_t(push.next)), std::make_move_iterator(step.cells.begin() + std::ptrdiff_t(to)));
        const bool ok = apply(&part, true);
        m_longestSlice = std::max<qint64>(m_longestSlice, qint64(part.cells.size()));
        ++m_slicesPushed;
        std::move(part.cells.begin(), part.cells.end(), step.cells.begin() + std::ptrdiff_t(push.next));
        if (!ok) {
            m_slicing = false;
            giveUpUndo(push.text, true);
            return true;
        }
        push.next = to;
        if (push.next < total) return false;
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
            if (!apply(&rest, true)) {
                m_slicing = false;
                giveUpUndo(push.text, true);
                return true;
            }
        }
    }
    m_slicing = false;
    pushStep(std::move(step), push.text, true);
    return true;
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
    NotesOfSheet &c = m_notes[sheet];
    if (c.notes && c.edits == m_edits.size() && c.permutes == m_permutes.size() && c.saves == m_saveGen) {
        if (cannot) *cannot = c.cannot;
        if (why) *why = c.why;
        return c.notes;
    }
    if (m_book->book->isConverted()) {
        c.cannot = true;
        c.why = tr("A workbook opened from an older kind of file, or from a text file, has its notes in a form JeffCrunch does not search. Save it as an .xlsx workbook, and look again.");
        c.notes = none;
    } else {
        if (!c.raw || c.saves != m_saveGen) {
            c.raw = std::make_shared<const std::map<quint64, QString>>(readSheetNotes(*m_book->book, sheet));
            c.saves = m_saveGen;
        }
        c.cannot = false;
        c.why.clear();
        if (m_edits.empty() && m_permutes.empty()) {
            c.notes = c.raw;
        } else if (m_edits.size() + m_permutes.size() > 64) {
            // (each note would be followed through every edit: past this many, the person is asked to save, which makes the file the base again)
            c.cannot = true;
            c.why = tr("There have been too many changes to rows, columns, and cells since the workbook was opened or saved to follow the notes through them. Save the workbook, and look again.");
            c.notes = none;
        } else {
            // each note goes where the cells went: through the edits in the order they were made, the cells put in another order among them
            std::map<quint64, QString> moved;
            for (const auto &note : *c.raw) {
                quint32 row = quint32(note.first >> 32), col = quint32(note.first & 0xFFFFFFFFu);
                bool alive = true;
                size_t next = 0;
                auto permutesBefore = [&](size_t edit) {
                    for (; next < m_permutes.size() && m_permutes[next].after <= edit; ++next) {
                        const CellPermute &p = m_permutes[next];
                        if (p.sheet != sheet) continue;
                        quint32 &line = p.columns ? col : row;
                        const quint32 other = p.columns ? row : col;
                        if (other < p.otherFirst || other > p.otherLast || line < p.first || line - p.first >= p.to.size()) continue;
                        line = p.to[line - p.first];
                    }
                };
                size_t i = 0;
                for (const StructEdit &e : m_edits) {
                    permutesBefore(i++);
                    int s = sheet;
                    if (adjustCell(e, &s, &row, &col) == RefFate::Gone) {
                        alive = false;
                        break;
                    }
                }
                if (!alive) continue;
                permutesBefore(i);
                moved[(quint64(row) << 32) | col] = note.second;
            }
            c.notes = std::make_shared<const std::map<quint64, QString>>(std::move(moved));
        }
    }
    c.edits = m_edits.size();
    c.permutes = m_permutes.size();
    if (cannot) *cannot = c.cannot;
    if (why) *why = c.why;
    return c.notes;
}

} // namespace jp::crunch
