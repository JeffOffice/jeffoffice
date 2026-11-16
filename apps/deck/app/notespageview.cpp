#include "app/notespageview.h"

#include "app/editor.h"
#include "app/notespane.h"
#include "core/deck.h"
#include "core/printheaderfooter.h"
#include "io/deckoutput.h"

#include <QFontMetricsF>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QTextDocument>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {
constexpr double kAround = 24;   // pixels between the page and the view's edge
constexpr double kCorner = 9;    // the corner handle, in pixels each way
const QColor kInk(0x20, 0x20, 0x20);

constexpr deck::NotesPart kNotesParts[] = {deck::NotesPart::SlideImage, deck::NotesPart::Body, deck::NotesPart::Header, deck::NotesPart::Date, deck::NotesPart::Footer, deck::NotesPart::Number};
constexpr deck::HandoutPart kHandoutParts[] = {deck::HandoutPart::Area, deck::HandoutPart::Header, deck::HandoutPart::Date, deck::HandoutPart::Footer, deck::HandoutPart::Number};

QString notesLabel(deck::NotesPart p)
{
    switch (p) {
    case deck::NotesPart::SlideImage: return NotesPageView::tr("Slide Image");
    case deck::NotesPart::Body: return NotesPageView::tr("Notes");
    case deck::NotesPart::Header: return NotesPageView::tr("Header");
    case deck::NotesPart::Date: return NotesPageView::tr("Date");
    case deck::NotesPart::Footer: return NotesPageView::tr("Footer");
    default: return NotesPageView::tr("Page Number");
    }
}
QString handoutLabel(deck::HandoutPart p)
{
    switch (p) {
    case deck::HandoutPart::Area: return NotesPageView::tr("Slides");
    case deck::HandoutPart::Header: return NotesPageView::tr("Header");
    case deck::HandoutPart::Date: return NotesPageView::tr("Date");
    case deck::HandoutPart::Footer: return NotesPageView::tr("Footer");
    default: return NotesPageView::tr("Page Number");
    }
}
} // namespace

NotesPageView::NotesPageView(Editor *ed, QWidget *parent) : QAbstractScrollArea(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("notesPageView"));
    setAccessibleName(tr("Notes Page"));
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setMouseTracking(true);
    m_box = new NotesPane(ed, viewport());
    m_box->setFrameShape(QFrame::NoFrame);
    m_box->document()->setDocumentMargin(0);
    m_box->viewport()->setAutoFillBackground(false);
    QPalette p = m_box->palette();
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Text, kInk);
    m_box->setPalette(p);
    m_box->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_box->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_box->hide();
    connect(ed, &Editor::pageChanged, this, [this] {
        m_notesKept = m_handoutKept = false;
        m_box->reload();
        viewport()->update();
    });
    connect(ed, &Editor::changed, this, [this] {
        m_notesKept = m_handoutKept = false;
        if (m_mode == Mode::NotesPage) m_box->reload();
        updateRanges();
        layoutBox();
        viewport()->update();
    });
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, [this] { layoutBox(); viewport()->update(); });
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { layoutBox(); viewport()->update(); });
}

double NotesPageView::scale() const { return m_zoom * logicalDpiX() / 72.0; }

QSizeF NotesPageView::paper() const
{
    const QSizeF base = deck::localePaper();
    const Document *d = m_ed->doc();
    if (m_mode != Mode::HandoutMaster || !d) return base;
    return deck::handoutPaper(handoutNow(), base);
}

void NotesPageView::setMode(Mode mode)
{
    m_mode = mode;
    m_selected = -1;
    m_box->setVisible(mode == Mode::NotesPage);
    if (mode == Mode::NotesPage) m_box->reload();
    updateRanges();
    layoutBox();
    viewport()->update();
}

double NotesPageView::fitZoom() const
{
    const QSizeF size = paper();
    const double toPixels = logicalDpiX() / 72.0;
    const double w = (viewport()->width() - 2 * kAround) / (size.width() * toPixels), h = (viewport()->height() - 2 * kAround) / (size.height() * toPixels);
    return std::clamp(std::min(w, h), 0.1, 4.0);
}

void NotesPageView::fitToView()
{
    m_fitPending = true;
    if (viewport()->width() >= 200 && viewport()->height() >= 200) {
        m_fitPending = false;
        setZoom(fitZoom());
    }
}

