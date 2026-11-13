# Зміна настройок JeffCrunch
<!-- keywords: відновлення, відновити незбережену роботу, автоматичне відновлення, перевірка, редагування, автозавершення, маркер заповнення, перетягування, перетягнути й вставити, десяткові знаки, параметри, настройки, налаштування, статистика використання, номер встановлення, конфіденційність, повідомлення про конфіденційність, JP_NO_TELEMETRY, видалити статистику, оновлення, про програму, редагування в комірці, колір сітки, масштаб за замовчуванням, стандартна ширина, персональні дані, настроювання клавіатури, настроювання стрічки, настроювання панелі швидкого доступу, панель швидкого доступу, додати команду на панель швидкого доступу, приховати вкладку, приховати групу, додати вкладку, додати групу, перейменувати вкладку, скинути стрічку, змінити комбінації клавіш, призначити клавіші, скинути клавіші -->

Your settings are in the **JeffCrunch Options** window. Click **File > Options** to open it. Click **OK** to keep your changes, or **Cancel** to throw them away.

## Editing

The **Editing** page changes how typing in cells works.

- **After pressing Enter, move the selection** says where the selection goes when you finish an entry: down, right, up, left, or nowhere.
- **Offer to finish a text from the cells above and below** is called AutoComplete. When you type the first letters of a text, and only one text in that column starts with them, JeffCrunch shows the rest of it, selected. Press **Enter** to keep it. Keep typing to ignore it, or press **Backspace** to take it away. It looks only at the block of cells next to the one you are typing in, and only at texts, not numbers.
- **Use the fill handle, and drag cells to move or copy them** turns on the small square at the corner of the selection and the dragging of the selection's border. Turn it off if you keep moving cells by accident. Copy and Paste still work.
- **Show a zero in cells that have a zero value (this sheet)** turns zeros on or off for the sheet you are looking at. When it is off, a cell that holds 0 looks empty, but it still holds the 0 and adds up as 0. The choice is saved with the sheet.
- **Put a decimal point in whole numbers that are typed** is called automatic decimal places. Set **Places** to 2, and typing 1234 enters 12.34. Set it to -2, and typing 5 enters 500. A number you type with a decimal point is entered as typed.

- **Allow editing directly in cells** is on by default. When you turn it off, a typed entry or F2 goes to the formula bar, and the cell shows the result only when you finish.

## Save options

**Save automatic recovery information every** sets how often JeffCrunch keeps a copy of a workbook with changes you have not saved, from 1 to 120 minutes. The copy is a .xlsx file in a folder that JeffCrunch keeps for this purpose, made in the background so you can go on typing. It is removed when you save or close the workbook. If JeffCrunch closes by surprise, it offers the copies the next time it starts. You can also click **File > Open > Recover Unsaved Work**. A workbook that is protected with a password is not copied, because the copy would not be protected.

## Proofing options

The **Proofing** page is shared by the JeffOffice apps. It decides whether spelling is checked as you type in a cell (turn off **Check spelling as you type** to get rid of the red wavy lines in every open window and the other apps; **Spelling** on the Review tab still works), whether words in capital letters and words with numbers are ignored when spelling is checked, whether AutoCorrect fixes words as you type, whether straight quote marks become curly ones, whether dashes, fractions, and ordinals are changed, and whether a web address becomes a link. See [Spelling](notes-links) and [Type in cells](edit-cells).

## Display options

- **Gridline color** is the color of the lines between cells on your screen. It is a choice for your screen only, not saved in the workbook.
- **Zoom of a new workbook** is how large the sheets of a new workbook look, from 10% to 400%.
- **Standard column width (this sheet)** is the width of every column of this sheet that has no width of its own. It is the same as **Default Width** on the Home tab.

## Customize

The **Customize** tab has three pages, **Ribbon**, **Quick Toolbar**, and **Keyboard**. The **Keyboard** page is where you choose the shortcut keys for JeffCrunch's commands. See "Choose your own shortcut keys" in the keyboard topic.

### Customize the ribbon

The **Ribbon** page lists the ribbon's tabs, each with its groups.

- **Hide or show a tab or a group.** Clear the box beside it to hide it, and tick the box to show it again. A hidden group takes no room, and its commands still work from their shortcut keys. **Home** has no box, because it always stays, and so does the **File** button. A tab that appears only when something is selected, such as a picture or a table, is at the end of the list and has no box.
- **Make your own tab or group.** Type a name in the **Name** box and click **New Tab**. To make a group, choose a tab and click **New Group**. A group of your own can go on any tab, including a tab that appears only when something is selected.
- **Put commands in your group.** Choose your group, find a command in the **Commands** list (type in the **Search commands** box to narrow the list), and click **Add Command**. Any command that has a name can go in. A command can be in a group only once.
- **Rename.** Choose a tab, group, or command of your own, type the new name, and click **Rename**. A command you rename does the same thing, and the new name is only what its button says. Rename a command with an empty name to give it its own name back.
- **Reorder.** Choose a tab and click **Move Up** or **Move Down**. You can move the ribbon's own tabs as well as yours, but not the tabs that appear only when something is selected. Your groups and your commands move the same way within their tab or group. The ribbon's own groups stay in their order.
- **Take away.** Choose a tab, group, or command of your own and click **Remove**.

