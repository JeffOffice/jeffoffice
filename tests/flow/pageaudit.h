#pragma once
// What the tests of the flow (tests/flow) and of the word processor (tests/write) both ask of a paginator's pages. A page is written out as text in its
// parts (its lines, the rows of its tables, its notes, its headers and footers, its boxes), so that two paginators of the same document can be compared
// part by part and a difference says which part it is in; and the rules the parts keep to one another are checked: a table inside the text area that
// the header and footer leave, a note on the page whose lines cite it and below what it cites, the page's text where the paginator's lines say it is.
// Nothing here knows a font: every expectation is made from the layout's own metrics.

#include "core/document.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QMap>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>
#include <memory>

namespace jp::flowaudit {

inline QString num(double v) { return QString::number(std::round(v * 100) / 100, 'f', 2); }
inline QString rectText(const QRectF &r) { return QStringLiteral("%1,%2 %3x%4").arg(num(r.left()), num(r.top()), num(r.width()), num(r.height())); }

// Whether two rectangles share more than a hair of area (touching, or a rounding apart, is not overlapping).
inline bool overlap(const QRectF &a, const QRectF &b)
{
    const QRectF s = a.intersected(b);
    return s.width() > 0.05 && s.height() > 0.05;
}

inline QString shown(QString text, int most = 30)
{
    text.replace(QLatin1Char('\n'), QLatin1Char('|'));
    text.replace(QChar::ObjectReplacementCharacter, QLatin1Char('#'));
    text.replace(QChar(0x200B), QLatin1Char('_'));
    return text.left(most);
}

// A page as text, in the parts that a person sees.
struct Facts {
    bool made = false;
    QString start;              // where it starts, its number, its area, and the flags of its first and last lines
    QStringList lines;          // the lines of its story (the rows of tables among them, a line each)
    QStringList rows;           // the tables on it: the rows, where, and the height of each
    QStringList notes;          // the boxes of its notes and the rules above them
    QStringList furniture;      // its header and footer
    QStringList boxes;          // everything else: the text boxes of the body, and the objects
};

inline Facts factsOf(flow::Paginator &pg, int k, bool made = true)
{
    Facts f;
    const flow::Paginator::Page page = pg.page(k);
    const flow::Paginator::PageStart &s = page.start;
    f.start = QStringLiteral("pos %1 skip %2 hard %3 section %4 first-of-section %5 piece %6/%7 foot %8 end %9 carried [%10] number %11 area %12 columns %13")
                  .arg(s.pos).arg(s.skip).arg(s.hard).arg(s.section).arg(s.firstOfSection).arg(s.piece).arg(s.pieceRoom).arg(s.foot).arg(s.end)
                  .arg(s.carried.join(QLatin1Char(','))).arg(page.number).arg(rectText(page.content)).arg(page.columns);
    for (const auto &l : pg.lines(k))
        f.lines << QStringLiteral("%1-%2 line %3/%4 block %5 frame %6 at %7 below %8").arg(l.pos).arg(l.end).arg(l.line).arg(l.lines).arg(l.block).arg(l.frame).arg(rectText(l.rect), num(l.below));
    if (!made) return f;
    const std::unique_ptr<flow::MaterializedPage> m = pg.materialize(k);
    if (!m) return f;
    f.made = true;
    f.start += QStringLiteral(" | story from %1 continues %2/%3 section %4 total %5/%6").arg(m->storyFrom).arg(m->continuesFromBefore).arg(m->continuesOnAfter).arg(m->section)
                   .arg(m->showsTotal).arg(m->showsTotal ? m->pageTotal : 0);
    for (const auto &t : m->tables) {
        QString heights;
        for (int r = t->firstRow; r < t->endRow; ++r) heights += num(t->heightOf(r)) + QLatin1Char(',');
        f.rows << QStringLiteral("rows %1-%2 header %3 at %4 heights %5").arg(t->firstRow).arg(t->endRow).arg(t->header).arg(rectText(t->rect()), heights);
    }
    for (const auto &item : m->doc->pages[0]->items) {
        QString text, fields;
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const Story *st = m->doc->story(t->storyId)) {
                text = st->doc->toPlainText();
                for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it) {
                        const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                        if (!code.isEmpty()) fields += code + QLatin1Char(' ');
                    }
            }
        const QString line = QStringLiteral("%1 %2 [%3] %4").arg(int(item->type())).arg(rectText(item->rect), shown(text, 40), fields);
        if (item->id.startsWith(QLatin1String("flow-header-box")) || item->id.startsWith(QLatin1String("flow-footer-box")))
            f.furniture << (item->id.contains(QLatin1String("header")) ? QStringLiteral("header ") : QStringLiteral("footer ")) + QStringLiteral("%1 [%2] %3").arg(rectText(item->rect), shown(text, 200), fields);
        else if (item->id.startsWith(QLatin1String("flow-note-")))
            f.notes << line;
        else
            f.boxes << line;
    }
    return f;
}

