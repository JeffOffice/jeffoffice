#include "wml/htmlguard.h"

#include <QCoreApplication>
#include <QHash>
#include <QHashFunctions>
#include <QMimeData>

#include <algorithm>
#include <cstring>
#include <vector>

namespace jp::office::wml {

namespace {

// What the parser's time follows, in nanoseconds of this machine (measured with the page's own parser, Nov 26 2026, on the shapes in tests/office/tst_wml.cpp and by hand: a part's start
// costs about 5 microseconds (a cell 7) and each level of depth under it 60 nanoseconds; a list item 80 nanoseconds for each item before it in its list; a picture 400 for each before it in its
// paragraph; an empty cell of a table's grid (the grid is its rows times its widest row, less the cells it has) 4.5 microseconds; each byte of the page 50). Each is the measured number rounded up: the page's own parser took no more than the model says on any shape tried.
constexpr qint64 kStartNs = 5000, kCellNs = 2000, kStrayCellNs = 8000, kTextNs = 1000, kCloseNs = 300, kDepthNs = 60, kItemNs = 80, kPictureNs = 400, kPictureBaseNs = 5000, kGridNs = 4500, kByteNs = 50, kMarkupNs = 50;
constexpr int kMostColspan = 1000000;

enum Kind : quint8 { KOther, KTable, KTr, KCell, KLi, KList, KDl, KDt, KP, KSection, KindCount };

struct Known {
    const char *name;
    Kind kind;
    bool empty;     // the parser knows it has no end (and no other part is known to be one: input, link, base, col, and the rest of what a page may call empty are parts that stand open)
    bool picture;
    bool block;     // starts a new paragraph (a run of pictures begins again)
};

// (sorted: looked up by binary search)
const Known kKnown[] = {
    {"blockquote", KOther, false, false, true}, {"br", KOther, true, false, false}, {"caption", KSection, false, false, false}, {"center", KOther, false, false, true},
    {"dd", KDt, false, false, true}, {"div", KOther, false, false, true}, {"dl", KDl, false, false, false}, {"dt", KDt, false, false, true},
    {"h1", KOther, false, false, true}, {"h2", KOther, false, false, true}, {"h3", KOther, false, false, true}, {"h4", KOther, false, false, true},
    {"h5", KOther, false, false, true}, {"h6", KOther, false, false, true}, {"hr", KOther, true, false, true}, {"img", KOther, true, true, false},
    {"li", KLi, false, false, true}, {"meta", KOther, true, false, false}, {"ol", KList, false, false, false}, {"p", KP, false, false, true},
    {"pre", KOther, false, false, true}, {"table", KTable, false, false, false}, {"tbody", KSection, false, false, false}, {"td", KCell, false, false, true},
    {"tfoot", KSection, false, false, false}, {"th", KCell, false, false, true}, {"thead", KSection, false, false, false}, {"tr", KTr, false, false, false},
    {"ul", KList, false, false, false},
};

const Known *known(const char *name, int len)
{
    static const Known none{"", KOther, false, false, false};
    if (len <= 0 || len > 10) return &none;
    const Known *first = std::begin(kKnown), *last = std::end(kKnown);
    while (first < last) {
        const Known *mid = first + (last - first) / 2;
        const int c = std::strncmp(mid->name, name, size_t(len));
        const int cmp = c != 0 ? c : (mid->name[len] ? 1 : 0);
        if (cmp == 0) return mid;
        if (cmp < 0) first = mid + 1;
        else last = mid;
    }
    return &none;
}

struct Frame {
    quint64 key;
    Kind kind;
    qint64 items = 0;     // a list: its items so far
    int prevList = -1;    // a list: the list that was the innermost before it
};

struct Grid {
    qint64 rows = 0, cur = 0, widest = 0, cells = 0;
};

template <class Get>
HtmlScan scan(qsizetype n, Get at, const HtmlLimits &lim)
{
    HtmlScan out;
    if (n > lim.bytes) {
        out.refusal = HtmlRefusal::TooLarge;
        return out;
    }
    std::vector<Frame> stack;
    std::vector<Grid> grids;
    QHash<quint64, int> open;      // how many parts of each name are open
    int openKind[KindCount] = {};
    int lastList = -1;
    qint64 pictures = 0, cells = 0, rows = 0;
    const size_t seed = size_t(QHashSeed::globalSeed());
    bool bad = false;
    auto add = [&](qint64 ns) {
        out.work += ns;
        if (out.work > lim.work) bad = true;
    };
    auto finishGrid = [&](const Grid &g) { add(kGridNs * std::max<qint64>(0, std::max<qint64>(g.rows, 1) * g.widest - g.cells)); };
    auto popTo = [&](qsizetype keep) {
        while (qsizetype(stack.size()) > keep) {
            const Frame f = stack.back();
            stack.pop_back();
            --open[f.key];
            --openKind[f.kind];
            if (f.kind == KList) lastList = f.prevList;
            if (f.kind == KTable && !grids.empty()) {
                finishGrid(grids.back());
                grids.pop_back();
            }
        }
    };
    // The innermost open part of kind `target` above any part of kind `stop1` or `stop2` is closed with everything above it.
    auto closeKind = [&](Kind target, Kind stop1, Kind stop2) {
        if (!openKind[target]) return;
        for (qsizetype k = qsizetype(stack.size()) - 1; k >= 0; --k) {
            ++out.walked;
            add(kDepthNs);
            const Kind kind = stack[size_t(k)].kind;
            if (kind == stop1 || kind == stop2) return;
            if (kind == target) {
                popTo(k);
                return;
            }
        }
    };
    add(qint64(n) * kByteNs);
    qsizetype i = 0;
    while (i < n && !bad) {
        if (at(i) != '<') {
            while (i < n && at(i) != '<') ++i;
            add(kTextNs + qint64(stack.size() + 1) * kDepthNs);
            continue;
        }
        ++i;
        if (i >= n) break;
        if (at(i) == '!') {
            // (a comment ends at "-->", anything else of this kind at ">")
            add(kMarkupNs);
            if (i + 2 < n && at(i + 1) == '-' && at(i + 2) == '-') {
                i += 3;
                while (i + 2 < n && !(at(i) == '-' && at(i + 1) == '-' && at(i + 2) == '>')) ++i;
                i += 3;
            } else {
                while (i < n && at(i) != '>') ++i;
                ++i;
            }
            continue;
        }
        const qsizetype tagStart = i;
        const bool closing = at(i) == '/';
        if (closing) ++i;
        // (the parser lets blanks stand between "<" and the name of a start tag, and takes anything else but a blank, ">", or "/" to be part of the name)
        if (!closing)
            while (i < n && (at(i) == ' ' || at(i) == '\t' || at(i) == '\n' || at(i) == '\r' || at(i) == '\f')) ++i;
        char buf[16];
        int len = 0;
        quint64 key = 0xcbf29ce484222325ull ^ quint64(seed);
        while (i < n) {
            const int ch = at(i);
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '>' || ch == '/') break;
            const int low = ch >= 'A' && ch <= 'Z' ? ch + 32 : ch;
            if (len < 16) buf[len] = char(low);
            ++len;
            key = (key ^ quint64(low)) * 0x100000001b3ull;
            ++i;
        }
        if (len == 0) {
            // (not a tag: a "<" in the text; what follows is looked at as it comes)
            i = tagStart;
            add(kTextNs);
            continue;
        }
        const Known &kn = *known(buf, len);
        // The rest of the tag: its attributes (a cell's colspan is read), to the ">" that is not in quotes.
        bool selfClosing = false;
        qint64 colspan = 1;
        while (i < n) {
            const int ch = at(i);
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f') {
                ++i;
                continue;
            }
            if (ch == '>') {
                ++i;
                break;
            }
            if (ch == '/') {
                if (i + 1 < n && at(i + 1) == '>') {
                    selfClosing = true;
                    i += 2;
                    break;
                }
                ++i;
                continue;
            }
            char attr[8];
            int alen = 0;
            while (i < n) {
                const int c = at(i);
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '=' || c == '>' || c == '/') break;
                if (alen < 8) attr[alen] = char(c >= 'A' && c <= 'Z' ? c + 32 : c);
                ++alen;
                ++i;
            }
            if (alen == 0) {
                ++i;   // (an "=" with no name: past it)
                continue;
            }
            while (i < n && (at(i) == ' ' || at(i) == '\t' || at(i) == '\n' || at(i) == '\r' || at(i) == '\f')) ++i;
            if (i < n && at(i) == '=') {
                ++i;
                while (i < n && (at(i) == ' ' || at(i) == '\t' || at(i) == '\n' || at(i) == '\r' || at(i) == '\f')) ++i;
                qint64 number = 0;
                bool digits = true;
                int seen = 0;
                const bool quoted = i < n && (at(i) == '"' || at(i) == '\'');
                const int quote = quoted ? at(i) : 0;
                if (quoted) ++i;
                while (i < n) {
                    const int c = at(i);
                    if (quoted ? c == quote : (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '>')) break;
                    if (c >= '0' && c <= '9') {
                        if (digits && seen < 8) number = number * 10 + (c - '0');
                        ++seen;
                    } else if (seen > 0 || (c != ' ' && c != '+')) {
                        digits = false;
                    }
                    ++i;
                }
                if (quoted && i < n) ++i;
                if (kn.kind == KCell && alen == 7 && std::memcmp(attr, "colspan", 7) == 0 && seen > 0) colspan = std::clamp<qint64>(number, 1, kMostColspan);
            }
        }
        const qint64 depth = qint64(stack.size());
        if (closing) {
            add(kCloseNs);
            if (!open.value(key)) {
                add(depth * kDepthNs);
                continue;
            }
            for (qsizetype k = qsizetype(stack.size()) - 1; k >= 0; --k) {
                ++out.walked;
                add(kDepthNs);
                if (stack[size_t(k)].key == key) {
                    popTo(k);
                    break;
                }
            }
            continue;
        }
        add(kMarkupNs);
        if (kn.kind == KCell) {
            if (++cells > lim.cells) {
                out.refusal = HtmlRefusal::TooManyCells;
                return out;
            }
            closeKind(KCell, KTable, KSection);
        } else if (kn.kind == KTr) {
            if (++rows > lim.rows) {
                out.refusal = HtmlRefusal::TooManyCells;
                return out;
            }
            closeKind(KCell, KTable, KSection);
            closeKind(KTr, KTable, KTable);
        } else if (kn.kind == KDt) {
            closeKind(KDt, KDl, KDl);
        } else if ((kn.kind == KLi || kn.kind == KP) && !stack.empty() && stack.back().kind == kn.kind) {
            popTo(qsizetype(stack.size()) - 1);
        }
        if (kn.block) pictures = 0;
        const qint64 level = qint64(stack.size()) + 1;
        add(kStartNs + (kn.kind == KCell ? kCellNs : 0) + level * kDepthNs);
        if (kn.kind == KLi && lastList >= 0) add(kItemNs * stack[size_t(lastList)].items++);
        if (kn.picture) add(kPictureBaseNs + kPictureNs * ++pictures);
        if (!grids.empty() && kn.kind == KTr) {
            ++grids.back().rows;
            grids.back().cur = 0;
        }
        if (kn.kind == KCell && grids.empty()) add(kStrayCellNs);
        if (!grids.empty() && kn.kind == KCell) {
            Grid &g = grids.back();
            if (g.rows == 0) g.rows = 1;
            ++g.cells;
            g.cur += colspan;
            g.widest = std::max(g.widest, g.cur);
        }
        if (selfClosing || kn.empty) continue;
        Frame f{key, kn.kind};
        if (kn.kind == KList) {
            f.prevList = lastList;
            lastList = int(stack.size());
        }
        stack.push_back(f);
        ++open[key];
        ++openKind[kn.kind];
        if (kn.kind == KTable) {
            grids.push_back(Grid());
            if (openKind[KTable] > lim.tableDepth) {
                out.refusal = HtmlRefusal::TooDeep;
                return out;
            }
        }
        out.deepest = std::max(out.deepest, int(stack.size()));
        if (int(stack.size()) > lim.depth) {
            out.refusal = HtmlRefusal::TooDeep;
            return out;
        }
    }
    for (const Grid &g : grids) finishGrid(g);
    if (bad || out.work > lim.work) out.refusal = HtmlRefusal::TooMuchWork;
    return out;
}

} // namespace

