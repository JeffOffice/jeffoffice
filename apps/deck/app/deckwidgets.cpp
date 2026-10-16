#include "app/deckwidgets.h"

#include "app/editor.h"
#include "app/icons.h"
#include "core/deck.h"
#include "render/renderer.h"

#include <QCoreApplication>
#include <QFocusEvent>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

namespace jp {

namespace {
constexpr int kPictureWidth = 112;   // a layout's picture in the gallery
constexpr int kColumns = 3;
constexpr int kPad = 6;

QSize pictureSize(const Document &d)
{
    const QSizeF ps = d.pageSize();
    return QSize(kPictureWidth, std::max(1, int(std::lround(kPictureWidth * ps.height() / std::max(1.0, ps.width())))));
}
} // namespace

QImage layoutPicture(const Document &d, const MasterPage &layout, int width)
{
    const QSizeF ps = d.pageSize();
    const double scale = width / std::max(1.0, ps.width());
    QImage img(width, std::max(1, int(std::lround(ps.height() * scale))), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.scale(scale, scale);
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &d;
    ctx.cache = &cache;
    ctx.opt.output = true;
    const QRectF page(QPointF(0, 0), ps);
    const QVector<MasterPage *> chain = d.masterChain(&layout);
    for (int i = int(chain.size()) - 1; i >= 0; --i)
        if (chain[i]->background.type != Fill::NoFill) {
            Renderer::paintBackground(&p, ctx, chain[i]->background, page);
            break;
        }
    // What each master draws on every slide (unless the layout hides it); the layout's own placeholders show their sample text.
    for (const MasterPage *m : chain) {
        if (m == &layout) {
            Renderer::paintMaster(&p, ctx, layout);
            continue;
        }
        if (!layout.showMasterItems) continue;
        MasterPage drawn;
        for (const auto &it : m->items)
            if (!it->role.startsWith(QLatin1String("ph:"))) drawn.items.push_back(it);
        Renderer::paintMaster(&p, ctx, drawn);
    }
    // Where the placeholders are, including the ones with nothing to say (a picture's).
    QPen outline(QColor(0x80, 0x84, 0x8C));
    outline.setWidth(0);
    outline.setStyle(Qt::DotLine);
    p.setPen(outline);
    p.setBrush(Qt::NoBrush);
    for (const auto &it : layout.items) {
        const QString type = deck::placeholderType(*it);
        if (type.isEmpty() || type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum")) continue;
        p.drawPolygon(it->outline());
    }
    return img;
}

// ---------------- LayoutGallery ----------------
LayoutGallery::LayoutGallery(Editor *ed, QWidget *parent) : QWidget(parent), m_ed(ed)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Slide layouts"));
    setAccessibleDescription(tr("The arrow keys move between layouts, and Enter chooses one."));
    reload();
}

void LayoutGallery::reload()
{
    m_tiles.clear();
    const Document *d = m_ed->doc();
    if (!d) return;
    const qreal dpr = devicePixelRatioF();
    const int page = m_ed->currentPage();
    const MasterPage *current = page >= 0 && page < d->pages.size() ? d->masterFor(*d->pages[page]) : nullptr;
    m_current = current ? deck::layoutKey(*d, *current) : QString();
    for (MasterPage *l : deck::layouts(*d, deck::slideMasterOf(*d, page))) {
        QImage pic = layoutPicture(*d, *l, int(std::lround(kPictureWidth * dpr)));
        pic.setDevicePixelRatio(dpr);
        m_tiles << Tile{deck::layoutKey(*d, *l), QCoreApplication::translate("Ribbon", l->name.toUtf8().constData()), pic};
    }
    updateGeometry();
    update();
}

QRect LayoutGallery::tileRect(int i) const
{
    const QSize pic = m_ed->doc() ? pictureSize(*m_ed->doc()) : QSize(kPictureWidth, kPictureWidth * 9 / 16);
    const int w = pic.width() + 2 * kPad + 4, h = pic.height() + QFontMetrics(font()).height() + 3 * kPad + 2;
    return QRect(kPad + (i % kColumns) * w, kPad + (i / kColumns) * h, w, h);
}

QSize LayoutGallery::sizeHint() const
{
    const QRect last = tileRect(std::max(0, int(m_tiles.size()) - 1)), first = tileRect(0);
    return QSize(first.width() * kColumns + 2 * kPad, last.bottom() + kPad + 1);
}

int LayoutGallery::tileAt(const QPoint &p) const
{
    for (int i = 0; i < m_tiles.size(); ++i)
        if (tileRect(i).contains(p)) return i;
    return -1;
}

void LayoutGallery::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    for (int i = 0; i < m_tiles.size(); ++i) {
        const Tile &t = m_tiles[i];
        const QRectF r = QRectF(tileRect(i)).adjusted(2, 2, -2, -2);
        const bool current = t.type == m_current, hot = i == m_hover;
        if (hot || current) {
            QColor bg = uiAccent();
            bg.setAlphaF(hot ? 0.18f : 0.10f);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(r, 6, 6);
        }
        if (current) {
            p.setPen(QPen(uiAccent(), 2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r, 6, 6);
        }
        const QSize ps = t.picture.deviceIndependentSize().toSize();
        const QRect pr(int(r.center().x() - ps.width() / 2.0), int(r.top() + kPad), ps.width(), ps.height());
        p.drawImage(pr, t.picture);
        p.setPen(QPen(uiDark() ? QColor(0x4A, 0x52, 0x5E) : QColor(0xC4, 0xC8, 0xD0), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(pr.adjusted(0, 0, -1, -1));
        p.setPen(uiText());
        const QRectF label(r.left() + 2, pr.bottom() + kPad / 2.0 + 1, r.width() - 4, r.bottom() - pr.bottom() - kPad / 2.0 - 1);
        p.drawText(label, Qt::AlignHCenter | Qt::AlignTop, QFontMetrics(font()).elidedText(t.name, Qt::ElideRight, int(label.width())));
    }
    if (hasFocus() && m_hover >= 0) {
        p.setPen(QPen(focusRingColor(), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(tileRect(m_hover)).adjusted(1, 1, -1, -1), 6, 6);
    }
}

void LayoutGallery::mouseMoveEvent(QMouseEvent *e)
{
    const int i = tileAt(e->position().toPoint());
    if (i != m_hover) {
        m_hover = i;
        update();
    }
    if (i >= 0) QToolTip::showText(e->globalPosition().toPoint(), m_tiles[i].name, this);
}

void LayoutGallery::leaveEvent(QEvent *)
{
    if (!hasFocus()) m_hover = -1;
    update();
}

void LayoutGallery::mousePressEvent(QMouseEvent *e)
{
    const int i = tileAt(e->position().toPoint());
    if (i >= 0) Q_EMIT chosen(m_tiles[i].type);
}

void LayoutGallery::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    if (m_hover < 0)
        for (int i = 0; i < m_tiles.size(); ++i)
            if (m_tiles[i].type == m_current) m_hover = i;
    if (m_hover < 0 && !m_tiles.isEmpty()) m_hover = 0;
    update();
}

void LayoutGallery::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    update();
}

void LayoutGallery::keyPressEvent(QKeyEvent *e)
{
    const int n = int(m_tiles.size());
    int i = m_hover;
    switch (e->key()) {
    case Qt::Key_Left: i = std::max(0, i - 1); break;
    case Qt::Key_Right: i = std::min(n - 1, i + 1); break;
    case Qt::Key_Up: i = i - kColumns >= 0 ? i - kColumns : i; break;
    case Qt::Key_Down: i = i + kColumns < n ? i + kColumns : i; break;
    case Qt::Key_Home: i = 0; break;
    case Qt::Key_End: i = n - 1; break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        if (i >= 0 && i < n) Q_EMIT chosen(m_tiles[i].type);
        return;
    default: QWidget::keyPressEvent(e); return;
    }
    m_hover = i;
    update();
}

// ---------------- ThemeColors ----------------
namespace {
constexpr int kSwatch = 20, kGap = 4;
// The theme's colors in the order shown: the accents above, the rest below.
constexpr int kOrder[12] = {4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 10, 11};
} // namespace

ThemeColors::ThemeColors(Editor *ed, QWidget *parent) : QWidget(parent), m_ed(ed)
{
    setAccessibleName(tr("Theme colors"));
    setAccessibleDescription(tr("The twelve colors of this presentation's theme."));
}

QSize ThemeColors::sizeHint() const { return QSize(6 * (kSwatch + kGap) + kGap, 2 * (kSwatch + kGap) + kGap); }

QRect ThemeColors::swatchRect(int i) const { return QRect(kGap + (i % 6) * (kSwatch + kGap), kGap + (i / 6) * (kSwatch + kGap), kSwatch, kSwatch); }

int ThemeColors::swatchAt(const QPoint &p) const
{
    for (int i = 0; i < 12; ++i)
        if (swatchRect(i).contains(p)) return i;
    return -1;
}

void ThemeColors::paintEvent(QPaintEvent *)
{
    if (!m_ed->doc()) return;
    const deck::DeckTheme theme = deck::themeOf(*m_ed->doc());
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    for (int i = 0; i < 12; ++i) {
        p.setPen(QPen(uiDark() ? QColor(0x5A, 0x61, 0x6D) : QColor(0xB8, 0xBE, 0xC8), 1));
        p.setBrush(theme.colors[kOrder[i]]);
        p.drawRoundedRect(QRectF(swatchRect(i)).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
    }
}

bool ThemeColors::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip && m_ed->doc()) {
        const auto *he = static_cast<QHelpEvent *>(e);
        const int i = swatchAt(he->pos());
        if (i >= 0) {
            static const char *names[12] = {QT_TR_NOOP("Dark 1"),   QT_TR_NOOP("Light 1"),  QT_TR_NOOP("Dark 2"),   QT_TR_NOOP("Light 2"),
                                            QT_TR_NOOP("Accent 1"), QT_TR_NOOP("Accent 2"), QT_TR_NOOP("Accent 3"), QT_TR_NOOP("Accent 4"),
                                            QT_TR_NOOP("Accent 5"), QT_TR_NOOP("Accent 6"), QT_TR_NOOP("Hyperlink"), QT_TR_NOOP("Followed Hyperlink")};
            const int k = kOrder[i];
            QToolTip::showText(he->globalPos(), QStringLiteral("%1 (%2)").arg(tr(names[k]), deck::themeOf(*m_ed->doc()).colors[k].name().toUpper()), this);
        } else {
            QToolTip::hideText();
        }
        return true;
    }
    return QWidget::event(e);
}

} // namespace jp
