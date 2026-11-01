// Tracked changes: finding, accepting, and rejecting them.

#include "core/reviewops.h"

#include "core/commentops.h"
#include "core/editops.h"
#include "core/tableops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QMap>
#include <QTextDocument>
#include <algorithm>
#include <atomic>

using namespace jp::office::wml;

namespace jp::write {

namespace {

std::atomic<qint64> s_blocksScanned{0}, s_runsVisited{0};
thread_local bool t_withEndnotes = false;   // (the words of the endnotes the pages set, which the changes in them are looked for in, while Accept All and Reject All go through them)

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
    if (d.isValid() && d.timeSpec() == Qt::LocalTime) d.setTimeSpec(Qt::UTC);
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

bool hasMarkOf(const QVector<RawChild> &extra, const char *name)
{
    for (const RawChild &c : extra)
        if (elementName(c.xml) == name) return true;
    return false;
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
            out->push_back(r);
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
QDateTime revisionDateOf(const QByteArray &startTag) { return parseDate(attributeOf(startTag, "date")); }

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

bool isOwnInsertion(const QTextDocument *story, int from, int to, const QString &author)
{
    const QTextBlock b = story->findBlock(from);
    if (!b.isValid() || to > b.position() + b.length() - 1) return false;
    const QVector<Pair> pairs = pairsOf(piecesOf(b));
    for (const Pair &p : pairs)
        if (p.tag == QLatin1String("ins") && p.open < from && p.close >= to && attributeOf(p.raw, "author") == author) {
            // (And no deletion by another person in it.)
            return true;
        }
    return false;
}

QVector<Revision> revisionsIn(const QTextDocument *story, int from, int to, int most)
{
    QVector<Revision> out;
    QTextBlock b = story->findBlock(std::max(0, from));
    for (; b.isValid() && (to < 0 || b.position() <= to) && out.size() < most; b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock) && !(t_withEndnotes && b.blockFormat().intProperty(prop::EndnoteNote) > 0)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        for (const Revision &r : std::as_const(inBlock)) {
            if (r.to < from || (to >= 0 && r.from > to)) continue;
            out.push_back(r);
        }
    }
    std::stable_sort(out.begin(), out.end(), [](const Revision &a, const Revision &b) { return a.from != b.from ? a.from < b.from : a.to > b.to; });
    if (out.size() > most) out.resize(most);
    return out;
}

int revisionCount(const QTextDocument *story)
{
    int n = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
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
        if (r.kind == Revision::ParagraphFormat && best) continue;
        if (!best || (r.to - r.from) < (best->to - best->from) || best->kind == Revision::ParagraphFormat) best = r;
    }
    return best;
}

std::optional<Revision> nextRevision(const QTextDocument *story, int position)
{
    for (QTextBlock b = story->findBlock(position); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        std::optional<Revision> best;
        for (const Revision &r : std::as_const(inBlock))
            if (r.from > position && (!best || r.from < best->from)) best = r;
        if (best) return best;
    }
    return std::nullopt;
}

std::optional<Revision> previousRevision(const QTextDocument *story, int position)
{
    for (QTextBlock b = story->findBlock(position); b.isValid(); b = b.previous()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        QVector<Revision> inBlock;
        addRevisionsOf(b, &inBlock);
        std::optional<Revision> best;
        for (const Revision &r : std::as_const(inBlock))
            if (r.from < position && (!best || r.from > best->from)) best = r;
        if (best) return best;
    }
    return std::nullopt;
}

QString revisionText(const QTextDocument *story, const Revision &r, int most)
{
    if (r.kind == Revision::Format || r.kind == Revision::ParagraphFormat || r.kind == Revision::MarkInserted || r.kind == Revision::MarkDeleted) return QString();
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
            const QVector<RawChild> kept = old.extra;
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

void resolveParagraphChange(WriteDoc &doc, const QTextBlock &b, bool accept)
{
    QTextBlockFormat bf = b.blockFormat();
    QVector<RawChild> extra = extraOf(bf, prop::RawPPr);
    const RawChild *rec = recordOf(extra, "pPrChange");
    if (!rec) return;
    QTextBlockFormat nf = bf;
    if (!accept) {
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
        // (A paragraph's list is its own to change with the list commands: the record's numbering is not put back.)
        auto listKey = [](int key) { return key == prop::NumId || key == prop::NumLevel || key == QTextFormat::ObjectIndex || key == tp::BulletChar || key == tp::ListLevel; };
        for (auto it = was.cbegin(); it != was.cend(); ++it)
            if (!will.contains(it.key()) && !listKey(it.key())) nf.clearProperty(it.key());
        for (auto it = will.cbegin(); it != will.cend(); ++it)
            if (!listKey(it.key())) nf.setProperty(it.key(), it.value());
    }
    QVector<RawChild> kept = extraOf(nf, prop::RawPPr);
    for (int i = int(kept.size()) - 1; i >= 0; --i)
        if (elementName(kept[i].xml) == "pPrChange") kept.removeAt(i);
    if (kept.isEmpty()) nf.clearProperty(prop::RawPPr);
    else setStored(nf, prop::RawPPr, packRaw(kept));
    QTextCursor t(b);
    t.setBlockFormat(nf);
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

bool onlyInvisible(const QTextBlock &b)
{
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        if (!isInvisibleAtom(cf)) return false;
    }
    return true;
}

// Takes a paragraph's mark away (it was deleted and the change is accepted, or inserted and the change is rejected): the paragraph is joined to the one after it, which keeps its own look
// (a paragraph with nothing left is gone). Where the paragraph can't be joined (the last of the story, one that ends a section, one of a table's cell with the next in another) the mark stays.
void removeMark(WriteDoc &doc, QTextDocument *d, int blockNumber)
{
    QTextBlock b = d->findBlockByNumber(blockNumber);
    if (!b.isValid()) return;
    const QTextBlock n = b.next();
    const QTextBlockFormat bf = b.blockFormat();
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


// A move has two halves, the text taken from a place and the same text put in another, joined by the name of the move that the marks of their ranges give. Where one half is among the changes
// to resolve, the other half is too.
QVector<Revision> withMovePairs(const QTextDocument *story, const QVector<Revision> &which)
{
    bool any = false;
    for (const Revision &r : which) any = any || r.kind == Revision::MovedFrom || r.kind == Revision::MovedTo;
    if (!any) return which;
    // The places of the marks of the ranges of moves, by name.
    struct Start { int pos; QString name; };
    QVector<Start> fromStarts, toStarts;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (a.kind != Inline::ParaAtom || a.role != Inline::Single) continue;
            if (a.tag == QLatin1String("moveFromRangeStart")) fromStarts.push_back({it.fragment().position(), attributeOf(a.raw, "name")});
            else if (a.tag == QLatin1String("moveToRangeStart")) toStarts.push_back({it.fragment().position(), attributeOf(a.raw, "name")});
        }
    }
    // (In the order of the story, so the one before a place is found by search.)
    auto nameBefore = [](const QVector<Start> &starts, int pos) {
        auto it = std::lower_bound(starts.begin(), starts.end(), pos, [](const Start &s, int v) { return s.pos < v; });
        return it == starts.begin() ? QString() : (it - 1)->name;
    };
    QHash<QString, int> fromByName, toByName;
    for (const Start &s : fromStarts) fromByName[s.name] = s.pos;
    for (const Start &s : toStarts) toByName[s.name] = s.pos;
    QVector<Revision> out = which;
    QSet<qint64> have;
    for (const Revision &r : which) have.insert(qint64(r.kind) << 40 | r.from);
    for (const Revision &r : which) {
        if (r.kind != Revision::MovedFrom && r.kind != Revision::MovedTo) continue;
        const bool isFrom = r.kind == Revision::MovedFrom;
        const QString name = nameBefore(isFrom ? fromStarts : toStarts, r.from);
        if (name.isEmpty()) continue;
        const int otherPos = (isFrom ? toByName : fromByName).value(name, -1);
        if (otherPos < 0) continue;
        // The first move of the other kind that begins after that mark.
        const Revision::Kind want = isFrom ? Revision::MovedTo : Revision::MovedFrom;
        std::optional<Revision> pair;
        const QTextBlock ob = story->findBlock(otherPos);
        for (const Revision &o : revisionsIn(story, otherPos, ob.position() + ob.length()))
            if (o.kind == want && o.from > otherPos && (!pair || o.from < pair->from)) pair = o;
        if (!pair || have.contains(qint64(pair->kind) << 40 | pair->from)) continue;
        have.insert(qint64(pair->kind) << 40 | pair->from);
        out.push_back(*pair);
    }
    return out;
}

int resolveRevisions(WriteDoc &doc, QTextDocument *story, const QVector<Revision> &whichGiven, bool accept)
{
    if (whichGiven.isEmpty()) return 0;
    const QVector<Revision> which = withMovePairs(story, whichGiven);
    QMap<int, QVector<Revision>> byBlock;
    for (const Revision &r : which) byBlock[r.block].push_back(r);
    EditScope scope(doc, story);
    const UntrackedEdit untracked;   // (accepting and rejecting are not themselves tracked)
    QTextCursor edit(story);
    edit.beginEditBlock();
    int done = 0;
    for (auto bi = byBlock.end(); bi != byBlock.begin();) {
        --bi;
        const int blockNumber = bi.key();
        QVector<Revision> inBlock = bi.value();
        // Inner changes before outer ones, later before earlier, so that the places of the rest stay: a change's end is moved by what the changes inside it did.
        std::stable_sort(inBlock.begin(), inBlock.end(), [](const Revision &a, const Revision &b) { return a.from != b.from ? a.from > b.from : a.to < b.to; });
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
        bool markDeleted = false, markInserted = false, structural = false;
        for (const Revision &r : std::as_const(inBlock)) {
            if (r.kind == Revision::MarkInserted) { markInserted = true; continue; }
            if (r.kind == Revision::MarkDeleted) { markDeleted = true; continue; }
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
                break;
            }
            case Revision::Format:
                break;   // (done above)
            case Revision::ParagraphFormat:
                resolveParagraphChange(doc, story->findBlockByNumber(blockNumber), accept);
                ++done;
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
        }
    }
    edit.endEditBlock();
    return done;
}

