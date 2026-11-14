#include "core/outlineops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace jp::write {

namespace {

int levelOf(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    if (!bf.hasProperty(prop::Outline)) return kBodyLevel;
    return qBound(0, bf.intProperty(prop::Outline), kBodyLevel);
}

int rowAt(const QVector<OutlineRow> &rows, int position)
{
    int found = -1;
    for (int i = 0; i < rows.size() && rows[i].position <= position; ++i) found = i;
    return found;
}

} // namespace

QVector<OutlineRow> outlineRows(WriteDoc &doc)
{
    QVector<OutlineRow> rows;
    for (QTextBlock b = doc.story()->begin(); b.isValid() && rows.size() < kMostOutlineRows; b = b.next()) {
        OutlineRow r;
        r.position = b.position();
        r.level = levelOf(b);
        r.text = b.text();
        r.editable = !r.text.contains(QChar(0xFFFC)) && !b.blockFormat().hasProperty(prop::RawBlock);
        r.text.replace(QChar::LineSeparator, QLatin1Char(' ')).remove(QChar(0xFFFC));
        rows << r;
    }
    return rows;
}

int outlineSubtreeEnd(const QVector<OutlineRow> &rows, int i)
{
    if (i < 0 || i >= rows.size() || !rows[i].heading()) return qMin(i + 1, int(rows.size()));
    int j = i + 1;
    while (j < rows.size() && rows[j].level > rows[i].level) ++j;
    return j;
}

bool outlineSetText(WriteDoc &doc, int position, const QString &textIn)
{
    const QVector<OutlineRow> rows = outlineRows(doc);
    const int i = rowAt(rows, position);
    if (i < 0 || !rows[i].editable) return false;
    QString text = textIn;
    text.replace(QRegularExpression(QStringLiteral("[\\r\\n\\x{2028}\\x{2029}]+")), QStringLiteral(" "));
    QTextBlock b = doc.story()->findBlock(rows[i].position);
    QTextCursor c(doc.story());
    c.setPosition(b.position());
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    const WriteDoc::StepGroup group(doc);
    if (text.isEmpty()) {
        if (c.hasSelection()) removeSelection(doc, c);
        return true;
    }
    insertText(doc, c, text);
    return true;
}

bool outlineInsertAfter(WriteDoc &doc, int position, int *newAt)
{
    const QVector<OutlineRow> rows = outlineRows(doc);
    const int i = rowAt(rows, position);
    if (i < 0 || !rows[i].editable) return false;
    const WriteDoc::StepGroup group(doc);
    QTextCursor c(doc.story());
    c.setPosition(rows[i].position);
    c.movePosition(QTextCursor::EndOfBlock);
    insertParagraph(doc, c);
    const int at = c.block().position();
    if (levelOf(c.block()) != rows[i].level) outlineSetLevel(doc, at, at, rows[i].level);
    if (newAt) *newAt = at;
    return true;
}

bool outlineSetLevel(WriteDoc &doc, int first, int last, int level)
{
    const QVector<OutlineRow> rows = outlineRows(doc);
    const int a = rowAt(rows, qMin(first, last)), b = rowAt(rows, qMax(first, last));
    if (a < 0 || b < a) return false;
    level = qBound(0, level, kBodyLevel);
    const WriteDoc::StepGroup group(doc);
    bool any = false;
    for (int i = a; i <= b; ++i) {
        if (rows[i].level == level) continue;
        QTextCursor c(doc.story());
        c.setPosition(rows[i].position);
        applyParagraphStyle(doc, c, level == kBodyLevel ? doc.defaultParagraphStyle() : QStringLiteral("Heading%1").arg(level + 1));
        any = true;
    }
    return any;
}

