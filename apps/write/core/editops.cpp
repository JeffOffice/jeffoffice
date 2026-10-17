#include "core/editops.h"

#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <cmath>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

// The properties that go with the end of a paragraph (what a section ending in it and the XML after it belong to: the paragraph mark's own), and
// with its beginning (the XML before it, the attributes it had in the file). When a paragraph is cut in two, the second half has the mark's, and
// the first keeps the beginning's; when two become one, the result has the first's beginning and the last's end.
const int kTail[] = {flow::fp::SectionBreakAfter, prop::SectionData, prop::RawAfter};
const int kHead[] = {prop::RawBefore, prop::ParaAttrs, prop::LastSection, prop::MarkFormat};

void copyTail(const QTextBlockFormat &from, QTextBlockFormat *to)
{
    for (int key : kTail) {
        if (from.hasProperty(key)) to->setProperty(key, from.property(key));
        else to->clearProperty(key);
    }
}

void copyHead(const QTextBlockFormat &from, QTextBlockFormat *to)
{
    for (int key : kHead) {
        if (from.hasProperty(key)) to->setProperty(key, from.property(key));
        else to->clearProperty(key);
    }
}

void clearTail(QTextBlockFormat *f)
{
    for (int key : kTail) f->clearProperty(key);
}

void clearHead(QTextBlockFormat *f)
{
    for (int key : kHead) f->clearProperty(key);
}

void setBlockFormatKeepingList(QTextBlock block, QTextBlockFormat nf)
{
    nf.setObjectIndex(block.blockFormat().objectIndex());
    QTextCursor(block).setBlockFormat(nf);
}

// Gives a paragraph a format of its own making (a style's, a copy's), with `list` its list (none: it is in no list).
void setBlockFormatInList(QTextBlock block, QTextBlockFormat nf, QTextList *list)
{
    nf.setObjectIndex(-1);
    if (QTextList *old = block.textList()) old->remove(block);
    QTextCursor(block).setBlockFormat(nf);
    if (list) list->add(block);
}

bool isZeroWidth(const QTextCharFormat &f) { return isAtom(f) && f.stringProperty(tp::Field) == QLatin1String("raw:"); }

// The format of the character at `position` (the one that comes after it).
QTextCharFormat charAt(QTextDocument *doc, int position)
{
    QTextCursor t(doc);
    t.setPosition(position + 1);
    return t.charFormat();
}

QTextCharFormat typingFormat(const QTextCursor &c)
{
    QTextCharFormat f = c.charFormat();
    // (Text typed beside what a tracked change deleted is not deleted, nor hidden.)
    if (f.hasProperty(prop::TrackedDeleted)) {
        if (f.intProperty(prop::TrackedDeleted) == prop::HiddenByDeletion) f.clearProperty(tp::HiddenText);
        f.clearProperty(prop::TrackedDeleted);
    }
    if (!isAtom(f)) return f;
    // Beside a piece kept as XML: the look of the text before it, else of the paragraph's mark.
    const QTextBlock b = c.block();
    for (int p = c.position() - 1; p >= b.position(); --p) {
        const QTextCharFormat t = charAt(c.document(), p);
        if (!isAtom(t)) return t;
    }
    return withoutAtom(markFormat(b));
}

// The place of the visible character before (after) `position` in its paragraph; -1 when the paragraph has none that way.
int visibleBefore(QTextDocument *d, int position)
{
    const int start = d->findBlock(position).position();
    for (int p = position - 1; p >= start; --p)
        if (!isZeroWidth(charAt(d, p))) return p;
    return -1;
}

int visibleAfter(QTextDocument *d, int position)
{
    const QTextBlock b = d->findBlock(position);
    const int end = b.position() + b.length() - 1;
    for (int p = position; p < end; ++p)
        if (!isZeroWidth(charAt(d, p))) return p;
    return -1;
}

// A tag that has a start and an end in a paragraph (a hyperlink, a content control, a tracked change...), open where the paragraph is cut:
// what the first half must close, and the second open again.
struct Wrapper {
    QTextCharFormat openFormat, closeFormat;
};

QVector<Wrapper> wrappersOpenAt(const QTextBlock &b, int position)
{
    struct Item { office::wml::Inline atom; QTextCharFormat format; int at; };
    QVector<Item> items;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        if (!cf.hasProperty(prop::Atom)) continue;
        const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
        if (a.kind != Inline::ParaAtom || (a.role != Inline::Open && a.role != Inline::Close)) continue;
        const QString t = f.text();
        for (int k = 0; k < t.size(); ++k)
            if (t[k] == QChar::ObjectReplacementCharacter) items.push_back({a, cf, f.position() + k});
    }
    QVector<int> stack;
    QVector<QPair<int, int>> pairs;
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].atom.role == Inline::Open) {
            stack.push_back(i);
        } else {
            // (An end closes the nearest start of its tag that is open: a stray start above it, from a copy, is not part of the pair.)
            int at = int(stack.size()) - 1;
            while (at >= 0 && items[stack[at]].atom.tag != items[i].atom.tag) --at;
            if (at >= 0) {
                pairs.push_back({stack[at], i});
                stack.resize(at);
            }
        }
    }
    QVector<Wrapper> out;
    std::sort(pairs.begin(), pairs.end());
    for (const auto &pr : std::as_const(pairs))
        if (items[pr.first].at < position && items[pr.second].at >= position) out.push_back({items[pr.first].format, items[pr.second].format});
    return out;
}

// Where the cursor is as the person sees it: before the first visible character and after the last, whatever pieces kept as XML
// that show nothing are beside it (a cut there is a cut at the start or the end of the paragraph).
void settleAtEdges(QTextCursor &c)
{
    QTextDocument *d = c.document();
    const QTextBlock b = c.block();
    if (visibleAfter(d, c.position()) < 0) c.setPosition(b.position() + b.length() - 1);
    else if (visibleBefore(d, c.position()) < 0) c.setPosition(b.position());
}

// Closes, at the end of `first`, the wrappers that were open where it was cut from `second`, and opens them again at the start of `second`.
void rewrap(const QVector<Wrapper> &wrappers, QTextBlock first, QTextBlock second)
{
    if (wrappers.isEmpty()) return;
    QTextCursor t(first);
    t.movePosition(QTextCursor::EndOfBlock);
    for (int i = int(wrappers.size()) - 1; i >= 0; --i) t.insertText(QString(QChar::ObjectReplacementCharacter), wrappers[i].closeFormat);
    QTextCursor u(second);
    u.setPosition(second.position());
    for (const Wrapper &w : wrappers) u.insertText(QString(QChar::ObjectReplacementCharacter), w.openFormat);
}

} // namespace

bool isLocked(const QTextBlock &block) { return block.isValid() && block.blockFormat().hasProperty(prop::RawBlock); }

bool isInvisibleAtom(const QTextCharFormat &format) { return isZeroWidth(format); }

