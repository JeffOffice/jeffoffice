# Lookup and reference functions
<!-- keywords: functions, ADDRESS, AREAS, COLUMN, COLUMNS, FILTER, FORMULATEXT, GETPIVOTDATA, HLOOKUP, HYPERLINK, INDEX, INDIRECT, LOOKUP, MATCH, OFFSET, ROW, ROWS, SORT, SORTBY, TRANSPOSE, UNIQUE, VLOOKUP, XLOOKUP, XMATCH -->

These are the lookup and reference functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## ADDRESS

`ADDRESS(row_num, column_num, [abs_num], [a1], [sheet_text])`

Writes the address of a cell as text, from its row and column numbers.

- `row_num`: The number of the row in the block, counting the first row as 1.
- `column_num`: The number of the column in the block, counting the first column as 1.
- `abs_num` (optional): How much of the address is fixed: 1 all of it, 2 the row only, 3 the column only, 4 none of it.
- `a1` (optional): True or left out to read the address as a column letter and row number; false to read it as row and column numbers.
- `sheet_text` (optional): The name of the sheet to put in front of the address.

## AREAS

`AREAS(reference)`

Gives the number of separate blocks of cells in a reference.

- `reference`: A cell or block of cells.

## COLUMN

`COLUMN([reference])`

Gives the column number of a cell.

- `reference` (optional): A cell or block of cells.

## COLUMNS

`COLUMNS(array)`

Counts the columns in a block of cells.

- `array`: A block of cells, or a list of values written in braces.

## FILTER

`FILTER(array, include, [if_empty])`

Gives the rows or columns of a block that pass a test.

