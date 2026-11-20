// Tracked changes: finding, accepting, and rejecting them.

#include "core/reviewops.h"

#include "core/commentops.h"
#include "core/editops.h"
#include "core/pictureops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QMap>
#include <QTextDocument>
#include <QTimeZone>
#include <algorithm>
#include <atomic>

using namespace jp::office::wml;

namespace jp::write {

namespace {

std::atomic<qint64> s_blocksScanned{0}, s_runsVisited{0};
thread_local bool t_withEndnotes = false;   // (the words of the endnotes the pages set, which the changes in them are looked for in, while Accept All and Reject All go through them)

// A paragraph of an endnote the pages set is out of the changes looked for, but while the endnotes are in scope (Accept All, and the list of the changes of the whole document).
bool outOfScope(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    return bf.hasProperty(prop::EndnoteBlock) && !(t_withEndnotes && bf.intProperty(prop::EndnoteNote) > 0);
}
struct EndnoteScope {
    bool was = t_withEndnotes;
    EndnoteScope() { t_withEndnotes = true; }
    ~EndnoteScope() { t_withEndnotes = was; }
};

bool isRevisionTag(const QString &tag)
{
    return tag == QLatin1String("ins") || tag == QLatin1String("del") || tag == QLatin1String("moveFrom") || tag == QLatin1String("moveTo");
}

int revisionKindOf2(const QString &tag)
{
    return tag == QLatin1String("ins") ? 1 : tag == QLatin1String("del") ? 2 : tag == QLatin1String("moveFrom") ? 3 : tag == QLatin1String("moveTo") ? 4 : 0;
}

int runChildRankOf(const char *name) { return runChildRank(QByteArray(name)); }

Revision::Kind kindOfTag(const QString &tag)
{
    return tag == QLatin1String("ins") ? Revision::Insertion : tag == QLatin1String("del") ? Revision::Deletion : tag == QLatin1String("moveFrom") ? Revision::MovedFrom : Revision::MovedTo;
}

// The local name of the element a piece of kept XML starts with.
QByteArray elementName(const QByteArray &xml)
{
    if (xml.size() < 2 || xml[0] != '<') return QByteArray();
    qsizetype e = 1;
    while (e < xml.size() && xml[e] != ' ' && xml[e] != '/' && xml[e] != '>' && xml[e] != '\r' && xml[e] != '\n' && xml[e] != '\t') ++e;
    const QByteArray qname = xml.mid(1, e - 1);
    return qname.mid(qname.indexOf(':') + 1);
}

// An attribute of a start tag kept as written (any prefix).
QString attributeOf(const QByteArray &xml, const char *name)
{
    const qsizetype end = xml.indexOf('>');
    const QByteArray tag = end < 0 ? xml : xml.left(end);
    qsizetype from = -1;
    const QByteArray prefixed = QByteArray(":") + name + "=\"";
    if (const qsizetype at = tag.indexOf(prefixed); at >= 0) from = at + prefixed.size();
    if (from < 0) {
        const QByteArray plain = QByteArray(" ") + name + "=\"";
        if (const qsizetype at = tag.indexOf(plain); at >= 0) from = at + plain.size();
    }
    if (from < 0) return QString();
    const qsizetype to = tag.indexOf('"', from);
    if (to < 0) return QString();
    QString v = QString::fromUtf8(tag.mid(from, to - from));
    if (v.contains(QLatin1Char('&'))) v.replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&amp;"), QLatin1String("&"));
    return v;
}

QDateTime parseDate(const QString &s)
{
    if (s.isEmpty()) return QDateTime();
    QDateTime d = QDateTime::fromString(s, Qt::ISODate);
    if (d.isValid() && d.timeSpec() == Qt::LocalTime) d.setTimeZone(QTimeZone::UTC);
    return d;
}

QByteArray escapeAttribute(const QString &s)
{
    QByteArray out;
    for (const QChar c : s) {
        if (c == QLatin1Char('&')) out += "&amp;";
        else if (c == QLatin1Char('<')) out += "&lt;";
        else if (c == QLatin1Char('>')) out += "&gt;";
        else if (c == QLatin1Char('"')) out += "&quot;";
        else if (c.unicode() < 0x20) continue;
        else out += QString(c).toUtf8();
    }
    return out;
}

// A piece of the paragraph that begins or ends a change.
struct Piece {
    int pos = 0;
    bool open = false;
    QString tag;
    QByteArray raw;
    QString id;
};

QVector<Piece> piecesOf(const QTextBlock &b)
{
    QVector<Piece> out;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        if (!cf.hasProperty(prop::Atom)) continue;
        const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
        if (a.kind != Inline::ParaAtom || (a.role != Inline::Open && a.role != Inline::Close) || !isRevisionTag(a.tag)) continue;
        const QString t = f.text();
        for (int k = 0; k < t.size(); ++k)
            if (t[k] == QChar::ObjectReplacementCharacter) out.push_back({f.position() + k, a.role == Inline::Open, a.tag, a.raw, a.key});
    }
    return out;
}

struct Pair {
    int open = 0, close = 0;
    QString tag;
    QByteArray raw;
    QString id;
};

QVector<Pair> pairsOf(const QVector<Piece> &pieces)
{
    QVector<Pair> out;
    QVector<int> stack;
    for (int i = 0; i < pieces.size(); ++i) {
        if (pieces[i].open) {
            if (stack.size() < 4096) stack.push_back(i);
        } else {
            int at = int(stack.size()) - 1;
            while (at >= 0 && pieces[stack[at]].tag != pieces[i].tag) --at;
            if (at < 0) continue;
            const Piece &o = pieces[stack[at]];
            out.push_back({o.pos, pieces[i].pos, o.tag, o.raw, o.id});
            stack.resize(at);
        }
    }
    return out;
}

QVector<RawChild> extraOf(const QTextFormat &f, int property)
{
    return f.hasProperty(property) ? unpackRaw(storedBytes(f, property)) : QVector<RawChild>();
}

// The records of tracked changes among the kept properties of a run, a paragraph, or a mark: ins, del, moveFrom, moveTo of a mark; rPrChange; pPrChange.
const RawChild *recordOf(const QVector<RawChild> &extra, const char *name)
{
    for (const RawChild &c : extra)
        if (elementName(c.xml) == name) return &c;
    return nullptr;
}

// The tracked marks among the rest of a paragraph's numbering (the elements of w:numPr after the level and the list, kept as written): w:ins, which says the numbering was added while changes
// were tracked (section 17.13.5.19), and w:numberingChange, which says it was changed and keeps what it was as a label in `original` (Part 4, section 14.7.1.2; it names no list to go back to).
struct NumberingMark {
    qsizetype at = 0, end = 0;
    Revision::Kind kind = Revision::NumberingInserted;
    QByteArray xml;
};

QVector<NumberingMark> numberingMarksOf(const QByteArray &extra)
{
    QVector<NumberingMark> out;
    for (const char *name : {"<w:ins", "<w:numberingChange"}) {
        const qsizetype len = qstrlen(name);
        for (qsizetype at = extra.indexOf(name); at >= 0 && out.size() < 64; at = extra.indexOf(name, at + len)) {
            const char next = at + len < extra.size() ? extra.at(at + len) : '\0';
            if (next != ' ' && next != '/' && next != '>' && next != '\t' && next != '\n' && next != '\r') continue;
            const qsizetype gt = extra.indexOf('>', at);
            if (gt < 0) continue;
            qsizetype end = gt + 1;
            if (extra.at(gt - 1) != '/') {
                const QByteArray close = QByteArray("</") + (name + 1) + ">";
                const qsizetype c = extra.indexOf(close, gt);
                if (c < 0) continue;
                end = c + close.size();
            }
            out.push_back({at, end, name[3] == 'i' ? Revision::NumberingInserted : Revision::NumberingChanged, extra.mid(at, end - at)});
        }
    }
    std::sort(out.begin(), out.end(), [](const NumberingMark &a, const NumberingMark &b) { return a.at < b.at; });
    return out;
}

void addRevisionsOf(const QTextBlock &b, QVector<Revision> *out)
{
    ++s_blocksScanned;
    const int blockNo = b.blockNumber();
    const QVector<Piece> pieces = piecesOf(b);
    for (const Pair &p : pairsOf(pieces)) {
        Revision r;
        r.kind = kindOfTag(p.tag);
        r.id = attributeOf(p.raw, "id");
        r.author = attributeOf(p.raw, "author");
        r.date = parseDate(attributeOf(p.raw, "date"));
        r.from = p.open;
        r.to = p.close + 1;
        r.block = blockNo;
        out->push_back(r);
    }
    // A change of a run's properties: the record is in the kept properties of each run it covers.
    Revision *open = nullptr;
    QVector<Revision> formats;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        if (!cf.hasProperty(prop::RawRPr) || !storedBytes(cf, prop::RawRPr).contains("rPrChange")) {
            open = nullptr;
            continue;
        }
        const QVector<RawChild> extra = extraOf(cf, prop::RawRPr);
        const RawChild *rec = recordOf(extra, "rPrChange");
        if (!rec) {
            open = nullptr;
            continue;
        }
        const QString id = attributeOf(rec->xml, "id");
        if (open && open->id == id && open->to == f.position()) {
            open->to = f.position() + f.length();
            continue;
        }
        Revision r;
        r.kind = Revision::Format;
        r.id = id;
        r.author = attributeOf(rec->xml, "author");
        r.date = parseDate(attributeOf(rec->xml, "date"));
        r.from = f.position();
        r.to = f.position() + f.length();
        r.block = blockNo;
        formats.push_back(r);
        open = &formats.last();
    }
    *out += formats;
    const QTextBlockFormat bf = b.blockFormat();
    if (bf.hasProperty(prop::NumExtra)) {
        for (const NumberingMark &m : numberingMarksOf(storedBytes(bf, prop::NumExtra))) {
            Revision r;
            r.kind = m.kind;
            r.id = attributeOf(m.xml, "id");
            r.author = attributeOf(m.xml, "author");
            r.date = parseDate(attributeOf(m.xml, "date"));
            r.from = b.position();
            r.to = b.position() + b.length() - 1;
            r.block = blockNo;
            out->push_back(r);
        }
    }
    if (bf.hasProperty(prop::RawPPr)) {
        const QVector<RawChild> extra = extraOf(bf, prop::RawPPr);
        if (const RawChild *rec = recordOf(extra, "pPrChange")) {
            Revision r;
            r.kind = Revision::ParagraphFormat;
            r.id = attributeOf(rec->xml, "id");
            r.author = attributeOf(rec->xml, "author");
            r.date = parseDate(attributeOf(rec->xml, "date"));
            r.from = b.position();
            r.to = b.position() + b.length() - 1;
            r.block = blockNo;
            out->push_back(r);
        }
    }
    // A change of a section's properties (section 17.13.5.32): the record is among the kept properties of the section, which a paragraph that ends a section holds, and the last section's the
    // first paragraph holds; it is listed at the paragraph that ends the section, and at the last paragraph of the text for the last section.
    {
        auto listSection = [&](const QByteArray &packed) {
            if (!packed.contains("sectPrChange")) return;
            const SectionProps props = unpackSection(packed);
            const RawChild *rec = recordOf(props.extra, "sectPrChange");
            if (!rec) return;
            Revision r;
            r.kind = Revision::SectionFormat;
            r.id = attributeOf(rec->xml, "id");
            r.author = attributeOf(rec->xml, "author");
            r.date = parseDate(attributeOf(rec->xml, "date"));
            r.from = b.position();
            r.to = b.position() + b.length() - 1;
            r.block = blockNo;
            out->push_back(r);
        };
        if (bf.boolProperty(flow::fp::SectionBreakAfter) && bf.hasProperty(prop::SectionData)) listSection(storedBytes(bf, prop::SectionData));
        const QTextBlock first = b.document()->begin();
        if (first.blockFormat().hasProperty(prop::LastSection) && !bf.hasProperty(prop::EndnoteBlock) && (!b.next().isValid() || b.next().blockFormat().hasProperty(prop::EndnoteBlock)))
            listSection(storedBytes(first.blockFormat(), prop::LastSection));
    }
    // The paragraph's mark, inserted or deleted.
    const QTextCharFormat mark = markFormat(b);
    if (mark.hasProperty(prop::RawRPr)) {
        const QVector<RawChild> extra = extraOf(mark, prop::RawRPr);
        for (const char *name : {"ins", "del", "moveFrom", "moveTo"}) {
            const RawChild *rec = recordOf(extra, name);
            if (!rec) continue;
            Revision r;
            r.kind = qstrcmp(name, "ins") == 0 || qstrcmp(name, "moveTo") == 0 ? Revision::MarkInserted : Revision::MarkDeleted;
            r.id = attributeOf(rec->xml, "id");
            r.author = attributeOf(rec->xml, "author");
            r.date = parseDate(attributeOf(rec->xml, "date"));
            r.from = b.position() + b.length() - 1;
            r.to = r.from + 1;
            r.block = blockNo;
            r.moved = name[0] == 'm';
            out->push_back(r);
        }
    }
    // What a table records at this paragraph (it begins a cell, and may begin a row and the table).
    if (b.blockFormat().hasProperty(flow::fp::Cell)) {
        const QVariant store = b.document()->property(kTableStoreProperty);
        if (store.isValid()) {
            QVector<TableChange> changes;
            tableChangesAt(*reinterpret_cast<const flow::TableStore *>(store.value<quintptr>()), b, &changes);
            static const Revision::Kind kinds[] = {Revision::RowInserted, Revision::RowDeleted, Revision::CellInserted, Revision::CellDeleted, Revision::CellMerged, Revision::TableFormat, Revision::RowFormat, Revision::CellFormat};
            for (const TableChange &c : std::as_const(changes)) {
                Revision r;
                r.kind = kinds[c.kind];
                r.id = attributeOf(c.record, "id");
                r.author = attributeOf(c.record, "author");
                r.date = parseDate(attributeOf(c.record, "date"));
                r.from = c.from;
                r.to = c.to;
                r.block = blockNo;
                out->push_back(r);
            }
        }
    }
}

