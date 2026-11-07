# Cómo se muestran y calculan las fórmulas
<!-- keywords: error siguiente, error anterior, recorrer errores, rastrear error, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, función, funciones, valor, resultado, almacenado, F9, circular, mostrar fórmulas, barra de fórmulas, error, señalar, hacer clic en una celda, referencia, F4, comprobación de errores, ignorar error, evaluar fórmula, paso, ventana inspección, inspección, LET, nombrar valor, CELL, INFO, FILTERXML, xml, xpath-->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, como =SUM(B3:B8). La celda muestra el valor, y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con ellas

Cuando abre un libro, el archivo guarda el último valor de cada fórmula, de modo que la hoja se puede mostrar de inmediato. Después, JeffCrunch vuelve a calcular cada fórmula, unas pocas cada vez, para que la ventana siga siendo rápida. Mientras lo hace, la barra de estado muestra **Calculando** con un porcentaje. Cuando termina, las celdas contienen los resultados de JeffCrunch. Para volver a calcularlo todo, haga clic en **Recalcular todo** en la ficha **Fórmulas**, o pulse **F9**.

## Cuándo JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 400 funciones, y sigue creciendo. Una fórmula que usa una función que JeffCrunch todavía no tiene conserva el valor que el archivo guardó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de estas se calculan a partir de los valores guardados.

## Funciones que necesitan algunas notas

- **Bonos y préstamos.** PRICE, YIELD, DURATION, ODDFPRICE y las demás funciones de bonos cuentan los días según la **base** que indique (0 son 30 días por mes en un año de 360 días; 1 son los días reales, y así sucesivamente). YIELD, ODDFYIELD, ODDLYIELD y RATE calculan su resultado probando valores hasta que el precio o el pago coinciden. Se detienen cuando el resultado es correcto hasta unos 15 dígitos, así que las últimas cifras pueden diferir de las de otro programa.
- **Distribuciones y pruebas.** Las funciones que terminan en .DIST, .INV, .TEST, y los nombres anteriores como NORMDIST y TDIST, dan los mismos resultados: el nombre anterior se conserva para que los archivos antiguos sigan funcionando. Las funciones *.INV también se calculan probando valores.
- **Listas que ocupan varias celdas.** SORT, SORTBY, FILTER, UNIQUE, SEQUENCE, RANDARRAY, XLOOKUP, MMULT, MINVERSE y LINEST dan un bloque de valores. Introdúzcalas como fórmula matricial (**Ctrl+Mayús+Intro**) sobre las celdas que deben llenar. Todavía no se admite que una fórmula se extienda a las celdas contiguas.
- **Textos en otros alfabetos.** LENB, LEFTB, MIDB, RIGHTB, FINDB, SEARCHB y REPLACEB cuentan cada carácter como uno, como ocurre en un equipo que no está configurado para texto de doble byte. PHONETIC devuelve el texto de la celda, porque JeffCrunch no guarda las lecturas del texto en japonés.
- **CONVERT.** Las unidades se escriben como texto, con mayúsculas y minúsculas tal como aparecen en la lista de la función: "kg", "lbm", "mi", "F". Un prefijo métrico puede ir delante de una unidad que lo admita, como "km" o "mL".

## MOD: el resto

