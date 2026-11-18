# Añadir una ecuación
<!-- keywords: pestaña ecuación, estructuras, símbolos, convertir, ecuación, ecuaciones, matemáticas, fórmula, fracción, raíz cuadrada, superíndice, subíndice, exponente, suma, integral, griego, símbolo, insertar ecuación, alt+=, acento, matriz, paréntesis, ecuaciones integradas, galería, cambiar una ecuación, editar una ecuación, vector, pdf, jpub, archivo pub -->

Una **ecuación** es una línea de matemáticas, como una fracción o una raíz cuadrada. En JeffPub, una ecuación es un objeto de la página, como una imagen: puede moverla, cambiarle el tamaño, girarla y darle un color.

## Colocar una ecuación en la página

1. Haga clic en **Insertar > Texto > Ecuación**, o presione **Alt+=**.
2. Escriba la ecuación en una sola línea. Debajo del cuadro verá cómo quedará. Haga clic en **Aceptar**.

La ecuación se coloca en el centro de la página, seleccionada. Para partir de una conocida, haga clic en la flecha junto a **Ecuación** y elija una ecuación integrada, como la fórmula cuadrática o el área de un círculo.

## Cómo escribirla

- **Fracciones:** escriba una barra inclinada. `a/b` es a sobre b. `(x+1)/(x-1)` toma cada grupo entre paréntesis como un único elemento.
- **Potencias y subíndices:** `x^2` es x al cuadrado, y `x_i` es x con una i pequeña debajo. Puede usar ambos, como en `x_i^2`.
- **Raíces:** escriba `sqrt(x+1)` para una raíz cuadrada, o `sqrt[3]{x}` para una raíz cúbica.
- **Sumas, integrales y productos:** escriba `\sum_(i=1)^n i`, `\int_0^1 x` o `\prod_(k=1)^n k`. La parte que va después del guion bajo se coloca debajo, y la que va después del acento circunflejo, encima.
- **Símbolos:** escriba una barra invertida y un nombre. `\alpha` es α, `\pi` es π, `\infty` es ∞, `\leq` es ≤ y `\times` es ×. Funcionan unos 70 nombres.
- **Acentos:** `\hat{x}`, `\vec{v}` y `\bar{x}` colocan una marca sobre la letra.
- **Paréntesis que crecen:** escriba `\left( ... \right)`, y los paréntesis crecerán hasta la altura de lo que hay entre ellos.
- **Matrices:** una **matriz** es una cuadrícula de números. Escriba `\pmatrix{a & b \\ c & d}`. Ponga un `&` entre las celdas de una fila, y dos barras invertidas entre las filas.

## Escribirla en UnicodeMath o LaTeX

Junto al cuadro hay una lista de formatos de escritura: **Línea simple** (la que se ha usado arriba), **UnicodeMath** y **LaTeX**. Son dos formas de escribir matemáticas como texto sin formato que muchas personas ya conocen. Al elegir otro formato, lo que escribió se vuelve a escribir en él.

- **UnicodeMath:** `a/b` es una fracción, `x^2_i` tiene una potencia y un subíndice, `√(x)` es una raíz y `∑_(i=1)^n i` es una suma. Ponga las palabras entre comillas, como `"si x"`, y se mantendrán como texto normal en lugar de como letras matemáticas. `(a|b)` crea paréntesis con dos partes, y `\matrix(a&b@c&d)` crea una matriz (`&` entre celdas, `@` entre filas).
- **LaTeX:** `\frac{a}{b}` es una fracción, `\sqrt[3]{x}` es una raíz cúbica, `\sum_{i=1}^n i` es una suma, `\text{si x}` es texto normal, y `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` es una matriz.

Los dos botones **Profesional** y **Lineal** cambian la forma en que el cuadro muestra la ecuación. Profesional la dibuja con sus elementos apilados, tal como quedará. Lineal muestra solo la línea que escribió. Si una ecuación no se puede escribir en el formato elegido, el cuadro se lo indica y conserva lo que escribió.

## Cambiar una ecuación

Seleccione la ecuación y presione Intro, o haga doble clic en ella, o haga clic en **Insertar > Texto > Ecuación**. El cuadro se abre con la ecuación escrita en una línea. Cámbiela y haga clic en **Aceptar**. Se deshace con un solo paso.

Una ecuación que proviene de un archivo se abre en UnicodeMath, o en LaTeX cuando UnicodeMath no puede escribirla. Si ninguno puede, el cuadro muestra el texto de la ecuación. Haga clic en **Aceptar** sin cambiar nada, o en **Cancelar**, para dejar la ecuación como está, o escriba una nueva para reemplazarla.

## La pestaña Ecuación

Al seleccionar una ecuación, aparece una pestaña llamada **Ecuación** al final de la cinta de opciones. Desaparece cuando selecciona otra cosa.

- **Herramientas:** **Editar ecuación** abre el cuadro. **Profesional** y **Lineal** cambian la forma en que se muestra en la página la ecuación seleccionada: Profesional la dibuja con sus elementos apilados, y Lineal muestra la línea en la que se escribió. La elección se guarda con la ecuación en el archivo `.jpub`, así que es la misma al volver a abrirlo. Un archivo `.pub` recibe la imagen de la ecuación tal como se muestra en la página. **Texto normal** abre el cuadro con un par de comillas añadido, para palabras que no son matemáticas.
- **Convertir:** elija el formato en que se abre el cuadro: **Línea simple**, **UnicodeMath** o **LaTeX**. La próxima vez que edite una ecuación, se escribirá en el formato elegido. Una ecuación que ninguna línea de tipo simple puede escribir, como una suma con sus límites arriba y abajo, se abre en UnicodeMath.
- **Símbolos:** letras griegas, operadores, relaciones, flechas, y conjuntos y lógica. Elija un símbolo, y el cuadro se abre con él añadido a la línea.
- **Estructuras:** fracción, superíndice y subíndice, radical, integral, operador grande, paréntesis, función, acento, límite, operador y matriz. Haga clic en un botón para su forma habitual, o en la flecha junto a él para más formas, como una derivada, una raíz con índice o una matriz identidad.

Una estructura se inserta al final de la línea, con letras de ejemplo, y la primera queda seleccionada para que pueda escribir encima. El cuadro tiene las mismas estructuras en una fila de botones pequeños sobre la línea. Allí, una estructura se inserta en el cursor, y si hay una parte de la línea seleccionada, esa parte pasa al primer lugar de la estructura.

## Mover, cambiar el tamaño y el color

Arrastre la ecuación para moverla. Arrastre una esquina para hacerla más grande o más pequeña; la ecuación mantiene sus proporciones. Use **Relleno de forma** para cambiar el color de las letras.

## Guardar

- **.jpub** conserva la ecuación, así que puede volver a editarla.
- **PDF e impresión** dibujan la ecuación como formas, así que se mantiene nítida a cualquier tamaño.
- **.pub** no admite ecuaciones. JeffPub guarda cada una como una imagen de ella, en el mismo lugar y tamaño, y se lo indica al guardar.
- **EPUB** y **HTML** incluyen la ecuación como la línea que escribió.
