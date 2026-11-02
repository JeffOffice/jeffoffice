#include "sheet/pivot.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

QString num(double d)
{
    if (d == std::floor(d) && std::abs(d) < 1e15) return QString::number(qint64(d));
    return QString::number(d, 'g', 17);
}

QString isoDate(double serial, jp::calc::DateSystem dates)
{
    int y = 1899, m = 12, d = 31;
    if (!(serial < 1 && dates == jp::calc::DateSystem::Date1900) && !jp::calc::serialToDate(serial, dates, &y, &m, &d)) return QStringLiteral("1900-01-01T00:00:00");
    double frac = serial - std::floor(serial);
    qint64 secs = qint64(std::llround(frac * 86400.0));
    if (secs >= 86400) secs = 86399;
    return QStringLiteral("%1-%2-%3T%4:%5:%6")
        .arg(y, 4, 10, QLatin1Char('0'))
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(d, 2, 10, QLatin1Char('0'))
        .arg(secs / 3600, 2, 10, QLatin1Char('0'))
        .arg(secs / 60 % 60, 2, 10, QLatin1Char('0'))
        .arg(secs % 60, 2, 10, QLatin1Char('0'));
}

QString esc(const QString &s)
{
    QString out;
    out.reserve(s.size() + 8);
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u == '&') out += QLatin1String("&amp;");
        else if (u == '<') out += QLatin1String("&lt;");
        else if (u == '>') out += QLatin1String("&gt;");
        else if (u == '"') out += QLatin1String("&quot;");
        else if (u < 0x20 && u != 9 && u != 10 && u != 13) out += QStringLiteral("_x%1_").arg(u, 4, 16, QLatin1Char('0')).toUpper().replace(QLatin1String("_X"), QLatin1String("_x"));
        else if (u == 10) out += QLatin1String("&#10;");
        else if (u == 13) out += QLatin1String("&#13;");
        else if (u == 9) out += QLatin1String("&#9;");
        else out += c;
    }
    return out;
}

QString valueTag(const PivotValue &v, jp::calc::DateSystem dates)
{
    switch (v.kind) {
    case PivotValue::Missing: return QStringLiteral("<m/>");
    case PivotValue::Number: return QStringLiteral("<n v=\"%1\"/>").arg(num(v.number));
    case PivotValue::Date: return QStringLiteral("<d v=\"%1\"/>").arg(isoDate(v.number, dates));
    case PivotValue::Bool: return QStringLiteral("<b v=\"%1\"/>").arg(v.number != 0 ? 1 : 0);
    case PivotValue::Text: return QStringLiteral("<s v=\"%1\"/>").arg(esc(v.text));
    case PivotValue::Error: return QStringLiteral("<e v=\"%1\"/>").arg(esc(v.text));
    }
    return QString();
}

// The attributes of <sharedItems> that say what a field holds.
void sharedAttributes(const PivotCacheField &f, QVector<std::pair<QString, QString>> *attrs)
{
    bool text = false, number = false, blank = false, date = false, nonDate = false, bools = false, errors = false, integer = true;
    double lo = 0, hi = 0, dlo = 0, dhi = 0;
    bool firstNum = true, firstDate = true;
    const int n = f.hasShared ? f.sharedCount : int(f.items.size());
    for (int i = 0; i < n; ++i) {
        const PivotValue &v = f.items[i];
        switch (v.kind) {
        case PivotValue::Text: text = true; nonDate = true; break;
        case PivotValue::Missing: blank = true; break;
        case PivotValue::Bool: bools = true; nonDate = true; break;
        case PivotValue::Error: errors = true; nonDate = true; break;
        case PivotValue::Number:
            number = true;
            nonDate = true;
            if (v.number != std::floor(v.number)) integer = false;
            if (firstNum) lo = hi = v.number;
            else {
                lo = std::min(lo, v.number);
                hi = std::max(hi, v.number);
            }
            firstNum = false;
            break;
        case PivotValue::Date:
            date = true;
            if (firstDate) dlo = dhi = v.number;
            else {
                dlo = std::min(dlo, v.number);
                dhi = std::max(dhi, v.number);
            }
            firstDate = false;
            break;
        }
    }
    const bool mixed = (text || bools || errors) && (number || date);
    const bool semi = text || bools || errors || mixed;
    if (!semi && !blank) attrs->append({QStringLiteral("containsSemiMixedTypes"), QStringLiteral("0")});
    if (date && !nonDate && !text) attrs->append({QStringLiteral("containsNonDate"), QStringLiteral("0")});
    if (date) attrs->append({QStringLiteral("containsDate"), QStringLiteral("1")});
    if (!text && !bools && !errors) attrs->append({QStringLiteral("containsString"), QStringLiteral("0")});
    if (blank) attrs->append({QStringLiteral("containsBlank"), QStringLiteral("1")});
    if (mixed) attrs->append({QStringLiteral("containsMixedTypes"), QStringLiteral("1")});
    if (number) {
        attrs->append({QStringLiteral("containsNumber"), QStringLiteral("1")});
        if (integer) attrs->append({QStringLiteral("containsInteger"), QStringLiteral("1")});
        attrs->append({QStringLiteral("minValue"), num(lo)});
        attrs->append({QStringLiteral("maxValue"), num(hi)});
    }
    if (date) {
        attrs->append({QStringLiteral("minDate"), QStringLiteral("%1").arg(dlo)});
        attrs->append({QStringLiteral("maxDate"), QStringLiteral("%1").arg(dhi)});
    }
}

