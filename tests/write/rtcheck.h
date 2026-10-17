#pragma once
// The checks that a saved .docx is a faithful, well-formed copy of what was opened, used by the tests and by the developer's round-trip hook.
// They compare more than the text and the section count (which let real losses through): every XML part JeffWrite wrote is parsed strictly,
// the parts, content types, and relationships are compared, and so are the paragraphs one by one (their attributes, the properties kept as
// XML, the bookmarks, comment ranges, hyperlinks with their relationship ids, content controls, tracked changes, and fields), the kept tables,
// and the sections with their properties.

#include "core/editops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"
#include "text/textprops.h"

#include <QDebug>
#include <QHash>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <QXmlStreamReader>
#include <algorithm>
#include <functional>

namespace rt {

namespace office = jp::office;

// ---- a parsed XML part

struct Node {
    QString name;                                   // "{namespace}local"
    QVector<QPair<QString, QString>> attrs;         // by name, sorted
    QVector<Node> kids;
    QString text;
};

// Parses strictly (QXmlStreamReader's own checks: well-formed, no duplicate attribute, every prefix bound). False, with the reason, if it isn't.
inline bool parseTree(const QByteArray &bytes, Node *root, QString *error)
{
    QXmlStreamReader r(bytes);
    QVector<Node *> stack;
    Node top;
    stack.push_back(&top);
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType t = r.readNext();
        if (t == QXmlStreamReader::StartElement) {
            // (A namespace declared twice on one element is a duplicate attribute, which the reader's own checks let by.)
            QSet<QString> declared;
            for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations())
                if (declared.contains(d.prefix().toString())) {
                    if (error) *error = QStringLiteral("xmlns:%1 is declared twice on <%2> (line %3)").arg(d.prefix(), r.name().toString()).arg(r.lineNumber());
                    return false;
                } else {
                    declared.insert(d.prefix().toString());
                }
            if (stack.size() > 1500) {   // (the trees are walked and dropped by recursion: what is deeper than any real document is not compared)
                if (error) *error = QStringLiteral("nested too deeply to compare");
                return false;
            }
            Node n;
            n.name = QLatin1Char('{') + r.namespaceUri().toString() + QLatin1Char('}') + r.name().toString();
            for (const QXmlStreamAttribute &a : r.attributes())
                n.attrs.push_back({QLatin1Char('{') + a.namespaceUri().toString() + QLatin1Char('}') + a.name().toString(), a.value().toString()});
            std::sort(n.attrs.begin(), n.attrs.end());
            stack.back()->kids.push_back(std::move(n));
            stack.push_back(&stack.back()->kids.last());
        } else if (t == QXmlStreamReader::EndElement) {
            stack.pop_back();
        } else if (t == QXmlStreamReader::Characters && stack.size() > 1) {
            stack.back()->text += r.text();
        }
    }
    if (r.hasError()) {
        if (error) *error = QStringLiteral("%1 (line %2, column %3)").arg(r.errorString()).arg(r.lineNumber()).arg(r.columnNumber());
        return false;
    }
    if (top.kids.size() != 1) {
        if (error) *error = QStringLiteral("no single root element");
        return false;
    }
    *root = std::move(top.kids[0]);
    return true;
}

constexpr const char *kW = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}";

inline QString wName(const char *local) { return QString::fromLatin1(kW) + QLatin1String(local); }
inline bool isW(const Node &n, const char *local) { return n.name == wName(local); }

inline QString shortName(QString s)
{
    s.replace(QString::fromLatin1(kW), QStringLiteral("w:"));
    s.replace(QStringLiteral("{http://schemas.openxmlformats.org/officeDocument/2006/relationships}"), QStringLiteral("r:"));
    s.replace(QStringLiteral("{http://schemas.microsoft.com/office/word/2010/wordml}"), QStringLiteral("w14:"));
    s.replace(QStringLiteral("{http://www.w3.org/XML/1998/namespace}"), QStringLiteral("xml:"));
    return s;
}

// The element and everything in it, as text that doesn't depend on prefixes, attribute order, or the xml:space of text.
inline void canonical(const Node &n, QString *out)
{
    *out += QLatin1Char('<') + shortName(n.name);
    const bool textual = isW(n, "t") || isW(n, "delText") || isW(n, "instrText");
    const bool columns = isW(n, "cols");
    for (const auto &a : n.attrs) {
        if (textual && a.first.endsWith(QLatin1String("}space"))) continue;
        // (Columns: an equal width and no line between them are what is meant when nothing is said, said or not.)
        if (columns && a.first.endsWith(QLatin1String("}equalWidth")) && (a.second == QLatin1String("true") || a.second == QLatin1String("1") || a.second == QLatin1String("on"))) continue;
        if (columns && a.first.endsWith(QLatin1String("}sep")) && (a.second == QLatin1String("false") || a.second == QLatin1String("0") || a.second == QLatin1String("off"))) continue;
        if (columns && a.first.endsWith(QLatin1String("}sep"))) {   // (said as "true", "1", or "on": a line between)
            *out += QStringLiteral(" w:sep=\"1\"");
            continue;
        }
        if (isW(n, "pgMar") && (a.first.endsWith(QLatin1String("}header")) || a.first.endsWith(QLatin1String("}footer"))) && a.second == QLatin1String("720")) continue;   // (and what is meant when nothing is said)
        if (isW(n, "pgMar") && a.first.endsWith(QLatin1String("}gutter")) && a.second == QLatin1String("0")) continue;
        if (columns && a.first.endsWith(QLatin1String("}space")) && a.second == QLatin1String("720")) continue;   // (the space between columns, when nothing is said)
        if (isW(n, "col") && a.first.endsWith(QLatin1String("}space")) && a.second == QLatin1String("0")) continue;
        if (columns && a.first.endsWith(QLatin1String("}equalWidth"))) {   // (said as "false", "0", or "off": unequal)
            *out += QStringLiteral(" w:equalWidth=\"0\"");
            continue;
        }
        *out += QLatin1Char(' ') + shortName(a.first) + QLatin1String("=\"") + a.second + QLatin1Char('"');
    }
    *out += QLatin1Char('>');
    *out += n.text.trimmed().isEmpty() && !textual ? QString() : n.text;
    for (const Node &k : n.kids) canonical(k, out);
    *out += QStringLiteral("</>");
}

