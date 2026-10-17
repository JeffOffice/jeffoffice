// Writing a .docx: a wml::Document into its package. Elements are written in the order ECMA-376 gives, the parts the document kept
// are written as they were read, and the pieces that were kept as XML but whose partner is gone are left out so that what is written
// is whole (a field with no end, or a bookmark with no start, would make a program that opens the file complain).

#include "wml/wml.h"

#include "office/package.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QHash>
#include <QSet>
#include <algorithm>
#include <vector>

namespace jp::office::wml {

namespace {

const char kXmlHead[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n";
const char kWNs[] = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
const char kRNs[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

// The qualified name at the start of a start tag ("w:document").
QByteArray qnameOf(const QByteArray &startTag)
{
    int i = 1;
    while (i < startTag.size() && startTag[i] != ' ' && startTag[i] != '>' && startTag[i] != '/' && startTag[i] != '\t' && startTag[i] != '\r' && startTag[i] != '\n') ++i;
    return startTag.mid(1, i - 1);
}

// The start tag with the word processing namespace declared on the prefix "w", which the elements written here use.
QByteArray withW(QByteArray tag)
{
    if (!tag.contains("xmlns:w=")) {
        const int at = int(qnameOf(tag).size()) + 1;
        tag.insert(at, QByteArray(" xmlns:w=\"") + kWNs + "\"");
    }
    return tag;
}

// The prefixes of the namespaces word processing files use besides the main ones, with their names. A piece kept as XML may come from
// another file (copied across), whose root declared a prefix that this file's does not; the prefixes the pieces use are declared on the root.
const char *const kKnownPrefixes[][2] = {
    {"wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"},
    {"a", "http://schemas.openxmlformats.org/drawingml/2006/main"},
    {"pic", "http://schemas.openxmlformats.org/drawingml/2006/picture"},
    {"c", "http://schemas.openxmlformats.org/drawingml/2006/chart"},
    {"dgm", "http://schemas.openxmlformats.org/drawingml/2006/diagram"},
    {"mc", "http://schemas.openxmlformats.org/markup-compatibility/2006"},
    {"m", "http://schemas.openxmlformats.org/officeDocument/2006/math"},
    {"v", "urn:schemas-microsoft-com:vml"},
    {"o", "urn:schemas-microsoft-com:office:office"},
    {"w10", "urn:schemas-microsoft-com:office:word"},
    {"w14", "http://schemas.microsoft.com/office/word/2010/wordml"},
    {"w15", "http://schemas.microsoft.com/office/word/2012/wordml"},
    {"wp14", "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"},
    {"wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape"},
    {"wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"},
    {"wpc", "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas"},
};

void collectPrefixes(const QByteArray &xml, QSet<QByteArray> *used)
{
    for (qsizetype at = xml.indexOf(':'); at > 0; at = xml.indexOf(':', at + 1)) {
        qsizetype from = at;
        while (from > 0 && ((xml[from - 1] >= 'a' && xml[from - 1] <= 'z') || (xml[from - 1] >= 'A' && xml[from - 1] <= 'Z') || (xml[from - 1] >= '0' && xml[from - 1] <= '9'))) --from;
        if (from == at || from == 0) continue;
        const char before = xml[from - 1];
        if (before != '<' && before != ' ' && before != '/') continue;
        used->insert(xml.mid(from, at - from));
    }
}

// The prefixes a start tag declares, read from its attributes (they may be separated by any white space).
QSet<QByteArray> declaredPrefixes(const QByteArray &tag)
{
    QSet<QByteArray> out;
    qsizetype i = 1;
    auto space = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    while (i < tag.size() && !space(tag[i]) && tag[i] != '>' && tag[i] != '/') ++i;
    while (i < tag.size()) {
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || tag[i] == '>' || tag[i] == '/') break;
        const qsizetype nameAt = i;
        while (i < tag.size() && !space(tag[i]) && tag[i] != '=' && tag[i] != '>') ++i;
        const QByteArray name = tag.mid(nameAt, i - nameAt);
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || tag[i] != '=') continue;
        ++i;
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || (tag[i] != '"' && tag[i] != '\'')) continue;
        const char quote = tag[i++];
        while (i < tag.size() && tag[i] != quote) ++i;
        ++i;
        if (name.startsWith("xmlns:")) out.insert(name.mid(6));
    }
    return out;
}

QByteArray withKnownPrefixes(QByteArray rootTag, const Document &doc)
{
    const QSet<QByteArray> declared = declaredPrefixes(rootTag);
    QSet<QByteArray> used;
    for (const Block &b : doc.body) {
        if (b.isRaw) {
            collectPrefixes(b.raw, &used);
            continue;
        }
        for (const Inline &a : b.para.content)
            if (a.kind != Inline::Text) collectPrefixes(a.raw, &used);
    }
    for (const auto &known : kKnownPrefixes) {
        const QByteArray prefix(known[0]);
        if (!used.contains(prefix) || declared.contains(prefix)) continue;
        rootTag.insert(int(qnameOf(rootTag).size()) + 1, " xmlns:" + prefix + "=\"" + known[1] + "\"");
    }
    return rootTag;
}

constexpr quint64 coord(int block, int inl) { return (quint64(block) << 32) | quint32(inl + 1); }

// The value (w:val) of the first element of `name` in a start of a table's row or cell kept as written, or `none`.
int rawVal(const QByteArray &raw, const char *name, int none = 0)
{
    const qsizetype at = raw.indexOf(QByteArray("<w:") + name);
    if (at < 0) return none;
    const qsizetype end = raw.indexOf('>', at), val = raw.indexOf("w:val=\"", at);
    if (end < 0 || val < 0 || val > end) return none;
    const qsizetype from = val + 7, to = raw.indexOf('"', from);
    bool ok = false;
    const int v = to > from ? raw.mid(from, to - from).toInt(&ok) : 0;
    return ok ? std::clamp(v, 0, 1000) : none;
}

// The start of a row (and its properties) with the number of grid columns it leaves empty at the end said to be `total`.
QByteArray withGridAfter(const QByteArray &raw, int total)
{
    const QByteArray element = "<w:gridAfter w:val=\"" + QByteArray::number(total) + "\"/>";
    if (const qsizetype at = raw.indexOf("<w:gridAfter"); at >= 0) {
        const qsizetype end = raw.indexOf('>', at);
        return end > at && raw[end - 1] == '/' ? raw.left(at) + element + raw.mid(end + 1) : raw;
    }
    if (const qsizetype at = raw.indexOf("<w:trPr>"); at >= 0) return raw.left(at + 8) + element + raw.mid(at + 8);
    if (raw.contains("trPr")) return raw;   // (in a prefix of its own, or with attributes: left as it is)
    return raw + "<w:trPr>" + element + "</w:trPr>";
}

// Which of the pieces kept as XML are written: the ones whose partners are there.
class Whole {
public:
    explicit Whole(const Document &doc)
    {
        const int n = int(doc.body.size());
        m_blocks.assign(n, true);
        m_inlines.resize(n);
        for (int b = 0; b < n; ++b) m_inlines[b].assign(doc.body[b].para.content.size(), true);

        // Tags with a start and an end (hyperlink, content controls...), in the body and in each paragraph. An end closes the nearest start of its
        // tag that is open; a start that is open above that one (a stray one, from a copy that took the start of a tag and not its end) is left
        // out, and the pair is kept.
        QVector<int> stack;
        for (int b = 0; b < n; ++b) {
            const Block &blk = doc.body[b];
            if (blk.isRaw) {
                if (blk.role == Inline::Open) {
                    stack.push_back(b);
                } else if (blk.role == Inline::Close) {
                    int at = int(stack.size()) - 1;
                    while (at >= 0 && doc.body[stack[at]].tag != blk.tag) --at;
                    if (at < 0) {
                        m_blocks[b] = false;
                    } else {
                        for (int k = at + 1; k < stack.size(); ++k) m_blocks[stack[k]] = false;
                        stack.resize(at);
                    }
                }
                continue;
            }
            QVector<int> inner;
            const auto &content = blk.para.content;
            for (int i = 0; i < content.size(); ++i) {
                if (content[i].kind != Inline::ParaAtom) continue;
                if (content[i].role == Inline::Open) {
                    inner.push_back(i);
                } else if (content[i].role == Inline::Close) {
                    int at = int(inner.size()) - 1;
                    while (at >= 0 && content[inner[at]].tag != content[i].tag) --at;
                    if (at < 0) {
                        m_inlines[b][i] = false;
                    } else {
                        for (int k = at + 1; k < inner.size(); ++k) m_inlines[b][inner[k]] = false;
                        inner.resize(at);
                    }
                }
            }
            for (int i : std::as_const(inner)) m_inlines[b][i] = false;
        }
        for (int b : std::as_const(stack)) m_blocks[b] = false;

        // Bookmarks and ranges begin before they end. A start and an end may be on either side of a piece kept as XML (a bookmark that begins in
        // a paragraph and ends in a table): what is inside such a piece is read as written, and can't be left out, but its partner can be
        // found. A start that has no end, and an end that has no start, are left out. An Id used by two bookmarks is a mistake that a
        // program may have made (it is no use for telling them apart): the second bookmark stays and has an Id of its own; and so does the
        // second revision mark (w:ins, w:del) of an Id.
        struct Event { QString kind, id; bool start; int block, inl; bool droppable; };
        QVector<Event> events;
        long long mostId = 0;
        auto noteId = [&](const QString &id) {
            bool ok = false;
            const long long v = id.toLongLong(&ok);
            if (ok) mostId = std::max(mostId, v);
        };
        auto base = [](const QString &tag, bool *start) {
            static const char *const pairs[][2] = {{"bookmarkStart", "bookmarkEnd"},
                                                   {"commentRangeStart", "commentRangeEnd"},
                                                   {"permStart", "permEnd"},
                                                   {"moveFromRangeStart", "moveFromRangeEnd"},
                                                   {"moveToRangeStart", "moveToRangeEnd"}};
            for (const auto &p : pairs) {
                if (tag == QLatin1String(p[0])) { *start = true; return QString::fromLatin1(p[0]); }
                if (tag == QLatin1String(p[1])) { *start = false; return QString::fromLatin1(p[0]); }
            }
            return QString();
        };
        auto isRevision = [](const QString &tag) { return tag == QLatin1String("ins") || tag == QLatin1String("del") || tag == QLatin1String("moveFrom") || tag == QLatin1String("moveTo"); };
        QVector<Event> revisions;
        auto add = [&](const QString &tag, const QString &id, int block, int inl, bool droppable) {
            noteId(id);
            bool start = false;
            const QString kind = base(tag, &start);
            if (!kind.isEmpty()) events.push_back({kind, id, start, block, inl, droppable});
        };
        auto scanRaw = [&](const QByteArray &raw, int block) {
            if (!raw.contains("bookmark") && !raw.contains("commentRange") && !raw.contains("perm") && !raw.contains("moveFromRange") && !raw.contains("moveToRange") && !raw.contains("w:id")) return;
            for (qsizetype at = raw.indexOf('<'); at >= 0; at = raw.indexOf('<', at + 1)) {
                qsizetype e = at + 1;
                while (e < raw.size() && raw[e] != ' ' && raw[e] != '/' && raw[e] != '>' && raw[e] != '\r' && raw[e] != '\n' && raw[e] != '\t') ++e;
                const QByteArray qname = raw.mid(at + 1, e - at - 1);
                const QByteArray local = qname.mid(qname.indexOf(':') + 1);
                const QString tag = QString::fromLatin1(local);
                bool start = false;
                const bool pair = !base(tag, &start).isEmpty();
                if (!pair && !isRevision(tag)) continue;
                const qsizetype close = raw.indexOf('>', e);
                if (close < 0) break;
                const qsizetype idAt = raw.indexOf(":id=\"", e);
                QString id;
                if (idAt >= 0 && idAt < close) {
                    const qsizetype from = idAt + 5, to = raw.indexOf('"', from);
                    if (to > from && to <= close) id = QString::fromLatin1(raw.mid(from, to - from));
                }
                noteId(id);
                if (pair) events.push_back({base(tag, &start), id, start, block, -2, false});
            }
        };
        for (int b = 0; b < n; ++b) {
            const Block &blk = doc.body[b];
            if (blk.isRaw) {
                if (blk.role == Inline::Open || blk.role == Inline::Close) {
                    if (blk.role == Inline::Open && isRevision(blk.tag)) {
                        noteId(blk.key);
                        revisions.push_back({blk.tag, blk.key, true, b, -1, true});
                    }
                } else {
                    bool start = false;
                    if (!base(blk.tag, &start).isEmpty()) add(blk.tag, blk.key, b, -1, true);
                    else scanRaw(blk.raw, b);
                }
                continue;
            }
            for (int i = 0; i < blk.para.content.size(); ++i) {
                const Inline &a = blk.para.content[i];
                if (a.kind == Inline::ParaAtom && a.role == Inline::Single) {
                    add(a.tag, a.key, b, i, true);
                } else if (a.kind == Inline::ParaAtom && a.role == Inline::Open && isRevision(a.tag)) {
                    noteId(a.key);
                    revisions.push_back({a.tag, a.key, true, b, i, true});
                }
            }
        }
        auto drop = [&](int b, int i) {
            if (i == -1) m_blocks[b] = false;
            else if (i >= 0) m_inlines[b][i] = false;
        };
        // The starts and ends of each Id, paired the way they nest (an end closes the last start of its Id that is open).
        struct Pair { Event start; const Event *end = nullptr; };
        QVector<Pair> pairs;
        QHash<QString, QVector<int>> open;   // the pairs that have no end yet, by kind and Id
        for (const Event &ev : std::as_const(events)) {
            const QString key = ev.kind + QLatin1Char('/') + ev.id;
            if (ev.start) {
                open[key].push_back(int(pairs.size()));
                pairs.push_back({ev, nullptr});
            } else {
                auto it = open.find(key);
                if (it == open.end() || it->isEmpty()) {
                    if (ev.droppable) drop(ev.block, ev.inl);   // an end with no start
                    continue;
                }
                pairs[it->takeLast()].end = &ev;
            }
        }
        // A start with no end is left out (one inside a kept piece can't be).
        QHash<QString, QVector<int>> byId;
        for (int i = 0; i < pairs.size(); ++i) {
            const Pair &pr = pairs[i];
            if (!pr.end) {
                if (pr.start.droppable) drop(pr.start.block, pr.start.inl);
                continue;
            }
            byId[pr.start.kind + QLatin1Char('/') + pr.start.id].push_back(i);
        }
        // An Id used by more than one bookmark: the one that has an end inside a kept piece (whose Id can't change), or the first, keeps it; each
        // of the others has an Id of its own if it can (its start and its end are both in paragraphs), and else is left out. The others of a
        // kind that is tied to what has the Id (comment ranges) are left out.
        for (auto it = byId.cbegin(); it != byId.cend(); ++it) {
            if (it->size() < 2) continue;
            int owner = it->first();
            for (int i : it.value())
                if (!pairs[i].start.droppable || !pairs[i].end->droppable) {
                    owner = i;
                    break;
                }
            for (int i : it.value()) {
                if (i == owner) continue;
                const Pair &pr = pairs[i];
                if (pr.start.kind == QLatin1String("bookmarkStart") && pr.start.droppable && pr.end->droppable) {
                    const QByteArray id = QByteArray::number(++mostId);
                    m_newId.insert(coord(pr.start.block, pr.start.inl), id);
                    m_newId.insert(coord(pr.end->block, pr.end->inl), id);
                } else {
                    if (pr.start.droppable) drop(pr.start.block, pr.start.inl);
                    if (pr.end->droppable) drop(pr.end->block, pr.end->inl);
                }
            }
        }
        // (An end that came for a start of a raw piece, and the like, that never ended, is the piece's own.)
        QSet<QString> usedRevisions;
        for (const Event &r : std::as_const(revisions)) {
            if (r.id.isEmpty()) continue;
            if (usedRevisions.contains(r.id)) {
                m_newId.insert(coord(r.block, r.inl), QByteArray::number(++mostId));
            } else {
                usedRevisions.insert(r.id);
            }
        }

        // Fields: a begin, its code, its separator, and its end go together or not at all.
        struct Field { QVector<QPair<int, int>> pieces; bool separated = false; };
        QVector<Field> fields;
        for (int b = 0; b < n; ++b) {
            const auto &content = doc.body[b].para.content;
            for (int i = 0; i < content.size(); ++i) {
                const Inline &a = content[i];
                if (a.kind != Inline::RunAtom || (a.tag != QLatin1String("fldChar") && a.tag != QLatin1String("instrText"))) continue;
                m_inlines[b][i] = false;
                if (a.tag == QLatin1String("instrText")) {
                    if (!fields.isEmpty() && !fields.back().separated) fields.back().pieces.push_back({b, i});
                } else if (a.key == QLatin1String("begin")) {
                    if (fields.size() < 200) fields.push_back(Field{{{b, i}}, false});
                } else if (a.key == QLatin1String("separate")) {
                    if (!fields.isEmpty() && !fields.back().separated) {
                        fields.back().separated = true;
                        fields.back().pieces.push_back({b, i});
                    }
                } else if (a.key == QLatin1String("end")) {
                    if (!fields.isEmpty()) {
                        fields.back().pieces.push_back({b, i});
                        for (const auto &piece : std::as_const(fields.back().pieces)) m_inlines[piece.first][piece.second] = true;
                        fields.removeLast();
                    }
                }
            }
        }

        // A row whose cells and gaps stop short of the table's grid: a reader repairs it. It says it leaves the end of the grid empty.
        struct Frame { int grid = 0, row = -1, covered = 0, cells = 0, after = 0; };
        QVector<Frame> tables;
        for (int b = 0; b < n; ++b) {
            const Block &blk = doc.body[b];
            if (!blk.isRaw || !m_blocks[b] || blk.role == Inline::Single) continue;
            const bool opens = blk.role == Inline::Open;
            if (blk.tag == QLatin1String("tbl")) {
                if (opens) tables.push_back(Frame{int(blk.raw.count("<w:gridCol")), -1, 0, 0, 0});
                else if (!tables.isEmpty()) tables.removeLast();
            } else if (blk.tag == QLatin1String("tr") && !tables.isEmpty()) {
                Frame &t = tables.last();
                if (opens) {
                    t.row = b;
                    t.after = rawVal(blk.raw, "gridAfter");
                    t.covered = rawVal(blk.raw, "gridBefore") + t.after;
                    t.cells = 0;
                } else {
                    if (t.row >= 0 && t.cells > 0 && t.grid > 0 && t.covered < t.grid) m_gridAfter.insert(t.row, t.after + t.grid - t.covered);
                    t.row = -1;
                }
            } else if (blk.tag == QLatin1String("tc") && opens && !tables.isEmpty() && tables.last().row >= 0) {
                ++tables.last().cells;
                tables.last().covered += std::max(1, rawVal(blk.raw, "gridSpan", 1));
            }
        }
    }

