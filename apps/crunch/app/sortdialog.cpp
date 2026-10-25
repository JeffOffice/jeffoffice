#include "app/sortdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace jp {

using jp::crunch::SortLevel;
using jp::crunch::SortSpec;
using jp::office::sheet::ColorSpec;

namespace {

enum Column { LevelColumn, KeyColumn, OnColumn, OrderColumn, WhereColumn };
constexpr int kListRole = Qt::UserRole, kColorRole = Qt::UserRole + 1, kKindRole = Qt::UserRole + 2;
// the kinds of item in the order box
enum Kind { Ascending = 1, Descending, List, Custom, ColorItem, NoColorItem };

QIcon swatch(const QColor &color)
{
    QPixmap pm(14, 14);
    pm.fill(color);
    return QIcon(pm);
}

QString preview(const QStringList &list)
{
    return list.mid(0, 3).join(QStringLiteral(", ")) + QStringLiteral(", …");
}

} // namespace

SortDialog::SortDialog(const Provider &provider, const SortSpec &initial, QWidget *parent) : QDialog(parent), m_provider(provider)
{
    setObjectName(QStringLiteral("sortDialog"));
    setWindowTitle(tr("Sort"));
    m_lists = jp::crunch::builtInSortLists();
    auto *layout = new QVBoxLayout(this);
    auto *tools = new QHBoxLayout;
    m_add = new QPushButton(tr("Add Rule"));
    m_add->setObjectName(QStringLiteral("sortAddLevel"));
    m_delete = new QPushButton(tr("Remove Rule"));
    m_delete->setObjectName(QStringLiteral("sortDeleteLevel"));
    m_copy = new QPushButton(tr("Duplicate Rule"));
    m_copy->setObjectName(QStringLiteral("sortCopyLevel"));
    m_up = new QPushButton(tr("Move Up"));
    m_up->setObjectName(QStringLiteral("sortMoveUp"));
    m_down = new QPushButton(tr("Move Down"));
    m_down->setObjectName(QStringLiteral("sortMoveDown"));
    for (QPushButton *b : {m_add, m_delete, m_copy, m_up, m_down}) tools->addWidget(b);
    tools->addStretch(1);
    layout->addLayout(tools);
    m_levels = new QTableWidget(0, 5);
    m_levels->setObjectName(QStringLiteral("sortLevels"));
    m_levels->setAccessibleName(tr("Sort rules"));
    m_levels->setHorizontalHeaderLabels({QString(), tr("Column"), tr("Using"), tr("Order"), tr("Place color")});
    m_levels->verticalHeader()->hide();
    m_levels->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_levels->setSelectionMode(QAbstractItemView::SingleSelection);
    m_levels->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_levels->horizontalHeader()->setStretchLastSection(false);
    m_levels->horizontalHeader()->setSectionResizeMode(KeyColumn, QHeaderView::Stretch);
    m_levels->horizontalHeader()->setSectionResizeMode(OrderColumn, QHeaderView::Stretch);
    m_levels->setMinimumWidth(640);
    layout->addWidget(m_levels, 1);
    auto *options = new QHBoxLayout;
    m_header = new QCheckBox(tr("The first row names the columns"));
    m_header->setObjectName(QStringLiteral("sortHeader"));
    m_case = new QCheckBox(tr("Capitals count"));
    m_case->setObjectName(QStringLiteral("sortCase"));
    m_leftToRight = new QCheckBox(tr("Sort columns, not rows"));
    m_leftToRight->setObjectName(QStringLiteral("sortLeftToRight"));
    options->addWidget(m_header);
    options->addWidget(m_case);
    options->addWidget(m_leftToRight);
    options->addStretch(1);
    layout->addLayout(options);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_header->setChecked(initial.hasHeader);
    m_case->setChecked(initial.caseSensitive);
    m_leftToRight->setChecked(initial.leftToRight);
    for (const SortLevel &level : initial.levels) addLevel(level);
    if (m_levels->rowCount() == 0) addLevel(SortLevel());
    m_levels->selectRow(0);
    connect(m_add, &QPushButton::clicked, this, [this] {
        SortLevel level;
        addLevel(level);
        m_levels->selectRow(m_levels->rowCount() - 1);
    });
    connect(m_delete, &QPushButton::clicked, this, [this] {
        const int row = m_levels->currentRow();
        if (row >= 0 && m_levels->rowCount() > 1) {
            m_levels->removeRow(row);
            renumber();
        }
    });
    connect(m_copy, &QPushButton::clicked, this, [this] {
        const int row = m_levels->currentRow();
        if (row >= 0) {
            addLevel(levelOf(row));
            m_levels->selectRow(m_levels->rowCount() - 1);
        }
    });
    auto move = [this](int by) {
        const int row = m_levels->currentRow(), to = row + by;
        if (row < 0 || to < 0 || to >= m_levels->rowCount()) return;
        const SortLevel a = levelOf(row), b = levelOf(to);
        fillRow(row, b);
        fillRow(to, a);
        m_levels->selectRow(to);
    };
    connect(m_up, &QPushButton::clicked, this, [move] { move(-1); });
    connect(m_down, &QPushButton::clicked, this, [move] { move(1); });
    connect(m_header, &QCheckBox::toggled, this, [this] { refillNames(); });
    connect(m_leftToRight, &QCheckBox::toggled, this, [this] { refillNames(); });
}