inline QString canonical(const Node &n)
{
    QString s;
    canonical(n, &s);
    return s;
}

// ---- strict parsing of what was written

inline bool isXmlPart(const office::Package &p, const QString &name)
{
    const QString type = p.contentType(name);
    return type.endsWith(QLatin1String("+xml")) || type.endsWith(QLatin1String("/xml")) || name.endsWith(QLatin1String(".xml")) || name.endsWith(QLatin1String(".rels"));
}

// ---- the shape of the tables a part holds: what a word processor needs of a table

namespace detail {

inline void collectTables(const Node &n, QVector<const Node *> *out)
{
    if (isW(n, "tbl")) out->push_back(&n);
    for (const Node &k : n.kids) collectTables(k, out);
}

inline int valOf(const Node &n)
{
    for (const auto &a : n.attrs)
        if (a.first.endsWith(QLatin1String("}val"))) return a.second.toInt();
    return 0;
}

// The cell's properties: the grid columns it spans, and whether it joins the one above (a restart starts a chain).
struct CellShape {
    int span = 1;
    bool joins = false, restarts = false;
};

inline CellShape shapeOfCell(const Node &tc)
{
    CellShape s;
    for (const Node &pr : tc.kids) {
        if (!isW(pr, "tcPr")) continue;
        for (const Node &x : pr.kids) {
            if (isW(x, "gridSpan")) s.span = std::max(1, valOf(x));
            if (isW(x, "vMerge")) {
                QString v;
                for (const auto &a : x.attrs)
                    if (a.first.endsWith(QLatin1String("}val"))) v = a.second;
                s.restarts = v == QLatin1String("restart");
                s.joins = !s.restarts;
            }
        }
    }
    return s;
}

// A table: it has a row, a row has a cell, every cell ends in a paragraph, the cells of a row and the gaps before and after them are as wide as the grid, and
// a cell that joins the one above has one above to join (a chain of them starts with a restart).
inline void tableShape(const Node &tbl, QStringList *out)
{
    int grid = -1, rows = 0;
    for (const Node &k : tbl.kids) {
        if (isW(k, "tblGrid")) grid = int(k.kids.size());
        rows += isW(k, "tr");
    }
    if (!rows) *out << QStringLiteral("a table with no row");
    QVector<QPair<int, int>> above;   // the previous row's cells: where each begins and what it spans, with whether it is part of a chain
    QVector<bool> aboveChain;
    int rowNo = 0;
    for (const Node &tr : tbl.kids) {
        if (!isW(tr, "tr")) continue;
        int before = 0, after = 0, cells = 0, col = 0;
        for (const Node &k : tr.kids)
            if (isW(k, "trPr"))
                for (const Node &x : k.kids) {
                    if (isW(x, "gridBefore")) before = valOf(x);
                    if (isW(x, "gridAfter")) after = valOf(x);
                }
        col = before;
        QVector<QPair<int, int>> here;
        QVector<bool> chain;
        for (const Node &tc : tr.kids) {
            if (!isW(tc, "tc")) continue;
            ++cells;
            const CellShape cs = shapeOfCell(tc);
            QString last = QStringLiteral("nothing");
            for (const Node &c : tc.kids)
                if (!isW(c, "tcPr")) last = shortName(c.name);
            if (last != QLatin1String("w:p")) *out << QStringLiteral("row %1: a cell that ends in %2").arg(rowNo).arg(last);
            if (cs.joins) {
                bool found = false;
                for (int i = 0; i < above.size(); ++i)
                    if (above[i].first == col && above[i].second == cs.span && aboveChain[i]) found = true;
                if (!found) *out << QStringLiteral("row %1: a cell joined to the one above, which is not the start or a part of a chain of joined cells").arg(rowNo);
            }
            here.push_back({col, cs.span});
            chain.push_back(cs.restarts || cs.joins);
            col += cs.span;
        }
        if (!cells) *out << QStringLiteral("row %1: a row with no cell").arg(rowNo);
        if (grid >= 0 && col + after != grid) *out << QStringLiteral("row %1: its cells and gaps cover %2 columns, the table's grid has %3").arg(rowNo).arg(col + after).arg(grid);
        above = here;
        aboveChain = chain;
        ++rowNo;
    }
}

} // namespace detail

// The problems in the shape of the tables of a part: those that are not, as they are, tables of `known` (the same part as it was opened: a table that has not
// been changed is as the file had it).
inline QStringList tableProblems(const Node &root, const QSet<QString> &known = QSet<QString>())
{
    QVector<const Node *> tables;
    detail::collectTables(root, &tables);
    QStringList out;
    for (const Node *t : std::as_const(tables)) {
        if (known.contains(canonical(*t))) continue;
        QStringList mine;
        detail::tableShape(*t, &mine);
        out += mine;
    }
    out.removeDuplicates();
    return out;
}