bool outlineShift(WriteDoc &doc, int first, int last, int by, bool withSubtree)
{
    if (by == 0) return false;
    const QVector<OutlineRow> rows = outlineRows(doc);
    const int a = rowAt(rows, qMin(first, last)), b = rowAt(rows, qMax(first, last));
    if (a < 0 || b < a) return false;
    // The new level of each paragraph that changes.
    QVector<int> now(rows.size(), -1);
    int end = b + 1;
    if (withSubtree)
        for (int i = a; i <= b; ++i) end = qMax(end, outlineSubtreeEnd(rows, i));
    for (int i = a; i < end; ++i) {
        int level = rows[i].level;
        if (rows[i].heading()) {
            level = qBound(0, rows[i].level + by, 9);
            if (level > 8) level = kBodyLevel;
        } else if (by < 0 && i <= b) {
            // (Body text under a heading that moves stays body text.)
            int above = 0;
            for (int k = i - 1; k >= 0; --k)
                if (rows[k].heading()) {
                    above = rows[k].level;
                    break;
                }
            level = above;
        }
        if (level != rows[i].level) now[i] = level;
    }
    const WriteDoc::StepGroup group(doc);
    bool any = false;
    for (int i = 0; i < rows.size(); ++i) {
        if (now[i] < 0) continue;
        QTextCursor c(doc.story());
        c.setPosition(rows[i].position);
        applyParagraphStyle(doc, c, now[i] == kBodyLevel ? doc.defaultParagraphStyle() : QStringLiteral("Heading%1").arg(now[i] + 1));
        any = true;
    }
    return any;
}

namespace {

// Moves rows [a, b) to before row t (t <= a or t >= b). The paragraphs are cut and put back with the paragraph break before them, so that each keeps its own look (a paragraph break
// carries the look of the paragraph after it); the first paragraph of the text has no break before it, so a move to the top goes by way of a blank paragraph there, which is taken away.
bool moveRows(WriteDoc &doc, const QVector<OutlineRow> &rows, int a, int b, int t, int *movedTo)
{
    const int n = rows.size();
    QTextDocument *story = doc.story();
    if (a == 0) {
        if (t < b) return false;
        return moveRows(doc, rows, b, t, 0, movedTo);
    }
    const WriteDoc::StepGroup group(doc);
    // The look of each paragraph, to give back to the ones that took another's at the ends of the cut and of the place it goes to.
    QVector<QTextBlockFormat> looks;
    for (const OutlineRow &r : rows) looks << story->findBlock(r.position).blockFormat();
    int shift = 0;
    if (t == 0) {
        QTextCursor top(story);
        top.setPosition(0);
        insertParagraph(doc, top);
        shift = 1;
    }
    const int from = rows[a].position - 1 + shift;
    const int end = b >= n ? story->characterCount() - 1 : rows[b].position - 1 + shift;
    const int to = t >= n ? story->characterCount() - 1 : rows[t].position - 1 + shift;
    QTextCursor sel(story);
    sel.setPosition(from);
    sel.setPosition(end, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(doc, sel);
    if (clip.isEmpty()) return false;
    QTextCursor drop(story);   // (the place follows the cut by being a cursor)
    drop.setPosition(to);
    removeSelection(doc, sel);
    QTextCursor dest(story);
    dest.setPosition(drop.position());
    const int at = dest.position();
    pasteClip(doc, dest, clip);
    int first = at + 1;
    if (shift) {
        QTextCursor blank(story);
        blank.setPosition(0);
        blank.setPosition(1, QTextCursor::KeepAnchor);
        removeSelection(doc, blank);
        first -= 1;
    }
    // The new order of the old paragraphs.
    QVector<int> order;
    const int low = qMin(a, t), high = qMax(b, t);
    for (int k = 0; k < n; ++k) order << k;
    if (t <= a) {
        for (int k = 0; k < b - a; ++k) order[t + k] = a + k;
        for (int k = 0; k < a - t; ++k) order[t + (b - a) + k] = t + k;
    } else {
        for (int k = 0; k < t - b; ++k) order[a + k] = b + k;
        for (int k = 0; k < b - a; ++k) order[a + (t - b) + k] = a + k;
    }
    auto sameLook = [](QTextBlockFormat x, QTextBlockFormat y) {
        x.setObjectIndex(-1);
        y.setObjectIndex(-1);
        return x == y;
    };
    QTextBlock block = story->findBlock(rows[low].position);
    for (int k = low; k < high && block.isValid(); ++k, block = block.next()) {
        if (sameLook(block.blockFormat(), looks[order[k]])) continue;
        QTextCursor at(story);
        at.setPosition(block.position());
        const QTextBlockFormat want = looks[order[k]];
        changeBlockFormat(doc, at, [&](QTextBlockFormat &f) { f = want; });
        block = story->findBlock(at.position());
    }
    if (movedTo) *movedTo = first;
    return true;
}

} // namespace

bool outlineMove(WriteDoc &doc, int position, bool up, bool withSubtree, int *movedTo)
{
    const QVector<OutlineRow> rows = outlineRows(doc);
    const int i = rowAt(rows, position);
    if (i < 0) return false;
    const int j = withSubtree ? outlineSubtreeEnd(rows, i) : i + 1;   // the unit is [i, j)
    if (up) {
        if (i == 0) return false;
        int p = i - 1;
        if (withSubtree && rows[i].heading()) {
            // Before the section above that is of the same level; a heading that is the first of its parent's can't pass it.
            while (p >= 0 && (!rows[p].heading() || rows[p].level > rows[i].level)) --p;
            if (p < 0 || rows[p].level < rows[i].level) return false;
        }
        return moveRows(doc, rows, i, j, p, movedTo);
    }
    if (j >= rows.size()) return false;
    int t;
    if (withSubtree && rows[i].heading()) {
        if (rows[j].heading() && rows[j].level < rows[i].level) return false;
        t = outlineSubtreeEnd(rows, j);
    } else {
        t = j + 1;
    }
    // (Down is the paragraphs below going up, over this unit.)
    if (!moveRows(doc, rows, j, t, i, nullptr)) return false;
    if (movedTo) *movedTo = outlineRows(doc).value(i + (t - j)).position;
    return true;
}

// ---- collapsed headings

namespace {
constexpr int kCollapsedHeading = 0x4A48;   // a heading block's user state while it is collapsed (tp::kCollapsedBlock marks the paragraphs it hides)

// Sets the paragraphs the collapsed headings hide and clears the others'; true if any paragraph's state changed.
bool settleCollapse(WriteDoc &doc, bool *any)
{
    bool changed = false, some = false;
    int hideBelow = -1;   // the level at or above which a heading ends the run being hidden
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        const int level = levelOf(b);
        int want = b.userState() == kCollapsedHeading ? kCollapsedHeading : -1;
        if (want == kCollapsedHeading && level >= kBodyLevel) want = -1;   // (no heading now)
        if (hideBelow >= 0 && level > hideBelow) {
            want = tp::kCollapsedBlock;
        } else {
            hideBelow = -1;
            if (want == kCollapsedHeading) {
                hideBelow = level;
                some = true;
            }
        }
        if (b.userState() != want) {
            b.setUserState(want);
            changed = true;
        }
    }
    if (any) *any = some;
    return changed;
}
} // namespace

