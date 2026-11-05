# Cómo se muestran y calculan las fórmulas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, función, funciones, valor, resultado, guardado, F9, circular, mostrar fórmulas, barra de fórmulas, error, señalar, apuntar, hacer clic en una celda, referencia, F4, comprobación de errores, ignorar error, evaluar fórmula, paso, ventana inspección, inspección, LET, nombrar un valor, función CELDA, función INFO, FILTERXML, XML, XPath -->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, por ejemplo =SUMA(B3:B8). La celda muestra el valor y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con las fórmulas

Al abrir un libro, el archivo guarda el último valor de cada fórmula, de modo que la hoja se puede mostrar de inmediato. Después, JeffCrunch vuelve a calcular cada fórmula, unas pocas cada vez, para que la ventana siga siendo rápida. La barra de estado muestra **Calculando** con un porcentaje mientras lo hace. Cuando termina, las celdas contienen los resultados de JeffCrunch. Para volver a calcularlo todo, haga clic en **Recalcular todo** en la pestaña **Fórmulas**, o pulse **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 400 funciones, y sigue creciendo. Una fórmula que usa una función que JeffCrunch todavía no tiene conserva el valor que el archivo guardó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de estas se calculan a partir de los valores guardados.

## Funciones que necesitan algunas notas

- **Bonos y préstamos.** PRECIO, RENDTO, DURACION, PRECIO.PAR.IRREGULAR y las demás funciones de bonos cuentan los días según la **base** que indique (0 son 30 días por mes en un año de 360 días, 1 son los días reales, y así sucesivamente). RENDTO, RENDTO.PAR.IRREGULAR, RENDTO.ULT.IRREGULAR y TASA encuentran su respuesta probando valores hasta que el precio o el pago coinciden. Se detienen cuando la respuesta es exacta hasta unos 15 dígitos, así que las últimas cifras pueden diferir de las de otro programa.
- **Distribuciones y pruebas.** Las funciones que terminan en .DIST, .INV, .PRUEBA y los nombres anteriores, como DISTR.NORM y DISTR.T, dan las mismas respuestas: un nombre anterior se conserva para que los archivos antiguos sigan funcionando. Las funciones *.INV también se calculan probando valores.
- **Listas que ocupan varias celdas.** ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEA, BUSCARX, MMULT, MINVERSA y ESTIMACION.LINEAL devuelven un bloque de valores. Introdúzcalas como fórmula matricial (**Ctrl+Mayús+Intro**) sobre las celdas que deben llenar. Todavía no está disponible que una fórmula se extienda automáticamente a las celdas contiguas.
- **Textos en otros alfabetos.** SUMAB, IZQUIERDAB, EXTRAEB, DERECHAB, HALLARB, BUSCARB y REEMPLAZARB cuentan cada carácter como uno, como ocurre en un equipo que no está configurado para texto de doble byte. FONETICO devuelve el texto de la celda, porque JeffCrunch no guarda las lecturas del texto japonés.
- **CONVERTIR.** Las unidades se escriben como texto, con mayúsculas y minúsculas tal como aparecen en la lista de la función: "kg", "lbm", "mi", "F". Un prefijo métrico puede ir delante de una unidad que lo admita, como "km" o "mL".

## RESIDUO: el resto

