#include "sheet/pivot.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <functional>
#include <cmath>

namespace jp::office::sheet {

namespace {

QString num(double d)
{
    if (d == std::floor(d) && std::abs(d) < 1e15) return QString::number(qint64(d));
    return QString::number(d, 'g', 17);
}

// The date and time of a day number as the schema writes one; empty for a number the calendar has no date for (day 60 of the 1900 system, a negative number, a year past 9999).
QString isoDate(double serial, jp::calc::DateSystem dates)
{
    int y = 1899, m = 12, d = 31;
    if (!std::isfinite(serial) || serial < 0) return QString();
    if (dates == jp::calc::DateSystem::Date1900 && std::floor(serial) == 60) return QString();
    if (!(serial < 1 && dates == jp::calc::DateSystem::Date1900) && !jp::calc::serialToDate(serial, dates, &y, &m, &d)) return QString();
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
    auto hex = [](ushort u) { return QStringLiteral("_x%1_").arg(u, 4, 16, QLatin1Char('0')).toUpper().replace(QLatin1String("_X"), QLatin1String("_x")); };
    for (qsizetype i = 0; i < s.size(); ++i) {
        const QChar c = s[i];
        const ushort u = c.unicode();
        if (u == '&') out += QLatin1String("&amp;");
        else if (u == '<') out += QLatin1String("&lt;");
        else if (u == '>') out += QLatin1String("&gt;");
        else if (u == '"') out += QLatin1String("&quot;");
        else if (u < 0x20 && u != 9 && u != 10 && u != 13) out += hex(u);
        else if (u == 10) out += QLatin1String("&#10;");
        else if (u == 13) out += QLatin1String("&#13;");
        else if (u == 9) out += QLatin1String("&#9;");
        // (what XML 1.0 has no character for: U+FFFE, U+FFFF, and half of a pair of surrogates, written as the file's escape of a code unit)
        else if (u == 0xFFFE || u == 0xFFFF) out += hex(u);
        else if (c.isHighSurrogate()) {
            if (i + 1 < s.size() && s[i + 1].isLowSurrogate()) {
                out += c;
                out += s[++i];
            } else {
                out += hex(u);
            }
        } else if (c.isLowSurrogate()) out += hex(u);
        else out += c;
    }
    return out;
}

QString valueTag(const PivotValue &v, jp::calc::DateSystem dates)
{
    switch (v.kind) {
    case PivotValue::Missing: return QStringLiteral("<m/>");
    case PivotValue::Number: return QStringLiteral("<n v=\"%1\"/>").arg(num(v.number));
    case PivotValue::Date: {
        const QString iso = isoDate(v.number, dates);
        return iso.isEmpty() ? QStringLiteral("<n v=\"%1\"/>").arg(num(v.number)) : QStringLiteral("<d v=\"%1\"/>").arg(iso);
    }
    case PivotValue::Bool: return QStringLiteral("<b v=\"%1\"/>").arg(v.number != 0 ? 1 : 0);
    case PivotValue::Text: return QStringLiteral("<s v=\"%1\"/>").arg(esc(pivotEncodeXString(v.text)));
    case PivotValue::Error: return QStringLiteral("<e v=\"%1\"/>").arg(esc(pivotEncodeXString(v.text)));
    }
    return QString();
}

// The attributes of <sharedItems> that say what a field holds.
void sharedAttributes(const QVector<PivotValue> &items, bool hasShared, int sharedCount, jp::calc::DateSystem dates, QVector<std::pair<QString, QString>> *attrs)
{
    bool text = false, number = false, blank = false, date = false, nonDate = false, bools = false, errors = false, integer = true;
    double lo = 0, hi = 0, dlo = 0, dhi = 0;
    bool firstNum = true, firstDate = true;
    const int n = hasShared ? sharedCount : int(items.size());
    for (int i = 0; i < n; ++i) {
        const PivotValue &v = items[i];
        switch (v.kind) {
        case PivotValue::Text: text = true; nonDate = true; break;
        case PivotValue::Missing: blank = true; break;
        case PivotValue::Bool: bools = true; nonDate = true; break;
        case PivotValue::Error: errors = true; nonDate = true; break;
        case PivotValue::Date:
            if (isoDate(v.number, dates).isEmpty()) {
                // (a day that has no date is written, and counted, as a number)
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
            }
            date = true;
            if (firstDate) dlo = dhi = v.number;
            else {
                dlo = std::min(dlo, v.number);
                dhi = std::max(dhi, v.number);
            }
            firstDate = false;
            break;
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
        }
    }
    // (the marks as the original program writes them: dates among numbers are "mixed" and have no containsNumber; a blank is not a date, so a field of dates and blanks is not "all dates")
    const bool anyText = text || bools || errors;
    const bool mixed = (anyText && (number || date)) || (date && number);
    if (!anyText && !blank) attrs->append({QStringLiteral("containsSemiMixedTypes"), QStringLiteral("0")});
    if (date && !number && !anyText && !blank) attrs->append({QStringLiteral("containsNonDate"), QStringLiteral("0")});
    if (date) attrs->append({QStringLiteral("containsDate"), QStringLiteral("1")});
    if (!anyText) attrs->append({QStringLiteral("containsString"), QStringLiteral("0")});
    if (blank) attrs->append({QStringLiteral("containsBlank"), QStringLiteral("1")});
    if (mixed) attrs->append({QStringLiteral("containsMixedTypes"), QStringLiteral("1")});
    if (number && !date) {
        attrs->append({QStringLiteral("containsNumber"), QStringLiteral("1")});
        if (integer) attrs->append({QStringLiteral("containsInteger"), QStringLiteral("1")});
        attrs->append({QStringLiteral("minValue"), num(lo)});
        attrs->append({QStringLiteral("maxValue"), num(hi)});
    }
    if (date) {
        // (a date the calendar has no day for would be written as text that is no date: it is left out)
        if (!isoDate(dlo, dates).isEmpty()) attrs->append({QStringLiteral("minDate"), isoDate(dlo, dates)});
        if (!isoDate(dhi, dates).isEmpty()) attrs->append({QStringLiteral("maxDate"), isoDate(dhi, dates)});
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
    // (A part nested deeper than a pivot table ever is, or of more elements than the document tree holds in a few gigabytes, is not parsed: the parser takes time that grows with the square of the depth,
    // and the tree takes about a kilobyte and a half for each element. The part the file has is kept.)
    if (pivotXmlDepth(original, kPivotMaxXmlDepth) > kPivotMaxXmlDepth || original.count('<') > kPivotMaxDomElements) return QByteArray();
    QDomDocument doc;
    if (!doc.setContent(original)) return QByteArray();
    QDomElement root = doc.documentElement();
    root.setAttribute(QStringLiteral("recordCount"), QString::number(recordCount));
    root.setAttribute(QStringLiteral("refreshedDate"), QString::number(refreshedDate, 'g', 17));
    setBool(root, QStringLiteral("refreshOnLoad"), cache.refreshOnLoad, false);
    QDomElement source = childNamed(root, QStringLiteral("cacheSource"));
    QDomElement ws = source.isNull() ? QDomElement() : childNamed(source, QStringLiteral("worksheetSource"));
    if (!ws.isNull()) {
        ws.removeAttribute(QStringLiteral("r:id"));   // (a source that was in another workbook is now where the cache says)
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
        if (n.isElement()) was.insert(pivotDecodeXString(n.toElement().attribute(QStringLiteral("name"))), n.toElement());
    for (const PivotCacheField &f : cache.fields) {
        QDomElement e = doc.createElement(QStringLiteral("cacheField"));
        e.setAttribute(QStringLiteral("name"), pivotEncodeXString(f.name));
        if (was.contains(f.name)) {
            const QDomElement w = was.value(f.name);
            const QDomNamedNodeMap attrs = w.attributes();
            for (int i = 0; i < attrs.size(); ++i) {
                const QDomAttr a = attrs.item(i).toAttr();
                if (a.name() != QLatin1String("name") && a.name() != QLatin1String("numFmtId")) e.setAttribute(a.name(), a.value());
            }
        }
        e.setAttribute(QStringLiteral("numFmtId"), QString::number(f.numFmtId));
        const bool derived = !f.group.databaseField && (f.group.kind != PivotFieldGroup::None || f.isCalculated());
        if (derived) e.setAttribute(QStringLiteral("databaseField"), QStringLiteral("0"));
        else e.removeAttribute(QStringLiteral("databaseField"));
        if (f.isCalculated()) e.setAttribute(QStringLiteral("formula"), f.formula);
        else e.removeAttribute(QStringLiteral("formula"));
        const QVector<PivotValue> &stored = f.storedItems();
        const bool storedShared = f.storedHasShared();
        const int storedCount = f.storedSharedCount();
        QDomElement shared = doc.createElement(QStringLiteral("sharedItems"));
        QVector<std::pair<QString, QString>> attrs;
        sharedAttributes(stored, storedShared, storedCount, dates, &attrs);
        for (const auto &a : attrs) shared.setAttribute(a.first, a.second);
        if (storedShared) {
            shared.setAttribute(QStringLiteral("count"), QString::number(storedCount));
            for (int i = 0; i < storedCount; ++i) {
                const PivotValue &v = stored[i];
                static const char *tags[] = {"m", "n", "s", "b", "e", "d"};
                const bool asNumber = v.kind == PivotValue::Date && isoDate(v.number, dates).isEmpty();
                QDomElement ve = doc.createElement(QLatin1String(tags[asNumber ? 1 : v.kind == PivotValue::Missing ? 0 : v.kind == PivotValue::Number ? 1 : v.kind == PivotValue::Text ? 2 : v.kind == PivotValue::Bool ? 3 : v.kind == PivotValue::Error ? 4 : 5]));
                switch (v.kind) {
                case PivotValue::Missing: break;
                case PivotValue::Number: ve.setAttribute(QStringLiteral("v"), num(v.number)); break;
                case PivotValue::Date: ve.setAttribute(QStringLiteral("v"), isoDate(v.number, dates).isEmpty() ? num(v.number) : isoDate(v.number, dates)); break;
                case PivotValue::Bool: ve.setAttribute(QStringLiteral("v"), v.number != 0 ? QStringLiteral("1") : QStringLiteral("0")); break;
                case PivotValue::Text:
                case PivotValue::Error: ve.setAttribute(QStringLiteral("v"), pivotEncodeXString(v.text)); break;
                }
                shared.appendChild(ve);
            }
        }
        if (!derived) e.appendChild(shared);
        if (f.group.kind != PivotFieldGroup::None) {
            const PivotFieldGroup &g = f.group;
            QDomElement fg = doc.createElement(QStringLiteral("fieldGroup"));
            if (g.par >= 0) fg.setAttribute(QStringLiteral("par"), QString::number(g.par));
            fg.setAttribute(QStringLiteral("base"), QString::number(g.base >= 0 ? g.base : int(&f - cache.fields.constData())));
            if (g.kind == PivotFieldGroup::Range || g.kind == PivotFieldGroup::Dates) {
                QDomElement rp = doc.createElement(QStringLiteral("rangePr"));
                if (g.kind == PivotFieldGroup::Dates) {
                    rp.setAttribute(QStringLiteral("groupBy"), g.groupBy);
                    // (a bound the calendar has no day for is left out: the attribute is optional, and an empty one is no date)
                    if (!isoDate(g.start, dates).isEmpty()) rp.setAttribute(QStringLiteral("startDate"), isoDate(g.start, dates));
                    if (!isoDate(g.end, dates).isEmpty()) rp.setAttribute(QStringLiteral("endDate"), isoDate(g.end, dates));
                } else {
                    rp.setAttribute(QStringLiteral("startNum"), num(g.start));
                    rp.setAttribute(QStringLiteral("endNum"), num(g.end));
                }
                if (!g.autoStart) rp.setAttribute(QStringLiteral("autoStart"), QStringLiteral("0"));
                if (!g.autoEnd) rp.setAttribute(QStringLiteral("autoEnd"), QStringLiteral("0"));
                if (g.interval != 1) rp.setAttribute(QStringLiteral("groupInterval"), num(g.interval));
                fg.appendChild(rp);
            } else {
                QDomElement dp = doc.createElement(QStringLiteral("discretePr"));
                dp.setAttribute(QStringLiteral("count"), QString::number(g.discrete.size()));
                for (qint32 x : g.discrete) {
                    QDomElement xe = doc.createElement(QStringLiteral("x"));
                    xe.setAttribute(QStringLiteral("v"), QString::number(x));
                    dp.appendChild(xe);
                }
                fg.appendChild(dp);
            }
            QDomElement gi = doc.createElement(QStringLiteral("groupItems"));
            gi.setAttribute(QStringLiteral("count"), QString::number(g.labels.size()));
            for (const QString &label : g.labels) {
                QDomElement se = doc.createElement(QStringLiteral("s"));
                se.setAttribute(QStringLiteral("v"), pivotEncodeXString(label));
                gi.appendChild(se);
            }
            fg.appendChild(gi);
            e.appendChild(fg);
        }
        made.appendChild(e);
    }
    if (!fields.isNull()) root.replaceChild(made, fields);
    else root.insertAfter(made, source);
    return doc.toByteArray(-1);
}

QByteArray writePivotCacheRecords(const PivotCache &cache, jp::calc::DateSystem dates)
{
    const int nf = int(cache.fields.size());
    qint64 n = 0;
    for (const PivotCacheField &f : cache.fields)
        if (f.group.databaseField) {
            n = f.storedRecords().size();
            break;
        }
    QByteArray out;
    out.reserve(qsizetype(std::min<qint64>(n * nf * 14 + 400, qint64(1) << 30)));
    out += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pivotCacheRecords xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" count=\"";
    out += QByteArray::number(n);
    out += "\">";
    // (a value that is not shared is written whole, once for each distinct value: its text is made once)
    std::vector<QVector<QByteArray>> inlineTags;
    inlineTags.resize(size_t(nf));
    for (int f = 0; f < nf; ++f)
        if (cache.fields[f].group.databaseField && !cache.fields[f].storedHasShared()) {
            inlineTags[size_t(f)].resize(cache.fields[f].storedItems().size());
            for (qsizetype i = 0; i < cache.fields[f].storedItems().size(); ++i) inlineTags[size_t(f)][i] = valueTag(cache.fields[f].storedItems()[i], dates).toUtf8();
        }
    for (qint64 r = 0; r < n; ++r) {
        out += "<r>";
        for (int f = 0; f < nf; ++f) {
            const PivotCacheField &pf = cache.fields[f];
            if (!pf.group.databaseField) continue;   // (a group of another field has no values of its own)
            const qint32 idx = pf.storedRecords()[r];
            if (pf.storedHasShared() && idx < pf.storedSharedCount()) {
                out += "<x v=\"";
                out += QByteArray::number(idx);
                out += "\"/>";
            } else if (pf.storedHasShared()) {
                out += valueTag(pf.storedItems()[idx], dates).toUtf8();
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
    // (A part nested deeper than a pivot table ever is, or of more elements than the document tree holds in a few gigabytes, is not parsed: the parser takes time that grows with the square of the depth,
    // and the tree takes about a kilobyte and a half for each element. The part the file has is kept.)
    if (pivotXmlDepth(original, kPivotMaxXmlDepth) > kPivotMaxXmlDepth || original.count('<') > kPivotMaxDomElements) return QByteArray();
    QDomDocument doc;
    if (!doc.setContent(original)) return QByteArray();
    QDomElement root = doc.documentElement();
    setBool(root, QStringLiteral("dataOnRows"), t.dataOnRows, false);
    setBool(root, QStringLiteral("compact"), t.compact, true);
    setBool(root, QStringLiteral("outline"), t.outline, false);
    if (root.hasAttribute(QStringLiteral("compactData")) || t.compact != true) setBool(root, QStringLiteral("compactData"), t.compact, true);
    if (root.hasAttribute(QStringLiteral("outlineData")) || t.outline) setBool(root, QStringLiteral("outlineData"), t.outline && !t.compact, false);
    setBool(root, QStringLiteral("rowGrandTotals"), t.rowGrandTotals, true);
    setBool(root, QStringLiteral("colGrandTotals"), t.colGrandTotals, true);
    if (!t.name.isEmpty()) root.setAttribute(QStringLiteral("name"), pivotEncodeXString(t.name));
    setBool(root, QStringLiteral("mergeItem"), t.mergeItem, false);
    setBool(root, QStringLiteral("showError"), t.showError, false);
    setBool(root, QStringLiteral("showMissing"), t.showMissing, true);
    if (t.errorCaption.isEmpty()) root.removeAttribute(QStringLiteral("errorCaption"));
    else root.setAttribute(QStringLiteral("errorCaption"), t.errorCaption);
    if (t.missingCaption.isEmpty()) root.removeAttribute(QStringLiteral("missingCaption"));
    else root.setAttribute(QStringLiteral("missingCaption"), t.missingCaption);
    setBool(root, QStringLiteral("preserveFormatting"), t.preserveFormatting, true);
    setBool(root, QStringLiteral("useAutoFormatting"), t.useAutoFormatting, false);
    setBool(root, QStringLiteral("showHeaders"), t.showHeaders, true);
    setBool(root, QStringLiteral("multipleFieldFilters"), t.multipleFieldFilters, true);
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
    QSet<int> changedItems;   // the fields whose list of items is not the one the part had
    // each field's element is its own, found by the place it had in the part: its marks and children (a sort by a value, an extension) go with its column; a field made here has a plain one
    QVector<QDomElement> ordered;
    {
        QVector<QDomElement> original;
        for (QDomNode n = fields.firstChild(); !n.isNull(); n = n.nextSibling())
            if (n.isElement() && n.toElement().tagName() == QLatin1String("pivotField")) original.append(n.toElement());
        for (const PivotField &pf : t.fields) {
            if (pf.src >= 0 && pf.src < original.size()) {
                ordered.append(original[pf.src].cloneNode(true).toElement());
            } else {
                QDomElement e = doc.createElement(QStringLiteral("pivotField"));
                e.setAttribute(QStringLiteral("showAll"), QStringLiteral("0"));
                if (pf.calculated) {
                    e.setAttribute(QStringLiteral("dragToRow"), QStringLiteral("0"));
                    e.setAttribute(QStringLiteral("dragToCol"), QStringLiteral("0"));
                    e.setAttribute(QStringLiteral("dragToPage"), QStringLiteral("0"));
                    e.setAttribute(QStringLiteral("defaultSubtotal"), QStringLiteral("0"));
                }
                ordered.append(e);
            }
        }
        for (const QDomElement &e : original) fields.removeChild(e);
        for (const QDomElement &e : ordered) fields.appendChild(e);
    }
    for (int fi = 0; fi < ordered.size(); ++fi) {
        QDomElement e = ordered[fi];
        const PivotField &pf = t.fields[fi];
        if (pf.axis.isEmpty()) e.removeAttribute(QStringLiteral("axis"));
        else e.setAttribute(QStringLiteral("axis"), pf.axis);
        setBool(e, QStringLiteral("dataField"), pf.dataField, false);
        if (pf.name.isEmpty()) e.removeAttribute(QStringLiteral("name"));
        else e.setAttribute(QStringLiteral("name"), pivotEncodeXString(pf.name));
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
        {
            // (the repeat of item labels is the field's extension x14:pivotField)
            QDomElement ext = childNamed(e, QStringLiteral("extLst"));
            QDomElement mark;
            if (!ext.isNull())
                for (QDomNode n = ext.firstChild(); !n.isNull() && mark.isNull(); n = n.nextSibling())
                    for (QDomNode m = n.firstChild(); !m.isNull() && mark.isNull(); m = m.nextSibling())
                        if (m.isElement() && m.toElement().localName() == QLatin1String("pivotField") && m.toElement().hasAttribute(QStringLiteral("fillDownLabels"))) mark = m.toElement();
            if (!mark.isNull()) {
                setBool(mark, QStringLiteral("fillDownLabels"), pf.fillDownLabels, false);
            } else if (pf.fillDownLabels) {
                const QString x14 = QStringLiteral("http://schemas.microsoft.com/office/spreadsheetml/2009/9/main");
                if (ext.isNull()) {
                    ext = doc.createElement(QStringLiteral("extLst"));
                    e.appendChild(ext);
                }
                QDomElement one = doc.createElement(QStringLiteral("ext"));
                one.setAttribute(QStringLiteral("uri"), QStringLiteral("{2946ED86-A175-432a-8AC1-64E0C546D7DE}"));
                one.setAttribute(QStringLiteral("xmlns:x14"), x14);
                QDomElement m = doc.createElementNS(x14, QStringLiteral("x14:pivotField"));
                m.setAttribute(QStringLiteral("fillDownLabels"), QStringLiteral("1"));
                one.appendChild(m);
                ext.appendChild(one);
            }
        }
        if (pf.sortType == QLatin1String("manual")) e.removeAttribute(QStringLiteral("sortType"));
        else e.setAttribute(QStringLiteral("sortType"), pf.sortType);
        if (pf.numFmtId > 0) e.setAttribute(QStringLiteral("numFmtId"), QString::number(pf.numFmtId));
        else e.removeAttribute(QStringLiteral("numFmtId"));
        // a sort by the values of a data field follows the data field's place; one whose data field is gone is dropped
        QDomElement scope = childNamed(e, QStringLiteral("autoSortScope"));
        if (!scope.isNull()) {
            QDomElement x;
            for (QDomNode n = scope.firstChild(); !n.isNull() && x.isNull(); n = n.nextSibling()) {
                // (the x of the one reference: pivotArea > references > reference > x)
                QDomElement area = n.toElement(), refs = childNamed(area, QStringLiteral("references")), ref = childNamed(refs, QStringLiteral("reference"));
                x = childNamed(ref, QStringLiteral("x"));
            }
            if (pf.sortByData >= 0 && !x.isNull()) x.setAttribute(QStringLiteral("v"), QString::number(pf.sortByData));
            else e.removeChild(scope);
        } else if (pf.sortByData >= 0) {
            // (a sort by the values of a data field a person chose: the scope, as the original program writes it)
            QDomElement made = doc.createElement(QStringLiteral("autoSortScope"));
            QDomElement area = doc.createElement(QStringLiteral("pivotArea"));
            area.setAttribute(QStringLiteral("dataOnly"), QStringLiteral("0"));
            area.setAttribute(QStringLiteral("outline"), QStringLiteral("0"));
            area.setAttribute(QStringLiteral("fieldPosition"), QStringLiteral("0"));
            QDomElement refs = doc.createElement(QStringLiteral("references"));
            refs.setAttribute(QStringLiteral("count"), QStringLiteral("1"));
            QDomElement ref = doc.createElement(QStringLiteral("reference"));
            ref.setAttribute(QStringLiteral("field"), QStringLiteral("4294967294"));
            ref.setAttribute(QStringLiteral("count"), QStringLiteral("1"));
            ref.setAttribute(QStringLiteral("selected"), QStringLiteral("0"));
            QDomElement xv = doc.createElement(QStringLiteral("x"));
            xv.setAttribute(QStringLiteral("v"), QString::number(pf.sortByData));
            ref.appendChild(xv);
            refs.appendChild(ref);
            area.appendChild(refs);
            made.appendChild(area);
            // (after the items, which are made below and put first; before the field's extension list)
            const QDomElement ext = childNamed(e, QStringLiteral("extLst"));
            if (ext.isNull()) e.appendChild(made);
            else e.insertBefore(made, ext);
        }
        QDomElement items = childNamed(e, QStringLiteral("items"));
        auto itemsText = [](const QDomElement &list) {
            QString out;
            for (QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (!n.isElement()) continue;
                const QDomNamedNodeMap a = n.toElement().attributes();
                QStringList parts;
                for (int i = 0; i < a.size(); ++i) parts << a.item(i).toAttr().name() + QLatin1Char('=') + a.item(i).toAttr().value();
                parts.sort();
                out += parts.join(QLatin1Char(',')) + QLatin1Char(';');
            }
            return out;
        };
        const QString itemsWere = items.isNull() ? QString() : itemsText(items);
        if (!items.isNull()) e.removeChild(items);
        if (pf.hasItems && !pf.items.isEmpty()) {
            QDomElement made = doc.createElement(QStringLiteral("items"));
            made.setAttribute(QStringLiteral("count"), QString::number(pf.items.size()));
            for (const PivotTableItem &it : pf.items) {
                QDomElement ie = doc.createElement(QStringLiteral("item"));
                if (it.t.isEmpty()) {
                    if (it.hidden) ie.setAttribute(QStringLiteral("h"), QStringLiteral("1"));
                    if (it.missing) ie.setAttribute(QStringLiteral("m"), QStringLiteral("1"));
                    if (!it.showDetails) ie.setAttribute(QStringLiteral("sd"), QStringLiteral("0"));
                    if (!it.n.isEmpty()) ie.setAttribute(QStringLiteral("n"), pivotEncodeXString(it.n));
                    ie.setAttribute(QStringLiteral("x"), QString::number(it.x));
                } else {
                    ie.setAttribute(QStringLiteral("t"), it.t);
                }
                made.appendChild(ie);
            }
            // (the items come first among the field's parts)
            e.insertBefore(made, e.firstChild());
            if (itemsText(made) != itemsWere) changedItems.insert(fi);
        } else if (!itemsWere.isEmpty()) {
            changedItems.insert(fi);
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
        if (items.isEmpty()) return QDomElement();
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
    // (a filter that was there keeps what the file said of it, the place of the item in the list apart)
    QHash<int, QDomElement> pageWas;
    {
        const QDomElement list = childNamed(root, QStringLiteral("pageFields"));
        for (QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling())
            if (n.isElement()) pageWas.insert(n.toElement().attribute(QStringLiteral("fld")).toInt(), n.toElement());
    }
    drop("pageFields");
    QDomElement pf;
    if (!t.pageFields.isEmpty()) {
        pf = doc.createElement(QStringLiteral("pageFields"));
        pf.setAttribute(QStringLiteral("count"), QString::number(t.pageFields.size()));
        for (const PivotPageField &p : t.pageFields) {
            QDomElement x = pageWas.contains(p.fld) && !structureChanged ? pageWas.value(p.fld).cloneNode(true).toElement() : doc.createElement(QStringLiteral("pageField"));
            x.setAttribute(QStringLiteral("fld"), QString::number(p.fld));
            if (p.item >= 0) x.setAttribute(QStringLiteral("item"), QString::number(p.item));
            else x.removeAttribute(QStringLiteral("item"));
            if (!x.hasAttribute(QStringLiteral("hier")) && pageWas.contains(p.fld) && pageWas.value(p.fld).hasAttribute(QStringLiteral("hier"))) x.setAttribute(QStringLiteral("hier"), pageWas.value(p.fld).attribute(QStringLiteral("hier")));   // (the file's own mark of it, as the original program writes it back)
            if (!x.hasAttribute(QStringLiteral("hier"))) x.setAttribute(QStringLiteral("hier"), QStringLiteral("-1"));
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
            x.setAttribute(QStringLiteral("name"), pivotEncodeXString(d.name));
            x.setAttribute(QStringLiteral("fld"), QString::number(d.fld));
            if (d.subtotal != QLatin1String("sum")) x.setAttribute(QStringLiteral("subtotal"), d.subtotal);
            const bool extended = d.showDataAs == QLatin1String("rankAscending") || d.showDataAs == QLatin1String("rankDescending") || d.showDataAs == QLatin1String("percentOfParentRow") ||
                                  d.showDataAs == QLatin1String("percentOfParentCol");
            if (d.showDataAs != QLatin1String("normal") && !extended) x.setAttribute(QStringLiteral("showDataAs"), d.showDataAs);
            x.setAttribute(QStringLiteral("baseField"), QString::number(d.baseField));
            x.setAttribute(QStringLiteral("baseItem"), QString::number(d.baseItem));
            if (d.numFmtId > 0) x.setAttribute(QStringLiteral("numFmtId"), QString::number(d.numFmtId));
            if (extended) {
                // (the schema's showDataAs has no word for these: they are the extension of the value field, as the 2010 program writes them)
                const QString x14 = QStringLiteral("http://schemas.microsoft.com/office/spreadsheetml/2009/9/main");
                QDomElement list = doc.createElement(QStringLiteral("extLst"));
                QDomElement ext = doc.createElement(QStringLiteral("ext"));
                ext.setAttribute(QStringLiteral("uri"), QStringLiteral("{E15A36E0-9728-4e99-A89B-3F7291B0FE68}"));
                ext.setAttribute(QStringLiteral("xmlns:x14"), x14);
                QDomElement m = doc.createElementNS(x14, QStringLiteral("x14:dataField"));
                m.setAttribute(QStringLiteral("pivotShowAs"), d.showDataAs);
                m.setAttribute(QStringLiteral("sourceField"), QStringLiteral("0"));
                m.setAttribute(QStringLiteral("uniqueName"), QStringLiteral("[Measures].[%1]").arg(pivotEncodeXString(d.name)));
                ext.appendChild(m);
                list.appendChild(ext);
                x.appendChild(list);
            }
            df.appendChild(x);
        }
    }
    place(rf);
    place(ri);
    place(cf);
    place(ci);
    place(pf);
    place(df);
    {
        // the style: after the formats and the pivot hierarchies, before the filters (the order of the schema)
        QDomElement style = childNamed(root, QStringLiteral("pivotTableStyleInfo"));
        if (style.isNull() && (t.hasStyleInfo || !t.styleName.isEmpty())) {
            style = doc.createElement(QStringLiteral("pivotTableStyleInfo"));
            QDomElement before;
            for (const char *name : {"filters", "rowHierarchiesUsage", "colHierarchiesUsage", "extLst"}) {
                before = childNamed(root, QLatin1String(name));
                if (!before.isNull()) break;
            }
            if (before.isNull()) root.appendChild(style);
            else root.insertBefore(style, before);
        }
        if (!style.isNull()) {
            if (t.styleName.isEmpty()) style.removeAttribute(QStringLiteral("name"));
            else style.setAttribute(QStringLiteral("name"), t.styleName);
            style.setAttribute(QStringLiteral("showRowHeaders"), t.showRowHeaders ? QStringLiteral("1") : QStringLiteral("0"));
            style.setAttribute(QStringLiteral("showColHeaders"), t.showColHeaders ? QStringLiteral("1") : QStringLiteral("0"));
            style.setAttribute(QStringLiteral("showRowStripes"), t.showRowStripes ? QStringLiteral("1") : QStringLiteral("0"));
            style.setAttribute(QStringLiteral("showColStripes"), t.showColStripes ? QStringLiteral("1") : QStringLiteral("0"));
            style.setAttribute(QStringLiteral("showLastColumn"), t.showLastColumn ? QStringLiteral("1") : QStringLiteral("0"));
        }
    }
    // the formats, rules, and chart formats name items by their place in a field's list: those of a field whose list changed, or of a place that is not there, go
    {
        QVector<int> itemCount(t.fields.size(), 0);
        for (int i = 0; i < t.fields.size(); ++i) {
            int k = 0;
            for (const PivotTableItem &it : t.fields[i].items) k += 1;
            itemCount[i] = k;
        }
        std::function<bool(const QDomElement &)> broken = [&](const QDomElement &e) {
            for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (!n.isElement()) continue;
                const QDomElement c = n.toElement();
                if (c.tagName() == QLatin1String("reference")) {
                    const qint64 f = c.attribute(QStringLiteral("field")).toLongLong();
                    if (f == 4294967294LL) continue;
                    if (f < 0 || f >= t.fields.size() || changedItems.contains(int(f))) return true;
                    for (QDomNode x = c.firstChild(); !x.isNull(); x = x.nextSibling())
                        if (x.isElement() && x.toElement().attribute(QStringLiteral("v")).toInt() >= std::max(itemCount[f], 1)) return true;
                } else if (broken(c)) {
                    return true;
                }
            }
            return false;
        };
        for (const char *name : {"formats", "conditionalFormats", "chartFormats"}) {
            QDomElement list = childNamed(root, QLatin1String(name));
            if (list.isNull()) continue;
            QList<QDomElement> gone;
            for (QDomNode n = list.firstChild(); !n.isNull(); n = n.nextSibling())
                if (n.isElement() && broken(n.toElement())) gone.append(n.toElement());
            for (const QDomElement &e : gone) list.removeChild(e);
            const int left = int(list.childNodes().size());
            if (left == 0) root.removeChild(list);
            else if (!gone.isEmpty()) list.setAttribute(QStringLiteral("count"), QString::number(left));
        }
    }
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

namespace jp::office::sheet {

QByteArray patchPivotCacheSource(const QByteArray &def, const QString &sheet, const QString &ref)
{
    const qsizetype at = def.indexOf("worksheetSource");
    if (at < 0) return def;
    const qsizetype start = def.lastIndexOf('<', at), end = def.indexOf('>', at);
    if (start < 0 || end < 0) return def;
    QString tag = QString::fromUtf8(def.mid(start, end - start + 1));
    auto set = [&](const QString &name, const QString &value) {
        const QRegularExpression re(QStringLiteral("(\\s%1\\s*=\\s*)(\"[^\"]*\"|'[^']*')").arg(name));
        const QString quoted = QLatin1Char('"') + value.toHtmlEscaped().replace(QLatin1Char('"'), QLatin1String("&quot;")) + QLatin1Char('"');
        if (tag.contains(re)) tag.replace(re, QStringLiteral("\\1") + quoted);
        else tag.insert(tag.indexOf(QLatin1String("worksheetSource")) + 15, QLatin1Char(' ') + name + QLatin1Char('=') + quoted);
    };
    set(QStringLiteral("ref"), ref);
    set(QStringLiteral("sheet"), sheet);
    return def.left(start) + tag.toUtf8() + def.mid(end + 1);
}

} // namespace jp::office::sheet
