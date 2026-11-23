#include "core/sectionmove.h"

#include "core/blockmove.h"
#include "core/editops.h"
#include "core/tableops.h"
#include "core/writeprops.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextDocument>

namespace jp::write {

namespace {

// The outline level (0 to 8) of a paragraph, 9 for body text.
int levelOf(const QTextBlock &b)
{
    const QTextBlockFormat f = b.blockFormat();
    return f.hasProperty(prop::Outline) ? qBound(0, f.intProperty(prop::Outline), 9) : 9;
}

// Whether the paragraph can be the heading of a section.
bool isHeading(const QTextBlock &b)
{
    const QTextBlockFormat f = b.blockFormat();
    return levelOf(b) < 9 && !inTable(b) && !f.hasProperty(prop::EndnoteBlock) && !f.hasProperty(flow::fp::Frame);
}

} // namespace

int headingAtOrAbove(WriteDoc &doc, int position)
{
    QTextDocument *s = doc.story();
    if (position < 0 || position > editableEnd(s)) return -1;
    for (QTextBlock b = s->findBlock(position); b.isValid(); b = b.previous())
        if (isHeading(b)) return b.position();
    return -1;
}

bool sectionAt(WriteDoc &doc, int position, SectionRange *out)
{
    QTextDocument *s = doc.story();
    const int end = editableEnd(s);
    if (position < 0 || position > end) return false;
    const QTextBlock head = s->findBlock(position);
    const int level = levelOf(head);
    if (!isHeading(head)) return false;
    QTextBlock last = head;
    for (QTextBlock b = head.next(); b.isValid() && b.position() <= end && levelOf(b) > level; b = b.next()) last = b;
    if (out) {
        out->first = head.position();
        out->last = last.position();
        out->level = level;
        out->end = last.position() + last.length();
    }
    return true;
}

bool moveSection(WriteDoc &doc, int position, int before, QString *why, int *movedTo)
{
    if (why) why->clear();
    SectionRange range;
    if (!sectionAt(doc, position, &range)) {
        if (why) *why = QCoreApplication::translate("jp::write::SectionMove", "Only a heading, with what is under it, can be moved.");
        return false;
    }
    return moveBlocks(doc, range.first, range.last, before, why, movedTo);
}

bool moveSectionBy(WriteDoc &doc, int position, bool up, QString *why, int *movedTo)
{
    if (why) why->clear();
    SectionRange range;
    if (!sectionAt(doc, position, &range)) {
        if (why) *why = QCoreApplication::translate("jp::write::SectionMove", "Only a heading, with what is under it, can be moved.");
        return false;
    }
    QTextDocument *s = doc.story();
    const int end = editableEnd(s);
    int before = -1;
    if (up) {
        // The section above of the same level: the nearest heading above of that level, with no heading of a higher level between.
        QTextBlock b = s->findBlock(range.first).previous();
        for (; b.isValid(); b = b.previous()) {
            const int l = levelOf(b);
            if (l < range.level) break;
            if (l == range.level) {
                before = b.position();
                break;
            }
        }
        if (before < 0) {
            if (why) *why = QCoreApplication::translate("jp::write::SectionMove", "This section can't move up: there is no section of the same level above it.");
            return false;
        }
    } else {
        const QTextBlock next = s->findBlock(range.last).next();
        if (!next.isValid() || next.position() > end || levelOf(next) != range.level) {
            if (why) *why = QCoreApplication::translate("jp::write::SectionMove", "This section can't move down: there is no section of the same level below it.");
            return false;
        }
        SectionRange other;
        if (!sectionAt(doc, next.position(), &other)) return false;
        const QTextBlock beyond = s->findBlock(other.last).next();
        before = beyond.isValid() && beyond.position() <= end ? beyond.position() : -1;
        if (before < 0 && !beyond.isValid()) before = -1;
    }
    return moveBlocks(doc, range.first, range.last, before, why, movedTo);
}

} // namespace jp::write
