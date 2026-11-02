#include "sheet/pivot.h"

#include <QDate>
#include <QXmlStreamReader>

#include <cmath>
#include <unordered_map>

namespace jp::office::sheet {

namespace {

bool flag(QStringView v, bool def)
{
    if (v.isEmpty()) return def;
    return v == QLatin1String("1") || v == QLatin1String("true");
}

int toInt(QStringView v, int def)
{
    bool ok = false;
    const int n = v.toInt(&ok);
    return ok ? n : def;
}

} // namespace

// Finds the place of a value among a field's items, so that the records of a big list hold each distinct value once.
struct PivotIntern {
    std::unordered_map<double, qint32> numbers, dates;
    QHash<QString, qint32> texts, errors;
    qint32 bools[2] = {-1, -1}, missing = -1;
    qint32 find(const PivotValue &v) const
    {
        switch (v.kind) {
        case PivotValue::Missing: return missing;
        case PivotValue::Number: {
            const auto it = numbers.find(v.number);
            return it == numbers.end() ? -1 : it->second;
        }
        case PivotValue::Date: {
            const auto it = dates.find(v.number);
            return it == dates.end() ? -1 : it->second;
        }
        case PivotValue::Bool: return bools[v.number != 0];
        case PivotValue::Text: return texts.value(v.text, -1);
        case PivotValue::Error: return errors.value(v.text, -1);
        }
        return -1;
    }
    void put(const PivotValue &v, qint32 at)
    {
        switch (v.kind) {
        case PivotValue::Missing: if (missing < 0) missing = at; break;
        case PivotValue::Number: numbers.emplace(v.number, at); break;
        case PivotValue::Date: dates.emplace(v.number, at); break;
        case PivotValue::Bool: if (bools[v.number != 0] < 0) bools[v.number != 0] = at; break;
        case PivotValue::Text: if (!texts.contains(v.text)) texts.insert(v.text, at); break;
        case PivotValue::Error: if (!errors.contains(v.text)) errors.insert(v.text, at); break;
        }
    }
};

namespace {

// The day number of a date as the cache writes it ("2001-04-12T00:00:00"); the day before January 1, 1900 is day 0 in the 1900 system.
bool dateSerial(const QString &text, jp::calc::DateSystem system, double *serial)
{
    if (jp::calc::parseIsoDate(text, system, serial)) return true;
    if (system != jp::calc::DateSystem::Date1900 || text.size() < 10) return false;
    const QDate d = QDate::fromString(text.left(10), Qt::ISODate);
    if (!d.isValid() || d.year() != 1899 || d.month() != 12 || d.day() != 31) return false;
    double frac = 0;
    if (text.size() > 11 && !jp::calc::parseIsoDate(text.mid(11), system, &frac)) return false;
    *serial = text.size() > 11 ? frac : 0;
    return true;
}

// A value element of the cache: <s v=""/>, <n v=""/>, <d v=""/>, <b v=""/>, <e v=""/>, <m/>.
bool valueOf(const QXmlStreamReader &r, jp::calc::DateSystem dates, PivotValue *out)
{
    const QStringView name = r.name();
    const QStringView v = r.attributes().value(QLatin1String("v"));
    if (name == QLatin1String("s")) {
        out->kind = PivotValue::Text;
        out->text = v.toString();
        return true;
    }
    if (name == QLatin1String("n")) {
        bool ok = false;
        const double d = v.toDouble(&ok);
        if (!ok || !std::isfinite(d)) return false;
        out->kind = PivotValue::Number;
        out->number = d;
        return true;
    }
    if (name == QLatin1String("d")) {
        double serial = 0;
        if (!dateSerial(v.toString(), dates, &serial)) return false;
        out->kind = PivotValue::Date;
        out->number = serial;
        return true;
    }
    if (name == QLatin1String("b")) {
        out->kind = PivotValue::Bool;
        out->number = flag(v, false) ? 1 : 0;
        return true;
    }
    if (name == QLatin1String("e")) {
        out->kind = PivotValue::Error;
        out->text = v.toString();
        return true;
    }
    if (name == QLatin1String("m")) {
        out->kind = PivotValue::Missing;
        return true;
    }
    return false;
}

} // namespace

bool readPivotCache(const QByteArray &definition, const QByteArray &records, jp::calc::DateSystem dates, PivotCache *out, QString *why)
{
    *out = PivotCache();
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    QXmlStreamReader r(definition);
    int depth = 0;
    bool sawRoot = false;
    PivotCacheField *cur = nullptr;
    bool inShared = false;
    std::vector<PivotIntern> interns;
    qint64 distinct = 0;
    while (!r.atEnd()) {
        const auto tok = r.readNext();
        if (tok == QXmlStreamReader::StartElement) {
            ++depth;
            const QStringView n = r.name();
            if (depth == 1) {
                if (n != QLatin1String("pivotCacheDefinition")) return fail(QStringLiteral("not a pivot cache definition"));
                sawRoot = true;
                const auto a = r.attributes();
                out->recordCount = a.value(QLatin1String("recordCount")).toLongLong();
                out->saveData = flag(a.value(QLatin1String("saveData")), true);
                out->refreshOnLoad = flag(a.value(QLatin1String("refreshOnLoad")), false);
            } else if (n == QLatin1String("cacheSource") && depth == 2) {
                out->sourceType = r.attributes().value(QLatin1String("type")).toString();
            } else if (n == QLatin1String("worksheetSource")) {
                const auto a = r.attributes();
                out->ref = a.value(QLatin1String("ref")).toString();
                out->sheet = a.value(QLatin1String("sheet")).toString();
                out->name = a.value(QLatin1String("name")).toString();
            } else if (n == QLatin1String("cacheField") && depth == 3) {
                if (out->fields.size() >= kPivotMaxFields) return fail(QStringLiteral("too many fields"));
                out->fields.append(PivotCacheField());
                interns.emplace_back();
                cur = &out->fields.last();
                const auto a = r.attributes();
                cur->name = a.value(QLatin1String("name")).toString();
                cur->numFmtId = toInt(a.value(QLatin1String("numFmtId")), 0);
                if (!flag(a.value(QLatin1String("databaseField")), true) || !a.value(QLatin1String("formula")).isEmpty())
                    out->unsupported = QStringLiteral("a calculated field");
            } else if (n == QLatin1String("sharedItems") && cur) {
                inShared = true;
                cur->hasShared = true;
                QString attrs;
                for (const auto &at : r.attributes()) attrs += at.name().toString() + QLatin1Char('=') + at.value().toString() + QLatin1Char(' ');
                cur->sharedAttrs = attrs;
            } else if (inShared && cur && depth == 5) {
                PivotValue v;
                if (!valueOf(r, dates, &v)) return fail(QStringLiteral("a shared item can't be read"));
                if (cur->items.size() >= kPivotMaxItemsPerField || ++distinct > 12000000) return fail(QStringLiteral("too many items"));
                interns.back().put(v, qint32(cur->items.size()));
                cur->items.append(v);
            } else if (n == QLatin1String("fieldGroup") && cur) {
                out->unsupported = QStringLiteral("a grouped field");
            }
        } else if (tok == QXmlStreamReader::EndElement) {
            if (r.name() == QLatin1String("sharedItems") && depth == 4) {
                inShared = false;
                if (cur) cur->sharedCount = int(cur->items.size());
            } else if (r.name() == QLatin1String("cacheField") && depth == 3) {
                cur = nullptr;
            }
            --depth;
        }
    }
    if (r.hasError() || !sawRoot) return fail(QStringLiteral("the cache definition is not well formed"));
    const int nf = int(out->fields.size());
    if (nf == 0) return fail(QStringLiteral("the cache has no fields"));

    if (records.isEmpty()) {
        if (out->recordCount > 0 || out->saveData) out->unsupported = out->unsupported.isEmpty() ? QStringLiteral("the records are not kept") : out->unsupported;
        return true;
    }
    QXmlStreamReader rr(records);
    int d = 0, field = 0;
    qint64 count = 0;
    bool inRecord = false;
    while (!rr.atEnd()) {
        const auto tok = rr.readNext();
        if (tok == QXmlStreamReader::StartElement) {
            ++d;
            const QStringView n = rr.name();
            if (d == 2 && n == QLatin1String("r")) {
                inRecord = true;
                field = 0;
                if (++count > kPivotMaxRecords || count * nf > kPivotMaxRecordCells) return fail(QStringLiteral("too many records"));
            } else if (d == 3 && inRecord) {
                if (field >= nf) return fail(QStringLiteral("a record has more values than the cache has fields"));
                PivotCacheField &f = out->fields[field];
                qint32 at = -1;
                if (n == QLatin1String("x")) {
                    at = toInt(rr.attributes().value(QLatin1String("v")), -1);
                    if (at < 0 || at >= f.sharedCount) return fail(QStringLiteral("a record names a shared item that is not there"));
                } else {
                    PivotValue v;
                    if (!valueOf(rr, dates, &v)) return fail(QStringLiteral("a value of a record can't be read"));
                    PivotIntern &in = interns[field];
                    at = in.find(v);
                    if (at < 0) {
                        if (f.items.size() >= kPivotMaxItemsPerField || ++distinct > 12000000) return fail(QStringLiteral("too many items"));
                        at = qint32(f.items.size());
                        in.put(v, at);
                        f.items.append(v);
                    }
                }
                f.records.append(at);
                ++field;
            }
        } else if (tok == QXmlStreamReader::EndElement) {
            if (d == 2 && inRecord) {
                inRecord = false;
                if (field != nf) return fail(QStringLiteral("a record has fewer values than the cache has fields"));
            }
            --d;
        }
    }
    if (rr.hasError()) return fail(QStringLiteral("the cache records are not well formed"));
    if (out->recordCount != count) out->recordCount = count;
    return true;
}

bool readPivotTable(const QByteArray &xml, PivotTable *out, QString *why)
{
    *out = PivotTable();
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    QXmlStreamReader r(xml);
    int depth = 0;
    bool sawRoot = false;
    QString section;   // the child of the root being read
    PivotField *field = nullptr;
    QVector<PivotAxisItem> *axis = nullptr;
    PivotAxisItem *line = nullptr;
    QVector<int> *fieldList = nullptr;
    while (!r.atEnd()) {
        const auto tok = r.readNext();
        if (tok == QXmlStreamReader::StartElement) {
            ++depth;
            const QStringView n = r.name();
            const auto a = r.attributes();
            if (depth == 1) {
                if (n != QLatin1String("pivotTableDefinition")) return fail(QStringLiteral("not a pivot table"));
                sawRoot = true;
                out->name = a.value(QLatin1String("name")).toString();
                out->cacheId = toInt(a.value(QLatin1String("cacheId")), 0);
                out->dataOnRows = flag(a.value(QLatin1String("dataOnRows")), false);
                if (a.hasAttribute(QLatin1String("dataCaption"))) out->dataCaption = a.value(QLatin1String("dataCaption")).toString();
                if (a.hasAttribute(QLatin1String("grandTotalCaption"))) out->grandTotalCaption = a.value(QLatin1String("grandTotalCaption")).toString();
                out->rowHeaderCaption = a.value(QLatin1String("rowHeaderCaption")).toString();
                out->colHeaderCaption = a.value(QLatin1String("colHeaderCaption")).toString();
                out->missingCaption = a.value(QLatin1String("missingCaption")).toString();
                out->errorCaption = a.value(QLatin1String("errorCaption")).toString();
                out->showMissing = flag(a.value(QLatin1String("showMissing")), true);
                out->showError = flag(a.value(QLatin1String("showError")), false);
                out->rowGrandTotals = flag(a.value(QLatin1String("rowGrandTotals")), true);
                out->colGrandTotals = flag(a.value(QLatin1String("colGrandTotals")), true);
                out->compact = flag(a.value(QLatin1String("compact")), true);
                out->outline = flag(a.value(QLatin1String("outline")), false);
                out->mergeItem = flag(a.value(QLatin1String("mergeItem")), false);
                out->gridDropZones = flag(a.value(QLatin1String("gridDropZones")), false);
            } else if (depth == 2) {
                section = n.toString();
                if (n == QLatin1String("location")) {
                    out->ref = a.value(QLatin1String("ref")).toString();
                    out->firstHeaderRow = toInt(a.value(QLatin1String("firstHeaderRow")), 1);
                    out->firstDataRow = toInt(a.value(QLatin1String("firstDataRow")), 1);
                    out->firstDataCol = toInt(a.value(QLatin1String("firstDataCol")), 1);
                    out->rowPageCount = toInt(a.value(QLatin1String("rowPageCount")), 0);
                    out->colPageCount = toInt(a.value(QLatin1String("colPageCount")), 0);
                } else if (n == QLatin1String("rowItems")) {
                    axis = &out->rowItems;
                } else if (n == QLatin1String("colItems")) {
                    axis = &out->colItems;
                } else if (n == QLatin1String("rowFields")) {
                    fieldList = &out->rowFields;
                } else if (n == QLatin1String("colFields")) {
                    fieldList = &out->colFields;
                }
            } else if (depth == 3) {
                if (section == QLatin1String("pivotFields") && n == QLatin1String("pivotField")) {
                    if (out->fields.size() >= kPivotMaxFields) return fail(QStringLiteral("too many fields"));
                    out->fields.append(PivotField());
                    field = &out->fields.last();
                    field->name = a.value(QLatin1String("name")).toString();
                    field->axis = a.value(QLatin1String("axis")).toString();
                    field->dataField = flag(a.value(QLatin1String("dataField")), false);
                    field->defaultSubtotal = flag(a.value(QLatin1String("defaultSubtotal")), true);
                    static const std::pair<const char *, const char *> subs[] = {
                        {"sumSubtotal", "sum"}, {"countASubtotal", "countA"}, {"avgSubtotal", "avg"}, {"maxSubtotal", "max"}, {"minSubtotal", "min"},
                        {"productSubtotal", "product"}, {"countSubtotal", "count"}, {"stdDevSubtotal", "stdDev"}, {"stdDevPSubtotal", "stdDevP"},
                        {"varSubtotal", "var"}, {"varPSubtotal", "varP"}};
                    for (const auto &s : subs)
                        if (flag(a.value(QLatin1String(s.first)), false)) field->subtotals << QLatin1String(s.second);
                    field->numFmtId = toInt(a.value(QLatin1String("numFmtId")), 0);
                    if (a.hasAttribute(QLatin1String("sortType"))) field->sortType = a.value(QLatin1String("sortType")).toString();
                    field->showAll = flag(a.value(QLatin1String("showAll")), true);
                    field->subtotalTop = flag(a.value(QLatin1String("subtotalTop")), true);
                    field->compact = flag(a.value(QLatin1String("compact")), true);
                    field->outline = flag(a.value(QLatin1String("outline")), true);
                    field->insertBlankRow = flag(a.value(QLatin1String("insertBlankRow")), false);
                } else if ((section == QLatin1String("rowItems") || section == QLatin1String("colItems")) && n == QLatin1String("i") && axis) {
                    if (axis->size() > 4000000) return fail(QStringLiteral("too many lines"));
                    axis->append(PivotAxisItem());
                    line = &axis->last();
                    line->t = a.value(QLatin1String("t")).toString();
                    line->r = toInt(a.value(QLatin1String("r")), 0);
                    line->i = toInt(a.value(QLatin1String("i")), 0);
                } else if ((section == QLatin1String("rowFields") || section == QLatin1String("colFields")) && n == QLatin1String("field") && fieldList) {
                    fieldList->append(toInt(a.value(QLatin1String("x")), -1));
                } else if (section == QLatin1String("pageFields") && n == QLatin1String("pageField")) {
                    PivotPageField p;
                    p.fld = toInt(a.value(QLatin1String("fld")), 0);
                    p.item = a.hasAttribute(QLatin1String("item")) ? toInt(a.value(QLatin1String("item")), -1) : -1;
                    out->pageFields.append(p);
                } else if (section == QLatin1String("dataFields") && n == QLatin1String("dataField")) {
                    PivotDataField d;
                    d.name = a.value(QLatin1String("name")).toString();
                    d.fld = toInt(a.value(QLatin1String("fld")), 0);
                    if (a.hasAttribute(QLatin1String("subtotal"))) d.subtotal = a.value(QLatin1String("subtotal")).toString();
                    if (a.hasAttribute(QLatin1String("showDataAs"))) d.showDataAs = a.value(QLatin1String("showDataAs")).toString();
                    d.numFmtId = toInt(a.value(QLatin1String("numFmtId")), 0);
                    out->dataFields.append(d);
                } else if (section == QLatin1String("calculatedItems")) {
                    out->unsupported = QStringLiteral("a calculated item");
                }
            } else if (depth == 4 && field && section == QLatin1String("pivotFields") && n == QLatin1String("items")) {
                field->hasItems = true;
            } else if (depth == 5 && field && section == QLatin1String("pivotFields") && n == QLatin1String("item")) {
                if (field->items.size() >= kPivotMaxItemsPerField) return fail(QStringLiteral("too many items"));
                PivotTableItem it;
                it.t = a.value(QLatin1String("t")).toString();
                it.x = a.hasAttribute(QLatin1String("x")) ? toInt(a.value(QLatin1String("x")), -1) : -1;
                it.hidden = flag(a.value(QLatin1String("h")), false);
                it.missing = flag(a.value(QLatin1String("m")), false);
                it.showDetails = flag(a.value(QLatin1String("sd")), true);
                it.n = a.value(QLatin1String("n")).toString();
                field->items.append(it);
            } else if (field && section == QLatin1String("pivotFields") && n == QLatin1String("reference") && a.value(QLatin1String("field")) == QLatin1String("4294967294")) {
                field->sortByData = -2;   // (the data field is the x inside)
            } else if (field && field->sortByData == -2 && section == QLatin1String("pivotFields") && n == QLatin1String("x")) {
                field->sortByData = toInt(a.value(QLatin1String("v")), 0);
            } else if (depth == 4 && line && n == QLatin1String("x")) {
                line->x.append(toInt(a.value(QLatin1String("v")), 0));
            }
        } else if (tok == QXmlStreamReader::EndElement) {
            if (depth == 2) {
                axis = nullptr;
                fieldList = nullptr;
                section.clear();
            } else if (depth == 3) {
                field = nullptr;
                line = nullptr;
            }
            --depth;
        }
    }
    if (r.hasError() || !sawRoot) return fail(QStringLiteral("the pivot table is not well formed"));
    if (out->ref.isEmpty()) return fail(QStringLiteral("the pivot table has no location"));
    return true;
}

QString pivotRangeRef(quint32 row1, quint32 col1, quint32 row2, quint32 col2)
{
    auto colName = [](quint32 c) {
        QString s;
        for (qint64 n = qint64(c) + 1; n > 0; n = (n - 1) / 26) s.prepend(QChar(u'A' + int((n - 1) % 26)));
        return s;
    };
    return colName(col1) + QString::number(row1 + 1) + QLatin1Char(':') + colName(col2) + QString::number(row2 + 1);
}

} // namespace jp::office::sheet
