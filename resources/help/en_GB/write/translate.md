# Translate text
<!-- keywords: translate, translation, translator, language, languages, spanish, french, german, italian, portuguese, dutch, russian, ukrainian, indonesian, offline, on this computer, download, models, replace, insert below, track changes -->

**Review > Language > Translate** opens the **Translate** pane on the right. It translates your text on your own computer. Your words are not sent anywhere, and no internet connection is needed once the translation files for a language are on your computer.

## Translate some text

1. Select the text you want to translate. With nothing selected, the pane uses the paragraph the cursor is in.
2. Click **Translate** in the **Language** group. The pane shows your text under **Original**.
3. Under **From**, choose the language your text is in. JeffWrite starts with the language the text is marked as. Under **To**, choose the language you want. The **Swap** button trades the two.
4. Click **Translate**. The translation appears under **Translation**. Bold, italic, and underlined words stay bold, italic, and underlined, and a colour or a size is kept where the words had the same look before.
5. Click **Replace** to put the translation where your text was, or **Insert Below** to add it in new paragraphs under your text. Either one is a single step, so **Undo** takes it back. The translated words are marked with their language, so spelling checks them in that language.

If you move the cursor or change the text after you click **Translate**, click **Use Selection** to read the text again, then click **Translate** again. The pane will not put a translation over text that is no longer the text it translated.

When **Track Changes** is on, **Replace** and **Insert Below** are tracked changes like your typing, so a reviewer can see what was there before. The translated words then take the look your typing would have. Words that a tracked change has deleted are left alone: they are not translated, and not copied by **Insert Below**.

**Cancel** stops a translation that is taking too long, and so does closing the pane: the window never waits for it. Long texts take longer: a page of text takes a few seconds on most computers. The pane translates at most 1,000,000 letters at once (about 200 pages); with more selected, it says so and asks you to select less. It cuts long paragraphs into pieces of about 1,500 letters, at the end of a sentence where it can, and puts them back together.

## The first time you use a language

A translation needs a set of files, called a model, for each direction: one for English to German, another for German to English. Each model is about 37 MB. The files are not part of JeffWrite. The first time you translate with a language, the pane tells you what it needs and how big it is, for example "Download German (74 MB)?". Nothing is downloaded until you click that button. The pane shows how far the download has come, and **Cancel** stops it and leaves nothing behind.

JeffWrite checks each file against a fingerprint (a SHA-256 checksum) before it uses it. A file that does not match is thrown away, and the language is not installed.

The files are kept in one folder that all the JeffOffice apps share (a folder named JeffOffice in your user data folder, in Translation/models), so a language you download in one app is ready in the others, and it is downloaded only once. Two languages that have no model of their own, such as German and French, go through English, so the pane needs both models.

## When a language has no model

If a pair of languages has no model, the pane says so and offers **Open in Your Browser Instead**. That opens a free translation website, LibreTranslate (or the service whose address you chose in the setting for it), with your text. Only then does your text leave your computer, because it goes to that website, and the pane no longer says that it stays on your computer. Nothing is sent until you click that button.

## About the quality

The models are small, so they run fast on an ordinary computer. They translate sentences well and short labels less well: a single word or a button's name can come out wrong. Read the translation before you use it. Names, numbers, and tabs are kept as they were.

Pictures, tabs, and line breaks inside a paragraph are left alone. The translation fills in the words between them.

## Related

- [Count words, set the language, check accessibility, and look up words](proofing-tools)
