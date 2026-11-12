# Agregar una ecuación
<!-- keywords: ecuación, ecuaciones, matemáticas, fórmula, fracción, raíz cuadrada, superíndice, subíndice, exponente, suma, integral, griega, letra griega, símbolo, insertar ecuación, alt+=, acento, matriz, corchetes, ecuaciones integradas, galería, cambiar una ecuación, editar una ecuación, vector, pdf, jpub, archivo pub, fichero pub -->

Una **ecuación** es una línea de matemáticas, como una fracción o una raíz cuadrada. En JeffPub, una ecuación es un objeto de la página, como una imagen: puede moverla, cambiarle el tamaño, girarla y darle un color.

## Colocar una ecuación en la página

1. Haga clic en **Insertar > Texto > Ecuación**, o pulse **Alt+=**.
2. Escriba la ecuación en una sola línea. Debajo del cuadro verá cómo quedará. Haga clic en **Aceptar**.

La ecuación se coloca en el centro de la página, seleccionada. Para partir de una conocida, haga clic en la flecha junto a **Ecuación** y elija una ecuación integrada, como la fórmula cuadrática o el área de un círculo.

## Cómo escribirla

- **Fracciones:** escriba una barra inclinada. `a/b` es a sobre b. `(x+1)/(x-1)` toma cada grupo entre paréntesis como una sola parte.
- **Potencias y subíndices:** `x^2` es x al cuadrado, y `x_i` es x con una i pequeña debajo. Puede usar ambos, como en `x_i^2`.
- **Raíces:** escriba `sqrt(x+1)` para una raíz cuadrada, o `sqrt[3]{x}` para una raíz cúbica.
- **Sumas, integrales y productos:** escriba `\sum_(i=1)^n i`, `\int_0^1 x` o `\prod_(k=1)^n k`. La parte que va después del guion bajo se coloca debajo, y la que va después del acento circunflejo, encima.
- **Símbolos:** escriba una barra invertida y un nombre. `\alpha` es α, `\pi` es π, `\infty` es ∞, `\leq` es ≤, y `\times` es ×. Funcionan unos 70 nombres.
- **Acentos:** `\hat{x}`, `\vec{v}` y `\bar{x}` colocan una marca sobre la letra.
- **Corchetes que crecen:** escriba `\left( ... \right)`, y los corchetes crecerán hasta la altura de lo que hay entre ellos.
- **Matrices:** una **matriz** es una cuadrícula de números. Escriba `\pmatrix{a & b \\ c & d}`. Ponga un `&` entre las celdas de una fila, y dos barras invertidas entre las filas.

## Cambiar una ecuación

Seleccione la ecuación y pulse Entrar, o haga doble clic en ella, o haga clic en **Insertar > Texto > Ecuación**. El cuadro se abre con la ecuación escrita en una sola línea. Modifíquela y haga clic en **Aceptar**. Basta un paso para deshacer el cambio.

Si un fichero trae una ecuación con partes que el cuadro no puede escribir, se muestra tal como está. Para cambiarla, escriba una nueva.

## Mover, cambiar el tamaño y el color

Arrastre la ecuación para moverla. Arrastre una esquina para agrandarla o reducirla; la ecuación conserva sus proporciones. Use **Relleno de forma** para cambiar el color de las letras.

## Guardar

- **.jpub** conserva la ecuación, de modo que puede editarla de nuevo.
- **PDF e impresión** dibujan la ecuación como formas, de modo que se ve nítida a cualquier tamaño.
- **.pub** no admite ecuaciones. JeffPub guarda cada una como una imagen suya, en la misma posición y tamaño, y se lo indica al guardar.
- **EPUB** y **HTML** incluyen la ecuación como la línea que escribió.
