#pragma once
// The cells of a selection as tab-separated text, a few at a time, for the clipboard: what each cell shows, a tab between cells, a line to a row, a text with a tab, a line break, or
// a quote in it quoted, and rows and columns that are hidden left out. A selection of a million cells is not turned to text at once (it would hold the window up); a TextWalk does
// about a given number of cells at a time and says when it is done. Cells past the last the sheet has hold nothing, so a whole column is its used rows.

#include "grid/gridsheet.h"
#include "grid/selection.h"

#include <QString>

namespace jp::grid {

class TextWalk {
public:
    // The most cells a selection may have for its text to be made (a million).
    static constexpr qint64 kMaxCells = 1000000;
    TextWalk(const GridSheet *sheet, const GridRange &range);
    // The most characters of text a copy may hold (a cell may hold a text of any length up to a file's limit, and a million of them are too many to put on a clipboard).
    static constexpr qint64 kMaxChars = 64000000;
    // True when the selection has more cells than kMaxCells (nothing is made).
    bool tooMany() const { return m_tooMany; }
    // True when the text made came to kMaxChars and the walk stopped: what is in text() is not the whole of it, and should not be copied.
    bool tooLarge() const { return m_tooLarge; }
    // The characters of text made so far.
    qint64 chars() const { return m_chars; }
    // Looks at about `budget` cells (0 for all of them); true when every cell has been looked at.
    bool step(qint64 budget);
    const QString &text() const { return m_text; }
    // Work done so far, in eighths of a cell with something in it: a place with nothing is one, and a cell that shows something is eight. A test counts it.
    qint64 looked() const { return m_looked; }

private:
    const GridSheet *m_sheet;
    GridRange m_range;
    bool m_tooMany = false, m_done = false, m_firstInLine = true, m_tooLarge = false;
    qint64 m_chars = 0;
    qint64 m_row, m_col;
    QString m_text, m_line;
    qint64 m_looked = 0;
};

} // namespace jp::grid