void SortDialog::renumber()
{
    for (int r = 0; r < m_levels->rowCount(); ++r) m_levels->setItem(r, LevelColumn, new QTableWidgetItem(r == 0 ? tr("First by") : tr("After that")));
    m_add->setEnabled(m_levels->rowCount() < jp::crunch::kMaxSortLevels);
    m_delete->setEnabled(m_levels->rowCount() > 1);
}

void SortDialog::addLevel(const SortLevel &level)
{
    if (m_levels->rowCount() >= jp::crunch::kMaxSortLevels) return;
    const int row = m_levels->rowCount();
    m_levels->insertRow(row);
    for (int c : {KeyColumn, OnColumn, OrderColumn, WhereColumn}) {
        auto *box = new QComboBox;
        box->setObjectName(QStringLiteral("sort%1%2").arg(c == KeyColumn ? "Column" : c == OnColumn ? "On" : c == OrderColumn ? "Order" : "Where").arg(row));
        m_levels->setCellWidget(row, c, box);
    }
    auto *on = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OnColumn));
    on->addItems({tr("Values"), tr("Fill color"), tr("Text color")});
    auto *where = qobject_cast<QComboBox *>(m_levels->cellWidget(row, WhereColumn));
    where->addItems({tr("First"), tr("Last")});
    auto *key = qobject_cast<QComboBox *>(m_levels->cellWidget(row, KeyColumn));
    key->setAccessibleName(tr("Column to sort by"));
    on->setAccessibleName(tr("Using"));
    auto *orderBox = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OrderColumn));
    orderBox->setAccessibleName(tr("Order"));
    connect(orderBox, &QComboBox::activated, this, [this, orderBox](int index) {
        if (orderBox->itemData(index, kKindRole).toInt() != Custom) return;
        bool ok = false;
        const QString text = QInputDialog::getText(this, tr("Own list"), tr("Type the values in the order you want, with commas between them:"), QLineEdit::Normal, QString(), &ok);
        QStringList list;
        for (const QString &item : text.split(QLatin1Char(','), Qt::SkipEmptyParts))
            if (!item.trimmed().isEmpty() && list.size() < 500) list << item.trimmed();
        if (!ok || list.isEmpty()) {
            orderBox->setCurrentIndex(0);
            return;
        }
        orderBox->insertItem(index, preview(list));
        orderBox->setItemData(index, list, kListRole);
        orderBox->setItemData(index, List, kKindRole);
        orderBox->setCurrentIndex(index);
    });
    where->setAccessibleName(tr("Where to put the color"));
    connect(on, &QComboBox::currentIndexChanged, this, [this, on] {
        for (int r = 0; r < m_levels->rowCount(); ++r)
            if (m_levels->cellWidget(r, OnColumn) == on) fillOrder(r, nullptr);
    });
    connect(key, &QComboBox::currentIndexChanged, this, [this, key] {
        for (int r = 0; r < m_levels->rowCount(); ++r)
            if (m_levels->cellWidget(r, KeyColumn) == key) {
                const int by = qobject_cast<QComboBox *>(m_levels->cellWidget(r, OnColumn))->currentIndex();
                if (by != 0) fillOrder(r, nullptr);   // (the colors are those of the column)
            }
    });
    fillRow(row, level);
    renumber();
}

void SortDialog::refillNames()
{
    const QStringList names = m_provider.names ? m_provider.names(m_header->isChecked(), m_leftToRight->isChecked()) : QStringList();
    for (int r = 0; r < m_levels->rowCount(); ++r) {
        auto *key = qobject_cast<QComboBox *>(m_levels->cellWidget(r, KeyColumn));
        const QSignalBlocker block(key);
        const int was = std::max(0, key->currentIndex());
        key->clear();
        key->addItems(names);
        key->setCurrentIndex(std::min(was, int(names.size()) - 1));
    }
}

void SortDialog::fillRow(int row, const SortLevel &level)
{
    auto *key = qobject_cast<QComboBox *>(m_levels->cellWidget(row, KeyColumn));
    auto *on = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OnColumn));
    auto *where = qobject_cast<QComboBox *>(m_levels->cellWidget(row, WhereColumn));
    {
        const QSignalBlocker a(key), b(on), c(where);
        const QStringList names = m_provider.names ? m_provider.names(m_header->isChecked(), m_leftToRight->isChecked()) : QStringList();
        key->clear();
        key->addItems(names);
        key->setCurrentIndex(std::clamp(level.key, 0, std::max(0, int(names.size()) - 1)));
        on->setCurrentIndex(level.by == SortLevel::By::CellColor ? 1 : level.by == SortLevel::By::FontColor ? 2 : 0);
        where->setCurrentIndex(level.onTop ? 0 : 1);
    }
    fillOrder(row, &level);
}