QTextCharFormat asDeletedFormat(QTextCharFormat format)
{
    format.setProperty(prop::TrackedDeleted, format.boolProperty(tp::HiddenText) ? prop::HiddenAlready : prop::HiddenByDeletion);
    format.setProperty(tp::HiddenText, true);
    return format;
}

QTextCharFormat notDeletedFormat(QTextCharFormat format)
{
    if (format.hasProperty(prop::TrackedDeleted)) {
        if (format.intProperty(prop::TrackedDeleted) == prop::HiddenByDeletion) format.clearProperty(tp::HiddenText);
        format.clearProperty(prop::TrackedDeleted);
    }
    return format;
}

} // namespace

QTextCharFormat withRevisionLook(QTextCharFormat f, bool deleted, int kind, int author, bool alsoInserted)
{
    if (deleted && !f.hasProperty(prop::TrackedDeleted)) f = asDeletedFormat(f);
    else if (!deleted && f.hasProperty(prop::TrackedDeleted)) f = notDeletedFormat(f);
    if (kind > 0 && !((kind == 2 || kind == 3) && f.intProperty(prop::TrackedDeleted) == prop::HiddenAlready)) {
        f.setProperty(tp::Revision, kind);
        f.setProperty(tp::RevisionAuthor, author);
        if (alsoInserted && (kind == 2 || kind == 3)) f.setProperty(tp::RevisionInserted, true);
        else f.clearProperty(tp::RevisionInserted);
    } else {
        f.clearProperty(tp::Revision);
        f.clearProperty(tp::RevisionAuthor);
        f.clearProperty(tp::RevisionInserted);
    }
    return f;
}

QString revisionAuthorOf(const QByteArray &startTag) { return attributeOf(startTag, "author"); }

QByteArray revisionStartTag(WriteDoc &doc, const QString &tag)
{
    const QString date = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ"));
    return "<w:" + tag.toLatin1() + " w:id=\"" + QByteArray::number(doc.nextRevisionId()) + "\" w:author=\"" + escapeAttribute(doc.reviewer()) + "\" w:date=\"" + date.toLatin1() + "\">";
}

QTextCharFormat revisionPiece(const QByteArray &startTag, const QString &tag, bool open, const QTextCharFormat &like)
{
    Inline a;
    a.kind = Inline::ParaAtom;
    a.role = open ? Inline::Open : Inline::Close;
    a.tag = tag;
    a.key = attributeOf(startTag, "id");
    a.raw = open ? startTag : "</w:" + tag.toLatin1() + ">";
    QTextCharFormat f = like;
    f.clearProperty(tp::Revision);
    f.clearProperty(tp::RevisionAuthor);
    f.clearProperty(tp::RevisionFormat);
    f.clearProperty(prop::RawRPr);
    f.clearProperty(tp::InlineObject);
    f.clearProperty(flow::fp::AnchorId);
    setStored(f, prop::Atom, packAtom(a));
    f.setProperty(tp::Field, QStringLiteral("raw:"));
    return f;
}

QVector<std::pair<QString, QByteArray>> openRevisionsAt(const QTextBlock &block, int position)
{
    const QVector<Piece> pieces = piecesOf(block);
    QVector<std::pair<QString, QByteArray>> open;
    QVector<Piece> stack;
    for (const Piece &p : pieces) {
        if (p.pos >= position) break;
        if (p.open) {
            stack.push_back(p);
        } else {
            int at = int(stack.size()) - 1;
            while (at >= 0 && stack[at].tag != p.tag) --at;
            if (at >= 0) stack.resize(at);
        }
    }
    for (const Piece &p : stack) open.push_back({p.tag, p.raw});
    return open;
}

namespace {

// The order the changes are kept in: where they begin, then the longer (outer) before the shorter, then their kind, then their Id.
bool revisionBefore(const Revision &a, const Revision &b)
{
    if (a.from != b.from) return a.from < b.from;
    if (a.to != b.to) return a.to > b.to;
    if (a.kind != b.kind) return a.kind < b.kind;
    const int c = QString::compare(a.id, b.id);
    return c != 0 ? c < 0 : a.author < b.author;
}

} // namespace

