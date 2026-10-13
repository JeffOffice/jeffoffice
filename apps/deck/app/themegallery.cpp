#include "app/themegallery.h"

#include "app/editor.h"
#include "app/icons.h"
#include "core/deck.h"
#include "core/themes.h"
#include "render/renderer.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QHash>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <cmath>

namespace jp {

namespace {

constexpr int kPad = 6;

// The first slide of a presentation, `width` pixels across at `dpr`.
QImage firstSlide(const Document &d, int width, qreal dpr)
{
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &d;
    ctx.cache = &cache;
    ctx.opt.output = true;
    QImage img = Renderer::renderToImage(ctx, 0, width * dpr / std::max(1.0, d.pageSize().width()));
    img.setDevicePixelRatio(dpr);
    return img;
}

QString signatureOf(const Document &d)
{
    const deck::DeckTheme t = deck::themeOf(d);
    QString s = t.name;
    for (const QColor &c : t.colors) s += QLatin1Char(' ') + c.name();
    const MasterPage *master = nullptr;
    for (const auto &m : d.masters)
        if (deck::isSlideMaster(*m)) {
            master = m.get();
            break;
        }
    return s + QLatin1Char(' ') + QString::number(master ? deck::backgroundStyleOf(master->background, t) : -1);
}

} // namespace

QImage themePicture(int theme, int variant, int width, qreal dpr)
{
    static QHash<QString, QImage> kept;
    const QString key = QStringLiteral("%1|%2|%3|%4").arg(theme).arg(variant).arg(width).arg(dpr);
    auto found = kept.constFind(key);
    if (found == kept.cend()) found = kept.insert(key, firstSlide(*deck::themePresentation(theme, variant), width, dpr));
    return found.value();
}

// ---------------- PictureGrid ----------------
PictureGrid::PictureGrid(int pictureWidth, int columns, bool names, QWidget *parent)
    : QWidget(parent), m_width(pictureWidth), m_columns(std::max(1, columns)), m_names(names)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QSize PictureGrid::pictureSize() const { return QSize(m_width, std::max(1, int(std::lround(m_width * 9.0 / 16.0)))); }

void PictureGrid::setTiles(const QVector<Tile> &tiles)
{
    m_tiles = tiles;
    m_pictures.clear();
    m_pictures.resize(tiles.size());
    m_hover = -1;
    m_firstRow = std::clamp(m_firstRow, 0, std::max(0, rows() - 1));
    updateGeometry();
    update();
}

void PictureGrid::reload()
{
    m_pictures.clear();
    m_pictures.resize(m_tiles.size());
    update();
}

void PictureGrid::setCurrent(int tile)
{
    if (m_current == tile) return;
    m_current = tile;
    update();
}

void PictureGrid::setVisibleRows(int rows)
{
    m_visibleRows = std::max(0, rows);
    updateGeometry();
    update();
}

void PictureGrid::setFirstRow(int row)
{
    m_firstRow = std::clamp(row, 0, std::max(0, rows() - std::max(1, m_visibleRows)));
    update();
}

const QImage &PictureGrid::pictureOf(int i)
{
    if (m_pictures.size() != m_tiles.size()) m_pictures.resize(m_tiles.size());
    if (m_pictures[i].isNull() && m_provider) m_pictures[i] = m_provider(i, m_width, devicePixelRatioF());
    return m_pictures[i];
}

QRect PictureGrid::tileRect(int i) const
{
    if (i < 0 || i >= m_tiles.size()) return QRect();
    const int row = i / m_columns - m_firstRow;
    const int shown = m_visibleRows ? m_visibleRows : rows();
    if (row < 0 || row >= shown) return QRect();
    const QSize pic = pictureSize();
    const int w = pic.width() + 2 * kPad + 4, h = pic.height() + (m_names ? fontMetrics().height() + 3 * kPad + 2 : 2 * kPad + 2);
    return QRect(kPad / 2 + (i % m_columns) * w, kPad / 2 + row * h, w, h);
}

QSize PictureGrid::sizeHint() const
{
    const QSize pic = pictureSize();
    const int w = pic.width() + 2 * kPad + 4, h = pic.height() + (m_names ? fontMetrics().height() + 3 * kPad + 2 : 2 * kPad + 2);
    const int shown = std::max(1, m_visibleRows ? m_visibleRows : rows());
    return QSize(w * m_columns + kPad, h * shown + kPad);
}

int PictureGrid::tileAt(const QPoint &p) const
{
    for (int i = 0; i < m_tiles.size(); ++i)
        if (tileRect(i).contains(p)) return i;
    return -1;
}

void PictureGrid::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    for (int i = 0; i < m_tiles.size(); ++i) {
        const QRect tile = tileRect(i);
        if (tile.isEmpty()) continue;
        const QRectF r = QRectF(tile).adjusted(2, 2, -2, -2);
        const bool current = i == m_current, hot = i == m_hover;
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
        const QImage &picture = pictureOf(i);
        const QSize ps = picture.isNull() ? pictureSize() : picture.deviceIndependentSize().toSize();
        const QRect pr(int(r.center().x() - ps.width() / 2.0), int(r.top() + kPad), ps.width(), ps.height());
        if (!picture.isNull()) p.drawImage(pr, picture);
        p.setPen(QPen(uiDark() ? QColor(0x4A, 0x52, 0x5E) : QColor(0xC4, 0xC8, 0xD0), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(pr.adjusted(0, 0, -1, -1));
        // The colors of a variant along the foot of its picture.
        const QVector<QColor> &chips = m_tiles[i].chips;
        if (!chips.isEmpty()) {
            const int gap = 1, h = std::max(3, ps.height() / 9);
            const double w = (ps.width() - 6.0 - gap * (chips.size() - 1)) / chips.size();
            for (int c = 0; c < chips.size(); ++c) {
                const QRectF chip(pr.left() + 3 + c * (w + gap), pr.bottom() - 3 - h, w, h);
                p.setPen(QPen(QColor(255, 255, 255, 190), 0.8));
                p.setBrush(chips[c]);
                p.drawRect(chip);
            }
        }
        if (m_names) {
            p.setPen(uiText());
            const QRectF label(r.left() + 2, pr.bottom() + kPad / 2.0 + 1, r.width() - 4, r.bottom() - pr.bottom() - kPad / 2.0 - 1);
            p.drawText(label, Qt::AlignHCenter | Qt::AlignTop, fontMetrics().elidedText(m_tiles[i].name, Qt::ElideRight, int(label.width())));
        }
    }
    if (hasFocus() && m_hover >= 0 && !tileRect(m_hover).isEmpty()) {
        p.setPen(QPen(focusRingColor(), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(tileRect(m_hover)).adjusted(1, 1, -1, -1), 6, 6);
    }
}

void PictureGrid::mouseMoveEvent(QMouseEvent *e)
{
    const int i = tileAt(e->position().toPoint());
    if (i != m_hover) {
        m_hover = i;
        update();
    }
    if (i >= 0) QToolTip::showText(e->globalPosition().toPoint(), m_tiles[i].name, this);
}

void PictureGrid::leaveEvent(QEvent *)
{
    if (!hasFocus()) m_hover = -1;
    update();
}

void PictureGrid::mousePressEvent(QMouseEvent *e)
{
    const int i = tileAt(e->position().toPoint());
    if (i >= 0 && e->button() == Qt::LeftButton) Q_EMIT chosen(i);
}

void PictureGrid::contextMenuEvent(QContextMenuEvent *e)
{
    int i = e->reason() == QContextMenuEvent::Keyboard ? m_hover : tileAt(e->pos());
    if (i < 0 || i >= m_tiles.size()) return;
    const QPoint at = e->reason() == QContextMenuEvent::Keyboard ? mapToGlobal(tileRect(i).center()) : e->globalPos();
    Q_EMIT contextRequested(i, at);
    e->accept();
}

void PictureGrid::wheelEvent(QWheelEvent *e)
{
    if (m_visibleRows <= 0 || rows() <= m_visibleRows) {
        e->ignore();
        return;
    }
    setFirstRow(m_firstRow + (e->angleDelta().y() < 0 ? 1 : -1));
    e->accept();
}

void PictureGrid::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    if (m_hover < 0) m_hover = m_current >= 0 ? m_current : (m_tiles.isEmpty() ? -1 : 0);
    update();
}

void PictureGrid::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    update();
}

void PictureGrid::keyPressEvent(QKeyEvent *e)
{
    const int n = int(m_tiles.size());
    int i = m_hover < 0 ? 0 : m_hover;
    switch (e->key()) {
    case Qt::Key_Left: i = std::max(0, i - 1); break;
    case Qt::Key_Right: i = std::min(n - 1, i + 1); break;
    case Qt::Key_Up: i = i - m_columns >= 0 ? i - m_columns : i; break;
    case Qt::Key_Down: i = i + m_columns < n ? i + m_columns : i; break;
    case Qt::Key_Home: i = 0; break;
    case Qt::Key_End: i = n - 1; break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        if (m_hover >= 0 && m_hover < n) Q_EMIT chosen(m_hover);
        return;
    default: QWidget::keyPressEvent(e); return;
    }
    m_hover = std::clamp(i, 0, std::max(0, n - 1));
    // A row out of view scrolls into it.
    if (m_visibleRows > 0) {
        const int row = m_hover / m_columns;
        if (row < m_firstRow) setFirstRow(row);
        else if (row >= m_firstRow + m_visibleRows) setFirstRow(row - m_visibleRows + 1);
    }
    update();
}

// ---------------- ThemeGallery ----------------
ThemeGallery::ThemeGallery(Editor *ed, QWidget *parent) : QFrame(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("themeGallery"));
    setFrameShape(QFrame::NoFrame);
    setAccessibleName(tr("Themes"));
    setAccessibleDescription(tr("The arrow keys move between themes, and Enter applies one to every slide."));
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);
    m_grid = new PictureGrid(78, 4, false, this);
    m_grid->setObjectName(QStringLiteral("themeGrid"));
    m_grid->setVisibleRows(1);
    m_grid->setAccessibleName(tr("Themes"));
    setUp(m_grid);
    connect(m_grid, &PictureGrid::chosen, this, [this](int theme) { Q_EMIT themeChosen(theme, true); });
    row->addWidget(m_grid);
    auto *side = new QVBoxLayout();
    side->setSpacing(3);
    side->setContentsMargins(0, 1, 0, 1);
    auto button = [&](const QString &iconName, const QString &tip, const std::function<void()> &fn) {
        auto *b = new QToolButton(this);
        b->setIcon(icon(iconName));
        b->setIconSize(QSize(12, 12));
        b->setAutoRaise(true);
        b->setFixedSize(20, 20);
        b->setToolTip(tip);
        b->setAccessibleName(tip);
        b->setFocusPolicy(Qt::TabFocus);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QStringLiteral("QToolButton{border:none; border-radius:6px; background:transparent;}"
                                        "QToolButton:hover{background:rgba(127,127,127,0.18);}"));
        connect(b, &QToolButton::clicked, this, fn);
        side->addWidget(b);
        return b;
    };
    m_up = button(QStringLiteral("chevron-up"), tr("Previous row"), [this] { m_grid->setFirstRow(m_grid->firstRow() - 1); });
    m_down = button(QStringLiteral("chevron-down"), tr("Next row"), [this] { m_grid->setFirstRow(m_grid->firstRow() + 1); });
    button(QStringLiteral("layout-grid"), tr("All themes"), [this] { openAll(); });
    row->addLayout(side);
    reload();
}