    bool block(int b) const { return m_blocks[b]; }
    // The start of the row at block `b` as it is written: the same, unless the row stops short of the grid.
    QByteArray rowStart(int b, const QByteArray &raw) const
    {
        const auto it = m_gridAfter.constFind(b);
        return it == m_gridAfter.cend() ? raw : withGridAfter(raw, it.value());
    }
    bool inlineKept(int b, int i) const { return m_inlines[b][i]; }
    // The Id a kept piece is written with when it isn't the one it was read with (inl -1 for a body element); empty for none.
    QByteArray newId(int b, int i) const
    {
        const auto it = m_newId.constFind(coord(b, i));
        return it == m_newId.cend() ? QByteArray() : it.value();
    }

private:
    std::vector<bool> m_blocks;
    QVector<std::vector<bool>> m_inlines;
    QHash<quint64, QByteArray> m_newId;
    QHash<int, int> m_gridAfter;
};

// A kept element's text with the value of its first Id attribute replaced.
QByteArray withId(const QByteArray &raw, const QByteArray &id)
{
    if (id.isEmpty()) return raw;
    const qsizetype tagEnd = raw.indexOf('>');
    const qsizetype at = raw.indexOf(":id=\"");
    if (at < 0 || (tagEnd >= 0 && at > tagEnd)) return raw;
    const qsizetype from = at + 5, to = raw.indexOf('"', from);
    if (to < 0) return raw;
    return raw.left(from) + id + raw.mid(to);
}

// Text made fit for a w:t: the characters XML has no place for left out.
QString xmlSafe(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u < 0x20 && u != 9 && u != 10 && u != 13) continue;
        if (u == 0xFFFE || u == 0xFFFF) continue;
        out += c;
    }
    return out;
}

