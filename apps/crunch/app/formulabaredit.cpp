#include "app/formulabaredit.h"

#include <QApplication>
#include <QFontMetrics>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QTextOption>
#include <QWheelEvent>

#include <algorithm>

namespace jp {

// The handle at the bottom edge of an expanded bar: dragged, or given Up and Down on the keyboard, it makes the bar taller or shorter.
class FormulaBarGrip : public QWidget {
public:
    explicit FormulaBarGrip(FormulaBarEdit *bar) : QWidget(bar), m_bar(bar)
    {
        setObjectName(QStringLiteral("formulaBarGrip"));
        setCursor(Qt::SizeVerCursor);
        setFocusPolicy(Qt::StrongFocus);
        setFixedHeight(6);
        setAccessibleName(QCoreApplication::translate("jp::FormulaBarEdit", "Formula bar height"));
        setAccessibleDescription(QCoreApplication::translate("jp::FormulaBarEdit", "Press Up or Down to make the formula bar shorter or taller, or drag this edge with the mouse."));
        setToolTip(QCoreApplication::translate("jp::FormulaBarEdit", "Drag to change the height of the formula bar"));
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        const QColor c = palette().color(QPalette::Mid);
        p.setPen(QPen(hasFocus() ? palette().color(QPalette::Highlight) : c, 2));
        p.drawLine(width() / 2 - 12, height() / 2, width() / 2 + 12, height() / 2);
    }
    void mousePressEvent(QMouseEvent *e) override { if (e->button() == Qt::LeftButton) setFocus(Qt::MouseFocusReason); }
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (!(e->buttons() & Qt::LeftButton)) return;
        const int y = m_bar->mapFromGlobal(e->globalPosition().toPoint()).y();
        const int lh = std::max(1, m_bar->fontMetrics().lineSpacing());
        m_bar->setExpandedLines(int((y - 6 + lh / 2) / lh));
    }
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Up) m_bar->setExpandedLines(m_bar->expandedLines() - 1);
        else if (e->key() == Qt::Key_Down) m_bar->setExpandedLines(m_bar->expandedLines() + 1);
        else QWidget::keyPressEvent(e);
    }
    void focusInEvent(QFocusEvent *e) override { QWidget::focusInEvent(e); update(); }
    void focusOutEvent(QFocusEvent *e) override { QWidget::focusOutEvent(e); update(); }

private:
    FormulaBarEdit *m_bar;
};

FormulaBarEdit::FormulaBarEdit(QWidget *parent) : QLineEdit(parent)
{
    m_oneLine = QLineEdit::sizeHint().height();
    m_scroll = new QScrollBar(Qt::Vertical, this);
    m_scroll->setObjectName(QStringLiteral("formulaBarScroll"));
    m_scroll->setAccessibleName(QCoreApplication::translate("jp::FormulaBarEdit", "Formula bar scroll bar"));
    m_scroll->setFocusPolicy(Qt::NoFocus);
    m_scroll->hide();
    m_grip = new FormulaBarGrip(this);
    m_grip->hide();
    connect(m_scroll, &QScrollBar::valueChanged, this, [this](int v) {
        m_firstLine = v;
        update();
    });
    connect(this, &QLineEdit::textChanged, this, [this] {
        m_dirtyLayout = true;
        if (m_expanded) {
            updateScroll();
            update();
        }
    });
    connect(this, &QLineEdit::cursorPositionChanged, this, [this] {
        if (m_expanded) {
            scrollToCaret();
            update();
        }
    });
}

QWidget *FormulaBarEdit::grip() const { return m_grip; }

int FormulaBarEdit::lineHeight() const { return std::max(1, fontMetrics().lineSpacing()); }

QRect FormulaBarEdit::textRect() const
{
    const int bottom = m_grip->height();
    const int right = m_scroll->isVisibleTo(this) ? m_scroll->width() : 0;
    return QRect(5, 3, std::max(10, width() - 10 - right), std::max(lineHeight(), height() - 6 - bottom));
}