// The tiles of the built-in themes in a grid, marked for the presentation's theme.
void ThemeGallery::setUp(PictureGrid *grid)
{
    QVector<PictureGrid::Tile> tiles;
    for (int i = 0; i < deck::builtinThemeCount(); ++i) tiles << PictureGrid::Tile{deck::builtinThemeName(i), {}};
    grid->setTiles(tiles);
    grid->setPictures([](int theme, int width, qreal dpr) { return themePicture(theme, 0, width, dpr); });
    const int current = m_ed && m_ed->doc() ? deck::builtinThemeIndex(deck::themeOf(*m_ed->doc()).name) : -1;
    grid->setCurrent(current);
    connect(grid, &PictureGrid::contextRequested, this, [this](int theme, const QPoint &at) {
        QMenu *menu = contextMenu(theme, this);
        menu->exec(at);
        delete menu;
    });
}

void ThemeGallery::reload()
{
    const Document *d = m_ed->doc();
    const int now = d ? deck::builtinThemeIndex(deck::themeOf(*d).name) : -1;
    if (now != m_grid->current() && now >= 0) {
        // The row it is in comes into view.
        const int row = now / m_grid->columns();
        if (row < m_grid->firstRow() || row >= m_grid->firstRow() + std::max(1, m_grid->visibleRows())) m_grid->setFirstRow(row);
    }
    m_grid->setCurrent(now);
}

