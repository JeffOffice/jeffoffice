# Cómo se muestran y se calculan las fórmulas
<!-- keywords: desbordamiento, desbordada, #SPILL!, matriz dinámica, rango de desbordamiento, A1#, siguiente error, error anterior, recorrer errores, rastrear error, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, función, funciones, valor, resultado, almacenado, F9, circular, mostrar fórmulas, barra de fórmulas, error, señalar, apuntar, hacer clic en una celda, referencia, F4, comprobación de errores, ignorar error, evaluar fórmula, paso, ventana inspección, inspección, let, asignar nombre a un valor, función cell, función info, filterxml, xml, xpath, precisión como se muestra, establecer precisión como se muestra, redondear, redondeo, decimales, dígitos-->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, por ejemplo =SUMA(B3:B8). La celda muestra el valor, y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con las fórmulas

Cuando abre un libro, el fichero guarda el último valor de cada fórmula, para que la hoja se pueda mostrar de inmediato. Después, JeffCrunch vuelve a calcular todas las fórmulas, unas pocas cada vez, para que la ventana siga siendo rápida. La barra de estado muestra **Calculando** con un porcentaje mientras lo hace. Cuando termina, las celdas contienen los resultados de JeffCrunch. Para volver a calcular todo, haga clic en **Recalcular todo** en la ficha **Fórmulas**, o pulse **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 400 funciones, y sigue creciendo. Una fórmula que usa una función que JeffCrunch todavía no tiene conserva el valor que el fichero guardó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de estas se calculan a partir de los valores guardados.

## Funciones que necesitan algunas notas

- **Bonos y préstamos.** PRECIO, RENDTO, DURACION, PRECIOPRIMPER y las demás funciones de bonos cuentan los días según la **base** que indique (0 son 30 días por mes en un año de 360 días, 1 son los días reales, y así sucesivamente). RENDTO, RENDTOPRIMAPER, RENDTOULTIM y TASA hallan su resultado probando valores hasta que el precio o el pago coinciden. Se detienen cuando el resultado es exacto hasta unos 15 dígitos, así que las últimas cifras pueden diferir de las de otro programa.
- **Distribuciones y pruebas.** Las funciones que terminan en .DIST, .INV, .PRUEBA y los nombres anteriores como DISTR.NORM y DISTR.T dan las mismas respuestas: un nombre anterior se conserva para que los ficheros antiguos sigan funcionando. Las funciones *.INV también se hallan probando valores.
- **Listas que ocupan varias celdas.** ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEATORIA, BUSCARX, MMULT, MINVERSA y ESTIMACION.LINEAL dan un bloque de valores. Escríbala en una sola celda y el bloque se desborda hacia las celdas de debajo y de al lado (vea **Fórmulas que ocupan las celdas contiguas**), o introdúzcala como fórmula matricial (**Ctrl+Mayús+Intro**) sobre las celdas que deba llenar.
- **Textos en otros alfabetos.** LENB, IZQUIERDAB, EXTRAEB, DERECHAB, ENCONTRARB, HALLARB y REEMPLAZARB cuentan cada carácter como uno, igual que en un equipo que no está configurado para texto de doble byte. FONETICO da el texto de la celda, porque JeffCrunch no guarda las lecturas del texto japonés.
- **CONVERTIR.** Las unidades se escriben como texto, con mayúsculas y minúsculas tal como aparecen en la lista de la función: "kg", "lbm", "mi", "F". Un prefijo métrico puede ir delante de una unidad que lo admita, como "km" o "mL".

## MOD: el resto

