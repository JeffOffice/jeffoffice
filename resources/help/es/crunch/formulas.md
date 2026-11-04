# Cómo se muestran y calculan las fórmulas
<!-- keywords: fórmula, fórmulas, calcular, cálculo, recalcular, función, funciones, valor, resultado, almacenado, F9, circular, mostrar fórmulas, barra de fórmulas, error, señalar, apuntar, hacer clic en una celda, referencia, F4 -->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, como =SUMA(B3:B8). La celda muestra el valor, y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con las fórmulas

Al abrir un libro, el archivo guarda el último valor de cada fórmula, de modo que la hoja puede mostrarse de inmediato. Después, JeffCrunch vuelve a calcular cada fórmula, unas pocas cada vez, para que la ventana siga siendo rápida. La barra de estado muestra **Calculando** con un porcentaje mientras lo hace. Cuando termina, las celdas contienen los resultados de JeffCrunch. Para volver a calcular todo, haga clic en **Volver a calcular todo** en la pestaña **Fórmulas**, o pulse **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 400 funciones, y sigue creciendo. Una fórmula que usa una función que JeffCrunch todavía no tiene conserva el valor que el archivo guardó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de esas se calculan a partir de los valores guardados.

## Funciones que necesitan algunas notas

- **Bonos y préstamos.** PRECIO, RENDTO, DURACION, PRECIO.PERIODO.IMPAR y las demás funciones de bonos cuentan los días según la **base** que indique (0 es de 30 días por mes en un año de 360 días, 1 son los días reales, y así sucesivamente). RENDTO, RENDTO.PERIODO.IMPAR, RENDTO.ULT.PERIODO.IMPAR y TASA encuentran su respuesta probando valores hasta que el precio o el pago coinciden. Se detienen cuando la respuesta es correcta hasta unos 15 dígitos, así que las últimas cifras pueden diferir de las de otro programa.
- **Distribuciones y pruebas.** Las funciones que terminan en .DIST, .INV, .PRUEBA y los nombres anteriores, como DISTR.NORM y DISTR.T, dan las mismas respuestas: un nombre anterior se conserva para que los archivos antiguos sigan funcionando. Las funciones *.INV también se calculan probando valores.
- **Listas que ocupan varias celdas.** ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEATORIA, BUSCARX, MMULT, MINVERSA y ESTIMACION.LINEAL devuelven un bloque de valores. Introdúzcalas como fórmula matricial (**Ctrl+Mayús+Intro**) sobre las celdas que deben llenar. Todavía no se admite que una fórmula se extienda automáticamente a las celdas contiguas.
- **Textos en otros alfabetos.** LENB, IZQUIERDAB, EXTRAEB, DERECHAB, ENCONTRARB, HALLARB y REEMPLAZARB cuentan cada carácter como uno, como ocurre en un equipo que no está configurado para textos de doble byte. FONETICO devuelve el texto de la celda, porque JeffCrunch no guarda las lecturas del texto japonés.
- **CONVERTIR.** Las unidades se escriben como texto, con mayúsculas y minúsculas tal como aparecen en la lista de la función: "kg", "lbm", "mi", "F". Un prefijo métrico puede ir delante de una unidad que lo admita, como "km" o "mL".

## MOD: el resto