QMenu *ThemeGallery::contextMenu(int theme, QWidget *parent)
{
    auto *menu = new QMenu(parent);
    menu->addAction(tr("Apply to All Slides"), this, [this, theme] { Q_EMIT themeChosen(theme, true); });
    menu->addAction(tr("Apply to Selected Slides"), this, [this, theme] { Q_EMIT themeChosen(theme, false); });
    menu->addSeparator();
    menu->addAction(icon(QStringLiteral("folder-open")), tr("Browse for Themes…"), this, [this] { Q_EMIT browseRequested(); });
    return menu;
}

QFrame *ThemeGallery::openAll()
{
    auto *pop = new QFrame(this, Qt::Popup);
    pop->setObjectName(QStringLiteral("themePopup"));
    pop->setAttribute(Qt::WA_DeleteOnClose);
    pop->setFrameShape(QFrame::StyledPanel);
    auto *column = new QVBoxLayout(pop);
    column->setContentsMargins(4, 4, 4, 4);
    column->setSpacing(2);
    auto *title = new QLabel(tr("Built-In"), pop);
    QFont f = title->font();
    f.setBold(true);
    title->setFont(f);
    title->setStyleSheet(QStringLiteral("padding:3px 2px;"));
    column->addWidget(title);
    auto *all = new PictureGrid(112, 3, true, pop);
    all->setAccessibleName(tr("Themes"));
    setUp(all);
    column->addWidget(all);
    auto *browse = new QPushButton(icon(QStringLiteral("folder-open")), tr("Browse for Themes…"), pop);
    browse->setFlat(true);
    browse->setStyleSheet(QStringLiteral("text-align:left; padding:4px 6px;"));
    column->addWidget(browse);
    connect(all, &PictureGrid::chosen, pop, [this, pop](int theme) {
        pop->close();
        Q_EMIT themeChosen(theme, true);
    });
    connect(browse, &QPushButton::clicked, pop, [this, pop] {
        pop->close();
        Q_EMIT browseRequested();
    });
    pop->adjustSize();
    pop->move(mapToGlobal(QPoint(0, height())));
    pop->show();
    all->setFocus();
    return pop;
}

