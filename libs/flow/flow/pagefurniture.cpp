// What a page has besides its text: the header and the footer, and the room they take. (The paginator's functions for them are here, apart from
// its layout of the text.)

#include "flow/paginator.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::flow {

namespace {

// A header or footer never leaves the text less than this much of the page's height.
constexpr double kLeastBody = 36;

} // namespace

// The height a header or footer story takes laid out at `width`: its lines from the top to the bottom of the last, and (a header's) the space after that.
double Paginator::storyHeight(const QTextDocument *story, double width, int number, bool withSpaceAfter, double rightRoom)
{
    const auto key = storyKey(story, width, rightRoom);
    const auto found = m_storyHeights.find(key);
    if (found != m_storyHeights.end()) return found->second;
    if (storyHasTables(story)) {
        // As tall as its text and its tables together, measured once, at the first page that uses it and laid out as for page 1 of a document of one page: the pages are made before
        // their count is known, and a story is measured for all of them as a story without tables is. (A table that the page's number or count makes wrap differently, as the
        // tenth page's two digits may, is then not as tall as the room kept for it: it is drawn as the page's own number has it.)
        const std::shared_ptr<const HeaderTables> laid = headerTablesOf(story, width, 1, rightRoom, withSpaceAfter, 1, 1);
        if (m_storyLines.size() > 4096) m_storyLines.clear();
        m_storyLines[key] = laid->lines;   // (what its paragraphs anchor is set from these)
        if (m_storyHeights.size() > 4096) m_storyHeights.clear();
        m_storyHeights.emplace(key, laid->height);
        return laid->height;
    }
    FrameSpec fs;
    fs.size = QSizeF(width, 100000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.columns = 1;
    fs.hyphenate = false;
    fs.rightRoom = rightRoom;
    fs.ctx.doc = m_doc->fields;
    fs.ctx.pageNumber = number;
    fs.ctx.pageCount = std::max(number, int(m_starts.size()));
    StoryLayout layout;
    layout.build(story, {fs}, m_doc->env);
    double height = layout.usedHeight(0) + (withSpaceAfter ? layout.trailingSpace() : 0.0);
    const QVector<StoryLayout::LineInfo> lines = layout.lineInfo(0);
    // The paragraphs set out of the text (a frame, as the page number's) are drawn where the paragraph after them begins and take no room there; but the story is as tall as
    // they are, one under another, from there: as the lowest of them reaches.
    {
        bool inFrame = false;
        double top = 0, reach = 0;
        for (const StoryLayout::LineInfo &l : lines) {
            if (!story->findBlock(l.docStart).blockFormat().boolProperty(tp::OutOfFlow)) {
                inFrame = false;
                continue;
            }
            if (!inFrame) {
                inFrame = true;
                top = l.rect.top();
                reach = 0;
            }
            reach += l.rect.height();
            height = std::max(height, top + reach);
        }
    }
    if (m_storyLines.size() > 4096) m_storyLines.clear();
    m_storyLines[key] = lines;
    // (A story is measured once, at the first page that uses it: a page number is no wider on the next.)
    if (m_storyHeights.size() > 4096) m_storyHeights.clear();
    m_storyHeights.emplace(key, height);
    return height;
}

namespace {

// What the fields of a story shown on a page that the document has pages of: the page's number, the document's page count, the section's, from the document's notes' source for
// the rest.
class PageFields : public FieldSource {
public:
    PageFields(const FieldSource *base, int pages, int sectionPages) : m_base(base), m_pages(pages), m_sectionPages(sectionPages) {}
    int firstPageNumber() const override { return m_base ? m_base->firstPageNumber() : 1; }
    QString pageNumberFormat() const override { return m_base ? m_base->pageNumberFormat() : QString(); }
    QString businessField(const QString &key) const override { return m_base ? m_base->businessField(key) : QString(); }
    QStringList mergeFields() const override { return m_base ? m_base->mergeFields() : QStringList(); }
    QString mergeValue(int record, const QString &field) const override { return m_base ? m_base->mergeValue(record, field) : QString(); }
    const QTextDocument *storyDoc(const QString &id) const override { return m_base ? m_base->storyDoc(id) : nullptr; }
    QList<const QTextDocument *> storyDocs() const override { return m_base ? m_base->storyDocs() : QList<const QTextDocument *>(); }
    int pageTotal() const override { return m_pages; }
    int sectionPageTotal() const override { return m_sectionPages; }
    int footnotesBefore() const override { return m_base ? m_base->footnotesBefore() : 0; }
    int endnotesBefore() const override { return m_base ? m_base->endnotesBefore() : 0; }
    QStringList carriedNotes() const override { return m_base ? m_base->carriedNotes() : QStringList(); }
    double noteSeparatorHeight() const override { return m_base ? m_base->noteSeparatorHeight() : 0; }
    double noteNoticeHeight() const override { return m_base ? m_base->noteNoticeHeight() : 0; }
    double noteContinuationHeight() const override { return m_base ? m_base->noteContinuationHeight() : 0; }

private:
    const FieldSource *m_base;
    int m_pages, m_sectionPages;
};

// The most cells of the tables of headers and footers that are kept laid out, and the most layouts (a story that differs on each page is laid out for each: only the pages being
// drawn need theirs).
constexpr qint64 kMostHeaderTableCells = 2000;
constexpr size_t kMostHeaderTableLayouts = 64;

} // namespace

Paginator::StoryFacts Paginator::storyFacts(const QTextDocument *story)
{
    const auto key = std::make_pair(story, story->revision());
    if (const auto found = m_storyFacts.find(key); found != m_storyFacts.end()) return found->second;
    StoryFacts facts;
    if (!m_doc->tables.isEmpty())
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            facts.tables |= b.blockFormat().intProperty(fp::Cell) != 0;
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                facts.page |= code == QLatin1String("page") || code.startsWith(QLatin1String("page:"));
                facts.pages |= code == QLatin1String("pages");
                facts.sectionPages |= code == QLatin1String("sectionpages");
            }
        }
    if (m_storyFacts.size() > 4096) m_storyFacts.clear();
    m_storyFacts.emplace(key, facts);
    return facts;
}

