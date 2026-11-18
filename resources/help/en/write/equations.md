# Equations
<!-- keywords: equation, equations, math, formula, fraction, square root, superscript, subscript, exponent, sum, integral, greek, symbol, insert equation, alt+=, accent, hat, vector, bar, matrix, brackets, built-in equations, gallery, change an equation, edit an equation -->

An **equation** is a line of math, such as a fraction or a square root. This topic shows how to type one and put it in a document.

## Put an equation in

1. Put the caret where the equation should go.
2. Click **Insert > Equation**, or press **Alt+=**.
3. Type the equation in one line. Under the box you see the equation drawn as it will look, and the line of text the page will show. To start from a well-known equation, pick one from the **Built-In Equations** list in the box. Click **OK**.

The equation is saved in the document in the standard form for math, so other programs can open it.

## How to type it

- **Fractions:** type a slash. `a/b` is a over b. `(x+1)/(x-1)` takes each group in brackets as one piece. `\frac{1}{2}` works too.
- **Powers:** type a caret. `x^2` is x squared. `e^(i*pi)` takes the group in brackets.
- **Subscripts:** type an underscore. `x_i` is x with a small i below. You can use both, as in `x_i^2`.
- **Roots:** type `sqrt(x+1)` for a square root. Type `sqrt[3]{x}` for a cube root.
- **Sums, integrals, and products:** type `\sum_(i=1)^n i`, `\int_0^1 x`, or `\prod_(k=1)^n k`. The part after the underscore sits below (or at the bottom of an integral sign), and the part after the caret sits above.
- **Symbols:** type a backslash and a name. `\alpha` is α, `\pi` is π, `\infty` is ∞, `\leq` is ≤, `\neq` is ≠, `\times` is ×, `\pm` is ±, and `\to` is →. About 70 names work.
- **Accents:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}`, and `\vec{v}` put a mark over the letter. `\overline{ab}` draws a line over the group, and `\underline{ab}` draws one under it.
- **Brackets that grow:** type `\left( ... \right)`, and the brackets grow to the height of what is between them. Use any bracket after `\left` and `\right`, or a dot to leave one side out, as in `\left[ x \right.`.
- **Matrices:** a **matrix** is a grid of numbers. Type `\matrix{a & b \\ c & d}`. Put an `&` between the cells of a row, and two backslashes between the rows. Use `\pmatrix` for round brackets round the grid, `\bmatrix` for square ones, and `\vmatrix` for straight lines.
- **Functions:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min`, and `max` keep their names whole.

In the page, JeffWrite shows an equation as one line of text. The equation in the file keeps its fractions and roots, so other programs lay it out.

## Type it in UnicodeMath or LaTeX

Next to the box is a list of typing formats: **Simple line** (the one above), **UnicodeMath**, and **LaTeX**. These are two ways of writing math as plain text that many people already know. When you pick another format, what you typed is written again in it.

- **UnicodeMath:** `a/b` is a fraction, `x^2_i` has a power and a subscript, `√(x)` is a root, and `∑_(i=1)^n i` is a sum. Put words in quotes, such as `"if x"`, and they stay as plain text instead of math letters. `(a|b)` makes brackets with two parts, and `\matrix(a&b@c&d)` makes a matrix (`&` between cells, `@` between rows).
- **LaTeX:** `\frac{a}{b}` is a fraction, `\sqrt[3]{x}` is a cube root, `\sum_{i=1}^n i` is a sum, `\text{if x}` is plain text, and `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` is a matrix.

The two buttons **Professional** and **Linear** change how the box shows your equation. Professional draws it built up, as it will look. Linear shows only the line you typed. If an equation can't be written in the format you pick, the box tells you and keeps what you typed.

## Built-in equations

Click **Insert > Built-In Equations** to see a list of well-known equations, such as the quadratic formula and the area of a circle. Click one to put it in at the caret.

## Change an equation

Put the caret next to an equation, or select it, and click **Insert > Equation**. The box opens with the equation written out in one line. Change it and click **OK**. It is one step to undo.

## Equations in other documents

JeffWrite shows the equations that a document holds, and keeps them as they were. It can change an equation that is made of the parts listed above. An equation with other parts, such as grouping marks, is kept as it is, and Insert > Equation puts in a new one instead of changing it.