void writeRunText(QByteArray &out, const RunProps &props, const QString &text)
{
    const QByteArray rpr = runPropsXml(props);
    out += "<w:r>";
    if (!rpr.isEmpty()) out += "<w:rPr>" + rpr + "</w:rPr>";
    QString seg;
    auto flush = [&] {
        if (seg.isEmpty()) return;
        out += "<w:t xml:space=\"preserve\">" + escapeXml(xmlSafe(seg)) + "</w:t>";
        seg.clear();
    };
    for (const QChar c : text) {
        const ushort u = c.unicode();
        if (u == '\t') { flush(); out += "<w:tab/>"; }
        else if (u == QChar::LineSeparator || u == '\n') { flush(); out += "<w:br/>"; }
        else if (u == 0x000C) { flush(); out += "<w:br w:type=\"page\"/>"; }
        else if (u == 0x2011) { flush(); out += "<w:noBreakHyphen/>"; }
        else if (u == 0x00AD) { flush(); out += "<w:softHyphen/>"; }
        else seg += c;
    }
    flush();
    out += "</w:r>";
}

// Block `b` of a body as XML. `whole` says which pieces kept as XML are written (all of them without).
void writeBlock(QByteArray &out, const Block &blk, qsizetype b, const Whole *whole)
{
    {
        if (whole && !whole->block(int(b))) return;
        if (blk.isRaw) {
            out += whole ? withId(blk.tag == QLatin1String("tr") && blk.role == Inline::Open ? whole->rowStart(int(b), blk.raw) : blk.raw, whole->newId(int(b), -1)) : blk.raw;
            return;
        }
        out += "<w:p" + blk.para.attrs + ">";
        const QByteArray ppr = paraPropsXml(blk.para.props);
        if (!ppr.isEmpty()) out += "<w:pPr>" + ppr + "</w:pPr>";
        const auto &content = blk.para.content;
        // Runs of text with the same properties are one run (those that pieces kept as XML, left out above, had kept apart, too).
        QString pending;
        const RunProps *pendingProps = nullptr;
        auto flush = [&] {
            if (!pendingProps) return;
            writeRunText(out, *pendingProps, pending);
            pendingProps = nullptr;
            pending.clear();
        };
        for (int i = 0; i < content.size(); ++i) {
            if (whole && !whole->inlineKept(int(b), i)) continue;
            const Inline &a = content[i];
            if (a.kind == Inline::Text) {
                if (pendingProps && *pendingProps == a.props) {
                    pending += a.text;
                } else {
                    flush();
                    pendingProps = &a.props;
                    pending = a.text;
                }
                continue;
            }
            flush();
            if (a.kind == Inline::RunAtom) {
                const QByteArray rpr = runPropsXml(a.props);
                out += "<w:r>";
                if (!rpr.isEmpty()) out += "<w:rPr>" + rpr + "</w:rPr>";
                out += a.raw + "</w:r>";
            } else {
                out += whole ? withId(a.raw, whole->newId(int(b), i)) : a.raw;
            }
        }
        flush();
        out += "</w:p>";
    }
}

