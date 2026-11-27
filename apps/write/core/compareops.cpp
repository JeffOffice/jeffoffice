#include "core/compareops.h"

#include "core/editops.h"
#include "core/pictureops.h"
#include "core/reviewops.h"
#include "core/styleops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "review/textdiff.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QCoreApplication>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <climits>

namespace jp::write {

namespace {

using textdiff::commonRun;

constexpr int kMostParagraphs = 20000;   // the most paragraphs of a text that are compared
constexpr qint64 kMostTableCells = 4000000;   // the most cells of the table of lengths for the paragraphs between two anchors (16 MB)

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
    enum Kind { Normal, Cell, Endnote, Raw } kind = Normal;   // (what a paragraph that can't be compared is)
    struct Run {
        int from = 0;               // the first character of text that the run has
        QString key;                // its look, in a few words (lookKey)
        QTextCharFormat format;
    };
    QVector<Run> runs;      // the pieces of text of one look, in order (when the looks are compared)
};

// What of a letters' look is compared, as a few words: its weight, slant, underline, strike, size, face, colors, highlight, capitals, and raising.
QString lookKey(const QTextCharFormat &f)
{
    auto brush = [](const QBrush &b) { return b.style() == Qt::NoBrush ? QString() : b.color().name(QColor::HexArgb); };
    QStringList k;
    k << QString::number(f.fontWeight()) << QString::number(f.fontItalic() ? 1 : 0) << QString::number(int(f.underlineStyle())) + QLatin1Char('/') + QString::number(f.intProperty(tp::UnderlineKind))
      << QString::number(f.fontStrikeOut() ? 1 : 0) << (f.hasProperty(QTextFormat::FontPointSize) ? QString::number(f.fontPointSize(), 'f', 2) : QString())
      << f.property(QTextFormat::FontFamilies).toStringList().join(QLatin1Char(',')) << f.stringProperty(tp::ThemeFont) << brush(f.foreground()) << f.stringProperty(tp::ColorRefP)
      << f.stringProperty(tp::HighlightRefP) << QString::number(int(f.fontCapitalization())) << QString::number(int(f.verticalAlignment()));
    return k.join(QLatin1Char('|'));
}

// The tables of a story, for telling which cells can be compared: each table (a top-level one, by order) is its shape written as a few letters (a "p" for each paragraph, "," between cells, "|" between
// rows; a table that holds another table is never the same as another), and each paragraph says which table it is in (-1: none).
struct TableShapes {
    QStringList shapes;
    QVector<int> tableOfBlock;
};
TableShapes tableShapesOf(WriteDoc &doc, const QTextDocument *story, QChar side)
{
    TableShapes out;
    QHash<const void *, int> ordinals;
    int lastKey = 0;
    const void *lastRow = nullptr;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        int key = cellKeyOf(b);
        const flow::CellRecord *rec = key ? doc.flow().tables.get(key) : nullptr;
        if (!rec) {
            out.tableOfBlock.push_back(-1);
            lastKey = 0;
            lastRow = nullptr;
            continue;
        }
        bool nested = false;
        const flow::CellRecord *root = rec;
        for (int guard = 0; root->parent && guard < 64; ++guard) {
            const flow::CellRecord *up = doc.flow().tables.get(root->parent);
            if (!up) break;
            root = up;
            nested = true;
        }
        const void *table = root->table.get();
        if (!ordinals.contains(table)) {
            ordinals.insert(table, int(out.shapes.size()));
            out.shapes << QString();
        }
        const int ordinal = ordinals.value(table);
        out.tableOfBlock.push_back(ordinal);
        QString &shape = out.shapes[ordinal];
        if (nested) {
            if (!shape.startsWith(QLatin1Char('!'))) shape = QLatin1Char('!') + side + shape;   // (never the same as the other side's)
            continue;
        }
        if (rec->row.get() != lastRow) shape += QLatin1Char('|');
        else if (key != lastKey) shape += QLatin1Char(',');
        lastRow = rec->row.get();
        lastKey = key;
        shape += QLatin1Char('p');
    }
    return out;
}