**RESIDUO(n; d)** da lo que queda de **n** después de quitar las copias enteras de **d** que caben: es n - d * ENTERO(n / d). Ejemplo: **=RESIDUO(10; 3)** es 1, y **=RESIDUO(-10; 3)** es 2, porque ENTERO(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #¡DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la pestaña **Fórmulas**, o pulse **Ctrl+`** (en un Mac, la tecla Control y no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se hacen más anchas, hasta un límite, para que las fórmulas se vean completas; vuelven a su anchura al desactivarlo, y las anchuras de su archivo no cambian.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, pulse **F2**, modifíquela y pulse **Entrar**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Consulte [Escribir en celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparece una lista corta con las funciones y [nombres](names) que empiezan así, con una frase sobre cada una. Pulse **Tabulador** para elegir la que está marcada. Use las flechas **Abajo** y **Arriba** para marcar otra, y luego **Tabulador** o **Entrar**. **Esc** cierra la lista. Una vez dentro de los paréntesis de una función, sus argumentos aparecen debajo de la celda, y el que está escribiendo aparece en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con comas, o con punto y coma cuando su idioma usa la coma como separador decimal.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde puede ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de una coma) y haga clic en la celda. Su dirección se escribe por usted. Arrastre sobre celdas para escribir un bloque como B2:D5. Haga clic de nuevo antes de escribir otra cosa, y la nueva celda sustituye a la que acaba de escribirse. Mantenga pulsado **Ctrl** y haga clic para agregar otra dirección después de una coma. Haga clic en una letra de columna para escribir una columna completa, como A:A, o en un número de fila para escribir una fila completa, como 3:3, y arrastre por las letras o los números para B:D o 2:5.

Las teclas de flecha también sirven. Pulse una flecha y un puntero se aleja de la celda en la que escribe, y su dirección se escribe; pulse más flechas para seguir avanzando, o mantenga pulsado **Mayús** para extender el puntero sobre un bloque. La barra de estado muestra **Señalar** mientras las flechas señalan, **Entrar** mientras escribe, y **Editar** mientras las flechas mueven el cursor dentro del texto. Pulse **F2** para cambiar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor esté donde puede ir una celda, y luego haga clic en la celda. El nombre de la hoja se escribe junto con ella, entre comillas cuando hace falta, como en 'Mi hoja'!B3. **Entrar** le devuelve a la hoja en la que empezó y coloca la fórmula en su celda. **Esc** cancela todo, incluido el cambio de hoja.

También puede arrastrar el borde de un bloque con contorno en la hoja para mover esa dirección, o arrastrar su cuadrado de esquina para estirarlo; la fórmula cambia mientras arrastra.

Pulse **F4** con el cursor dentro de una dirección, o justo después de ella, para fijar partes de la dirección con el signo de dólar: A1 pasa a $A$1, luego a A$1, luego a $A1, y después otra vez a A1.

Cada dirección de la fórmula se muestra en un color en el texto, y las celdas a las que hace referencia aparecen con contorno en la hoja del mismo color, así puede ver de un vistazo qué lee la fórmula. Un clic después de una dirección completa, como =A1, termina la entrada y selecciona la celda en la que hizo clic, como ocurre con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Al abrir un libro que tiene una, JeffCrunch calcula sus celdas: en una tabla con dos entradas, la fórmula está en la celda de la esquina superior izquierda de la tabla, los valores que se prueban en horizontal van en la celda de entrada de fila, y los que van en vertical, en la celda de entrada de columna. En una tabla de una entrada, los valores van en la columna de su izquierda (y las fórmulas están en la fila de encima) o en la fila de encima (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de cada fórmula que depende de la celda de entrada, con la entrada fijada a ese valor; no se modifica nada más del libro. En modo **Automático** las tablas se calculan junto con las fórmulas, y las fórmulas que leen celdas de una tabla se actualizan. Todavía no se ofrece crear una tabla de datos nueva. Una tabla que no se puede calcular (una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como ALEATORIO o AHORA) conserva los valores que contiene su archivo.

## Calcular manualmente

Haga clic en **Modo de cálculo** en la pestaña **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula a medida que cambia las celdas.
- **Automático, excepto tablas** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que pulse **F9**.
- **Manual** no calcula nada, incluidas las tablas de datos, hasta que usted lo pida. La barra de estado muestra **Desactualizado** después de un cambio. Pulse **F9** (**Recalcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que está, y lo que estuviera pendiente. Un libro configurado en Manual se abre con los resultados que contiene su archivo.

La elección se guarda con el libro, y es un paso que puede deshacer.

## Fórmulas que se leen a sí mismas

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no se puede calcular de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las marca con contorno en la hoja; conservan los valores que tenían. Haga clic en **Referencias circulares** en la pestaña **Fórmulas** para ver la lista, y haga doble clic en una celda de ella para ir allí.

Algunos modelos necesitan intencionadamente un círculo de este tipo. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se calculan una y otra vez, cada una leyendo los valores más recientes de las demás, hasta que ningún valor cambie más de la cantidad que indique o se agote el número de vueltas que permita.

## Ver de qué lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña hoja representa una celda de otra hoja. Se dibujan como máximo 2000 flechas, y la barra de estado indica cuando había más. **Quitar flechas** las borra, y también las borra cualquier cambio en el libro.

## Comprobación de errores

Elija **Comprobación de errores** en el grupo **Comprobar** de la pestaña Fórmulas para revisar la hoja en busca de celdas que puedan estar mal. Busca dos cosas: una fórmula que da un error (como #¡DIV/0!), y un número guardado como texto, que las fórmulas pueden pasar por alto.

El cuadro de diálogo muestra una celda cada vez, va a esa celda en la hoja y explica en una frase cuál es el problema. **Siguiente** y **Anterior** se mueven entre las celdas. Un pequeño triángulo verde en la esquina superior izquierda de una celda la marca en la hoja. **Ignorar error** quita la celda de la lista, elimina su triángulo y se guarda en el archivo. Un paso de Deshacer la devuelve. Si no hay ningún problema, se le indica.

## Evaluar fórmula

Use esta opción para ver cómo llega una fórmula a su valor. Seleccione una celda que contenga una fórmula y elija **Evaluar fórmula**. La fórmula se muestra con la siguiente parte subrayada. Pulse **Evaluar** para calcular esa parte; su valor ocupa su lugar. Las partes se toman en este orden: primero cada celda individual de izquierda a derecha, luego cada función con sus paréntesis, empezando por la más interna, y después lo que quede. **Reiniciar** vuelve a empezar. Si una parte no se puede calcular, el cuadro de diálogo indica el motivo y deja la fórmula como estaba.

## Ventana Inspección

La **ventana Inspección** es una ventana pequeña que permanece visible mientras trabaja. Muestra las celdas que quiere vigilar, con su hoja, celda, valor y fórmula, incluso si están en otra hoja. Seleccione celdas y pulse **Agregar inspección**. Seleccione filas de la lista y pulse **Eliminar inspección**. Haga doble clic en una fila para ir a la celda. Los valores cambian cuando la hoja se vuelve a calcular. La lista se conserva mientras el libro está abierto.

## Dar nombres a valores dentro de una fórmula

**LET** da nombre a un valor en medio de una fórmula, de modo que lo escribe una vez y lo usa de nuevo. Los pares de nombre y valor van primero, y el cálculo, al final. En =LET(precio; B2; impuesto; precio*0,08; precio+impuesto), el nombre **precio** representa el valor de B2, **impuesto** se calcula a partir de él, y la última parte los suma. Un nombre puede usar los nombres anteriores. Un nombre solo funciona dentro de su propio LET, y un LET dentro de otro puede volver a usar un nombre. Un nombre también puede representar un bloque de celdas, como en =LET(filas; B2:B9; SUMA(filas)/CONTAR(filas)).

## Preguntar por una celda o por el libro

**CELDA** informa sobre una celda. Su primer argumento indica qué información: "address" (dirección), "row" (fila), "col" (columna), "contents" (contenido), "type" (tipo), "format" (cómo se muestran los números, como "F2" para dos decimales, "P0" para un porcentaje o "D1" para una fecha), "color" (1 cuando los números negativos se muestran en color), "parentheses" (1 cuando los números positivos se muestran entre paréntesis), "prefix" (la marca de un texto alineado a la izquierda, a la derecha o centrado), "protect" (1 cuando la celda está bloqueada), "width" (el ancho de la columna en caracteres), y "filename" (la carpeta, el archivo y la hoja, como /home/yo/[Presupuesto.xlsx]Hoja1; vacío hasta que se guarda el libro). **INFO** informa sobre el programa y el libro: "directory", "numfile" (cuántas hojas se muestran), "origin", "osversion", "recalc" (Automatic o Manual), "release" y "system".

## Obtener valores de XML

**FILTERXML** toma un texto que contiene XML y una XPath, que es una forma breve de indicar qué partes quiere, y devuelve los valores que encuentra. =FILTERXML(A1; "//articulo/precio") da todos los precios del texto de A1, uno debajo de otro. Use "/tienda/articulo[2]/nombre" para el segundo artículo, "//articulo[@id='b']/nombre" para el artículo cuyo id es b, o "count(//articulo)" para saber cuántos hay. Si el texto no es XML válido, o la ruta no es válida, el resultado es #¡VALOR!. Si nada coincide, es #N/A. Se rechaza el XML que tiene una declaración de tipo de documento.