std::shared_ptr<const Paginator::HeaderTables> Paginator::headerTablesOf(const QTextDocument *story, double width, int number, double rightRoom, bool withSpaceAfter, int pages, int sectionPages)
{
    const StoryFacts facts = storyFacts(story);
    const HeaderTablesKey key{story, int(std::lround(width * 100)), story->revision(), int(std::lround(rightRoom * 100)), withSpaceAfter, facts.page ? number : 0, facts.pages ? pages : 0, facts.sectionPages ? sectionPages : 0};
    if (const auto found = m_headerTables.find(key); found != m_headerTables.end()) {
        // (The one used last is the last to go.)
        if (const auto at = std::find(m_headerTablesOrder.begin(), m_headerTablesOrder.end(), key); at != m_headerTablesOrder.end()) {
            m_headerTablesOrder.erase(at);
            m_headerTablesOrder.push_back(key);
        }
        return found->second;
    }
    ++m_headerTablesMade;
    auto out = std::make_shared<HeaderTables>();
    out->showsTotal = facts.pages || facts.sectionPages;
    // The story in a document of its own, with the tables the file has (the cells' records are shared) and what its fields show: the table machinery lays out the tables of a
    // story it is given the document of.
    out->fields = std::make_shared<PageFields>(m_doc->fields, pages, sectionPages);
    out->doc = std::make_shared<FlowDocument>();
    out->doc->adoptStory(cloneStory(story));
    out->doc->tables = m_doc->tables;
    out->doc->images = m_doc->images;
    out->doc->env = m_doc->env;
    out->doc->fields = out->fields.get();
    out->doc->fieldPageNumber = number;
    out->doc->fieldPageCount = pages;
    out->flow = std::make_shared<TableFlow>(out->doc.get());
    const QTextDocument *copy = out->doc->story();
    const TableIndex &index = out->flow->index();
    const int end = copy->characterCount();   // (the stretch after the last table ends past the last paragraph's mark: an empty last paragraph is a line of it)
    double y = 0;
    // The stretch of paragraphs [from, to) laid out at the width, one under another.
    auto stretch = [&](int from, int to, bool last) {
        if (to <= from) return;
        FrameSpec fs;
        fs.size = QSizeF(std::max(1.0, width), 100000);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.columns = 1;
        fs.hyphenate = false;
        fs.rightRoom = rightRoom;
        fs.ctx.doc = out->fields.get();
        fs.ctx.pageNumber = number;
        fs.ctx.pageCount = pages;
        StoryLayout::Span span;
        span.from = from;
        span.end = to;
        span.keepRules = false;
        StoryLayout layout;
        layout.build(copy, {fs}, out->doc->env, span);
        double height = layout.usedHeight(0);
        if (!last || withSpaceAfter) height += layout.trailingSpace();
        // The paragraphs set out of the text (a frame, as the page number's) take no room where they are; but the stretch is as tall as the lowest of them reaches.
        const QVector<StoryLayout::LineInfo> lines = layout.lineInfo(0);
        {
            bool inFrame = false;
            double top = 0, reach = 0;
            for (const StoryLayout::LineInfo &l : lines) {
                if (!copy->findBlock(l.docStart).blockFormat().boolProperty(tp::OutOfFlow)) {
                    inFrame = false;
                    continue;
                }
                if (!inFrame) {
                    inFrame = true;
                    top = l.rect.top();
                    reach = 0;
                }
                reach += l.rect.height();
                height = std::max(height, top + reach);
            }
        }
        for (StoryLayout::LineInfo l : lines) {
            l.rect.translate(0, y);
            out->lines << l;
        }
        out->parts.push_back(HeaderPart{y, height, from, to, nullptr});
        y += height;
    };
    int at = 0;
    const auto &spans = index.spans();
    for (size_t i = 0; i < spans.size(); ++i) {
        stretch(at, spans[i].from, false);
        if (const std::shared_ptr<TableLayout> table = out->flow->layout(int(i)); table && table->rowCount() > 0) {
            if (const std::shared_ptr<TableFragment> fragment = out->flow->fragment(table, 0, table->rowCount(), 1e9)) {
                out->parts.push_back(HeaderPart{y, fragment->height, spans[i].from, spans[i].to, fragment});
                out->cells += table->cellCount();
                // The lines of its cells, for the objects they anchor (from the table's place in the story's width).
                const QPointF at(table->leftEdgeIn(width), y);
                fragment->forLines([&](const StoryLayout::LineBrief &ln, const QRectF &rect) {
                    StoryLayout::LineInfo l;
                    l.rect = rect.translated(at);
                    l.below = ln.below;
                    l.docStart = ln.block;
                    l.docPos = ln.pos;
                    l.docEnd = ln.end;
                    l.above = ln.above;
                    out->lines << l;
                });
                y += fragment->height;
            }
        }
        at = spans[i].to;
    }
    stretch(at, end, true);
    out->height = y;
    // Kept, the one used longest ago given up when the layouts or their cells are many.
    m_headerTables.emplace(key, out);
    m_headerTablesOrder.push_back(key);
    m_headerTableCells += out->cells;
    while (m_headerTablesOrder.size() > 1 && (m_headerTableCells > kMostHeaderTableCells || m_headerTablesOrder.size() > kMostHeaderTableLayouts)) {
        if (const auto old = m_headerTables.find(m_headerTablesOrder.front()); old != m_headerTables.end()) {
            m_headerTableCells -= old->second->cells;
            m_headerTables.erase(old);
        }
        m_headerTablesOrder.pop_front();
    }
    return out;
}