QVector<Para> paragraphsOf(QTextDocument *story, const QVector<char> *comparableTable, const TableShapes &tables, bool *tooMany, bool withLooks, CompareContext &ctx, bool *looksCut)
{
    QVector<Para> out;
    *tooMany = false;
    int blockNumber = -1;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        ++blockNumber;
        if (out.size() >= kMostParagraphs) {
            *tooMany = true;
            break;
        }
        Para p;
        p.blockFrom = b.position();
        p.blockEnd = b.position() + b.length() - 1;
        p.ok = !isLocked(b) && !b.blockFormat().hasProperty(prop::EndnoteBlock);
        if (!p.ok) p.kind = b.blockFormat().hasProperty(prop::EndnoteBlock) ? Para::Endnote : Para::Raw;
        const int table = blockNumber < tables.tableOfBlock.size() ? tables.tableOfBlock.at(blockNumber) : -1;
        if (table >= 0 && (!comparableTable || table >= comparableTable->size() || !comparableTable->at(table))) p.ok = false;   // (a cell of a table that is not the same shape in both)
        if (!p.ok && p.kind == Para::Normal) p.kind = Para::Cell;
        if (p.ok) {
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                const QTextCharFormat cf = f.charFormat();
                if (isAtom(cf) || cf.hasProperty(prop::TrackedDeleted)) continue;
                const QString text = f.text();
                const int before = int(p.text.size());
                for (int k = 0; k < text.size(); ++k) {
                    if (text.at(k) == QChar::ObjectReplacementCharacter) continue;
                    p.text += text.at(k);
                    p.places.push_back(f.position() + k);
                }
                if (withLooks && p.text.size() > before) {
                    if (ctx.work.formatFragments >= ctx.limits.formatFragments) {
                        *looksCut = true;   // (the looks of the rest are not held)
                        withLooks = false;
                        p.runs.clear();
                    } else {
                        ++ctx.work.formatFragments;
                        p.runs.push_back({before, lookKey(cf), cf});
                    }
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
int insertRevisedParagraph(WriteDoc &out, QTextDocument *ostory, QTextCursor &c, WriteDoc &revised, QTextDocument *rstory, const Para &source, bool before)
{
    const QTextBlock srcBlock = rstory->findBlock(source.blockFrom);
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
            QTextCursor range(ostory);
            range.setPosition(from);
            range.setPosition(c.position(), QTextCursor::KeepAnchor);
            changeCharFormat(out, range, [&](QTextCharFormat &g) { giveLook(g, cf); });
        }
    };
    int blockStart = 0;
    if (before) {
        words();
        insertParagraph(out, c);
        blockStart = ostory->findBlock(c.position()).previous().position();
    } else {
        insertParagraph(out, c);
        blockStart = ostory->findBlock(c.position()).position();
        words();
    }
    const UntrackedEdit untracked;
    QTextCursor at(ostory);
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
    if (kind != ListKind::None && listKindOf(out, ostory->findBlock(blockStart)) == ListKind::None) toggleList(out, at, kind == ListKind::Numbers);
    return blockStart;
}

// ---- moves

// The Id a start tag gives.
QString idOfTag(const QByteArray &tag)
{
    const int at = tag.indexOf("w:id=\"");
    if (at < 0) return QString();
    const int end = tag.indexOf('"', at + 6);
    return end < 0 ? QString() : QString::fromLatin1(tag.mid(at + 6, end - at - 6));
}

// Which paragraphs only in the first text are the same (or nearly) as paragraphs only in the second, in another place: as (first, second). `gapA` and `gapB` say which stretch between the kept paragraphs each is in; a
// paragraph that moved is in two different ones. Only paragraphs of at least `options.minMoveWords` words are looked at. The same words are found by hash; the rest by how many words they share (at least 90%),
// within the limits (`skipped` says they were reached, and no move is then looked for, or the looking stopped). Of the paragraphs that share enough words, the words must also be mostly in the same order (a
// longest run of the same words, in order, of at least 90%).
QVector<QPair<int, int>> findMoves(const QVector<Para> &a, const QVector<Para> &b, const QStringList &ka, const QStringList &kb, const QVector<int> &gapA, const QVector<int> &gapB,
                                   const QVector<char> &matchedA, const QVector<char> &matchedB, const CompareOptions &options, CompareContext &ctx, bool *skipped)
{
    QVector<QPair<int, int>> found;
    const CompareLimits &limit = ctx.limits;
    CompareWork &work = ctx.work;
    qint64 chars = 0;
    for (int i = 0; i < a.size(); ++i)
        if (!matchedA.at(i) && a.at(i).ok) chars += a.at(i).text.size();
    for (int j = 0; j < b.size(); ++j)
        if (!matchedB.at(j) && b.at(j).ok) chars += b.at(j).text.size();
    if (chars > limit.moveBytes) {
        *skipped = true;
        return found;
    }
    static const QRegularExpression space(QStringLiteral("\\s+"));
    struct Cand {
        int index;
        QVector<uint> words;   // the hashes of its words, in order of value
        QVector<uint> order;   // the same, in the order of the paragraph
    };
    auto candidates = [&](const QVector<Para> &paras, const QStringList &keys, const QVector<char> &matched) {
        QVector<Cand> out;
        for (int i = 0; i < paras.size(); ++i) {
            if (matched.at(i) || !paras.at(i).ok || (&paras == &a && i == 0)) continue;   // (the first paragraph has none before it to take the mark of)
            const QStringList words = keys.at(i).split(space, Qt::SkipEmptyParts);
            if (words.size() < options.minMoveWords) continue;
            Cand c;
            c.index = i;
            for (const QString &w : words) c.order.push_back(qHash(w));
            c.words = c.order;
            std::sort(c.words.begin(), c.words.end());
            out.push_back(std::move(c));
        }
        return out;
    };
    const QVector<Cand> da = candidates(a, ka, matchedA);
    const QVector<Cand> ib = candidates(b, kb, matchedB);
    if (da.size() > limit.moveParagraphs || ib.size() > limit.moveParagraphs) {
        *skipped = true;
        return found;
    }
    QVector<char> usedA(da.size(), 0), usedB(ib.size(), 0);
    // The same words.
    QHash<QString, QVector<int>> byKey;
    for (int j = 0; j < ib.size(); ++j) byKey[kb.at(ib.at(j).index)].push_back(j);
    for (int i = 0; i < da.size(); ++i) {
        const QVector<int> same = byKey.value(ka.at(da.at(i).index));
        for (const int j : same) {
            if (usedB.at(j) || gapB.at(ib.at(j).index) == gapA.at(da.at(i).index)) continue;
            usedA[i] = usedB[j] = 1;
            found.push_back({da.at(i).index, ib.at(j).index});
            break;
        }
    }
    // Nearly the same words: the words the two share, as at least 90% of the words of both.
    for (int i = 0; i < da.size(); ++i) {
        if (usedA.at(i)) continue;
        const QVector<uint> &wa = da.at(i).words;
        int best = -1;
        double bestSimilarity = 0;
        for (int j = 0; j < ib.size(); ++j) {
            if (usedB.at(j) || gapB.at(ib.at(j).index) == gapA.at(da.at(i).index)) continue;
            const QVector<uint> &wb = ib.at(j).words;
            const qint64 total = qint64(wa.size()) + wb.size();
            if (qint64(std::min(wa.size(), wb.size())) * 20 < total * 9) continue;   // (the one that is shorter can't share 90%)
            if (work.moveWords + total > limit.moveWork) {
                *skipped = true;
                i = int(da.size());
                break;
            }
            work.moveWords += total;
            ++work.moveComparisons;
            qint64 shared = 0;
            for (int x = 0, y = 0; x < wa.size() && y < wb.size();) {
                if (wa.at(x) == wb.at(y)) {
                    ++shared;
                    ++x;
                    ++y;
                } else if (wa.at(x) < wb.at(y)) {
                    ++x;
                } else {
                    ++y;
                }
            }
            if (shared * 20 < total * 9) continue;
            // (Only now, for the pair that passed: the words in order, with what the two start and end with in common left out.)
            const QVector<uint> &oa = da.at(i).order, &ob = ib.at(j).order;
            int lo = 0, ea = int(oa.size()), eb = int(ob.size());
            while (lo < ea && lo < eb && oa.at(lo) == ob.at(lo)) ++lo;
            while (ea > lo && eb > lo && oa.at(ea - 1) == ob.at(eb - 1)) {
                --ea;
                --eb;
            }
            const qint64 cells = qint64(ea - lo + 1) * qint64(eb - lo + 1);
            if (work.moveCells + cells > limit.moveCells) {
                *skipped = true;
                i = int(da.size());
                break;
            }
            work.moveCells += cells;
            QVector<int> row(eb - lo + 1, 0);
            for (int x = lo; x < ea; ++x) {
                int diagonal = 0;
                for (int y = lo; y < eb; ++y) {
                    const int up = row.at(y - lo + 1);
                    row[y - lo + 1] = oa.at(x) == ob.at(y) ? diagonal + 1 : std::max(up, row.at(y - lo));
                    diagonal = up;
                }
            }
            const qint64 ordered = qint64(lo) + (int(oa.size()) - ea) + row.at(eb - lo);
            if (ordered * 20 < total * 9) continue;
            const double similarity = double(ordered) / double(total);
            if (similarity > bestSimilarity) {
                bestSimilarity = similarity;
                best = j;
            }
        }
        if (best >= 0) {
            usedA[i] = usedB[best] = 1;
            found.push_back({da.at(i).index, ib.at(best).index});
        }
    }
    return found;
}

// Makes the tracked deletion (or insertion) that the reviewer just made of the whole of the paragraph at `blockPos` into one half of the move `name`: its marks become w:moveFrom (w:moveTo), the paragraph's mark too, and
// the two marks of the range of the move are put round its text. `markPos` is the start of the paragraph whose mark was made deleted (inserted) with the text: the paragraph's own, or the one before it.
void makeMove(WriteDoc &out, int blockPos, int markPos, const QString &name, bool from)
{
    const UntrackedEdit untracked;
    QTextDocument *story = out.story();
    QTextBlock blk = story->findBlock(blockPos);
    const QString oldTag = from ? QStringLiteral("del") : QStringLiteral("ins"), newTag = from ? QStringLiteral("moveFrom") : QStringLiteral("moveTo");
    QSet<QString> mine;
    for (QTextBlock::iterator it = blk.begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        if (!cf.hasProperty(prop::Atom)) continue;
        const office::wml::Inline a = unpackAtom(storedBytes(cf, prop::Atom));
        if (a.kind == office::wml::Inline::ParaAtom && a.role == office::wml::Inline::Open && a.tag == oldTag && revisionAuthorOf(a.raw) == out.reviewer()) mine.insert(a.key);
    }
    struct Change {
        int from, to;
        QTextCharFormat format;
    };
    QVector<Change> changes;
    QTextCharFormat like;
    for (QTextBlock::iterator it = blk.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        QTextCharFormat cf = f.charFormat();
        if (!cf.hasProperty(prop::Atom)) continue;
        office::wml::Inline a = unpackAtom(storedBytes(cf, prop::Atom));
        if (a.kind != office::wml::Inline::ParaAtom || a.tag != oldTag || a.role == office::wml::Inline::Single || !mine.contains(a.key)) continue;
        a.tag = newTag;
        if (a.role == office::wml::Inline::Open) a.raw.replace("<w:" + oldTag.toLatin1(), "<w:" + newTag.toLatin1());
        else a.raw = "</w:" + newTag.toLatin1() + ">";
        setStored(cf, prop::Atom, packAtom(a));
        if (like == QTextCharFormat()) like = cf;
        changes.push_back({f.position(), f.position() + int(f.length()), cf});
    }
    if (changes.isEmpty()) return;
    QTextCursor t(story);
    for (const Change &c : std::as_const(changes)) {
        t.setPosition(c.from);
        t.setPosition(c.to, QTextCursor::KeepAnchor);
        t.setCharFormat(c.format);
    }
    // The mark of the paragraph, where it was made deleted (or inserted) too.
    const QTextBlock markBlock = story->findBlock(markPos);
    if (markBlock.isValid())
        for (const Revision &r : revisionsIn(story, markBlock.position() + markBlock.length() - 1, markBlock.position() + markBlock.length()))
            if (r.block == markBlock.blockNumber() && r.author == out.reviewer() && (from ? r.kind == Revision::MarkDeleted : r.kind == Revision::MarkInserted)) {
                markParagraphMark(out, markBlock, newTag.toLatin1().constData());
                break;
            }
    // The marks of the range that name the move.
    const QString range = from ? QStringLiteral("moveFromRange") : QStringLiteral("moveToRange");
    // (The end of the range has the Id of its start.)
    QByteArray startRaw = revisionStartTag(out, range + QStringLiteral("Start"));
    startRaw.chop(1);
    startRaw += " w:name=\"" + name.toLatin1() + "\"/>";
    const QString rangeId = idOfTag(startRaw);
    auto rangeMark = [&](const QString &tag, const QByteArray &raw) {
        office::wml::Inline a;
        a.kind = office::wml::Inline::ParaAtom;
        a.role = office::wml::Inline::Single;
        a.tag = tag;
        a.key = rangeId;
        a.raw = raw;
        QTextCharFormat f = like;
        setStored(f, prop::Atom, packAtom(a));
        return f;
    };
    blk = story->findBlock(blockPos);
    t.setPosition(blk.position() + blk.length() - 1);
    t.insertText(QString(QChar::ObjectReplacementCharacter), rangeMark(range + QStringLiteral("End"), "<w:" + (range + QStringLiteral("End")).toLatin1() + " w:id=\"" + rangeId.toLatin1() + "\"/>"));
    t.setPosition(blk.position());
    t.insertText(QString(QChar::ObjectReplacementCharacter), rangeMark(range + QStringLiteral("Start"), startRaw));
    refreshRevisionLook(out, story->findBlock(blockPos));
}

// In the text put in by a move (the paragraph at `blockPos`, the revised one's words), what differs from the paragraph it was moved from: the words that are not in the first are put in as insertions inside the
// move, and the words of the first that are not in the second as deletions beside them. `edits` turn `fromText` into the moved text. What is accepted is the revised text, and rejecting the move takes all of it away.
void markMovedEdits(WriteDoc &out, int blockPos, const QString &fromText, const QVector<TextEdit> &edits)
{
    if (edits.isEmpty()) return;
    const UntrackedEdit untracked;
    QTextDocument *story = out.story();
    QVector<int> places;   // for each character of the moved text, its place (what the marks of the changes are is left out)
    for (QTextBlock::iterator it = story->findBlock(blockPos).begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (f.charFormat().hasProperty(prop::Atom)) continue;
        const QString text = f.text();
        for (int k = 0; k < text.size(); ++k)
            if (text.at(k) != QChar::ObjectReplacementCharacter) places.push_back(f.position() + k);
    }
    QVector<int> starts;   // where each edit's inserted words begin, in the moved text
    int delta = 0;
    for (const TextEdit &ed : edits) {
        starts.push_back(ed.deleteFrom + delta);
        delta += int(ed.insertText.size()) - (ed.deleteTo - ed.deleteFrom);
    }
    auto formatAt = [&](int place) {
        QTextCursor q(story);
        q.setPosition(place + 1);
        return q.charFormat();
    };
    for (int e = int(edits.size()) - 1; e >= 0; --e) {
        const TextEdit &ed = edits.at(e);
        const int b0 = starts.at(e), n = int(ed.insertText.size());
        if (b0 < 0 || b0 + n > places.size() || places.isEmpty()) continue;
        // The inserted words, in pieces that touch (so that no piece starts in one of the move's marks and ends in another).
        for (int end = b0 + n; end > b0;) {
            int begin = end - 1;
            while (begin > b0 && places.at(begin - 1) + 1 == places.at(begin)) --begin;
            const QByteArray tag = revisionStartTag(out, QStringLiteral("ins"));
            const QTextCharFormat like = formatAt(places.at(begin));
            QTextCursor t(story);
            t.setPosition(places.at(end - 1) + 1);
            t.insertText(QString(QChar::ObjectReplacementCharacter), revisionPiece(tag, QStringLiteral("ins"), false, like));
            t.setPosition(places.at(begin));
            t.insertText(QString(QChar::ObjectReplacementCharacter), revisionPiece(tag, QStringLiteral("ins"), true, like));
            end = begin;
        }
        if (ed.deleteTo > ed.deleteFrom) {
            const int at = b0 > 0 ? places.at(b0 - 1) + 1 : places.first();
            const QTextCharFormat like = formatAt(places.at(std::min(b0, int(places.size()) - 1)));
            QTextCursor t(story);
            t.setPosition(at);
            const QString gone = fromText.mid(ed.deleteFrom, ed.deleteTo - ed.deleteFrom);
            t.insertText(gone, like);
            markDeleted(out, story, at, at + int(gone.size()));
        }
    }
    refreshRevisionLook(out, story->findBlock(blockPos));
}

// ---- looks

struct Segment {
    int from, to, length;   // the same text: [from, from + length) of the first paragraph's text is [to, to + length) of the second's
};

// The stretches of the two paragraphs' texts that are the same, given what differs. A stretch that is not the same letter for letter (what the options ignore) is left out.
QVector<Segment> sameStretches(const Para &pa, const Para &pb, const QVector<TextEdit> &edits, bool ignoreCase)
{
    QVector<Segment> out;
    const Qt::CaseSensitivity cs = ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive;
    auto add = [&](int from, int to, int length) {
        if (length > 0 && from + length <= pa.text.size() && to + length <= pb.text.size() && QStringView(pa.text).mid(from, length).compare(QStringView(pb.text).mid(to, length), cs) == 0) out.push_back({from, to, length});
    };
    int from = 0, to = 0;
    for (const TextEdit &e : edits) {
        add(from, to, e.deleteFrom - from);
        to += e.deleteFrom - from + int(e.insertText.size());
        from = e.deleteTo;
    }
    add(from, to, int(pa.text.size()) - from);
    return out;
}

int runIndexAt(const QVector<Para::Run> &runs, int index)
{
    int lo = 0, hi = int(runs.size());
    while (hi - lo > 1) {
        const int mid = (lo + hi) / 2;
        if (runs.at(mid).from <= index) lo = mid;
        else hi = mid;
    }
    return lo;
}

// Records the changes of the look of the letters of text that is the same in both: the old look is kept as a change of properties and the new look is the revised one's.
int compareRunLooks(WriteDoc &out, QTextDocument *ostory, const Para &pa, const Para &pb, const QVector<Segment> &same, bool *capped, CompareContext &ctx)
{
    int made = 0;
    CompareWork &work = ctx.work;
    for (const Segment &s : same) {
        for (int i = s.from; i < s.from + s.length;) {
            const int ra = runIndexAt(pa.runs, i), rb = runIndexAt(pb.runs, s.to + i - s.from);
            if (ra >= pa.runs.size() || rb >= pb.runs.size()) break;
            const int endA = ra + 1 < pa.runs.size() ? pa.runs.at(ra + 1).from : int(pa.text.size()), endB = rb + 1 < pb.runs.size() ? pb.runs.at(rb + 1).from : int(pb.text.size());
            const int span = std::min({endA - i, endB - (s.to + i - s.from), s.from + s.length - i});
            if (span <= 0) break;
            if (pa.runs.at(ra).key != pb.runs.at(rb).key) {
                // (In pieces whose places touch, so that a picture between two words is not given a look.)
                for (int k = i; k < i + span;) {
                    int e = k + 1;
                    while (e < i + span && pa.places.at(e - 1) + 1 == pa.places.at(e)) ++e;
                    if (work.formatChanges >= ctx.limits.formatChanges) {
                        *capped = true;
                        return made;
                    }
                    QTextCursor c(ostory);
                    c.setPosition(pa.places.at(k));
                    c.setPosition(pa.places.at(e - 1) + 1, QTextCursor::KeepAnchor);
                    const QTextCharFormat look = pb.runs.at(rb).format;
                    changeCharFormat(out, c, [&](QTextCharFormat &g) { giveLook(g, look); });
                    // (Only a change that was made is counted: the edit can be refused, or leave the look as it was.)
                    QTextCursor after(ostory);
                    after.setPosition(pa.places.at(k) + 1);
                    if (after.charFormat().boolProperty(tp::RevisionFormat)) {
                        ++work.formatChanges;
                        ++made;
                    }
                    k = e;
                }
            }
            i += span;
        }
    }
    return made;
}

bool paragraphLooksDiffer(const QTextBlockFormat &x, const QString &xStyle, const QTextBlockFormat &y, const QString &yStyle)
{
    auto near = [](qreal p, qreal q) { return qAbs(p - q) < 0.01; };
    return xStyle != yStyle || x.alignment() != y.alignment() || !near(x.leftMargin(), y.leftMargin()) || !near(x.rightMargin(), y.rightMargin()) || !near(x.textIndent(), y.textIndent()) ||
           !near(x.topMargin(), y.topMargin()) || !near(x.bottomMargin(), y.bottomMargin()) || x.lineHeightType() != y.lineHeightType() || !near(x.lineHeight(), y.lineHeight());
}

// Records a change of the paragraph's style, alignment, indents, or spacing (the old ones are kept) and gives it the revised paragraph's (with `apply` false, only tells whether they differ).
int compareParagraphLooks(WriteDoc &out, QTextDocument *ostory, int blockPos, WriteDoc &revised, QTextDocument *rstory, int revisedBlockPos, bool apply = true)
{
    QTextBlock blk = ostory->findBlock(blockPos);
    const QTextBlock src = rstory->findBlock(revisedBlockPos);
    if (!blk.isValid() || !src.isValid() || blk.blockFormat().hasProperty(prop::RawBlock) || src.blockFormat().hasProperty(prop::RawBlock)) return 0;
    auto styleOf = [](WriteDoc &d, const QTextBlock &b) {
        const QString s = b.blockFormat().stringProperty(tp::StyleName);
        return s.isEmpty() ? d.defaultParagraphStyle() : s;
    };
    const QString ours = styleOf(out, blk), theirs = styleOf(revised, src);
    const QTextBlockFormat sf = src.blockFormat();
    if (!paragraphLooksDiffer(blk.blockFormat(), ours, sf, theirs)) return 0;
    if (!apply) return 1;
    const QTextBlockFormat was = blk.blockFormat();
    {
        const UntrackedEdit untracked;
        QTextCursor at(blk);
        if (ours != theirs && styleOfDocument(out, theirs)) applyParagraphStyle(out, at, theirs);
        changeBlockFormat(out, at, [&](QTextBlockFormat &bf) {
            bf.setAlignment(sf.alignment());
            bf.setLeftMargin(sf.leftMargin());
            bf.setRightMargin(sf.rightMargin());
            bf.setTextIndent(sf.textIndent());
            bf.setTopMargin(sf.topMargin());
            bf.setBottomMargin(sf.bottomMargin());
            bf.setLineHeight(sf.lineHeight(), sf.lineHeightType());
        });
    }
    blk = ostory->findBlock(blockPos);
    const QTextBlockFormat now = blk.blockFormat();
    if (now == was) return 0;
    QTextCursor(blk).setBlockFormat(withParagraphRecord(out, was, now));
    return 1;
}

} // namespace