QTextCharFormat markFormat(const QTextBlock &block)
{
    if (!block.previous().isValid() && block.blockFormat().hasProperty(prop::MarkFormat)) return unpackFormat(storedBytes(block.blockFormat(), prop::MarkFormat));
    return block.charFormat();
}

void setMarkFormat(QTextBlock block, const QTextCharFormat &format)
{
    QTextCursor(block).setBlockCharFormat(format);
    if (!block.previous().isValid()) {
        QTextBlockFormat bf = block.blockFormat();
        setStored(bf, prop::MarkFormat, packFormat(format));
        QTextCursor(block).setBlockFormat(bf);
    }
}

QVector<QTextBlock> selectedBlocks(const QTextCursor &c)
{
    QVector<QTextBlock> out;
    QTextDocument *d = c.document();
    QTextBlock first = d->findBlock(c.selectionStart()), last = d->findBlock(c.selectionEnd());
    if (c.hasSelection() && last != first && c.selectionEnd() == last.position()) last = last.previous();
    for (QTextBlock b = first; b.isValid(); b = b.next()) {
        out.push_back(b);
        if (b == last) break;
    }
    return out;
}

// ---- typing

void insertText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    Q_UNUSED(doc);
    if (text.isEmpty()) return;
    if (isLocked(c.block()) && !c.hasSelection()) return;
    QTextCharFormat f = typingFormat(c);
    if (c.hasSelection()) {
        c.beginEditBlock();
        removeSelection(doc, c);
        f = typingFormat(c);
        c.insertText(text, f);
        c.endEditBlock();
    } else {
        c.insertText(text, f);
    }
}

static void removeFromList(WriteDoc &doc, QTextBlock block)
{
    if (QTextList *l = block.textList()) l->remove(block);
    QTextBlockFormat nf = block.blockFormat();
    const QString style = nf.stringProperty(tp::StyleName);
    const QTextBlockFormat base = doc.styleBlockFormat(style);
    nf.clearProperty(prop::NumId);
    nf.clearProperty(prop::NumLevel);
    nf.clearProperty(tp::BulletChar);
    nf.setLeftMargin(base.leftMargin());
    nf.setTextIndent(base.textIndent());
    nf.setObjectIndex(-1);
    QTextCursor(block).setBlockFormat(nf);
}

// Cuts the paragraph at the cursor in two, as Enter does: the first half keeps what comes before the paragraph (the XML kept before it, its
// attributes in the file), the second has the paragraph mark's own (a section that ends here, the XML after it), and what is open where it is
// cut (a hyperlink, a content control, a tracked change) is closed in the first and opened again in the second. The cursor is at the start of
// the second. Returns the first half.
static QTextBlock splitParagraph(QTextCursor &c)
{
    settleAtEdges(c);
    const QVector<Wrapper> wrappers = wrappersOpenAt(c.block(), c.position());
    c.insertBlock();
    QTextBlock first = c.block().previous(), second = c.block();
    QTextBlockFormat a = first.blockFormat(), b = second.blockFormat();
    clearTail(&a);
    clearHead(&b);
    setBlockFormatKeepingList(first, a);
    setBlockFormatKeepingList(second, b);
    rewrap(wrappers, first, second);
    c.setPosition(second.position() + int(wrappers.size()));
    return first;
}

void insertParagraph(WriteDoc &doc, QTextCursor &c)
{
    c.beginEditBlock();
    if (c.hasSelection()) removeSelection(doc, c);
    QTextBlock cur = c.block();
    if (isLocked(cur)) {
        // After the element kept as XML: a paragraph of the default style.
        c.movePosition(QTextCursor::EndOfBlock);
        const QString style = doc.defaultParagraphStyle();
        QTextBlockFormat nf = doc.styleBlockFormat(style);
        if (cur.blockFormat().hasProperty(flow::fp::Cell)) nf.setProperty(flow::fp::Cell, cur.blockFormat().property(flow::fp::Cell));   // (in the cell the element is in)
        c.insertBlock(nf, doc.styleCharFormat(style));
        c.endEditBlock();
        return;
    }
    // Enter on an empty list item ends the list.
    if (cur.blockFormat().hasProperty(prop::NumId) && cur.length() <= 1) {
        removeFromList(doc, cur);
        c.endEditBlock();
        return;
    }
    settleAtEdges(c);
    const bool atEnd = c.atBlockEnd();
    const QString style = cur.blockFormat().stringProperty(tp::StyleName);
    splitParagraph(c);
    // At the end of a paragraph, the next one is in the style the style says follows it (a heading is followed by body text); it is a paragraph of
    // that style, with the paragraph mark's own (a section that ends here...) that the end of the paragraph had.
    if (atEnd) {
        const Style *s = doc.styleSheet().find(style);
        if (s && !s->next.isEmpty() && s->next != style && doc.ensureStyle(s->next)) {
            QTextBlock b = c.block();
            const QTextBlockFormat tailOwner = b.blockFormat();
            QTextBlockFormat nf = doc.styleBlockFormat(s->next);
            copyTail(tailOwner, &nf);
            if (tailOwner.hasProperty(flow::fp::Cell)) nf.setProperty(flow::fp::Cell, tailOwner.property(flow::fp::Cell));   // (the paragraph after it is in its cell)
            setBlockFormatInList(b, nf, nullptr);
            setMarkFormat(b, doc.styleCharFormat(s->next));
            c.setCharFormat(doc.styleCharFormat(s->next));
        }
    }
    c.endEditBlock();
}

void insertPageBreak(WriteDoc &doc, QTextCursor &c)
{
    if (isLocked(c.block())) return;
    c.beginEditBlock();
    if (c.hasSelection()) removeSelection(doc, c);
    // The break is a character of the paragraph (the page ends after the line that has it); what follows it goes on in a paragraph of its own,
    // as the program a person may be used to does it, so that the cursor is at the top of the next page.
    c.insertText(QString(flow::kPageBreak), typingFormat(c));
    splitParagraph(c);
    c.endEditBlock();
}

void insertSectionBreak(WriteDoc &doc, QTextCursor &c)
{
    if (isLocked(c.block()) || inTable(c.block())) return;   // (a section ends in a paragraph that is not in a table's cell)
    c.beginEditBlock();
    if (c.hasSelection()) removeSelection(doc, c);
    // The paragraph the cursor is in ends the section; the cursor goes to a new paragraph in the section that follows.
    const QTextBlock first = splitParagraph(c);
    doc.insertSectionBreak(first);
    c.endEditBlock();
}

