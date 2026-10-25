// A table's part read into the table's model and written from it again. A part is read into a small tree of elements for the writing (a table's part is short and its size is bounded), the
// model is put in the tree, and the tree is written: everything the program keeps no model of (the extension list, the columns' other attributes, the elements it does not know) is as the
// file had it. A part the person did not change is not written at all (Workbook::changedTableParts compares the signatures).
#include "sheet/tables.h"

#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

constexpr int kMaxPartNodes = 200000;
constexpr qint64 kMaxPartBytes = 16 << 20;
constexpr int kMaxTableColumns = 16384;
const char *const kMainNs = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

struct XNode {
    QString name;                                    // as written, with its prefix
    std::vector<std::pair<QString, QString>> attrs;  // in order, values unescaped
    std::vector<XNode> kids;
    QString text;

    QStringView local() const
    {
        const int colon = int(name.indexOf(QLatin1Char(':')));
        return QStringView(name).mid(colon + 1);
    }
    QString prefix() const
    {
        const int colon = int(name.indexOf(QLatin1Char(':')));
        return colon < 0 ? QString() : name.left(colon + 1);
    }
    const QString *attr(const char *key) const
    {
        for (const auto &a : attrs)
            if (a.first == QLatin1String(key)) return &a.second;
        return nullptr;
    }
    QString attrOr(const char *key, const QString &fallback = QString()) const
    {
        const QString *a = attr(key);
        return a ? *a : fallback;
    }
    void set(const char *key, const QString &value)
    {
        for (auto &a : attrs)
            if (a.first == QLatin1String(key)) {
                a.second = value;
                return;
            }
        attrs.emplace_back(QString::fromLatin1(key), value);
    }
    void drop(const char *key)
    {
        attrs.erase(std::remove_if(attrs.begin(), attrs.end(), [&](const auto &a) { return a.first == QLatin1String(key); }), attrs.end());
    }
    XNode *child(const char *localName)
    {
        for (XNode &k : kids)
            if (k.local() == QLatin1String(localName)) return &k;
        return nullptr;
    }
};

bool parseTree(const QByteArray &xml, XNode *root)
{
    if (xml.size() > kMaxPartBytes) return false;
    QXmlStreamReader r(xml);
    r.setNamespaceProcessing(false);
    std::vector<XNode *> stack;
    int count = 0;
    bool any = false;
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType tok = r.readNext();
        if (tok == QXmlStreamReader::StartElement) {
            if (++count > kMaxPartNodes) return false;
            XNode node;
            node.name = r.qualifiedName().toString();
            for (const QXmlStreamAttribute &a : r.attributes()) node.attrs.emplace_back(a.qualifiedName().toString(), a.value().toString());
            if (stack.empty()) {
                if (any) return false;
                *root = std::move(node);
                stack.push_back(root);
                any = true;
            } else {
                XNode *top = stack.back();
                top->kids.push_back(std::move(node));
                stack.push_back(&top->kids.back());
            }
        } else if (tok == QXmlStreamReader::EndElement) {
            if (stack.empty()) return false;
            stack.pop_back();
        } else if (tok == QXmlStreamReader::Characters && !stack.empty() && !r.isWhitespace()) {
            stack.back()->text += r.text().toString();
        }
    }
    return any && !r.hasError() && stack.empty();
}

void writeNode(const XNode &n, QByteArray *out)
{
    *out += "<" + n.name.toUtf8();
    for (const auto &a : n.attrs) *out += " " + a.first.toUtf8() + "=\"" + xmlscan::escapeAttribute(a.second) + "\"";
    if (n.kids.empty() && n.text.isEmpty()) {
        *out += "/>";
        return;
    }
    *out += ">";
    if (!n.text.isEmpty()) *out += xmlscan::escapeText(n.text);
    for (const XNode &k : n.kids) writeNode(k, out);
    *out += "</" + n.name.toUtf8() + ">";
}

