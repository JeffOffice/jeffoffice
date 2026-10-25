#include "sheet/drawingsave.h"

#include "sheet/xmlscan.h"

#include <QLocale>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>
#include <set>

namespace jp::office::sheet {

using xmlscan::Tag;

namespace {

// The end (after its closing tag) of the element whose open tag is `open`.
qsizetype elementEnd(const QByteArray &xml, const Tag &open)
{
    if (open.selfClosing) return open.end;
    int depth = 1;
    qsizetype pos = open.end;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            if (--depth == 0) return t.end;
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    return -1;
}

// The children of the root: where each starts and ends.
bool childrenOfRoot(const QByteArray &xml, std::vector<std::pair<qsizetype, qsizetype>> *out)
{
    Tag t;
    qsizetype pos = 0;
    bool haveRoot = false;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        haveRoot = true;
        break;
    }
    if (!haveRoot) return false;
    if (t.selfClosing) return true;
    int depth = 1;
    qsizetype childStart = -1;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (depth == 1 && !t.closing) {
            childStart = t.start;
            if (t.selfClosing) {
                out->emplace_back(childStart, t.end);
                continue;
            }
            depth = 2;
            continue;
        }
        if (t.closing) {
            if (--depth == 1) out->emplace_back(childStart, t.end);
            if (depth == 0) break;
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    return true;
}

const QRegularExpression &relAttributeRe()
{
    static const QRegularExpression re(QStringLiteral("\\b[A-Za-z_][\\w.-]*:(?:embed|link|id|pict)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    return re;
}

} // namespace

QStringList namedRelationships(const QByteArray &xml)
{
    QStringList out;
    const QString text = QString::fromUtf8(xml);
    QRegularExpressionMatchIterator it = relAttributeRe().globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString id = m.captured(1).isEmpty() ? m.captured(2) : m.captured(1);
        if (!id.isEmpty() && !out.contains(id)) out << id;
    }
    return out;
}

bool removeDrawingObjects(const QByteArray &xml, const std::vector<int> &ordinals, QByteArray *out, bool *changed, QStringList *relIds)
{
    *out = xml;
    *changed = false;
    if (ordinals.empty()) return true;
    const std::set<int> gone(ordinals.begin(), ordinals.end());
    std::vector<std::pair<qsizetype, qsizetype>> children;
    if (!childrenOfRoot(xml, &children)) return false;
    QByteArray result;
    qsizetype copied = 0;
    bool any = false;
    for (size_t i = 0; i < children.size(); ++i) {
        if (!gone.count(int(i))) continue;
        result += xml.mid(copied, children[i].first - copied);
        copied = children[i].second;
        any = true;
        if (relIds)
            for (const QString &id : namedRelationships(xml.mid(children[i].first, children[i].second - children[i].first)))
                if (!relIds->contains(id)) *relIds << id;
    }
    if (!any) return true;
    result += xml.mid(copied);
    *out = result;
    *changed = *out != xml;
    return true;
}

bool reorderDrawingChildren(const QByteArray &xml, const std::vector<int> &order, QByteArray *out, bool *changed)
{
    *out = xml;
    *changed = false;
    std::vector<std::pair<qsizetype, qsizetype>> children;
    if (!childrenOfRoot(xml, &children) || children.size() != order.size()) return false;
    std::vector<char> seen(order.size(), 0);
    bool identity = true;
    for (size_t k = 0; k < order.size(); ++k) {
        const int at = order[k];
        if (at < 0 || size_t(at) >= order.size() || seen[size_t(at)]) return false;
        seen[size_t(at)] = 1;
        identity = identity && size_t(at) == k;
    }
    if (identity || children.empty()) return true;
    QByteArray result = xml.left(children.front().first);
    for (size_t k = 0; k < order.size(); ++k) {
        const auto &c = children[size_t(order[k])];
        result += xml.mid(c.first, c.second - c.first);
        if (k + 1 < children.size()) result += xml.mid(children[k].second, children[k + 1].first - children[k].second);
    }
    result += xml.mid(children.back().second);
    *out = result;
    *changed = *out != xml;
    return true;
}

bool setDrawingAnchors(const QByteArray &xml, const std::map<int, DrawingAnchor> &anchors, QByteArray *out, bool *changed)
{
    *out = xml;
    *changed = false;
    if (anchors.empty()) return true;
    std::vector<std::pair<qsizetype, qsizetype>> children;
    if (!childrenOfRoot(xml, &children)) return false;
    QByteArray result;
    qsizetype copied = 0;
    auto point = [](const QByteArray &prefix, const char *tag, const AnchorPoint &p) {
        return "<" + prefix + tag + "><" + prefix + "col>" + QByteArray::number(qulonglong(p.col)) + "</" + prefix + "col><" + prefix + "colOff>" + QByteArray::number(qlonglong(p.colOff)) + "</" + prefix + "colOff><" + prefix + "row>" +
               QByteArray::number(qulonglong(p.row)) + "</" + prefix + "row><" + prefix + "rowOff>" + QByteArray::number(qlonglong(p.rowOff)) + "</" + prefix + "rowOff></" + prefix + tag + ">";
    };
    for (const auto &[ordinal, anchor] : anchors) {
        if (ordinal < 0 || size_t(ordinal) >= children.size() || anchor.kind != AnchorKind::TwoCell) continue;
        const qsizetype from0 = children[size_t(ordinal)].first, to0 = children[size_t(ordinal)].second;
        if (from0 < copied) continue;
        // the first anchor of the child (a two-cell, one-cell, or absolute one)
        Tag t;
        qsizetype pos = from0;
        Tag open;
        bool found = false;
        while (xmlscan::nextTag(xml, pos, &t) && t.start < to0) {
            pos = t.end;
            if (t.closing) continue;
            if (t.local == "twoCellAnchor" || t.local == "oneCellAnchor" || t.local == "absoluteAnchor") {
                open = t;
                found = true;
                break;
            }
        }
        if (!found) continue;
        const qsizetype anchorEnd = elementEnd(xml, open);
        if (anchorEnd < 0) continue;
        const QByteArray prefix(xmlscan::prefixOf(open.name).data(), xmlscan::prefixOf(open.name).size());
        // what the anchor holds besides its places (the object, its client data...) is kept as it is; the places are written again
        QByteArray kept;
        qsizetype keepFrom = open.end, closeStart = anchorEnd;
        if (!open.selfClosing) {
            int depth = 1;
            Tag c;
            qsizetype at = open.end;
            qsizetype childStart = -1;
            while (xmlscan::nextTag(xml, at, &c) && c.start < anchorEnd) {
                at = c.end;
                if (c.closing) {
                    if (--depth == 0) {
                        closeStart = c.start;
                        break;
                    }
                    if (depth == 1 && childStart >= 0) {
                        const QByteArrayView l = c.local;
                        if (l == "from" || l == "to" || l == "pos" || l == "ext") {
                            kept += xml.mid(keepFrom, childStart - keepFrom);
                            keepFrom = c.end;
                        }
                        childStart = -1;
                    }
                    continue;
                }
                if (depth == 1) {
                    childStart = c.start;
                    const QByteArrayView l = c.local;
                    if (c.selfClosing && (l == "from" || l == "to" || l == "pos" || l == "ext")) {
                        kept += xml.mid(keepFrom, c.start - keepFrom);
                        keepFrom = c.end;
                        childStart = -1;
                    }
                }
                if (!c.selfClosing) ++depth;
            }
            kept += xml.mid(keepFrom, closeStart - keepFrom);
        }
        QByteArray tag = xml.mid(open.start, open.end - open.start);
        const bool selfClosed = open.selfClosing;
        if (selfClosed) {
            tag.chop(2);
            tag += ">";
        }
        // the element is a two-cell anchor, with what it does when cells change as `anchor` says
        const QByteArray oldName(open.name);
        const QByteArray newName = prefix + "twoCellAnchor";
        tag.replace(0, 1 + oldName.size(), "<" + newName);
        qsizetype vs = 0, ve = 0;
        if (xmlscan::attributeValue(tag, "editAs", &vs, &ve)) {
            // (the attribute and the space before it)
            qsizetype from = vs;
            while (from > 0 && tag[from - 1] != ' ' && tag[from - 1] != '\n' && tag[from - 1] != '\t' && tag[from - 1] != '\r') --from;
            while (from > 0 && (tag[from - 1] == ' ' || tag[from - 1] == '\n' || tag[from - 1] == '\t' || tag[from - 1] == '\r')) --from;
            tag.remove(from, ve + 1 - from);
        }
        if (anchor.editAs != AnchorKind::TwoCell) {
            const qsizetype at = tag.size() - 1;
            tag.insert(at, anchor.editAs == AnchorKind::OneCell ? " editAs=\"oneCell\"" : " editAs=\"absolute\"");
        }
        result += xml.mid(copied, open.start - copied) + tag + point(prefix, "from", anchor.from) + point(prefix, "to", anchor.to) + kept + "</" + newName + ">";
        copied = anchorEnd;
    }
    if (copied == 0 && result.isEmpty()) return true;
    result += xml.mid(copied);
    *out = result;
    *changed = *out != xml;
    return true;
}

QStringList chartPartRefs(const QByteArray &xml)
{
    QStringList out;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.selfClosing || t.local != "f") continue;
        Tag close;
        if (!xmlscan::nextTag(xml, t.end, &close) || !close.closing || close.local != "f") continue;
        const QString ref = xmlscan::unescape(QByteArrayView(xml).sliced(t.end, close.start - t.end)).trimmed();
        if (!ref.isEmpty() && out.size() < 100000) out << ref;
        pos = close.end;
    }
    return out;
}