// Every XML part of `saved` that is not byte for byte one of `original`'s (those the program wrote) must parse strictly, and its tables must have the shape of
// tables. Problems, as text.
inline QStringList strictProblems(const QByteArray &saved, const QByteArray &original = QByteArray())
{
    QStringList out;
    office::Package s, o;
    if (!s.read(saved)) return {QStringLiteral("the saved file is not a package")};
    if (!original.isEmpty()) o.read(original);
    for (const QString &name : s.partNames()) {
        if (!isXmlPart(s, name)) continue;
        const QByteArray bytes = s.part(name);
        if (o.hasPart(name) && o.part(name) == bytes) continue;
        Node root;
        QString err;
        if (!parseTree(bytes, &root, &err)) {
            out << QStringLiteral("%1 is not well-formed: %2").arg(name, err);
            continue;
        }
        if (name != s.mainPart()) continue;
        QSet<QString> known;
        Node was;
        if (o.hasPart(name) && parseTree(o.part(name), &was, &err)) {
            QVector<const Node *> tables;
            detail::collectTables(was, &tables);
            for (const Node *t : std::as_const(tables)) known.insert(canonical(*t));
        }
        for (const QString &p : tableProblems(root, known)) out << QStringLiteral("%1: %2").arg(name, p);
    }
    return out;
}

// The relationship Ids that the main part refers to (r:id, r:embed, r:link...) and the package does not give it: nothing saved may point at a
// part that is not there.
inline QStringList danglingReferences(const QByteArray &zip)
{
    office::Package p;
    if (!p.read(zip)) return {};
    const QString main = p.mainPart();
    const QByteArray xml = p.part(main);
    QStringList out;
    for (qsizetype i = xml.indexOf("r:"); i >= 0; i = xml.indexOf("r:", i + 1)) {
        if (i == 0 || (xml[i - 1] != ' ' && xml[i - 1] != '\n' && xml[i - 1] != '\t' && xml[i - 1] != '\r')) continue;
        qsizetype j = i + 2;
        while (j < xml.size() && ((xml[j] >= 'a' && xml[j] <= 'z') || (xml[j] >= 'A' && xml[j] <= 'Z'))) ++j;
        if (j + 1 >= xml.size() || xml[j] != '=' || (xml[j + 1] != '"' && xml[j + 1] != '\'')) continue;
        const qsizetype from = j + 2, to = xml.indexOf(xml[j + 1], from);
        if (to < 0) break;
        const QString id = QString::fromUtf8(xml.mid(from, to - from));
        const office::Relationship r = p.relationship(main, id);
        if (r.id.isEmpty() || (!r.external && !p.hasPart(r.target))) out << id;
        i = to;
    }
    return out;
}

// ---- the paragraphs