// The order box: for values, A to Z, Z to A, and the lists; for a color, the colors in the column (and no color).
void SortDialog::fillOrder(int row, const SortLevel *level)
{
    auto *order = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OrderColumn));
    auto *on = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OnColumn));
    auto *key = qobject_cast<QComboBox *>(m_levels->cellWidget(row, KeyColumn));
    auto *where = qobject_cast<QComboBox *>(m_levels->cellWidget(row, WhereColumn));
    const QSignalBlocker block(order);
    order->clear();
    const bool color = on->currentIndex() != 0;
    where->setEnabled(color);
    if (!color) {
        order->addItem(tr("A–Z"), 0);
        order->setItemData(0, Ascending, kKindRole);
        order->addItem(tr("Z–A"), 0);
        order->setItemData(1, Descending, kKindRole);
        for (const QStringList &list : m_lists) {
            order->addItem(preview(list));
            const int at = order->count() - 1;
            order->setItemData(at, list, kListRole);
            order->setItemData(at, List, kKindRole);
        }
        order->addItem(tr("Own list…"));
        order->setItemData(order->count() - 1, Custom, kKindRole);
        int at = 0;
        if (level) {
            at = level->descending ? 1 : 0;
            if (!level->list.isEmpty()) {
                at = -1;
                for (int i = 0; i < order->count(); ++i)
                    if (order->itemData(i, kKindRole).toInt() == List && order->itemData(i, kListRole).toStringList() == level->list) at = i;
                if (at < 0) {
                    order->insertItem(order->count() - 1, preview(level->list));
                    at = order->count() - 2;
                    order->setItemData(at, level->list, kListRole);
                    order->setItemData(at, List, kKindRole);
                }
            }
        }
        order->setCurrentIndex(at);
        return;
    }
    const SortDialog::Colors colors = m_provider.colors ? m_provider.colors(std::max(0, key->currentIndex()), on->currentIndex() == 2, m_leftToRight->isChecked()) : Colors();
    int selected = -1;
    for (const auto &c : colors.colors) {
        order->addItem(swatch(c.second), c.second.name().toUpper());
        const int at = order->count() - 1;
        order->setItemData(at, ColorItem, kKindRole);
        order->setItemData(at, c.second.rgb() & 0xFFFFFF, Qt::UserRole + 3);
        order->setItemData(at, c.first.rgb, kColorRole);
        if (level && level->color.isSet() && level->color.rgb == c.first.rgb) selected = at;
    }
    if (colors.none || colors.colors.empty()) {
        order->addItem(tr("No color"));
        order->setItemData(order->count() - 1, NoColorItem, kKindRole);
        if (level && !level->color.isSet()) selected = order->count() - 1;
    }
    if (selected < 0 && level && level->color.isSet()) {
        order->addItem(swatch(QColor::fromRgb(level->color.rgb)), QColor::fromRgb(level->color.rgb).name().toUpper());
        selected = order->count() - 1;
        order->setItemData(selected, ColorItem, kKindRole);
        order->setItemData(selected, level->color.rgb, kColorRole);
    }
    order->setCurrentIndex(std::max(0, selected));
}

SortLevel SortDialog::levelOf(int row) const
{
    SortLevel level;
    auto *key = qobject_cast<QComboBox *>(m_levels->cellWidget(row, KeyColumn));
    auto *on = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OnColumn));
    auto *order = qobject_cast<QComboBox *>(m_levels->cellWidget(row, OrderColumn));
    auto *where = qobject_cast<QComboBox *>(m_levels->cellWidget(row, WhereColumn));
    level.key = std::max(0, key->currentIndex());
    level.by = on->currentIndex() == 1 ? SortLevel::By::CellColor : on->currentIndex() == 2 ? SortLevel::By::FontColor : SortLevel::By::Value;
    level.onTop = where->currentIndex() == 0;
    const int kind = order->currentData(kKindRole).toInt();
    if (level.by == SortLevel::By::Value) {
        level.descending = kind == Descending;
        if (kind == List) level.list = order->currentData(kListRole).toStringList();
    } else if (kind == ColorItem) {
        level.color = ColorSpec::fromRgb(order->currentData(kColorRole).toUInt());
    }
    return level;
}

SortSpec SortDialog::spec() const
{
    SortSpec spec;
    for (int r = 0; r < m_levels->rowCount(); ++r) spec.levels.append(levelOf(r));
    spec.hasHeader = m_header->isChecked();
    spec.caseSensitive = m_case->isChecked();
    spec.leftToRight = m_leftToRight->isChecked();
    return spec;
}

} // namespace jp