namespace {

// What a cache holds: its points by place, and how many places it says it has.
struct CacheContent {
    qint64 count = -1;
    std::map<qint64, QString> points;
    QString format;
};

// The cache element at [from, to) of the part: its points.
CacheContent readCache(const QByteArray &xml, qsizetype from, qsizetype to)
{
    CacheContent c;
    Tag t;
    qsizetype pos = from;
    qint64 idx = -1;
    bool inV = false, inFormat = false;
    qsizetype textStart = 0;
    while (xmlscan::nextTag(xml, pos, &t) && t.start < to) {
        pos = t.end;
        const QByteArrayView whole = QByteArrayView(xml).sliced(t.start, t.end - t.start);
        if (t.local == "ptCount" && !t.closing) {
            bool found = false;
            const qint64 n = xmlscan::attribute(whole, "val", &found).toLongLong();
            c.count = found ? n : -1;
        } else if (t.local == "pt" && !t.closing) {
            bool found = false;
            const qint64 n = xmlscan::attribute(whole, "idx", &found).toLongLong();
            idx = found ? n : -1;
        } else if (t.local == "v") {
            if (!t.closing) {
                inV = true;
                textStart = t.end;
            } else if (inV) {
                if (idx >= 0) c.points[idx] = xmlscan::unescape(QByteArrayView(xml).sliced(textStart, t.start - textStart));
                inV = false;
            }
        } else if (t.local == "formatCode") {
            if (!t.closing) {
                inFormat = true;
                textStart = t.end;
            } else if (inFormat) {
                c.format = xmlscan::unescape(QByteArrayView(xml).sliced(textStart, t.start - textStart));
                inFormat = false;
            }
        }
    }
    return c;
}

QString numberText(double v) { return QLocale::c().toString(v, 'g', QLocale::FloatingPointShortest); }

bool sameNumberText(const QString &a, const QString &b)
{
    bool okA = false, okB = false;
    const double x = a.toDouble(&okA), y = b.toDouble(&okB);
    if (!okA || !okB) return a == b;
    return x == y || std::abs(x - y) <= 1e-15 * std::max(std::abs(x), std::abs(y));
}

// Where the parts of a reference element are in the part: its formula's text, its cache, and its end.
struct RefPlace {
    qsizetype fText0 = -1, fText1 = -1, fEnd = -1;
    qsizetype cache0 = -1, cache1 = -1;
    qsizetype end = -1;
};

bool scanRef(const QByteArray &xml, const Tag &open, bool numeric, RefPlace *r)
{
    const QByteArrayView cacheName = numeric ? "numCache" : "strCache";
    int depth = 1;
    qsizetype pos = open.end;
    bool inF = false;
    Tag u;
    while (xmlscan::nextTag(xml, pos, &u)) {
        pos = u.end;
        if (u.closing) {
            --depth;
            if (depth == 1 && u.local == cacheName && r->cache0 >= 0 && r->cache1 < 0) r->cache1 = u.end;
            if (depth == 1 && inF && u.local == "f") {
                r->fText1 = u.start;
                r->fEnd = u.end;
                inF = false;
            }
            if (depth == 0) {
                r->end = u.end;
                return true;
            }
        } else {
            if (depth == 1 && u.local == "f" && !u.selfClosing && r->fText0 < 0) {
                inF = true;
                r->fText0 = u.end;
            }
            if (depth == 1 && u.local == cacheName && r->cache0 < 0) {
                r->cache0 = u.start;
                if (u.selfClosing) r->cache1 = u.end;
            }
            if (!u.selfClosing) ++depth;
        }
    }
    return false;
}

} // namespace

