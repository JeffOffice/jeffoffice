# Data tools
<!-- keywords: data, data tab, text to columns, split, separator, comma, tab, fixed width, remove duplicates, goal seek, subtotal, totals, advanced filter, conditions, repeated rows, unique, text file, csv, from text, columns, delimiter, quotes -->

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

## Goal Seek

Use this when you know the answer you want but not the number that gives it. For example, a payment cell holds a formula that uses an interest rate, and you want the payment to be 500. Goal Seek tries numbers in the rate cell until the payment is 500.

1. Choose **Goal Seek** on the **Data** tab.
2. In **Cell with the formula**, type the cell that holds the formula, such as B3. It starts as the cell you selected.
3. In **Value you want**, type the answer you want the formula to give.
4. In **Cell to change**, type the cell that Goal Seek may change, such as B1. It must hold a number, not a formula.
5. Press **OK**.

If Goal Seek finds a number, it stays in the cell to change, and the status bar says what it is. If it can't, nothing changes and you are told. Try a different starting number in the cell to change, and check that the formula really uses that cell. Goal Seek is one step that you can undo.

## Subtotal

Use this to put a row of totals under each group of rows that have the same entry in one column, and a grand total at the bottom. The list needs a header row, and it should be sorted by the column you group at, so that equal entries are next to each other.

1. Select a cell in the list.
2. Choose **Subtotal** in the **Outline** group of the **Data** tab.
3. In **At each change in**, choose the column that groups the rows, such as Region.
4. In **Use function**, choose what to work out: **Sum**, **Count**, **Average**, **Max**, **Min**, and others.
5. In **Add subtotal to**, check each column to total.
6. Leave **Replace current subtotals** checked to swap out subtotals that are already there. Press **OK**.

Each total row says "East Total" (or the entry it is for) and holds a SUBTOTAL formula, so it leaves out the other total rows. The rows of each group are grouped, so you can use the 1, 2, and 3 buttons beside the rows to show only the totals. To take the totals away, open **Subtotal** again and press **Remove All**. Adding or removing subtotals is one step that you can undo.

## Advanced Filter

The Filter buttons pick rows by a column at a time. **Advanced** (in the **Sort & Filter** group) lets you write the conditions on the sheet, so they can be as long as you need.

1. Next to the list, copy the headings of the columns you want to test into a row of empty cells.
2. Under each heading, write a condition. Conditions in the **same row** must all be true (this is "and"). Conditions in **different rows** are alternatives: a row of the list needs to meet only one of those rows (this is "or").
3. Choose **Advanced**. Check that **List range** is the list with its header row, and type the cells of your headings and conditions in **Conditions range**.
4. Choose **Hide the other rows, in the list**, or **Copy the rows to another place** and type the first cell of the place, such as H1. Check **Leave out rows that repeat an earlier one** to copy only one of each.
5. Press **OK**.

How to write a condition:

- A word, such as **An**, picks text that begins with it. Capital and small letters count as the same. **?** stands for any one character and ***** for any run of characters.
- A number picks cells that equal it.
- **=Ann** picks text that is exactly Ann. **=** alone picks empty cells.
- **>**, **>=**, **<**, **<=**, and **<>** (not equal) before a number or a word compare with it, such as **>25**. **<>** alone picks cells that are not empty.

A row of conditions with nothing in it picks every row. To show the hidden rows again, choose **Clear**. A copy is made at once and does not change when the list does. Advanced Filter is one step that you can undo.
