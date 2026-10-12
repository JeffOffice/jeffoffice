#include "app/slidesorter.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/sectionui.h"
#include "app/theme.h"
#include "core/deck.h"
#include "core/sections.h"
#include "core/transition.h"
#include "render/renderer.h"
#include "text/storyio.h"
#include "text/textprops.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QJsonArray>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {
constexpr int kBaseWidth = 180;   // a thumbnail's width at 100%, in pixels
constexpr double kMinZoom = 0.4, kMaxZoom = 2.5;
constexpr int kMargin = 20;       // around the grid
constexpr int kGap = 14;          // between slides
constexpr int kPad = 6;           // around a thumbnail, inside its slide's cell
constexpr double kPi = 3.14159265358979323846;

QByteArray compact(const QJsonObject &o) { return QJsonDocument(o).toJson(QJsonDocument::Compact); }

// The stories of an object: a text box's or shape's, and a table's cells'.
QStringList storiesOf(const Item &it)
{
    QStringList ids;
    if (it.type() == ItemType::Text) ids << static_cast<const TextItem &>(it).storyId;
    else if (it.type() == ItemType::Shape) ids << static_cast<const ShapeItem &>(it).storyId;
    else if (it.type() == ItemType::Table)
        for (const TableCell &c : static_cast<const TableItem &>(it).cells) ids << c.storyId << c.storyId2;
    return ids;
}

// Adds what a page looks like to the hash: its background, its objects, and
// their text. `fields` is set when some text holds a field (a slide number
// reads where the slide is).
void addPage(QCryptographicHash &h, const Document &d, const PageBase &page, bool *fields)
{
    h.addData(compact(page.background.toJson()));
    h.addData(page.showMasterItems ? QByteArrayView("m1") : QByteArrayView("m0"));
    walkItems(page.items, [&](const ItemPtr &it) {
        h.addData(compact(it->toJson()));
        for (const QString &id : storiesOf(*it)) {
            const QTextDocument *story = id.isEmpty() ? nullptr : d.storyDoc(id);
            if (!story) continue;
            h.addData(compact(storyToJson(story)));
            if (*fields) continue;
            for (QTextBlock b = story->begin(); b.isValid() && !*fields; b = b.next())
                for (auto f = b.begin(); !f.atEnd(); ++f)
                    if (f.fragment().charFormat().hasProperty(tp::Field)) *fields = true;
        }
    });
}

QColor starColor() { return uiDark() ? QColor(0xF0, 0x8A, 0x4B) : QColor(0xB6, 0x42, 0x0F); }

void drawStar(QPainter *p, const QPointF &center, double radius)
{
    QPolygonF star;
    for (int k = 0; k < 10; ++k) {
        const double angle = -kPi / 2 + k * kPi / 5, r = k % 2 ? radius * 0.45 : radius;
        star << QPointF(center.x() + r * std::cos(angle), center.y() + r * std::sin(angle));
    }
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(starColor());
    p->drawPolygon(star);
    p->restore();
}
}

SlideSorter::SlideSorter(Editor *ed, QWidget *parent) : QAbstractScrollArea(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("slideSorter"));
    setAccessibleName(tr("Slide Sorter"));
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->setAutoFillBackground(false);
    verticalScrollBar()->setSingleStep(40);
    m_syncTimer.setSingleShot(true);
    m_syncTimer.setInterval(60);
    connect(&m_syncTimer, &QTimer::timeout, this, &SlideSorter::sync);
    m_renderTimer.setSingleShot(true);
    m_renderTimer.setInterval(0);
    connect(&m_renderTimer, &QTimer::timeout, this, &SlideSorter::renderNext);
    m_scrollTimer.setInterval(40);
    connect(&m_scrollTimer, &QTimer::timeout, this, &SlideSorter::autoScroll);
    connect(m_ed, &Editor::changed, this, &SlideSorter::documentChanged);
    connect(m_ed, &Editor::pageChanged, this, &SlideSorter::currentChanged);
    connect(m_ed, &Editor::documentReplaced, this, [this] {
        m_sel.clear();
        documentChanged();
        currentChanged();
    });
    connect(UiTheme::instance(), &UiTheme::changed, viewport(), qOverload<>(&QWidget::update));
    m_sel = {std::clamp(m_ed->currentPage(), 0, std::max(0, slideCount() - 1))};
    m_anchor = m_sel.first();
}

int SlideSorter::slideCount() const { return m_ed->doc() ? int(m_ed->doc()->pages.size()) : 0; }

QSize SlideSorter::thumbSize() const
{
    const QSizeF page = m_ed->doc() ? m_ed->doc()->pageSize() : QSizeF(deck::kSlideWidth, deck::kSlideHeight);
    const int w = std::max(1, int(std::lround(kBaseWidth * m_zoom)));
    return QSize(w, std::max(1, int(std::lround(w * page.height() / std::max(1.0, page.width())))));
}

QSize SlideSorter::cellSize() const
{
    const QSize t = thumbSize();
    return QSize(t.width() + 2 * kPad, kPad + t.height() + QFontMetrics(font()).height() + 8);
}

int SlideSorter::columns() const
{
    return std::max(1, (viewport()->width() - 2 * kMargin + kGap) / (cellSize().width() + kGap));
}

