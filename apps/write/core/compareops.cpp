#include "core/compareops.h"

#include "core/editops.h"
#include "core/reviewops.h"
#include "core/styleops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextBlock>
#include <algorithm>

namespace jp::write {

namespace {

constexpr int kMostTokens = 3000;        // the most words of a paragraph that are told apart word by word (the table holds the square of it)
constexpr int kMostParagraphs = 20000;   // the most paragraphs of a text that are compared
constexpr qint64 kMostTableCells = 4000000;   // the most cells of the table of lengths for the paragraphs between two anchors (16 MB)

QStringList tokensOf(const QString &text, const CompareOptions &o)
{
    QStringList out;
    if (o.byCharacter) {
        for (const QChar ch : text) out << QString(ch);
        return out;
    }
    static const QRegularExpression re(QStringLiteral("\\w+|\\s+|[^\\w\\s]"));
    auto it = re.globalMatch(text);
    while (it.hasNext()) out << it.next().captured(0);
    return out;
}

QString keyOf(const QString &token, const CompareOptions &o)
{
    QString k = token;
    if (o.ignoreCase) k = k.toLower();
    if (o.ignoreWhiteSpace && !k.isEmpty() && k.at(0).isSpace()) k = QStringLiteral(" ");
    return k;
}

// The longest common run of two lists, as which of each are kept (a table of lengths, walked back).
void commonRun(const QStringList &a, const QStringList &b, QVector<int> *keepA, QVector<int> *keepB)
{
    const int n = int(a.size()), m = int(b.size());
    QVector<quint32> table(size_t(n + 1) * size_t(m + 1), 0);
    auto at = [&](int i, int j) -> quint32 & { return table[size_t(i) * size_t(m + 1) + size_t(j)]; };
    for (int i = n - 1; i >= 0; --i)
        for (int j = m - 1; j >= 0; --j) at(i, j) = a.at(i) == b.at(j) ? at(i + 1, j + 1) + 1 : std::max(at(i + 1, j), at(i, j + 1));
    int i = 0, j = 0;
    while (i < n && j < m) {
        if (a.at(i) == b.at(j)) {
            keepA->push_back(i++);
            keepB->push_back(j++);
        } else if (at(i + 1, j) >= at(i, j + 1)) {
            ++i;
        } else {
            ++j;
        }
    }
}

} // namespace

QVector<TextEdit> diffText(const QString &from, const QString &to, const CompareOptions &options)
{
    QVector<TextEdit> edits;
    if (from == to) return edits;
    const QStringList ta = tokensOf(from, options), tb = tokensOf(to, options);
    if (ta.size() > kMostTokens || tb.size() > kMostTokens) {
        edits.push_back({0, int(from.size()), to});
        return edits;
    }
    QStringList ka, kb;
    for (const QString &t : ta) ka << keyOf(t, options);
    for (const QString &t : tb) kb << keyOf(t, options);
    QVector<int> keepA, keepB;
    commonRun(ka, kb, &keepA, &keepB);
    // Where each token of `from` begins, in its characters.
    QVector<int> startA(ta.size() + 1, 0);
    for (int i = 0; i < ta.size(); ++i) startA[i + 1] = startA[i] + int(ta.at(i).size());
    int i = 0, j = 0;
    for (int k = 0; k <= keepA.size(); ++k) {
        const int ei = k < keepA.size() ? keepA.at(k) : int(ta.size()), ej = k < keepB.size() ? keepB.at(k) : int(tb.size());
        if (i < ei || j < ej) {
            TextEdit e;
            e.deleteFrom = startA.at(i);
            e.deleteTo = startA.at(ei);
            for (int t = j; t < ej; ++t) e.insertText += tb.at(t);
            if (e.deleteTo > e.deleteFrom || !e.insertText.isEmpty()) {
                // (With no deletion the insert goes at the start of the next kept word.)
                edits.push_back(e);
            }
        }
        i = ei + 1;
        j = ej + 1;
    }
    return edits;
}

namespace {

// The paragraphs of the two texts that are the same, in order, as which of each are kept: the same paragraphs at the start and at the end first, then (when what is between is too large for a table of
// lengths) the paragraphs that are alone in each text and the same, as anchors in their order (patience diff), and in each stretch between the anchors the same again.
void matchParagraphs(const QStringList &ka, const QStringList &kb, int a0, int a1, int b0, int b1, QVector<QPair<int, int>> *out, int depth)
{
    while (a0 < a1 && b0 < b1 && ka.at(a0) == kb.at(b0)) out->push_back({a0++, b0++});
    QVector<QPair<int, int>> tail;
    while (a0 < a1 && b0 < b1 && ka.at(a1 - 1) == kb.at(b1 - 1)) tail.push_back({--a1, --b1});
    if (a0 < a1 && b0 < b1) {
        if (qint64(a1 - a0) * qint64(b1 - b0) <= kMostTableCells) {
            const QStringList sa = ka.mid(a0, a1 - a0), sb = kb.mid(b0, b1 - b0);
            QVector<int> keepA, keepB;
            commonRun(sa, sb, &keepA, &keepB);
            for (int i = 0; i < keepA.size(); ++i) out->push_back({a0 + keepA.at(i), b0 + keepB.at(i)});
        } else if (depth > 0) {
            QHash<QString, int> countA, countB, placeB;
            for (int i = a0; i < a1; ++i) ++countA[ka.at(i)];
            for (int j = b0; j < b1; ++j) {
                ++countB[kb.at(j)];
                placeB.insert(kb.at(j), j);
            }
            // The paragraphs alone in both, in the order of the first text, then the longest run of them that is in order in the second (patience sorting).
            QVector<QPair<int, int>> pairs;
            for (int i = a0; i < a1; ++i)
                if (countA.value(ka.at(i)) == 1 && countB.value(ka.at(i)) == 1) pairs.push_back({i, placeB.value(ka.at(i))});
            QVector<int> tails, prev(pairs.size(), -1);   // (tails: for each length, the pair that ends the best run of it)
            for (int k = 0; k < pairs.size(); ++k) {
                int lo = 0, hi = int(tails.size());
                while (lo < hi) {
                    const int mid = (lo + hi) / 2;
                    if (pairs.at(tails.at(mid)).second < pairs.at(k).second) lo = mid + 1;
                    else hi = mid;
                }
                prev[k] = lo > 0 ? tails.at(lo - 1) : -1;
                if (lo == tails.size()) tails.push_back(k);
                else tails[lo] = k;
            }
            QVector<QPair<int, int>> anchors;
            for (int k = tails.isEmpty() ? -1 : tails.last(); k >= 0; k = prev.at(k)) anchors.prepend(pairs.at(k));
            int pa = a0, pb = b0;
            for (const auto &anchor : std::as_const(anchors)) {
                matchParagraphs(ka, kb, pa, anchor.first, pb, anchor.second, out, depth - 1);
                out->push_back(anchor);
                pa = anchor.first + 1;
                pb = anchor.second + 1;
            }
            matchParagraphs(ka, kb, pa, a1, pb, b1, out, depth - 1);
        }
    }
    for (int i = int(tail.size()) - 1; i >= 0; --i) out->push_back(tail.at(i));
}

struct Para {
    QString text;           // what is compared: the text without the pieces kept as XML
    QVector<int> places;    // for each character of text, its place in the story
    int blockFrom = 0;      // the paragraph's start in the story
    int blockEnd = 0;       // the end of its text (before its mark)
    bool ok = true;         // a paragraph that can be compared (not one that stands for a table or a note)
};

QVector<Para> paragraphsOf(WriteDoc &doc, bool *tooMany)
{
    QVector<Para> out;
    *tooMany = false;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        if (out.size() >= kMostParagraphs) {
            *tooMany = true;
            break;
        }
        Para p;
        p.blockFrom = b.position();
        p.blockEnd = b.position() + b.length() - 1;
        p.ok = !isLocked(b) && !b.blockFormat().hasProperty(prop::EndnoteBlock);
        if (p.ok) {
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                const QTextCharFormat cf = f.charFormat();
                if (isAtom(cf) || cf.hasProperty(prop::TrackedDeleted)) continue;
                const QString text = f.text();
                for (int k = 0; k < text.size(); ++k) {
                    if (text.at(k) == QChar::ObjectReplacementCharacter) continue;
                    p.text += text.at(k);
                    p.places.push_back(f.position() + k);
                }
            }
        }
        out.push_back(p);
    }
    return out;
}

} // namespace