void NotesPageView::setZoom(double zoom)
{
    zoom = std::clamp(zoom, 0.1, 4.0);
    if (std::abs(zoom - m_zoom) < 1e-9) return;
    m_zoom = zoom;
    updateRanges();
    layoutBox();
    viewport()->update();
    Q_EMIT zoomChanged(m_zoom);
}

void NotesPageView::updateRanges()
{
    const QSizeF size = paper() * scale();
    horizontalScrollBar()->setRange(0, std::max(0, int(std::ceil(size.width() + 2 * kAround)) - viewport()->width()));
    verticalScrollBar()->setRange(0, std::max(0, int(std::ceil(size.height() + 2 * kAround)) - viewport()->height()));
    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setPageStep(viewport()->height());
}

QRectF NotesPageView::pageRect() const
{
    const QSizeF size = paper() * scale();
    const double x = size.width() + 2 * kAround < viewport()->width() ? (viewport()->width() - size.width()) / 2 : kAround - horizontalScrollBar()->value();
    return QRectF(QPointF(x, kAround - verticalScrollBar()->value()), size);
}

QRectF NotesPageView::toView(const QRectF &f) const
{
    const QRectF page = pageRect();
    return QRectF(page.x() + f.x() * page.width(), page.y() + f.y() * page.height(), f.width() * page.width(), f.height() * page.height());
}

QRectF NotesPageView::toFractions(const QRectF &v) const
{
    const QRectF page = pageRect();
    return QRectF((v.x() - page.x()) / page.width(), (v.y() - page.y()) / page.height(), v.width() / page.width(), v.height() / page.height());
}

const deck::NotesMaster &NotesPageView::notesNow() const
{
    if (!m_notesKept) {
        m_notesLayout = deck::notesMasterOf(*m_ed->doc(), deck::localePaper());
        m_notesKept = true;
        ++m_reads;
    }
    return m_notesLayout;
}

const deck::HandoutMaster &NotesPageView::handoutNow() const
{
    if (!m_handoutKept) {
        m_handoutLayout = deck::handoutMasterOf(*m_ed->doc(), deck::localePaper());
        m_handoutKept = true;
        ++m_reads;
    }
    return m_handoutLayout;
}

QFont NotesPageView::labelFont(const QFont &base, const QRectF &box, double scale)
{
    QFont f = base;
    // The size of the text follows the zoom, but never more than the box is high.
    f.setPixelSize(std::max(1, int(std::min(10 * scale, box.height() - 2))));
    return f;
}

QString NotesPageView::labelText(const QString &text, const QFont &font, const QRectF &box)
{
    if (font.pixelSize() < 5) return QString();
    return QFontMetricsF(font).elidedText(text, Qt::ElideRight, std::max(0.0, box.width() - 6));
}

QRectF NotesPageView::slideImageRect() const
{
    const Document *d = m_ed->doc();
    if (!d) return QRectF();
    const deck::NotesPageGeometry g = deck::notesPageGeometry(notesNow(), paper(), d->pageSize());
    const QSizeF size = paper();
    return toView(QRectF(g.slideImage.x() / size.width(), g.slideImage.y() / size.height(), g.slideImage.width() / size.width(), g.slideImage.height() / size.height()));
}

QRectF NotesPageView::notesRect() const { return placeRect(deck::NotesPart::Body); }
QRectF NotesPageView::placeRect(deck::NotesPart part) const { return toView(notesNow().part(part)); }
QRectF NotesPageView::placeRect(deck::HandoutPart part) const { return toView(handoutNow().part(part)); }

void NotesPageView::layoutBox()
{
    if (m_mode != Mode::NotesPage || !m_ed->doc()) return;
    m_box->setGeometry(notesRect().toRect());
    QFont f = m_box->font();
    f.setPixelSize(std::max(6, int(std::lround(12 * scale()))));
    m_box->setFont(f);
}

void NotesPageView::commit() { m_box->commit(); }
void NotesPageView::reload() { m_box->reload(); }

void NotesPageView::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    if (m_fitPending && viewport()->width() >= 200 && viewport()->height() >= 200) {
        m_fitPending = false;
        setZoom(fitZoom());
    }
    updateRanges();
    layoutBox();
}

