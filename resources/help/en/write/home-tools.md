# Change case, sort, and paste in other ways
<!-- keywords: paste options button, smart cut and paste, spaces when pasting, horizontal line, rule, reveal formatting, style inspector, shift+f1, tabs, tab stops, bar tab, bar stop, default tab stops, tab distance, leader, keep with next, keep lines together, page break before, widow, orphan, outline level, line and page breaks, change case, uppercase, lowercase, sentence case, capitalize, toggle case, shift+f3, sort, sort paragraphs, alphabetical, order, paste, paste special, continue list, new list, merge list, nest table, spreadsheet cells, keep text only, merge formatting, rtf, rich text, formatted text, drop, drag text, unformatted text, picture, clipboard -->

These tools are on the **Home** tab. They work on the text you select.

## Change the case of letters

The **case** of a letter is whether it is a capital letter (A) or a small letter (a).

1. Select the text. With nothing selected, JeffWrite changes the word the cursor is in.
2. In the **Font** group, click **Change Case**.
3. Pick one:
   - **Sentence case** makes the first letter of each sentence a capital. The rest are small.
   - **lowercase** makes every letter small.
   - **UPPERCASE** makes every letter a capital.
   - **Capitalize Words** makes the first letter of each word a capital.
   - **tOGGLE cASE** turns every capital into a small letter, and every small letter into a capital.

Press **Shift+F3** to go through three of them in turn: **Capitalize Words**, then **UPPERCASE**, then **lowercase**.

The look of the letters stays the same. Bold stays bold. **Ctrl+Z** takes the change back in one step. If Track Changes is on, the change shows as a deletion and an insertion.

## Sort paragraphs

Sorting puts paragraphs in order, such as A to Z. Use it for a list of names, or for lines of a table made with tabs or commas.

1. Select two or more paragraphs.
2. In the **Paragraph** group, click **Sort**.
3. Choose what to sort by. Each choice has a **field**, a **type**, and an **order**.
   - The **field** is a part of the paragraph. Field 1 is the first part. Say the parts are split by commas or tabs, in the box below the keys. If you leave it as **Paragraphs**, the whole paragraph is the one field.
   - The **type** is **Text**, **Number**, or **Date**. A number sorts by its value, so 9 comes before 10.
   - The **order** is **Ascending** (A to Z, small to large, old to new) or **Descending**.
4. Use **Then by** to break ties.
5. Check **The first paragraph is a header** to leave it where it is.
6. Click **OK**.

Each place keeps its own paragraph look. Sorting cannot be used while Track Changes is on, because a sort is not a change to show. Turn Track Changes off, sort, and turn it on again.

## Paste in other ways

Click the small arrow under **Paste** for more choices:

- When the text comes from another program that puts rich text (RTF) on the clipboard, **Ctrl+V** keeps its look: fonts, sizes, colors, bold, lists, tables, links, and pictures. If the program gave no rich text, JeffWrite uses the web page form of it (HTML), and then plain words. Rich text over 16 MB is not read, and the paste uses the next form. Dropping formatted text from another program on the page does the same.
- **Merge Formatting** pastes the text in the look of the place it goes to. Only bold, italic, underline, and strikethrough are kept.
- **Keep Text Only** pastes the words with no look at all.
- **Picture** pastes a picture from the clipboard. It is dimmed when the clipboard holds no picture.
- **Paste Special** (**Ctrl+Alt+V**) lists every way the clipboard can be pasted now, and lets you pick one, such as **Formatted text (RTF)**, **Unformatted text**, or **HTML**.

A choice is dimmed when the clipboard has nothing it can use.

Right after you paste, a small **Paste Options** button shows up next to your cursor. Click it to pick **Keep Source Formatting**, **Merge Formatting**, or **Keep Text Only**. JeffWrite takes the paste back and does it again that way. The change is one step: a single **Undo** takes the whole paste back, however many times you changed your mind. From the keyboard, tap the **Ctrl** key (the **Control** key on a Mac) after a paste to open the button's menu, then use the arrow keys and **Enter** to choose. The button goes away as soon as you type, move the cursor, or press **Esc**, and it is not offered in a document you cannot change.

When the paste brought list paragraphs next to a list, the button also offers **Continue List** (the numbers go on from the list beside them), **New List** (the numbering starts over at 1), and **Merge List** (the items join the list beside them and take its look). Plain **Ctrl+V** puts the items in as they came, and a choice changes that. The same choices are commands in the Paste menu on the Home tab and in **Paste Special**, so the keyboard reaches them too.

