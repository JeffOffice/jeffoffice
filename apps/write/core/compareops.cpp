#include "core/compareops.h"

#include "core/contentcontrolops.h"
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

// What the lining up of paragraphs may still do, for all the stories of a compare: the cells of the tables of lengths it may still fill, and the stretches (by where they begin in each text) it left without lining up
// because it had no room for them, which are shown whole as taken out and put in.
struct MatchBudget {
    CompareContext &context;
    QSet<QPair<int, int>> left;
    explicit MatchBudget(CompareContext &c) : context(c) {}
};

// The paragraphs of the two texts that are the same, in order, as which of each are kept: the same paragraphs at the start and at the end first, then (when what is between is too large for a table of
// lengths, or the compare has no more room for one) the paragraphs that are alone in each text and the same, as anchors in their order (patience diff), and in each stretch between the anchors the same again.
// A stretch that is still too large for a table, with no anchor in it, is left (`budget` says where it began).
void matchParagraphs(const QStringList &ka, const QStringList &kb, int a0, int a1, int b0, int b1, QVector<QPair<int, int>> *out, int depth, MatchBudget *budget = nullptr)
{
    while (a0 < a1 && b0 < b1 && ka.at(a0) == kb.at(b0)) out->push_back({a0++, b0++});
    QVector<QPair<int, int>> tail;
    while (a0 < a1 && b0 < b1 && ka.at(a1 - 1) == kb.at(b1 - 1)) tail.push_back({--a1, --b1});
    if (a0 < a1 && b0 < b1) {
        const qint64 cells = qint64(a1 - a0) * qint64(b1 - b0);
        if (cells <= kMostTableCells && (!budget || budget->context.work.matchCells + cells <= budget->context.limits.matchCells)) {
            if (budget) budget->context.work.matchCells += cells;
            const QStringList sa = ka.mid(a0, a1 - a0), sb = kb.mid(b0, b1 - b0);
            QVector<int> keepA, keepB;
            commonRun(sa, sb, &keepA, &keepB);
            for (int i = 0; i < keepA.size(); ++i) out->push_back({a0 + keepA.at(i), b0 + keepB.at(i)});
        } else {
            QVector<QPair<int, int>> anchors;
            if (depth > 0) {
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
                for (int k = tails.isEmpty() ? -1 : tails.last(); k >= 0; k = prev.at(k)) anchors.prepend(pairs.at(k));
            }
            if (anchors.isEmpty()) {
                if (budget) budget->left.insert({a0, b0});
            } else {
                int pa = a0, pb = b0;
                for (const auto &anchor : std::as_const(anchors)) {
                    matchParagraphs(ka, kb, pa, anchor.first, pb, anchor.second, out, depth - 1, budget);
                    out->push_back(anchor);
                    pa = anchor.first + 1;
                    pb = anchor.second + 1;
                }
                matchParagraphs(ka, kb, pa, a1, pb, b1, out, depth - 1, budget);
            }
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
    int cellIndex = 0;      // and which paragraph of that table it is (of the rows that are lined up, when only some are)
    int tableOrd = -1;      // the table it is in (by order among its own text's tables), else -1
    int row = -1;           // and its row
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

// What a paragraph says as words to compare: the words without the pieces kept as they were written and what a tracked change took out, as the options say.
QString comparedWords(const QTextBlock &b, const CompareOptions &options)
{
    QString k;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        if (isAtom(cf) || cf.hasProperty(prop::TrackedDeleted)) continue;
        k += it.fragment().text();
    }
    k.remove(QChar::ObjectReplacementCharacter);
    if (options.ignoreCase) k = k.toLower();
    if (options.ignoreWhiteSpace) k = k.simplified();
    return k;
}

// A row of a table, as the compare tells rows apart: where its paragraphs are, its words cell by cell, and its shape (for each cell the columns it takes, whether it is joined down, and how many
// paragraphs it has, and the columns left before and after it).
struct RowInfo {
    int firstBlock = 0, blocks = 0;     // its paragraphs, by number
    QString key, sig, form;             // (`form`: the shape without the paragraphs of each cell)
    QVector<int> cellParas;             // how many paragraphs each cell has
    QVector<QString> cellKeys;          // the words of each cell
    QVector<QPair<int, int>> cellSpan;  // for each cell: the columns it takes, and 0 for a cell that is not joined down, 1 for one that begins a join, 2 for one that continues it
    int before = 0, after = 0;
    bool kept = false;                  // it holds an element that is kept as it was written (a picture, a field, a paragraph kept whole)
};
struct TableInfo {
    QVector<RowInfo> rows;
    bool nested = false;                // another table is in it
    bool merged = false;                // a cell is joined with the one below
    bool kept = false;
    int columns = 0;
    QString content;                    // all the words and the shape, as one short key (two tables with the same are the same)
};

// The tables of a story, for telling which cells can be compared: each table (a top-level one, by order) is its shape written as a few letters (a "p" for each paragraph, "," between cells, "|" between
// rows; a table that holds another table is "!", its shape, and all its words, so that it is the same as another only when everything in it is), and each paragraph says which table it is in (-1: none)
// and which row of it.
struct TableShapes {
    QStringList shapes;
    QVector<int> tableOfBlock, rowOfBlock;
    QVector<TableInfo> infos;
};
TableShapes tableShapesOf(WriteDoc &doc, const QTextDocument *story, const CompareOptions &options)
{
    TableShapes out;
    QHash<const void *, int> ordinals;
    QVector<char> nestedSeen;
    QStringList words;
    int lastKey = 0;
    const void *lastRow = nullptr;
    int blockNumber = -1;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        ++blockNumber;
        int key = cellKeyOf(b);
        const flow::CellRecord *rec = key ? doc.flow().tables.get(key) : nullptr;
        out.rowOfBlock.push_back(-1);
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
            out.infos.push_back(TableInfo());
        }
        const int ordinal = ordinals.value(table);
        out.tableOfBlock.push_back(ordinal);
        words[ordinal] += b.text();
        bool kept = b.blockFormat().hasProperty(prop::RawBlock) || b.blockFormat().hasProperty(prop::RawBefore) || b.blockFormat().hasProperty(prop::RawAfter);   // (a content control's start or end is kept round the paragraph)
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)   // (what an element kept as it was written holds is told by its bytes)
            if (it.fragment().charFormat().hasProperty(prop::Atom)) {
                words[ordinal] += QString::number(qHash(storedBytes(it.fragment().charFormat(), prop::Atom)), 16);
                kept = true;
            }
        if (b.blockFormat().hasProperty(prop::RawBlock)) words[ordinal] += QString::number(qHash(storedBytes(b.blockFormat(), prop::RawBlock)), 16);   // (so is what a paragraph kept as written holds)
        words[ordinal] += QLatin1Char('\x1e');
        TableInfo &info = out.infos[ordinal];
        if (kept) info.kept = true;
        if (nested) {
            nestedSeen[ordinal] = 1;
            info.nested = true;
            continue;
        }
        QString &shape = out.shapes[ordinal];
        const bool newRow = rec->row.get() != lastRow;
        if (newRow) shape += QLatin1Char('|');
        else if (key != lastKey) shape += QLatin1Char(',');
        if (info.columns == 0) info.columns = int(rec->table->columns.size());
        if (rec->cell.vmerge != flow::CellData::VMerge::None) info.merged = true;
        if (newRow) {
            RowInfo row;
            row.firstBlock = blockNumber;
            row.before = rec->row ? rec->row->before : 0;
            row.after = rec->row ? rec->row->after : 0;
            info.rows.push_back(row);
        }
        RowInfo &row = info.rows.last();
        if (newRow || key != lastKey) {
            row.cellParas.push_back(0);
            row.cellKeys.push_back(QString());
            row.cellSpan.push_back({rec->cell.span, rec->cell.vmerge == flow::CellData::VMerge::None ? 0 : rec->cell.vmerge == flow::CellData::VMerge::Restart ? 1 : 2});
        }
        ++row.cellParas.last();
        row.cellKeys.last() += comparedWords(b, options) + QLatin1Char('\x1e');
        ++row.blocks;
        if (kept) row.kept = true;
        out.rowOfBlock.last() = int(info.rows.size()) - 1;
        lastRow = rec->row.get();
        lastKey = key;
        shape += QLatin1Char('p');
    }
    for (int k = 0; k < out.shapes.size(); ++k) {
        if (nestedSeen.at(k)) out.shapes[k] = QLatin1Char('!') + out.shapes.at(k) + QLatin1Char('\x1f') + words.at(k);
        TableInfo &info = out.infos[k];
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(out.shapes.at(k).toUtf8());
        for (RowInfo &row : info.rows) {
            row.sig = row.form = QStringLiteral("%1,%2;").arg(row.before).arg(row.after);
            for (int c = 0; c < row.cellParas.size(); ++c) {
                row.sig += QStringLiteral("%1.%2.%3;").arg(row.cellSpan.at(c).first).arg(row.cellSpan.at(c).second).arg(row.cellParas.at(c));
                row.form += QStringLiteral("%1.%2;").arg(row.cellSpan.at(c).first).arg(row.cellSpan.at(c).second);
            }
            row.key = row.cellKeys.join(QLatin1Char('\x1f'));
            hash.addData(row.sig.toUtf8());
            hash.addData(row.key.toUtf8());
            hash.addData("\x1c");
        }
        info.content = QString::fromLatin1(hash.result().toHex());
    }
    return out;
}