void removeSelection(WriteDoc &doc, QTextCursor &c)
{
    if (!c.hasSelection()) return;
    if (removeSelectionAcrossCells(doc, c)) return;   // (cells are emptied, and tables the selection holds go: paragraphs of different cells are never joined)
    QTextDocument *d = c.document();
    c.beginEditBlock();
    const int from = c.selectionStart(), to = c.selectionEnd();
    QTextBlock a = d->findBlock(from), b = d->findBlock(to);
    if (a == b) {
        if (isLocked(a)) {
            // The element kept as XML goes, with its paragraph.
            if (a.previous().isValid()) {
                QTextCursor t(d);
                t.setPosition(a.position() - 1);
                t.setPosition(a.position() + a.length() - 1, QTextCursor::KeepAnchor);
                t.removeSelectedText();
                QTextBlock m = d->findBlock(std::max(0, a.position() - 1));
                c.setPosition(std::min(m.position() + m.length() - 1, d->characterCount() - 1));
            } else {
                c.removeSelectedText();
                QTextBlockFormat nf = a.blockFormat();
                nf.clearProperty(prop::RawBlock);
                setBlockFormatKeepingList(a, nf);
            }
        } else {
            c.removeSelectedText();
        }
        c.endEditBlock();
        return;
    }
    const QTextBlockFormat aFormat = a.blockFormat(), bFormat = b.blockFormat();
    const QTextCharFormat bMark = markFormat(b);
    const bool aLocked = isLocked(a);
    // The XML kept between the two paragraphs' text stays: it ends up after the paragraph they become, in its order (a content control's end
    // that was before the last paragraph is before what comes after the result, and so still ends it).
    QVector<Block> between;
    auto carry = [&](const QTextBlockFormat &f, int key) {
        if (f.hasProperty(key)) between += unpackBlocks(storedBytes(f, key));
    };
    carry(aFormat, prop::RawAfter);
    for (QTextBlock m = a.next(); m.isValid() && m != b; m = m.next()) {
        const QTextBlockFormat mf = m.blockFormat();
        carry(mf, prop::RawBefore);
        carry(mf, prop::RawAfter);
    }
    carry(bFormat, prop::RawBefore);
    c.removeSelectedText();
    QTextBlock m = d->findBlock(from);
    QTextBlockFormat nf = m.blockFormat();
    clearTail(&nf);
    for (int key : {int(flow::fp::SectionBreakAfter), int(prop::SectionData)})
        if (bFormat.hasProperty(key)) nf.setProperty(key, bFormat.property(key));
    QVector<Block> rawAfter = between;
    rawAfter += unpackBlocks(storedBytes(bFormat, prop::RawAfter));
    if (aLocked) {
        // What is left of the last paragraph takes the place of the one that was kept as XML, with the XML that was before that one, and what was
        // between them before it.
        const QByteArray before = storedBytes(aFormat, prop::RawBefore);
        const int list = nf.objectIndex();
        nf = bFormat;
        nf.setObjectIndex(list);
        clearHead(&nf);
        if (aFormat.hasProperty(prop::LastSection)) nf.setProperty(prop::LastSection, aFormat.property(prop::LastSection));
        clearTail(&nf);
        for (int key : {int(flow::fp::SectionBreakAfter), int(prop::SectionData)})
            if (bFormat.hasProperty(key)) nf.setProperty(key, bFormat.property(key));
        QVector<Block> head = unpackBlocks(before);
        head += between;
        rawAfter = unpackBlocks(storedBytes(bFormat, prop::RawAfter));
        if (!head.isEmpty()) setStored(nf, prop::RawBefore, packBlocks(head));
        QTextCursor(m).setBlockCharFormat(bMark);
        if (!m.previous().isValid()) setStored(nf, prop::MarkFormat, packFormat(bMark));
    }
    if (!rawAfter.isEmpty()) setStored(nf, prop::RawAfter, packBlocks(rawAfter));
    QTextCursor(m).setBlockFormat(nf);
    c.setPosition(from);
    c.endEditBlock();
}

void deleteBack(WriteDoc &doc, QTextCursor &c, bool byWord)
{
    if (c.hasSelection()) {
        removeSelection(doc, c);
        return;
    }
    QTextDocument *d = c.document();
    if (isLocked(c.block())) return;
    if (byWord && !c.atBlockStart()) {
        c.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        removeSelection(doc, c);
        return;
    }
    const int at = visibleBefore(d, c.position());
    if (at >= 0) {
        if (at + 1 == c.position()) {
            c.deletePreviousChar();
            return;
        }
        // The pieces kept as XML between the cursor and the letter stay: the letter goes alone.
        QTextCursor t(d);
        t.setPosition(at + 1);
        t.deletePreviousChar();
        return;
    }
    // At the start of a paragraph.
    QTextBlock b = c.block();
    if (b.blockFormat().hasProperty(prop::NumId)) {
        c.beginEditBlock();
        removeFromList(doc, b);
        c.endEditBlock();
        return;
    }
    QTextBlock prev = b.previous();
    if (!prev.isValid() || isLocked(prev) || !canJoinBlocks(prev, b)) return;
    QTextCursor t(d);
    t.setPosition(b.position());
    t.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    c.beginEditBlock();
    c.setPosition(t.selectionStart());
    c.setPosition(t.selectionEnd(), QTextCursor::KeepAnchor);
    removeSelection(doc, c);
    c.endEditBlock();
}

void deleteForward(WriteDoc &doc, QTextCursor &c, bool byWord)
{
    if (c.hasSelection()) {
        removeSelection(doc, c);
        return;
    }
    QTextDocument *d = c.document();
    if (isLocked(c.block())) return;
    if (byWord && !c.atBlockEnd()) {
        c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        removeSelection(doc, c);
        return;
    }
    const int at = visibleAfter(d, c.position());
    if (at >= 0) {
        if (at == c.position()) {
            c.deleteChar();
            return;
        }
        QTextCursor t(d);
        t.setPosition(at);
        t.deleteChar();
        return;
    }
    QTextBlock next = c.block().next();
    if (!next.isValid() || isLocked(next) || !canJoinBlocks(c.block(), next)) return;
    c.movePosition(QTextCursor::EndOfBlock);
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    removeSelection(doc, c);
}

void insertTab(WriteDoc &doc, QTextCursor &c, bool back)
{
    const QTextBlock b = c.block();
    if (!c.hasSelection() && b.blockFormat().hasProperty(prop::NumId) && (c.atBlockStart() || back)) {
        changeIndent(doc, c, back ? -36 : 36);
        return;
    }
    if (tabThroughCells(doc, c, back)) return;
    if (back) return;
    insertText(doc, c, QStringLiteral("\t"));
}

// ---- the clipboard

QTextDocumentFragment selectionFragment(const QTextCursor &c)
{
    return c.hasSelection() ? QTextDocumentFragment(c) : QTextDocumentFragment();
}

QString plainTextOf(const QTextDocumentFragment &f)
{
    QString t = f.toPlainText();
    t.remove(QChar::ObjectReplacementCharacter);
    t.replace(QChar(QChar::LineSeparator), QLatin1Char('\n'));
    return t;
}

