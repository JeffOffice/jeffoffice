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
    // Moves the values of a sheet's cells (rows or columns inserted or deleted): `where` is called with the place of each cell that has a value, and sets its new place; false for a
    // cell that is gone (its value is let go). Returns how many values moved or went; nothing is touched when none did.
    qint64 remap(int sheet, const std::function<bool(quint32 *row, quint32 *col)> &where);
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