// Lays the grid out again if what it comes from changed: the slides, the sections, the folded ones, the width, and the size of a cell. A
// section starts on a row of its own under its heading; without sections the slides fill the rows in order.
void SlideSorter::ensureLayout() const
{
    const Document *d = m_ed->doc();
    const int n = slideCount(), cols = columns();
    const QSize cs = cellSize();
    const QJsonArray key = d ? d->appData.value(QStringLiteral("sections")).toArray() : QJsonArray();
    if (m_laidOut && n == m_laidCount && cols == m_laidColumns && viewport()->width() == m_laidWidth && cs == m_laidCell && m_collapsedSerial == m_laidSerial && key == m_laidSections) return;
    m_laidOut = true;
    m_laidCount = n;
    m_laidColumns = cols;
    m_laidWidth = viewport()->width();
    m_laidCell = cs;
    m_laidSerial = m_collapsedSerial;
    m_laidSections = key;
    m_origins.fill(QPoint(-1000000, -1000000), n);
    m_headerRects.clear();
    m_sectionStart.clear();
    m_sectionCount.clear();
    const QVector<deck::Section> sections = d ? deck::sectionsOf(*d) : QVector<deck::Section>();
    if (sections.isEmpty()) {
        for (int i = 0; i < n; ++i) m_origins[i] = QPoint(kMargin + (i % cols) * (cs.width() + kGap), kMargin + (i / cols) * (cs.height() + kGap));
        const int rows = (n + cols - 1) / cols;
        m_contentHeight = 2 * kMargin + rows * cs.height() + std::max(0, rows - 1) * kGap;
        return;
    }
    const int width = std::max(0, viewport()->width() - 2 * kMargin);
    int y = kMargin, slide = 0;
    for (const deck::Section &section : sections) {
        m_headerRects << QRect(kMargin, y, width, kSectionHeaderHeight);
        m_sectionStart << slide;
        m_sectionCount << section.count;
        y += kSectionHeaderHeight + kGap / 2;
        if (m_collapsed.contains(section.id)) {
            y += kGap / 2;
        } else {
            for (int k = 0; k < section.count; ++k) m_origins[slide + k] = QPoint(kMargin + (k % cols) * (cs.width() + kGap), y + (k / cols) * (cs.height() + kGap));
            y += ((section.count + cols - 1) / cols) * (cs.height() + kGap);
        }
        slide += section.count;
    }
    m_contentHeight = y - kGap + kMargin;
}

bool SlideSorter::slideShown(int slide) const
{
    ensureLayout();
    return slide >= 0 && slide < m_origins.size() && m_origins[slide].x() >= 0;
}

QPoint SlideSorter::cellOrigin(int slide) const
{
    ensureLayout();
    return m_origins.value(slide) - QPoint(0, verticalScrollBar()->value());
}

QRect SlideSorter::thumbnailRect(int slide) const
{
    if (slide < 0 || slide >= slideCount() || !slideShown(slide)) return QRect();
    return QRect(cellOrigin(slide) + QPoint(kPad, kPad), thumbSize());
}

int SlideSorter::slideAt(const QPoint &pos) const
{
    ensureLayout();
    const QSize cs = cellSize();
    for (int i = 0; i < m_origins.size(); ++i)
        if (m_origins[i].x() >= 0 && QRect(cellOrigin(i), cs).contains(pos)) return i;
    return -1;
}

QRect SlideSorter::sectionHeaderRect(int section) const
{
    ensureLayout();
    return section >= 0 && section < m_headerRects.size() ? m_headerRects[section].translated(0, -verticalScrollBar()->value()) : QRect();
}

int SlideSorter::headerAt(const QPoint &pos) const
{
    ensureLayout();
    for (int s = 0; s < m_headerRects.size(); ++s)
        if (sectionHeaderRect(s).contains(pos)) return s;
    return -1;
}

// The slides whose cells are in view, from the first to the last (-1 for none).
void SlideSorter::visibleRange(int *first, int *last) const
{
    ensureLayout();
    *first = *last = -1;
    const int top = verticalScrollBar()->value(), bottom = top + viewport()->height(), height = cellSize().height();
    for (int i = 0; i < m_origins.size(); ++i) {
        if (m_origins[i].x() < 0 || m_origins[i].y() + height < top || m_origins[i].y() > bottom) continue;
        if (*first < 0) *first = i;
        *last = i;
    }
}

// The slide `rows` rows down (up when negative) that stands nearest across, or `slide` when there is none.
int SlideSorter::neighbor(int slide, int rows) const
{
    if (!slideShown(slide)) return slide;
    int at = slide;
    for (int step = 0; step < std::abs(rows); ++step) {
        const QPoint o = m_origins[at];
        int best = -1, bestDy = 0, bestDx = 0;
        for (int i = 0; i < m_origins.size(); ++i) {
            const QPoint p = m_origins[i];
            if (p.x() < 0 || (rows < 0 ? p.y() >= o.y() : p.y() <= o.y())) continue;
            const int dy = std::abs(p.y() - o.y()), dx = std::abs(p.x() - o.x());
            if (best < 0 || dy < bestDy || (dy == bestDy && dx < bestDx)) best = i, bestDy = dy, bestDx = dx;
        }
        if (best < 0) break;
        at = best;
    }
    return at;
}

// The next slide that is shown from `slide` in the direction of `step`, or -1.
int SlideSorter::nextShown(int slide, int step) const
{
    for (int i = slide; i >= 0 && i < slideCount(); i += step)
        if (slideShown(i)) return i;
    return -1;
}

void SlideSorter::layoutChanged()
{
    m_laidOut = false;
    ensureLayout();
    verticalScrollBar()->setRange(0, std::max(0, m_contentHeight - viewport()->height()));
    verticalScrollBar()->setPageStep(viewport()->height());
    viewport()->update();
    scheduleRender();
}

