#include "app/undohistory.h"
#include "app/plaintext.h"

#include <QAccessible>
#include <QCoreApplication>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <algorithm>

namespace jp {

namespace {

// A list that marks the rows from the first to the one under the pointer.
class StepList : public QListWidget {
public:
    std::function<void(int)> hovered, picked;
    explicit StepList(QWidget *parent) : QListWidget(parent)
    {
        setMouseTracking(true);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);   // (the keys go to the menu, which marks the rows)
        setTextElideMode(Qt::ElideRight);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        viewport()->setMouseTracking(true);
    }

protected:
    void mouseMoveEvent(QMouseEvent *e) override
    {
        const QModelIndex i = indexAt(e->position().toPoint());
        if (hovered) hovered(i.isValid() ? i.row() + 1 : 0);
    }
    void mouseReleaseEvent(QMouseEvent *e) override
    {
        const QModelIndex i = indexAt(e->position().toPoint());
        if (i.isValid() && picked) picked(i.row() + 1);
    }
    void leaveEvent(QEvent *) override
    {
        if (hovered) hovered(0);
    }
};

} // namespace

UndoHistoryMenu::UndoHistoryMenu(bool redo, std::function<QStringList()> names, std::function<void(int)> run, QWidget *parent)
    : QMenu(parent), m_redo(redo), m_names(std::move(names)), m_run(std::move(run))
{
    setObjectName(redo ? QStringLiteral("redoHistoryMenu") : QStringLiteral("undoHistoryMenu"));
    auto *body = new QWidget(this);
    auto *layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *list = new StepList(body);
    m_list = list;
    list->hovered = [this](int n) { markSteps(n); };
    list->picked = [this](int n) { choose(n); };
    list->setMinimumWidth(220);
    list->setMaximumWidth(UndoHistoryMenu::kMaxWidth);
    list->setFixedHeight(180);
    list->setAccessibleName(redo ? QCoreApplication::translate("UndoHistory", "Actions that can be redone") : QCoreApplication::translate("UndoHistory", "Actions that can be undone"));
    m_footer = new QLabel(body);
    m_footer->setTextFormat(Qt::PlainText);
    m_footer->setContentsMargins(8, 4, 8, 4);
    layout->addWidget(list);
    layout->addWidget(m_footer);
    auto *wa = new QWidgetAction(this);
    wa->setDefaultWidget(body);
    addAction(wa);
    connect(this, &QMenu::aboutToShow, this, [this] { fill(); });
}

void UndoHistoryMenu::fill()
{
    m_list->clear();
    const QStringList names = m_names();
    const int n = int(std::min<qsizetype>(names.size(), kMaxShown));
    const QFontMetrics fm(m_list->font());
    for (int i = 0; i < n; ++i) {
        auto *item = new QListWidgetItem(fm.elidedText(names.at(i), Qt::ElideRight, kMaxWidth - 40), m_list);
        item->setToolTip(plainToolTip(names.at(i)));   // (the whole name where it was cut)
        item->setData(Qt::AccessibleTextRole, names.at(i));
    }
    markSteps(0);
}

int UndoHistoryMenu::shownSteps() const { return m_list->count(); }

void UndoHistoryMenu::markSteps(int count)
{
    m_marked = std::clamp(count, 0, m_list->count());
    for (int i = 0; i < m_list->count(); ++i) m_list->item(i)->setSelected(i < m_marked);   // (set in code: the list itself takes no selection from the mouse)
    if (m_marked == 0) m_footer->setText(m_redo ? QCoreApplication::translate("UndoHistory", "Redo") : QCoreApplication::translate("UndoHistory", "Undo"));
    else m_footer->setText(m_redo ? QCoreApplication::translate("UndoHistory", "Redo %n action(s)", nullptr, m_marked) : QCoreApplication::translate("UndoHistory", "Undo %n action(s)", nullptr, m_marked));
    m_list->setCurrentRow(m_marked - 1, QItemSelectionModel::NoUpdate);
    announce();
}

// The footer says what Enter would do: its text is its name for a screen reader, which is told when it changes.
void UndoHistoryMenu::announce()
{
    m_footer->setAccessibleName(m_footer->text());
    QAccessibleEvent changed(m_footer, QAccessible::NameChanged);
    QAccessible::updateAccessibility(&changed);
}

QString UndoHistoryMenu::rowText(int row) const { return row >= 0 && row < m_list->count() ? m_list->item(row)->text() : QString(); }
QString UndoHistoryMenu::rowName(int row) const { return row >= 0 && row < m_list->count() ? m_list->item(row)->data(Qt::AccessibleTextRole).toString() : QString(); }
QString UndoHistoryMenu::footerText() const { return m_footer->text(); }

int UndoHistoryMenu::page() const
{
    const int rowHeight = std::max(1, m_list->count() > 0 ? m_list->visualItemRect(m_list->item(0)).height() : 1);
    return std::max(1, m_list->viewport()->height() / rowHeight - 1);
}

void UndoHistoryMenu::keyPressEvent(QKeyEvent *e)
{
    if (e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
        QMenu::keyPressEvent(e);
        return;
    }
    switch (e->key()) {
    case Qt::Key_Down: markSteps(m_marked + 1); break;
    case Qt::Key_Up: markSteps(m_marked - 1); break;
    case Qt::Key_PageDown: markSteps(m_marked + page()); break;
    case Qt::Key_PageUp: markSteps(m_marked - page()); break;
    case Qt::Key_Home: markSteps(1); break;
    case Qt::Key_End: markSteps(m_list->count()); break;
    case Qt::Key_Return:
    case Qt::Key_Enter: choose(m_marked); break;
    case Qt::Key_Escape: close(); break;
    default: QMenu::keyPressEvent(e); return;
    }
    e->accept();
}

void UndoHistoryMenu::choose(int count)
{
    const int n = std::clamp(count, 0, m_list->count());
    close();
    if (n > 0 && m_run) m_run(n);
}

} // namespace jp