namespace detail {

inline bool inList(const QString &local, std::initializer_list<const char *> list)
{
    for (const char *s : list)
        if (local == QLatin1String(s)) return true;
    return false;
}

inline QString localOf(const QString &name) { return name.mid(name.indexOf(QLatin1Char('}')) + 1); }
inline bool isWordNs(const QString &name) { return name.startsWith(QString::fromLatin1(kW)); }

// The properties the program understands: what is said of them is normalized when it is written (one equal to the style's is left out), so
// only what it does not understand is compared.
const std::initializer_list<const char *> kModeledPPr = {"pStyle", "keepNext", "keepLines", "pageBreakBefore", "widowControl", "numPr", "tabs", "spacing", "ind", "contextualSpacing", "jc", "outlineLvl", "rPr", "sectPr"};
const std::initializer_list<const char *> kModeledRPr = {"rStyle", "rFonts", "b", "i", "caps", "smallCaps", "strike", "dstrike", "vanish", "noProof", "color", "spacing", "position", "sz", "highlight", "u", "vertAlign"};
// The attributes of those that it does.
const QMap<QString, QStringList> kKnownAttrs = {
    {QStringLiteral("pStyle"), {"val"}}, {QStringLiteral("jc"), {"val"}}, {QStringLiteral("ind"), {"left", "right", "hanging", "firstLine", "start", "end"}},
    {QStringLiteral("spacing"), {"before", "after", "line", "lineRule", "val"}}, {QStringLiteral("sz"), {"val"}}, {QStringLiteral("b"), {"val"}},
    {QStringLiteral("i"), {"val"}}, {QStringLiteral("rStyle"), {"val"}}, {QStringLiteral("highlight"), {"val"}}, {QStringLiteral("vertAlign"), {"val"}},
    {QStringLiteral("position"), {"val"}}, {QStringLiteral("color"), {"val", "themeColor", "themeTint", "themeShade"}},
    {QStringLiteral("rFonts"), {"ascii", "hAnsi", "eastAsia", "cs", "asciiTheme", "hAnsiTheme", "eastAsiaTheme", "cstheme", "hint"}},
    {QStringLiteral("numPr"), {}}, {QStringLiteral("tabs"), {}}};

// The things in a paragraph that mark where something begins, ends, or stands (written in order, with what they say).
inline bool isMarker(const QString &local)
{
    return inList(local, {"bookmarkStart", "bookmarkEnd", "commentRangeStart", "commentRangeEnd", "commentReference", "hyperlink", "sdt", "ins", "del", "moveFrom", "moveTo", "smartTag",
                          "customXml", "fldSimple", "fldChar", "instrText", "footnoteReference", "endnoteReference", "permStart", "permEnd", "drawing", "pict", "object", "sym",
                          "moveFromRangeStart", "moveFromRangeEnd", "moveToRangeStart", "moveToRangeEnd", "br", "tab"});
}

// (A soft or a non-breaking hyphen is the character in the text or its own element: the same to a reader.)
inline QString plainHyphens(QString t) { return t.remove(QChar(0x00AD)).remove(QChar(0x2011)); }

struct ParaSig {
    QString attrs, text;
    QStringList markers, pPrOther, rPrOther, unknownAttrs;
    // Each property of a run of the paragraph, as written (every one, the ones the program understands too): a save may leave out what it normalizes, but a run
    // that had none gaining one is a change to a file nobody touched.
    QSet<QString> runProps;
    // What the paragraph hides, as written, runs' and mark's (w:vanish and its kin): a save that lost or gained one changes what the file shows or hides.
    QSet<QString> hiddenProps;
    // The text of what a tracked change deleted (w:delText), in order: a save keeps it as it was.
    QString deletedText;
    bool operator==(const ParaSig &o) const {
        return attrs == o.attrs && plainHyphens(text) == plainHyphens(o.text) && markers == o.markers && pPrOther == o.pPrOther && rPrOther == o.rPrOther && unknownAttrs == o.unknownAttrs &&
               hiddenProps == o.hiddenProps && deletedText == o.deletedText;
    }
};

inline void collectProps(const Node &props, const std::initializer_list<const char *> &modeled, const QString &where, ParaSig *sig, QStringList *other)
{
    for (const Node &k : props.kids) {
        const QString local = localOf(k.name);
        if (!isWordNs(k.name) || !inList(local, modeled)) {
            other->push_back(canonical(k));
            continue;
        }
        // (A paragraph mark's own run properties are the paragraph's, compared with the others.)
        if (local == QLatin1String("rPr")) {
            for (const Node &m : k.kids) {
                const QString ml = localOf(m.name);
                if (isWordNs(m.name) && inList(ml, {"vanish", "webHidden", "specVanish"})) sig->hiddenProps.insert(QStringLiteral("mark:") + canonical(m));
                if (!isWordNs(m.name) || !inList(ml, kModeledRPr)) sig->rPrOther.push_back(canonical(m));
            }
            continue;
        }
        if (local == QLatin1String("sectPr")) continue;
        const QStringList known = kKnownAttrs.value(local, QStringList());
        for (const auto &a : k.attrs) {
            const QString al = localOf(a.first);
            if ((!isWordNs(a.first) || !known.contains(al)) && kKnownAttrs.contains(local)) sig->unknownAttrs.push_back(where + QLatin1Char(':') + local + QLatin1Char('@') + shortName(a.first) + QLatin1Char('=') + a.second);
        }
    }
}

inline void walkInline(const Node &n, ParaSig *sig)
{
    for (const Node &k : n.kids) {
        const QString local = localOf(k.name);
        if (isWordNs(k.name) && local == QLatin1String("rPr")) {
            for (const Node &m : k.kids) {
                const QString ml = localOf(m.name);
                sig->runProps.insert(canonical(m));
                if (isWordNs(m.name) && inList(ml, {"vanish", "webHidden", "specVanish"})) sig->hiddenProps.insert(QStringLiteral("run:") + canonical(m));
                if (!isWordNs(m.name) || !inList(ml, kModeledRPr)) sig->rPrOther.push_back(canonical(m));
                else {
                    const QStringList known = kKnownAttrs.value(ml, QStringList());
                    for (const auto &a : m.attrs) {
                        const QString al = localOf(a.first);
                        if ((!isWordNs(a.first) || !known.contains(al)) && kKnownAttrs.contains(ml)) sig->unknownAttrs.push_back(QStringLiteral("r:") + ml + QLatin1Char('@') + shortName(a.first) + QLatin1Char('=') + a.second);
                    }
                }
            }
            continue;
        }
        if (isWordNs(k.name) && local == QLatin1String("pPr")) continue;
        // (A run with nothing in it, only properties, is no run: it is not written again.)
        if (isWordNs(k.name) && local == QLatin1String("r")) {
            bool content = false;
            for (const Node &c : k.kids) content = content || !(isWordNs(c.name) && localOf(c.name) == QLatin1String("rPr"));
            if (!content) continue;
        }
        if (isWordNs(k.name) && local == QLatin1String("t")) {
            sig->text += k.text;
            continue;
        }
        if (isWordNs(k.name) && local == QLatin1String("delText")) {
            sig->deletedText += k.text;
            continue;
        }
        // (Bookmarks and ranges are compared by their pairs, over the whole document.)
        const bool marker = isWordNs(k.name) && isMarker(local) && !inList(local, {"bookmarkStart", "bookmarkEnd", "commentRangeStart", "commentRangeEnd"});
        if (marker) {
            QString s = QLatin1Char('<') + local;
            // (A bookmark is told by its name: an Id used by two bookmarks is given to the second afresh.)
            const bool bookmark = local == QLatin1String("bookmarkStart") || local == QLatin1String("bookmarkEnd");
            for (const auto &a : k.attrs)
                if (!bookmark || !a.first.endsWith(QLatin1String("}id"))) s += QLatin1Char(' ') + shortName(a.first) + QLatin1Char('=') + a.second;
            if (local == QLatin1String("instrText")) s += QLatin1Char(' ') + k.text.trimmed();
            sig->markers.push_back(s);
        }
        if (isWordNs(k.name) && (local == QLatin1String("drawing") || local == QLatin1String("pict") || local == QLatin1String("object"))) continue;   // (kept whole)
        walkInline(k, sig);
        if (marker && inList(local, {"hyperlink", "sdt", "ins", "del", "moveFrom", "moveTo", "smartTag", "customXml"})) sig->markers.push_back(QStringLiteral(">") + local);
    }
}

inline ParaSig sigOf(const Node &p)
{
    ParaSig s;
    for (const auto &a : p.attrs) s.attrs += shortName(a.first) + QLatin1Char('=') + a.second + QLatin1Char(' ');
    for (const Node &k : p.kids)
        if (isW(k, "pPr")) {
            collectProps(k, kModeledPPr, QStringLiteral("p"), &s, &s.pPrOther);
        }
    walkInline(p, &s);
    std::sort(s.pPrOther.begin(), s.pPrOther.end());
    std::sort(s.rPrOther.begin(), s.rPrOther.end());
    s.rPrOther.erase(std::unique(s.rPrOther.begin(), s.rPrOther.end()), s.rPrOther.end());
    std::sort(s.unknownAttrs.begin(), s.unknownAttrs.end());
    s.unknownAttrs.erase(std::unique(s.unknownAttrs.begin(), s.unknownAttrs.end()), s.unknownAttrs.end());
    return s;
}

// The paragraphs outside tables, in order; the kept tables; the sections' properties; and how many of each kept element there is.
struct DocSig {
    QVector<ParaSig> paragraphs;
    QStringList tables, sections, pairedNames;   // (the names of the bookmarks, and Ids of the comment ranges, that have both ends)
    QMap<QString, int> counts;
    QString text;
};

inline void gather(const Node &n, DocSig *d)
{
    for (const Node &k : n.kids) {
        d->counts[shortName(k.name)] += 1;
        if (isW(k, "tbl")) {
            d->tables.push_back(canonical(k));
            // (what is in a table is counted with the document's)
            std::function<void(const Node &)> count = [&](const Node &t) {
                for (const Node &c : t.kids) {
                    d->counts[shortName(c.name)] += 1;
                    if (isW(c, "t")) d->text += c.text;
                    count(c);
                }
            };
            count(k);
            continue;
        }
        if (isW(k, "p")) {
            d->paragraphs.push_back(sigOf(k));
            d->text += d->paragraphs.last().text;
            // The counts of what is in it.
            std::function<void(const Node &)> count = [&](const Node &t) {
                for (const Node &c : t.kids) {
                    d->counts[shortName(c.name)] += 1;
                    count(c);
                }
            };
            count(k);
            for (const Node &c : k.kids)
                if (isW(c, "pPr"))
                    for (const Node &e : c.kids)
                        if (isW(e, "sectPr")) d->sections.push_back(canonical(e));
            continue;
        }
        if (isW(k, "sectPr")) d->sections.push_back(canonical(k));
        gather(k, d);
    }
}

inline QMap<QString, int> keptCounts(const DocSig &d)
{
    // (A bookmark or a range with no partner is a mistake in a file that the program leaves out: it is not counted in what the file held.)
    static const QStringList kept = {"w:bookmarkStart", "w:bookmarkEnd", "w:commentRangeStart", "w:commentRangeEnd", "w:commentReference", "w:footnoteReference", "w:endnoteReference",
                                     "w:fldChar", "w:instrText", "w:fldSimple", "w:hyperlink", "w:drawing", "w:pict", "w:object", "w:tbl", "w:tr", "w:tc", "w:sdt", "w:ins", "w:del",
                                     "w:delText", "w:moveFrom", "w:moveTo", "w:sectPr", "w:headerReference", "w:footerReference", "w:smartTag", "w:customXml", "w:sym", "w:permStart",
                                     "w:permEnd", "w:ptab", "w:noBreakHyphen", "w:txbxContent"};
    QMap<QString, int> out;
    for (const QString &k : kept)
        if (d.counts.contains(k)) out[k] = d.counts.value(k);
    for (auto it = d.counts.cbegin(); it != d.counts.cend(); ++it)
        if (it.key().contains(QLatin1String("AlternateContent")) || it.key().contains(QLatin1String("math")) || it.key().contains(QLatin1String("drawingml"))) out[it.key()] = it.value();
    return out;
}

} // namespace detail

