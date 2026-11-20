# Track changes, and accept or reject them
<!-- keywords: compare, compare documents, two versions, what changed, track changes, tracked changes, revisions, markup, accept, reject, accept all, reject all, display for review, simple markup, all markup, no markup, original, show markup, balloons, reviewing pane, change bar, insertion, deletion, ctrl+shift+e, author, reviewer, user name, edits from someone else -->

**Track Changes** keeps a record of every edit you make, so that you (or someone else) can see what changed and decide whether to keep it. Use it when you edit a document that belongs to someone else, or that someone will review after you.

## Turn Track Changes on and off

Click **Review > Track Changes**, or press **Ctrl+Shift+E**. The status bar at the bottom of the window says whether it is on. A click on those words turns it the other way. The setting is saved in the document, so it is still on when the document comes back from someone else.

While it is on:

- **What you type** is marked as an insertion.
- **What you delete** is not removed. It stays in the text, struck through, so that it can be brought back. Text you typed yourself while tracking was on is the one exception: deleting it just removes it, because nobody else ever saw it.
- **A paragraph break you make** (Enter) is marked as inserted, and one you delete (Backspace at the start of a paragraph) is marked as deleted.
- **A change of look** (bold, a font, a size, a colour, alignment, indents, or a style) is recorded with how the text looked before.

Each change is saved with who made it and when. The name is the one in **File > Options**, under **User name**. Change it there if it is wrong.

Each edit is one step, so **Ctrl+Z** takes it back as it does any other edit.

## What a change looks like

By default, each person's changes have a colour of their own. The first person whose change the document shows gets the first colour, the next person the second, and so on, and a person keeps the same colour for as long as the document is open. Inserted text is underlined in the person's colour, and deleted text is struck through. Text that was moved is green: double struck through where it left, and double underlined where it arrived. Text whose formatting was changed is bold. A bar in the left margin marks each line that has a change.

You can change all of this. Click **Review > Tracking > Tracking Options** to open the **Track Changes Options** box. Under **Markup**, each kind of change has a **Look** and a **Colour**:

- **Insertions**: underline, double underline, strikethrough, bold, italic, colour only, or (none).
- **Deletions**: strikethrough, double strikethrough, hidden, caret, or colour only. With **Hidden**, deleted words are left out of the line. With **Caret**, they are left out too, and a small ^ shows where they were. Moved-away text is left out as well when you choose **Balloons** for deletions, but not otherwise.
- **Moved from** and **Moved to**: the looks and colours for moved text. These are available while **Track moves** is checked. If you uncheck it, moved text shows as an ordinary deletion and an ordinary insertion.
- **Formatting**: bold, colour only, or (none). If you uncheck **Track formatting**, changes of formatting are not marked in the text.
- **Change bars**: left border, right border, outside border (the right side of an odd page and the left side of an even one), or (none), and the bar's colour.

