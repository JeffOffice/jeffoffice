# Find and replace words
<!-- keywords: advanced find, sounds like, soundex, word forms, all forms of a word, ignore punctuation, ignore spaces, special, paragraph mark, tab, find by format, format, bold, font, style, wildcards, wildcard, pattern, regular expression, groups, match count, highlight all matches, replace, replace all, search, look for, change words, match case, whole words, find next, ctrl+f, ctrl+h, editing, pane, hidden text, undo -->

Use Find to jump to words in your document. Use Replace to change them. Both work in one pane on the right of the window.

## Find words

1. Press **Ctrl+F**, or click **Home > Editing > Find**. The **Find and Replace** pane opens, with the cursor in **Find what**.
2. Type the words you want. Then press **Enter**, or click **Find Next**.

JeffWrite starts where your cursor is. It selects the next place where it finds your words, and scrolls the pages so you can see it. That place is called a **match**. Click **Find Next** again to go to the next match. After the last match, it goes back to the start of the document.

As you type, the pane counts the matches, such as "9 matches", and every match gets a yellow band on the pages. The bands show in table cells, headers, footers, and footnotes too. After **Find Next**, the pane says which one you are on, such as "Match 2 of 4". The count and the bands change when you click **Match case** or **Find whole words only**, or when you change the text. They go away when you close the pane.

## Find with wildcards

Check **Use wildcards** to search for a pattern, not just words. A **wildcard** is a mark that stands for other letters. These are the marks:

- **?** stands for any one letter. **c?t** finds "cat" and "cot".
- **\*** stands for any run of letters. **c\*t** finds "cat" and "cart".
- **<** is the start of a word, and **>** is the end of one. **<the>** finds "the" but not "other".
- **[abc]** is any one of those letters, **[a-c]** is any letter from a to c, and **[!abc]** is any letter but those.
- **{2}** repeats the letter before it twice. **{1,3}** repeats it one to three times.
- **@** means one or more of the letter before it.
- **\\** in front of a mark finds the mark itself. **\\?** finds a question mark.
- **^p** is a paragraph mark, **^t** is a tab, **^#** is a digit, **^$** is a letter, and **^w** is white space.

Put part of the pattern in **( )** to make a **group**. In **Replace with**, **\1** is the first group, **\2** is the second, and so on up to **\9**. **^&** is everything that was found. For example, find **(<[A-Z]@>), (<[A-Z]@>)** and replace with **\2 \1** to turn "Smith, John" into "John Smith". **Find whole words only** is dimmed while wildcards are on, because **<** and **>** do that job.

If the pattern has a mistake, the pane says what it is.

If your words are not in the document, the pane says so, and nothing moves.

You can keep working in the document while the pane is open. Click the page to type. Click the **X** at the top of the pane to close it.

## Advanced Find

These choices are in the pane under **Use wildcards**. Each one changes how JeffWrite reads the words in **Find what**.

- **Sounds like** finds words that are said the way your word is said. It gives each word a code from its sounds, and finds the words with the same code. Type **Smith**, and it finds "Smyth" too.
- **Find all word forms** finds every form of the word. Type **run**, and it finds "runs," "running," and "ran." It works on whole words. If you type two words, such as **big dog**, it finds them side by side, in any of their forms.
- **Ignore punctuation characters** skips marks such as hyphens and periods between the letters. Type **email**, and it finds "e-mail" too.
- **Ignore white-space characters** skips spaces and tabs between the letters. Type **e mail**, and it finds "email."

Only one of **Sounds like**, **Find all word forms**, and **Use wildcards** can be on at a time. **Sounds like** and **Find all word forms** always find whole words, so **Find whole words only** and **Match case** are dimmed.

### Special

Click **Special** to put a code in **Find what**: **Paragraph Mark** (^p), **Tab Character** (^t), **Any Digit** (^#), **Any Letter** (^$), **White Space** (^w), or **Caret Character** (^^). The codes work without **Use wildcards**. The other marks in the box, such as **?** and **\***, are then just those characters.

### Format

Click **Format** to find only text that has a certain look: **Bold**, **Italic**, **Underline**, **Strikethrough**, **Highlighted**, a **Font**, or a paragraph **Style**. The pane lists the looks you chose under the buttons. Pick **No Formatting** to clear them.

You can leave **Find what** empty. Then every stretch of text with that look is a match. With words in **Find what**, only the words with that look are found. **Replace All** changes only those words.

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

- It looks at the text of the document, including the text in tables. A bookmark in the middle of some words, which some files have, does not stop them from being found. **Replace** and **Replace All** keep the bookmark.
- **Find Next** looks in the text first. Then it goes on to the headers, then the footers, then the footnotes, and then back to the start of the text. When a match is in one of them, JeffWrite opens it for editing and selects the match. The pane tells you where it is, such as "Found in the header of page 3." **Replace** swaps the match you are on, in the text or in a header, footer, or footnote. **Replace All** changes every match in all of them, and one **Undo** brings them all back. The endnotes are part of the text, so they are searched with it. See [Headers and footers](headers-footers).
- It skips text that is set to **Hidden**, because that text is not shown. See [Change how text looks](font).

## Related

- [Change how text looks](font)
- [Use JeffWrite with the keyboard](keyboard)