bool refreshChartCaches(const QByteArray &xml, const ChartCaches &caches, QByteArray *out, bool *changed)
{
    *out = xml;
    *changed = false;
    if (caches.empty()) return true;
    QByteArray result;
    qsizetype copied = 0, pos = 0;
    bool any = false;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.selfClosing || (t.local != "numRef" && t.local != "strRef")) continue;
        const bool numeric = t.local == "numRef";
        RefPlace r;
        if (!scanRef(xml, t, numeric, &r)) break;
        pos = r.end;
        if (r.fText0 < 0 || r.fText1 < 0) continue;
        const QString ref = xmlscan::unescape(QByteArrayView(xml).sliced(r.fText0, r.fText1 - r.fText0)).trimmed();
        const auto wanted = caches.find(ref);
        if (wanted == caches.end()) continue;
        const ChartRefData &data = wanted->second;
        // what the cells hold, as a cache holds it
        std::map<qint64, QString> points;
        for (qint64 i = 0; i < data.points.size(); ++i) {
            const ChartPoint &c = data.points[int(i)];
            if (c.empty) continue;
            if (numeric) {
                if (c.isNumber) points[i] = numberText(c.number);
            } else if (!c.text.isEmpty()) {
                points[i] = c.text;
            }
        }
        const bool haveCache = r.cache0 >= 0 && r.cache1 > r.cache0;
        CacheContent old;
        if (haveCache) {
            old = readCache(xml, r.cache0, r.cache1);
            if (old.count == qint64(data.points.size()) && old.points.size() == points.size()) {
                bool same = true;
                for (const auto &[i, text] : points) {
                    const auto it = old.points.find(i);
                    if (it == old.points.end() || (numeric ? !sameNumberText(it->second, text) : it->second != text)) {
                        same = false;
                        break;
                    }
                }
                if (same) continue;
            }
        }
        // the cache made again, with the prefix the reference is written with
        const QByteArrayView prefix = xmlscan::prefixOf(t.name);
        const QByteArray pre(prefix.data(), prefix.size());
        const QByteArray name = pre + (numeric ? "numCache" : "strCache");
        QByteArray fresh = "<" + name + ">";
        if (numeric) fresh += "<" + pre + "formatCode>" + xmlscan::escapeText(old.format.isEmpty() ? QStringLiteral("General") : old.format) + "</" + pre + "formatCode>";
        fresh += "<" + pre + "ptCount val=\"" + QByteArray::number(qint64(data.points.size())) + "\"/>";
        for (const auto &[i, text] : points) fresh += "<" + pre + "pt idx=\"" + QByteArray::number(i) + "\"><" + pre + "v>" + xmlscan::escapeText(text) + "</" + pre + "v></" + pre + "pt>";
        fresh += "</" + name + ">";
        if (haveCache) {
            result += xml.mid(copied, r.cache0 - copied) + fresh;
            copied = r.cache1;
        } else {
            result += xml.mid(copied, r.fEnd - copied) + fresh;
            copied = r.fEnd;
        }
        any = true;
    }
    if (!any) return true;
    result += xml.mid(copied);
    *out = result;
    *changed = *out != xml;
    return true;
}

} // namespace jp::office::sheet