int resolveRevisionsIn(WriteDoc &doc, QTextDocument *story, int from, int to, bool accept)
{
    return resolveRevisions(doc, story, revisionsIn(story, from, to), accept);
}

namespace {

// The text boxes of a story: the changes in the XML that the pieces of a text box keep are accepted or rejected, and the box is made again from it. Returns how many boxes were changed.
int resolveTextBoxes(WriteDoc &doc, QTextDocument *story, bool accept)
{
    struct Box { int pos; Inline atom; QTextCharFormat format; };
    QVector<Box> boxes;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (a.kind != Inline::RunAtom || !a.raw.contains("txbxContent") || !xmlHasChanges(a.raw)) continue;
            boxes.push_back({it.fragment().position(), a, cf});
        }
    }
    int done = 0;
    QTextCursor t(story);
    for (const Box &box : std::as_const(boxes)) {
        Inline made = box.atom;
        made.raw = resolveChangesInXml(box.atom.raw, accept);
        if (made.raw == box.atom.raw) continue;
        QTextCharFormat f = box.format;
        f.clearProperty(flow::fp::AnchorId);
        f.clearProperty(tp::InlineObject);
        setStored(f, prop::Atom, packAtom(made));
        if (!doc.makeDrawnFormat(made, &f)) continue;
        t.setPosition(box.pos);
        t.setPosition(box.pos + 1, QTextCursor::KeepAnchor);
        t.setCharFormat(f);
        ++done;
    }
    if (done) doc.keepObjectsForTheHistory();
    return done;
}

