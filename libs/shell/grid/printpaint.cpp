#include "grid/printpaint.h"

#include <QBuffer>
#include <QImageReader>

#include <QFontMetricsF>
#include <QPen>

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::columnName;
using jp::office::sheet::HeaderFooter;
using jp::office::sheet::HfContext;
using jp::office::sheet::HfField;
using jp::office::sheet::HfLine;
using jp::office::sheet::HfRun;
using jp::office::sheet::HfText;
using jp::office::sheet::HfUnderline;

namespace {

// The zoom the grid is painted at for a scale (a fraction): large enough that the rounding of the cells' places to whole units of the painter is a small part of a point, and within what the
// grid allows.
double zoomFor(double scale) { return std::min(GridLayout::kMaxZoom, std::max(GridLayout::kMinZoom, scale * 8)); }

qint64 extentOf(const Axis &axis, qint64 first, qint64 last) { return first < 0 || last < first ? 0 : axis.position(last + 1) - axis.position(first); }

} // namespace

PageMetrics pageMetrics(const SheetPrint &print, const PageCut &page, const PageSetup &setup, const Axis &rows, const Axis &cols)
{
    PageMetrics m;
    const double s = print.scalePercent / 100.0;
    m.zoom = zoomFor(s);
    m.unit = kPointsPerPixel * s / m.zoom;
    // (the title rows and columns are measured between rows that the axis keeps, so a skip set for drawing does not matter: only differences are taken)
    const qint64 widthPx = extentOf(cols, page.titleCol1, page.titleCol2) + extentOf(cols, page.col1, page.col2);
    const qint64 heightPx = extentOf(rows, page.titleRow1, page.titleRow2) + extentOf(rows, page.row1, page.row2);
    m.widthUnits = qint64(std::llround(double(widthPx) * m.zoom));
    m.heightUnits = qint64(std::llround(double(heightPx) * m.zoom));
    const double pt = kPointsPerPixel * s;
    const double headW = setup.headings ? kRowHeadingPixels * pt : 0, headH = setup.headings ? kColumnHeadingPixels * pt : 0;
    const double totalW = headW + double(widthPx) * pt, totalH = headH + double(heightPx) * pt;
    const QRectF box = print.paper.content();
    const double x0 = box.left() + (setup.centerHorizontally ? std::max(0.0, (box.width() - totalW) / 2) : 0);
    const double y0 = box.top() + (setup.centerVertically ? std::max(0.0, (box.height() - totalH) / 2) : 0);
    m.cells = QRectF(x0 + headW, y0 + headH, double(widthPx) * pt, double(heightPx) * pt);
    if (setup.headings) {
        m.headingColumn = QRectF(x0, y0 + headH, headW, double(heightPx) * pt);
        m.headingRow = QRectF(x0 + headW, y0, double(widthPx) * pt, headH);
    }
    return m;
}

PagePainter::PagePainter(GridSheet *sheet) : m_sheet(sheet), m_painter(std::make_unique<GridPainter>(sheet))
{
    m_base.setFamily(QStringLiteral("Calibri"));
    m_base.setPointSizeF(11);
}

PagePainter::~PagePainter() = default;

void PagePainter::setBaseFont(const QString &family, double points)
{
    if (!family.isEmpty()) m_base.setFamily(family);
    if (points > 0) m_base.setPointSizeF(std::min(points, 409.0));
}

void PagePainter::paint(QPainter *p, const SheetPrint &print, const PageCut &page, const PageSetup &setup, const HfContext &labels, bool firstOfSheet, const PageObjects &objects)
{
    GridLayout &L = m_sheet->layout();
    const bool titleRows = page.titleRow1 >= 0, titleCols = page.titleCol1 >= 0;
    // ---- the cells: the title rows and columns are the frozen part, the page's own rows and columns are what is scrolled to
    L.setSkipped(titleRows ? page.titleRow1 : 0, titleCols ? page.titleCol1 : 0);
    L.setFrozen(titleRows ? page.titleRow2 + 1 : 0, titleCols ? page.titleCol2 + 1 : 0);
    L.setHeaders(false, 0, 0);
    const PageMetrics m = pageMetrics(print, page, setup, L.rows, L.cols);
    L.setZoom(m.zoom);
    L.setViewport(QSize(int(std::min<qint64>(m.widthUnits, 1 << 28)), int(std::min<qint64>(m.heightUnits, 1 << 28))));
    L.setScroll(L.cols.position(page.col1) - L.cols.position(L.frozenCols()), L.rows.position(page.row1) - L.rows.position(L.frozenRows()));
    m_sheet->invalidateTexts();
    PaintOptions options;
    options.headers = false;
    options.gridlines = setup.gridLines && !setup.draft;
    options.gridColor = setup.blackAndWhite ? QColor(Qt::black) : m_gridline;
    options.backdrop = Qt::white;
    options.paper = Qt::white;
    options.monochrome = setup.blackAndWhite;
    options.strokedDashes = true;
    const double scale = print.scalePercent / 100.0;
    p->save();
    p->translate(m.cells.left(), m.cells.top());
    p->scale(m.unit, m.unit);
    const QRect cells(0, 0, int(m.widthUnits), int(m.heightUnits));
    p->setClipRect(cells);
    m_painter->paint(p, cells, options);
    if (objects && !setup.draft) {
        p->setClipRect(cells);
        objects(p, L, cells);
    }
    p->restore();
    if (setup.headings) paintHeadings(p, L, page, m, scale);
    paintHeaderFooter(p, print.paper, setup, labels, firstOfSheet, scale);
}