constexpr qint64 kLargestToCompare = 40 << 20;

// How many of each kept element a package's main part holds (for the checks that editing that only adds never takes any away).
namespace detail {

// The bookmarks and ranges that have both a start and an end (an end closes the last start of its Id that is open), counted for each of the four
// elements; those with no partner are a mistake in a file that the program leaves out, and are not counted in what the file held.
inline void pairedMarkers(const Node &root, DocSig *d)
{
    struct Event { QString kind, id, name; bool start; };
    QVector<Event> events;
    QVector<const Node *> stack{&root};
    // (Document order, without recursing: the tree may be deep.)
    QVector<QPair<const Node *, int>> walk{{&root, 0}};
    while (!walk.isEmpty()) {
        auto &top = walk.last();
        if (top.second == 0) {
            const QString local = localOf(top.first->name);
            if (isWordNs(top.first->name) && (local == QLatin1String("bookmarkStart") || local == QLatin1String("bookmarkEnd") || local == QLatin1String("commentRangeStart") || local == QLatin1String("commentRangeEnd"))) {
                QString id, name;
                for (const auto &a : top.first->attrs) {
                    if (a.first.endsWith(QLatin1String("}id"))) id = a.second;
                    if (a.first.endsWith(QLatin1String("}name"))) name = a.second;
                }
                events.push_back({local.startsWith(QLatin1String("bookmark")) ? QStringLiteral("bookmark") : QStringLiteral("comment"), id, name, local.endsWith(QLatin1String("Start"))});
            }
        }
        if (top.second < top.first->kids.size()) {
            const Node *child = &top.first->kids[top.second++];
            walk.push_back({child, 0});
        } else {
            walk.removeLast();
        }
    }
    QHash<QString, QStringList> open;   // the names of the starts that have no end yet, by kind and Id
    int pairs[2] = {0, 0};
    d->pairedNames.clear();
    for (const Event &e : std::as_const(events)) {
        const QString key = e.kind + QLatin1Char('/') + e.id;
        if (e.start) {
            open[key].push_back(e.kind == QLatin1String("bookmark") ? e.name : QStringLiteral("c:") + e.id);
        } else if (!open.value(key).isEmpty()) {
            d->pairedNames << open[key].takeLast();
            ++pairs[e.kind == QLatin1String("bookmark") ? 0 : 1];
        }
    }
    std::sort(d->pairedNames.begin(), d->pairedNames.end());
    d->counts[QStringLiteral("w:bookmarkStart")] = d->counts[QStringLiteral("w:bookmarkEnd")] = pairs[0];
    d->counts[QStringLiteral("w:commentRangeStart")] = d->counts[QStringLiteral("w:commentRangeEnd")] = pairs[1];
}

} // namespace detail