QVector<Revision> revisionsIn(const QTextDocument *story, int from, int to, int most)
{
    QVector<Revision> out;
    QTextBlock b = story->findBlock(std::max(0, from));
    for (; b.isValid() && (to < 0 || b.position() <= to) && out.size() < most; b = b.next()) {
        if (outOfScope(b)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        for (const Revision &r : std::as_const(inBlock)) {
            if (r.to < from || (to >= 0 && r.from > to)) continue;
            out.push_back(r);
        }
    }
    std::stable_sort(out.begin(), out.end(), revisionBefore);
    if (out.size() > most) out.resize(most);
    return out;
}

int revisionCount(const QTextDocument *story)
{
    int n = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (outOfScope(b)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        n += int(inBlock.size());
    }
    return n;
}

std::optional<Revision> revisionAt(const QTextDocument *story, int position)
{
    const QTextBlock b = story->findBlock(position);
    if (!b.isValid()) return std::nullopt;
    QVector<Revision> inBlock;
    addRevisionsOf(b, &inBlock);
    std::optional<Revision> best;
    for (const Revision &r : std::as_const(inBlock)) {
        const bool inside = r.kind == Revision::MarkInserted || r.kind == Revision::MarkDeleted ? position == r.from : position >= r.from && position <= r.to;
        if (!inside) continue;
        // (A change of the whole paragraph's properties is the one at a place only where nothing narrower is.)
        const auto wide = [](Revision::Kind k) { return k == Revision::ParagraphFormat || k == Revision::NumberingInserted || k == Revision::NumberingChanged || k == Revision::SectionFormat; };
        if (wide(r.kind) && best) continue;
        if (!best || (r.to - r.from) < (best->to - best->from) || wide(best->kind)) best = r;
    }
    return best;
}

namespace {

// The first change (in the order they are kept in) that `wanted` says is after, going from the paragraph at `start` towards the end; none where there is none.
template <typename After>
std::optional<Revision> firstAfter(const QTextDocument *story, int start, After wanted)
{
    for (QTextBlock b = story->findBlock(std::max(0, start)); b.isValid(); b = b.next()) {
        if (outOfScope(b)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        std::optional<Revision> best;
        for (const Revision &r : std::as_const(inBlock))
            if (wanted(r) && (!best || revisionBefore(r, *best))) best = r;
        if (best) return best;
    }
    return std::nullopt;
}

template <typename Before>
std::optional<Revision> lastBefore(const QTextDocument *story, int start, Before wanted)
{
    for (QTextBlock b = story->findBlock(std::min(start, story->characterCount() - 1)); b.isValid(); b = b.previous()) {
        if (outOfScope(b)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        std::optional<Revision> best;
        for (const Revision &r : std::as_const(inBlock))
            if (wanted(r) && (!best || revisionBefore(*best, r))) best = r;
        if (best) return best;
    }
    return std::nullopt;
}

} // namespace

std::optional<Revision> nextRevision(const QTextDocument *story, int position)
{
    return firstAfter(story, position, [&](const Revision &r) { return r.from > position; });
}

std::optional<Revision> previousRevision(const QTextDocument *story, int position)
{
    return lastBefore(story, position, [&](const Revision &r) { return r.from < position; });
}

std::optional<Revision> nextRevision(const QTextDocument *story, const Revision &after)
{
    return firstAfter(story, after.from, [&](const Revision &r) { return revisionBefore(after, r); });
}

std::optional<Revision> previousRevision(const QTextDocument *story, const Revision &before)
{
    return lastBefore(story, before.from, [&](const Revision &r) { return revisionBefore(r, before); });
}

QString revisionText(const QTextDocument *story, const Revision &r, int most)
{
    if (r.kind == Revision::Format || r.kind == Revision::ParagraphFormat || r.kind == Revision::NumberingInserted || r.kind == Revision::NumberingChanged || r.kind == Revision::SectionFormat || r.kind == Revision::MarkInserted || r.kind == Revision::MarkDeleted) return QString();
    // Read from the place it begins, a character at a time (each found in the document's tree, not by going along the runs before it), and no further than the words need (`most`).
    const int end = std::max(0, std::min({r.to, r.from + most * 2 + 64, story->characterCount() - 1}));
    QString out;
    for (int at = std::max(0, std::min(r.from, end)); at < end; ++at) {
        ++s_runsVisited;
        out += story->characterAt(at);
    }
    out.remove(QChar::ObjectReplacementCharacter);
    out.replace(QChar::ParagraphSeparator, QLatin1Char(' '));
    return out.left(most);
}

qint64 revisionBlocksScannedForTest() { return s_blocksScanned.load(); }
qint64 revisionRunsVisitedForTest() { return s_runsVisited.load(); }

QStringList revisionAuthors(const QTextDocument *story)
{
    QStringList out;
    for (const Revision &r : revisionsIn(story))
        if (!r.author.isEmpty() && !out.contains(r.author)) out.push_back(r.author);
    return out;
}

// ---- looks

void refreshRevisionLook(WriteDoc &doc, QTextBlock block)
{
    if (!block.isValid()) return;
    QTextDocument *d = const_cast<QTextDocument *>(block.document());
    // What each place is inside of, from the pieces that are there.
    struct Open { QString tag; int author; };
    QVector<Open> stack;
    struct Change { int from, to; QTextCharFormat format; };
    QVector<Change> changes;
    int deleting = 0;
    for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        QTextCharFormat cf = f.charFormat();
        bool piece = false;
        if (cf.hasProperty(prop::Atom)) {
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            piece = a.kind == Inline::ParaAtom && isRevisionTag(a.tag) && a.role != Inline::Single;
            if (piece) {
                const int n = int(f.length());
                for (int k = 0; k < n; ++k) {
                    if (a.role == Inline::Open) {
                        stack.push_back({a.tag, doc.reviewerIndex(attributeOf(a.raw, "author"))});
                        if (a.tag == QLatin1String("del") || a.tag == QLatin1String("moveFrom")) ++deleting;
                    } else if (!stack.isEmpty()) {
                        if (stack.last().tag == QLatin1String("del") || stack.last().tag == QLatin1String("moveFrom")) deleting = std::max(0, deleting - 1);
                        stack.pop_back();
                    }
                }
                continue;
            }
        }
        // The kind that marks the place: a deletion over an insertion.
        int kind = 0, author = 0;
        bool inserted = false;
        for (const Open &o : std::as_const(stack)) {
            const int k = o.tag == QLatin1String("ins") ? 1 : o.tag == QLatin1String("del") ? 2 : o.tag == QLatin1String("moveFrom") ? 3 : 4;
            inserted = inserted || k == 1 || k == 4;
            if ((kind == 2 || kind == 3) && k != 2 && k != 3) continue;
            kind = k;
            author = o.author;
        }
        QTextCharFormat nf = withRevisionLook(cf, deleting > 0, kind, author, inserted);
        // A change of the run's properties.
        bool changed = false;
        if (cf.hasProperty(prop::RawRPr)) changed = recordOf(extraOf(cf, prop::RawRPr), "rPrChange") != nullptr;
        if (changed) nf.setProperty(tp::RevisionFormat, true);
        else nf.clearProperty(tp::RevisionFormat);
        if (nf != cf) changes.push_back({f.position(), f.position() + int(f.length()), nf});
    }
    if (changes.isEmpty()) return;
    QTextCursor t(d);
    for (const Change &c : std::as_const(changes)) {
        t.setPosition(c.from);
        t.setPosition(c.to, QTextCursor::KeepAnchor);
        t.setCharFormat(c.format);
    }
}


// ---- accepting and rejecting

namespace {

// Takes the records of tracked changes out of a run's kept properties (`names`), and the look that follows them.
QTextCharFormat withoutRecord(QTextCharFormat f, int property, const char *name)
{
    QVector<RawChild> extra = extraOf(f, property);
    bool any = false;
    for (int i = int(extra.size()) - 1; i >= 0; --i)
        if (elementName(extra[i].xml) == name) {
            extra.removeAt(i);
            any = true;
        }
    if (!any) return f;
    if (extra.isEmpty()) f.clearProperty(property);
    else setStored(f, property, packRaw(extra));
    return f;
}

// The run properties a record of a change (w:rPrChange) kept: the old ones.
// Where the element `name` (an opening tag, such as "<w:rPr") starts in the bytes of a record: the tag itself, not one whose name goes on (<w:rPrChange).
qsizetype tagStart(const QByteArray &xml, const QByteArray &open)
{
    for (qsizetype at = xml.indexOf(open); at >= 0; at = xml.indexOf(open, at + 1)) {
        const char next = at + open.size() < xml.size() ? xml[at + open.size()] : '>';
        if (next == '>' || next == '/' || next == ' ') return at;
    }
    return -1;
}

// The element `name` of a record of a change (the old properties it kept), as bytes.
QByteArray recordedElement(const RawChild &record, const QByteArray &name)
{
    const qsizetype at = tagStart(record.xml, "<w:" + name);
    if (at < 0) return QByteArray();
    const qsizetype gt = record.xml.indexOf('>', at);
    if (gt > 0 && record.xml[gt - 1] == '/') return record.xml.mid(at, gt + 1 - at);
    const qsizetype end = record.xml.lastIndexOf("</w:" + name + ">");
    return end < 0 ? QByteArray() : record.xml.mid(at, end + name.size() + 4 - at + 1);
}

// The run properties a record of a change (w:rPrChange) kept: the old ones.
RunProps recordedRunProps(const RawChild &record)
{
    const QByteArray element = recordedElement(record, "rPr");
    return element.isEmpty() ? RunProps() : readRunPropsXml(element);
}

ParaProps recordedParaProps(const RawChild &record)
{
    const QByteArray element = recordedElement(record, "pPr");
    return element.isEmpty() ? ParaProps() : readParaPropsXml(element);
}

// What a run is when a change of its properties is rejected: the format with the part that is its properties put back as they were, and the rest (a piece kept as XML, a field) as it is.
QTextCharFormat restoredFormat(const WriteDoc &doc, const QString &paragraphStyle, const QTextCharFormat &now, const RunProps &old, const QVector<RawChild> &oldExtra)
{
    const StyleSheet &sheet = doc.styleSheet();
    RunProps cur = runPropsOf(now);
    const QTextCharFormat before = charFormatOf(cur, doc.theme(), nullptr);
    RunProps target = sheet.run(paragraphStyle, old.style);
    target.overlay(old);
    target.extra = oldExtra;
    target.style = old.style;
    RunProps direct = old;
    direct.extra = oldExtra;
    const QTextCharFormat after = charFormatOf(target, doc.theme(), &direct);
    QTextCharFormat out = now;
    const QMap<int, QVariant> was = before.properties(), will = after.properties();
    for (auto it = was.cbegin(); it != was.cend(); ++it)
        if (!will.contains(it.key())) out.clearProperty(it.key());
    for (auto it = will.cbegin(); it != will.cend(); ++it) out.setProperty(it.key(), it.value());
    out.clearProperty(tp::RevisionFormat);
    return out;
}

QString styleIdOf(const WriteDoc &doc, const QTextBlock &b)
{
    const QString id = b.blockFormat().stringProperty(tp::StyleName);
    return id.isEmpty() ? doc.defaultParagraphStyle() : id;
}

// Accepts or rejects the changes of properties in the runs of a paragraph that the stretches `ranges` (sorted, places of the story) cover: one pass over the paragraph's runs.
void resolveFormatChanges(WriteDoc &doc, QTextDocument *d, int blockNumber, const QVector<std::pair<int, int>> &ranges, bool accept)
{
    struct Edit { int from, to; QTextCharFormat format; };
    QVector<Edit> edits;
    const QTextBlock b = d->findBlockByNumber(blockNumber);
    const QString style = styleIdOf(doc, b);
    int at = 0;   // the first stretch that has not ended before the run
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        ++s_runsVisited;
        const QTextFragment f = it.fragment();
        const int fs = f.position(), fe = fs + int(f.length());
        while (at < ranges.size() && ranges[at].second <= fs) ++at;
        if (at >= ranges.size()) break;
        if (ranges[at].first >= fe) continue;
        QTextCharFormat cf = f.charFormat();
        QVector<RawChild> extra = extraOf(cf, prop::RawRPr);
        const RawChild *rec = recordOf(extra, "rPrChange");
        if (!rec) continue;
        QTextCharFormat nf;
        if (accept) {
            nf = withoutRecord(cf, prop::RawRPr, "rPrChange");
            nf.clearProperty(tp::RevisionFormat);
        } else {
            const RunProps old = recordedRunProps(*rec);
            QVector<RawChild> kept;
            for (const RawChild &child : old.extra)
                if (elementName(child.xml) != "rPrChange") kept.push_back(child);   // (a record in the old properties is not a change that is made again)
            nf = restoredFormat(doc, style, cf, old, kept);
        }
        edits.push_back({fs, fe, nf});
    }
    QTextCursor t(d);
    for (const Edit &e : std::as_const(edits)) {
        t.setPosition(e.from);
        t.setPosition(e.to, QTextCursor::KeepAnchor);
        t.setCharFormat(e.format);
    }
}

// Accepts or rejects a change of a paragraph's properties (section 17.13.5.29): the record goes, and, when it is rejected, the old properties come back, the list the paragraph was in too. False where
// the old list is not one the document has any more: the paragraph stays in the list it is in (no list is named that is not defined), and the change is told as kept.
bool resolveParagraphChange(WriteDoc &doc, const QTextBlock &b, bool accept)
{
    QTextBlockFormat bf = b.blockFormat();
    QVector<RawChild> extra = extraOf(bf, prop::RawPPr);
    const RawChild *rec = recordOf(extra, "pPrChange");
    if (!rec) return true;
    QTextBlockFormat nf = bf;
    bool restoreList = false;
    int oldNumId = 0, oldLevel = 0;
    std::optional<qreal> oldLeft, oldFirst;
    if (!accept) {
        if (recordedElement(*rec, "pPr").isEmpty()) return false;   // (the old properties can't be read: they are not replaced by defaults)
        const ParaProps old = recordedParaProps(*rec);
        const StyleSheet &sheet = doc.styleSheet();
        ParaProps target = sheet.paragraph(old.style.isEmpty() ? styleIdOf(doc, b) : old.style);
        target.overlay(old);
        target.style = old.style.isEmpty() ? styleIdOf(doc, b) : old.style;
        QVector<RawChild> kept;
        for (const RawChild &c : extra)
            if (elementName(c.xml) != "pPrChange") kept.push_back(c);
        target.extra = kept;
        ParaProps cur = paraPropsOf(bf);
        const QTextBlockFormat before = blockFormatOf(cur);
        const QTextBlockFormat after = blockFormatOf(target);
        const QMap<int, QVariant> was = before.properties(), will = after.properties();
        // (The list is put back below, with the list commands' own steps: the numbers of the list are not properties to set.)
        auto listKey = [](int key) { return key == prop::NumId || key == prop::NumLevel || key == QTextFormat::ObjectIndex || key == tp::BulletChar || key == tp::ListLevel; };
        for (auto it = was.cbegin(); it != was.cend(); ++it)
            if (!will.contains(it.key()) && !listKey(it.key())) nf.clearProperty(it.key());
        for (auto it = will.cbegin(); it != will.cend(); ++it)
            if (!listKey(it.key())) nf.setProperty(it.key(), it.value());
        restoreList = true;
        oldNumId = std::max(0, target.numId.value_or(0));
        oldLevel = std::clamp(target.ilvl.value_or(0), 0, 8);
        if (target.indLeft) oldLeft = after.leftMargin();
        if (target.indFirst) oldFirst = after.textIndent();
        // (All or nothing: a list the document no longer defines can't be put back, so nothing of the old properties is, and the change stays as it is.)
        const bool inIt = bf.hasProperty(prop::NumId) && bf.intProperty(prop::NumId) == oldNumId && bf.intProperty(prop::NumLevel) == oldLevel;
        if (oldNumId > 0 && !inIt && doc.deepestLevel(oldNumId) < 0) return false;
    }
    QVector<RawChild> kept = extraOf(nf, prop::RawPPr);
    for (int i = int(kept.size()) - 1; i >= 0; --i)
        if (elementName(kept[i].xml) == "pPrChange") kept.removeAt(i);
    if (kept.isEmpty()) nf.clearProperty(prop::RawPPr);
    else setStored(nf, prop::RawPPr, packRaw(kept));
    QTextCursor t(b);
    t.setBlockFormat(nf);
    if (!restoreList) return true;
    const QTextBlockFormat now = b.blockFormat();
    const bool inList = now.hasProperty(prop::NumId);
    bool changed = false, ok = true;
    if (oldNumId > 0) {
        if (!inList || now.intProperty(prop::NumId) != oldNumId || now.intProperty(prop::NumLevel) != oldLevel) {
            ok = putBlockInList(doc, b, oldNumId, oldLevel);
            changed = ok;
        }
    } else if (inList) {
        takeBlockOutOfList(doc, b);
        changed = true;
    }
    if (changed && (oldLeft || oldFirst)) {
        // (An indent the old properties stated is theirs, not the list's.)
        QTextBlockFormat f = b.blockFormat();
        if (oldLeft) f.setLeftMargin(*oldLeft);
        if (oldFirst) f.setTextIndent(*oldFirst);
        QTextCursor(b).setBlockFormat(f);
    }
    return ok;
}

// Accepts or rejects a tracked numbering of a paragraph (the mark of it among the rest of its numbering is taken out): rejected, the paragraph is taken out of its list when the mark says the
// numbering was added (w:ins), or when the label of the old numbering is empty (a paragraph that had none). A label that names an old numbering gives no list to go back to, so the change is
// kept as written. False where it is kept.
bool resolveNumbering(WriteDoc &doc, const QTextBlock &b, const Revision &r, bool accept)
{
    QTextBlockFormat bf = b.blockFormat();
    const QByteArray extra = bf.hasProperty(prop::NumExtra) ? storedBytes(bf, prop::NumExtra) : QByteArray();
    for (const NumberingMark &m : numberingMarksOf(extra)) {
        if (m.kind != r.kind || attributeOf(m.xml, "id") != r.id) continue;
        const bool noOld = r.kind == Revision::NumberingInserted || attributeOf(m.xml, "original").isEmpty();
        if (!accept && !noOld) return false;
        QByteArray rest = extra;
        rest.remove(int(m.at), int(m.end - m.at));
        if (rest.isEmpty()) bf.clearProperty(prop::NumExtra);
        else setStored(bf, prop::NumExtra, rest);
        QTextCursor(b).setBlockFormat(bf);
        if (!accept) {
            // Before an inserted numbering the paragraph had none of its own (section 17.13.5.19), so it has what its style gives; where the old numbering was none (the label is empty), it has none,
            // which is a "no numbering" of its own where the style gives one.
            const QTextBlockFormat base = r.kind == Revision::NumberingInserted ? doc.styleBlockFormat(styleIdOf(doc, b)) : QTextBlockFormat();
            const int numId = base.hasProperty(prop::NumId) ? base.intProperty(prop::NumId) : 0;
            if (numId > 0 && doc.deepestLevel(numId) >= 0) {
                putBlockInList(doc, b, numId, base.intProperty(prop::NumLevel));
                QTextBlockFormat f = b.blockFormat();
                f.setLeftMargin(base.leftMargin());
                f.setTextIndent(base.textIndent());
                QTextCursor(b).setBlockFormat(f);
            } else if (b.blockFormat().hasProperty(prop::NumId)) {
                takeBlockOutOfList(doc, b);
            }
        }
        return true;
    }
    return true;
}

// Where the sections that tracked changes of properties are in the document's list of sections, for the changes `which` names (a block, and the Id of the record): a paragraph that ends a section
// is that section's, and the last paragraph of the text can be the last section's. The ones the document has no such record in are not in the list (the caller counts them as kept).
QVector<int> sectionsOfChanges(WriteDoc &doc, QTextDocument *story, const QVector<Revision> &which)
{
    QVector<int> out;
    int most = 0;
    for (const Revision &r : which)
        if (r.kind == Revision::SectionFormat) most = std::max(most, r.block);
    if (story != doc.story() || doc.sectionCount() == 0) return out;
    bool any = false;
    for (const Revision &r : which) any = any || r.kind == Revision::SectionFormat;
    if (!any) return out;
    QVector<int> breaksBefore(most + 2, 0);
    int n = 0, blockNo = 0;
    for (QTextBlock b = story->begin(); b.isValid() && blockNo <= most; b = b.next(), ++blockNo) {
        breaksBefore[blockNo] = n;
        if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) ++n;
    }
    for (const Revision &r : which) {
        if (r.kind != Revision::SectionFormat) continue;
        const QTextBlock b = story->findBlockByNumber(r.block);
        QVector<int> candidates;
        if (b.isValid() && b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) candidates.push_back(breaksBefore[r.block]);
        candidates.push_back(doc.sectionCount() - 1);
        for (const int c : std::as_const(candidates)) {
            const RawChild *rec = recordOf(doc.sectionProps(c).extra, "sectPrChange");
            if (rec && attributeOf(rec->xml, "id") == r.id && !out.contains(c)) {
                out.push_back(c);
                break;
            }
        }
    }
    return out;
}

// Accepts or rejects the change of a section's properties (section 17.13.5.32): the record goes, and, when it is rejected, the properties it kept are the section's again (the page size, margins,
// columns, numbering of pages, type, and the rest it states; the headers and footers the section names stay, and what belongs to the document and not to a section stays). 1 where it was done, 0 where
// the section has no such record, and -1 where the record's old properties can't be read (it is kept as it is: no default page is put in their place).
int resolveSectionProps(SectionProps &p, bool accept)
{
    const RawChild *rec = recordOf(p.extra, "sectPrChange");
    if (!rec) return 0;
    if (!accept) {
        SectionProps old;
        if (!readRecordedSectionProps(rec->xml, &old)) return -1;
        QVector<RawChild> extra;
        for (const RawChild &c : std::as_const(p.extra)) {
            const QByteArray name = elementName(c.xml);
            if (name == "headerReference" || name == "footerReference") extra.push_back(c);
        }
        for (const RawChild &c : std::as_const(old.extra)) {
            const QByteArray name = elementName(c.xml);
            if (name != "headerReference" && name != "footerReference" && name != "sectPrChange") extra.push_back(c);
        }
        std::stable_sort(extra.begin(), extra.end(), [](const RawChild &a, const RawChild &b) { return a.rank < b.rank; });
        p.type = old.type;
        p.pageW = old.pageW;
        p.pageH = old.pageH;
        p.orient = old.orient;
        p.top = old.top;
        p.right = old.right;
        p.bottom = old.bottom;
        p.left = old.left;
        p.header = old.header;
        p.footer = old.footer;
        p.gutter = old.gutter;
        p.cols = old.cols;
        p.colSpace = old.colSpace;
        p.colSep = old.colSep;
        p.colEqual = old.colEqual;
        p.colDefs = old.colDefs;
        p.pageStart = old.pageStart;
        p.pageFormat = old.pageFormat;
        p.titlePg = old.titlePg;
        p.hasPgSz = old.hasPgSz;
        p.hasPgMar = old.hasPgMar;
        p.hasCols = old.hasCols;
        p.pgSzAttrs = old.pgSzAttrs;
        p.pgMarAttrs = old.pgMarAttrs;
        p.pgNumAttrs = old.pgNumAttrs;
        p.colsAttrs = old.colsAttrs;
        p.extra = extra;
    }
    for (int i = int(p.extra.size()) - 1; i >= 0; --i)
        if (elementName(p.extra[i].xml) == "sectPrChange") p.extra.removeAt(i);
    return 1;
}

// The paragraph's mark without the record that it was inserted or deleted (and, if it was deleted, no longer hidden).
void clearMarkRecord(const QTextBlock &b)
{
    QTextCharFormat m = markFormat(b);
    QVector<RawChild> extra = extraOf(m, prop::RawRPr);
    for (int i = int(extra.size()) - 1; i >= 0; --i) {
        const QByteArray name = elementName(extra[i].xml);
        if (name == "ins" || name == "del" || name == "moveFrom" || name == "moveTo") extra.removeAt(i);
    }
    if (extra.isEmpty()) m.clearProperty(prop::RawRPr);
    else setStored(m, prop::RawRPr, packRaw(extra));
    m = notDeletedFormat(m);
    m.clearProperty(tp::Revision);
    m.clearProperty(tp::RevisionAuthor);
    setMarkFormat(b, m);
}

// Takes a paragraph's mark away (it was deleted and the change is accepted, or inserted and the change is rejected): the paragraph is joined to the one after it, which keeps its own look
// (a paragraph with nothing left is gone). Where the paragraph can't be joined (the last of the story, one that ends a section, one of a table's cell with the next in another) the mark stays.
void removeMark(WriteDoc &doc, QTextDocument *d, int blockNumber)
{
    QTextBlock b = d->findBlockByNumber(blockNumber);
    if (!b.isValid()) return;
    const QTextBlock n = b.next();
    const QTextBlockFormat bf = b.blockFormat();
    if (!n.isValid() && b.length() == 1 && b.position() > 0 && !b.previous().blockFormat().hasProperty(prop::EndnoteBlock) && !isLocked(b) && !isLocked(b.previous()) && canJoinBlocks(b.previous(), b) &&
        !bf.hasProperty(prop::RawBefore) && !bf.hasProperty(prop::RawAfter) && !bf.boolProperty(flow::fp::SectionBreakAfter) && bf.intProperty(flow::fp::Frame) == b.previous().blockFormat().intProperty(flow::fp::Frame)) {
        // Nothing of the last paragraph is left (it was all inserted): it goes, and the one before it is the last.
        QTextCursor t(d);
        t.setPosition(b.position() - 1);
        t.setPosition(b.position(), QTextCursor::KeepAnchor);
        t.removeSelectedText();
        return;
    }
    if (!n.isValid() || isLocked(b) || isLocked(n) || !canJoinBlocks(b, n) || n.blockFormat().hasProperty(prop::EndnoteBlock) ||
        bf.intProperty(flow::fp::Frame) != n.blockFormat().intProperty(flow::fp::Frame)) {
        clearMarkRecord(b);
        return;
    }
    const int pos = b.position(), len = b.length() - 1;
    if (len == 0 && pos > 0 && !bf.hasProperty(prop::RawBefore) && !bf.hasProperty(prop::RawAfter) && !bf.boolProperty(flow::fp::SectionBreakAfter)) {
        // Nothing of it is left: the paragraph goes, and the next keeps its own look.
        QTextCursor t(d);
        t.setPosition(pos - 1);
        t.setPosition(pos + len, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        return;
    }
    if (len == 0 && pos == 0 && !n.textList() && !bf.hasProperty(prop::RawBefore) && !bf.hasProperty(prop::RawAfter) && !bf.boolProperty(flow::fp::SectionBreakAfter)) {
        // Nothing of the first paragraph is left, and no paragraph is before it: its mark goes, and the next paragraph, which is the first now, keeps its own look.
        const QTextBlockFormat nextFormat = n.blockFormat();
        const QTextCharFormat nextMark = markFormat(n);
        QTextCursor t(d);
        t.setPosition(0);
        t.setPosition(1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        t.setPosition(0);
        t.setBlockFormat(nextFormat);
        setMarkFormat(d->begin(), nextMark);
        return;
    }
    // The text of the paragraph joins the next one's.
    const QTextCharFormat nextMark = markFormat(n);
    QTextCursor t(d);
    t.setPosition(pos + len);
    t.setPosition(pos + len + 1, QTextCursor::KeepAnchor);
    removeSelection(doc, t);
    QTextBlock joined = d->findBlock(pos);
    QTextCharFormat m = nextMark;
    setMarkFormat(joined, m);
}

} // namespace


// The tag of the element kept as written that is the one character at `pos` of the story (none where it is no such element).
QString atomTagAt(const QTextDocument *story, int pos)
{
    if (pos < 0 || pos >= story->characterCount() - 1 || story->characterAt(pos) != QChar::ObjectReplacementCharacter) return QString();
    QTextCursor c(const_cast<QTextDocument *>(story));
    c.setPosition(pos + 1);
    const QTextCharFormat cf = c.charFormat();
    if (!cf.hasProperty(prop::Atom)) return QString();
    const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
    return a.kind == Inline::ParaAtom && a.role == Inline::Single ? a.tag : QString();
}

// The marks of the ranges of the moves of a story: where each range begins and ends, and the name that joins it to the range of the other side.
namespace {

struct MoveRange {
    int start = 0, end = -1;   // the places of the mark that begins it and of the one that ends it (-1: there is none)
    QString name;
};

struct MoveRanges {
    QVector<MoveRange> from, to;   // in the order of the story
};

MoveRanges moveRangesOf(const QTextDocument *story)
{
    MoveRanges out;
    QHash<QString, QVector<int>> fromEnds, toEnds;   // the places of the ends, by the Id the range gives
    struct Start { MoveRange range; QString id; bool from; };
    QVector<Start> starts;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (outOfScope(b)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (a.kind != Inline::ParaAtom || a.role != Inline::Single || !a.tag.startsWith(QLatin1String("move"))) continue;
            const bool isFrom = a.tag.startsWith(QLatin1String("moveFrom")), isStart = a.tag.endsWith(QLatin1String("RangeStart")), isEnd = a.tag.endsWith(QLatin1String("RangeEnd"));
            if (!isStart && !isEnd) continue;
            const QString t = it.fragment().text();
            for (int k = 0; k < t.size(); ++k) {
                if (t[k] != QChar::ObjectReplacementCharacter) continue;
                const int pos = it.fragment().position() + k;
                if (isStart) starts.push_back({{pos, -1, attributeOf(a.raw, "name")}, attributeOf(a.raw, "id"), isFrom});
                else (isFrom ? fromEnds : toEnds)[attributeOf(a.raw, "id")].push_back(pos);
            }
        }
    }
    for (Start &s : starts) {
        const QVector<int> &ends = (s.from ? fromEnds : toEnds).value(s.id);
        const auto e = std::upper_bound(ends.begin(), ends.end(), s.range.start);
        if (e != ends.end()) s.range.end = *e;
        (s.from ? out.from : out.to).push_back(s.range);
    }
    return out;
}

// The range of the side that the place is inside of (a half of a move begins there), as an index; -1 where none is.
int rangeAround(const QVector<MoveRange> &ranges, int pos)
{
    auto it = std::lower_bound(ranges.begin(), ranges.end(), pos, [](const MoveRange &r, int p) { return r.start < p; });
    if (it == ranges.begin()) return -1;
    --it;
    return it->end < 0 || pos < it->end ? int(it - ranges.begin()) : -1;
}

} // namespace

// A move has two halves, the text taken from a place and the same text put in another, joined by the name of the move that the marks of their ranges give. Where one half is among the changes
// to resolve, the other half is too, and so are the marks of the paragraphs that are wholly in a half. A move may have a piece in each of several paragraphs, all inside the one range: every
// piece is a half, and a name that each side has the same number of ranges of (one for each piece, or one for the lot) joins them all. A name that is unbalanced (two ranges of one side and one of the
// other), or that has no range on the other side, joins nothing: each half of it is then a change of its own. `marks` gets the places of the marks of the ranges that the halves are inside of;
// `secondary` gets (kind << 40 | place) of the changes that are a move's second half and the paragraph marks that went with it (the lists show a move once).
static QVector<Revision> withMovePairs(const QTextDocument *story, const QVector<Revision> &which, QVector<int> *marks)
{
    bool any = false;
    for (const Revision &r : which) any = any || r.kind == Revision::MovedFrom || r.kind == Revision::MovedTo;
    if (!any) return which;
    QSet<qint64> secondaryKeys, *secondary = &secondaryKeys;
    const MoveRanges ranges = moveRangesOf(story);
    // Every half of every move there is, by side, in order: the pieces of a range are found among them by place.
    QVector<Revision> fromHalves, toHalves;
    for (const Revision &r : revisionsIn(story)) {
        if (r.kind == Revision::MovedFrom) fromHalves.push_back(r);
        else if (r.kind == Revision::MovedTo) toHalves.push_back(r);
    }
    QHash<QString, QVector<int>> fromByName, toByName;
    for (int i = 0; i < ranges.from.size(); ++i) fromByName[ranges.from[i].name].push_back(i);
    for (int i = 0; i < ranges.to.size(); ++i) toByName[ranges.to[i].name].push_back(i);
    QVector<Revision> out = which;
    QSet<qint64> have;
    for (const Revision &r : which) have.insert(qint64(r.kind) << 40 | r.from);
    QVector<Revision> halves;   // (the halves of the moves: those asked for and their partners)
    QSet<qint64> markPlaces;
    QSet<QString> groupsDone;
    auto addHalf = [&](const Revision &h) {
        halves.push_back(h);
        if (have.contains(qint64(h.kind) << 40 | h.from)) return;
        have.insert(qint64(h.kind) << 40 | h.from);
        out.push_back(h);
    };
    // The halves that begin inside a range, in order (one only where the range has no end: the piece it was put before).
    auto piecesIn = [&](const QVector<Revision> &sorted, const MoveRange &range) {
        QVector<Revision> pieces;
        auto it = std::upper_bound(sorted.begin(), sorted.end(), range.start, [](int p, const Revision &r) { return p < r.from; });
        for (; it != sorted.end() && (range.end < 0 || it->from < range.end); ++it) {
            pieces.push_back(*it);
            if (range.end < 0) break;
        }
        return pieces;
    };
    auto markRange = [&](const QVector<MoveRange> &side, int index) {
        if (index < 0) return;
        markPlaces.insert(side[index].start);
        if (side[index].end >= 0) markPlaces.insert(side[index].end);
    };
    for (const Revision &r : which) {
        if (r.kind != Revision::MovedFrom && r.kind != Revision::MovedTo) continue;
        const bool isFrom = r.kind == Revision::MovedFrom;
        const QVector<MoveRange> &mine = isFrom ? ranges.from : ranges.to, &theirs = isFrom ? ranges.to : ranges.from;
        const int at = rangeAround(mine, r.from);
        addHalf(r);
        if (at < 0) continue;
        markRange(mine, at);
        const QString name = mine[at].name;
        const QVector<int> &ownNamed = (isFrom ? fromByName : toByName).value(name), &otherNamed = (isFrom ? toByName : fromByName).value(name);
        if (name.isEmpty() || ownNamed.isEmpty() || ownNamed.size() != otherNamed.size()) {
            // (A name that joins nothing: the pieces of this half's own range are its own.)
            for (const Revision &piece : piecesIn(isFrom ? fromHalves : toHalves, mine[at])) addHalf(piece);
            continue;
        }
        if (groupsDone.contains(name)) continue;
        groupsDone.insert(name);
        auto addPiece = [&](const Revision &piece) {
            addHalf(piece);
            if (secondary && piece.kind == Revision::MovedTo) secondary->insert(qint64(piece.kind) << 40 | piece.from);
        };
        for (int index : ownNamed) {
            markRange(mine, index);
            for (const Revision &piece : piecesIn(isFrom ? fromHalves : toHalves, mine[index])) addPiece(piece);
        }
        for (int index : otherNamed) {
            markRange(theirs, index);
            for (const Revision &piece : piecesIn(isFrom ? toHalves : fromHalves, theirs[index])) addPiece(piece);
        }
    }
    // The mark of a paragraph that a half covers whole (the paragraph was moved with its mark).
    for (const Revision &h : std::as_const(halves)) {
        const QTextBlock b = story->findBlock(h.from);
        if (!b.isValid()) continue;
        // (the half is the whole paragraph, but for the marks of its range at either end)
        const int markAt = b.position() + b.length() - 1;
        const bool startsIt = h.from == b.position() || (h.from == b.position() + 1 && atomTagAt(story, b.position()).endsWith(QLatin1String("RangeStart")));
        const bool endsIt = h.to == markAt || (h.to == markAt - 1 && atomTagAt(story, markAt - 1).endsWith(QLatin1String("RangeEnd")));
        if (!startsIt || !endsIt) continue;
        for (const Revision &m : revisionsIn(story, b.position() + b.length() - 1, b.position() + b.length() - 1)) {
            if (!m.moved || m.block != b.blockNumber()) continue;
            if (m.kind != (h.kind == Revision::MovedFrom ? Revision::MarkDeleted : Revision::MarkInserted)) continue;
            if (secondary) secondary->insert(qint64(m.kind) << 40 | m.from);
            if (have.contains(qint64(m.kind) << 40 | m.from)) continue;
            have.insert(qint64(m.kind) << 40 | m.from);
            out.push_back(m);
        }
    }
    if (marks)
        for (qint64 place : std::as_const(markPlaces)) marks->push_back(int(place));
    if (secondary)
        for (Revision &r : out)
            if (secondary->contains(qint64(r.kind) << 40 | r.from)) r.partner = true;
    return out;
}

namespace {

// A change a table records, and which of its kind it is in the story (a change is told by where it is, never by its Id, which a file may repeat).
struct PlacedTableChange {
    Revision::Kind kind;
    TableChange change;
    int ordinal = 0;
};

QVector<PlacedTableChange> tableChangesOf(const QTextDocument *story)
{
    QVector<PlacedTableChange> out;
    const QVariant store = story->property(kTableStoreProperty);
    if (!store.isValid()) return out;
    static const Revision::Kind kinds[] = {Revision::RowInserted, Revision::RowDeleted, Revision::CellInserted, Revision::CellDeleted, Revision::CellMerged, Revision::TableFormat, Revision::RowFormat, Revision::CellFormat};
    int perKind[16] = {};
    QVector<TableChange> now;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (!b.blockFormat().hasProperty(flow::fp::Cell)) continue;
        now.clear();
        tableChangesAt(*reinterpret_cast<const flow::TableStore *>(store.value<quintptr>()), b, &now);
        for (const TableChange &c : std::as_const(now)) out.push_back({kinds[c.kind], c, perKind[int(kinds[c.kind])]++});
    }
    return out;
}

// Which table changes of the story `wanted` names, by their places as the story is now: (kind, which of that kind).
QVector<std::pair<int, int>> tablePlacesOf(const QTextDocument *story, const QVector<Revision> &wanted)
{
    const QVector<PlacedTableChange> all = tableChangesOf(story);
    QHash<qint64, QVector<int>> byPlace;
    for (int i = 0; i < all.size(); ++i) byPlace[qint64(all[i].kind) << 40 | all[i].change.from].push_back(i);
    QVector<std::pair<int, int>> out;
    for (const Revision &r : wanted) {
        auto it = byPlace.find(qint64(r.kind) << 40 | r.from);
        if (it == byPlace.end() || it->isEmpty()) continue;
        const PlacedTableChange &c = all[it->takeFirst()];
        out.push_back({int(c.kind), c.ordinal});
    }
    return out;
}

} // namespace