// The elements in the order the format has them.
int orderOf(QStringView local)
{
    if (local == QLatin1String("autoFilter")) return 0;
    if (local == QLatin1String("sortState")) return 1;
    if (local == QLatin1String("tableColumns")) return 2;
    if (local == QLatin1String("tableStyleInfo")) return 3;
    if (local == QLatin1String("extLst")) return 4;
    return 2;
}

bool flag(const QString &v, bool fallback)
{
    if (v.isEmpty()) return fallback;
    return v == QLatin1String("1") || v.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
}

} // namespace

bool parseAutoFilterElement(const QByteArray &xml, AutoFilter *out)
{
    XNode root;
    if (!parseTree(xml, &root) || root.local() != QLatin1String("autoFilter")) return false;
    AutoFilter f;
    CellRange range;
    if (parseRangeRef(root.attrOr("ref"), &range)) f.ref = range;
    for (const XNode &fc : root.kids) {
        if (fc.local() == QLatin1String("sortState")) {
            QString sort = QStringLiteral("<sortState");
            for (const auto &a : fc.attrs)
                if (!a.first.contains(QLatin1Char(':'))) sort += QStringLiteral(" %1=\"%2\"").arg(a.first, a.second.toHtmlEscaped());
            sort += QLatin1Char('>');
            for (const XNode &sc : fc.kids) {
                if (sc.local() != QLatin1String("sortCondition")) continue;
                sort += QStringLiteral("<sortCondition");
                for (const auto &a : sc.attrs)
                    if (!a.first.contains(QLatin1Char(':'))) sort += QStringLiteral(" %1=\"%2\"").arg(a.first, a.second.toHtmlEscaped());
                sort += QStringLiteral("/>");
            }
            sort += QStringLiteral("</sortState>");
            f.sortState = sort;
            continue;
        }
        if (fc.local() != QLatin1String("filterColumn")) continue;
        FilterColumn c;
        bool good = false;
        const int id = fc.attrOr("colId").toInt(&good);
        if (!good || id < 0 || id >= 16384) continue;
        c.column = quint32(id);
        c.hideButton = flag(fc.attrOr("hiddenButton"), false) || (!fc.attrOr("showButton").isEmpty() && !flag(fc.attrOr("showButton"), true));
        for (const XNode &k : fc.kids) {
            const QStringView l = k.local();
            if (l == QLatin1String("filters")) {
                c.kind = FilterColumn::Kind::Values;
                c.blank = flag(k.attrOr("blank"), false);
                for (const XNode &v : k.kids) {
                    if (v.local() == QLatin1String("filter")) {
                        if (c.values.size() < 10000) c.values.append(v.attrOr("val").left(1000));
                    } else if (v.local() == QLatin1String("dateGroupItem") && c.dates.size() < 10000) {
                        FilterColumn::DateGroup g;
                        const QString by = v.attrOr("dateTimeGrouping");
                        using U = FilterColumn::DateGroup::Unit;
                        g.unit = by == QLatin1String("year") ? U::Year : by == QLatin1String("month") ? U::Month : by == QLatin1String("hour") ? U::Hour : by == QLatin1String("minute") ? U::Minute
                                 : by == QLatin1String("second") ? U::Second : U::Day;
                        g.year = v.attrOr("year").toInt();
                        g.month = v.attrOr("month").toInt();
                        g.day = v.attrOr("day").toInt();
                        g.hour = v.attrOr("hour").toInt();
                        g.minute = v.attrOr("minute").toInt();
                        g.second = v.attrOr("second").toInt();
                        c.dates.append(g);
                    }
                }
            } else if (l == QLatin1String("customFilters")) {
                c.kind = FilterColumn::Kind::Custom;
                c.both = flag(k.attrOr("and"), false);
                for (const XNode &v : k.kids) {
                    if (v.local() != QLatin1String("customFilter") || c.conditions.size() >= 2) continue;
                    FilterColumn::Condition cond;
                    const QString op = v.attrOr("operator");
                    using O = FilterColumn::Op;
                    cond.op = op == QLatin1String("notEqual") ? O::NotEqual : op == QLatin1String("greaterThan") ? O::Greater : op == QLatin1String("greaterThanOrEqual") ? O::GreaterEqual
                              : op == QLatin1String("lessThan") ? O::Less : op == QLatin1String("lessThanOrEqual") ? O::LessEqual : O::Equal;
                    cond.value = v.attrOr("val").left(255);
                    c.conditions.append(cond);
                }
            } else if (l == QLatin1String("top10")) {
                c.kind = FilterColumn::Kind::Top10;
                c.top = flag(k.attrOr("top"), true);
                c.percent = flag(k.attrOr("percent"), false);
                c.count = std::clamp(k.attrOr("val").toDouble(), 0.0, 1e6);
                if (k.attr("val") == nullptr) c.count = 10;
                c.hasLimit = !k.attrOr("filterVal").isEmpty();
                c.limit = k.attrOr("filterVal").toDouble();
            } else if (l == QLatin1String("dynamicFilter")) {
                c.kind = FilterColumn::Kind::Dynamic;
                c.dynamic = k.attrOr("type").left(40);
                c.hasDynVal = !k.attrOr("val").isEmpty();
                c.dynVal = k.attrOr("val").toDouble();
                c.dynMax = k.attrOr("maxVal").toDouble();
            } else if (l == QLatin1String("colorFilter")) {
                c.kind = FilterColumn::Kind::Color;
                bool dxfGood = false;
                const int dxf = k.attrOr("dxfId").toInt(&dxfGood);
                c.dxf = dxfGood ? dxf : -1;
                c.fontColor = !k.attrOr("cellColor").isEmpty() && !flag(k.attrOr("cellColor"), true);
            }
        }
        f.columns.append(c);
    }
    *out = f;
    return true;
}