void PagePainter::paintHeadings(QPainter *p, const GridLayout &layout, const PageCut &page, const PageMetrics &m, double scale)
{
    p->save();
    const double U = 8;   // the units of the text: an eighth of a point, so that a font's size is a whole number of them
    const double pt = kPointsPerPixel * scale;
    QFont font = m_base;
    font.setPixelSize(std::max(1, int(std::lround(std::min(m_base.pointSizeF(), 11.0) * scale * U))));
    const QColor ink = Qt::black, fill(0xE6, 0xE6, 0xE6);
    const QPen line(ink, std::max(0.25, 0.5 * scale));
    auto strip = [&](bool rowsStrip) {
        const qint64 first1 = rowsStrip ? page.titleRow1 : page.titleCol1, last1 = rowsStrip ? page.titleRow2 : page.titleCol2;
        const qint64 first2 = rowsStrip ? page.row1 : page.col1, last2 = rowsStrip ? page.row2 : page.col2;
        const Axis &axis = rowsStrip ? layout.rows : layout.cols;
        const qint64 frozen = rowsStrip ? layout.frozenRows() : layout.frozenCols();
        auto one = [&](qint64 i) {
            const bool fz = i < frozen;
            const int a = rowsStrip ? layout.yEdge(i, fz) : layout.xEdge(i, fz), b = rowsStrip ? layout.yEdge(i + 1, fz) : layout.xEdge(i + 1, fz);
            if (b <= a) return;
            // (the first body row's edge is measured from the frozen part: the layout puts it right after, in the painter's units)
            const double start = (rowsStrip ? m.cells.top() : m.cells.left()) + a * m.unit, size = (b - a) * m.unit;
            const QRectF r = rowsStrip ? QRectF(m.headingColumn.left(), start, kRowHeadingPixels * pt, size) : QRectF(start, m.headingRow.top(), size, kColumnHeadingPixels * pt);
            p->fillRect(r, fill);
            p->setPen(line);
            p->drawRect(r);
            const QString text = rowsStrip ? QString::number(i + 1) : columnName(quint32(i));
            p->save();
            p->scale(1 / U, 1 / U);
            p->setFont(font);
            p->setPen(ink);
            p->drawText(QRectF(r.left() * U, r.top() * U, r.width() * U, r.height() * U), Qt::AlignCenter | Qt::TextSingleLine, text);
            p->restore();
        };
        if (first1 >= 0)
            for (qint64 i = first1; i <= last1; i = axis.nextVisible(i)) {
                if (i >= axis.count()) break;
                one(i);
            }
        for (qint64 i = first2; i <= last2; i = axis.nextVisible(i)) {
            if (i >= axis.count()) break;
            one(i);
        }
    };
    if (!m.headingColumn.isEmpty()) strip(true);
    if (!m.headingRow.isEmpty()) strip(false);
    p->restore();
}

