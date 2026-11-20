#include "io/deckoutput.h"
#include "core/items.h"
#include "core/notesmaster.h"
#include "core/printheaderfooter.h"

#include "app/settings.h"
#include "app/pdfproperties.h"
#include "app/printpage.h"
#include "app/privacy.h"
#include "core/comments.h"
#include "core/appinfo.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "core/linkareas.h"
#include "office/hyperlink.h"
#include "core/reuse.h"
#include "io/pdfbookmarks.h"
#include "io/pdftransparency.h"
#include "render/pagetags.h"
#include "render/svgexport.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDir>
#include <QDate>
#include <QFileInfo>
#include <QFontMetricsF>
#include <QLocale>
#include <QPageSize>
#include <QPainter>
#include <QBuffer>
#include <QPdfWriter>
#include <QPrinter>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

// In points: the outline keeps 0.75 inch clear on every side (the places of notes pages and handouts are core/notesmaster's).
constexpr double kMargin = 54;
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
            if (it->type() != ItemType::Text || it->hidden) continue;   // (a hidden object is not in the outline)
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
    // (a printer that makes a PDF file does so too: the text stays text)
    if (const auto *printer = dynamic_cast<const QPrinter *>(p->device())) return printer->outputFormat() == QPrinter::PdfFormat && printer->colorMode() == QPrinter::GrayScale;
    const auto *pdf = dynamic_cast<const QPdfWriter *>(p->device());
    return pdf && pdf->colorModel() == QPdfWriter::ColorModel::Grayscale;
}

struct Sheet {
    QPainter *p;
    const Document &d;
    PaintContext ctx;
    const OutputOptions &o;
    QSizeF paper;
    PageTagger *tagger = nullptr;   // set when the output is tagged
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
    {
        ArtifactSpan paper(s.tagger, p);
        p->fillRect(QRectF(QPointF(0, 0), size), Qt::white);
    }
    // The slide's objects belong to one section of the tags, in the order the slide reads them.
    if (s.tagger) s.tagger->beginSection();
    Renderer::paintPage(p, s.ctx, slide);
    p->restore();
    if (!frame) return;
    // Inside the slide's edge, so a slide that fills the page is framed whole.
    ArtifactSpan edge(s.tagger, p);
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
    const QRectF paper(QPointF(0, 0), s.paper);
    QRectF target = fitInside(s.d.pageSize(), paper);
    // (not scaled to the paper: a slide that fits it is at its own size)
    if (!s.o.scaleToFit && target.width() > s.d.pageSize().width()) target = QRectF(paper.center() - QPointF(s.d.pageSize().width() / 2, s.d.pageSize().height() / 2), s.d.pageSize());
    drawSlide(s, slide, target, s.o.frameSlides);
}

// The header, date, footer, and page number of a notes page or a handout (Print > Header & Footer), in the places the master gives them. `number` is the page's number.
void paintHeaderFooter(const Sheet &s, PrintedPages pages, int number, const QRectF &header, const QRectF &date, const QRectF &footer, const QRectF &numberBox)
{
    const PrintHeaderFooter hf = printHeaderFooter(s.d, pages);
    const Face label(s.d.fonts.body, 10);
    ArtifactSpan lines(s.tagger, s.p);
    if (hf.header && !hf.headerText.isEmpty()) label.draw(s.p, header, Qt::AlignLeft, plain(hf.headerText));
    if (hf.date) label.draw(s.p, date, Qt::AlignRight, hf.dateText.isEmpty() ? QLocale().toString(QDate::currentDate(), QLocale::ShortFormat) : plain(hf.dateText));
    if (hf.footer && !hf.footerText.isEmpty()) label.draw(s.p, footer, Qt::AlignLeft, plain(hf.footerText));
    if (hf.pageNumber) label.draw(s.p, numberBox, Qt::AlignRight, QString::number(number));
}

// The page's own color, when the master has one.
void paintPageBackground(const Sheet &s, const QColor &color)
{
    if (!color.isValid()) return;
    ArtifactSpan paper(s.tagger, s.p);
    s.p->fillRect(QRectF(QPointF(0, 0), s.paper), color);
}