When the clipboard holds spreadsheet cells (copied from JeffCrunch or another program as a table or as lines with tabs between the cells), plain **Ctrl+V** pastes them as the words they are. To paste them as cells, choose **Table** from the button, the Paste menu, or **Paste Special**. With the cursor inside a table, the button offers **Nest Table** (a table in the cell), **Merge Table** (the cells join the table, which gets the columns they need), **Insert as New Rows**, **Insert as New Columns** (the cells go in new columns after the cursor's cell, from its row down; a table can have at most 256 columns and 50,000 cells, and it is refused, with a message, where a joined cell runs across the place for the columns), and **Overwrite Cells** (the cells replace the table's own, from the cursor's cell on). A pasted table has at most 500 rows and 63 columns, and a cell holds at most 4,000 characters; what is past that is left out, and a message says so. A table nested in a table can go at most 8 tables deep, and a nested table whose columns would be too narrow for the cell is refused. Each paste is one step: Undo takes all of it back, and Redo puts it back.

**Smart cut and paste** keeps the spaces right when you move words. If you cut whole words from between two spaces, JeffWrite takes one of the spaces too, so you are not left with a double space. If you paste a word right up against another word, it adds a space between them. It leaves the middle of a word alone. To turn it off, click **File > Options > Advanced** and clear **Smart cut and paste**.

## Line and page breaks, and tabs

Click the small button in the corner of the **Paragraph** group to open the **Paragraph** box. Under the spacing choices are these:

- **Widow and orphan control** keeps one lone line of a paragraph from sitting by itself at the top or the bottom of a page.
- **Keep with next** keeps the paragraph on the same page as the one after it. Use it for a heading.
- **Keep lines together** keeps all the lines of the paragraph on one page.
- **Suppress line numbers** leaves this paragraph out when the page numbers its lines (see Layout > Line Numbers).
- **Page break before** starts the paragraph on a new page.
- **Don't hyphenate** stops the paragraph's words from being split with a hyphen at the end of a line.
- **Don't add space between paragraphs of the same style** removes the space between two paragraphs that have the same style.
- **Outline level** says where the paragraph is in the outline of the document: **Body text**, or a level from 1 to 9.

Click **Tabs** in the box to set tab stops. A tab stop is a place the text jumps to when you press **Tab**. Type its position, choose how the text lines up there (left, center, right, or decimal), and choose a leader. A leader is the row of dots, dashes, or a line that fills the space before the stop. Click **Set** to add it. To remove one, pick it in the list and click **Clear**. **Clear All** removes them all.

Choose **Bar** to set a bar. A bar does not stop the text. It draws a thin straight line, up and down, at that place on every line of the paragraph. It is a way to put a rule between columns of text. A bar has no leader. The ruler does not show bars. A paragraph keeps up to 64 of them. Not yet: in a paragraph that reads right to left, bars are still measured from the left edge. Set and clear them in the **Tabs** box.

**Default tab stops** is the distance between the stops that a paragraph gets when it has none of its own, and past the last stop it does have. The usual distance is half an inch. This one is for the whole document, not just the paragraph you are in. The new distance applies to every paragraph as soon as you click **OK**.

## Borders and shading for paragraphs

Select one or more paragraphs. In the **Paragraph** group, the **Borders** button has the quick choices: **Bottom Border**, **Top Border**, **Left Border**, **Right Border**, **All Borders**, and **No Border**. A quick choice draws a single line, half a point thick, a point away from the words. A choice for one side leaves the other sides as they were.

**Borders and Shading** at the bottom of that list opens a box. Tick the sides that get a line, or click **Box** or **None**. Then pick the **Style** (single, double, thick, dotted, or dashed), the **Width**, the **Color**, and the **Distance from text**. **Shading** fills the paragraphs with a color. The **Shading** button beside **Borders** sets only the fill, from the same colors as **Font color**.

The lines and the fill are one step that **Undo** takes back, and they are kept when you save a .docx or .odt file. If a paragraph came from a file with its own border or fill, your choice takes its place.

## Draw a horizontal line

Click **Horizontal Line** in the **Paragraph** group. JeffWrite adds a paragraph of its own with a thin line under it, after the paragraph the cursor is in. The cursor moves to a plain paragraph below the line. To take the line away, select its paragraph and delete it.

## See what is set where the cursor is

Press **Shift+F1**, or use **Reveal Formatting**. A box lists the styles, the font, the size, the look of the letters, the color, and the paragraph's alignment, indents, and spacing at the cursor. It only shows them. To change them, use the **Font** and **Paragraph** boxes.
