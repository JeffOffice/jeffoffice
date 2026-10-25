#include "sheet/tablestyle.h"

#include <QCoreApplication>

#include <algorithm>
#include <memory>

namespace jp::office::sheet {

namespace {

// ---- colors and lines

ColorSpec theme(int index, double tint = 0)
{
    ColorSpec c;
    c.kind = ColorSpec::Kind::Theme;
    c.index = index;
    c.tint = tint;
    return c;
}

// The strong color of a color number (0: black, 1 to 6: the theme's accents), and a pale one of it (`amount`: how far to white).
ColorSpec strong(int k) { return k == 0 ? theme(1, 0.25) : theme(3 + k); }
ColorSpec pale(int k, double amount) { return k == 0 ? theme(0, -0.25 * (1 - amount)) : theme(3 + k, amount); }
ColorSpec deep(int k) { return k == 0 ? theme(1) : theme(3 + k, -0.25); }
ColorSpec white() { return theme(0); }

FillSpec solid(const ColorSpec &c)
{
    FillSpec f;
    f.pattern = PatternKind::Solid;
    f.fg = c;
    f.bg = c;
    return f;
}

BorderSide line(BorderStyle style, const ColorSpec &c)
{
    BorderSide s;
    s.style = style;
    s.color = c;
    return s;
}

void fill(DifferentialFormat &d, const ColorSpec &c)
{
    d.hasFill = true;
    d.fill = solid(c);
}

void textColor(DifferentialFormat &d, const ColorSpec &c) { d.fontColor = c; }

void borders(DifferentialFormat &d, const BorderSide &left, const BorderSide &right, const BorderSide &top, const BorderSide &bottom)
{
    d.hasBorder = true;
    d.border.left = left;
    d.border.right = right;
    d.border.top = top;
    d.border.bottom = bottom;
}

void topBottom(DifferentialFormat &d, const BorderSide &top, const BorderSide &bottom) { borders(d, BorderSide(), BorderSide(), top, bottom); }
void bottomOnly(DifferentialFormat &d, const BorderSide &bottom) { borders(d, BorderSide(), BorderSide(), BorderSide(), bottom); }
void topOnly(DifferentialFormat &d, const BorderSide &top) { borders(d, BorderSide(), BorderSide(), top, BorderSide()); }
void allSides(DifferentialFormat &d, const BorderSide &side) { borders(d, side, side, side, side); }

// One built-in style: `family` 0 Light, 1 Medium, 2 Dark; `group` the way it looks inside the family; `k` its color.
TableStyle makeStyle(const QString &name, int family, int group, int k)
{
    TableStyle st;
    st.name = name;
    const BorderStyle thin = BorderStyle::Thin;
    if (family == 0) {
        if (group == 0) {
            // light lines above and below, a bold header with a line under it, pale stripes
            topBottom(st.part(TablePart::WholeTable), line(thin, strong(k)), line(thin, strong(k)));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            bottomOnly(h, line(thin, strong(k)));
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            topOnly(t, line(BorderStyle::Double, strong(k)));
            fill(st.part(TablePart::FirstRowStripe), pale(k, 0.8));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.8));
            st.part(TablePart::FirstColumn).bold = true;
            st.part(TablePart::LastColumn).bold = true;
        } else if (group == 1) {
            // a filled header, lines between the rows
            topBottom(st.part(TablePart::WholeTable), line(thin, strong(k)), line(thin, strong(k)));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            fill(h, strong(k));
            textColor(h, white());
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            topOnly(t, line(BorderStyle::Double, strong(k)));
            topBottom(st.part(TablePart::FirstRowStripe), line(thin, pale(k, 0.6)), line(thin, pale(k, 0.6)));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.8));
            st.part(TablePart::FirstColumn).bold = true;
            st.part(TablePart::LastColumn).bold = true;
        } else {
            // a grid
            allSides(st.part(TablePart::WholeTable), line(thin, strong(k)));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            allSides(h, line(thin, strong(k)));
            h.border.bottom = line(BorderStyle::Medium, strong(k));
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            allSides(t, line(thin, strong(k)));
            t.border.top = line(BorderStyle::Medium, strong(k));
            fill(st.part(TablePart::FirstRowStripe), pale(k, 0.8));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.8));
            st.part(TablePart::FirstColumn).bold = true;
            st.part(TablePart::LastColumn).bold = true;
        }
    } else if (family == 1) {
        if (group == 0) {
            topBottom(st.part(TablePart::WholeTable), line(thin, strong(k)), line(thin, strong(k)));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            bottomOnly(h, line(BorderStyle::Medium, strong(k)));
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            topOnly(t, line(BorderStyle::Medium, strong(k)));
            fill(st.part(TablePart::FirstRowStripe), pale(k, 0.8));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.8));
            st.part(TablePart::FirstColumn).bold = true;
            st.part(TablePart::LastColumn).bold = true;
        } else if (group == 1) {
            allSides(st.part(TablePart::WholeTable), line(thin, pale(k, 0.4)));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            fill(h, strong(k));
            textColor(h, white());
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            topOnly(t, line(BorderStyle::Medium, strong(k)));
            fill(st.part(TablePart::FirstRowStripe), pale(k, 0.8));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.8));
            st.part(TablePart::FirstColumn).bold = true;
            st.part(TablePart::LastColumn).bold = true;
        } else if (group == 2) {
            DifferentialFormat &w = st.part(TablePart::WholeTable);
            fill(w, pale(k, 0.8));
            allSides(w, line(thin, white()));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            fill(h, strong(k));
            textColor(h, white());
            bottomOnly(h, line(BorderStyle::Medium, white()));
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            fill(t, pale(k, 0.8));
            topOnly(t, line(BorderStyle::Medium, white()));
            fill(st.part(TablePart::FirstRowStripe), pale(k, 0.6));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.6));
            DifferentialFormat &fc = st.part(TablePart::FirstColumn);
            fc.bold = true;
            fill(fc, strong(k));
            textColor(fc, white());
            DifferentialFormat &lc = st.part(TablePart::LastColumn);
            lc.bold = true;
            fill(lc, strong(k));
            textColor(lc, white());
        } else {
            allSides(st.part(TablePart::WholeTable), line(thin, pale(k, 0.4)));
            DifferentialFormat &h = st.part(TablePart::HeaderRow);
            h.bold = true;
            fill(h, pale(k, 0.6));
            DifferentialFormat &t = st.part(TablePart::TotalRow);
            t.bold = true;
            fill(t, pale(k, 0.6));
            fill(st.part(TablePart::FirstRowStripe), pale(k, 0.8));
            fill(st.part(TablePart::FirstColumnStripe), pale(k, 0.8));
            st.part(TablePart::FirstColumn).bold = true;
            st.part(TablePart::LastColumn).bold = true;
        }
    } else {
        DifferentialFormat &w = st.part(TablePart::WholeTable);
        fill(w, k == 0 ? theme(1, 0.35) : strong(k));
        textColor(w, white());
        DifferentialFormat &h = st.part(TablePart::HeaderRow);
        h.bold = true;
        fill(h, k == 0 ? theme(1) : deep(k));
        textColor(h, white());
        bottomOnly(h, line(BorderStyle::Medium, white()));
        DifferentialFormat &t = st.part(TablePart::TotalRow);
        t.bold = true;
        fill(t, k == 0 ? theme(1) : deep(k));
        textColor(t, white());
        topOnly(t, line(BorderStyle::Medium, white()));
        DifferentialFormat &s1 = st.part(TablePart::FirstRowStripe);
        fill(s1, k == 0 ? theme(1, 0.25) : theme(3 + k, -0.1));
        fill(st.part(TablePart::FirstColumnStripe), k == 0 ? theme(1, 0.25) : theme(3 + k, -0.1));
        DifferentialFormat &fc = st.part(TablePart::FirstColumn);
        fc.bold = true;
        fill(fc, k == 0 ? theme(1) : deep(k));
        DifferentialFormat &lc = st.part(TablePart::LastColumn);
        lc.bold = true;
        fill(lc, k == 0 ? theme(1) : deep(k));
    }
    return st;
}

