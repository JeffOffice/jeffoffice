#include "app/ruler.h"

#include "app/icons.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "core/editops.h"
#include "core/pagesetup.h"

#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QPolygonF>
#include <QTextBlock>
#include <QToolTip>
#include <algorithm>
#include <cmath>

namespace jp::write {

namespace {

constexpr double kMarkerHalf = 5;      // the half width of a marker's target, in pixels
constexpr double kBandTop = 8, kBandBottom = 18;

// The step a drag snaps to, and the marks of the ruler, in points, for the unit the person measures in.
struct Scale {
    double major, minorsPerMajor, snap;
};

Scale scaleFor(Unit unit)
{
    switch (unit) {
    case Unit::Centimeter: return {72 / 2.54, 2, 72 / 25.4};
    case Unit::Millimeter: return {72 / 2.54, 2, 72 / 25.4};
    case Unit::Point: return {72, 4, 1};
    case Unit::Pica: return {72, 6, 3};
    case Unit::Inch:
    default: return {72, 8, 72 / 16.0};
    }
}

QTextBlock firstBlockOf(const QTextCursor &c) { return c.document()->findBlock(c.selectionStart()); }

} // namespace

Ruler::Ruler(PageView *view, QWidget *parent) : QWidget(parent), m_view(view)
{
    setObjectName(QStringLiteral("ruler"));
    setFixedHeight(kHeight);
    setMouseTracking(true);
    setAccessibleName(QCoreApplication::translate("jp::write::Ruler", "Ruler"));
    setAccessibleDescription(QCoreApplication::translate("jp::write::Ruler", "The margins, indents, and tab stops of the paragraph the caret is in. Drag a marker to change them."));
    connect(view, &PageView::caretChanged, this, &Ruler::sync);
    connect(view, &PageView::contentChanged, this, &Ruler::sync);
    connect(view, &PageView::zoomChanged, this, &Ruler::sync);
    connect(view, &PageView::pagesChanged, this, &Ruler::sync);
    connect(view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &Ruler::sync);
    connect(view->verticalScrollBar(), &QScrollBar::valueChanged, this, &Ruler::sync);
    // The pages stay centered, so they move when the window's width changes:
    // read them again once the view has laid them out at the new size.
    view->viewport()->installEventFilter(this);
}

bool Ruler::eventFilter(QObject *watched, QEvent *e)
{
    if (watched == m_view->viewport() && (e->type() == QEvent::Resize || e->type() == QEvent::Move || e->type() == QEvent::Show))
        QMetaObject::invokeMethod(this, &Ruler::sync, Qt::QueuedConnection);
    return QWidget::eventFilter(watched, e);
}

QTextCursor Ruler::cursor() const { return m_view->cursor(); }

Ruler::State Ruler::read() const
{
    State s;
    WriteDoc *doc = m_view->document();
    if (!doc) return s;
    const QTextCursor c = m_view->cursor();
    if (c.isNull()) return s;
    const QRect page = m_view->pageRect(m_view->currentPage());
    if (page.isNull()) return s;
    const int section = m_view->cursorSection();
    const flow::Section &sec = doc->flow().section(section);
    s.valid = true;
    s.section = section;
    s.scale = m_view->zoom() * 96.0 / 72.0;
    s.pageLeftPx = mapFromGlobal(m_view->viewport()->mapToGlobal(QPoint(0, 0))).x() + page.left();
    s.paperWidth = sec.pageSize().width();
    s.textLeft = sec.margins.left();
    s.textWidth = std::max(1.0, s.paperWidth - sec.margins.left() - sec.margins.right());
    const QTextBlockFormat bf = firstBlockOf(c).blockFormat();
    s.left = bf.leftMargin();
    s.right = bf.rightMargin();
    s.first = bf.textIndent();
    s.tabs = tabStopsOf(bf);
    if (s.tabs.size() > kMostTabStopsShown) s.tabs.resize(kMostTabStopsShown);   // (drawn, not changed: the stops are the paragraph's)
    return s;
}

void Ruler::sync()
{
    if (m_drag != Marker::None) return;   // (what is dragged is drawn from the drag; the document is not changed until it ends)
    m_state = read();
    update();
}

void Ruler::setTabKind(QTextOption::TabType kind)
{
    m_tabKind = kind;
    update();
}

double Ruler::snapped(double points, Qt::KeyboardModifiers modifiers) const
{
    if (modifiers & Qt::AltModifier) return std::round(points * 20) / 20;
    const double step = scaleFor(Settings::get().unit()).snap;
    return std::round(points / step) * step;
}

bool Ruler::markerPoint(Marker marker, int index, QPointF *point) const
{
    if (!m_state.valid) return false;
    switch (marker) {
    case Marker::FirstLine: *point = QPointF(xOf(m_state.left + m_state.first), 4.5); return true;
    case Marker::Hanging: *point = QPointF(xOf(m_state.left), 21); return true;
    case Marker::LeftIndent: *point = QPointF(xOf(m_state.left), 27); return true;
    case Marker::RightIndent: *point = QPointF(xOf(m_state.textWidth - m_state.right), 21); return true;
    case Marker::LeftMargin: *point = QPointF(xOf(0), 13); return true;
    case Marker::RightMargin: *point = QPointF(xOf(m_state.textWidth), 13); return true;
    case Marker::TabStop:
        if (index < 0 || index >= m_state.tabs.size()) return false;
        *point = QPointF(xOf(m_state.tabs[index].position), 13);
        return true;
    case Marker::TabKind: *point = QPointF(12, kHeight / 2.0); return true;
    default: return false;
    }
}

Ruler::Marker Ruler::markerAt(const QPointF &p, int *index) const
{
    if (!m_state.valid) return Marker::None;
    if (p.x() < 24) return Marker::TabKind;
    auto near = [&](Marker m, int i, double halfWidth, double top, double bottom) {
        QPointF at;
        return markerPoint(m, i, &at) && std::abs(p.x() - at.x()) <= halfWidth && p.y() >= top && p.y() <= bottom;
    };
    if (near(Marker::FirstLine, 0, kMarkerHalf, 0, kBandTop)) return Marker::FirstLine;
    if (near(Marker::LeftIndent, 0, kMarkerHalf, 25, kHeight)) return Marker::LeftIndent;
    if (near(Marker::Hanging, 0, kMarkerHalf, kBandBottom, 25)) return Marker::Hanging;
    if (near(Marker::RightIndent, 0, kMarkerHalf, kBandBottom, kHeight)) return Marker::RightIndent;
    for (int i = int(m_state.tabs.size()) - 1; i >= 0; --i)
        if (near(Marker::TabStop, i, 4, kBandTop + 1, kBandBottom)) {
            if (index) *index = i;
            return Marker::TabStop;
        }
    if (near(Marker::LeftMargin, 0, 3, kBandTop, kBandBottom)) return Marker::LeftMargin;
    if (near(Marker::RightMargin, 0, 3, kBandTop, kBandBottom)) return Marker::RightMargin;
    if (p.x() > xOf(0) && p.x() < xOf(m_state.textWidth) && p.y() >= kBandTop && p.y() <= kBandBottom) return Marker::Blank;
    return Marker::None;
}

void Ruler::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return;
    m_state = read();
    int index = -1;
    const Marker m = markerAt(e->position(), &index);
    if (m == Marker::None) return;
    if (m == Marker::TabKind) {
        // Left, center, right, and decimal, in turn.
        setTabKind(m_tabKind == QTextOption::LeftTab ? QTextOption::CenterTab : m_tabKind == QTextOption::CenterTab ? QTextOption::RightTab
                                                                              : m_tabKind == QTextOption::RightTab ? QTextOption::DelimiterTab : QTextOption::LeftTab);
        return;
    }
    m_drag = m == Marker::Blank ? Marker::TabStop : m;
    m_dragIndex = m == Marker::Blank ? -1 : index;
    m_dragMoved = false;
    m_dragOut = false;
    const double at = pointsAt(e->position().x());
    switch (m_drag) {
    case Marker::FirstLine: m_dragFrom = m_state.left + m_state.first; break;
    case Marker::Hanging:
    case Marker::LeftIndent: m_dragFrom = m_state.left; break;
    case Marker::RightIndent: m_dragFrom = m_state.textWidth - m_state.right; break;
    case Marker::LeftMargin: m_dragFrom = 0; break;
    case Marker::RightMargin: m_dragFrom = m_state.textWidth; break;
    case Marker::TabStop: m_dragFrom = m_dragIndex >= 0 ? m_state.tabs[m_dragIndex].position : snapped(at, e->modifiers()); break;
    default: break;
    }
    m_dragValue = m_dragFrom;
    if (m == Marker::Blank) m_dragMoved = true;   // (a new stop is made where the press was)
    update();
}