QDomElement childNamed(const QDomElement &parent, const QString &name)
{
    for (QDomNode n = parent.firstChild(); !n.isNull(); n = n.nextSibling())
        if (n.isElement() && n.toElement().tagName() == name) return n.toElement();
    return QDomElement();
}

void setBool(QDomElement &e, const QString &name, bool value, bool def)
{
    if (value == def) e.removeAttribute(name);
    else e.setAttribute(name, value ? QStringLiteral("1") : QStringLiteral("0"));
}

} // namespace

QByteArray writePivotCacheDefinition(const QByteArray &original, const PivotCache &cache, qint64 recordCount, double refreshedDate, jp::calc::DateSystem dates)
{
    QDomDocument doc;
    if (!doc.setContent(original)) return QByteArray();
    QDomElement root = doc.documentElement();
    root.setAttribute(QStringLiteral("recordCount"), QString::number(recordCount));
    root.setAttribute(QStringLiteral("refreshedDate"), QString::number(refreshedDate, 'g', 17));
    QDomElement source = childNamed(root, QStringLiteral("cacheSource"));
    QDomElement ws = source.isNull() ? QDomElement() : childNamed(source, QStringLiteral("worksheetSource"));
    if (!ws.isNull()) {
        if (!cache.name.isEmpty()) {
            ws.setAttribute(QStringLiteral("name"), cache.name);
            ws.removeAttribute(QStringLiteral("ref"));
            ws.removeAttribute(QStringLiteral("sheet"));
        } else {
            ws.removeAttribute(QStringLiteral("name"));
            ws.setAttribute(QStringLiteral("ref"), cache.ref);
            ws.setAttribute(QStringLiteral("sheet"), cache.sheet);
        }
    }
    // the fields
    QDomElement fields = childNamed(root, QStringLiteral("cacheFields"));
    QDomElement made = doc.createElement(QStringLiteral("cacheFields"));
    made.setAttribute(QStringLiteral("count"), QString::number(cache.fields.size()));
    // (the fields the original had, by name: what the program has no model of — a caption, say — is kept)
    QHash<QString, QDomElement> was;
    for (QDomNode n = fields.firstChild(); !n.isNull(); n = n.nextSibling())
        if (n.isElement()) was.insert(n.toElement().attribute(QStringLiteral("name")), n.toElement());
    for (const PivotCacheField &f : cache.fields) {
        QDomElement e = doc.createElement(QStringLiteral("cacheField"));
        e.setAttribute(QStringLiteral("name"), f.name);
        if (was.contains(f.name)) {
            const QDomElement w = was.value(f.name);
            const QDomNamedNodeMap attrs = w.attributes();
            for (int i = 0; i < attrs.size(); ++i) {
                const QDomAttr a = attrs.item(i).toAttr();
                if (a.name() != QLatin1String("name") && a.name() != QLatin1String("numFmtId")) e.setAttribute(a.name(), a.value());
            }
        }
        e.setAttribute(QStringLiteral("numFmtId"), QString::number(f.numFmtId));
        QDomElement shared = doc.createElement(QStringLiteral("sharedItems"));
        QVector<std::pair<QString, QString>> attrs;
        sharedAttributes(f, &attrs);
        for (const auto &a : attrs) shared.setAttribute(a.first, a.second);
        if (f.hasShared) {
            shared.setAttribute(QStringLiteral("count"), QString::number(f.sharedCount));
            for (int i = 0; i < f.sharedCount; ++i) {
                const PivotValue &v = f.items[i];
                static const char *tags[] = {"m", "n", "s", "b", "e", "d"};
                QDomElement ve = doc.createElement(QLatin1String(tags[v.kind == PivotValue::Missing ? 0 : v.kind == PivotValue::Number ? 1 : v.kind == PivotValue::Text ? 2 : v.kind == PivotValue::Bool ? 3 : v.kind == PivotValue::Error ? 4 : 5]));
                switch (v.kind) {
                case PivotValue::Missing: break;
                case PivotValue::Number: ve.setAttribute(QStringLiteral("v"), num(v.number)); break;
                case PivotValue::Date: ve.setAttribute(QStringLiteral("v"), isoDate(v.number, dates)); break;
                case PivotValue::Bool: ve.setAttribute(QStringLiteral("v"), v.number != 0 ? QStringLiteral("1") : QStringLiteral("0")); break;
                case PivotValue::Text:
                case PivotValue::Error: ve.setAttribute(QStringLiteral("v"), v.text); break;
                }
                shared.appendChild(ve);
            }
        }
        e.appendChild(shared);
        made.appendChild(e);
    }
    if (!fields.isNull()) root.replaceChild(made, fields);
    else root.insertAfter(made, source);
    return doc.toByteArray(-1);
}

