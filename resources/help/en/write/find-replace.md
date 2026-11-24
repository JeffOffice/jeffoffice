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

## Find with regular expressions

Check **Use regular expressions** for patterns that go further than wildcards. A **regular expression** is a short pattern made of letters and marks. It is turned on instead of **Use wildcards**: only one of the two can be on. These are the common marks:

- **.** is any one character, **\d** is a digit, **\w** is a letter, digit, or underscore, and **\s** is a space or tab. **[a-c]** is any letter from a to c, and **[^abc]** is any character but those.
- **^** is the start of a paragraph, **$** is the end of one, and **\b** is the edge of a word. **\bcat\b** finds "cat" but not "category."
- **\*** means none or more of what comes before, **+** means one or more, and **?** means none or one. **{2}** means exactly twice, and **{2,5}** means two to five times. Add **?** after a repeat, as in **.+?**, to find the shortest run that fits.
- **|** means "or": **cat|dog** finds either word.
- **( )** makes a **group**. Use **(?: )** for a group you do not need in **Replace with**.
- **\\** in front of a mark finds the mark itself: **\.** finds a period. **\t** is a tab.

**Match case** works as it does for words. Letters with accents and letters in other alphabets count as letters.

A pattern is searched one paragraph at a time, so **^** and **$** mean the start and end of each paragraph. To search over the end of a paragraph, put **\n** (a paragraph mark) in the pattern: **home\.\nLee** finds "home." at the end of one paragraph and "Lee" at the start of the next.

In **Replace with**, **$1** or **\1** is the first group, **$2** or **\2** is the second, and so on up to nine groups. **$0** or **\0** is everything that was found. **$$** is a dollar sign, **\\** is a backslash, and **\t** is a tab. For example, find **(\w+), (\w+)** and replace with **$2 $1** to turn "Smith, John" into "John Smith."

**Replace All** changes every match in the text, headers, footers, notes, and text boxes as one step, so one **Undo** takes all of them back. With Track Changes on, each change is tracked, as it is for words. Hidden text is left alone.

If the pattern has a mistake, the pane says near which character. A pattern can match no characters at all, such as **^** (the start of a paragraph) or **$** (the end): **Replace with** then puts its words in at that place, so **^** with **> ** adds those marks to the front of every paragraph. A pattern that would take far too long on your text, such as **(a+)+$** on a long line of a's, is stopped by JeffWrite, which says so; nothing is changed. A long search shows a **Searching** box with a **Cancel** button; cancelling leaves your document as it was, and you cannot type in the document until the search is done. Patterns that look ahead or behind without using up what they check, such as **x(?=\d)**, are not supported; JeffWrite says so. In a paragraph longer than 10,000 characters, JeffWrite searches 10,000 characters at a time, so a single match longer than 2,000 characters can be missed there.

## Advanced Find

These choices are in the pane under **Use regular expressions**. Each one changes how JeffWrite reads the words in **Find what**.

- **Sounds like** finds words that are said the way your word is said. It gives each word a code from its sounds, and finds the words with the same code. Type **Smith**, and it finds "Smyth" too.
- **Find all word forms** finds every form of the word. Type **run**, and it finds "runs," "running," and "ran." It works on whole words. If you type two words, such as **big dog**, it finds them side by side, in any of their forms.
- **Ignore punctuation characters** skips marks such as hyphens and periods between the letters. Type **email**, and it finds "e-mail" too.
- **Ignore white-space characters** skips spaces and tabs between the letters. Type **e mail**, and it finds "email."

Only one of **Sounds like**, **Find all word forms**, **Use wildcards**, and **Use regular expressions** can be on at a time. **Sounds like** and **Find all word forms** always find whole words, so **Find whole words only** and **Match case** are dimmed.

### Special

Click **Special** to put a code in **Find what**: **Paragraph Mark** (^p), **Tab Character** (^t), **Any Digit** (^#), **Any Letter** (^$), **White Space** (^w), **Section Break** (^b), **Field** (^d), **Footnote Mark** (^f), **Endnote Mark** (^e), or **Caret Character** (^^). The codes work without **Use wildcards**. A **section break** is the mark that ends a section. A **field** is a piece of text that the program works out, such as a page number or a date, and **^d** finds where each one starts. The note marks are the small numbers in the text that point to a footnote or an endnote. These four are found, but **Replace** and **Replace All** leave them alone, because changing one would lose what it stands for. The other marks in the box, such as **?** and **\***, are then just those characters.

### Format

Click **Format** to find only text that has a certain look: **Bold**, **Italic**, **Underline**, **Strikethrough**, **Highlighted**, a **Font**, or a paragraph **Style**. The pane lists the looks you chose under the buttons. Pick **No Formatting** to clear them.

You can leave **Find what** empty. Then every stretch of text with that look is a match. With words in **Find what**, only the words with that look are found. **Replace All** changes only those words.

## Narrow the search

- **Match case** finds only words with the same capital letters. With it on, "Plan" does not find "plan."
- **Find whole words only** skips longer words that hold yours. With it on, "plan" does not find "planet."
- **Search** sets which way **Find Next** goes from the cursor. **Down** goes to the end of the document, and **Up** goes to the start. Each stops there and says so. **All** goes on round to the other end and is the usual choice. **Replace All** always changes the whole document.

## Replace words

1. Press **Ctrl+H**, or click **Home > Editing > Replace**. The cursor goes to **Replace with**.
2. Type the new words in **Replace with**. To remove the words you found, leave it empty.
3. Type the old words in **Find what**.
4. Click **Replace** to see the first match. Click **Replace** again to change it. JeffWrite then moves to the next match. Each change is one step, so **Undo** (Ctrl+Z) takes back one at a time.
5. Click **Replace All** to change every match at once. The pane tells you how many it changed. **Replace All** also changes the words in text boxes and shapes, and is one step, so one **Undo** puts every word back. When **Track Changes** is on, every replacement is recorded as a tracked change, in the text and in the boxes. A box that is too large to change is counted in the message. **Replace All** does not reach text boxes in headers and footers, shapes grouped together, or boxes linked to each other. **Find Next** does not look in text boxes yet.

The new words look like the first letter of the words they replace. If you replace a bold word, the new word is bold.

For example, to change "color" to "colour" everywhere: type **color** in **Find what**, type **colour** in **Replace with**, and click **Replace All**.

## What the search looks at

- It looks at the text of the document, including the text in tables. A bookmark in the middle of some words, which some files have, does not stop them from being found. **Replace** and **Replace All** keep the bookmark.
- **Find Next** looks in the text first. Then it goes on to the headers, then the footers, then the footnotes, and then back to the start of the text. When a match is in one of them, JeffWrite opens it for editing and selects the match. The pane tells you where it is, such as "Found in the header of page 3." **Replace** swaps the match you are on, in the text or in a header, footer, or footnote. **Replace All** changes every match in all of them, and one **Undo** brings them all back. The endnotes are part of the text, so they are searched with it. See [Headers and footers](headers-footers).
- It skips text that is set to **Hidden**, because that text is not shown. See [Change how text looks](font).

## Related

- [Change how text looks](font)
- [Use JeffWrite with the keyboard](keyboard)
