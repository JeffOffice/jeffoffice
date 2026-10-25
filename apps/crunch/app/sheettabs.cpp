#include "app/sheettabs.h"

#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStylePainter>

#include <algorithm>

namespace jp::crunch {

namespace {

// The line a sheet's name is typed in: Esc gives it up.
class NameEditor : public QLineEdit {
public:
    using QLineEdit::QLineEdit;
    bool cancelled = false;

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Escape) {
            cancelled = true;
            clearFocus();
            e->accept();
            return;
        }
        QLineEdit::keyPressEvent(e);
    }
};

} // namespace

SheetTabBar::SheetTabBar(QWidget *parent) : QTabBar(parent)
{
    setObjectName(QStringLiteral("sheetTabs"));
    setShape(QTabBar::RoundedSouth);
    setExpanding(false);
    setDrawBase(false);
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideRight);
    setMovable(true);
    setAccessibleName(tr("Sheets"));
    // A tab the cursor goes to that is not among those picked is picked alone.
    connect(this, &QTabBar::currentChanged, this, [this](int index) {
        if (index >= 0 && !m_moving && !m_selected.contains(index)) selectOnly(index);
    });
    // Tabs moved while the button is down are a drag (each one that is passed moves over one place): the sheets are told once, when it is let go.
    connect(this, &QTabBar::tabMoved, this, [this] {
        if (m_pressed) m_dragMoved = true;
    });
}

void SheetTabBar::setStripe(int tab, const QColor &color)
{
    if (tab < 0) return;
    if (m_stripes.size() <= tab) m_stripes.resize(tab + 1);
    m_stripes[tab] = color;
    update();
}

void SheetTabBar::clearTabs()
{
    cancelRename();
    while (count() > 0) removeTab(count() - 1);
    m_stripes.clear();
    m_selected.clear();
}

QVector<int> SheetTabBar::selectedTabs() const
{
    QVector<int> out;
    for (int i = 0; i < count(); ++i)
        if (m_selected.contains(i) || i == currentIndex()) out.append(i);
    return out;
}

void SheetTabBar::selectOnly(int tab)
{
    m_selected.clear();
    if (tab >= 0 && tab < count()) m_selected.insert(tab);
    update();
    Q_EMIT selectionChanged();
}

void SheetTabBar::setSelectedTabs(const QSet<int> &tabs)
{
    m_selected = tabs;
    update();
    Q_EMIT selectionChanged();
}

void SheetTabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
    // (the stripes and the picked tabs after it are one place later)
    if (index < m_stripes.size()) m_stripes.insert(index, QColor());
    QSet<int> moved;
    for (const int t : m_selected) moved.insert(t >= index ? t + 1 : t);
    m_selected = moved;
}

void SheetTabBar::tabRemoved(int index)
{
    QTabBar::tabRemoved(index);
    if (index < m_stripes.size()) m_stripes.remove(index);
    QSet<int> moved;
    for (const int t : m_selected)
        if (t != index) moved.insert(t > index ? t - 1 : t);
    m_selected = moved;
}

QSize SheetTabBar::tabSizeHint(int index) const
{
    QSize s = QTabBar::tabSizeHint(index);
    s.setHeight(std::max(s.height(), 24));
    s.setWidth(std::min(std::max(s.width(), 60), 220));
    return s;
}

