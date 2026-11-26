# Cómo se muestran y se calculan las fórmulas
<!-- keywords: desbordamiento, desbordada, #DESBORDAMIENTO!, matriz dinámica, rango de desbordamiento, A1#, siguiente error, error siguiente, error anterior, recorrer errores, rastrear error, calcular antes de guardar, fórmula, fórmulas, calcular, cálculo, recalcular, función, funciones, valor, resultado, almacenado, F9, circular, mostrar fórmulas, barra de fórmulas, error, señalar, señalando, hacer clic en una celda, referencia, F4, comprobación de errores, ignorar error, evaluar fórmula, paso a paso, inspección, ventana de inspección, let, asignar nombre a un valor, función de celda, función de información, filterxml, xml, xpath, precisión como se muestra, establecer precisión como se muestra, redondear, redondeo, decimales, dígitos-->

Una **fórmula** es una celda que calcula su valor a partir de otras celdas, por ejemplo =SUMA(B3:B8). La celda muestra el valor, y la **barra de fórmulas** muestra la fórmula.

## Qué hace JeffCrunch con las fórmulas

Al abrir un libro, el fichero guarda el último valor de cada fórmula, de modo que la hoja se muestra enseguida. Después, JeffCrunch vuelve a calcular todas las fórmulas, unas pocas cada vez, para que la ventana siga siendo rápida. La barra de estado muestra **Calculando** con un porcentaje mientras lo hace. Cuando termina, las celdas contienen los resultados de JeffCrunch. Para volver a calcular todo, haga clic en **Recalcular todo** en la ficha **Fórmulas**, o pulse **F9**.

## Cuando JeffCrunch no puede calcular una fórmula

JeffCrunch tiene más de 400 funciones, y sigue creciendo. Una fórmula que usa una función que JeffCrunch todavía no tiene conserva el valor que el fichero guardó para ella. Lo mismo ocurre con una fórmula que hace referencia a otro libro. Las celdas que dependen de ellas se calculan a partir de los valores guardados.

## Funciones que necesitan algunas notas

- **Bonos y préstamos.** PRECIO, RENDTO, DURACION, PRECIO.PRIMER.PER.IRREG y las demás funciones de bonos cuentan los días según la **base** que indique (0 son 30 días por mes en un año de 360 días, 1 son los días reales, y así sucesivamente). RENDTO, RENDTO.PRIMER.PER.IRREG, RENDTO.ULTIMO.PER.IRREG y TASA hallan su resultado probando valores hasta que el precio o el pago coinciden. Se detienen cuando el resultado es correcto hasta unos 15 dígitos, así que las últimas cifras pueden diferir de las de otro programa.
- **Distribuciones y pruebas.** Las funciones que terminan en .DIST, .INV, .PRUEBA, y los nombres anteriores como DISTR.NORM y DISTR.T, dan las mismas respuestas: un nombre anterior se mantiene para que los ficheros antiguos sigan funcionando. Las funciones *.INV también se calculan probando valores.
- **Listas que ocupan varias celdas.** ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEATORIA, BUSCARX, MMULT, MINVERSA y ESTIMACION.LINEAL dan un bloque de valores. Escríbala en una sola celda y el bloque se desborda hacia las celdas de abajo y de al lado (vea **Fórmulas que llenan las celdas de al lado**), o introdúzcala como fórmula matricial (**Ctrl+Mayús+Intro**) sobre las celdas que debe llenar.
- **Textos en otros alfabetos.** LENB, IZQUIERDAB, EXTRAEB, DERECHAB, HALLARB, BUSCARB y REEMPLAZARB cuentan cada carácter como uno, como ocurre en un equipo que no está configurado para textos de doble byte. FONETICO da el texto de la celda, porque JeffCrunch no guarda las lecturas del texto japonés.
- **CONVERTIR.** Las unidades se escriben como texto, con mayúsculas y minúsculas como aparecen en la lista de la función: "kg", "lbm", "mi", "F". Un prefijo métrico puede ir delante de una unidad que lo admita, como "km" o "mL".

