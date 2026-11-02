// A check of the parts of a pivot table as they are written: that the original program, which is strict about them, finds them as the schema has them. (It does not read the cells of the sheet.)
#include "sheet/pivot.h"

#include "sheet/cellref.h"

#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>
#include <QSet>
#include <QXmlStreamReader>

#include <cmath>

namespace jp::office::sheet {

namespace {

QDomElement child(const QDomElement &p, const char *name)
{
    for (QDomNode n = p.firstChild(); !n.isNull(); n = n.nextSibling())
        if (n.isElement() && n.toElement().tagName() == QLatin1String(name)) return n.toElement();
    return QDomElement();
}

int childCount(const QDomElement &p)
{
    int n = 0;
    for (QDomNode c = p.firstChild(); !c.isNull(); c = c.nextSibling()) n += c.isElement();
    return n;
}

bool boolAttr(const QDomElement &e, const char *name, bool def)
{
    if (!e.hasAttribute(QLatin1String(name))) return def;
    const QString v = e.attribute(QLatin1String(name));
    return v == QLatin1String("1") || v == QLatin1String("true");
}

} // namespace

bool pivotCheckShape(const QByteArray &tableXml, const QByteArray &defXml, const QByteArray &recordsXml, QString *why)
{
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    if (pivotXmlDepth(tableXml, kPivotMaxXmlDepth) > kPivotMaxXmlDepth || pivotXmlDepth(defXml, kPivotMaxXmlDepth) > kPivotMaxXmlDepth) return fail(QStringLiteral("nested too deep"));
    QDomDocument tdoc, ddoc;
    if (!tdoc.setContent(tableXml)) return fail(QStringLiteral("the table is not well formed"));
    if (!ddoc.setContent(defXml)) return fail(QStringLiteral("the cache definition is not well formed"));
    const QDomElement troot = tdoc.documentElement(), droot = ddoc.documentElement();

    // ---- the cache
    {
        static const char *const order[] = {"cacheSource", "cacheFields", "cacheHierarchies", "kpis", "tupleCache", "calculatedItems", "calculatedMembers", "dimensions", "measureGroups", "maps", "extLst"};
        int last = -1;
        for (QDomNode n = droot.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (!n.isElement()) continue;
            int at = -1;
            for (int i = 0; i < 11; ++i)
                if (n.toElement().tagName() == QLatin1String(order[i])) at = i;
            if (at < 0) return fail(QStringLiteral("cache definition: an element %1 that the schema does not have").arg(n.toElement().tagName()));
            if (at < last) return fail(QStringLiteral("cache definition: %1 is out of order").arg(n.toElement().tagName()));
            last = at;
        }
    }
    const QDomElement cacheFields = child(droot, "cacheFields");
    if (cacheFields.isNull()) return fail(QStringLiteral("cache definition: no cacheFields"));
    const int nf = childCount(cacheFields);
    if (nf < 1) return fail(QStringLiteral("cache definition: no fields"));
    if (cacheFields.hasAttribute(QStringLiteral("count")) && cacheFields.attribute(QStringLiteral("count")).toInt() != nf) return fail(QStringLiteral("cache definition: cacheFields count is not the number of fields"));
    static const QRegularExpression dateTime(QStringLiteral("^\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d(\\.\\d+)?$"));
    struct Kinds {
        bool hasN = false, hasS = false, hasM = false, hasD = false, hasB = false, hasE = false, integers = true;
        double lo = 0, hi = 0;
        QString dlo, dhi;
        void number(double v)
        {
            if (!hasN) lo = hi = v;
            lo = std::min(lo, v);
            hi = std::max(hi, v);
            if (v != std::floor(v)) integers = false;
            hasN = true;
        }
        void date(const QString &v)
        {
            if (!hasD || v < dlo) dlo = v;
            if (!hasD || v > dhi) dhi = v;
            hasD = true;
        }
    };
    // (the marks of a field as the original program writes them, and the range of its numbers and dates: what the items, or the values in the records, are)
    auto agree = [&](const QDomElement &si, const Kinds &k, const QString &name) -> QString {
        const bool text = k.hasS || k.hasB || k.hasE;
        if (boolAttr(si, "containsNumber", false) != (k.hasN && !k.hasD)) return QStringLiteral("field %1: containsNumber does not agree with the items").arg(name);
        if (boolAttr(si, "containsBlank", false) != k.hasM) return QStringLiteral("field %1: containsBlank does not agree with the items").arg(name);
        if (boolAttr(si, "containsDate", false) != k.hasD) return QStringLiteral("field %1: containsDate does not agree with the items").arg(name);
        if (boolAttr(si, "containsString", true) != text) return QStringLiteral("field %1: containsString does not agree with the items").arg(name);
        if (boolAttr(si, "containsMixedTypes", false) != ((text && (k.hasN || k.hasD)) || (k.hasD && k.hasN))) return QStringLiteral("field %1: containsMixedTypes does not agree with the items").arg(name);
        if (boolAttr(si, "containsSemiMixedTypes", true) != (text || k.hasM)) return QStringLiteral("field %1: containsSemiMixedTypes does not agree with the items").arg(name);
        if (boolAttr(si, "containsNonDate", true) != !(k.hasD && !k.hasN && !text && !k.hasM)) return QStringLiteral("field %1: containsNonDate does not agree with the items").arg(name);
        if (k.hasN && !k.hasD && boolAttr(si, "containsInteger", false) != k.integers) return QStringLiteral("field %1: containsInteger does not agree with the items").arg(name);
        if (k.hasN && !k.hasD) {
            if (!si.hasAttribute(QStringLiteral("minValue")) || !si.hasAttribute(QStringLiteral("maxValue"))) return QStringLiteral("field %1: numbers and no range of numbers").arg(name);
            if (si.attribute(QStringLiteral("minValue")).toDouble() > k.lo + 1e-9 * std::max(1.0, std::abs(k.lo)) || si.attribute(QStringLiteral("maxValue")).toDouble() < k.hi - 1e-9 * std::max(1.0, std::abs(k.hi)))
                return QStringLiteral("field %1: a number outside minValue and maxValue").arg(name);
        }
        if (k.hasD) {
            if (!si.hasAttribute(QStringLiteral("minDate")) || !si.hasAttribute(QStringLiteral("maxDate"))) return QStringLiteral("field %1: dates and no range of dates").arg(name);
            if (!k.hasN && (si.attribute(QStringLiteral("minDate")) > k.dlo || si.attribute(QStringLiteral("maxDate")) < k.dhi)) return QStringLiteral("field %1: a date outside minDate and maxDate").arg(name);
        }
        return QString();
    };
    QVector<QDomElement> marks(nf);
    QVector<QString> names(nf);
    QVector<Kinds> inlined(nf);
    QVector<qint64> inlineCount(nf, 0);
    QVector<int> shared(nf, 0);
    int fi = 0;
    for (QDomNode n = cacheFields.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement()) continue;
        const QDomElement cf = n.toElement();
        const QString name = cf.attribute(QStringLiteral("name"));
        names[fi] = name;
        const QDomElement si = child(cf, "sharedItems");
        if (!si.isNull()) {
            const int items = childCount(si);
            shared[fi] = items;
            if (si.hasAttribute(QStringLiteral("count")) && si.attribute(QStringLiteral("count")).toInt() != items) return fail(QStringLiteral("field %1: sharedItems count is not the number of items").arg(name));
            if (items > 0) {
                Kinds k;
                QSet<QString> seenText;
                for (QDomNode c = si.firstChild(); !c.isNull(); c = c.nextSibling()) {
                    if (!c.isElement()) continue;
                    const QDomElement e = c.toElement();
                    const QString t = e.tagName();
                    if (t == QLatin1String("n")) {
                        k.number(e.attribute(QStringLiteral("v")).toDouble());
                    } else if (t == QLatin1String("s")) {
                        k.hasS = true;
                        // (two texts that differ only in case are one item to the original program)
                        const QString folded = e.attribute(QStringLiteral("v")).toCaseFolded();
                        if (seenText.contains(folded)) return fail(QStringLiteral("field %1: two shared items that differ only in case").arg(name));
                        seenText.insert(folded);
                    } else if (t == QLatin1String("m")) {
                        k.hasM = true;
                    } else if (t == QLatin1String("b")) {
                        k.hasB = true;
                    } else if (t == QLatin1String("e")) {
                        k.hasE = true;
                    } else if (t == QLatin1String("d")) {
                        const QString v = e.attribute(QStringLiteral("v"));
                        if (!dateTime.match(v).hasMatch()) return fail(QStringLiteral("field %1: a date that is not a date and time: %2").arg(name, v));
                        k.date(v);
                    } else {
                        return fail(QStringLiteral("field %1: a shared item %2 that is not one").arg(name, t));
                    }
                }
                const QString bad = agree(si, k, name);
                if (!bad.isEmpty()) return fail(bad);
            }
        }
        marks[fi] = si;
        ++fi;
    }