inline QString firstDifference(const QString &what, const QStringList &a, const QStringList &b)
{
    for (int i = 0; i < std::max<int>(a.size(), b.size()); ++i)
        if (a.value(i) != b.value(i))
            return QStringLiteral("%1 differ (%2 and %3 afresh): #%4 is [%5] but afresh [%6]").arg(what).arg(a.size()).arg(b.size()).arg(i).arg(a.value(i).left(170), b.value(i).left(170));
    return QString();
}

// The first part of the first page where `incremental` (a paginator that laid the document out before its edits) differs from `fresh` (one made after), or nothing:
// the page count, each page's start, lines, table rows, notes, headers and footers, and boxes. (Without `made`, the start and the lines only: the pages are not
// made, which costs about as much as laying them out.)
inline QString differenceFromFresh(flow::Paginator &incremental, flow::Paginator &fresh, bool made = true)
{
    const int pages = incremental.pageCount(), want = fresh.pageCount();
    if (pages != want) return QStringLiteral("the page count: %1 pages after the edits, %2 when made afresh").arg(pages).arg(want);
    for (int k = 0; k < pages; ++k) {
        const Facts a = factsOf(incremental, k, made), b = factsOf(fresh, k, made);
        if (made && a.made != b.made) return QStringLiteral("page %1: %2 made, %3 when made afresh").arg(k).arg(a.made).arg(b.made);
        if (a.start != b.start) return QStringLiteral("page %1's start: [%2] but afresh [%3]").arg(k).arg(a.start.left(260), b.start.left(260));
        const std::pair<const char *, std::pair<const QStringList *, const QStringList *>> parts[] = {
            {"lines", {&a.lines, &b.lines}}, {"table rows", {&a.rows, &b.rows}}, {"notes", {&a.notes, &b.notes}}, {"headers and footers", {&a.furniture, &b.furniture}}, {"boxes", {&a.boxes, &b.boxes}}};
        for (const auto &p : parts)
            if (const QString d = firstDifference(QLatin1String(p.first), *p.second.first, *p.second.second); !d.isEmpty()) return QStringLiteral("page %1's %2").arg(k).arg(d);
    }
    return QString();
}

// ---- the rules the parts keep

// The notes the story cites, in order (a reference is a character with the field "footnote:<id>"); `restarts`: the numbers begin again after each section break.
inline QStringList referencesIn(const QTextDocument *story)
{
    QStringList out;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QString code = it.fragment().charFormat().stringProperty(tp::Field);
            if (code.startsWith(QLatin1String("footnote:")))
                for (int i = 0; i < it.fragment().length(); ++i) out << code.mid(9);
        }
    return out;
}

// The number each reference of the story shows (from 1 in the story, or in each section when the numbers restart), in the order of `referencesIn`.
inline QVector<int> numbersOf(const QTextDocument *story, bool restarts)
{
    QVector<int> out;
    int n = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:")))
                for (int i = 0; i < it.fragment().length(); ++i) out << ++n;
        if (restarts && b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) n = 0;
    }
    return out;
}