## MOD: el resto

**RESTO(n; d)** da lo que queda de **n** después de quitar las copias enteras más grandes de **d**: es n - d * ENTERO(n / d). Ejemplo: **=RESTO(10; 3)** es 1, y **=RESTO(-10; 3)** es 2, porque ENTERO(-10 / 3) es -4 y -10 - 3 * -4 es 2. El resultado tiene el signo de **d**. Si **d** es 0, el resultado es #¡DIV/0!.

## Mostrar las fórmulas

Haga clic en **Mostrar fórmulas** en la ficha **Fórmulas**, o pulse **Ctrl+'** (en un Mac, la tecla Control, no la tecla Comando), para ver la fórmula de cada celda en lugar de su valor. Las columnas que contienen fórmulas se ensanchan, hasta un límite, para que las fórmulas se vean completas; vuelven a su anchura al desactivarlo, y no se cambian los anchos de su fichero.

## Cambiar fórmulas

Para cambiar una fórmula, seleccione su celda, pulse **F2**, edítela y pulse **Entrar**. También puede editarla en la barra de fórmulas. Cuando cambia una celda, las fórmulas que dependen de ella se vuelven a calcular en segundo plano. Vea [Escribir en las celdas](edit-cells).

## Escribir una fórmula

Escriba **=** y las primeras letras de una función, y aparece una lista breve con las funciones y [nombres](names) que empiezan así, con una frase sobre cada una. Pulse **Tab** para elegir la que está marcada. Use las flechas **Abajo** y **Arriba** para marcar otra, y después **Tab** o **Entrar**. **Esc** cierra la lista. Una vez dentro de los paréntesis de una función, sus argumentos aparecen debajo de la celda, y el que está escribiendo aparece en negrita. La lista y los argumentos son los mismos que muestra **Elegir una función**. Los argumentos se separan con comas, o con punto y coma donde su idioma usa la coma como separador decimal.

## Nombres de funciones en su idioma

Cuando JeffCrunch se muestra en alemán, francés, español, italiano, neerlandés, portugués o ruso, puede escribir una fórmula con los nombres de funciones que usa su idioma, como `=SUMME(A1;A2)` en alemán o `=SOMME(A1;A2)` en francés. Los nombres en inglés también funcionan, así que una fórmula copiada de la web no necesita cambios. La barra de fórmulas, la celda, la lista de funciones y **Elegir una función** muestran los nombres de su idioma.

El libro no cambia al cambiar de idioma. Un fichero siempre guarda los nombres en inglés, así que un libro creado en un idioma se abre y se lee correctamente en otro, y en cualquier otro programa. Los nombres locales están disponibles en alemán, francés, español, italiano, neerlandés, portugués (Brasil y Portugal) y ruso; los nombres en inglés siempre funcionan en todos los idiomas. Los programas en ucraniano e indonesio mantienen los nombres en inglés para casi todas las funciones. Cuando un idioma tiene dos nombres habituales para una función, se leen ambos y se muestra el más común. Una función que no tiene un nombre habitual en su idioma conserva el inglés. VERDADERO y FALSO también siguen a su idioma. Los valores de error de su idioma, como `#NV` en alemán, se leen al escribir una fórmula y se muestran en la barra de fórmulas, pero una celda con error muestra siempre la forma estándar, como `#N/A`.

Cuando la configuración regional de su equipo usa la coma como separador decimal, los argumentos de una función se separan con punto y coma. Una matriz entre llaves con más de una fila se muestra con punto y coma entre los valores de una fila y una barra invertida entre filas, como `{1;2\3;4}`, y puede escribirla así. Una matriz escrita sin barra invertida se lee como la escribe un fichero: comas entre los valores de una fila, punto y coma entre filas, y punto como separador decimal.

Para escribir solo nombres en inglés, haga clic en **Archivo > Opciones > Fórmulas** y marque **Usar nombres de funciones en inglés**.

## Señalar celdas