    // ---- the table
    {
        static const char *const order[] = {"location", "pivotFields", "rowFields", "rowItems", "colFields", "colItems", "pageFields", "dataFields", "formats", "conditionalFormats",
                                            "chartFormats", "pivotHierarchies", "pivotTableStyleInfo", "filters", "rowHierarchiesUsage", "colHierarchiesUsage", "extLst"};
        int last = -1;
        for (QDomNode n = troot.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (!n.isElement()) continue;
            int at = -1;
            for (int i = 0; i < 17; ++i)
                if (n.toElement().tagName() == QLatin1String(order[i])) at = i;
            if (at < 0) return fail(QStringLiteral("table: an element %1 that the schema does not have").arg(n.toElement().tagName()));
            if (at < last) return fail(QStringLiteral("table: %1 is out of order").arg(n.toElement().tagName()));
            last = at;
        }
    }
    const QDomElement location = child(troot, "location");
    if (location.isNull()) return fail(QStringLiteral("table: no location"));
    CellRange where;
    if (!parseRangeRef(location.attribute(QStringLiteral("ref")), &where)) return fail(QStringLiteral("table: the location is not a range"));
    const qint64 rows = qint64(where.row2) - where.row1 + 1, cols = qint64(where.col2) - where.col1 + 1;
    const int fh = location.attribute(QStringLiteral("firstHeaderRow")).toInt(), fr = location.attribute(QStringLiteral("firstDataRow")).toInt(), fc = location.attribute(QStringLiteral("firstDataCol")).toInt();
    if (fh < 0 || fr < fh || fr > rows || fc < 0 || fc > cols) return fail(QStringLiteral("table: the first row of the header, first row of data, and first column of data do not fit the location"));
    for (const char *name : {"pivotFields", "rowFields", "rowItems", "colFields", "colItems", "pageFields", "dataFields", "formats", "conditionalFormats", "chartFormats", "filters"}) {
        const QDomElement e = child(troot, name);
        if (e.isNull()) continue;
        const int n = childCount(e);
        if (e.hasAttribute(QStringLiteral("count")) && e.attribute(QStringLiteral("count")).toInt() != n) return fail(QStringLiteral("table: the count of %1 is not the number of its children").arg(QLatin1String(name)));
        if (n < 1) return fail(QStringLiteral("table: %1 has no child").arg(QLatin1String(name)));
    }
    const QDomElement pivotFields = child(troot, "pivotFields");
    if (pivotFields.isNull()) return fail(QStringLiteral("table: no pivotFields"));
    if (childCount(pivotFields) != nf) return fail(QStringLiteral("table: as many pivotField elements as the cache has fields are needed"));
    QVector<int> itemCount(nf, 0);
    fi = 0;
    for (QDomNode n = pivotFields.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement()) continue;
        const QDomElement pf = n.toElement();
        const QDomElement items = child(pf, "items");
        if (!items.isNull()) {
            const int k = childCount(items);
            if (k < 1) return fail(QStringLiteral("table: a field with an empty list of items"));
            if (items.hasAttribute(QStringLiteral("count")) && items.attribute(QStringLiteral("count")).toInt() != k) return fail(QStringLiteral("table: the count of a field's items is not their number"));
            itemCount[fi] = k;
            for (QDomNode c = items.firstChild(); !c.isNull(); c = c.nextSibling()) {
                if (!c.isElement()) continue;
                const QDomElement it = c.toElement();
                if (it.hasAttribute(QStringLiteral("t"))) continue;
                if (!it.hasAttribute(QStringLiteral("x"))) return fail(QStringLiteral("table: an item with no place in the cache"));
                const int x = it.attribute(QStringLiteral("x")).toInt();
                if (x < 0 || x >= shared[fi]) return fail(QStringLiteral("table: an item names a shared item that is not there (%1)").arg(x));
            }
        }
        ++fi;
    }
    auto fieldsOf = [&](const char *name) {
        QVector<int> out;
        const QDomElement list = child(troot, name);
        for (QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling())
            if (n.isElement()) out.append(n.toElement().attribute(QStringLiteral("x")).toInt());
        return out;
    };
    const QVector<int> rowF = fieldsOf("rowFields"), colF = fieldsOf("colFields");
    int dataCount = 0;
    {
        const QDomElement list = child(troot, "dataFields");
        for (QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling())
            if (n.isElement()) {
                ++dataCount;
                const int f = n.toElement().attribute(QStringLiteral("fld")).toInt();
                if (f < 0 || f >= nf) return fail(QStringLiteral("table: a value names a field that is not there"));
            }
    }
    for (const QVector<int> &list : {rowF, colF})
        for (int f : list)
            if (f != -2 && (f < 0 || f >= nf)) return fail(QStringLiteral("table: an axis names a field that is not there"));
    {
        const QDomElement list = child(troot, "pageFields");
        for (QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (!n.isElement()) continue;
            const QDomElement p = n.toElement();
            const int f = p.attribute(QStringLiteral("fld")).toInt();
            if (f < 0 || f >= nf) return fail(QStringLiteral("table: a filter names a field that is not there"));
            if (p.hasAttribute(QStringLiteral("item")) && p.attribute(QStringLiteral("item")).toInt() >= itemCount[f]) return fail(QStringLiteral("table: a filter names an item that is not there"));
        }
    }
    // the formats, rules, and chart formats name items by their place in a field's list: a place that is not there is a part the original program cannot open
    for (const char *name : {"formats", "conditionalFormats", "chartFormats"}) {
        const QDomElement list = child(troot, name);
        if (list.isNull()) continue;
        std::vector<QDomElement> pending{list};
        while (!pending.empty()) {
            const QDomElement e = pending.back();
            pending.pop_back();
            for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (!n.isElement()) continue;
                const QDomElement c = n.toElement();
                if (c.tagName() == QLatin1String("reference")) {
                    const qint64 f = c.attribute(QStringLiteral("field")).toLongLong();
                    const int k = childCount(c);
                    if (c.hasAttribute(QStringLiteral("count")) && c.attribute(QStringLiteral("count")).toInt() != k) return fail(QStringLiteral("table: the count of a reference is not the number of its items"));
                    if (f != 4294967294LL) {
                        if (f < 0 || f >= nf) return fail(QStringLiteral("table: a format names a field that is not there"));
                        for (QDomNode x = c.firstChild(); !x.isNull(); x = x.nextSibling())
                            if (x.isElement() && x.toElement().attribute(QStringLiteral("v")).toInt() >= std::max(itemCount[f], 1))
                                return fail(QStringLiteral("table: a format names an item that is not there (field %1)").arg(f));
                    }
                } else {
                    pending.push_back(c);
                }
            }
        }
    }
    for (int axis = 0; axis < 2; ++axis) {
        const QDomElement lines = child(troot, axis == 0 ? "rowItems" : "colItems");
        const QVector<int> &fields = axis == 0 ? rowF : colF;
        for (QDomNode n = lines.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (!n.isElement()) continue;
            const QDomElement i = n.toElement();
            if (i.attribute(QStringLiteral("i")).toInt() >= std::max(dataCount, 1)) return fail(QStringLiteral("table: a line names a value that is not there"));
            const QString t = i.attribute(QStringLiteral("t"));
            int level = i.attribute(QStringLiteral("r")).toInt();
            for (QDomNode c = i.firstChild(); !c.isNull(); c = c.nextSibling()) {
                if (!c.isElement()) continue;
                const int v = c.toElement().attribute(QStringLiteral("v")).toInt();
                if (t == QLatin1String("grand")) continue;
                if (level >= fields.size()) return fail(QStringLiteral("table: a line has more items than the axis has fields"));
                const int f = fields[level++];
                const int limit = f == -2 ? std::max(dataCount, 1) : itemCount[f] > 0 ? itemCount[f] : shared[f];
                if (v < 0 || v >= limit) return fail(QStringLiteral("table: a line names an item that is not there (%1 of %2)").arg(v).arg(limit));
            }
        }
    }

    // ---- the records
    if (!recordsXml.isEmpty()) {
        QXmlStreamReader r(recordsXml);
        int depth = 0, field = 0;
        qint64 count = 0, declared = -1;
        while (!r.atEnd()) {
            const auto tok = r.readNext();
            if (tok == QXmlStreamReader::StartElement) {
                ++depth;
                if (depth == 1) {
                    declared = r.attributes().hasAttribute(QLatin1String("count")) ? r.attributes().value(QLatin1String("count")).toLongLong() : -1;
                } else if (depth == 2) {
                    ++count;
                    field = 0;
                } else if (depth == 3) {
                    if (field >= nf) return fail(QStringLiteral("records: a record with more values than the cache has fields"));
                    if (r.name() == QLatin1String("x")) {
                        const int x = r.attributes().value(QLatin1String("v")).toInt();
                        if (x < 0 || x >= shared[field]) return fail(QStringLiteral("records: a record names a shared item that is not there"));
                    } else {
                        Kinds &k = inlined[field];
                        ++inlineCount[field];
                        const QStringView t = r.name();
                        if (t == QLatin1String("n")) k.number(r.attributes().value(QLatin1String("v")).toDouble());
                        else if (t == QLatin1String("s")) k.hasS = true;
                        else if (t == QLatin1String("m")) k.hasM = true;
                        else if (t == QLatin1String("b")) k.hasB = true;
                        else if (t == QLatin1String("e")) k.hasE = true;
                        else if (t == QLatin1String("d")) k.date(r.attributes().value(QLatin1String("v")).toString());
                    }
                    ++field;
                }
            } else if (tok == QXmlStreamReader::EndElement) {
                if (depth == 2 && field != nf) return fail(QStringLiteral("records: a record with fewer values than the cache has fields"));
                --depth;
            }
        }
        if (r.hasError()) return fail(QStringLiteral("records: not well formed"));
        // a field with no shared items is what its records hold
        for (int f = 0; f < nf; ++f)
            if (shared[f] == 0 && inlineCount[f] > 0 && !marks[f].isNull()) {
                const QString bad = agree(marks[f], inlined[f], names[f]);
                if (!bad.isEmpty()) return fail(bad);
            }
        if (declared >= 0 && declared != count) return fail(QStringLiteral("records: the count is not the number of records"));
        if (droot.hasAttribute(QStringLiteral("recordCount")) && droot.attribute(QStringLiteral("recordCount")).toLongLong() != count) return fail(QStringLiteral("cache definition: recordCount is not the number of records"));
    }
    return true;
}

} // namespace jp::office::sheet