**Reset One** puts back the chosen tab: it is shown, its groups are shown, and the groups you added to it are gone. A tab of your own is taken away. **Reset All** puts back the whole ribbon, including the order of the tabs.

Nothing changes until you click **OK**. Then every open JeffCrunch window shows your ribbon, and so does every window you open later. JeffCrunch keeps only what differs from the usual ribbon, so a tab or group a later version adds shows up on its own. Your tabs and groups get KeyTips (the letters shown when you press **Alt**) that do not clash with the others. You can have 20 tabs of your own, 30 groups of your own on a tab, and 60 commands in a group.

### Customize the quick toolbar

The **Quick Toolbar** page is for the small toolbar at the top left of the window. Its left list shows what is on the toolbar now, and the **Commands** list on the right has every command that has a name (type in the **Search commands** box to narrow it, the same as on the **Ribbon** page).

- **Add a command.** Choose it in the **Commands** list and click **Add Command**. It goes at the end of the toolbar. A command that opens a menu keeps its menu on the toolbar. A command can be on the toolbar only once.
- **Right-click.** You can also right-click any command on the ribbon and choose **Add to Quick Toolbar**, or right-click a button on the toolbar and choose **Remove from Quick Toolbar**. The same menu has **Customize the Quick Toolbar** and **Customize the Ribbon**, which open this page.
- **Add a separator.** Click **Add Separator** to put a thin line at the end, then move it where you want it.
- **Reorder.** Choose a command or a separator in the left list and click **Move Up** or **Move Down**.
- **Take away.** Choose it and click **Remove**.

**Reset One** puts back the chosen command: where JeffCrunch usually has it, or off the toolbar when JeffCrunch does not usually have it there. **Reset All** puts back the toolbar JeffCrunch comes with. Reset All on this page does not change your ribbon, and Reset All on the **Ribbon** page does not change your toolbar.

Nothing changes until you click **OK**. Then every open JeffCrunch window shows your toolbar, and so does every window you open later. The toolbar can have 100 commands and separators in all. When there are more buttons than fit in a third of the window's width, the rest wait behind a **»** button at the end of the toolbar, so the title of the window is never pushed off the screen. JeffCrunch keeps a command in your list when a later version no longer has it, and shows it again if the command comes back. The buttons get KeyTips, the numbers 1 to 9 and then two letters, in the order they are on the toolbar. The toolbar always sits at the top left, beside the **File** button.

## Privacy options

- **Remove personal information from this file when saving** takes out the author, the person who saved the file last, the company, the manager, and the names and times on notes and comments, from the file that is written (a workbook, a copy, and a workbook sent by email). What the file says stays as it is. The file's custom properties are left as they are, and the document you have open is not changed. It is the same choice in every JeffOffice app: change it in one and it changes in all of them. To do this just once, use **Inspect Workbook** in [Info](info). The same box is on the **Info** page.

## Usage statistics

**Send anonymous usage statistics** lets you share facts about how JeffCrunch is used. Anonymous means the facts say nothing about who you are. When this is on, JeffCrunch sends these facts once a day, and on the day you update: that it is JeffCrunch, its version, your operating system and language, how many times it was started, how often each command is used, and what kind of file you opened or saved. If JeffCrunch ever closes by surprise, it also sends a crash report the next time it starts. JeffCrunch never sends your files, their names, or anything in them, and nothing you typed. If this is off, no crash report is sent either.

The choice belongs to all the JeffOffice apps: change it here and it changes in each of them. JeffCrunch asks about it the first time it starts, in a window called **Help improve JeffCrunch**.

- **Privacy notice** next to the check box opens our privacy notice at jeffoffice.com/privacy. It says what is sent, where it goes, and how long we keep it. We keep statistics for 24 months.
- **Install number** is shown under the check box. It is a random number made up for your copy of JeffOffice, the same in every JeffOffice app. It says nothing about you. Click **Copy** to copy it. If statistics are off, or no number has been made yet, it says **none**.
- To have the statistics we already hold deleted, send your install number to info@jeffoffice.com **before** you turn statistics off. Turning them off erases the number from your computer, and without it we cannot tell which records are yours.
- To turn statistics off without opening JeffCrunch, set the environment variable `JP_NO_TELEMETRY` to any value before you start it. Nothing is then counted or sent, and JeffCrunch does not ask the first-run question.

## Related

- [Welcome to JeffCrunch](start)