En lugar de escribir la dirección de una celda, puede señalarla. Mientras escribe una fórmula, coloque el cursor donde puede ir una celda (justo después de **=**, de un operador como **+**, de un paréntesis de apertura o de una coma) y haga clic en la celda. Su dirección se escribe por usted. Arrastre sobre las celdas para escribir un rango como B2:D5. Haga clic de nuevo antes de escribir cualquier otra cosa, y la nueva celda reemplaza a la que acaba de escribir. Mantenga pulsada **Ctrl** y haga clic para añadir otra dirección después de una coma. Haga clic en una letra de columna para escribir una columna completa, como A:A, o en un número de fila para escribir una fila completa, como 3:3, y arrastre por las letras o los números para B:D o 2:5.

Las teclas de flecha también funcionan. Pulse una flecha y un puntero se aleja de la celda en la que escribe, y su dirección se escribe; pulse más flechas para seguir avanzando, o mantenga pulsada **Mayús** para extender el puntero sobre un rango. La barra de estado muestra **Señalar** mientras las flechas señalan, **Entrar** mientras escribe, y **Editar** mientras las flechas mueven el cursor dentro del texto. Pulse **F2** para alternar entre señalar y editar el texto.

Para señalar una celda de otra hoja, haga clic en la pestaña de esa hoja mientras el cursor está donde puede ir una celda, y después haga clic en la celda. El nombre de la hoja se escribe junto con ella, entre comillas cuando hace falta, como en 'Mi hoja'!B3. **Entrar** le lleva de nuevo a la hoja en la que empezó y coloca la fórmula en su celda. **Esc** lo cancela todo, incluido el cambio de hoja.

También puede arrastrar el borde de un rango con contorno de la hoja para mover esa dirección, o arrastrar su cuadrado de esquina para estirarlo; la fórmula cambia mientras arrastra.

Pulse **F4** con el cursor dentro de una dirección o justo después de ella para fijar partes con un signo de dólar: A1 pasa a $A$1, luego a A$1, después a $A1, y de nuevo a A1.

Cada dirección de la fórmula se colorea en el texto, y las celdas que nombra se marcan con un contorno en la hoja con el mismo color, así que puede ver de un vistazo lo que lee la fórmula. Un clic después de una dirección completa, como en =A1, termina la entrada y selecciona la celda en la que hizo clic, igual que con cualquier otro texto.

## Tablas de datos

Una tabla de datos (tabla de hipótesis) muestra el resultado de una fórmula para muchos valores de una o dos celdas. Al abrir un libro que tiene una, JeffCrunch calcula sus celdas: en una tabla con dos entradas, la fórmula está en la celda de la esquina superior izquierda de la tabla, los valores a probar en horizontal van en la celda de entrada de fila, y los que van en vertical, en la celda de entrada de columna. En una tabla con una entrada, los valores van en la columna de su izquierda (y las fórmulas están en la fila de encima) o en la fila de encima (y las fórmulas están en la columna de su izquierda). Cada celda de la tabla es el resultado de la fórmula, y de todas las fórmulas que dependen de la celda de entrada, con la entrada fijada en ese valor; no se modifica nada más del libro. En modo **Automático**, las tablas se calculan junto con las fórmulas, y las fórmulas que leen las celdas de una tabla se actualizan también. Crear una nueva tabla de datos todavía no está disponible. Una tabla que no se puede calcular (una fórmula que depende de la entrada usa una función que JeffCrunch no tiene, o cambia por sí sola, como ALEATORIO o AHORA) conserva los valores de su fichero.

## Calcular a mano

Haga clic en **Modo de cálculo** en la ficha **Fórmulas** para elegir cómo calcula el libro sus fórmulas:

