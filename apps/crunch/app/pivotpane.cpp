#include "app/pivotpane.h"
#include <algorithm>

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

using namespace jp::office::sheet;

namespace jp {

namespace {

const char *const kFunctions[] = {"sum", "count", "countNums", "average", "max", "min", "product", "stdDev", "stdDevp", "var", "varp"};

QString functionLabel(const QString &fn)
{
    if (fn == QLatin1String("sum")) return QCoreApplication::translate("jp::PivotFieldsPane", "Sum");
    if (fn == QLatin1String("count")) return QCoreApplication::translate("jp::PivotFieldsPane", "Count");
    if (fn == QLatin1String("countNums")) return QCoreApplication::translate("jp::PivotFieldsPane", "Count Numbers");
    if (fn == QLatin1String("average")) return QCoreApplication::translate("jp::PivotFieldsPane", "Average");
    if (fn == QLatin1String("max")) return QCoreApplication::translate("jp::PivotFieldsPane", "Max");
    if (fn == QLatin1String("min")) return QCoreApplication::translate("jp::PivotFieldsPane", "Min");
    if (fn == QLatin1String("product")) return QCoreApplication::translate("jp::PivotFieldsPane", "Product");
    if (fn == QLatin1String("stdDev")) return QCoreApplication::translate("jp::PivotFieldsPane", "StdDev (a sample)");
    if (fn == QLatin1String("stdDevp")) return QCoreApplication::translate("jp::PivotFieldsPane", "StdDevp (all the values)");
    if (fn == QLatin1String("var")) return QCoreApplication::translate("jp::PivotFieldsPane", "Var (a sample)");
    return QCoreApplication::translate("jp::PivotFieldsPane", "Varp (all the values)");
}

} // namespace

PivotFieldsPane::PivotFieldsPane(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("pivot.fieldsPane"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    m_title = new QLabel(tr("Choose fields to add to the report"), this);
    m_title->setTextFormat(Qt::PlainText);   // (a table's name comes from a file)
    m_title->setWordWrap(true);
    layout->addWidget(m_title);
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("pivot.search"));
    m_search->setPlaceholderText(tr("Search"));
    m_search->setClearButtonEnabled(true);
    m_search->setAccessibleName(tr("Search fields"));
    layout->addWidget(m_search);
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &text) { search(text); });
    m_fields = new QListWidget(this);
    m_fields->setObjectName(QStringLiteral("pivot.fields"));
    m_fields->setAccessibleName(tr("Pivot table fields"));
    layout->addWidget(m_fields, 3);
    auto *grid = new QGridLayout();
    const char *titles[4] = {QT_TR_NOOP("Filters"), QT_TR_NOOP("Columns"), QT_TR_NOOP("Rows"), QT_TR_NOOP("Values")};
    for (int i = 0; i < 4; ++i) {
        auto *label = new QLabel(tr(titles[i]), this);
        m_boxes[i] = new QListWidget(this);
        m_boxes[i]->setObjectName(QStringLiteral("pivot.box%1").arg(i));
        m_boxes[i]->setAccessibleName(tr(titles[i]));
        m_boxes[i]->setSelectionMode(QAbstractItemView::SingleSelection);
        m_boxes[i]->setMaximumHeight(110);
        auto *cell = new QVBoxLayout();
        cell->addWidget(label);
        cell->addWidget(m_boxes[i]);
        grid->addLayout(cell, i / 2, i % 2);
    }
    layout->addLayout(grid, 2);
    auto *buttons = new QHBoxLayout();
    m_moveTo = new QToolButton(this);
    m_moveTo->setText(tr("Move To"));
    m_moveTo->setObjectName(QStringLiteral("pivot.moveTo"));
    m_moveTo->setPopupMode(QToolButton::InstantPopup);
    auto *menu = new QMenu(m_moveTo);
    for (int i = 0; i < 4; ++i) {
        const QString text = i == 0 ? tr("Move to Filters") : i == 1 ? tr("Move to Columns") : i == 2 ? tr("Move to Rows") : tr("Move to Values");
        menu->addAction(text, this, [this, i] { moveSelected(i); });
    }
    m_moveTo->setMenu(menu);
    m_up = new QToolButton(this);
    m_up->setText(tr("Move Up"));
    m_up->setObjectName(QStringLiteral("pivot.up"));
    m_down = new QToolButton(this);
    m_down->setText(tr("Move Down"));
    m_down->setObjectName(QStringLiteral("pivot.down"));
    m_remove = new QToolButton(this);
    m_remove->setText(tr("Remove Field"));
    m_remove->setObjectName(QStringLiteral("pivot.remove"));
    m_settings = new QToolButton(this);
    m_settings->setText(tr("Value Field Settings…"));
    m_settings->setObjectName(QStringLiteral("pivot.valueSettings"));
    m_fieldSettings = new QToolButton(this);
    m_fieldSettings->setText(tr("Field Settings…"));
    m_fieldSettings->setObjectName(QStringLiteral("pivot.fieldSettings"));
    for (QToolButton *b : {m_moveTo, m_up, m_down, m_remove, m_settings, m_fieldSettings}) {
        b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        buttons->addWidget(b);
    }
    buttons->addStretch(1);
    layout->addLayout(buttons);
    auto *defer = new QHBoxLayout();
    m_deferBox = new QCheckBox(tr("Defer Layout Update"), this);
    m_deferBox->setObjectName(QStringLiteral("pivot.defer"));
    m_update = new QToolButton(this);
    m_update->setText(tr("Update"));
    m_update->setObjectName(QStringLiteral("pivot.update"));
    m_update->setEnabled(false);
    defer->addWidget(m_deferBox);
    defer->addStretch(1);
    defer->addWidget(m_update);
    layout->addLayout(defer);
    connect(m_deferBox, &QCheckBox::toggled, this, [this](bool on) { setDeferred(on); });
    connect(m_update, &QToolButton::clicked, this, [this] { applyDeferred(); });
    connect(m_up, &QToolButton::clicked, this, [this] { shiftSelected(-1); });
    connect(m_down, &QToolButton::clicked, this, [this] { shiftSelected(1); });
    connect(m_remove, &QToolButton::clicked, this, [this] { removeSelected(); });
    connect(m_settings, &QToolButton::clicked, this, [this] {
        const int b = selectedBox();
        if (b != 3) return;
        const Entry e = entryOf(m_boxes[3]->currentItem());
        if (e.data >= 0) Q_EMIT valueSettingsRequested(e.data);
    });
    connect(m_fieldSettings, &QToolButton::clicked, this, [this] {
        const int f = selectedField();
        if (f >= 0) Q_EMIT fieldSettingsRequested(f);
    });
    for (int i = 0; i < 3; ++i)
        connect(m_boxes[i], &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
            const Entry e = entryOf(item);
            if (e.field >= 0) Q_EMIT fieldSettingsRequested(e.field);
        });
    connect(m_boxes[3], &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        const Entry e = entryOf(item);
        if (e.data >= 0) Q_EMIT valueSettingsRequested(e.data);
    });
    connect(m_fields, &QListWidget::itemChanged, this, [this](QListWidgetItem *item) {
        if (m_filling) return;
        checkField(m_fields->row(item), item->checkState() == Qt::Checked);
    });
    for (int i = 0; i < 4; ++i)
        connect(m_boxes[i], &QListWidget::itemSelectionChanged, this, [this, i] {
            if (m_filling) return;
            if (m_boxes[i]->selectedItems().isEmpty()) {
                Q_EMIT fieldSelectionChanged();
                return;
            }
            for (int k = 0; k < 4; ++k)
                if (k != i) {
                    const QSignalBlocker block(m_boxes[k]);
                    m_boxes[k]->clearSelection();
                    m_boxes[k]->setCurrentRow(-1);
                }
            Q_EMIT fieldSelectionChanged();
        });
    showNothing();
}

