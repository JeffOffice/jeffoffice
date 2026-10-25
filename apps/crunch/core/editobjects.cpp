// Taking an object over the cells away, putting one there, moving it, or changing it (a chart, picture, or shape): one step of the undo stack. See EditSession::changeObjects.

#include "core/editsession.h"

namespace jp::crunch {

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
    Step step;
    step.objects = std::move(edits);
    pushStep(std::move(step), text);
    return out;
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
