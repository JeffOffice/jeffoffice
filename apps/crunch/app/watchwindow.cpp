#include "app/watchwindow.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace jp {

WatchWindow::WatchWindow(QWidget *parent) : QDialog(parent, Qt::Tool)
{
    setObjectName(QStringLiteral("watchWindow"));
    setWindowTitle(tr("Watch Window"));
    auto *layout = new QVBoxLayout(this);
    auto *buttons = new QHBoxLayout;
    auto *add = new QPushButton(tr("Add Watch"), this);
    add->setObjectName(QStringLiteral("watchAdd"));
    add->setToolTip(tr("Watch the selected cells"));
    auto *remove = new QPushButton(tr("Delete Watch"), this);
    remove->setObjectName(QStringLiteral("watchDelete"));
    buttons->addWidget(add);
    buttons->addWidget(remove);
    buttons->addStretch(1);
    layout->addLayout(buttons);
    m_table = new QTableWidget(0, 4, this);
    m_table->setObjectName(QStringLiteral("watchTable"));
    m_table->setHorizontalHeaderLabels({tr("Sheet"), tr("Cell"), tr("Value"), tr("Formula")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_table);
    resize(480, 260);
    connect(add, &QPushButton::clicked, this, &WatchWindow::addRequested);
    connect(remove, &QPushButton::clicked, this, [this] {
        QList<int> rows;
        for (const QModelIndex &i : m_table->selectionModel()->selectedRows()) rows << i.row();
        std::sort(rows.begin(), rows.end());
        if (!rows.isEmpty()) Q_EMIT deleteRequested(rows);
    });
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { Q_EMIT goToRequested(row); });
}

void WatchWindow::setRows(const QVector<QStringList> &rows)
{
    m_table->setRowCount(int(rows.size()));
    for (int r = 0; r < rows.size(); ++r)
        for (int c = 0; c < 4; ++c) m_table->setItem(r, c, new QTableWidgetItem(rows[r].value(c)));
}

int WatchWindow::rowCount() const { return m_table->rowCount(); }

} // namespace jp
