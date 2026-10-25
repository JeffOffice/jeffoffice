# Protect a sheet or the workbook
<!-- keywords: protect, protection, password, lock, locked cells, unlock, unprotect, structure, sheets, read only, allow, format cells, insert rows, sort, filter, hash -->

## Protect a sheet

**Review > Protect Sheet** keeps people from changing the sheet by accident. In a protected sheet, the cells that are **locked** can't be changed. Every cell is locked until you say otherwise: select the cells that may be changed, open **Format Cells > Protection**, and clear **Locked**. (This matters only once the sheet is protected.)

The dialog asks what everyone may still do on the sheet: select locked or unlocked cells, format cells, change columns or rows, insert or delete rows and columns, put in links, sort, use filters, and change drawing objects and notes. Whatever you leave unchecked is refused, with a message that says what was refused.

A **password** is optional. If you give one, the sheet can be unprotected only with it. JeffCrunch keeps a scrambled form of the password (a salted hash, repeated 100,000 times) and never the password itself, so it can't show you the password later: keep it somewhere safe. Passwords that other programs made, in either the older or the newer form, are checked the same way.

Protecting and unprotecting are steps you can undo. To take the protection off, use **Unprotect Sheet**, and type the password if there is one.

A few things stay off limits on a protected sheet whatever you allow: merging cells, and deleting rows or columns that hold locked cells.

## Protect the workbook

**Review > Protect Workbook** locks the **structure**: no sheet can be added, deleted, renamed, moved, hidden, or shown. It can also ask that the windows be kept as they are. It may have a password too. **Unprotect Workbook** takes it off.