double Paginator::headerObjectsBottom(const QTextDocument *story, const Section &sec, int number)
{
    if (m_doc->objects.isEmpty()) return 0;
    const QRectF content = sec.content();
    // (As the story's height is: once, where the story and the page's places are what they were.)
    const QString places = QStringLiteral("%1 %2 %3 %4 %5 %6").arg(content.left()).arg(content.width()).arg(sec.headerDistance).arg(sec.pageSize().width()).arg(sec.pageSize().height()).arg(sec.margins.top());
    const auto cacheKey = std::make_tuple(story, story->revision(), places);
    if (const auto found = m_headerBottoms.find(cacheKey); found != m_headerBottoms.end()) return found->second;
    ++m_headerReachesMade;
    storyHeight(story, content.width(), number, false, sec.margins.right());   // (laying the story out is what has its lines to hand)
    const auto lines = m_storyLines.find(storyKey(story, content.width(), sec.margins.right()));
    if (lines == m_storyLines.end()) return 0;
    QVector<AnchorLine> held;
    for (const StoryLayout::LineInfo &l : lines->second) held << AnchorLine{l.docStart, l.docPos, l.docEnd, l.rect.translated(content.left(), sec.headerDistance), content.left(), 0.0, false, l.docEnd, l.above};
    double bottom = 0;
    int seen = 0;
    for (const Anchor &a : anchorsIn(story, 0, story->characterCount(), held))
        for (int id = a.object; id > 0 && id <= m_doc->objects.size() && seen < kMostObjects; id = objectAt(a, id).next, ++seen) {
            const FloatingObject &o = objectAt(a, id);
            using W = FloatingObject::Wrap;
            if (!o.item || o.behind || (o.wrap != W::Square && o.wrap != W::Tight && o.wrap != W::TopBottom)) continue;
            const QRectF r = placeOf(o, a, sec);
            if (r.right() <= content.left() || r.left() >= content.right()) continue;   // (beside the text area: the text is not under it)
            bottom = std::max(bottom, r.bottom() + o.gap.bottom());
        }
    if (m_headerBottoms.size() > 4096) m_headerBottoms.clear();
    m_headerBottoms.emplace(cacheKey, bottom);
    return bottom;
}

