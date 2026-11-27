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
#include <QMap>
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
    int tablePair = -1;     // for a paragraph of a table that is told apart cell by cell: which table (its place among the original's tables); else -1
    int cellIndex = 0;      // and which paragraph of that table it is
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
// rows; a table that holds another table is "!", its shape, and all its words, so that it is the same as another only when everything in it is), and each paragraph says which table it is in (-1: none).
struct TableShapes {
    QStringList shapes;
    QVector<int> tableOfBlock;
};
TableShapes tableShapesOf(WriteDoc &doc, const QTextDocument *story)
{
    TableShapes out;
    QHash<const void *, int> ordinals;
    QVector<char> nestedSeen;
    QStringList words;
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
            words << QString();
            nestedSeen.push_back(0);
        }
        const int ordinal = ordinals.value(table);
        out.tableOfBlock.push_back(ordinal);
        words[ordinal] += b.text();
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)   // (what an element kept as it was written holds is told by its bytes)
            if (it.fragment().charFormat().hasProperty(prop::Atom)) words[ordinal] += QString::number(qHash(storedBytes(it.fragment().charFormat(), prop::Atom)), 16);
        if (b.blockFormat().hasProperty(prop::RawBlock)) words[ordinal] += QString::number(qHash(storedBytes(b.blockFormat(), prop::RawBlock)), 16);   // (so is what a paragraph kept as written holds)
        words[ordinal] += QLatin1Char('\x1e');
        if (nested) {
            nestedSeen[ordinal] = 1;
            continue;
        }
        QString &shape = out.shapes[ordinal];
        if (rec->row.get() != lastRow) shape += QLatin1Char('|');
        else if (key != lastKey) shape += QLatin1Char(',');
        lastRow = rec->row.get();
        lastKey = key;
        shape += QLatin1Char('p');
    }
    for (int k = 0; k < out.shapes.size(); ++k)
        if (nestedSeen.at(k)) out.shapes[k] = QLatin1Char('!') + out.shapes.at(k) + QLatin1Char('\x1f') + words.at(k);
    return out;
}

// What a paragraph or table is as a thing to line up: a paragraph by its words, a table (one block, at its first paragraph) by its shape.
struct Blocks {
    QStringList keys;
    QVector<int> table;   // for each: the table's ordinal, or -1 for a paragraph
};
Blocks blocksOf(const QTextDocument *story, const TableShapes &tables, const CompareOptions &options)
{
    Blocks out;
    QVector<char> seen(tables.shapes.size(), 0);
    int n = -1;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        ++n;
        const int t = n < tables.tableOfBlock.size() ? tables.tableOfBlock.at(n) : -1;
        if (t >= 0) {
            if (seen.at(t)) continue;
            seen[t] = 1;
            out.keys << QLatin1Char('\x02') + tables.shapes.at(t);
            out.table << t;
            continue;
        }
        QString k = b.text();
        k.remove(QChar::ObjectReplacementCharacter);
        if (options.ignoreCase) k = k.toLower();
        if (options.ignoreWhiteSpace) k = k.simplified();
        out.keys << QLatin1Char('p') + k;
        out.table << -1;
    }
    return out;
}