- **Automático** las calcula a medida que cambia las celdas.
- **Automático, excepto tablas de datos** hace lo mismo, pero deja las celdas de las tablas de datos como están hasta que pulse **F9**.
- **Manual** no calcula nada, tablas de datos incluidas, hasta que lo pida. La barra de estado muestra **Desactualizado** después de un cambio. Pulse **F9** (**Recalcular todo**) para calcularlo todo, o **Mayús+F9** (**Recalcular esta hoja**) para calcular las fórmulas de la hoja en la que está, y lo que más estuviera pendiente. Un libro configurado en Manual se abre con los resultados que contiene su fichero. **Calcular antes de guardar**, también en el menú **Modo de cálculo**, viene marcado al principio: al guardar un libro en Manual que está desactualizado, primero se calculan las fórmulas, de modo que el fichero contiene números actualizados. Desactívelo para guardar los números tal como están.

La elección se guarda con el libro, y es un paso que puede deshacer.

## Fórmulas que leen su propio resultado

Una fórmula que lee su propio resultado, directamente o a través de otras celdas, no se puede calcular de la forma habitual. JeffCrunch se lo indica una vez, nombra las celdas y las marca con un contorno en la hoja; estas conservan los valores que tenían. Haga clic en **Referencias circulares** en la ficha **Fórmulas** para ver la lista, y haga doble clic en una celda de ella para ir allí.

Algunos modelos necesitan un círculo así a propósito. Elija **Iterar fórmulas circulares** en el menú **Modo de cálculo**, y las fórmulas se vuelven a calcular una y otra vez, leyendo cada una los valores más recientes de las demás, hasta que ningún valor cambie más de la cantidad que indique o se agote el número de iteraciones permitido.

## Mantener los números como los muestran las celdas

Una celda puede mostrar menos cifras de las que contiene. Si una celda contiene 2,675 y muestra 2,68, las fórmulas que la leen siguen usando 2,675. A veces quiere que usen lo que ve. Entonces elija **Establecer precisión como se muestra** en el menú **Opciones de cálculo** de la ficha **Fórmulas**.

JeffCrunch le avisa antes, porque las cifras ocultas se pierden. Elija **Cancelar** y no cambia nada. Elija **Aceptar** y cada número del libro se redondea a lo que muestra su celda. Una celda que muestra dos decimales conserva dos. Una celda que muestra un porcentaje conserva el porcentaje. Una celda con una fecha conserva el día, y una celda con una hora conserva la menor unidad de tiempo que muestra, como el minuto. Una celda que muestra **General** conserva todas sus cifras. Un libro grande muestra un aviso con un botón **Cancelar** mientras se hace. Cancele mientras el aviso está visible, y no se cambia nada.

Después, cada número que escriba o pegue, y cada resultado de una fórmula, se redondea de la misma manera. Las fórmulas en sí no se cambian. La elección se guarda con el libro.

El redondeo es un paso que puede deshacer, y **Deshacer** devuelve los números a su valor original. Elegir **Establecer precisión como se muestra** de nuevo lo desactiva. Eso solo impide que se redondee a partir de ese momento. Los números que ya estaban redondeados siguen redondeados.

## Ver lo que lee una fórmula

Seleccione una celda y haga clic en **Mostrar lo que alimenta esta celda** para dibujar flechas desde las celdas que lee su fórmula. Haga clic de nuevo para ir un paso más atrás. **Mostrar lo que usa esta celda** dibuja flechas hacia las fórmulas que leen la celda, y cada clic avanza un paso. Una pequeña hoja representa una celda de otra hoja. Se dibujan como máximo 2.000 flechas, y la barra de estado indica cuando había más. **Quitar flechas** las borra, y también las borra cualquier cambio en el libro.

Cuando una celda muestra un error como #¡DIV/0!, selecciónela y haga clic en **Rastrear error**. Se dibujan flechas hacia las celdas que lee y que también muestran un error. Haga clic de nuevo para ir un paso más atrás. Cuando ninguna de las celdas que lee muestra un error, la barra de estado indica que esta celda produce el error, y es la que hay que corregir.

Para recorrer todos los errores de la hoja, haga clic en **Error siguiente** o **Error anterior**. Las celdas que muestran un error se visitan fila por fila, y después de la última se vuelve a empezar por la primera. Cada celda se selecciona, y las flechas se dibujan hasta la celda que produce el error. La barra de estado indica en qué error se encuentra y cuántos hay.