// ---------------- VariantGallery ----------------
VariantGallery::VariantGallery(Editor *ed, QWidget *parent) : QFrame(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("themeVariants"));
    setFrameShape(QFrame::NoFrame);
    setAccessibleName(tr("Variants"));
    setAccessibleDescription(tr("The arrow keys move between the variants of this theme, and Enter applies one."));
    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);
    m_grid = new PictureGrid(60, 4, false, this);
    m_grid->setObjectName(QStringLiteral("variantGrid"));
    m_grid->setAccessibleName(tr("Variants"));
    m_grid->setPictures([this](int variant, int width, qreal dpr) {
        const Document *d = m_ed->doc();
        if (!d) return QImage();
        if (const int theme = deck::builtinThemeIndex(deck::themeOf(*d).name); theme >= 0) return themePicture(theme, variant, width, dpr);
        return firstSlide(*deck::variantSample(*d, variant), width, dpr);
    });
    connect(m_grid, &PictureGrid::chosen, this, &VariantGallery::variantChosen);
    row->addWidget(m_grid);
    m_more = new QToolButton(this);
    m_more->setObjectName(QStringLiteral("variantsMore"));
    m_more->setIcon(icon(QStringLiteral("chevron-down")));
    m_more->setIconSize(QSize(12, 12));
    m_more->setAutoRaise(true);
    m_more->setFixedWidth(20);
    m_more->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_more->setToolTip(tr("Colors, fonts, and background styles"));
    m_more->setAccessibleName(tr("Colors, fonts, and background styles"));
    m_more->setPopupMode(QToolButton::InstantPopup);
    m_more->setStyleSheet(QStringLiteral("QToolButton{border:none; border-radius:6px; background:transparent;} QToolButton::menu-indicator{image:none;}"
                                         "QToolButton:hover{background:rgba(127,127,127,0.18);}"));
    row->addWidget(m_more);
    reload();
}

void VariantGallery::setMenu(QMenu *menu) { m_more->setMenu(menu); }

void VariantGallery::reload()
{
    const Document *d = m_ed->doc();
    if (!d) return;
    const QString signature = signatureOf(*d);
    if (signature == m_signature) return;
    m_signature = signature;
    const QVector<deck::Variant> variants = deck::variantsOf(*d);
    {
        QVector<PictureGrid::Tile> tiles;
        for (const deck::Variant &v : variants) {
            PictureGrid::Tile t;
            t.name = tr("%1, %2").arg(v.colors.name, deck::backgroundStyleName(v.background));
            for (int slot = 4; slot < 10; ++slot) t.chips << v.colors.colors[slot];
            tiles << t;
        }
        m_grid->setTiles(tiles);
        m_grid->reload();
    }
    m_grid->setCurrent(deck::currentVariant(*d));
}

