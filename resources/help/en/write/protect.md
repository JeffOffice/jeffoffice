# Protect a document and lock tracking
<!-- keywords: protect, protection, restrict editing, read only, password, lock tracking, track changes options, user name, initials, tracked changes only, comments only, stop protection -->

Protection limits what people can do in a document. These tools are on the **Review** tab.

## Restrict editing

1. Click **Restrict Editing** in the **Protect** group.
2. Choose what people may do:
   - **No changes (read only)**: the text cannot be changed.
   - **Tracked changes**: people can edit, but every edit is tracked, and Track Changes cannot be turned off.
   - **Comments**: people can add comments, but cannot change the text.
   - **Filling in forms**: the text is read only, but people can fill in the form fields that are in it.
3. Type a password, twice. You can leave the password empty. Then anyone can stop the protection.
4. Click **Start Enforcing**.

The title bar then says "Protected". To stop, click **Restrict Editing** again, then **Stop Protection**. Type the password if there is one.

A password keeps honest people from changing the document by mistake. It is not strong security. Do not use protection to hide private information.

The password is kept in the file as a scrambled number, called a hash. The password itself is never saved. If you forget it, JeffWrite cannot get it back.

## Fill in a form

A document can have form fields: boxes to type in, check boxes, and lists to choose from. JeffWrite fills them in but does not add new ones.

- **Type in a field.** Click it and type. A field can allow only so many letters, or change what you type to capitals, or to a number or a date. JeffWrite follows that.
- **Tick a check box.** Click it, or move to it and press Space.
- **Choose from a list.** Click it, or move to it and press Enter. Then pick an entry. Pressing a letter picks the next entry that begins with it.
- **Move between fields.** When the document is protected for filling in forms, press Tab to go to the next field and Shift+Tab to go back. Only the fields can be changed. A part of the document that the person who made the form left out of the protection can be changed too; there Tab is the ordinary key, and Replace All changes only the matches in that part and tells you how many it left. What you type, paste, drop, or correct is checked by where it would land, not by where the cursor is. Pictures, shapes, and tables are not changed in a protected document.
- **Take it back.** Press Undo. Each entry in a field is one step.

A field that is turned off in the form, or one that the form works out by itself, cannot be filled in. In any document, a field's begin, end, and other marks are never deleted piece by piece: select the whole field to delete it. A field in a document that is read only, or open for comments only, cannot be filled in either. With Track Changes on in a document that is not protected, what you type in a text field is a tracked change, and the field's longest text and capitals are not enforced.

## Lock tracking

Click **Tracking Options**, then **Lock Tracking**. Every change from then on is tracked. Nobody can turn Track Changes off, or accept or reject a change, without the password. It is the same as protecting the document for tracked changes.

## Change your name on changes

Click **Tracking Options**, then **Change Tracking Options**. Type your name and your initials. They sign the changes and comments you make from then on.
