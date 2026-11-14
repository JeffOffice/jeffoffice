// Form controls in the window: Insert > Form Controls puts one on the sheet, a click on one works it (it writes the cell it is linked to, or changes its own state, as one step of Undo), and
// Format Control changes the cell, the range, and the limits of one. See core/sheetcontrols.h.
#include "app/crunchwindow.h"

#include "core/sheetcontrols.h"
#include "core/sheetobjects.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSpinBox>
#include <QUndoStack>

#include <algorithm>

namespace jp {

using namespace jp::office::sheet;

namespace {

struct ControlKindInfo {
    const char *id;
    ControlKind kind;
    const char *text;
    const char *icon;
    double width, height;   // the size a new one has, in the grid's units
};

const ControlKindInfo kControlKinds[] = {
    {"insert.control.checkBox", ControlKind::CheckBox, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Check Box"), "square-check", 108, 20},
    {"insert.control.optionButton", ControlKind::Radio, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Option Button"), "circle-dot", 124, 20},
    {"insert.control.dropDown", ControlKind::Drop, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Drop-Down"), "square-chevron-down", 120, 22},
    {"insert.control.button", ControlKind::Button, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Button"), "square-mouse-pointer", 80, 24},
    {"insert.control.label", ControlKind::Label, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Label"), "type", 80, 20},
    {"insert.control.groupBox", ControlKind::GroupBox, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Group Box"), "box", 160, 100},
    {"insert.control.spinButton", ControlKind::Spin, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Spin Button"), "arrow-up-down", 20, 40},
    {"insert.control.scrollBar", ControlKind::Scroll, QT_TRANSLATE_NOOP("jp::CrunchWindow", "Scroll Bar"), "sliders-horizontal", 16, 80},
    {"insert.control.listBox", ControlKind::List, QT_TRANSLATE_NOOP("jp::CrunchWindow", "List Box"), "list", 120, 80},
};

}   // namespace

void CrunchWindow::createControlActions()
{
    for (const ControlKindInfo &k : kControlKinds) {
        const ControlKind kind = k.kind;
        mk(QString::fromLatin1(k.id), QCoreApplication::translate("jp::CrunchWindow", k.text), QString::fromLatin1(k.icon), QKeySequence(), [this, kind] { insertFormControl(kind); });
        m_opsActions << QString::fromLatin1(k.id);
    }
}

// The window carries out what a click on a control does, and shows what the others say.
void CrunchWindow::connectControls()
{
    if (!m_objects || !m_session) return;
    crunch::SheetControls *c = m_objects->controls();
    c->setValueSource([this](int sheet, quint32 row, quint32 col) { return m_session ? m_session->valueAt(sheet, row, col) : jp::calc::Value(); });
    disconnect(c, nullptr, this, nullptr);
    connect(c, &crunch::SheetControls::operated, this, [this](const crunch::ControlOperation &op) { carryOutControl(op); });
    connect(c, &crunch::SheetControls::macroSkipped, this, [this] { setStatus(tr("This button runs a macro. Macros don't run in JeffCrunch, so it does nothing here.")); });
    connect(c, &crunch::SheetControls::listRequested, this, [this](int sheet, int id, const QStringList &items, int selected) {
        if (sheet != m_sheet || !m_objects) return;
        auto *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->setObjectName(QStringLiteral("controlListMenu"));
        menu->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
        for (int i = 0; i < items.size(); ++i) {
            QAction *a = menu->addAction(items.at(i));
            a->setCheckable(true);
            a->setChecked(i + 1 == selected);
            connect(a, &QAction::triggered, this, [this, sheet, id, i] {
                crunch::ControlOperation op;
                if (m_objects->controls()->choose(sheet, id, i + 1, &op)) carryOutControl(op);
            });
        }
        if (items.isEmpty()) menu->addAction(tr("(nothing to choose from)"))->setEnabled(false);
        const QRect box = m_objects->layer()->deviceRect(id, grid::Pane::Body);
        menu->popup(m_view->viewport()->mapToGlobal(QPoint(box.left(), box.bottom() + 1)));
    });
}

// A click's result: a cell typed (one step), or the controls' own states changed (one step).
void CrunchWindow::carryOutControl(const crunch::ControlOperation &op)
{
    if (!m_session || !m_objects || m_edit.active || m_saving || m_chartSheetShown) return;
    if (op.setsCell) {
        if (!editingAllowed()) return;
        const qint64 before = m_loader.changesMade();
        const crunch::EditSession::Outcome o = m_session->type(op.cellSheet, op.row, op.col, op.text, false);
        if (!o.ok) {
            notice(tr("Form Control"), o.problem);
            return;
        }
        if (m_loader.changesMade() != before) m_calculating = true;
        m_objects->controls()->refresh(op.sheet);
        return;
    }
    if (op.changes.empty() || !editingAllowed()) return;
    std::vector<crunch::ObjectEdit> edits;
    for (const crunch::ControlChange &ch : op.changes) {
        crunch::ObjectEdit e;
        e.sheet = op.sheet;
        e.id = ch.id;
        e.before = m_objects->stateOf(op.sheet, ch.id);
        e.after = e.before;
        e.before.control = std::make_shared<const FormControl>(ch.before);
        e.after.control = std::make_shared<const FormControl>(ch.after);
        edits.push_back(e);
    }
    const crunch::EditSession::Outcome o = m_session->changeObjects(edits, tr("Form Control"));
    if (!o.ok) notice(tr("Form Control"), o.problem);
}

// Insert > Form Controls: the control is put at the cell with the cursor, as one step of Undo.
bool CrunchWindow::insertFormControl(ControlKind kind)
{
    if (!m_session || !m_objects || !m_loaded || m_chartSheetShown) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    const ControlKindInfo *info = nullptr;
    for (const ControlKindInfo &k : kControlKinds)
        if (k.kind == kind) info = &k;
    if (!info) return false;
    const grid::GridLayout &L = m_view->sheet().layout();
    const grid::Selection &sel = m_view->selection();
    const qint64 row = std::max<qint64>(0, sel.activeRow), col = std::max<qint64>(0, sel.activeCol);
    grid::ObjectBox box;
    box.x = double(L.cols.position(col)) + 2;
    box.y = double(L.rows.position(row)) + 2;
    box.w = info->width;
    box.h = info->height;
    DrawingAnchor anchor;
    m_objects->layer()->attach(&L);
    if (!m_objects->layer()->anchorAtBox(box, AnchorKind::TwoCell, &anchor)) anchor = newObjectAnchor(box.w, box.h);
    const QRectF placePt(box.x * 0.75, box.y * 0.75, box.w * 0.75, box.h * 0.75);
    QString why;
    const int id = m_objects->controls()->make(m_sheet, kind, anchor, placePt, &why);
    if (id < 0) {
        notice(tr("Form Control"), why);
        return false;
    }
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("Insert Form Control"));
    if (!o.ok) {
        notice(tr("Form Control"), o.problem);
        return false;
    }
    m_view->setFocus(Qt::OtherFocusReason);
    setStatus(tr("Form control inserted"));
    return true;
}

// Format Control: the properties the dialog asks for are changed as one step of Undo (nothing is done, and false said, when none differs or one is wrong).
bool CrunchWindow::setControlProperties(int id, const QString &link, const QString &range, int min, int max, int inc, int page, bool checked)
{
    if (!m_session || !m_objects || !crunch::isControlId(id)) return false;
    bool found = false;
    const FormControl was = m_objects->controls()->controlOf(m_sheet, id, &found);
    if (!found) return false;
    const QString title = tr("Format Control");
    if (!link.isEmpty() && !m_objects->controls()->validReference(m_sheet, link, false)) {
        notice(title, tr("The cell link \"%1\" is not a cell of this workbook.").arg(link));
        return false;
    }
    if (!range.isEmpty() && !m_objects->controls()->validReference(m_sheet, range, true)) {
        notice(title, tr("The input range \"%1\" is not a range of cells of this workbook.").arg(range));
        return false;
    }
    if (min > max || inc < 1 || page < 1) {
        notice(title, tr("The minimum can't be more than the maximum, and the changes must be at least 1."));
        return false;
    }
    FormControl after = was;
    after.link = link.trimmed();
    after.range = range.trimmed();
    if (was.kind == ControlKind::Spin || was.kind == ControlKind::Scroll) {
        after.min = min;
        after.max = max;
        after.inc = inc;
        after.page = page;
        after.val = std::clamp(after.val, min, max);
    }
    if (was.kind == ControlKind::CheckBox || was.kind == ControlKind::Radio) after.checked = checked ? 1 : (was.checked == 2 ? 2 : 0);
    if (sameControlState(was, after)) return false;
    crunch::ControlOperation op;
    if (!m_objects->controls()->formatted(m_sheet, id, after, &op)) return false;
    carryOutControl(op);
    return true;
}

void CrunchWindow::formatControlDialog(int id)
{
    if (!m_objects || !crunch::isControlId(id)) return;
    bool found = false;
    const FormControl c = m_objects->controls()->controlOf(m_sheet, id, &found);
    if (!found) return;
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Format Control"));
    auto *form = new QFormLayout(&dialog);
    const bool hasLink = c.kind != ControlKind::Button && c.kind != ControlKind::Label && c.kind != ControlKind::GroupBox;
    const bool hasRange = c.kind == ControlKind::Drop || c.kind == ControlKind::List;
    const bool hasLimits = c.kind == ControlKind::Spin || c.kind == ControlKind::Scroll;
    const bool hasState = c.kind == ControlKind::CheckBox || c.kind == ControlKind::Radio;
    QLineEdit *link = new QLineEdit(c.link, &dialog), *range = new QLineEdit(c.range, &dialog);
    auto *min = new QSpinBox(&dialog), *max = new QSpinBox(&dialog), *inc = new QSpinBox(&dialog), *page = new QSpinBox(&dialog);
    for (QSpinBox *s : {min, max, inc, page}) s->setRange(0, 30000);
    min->setValue(c.min);
    max->setValue(c.max);
    inc->setValue(c.inc);
    page->setValue(c.page);
    auto *checked = new QCheckBox(tr("Checked"), &dialog);
    checked->setChecked(c.checked == 1);
    if (hasState) form->addRow(checked);
    if (hasLimits) {
        form->addRow(tr("Minimum value:"), min);
        form->addRow(tr("Maximum value:"), max);
        form->addRow(tr("Incremental change:"), inc);
        if (c.kind == ControlKind::Scroll) form->addRow(tr("Page change:"), page);
    }
    if (hasRange) form->addRow(tr("Input range:"), range);
    if (hasLink) form->addRow(tr("Cell link:"), link);
    if (!hasLink && !hasRange && !hasLimits && !hasState) form->addRow(new QLabel(tr("This control has nothing to format here."), &dialog));
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    setControlProperties(id, link->text(), range->text(), min->value(), max->value(), inc->value(), page->value(), checked->isChecked());
}

} // namespace jp
