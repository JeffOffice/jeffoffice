# Info: properties, passwords, and Inspect
<!-- keywords: info, properties, title, author, subject, keywords, categories, comments, company, manager, custom properties, mark as final, final, read-only, password, encrypt, protect, inspect workbook, personal information, hidden sheets, notes, external links, custom xml, show in folder, copy path, folder, path, version history, versions, go back -->

Click **File > Info** to see the facts about your workbook, and to protect it before you share it. Press **Esc** to go back to your workbook.

## Where the file is

At the top, the page shows where the file is saved. Click **Show in Folder** to open that folder. Click **Copy Path** to copy the path, so you can paste it into an email or a chat.

## Properties

Properties are facts about the workbook, such as its title and who wrote it. The page has a box for each one: **Title**, **Author**, **Subject**, **Keywords**, **Categories**, **Comments**, **Company**, and **Manager**. Type in a box and press **Tab** or **Enter**. JeffCrunch keeps the change, and writes it into the file the next time you save the workbook. The page also shows when the workbook was made, saved, and printed.

Click **Custom Properties** to make properties of your own, such as **Project** or **Client**. Click **Add**, type a name, choose what kind of value it is (text, a whole number, a number, yes or no, or a date), and type the value. JeffCrunch saves them as you typed them.

## Mark as Final

Click **Mark as Final** to tell readers the workbook is finished. JeffCrunch saves the workbook and then holds it for reading only. When someone opens it, the title bar says it is read-only. This is a gentle way to say "please don't change this". It is not a lock, because anyone can choose **Marked as Final** again to edit it.

## Encrypt with Password

Click **Encrypt with Password** to ask for a password whenever the file is opened. This is the same as **Protect with Password**. See [Open password-protected workbooks](open-password). A password protects an **.xlsx** file with strong protection. A password **can't be found again** if you forget it.

For stopping changes to a sheet or its layout, see [Protect a sheet or the workbook](protect).

## Protect the sheet or the sheets

**Protect Current Sheet** lets you choose what people may change on the sheet you are on, with a password if you like. **Protect Workbook Structure** stops people from adding, deleting, moving, hiding, or renaming sheets. Both are also on the **Review** tab.

## Version History

**Version History** keeps copies of your workbook on this computer. Nothing is sent anywhere. A copy is called a version. Keep one before a big change.

- **Keep a Version** saves the workbook as it is now, with a name you choose.
- **Open a Copy** opens a version in a window of its own, so you can look at it beside the workbook.
- **Go Back to It** puts a version back as your workbook. The workbook as it is now is kept as a version first, so nothing is lost.
- **Delete** throws a version away.

The workbook must be saved first, because versions belong to a file. Going back works for an .xlsx file.

## Inspect Workbook

Click **Inspect Workbook** before you send a workbook to someone. It lists what the file holds that you may not want to share, and how many of each it found:

- **Notes** are notes that people left on cells. **Remove All** takes them off.
- **Document properties and personal information** are the author, the person who last saved it, the company, and the manager. **Remove All** clears them.
- **Hidden sheets** are sheets that do not show as tabs. **Show All** shows them, so you can see what they hold, and delete the ones you don't want.
- **Hidden rows and columns** are lines you hid by hand. Rows hidden by a filter are not counted. **Show All** shows them, so you can see what they hold.
- **Headers and footers** are the text printed at the top and the bottom of every page. **Remove All** clears them on every sheet.
- **External links** are links to values in other workbooks. **Remove All** turns each cell that reads one into the number or text it shows now, and leaves out the links when you save. A name that stood for a place in another workbook becomes **#REF!**, which is an error that says the place is gone. You can't undo this, so save a copy first if you may want the links back.
- **Custom XML data** is data that other programs added to the file, which you can't see on a sheet. **Remove All** leaves it out when you save.

Save the workbook afterward to keep the clean copy.

## Check Compatibility

Click **Check Compatibility** to see what an older file, an .xls file, would not hold. It lists the parts of the workbook that would be left out or cut to fit, such as charts, notes, or rows past the old limit. It saves nothing, and your workbook stays as it is.
