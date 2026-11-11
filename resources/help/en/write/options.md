# Change JeffWrite's settings
<!-- keywords: overtype, insert key, typing replaces selection, ctrl+click, follow link, drag and drop, personal information, remove personal information, advanced, units, measurements, inches, centimeters, options, settings, preferences, save, default format, default folder, default file location, recovery, formatting marks, gridlines, display, usage statistics, install number, privacy, privacy notice, JP_NO_TELEMETRY, delete statistics, updates, about, customize keyboard, customize the ribbon, hide a tab, hide a group, add a tab, add a group, rename a tab, reset the ribbon, change shortcut keys, reassign keys, reset keys -->

Your settings are in the **JeffWrite Options** window. Click **File > Options** to open it. Click **OK** to keep your changes, or **Cancel** to throw them away.

## Save

The **Save** page of the Options window has four choices.

- **Save new documents as** picks the kind of file that **Save As** offers first for a document that has no file yet: a .docx file or an .odt file.
- **Save automatic recovery information every** is how many minutes pass between the spare copies JeffWrite keeps of unsaved work. See [Open, save, print, and export](open-save).
- **Default file location** is the folder the **Open** and **Save As** boxes start in. Click **Browse** to pick it. Leave it empty to start in your Documents folder.
- **Remove personal information from this file when saving** takes out the author, the person who saved the file last, the company, the manager, the names and initials on comments, and the names and times on tracked changes, from the file that is written (a .docx or .odt file, a template, and a document sent by email). What the file says stays as it is. The file's custom properties are left as they are, and the document you have open is not changed. It is the same choice in every JeffOffice app: change it in one and it changes in all of them. To do this just once, use **Inspect Document** in [Info](info). The same box is on the **Info** page.

## Display

The **Display** page chooses what a window shows when it opens.

- **Show formatting marks** turns on the paragraph marks and other signs that do not print. Each window can still turn them on or off with the **¶** button.
- **Show table gridlines** draws the faint edges of table cells that have no lines.

A change on this page applies to the window you are in at once, and to each window you open after it.

## Advanced

**Show measurements in** picks the unit for the ruler and the dialogs: inches, centimeters, millimeters, points, or picas. A point is 1/72 of an inch. A pica is 12 points.

**Use Ctrl+click to follow a link** is on at first: you hold **Ctrl** and click a link to go to it, and a plain click only puts the cursor in the link's words. Turn it off to follow a link with a plain click. Holding **Shift** while you click still selects text.

**Allow text to be dragged and dropped** lets you select words and drag them to a new place. Hold **Ctrl** while you drop to copy them instead of moving them. Turn it off if you drag by accident.

**Typing replaces the selected text** is on at first: when words are selected and you type, the new letters take their place. Turn it off and the new letters go after the selected words instead.

**Use the Insert key to turn overtype mode on and off** is off at first. When it is on, the **Insert** key switches between two ways of typing. Normally each letter you type pushes the letters after it along. In **overtype** mode each letter you type replaces the letter after the cursor. Press **Insert** again to go back.

These choices apply to the window you are in at once, and to each window you open after it.

## Customize

The **Customize** tab has two pages, **Ribbon** and **Keyboard**. The **Keyboard** page is where you choose the shortcut keys for JeffWrite's commands. See "Choose your own shortcut keys" in the keyboard topic.

### Customize the ribbon

The **Ribbon** page lists the ribbon's tabs, each with its groups.

- **Hide or show a tab or a group.** Clear the box beside it to hide it, and tick the box to show it again. A hidden group takes no room, and its commands still work from their shortcut keys. **Home** has no box, because it always stays, and so does the **File** button. A tab that appears only when something is selected, such as a picture or a table, is at the end of the list and has no box.
- **Make your own tab or group.** Type a name in the **Name** box and click **New Tab**. To make a group, choose a tab and click **New Group**. A group of your own can go on any tab, including a tab that appears only when something is selected.
- **Put commands in your group.** Choose your group, find a command in the **Commands** list (type in the **Search commands** box to narrow the list), and click **Add Command**. Any command that has a name can go in. A command can be in a group only once.
- **Rename.** Choose a tab, group, or command of your own, type the new name, and click **Rename**. A command you rename does the same thing, and the new name is only what its button says. Rename a command with an empty name to give it its own name back.
- **Reorder.** Choose a tab and click **Move Up** or **Move Down**. You can move the ribbon's own tabs as well as yours, but not the tabs that appear only when something is selected. Your groups and your commands move the same way within their tab or group. The ribbon's own groups stay in their order.
- **Take away.** Choose a tab, group, or command of your own and click **Remove**.

**Reset One** puts back the chosen tab: it is shown, its groups are shown, and the groups you added to it are gone. A tab of your own is taken away. **Reset All** puts back the whole ribbon, including the order of the tabs.

Nothing changes until you click **OK**. Then every open JeffWrite window shows your ribbon, and so does every window you open later. JeffWrite keeps only what differs from the usual ribbon, so a tab or group a later version adds shows up on its own. Your tabs and groups get KeyTips (the letters shown when you press **Alt**) that do not clash with the others. You can have 20 tabs of your own, 30 groups of your own on a tab, and 60 commands in a group. The quick toolbar cannot be changed.

## Usage statistics

**Send anonymous usage statistics** lets you share facts about how JeffWrite is used. Anonymous means the facts say nothing about who you are. When this is on, JeffWrite sends these facts once a day, and on the day you update: that it is JeffWrite, its version, your operating system and language, how many times it was started, how often each command is used, and what kind of file you opened or saved. If JeffWrite ever closes by surprise, it also sends a crash report the next time it starts. JeffWrite never sends your files, their names, or anything in them, and nothing you typed. If this is off, no crash report is sent either.

The choice belongs to all the JeffOffice apps: change it here and it changes in each of them. JeffWrite asks about it the first time it starts, in a window called **Help improve JeffWrite**.

- **Privacy notice** next to the check box opens our privacy notice at jeffoffice.com/privacy. It says what is sent, where it goes, and how long we keep it. We keep statistics for 24 months.
- **Install number** is shown under the check box. It is a random number made up for your copy of JeffOffice, the same in every JeffOffice app. It says nothing about you. Click **Copy** to copy it. If statistics are off, or no number has been made yet, it says **none**.
- To have the statistics we already hold deleted, send your install number to info@jeffoffice.com **before** you turn statistics off. Turning them off erases the number from your computer, and without it we cannot tell which records are yours.
- To turn statistics off without opening JeffWrite, set the environment variable `JP_NO_TELEMETRY` to any value before you start it. Nothing is then counted or sent, and JeffWrite does not ask the first-run question.

## Related

- [Welcome to JeffWrite](start)
