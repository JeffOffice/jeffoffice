#include "sheet/datasteps.h"

#include <QCoreApplication>
#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

const QStringList &comparisons()
{
    static const QStringList list{QStringLiteral("eq"), QStringLiteral("ne"), QStringLiteral("contains"), QStringLiteral("notcontains"), QStringLiteral("gt"), QStringLiteral("ge"), QStringLiteral("lt"), QStringLiteral("le"), QStringLiteral("blank"), QStringLiteral("notblank")};
    return list;
}

const char *opName(DataStep::Op op)
{
    switch (op) {
    case DataStep::Op::Keep: return "keep";
    case DataStep::Op::Remove: return "remove";
    case DataStep::Op::Rename: return "rename";
    case DataStep::Op::Filter: return "filter";
    case DataStep::Op::Sort: return "sort";
    case DataStep::Op::Kind: return "kind";
    }
    return "";
}

bool opFromName(const QString &name, DataStep::Op *op)
{
    for (int i = 0; i <= int(DataStep::Op::Kind); ++i)
        if (name == QLatin1String(opName(DataStep::Op(i)))) {
            *op = DataStep::Op(i);
            return true;
        }
    return false;
}

QJsonArray toArray(const QStringList &list)
{
    QJsonArray a;
    for (const QString &s : list) a.append(s);
    return a;
}

}   // namespace

bool DataStep::operator==(const DataStep &o) const
{
    return op == o.op && cols == o.cols && col == o.col && to == o.to && cmp == o.cmp && value == o.value && descending == o.descending && kind == o.kind && order == o.order && decimal == o.decimal
           && group == o.group && label == o.label;
}

QString dataStepsToJson(const QVector<DataStep> &steps)
{
    QJsonArray all;
    for (const DataStep &s : steps) {
        QJsonObject o;
        o.insert(QStringLiteral("op"), QString::fromLatin1(opName(s.op)));
        if (!s.cols.isEmpty()) o.insert(QStringLiteral("cols"), toArray(s.cols));
        if (!s.col.isEmpty()) o.insert(QStringLiteral("col"), s.col);
        if (!s.to.isEmpty()) o.insert(QStringLiteral("to"), s.to);
        if (!s.cmp.isEmpty()) o.insert(QStringLiteral("cmp"), s.cmp);
        if (!s.value.isEmpty()) o.insert(QStringLiteral("value"), s.value);
        if (s.descending) o.insert(QStringLiteral("desc"), true);
        if (!s.kind.isEmpty()) o.insert(QStringLiteral("kind"), s.kind);
        if (!s.order.isEmpty()) o.insert(QStringLiteral("order"), s.order);
        if (!s.decimal.isEmpty()) o.insert(QStringLiteral("decimal"), s.decimal);
        if (!s.group.isEmpty()) o.insert(QStringLiteral("group"), s.group);
        if (!s.label.isEmpty()) o.insert(QStringLiteral("label"), s.label);
        all.append(o);
    }
    return QString::fromUtf8(QJsonDocument(all).toJson(QJsonDocument::Compact));
}

