# Cómo se muestran y calculan las fórmulas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, función, funciones, valor, resultado, almacenado, F9, circular, mostrar fórmulas, barra de fórmulas, error, señalar, apuntar, hacer clic en una celda, referencia, F4, comprobación de errores, ignorar error, evaluar fórmula, paso, ventana inspección, inspeccionar, vigilar -->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, como =SUMA(B3:B8). La celda muestra el valor, y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con las fórmulas

Al abrir un libro, el archivo guarda el último valor de cada fórmula, de modo que la hoja puede mostrarse de inmediato. Después, JeffCrunch vuelve a calcular cada fórmula, unas pocas cada vez, para que la ventana siga siendo rápida. La barra de estado muestra **Calculando** con un porcentaje mientras lo hace. Cuando termina, las celdas contienen los resultados de JeffCrunch. Para volver a calcular todo, haga clic en **Volver a calcular todo** en la pestaña **Fórmulas**, o presione **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 400 funciones, y sigue creciendo. Una fórmula que usa una función que JeffCrunch todavía no tiene conserva el valor que el archivo guardó para ella. Lo mismo ocurre con una que hace referencia a otro libro. Las celdas que dependen de estas se calculan a partir de los valores guardados.

## Funciones que requieren algunas notas

- **Bonos y préstamos.** PRECIO, RENDTO, DURACION, PRECIO.PRIMER.PER y las demás funciones de bonos cuentan los días según la **base** que indique (0 son 30 días por mes en un año de 360 días, 1 son los días reales, y así sucesivamente). RENDTO, RENDTO.PRIMER.PER, RENDTO.ULTIMO.PER y TASA hallan su resultado probando valores hasta que el precio o el pago coinciden. Se detienen cuando el resultado es correcto hasta unos 15 dígitos, por lo que las últimas cifras pueden diferir de las de otro programa.
- **Distribuciones y pruebas.** Las funciones que terminan en .DIST, .INV, .PRUEBA y los nombres anteriores, como DISTR.NORM y DISTR.T, dan las mismas respuestas: se conserva un nombre anterior para que los archivos antiguos sigan funcionando. Las funciones *.INV también se hallan probando valores.
- **Listas que ocupan varias celdas.** ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEATORIA, BUSCARX, MMULT, MINVERSA y ESTIMACION.LINEAL devuelven un bloque de valores. Introdúzcalas como fórmula matricial (**Ctrl+Mayús+Intro**) sobre las celdas que deben rellenar. Todavía no está disponible que una fórmula se extienda a las celdas contiguas.
- **Textos en otros alfabetos.** LENB, IZQUIERDAB, EXTRAEB, DERECHAB, HALLARB, BUSCARB y REEMPLAZARB cuentan cada carácter como uno, como ocurre en un equipo que no está configurado para texto de doble byte. FONETICO devuelve el texto de la celda, porque JeffCrunch no guarda las lecturas del texto japonés.
- **CONVERTIR.** Las unidades se escriben como texto, con mayúsculas y minúsculas como aparecen en la lista de la función: "kg", "lbm", "mi", "F". Un prefijo métrico puede ir delante de una unidad que lo admita, como "km" o "mL".

## MOD: el resto

