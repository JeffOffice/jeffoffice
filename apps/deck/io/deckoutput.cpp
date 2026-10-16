#include "io/deckoutput.h"

#include "core/appinfo.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDate>
#include <QFileInfo>
#include <QFontMetricsF>
#include <QLocale>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

// In points: notes pages and the outline keep 0.75 inch clear on every side,
// a handout's header and footer sit 0.5 inch in from the page's edges.
constexpr double kMargin = 54, kEdge = 36;
constexpr double kFrameWidth = 1;
const QColor kInk(0x20, 0x20, 0x20);
const QColor kFrame(0x40, 0x40, 0x40);
const QColor kRule(0x70, 0x70, 0x70);

// Qt lays text out in whole pixels, so text is measured and drawn at this many
// times its size, as the text engine does.
constexpr double kFine = 8;

// A font for the labels and the outline, measured the way the text engine
// draws it.
class Face {
public:
    Face(const QString &family, double points, bool bold = false) : m_font(sized(family, points, bold)), m_metrics(sized(family, points * kFine, bold)) {}
    const QFont &font() const { return m_font; }
    double width(const QString &text) const { return m_metrics.horizontalAdvance(text) / kFine; }
    double ascent() const { return m_metrics.ascent() / kFine; }
    double lineHeight() const { return m_metrics.height() / kFine * 1.1; }
    // Text with its baseline at `y`, its left end at `x`.
    void draw(QPainter *p, double x, double y, const QString &text, const QColor &color = kInk) const
    {
        p->setPen(color);
        drawPlainText(p, QPointF(x, y), m_font, text);
    }
    // In a box: aligned across it, the line's middle at the box's middle.
    void draw(QPainter *p, const QRectF &box, Qt::Alignment across, const QString &text, const QColor &color = kInk) const
    {
        const double x = across & Qt::AlignRight ? box.right() - width(text) : across & Qt::AlignHCenter ? box.center().x() - width(text) / 2 : box.left();
        draw(p, x, box.center().y() - lineHeight() / 2 + ascent(), text, color);
    }

private:
    static QFont sized(const QString &family, double points, bool bold)
    {
        QFont f(family);
        f.setPointSizeF(points * fontPointFactor());
        f.setBold(bold);
        return f;
    }

    QFont m_font;
    QFontMetricsF m_metrics;
};

// ---- the slides to print ----

QRectF fitInside(const QSizeF &size, const QRectF &box)
{
    const double k = std::min(box.width() / size.width(), box.height() / size.height());
    const QSizeF fitted = size * k;
    return QRectF(box.center() - QPointF(fitted.width() / 2, fitted.height() / 2), fitted);
}

struct Grid {
    int columns = 1, rows = 1;
    bool lines = false;   // lines for notes beside each slide
};

Grid gridOf(PrintLayout layout)
{
    switch (layout) {
    case PrintLayout::Handout2: return {1, 2, false};
    case PrintLayout::Handout3: return {1, 3, true};
    case PrintLayout::Handout4: return {2, 2, false};
    case PrintLayout::Handout6: return {2, 3, false};
    case PrintLayout::Handout9: return {3, 3, false};
    default: return {};
    }
}

// ---- the outline ----

struct OutlineEntry {
    int slide = 0;
    QString title;
    QVector<QPair<int, QString>> body;   // level, text
};

bool isTitle(const QString &type) { return type == QLatin1String("title") || type == QLatin1String("ctrTitle"); }
bool isBody(const QString &type) { return type == QLatin1String("body") || type == QLatin1String("obj") || type == QLatin1String("subTitle"); }

// A paragraph's words on one line: no fields, soft line breaks, or tabs.
QString plain(QString text)
{
    text.replace(QChar::ObjectReplacementCharacter, QString());
    text.replace(QChar::LineSeparator, QLatin1Char(' '));
    text.replace(QLatin1Char('\t'), QLatin1Char(' '));
    return text.simplified();
}

QVector<OutlineEntry> outlineOf(const Document &d, const QVector<int> &slides)
{
    QVector<OutlineEntry> out;
    for (int index : slides) {
        const Page &slide = *d.pages[index];
        OutlineEntry e;
        e.slide = index;
        for (const auto &it : slide.items) {
            if (it->type() != ItemType::Text) continue;
            const QString type = placeholderType(*it);
            const bool title = isTitle(type);
            if (!title && !isBody(type)) continue;
            const QTextDocument *story = d.storyDoc(static_cast<const TextItem &>(*it).storyId);
            if (!story) continue;
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
                const QString text = plain(b.text());
                if (text.isEmpty()) continue;
                if (title) e.title += (e.title.isEmpty() ? QString() : QStringLiteral(" ")) + text;
                else e.body << qMakePair(std::clamp(b.blockFormat().intProperty(tp::ListLevel), 0, 8), text);
            }
        }
        out << e;
    }
    return out;
}