// The changes the tables record at `places` (found before anything else was done, as taking text out has moved them), resolved together.
static int resolveTableRevisions(WriteDoc &doc, QTextDocument *story, const QVector<std::pair<int, int>> &places, bool accept, int *kept)
{
    QSet<qint64> wanted;
    for (const auto &p : places) wanted.insert(qint64(p.first) << 32 | p.second);
    QVector<TableChange> chosen;
    for (const PlacedTableChange &c : tableChangesOf(story))
        if (wanted.contains(qint64(c.kind) << 32 | c.ordinal)) chosen.push_back(c.change);
    return resolveTableChanges(doc, story, chosen, accept, kept);
}

int resolveRevisions(WriteDoc &doc, QTextDocument *story, const QVector<Revision> &whichGiven, bool accept, int *kept, int *partners)
{
    if (whichGiven.isEmpty()) return 0;
    QVector<int> moveMarks;
    QVector<Revision> which = withMovePairs(story, whichGiven, &moveMarks);
    auto second = [](const Revision &r) { return r.partner; };
    QVector<Revision> inTables;   // (what the tables record is done last, found again by its place: taking text out has moved it)
    for (const Revision &r : which)
        if (r.inTable()) inTables.push_back(r);
    const QVector<std::pair<int, int>> tablePlaces = inTables.isEmpty() ? QVector<std::pair<int, int>>() : tablePlacesOf(story, inTables);
    EditScope scope(doc, story);
    const UntrackedEdit untracked;   // (accepting and rejecting are not themselves tracked)
    int done = 0;
    {
        // The sections first, while the text is as the changes were listed: a section is found by the paragraph that ends it, and the changes that follow (a section break taken out, one put back)
        // move the sections after it. (They are the document's own: the page layout is made again once, for all of them.)
        const QVector<int> sectionIndexes = sectionsOfChanges(doc, story, which);
        int sectionsListed = 0;
        for (const Revision &r : which) sectionsListed += r.kind == Revision::SectionFormat;
        if (kept && sectionsListed > sectionIndexes.size()) *kept += sectionsListed - int(sectionIndexes.size());
        if (!sectionIndexes.isEmpty()) {
            const QSet<int> wanted(sectionIndexes.cbegin(), sectionIndexes.cend());
            int index = 0, unreadable = 0;
            done += doc.changeSections(0, doc.sectionCount() - 1, [&](SectionProps &p) {
                if (!wanted.contains(index++)) return false;
                const int result = resolveSectionProps(p, accept);
                unreadable += result < 0;
                return result > 0;
            });
            if (kept) *kept += unreadable;
        }
    }
    QTextCursor edit(story);
    edit.beginEditBlock();
    if (!moveMarks.isEmpty()) {
        // The marks of the ranges of the moves go first (the places of the rest follow from where they were): each is the one character, and is taken out only if it is a mark of a range of a move.
        std::sort(moveMarks.begin(), moveMarks.end());
        moveMarks.erase(std::unique(moveMarks.begin(), moveMarks.end()), moveMarks.end());
        QVector<int> gone;
        for (qsizetype i = moveMarks.size(); i-- > 0;) {
            const int at = moveMarks[i];
            const QString tag = atomTagAt(story, at);
            if (!tag.startsWith(QLatin1String("move")) || !(tag.endsWith(QLatin1String("RangeStart")) || tag.endsWith(QLatin1String("RangeEnd")))) continue;
            QTextCursor t(story);
            t.setPosition(at);
            t.setPosition(at + 1, QTextCursor::KeepAnchor);
            t.removeSelectedText();
            gone.push_back(at);
        }
        std::sort(gone.begin(), gone.end());
        auto shifted = [&](int p) { return p - int(std::lower_bound(gone.begin(), gone.end(), p) - gone.begin()); };
        for (Revision &r : which) {
            r.from = shifted(r.from);
            r.to = shifted(r.to);
        }
    }
    QMap<int, QVector<Revision>> byBlock;
    for (const Revision &r : which) byBlock[r.block].push_back(r);
    for (auto bi = byBlock.end(); bi != byBlock.begin();) {
        --bi;
        const int blockNumber = bi.key();
        QVector<Revision> inBlock = bi.value();
        // Inner changes before outer ones, later before earlier, so that the places of the rest stay: a change's end is moved by what the changes inside it did.
        // (A paragraph's numbering marks before a change of its properties, which may give the list it was in back.)
        auto numbering = [](const Revision &r) { return r.kind == Revision::NumberingInserted || r.kind == Revision::NumberingChanged; };
        std::stable_sort(inBlock.begin(), inBlock.end(), [&](const Revision &a, const Revision &b) { return a.from != b.from ? a.from > b.from : a.to != b.to ? a.to < b.to : numbering(a) && !numbering(b); });
        // The changes of properties first, all in one pass (they change no length, so the places of the rest stay).
        {
            QVector<std::pair<int, int>> formats;
            for (const Revision &r : std::as_const(inBlock))
                if (r.kind == Revision::Format) formats.push_back({r.from, r.to});
            if (!formats.isEmpty()) {
                std::sort(formats.begin(), formats.end());
                resolveFormatChanges(doc, story, blockNumber, formats, accept);
                done += int(formats.size());
            }
        }
        struct Did { int from, delta; };
        QVector<Did> did;
        bool markDeleted = false, markInserted = false, structural = false, markSecond = false;
        for (const Revision &r : std::as_const(inBlock)) {
            if (r.kind == Revision::MarkInserted) { markInserted = true; markSecond = second(r); continue; }
            if (r.kind == Revision::MarkDeleted) { markDeleted = true; markSecond = second(r); continue; }
            int to = r.to;
            for (int i = int(did.size()) - 1; i >= 0 && did[i].from < r.to; --i)
                if (did[i].from > r.from) to += did[i].delta;
            QTextCursor t(story);
            switch (r.kind) {
            case Revision::Insertion:
            case Revision::MovedTo:
            case Revision::Deletion:
            case Revision::MovedFrom: {
                const bool removes = (r.kind == Revision::Insertion || r.kind == Revision::MovedTo) ? !accept : accept;
                if (to <= r.from) break;
                if (removes) {
                    t.setPosition(r.from);
                    t.setPosition(to, QTextCursor::KeepAnchor);
                    t.removeSelectedText();
                    did.push_back({r.from, -(to - r.from)});
                } else {
                    t.setPosition(to - 1);
                    t.setPosition(to, QTextCursor::KeepAnchor);
                    t.removeSelectedText();
                    t.setPosition(r.from);
                    t.setPosition(r.from + 1, QTextCursor::KeepAnchor);
                    t.removeSelectedText();
                    did.push_back({r.from, -2});
                }
                structural = true;
                ++done;
                if (partners && second(r)) ++*partners;
                break;
            }
            case Revision::NumberingInserted:
            case Revision::NumberingChanged:
                // (Before a change of the paragraph's properties: that one's old numbering is the one a paragraph goes back to.)
                if (resolveNumbering(doc, story->findBlockByNumber(blockNumber), r, accept)) ++done;
                else if (kept) ++*kept;
                break;
            case Revision::SectionFormat:
                break;   // (done below)
            case Revision::Format:
                break;   // (done above)
            case Revision::ParagraphFormat:
                if (resolveParagraphChange(doc, story->findBlockByNumber(blockNumber), accept)) ++done;
                else if (kept) ++*kept;
                break;
            default:
                break;
            }
        }
        if (structural) refreshRevisionLook(doc, story->findBlockByNumber(blockNumber));
        // The paragraph's mark, last: taking it away joins the paragraph to the one after it.
        if (markDeleted || markInserted) {
            const bool removes = markDeleted ? accept : !accept;
            if (removes) removeMark(doc, story, blockNumber);
            else clearMarkRecord(story->findBlockByNumber(blockNumber));
            ++done;
            if (partners && markSecond) ++*partners;
        }
    }
    edit.endEditBlock();
    if (!tablePlaces.isEmpty()) done += resolveTableRevisions(doc, story, tablePlaces, accept, kept);
    return done;
}

