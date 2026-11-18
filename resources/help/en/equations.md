# Add an equation
<!-- keywords: equation tab, structures, symbols, convert, equation, equations, math, formula, fraction, square root, superscript, subscript, exponent, sum, integral, greek, symbol, insert equation, alt+=, accent, matrix, brackets, built-in equations, gallery, change an equation, edit an equation, vector, pdf, jpub, pub file -->

An **equation** is a line of math, such as a fraction or a square root. In JeffPub an equation is an object on the page, like a picture: you can move it, size it, turn it, and give it a color.

## Put an equation on the page

1. Click **Insert > Text > Equation**, or press **Alt+=**.
2. Type the equation in one line. Under the box you see how it will look. Click **OK**.

The equation goes in the middle of the page, selected. To start from a well-known one, click the arrow beside **Equation** and pick a built-in equation, such as the quadratic formula or the area of a circle.

## How to type it

- **Fractions:** type a slash. `a/b` is a over b. `(x+1)/(x-1)` takes each group in brackets as one piece.
- **Powers and subscripts:** `x^2` is x squared, and `x_i` is x with a small i below. You can use both, as in `x_i^2`.
- **Roots:** type `sqrt(x+1)` for a square root, or `sqrt[3]{x}` for a cube root.
- **Sums, integrals, and products:** type `\sum_(i=1)^n i`, `\int_0^1 x`, or `\prod_(k=1)^n k`. The part after the underscore goes below, and the part after the caret goes above.
- **Symbols:** type a backslash and a name. `\alpha` is α, `\pi` is π, `\infty` is ∞, `\leq` is ≤, and `\times` is ×. About 70 names work.
- **Accents:** `\hat{x}`, `\vec{v}`, and `\bar{x}` put a mark over the letter.
- **Brackets that grow:** type `\left( ... \right)`, and the brackets grow to the height of what is between them.
- **Matrices:** a **matrix** is a grid of numbers. Type `\pmatrix{a & b \\ c & d}`. Put an `&` between the cells of a row, and two backslashes between the rows.

## Type it in UnicodeMath or LaTeX

Next to the box is a list of typing formats: **Simple line** (the one above), **UnicodeMath**, and **LaTeX**. These are two ways of writing math as plain text that many people already know. When you pick another format, what you typed is written again in it.

- **UnicodeMath:** `a/b` is a fraction, `x^2_i` has a power and a subscript, `√(x)` is a root, and `∑_(i=1)^n i` is a sum. Put words in quotes, such as `"if x"`, and they stay as plain text instead of math letters. `(a|b)` makes brackets with two parts, and `\matrix(a&b@c&d)` makes a matrix (`&` between cells, `@` between rows).
- **LaTeX:** `\frac{a}{b}` is a fraction, `\sqrt[3]{x}` is a cube root, `\sum_{i=1}^n i` is a sum, `\text{if x}` is plain text, and `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` is a matrix.

The two buttons **Professional** and **Linear** change how the box shows your equation. Professional draws it built up, as it will look. Linear shows only the line you typed. If an equation can't be written in the format you pick, the box tells you and keeps what you typed.

## Change an equation

Select the equation and press Enter, or double-click it, or click **Insert > Text > Equation**. The box opens with the equation written out in one line. Change it and click **OK**. It is one step to undo.

An equation that a file brought opens in UnicodeMath, or in LaTeX when UnicodeMath can't write it. If neither can, the box starts with the text of the equation. Click **OK** with no change, or **Cancel**, to leave the equation as it is, or type a new one to replace it.

## The Equation tab

Select an equation, and a tab named **Equation** appears at the end of the ribbon. It goes away when you select something else.

- **Tools:** **Edit Equation** opens the box. **Professional** and **Linear** change how the selected equation is shown on the page: Professional draws it built up, and Linear shows the line it is typed in. The choice is saved with the equation, so it is the same when you open the file again. **Normal Text** opens the box with a pair of quotes added, for words that are not math.
- **Convert:** pick the format the box opens in: **Simple Line**, **UnicodeMath**, or **LaTeX**. The next time you edit an equation, it is written in the format you picked. An equation that no line of the simple kind can write, such as a sum with its limits above and below, opens in UnicodeMath instead.
- **Symbols:** Greek letters, operators, relations, arrows, and sets and logic. Pick a symbol, and the box opens with it added to the line.
- **Structures:** fraction, script, radical, integral, large operator, bracket, function, accent, limit, operator, and matrix. Click a button for its usual form, or the arrow beside it for more forms, such as a derivative, a root with a degree, or a 3 by 3 matrix.

A structure goes in at the end of the line, with sample letters in it, and the first one is selected so you can type over it. The box has the same structures in a row of small buttons above the line. There a structure goes in at the cursor, and if part of the line is selected, that part goes into the first place of the structure.

## Move, size, and color

Drag the equation to move it. Drag a corner to make it larger or smaller; the equation keeps its proportions. Use **Shape Fill** to change the color of the letters.

## Saving

- **.jpub** keeps the equation, so you can edit it again.
- **PDF and printing** draw the equation as shapes, so it stays sharp at any size.
- **.pub** has no equations. JeffPub saves each one as a picture of it, in the same place and size, and tells you so when you save.
- **EPUB** and **HTML** carry the equation as the line you typed.