struct OutlineLine {
    QString text;
    bool title = false;
    double x = 0, baseline = 0;
};

QStringList wrapped(const Face &face, QString text, double width)
{
    QStringList lines;
    QString line;
    const QStringList words = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (QString word : words) {
        // A word wider than the line is broken where it must be.
        while (face.width(word) > width && word.size() > 1) {
            int n = 1;
            while (n < word.size() - 1 && face.width(word.left(n + 1)) <= width) ++n;
            if (!line.isEmpty()) {
                lines << line;
                line.clear();
            }
            lines << word.left(n);
            word = word.mid(n);
        }
        const QString joined = line.isEmpty() ? word : line + QLatin1Char(' ') + word;
        if (line.isEmpty() || face.width(joined) <= width) {
            line = joined;
        } else {
            lines << line;
            line = word;
        }
    }
    if (!line.isEmpty() || lines.isEmpty()) lines << line;
    return lines;
}

Face outlineTitleFace(const Document &d) { return Face(d.fonts.heading, 14, true); }
Face outlineBodyFace(const Document &d) { return Face(d.fonts.body, 11); }
constexpr double kNumberWidth = 30, kLevelIndent = 18, kSlideGap = 10;

// The outline set on pages: lines with their places, a page of them each.
QVector<QVector<OutlineLine>> outlinePages(const Document &d, const QVector<int> &slides, const QSizeF &paper)
{
    const Face head = outlineTitleFace(d), body = outlineBodyFace(d);
    const double w = std::max(paper.width() - 2 * kMargin, 80.0), h = std::max(paper.height() - 2 * kMargin, 6 * head.lineHeight());
    QVector<QVector<OutlineLine>> pages(1);
    double y = 0;
    // Starts a page when `height` more doesn't fit on this one.
    auto room = [&](double height) {
        if (!pages.last().isEmpty() && y + height > h + 0.01) {
            pages.append(QVector<OutlineLine>());
            y = 0;
        }
    };
    for (const OutlineEntry &e : outlineOf(d, slides)) {
        const QStringList title = wrapped(head, e.title, w - kNumberWidth);
        if (!pages.last().isEmpty()) y += kSlideGap;
        // The title keeps company with the first line under it.
        room(title.size() * head.lineHeight() + (e.body.isEmpty() ? 0 : body.lineHeight()));
        OutlineLine number;
        number.text = QString::number(e.slide + 1);
        number.title = true;
        number.baseline = y + head.ascent();
        pages.last() << number;
        for (const QString &line : title) {
            OutlineLine t;
            t.text = line;
            t.title = true;
            t.x = kNumberWidth;
            t.baseline = y + head.ascent();
            if (!line.isEmpty()) pages.last() << t;
            y += head.lineHeight();
        }
        for (const auto &para : e.body) {
            const double indent = kNumberWidth + kLevelIndent * para.first;
            for (const QString &line : wrapped(body, para.second, std::max(w - indent, 40.0))) {
                room(body.lineHeight());
                OutlineLine t;
                t.text = line;
                t.x = indent;
                t.baseline = y + body.ascent();
                pages.last() << t;
                y += body.lineHeight();
            }
        }
    }
    return pages;
}

// ---- where the output is drawn ----

TextOutput textOutputOf(const QPainter *p)
{
    if (dynamic_cast<const QPdfWriter *>(p->device())) return TextOutput::Pdf;
    if (dynamic_cast<const QPrinter *>(p->device())) return TextOutput::Print;
    return TextOutput::Picture;
}

// Whether the painter's device turns colors to gray itself (a PDF set to a
// gray color model), so they can be painted as they are.
bool deviceMakesGray(const QPainter *p)
{
    const auto *pdf = dynamic_cast<const QPdfWriter *>(p->device());
    return pdf && pdf->colorModel() == QPdfWriter::ColorModel::Grayscale;
}

struct Sheet {
    QPainter *p;
    const Document &d;
    PaintContext ctx;
    const OutputOptions &o;
    QSizeF paper;
};

