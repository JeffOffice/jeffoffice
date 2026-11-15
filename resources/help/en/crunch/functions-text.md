# Text functions
<!-- keywords: functions, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE -->

These are the text functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## ASC

`ASC(text)`

Changes full-width letters and digits in text to half-width ones.

- `text`: The text to work on, or a cell that holds it.

## BAHTTEXT

`BAHTTEXT(number)`

Writes a number as Thai words with the word for baht.

- `number`: The number to work on.

## CHAR

`CHAR(number)`

Gives the character that has a code number.

- `number`: The number to work on.

## CLEAN

`CLEAN(text)`

Takes characters that do not print out of a text.

- `text`: The text to work on, or a cell that holds it.

## CODE

`CODE(text)`

Gives the code number of the first character of a text.

- `text`: The text to work on, or a cell that holds it.

## CONCAT

`CONCAT(text1, ...)`

Joins texts and the texts in blocks of cells into one.

- `text1`: The first text, or block of cells with text.
- `...`: As many more as you need, of the same kind as the one before it.

## CONCATENATE

`CONCATENATE(text1, ...)`

Joins texts into one.

- `text1`: The first text, or block of cells with text.
- `...`: As many more as you need, of the same kind as the one before it.

## DBCS

`DBCS(text)`

Changes half-width letters and digits in text to full-width ones.

- `text`: The text to work on, or a cell that holds it.

## DOLLAR

`DOLLAR(number, [decimals])`

Turns a number into text as an amount of money.

- `number`: The number to work on.
- `decimals` (optional): How many digits to keep after the decimal point.

## ENCODEURL

`ENCODEURL(text)`

Changes text so it can be used in a web address.

- `text`: The text to work on, or a cell that holds it.

## EXACT

`EXACT(text1, text2)`

True when two texts are exactly the same, capitals included.

- `text1`: The first text, or block of cells with text.
- `text2`: The second text.

## FILTERXML

`FILTERXML(xml, xpath)`

Picks values out of text that holds XML, using an XPath expression.

- `xml`: The text that holds the XML.
- `xpath`: The XPath that says which parts to give, such as //item/price.

## FIND

`FIND(find_text, within_text, [start_num])`

Gives where a text starts inside another, telling capitals from small letters.

- `find_text`: The text to look for.
- `within_text`: The text to look in.
- `start_num` (optional): The place in the text to start at, counting the first character as 1.

## FINDB

`FINDB(find_text, within_text, [start_num])`

Gives where text first appears in other text, telling capitals apart.

- `find_text`: The text to look for.
- `within_text`: The text to look in.
- `start_num` (optional): The place in the text to start at, counting the first character as 1.

## FIXED

`FIXED(number, [decimals], [no_commas])`

Turns a number into text with a set number of decimals.

- `number`: The number to work on.
- `decimals` (optional): How many digits to keep after the decimal point.
- `no_commas` (optional): True to leave out the commas that group thousands.

## LEFT

`LEFT(text, [num_chars])`

Gives the first characters of a text.

- `text`: The text to work on, or a cell that holds it.
- `num_chars` (optional): How many characters to take (1 if left out).

## LEFTB

`LEFTB(text, [num_chars])`

Gives the first characters of text.

- `text`: The text to work on, or a cell that holds it.
- `num_chars` (optional): How many characters to take (1 if left out).

## LEN

`LEN(text)`

Counts the characters in a text.

- `text`: The text to work on, or a cell that holds it.

## LENB

`LENB(text)`

Gives the number of characters in text.

- `text`: The text to work on, or a cell that holds it.

## LOWER

`LOWER(text)`

Turns a text into small letters.

- `text`: The text to work on, or a cell that holds it.

## MID

`MID(text, start_num, num_chars)`

Gives characters from the middle of a text.

- `text`: The text to work on, or a cell that holds it.
- `start_num`: The place in the text to start at, counting the first character as 1.
- `num_chars`: How many characters to take (1 if left out).

## MIDB

`MIDB(text, start_num, num_chars)`

Gives characters from the middle of text.

- `text`: The text to work on, or a cell that holds it.
- `start_num`: The place in the text to start at, counting the first character as 1.
- `num_chars`: How many characters to take (1 if left out).