void Ruler::mouseMoveEvent(QMouseEvent *e)
{
    if (m_drag == Marker::None) {
        // The shape of the pointer says what a press would do.
        const Marker m = markerAt(e->position());
        setCursor(m == Marker::None ? Qt::ArrowCursor : m == Marker::LeftMargin || m == Marker::RightMargin ? Qt::SizeHorCursor : Qt::PointingHandCursor);
        return;
    }
    double v = snapped(pointsAt(e->position().x()), e->modifiers());
    // The margins are dragged within the page, leaving some text; a tab stop within the farthest a stop may be; the indents within the text area.
    double least = 0, most = m_state.textWidth;
    if (m_drag == Marker::LeftMargin) {
        least = -m_state.textLeft;
        most = m_state.textWidth - kLeastText;
    } else if (m_drag == Marker::RightMargin) {
        least = kLeastText;
        most = m_state.paperWidth - m_state.textLeft;
    } else if (m_drag == Marker::TabStop) {
        most = kMostTabPosition;
    }
    v = std::clamp(v, least, std::max(least, most));
    if (std::abs(v - m_dragValue) > 0.001) m_dragMoved = true;
    m_dragValue = v;
    // A tab stop pulled well off the ruler is a stop to take away.
    m_dragOut = m_drag == Marker::TabStop && m_dragIndex >= 0 && (e->position().y() < -12 || e->position().y() > kHeight + 24);
    update();
}

