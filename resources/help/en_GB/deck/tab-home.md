# Home tab
<!-- keywords: home tab, ribbon, clipboard, slides group, new slide, layout, reset, section, delete slide, font group, paragraph group, drawing group, shapes, arrange, shape fill, shape outline, formatting tools, editing group, find, replace, select, select all, select objects -->

The **Home** tab holds the tools you use most while you build slides.

- **Clipboard** has **Paste** (its arrow has **Keep Text Only**, which pastes words with no formatting, and **Paste Special** (Ctrl+Alt+V), which lists the forms the clipboard can go in as: source formatting, text only, or a picture), **Cut**, **Copy**, and **Copy Formatting**, which copies how text or an object looks onto another. Right after you paste on a slide, a small **Paste Options** button shows up by what you pasted. Click it to pick **Keep Source Formatting**, **Keep Text Only**, **Picture** (when the clipboard has a picture), or **Spreadsheet Cells as Table** (when it has cells). JeffDeck takes the paste back and does it again that way, and a single **Undo** takes the whole paste back, however many times you changed your mind. From the keyboard, tap the **Ctrl** key (the **Control** key on a Mac) after a paste to open the button's menu, then use the arrow keys and **Enter** to choose. The button goes away when you change anything, select something else, copy something else, press **Esc**, or scroll the pasted object out of sight. **Set Default Paste** is on the **Advanced** page of **Options**: a box for pasting within the same presentation, between presentations, and from other programs, each **Keep Source Formatting** or **Keep Text Only**. See [Type and format text](text).
- **Slides** has **New Slide** (Ctrl+M, with an arrow for the layouts and **Reuse Slides…**), **Layout**, **Reset**, **Section**, and **Delete Slide**. See [Add, change, and delete slides](slides).
- **Font** changes the font, size, colour, and style of letters, their spacing, and their case. See [Type and format text](text).
- **Paragraph** has bullets, numbering, list levels, line spacing, columns, alignment, text direction, and where the text sits in its box. See [Type and format text](text).
- **Drawing** has **Shapes**, **Arrange**, **Shape Fill**, and **Shape Outline**. See [Shapes, pictures, tables, and Text Art](objects). The **Shape Format** tab and the Format Shape pane (Ctrl+1) have more.
- **Editing** has **Find** (Ctrl+F), **Replace** (Ctrl+H), **Go To** (Ctrl+G), and **Select**. Find and Replace open a pane on the right that searches every slide and its notes. **Go To** opens a window with a **Go to what** list: **Slide**, **Section**, **Comment**, or **Object**. It can also go to a section, a comment, or an object. **Select** has **Select All** (Ctrl+A) and **Select Objects**, which goes back to the pointer after you draw.

## Related

- [Type and format text](text)
- [Add, change, and delete slides](slides)
- [Shapes, pictures, tables, and Text Art](objects)

## The Clipboard pane

The Clipboard pane remembers the last 24 things you copied or cut in this window's program, so you can paste an older one without copying it again. Click the small arrow in the corner of the **Clipboard** group on the **Home** tab (or press **Alt**, then **H**, then **FO**) to open it. A panel appears beside the page.

Each item shows a small preview and what kind of thing it is: text, formatted text, cells, a picture, shapes, or slides. The newest one is at the top. Click an item to paste it where the cursor or the selection is. It pastes like any other paste: one step of **Undo** takes it back, and the Paste Options button appears as usual.

- **Paste All** pastes every item, the oldest first, so they read in the order you copied them.
- **Delete** removes the item you picked. **Clear All** removes all of them.
- The pane collects only while it is open. To keep collecting after you close it, tick **Keep collecting when this pane is closed**. The choice is remembered, and collecting starts when the program starts, before you open the pane.
- To also collect what you copy in other programs, tick **Also collect what is copied in other programs**. Only plain text and pictures are kept, and it starts off again each time you open the program. With this on, a copy made in another program is read in full before this program decides whether to keep it, so a very large copy can make this program pause for a moment and use as much memory as the copy is big. Only one form of it is read: its text when it has text, otherwise one picture. With this off, nothing copied in other programs is read.

The items are kept in the program's memory only. Nothing is saved to a file, and the list is emptied when you close the last window of the program. An item stays in the list after the document it came from is closed. An item bigger than 8 MB is not kept, and the pane tells you when that happens; a picture counts 4 bytes for each dot (pixel), so a picture of more than about 2 million pixels is too big. All the items together are held to 64 MB, and the oldest go first. Anything a password manager marks as secret is never kept.
