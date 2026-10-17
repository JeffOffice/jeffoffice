#pragma once
// What a table in a flowing document is made of. A table is the paragraphs of its cells in the story, one after another (a row's cells, a table's
// rows, a nested table inside the cell it is in), and each of those paragraphs has the key of its cell's record in its block format (fp::Cell). A
// record says which table and row the cell is in (those are shared by all the cells of a row or a table) and how it looks: so the structure is in
// the paragraphs' formats, where undo restores it with the text, and a new paragraph made in a cell (Enter) is in the cell without more ado.
//
// Records are made and never changed or taken away while the document lives, so a key stays good through undo and redo; a change to a cell (its
// shading, its width) is a new record that its paragraphs are given. Rows and tables are told apart by their records' addresses, cells by their keys.

#include <QColor>
#include <QMarginsF>
#include <QVector>
#include <memory>
#include <optional>
#include <vector>

namespace jp::flow {

// One edge of a table or of a cell.
struct TableLine {
    enum class Style : quint8 { None, Single, Double, Dotted, Dashed, Triple };
    Style style = Style::None;
    double width = 0;       // points, of each line
    QColor color;           // invalid: automatic (black)
    bool visible() const { return style != Style::None && width > 0; }
    double extent() const { return style == Style::Double ? width * 3 : style == Style::Triple ? width * 5 : width; }   // all it takes: the lines and the spaces between them
    // Which of two lines at an edge is the line: the one of the greater weight, a line's width times the number of lines it is drawn with.
    double weight() const { return !visible() ? 0.0 : style == Style::Double ? width * 2 : style == Style::Triple ? width * 3 : width; }
    bool operator==(const TableLine &o) const { return style == o.style && width == o.width && color == o.color; }
    bool operator!=(const TableLine &o) const { return !(*this == o); }
};

enum class CellAlign : quint8 { Top, Center, Bottom };

// What all the cells of a table share. Lengths are in points.
struct TableData {
    enum class Align : quint8 { Left, Center, Right };
    QVector<double> columns;        // the grid: each column's width
    Align align = Align::Left;
    double indent = 0;              // from the text area's edge to the table's left edge, or to the first cell's text (indentToText)
    bool indentToText = false;      // the indent is to the text of the first cell of the first row: the edge is left of it by that cell's margin
    double spacing = 0;             // between cells
    bool spaced = false;            // the table says it has space between its cells (even none): its lines are a frame round all of them, not the cells' own
    QMarginsF margins{5.4, 0, 5.4, 0};   // the cells' own space inside (left, top, right, bottom), unless a cell says
    TableLine top, left, bottom, right, insideH, insideV;   // the table's borders: the cells on its edges take the first four, those between cells the others
    std::optional<QColor> fill;
    // A table set at a place on the page, with the text going round it (a word processor's floating table), instead of among the paragraphs: its rows are drawn there, as one
    // block that the page does not break, at the paragraph after the table; none for a table in the text. A position is from the text (the column across, the paragraph down),
    // the margin, or the page, by an offset or at the start, middle, or end; `room` is what the text keeps from it (left, top, right, bottom).
    struct Place {
        enum class From : quint8 { Text, Margin, Page };
        enum class Spec : quint8 { None, Start, Center, End };
        From across = From::Text, down = From::Text;
        Spec acrossSpec = Spec::None, downSpec = Spec::None;
        double x = 0, y = 0;
        QMarginsF room;
        bool overlap = true;        // it may be over another floating table (else it goes down from one)
    };
    std::optional<Place> place;
    std::shared_ptr<const void> app;    // what the program that made the table keeps with it (the file's own properties of it)
};

struct RowData {
    double height = 0;              // 0: as tall as its cells need
    bool exact = false;             // the height is exactly that, whatever the cells need; else at least
    bool cantSplit = false;
    bool header = false;            // repeated at the top of each page the table goes on to
    int before = 0, after = 0;      // grid columns left empty before the first cell and after the last
    std::shared_ptr<const void> app;
};

struct CellData {
    enum class VMerge : quint8 { None, Restart, Continue };
    int span = 1;                   // grid columns it covers
    VMerge vmerge = VMerge::None;   // a cell that continues the one above it has no text of its own to show and is drawn as part of it
    std::optional<QMarginsF> margins;   // its own; else the table's
    std::optional<TableLine> top, left, bottom, right;   // its own borders (a line of no style says none); else the table's
    std::optional<QColor> fill;     // its own (a transparent one says none); else the table's, if any
    CellAlign valign = CellAlign::Top;
    bool noWrap = false;
    std::shared_ptr<const void> app;
};

struct CellRecord {
    int key = 0;                    // set by the store
    std::shared_ptr<const TableData> table;
    std::shared_ptr<const RowData> row;
    CellData cell;
    int parent = 0;                 // the key of the cell this table is in, when it is nested (0: it is not)
};

// The records of a document, by key (keys are 1 and up; 0 is no cell).
class TableStore {
public:
    int add(CellRecord record)
    {
        record.key = int(m_cells.size()) + 1;
        m_cells.push_back(std::make_unique<const CellRecord>(std::move(record)));
        return int(m_cells.size());
    }
    const CellRecord *get(int key) const { return key >= 1 && size_t(key) <= m_cells.size() ? m_cells[size_t(key) - 1].get() : nullptr; }
    bool isEmpty() const { return m_cells.empty(); }
    int count() const { return int(m_cells.size()); }

private:
    std::vector<std::unique_ptr<const CellRecord>> m_cells;
};

} // namespace jp::flow