void NotesPageView::paintEvent(QPaintEvent *)
{
    QPainter p(viewport());
    p.fillRect(viewport()->rect(), palette().color(QPalette::Mid));
    const Document *d = m_ed->doc();
    if (!d) return;
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    const QRectF page = pageRect();
    p.fillRect(page.translated(3, 3), QColor(0, 0, 0, 60));
    p.fillRect(page, Qt::white);
    if (m_mode != Mode::NotesPage) {
        paintMaster(p);
        return;
    }
    const int slide = m_ed->currentPage();
    if (slide < 0 || slide >= d->pages.size()) return;
    deck::OutputOptions o;
    o.layout = deck::PrintLayout::NotesPages;
    o.hiddenSlides = true;
    o.skipNotesText = true;
    o.slides = {slide};
    p.save();
    p.translate(page.topLeft());
    p.scale(scale(), scale());
    deck::paintOutputPage(&p, *d, &m_ed->cache(), o, 0, paper());
    p.restore();
}

void NotesPageView::paintMaster(QPainter &p)
{
    const Document *d = m_ed->doc();
    const bool notes = m_mode == Mode::NotesMaster;
    const bool dragging = m_drag.part >= 0 && m_dragNow.isValid();
    deck::NotesMaster nm = notesNow();
    deck::HandoutMaster hm = handoutNow();
    if (dragging) {
        if (notes) nm.part(deck::NotesPart(m_drag.part)) = m_dragNow;
        else hm.part(deck::HandoutPart(m_drag.part)) = m_dragNow;
    }
    const QColor background = notes ? nm.background : hm.background;
    const QRectF page = pageRect();
    if (background.isValid()) p.fillRect(page, background);
    QFont font = p.font();
    font.setPixelSize(std::max(6, int(std::lround(10 * scale()))));
    p.setFont(font);
    const QColor label(0x55, 0x55, 0x55), fill(0xE6, 0xE8, 0xEC);
    auto outline = [&](const QRectF &r, int part, const QString &text, bool dim) {
        const bool selected = part == m_selected;
        p.setBrush(Qt::NoBrush);
        QPen pen(selected ? palette().color(QPalette::Highlight) : QColor(0x70, 0x70, 0x70), selected ? 2 : 1, Qt::DashLine);
        p.setPen(pen);
        p.drawRect(r);
        p.setPen(dim ? QColor(0xA0, 0xA0, 0xA0) : label);
        // (the words fit the box: in a size its height holds, cut short to its width, at any zoom)
        const QFont small = labelFont(font, r, scale());
        p.save();
        p.setFont(small);
        p.drawText(r.adjusted(3, 0, -3, 0), Qt::AlignLeft | (r.height() > 3 * small.pixelSize() ? Qt::AlignTop : Qt::AlignVCenter), labelText(text, small, r));
        p.restore();
        if (selected) {
            p.setPen(Qt::NoPen);
            p.setBrush(palette().color(QPalette::Highlight));
            p.drawRect(QRectF(r.right() - kCorner, r.bottom() - kCorner, kCorner, kCorner));
        }
    };
    const deck::PrintHeaderFooter hf = deck::printHeaderFooter(*d, notes ? deck::PrintedPages::Notes : deck::PrintedPages::Handouts);
    auto dimmed = [&](int part) {
        if (notes) return (part == int(deck::NotesPart::Header) && !hf.header) || (part == int(deck::NotesPart::Date) && !hf.date) || (part == int(deck::NotesPart::Footer) && !hf.footer) || (part == int(deck::NotesPart::Number) && !hf.pageNumber);
        return (part == int(deck::HandoutPart::Header) && !hf.header) || (part == int(deck::HandoutPart::Date) && !hf.date) || (part == int(deck::HandoutPart::Footer) && !hf.footer) || (part == int(deck::HandoutPart::Number) && !hf.pageNumber);
    };
    if (notes) {
        for (deck::NotesPart part : kNotesParts) {
            const QRectF r = toView(nm.part(part));
            if (part == deck::NotesPart::SlideImage) {
                const deck::NotesPageGeometry g = deck::notesPageGeometry(nm, paper(), d->pageSize());
                const double k = scale();
                p.fillRect(QRectF(page.x() + g.slideImage.x() * k, page.y() + g.slideImage.y() * k, g.slideImage.width() * k, g.slideImage.height() * k), fill);
            } else if (part == deck::NotesPart::Body) {
                p.setPen(label);
                static const char *const samples[] = {QT_TR_NOOP("Notes text: change its look here"), QT_TR_NOOP("Level 2 text"), QT_TR_NOOP("Level 3 text")};
                QRectF line = r.adjusted(4, 2 + font.pixelSize() * 1.6, -4, 0);
                for (int i = 0; i < 3; ++i) {
                    p.drawText(QRectF(line.x() + i * font.pixelSize() * 1.5, line.y() + i * font.pixelSize() * 1.4, line.width(), font.pixelSize() * 1.4), Qt::AlignLeft | Qt::AlignVCenter,
                               QFontMetricsF(font).elidedText(tr(samples[i]), Qt::ElideRight, std::max(0.0, line.width() - i * font.pixelSize() * 1.5)));
                }
            }
            outline(r, int(part), notesLabel(part), dimmed(int(part)));
        }
        return;
    }
    const deck::HandoutGeometry g = deck::handoutGeometry(hm, paper(), hm.perPage, true);
    for (int i = 0; i < g.slots.size(); ++i) {
        const deck::HandoutSlot &s = g.slots[i];
        const QRectF r = toView(QRectF(s.room.x() / paper().width(), s.room.y() / paper().height(), s.room.width() / paper().width(), s.room.height() / paper().height()));
        p.fillRect(r, fill);
        p.setPen(QPen(QColor(0x60, 0x60, 0x60), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        p.setPen(label);
        p.drawText(r, Qt::AlignCenter, QString::number(i + 1));
        if (s.lines) {
            p.setPen(QPen(QColor(0x90, 0x90, 0x90), 1));
            const double from = page.x() + (s.cell.left() + s.cell.width() * 0.52) * scale(), to = page.x() + (s.cell.right() - 10) * scale();
            for (int k = 1; k <= 5; ++k) p.drawLine(QPointF(from, r.top() + r.height() * k / 5), QPointF(to, r.top() + r.height() * k / 5));
        }
    }
    for (deck::HandoutPart part : kHandoutParts) outline(toView(hm.part(part)), int(part), handoutLabel(part), dimmed(int(part)));
}

NotesPageView::Hit NotesPageView::hitAt(const QPointF &at) const
{
    Hit hit;
    if (m_mode == Mode::NotesPage || !m_ed->doc()) return hit;
    auto test = [&](int part, const QRectF &r) {
        if (hit.part >= 0) return;
        if (QRectF(r.right() - kCorner, r.bottom() - kCorner, kCorner, kCorner).contains(at) && part == m_selected) {
            hit.part = part;
            hit.corner = true;
        } else if (r.contains(at)) {
            hit.part = part;
        }
    };
    // The small places are on top of the big ones: the last drawn is tested first.
    if (m_mode == Mode::NotesMaster) {
        const deck::NotesMaster m = notesNow();
        for (int i = int(std::size(kNotesParts)) - 1; i >= 0; --i) test(int(kNotesParts[i]), toView(m.part(kNotesParts[i])));
    } else {
        const deck::HandoutMaster m = handoutNow();
        for (int i = int(std::size(kHandoutParts)) - 1; i >= 0; --i) test(int(kHandoutParts[i]), toView(m.part(kHandoutParts[i])));
    }
    return hit;
}

void NotesPageView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || m_mode == Mode::NotesPage) return QAbstractScrollArea::mousePressEvent(e);
    m_drag = hitAt(e->position());
    m_selected = m_drag.part;
    m_dragNow = QRectF();
    if (m_drag.part >= 0) {
        m_dragFrom = e->position();
        m_dragStart = m_mode == Mode::NotesMaster ? notesNow().part(deck::NotesPart(m_drag.part)) : handoutNow().part(deck::HandoutPart(m_drag.part));
    }
    viewport()->update();
}

