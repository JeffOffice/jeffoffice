#include "app/quickanalysis.h"

#include "app/plaintext.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCoreApplication>
#include <QEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScreen>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QGridLayout>

#include <algorithm>

namespace jp {

namespace {
constexpr int kTileWidth = 108, kTileHeight = 76, kIconSize = 40, kPerRow = 5;
}

QuickAnalysis::QuickAnalysis(QWidget *host, QWidget *focus, const Tabs &tabs) : QObject(host), m_host(host), m_focus(focus), m_tabsOf(tabs)
{
    m_button = new QToolButton(host);
    m_button->setObjectName(QStringLiteral("quickAnalysisButton"));
    m_button->setAccessibleName(QCoreApplication::translate("jp::QuickAnalysis", "Quick Analysis"));
    m_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_button->setIconSize(QSize(16, 16));
    m_button->setFixedSize(24, 24);
    m_button->setFocusPolicy(Qt::NoFocus);   // (a click on it leaves the keyboard where it was)
    m_button->setAutoRaise(false);
    m_button->hide();
    connect(m_button, &QToolButton::clicked, this, [this] {
        if (isOpen()) close();
        else open();
    });
}

QuickAnalysis::~QuickAnalysis()
{
    if (m_box) {
        m_box->removeEventFilter(this);
        delete m_box;
    }
    if (m_button) delete m_button;
}

void QuickAnalysis::setButtonIcon(const QIcon &icon)
{
    if (m_button) m_button->setIcon(icon);
}

void QuickAnalysis::setButtonToolTip(const QString &text)
{
    if (m_button) m_button->setToolTip(plainToolTip(text));
}

bool QuickAnalysis::isShown() const { return m_button && m_button->isVisible(); }

void QuickAnalysis::hide()
{
    close();
    if (m_button) m_button->hide();
}

void QuickAnalysis::showAt(const QRect &corner, const QRect &handle)
{
    if (!m_button || !m_host) return;
    if (corner.isNull()) {
        hide();
        return;
    }
    const QSize size = m_button->size();
    const QRect area = m_host->rect();
    // (outside the corner and clear of the fill handle: right and below first, then left of it, then above)
    const int right = std::max(corner.right(), handle.right()) + 3, below = std::max(corner.bottom(), handle.bottom()) + 3;
    const int left = std::min(corner.right(), handle.left()) - size.width() - 3, above = std::min(corner.bottom(), handle.top()) - size.height() - 3;
    const QPoint tries[] = {QPoint(right, below), QPoint(left, below), QPoint(right, above), QPoint(left, above)};
    QPoint at = tries[0];
    for (const QPoint &p : tries) {
        const QRect r(p, size);
        if (area.contains(r) && !r.intersects(handle)) {
            at = p;
            break;
        }
    }
    m_button->move(at);
    m_button->raise();
    m_button->show();
}

bool QuickAnalysis::isOpen() const { return m_box && m_box->isVisible(); }

QString QuickAnalysis::currentTab() const { return m_tab >= 0 && m_tab < m_tabs.size() ? m_tabs[m_tab].id : QString(); }

void QuickAnalysis::build()
{
    if (m_box) {
        m_box->removeEventFilter(this);
        delete m_box;
    }
    m_tabButtons.clear();
    m_tiles.clear();
    m_current.clear();
    QWidget *top = m_host ? m_host->window() : nullptr;
    m_box = new QFrame(top, Qt::Popup);
    m_box->setObjectName(QStringLiteral("quickAnalysisBox"));
    m_box->setFrameShape(QFrame::StyledPanel);
    m_box->setAttribute(Qt::WA_ShowWithoutActivating, false);
    m_box->installEventFilter(this);
    auto *column = new QVBoxLayout(m_box);
    column->setContentsMargins(6, 6, 6, 6);
    column->setSpacing(4);
    auto *tabRow = new QHBoxLayout();
    tabRow->setSpacing(2);
    column->addLayout(tabRow);
    auto *tabGroup = new QButtonGroup(m_box);
    tabGroup->setExclusive(true);
    auto *body = new QWidget(m_box);
    auto *grid = new QGridLayout(body);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(2);
    column->addWidget(body);
    int wide = 0, high = 1;
    for (const QuickTab &tab : m_tabs) {
        wide = std::max<int>(wide, std::min<int>(kPerRow, int(tab.choices.size())));
        high = std::max<int>(high, (int(tab.choices.size()) + kPerRow - 1) / kPerRow);
    }
    body->setMinimumSize(std::max(3, wide) * (kTileWidth + 2), high * (kTileHeight + 2));
    m_tiles.resize(m_tabs.size());
    for (int t = 0; t < m_tabs.size(); ++t) {
        auto *tabButton = new QToolButton(m_box);
        tabButton->setObjectName(QStringLiteral("quickTab.") + m_tabs[t].id);
        tabButton->setText(m_tabs[t].title);
        tabButton->setCheckable(true);
        tabButton->setAutoRaise(true);
        tabButton->setFocusPolicy(Qt::NoFocus);
        tabGroup->addButton(tabButton, t);
        tabRow->addWidget(tabButton);
        m_tabButtons.append(tabButton);
        connect(tabButton, &QToolButton::clicked, this, [this, t] { showTab(t); });
        for (const QuickChoice &c : m_tabs[t].choices) {
            auto *tile = new QToolButton(body);
            tile->setObjectName(QStringLiteral("quickChoice.") + c.id);
            tile->setText(c.text);
            tile->setIcon(c.icon);
            tile->setIconSize(QSize(kIconSize, kIconSize));
            tile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            tile->setFixedSize(kTileWidth, kTileHeight);
            tile->setToolTip(plainToolTip(c.tip));
            tile->setAccessibleName(c.text);
            tile->setAccessibleDescription(c.tip);
            tile->setAutoRaise(true);
            tile->setCheckable(true);
            tile->setEnabled(c.enabled);
            tile->setFocusPolicy(Qt::NoFocus);
            tile->setAttribute(Qt::WA_Hover, true);
            tile->installEventFilter(this);
            tile->hide();
            grid->addWidget(tile, int(m_tiles[t].size()) / kPerRow, int(m_tiles[t].size()) % kPerRow);
            m_tiles[t].append(tile);
            const QString id = c.id;
            connect(tile, &QToolButton::clicked, this, [this, id] { choose(id); });
        }
    }
    column->addStretch(0);
}

void QuickAnalysis::showTab(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    endLook();
    m_tab = index;
    for (int t = 0; t < m_tiles.size(); ++t)
        for (QToolButton *tile : m_tiles[t]) {
            tile->setVisible(t == index);
            tile->setChecked(false);
        }
    for (int t = 0; t < m_tabButtons.size(); ++t) m_tabButtons[t]->setChecked(t == index);
}

bool QuickAnalysis::open()
{
    if (!m_tabsOf || !m_host) return false;
    close();
    m_tabs = m_tabsOf();
    if (m_tabs.isEmpty()) return false;
    build();
    m_tab = 0;
    showTab(0);
    m_box->adjustSize();
    QPoint at = m_host->mapToGlobal(QPoint(0, 0));
    if (m_button && m_button->isVisible()) at = m_button->mapToGlobal(QPoint(0, m_button->height() + 2));
    if (const QScreen *screen = m_host->screen() ? m_host->screen() : QGuiApplication::primaryScreen()) {
        const QRect avail = screen->availableGeometry();
        if (at.y() + m_box->height() > avail.bottom() && m_button) at.setY(m_button->mapToGlobal(QPoint(0, 0)).y() - m_box->height() - 2);
        at.setX(std::max(avail.left(), std::min(at.x(), avail.right() - m_box->width())));
        at.setY(std::max(avail.top(), at.y()));
    }
    m_box->move(at);
    m_box->show();
    Q_EMIT opened();
    return true;
}

void QuickAnalysis::endLook()
{
    if (m_current.isEmpty()) return;
    m_current.clear();
    for (const auto &list : m_tiles)
        for (QToolButton *tile : list)
            if (tile->isChecked()) tile->setChecked(false);
    Q_EMIT previewEnded();
}

void QuickAnalysis::close()
{
    if (!m_box || m_closing) return;
    m_closing = true;
    const bool wasOpen = m_box->isVisible();
    m_box->hide();
    endLook();
    m_closing = false;
    if (wasOpen && m_focus) QTimer::singleShot(0, this, [this] {
        if (!m_focus || QApplication::activePopupWidget()) return;
        if (QWidget *window = m_focus->window(); window->isVisible() && QApplication::activeWindow() != window) window->activateWindow();
        m_focus->setFocus(Qt::OtherFocusReason);
    });
}

void QuickAnalysis::look(const QString &id)
{
    if (id == m_current) return;
    if (!m_current.isEmpty()) {
        m_current.clear();
        Q_EMIT previewEnded();
    }
    for (int t = 0; t < m_tabs.size(); ++t)
        for (int i = 0; i < m_tabs[t].choices.size(); ++i) {
            if (m_tabs[t].choices[i].id != id) continue;
            if (!m_tabs[t].choices[i].enabled) return;
            if (t != m_tab) showTab(t);
            for (QToolButton *tile : m_tiles[t]) tile->setChecked(false);
            m_tiles[t][i]->setChecked(true);
            m_current = id;
            Q_EMIT previewed(id);
            return;
        }
}

void QuickAnalysis::choose(QString id)
{
    for (const QuickTab &tab : m_tabs)
        for (const QuickChoice &c : tab.choices)
            if (c.id == id && c.enabled) {
                close();   // (the look is over before the choice is made)
                Q_EMIT chosen(id);
                return;
            }
}

void QuickAnalysis::selectTab(const QString &id)
{
    for (int t = 0; t < m_tabs.size(); ++t)
        if (m_tabs[t].id == id) showTab(t);
}

// The arrow keys: Left and Right go over the choices of the tab, Up and Down over the tabs.
void QuickAnalysis::move(int dx, int dy)
{
    if (m_tab < 0 || m_tab >= m_tabs.size()) return;
    if (dy) {
        const int next = std::max(0, std::min(int(m_tabs.size()) - 1, m_tab + dy));
        if (next != m_tab) showTab(next);
        return;
    }
    const auto &choices = m_tabs[m_tab].choices;
    int at = -1;
    for (int i = 0; i < choices.size(); ++i)
        if (choices[i].id == m_current) at = i;
    for (int step = 1; step <= choices.size(); ++step) {
        const int i = at < 0 ? (dx > 0 ? step - 1 : int(choices.size()) - step) : ((at + dx * step) % int(choices.size()) + int(choices.size())) % int(choices.size());
        if (choices[i].enabled) {
            look(choices[i].id);
            return;
        }
    }
}

bool QuickAnalysis::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_box) {
        if (event->type() == QEvent::Hide && !m_closing) {
            m_closing = true;   // (closed by the system: a click outside)
            endLook();
            m_closing = false;
            if (m_focus) QTimer::singleShot(0, this, [this] {
                if (m_focus && !QApplication::activePopupWidget()) m_focus->setFocus(Qt::OtherFocusReason);
            });
        } else if (event->type() == QEvent::KeyPress) {
            auto *k = static_cast<QKeyEvent *>(event);
            switch (k->key()) {
            case Qt::Key_Left: move(-1, 0); return true;
            case Qt::Key_Right: move(1, 0); return true;
            case Qt::Key_Up: move(0, -1); return true;
            case Qt::Key_Down: move(0, 1); return true;
            case Qt::Key_Escape: close(); return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
            case Qt::Key_Space:
                if (!m_current.isEmpty()) choose(m_current);
                return true;
            default: break;
            }
        }
        return false;
    }
    // a tile: the pointer over it is a look at it, and off it the look is over
    if (event->type() == QEvent::Enter || event->type() == QEvent::HoverEnter) {
        for (int t = 0; t < m_tiles.size(); ++t)
            for (int i = 0; i < m_tiles[t].size(); ++i)
                if (m_tiles[t][i] == watched && t == m_tab) look(m_tabs[t].choices[i].id);
    } else if (event->type() == QEvent::Leave || event->type() == QEvent::HoverLeave) {
        for (int i = 0; i < m_tiles.value(m_tab).size(); ++i)
            if (m_tiles[m_tab][i] == watched && m_current == m_tabs[m_tab].choices[i].id) endLook();
    }
    return false;
}

QuickPreviewLayer::QuickPreviewLayer(QWidget *host) : QWidget(host)
{
    setObjectName(QStringLiteral("quickPreviewLayer"));
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setFocusPolicy(Qt::NoFocus);
    hide();
}

void QuickPreviewLayer::showLook(const Painter &painter)
{
    m_painter = painter;
    if (QWidget *host = parentWidget()) setGeometry(host->rect());
    raise();
    show();
    update();
}

void QuickPreviewLayer::clear()
{
    m_painter = nullptr;
    hide();
}

void QuickPreviewLayer::paintEvent(QPaintEvent *)
{
    if (!m_painter) return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    m_painter(p);
}

} // namespace jp