// What a paragraph or table is as a thing to line up: a paragraph by its words, a table (one block, at its first paragraph) by all it holds.
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
            out.keys << QLatin1Char('\x02') + tables.infos.at(t).content;
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

QVector<Para> paragraphsOf(QTextDocument *story, const QVector<int> &pairOfTable, const QVector<QVector<char>> &keptRows, const TableShapes &tables, bool *tooMany, bool withLooks, CompareContext &ctx, bool *looksCut)
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
            p.tableOrd = table;
            p.row = blockNumber < tables.rowOfBlock.size() ? tables.rowOfBlock.at(blockNumber) : -1;
            const bool lined = keptRows.value(table).isEmpty() || (p.row >= 0 && p.row < keptRows.at(table).size() && keptRows.at(table).at(p.row));
            if (table < pairOfTable.size() && pairOfTable.at(table) >= 0 && lined) {
                p.tablePair = pairOfTable.at(table);
                p.cellIndex = cellsSeen[table]++;
            } else {
                p.ok = false;   // (a cell of a table that has no partner of the same shape, or of a row that has none)
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
// The words of the revised paragraph `srcBlock`, each piece in its own look, put in at `c` (which ends after them). `inOwnRow`: it is a paragraph of a row that is put in, whose words are not changes of their own (they
// take the paragraph mark's record of the row's insertion, which is let go).
void countDropped(CompareWork *work, const QTextCharFormat &cf, int n = 1)
{
    if (!work) return;
    const office::wml::Inline a = unpackAtom(storedBytes(cf, prop::Atom));
    if (a.role == office::wml::Inline::Close) return;   // (the end of a pair is counted with its start)
    const QString &t = a.tag;
    if (t == QLatin1String("footnoteReference") || t == QLatin1String("endnoteReference")) work->dropped[CompareWork::NoteMarks] += n;
    else if (t == QLatin1String("drawing") || t == QLatin1String("pict") || t == QLatin1String("object")) work->dropped[CompareWork::Pictures] += n;
    else if (t == QLatin1String("hyperlink")) work->dropped[CompareWork::Links] += n;
    else if (cf.hasProperty(tp::Field) || t.startsWith(QLatin1String("fld")) || t == QLatin1String("instrText")) work->dropped[CompareWork::Fields] += n;
    else work->dropped[CompareWork::OtherParts] += n;
}

// What the words of a paragraph of the revised text hold that new text can't carry: counted (one pass over its pieces; a piece of equal atoms is one fragment, so each mark in it counts).
void countDroppedIn(CompareWork *work, const QTextBlock &block)
{
    for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        if (isAtom(cf) && !cf.hasProperty(prop::TrackedDeleted)) countDropped(work, cf, std::max(1, int(f.text().count(QChar::ObjectReplacementCharacter))));
    }
}

void putRevisedWords(WriteDoc &out, QTextDocument *ostory, QTextCursor &c, const QTextBlock &srcBlock, bool inOwnRow = false, CompareWork *work = nullptr)
{
    for (QTextBlock::iterator it = srcBlock.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        if (isAtom(cf)) {
            if (!cf.hasProperty(prop::TrackedDeleted)) countDropped(work, cf, std::max(1, int(f.text().count(QChar::ObjectReplacementCharacter))));
            continue;
        }
        if (cf.hasProperty(prop::TrackedDeleted)) continue;
        QString piece = f.text();
        piece.remove(QChar::ObjectReplacementCharacter);
        if (piece.isEmpty()) continue;
        const int from = c.position();
        insertText(out, c, piece);
        const UntrackedEdit untracked;
        QTextCursor range(ostory);
        range.setPosition(from);
        range.setPosition(c.position(), QTextCursor::KeepAnchor);
        changeCharFormat(out, range, [&](QTextCharFormat &g) {
            giveLook(g, cf);
            if (inOwnRow) {
                g.clearProperty(prop::RawRPr);
                g.clearProperty(tp::Revision);
                g.clearProperty(tp::RevisionAuthor);
            }
        });
    }
}

// The paragraph at `blockStart` made like the revised paragraph `srcBlock`: its style, alignment, indents, and list.
void dressLikeRevised(WriteDoc &out, QTextDocument *ostory, int blockStart, WriteDoc &revised, const QTextBlock &srcBlock)
{
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
}