namespace {

// The look of the letters of a piece of the revised text that an inserted piece takes: its emphasis, size, face, and color (what is of the file's own, such as a kept piece of XML, stays out).
void giveLook(QTextCharFormat &f, const QTextCharFormat &from)
{
    for (const int key : {int(QTextFormat::FontWeight), int(QTextFormat::FontItalic), int(QTextFormat::TextUnderlineStyle), int(QTextFormat::FontUnderline), int(QTextFormat::FontStrikeOut),
                          int(QTextFormat::FontPointSize), int(QTextFormat::FontFamilies), int(QTextFormat::ForegroundBrush), int(QTextFormat::FontCapitalization), int(QTextFormat::TextVerticalAlignment),
                          int(tp::ThemeFont), int(tp::ColorRefP), int(tp::HighlightRefP)}) {
        if (from.hasProperty(key)) f.setProperty(key, from.property(key));
        else f.clearProperty(key);
    }
}

// A paragraph put in after the cursor's paragraph: its words, in the look of the revised paragraph's, and the style, list, alignment, and indents of the revised paragraph. The paragraph mark and the words
// are tracked insertions; the look given to them is part of the insertion (not a change of its own).
void insertRevisedParagraph(WriteDoc &out, QTextCursor &c, WriteDoc &revised, const Para &source, bool before)
{
    const QTextBlock srcBlock = revised.story()->findBlock(source.blockFrom);
    auto words = [&] {
        for (QTextBlock::iterator it = srcBlock.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (isAtom(cf) || cf.hasProperty(prop::TrackedDeleted)) continue;
            QString piece = f.text();
            piece.remove(QChar::ObjectReplacementCharacter);
            if (piece.isEmpty()) continue;
            const int from = c.position();
            insertText(out, c, piece);
            const UntrackedEdit untracked;
            QTextCursor range(out.story());
            range.setPosition(from);
            range.setPosition(c.position(), QTextCursor::KeepAnchor);
            changeCharFormat(out, range, [&](QTextCharFormat &g) { giveLook(g, cf); });
        }
    };
    int blockStart = 0;
    if (before) {
        words();
        insertParagraph(out, c);
        blockStart = out.story()->findBlock(c.position()).previous().position();
    } else {
        insertParagraph(out, c);
        blockStart = out.story()->findBlock(c.position()).position();
        words();
    }
    const UntrackedEdit untracked;
    QTextCursor at(out.story());
    at.setPosition(blockStart);
    const QString style = srcBlock.blockFormat().stringProperty(tp::StyleName);
    if (!style.isEmpty() && styleOfDocument(out, style)) applyParagraphStyle(out, at, style);
    const QTextBlockFormat sf = srcBlock.blockFormat();
    changeBlockFormat(out, at, [&](QTextBlockFormat &bf) {
        bf.setAlignment(sf.alignment());
        bf.setLeftMargin(sf.leftMargin());
        bf.setRightMargin(sf.rightMargin());
        bf.setTextIndent(sf.textIndent());
    });
    const ListKind kind = listKindOf(revised, srcBlock);
    if (kind != ListKind::None && listKindOf(out, out.story()->findBlock(blockStart)) == ListKind::None) toggleList(out, at, kind == ListKind::Numbers);
}

} // namespace

