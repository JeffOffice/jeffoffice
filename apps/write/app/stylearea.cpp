#include "app/stylearea.h"

#include "app/icons.h"
#include "app/plaintext.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <algorithm>

namespace jp::write {

namespace {
constexpr int kEdgeGrab = 4;     // pixels at the edge toward the text that take the drag
constexpr int kMostRows = 2000;  // rows kept (a very small zoom shows paragraphs by the hundred; no window shows more)

// A name, cut with an ellipsis where it is longer than its room.
class NameLabel : public QLabel {
public:
    explicit NameLabel(QWidget *parent) : QLabel(parent)
    {
        setTextFormat(Qt::PlainText);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setContentsMargins(4, 0, 4, 0);
    }
    bool cut() const { return m_cut; }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setPen(uiText());
        const QRect r = contentsRect();
        const QFontMetrics fm(font());
        const QString shown = fm.elidedText(text(), Qt::ElideRight, r.width());
        m_cut = shown != text();
        p.drawText(r, Qt::AlignLeading | Qt::AlignTop, shown);
    }

private:
    mutable bool m_cut = false;
};
} // namespace

StyleArea::StyleArea(QAbstractScrollArea *host, RowSource source, std::function<void(int)> setMargin) : QWidget(host), m_host(host), m_source(std::move(source)), m_setMargin(std::move(setMargin))
{
    setObjectName(QStringLiteral("styleArea"));
    setAccessibleName(QCoreApplication::translate("jp::write::StyleArea", "Style area"));
    setAccessibleDescription(QCoreApplication::translate("jp::write::StyleArea", "The style of each paragraph, beside its first line."));
    setFocusPolicy(Qt::TabFocus);
    setMouseTracking(true);
    setAutoFillBackground(false);
    hide();
    host->viewport()->installEventFilter(this);
    host->installEventFilter(this);
    connect(host->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { if (m_width) refresh(); });
    connect(host->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this] { scheduleRefresh(); });
}

void StyleArea::setAreaWidth(int width)
{
    const int w = width <= 0 ? 0 : std::clamp(width, kLeastWidth, kMostWidth);
    if (w == m_width) return;
    m_width = w;
    if (m_setMargin) m_setMargin(w);
    setVisible(w > 0);
    if (w > 0) {
        place();
        refresh();
    } else {
        m_rows.clear();
        for (QLabel *l : std::as_const(m_labels)) l->hide();
        m_current = -1;
    }
}

// The strip sits in the margin the viewport leaves, whichever side of the window that is (a mirrored window puts it on the right).
void StyleArea::place()
{
    if (!m_width) return;
    const QRect vp = m_host->viewport()->geometry();
    const int x = vp.left() >= m_width ? vp.left() - m_width : vp.right() + 1;
    setGeometry(x, vp.top(), m_width, vp.height());
}

// (Escape, Enter, and the arrows are the strip's own keys while it has the focus: no window shortcut takes them.)
bool StyleArea::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        const int key = static_cast<QKeyEvent *>(e)->key();
        if (key == Qt::Key_Escape || key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Up || key == Qt::Key_Down) {
            e->accept();
            return true;
        }
    }
    return QWidget::event(e);
}

bool StyleArea::eventFilter(QObject *watched, QEvent *e)
{
    if (m_width && (e->type() == QEvent::Resize || e->type() == QEvent::Move || e->type() == QEvent::LayoutRequest || e->type() == QEvent::Show)) {
        place();
        if (watched == m_host->viewport() && e->type() != QEvent::Move) scheduleRefresh();
    }
    return QWidget::eventFilter(watched, e);
}

void StyleArea::scheduleRefresh()
{
    if (!m_width || m_pending) return;
    m_pending = true;
    QTimer::singleShot(0, this, [this] {
        m_pending = false;
        refresh();
    });
}

void StyleArea::refresh()
{
    if (!m_width || !m_source) return;
    ++m_refreshes;
    place();
    QVector<StyleRow> rows = m_source();
    if (rows.size() > kMostRows) rows.resize(kMostRows);
    m_rows = rows;
    fitLabels();
    if (m_current >= m_rows.size()) m_current = m_rows.isEmpty() ? -1 : m_rows.size() - 1;
    update();
}

