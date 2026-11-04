#include "core/compareops.h"

#include "core/editops.h"
#include "core/reviewops.h"
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

std::unique_ptr<WriteDoc> compareDocuments(WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes, QString *error)
{
    if (changes) *changes = 0;
    QString err;
    std::unique_ptr<WriteDoc> out = WriteDoc::fromDocx(original.toDocx(&err), &err);
    if (!out) {
        if (error) *error = err;
        return nullptr;
    }
    out->setReviewer(options.author.trimmed().isEmpty() ? QStringLiteral("Author") : options.author.trimmed(), QString());
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
    QVector<int> keepA, keepB;
    // (A paragraph that is the same is an anchor; between anchors the paragraphs differ.)
    if (ka.size() * kb.size() <= 40000000) {
        commonRun(ka, kb, &keepA, &keepB);
    }
    out->setTrackChanges(true);
    int made = 0;
    QTextDocument *story = out->story();
    // From the last gap to the first, so that a place found before an edit is still its place after the edits after it.
    for (int k = int(keepA.size()); k >= 0; --k) {
        const int a0 = k == 0 ? 0 : keepA.at(k - 1) + 1, a1 = k < keepA.size() ? keepA.at(k) : int(a.size());
        const int b0 = k == 0 ? 0 : keepB.at(k - 1) + 1, b1 = k < keepB.size() ? keepB.at(k) : int(b.size());
        const int na = a1 - a0, nb = b1 - b0;
        if (na == 0 && nb == 0) continue;
        const int pairs = std::min(na, nb);
        // The paragraphs that are only in the revised version, put after the last paragraph of the gap (or after the one before it).
        if (nb > pairs) {
            QTextCursor c(story);
            const int after = a0 + (na > 0 ? na : 0) - 1;
            if (after >= 0) c.setPosition(a.at(after).blockEnd);
            else c.setPosition(0);
            const bool atStart = after < 0;
            for (int t = b0 + pairs; t < b1; ++t) {
                const QString text = b.at(t).text;
                if (atStart) {
                    if (!text.isEmpty()) insertText(*out, c, text);
                    insertParagraph(*out, c);
                } else {
                    insertParagraph(*out, c);
                    if (!text.isEmpty()) insertText(*out, c, text);
                }
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
                int from = ed.deleteFrom, to = ed.deleteTo;
                int endPlace = to > from ? pa.places.at(to - 1) + 1 : (from < pa.places.size() ? pa.places.at(from) : pa.blockEnd);
                if (to > from) {
                    QTextCursor c(story);
                    c.setPosition(pa.places.at(from));
                    c.setPosition(endPlace, QTextCursor::KeepAnchor);
                    removeSelection(*out, c);
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
    if (changes) *changes = made;
    return out;
}

} // namespace jp::write