std::unique_ptr<WriteDoc> compareDocuments(WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes, QString *error, const std::function<bool(int, int)> &progress)
{
    if (changes) *changes = 0;
    if (error) error->clear();
    QString err;
    std::unique_ptr<WriteDoc> out = WriteDoc::fromDocx(original.toDocx(&err), &err);
    if (!out) {
        if (error) *error = err;
        return nullptr;
    }
    out->setReviewer(options.author.trimmed().isEmpty() ? QStringLiteral("Author") : options.author.trimmed(), QString());
    // (A copy that shows the differences is not a protected document, whatever the original was.)
    out->setProtection(std::nullopt);
    bool many1 = false, many2 = false;
    QVector<Para> a = paragraphsOf(*out, &many1);
    const QVector<Para> b = paragraphsOf(revised, &many2);
    if (many1 || many2) {
        if (error) *error = QCoreApplication::translate("jp::write::Compare", "A document with that many paragraphs can't be compared.");
        return nullptr;
    }
    // The paragraphs that are the same in both, in order.
    auto key = [&](const Para &p) {
        QString k = p.ok ? p.text : QStringLiteral("\x01");
        if (options.ignoreCase) k = k.toLower();
        if (options.ignoreWhiteSpace) k = k.simplified();
        return k;
    };
    QStringList ka, kb;
    for (const Para &p : a) ka << key(p);
    for (const Para &p : b) kb << key(p);
    QVector<QPair<int, int>> matches;
    matchParagraphs(ka, kb, 0, int(ka.size()), 0, int(kb.size()), &matches, 24);
    std::sort(matches.begin(), matches.end());
    QVector<int> keepA, keepB;
    for (const auto &m : std::as_const(matches)) {
        keepA << m.first;
        keepB << m.second;
    }
    // The places where the two differ, to tell the progress of.
    int places = 0;
    for (int k = int(keepA.size()); k >= 0; --k) {
        const int na = (k < keepA.size() ? keepA.at(k) : int(a.size())) - (k == 0 ? 0 : keepA.at(k - 1) + 1), nb = (k < keepB.size() ? keepB.at(k) : int(b.size())) - (k == 0 ? 0 : keepB.at(k - 1) + 1);
        if (na > 0 || nb > 0) ++places;
    }
    out->setTrackChanges(true);
    int made = 0, done = 0;
    QTextDocument *story = out->story();
    // From the last gap to the first, so that a place found before an edit is still its place after the edits after it.
    for (int k = int(keepA.size()); k >= 0; --k) {
        const int a0 = k == 0 ? 0 : keepA.at(k - 1) + 1, a1 = k < keepA.size() ? keepA.at(k) : int(a.size());
        const int b0 = k == 0 ? 0 : keepB.at(k - 1) + 1, b1 = k < keepB.size() ? keepB.at(k) : int(b.size());
        const int na = a1 - a0, nb = b1 - b0;
        if (na == 0 && nb == 0) continue;
        if (progress && !progress(done++, places)) {
            if (error) error->clear();
            return nullptr;
        }
        const int pairs = std::min(na, nb);
        // The paragraphs that are only in the revised version, put after the last paragraph of the gap (or after the one before it).
        if (nb > pairs) {
            QTextCursor c(story);
            const int after = a0 + (na > 0 ? na : 0) - 1;
            if (after >= 0) c.setPosition(a.at(after).blockEnd);
            else c.setPosition(0);
            const bool atStart = after < 0;
            for (int t = b0 + pairs; t < b1; ++t) {
                insertRevisedParagraph(*out, c, revised, b.at(t), atStart);
                ++made;
            }
        }
        // The paragraphs that are only in the original are deleted, with their marks.
        for (int t = a1 - 1; t >= a0 + pairs; --t) {
            if (!a.at(t).ok) continue;
            QTextCursor c(story);
            const QTextBlock blk = story->findBlock(a.at(t).blockFrom);
            c.setPosition(blk.position());
            if (blk.next().isValid()) {
                c.setPosition(blk.next().position(), QTextCursor::KeepAnchor);
            } else if (blk.previous().isValid()) {
                // (The last paragraph: its text, and the mark of the paragraph before it, so that no empty paragraph is left.)
                c.setPosition(blk.previous().position() + blk.previous().length() - 1);
                c.setPosition(blk.position() + blk.length() - 1, QTextCursor::KeepAnchor);
            } else {
                c.setPosition(blk.position() + blk.length() - 1, QTextCursor::KeepAnchor);
            }
            if (c.hasSelection()) {
                removeSelection(*out, c);
                ++made;
            }
        }
        // The paragraphs in both that differ: the words are told apart.
        for (int t = pairs - 1; t >= 0; --t) {
            const Para &pa = a.at(a0 + t);
            const Para &pb = b.at(b0 + t);
            if (!pa.ok || !pb.ok) continue;
            const QVector<TextEdit> edits = diffText(pa.text, pb.text, options);
            for (int e = int(edits.size()) - 1; e >= 0; --e) {
                const TextEdit &ed = edits.at(e);
                const int from = ed.deleteFrom, to = ed.deleteTo;
                int endPlace = to > from ? pa.places.at(to - 1) + 1 : (from < pa.places.size() ? pa.places.at(from) : pa.blockEnd);
                // A range is deleted in pieces that touch, so that a picture or another object between two words that go is left where it is.
                for (int end = to; end > from;) {
                    int begin = end - 1;
                    while (begin > from && pa.places.at(begin - 1) + 1 == pa.places.at(begin)) --begin;
                    QTextCursor c(story);
                    c.setPosition(pa.places.at(begin));
                    c.setPosition(pa.places.at(end - 1) + 1, QTextCursor::KeepAnchor);
                    removeSelection(*out, c);
                    end = begin;
                }
                if (!ed.insertText.isEmpty()) {
                    QTextCursor c(story);
                    c.setPosition(endPlace);
                    insertText(*out, c, ed.insertText);
                }
                ++made;
            }
        }
    }
    if (progress) progress(places, places);
    if (changes) *changes = made;
    return out;
}

} // namespace jp::write