void PivotFieldsPane::showNothing()
{
    m_filling = true;
    m_fields->clear();
    for (QListWidget *b : m_boxes) b->clear();
    m_table = PivotTable();
    m_pending = false;
    if (m_update) m_update->setEnabled(false);
    m_names.clear();
    m_numeric.clear();
    m_title->setText(tr("Click a pivot table to see its fields."));
    m_filling = false;
}

void PivotFieldsPane::showTable(const QString &name, const PivotTable &table, const QStringList &fieldNames, const QVector<bool> &numeric, const void *identity)
{
    // (changes waiting for Update are not thrown away by the same table being shown again; a different table, or the same one changed by something else, drops them)
    const quintptr stamp = quintptr(identity);
    if (m_pending) {
        if (name == m_pendingName && stamp == m_pendingStamp) return;
        m_pending = false;
        if (m_update) m_update->setEnabled(false);
    }
    m_shownName = name;
    m_shownStamp = stamp;
    m_table = table;
    m_names = fieldNames;
    m_numeric = numeric;
    m_title->setText(tr("Fields of %1. Check a field to add it; the boxes show where each is.").arg(name));
    fillBoxes();
}

void PivotFieldsPane::fillBoxes()
{
    m_filling = true;
    // the selection to put back
    const int boxWas = selectedBox();
    const int rowWas = boxWas >= 0 ? m_boxes[boxWas]->currentRow() : -1;
    m_fields->clear();
    QVector<bool> used(m_names.size(), false);
    for (int f : m_table.rowFields) if (f >= 0 && f < used.size()) used[f] = true;
    for (int f : m_table.colFields) if (f >= 0 && f < used.size()) used[f] = true;
    for (const PivotPageField &p : m_table.pageFields) if (p.fld >= 0 && p.fld < used.size()) used[p.fld] = true;
    for (const PivotDataField &d : m_table.dataFields) if (d.fld >= 0 && d.fld < used.size()) used[d.fld] = true;
    for (int i = 0; i < m_names.size(); ++i) {
        auto *item = new QListWidgetItem(m_names[i], m_fields);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(used[i] ? Qt::Checked : Qt::Unchecked);
    }
    for (QListWidget *b : m_boxes) b->clear();
    auto add = [&](int box, const QString &text, int field, int data) {
        auto *item = new QListWidgetItem(text, m_boxes[box]);
        item->setData(Qt::UserRole, field);
        item->setData(Qt::UserRole + 1, data);
    };
    for (const PivotPageField &p : m_table.pageFields) add(0, p.fld >= 0 && p.fld < m_names.size() ? m_names[p.fld] : QString(), p.fld, -1);
    for (int f : m_table.colFields) add(1, f == -2 ? tr("Values") : (f >= 0 && f < m_names.size() ? m_names[f] : QString()), f, -1);
    for (int f : m_table.rowFields) add(2, f == -2 ? tr("Values") : (f >= 0 && f < m_names.size() ? m_names[f] : QString()), f, -1);
    for (int i = 0; i < m_table.dataFields.size(); ++i) add(3, m_table.dataFields[i].name, m_table.dataFields[i].fld, i);
    if (boxWas >= 0) m_boxes[boxWas]->setCurrentRow(std::min(rowWas, m_boxes[boxWas]->count() - 1));
    m_filling = false;
    if (m_search && !m_search->text().isEmpty()) search(m_search->text());
}

int PivotFieldsPane::selectedField() const
{
    const int b = selectedBox();
    if (b < 0 || b > 2) return -1;
    const Entry e = entryOf(m_boxes[b]->currentItem());
    return e.field >= 0 ? e.field : -1;
}

int PivotFieldsPane::selectedBox() const
{
    for (int i = 0; i < 4; ++i)
        if (m_boxes[i]->currentRow() >= 0 && !m_boxes[i]->selectedItems().isEmpty()) return i;
    return -1;
}

PivotFieldsPane::Entry PivotFieldsPane::entryOf(QListWidgetItem *item) const
{
    Entry e;
    if (!item) return e;
    for (int i = 0; i < 4; ++i)
        if (m_boxes[i] == item->listWidget()) e.box = i;
    e.index = item->listWidget()->row(item);
    e.field = item->data(Qt::UserRole).toInt();
    e.data = item->data(Qt::UserRole + 1).toInt();
    return e;
}