// Pastes, and says where what was pasted is (from, to) when the paste is done.
static void pasteInto(WriteDoc &doc, QTextCursor &c, const QTextDocumentFragment &fragment, int *pastedFrom, int *pastedTo)
{
    c.beginEditBlock();
    if (c.hasSelection()) removeSelection(doc, c);
    if (isLocked(c.block())) {
        c.movePosition(QTextCursor::EndOfBlock);
        const QString style = doc.defaultParagraphStyle();
        QTextBlockFormat nf = doc.styleBlockFormat(style);
        if (c.block().blockFormat().hasProperty(flow::fp::Cell)) nf.setProperty(flow::fp::Cell, c.block().blockFormat().property(flow::fp::Cell));
        c.insertBlock(nf, doc.styleCharFormat(style));
    }
    QTextDocument *d = c.document();
    // The paragraph pasted into: its paragraph's look and what it keeps stay with the part of it that is there after the paste (the text
    // after the cursor), and what it keeps before it with the part before. (The text engine's own way of pasting gives the paragraph the
    // look of what was copied where the cursor is at its start, and takes its section break away.)
    const QTextBlock target = c.block();
    const QTextBlockFormat tf = target.blockFormat();
    const int targetCell = cellKeyOf(target);
    const QTextCharFormat tmark = markFormat(target);
    const int targetListId = target.textList() ? target.blockFormat().objectIndex() : -1;   // (the list, found again after the paste: it may be gone with the paragraph that was its only one)
    const bool atStart = c.position() == target.position();
    const QVector<Wrapper> wrappers = wrappersOpenAt(target, c.position());
    const int from = c.position();
    c.insertFragment(fragment);
    QTextList *targetList = targetListId >= 0 ? qobject_cast<QTextList *>(d->object(targetListId)) : nullptr;
    const int to = c.position();
    *pastedFrom = from;
    const QTextBlock first = d->findBlock(from), last = d->findBlock(to);
    const bool several = first != last;
    auto sameLook = [](QTextBlockFormat x, QTextBlockFormat y) {
        x.setObjectIndex(-1);
        y.setObjectIndex(-1);
        return x == y;
    };
    if (!several) {
        if (!sameLook(first.blockFormat(), tf) || (first.textList() != targetList)) setBlockFormatInList(first, tf, targetList);
        if (markFormat(first) != tmark) setMarkFormat(first, tmark);
    } else {
        for (QTextBlock b = first;; b = b.next()) {
            QTextBlockFormat nf = b.blockFormat();
            if (b == first) {
                clearTail(&nf);
                if (atStart) clearHead(&nf);
                copyHead(tf, &nf);
                if (nf.hasProperty(prop::RawBlock) && b.length() != 2) nf.clearProperty(prop::RawBlock);
                if (nf != b.blockFormat()) setBlockFormatKeepingList(b, nf);
            } else if (b == last) {
                QTextBlockFormat rest = tf;
                clearHead(&rest);
                setBlockFormatInList(b, rest, targetList);
                setMarkFormat(b, tmark);
            } else {
                clearHead(&nf);
                clearTail(&nf);
                if (nf.hasProperty(prop::RawBlock) && b.length() != 2) nf.clearProperty(prop::RawBlock);
                if (nf != b.blockFormat()) setBlockFormatKeepingList(b, nf);
            }
            if (b == last) break;
        }
        rewrap(wrappers, first, last);
    }
    // What is pasted is in the cell it was pasted into (or in none).
    giveCellKey(d->findBlock(from), d->findBlock(to), targetCell);
    // The ids a paragraph carries in the file are its own: a copy has none (the first, if it is the target's, kept them above).
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() <= to; b = b.next()) {
        QTextBlockFormat nf = b.blockFormat();
        if (b != first && nf.hasProperty(prop::ParaAttrs)) {
            nf.clearProperty(prop::ParaAttrs);
            setBlockFormatKeepingList(b, nf);
        }
    }
    // And the stand-in characters that came without their paragraph (or the piece they stood for) go.
    QVector<QPair<int, int>> strays;
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() <= to; b = b.next()) {
        if (isLocked(b)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom) && cf.stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) strays.push_back({f.position(), f.length()});
        }
    }
    for (int i = int(strays.size()) - 1; i >= 0; --i) {
        QTextCursor t(d);
        t.setPosition(strays[i].first);
        t.setPosition(strays[i].first + strays[i].second, QTextCursor::KeepAnchor);
        t.removeSelectedText();
    }
    *pastedTo = c.position();
    c.endEditBlock();
}

void pasteFragment(WriteDoc &doc, QTextCursor &c, const QTextDocumentFragment &fragment)
{
    if (fragment.isEmpty()) return;
    int from = 0, to = 0;
    pasteInto(doc, c, fragment, &from, &to);
}

namespace {

// The values of the attributes in kept XML that name a relationship of the file (r:id, r:embed, r:link...).
struct Reference {
    qsizetype at = 0, length = 0;
    QString id;
};

QVector<Reference> referencesIn(const QByteArray &xml)
{
    QVector<Reference> out;
    for (qsizetype i = xml.indexOf("r:"); i >= 0; i = xml.indexOf("r:", i + 1)) {
        if (i == 0 || (xml[i - 1] != ' ' && xml[i - 1] != '\n' && xml[i - 1] != '\t' && xml[i - 1] != '\r')) continue;
        qsizetype j = i + 2;
        while (j < xml.size() && ((xml[j] >= 'a' && xml[j] <= 'z') || (xml[j] >= 'A' && xml[j] <= 'Z'))) ++j;
        if (j + 1 >= xml.size() || xml[j] != '=') continue;
        const char quote = xml[j + 1];
        if (quote != '"' && quote != '\'') continue;
        const qsizetype from = j + 2, to = xml.indexOf(quote, from);
        if (to < 0) break;
        out.push_back({from, to - from, QString::fromUtf8(xml.mid(from, to - from))});
        i = to;
    }
    return out;
}

} // namespace

Clip copySelection(const WriteDoc &doc, const QTextCursor &c)
{
    Clip clip;
    clip.fragment = selectionFragment(c);
    clip.source = doc.id();
    clip.generation = byteStoreGeneration();
    if (!c.hasSelection()) return clip;
    QSet<QString> ids;
    auto collect = [&](const QByteArray &raw) {
        for (const Reference &r : referencesIn(raw)) ids.insert(r.id);
    };
    QTextDocument *d = c.document();
    const int from = c.selectionStart(), to = c.selectionEnd();
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (isLocked(b)) {
            for (const Block &r : unpackBlocks(storedBytes(b.blockFormat(), prop::RawBlock))) collect(r.raw);
            continue;
        }
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (f.position() + f.length() <= from || f.position() >= to || !f.charFormat().hasProperty(prop::Atom)) continue;
            collect(unpackAtom(storedBytes(f.charFormat(), prop::Atom)).raw);
        }
    }
    for (const QString &id : std::as_const(ids)) {
        const Relationship r = doc.mainRelationship(id);
        if (r.id.isEmpty()) continue;
        Clip::Link link;
        link.id = r.id;
        link.type = r.type;
        link.target = r.target;
        link.external = r.external;
        if (!r.external && r.type.endsWith(QLatin1String("/image"))) link.data = doc.partData(r.target, &link.contentType);
        clip.links.push_back(link);
    }
    return clip;
}