bool readTableModel(const QByteArray &xml, Table *t)
{
    XNode root;
    if (!parseTree(xml, &root) || root.local() != QLatin1String("table")) return false;
    t->id = root.attrOr("id").toInt();
    t->filterButtons = false;
    t->filter = AutoFilter();
    t->styleName.clear();
    t->showFirstColumn = t->showLastColumn = t->showColumnStripes = false;
    t->showRowStripes = false;
    t->info.clear();
    int source = 0;
    for (const XNode &k : root.kids) {
        const QStringView l = k.local();
        if (l == QLatin1String("autoFilter")) {
            t->filterButtons = true;
            QByteArray bytes;
            writeNode(k, &bytes);
            AutoFilter f;
            if (parseAutoFilterElement(bytes, &f)) t->filter = f;
        } else if (l == QLatin1String("tableStyleInfo")) {
            t->styleName = k.attrOr("name");
            t->showFirstColumn = flag(k.attrOr("showFirstColumn"), false);
            t->showLastColumn = flag(k.attrOr("showLastColumn"), false);
            t->showRowStripes = flag(k.attrOr("showRowStripes"), false);
            t->showColumnStripes = flag(k.attrOr("showColumnStripes"), false);
        } else if (l == QLatin1String("tableColumns")) {
            for (const XNode &c : k.kids) {
                if (c.local() != QLatin1String("tableColumn")) continue;
                if (t->info.size() >= kMaxTableColumns) break;
                TableColumnInfo ci;
                ci.src = source++;
                ci.id = c.attrOr("id").toInt();
                ci.totalsFunction = c.attrOr("totalsRowFunction");
                if (ci.totalsFunction == QLatin1String("none")) ci.totalsFunction.clear();
                ci.totalsLabel = c.attrOr("totalsRowLabel");
                for (const XNode &f : c.kids) {
                    if (f.local() == QLatin1String("calculatedColumnFormula")) {
                        ci.calcFormula = f.text;
                        ci.calcArray = flag(f.attrOr("array"), false);
                    } else if (f.local() == QLatin1String("totalsRowFormula")) {
                        ci.totalsFormula = f.text;
                    }
                }
                t->info.append(ci);
            }
        }
    }
    // (as many as there are columns; a part whose columns the reader took in a different number is not trusted)
    while (t->info.size() < t->columns.size()) {
        TableColumnInfo ci;
        ci.id = t->info.size() + 1;
        t->info.append(ci);
    }
    while (t->info.size() > t->columns.size()) t->info.removeLast();
    return true;
}