// The picture decoded, kept while the painter draws the sheet's pages (the picture is the setup's, which outlives them): at most a few thousand pixels across, and not at all when it is huge.
QImage PagePainter::headerPicture(const jp::office::sheet::HeaderPicture &pic)
{
    const std::pair<const void *, qsizetype> key(pic.image.constData(), pic.image.size());
    const auto have = m_pictures.find(key);
    if (have != m_pictures.end()) return have->second;
    QImage image;
    if (m_pictures.size() < size_t(jp::office::sheet::kMaxHeaderPictures * 4)) {
        QBuffer buffer;
        buffer.setData(pic.image);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer);
        const QSize size = reader.size();
        if (size.isValid() && qint64(size.width()) * size.height() <= jp::office::sheet::kMaxHeaderPicturePixels && reader.read(&image)) {
            const int most = 2400;   // (a page's width at 300 to the inch)
            if (image.width() > most || image.height() > most) image = image.scaled(most, most, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    m_pictures.emplace(key, image);
    return image;
}

void PagePainter::paintHeaderFooter(QPainter *p, const PaperBox &paper, const PageSetup &setup, const HfContext &labels, bool firstOfSheet, double scale)
{
    const HeaderFooter &hf = setup.headerFooter;
    for (bool header : {true, false}) {
        QString code;
        if (hf.differentFirst && firstOfSheet) code = header ? hf.firstHeader : hf.firstFooter;
        else if (hf.differentOddEven && labels.page % 2 == 0) code = header ? hf.evenHeader : hf.evenFooter;
        else code = header ? hf.oddHeader : hf.oddFooter;
        if (!code.isEmpty()) paintHeaderFooterText(p, code, paper, setup, labels, header, scale, hf.differentFirst && firstOfSheet, hf.differentOddEven && labels.page % 2 == 0 && !(hf.differentFirst && firstOfSheet));
    }
}

void PagePainter::paintHeaderFooterText(QPainter *p, const QString &code, const PaperBox &paper, const PageSetup &setup, const HfContext &labels, bool header, double scale, bool first, bool even)
{
    const HfText text = jp::office::sheet::parseHeaderFooter(code);
    if (text.isEmpty()) return;
    const HeaderFooter &hf = setup.headerFooter;
    const double U = 8;   // an eighth of a point
    const double sizeFactor = hf.scaleWithDoc ? scale : 1.0;
    const double left = (hf.alignWithMargins ? paper.left : std::min(paper.left, 18.0)) * U, right = (paper.width - (hf.alignWithMargins ? paper.right : std::min(paper.right, 18.0))) * U;
    struct Piece {
        QString text;
        QFont font;
        QColor color;
        double width, shift;
        QImage picture;          // a picture (`&G`): drawn in place of the text, its height above the line's baseline
        double pictureHeight = 0;
    };
    struct Line {
        std::vector<Piece> pieces;
        double width = 0, height = 0, ascent = 0, descent = 0;
    };
    auto layoutPart = [&](const jp::office::sheet::HfPart &part, QChar which) {
        std::vector<Line> lines;
        for (const HfLine &src : part.lines) {
            Line line;
            for (const HfRun &run : src) {
                Piece piece;
                if (run.field == HfField::Picture) {
                    // (the picture of this part, at the size the file gives it; none, or one that can't be read, takes no room)
                    const jp::office::sheet::HeaderPicture *pic = hf.pictureFor(which, header, first, even);
                    if (!pic) continue;
                    piece.picture = headerPicture(*pic);
                    if (piece.picture.isNull()) continue;
                    piece.width = pic->width * sizeFactor * U;
                    piece.pictureHeight = pic->height * sizeFactor * U;
                    piece.shift = 0;
                    piece.color = Qt::black;
                    line.width += piece.width;
                    line.ascent = std::max(line.ascent, piece.pictureHeight);
                    line.pieces.push_back(piece);
                    continue;
                }
                piece.text = run.field == HfField::None ? run.text : jp::office::sheet::hfFieldText(run.field, labels);
                double size = (run.size > 0 ? run.size : m_base.pointSizeF()) * sizeFactor;
                piece.shift = 0;
                if (run.superscript || run.subscript) size *= 0.65;
                piece.font = m_base;
                if (!run.font.isEmpty()) piece.font.setFamily(run.font);
                piece.font.setPixelSize(std::max(1, int(std::lround(size * U))));
                piece.font.setBold(run.bold);
                piece.font.setItalic(run.italic);
                piece.font.setStrikeOut(run.strike);
                piece.font.setUnderline(run.underline != HfUnderline::None);
                piece.color = (run.hasColor && !setup.blackAndWhite) ? run.color : QColor(Qt::black);
                const QFontMetricsF fm(piece.font);
                piece.width = fm.horizontalAdvance(piece.text);
                if (run.superscript) piece.shift = -fm.ascent() * 0.5;
                if (run.subscript) piece.shift = fm.descent() * 0.9;
                line.width += piece.width;
                line.ascent = std::max(line.ascent, fm.ascent() - std::min(0.0, piece.shift));
                line.descent = std::max(line.descent, fm.descent() + std::max(0.0, piece.shift));
                line.pieces.push_back(piece);
            }
            if (line.pieces.empty()) {
                QFont f = m_base;
                f.setPixelSize(std::max(1, int(std::lround(m_base.pointSizeF() * sizeFactor * U))));
                const QFontMetricsF fm(f);
                line.ascent = fm.ascent();
                line.descent = fm.descent();
            }
            line.height = line.ascent + line.descent;
            lines.push_back(line);
        }
        return lines;
    };
    p->save();
    p->scale(1 / U, 1 / U);
    auto drawPart = [&](const jp::office::sheet::HfPart &part, int align) {
        const std::vector<Line> lines = layoutPart(part, align == 0 ? QLatin1Char('L') : align == 1 ? QLatin1Char('C') : QLatin1Char('R'));
        double total = 0;
        for (const Line &l : lines) total += l.height;
        double y = header ? paper.header * U : (paper.height - paper.footer) * U - total;
        for (const Line &l : lines) {
            double x = align == 0 ? left : align == 1 ? (left + right - l.width) / 2 : right - l.width;
            for (const Piece &piece : l.pieces) {
                if (!piece.picture.isNull()) {
                    p->drawImage(QRectF(x, y + l.ascent - piece.pictureHeight, piece.width, piece.pictureHeight), piece.picture);
                    x += piece.width;
                    continue;
                }
                p->setFont(piece.font);
                p->setPen(piece.color);
                p->drawText(QPointF(x, y + l.ascent + piece.shift), piece.text);
                x += piece.width;
            }
            y += l.height;
        }
    };
    drawPart(text.left, 0);
    drawPart(text.center, 1);
    drawPart(text.right, 2);
    p->restore();
}

} // namespace jp::grid