// The blocks [from, to) of a body as XML.
void writeBlocks(QByteArray &out, const QVector<Block> &body, qsizetype from, qsizetype to, const Whole *whole)
{
    for (qsizetype b = from; b < to; ++b) writeBlock(out, body[b], b, whole);
}

QByteArray documentXml(const Document &doc)
{
    const Whole whole(doc);
    QByteArray out;
    out.reserve(1 << 16);
    out += kXmlHead;
    QByteArray root = doc.rootStart;
    if (root.isEmpty()) root = QByteArray("<w:document xmlns:w=\"") + kWNs + "\" xmlns:r=\"" + kRNs + "\">";
    root = withKnownPrefixes(withW(root), doc);
    out += root;
    out += doc.beforeBody;
    out += "<w:body>";
    const qsizetype n = doc.body.size();
    for (qsizetype b = 0; b < n; ++b) {
        const Block &blk = doc.body[b];
        // A table that is as it was read (its blocks come to the fingerprint they had) is written as it was: what the writing of a body leaves out or gives an Id of its own (a
        // bookmark with no end, a second of an Id) is the file's own in a table nobody changed, and the fingerprint was taken of the blocks as they are.
        if (blk.isRaw && blk.role == Inline::Open && blk.tag == QLatin1String("tbl") && blk.table && !blk.table->original.isEmpty() && whole.block(int(b))) {
            qsizetype depth = 0, end = b;
            for (; end < n; ++end) {
                const Block &x = doc.body[end];
                if (!x.isRaw || x.tag != QLatin1String("tbl")) continue;
                depth += x.role == Inline::Open ? 1 : x.role == Inline::Close ? -1 : 0;
                if (depth == 0) break;
            }
            if (end < n) {
                QByteArray piece;
                writeBlocks(piece, doc.body, b, end + 1, nullptr);
                if (QCryptographicHash::hash(piece, QCryptographicHash::Sha1) == blk.table->fingerprint) {
                    out += blk.table->original;
                    b = end;
                    continue;
                }
            }
        }
        writeBlocks(out, doc.body, b, b + 1, &whole);
    }
    out += sectionPropsXml(doc.lastSection);
    out += "</w:body>";
    out += doc.afterBody;
    out += "</" + qnameOf(root) + ">";
    return out;
}

