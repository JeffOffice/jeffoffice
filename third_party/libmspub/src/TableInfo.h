/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libmspub project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TABLEINFO_H
#define INCLUDED_TABLEINFO_H

#include <vector>

namespace libmspub
{

struct CellInfo
{
  CellInfo()
    : m_startRow()
    , m_endRow()
    , m_startColumn()
    , m_endColumn()
  {
  }

  unsigned m_startRow;
  unsigned m_endRow;
  unsigned m_startColumn;
  // JeffPub: inner margins in EMUs (left, right, top, bottom); -1 = Publisher's default.
  long m_margins[4] = {-1, -1, -1, -1};
  // JeffPub: Text Direction (field 06): 2 turns the cell's text a quarter turn.
  unsigned m_textDirection = 0;
  // JeffPub: vertical alignment (field 07): 1 middle, 2 bottom, 0 top.
  unsigned m_verticalAlign = 0;
  // JeffPub: a cell divided by a diagonal is two records at the same place
  // (field 05, low byte): bit 0 = divided, bit 1 = from top left to bottom
  // right, bit 2 = the first half (the second half is listed after the grid).
  unsigned m_split = 0;
  unsigned m_endColumn;
};

// JeffPub: a cell fill or border from the table's formatting shapes.
// Publisher keeps these as drawing shapes with no page placement; their
// client anchor names the table (field 2) and the cells:
//   kind 0 (field 1 absent): fill of the cell at column field 3, row field 4.
//   kind 1: horizontal rule on row boundary field 4 (= field 6), from
//           column field 5 up to (not including) column field 7.
//   kind 2: vertical rule on column boundary field 5 (= field 7), from row
//           field 4 up to (not including) row field 6.
//   kind 3: the diagonal border of the divided cell at rows field 4 up to
//           field 6 and columns field 5 up to field 7; a line shape.
// The color is the shape's fill color (a diagonal's is its line color); a
// rule's width is its line width.
struct TableCellFormat
{
  unsigned kind = 0;
  unsigned f[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  bool hasColor = false;
  unsigned color = 0;
  unsigned widthEmu = 0;
};

struct TableInfo
{
  std::vector<TableCellFormat> m_formats;
  std::vector<unsigned> m_rowHeightsInEmu;
  std::vector<unsigned> m_columnWidthsInEmu;
  unsigned m_numRows;
  unsigned m_numColumns;
  std::vector<CellInfo> m_cells;
  TableInfo(unsigned numRows, unsigned numColumns) : m_rowHeightsInEmu(),
    m_columnWidthsInEmu(), m_numRows(numRows), m_numColumns(numColumns),
    m_cells()
  {
  }
};
} // namespace libmspub

#endif /* INCLUDED_TABLEINFO_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
