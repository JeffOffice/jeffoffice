# Enter a function with the Function Arguments dialog
<!-- keywords: function arguments, argument, fx, formula result, Shift+F3, pick cells, range, optional, repeat, edit a function, nested function, error -->

The **Function Arguments** dialog helps you fill in a function one argument at a time and shows what the formula comes to before you put it in the cell.

## Open it

- Choose **Pick a Function** on the Formulas tab (**Shift+F3**), or click **fx** beside the formula bar, then pick a function. The dialog opens for it.
- To change a function that is already in a formula, put the cursor inside its brackets (in the cell or in the formula bar) and press **Shift+F3** or click **fx**. The dialog opens for that function, with its arguments in the boxes. In a formula such as =SUM(A1, IF(B1>2, 5, 6)), a cursor inside the IF changes the IF, and a cursor in the SUM but outside the IF changes the SUM.

## Fill it in

Each argument has a row: its name, a box to type it in, and, to the right, what it comes to as it stands, such as 12.5, "text", TRUE, or {1,2,3} for a block of cells. A name in bold must be filled in; a name that is not bold is optional and can be left empty.

- **Tab** moves to the next box, and **Shift+Tab** to the one before.
- The line under the boxes says what the function does, and the line after it says what the argument you are in is.
- Functions such as SUM take as many arguments as you need: when you type in the last box, a new one appears.
- The button at the right of a box lets you point at cells. The dialog gives way to a small strip, and you select cells on the sheet with the mouse (the box shows their address as you drag). Click the button on the strip, or press **Enter**, to come back.

## The result

**Formula result =** shows what the whole function comes to, a moment after you stop typing. Nothing is written to the sheet until you press **OK**. If something is wrong, a line under the result says so in words: for example that a value was not found, that something is divided by zero, or that a number is not allowed. A formula that would take too long to work out here is not shown; press **OK** and the sheet works it out.

**Help on this function** opens this page.

## Finish

Press **Enter** or click **OK** to put the function in. In a cell with no entry going, the formula is written into the cell. In a formula you are typing, the function goes in at the cursor, or takes the place of the one you changed, and you can go on typing. Press **Esc** or click **Cancel** and nothing changes.