void NotesPageView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_mode == Mode::NotesPage) return QAbstractScrollArea::mouseMoveEvent(e);
    if (m_drag.part < 0 || !(e->buttons() & Qt::LeftButton)) {
        const Hit over = hitAt(e->position());
        viewport()->setCursor(over.part < 0 ? Qt::ArrowCursor : over.corner ? Qt::SizeFDiagCursor : Qt::SizeAllCursor);
        return;
    }
    const QRectF page = pageRect();
    const double dx = (e->position().x() - m_dragFrom.x()) / page.width(), dy = (e->position().y() - m_dragFrom.y()) / page.height();
    QRectF r = m_dragStart;
    if (m_drag.corner) r.setSize(QSizeF(m_dragStart.width() + dx, m_dragStart.height() + dy));
    else r.translate(dx, dy);
    m_dragNow = deck::sanitizedPlace(r, m_dragStart);
    viewport()->update();
}

void NotesPageView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_mode == Mode::NotesPage || m_drag.part < 0) return QAbstractScrollArea::mouseReleaseEvent(e);
    const Hit hit = m_drag;
    const QRectF now = m_dragNow;
    m_drag = Hit();
    m_dragNow = QRectF();
    if (now.isValid() && !deck::nearly(now, m_dragStart, 1e-6)) {
        if (m_mode == Mode::NotesMaster) setPlace(deck::NotesPart(hit.part), now);
        else setPlace(deck::HandoutPart(hit.part), now);
    }
    viewport()->update();
}

