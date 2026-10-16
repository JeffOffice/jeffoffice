#include "render/svgexport.h"

#include "core/document.h"

#include <QBuffer>
#include <QFontMetricsF>
#include <QPaintEngine>
#include <QPainter>
#include <QSaveFile>
#include <QSvgGenerator>

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

// Qt's SVG writer turns text into <text> elements, split where the layout
// split the line (at hyphenation points) and spaced by whatever font the
// viewer has. This engine sits in between: it hands every drawing step to
// the SVG writer as it is, except text, which the base class turns into
// the letters' outlines (a fillPath, arriving here as drawPath).
class OutlineEngine : public QPaintEngine {
public:
    OutlineEngine(QPainter *out, bool withText) : QPaintEngine(QPaintEngine::AllFeatures), m_out(out), m_withText(withText) {}
    bool begin(QPaintDevice *) override { return true; }
    Type type() const override { return QPaintEngine::User; }
    // The state is read from the painter at each step instead.
    void updateState(const QPaintEngineState &) override {}

    // The letters as outlines, and with `withText`, the text too with no ink (a text element that shows nothing), for what draws the drawing into a
    // PDF to say the words as text. The pieces of a line (the layout cuts a run where kerning or spacing changes) are said as one text when they
    // follow each other along a baseline, so the words are whole.
    void drawTextItem(const QPointF &at, const QTextItem &item) override
    {
        QPaintEngine::drawTextItem(at, item);
        const QString text = item.text();
        if (!m_withText || text.trimmed().isEmpty()) return;
        const QFont font = item.font();
        const QTransform transform = painter()->transform();
        if (m_said.valid) {
            // (The layout sets sizes in points; the device is 72 to the inch, so a point is a unit.)
            const double size = font.pixelSize() > 0 ? font.pixelSize() : std::max(1.0, font.pointSizeF());
            const double gap = at.x() - m_said.end;
            const bool same = m_said.transform == transform && m_said.font.family() == font.family() && m_said.font.pixelSize() == font.pixelSize() && m_said.font.pointSizeF() == font.pointSizeF() &&
                              m_said.font.weight() == font.weight() && m_said.font.italic() == font.italic() && std::abs(at.y() - m_said.at.y()) < 0.02 * size && gap >= -0.15 * size &&
                              gap <= 0.35 * size;
            if (same) {
                m_said.text += text;
                m_said.end = at.x() + QFontMetricsF(font, painter()->device()).horizontalAdvance(text);
                return;
            }
            sayText();
        }
        m_said = {true, text, font, transform, at, at.x() + QFontMetricsF(font, painter()->device()).horizontalAdvance(text)};
    }
    bool end() override
    {
        sayText();
        return true;
    }
    void drawPath(const QPainterPath &path) override { sync(); m_out->drawPath(path); }
    void drawPolygon(const QPointF *points, int count, PolygonDrawMode mode) override
    {
        sync();
        if (mode == PolylineMode) m_out->drawPolyline(points, count);
        else m_out->drawPolygon(points, count, mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
    }
    void drawRects(const QRectF *rects, int count) override { sync(); m_out->drawRects(rects, count); }
    void drawLines(const QLineF *lines, int count) override { sync(); m_out->drawLines(lines, count); }
    void drawEllipse(const QRectF &r) override { sync(); m_out->drawEllipse(r); }
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override { sync(); m_out->drawPixmap(r, pm, sr); }
    void drawImage(const QRectF &r, const QImage &img, const QRectF &sr, Qt::ImageConversionFlags) override { sync(); m_out->drawImage(r, img, sr); }
    void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &at) override { sync(); m_out->drawTiledPixmap(r, pm, at); }

private:
    // The text held back (the line so far) is drawn into the file, with no ink.
    void sayText()
    {
        if (!m_said.valid) return;
        m_said.valid = false;
        sync();
        m_out->setTransform(m_said.transform);
        m_out->setPen(QColor(0, 0, 0, 0));
        m_out->setFont(m_said.font);
        m_out->drawText(m_said.at, m_said.text);
    }
    void sync()
    {
        const QPainter *src = painter();
        m_out->setTransform(src->transform());
        m_out->setPen(src->pen());
        m_out->setBrush(src->brush());
        m_out->setBrushOrigin(src->brushOrigin());
        m_out->setOpacity(src->opacity());
        m_out->setRenderHints(src->renderHints());
        // The clip is set again only when it changed, as each one becomes
        // a clip path in the file.
        const bool clipping = src->hasClipping();
        const QPainterPath clip = clipping ? src->transform().map(src->clipPath()) : QPainterPath();
        if (clipping != m_clipping || clip != m_clip) {
            m_clipping = clipping;
            m_clip = clip;
            if (clipping) {
                m_out->setClipPath(src->clipPath());
            } else {
                m_out->setClipping(false);
            }
        }
    }