**RESIDUO(n; d)** da lo que queda de **n** después de quitar las copias enteras más grandes de **d**: es n - d * ENTERO(n / d). Ejemplo: **=RESIDUO(10; 3)** es 1, y **=RESIDUO(-10; 3)** es 2, porque ENTERO(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #¡DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la ficha **Fórmulas**, o pulse **Ctrl+'** (en un Mac, la tecla Control, no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas; vuelven a su anchura al desactivar la opción, y las anchuras de su fichero no cambian.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, pulse **F2**, edítela y pulse **Entrar**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Vea [Escribir en las celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparece una lista corta con las funciones y [nombres](names) que empiezan así, con una frase sobre cada una. Pulse **Tabulador** para elegir la que está marcada. Use las flechas **Abajo** y **Arriba** para marcar otra, y luego **Tabulador** o **Entrar**. **Esc** cierra la lista. Cuando está dentro del paréntesis de una función, sus argumentos aparecen debajo de la celda, y el que está escribiendo aparece en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con comas, o con punto y coma donde su idioma usa la coma como separador decimal.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde puede ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de una coma) y haga clic en la celda. Su dirección se escribe por usted. Arrastre sobre las celdas para escribir un rango como B2:D5. Haga clic de nuevo antes de escribir cualquier otra cosa, y la nueva celda sustituye a la que acaba de escribir. Mantenga pulsado **Ctrl** y haga clic para añadir otra dirección después de una coma. Haga clic en una letra de columna para escribir una columna completa, como A:A, o en un número de fila para escribir una fila completa, como 3:3, y arrastre por las letras o los números para B:D o 2:5.

Las teclas de flecha también funcionan. Pulse una flecha y un puntero se aleja de la celda en la que escribe, y se escribe su dirección; pulse más flechas para seguir avanzando, o mantenga pulsado **Mayús** para extender el puntero sobre un rango. La barra de estado dice **Señalar** mientras las flechas señalan, **Entrar** mientras escribe, y **Editar** mientras las flechas mueven el cursor dentro del texto. Pulse **F2** para cambiar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor está donde puede ir una celda, y luego haga clic en la celda. El nombre de la hoja se escribe junto con ella, entre comillas cuando hace falta, como en 'Mi hoja'!B3. **Entrar** le devuelve a la hoja desde la que empezó y coloca la fórmula en su celda. **Esc** cancela todo, incluido el cambio de hoja.

También puede arrastrar el borde de un rango marcado en la hoja para mover esa dirección, o arrastrar su cuadrado de esquina para ampliarla; la fórmula cambia mientras arrastra.

Pulse **F4** con el cursor dentro de una dirección o justo después de ella para fijar partes con el signo de dólar: A1 pasa a ser $A$1, luego A$1, después $A1, y por último de nuevo A1.

Cada dirección de la fórmula se colorea en el texto, y las celdas a las que se refiere se marcan en la hoja con el mismo color, así puede ver de un vistazo lo que lee la fórmula. Un clic después de una dirección completa, como =A1, termina la entrada y selecciona la celda en la que hizo clic, como ocurre con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Al abrir un libro que tiene una, JeffCrunch calcula sus celdas: en una tabla con dos entradas, la fórmula va en la celda de la esquina superior izquierda de la tabla, los valores que se prueban en horizontal van en la celda de entrada de fila, y los que van en vertical, en la celda de entrada de columna. En una tabla de una entrada, los valores van en la columna de su izquierda (y las fórmulas están en la fila de encima) o en la fila de encima (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de cada fórmula que depende de la celda de entrada, con la entrada fijada a ese valor; no se modifica nada más del libro. En modo **Automático**, las tablas se calculan junto con las fórmulas, y las fórmulas que leen las celdas de una tabla se actualizan. Crear una tabla de datos nueva todavía no está disponible. Una tabla que no puede calcularse (una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como ALEATORIO o AHORA) conserva los valores que contiene su fichero.

## Calcular manualmente

Haga clic en **Modo de cálculo** en la ficha **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula a medida que cambia las celdas.
- **Automático excepto tablas de datos** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que pulse **F9**.
- **Manual** no calcula nada, tampoco las tablas de datos, hasta que se lo pida. La barra de estado muestra **No actualizado** después de un cambio. Pulse **F9** (**Recalcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que está, y lo que más estuviera pendiente. Un libro configurado como Manual se abre con los resultados que contiene su fichero. **Calcular antes de guardar**, también en el menú **Modo de cálculo**, está marcada al principio: al guardar un libro Manual que no está actualizado, las fórmulas se calculan primero, de modo que el fichero contiene números actualizados. Desactívela para guardar los números tal como están.

La elección se guarda con el libro, y es un paso que puede deshacer.

## Fórmulas que se leen a sí mismas

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no puede calcularse de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las marca en la hoja; conservan los valores que tenían. Haga clic en **Referencias circulares** en la ficha **Fórmulas** para ver la lista, y haga doble clic en una celda de ella para ir allí.

Algunos modelos necesitan intencionadamente un círculo así. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se vuelven a calcular una y otra vez, leyendo cada una los valores más recientes de las demás, hasta que ningún valor cambie más que la cantidad que indique o se agote el número de iteraciones permitido.

## Mantener los números tal como los muestran las celdas

Una celda puede mostrar menos dígitos de los que contiene. Si una celda contiene 2,675 y muestra 2,68, las fórmulas que la leen siguen usando 2,675. A veces interesa que usen lo que se ve. En ese caso, elija **Establecer precisión como se muestra** en el menú **Opciones de cálculo** de la ficha **Fórmulas**.

JeffCrunch le avisa antes, porque los dígitos ocultos se pierden. Elija **Cancelar** y no cambia nada. Elija **Aceptar** y cada número del libro se redondea a lo que muestra su celda. Una celda que muestra dos decimales conserva dos. Una celda que muestra un porcentaje conserva el porcentaje. Una celda con una fecha conserva el día, y una celda con una hora conserva la unidad menor de tiempo que muestra, como el minuto. Una celda que muestra **General** conserva todos sus dígitos. Un libro grande muestra un aviso con un botón **Cancelar** mientras se hace esto. Si cancela mientras el aviso está visible, no se cambia nada.

Después, cada número que escriba o pegue, y cada respuesta que dé una fórmula, se redondea de la misma forma. Las fórmulas en sí no se cambian. La elección se guarda con el libro.

El redondeo es un paso que puede deshacer, y **Deshacer** devuelve los números originales. Elegir **Establecer precisión como se muestra** de nuevo desactiva la opción. Eso solo impide que se siga redondeando a partir de ese momento. Los números que ya se habían redondeado siguen redondeados.

## Ver lo que lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para retroceder un paso más. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña página representa una celda de otra hoja. Se dibujan como máximo 2.000 flechas, y la barra de estado indica cuando hay más. **Quitar flechas** las borra, y también lo hace cualquier cambio en el libro.

Cuando una celda muestra un error como #¡DIV/0!, selecciónela y haga clic en **Rastrear error**. Se dibujan flechas hacia las celdas que lee y que también muestran un error. Haga clic de nuevo para retroceder un paso más. Cuando ninguna de las celdas que lee muestra un error, la barra de estado indica que esta celda produce el error, y esa es la que hay que corregir.

Para recorrer todos los errores de la hoja, haga clic en **Siguiente error** o **Error anterior**. Las celdas que muestran un error se visitan una fila tras otra, y después de la última se vuelve a empezar por la primera. Cada celda se selecciona, y las flechas se dibujan hasta la celda que produce el error. La barra de estado indica en qué error se encuentra y cuántos hay.

## Comprobación de errores

Elija **Comprobación de errores** en el grupo **Auditoría de fórmulas** de la ficha Fórmulas para buscar en la hoja celdas que puedan estar mal. Busca dos cosas: una fórmula que da un error (como #¡DIV/0!), y un número guardado como texto, que las fórmulas pueden omitir.

El cuadro de diálogo muestra una celda cada vez, va a esa celda en la hoja y explica en una frase qué ocurre. **Siguiente** y **Anterior** recorren las celdas. Un pequeño triángulo verde en la esquina superior izquierda de una celda la marca en la hoja. **Ignorar error** quita la celda de la lista, le quita el triángulo y se guarda en el fichero. Un paso de Deshacer la devuelve. Si no hay nada que corregir, se le indica.

## Evaluar fórmula

Use esta opción para ver cómo llega una fórmula a su valor. Seleccione una celda que contenga una fórmula y elija **Evaluar fórmula**. La fórmula se muestra con la parte siguiente subrayada. Pulse **Evaluar** para calcular esa parte; su valor ocupa su lugar. Las partes van en este orden: cada celda individual de izquierda a derecha, cada función con su paréntesis, empezando por la más interna, y después lo que quede. **Reiniciar** vuelve a empezar. Si una parte no puede calcularse, el cuadro de diálogo indica por qué y deja la fórmula como estaba.

## Ventana Inspección

La **ventana Inspección** es una pequeña ventana que permanece visible mientras trabaja. Muestra las celdas que quiere vigilar, con su hoja, celda, valor y fórmula, incluso cuando están en otra hoja. Seleccione celdas y pulse **Agregar inspección**. Seleccione filas de la lista y pulse **Eliminar inspección**. Haga doble clic en una fila para ir a la celda. Los valores cambian cuando la hoja se vuelve a calcular. La lista se conserva mientras el libro está abierto.

## Fórmulas que ocupan las celdas contiguas

Cuando una fórmula da un bloque de valores, el bloque ocupa las celdas de debajo y de al lado de la celda en la que la escribió. Esto se llama desbordamiento. Pruebe =SECUENCIA(3) en A1: A1 muestra 1, y A2 y A3 muestran 2 y 3. Una fórmula se desborda cuando llama a una función que da un bloque (ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEATORIA, TRANSPONER, BUSCARX o MMULT), escribe un bloque entre llaves, usa un rango de desbordamiento (más abajo) o hace operaciones aritméticas con un rango, como en =A1:A3*2.

- **Las celdas que ocupa no tienen fórmula propia.** Muestran los valores. Cuando selecciona una de ellas, la barra de fórmulas muestra en gris la fórmula de la celda que la ocupa. Pulsar Entrar sobre ella tal cual no cambia nada. Para cambiar el bloque, cambie primero esa primera celda.
- **El bloque sigue al resultado.** Si la fórmula da más o menos valores después de que cambie algo que lee, el bloque crece o se reduce, y las fórmulas que leen esas celdas se vuelven a calcular.
- **Una celda en medio da #SPILL!.** Si alguna celda donde iría el bloque contiene un valor o una fórmula, la primera celda muestra **#SPILL!** y no se rellena nada. Borre la celda que estorba y el bloque se rellena al instante. Escribir en una celda que ocupa el bloque produce el mismo efecto: la primera celda muestra #SPILL!.
- **Un bloque tiene un límite de tamaño.** Un bloque más grande que la hoja, o de más de 200.000 celdas, da #SPILL!.
- **A1# es todo el bloque.** Escriba la celda que contiene la fórmula seguida de **#**, como en =SUMA(A1#), para usar el bloque completo tal como está ahora, desde esta hoja o desde otra (=SUMA(Hoja1!A1#)). Si A1 no se desborda, A1# es #¡REF!.

Un desbordamiento se guarda con el fichero como matriz dinámica, de modo que el programa en el que se creó el fichero también lo muestra como desbordamiento. Al guardar en el formato antiguo .xls o .ods se conservan los valores, y la fórmula como fórmula matricial sobre el bloque cuando el formato antiguo tiene las funciones que usa. Comprobar compatibilidad indica estas fórmulas.

## Dar nombres a valores dentro de una fórmula

**LET** asigna un nombre a un valor en medio de una fórmula, de modo que lo escribe una vez y lo usa de nuevo. Primero van los pares de nombre y valor, y al final el cálculo. En =LET(precio; B2; impuesto; precio*0,08; precio+impuesto) el nombre **precio** representa el valor de B2, **impuesto** se calcula a partir de él, y la última parte los suma. Un nombre puede usar los nombres anteriores, y un LET puede contener hasta 126 pares. Un nombre solo funciona dentro de su propio LET, y un LET dentro de otro puede volver a usar un nombre. Un nombre también puede representar un rango de celdas, como en =LET(filas; B2:B9; SUMA(filas)/CONTAR(filas)).

## Preguntar por una celda o por el libro

**CELDA** informa sobre una celda. Su primer argumento indica qué información: "dirección", "fila", "col", "contenido", "tipo", "formato" (cómo se muestran los números, como "F2" para dos decimales, "P0" para un porcentaje o "D1" para una fecha), "color" (1 cuando los números negativos se muestran en color), "paréntesis" (1 cuando los números positivos se muestran entre paréntesis), "prefijo" (la marca de un texto alineado a la izquierda, a la derecha o centrado), "proteger" (1 cuando la celda está bloqueada), "ancho" (el ancho de la columna en caracteres) y "nombrearchivo" (la carpeta, el fichero y la hoja, como /home/yo/[Presupuesto.xlsx]Hoja1; vacío hasta que se guarda el libro). **INFO** informa sobre el programa y el libro: "directorio", "numarchivo" (cuántas hojas se muestran), "origen", "versiónsO", "recalc" (Automático o Manual), "versión" y "sistema".

## Extraer valores de XML

**FILTROXML** toma un texto que contiene XML y una XPath, que es una forma breve de indicar qué partes quiere, y devuelve los valores que encuentra. =FILTROXML(A1; "//item/price") da todos los precios del texto de A1, uno debajo de otro. Use "/shop/item[2]/name" para el segundo elemento, "//item[@id='b']/name" para el elemento cuyo id es b, o "count(//item)" para saber cuántos hay. Si el texto no es XML válido, o la ruta no es válida, el resultado es #¡VALOR!. Si no coincide nada, es #N/D. Se rechaza el XML que tiene un tipo de documento.