void SheetTabBar::mousePressEvent(QMouseEvent *e)
{
    const int tab = tabAt(e->position().toPoint());
    if (e->button() == Qt::LeftButton && tab >= 0 && (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
        // (Ctrl adds the tab to those picked, or takes it away; Shift picks the tabs from the one with the cursor to it; the tab with the cursor stays where it is)
        if (e->modifiers() & Qt::ShiftModifier) {
            const int from = std::max(0, currentIndex());
            m_selected.clear();
            for (int i = std::min(from, tab); i <= std::max(from, tab); ++i) m_selected.insert(i);
        } else if (tab != currentIndex()) {
            if (m_selected.contains(tab)) m_selected.remove(tab);
            else m_selected.insert(tab);
        }
        update();
        Q_EMIT selectionChanged();
        e->accept();
        return;
    }
    const bool inGroup = tab >= 0 && selectedTabs().size() > 1 && selectedTabs().contains(tab);
    m_pressed = e->button() == Qt::LeftButton && tab >= 0;
    m_dragFrom = tab;
    m_dragMoved = false;
    QTabBar::mousePressEvent(e);
    // (a click on a tab outside those picked picks that one alone; one among them keeps the group, and is the one with the cursor)
    if (e->button() == Qt::LeftButton && tab >= 0 && !inGroup) selectOnly(currentIndex());
    else if (e->button() == Qt::LeftButton && inGroup) update();
}

void SheetTabBar::mouseMoveEvent(QMouseEvent *e)
{
    m_moving = true;
    QTabBar::mouseMoveEvent(e);
    m_moving = false;
}

void SheetTabBar::mouseReleaseEvent(QMouseEvent *e)
{
    QTabBar::mouseReleaseEvent(e);
    // (the tab pressed is the one with the cursor, and goes where the tabs are when it is let go)
    const int from = m_dragFrom, to = currentIndex();
    const bool moved = m_pressed && m_dragMoved;
    m_pressed = m_dragMoved = false;
    m_dragFrom = -1;
    if (moved && from >= 0 && to >= 0 && from != to) {
        // (the tab dragged is the only one picked)
        selectOnly(currentIndex());
        Q_EMIT tabDragged(from, to);
    }
}

void SheetTabBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    const int tab = tabAt(e->position().toPoint());
    if (e->button() == Qt::LeftButton && tab >= 0) {
        beginRename(tab);
        e->accept();
        return;
    }
    QTabBar::mouseDoubleClickEvent(e);
}

void SheetTabBar::contextMenuEvent(QContextMenuEvent *e)
{
    const int tab = tabAt(e->pos());
    if (tab < 0) {
        QTabBar::contextMenuEvent(e);
        return;
    }
    // (a tab that is not among those picked is picked alone, and gets the cursor, as it would with a click)
    if (!selectedTabs().contains(tab)) {
        setCurrentIndex(tab);
        selectOnly(tab);
    }
    Q_EMIT contextRequested(tab, e->globalPos());
    e->accept();
}


void SheetTabBar::cancelRename()
{
    if (!m_editor) return;
    QLineEdit *edit = m_editor;
    static_cast<NameEditor *>(edit)->cancelled = true;
    m_editor = nullptr;   // (the line losing the focus as it goes is not a name given)
    edit->hide();
    edit->deleteLater();
}

void SheetTabBar::beginRename(int tab)
{
    if (tab < 0 || tab >= count()) return;
    cancelRename();
    auto *edit = new NameEditor(this);
    edit->setObjectName(QStringLiteral("sheetNameEditor"));
    edit->setText(tabData(tab).isValid() ? tabData(tab).toString() : tabText(tab));
    edit->setMaxLength(64);
    edit->setAccessibleName(tr("Sheet name"));
    edit->setGeometry(tabRect(tab).adjusted(2, 2, -2, -2));
    edit->selectAll();
    edit->show();
    edit->setFocus();
    m_editor = edit;
    connect(edit, &QLineEdit::editingFinished, this, [this, edit, tab] {
        if (m_editor != edit) return;
        const QString text = edit->text();
        const bool cancelled = static_cast<NameEditor *>(edit)->cancelled;
        m_editor = nullptr;
        edit->deleteLater();
        if (!cancelled) Q_EMIT renamed(tab, text);
    });
}

void SheetTabBar::paintEvent(QPaintEvent *e)
{
    QTabBar::paintEvent(e);
    QPainter p(this);
    // the tabs picked besides the one with the cursor
    for (const int i : selectedTabs()) {
        if (i == currentIndex() || selectedTabs().size() < 2) continue;
        QColor c = palette().color(QPalette::Highlight);
        c.setAlpha(60);
        p.fillRect(tabRect(i).adjusted(2, 2, -2, -2), c);
    }
    for (int i = 0; i < count() && i < m_stripes.size(); ++i) {
        if (!m_stripes[i].isValid()) continue;
        const QRect r = tabRect(i);
        p.fillRect(QRect(r.left() + 4, r.bottom() - 3, r.width() - 8, 3), m_stripes[i]);
    }
}

} // namespace jp::crunch