## NUMBERVALUE

`NUMBERVALUE(text, [decimal_separator], [group_separator])`

Reads text as a number, with the separators you name.

- `text`: The text to work on, or a cell that holds it.
- `decimal_separator` (optional): The character that marks the decimals in the text (the point if left out).
- `group_separator` (optional): The character that groups the digits in the text (the comma if left out).

## PHONETIC

`PHONETIC(reference)`

Gives the text of a cell, for the readings of Japanese text.

- `reference`: A cell or block of cells.

## PROPER

`PROPER(text)`

Gives each word of a text a capital first letter.

- `text`: The text to work on, or a cell that holds it.

## REPLACE

`REPLACE(old_text, start_num, num_chars, new_text)`

Puts new text in place of some characters of a text.

- `old_text`: The text to change.
- `start_num`: The place in the text to start at, counting the first character as 1.
- `num_chars`: How many characters to take (1 if left out).
- `new_text`: The text to put in.

## REPLACEB

`REPLACEB(old_text, start_num, num_chars, new_text)`

Replaces part of text with other text.

- `old_text`: The text to change.
- `start_num`: The place in the text to start at, counting the first character as 1.
- `num_chars`: How many characters to take (1 if left out).
- `new_text`: The text to put in.

## REPT

`REPT(text, number_times)`

Repeats a text a number of times.

- `text`: The text to work on, or a cell that holds it.
- `number_times`: How many times to repeat the text.

## RIGHT

`RIGHT(text, [num_chars])`

Gives the last characters of a text.

- `text`: The text to work on, or a cell that holds it.
- `num_chars` (optional): How many characters to take (1 if left out).

## RIGHTB

`RIGHTB(text, [num_chars])`

Gives the last characters of text.

- `text`: The text to work on, or a cell that holds it.
- `num_chars` (optional): How many characters to take (1 if left out).

## SEARCH

`SEARCH(find_text, within_text, [start_num])`

Gives where a text starts inside another, ignoring capitals, and allowing ? and * in what is sought.

- `find_text`: The text to look for.
- `within_text`: The text to look in.
- `start_num` (optional): The place in the text to start at, counting the first character as 1.

## SEARCHB

`SEARCHB(find_text, within_text, [start_num])`

Gives where text first appears in other text, ignoring capitals.

- `find_text`: The text to look for.
- `within_text`: The text to look in.
- `start_num` (optional): The place in the text to start at, counting the first character as 1.

## SUBSTITUTE

`SUBSTITUTE(text, old_text, new_text, [instance_num])`

Swaps one piece of text for another wherever it occurs.

- `text`: The text to work on, or a cell that holds it.
- `old_text`: The text to find.
- `new_text`: The text to put in.
- `instance_num` (optional): Which match to replace (all of them if left out).

## T

`T(value)`

Gives the text itself, and nothing for anything that is not text.

- `value`: The value to work on, or a cell that holds it.

## TEXT

`TEXT(value, format_text)`

Turns a number into text in a format you choose.

- `value`: The value to work on, or a cell that holds it.
- `format_text`: The number format to show the value in, such as "0.00" or "mm/dd/yyyy".

## TEXTJOIN

`TEXTJOIN(delimiter, ignore_empty, text1, ...)`

Joins texts with a separator between them.

- `delimiter`: The text to put between the pieces.
- `ignore_empty`: True to leave out empty pieces; false to keep them.
- `text1`: The first text, or block of cells with text.
- `...`: As many more as you need, of the same kind as the one before it.

## TRIM

`TRIM(text)`

Takes extra spaces out of a text, leaving single spaces between words.

- `text`: The text to work on, or a cell that holds it.

## UNICHAR

`UNICHAR(number)`

Gives the character that goes with a Unicode number.

- `number`: The number to work on.

## UNICODE

`UNICODE(text)`

Gives the Unicode number of the first character of text.

- `text`: The text to work on, or a cell that holds it.

## UPPER

`UPPER(text)`

Turns a text into capital letters.

- `text`: The text to work on, or a cell that holds it.

## VALUE

`VALUE(text)`

Turns a text that looks like a number into a number.

- `text`: The text to work on, or a cell that holds it.
