# Equations
<!-- keywords: equation tab, structures, symbols, convert, equation, equations, math, formula, fraction, square root, superscript, subscript, exponent, sum, integral, greek, symbol, insert equation, accent, matrix, brackets, built-in equations, gallery, change an equation, edit an equation, ods, picture, pdf -->

An **equation** is a line of math, such as a fraction or a square root. In JeffCrunch an equation is an object over the cells, like a picture or a text box. It is not a cell: it does not change any number on the sheet.

## Put an equation on the sheet

1. Click **Insert > Symbols > Equation**.
2. Type the equation in one line. Under the box you see how it will look. Click **OK**.

The equation is put at the cell you are in, selected. To start from a well-known one, click **Built-In Equations** and pick one, such as the quadratic formula or the area of a circle.

## How to type it

- **Fractions:** type a slash. `a/b` is a over b. `(x+1)/(x-1)` takes each group in brackets as one piece.
- **Powers and subscripts:** `x^2` is x squared, and `x_i` is x with a small i below.
- **Roots:** type `sqrt(x+1)` for a square root.
- **Sums, integrals, and products:** type `\sum_(i=1)^n i`, `\int_0^1 x`, or `\prod_(k=1)^n k`.
- **Symbols:** type a backslash and a name. `\alpha` is α, `\pi` is π, `\infty` is ∞, and `\leq` is ≤.
- **Accents, brackets, and matrices:** `\hat{x}`, `\left( ... \right)`, and `\pmatrix{a & b \\ c & d}`.

## Type it in UnicodeMath or LaTeX

Next to the box is a list of typing formats: **Simple line** (the one above), **UnicodeMath**, and **LaTeX**. These are two ways of writing math as plain text that many people already know. When you pick another format, what you typed is written again in it.

- **UnicodeMath:** `a/b` is a fraction, `x^2_i` has a power and a subscript, `√(x)` is a root, and `∑_(i=1)^n i` is a sum. Put words in quotes, such as `"if x"`, and they stay as plain text instead of math letters. `(a|b)` makes brackets with two parts, and `\matrix(a&b@c&d)` makes a matrix (`&` between cells, `@` between rows).
- **LaTeX:** `\frac{a}{b}` is a fraction, `\sqrt[3]{x}` is a cube root, `\sum_{i=1}^n i` is a sum, `\text{if x}` is plain text, and `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` is a matrix.

The two buttons **Professional** and **Linear** change how the box shows your equation. Professional draws it built up, as it will look. Linear shows only the line you typed. If an equation can't be written in the format you pick, the box tells you and keeps what you typed.

## The Equation tab

Pick an equation, and a tab named **Equation** appears at the end of the ribbon. It goes away when you select something else.

- **Tools:** **Edit Equation** opens the box. **Professional** and **Linear** choose how the box starts, built up or as the line only. **Normal Text** opens the box with a pair of quotes added, for words that are not math.
- **Convert:** pick the format the box opens in: **Simple Line**, **UnicodeMath**, or **LaTeX**. The next time you edit an equation, it is written in the format you picked. An equation that no line of the simple kind can write, such as a sum with its limits above and below, opens in UnicodeMath instead.
- **Symbols:** Greek letters, operators, relations, arrows, and sets and logic. Pick a symbol, and the box opens with it added to the line.
- **Structures:** fraction, script, radical, integral, large operator, bracket, function, accent, limit, operator, and matrix. Click a button for its usual form, or the arrow beside it for more forms, such as a derivative, a root with a degree, or a 3 by 3 matrix.

A structure goes in at the end of the line, with sample letters in it, and the first one is selected so you can type over it. The box has the same structures in a row of small buttons above the line. There a structure goes in at the cursor, and if part of the line is selected, that part goes into the first place of the structure.

## Change, move, and size

Double-click the equation, or select it and click **Insert > Symbols > Equation**, to open the box with its line. Change it and click **OK**. It is one step to undo. Drag the equation to move it, and drag a handle to size it; the equation keeps its proportions inside the box.

## Saving

- **.xlsx** keeps the equation as an equation, in the standard form for math, with a picture beside it for programs that can't read math. Opening the file again shows the equation. An equation made in another program is shown the same way; if its math can't be read, you see its picture.
- **PDF and printing** draw the equation as shapes, so it stays sharp at any size.
- **.ods** copies have no equations. JeffCrunch saves each one as a picture of it, in the same place, and names the equations when it tells you what the copy leaves out.
