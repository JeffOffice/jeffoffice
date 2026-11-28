# Data tools
<!-- keywords: forecast sheet, forecast, future values, confidence interval, seasonality, trend, smoothing, form, data form, data entry, record, records, new record, find next, find prev, criteria, scenario manager, scenarios, what-if, what if, scenario summary, data, data tab, text to columns, split, separator, comma, tab, fixed width, remove duplicates, goal seek, subtotal, totals, advanced filter, conditions, data validation, circle invalid data, red oval, clear validation circles, consolidate, combine ranges, allow, list, drop-down, input message, error alert, repeated rows, unique, text file, csv, from text, columns, delimiter, quotes, edit links, update values, change source, links, external, source, break link, check status, open source -->

The **Data** tab has the tools that work on a list of values: getting text from a file, putting a list in order, cutting text into columns, and taking out repeated rows.

## From Text/CSV

**From Text/CSV** opens a text file (.csv, .tsv, or .txt). It is the same as **Open Text File** on the File tab. JeffCrunch asks how the file is split into columns and what each column holds, then shows it as a new workbook. The same window opens when you open a text file or use **Paste Text Import**.

The window guesses what each column holds from the first 200 rows of the file and shows the guess in the box above the column. Change any column before you open the file. A kind you change is never replaced by a later guess, even if you then change the separator. A guess does only three things to a column: it leaves the column **As typed**, which is how a file is read where nothing is guessed; it makes the column **Text** to protect it; or it makes the column dates in one order that the values prove. These are the rules:

- A column stays **As typed** when As typed reads its values right: numbers, true and false, times, dates written with month names, signed numbers such as -5 and +3, words, and version numbers such as 1.13.2. Changing these would only make them worse.
- A column is a **date** only when every value in it has a four-digit year (or is a full date such as 2024-05-13) and reads as a date in one order and no other (month, day, year; day, month, year; or year, month, day). Its dates are shown as dates you type are shown. When more than one order fits every value, such as 05/06/2024, the column is kept as **Text**, the window lists it, and you pick the order from its box.
- Values with three numbers and no four-digit year, such as 1.13.2 or 13.05.24, are never made into dates by the guess. If As typed would keep them as text, the column stays As typed; if As typed would turn them into dates, the column is **Text**. Either way the window lists a column that could be dates with two-digit years, so you can pick an order.
- A column is **Text** when a value has a zero in front of more digits or has more than 15 digits, when a plus sign comes before seven or more digits (a phone number), or when an exponent such as 1E5 sits beside other values.
- A value that begins with an equals sign is left to the **Read formulas** box. When the box is ticked, the column stays As typed so the value is read as a formula. When it is not ticked, the column is **Text**.
- Tick **The first row has column names** and the first row is not used for the guess. The window ticks it when the first row is text over numbers or dates.
- The columns after the first 100 are brought in as text.

Only the first 200 rows are looked at, so a value further down can be a surprise. In a date column, a value further down that is not a date in the chosen order comes in as text, never as a wrong date. In a Text column, every value comes in as written. In an As typed column, a value further down is read the way As typed reads it anywhere else.

**Paste Text Import** enters the text as if you had typed it, in your own decimal and thousands marks, so its window has no boxes for those marks and the guess reads the text the same way the paste does. The window for a file has them.

## From File

**From File** in the **Get Data** group loads the rows of a JSON file, an XML file, or another workbook as a table. Only a file on this computer can be loaded. A web address or a file on another computer is refused.

1. Click **From File**, then **Browse** and pick the file. The first rows show at once, with the kind JeffCrunch found for each column from those first rows: number, date, true or false, or text. The whole file is checked when it loads, so a column can end up as text. Nothing on the sheet changes until you click **Load**.
2. When the file holds more than one list, pick the one to load. In a JSON file it is a list of items with named values, such as `$.items`. In an XML file it is an item that repeats, such as `/feed/entry`. In a workbook it is a sheet, a table, or a named range.
3. A JSON item inside an item becomes a column named `outer.inner`, down to four levels. A list of plain values is joined into one cell (the first 10,000 values), or spread over columns (the first 16), as you choose. In XML, the attributes and the child items become columns. Two different values never share a column: when two would get the same name, the second gets a number added or, in XML, its prefix, and a note says so. A name repeated inside one JSON item keeps its last value, and a note says so. A number too large or too small for a cell is kept as text, as written, and a character that can't be read is shown as a replacement mark; both are noted.
4. Tick **Load every column as text** to keep every value as it is written. A column of numbers with a leading zero, such as 007, is always kept as text.
5. Type the cell where the table goes, then click **Load**. If the table would cover cells that hold something, JeffCrunch asks before it replaces them. If there is no room, or the sheet is protected, you are told and nothing changes.

