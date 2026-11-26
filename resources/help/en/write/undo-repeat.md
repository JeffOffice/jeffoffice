# Undo, Redo, and Repeat
<!-- keywords: undo, redo, repeat, f4, ctrl+y, history, go back, do again, last action, step -->

**{key:Undo}** takes back your last change, and **{key:Redo}** does it again. Both are on the quick toolbar at the top. Each command you give is one step, and a run of typed words is one step.

## Go back several steps at once

Each of **Undo** and **Redo** has an arrow beside it. Click the arrow to see the steps, the newest first, with up to 100 listed. A step has the name of the command that made it, such as "Bold" or "Typing", and "Edit" when the change came from somewhere else. Move the pointer down the list to mark how many steps you mean, and click to do them. Choosing the third step undoes the first three. The steps you undid stay on the Redo list until you make a new change.

## Repeat the last command

**F4** does your last command again on what is selected now. For example, make a word bold, select another word, and press **F4**. The second word becomes bold, and one **Undo** takes it back. A repeat is always a step of its own, even right after you typed.

Commands that switch something on or off, such as bold, italic, underline, bullets, and numbering, are repeated as the result you got, not as a switch. If you made a word bold, **F4** makes the next selection bold, even if part of it is bold already, and a selection that is bold already is left alone. If you took bold off a word, **F4** takes bold off the next selection.

Repeat works for:

- Formatting: bold, italic, underline, strikethrough, superscript, subscript, font, font size, font color, highlight, text shading, change case, and clear formatting.
- Paragraph settings: alignment, indents, spacing, bullets, numbering, borders, and shading.
- Styles, from the gallery, and Normal and the heading styles.
- Table commands that insert, delete, merge, align, or set borders.
- Page breaks and blank pages, and the special characters you insert with a command.
- Typing: after you type some words, **F4** types the same words again where the caret is. Only the words of one run are kept, up to 1,000 letters, and a run ends when you move the caret. The run is what the text came out as, after corrections such as AutoCorrect, so a repeat puts those words in as they are. Backspace shortens the run by the letters it deletes.

Repeat does not work for opening and saving, changing the view, Find and Replace, copying and pasting, or commands that open a window to ask you something. Those are never the "last command" for Repeat, so **F4** still repeats the formatting you did before them.

The tip on the quick toolbar button names what it would do, for example "Repeat Bold (F4)". Repeat follows the same rules as the command it repeats: with Track Changes on, the repeat is tracked as a change, and in a document open for reading only, or one that is protected, it does nothing and says why in the status bar. Each document has its own last command, so Repeat never carries a command from one document to another.

Repeat works where you are editing. In a box of text or a shape, it types into the box and does the commands the box has; a command the box does not have is not repeated there, and the status bar says so. In a header, a footer, a note, or a table cell, it works in that place.

**Ctrl+Y** is **Redo** while you have steps to redo. When you have none, **Ctrl+Y** repeats like **F4**.