namespace {
CellRange rangeOnly()
{
    CellRange r;
    r.valid = true;
    return r;
}
} // namespace

QByteArray tableSignature(const Table &t)
{
    QByteArray s;
    auto add = [&](const QString &v) { s += v.toUtf8() + '\x1f'; };
    auto num = [&](qint64 v) { s += QByteArray::number(v) + '\x1f'; };
    num(t.id);
    add(t.name);
    num(t.sheet);
    num(t.row1);
    num(t.col1);
    num(t.row2);
    num(t.col2);
    num(t.headerRows);
    num(t.totalsRows);
    add(t.styleName);
    num(t.showFirstColumn | (t.showLastColumn << 1) | (t.showRowStripes << 2) | (t.showColumnStripes << 3) | (t.filterButtons << 4));
    for (int i = 0; i < t.columns.size(); ++i) {
        add(t.columns[i]);
        if (i < t.info.size()) {
            const TableColumnInfo &c = t.info[i];
            num(c.id);
            add(c.totalsFunction);
            add(c.totalsLabel);
            add(c.totalsFormula);
            add(c.calcFormula);
            num(c.calcArray);
        }
    }
    // (the filter without its range, which follows the table's)
    AutoFilter f = t.filter;
    f.ref = rangeOnly();
    s += autoFilterElement(f, QByteArray());
    return s;
}