The table is one step of **Undo**. It remembers the file and your choices. To read the file again, click a cell in the table and click **Refresh** on the Data tab. **Refresh All** is the large button beside it: it reads every table made by Get Data, then refreshes every pivot table, all as one step of Undo. The table grows or shrinks to the rows of the file. Columns you added beside the table stay: a formula you filled down is filled into new rows, and the rows that are gone from the file go from them too. If the file is gone or can't be read, you are told and nothing changes. If the file's own columns changed, you are told which ones are missing or new, and nothing changes. Make the table again with Get Data, or put the columns back in the file.

The link to the file is kept in the table in a saved `.xlsx` workbook, in a part that other programs ignore. The link holds the file's full path on this computer, so anyone who gets the workbook can read that path. To take it out before you share the workbook, use **Inspect**, then **Links to data files**, then **Remove**. A copy saved as another kind of file does not keep it, and **Check Compatibility** says so. **Inspect** lists the links under **Links to data files** and can remove them. The tables and their cells stay.

Files are read with limits: a file up to 64 MB, 200,000 rows, 256 columns, and 32,767 characters in a cell. XML with a document type or entities is refused. Anything cut by a limit is said.


## Queries & Connections

**Queries & Connections** in the **Get Data** group lists every table that was made by Get Data. Each row shows the query's name, the file it reads, when it last ran, the rows it gave, and whether it can run. Pick a row and click **Refresh** to read its file again, or click **Refresh All** for every query that can run. A query can carry steps, such as choosing columns, filtering rows, sorting, or changing a column to a number or a date. Refresh runs them again on the file as it is now. If a step can't run, you are told which step and why, and the table keeps its old data. A query saved by a newer version of the program is listed, can't be run, and is saved just as it was. A query that can't be read is listed too, and says so.

When you open a workbook that someone else made, its queries are not read until you say so. The first time you click **Refresh** or **Refresh All** for a query from an opened workbook, a box shows the full path of the file the workbook asks to read. Click **Read the File** only if you trust where the workbook came from. **Don't Read** leaves everything as it was. After you click **Read the File**, that window does not ask about the same file again until you close it. The answer is never saved in the workbook. If you chose **Update them** in **Options > Security**, JeffCrunch also remembers the pair (this workbook and that file) in your own settings, and **Forget All** there clears it. A query you made yourself with **From File** is read without asking.

**Change Type** turns a column into text, a number, or a date. A number may have a decimal mark and a grouping mark, such as 1.234,5 or 1,234.5. A grouping mark counts only between groups of three digits, so 12,34 is not read as 1234. A date is read in the order you choose, with four digits for the year. Two-digit years, a negative number in parentheses such as (5), and a percent such as 5% are not read. The step stops, names the value and its row, and the table keeps its old data. Filters and sorts put numbers first, then dates written as year-month-day, then text, and blanks last. A filter that compares a number with text or a date keeps nothing.

## Edit Links

If a workbook has formulas that read another workbook (such as ='[Prices.xlsx]Sheet1'!A1), **Edit Links** in the **Get Data** group lists those other workbooks, which are called sources. Each row shows where the source is, and its status. Until the workbook is opened again with its sources, it shows the numbers its file stored from them.

- **Check Status** looks for each source on this computer and says **Found**, or says that it was not found. A source on the web is not fetched, so its status says it was not checked.
- **Update Values** reads the chosen source from this computer and gives each cell that is only a reference to one of its cells the value that cell has now. A cell that does more with the value, such as `=[1]Prices!A1*2`, keeps the number its file stored, and the message under the list says how many cells were updated and how many were not. Update Values is one step, so **Undo** puts the old values back. The new values are also written into the link's own part of the file when you save, so the workbook shows them the next time it is opened, even when the source is not there.
- **Change Source** lets you pick another file for the chosen link, for example when the workbook was moved. The formulas stay as they are. The new place is written to the file when you save, and **Undo** takes the change back.
- **Open Source** opens the chosen source in a new window, when it can be found.
- **Break Link** turns every cell that reads the chosen source into the value it shows now, and a name that stood for a place in it into **#REF!**. JeffCrunch asks first. Break Link is one step, so **Undo** brings the formulas, the names, and the links back, until you save the workbook. When no link is left, the file is saved without the parts that held them.

## Form

**Form** (in the **Data Tools** group, and in the search box) shows one row of a list at a time, as a small form. A list is a table, or a block of cells with a row of names on top and a row of data under each name. Click any cell of the list first.