**MOD(n; d)** devuelve lo que queda de **n** después de quitar el mayor número entero de veces **d**: es n - d * ENTERO(n / d). Ejemplo: **=MOD(10; 3)** es 1, y **=MOD(-10; 3)** es 2, porque ENTERO(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #¡DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la pestaña **Fórmulas**, o pulse **Ctrl+'** (en un Mac, la tecla Control y no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas; vuelven a su anchura al desactivar la opción, y las anchuras de su archivo no se modifican.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, pulse **F2**, edítela y pulse **Entrar**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Consulte [Escribir en celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparecerá una lista corta con las funciones y [nombres](names) que empiezan así, con una frase sobre cada una. Pulse **Tab** para tomar la que está marcada. Use las flechas **Abajo** y **Arriba** para marcar otra, y después **Tab** o **Entrar**. **Esc** cierra la lista. Una vez dentro de los paréntesis de una función, sus argumentos aparecen debajo de la celda, con el que está escribiendo en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con comas, o con punto y coma donde su idioma usa la coma como separador decimal.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde puede ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de una coma) y haga clic en la celda. Su dirección se escribe por usted. Arrastre sobre las celdas para escribir un bloque, como B2:D5. Haga clic de nuevo antes de escribir cualquier otra cosa, y la nueva celda reemplazará a la que acaba de escribir. Mantenga pulsada **Ctrl** y haga clic para añadir otra dirección después de una coma. Haga clic en una letra de columna para escribir una columna completa, como A:A, o en un número de fila para escribir una fila completa, como 3:3, y arrastre por las letras o los números para obtener B:D o 2:5.

Las teclas de flecha también funcionan. Pulse una flecha y un puntero se aleja de la celda en la que escribe, y su dirección se escribe; pulse más flechas para seguir avanzando, o mantenga pulsada **Mayús** para extender el puntero sobre un bloque. La barra de estado muestra **Señalar** mientras las flechas señalan, **Entrar** mientras escribe, y **Editar** mientras las flechas mueven el cursor dentro del texto. Pulse **F2** para cambiar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor está donde puede ir una celda, y después haga clic en la celda. El nombre de la hoja se escribe junto con la dirección, entre comillas cuando hacen falta, como en 'Mi hoja'!B3. **Entrar** le devuelve a la hoja en la que empezó y coloca la fórmula en su celda. **Esc** descarta todo, incluido el cambio de hoja.

También puede arrastrar el borde de un bloque con contorno en la hoja para mover esa dirección, o arrastrar su cuadrado de esquina para estirarlo; la fórmula cambia mientras arrastra.

Pulse **F4** con el cursor dentro de una dirección o justo detrás de ella para fijar partes de la dirección con un signo de dólar: A1 pasa a ser $A$1, después A$1, luego $A1, y después vuelve a A1.

Cada dirección de la fórmula aparece coloreada en el texto, y las celdas a las que hace referencia se marcan con un contorno de ese mismo color en la hoja, así puede ver de un vistazo qué lee la fórmula. Un clic después de una dirección completa, como en =A1, termina la entrada y selecciona la celda en la que hizo clic, como ocurre con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Al abrir un libro que contiene una, JeffCrunch calcula sus celdas: en una tabla con dos entradas, la fórmula está en la celda de la esquina superior izquierda de la tabla, los valores que se prueban en horizontal van en la celda de entrada de fila, y los que van en vertical, en la celda de entrada de columna. En una tabla de una sola entrada, los valores van en la columna de su izquierda (y las fórmulas están en la fila de encima) o en la fila de encima (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de todas las fórmulas que dependen de la celda de entrada, con la entrada fijada a ese valor; no se modifica nada más del libro. En modo **Automático** las tablas se calculan junto con las fórmulas, y las fórmulas que leen las celdas de una tabla se actualizan con ellas. Crear una nueva tabla de datos todavía no está disponible. Una tabla que no puede calcularse (una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como ALEATORIO o AHORA) conserva los valores que contiene su archivo.

## Calcular manualmente

Haga clic en **Modo de cálculo** en la pestaña **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula a medida que cambia las celdas.
- **Automático, excepto tablas** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que pulse **F9**.
- **Manual** no calcula nada, tablas de datos incluidas, hasta que lo pida. La barra de estado muestra **Desactualizado** después de un cambio. Pulse **F9** (**Volver a calcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que está, y lo que estuviera pendiente. Un libro configurado como Manual se abre con los resultados que contiene su archivo.

La elección se guarda con el libro, y es un paso que puede deshacer.

## Fórmulas que se leen a sí mismas

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no puede calcularse de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las marca con un contorno en la hoja; conservan los valores que tenían. Haga clic en **Referencias circulares** en la pestaña **Fórmulas** para ver la lista, y haga doble clic en una celda de la lista para ir a ella.

Algunos modelos necesitan un circuito así a propósito. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se calculan una y otra vez, leyendo cada una los valores más recientes de las demás, hasta que ningún valor cambie más de la cantidad que indique o se agote el número de iteraciones permitido.

## Ver qué lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña hoja representa una celda de otra hoja. Se dibujan como máximo 2000 flechas, y la barra de estado indica cuando había más. **Quitar flechas** las borra, y también lo hace cualquier cambio en el libro.