int insertRevisedParagraph(WriteDoc &out, QTextDocument *ostory, QTextCursor &c, WriteDoc &revised, QTextDocument *rstory, const Para &source, bool before, bool inOwnRow = false, CompareWork *work = nullptr)
{
    const QTextBlock srcBlock = rstory->findBlock(source.blockFrom);
    int blockStart = 0;
    if (before) {
        putRevisedWords(out, ostory, c, srcBlock, inOwnRow, work);
        insertParagraph(out, c);
        blockStart = ostory->findBlock(c.position()).previous().position();
    } else {
        insertParagraph(out, c);
        blockStart = ostory->findBlock(c.position()).position();
        putRevisedWords(out, ostory, c, srcBlock, inOwnRow, work);
    }
    dressLikeRevised(out, ostory, blockStart, revised, srcBlock);
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

// What the compare does with the tables of a story. A table is paired with the table of the other copy that is its partner, and then it is
//   Exact: the same, so nothing is done;
//   Same: the same shape, so told apart cell by cell (as every table was, once);
//   Rows: not the same shape (rows put in or taken out), so told apart row by row: the rows that are the same, or only changed in some of their cells, are lined up and compared cell by cell, a row only
//         in the revised copy is put in as a tracked row, one only in the original is taken out as a tracked row;
// or it has no partner and it is
//   Whole: put in or taken out whole as a tracked table;
//   Counted: a difference that can't be marked yet (a column put in or taken out, cells joined or split, a table in a table, a table in a note, a header, or a footer, or too many changes), counted and left as it is.
// Two tables are partners when the cells of their words, each matched once, are at least half of the cells that have words in the larger of the two (the same table with changed rows), or when they are the only
// two between the same two paragraphs that are the same and have the same shape; tables that are not alike are one taken out and another put in.
struct RowEdit {
    int a = -1, b = -1;                 // the original's table, and the revised one's (by order)
    QVector<int> deleted;               // the rows of the original's table that are only in it
    struct Put {
        int row;                        // a row of the revised table that is only in it
        int ref;                        // the row of the original's table it goes beside, which has its shape
        bool below;                     // under it (else above)
    };
    QVector<Put> put;
};
struct TablePlan {
    enum State : char { Counted, Whole, Exact, Same, Rows };
    QVector<int> partnerA, partnerB;
    QVector<char> stateA, stateB;
    QVector<QVector<char>> keptA, keptB;    // for a table told apart by row: which rows are lined up (each with one of the other's)
    QVector<RowEdit> edits;
};

// Lines up the rows of two tables that are not of the same shape: rows that are the same, then in each stretch between them the rows in order, those that have the same shape each with each. False when the
// tables can't be told apart by row (and `structure` says it is because of their shape; else no row is the same: they are not the same table).
bool planRows(const TableInfo &ta, const TableInfo &tb, CompareContext &context, RowEdit *out, bool *structure, bool *cut)
{
    *structure = false;
    if (ta.nested || tb.nested || ta.columns != tb.columns || ta.rows.isEmpty() || tb.rows.isEmpty()) {
        *structure = true;
        return false;
    }
    const qint64 rows = qint64(ta.rows.size()) + tb.rows.size();
    if (context.work.rowsLined + rows > context.limits.alignRows) {
        *cut = true;
        *structure = true;
        return false;
    }
    context.work.rowsLined += rows;
    QStringList ka, kb;
    for (const RowInfo &r : ta.rows) ka << r.sig + QLatin1Char('\x1d') + r.key;
    for (const RowInfo &r : tb.rows) kb << r.sig + QLatin1Char('\x1d') + r.key;
    QVector<QPair<int, int>> same;
    MatchBudget budget(context);
    matchParagraphs(ka, kb, 0, int(ka.size()), 0, int(kb.size()), &same, 24, &budget);
    std::sort(same.begin(), same.end());
    QVector<QPair<int, int>> lined;
    int ia = 0, ib = 0;
    for (int m = 0; m <= same.size(); ++m) {
        const int ea = m < same.size() ? same.at(m).first : int(ta.rows.size()), eb = m < same.size() ? same.at(m).second : int(tb.rows.size());
        // (Between two rows that are the same: those of each in order, the ones of the same shape with each other.)
        for (int t = 0; t < std::min(ea - ia, eb - ib); ++t)
            if (ta.rows.at(ia + t).sig == tb.rows.at(ib + t).sig) lined.push_back({ia + t, ib + t});
        if (m < same.size()) lined.push_back({ea, eb});
        ia = ea + 1;
        ib = eb + 1;
    }
    std::sort(lined.begin(), lined.end());
    if (lined.isEmpty()) return false;
    QVector<char> inA(ta.rows.size(), 0), inB(tb.rows.size(), 0);
    for (const auto &p : std::as_const(lined)) inA[p.first] = inB[p.second] = 1;
    for (int i = 0; i < inA.size(); ++i)
        if (!inA.at(i)) out->deleted << i;
    // (A row is not taken out of a table that has cells joined down, nor put in one: the model can't record that, so those tables are told apart only when no row goes or comes.)
    if ((ta.merged || tb.merged) && (!out->deleted.isEmpty() || lined.size() < tb.rows.size())) {
        *structure = true;
        out->deleted.clear();
        return false;
    }
    // A row only in the revised table goes beside the row of the original that its neighbor above is lined up with (under it), or, at the top, beside the first (above it); a row with cells that are joined or
    // of other widths than that row's, or one that holds an element kept as written, can't be put in.
    int up = -1;   // the lined-up pair just above
    int next = 0;
    for (int j = 0; j < inB.size(); ++j) {
        if (inB.at(j)) {
            up = next++;
            continue;
        }
        RowEdit::Put put;
        put.row = j;
        put.below = up >= 0;
        put.ref = lined.at(up >= 0 ? up : 0).first;
        if (ta.rows.at(put.ref).form != tb.rows.at(j).form || tb.rows.at(j).kept) {
            *structure = true;
            out->deleted.clear();
            out->put.clear();
            return false;
        }
        out->put << put;
    }
    return true;
}

// A bag of the words of the cells of a table that have words, each cell by its words.
QHash<QString, int> bagOfCells(const TableInfo &t, qint64 *cells)
{
    QHash<QString, int> bag;
    *cells = 0;
    for (const RowInfo &r : t.rows)
        for (const QString &k : r.cellKeys)
            {
                const QString words = QString(k).remove(QLatin1Char('\x1e'));
                if (words.isEmpty()) continue;
                ++bag[words];
                ++*cells;
            }
    return bag;
}

// Which table of the second story is which of the first, with what is done with each (see TablePlan). The paragraphs and tables of the two are lined up (a table that is the same as another by all it holds is
// that one), so a table put in or taken out never shifts the pairing of the rest; the tables between two lined-up blocks are paired by how alike they are. Counted: the tables that have no partner and can't be
// put in or taken out are counted as one difference for the unpaired of each between two kept blocks, the greater of their numbers. The work is the blocks lined up and the rows told apart, within the limits of
// all the stories; past them `cut` says so.
void pairTables(CompareContext &context, const QTextDocument *story, const TableShapes &ta, const QTextDocument *rstory, const TableShapes &tb, const CompareOptions &options, bool isBody, TablePlan *plan, bool *cut,
                bool *rowsCut)
{
    const int na = int(ta.infos.size()), nb = int(tb.infos.size());
    plan->partnerA.fill(-1, na);
    plan->partnerB.fill(-1, nb);
    plan->stateA.fill(TablePlan::Counted, na);
    plan->stateB.fill(TablePlan::Counted, nb);
    plan->keptA.resize(na);
    plan->keptB.resize(nb);
    if (na == 0 && nb == 0) return;
    // (The rows a table puts in or takes out are held to a total for the whole compare: a table that would go past it is counted.)
    auto reserve = [&](qint64 taken, qint64 put) {
        if (context.work.rowsPut + context.work.rowsTaken + taken + put > context.limits.tableRows) {
            *rowsCut = true;
            return false;
        }
        context.work.rowsTaken += taken;
        context.work.rowsPut += put;
        return true;
    };
    auto settleLone = [&](int k, bool original) {
        const TableInfo &t = original ? ta.infos.at(k) : tb.infos.at(k);
        // (A table taken out can't have cells joined down, and one put in can't hold a picture, a field, or a table.)
        const bool can = isBody && !t.rows.isEmpty() && !t.nested && (original ? !t.merged : !t.kept);
        if (can && (original ? reserve(t.rows.size(), 0) : reserve(0, t.rows.size()))) (original ? plan->stateA : plan->stateB)[k] = TablePlan::Whole;
    };
    QStringList contentA, contentB;
    for (const TableInfo &t : ta.infos) contentA << t.content;
    for (const TableInfo &t : tb.infos) contentB << t.content;
    if (contentA == contentB) {   // (the same tables in the same order: each is its own partner)
        for (int k = 0; k < na; ++k) {
            plan->partnerA[k] = plan->partnerB[k] = k;
            plan->stateA[k] = plan->stateB[k] = TablePlan::Exact;
        }
        return;
    }
    if (na == 0 || nb == 0) {
        qint64 counted = 0;
        for (int k = 0; k < na; ++k) {
            settleLone(k, true);
            if (plan->stateA.at(k) == TablePlan::Counted) ++counted;
        }
        for (int k = 0; k < nb; ++k) {
            settleLone(k, false);
            if (plan->stateB.at(k) == TablePlan::Counted) ++counted;
        }
        context.work.countUnmarked(CompareWork::Tables, counted);
        return;
    }
    const Blocks ba = blocksOf(story, ta, options), bb = blocksOf(rstory, tb, options);
    const qint64 blocks = qint64(ba.keys.size()) + bb.keys.size();
    if (context.work.alignBlocks + blocks > context.limits.alignBlocks) {
        *cut = true;
        context.work.countUnmarked(CompareWork::Tables, std::max(na, nb));
        return;
    }
    context.work.alignBlocks += blocks;
    QVector<QPair<int, int>> matches;
    MatchBudget budget(context);
    matchParagraphs(ba.keys, bb.keys, 0, int(ba.keys.size()), 0, int(bb.keys.size()), &matches, 24, &budget);
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
    for (const auto &m : std::as_const(matches))
        if (ba.table.at(m.first) >= 0 && bb.table.at(m.second) >= 0) {
            const int a = ba.table.at(m.first), b = bb.table.at(m.second);
            plan->partnerA[a] = b;
            plan->partnerB[b] = a;
            plan->stateA[a] = plan->stateB[b] = TablePlan::Exact;
        }
    QVector<QVector<int>> loneA(matches.size() + 1), loneB(matches.size() + 1);   // the tables with no partner yet, by the stretch between kept blocks
    for (int i = 0; i < ba.keys.size(); ++i)
        if (!keptA.at(i) && ba.table.at(i) >= 0) loneA[gapOfA.at(i)] << ba.table.at(i);
    for (int j = 0; j < bb.keys.size(); ++j)
        if (!keptB.at(j) && bb.table.at(j) >= 0) loneB[gapOfB.at(j)] << bb.table.at(j);
    QVector<qint64> counted(matches.size() + 1, 0);   // (the unpaired tables that are counted, of each side)
    QVector<qint64> countedB(matches.size() + 1, 0);
    // How alike two tables are, as the cells of their words that are in both (each in as many as it is in both), of the cells with words in the larger one: at least a half makes them partners.
    QHash<int, QHash<QString, int>> bagsA, bagsB;
    QHash<int, qint64> cellsA, cellsB;
    auto bagOf = [&](const TableInfo &t, int k, QHash<int, QHash<QString, int>> &bags, QHash<int, qint64> &cells) -> const QHash<QString, int> & {
        auto it = bags.find(k);
        if (it == bags.end()) {
            context.work.rowsLined += t.rows.size();
            qint64 n = 0;
            it = bags.insert(k, bagOfCells(t, &n));
            cells.insert(k, n);
        }
        return it.value();
    };
    for (int g = 0; g < loneA.size(); ++g) {
        const QVector<int> &la = loneA.at(g), &lb = loneB.at(g);
        QVector<QPair<int, int>> paired;
        const bool both = !la.isEmpty() && !lb.isEmpty();
        if (both && qint64(la.size()) * lb.size() <= 4096) {
            QVector<QVector<double>> weight(la.size(), QVector<double>(lb.size(), 0.0));
            for (int i = 0; i < la.size(); ++i)
                for (int j = 0; j < lb.size(); ++j) {
                    const TableInfo &x = ta.infos.at(la.at(i)), &y = tb.infos.at(lb.at(j));
                    const qint64 needed = (bagsA.contains(la.at(i)) ? 0 : x.rows.size()) + (bagsB.contains(lb.at(j)) ? 0 : y.rows.size());
                    if (context.work.rowsLined + needed > context.limits.alignRows) {   // (no room to look at the rows of these two tables)
                        *rowsCut = true;
                        continue;
                    }
                    const QHash<QString, int> &bx = bagOf(x, la.at(i), bagsA, cellsA), &by = bagOf(y, lb.at(j), bagsB, cellsB);
                    qint64 common = 0;
                    for (auto it = bx.constBegin(); it != bx.constEnd(); ++it) common += std::min(it.value(), by.value(it.key()));
                    const qint64 most = std::max(cellsA.value(la.at(i)), cellsB.value(lb.at(j)));
                    double w = 0;
                    if (most > 0 && common * 2 >= most) w = double(common) / double(most);
                    else if (most == 0 && ta.shapes.at(la.at(i)) == tb.shapes.at(lb.at(j))) w = 1;
                    else if (la.size() == 1 && lb.size() == 1 && ta.shapes.at(la.at(i)) == tb.shapes.at(lb.at(j)) && !x.nested) w = 0.01;   // (the only two, of the same shape)
                    weight[i][j] = w;
                }
            // The best way to pair them in order (as the longest run of words is found).
            QVector<QVector<double>> best(la.size() + 1, QVector<double>(lb.size() + 1, 0.0));
            for (int i = int(la.size()) - 1; i >= 0; --i)
                for (int j = int(lb.size()) - 1; j >= 0; --j) {
                    best[i][j] = std::max(best[i + 1][j], best[i][j + 1]);
                    if (weight[i][j] > 0) best[i][j] = std::max(best[i][j], best[i + 1][j + 1] + weight[i][j]);
                }
            for (int i = 0, j = 0; i < la.size() && j < lb.size();) {
                if (weight[i][j] > 0 && best[i][j] == best[i + 1][j + 1] + weight[i][j]) {
                    paired.push_back({la.at(i), lb.at(j)});
                    ++i;
                    ++j;
                } else if (best[i + 1][j] >= best[i][j + 1]) {
                    ++i;
                } else {
                    ++j;
                }
            }
        }
        QSet<int> usedA, usedB;
        for (const auto &p : std::as_const(paired)) {
            const int a = p.first, b = p.second;
            const TableInfo &x = ta.infos.at(a), &y = tb.infos.at(b);
            if (!isBody) {   // (tables of a note, a header, or a footer are told apart only cell by cell, when their rows and cells are alike in shape)
                bool same = ta.shapes.at(a) == tb.shapes.at(b) && !x.nested && x.columns == y.columns && x.rows.size() == y.rows.size();
                for (int r = 0; same && r < x.rows.size(); ++r) same = x.rows.at(r).sig == y.rows.at(r).sig;
                usedA.insert(a);
                usedB.insert(b);
                if (same) {
                    plan->partnerA[a] = b;
                    plan->partnerB[b] = a;
                    plan->stateA[a] = plan->stateB[b] = TablePlan::Same;
                } else {
                    ++counted[g];
                    ++countedB[g];
                }
                continue;
            }
            RowEdit edit;
            bool structure = false;
            if (planRows(x, y, context, &edit, &structure, rowsCut)) {
                if (reserve(edit.deleted.size(), edit.put.size())) {
                    edit.a = a;
                    edit.b = b;
                    plan->partnerA[a] = b;
                    plan->partnerB[b] = a;
                    plan->stateA[a] = plan->stateB[b] = TablePlan::Rows;
                    plan->keptA[a].fill(1, int(x.rows.size()));
                    plan->keptB[b].fill(1, int(y.rows.size()));
                    for (const int r : std::as_const(edit.deleted)) plan->keptA[a][r] = 0;
                    for (const RowEdit::Put &put : std::as_const(edit.put)) plan->keptB[b][put.row] = 0;
                    plan->edits << edit;
                    usedA.insert(a);
                    usedB.insert(b);
                    continue;
                }
                structure = true;
            }
            if (structure) {   // (cells joined or split, or a column put in or taken out: a difference that is counted, and the two tables are left as they are)
                usedA.insert(a);
                usedB.insert(b);
                ++counted[g];
                ++countedB[g];
            }
        }
        for (const int a : la)
            if (!usedA.contains(a) && plan->partnerA.at(a) < 0) {
                settleLone(a, true);
                if (plan->stateA.at(a) == TablePlan::Counted) ++counted[g];
            }
        for (const int b : lb)
            if (!usedB.contains(b) && plan->partnerB.at(b) < 0) {
                settleLone(b, false);
                if (plan->stateB.at(b) == TablePlan::Counted) ++countedB[g];
            }
        // (A table taken out beside one that can't be put in, or put in beside one that can't be taken out, is not marked: Accept All would give neither the revised text nor the original's, so both are counted.)
        auto anyCounted = [&](const QVector<int> &list, const QVector<char> &state, const QVector<int> &partner) {
            for (const int k : list)
                if (partner.at(k) < 0 && state.at(k) == TablePlan::Counted) return true;
            return false;
        };
        const bool aStays = anyCounted(la, plan->stateA, plan->partnerA), bStays = anyCounted(lb, plan->stateB, plan->partnerB);
        if (aStays != bStays) {
            for (const int k : aStays ? lb : la) {
                auto &state = aStays ? plan->stateB : plan->stateA;
                const auto &partner = aStays ? plan->partnerB : plan->partnerA;
                if (partner.at(k) >= 0 || state.at(k) != TablePlan::Whole) continue;
                state[k] = TablePlan::Counted;
                (aStays ? context.work.rowsPut : context.work.rowsTaken) -= (aStays ? tb : ta).infos.at(k).rows.size();
                ++(aStays ? countedB[g] : counted[g]);
            }
        }
    }
    for (int g = 0; g < counted.size(); ++g) context.work.countUnmarked(CompareWork::Tables, std::max(counted.at(g), countedB.at(g)));
}

// ---- tables put in and taken out

// The row a paragraph is in (told by the record of its cell), or null.
const void *rowOfParagraph(WriteDoc &doc, const QTextBlock &b)
{
    const int key = cellKeyOf(b);
    const flow::CellRecord *rec = key ? doc.flow().tables.get(key) : nullptr;
    return rec && rec->row ? rec->row.get() : nullptr;
}

// The revised table `t` put in whole at `c`, as a person's paste of it is made (tracked as an insertion). False (nothing done) when it can't be copied as a table.
bool putWholeTable(WriteDoc &out, QTextCursor &c, WriteDoc &revised, QTextDocument *rstory, const TableInfo &t)
{
    if (t.rows.isEmpty()) return false;
    const RowInfo &lastRow = t.rows.last();
    const QTextBlock first = rstory->findBlockByNumber(t.rows.first().firstBlock), last = rstory->findBlockByNumber(lastRow.firstBlock + lastRow.blocks - 1);
    if (!first.isValid() || !last.isValid()) return false;
    QTextCursor selection(rstory);
    selection.setPosition(first.position());
    selection.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(revised, selection);
    if (clip.tables.size() != 1 || clip.tablesAsWords > 0) return false;
    pasteClip(out, c, clip);
    return true;
}

// The words of the rows `srcs` of the revised table (top to bottom) put in the empty tracked rows that `insertTableRowRuns` made beside the row of the original's table whose first paragraph `refAt` holds (under it, or
// above it). Returns how many rows got their words (0: the rows beside it are not as made; a row that has not the cells of its source stays empty, and the caller counts it).
int putRows(CompareWork *work, WriteDoc &out, QTextDocument *story, const QTextCursor &refAt, bool below, WriteDoc &revised, QTextDocument *rstory, const QVector<const RowInfo *> &srcs)
{
    const int count = int(srcs.size());
    const QTextBlock ref = story->findBlock(refAt.position());
    const void *refRow = rowOfParagraph(out, ref);
    // The made rows (their paragraphs), top to bottom: the ones under the row follow its last paragraph, the ones above it come before its first.
    QVector<QVector<QTextBlock>> rows;
    QTextBlock x = ref;
    if (below) {
        for (x = ref.next(); x.isValid() && rowOfParagraph(out, x) == refRow; x = x.next()) {}
    } else {
        x = ref.previous();
    }
    for (int n = 0; n < count && x.isValid(); ++n) {
        const void *made = rowOfParagraph(out, x);
        if (!made || made == refRow) return 0;
        QVector<QTextBlock> cells;
        if (below) {
            for (; x.isValid() && rowOfParagraph(out, x) == made; x = x.next()) cells << x;
        } else {
            for (; x.isValid() && rowOfParagraph(out, x) == made; x = x.previous()) cells.prepend(x);
        }
        rows << cells;
    }
    if (rows.size() != count) return 0;
    if (!below) std::reverse(rows.begin(), rows.end());
    const UntrackedEdit untracked;   // (the row says it is put in; what is in its cells is that row's)
    int made = 0;
    for (int ri = 0; ri < count; ++ri) {
        const RowInfo &src = *srcs.at(ri);
        const QVector<QTextBlock> &cells = rows.at(ri);
        if (cells.size() != src.cellParas.size()) continue;   // (the row stays an empty inserted row: it is counted by the caller)
        int n = 0;
        for (int ci = 0; ci < cells.size(); ++ci) {
            QTextCursor cur(story);
            cur.setPosition(cells.at(ci).position());
            for (int k = 0; k < src.cellParas.at(ci); ++k, ++n) {
                const QTextBlock sb = rstory->findBlockByNumber(src.firstBlock + n);
                if (k == 0) {
                    putRevisedWords(out, story, cur, sb, true, work);
                    dressLikeRevised(out, story, story->findBlock(cur.position()).position(), revised, sb);
                } else {
                    Para fake;
                    fake.blockFrom = sb.position();
                    insertRevisedParagraph(out, story, cur, revised, rstory, fake, false, true, work);
                }
            }
        }
        ++made;
    }
    return made;
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
    const TableShapes tablesA = tableShapesOf(*out, story, options), tablesB = tableShapesOf(revised, rstory, options);
    // Which table is which, and what is done with each (see TablePlan): a table of the text is told apart from its partner cell by cell, or row by row, or put in or taken out whole; what can't be marked is counted.
    TablePlan plan;
    bool tablesCut = false, rowsCut = false;
    pairTables(context, story, tablesA, rstory, tablesB, options, isBody, &plan, &tablesCut, &rowsCut);
    if (tablesCut) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too much text to line up the tables, so they are not compared."));
    if (rowsCut) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too many rows and tables changed to mark them all."));
    QVector<int> pairOfA(plan.partnerA.size(), -1), pairOfB(plan.partnerB.size(), -1);
    for (int k = 0; k < plan.partnerA.size(); ++k)
        if (plan.partnerA.at(k) >= 0 && !tablesA.shapes.at(k).startsWith(QLatin1Char('!'))) {
            pairOfA[k] = k;
            pairOfB[plan.partnerA.at(k)] = k;
        }
    bool many1 = false, many2 = false;
    bool looksCut = false;
    QVector<Para> a = paragraphsOf(story, pairOfA, plan.keptA, tablesA, &many1, options.compareFormatting, context, &looksCut);
    const QVector<Para> b = paragraphsOf(rstory, pairOfB, plan.keptB, tablesB, &many2, options.compareFormatting, context, &looksCut);
    if (many1 || many2) return Outcome::TooMany;
    // The paragraphs that are the same in both, in order (the paragraphs of a table, only as the same place of its partner; those of a table or row with no partner are never the same as any).
    auto key = [&](const Para &p, int side) {
        QString k = p.ok ? p.text : QStringLiteral("\x01");
        if (options.ignoreCase) k = k.toLower();
        if (options.ignoreWhiteSpace) k = k.simplified();
        if (p.ok && p.tablePair >= 0) k = QStringLiteral("\x03%1:%2\x1f").arg(p.tablePair).arg(p.cellIndex) + k;
        if (!p.ok && p.kind == Para::Cell) k = QStringLiteral("\x04%1:%2").arg(side).arg(p.blockFrom);
        return k;
    };
    QStringList ka, kb;
    for (const Para &p : a) ka << key(p, 0);
    for (const Para &p : b) kb << key(p, 1);
    QVector<QPair<int, int>> matches;
    MatchBudget budget(context);
    matchParagraphs(ka, kb, 0, int(ka.size()), 0, int(kb.size()), &matches, 24, &budget);
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
        bool whole = false;             // the compare had no room to line the paragraphs of this stretch up: each is taken out or put in whole
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
        if (g.a1 > g.a0 && g.b1 > g.b0 && budget.left.contains({g.a0, g.b0})) {
            g.whole = true;
            context.work.matchLeft += (g.a1 - g.a0) + (g.b1 - g.b0);
        }
        gaps.push_back(g);
    }
    if (context.work.matchLeft > 0) addNote(notes, QCoreApplication::translate("jp::write::Compare", "Too much changed to line up all the paragraphs, so a stretch of them is shown as taken out and put in whole. The new text keeps its styles, lists, bold, and italic, but not its text colors, shading, or the spacing between paragraphs."));
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
        auto classOf = [](const Para &p, int side) { return p.ok ? p.tablePair : p.kind == Para::Cell ? -3 - side : -2; };   // (what can't be compared is paired only with the same, and a table or row with no partner with none)
        if (!g.whole) {
            for (const int i : std::as_const(g.aOnly)) byClassA[classOf(a.at(i), 0)] << i;
            for (const int j : std::as_const(g.bOnly)) byClassB[classOf(b.at(j), 1)] << j;
        }
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
    // The tables of the original that are changed by rows, or taken out whole: held at the first paragraph of each row that is a place of the changes (the places follow the edits made to the text).
    struct TableJob {
        bool whole = false;
        RowEdit edit;
        QMap<int, QTextCursor> rowAt;
    };
    QVector<TableJob> jobs;
    {
        auto cursorAt = [&](const TableInfo &t, int row) {
            QTextCursor c(story);
            c.setPosition(story->findBlockByNumber(t.rows.at(row).firstBlock).position());
            return c;
        };
        for (int k = 0; k < plan.stateA.size(); ++k)
            if (plan.stateA.at(k) == TablePlan::Whole) {
                TableJob job;
                job.whole = true;
                job.edit.a = k;
                job.rowAt.insert(0, cursorAt(tablesA.infos.at(k), 0));
                jobs << job;
            }
        for (const RowEdit &e : std::as_const(plan.edits)) {
            TableJob job;
            job.edit = e;
            const TableInfo &t = tablesA.infos.at(e.a);
            for (const int r : e.deleted) job.rowAt.insert(r, cursorAt(t, r));
            for (const RowEdit::Put &put : e.put)
                if (!job.rowAt.contains(put.ref)) job.rowAt.insert(put.ref, cursorAt(t, put.ref));
            jobs << job;
        }
    }
    QSet<int> putTables;   // the tables of the revised text that are put in (by order)
    // From the last gap to the first, so that a place found before an edit is still its place after the edits after it.
    for (int k = int(gaps.size()) - 1; k >= 0; --k) {
        const Gap &g = gaps.at(k);
        if (g.a1 == g.a0 && g.b1 == g.b0) continue;
        if (progress) {
            const int d = (*doneTotal)++;
            if (!progress(d, d + places + storiesLeft)) return Outcome::Canceled;
        }
        // A stretch the compare had no room to line up (see MatchBudget) that holds only paragraphs of words: all its paragraphs are taken out with one deletion and the revised stretch is put in with one insertion
        // (a paragraph at a time, every edit looks through the whole text for what is locked, and the work grows with the square of the text). The new words have no look or style of the revised paragraphs'.
        if (g.whole && g.bLeft.size() == g.b1 - g.b0 && g.aLeft.size() == g.a1 - g.a0) {
            auto plain = [&](const QVector<Para> &list, int i, const QVector<int> &moved, const TableShapes &tables) {
                return list.at(i).ok && list.at(i).kind == Para::Normal && list.at(i).tablePair < 0 && moved.at(i) < 0 && tables.tableOfBlock.value(i, -1) < 0;
            };
            bool all = true;
            for (int i = g.a0; i < g.a1 && all; ++i) all = plain(a, i, movedA, tablesA);
            for (int j = g.b0; j < g.b1 && all; ++j) all = plain(b, j, movedB, tablesB);
            all = all && tablesA.tableOfBlock.value(g.a0 - 1, -1) < 0 && tablesA.tableOfBlock.value(g.a1, -1) < 0;
            if (all) {
                QString text;
                for (int j = g.b0; j < g.b1; ++j) {
                    text += QLatin1Char('\n') + b.at(j).text;
                    countDroppedIn(&context.work, rstory->findBlock(b.at(j).blockFrom));
                }
                QTextCursor c(story);
                c.setPosition(a.at(g.a1 - 1).blockEnd);
                insertText(*out, c, text);
                const QTextBlock first = story->findBlock(a.at(g.a0).blockFrom), last = story->findBlock(a.at(g.a1 - 1).blockFrom);
                QTextCursor d(story);
                d.setPosition(first.position());
                d.setPosition(last.next().position(), QTextCursor::KeepAnchor);
                removeSelection(*out, d);
                made += 2;
                continue;
            }
        }
        // The paragraphs that are only in the revised version (or are moved here), put after the last paragraph of the gap (or after the one before it).
        QVector<int> inserts = g.bLeft;
        for (int j = g.b0; j < g.b1; ++j)
            if (movedB.at(j) >= 0) inserts << j;
        std::sort(inserts.begin(), inserts.end());
        QTextCursor preAt(story);
        QVector<int> preInserts;
        if (!inserts.isEmpty()) {
            QTextCursor c(story);
            const int after = g.a1 - 1;
            auto inTable = [&](int i) { return i >= 0 && i < a.size() && tablesA.tableOfBlock.value(i, -1) >= 0; };
            bool putsTable = false;   // (a table that is only in the revised text is among them)
            for (const int t : std::as_const(inserts))
                if (!b.at(t).ok && b.at(t).kind == Para::Cell && b.at(t).tableOrd >= 0 && plan.stateB.value(b.at(t).tableOrd) == TablePlan::Whole && !putTables.contains(b.at(t).tableOrd)) putsTable = true;
            // After the paragraph before the gap; but not inside a table (right after a table's last cell, or at the start of a table): then before the paragraph the gap ends with, if that is not in a table.
            // A table put in goes before the paragraph the gap ends with (as a person's paste at its start does), when there is one.
            bool atStart = after < 0, placed = true;
            bool holdMode = false;   // what is put in goes before the paragraph the gap ends with
            if (putsTable && g.a1 < a.size() && !inTable(g.a1)) {
                atStart = true;
                holdMode = true;
                c.setPosition(a.at(g.a1).blockFrom);
            } else if (after >= 0 && !inTable(after)) {
                c.setPosition(a.at(after).blockEnd);
            } else {
                const int next = after >= 0 ? g.a1 : 0;
                atStart = true;
                if (next < a.size() && !inTable(next)) {
                    c.setPosition(a.at(next).blockFrom);
                    holdMode = true;
                } else {
                    placed = false;
                }
            }
            bool fillLast = false;   // the empty paragraph a table put in at the end of the text leaves is where the next paragraph's words go
            // Paragraphs put in before a table (of the revised text) in a stretch that holds a table of the original go before that table, after the paragraph before it (and not after the whole table, where the
            // end of the stretch is).
            int firstATable = -1, firstBTable = -1;
            for (int i = g.a0; i < g.a1 && firstATable < 0; ++i)
                if (inTable(i)) firstATable = i;
            for (int j = g.b0; j < g.b1 && firstBTable < 0; ++j)
                if (tablesB.tableOfBlock.value(j, -1) >= 0) firstBTable = j;
            const int beforeTable = firstATable - 1;   // (the paragraph before it)
            const bool havePre = firstATable >= 0 && firstBTable >= 0 && beforeTable >= 0 && !inTable(beforeTable);
            bool preCounted = false;
            if (havePre) preAt.setPosition(a.at(beforeTable).blockEnd);
            for (const int t : std::as_const(inserts)) {
                if (firstATable >= 0 && firstBTable >= 0 && t < firstBTable && b.at(t).ok && movedB.at(t) < 0 && b.at(t).tablePair < 0) {
                    if (!havePre) {   // (the table starts the text: nothing can go before it)
                        if (!preCounted) context.work.countUnmarked(CompareWork::Tables);
                        preCounted = true;
                    } else {
                        preInserts << t;   // (put in after the edits of the stretch: the places of the paragraphs the edits are in are those before any of them)
                    }
                    continue;
                }
                if (!b.at(t).ok) {
                    if (b.at(t).kind == Para::Raw) {
                        context.work.countUnmarked(CompareWork::KeptAsWritten);   // (a paragraph that stands for something kept as it was written)
                    } else if (b.at(t).kind == Para::Cell && b.at(t).tableOrd >= 0 && plan.stateB.value(b.at(t).tableOrd) == TablePlan::Whole && !putTables.contains(b.at(t).tableOrd)) {
                        // (A table that is only in the revised text: put in whole, at the place of its first paragraph among the ones put in.)
                        putTables.insert(b.at(t).tableOrd);
                        if (holdMode) c.setPosition(c.block().position());   // (the start of the paragraph itself, before the marks of the changes that were put in it)
                        if (placed && putWholeTable(*out, c, revised, rstory, tablesB.infos.at(b.at(t).tableOrd))) {
                            ++context.work.tablesPut;
                            ++made;
                            if (!holdMode) {   // (put in at the end of a paragraph: the empty paragraph the table leaves, at the end of the text or before another table, is where the next paragraph's words go)
                                fillLast = c.block().length() == 1 && cellKeyOf(c.block()) == 0;
                                atStart = false;
                            }
                        } else {
                            context.work.countUnmarked(CompareWork::Tables);
                        }
                    }
                    continue;
                }
                if (!placed || b.at(t).tablePair >= 0) {   // (no place that is not in a table, or a paragraph of a table: not put in)
                    context.work.countUnmarked(CompareWork::Tables);
                    continue;
                }
                if (fillLast && movedB.at(t) < 0) {
                    fillLast = false;
                    const QTextBlock src = rstory->findBlock(b.at(t).blockFrom);
                    putRevisedWords(*out, story, c, src, false, &context.work);
                    dressLikeRevised(*out, story, story->findBlock(c.position()).position(), revised, src);
                    ++made;
                    continue;
                }
                const int at = insertRevisedParagraph(*out, story, c, revised, rstory, b.at(t), atStart, false, &context.work);
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
        QVector<QTextCursor> beforeTables;
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
                    if (plan.partnerA.value(before, -1) >= 0 || plan.stateA.value(before) == TablePlan::Whole) context.work.countUnmarked(CompareWork::Tables);   // (after a table that is not marked, it is that table's difference, counted already)
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
                    // (A paragraph before a table: its mark is not one a deletion records, as nothing joins a paragraph to a table; it is marked as taken out all the same, after the paragraphs before it are dealt
                    // with, and Accept All takes the empty paragraph out.)
                    if (!lastWay && blk.next().isValid() && cellKeyOf(blk.next()) && !cellKeyOf(blk)) beforeTables.push_back(QTextCursor(story->findBlock(a.at(t).blockFrom)));
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
        for (const QTextCursor &at : std::as_const(beforeTables)) {
            const QTextBlock blk = at.block();
            if (blk.isValid() && !markFormat(blk).hasProperty(prop::TrackedDeleted)) markParagraphMark(*out, blk, "del");
        }
        for (const int t : std::as_const(preInserts)) {
            insertRevisedParagraph(*out, story, preAt, revised, rstory, b.at(t), false, false, &context.work);
            ++made;
        }
    }
    // The tables of the original that are taken out whole, and the rows of the ones told apart by row that are taken out and put in (the rows taken out first; the places held above follow every edit made).
    for (int ji = int(jobs.size()) - 1; ji >= 0; --ji) {   // (the last table first: a change of a table leaves the tables after it as they were)
        TableJob &job = jobs[ji];
        if (job.whole) {
            QTextCursor c = job.rowAt.value(0);
            deleteTable(*out, c);
            ++context.work.tablesTaken;
            ++made;
            continue;
        }
        const QVector<int> &gone = job.edit.deleted;
        for (int hi = int(gone.size()) - 1; hi >= 0;) {   // (the runs of rows that go, the last first)
            int lo = hi;
            while (lo > 0 && gone.at(lo - 1) == gone.at(lo) - 1) --lo;
            QTextCursor c(story);
            const QTextBlock lastFirst = story->findBlock(job.rowAt.value(gone.at(hi)).position());   // (the first paragraph of the last row: the rows between the two cursors' paragraphs go)
            c.setPosition(job.rowAt.value(gone.at(lo)).position());
            c.setPosition(lastFirst.position() + lastFirst.length() - 1, QTextCursor::KeepAnchor);
            deleteTableRows(*out, c);
            made += hi - lo + 1;
            hi = lo - 1;
        }
        const TableInfo &partner = tablesB.infos.at(job.edit.b);
        const QVector<RowEdit::Put> &put = job.edit.put;
        // The runs of rows that go beside the same row (those under it go in order, as do those above it), made all together, from the last: one command, one layout, one rebuild of the table's index for the table.
        struct Run {
            int lo, hi;
        };
        QVector<Run> runs;
        QVector<RowRun> rowRuns;
        for (int lo = 0; lo < put.size();) {
            int hi = lo;
            while (hi + 1 < put.size() && put.at(hi + 1).ref == put.at(lo).ref && put.at(hi + 1).below == put.at(lo).below) ++hi;
            runs.push_back({lo, hi});
            rowRuns.push_back({job.rowAt.value(put.at(lo).ref), put.at(lo).below, hi - lo + 1});
            lo = hi + 1;
        }
        const WriteDoc::StepGroup oneStep(*out);   // (the rows and their words are one step of the history)
        const bool inserted = insertTableRowRuns(*out, rowRuns);
        ++context.work.rowRunBatches;
        // (The words of every row go in one edit, as a person's paste is one: the pages are told once.)
        QTextCursor batch(story);
        batch.beginEditBlock();
        const NoLockedContent noLocked(story);
        for (const Run &run : std::as_const(runs)) {
            QVector<const RowInfo *> srcs;   // (top to bottom)
            for (int t = run.lo; t <= run.hi; ++t) srcs.push_back(&partner.rows.at(put.at(t).row));
            const int madeRows = inserted ? putRows(&context.work, *out, story, job.rowAt.value(put.at(run.lo).ref), put.at(run.lo).below, revised, rstory, srcs) : 0;
            made += madeRows;
            for (int t = madeRows; t < int(srcs.size()); ++t) context.work.countUnmarked(CompareWork::Tables);
        }
        batch.endEditBlock();
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

// A list of names as words: "a", "a and b", "a, b, and c".
QString joinedKinds(QStringList names)
{
    if (names.size() < 2) return names.value(0);
    const QString last = names.takeLast();
    if (names.size() == 1) return QCoreApplication::translate("jp::write::Compare", "%1 and %2").arg(names.first(), last);
    return QCoreApplication::translate("jp::write::Compare", "%1, and %2").arg(names.join(QLatin1String(", ")), last);
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
    return joinedKinds(names);
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
    qint64 dropped = 0;
    QStringList droppedNames;
    for (int k = 0; k < CompareWork::DroppedKinds; ++k) {
        if (context.work.dropped[k] <= 0) continue;
        dropped += context.work.dropped[k];
        switch (k) {
        case CompareWork::NoteMarks: droppedNames << QCoreApplication::translate("jp::write::Compare", "footnote and endnote marks"); break;
        case CompareWork::Fields: droppedNames << QCoreApplication::translate("jp::write::Compare", "fields"); break;
        case CompareWork::Pictures: droppedNames << QCoreApplication::translate("jp::write::Compare", "pictures"); break;
        case CompareWork::Links: droppedNames << QCoreApplication::translate("jp::write::Compare", "links"); break;
        default: droppedNames << QCoreApplication::translate("jp::write::Compare", "parts kept as they were written"); break;
        }
    }
    if (dropped > 0)
        notes.push_back(QCoreApplication::translate("jp::write::Compare", "%n part of the new text could not be shown (%1).", nullptr, int(dropped)).arg(joinedKinds(droppedNames)));
    if (context.work.notMarked > 0)
        notes.push_back(QCoreApplication::translate("jp::write::Compare", "%n difference is in a place that Compare can't mark yet (%1), so it is not shown.", nullptr, int(context.work.notMarked)).arg(kindsOfUnmarked(context.work)));
    if (progress) progress(done, done);
    if (changes) *changes = made;
    if (note) *note = notes.join(QLatin1Char(' ')).trimmed();
    return out;
}

} // namespace jp::write