struct Builtins {
    std::vector<TableStyle> styles;
    QStringList names;
    QVector<TableStyleGroup> groups;
    Builtins()
    {
        // Light and Medium: 7 colors in each group (black, then the six accents); Dark: 7 of the first group, then 4 of a second
        struct Family {
            const char *word;
            int groups;
            int count;
        };
        const Family families[] = {{"Light", 3, 21}, {"Medium", 4, 28}, {"Dark", 2, 11}};
        for (int f = 0; f < 3; ++f) {
            TableStyleGroup g;
            g.title = f == 0 ? QCoreApplication::translate("jp::office::sheet::TableStyles", "Light") : f == 1 ? QCoreApplication::translate("jp::office::sheet::TableStyles", "Medium")
                                                                                                     : QCoreApplication::translate("jp::office::sheet::TableStyles", "Dark");
            for (int n = 1; n <= families[f].count; ++n) {
                const QString name = QStringLiteral("TableStyle%1%2").arg(QLatin1String(families[f].word)).arg(n);
                const int group = f == 2 ? (n <= 7 ? 0 : 1) : (n - 1) / 7;
                const int k = f == 2 ? (n <= 7 ? n - 1 : n - 8) : (n - 1) % 7;
                styles.push_back(makeStyle(name, f, group, k));
                names << name;
                g.names << name;
            }
            groups.push_back(g);
        }
    }
};

const Builtins &builtins()
{
    static const Builtins b;
    return b;
}

// The place of a part among those that apply, lowest first.
bool isRowStripe(TablePart p) { return p == TablePart::FirstRowStripe || p == TablePart::SecondRowStripe; }

} // namespace

QStringList builtinTableStyleNames() { return builtins().names; }

const TableStyle *builtinTableStyle(const QString &name)
{
    const Builtins &b = builtins();
    for (size_t i = 0; i < b.styles.size(); ++i)
        if (b.styles[i].name.compare(name, Qt::CaseInsensitive) == 0) return &b.styles[i];
    return nullptr;
}