void Ruler::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if (m_drag == Marker::None) return;
    if (m_dragMoved || m_dragOut) commit();
    m_drag = Marker::None;
    m_dragIndex = -1;
    m_dragOut = false;
    m_state = read();
    update();
}

void Ruler::commit()
{
    WriteDoc *doc = m_view->document();
    if (!doc) return;
    QTextCursor c = cursor();
    const double value = m_dragValue;
    switch (m_drag) {
    case Marker::FirstLine: {
        const double d = value - (m_state.left + m_state.first);
        changeBlockFormat(*doc, c, [d](QTextBlockFormat &bf) { bf.setTextIndent(bf.textIndent() + d); });
        break;
    }
    case Marker::Hanging: {
        // The left indent moves and the first line stays where it is.
        const double d = value - m_state.left;
        changeBlockFormat(*doc, c, [d](QTextBlockFormat &bf) {
            const double left = std::max(0.0, bf.leftMargin() + d);
            bf.setTextIndent(bf.textIndent() - (left - bf.leftMargin()));
            bf.setLeftMargin(left);
        });
        break;
    }
    case Marker::LeftIndent: {
        const double d = value - m_state.left;
        changeBlockFormat(*doc, c, [d](QTextBlockFormat &bf) { bf.setLeftMargin(std::max(0.0, bf.leftMargin() + d)); });
        break;
    }
    case Marker::RightIndent: {
        const double d = (m_state.textWidth - value) - m_state.right;
        changeBlockFormat(*doc, c, [d](QTextBlockFormat &bf) { bf.setRightMargin(std::max(0.0, bf.rightMargin() + d)); });
        break;
    }
    case Marker::LeftMargin:
    case Marker::RightMargin: {
        const SectionSetup before = pageSetupOf(doc->sectionProps(m_state.section), doc->differentOddEven());
        SectionSetup after = before;
        if (m_drag == Marker::LeftMargin) after.left = std::max(0.0, m_state.textLeft + value - before.gutter);
        else after.right = std::max(0.0, m_state.paperWidth - (m_state.textLeft + value));
        applyPageSetup(*doc, m_state.section, PageSetupScope::ThisSection, before, after);
        break;
    }
    case Marker::TabStop:
        if (m_dragIndex < 0) {
            TabStopEntry stop;
            stop.position = value;
            stop.type = m_tabKind;
            addTabStop(*doc, c, stop);
        } else if (m_dragOut) {
            removeTabStop(*doc, c, m_dragFrom);
        } else {
            moveTabStop(*doc, c, m_dragFrom, value);
        }
        break;
    default: break;
    }
    m_view->refresh();
}