inline QMap<QString, int> keptCountsOf(const QByteArray &zip)
{
    office::Package p;
    if (!p.read(zip)) return {};
    Node root;
    if (!parseTree(p.part(p.mainPart()), &root, nullptr)) return {};
    detail::DocSig d;
    detail::gather(root, &d);
    detail::pairedMarkers(root, &d);
    return detail::keptCounts(d);
}

// What the saved package lost or changed against the one that was opened. Empty when it is a faithful copy.
inline QStringList lossProblems(const QByteArray &original, const QByteArray &saved)
{
    QStringList out;
    office::Package a, b;
    if (!a.read(original) || !b.read(saved)) return {QStringLiteral("a package does not read")};
    QSet<QString> pa, pb;
    for (const QString &n : a.partNames()) pa.insert(n.toLower());
    for (const QString &n : b.partNames()) pb.insert(n.toLower());
    auto isMacro = [&](const QString &n) {
        const QString type = a.contentType(n);
        return type.startsWith(QLatin1String("application/vnd.ms-office.vba")) || type.contains(QLatin1String("macroEnabled")) || type.contains(QLatin1String("vbaData"));
    };
    QSet<QString> macros;
    for (const QString &n : a.partNames())
        if (isMacro(n)) macros.insert(n.toLower());
    for (const QString &n : pa)
        if (!pb.contains(n) && !macros.contains(n)) out << QStringLiteral("part lost: ") + n;
    for (const QString &n : pb)
        if (!pa.contains(n)) out << QStringLiteral("part added: ") + n;
    for (const QString &n : a.partNames()) {
        if (!b.hasPart(n)) continue;
        // (A macro-enabled file or a template is saved as the plain kind: said to the person when it is saved.)
        const bool main = n.compare(a.mainPart(), Qt::CaseInsensitive) == 0 && (a.contentType(n).contains(QLatin1String("macroEnabled")) || a.contentType(n).contains(QLatin1String("template")));
        if (a.contentType(n) != b.contentType(n) && !main) out << QStringLiteral("content type of %1: %2 -> %3").arg(n, a.contentType(n), b.contentType(n));
    }
    // The relationships (by source, each as id, type, target, and whether external).
    auto relsOf = [](const office::Package &p) {
        QMap<QString, QSet<QString>> m;
        QStringList sources = p.partNames();
        sources << QStringLiteral("/");
        for (const QString &s : sources)
            for (const office::Relationship &r : p.relationships(s)) m[s.toLower()].insert(r.id + QLatin1Char('|') + r.type + QLatin1Char('|') + r.target + QLatin1Char('|') + (r.external ? QStringLiteral("ext") : QStringLiteral("int")));
        return m;
    };
    const QMap<QString, QSet<QString>> ra = relsOf(a), rb = relsOf(b);
    for (auto it = ra.cbegin(); it != ra.cend(); ++it) {
        if (!pb.contains(it.key()) && it.key() != QLatin1String("/")) continue;   // (of a part that was lost, already said)
        QSet<QString> lost = QSet<QString>(it.value()).subtract(rb.value(it.key()));
        for (auto l = lost.begin(); l != lost.end();)   // (a macro's link goes with the macro)
            if (l->contains(QLatin1String("vba")) || l->contains(QLatin1String("keyMap")) || macros.contains(l->section(QLatin1Char('|'), 2, 2).toLower())) l = lost.erase(l);
            else ++l;
        if (!lost.isEmpty()) out << QStringLiteral("relationships of %1 lost: %2").arg(it.key(), QStringList(lost.cbegin(), lost.cend()).join(QStringLiteral("; ")).left(200));
    }
    for (auto it = rb.cbegin(); it != rb.cend(); ++it) {
        const QSet<QString> added = QSet<QString>(it.value()).subtract(ra.value(it.key()));
        if (!added.isEmpty()) out << QStringLiteral("relationships of %1 added: %2").arg(it.key(), QStringList(added.cbegin(), added.cend()).join(QStringLiteral("; ")).left(200));
    }
    // The parts kept as they were.
    const QString mainPart = a.mainPart();
    for (const QString &n : a.partNames()) {
        if (!b.hasPart(n) || n.compare(mainPart, Qt::CaseInsensitive) == 0) continue;
        if (a.part(n) == b.part(n)) continue;
        // (the styles and the numbering are written again when they change; the sheet they make is compared elsewhere)
        if (n.endsWith(QLatin1String("styles.xml")) || n.endsWith(QLatin1String("numbering.xml")) || n.endsWith(QLatin1String("settings.xml")) || n.contains(QLatin1String("docProps/"))) continue;
        out << QStringLiteral("part changed: ") + n;
    }
    if (mainPart.isEmpty() || !b.hasPart(mainPart)) return out;
    const QByteArray ma = a.part(mainPart), mb = b.part(mainPart);
    detail::DocSig da, db;
    Node ra1, rb1;
    QString err;
    if (!parseTree(mb, &rb1, &err)) {
        out << QStringLiteral("the main part is not well-formed: ") + err;
        return out;
    }
    if (ma.size() > kLargestToCompare) return out;
    if (!parseTree(ma, &ra1, &err)) return out;   // (a main part that was not well-formed to begin with: nothing to compare)
    detail::gather(ra1, &da);
    detail::gather(rb1, &db);
    detail::pairedMarkers(ra1, &da);
    detail::pairedMarkers(rb1, &db);
    if (detail::plainHyphens(da.text) != detail::plainHyphens(db.text)) out << QStringLiteral("text differs");
    const QMap<QString, int> ca = detail::keptCounts(da), cb = detail::keptCounts(db);
    QStringList diffs;
    for (auto it = ca.cbegin(); it != ca.cend(); ++it)
        if (cb.value(it.key()) != it.value()) diffs << QStringLiteral("%1 %2->%3").arg(it.key()).arg(it.value()).arg(cb.value(it.key()));
    for (auto it = cb.cbegin(); it != cb.cend(); ++it)
        if (!ca.contains(it.key())) diffs << QStringLiteral("%1 0->%2").arg(it.key()).arg(it.value());
    diffs.erase(std::remove_if(diffs.begin(), diffs.end(), [&](const QString &d) { return d == QStringLiteral("w:sectPr 0->1"); }), diffs.end());
    if (!diffs.isEmpty()) out << QStringLiteral("element counts: ") + diffs.join(QStringLiteral("; ")).left(300);
    if (da.pairedNames != db.pairedNames) out << QStringLiteral("bookmarks or ranges differ (%1 -> %2 pairs)").arg(da.pairedNames.size()).arg(db.pairedNames.size());
    if (da.tables != db.tables) out << QStringLiteral("a kept table differs (%1 -> %2 tables)").arg(da.tables.size()).arg(db.tables.size());
    // (A file that states no section properties at all is written with the usual ones.)
    if (da.sections != db.sections && !(da.sections.isEmpty() && db.sections.size() == 1)) out << QStringLiteral("section properties differ (%1 -> %2 sections)").arg(da.sections.size()).arg(db.sections.size());
    if (da.paragraphs.size() != db.paragraphs.size()) {
        out << QStringLiteral("paragraph count %1 -> %2").arg(da.paragraphs.size()).arg(db.paragraphs.size());
    } else {
        int shown = 0;
        for (int i = 0; i < da.paragraphs.size() && shown < 3; ++i) {
            const detail::ParaSig &x = da.paragraphs[i], &y = db.paragraphs[i];
            QSet<QString> gained = y.runProps;
            gained.subtract(x.runProps);
            if (!gained.isEmpty() && shown < 3) {
                ++shown;
                out << QStringLiteral("paragraph %1 gains run properties: %2").arg(i).arg(QStringList(gained.cbegin(), gained.cend()).join(QLatin1Char(',')).left(160));
            }
            if (x == y) continue;
            ++shown;
            QString what;
            if (x.attrs != y.attrs) what += QStringLiteral(" attrs[%1|%2]").arg(x.attrs, y.attrs);
            if (x.markers != y.markers) what += QStringLiteral(" markers[%1|%2]").arg(x.markers.join(QLatin1Char(',')).left(160), y.markers.join(QLatin1Char(',')).left(160));
            if (x.pPrOther != y.pPrOther) what += QStringLiteral(" pPr[%1|%2]").arg(x.pPrOther.join(QLatin1Char(',')).left(120), y.pPrOther.join(QLatin1Char(',')).left(120));
            if (x.rPrOther != y.rPrOther) what += QStringLiteral(" rPr[%1|%2]").arg(x.rPrOther.join(QLatin1Char(',')).left(120), y.rPrOther.join(QLatin1Char(',')).left(120));
            if (x.unknownAttrs != y.unknownAttrs) what += QStringLiteral(" attributes[%1|%2]").arg(x.unknownAttrs.join(QLatin1Char(',')).left(120), y.unknownAttrs.join(QLatin1Char(',')).left(120));
            if (detail::plainHyphens(x.text) != detail::plainHyphens(y.text)) what += QStringLiteral(" text");
            if (x.deletedText != y.deletedText) what += QStringLiteral(" deleted text[%1|%2]").arg(x.deletedText.left(60), y.deletedText.left(60));
            if (x.hiddenProps != y.hiddenProps) what += QStringLiteral(" hidden[%1|%2]").arg(QStringList(x.hiddenProps.cbegin(), x.hiddenProps.cend()).join(QLatin1Char(',')).left(120), QStringList(y.hiddenProps.cbegin(), y.hiddenProps.cend()).join(QLatin1Char(',')).left(120));
            out << QStringLiteral("paragraph %1 differs:").arg(i) + what;
        }
    }
    return out;
}