int resolveRevisionsIn(WriteDoc &doc, QTextDocument *story, int from, int to, bool accept, int *kept, int *partners)
{
    return resolveRevisions(doc, story, revisionsIn(story, from, to), accept, kept, partners);
}

namespace {

// The text boxes of a story: the changes in the XML that the pieces of a text box keep are accepted or rejected, and the box is made again from it. Returns how many changes were done. The boxes in the
// words of the endnotes that are edited in place are done too, as the footnotes' are (an endnote is done where the pages set its first copy, and its other copies follow).
int resolveTextBoxes(WriteDoc &doc, QTextDocument *story, bool accept)
{
    const EndnoteScope inScope;
    QVector<int> places;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (outOfScope(b)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("txbxContent") || !xmlHasChanges(bytes)) continue;
            places.push_back(it.fragment().position());
        }
    }
    int done = 0;
    for (const int pos : std::as_const(places)) {
        // (Read again at each: an endnote done before set its other copies again.)
        QTextCursor t(story);
        t.setPosition(pos);
        t.setPosition(pos + 1, QTextCursor::KeepAnchor);
        const QTextCharFormat format = t.charFormat();
        if (!format.hasProperty(prop::Atom)) continue;
        const Inline atom = unpackAtom(storedBytes(format, prop::Atom));
        if (atom.kind != Inline::RunAtom || !atom.raw.contains("txbxContent") || !xmlHasChanges(atom.raw)) continue;
        Inline made = atom;
        made.raw = resolveChangesInXml(atom.raw, accept);
        if (made.raw == atom.raw) continue;
        QTextCharFormat f = format;
        f.clearProperty(flow::fp::AnchorId);
        f.clearProperty(tp::InlineObject);
        setStored(f, prop::Atom, packAtom(made));
        if (!doc.makeDrawnFormat(made, &f)) continue;
        const QTextBlock block = story->findBlock(pos);
        std::unique_ptr<NoteScopeGuard> guard;
        if (block.blockFormat().hasProperty(prop::EndnoteBlock)) guard = std::make_unique<NoteScopeGuard>(doc, t);
        t.setPosition(pos);
        t.setPosition(pos + 1, QTextCursor::KeepAnchor);
        t.setCharFormat(f);
        done += std::max(1, xmlChangeCount(atom.raw) - xmlChangeCount(made.raw));
    }
    if (done) doc.keepObjectsForTheHistory();
    return done;
}