- Each box is one column, labeled with the name in the top row. Type in a box to change the value. A box that is grayed out holds a formula: the form shows what it comes to, and you change the formula in the sheet.
- **Find Next** and **Find Prev** go to the next and the earlier row. Going to another row, or pressing **Enter**, writes what you typed in this one.
- **New** shows an empty form. Fill it in, then press **New** again, **Find Next**, or **Close**. The row is added under the last row of the list. A table grows by one row. If a cell under the list is not empty, the form tells you and adds nothing.
- **Delete** takes out the row you are looking at, after it asks you. The rows under it move up. A table always keeps at least one row of data. A row that is hidden, or that a filter hides, is not shown in the form, is not counted, and is never deleted by it. If the list is too long to be found whole, the form says that it shows only the first rows.
- **Restore** takes back what you typed in this row, before you leave it.
- **Criteria** lets you look for rows. Type a condition in one or more boxes, then press **Find Next** or **Find Prev**. A condition is a value, such as **Ann** (text matches the start of the word, and **?** and ***** stand for letters), or a value after a sign: **>100**, **<=5**, **<>0**, **=Oslo**. A row must meet every condition you type. Press **Form** to go back, or **Clear** to start again. While criteria are set, **Find Next** and **Find Prev** skip the rows that do not meet them.
- **Close** writes the row and closes the form.

Each row you change, add, or delete is one step, so **Undo** in the sheet takes it back in one go.

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
   To treat one piece differently, type its number in **Piece** and choose how it is read: **Same as above**, **As typed**, **Text**, a date written as **month, day, year**, **day, month, year**, or **year, month, day**, or **Leave out**, which puts that piece nowhere. The first piece is 1. A date piece may have the month as a name (**5 Mar 2024**, **March 5, 2024**) in English or in your language, may be separated by /, -, a dot, a comma, or spaces, and may be followed by a time (**10:30**, **10:30:45**, or **10:30 PM**). A piece that is not a date in the order you chose (a month 13, a day that does not exist, a year that is not two or four digits, or a time that does not make sense) is kept as text, just as it was typed; it is never read in another order.
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

## Scenario Manager

A scenario is a named set of values for some cells. Use scenarios to try "what if" on one sheet. For example, keep a "Low sales" scenario and a "High sales" scenario for the same input cells, and show the one you want to look at.

1. Choose **Scenario Manager** on the **Data** tab.
2. Press **Add**. Type a name, such as High sales. In **Changing cells**, type the cells the scenario sets, such as B1:B3 or D5. A scenario can change up to 32 cells. The cells you selected are filled in for you. Each cell starts with the value it holds now. Type the value this scenario should use in the table, and press **OK**.
3. Pick a scenario in the list and press **Show** to put its values in the cells. Double-clicking a name does the same. Press **Edit** to change its name, its comment, or its values. Press **Delete** to take it away.

Show, Add, Edit, and Delete are each one step that you can undo.

**Summary** puts all the scenarios side by side on a new sheet called Scenario Summary. It asks for the result cells, which are the cells you want to compare, such as the total in B10. For each scenario, the summary shows the values of the changing cells and what the result cells come to, worked out with that scenario's values. A column called Current Values shows how the cells were when you made the summary. Your own cells are left as they were.

The scenarios are saved in the workbook. A workbook made by another program keeps its scenarios, and you see them here. A copy saved as an older .xls file or as .ods does not hold them, and the save tells you so.

## Subtotal

Use this to put a row of totals under each group of rows that have the same entry in one column, and a grand total at the bottom. The list needs a header row, and it should be sorted by the column you group at, so that equal entries are next to each other.

1. Select a cell in the list.
2. Choose **Subtotal** in the **Outline** group of the **Data** tab.
3. In **At each change in**, choose the column that groups the rows, such as Region.
4. In **Use function**, choose what to work out: **Sum**, **Count**, **Average**, **Max**, **Min**, and others.
5. In **Add subtotal to**, check each column to total.
6. Leave **Replace current subtotals** checked to swap out subtotals that are already there.
7. **Summary below data** starts as the sheet's outline has it. Leave it checked to put each total row under its group, or clear it to put each total row above its group, with the grand total at the top. Press **OK**.

Each total row says "East Total" (or the entry it is for) and holds a SUBTOTAL formula, so it leaves out the other total rows. The rows of each group are grouped, so you can use the 1, 2, and 3 buttons beside the rows to show only the totals. To take the totals away, open **Subtotal** again and press **Remove All**. If the totals went to the other side of a sheet that already had its own groups, the status bar says that those groups now have their summary rows on that side, and Remove All puts the side back as it was before the first subtotal, unless you changed it yourself since. When the totals are above, the outline's buttons sit at the total rows, so a plus or minus button next to "East Total" shows or hides the East rows below it. If you turn off **Replace current subtotals**, the new totals must go on the same side as the ones already there. Otherwise nothing is changed, and a message says so. Adding or removing subtotals is one step that you can undo.

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

## Data Validation

Use this to say what may be typed in some cells, so that a wrong entry is stopped or questioned.

