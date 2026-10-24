# Rules for what a cell may hold
<!-- keywords: validation, rule, data validation, list, drop-down, dropdown, whole number, decimal, date, time, length, custom, stop, warning, information, error alert, not allowed, choices, arrow, Alt+Down -->

A workbook can have **rules** that say what may go in some cells: a whole number from 1 to 10, a date after a given day, a text of at most five letters, or one choice from a list. The rules come from the workbook; JeffCrunch applies them when you type or paste.

## When a value does not fit

Each rule has a **style**, and the message is the rule's own when the workbook has one:

- **Stop**: the message appears and the entry stays in the cell for you to change. Press **Esc** to give it up.
- **Warning**: you are asked whether to keep the value anyway.
- **Information**: you are told the value does not fit, and may go on or give up.

A value you paste is held to the rule too. If any pasted cell does not fit a rule that stops, nothing is pasted, and the message tells how many cells did not fit. Clearing a cell is always allowed.

## Choose from a list

A cell that has a list shows a small button with an arrow at its right edge when it has the cursor. Click the button, or press **Alt+Down**, to see the choices and click one to enter it.

## What is not done

JeffCrunch keeps the rules the workbook has but has no command to make new ones yet. Rules that read their choices from another sheet in the newer way of writing them are kept in the file but not applied. The messages that appear when you select a cell are not shown.