// The keys the sorter takes for itself, whatever commands have them as shortcuts.
bool SlideSorter::event(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        const auto *k = static_cast<QKeyEvent *>(e);
        const bool ctrl = k->modifiers() & Qt::ControlModifier;
        switch (k->key()) {
        case Qt::Key_A:
        case Qt::Key_D:
            if (ctrl) e->accept();
            break;
        case Qt::Key_Escape:
            if (m_dragging) e->accept();
            break;
        case Qt::Key_Delete: case Qt::Key_Backspace: case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Left: case Qt::Key_Right: case Qt::Key_Up:
        case Qt::Key_Down: case Qt::Key_Home: case Qt::Key_End: case Qt::Key_PageUp: case Qt::Key_PageDown:
            e->accept();
            break;
        default: break;
        }
        if (e->isAccepted()) return true;
    }
    return QAbstractScrollArea::event(e);
}

void SlideSorter::scrollContentsBy(int, int)
{
    viewport()->update();
    scheduleRender();
}

void SlideSorter::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    layoutChanged();
}

void SlideSorter::showEvent(QShowEvent *e)
{
    QAbstractScrollArea::showEvent(e);
    if (m_dirty) sync();
    currentChanged();
    layoutChanged();
}

// ---- the look of each slide, and its thumbnail ----

QList<QByteArray> SlideSorter::slideKeys() const
{
    QList<QByteArray> keys;
    const Document *d = m_ed->doc();
    if (!d) return keys;
    QCryptographicHash global(QCryptographicHash::Md5);
    global.addData(QByteArray::number(d->pageSize().width()) + 'x' + QByteArray::number(d->pageSize().height()));
    global.addData(compact(d->appData));
    for (int k = 0; k < SlotCount; ++k) global.addData(QByteArray::number(d->colors.c[k].rgba()));
    global.addData(d->fonts.heading.toUtf8() + '|' + d->fonts.body.toUtf8());
    const QByteArray context = global.result().toHex() + '@' + QByteArray::number(thumbSize().width()) + 'x' + QByteArray::number(devicePixelRatioF());
    // The masters a slide is drawn over, once each.
    QHash<QString, QPair<QByteArray, bool>> masters;
    for (int i = 0; i < d->pages.size(); ++i) {
        const Page &page = *d->pages[i];
        bool fields = false;
        QCryptographicHash h(QCryptographicHash::Md5);
        h.addData(page.masterId.toUtf8());
        addPage(h, *d, page, &fields);
        if (const MasterPage *m = d->masterFor(page)) {
            auto found = masters.constFind(m->id);
            if (found == masters.constEnd()) {
                QCryptographicHash mh(QCryptographicHash::Md5);
                bool mfields = false;
                for (const MasterPage *base : d->masterChain(m)) addPage(mh, *d, *base, &mfields);
                found = masters.insert(m->id, qMakePair(mh.result(), mfields));
            }
            h.addData(found->first);
            fields |= found->second;
        }
        if (fields) h.addData(QByteArray::number(i));
        keys << h.result().toHex() + context;
    }
    return keys;
}

QByteArray SlideSorter::keyOf(int slide) const { return slide >= 0 && slide < m_keys.size() ? m_keys[slide] : QByteArray(); }

// What a slide looks like, apart from the size its thumbnail is drawn at.
QByteArray SlideSorter::lookOf(const QByteArray &key) { return key.left(std::max<qsizetype>(0, key.indexOf('@'))); }

int SlideSorter::renderedCount() const
{
    int n = 0;
    for (const QByteArray &k : m_keys) n += m_images.contains(k);
    return n;
}

void SlideSorter::documentChanged()
{
    m_dirty = true;
    if (!isVisible()) return;
    // Slides added, removed, or reordered change which thumbnail is whose, so that is seen at once.
    if (pageIds() != m_ids) sync();
    else m_syncTimer.start();
}

QStringList SlideSorter::pageIds() const
{
    QStringList ids;
    if (const Document *d = m_ed->doc())
        for (const auto &p : d->pages) ids << p->id;
    return ids;
}

// The slides changed: their looks again, the selection kept to what's there,
// and thumbnails drawn for the new looks.
void SlideSorter::sync()
{
    m_syncTimer.stop();
    if (!isVisible()) return;
    m_dirty = false;
    m_keys = slideKeys();
    m_ids = pageIds();
    const int n = slideCount();
    m_sel.erase(std::remove_if(m_sel.begin(), m_sel.end(), [n](int i) { return i >= n; }), m_sel.end());
    if (m_sel.isEmpty() && n) m_sel = {std::clamp(m_ed->currentPage(), 0, n - 1)};
    m_anchor = std::clamp(m_anchor, 0, std::max(0, n - 1));
    layoutChanged();
    updateAccessibleText();
}

void SlideSorter::scheduleRender()
{
    if (isVisible()) m_renderTimer.start();
}

