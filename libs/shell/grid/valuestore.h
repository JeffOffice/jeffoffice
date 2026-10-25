#pragma once
// The values a calculation has worked out, as the window holds them for the grid to show: by sheet and cell. The calculation runs on a worker thread and tells what it works out in
// batches; the window puts each batch in a ValueStore (on its own thread, a few thousand at a time, so that it is never held up by it), and the grid shows a cell's value from here
// when the store has one, and the value the file stored when it has not (a formula not yet worked out, or one the engine keeps apart).
//
// A table of the cells of a sheet is a thousand and twenty-four small tables, the cell's hash choosing among them, so that one that grows is copied a thousand times smaller than all of
// them would be: no put in a batch ever costs more than a few of its own kind. A value is a number, true or false, an error, or a text, held as 24 bytes (a text is in a list of its own).

#include "calc/value.h"

#include <QString>

#include <deque>
#include <functional>
#include <memory>
#include <vector>

namespace jp::grid {

class ValueStore {
public:
    struct Item {
        qint32 sheet = 0;
        quint32 row = 0;
        quint32 col = 0;
        jp::calc::Value value;
    };

    ValueStore();
    ~ValueStore();
    ValueStore(const ValueStore &) = delete;
    ValueStore &operator=(const ValueStore &) = delete;

    // Forgets every value.
    void clear();
    // Puts the values in: a value for a cell that has one replaces it. An empty value is a value (the cell is empty now); a value that is an array is its first cell.
    void apply(const std::vector<Item> &items);
    // The value of a cell, if the store has one.
    bool find(int sheet, quint32 row, quint32 col, jp::calc::Value *out) const;
    // Takes the values of cells away (the cost is that of the cells named, not of the sheet): `erase` lets them go, `relocate` puts each at another place on the same sheet (the value of
    // a cell that has none there is none), `take` gives them back to put elsewhere (apply).
    void erase(int sheet, const std::vector<std::pair<quint32, quint32>> &cells);
    struct Move {
        quint32 fromRow, fromCol, toRow, toCol;
    };
    void relocate(int sheet, const std::vector<Move> &moves);
    // And a cell's value let go of, true if there was one.
    bool erase(int sheet, quint32 row, quint32 col);
    // Lets go of every value of a sheet (a sheet that was added and is taken away again: when another is added in its place, it starts with none).
    void dropSheet(int sheet);
    // How many cells have values.
    qint64 count() const { return m_count; }

private:
    struct Slot;
    struct Chunk;
    struct Table;
    Table &table(int sheet);
    std::vector<std::unique_ptr<Table>> m_sheets;
    std::deque<QString> m_texts;
    std::vector<qint32> m_freeTexts;
    qint64 m_count = 0;
};

} // namespace jp::grid
