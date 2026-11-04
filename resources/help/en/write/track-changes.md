# Track changes, and accept or reject them
<!-- keywords: compare, compare documents, two versions, what changed, track changes, tracked changes, revisions, markup, accept, reject, accept all, reject all, display for review, simple markup, all markup, no markup, original, show markup, balloons, reviewing pane, change bar, insertion, deletion, ctrl+shift+e, author, reviewer, user name, edits from someone else -->

**Track Changes** keeps a record of every edit you make, so that you (or someone else) can see what changed and decide whether to keep it. Use it when you edit a document that belongs to someone else, or that someone will review after you.

## Turn Track Changes on and off

Click **Review > Track Changes**, or press **Ctrl+Shift+E**. The status bar at the bottom of the window says whether it is on. A click on those words turns it the other way. The setting is saved in the document, so it is still on when the document comes back from someone else.

While it is on:

- **What you type** is marked as an insertion.
- **What you delete** is not removed. It stays in the text, struck through, so that it can be brought back. Text you typed yourself while tracking was on is the one exception: deleting it just removes it, because nobody else ever saw it.
- **A paragraph break you make** (Enter) is marked as inserted, and one you delete (Backspace at the start of a paragraph) is marked as deleted.
- **A change of look** (bold, a font, a size, a color, alignment, indents, or a style) is recorded with how the text looked before.

Each change is saved with who made it and when. The name is the one in **File > Options**, under **User name**. Change it there if it is wrong.

Each edit is one step, so **Ctrl+Z** takes it back as it does any other edit.

## What a change looks like

Each person's changes have a color of their own. Inserted text is underlined in the person's color, and deleted text is struck through. A bar in the left margin marks each line that has a change.

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
- **Previous** and **Next** in the **Changes** group go from one change to the next without changing anything.

Each of these is one step, so **Ctrl+Z** brings back the document exactly as it was.

Accepting a deletion removes the text for good. Rejecting an insertion removes the text too. Accepting an insertion keeps the text and removes the mark that it was new. Rejecting a deletion keeps the text and removes the strike-through. Rejecting a change of look puts back the look the text had.

## The Reviewing pane

Click **Review > Reviewing Pane** to open a list of the changes in the document: what kind, who made it, when, and the words. A click on one shows it in the text. Use **Accept**, **Reject**, **Previous**, and **Next** under the list, or choose one reviewer from the box above it to see only that person's changes.

## What is not tracked yet

Changes to tables (rows, cells, merging), to numbering and lists, to sections and page setup, and to text in headers, footers, and notes are made as usual, without a record. Changes other programs have recorded in those places are kept and saved as they were.

## Compare two versions

Compare finds the differences between two versions of a document and shows them as tracked changes.

1. Open the older version. It is the **original**.
2. Click **Compare** in the **Compare** group on the **Review** tab.
3. Click **Browse** and choose the newer version. It is the **revised** version.
4. Type the name to sign the changes with. Pick any choices you want, such as ignoring capital letters.
5. Click **OK**.

A new window opens with the original text and the changes that turn it into the revised text. Your two files are not changed. Use **Accept** and **Reject** as you do for any tracked change. If you accept every change, you get the revised text. If you reject every change, you get the original text.

Compare looks at the text of the body of the document. It does not look at tables, headers, footers, footnotes, or how the text looks. A change of look alone is not found.
