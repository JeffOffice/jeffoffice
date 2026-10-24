#include "sheet/drawingsave.h"

#include "sheet/xmlscan.h"

#include <QLocale>

#include <algorithm>
#include <cmath>
#include <set>

namespace jp::office::sheet {

using xmlscan::Tag;

bool removeDrawingObjects(const QByteArray &xml, const std::vector<int> &ordinals, QByteArray *out, bool *changed)
{
    *out = xml;
    *changed = false;
    if (ordinals.empty()) return true;
    const std::set<int> gone(ordinals.begin(), ordinals.end());
    Tag t;
    qsizetype pos = 0;
    // the root
    bool haveRoot = false;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        haveRoot = true;
        break;
    }
    if (!haveRoot || t.selfClosing) return haveRoot;
    int depth = 1, ordinal = -1;
    qsizetype childStart = -1;
    QByteArray result;
    qsizetype copied = 0;
    auto finishChild = [&](qsizetype end) {
        if (gone.count(ordinal)) {
            result += xml.mid(copied, childStart - copied);
            copied = end;
        }
    };
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (depth == 1 && !t.closing) {
            ++ordinal;
            childStart = t.start;
            if (t.selfClosing) {
                finishChild(t.end);
                continue;
            }
            depth = 2;
            continue;
        }
        if (t.closing) {
            if (--depth == 1) finishChild(t.end);
            if (depth == 0) break;
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    if (copied == 0 && result.isEmpty()) return true;
    result += xml.mid(copied);
    *out = result;
    *changed = *out != xml;
    return true;
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
