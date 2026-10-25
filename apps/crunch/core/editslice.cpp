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