QByteArray stylesXml(const StyleSheet &sheet)
{
    if (!sheet.original.isEmpty() && !sheet.modified) return sheet.original;
    QByteArray out = kXmlHead;
    QByteArray root = sheet.rootStart.isEmpty() ? QByteArray("<w:styles xmlns:w=\"") + kWNs + "\">" : sheet.rootStart;
    root = withW(root);
    out += root;
    if (!sheet.docDefaults.isEmpty()) {
        out += sheet.docDefaults;
    } else {
        out += "<w:docDefaults>";
        const QByteArray r = runPropsXml(sheet.defaultRun), p = paraPropsXml(sheet.defaultPara);
        if (!r.isEmpty()) out += "<w:rPrDefault><w:rPr>" + r + "</w:rPr></w:rPrDefault>";
        if (!p.isEmpty()) out += "<w:pPrDefault><w:pPr>" + p + "</w:pPr></w:pPrDefault>";
        out += "</w:docDefaults>";
    }
    out += sheet.latentStyles;
    for (const Style &s : sheet.styles) {
        if (!s.raw.isEmpty()) {
            out += s.raw;
            continue;
        }
        out += "<w:style w:type=\"" + escapeXml(s.type, true) + "\"";
        if (s.isDefault) out += " w:default=\"1\"";
        if (s.customStyle) out += " w:customStyle=\"1\"";
        out += " w:styleId=\"" + escapeXml(s.id, true) + "\">";
        out += "<w:name w:val=\"" + escapeXml(s.name.isEmpty() ? s.id : s.name, true) + "\"/>";
        if (!s.basedOn.isEmpty()) out += "<w:basedOn w:val=\"" + escapeXml(s.basedOn, true) + "\"/>";
        if (!s.next.isEmpty()) out += "<w:next w:val=\"" + escapeXml(s.next, true) + "\"/>";
        if (!s.link.isEmpty()) out += "<w:link w:val=\"" + escapeXml(s.link, true) + "\"/>";
        if (s.uiPriority) out += "<w:uiPriority w:val=\"" + QByteArray::number(*s.uiPriority) + "\"/>";
        if (s.semiHidden) out += "<w:semiHidden/>";
        if (s.unhideWhenUsed) out += "<w:unhideWhenUsed/>";
        if (s.qFormat) out += "<w:qFormat/>";
        const QByteArray p = paraPropsXml(s.pPr), r = runPropsXml(s.rPr);
        if (!p.isEmpty()) out += "<w:pPr>" + p + "</w:pPr>";
        if (!r.isEmpty()) out += "<w:rPr>" + r + "</w:rPr>";
        out += s.tail;
        out += "</w:style>";
    }
    out += "</" + qnameOf(root) + ">";
    return out;
}

