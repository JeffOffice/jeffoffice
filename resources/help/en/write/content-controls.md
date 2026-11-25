# Make forms with content controls
<!-- keywords: content control, content controls, form, forms, fill in, rich text, picture control, combo box, repeating section, add item, plain text, drop-down list, dropdown, date picker, calendar, check box, checkbox, title, tag, placeholder, lock, cannot be deleted, properties -->

A **content control** is a box in your text that a person fills in: a line of text, formatted paragraphs, a picture, a choice from a list, a date from a calendar, a check box, or a section that can be repeated. Use them to make a form that others can complete without changing the rest of the document.

## Put a control in

1. Put the caret where the control should go.
2. Choose **Insert > Content Control**, and then pick the kind:
   - **Rich Text:** a box for formatted words. With paragraphs selected, or in a paragraph with nothing in it, the control holds whole paragraphs, including lists and tables, and Enter makes a new paragraph inside it. Made in a paragraph that has other words, it holds words of that paragraph, and Enter there splits it into two controls.
   - **Plain Text:** a box for words. It shows the placeholder "Enter text" until someone types. With words selected, the control is made around them.
   - **Picture:** a place for one picture. It shows "Add a picture" until one is put in.
   - **Combo Box:** a list of choices that also takes what you type. It shows "Choose or type an item" until one is picked or typed.
   - **Drop-Down List:** a list of choices. It shows "Choose an item" until one is picked.
   - **Date Picker:** a date from a calendar. It shows "Pick a date" until one is picked.
   - **Check Box:** a small box that is checked or cleared.
   - **Repeating Section:** the paragraph the caret is in (or the paragraphs selected) become the first item of a section that can have more items.

A control gets a light shade so you can see where it begins and ends. When the caret is in one, it has an outline, and its title is on a tab above it. Putting in a control is one step to undo.

## Fill in a control

- **Rich Text:** click the placeholder and type. Anything you can do in the text, such as bold, lists, and tables, you can do inside a control that holds paragraphs.
- **Picture:** click the control, or press Enter with the caret in it, and choose a picture file; or drag a picture file onto it. A picture put in place of another takes the size the first had, kept in proportion. A picture control starts two inches square. **Clear Picture** (in the Content Control menu) brings the placeholder back.
- **Combo Box:** click the placeholder and type, or double-click the control (or press Alt+Down with the caret in it) and click an item.
- **Repeating Section:** click the **+** button at the end of the section, or choose **Add Item**, to add an item after the last one. Each new item is a copy of the last with every control in it empty again. **Remove Item** takes out the item the caret is in; a section keeps at least one item, and holds at most 1,000. With Track Changes on, an added item is a tracked insertion and a removed item is a tracked deletion, so a reviewer can see them, and Reject All puts everything back. With Track Changes on, an item can't be added to or removed from a section that ends the document: turn Track Changes off, or put a paragraph after the section.
- **Plain Text:** click the placeholder and type. What you type takes its place. Press Shift+Enter for a line break; Enter does not make a new paragraph inside a control.
- **Drop-Down List:** click the control (or press Enter or Alt+Down with the caret in it) and click an item. The item you picked is checked the next time.
- **Date Picker:** click the control, and then click a day in the calendar. **Today** puts in today's date. **Clear** brings the placeholder back.
- **Check Box:** click it, or press the Space bar with the caret in it, to check or clear it.

A drop-down list, a date picker, a check box, and a picture control hold only what is chosen for them: you cannot type in them.

## Change a control's properties

Put the caret in the control and choose **Insert > Content Control > Properties**. You can set:

- **Title** and **Tag:** the title shows on the tab above the control. The tag is a short name that other programs can use to find the control.
- **Items in the list** (drop-down list and combo box): Add, rename (double-click an item), Remove, Move Up, and Move Down. A list holds up to 1,000 items.
- **Date format** (date picker): pick a format or type your own, such as MMMM d, yyyy. The box shows what today's date looks like in it.
- **The control can't be deleted:** the control stays, though what is inside it can be changed.
- **The contents can't be edited:** the control can be deleted, but what is inside it can't be changed. That includes typing, pasting, dragging, Find and Replace, a spelling change, a new look for the words, Copy Formatting, table commands, and accepting or rejecting a tracked change inside it. A change inside it that was tracked before the lock is left to resolve later.
- **Items can't be added or removed** (repeating section): Add Item and Remove Item do nothing, and say why. Put the caret in any item to see this box for the whole section.

Choose **OK** to apply the changes as one step to undo.

## Good to know

- A control that holds whole paragraphs keeps its text apart from the paragraphs round it. If it can't be deleted, or its contents can't be edited, Backspace and Delete at its edge do nothing, because they would move words into it or out of it. If it has no lock, joining a paragraph across its edge puts the joined paragraph on the side the caret is on, and the control keeps its other paragraphs; a control with only one paragraph is never left with none, so the joined paragraph stays in it.
- A lock on a control round whole table rows or cells is kept in the file, but JeffWrite does not hold it. The same goes for a control that begins with a table.
- A document protected for filling in forms is read only all through, so no control can be filled in there, and the Tab key does not move between controls.
- A document holds up to 10,000 controls, and controls can be inside one another up to 8 deep. A repeating section and its items count as controls.
- Controls in files from other programs are shown and kept as they were. A kind that JeffWrite cannot make (such as a building block gallery, an equation, or a group) is kept in place; you can change its title, tag, and locks, but not its contents.
- Controls are saved in `.docx` files. In other file types the words stay and the control does not. Check Compatibility says so, and names the kinds with nothing to match: repeating sections, picture controls, and combo boxes.