// Draws one more thumbnail: the first slide in view without one, else the
// nearest one outside it (up to a screenful or two each way), then again.
void SlideSorter::renderNext()
{
    const Document *d = m_ed->doc();
    const int n = slideCount();
    if (!d || !isVisible() || m_keys.size() != n || n == 0) return;
    int first, last;
    visibleRange(&first, &last);
    if (first < 0) first = last = std::clamp(m_ed->currentPage(), 0, n - 1);
    const int reach = std::max(30, 2 * (last - first + 1));
    auto missing = [&](int i) { return i >= 0 && i < n && slideShown(i) && !m_images.contains(m_keys[i]); };
    int pick = -1;
    for (int i = first; i <= last && pick < 0; ++i)
        if (missing(i)) pick = i;
    for (int k = 1; k <= reach && pick < 0; ++k) {
        if (missing(last + k)) pick = last + k;
        else if (missing(first - k)) pick = first - k;
    }
    if (pick < 0) return;
    PaintContext ctx;
    ctx.doc = d;
    ctx.cache = &m_ed->cache();
    const double dpr = devicePixelRatioF();
    QImage img = Renderer::renderToImage(ctx, pick, thumbSize().width() * dpr / std::max(1.0, d->pageSize().width()));
    img.setDevicePixelRatio(dpr);
    m_images.insert(m_keys[pick], img);
    m_latest.insert(lookOf(m_keys[pick]), img);
    prune(first - 2 * reach, last + 2 * reach);
    viewport()->update(thumbnailRect(pick).adjusted(-kPad, -kPad, kPad, kPad));
    m_renderTimer.start();
}

// Lets go of the thumbnails of slides that changed since, and of those far
// from view when there are many.
void SlideSorter::prune(int first, int last)
{
    QSet<QByteArray> keep, looks;
    for (int i = 0; i < m_keys.size(); ++i)
        if (i >= first && i <= last) {
            keep.insert(m_keys[i]);
            looks.insert(lookOf(m_keys[i]));
        }
    for (auto it = m_images.begin(); it != m_images.end();) it = keep.contains(it.key()) ? std::next(it) : m_images.erase(it);
    for (auto it = m_latest.begin(); it != m_latest.end();) it = looks.contains(it.key()) ? std::next(it) : m_latest.erase(it);
}

void SlideSorter::setZoom(double zoom)
{
    zoom = std::clamp(zoom, kMinZoom, kMaxZoom);
    if (zoom == m_zoom) return;
    // Stay on the same slide as the thumbnails grow.
    const int cur = std::clamp(m_ed->currentPage(), 0, std::max(0, slideCount() - 1));
    m_zoom = zoom;
    m_keys = slideKeys();
    layoutChanged();
    ensureVisible(cur);
    Q_EMIT zoomChanged(m_zoom);
}

// ---- drawing ----

void SlideSorter::paintEvent(QPaintEvent *e)
{
    QPainter p(viewport());
    p.fillRect(e->rect(), uiPanel());
    const Document *d = m_ed->doc();
    const int n = slideCount();
    if (!d || n == 0) return;
    const QSize cs = cellSize(), ts = thumbSize();
    const int current = m_ed->currentPage();
    const bool dark = uiDark();
    const QColor accent = uiAccent();
    QColor tint = accent;
    tint.setAlphaF(dark ? 0.28f : 0.16f);
    int firstShown, lastShown;
    visibleRange(&firstShown, &lastShown);
    const QVector<deck::Section> sections = deck::sectionsOf(*d);
    for (int s = 0; s < sections.size(); ++s) {
        const QRect header = sectionHeaderRect(s);
        if (header.intersects(e->rect())) paintSectionHeader(&p, header, sections[s].name, sections[s].count, m_collapsed.contains(sections[s].id), false);
    }
    for (int i = std::max(0, firstShown); firstShown >= 0 && i <= lastShown; ++i) {
        if (!slideShown(i)) continue;
        const QRect cell(cellOrigin(i), cs);
        if (!cell.adjusted(-3, -3, 3, 3).intersects(e->rect())) continue;
        const bool selected = m_sel.contains(i), isCurrent = i == current;
        const Page &page = *d->pages[i];
        const bool hidden = page.hidden;
        if (selected) {
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(tint);
            p.drawRoundedRect(cell.adjusted(-2, -2, 2, 2), 4, 4);
            p.setRenderHint(QPainter::Antialiasing, false);
        }
        const QRect thumb(cell.topLeft() + QPoint(kPad, kPad), ts);
        p.setOpacity(hidden ? 0.4 : 1.0);
        // Until its thumbnail is drawn at this size, the one drawn at another stands in.
        QImage img = m_images.value(keyOf(i));
        if (img.isNull()) img = m_latest.value(lookOf(keyOf(i)));
        if (img.isNull()) {
            p.fillRect(thumb, Qt::white);
        } else {
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.drawImage(thumb, img);
        }
        p.setOpacity(1.0);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(dark ? QColor(0x4A, 0x52, 0x5E) : QColor(0xC4, 0xC8, 0xD0), 1));
        p.drawRect(QRectF(thumb).adjusted(0.5, 0.5, -0.5, -0.5));
        if (selected || isCurrent) {
            const double w = isCurrent ? 3 : 2;
            p.setPen(QPen(accent, w));
            p.drawRect(QRectF(thumb).adjusted(-w / 2, -w / 2, w / 2, w / 2));
        }
        // The number under the thumbnail, with a star before it for a slide that has a transition.
        QFont f = font();
        f.setStrikeOut(hidden);
        if (isCurrent) f.setWeight(QFont::DemiBold);
        p.setFont(f);
        QColor text = uiText();
        if (!selected) text.setAlphaF(0.65f);
        p.setPen(text);
        const QRect label(cell.left(), thumb.bottom() + 4, cell.width(), QFontMetrics(f).height() + 2);
        const QString number = QString::number(i + 1);
        p.drawText(label, Qt::AlignHCenter | Qt::AlignTop, number);
        if (deck::transitionOf(page).type != QLatin1String("none")) {
            const int w = QFontMetrics(f).horizontalAdvance(number);
            drawStar(&p, QPointF(label.center().x() - w / 2.0 - 10, label.top() + label.height() / 2.0), 5.5);
        }
    }
    if (m_dragging && m_drop.before >= 0) {
        p.setPen(Qt::NoPen);
        p.setBrush(accent);
        p.drawRoundedRect(dropMark(m_drop), 1.5, 1.5);
        // The slides being moved, faded, at the pointer.
        p.setOpacity(0.55);
        int shown = 0;
        for (int i : m_sel) {
            if (shown >= 3) break;
            QImage img = m_images.value(keyOf(i));
            if (img.isNull()) img = m_latest.value(lookOf(keyOf(i)));
            const QSize small(ts.width() / 2, ts.height() / 2);
            const QRect at(m_lastPos - QPoint(small.width() / 2, small.height() / 2) + QPoint(shown * 8, shown * 8), small);
            if (img.isNull()) p.fillRect(at, Qt::white);
            else p.drawImage(at, img);
            p.setPen(QPen(accent, 1));
            p.setBrush(Qt::NoBrush);
            p.drawRect(at);
            ++shown;
        }
    }
}