void paintNotesPage(const Sheet &s, int slide)
{
    const NotesMaster master = notesMasterOf(s.d, s.paper);
    const NotesPageGeometry g = notesPageGeometry(master, s.paper, s.d.pageSize());
    paintPageBackground(s, master.background);
    // The slide at the top, and the notes under it.
    drawSlide(s, slide, g.slideImage, true);
    // The notes are set as a text box would set them, shrinking to fit what room there is.
    const Page &page = *s.d.pages[slide];
    if (!s.o.skipNotesText && !page.notesStoryId.isEmpty() && s.d.story(page.notesStoryId)) {
        auto box = std::make_shared<TextItem>();
        box->id = QStringLiteral("output-notes-") + page.id;
        box->storyId = page.notesStoryId;
        box->rect = g.body;
        box->insets = QMarginsF(0, 0, 0, 0);
        box->autofit = TextItem::ShrinkOnOverflow;
        s.p->save();
        s.p->setClipRect(box->rect);
        // (the notes are the page's text: a paragraph of its own)
        if (s.ctx.mark) s.ctx.mark(s.p, *box, true);
        Renderer::paintItems(s.p, s.ctx, ItemList{box});
        if (s.ctx.mark) s.ctx.mark(s.p, *box, false);
        s.p->restore();
    }
    paintHeaderFooter(s, PrintedPages::Notes, slide + 1, g.header, g.date, g.footer, g.number);
}