1. Select the cells.
2. Choose **Data Validation** in the **Data Tools** group of the **Data** tab.
3. On the **Settings** tab, choose what to **Allow**: **Any value** (no rule), **Whole number**, **Decimal**, **List**, **Date**, **Time**, **Text length**, or **Custom**.
4. For a number, date, time, or text length, choose **Data** (**between**, **not between**, **equal to**, and so on) and type the limits. A limit can be a value or a formula that starts with =.
5. For a **List**, type the choices with commas between them, such as Red,Green,Blue, or type a range of cells after =, such as =$A$1:$A$5. The cell then shows an arrow with the choices. Turn off **In-cell drop-down** to hide the arrow.
6. For **Custom**, type a formula that starts with =. The entry is allowed when the formula is true.
7. **Ignore blank** lets an empty cell pass.

On the **Input Message** tab, check the box and write a title and a message. They show beside the cell whenever it is selected.

On the **Error Alert** tab, choose what happens to an entry that is not allowed. **Stop** refuses it. **Warning** asks whether to keep it. **Information** tells the person and keeps it. Write a title and a message for the alert.

**Circle Invalid Data** (next to **Data Validation**) puts a red oval round each cell on the sheet whose value its rule does not allow, such as a number that was there before the rule, or the result of a formula. A message tells you if every cell is fine. A cell that you put right loses its oval. **Clear Validation Circles** takes all the ovals away.

**Clear All** takes the rules off the selected cells. The new rule replaces any rule the selected cells had; other cells keep theirs. Changing rules is one step that you can undo, and the rules are saved in the file.

## Consolidate

Use this to combine ranges that look alike, such as the same table from several sheets, into one table.

1. Choose **Consolidate** in the **Data Tools** group of the **Data** tab.
2. Choose the **Function** to combine with, such as **Sum** or **Average**.
3. Type a range in the box and press **Add**. Use **Sheet2!A1:C10** for a range on another sheet. Add each range you want to combine. Select a range in the list and press **Delete** to take it out.
4. If the ranges have names in their first row, check **Top row holds the names of columns**. If they have names in their first column, check **Left column holds the names of rows**.
5. In **Put the result at**, type the first cell for the result.
6. Check **Create links to the source data** if each result should be a formula that reads its source cells, so it changes when they do. Without it, the results are plain numbers.
7. Press **OK**.

With no names checked, the cell at the same place in each range is combined (the first cell of every range, then the second, and so on). With names checked, cells with the same names are combined, wherever they are in each range, and a name that only some ranges have still gets a row or column. Consolidate is one step that you can undo.

## Forecast Sheet

Forecast Sheet makes a new sheet with a table and a chart of future values, worked out from a timeline and a series of values. It finds the trend and the repeating season of the values, and adds a band that the future values are likely to stay inside.

1. Select the timeline and the values, two columns side by side (the timeline first). The timeline is dates or numbers the same distance apart, such as every month or every 5 units. A first row of names is left out.
2. Choose **Forecast Sheet** in the **Forecast** group of the **Data** tab.
3. A picture shows the forecast, and changes as you change the choices. Pick **Line chart** or **Column chart**.
4. Set the choices:
   - **Timeline range** and **Values range** are the cells to read. Write other cells here if the selection was wrong.
   - **Forecast start** is empty to start after the last point. A date or number before the last point starts there and uses only the points before it, so you can compare the forecast with what happened.
   - **Forecast end** is the last time to forecast. At most 10,000 points are forecast.
   - **Confidence interval** is how sure the band is, 95% to begin with.
   - **Seasonality** is found by looking at the values, or you can set the length of the season in points, or choose none. The values must hold two whole seasons.
   - **Fill missing points using** says what a time that has no value gets: a straight line between its neighbors, or zero.
   - **Aggregate duplicates using** says how values at the same time are combined: Average, Count, CountA, Max, Median, Min, or Sum.
   - **Include forecast statistics** adds a table of Alpha, Beta, Gamma, MASE, SMAPE, MAE, and RMSE, which say how well the forecast fits the values.
5. If something is wrong, the dialog says what in plain words (steps that are not equal, too few points, text in the values), and **Create** waits.
6. Press **Create**.

The new sheet is put before the one you were on. Its table has the columns Timeline, Values, Forecast, Lower Confidence Bound, and Upper Confidence Bound. The Forecast and the bounds are formulas that use `FORECAST.ETS` and `FORECAST.ETS.CONFINT` on the table's own Timeline and Values columns, so they are worked out again when you change a value. The chart shows the values, the forecast, and the band. Forecast Sheet is one step that you can undo.

The numbers can differ by a few percent from those of other spreadsheet programs, which use their own way of fitting the smoothing, and a forecast that a file already holds keeps the values the file stored until the data it reads is changed.

A file with more than 1,000,000 points is not used.
