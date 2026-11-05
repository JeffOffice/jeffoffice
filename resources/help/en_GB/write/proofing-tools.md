# Count words, check, and look up words
<!-- keywords: merged cells, link text, count words, accessibility, check accessibility, alt text, header row, screen reader, contrast, thesaurus, synonym, synonyms, translate, translation, libretranslate, word count, count words, characters, paragraphs, pages, selection, footnotes, endnotes, language, proofing language, set language, do not check spelling, spelling language, status bar -->

These tools are on the **Review** tab.

## Count the words

1. Click **Count Words** in the **Proofing** group.
2. The box shows the pages, words, characters without spaces, characters with spaces, and paragraphs.

If you select some text first, the box counts only that text. The status bar at the bottom of the window also shows it, as "12 of 340 words".

Check **Include footnotes and endnotes** to count them too. Hidden text is never counted. Neither is text that a tracked change deleted.

## Set the language of text

JeffWrite checks spelling with the dictionary of the language the text is in. Mark a quote in French as French, and its words are checked in French.

1. Select the text.
2. In the **Language** group, click **Set Proofing Language**. You can also click the language name in the status bar.
3. Pick a language and click **OK**.

The language goes in the file, so other programs know it too. The status bar shows the language of the text at the cursor.

## Do not check some text

Some text is not meant to be checked: a code, a name, or a line in a made-up language. Select it and click **Do Not Check Spelling** in the **Language** group. The red wavy lines go away. Click it again to check the text once more.

## Look up a word in the thesaurus

A thesaurus lists words that mean nearly the same thing.

1. Click a word, or select it.
2. Click **Thesaurus** in the **Proofing** group, or press **Shift+F7**.
3. Pick a word in the list and click **Replace**. To look up one of the words instead, click **Look Up**. You can also type another word in the box and press **Enter**.

The list comes from a file on your computer. No internet connection is used.

## Translate text

1. Select the text. With nothing selected, the whole document is used.
2. Click **Translate** in the **Language** group.
3. Choose the language to translate into.
4. Click **OK**.

Your web browser then opens a translation service called LibreTranslate, with your text in it. Nothing is sent until you click **OK**. You can change the address of the service in the box, for example to one that you run yourself. Only web addresses (http or https) are used. At most 5,000 letters are sent.

## Check accessibility

Some people read with a screen reader, which speaks the document aloud. Some have trouble seeing pale text. The accessibility check finds what makes a document hard for them.

1. Click **Check Accessibility** in the **Proofing** group.
2. A list opens at the right. Each line says what is wrong. Click a line to go to the place.
3. Fix the problem. Click **Check Again**.

It looks for these:

- **A picture with no alt text.** Alt text is a short description of a picture. To add it, right-click the picture and choose its alt text.
- **A table with no header row.** The header row holds the names of the columns.
- **A blank table cell.**
- **A heading that skips a level**, such as Heading 3 right after Heading 1.
- **Text that is hard to read**, because its colour is too close to the colour behind it.
- **A picture that floats on the page** instead of sitting in line with the text.
- **A table with merged cells.** A merged cell is one cell that covers two or more columns or rows. A screen reader can lose its place in such a table. This is said once for each table.
- **A link whose words say nothing about where it goes**, such as "click here" or "read more", or a web address shown as it is. A person who listens to the list of links on a page needs words that name the page.

You can also start the check from **File > Info**: click **Check Accessibility**.