- `array`: A block of cells, or a list of values written in braces.
- `include`: A column or row of true and false values, one for each row or column of the block: true keeps it.
- `if_empty` (optional): What to give when no row or column passes (the #CALC! error if left out).

## FORMULATEXT

`FORMULATEXT(reference)`

Gives the formula of a cell as text.

- `reference`: A cell or block of cells.

## GETPIVOTDATA

`GETPIVOTDATA(data_field, pivot_table, [field1], [item1], ...)`

Gives the value a pivot table shows for a value and the items of its rows and columns. A cell of the table is the second argument; the fields named must be the first ones of the rows or of the columns.

- `data_field`: The name of the value to give, in quotes, as the pivot table shows it ("Sum of Amount").
- `pivot_table`: Any cell of the pivot table.
- `field1` (optional): The name of a field of the rows or of the columns, in quotes (can be left out with its item).
- `item1` (optional): The item of that field to give the value for, in quotes.
- `...`: As many more as you need, of the same kind as the one before it.

## HLOOKUP

`HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])`

Looks for a value in the first row of a table and gives a value from the same column.

- `lookup_value`: The value to look for.
- `table_array`: The block of cells to look in; the value looked for must be in its first column.
- `row_index_num`: The number of the row of the table to give a value from, counting the first row as 1.
- `range_lookup` (optional): False finds an exact match; true or left out finds the closest value below it (the first column must be sorted up).

## HYPERLINK

`HYPERLINK(link_location, [friendly_name])`

Makes a cell that opens a link when it is clicked.

- `link_location`: Where the link goes: a web address, a file, or a place in this workbook.
- `friendly_name` (optional): The text to show in the cell (the link itself if left out).

## INDEX

`INDEX(array, row_num, [column_num], [area_num])`

Gives the value at a row and column of a block of cells.

- `array`: A block of cells, or a list of values written in braces.
- `row_num`: The number of the row in the block, counting the first row as 1.
- `column_num` (optional): The number of the column in the block, counting the first column as 1.
- `area_num` (optional): Which block to use when the first argument names several (1 for the first).

## INDIRECT

`INDIRECT(ref_text, [a1])`

Gives the cell that an address written as text points to.

- `ref_text`: An address written as text, such as "B5".
- `a1` (optional): True or left out to read the address as a column letter and row number; false to read it as row and column numbers.

## LOOKUP

`LOOKUP(lookup_value, lookup_vector, [result_vector])`

Looks for a value in one row or column and gives the value at the same place in another.

- `lookup_value`: The value to look for.
- `lookup_vector`: The one row or one column of cells to look in.
- `result_vector` (optional): The one row or one column of cells to give the result from.

## MATCH

`MATCH(lookup_value, lookup_array, [match_type])`

Gives the place of a value in a list.

- `lookup_value`: The value to look for.
- `lookup_array`: The block of cells, one row or one column, to look in.
- `match_type` (optional): 1 or left out finds the largest value not above it (the list must be sorted up); 0 finds an exact match; -1 finds the smallest value not below it (the list must be sorted down).

## OFFSET

`OFFSET(reference, rows, cols, [height], [width])`

Gives the cells a given number of rows and columns away from a starting cell.

- `reference`: A cell or block of cells.
- `rows`: How many rows to move: positive down, negative up.
- `cols`: How many columns to move: positive to the right, negative to the left.
- `height` (optional): How many rows tall the result is.
- `width` (optional): How many columns wide the result is.

## ROW

`ROW([reference])`

Gives the row number of a cell.

- `reference` (optional): A cell or block of cells.

## ROWS

`ROWS(array)`

Counts the rows in a block of cells.

- `array`: A block of cells, or a list of values written in braces.

## SORT

`SORT(array, [sort_index], [sort_order], [by_col])`

Gives a block with its rows or columns put in order.

- `array`: A block of cells, or a list of values written in braces.
- `sort_index` (optional): The number of the column (or row) to sort by (1 if left out).
- `sort_order` (optional): 1 or left out for ascending; -1 for descending.
- `by_col` (optional): True to work on columns; false or left out to work on rows.

## SORTBY

`SORTBY(array, by_array1, [sort_order1], ...)`

Gives a block put in the order of one or more other blocks.

- `array`: A block of cells, or a list of values written in braces.
- `by_array1`: The block of cells to put the first block in the order of; the same length as the first block.
- `sort_order1` (optional): 1 or left out for ascending; -1 for descending.
- `...`: As many more as you need, of the same kind as the one before it.

## TRANSPOSE

`TRANSPOSE(array)`

Turns rows into columns and columns into rows.

- `array`: A block of cells, or a list of values written in braces.

## UNIQUE

`UNIQUE(array, [by_col], [exactly_once])`

Gives the different rows or columns of a block.

- `array`: A block of cells, or a list of values written in braces.
- `by_col` (optional): True to work on columns; false or left out to work on rows.
- `exactly_once` (optional): True to give only the items that appear once; false or left out to give each different item.

## VLOOKUP

`VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])`

Looks for a value in the first column of a table and gives a value from the same row.

- `lookup_value`: The value to look for.
- `table_array`: The block of cells to look in; the value looked for must be in its first column.
- `col_index_num`: The number of the column of the table to give a value from, counting the first column as 1.
- `range_lookup` (optional): False finds an exact match; true or left out finds the closest value below it (the first column must be sorted up).

## XLOOKUP

`XLOOKUP(lookup_value, lookup_array, return_array, [if_not_found], [match_mode], [search_mode])`

Looks a value up in a list and gives the matching item of another list.

- `lookup_value`: The value to look for.
- `lookup_array`: The block of cells, one row or one column, to look in.
- `return_array`: The block of cells to give the match from.
- `if_not_found` (optional): What to give when there is no match (the #N/A error if left out).
- `match_mode` (optional): 0 or left out for an exact match, -1 for the next smaller, 1 for the next larger, 2 to use wildcards.
- `search_mode` (optional): 1 or left out to search from the first, -1 from the last, 2 or -2 for a binary search on a sorted list.

## XMATCH

`XMATCH(lookup_value, lookup_array, [match_mode], [search_mode])`

Gives the place of a value in a list, with choices for how to match and which way to look.

- `lookup_value`: The value to look for.
- `lookup_array`: The block of cells, one row or one column, to look in.
- `match_mode` (optional): 0 or left out for an exact match, -1 for the next smaller, 1 for the next larger, 2 to use wildcards.
- `search_mode` (optional): 1 or left out to search from the first, -1 from the last, 2 or -2 for a binary search on a sorted list.