Paginator::Furniture Paginator::furnitureOf(int index, const PageStart &st, int number)
{
    const Section &sec = m_doc->section(st.section);
    Furniture f;
    f.body = sec.content();
    if (st.blank) return f;   // (a blank page that a section break leaves has no header or footer)
    const bool first = index <= 0 || st.firstOfSection;
    f.header = sec.headerFor(first, number);
    f.footer = sec.footerFor(first, number);
    if (!f.header && !f.footer) return f;
    const double width = sec.content().width();
    double top = sec.margins.top(), bottom = sec.margins.bottom();
    if (f.header) {
        f.headerHeight = storyHeight(f.header, width, number, true, sec.margins.right());
        top = std::max(top, sec.headerDistance + f.headerHeight);
        top = std::max(top, headerObjectsBottom(f.header, sec, number));
    }
    if (f.footer) {
        f.footerHeight = storyHeight(f.footer, width, number, false, sec.margins.right());
        bottom = std::max(bottom, sec.footerDistance + f.footerHeight);
    }
    const double pageHeight = sec.pageSize().height();
    if (pageHeight - top - bottom < kLeastBody) {
        // (More room than the page has: the text keeps what it can of its own margins.)
        top = std::min(top, std::max(sec.margins.top(), (pageHeight - kLeastBody) / 2));
        bottom = std::min(bottom, std::max(sec.margins.bottom(), pageHeight - top - kLeastBody));
    }
    f.body.setTop(top);
    f.body.setBottom(std::max(top + kLeastBody, pageHeight - bottom));   // (the least a text area is, as Section::content() has it)
    return f;
}