// The words of the endnotes (each note that is edited in place): the changes in them, one note at a time.
int resolveEndnotes(WriteDoc &doc, bool accept, int *kept, int *partners)
{
    QTextDocument *story = doc.story();
    QMap<int, std::pair<int, int>> notes;   // by the note's serial: where its words begin and end
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.hasProperty(prop::EndnoteBlock) || bf.intProperty(prop::EndnoteNote) <= 0) continue;
        const int serial = bf.intProperty(prop::EndnoteNote);
        const int end = b.position() + b.length() - 1;
        if (!notes.contains(serial)) notes.insert(serial, {b.position(), end});
        else notes[serial].second = end;
    }
    int done = 0;
    struct Scope { bool was; Scope() : was(t_withEndnotes) { t_withEndnotes = true; } ~Scope() { t_withEndnotes = was; } } scope;
    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        QTextCursor c(story);
        c.setPosition(it->first);
        const NoteScopeGuard guard(doc, c);
        done += resolveRevisionsIn(doc, story, it->first, it->second, accept, kept, partners);
    }
    return done;
}

// Every change of the document, as a person expects: the text, the text boxes, the endnotes, and the headers, footers, and footnotes. One step of the history.
int resolveEverywhere(WriteDoc &doc, bool accept, int *kept, int *partners)
{
    WriteDoc::StepGroup group(doc);
    if (kept) *kept = 0;
    if (partners) *partners = 0;
    int n = resolveRevisionsIn(doc, doc.story(), 0, -1, accept, kept, partners);
    n += resolveTextBoxes(doc, doc.story(), accept);
    n += resolveEndnotes(doc, accept, kept, partners);
    for (QTextDocument *s : doc.storiesWithChanges()) {
        n += resolveRevisionsIn(doc, s, 0, -1, accept, kept, partners);
        n += resolveTextBoxes(doc, s, accept);
    }
    return n;
}

} // namespace

int acceptAll(WriteDoc &doc, int *kept, int *partners) { return resolveEverywhere(doc, true, kept, partners); }
int rejectAll(WriteDoc &doc, int *kept, int *partners) { return resolveEverywhere(doc, false, kept, partners); }

namespace {

struct Site {
    QTextDocument *story = nullptr;
    QString container;
    int box = -1;   // for the words of a text box of the text: the place of its drawing in the text
};

std::atomic<qint64> s_boxStories{0};
constexpr int kMostBoxesTried = 2000;   // (the boxes looked at, as words, in one list; the rest are only counted)

// The text boxes of the text whose drawings hold changes (the paragraphs of an endnote the pages set are out), by the place of the drawing, with the drawing as it is written; made again only when
// the text has been edited. What a drawing is is learned once for its bytes, not for every edit.
const QVector<WriteDoc::BoxReviewCache::Hit> &boxesWithChanges(WriteDoc &doc)
{
    WriteDoc::BoxReviewCache &cache = doc.boxReview;
    const QTextDocument *story = doc.story();
    const qint64 stamp = qint64(story->revision()) * 1000003 + story->characterCount();
    if (cache.stamp == stamp) return cache.hits;
    cache.hits.clear();
    QHash<QByteArray, int> seen;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("txbxContent")) continue;
            bool holds = false;
            const auto known = cache.holdsChanges.constFind(bytes);
            if (known != cache.holdsChanges.cend()) {
                holds = known.value();
            } else {
                if (xmlHasChanges(bytes) || xmlHasTableChanges(bytes)) {
                    const Inline a = unpackAtom(bytes);
                    holds = a.kind == Inline::RunAtom && a.raw.contains("txbxContent");
                }
                if (cache.holdsChanges.size() > 20000) cache.holdsChanges.clear();
                cache.holdsChanges.insert(bytes, holds);
            }
            if (!holds) continue;
            for (int k = 0; k < int(f.length()); ++k)
                if (f.text().at(k) == QChar::ObjectReplacementCharacter) cache.hits.push_back({f.position() + k, bytes, seen[bytes]++});
        }
    }
    cache.stamp = stamp;
    // (A story of a box that has no changes now, or whose drawing was written, goes: one pass.)
    for (auto it = cache.stories.begin(); it != cache.stories.end();)
        it = seen.value(it->first.first, 0) <= it->first.second ? cache.stories.erase(it) : std::next(it);
    return cache.hits;
}

// The words of a text box as a story the changes are listed and settled in: none where the box can't be read as one (a group, words linked on in other boxes, too many paragraphs).
QTextDocument *boxStoryAt(WriteDoc &doc, const WriteDoc::BoxReviewCache::Hit &hit)
{
    auto &stories = doc.boxReview.stories;
    const std::pair<QByteArray, int> key(hit.bytes, hit.nth);
    const auto it = stories.find(key);
    if (it != stories.end()) return it->second.get();
    ++s_boxStories;
    return stories.emplace(key, textBoxStory(doc, hit.position, nullptr, true)).first->second.get();
}

// The story of a box is forgotten (its drawing was written, or is to be).
void forgetBoxStory(WriteDoc &doc, const QTextDocument *story)
{
    auto &stories = doc.boxReview.stories;
    for (auto it = stories.begin(); it != stories.end(); ++it)
        if (it->second.get() == story) {
            stories.erase(it);
            return;
        }
}

// The stories that can hold changes, in the order of the document: the text (with the endnotes the pages set, which are in it), the footnotes by their numbers, then the headers
// and footers by section (a header, then a footer), then any other part. A story is once in the list, however many sections share it.
std::atomic<qint64> s_siteLists{0};

QVector<Site> sitesOf(WriteDoc &doc)
{
    ++s_siteLists;
    QVector<Site> out;
    out.push_back({doc.story(), QStringLiteral("body")});
    // (After the text, the words of its text boxes, in the order of the text: the way the spelling pane goes through them.)
    int listed = 0, tried = 0;
    for (const auto &hit : boxesWithChanges(doc)) {
        if (listed >= kMostBoxStories || tried >= kMostBoxesTried) break;
        ++tried;
        if (QTextDocument *box = boxStoryAt(doc, hit)) {
            ++listed;
            out.push_back({box, QStringLiteral("box:%1").arg(hit.position), hit.position});
        }
    }
    const QVector<QTextDocument *> others = doc.storiesWithChanges();
    QSet<const QTextDocument *> used;
    QVector<std::pair<int, QTextDocument *>> notes;
    for (QTextDocument *s : others) {
        int id = 0;
        if (doc.footnoteIdOf(s, &id)) {
            notes.push_back({id, s});
            used.insert(s);
        }
    }
    std::stable_sort(notes.begin(), notes.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    for (const auto &n : std::as_const(notes)) out.push_back({n.second, QStringLiteral("footnote:%1").arg(n.first)});
    const QSet<const QTextDocument *> wanted(others.cbegin(), others.cend());
    for (int section = 0; section < doc.sectionCount(); ++section)
        for (const bool footer : {false, true})
            for (int slot = 0; slot < 3; ++slot) {
                QTextDocument *s = doc.headerFooterStory(section, footer, slot);
                if (!s || !wanted.contains(s) || used.contains(s)) continue;
                used.insert(s);
                out.push_back({s, QStringLiteral("%1:%2:%3").arg(footer ? QStringLiteral("footer") : QStringLiteral("header")).arg(section + 1).arg(slot)});
            }
    for (QTextDocument *s : others)
        if (!used.contains(s)) out.push_back({s, QStringLiteral("part")});
    return out;
}

// The number of the endnote a paragraph of the text belongs to (0 where it is none).
int endnoteOf(const QTextDocument *story, int position)
{
    const QTextBlock b = story->findBlock(position);
    return b.isValid() && b.blockFormat().hasProperty(prop::EndnoteBlock) ? b.blockFormat().intProperty(prop::EndnoteNote) : 0;
}

} // namespace

// Marks, in a story's list of changes, the second half of each move and the paragraph marks that went with it (the lists show a move once).
static void markMovePartners(const QTextDocument *story, QVector<Revision> *list)
{
    QVector<Revision> halves;
    for (const Revision &r : *list)
        if (r.kind == Revision::MovedFrom || r.kind == Revision::MovedTo) halves.push_back(r);
    if (halves.isEmpty()) return;
    QSet<qint64> partners;
    for (const Revision &r : withMovePairs(story, halves, nullptr))
        if (r.partner) partners.insert(qint64(r.kind) << 40 | r.from);
    for (Revision &r : *list)
        if (partners.contains(qint64(r.kind) << 40 | r.from)) r.partner = true;
}

QVector<StoryRevision> documentRevisions(WriteDoc &doc, int most)
{
    QVector<StoryRevision> out;
    const EndnoteScope scope;
    for (const Site &site : sitesOf(doc)) {
        if (out.size() >= most) break;
        QVector<Revision> list = revisionsIn(site.story, 0, -1, most - int(out.size()));
        markMovePartners(site.story, &list);
        for (const Revision &r : std::as_const(list)) out.push_back({site.story, r, site.container});
    }
    return out;
}

qint64 reviewStamp(const WriteDoc &doc)
{
    qint64 stamp = qint64(doc.story()->revision()) * 4096 + doc.story()->characterCount();
    for (const QTextDocument *side : doc.sideStories()) stamp += qint64(side->revision()) * 4099 + side->characterCount() + 1;
    return stamp;
}

int documentRevisionCount(WriteDoc &doc)
{
    const EndnoteScope scope;
    int n = 0;
    for (const Site &site : sitesOf(doc)) n += revisionCount(site.story);
    return n;
}

int textBoxChangeCount(WriteDoc &doc)
{
    // (A box whose words are listed has its changes in the lists; the rest are kept as the XML they are written in.)
    const QVector<Site> sites = sitesOf(doc);
    QSet<int> listed;
    for (const Site &site : sites)
        if (site.box >= 0) listed.insert(site.box);
    const EndnoteScope inScope;   // (the boxes in the words of the endnotes that are edited in place are counted, as the footnotes' are)
    int n = 0;
    for (const Site &site : sites) {
        for (QTextBlock b = site.story->begin(); b.isValid(); b = b.next()) {
            if (outOfScope(b)) continue;
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (!cf.hasProperty(prop::Atom)) continue;
                const QByteArray bytes = storedBytes(cf, prop::Atom);
                if (!bytes.contains("txbxContent") || !xmlHasChanges(bytes)) continue;
                if (site.story == doc.story() && listed.contains(it.fragment().position())) continue;
                const Inline a = unpackAtom(bytes);
                if (a.kind == Inline::RunAtom && a.raw.contains("txbxContent")) n += xmlChangeCount(a.raw);
            }
        }
    }
    return n;
}

bool documentHasRevisions(WriteDoc &doc)
{
    const EndnoteScope scope;
    if (revisionCount(doc.story()) > 0) return true;
    for (const Site &site : sitesOf(doc))
        if (site.story != doc.story() && revisionCount(site.story) > 0) return true;
    return textBoxChangeCount(doc) > 0;
}