void NotesPageView::keyPressEvent(QKeyEvent *e)
{
    const int n = m_ed->doc() ? int(m_ed->doc()->pages.size()) : 0;
    if (m_mode == Mode::NotesPage && (e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) && n > 0) {
        commit();
        m_ed->setCurrentPage(std::clamp(m_ed->currentPage() + (e->key() == Qt::Key_PageUp ? -1 : 1), 0, n - 1));
        return;
    }
    QAbstractScrollArea::keyPressEvent(e);
}

void NotesPageView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        setZoom(m_zoom * std::pow(1.0015, e->angleDelta().y()));
        e->accept();
        return;
    }
    QAbstractScrollArea::wheelEvent(e);
}

void NotesPageView::change(const QString &label, const std::function<void(Document &)> &fn)
{
    Document *d = m_ed->doc();
    if (!d || m_ed->readOnly()) return;
    m_ed->beginChange(label);
    fn(*d);
    m_ed->endChange();
}

void NotesPageView::setPlace(deck::NotesPart part, const QRectF &fractions)
{
    change(tr("Edit Notes Master"), [&](Document &d) {
        deck::NotesMaster m = deck::notesMasterOf(d, deck::localePaper());
        m.part(part) = deck::sanitizedPlace(fractions, m.part(part));
        m.custom = true;
        deck::setNotesMaster(d, m);
    });
}

void NotesPageView::setPlace(deck::HandoutPart part, const QRectF &fractions)
{
    change(tr("Edit Handout Master"), [&](Document &d) {
        deck::HandoutMaster m = deck::handoutMasterOf(d, deck::localePaper());
        m.part(part) = deck::sanitizedPlace(fractions, m.part(part));
        m.custom = true;
        deck::setHandoutMaster(d, m);
    });
}

void NotesPageView::resetLayout()
{
    const bool notes = m_mode == Mode::NotesMaster;
    change(notes ? tr("Reset Notes Master") : tr("Reset Handout Master"), [&](Document &d) {
        if (notes) deck::clearNotesMaster(d);
        else deck::clearHandoutMaster(d);
    });
}

void NotesPageView::setBackground(const QColor &color)
{
    const bool notes = m_mode == Mode::NotesMaster;
    change(tr("Page Background"), [&](Document &d) {
        if (notes) {
            deck::NotesMaster m = deck::notesMasterOf(d, deck::localePaper());
            m.background = color;
            m.custom = true;
            deck::setNotesMaster(d, m);
        } else {
            deck::HandoutMaster m = deck::handoutMasterOf(d, deck::localePaper());
            m.background = color;
            m.custom = true;
            deck::setHandoutMaster(d, m);
        }
    });
}

void NotesPageView::setHandoutCount(int perPage)
{
    if (!deck::isHandoutCount(perPage)) return;
    change(tr("Slides Per Page"), [&](Document &d) { deck::setHandoutPerPage(d, perPage); });
    viewport()->update();
}

void NotesPageView::setHandoutLandscape(bool landscape)
{
    change(landscape ? tr("Landscape") : tr("Portrait"), [&](Document &d) {
        deck::HandoutMaster m = deck::handoutMasterOf(d, deck::localePaper());
        if (m.landscape == landscape) return;
        // (the places are fractions of the page: the page's shape changes, so they start again at the standard for the new shape, with the background and count kept)
        const QSizeF base = deck::localePaper();
        const deck::HandoutMaster fresh = deck::defaultHandoutMaster(landscape ? base.transposed() : base);
        m.landscape = landscape;
        m.area = fresh.area;
        m.header = fresh.header;
        m.date = fresh.date;
        m.footer = fresh.footer;
        m.number = fresh.number;
        m.custom = true;
        deck::setHandoutMaster(d, m);
    });
    updateRanges();
    viewport()->update();
}

} // namespace jp