bool PivotFieldsPane::select(int box, int row)
{
    if (box < 0 || box > 3 || row < 0 || row >= m_boxes[box]->count()) return false;
    m_boxes[box]->setCurrentRow(row);
    m_boxes[box]->item(row)->setSelected(true);
    return true;
}

void PivotFieldsPane::changed(const PivotTable &table, const QString &text)
{
    if (m_deferBox && m_deferBox->isChecked()) {
        // (Defer Layout Update: the change is kept, and shown in the boxes, until Update)
        m_table = table;
        m_pending = true;
        m_pendingName = m_shownName;
        m_pendingStamp = m_shownStamp;
        m_pendingText = text;
        m_update->setEnabled(true);
        fillBoxes();
        return;
    }
    Q_EMIT changeRequested(table, text);
}

void PivotFieldsPane::search(const QString &text)
{
    for (int i = 0; i < m_fields->count(); ++i) m_fields->item(i)->setHidden(!text.trimmed().isEmpty() && !m_fields->item(i)->text().contains(text.trimmed(), Qt::CaseInsensitive));
}

void PivotFieldsPane::setDeferred(bool on)
{
    if (m_deferBox->isChecked() != on) {
        const QSignalBlocker block(m_deferBox);
        m_deferBox->setChecked(on);
    }
    if (!on) applyDeferred();
}

void PivotFieldsPane::applyDeferred()
{
    if (!m_pending) return;
    m_pending = false;
    m_update->setEnabled(false);
    Q_EMIT changeRequested(m_table, m_pendingText.isEmpty() ? tr("Move Field in Pivot Table") : m_pendingText);
}

// A value for a field: its sum, named so that no field and no other value has the name.
PivotDataField PivotFieldsPane::newValue(const PivotTable &t, int field) const
{
    PivotDataField d;
    d.fld = field;
    d.name = tr("Sum of %1").arg(m_names.value(field));
    for (;;) {
        bool clash = false;
        for (const PivotDataField &o : t.dataFields) clash = clash || o.name.compare(d.name, Qt::CaseInsensitive) == 0;
        for (const QString &n : m_names) clash = clash || n.compare(d.name, Qt::CaseInsensitive) == 0;
        if (!clash) break;
        d.name += QLatin1Char(' ');
    }
    return d;
}

void PivotFieldsPane::checkField(int field, bool on)
{
    if (field < 0 || field >= m_names.size()) return;
    PivotTable t = m_table;
    if (on) {
        // (a field of numbers is totaled, any other is a row)
        if (m_numeric.value(field)) {
            t.dataFields.append(newValue(t, field));
        } else {
            t.rowFields.append(field);
        }
        changed(t, tr("Add Field to Pivot Table"));
    } else {
        t.rowFields.removeAll(field);
        t.colFields.removeAll(field);
        for (int i = t.pageFields.size() - 1; i >= 0; --i)
            if (t.pageFields[i].fld == field) t.pageFields.remove(i);
        for (int i = t.dataFields.size() - 1; i >= 0; --i)
            if (t.dataFields[i].fld == field) t.dataFields.remove(i);
        changed(t, tr("Remove Field from Pivot Table"));
    }
}

void PivotFieldsPane::removeSelected()
{
    const int b = selectedBox();
    if (b < 0) return;
    const Entry e = entryOf(m_boxes[b]->currentItem());
    PivotTable t = m_table;
    if (b == 0 && e.index < t.pageFields.size()) t.pageFields.remove(e.index);
    else if (b == 1 && e.index < t.colFields.size()) t.colFields.remove(e.index);
    else if (b == 2 && e.index < t.rowFields.size()) t.rowFields.remove(e.index);
    else if (b == 3 && e.data >= 0 && e.data < t.dataFields.size()) t.dataFields.remove(e.data);
    else return;
    changed(t, tr("Remove Field from Pivot Table"));
}

void PivotFieldsPane::moveSelected(int target)
{
    const int b = selectedBox();
    if (b < 0 || target < 0 || target > 3) return;
    const Entry e = entryOf(m_boxes[b]->currentItem());
    if (b == target) return;
    PivotTable t = m_table;
    // the values themselves (-2) go between Columns and Rows only
    if (e.field == -2) {
        if (target != 1 && target != 2) return;
        t.rowFields.removeAll(-2);
        t.colFields.removeAll(-2);
        (target == 1 ? t.colFields : t.rowFields).append(-2);
        t.dataOnRows = target == 2;
        changed(t, tr("Move Field in Pivot Table"));
        return;
    }
    const int field = e.field;
    if (b == 0 && e.index < t.pageFields.size()) t.pageFields.remove(e.index);
    else if (b == 1 && e.index < t.colFields.size()) t.colFields.remove(e.index);
    else if (b == 2 && e.index < t.rowFields.size()) t.rowFields.remove(e.index);
    else if (b == 3 && e.data >= 0 && e.data < t.dataFields.size()) t.dataFields.remove(e.data);
    else return;
    if (target == 0) {
        PivotPageField p;
        p.fld = field;
        t.pageFields.append(p);
    } else if (target == 1) {
        t.colFields.append(field);
    } else if (target == 2) {
        t.rowFields.append(field);
    } else {
        t.dataFields.append(newValue(t, field));
    }
    changed(t, tr("Move Field in Pivot Table"));
}

void PivotFieldsPane::shiftSelected(int by)
{
    const int b = selectedBox();
    if (b < 0) return;
    const Entry e = entryOf(m_boxes[b]->currentItem());
    PivotTable t = m_table;
    auto swapIn = [&](auto &list, int at) {
        const int to = at + by;
        if (at < 0 || at >= list.size() || to < 0 || to >= list.size()) return false;
        std::swap(list[at], list[to]);
        return true;
    };
    bool done = false;
    if (b == 0) done = swapIn(t.pageFields, e.index);
    else if (b == 1) done = swapIn(t.colFields, e.index);
    else if (b == 2) done = swapIn(t.rowFields, e.index);
    else done = swapIn(t.dataFields, e.index);
    if (done) changed(t, tr("Move Field in Pivot Table"));
}

// ---------------------------------------------------------------- Group Field