void StyleArea::fitLabels()
{
    while (m_labels.size() < m_rows.size()) {
        auto *l = new NameLabel(this);
        l->setObjectName(QStringLiteral("styleAreaName"));
        m_labels << l;
    }
    for (int i = 0; i < m_labels.size(); ++i) {
        QLabel *l = m_labels[i];
        if (i >= m_rows.size()) {
            l->hide();
            continue;
        }
        const StyleRow &r = m_rows[i];
        // (The name's own line is as tall as the paragraph's first line, at least one line of the strip's type.)
        const int h = std::max(r.height, QFontMetrics(l->font()).height());
        l->setGeometry(0, r.top, m_width - 1, h);
        if (l->text() != r.name) {
            l->setTextFormat(Qt::PlainText);
            l->setText(r.name);
            l->setAccessibleName(r.name);
            l->setToolTip(jp::plainToolTip(r.name));
        }
        l->show();
    }
}

QStringList StyleArea::shownNames() const
{
    QStringList out;
    for (const StyleRow &r : m_rows) out << r.name;
    return out;
}

int StyleArea::rowAt(const QPoint &point) const
{
    for (int i = 0; i < m_rows.size(); ++i) {
        const StyleRow &r = m_rows[i];
        if (point.y() >= r.top && point.y() < r.top + std::max(r.height, QFontMetrics(font()).height())) return i;
    }
    return -1;
}

bool StyleArea::nameCut(int row) const
{
    if (row < 0 || row >= m_rows.size()) return false;
    return QFontMetrics(font()).horizontalAdvance(m_rows[row].name) > m_width - 1 - 8;
}

bool StyleArea::onEdge(const QPoint &p) const
{
    // The edge toward the text is the right one where the strip is on the left of the viewport.
    const bool onLeftOfText = x() < m_host->viewport()->x();
    return onLeftOfText ? p.x() >= width() - kEdgeGrab : p.x() < kEdgeGrab;
}

void StyleArea::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const bool dark = uiDark();
    p.fillRect(rect(), dark ? QColor(0x2A, 0x2A, 0x2A) : QColor(0xF1, 0xF2, 0xF4));
    const bool onLeftOfText = x() < m_host->viewport()->x();
    p.setPen(dark ? QColor(0x55, 0x55, 0x55) : QColor(0xC4, 0xC8, 0xCE));
    const int edgeX = onLeftOfText ? width() - 1 : 0;
    p.drawLine(edgeX, 0, edgeX, height());
    if (m_current >= 0 && m_current < m_rows.size() && hasFocus()) {
        const StyleRow &r = m_rows[m_current];
        p.setPen(QPen(palette().highlight().color(), 2));
        p.drawRect(QRect(1, r.top, width() - 3, std::max(r.height, QFontMetrics(font()).height())));
    }
}

void StyleArea::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return;
    if (onEdge(e->position().toPoint())) {
        m_dragging = true;
        m_dragFrom = e->globalPosition().toPoint().x();
        m_dragWidth = m_width;
        return;
    }
    const int row = rowAt(e->position().toPoint());
    if (row >= 0) {
        m_current = row;
        Q_EMIT rowClicked(m_rows[row].position);
        m_host->setFocus(Qt::MouseFocusReason);
    }
}

void StyleArea::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging) {
        const bool onLeftOfText = x() < m_host->viewport()->x();
        const int moved = e->globalPosition().toPoint().x() - m_dragFrom;
        const int w = std::clamp(m_dragWidth + (onLeftOfText ? moved : -moved), int(kLeastWidth), int(kMostWidth));
        if (w != m_width) {
            setAreaWidth(w);
            Q_EMIT widthDragged(w);
        }
        return;
    }
    setCursor(onEdge(e->position().toPoint()) ? Qt::SplitHCursor : Qt::ArrowCursor);
}

void StyleArea::mouseReleaseEvent(QMouseEvent *)
{
    m_dragging = false;
}

void StyleArea::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || onEdge(e->position().toPoint())) return;
    const int row = rowAt(e->position().toPoint());
    if (row >= 0) Q_EMIT rowDoubleClicked(m_rows[row].position);
}

// The keyboard: Up and Down go from name to name on show, Enter opens the style's box, Escape returns to the text.
void StyleArea::keyPressEvent(QKeyEvent *e)
{
    if (m_rows.isEmpty()) {
        QWidget::keyPressEvent(e);
        return;
    }
    switch (e->key()) {
    case Qt::Key_Down: m_current = std::min(m_current + 1, int(m_rows.size()) - 1); break;
    case Qt::Key_Up: m_current = std::max(m_current - 1, 0); break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_current >= 0) Q_EMIT rowDoubleClicked(m_rows[m_current].position);
        return;
    case Qt::Key_Escape: m_host->setFocus(Qt::OtherFocusReason); return;
    default: QWidget::keyPressEvent(e); return;
    }
    update();
}

void StyleArea::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    if (m_current < 0 && !m_rows.isEmpty()) m_current = 0;
    update();
}

void StyleArea::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    update();
}

} // namespace jp::write