QByteArray numberingXml(const Numbering &num)
{
    if (!num.original.isEmpty() && !num.modified) return num.original;
    QByteArray out = kXmlHead;
    QByteArray root = num.rootStart.isEmpty() ? QByteArray("<w:numbering xmlns:w=\"") + kWNs + "\">" : num.rootStart;
    root = withW(root);
    out += root;
    out += num.prefix;
    for (const AbstractNum &a : num.abstracts) {
        if (!a.raw.isEmpty()) {
            out += a.raw;
            continue;
        }
        out += "<w:abstractNum w:abstractNumId=\"" + QByteArray::number(a.id) + "\">" + a.head;
        if (a.head.isEmpty()) out += "<w:multiLevelType w:val=\"" + escapeXml(a.multiLevelType.isEmpty() ? QStringLiteral("hybridMultilevel") : a.multiLevelType, true) + "\"/>";
        for (int i = 0; i < 9; ++i) {
            const Level &l = a.levels[i];
            if (!l.present) continue;
            out += "<w:lvl w:ilvl=\"" + QByteArray::number(i) + "\">";
            out += "<w:start w:val=\"" + QByteArray::number(l.start) + "\"/>";
            out += "<w:numFmt w:val=\"" + escapeXml(l.numFmt, true) + "\"/>";
            if (l.restart) out += "<w:lvlRestart w:val=\"" + QByteArray::number(*l.restart) + "\"/>";
            if (!l.pStyle.isEmpty()) out += "<w:pStyle w:val=\"" + escapeXml(l.pStyle, true) + "\"/>";
            if (l.legal) out += "<w:isLgl/>";
            if (l.suff != QLatin1String("tab")) out += "<w:suff w:val=\"" + escapeXml(l.suff, true) + "\"/>";
            out += "<w:lvlText w:val=\"" + escapeXml(l.text, true) + "\"/>";
            out += "<w:lvlJc w:val=\"" + escapeXml(l.jc, true) + "\"/>";
            const QByteArray p = paraPropsXml(l.pPr), r = runPropsXml(l.rPr);
            if (!p.isEmpty()) out += "<w:pPr>" + p + "</w:pPr>";
            if (!r.isEmpty()) out += "<w:rPr>" + r + "</w:rPr>";
            out += "</w:lvl>";
        }
        out += a.tail + "</w:abstractNum>";
    }
    for (const Num &n : num.nums) {
        if (!n.raw.isEmpty()) {
            out += n.raw;
            continue;
        }
        out += "<w:num w:numId=\"" + QByteArray::number(n.id) + "\"><w:abstractNumId w:val=\"" + QByteArray::number(n.abstractId) + "\"/>";
        for (auto it = n.startOverrides.cbegin(); it != n.startOverrides.cend(); ++it)
            out += "<w:lvlOverride w:ilvl=\"" + QByteArray::number(it.key()) + "\"><w:startOverride w:val=\"" + QByteArray::number(it.value()) + "\"/></w:lvlOverride>";
        out += "</w:num>";
    }
    out += num.suffix;
    out += "</" + qnameOf(root) + ">";
    return out;
}

} // namespace