GroupDialog::GroupDialog(const GroupChoice &c, QWidget *parent) : QDialog(parent), m_choice(c)
{
    setWindowTitle(tr("Grouping"));
    setObjectName(QStringLiteral("groupDialog"));
    auto *form = new QFormLayout(this);
    m_start = new QLineEdit(c.startText, this);
    m_start->setObjectName(QStringLiteral("group.start"));
    m_start->setAccessibleName(tr("Starting at"));
    form->addRow(tr("Starting at:"), m_start);
    m_end = new QLineEdit(c.endText, this);
    m_end->setObjectName(QStringLiteral("group.end"));
    m_end->setAccessibleName(tr("Ending at"));
    form->addRow(tr("Ending at:"), m_end);
    if (c.isDate) {
        m_levels = new QListWidget(this);
        m_levels->setObjectName(QStringLiteral("group.levels"));
        m_levels->setAccessibleName(tr("By"));
        const std::pair<const char *, const char *> parts[] = {{"seconds", QT_TR_NOOP("Seconds")}, {"minutes", QT_TR_NOOP("Minutes")}, {"hours", QT_TR_NOOP("Hours")}, {"days", QT_TR_NOOP("Days")},
                                                              {"months", QT_TR_NOOP("Months")}, {"quarters", QT_TR_NOOP("Quarters")}, {"years", QT_TR_NOOP("Years")}};
        for (const auto &p : parts) {
            auto *item = new QListWidgetItem(tr(p.second), m_levels);
            item->setData(Qt::UserRole, QString::fromLatin1(p.first));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(c.levels.contains(QLatin1String(p.first)) ? Qt::Checked : Qt::Unchecked);
        }
        form->addRow(tr("By:"), m_levels);
    } else {
        m_step = new QLineEdit(c.stepText, this);
        m_step->setObjectName(QStringLiteral("group.step"));
        m_step->setAccessibleName(tr("By"));
        form->addRow(tr("By:"), m_step);
    }
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

GroupChoice GroupDialog::choice() const
{
    GroupChoice c = m_choice;
    c.levels.clear();
    if (m_levels)
        for (int i = 0; i < m_levels->count(); ++i)
            if (m_levels->item(i)->checkState() == Qt::Checked) c.levels << m_levels->item(i)->data(Qt::UserRole).toString();
    c.autoStart = m_start->text().trimmed() == m_choice.startText;
    c.autoEnd = m_end->text().trimmed() == m_choice.endText;
    c.startText = m_start->text().trimmed();
    c.endText = m_end->text().trimmed();
    if (m_step) c.stepText = m_step->text().trimmed();
    return c;
}

// ---------------------------------------------------------------- Calculated Field

CalculatedFieldDialog::CalculatedFieldDialog(const CalculatedFieldChoice &c, QWidget *parent) : QDialog(parent), m_choice(c)
{
    setWindowTitle(tr("Insert Calculated Field"));
    setObjectName(QStringLiteral("calculatedFieldDialog"));
    auto *form = new QFormLayout(this);
    m_name = new QComboBox(this);
    m_name->setEditable(true);
    m_name->setObjectName(QStringLiteral("calc.name"));
    m_name->setAccessibleName(tr("Name"));
    m_name->addItems(c.existingNames);
    m_name->setCurrentText(c.name);
    form->addRow(tr("Name:"), m_name);
    m_formula = new QLineEdit(c.formula, this);
    m_formula->setObjectName(QStringLiteral("calc.formula"));
    m_formula->setAccessibleName(tr("Formula"));
    m_formula->setMinimumWidth(320);
    form->addRow(tr("Formula:"), m_formula);
    m_fields = new QListWidget(this);
    m_fields->setObjectName(QStringLiteral("calc.fields"));
    m_fields->setAccessibleName(tr("Fields"));
    m_fields->addItems(c.fieldNames);
    form->addRow(tr("Fields:"), m_fields);
    auto *hint = new QLabel(tr("Double-click a field to put its name in the formula. A field's name stands for its total in each cell."), this);
    hint->setWordWrap(true);
    form->addRow(hint);
    auto *box = new QDialogButtonBox(this);
    m_add = box->addButton(tr("Add"), QDialogButtonBox::AcceptRole);
    m_add->setObjectName(QStringLiteral("calc.add"));
    m_delete = box->addButton(tr("Delete"), QDialogButtonBox::DestructiveRole);
    m_delete->setObjectName(QStringLiteral("calc.delete"));
    box->addButton(QDialogButtonBox::Close);
    form->addRow(box);
    connect(m_add, &QAbstractButton::clicked, this, [this] {
        m_choice.action = CalculatedFieldChoice::Add;
        accept();
    });
    connect(m_delete, &QAbstractButton::clicked, this, [this] {
        m_choice.action = CalculatedFieldChoice::Delete;
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_fields, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        const QString name = item->text();
        const bool plain = !name.isEmpty() && std::all_of(name.begin(), name.end(), [](QChar ch) { return ch.isLetterOrNumber() || ch == QLatin1Char('_'); }) && !name.at(0).isDigit();
        m_formula->insert(plain ? name : QLatin1Char('\'') + QString(name).replace(QLatin1Char('\''), QStringLiteral("''")) + QLatin1Char('\''));
        m_formula->setFocus();
    });
    connect(m_name, &QComboBox::currentTextChanged, this, [this] { nameChanged(); });
    nameChanged();
}

void CalculatedFieldDialog::nameChanged()
{
    const int at = int(m_choice.existingNames.indexOf(m_name->currentText().trimmed()));
    if (at >= 0 && at < m_choice.existingFormulas.size() && m_formula->text() != m_choice.existingFormulas[at]) m_formula->setText(QLatin1Char('=') + m_choice.existingFormulas[at]);
    m_add->setText(at >= 0 ? tr("Modify") : tr("Add"));
    m_delete->setEnabled(at >= 0);
}

CalculatedFieldChoice CalculatedFieldDialog::choice() const
{
    CalculatedFieldChoice c = m_choice;
    c.name = m_name->currentText().trimmed();
    c.formula = m_formula->text().trimmed();
    return c;
}

// ---------------------------------------------------------------- Calculated Item

CalculatedItemDialog::CalculatedItemDialog(const CalculatedItemChoice &c, QWidget *parent) : QDialog(parent), m_choice(c)
{
    setWindowTitle(tr("Insert Calculated Item"));
    setObjectName(QStringLiteral("calculatedItemDialog"));
    auto *form = new QFormLayout(this);
    m_field = new QComboBox(this);
    m_field->setObjectName(QStringLiteral("calcItem.field"));
    m_field->setAccessibleName(tr("Field"));
    for (const CalculatedItemChoice::Field &f : c.fields) m_field->addItem(f.name);
    const int at = m_field->findText(c.field);
    if (at >= 0) m_field->setCurrentIndex(at);
    form->addRow(tr("Field:"), m_field);
    m_name = new QComboBox(this);
    m_name->setEditable(true);
    m_name->setObjectName(QStringLiteral("calcItem.name"));
    m_name->setAccessibleName(tr("Name"));
    form->addRow(tr("Name:"), m_name);
    m_formula = new QLineEdit(c.formula, this);
    m_formula->setObjectName(QStringLiteral("calcItem.formula"));
    m_formula->setAccessibleName(tr("Formula"));
    m_formula->setMinimumWidth(320);
    form->addRow(tr("Formula:"), m_formula);
    m_items = new QListWidget(this);
    m_items->setObjectName(QStringLiteral("calcItem.items"));
    m_items->setAccessibleName(tr("Items"));
    form->addRow(tr("Items:"), m_items);
    auto *hint = new QLabel(tr("Double-click an item to put its name in the formula. An item's name stands for its value in each cell."), this);
    hint->setWordWrap(true);
    form->addRow(hint);
    auto *box = new QDialogButtonBox(this);
    m_add = box->addButton(tr("Add"), QDialogButtonBox::AcceptRole);
    m_add->setObjectName(QStringLiteral("calcItem.add"));
    m_delete = box->addButton(tr("Delete"), QDialogButtonBox::DestructiveRole);
    m_delete->setObjectName(QStringLiteral("calcItem.delete"));
    box->addButton(QDialogButtonBox::Close);
    form->addRow(box);
    connect(m_add, &QAbstractButton::clicked, this, [this] {
        m_choice.action = CalculatedItemChoice::Add;
        accept();
    });
    connect(m_delete, &QAbstractButton::clicked, this, [this] {
        m_choice.action = CalculatedItemChoice::Delete;
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_items, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        const QString name = item->text();
        const bool plain = !name.isEmpty() && std::all_of(name.begin(), name.end(), [](QChar ch) { return ch.isLetterOrNumber() || ch == QLatin1Char('_'); }) && !name.at(0).isDigit();
        m_formula->insert(plain ? name : QLatin1Char('\'') + QString(name).replace(QLatin1Char('\''), QStringLiteral("''")) + QLatin1Char('\''));
        m_formula->setFocus();
    });
    connect(m_field, &QComboBox::currentTextChanged, this, [this] { fieldChanged(); });
    connect(m_name, &QComboBox::currentTextChanged, this, [this] { nameChanged(); });
    fieldChanged();
    m_name->setCurrentText(c.name);
}

void CalculatedItemDialog::fieldChanged()
{
    const int at = m_field->currentIndex();
    m_items->clear();
    const QString typed = m_name->currentText();
    m_name->clear();
    if (at >= 0 && at < m_choice.fields.size()) {
        m_items->addItems(m_choice.fields[at].items);
        m_name->addItems(m_choice.fields[at].calcNames);
    }
    m_name->setCurrentText(typed);
    nameChanged();
}

void CalculatedItemDialog::nameChanged()
{
    const int f = m_field->currentIndex();
    const int at = f >= 0 && f < m_choice.fields.size() ? int(m_choice.fields[f].calcNames.indexOf(m_name->currentText().trimmed())) : -1;
    if (at >= 0 && at < m_choice.fields[f].calcFormulas.size() && m_formula->text() != m_choice.fields[f].calcFormulas[at]) m_formula->setText(QLatin1Char('=') + m_choice.fields[f].calcFormulas[at]);
    m_add->setText(at >= 0 ? tr("Modify") : tr("Add"));
    m_delete->setEnabled(at >= 0);
}

CalculatedItemChoice CalculatedItemDialog::choice() const
{
    CalculatedItemChoice c = m_choice;
    c.field = m_field->currentText();
    c.name = m_name->currentText().trimmed();
    c.formula = m_formula->text().trimmed();
    return c;
}

// ---------------------------------------------------------------- Solve Order

SolveOrderDialog::SolveOrderDialog(const SolveOrderChoice &c, QWidget *parent) : QDialog(parent), m_choice(c)
{
    setWindowTitle(tr("Solve Order"));
    setObjectName(QStringLiteral("solveOrderDialog"));
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("When a cell has two calculated items, the one of the higher number is worked out last."), this));
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("solveOrder.list"));
    m_list->setAccessibleName(tr("Calculated items"));
    layout->addWidget(m_list);
    auto *form = new QFormLayout;
    m_order = new QSpinBox(this);
    m_order->setObjectName(QStringLiteral("solveOrder.order"));
    m_order->setAccessibleName(tr("Solve order"));
    m_order->setRange(1, 100000);
    form->addRow(tr("Solve order:"), m_order);
    layout->addLayout(form);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(box);
    auto label = [this](int i) { return tr("%1: %2 (%3)").arg(m_choice.fields[i], m_choice.names[i]).arg(m_choice.orders[i]); };
    for (int i = 0; i < m_choice.names.size(); ++i) m_list->addItem(label(i));
    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
        m_order->setEnabled(row >= 0);
        if (row >= 0 && row < m_choice.orders.size()) m_order->setValue(m_choice.orders[row]);
    });
    connect(m_order, qOverload<int>(&QSpinBox::valueChanged), this, [this, label](int v) {
        const int row = m_list->currentRow();
        if (row < 0 || row >= m_choice.orders.size()) return;
        m_choice.orders[row] = v;
        m_list->item(row)->setText(label(row));
    });
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_order->setEnabled(false);
    if (m_list->count() > 0) m_list->setCurrentRow(0);
}