void drawSlide(const Sheet &s, int slide, const QRectF &target, bool frame)
{
    QPainter *p = s.p;
    const QSizeF size = s.d.pageSize();
    p->save();
    p->translate(target.topLeft());
    const double k = target.width() / size.width();
    p->scale(k, k);
    p->setClipRect(QRectF(QPointF(0, 0), size));
    p->fillRect(QRectF(QPointF(0, 0), size), Qt::white);
    Renderer::paintPage(p, s.ctx, slide);
    p->restore();
    if (!frame) return;
    // Inside the slide's edge, so a slide that fills the page is framed whole.
    p->save();
    QPen pen(kFrame, kFrameWidth);
    pen.setJoinStyle(Qt::MiterJoin);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->drawRect(target.adjusted(kFrameWidth / 2, kFrameWidth / 2, -kFrameWidth / 2, -kFrameWidth / 2));
    p->restore();
}

void paintSlidePage(const Sheet &s, int slide)
{
    drawSlide(s, slide, fitInside(s.d.pageSize(), QRectF(QPointF(0, 0), s.paper)), s.o.frameSlides);
}

void paintNotesPage(const Sheet &s, int slide)
{
    const double w = s.paper.width() - 2 * kMargin, half = s.paper.height() / 2;
    // The slide at the top, as large as the upper half lets it be, and the notes under it.
    QRectF target = fitInside(s.d.pageSize(), QRectF(kMargin, kMargin, w, half - kMargin - 18));
    target.moveTop(kMargin);
    drawSlide(s, slide, target, true);
    // The notes are set as a text box would set them, shrinking to fit what room there is.
    const Page &page = *s.d.pages[slide];
    if (!page.notesStoryId.isEmpty() && s.d.story(page.notesStoryId)) {
        auto box = std::make_shared<TextItem>();
        box->id = QStringLiteral("output-notes-") + page.id;
        box->storyId = page.notesStoryId;
        const double top = target.bottom() + 24;
        box->rect = QRectF(kMargin, top, w, std::max(s.paper.height() - kMargin - 24 - top, 20.0));
        box->insets = QMarginsF(0, 0, 0, 0);
        box->autofit = TextItem::ShrinkOnOverflow;
        s.p->save();
        s.p->setClipRect(box->rect);
        Renderer::paintItems(s.p, s.ctx, ItemList{box});
        s.p->restore();
    }
    const Face label(s.d.fonts.body, 10);
    label.draw(s.p, QRectF(kEdge, s.paper.height() - kEdge - 12, s.paper.width() - 2 * kEdge, 12), Qt::AlignRight, QString::number(slide + 1));
}

void paintHandoutPage(const Sheet &s, const QVector<int> &slides, int page)
{
    const Grid grid = gridOf(s.o.layout);
    const int per = grid.columns * grid.rows;
    const Face label(s.d.fonts.body, 10);
    const QRectF header(kEdge, kEdge - 6, s.paper.width() - 2 * kEdge, 12), footer(kEdge, s.paper.height() - kEdge - 6, s.paper.width() - 2 * kEdge, 12);
    if (!s.d.props.title.isEmpty()) label.draw(s.p, header, Qt::AlignLeft, plain(s.d.props.title));
    label.draw(s.p, header, Qt::AlignRight, QLocale().toString(QDate::currentDate(), QLocale::ShortFormat));
    label.draw(s.p, footer, Qt::AlignRight, QString::number(page + 1));
    const QRectF area(kEdge, 60, s.paper.width() - 2 * kEdge, s.paper.height() - 120);
    const double cellW = area.width() / grid.columns, cellH = area.height() / grid.rows;
    constexpr double gap = 10;
    for (int k = 0; k < per; ++k) {
        const int at = page * per + k;
        if (at >= slides.size()) break;
        const int column = s.o.horizontalOrder ? k % grid.columns : k / grid.rows, row = s.o.horizontalOrder ? k / grid.columns : k % grid.rows;
        const QRectF cell(area.left() + column * cellW, area.top() + row * cellH, cellW, cellH);
        QRectF room = cell.adjusted(gap, gap, -gap, -gap);
        if (grid.lines) room.setWidth(cell.width() * 0.46 - gap);
        const QRectF target = fitInside(s.d.pageSize(), room);
        drawSlide(s, slides[at], target, true);
        if (!grid.lines) continue;
        // Lines to write on, down the slide's height.
        s.p->save();
        s.p->setPen(QPen(kRule, 0.75));
        const double from = cell.left() + cell.width() * 0.52, to = cell.right() - gap;
        for (int i = 1; i <= 5; ++i) {
            const double y = target.top() + target.height() * i / 5;
            s.p->drawLine(QPointF(from, y), QPointF(to, y));
        }
        s.p->restore();
    }
}