void paintHandoutPage(const Sheet &s, const QVector<int> &slides, int page)
{
    const int per = slidesPerPage(s.o.layout);
    const HandoutMaster master = handoutMasterOf(s.d, s.paper);
    const HandoutGeometry g = handoutGeometry(master, s.paper, per, s.o.horizontalOrder);
    // (a handout's header, footer, and lines to write on are decoration, drawn between its slides)
    PageTagger *tagger = s.tagger;
    paintPageBackground(s, master.background);
    paintHeaderFooter(s, PrintedPages::Handouts, page + 1, g.header, g.date, g.footer, g.number);
    constexpr double gap = 10;
    for (int k = 0; k < per; ++k) {
        const int at = page * per + k;
        if (at >= slides.size()) break;
        const HandoutSlot &slot = g.slots[k];
        const QRectF target = fitInside(s.d.pageSize(), slot.room);
        drawSlide(s, slides[at], target, true);
        if (!slot.lines) continue;
        // Lines to write on, down the slide's height.
        ArtifactSpan writing(tagger, s.p);
        s.p->save();
        s.p->setPen(QPen(kRule, 0.75));
        const double from = slot.cell.left() + slot.cell.width() * 0.52, to = slot.cell.right() - gap;
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

// ---- the review comments ----

constexpr int kMostCommentPages = 20;      // the pages of comments one slide gets, at most (what is left is said to be left out)
constexpr int kMostCommentChars = 20000;   // of one comment or reply that is set

QString commentsHeading(int slide, int part)
{
    return part == 0 ? QCoreApplication::translate("DeckOutput", "Comments on slide %1").arg(slide + 1) : QCoreApplication::translate("DeckOutput", "Comments on slide %1, continued").arg(slide + 1);
}

// One line of a comment ("who: what it says", the names left out when personal information is removed) and where it stands on its page.
struct CommentLine {
    QString text;
    double indent = 0, y = 0;   // y: from the top of the text area
    int item = 0;               // the comment or reply it belongs to, counted down the slide's list
};
struct CommentsLayout {
    QVector<QVector<CommentLine>> pages;
    int notShown = 0;           // comments and replies that have no line on any page (the pages of one slide are at most kMostCommentPages)
};

double commentsHeadHeight(const Face &head, const QString &heading, double w) { return wrapped(head, heading, w).size() * head.lineHeight() + kSlideGap; }

// The pages of one slide's comments: its heading, then each comment, its replies under it, set line by line, so that a comment taller than a page goes on to the next.
CommentsLayout commentsLayout(const Document &d, int slide, const QSizeF &sheet, bool names)
{
    const Face head = outlineTitleFace(d), body = outlineBodyFace(d);
    const double w = std::max(sheet.width() - 2 * kMargin, 80.0), h = std::max(sheet.height() - 2 * kMargin, 6 * head.lineHeight()), lh = body.lineHeight();
    const QVector<comments::Comment> list = comments::commentsOf(*d.pages[slide]);
    int total = 0;
    for (const comments::Comment &c : list) total += 1 + int(c.replies.size());
    CommentsLayout out;
    out.pages.push_back({});
    double y = commentsHeadHeight(head, commentsHeading(slide, 0), w);
    int item = 0;
    auto put = [&](const QString &author, const QString &text, double indent) {
        QString what = plain(text);
        if (what.size() > kMostCommentChars) what = what.left(what[kMostCommentChars - 1].isHighSurrogate() ? kMostCommentChars - 1 : kMostCommentChars) + QStringLiteral("…");
        if (names && !author.trimmed().isEmpty()) what = author.trimmed() + QStringLiteral(": ") + what;
        for (const QString &line : wrapped(body, what, std::max(w - indent, 40.0))) {
            // (a line that fits on no page at all is set on an empty one all the same)
            if (y + lh > h + 0.01 && !out.pages.last().isEmpty()) {
                if (out.pages.size() >= kMostCommentPages) return false;
                out.pages.push_back({});
                y = commentsHeadHeight(head, commentsHeading(slide, int(out.pages.size()) - 1), w);
            }
            out.pages.last().push_back({line, indent, y, item});
            y += lh;
        }
        y += 3;
        ++item;
        return true;
    };
    bool all = true;
    for (int c = 0; all && c < list.size(); ++c) {
        all = put(list[c].author, list[c].text, 0);
        for (int r = 0; all && r < list[c].replies.size(); ++r) all = put(list[c].replies[r].author, list[c].replies[r].text, kLevelIndent);
    }
    if (!all) {
        // The line that says so has a place of its own at the foot of the last page: the lines that would be under it are taken off, and what is left with no line is counted.
        QVector<CommentLine> &last = out.pages.last();
        while (!last.isEmpty() && last.last().y + lh > h - lh + 0.01) last.removeLast();
        int shown = 0;
        for (const QVector<CommentLine> &page : std::as_const(out.pages))
            for (const CommentLine &line : page) shown = std::max(shown, line.item + 1);
        out.notShown = total - shown;
    }
    return out;
}

} // namespace

QVector<CommentsPage> commentsPages(const Document &d, int slide, const QSizeF &sheet, bool names)
{
    QVector<CommentsPage> out;
    if (slide < 0 || slide >= d.pages.size()) return out;
    const CommentsLayout layout = commentsLayout(d, slide, sheet, names);
    const Face head = outlineTitleFace(d), body = outlineBodyFace(d);
    const double h = std::max(sheet.height() - 2 * kMargin, 6 * head.lineHeight());
    for (int i = 0; i < layout.pages.size(); ++i) {
        CommentsPage page;
        page.lineHeight = body.lineHeight();
        page.bottom = h;
        for (const CommentLine &line : layout.pages[i]) {
            page.lines << line.text;
            page.tops << line.y;
        }
        if (i == layout.pages.size() - 1 && layout.notShown > 0) {
            page.noticeTop = h - body.lineHeight();
            page.notShown = layout.notShown;
        }
        out << page;
    }
    return out;
}

namespace {

// The pages of slides and notes pages: each slide, and after it, when comments are asked for and it has any, the pages of its comments.
struct PlannedPage {
    int slide = 0;
    bool comments = false;
    int part = 0;   // of a slide's comments
};
struct CommentsPlan {
    QVector<PlannedPage> plan;
    QHash<int, CommentsLayout> layouts;   // by slide
};

bool namesShown(const OutputOptions &o) { return !privacy::removeOnSave() && !o.namesOut; }

// What the pages of comments depend on, as one number: the slides, the sheet, the names, the fonts, and every comment's words (a plan is the same while this is).
quint64 commentsKey(const Document &d, const OutputOptions &o, const QVector<int> &slides, const QSizeF &sheet)
{
    size_t k = qHash(QPair<int, int>(int(std::lround(sheet.width() * 100)), int(std::lround(sheet.height() * 100))), namesShown(o) ? 1 : 2);
    k = qHash(d.fonts.heading, k);
    k = qHash(d.fonts.body, k);
    for (int slide : slides) {
        k = qHash(slide, k);
        if (slide < 0 || slide >= d.pages.size()) continue;
        for (const comments::Comment &c : comments::commentsOf(*d.pages[slide])) {
            k = qHash(c.author, k);
            k = qHash(c.text, k);
            for (const auto &r : c.replies) {
                k = qHash(r.author, k);
                k = qHash(r.text, k);
            }
        }
    }
    return quint64(k);
}

// The sheet the pages of comments are set on: the one the output is painted on, or, where the caller has none, the one the print preview shows.
QSizeF commentsSheet(const Document &d, const OutputOptions &o, const QSizeF &paper) { return paper.isEmpty() ? printSheetSize(d, o) : paper; }

const CommentsPlan &commentsPlan(const Document &d, const OutputOptions &o, const QVector<int> &slides, const QSizeF &paper)
{
    const bool withComments = o.comments && (o.layout == PrintLayout::Slides || o.layout == PrintLayout::NotesPages);
    if (!withComments) {
        static thread_local CommentsPlan bare;
        bare.plan.clear();
        bare.layouts.clear();
        for (int slide : slides) bare.plan.push_back({slide, false, 0});
        return bare;
    }
    // (kept from one page to the next of a job: every page asks for it)
    static thread_local struct {
        quint64 key = 0;
        bool valid = false;
        CommentsPlan plan;
    } kept;
    const QSizeF sheet = commentsSheet(d, o, paper);
    const quint64 key = commentsKey(d, o, slides, sheet);
    if (kept.valid && kept.key == key) return kept.plan;
    CommentsPlan fresh;
    for (int slide : slides) {
        fresh.plan.push_back({slide, false, 0});
        if (slide < 0 || slide >= d.pages.size() || comments::commentsOf(*d.pages[slide]).isEmpty()) continue;
        const CommentsLayout layout = commentsLayout(d, slide, sheet, namesShown(o));
        for (int part = 0; part < layout.pages.size(); ++part) fresh.plan.push_back({slide, true, part});
        fresh.layouts.insert(slide, layout);
    }
    kept.key = key;
    kept.valid = true;
    kept.plan = std::move(fresh);
    return kept.plan;
}

// The page of a slide's comments, one part of them: its heading, and each line where the layout puts it. The words are real content for a tagged PDF (a heading and a paragraph for
// each comment or reply), in the order they are read, after the slide's own page.
void paintCommentsPage(const Sheet &s, int slide, int part, const CommentsLayout &layout)
{
    const Face head = outlineTitleFace(s.d), body = outlineBodyFace(s.d);
    const double w = std::max(s.paper.width() - 2 * kMargin, 80.0), h = std::max(s.paper.height() - 2 * kMargin, 6 * head.lineHeight());
    PdfTagger *tags = s.tagger ? s.tagger->tags() : nullptr;
    const int section = s.tagger ? s.tagger->beginSection() : -1;
    // (the tag a run of lines is drawn in)
    auto tagged = [&](const QString &type, const std::function<void()> &draw) {
        if (!tags) {
            draw();
            return;
        }
        PdfTag tag;
        tag.type = type;
        tag.parent = section;
        const int id = tags->add(tag);
        tags->begin(s.p, id);
        draw();
        tags->end(s.p, id);
    };
    tagged(QStringLiteral("H2"), [&] {
        double y = 0;
        for (const QString &line : wrapped(head, commentsHeading(slide, part), w)) {
            head.draw(s.p, kMargin, kMargin + y + head.ascent(), line);
            y += head.lineHeight();
        }
    });
    const QVector<CommentLine> lines = part < layout.pages.size() ? layout.pages[part] : QVector<CommentLine>();
    for (int i = 0; i < lines.size();) {
        // (the lines of one comment or reply on this page are one paragraph)
        int j = i;
        while (j < lines.size() && lines[j].item == lines[i].item) ++j;
        tagged(QStringLiteral("P"), [&] {
            for (int k = i; k < j; ++k) body.draw(s.p, kMargin + lines[k].indent, kMargin + lines[k].y + body.ascent(), lines[k].text);
        });
        i = j;
    }
    if (part == layout.pages.size() - 1 && layout.notShown > 0) {
        const QString more = QCoreApplication::translate("DeckOutput", "%n more comment(s) not shown.", nullptr, layout.notShown);
        tagged(QStringLiteral("P"), [&] { body.draw(s.p, kMargin, kMargin + h - body.lineHeight() + body.ascent(), more, kRule); });
    }
}

// Paints the page as the device draws it.
void paintPlain(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper, TextOutput output, PdfTagger *tags = nullptr)
{
    LayoutCache own;
    Sheet s{p, d, PaintContext(), o, paper};
    s.ctx.doc = &d;
    s.ctx.cache = cache ? cache : &own;
    s.ctx.opt.output = true;
    s.ctx.opt.textOutput = output;
    s.ctx.opt.flattenTransparency = o.archive;   // (PDF/A-1 allows no transparency)
    PageTagger tagger(tags, [](const Item &it) {
        const QString type = placeholderType(it);
        return it.type() == ItemType::Text && (type == QLatin1String("title") || type == QLatin1String("ctrTitle"));
    });
    if (tags) {
        s.tagger = &tagger;
        tagger.install(&s.ctx);
    }
    const QVector<int> slides = outputSlides(d, o);
    p->save();
    switch (o.layout) {
    case PrintLayout::Slides:
    case PrintLayout::NotesPages: {
        const CommentsPlan &plan = commentsPlan(d, o, slides, paper);
        if (page < 0 || page >= plan.plan.size()) break;
        const PlannedPage &at = plan.plan[page];
        if (at.comments) paintCommentsPage(s, at.slide, at.part, plan.layouts.value(at.slide));
        else (o.layout == PrintLayout::Slides ? paintSlidePage : paintNotesPage)(s, at.slide);
        break;
    }
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
    // As many pixels to the point as the device has, up to 300 to the inch (600 for High quality, where the page's picture is not more than kMostHighQualityPixels).
    const double k = std::clamp(std::hypot(toDevice.m11(), toDevice.m12()), 0.05, (o.highQuality && highQualityFits(paper) ? 600.0 : 300.0) / 72);
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
    // (pure black and white: each point is black or white, from the middle of the gray; the pixels are not smoothed into grays again)
    if (o.blackAndWhite) {
        p->drawImage(QRectF(QPointF(0, 0), paper), img.convertToFormat(QImage::Format_Grayscale8).convertToFormat(QImage::Format_Mono, Qt::ThresholdDither | Qt::AvoidDither));
    } else {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawImage(QRectF(QPointF(0, 0), paper), img.convertToFormat(QImage::Format_Grayscale8));
    }
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
    return QPageSize(jp::usesLetterPaper(QLocale()) ? QPageSize::Letter : QPageSize::A4).size(QPageSize::Point);
}

bool highQualityFits(const QSizeF &paper)
{
    const double k = 600.0 / 72;
    return paper.width() * k * paper.height() * k <= double(kMostHighQualityPixels);
}

QSizeF outputPageSize(const Document &d, const OutputOptions &o, const QSizeF &paper)
{
    if (o.layout == PrintLayout::Slides) return d.pageSize();
    const QSizeF sheet = paper.isEmpty() ? (o.paper.isEmpty() ? localePaper() : o.paper) : paper;
    // (a handout lies across when its master says so)
    return slidesPerPage(o.layout) && paper.isEmpty() && hasHandoutMaster(d) ? handoutPaper(handoutMasterOf(d, sheet), sheet) : sheet;
}

QSizeF printSheetSize(const Document &d, const OutputOptions &o)
{
    if (o.paper.isEmpty()) return outputPageSize(d, o);
    const QSizeF slide = d.pageSize();
    return o.layout == PrintLayout::Slides && slide.width() > slide.height() ? o.paper.transposed() : o.paper;
}

int outputPageCount(const Document &d, const OutputOptions &o, const QSizeF &paper)
{
    const QVector<int> slides = outputSlides(d, o);
    if (slides.isEmpty()) return 0;
    if (o.layout == PrintLayout::Outline) return int(outlinePages(d, slides, outputPageSize(d, o, paper)).size());
    const int per = slidesPerPage(o.layout);
    if (!per) return int(commentsPlan(d, o, slides, paper).plan.size());
    return int((slides.size() + per - 1) / per);
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

void paintOutputPage(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper, PdfTagger *tags)
{
    if (o.blackAndWhite || (o.grayscale && !deviceMakesGray(p))) paintGrayPixels(p, d, cache, o, page, paper);
    else paintPlain(p, d, cache, o, page, paper, textOutputOf(p), tags);
}

// The page of the PDF that each of `slides` is on (the pages of comments are between them).
static QVector<int> pdfPagesOfSlides(const Document &d, const OutputOptions &o, const QVector<int> &slides, const QSizeF &sheet)
{
    QVector<int> at;
    const CommentsPlan &plan = commentsPlan(d, o, slides, sheet);
    for (int i = 0; i < plan.plan.size(); ++i)
        if (!plan.plan[i].comments) at << i;
    return at;
}

// The clickable boxes of a PDF of printed slides, one to a page: a web or mail address goes to the web, a jump to the next, previous, first, last, or a numbered slide goes to
// that slide's page. A file's link is never written (what is on another computer must not be touched, and a path means nothing on another's), nor are the end of the show and
// the rest that only a show does.
static QVector<PdfLink> slideLinks(const Document &d, const OutputOptions &o, const QVector<int> &slides)
{
    QVector<PdfLink> out;
    if (o.layout != PrintLayout::Slides) return out;
    const QSizeF page = outputPageSize(d, o);
    const QVector<int> at = pdfPagesOfSlides(d, o, slides, page);
    LayoutCache cache;
    RenderOptions drawn;
    drawn.output = true;   // (as the slides are drawn into the PDF)
    for (int k = 0; k < slides.size(); ++k) {
        const QRectF target = fitInside(d.pageSize(), QRectF(QPointF(0, 0), page));
        const double scale = target.width() / d.pageSize().width();
        for (const LinkArea &area : linkAreas(d, cache, slides[k], drawn)) {
            const office::LinkTarget t = office::parseLink(area.link);
            PdfLink link;
            link.page = at[k];
            // (a page of comments between slides: a jump goes to the slide's own page)
            auto toSlide = [&](int index) { return index >= 0 && index < at.size() ? at[index] : -1; };
            switch (t.kind) {
            case office::LinkKind::Web:
            case office::LinkKind::Mail: link.uri = t.address; break;
            case office::LinkKind::Next: link.toPage = toSlide(k + 1 < slides.size() ? k + 1 : -1); break;
            case office::LinkKind::Previous: link.toPage = toSlide(k - 1); break;
            case office::LinkKind::First: link.toPage = toSlide(0); break;
            case office::LinkKind::Last: link.toPage = toSlide(int(slides.size()) - 1); break;
            case office::LinkKind::Slide: link.toPage = toSlide(int(slides.indexOf(t.slide - 1))); break;
            default: continue;
            }
            if (link.uri.isEmpty() && link.toPage < 0) continue;
            const QRectF r = area.rect.intersected(QRectF(QPointF(0, 0), d.pageSize()));
            if (r.isEmpty()) continue;
            link.rect = QRectF(QPointF(target.left() + r.left() * scale, page.height() - (target.top() + r.bottom() * scale)), QSizeF(r.width() * scale, r.height() * scale));
            out.push_back(link);
        }
    }
    return out;
}

bool exportPdf(const Document &d, const QString &path, const OutputOptions &o, QString *error, const PdfProtection *protect, QString *notTagged)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    // (the pages of comments are set on the sheet the slides are painted on)
    const QSizeF exportSheet = outputPageSize(d, o);
    const int pages = outputPageCount(d, o, o.layout == PrintLayout::Slides || o.layout == PrintLayout::NotesPages ? exportSheet : QSizeF());
    if (pages <= 0) return fail(QCoreApplication::translate("DeckOutput", "There are no slides to print."));
    const bool encrypted = protect && protect->isSet();
    // The place is checked before any slide is made.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    const bool tagging = o.tags && o.layout != PrintLayout::Outline && !o.grayscale;
    const bool bookmarking = o.bookmarks && o.layout != PrintLayout::Outline;
    // What the file says of itself, worked out once so that its information and its metadata agree (with personal information out, a file's name is not its title either).
    const PdfInfo info = pdfDocumentInfo(d.props.toPackageProps(), QFileInfo(path).completeBaseName(), o.namesOut);
    const QString title = info.title;
    // The PDF is made in memory when it has a password (the plain PDF is never on the disk) or the bookmarks, the links, and the tags are to be added to the finished file; otherwise it goes
    // to a file of its own, the document information is added to the end of that, and it is copied to the place a piece at a time, so that no whole PDF is held in memory.
    const QVector<PdfLink> links = slideLinks(d, o, outputSlides(d, o));
    const bool archiving = o.archive && !encrypted;
    const bool inMemory = encrypted || tagging || bookmarking || archiving || !links.isEmpty();
    QBuffer memory;
    QTemporaryFile spool(QDir::tempPath() + QStringLiteral("/jeffdeck-XXXXXX.pdf"));
    if (inMemory) memory.open(QIODevice::WriteOnly);
    else if (!spool.open()) return fail(spool.errorString());
    auto make = [&](PdfTagger *tagger) {
        if (inMemory) {
            memory.close();
            memory.setData(QByteArray());
            memory.open(QIODevice::WriteOnly);
        }
        QPdfWriter pdf(inMemory ? static_cast<QIODevice *>(&memory) : static_cast<QIODevice *>(&spool));
        pdf.setCreator(appInfo().name);
        pdf.setTitle(title);
        if (archiving) {
            pdf.setPdfVersion(QPdfWriter::PdfVersion_A1b);
            pdf.setDocumentXmpMetadata(pdfInfoXmp(info, true, 1));
        }
        if (o.grayscale) pdf.setColorModel(QPdfWriter::ColorModel::Grayscale);
        const QSizeF size = exportSheet;
        pdf.setPageSize(QPageSize(size, QPageSize::Point, QString(), QPageSize::ExactMatch));
        pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
        pdf.setResolution(1200);
        QPainter p;
        if (!p.begin(&pdf)) return false;
        const double scale = 1200.0 / 72.0;
        LayoutCache cache;
        for (int i = 0; i < pages; ++i) {
            if (i) pdf.newPage();
            p.save();
            p.scale(scale, scale);
            paintOutputPage(&p, d, &cache, o, i, size, tagger);
            p.restore();
        }
        p.end();
        return true;
    };
    PdfTagger tagger;
    if (!make(tagging ? &tagger : nullptr)) {
        file.cancelWriting();
        return fail(QCoreApplication::translate("DeckOutput", "The PDF can't be started."));
    }
    if (!inMemory) {
        // (the information goes on the end of the file the pages are in; a file made in a way the update can't extend keeps the information Qt wrote)
        appendPdfInfo(&spool, info);
        if (!spool.seek(0)) return fail(spool.errorString());
        while (!spool.atEnd()) {
            const QByteArray piece = spool.read(1 << 20);
            if (piece.isEmpty() || file.write(piece) != piece.size()) {
                file.cancelWriting();
                return fail(piece.isEmpty() ? spool.errorString() : file.errorString());
            }
        }
        if (!file.commit()) return fail(file.errorString());
        return true;
    }
    QByteArray bytes = memory.data();
    // PDF/A-1 allows no transparency: what could not be flattened is not made into a file that claims to be one.
    if (archiving) {
        const PdfTransparency clear = pdfTransparency(bytes);
        if (clear.refused()) {
            file.cancelWriting();
            // (a PDF page is a slide that is printed: the hidden ones are not in it)
            const QVector<int> shown = outputSlides(d, o);
            return fail(pdfTransparencyRefusal(clear, QStringLiteral("PDF/A"), [&](int pdfPage) {
                return o.layout == PrintLayout::Slides && pdfPage >= 1 && pdfPage <= shown.size() ? QCoreApplication::translate("DeckOutput", "slide %1").arg(shown[pdfPage - 1] + 1) : QString();
            }));
        }
    }
    if (tagging) {
        PdfTagResult made = tagPdf(bytes, tagger, {documentLanguage(d), title});
        if (made.pdf.isEmpty()) {
            // The marks could not be taken out: the slides are made again, without them.
            if (!make(nullptr)) {
                file.cancelWriting();
                return fail(QCoreApplication::translate("DeckOutput", "The PDF can't be started."));
            }
            bytes = memory.data();
            if (notTagged) *notTagged = made.why;
        } else {
            bytes = made.pdf;
            if (!made.tagged && notTagged) *notTagged = made.why;
        }
    }
    if (!links.isEmpty()) bytes = addPdfLinks(bytes, links);
    if (bookmarking) {
        // One for each slide: its title, or its number, going to the page it is on.
        const QVector<int> slides = outputSlides(d, o);
        const int per = std::max(1, slidesPerPage(o.layout));
        const QVector<int> at = pdfPagesOfSlides(d, o, slides, exportSheet);
        QVector<PdfBookmark> marks;
        for (int k = 0; k < slides.size(); ++k) {
            const QString slideName = slideTitle(d, slides[k]).trimmed();
            marks.push_back({slideName.isEmpty() ? QCoreApplication::translate("DeckOutput", "Slide %1").arg(slides[k] + 1) : slideName, 0, slidesPerPage(o.layout) ? k / per : at.value(k, k)});
        }
        bytes = addPdfBookmarks(bytes, marks);
    }
    bytes = setPdfInfo(bytes, info);
    if (encrypted) {
        QString why;
        bytes = encryptPdf(bytes, *protect, &why);
        if (bytes.isEmpty()) return fail(why);
    }
    if (file.write(bytes) != bytes.size()) return fail(file.errorString());
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

QString slidePictureExtension(SlidePictureFormat format)
{
    switch (format) {
    case SlidePictureFormat::Jpeg: return QStringLiteral("jpg");
    case SlidePictureFormat::Gif: return QStringLiteral("gif");
    case SlidePictureFormat::Tiff: return QStringLiteral("tif");
    case SlidePictureFormat::Bmp: return QStringLiteral("bmp");
    case SlidePictureFormat::Svg: return QStringLiteral("svg");
    default: return QStringLiteral("png");
    }
}

QString slidePictureName(int index, SlidePictureFormat format)
{
    return QCoreApplication::translate("jp::deck::Output", "Slide %1").arg(index + 1) + QLatin1Char('.') + slidePictureExtension(format);
}

QString slidePictureName(int index, bool jpeg) { return slidePictureName(index, jpeg ? SlidePictureFormat::Jpeg : SlidePictureFormat::Png); }

namespace {
SlidePictureFormat formatOf(const SlidePicturesOptions &o) { return o.format == SlidePictureFormat::Png && o.jpeg ? SlidePictureFormat::Jpeg : o.format; }
}

qint64 slidePicturePixels(const Document &d, int slide)
{
    if (slide < 0 || slide >= d.pages.size()) return 0;
    qint64 total = 0;
    auto count = [&](const ItemList &items) {
        for (const auto &it : items)
            if (const auto *pic = dynamic_cast<const PictureItem *>(it.get())) {
                const auto found = d.images.constFind(pic->imageId);
                if (found != d.images.cend()) total += qint64(found->pixelSize.width()) * found->pixelSize.height();
            }
    };
    count(d.pages[slide]->items);
    if (const MasterPage *m = d.masterFor(*d.pages[slide])) count(m->items);
    return total;
}

QByteArray slideSvg(const Document &d, LayoutCache *cache, int slide, QString *why)
{
    if (slide < 0 || slide >= d.pages.size()) return {};
    if (slidePicturePixels(d, slide) > kMostSlidePicturePixels) {
        if (why) *why = QCoreApplication::translate("jp::deck::Output", "The pictures on slide %1 come to more than %2 million pixels, too many for an SVG drawing, which holds each at its full size. Save the slide as a PNG instead.").arg(slide + 1).arg(kMostSlidePicturePixels / 1000000);
        return {};
    }
    LayoutCache own;
    PaintContext ctx;
    ctx.doc = &d;
    ctx.cache = cache ? cache : &own;
    ctx.opt.output = true;
    const QString title = slideTitle(d, slide).trimmed();
    return pageSvg(ctx, slide, title.isEmpty() ? QCoreApplication::translate("jp::deck::Output", "Slide %1").arg(slide + 1) : title, false, QStringLiteral("Made with JeffDeck"));
}

QStringList slidePictureNames(const Document &d, const SlidePicturesOptions &o)
{
    QStringList names;
    for (int i = 0; i < d.pages.size(); ++i)
        if (o.hidden || !d.pages[i]->hidden) names << slidePictureName(i, formatOf(o));
    return names;
}

int slidePictureWidth(const Document &d, int width)
{
    const QSizeF size = d.pageSize();
    width = std::clamp(width, 16, 16000);
    if (size.width() <= 0 || size.height() <= 0) return width;
    // (the height follows the slide's shape: width * height <= the most pixels)
    const double most = std::sqrt(double(kMostSlidePicturePixels) * size.width() / size.height());
    return std::max(16, std::min(width, int(std::floor(most))));
}

SlidePicturesResult exportSlidePictures(const Document &d, LayoutCache *cache, const SlidePicturesOptions &o)
{
    SlidePicturesResult result;
    const QDir dir(o.folder);
    if (o.folder.isEmpty() || !dir.exists()) {
        result.error = QCoreApplication::translate("jp::deck::Output", "The folder \"%1\" is not there.").arg(o.folder);
        return result;
    }
    result.widthUsed = slidePictureWidth(d, o.width);
    const int total = int(slidePictureNames(d, o).size());
    int done = 0;
    for (int i = 0; i < d.pages.size(); ++i) {
        if (d.pages[i]->hidden && !o.hidden) continue;
        const SlidePictureFormat format = formatOf(o);
        const QString name = slidePictureName(i, format);
        // (Whole or not at all: the old file stays if this one fails.)
        QSaveFile file(dir.filePath(name));
        bool made = false;
        QString pictureWhy;
        if (format == SlidePictureFormat::Svg) {
            QString refused;
            const QByteArray svg = slideSvg(d, cache, i, &refused);
            if (!refused.isEmpty()) {
                result.error = refused;
                return result;
            }
            made = !svg.isEmpty() && file.open(QIODevice::WriteOnly) && file.write(svg) == svg.size();
        } else {
            const QImage img = slideImage(d, cache, i, result.widthUsed);
            made = !img.isNull() && file.open(QIODevice::WriteOnly) && writePicture(&file, img, PictureFormat(int(format)), 92, &pictureWhy);
        }
        if (!made || !file.commit()) {
            result.error = file.error() != QFileDevice::NoError ? file.errorString() : !pictureWhy.isEmpty() ? pictureWhy : QCoreApplication::translate("jp::deck::Output", "The picture of slide %1 can't be made.").arg(i + 1);
            return result;
        }
        result.written << name;
        ++done;
        if (o.progress && !o.progress(done, total)) {
            result.cancelled = true;
            return result;
        }
    }
    return result;
}

} // namespace jp::deck