QVector<TableStyleGroup> tableStyleGroups() { return builtins().groups; }

void TableStyleBook::setCustom(const QVector<CustomTableStyle> &custom, const QVector<DifferentialFormat> &dxfs)
{
    m_custom.clear();
    for (const CustomTableStyle &c : custom) {
        if (int(m_custom.size()) >= kMaxTableStyles) break;
        TableStyle st;
        st.name = c.name;
        for (const CustomTableStyle::Part &p : c.parts) {
            if (p.dxf < 0 || p.dxf >= dxfs.size()) continue;
            st.part(p.part) = dxfs[p.dxf];
            if (p.part == TablePart::FirstRowStripe || p.part == TablePart::SecondRowStripe) st.rowStripeSize = std::clamp(p.size, 1, 9);
            if (p.part == TablePart::FirstColumnStripe || p.part == TablePart::SecondColumnStripe) st.columnStripeSize = std::clamp(p.size, 1, 9);
        }
        m_custom.push_back(std::move(st));
    }
}

const TableStyle *TableStyleBook::find(const QString &name) const
{
    for (const TableStyle &s : m_custom)
        if (s.name.compare(name, Qt::CaseInsensitive) == 0) return &s;
    return builtinTableStyle(name);
}

int tablePartsAt(const Table &t, quint32 row, quint32 col, TablePart *out)
{
    if (row < t.row1 || row > t.row2 || col < t.col1 || col > t.col2) return 0;
    int n = 0;
    out[n++] = TablePart::WholeTable;
    const bool header = t.headerRows > 0 && row == t.row1;
    const bool total = t.totalsRows > 0 && row == t.row2 && !header;
    const quint32 firstData = t.row1 + quint32(t.headerRows);
    const bool data = !header && !total;
    if (data && t.showColumnStripes) out[n++] = ((col - t.col1) % 2 == 0) ? TablePart::FirstColumnStripe : TablePart::SecondColumnStripe;
    if (data && t.showRowStripes) out[n++] = ((row - firstData) % 2 == 0) ? TablePart::FirstRowStripe : TablePart::SecondRowStripe;
    if (t.showFirstColumn && col == t.col1) out[n++] = TablePart::FirstColumn;
    if (t.showLastColumn && col == t.col2) out[n++] = TablePart::LastColumn;
    if (header) out[n++] = TablePart::HeaderRow;
    if (total) out[n++] = TablePart::TotalRow;
    if (header && col == t.col1) out[n++] = TablePart::FirstHeaderCell;
    if (header && col == t.col2) out[n++] = TablePart::LastHeaderCell;
    if (total && col == t.col1) out[n++] = TablePart::FirstTotalCell;
    if (total && col == t.col2) out[n++] = TablePart::LastTotalCell;
    return n;
}

DifferentialFormat mergedDifferential(const DifferentialFormat &base, const DifferentialFormat &over)
{
    DifferentialFormat d = base;
    if (over.bold) d.bold = over.bold;
    if (over.italic) d.italic = over.italic;
    if (over.strike) d.strike = over.strike;
    if (over.underline) d.underline = over.underline;
    if (over.fontColor.isSet()) d.fontColor = over.fontColor;
    if (over.hasFill) {
        d.hasFill = true;
        d.fill = over.fill;
    }
    if (over.hasBorder) {
        if (!d.hasBorder) d.border = BorderSpec();
        d.hasBorder = true;
        auto side = [](BorderSide *to, const BorderSide &from) {
            if (from.style != BorderStyle::None) *to = from;
        };
        side(&d.border.left, over.border.left);
        side(&d.border.right, over.border.right);
        side(&d.border.top, over.border.top);
        side(&d.border.bottom, over.border.bottom);
    }
    if (over.numberFormat) d.numberFormat = over.numberFormat;
    return d;
}

DifferentialFormat tableLookAt(const TableStyle &style, const Table &t, quint32 row, quint32 col, quint32 *key)
{
    TablePart parts[16];
    const int n = tablePartsAt(t, row, col, parts);
    DifferentialFormat d;
    quint32 mask = 0;
    for (int i = 0; i < n; ++i) {
        const TablePart p = parts[i];
        if (!style.has[size_t(p)]) continue;
        // (stripes of more than one row: the stripe a row is in)
        if (isRowStripe(p) && style.rowStripeSize > 1) {
            const quint32 firstData = t.row1 + quint32(t.headerRows);
            const bool first = ((row - firstData) / quint32(style.rowStripeSize)) % 2 == 0;
            if (first != (p == TablePart::FirstRowStripe)) continue;
        }
        if ((p == TablePart::FirstColumnStripe || p == TablePart::SecondColumnStripe) && style.columnStripeSize > 1) {
            const bool first = ((col - t.col1) / quint32(style.columnStripeSize)) % 2 == 0;
            if (first != (p == TablePart::FirstColumnStripe)) continue;
        }
        d = mergedDifferential(d, style.parts[size_t(p)]);
        mask |= 1u << unsigned(p);
    }
    if (key) *key = mask;
    return d;
}

} // namespace jp::office::sheet