bool Ruler::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        const auto *h = static_cast<QHelpEvent *>(e);
        int index = -1;
        const Marker m = markerAt(h->pos(), &index);
        const char *text = nullptr;
        switch (m) {
        case Marker::FirstLine: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "First line indent"); break;
        case Marker::Hanging: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Hanging indent"); break;
        case Marker::LeftIndent: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Left indent"); break;
        case Marker::RightIndent: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Right indent"); break;
        case Marker::LeftMargin: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Left margin"); break;
        case Marker::RightMargin: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Right margin"); break;
        case Marker::TabStop: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Tab stop"); break;
        case Marker::TabKind: text = QT_TRANSLATE_NOOP("jp::write::Ruler", "Kind of tab stop (click to change)"); break;
        default: break;
        }
        if (text) QToolTip::showText(h->globalPos(), QCoreApplication::translate("jp::write::Ruler", text), this);
        else QToolTip::hideText();
        return true;
    }
    return QWidget::event(e);
}

void Ruler::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const bool dark = uiDark();
    const QColor ground = dark ? QColor(0x30, 0x30, 0x30) : QColor(0xE4, 0xE6, 0xE9);
    const QColor margin = dark ? QColor(0x45, 0x45, 0x45) : QColor(0xC4, 0xC8, 0xCE);
    const QColor paper = dark ? QColor(0x1E, 0x1E, 0x1E) : QColor(Qt::white);
    const QColor ink = uiText();
    p.fillRect(rect(), ground);
    if (!m_state.valid) return;
    p.setRenderHint(QPainter::Antialiasing, false);
    // The values as dragged.
    State s = m_state;
    if (m_drag == Marker::FirstLine) s.first = m_dragValue - s.left;
    else if (m_drag == Marker::Hanging) {
        s.first -= m_dragValue - s.left;
        s.left = m_dragValue;
    } else if (m_drag == Marker::LeftIndent) s.left = m_dragValue;
    else if (m_drag == Marker::RightIndent) s.right = s.textWidth - m_dragValue;
    else if (m_drag == Marker::LeftMargin) {
        s.textLeft += m_dragValue;
        s.textWidth -= m_dragValue;
    } else if (m_drag == Marker::RightMargin) s.textWidth = m_dragValue;
    auto x = [&](double pt) { return s.pageLeftPx + (s.textLeft + pt) * s.scale; };
    // The page's strip: margins gray, the text area white.
    const double pageLeft = s.pageLeftPx, pageRight = s.pageLeftPx + s.paperWidth * s.scale;
    p.fillRect(QRectF(pageLeft, kBandTop, pageRight - pageLeft, kBandBottom - kBandTop), margin);
    p.fillRect(QRectF(x(0), kBandTop, x(s.textWidth) - x(0), kBandBottom - kBandTop), paper);
    // The marks, from the left edge of the text, both ways.
    const Scale sc = scaleFor(Settings::get().unit());
    const double step = sc.major / sc.minorsPerMajor;
    p.setPen(ink);
    QFont f = font();
    f.setPixelSize(9);
    p.setFont(f);
    const int from = int(std::floor(-s.textLeft / step)) - 1, to = int(std::ceil((s.paperWidth - s.textLeft) / step)) + 1;
    const int count = std::min(to - from, 4000);
    for (int i = from; i < from + count; ++i) {
        const double pt = i * step;
        const double px = std::round(x(pt)) + 0.5;
        if (px < 0 || px > width()) continue;
        if (px < pageLeft - 1 || px > pageRight + 1) continue;
        const int k = ((i % int(sc.minorsPerMajor)) + int(sc.minorsPerMajor)) % int(sc.minorsPerMajor);
        if (k == 0) {
            if (i != 0 && s.scale * step > 3) {
                const QString label = QString::number(std::abs(i) / int(sc.minorsPerMajor));
                p.drawText(QRectF(px - 12, kBandTop, 24, kBandBottom - kBandTop), Qt::AlignCenter, label);
            }
        } else if (s.scale * step > 3.5) {
            const bool mid = int(sc.minorsPerMajor) % 2 == 0 && k == int(sc.minorsPerMajor) / 2;
            const double h = mid ? 4 : 2;
            p.drawLine(QPointF(px, (kBandTop + kBandBottom) / 2 - h / 2), QPointF(px, (kBandTop + kBandBottom) / 2 + h / 2));
        }
    }
    // The tab stops.
    p.setRenderHint(QPainter::Antialiasing, true);
    const QColor mark = dark ? QColor(0xD8, 0xDC, 0xE2) : QColor(0x33, 0x3A, 0x45);
    p.setPen(QPen(mark, 1.4));
    auto drawTab = [&](double px, QTextOption::TabType type, double top) {
        const double base = top + 8;
        switch (type) {
        case QTextOption::RightTab: p.drawLine(QPointF(px, top), QPointF(px, base)); p.drawLine(QPointF(px, base), QPointF(px - 5, base)); break;
        case QTextOption::CenterTab: p.drawLine(QPointF(px, top), QPointF(px, base)); p.drawLine(QPointF(px - 4, base), QPointF(px + 4, base)); break;
        case QTextOption::DelimiterTab:
            p.drawLine(QPointF(px, top), QPointF(px, base));
            p.drawLine(QPointF(px - 4, base), QPointF(px + 4, base));
            p.drawPoint(QPointF(px + 3, base - 3));
            break;
        default: p.drawLine(QPointF(px, top), QPointF(px, base)); p.drawLine(QPointF(px, base), QPointF(px + 5, base)); break;
        }
    };
    for (int i = 0; i < s.tabs.size(); ++i) {
        if (m_drag == Marker::TabStop && i == m_dragIndex) continue;
        drawTab(std::round(x(s.tabs[i].position)) + 0.5, s.tabs[i].type, 9.5);
    }
    if (m_drag == Marker::TabStop && !m_dragOut) {
        const QTextOption::TabType type = m_dragIndex >= 0 ? s.tabs[m_dragIndex].type : m_tabKind;
        drawTab(std::round(x(m_dragValue)) + 0.5, type, 9.5);
    }
    // The indents: the first line's marker points down from above, the hanging indent's up from below with the left indent's box under it, and the right indent's up.
    p.setPen(QPen(mark, 1));
    p.setBrush(dark ? QColor(0x9A, 0xA4, 0xB2) : QColor(0xF4, 0xF5, 0xF7));
    const double fx = x(s.left + s.first), lx = x(s.left), rx = x(s.textWidth - s.right);
    p.drawPolygon(QPolygonF({QPointF(fx - 4.5, 1.5), QPointF(fx + 4.5, 1.5), QPointF(fx + 4.5, 4.5), QPointF(fx, 8.5), QPointF(fx - 4.5, 4.5)}));
    p.drawPolygon(QPolygonF({QPointF(lx, 18.5), QPointF(lx + 4.5, 22), QPointF(lx + 4.5, 24.5), QPointF(lx - 4.5, 24.5), QPointF(lx - 4.5, 22)}));
    p.drawRect(QRectF(lx - 4.5, 25.5, 9, 3.5));
    p.drawPolygon(QPolygonF({QPointF(rx, 18.5), QPointF(rx + 4.5, 22), QPointF(rx + 4.5, 24.5), QPointF(rx - 4.5, 24.5), QPointF(rx - 4.5, 22)}));
    // The box at the left end: the kind of tab stop a click adds.
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(margin, 1));
    p.drawRect(QRectF(3.5, 3.5, 17, kHeight - 7));
    p.setPen(QPen(mark, 1.4));
    drawTab(12, m_tabKind, 9);
}

