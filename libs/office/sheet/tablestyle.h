#pragma once
// How a table looks: a table style says, for each part of a table (the whole of it, its header row, its totals row, its first and last column, the stripes of its rows and of its columns,
// and its corner cells), what changes in the look of the cells there (a differential format, as conditional formatting has). The file names a style ("TableStyleMedium2"); the programs
// that read it all draw the built-in names, so the names are drawn here from the rules below: the built-in names are made from a group (what the style does) and a color (black, or one
// of the theme's six accents), in our own colors and lines. A style a workbook defines for itself (its styles part's `tableStyles`) is read as it says.

#include "sheet/styles.h"
#include "sheet/tables.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <array>

namespace jp::office::sheet {

enum class TablePart : quint8 {
    WholeTable, HeaderRow, TotalRow, FirstColumn, LastColumn, FirstRowStripe, SecondRowStripe, FirstColumnStripe, SecondColumnStripe,
    FirstHeaderCell, LastHeaderCell, FirstTotalCell, LastTotalCell, Count
};

struct TableStyle {
    QString name;
    std::array<DifferentialFormat, size_t(TablePart::Count)> parts;
    std::array<bool, size_t(TablePart::Count)> has{};      // the style says something for the part
    int rowStripeSize = 1, columnStripeSize = 1;            // rows (columns) in one stripe
    DifferentialFormat &part(TablePart p)
    {
        has[size_t(p)] = true;
        return parts[size_t(p)];
    }
};

// A table style the workbook defines (the file's `tableStyle`): the parts as the file names them, each the place of a differential format of the styles part.
struct CustomTableStyle {
    QString name;
    struct Part {
        TablePart part = TablePart::WholeTable;
        qint32 dxf = -1;
        int size = 1;
    };
    QVector<Part> parts;
};

constexpr int kMaxTableStyles = 1000;

// The built-in styles, by name, in the order of the gallery: Light, then Medium, then Dark. A name that is none of them gives null.
QStringList builtinTableStyleNames();
const TableStyle *builtinTableStyle(const QString &name);
// The gallery's groups: for each, the title shown over it and the names in it, in order.
struct TableStyleGroup {
    QString title;
    QStringList names;
};
QVector<TableStyleGroup> tableStyleGroups();
// The style a name stands for, from the workbook's own styles if it defines one by that name, else the built-in; null if neither. `dxfs` are the workbook's differential formats.
class TableStyleBook {
public:
    void setCustom(const QVector<CustomTableStyle> &custom, const QVector<DifferentialFormat> &dxfs);
    const TableStyle *find(const QString &name) const;
    int customCount() const { return int(m_custom.size()); }
private:
    std::vector<TableStyle> m_custom;
};

// The part of the table the cell at (row, col) is in, as the parts that apply, lowest first (the whole table, the stripes, the first and last column, the header or totals row, the corner
// cells); `out` has room for them all, and the number is returned. A cell outside the table has none.
int tablePartsAt(const Table &table, quint32 row, quint32 col, TablePart *out);
// What the style changes in the cell at (row, col): the parts that apply put over one another (a later one overrides what an earlier says). Empty for a cell outside the table, or a style with
// nothing to say there. `key` (if given) is a number for the combination of parts, the same for cells that get the same parts, so that a look made from it can be kept.
DifferentialFormat tableLookAt(const TableStyle &style, const Table &table, quint32 row, quint32 col, quint32 *key = nullptr);
// Puts `over` over `base`: each thing `over` sets replaces the base's (a border by its sides).
DifferentialFormat mergedDifferential(const DifferentialFormat &base, const DifferentialFormat &over);

} // namespace jp::office::sheet