// The notes that the lines of page `k` cite, in order: the references in the paragraphs the lines hold, rows of tables and their cells too.
inline QStringList citedNotes(flow::Paginator &pg, const QTextDocument *story, int k)
{
    QStringList ids;
    for (const auto &ln : pg.lines(k))
        for (QTextBlock b = story->findBlock(ln.pos); b.isValid() && b.position() < ln.end; b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment frag = it.fragment();
                if (frag.position() >= ln.end || frag.position() + frag.length() <= ln.pos) continue;
                const QString code = frag.charFormat().stringProperty(tp::Field);
                if (!code.startsWith(QLatin1String("footnote:"))) continue;
                const int lo = std::max(frag.position(), ln.pos), hi = std::min(frag.position() + frag.length(), ln.end);
                for (int i = lo; i < hi; ++i) ids << code.mid(9);
            }
    return ids;
}

// The items of a made page whose ids begin with a prefix.
inline QVector<const Item *> itemsWith(const flow::MaterializedPage &m, const QString &prefix)
{
    QVector<const Item *> out;
    for (const auto &item : m.doc->pages[0]->items)
        if (item->id.startsWith(prefix)) out << item.get();
    return out;
}

inline QString storyOf(const flow::MaterializedPage &m, const Item *item)
{
    if (const auto *t = dynamic_cast<const TextItem *>(item))
        if (const Story *st = m.doc->story(t->storyId)) return st->doc->toPlainText();
    return QString();
}

// The number a note box begins with (from the field "raw:<n>" its mark was given), or 0.
inline int numberShownIn(const flow::MaterializedPage &m, const Item *item)
{
    if (const auto *t = dynamic_cast<const TextItem *>(item))
        if (const Story *st = m.doc->story(t->storyId))
            for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                    if (code.startsWith(QLatin1String("raw:"))) return code.mid(4).toInt();
                }
    return 0;
}