// The bar that shows where dragged slides would land: at the left of the slide
// they'd go before, or at the right of the last one of the section; in a
// section with no slides showing, under its heading.
QRect SlideSorter::dropMark(const Drop &drop) const
{
    ensureLayout();
    const QSize cs = cellSize();
    const int n = slideCount();
    if (drop.section >= 0 && drop.section < m_headerRects.size()) {
        const int start = m_sectionStart[drop.section], count = m_sectionCount[drop.section];
        if (!slideShown(start) || count == 0) {
            const QRect header = sectionHeaderRect(drop.section);
            return QRect(header.left(), header.bottom() + 3, 80, 3);
        }
        if (drop.before < start + count) {
            const QPoint o = cellOrigin(drop.before);
            return QRect(o.x() - kGap / 2 - 1, o.y(), 3, cs.height());
        }
        const QPoint o = cellOrigin(start + count - 1);
        return QRect(o.x() + cs.width() + kGap / 2 - 1, o.y(), 3, cs.height());
    }
    if (drop.before < n) {
        const QPoint o = cellOrigin(drop.before);
        return QRect(o.x() - kGap / 2 - 1, o.y(), 3, cs.height());
    }
    const QPoint o = cellOrigin(n - 1);
    return QRect(o.x() + cs.width() + kGap / 2 - 1, o.y(), 3, cs.height());
}

// ---- selecting ----

void SlideSorter::setCurrent(int slide)
{
    m_selfChange = true;
    m_ed->setCurrentPage(slide);
    m_selfChange = false;
}

void SlideSorter::selectOnly(int slide)
{
    m_sel = {slide};
    m_anchor = slide;
    setCurrent(slide);
    updateAccessibleText();
    viewport()->update();
}

void SlideSorter::selectRange(int from, int to)
{
    m_sel.clear();
    for (int i = std::min(from, to); i <= std::max(from, to); ++i) m_sel << i;
    setCurrent(to);
    updateAccessibleText();
    viewport()->update();
}

void SlideSorter::select(const QList<int> &slides, int current)
{
    const int n = slideCount();
    QList<int> list;
    for (int i : slides)
        if (i >= 0 && i < n && !list.contains(i)) list << i;
    std::sort(list.begin(), list.end());
    if (list.isEmpty()) return;
    m_sel = list;
    if (current < 0 || current >= n) current = list.last();
    if (!m_sel.contains(current)) m_sel << current, std::sort(m_sel.begin(), m_sel.end());
    m_anchor = current;
    setCurrent(current);
    updateAccessibleText();
    viewport()->update();
}

// The editor's current slide changed: if it wasn't us, it alone is selected.
void SlideSorter::currentChanged()
{
    if (m_selfChange) return;
    const int cur = std::clamp(m_ed->currentPage(), 0, std::max(0, slideCount() - 1));
    if (!m_sel.contains(cur)) {
        m_sel = {cur};
        m_anchor = cur;
    }
    if (isVisible()) ensureVisible(cur);
    updateAccessibleText();
    viewport()->update();
}

void SlideSorter::moveCurrent(int slide, bool extend)
{
    const int n = slideCount();
    if (n == 0) return;
    slide = std::clamp(slide, 0, n - 1);
    if (extend) selectRange(m_anchor, slide);
    else selectOnly(slide);
    ensureVisible(slide);
}

void SlideSorter::ensureVisible(int slide)
{
    if (slide < 0 || slide >= slideCount()) return;
    // A slide in a section that is folded away opens it.
    if (!slideShown(slide)) setCollapsed(deck::sectionOfSlide(*m_ed->doc(), slide), false);
    if (!slideShown(slide)) return;
    const QSize cs = cellSize();
    const int top = m_origins[slide].y();
    QScrollBar *bar = verticalScrollBar();
    if (top - kGap < bar->value()) bar->setValue(top - kGap);
    else if (top + cs.height() + kGap > bar->value() + viewport()->height()) bar->setValue(top + cs.height() + kGap - viewport()->height());
    scheduleRender();
}

void SlideSorter::updateAccessibleText()
{
    setAccessibleDescription(tr("Slide %1 of %2").arg(m_ed->currentPage() + 1).arg(slideCount()));
}

// ---- the mouse ----