bool parseDataSteps(const QString &json, QVector<DataStep> *out, QString *why)
{
    auto fail = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    out->clear();
    if (json.size() > kMaxDataStepsJsonChars) return fail(QCoreApplication::translate("jp::DataSteps", "The list of steps is too long."));
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isArray()) return fail(QCoreApplication::translate("jp::DataSteps", "The list of steps can't be read."));
    const QJsonArray all = doc.array();
    if (all.size() > kMaxDataSteps) return fail(QCoreApplication::translate("jp::DataSteps", "There are more steps than JeffCrunch runs."));
    auto text = [&](const QJsonObject &o, const char *key, QString *dest) {
        const QJsonValue v = o.value(QLatin1String(key));
        if (v.isUndefined()) return true;
        if (!v.isString() || v.toString().size() > kMaxDataStepChars) return false;
        *dest = v.toString();
        return true;
    };
    for (int i = 0; i < all.size(); ++i) {
        if (!all[i].isObject()) return fail(QCoreApplication::translate("jp::DataSteps", "The list of steps can't be read."));
        const QJsonObject o = all[i].toObject();
        DataStep s;
        if (!o.value(QLatin1String("op")).isString() || !opFromName(o.value(QLatin1String("op")).toString(), &s.op))
            return fail(QCoreApplication::translate("jp::DataSteps", "Step %1 is of a kind this version of JeffCrunch does not have. It may have been made in a newer version.").arg(i + 1));
        bool good = text(o, "col", &s.col) && text(o, "to", &s.to) && text(o, "cmp", &s.cmp) && text(o, "value", &s.value) && text(o, "kind", &s.kind) && text(o, "order", &s.order)
                    && text(o, "decimal", &s.decimal) && text(o, "group", &s.group) && text(o, "label", &s.label);
        if (o.contains(QLatin1String("cols"))) {
            const QJsonValue v = o.value(QLatin1String("cols"));
            if (!v.isArray() || v.toArray().size() > 256) good = false;
            else
                for (const QJsonValue &c : v.toArray()) {
                    if (!c.isString() || c.toString().size() > kMaxDataStepChars) good = false;
                    else s.cols << c.toString();
                }
        }
        if (o.contains(QLatin1String("desc"))) {
            if (!o.value(QLatin1String("desc")).isBool()) good = false;
            s.descending = o.value(QLatin1String("desc")).toBool();
        }
        if (s.op == DataStep::Op::Filter && !comparisons().contains(s.cmp)) good = false;
        if (s.op == DataStep::Op::Kind && s.kind != QLatin1String("text") && s.kind != QLatin1String("number") && s.kind != QLatin1String("date")) good = false;
        if (s.op == DataStep::Op::Kind && s.kind == QLatin1String("date") && s.order != QLatin1String("MDY") && s.order != QLatin1String("DMY") && s.order != QLatin1String("YMD")) good = false;
        if (!good) return fail(QCoreApplication::translate("jp::DataSteps", "Step %1 can't be read.").arg(i + 1));
        out->push_back(s);
    }
    return true;
}

QString dataStepTitle(const DataStep &s)
{
    if (!s.label.isEmpty()) return s.label;
    const QString comma = QStringLiteral(", ");
    switch (s.op) {
    case DataStep::Op::Keep: return QCoreApplication::translate("jp::DataSteps", "Choose columns: %1").arg(s.cols.join(comma));
    case DataStep::Op::Remove: return QCoreApplication::translate("jp::DataSteps", "Remove columns: %1").arg(s.cols.join(comma));
    case DataStep::Op::Rename: return QCoreApplication::translate("jp::DataSteps", "Rename %1 to %2").arg(s.col, s.to);
    case DataStep::Op::Filter: {
        const QStringList words{QCoreApplication::translate("jp::DataSteps", "equals"), QCoreApplication::translate("jp::DataSteps", "does not equal"), QCoreApplication::translate("jp::DataSteps", "contains"), QCoreApplication::translate("jp::DataSteps", "does not contain"), QCoreApplication::translate("jp::DataSteps", "is greater than"), QCoreApplication::translate("jp::DataSteps", "is at least"), QCoreApplication::translate("jp::DataSteps", "is less than"), QCoreApplication::translate("jp::DataSteps", "is at most"), QCoreApplication::translate("jp::DataSteps", "is blank"), QCoreApplication::translate("jp::DataSteps", "is not blank")};
        const int i = comparisons().indexOf(s.cmp);
        const QString word = i >= 0 ? words[i] : s.cmp;
        return i >= 8 ? QCoreApplication::translate("jp::DataSteps", "Keep rows where %1 %2").arg(s.col, word) : QCoreApplication::translate("jp::DataSteps", "Keep rows where %1 %2 %3").arg(s.col, word, s.value);
    }
    case DataStep::Op::Sort: return s.descending ? QCoreApplication::translate("jp::DataSteps", "Sort %1, largest first").arg(s.col) : QCoreApplication::translate("jp::DataSteps", "Sort %1, smallest first").arg(s.col);
    case DataStep::Op::Kind: return QCoreApplication::translate("jp::DataSteps", "Change %1 to %2").arg(s.col, s.kind == QLatin1String("number") ? QCoreApplication::translate("jp::DataSteps", "number") : s.kind == QLatin1String("date") ? QCoreApplication::translate("jp::DataSteps", "date") : QCoreApplication::translate("jp::DataSteps", "text"));
    }
    return QString();
}

