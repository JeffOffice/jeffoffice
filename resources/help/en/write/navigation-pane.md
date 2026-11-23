# Find your way with the Navigation pane
<!-- keywords: web layout, web view, wrap to window, collapse heading, expand heading, fold, triangle, navigation pane, navigation, headings, outline, pages, thumbnails, results, results by heading, results by page, search, promote, demote, collapse, expand, outline view, outlining, page movement, side to side, read mode, draft view, split, move heading, move section, reorder sections, drag a heading, show level, focus, focus mode, full screen, gridlines -->

The **Navigation pane** is a panel on the left of the window. Open it with **View > Navigation Pane**. It has three tabs.

## Headings

This is an outline of your document. It lists each paragraph that has an **outline level**, like a heading, and puts the smaller headings under the bigger ones.

- Click a heading to go to it.
- Click the small arrow beside a heading to hide or show the headings under it.
- Right-click a heading and choose **Promote** or **Demote** to make it, and the headings under it, one level bigger or smaller. This gives them the style Heading 1, Heading 2, and so on. It is one step to undo.
- Drag a heading up or down the list to move it, with everything under it (up to the next heading of the same or a bigger size), to a new place. A line shows where it will land. Drop it on itself, or inside its own section, to do nothing, or press **Esc** to cancel. The section keeps its level, and it is one step to undo.
- Right-click a heading and choose **Move Section Up** or **Move Section Down** to move it past the section of the same level next to it. In the text, **View > Move Section Up** and **Move Section Down** (**Alt+Shift+Up** and **Alt+Shift+Down**) move the section the caret is in. The caret stays at the same place in the same words. In Outline view, the same keys move the selected heading there.
- The two commands are dimmed, and their tooltips say why, when there is no heading at or above the caret, when Track Changes is on, when the document is protected or open for reading only, and when the caret is in a header, footer, note, text box, or comment instead of the main text.
- A section is not moved, and the status bar says why, when it holds one end of a bookmark, comment, field, or content control whose other end is outside it, when it includes a section break that would change the page layout of other text, when it holds endnotes or a text frame, when the document collects endnotes at the end of each section, or when the move would cut a table. A content control, a block of custom XML, or a tracked insertion that holds whole paragraphs of the section moves with them.
- If you edit the text while the list is still catching up, a move from the list is refused and the list is updated. Choose the heading again.
- Moving a section does not record a tracked change, so it is refused while Track Changes is on. Outline view is different: its Move Up and Move Down record the move as a tracked change when Track Changes is on.
- Right-click and choose **Expand All** or **Collapse All** to open or close the whole list.

## Pages

Small pictures of the pages. Click one to go to that page. It shows up to the first 60 pages.

## Results

Type a word or phrase and press Enter. The tab lists each place it is found, with the words around it. Click a result to select it in the text. At most 500 results are listed.

The box under the search box sets how the places are listed. **As text** lists them in the order of the document. **By heading** puts them under the heading each one is in, with the headings in bold. **By page** puts them under the page each one is on, such as **Page 3**. Click a place, not a heading or a page name, to go to it.

## Outline view

**View > Outline** shows your document as a list of its paragraphs, with the pages out of the way. A heading has the smaller headings and the text under it, and the **Outlining** tab opens while you are in this view.

- **Promote** and **Demote** make the selected headings, and the headings under them, one level bigger or smaller. Alt+Shift+Left and Alt+Shift+Right do the same.
- **Promote to Heading 1** and **Demote to Body Text** jump to the biggest heading or to plain text.
- **Move Up** and **Move Down** (Alt+Shift+Up and Alt+Shift+Down) move a heading, with everything under it, above or below the one next to it.
- The small arrow beside a heading, or **Expand** and **Collapse**, shows or hides what is under it.
- **Show Level** shows only the headings down to the level you pick, such as Level 2, and hides the text. **All Levels** shows everything.
- **Show Text Formatting** draws big headings in bold. **Show First Line Only** cuts each paragraph of text to one line.
- Press **F2** on a row to change its words, and press Enter to keep them. A paragraph that holds a field, a picture, or the like can't be changed here.
- Press **Enter** on a row to start a new paragraph below it at the same level, and type its words. **Tab** demotes the row and **Shift+Tab** promotes it.
- Double-click a paragraph of text to open it on the pages. **Close Outline View**, or **View > Print Layout**, goes back to the pages.

Each change is one step to undo. For more writing, such as new paragraphs, go to Print Layout.

## Collapse a heading on the page

You can fold away what is under a heading, right in the document. Point at a heading. A small triangle shows in the margin, just to the left of it. Click the triangle. The paragraphs under the heading, up to the next heading of the same size or bigger, are hidden. A triangle that points right stays beside a heading that is collapsed. Click it again to open the heading.

This only changes what you see on the screen. Printing, saving to PDF, and saving the file all keep every paragraph. If you move the cursor into a hidden paragraph, such as when Find reaches it, the heading opens. Tables and text boxes under a collapsed heading stay in view.

## Focus

**View > Focus** (F11) hides the ribbon, the ruler, the status bar, and the panes, and fills the screen with your page, so you can write with nothing else in sight. Press **Esc** or F11 to bring them back.

## Page movement

**View > Page Movement** chooses how the pages are laid out. **Vertical** puts them in a column, one below the next. **Side to Side** puts all of them in one row, and the mouse wheel moves along the row.

## Draft view

**View > Draft** shows the pages one after the other, with no page edges, shadows, headers, or footers. A dotted line shows where each page ends, and it says **Page Break** or **Section Break** where one was put in. You can still write and edit. Choose **Print Layout** to go back to the pages as they print.

### The style area

The style area is a strip down the left edge of Draft view and Outline view. It names the style of each paragraph beside the paragraph's first line, and a name that is too long for the strip is cut with an ellipsis. Click a name to select that paragraph. Double-click a name to open the **Modify Style** box for that style. Press Tab to move to the strip, Up and Down to move between names, Enter to open the box, and Esc to go back to the text. In a table, the name beside a row is the style of the paragraph in its first cell.

The strip is hidden until you give it a width. Choose **File > Options > Display** and set **Style area width in Draft and Outline views**; Hidden hides it. You can also drag the strip's edge toward the text. The strip is never printed, never saved in the file, and not shown in Print Layout, Web Layout, or Read Mode.

## Web Layout

**View > Web Layout** shows your words as they would look on a web page. The text is as wide as the window, and it wraps again when you make the window wider or narrower. There are no page edges, no page breaks, no headers, and no footers. You can still write and edit. Choose **Print Layout** to go back to the pages as they print.

Web Layout is only a way to look at the document. Printing, saving to PDF, and saving the file all use the pages as the document has them. Page numbers in fields, such as in a table of contents, are worked out from those pages too.

## Split

**View > Split** adds a second pane under the pages. It shows the same document, and each pane scrolls and zooms on its own. Use it to look at one place, such as a table, while you write in another. You write in the first pane; the second is for looking, and it shows each change at once. Choose **Split** again to close it.

## Read Mode

**View > Read Mode** is for reading. It hides the ribbon, the ruler, and the panes, puts the pages side by side in a row with each page fitted to the window, and keeps the text from being changed. Roll the mouse wheel to move along the row. Press **Esc** or **F11**, or choose **Read Mode** again, to go back.

## Gridlines

**View > Gridlines** shows the faint edges of table cells that have no lines. They show only on the screen.