void SlideSorter::mousePressEvent(QMouseEvent *e)
{
    const int i = slideAt(e->pos());
    m_pressSlide = -1;
    m_pressHeader = -1;
    m_collapseOnRelease = false;
    m_dragging = false;
    // A heading is no slide: a click on it folds or opens its section when let go.
    if (const int header = headerAt(e->pos()); header >= 0) {
        if (e->button() == Qt::LeftButton) {
            setFocus(Qt::MouseFocusReason);
            m_pressHeader = header;
        }
        return;
    }
    if (e->button() == Qt::RightButton) {
        if (i >= 0 && !m_sel.contains(i)) selectOnly(i);
        return;
    }
    if (e->button() != Qt::LeftButton) return;
    setFocus(Qt::MouseFocusReason);
    if (i < 0) return;
    m_pressSlide = i;
    m_pressPos = m_lastPos = e->pos();
    if (e->modifiers() & Qt::ShiftModifier) {
        selectRange(m_anchor, i);
    } else if (e->modifiers() & Qt::ControlModifier) {
        if (m_sel.contains(i)) {
            if (m_sel.size() > 1) {
                m_sel.removeOne(i);
                setCurrent(m_sel.contains(m_ed->currentPage()) ? m_ed->currentPage() : m_sel.last());
            }
        } else {
            m_sel << i;
            std::sort(m_sel.begin(), m_sel.end());
            setCurrent(i);
        }
        m_anchor = i;
        updateAccessibleText();
        viewport()->update();
    } else if (!m_sel.contains(i)) {
        selectOnly(i);
    } else {
        // Pressed on one of several selected: they move together, or it alone is selected when let go.
        m_collapseOnRelease = m_sel.size() > 1;
        if (m_ed->currentPage() != i) {
            setCurrent(i);
            viewport()->update();
        }
    }
}

void SlideSorter::mouseMoveEvent(QMouseEvent *e)
{
    if (m_pressSlide < 0) return;
    m_lastPos = e->pos();
    if (!m_dragging && (e->pos() - m_pressPos).manhattanLength() >= QApplication::startDragDistance()) {
        m_dragging = true;
        m_collapseOnRelease = false;
        m_scrollTimer.start();
    }
    if (!m_dragging) return;
    m_drop = dropAt(e->pos());
    viewport()->update();
}

// Where slides dropped at `pos` would go. Without sections: before the slide
// the pointer is on the left half of, or after the one it is on the right
// half of, the end for a place past the last slide. With them: the same
// among the slides of the section the pointer is in, so that the end of a
// section is after its last slide and not before the next section's first.
SlideSorter::Drop SlideSorter::dropAt(const QPoint &pos) const
{
    ensureLayout();
    Drop drop;
    const int n = slideCount();
    if (n == 0) {
        drop.before = 0;
        return drop;
    }
    const QSize cs = cellSize();
    const int cols = columns();
    const int y = pos.y() + verticalScrollBar()->value(), x = pos.x() - kMargin + kGap / 2;
    // Among `count` slides from `first` whose rows start at content height `top`.
    auto among = [&](int first, int count, int top) {
        const int rows = (count + cols - 1) / cols;
        const int row = std::clamp(int(std::floor(double(y - top + kGap / 2) / (cs.height() + kGap))), 0, rows - 1);
        const int col = std::clamp(int(std::floor(double(x) / (cs.width() + kGap))), 0, cols - 1);
        const int i = row * cols + col;
        if (i >= count) return first + count;
        return pos.x() > m_origins[first + i].x() + cs.width() / 2 ? first + i + 1 : first + i;
    };
    if (m_headerRects.isEmpty()) {
        drop.before = among(0, n, kMargin);
        return drop;
    }
    const QVector<deck::Section> sections = deck::sectionsOf(*m_ed->doc());
    for (int s = 0; s < sections.size(); ++s) {
        const bool last = s == sections.size() - 1;
        if (!last && y >= m_headerRects[s + 1].top() - kGap / 2) continue;
        drop.section = s;
        const int start = m_sectionStart[s], count = m_sectionCount[s];
        const int grid = m_headerRects[s].bottom() + 1 + kGap / 2;
        if (m_collapsed.contains(sections[s].id)) drop.before = start + count;
        else if (y < grid || count == 0) drop.before = start;
        else drop.before = among(start, count, grid);
        return drop;
    }
    return drop;
}

// While dragging near the top or the bottom the view scrolls.
void SlideSorter::autoScroll()
{
    if (!m_dragging) return;
    const int edge = 28;
    int dy = 0;
    if (m_lastPos.y() < edge) dy = -(edge - m_lastPos.y()) / 2 - 4;
    else if (m_lastPos.y() > viewport()->height() - edge) dy = (m_lastPos.y() - (viewport()->height() - edge)) / 2 + 4;
    if (dy == 0) return;
    verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
    m_drop = dropAt(m_lastPos);
    scheduleRender();
    viewport()->update();
}

void SlideSorter::finishDrag(const QPoint &pos)
{
    m_scrollTimer.stop();
    const Drop drop = dropAt(pos);
    m_dragging = false;
    m_drop = Drop();
    moveSlides(m_sel, drop.before, drop.section);
    viewport()->update();
}

void SlideSorter::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressHeader >= 0) {
        const int header = m_pressHeader;
        m_pressHeader = -1;
        if (e->button() == Qt::LeftButton && headerAt(e->pos()) == header) setCollapsed(header, !isCollapsed(header));
        return;
    }
    if (e->button() != Qt::LeftButton || m_pressSlide < 0) return;
    const int pressed = m_pressSlide;
    m_pressSlide = -1;
    if (m_dragging) {
        finishDrag(e->pos());
    } else if (m_collapseOnRelease) {
        selectOnly(pressed);
    }
    m_collapseOnRelease = false;
}

