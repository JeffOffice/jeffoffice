# Equations
<!-- keywords: equation tab, structures, symbols, convert, equation, equations, math, formula, fraction, square root, superscript, subscript, exponent, sum, integral, greek, symbol, insert equation, accent, matrix, brackets, built-in equations, gallery, change an equation, edit an equation, pptx, odp, picture, pdf, slide show, print -->

An **equation** is a line of math, such as a fraction or a square root. On a slide an equation is an object, like a picture or a text box. You can move it, resize it, turn it, copy it, and delete it.

## Put an equation on the slide

1. Click **Insert > Symbols > Equation** (or press **Alt+=**).
2. Type the equation in one line. Under the box you see how it will look. Click **OK**.

The equation goes in the middle of the slide, selected. Its letters are about as tall as a heading. One that is very wide is made smaller so it fits on the slide. To start from a well-known one, click **Built-In Equations** and pick one, such as the quadratic formula or the area of a circle.

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

## Change, move, and size

Double-click the equation, or select it and click **Insert > Symbols > Equation**, to open the box with its line. Change it and click **OK**. It stays where it is and keeps its size, and the change is one step to undo. Drag the equation to move it. Drag a handle to size it; the equation keeps its proportions inside its box.

An equation that came from a file can be changed too. Until you change it, JeffDeck shows the picture the file keeps and saves it back exactly as it was. Double-click it (or select it and click **Equation**). The box starts with a line that says what the equation is. If you change the line, the picture becomes an equation, in the same place, and JeffDeck sets the math again from what you typed, so any special fonts in the old math are gone. If you click **OK** without changing the line, nothing changes.

## The Equation tab

Select an equation, and a tab named **Equation** appears at the end of the ribbon. It goes away when you select something else.

- **Tools:** **Edit Equation** opens the box. **Professional** and **Linear** change how the selected equation is shown on the slide: Professional draws it built up, and Linear shows the line it is typed in. The choice stays with the equation while the presentation is open, and a saved `.pptx` always has the equation built up. **Normal Text** opens the box with a pair of quotes added, for words that are not math.
- **Convert:** pick the format the box opens in: **Simple Line**, **UnicodeMath**, or **LaTeX**. The next time you edit an equation, it is written in the format you picked. An equation that no line of the simple kind can write, such as a sum with its limits above and below, opens in UnicodeMath instead.
- **Symbols:** Greek letters, operators, relations, arrows, and sets and logic. Pick a symbol, and the box opens with it added to the line.
- **Structures:** fraction, script, radical, integral, large operator, bracket, function, accent, limit, operator, and matrix. Click a button for its usual form, or the arrow beside it for more forms, such as a derivative, a root with a degree, or a 3 by 3 matrix.

A structure goes in at the end of the line, with sample letters in it, and the first one is selected so you can type over it. The box has the same structures in a row of small buttons above the line. There a structure goes in at the cursor, and if part of the line is selected, that part goes into the first place of the structure.

## Slide show, printing, and PDF

The slide show, printing, and PDF files draw the equation as shapes, so it stays sharp at any size.

## Saving

- **.pptx** keeps the equation as an equation, in the standard form for math, with a picture of it beside it for programs that can't read math. When you open the file again, JeffDeck shows the picture and keeps the math, and you can still change it as described above. A presentation holds at most 2,000 equations and 16 MB of math in all. Past that, JeffDeck won't add another.
- **.odp** has no equations in JeffDeck's copy. Each one is saved as a picture of it, in the same place, and the message after the save says so.
- A text box with words around its math is not an equation here. It is shown as the picture the file keeps and saved back whole.

## Related

- [Shapes, pictures, tables, and Text Art](objects)
- [Insert tab](tab-insert)