    QPainter *m_out;
    bool m_withText;
    struct Said {
        bool valid = false;
        QString text;
        QFont font;
        QTransform transform;
        QPointF at;
        double end = 0;   // where the last piece of it ends along the baseline
    } m_said;
    bool m_clipping = false;
    QPainterPath m_clip;
};

class OutlineDevice : public QPaintDevice {
public:
    OutlineDevice(QPainter *out, const QSize &size, bool withText) : m_engine(out, withText), m_size(size) {}
    QPaintEngine *paintEngine() const override { return &m_engine; }

protected:
    int metric(PaintDeviceMetric m) const override
    {
        switch (m) {
        case PdmWidth: return m_size.width();
        case PdmHeight: return m_size.height();
        case PdmWidthMM: return qRound(m_size.width() * 25.4 / 72);
        case PdmHeightMM: return qRound(m_size.height() * 25.4 / 72);
        case PdmDpiX: case PdmDpiY: case PdmPhysicalDpiX: case PdmPhysicalDpiY: return 72;
        case PdmDepth: return 32;
        case PdmNumColors: return INT_MAX;
        case PdmDevicePixelRatio: return 1;
        case PdmDevicePixelRatioScaled: return int(devicePixelRatioFScale());
        default: return QPaintDevice::metric(m);
        }
    }

private:
    mutable OutlineEngine m_engine;
    QSize m_size;
};

} // namespace

QByteArray pageSvg(const PaintContext &ctx, int pageIndex, const QString &title, bool withText)
{
    const QSizeF size = ctx.doc->pageSize();
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    // SVG 1.1, as the clip paths that crop pictures need it.
    QSvgGenerator gen(QSvgGenerator::SvgVersion::Svg11);
    gen.setOutputDevice(&buf);
    gen.setResolution(72);
    gen.setSize(size.toSize());
    gen.setViewBox(QRectF(QPointF(0, 0), size));
    gen.setTitle(title);
    gen.setDescription(QStringLiteral("Made with JeffPub"));
    {
        QPainter out(&gen);
        OutlineDevice dev(&out, size.toSize(), withText);
        QPainter p(&dev);
        Renderer::paintPage(&p, ctx, pageIndex);
    }
    // The text elements there are, with no ink, are for the words only: the renderer that draws the drawing leaves them out (display none), and
    // whoever says the words (svg::paintText) knows them by their class.
    if (withText) {
        QByteArray svg = buf.data();
        svg.replace("<text ", "<text class=\"jo-words\" display=\"none\" ");
        return svg;
    }
    return buf.data();
}

bool writePageSvg(const PaintContext &ctx, int pageIndex, const QString &path, const QString &title, QString *error)
{
    const QByteArray svg = pageSvg(ctx, pageIndex, title);
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(svg) != svg.size() || !f.commit()) {
        if (error) *error = f.errorString();
        return false;
    }
    return true;
}

} // namespace jp
