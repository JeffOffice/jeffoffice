# Review tab
<!-- keywords: review tab, ribbon, proofing, spelling, thesaurus, word count, comments, new comment, delete comment, previous, next, show comments, language, proofing language, set language, do not check spelling, translate, translation, libretranslate, grammar, grammar check, style, blue line, dotted line, check grammar as you type -->

The **Review** tab checks your words, translates them, and handles comments.

- **Proofing** has **Spelling** (F7), **Thesaurus** (Shift+F7), **Word Count**, and **Check Accessibility**. See [Type and format text](text).
- **Language** has **Translate**, **Set Proofing Language**, and **Do Not Check Spelling**. See below.
- **Comments** has **New Comment** (Ctrl+Alt+M), **Delete** with its menu, **Previous**, **Next**, and **Show Comments**. See [Comments](comments).

## Check grammar as you type

JeffDeck also looks for clear grammar and style mistakes in English text, on slides and in the speaker notes. An issue gets a blue dotted line under it, and a misspelled word keeps its red wavy line. Right-click the issue to see what is wrong and to pick a suggestion, or choose **Ignore This Issue** or **Ignore All**. **Spelling** (F7) goes on to the grammar issues in the slides after the spelling. Only English is checked: text in another language, and text marked "do not check", is skipped. To turn it off, click **File > Options > Proofing** and clear **Check grammar as you type**. It is one choice for all the JeffOffice apps.

**Choose what it looks for.** Click **File > Options > Proofing > Grammar settings** to pick the kinds of issues: **Grammar** (verbs that do not agree, the wrong form of a verb, "a" and "an"), **Punctuation** (extra or missing spaces, commas, and hyphens), **Capitalization**, **Mixed-up words** (words that sound alike), **Repetition**, and **Style**. **Style** is off until you turn it on. It suggests, and it is not about mistakes: sentences of more than 40 words, the passive voice, three sentences in a row that begin with the same word, and wordy phrases ("in order to", "due to the fact that"). A change applies at once to the blue lines and to the Spelling pane (F7), in every open window and every JeffOffice app.

## Set the language of some text

Spelling checks each piece of text in its own language. To change the language of some words, select them, or select a whole text box or placeholder, and click **Set Proofing Language** in the **Language** group. Pick the language in the list, then click **OK**. Words in that language are checked with that language's dictionary, and the file keeps the choice when you save it as a presentation or an OpenDocument presentation. Text that you have not marked is checked in American English.

## Do not check some text

Some text is not meant to be checked: a code, a name, or a line in a made-up language. Select it and click **Do Not Check Spelling** in the **Language** group. The red wavy lines go away. Click it again to check the text once more. The notes box holds plain text only, so it has no language of its own.

## Translate text

1. Select the words in a text box, or in the notes. To translate everything in a text box, select the box.
2. Click **Translate** in the **Language** group.
3. Choose the language to translate into.
4. Click **OK**.

Your web browser then opens a translation service called LibreTranslate, with your text in it. Nothing is sent until you click **OK**. You can change the address of the service in the box, for example to one that you run yourself. Only secure web addresses (https) are used, and http addresses for this computer. At most 5,000 letters are sent.

## Check accessibility

**Check Accessibility** opens a small window that lists what may be hard for some people to use: a picture or a shape with no alt text, a slide with no title, a table with no header row, a video with no captions, text that is hard to read against its background, and objects that a screen reader reads in a different order than their places suggest. Text needs a contrast of 4.5 to 1 with its background (3 to 1 for text of 18 points or more, or 14 points or more in bold); where text is over a picture, a tip asks you to check the contrast by eye. A screen reader reads a slide's objects from the back of the stack to the front. Click **Reading Order** to see them in that order, and use **Read Earlier** and **Read Later** (or drag) to change it; each move is one step of Undo. A screen reader uses the first row of a table to name its columns: select a table problem and click **Use the first row as a header** to turn the header row on (one step of Undo). A video with no captions is listed as a tip: videos need captions for people who can't hear them. There is no one-click fix; select the video and use **Insert Captions** on the **Playback** tab, and the tip goes away. Click a problem to go to its slide and select the object. Give a picture alt text in the **Format Picture** pane, or mark it as decoration, then click **Check Again**. You can also start the check from **File > Info**.

## Related

- [Comments](comments)
- [Type and format text](text)
