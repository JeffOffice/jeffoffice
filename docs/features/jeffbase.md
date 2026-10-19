# JeffBase feature list

JeffBase is the desktop database app of JeffOffice, for contact lists, inventories, and small-business records. JeffOffice is an independent project: JeffBase aims to work with Access databases and the people who use them (compatibility and interoperability), and then to do more, and do it better. It keeps its own data in a **SQLite** file by default, opens Access's `.accdb` and `.mdb` files, and saves `.accdb` for people who still use Access. It is not built yet: nothing exists except what the suite already does for every app, so `[x]` and `[~]` count only that. This list is the whole job, a checklist of features: everything people rely on so their Access databases open, give the right answers, and keep working, and then more, grouped the way the app's ribbon is (File, then each ribbon tab, the tabs that appear only while something is selected, then what sits under the ribbon), so the work can be planned and tracked.

Last reviewed October 16, 2026.

Legend: `[x]` done · `[~]` partly there (the note says what's missing, or which shared JeffOffice part it builds on) · `[ ]` not yet · **(alt)** the usual way depends on a proprietary online service, so the app offers an open replacement.

Every item carries a tier:

- **first release**: needed in JeffBase's first public release.
- **1.0**: needed before JeffBase is called finished (version 1.0).
- **later**: wanted, but not before 1.0.

## How the tiers were chosen

**First release** is the first public build (a preview, as JeffDeck 0.7.0 is). It must be good enough for a contact list, an inventory, or a small record book: make tables, type and edit rows in the datasheet (the grid that shows a table), join tables with relationships, ask questions with select queries, enter data on simple forms, print a simple report, bring data in from and send it out to CSV files and spreadsheets, and open a person's existing `.accdb` or `.mdb` without damaging it. **1.0** is what a full replacement needs. Version 1.0 is ready to ship when every `.accdb` JeffBase saves (tables, relationships, and queries) opens without a repair message, Compact & Repair finishes on it without a message, and JeffBase's forms, reports, and queries give the reference answers on the sample databases. An item is **later** when it is used by few databases, needs a piece the suite doesn't have yet, runs code, or depends on something outside the program. In this list **(alt)** also marks a usual way that depends on a Windows-only part, not only on an online service.

There are no usage counts for databases. The owner has no Access files, and the public collections used for the other apps hold almost none, so the tiers rest on what a person moving from Access needs on day one, then on what a full replacement needs. Three cautions:

- Microsoft never published the `.accdb` and `.mdb` formats. Writing `.accdb` follows the format knowledge of Jackcess, an open-source Java library, and every claim is proved by checks on the saved file: it must open without a repair message.
- The ribbon below has 1,571 controls on the tabs, the Quick Access Toolbar, and the File page. Access's commands for SharePoint, web apps, replication, and source control are collected under "Left out on purpose".
- How Access's expressions and SQL behave (what `Null` does in arithmetic, how `Like` matches, what `True` equals) is documented only in part. Each such rule is checked against reference answers from real databases before JeffBase copies it.

## Words used here

A small example carries through the list. A shop keeps a **table** (a list of rows that all have the same fields) named Customers, with one row per customer and fields such as Name, City, and Phone. A second table, Orders, has one row per order. A **primary key** is the field (often an AutoNumber that counts 1, 2, 3, and so on) that tells one row from another. A **relationship** joins Orders.CustomerID to Customers.ID, so each order points to one customer and the program can refuse an order for a customer who isn't there. The **datasheet** is the grid where rows are typed and edited. A **query** is a saved question, such as "orders over $100 from customers in Ohio, newest first"; its answer looks like a table. A **form** is a screen for entering or looking at one row at a time, such as a new order. A **report** is a printed or PDF page built from a table or query, such as each customer's orders with a total. A **macro** is a saved list of steps, and **VBA** is Access's programming language; both can run on their own, which is why they need care. **SQL** is the text language for asking a database questions; a query is SQL underneath. **ODBC** is a Windows-born standard for talking to database servers. **Compact & Repair** rewrites a file to shrink it and fix damage.

## Beyond Access

What JeffBase offers that Access doesn't, then the plans that go past Access.

- [~] **first release** Runs on Linux and Windows, and on macOS as the rest of the suite does; Access runs only on Windows *(shared: the suite's builds, installers, and AppImage; the macOS app is still a test build)*
- [x] **first release** Free and open source (GPL-3.0): no license fee, no subscription, no account or sign-in, nothing that has to be online
- [~] **first release** Installs without administrator rights, or runs with no installation (Windows `.zip`, Linux AppImage) *(shared: the suite's packaging)*
- [ ] **first release** The file is an ordinary SQLite database that any other tool can open (the `sqlite3` program, Python, a phone, a browser extension, a spreadsheet's database import): every table reads as a table, and every saved select query reads as a view
- [ ] **first release** Nothing is hidden in a private format. JeffBase's own extras (forms, reports, field settings, the drawn layout of relationships and queries) sit in plainly named tables inside the same file, described in the user guide, so a file stays readable and its data stays recoverable even if JeffBase is gone
- [ ] **first release** Opening a SQLite file made by another program changes nothing in it: JeffBase adds its own tables only when the person first saves a form, report, or setting
- [ ] **first release** Saving is crash-safe: each change is committed whole or not at all, so a power cut leaves a good file, not a "needs repair" message, and no lock file is left behind to clean up
- [ ] **first release** No 2 GB ceiling (Access stops there): a file may hold millions of rows, limited by the disk; the datasheet scrolls a million-row table smoothly
- [ ] **first release** SQL you can see and edit: every query has a SQL view that stays in step with the designer, and a pane shows the exact SQL that ran for any filter, sort, or query
- [ ] **1.0** Open any SQLite file from another program (a phone app's data, a browser's history, a mapping tool, a game's save) and work on it as a database: tables, views, indexes, and foreign keys (shown as relationships) appear, and anything JeffBase can't edit is kept and written back as it was
- [ ] **1.0** Search everywhere: one search box over all the text in all tables, fast because it uses SQLite's full-text index (Access searches one table at a time)
- [ ] **1.0** **(alt)** Starter databases that come with the program and need no online store (Access downloads its templates from Microsoft's servers)
- [ ] **1.0** `jbasetool`, a command-line tool that converts between formats, runs a query and writes the answer as CSV, dumps a file's design as SQL, checks a file's round trip, and runs the fidelity tests, as `jpubtool` and `jdecktool` do for the other apps *(shared: the pattern of those tools)*
- [ ] **later** Compare two databases: the tables, fields, indexes, relationships, and queries that differ, and the rows that differ in a chosen table
- [ ] **later** Undo across records: a list of the edits made since the file was opened, undone one at a time (Access can undo only the record being edited)
- [ ] **later** Work in a transaction: gather several edits, then commit them together or roll them all back
- [ ] **later** A JSON field type with path lookups, and JSON import and export, using SQLite's own JSON functions
- [ ] **later** Full outer joins in queries (Access has none; SQLite does)

## File (backstage)

### Info
- [~] **first release** Database properties: title, author, subject, category, keywords, comments, manager, and company; custom properties (a name, a type, and a value); the file's name, folder, format and version, size, and dates; the counts of tables, queries, forms, and reports; and Open Folder and Copy Path *(shared: JeffPub's properties page)*
- [ ] **first release** Compact & Repair Database: rewrite the file to give back unused space, check it for damage, and rebuild its indexes, then say what was fixed (details under Database Tools)
- [ ] **first release** The security panel: lists what the file holds that could run or reach out (macros, VBA, a startup form, tables linked to a server, pass-through queries) and lets the person allow it, for this time or for this file
- [ ] **1.0** Encrypt with Password (and Remove Database Password, as the Database Tools tab calls it): open and save password-protected files (details under File formats)
- [ ] **1.0** Check Compatibility: before Save As `.accdb`, names what the format can't hold (a SQLite view Access can't run, a column type with no match, a name longer than 64 characters, a table with more than 255 fields), and after opening, lists what was kept but can't be edited
- [ ] **1.0** Reconnect linked tables: when a linked file has moved, point the links at its new place

### New
- [ ] **first release** Blank database: choose the name and folder, and a new SQLite file opens with one empty table in the datasheet
- [~] **first release** Template gallery with live previews and categories, starting with a few starter databases written by JeffOffice and open-licensed (contacts, inventory, projects and tasks, event planner) *(shared: JeffPub's New page)*
- [ ] **1.0** More starter databases: assets, donations, vehicles, recipes, classroom, orders and invoices, a small library, and a new database made from an existing file's design without its data
- [ ] **1.0** Save the open database as a template (its design, with or without its data) and reuse it from My Templates

### Open, Save, Save As, and Close
- [~] **first release** Open `.sqlite`, `.sqlite3`, `.db`, JeffBase's own, `.accdb`, and `.mdb` files, from the Recent list with pinning, from Browse, by drag-and-drop, and from the command line *(shared: the recent list and file pages in libs/shell)*
- [ ] **first release** Opening an `.accdb` or `.mdb` shows its tables, data, relationships, and queries at once, and keeps everything else in it (forms, reports, macros, modules) untouched; Save puts them back (see File formats)
- [ ] **first release** Open Read-Only, so nothing can change; a file Access (or another program) has open is noticed from its lock file and opens read-only with a note
- [ ] **first release** Data saves itself: a row is committed the moment the person leaves it (Shift+Enter commits it at once, and Esc undoes it), so Save in the File page saves the design of the open object; Ctrl+S saves the open table, query, form, or report design, and Close Database or Exit asks first about a changed design
- [ ] **first release** Save Database As: JeffBase's SQLite database, or an Access `.accdb`; a copy leaves the open file alone
- [ ] **first release** Save Object As: copy the open table, query, form, or report under a new name, or into another database
- [ ] **first release** Back Up Database: a dated copy made while the file is open and in use, consistent as of one moment (SQLite's online backup)
- [ ] **1.0** A warning before saving into a format that can't hold something in the database, naming what would change, with the choice to keep a SQLite copy as well
- [ ] **1.0** Open and Repair: recover the tables, rows, and queries that can be read from a damaged SQLite, `.accdb`, or `.mdb` file into a new file, and say what was dropped
- [~] **first release** AutoRecover for unsaved design work in forms, reports, and queries *(shared: libs/shell recovery)*

### Print
- [~] **first release** Print, Quick Print, and Print Preview for the open datasheet, query answer, form, or report, with printer choice, copies, page ranges, selected records, and printer properties *(shared: JeffPub's print and print preview)*

### Options
- [~] **first release** General: user name and initials, light or dark interface, display language, usage statistics (opt in), update checks, the default folder for databases, and the default file type for new ones *(shared: libs/shell settings)*
- [ ] **first release** Current Database: the application title, which form opens at startup (it never opens a form that runs code until allowed), whether to show the Navigation pane, tabbed documents or overlapping windows, and whether Layout view is on
- [ ] **1.0** Datasheet: font, colors, gridlines, cell effect, default column width and row height
- [ ] **1.0** Object Designers: default field type and text size for new fields, automatic indexes for names like ID and Code, show table names in the query designer, output all fields, the font for the query designer and for SQL view, and the control and error checks in the form and report designers
- [~] **first release** Proofing: AutoCorrect, spelling as you type, ignoring words in capitals or with numbers, and custom dictionaries *(shared: JeffPub's Proofing options)*
- [ ] **1.0** Client Settings and Advanced: the Enter key moves to the next field, next record, or stays; arrow keys move between fields; select the whole field on entering it; confirm record changes, deletions, and action queries; default find and replace behavior; how many recent databases to list; four-digit years; default print margins; open the last database at start; the default open mode (shared or exclusive); and the time to wait on a busy file
- [ ] **1.0** Customize the Ribbon, the Quick Access Toolbar, and the keyboard: show, hide, rename, and reorder tabs and groups, add your own, assign keys to commands, and reset them
- [ ] **1.0** Privacy: remove personal information on save, and the choice about what is sent anywhere (statistics, crash reports) *(shared: libs/shell telemetry)*
- [ ] **first release** Trust Center: the list of what files may do (macros, VBA, remote links), the default of "ask, and keep disabled until allowed", and the list of files and folders the person has chosen to trust (see Macros and code)

## Home tab

### Views
- [ ] **first release** View button and menu: Datasheet View, Design View, Form View, Layout View, Report View, and SQL View for a query, each offered only for the objects that have it, with Print Preview among them; the top half switches to the object's default view

### Clipboard
- [~] **first release** Cut, Copy, and Paste of cells, rows, and columns in the datasheet, and of text in a box; copying makes tab-separated text, an HTML table, and rich text, and pasting takes the same from other programs, including JeffCrunch's cells *(shared: the suite's clipboard handling)*
- [ ] **first release** Paste Append: add copied rows to the end of a table when their fields match, by position or by name; and Paste as a new table when a spreadsheet range or a CSV is pasted onto the Navigation pane
- [ ] **1.0** Paste Special: as text, CSV, Unicode text, HTML, and JeffBase records (types kept), and Paste Table As (structure only, structure and data, or append the data to an existing table)
- [~] **1.0** Format Painter for controls on forms and reports and for datasheet formatting *(shared: JeffPub's Format Painter)*

### Sort & Filter
- [ ] **first release** Sort Ascending and Sort Descending on the selected column or on several side-by-side columns, and Remove Sort; the sort is part of the view and can be kept with the object
- [ ] **first release** The filter menu on each column heading: a check list of the values in the column with a search box, and the text filters (equals, does not equal, begins with, does not begin with, contains, does not contain, ends with, does not end with, between), number filters (the same comparisons), and date filters (equals, before, after, between, and all dates in a period such as a quarter or month)
- [ ] **first release** Selection filters: equals, does not equal, contains, does not contain, begins with, and ends with the value in the current cell, and before, after, or between for dates
- [ ] **first release** Toggle Filter and Clear All Filters; a filter indicator and a No Filter button show in the record bar; a filter and sort can be kept to apply when the object opens
- [ ] **1.0** Filter by Form (type the conditions into a blank row of the datasheet or form, with an Or tab for each alternative) and Advanced Filter/Sort (a small query grid over one table or form with expressions, with Apply Filter, Load from Query, and Save as Query)

### Records
- [ ] **first release** New Record (the blank row at the foot, marked with an asterisk), Save Record, and Delete Record for one or many selected rows, with a confirmation that says how many rows go and what depends on them
- [ ] **first release** Refresh All: read the rows again from the file, so changes made by another program show
- [ ] **1.0** Totals row at the foot of the datasheet: Sum, Average, Count, Maximum, Minimum, Standard Deviation, and Variance per column, as the type allows
- [~] **1.0** Spelling for text in the datasheet and forms *(shared: the bundled dictionaries and libs/shell spelling)*
- [ ] **first release** Row Height and Field Width (standard width, best fit), Hide Fields and Unhide Fields (a list of check boxes), and Freeze Fields and Unfreeze All Fields
- [ ] **1.0** Subdatasheets: a small plus sign at each row opens the related rows beneath it; Insert and Remove Subdatasheet, and Expand All and Collapse All

### Find
- [~] **first release** Find and Replace in a datasheet or form: look in the current field or the whole table; match any part, the whole field, or the start; search up, down, or all; match case; search fields as formatted; Find Next; and Replace All *(shared: the Find and Replace dialog in libs/page/app)*
- [ ] **first release** Go To: first, previous, next, last, and new record; Select and Select All records
- [ ] **first release** The record bar at the foot of the datasheet and form: first, previous, a record number box, next, last, new, a filter indicator, and a search box that finds as the person types

### Text Formatting
- [~] **first release** Font, size, bold, italic, underline, font color, background color, alternate row color, column alignment, and gridlines (both, horizontal, vertical, or none) for the whole datasheet *(shared: JeffPub's font and color pickers)*
- [ ] **1.0** The Datasheet Formatting dialog: cell effect (flat, raised, sunken), gridline color, background and alternate colors, and border and line styles
- [ ] **1.0** Rich text in Long Text fields set to Rich Text: bold, italic, underline, color, highlight, bullets, numbering, indents, alignment, and text direction, kept in the HTML form Access stores

## Create tab

### Templates
- [ ] **1.0** Application Parts and table templates: ready-made pieces added to the open database (Contacts, Tasks, Issues, Users, Comments, and blank forms in a few layouts, each with its table, form, and report) and groups of related fields (an address, a name, a phone number) added to a table at once

### Tables
- [ ] **first release** Table: a new blank table in the datasheet, with an AutoNumber ID and a heading that says Click to Add, where the next field is typed or its type chosen
- [ ] **first release** Table Design: a new table in Design view (details under Tables and fields)

### Queries
- [ ] **first release** Query Design: a new query in the designer, with the Show Table box
- [ ] **first release** Query Wizard, Simple: pick the fields from one or more tables, choose detail or summary, and name it
- [ ] **1.0** Query Wizards for Crosstab, Find Duplicates, and Find Unmatched records

### Forms
- [ ] **first release** Form: one click builds a stacked form for the selected table or query; Blank Form (starts in Layout view), Form Design, and the Form Wizard (fields, layout, and title)
- [ ] **1.0** More Forms: Multiple Items (rows as a list of form rows), Datasheet (a form that looks like the grid), Split Form (grid and form together), and Modal Dialog (a small form that must be closed first)
- [ ] **1.0** Navigation forms: tabs along the top, down the left, or down the right, with a second row of tabs in the five combinations, each tab showing a form or report

### Reports
- [ ] **first release** Report: one click builds a tabular report for the selected table or query; Blank Report, Report Design, and the Report Wizard (fields, grouping levels, sort order, layout, and orientation)
- [ ] **1.0** Labels: a wizard that makes a report of mailing or product labels on the common sheet sizes or a custom size, with the fields placed, and barcodes from a field in a label or any report *(shared: libs/base/core barcode, and the label sizes JeffPub's several-to-a-sheet printing uses)*

### Macros and Code
- [ ] **later** Macro, Module, and Class Module: create them for a database (see Macros and code); Visual Basic is not offered

## External Data tab

The commands only; what each import and export does is under Import, export, and linking.

- [ ] **first release** New Data Source > From File: Excel workbook, Text File (CSV, tab, or other delimiters, or fixed width), and XML; each can import into a new table, append to a table, or link
- [ ] **1.0** New Data Source > From File: HTML document (a table in a web page), and JSON *(shared: JeffCrunch's data-source layer)*
- [ ] **first release** New Data Source > From Database: another JeffBase or SQLite database, and Access (`.accdb` and `.mdb`)
- [ ] **later** New Data Source > From Database: dBASE files, and ODBC databases (see Import, export, and linking)
- [ ] **1.0** Saved Imports and Saved Exports: steps saved by name and run again, with the file path and the settings kept
- [ ] **1.0** Linked Table Manager: lists every linked table, shows whether each link works, refreshes it, and relinks it to a new place
- [ ] **first release** Export > Excel, Text File, XML, PDF, and another JeffBase, SQLite, or Access `.accdb` database
- [ ] **1.0** Export > Email (as an attachment), Word Merge (Merge to JeffWrite), and More: a document with the data (`.docx`, `.rtf`), HTML, and dBASE

## Database Tools tab

- [ ] **first release** Compact & Repair Database (see Info)
- [ ] **1.0** Object Dependencies pane: for the selected object, the objects that use it, and the objects it uses
- [ ] **1.0** Database Documenter: a report on any chosen objects with their properties, fields, indexes, relationships, permissions, and (for forms and reports) controls and their properties; printed, saved as PDF, or exported
- [ ] **1.0** Analyze Performance: looks at tables, queries, forms, and reports and suggests indexes, relationships, and changes, with a button to apply each one
- [ ] **1.0** Analyze Table: finds repeated values in one table and offers to split it into related tables, with a preview and the new relationships
- [ ] **later** Split Database (Move Data > Access Database): move the tables into a back-end file and link to them from the front end, so several copies of the front end can share one set of data

## Help tab

- [~] **1.0** Help (offline), What's New, and Contact Us (a link to the project's pages) *(shared: libs/shell help)*

## Contextual tabs

The tabs that appear only while an object is open or selected: a table in the datasheet or in Design view, a query, a form, a report, the relationships, and a macro, and Layout view, where the form and report tabs apply to the real rows.

### Table Fields and Table tabs (datasheet view)

- [ ] **first release** Fields tab, Add & Delete: Short Text, Number, Currency, Date & Time, and Yes/No buttons add a field of that type at the cursor; a More Fields gallery of ready-made kinds (name, address, phone, priority, status, tag) adds a field with its settings; Delete removes the column after a warning
- [ ] **first release** Fields tab, Properties and Formatting: Name & Caption, Default Value, Field Size, Modify Lookups, and Modify Expression, each opening the matching small dialog or pane; the Data Type and Format boxes; Currency, Percent, and Comma buttons; and Increase and Decrease Decimals
- [ ] **first release** Fields tab, Field Validation: Required, Unique, and Indexed check boxes; Field Validation Rule and Message; and Table Validation Rule and Message
- [ ] **1.0** Fields tab: Long Text settings (Append Only, Rich Text), and the Memo Settings menu
- [ ] **first release** Table tab: Table Properties, and Relationships and Object Dependencies buttons

### Table Design (Design view)

- [ ] **first release** Design view: a grid of Field Name, Data Type, and Description, with the field's property sheet beneath; rows are inserted, deleted, and moved by dragging; the key symbol marks the primary key
- [ ] **first release** Primary Key button (one field or several selected together), Insert Rows and Delete Rows, and Test Validation Rules (checks existing rows against a new rule and reports the ones that fail)
- [ ] **first release** Indexes window: name, field, and sort order per row; Primary, Unique, and Ignore Nulls; indexes over several fields
- [ ] **first release** Property Sheet for the table (see Tables and fields), and Modify Lookups

### Query Design tab

- [ ] **first release** Query Type: Select
- [ ] **1.0** Query Type: Make Table, Append, Update, Crosstab, and Delete; Union and Data Definition (typed as SQL)
- [ ] **later** Query Type: Pass-Through, which sends text to a database server (see Macros and code and Import, export, and linking)
- [ ] **first release** Query Setup: Show Table, Insert and Delete Rows and Columns, the Builder (Expression Builder), and Return (All, 5, 25, 100, a number, or a percent of the rows)
- [ ] **first release** Show/Hide: Totals (adds the Total row), Parameters (the box that names each prompt and its type), Property Sheet, and Table Names
- [ ] **first release** The designer and Results group (Run, and the views menu): field lists at the top with join lines, the grid below (Field, Table, Sort, Show, Criteria, Or), columns resized and dragged, a field list's Add All (the star) and double-click to add a field

### Form tabs: Design, Arrange, and Format

- [~] **1.0** Design tab, Themes: the Themes, Colors, and Fonts galleries, with Browse for themes and Save Current Theme *(shared: color schemes and font schemes in JeffPub's templates, and the themes format planned for the suite)*
- [ ] **first release** Design tab, Controls: the gallery of controls (see Forms), Use Control Wizards, Set Control Defaults, and Insert Image with a shared gallery of pictures kept in the database
- [ ] **first release** Design tab, Header / Footer and Tools: Logo, Title, and Date and Time; Add Existing Fields (the field list), Property Sheet, Tab Order, and Subform in New Window
- [ ] **first release** Arrange tab, Table and Rows & Columns: Stacked and Tabular layouts, Remove Layout, Gridlines (style, width, and color), Insert Above, Below, Left, and Right, and Select Layout, Column, and Row
- [ ] **1.0** Arrange tab, Merge/Split and Move: merge cells of a layout, split a cell vertically or horizontally, and move a row up or down
- [~] **first release** Arrange tab, Position and Sizing & Ordering: control margins and padding, anchoring (which edges a control keeps when the window resizes), Size to Fit, to Tallest, Shortest, Widest, and Narrowest, spacing, Align (left, right, top, bottom, to grid), Bring to Front and Send to Back, Group and Ungroup, Snap to Grid, and the Grid and Ruler *(shared: JeffPub's Arrange group, snap, and rulers)*
- [~] **first release** Format tab: Selection (the object box and Select All), Font, size, bold, italic, underline, alignment, font and fill colors, Format Painter, and Alternate Row Color *(shared: JeffPub's font and color pickers)*
- [ ] **first release** Format tab, Number: the Format box, Currency, Percent, Comma, and decimals
- [ ] **1.0** Format tab, Control Formatting: Quick Styles and Change Shape, Conditional Formatting, Shape Fill, Outline (color, thickness, and type), and Shape Effects (shadow, glow, soft edges, and bevel) *(shared: libs/draw effects)*
- [ ] **1.0** Form Datasheet tab: when a form shows as a datasheet, Themes, the Field List and Property Sheet, background and alternate colors, and Conditional Formatting

### Report tabs: Design, Arrange, Format, and Page Setup

- [ ] **first release** Design tab: the same Views, Themes, Controls, Header / Footer, and Tools groups as a form, with Page Numbers added to Header / Footer, and Subreport in New Window
- [ ] **first release** Design tab, Grouping & Totals: Group & Sort (the pane at the foot), Totals (Sum, Average, Count Records, Count Values, Maximum, Minimum, Standard Deviation, Variance), and Hide Details
- [ ] **1.0** Arrange and Format tabs: the same as a form's (layouts, merge and split, position, size, order, fonts, colors, shape styles, and conditional formatting)
- [ ] **first release** Page Setup tab: page size, margins (Normal, Wide, Narrow, and custom), Show Margins, Print Data Only, Portrait and Landscape, Columns, and the Page Setup dialog *(shared: JeffPub's page setup)*
- [~] **first release** Print Preview tab: Print, page size and margins, orientation, Columns, zoom (a percent, One Page, Two Pages, and More Pages), refresh, export to Excel, text, PDF, and email, and Close Print Preview *(shared: JeffPub's print preview)*

### Relationship Design tab

- [ ] **first release** The Relationships window: each table as a box of its fields with the key marked, lines for relationships with 1 and many marks at the ends, tables dragged where wanted, and the layout saved with the file
- [ ] **first release** Edit Relationships: choose the tables and fields, Enforce Referential Integrity (the program refuses a row that points nowhere, and refuses to delete a row that others point to), Cascade Update Related Fields, Cascade Delete Related Records, and the join type (rows from both tables that match, all rows of one side, or all rows of the other); the kind (one-to-many, one-to-one) is told from the keys
- [ ] **first release** Show Table, Hide Table, Show Direct Relationships, Show All Relationships, and Clear Layout; delete a relationship from its line
- [ ] **1.0** Relationship Report: prints the diagram as a report

### Macro Design tab

- [ ] **later** Macro builder: the Action Catalog, Run, Single Step, Expand and Collapse for blocks, Show All Actions, and Save, Save As, and Close; offered only if JeffBase ever runs macros (see Macros and code)

### Layout View

- [ ] **first release** Layout view for forms and reports: the real rows show while the design is changed, so a text box can be sized to the data it holds; fields come from the field list by dragging; controls move and resize in a layout together
- [ ] **first release** Stacked layouts (label beside box, one under another) and tabular layouts (labels as a header row, boxes as columns) hold controls in a grid, so adding a field keeps everything aligned
- [ ] **1.0** Report Layout view adds the Group, Sort, and Total pane, column widths from the data, and page breaks drawn as the report will page

## Shortcut menus (right-click)

What a right-click offers depends on what is under the pointer.

- [ ] **first release** Object actions on a right-click: Open, Design View, Layout View, Rename, Copy, Paste, Delete (with a warning that lists what depends on the object), Hide in This Group, Import, Export, Linked Table Manager, Properties, and Object Dependencies
- [ ] **first release** In the datasheet: Cut, Copy, Paste, Sort Ascending and Descending, Filter by the value under the pointer, Find, Delete Record, and Row Height and Column Width
- [ ] **first release** In a form or report design: Cut, Copy, Paste, Delete, Align, Size, Bring to Front and Send to Back, Properties, and Build Event (which only lists code, since nothing runs until allowed)
- [ ] **1.0** In the query designer: on a field, Build, Properties, and Totals; on a table in the diagram, Remove Table, Add Table, and Join Properties; and on the grid, Cut, Copy, Paste, and Delete
- [ ] **1.0** On a relationship line in the Relationships window: Edit Relationship, Delete, and Show Table

## Under the ribbon: the Navigation pane, tabs, and status bar

- [ ] **first release** The Navigation pane on the left lists the database's objects (tables, queries, forms, reports, macros, modules) and opens one on a double-click; it can be grouped by object type, by created or modified date, by table and related views, or by custom groups, filtered by name, shown as a narrow strip, and set to show hidden and system objects (the tables Access names with `MSys` and `USys` and JeffBase's own catalog tables, kept and never changed by the person's actions), with custom groups the person drags objects into and shortcuts to objects in other databases
- [~] **first release** Open objects are tabs in a row above the work area (the default); the person can switch to overlapping windows in Options, with Switch Windows, Size to Fit Form, Fit to Window, Tile, and Cascade *(shared: JeffPub's windows, for the overlapping mode)*
- [ ] **first release** The status bar names the view and its state (Datasheet View, Design View, filtered, num lock) and has the view buttons and the zoom slider *(shared: JeffPub's and JeffDeck's status bars)*
- [~] **first release** Collapse the ribbon with Ctrl+F1, a double-click on a tab, or its chevron; KeyTips (Alt, then letters) for every command *(shared: libs/shell ribbon and KeyTips)*
- [ ] **1.0** Quick Access Toolbar: New, Open, Save, Print Preview, Spelling, Undo, Redo, Refresh All, and the Views button, with room for any command

## Under the ribbon: tables and fields

How a table is made, what a field can hold, and how the rules on it work. In the native file each Access type is stored as the SQLite type that other tools read naturally, and the exact Access type is kept in the file's field catalog, so a field comes back as the same kind after a round trip.

### Data types
- [ ] **first release** Short Text (up to 255 characters) and Long Text (long notes, up to a billion characters), stored as Unicode text
- [ ] **first release** Number, with Byte, Integer, Long Integer, Single, Double, and Decimal sizes; each size refuses a value that doesn't fit
- [ ] **first release** Date/Time, stored as ISO 8601 text in the native file so every other tool reads and sorts it, and converted to Access's day-number form in `.accdb`
- [ ] **first release** Currency, exact to four decimal places, with no rounding error in sums *(how it is stored in the native file is an open question)*
- [ ] **first release** AutoNumber: a whole number that counts up for each new row (increment) and is never reused after a delete; the key of most tables
- [ ] **first release** Yes/No, shown as a check box, Yes/No, True/False, or On/Off; in expressions True is -1 as in Access, and the stored value is converted to 0 and 1 in the native file
- [ ] **first release** Hyperlink: text to show, an address, a place within it, and a tip, kept as Access keeps them, and opened only when the person clicks (and the address is shown first if it isn't a web page)
- [ ] **1.0** Attachment: any number of files in one field, shown as a paperclip with a count; add, remove, open, and save out; stored in the file *(shown read-only and kept in the first release)*
- [ ] **1.0** Calculated: a field whose value is an expression of the other fields in the row, with a result type; shown but not typed into
- [ ] **1.0** Lookup Wizard: a field whose value is picked from another table's rows or from a typed list (details under Lookup fields)
- [ ] **1.0** Large Number (64-bit whole numbers), Date/Time Extended (years 1 to 9999 with 100-nanosecond steps), and Replication ID (a 128-bit unique number, shown in braces)
- [ ] **1.0** Multi-value fields: one field holds several picked values, kept in a hidden linking table the person never sees *(shown read-only and kept in the first release)*
- [ ] **later** OLE Object: pictures and documents embedded by other programs are shown as a picture when they hold one and kept as they are; they are not edited in place, which needs Windows

### Field properties
- [ ] **first release** Field Size, Required, Allow Zero Length, Default Value, Caption, and Description
- [ ] **first release** Format: the named formats for numbers (General Number, Currency, Fixed, Standard, Percent, Scientific), dates (General Date, Long, Medium, Short, and the time forms), and Yes/No, and custom format codes *(shared: JeffCrunch's number-format language where the codes overlap; Access's own text codes are added)*
- [ ] **first release** Decimal Places, and Text Align
- [ ] **1.0** Input Mask with the Input Mask Wizard (phone number, postal code, Social Security number, password, dates, and times) and the mask characters that require or allow a digit, a letter, or either, change case, or add fixed characters stored or not
- [ ] **1.0** Show Date Picker, Text Format (plain or rich), Append Only (a Long Text that keeps every earlier version), and IME settings for East Asian input, and Unicode compression and other file-size settings (kept as saved, and set for new `.accdb` fields)

### Rules on data
- [ ] **first release** Validation rule and message for a field and for a whole row, written as an Access expression (`>=0 And <=100`, `Like "A*"`, `[End] > [Start]`), enforced on every edit and every query that writes, with the message shown in plain words
- [ ] **first release** Primary key on one field or several; Required; Unique through an index; and clear, plain-language messages when a rule stops a change (a missing related row, a duplicate, a value that is too long or the wrong type)
- [ ] **first release** Default values that are fixed or are expressions such as `Date()` and `Now()`, filled in when a new row starts
- [ ] **1.0** The rules are written as SQLite `CHECK` constraints and `DEFAULT` clauses when the expression translates, so other programs obey them too; when it doesn't translate, JeffBase checks it and says so in the property sheet
- [ ] **first release** Relationships with referential integrity are SQLite foreign keys, switched on for every connection JeffBase opens (SQLite leaves them off unless told), with cascade update and cascade delete

### Lookup fields
- [ ] **1.0** Lookup properties: display control (text box, combo box, or list box), row source type (table or query, value list, or field list), row source, bound column, column count, column heads and widths, list rows and width, limit to list, allow value list edits, and show only row source values
- [ ] **1.0** A lookup field stores the key of the row it points to and shows the readable value (a city's name, not its number) in the datasheet, in forms, and in queries' output; other tools see the plain key and, through a view, the readable form; in the datasheet the lookup combo box fills in as the person types and adds a new row to the source table when allowed

### Indexes
- [ ] **first release** Indexed: Yes (duplicates allowed), Yes (no duplicates), or No, and indexes over several fields, kept as SQLite indexes
- [ ] **1.0** Ignore Nulls (rows with no value are left out of the index), written as a partial index; and automatic indexes on fields named ID, Key, Code, or Num when a table is made or imported (an option)

### Table properties and names
- [ ] **first release** Table properties: Description, Default View, Validation Rule and Text, Filter and Order By (with Filter On Load and Order By On Load), and Orientation
- [ ] **1.0** Subdatasheet properties, which control the related rows shown under each row: Subdatasheet Name, Link Child Fields, Link Master Fields, Height, and Expanded
- [ ] **1.0** Rename a table or a field and every query, form, report, validation rule, lookup, and relationship that uses the name follows (Name AutoCorrect), with a list of what was changed
- [ ] **first release** Names: up to 64 characters of letters, digits, spaces, and most punctuation; `.`, `!`, `[`, `]`, and the backtick are not allowed, so the file stays readable by Access, and a SQLite file from elsewhere that has such a name opens with a note

### Sort order and text
- [ ] **first release** Text compares and sorts the way Access does by default: without regard to case or accents, by the order of the person's language, so "apple" and "Apple" are the same in a query's criteria and sort
- [ ] **1.0** The database sort order (General, or a language's rules), chosen when a database is made and shown in its properties; a file opened from Access keeps the order it was made with

### Entering data
- [ ] **first release** Data entry in the datasheet and forms: Tab and Enter to move, F2 to edit a cell, Shift+F2 for a zoom box on long text, Ctrl+; for today's date, Ctrl+' to copy the value above, and Esc once to undo the field and twice to undo the row; a pencil mark shows on a row being edited, and the row commits when the person moves to another, closes the object, or presses Shift+Enter
- [ ] **1.0** Edit Hyperlink dialog, the Attachments dialog (Add, Remove, Open, Save As, Save All), and a date picker that opens from the calendar button on a date cell

## Under the ribbon: queries

A query is a saved question. A select query shows rows; the other kinds change them. In the native file a select query is saved as a SQLite view (so every other tool can read its answer), and the part a view cannot hold (the designer's drawn layout, the prompts of a parameter query, and the action queries) is kept in JeffBase's catalog.

### Select queries and the designer
- [ ] **first release** Select query over one table or several: choose tables in the Show Table box, drag fields into the grid, set Sort and Show, and type Criteria and Or rows; the answer opens in a datasheet
- [ ] **first release** Criteria: comparison operators, `Between ... And`, `In (...)`, `Is Null` and `Is Not Null`, `Not`, `And` and `Or` across rows and columns, `Like` with wildcards, date values written `#10/15/2026#`, and text in quotes; wildcard matching works on any Unicode letter, not only the ones SQLite's own `LIKE` knows
- [ ] **first release** Joins: drag a field to another to join; inner, left outer, and right outer joins chosen in the Join Properties box; joins from relationships appear on their own (AutoJoin); more than one join field; a table joined to itself (give it an alias)
- [ ] **1.0** Joins on `<`, `>`, `<>`, and other comparisons (non-equi joins), and a cross join (a table with no join line, which pairs every row with every row), with a warning where Access also warns
- [ ] **first release** Calculated fields: `Total: [Qty] * [Price]`, using the field names in brackets, the Access operators (`&` to join text, `\` for whole-number division, `Mod`, `^`), and the built-in functions
- [ ] **first release** The Expression Builder: a dialog with three panes (elements such as functions, tables, and fields; their categories; and the values) that builds an expression by double-clicks, with completion while typing *(shared: JeffCrunch's formula completion)*
- [ ] **first release** Totals: Group By, Sum, Avg, Min, Max, Count, StDev, Var, First, Last, Expression, and Where in the Total row
- [ ] **first release** Return the top rows (5, 25, 100, a number, or a percent), and the Unique Values and Unique Records properties
- [ ] **first release** Parameter queries: `[Enter the start date]` in a criterion asks the person when the query runs; the Parameters box sets each one's type
- [ ] **1.0** Criteria that read an open form's box (`[Forms]![Orders]![StartDate]`) or a report's, as many Access databases do
- [ ] **1.0** Query properties: Description, Default View, Output All Fields, Top Values, Unique Values and Records, Source Database, Source Connect Str, Record Locks, Recordset Type, Filter, Order By, Max Records, Orientation, ODBC Timeout, and the field properties of an output column (Format, Decimal Places, Input Mask, Caption)
- [ ] **first release** Editing in a query's answer: for a query over one table, a change goes to that table's row; the rows of a one-to-many query can be edited when Access would allow it; the status bar tells when a query can't be edited and why
- [ ] **1.0** Editing in the answer of a join over several tables, adding a row through the "many" side with the key filled in from the parent, and the Recordset Type choices (dynaset, snapshot)

### Query kinds
- [ ] **1.0** Make Table, Append, Update, and Delete queries: save a query's answer as a new table (as in Access, the new table gets no primary key or indexes); add a query's rows to a table, matching fields by name; change a field's value in the rows that match, with an expression as the new value; or remove the rows that match, from one table or one side of a join; each shows a count of the rows before it runs
- [ ] **1.0** Action queries run in a transaction and ask "this will change N rows" before they commit; the person can say no and nothing changes, and a failure part way through leaves the data as it was
- [ ] **1.0** Crosstab: row headings, column headings, and a summary value in a grid (`TRANSFORM ... PIVOT`), with fixed column headings when given
- [ ] **1.0** Queries only SQL view can write: Union (`UNION` and `UNION ALL`), subqueries in a criterion or field list (`IN (SELECT ...)`, `EXISTS`), and Data Definition queries (`CREATE TABLE`, `ALTER TABLE`, `CREATE INDEX`, and `DROP`)
- [ ] **1.0** Find Duplicates and Find Unmatched queries, from their wizards, written out as ordinary queries
- [ ] **later** Pass-Through: text sent unchanged to a database server over ODBC, run only after the person allows it (see Macros and code)

### SQL view
- [ ] **first release** SQL view: the query as text, editable; changes show in the designer when the designer can draw them, and the view says so when it can't (a union or a subquery in the From part)
- [ ] **first release** Access's own SQL is read and written (brackets around names, `&`, `Like "a*"`, `IIf`, `Nz`, `#date#`, `TOP`, `DISTINCTROW`, `PARAMETERS`, `TRANSFORM`), and the same query can be shown as SQLite's SQL, which is what actually runs *(see the open questions)*
- [ ] **first release** The functions of Access's expression language that queries use: text (`Left`, `Mid`, `Right`, `Trim`, `InStr`, `Len`, `UCase`, `LCase`, `Replace`, `StrConv`, `Format`), numbers (`Abs`, `Int`, `Fix`, `Round`, `Sgn`, `Sqr`, `Rnd`), dates (`Date`, `Now`, `DateAdd`, `DateDiff`, `DatePart`, `DateSerial`, `Year`, `Month`, `Day`, `Weekday`, `MonthName`), conversion (`CStr`, `CInt`, `CLng`, `CDbl`, `CCur`, `CDate`, `Val`), and logic (`IIf`, `Switch`, `Choose`, `Nz`, `IsNull`, `IsNumeric`, `IsDate`), with Access's `Null` rules *(shared: JeffCrunch's calculation engine where a function means the same)*
- [ ] **1.0** Domain aggregate functions in expressions (`DLookup`, `DCount`, `DSum`, `DAvg`, `DMax`, `DMin`, `DFirst`, `DLast`), and the date and text functions less often used (`Format` with every named and custom code, `Partition`, `StrReverse`, `Space`, `String`)
- [ ] **1.0** A query's SQL as saved in an `.accdb` is kept as written (comments, spacing, and Access-only clauses included) so it goes back unchanged, while the SQLite form is built from it for running
- [ ] **1.0** The SQL clauses that name another file (`IN 'file.mdb'`) are shown, never followed until the person allows it
- [ ] **1.0** Show the query plan (which indexes SQLite uses) beside the SQL, and Analyze Performance suggestions that point at it

### Running and results
- [ ] **first release** A query runs in the background and shows its first rows at once; a long one can be canceled, and the window stays usable
- [ ] **first release** Datasheet of the answer: the same sort, filter, find, totals, hide, and freeze as a table; the answer's columns keep the format, caption, and decimals of the fields they come from
- [ ] **first release** A query can be used wherever a table can: as the source of another query, a form, a report, a lookup, and a mail merge

## Under the ribbon: forms

A form shows one row (or a few) of a table or query in a layout the person arranges. The wizards and the one-click Form button make the usual ones; the designer makes the rest.

### Building a form
- [ ] **first release** Form views: Form view (edit data), Layout view (change the design with live data), Design view (exact placement), and Datasheet view of the form's source; a form's Default View property chooses single form, continuous forms (rows as repeated copies of the design), datasheet, or split form
- [ ] **first release** Controls on the form, from the gallery: label, text box, combo box, list box, check box, option group with option buttons and toggle buttons, button, image, line, rectangle, and hyperlink; bound to a field (changes go to the row) or unbound (a typed value or a calculation)
- [ ] **first release** Calculated controls: a text box whose Control Source is an expression (`=[Qty] * [Price]`) using the same functions as queries, updating as the row changes
- [ ] **1.0** More controls: tab control (pages of controls), subform and subreport (a related table's rows inside the form, linked by Link Master and Child Fields), navigation control, attachment, page break, chart, and the bound and unbound object frames (pictures and documents embedded by other programs, shown and kept) *(shared: libs/office chart for charts)*
- [ ] **later** The web browser control and ActiveX controls: not offered; a form that has one keeps it and shows a gray box (see Left out on purpose)
- [ ] **first release** Button actions that need no code: go to a record (first, previous, next, last, new), save, delete, undo, close the form, open another form or report (with a filter), print, and refresh; the Command Button Wizard picks one and stores it as a plain action JeffBase runs itself **(alt)** (Access writes VBA for these)
- [ ] **1.0** More button actions: run a saved query, apply a filter, export to PDF or a file, send the row as an email through JeffMail, and open a file or a web page after the person confirms
- [ ] **first release** Form properties: Record Source, Caption, Default View, Allow Edits, Allow Additions, Allow Deletions, Data Entry, Filter, Order By, Scroll Bars, Record Selectors, Navigation Buttons, Dividing Lines, Auto Center, Border Style, Pop Up, Modal, Cycle, and Min/Max buttons
- [ ] **first release** Control properties: Control Source, Format, Decimal Places, Input Mask, Default Value, Validation Rule and Text, Enabled, Locked, Visible, Tab Stop and Tab Index, Control Tip Text, Special Effect, Can Grow and Can Shrink, and the font, color, and border settings, shown in a Property Sheet with tabs for Format, Data, Event, Other, and All
- [ ] **first release** Tab Order: a dialog that lists controls in order and drags them; Auto Order
- [ ] **1.0** Conditional formatting on controls: rules by value or expression that change the font, fill, or border, with data bars for number fields
- [~] **1.0** Themes, theme colors, and theme fonts for a whole database, so a change recolors every form and report *(shared: color schemes and font schemes from JeffPub; the themes gallery planned for the suite)*
- [ ] **1.0** Anchoring: controls keep their distance from chosen edges so a form fills a larger window
- [~] **1.0** Pictures and icons: a logo and background picture, images from the database's shared pictures or from files, pictures stored in a field, and icons on buttons *(shared: JeffPub's picture handling and icon picker)*

### Using a form
- [ ] **first release** The record bar (first, previous, number box, next, last, new, filter, and search) at the foot of every form, unless turned off
- [ ] **first release** Entering data follows the datasheet's rules (required and validation messages, defaults, input masks, combo boxes that suggest as the person types, and a clear message when a rule blocks a change); a row is saved when the person moves off it, a form closed with an unsaved change asks first, and Esc undoes
- [ ] **first release** Sort and filter a form's rows by the same commands as the datasheet; Find and Replace across the form's rows
- [ ] **1.0** Main and detail forms: a customer on top and the customer's orders below in a subform, with new orders getting the customer's key by themselves
- [ ] **1.0** Split Form, Multiple Items, Datasheet form, Modal Dialog, and Navigation forms, as created from the Create tab
- [ ] **first release** Print a form, and save it as PDF *(shared: JeffPub's print and PDF export)*

## Under the ribbon: reports

A report lays the rows of a table or query out on pages, in groups, with totals. It shares JeffWrite's page layout and printing, so pages break and print the same way in both.

### Building a report
- [ ] **first release** Report views: Report view (on screen, where a reader can sort, filter, and find without changing the design), Print Preview (pages as they print), Layout view (change the design with live data), and Design view
- [ ] **first release** Sections: Report Header and Footer, Page Header and Footer, Group Header and Footer for each level, and Detail; controls placed in them as on a form, and the heights and backgrounds of sections set; sections can be hidden
- [ ] **first release** Grouping and sorting: group on a field or an expression, with intervals for text (first characters) and dates (day, week, month, quarter, year), up to ten levels; sort within; the Group, Sort, and Total pane at the foot
- [ ] **first release** Totals and counts in group and report footers: Sum, Average, Count, Min, Max, Standard Deviation, and Variance; percent of the group's or report's total; and Running Sum, over a group or over all
- [ ] **first release** Page numbers and the date and time (`=[Page]`, `=[Pages]`, `=Now()`), and a logo and title in the header
- [ ] **first release** Report Wizard: fields, grouping levels, sort order and summary choices, Stepped, Block, or Outline layout, portrait or landscape, and a title
- [ ] **1.0** Section properties: Force New Page, New Row Or Col, Keep Together, Repeat Section, Can Grow and Can Shrink (a box grows with its text, and shrinks when empty), and Hide Duplicates
- [ ] **1.0** Multiple-column reports and label sheets: column count, size, spacing, and the order (down then across, or across then down), with the label sizes of the common sheets
- [ ] **1.0** Subreports: a report inside a report, linked by fields (the orders of each customer)
- [ ] **1.0** Charts in a report: column, bar, line, and pie, from a query, drawn in print and in PDF *(shared: libs/office chart)*
- [ ] **1.0** Conditional formatting in a report, by value or by expression

### Pages, print, and sending out
- [ ] **first release** Report pages use the flow-document layout shared with JeffWrite: sections that grow with their rows, pages that break at group ends, and headers and footers that repeat *(needed by: JeffWrite, JeffMail, JeffBase)*
- [ ] **first release** Page Setup for a report: page size, margins, orientation, columns, Show Margins, and Print Data Only (no labels or lines) *(shared: JeffPub's page setup)*
- [~] **first release** Print and print preview of a report with page navigation, zoom, and many pages at once *(shared: JeffPub's print preview)*
- [ ] **first release** Save a report as a PDF (with bookmarks from the groups, and PDF/A) *(shared: JeffPub's PDF export)*
- [ ] **1.0** Export a report as a JeffWrite document (`.docx`), HTML, or plain text with the page boundaries marked **(alt)** (Access exports to Word's RTF)

## Under the ribbon: macros and code

Access databases often hold macros (saved steps) and VBA (programs). They can do anything the person can, so the rule for the whole suite applies in full: nothing in a file runs or reaches out until the person allows it. What JeffBase keeps, shows, and runs is split below.

### Kept, never lost
- [ ] **first release** Everything an opened `.accdb` or `.mdb` holds that JeffBase doesn't run is kept untouched and written back on Save: macros (including AutoExec and AutoKeys), data macros on tables, VBA modules, class modules, form and report code, references, the compiled state, and startup settings
- [ ] **first release** Save a copy without macros and code: a database with only its tables, queries, forms, and reports (no programs), for sharing
- [ ] **1.0** Read-only listing of what would run: each macro's actions and arguments, each module's source text (VBA source is stored compressed in the file, and the format is documented), and which form events call them, so a person can look before allowing; and the Object Dependencies pane lists the objects that call a macro or module, as far as that can be read

### Run, only if allowed
- [ ] **later** Run Access's macro actions after the person allows them: the builder's actions (open or close a form or report, run a query, set a value, go to a record, show a message, run another macro), conditions, sub-macros, and error handling; JeffBase's own interpreter runs them, not Windows (see the open questions)
- [ ] **later** Data macros (table events) as SQLite triggers where the logic is plain: before change, after insert, after update, and after delete, with named macros called from them
- [ ] **later** An open scripting language for people who want to automate JeffBase, in place of VBA (see the open questions)
- [ ] **later** VBA is not run: no VBA engine is planned, since VBA depends on Microsoft's runtime and Windows libraries; a database that needs VBA to work shows what stopped and keeps the code for Access

## Under the ribbon: import, export, and linking

Three different things: **import** copies data into a table, **link** leaves the data where it is and reads it from there, and **export** writes data out. The saved-steps buttons and the wizards are the same on every kind.

### Import
- [ ] **first release** Text and CSV import wizard: choose the file; delimited by comma, tab, semicolon, space, or another character, or fixed width with column lines dragged on a preview; text qualifier; first row as field names; the file's encoding (UTF-8, UTF-16, and the Windows and ISO code pages) detected and changeable; a preview of the first rows
- [ ] **first release** Per-column choices on import: field name, data type (guessed from the first rows and changeable), indexed or not, and skip; a choice of primary key (let JeffBase add one, use a column, or none); and a table that collects the rows that didn't fit (type errors) with the row numbers
- [ ] **first release** Import into a new table, or append to an existing one with fields matched by name or by position; a dry run reports how many rows would go in and which would fail
- [~] **first release** Excel and spreadsheet import: a sheet or a named range from `.xlsx`, `.xls`, and `.ods`, with the first row as names and cell types read as they are (dates stay dates) *(shared: JeffCrunch's readers, which JeffPub's merge-source reader gives way to)*
- [ ] **first release** Import tables, queries, and relationships from another JeffBase, SQLite, `.accdb`, or `.mdb` file; choose which objects, and whether to bring the data
- [ ] **1.0** Import XML (with the shape it finds shown and the structure choosable), HTML tables from a saved page, and JSON *(shared: the data-source layer JeffCrunch plans)*
- [ ] **1.0** Saved Imports: the steps (file, options, and mapping) saved by name and run again, as often as needed
- [ ] **later** Import dBASE (`.dbf`) files, and import from an ODBC data source (a copy of the server's table)
- [ ] **later** Import contacts from vCard files into a table *(shared: the vCard library JeffMail plans)*

### Export
- [ ] **first release** Export a table or a query's answer to CSV or other delimited text (delimiter, qualifier, field names in the first row, encoding, and date and number format), and to `.xlsx` (with types, widths, and a header row) *(shared: JeffCrunch's workbook writer)*
- [ ] **first release** Export formatted: the data as the datasheet shows it (the captions, formats, and lookup text), or the stored values
- [ ] **1.0** Export to `.ods` and `.xls`, XML (with a schema), HTML, JSON, and dBASE
- [ ] **1.0** Saved Exports, and an Export that writes only the filtered, sorted rows currently on screen
- [ ] **1.0** Export a table or query as SQL text (`CREATE TABLE` and `INSERT` statements) that rebuilds it in another database
- [ ] **1.0** Mail merge: JeffWrite, JeffPub, and JeffMail read a JeffBase table or query as the recipient list, live (the list follows the data), with the filter and sort kept; "Merge to JeffWrite" on the Export menu starts the merge with a table or query chosen *(needed by: the merge-source library; shared: JeffPub's mail merge)*

### Linking
- [ ] **first release** Link to a text or CSV file or a spreadsheet: the table shows the file's rows and follows changes to the file; reading only at first
- [ ] **1.0** Link to a table in another SQLite or JeffBase file (attached while open), read and write, with the link kept as a path and checked on open; Link Table Manager shows each link's state and relinks
- [ ] **1.0** A link to a file on a network share or any other computer is treated as a remote link: it is not followed until the person allows it
- [ ] **later** Link to `.accdb` and `.mdb` tables, read-only first
- [ ] **later** ODBC linked tables for database servers (PostgreSQL, MySQL, MariaDB, SQL Server): a saved connection with no password in the file unless chosen, refresh, and read and write with the key the server shows; always allowed per file, never on open *(shared: the data-source layer JeffCrunch plans)*
- [ ] **later** Linked tables from SharePoint lists and Microsoft online services: not offered (see Left out on purpose)

## Fidelity and testing

JeffBase will be checked against real databases and their recorded answers. Every file JeffBase saves as `.accdb`, and every rule of its expressions, SQL, and sort orders that is only partly documented, is judged against reference files and reference answers. Reference pictures stay on the build machine, and the test databases are kept out of the repository.

- [ ] **first release** Every `.accdb` JeffBase saves opens without a repair message, and Compact & Repair finishes on it without a message (see File formats and Decided)
- [ ] **first release** Databases held out from tuning are scored apart from the ones used for tuning, so the numbers are not flattered by the files the readers were fitted to
- [ ] **1.0** Queries, forms, and reports give the reference answers on the sample databases: each query's rows are compared with the recorded rows, and each form and report is drawn by JeffBase and compared as a picture with its reference picture, on the scale JeffPub's comparison uses
- [ ] **1.0** A collection of real `.accdb` and `.mdb` files is built for the test (the owner has none, and the public collections have almost none): the test databases published with open-source readers (Jackcess is one; licenses checked first) and files gathered from public sources, kept out of the repository

## Safety and limits

Suite-wide rules decided October 11, 2026 (see Decided): nothing in a file runs or reaches out until the person allows it, every file reader is fuzzed and held to limits on memory and time, and a damaged file opens as far as it can without crashing. It also holds the list's speed, accessibility, and language items.

- [ ] **first release** Nothing runs on open: no startup macro, startup form, event handler, or VBA, and no query that reaches another file; an Info panel lists what is waiting and why it was held back
- [ ] **first release** The person chooses per file: keep disabled (the default), allow for this session, or always allow this file, and a changed file asks again; the choice also covers remote links and pass-through queries, and a Trusted folders list in the Trust Center holds places the person has chosen
- [ ] **first release** Every file reader (SQLite files, `.accdb`, `.mdb`, CSV, spreadsheets, XML, HTML, JSON, and `.odb`) is fuzzed and held to memory and time limits, and a bad file shows an error in place, never a crash or a freeze; the hostile-file sweep that runs before each release covers JeffBase's files too
- [ ] **first release** Opening a SQLite file from elsewhere runs with the protections SQLite offers: the loading of extensions off, defensive mode on, schema objects allowed to call only functions marked harmless, limits on sizes and depth, a hook that stops a statement that runs too long, and a check that refuses writes to other files (`ATTACH` and the `load_extension` function are shut off for the file's own queries); and a query copied out of a file runs on a read-only connection until the person confirms
- [ ] **1.0** Readers for `.accdb`, `.mdb`, and other outside formats run in a helper process with no network access and no reach into the person's other files, and a crash there shows an error in the window
- [ ] **first release** The datasheet is virtual: it fetches only the rows on screen, so a table of ten million rows opens as fast as a small one; counts are taken in the background, and sorts and filters use an index when there is one *(shared: the grid control JeffCrunch plans)*
- [ ] **first release** Long work (an import, an export, a Compact & Repair, a big action query, a report on a large table) runs off the main thread with progress and Cancel, and a canceled import leaves no half-built table
- [ ] **1.0** Many users: while the file is open in JeffBase, a second JeffBase on the same computer opens it read-only or waits, and does not corrupt it (see the open questions for sharing across a network)
- [ ] **first release** Screen readers and the keyboard: every datasheet cell is read with its column name and row, every ribbon command and dialog is named, and everything can be done without a mouse; high contrast and large text follow the system *(shared: libs/shell accessibility)*
- [~] **first release** Translations and right-to-left layout in the window, the datasheet, forms, and reports *(shared: libs/shell translations)*
- [ ] **1.0** Dates, numbers, and sort orders follow the person's language and region, with the database's own sort order kept for comparisons, so a file means the same thing on every computer

## File formats

What each format needs, to open, to save, and to come back unchanged. "Round trip" means open, save, and open again without a repair message or a visible change in the tables, the data, or the answers of the queries.

### JeffBase's own SQLite file
- [ ] **first release** Open and save standard SQLite 3 files (`.sqlite`, `.sqlite3`, `.db`, and JeffBase's own extension), with the rollback journal and write-ahead log modes, and the sidecar files they use; a file on a network drive is flagged, since SQLite warns against sharing a file that way
- [ ] **first release** JeffBase's catalog lives in tables with a fixed prefix in the same file: field types and settings, relationship and query layouts, forms, reports, saved imports, and options; an `application_id` marks the file and a `user_version` numbers the layout, so a newer file opens read-only in an older JeffBase with a note
- [ ] **first release** Everything SQLite allows opens and is kept as it was: strict tables, tables without a rowid, generated columns, partial and expression indexes, views, triggers, full-text and other virtual tables, and the file's text encoding and page size; what JeffBase can't edit shows with a note and is never rewritten
- [ ] **first release** The file's schema text is changed only for the object the person edited, never rewritten as a whole, so comments and layout in other tools' definitions survive
- [ ] **1.0** Table design changes that SQLite can't do in place (changing a type, dropping a constraint) run as one all-or-nothing step on a copy of the table, with a backup offered first when the table is large

### Access `.accdb` (Access 2007 to 2021)
- [ ] **first release** Open `.accdb` made by Access 2007, 2010, 2013, 2016, 2019, and 2021: tables and their data of every type (attachments, multi-value fields, Large Number, and Date/Time Extended fields are shown read-only and kept), primary keys, indexes, relationships with their integrity and cascade settings, table and field properties (caption, format, input mask, default, validation, lookup), query definitions, hidden and system objects, linked-table definitions, and the database's own properties
- [ ] **first release** Opening is by reading the file's pages directly, bounded in memory and time, never by running anything in it; a damaged page is reported with its place and the rest still opens
- [ ] **first release** Save As `.accdb`: the tables, their data, primary keys, indexes, and relationships, so Access opens the file with no repair message and the same rows, and Compact & Repair finishes without a message *(ships only when the checks on saved files pass; see Decided)*
- [ ] **1.0** Save As `.accdb` with queries: select queries and the action queries, parameter prompts, and crosstabs, as Access reads them, with the same answers on the sample databases
- [ ] **1.0** Round trip keeps what JeffBase doesn't edit: forms, reports, macros, modules, system tables, and unknown properties, written back as they were from a file that came from Access *(approach: see Decided)*
- [ ] **1.0** Complex columns (attachments, multi-value fields, and version-history memo fields), Large Number, and Date/Time Extended fields edited and written back to `.accdb`
- [ ] **1.0** Open encrypted `.accdb` (the Office encryption Access applies when a password is set), asking for the password; save with a password follows once saving without one is solid
- [ ] **later** Forms and reports saved in `.accdb`: only when the way Access stores them is understood well enough that Access opens them and draws them as designed (see Decided)
- [ ] **1.0** `.accde` and `.mde` (the compiled copies Access makes) opened for their data and queries; their forms and code stay as they are and can't be edited
- [ ] **later** `.accdr` (a copy that opens in runtime mode), `.accdt` (templates), and add-in files (`.accda`): not offered

### Access `.mdb` (Access 97 to 2003)
- [ ] **first release** Open `.mdb` from Access 2000, 2002, and 2003 (the Jet 4 engine): tables and data (including compressed text), indexes, relationships, queries, properties, memo and OLE fields
- [ ] **1.0** Open `.mdb` from Access 95 and 97 (the Jet 3 engine) for data and queries
- [ ] **1.0** Open `.mdb` files protected by a database password (the old, weak scheme), asking for it
- [ ] **later** Save as `.mdb` (2000 and 2002 to 2003 formats): tables, data, indexes, and relationships only
- [ ] **later** Open Access 2 (`.mdb` from the Jet 2 engine) files
- [ ] **later** Files that use user-level security (`.mdw` workgroup files): not opened by going around the permissions; a note says the file needs Access

### Other database files
- [ ] **later** LibreOffice Base `.odb`: open the tables, queries, forms, and reports of a file whose data is stored inside it; a file that only points at a server shows its connection and nothing is run (see the open questions)

### What a round trip must keep
- [ ] **first release** SQLite to JeffBase to SQLite: every table, row, index, view, trigger, and pragma value the other tool wrote, the schema text of the objects not edited, and the file's encoding and page size
- [ ] **first release** `.accdb` or `.mdb` to JeffBase to `.accdb`: all rows and values, keys and indexes, relationships with their options, field and table properties (including ones JeffBase doesn't use), and query definitions, checked by reading the saved file back and comparing every table and every query's answer with the original's

## Shared with other JeffOffice apps

The suite-wide map of shared parts, [shared.md](shared.md), says where each lives, which app builds it first, and who uses it; where this list disagrees with it, the map holds.

What JeffBase builds on, and the pieces it should build so that JeffCrunch, JeffWrite, JeffDeck, and JeffMail can use them.

### Already there
- [~] **first release** libs/shell: the window, ribbon, KeyTips, file pages, Help, translations, AutoRecover, settings, spelling, updates, and statistics
- [~] **first release** libs/base and libs/draw: zip, colors, fonts, barcodes, EMF and WMF, PDF, the dictionaries, shapes, fills, effects, SVG, and picture effects, for report and form decoration, PDF export, and label barcodes
- [~] **first release** libs/office: Open XML packages and DrawingML (the base that JeffCrunch's `.xlsx` and JeffWrite's `.docx` build on, used here to read spreadsheets and write a report as a document), and charts read from stored values and drawn as SVG
- [~] **first release** libs/text, the text engine, for the text in controls, report sections, and notes; libs/page's editor pieces: Find and Replace, proofing dialogs, options, and online pictures
- [~] **first release** JeffPub's print and print preview, PDF export, mail merge, and merge-source reader (CSV and `.xlsx`, values only), and its New page, color and font schemes, Format Painter, and Arrange group

### To build once, for several apps
- [ ] **first release** The grid control (virtual rows and columns, frozen panes, selection, in-cell editing, and fast scrolling over millions of rows), which is JeffBase's datasheet *(needed by: JeffCrunch, JeffBase, JeffWrite, JeffDeck)*
- [ ] **first release** The calculation engine and the number-format language, shared with JeffCrunch's formulas, for expressions in queries, validation rules, defaults, calculated fields, and controls; the parts of Access's expression language that JeffCrunch's formulas don't cover (the `Null` rules, `Like`, `&`, `IIf`, `Nz`, the domain functions, and `Format` codes) are added on top *(needed by: JeffCrunch, JeffWrite, JeffDeck, JeffBase)*
- [ ] **first release** A flow-document model and its page layout, which JeffWrite builds first, used for reports *(needed by: JeffWrite, JeffMail, JeffBase)*
- [ ] **first release** A SQLite layer with the safety settings above, the Access-to-SQLite SQL translator, and the Access functions registered as SQLite functions, so JeffCrunch's "from a database" import and JeffBase run queries the same way *(needed by: JeffBase, JeffCrunch)*
- [ ] **first release** A reader and writer for Jet and ACE database pages (`.mdb` and `.accdb`), built on what Jackcess documents, with a test harness that reads the saved files back and checks them *(needed by: JeffBase; JeffCrunch's database import could read `.accdb` through it)*
- [ ] **1.0** A merge-source library (CSV, tab-delimited, spreadsheets, vCard, and JeffBase tables and queries) with filter, sort, duplicate, and address checks *(needed by: JeffPub, JeffWrite, JeffMail, JeffBase)*
- [ ] **1.0** A data-source layer (text, CSV, JSON, XML, HTML tables, SQLite, ODBC, PostgreSQL, and MySQL) with the editable list of steps *(needed by: JeffCrunch, JeffBase)*
- [ ] **1.0** Chart editing on top of libs/office chart, with the data picked from a JeffBase query *(needed by: JeffWrite, JeffDeck, JeffCrunch, JeffBase)*
- [ ] **1.0** Themes, table styles, and cell styles in one format and gallery, so a theme chosen in JeffDeck colors a JeffBase form *(needed by: all)*
- [ ] **1.0** An accessibility checker and its pane, applied to forms and reports *(needed by: JeffWrite, JeffDeck, JeffCrunch, JeffMail, JeffBase)*
- [ ] **1.0** A property sheet control (a tabbed list of named properties with editors for colors, fonts, expressions, and lists), used for tables, queries, forms, and reports *(needed by: JeffBase; JeffPub's and JeffDeck's panes can adopt it)*
- [ ] **later** An iCalendar and vCard library *(needed by: JeffMail, JeffPub, JeffWrite, JeffBase)*

## Left out on purpose

Not planned, because they only work with an online service or a product JeffOffice doesn't have: SharePoint lists and workflows, Access web apps and Access Services (publishing a database to a server), Microsoft Dataverse and other online services as data sources, Office add-ins and the Store, the online template store, source-control integration through Visual SourceSafe, replication and synchronization of replicas, user-level security and workgroup files, `.adp` and `.ade` projects (removed from Access in 2013), PivotTable and PivotChart views (removed in 2013), the web browser control and ActiveX controls, Paradox and Lotus import, Outlook folder import, the Switchboard Manager, Make ACCDE, encoding and decoding of old databases, and sign-in with a Microsoft account.

## Decided

- Native file (October 11, 2026): JeffBase's own file is **SQLite** by default, an open, widely used single-file database. Save As `.accdb` is offered for people who use Access.
- Writing `.accdb` (October 11, 2026): follows the format knowledge of Jackcess, the open-source Java library, since Microsoft never published the format. Tables, relationships, and queries come first; forms and reports in `.accdb` come later, and only if they can be made reliable.
- The `.accdb` gate (October 11, 2026): every `.accdb` JeffBase saves must open without a repair message, and Compact & Repair must finish on it without a message, before that feature ships.
- Shared parts (October 11, 2026): the datasheet (the table grid) reuses JeffCrunch's spreadsheet grid; formulas and expressions share JeffCrunch's calculation engine where they overlap; reports share JeffWrite's page layout and printing; and mail merge to JeffWrite uses JeffBase tables and queries as a data source.
- Security (October 11, 2026, decided for the whole suite): nothing in a file runs or reaches out until the person allows it (macros, VBA, linked tables to remote servers, and pass-through queries), and every file reader is fuzzed and held to memory and time limits.
- Old formats (October 11, 2026): opening `.mdb` (Access 97 to 2003 files) as well as `.accdb` is in scope.
- How an opened `.accdb` is edited (in place, page by page, or read into a working SQLite copy and written whole at Save) is an engineering choice, made when building starts. What must hold either way: nothing in the file that JeffBase doesn't understand is lost.

## Open questions

1. What extension and name does JeffBase's own file use: a JeffBase extension with a SQLite application id (a double-click opens JeffBase), or a plain `.sqlite` or `.db`?
2. Currency, dates, and Yes/No in the native file: readable decimal numbers, ISO text, and 0 and 1 (what every other tool reads best), or exact scaled whole numbers, day numbers, and Access's -1?
3. In SQL view, show Access's SQL, SQLite's SQL, or both with a switch, and which is saved as the query's text in the native file?
4. Macros: keep them untouched and never run them, or run Access's macro actions after the person allows it (and should JeffBase's own buttons and events use the same actions)?
5. VBA and automation: never run VBA, or give JeffBase an open scripting language later, and which one?
6. Encrypting the native file: SQLite has no built-in encryption, so which scheme (a SQLCipher-compatible one that other tools can open with the password, or another), and is encrypting `.accdb` for Save As needed at 1.0?
7. Several people at once: single user only; several users on one computer; or a shared folder (SQLite warns against network file systems), with a server database (PostgreSQL) as the way to share?
8. Should the "allow this file" choice be remembered per file, per folder (like Trusted Locations), or asked every time?
9. Should ODBC linked tables to servers move up to 1.0, since a small business may keep its data on a server?
10. Which starter databases come first: contacts, inventory, projects and tasks, event planner, or others?
11. Is reading LibreOffice Base `.odb` files worth the work, given that most keep their data in an embedded HSQLDB or Firebird file?
