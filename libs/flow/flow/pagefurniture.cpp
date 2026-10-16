// What a page has besides its text: the header and the footer, and the room they take. (The paginator's functions for them are here, apart from
// its layout of the text.)

#include "flow/paginator.h"

#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::flow {

namespace {

// A header or footer never leaves the text less than this much of the page's height.
constexpr double kLeastBody = 36;

} // namespace

// The height a header or footer story takes laid out at `width`: its lines from the top to the bottom of the last.
double Paginator::storyHeight(const QTextDocument *story, double width, int number)
{
    const auto key = std::make_tuple(story, int(std::lround(width * 100)), story->revision());
    const auto found = m_storyHeights.find(key);
    if (found != m_storyHeights.end()) return found->second;
    FrameSpec fs;
    fs.size = QSizeF(width, 100000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.columns = 1;
    fs.hyphenate = false;
    fs.ctx.doc = m_doc->fields;
    fs.ctx.pageNumber = number;
    fs.ctx.pageCount = std::max(number, int(m_starts.size()));
    StoryLayout layout;
    layout.build(story, {fs}, m_doc->env);
    const double height = layout.usedHeight(0);
    // (A story is measured once, at the first page that uses it: a page number is no wider on the next.)
    if (m_storyHeights.size() > 4096) m_storyHeights.clear();
    m_storyHeights.emplace(key, height);
    return height;
}

Paginator::Furniture Paginator::furnitureOf(int index, const PageStart &st, int number)
{
    const Section &sec = m_doc->section(st.section);
    Furniture f;
    f.body = sec.content();
    const bool first = index <= 0 || (index - 1 < int(m_starts.size()) && m_starts[index - 1].section != st.section);
    f.header = sec.headerFor(first, number);
    f.footer = sec.footerFor(first, number);
    if (!f.header && !f.footer) return f;
    const double width = sec.content().width();
    double top = sec.margins.top(), bottom = sec.margins.bottom();
    if (f.header) {
        f.headerHeight = storyHeight(f.header, width, number);
        top = std::max(top, sec.headerDistance + f.headerHeight);
    }
    if (f.footer) {
        f.footerHeight = storyHeight(f.footer, width, number);
        bottom = std::max(bottom, sec.footerDistance + f.footerHeight);
    }
    const double pageHeight = sec.pageSize().height();
    if (pageHeight - top - bottom < kLeastBody) {
        // (More room than the page has: the text keeps what it can of its own margins.)
        top = std::min(top, std::max(sec.margins.top(), (pageHeight - kLeastBody) / 2));
        bottom = std::min(bottom, std::max(sec.margins.bottom(), pageHeight - top - kLeastBody));
    }
    f.body.setTop(top);
    f.body.setBottom(pageHeight - bottom);
    return f;
}

void Paginator::addFurniture(Document *d, const Furniture &f, const Section &sec, int section, int number)
{
    // The counts the page's NUMPAGES and SECTIONPAGES fields show.
    d->flowPageTotal = std::max(1, int(m_starts.size()));
    int inSection = 0;
    for (const PageStart &s : m_starts) inSection += s.section == section;
    d->flowSectionPageTotal = std::max(1, inSection);
    const QRectF content = sec.content();
    auto place = [&](const QTextDocument *story, double top, double height, const char *name) {
        if (!story) return;
        auto s = std::make_shared<Story>();
        s->id = QStringLiteral("flow-%1-%2").arg(QLatin1String(name)).arg(d->pages[0]->id);
        s->doc.reset(story->clone());
        d->stories.insert(s->id, s);
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("flow-%1-box-%2").arg(QLatin1String(name)).arg(d->pages[0]->id);
        t->rect = QRectF(content.left(), top, content.width(), height + 0.005);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->columns = 1;
        t->hyphenate = false;
        t->storyId = s->id;
        t->wrap.mode = Wrap::None;
        d->pages[0]->items.push_back(t);
    };
    place(f.header, sec.headerDistance, f.headerHeight, "header");
    place(f.footer, sec.pageSize().height() - sec.footerDistance - f.footerHeight, f.footerHeight, "footer");
}

} // namespace jp::flow
