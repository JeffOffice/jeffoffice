# Ecuaciones
<!-- keywords: ficha ecuación, estructuras, símbolos, convertir, ecuación, ecuaciones, matemáticas, matemática, fórmula, fórmulas, fracción, raíz cuadrada, superíndice, subíndice, exponente, suma, integral, griego, símbolo, insertar ecuación, acento, matriz, corchetes, ecuaciones integradas, galería, cambiar una ecuación, editar una ecuación, ods, imagen, pdf -->

Una **ecuación** es una línea de matemáticas, como una fracción o una raíz cuadrada. En JeffCrunch, una ecuación es un objeto sobre las celdas, como una imagen o un cuadro de texto. No es una celda: no cambia ningún número de la hoja.

## Colocar una ecuación en la hoja

1. Haga clic en **Insertar > Símbolos > Ecuación**.
2. Escriba la ecuación en una sola línea. Debajo del cuadro verá cómo quedará. Haga clic en **Aceptar**.

La ecuación se inserta en la celda en la que se encuentra, seleccionada. Para empezar desde una conocida, haga clic en **Ecuaciones integradas** y elija una, como la fórmula cuadrática o el área de un círculo.

## Cómo escribirla

- **Fracciones:** escriba una barra. `a/b` es a partido por b. `(x+1)/(x-1)` toma cada grupo entre paréntesis como una sola parte.
- **Potencias y subíndices:** `x^2` es x al cuadrado, y `x_i` es x con una i pequeña debajo.
- **Raíces:** escriba `sqrt(x+1)` para una raíz cuadrada.
- **Sumas, integrales y productos:** escriba `\sum_(i=1)^n i`, `\int_0^1 x` o `\prod_(k=1)^n k`.
- **Símbolos:** escriba una barra invertida y un nombre. `\alpha` es α, `\pi` es π, `\infty` es ∞, y `\leq` es ≤.
- **Acentos, corchetes y matrices:** `\hat{x}`, `\left( ... \right)` y `\pmatrix{a & b \\ c & d}`.

## Escribirla en UnicodeMath o LaTeX

Junto al cuadro hay una lista de formatos de escritura: **Línea simple** (la de arriba), **UnicodeMath** y **LaTeX**. Son dos formas de escribir matemáticas como texto sin formato que muchas personas ya conocen. Al elegir otro formato, lo que escribió se vuelve a escribir en él.

- **UnicodeMath:** `a/b` es una fracción, `x^2_i` tiene una potencia y un subíndice, `√(x)` es una raíz, y `∑_(i=1)^n i` es una suma. Ponga las palabras entre comillas, como `"si x"`, y se mantendrán como texto sin formato en lugar de letras matemáticas. `(a|b)` crea corchetes con dos partes, y `\matrix(a&b@c&d)` crea una matriz (`&` entre celdas, `@` entre filas).
- **LaTeX:** `\frac{a}{b}` es una fracción, `\sqrt[3]{x}` es una raíz cúbica, `\sum_{i=1}^n i` es una suma, `\text{si x}` es texto sin formato, y `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` es una matriz.

Los dos botones **Profesional** y **Lineal** cambian la forma en que el cuadro muestra su ecuación. Profesional la dibuja con sus elementos apilados, tal como quedará. Lineal muestra solo la línea que escribió. Si una ecuación no se puede escribir en el formato elegido, el cuadro se lo indica y conserva lo que escribió.

## La ficha Ecuación

Al seleccionar una ecuación, aparece una ficha llamada **Ecuación** al final de la cinta de opciones. Desaparece al seleccionar otra cosa.

- **Herramientas:** **Editar ecuación** abre el cuadro. **Profesional** y **Lineal** eligen cómo se abre el cuadro: con los elementos apilados o solo la línea. **Texto normal** abre el cuadro con un par de comillas añadido, para palabras que no son matemáticas.
- **Convertir:** elija el formato en el que se abre el cuadro: **Línea simple**, **UnicodeMath** o **LaTeX**. La próxima vez que edite una ecuación, se escribirá en el formato que haya elegido. Una ecuación que ninguna línea del tipo simple puede escribir, como una suma con sus límites arriba y abajo, se abre en UnicodeMath.
- **Símbolos:** letras griegas, operadores, relaciones, flechas, conjuntos y lógica. Elija un símbolo, y el cuadro se abre con este añadido a la línea.
- **Estructuras:** fracción, subíndice y superíndice, radical, integral, operador grande, corchete, función, acento, límite, operador y matriz. Haga clic en un botón para su forma habitual, o en la flecha junto a él para ver más formas, como una derivada, una raíz con índice o una matriz identidad.

Una estructura se inserta al final de la línea, con letras de ejemplo, y la primera queda seleccionada para que pueda escribir encima. El cuadro tiene las mismas estructuras en una fila de botones pequeños encima de la línea. Allí, una estructura se inserta en el cursor, y si parte de la línea está seleccionada, esa parte pasa a ocupar el primer lugar de la estructura.

## Cambiar, mover y cambiar el tamaño

Haga doble clic en la ecuación, o selecciónela y haga clic en **Insertar > Símbolos > Ecuación**, para abrir el cuadro con su línea. Modifíquela y haga clic en **Aceptar**. Se puede deshacer en un solo paso. Arrastre la ecuación para moverla, y arrastre un controlador para cambiar su tamaño; la ecuación mantiene sus proporciones dentro del cuadro.

## Guardar

- **.xlsx** conserva la ecuación como ecuación, en el formato estándar de las matemáticas, con una imagen al lado para los programas que no pueden leer matemáticas. Al volver a abrir el fichero, la ecuación se muestra. Una ecuación creada en otro programa se muestra de la misma forma; si sus matemáticas no se pueden leer, verá su imagen.
- **PDF e impresión** dibujan la ecuación como formas, por lo que se mantiene nítida a cualquier tamaño.
- **Las copias .ods** no tienen ecuaciones. JeffCrunch guarda cada una como una imagen suya, en el mismo lugar, y nombra las ecuaciones al indicarle lo que la copia no conserva.