void FormulaBarEdit::setExpanded(bool on)
{
    if (on == m_expanded) return;
    m_expanded = on;
    m_dirtyLayout = true;
    m_grip->setVisible(on);
    applyHeight();
    if (on) {
        updateScroll();
        scrollToCaret();
    } else {
        m_scroll->hide();
        m_firstLine = 0;
    }
    update();
    Q_EMIT expandedChanged(on);
}

void FormulaBarEdit::setExpandedLines(int lines)
{
    lines = std::clamp(lines, int(kMinLines), int(kMaxLines));
    if (lines == m_lines) return;
    m_lines = lines;
    if (m_expanded) {
        applyHeight();
        m_dirtyLayout = true;
        updateScroll();
    }
    Q_EMIT linesChanged(lines);
}

void FormulaBarEdit::applyHeight()
{
    if (!m_expanded) {
        setMinimumHeight(0);
        setMaximumHeight(QWIDGETSIZE_MAX);
        return;
    }
    setFixedHeight(std::max(m_oneLine, m_lines * lineHeight() + 6 + m_grip->height()));
}

void FormulaBarEdit::layoutText()
{
    if (!m_dirtyLayout) return;
    m_dirtyLayout = false;
    m_laidText = text();
    m_laidText.replace(QLatin1Char('\n'), QChar(QChar::LineSeparator));
    m_layout.setText(m_laidText);
    m_layout.setFont(font());
    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    opt.setTextDirection(layoutDirection());
    m_layout.setTextOption(opt);
    auto run = [this](int width) {
        m_layout.beginLayout();
        qreal y = 0;
        for (;;) {
            QTextLine line = m_layout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(width);
            line.setPosition(QPointF(0, y));
            y += lineHeight();
        }
        m_layout.endLayout();
    };
    const int visible = std::max(1, textRect().height() / lineHeight());
    const int sb = m_scroll->sizeHint().width();
    const int full = std::max(10, width() - 10);
    run(full);
    if (m_layout.lineCount() > visible) run(std::max(10, full - sb));
}

int FormulaBarEdit::lineCount() const
{
    const_cast<FormulaBarEdit *>(this)->layoutText();
    return m_layout.lineCount();
}

int FormulaBarEdit::lineOfPosition(int position) const
{
    const_cast<FormulaBarEdit *>(this)->layoutText();
    for (int i = 0; i < m_layout.lineCount(); ++i) {
        const QTextLine l = m_layout.lineAt(i);
        if (position < l.textStart() + l.textLength() || i == m_layout.lineCount() - 1) return i;
    }
    return 0;
}

void FormulaBarEdit::updateScroll()
{
    if (!m_expanded) return;
    layoutText();
    const int visible = std::max(1, textRect().height() / lineHeight());
    const int count = m_layout.lineCount();
    const bool needed = count > visible;
    if (needed != m_scroll->isVisibleTo(this)) {
        m_scroll->setVisible(needed);
        m_dirtyLayout = true;
        layoutText();
    }
    m_scroll->setGeometry(width() - m_scroll->sizeHint().width() - 1, 1, m_scroll->sizeHint().width(), std::max(1, height() - 2 - m_grip->height()));
    m_scroll->setRange(0, std::max(0, count - visible));
    m_scroll->setPageStep(visible);
    m_firstLine = std::clamp(m_firstLine, 0, std::max(0, count - visible));
    if (m_scroll->value() != m_firstLine) m_scroll->setValue(m_firstLine);
}

void FormulaBarEdit::scrollToCaret()
{
    const int visible = std::max(1, textRect().height() / lineHeight());
    const int line = lineOfPosition(cursorPosition());
    int first = m_firstLine;
    if (line < first) first = line;
    else if (line >= first + visible) first = line - visible + 1;
    if (first != m_firstLine) {
        m_firstLine = first;
        if (m_scroll->value() != first) m_scroll->setValue(first);
    }
}

void FormulaBarEdit::resizeEvent(QResizeEvent *e)
{
    QLineEdit::resizeEvent(e);
    m_grip->setGeometry(0, height() - m_grip->height(), width(), m_grip->height());
    m_dirtyLayout = true;
    updateScroll();
}

void FormulaBarEdit::changeEvent(QEvent *e)
{
    QLineEdit::changeEvent(e);
    if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange) {
        m_dirtyLayout = true;
        if (m_expanded) applyHeight();
    }
}

