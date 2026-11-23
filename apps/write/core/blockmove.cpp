#include "core/blockmove.h"

#include "core/contentcontrolops.h"
#include "core/editops.h"
#include "core/tableops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <algorithm>

namespace jp::write {

namespace {

qint64 g_work = 0, g_scan = 0;

bool sameLook(QTextBlockFormat x, QTextBlockFormat y)
{
    x.setObjectIndex(-1);
    y.setObjectIndex(-1);
    return x == y;
}

QByteArray idOf(const QByteArray &raw)
{
    const qsizetype at = raw.indexOf("w:id=\"");
    if (at < 0) return {};
    const qsizetype from = at + 6, to = raw.indexOf('"', from);
    return to < 0 ? QByteArray() : raw.mid(from, to - from);
}

bool breaksSection(const QTextBlock &b) { return b.blockFormat().hasProperty(flow::fp::SectionBreakAfter); }

// Whether the paragraphs from `a` to `z` hold a start or an end (of a bookmark, a comment's range, or a field) whose partner is not among them.
bool holdsHalfAPair(const QTextBlock &a, const QTextBlock &z)
{
    QSet<QByteArray> bookmarks, comments, bookmarkEnds, commentEnds;
    int fieldDepth = 0;
    bool broken = false;
    for (QTextBlock b = a; b.isValid() && !broken; b = b.next()) {
        ++g_work;
        if (!isLocked(b))
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                const QTextCharFormat cf = f.charFormat();
                if (!cf.hasProperty(prop::Atom)) continue;
                const QByteArray bytes = storedBytes(cf, prop::Atom);
                const bool mark = bytes.contains("bookmark") || bytes.contains("commentRange") || bytes.contains("fldChar");
                if (!mark) continue;
                const office::wml::Inline atom = unpackAtom(bytes);
                const QString tag = atom.tag;
                for (int k = 0; k < f.length(); ++k) {
                    if (tag == QLatin1String("bookmarkStart")) bookmarks.insert(idOf(atom.raw));
                    else if (tag == QLatin1String("bookmarkEnd")) bookmarkEnds.insert(idOf(atom.raw));
                    else if (tag == QLatin1String("commentRangeStart")) comments.insert(idOf(atom.raw));
                    else if (tag == QLatin1String("commentRangeEnd")) commentEnds.insert(idOf(atom.raw));
                    else if (tag == QLatin1String("fldChar")) {
                        if (atom.key == QLatin1String("begin")) ++fieldDepth;
                        else if (atom.key == QLatin1String("end") && --fieldDepth < 0) broken = true;
                    }
                }
            }
        if (b == z) break;
    }
    return broken || fieldDepth != 0 || bookmarks != bookmarkEnds || comments != commentEnds;
}

// The elements kept as XML before or after a paragraph (packBlocks).
QVector<office::wml::Block> rawOf(const QTextBlock &b, int key)
{
    return b.isValid() && b.blockFormat().hasProperty(key) ? unpackBlocks(storedBytes(b.blockFormat(), key)) : QVector<office::wml::Block>();
}

QVector<office::wml::Block> rawOfFormat(const QTextBlockFormat &f, int key)
{
    return f.hasProperty(key) ? unpackBlocks(storedBytes(f, key)) : QVector<office::wml::Block>();
}

// What stands kept before a paragraph, in two parts: the ends of wrappers (a block content control, a custom-XML block, a tracked insertion) that hold the paragraphs before it, which belong
// with those paragraphs, and the rest, which belongs with this one.
void splitLead(const QVector<office::wml::Block> &raw, QVector<office::wml::Block> *lead, QVector<office::wml::Block> *rest)
{
    int depth = 0, cut = 0;
    for (int i = 0; i < raw.size(); ++i) {
        if (raw[i].role == office::wml::Inline::Open) ++depth;
        else if (raw[i].role == office::wml::Inline::Close && depth > 0) --depth;
        else if (raw[i].role == office::wml::Inline::Close) cut = i + 1;
    }
    *lead = raw.mid(0, cut);
    *rest = raw.mid(cut);
}

// Whether every wrapper kept as XML between the paragraphs `a` to `z` is whole in them: none opens before the run and ends in it, and none opens in it and ends after it. `tail` is what
// stands before the paragraph after the run that ends wrappers of the run.
bool wrappersWhole(const QTextBlock &a, const QTextBlock &z, const QVector<office::wml::Block> &tail)
{
    int depth = 0;
    bool whole = true;
    auto walk = [&](const QVector<office::wml::Block> &raw) {
        for (const office::wml::Block &r : raw) {
            if (r.role == office::wml::Inline::Open) ++depth;
            else if (r.role == office::wml::Inline::Close && --depth < 0) whole = false;
        }
    };
    for (QTextBlock b = a; b.isValid(); b = b.next()) {
        ++g_scan;
        QVector<office::wml::Block> before = rawOf(b, prop::RawBefore), lead;
        if (b == a) splitLead(QVector<office::wml::Block>(before), &lead, &before);
        walk(before);
        walk(rawOf(b, prop::RawAfter));
        if (b == z) break;
    }
    walk(tail);
    return whole && depth == 0;
}

struct Look {
    int number = 0;
    int was = 0;
    QTextBlockFormat format;
    QTextCharFormat mark;
};

} // namespace

