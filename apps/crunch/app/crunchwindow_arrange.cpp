// Picking several objects, and Group, Ungroup, and Align for them (the Shape Format tab's Arrange group).
#include "app/crunchwindow.h"

#include "app/objectpicker.h"
#include "core/editsession.h"
#include "core/objectgroup.h"
#include "core/objectstate.h"
#include "core/sheetobjects.h"

#include <QAction>
#include <QSignalBlocker>
#include <QUndoStack>

namespace jp {

using jp::office::sheet::DrawingAnchor;
using jp::office::sheet::ObjectKind;

void CrunchWindow::createArrangeActions()
{
    auto mkObject = [this](const char *id, const QString &text, const char *icon, const std::function<void()> &fn) {
        mk(QString::fromLatin1(id), text, icon, QKeySequence(), fn, false, false);
    };
    mkObject("object.group", tr("Group"), "group", [this] { groupObjects(); });
    mkObject("object.ungroup", tr("Ungroup"), "ungroup", [this] { ungroupObjects(); });
    mkObject("object.alignLeft", tr("Align Left"), "align-horizontal-justify-start", [this] { alignObjects(crunch::ObjectAlign::Left); });
    mkObject("object.alignCenter", tr("Align Center"), "align-horizontal-justify-center", [this] { alignObjects(crunch::ObjectAlign::Center); });
    mkObject("object.alignRight", tr("Align Right"), "align-horizontal-justify-end", [this] { alignObjects(crunch::ObjectAlign::Right); });
    mkObject("object.alignTop", tr("Align Top"), "align-vertical-justify-start", [this] { alignObjects(crunch::ObjectAlign::Top); });
    mkObject("object.alignMiddle", tr("Align Middle"), "align-vertical-justify-center", [this] { alignObjects(crunch::ObjectAlign::Middle); });
    mkObject("object.alignBottom", tr("Align Bottom"), "align-vertical-justify-end", [this] { alignObjects(crunch::ObjectAlign::Bottom); });
    mkObject("object.distributeH", tr("Distribute Horizontally"), "align-horizontal-distribute-center", [this] { alignObjects(crunch::ObjectAlign::DistributeHorizontally); });
    mkObject("object.distributeV", tr("Distribute Vertically"), "align-vertical-distribute-center", [this] { alignObjects(crunch::ObjectAlign::DistributeVertically); });
    mk("object.selectObjects", tr("Select Objects"), "square-dashed-mouse-pointer", QKeySequence(), [this] { setSelectObjectsMode(!(m_picker && m_picker->boxMode())); }, true, false);
    m_opsActions << QStringLiteral("object.selectObjects");
    updateArrangeActions();
}

std::vector<int> CrunchWindow::pickedObjects() const
{
    std::vector<int> out;
    if (!m_picker || !m_objects || m_objects->shownSheet() != m_sheet) return out;
    for (const int id : m_picker->picked())
        if (m_objects->exists(m_sheet, id) && !m_objects->layer()->isDeleted(id)) out.push_back(id);
    return out;
}

void CrunchWindow::pickObjects(const std::vector<int> &ids)
{
    if (m_picker) m_picker->setPicked(ids);
}

void CrunchWindow::setSelectObjectsMode(bool on)
{
    if (m_picker) m_picker->setBoxMode(on);
    if (QAction *a = act(QStringLiteral("object.selectObjects"))) {
        QSignalBlocker block(a);
        a->setChecked(on);
    }
}

void CrunchWindow::updateArrangeActions()
{
    const std::vector<int> ids = pickedObjects();
    const bool usable = m_session && !m_saving && !m_chartSheetShown;
    bool groupable = ids.size() >= 2 && usable;
    for (const int id : ids) groupable = groupable && m_objects->kindOf(m_sheet, id) != ObjectKind::Chart;
    act(QStringLiteral("object.group"))->setEnabled(groupable);
    act(QStringLiteral("object.ungroup"))->setEnabled(usable && ids.size() == 1 && m_objects->kindOf(m_sheet, ids.front()) == ObjectKind::Group);
    for (const char *name : {"object.alignLeft", "object.alignCenter", "object.alignRight", "object.alignTop", "object.alignMiddle", "object.alignBottom"}) act(QString::fromLatin1(name))->setEnabled(usable && ids.size() >= 2);
    for (const char *name : {"object.distributeH", "object.distributeV"}) act(QString::fromLatin1(name))->setEnabled(usable && ids.size() >= 3);
    act(QStringLiteral("object.selectObjects"))->setEnabled(usable);
}

namespace {

QString alignText(crunch::ObjectAlign how)
{
    using A = crunch::ObjectAlign;
    switch (how) {
    case A::Left: return CrunchWindow::tr("Align Left");
    case A::Center: return CrunchWindow::tr("Align Center");
    case A::Right: return CrunchWindow::tr("Align Right");
    case A::Top: return CrunchWindow::tr("Align Top");
    case A::Middle: return CrunchWindow::tr("Align Middle");
    case A::Bottom: return CrunchWindow::tr("Align Bottom");
    case A::DistributeHorizontally: return CrunchWindow::tr("Distribute Horizontally");
    case A::DistributeVertically: return CrunchWindow::tr("Distribute Vertically");
    }
    return QString();
}

} // namespace

bool CrunchWindow::alignObjects(crunch::ObjectAlign how)
{
    const bool spread = how == crunch::ObjectAlign::DistributeHorizontally || how == crunch::ObjectAlign::DistributeVertically;
    const std::vector<int> ids = pickedObjects();
    if (ids.size() < (spread ? 3u : 2u) || !m_session || !m_objects || m_edit.active || m_saving || m_chartSheetShown) return false;
    std::vector<grid::ObjectBox> boxes;
    for (const int id : ids) boxes.push_back(m_objects->layer()->boxNow(id));
    const std::vector<grid::ObjectBox> now = crunch::alignedBoxes(boxes, how);
    m_objects->layer()->attach(&m_view->sheet().layout());
    std::vector<crunch::ObjectEdit> edits;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (std::abs(now[i].x - boxes[i].x) < 0.5 && std::abs(now[i].y - boxes[i].y) < 0.5) continue;
        DrawingAnchor anchor;
        if (!m_objects->layer()->anchorAtBox(now[i], m_objects->anchorOf(m_sheet, ids[i]).kind, &anchor)) continue;
        crunch::ObjectEdit e;
        e.sheet = m_sheet;
        e.id = ids[i];
        e.before = m_objects->stateOf(m_sheet, ids[i]);
        e.after = m_objects->stateMoved(m_sheet, ids[i], anchor);
        edits.push_back(e);
    }
    if (edits.empty()) return false;   // (they are lined up already)
    const crunch::EditSession::Outcome o = m_session->changeObjects(edits, alignText(how));
    if (!o.ok) {
        setStatus(o.problem);
        return false;
    }
    return true;
}

