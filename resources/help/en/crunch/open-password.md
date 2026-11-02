# Open password-protected workbooks
<!-- keywords: password, protected, encrypted, encryption, open, locked, wrong password, cancel, xlsx, xls, ods, file protection, default password, private -->

Some workbooks are protected with a password when they are saved: without it, no program can read what is in the file. This is different from **Protect Sheet** and **Protect Workbook** (see [Protect a sheet or the workbook](protect)), which only stop changes. A workbook that is protected with a password cannot be opened at all until the password is typed.

## Open one

Open the file as you open any other: press **{key:Open}**, choose it, or drag it onto the window. JeffCrunch sees that the file is protected and asks for the password before it reads anything:

1. Type the password in the box that says **This file is protected with a password.** The letters show as dots.
2. Click **Open**.

If the password is not the right one, JeffCrunch says so and asks again, as many times as you like. Click **Cancel** to stop: nothing is opened, and the workbooks you already have open stay as they are. Passwords are exact: the capital letters, the spaces, and the accents all count.

JeffCrunch uses the password only to open the file. It does not save the password, does not write it anywhere, and forgets it as soon as the file is read, so the next time you open the same file it asks again.

## What can be opened

- **.xlsx and .xlsm workbooks** protected in either of the two ways the original program has used since 2007: the newer one (the key is 128, 192, or 256 bits, and JeffCrunch also checks that nobody changed the file after it was protected) and the older one (128 bits).
- **.xls workbooks** protected in any of the three ways of that format: the simple scrambling of the oldest files, the 40-bit kind, and the stronger kind that uses the computer's own key service.
- **.ods spreadsheets** protected with a password, both the current kind (256 bits) and the older kind that older versions of LibreOffice and OpenOffice wrote.

Some files are locked with a password that everyone knows, so that they can be opened by anyone but not changed by accident. JeffCrunch opens those without asking.

## When a file can't be opened

- **"The password is not correct."** The password you typed is not the one the file was protected with. Try again with care for capitals and spaces; JeffCrunch can't recover a lost password.
- **"This file is protected in a way this program can't open."** The file is locked by something other than a password (for example a key held by a certificate), or in a form JeffCrunch does not read.
- **"The file is damaged, or it was changed after it was protected."** The password was right, but the file's contents are not what was locked: it was cut short, damaged, or changed by another program. Copy it again from the original.

## Saving

A workbook you opened this way is a new workbook as far as JeffCrunch is concerned: when you save it, the copy is **not** protected with a password, and the file you opened is never changed. To protect the copy again, use the program that made the original.