void Paginator::addPageBorders(Document *d, const Section &sec, bool firstOfSection, const Furniture &f)
{
    const PageBorders &b = sec.pageBorders;
    if (!b.any() || (b.show == PageBorders::Show::FirstPage && !firstOfSection) || (b.show == PageBorders::Show::NotFirstPage && firstOfSection)) return;
    const double width = sec.pageSize().width(), height = sec.pageSize().height();
    // The edges the borders are measured from: the page's, or the text area's with the header and the footer inside it.
    double top = 0, left = 0, bottom = height, right = width;
    if (!b.fromPage) {
        top = f.header ? std::min(sec.margins.top(), sec.headerDistance) : sec.margins.top();
        bottom = height - (f.footer ? std::min(sec.margins.bottom(), sec.footerDistance) : sec.margins.bottom());
        left = sec.margins.left();
        right = width - sec.margins.right();
    }
    // Where each side's line is centered: its outer edge is `space` out from the edge it is measured from (in from the page's), the line's own width inside that.
    // (A double border is two lines and the gap of a line's width between them.)
    auto total = [](const PageBorders::Side &s) { return s.line == PageBorders::Side::Line::Double ? 3 * s.width : s.width; };
    auto at = [&](const PageBorders::Side &s, double edge, int outward) {
        const double c = edge + outward * (s.space + total(s) / 2);
        return c;
    };
    const double up = at(b.top, top, b.fromPage ? 1 : -1), down = at(b.bottom, bottom, b.fromPage ? -1 : 1);
    const double l = at(b.left, left, b.fromPage ? 1 : -1), r = at(b.right, right, b.fromPage ? -1 : 1);
    auto line = [&](const PageBorders::Side &s, QPointF p1, QPointF p2, const char *name) {
        if (!s.present || s.width <= 0) return;
        auto item = std::make_shared<LineItem>();
        item->id = QStringLiteral("flow-page-border-%1-%2").arg(d->pages[0]->id, QLatin1String(name));
        item->p1 = p1;
        item->p2 = p2;
        item->stroke = Stroke::line(ColorRef::fromString(s.color.name()), total(s));
        if (s.line == PageBorders::Side::Line::Double) item->stroke.compound = Stroke::Double;
        else if (s.line == PageBorders::Side::Line::Dotted) item->stroke.dash = Stroke::RoundDot;
        else if (s.line == PageBorders::Side::Line::Dashed) item->stroke.dash = Stroke::DashLine;
        item->wrap.mode = Wrap::None;
        item->syncRect();
        d->pages[0]->items.push_back(item);
    };
    // (The horizontal lines reach out to the vertical ones' outer edges, so the corners meet.)
    const double reachL = l - (b.left.present ? total(b.left) / 2 : 0), reachR = r + (b.right.present ? total(b.right) / 2 : 0);
    line(b.top, QPointF(reachL, up), QPointF(reachR, up), "top");
    line(b.bottom, QPointF(reachL, down), QPointF(reachR, down), "bottom");
    line(b.left, QPointF(l, up), QPointF(l, down), "left");
    line(b.right, QPointF(r, up), QPointF(r, down), "right");
}