bool CrunchWindow::groupObjects()
{
    const QString title = tr("Group");
    const std::vector<int> ids = pickedObjects();
    if (ids.size() < 2 || !m_session || !m_objects || m_edit.active || m_saving || m_chartSheetShown) return false;
    std::vector<crunch::GroupMember> members;
    for (const int id : ids) {
        QString why;
        if (m_objects->kindOf(m_sheet, id) == ObjectKind::Chart) {
            notice(title, tr("A chart can't be put in a group."));
            return false;
        }
        if (!m_objects->canDelete(m_sheet, id, &why)) {
            notice(title, why);
            return false;
        }
        crunch::GroupMember m;
        if (!m_objects->copyElement(m_sheet, id, &m.clip, &why)) {
            notice(title, why);
            return false;
        }
        m.box = m_objects->layer()->boxNow(id);
        members.push_back(m);
    }
    crunch::ObjectClip clip;
    grid::ObjectBox box;
    QString why;
    if (!crunch::buildGroupClip(members, &clip, &box, &why)) {
        notice(title, why);
        return false;
    }
    m_objects->layer()->attach(&m_view->sheet().layout());
    DrawingAnchor anchor;
    if (!m_objects->layer()->anchorAtBox(box, jp::office::sheet::AnchorKind::TwoCell, &anchor)) return false;
    const int made = m_objects->makeElement(m_sheet, clip, anchor, &why);
    if (made < 0) {
        notice(title, why);
        return false;
    }
    std::vector<crunch::ObjectEdit> edits;
    crunch::ObjectEdit add;
    add.sheet = m_sheet;
    add.id = made;
    add.before = m_objects->stateOf(m_sheet, made);
    add.after = add.before;
    add.after.deleted = false;
    edits.push_back(add);
    for (const int id : ids) {
        crunch::ObjectEdit e;
        e.sheet = m_sheet;
        e.id = id;
        const crunch::ObjectState before = m_objects->stateOf(m_sheet, id);
        e.after = before;
        e.after.deleted = true;
        e.after.revive = m_objects->captureRevive(m_sheet, id);
        e.before = before;
        e.before.revive = e.after.revive;
        edits.push_back(e);
    }
    const crunch::EditSession::Outcome o = m_session->changeObjects(edits, title);
    if (!o.ok) {
        m_objects->discardMade(m_sheet, made);
        notice(title, o.problem);
        return false;
    }
    pickObjects({made});
    return true;
}