qint64 siteListsBuiltForTest() { return s_siteLists.load(); }
qint64 boxStoriesBuiltForTest() { return s_boxStories.load(); }

QTextDocument *storyOfContainer(WriteDoc &doc, const QString &container)
{
    if (container == QLatin1String("body")) return doc.story();
    for (const Site &site : sitesOf(doc))
        if (site.container == container) return site.story;
    return nullptr;
}

std::optional<StoryRevision> findRevision(WriteDoc &doc, const QString &container, int kind, const QString &id, int from)
{
    QTextDocument *story = storyOfContainer(doc, container);
    if (!story) return std::nullopt;
    const EndnoteScope scope;
    for (const Revision &r : revisionsIn(story, from, from))
        if (r.from == from && int(r.kind) == kind && r.id == id) return StoryRevision{story, r, container};
    return std::nullopt;
}

std::optional<StoryRevision> nextDocumentRevision(WriteDoc &doc, const QTextDocument *story, int position, bool forward, const StoryRevision *after)
{
    const EndnoteScope scope;
    const QVector<Site> sites = sitesOf(doc);
    int at = 0;
    for (int i = 0; i < sites.size(); ++i)
        if (sites[i].story == story) at = i;
    if (forward) {
        for (int i = at; i < sites.size(); ++i) {
            const std::optional<Revision> r = i == at && after && after->story == sites[i].story ? nextRevision(sites[i].story, after->revision) : nextRevision(sites[i].story, i == at ? position : -1);
            if (r) return StoryRevision{sites[i].story, *r, sites[i].container};
        }
    } else {
        for (int i = at; i >= 0; --i) {
            const std::optional<Revision> r = i == at && after && after->story == sites[i].story ? previousRevision(sites[i].story, after->revision) : previousRevision(sites[i].story, i == at ? position : sites[i].story->characterCount());
            if (r) return StoryRevision{sites[i].story, *r, sites[i].container};
        }
    }
    return std::nullopt;
}

std::optional<StoryRevision> nextDocumentRevisionIn(WriteDoc &doc, const QString &container, int position, bool forward)
{
    const EndnoteScope scope;
    const QVector<Site> sites = sitesOf(doc);
    int at = -1;
    for (int i = 0; i < sites.size(); ++i)
        if (sites[i].container == container) at = i;
    bool gone = false;
    if (at < 0 && container.startsWith(QLatin1String("box:"))) {
        // (A box that is gone stood among the boxes by the place of its drawing.)
        const int place = container.mid(4).toInt();
        gone = true;
        at = 0;
        for (int i = 0; i < sites.size(); ++i)
            if (sites[i].box >= 0 && sites[i].box < place) at = i;
            else if (sites[i].box >= place) break;
    }
    if (at < 0) at = 0;
    if (forward) {
        for (int i = gone ? at + 1 : at; i < sites.size(); ++i) {
            const std::optional<Revision> r = nextRevision(sites[i].story, i == at && !gone ? position : -1);
            if (r) return StoryRevision{sites[i].story, *r, sites[i].container};
        }
    } else {
        for (int i = at; i >= 0; --i) {
            const std::optional<Revision> r = previousRevision(sites[i].story, i == at && !gone ? position : sites[i].story->characterCount());
            if (r) return StoryRevision{sites[i].story, *r, sites[i].container};
        }
    }
    return std::nullopt;
}

std::optional<StoryRevision> boxRevisionLike(WriteDoc &doc, int boxPosition, const QTextDocument *typedIn, const Revision &r)
{
    int which = 0;
    for (const Revision &t : revisionsIn(typedIn))
        if (t.kind == r.kind && t.id == r.id && t.from < r.from) ++which;
    const QString container = QStringLiteral("box:%1").arg(boxPosition);
    QTextDocument *story = storyOfContainer(doc, container);
    if (!story) return std::nullopt;
    for (const Revision &c : revisionsIn(story))
        if (c.kind == r.kind && c.id == r.id && which-- == 0) return StoryRevision{story, c, container};
    return std::nullopt;
}

int resolveDocumentRevisions(WriteDoc &doc, const QVector<StoryRevision> &which, bool accept, int *kept, int *partners)
{
    if (which.isEmpty()) return 0;
    const EndnoteScope scope;
    WriteDoc::StepGroup group(doc);
    // The stories in the order they are first met; a story that resolving the one before took away (a note whose reference was deleted) is left alone, and the changes of a story after
    // the first are found again by what they are, as an earlier step may have moved them.
    QVector<QTextDocument *> order;
    QHash<const QTextDocument *, QVector<Revision>> by;
    for (const StoryRevision &w : which) {
        if (!w.story) continue;
        if (!by.contains(w.story)) order.push_back(w.story);
        by[w.story].push_back(w.revision);
    }
    // (The words of text boxes first: writing a box back changes no place of the text, but settling the text moves the places of the boxes in it.)
    QHash<const QTextDocument *, int> boxPlace;
    QVector<Site> sites = sitesOf(doc);
    bool sitesFresh = true;
    for (const Site &site : std::as_const(sites))
        if (site.box >= 0) boxPlace.insert(site.story, site.box);
    std::stable_partition(order.begin(), order.end(), [&](const QTextDocument *d) { return boxPlace.contains(d); });
    // Where each change of the stories after the first is, by its kind and which of that kind it is in its story, told now before anything is done (the Ids a file gives may repeat).
    QHash<const QTextDocument *, QVector<std::pair<int, int>>> places;
    for (int i = 1; i < order.size(); ++i) {
        const QVector<Revision> now = revisionsIn(order[i]);
        QVector<int> perKind(32, 0);
        QHash<qint64, QVector<int>> byPlace;
        QVector<int> ordinal;
        for (const Revision &n : now) {
            ordinal.push_back(perKind[int(n.kind)]++);
            byPlace[qint64(n.kind) << 40 | n.from].push_back(int(ordinal.size()) - 1);
        }
        QVector<std::pair<int, int>> mine;
        for (const Revision &r : by.value(order[i])) {
            auto it = byPlace.find(qint64(r.kind) << 40 | r.from);
            if (it == byPlace.end() || it->isEmpty()) continue;
            mine.push_back({int(r.kind), ordinal[it->takeFirst()]});
        }
        places.insert(order[i], mine);
    }
    int done = 0;
    bool first = true;
    QSet<const QTextDocument *> alive;
    bool aliveKnown = false;
    for (QTextDocument *story : std::as_const(order)) {
        // (The stories there are, made once: only taking out text can end one, a note whose reference was in the text.)
        if (!aliveKnown) {
            alive.clear();
            if (!sitesFresh) sites = sitesOf(doc);
            sitesFresh = false;
            for (const Site &site : std::as_const(sites)) alive.insert(site.story);
            aliveKnown = true;
        }
        if (!alive.contains(story)) continue;
        QVector<Revision> revisions = by.value(story);
        if (!first) {
            QVector<Revision> found;
            QSet<qint64> wanted;
            for (const auto &p : places.value(story)) wanted.insert(qint64(p.first) << 32 | p.second);
            QVector<int> perKind(32, 0);
            for (const Revision &n : revisionsIn(story))
                if (wanted.contains(qint64(n.kind) << 32 | perKind[int(n.kind)]++)) found.push_back(n);
            revisions = found;
        }
        first = false;
        // The paragraphs of an endnote are changed one note at a time, under that note's own scope.
        QMap<int, QVector<Revision>> inNotes;
        QVector<Revision> inText;
        for (const Revision &r : std::as_const(revisions)) {
            const int note = story == doc.story() ? endnoteOf(story, r.from) : 0;
            if (note > 0) inNotes[note].push_back(r);
            else inText.push_back(r);
        }
        // (The endnotes are after the text, and the later notes after the earlier: done from the last, so that what is done does not move the places of what is left.)
        for (auto it = inNotes.cend(); it != inNotes.cbegin();) {
            --it;
            QTextCursor c(story);
            c.setPosition(it.value().first().from);
            const NoteScopeGuard guard(doc, c);
            done += resolveRevisions(doc, story, it.value(), accept, kept, partners);
        }
        const int before = done;
        done += resolveRevisions(doc, story, inText, accept, kept, partners);
        if (story == doc.story()) aliveKnown = false;
        if (boxPlace.contains(story)) {
            // The box is written back with its changes settled, in the same step; a box that can't be written (it would be too big) keeps what it had, and the changes are told as kept.
            const int at = boxPlace.value(story);
            bool refused = false;
            if (!setTextBoxStory(doc, at, story, &refused) && refused) {
                if (kept) *kept += done - before;
                done = before;
            }
            forgetBoxStory(doc, story);
            aliveKnown = false;
        }
    }
    return done;
}

int resolveAtCursor(WriteDoc &doc, QTextCursor &c, bool accept)
{
    QTextDocument *story = c.document();
    const EndnoteScope scope;
    QVector<Revision> inStory;
    if (c.hasSelection()) {
        // (Every change the selection covers, not one that only touches it: the change that begins where the selection ends is not in it.)
        for (const Revision &r : revisionsIn(story, c.selectionStart(), c.selectionEnd()))
            if (r.to > c.selectionStart() && r.from < c.selectionEnd()) inStory.push_back(r);
    } else if (const std::optional<Revision> r = revisionAt(story, c.position())) {
        inStory.push_back(*r);
    }
    if (inStory.isEmpty()) {
        if (const std::optional<StoryRevision> next = nextDocumentRevision(doc, story, c.selectionStart(), true)) {
            c = QTextCursor(next->story);
            c.setPosition(next->revision.from);
        }
        return 0;
    }
    int at = c.selectionStart();
    QVector<StoryRevision> which;
    for (const Revision &r : std::as_const(inStory)) {
        which.push_back({story, r, QString()});
        at = std::min(at, r.from);   // (the next change is looked for from where the first of them was)
    }
    const int done = resolveDocumentRevisions(doc, which, accept);
    c = QTextCursor(story);
    c.setPosition(std::min(at, story->characterCount() - 1));
    if (const std::optional<StoryRevision> next = nextDocumentRevision(doc, story, c.position() - 1, true)) {
        c = QTextCursor(next->story);
        c.setPosition(next->revision.from);
    }
    return done;
}

namespace {

// The value of the first w:id in a start tag kept as written, or -1; and the bytes with it set to `id`.
int idInTag(const QByteArray &xml)
{
    const qsizetype end = xml.indexOf('>');
    const qsizetype at = xml.indexOf(":id=\"");
    if (at < 0 || (end >= 0 && at > end)) return -1;
    const qsizetype to = xml.indexOf('"', at + 5);
    bool ok = false;
    const int v = xml.mid(at + 5, to - at - 5).toInt(&ok);
    return ok ? v : -1;
}

void setIdInTag(QByteArray *xml, int id)
{
    const qsizetype at = xml->indexOf(":id=\"");
    const qsizetype to = xml->indexOf('"', at + 5);
    if (at < 0 || to < 0) return;
    xml->replace(at + 5, to - at - 5, QByteArray::number(id));
}

bool isChangeMark(const QByteArray &xml)
{
    const QByteArray name = elementName(xml);
    return name == "ins" || name == "del" || name == "moveFrom" || name == "moveTo" || name == "rPrChange" || name == "pPrChange" || name == "sectPrChange" || name == "tblPrChange" ||
           name == "trPrChange" || name == "tcPrChange" || name == "numberingChange";
}

} // namespace

void makeChangeIdsUnique(WriteDoc &doc, QVector<Block> *blocks)
{
    QSet<int> reserved = allCommentIds(doc);
    int most = 0;
    for (const int id : std::as_const(reserved)) most = std::max(most, id);
    // The Ids of comments' marks and bookmarks in the text, and the most there is.
    auto noteReserved = [&](const Inline &a) {
        if (a.kind == Inline::ParaAtom && a.role == Inline::Single && (a.tag.startsWith(QLatin1String("bookmark")) || a.tag.startsWith(QLatin1String("commentRange")) || a.tag.startsWith(QLatin1String("perm")))) {
            const int id = a.key.isEmpty() ? idInTag(a.raw) : a.key.toInt();
            if (id >= 0) {
                reserved.insert(id);
                most = std::max(most, id);
            }
        } else if (a.kind == Inline::RunAtom && a.tag == QLatin1String("commentReference")) {
            const int id = idInTag(a.raw);
            if (id >= 0) {
                reserved.insert(id);
                most = std::max(most, id);
            }
        }
    };
    for (const Block &b : std::as_const(*blocks))
        if (!b.isRaw)
            for (const Inline &a : b.para.content) noteReserved(a);
    // The marks of changes, in order; each Id is used once.
    QSet<int> used;
    int next = most + 1;
    auto settle = [&](QByteArray *xml, QString *key) {
        const int id = idInTag(*xml);
        if (id < 0) return;
        most = std::max(most, id);
        if (!used.contains(id) && !reserved.contains(id)) {
            used.insert(id);
            return;
        }
        while (used.contains(next) || reserved.contains(next)) ++next;
        setIdInTag(xml, next);
        if (key) *key = QString::number(next);
        used.insert(next++);
    };
    // (A first pass finds the most, so that no new Id is one that a later mark has.)
    for (const Block &b : std::as_const(*blocks)) {
        if (b.isRaw) continue;
        for (const Inline &a : b.para.content) {
            if (a.kind == Inline::ParaAtom && a.role == Inline::Open && isRevisionTag(a.tag)) most = std::max(most, idInTag(a.raw));
            for (const RawChild &c : a.props.extra)
                if (isChangeMark(c.xml)) most = std::max(most, idInTag(c.xml));
        }
        if (b.para.props.markRun)
            for (const RawChild &c : b.para.props.markRun->extra)
                if (isChangeMark(c.xml)) most = std::max(most, idInTag(c.xml));
        for (const RawChild &c : b.para.props.extra)
            if (isChangeMark(c.xml)) most = std::max(most, idInTag(c.xml));
    }
    next = most + 1;
    for (Block &b : *blocks) {
        if (b.isRaw) continue;
        if (b.para.props.markRun)
            for (RawChild &c : b.para.props.markRun->extra)
                if (isChangeMark(c.xml)) settle(&c.xml, nullptr);
        for (RawChild &c : b.para.props.extra)
            if (isChangeMark(c.xml)) settle(&c.xml, nullptr);
        for (Inline &a : b.para.content) {
            if (a.kind == Inline::ParaAtom && a.role == Inline::Open && isRevisionTag(a.tag)) settle(&a.raw, &a.key);
            for (RawChild &c : a.props.extra)
                if (isChangeMark(c.xml)) settle(&c.xml, nullptr);
        }
    }
}