void Paginator::addFurniture(Document *d, const Furniture &f, const Section &sec, int section, int number, MaterializedPage *page)
{
    // The counts the page's NUMPAGES and SECTIONPAGES fields show.
    d->flowPageTotal = std::max(1, int(m_starts.size()));
    int inSection = 0;
    for (const PageStart &s : m_starts) inSection += s.section == section;
    d->flowSectionPageTotal = std::max(1, inSection);
    const QRectF content = sec.content();
    QVector<Anchor> anchors;
    auto place = [&](const QTextDocument *story, double top, double height, const char *name) {
        if (!story) return;
        // The objects the story's paragraphs anchor are where its lines put them (held: the lines, from the story's top and left, put at the place the story is drawn).
        auto anchorObjects = [&](const QVector<StoryLayout::LineInfo> &lines) {
            QVector<AnchorLine> held;
            for (const StoryLayout::LineInfo &l : lines) held << AnchorLine{l.docStart, l.docPos, l.docEnd, l.rect.translated(content.left(), top), content.left(), 0.0, false, l.docEnd, l.above};
            anchors << anchorsIn(story, 0, story->characterCount(), held);
        };
        // A story with tables is drawn in its stretches of text and its tables, each at the place the layout gave it.
        if (page && storyHasTables(story)) {
            const auto laid = headerTablesOf(story, content.width(), number, sec.margins.right(), std::string(name) == "header", d->flowPageTotal, d->flowSectionPageTotal);
            page->showsTotal = page->showsTotal || laid->showsTotal;
            int stretches = 0;
            for (const HeaderPart &part : laid->parts) {
                if (part.table) {
                    auto fragment = std::make_shared<TableFragment>(*part.table);
                    fragment->origin = QPointF(content.left() + fragment->layout->leftEdgeIn(content.width()), top + part.y);
                    page->furnitureTables.push_back(fragment);
                    continue;
                }
                // (The text of the stretch, as it is in the story: its paragraphs without the separator that ends the last.)
                auto text = std::make_unique<QTextDocument>();
                text->setDocumentMargin(story->documentMargin());
                text->setDefaultFont(story->defaultFont());
                text->setDefaultTextOption(story->defaultTextOption());
                {
                    QTextCursor src(const_cast<QTextDocument *>(story));
                    src.setPosition(part.from);
                    src.setPosition(std::max(part.from, part.to - 1), QTextCursor::KeepAnchor);
                    QTextCursor(text.get()).insertFragment(QTextDocumentFragment(src));
                }
                QTextCursor(text->begin()).setBlockFormat(story->findBlock(part.from).blockFormat());
                auto s = std::make_shared<Story>();
                s->id = QStringLiteral("flow-%1-%2-%3").arg(QLatin1String(name)).arg(d->pages[0]->id).arg(stretches);
                s->doc = std::move(text);
                resolveImages(s->doc.get(), d);
                d->stories.insert(s->id, s);
                auto t = std::make_shared<TextItem>();
                t->id = QStringLiteral("flow-%1-box-%2-%3").arg(QLatin1String(name)).arg(d->pages[0]->id).arg(stretches++);
                t->rect = QRectF(content.left(), top + part.y, content.width(), part.height + 0.005);
                t->insets = QMarginsF(0, 0, 0, 0);
                t->columns = 1;
                t->hyphenate = false;
                t->rightRoom = sec.margins.right();
                t->storyId = s->id;
                t->wrap.mode = Wrap::None;
                d->pages[0]->items.push_back(t);
            }
            if (!m_doc->images.isEmpty()) resolveCellImages(page, d);
            if (!m_doc->objects.isEmpty()) anchorObjects(laid->lines);
            anchors << storyFrameAnchors(story, content.left(), content.width(), top);
            return;
        }
        auto s = std::make_shared<Story>();
        s->id = QStringLiteral("flow-%1-%2").arg(QLatin1String(name)).arg(d->pages[0]->id);
        s->doc = cloneStory(story);
        resolveImages(s->doc.get(), d);
        if (!m_doc->objects.isEmpty()) {
            // The objects set in the header or footer are where its lines put them.
            storyHeight(story, content.width(), number, false, sec.margins.right());
            const auto lines = m_storyLines.find(storyKey(story, content.width(), sec.margins.right()));
            if (lines != m_storyLines.end()) anchorObjects(lines->second);
        }
        // The frames of the story (a letterhead's logo or a page number set at a place on the page) are text boxes where they say.
        anchors << storyFrameAnchors(story, content.left(), content.width(), top);
        d->stories.insert(s->id, s);
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("flow-%1-box-%2").arg(QLatin1String(name)).arg(d->pages[0]->id);
        t->rect = QRectF(content.left(), top, content.width(), height + 0.005);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->columns = 1;
        t->hyphenate = false;
        t->rightRoom = sec.margins.right();   // (the room the header's lines were measured with)
        t->storyId = s->id;
        t->wrap.mode = Wrap::None;
        d->pages[0]->items.push_back(t);
    };
    place(f.header, sec.headerDistance, f.headerHeight, "header");
    place(f.footer, sec.pageSize().height() - sec.footerDistance - f.footerHeight, f.footerHeight, "footer");
    if (!anchors.isEmpty()) addObjects(d, anchors, sec, number);
}

