# Open password-protected workbooks
<!-- keywords: password, protected, encrypted, encryption, open, locked, wrong password, cancel, xlsx, xls, ods, file protection, default password, private -->

Some workbooks are protected with a password when they are saved: without it, no program can read what is in the file. This is different from **Protect Sheet** and **Protect Workbook** (see [Protect a sheet or the workbook](protect)), which only stop changes. A workbook that is protected with a password cannot be opened at all until the password is typed. This page says how to open one, and how to make one.

## Open one

Open the file as you open any other: press **{key:Open}**, choose it, or drag it onto the window. JeffCrunch sees that the file is protected and asks for the password before it reads anything:

1. Type the password in the box that says **This file is protected with a password.** The letters show as dots.
2. Click **Open**.

If the password is not the right one, JeffCrunch says so and asks again, as many times as you like. Click **Cancel** to stop: nothing is opened, and the workbooks you already have open stay as they are. Passwords are exact: the capital letters, the spaces, and the accents all count.

JeffCrunch does not write the password anywhere. It keeps it in memory while the workbook is open, so that **Save** can protect the file again, and wipes it when you close the workbook. The next time you open the file, it asks again.

## What can be opened

- **.xlsx and .xlsm workbooks** protected in either of the two ways the original program has used since 2007: the newer one (the key is 128, 192, or 256 bits, and JeffCrunch also checks that nobody changed the file after it was protected) and the older one (AES with a key of 128, 192, or 256 bits).
- **.xls workbooks** protected in any of the three ways of that format: the simple scrambling of the oldest files, the 40-bit kind, and the stronger kind that uses the computer's own key service.
- **.ods spreadsheets** protected with a password, in all three ways LibreOffice and OpenOffice have written them: the newest (LibreOffice 25 and later lock the whole file with a 256-bit key, and make the key from the password with a method that is made slow on purpose and uses a lot of memory, so opening such a file takes a moment longer), the 256-bit kind of the versions before it, and the oldest kind.

Older .xls files have limits on the password itself. The simple scrambling of the oldest files uses only the first 15 characters of a password and can hold only letters, digits, and signs of the Western (Latin-1) character set: a password with other letters (such as Greek or Chinese) is never taken for the right one there. The 40-bit kind takes any letters, but only the first 16 characters. JeffCrunch does not write .xls files with a password.

Some files are locked with a password that everyone knows, so that they can be opened by anyone but not changed by accident. JeffCrunch opens those without asking.

## When a file can't be opened

- **"The password is not correct."** The password you typed is not the one the file was protected with. Try again with care for capitals and spaces; JeffCrunch can't recover a lost password.
- **"This file is protected in a way this program can't open."** The file is locked by something other than a password (for example a key held by a certificate), or in a form JeffCrunch does not read.
- **"This file's password protection asks for more memory or time than this program allows, so it is not opened."** A file from LibreOffice 25 or later sets how much memory and time it takes to check the password. JeffCrunch accepts up to ten times what LibreOffice itself sets (640 MiB of memory), so that a file made to tie up your computer can't. A file that LibreOffice wrote itself is well inside that.
- **"The file is damaged, or it was changed after it was protected."** The password was right, but the file's contents are not what was locked: it was cut short, damaged, or changed by another program. Copy it again from the original.

## Save with a password

Click **File**, then **Protect with Password** (or **Review > Protect with Password**), type the password twice, and click **Protect**. The workbook is written protected the next time you save it: anyone who opens the file must type the password. To change the password, choose the same command and type a new one; to take it off, click **Remove Password**, and save.

A workbook you opened with a password keeps it while it is open, so **Save** writes the file protected again with the same password. JeffCrunch holds the password in memory only until you close the workbook, and then wipes it. The file you opened with the password is not left unprotected by accident: if you save a copy as an .xls or .ods file, which JeffCrunch can't protect, it tells you first that the copy will not be protected, and you can cancel.

Use a long password, and keep it somewhere safe: nobody, including us, can open the file without it.
