// Taking an object over the cells away (a chart, picture, or shape): one step of the undo stack. See EditSession::deleteObject.

#include "core/editsession.h"

namespace jp::crunch {

EditSession::Outcome EditSession::deleteObject(int sheet, int id, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    Step step;
    step.objects.push_back(ObjectEdit{sheet, id});
    pushStep(std::move(step), text);
    return out;
}

} // namespace jp::crunch
