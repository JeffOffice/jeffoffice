# Information functions
<!-- keywords: functions, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE -->

These are the information functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## CELL

`CELL(info_type, [reference])`

Tells something about a cell, such as its address or its format.

- `info_type`: What to tell about the cell, as text such as "address", "row", "col", or "format".
- `reference` (optional): The cell to tell about (the cell with the cursor if left out).

## ERROR.TYPE

`ERROR.TYPE(error_value)`

Gives the number that stands for a kind of error.

- `error_value`: The error to look up, or a cell that holds it.

## INFO

`INFO(info_type)`

Tells something about the program or the workbook, such as its folder or how it calculates.

- `info_type`: What to tell about the cell, as text such as "address", "row", "col", or "format".

## ISBLANK

`ISBLANK(value)`

True when the cell is empty.

- `value`: The value to work on, or a cell that holds it.

## ISERR

`ISERR(value)`

True when the value is an error other than not-available.

- `value`: The value to work on, or a cell that holds it.

## ISERROR

`ISERROR(value)`

True when the value is any error.

- `value`: The value to work on, or a cell that holds it.

## ISEVEN

`ISEVEN(number)`

True when a whole number is even.

- `number`: The number to work on.

## ISFORMULA

`ISFORMULA(reference)`

True when the cell holds a formula.

- `reference`: A cell or block of cells.

## ISLOGICAL

`ISLOGICAL(value)`

True when the value is true or false.

- `value`: The value to work on, or a cell that holds it.

## ISNA

`ISNA(value)`

True when the value is the not-available error.

- `value`: The value to work on, or a cell that holds it.

## ISNONTEXT

`ISNONTEXT(value)`

True when the value is anything but text.

- `value`: The value to work on, or a cell that holds it.

## ISNUMBER

`ISNUMBER(value)`

True when the value is a number.

- `value`: The value to work on, or a cell that holds it.

## ISODD

`ISODD(number)`

True when a whole number is odd.

- `number`: The number to work on.

## ISREF

`ISREF(value)`

True when the value is a reference to cells.

- `value`: The value to work on, or a cell that holds it.

## ISTEXT

`ISTEXT(value)`

True when the value is text.

- `value`: The value to work on, or a cell that holds it.

## N

`N(value)`

Turns a value into a number: dates and true values become numbers, text becomes zero.

- `value`: The value to work on, or a cell that holds it.

## NA

`NA()`

Gives the not-available error, to mark a value that is missing.

## SHEET

`SHEET([value])`

Gives the number of a sheet, counting the first sheet as 1.

- `value` (optional): The value to work on, or a cell that holds it.

## SHEETS

`SHEETS([reference])`

Gives the number of sheets in a reference, or in the workbook.

- `reference` (optional): A cell or block of cells.

## TYPE

`TYPE(value)`

Gives a number that says what kind of value it is: number, text, logical, error, or array.

- `value`: The value to work on, or a cell that holds it.