QVector<Para> paragraphsOf(QTextDocument *story, const QVector<int> &pairOfTable, const TableShapes &tables, bool *tooMany, bool withLooks, CompareContext &ctx, bool *looksCut)
{
    QVector<Para> out;
    *tooMany = false;
    int blockNumber = -1;
    QVector<int> cellsSeen(pairOfTable.size(), 0);
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
        if (table >= 0) {
            if (table < pairOfTable.size() && pairOfTable.at(table) >= 0) {
                p.tablePair = pairOfTable.at(table);
                p.cellIndex = cellsSeen[table]++;
            } else {
                p.ok = false;   // (a cell of a table that has no partner of the same shape)
            }
        }
        if (!p.ok && p.kind == Para::Normal) p.kind = Para::Cell;
        if (!p.ok) p.tablePair = -1;
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
            if (matched.at(i) || !paras.at(i).ok || paras.at(i).tablePair >= 0 || (&paras == &a && i == 0)) continue;   // (the first paragraph has none before it to take the mark of; a paragraph of a table is never moved out of it)
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

// Which table of the second story is which of the first: for each table of each, its partner's place (-1: none). The paragraphs and tables of the two are lined up (tables by their shape), so a
// table put in or taken out, or one that changed its shape, never shifts the pairing of the rest. Tables with no partner are counted (a changed table is one difference: the unpaired of each between
// two kept blocks are counted as the greater of their numbers). The work is the blocks lined up, within the limit of all the stories; past it no table is paired, and `cut` says so.
void pairTables(CompareContext &context, const QTextDocument *story, const TableShapes &ta, const QTextDocument *rstory, const TableShapes &tb, const CompareOptions &options, QVector<int> *partnerA,
                QVector<int> *partnerB, bool *cut)
{
    partnerA->fill(-1, int(ta.shapes.size()));
    partnerB->fill(-1, int(tb.shapes.size()));
    if (ta.shapes.isEmpty() && tb.shapes.isEmpty()) return;
    if (ta.shapes == tb.shapes) {   // (the same tables in the same order: each is its own partner)
        for (int k = 0; k < ta.shapes.size(); ++k) (*partnerA)[k] = (*partnerB)[k] = k;
        return;
    }
    if (ta.shapes.isEmpty() || tb.shapes.isEmpty()) {
        context.work.countUnmarked(CompareWork::Tables, ta.shapes.size() + tb.shapes.size());
        return;
    }
    const Blocks ba = blocksOf(story, ta, options), bb = blocksOf(rstory, tb, options);
    const qint64 blocks = qint64(ba.keys.size()) + bb.keys.size();
    if (context.work.alignBlocks + blocks > context.limits.alignBlocks) {
        *cut = true;
        context.work.countUnmarked(CompareWork::Tables, std::max(ta.shapes.size(), tb.shapes.size()));
        return;
    }
    context.work.alignBlocks += blocks;
    QVector<QPair<int, int>> matches;
    matchParagraphs(ba.keys, bb.keys, 0, int(ba.keys.size()), 0, int(bb.keys.size()), &matches, 24);
    std::sort(matches.begin(), matches.end());
    QVector<int> gapOfA(ba.keys.size(), 0), gapOfB(bb.keys.size(), 0);
    QVector<char> keptA(ba.keys.size(), 0), keptB(bb.keys.size(), 0);
    for (const auto &m : std::as_const(matches)) keptA[m.first] = keptB[m.second] = 1;
    for (int i = 0, gap = 0; i < ba.keys.size(); ++i) {
        gapOfA[i] = gap;
        if (keptA.at(i)) ++gap;
    }
    for (int j = 0, gap = 0; j < bb.keys.size(); ++j) {
        gapOfB[j] = gap;
        if (keptB.at(j)) ++gap;
    }
    QVector<int> unpairedA(matches.size() + 1, 0), unpairedB(matches.size() + 1, 0);
    for (const auto &m : std::as_const(matches))
        if (ba.table.at(m.first) >= 0 && bb.table.at(m.second) >= 0) (*partnerA)[ba.table.at(m.first)] = bb.table.at(m.second), (*partnerB)[bb.table.at(m.second)] = ba.table.at(m.first);
    for (int i = 0; i < ba.keys.size(); ++i)
        if (!keptA.at(i) && ba.table.at(i) >= 0) ++unpairedA[gapOfA.at(i)];
    for (int j = 0; j < bb.keys.size(); ++j)
        if (!keptB.at(j) && bb.table.at(j) >= 0) ++unpairedB[gapOfB.at(j)];
    for (int g = 0; g < unpairedA.size(); ++g) context.work.countUnmarked(CompareWork::Tables, std::max(unpairedA.at(g), unpairedB.at(g)));
}

// What the compare of the text says about where each paragraph of the original went, for telling whether a note's mark moved: for each paragraph of the original, the paragraph of the revised one that
// is its partner (-1: none, -2: not a paragraph that is compared), whether the two have the same words, the words that differ, and whether each paragraph of the revised one is compared.
struct Alignment {
    QVector<int> partner;
    QVector<char> exact, bOk;
    QHash<int, QVector<TextEdit>> edits;
};

void addNote(QStringList *notes, const QString &text)
{
    if (!notes->contains(text)) notes->push_back(text);
}

// One story of the copy against its partner in the revised document, with the limits and the counts of `context`. The body is the one that looks for moves.
Outcome compareStory(CompareContext &context, WriteDoc *out, QTextDocument *story, WriteDoc &revised, QTextDocument *rstory, const CompareOptions &options, bool isBody, int *madeTotal, int *doneTotal, int storiesLeft,
                     QStringList *notes, const std::function<bool(int, int)> &progress, Alignment *alignment = nullptr)
{
    const TableShapes tablesA = tableShapesOf(*out, story), tablesB = tableShapesOf(revised, rstory);
    // Which table is which: a table is told apart cell by cell only with the partner of the same shape that the paragraphs and tables around it lead to; a table with no partner is counted, and left as it is.
    QVector<int> partnerA, partnerB, pairOfA, pairOfB;
    bool tablesCut = false;
    pairTables(context, story, tablesA, rstory, tablesB, options, &partnerA, &partnerB, &tablesCut);
    if (tablesCut) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too much text to line up the tables, so they are not compared."));
    pairOfA.fill(-1, int(partnerA.size()));
    pairOfB.fill(-1, int(partnerB.size()));
    for (int k = 0; k < partnerA.size(); ++k)
        if (partnerA.at(k) >= 0 && !tablesA.shapes.at(k).startsWith(QLatin1Char('!'))) {
            pairOfA[k] = k;
            pairOfB[partnerA.at(k)] = k;
        }
    bool many1 = false, many2 = false;
    bool looksCut = false;
    QVector<Para> a = paragraphsOf(story, pairOfA, tablesA, &many1, options.compareFormatting, context, &looksCut);
    const QVector<Para> b = paragraphsOf(rstory, pairOfB, tablesB, &many2, options.compareFormatting, context, &looksCut);
    if (many1 || many2) return Outcome::TooMany;
    // The paragraphs that are the same in both, in order (the paragraphs of a table, only as the same place of its partner).
    auto key = [&](const Para &p) {
        QString k = p.ok ? p.text : QStringLiteral("\x01");
        if (options.ignoreCase) k = k.toLower();
        if (options.ignoreWhiteSpace) k = k.simplified();
        if (p.ok && p.tablePair >= 0) k = QStringLiteral("\x03%1:%2\x1f").arg(p.tablePair).arg(p.cellIndex) + k;
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
        QVector<int> aLeft, bLeft;      // the paragraphs of each that are in no pair (only in one: deleted, or put in)
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
            // (A paragraph just after a table is deleted with the mark before it, which is a cell's: it is not made a move.)
            if (m.first > 0 && tablesA.tableOfBlock.value(m.first - 1, -1) >= 0) continue;
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
        context.work.moves = moveNames.size();
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
    QVector<char> pairedA(a.size(), 0), pairedB(b.size(), 0);
    for (int k = 0; k < gaps.size(); ++k) {
        Gap &g = gaps[k];
        // (A paragraph is paired with one of the same kind: a paragraph of a table only with the same table's, and the rest with each other, each in order.)
        QMap<int, QVector<int>> byClassA, byClassB;
        auto classOf = [](const Para &p) { return p.ok ? p.tablePair : -2; };   // (what can't be compared is paired only with the same)
        for (const int i : std::as_const(g.aOnly)) byClassA[classOf(a.at(i))] << i;
        for (const int j : std::as_const(g.bOnly)) byClassB[classOf(b.at(j))] << j;
        for (auto it = byClassA.constBegin(); it != byClassA.constEnd(); ++it) {
            const QVector<int> &inA = it.value();
            const QVector<int> inB = byClassB.value(it.key());
            const int pairs = int(std::min(inA.size(), inB.size()));
            for (int t = 0; t < pairs; ++t) {
                PairInfo p{inA.at(t), inB.at(t), {}};
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
        for (const int pi : std::as_const(g.pairs)) pairedA[pairList.at(pi).a] = pairedB[pairList.at(pi).b] = 1;
        for (const int i : std::as_const(g.aOnly))
            if (!pairedA.at(i)) g.aLeft << i;
        for (const int j : std::as_const(g.bOnly))
            if (!pairedB.at(j)) g.bLeft << j;
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
        // The paragraphs that are only in the revised version (or are moved here), put after the last paragraph of the gap (or after the one before it).
        QVector<int> inserts = g.bLeft;
        for (int j = g.b0; j < g.b1; ++j)
            if (movedB.at(j) >= 0) inserts << j;
        std::sort(inserts.begin(), inserts.end());
        if (!inserts.isEmpty()) {
            QTextCursor c(story);
            const int after = g.a1 - 1;
            auto inTable = [&](int i) { return i >= 0 && i < a.size() && tablesA.tableOfBlock.value(i, -1) >= 0; };
            // After the paragraph before the gap; but not inside a table (right after a table's last cell, or at the start of a table): then before the paragraph the gap ends with, if that is not in a table.
            bool atStart = after < 0, placed = true;
            if (after >= 0 && !inTable(after)) {
                c.setPosition(a.at(after).blockEnd);
            } else {
                const int next = after >= 0 ? g.a1 : 0;
                atStart = true;
                if (next < a.size() && !inTable(next)) c.setPosition(a.at(next).blockFrom);
                else placed = false;
            }
            for (const int t : std::as_const(inserts)) {
                if (!b.at(t).ok) {
                    if (b.at(t).kind == Para::Raw) context.work.countUnmarked(CompareWork::KeptAsWritten);   // (a paragraph that stands for something kept as it was written)
                    continue;
                }
                if (!placed || b.at(t).tablePair >= 0) {   // (no place that is not in a table, or a paragraph of a table: not put in)
                    context.work.countUnmarked(CompareWork::Tables);
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
        for (int t = 0; t < g.pairs.size(); ++t) ops.push_back({pairList.at(g.pairs.at(t)).a, t});
        for (const int t : std::as_const(g.aLeft)) ops.push_back({t, -1});
        for (int i = g.a0; i < g.a1; ++i)
            if (movedA.at(i) >= 0) ops.push_back({i, -1});
        std::sort(ops.begin(), ops.end(), [](const Op &x, const Op &y) { return x.a > y.a; });
        for (const Op &op : std::as_const(ops)) {
            if (op.pair < 0) {
                const int t = op.a;
                if (!a.at(t).ok) {
                    if (a.at(t).kind == Para::Raw) context.work.countUnmarked(CompareWork::KeptAsWritten);
                    continue;
                }
                if (a.at(t).tablePair >= 0) {   // (a paragraph of a table that has no partner here: the table is not taken apart)
                    context.work.countUnmarked(CompareWork::Tables);
                    continue;
                }
                QTextCursor c(story);
                const QTextBlock blk = story->findBlock(a.at(t).blockFrom);
                // (A paragraph that moved is deleted as the last one is: its text and the mark of the paragraph before it. The marks of the moves' ranges stay in it when the change is accepted, and a
                // paragraph with its own mark deleted that has them left can't be joined to the next, where this one's mark is the one that is left.)
                const bool lastWay = !blk.next().isValid() || movedA.at(t) >= 0;
                if (lastWay && blk.previous().isValid() && tablesA.tableOfBlock.value(t - 1, -1) >= 0) {   // (the last paragraph, after a table: its mark is not the one that can go)
                    const int before = tablesA.tableOfBlock.value(t - 1, -1);
                    if (partnerA.value(before, -1) >= 0) context.work.countUnmarked(CompareWork::Tables);   // (after a table with no partner, it is that table's difference, counted already)
                    continue;
                }
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
    if (alignment) {
        alignment->partner.fill(-1, int(a.size()));
        alignment->exact.fill(0, int(a.size()));
        alignment->bOk.fill(0, int(b.size()));
        for (int i = 0; i < a.size(); ++i)
            if (!a.at(i).ok) alignment->partner[i] = -2;
        for (int j = 0; j < b.size(); ++j) alignment->bOk[j] = b.at(j).ok;
        for (int k = 0; k < keepA.size(); ++k) {
            const int i = keepA.at(k), j = keepB.at(k);
            if (!a.at(i).ok || !b.at(j).ok) continue;
            alignment->partner[i] = j;
            alignment->exact[i] = a.at(i).text == b.at(j).text;
        }
        for (const PairInfo &p : std::as_const(pairList)) {
            if (!a.at(p.a).ok || !b.at(p.b).ok) continue;
            alignment->partner[p.a] = p.b;
            alignment->edits.insert(p.a, p.edits);
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

// Where a footnote's first mark is: its paragraph (by place in the text) and how many letters of the paragraph's words come before it.
struct NoteMark {
    int block = -1;
    int offset = 0;
};
NoteMark markOfNote(WriteDoc &doc, int id)
{
    NoteMark m;
    const QVector<int> refs = doc.footnoteReferences(id);
    if (refs.isEmpty()) return m;
    const int position = refs.first();
    const QTextBlock b = doc.story()->findBlock(position);
    if (!b.isValid()) return m;
    m.block = b.blockNumber();
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        if (f.position() >= position) break;
        if (isAtom(cf) || cf.hasProperty(prop::TrackedDeleted)) continue;
        const QString text = f.text();
        for (int k = 0; k < text.size() && f.position() + k < position; ++k)
            if (text.at(k) != QChar::ObjectReplacementCharacter) ++m.offset;
    }
    return m;
}

// Whether a mark that is `oa` letters into a paragraph of the original is where the revised paragraph's mark is (`ob`), given the words that differ between the two.
bool sameSpot(int oa, int ob, const QVector<TextEdit> &edits)
{
    int delta = 0;
    for (const TextEdit &e : edits) {
        if (oa > e.deleteTo) {
            delta += int(e.insertText.size()) - (e.deleteTo - e.deleteFrom);
            continue;
        }
        if (oa < e.deleteFrom) break;
        const int start = e.deleteFrom + delta;   // (in words that were replaced: anywhere in what took their place)
        return ob >= start && ob <= start + int(e.insertText.size());
    }
    return ob == oa + delta;
}

// A fixed list of the kinds of difference that are not marked, as words: "a", "a and b", "a, b, and c".
QString kindsOfUnmarked(const CompareWork &work)
{
    QStringList names;
    for (int k = 0; k < CompareWork::UnmarkedKinds; ++k) {
        if (work.unmarked[k] <= 0) continue;
        switch (k) {
        case CompareWork::Tables: names << QCoreApplication::translate("jp::write::Compare", "tables"); break;
        case CompareWork::Footnotes: names << QCoreApplication::translate("jp::write::Compare", "footnotes"); break;
        case CompareWork::Endnotes: names << QCoreApplication::translate("jp::write::Compare", "endnotes"); break;
        case CompareWork::TextBoxes: names << QCoreApplication::translate("jp::write::Compare", "text boxes"); break;
        case CompareWork::HeadersFooters: names << QCoreApplication::translate("jp::write::Compare", "headers or footers"); break;
        default: names << QCoreApplication::translate("jp::write::Compare", "parts kept as they were written"); break;
        }
    }
    if (names.size() < 2) return names.value(0);
    const QString last = names.takeLast();
    if (names.size() == 1) return QCoreApplication::translate("jp::write::Compare", "%1 and %2").arg(names.first(), last);
    return QCoreApplication::translate("jp::write::Compare", "%1, and %2").arg(names.join(QLatin1String(", ")), last);
}

// The words of the endnotes' paragraphs and of the text boxes in the text, as lists to tell how many differ (these are not marked yet). Only real text boxes are read, within the limits.
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
        int drawings = 0, read = 0;
        for (QTextBlock blk = d.story()->begin(); blk.isValid(); blk = blk.next())
            for (QTextBlock::iterator it = blk.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                const QTextCharFormat cf = f.charFormat();
                if (!cf.hasProperty(prop::Atom) || cf.hasProperty(prop::TrackedDeleted) || !storedBytes(cf, prop::Atom).contains("drawing")) continue;   // (a picture, a shape, or a text box)
                for (int i = 0; i < f.length(); ++i) {
                    if (++drawings > context.limits.drawings) {
                        addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many pictures and shapes to look for all the text boxes among them."));
                        return out;
                    }
                    ++context.work.drawings;
                    const std::optional<QStringList> words = textBoxText(d, f.position() + i);
                    if (!words) continue;
                    if (++read > context.limits.boxes) {
                        addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many text boxes to compare them all."));
                        return out;
                    }
                    ++context.work.boxesRead;
                    out << words->join(QLatin1Char('\n'));
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
    context.work.countUnmarked(CompareWork::Endnotes, apart(endnotes(original), endnotes(revised)));
    context.work.countUnmarked(CompareWork::TextBoxes, apart(boxes(original), boxes(revised)));
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
    // The stories that are told apart: the text, then the footnotes (lined up by their words, so that a note put in or taken out shifts none of the rest), then the headers and footers (by section, kind, and slot,
    // those a section has of its own in both).
    struct Pair {
        QTextDocument *story, *partner;
        bool body;
        CompareWork::Unmarked kind;   // (what is counted when its paragraphs are too many to compare)
    };
    QVector<Pair> pairs;
    pairs.push_back({out->story(), revised.story(), true, CompareWork::Tables});
    int skipped = 0;
    auto room = [&] {
        if (pairs.size() - 1 < context.limits.stories) return true;
        ++skipped;
        return false;
    };
    // The footnotes: the first of each document that the limit allows are made into stories, and lined up by what they say. A note in one only (put in or taken out), and a note whose mark is in another place,
    // are counted: the calls that a person's tracked edits use don't record a footnote's mark put in, so they are not marked.
    struct Note {
        QTextDocument *story = nullptr;
        QString key;
        NoteMark mark;
    };
    auto notesOf = [&](WriteDoc &d) {
        QVector<Note> list;
        const QVector<int> ids = footnotesInOrder(d);
        const int most = int(std::max(0, context.limits.stories - int(pairs.size() - 1)));
        if (ids.size() > most) ++skipped;
        for (int k = 0; k < ids.size() && k < most; ++k) {
            Note n;
            n.mark = markOfNote(d, ids.at(k));
            n.story = d.editableFootnote(ids.at(k));
            if (n.story) {
                n.key = n.story->toPlainText();
                n.key.remove(QChar::ObjectReplacementCharacter);
                if (options.ignoreCase) n.key = n.key.toLower();
                if (options.ignoreWhiteSpace) n.key = n.key.simplified();
            }
            list << n;
        }
        return list;
    };
    const QVector<Note> notesA = notesOf(*out), notesB = notesOf(revised);
    QVector<std::pair<NoteMark, NoteMark>> notePairs;
    {
        QStringList keysA, keysB;
        for (const Note &n : notesA) keysA << n.key;
        for (const Note &n : notesB) keysB << n.key;
        QVector<QPair<int, int>> matches;
        matchParagraphs(keysA, keysB, 0, int(keysA.size()), 0, int(keysB.size()), &matches, 24);
        std::sort(matches.begin(), matches.end());
        auto pairUp = [&](int i, int j) {
            if (!notesA.at(i).story || !notesB.at(j).story) {
                context.work.countUnmarked(CompareWork::Footnotes);
                return;
            }
            pairs.push_back({notesA.at(i).story, notesB.at(j).story, false, CompareWork::Footnotes});
            notePairs.push_back({notesA.at(i).mark, notesB.at(j).mark});
        };
        int ia = 0, ib = 0;
        for (int m = 0; m <= matches.size(); ++m) {
            const int ea = m < matches.size() ? matches.at(m).first : int(notesA.size()), eb = m < matches.size() ? matches.at(m).second : int(notesB.size());
            // (The notes between two that are the same: those that are changed are told apart in order, and the ones left over are a note put in or taken out.)
            const int together = std::min(ea - ia, eb - ib);
            for (int t = 0; t < together; ++t) pairUp(ia + t, ib + t);
            context.work.countUnmarked(CompareWork::Footnotes, std::max(ea - ia, eb - ib) - together);
            if (m < matches.size()) pairUp(ea, eb);
            ia = ea + 1;
            ib = eb + 1;
        }
    }
    for (int s = 0; s < std::max(out->sectionCount(), revised.sectionCount()); ++s)
        for (const bool footer : {false, true})
            for (int slot = 0; slot < 3; ++slot) {
                const bool ownA = s < out->sectionCount() && out->hasOwnHeaderFooter(s, footer, slot), ownB = s < revised.sectionCount() && revised.hasOwnHeaderFooter(s, footer, slot);
                if (ownA && ownB) {
                    if (!room()) continue;   // (the limit is applied before the stories are made)
                    QTextDocument *x = out->editableHeaderFooter(s, footer, slot), *y = revised.editableHeaderFooter(s, footer, slot);
                    if (x && y) pairs.push_back({x, y, false, CompareWork::HeadersFooters});
                    else context.work.countUnmarked(CompareWork::HeadersFooters);
                } else if (ownA || ownB) {
                    context.work.countUnmarked(CompareWork::HeadersFooters);
                }
            }
    context.work.stories = pairs.size();
    if (skipped) addNote(&notes, QCoreApplication::translate("jp::write::Compare", "Too many notes, headers, and footers to compare them all."));
    Alignment alignment;
    for (int k = 0; k < pairs.size(); ++k) {
        CompareOptions here = options;
        here.detectMoves = options.detectMoves && pairs.at(k).body;
        // (A change in a header, a footer, or a note is recorded as a change of that story, as the words of a text box are.)
        std::optional<TrackedStory> tracked;
        if (!pairs.at(k).body) tracked.emplace(pairs.at(k).story);
        const Outcome r = compareStory(context, out.get(), pairs.at(k).story, revised, pairs.at(k).partner, here, pairs.at(k).body, &made, &done, int(pairs.size()) - k - 1, &notes, progress, pairs.at(k).body ? &alignment : nullptr);
        if (r == Outcome::Canceled) {
            if (error) error->clear();
            return nullptr;
        }
        if (r == Outcome::TooMany) {
            if (pairs.at(k).body) {
                if (error) *error = QCoreApplication::translate("jp::write::Compare", "A document with that many paragraphs can't be compared.");
                return nullptr;
            }
            context.work.countUnmarked(pairs.at(k).kind);   // (a note, header, or footer with that many paragraphs: left as it is, and told)
        }
    }
    // A note whose mark is in another paragraph, or at another place in a paragraph that is the same (or whose words that differ don't explain it), is a change the compare doesn't mark.
    for (const auto &np : std::as_const(notePairs)) {
        if (np.first.block < 0 || np.second.block < 0) continue;
        const int partner = alignment.partner.value(np.first.block, -1);
        if (partner == -2 || (np.second.block < alignment.bOk.size() && !alignment.bOk.at(np.second.block))) continue;   // (in a place that is counted as it is)
        bool same = partner == np.second.block;
        if (same && alignment.edits.contains(np.first.block)) same = sameSpot(np.first.offset, np.second.offset, alignment.edits.value(np.first.block));
        else if (same && alignment.exact.value(np.first.block)) same = np.first.offset == np.second.offset;
        if (!same) context.work.countUnmarked(CompareWork::Footnotes);
    }
    countUnmarkedElsewhere(original, revised, context, &notes);
    if (context.work.notMarked > 0)
        notes.push_back(QCoreApplication::translate("jp::write::Compare", "%n difference is in a place that Compare can't mark yet (%1), so it is not shown.", nullptr, int(context.work.notMarked)).arg(kindsOfUnmarked(context.work)));
    if (progress) progress(done, done);
    if (changes) *changes = made;
    if (note) *note = notes.join(QLatin1Char(' ')).trimmed();
    return out;
}

} // namespace jp::write