**MOD(n; d)** da lo que queda de **n** después de quitar la mayor cantidad posible de copias enteras de **d**: es n - d * INT(n / d). Por ejemplo, **=MOD(10; 3)** es 1, y **=MOD(-10; 3)** es 2, porque INT(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la ficha **Fórmulas**, o pulse **Ctrl+`** (en un Mac, la tecla Control, no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas; vuelven a su anchura al desactivar la opción, y las anchuras de su archivo no se modifican.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, pulse **F2**, edítela y pulse **Intro**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Consulte [Escribir en celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparece una lista corta con las funciones y los [nombres](names) que empiezan así, con una frase sobre cada una. Pulse **Tabulador** para elegir la que está marcada. Use las flechas **Abajo** y **Arriba** para marcar otra, y luego **Tabulador** o **Intro**. **Esc** cierra la lista. Una vez dentro de los paréntesis de una función, sus argumentos aparecen debajo de la celda, y el que está escribiendo aparece en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con punto y coma, o con comas cuando su idioma usa la coma como separador decimal.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde pueda ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de una coma) y haga clic en la celda. Su dirección se escribe automáticamente. Arrastre sobre las celdas para escribir un rango, como B2:D5. Haga clic de nuevo antes de escribir cualquier otra cosa, y la nueva celda sustituye a la que se acaba de escribir. Mantenga pulsado **Ctrl** y haga clic para añadir otra dirección después de una coma. Haga clic en la letra de una columna para escribir una columna completa, como A:A, o en el número de una fila para escribir una fila completa, como 3:3, y arrastre por las letras o los números para escribir B:D o 2:5.

Las teclas de flecha también funcionan. Pulse una flecha y un puntero se aleja de la celda en la que escribe, y su dirección se escribe; pulse más flechas para seguir avanzando, o mantenga pulsado **Mayús** para extender el puntero sobre un rango. La barra de estado muestra **Señalar** mientras las flechas señalan, **Introducir** mientras escribe, y **Edición** mientras las flechas mueven el cursor dentro del texto. Pulse **F2** para cambiar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor está donde puede ir una celda, y después haga clic en la celda. El nombre de la hoja se escribe junto con ella, entre comillas cuando es necesario, como en 'Mi hoja'!B3. **Intro** le lleva de vuelta a la hoja en la que empezó y coloca la fórmula en su celda. **Esc** cancela todo, incluido el cambio de hoja.

También puede arrastrar el borde de un rango con contorno de la hoja para mover esa dirección, o arrastrar su cuadrado de esquina para estirarlo; la fórmula cambia mientras arrastra.

Pulse **F4** con el cursor dentro de una dirección o justo detrás de ella para fijar partes de la dirección con el signo de dólar: A1 pasa a ser $A$1, luego A$1, después $A1 y, finalmente, otra vez A1.

Cada dirección de la fórmula se colorea en el texto, y las celdas a las que hace referencia se marcan con un contorno del mismo color en la hoja, de modo que puede ver de un vistazo qué lee la fórmula. Un clic después de una dirección completa, como en =A1, termina la entrada y selecciona la celda en la que hizo clic, igual que con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de análisis de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Al abrir un libro que contiene una, JeffCrunch calcula sus celdas: en una tabla con dos entradas, la fórmula va en la celda de la esquina superior izquierda de la tabla, los valores que se prueban en horizontal se escriben en la celda de entrada de fila, y los que van en vertical, en la celda de entrada de columna. En una tabla de una sola entrada, los valores van en la columna de su izquierda (y las fórmulas están en la fila de encima) o en la fila de encima (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de cada fórmula que depende de la celda de entrada, con la entrada fijada a ese valor; no se modifica nada más del libro. En modo **Automático** las tablas se calculan junto con las fórmulas, y las fórmulas que leen las celdas de una tabla se actualizan. Todavía no se ofrece crear una tabla de datos nueva. Una tabla que no se puede calcular (una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como RAND o NOW) conserva los valores que contiene su archivo.

## Calcular manualmente

Haga clic en **Modo de cálculo** en la ficha **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula al cambiar las celdas.
- **Automático, excepto tablas** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que pulse **F9**.
- **Manual** no calcula nada, tampoco las tablas de datos, hasta que usted lo pida. La barra de estado muestra **Desactualizado** después de un cambio. Pulse **F9** (**Recalcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que está, junto con lo que estuviera pendiente. Un libro configurado en Manual se abre con los resultados que contiene su archivo. **Calcular antes de guardar**, también en el menú **Modo de cálculo**, está marcado de forma predeterminada: al guardar un libro en Manual que está desactualizado, primero se calculan las fórmulas, de modo que el archivo contenga números actualizados. Desactívelo para guardar los números tal como están.

La elección se guarda con el libro, y es un paso que puede deshacer.

## Fórmulas que se leen a sí mismas

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no se puede calcular de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las marca con un contorno en la hoja; estas conservan los valores que tenían. Haga clic en **Referencias circulares** en la ficha **Fórmulas** para ver la lista, y haga doble clic en una celda de ella para ir allí.

Algunos modelos necesitan un ciclo así a propósito. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se calculan una y otra vez, cada una leyendo los valores más recientes de las demás, hasta que ningún valor cambie más que la cantidad que indique o se agote el número de iteraciones permitido.

## Ver qué lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso más. Una pequeña hoja representa una celda de otra hoja. Se dibujan como máximo 2000 flechas, y la barra de estado indica cuando hay más. **Quitar flechas** las borra, y cualquier cambio en el libro también las borra.

Cuando una celda muestra un error, como #DIV/0!, selecciónela y haga clic en **Rastrear error**. Se dibujan flechas hacia las celdas que lee y que también muestran un error. Haga clic de nuevo para retroceder un paso más. Cuando ninguna de las celdas que lee muestra un error, la barra de estado indica que esta celda es la que produce el error, y esa es la que debe corregir.

Para revisar todos los errores de la hoja, haga clic en **Error siguiente** o en **Error anterior**. Las celdas que muestran un error se visitan una fila tras otra, y después de la última se vuelve a empezar por la primera. Cada celda se selecciona, y las flechas se dibujan hasta la celda que produce el error. La barra de estado indica en qué error está y cuántos hay en total.

## Comprobación de errores

Elija **Comprobación de errores** en el grupo **Comprobar** de la ficha Fórmulas para revisar la hoja en busca de celdas que puedan estar mal. Busca dos cosas: una fórmula que da un error (como #DIV/0!), y un número que está guardado como texto, que las fórmulas pueden pasar por alto.

El cuadro de diálogo muestra una celda cada vez, va a esa celda en la hoja y explica en una frase qué ocurre. **Siguiente** y **Anterior** se mueven entre las celdas. Un pequeño triángulo verde en la esquina superior izquierda de una celda la marca en la hoja. **Ignorar error** quita la celda de la lista, le quita el triángulo y se guarda en el archivo. Un paso de Deshacer la devuelve a la lista. Si no hay nada que corregir, se le indica.

## Evaluar fórmula

Use esta función para ver cómo calcula una fórmula su valor. Seleccione una celda que contenga una fórmula y elija **Evaluar fórmula**. La fórmula se muestra con la siguiente parte subrayada. Pulse **Evaluar** para calcular esa parte; su valor ocupa su lugar. Las partes se evalúan en este orden: primero cada celda individual de izquierda a derecha, después cada función con sus paréntesis, empezando por la más interna, y por último lo que queda. **Reiniciar** vuelve a empezar. Si una parte no se puede calcular, el cuadro de diálogo indica el motivo y deja la fórmula como estaba.

## Ventana Inspección

La **Ventana Inspección** es una ventana pequeña que permanece visible mientras trabaja. Muestra las celdas que quiere vigilar, con su hoja, celda, valor y fórmula, incluso cuando están en otra hoja. Seleccione celdas y pulse **Agregar inspección**. Seleccione filas de la lista y pulse **Eliminar inspección**. Haga doble clic en una fila para ir a la celda. Los valores cambian cuando la hoja se vuelve a calcular. La lista se conserva mientras el libro esté abierto.

## Dar nombre a valores dentro de una fórmula

**LET** asigna un nombre a un valor dentro de una fórmula, de modo que lo escribe una vez y lo usa de nuevo. Los pares de nombre y valor van primero, y el cálculo al final. En =LET(precio; B2; impuesto; precio*0,08; precio+impuesto), el nombre **precio** representa el valor de B2, **impuesto** se calcula a partir de él, y la última parte los suma. Un nombre puede usar los nombres definidos antes que él. Un nombre solo funciona dentro de su propio LET, y un LET dentro de otro puede volver a usar un nombre. Un nombre también puede representar un rango de celdas, como en =LET(filas; B2:B9; SUMA(filas)/CONTAR(filas)).

## Consultar una celda o el libro

**CELL** da información sobre una celda. Su primer argumento indica qué información: "address", "row", "col", "contents", "type", "format" (cómo se muestran los números, como "F2" para dos decimales, "P0" para un porcentaje, o "D1" para una fecha), "color" (1 cuando los números negativos se muestran en color), "parentheses" (1 cuando los números positivos se muestran entre paréntesis), "prefix" (la marca de un texto alineado a la izquierda, a la derecha o centrado), "protect" (1 cuando la celda está bloqueada), "width" (el ancho de la columna en caracteres), y "filename" (la carpeta, el archivo y la hoja, como /home/yo/[Presupuesto.xlsx]Sheet1; está vacío hasta que se guarda el libro). **INFO** da información sobre el programa y el libro: "directory", "numfile" (cuántas hojas se muestran), "origin", "osversion", "recalc" (Automatic o Manual), "release" y "system".

## Extraer valores de XML

**FILTERXML** toma un texto que contiene XML y una XPath, que es una forma breve de indicar qué partes quiere, y devuelve los valores que encuentra. =FILTERXML(A1; "//item/price") da todos los precios del texto de A1, uno debajo de otro. Use "/shop/item[2]/name" para el segundo elemento, "//item[@id='b']/name" para el elemento cuyo id es b, o "count(//item)" para saber cuántos hay. Si el texto no es XML válido, o la ruta no es válida, el resultado es #VALUE!. Si no coincide nada, es #N/A. No se admite XML que contenga un tipo de documento.