namespace {

struct Engine {
    DataRows &rows;
    QVector<char> forced;
    const std::function<bool()> &cancelled;
    qint64 work = 0;
    qint64 limit = kMaxDataStepWork;
    QString why;

    bool stopped() const { return cancelled && cancelled(); }

    int column(const QString &name) const
    {
        for (int i = 0; i < rows.header.size(); ++i)
            if (rows.header[i].compare(name, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }
    bool needColumn(const QString &name, int *index)
    {
        *index = column(name);
        if (*index >= 0) return true;
        why = QCoreApplication::translate("jp::DataSteps", "The column \"%1\" is not there. The columns are: %2.").arg(name, rows.header.join(QStringLiteral(", ")));
        return false;
    }
    bool budget(qint64 cells)
    {
        work += cells;
        if (work > limit) {
            why = QCoreApplication::translate("jp::DataSteps", "The steps would take too long on this many rows.");
            return false;
        }
        return true;
    }
};

bool plainNumber(const QString &t, double *d)
{
    static const QRegularExpression number(QStringLiteral("^[+-]?([0-9]+(\\.[0-9]*)?|\\.[0-9]+)([eE][+-]?[0-9]+)?$"));
    const QString s = t.trimmed();
    if (s.isEmpty() || s.size() > 40 || !number.match(s).hasMatch()) return false;
    bool ok = false;
    const double v = QLocale::c().toDouble(s, &ok);
    if (!ok || !std::isfinite(v)) return false;
    *d = v;
    return true;
}

bool blank(const DataCell &c) { return c.text.trimmed().isEmpty(); }

bool filterKeeps(const DataCell &c, const QString &cmp, const QString &value, bool valueIsNumber, double valueNumber)
{
    if (cmp == QLatin1String("blank")) return blank(c);
    if (cmp == QLatin1String("notblank")) return !blank(c);
    if (cmp == QLatin1String("contains")) return c.text.contains(value, Qt::CaseInsensitive);
    if (cmp == QLatin1String("notcontains")) return !c.text.contains(value, Qt::CaseInsensitive);
    if (blank(c)) return cmp == QLatin1String("ne");
    double d = 0;
    int order;
    if (valueIsNumber && plainNumber(c.text, &d)) order = d < valueNumber ? -1 : d > valueNumber ? 1 : 0;
    else order = QString::compare(c.text, value, Qt::CaseInsensitive);
    if (cmp == QLatin1String("eq")) return order == 0;
    if (cmp == QLatin1String("ne")) return order != 0;
    if (cmp == QLatin1String("gt")) return order > 0;
    if (cmp == QLatin1String("ge")) return order >= 0;
    if (cmp == QLatin1String("lt")) return order < 0;
    return order <= 0;
}

bool runFilter(Engine &e, const DataStep &s)
{
    int c;
    if (!e.needColumn(s.col, &c)) return false;
    if (!e.budget(e.rows.rows.size())) return false;
    double number = 0;
    const bool isNumber = plainNumber(s.value, &number);
    QVector<QVector<DataCell>> kept;
    for (int i = 0; i < e.rows.rows.size(); ++i) {
        if ((i & 4095) == 0 && e.stopped()) {
            e.why = QCoreApplication::translate("jp::DataSteps", "The read was stopped.");
            return false;
        }
        if (filterKeeps(e.rows.rows[i].value(c), s.cmp, s.value, isNumber, number)) kept.push_back(std::move(e.rows.rows[i]));
    }
    e.rows.rows = std::move(kept);
    return true;
}

bool runSort(Engine &e, const DataStep &s)
{
    int c;
    if (!e.needColumn(s.col, &c)) return false;
    const int n = e.rows.rows.size();
    if (!e.budget(qint64(n) * 4)) return false;
    struct Key {
        int rank;   // 0 a number, 1 text, 2 blank
        double number;
        QString text;
        int row;
    };
    QVector<Key> keys;
    keys.reserve(n);
    for (int i = 0; i < n; ++i) {
        const DataCell cell = e.rows.rows[i].value(c);
        Key k{1, 0, cell.text, i};
        if (blank(cell)) k.rank = 2;
        else if (plainNumber(cell.text, &k.number)) k.rank = 0;
        keys.push_back(std::move(k));
    }
    if (e.stopped()) {
        e.why = QCoreApplication::translate("jp::DataSteps", "The read was stopped.");
        return false;
    }
    const bool desc = s.descending;
    std::stable_sort(keys.begin(), keys.end(), [desc](const Key &a, const Key &b) {
        if ((a.rank == 2) != (b.rank == 2)) return a.rank != 2;   // blanks last, in either direction
        if (a.rank == 2) return false;
        if (a.rank != b.rank) return desc ? a.rank > b.rank : a.rank < b.rank;
        const int order = a.rank == 0 ? (a.number < b.number ? -1 : a.number > b.number ? 1 : 0) : QString::compare(a.text, b.text, Qt::CaseInsensitive);
        return desc ? order > 0 : order < 0;
    });
    QVector<QVector<DataCell>> sorted;
    sorted.reserve(n);
    for (const Key &k : keys) sorted.push_back(std::move(e.rows.rows[k.row]));
    e.rows.rows = std::move(sorted);
    return true;
}

// The columns named, as places; false (with the words) when one is not there or is named twice.
bool placesOf(Engine &e, const QStringList &names, QVector<int> *places)
{
    if (names.isEmpty()) {
        e.why = QCoreApplication::translate("jp::DataSteps", "No column is chosen.");
        return false;
    }
    for (const QString &name : names) {
        int c;
        if (!e.needColumn(name, &c)) return false;
        if (places->contains(c)) {
            e.why = QCoreApplication::translate("jp::DataSteps", "The column \"%1\" is named twice.").arg(name);
            return false;
        }
        places->push_back(c);
    }
    return true;
}

bool runColumns(Engine &e, const DataStep &s)
{
    QVector<int> places;
    if (!placesOf(e, s.cols, &places)) return false;
    QVector<int> order;
    if (s.op == DataStep::Op::Keep) order = places;
    else
        for (int c = 0; c < e.rows.header.size(); ++c)
            if (!places.contains(c)) order.push_back(c);
    if (order.isEmpty()) {
        e.why = QCoreApplication::translate("jp::DataSteps", "That would leave no columns.");
        return false;
    }
    if (!e.budget(qint64(e.rows.rows.size()) * order.size())) return false;
    QStringList header;
    QVector<char> forced;
    for (int c : order) {
        header << e.rows.header[c];
        forced << e.forced.value(c);
    }
    for (int i = 0; i < e.rows.rows.size(); ++i) {
        if ((i & 4095) == 0 && e.stopped()) {
            e.why = QCoreApplication::translate("jp::DataSteps", "The read was stopped.");
            return false;
        }
        QVector<DataCell> row;
        row.reserve(order.size());
        for (int c : order) row.push_back(e.rows.rows[i].value(c));
        e.rows.rows[i] = std::move(row);
    }
    e.rows.header = header;
    e.forced = forced;
    return true;
}

bool runRename(Engine &e, const DataStep &s)
{
    int c;
    if (!e.needColumn(s.col, &c)) return false;
    const QString name = s.to.trimmed();
    if (name.isEmpty()) {
        e.why = QCoreApplication::translate("jp::DataSteps", "The new name is empty.");
        return false;
    }
    const int other = e.column(name);
    if (other >= 0 && other != c) {
        e.why = QCoreApplication::translate("jp::DataSteps", "Another column is already called \"%1\".").arg(name);
        return false;
    }
    e.rows.header[c] = name;
    return true;
}

bool toNumber(const QString &text, const QString &decimal, const QString &group, QString *out)
{
    QString t = text.trimmed();
    const QString dec = decimal.isEmpty() ? QStringLiteral(".") : decimal;
    const QString grp = group.isEmpty() ? (dec == QLatin1String(".") ? QStringLiteral(",") : QStringLiteral(".")) : group;
    if (dec == grp) return false;
    if (grp == QLatin1String(" ")) t.remove(QChar(0x00A0)).remove(QChar(0x202F));
    t.remove(grp);
    t.replace(dec, QStringLiteral("."));
    double d;
    if (!plainNumber(t, &d)) return false;
    *out = QLocale::c().toString(d, 'g', QLocale::FloatingPointShortest);
    return true;
}

bool toDate(const QString &text, const QString &order, QString *out)
{
    static const QRegularExpression parts(QStringLiteral("^([0-9]{1,4})[-/. ]([0-9]{1,2}|[0-9]{4})[-/. ]([0-9]{1,4})$"));
    const QRegularExpressionMatch m = parts.match(text.trimmed());
    if (!m.hasMatch()) return false;
    int y = 0, mo = 0, d = 0;
    const QString a = m.captured(1), b = m.captured(2), c = m.captured(3);
    if (order == QLatin1String("YMD")) {
        if (a.size() != 4) return false;
        y = a.toInt(), mo = b.toInt(), d = c.toInt();
    } else {
        if (c.size() != 4) return false;
        y = c.toInt();
        if (order == QLatin1String("MDY")) mo = a.toInt(), d = b.toInt();
        else d = a.toInt(), mo = b.toInt();
    }
    const QDate date(y, mo, d);
    if (!date.isValid()) return false;
    *out = date.toString(Qt::ISODate);
    return true;
}

bool runKind(Engine &e, const DataStep &s)
{
    int c;
    if (!e.needColumn(s.col, &c)) return false;
    if (!e.budget(e.rows.rows.size())) return false;
    const bool number = s.kind == QLatin1String("number"), date = s.kind == QLatin1String("date");
    for (int i = 0; i < e.rows.rows.size(); ++i) {
        if ((i & 4095) == 0 && e.stopped()) {
            e.why = QCoreApplication::translate("jp::DataSteps", "The read was stopped.");
            return false;
        }
        DataCell cell = e.rows.rows[i].value(c);
        if (!blank(cell) && (number || date)) {
            QString converted;
            const bool ok = number ? toNumber(cell.text, s.decimal, s.group, &converted) : toDate(cell.text, s.order, &converted);
            if (!ok) {
                e.why = number ? QCoreApplication::translate("jp::DataSteps", "The value \"%1\" in row %2 of \"%3\" is not a number.").arg(cell.text.left(40)).arg(i + 1).arg(e.rows.header[c])
                               : QCoreApplication::translate("jp::DataSteps", "The value \"%1\" in row %2 of \"%3\" is not a date in that order.").arg(cell.text.left(40)).arg(i + 1).arg(e.rows.header[c]);
                return false;
            }
            cell.text = converted;
        }
        cell.kind = blank(cell) ? 0 : number ? 'n' : 's';
        if (cell.kind == 0) cell.text.clear();
        e.rows.rows[i].resize(std::max<int>(e.rows.rows[i].size(), c + 1));
        e.rows.rows[i][c] = cell;
    }
    e.forced.resize(e.rows.header.size());
    e.forced[c] = number ? 'n' : date ? 'd' : 't';
    return true;
}

}   // namespace

DataStepsRun applyDataSteps(const QVector<DataStep> &steps, DataRows *rows, const std::function<bool()> &cancelled, qint64 workLimit)
{
    DataStepsRun run;
    Engine e{*rows, QVector<char>(rows->header.size(), 0), cancelled};
    e.limit = workLimit;
    if (steps.size() > kMaxDataSteps) {
        run.why = QCoreApplication::translate("jp::DataSteps", "There are more steps than JeffCrunch runs.");
        return run;
    }
    for (int i = 0; i < steps.size(); ++i) {
        const DataStep &s = steps[i];
        bool ok = false;
        switch (s.op) {
        case DataStep::Op::Keep:
        case DataStep::Op::Remove: ok = runColumns(e, s); break;
        case DataStep::Op::Rename: ok = runRename(e, s); break;
        case DataStep::Op::Filter: ok = runFilter(e, s); break;
        case DataStep::Op::Sort: ok = runSort(e, s); break;
        case DataStep::Op::Kind: ok = runKind(e, s); break;
        }
        if (!ok) {
            run.failedStep = i;
            run.why = QCoreApplication::translate("jp::DataSteps", "Step %1 (%2): %3").arg(i + 1).arg(dataStepTitle(s), e.why);
            run.work = e.work;
            return run;
        }
    }
    e.forced.resize(rows->header.size());
    run.forcedKinds = e.forced;
    run.work = e.work;
    run.ok = true;
    return run;
}

}   // namespace jp::office::sheet