// ---- the story

inline QTextFormat resolvedFormat(const jp::write::WriteDoc &doc, QTextFormat f);

inline QTextFormat resolvedFormat(const jp::write::WriteDoc &doc, QTextFormat f)
{
    const QMap<int, QVariant> props = f.properties();
    for (auto it = props.cbegin(); it != props.cend(); ++it) {
        if (it.key() == QTextFormat::ObjectIndex || it.key() == jp::write::prop::LastSection) f.clearProperty(it.key());   // (the last section is compared with the others)
        else if (it.key() == jp::write::prop::Atom) {
            // (A bookmark is told by its name: an Id used by two bookmarks is given to the second afresh.)
            jp::office::wml::Inline a = jp::write::unpackAtom(jp::write::storedBytesOf(it.value().toInt()));
            if (a.tag == QLatin1String("bookmarkStart") || a.tag == QLatin1String("bookmarkEnd")) {
                a.key.clear();
                const qsizetype at = a.raw.indexOf(":id=\"");
                if (at >= 0) a.raw = a.raw.left(at - 1) + a.raw.mid(a.raw.indexOf('"', at + 5) + 1);
            }
            f.setProperty(it.key(), jp::write::packAtom(a));
        } else if (it.key() == jp::write::prop::MarkFormat) {
            // (The first paragraph's mark, kept as a record of a character format: compared as that format.)
            f.setProperty(it.key(), QVariant::fromValue(resolvedFormat(doc, jp::write::unpackFormat(jp::write::storedBytesOf(it.value().toInt())))));
        } else if (it.key() == jp::write::prop::FontsOther) {
            f.clearProperty(it.key());   // (the fonts of other scripts a run says that are the style's are not said again: the same fonts)
        } else if (it.key() == jp::write::prop::RawRPr || it.key() == jp::write::prop::RawPPr) {
            // (The children a file keeps are written in the schema's order, whatever order the file had them in.)
            QVector<jp::office::wml::RawChild> kept = jp::write::unpackRaw(jp::write::storedBytesOf(it.value().toInt()));
            std::stable_sort(kept.begin(), kept.end(), [](const auto &x, const auto &y) { return x.rank < y.rank; });
            f.setProperty(it.key(), jp::write::packRaw(kept));
        } else if (jp::write::isStoredProperty(it.key())) {
            f.setProperty(it.key(), jp::write::resolveStored(doc, it.value(), it.key()));
        }
    }
    return f;
}

