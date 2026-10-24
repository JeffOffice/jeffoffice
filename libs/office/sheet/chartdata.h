#pragma once
// Charts that follow their cells. A chart in a workbook says where each series reads its name, categories, and values (the c:f of its parts: "Sheet1!$B$2:$B$13",
// "'My Sheet'!$A$1:$A$9", "(Sheet1!$A$2:$A$3,Sheet1!$A$6:$A$7)", a defined name, a whole column); the numbers the part also stores are what those cells held when the file was
// saved. This puts what the cells hold now in the chart model (libs/office/office/chart.h), and says which cells a chart reads, so that whoever shows it can draw it again when one
// of them changes. The drawing itself is chart.h's, as for presentations and documents.
//
// The cells are asked of a ChartCells, which the program gives (the window's reads the values its calculation has worked out), so no workbook is needed to test this. A reference it
// cannot resolve (another workbook, a sheet that is not there, a name that is a formula) leaves the series as the file stored it. What a stranger's chart can ask is bounded: a
// series reads at most kMaxChartSeriesCells cells and a chart kMaxChartCells, a reference has at most kMaxChartBlocks areas, and names are followed kMaxChartNameDepth deep.

#include "calc/value.h"
#include "office/chart.h"
#include "sheet/cellref.h"
#include "sheet/refshift.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <vector>

namespace jp::office::sheet {

constexpr int kMaxChartSeriesCells = 10000;   // as many points as chart.cpp keeps of a series
constexpr int kMaxChartCells = 100000;        // and of a chart
constexpr int kMaxChartBlocks = 1000;
constexpr int kMaxChartNameDepth = 4;

// What a chart reads of a workbook.
class ChartCells {
public:
    virtual ~ChartCells();
    virtual int sheetCount() const = 0;
    // The place of the sheet with this name (capitals and small letters alike), -1 for none.
    virtual int sheetIndex(const QString &name) const = 0;
    // The value of a cell as the sheet has it now (empty for a cell that is not there), and the text its number format makes of it.
    virtual jp::calc::Value value(int sheet, quint32 row, quint32 col) const = 0;
    virtual QString text(int sheet, quint32 row, quint32 col) const = 0;
    virtual bool rowHidden(int sheet, quint32 row) const = 0;
    virtual bool columnHidden(int sheet, quint32 col) const = 0;
    // The rows and columns the sheet's cells reach (a whole column is read as far as the rows go).
    virtual void extent(int sheet, quint32 *rows, quint32 *cols) const = 0;
    // What a defined name stands for, as written ("Sheet1!$A$1:$A$9"); empty if there is none. A name of the sheet itself is found before one of the workbook.
    virtual QString definedName(const QString &name, int sheet) const = 0;
};

struct ChartBlock {
    int sheet = 0;
    CellRange range;
};

// The blocks of cells a reference names, in the order it lists them. `ownSheet` is the sheet a reference with no sheet in it is on. False if none of it could be resolved (blocks
// that could are still given). A whole column or row is a block of all its cells; readChartRef reads it as far as the sheet's cells go.
bool chartBlocks(const QString &ref, int ownSheet, const ChartCells &cells, QVector<ChartBlock> *out);

// One cell of what a reference reads.
struct ChartPoint {
    bool empty = true;           // no cell, or an empty one
    bool isNumber = false;       // a number, a date, or true or false (as 1 or 0)
    double number = 0;
    bool isError = false;
    QString text;                // as the sheet shows it
    bool hidden = false;         // in a hidden row or column
};

// What a reference reads, cell by cell, in order: row by row within each block. False if it could not be resolved. `cut` says it was cut at the limits above.
struct ChartRefData {
    QVector<ChartPoint> points;
    qint64 rows = 0, cols = 0;   // of the first block
    bool cut = false;
};
bool readChartRef(const QString &ref, int ownSheet, const ChartCells &cells, int maxCells, ChartRefData *out);

// Every block the chart reads (its series' names, categories, and values, and its linked titles), for deciding whether a change to the cells is one it draws. Blocks of a reference that
// does not resolve are left out.
QVector<ChartBlock> chartReads(const Chart &chart, int ownSheet, const ChartCells &cells);

// The chart's references after the edits that insert and delete rows and columns, as saving rewrites the chart part's (so a chart follows its cells when they move): each reference of
// each series and linked title. `sheetNames` are the workbook's, in order.
void moveChartRefs(Chart *chart, const QStringList &sheetNames, const std::vector<StructEdit> &edits);

// Puts the cells' contents in the chart: for each series that has the reference, its name, categories (or a scatter's x values), and values, without the points of hidden rows and
// columns when the chart leaves them out (plotVisibleOnly); a point of an empty cell is a blank (NaN) and a text is 0. Linked titles take their cells' text. Returns the number of
// series whose values were read. A series whose reference does not resolve keeps what the file stored.
int applyChartCells(Chart *chart, int ownSheet, const ChartCells &cells);

} // namespace jp::office::sheet
