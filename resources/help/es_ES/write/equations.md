# Ecuaciones
<!-- keywords: pestaña ecuación, estructuras, símbolos, convertir, ecuación, ecuaciones, matemáticas, fórmula, fracción, raíz cuadrada, superíndice, subíndice, exponente, suma, integral, griego, símbolo, insertar ecuación, alt+=, acento, sombrero, vector, barra, matriz, corchetes, ecuaciones integradas, galería, cambiar una ecuación, editar una ecuación -->

Una **ecuación** es una línea de notación matemática, como una fracción o una raíz cuadrada. Este tema muestra cómo escribir una y cómo colocarla en un documento.

## Insertar una ecuación

1. Coloque el cursor donde debe ir la ecuación.
2. Haga clic en **Insertar > Ecuación**, o pulse **Alt+=**.
3. Escriba la ecuación en una sola línea. Debajo del cuadro verá la ecuación dibujada tal como quedará, y la línea de texto que mostrará la página. Para partir de una ecuación conocida, elija una de la lista **Ecuaciones integradas** del cuadro. Haga clic en **Aceptar**.

La ecuación se guarda en el documento con el formato estándar para las matemáticas, de modo que otros programas puedan abrirla.

## Cómo escribirla

- **Fracciones:** escriba una barra. `a/b` es a entre b. `(x+1)/(x-1)` toma cada grupo entre paréntesis como una sola parte. `\frac{1}{2}` también funciona.
- **Potencias:** escriba un acento circunflejo. `x^2` es x al cuadrado. `e^(i*pi)` toma el grupo entre paréntesis.
- **Subíndices:** escriba un guion bajo. `x_i` es x con una i pequeña debajo. Puede usar ambos, como en `x_i^2`.
- **Raíces:** escriba `sqrt(x+1)` para una raíz cuadrada. Escriba `sqrt[3]{x}` para una raíz cúbica.
- **Sumas, integrales y productos:** escriba `\sum_(i=1)^n i`, `\int_0^1 x` o `\prod_(k=1)^n k`. La parte después del guion bajo va debajo (o en la parte inferior del signo de integral), y la parte después del acento circunflejo va encima.
- **Símbolos:** escriba una barra invertida y un nombre. `\alpha` es α, `\pi` es π, `\infty` es ∞, `\leq` es ≤, `\neq` es ≠, `\times` es ×, `\pm` es ±, y `\to` es →. Funcionan unos 70 nombres.
- **Acentos:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` y `\vec{v}` colocan una marca sobre la letra. `\overline{ab}` dibuja una línea sobre el grupo, y `\underline{ab}` dibuja una debajo.
- **Corchetes que crecen:** escriba `\left( ... \right)`, y los corchetes crecerán hasta la altura de lo que hay entre ellos. Use cualquier corchete después de `\left` y `\right`, o un punto para omitir un lado, como en `\left[ x \right.`.
- **Matrices:** una **matriz** es una cuadrícula de números. Escriba `\matrix{a & b \\ c & d}`. Ponga un `&` entre las celdas de una fila, y dos barras invertidas entre las filas. Use `\pmatrix` para paréntesis alrededor de la cuadrícula, `\bmatrix` para corchetes y `\vmatrix` para líneas rectas.
- **Funciones:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` y `max` mantienen su nombre completo.

En la página, JeffWrite muestra una ecuación como una línea de texto. La ecuación del fichero conserva sus fracciones y raíces, de modo que otros programas la distribuyan correctamente.

## Escribirla en UnicodeMath o LaTeX

Junto al cuadro hay una lista de formatos de escritura: **Línea simple** (la de arriba), **UnicodeMath** y **LaTeX**. Son dos formas de escribir matemáticas como texto sin formato que muchas personas ya conocen. Al elegir otro formato, lo que escribió se vuelve a escribir en él.

- **UnicodeMath:** `a/b` es una fracción, `x^2_i` tiene una potencia y un subíndice, `√(x)` es una raíz, y `∑_(i=1)^n i` es una suma. Ponga las palabras entre comillas, como `"si x"`, y se mantendrán como texto normal en lugar de letras matemáticas. `(a|b)` crea paréntesis con dos partes, y `\matrix(a&b@c&d)` crea una matriz (`&` entre celdas, `@` entre filas).
- **LaTeX:** `\frac{a}{b}` es una fracción, `\sqrt[3]{x}` es una raíz cúbica, `\sum_{i=1}^n i` es una suma, `\text{si x}` es texto normal, y `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` es una matriz.

Los dos botones **Profesional** y **Lineal** cambian la forma en que el cuadro muestra la ecuación. Profesional la dibuja con sus elementos apilados, tal como quedará. Lineal muestra solo la línea que escribió. Si una ecuación no se puede escribir en el formato elegido, el cuadro se lo indica y conserva lo que escribió.

## La pestaña Ecuación

Seleccione una ecuación (haga clic en ella, o mueva el cursor sobre ella con Mayús), y aparecerá una pestaña llamada **Ecuación** al final de la cinta de opciones. Desaparece al seleccionar otra cosa.

- **Herramientas:** **Editar ecuación** abre el cuadro. **Profesional** y **Lineal** eligen cómo se abre el cuadro: con los elementos apilados o solo con la línea. El documento siempre muestra una ecuación como una línea de texto. **Texto normal** abre el cuadro con un par de comillas añadido, para palabras que no son matemáticas.
- **Convertir:** elija el formato en que se abre el cuadro: **Línea simple**, **UnicodeMath** o **LaTeX**. La próxima vez que edite una ecuación, se escribirá en el formato que eligió. Una ecuación que ninguna línea del tipo simple puede escribir, como una suma con sus límites encima y debajo, se abre en UnicodeMath.
- **Símbolos:** letras griegas, operadores, relaciones, flechas, y conjuntos y lógica. Elija un símbolo, y el cuadro se abre con este añadido a la línea.
- **Estructuras:** fracción, subíndice y superíndice, radical, integral, operador grande, corchete, función, acento, límite, operador y matriz. Haga clic en un botón para su forma habitual, o en la flecha junto a él para más formas, como una derivada, una raíz con índice o una matriz identidad.

Una estructura se inserta al final de la línea, con letras de ejemplo, y se selecciona la primera para que pueda escribir encima. El cuadro tiene las mismas estructuras en una fila de botones pequeños encima de la línea. Allí, una estructura se inserta en el cursor, y si hay una parte de la línea seleccionada, esa parte pasa al primer lugar de la estructura.

## Ecuaciones integradas

Haga clic en **Insertar > Ecuaciones integradas** para ver una lista de ecuaciones conocidas, como la fórmula cuadrática o el área de un círculo. Haga clic en una para insertarla en el cursor.

## Cambiar una ecuación

Coloque el cursor junto a una ecuación, o selecciónela, y haga clic en **Insertar > Ecuación**. El cuadro se abre con la ecuación escrita en una línea. Cámbiela y haga clic en **Aceptar**. Deshacer el cambio es un solo paso.

## Ecuaciones en otros documentos

JeffWrite muestra las ecuaciones que contiene un documento y las conserva tal como estaban. Una ecuación creada con otro programa se abre en UnicodeMath, o en LaTeX cuando UnicodeMath no puede escribirla, para que pueda modificarla. Si ninguno de los dos puede escribir todas sus partes, como los signos de agrupación, el cuadro empieza con el texto de la ecuación. Haga clic en **Cancelar** para conservar la ecuación tal como está, o escriba una nueva para reemplazarla.