void pasteClip(WriteDoc &doc, QTextCursor &c, const Clip &clip)
{
    if (clip.isEmpty()) return;
    const bool same = clip.source == doc.id();
    if (!same && clip.generation != byteStoreGeneration()) {
        pasteText(doc, c, plainTextOf(clip.fragment));   // (the kept bytes its formats stood for are gone)
        return;
    }
    c.beginEditBlock();
    int from = 0, to = 0;
    pasteInto(doc, c, clip.fragment, &from, &to);
    if (same) {
        c.endEditBlock();
        return;
    }
    // From another document: what the pasted pieces refer to by relationship Id is made over in this one's, or the pieces are left out.
    QTextDocument *d = c.document();
    QHash<QString, QString> mapped;
    auto remap = [&](const QByteArray &raw, bool *ok) {
        *ok = true;
        QByteArray out;
        qsizetype last = 0;
        for (const Reference &r : referencesIn(raw)) {
            QString id = mapped.value(r.id);
            if (id.isEmpty()) {
                const Clip::Link *link = nullptr;
                for (const Clip::Link &l : clip.links)
                    if (l.id == r.id) link = &l;
                if (!link) { *ok = false; return raw; }
                if (link->external) id = doc.addExternalLink(link->type, link->target);
                else if (link->type.endsWith(QLatin1String("/image")) && !link->data.isEmpty()) id = doc.addMedia(link->data, link->target.section(QLatin1Char('.'), -1).toLower(), link->contentType);
                else { *ok = false; return raw; }
                mapped.insert(r.id, id);
            }
            out += raw.mid(last, r.at - last) + id.toUtf8();
            last = r.at + r.length;
        }
        out += raw.mid(last);
        return out;
    };
    struct Fix { int from, to; QTextCharFormat format; bool remove; };
    QVector<Fix> fixes;
    QVector<int> standIns;
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() <= to; b = b.next()) {
        if (isLocked(b)) {
            QVector<Block> blocks = unpackBlocks(storedBytes(b.blockFormat(), prop::RawBlock));
            bool all = true, changed = false;
            for (Block &r : blocks) {
                bool ok = true;
                const QByteArray raw = remap(r.raw, &ok);
                all = all && ok;
                if (ok && raw != r.raw) {
                    r.raw = raw;
                    changed = true;
                }
            }
            if (!all) {
                standIns.push_back(b.position());
            } else if (changed) {
                QTextBlockFormat nf = b.blockFormat();
                setStored(nf, prop::RawBlock, packBlocks(blocks));
                setBlockFormatKeepingList(b, nf);
            }
            continue;
        }
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            // A reference to one of the other document's notes points at nothing here (its text isn't), and an object's place on the page is a place in the
            // other's objects: the first is left out, and the second isn't carried (what the object is as the file has it is kept).
            const QString field = f.charFormat().stringProperty(tp::Field);
            const bool referenceToNote = field.startsWith(QLatin1String("footnote:"));
            Inline a;
            const bool atom = f.charFormat().hasProperty(prop::Atom);
            if (atom) a = unpackAtom(storedBytes(f.charFormat(), prop::Atom));
            if (referenceToNote || (atom && (a.tag == QLatin1String("footnoteReference") || a.tag == QLatin1String("endnoteReference")))) {
                fixes.push_back({f.position(), f.position() + f.length(), QTextCharFormat(), true});
                continue;
            }
            QTextCharFormat nf = f.charFormat();
            bool changed = false;
            if (nf.hasProperty(flow::fp::AnchorId)) {
                nf.clearProperty(flow::fp::AnchorId);
                changed = true;
            }
            if (atom && !referencesIn(a.raw).isEmpty()) {
                bool ok = true;
                const QByteArray raw = remap(a.raw, &ok);
                if (!ok) {
                    fixes.push_back({f.position(), f.position() + f.length(), QTextCharFormat(), true});
                    continue;
                }
                if (raw != a.raw) {
                    a.raw = raw;
                    setStored(nf, prop::Atom, packAtom(a));
                    changed = true;
                }
            }
            if (changed) fixes.push_back({f.position(), f.position() + f.length(), nf, false});
        }
    }
    for (int i = int(fixes.size()) - 1; i >= 0; --i) {
        QTextCursor t(d);
        t.setPosition(fixes[i].from);
        t.setPosition(fixes[i].to, QTextCursor::KeepAnchor);
        if (fixes[i].remove) t.removeSelectedText();
        else t.setCharFormat(fixes[i].format);
    }
    for (int i = int(standIns.size()) - 1; i >= 0; --i) {
        const QTextBlock b = d->findBlock(standIns[i]);
        if (!b.isValid() || !isLocked(b)) continue;
        QTextCursor t(d);
        t.setPosition(b.position());
        t.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
        removeSelection(doc, t);
    }
    c.endEditBlock();
}

void pasteText(WriteDoc &doc, QTextCursor &c, const QString &text)
{
    QString t = text;
    t.replace(QLatin1String("\r\n"), QLatin1String("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));
    c.beginEditBlock();
    if (c.hasSelection()) removeSelection(doc, c);
    const QStringList lines = t.split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); ++i) {
        if (i > 0) insertParagraph(doc, c);
        if (!lines[i].isEmpty()) insertText(doc, c, lines[i]);
    }
    c.endEditBlock();
}

// ---- moving

void moveCursor(QTextCursor &c, QTextCursor::MoveOperation op, bool select, int n)
{
    const QTextCursor::MoveMode mode = select ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    QTextDocument *d = c.document();
    if (op == QTextCursor::NextCharacter || op == QTextCursor::Right) {
        for (int i = 0; i < n; ++i) {
            if (!select && c.hasSelection()) {
                c.setPosition(c.selectionEnd());
                continue;
            }
            // The pieces kept as XML that show nothing are passed on the way to the next letter.
            for (int guard = 0; guard < 1000 && !c.atBlockEnd() && isZeroWidth(charAt(d, c.position())); ++guard) c.movePosition(QTextCursor::NextCharacter, mode);
            c.movePosition(QTextCursor::NextCharacter, mode);
        }
    } else if (op == QTextCursor::PreviousCharacter || op == QTextCursor::Left) {
        for (int i = 0; i < n; ++i) {
            if (!select && c.hasSelection()) {
                c.setPosition(c.selectionStart());
                continue;
            }
            for (int guard = 0; guard < 1000 && !c.atBlockStart() && isZeroWidth(charAt(d, c.position() - 1)); ++guard) c.movePosition(QTextCursor::PreviousCharacter, mode);
            c.movePosition(QTextCursor::PreviousCharacter, mode);
        }
    } else {
        c.movePosition(op, mode, n);
    }
}

// ---- characters

