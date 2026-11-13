# Translate text
<!-- keywords: translate, translation, translator, language, languages, spanish, french, german, italian, portuguese, dutch, russian, ukrainian, indonesian, offline, on this computer, download, models, replace, insert below -->

**Review > Language > Translate** opens the **Translate** pane. It translates your text on your own computer. Your words are not sent anywhere, and no internet connection is needed once the translation files for a language are on your computer.

## Translate some text

1. Select the text you want to translate. With nothing selected, the pane uses the paragraph the cursor is in. In a text box that is selected but not being edited, it uses all of the box's text.
2. Click **Review > Language > Translate**. The pane shows your text under **Original**.
3. Under **From**, choose the language your text is in. Under **To**, choose the language you want. The **Swap** button trades the two.
4. Click **Translate**. The translation appears under **Translation**. Bold, italic, and underlined words stay bold, italic, and underlined.
5. Click **Replace** to put the translation where your text was, or **Insert Below** to add it in new paragraphs under your text. Either one is a single step, so **Undo** takes it back.

If you move the cursor or change the text after you click **Translate**, click **Use Selection** to read the text again, then click **Translate** again. The pane will not put a translation over text that is no longer the text it translated.

**Cancel** stops a translation that is taking too long. Long texts take longer: a page of text takes a few seconds on most computers.

## The first time you use a language

A translation needs a set of files, called a model, for each direction: one for English to German, another for German to English. Each model is about 37 MB. The files are not part of JeffPub. The first time you translate with a language, the pane tells you what it needs and how big it is, for example "Download German (74 MB)?". Nothing is downloaded until you click that button. The pane shows how far the download has come, and **Cancel** stops it and leaves nothing behind.

JeffPub checks each file against a fingerprint (a SHA-256 checksum) before it uses it. A file that does not match is thrown away, and the language is not installed.

The files are kept in JeffPub's data folder, and you can use them in all the JeffOffice apps that have the Translate pane. Two languages that have no model of their own, such as German and French, go through English, so the pane needs both models.

## When a language has no model

If a pair of languages has no model, the pane says so and offers **Open in Your Browser Instead**. That opens a free translation website, LibreTranslate, with your text. Only then does your text leave your computer, because it goes to that website. Nothing is sent until you click that button.

## About the quality

The models are small, so they run fast on an ordinary computer. They translate sentences well and short labels less well: a single word or a button's name can come out wrong. Read the translation before you use it. Names, numbers, and tabs are kept as they were.

Pictures, tabs, and line breaks inside a paragraph are left alone. The translation fills in the words between them.

## Related

- [Spelling, thesaurus, research, and hyphenation](proofing)