// ---------------- BackgroundStyles ----------------
namespace {
constexpr int kSwatchW = 40, kSwatchH = 28, kSwatchGap = 6;
}

BackgroundStyles::BackgroundStyles(Editor *ed, QWidget *parent) : QWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("backgroundStyles"));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Background styles"));
    setAccessibleDescription(tr("The arrow keys move between the twelve styles, and Enter applies one."));
}

QSize BackgroundStyles::sizeHint() const { return QSize(4 * (kSwatchW + kSwatchGap) + kSwatchGap, 3 * (kSwatchH + kSwatchGap) + kSwatchGap); }

// The four colors across, each flat, soft, and strong going down.
QRect BackgroundStyles::swatchRect(int style) const
{
    return QRect(kSwatchGap + (style / 3) * (kSwatchW + kSwatchGap), kSwatchGap + (style % 3) * (kSwatchH + kSwatchGap), kSwatchW, kSwatchH);
}

int BackgroundStyles::styleAt(const QPoint &p) const
{
    for (int s = 0; s < deck::kBackgroundStyles; ++s)
        if (swatchRect(s).contains(p)) return s;
    return -1;
}

int BackgroundStyles::current() const
{
    const Document *d = m_ed->doc();
    if (!d) return -1;
    for (const auto &m : d->masters)
        if (deck::isSlideMaster(*m)) return deck::backgroundStyleOf(m->background, deck::themeOf(*d));
    return -1;
}

void BackgroundStyles::paintEvent(QPaintEvent *)
{
    const Document *d = m_ed->doc();
    if (!d) return;
    const deck::DeckTheme theme = deck::themeOf(*d);
    const int now = current();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    for (int style = 0; style < deck::kBackgroundStyles; ++style) {
        const QRectF r(swatchRect(style));
        const Fill f = deck::backgroundStyle(theme, style);
        p.setPen(QPen(uiDark() ? QColor(0x5A, 0x61, 0x6D) : QColor(0xB8, 0xBE, 0xC8), 1));
        p.setBrush(f.brush(r, d->colors));
        p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
        if (style == now) {
            p.setPen(QPen(uiAccent(), 2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(-2, -2, 2, 2), 4, 4);
        } else if (style == m_hover) {
            p.setPen(QPen(uiAccent(), 1));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(-2, -2, 2, 2), 4, 4);
        }
    }
    if (hasFocus() && m_hover >= 0) {
        p.setPen(QPen(focusRingColor(), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(swatchRect(m_hover)).adjusted(-3, -3, 3, 3), 5, 5);
    }
}

void BackgroundStyles::mouseMoveEvent(QMouseEvent *e)
{
    const int style = styleAt(e->position().toPoint());
    if (style != m_hover) {
        m_hover = style;
        update();
    }
}

void BackgroundStyles::leaveEvent(QEvent *)
{
    if (!hasFocus()) m_hover = -1;
    update();
}

void BackgroundStyles::mousePressEvent(QMouseEvent *e)
{
    const int style = styleAt(e->position().toPoint());
    if (style >= 0 && e->button() == Qt::LeftButton) Q_EMIT chosen(style);
}

void BackgroundStyles::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    if (m_hover < 0) m_hover = 0;
    update();
}

void BackgroundStyles::keyPressEvent(QKeyEvent *e)
{
    int i = m_hover < 0 ? 0 : m_hover;
    switch (e->key()) {
    case Qt::Key_Left: i = i >= 3 ? i - 3 : i; break;
    case Qt::Key_Right: i = i < 9 ? i + 3 : i; break;
    case Qt::Key_Up: i = i % 3 ? i - 1 : i; break;
    case Qt::Key_Down: i = i % 3 < 2 ? i + 1 : i; break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        if (m_hover >= 0) Q_EMIT chosen(m_hover);
        return;
    default: QWidget::keyPressEvent(e); return;
    }
    m_hover = i;
    update();
}

bool BackgroundStyles::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        const auto *he = static_cast<QHelpEvent *>(e);
        const int style = styleAt(he->pos());
        if (style >= 0) QToolTip::showText(he->globalPos(), deck::backgroundStyleName(style), this);
        else QToolTip::hideText();
        return true;
    }
    return QWidget::event(e);
}

} // namespace jp