// ---- making changes

namespace {

// How a character of a paragraph stands among the pieces of the changes.
struct Stand {
    int deleting = 0;
    QVector<Piece> open;   // the changes it is inside of, outermost first
};

} // namespace

RevisionLook revisionLookAt(const QTextBlock &block, int position)
{
    RevisionLook look;
    const QVector<std::pair<QString, QByteArray>> open = openRevisionsAt(block, position);
    for (const auto &o : open) {
        const int k = revisionKindOf2(o.first);
        const bool deletes = k == 2 || k == 3;
        if (deletes) look.deleted = true;
        if ((look.kind == 2 || look.kind == 3) && !deletes) continue;
        look.kind = k;
        look.author = attributeOf(o.second, "author");
    }
    return look;
}

DeletionPlan planDeletion(const QTextBlock &block, int from, int to, const QString &author)
{
    DeletionPlan plan;
    struct Open { QString tag; bool mine; };
    QVector<Open> stack;
    int deleting = 0;
    auto add = [](QVector<std::pair<int, int>> *runs, int a, int b) {
        if (!runs->isEmpty() && runs->last().second == a) runs->last().second = b;
        else runs->push_back({a, b});
    };
    for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const int fs = f.position(), fe = fs + int(f.length());
        if (fs >= to) break;
        const QTextCharFormat cf = f.charFormat();
        if (cf.hasProperty(prop::Atom)) {
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (a.kind == Inline::ParaAtom && a.role != Inline::Single && isRevisionTag(a.tag)) {
                for (int k = 0; k < int(f.length()); ++k) {
                    if (a.role == Inline::Open) {
                        stack.push_back({a.tag, a.tag == QLatin1String("ins") && attributeOf(a.raw, "author") == author});
                        if (a.tag == QLatin1String("del") || a.tag == QLatin1String("moveFrom")) ++deleting;
                    } else if (!stack.isEmpty()) {
                        if (stack.last().tag == QLatin1String("del") || stack.last().tag == QLatin1String("moveFrom")) deleting = std::max(0, deleting - 1);
                        stack.pop_back();
                    }
                }
                continue;
            }
        }
        if (fe <= from) continue;
        const int a = std::max(fs, from), b = std::min(fe, to);
        if (deleting > 0 || cf.hasProperty(prop::TrackedDeleted)) continue;
        if (!stack.isEmpty() && stack.last().mine) add(&plan.remove, a, b);
        else add(&plan.mark, a, b);
    }
    return plan;
}

QVector<std::pair<int, int>> ownInsertionRanges(const QTextBlock &block, const QString &author)
{
    QVector<std::pair<int, int>> out;
    for (const Pair &p : pairsOf(piecesOf(block)))
        if (p.tag == QLatin1String("ins") && attributeOf(p.raw, "author") == author) out.push_back({p.open + 1, p.close});
    std::sort(out.begin(), out.end());
    return out;
}

void removeOwnInsertion(QTextDocument *story, int from, int to)
{
    if (to <= from) return;
    QTextCursor t(story);
    t.setPosition(from);
    t.setPosition(to, QTextCursor::KeepAnchor);
    t.removeSelectedText();
    // An insertion with nothing left in it goes too.
    const QTextBlock b = story->findBlock(from);
    const QVector<Piece> pieces = piecesOf(b);
    for (int i = int(pieces.size()) - 2; i >= 0; --i)
        if (pieces[i].open && !pieces[i + 1].open && pieces[i].tag == pieces[i + 1].tag && pieces[i + 1].pos == pieces[i].pos + 1 && pieces[i].tag == QLatin1String("ins") && pieces[i].pos <= from && pieces[i + 1].pos >= from) {
            t.setPosition(pieces[i].pos);
            t.setPosition(pieces[i].pos + 2, QTextCursor::KeepAnchor);
            t.removeSelectedText();
        }
}

void markDeleted(WriteDoc &doc, QTextDocument *story, int from, int to)
{
    if (to <= from) return;
    const QTextBlock b = story->findBlock(from);
    const int author = doc.reviewerIndex(doc.reviewer());
    // (Text inside someone's insertion was inserted too.)
    bool insideInsertion = false;
    for (const auto &o : openRevisionsAt(b, from)) insideInsertion = insideInsertion || o.first == QLatin1String("ins") || o.first == QLatin1String("moveTo");
    // A piece that begins (or ends) a deletion of the reviewer's at a place of this paragraph: its start tag (kept as the piece has it) and its Id.
    auto pieceAt = [&](int pos, bool wantOpen, QByteArray *raw, QString *id) {
        if (pos < b.position() || pos >= b.position() + b.length() - 1) return false;
        QTextCursor q(story);
        q.setPosition(pos + 1);
        const QTextCharFormat cf = q.charFormat();
        if (!cf.hasProperty(prop::Atom)) return false;
        const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
        if (a.kind != Inline::ParaAtom || a.tag != QLatin1String("del") || a.role != (wantOpen ? Inline::Open : Inline::Close)) return false;
        if (wantOpen && attributeOf(a.raw, "author") != doc.reviewer()) return false;
        if (raw) *raw = a.raw;
        if (id) *id = a.key;
        return true;
    };
    QByteArray tagAfter;
    QString idBefore;
    const bool endsBefore = pieceAt(from - 1, false, nullptr, &idBefore) && [&] {
        // (The deletion that ends there is the reviewer's: its start is found among the pieces.)
        const QVector<std::pair<QString, QByteArray>> open = openRevisionsAt(b, from);
        Q_UNUSED(open);
        for (const Pair &p : pairsOf(piecesOf(b)))
            if (p.close == from - 1) return p.tag == QLatin1String("del") && attributeOf(p.raw, "author") == doc.reviewer();
        return false;
    }();
    const bool beginsAfter = pieceAt(to, true, &tagAfter, nullptr);
    QTextCursor t(story);
    t.beginEditBlock();
    QTextCharFormat like;   // (what the pieces that mark the deletion are made like: the text's own look before it is deleted)
    {
        QTextCursor q(story);
        q.setPosition(from + 1);
        like = q.charFormat();
    }
    // The look of what is deleted, in the person's color.
    struct Edit { int from, to; QTextCharFormat format; };
    QVector<Edit> edits;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const int fs = std::max(f.position(), from), fe = std::min(f.position() + int(f.length()), to);
        if (fs >= fe) continue;
        edits.push_back({fs, fe, withRevisionLook(f.charFormat(), true, 2, author, insideInsertion)});
    }
    for (const Edit &ed : std::as_const(edits)) {
        t.setPosition(ed.from);
        t.setPosition(ed.to, QTextCursor::KeepAnchor);
        t.setCharFormat(ed.format);
    }
    auto removeAt = [&](int pos) {
        t.setPosition(pos);
        t.setPosition(pos + 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
    };
    auto insertPiece = [&](int pos, const QByteArray &startTag, bool open) {
        t.setPosition(pos);
        t.insertText(QString(QChar::ObjectReplacementCharacter), revisionPiece(startTag, QStringLiteral("del"), open, like));
    };
    if (endsBefore && beginsAfter) {
        removeAt(to);
        removeAt(from - 1);
    } else if (endsBefore) {
        removeAt(from - 1);
        QByteArray idTag = " w:id=\"" + idBefore.toLatin1() + "\"";
        insertPiece(to - 1, "<w:del" + idTag + ">", false);
    } else if (beginsAfter) {
        removeAt(to);
        insertPiece(from, tagAfter, true);
    } else {
        const QByteArray tag = revisionStartTag(doc, QStringLiteral("del"));
        insertPiece(to, tag, false);
        insertPiece(from, tag, true);
    }
    t.endEditBlock();
}

void markParagraphMark(WriteDoc &doc, QTextBlock block, const char *tag)
{
    QTextCharFormat m = markFormat(block);
    QVector<RawChild> extra = extraOf(m, prop::RawRPr);
    for (int i = int(extra.size()) - 1; i >= 0; --i) {
        const QByteArray name = elementName(extra[i].xml);
        if (name == "ins" || name == "del" || name == "moveFrom" || name == "moveTo") extra.removeAt(i);
    }
    QByteArray element = revisionStartTag(doc, QString::fromLatin1(tag));
    element.chop(1);
    element += "/>";
    extra.push_back({runChildRankOf(tag), element});
    setStored(m, prop::RawRPr, packRaw(extra));
    if (qstrcmp(tag, "del") == 0 || qstrcmp(tag, "moveFrom") == 0) m = withRevisionLook(m, true, 2, doc.reviewerIndex(doc.reviewer()));
    else m = withRevisionLook(m, false, qstrcmp(tag, "ins") == 0 ? 1 : 4, doc.reviewerIndex(doc.reviewer()));
    setMarkFormat(block, m);
}

bool paragraphMarkInsertedBy(const QTextBlock &block, const QString &author)
{
    const QVector<RawChild> extra = extraOf(markFormat(block), prop::RawRPr);
    const RawChild *rec = recordOf(extra, "ins");
    return rec && attributeOf(rec->xml, "author") == author;
}

void removeParagraphMark(WriteDoc &doc, QTextDocument *story, int blockNumber)
{
    const UntrackedEdit untracked;
    removeMark(doc, story, blockNumber);
}

QTextCharFormat withFormatRecord(WriteDoc &doc, const QString &paragraphStyle, const QTextCharFormat &old, QTextCharFormat now)
{
    if (old.hasProperty(prop::Atom) || old.hasProperty(prop::TrackedDeleted)) return now;
    const QVector<RawChild> had = extraOf(old, prop::RawRPr);
    if (recordOf(had, "rPrChange")) return now;   // (the first change of a run is the one it keeps the old properties of)
    RunProps was = doc.writtenRunProps(old, paragraphStyle);
    const QByteArray inner = runPropsXml(was);
    const QByteArray start = revisionStartTag(doc, QStringLiteral("rPrChange"));
    QVector<RawChild> extra = extraOf(now, prop::RawRPr);
    extra.push_back({runChildRankOf("rPrChange"), start + "<w:rPr>" + inner + "</w:rPr></w:rPrChange>"});
    setStored(now, prop::RawRPr, packRaw(extra));
    now.setProperty(tp::RevisionFormat, true);
    return now;
}

QTextBlockFormat withParagraphRecord(WriteDoc &doc, const QTextBlockFormat &old, QTextBlockFormat now)
{
    if (old.hasProperty(prop::RawBlock)) return now;
    if (recordOf(extraOf(old, prop::RawPPr), "pPrChange")) return now;
    const QString style = old.stringProperty(tp::StyleName).isEmpty() ? doc.defaultParagraphStyle() : old.stringProperty(tp::StyleName);
    const ParaProps full = paraPropsOf(old);
    ParaProps was = full.differences(doc.baselineParagraph(style, full.numId.value_or(0), full.ilvl.value_or(0), old.boolProperty(prop::NumDirect)));
    was.style = style == doc.defaultParagraphStyle() ? QString() : style;
    was.extra.clear();
    for (const RawChild &c : full.extra)
        if (elementName(c.xml) != "pPrChange") was.extra.push_back(c);
    const QByteArray inner = paraPropsXml(was);
    QByteArray start = revisionStartTag(doc, QStringLiteral("pPrChange"));
    QVector<RawChild> extra = extraOf(now, prop::RawPPr);
    extra.push_back({paraChildRank("pPrChange"), start + "<w:pPr>" + inner + "</w:pPr></w:pPrChange>"});
    setStored(now, prop::RawPPr, packRaw(extra));
    return now;
}

} // namespace jp::write
