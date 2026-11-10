// The Selection Pane: a list of a page's objects, with what a person does to them (see selectionpane.h).

#include "app/selectionpane.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

SelectionPane::SelectionPane(QWidget *parent, const Source &source) : QDialog(parent), m_source(source)
{
    setObjectName(QStringLiteral("selectionPane"));
    setWindowTitle(tr("Selection Pane"));
    setAttribute(Qt::WA_DeleteOnClose);
    auto *layout = new QVBoxLayout(this);
    if (m_source.show) {
        auto *row = new QHBoxLayout;
        m_showAll = new QPushButton(tr("Show All"), this);
        m_showAll->setObjectName(QStringLiteral("selectionShowAll"));
        m_hideAll = new QPushButton(tr("Hide All"), this);
        m_hideAll->setObjectName(QStringLiteral("selectionHideAll"));
        row->addWidget(m_showAll);
        row->addWidget(m_hideAll);
        layout->addLayout(row);
        connect(m_showAll, &QPushButton::clicked, this, [this] {
            if (m_source.showAll) m_source.showAll(true);
            refresh();
        });
        connect(m_hideAll, &QPushButton::clicked, this, [this] {
            if (m_source.showAll) m_source.showAll(false);
            refresh();
        });
        m_showAll->setVisible(bool(m_source.showAll));
        m_hideAll->setVisible(bool(m_source.showAll));
    }
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("selectionList"));
    m_list->setAccessibleName(tr("Objects on the page"));
    m_list->setDragDropMode(QAbstractItemView::InternalMove);
    m_list->setDefaultDropAction(Qt::MoveAction);
    layout->addWidget(m_list);
    auto *buttons = new QHBoxLayout;
    auto *forward = new QPushButton(tr("Bring Forward"), this);
    forward->setObjectName(QStringLiteral("selectionForward"));
    auto *backward = new QPushButton(tr("Send Backward"), this);
    backward->setObjectName(QStringLiteral("selectionBackward"));
    buttons->addWidget(forward);
    buttons->addWidget(backward);
    layout->addLayout(buttons);

    // (A signal of the list is answered inside a Busy: see m_busy.)
    struct Busy {
        SelectionPane *pane;
        explicit Busy(SelectionPane *p) : pane(p) { pane->m_busy = true; }
        ~Busy()
        {
            pane->m_busy = false;
            if (pane->m_again) QTimer::singleShot(0, pane, [p = QPointer<SelectionPane>(pane)] { if (p) p->refresh(); });
        }
    };
    connect(m_list, &QListWidget::currentRowChanged, this, [this](int r) {
        if (m_filling || r < 0 || !m_source.pick) return;
        Busy busy(this);
        m_source.pick(r);
    });
    // A box ticked, or a name typed.
    connect(m_list, &QListWidget::itemChanged, this, [this](QListWidgetItem *item) {
        if (m_filling) return;
        Busy busy(this);
        const int r = m_list->row(item);
        const QVector<Row> now = m_source.rows ? m_source.rows() : QVector<Row>();
        if (r < 0 || r >= now.size()) return;
        const bool shown = item->checkState() == Qt::Checked;
        if (m_source.show && shown != now[r].shown) m_source.show(r, shown);
        else if (m_source.rename && item->text() != now[r].name) m_source.rename(r, item->text());
        refresh();
    });
    // A row dragged to another place.
    connect(m_list->model(), &QAbstractItemModel::rowsMoved, this, [this](const QModelIndex &, int start, int, const QModelIndex &, int row) {
        if (m_filling) return;
        Busy busy(this);
        const int to = row > start ? row - 1 : row;
        if (m_source.move && to != start) m_source.move(start, to);
        refresh();
    });
    connect(forward, &QPushButton::clicked, this, [this] { moveCurrent(-1); });
    connect(backward, &QPushButton::clicked, this, [this] { moveCurrent(1); });
    refresh();
}

// One place toward the front (-1) or the back (1) of the list.
void SelectionPane::moveCurrent(int by)
{
    const int from = m_list->currentRow(), to = from + by;
    if (from < 0 || to < 0 || to >= m_list->count() || !m_source.move) return;
    m_source.move(from, to);
    refresh();
}

void SelectionPane::refresh()
{
    if (!m_source.rows) return;
    if (m_busy) {
        m_again = true;
        return;
    }
    m_again = false;
    const QVector<Row> rows = m_source.rows();
    m_filling = true;
    m_list->clear();
    int picked = -1;
    for (int i = 0; i < rows.size(); ++i) {
        auto *item = new QListWidgetItem(rows[i].name);
        Qt::ItemFlags flags = item->flags() | Qt::ItemIsDragEnabled;
        flags &= ~Qt::ItemIsDropEnabled;
        if (m_source.rename) flags |= Qt::ItemIsEditable;
        if (m_source.show) {
            flags |= Qt::ItemIsUserCheckable;
            item->setCheckState(rows[i].shown ? Qt::Checked : Qt::Unchecked);
        }
        item->setFlags(flags);
        m_list->addItem(item);
        if (rows[i].picked && picked < 0) picked = i;
    }
    if (picked >= 0) m_list->setCurrentRow(picked);
    m_filling = false;
}

} // namespace jp
