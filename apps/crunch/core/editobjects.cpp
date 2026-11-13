// Taking an object over the cells away, putting one there, moving it, or changing it (a chart, picture, or shape): one step of the undo stack. See EditSession::changeObjects.

#include "core/editsession.h"

namespace jp::crunch {

using namespace jp::office::sheet;

EditSession::Outcome EditSession::changeObjects(std::vector<ObjectEdit> edits, const QString &text)
{
    Outcome out;
    QString why;
    if (edits.empty()) return out;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    // a protected sheet that does not let its objects be changed keeps them as they are
    for (const ObjectEdit &e : edits)
        if (!allows(e.sheet, Allow::Objects, &why)) {
            out.ok = false;
            out.problem = why;
            return out;
        }
    Step step;
    step.objects = std::move(edits);
    pushStep(std::move(step), text);
    return out;
}

bool EditSession::amendLastStep(std::vector<ObjectEdit> edits)
{
    if (edits.empty()) return false;
    QString why;
    if (!canEdit(&why)) return false;
    for (const ObjectEdit &e : edits)
        if (!allows(e.sheet, Allow::Objects, &why)) return false;
    return amendTopStep(std::move(edits));
}

QString EditSession::linkText(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book || sheet < 0 || sheet >= m_book->calc->sheetCount()) return {};
    const Cell *c = m_book->calc->sheet(sheet).cells.find(row, col);
    if (!c) return {};
    const jp::calc::Value v = valueAt(sheet, row, col);
    if (v.kind() == jp::calc::Value::Kind::Error) return {};
    if (c->hasFormula() && v.kind() == jp::calc::Value::Kind::Empty) return {};   // (not worked out yet)
    return neutralText(sheet, row, col).trimmed();
}

EditSession::Outcome EditSession::deleteObject(int sheet, int id, const QString &text, const ObjectState &before, const ObjectState &after)
{
    ObjectEdit e;
    e.sheet = sheet;
    e.id = id;
    e.before = before;
    e.after = after;
    return changeObjects({e}, text);
}

} // namespace jp::crunch