// ---------------------------------------------------------------- Pivot Table Options

PivotOptionsDialog::PivotOptionsDialog(const PivotOptionsChoice &c, QWidget *parent) : QDialog(parent), m_choice(c)
{
    setWindowTitle(tr("Pivot Table Options"));
    setObjectName(QStringLiteral("pivotOptionsDialog"));
    auto *layout = new QVBoxLayout(this);
    auto check = [&](const QString &text, const char *objectName, bool on) {
        auto *b = new QCheckBox(text, this);
        b->setObjectName(QLatin1String(objectName));
        b->setChecked(on);
        layout->addWidget(b);
        return b;
    };
    if (c.hasTable) {
        auto *form = new QFormLayout();
        m_name = new QLineEdit(c.name, this);
        m_name->setObjectName(QStringLiteral("pivotOptions.name"));
        m_name->setAccessibleName(tr("Pivot table name"));
        form->addRow(tr("Name:"), m_name);
        layout->addLayout(form);
        m_merge = check(tr("Merge and center cells with labels"), "pivotOptions.merge", c.mergeLabels);
        m_showError = check(tr("For error values show:"), "pivotOptions.showError", c.showError);
        m_error = new QLineEdit(c.errorText, this);
        m_error->setObjectName(QStringLiteral("pivotOptions.errorText"));
        m_error->setAccessibleName(tr("Text for error values"));
        layout->addWidget(m_error);
        m_showMissing = check(tr("For empty cells show:"), "pivotOptions.showMissing", c.showMissing);
        m_missing = new QLineEdit(c.missingText, this);
        m_missing->setObjectName(QStringLiteral("pivotOptions.missingText"));
        m_missing->setAccessibleName(tr("Text for empty cells"));
        layout->addWidget(m_missing);
        m_autofit = check(tr("Fit column widths when the table is updated"), "pivotOptions.autofit", c.autofit);
        m_preserve = check(tr("Keep the formatting of cells when the table is updated"), "pivotOptions.preserve", c.preserveFormatting);
        m_colGrand = check(tr("Show grand totals for rows"), "pivotOptions.rowGrand", c.colGrandTotals);
        m_rowGrand = check(tr("Show grand totals for columns"), "pivotOptions.colGrand", c.rowGrandTotals);
        m_headers = check(tr("Show field headers"), "pivotOptions.headers", c.showHeaders);
        m_refresh = check(tr("Refresh the data when the file is opened"), "pivotOptions.refresh", c.refreshOnOpen);
    }
    m_generate = check(tr("Write GETPIVOTDATA when I click a value of a pivot table while typing a formula"), "pivotOptions.generate", c.generateGetPivotData);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

PivotOptionsChoice PivotOptionsDialog::choice() const
{
    PivotOptionsChoice c = m_choice;
    c.generateGetPivotData = m_generate->isChecked();
    if (!c.hasTable) return c;
    c.name = m_name->text().trimmed();
    c.mergeLabels = m_merge->isChecked();
    c.showError = m_showError->isChecked();
    c.errorText = m_error->text();
    c.showMissing = m_showMissing->isChecked();
    c.missingText = m_missing->text();
    c.autofit = m_autofit->isChecked();
    c.preserveFormatting = m_preserve->isChecked();
    // (the text says "for rows" for the totals the file calls the column totals: a row of totals at the right is one for each row)
    c.colGrandTotals = m_colGrand->isChecked();
    c.rowGrandTotals = m_rowGrand->isChecked();
    c.showHeaders = m_headers->isChecked();
    c.refreshOnOpen = m_refresh->isChecked();
    return c;
}

// ---------------------------------------------------------------- Field Settings

FieldSettingsDialog::FieldSettingsDialog(const FieldSettings &settings, QWidget *parent) : QDialog(parent), m_settings(settings)
{
    setWindowTitle(tr("Field Settings"));
    setObjectName(QStringLiteral("fieldSettingsDialog"));
    auto *form = new QFormLayout(this);
    auto *source = new QLabel(settings.sourceName, this);
    source->setTextFormat(Qt::PlainText);
    form->addRow(tr("Source name:"), source);
    m_name = new QLineEdit(settings.name, this);
    m_name->setObjectName(QStringLiteral("field.name"));
    m_name->setAccessibleName(tr("Custom name"));
    form->addRow(tr("Custom name:"), m_name);
    m_subtotals = new QComboBox(this);
    m_subtotals->setObjectName(QStringLiteral("field.subtotals"));
    m_subtotals->setAccessibleName(tr("Subtotals"));
    m_subtotals->addItem(tr("Automatic"), true);
    m_subtotals->addItem(tr("None"), false);
    m_subtotals->setCurrentIndex(settings.subtotals ? 0 : 1);
    form->addRow(tr("Subtotals:"), m_subtotals);
    m_form = new QComboBox(this);
    m_form->setObjectName(QStringLiteral("field.form"));
    m_form->setAccessibleName(tr("Layout"));
    m_form->addItem(tr("Compact form"), 2);
    m_form->addItem(tr("Outline form"), 1);
    m_form->addItem(tr("Tabular form"), 0);
    m_form->setCurrentIndex(settings.form == 2 ? 0 : settings.form == 1 ? 1 : 2);
    form->addRow(tr("Show item labels in:"), m_form);
    m_blank = new QCheckBox(tr("Insert a blank line after each item"), this);
    m_blank->setObjectName(QStringLiteral("field.blank"));
    m_blank->setChecked(settings.blankRow);
    form->addRow(m_blank);
    m_repeat = new QCheckBox(tr("Repeat item labels (outline and tabular forms)"), this);
    m_repeat->setObjectName(QStringLiteral("field.repeat"));
    m_repeat->setChecked(settings.repeatLabels);
    form->addRow(m_repeat);
    // sort
    m_sort = new QComboBox(this);
    m_sort->setObjectName(QStringLiteral("field.sort"));
    m_sort->setAccessibleName(tr("Sort"));
    m_sort->addItem(tr("Manual (keep the order of the items)"), 0);
    m_sort->addItem(tr("A to Z by the labels"), 1);
    m_sort->addItem(tr("Z to A by the labels"), 2);
    m_sort->addItem(tr("Smallest to largest by a value"), 3);
    m_sort->addItem(tr("Largest to smallest by a value"), 4);
    m_sort->setCurrentIndex(settings.sortByData >= 0 ? (settings.sortByDescending ? 4 : 3) : settings.sort);
    form->addRow(tr("Sort:"), m_sort);
    m_sortData = new QComboBox(this);
    m_sortData->setObjectName(QStringLiteral("field.sortData"));
    m_sortData->setAccessibleName(tr("Sort by this value"));
    for (int i = 0; i < settings.dataNames.size(); ++i) m_sortData->addItem(settings.dataNames[i], i);
    if (settings.sortByData >= 0) m_sortData->setCurrentIndex(settings.sortByData);
    form->addRow(tr("by the value:"), m_sortData);
    // filter
    m_filter = new QComboBox(this);
    m_filter->setObjectName(QStringLiteral("field.filter"));
    m_filter->setAccessibleName(tr("Filter"));
    const std::pair<PivotFilter::Kind, const char *> kinds[] = {
        {PivotFilter::None, QT_TR_NOOP("No filter")}, {PivotFilter::LabelBeginsWith, QT_TR_NOOP("Label begins with")}, {PivotFilter::LabelEndsWith, QT_TR_NOOP("Label ends with")},
        {PivotFilter::LabelContains, QT_TR_NOOP("Label contains")}, {PivotFilter::LabelNotContains, QT_TR_NOOP("Label does not contain")}, {PivotFilter::LabelEquals, QT_TR_NOOP("Label equals")},
        {PivotFilter::LabelNotEquals, QT_TR_NOOP("Label does not equal")}, {PivotFilter::ValueGreater, QT_TR_NOOP("Value is greater than")},
        {PivotFilter::ValueGreaterOrEqual, QT_TR_NOOP("Value is greater than or equal to")}, {PivotFilter::ValueLess, QT_TR_NOOP("Value is less than")},
        {PivotFilter::ValueLessOrEqual, QT_TR_NOOP("Value is less than or equal to")}, {PivotFilter::ValueEquals, QT_TR_NOOP("Value equals")},
        {PivotFilter::ValueNotEquals, QT_TR_NOOP("Value does not equal")}, {PivotFilter::ValueBetween, QT_TR_NOOP("Value is between")},
        {PivotFilter::ValueNotBetween, QT_TR_NOOP("Value is not between")}, {PivotFilter::TopItems, QT_TR_NOOP("Top items (by a value)")}, {PivotFilter::BottomItems, QT_TR_NOOP("Bottom items (by a value)")}};
    for (const auto &k : kinds) m_filter->addItem(tr(k.second), int(k.first));
    form->addRow(tr("Filter:"), m_filter);
    m_filterData = new QComboBox(this);
    m_filterData->setObjectName(QStringLiteral("field.filterData"));
    m_filterData->setAccessibleName(tr("Filter by this value"));
    for (int i = 0; i < settings.dataNames.size(); ++i) m_filterData->addItem(settings.dataNames[i], i);
    form->addRow(tr("by the value:"), m_filterData);
    m_text = new QLineEdit(this);
    m_text->setObjectName(QStringLiteral("field.filterText"));
    m_text->setAccessibleName(tr("Filter text or number"));
    form->addRow(tr("Text or number:"), m_text);
    m_text2 = new QLineEdit(this);
    m_text2->setObjectName(QStringLiteral("field.filterText2"));
    m_text2->setAccessibleName(tr("Filter second number"));
    form->addRow(tr("and:"), m_text2);
    m_items = new QListWidget(this);
    m_items->setObjectName(QStringLiteral("field.items"));
    m_items->setAccessibleName(tr("Items to show"));
    for (const auto &it : settings.items) {
        auto *w = new QListWidgetItem(it.label, m_items);
        w->setFlags(w->flags() | Qt::ItemIsUserCheckable);
        w->setCheckState(it.shown ? Qt::Checked : Qt::Unchecked);
    }
    form->addRow(tr("Show these items:"), m_items);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

FieldSettings FieldSettingsDialog::settings() const
{
    FieldSettings s = m_settings;
    s.name = m_name->text().trimmed();
    s.subtotals = m_subtotals->currentData().toBool();
    s.form = m_form->currentData().toInt();
    s.blankRow = m_blank->isChecked();
    s.repeatLabels = m_repeat->isChecked();
    const int sort = m_sort->currentData().toInt();
    s.sortByData = -1;
    s.sort = 0;
    if (sort == 1 || sort == 2) s.sort = sort;
    else if (sort >= 3 && m_sortData->count() > 0) {
        s.sortByData = m_sortData->currentData().toInt();
        s.sortByDescending = sort == 4;
    }
    s.filter = PivotFilter();
    s.filter.kind = PivotFilter::Kind(m_filter->currentData().toInt());
    s.filter.text = m_text->text();
    s.filter.a = m_text->text().toDouble();
    s.filter.b = m_text2->text().toDouble();
    s.filter.dataField = m_filterData->count() > 0 ? m_filterData->currentData().toInt() : 0;
    s.itemsChanged = false;
    for (int i = 0; i < s.items.size() && i < m_items->count(); ++i) {
        const bool shown = m_items->item(i)->checkState() == Qt::Checked;
        if (shown != s.items[i].shown) s.itemsChanged = true;
        s.items[i].shown = shown;
    }
    return s;
}

// ---------------------------------------------------------------- Value Field Settings

ValueFieldDialog::ValueFieldDialog(const QString &sourceName, const QString &name, const QString &function, QWidget *parent) : QDialog(parent)
{
    ValueFieldSettings s;
    s.sourceName = sourceName;
    s.name = name;
    s.function = function;
    build(s);
}

ValueFieldDialog::ValueFieldDialog(const ValueFieldSettings &settings, QWidget *parent) : QDialog(parent)
{
    build(settings);
}

void ValueFieldDialog::build(const ValueFieldSettings &settings)
{
    m_settings = settings;
    setWindowTitle(tr("Value Field Settings"));
    setObjectName(QStringLiteral("valueFieldDialog"));
    auto *form = new QFormLayout(this);
    m_source = new QLabel(settings.sourceName, this);
    m_source->setTextFormat(Qt::PlainText);   // (a field's name comes from a file)
    form->addRow(tr("Source name:"), m_source);
    m_name = new QLineEdit(settings.name, this);
    m_name->setObjectName(QStringLiteral("pivot.valueName"));
    m_name->setAccessibleName(tr("Custom name"));
    form->addRow(tr("Custom name:"), m_name);
    m_functions = new QListWidget(this);
    m_functions->setObjectName(QStringLiteral("pivot.valueFunction"));
    m_functions->setAccessibleName(tr("Summarize value field by"));
    for (const char *fn : kFunctions) {
        auto *item = new QListWidgetItem(functionLabel(QLatin1String(fn)), m_functions);
        item->setData(Qt::UserRole, QString::fromLatin1(fn));
        if (settings.function == QLatin1String(fn)) m_functions->setCurrentItem(item);
    }
    form->addRow(tr("Summarize value field by:"), m_functions);
    // Show Values As
    m_showAs = new QComboBox(this);
    m_showAs->setObjectName(QStringLiteral("pivot.showAs"));
    m_showAs->setAccessibleName(tr("Show values as"));
    const std::pair<const char *, const char *> kinds[] = {
        {"normal", QT_TR_NOOP("No Calculation")}, {"percentOfTotal", QT_TR_NOOP("% of Grand Total")}, {"percentOfCol", QT_TR_NOOP("% of Column Total")},
        {"percentOfRow", QT_TR_NOOP("% of Row Total")}, {"index", QT_TR_NOOP("Index")}, {"difference", QT_TR_NOOP("Difference From")}, {"percent", QT_TR_NOOP("% Of")},
        {"percentDiff", QT_TR_NOOP("% Difference From")}, {"runTotal", QT_TR_NOOP("Running Total In")},
        {"percentOfParentRow", QT_TR_NOOP("% of Parent Row Total")}, {"percentOfParentCol", QT_TR_NOOP("% of Parent Column Total")}, {"rankAscending", QT_TR_NOOP("Rank Smallest to Largest")},
        {"rankDescending", QT_TR_NOOP("Rank Largest to Smallest")}};
    for (const auto &k : kinds) {
        m_showAs->addItem(tr(k.second), QString::fromLatin1(k.first));
        if (settings.showAs == QLatin1String(k.first)) m_showAs->setCurrentIndex(m_showAs->count() - 1);
    }
    form->addRow(tr("Show values as:"), m_showAs);
    m_baseField = new QComboBox(this);
    m_baseField->setObjectName(QStringLiteral("pivot.baseField"));
    m_baseField->setAccessibleName(tr("Base field"));
    for (const auto &b : settings.bases) {
        m_baseField->addItem(b.name, b.field);
        if (b.field == settings.baseField) m_baseField->setCurrentIndex(m_baseField->count() - 1);
    }
    form->addRow(tr("Base field:"), m_baseField);
    m_baseItem = new QComboBox(this);
    m_baseItem->setObjectName(QStringLiteral("pivot.baseItem"));
    m_baseItem->setAccessibleName(tr("Base item"));
    form->addRow(tr("Base item:"), m_baseItem);
    connect(m_baseField, &QComboBox::currentIndexChanged, this, [this] { baseChanged(); });
    connect(m_showAs, &QComboBox::currentIndexChanged, this, [this] {
        const QString k = m_showAs->currentData().toString();
        const bool ranked = k == QLatin1String("rankAscending") || k == QLatin1String("rankDescending");
        const bool relative = k == QLatin1String("difference") || k == QLatin1String("percent") || k == QLatin1String("percentDiff") || k == QLatin1String("runTotal") || ranked;
        m_baseField->setEnabled(relative && m_baseField->count() > 0);
        m_baseItem->setEnabled(relative && !ranked && k != QLatin1String("runTotal") && m_baseField->count() > 0);
    });
    baseChanged();
    // the number format, as a code
    m_format = new QComboBox(this);
    m_format->setObjectName(QStringLiteral("pivot.numberFormat"));
    m_format->setAccessibleName(tr("Number format"));
    m_format->setEditable(true);
    m_format->addItems({QString(), QStringLiteral("General"), QStringLiteral("0"), QStringLiteral("0.00"), QStringLiteral("#,##0"), QStringLiteral("#,##0.00"), QStringLiteral("$#,##0.00"),
                        QStringLiteral("0%"), QStringLiteral("0.00%"), QStringLiteral("m/d/yyyy")});
    m_format->setCurrentText(settings.numFmtCode);
    form->addRow(tr("Number format (a code; empty keeps the cells' own):"), m_format);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    Q_EMIT m_showAs->currentIndexChanged(m_showAs->currentIndex());
}

void ValueFieldDialog::baseChanged()
{
    m_baseItem->clear();
    const int field = m_baseField->currentData().toInt();
    for (const auto &b : m_settings.bases)
        if (b.field == field) {
            m_baseItem->addItem(tr("(previous)"), kPivotPreviousItem);
            m_baseItem->addItem(tr("(next)"), kPivotNextItem);
            for (int i = 0; i < b.items.size(); ++i) m_baseItem->addItem(b.items[i], i);
        }
    const int at = m_baseItem->findData(m_settings.baseItem);
    if (at >= 0) m_baseItem->setCurrentIndex(at);
    else if (m_baseItem->count() > 2) m_baseItem->setCurrentIndex(2);
}

QString ValueFieldDialog::name() const { return m_name->text(); }

QString ValueFieldDialog::function() const { return m_functions->currentItem() ? m_functions->currentItem()->data(Qt::UserRole).toString() : QStringLiteral("sum"); }

ValueFieldSettings ValueFieldDialog::settings() const
{
    ValueFieldSettings s = m_settings;
    s.name = name();
    s.function = function();
    s.showAs = m_showAs->currentData().toString();
    s.baseField = m_baseField->count() > 0 ? m_baseField->currentData().toInt() : 0;
    s.baseItem = m_baseItem->count() > 0 ? m_baseItem->currentData().toInt() : 0;
    s.numFmtCode = m_format->currentText().trimmed();
    return s;
}

} // namespace jp