QByteArray writeTablePart(const Table &t, const QByteArray &original, bool *ok)
{
    if (ok) *ok = true;
    XNode root;
    const bool fresh = original.isEmpty();
    if (fresh) {
        root.name = QStringLiteral("table");
        root.attrs.emplace_back(QStringLiteral("xmlns"), QString::fromLatin1(kMainNs));
    } else if (!parseTree(original, &root) || root.local() != QLatin1String("table")) {
        if (ok) *ok = false;
        return {};
    }
    const QString prefix = root.prefix();
    auto nodeNamed = [&](const char *local) {
        XNode n;
        n.name = prefix + QString::fromLatin1(local);
        return n;
    };
    CellRange all;
    all.valid = true;
    all.row1 = t.row1;
    all.col1 = t.col1;
    all.row2 = t.row2;
    all.col2 = t.col2;
    root.set("id", QString::number(t.id));
    root.set("name", t.name);
    root.set("displayName", t.name);
    root.set("ref", rangeName(all));
    if (t.headerRows == 0) root.set("headerRowCount", QStringLiteral("0"));
    else root.drop("headerRowCount");
    if (t.totalsRows > 0) {
        root.set("totalsRowCount", QStringLiteral("1"));
        root.drop("totalsRowShown");
    } else {
        root.drop("totalsRowCount");
        root.set("totalsRowShown", QStringLiteral("0"));
    }
    // the elements the model owns are taken out and put back in the order the format has them
    std::vector<XNode> kept;
    XNode oldFilter, oldColumns, oldStyle;
    bool hadFilter = false, hadColumns = false, hadStyle = false;
    for (XNode &k : root.kids) {
        const QStringView l = k.local();
        if (l == QLatin1String("autoFilter")) {
            oldFilter = std::move(k);
            hadFilter = true;
        } else if (l == QLatin1String("tableColumns")) {
            oldColumns = std::move(k);
            hadColumns = true;
        } else if (l == QLatin1String("tableStyleInfo")) {
            oldStyle = std::move(k);
            hadStyle = true;
        } else {
            kept.push_back(std::move(k));
        }
    }
    // the filter: its range is the header row and the data (not the totals row); the conditions as the model has them
    XNode filterNode;
    bool haveFilter = false;
    if (t.filterButtons && t.headerRows > 0) {
        AutoFilter f = t.filter;
        f.ref = all;
        if (t.totalsRows > 0 && f.ref.row2 > f.ref.row1) --f.ref.row2;
        XNode parsed;
        if (parseTree(autoFilterElement(f, prefix.toLatin1()), &parsed)) {
            // (the filter the file had, whole, when it holds what the model does: only its range is put in)
            AutoFilter had;
            QByteArray oldBytes;
            if (hadFilter) writeNode(oldFilter, &oldBytes);
            if (hadFilter && parseAutoFilterElement(oldBytes, &had)) {
                AutoFilter a = had, b = t.filter;
                a.ref = b.ref = rangeOnly();
                if (autoFilterElement(a, QByteArray()) == autoFilterElement(b, QByteArray())) {
                    oldFilter.set("ref", rangeName(f.ref));
                    parsed = std::move(oldFilter);
                }
            }
            filterNode = std::move(parsed);
            haveFilter = true;
        }
    }
    // the columns
    XNode columns = hadColumns ? std::move(oldColumns) : nodeNamed("tableColumns");
    std::vector<XNode> oldCols;
    for (XNode &c : columns.kids)
        if (c.local() == QLatin1String("tableColumn")) oldCols.push_back(std::move(c));
    columns.kids.clear();
    std::vector<XNode> after;   // (what follows the columns inside, if anything)
    for (int i = 0; i < t.columns.size(); ++i) {
        const TableColumnInfo ci = i < t.info.size() ? t.info[i] : TableColumnInfo();
        XNode c;
        if (ci.src >= 0 && size_t(ci.src) < oldCols.size()) c = oldCols[size_t(ci.src)];
        else c = nodeNamed("tableColumn");
        c.set("id", QString::number(ci.id > 0 ? ci.id : i + 1));
        c.set("name", t.columns[i]);
        if (t.totalsRows > 0 && !ci.totalsFunction.isEmpty()) c.set("totalsRowFunction", ci.totalsFunction);
        else c.drop("totalsRowFunction");
        if (t.totalsRows > 0 && !ci.totalsLabel.isEmpty()) c.set("totalsRowLabel", ci.totalsLabel);
        else c.drop("totalsRowLabel");
        // the formulas: the model's, in the place of the file's (an unchanged formula keeps its element)
        XNode *calc = c.child("calculatedColumnFormula");
        if (!ci.calcFormula.isEmpty()) {
            if (!calc) {
                c.kids.insert(c.kids.begin(), nodeNamed("calculatedColumnFormula"));
                calc = &c.kids.front();
            }
            calc->text = ci.calcFormula;
            if (ci.calcArray) calc->set("array", QStringLiteral("1"));
            else calc->drop("array");
        } else if (calc) {
            c.kids.erase(c.kids.begin() + (calc - c.kids.data()));
        }
        XNode *total = c.child("totalsRowFormula");
        if (t.totalsRows > 0 && !ci.totalsFormula.isEmpty()) {
            if (!total) {
                c.kids.push_back(nodeNamed("totalsRowFormula"));
                total = &c.kids.back();
            }
            total->text = ci.totalsFormula;
        } else if (total) {
            c.kids.erase(c.kids.begin() + (total - c.kids.data()));
        }
        columns.kids.push_back(std::move(c));
    }
    columns.set("count", QString::number(t.columns.size()));
    // the style
    XNode style = hadStyle ? std::move(oldStyle) : nodeNamed("tableStyleInfo");
    if (!t.styleName.isEmpty()) style.set("name", t.styleName);
    else style.drop("name");
    style.set("showFirstColumn", t.showFirstColumn ? QStringLiteral("1") : QStringLiteral("0"));
    style.set("showLastColumn", t.showLastColumn ? QStringLiteral("1") : QStringLiteral("0"));
    style.set("showRowStripes", t.showRowStripes ? QStringLiteral("1") : QStringLiteral("0"));
    style.set("showColumnStripes", t.showColumnStripes ? QStringLiteral("1") : QStringLiteral("0"));
    // put together in the format's order
    std::vector<XNode> all2;
    if (haveFilter) all2.push_back(std::move(filterNode));
    std::vector<XNode> before, behind;
    for (XNode &k : kept) (orderOf(k.local()) < 2 ? before : behind).push_back(std::move(k));
    for (XNode &k : before) all2.push_back(std::move(k));
    all2.push_back(std::move(columns));
    all2.push_back(std::move(style));
    for (XNode &k : behind) all2.push_back(std::move(k));
    root.kids = std::move(all2);
    QByteArray out = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    writeNode(root, &out);
    return out;
}