QByteArray writePivotCacheRecords(const PivotCache &cache, jp::calc::DateSystem dates)
{
    const int nf = int(cache.fields.size());
    const qint64 n = nf > 0 ? cache.fields[0].records.size() : 0;
    QByteArray out;
    out.reserve(qsizetype(std::min<qint64>(n * nf * 14 + 400, qint64(1) << 30)));
    out += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheRecords xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" count=\"";
    out += QByteArray::number(n);
    out += "\">";
    // (a value that is not shared is written whole, once for each distinct value: its text is made once)
    std::vector<QVector<QByteArray>> inlineTags;
    inlineTags.resize(size_t(nf));
    for (int f = 0; f < nf; ++f)
        if (!cache.fields[f].hasShared) {
            inlineTags[size_t(f)].resize(cache.fields[f].items.size());
            for (qsizetype i = 0; i < cache.fields[f].items.size(); ++i) inlineTags[size_t(f)][i] = valueTag(cache.fields[f].items[i], dates).toUtf8();
        }
    for (qint64 r = 0; r < n; ++r) {
        out += "<r>";
        for (int f = 0; f < nf; ++f) {
            const PivotCacheField &pf = cache.fields[f];
            const qint32 idx = pf.records[r];
            if (pf.hasShared && idx < pf.sharedCount) {
                out += "<x v=\"";
                out += QByteArray::number(idx);
                out += "\"/>";
            } else if (pf.hasShared) {
                out += valueTag(pf.items[idx], dates).toUtf8();
            } else {
                out += inlineTags[size_t(f)][idx];
            }
        }
        out += "</r>";
    }
    out += "</pivotCacheRecords>";
    return out;
}