HtmlScan scanHtml(const QByteArray &html, const HtmlLimits &limits)
{
    return scan(html.size(), [&](qsizetype k) { return int(quint8(html.constData()[k])); }, limits);
}

HtmlScan scanHtml(const QString &html, const HtmlLimits &limits)
{
    return scan(html.size(), [&](qsizetype k) { return int(html.at(k).unicode()); }, limits);
}

HtmlRefusal htmlRefusal(const QByteArray &html) { return scanHtml(html).refusal; }

HtmlRefusal htmlRefusal(const QString &html, qint64 mostBytes)
{
    HtmlLimits limits;
    limits.bytes = mostBytes;
    return scanHtml(html, limits).refusal;
}

QString htmlRefusalText(HtmlRefusal why)
{
    if (why == HtmlRefusal::TooDeep) return QCoreApplication::translate("jp::office::wml", "This web page nests its tables or other parts too deeply to open.");
    if (why == HtmlRefusal::TooManyCells) return QCoreApplication::translate("jp::office::wml", "This web page has too many table rows or cells to open.");
    if (why == HtmlRefusal::TooMuchWork) return QCoreApplication::translate("jp::office::wml", "This web page is too complicated to open.");
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

std::unique_ptr<QMimeData> plainTextInsteadOfRefusedHtml(const QMimeData *source)
{
    if (!source || !source->hasHtml()) return nullptr;
    const QString html = source->html();
    if (htmlRefusal(html) == HtmlRefusal::None) return nullptr;
    auto plain = std::make_unique<QMimeData>();
    plain->setText(source->hasText() ? source->text() : htmlPlainText(html));
    return plain;
}

QString htmlPasteRefusedNote(bool pastedText)
{
    if (pastedText) return QCoreApplication::translate("jp::office::wml", "The copied page was too deeply nested or too complicated to read, so its plain text was pasted.");
    return QCoreApplication::translate("jp::office::wml", "The copied page was too deeply nested or too complicated to read, so nothing was pasted.");
}

} // namespace jp::office::wml