bool allSelected(const QTextCursor &c, const std::function<bool(const QTextCharFormat &)> &test)
{
    if (!c.hasSelection()) return test(typingFormat(c));
    QTextDocument *d = c.document();
    const int from = c.selectionStart(), to = c.selectionEnd();
    // The pieces kept as XML (a bookmark, a field's code, a note's mark) are not the text's look: they are left out, unless the selection is nothing else.
    for (const bool withAtoms : {false, true}) {
        bool any = false;
        for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (f.position() + f.length() <= from || f.position() >= to) continue;
                if (!withAtoms && isAtom(f.charFormat())) continue;
                any = true;
                if (!test(f.charFormat())) return false;
            }
        if (any) return true;
    }
    return true;
}

void changeCharFormat(WriteDoc &doc, QTextCursor &c, const std::function<void(QTextCharFormat &)> &change)
{
    Q_UNUSED(doc);
    QTextDocument *d = c.document();
    if (!c.hasSelection()) {
        QTextCharFormat f = typingFormat(c);
        change(f);
        c.setCharFormat(f);
        if (c.block().length() <= 1) {
            QTextCharFormat mark = markFormat(c.block());
            change(mark);
            setMarkFormat(c.block(), mark);
        }
        return;
    }
    struct Edit { int from, to; QTextCharFormat format; };
    QVector<Edit> edits;
    const int from = c.selectionStart(), to = c.selectionEnd();
    QVector<QPair<QTextBlock, QTextCharFormat>> marks;
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const int fs = std::max(f.position(), from), fe = std::min(f.position() + f.length(), to);
            if (fs >= fe) continue;
            QTextCharFormat nf = f.charFormat();
            change(nf);
            if (nf != f.charFormat()) edits.push_back({fs, fe, nf});
        }
        if (from <= b.position() && to > b.position() + b.length() - 1 && !isLocked(b)) {
            QTextCharFormat nf = markFormat(b);
            change(nf);
            if (nf != markFormat(b)) marks.push_back({b, nf});
        }
    }
    c.beginEditBlock();
    for (const Edit &e : std::as_const(edits)) {
        QTextCursor t(d);
        t.setPosition(e.from);
        t.setPosition(e.to, QTextCursor::KeepAnchor);
        t.setCharFormat(e.format);
    }
    for (const auto &m : std::as_const(marks)) setMarkFormat(m.first, m.second);
    c.endEditBlock();
}

void toggleBold(WriteDoc &doc, QTextCursor &c)
{
    const bool on = !allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; });
    changeCharFormat(doc, c, [on](QTextCharFormat &f) { f.setFontWeight(on ? QFont::Bold : QFont::Normal); });
}

void toggleItalic(WriteDoc &doc, QTextCursor &c)
{
    const bool on = !allSelected(c, [](const QTextCharFormat &f) { return f.fontItalic(); });
    changeCharFormat(doc, c, [on](QTextCharFormat &f) { f.setFontItalic(on); });
}

void toggleUnderline(WriteDoc &doc, QTextCursor &c)
{
    const bool on = !allSelected(c, [](const QTextCharFormat &f) { return f.underlineStyle() != QTextCharFormat::NoUnderline; });
    changeCharFormat(doc, c, [on](QTextCharFormat &f) {
        f.setUnderlineStyle(on ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline);
        f.clearProperty(tp::UnderlineKind);
        f.clearProperty(prop::UnderlineAttrs);
        if (!on) f.clearProperty(QTextFormat::TextUnderlineColor);
        f.clearProperty(QTextFormat::FontUnderline);
    });
}

void toggleStrike(WriteDoc &doc, QTextCursor &c)
{
    const bool on = !allSelected(c, [](const QTextCharFormat &f) { return f.fontStrikeOut(); });
    changeCharFormat(doc, c, [on](QTextCharFormat &f) {
        f.setFontStrikeOut(on);
        f.clearProperty(prop::DoubleStrike);
    });
}

void toggleSubscript(WriteDoc &doc, QTextCursor &c)
{
    const bool on = !allSelected(c, [](const QTextCharFormat &f) { return f.verticalAlignment() == QTextCharFormat::AlignSubScript; });
    changeCharFormat(doc, c, [on](QTextCharFormat &f) { f.setVerticalAlignment(on ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal); });
}

void toggleSuperscript(WriteDoc &doc, QTextCursor &c)
{
    const bool on = !allSelected(c, [](const QTextCharFormat &f) { return f.verticalAlignment() == QTextCharFormat::AlignSuperScript; });
    changeCharFormat(doc, c, [on](QTextCharFormat &f) { f.setVerticalAlignment(on ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal); });
}

void setFontFamily(WriteDoc &doc, QTextCursor &c, const QString &family)
{
    changeCharFormat(doc, c, [family](QTextCharFormat &f) {
        if (family == QLatin1String("+Heading") || family == QLatin1String("+Body")) {
            f.clearProperty(QTextFormat::FontFamilies);
            f.clearProperty(QTextFormat::FontFamily);
            f.setProperty(tp::ThemeFont, family == QLatin1String("+Heading") ? QStringLiteral("major") : QStringLiteral("minor"));
        } else {
            f.setFontFamilies({family});
            f.clearProperty(tp::ThemeFont);
        }
    });
}

void setFontSize(WriteDoc &doc, QTextCursor &c, double points)
{
    points = std::clamp(points, 1.0, 1638.0);
    changeCharFormat(doc, c, [points](QTextCharFormat &f) { f.setFontPointSize(points); });
}

void changeFontSize(WriteDoc &doc, QTextCursor &c, int steps)
{
    static const double sizes[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10.5, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72, 80, 96, 120, 144, 200, 300, 500, 1000, 1638};
    const double now = typingFormat(c).fontPointSize() > 0 ? typingFormat(c).fontPointSize() : 11;
    double target = now;
    if (steps > 0) {
        for (double s : sizes)
            if (s > now + 0.01) {
                target = s;
                break;
            }
    } else {
        for (int i = int(sizeof sizes / sizeof *sizes) - 1; i >= 0; --i)
            if (sizes[i] < now - 0.01) {
                target = sizes[i];
                break;
            }
    }
    setFontSize(doc, c, target);
}

void setTextColor(WriteDoc &doc, QTextCursor &c, const QColor &color)
{
    changeCharFormat(doc, c, [color](QTextCharFormat &f) {
        f.clearProperty(prop::ColorAttrs);
        f.clearProperty(prop::ColorAttrsRgb);
        if (color.isValid()) {
            f.setForeground(QBrush(color));
            f.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
        } else {
            f.clearProperty(QTextFormat::ForegroundBrush);
            f.clearProperty(tp::ColorRefP);
        }
    });
}

void setHighlight(WriteDoc &doc, QTextCursor &c, const QString &name)
{
    changeCharFormat(doc, c, [name](QTextCharFormat &f) {
        if (name.isEmpty()) f.clearProperty(tp::HighlightRefP);
        else f.setProperty(tp::HighlightRefP, ColorRef::rgb(highlightColor(name)).toString());
    });
}