void paintOutlinePage(const Sheet &s, const QVector<int> &slides, int page)
{
    const QVector<QVector<OutlineLine>> pages = outlinePages(s.d, slides, s.paper);
    if (page < 0 || page >= pages.size()) return;
    const Face head = outlineTitleFace(s.d), body = outlineBodyFace(s.d);
    for (const OutlineLine &line : pages[page]) (line.title ? head : body).draw(s.p, kMargin + line.x, kMargin + line.baseline, line.text);
}

// Paints the page as the device draws it.
void paintPlain(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper, TextOutput output)
{
    LayoutCache own;
    Sheet s{p, d, PaintContext(), o, paper};
    s.ctx.doc = &d;
    s.ctx.cache = cache ? cache : &own;
    s.ctx.opt.output = true;
    s.ctx.opt.textOutput = output;
    s.ctx.opt.plainLetters = output == TextOutput::Pdf;
    const QVector<int> slides = outputSlides(d, o);
    p->save();
    switch (o.layout) {
    case PrintLayout::Slides:
    case PrintLayout::NotesPages:
        if (page >= 0 && page < slides.size()) (o.layout == PrintLayout::Slides ? paintSlidePage : paintNotesPage)(s, slides[page]);
        break;
    case PrintLayout::Outline: paintOutlinePage(s, slides, page); break;
    default:
        if (page >= 0 && page * slidesPerPage(o.layout) < slides.size()) paintHandoutPage(s, slides, page);
        break;
    }
    p->restore();
}

// The page painted in color into pixels, as gray pixels from there, for a
// device that can't make the colors gray itself.
void paintGrayPixels(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper)
{
    const QTransform toDevice = p->deviceTransform();
    // As many pixels to the point as the device has, up to 300 to the inch.
    const double k = std::clamp(std::hypot(toDevice.m11(), toDevice.m12()), 0.05, 300.0 / 72);
    QImage img((paper * k).toSize().expandedTo(QSize(1, 1)), QImage::Format_RGB32);
    img.fill(Qt::white);
    {
        QPainter ip(&img);
        ip.setRenderHint(QPainter::Antialiasing);
        ip.setRenderHint(QPainter::TextAntialiasing);
        ip.setRenderHint(QPainter::SmoothPixmapTransform);
        ip.scale(img.width() / paper.width(), img.height() / paper.height());
        paintPlain(&ip, d, cache, o, page, paper, textOutputOf(p));
    }
    p->save();
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawImage(QRectF(QPointF(0, 0), paper), img.convertToFormat(QImage::Format_Grayscale8));
    p->restore();
}

} // namespace

QVector<int> outputSlides(const Document &d, const OutputOptions &o)
{
    QVector<int> out;
    const int n = int(d.pages.size());
    auto take = [&](int i) {
        if (i >= 0 && i < n && (o.hiddenSlides || !d.pages[i]->hidden)) out << i;
    };
    if (o.slides.isEmpty()) {
        for (int i = 0; i < n; ++i) take(i);
    } else {
        for (int i : o.slides) take(i);
    }
    return out;
}

QVector<int> parseSlideRange(const QString &text, int count)
{
    QVector<int> out;
    const QStringList parts = text.split(QRegularExpression(QStringLiteral("[,;]")));
    for (const QString &part : parts) {
        static const QRegularExpression one(QStringLiteral("^(\\d*)\\s*(-?)\\s*(\\d*)$"));
        const QRegularExpressionMatch m = one.match(part.trimmed());
        if (!m.hasMatch()) return {};
        const bool span = !m.captured(2).isEmpty();
        bool hasFrom = false, hasTo = false;
        const int a = m.captured(1).toInt(&hasFrom), b = m.captured(3).toInt(&hasTo);
        if (!hasFrom && !hasTo) return {};
        // "5": that slide; "3-6": those; "3-": to the last; "-4": up to that.
        const int from = hasFrom ? a : 1, to = span ? (hasTo ? b : count) : a;
        if (from < 1 || from > count || to < from) return {};
        for (int i = from; i <= std::min(to, count); ++i) out << i - 1;
    }
    return out;
}