bool CrunchWindow::ungroupObjects()
{
    const QString title = tr("Ungroup");
    const std::vector<int> ids = pickedObjects();
    if (ids.size() != 1 || !m_session || !m_objects || m_edit.active || m_saving || m_chartSheetShown) return false;
    const int id = ids.front();
    QString why;
    if (m_objects->kindOf(m_sheet, id) != ObjectKind::Group) return false;
    if (!m_objects->canDelete(m_sheet, id, &why)) {
        notice(title, why);
        return false;
    }
    crunch::ObjectClip clip;
    if (!m_objects->copyElement(m_sheet, id, &clip, &why)) {
        notice(title, why);
        return false;
    }
    std::vector<crunch::GroupMember> parts;
    if (!crunch::splitGroupClip(clip, m_objects->layer()->boxNow(id), &parts, &why)) {
        notice(title, why);
        return false;
    }
    m_objects->layer()->attach(&m_view->sheet().layout());
    std::vector<int> made;
    auto drop = [&] {
        for (const int m : made) m_objects->discardMade(m_sheet, m);
    };
    std::vector<crunch::ObjectEdit> edits;
    for (const crunch::GroupMember &p : parts) {
        DrawingAnchor anchor;
        if (!m_objects->layer()->anchorAtBox(p.box, jp::office::sheet::AnchorKind::TwoCell, &anchor)) {
            drop();
            return false;
        }
        const int now = m_objects->makeElement(m_sheet, p.clip, anchor, &why);
        if (now < 0) {
            drop();
            notice(title, why);
            return false;
        }
        made.push_back(now);
        crunch::ObjectEdit e;
        e.sheet = m_sheet;
        e.id = now;
        e.before = m_objects->stateOf(m_sheet, now);
        e.after = e.before;
        e.after.deleted = false;
        edits.push_back(e);
    }
    crunch::ObjectEdit gone;
    gone.sheet = m_sheet;
    gone.id = id;
    const crunch::ObjectState before = m_objects->stateOf(m_sheet, id);
    gone.after = before;
    gone.after.deleted = true;
    gone.after.revive = m_objects->captureRevive(m_sheet, id);
    gone.before = before;
    gone.before.revive = gone.after.revive;
    edits.push_back(gone);
    const crunch::EditSession::Outcome o = m_session->changeObjects(edits, title);
    if (!o.ok) {
        drop();
        notice(title, o.problem);
        return false;
    }
    pickObjects(made);
    return true;
}

} // namespace jp