QByteArray blocksFingerprint(const QVector<Block> &body, qsizetype from, qsizetype to)
{
    QByteArray piece;
    writeBlocks(piece, body, from, to, nullptr);
    return QCryptographicHash::hash(piece, QCryptographicHash::Sha1);
}

BlocksFingerprint::BlocksFingerprint() : m_hash(QCryptographicHash::Sha1) {}

void BlocksFingerprint::add(const Block &block)
{
    m_piece.clear();
    writeBlock(m_piece, block, 0, nullptr);
    m_hash.addData(QByteArrayView(m_piece));
}

QByteArray BlocksFingerprint::finish() { return m_hash.result(); }

QByteArray writeDocx(const Document &doc, QString *error)
{
    PackageWriter w;
    // The extensions whose parts all have one type are declared once.
    QHash<QString, QString> extensionType;
    QSet<QString> mixed;
    for (const KeptPart &p : doc.parts) {
        const QString ext = p.name.section(QLatin1Char('.'), -1).toLower();
        if (ext.isEmpty() || ext == QLatin1String("xml") || ext == QLatin1String("rels")) continue;
        if (!extensionType.contains(ext)) extensionType.insert(ext, p.contentType);
        else if (extensionType.value(ext) != p.contentType) mixed.insert(ext);
    }
    for (auto it = extensionType.cbegin(); it != extensionType.cend(); ++it)
        if (!mixed.contains(it.key()) && !it.value().isEmpty()) w.addDefault(it.key(), it.value());

    static const char mainType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml";
    w.addPart(doc.mainName, documentXml(doc), doc.mainContentType.isEmpty() ? QString::fromLatin1(mainType) : doc.mainContentType);
    if (!doc.stylesName.isEmpty())
        w.addPart(doc.stylesName, stylesXml(doc.styles), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    if (!doc.numberingName.isEmpty())
        w.addPart(doc.numberingName, numberingXml(doc.numbering), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
    for (const KeptPart &p : doc.parts) w.addPart(p.name, p.data, p.contentType);

    for (const Relationship &r : doc.packageRelationships) w.addRelationshipWithId(QStringLiteral("/"), r.id, r.type, r.target, r.external);
    for (const Relationship &r : doc.mainRelationships) w.addRelationshipWithId(doc.mainName, r.id, r.type, r.target, r.external);
    for (auto it = doc.partRelationships.cbegin(); it != doc.partRelationships.cend(); ++it)
        for (const Relationship &r : it.value()) w.addRelationshipWithId(it.key(), r.id, r.type, r.target, r.external);
    const QByteArray zip = w.finish();
    if (zip.isEmpty() && error) *error = QCoreApplication::translate("jp::office::wml", "The document could not be packaged.");
    return zip;
}

} // namespace jp::office::wml