void SlideSorter::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (headerAt(e->pos()) >= 0) return;
    const int i = slideAt(e->pos());
    if (e->button() != Qt::LeftButton || i < 0) return;
    if (m_sel != QList<int>({i})) selectOnly(i);
    Q_EMIT openRequested(i);
}

void SlideSorter::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        setZoom(m_zoom * std::pow(1.0015, e->angleDelta().y()));
        e->accept();
        return;
    }
    QAbstractScrollArea::wheelEvent(e);
    scheduleRender();
}

void SlideSorter::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    if (const int section = headerAt(e->pos()); section >= 0) {
        SectionMenu commands;
        commands.rename = [this, section] { Q_EMIT renameSectionWanted(section); };
        commands.remove = [this, section] { Q_EMIT removeSectionWanted(section); };
        commands.removeWithSlides = [this, section] { Q_EMIT removeSectionAndSlidesWanted(section); };
        commands.moveUp = [this, section] { Q_EMIT moveSectionWanted(section, -1); };
        commands.moveDown = [this, section] { Q_EMIT moveSectionWanted(section, 1); };
        commands.collapseAll = [this] { setAllCollapsed(true); };
        commands.expandAll = [this] { setAllCollapsed(false); };
        commands.canMoveUp = section > 0;
        commands.canMoveDown = section < m_headerRects.size() - 1;
        fillSectionMenu(menu, commands);
        menu.exec(e->globalPos());
        return;
    }
    const int i = slideAt(e->pos());
    if (i >= 0 && !m_sel.contains(i)) selectOnly(i);
    menu.addAction(tr("New Slide"), this, &SlideSorter::newSlideAfterSelection);
    menu.addAction(tr("Duplicate Slide"), this, &SlideSorter::duplicateSelected, QKeySequence(Qt::CTRL | Qt::Key_D));
    menu.addAction(tr("Delete Slide"), this, &SlideSorter::deleteSelected, QKeySequence(Qt::Key_Delete));
    menu.addSeparator();
    menu.addAction(tr("Hide Slide"), this, &SlideSorter::toggleHiddenSelected);
    menu.addSeparator();
    const QList<int> chosen = validSelection();
    menu.addAction(tr("Add Section"), this, [this, i, chosen] { Q_EMIT addSectionWanted(i >= 0 ? i : chosen.value(0, m_ed->currentPage())); });
    menu.exec(e->globalPos());
}

// ---- the keyboard ----

void SlideSorter::keyPressEvent(QKeyEvent *e)
{
    const int n = slideCount(), cur = m_ed->currentPage();
    const bool shift = e->modifiers() & Qt::ShiftModifier;
    switch (e->key()) {
    case Qt::Key_A:
        if (e->modifiers() & Qt::ControlModifier) {
            m_sel.clear();
            for (int i = 0; i < n; ++i) m_sel << i;
            updateAccessibleText();
            viewport()->update();
            return;
        }
        break;
    case Qt::Key_D:
        if (e->modifiers() & Qt::ControlModifier) {
            duplicateSelected();
            return;
        }
        break;
    case Qt::Key_Escape:
        if (m_dragging) {
            m_dragging = false;
            m_pressSlide = -1;
            m_drop = Drop();
            m_scrollTimer.stop();
            viewport()->update();
            return;
        }
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace: deleteSelected(); return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (n) Q_EMIT openRequested(std::clamp(cur, 0, n - 1));
        return;
    // The slides of a section folded away are passed over.
    case Qt::Key_Left: moveCurrent(std::max(0, nextShown(cur - 1, -1)), shift); return;
    case Qt::Key_Right: moveCurrent(nextShown(cur + 1, 1) < 0 ? cur : nextShown(cur + 1, 1), shift); return;
    // Up and Down go to the slide in the next row, nearest across: down from a row with a shorter one below goes to the last slide of it.
    case Qt::Key_Up: moveCurrent(neighbor(cur, -1), shift); return;
    case Qt::Key_Down: moveCurrent(neighbor(cur, 1), shift); return;
    case Qt::Key_Home: moveCurrent(std::max(0, nextShown(0, 1)), shift); return;
    case Qt::Key_End: moveCurrent(nextShown(n - 1, -1) < 0 ? n - 1 : nextShown(n - 1, -1), shift); return;
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        const int rows = std::max(1, viewport()->height() / (cellSize().height() + kGap));
        moveCurrent(neighbor(cur, e->key() == Qt::Key_PageUp ? -rows : rows), shift);
        return;
    }
    default: break;
    }
    QAbstractScrollArea::keyPressEvent(e);
}

// ---- changing the slides, each as one undo step ----

// The selected slides that are there (a change from elsewhere may not have been seen yet).
QList<int> SlideSorter::validSelection() const
{
    QList<int> out;
    for (int i : m_sel)
        if (i >= 0 && i < slideCount()) out << i;
    std::sort(out.begin(), out.end());
    return out;
}

