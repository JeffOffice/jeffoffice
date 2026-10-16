#pragma once
// A table of the file in the story. The story has the paragraphs of the table's cells, each with the key of its cell's record (flow::fp::Cell, see
// libs/flow/flow/tabledata.h); what a record says that the layout needs is read from the file here, and what the file says beyond that is kept with the
// record (flow::TableData::app, RowData::app, CellData::app: the parts below) to be written where it was: the start of the table, a row, and a cell with
// the properties, the grid, and the elements between them that the program has no use for. Writing the story gives the table back as the same blocks
// (office::wml::Block, an Open and a Close for the table, each row, and each cell), which the file writer turns into XML; a table that is as it was
// opened is written from the bytes it was read from.

#include "flow/flowdocument.h"
#include "wml/wml.h"

#include <QByteArray>
#include <QVector>
#include <memory>

namespace jp::write {

class WriteDoc;

struct TablePart {
    QByteArray open;                                    // the Open block's bytes: the start tag, tblPr, tblGrid
    std::shared_ptr<const office::wml::TableMark> mark; // what was read (the grid's place in `open`)
    QByteArray original, fingerprint;                   // the table as it was read, and how its blocks came out when it was opened (empty: write the blocks)
    QVector<office::wml::Block> before, after;          // kept pieces before its start (a content control's), and between its last row and its end
};

struct RowPart {
    QByteArray open;
    QVector<office::wml::Block> before, after;          // kept pieces between the table's rows before it, and between its last cell and its end
};

struct CellPart {
    QByteArray open;
    QVector<office::wml::Block> before, tail;           // kept pieces between the row's cells before it, and after its last paragraph or table
};

// Makes the records of a table's cells in the document as the blocks of the file's tables come (see wml.h for how a table is blocks).
class TableBuilder {
public:
    TableBuilder(flow::FlowDocument *flow, int compatMode) : m_flow(flow), m_compat(compatMode) {}
    // Whether the block is a part of a table that is the builder's to take: an Open or a Close of a table, a row, or a cell, and what is kept between
    // the parts of a table (that is not in a cell).
    bool isStructural(const office::wml::Block &blk) const;
    // Takes it. `pending` are the kept pieces that waited for a paragraph; the ones before a table's start go with the table.
    void take(const office::wml::Block &blk, QVector<office::wml::Block> *pending);
    // The key of the cell the next paragraph is in (0: none).
    int key() const { return m_cells.isEmpty() ? 0 : m_cells.last(); }
    // Pieces that come after a cell's last paragraph when that is not a paragraph of its own (the cell ends in a table), kept with the cell.
    void addTail(const QVector<office::wml::Block> &pieces);

private:
    struct Level {
        enum Kind { Table, Row, Cell } kind = Table;
        std::shared_ptr<flow::TableData> table;
        std::shared_ptr<flow::RowData> row;
        std::shared_ptr<TablePart> tablePart;
        std::shared_ptr<RowPart> rowPart;
        std::shared_ptr<CellPart> cellPart;
        int rows = 0;
        QVector<double> firstRow;                       // (a table: the widths its first row's cells say, for a grid the file left out)
    };
    flow::FlowDocument *m_flow;
    int m_compat;
    QVector<Level> m_levels;
    QVector<int> m_cells;                               // the keys of the cells open, outermost first
    QVector<office::wml::Block> m_between;              // kept pieces seen between a table's rows or a row's cells
};

// Makes the blocks of the tables again from the story: for each paragraph, what it needs closed and opened of the cells, rows, and tables around it.
class TableExporter {
public:
    explicit TableExporter(const flow::FlowDocument *flow) : m_flow(flow) {}
    // Appends to `out` the blocks that close and open the parts of tables between the cell the last paragraph was in and the one of `key` (0: none).
    void moveTo(int key, QVector<office::wml::Block> *out);
    void finish(QVector<office::wml::Block> *out) { moveTo(0, out); }
    // The tables that were opened: where their Open block is in `out`, and the table's part.
    struct Opened {
        qsizetype at = 0;
        TablePart *part = nullptr;
    };
    const QVector<Opened> &opened() const { return m_opened; }

private:
    const flow::FlowDocument *m_flow;
    QVector<const flow::CellRecord *> m_open;
    QVector<Opened> m_opened;
};

} // namespace jp::write