void FormulaBarEdit::inputMethodEvent(QInputMethodEvent *e)
{
    // (the colors the window gives the references are kept to draw them again while the bar is several lines tall)
    const int caret = cursorPosition();
    if (e->preeditString().isEmpty()) {
        m_colors.clear();
        for (const QInputMethodEvent::Attribute &a : e->attributes()) {
            if (a.type != QInputMethodEvent::TextFormat) continue;
            QTextLayout::FormatRange r;
            r.start = a.start + caret;
            r.length = a.length;
            r.format = qvariant_cast<QTextFormat>(a.value).toCharFormat();
            if (r.start >= 0 && r.length > 0) m_colors.push_back(r);
        }
        if (m_expanded) update();
    }
    QLineEdit::inputMethodEvent(e);
}

QVariant FormulaBarEdit::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (m_expanded && query == Qt::ImCursorRectangle) {
        const_cast<FormulaBarEdit *>(this)->layoutText();
        const int line = lineOfPosition(cursorPosition());
        const QTextLine l = m_layout.lineAt(line);
        if (l.isValid()) return QRect(textRect().left() + int(l.cursorToX(cursorPosition())), textRect().top() + (line - m_firstLine) * lineHeight(), 1, lineHeight());
    }
    return QLineEdit::inputMethodQuery(query);
}

void FormulaBarEdit::paintEvent(QPaintEvent *e)
{
    if (!m_expanded) {
        QLineEdit::paintEvent(e);
        return;
    }
    layoutText();
    QPainter p(this);
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, &p, this);
    const QRect box = textRect();
    p.setClipRect(box);
    p.setPen(palette().color(isEnabled() ? QPalette::Active : QPalette::Disabled, QPalette::Text));
    const int size = int(m_laidText.size());
    QList<QTextLayout::FormatRange> colors;
    for (const QTextLayout::FormatRange &r : std::as_const(m_colors))
        if (r.start < size) colors.append({r.start, std::min(r.length, size - r.start), r.format});
    m_layout.setFormats(colors);
    QList<QTextLayout::FormatRange> selection;
    if (hasSelectedText() && selectionStart() < size) {
        QTextLayout::FormatRange r;
        r.start = selectionStart();
        r.length = std::min<int>(selectedText().size(), size - r.start);
        r.format.setBackground(palette().highlight());
        r.format.setForeground(palette().highlightedText());
        selection.append(r);
    }
    const QPointF origin(box.left(), box.top() - m_firstLine * lineHeight());
    m_layout.draw(&p, origin, selection, QRectF(box));
    if (hasFocus() && !isReadOnly()) m_layout.drawCursor(&p, origin, cursorPosition(), 1);
}

int FormulaBarEdit::positionAt(const QPoint &pt) const
{
    const_cast<FormulaBarEdit *>(this)->layoutText();
    const QRect box = textRect();
    const int line = std::clamp(m_firstLine + (pt.y() - box.top()) / lineHeight(), 0, std::max(0, m_layout.lineCount() - 1));
    const QTextLine l = m_layout.lineAt(line);
    if (!l.isValid()) return 0;
    const int at = l.xToCursor(pt.x() - box.left());
    const int end = l.textStart() + l.textLength();
    return line < m_layout.lineCount() - 1 ? std::min(at, std::max(l.textStart(), end - 1)) : std::min(at, int(m_laidText.size()));
}

int FormulaBarEdit::moveByLines(int delta) const
{
    const_cast<FormulaBarEdit *>(this)->layoutText();
    const int now = lineOfPosition(cursorPosition());
    const int to = now + delta;
    if (to < 0) return 0;
    if (to >= m_layout.lineCount()) return int(m_laidText.size());
    const qreal x = m_layout.lineAt(now).cursorToX(cursorPosition());
    const QTextLine l = m_layout.lineAt(to);
    const int end = l.textStart() + l.textLength();
    const int at = l.xToCursor(x);
    return to < m_layout.lineCount() - 1 ? std::min(at, std::max(l.textStart(), end - 1)) : at;
}