## Comprobación de errores

Elija **Comprobación de errores** en el grupo **Comprobar** de la ficha Fórmulas para revisar la hoja en busca de celdas que puedan estar mal. Busca una fórmula que da un error (como #¡DIV/0!), un número guardado como texto (que las fórmulas pueden omitir), y más: una fórmula que no es igual que las de al lado, una fecha guardada como texto con un año de dos cifras, una fórmula en una celda que no está bloqueada, una fórmula que omite un número junto a las celdas que usa (solo para una función que suma, como **SUMA**, **PROMEDIO**, **CONTAR**, **MIN** o **MAX**, cuando el rango aparece solo como argumento), y una fórmula que usa una celda vacía.

El cuadro de diálogo muestra una celda cada vez, va a esa celda en la hoja y explica en una frase cuál es el problema. **Siguiente** y **Anterior** recorren las celdas. Un pequeño triángulo verde en la esquina superior izquierda de una celda la marca en la hoja. Haga clic en el propio triángulo para abrir un menú junto a la celda: indica cuál es el problema, ofrece la corrección cuando existe, y tiene **Ignorar error** y **Comprobación de errores…**. Cuando la celda se puede arreglar, aparece un botón con la corrección (para una fórmula que omite un número, **Incluir la celda**); cambia la fórmula, y un paso de Deshacer devuelve la fórmula anterior. **Ignorar error** quita la celda de la lista, le quita su triángulo y se guarda en el fichero. Un paso de Deshacer lo devuelve. Si no hay ningún problema, se le indica. Para elegir cuáles de estos se marcan, abra **Opciones** y la página **Fórmulas**: hay una casilla para cada uno, y si desmarca una, ese tipo deja de marcarse. La elección es del programa, así que vale para todas las ventanas abiertas. Una celda puede tener más de un problema, y **Ignorar error** solo ignora el que se muestra.

## Evaluar fórmula

Use esta opción para ver cómo llega una fórmula a su valor. Seleccione una celda que contenga una fórmula y elija **Evaluar fórmula**. La fórmula aparece con la siguiente parte subrayada. Pulse **Evaluar** para calcular esa parte; su valor ocupa su lugar. Las partes van en este orden: cada celda individual de izquierda a derecha, después cada función con sus paréntesis, empezando por la más interna, y después lo que queda. **Reiniciar** empieza de nuevo. Si una parte no se puede calcular, el cuadro de diálogo indica el motivo y deja la fórmula como estaba.

## Ventana de inspección

La **ventana de inspección** es una pequeña ventana que permanece visible mientras trabaja. Muestra las celdas que quiere vigilar, con su hoja, celda, valor y fórmula, incluso cuando están en otra hoja. Seleccione celdas y pulse **Agregar inspección**. Seleccione filas de la lista y pulse **Eliminar inspección**. Haga doble clic en una fila para ir a la celda. Los valores cambian cuando la hoja se vuelve a calcular. La lista se conserva mientras el libro está abierto.

## Fórmulas que llenan las celdas de al lado

Cuando una fórmula da un bloque de valores, el bloque ocupa las celdas de debajo y de al lado de la celda en la que la escribió. Esto se llama desbordamiento. Pruebe =SECUENCIA(3) en A1: A1 muestra 1, y A2 y A3 muestran 2 y 3. Una fórmula se desborda cuando llama a una función que da un bloque (ORDENAR, ORDENARPOR, FILTRAR, UNICOS, SECUENCIA, MATRIZALEATORIA, TRANSPONER, BUSCARX o MMULT), escribe un bloque entre llaves, usa un rango de desbordamiento (más abajo), o hace operaciones aritméticas con un rango, como en =A1:A3*2.

- **Las celdas que ocupa no tienen una fórmula propia.** Muestran los valores. Cuando selecciona una de ellas, la barra de fórmulas muestra en gris la fórmula de la celda que la ocupa. Pulsar Entrar sobre ella tal como está no cambia nada. Para cambiar el bloque, cambie antes esa primera celda.
- **El bloque sigue al resultado.** Si la fórmula da más o menos valores después de que cambie algo que lee, el bloque crece o se reduce, y las fórmulas que leen esas celdas se vuelven a calcular.
- **Una celda en medio da #¡DESBORDAMIENTO!.** Si alguna celda donde iría el bloque contiene un valor o una fórmula, la primera celda muestra **#¡DESBORDAMIENTO!** y no se llena nada. Borre la celda que estorba y el bloque se llena al instante. Escribir en una celda que ocupa el bloque produce lo mismo: la primera celda muestra #¡DESBORDAMIENTO!.
- **Un bloque tiene un límite de tamaño.** Un bloque más grande que la hoja, o de más de 200.000 celdas, da #¡DESBORDAMIENTO!.
- **A1# es el bloque completo.** Escriba la celda que contiene la fórmula y una **#**, como en =SUMA(A1#), para usar el bloque completo tal como está ahora, desde esta hoja o desde otra (=SUMA(Hoja1!A1#)). Si A1 no desborda, A1# es #¡REF!.

Un desbordamiento se guarda con el fichero como matriz dinámica, así que el programa del que procede el fichero también lo muestra como desbordamiento. Al guardar en el formato antiguo .xls o .ods se conservan los valores, y la fórmula como fórmula matricial sobre el bloque cuando el formato antiguo tiene las funciones que usa. Comprobar compatibilidad señala estas fórmulas.

## Dar nombre a valores dentro de una fórmula

**LET** asigna un nombre a un valor en medio de una fórmula, de modo que lo escribe una vez y lo usa de nuevo. Los pares de nombre y valor van primero, y el cálculo, al final. En =LET(precio; B2; impuesto; precio*0,08; precio+impuesto) el nombre **precio** representa el valor de B2, **impuesto** se calcula a partir de él, y la última parte los suma. Un nombre puede usar los nombres anteriores, y un LET puede contener hasta 126 pares. Un nombre solo funciona dentro de su propio LET, y un LET dentro de otro puede usar de nuevo un nombre. Un nombre también puede representar un rango de celdas, como en =LET(filas; B2:B9; SUMA(filas)/CONTAR(filas)).

## Preguntar por una celda o por el libro

**CELDA** informa sobre una celda. Su primer argumento indica qué: "dirección", "fila", "col", "contenido", "tipo", "formato" (cómo se muestran los números, como "F2" para dos decimales, "P0" para un porcentaje, o "D1" para una fecha), "color" (1 cuando los números negativos se muestran en color), "paréntesis" (1 cuando los números positivos se muestran entre paréntesis), "prefijo" (la marca de un texto alineado a la izquierda, a la derecha o centrado), "proteger" (1 cuando la celda está bloqueada), "ancho" (el ancho de la columna en caracteres), y "nombrearchivo" (la carpeta, el fichero y la hoja, como /home/yo/[Presupuesto.xlsx]Hoja1; vacío hasta que se guarda el libro). **INFO** informa sobre el programa y el libro: "directorio", "numarchivo" (cuántas hojas se muestran), "origen", "versiónsO", "recalc" (Automático o Manual), "versión" y "sistema".

## Obtener valores de XML

**FILTRARXML** toma un texto que contiene XML y una XPath, que es una forma breve de indicar qué partes quiere, y devuelve los valores que encuentra. =FILTRARXML(A1; "//articulo/precio") da cada precio del texto de A1, uno debajo de otro. Use "/tienda/articulo[2]/nombre" para el segundo artículo, "//articulo[@id='b']/nombre" para el artículo cuyo id es b, o "count(//articulo)" para saber cuántos hay. Si el texto no es XML válido, o la ruta no es válida, el resultado es #¡VALOR!. Si nada coincide, es #N/A. El XML que tiene declaración de tipo de documento se rechaza.