Each **Colour** box offers **By author** (each person's own colour) or one colour for every person. These choices are yours. They are kept on this computer, not in the document, they apply to every document and every open window, and each window changes as soon as you click **OK**. The bars are drawn the same way on the screen, in a printout, and in a PDF whenever the markup is shown.

## Choose what you see: Display for Review

Click **Review > Display for Review** and pick one:

- **All Markup** shows every change in the text, with the bars in the margin.
- **Simple Markup** shows the document as it will look with the changes accepted, and a bar in the margin by each line that has a change.
- **No Markup** shows the document as it will look with every change accepted, and nothing else.
- **Original** shows the document as it was before any of the changes: deleted text is back, and inserted text is gone.

These only change what you see. They never change the document, and they are not saved with it. A document that has changes in it opens with **All Markup**.

**Review > Show Markup** chooses which kinds of change are shown: **Insertions and Deletions**, **Formatting**, and **Comments**. Below them, choose how deleted text is shown. **Show All Revisions Inline** keeps it in the text, struck through. **Show Revisions in Balloons** takes deleted text out of the line and puts it in a balloon in the margin. **Show Only Comments and Formatting in Balloons** keeps insertions and deletions in the text.

## Accept or reject changes

To accept a change is to keep it for good. To reject it is to undo it.

- **Review > Accept** accepts the change the caret is in (or every change in the text you selected), then moves to the next one. **Review > Reject** does the same, undoing the change.
- Click the arrow under **Accept** or **Reject** for more: **Accept All Changes Shown**, **Accept All Changes**, and **Accept All Changes and Stop Tracking**; and **Reject All Changes Shown** and **Reject All Changes**. "Shown" means the changes of the kinds that **Show Markup** has turned on (and, in the **Reviewing** pane, the reviewer you chose).
- **Previous** and **Next** in the **Changes** group go from one change to the next without changing anything. With the caret at the very start of the text, a header, or a note, **Next** goes to a change that begins right there.

Each of these is one step, so **Ctrl+Z** brings back the document exactly as it was.

Accepting a deletion removes the text for good. Rejecting an insertion removes the text too. Accepting an insertion keeps the text and removes the mark that it was new. Rejecting a deletion keeps the text and removes the strike-through. Rejecting a change of look puts back the look the text had. Text that was moved is a pair of changes, one where it left and one where it arrived, joined by the name of the move. When the pair is marked with the same name, as other programs save it, accepting or rejecting either half does the same to the other half, so the text is moved for good or put back. A name that is used more than once, or has only one half, joins nothing: each half is a change of its own.

## The Reviewing pane

Click **Review > Reviewing Pane** to open a list of the changes in the document: what kind, who made it, when, and the words. A click on one shows it in the text. Use **Accept**, **Reject**, **Previous**, and **Next** under the list, or choose one reviewer from the box above it to see only that person's changes.

## Changes in headers, footers, notes, and tables

Changes other people made in a header, a footer, a footnote, or an endnote are shown and listed like the changes in the text, including a header or footer that no page uses (the one for even pages when the document does not use different odd and even pages): **Next** goes past it, and **Accept** and **Reject** do the change that you last stepped to there. **Next**, **Previous**, **Accept**, **Reject**, and the **Reviewing** pane take you into the header, footer, or note that has the change, and the order is the text first, then the footnotes, then the headers and footers. **Accept All** and **Reject All** cover every one of them.

Changes to tables are listed too, in the text and in headers and footers: an inserted or deleted row or cell, cells that were merged, and a change of the look of a table, a row, or a cell. A bar in the margin marks the lines of the row or cell. Accepting a deleted row, or rejecting an inserted one, removes the row and its words. A row in which every cell has the change is that row's change. Rejecting a change of look puts back the look the table, row, or cell had, and rejecting a merge splits the cells again. When taking a change out would break the table, the change is kept instead and the message says how many were kept. That happens for a cell that is joined with others down, which stays and loses its mark, and for a change of the grid that the rows can't follow, which stays as a change that you can accept. Changes inside a table in a footnote are kept as they were written: they are not listed, and Accept and Reject do not act on them.

A move is one line in the list, and one change in the count that Accept All and Reject All give: its other half and the paragraph marks that went with it are done with it. Changes inside a text box are listed one by one, after the changes in the text and before those in the footnotes. **Next** and **Previous** open the box for typing, with the change selected, and **Accept** and **Reject** settle that change, each as one step. Moving the caret inside the open box makes **Next** and **Previous** go on from the caret. A box that is settled one at a time is saved without the copy that older programs read, as it is when you type in a box. The first 200 boxes that hold changes are listed this way. A text box that is part of a group, or whose words continue in other boxes, a box inside another box, a box in a header or a note, and the boxes past the first 200 can't be opened that way: the **Reviewing** pane counts their changes in a line under the list, and only **Accept All** and **Reject All** do them.

A change to a paragraph's numbering is listed as **Numbered** or **Numbering changed**. When a change of a paragraph's look recorded the list and level the paragraph had, rejecting it puts the paragraph back in that list at that level (or takes it out of the list it was not in). A change that only keeps the old numbering as a label (as some programs save it) names no list to go back to: accepting it keeps the numbering, and rejecting it takes the numbering away only when the label is empty, and otherwise leaves the change in place and counts it as kept. Rejecting a numbering that was added gives the paragraph back what its style gives (a heading style that has numbering keeps it), and no numbering where the style has none. If the list a change would put back is no longer in the document, nothing about that paragraph changes, and the change is counted as kept.

A change to a section (page size, orientation, margins, columns, page numbering, or the kind of break) is listed as **Formatted section**, at the paragraph that ends the section, or at the end of the text for the last section. Accepting it keeps the section as it is and drops the record, and rejecting it gives back the properties the record kept, and the pages are laid out again at once. The headers and footers a section names do not change. If the record's old properties can't be read, rejecting leaves the section and the change as they are, and counts it as kept; nothing is replaced by a default page. When Accept All or Reject All also takes a section break out or puts one back, each section's change is settled first, so none is left behind.

A change that can't be settled without damaging the document (a table that would break, a list or a section whose old state can't be put back, a text box too large to write back) is left as written, and the message after **Accept All** or **Reject All** says how many were kept.

## What is not tracked yet

Changes you make yourself to tables (rows, cells, merging), to numbering and lists, to sections and page setup, and to text in headers, footers, and notes are made as usual, without a record. Changes other programs have recorded in numbering and in sections are listed and can be accepted and rejected, as described above.

## Compare two versions

Compare finds the differences between two versions of a document and shows them as tracked changes.

1. Open the older version. It is the **original**.
2. Click **Compare** in the **Compare** group on the **Review** tab.
3. Click **Browse** and choose the newer version. It is the **revised** version.
4. Type the name to sign the changes with. Pick any choices you want, such as ignoring capital letters. **Find text that was moved** and **Find changes of formatting** are on unless you turn them off.
5. Click **OK**.

A new window opens with the original text and the changes that turn it into the revised text. Your two files are not changed. Use **Accept** and **Reject** as you do for any tracked change. If you accept every change, you get the revised text. If you reject every change, you get the original text.

Compare looks at the text of the body of the document. It does not look at tables, headers, footers, or footnotes.

A paragraph of 12 words or more that was cut from one place and put in another is shown as a **move**: one record where the text left, and one where it arrived. Accepting or rejecting one of them takes both. The paragraph has to be the same, or at least 9 words in 10 the same and mostly in the same order, or it is shown as a deletion and an insertion instead. Words that differ inside a moved paragraph are shown as insertions and deletions inside the move.

Text that is the same in both versions but looks different is shown as a **formatting change**. Compare checks bold, italic, underline, size, font, colour, highlight, and a paragraph's style, alignment, indents, and spacing. Rejecting the change gives the old look back.

If the two documents have too many changes, Compare says so. It then shows some paragraphs as replaced whole, and may not look for moves or formatting changes in all of them.