void FormulaBarEdit::keyPressEvent(QKeyEvent *e)
{
    const bool plain = !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier));
    const bool ctrlOnly = (e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & (Qt::AltModifier | Qt::MetaModifier));
    if (m_expanded && ctrlOnly && (e->key() == Qt::Key_Home || e->key() == Qt::Key_End)) {
        const int to = e->key() == Qt::Key_Home ? 0 : int(text().size());
        if (e->modifiers() & Qt::ShiftModifier) {
            const int anchor = hasSelectedText() ? (selectionStart() == cursorPosition() ? selectionEnd() : selectionStart()) : cursorPosition();
            setSelection(anchor, to - anchor);
        } else {
            setCursorPosition(to);
        }
        e->accept();
        return;
    }
    if (m_expanded && plain) {
        const int visible = std::max(1, textRect().height() / lineHeight());
        int to = -1;
        switch (e->key()) {
        case Qt::Key_Up: to = moveByLines(-1); break;
        case Qt::Key_Down: to = moveByLines(1); break;
        case Qt::Key_PageUp: to = moveByLines(-visible); break;
        case Qt::Key_PageDown: to = moveByLines(visible); break;
        case Qt::Key_Home:
        case Qt::Key_End: {
            layoutText();
            const QTextLine l = m_layout.lineAt(lineOfPosition(cursorPosition()));
            to = e->key() == Qt::Key_Home ? l.textStart() : l.textStart() + l.textLength();
            if (e->key() == Qt::Key_End && to > l.textStart() && to <= int(m_laidText.size()) && m_laidText.at(to - 1) == QChar(QChar::LineSeparator)) --to;
            break;
        }
        default: break;
        }
        if (to >= 0) {
            if (e->modifiers() & Qt::ShiftModifier) {
                const int anchor = hasSelectedText() ? (selectionStart() == cursorPosition() ? selectionEnd() : selectionStart()) : cursorPosition();
                setSelection(anchor, to - anchor);
            } else {
                setCursorPosition(to);
            }
            e->accept();
            return;
        }
    }
    QLineEdit::keyPressEvent(e);
}

void FormulaBarEdit::mousePressEvent(QMouseEvent *e)
{
    if (!m_expanded || e->button() != Qt::LeftButton) {
        QLineEdit::mousePressEvent(e);
        return;
    }
    if (!hasFocus()) setFocus(Qt::MouseFocusReason);
    const int at = positionAt(e->position().toPoint());
    if (e->modifiers() & Qt::ShiftModifier) {
        const int anchor = hasSelectedText() ? (selectionStart() == cursorPosition() ? selectionEnd() : selectionStart()) : cursorPosition();
        m_anchor = anchor;
        setSelection(anchor, at - anchor);
    } else {
        m_anchor = at;
        setCursorPosition(at);
    }
    e->accept();
}

void FormulaBarEdit::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_expanded || !(e->buttons() & Qt::LeftButton) || m_anchor < 0) {
        QLineEdit::mouseMoveEvent(e);
        return;
    }
    const int at = positionAt(e->position().toPoint());
    setSelection(m_anchor, at - m_anchor);
    e->accept();
}

void FormulaBarEdit::mouseReleaseEvent(QMouseEvent *e)
{
    if (!m_expanded) {
        QLineEdit::mouseReleaseEvent(e);
        return;
    }
    m_anchor = -1;
    e->accept();
}

void FormulaBarEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!m_expanded) {
        QLineEdit::mouseDoubleClickEvent(e);
        return;
    }
    const QString t = text();
    int at = std::clamp(positionAt(e->position().toPoint()), 0, int(t.size()));
    int a = at, b = at;
    while (a > 0 && t.at(a - 1).isLetterOrNumber()) --a;
    while (b < t.size() && t.at(b).isLetterOrNumber()) ++b;
    if (a == b && b < t.size() && t.at(b) != QLatin1Char('\n')) ++b;
    setSelection(a, b - a);
    e->accept();
}

void FormulaBarEdit::wheelEvent(QWheelEvent *e)
{
    if (!m_expanded || !m_scroll->isVisibleTo(this)) {
        QLineEdit::wheelEvent(e);
        return;
    }
    const int steps = e->angleDelta().y() / 40;
    if (steps) m_scroll->setValue(m_scroll->value() - steps);
    e->accept();
}

} // namespace jp