// The words of the endnotes (each note that is edited in place): the changes in them, one note at a time.
int resolveEndnotes(WriteDoc &doc, bool accept)
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
        done += resolveRevisionsIn(doc, story, it->first, it->second, accept);
    }
    return done;
}

// Every change of the document, as a person expects: the text, the text boxes, the endnotes, and the headers, footers, and footnotes. One step of the history.
int resolveEverywhere(WriteDoc &doc, bool accept)
{
    WriteDoc::StepGroup group(doc);
    int n = resolveRevisionsIn(doc, doc.story(), 0, -1, accept);
    n += resolveTextBoxes(doc, doc.story(), accept);
    n += resolveEndnotes(doc, accept);
    for (QTextDocument *s : doc.storiesWithChanges()) {
        n += resolveRevisionsIn(doc, s, 0, -1, accept);
        n += resolveTextBoxes(doc, s, accept);
    }
    return n;
}

} // namespace

int acceptAll(WriteDoc &doc) { return resolveEverywhere(doc, true); }
int rejectAll(WriteDoc &doc) { return resolveEverywhere(doc, false); }

int resolveAtCursor(WriteDoc &doc, QTextCursor &c, bool accept)
{
    QTextDocument *story = c.document();
    QVector<Revision> which;
    if (c.hasSelection()) {
        which = revisionsIn(story, c.selectionStart(), c.selectionEnd());
    } else if (const std::optional<Revision> r = revisionAt(story, c.position())) {
        which.push_back(*r);
    }
    if (which.isEmpty()) {
        if (const std::optional<Revision> next = nextRevision(story, c.selectionEnd())) c.setPosition(next->from);
        return 0;
    }
    const int at = c.selectionStart();
    const int done = resolveRevisions(doc, story, which, accept);
    c.setPosition(std::min(at, story->characterCount() - 1));
    if (const std::optional<Revision> next = nextRevision(story, std::max(0, c.position() - 1))) c.setPosition(next->from);
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