// ---- names

bool validTableName(const QString &name, QString *why)
{
    auto fail = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (name.isEmpty()) return fail(QCoreApplication::translate("jp::office::sheet::Tables", "A table needs a name. Type one."));
    if (name.size() > kMaxTableNameChars) return fail(QCoreApplication::translate("jp::office::sheet::Tables", "A table's name can have up to 255 characters. Shorten it."));
    const QChar first = name[0];
    if (!(first.isLetter() || first == QLatin1Char('_') || first == QLatin1Char('\\')))
        return fail(QCoreApplication::translate("jp::office::sheet::Tables", "A table's name begins with a letter, an underscore, or a backslash."));
    for (const QChar c : name)
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\')))
            return fail(QCoreApplication::translate("jp::office::sheet::Tables", "A table's name can hold letters, digits, underscores, and periods, but no spaces or other marks."));
    // a name that reads as a cell (A1, XFD1048576) or as a row or column in R1C1 form (R, C, R2C3) would be taken for one
    static const QRegularExpression cell(QStringLiteral("^\\$?[A-Za-z]{1,3}\\$?[0-9]+$")), r1c1(QStringLiteral("^[Rr](\\[?-?\\d*\\]?)([Cc](\\[?-?\\d*\\]?))?$|^[Cc](\\[?-?\\d*\\]?)$"));
    CellRange probe;
    if ((cell.match(name).hasMatch() && parseRangeRef(name, &probe)) || r1c1.match(name).hasMatch())
        return fail(QCoreApplication::translate("jp::office::sheet::Tables", "That name looks like a cell address. Pick another."));
    return true;
}

QString freshTableColumnName(const QStringList &columns, int from)
{
    auto taken = [&](const QString &candidate) {
        for (const QString &c : columns)
            if (c.compare(candidate, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    for (int n = std::max(1, from);; ++n) {
        const QString candidate = QCoreApplication::translate("jp::office::sheet::Tables", "Column") + QString::number(n);
        if (!taken(candidate)) return candidate;
    }
}

QString freshTableName(const QStringList &taken, const QString &stem)
{
    const QString base = stem.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Tables", "Table") : stem;
    auto has = [&](const QString &candidate) {
        for (const QString &t : taken)
            if (t.compare(candidate, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    for (int n = 1;; ++n) {
        const QString candidate = base + QString::number(n);
        if (!has(candidate)) return candidate;
    }
}

const std::vector<TotalsFunction> &totalsFunctions()
{
    static const std::vector<TotalsFunction> list = {{"average", 101}, {"count", 103}, {"countNums", 102}, {"max", 104}, {"min", 105}, {"sum", 109}, {"stdDev", 107}, {"var", 110}};
    return list;
}

QString totalsFormulaOf(const Table &t, int column)
{
    if (column < 0 || column >= t.columns.size() || column >= t.info.size()) return {};
    const TableColumnInfo &c = t.info[column];
    if (c.totalsFunction == QLatin1String("custom")) return c.totalsFormula;
    for (const TotalsFunction &f : totalsFunctions())
        if (c.totalsFunction == QLatin1String(f.word)) {
            const QString name = escapeTableColumn(t.columns[column]);
            return QStringLiteral("SUBTOTAL(%1,%2%3)").arg(f.number).arg(t.name, tableColumnNeedsDoubleBrackets(t.columns[column]) ? QStringLiteral("[[") + name + QStringLiteral("]]") : QStringLiteral("[") + name + QStringLiteral("]"));
        }
    return {};
}

} // namespace jp::office::sheet