namespace {

enum class Outcome { Done, TooMany, Canceled };

void addNote(QStringList *notes, const QString &text)
{
    if (!notes->contains(text)) notes->push_back(text);
}

// One story of the copy against its partner in the revised document, with the limits and the counts of `context`. The body is the one that looks for moves.
Outcome compareStory(CompareContext &context, WriteDoc *out, QTextDocument *story, WriteDoc &revised, QTextDocument *rstory, const CompareOptions &options, bool isBody, int *madeTotal, int *doneTotal, int storiesLeft,
                     QStringList *notes, const std::function<bool(int, int)> &progress)
{
    const TableShapes tablesA = tableShapesOf(*out, story, QLatin1Char('a')), tablesB = tableShapesOf(revised, rstory, QLatin1Char('b'));
    QVector<char> comparable(int(std::max(tablesA.shapes.size(), tablesB.shapes.size())), 0);
    for (int k = 0; k < comparable.size(); ++k) {
        comparable[k] = k < tablesA.shapes.size() && k < tablesB.shapes.size() && tablesA.shapes.at(k) == tablesB.shapes.at(k) && !tablesA.shapes.at(k).startsWith(QLatin1Char('!'));
        if (!comparable.at(k)) ++context.work.notMarked;   // (a table that is not the same shape in both, or is in one only: not looked into, and not flattened)
    }
    bool many1 = false, many2 = false;
    bool looksCut = false;
    QVector<Para> a = paragraphsOf(story, &comparable, tablesA, &many1, options.compareFormatting, context, &looksCut);
    const QVector<Para> b = paragraphsOf(rstory, &comparable, tablesB, &many2, options.compareFormatting, context, &looksCut);
    if (many1 || many2) return Outcome::TooMany;
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
    QVector<char> matchedA(a.size(), 0), matchedB(b.size(), 0);
    for (const auto &m : std::as_const(matches)) {
        keepA << m.first;
        keepB << m.second;
        matchedA[m.first] = matchedB[m.second] = 1;
    }
    // The stretches between the kept paragraphs, and which of them each paragraph that is not kept is in.
    struct Gap {
        int a0, a1, b0, b1;
        QVector<int> aOnly, bOnly;      // the paragraphs of each that are not kept and are not moved
        QVector<int> pairs;             // the pairs of them that are told apart word by word, as indices in `pairList`
    };
    QVector<Gap> gaps;
    QVector<int> gapA(a.size(), 0), gapB(b.size(), 0);
    for (int k = 0; k <= keepA.size(); ++k) {
        Gap g;
        g.a0 = k == 0 ? 0 : keepA.at(k - 1) + 1;
        g.a1 = k < keepA.size() ? keepA.at(k) : int(a.size());
        g.b0 = k == 0 ? 0 : keepB.at(k - 1) + 1;
        g.b1 = k < keepB.size() ? keepB.at(k) : int(b.size());
        for (int i = g.a0; i < g.a1; ++i) gapA[i] = k;
        for (int j = g.b0; j < g.b1; ++j) gapB[j] = k;
        gaps.push_back(g);
    }
    // Paragraphs that only moved.
    QVector<int> movedA(a.size(), -1), movedB(b.size(), -1);
    QStringList moveNames;
    QStringList moveFromTexts;
    QVector<QVector<TextEdit>> moveEdits;   // for each move, the words that differ between its two halves (letter for letter, whatever the options ignore)
    if (options.detectMoves && isBody) {
        bool skipped = false;
        const QVector<QPair<int, int>> moves = findMoves(a, b, ka, kb, gapA, gapB, matchedA, matchedB, options, context, &skipped);
        for (const auto &m : moves) {
            movedA[m.first] = movedB[m.second] = int(moveNames.size());
            moveNames << QStringLiteral("move%1").arg(out->nextRevisionId());
            QVector<TextEdit> edits;
            const Para &pa = a.at(m.first), &pb = b.at(m.second);
            if (pa.text != pb.text) {
                textdiff::DiffOptions exact;
                exact.byCharacter = options.byCharacter;
                const qint64 cells = textdiff::diffCells(pa.text, pb.text, exact);
                if (context.work.diffCells + cells > context.limits.diffCells) {
                    edits.push_back({0, int(pa.text.size()), pb.text});
                } else {
                    context.work.diffCells += cells;
                    edits = diffText(pa.text, pb.text, exact);
                }
            }
            moveEdits.push_back(edits);
            moveFromTexts << pa.text;
        }
        context.work.moves = moves.size();
        if (skipped) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many changes to look for all the moved text."));
    }
    for (Gap &g : gaps) {
        for (int i = g.a0; i < g.a1; ++i)
            if (movedA.at(i) < 0) g.aOnly << i;
        for (int j = g.b0; j < g.b1; ++j)
            if (movedB.at(j) < 0) g.bOnly << j;
    }
    // The pairs of paragraphs in both that are told apart: the words that differ (a table of lengths each, within what is allowed for all), and what is the same in both.
    struct PairInfo {
        int a, b;
        QVector<TextEdit> edits;
    };
    QVector<PairInfo> pairList;
    bool wholeReplaced = false;
    const CompareLimits &limit = context.limits;
    for (int k = 0; k < gaps.size(); ++k) {
        Gap &g = gaps[k];
        const int pairs = int(std::min(g.aOnly.size(), g.bOnly.size()));
        for (int t = 0; t < pairs; ++t) {
            PairInfo p{g.aOnly.at(t), g.bOnly.at(t), {}};
            const Para &pa = a.at(p.a), &pb = b.at(p.b);
            if (pa.ok && pb.ok) {
                const qint64 cells = textdiff::diffCells(pa.text, pb.text, options);
                if (context.work.diffCells + cells > limit.diffCells) {
                    p.edits.push_back({0, int(pa.text.size()), pb.text});
                    wholeReplaced = pa.text != pb.text;
                } else {
                    context.work.diffCells += cells;
                    p.edits = diffText(pa.text, pb.text, options);
                }
            }
            g.pairs << int(pairList.size());
            pairList.push_back(p);
        }
    }
    if (wholeReplaced) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many changes to compare every paragraph word by word, so some are shown as replaced whole."));
    // The places where the two differ, to tell the progress of.
    int places = 0;
    for (const Gap &g : std::as_const(gaps))
        if (g.a1 > g.a0 || g.b1 > g.b0) ++places;
    int &made = *madeTotal;
    // The looks: they change no length, so every place in the paragraphs stays where it is for the changes of the words that follow.
    if (options.compareFormatting) {
        bool capped = false;
        auto looks = [&](int ai, int bi, const QVector<TextEdit> &edits) {
            const Para &pa = a.at(ai), &pb = b.at(bi);
            if (!pa.ok || !pb.ok || capped) return;
            made += compareRunLooks(*out, story, pa, pb, sameStretches(pa, pb, edits, options.ignoreCase), &capped, context);
            if (capped) return;
            if (context.work.formatChanges >= context.limits.formatChanges) {
                // (The paragraph's own settings are counted as the looks of letters are.)
                if (compareParagraphLooks(*out, story, pa.blockFrom, revised, rstory, pb.blockFrom, false) > 0) capped = true;
                return;
            }
            const int changed = compareParagraphLooks(*out, story, pa.blockFrom, revised, rstory, pb.blockFrom);
            context.work.formatChanges += changed;
            made += changed;
        };
        for (int k = 0; k < keepA.size(); ++k) looks(keepA.at(k), keepB.at(k), {});
        for (const PairInfo &p : std::as_const(pairList)) looks(p.a, p.b, p.edits);
        if (capped || looksCut) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many changes of formatting to show them all."));
    }
    // From the last gap to the first, so that a place found before an edit is still its place after the edits after it.
    for (int k = int(gaps.size()) - 1; k >= 0; --k) {
        const Gap &g = gaps.at(k);
        if (g.a1 == g.a0 && g.b1 == g.b0) continue;
        if (progress) {
            const int d = (*doneTotal)++;
            if (!progress(d, d + places + storiesLeft)) return Outcome::Canceled;
        }
        const int pairs = int(std::min(g.aOnly.size(), g.bOnly.size()));
        // The paragraphs that are only in the revised version (or are moved here), put after the last paragraph of the gap (or after the one before it).
        QVector<int> inserts = g.bOnly.mid(pairs);
        for (int j = g.b0; j < g.b1; ++j)
            if (movedB.at(j) >= 0) inserts << j;
        std::sort(inserts.begin(), inserts.end());
        if (!inserts.isEmpty()) {
            QTextCursor c(story);
            const int after = g.a1 - 1;
            if (after >= 0) c.setPosition(a.at(after).blockEnd);
            else c.setPosition(0);
            const bool atStart = after < 0;
            for (const int t : std::as_const(inserts)) {
                if (!b.at(t).ok) {
                    if (b.at(t).kind == Para::Raw) ++context.work.notMarked;   // (a paragraph that stands for something kept as it was written)
                    continue;
                }
                const int at = insertRevisedParagraph(*out, story, c, revised, rstory, b.at(t), atStart);
                if (movedB.at(t) >= 0) {
                    makeMove(*out, at, atStart ? at : story->findBlock(at).previous().position(), moveNames.at(movedB.at(t)), false);
                    markMovedEdits(*out, at, moveFromTexts.at(movedB.at(t)), moveEdits.at(movedB.at(t)));
                }
                ++made;
            }
        }
        // The paragraphs that are only in the original (or are moved away) are deleted, with their marks, and the paragraphs in both that differ have the words told apart. All from the last paragraph of the original
        // to the first, so that the place found before a change is still its place after the changes after it (a paragraph that moves away is not always after the one that has words changed).
        struct Op {
            int a;      // the paragraph of the original
            int pair;   // which of the gap's pairs (or -1 for a paragraph that is deleted)
        };
        QVector<Op> ops;
        for (int t = 0; t < pairs; ++t) ops.push_back({g.aOnly.at(t), t});
        for (const int t : g.aOnly.mid(pairs)) ops.push_back({t, -1});
        for (int i = g.a0; i < g.a1; ++i)
            if (movedA.at(i) >= 0) ops.push_back({i, -1});
        std::sort(ops.begin(), ops.end(), [](const Op &x, const Op &y) { return x.a > y.a; });
        for (const Op &op : std::as_const(ops)) {
            if (op.pair < 0) {
                const int t = op.a;
                if (!a.at(t).ok) {
                    if (a.at(t).kind == Para::Raw) ++context.work.notMarked;
                    continue;
                }
                QTextCursor c(story);
                const QTextBlock blk = story->findBlock(a.at(t).blockFrom);
                // (A paragraph that moved is deleted as the last one is: its text and the mark of the paragraph before it. The marks of the moves' ranges stay in it when the change is accepted, and a
                // paragraph with its own mark deleted that has them left can't be joined to the next, where this one's mark is the one that is left.)
                const bool lastWay = !blk.next().isValid() || movedA.at(t) >= 0;
                const int markPos = !lastWay || !blk.previous().isValid() ? blk.position() : blk.previous().position();
                c.setPosition(blk.position());
                if (!lastWay && blk.next().isValid()) {
                    c.setPosition(blk.next().position(), QTextCursor::KeepAnchor);
                } else if (blk.previous().isValid()) {
                    c.setPosition(blk.previous().position() + blk.previous().length() - 1);
                    c.setPosition(blk.position() + blk.length() - 1, QTextCursor::KeepAnchor);
                } else {
                    c.setPosition(blk.position() + blk.length() - 1, QTextCursor::KeepAnchor);
                }
                if (c.hasSelection()) {
                    removeSelection(*out, c);
                    if (movedA.at(t) >= 0) makeMove(*out, a.at(t).blockFrom, markPos, moveNames.at(movedA.at(t)), true);
                    ++made;
                }
                continue;
            }
            const PairInfo &pi = pairList.at(g.pairs.at(op.pair));
            const Para &pa = a.at(pi.a);
            if (!pa.ok || !b.at(pi.b).ok) continue;
            const QVector<TextEdit> &edits = pi.edits;
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
    return Outcome::Done;
}

} // namespace

std::unique_ptr<WriteDoc> compareDocuments(WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes, QString *error, const std::function<bool(int, int)> &progress, QString *note)
{
    CompareContext context;
    return compareDocumentsWith(context, original, revised, options, changes, error, progress, note);
}

namespace {

// The footnotes' Ids in the order their marks come in the text.
QVector<int> footnotesInOrder(WriteDoc &doc)
{
    QVector<std::pair<int, int>> byPlace;
    for (const int id : doc.citedFootnotes()) {
        const QVector<int> refs = doc.footnoteReferences(id);
        byPlace.push_back({refs.isEmpty() ? INT_MAX : refs.first(), id});
    }
    std::sort(byPlace.begin(), byPlace.end());
    QVector<int> out;
    for (const auto &p : std::as_const(byPlace)) out << p.second;
    return out;
}

// The words of the endnotes' paragraphs and of the text boxes in the text, as lists to tell how many differ (these are not marked yet).
void countUnmarkedElsewhere(WriteDoc &original, WriteDoc &revised, CompareContext &context, QStringList *notes)
{
    auto endnotes = [](WriteDoc &d) {
        QStringList out;
        for (QTextBlock b = d.story()->begin(); b.isValid(); b = b.next())
            if (b.blockFormat().hasProperty(prop::EndnoteBlock)) out << b.text();
        return out;
    };
    auto boxes = [&](WriteDoc &d) {
        QStringList out;
        int looked = 0;
        for (QTextBlock blk = d.story()->begin(); blk.isValid(); blk = blk.next())
            for (QTextBlock::iterator it = blk.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (!f.charFormat().hasProperty(prop::Atom)) continue;
                for (int i = 0; i < f.length(); ++i) {
                    if (++looked > context.limits.boxes) {
                        addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many text boxes to compare them all."));
                        return out;
                    }
                    if (const std::optional<QStringList> words = textBoxText(d, f.position() + i)) out << words->join(QLatin1Char('\n'));
                }
            }
        return out;
    };
    auto apart = [](const QStringList &x, const QStringList &y) {
        QHash<QString, int> have;
        for (const QString &t : x) ++have[t];
        int missing = 0;
        for (const QString &t : y)
            if (have.value(t) > 0) --have[t];
            else ++missing;
        int left = 0;
        for (auto it = have.constBegin(); it != have.constEnd(); ++it) left += it.value();
        return std::max(missing, left);
    };
    context.work.notMarked += apart(endnotes(original), endnotes(revised));
    context.work.notMarked += apart(boxes(original), boxes(revised));
}

} // namespace

std::unique_ptr<WriteDoc> compareDocumentsWith(CompareContext &context, WriteDoc &original, WriteDoc &revised, const CompareOptions &options, int *changes, QString *error, const std::function<bool(int, int)> &progress,
                                               QString *note)
{
    if (changes) *changes = 0;
    if (error) error->clear();
    if (note) note->clear();
    context.work = CompareWork();
    QString err;
    std::unique_ptr<WriteDoc> out = WriteDoc::fromDocx(original.toDocx(&err), &err);
    if (!out) {
        if (error) *error = err;
        return nullptr;
    }
    out->setReviewer(options.author.trimmed().isEmpty() ? QStringLiteral("Author") : options.author.trimmed(), QString());
    // (A copy that shows the differences is not a protected document, whatever the original was.)
    out->setProtection(std::nullopt);
    out->setTrackChanges(true);
    QStringList notes;
    int made = 0, done = 0;
    // The stories that are told apart: the text, then the footnotes (paired by the order of their marks), then the headers and footers (by section, kind, and slot, those a section has of its own in both).
    struct Pair {
        QTextDocument *story, *partner;
        bool body;
    };
    QVector<Pair> pairs;
    pairs.push_back({out->story(), revised.story(), true});
    int skipped = 0;
    auto add = [&](QTextDocument *x, QTextDocument *y) {
        if (!x || !y) {
            ++context.work.notMarked;
        } else if (pairs.size() - 1 >= context.limits.stories) {
            ++skipped;
        } else {
            pairs.push_back({x, y, false});
        }
    };
    const QVector<int> notesA = footnotesInOrder(*out), notesB = footnotesInOrder(revised);
    for (int k = 0; k < std::max(notesA.size(), notesB.size()); ++k)
        add(k < notesA.size() ? out->editableFootnote(notesA.at(k)) : nullptr, k < notesB.size() ? revised.editableFootnote(notesB.at(k)) : nullptr);
    for (int s = 0; s < std::max(out->sectionCount(), revised.sectionCount()); ++s)
        for (const bool footer : {false, true})
            for (int slot = 0; slot < 3; ++slot) {
                const bool ownA = s < out->sectionCount() && out->hasOwnHeaderFooter(s, footer, slot), ownB = s < revised.sectionCount() && revised.hasOwnHeaderFooter(s, footer, slot);
                if (ownA && ownB) add(out->editableHeaderFooter(s, footer, slot), revised.editableHeaderFooter(s, footer, slot));
                else if (ownA || ownB) ++context.work.notMarked;
            }
    context.work.stories = pairs.size();
    if (skipped) addNote(&notes, QCoreApplication::translate("jp::write::Compare", "Too many notes, headers, and footers to compare them all."));
    for (int k = 0; k < pairs.size(); ++k) {
        CompareOptions here = options;
        here.detectMoves = options.detectMoves && pairs.at(k).body;
        // (A change in a header, a footer, or a note is recorded as a change of that story, as the words of a text box are.)
        std::optional<TrackedStory> tracked;
        if (!pairs.at(k).body) tracked.emplace(pairs.at(k).story);
        const Outcome r = compareStory(context, out.get(), pairs.at(k).story, revised, pairs.at(k).partner, here, pairs.at(k).body, &made, &done, int(pairs.size()) - k - 1, &notes, progress);
        if (r == Outcome::Canceled) {
            if (error) error->clear();
            return nullptr;
        }
        if (r == Outcome::TooMany) {
            if (pairs.at(k).body) {
                if (error) *error = QCoreApplication::translate("jp::write::Compare", "A document with that many paragraphs can't be compared.");
                return nullptr;
            }
            ++context.work.notMarked;   // (a note, header, or footer with that many paragraphs: left as it is, and told)
        }
    }
    countUnmarkedElsewhere(original, revised, context, &notes);
    if (context.work.notMarked > 0)
        notes.push_back(QCoreApplication::translate("jp::write::Compare", "%n difference is in a place that Compare can't mark yet (a changed table, an endnote, or a text box), so it is not shown.",
                                                    "", int(context.work.notMarked)));
    if (progress) progress(done, done);
    if (changes) *changes = made;
    if (note) *note = notes.join(QLatin1Char(' ')).trimmed();
    return out;
}

} // namespace jp::write