VerticalRuler::VerticalRuler(PageView *view, QWidget *parent) : QWidget(parent), m_view(view)
{
    setObjectName(QStringLiteral("verticalRuler"));
    setFixedWidth(kWidth);
    setAccessibleName(QCoreApplication::translate("jp::write::Ruler", "Vertical ruler"));
    setAccessibleDescription(QCoreApplication::translate("jp::write::Ruler", "The top and bottom margins of the pages on show."));
    connect(view, &PageView::contentChanged, this, qOverload<>(&QWidget::update));
    connect(view, &PageView::zoomChanged, this, qOverload<>(&QWidget::update));
    connect(view, &PageView::pagesChanged, this, qOverload<>(&QWidget::update));
    connect(view->verticalScrollBar(), &QScrollBar::valueChanged, this, qOverload<>(&QWidget::update));
}

bool VerticalRuler::textAreaOf(int page, double *top, double *bottom) const
{
    WriteDoc *doc = m_view->document();
    if (!doc || page < 0 || page >= m_view->pageCount()) return false;
    const QRect r = m_view->pageRect(page);
    if (r.isNull()) return false;
    const flow::Section &sec = doc->flow().section(m_view->cursorSection());
    const double scale = m_view->zoom() * 96.0 / 72.0;
    const double offset = mapFromGlobal(m_view->viewport()->mapToGlobal(QPoint(0, 0))).y();
    if (top) *top = offset + r.top() + sec.margins.top() * scale;
    if (bottom) *bottom = offset + r.bottom() - sec.margins.bottom() * scale;
    return true;
}

