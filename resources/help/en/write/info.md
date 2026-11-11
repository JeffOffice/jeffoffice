# Info: properties, passwords, and Inspect
<!-- keywords: check accessibility, info, properties, title, author, subject, keywords, categories, comments, company, manager, custom properties, docproperty, mark as final, final, read-only, password, encrypt, protect, open password, inspect document, personal information, hidden text, custom xml, check compatibility, version history, versions, keep a version, go back, restore, compare versions, show in folder, copy path, folder, path -->

Click **File > Info** to see the facts about your document, and to protect it before you share it. Press **Esc** to go back to your document.

## Where the file is

At the top, the page shows where the file is saved. Click **Show in Folder** to open that folder. Click **Copy Path** to copy the path, so you can paste it into an email or a chat.

## Properties

Properties are facts about the document, such as its title and who wrote it. The page has a box for each one: **Title**, **Author**, **Subject**, **Keywords**, **Categories**, **Comments**, **Company**, and **Manager**. Type in a box and press **Tab** or **Enter**. JeffWrite keeps the change, and saves it the next time you save the document.

Under the boxes, the page shows what JeffWrite works out: when the document was made, saved, and printed, and how many pages, words, characters, and paragraphs it has.

### Your own properties

Click **Custom Properties** to make properties of your own, such as **Project** or **Client**. Click **Add**, type a name, choose what kind of value it is (text, a whole number, a number, yes or no, or a date), and type the value.

A field can show a custom property. Click **Insert > Field**, and choose a field. Change its code to `DOCPROPERTY "Client"`, and click **OK**. The text shows the value. Press **F9** to update it after you change the property. See [Fields](fields).

## Mark as Final

Click **Mark as Final** to tell readers the document is finished. JeffWrite saves the document and then holds it for reading only. When someone opens it, the title bar says it is read-only, and a note tells them it was marked as final. This is a gentle way to say "please don't change this". It is not a lock, because anyone can choose **Marked as Final** again to edit it.

## Encrypt with Password

Click **Encrypt with Password** to ask for a password whenever the file is opened. Type the password twice, and click **Protect**. The file is scrambled when you save it, so nobody can read it without the password. JeffWrite uses the strong kind of protection that other word processors use, so they can open the file with the password too.

- A password **can't be found again** if you forget it. Keep it somewhere safe.
- The versions you kept and the backup copy follow the password: they are protected with it, and change when you change or remove it.
- To change the password, click the same card again. Click **Remove Password** to take it off. The change is saved the next time you save.
- Passwords work for **.docx** files. A **.odt** file can't hold a password yet, so JeffWrite will not save a protected document as .odt. It tells you instead of saving without the password.

To open a protected file, open it as usual. JeffWrite asks for the password. If it is not right, you can try again, or click **Cancel**.

## Inspect Document

Click **Inspect Document** before you send a document to someone. It lists what the file holds that you may not want to share, and how many of each it found:

- **Comments** are notes that people left in the margin.
- **Tracked changes** are edits that have not been accepted or rejected. **Accept All** keeps every change as part of the text.
- **Document properties and personal information** are the author, the person who last saved it, the company, and the manager.
- **Hidden text** is text set to be hidden. It does not show or print, but it is in the file.
- **Headers and footers** are the text at the top and bottom of the pages.
- **Custom XML data** is data that other programs added to the file. You can't see it on a page.

Click **Remove All** next to a kind to take it out. You can undo what changes the text with **Ctrl+Z**. Save the document afterward to keep the clean copy.

**Remove personal information from this file when saving**, under the cards, does it every time you save, for any file. It is the same box as in **Options**, and the same in every JeffOffice app.

## Check Compatibility

Click **Check Compatibility** to see what an older kind of file can't hold. JeffWrite saves **.docx** and **.odt** files. A .docx file holds everything in your document. An .odt file can't hold a few things, and this list says which. If nothing is listed, nothing is lost.

When you save as an .odt file, JeffWrite lists what the file can't hold **before** it writes anything. You can choose **Save as .odt Anyway**, **Save as .docx Instead** (the .docx file is put beside it, and nothing is lost), or **Cancel**.

## Check Accessibility

Click **Check Accessibility** to open the list of things that may be hard for some people to read or use, such as a picture with no description. It is the same check as **Review > Check Accessibility**. See [Count words, set the language, check accessibility, and look up words](proofing-tools).

## Version History

Click **Version History** to keep named copies of the document on your computer. Nothing is sent anywhere. A version is a whole copy of the document as it is at that moment. Keep one before a big change, so you can get back to it. The document must be saved first, because versions are kept under the file's name.

- **Keep a Version** asks for a name, such as "Before the edit", and keeps a copy.
- **Open a Copy** opens the chosen version in a new window. The version itself does not change.
- **Compare** puts the document and the chosen version side by side as one document, with the differences shown as tracked changes. See [Track changes](track-changes).
- **Go Back to It** makes the document the same as the chosen version. JeffWrite keeps the document as it was as a new version first, so nothing is lost. Save the document to keep the change.
- **Delete** takes the chosen version away.

JeffWrite keeps up to 50 versions of a file and drops the oldest when there are more.