qint64 moveBlocksWorkForTest() { return g_work; }
qint64 moveBlocksWrapperScanForTest() { return g_scan; }

bool moveBlocks(WriteDoc &doc, int first, int last, int before, QString *why, int *movedTo)
{
    if (why) why->clear();
    g_work = 0;
    g_scan = 0;
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    QTextDocument *s = doc.story();
    const int end = editableEnd(s);
    if (first < 0 || last < first || last > end) return false;
    const QTextBlock a = s->findBlock(first), z = s->findBlock(last);
    if (!a.isValid() || !z.isValid() || a.position() > z.position()) return false;
    const QTextBlock lastBody = s->findBlock(end);
    const QTextBlock after = z.position() >= lastBody.position() ? QTextBlock() : z.next();   // (none: the run ends the text)
    const bool toEnd = before < 0 || before > end;
    const QTextBlock t = toEnd ? QTextBlock() : s->findBlock(before);
    if (!toEnd && t.position() >= a.position() && after.isValid() && t.position() <= after.position()) return false;   // (where it is)
    if (!toEnd && !after.isValid() && t.position() >= a.position()) return false;
    if (toEnd && !after.isValid()) return false;
    if (doc.trackChanges()) return refuse(QCoreApplication::translate("jp::write::BlockMove", "Turn off Track Changes to move a section."));
    if (doc.protection() && doc.protection()->enforced) return refuse(QCoreApplication::translate("jp::write::BlockMove", "This document is protected, so a section can't be moved."));
    if (doc.hasSectionEndEndnotes()) return refuse(QCoreApplication::translate("jp::write::BlockMove", "A section can't be moved while endnotes are set at the ends of sections."));
    // The blocks the move is about: the run `r0` to `r1`, the one before it, the one after it, and the one it goes before with the one before that.
    const int tNumber = toEnd ? lastBody.blockNumber() + 1 : t.blockNumber();
    const bool up = tNumber < a.blockNumber();
    for (QTextBlock b = a;; b = b.next()) {
        ++g_work;
        if (b.blockFormat().hasProperty(prop::EndnoteBlock) || b.blockFormat().hasProperty(flow::fp::Frame)) return refuse(QCoreApplication::translate("jp::write::BlockMove", "A section holding the notes or a frame of the text can't be moved."));
        if (b == z) break;
    }
    // A table: not cut through at either end of the run or at the place, not put against another, and not left last (a paragraph follows it).
    const QTextBlock beforeRun = a.previous(), beforePlace = toEnd ? lastBody : t.previous(), atPlace = toEnd ? QTextBlock() : t;
    auto cellOf = [](const QTextBlock &b) { return b.isValid() && inTable(b); };
    if ((cellOf(beforeRun) && cellOf(a)) || (cellOf(z) && cellOf(after)) || (cellOf(beforePlace) && cellOf(atPlace))) return refuse(QCoreApplication::translate("jp::write::BlockMove", "A section can't be moved from or to the middle of a table."));
    if ((cellOf(beforeRun) && cellOf(after)) || (cellOf(beforePlace) && cellOf(a)) || (cellOf(z) && cellOf(atPlace))) return refuse(QCoreApplication::translate("jp::write::BlockMove", "A table can't be put right against another table. Keep a paragraph between them."));
    if (toEnd && cellOf(z)) return refuse(QCoreApplication::translate("jp::write::BlockMove", "A table can't be moved below the last paragraph of the document."));
    // A run with a section break moves only as whole sections, between sections: the page setup is the break's and describes the text before it.
    bool breaks = false;
    for (QTextBlock b = a;; b = b.next()) {
        breaks = breaks || breaksSection(b);
        if (b == z) break;
    }
    if (breaks) {
        const bool startsSection = !beforeRun.isValid() || breaksSection(beforeRun);
        const bool endsSection = breaksSection(z);
        const bool placeBetween = toEnd ? false : (!beforePlace.isValid() || breaksSection(beforePlace));
        if (!startsSection || !endsSection || !placeBetween) return refuse(QCoreApplication::translate("jp::write::BlockMove", "This section holds a page setup break that would change the layout of other text, so it can't be moved."));
    }
    // Two runs side by side swap places: P, then Q. The one that is moved is the person's (R), or its neighbor when R is the first paragraph of the text, which has no paragraph break before
    // it to be cut with.
    const int tPrev = toEnd ? lastBody.blockNumber() : tNumber - 1;
    const int p0 = up ? tNumber : a.blockNumber(), p1 = up ? a.blockNumber() - 1 : z.blockNumber();
    const int q0 = p1 + 1, q1 = up ? z.blockNumber() : tPrev;
    const int pLen = p1 - p0 + 1, qLen = q1 - q0 + 1;
    // The run may not hold one end of a bookmark, comment, field, or content control whose other end is outside it, and neither of the two runs that swap places may hold one end of a wrapper kept
    // as XML (a block content control, a custom-XML block, a tracked insertion) whose other end is outside it.
    const QTextBlock pa = s->findBlockByNumber(p0), pz = s->findBlockByNumber(p1), qa = s->findBlockByNumber(q0), qz = s->findBlockByNumber(q1);
    const QTextBlock post = s->findBlockByNumber(q1 + 1);
    const bool hasPost = post.isValid() && post.position() <= end;
    QVector<office::wml::Block> leadP0, headP0, leadQ0, headQ0, leadPost, headPost;
    splitLead(rawOf(pa, prop::RawBefore), &leadP0, &headP0);
    splitLead(rawOf(qa, prop::RawBefore), &leadQ0, &headQ0);
    if (hasPost) splitLead(rawOf(post, prop::RawBefore), &leadPost, &headPost);
    if (holdsHalfAPair(a, z) || splitsContentControl(s, a.position(), z.position() + z.length() - 1) || !wrappersWhole(pa, pz, leadQ0) || !wrappersWhole(qa, qz, leadPost))
        return refuse(QCoreApplication::translate("jp::write::BlockMove", "This section holds one end of a bookmark, comment, field, or content control whose other end is outside it, so it can't be moved."));
    const bool moveQ = up || p0 == 0;
    const int m0 = moveQ ? q0 : p0, m1 = moveQ ? q1 : p1;
    auto newNumber = [&](int x) { return x >= p0 && x <= p1 ? x + qLen : x >= q0 && x <= q1 ? x - pLen : x; };
    const int landsAt = newNumber(a.blockNumber());
    // What each paragraph looks like now: the moved run's, and those at the four seams, to put back as they were wherever the cut and the paste have changed them.
    QVector<Look> looks;
    auto keep = [&](int number) {
        const QTextBlock b = s->findBlockByNumber(number);
        if (number < 0 || !b.isValid() || b.position() > end) return;
        Look l;
        l.number = newNumber(number);
        l.was = number;
        l.format = b.blockFormat();
        l.mark = b.charFormat();
        looks << l;
    };
    for (int x = m0; x <= m1; ++x) keep(x);
    for (int x : {p0 - 1, p0, p1, q0, q1, q1 + 1}) keep(x);
    // Where a wrapper ends right after a run, the end is kept before the paragraph that follows: each paragraph that now follows a run is given the ends of the one it follows now.
    auto setRaw = [](QTextBlockFormat *f, int key, const QVector<office::wml::Block> &raw) {
        if (raw.isEmpty()) f->clearProperty(key);
        else setStored(*f, key, packBlocks(raw));
    };
    if (!leadP0.isEmpty() || !leadQ0.isEmpty() || !leadPost.isEmpty())
        for (Look &l : looks) {
            if (l.was == q0) setRaw(&l.format, prop::RawBefore, leadP0 + headQ0);
            if (l.was == p0) setRaw(&l.format, prop::RawBefore, leadPost + headP0);
            if (hasPost && l.was == q1 + 1) setRaw(&l.format, prop::RawBefore, leadQ0 + headPost);
            if (!hasPost && l.was == p1) setRaw(&l.format, prop::RawAfter, rawOfFormat(l.format, prop::RawAfter) + leadQ0);
        }
    const QByteArray lastSection = storedBytes(s->begin().blockFormat(), prop::LastSection);
    g_work += looks.size();

    const QTextBlock mFirst = s->findBlockByNumber(m0), mLast = s->findBlockByNumber(m1);
    const int cutFrom = mFirst.position() - 1, cutTo = mLast.position() + mLast.length() - 1;
    const bool atTheEnd = !moveQ && q1 >= lastBody.blockNumber();
    const int putAt = moveQ ? s->findBlockByNumber(p0).position() : atTheEnd ? end : s->findBlockByNumber(q1 + 1).position();
    const int blocksBefore = s->blockCount();
    const WriteDoc::StepGroup group(doc);
    QTextCursor edit(s);
    edit.beginEditBlock();
    QTextCursor whole(s);
    whole.setPosition(mFirst.position());
    whole.setPosition(cutTo, QTextCursor::KeepAnchor);
    const QTextDocumentFragment fragment = whole.selection();
    QTextCursor put(s);   // (a cursor follows the cut)
    put.setPosition(putAt);
    QTextCursor cut(s);
    cut.setPosition(cutFrom);
    cut.setPosition(cutTo, QTextCursor::KeepAnchor);
    cut.removeSelectedText();
    if (atTheEnd) {
        put.insertBlock();
        put.insertFragment(fragment);
    } else {
        put.insertFragment(fragment);
        put.insertBlock();
    }
    // (Where the paragraph the run was put before was the first of a list, the run went in as paragraphs of its own, and left an empty one before it: that one is taken out.)
    if (s->blockCount() > blocksBefore) {
        const int landing = newNumber(m0);
        QTextBlock stray = s->findBlockByNumber(landing);
        if (!(stray.isValid() && stray.length() == 1)) stray = s->findBlockByNumber(landing - 1);
        if (stray.isValid() && stray.length() == 1 && s->blockCount() == blocksBefore + 1) {
            QTextCursor gone(stray);
            if (stray.position() > 0) {
                gone.setPosition(stray.position() - 1);
                gone.setPosition(stray.position(), QTextCursor::KeepAnchor);
            } else {
                gone.setPosition(0);
                gone.setPosition(1, QTextCursor::KeepAnchor);
            }
            gone.removeSelectedText();
        }
    }
    // Each paragraph that was named has its own look back; the first paragraph of the text keeps the last section's properties, which belong to it and not to the paragraph that was first.
    QHash<int, Look> want;
    for (const Look &l : std::as_const(looks)) want.insert(l.number, l);
    for (auto it = want.begin(); it != want.end(); ++it) {
        QTextBlockFormat f = it->format;
        if (it.key() == 0) {
            if (!lastSection.isEmpty()) setStored(f, prop::LastSection, lastSection);
        } else {
            f.clearProperty(prop::LastSection);
        }
        const QTextBlock b = s->findBlockByNumber(it.key());
        if (!b.isValid()) continue;
        f.setObjectIndex(it->format.objectIndex() < 0 ? -1 : b.blockFormat().objectIndex());   // (in a list: the one it is in now, as the one it was in may be gone; in none: in none, whatever the break before it made of it)
        QTextCursor c(b);
        if (!sameLook(b.blockFormat(), f)) c.setBlockFormat(f);
        if (b.charFormat() != it->mark) c.setBlockCharFormat(it->mark);
    }
    edit.endEditBlock();
    doc.settleEndnotes();
    doc.renumberLists(s);
    doc.keepObjectsForTheHistory();
    if (movedTo) *movedTo = s->findBlockByNumber(landsAt).position();
    return true;
}

} // namespace jp::write
