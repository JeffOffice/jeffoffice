#include "app/slideshow.h"

#include "core/document.h"
#include "render/renderer.h"

#include <QKeyEvent>
#include <QPainter>

namespace jp {

SlideShow::SlideShow(const Document *doc, LayoutCache *cache, int startSlide, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint), m_doc(doc), m_cache(cache)
{
    setObjectName(QStringLiteral("slideShow"));
    setAccessibleName(tr("Slide Show"));
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::BlankCursor);
    m_slide = nextShown(std::clamp(startSlide, 0, std::max(0, int(doc->pages.size()) - 1)), 1);
    if (m_slide < 0) m_atEnd = true;
}

int SlideShow::nextShown(int from, int step) const
{
    for (int i = from; i >= 0 && i < m_doc->pages.size(); i += step)
        if (!m_doc->pages[i]->hidden) return i;
    return -1;
}

void SlideShow::next()
{
    m_blank = QColor();
    if (m_atEnd) {
        close();
        return;
    }
    const int n = nextShown(m_slide + 1, 1);
    if (n < 0) m_atEnd = true;
    else m_slide = n;
    update();
}

void SlideShow::previous()
{
    m_blank = QColor();
    if (m_atEnd) m_atEnd = false;
    else if (const int p = nextShown(m_slide - 1, -1); p >= 0) m_slide = p;
    update();
}

void SlideShow::goTo(int slide)
{
    if (slide < 0 || slide >= m_doc->pages.size()) return;
    m_blank = QColor();
    m_atEnd = false;
    m_slide = slide;
    update();
}

void SlideShow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), m_blank.isValid() ? m_blank : QColor(Qt::black));
    if (m_blank.isValid()) return;
    if (m_atEnd) {
        p.setPen(Qt::white);
        QFont f = font();
        f.setPointSizeF(16);
        p.setFont(f);
        p.drawText(rect().adjusted(0, 24, 0, 0), Qt::AlignHCenter | Qt::AlignTop, tr("End of slide show, click to exit."));
        return;
    }
    const QSizeF page = m_doc->pageSize();
    const double scale = std::min(width() / page.width(), height() / page.height());
    const QSizeF shown = page * scale;
    p.translate((width() - shown.width()) / 2, (height() - shown.height()) / 2);
    p.scale(scale, scale);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipRect(QRectF(QPointF(0, 0), page));
    p.fillRect(QRectF(QPointF(0, 0), page), Qt::white);
    PaintContext ctx;
    ctx.doc = m_doc;
    ctx.cache = m_cache;
    Renderer::paintPage(&p, ctx, m_slide);
}

void SlideShow::keyPressEvent(QKeyEvent *e)
{
    const int k = e->key();
    if (k >= Qt::Key_0 && k <= Qt::Key_9) {
        m_typed += QChar('0' + (k - Qt::Key_0));
        return;
    }
    if ((k == Qt::Key_Return || k == Qt::Key_Enter) && !m_typed.isEmpty()) {
        goTo(m_typed.toInt() - 1);
        m_typed.clear();
        return;
    }
    m_typed.clear();
    switch (k) {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Right: case Qt::Key_Down: case Qt::Key_Space: case Qt::Key_Return: case Qt::Key_Enter:
    case Qt::Key_PageDown: case Qt::Key_N:
        next();
        break;
    case Qt::Key_Left: case Qt::Key_Up: case Qt::Key_Backspace: case Qt::Key_PageUp: case Qt::Key_P:
        previous();
        break;
    case Qt::Key_Home:
        goTo(nextShown(0, 1));
        break;
    case Qt::Key_End:
        goTo(nextShown(int(m_doc->pages.size()) - 1, -1));
        break;
    case Qt::Key_B: case Qt::Key_Period:
        m_blank = m_blank == QColor(Qt::black) ? QColor() : QColor(Qt::black);
        update();
        break;
    case Qt::Key_W: case Qt::Key_Comma:
        m_blank = m_blank == QColor(Qt::white) ? QColor() : QColor(Qt::white);
        update();
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}

void SlideShow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) next();
    else if (e->button() == Qt::RightButton) previous();
}

} // namespace jp
