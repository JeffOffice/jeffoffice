# Find and replace words
<!-- keywords: find, replace, replace all, search, look for, change words, match case, whole words, find next, ctrl+f, ctrl+h, editing, pane, hidden text, undo -->

Use Find to jump to words in your document. Use Replace to change them. Both work in one pane on the right of the window.

## Find words

1. Press **Ctrl+F**, or click **Home > Editing > Find**. The **Find and Replace** pane opens, with the cursor in **Find what**.
2. Type the words you want. Then press **Enter**, or click **Find Next**.

JeffWrite starts where your cursor is. It selects the next place where it finds your words, and scrolls the pages so you can see it. That place is called a **match**. Click **Find Next** again to go to the next match. After the last match, it goes back to the start of the document.

If your words are not in the document, the pane says so, and nothing moves.

You can keep working in the document while the pane is open. Click the page to type. Click the **X** at the top of the pane to close it.

## Narrow the search

- **Match case** finds only words with the same capital letters. With it on, "Plan" does not find "plan."
- **Find whole words only** skips longer words that hold yours. With it on, "plan" does not find "planet."

## Replace words

1. Press **Ctrl+H**, or click **Home > Editing > Replace**. The cursor goes to **Replace with**.
2. Type the new words in **Replace with**. To remove the words you found, leave it empty.
3. Type the old words in **Find what**.
4. Click **Replace** to see the first match. Click **Replace** again to change it. JeffWrite then moves to the next match. Each change is one step, so **Undo** (Ctrl+Z) takes back one at a time.
5. Click **Replace All** to change every match at once. The pane tells you how many it changed. **Replace All** is one step, so one **Undo** puts every word back.

The new words look like the first letter of the words they replace. If you replace a bold word, the new word is bold.

For example, to change "color" to "colour" everywhere: type **color** in **Find what**, type **colour** in **Replace with**, and click **Replace All**.

## What the search looks at

- It looks at the text of the document, including the text in tables.
- It does not look in headers, footers, footnotes, or endnotes yet. Words there are not found, and **Replace All** does not change them.
- It skips text that is set to **Hidden**, because that text is not shown.

## Related

- [Use JeffWrite with the keyboard](keyboard)
