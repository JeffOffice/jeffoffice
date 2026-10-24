# Number formats and your language
<!-- keywords: number format, numbers, decimal, decimal mark, decimal places, thousands, separator, grouping, comma, period, point, language, locale, currency, percent, percentage, date, time, scientific, fraction, text, accounting, general, custom, code, sample, Format Cells, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, increase decimal, decrease decimal, comma style, formula separator, semicolon -->

A cell holds a number, and its **number format** says how that number is written on the screen. The format changes what you see, never what the cell holds. Example: the cell holds 0.256. With the Percent format it shows **25.6%**, with Currency it shows **$0.26**, and with General it shows **0.256**. Click the cell and the formula bar still says 0.256.

## Numbers are written the way your computer writes them

JeffCrunch uses the marks of the language your computer is set to. In English (United States), one thousand two hundred thirty-four and a half is **1,234.5**. In German it is **1.234,5**, and in French it is **1 234,5**. The workbook does not change: it keeps one neutral form, so the same file shows 1,234.5 to one person and 1.234,5 to another.

This goes everywhere a number is shown or typed:

- A cell, the copy of a cell, and the width that **AutoFit** gives a column follow the marks of your language.
- The **formula bar** and a cell you edit with **F2** give a number the way you type it. If your language writes a decimal with a comma, type **2,5**, not 2.5.
- In a language that writes decimals with a comma, a formula has **semicolons** between its arguments: **=SUM(A1;A2)**. The names of the functions stay in English.
- Dates and times follow the code of the format: a format that says "month, day, year" shows them in that order, and a format meant for your language (the ready-made ones in the list) uses the order and the names of your language.

Text that the format puts in quotation marks, such as "units", is shown as it is.

## Choose a format

1. Select the cells.
2. Use the **Number** group on the **Home** tab, or press **Ctrl+1** (on a Mac, **Command+1**) to open **Format Cells**, which opens on the **Number** tab.

The box in the **Number** group lists the common formats: General, Number, Currency, Accounting, Short Date, Long Date, Time, Percentage, Fraction, Scientific, Text, and Custom. **More Number Formats** at the end opens the Format Cells window. The buttons below the box are:

| Button | What it does |
| --- | --- |
| Currency | Shows the number as money, with the currency symbol of your language (**Ctrl+Shift+4**). The arrow next to it offers General and **More Number Formats**. |
| Percent Style | Multiplies by 100 and adds a percent sign, with no decimals (**Ctrl+Shift+5**). Click it again to take it away. |
| Comma Style | Groups the digits by thousands, shows two decimals, and lines the decimal points up down a column, with no symbol. |
| Increase Decimal, Decrease Decimal | Adds or takes away one decimal place. |
| General (in the box, and in the arrow next to Currency) | Takes the format away: the number shows as it is (**Ctrl+Shift+~**). |

## The Number tab of Format Cells

Pick a **Category** on the left; the options for it appear on the right, and the **Sample** shows what the first selected cell will look like, in your language.

- **Number**: decimal places, whether to group the digits by thousands, and how negative numbers look (a minus sign, red, or in parentheses).
- **Currency** and **Accounting**: decimal places and a symbol. Accounting lines up the symbols and the decimal points down a column.
- **Date** and **Time**: a list of ready-made formats, with the order and the names of your language.
- **Percentage**, **Scientific**: decimal places.
- **Fraction**: ready-made formats such as "up to one digit" (1/4), "up to two digits" (21/25), "as quarters", and "as sixteenths".
- **Special**: ready-made formats for a ZIP code, a ZIP code plus 4, a phone number, and a social security number.
- **Text**: the cell holds what you type as text, so a number such as 00123 keeps its zeros.
- **Custom**: type a code of your own, or start from one in the list (the common codes, and those the workbook already uses).

The **code** is the short text that tells JeffCrunch how to write a number. Example: **#,##0.00** means "group the digits by thousands and always show two decimals", so 1234.5 shows as 1,234.50. Each **0** is a digit that always shows, each **#** a digit that shows only if it is needed, and **%** multiplies by 100. A code can have up to four sections divided by semicolons, for positive numbers, negative numbers, zero, and text.

## What is kept

The code is saved in the workbook in its neutral form, so the workbook reads right on any computer. Formatting is one step you can undo, and a workbook that already used a format keeps it exactly: a format that is in the list is found again, and none is made twice.