void Paginator::addNotes(Document *d, const QVector<StoryLayout::NotePlace> &places, const QVector<StoryLayout::Rule> &rules, int firstFrame, const QStringList &ids,
                         const QVector<QRectF> &cols, const QRectF &body)
{
    if (!m_doc->fields) return;
    auto at = [&](int frame) { return frame - firstFrame; };
    int made = 0;
    for (const StoryLayout::NotePlace &pl : places) {
        const int column = at(pl.frame);
        if (column < 0 || column >= cols.size() || !ids.contains(pl.storyId) || made >= 2000) continue;
        const QTextDocument *source = m_doc->fields->storyDoc(pl.storyId);
        if (!source) continue;
        // The note's text from where this part of it starts, with its number where the text begins.
        // (Of a note of many thousand characters, the part that its box can show, and a good deal more; a page would otherwise copy all of it.)
        const int total = source->characterCount() - 1;
        const int wanted = std::clamp(int(pl.rect.height() * pl.rect.width() / 8) + 2000, 4000, 4000000);
        const bool whole = pl.from <= 0 && total <= wanted;
        std::unique_ptr<QTextDocument> text;
        if (whole) {
            text = cloneStory(source);
        } else {
            const int from = std::clamp(pl.from, 0, std::max(0, total));
            text = std::make_unique<QTextDocument>();
            text->setDocumentMargin(source->documentMargin());
            text->setDefaultFont(source->defaultFont());
            text->setDefaultTextOption(source->defaultTextOption());
            {
                QTextCursor src(const_cast<QTextDocument *>(source));
                src.setPosition(from);
                src.setPosition(std::min(total, from + wanted), QTextCursor::KeepAnchor);
                QTextCursor(text.get()).insertFragment(QTextDocumentFragment(src));
            }
            QTextBlockFormat bf = source->findBlock(from).blockFormat();
            if (from > source->findBlock(from).position() || pl.from > 0) bf.setTextIndent(0);
            QTextCursor(text->begin()).setBlockFormat(bf);
        }
        QVector<std::pair<int, int>> marks;
        for (QTextBlock b = text->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("notemark")) marks.push_back({it.fragment().position(), it.fragment().length()});
        for (const auto &m : marks) {
            QTextCursor c(text.get());
            c.setPosition(m.first);
            c.setPosition(m.first + m.second, QTextCursor::KeepAnchor);
            QTextCharFormat cf;
            cf.setProperty(tp::Field, QLatin1String("raw:") + (pl.number > 0 ? QString::number(pl.number) : QString()));
            c.mergeCharFormat(cf);
        }
        auto s = std::make_shared<Story>();
        s->id = QStringLiteral("flow-note-%1-%2").arg(d->pages[0]->id).arg(made);
        s->doc = std::move(text);
        resolveImages(s->doc.get(), d);
        d->stories.insert(s->id, s);
        const QRectF col = cols[column];
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("flow-note-box-%1-%2").arg(d->pages[0]->id).arg(made);
        t->rect = QRectF(col.left() + pl.rect.left(), body.top() + col.top() + pl.rect.top(), pl.rect.width(), pl.rect.height() + 0.005);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->columns = 1;
        t->hyphenate = m_doc->hyphenate;
        t->hyphenZone = m_doc->hyphenZone;
        t->storyId = s->id;
        t->wrap.mode = Wrap::None;
        d->pages[0]->items.push_back(t);
        ++made;
    }
    // The rule above the notes of each column.
    int ruled = 0;
    for (const StoryLayout::Rule &r : rules) {
        const int column = at(r.frame);
        if (column < 0 || column >= cols.size() || made == 0 || ruled >= cols.size()) continue;
        const QRectF col = cols[column];
        auto line = std::make_shared<LineItem>();
        line->id = QStringLiteral("flow-note-rule-%1-%2").arg(d->pages[0]->id).arg(ruled++);
        line->p1 = QPointF(col.left() + r.line.x1(), body.top() + col.top() + r.line.y1());
        line->p2 = QPointF(col.left() + r.line.x2(), body.top() + col.top() + r.line.y2());
        line->stroke.width = 0.5;
        line->stroke.color = ColorRef::fromString(QStringLiteral("#000000"));
        line->wrap.mode = Wrap::None;
        line->syncRect();
        d->pages[0]->items.push_back(line);
    }
}

} // namespace jp::flow