// How two stories differ: blocks, their formats, the formatted text in them, and the sections with their properties. At most a few lines.
inline QStringList storyProblems(jp::write::WriteDoc &a, jp::write::WriteDoc &b)
{
    QStringList out;
    QTextDocument *da = a.story(), *db = b.story();
    if (da->blockCount() != db->blockCount()) out << QStringLiteral("blocks %1 -> %2").arg(da->blockCount()).arg(db->blockCount());
    QTextBlock x = da->begin(), y = db->begin();
    int n = 0;
    for (; x.isValid() && y.isValid() && out.size() < 4; x = x.next(), y = y.next(), ++n) {
        // (The pieces kept as XML are compared in the file's own terms: a bookmark with no end is left out, and that is no change to the text.)
        QString tx = x.text(), ty = y.text();
        tx.remove(QChar::ObjectReplacementCharacter);
        ty.remove(QChar::ObjectReplacementCharacter);
        if (tx != ty) {
            out << QStringLiteral("block %1 text differs").arg(n);
            continue;
        }
        const QTextFormat fx = resolvedFormat(a, x.blockFormat()), fy = resolvedFormat(b, y.blockFormat());
        if (!(fx == fy)) {
            QStringList keys;
            const QMap<int, QVariant> px = fx.properties(), py = fy.properties();
            QSet<int> all;
            for (auto it = px.cbegin(); it != px.cend(); ++it) all.insert(it.key());
            for (auto it = py.cbegin(); it != py.cend(); ++it) all.insert(it.key());
            for (int k : std::as_const(all))
                if (px.value(k) != py.value(k)) {
                    QString a, b;
                    QDebug(&a) << px.value(k);
                    QDebug(&b) << py.value(k);
                    keys << QStringLiteral("%1 (%2 | %3)").arg(k).arg(a.left(90), b.left(90));
                }
            out << QStringLiteral("block %1 format differs in %2").arg(n).arg(keys.join(QLatin1Char(',')));
        }
        QVector<std::pair<QString, QTextFormat>> fa, fb;
        auto frags = [](jp::write::WriteDoc &d, const QTextBlock &blk, QVector<std::pair<QString, QTextFormat>> *v) {
            for (QTextBlock::iterator it = blk.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                if (f.charFormat().hasProperty(jp::write::prop::Atom)) continue;
                const QTextFormat rf = resolvedFormat(d, f.charFormat());
                if (!v->isEmpty() && v->last().second == rf) v->last().first += f.text();
                else v->push_back({f.text(), rf});
            }
        };
        frags(a, x, &fa);
        frags(b, y, &fb);
        if (fa != fb) {
            QString what;
            for (int i = 0; i < std::min(fa.size(), fb.size()) && what.isEmpty(); ++i)
                if (fa[i] != fb[i]) {
                    const QMap<int, QVariant> px = fa[i].second.properties(), py = fb[i].second.properties();
                    QStringList keys;
                    QSet<int> all;
                    for (auto it = px.cbegin(); it != px.cend(); ++it) all.insert(it.key());
                    for (auto it = py.cbegin(); it != py.cend(); ++it) all.insert(it.key());
                    for (int k : std::as_const(all))
                        if (px.value(k) != py.value(k)) {
                            QString va, vb;
                            QDebug(&va) << px.value(k);
                            QDebug(&vb) << py.value(k);
                            keys << QStringLiteral("%1 (%2 | %3)").arg(k).arg(va.left(60), vb.left(60));
                        }
                    what = QStringLiteral(" in \"%1\"%2: %3").arg(fa[i].first.left(20), fa[i].first != fb[i].first ? QStringLiteral(" (text %1)").arg(fb[i].first.left(20)) : QString(), keys.join(QLatin1Char(',')));
                }
            out << QStringLiteral("block %1 character formats differ (%2 and %3 pieces)%4").arg(n).arg(fa.size()).arg(fb.size()).arg(what);
        }
    }
    if (a.sectionCount() != b.sectionCount()) out << QStringLiteral("sections %1 -> %2").arg(a.sectionCount()).arg(b.sectionCount());
    else
        for (int i = 0; i < a.sectionCount() && out.size() < 6; ++i)
        {
            // (Whether the file said the paper, the margins, and the columns, or the usual ones were meant, is not what is compared.)
            auto stated = [](jp::office::wml::SectionProps p) {
                p.hasPgSz = p.hasPgMar = p.hasCols = true;
                return jp::write::packSection(p);
            };
            if (stated(a.sectionProps(i)) != stated(b.sectionProps(i))) out << QStringLiteral("section %1 properties differ").arg(i);
        }
    return out;
}

} // namespace rt