namespace {

// Properties of a character format that a character style or a paragraph style gives, and so are told apart from what the text says.
const int kStyleDriven[] = {QTextFormat::FontFamilies,   QTextFormat::FontPointSize,    QTextFormat::FontWeight,   QTextFormat::FontItalic,   QTextFormat::TextUnderlineStyle,
                            QTextFormat::FontStrikeOut,  QTextFormat::ForegroundBrush,  QTextFormat::FontCapitalization, QTextFormat::TextVerticalAlignment,
                            QTextFormat::FontLetterSpacing, QTextFormat::FontLetterSpacingType, QTextFormat::TextUnderlineColor,
                            tp::ThemeFont, tp::ColorRefP, tp::HighlightRefP, tp::UnderlineKind, prop::UnderlineAttrs, prop::ColorAttrs, prop::ColorAttrsRgb, prop::DoubleStrike, tp::HiddenText, prop::Raised, tp::NoProof, tp::Language};

// The format with what `was` gave it (and it still has) taken for what `now` gives.
QTextCharFormat restyled(const QTextCharFormat &f, const QTextCharFormat &was, const QTextCharFormat &now)
{
    QTextCharFormat out = f;
    for (int key : kStyleDriven) {
        if (f.property(key) != was.property(key)) continue;   // the text says this itself
        if (now.hasProperty(key)) out.setProperty(key, now.property(key));
        else out.clearProperty(key);
    }
    return out;
}

} // namespace

void clearFormatting(WriteDoc &doc, QTextCursor &c)
{
    c.beginEditBlock();
    applyParagraphStyle(doc, c, doc.defaultParagraphStyle());
    const QTextCharFormat base = doc.styleCharFormat(doc.defaultParagraphStyle());
    changeCharFormat(doc, c, [&](QTextCharFormat &f) {
        QTextCharFormat nf = base;
        // What makes the text a piece kept as XML, and what the file says besides, stays.
        // (And the language the run states, which the file keeps: spelling follows what the file says.)
        for (int key : {int(prop::Atom), int(tp::Field), int(prop::RawRPr), int(tp::Language)})
            if (f.hasProperty(key)) nf.setProperty(key, f.property(key));
        f = nf;
    });
    c.endEditBlock();
}

// ---- paragraphs

void changeBlockFormat(WriteDoc &doc, QTextCursor &c, const std::function<void(QTextBlockFormat &)> &change)
{
    Q_UNUSED(doc);
    c.beginEditBlock();
    for (const QTextBlock &b : selectedBlocks(c)) {
        if (isLocked(b)) continue;
        QTextBlockFormat nf = b.blockFormat();
        const QTextBlockFormat was = nf;
        change(nf);
        if (nf != was) QTextCursor(b).setBlockFormat(nf);
    }
    c.endEditBlock();
}

void setAlignment(WriteDoc &doc, QTextCursor &c, Qt::Alignment alignment)
{
    changeBlockFormat(doc, c, [alignment](QTextBlockFormat &bf) {
        bf.setAlignment(alignment);
        bf.clearProperty(prop::JcRaw);   // (a chosen alignment is the plain kind)
    });
}

static void setBlockList(WriteDoc &doc, QTextBlock block, int numId, int level, QTextList *join);

void changeIndent(WriteDoc &doc, QTextCursor &c, double points)
{
    c.beginEditBlock();
    for (const QTextBlock &b : selectedBlocks(c)) {
        if (isLocked(b)) continue;
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::NumId)) {
            const int level = std::clamp(bf.intProperty(prop::NumLevel) + (points > 0 ? 1 : -1), 0, 8);
            setBlockList(doc, b, bf.intProperty(prop::NumId), level, nullptr);
            continue;
        }
        QTextBlockFormat nf = bf;
        nf.setLeftMargin(std::max(0.0, bf.leftMargin() + points));
        if (nf != bf) QTextCursor(b).setBlockFormat(nf);
    }
    c.endEditBlock();
}

void setIndents(WriteDoc &doc, QTextCursor &c, double left, double right, double firstLine)
{
    changeBlockFormat(doc, c, [&](QTextBlockFormat &bf) {
        bf.setLeftMargin(std::max(0.0, left));
        bf.setRightMargin(std::max(0.0, right));
        bf.setTextIndent(firstLine);
    });
}

namespace {
// What the paragraph's spacing says on one side that a measure set by a person replaces: that the spacing is automatic, and a number of lines.
void dropSpacingAttributes(QTextBlockFormat &bf, const char *side)
{
    if (!bf.hasProperty(prop::SpacingAttrs)) return;
    QByteArray attrs = storedBytes(bf, prop::SpacingAttrs);
    for (const char *suffix : {"Autospacing", "Lines"}) {
        const QByteArray name = QByteArray(" w:") + side + suffix + "=\"";
        for (int at = attrs.indexOf(name); at >= 0; at = attrs.indexOf(name)) {
            const int end = attrs.indexOf('"', at + name.size());
            if (end < 0) break;
            attrs.remove(at, end + 1 - at);
        }
    }
    if (attrs.isEmpty()) bf.clearProperty(prop::SpacingAttrs);
    else setStored(bf, prop::SpacingAttrs, attrs);
}
} // namespace

void setSpaceBefore(QTextBlockFormat &bf, double points)
{
    bf.setTopMargin(std::max(0.0, points));
    bf.clearProperty(tp::SpaceBeforeAuto);
    dropSpacingAttributes(bf, "before");
}

void setSpaceAfter(QTextBlockFormat &bf, double points)
{
    bf.setBottomMargin(std::max(0.0, points));
    bf.clearProperty(tp::SpaceAfterAuto);
    dropSpacingAttributes(bf, "after");
}

void setSpacing(WriteDoc &doc, QTextCursor &c, double before, double after)
{
    changeBlockFormat(doc, c, [&](QTextBlockFormat &bf) {
        setSpaceBefore(bf, before);
        setSpaceAfter(bf, after);
    });
}

void setLineSpacing(WriteDoc &doc, QTextCursor &c, QTextBlockFormat::LineHeightTypes type, double value)
{
    changeBlockFormat(doc, c, [&](QTextBlockFormat &bf) {
        if (type == QTextBlockFormat::ProportionalHeight) bf.setLineHeight(std::max(0.1, value) * 100.0, type);
        else bf.setLineHeight(std::max(0.1, value), type);
    });
}

// ---- lists

ListKind listKindOf(const WriteDoc &doc, const QTextBlock &block)
{
    const QTextBlockFormat bf = block.blockFormat();
    if (!bf.hasProperty(prop::NumId)) return ListKind::None;
    const AbstractNum *a = doc.numbering().abstractOf(bf.intProperty(prop::NumId));
    const int level = std::clamp(bf.intProperty(prop::NumLevel), 0, 8);
    if (!a || !a->levels[level].present) return ListKind::None;
    const QString fmt = a->levels[level].numFmt;
    return fmt == QLatin1String("bullet") || fmt == QLatin1String("none") ? ListKind::Bullets : ListKind::Numbers;
}

