# Mail merge, envelopes, and labels
<!-- keywords: mail merge, merge, mailings, recipients, address list, form letter, labels, envelopes, merge field, address block, greeting line, rules, if then else, skip record, next record, preview results, finish and merge, directory, email messages, csv, xlsx, ods, vcard -->

A **mail merge** makes many copies of one letter, each with a different person's details in it. You write the letter once. You keep the details of the people in a **list**. JeffWrite puts the first person's name and address in the first copy, the second person's in the second copy, and so on.

The tools are on the **Mailings** tab. For a guided path, click **Mailings > Start Mail Merge > Step-by-Step Mail Merge Wizard**.

## Choose what you are making

Click **Mailings > Start Mail Merge** and pick one:

- **Letters**: one copy of the page for each person. Each copy starts on a new page.
- **Email Messages**: one message for each person. See "Send email messages" below.
- **Envelopes** and **Labels**: a page made for them. See the sections below.
- **Directory**: one list of everyone, with no page break between the people. Use it for a phone list or a catalogue.
- **Clear Merge Type** makes the document an ordinary document again.

JeffWrite keeps the kind of merge and the name of the list inside the file when you save it. When you open the file again, the merge is still there.

## Choose your recipients

A **recipient** is a person who gets a copy. Click **Mailings > Select Recipients**:

- **Type a New List**: type the people in the window that opens. Click **New Entry** for each one. JeffWrite then offers to save the list as a .csv file.
- **Use an Existing List**: pick a file that is already a list. It can be a .csv file or a text file with tabs (one row for each person, and the first row holds the column names), a spreadsheet (.xlsx, .xls, or .ods, from JeffCrunch or another program), or a contacts file (.vcf).
- **Select from Contacts**: pick a contacts file (.vcf). Most address books and phones can save their contacts as one.

Then click **Mailings > Edit Recipient List** to check the list:

- Uncheck a row to leave that person out.
- Click a column heading to sort.
- Type in **Filter recipients** to show only the rows that have your words in them.
- **Find Duplicates** unchecks repeats.
- **Validate Addresses** colours the rows that have no address or a ZIP code that looks wrong.

## Put the details in the letter

A **merge field** is a place holder. It shows the name of a column in double angle marks, such as «First Name». In each copy, it is replaced by that person's detail.

Click where the detail goes, then use **Mailings > Write & Insert Fields**:

- **Insert Merge Field** lists the columns of your list. Pick one.
  - **Barcode from Field**, in the same list, puts in a barcode whose value is a column, such as a part number or a book's 13-digit ISBN. Pick the column, then choose the type and look in the **Insert Barcode** window (what you type there is only a sample). The window offers only what a field can carry: the type, the size, the bar height, the digits, and the QR level. It leaves out a book's price, an add-on, a check character, and the clear background, which a field can't say; a book's ISBN is an EAN-13. In the document the field shows the column's name in guillemets. When you merge, each person's copy gets a barcode of that person's own value, drawn as a picture in the line, so it prints and saves to PDF like any picture. The value is only data: quotes or a backslash in it change nothing. If a value is empty, there is no barcode. If a code can't hold a value (a letter in an EAN-13, or more than 500 characters in Code 128), the copy shows a short message in square brackets in its place, and the other copies are not affected. One merge makes at most 2,000 barcodes, counting each value that can't be held; merge fewer people if it says there are too many. In a saved `.docx`, the field is a DISPLAYBARCODE field with the column inside, so it is kept when you save and open the file again. A field with a value typed in, or of a type JeffWrite does not draw (UPC-E, for example), is left as it is. Email merges put the value in as text.
- **Address Block** puts in a whole mailing address: the name, the company, the street, and the city. Pick the lines you want. A line that has nothing in it is left out. You can also name a country to leave out, such as your own. The **Preview** shows the first person's address.
- **Greeting Line** puts in a line such as "Dear Dr. Fenwick,". You choose the words. The first choice, "Dear Dr. Fenwick,", uses the person's title. If a person has no title, it uses the first and last name instead, as in "Dear Nora Pell,". If a person has no name, the greeting says something else, such as "Dear Sir or Madam,", which you can change.
- **Match Fields** tells the address block and the greeting line which column holds each part, when your column names are unusual.

