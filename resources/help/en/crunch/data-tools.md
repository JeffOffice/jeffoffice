# Data tools
<!-- keywords: data, data tab, text to columns, split, separator, comma, tab, fixed width, remove duplicates, repeated rows, unique, text file, csv, from text, columns, delimiter, quotes -->

The **Data** tab has the tools that work on a list of values: getting text from a file, putting a list in order, cutting text into columns, and taking out repeated rows.

## From Text/CSV

**From Text/CSV** opens a text file (.csv, .tsv, or .txt). It is the same as **Open Text File** on the File tab. JeffCrunch asks how the file is split into columns and what kind of text it holds, then shows it as a new workbook.

## Sort and filter

The **Sort & Filter** group has the same commands as the **Sort and Pick Rows** menu on the Home tab. See [Sort and filter](sort-filter).

## Text to Columns

Use this when one column holds several pieces of text that you want in separate columns, such as "Smith, Ann, 31".

1. Select the cells of **one** column.
2. Choose **Text to Columns** on the **Data** tab.
3. Choose how to split the text:
   - **Split at a separator**: check the marks that stand between the pieces: **Tab**, **Semicolon**, **Comma**, **Space**, or **Other**, where you type your own mark. **Treat separators in a row as one** keeps two commas in a row from making an empty piece. **Text in quotes** names the mark that wraps a piece that must not be split, such as "Lee, Bo". Inside the quotes, two marks in a row stand for one.
   - **Split at fixed places**: type the places where a new column begins, counting characters from 1. For example, 5, 12 starts a new column at the fifth and at the twelfth character.
4. Choose what each piece is: **General** reads numbers and dates as if you typed them, and **Text** keeps every piece as it is, so "007" stays "007".
5. In **Put the pieces from**, type the first cell for the pieces. It starts as the first cell you selected.
6. The box under the choices shows the first rows split the way you chose. Press **OK**.

The pieces go in the cells to the right of the first cell, and they replace what was in them. Text to Columns is one step that you can undo.

## Remove Duplicates

Use this to keep only the first of each group of rows that repeat.

1. Select the cells of the list, or one cell in it. JeffCrunch takes the whole block of data around a single cell.
2. Choose **Remove Duplicates**.
3. Check **My data has a header row** if the first row holds names. That row stays, and it is not compared.
4. Check the columns that must match for two rows to count as repeats. Two rows repeat only when every checked column is the same. Capital and small letters count as the same, and a formula counts as the value it gives.
5. Press **OK**. The dialog tells you how many rows were removed and how many are left.

The rows under a removed row move up inside the selected cells, with their formats and formulas. The cells under the selection do not move. The empty rows left at the bottom of the selection are cleared. Remove Duplicates is one step that you can undo.