**RESIDUO(n; d)** da lo que queda de **n** después de quitar el mayor número de copias enteras de **d**: es n - d * ENTERO(n / d). Ejemplo: **=RESIDUO(10; 3)** es 1, y **=RESIDUO(-10; 3)** es 2, porque ENTERO(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #¡DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la pestaña **Fórmulas**, o presione **Ctrl+'** (en un Mac, la tecla Control y no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas; vuelven a su anchura al desactivar la opción, y las anchuras de su archivo no cambian.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, presione **F2**, modifíquela y presione **Entrar**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Consulte [Escribir en celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparecerá una lista corta con las funciones y [nombres](names) que empiezan así, con una frase sobre cada una. Presione **Tab** para elegir la marcada. Use las flechas **Abajo** y **Arriba** para marcar otra, y después **Tab** o **Entrar**. **Esc** cierra la lista. Una vez dentro de los paréntesis de una función, sus argumentos aparecen debajo de la celda, y el que está escribiendo se muestra en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con punto y coma, o con comas donde su idioma usa la coma como separador decimal.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde puede ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de una coma) y haga clic en la celda. Su dirección se escribe por usted. Arrastre sobre celdas para escribir un bloque, como B2:D5. Haga clic de nuevo antes de escribir algo más, y la nueva celda sustituye a la que acaba de escribirse. Mantenga presionada **Ctrl** y haga clic para añadir otra dirección tras una coma. Haga clic en una letra de columna para escribir una columna completa, como A:A, o en un número de fila para escribir una fila completa, como 3:3, y arrastre sobre las letras o los números para B:D o 2:5.

Las teclas de flecha también funcionan. Presione una flecha y un puntero se aleja de la celda en la que escribe, y su dirección se escribe; presione más flechas para seguir avanzando, o mantenga presionada **Mayús** para extender el puntero sobre un bloque. La barra de estado muestra **Señalar** mientras las flechas señalan, **Entrar** mientras escribe, y **Editar** mientras las flechas mueven el cursor dentro del texto. Presione **F2** para alternar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor esté donde puede ir una celda, y después haga clic en la celda. El nombre de la hoja se escribe junto con ella, entre comillas cuando hace falta, como en 'Mi hoja'!B3. **Entrar** lo devuelve a la hoja en la que empezó y coloca la fórmula en su celda. **Esc** cancela todo, incluido el cambio de hoja.

También puede arrastrar el borde de un bloque con contorno de la hoja para mover esa dirección, o arrastrar su cuadrado de esquina para estirarlo; la fórmula cambia mientras arrastra.

Presione **F4** con el cursor dentro de una dirección o justo detrás de ella para fijar partes de la misma con un signo de dólar: A1 pasa a ser $A$1, luego A$1, después $A1, y de nuevo A1.

Cada dirección de la fórmula se muestra en un color en el texto, y las celdas a las que hace referencia se marcan con contorno en la hoja con ese mismo color, para que vea de un vistazo qué lee la fórmula. Un clic después de una dirección completa, como =A1, termina la entrada y selecciona la celda en la que hizo clic, como ocurre con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de análisis de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Al abrir un libro que contiene una, JeffCrunch calcula sus celdas: en una tabla con dos entradas, la fórmula está en la celda de la esquina superior izquierda de la tabla, los valores que se prueban en horizontal se escriben en la celda de entrada de fila, y los que se prueban en vertical, en la celda de entrada de columna. En una tabla de una sola entrada, los valores van en la columna de su izquierda (y las fórmulas están en la fila superior) o en la fila superior (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de todas las fórmulas que dependen de la celda de entrada, con la entrada establecida en ese valor; no se modifica nada más del libro. En modo **Automático** las tablas se calculan junto con las fórmulas, y las fórmulas que leen celdas de una tabla se actualizan. Todavía no se ofrece crear una tabla de datos nueva. Una tabla que no puede calcularse (una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como ALEATORIO o AHORA) conserva los valores de su archivo.

## Calcular manualmente

Haga clic en **Modo de cálculo** en la pestaña **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula al cambiar celdas.
- **Automático, excepto tablas** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que presione **F9**.
- **Manual** no calcula nada, tampoco las tablas de datos, hasta que lo pida. La barra de estado muestra **Desactualizado** tras un cambio. Presione **F9** (**Volver a calcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que se encuentra, y cualquier otra cosa pendiente. Un libro configurado en Manual se abre con los resultados que contiene su archivo.

La opción se guarda con el libro, y es un paso que puede deshacer.

## Fórmulas que leen su propio resultado

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no puede calcularse de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las marca con contorno en la hoja; conservan los valores que tenían. Haga clic en **Referencias circulares** en la pestaña **Fórmulas** para ver la lista, y haga doble clic en una celda de ella para ir allí.

Algunos modelos necesitan una circularidad a propósito. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se vuelven a calcular una y otra vez, leyendo cada una los valores más recientes de las demás, hasta que ningún valor cambie más que la cantidad que indique o se agote el número de iteraciones permitido.

## Ver qué lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña hoja representa una celda de otra hoja. Se dibujan como máximo 2000 flechas, y la barra de estado indica cuando había más. **Quitar flechas** las borra, y también lo hace cualquier cambio en el libro.

## Comprobación de errores

Elija **Comprobación de errores** en el grupo **Comprobar** de la pestaña Fórmulas para revisar la hoja y buscar celdas que puedan ser incorrectas. Busca dos cosas: una fórmula que da un error (como #¡DIV/0!), y un número guardado como texto, que las fórmulas pueden pasar por alto.

El cuadro de diálogo muestra una celda cada vez, va a esa celda en la hoja y explica en una frase qué ocurre. **Siguiente** y **Anterior** recorren las celdas. Un pequeño triángulo verde en la esquina superior izquierda de una celda la señala en la hoja. **Ignorar error** quita la celda de la lista, le quita el triángulo y se guarda en el archivo. Un paso de Deshacer la devuelve. Si no hay nada que corregir, se le indica.

## Evaluar fórmula

Use esta opción para ver cómo llega una fórmula a su valor. Seleccione una celda que contenga una fórmula y elija **Evaluar fórmula**. La fórmula se muestra con la siguiente parte subrayada. Presione **Evaluar** para calcular esa parte; su valor ocupa su lugar. Las partes se evalúan en este orden: cada celda individual de izquierda a derecha, cada función con sus paréntesis, empezando por la más interna, y después lo que quede. **Reiniciar** vuelve a empezar. Si una parte no puede calcularse, el cuadro de diálogo indica el motivo y deja la fórmula como estaba.

## Ventana Inspección

La **Ventana de inspección** es una ventana pequeña que permanece visible mientras trabaja. Muestra las celdas que quiere vigilar, con su hoja, celda, valor y fórmula, incluso cuando están en otra hoja. Seleccione celdas y presione **Agregar inspección**. Seleccione filas de la lista y presione **Eliminar inspección**. Haga doble clic en una fila para ir a la celda. Los valores cambian cuando la hoja se vuelve a calcular. La lista se conserva mientras el libro está abierto.
