// The Selection Pane: a list of a page's objects, with what a person does to them (see selectionpane.h).

#include "app/selectionpane.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QShowEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

SelectionPane::SelectionPane(QWidget *parent, const Source &source) : QDialog(parent), m_source(source)
{
    setObjectName(m_source.title.isEmpty() ? QStringLiteral("selectionPane") : QStringLiteral("readingOrderPane"));
    setWindowTitle(m_source.title.isEmpty() ? tr("Selection Pane") : m_source.title);
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
    if (!m_source.note.isEmpty()) {
        auto *note = new QLabel(m_source.note, this);
        note->setObjectName(QStringLiteral("selectionNote"));
        note->setWordWrap(true);
        note->setTextFormat(Qt::PlainText);
        layout->addWidget(note);
    }
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("selectionList"));
    m_list->setAccessibleName(m_source.listName.isEmpty() ? tr("Objects on the page") : m_source.listName);
    // (A list that can't be reordered is not dragged.)
    m_list->setDragDropMode(m_source.move ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
    m_list->setDefaultDropAction(Qt::MoveAction);
    layout->addWidget(m_list);
    auto *buttons = new QHBoxLayout;
    auto *forward = new QPushButton(m_source.forwardText.isEmpty() ? tr("Bring Forward") : m_source.forwardText, this);
    forward->setObjectName(QStringLiteral("selectionForward"));
    auto *backward = new QPushButton(m_source.backwardText.isEmpty() ? tr("Send Backward") : m_source.backwardText, this);
    backward->setObjectName(QStringLiteral("selectionBackward"));
    buttons->addWidget(forward);
    buttons->addWidget(backward);
    layout->addLayout(buttons);
    forward->setVisible(bool(m_source.move));
    backward->setVisible(bool(m_source.move));

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

void SelectionPane::requestRefresh()
{
    if (!isVisible()) {
        m_stale = true;
        return;
    }
    if (m_queued) return;
    m_queued = true;
    QTimer::singleShot(0, this, [this] {
        m_queued = false;
        refresh();
    });
}

void SelectionPane::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    if (m_stale) refresh();
}

// The list from the Source. With as many rows as before, the rows are updated where they stand (a name, a box, the pick), which keeps a long list from being built again for each change.
void SelectionPane::refresh()
{
    if (!m_source.rows) return;
    if (m_busy) {
        m_again = true;
        return;
    }
    m_again = false;
    m_stale = false;
    const QVector<Row> rows = m_source.rows();
    m_filling = true;
    int picked = -1;
    for (int i = 0; i < rows.size(); ++i)
        if (rows[i].picked) {
            picked = i;
            break;
        }
    if (m_list->count() == rows.size()) {
        for (int i = 0; i < rows.size(); ++i) {
            QListWidgetItem *item = m_list->item(i);
            if (item->text() != rows[i].name) item->setText(rows[i].name);
            if (m_source.show) {
                const Qt::CheckState want = rows[i].shown ? Qt::Checked : Qt::Unchecked;
                if (item->checkState() != want) item->setCheckState(want);
            }
        }
    } else {
        m_list->clear();
        for (int i = 0; i < rows.size(); ++i) {
            auto *item = new QListWidgetItem(rows[i].name);
            Qt::ItemFlags flags = item->flags() | Qt::ItemIsDragEnabled;
            flags &= ~Qt::ItemIsDropEnabled;
            if (m_source.rename) flags |= Qt::ItemIsEditable;
            if (!m_source.show) flags &= ~Qt::ItemIsUserCheckable;   // (an item is checkable unless it is told it is not)
            if (m_source.show) {
                flags |= Qt::ItemIsUserCheckable;
                item->setCheckState(rows[i].shown ? Qt::Checked : Qt::Unchecked);
            }
            item->setFlags(flags);
            m_list->addItem(item);
        }
    }
    if (picked != m_list->currentRow() && picked >= 0) m_list->setCurrentRow(picked);
    m_filling = false;
}

} // namespace jp