// What is wrong with the pages of the paginator, or nothing. The parts are checked against one another, from the layout's own metrics:
//  - a table's rows lie inside the text area (what the header and footer leave of the page), and clear of the header, the footer, and the notes;
//  - the text area itself lies between the header and the footer;
//  - a page draws the note of every reference its lines cite (`marks`: the note's id, and a word that only its text has), at the number the reference
//    has in the story, and draws none that its lines don't cite or that did not go on from the page before; and the notes lie inside the text area,
//    below what they are cited by;
//  - every page of the document can be made.
inline QString problems(flow::Paginator &pg, const QTextDocument *story, const QMap<QString, QString> &marks = {}, bool restartNumbers = false)
{
    const int pages = pg.pageCount();
    const QStringList order = referencesIn(story);
    const QVector<int> numbers = numbersOf(story, restartNumbers);
    for (int k = 0; k < pages; ++k) {
        const flow::Paginator::Page page = pg.page(k);
        const QRectF area = page.content;
        const std::unique_ptr<flow::MaterializedPage> m = pg.materialize(k);
        if (!m) return QStringLiteral("page %1 of %2 is not made").arg(k).arg(pages);
        const QVector<const Item *> headers = itemsWith(*m, QStringLiteral("flow-header-box")), footers = itemsWith(*m, QStringLiteral("flow-footer-box"));
        for (const Item *h : headers)
            if (h->rect.bottom() > area.top() + 0.05) return QStringLiteral("page %1: the header ends at %2, below where the text area begins (%3)").arg(k).arg(num(h->rect.bottom()), num(area.top()));
        for (const Item *f : footers)
            if (f->rect.top() < area.bottom() - 0.05) return QStringLiteral("page %1: the footer begins at %2, above where the text area ends (%3)").arg(k).arg(num(f->rect.top()), num(area.bottom()));
        // The tables.
        for (const auto &t : m->tables) {
            const QRectF r = t->rect();
            if (r.top() < area.top() - 0.05)
                return QStringLiteral("page %1: rows %2 to %3 of a table begin at %4, above the text area (%5)").arg(k).arg(t->firstRow).arg(t->endRow).arg(num(r.top()), num(area.top()));
            if (r.bottom() > area.bottom() + 0.05) {
                QString where;
                for (const Item *f : footers) where = QStringLiteral(", the footer begins at %1").arg(num(f->rect.top()));
                return QStringLiteral("page %1: rows %2 to %3 of a table end at %4, below the text area's foot (%5)%6").arg(k).arg(t->firstRow).arg(t->endRow).arg(num(r.bottom()), num(area.bottom()), where);
            }
            for (const Item *f : footers)
                if (overlap(r, f->rect)) return QStringLiteral("page %1: rows %2 to %3 of a table (%4) are over the footer (%5)").arg(k).arg(t->firstRow).arg(t->endRow).arg(rectText(r), rectText(f->rect));
            for (const Item *h : headers)
                if (overlap(r, h->rect)) return QStringLiteral("page %1: rows %2 to %3 of a table (%4) are over the header (%5)").arg(k).arg(t->firstRow).arg(t->endRow).arg(rectText(r), rectText(h->rect));
        }
        // The notes.
        const QStringList cited = citedNotes(pg, story, k);
        QStringList drawn;
        const QVector<const Item *> noteBoxes = itemsWith(*m, QStringLiteral("flow-note-box"));
        for (const Item *n : noteBoxes) {
            const QString text = storyOf(*m, n);
            QString which;
            for (auto it = marks.cbegin(); it != marks.cend(); ++it)
                if (text.contains(it.value())) which = it.key();
            if (which.isEmpty()) continue;   // (the rest of a note that began on a page before: the mark is not in it)
            drawn << which;
            if (n->rect.bottom() > area.bottom() + 0.05) return QStringLiteral("page %1: the note of %2 ends at %3, below the text area's foot (%4)").arg(k).arg(which, num(n->rect.bottom()), num(area.bottom()));
            if (n->rect.top() < area.top() - 0.05) return QStringLiteral("page %1: the note of %2 begins at %3, above the text area (%4)").arg(k).arg(which, num(n->rect.top()), num(area.top()));
            const int at = int(order.indexOf(which));
            if (at >= 0 && at < numbers.size() && numberShownIn(*m, n) != numbers[at])
                return QStringLiteral("page %1: the note of %2 shows the number %3, its reference is the %4th (it has %5)").arg(k).arg(which).arg(numberShownIn(*m, n)).arg(at + 1).arg(numbers[at]);
            for (const auto &t : m->tables)
                if (overlap(n->rect, t->rect())) return QStringLiteral("page %1: the note of %2 (%3) is over a table (%4)").arg(k).arg(which, rectText(n->rect), rectText(t->rect()));
        }
        if (!marks.isEmpty()) {
            for (const QString &id : cited)
                if (marks.contains(id) && !drawn.contains(id))
                    return QStringLiteral("page %1 cites the note %2 but does not draw it (it draws [%3]; its lines cite [%4])").arg(k).arg(id, drawn.join(QLatin1Char(' ')), cited.join(QLatin1Char(' ')));
            for (const QString &id : drawn)
                if (!cited.contains(id)) return QStringLiteral("page %1 draws the note %2, which its lines don't cite (they cite [%3])").arg(k).arg(id, cited.join(QLatin1Char(' ')));
            for (int i = 0; i < drawn.size(); ++i)
                if (drawn.indexOf(drawn[i]) != i) return QStringLiteral("page %1 draws the note %2 twice").arg(k).arg(drawn[i]);
        }
    }
    return QString();
}

