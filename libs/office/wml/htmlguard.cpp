#include "wml/htmlguard.h"

#include <QCoreApplication>
#include <QMimeData>

#include <algorithm>
#include <cstring>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jp::office::wml {

namespace {

// Preformatted text is written as a block that keeps its spaces and a block for each line (the parser takes a line of a preformatted part, with its own style, in 32 microseconds,
// and a block of a plain line in 11: see the measurements at the end of this file).
constexpr const char *kPreWrapper = "<div style=\"white-space:pre;font-family:monospace\">";
constexpr const char *kPreFirstLine = "<div style=\"margin-top:12px\">";

// The parts of a page that are kept. Every other part is left out and its words stay; the content of a script, a style sheet, a title, and a comment is left out too.
enum Cls : quint8 { CB, CI, CU, CS, CSub, CSup, CSpan, CFont, CA, CCode, CBig, CSmall, CP, CHead, CDiv, CQuote, CCenter, CPre, CUl, COl, CDl, CLi, CDt, CDd, CTable, CTr, CCell, ClsCount };

struct Src {
    Cls cls;
    const char *out;    // the name it is written under
};

const std::unordered_map<std::string_view, Src> &sources()
{
    static const std::unordered_map<std::string_view, Src> map = {
        {"b", {CB, "b"}}, {"strong", {CB, "b"}}, {"i", {CI, "i"}}, {"em", {CI, "i"}}, {"cite", {CI, "i"}}, {"dfn", {CI, "i"}}, {"var", {CI, "i"}},
        {"u", {CU, "u"}}, {"ins", {CU, "u"}}, {"s", {CS, "s"}}, {"strike", {CS, "s"}}, {"del", {CS, "s"}}, {"sub", {CSub, "sub"}}, {"sup", {CSup, "sup"}},
        {"span", {CSpan, "span"}}, {"font", {CFont, "font"}}, {"a", {CA, "a"}}, {"code", {CCode, "code"}}, {"tt", {CCode, "code"}}, {"kbd", {CCode, "code"}},
        {"samp", {CCode, "code"}}, {"big", {CBig, "big"}}, {"small", {CSmall, "small"}},
        {"p", {CP, "p"}}, {"h1", {CHead, "h1"}}, {"h2", {CHead, "h2"}}, {"h3", {CHead, "h3"}}, {"h4", {CHead, "h4"}}, {"h5", {CHead, "h5"}}, {"h6", {CHead, "h6"}},
        {"div", {CDiv, "div"}}, {"section", {CDiv, "div"}}, {"article", {CDiv, "div"}}, {"header", {CDiv, "div"}}, {"footer", {CDiv, "div"}}, {"main", {CDiv, "div"}},
        {"nav", {CDiv, "div"}}, {"aside", {CDiv, "div"}}, {"form", {CDiv, "div"}}, {"fieldset", {CDiv, "div"}}, {"figure", {CDiv, "div"}}, {"figcaption", {CDiv, "div"}},
        {"details", {CDiv, "div"}}, {"summary", {CDiv, "div"}}, {"address", {CDiv, "div"}}, {"legend", {CDiv, "div"}}, {"caption", {CDiv, "div"}}, {"hgroup", {CDiv, "div"}},
        {"blockquote", {CQuote, "blockquote"}}, {"center", {CCenter, "center"}}, {"pre", {CPre, "div"}}, {"listing", {CPre, "div"}},
        {"ul", {CUl, "ul"}}, {"menu", {CUl, "ul"}}, {"dir", {CUl, "ul"}}, {"ol", {COl, "ol"}}, {"dl", {CDl, "dl"}}, {"li", {CLi, "li"}}, {"dt", {CDt, "dt"}}, {"dd", {CDd, "dd"}},
        {"table", {CTable, "table"}}, {"tr", {CTr, "tr"}}, {"td", {CCell, "td"}}, {"th", {CCell, "th"}},
    };
    return map;
}

// Parts whose content is left out whole (up to their end tag, or the end of the page).
bool isRawText(std::string_view name)
{
    return name == "script" || name == "style" || name == "title" || name == "noscript" || name == "iframe" || name == "template" || name == "noembed";
}

// The attributes that are kept (written under these names).
enum At : quint8 { AHref, AName, ASrc, AWidth, AHeight, AAlt, AColspan, ARowspan, ABgcolor, ABorder, ACellpadding, ACellspacing, AAlign, AValign, AColor, ASize, AFace, AStart, AType, AValue, ADir, AStyle, ANowrap, AContent, AtCount };

const char *const kAtName[AtCount] = {"href", "name", "src", "width", "height", "alt", "colspan", "rowspan", "bgcolor", "border", "cellpadding", "cellspacing", "align", "valign", "color", "size", "face", "start", "type", "value", "dir", "style", "nowrap", "content"};

const std::unordered_map<std::string_view, At> &attributes()
{
    static const std::unordered_map<std::string_view, At> map = [] {
        std::unordered_map<std::string_view, At> m;
        for (int a = 0; a < AtCount; ++a) m[kAtName[a]] = At(a);
        return m;
    }();
    return map;
}

constexpr quint32 bit(At a) { return quint32(1) << a; }

// How an address begins: with no scheme at all (a relative address or an in-page #name: `relative`), or with the scheme named (lower case; empty when it cannot be told, which is as bad as a scheme that is not
// kept: a character that a part of a page can hide a colon behind, such as an entity, a backslash, or a control character, before the first slash).
struct AddressStart {
    bool relative = false;
    QString scheme;
};
AddressStart addressStart(QStringView raw)
{
    const QStringView v = raw.trimmed();
    AddressStart a;
    for (qsizetype k = 0; k < v.size(); ++k) {
        const ushort u = v[k].unicode();
        if (u == '/' || u == '?' || u == '#') break;
        if (u == ':') {
            a.scheme = v.left(k).toString().toLower();
            return a;
        }
        const bool ok = (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || u == '+' || u == '.' || u == '-' || u == '_' || u == '%' || u == '~' || u == ',' || u == '(' || u == ')' || u == '@' || u == '=' || u == '$' || u == '!' || u == '\'' || u == '*' || u >= 0x80;
        if (!ok) return a;   // (an entity, a backslash, a space, a control character: not told)
    }
    a.relative = true;
    return a;
}
// A link address kept as written when it goes to the web, to a mail address, to a telephone, to a file server, to a part of this page, or to a page beside this one; any other (script:, file:, data:, and so on) is left out.
bool linkAddressKept(QStringView raw)
{
    const AddressStart a = addressStart(raw);
    if (a.relative) return true;
    return a.scheme == QLatin1String("http") || a.scheme == QLatin1String("https") || a.scheme == QLatin1String("mailto") || a.scheme == QLatin1String("tel") || a.scheme == QLatin1String("ftp");
}
// A picture address kept: a picture the page holds (data:image, within the size the readers take), or a file beside the page (a relative address, or a file: address) that does not climb out of the page's folder. The
// readers read nothing else (never the network), so no other address is kept.
bool pictureAddressKept(QStringView raw, qint64 mostBytes)
{
    const QStringView v = raw.trimmed();
    const AddressStart a = addressStart(v);
    if (a.scheme == QLatin1String("data")) return v.startsWith(QLatin1String("data:image/"), Qt::CaseInsensitive) && v.size() <= mostBytes * 4 / 3 + 300;
    if (!a.relative && a.scheme != QLatin1String("file")) return false;
    if (a.relative && v.startsWith(QLatin1Char('/'))) return false;   // (a path from the root, or an address on another host: not beside the page)
    // (no part of the path is ".." or goes in through a percent sign or a backslash)
    if (v.contains(QLatin1Char('\\')) || v.contains(QLatin1String("%2e"), Qt::CaseInsensitive) || v.contains(QLatin1String("%5c"), Qt::CaseInsensitive) || v.contains(QLatin1String("%2f"), Qt::CaseInsensitive)) return false;
    qsizetype pathEnd = 0;
    while (pathEnd < v.size() && v[pathEnd] != QLatin1Char('?') && v[pathEnd] != QLatin1Char('#')) ++pathEnd;
    const QStringView path = v.left(pathEnd);
    for (const QStringView part : path.split(QLatin1Char('/')))
        if (part == QLatin1String("..")) return false;
    return true;
}

// The attributes each kind of part keeps.
quint32 allowedAttributes(Cls c, bool picture)
{
    if (picture) return bit(ASrc) | bit(AWidth) | bit(AHeight) | bit(AAlt) | bit(AAlign) | bit(AStyle);
    constexpr quint32 common = bit(AStyle) | bit(ADir);
    switch (c) {
    case CA: return common | bit(AHref) | bit(AName);
    case CFont: return common | bit(AColor) | bit(ASize) | bit(AFace);
    case CTable: return common | bit(ABorder) | bit(ACellpadding) | bit(ACellspacing) | bit(ABgcolor) | bit(AWidth) | bit(AHeight) | bit(AAlign);
    case CTr: return common | bit(ABgcolor) | bit(AAlign) | bit(AValign);
    case CCell: return common | bit(AColspan) | bit(ARowspan) | bit(ABgcolor) | bit(AWidth) | bit(AHeight) | bit(AAlign) | bit(AValign) | bit(ANowrap);
    case COl: return common | bit(AStart) | bit(AType);
    case CUl: return common | bit(AType);
    case CLi: return common | bit(AValue) | bit(AType) | bit(AAlign);
    case CP: case CHead: case CDiv: case CQuote: case CCenter: case CDt: case CDd: case CDl: return common | bit(AAlign);
    default: return common;
    }
}

// The style properties that are kept. Everything else (and every value that names a file or a script) is left out.
const std::unordered_set<std::string_view> &styleProperties()
{
    static const std::unordered_set<std::string_view> set = {
        "color", "background", "background-color", "font", "font-family", "font-size", "font-style", "font-weight", "font-variant", "text-decoration", "text-align", "text-indent", "text-transform",
        "vertical-align", "line-height", "letter-spacing", "word-spacing", "margin", "margin-top", "margin-bottom", "margin-left", "margin-right", "padding", "padding-top", "padding-bottom",
        "padding-left", "padding-right", "border", "border-top", "border-bottom", "border-left", "border-right", "border-width", "border-style", "border-color", "border-collapse",
        "border-top-width", "border-bottom-width", "border-left-width", "border-right-width", "border-top-style", "border-bottom-style", "border-left-style", "border-right-style",
        "border-top-color", "border-bottom-color", "border-left-color", "border-right-color", "width", "height", "white-space", "list-style-type", "direction", "float", "page-break-before", "page-break-after",
    };
    return set;
}

bool isBlankChar(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }
int lowerAscii(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }
bool isLetter(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

struct Frame {
    const char *name;
    Cls cls;
    bool emitted;
    bool block;     // a paragraph, a list, a table, or the like (not a part inside a line)
    bool header = false;    // a header cell
    int items = 0;  // a list: its items so far (in the list written so far)
    int base = 0;   // an ordered list: its items before this one (a list written as several)
};

struct Site {       // a cell that spans: each span is written as three digits, set when the table ends (when the rows and cells the table has are known)
    qsizetype colAt = -1, rowAt = -1;
    int cols = 1, rows = 1;
    int row = 0;
};

struct TableLevel {
    int pos;
    bool emitted;
    size_t firstSite;
    int maxCells = 0;
    bool bordered = false;      // the table has a border (a cell that says "no border" then means it)
    std::vector<int> rowCells;
};

class Rewriter {
public:
    Rewriter(QStringView in, const HtmlLimits &lim) : s(in), n(in.size()), lim(lim) { out.reserve(std::min<qsizetype>(n + n / 4 + 64, qsizetype(lim.bytes) + 64)); }

    GuardedHtml run()
    {
        GuardedHtml g;
        if (qint64(n) > lim.bytes) {
            g.refusal = HtmlRefusal::TooLarge;
            return g;
        }
        while (i < n && why == HtmlRefusal::None) step();
        if (why == HtmlRefusal::None) closeTo(0);
        if (why == HtmlRefusal::None && bodySeen) out += QLatin1String("</body>");
        if (why == HtmlRefusal::None && htmlSeen) out += QLatin1String("</html>");
        if (why == HtmlRefusal::None && counts.text > lim.text) why = HtmlRefusal::TooLarge;
        g.refusal = why;
        g.counts = counts;
        if (why == HtmlRefusal::None) g.html = std::move(out);
        return g;
    }

private:
    QStringView s;
    qsizetype n;
    qsizetype i = 0;
    HtmlLimits lim;
    QString out;
    HtmlCounts counts;
    HtmlRefusal why = HtmlRefusal::None;
    std::vector<Frame> stack;
    std::vector<int> pos[ClsCount];
    std::vector<TableLevel> tables;
    std::vector<Site> sites;
    int emitDepth = 0, openTables = 0, preOpen = 0, headers = 0;
    bool atEdge = true, pendingSpace = false, preFresh = false, htmlSeen = false, bodySeen = false, keepSpaces = false, startMark = false, endMark = false, wrapBody = false;
    // the attributes of the tag being read
    qsizetype vb[AtCount], ve[AtCount];
    quint32 seen = 0;

    // ---------------------------------------------------------------- writing
    void fail(HtmlRefusal r)
    {
        if (why == HtmlRefusal::None) why = r;
    }
    void part()
    {
        if (++counts.parts > lim.parts) fail(HtmlRefusal::TooManyParts);
    }
    void flushSpace()
    {
        if (pendingSpace && !atEdge) {
            out += QLatin1Char(' ');
            ++counts.text;
        }
        pendingSpace = false;
    }
    void lineBreak()
    {
        out += QLatin1String("<br>");
        part();
        atEdge = true;
        pendingSpace = false;
    }
    // An entity the page spells is kept (&name; &#123; &#x1F;, a name of at most 10 characters); any other ampersand is written as one.
    void putEntityOrAmp(QStringView v, qsizetype &k)
    {
        const qsizetype end = v.size();
        qsizetype j = k + 1;
        bool ok = false;
        auto hexDigit = [](QChar c) { const int u = lowerAscii(c.unicode()); return (u >= '0' && u <= '9') || (u >= 'a' && u <= 'f'); };
        if (j < end && v[j] == QLatin1Char('#')) {
            ++j;
            const bool hex = j < end && (v[j] == QLatin1Char('x') || v[j] == QLatin1Char('X'));
            if (hex) ++j;
            const qsizetype d0 = j;
            while (j < end && j - d0 < 8 && (hex ? hexDigit(v[j]) : (v[j].unicode() >= '0' && v[j].unicode() <= '9'))) ++j;
            ok = j > d0 && j < end && v[j] == QLatin1Char(';');
        } else {
            const qsizetype d0 = j;
            while (j < end && j - d0 < 10 && v[j].unicode() < 128 && v[j].isLetterOrNumber()) ++j;
            ok = j > d0 && j < end && v[j] == QLatin1Char(';');
        }
        if (ok) {
            out.append(v.mid(k, j + 1 - k));
            k = j + 1;
        } else {
            out += QLatin1String("&amp;");
            ++k;
        }
    }
    // A value of an attribute with its markup characters escaped.
    void putValue(QStringView v)
    {
        const qsizetype end = v.size();
        for (qsizetype k = 0; k < end;) {
            const QChar c = v[k];
            const ushort u = c.unicode();
            if (u == '&') {
                putEntityOrAmp(v, k);
                continue;
            }
            ++k;
            if (u == '"') out += QLatin1String("&quot;");
            else if (u == '<') out += QLatin1String("&lt;");
            else if (u == '>') out += QLatin1String("&gt;");
            else if (u < 0x20 && u != '\t') continue;
            else out += c;
        }
    }

    // ---------------------------------------------------------------- the stack
    int barrier() const
    {
        int b = -1;
        if (!pos[CCell].empty()) b = pos[CCell].back();
        if (!pos[CTable].empty()) b = std::max(b, pos[CTable].back());
        return b;
    }
    void popOne()
    {
        const Frame f = stack.back();
        ++counts.work;
        stack.pop_back();
        pos[f.cls].pop_back();
        if (f.emitted) {
            out += QLatin1String("</");
            out += QLatin1String(f.name);
            out += QLatin1Char('>');
            --emitDepth;
            if (f.cls == CPre) {
                out += QLatin1String("</div>");
                --emitDepth;
            }
        }
        if (f.header) --headers;
        if (f.block) {
            atEdge = true;
            pendingSpace = false;
        }
        if (f.cls == CPre) --preOpen;
        if (f.cls == CTable) endTable();
    }
    void closeTo(qsizetype keep)
    {
        while (qsizetype(stack.size()) > keep && why == HtmlRefusal::None) popOne();
    }
    void closeKind(Cls c)
    {
        if (!pos[c].empty() && pos[c].back() > barrier()) closeTo(pos[c].back());
    }
    void closeParagraph() { closeKind(CP); }
    void push(const Src &src, const char *name, bool emitted, bool block)
    {
        Frame f{name, src.cls, emitted, block};
        pos[src.cls].push_back(int(stack.size()));
        stack.push_back(f);
        counts.mostOpen = std::max(counts.mostOpen, int(stack.size()));
        if (emitted) {
            ++emitDepth;
            counts.deepest = std::max(counts.deepest, emitDepth);
        }
        if (src.cls == CPre) ++preOpen;
    }

    // ---------------------------------------------------------------- tables
    void endTable()
    {
        TableLevel t = std::move(tables.back());
        tables.pop_back();
        if (!t.emitted) {
            sites.resize(t.firstSite);
            return;
        }
        --openTables;
        const int rows = int(t.rowCells.size());
        // each span is no more than the table has (and than the page allows)
        bool anyRows = false;
        for (size_t k = t.firstSite; k < sites.size(); ++k) {
            Site &st = sites[k];
            st.cols = std::clamp(st.cols, 1, std::max(1, std::min(t.maxCells, lim.span)));
            st.rows = std::clamp(st.rows, 1, std::max(1, std::min(rows - st.row, lim.span)));
            anyRows = anyRows || st.rows > 1;
        }
        // the grid the parser makes: the cells of each row, those the cells spanning columns cover, and those the cells of rows above that span rows cover
        auto widestOf = [&](bool withRows) {
            std::vector<qint64> width(size_t(rows) + 1, 0), carry(size_t(rows) + 2, 0);
            for (int r = 0; r < rows; ++r) width[size_t(r)] = t.rowCells[size_t(r)];
            for (size_t k = t.firstSite; k < sites.size(); ++k) {
                const Site &st = sites[k];
                width[size_t(st.row)] += st.cols - 1;
                if (withRows && st.rows > 1) {
                    carry[size_t(st.row) + 1] += st.cols;
                    carry[size_t(st.row) + size_t(st.rows)] -= st.cols;
                }
            }
            qint64 widest = 0, run = 0;
            for (int r = 0; r < rows; ++r) {
                run += carry[size_t(r)];
                widest = std::max(widest, width[size_t(r)] + run);
            }
            return widest;
        };
        const qint64 widest = widestOf(anyRows);
        // the spans of a page are held to a budget. A span is never dropped or cut to fit (the cells after it would stand in the wrong columns): a page past the budget is refused
        for (size_t k = t.firstSite; k < sites.size(); ++k) {
            const Site &st = sites[k];
            const qint64 covered = qint64(st.cols) * st.rows - 1;
            if (covered == 0) continue;
            if (counts.spanCells >= lim.spanCells || counts.spanCovered + covered > lim.spanCovered) {
                sites.resize(t.firstSite);
                fail(HtmlRefusal::TooManyCells);
                return;
            }
            ++counts.spanCells;
            counts.spanCovered += covered;
        }
        for (size_t k = t.firstSite; k < sites.size(); ++k) {
            const Site &st = sites[k];
            for (int w = 0; w < 2; ++w) {
                const qsizetype at = w == 0 ? st.colAt : st.rowAt;
                if (at < 0) continue;
                int v = w == 0 ? st.cols : st.rows;
                for (int d = 5; d >= 0; --d, v /= 10) out[at + d] = QLatin1Char(char('0' + v % 10));
            }
        }
        sites.resize(t.firstSite);
        counts.grid += qint64(rows) * widest;
        if (counts.grid > lim.grid) fail(HtmlRefusal::TooManyCells);
    }

    // ---------------------------------------------------------------- attributes of the tag being read
    void clearAttributes() { seen = 0; }
    bool has(At a) const { return (seen & bit(a)) != 0; }
    // The number an attribute holds (digits, with a sign or a percent sign when `sign` or `percent`), clamped; false when it holds none.
    bool numberOf(At a, int most, bool sign, bool percent, int *value, bool *isPercent = nullptr) const
    {
        if (!has(a)) return false;
        QStringView v = s.mid(vb[a], ve[a] - vb[a]).trimmed();
        qsizetype k = 0;
        bool neg = false;
        if (sign && k < v.size() && (v[k] == QLatin1Char('+') || v[k] == QLatin1Char('-'))) neg = v[k++] == QLatin1Char('-');
        const qsizetype d0 = k;
        qint64 num = 0;
        while (k < v.size() && v[k].unicode() >= '0' && v[k].unicode() <= '9') {
            if (num < 100000000) num = num * 10 + (v[k].unicode() - '0');
            ++k;
        }
        if (k == d0) return false;
        if (isPercent) *isPercent = percent && k < v.size() && v[k] == QLatin1Char('%');
        *value = int(std::min<qint64>(num, most)) * (neg ? -1 : 1);
        return true;
    }
    static bool wordOk(QStringView v, int most)
    {
        if (v.isEmpty() || v.size() > most) return false;
        for (QChar c : v)
            if (!(c.unicode() < 128 && (c.isLetterOrNumber() || c == QLatin1Char('#') || c == QLatin1Char('-') || c == QLatin1Char('_') || c == QLatin1Char('%')))) return false;
        return true;
    }
    // Whether a declaration says what the part has without it (a spreadsheet's copy says it on every cell, and the parser takes time for each).
    bool isDefault(std::string_view name, const QString &value) const
    {
        const auto is = [&](const char *word) { return value == QLatin1String(word); };
        if (name.substr(0, 6) == "border" && name != "border-collapse") {
            const bool none = value.startsWith(QLatin1String("none")) || is("0") || value.startsWith(QLatin1String("0px")) || value.startsWith(QLatin1String("0pt"));
            // (a style says "no border" over the border attribute of the same part: that is not the default)
            int width = 0;
            const bool bordered = writingTable ? numberOf(ABorder, 100, false, false, &width) && width > 0 : !tables.empty() && tables.back().bordered;
            return none && !bordered;
        }
        if (name == "font-weight") return (is("400") || is("normal")) && pos[CB].empty() && pos[CHead].empty() && headers == 0;
        if (name == "font-style") return is("normal") && pos[CI].empty();
        if (name == "text-decoration") return is("none") && pos[CU].empty() && pos[CS].empty() && pos[CA].empty();
        if (name == "text-align") return is("general") || is("start") || is("auto") || is("inherit");
        if (name == "white-space") return is("normal");
        return false;
    }
    // A style attribute reduced to the properties that are kept.
    void putStyle(QStringView raw, bool keepSpacesToo = false)
    {
        // (the few entities a style says its quotes with are read first: the ";" of one is not the end of a declaration)
        QString read;
        read.reserve(std::min<qsizetype>(raw.size(), kMostStyleChars));
        for (qsizetype k = 0; k < raw.size() && read.size() < kMostStyleChars; ++k) {
            if (raw[k] == QLatin1Char('&')) {
                const QStringView rest = raw.mid(k, 6);
                if (rest.startsWith(QLatin1String("&quot;")) || rest.startsWith(QLatin1String("&#34;"))) {
                    read += QLatin1Char('"');
                    k += rest.startsWith(QLatin1String("&quot;")) ? 5 : 4;
                    continue;
                }
                if (rest.startsWith(QLatin1String("&apos;")) || rest.startsWith(QLatin1String("&#39;"))) {
                    read += QLatin1Char('\'');
                    k += rest.startsWith(QLatin1String("&apos;")) ? 5 : 4;
                    continue;
                }
                if (rest.startsWith(QLatin1String("&amp;"))) {
                    read += QLatin1Char('&');
                    k += 4;
                    continue;
                }
            }
            read += raw[k];
        }
        const QStringView v(read);
        QString kept;
        int count = 0;
        qsizetype k = 0;
        const qsizetype end = v.size();
        while (k < end && count < kMostStyleDeclarations) {
            // one declaration, to the ";" outside quotes and parentheses
            qsizetype e = k;
            int paren = 0;
            QChar quote;
            while (e < end) {
                const QChar c = v[e];
                if (!quote.isNull()) {
                    if (c == quote) quote = QChar();
                } else if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
                    quote = c;
                } else if (c == QLatin1Char('(')) {
                    ++paren;
                } else if (c == QLatin1Char(')')) {
                    if (paren > 0) --paren;
                } else if (c == QLatin1Char(';') && paren == 0) {
                    break;
                }
                ++e;
            }
            const QStringView decl = v.mid(k, e - k);
            k = e + 1;
            const qsizetype colon = decl.indexOf(QLatin1Char(':'));
            if (colon <= 0) continue;
            const QStringView nameView = decl.left(colon).trimmed();
            if (nameView.size() > 32) continue;
            char name[40];
            bool good = true;
            for (qsizetype t = 0; t < nameView.size(); ++t) {
                const ushort u = nameView[t].unicode();
                if (u >= 128) {
                    good = false;
                    break;
                }
                name[t] = char(lowerAscii(u));
            }
            if (!good || styleProperties().count(std::string_view(name, size_t(nameView.size()))) == 0) continue;
            QStringView value = decl.mid(colon + 1).trimmed();
            if (value.endsWith(QLatin1String("important"), Qt::CaseInsensitive)) {
                value = value.chopped(9).trimmed();
                if (value.endsWith(QLatin1Char('!'))) value = value.chopped(1).trimmed();
            }
            if (value.isEmpty() || value.size() > 100) continue;
            QString clean;
            for (qsizetype t = 0; t < value.size() && good; ++t) {
                const QChar c = value[t];
                const ushort u = c.unicode();
                if (u == '"') {
                    clean += QLatin1Char('\'');
                } else if (u < 128 && (c.isLetterOrNumber() || u == ' ' || u == '#' || u == '%' || u == '.' || u == ',' || u == '-' || u == '+' || u == '\'' || u == '(' || u == ')' || u == '/' || u == '!')) {
                    clean += c;
                } else if (u >= 128 && u > 0xA0) {
                    clean += c;     // (a font name in another script)
                } else {
                    good = false;
                }
            }
            if (!good) continue;
            const QString lower = clean.toLower();
            if (lower.contains(QLatin1String("url(")) || lower.contains(QLatin1String("expression")) || lower.contains(QLatin1String("image-set"))) continue;
            if (isDefault(std::string_view(name, size_t(nameView.size())), lower)) continue;
            if (!kept.isEmpty()) kept += QLatin1Char(';');
            kept += QString::fromLatin1(name, int(nameView.size())) + QLatin1Char(':') + clean;
            ++count;
        }
        if (keepSpacesToo) kept = kept.isEmpty() ? QStringLiteral("white-space:pre-wrap") : QStringLiteral("white-space:pre-wrap;") + kept;
        if (kept.isEmpty()) return;
        // (the parser takes about 5 microseconds for a style and 0.15 for each of its characters: a page keeps the styles of its first parts, up to a limit)
        if (counts.styles >= lim.styles || counts.styleChars + kept.size() > lim.styleChars) {
            ++counts.stylesLeftOut;
            return;
        }
        ++counts.styles;
        counts.styleChars += kept.size();
        out += QLatin1String(" style=\"");
        out += kept;
        out += QLatin1Char('"');
    }
    // Writes the kept attributes of a part. A span is written as three digits and set when the table ends.
    bool writingTable = false;   // the attributes being written are a table's own (its style then answers to its own border attribute, not to the table it stands in)
    void putAttributes(Cls c, bool picture, bool emitSpans, int row)
    {
        Site site;
        site.row = row;
        writingTable = c == CTable;
        const quint32 allowed = allowedAttributes(c, picture) & seen;
        if (!allowed) return;
        for (int a = 0; a < AtCount; ++a) {
            if (!(allowed & (quint32(1) << a))) continue;
            const QStringView raw = s.mid(vb[a], ve[a] - vb[a]);
            int number = 0;
            bool percent = false;
            switch (At(a)) {
            case AHref:
                if (raw.size() <= 4096 && linkAddressKept(raw)) {
                    out += QLatin1String(" href=\"");
                    putValue(raw);
                    out += QLatin1Char('"');
                }
                break;
            case AName: case AAlt: case AFace:
                if (raw.size() <= 256) {
                    out += QLatin1Char(' ');
                    out += QLatin1String(kAtName[a]);
                    out += QLatin1String("=\"");
                    putValue(raw);
                    out += QLatin1Char('"');
                }
                break;
            case ASrc:
                if (raw.size() <= qsizetype(lim.bytes)) {
                    out += QLatin1String(" src=\"");
                    putValue(raw);
                    out += QLatin1Char('"');
                }
                break;
            case AWidth: case AHeight:
                if (numberOf(At(a), 20000, false, true, &number, &percent)) out += QLatin1Char(' ') + QLatin1String(kAtName[a]) + QLatin1String("=\"") + QString::number(number) + (percent ? QLatin1String("%\"") : QLatin1String("\""));
                break;
            case ABorder: case ACellpadding: case ACellspacing:
                if (numberOf(At(a), 100, false, false, &number)) out += QLatin1Char(' ') + QLatin1String(kAtName[a]) + QLatin1String("=\"") + QString::number(number) + QLatin1String("\"");
                break;
            case ASize: case AStart: case AValue:
                if (numberOf(At(a), 999999, true, false, &number)) out += QLatin1Char(' ') + QLatin1String(kAtName[a]) + QLatin1String("=\"") + QString::number(number) + QLatin1String("\"");
                break;
            case AColspan: case ARowspan:
                if (emitSpans && numberOf(At(a), kMostHtmlSpan, false, false, &number) && number > 1) {
                    out += QLatin1Char(' ') + QLatin1String(kAtName[a]) + QLatin1String("=\"");
                    if (a == ARowspan) {
                        site.rowAt = out.size();
                        site.rows = number;
                    } else {
                        site.colAt = out.size();
                        site.cols = number;
                    }
                    out += QLatin1String("000000\"");
                }
                break;
            case ABgcolor: case AColor: case AAlign: case AValign: case AType: case ADir:
                if (wordOk(raw.trimmed(), 24)) {
                    out += QLatin1Char(' ');
                    out += QLatin1String(kAtName[a]);
                    out += QLatin1String("=\"");
                    out += raw.trimmed();
                    out += QLatin1Char('"');
                }
                break;
            case ANowrap:
                out += QLatin1String(" nowrap=\"nowrap\"");
                break;
            case AStyle:
                putStyle(raw);
                break;
            default:
                break;
            }
        }
        if (site.colAt >= 0 || site.rowAt >= 0) sites.push_back(site);
    }
    void openTag(const char *name, Cls c, bool picture = false, bool emitSpans = false, int row = 0)
    {
        out += QLatin1Char('<');
        out += QLatin1String(name);
        putAttributes(c, picture, emitSpans, row);
        out += QLatin1Char('>');
        part();
    }

    // ---------------------------------------------------------------- reading
    // Reads the rest of a start tag (its attributes, to the ">").
    void readAttributes()
    {
        clearAttributes();
        const auto &known = attributes();
        while (i < n) {
            const int ch = s[i].unicode();
            if (isBlankChar(ch) || ch == '/') {
                ++i;
                continue;
            }
            if (ch == '>') {
                ++i;
                return;
            }
            char name[16];
            int len = 0;
            const qsizetype nameStart = i;
            while (i < n) {
                const int c = s[i].unicode();
                if (isBlankChar(c) || c == '=' || c == '>' || (c == '/' && i + 1 < n && s[i + 1] == QLatin1Char('>'))) break;
                if (len < 15) name[len] = char(c < 128 ? lowerAscii(c) : '?');
                ++len;
                ++i;
            }
            if (i == nameStart) {
                ++i;   // (an "=" with no name)
                continue;
            }
            qsizetype b = i, e = i;
            qsizetype after = i;
            while (after < n && isBlankChar(s[after].unicode())) ++after;
            if (after < n && s[after] == QLatin1Char('=')) {
                ++after;
                while (after < n && isBlankChar(s[after].unicode())) ++after;
                if (after < n && (s[after] == QLatin1Char('"') || s[after] == QLatin1Char('\''))) {
                    const QChar q = s[after];
                    b = after + 1;
                    e = b;
                    while (e < n && s[e] != q) ++e;
                    i = e < n ? e + 1 : n;
                } else {
                    b = after;
                    e = b;
                    while (e < n && !isBlankChar(s[e].unicode()) && s[e] != QLatin1Char('>')) ++e;
                    i = e;
                }
            } else {
                b = e = i;
            }
            if (len > 0 && len <= 12) {
                const auto it = known.find(std::string_view(name, size_t(len)));
                if (it != known.end() && !(seen & bit(it->second))) {
                    seen |= bit(it->second);
                    vb[it->second] = b;
                    ve[it->second] = e;
                }
            }
        }
    }

    void openWrapper(const char *name)
    {
        out += QLatin1Char('<');
        out += QLatin1String(name);
        if (has(ADir) && wordOk(s.mid(vb[ADir], ve[ADir] - vb[ADir]).trimmed(), 8)) {
            out += QLatin1String(" dir=\"");
            out += s.mid(vb[ADir], ve[ADir] - vb[ADir]).trimmed();
            out += QLatin1Char('"');
        }
        // (a text editor writes the look of the words that have no look of their own on the body; and a page that says its spaces are kept has that said on the body)
        const bool body = name[0] == 'b';
        if (has(AStyle) || (body && wrapBody)) putStyle(has(AStyle) ? s.mid(vb[AStyle], ve[AStyle] - vb[AStyle]) : QStringView(), body && wrapBody);
        out += QLatin1Char('>');
        part();
    }
    void skipRaw(const char *name, int len)
    {
        // to the end tag of the part (or the end of the page)
        while (i < n) {
            if (s[i] == QLatin1Char('<') && i + 1 < n && s[i + 1] == QLatin1Char('/') && i + 2 + len <= n) {
                bool same = true;
                for (int k = 0; k < len && same; ++k) same = lowerAscii(s[i + 2 + k].unicode()) == name[k];
                const qsizetype after = i + 2 + len;
                if (same && (after >= n || isBlankChar(s[after].unicode()) || s[after] == QLatin1Char('>') || s[after] == QLatin1Char('/'))) {
                    i = after;
                    while (i < n && s[i] != QLatin1Char('>')) ++i;
                    if (i < n) ++i;
                    return;
                }
            }
            ++i;
        }
    }

    // One token of the page (a run of words, a tag, a comment); `work` counts the characters it went over, and each part closed.
    void step()
    {
        const qsizetype from = i;
        stepToken();
        counts.work += i - from + 1;
    }
    void stepToken()
    {
        if (s[i] != QLatin1Char('<')) {
            const qsizetype b = i;
            while (i < n && s[i] != QLatin1Char('<')) ++i;
            text(b, i);
            return;
        }
        if (i + 1 >= n) {
            text(i, i + 1);
            ++i;
            return;
        }
        const QChar next = s[i + 1];
        if (next == QLatin1Char('!')) {
            if (s.mid(i).startsWith(QLatin1String("<!--"))) {
                const qsizetype e = s.indexOf(QLatin1String("-->"), i + 4);
                // (the two comments that mark the part of a page that was copied are kept, the first of each: the parser reads only what is between them)
                if (e >= 0 && e - (i + 4) == 13 && !startMark && s.mid(i + 4, 13) == QLatin1String("StartFragment")) {
                    startMark = true;
                    out += QLatin1String("<!--StartFragment-->");
                } else if (e >= 0 && e - (i + 4) == 11 && !endMark && s.mid(i + 4, 11) == QLatin1String("EndFragment")) {
                    endMark = true;
                    out += QLatin1String("<!--EndFragment-->");
                }
                i = e < 0 ? n : e + 3;
            } else if (s.mid(i).startsWith(QLatin1String("<![CDATA["))) {
                const qsizetype e = s.indexOf(QLatin1String("]]>"), i + 9);
                i = e < 0 ? n : e + 3;
            } else {
                const qsizetype e = s.indexOf(QLatin1Char('>'), i);
                i = e < 0 ? n : e + 1;
            }
            return;
        }
        if (next == QLatin1Char('?')) {
            const qsizetype e = s.indexOf(QLatin1Char('>'), i);
            i = e < 0 ? n : e + 1;
            return;
        }
        const bool closing = next == QLatin1Char('/');
        const qsizetype nameStart = i + (closing ? 2 : 1);
        if (nameStart >= n || !isLetter(s[nameStart].unicode())) {
            // (a "<" that is a word)
            text(i, i + 1);
            ++i;
            return;
        }
        char buf[16];
        int len = 0;
        qsizetype k = nameStart;
        while (k < n) {
            const int c = s[k].unicode();
            if (isBlankChar(c) || c == '>' || c == '/') break;
            if (len < 15) buf[len] = char(c < 128 ? lowerAscii(c) : '?');
            ++len;
            ++k;
        }
        i = k;
        const std::string_view name(buf, size_t(std::min(len, 15)));
        if (closing) {
            const qsizetype e = s.indexOf(QLatin1Char('>'), i);
            i = e < 0 ? n : e + 1;
            if (len <= 12) endTag(name);
            return;
        }
        readAttributes();
        if (len > 12) return;
        if (isRawText(name)) {
            skipRaw(buf, len);
            return;
        }
        startTag(name);
    }

    // ---------------------------------------------------------------- words
    void text(qsizetype b, qsizetype e)
    {
        if (preOpen > 0) {
            preText(b, e);
            return;
        }
        const qint64 before = counts.text;
        for (qsizetype k = b; k < e;) {
            const ushort u = s[k].unicode();
            if (u == ' ' && keepSpaces) {
                flushSpace();
                out += QLatin1Char(' ');
                ++counts.text;
                atEdge = false;
                ++k;
                continue;
            }
            if (u == ' ' || u == '\t' || u == '\n' || u == '\r' || u == '\f') {
                pendingSpace = true;
                ++k;
                continue;
            }
            if (u < 0x20 || u == 0x7f) {
                ++k;
                continue;
            }
            flushSpace();
            atEdge = false;
            ++counts.text;
            if (u == '&') {
                putEntityOrAmp(s.left(e), k);
                continue;
            }
            ++k;
            if (u == '<') out += QLatin1String("&lt;");
            else if (u == '>') out += QLatin1String("&gt;");
            else out += QChar(u);
        }
        if (counts.text > lim.text && before <= lim.text) fail(HtmlRefusal::TooLarge);
    }
    void preText(qsizetype b, qsizetype e)
    {
        // (a line of its own paragraph when no part stands open inside the preformatted one; else a line break)
        const bool lines = !pos[CPre].empty() && pos[CPre].back() == int(stack.size()) - 1 && stack.back().emitted;
        for (qsizetype k = b; k < e;) {
            const ushort u = s[k].unicode();
            if (u == '\r' || u == '\n') {
                ++k;
                if (u == '\r' && k < e && s[k] == QLatin1Char('\n')) ++k;
                if (preFresh) {
                    preFresh = false;
                    continue;
                }
                if (lines) {
                    // (the parser makes no block of one that holds nothing: an empty line holds a space, which the block keeps)
                    if (atEdge) out += QLatin1Char(' ');
                    out += QLatin1String("</div><div>");
                    part();
                    atEdge = true;
                } else {
                    lineBreak();
                }
                continue;
            }
            preFresh = false;
            if (u < 0x20 && u != '\t') {
                ++k;
                continue;
            }
            atEdge = false;
            ++counts.text;
            if (u == '&') {
                putEntityOrAmp(s.left(e), k);
                continue;
            }
            ++k;
            if (u == '<') out += QLatin1String("&lt;");
            else if (u == '>') out += QLatin1String("&gt;");
            else out += QChar(u);
        }
        if (counts.text > lim.text) fail(HtmlRefusal::TooLarge);
    }

    // ---------------------------------------------------------------- tags
    void endTag(std::string_view name)
    {
        if (name == "br") {
            lineBreak();
            return;
        }
        const auto it = sources().find(name);
        if (it == sources().end()) return;
        const Cls c = it->second.cls;
        if (pos[c].empty()) return;
        const int p = pos[c].back();
        if (c == CTable) {
            // (any)
        } else if (c == CTr || c == CCell) {
            if (tables.empty() || p < tables.back().pos) return;
        } else if (p < barrier()) {
            return;
        }
        closeTo(p);
    }

    void startTag(std::string_view name)
    {
        if (name == "br") {
            lineBreak();
            return;
        }
        if (name == "html") {
            // (the page's parser sets the direction of each paragraph when the page has this part and the next: one of each is written, with its direction)
            if (htmlSeen) return;
            htmlSeen = true;
            openWrapper("html");
            return;
        }
        if (name == "body") {
            if (bodySeen) return;
            bodySeen = true;
            openWrapper("body");
            return;
        }
        if (name == "meta") {
            // (a page written by a text editor says its spaces are kept)
            if (has(AName) && has(AContent) && !keepSpaces && s.mid(vb[AName], ve[AName] - vb[AName]).trimmed().compare(QLatin1String("qrichtext"), Qt::CaseInsensitive) == 0
                && s.mid(vb[AContent], ve[AContent] - vb[AContent]).trimmed() == QLatin1String("1") && int(stack.size()) < lim.open) {
                keepSpaces = true;
                if (s.indexOf(QLatin1String("<body"), i, Qt::CaseInsensitive) >= 0) {
                    wrapBody = true;   // (said on the body when it comes)
                } else {
                    out += QLatin1String("<div style=\"white-space:pre-wrap\">");
                    part();
                    push({CDiv, "div"}, "div", true, false);
                }
            }
            return;
        }
        if (name == "hr") {
            closeParagraph();
            out += QLatin1String("<hr>");
            part();
            atEdge = true;
            pendingSpace = false;
            return;
        }
        if (name == "img") {
            // (a picture whose address no reader takes is not written: there is nothing to show, and nothing is left to fetch)
            if (!has(ASrc) || !pictureAddressKept(s.mid(vb[ASrc], ve[ASrc] - vb[ASrc]), kMostGuardedPictureBytes)) return;
            if (++counts.pictures > lim.pictures) {
                fail(HtmlRefusal::TooManyParts);
                return;
            }
            flushSpace();
            openTag("img", CSpan, true);
            atEdge = false;
            return;
        }
        const auto it = sources().find(name);
        if (it == sources().end()) return;
        const Src src = it->second;
        const Cls c = src.cls;
        if (int(stack.size()) >= lim.open) {
            ++counts.flattened;
            return;
        }
        switch (c) {
        case CTable: return startTable(src);
        case CTr: return startRow();
        case CCell: return startCell(src, name == "th");
        case CLi: case CDt: case CDd: return startItem(src);
        case CHead:
            closeParagraph();
            closeKind(CHead);
            break;
        case CP: case CDiv: case CQuote: case CCenter: case CPre: case CUl: case COl: case CDl:
            closeParagraph();
            break;
        case CA:
            closeKind(CA);
            break;
        default:
            break;
        }
        startPlain(src, src.out);
    }

    bool isBlockCls(Cls c) const { return c == CP || c == CHead || c == CDiv || c == CQuote || c == CCenter || c == CPre || c == CUl || c == COl || c == CDl || c == CLi || c == CDt || c == CDd; }

    // A part that is written, or (when it stands too deep) left out with its words kept.
    void startPlain(const Src &src, const char *name, int base = 0)
    {
        const bool block = isBlockCls(src.cls);
        const bool written = emitDepth + (src.cls == CPre ? 2 : 0) < lim.depth;
        if (!block) flushSpace();
        else {
            pendingSpace = false;
        }
        if (written) {
            if (src.cls == CPre) {
                out += QLatin1String(kPreWrapper);
                part();
                out += QLatin1String(kPreFirstLine);
                part();
                preFresh = true;
            } else {
                openTag(name, src.cls);
            }
            if (src.cls == COl && base > 0) {
                out.chop(1);
                out += QLatin1String(" start=\"") + QString::number(base + 1) + QLatin1String("\">");
            }
        } else {
            ++counts.flattened;
            if (src.cls == CPre) preFresh = true;
            if (block && !atEdge) lineBreak();
        }
        push(src, name, written, block);
        if (src.cls == CPre && written) {
            ++emitDepth;
            counts.deepest = std::max(counts.deepest, emitDepth);
        }
        stack.back().base = base;
        if (block) atEdge = true;
    }

    void startItem(const Src &src)
    {
        closeParagraph();
        const bool isLi = src.cls == CLi;
        // the list the item is in (one that is not open is opened)
        int listAt = -1;
        const Cls own = isLi ? CUl : CDl;
        const Cls alt = isLi ? COl : CDl;
        const int b = barrier();
        listAt = std::max(pos[own].empty() ? -1 : pos[own].back(), pos[alt].empty() ? -1 : pos[alt].back());
        if (listAt < 0 || listAt < b) {
            const Src implicit{own, isLi ? "ul" : "dl"};
            startPlain(implicit, implicit.out);
            listAt = int(stack.size()) - 1;
        }
        // the item that is open in it is closed
        for (const Cls item : {CLi, CDt, CDd})
            if (!pos[item].empty() && pos[item].back() > listAt) closeTo(pos[item].back());
        if (isLi && stack[size_t(listAt)].emitted && stack[size_t(listAt)].items >= lim.listItems) {
            // a long list is written as several (the parser is slow in a list of many items)
            const Frame was = stack[size_t(listAt)];
            const Src ls{was.cls, was.name};
            closeTo(listAt);
            ++counts.listSplits;
            startPlain(ls, was.name, was.cls == COl ? was.base + was.items : 0);
            listAt = int(stack.size()) - 1;
        }
        ++stack[size_t(listAt)].items;
        startPlain(src, src.out);
    }

    void startTable(const Src &src)
    {
        closeParagraph();
        const bool written = emitDepth + 3 <= lim.depth && openTables < lim.tableDepth;
        pendingSpace = false;
        if (written) {
            openTag("table", CTable);
            ++counts.tables;
            ++openTables;
        } else {
            ++counts.flattened;
            if (!atEdge) lineBreak();
        }
        push(src, "table", written, true);
        tables.push_back({int(stack.size()) - 1, written, sites.size()});
        int border = 0;
        if (numberOf(ABorder, 100, false, false, &border) && border > 0) tables.back().bordered = true;
        atEdge = true;
    }

    void startRow()
    {
        if (tables.empty()) return;
        const int tp = tables.back().pos;
        if (!pos[CCell].empty() && pos[CCell].back() > tp) closeTo(pos[CCell].back());
        if (!pos[CTr].empty() && pos[CTr].back() > tp) closeTo(pos[CTr].back());
        if (why != HtmlRefusal::None) return;
        openRowFor(true);
    }
    // A row: written when the table is. (`explicitTag` keeps the row's own attributes; a row made for a cell has none.)
    void openRowFor(bool explicitTag)
    {
        TableLevel &t = tables.back();
        ++counts.rows;
        if (counts.rows > lim.rows) {
            fail(HtmlRefusal::TooManyCells);
            return;
        }
        t.rowCells.push_back(0);
        const bool written = t.emitted;
        if (written) {
            if (explicitTag) openTag("tr", CTr);
            else {
                out += QLatin1String("<tr>");
                part();
            }
        } else {
            ++counts.flattened;
            if (!atEdge) lineBreak();
        }
        push({CTr, "tr"}, "tr", written, true);
        pendingSpace = false;
        atEdge = true;
    }

    void startCell(const Src &src, bool header)
    {
        if (tables.empty()) return;
        const int tp = tables.back().pos;
        if (!pos[CCell].empty() && pos[CCell].back() > tp) closeTo(pos[CCell].back());
        if (why != HtmlRefusal::None) return;
        if (pos[CTr].empty() || pos[CTr].back() <= tp) {
            openRowFor(false);
            if (why != HtmlRefusal::None) return;
        }
        TableLevel &t = tables.back();
        if (++counts.cells > lim.cells) {
            fail(HtmlRefusal::TooManyCells);
            return;
        }
        const int row = int(t.rowCells.size()) - 1;
        ++t.rowCells.back();
        t.maxCells = std::max(t.maxCells, t.rowCells.back());
        const char *name = header ? "th" : "td";
        const bool written = t.emitted;
        if (written) {
            openTag(name, CCell, false, true, row);
        } else {
            ++counts.flattened;
            pendingSpace = true;
        }
        push(src, name, written, true);
        if (header) {
            stack.back().header = true;
            ++headers;
        }
        if (written) {
            pendingSpace = false;
            atEdge = true;
        }
    }
};

} // namespace

GuardedHtml guardHtml(const QString &html, const HtmlLimits &limits) { return Rewriter(QStringView(html), limits).run(); }

GuardedHtml guardHtml(const QByteArray &utf8, const HtmlLimits &limits)
{
    if (utf8.size() > limits.bytes) {
        GuardedHtml g;
        g.refusal = HtmlRefusal::TooLarge;
        return g;
    }
    return guardHtml(QString::fromUtf8(utf8), limits);
}

QString htmlRefusalText(HtmlRefusal why)
{
    if (why == HtmlRefusal::TooManyCells) return QCoreApplication::translate("jp::office::wml", "This web page has too many table rows or cells to open.");
    if (why == HtmlRefusal::TooManyParts) return QCoreApplication::translate("jp::office::wml", "This web page is too complicated to open.");
    return QCoreApplication::translate("jp::office::wml", "The web page is too large to open.");
}

QString htmlPlainText(const QString &html)
{
    QString out;
    out.reserve(html.size() / 2);
    qsizetype i = 0;
    const qsizetype n = html.size();
    while (i < n) {
        if (html.at(i) != QLatin1Char('<')) {
            out += html.at(i++);
            continue;
        }
        const qsizetype end = html.indexOf(QLatin1Char('>'), i);
        if (end < 0) break;
        i = end + 1;
    }
    out.replace(QLatin1String("&nbsp;"), QLatin1String(" ")).replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\""));
    out.replace(QLatin1String("&amp;"), QLatin1String("&"));
    return out.simplified();
}