int slidesPerPage(PrintLayout layout)
{
    switch (layout) {
    case PrintLayout::Handout1: return 1;
    case PrintLayout::Handout2: return 2;
    case PrintLayout::Handout3: return 3;
    case PrintLayout::Handout4: return 4;
    case PrintLayout::Handout6: return 6;
    case PrintLayout::Handout9: return 9;
    default: return 0;
    }
}

QSizeF localePaper()
{
    // Letter in North America and a few countries beside it, A4 everywhere else.
    static const QList<QLocale::Territory> letter = {QLocale::UnitedStates, QLocale::Canada,     QLocale::Mexico,   QLocale::Philippines, QLocale::Colombia,
                                                     QLocale::Chile,        QLocale::Venezuela,  QLocale::CostaRica, QLocale::Guatemala,   QLocale::Panama,
                                                     QLocale::DominicanRepublic, QLocale::ElSalvador, QLocale::Nicaragua, QLocale::Honduras, QLocale::PuertoRico};
    return QPageSize(letter.contains(QLocale().territory()) ? QPageSize::Letter : QPageSize::A4).size(QPageSize::Point);
}

QSizeF outputPageSize(const Document &d, const OutputOptions &o, const QSizeF &paper)
{
    if (o.layout == PrintLayout::Slides) return d.pageSize();
    return paper.isEmpty() ? localePaper() : paper;
}

int outputPageCount(const Document &d, const OutputOptions &o, const QSizeF &paper)
{
    const QVector<int> slides = outputSlides(d, o);
    if (slides.isEmpty()) return 0;
    if (o.layout == PrintLayout::Outline) return int(outlinePages(d, slides, outputPageSize(d, o, paper)).size());
    const int per = slidesPerPage(o.layout);
    return per ? int((slides.size() + per - 1) / per) : int(slides.size());
}

QString outputOutlineText(const Document &d, const OutputOptions &o)
{
    QString out;
    for (const OutlineEntry &e : outlineOf(d, outputSlides(d, o))) {
        out += QString::number(e.slide + 1) + (e.title.isEmpty() ? QString() : QStringLiteral("  ") + e.title) + QLatin1Char('\n');
        for (const auto &para : e.body) out += QString(2 + 2 * para.first, QLatin1Char(' ')) + para.second + QLatin1Char('\n');
    }
    return out;
}

void paintOutputPage(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper)
{
    if (o.grayscale && !deviceMakesGray(p)) paintGrayPixels(p, d, cache, o, page, paper);
    else paintPlain(p, d, cache, o, page, paper, textOutputOf(p));
}

bool exportPdf(const Document &d, const QString &path, const OutputOptions &o, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    const int pages = outputPageCount(d, o);
    if (pages <= 0) return fail(QCoreApplication::translate("DeckOutput", "There are no slides to print."));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    {
        QPdfWriter pdf(&file);
        pdf.setCreator(appInfo().name);
        pdf.setTitle(d.props.title.isEmpty() ? QFileInfo(path).completeBaseName() : d.props.title);
        if (o.grayscale) pdf.setColorModel(QPdfWriter::ColorModel::Grayscale);
        const QSizeF size = outputPageSize(d, o);
        pdf.setPageSize(QPageSize(size, QPageSize::Point, QString(), QPageSize::ExactMatch));
        pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
        pdf.setResolution(1200);
        QPainter p;
        if (!p.begin(&pdf)) {
            file.cancelWriting();
            return fail(QCoreApplication::translate("DeckOutput", "The PDF can't be started."));
        }
        const double scale = 1200.0 / 72.0;
        LayoutCache cache;
        for (int i = 0; i < pages; ++i) {
            if (i) pdf.newPage();
            p.save();
            p.scale(scale, scale);
            paintOutputPage(&p, d, &cache, o, i, size);
            p.restore();
        }
        p.end();
    }
    if (!file.commit()) return fail(file.errorString());
    return true;
}

QImage slideImage(const Document &d, LayoutCache *cache, int slide, int width)
{
    if (slide < 0 || slide >= d.pages.size() || width < 1) return QImage();
    LayoutCache own;
    PaintContext ctx;
    ctx.doc = &d;
    ctx.cache = cache ? cache : &own;
    ctx.opt.output = true;
    const QSizeF size = d.pageSize();
    QImage img(QSize(width, std::max(1, int(std::lround(width * size.height() / size.width())))), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.scale(img.width() / size.width(), img.height() / size.height());
    Renderer::paintPage(&p, ctx, slide);
    p.end();
    return img;
}

} // namespace jp::deck
