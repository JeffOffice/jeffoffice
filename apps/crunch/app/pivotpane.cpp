#include "app/pivotpane.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
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
    m_title->setWordWrap(true);
    layout->addWidget(m_title);
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
    for (QToolButton *b : {m_moveTo, m_up, m_down, m_remove, m_settings}) {
        b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        buttons->addWidget(b);
    }
    buttons->addStretch(1);
    layout->addLayout(buttons);
    connect(m_up, &QToolButton::clicked, this, [this] { shiftSelected(-1); });
    connect(m_down, &QToolButton::clicked, this, [this] { shiftSelected(1); });
    connect(m_remove, &QToolButton::clicked, this, [this] { removeSelected(); });
    connect(m_settings, &QToolButton::clicked, this, [this] {
        const int b = selectedBox();
        if (b != 3) return;
        const Entry e = entryOf(m_boxes[3]->currentItem());
        if (e.data >= 0) Q_EMIT valueSettingsRequested(e.data);
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
            if (m_filling || m_boxes[i]->selectedItems().isEmpty()) return;
            for (int k = 0; k < 4; ++k)
                if (k != i) {
                    const QSignalBlocker block(m_boxes[k]);
                    m_boxes[k]->clearSelection();
                    m_boxes[k]->setCurrentRow(-1);
                }
        });
    showNothing();
}

void PivotFieldsPane::showNothing()
{
    m_filling = true;
    m_fields->clear();
    for (QListWidget *b : m_boxes) b->clear();
    m_table = PivotTable();
    m_names.clear();
    m_numeric.clear();
    m_title->setText(tr("Click a pivot table to see its fields."));
    m_filling = false;
}

void PivotFieldsPane::showTable(const QString &name, const PivotTable &table, const QStringList &fieldNames, const QVector<bool> &numeric, const QVector<QString> &)
{
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
    Q_EMIT changeRequested(table, text);
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

// ---------------------------------------------------------------- Value Field Settings

ValueFieldDialog::ValueFieldDialog(const QString &sourceName, const QString &name, const QString &function, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Value Field Settings"));
    auto *form = new QFormLayout(this);
    m_source = new QLabel(sourceName, this);
    form->addRow(tr("Source name:"), m_source);
    m_name = new QLineEdit(name, this);
    m_name->setObjectName(QStringLiteral("pivot.valueName"));
    m_name->setAccessibleName(tr("Custom name"));
    form->addRow(tr("Custom name:"), m_name);
    m_functions = new QListWidget(this);
    m_functions->setObjectName(QStringLiteral("pivot.valueFunction"));
    m_functions->setAccessibleName(tr("Summarize value field by"));
    for (const char *fn : kFunctions) {
        auto *item = new QListWidgetItem(functionLabel(QLatin1String(fn)), m_functions);
        item->setData(Qt::UserRole, QString::fromLatin1(fn));
        if (function == QLatin1String(fn)) m_functions->setCurrentItem(item);
    }
    form->addRow(tr("Summarize value field by:"), m_functions);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString ValueFieldDialog::name() const { return m_name->text(); }

QString ValueFieldDialog::function() const { return m_functions->currentItem() ? m_functions->currentItem()->data(Qt::UserRole).toString() : QStringLiteral("sum"); }

} // namespace jp