std::unique_ptr<QMimeData> guardedMimeData(const QMimeData *source, bool *refused)
{
    if (refused) *refused = false;
    if (!source) return nullptr;
    // (a text box parses a page from these two formats, and a Markdown text it reads with its own reader)
    static const QString kRich = QStringLiteral("application/x-qrichtext");
    const bool hasRich = source->hasFormat(kRich);
    if (!source->hasHtml() && !hasRich) return nullptr;
    auto copy = std::make_unique<QMimeData>();
    GuardedHtml page, rich;
    if (source->hasHtml()) page = guardHtml(source->html());
    if (hasRich) rich = guardHtml(source->data(kRich));
    if (page.refusal != HtmlRefusal::None || rich.refusal != HtmlRefusal::None) {
        if (refused) *refused = true;
        copy->setText(source->hasText() ? source->text() : htmlPlainText(source->html()));
        return copy;
    }
    // (in the order they came: a text box takes the first format it knows, and a Markdown text first of all)
    for (const QString &format : source->formats()) {
        if (format == QLatin1String("text/html")) copy->setHtml(page.html);
        else if (format == kRich) copy->setData(kRich, rich.html.toUtf8());
        else copy->setData(format, source->data(format));
    }
    return copy;
}

QString htmlPasteRefusedNote(bool pastedText)
{
    if (pastedText) return QCoreApplication::translate("jp::office::wml", "The copied page was too large or too complicated to read, so its plain text was pasted.");
    return QCoreApplication::translate("jp::office::wml", "The copied page was too large or too complicated to read, so nothing was pasted.");
}

} // namespace jp::office::wml