## Rules

A **rule** makes a copy different for some people. Click **Rules** and pick one:

- **If…Then…Else**: show one text when a column has a value, and another text when it does not. For example, if State is "CA", write "Welcome home".
- **Skip Record If**: leave a person out when a column has a value, such as when the last name is empty.
- **Next Record** and **Next Record If**: use the next person's details in the same place. Labels use this to put a different person in each label.
- **Merge Record #** puts in the number of the person's row in the list. **Merge Sequence #** counts the copies made so far.
- **Ask** and **Fill-in**: ask you a question when you finish the merge. Ask keeps the answer under a name. Fill-in puts the answer in the text.
- **Set Bookmark**: gives a name a value. A cross-reference to the name shows the value.

## Preview the results

Click **Mailings > Preview Results**. A pane opens that shows the page for one person. Use **First Record**, **Previous Record**, **Next Record**, and **Last Record** to go through the people, or type a number. **Find Recipient** jumps to the next person who has your words in a detail.

**Highlight Merge Fields** shades the merge fields in your page grey so you can see where they are. Other fields, such as page numbers, are not shaded. The shading shows on the screen and never prints.

Your own document is not changed. **Check for Errors** tells you when the list has no column for a field, or when no one is included.

## Finish the merge

Click **Mailings > Finish & Merge**:

- **Edit Individual Documents** makes a new document with every person's copy. You can change it before you print.
- **Print Documents** makes that document and opens the **Print** page.
- **Save as PDF** saves every copy in one PDF file.
- **Send Email Messages** makes one .eml message file for each person, in a folder you choose. Choose the column that holds the email addresses, and type a subject. JeffWrite does not send them. Open the files in your email program.

The people you unchecked in the list are left out.

A message file is made only for a person whose address is one email address. If an address holds two (with a comma or a semicolon between them), has a name with it, or has no domain, that person gets no message, and JeffWrite tells you how many. A long subject is saved in pieces that email programs put back together.

A merge shows a progress window, and **Cancel** stops it. If a rule asks a question (Ask or Fill-in), you are asked each different question once, up to 20 of them. After that, the rest use their default answers. If you cancel a question, the whole merge stops. A merge stops, and says so, if it would make more than 10,000 pages, so a very long document with a very long list does not take hours. Merge a part of the list at a time instead.

## Envelopes

Click **Mailings > Envelopes**. Type the delivery address and your return address, and pick the size of the envelope from the list. The sizes are listed by their width and height. Click **Add to Document** to put the envelope on its own page at the start of your document, or **Print** to print it. The page is as wide as the envelope's long side, which is how most printers take an envelope.

To make envelopes for a merge, click **Start Mail Merge > Envelopes**. Each envelope has an address block.

## Labels

Click **Mailings > Labels**. Type the address and pick the sheet from the list. A sheet is named by how many labels it holds and how big each label is. If your sheet is not in the list, pick **Your own sheet** and type the sizes: how many labels go across and down, how big a label is, the space above and beside the first label, and how far apart the labels are.

- **Full page of the same label** puts the same address on every label of the sheet.
- **Single label** puts the address on one label, so you can use up a sheet that is partly used. Say which row and column.

Click **New Document** to make a document you can look at and change, or **Print** to print it.

To make labels for a merge, click **Start Mail Merge > Labels**. The first label has an address block, and every other label starts with a **Next Record** rule. If you change the first label, click **Update Labels** to copy the change to all the others.

## What the file holds

The kind of merge, the name and place of the list, the subject, and the email column are saved in your .docx file. Which people you unchecked, and your Match Fields choices, are not saved. They are asked for again after you open the file. A list is read again from its file each time. If the file is not where it was, JeffWrite looks next to your document, and then asks you to choose it.

A list can have up to 1,000 columns and 4,000,000 cells in all, and a merge uses the first 100,000 people. If a file has more, JeffWrite uses what fits and tells you. Rows that have nothing in them are left out of the list.

## Related

- [Use fields](fields)
- [Print](print)
- [Make and change tables](tables)