// Makes the paragraph an item of the list of that definition and level, in `join` (null: the nearest list before it that is the same, else
// a new one).
static void setBlockList(WriteDoc &doc, QTextBlock block, int numId, int level, QTextList *join)
{
    level = std::clamp(level, 0, std::max(0, doc.deepestLevel(numId)));   // (a definition may have fewer than nine levels)
    const QTextListFormat lf = doc.listFormat(numId, level);
    if (!join) {
        for (QTextBlock p = block.previous(); p.isValid(); p = p.previous()) {
            const QTextBlockFormat pf = p.blockFormat();
            if (!pf.hasProperty(prop::NumId) || pf.intProperty(prop::NumId) != numId) continue;
            const int pl = pf.intProperty(prop::NumLevel);
            if (pl < level) break;
            if (pl == level && p.textList()) {
                join = p.textList();
                break;
            }
        }
    }
    if (QTextList *old = block.textList()) old->remove(block);
    QTextBlockFormat nf = block.blockFormat();
    nf.setObjectIndex(-1);
    nf.setProperty(prop::NumId, numId);
    nf.setProperty(prop::NumLevel, level);
    nf.clearProperty(tp::BulletChar);
    const ParaProps indents = doc.levelParagraph(numId, level);
    nf.setLeftMargin(indents.indLeft.value_or(0) / 20.0);
    nf.setTextIndent(indents.indFirst.value_or(0) / 20.0);
    QTextCursor t(block);
    t.setBlockFormat(nf);
    if (join) join->add(block);
    else t.createList(lf);
}

void toggleList(WriteDoc &doc, QTextCursor &c, bool numbered)
{
    const QVector<QTextBlock> blocks = selectedBlocks(c);
    const ListKind want = numbered ? ListKind::Numbers : ListKind::Bullets;
    bool all = true, any = false;
    for (const QTextBlock &b : blocks) {
        if (isLocked(b)) continue;
        any = true;
        all = all && listKindOf(doc, b) == want;
    }
    if (!any) return;
    c.beginEditBlock();
    if (all) {
        for (const QTextBlock &b : blocks)
            if (!isLocked(b)) removeFromList(doc, b);
        c.endEditBlock();
        return;
    }
    // Joins the list just before, if it is of this kind; else starts one.
    int numId = 0, level = 0;
    const QTextBlock before = blocks.first().previous();
    if (before.isValid() && listKindOf(doc, before) == want) {
        numId = before.blockFormat().intProperty(prop::NumId);
        level = before.blockFormat().intProperty(prop::NumLevel);
    } else {
        numId = doc.newListInstance(numbered);
    }
    QTextList *list = nullptr;
    for (const QTextBlock &b : blocks) {
        if (isLocked(b)) continue;
        setBlockList(doc, b, numId, level, list);
        list = b.textList();
    }
    c.endEditBlock();
}

QString paragraphStyleAt(const WriteDoc &doc, const QTextCursor &c)
{
    const QString id = c.block().blockFormat().stringProperty(tp::StyleName);
    return id.isEmpty() ? doc.defaultParagraphStyle() : id;
}

static void applyStyleToBlock(WriteDoc &doc, QTextBlock b, const QString &styleId)
{
    const QTextBlockFormat bf = b.blockFormat();
    const QString was = bf.stringProperty(tp::StyleName).isEmpty() ? doc.defaultParagraphStyle() : bf.stringProperty(tp::StyleName);
    QTextBlockFormat nf = doc.styleBlockFormat(styleId);
    // What belongs to the paragraph in the file, not to its style.
    for (int key : {int(prop::MarkFormat), int(prop::LastSection), int(prop::RawBefore), int(prop::RawAfter), int(prop::ParaAttrs), int(prop::RawPPr), int(prop::SpacingAttrs), int(prop::IndAttrs), int(prop::OddTabs), int(flow::fp::SectionBreakAfter), int(prop::SectionData), int(flow::fp::Cell)})
        if (bf.hasProperty(key)) nf.setProperty(key, bf.property(key));
    const int numId = nf.intProperty(prop::NumId), level = nf.intProperty(prop::NumLevel);
    const bool wasList = bf.hasProperty(prop::NumId);
    nf.clearProperty(prop::NumId);
    nf.clearProperty(prop::NumLevel);
    nf.clearProperty(tp::BulletChar);
    // A paragraph in a list keeps its list when the style names none.
    if (wasList && numId == 0) {
        nf.setProperty(prop::NumId, bf.property(prop::NumId));
        nf.setProperty(prop::NumLevel, bf.property(prop::NumLevel));
        nf.setLeftMargin(bf.leftMargin());
        nf.setTextIndent(bf.textIndent());
        if (bf.hasProperty(tp::BulletChar)) nf.setProperty(tp::BulletChar, bf.property(tp::BulletChar));
    }
    setBlockFormatKeepingList(b, nf);
    if (numId > 0) setBlockList(doc, b, numId, level, nullptr);

    // The characters: what the old style gave them becomes what the new style gives.
    const QTextCharFormat oldBase = doc.styleCharFormat(was), newBase = doc.styleCharFormat(styleId);
    QVector<std::tuple<int, int, QTextCharFormat>> edits;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat nfmt = restyled(f.charFormat(), oldBase, newBase);
        if (nfmt != f.charFormat()) edits.push_back({f.position(), f.position() + f.length(), nfmt});
    }
    for (const auto &e : std::as_const(edits)) {
        QTextCursor t(b);
        t.setPosition(std::get<0>(e));
        t.setPosition(std::get<1>(e), QTextCursor::KeepAnchor);
        t.setCharFormat(std::get<2>(e));
    }
    setMarkFormat(b, restyled(markFormat(b), oldBase, newBase));
}

void applyParagraphStyle(WriteDoc &doc, QTextCursor &c, const QString &styleId)
{
    if (!doc.ensureStyle(styleId)) return;
    c.beginEditBlock();
    for (const QTextBlock &b : selectedBlocks(c))
        if (!isLocked(b)) applyStyleToBlock(doc, b, styleId);
    // The text typed next in an empty paragraph has the style's look too.
    if (!c.hasSelection() && c.block().length() <= 1) c.setCharFormat(doc.styleCharFormat(styleId));
    c.endEditBlock();
}

void applyCharacterStyle(WriteDoc &doc, QTextCursor &c, const QString &styleId)
{
    if (!doc.ensureStyle(styleId)) return;
    const QString paraStyle = paragraphStyleAt(doc, c);
    const QTextCharFormat base = doc.styleCharFormat(paraStyle);
    const QTextCharFormat with = doc.styleCharFormat(paraStyle, styleId);
    changeCharFormat(doc, c, [&](QTextCharFormat &f) {
        const QString was = f.stringProperty(tp::CharStyle);
        const QTextCharFormat wasBase = was.isEmpty() ? base : doc.styleCharFormat(paraStyle, was);
        f = restyled(f, wasBase, with);
        f.setProperty(tp::CharStyle, styleId);
    });
}

} // namespace jp::write