void VerticalRuler::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    const bool dark = uiDark();
    const QColor ground = dark ? QColor(0x30, 0x30, 0x30) : QColor(0xE4, 0xE6, 0xE9);
    const QColor margin = dark ? QColor(0x45, 0x45, 0x45) : QColor(0xC4, 0xC8, 0xCE);
    const QColor paper = dark ? QColor(0x1E, 0x1E, 0x1E) : QColor(Qt::white);
    const QColor ink = uiText();
    p.fillRect(rect(), ground);
    WriteDoc *doc = m_view->document();
    if (!doc) return;
    p.setRenderHint(QPainter::Antialiasing, false);
    const double scale = m_view->zoom() * 96.0 / 72.0;
    const double offset = mapFromGlobal(m_view->viewport()->mapToGlobal(QPoint(0, 0))).y();
    const Scale sc = scaleFor(Settings::get().unit());
    const double step = sc.major / sc.minorsPerMajor;
    QFont f = font();
    f.setPixelSize(9);
    p.setFont(f);
    const int bandLeft = 4, bandRight = kWidth - 4;
    const int pages = std::min(m_view->pageCount(), 100000);
    for (int page = 0; page < pages; ++page) {
        const QRect r = m_view->pageRect(page);
        if (r.isNull()) continue;
        const double pageTop = offset + r.top(), pageBottom = offset + r.bottom();
        if (pageBottom < 0) continue;
        if (pageTop > height()) break;
        double textTop = 0, textBottom = 0;
        if (!textAreaOf(page, &textTop, &textBottom)) continue;
        p.fillRect(QRectF(bandLeft, pageTop, bandRight - bandLeft, pageBottom - pageTop), margin);
        p.fillRect(QRectF(bandLeft, textTop, bandRight - bandLeft, textBottom - textTop), paper);
        // The marks, from the top of the text area, both ways.
        p.setPen(ink);
        const int from = int(std::floor((pageTop - textTop) / scale / step)) - 1, to = int(std::ceil((pageBottom - textTop) / scale / step)) + 1;
        const int count = std::min(to - from, 4000);
        for (int i = from; i < from + count; ++i) {
            const double y = std::round(textTop + i * step * scale) + 0.5;
            if (y < pageTop - 1 || y > pageBottom + 1 || y < 0 || y > height()) continue;
            const int k = ((i % int(sc.minorsPerMajor)) + int(sc.minorsPerMajor)) % int(sc.minorsPerMajor);
            if (k == 0) {
                if (i != 0 && step * scale > 3) {
                    p.save();
                    p.translate(kWidth / 2.0, y);
                    p.rotate(-90);
                    p.drawText(QRectF(-12, -kWidth / 2.0, 24, kWidth), Qt::AlignCenter, QString::number(std::abs(i) / int(sc.minorsPerMajor)));
                    p.restore();
                }
            } else if (step * scale > 3.5) {
                const bool mid = int(sc.minorsPerMajor) % 2 == 0 && k == int(sc.minorsPerMajor) / 2;
                const double h = mid ? 4 : 2;
                p.drawLine(QPointF(kWidth / 2.0 - h / 2, y), QPointF(kWidth / 2.0 + h / 2, y));
            }
        }
    }
}

} // namespace jp::write
