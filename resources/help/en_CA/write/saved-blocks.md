# Save a block and put it in again
<!-- keywords: saved blocks, boilerplate, signature, letterhead, text shortcut, text shortcuts, reuse text, save selection, insert again, F3, library -->

A **block** is a piece of a document that you saved so that you can put it in again: a signature, a paragraph you use in every letter, a letterhead, or a picture with a caption. A block keeps its text with its fonts, colours, styles, bullets and numbers, pictures, links, fields, and tables (rows, columns, widths, borders, shading, merged cells, a header row that repeats, and the table's style). It does not keep comments, footnotes, endnotes, or section breaks (see Good to know).

## Save a block

1. Select what you want to save.
2. Choose **Insert > Saved Blocks > Save Selection as a Block**.
3. Type a **Name**. You can also add a **Category** and a **Description**, which help you find the block later.
4. In **Gallery**, choose **Saved Blocks** (a block you pick from a gallery) or **Text Shortcuts** (a block you bring in by typing its name).
5. In **Put it in**, choose how the block goes into a document: **As it is, at the cursor**, **In a paragraph of its own**, or **On a page of its own**.
6. Choose **Save**.

If a block with that name is already saved, JeffWrite asks whether to replace it.

## Put a block in

1. Put the cursor where the block should go. If you select words first, the block takes their place.
2. Choose **Insert > Saved Blocks**, and then click the block. Type in the search box to find a block by its name, category, or description.

Putting a block in is one step to undo. A style the block uses that your document does not have is added; a style your document already has under the same name is never changed. A field in the block, such as the page number, is updated for the document it is in. The first and last paragraphs of a block take the look of the paragraphs they join, as when you paste.

## Your own cover pages, tables, and text boxes

Your own cover page, table, or text box can be kept in a gallery of its own, under the built-in ones.

- **Cover page.** Select the cover page and its page break (or put the cursor anywhere and select nothing, when the document has a cover page already), and choose **Insert > Cover Page > Your Cover Pages > Save Selection to Cover Page Gallery**.
- **Table.** Select the whole table, and choose **Insert > Table > Ready-Made Tables > Your Tables > Save Selection to Table Gallery**.
- **Text box.** Click the text box so it is selected, and choose **Insert > Text Box > Your Text Boxes > Save Selection to Text Box Gallery**.

Give it a name and click **Save**. It is then listed under **Your Cover Pages**, **Your Tables**, or **Your Text Boxes**. Click it to put it in. A cover page goes at the start of the document, on a page of its own, and takes the place of the cover page the document has. A table or a text box goes in at the cursor. **Manage Saved Blocks** lists these galleries too, so you can rename, change, delete, export, and import them.

## Text Shortcuts

A Text Shortcut is a block with a short name. Type the name, then press **F3** (or choose **Insert > Saved Blocks > Expand Text Shortcut**), and the name is replaced by the block. The name must start a word: "sig" works after a space, but not inside "design".

## Manage saved blocks

Choose **Insert > Saved Blocks > Manage Saved Blocks**. You can see each block, change its name, category, description, or way of going in with **Edit Properties**, and **Delete** it. **Undo Delete** puts back what you deleted until you close the box. **Export** saves your blocks in a file, and **Import** adds the blocks of such a file; a block whose name you already have is left as it is.

## Good to know

- Your saved blocks are kept in one file on this computer, for your own account only: they are not in your documents and are not sent anywhere. The file is `library.jblocks`, in the folder `Saved Blocks` inside your data folder: `AppData\Local\JeffOffice` on Windows, `Library/Application Support/JeffOffice` on a Mac, and `.local/share/JeffOffice` on Linux. Every JeffOffice program on your computer shares it, and two windows can add blocks at the same time without losing each other's.
- A block can be up to 8 megabytes, and the library holds up to 5,000 blocks.
- A table is saved as a table when the selection holds all of it. If the selection holds only part of a table, that part is saved as its words, one paragraph for each cell, and JeffWrite tells you before it saves.
- A bookmark in a block gets a name of its own when you put the block in: if your document already has a bookmark of that name, the new one is named with "_2", "_3", and so on after it.
- Comments are left out. Footnotes and endnotes are left out. Section breaks are not kept. Shapes and text boxes are kept and come in again with the block.
- Tracked changes are accepted when the block is saved: the words that were inserted stay, and the words that were deleted are not in the block, so a block never holds another person's tracked words.
- A document never adds blocks to your library. Only you do.