QByteArray writePivotTable(const QByteArray &original, const PivotTable &t, bool structureChanged)
{
    QDomDocument doc;
    if (!doc.setContent(original)) return QByteArray();
    QDomElement root = doc.documentElement();
    setBool(root, QStringLiteral("dataOnRows"), t.dataOnRows, false);
    setBool(root, QStringLiteral("rowGrandTotals"), t.rowGrandTotals, true);
    setBool(root, QStringLiteral("colGrandTotals"), t.colGrandTotals, true);
    QDomElement loc = childNamed(root, QStringLiteral("location"));
    if (loc.isNull()) {
        loc = doc.createElement(QStringLiteral("location"));
        root.insertBefore(loc, root.firstChild());
    }
    loc.setAttribute(QStringLiteral("ref"), t.ref);
    loc.setAttribute(QStringLiteral("firstHeaderRow"), QString::number(t.firstHeaderRow));
    loc.setAttribute(QStringLiteral("firstDataRow"), QString::number(t.firstDataRow));
    loc.setAttribute(QStringLiteral("firstDataCol"), QString::number(t.firstDataCol));
    if (t.rowPageCount > 0) loc.setAttribute(QStringLiteral("rowPageCount"), QString::number(t.rowPageCount));
    else loc.removeAttribute(QStringLiteral("rowPageCount"));
    if (t.colPageCount > 0) loc.setAttribute(QStringLiteral("colPageCount"), QString::number(t.colPageCount));
    else loc.removeAttribute(QStringLiteral("colPageCount"));

    // the fields
    QDomElement fields = childNamed(root, QStringLiteral("pivotFields"));
    {
        // (as many as the table has: fields the source gained or lost)
        int have = 0;
        QList<QDomElement> extra;
        for (QDomNode n = fields.firstChild(); !n.isNull(); n = n.nextSibling())
            if (n.isElement() && n.toElement().tagName() == QLatin1String("pivotField") && ++have > t.fields.size()) extra.append(n.toElement());
        for (const QDomElement &e : extra) fields.removeChild(e);
        for (; have < t.fields.size(); ++have) {
            QDomElement e = doc.createElement(QStringLiteral("pivotField"));
            e.setAttribute(QStringLiteral("showAll"), QStringLiteral("0"));
            fields.appendChild(e);
        }
    }
    int fi = 0;
    for (QDomNode n = fields.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement()) continue;
        QDomElement e = n.toElement();
        if (e.tagName() != QLatin1String("pivotField")) continue;
        if (fi >= t.fields.size()) break;
        const PivotField &pf = t.fields[fi++];
        if (pf.axis.isEmpty()) e.removeAttribute(QStringLiteral("axis"));
        else e.setAttribute(QStringLiteral("axis"), pf.axis);
        setBool(e, QStringLiteral("dataField"), pf.dataField, false);
        if (pf.name.isEmpty()) e.removeAttribute(QStringLiteral("name"));
        else e.setAttribute(QStringLiteral("name"), pf.name);
        // the subtotal marks follow the items that say them
        static const std::pair<const char *, const char *> subs[] = {
            {"sumSubtotal", "sum"}, {"countASubtotal", "countA"}, {"avgSubtotal", "avg"}, {"maxSubtotal", "max"}, {"minSubtotal", "min"},
            {"productSubtotal", "product"}, {"countSubtotal", "count"}, {"stdDevSubtotal", "stdDev"}, {"stdDevPSubtotal", "stdDevP"},
            {"varSubtotal", "var"}, {"varPSubtotal", "varP"}};
        QStringList totals;
        for (const PivotTableItem &it : pf.items)
            if (!it.t.isEmpty()) totals << it.t;
        setBool(e, QStringLiteral("defaultSubtotal"), pf.hasItems ? totals.contains(QStringLiteral("default")) : pf.defaultSubtotal, true);
        for (const auto &s : subs) setBool(e, QLatin1String(s.first), pf.hasItems ? totals.contains(QLatin1String(s.second)) : pf.subtotals.contains(QLatin1String(s.second)), false);
        setBool(e, QStringLiteral("compact"), pf.compact, true);
        setBool(e, QStringLiteral("outline"), pf.outline, true);
        setBool(e, QStringLiteral("subtotalTop"), pf.subtotalTop, true);
        setBool(e, QStringLiteral("insertBlankRow"), pf.insertBlankRow, false);
        if (pf.sortType == QLatin1String("manual")) e.removeAttribute(QStringLiteral("sortType"));
        else e.setAttribute(QStringLiteral("sortType"), pf.sortType);
        if (pf.numFmtId > 0) e.setAttribute(QStringLiteral("numFmtId"), QString::number(pf.numFmtId));
        else e.removeAttribute(QStringLiteral("numFmtId"));
        QDomElement items = childNamed(e, QStringLiteral("items"));
        if (!items.isNull()) e.removeChild(items);
        if (pf.hasItems) {
            QDomElement made = doc.createElement(QStringLiteral("items"));
            made.setAttribute(QStringLiteral("count"), QString::number(pf.items.size()));
            for (const PivotTableItem &it : pf.items) {
                QDomElement ie = doc.createElement(QStringLiteral("item"));
                if (it.t.isEmpty()) {
                    if (it.hidden) ie.setAttribute(QStringLiteral("h"), QStringLiteral("1"));
                    if (it.missing) ie.setAttribute(QStringLiteral("m"), QStringLiteral("1"));
                    if (!it.showDetails) ie.setAttribute(QStringLiteral("sd"), QStringLiteral("0"));
                    if (!it.n.isEmpty()) ie.setAttribute(QStringLiteral("n"), it.n);
                    ie.setAttribute(QStringLiteral("x"), QString::number(it.x));
                } else {
                    ie.setAttribute(QStringLiteral("t"), it.t);
                }
                made.appendChild(ie);
            }
            // (the items come first among the field's parts)
            e.insertBefore(made, e.firstChild());
        }
    }
    fields.setAttribute(QStringLiteral("count"), QString::number(t.fields.size()));

    // the lists, in the order the schema has them
    auto drop = [&](const char *name) {
        QDomElement e = childNamed(root, QLatin1String(name));
        if (!e.isNull()) root.removeChild(e);
    };
    auto fieldList = [&](const char *name, const QVector<int> &list) {
        drop(name);
        if (list.isEmpty()) return QDomElement();
        QDomElement e = doc.createElement(QLatin1String(name));
        e.setAttribute(QStringLiteral("count"), QString::number(list.size()));
        for (int f : list) {
            QDomElement x = doc.createElement(QStringLiteral("field"));
            x.setAttribute(QStringLiteral("x"), QString::number(f));
            e.appendChild(x);
        }
        return e;
    };
    auto lines = [&](const char *name, const QVector<PivotAxisItem> &items) {
        drop(name);
        QDomElement e = doc.createElement(QLatin1String(name));
        e.setAttribute(QStringLiteral("count"), QString::number(items.size()));
        for (const PivotAxisItem &it : items) {
            QDomElement i = doc.createElement(QStringLiteral("i"));
            if (!it.t.isEmpty()) i.setAttribute(QStringLiteral("t"), it.t);
            if (it.r > 0) i.setAttribute(QStringLiteral("r"), QString::number(it.r));
            if (it.i > 0) i.setAttribute(QStringLiteral("i"), QString::number(it.i));
            for (int x : it.x) {
                QDomElement xe = doc.createElement(QStringLiteral("x"));
                if (x != 0) xe.setAttribute(QStringLiteral("v"), QString::number(x));
                i.appendChild(xe);
            }
            e.appendChild(i);
        }
        return e;
    };
    // after pivotFields: rowFields rowItems colFields colItems pageFields dataFields
    QDomElement anchor = fields;
    auto place = [&](const QDomElement &e) {
        if (e.isNull()) return;
        root.insertAfter(e, anchor);
        anchor = e;
    };
    QDomElement rf = fieldList("rowFields", t.rowFields);
    QDomElement ri = lines("rowItems", t.rowItems);
    QDomElement cf = fieldList("colFields", t.colFields);
    QDomElement ci = lines("colItems", t.colItems);
    drop("pageFields");
    QDomElement pf;
    if (!t.pageFields.isEmpty()) {
        pf = doc.createElement(QStringLiteral("pageFields"));
        pf.setAttribute(QStringLiteral("count"), QString::number(t.pageFields.size()));
        for (const PivotPageField &p : t.pageFields) {
            QDomElement x = doc.createElement(QStringLiteral("pageField"));
            x.setAttribute(QStringLiteral("fld"), QString::number(p.fld));
            if (p.item >= 0) x.setAttribute(QStringLiteral("item"), QString::number(p.item));
            x.setAttribute(QStringLiteral("hier"), QStringLiteral("-1"));
            pf.appendChild(x);
        }
    }
    drop("dataFields");
    QDomElement df;
    if (!t.dataFields.isEmpty()) {
        df = doc.createElement(QStringLiteral("dataFields"));
        df.setAttribute(QStringLiteral("count"), QString::number(t.dataFields.size()));
        for (const PivotDataField &d : t.dataFields) {
            QDomElement x = doc.createElement(QStringLiteral("dataField"));
            x.setAttribute(QStringLiteral("name"), d.name);
            x.setAttribute(QStringLiteral("fld"), QString::number(d.fld));
            if (d.subtotal != QLatin1String("sum")) x.setAttribute(QStringLiteral("subtotal"), d.subtotal);
            if (d.showDataAs != QLatin1String("normal")) x.setAttribute(QStringLiteral("showDataAs"), d.showDataAs);
            x.setAttribute(QStringLiteral("baseField"), QStringLiteral("0"));
            x.setAttribute(QStringLiteral("baseItem"), QStringLiteral("0"));
            if (d.numFmtId > 0) x.setAttribute(QStringLiteral("numFmtId"), QString::number(d.numFmtId));
            df.appendChild(x);
        }
    }
    place(rf);
    place(ri);
    place(cf);
    place(ci);
    place(pf);
    place(df);
    if (structureChanged) {
        drop("formats");
        drop("conditionalFormats");
        drop("chartFormats");
        drop("pivotHierarchies");
        drop("filters");
        drop("rowHierarchiesUsage");
        drop("colHierarchiesUsage");
    }
    return doc.toByteArray(-1);
}

} // namespace jp::office::sheet

namespace jp::office::sheet {

void pivotApplyGrid(PivotTable *t, const PivotGrid &grid, quint32 row, quint32 col)
{
    t->firstHeaderRow = grid.firstHeaderRow;
    t->firstDataRow = grid.firstDataRow;
    t->firstDataCol = grid.firstDataCol;
    t->rowItems = grid.rowItems;
    t->colItems = grid.colItems;
    t->ref = pivotRangeRef(row, col, row + quint32(std::max(grid.rows, 1)) - 1, col + quint32(std::max(grid.cols, 1)) - 1);
}

} // namespace jp::office::sheet