void SlideSorter::moveSlides(const QList<int> &slides, int before, int into)
{
    Document *d = m_ed->doc();
    const int n = slideCount();
    QList<int> moved;
    for (int i : slides)
        if (i >= 0 && i < n && !moved.contains(i)) moved << i;
    std::sort(moved.begin(), moved.end());
    if (moved.isEmpty()) return;
    before = std::clamp(before, 0, n);
    // The old index of each place in the new order: the others, with the moved ones together before `before`.
    const QList<int> order = deck::orderAfterMoving(n, moved, before);
    bool same = true;
    for (int k = 0; k < n; ++k) same &= order[k] == k;
    // Where they already stand, asked into the section they are in, nothing changes.
    if (same) {
        bool there = true;
        if (into >= 0)
            for (int i : moved) there &= deck::sectionOfSlide(*d, i) == into;
        if (there) return;
    }
    const std::shared_ptr<Page> current = d->pages[std::clamp(m_ed->currentPage(), 0, n - 1)];
    m_ed->endTextEdit();
    m_ed->clearSelection();
    m_ed->beginChange(moved.size() == 1 ? tr("Move Slide") : tr("Move Slides"));
    deck::slidesReordered(*d, order, moved, into);
    const QVector<std::shared_ptr<Page>> old = d->pages;
    for (int k = 0; k < n; ++k) d->pages[k] = old[order[k]];
    QList<int> now;
    for (int k = 0; k < n; ++k)
        if (moved.contains(order[k])) now << k;
    int cur = int(d->pages.indexOf(current));
    if (!now.contains(cur)) cur = now.last();
    m_sel = now;
    m_anchor = cur;
    setCurrent(cur);
    m_ed->endChange();
    updateAccessibleText();
    ensureVisible(cur);
}

void SlideSorter::deleteSelected()
{
    Document *d = m_ed->doc();
    const int n = slideCount();
    QList<int> doomed = validSelection();
    // A presentation keeps at least one slide: the first stays when all are selected.
    if (doomed.size() >= n && !doomed.isEmpty()) doomed.removeFirst();
    if (doomed.isEmpty()) return;
    m_ed->endTextEdit();
    m_ed->clearSelection();
    m_ed->beginChange(doomed.size() == 1 ? tr("Delete Slide") : tr("Delete Slides"));
    for (int k = int(doomed.size()) - 1; k >= 0; --k) {
        d->pages.removeAt(doomed[k]);
        deck::slideRemoved(*d, doomed[k]);
    }
    const int to = std::clamp(doomed.first(), 0, int(d->pages.size()) - 1);
    m_sel = {to};
    m_anchor = to;
    setCurrent(to);
    m_ed->endChange();
    updateAccessibleText();
    ensureVisible(to);
}

void SlideSorter::duplicateSelected()
{
    Document *d = m_ed->doc();
    const QList<int> from = validSelection();
    if (from.isEmpty()) return;
    m_ed->endTextEdit();
    m_ed->clearSelection();
    m_ed->beginChange(from.size() == 1 ? tr("Duplicate Slide") : tr("Duplicate Slides"));
    // From the last, so the earlier slides' places stay as they were.
    for (int k = int(from.size()) - 1; k >= 0; --k) deck::duplicateSlide(*d, from[k]);
    QList<int> made;
    for (int k = 0; k < from.size(); ++k) made << from[k] + k + 1;
    m_sel = made;
    m_anchor = made.last();
    setCurrent(made.last());
    m_ed->endChange();
    updateAccessibleText();
    ensureVisible(made.last());
}

void SlideSorter::toggleHiddenSelected()
{
    Document *d = m_ed->doc();
    const QList<int> slides = validSelection();
    if (slides.isEmpty()) return;
    bool allHidden = true;
    for (int i : slides) allHidden &= d->pages[i]->hidden;
    m_ed->endTextEdit();
    m_ed->beginChange(slides.size() == 1 ? (allHidden ? tr("Show Slide") : tr("Hide Slide")) : (allHidden ? tr("Show Slides") : tr("Hide Slides")));
    for (int i : slides) d->pages[i]->hidden = !allHidden;
    m_ed->endChange();
    viewport()->update();
}

void SlideSorter::newSlideAfterSelection()
{
    const QList<int> slides = validSelection();
    if (slides.isEmpty()) return;
    setCurrent(slides.last());
    Q_EMIT newSlideWanted();
}

// ---- sections folded away ----

bool SlideSorter::isCollapsed(int section) const
{
    const QVector<deck::Section> sections = m_ed->doc() ? deck::sectionsOf(*m_ed->doc()) : QVector<deck::Section>();
    return section >= 0 && section < sections.size() && m_collapsed.contains(sections[section].id);
}

void SlideSorter::setCollapsed(int section, bool collapsed)
{
    const QVector<deck::Section> sections = m_ed->doc() ? deck::sectionsOf(*m_ed->doc()) : QVector<deck::Section>();
    if (section < 0 || section >= sections.size() || isCollapsed(section) == collapsed) return;
    if (collapsed) m_collapsed.insert(sections[section].id);
    else m_collapsed.remove(sections[section].id);
    ++m_collapsedSerial;
    layoutChanged();
    Q_EMIT collapsedChanged();
}

void SlideSorter::setAllCollapsed(bool collapsed)
{
    const QVector<deck::Section> sections = m_ed->doc() ? deck::sectionsOf(*m_ed->doc()) : QVector<deck::Section>();
    QSet<QString> now;
    if (collapsed)
        for (const deck::Section &s : sections) now.insert(s.id);
    if (now == m_collapsed) return;
    m_collapsed = now;
    ++m_collapsedSerial;
    layoutChanged();
    Q_EMIT collapsedChanged();
}

void SlideSorter::setCollapsedSections(const QSet<QString> &ids)
{
    if (ids == m_collapsed) return;
    m_collapsed = ids;
    ++m_collapsedSerial;
    layoutChanged();
}

} // namespace jp