int headingLevelAt(WriteDoc &doc, int position)
{
    const QTextBlock b = doc.story()->findBlock(position);
    if (!b.isValid()) return -1;
    const int level = levelOf(b);
    return level < kBodyLevel ? level : -1;
}

bool headingCollapsed(WriteDoc &doc, int position) { return doc.story()->findBlock(position).userState() == kCollapsedHeading; }

bool hiddenByCollapse(WriteDoc &doc, int position) { return doc.story()->findBlock(position).userState() == tp::kCollapsedBlock; }

bool setHeadingCollapsed(WriteDoc &doc, int position, bool collapsed)
{
    QTextBlock b = doc.story()->findBlock(position);
    if (!b.isValid() || levelOf(b) >= kBodyLevel) return false;
    if ((b.userState() == kCollapsedHeading) == collapsed) return false;
    if (collapsed) {
        const QTextBlock next = b.next();
        if (!next.isValid() || levelOf(next) <= levelOf(b)) return false;   // (nothing under it)
    }
    b.setUserState(collapsed ? kCollapsedHeading : -1);
    bool any = false;
    settleCollapse(doc, &any);
    doc.setCollapseActive(any);
    return true;
}

bool expandAround(WriteDoc &doc, int position)
{
    QTextBlock b = doc.story()->findBlock(position);
    if (!b.isValid() || b.userState() != tp::kCollapsedBlock) return false;
    // The headings that hide it: going back, each heading above the levels met so far that is collapsed.
    int level = kBodyLevel;
    bool opened = false;
    for (b = b.previous(); b.isValid() && level > 0; b = b.previous()) {
        const int l = levelOf(b);
        if (l >= level) continue;
        level = l;
        if (b.userState() == kCollapsedHeading) {
            b.setUserState(-1);
            opened = true;
        }
    }
    if (opened) {
        bool any = false;
        settleCollapse(doc, &any);
        doc.setCollapseActive(any);
    }
    return opened;
}

void refreshCollapsed(WriteDoc &doc)
{
    if (!doc.collapseActive()) return;
    bool any = false;
    if (settleCollapse(doc, &any) || !any) doc.setCollapseActive(any);
}

} // namespace jp::write