// Whether the text of a made page is laid out, as the page's own text box will be, where the paginator's lines say it is. The page's story is laid out
// as the page layer does (one frame as wide as the box), and each line of the paginator that is not a table's row is found by its place in the story:
// its top and height are the same; the line that a table takes in the page's story (the stand-in for its rows) is as tall as the rows and where they
// begin. Only pages with one column of text and no section that begins below another's are checked (the others have a box to each column).
inline QString placementProblems(flow::Paginator &pg, const flow::FlowDocument &fd, int k, const LayoutEnv *env = nullptr)
{
    const std::unique_ptr<flow::MaterializedPage> m = pg.materialize(k);
    if (!m || m->boxes.size() != 1 || pg.page(k).columns != 1) return QString();
    const TextItem *box = nullptr;
    for (const auto &item : m->doc->pages[0]->items)
        if (item->id == m->boxes[0]) box = dynamic_cast<const TextItem *>(item.get());
    const Story *st = box ? m->doc->story(box->storyId) : nullptr;
    if (!st) return QStringLiteral("page %1 has no text box").arg(k);
    FrameSpec fs;
    fs.size = QSizeF(box->rect.width(), 100000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.columns = 1;
    fs.hyphenate = fd.hyphenate;
    fs.hyphenZone = fd.hyphenZone;
    StoryLayout lay;
    lay.build(st->doc.get(), {fs}, env ? *env : fd.env);
    QMap<int, StoryLayout::LineBrief> byPos;
    for (const StoryLayout::LineBrief &l : lay.lineBriefs()) byPos.insert(l.pos, l);
    const auto lines = pg.lines(k);
    auto isRow = [&](const StoryLayout::LineBrief &l) { return fd.story()->findBlock(l.pos).blockFormat().intProperty(flow::fp::Cell) != 0; };
    for (int i = 0; i < lines.size(); ++i) {
        const auto &l = lines[i];
        const auto at = byPos.find(l.pos - m->storyFrom);
        if (isRow(l)) {
            // A run of rows is one line in the page's text, as tall as they are together.
            int last = i;
            while (last + 1 < lines.size() && isRow(lines[last + 1]) && lines[last + 1].frame == l.frame) ++last;
            if (at == byPos.end()) return QStringLiteral("page %1: the table at %2 has no line in the page's text").arg(k).arg(l.pos);
            const double top = l.rect.top(), height = lines[last].rect.bottom() - top;
            if (std::abs(at->rect.top() - top) > 0.05)
                return QStringLiteral("page %1: the table's rows begin at %2, the stand-in for them in the page's text at %3").arg(k).arg(num(top), num(at->rect.top()));
            if (std::abs(at->rect.height() - height) > 0.05)
                return QStringLiteral("page %1: the table's rows are %2 high, the stand-in for them in the page's text %3").arg(k).arg(num(height), num(at->rect.height()));
            i = last;
            continue;
        }
        if (at == byPos.end()) return QStringLiteral("page %1: the line at %2 is not in the page's text").arg(k).arg(l.pos);
        if (std::abs(at->rect.top() - l.rect.top()) > 0.05 || std::abs(at->rect.height() - l.rect.height()) > 0.05)
            return QStringLiteral("page %1: the line at %2 is at %3 and %4 high, in the page's text at %5 and %6 high").arg(k).arg(l.pos).arg(num(l.rect.top()), num(l.rect.height()), num(at->rect.top()), num(at->rect.height()));
    }
    return QString();
}

// For rows of tables that go over page breaks: every paragraph of a cell that a part of the row shows (the lines from the part's `offsets` to its `ends`) is on the page that
// shows that part, as Paginator::pageOfTablePosition says; empty if every one is.
inline QString splitRowProblems(flow::Paginator &pg, const QTextDocument *story)
{
    int checked = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const std::unique_ptr<flow::MaterializedPage> m = pg.materialize(k);
        if (!m) continue;
        for (const auto &t : m->tables)
            for (const flow::TableFragment::Part &part : t->parts) {
                const flow::TableLayout &layout = *t->layout;
                const flow::TableLayout::Row &row = layout.row(part.row);
                for (int at = 0; at < part.piece.offsets.size() && at < part.piece.ends.size() && at < row.cells; ++at) {
                    const flow::TableLayout::Cell &cell = layout.cell(row.cell0 + at);
                    for (QTextBlock b = story->findBlock(cell.from); b.isValid() && b.position() < cell.to; b = b.next()) {
                        const double y = layout.lineTopIn(row.cell0 + at, b.position());
                        if (y < 0 || y + 0.01 < part.piece.offsets[at] || y + 0.01 >= part.piece.ends[at]) continue;
                        ++checked;
                        const int found = pg.pageOfTablePosition(b.position());
                        if (found != k)
                            return QStringLiteral("page %1 shows the paragraph at %2 (%3 down its cell, in a part that shows %4 to %5), the page of that position is said to be %6")
                                .arg(k).arg(b.position()).arg(num(y), num(part.piece.offsets[at])).arg(num(part.piece.ends[at])).arg(found);
                    }
                }
            }
    }
    return checked > 0 ? QString() : QStringLiteral("no row goes over a page break in the document");
}

} // namespace jp::flowaudit
