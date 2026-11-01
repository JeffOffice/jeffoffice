# Abrir y guardar libros
<!-- keywords: copia, formato antiguo, opendocument, compatibilidad, nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, importar, exportar, recientes, anclado, arrastrar, colocar, cargando, grande, enorme, macros, sin guardar, cambios, cerrar, copia, cadena de cálculo, plantilla, xltx, xltm, no se puede abrir, dañado, cifrado, contraseña -->

Cuando JeffCrunch se inicia sin ningún archivo que abrir, la ventana contiene un libro nuevo y vacío con una hoja, llamado Libro1. Cada libro nuevo recibe el número siguiente (Libro2, Libro3, etc.), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, presione **{key:New}**, o haga clic en **Archivo**, luego en **Nuevo**, y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado deja su sitio cuando abre un archivo en su ventana; uno que ha modificado se mantiene, y el archivo se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, como hace **Guardar como**, y lo guarda como archivo .xlsx. Al cerrar un libro nuevo que no ha modificado, no se pregunta nada.

Para abrir un libro, presione **{key:Open}**, o haga clic en **Archivo** y luego en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde el administrador de archivos hasta la ventana. Un libro que ya está abierto en una ventana no se abre por segunda vez: esa ventana pasa al frente.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros), y las plantillas **.xltx** y **.xltm** (una plantilla que contiene macros). No ejecuta las macros, y las conserva intactas: el proyecto de macros, su firma y las partes que hacen referencia a ellas se vuelven a escribir exactamente como estaban. Una plantilla se guarda como plantilla, y un libro con macros como libro con macros, porque el tipo forma parte del archivo.

## Otros tipos de archivos

JeffCrunch también abre libros **.xls** antiguos, hojas de cálculo OpenDocument (**.ods** y el formato plano **.fods**) y archivos de texto de valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde uno de ellos se guarda como un archivo **.xlsx** nuevo (o como una copia **.xls** u **.ods**: vea más abajo): cuando presiona **{key:Save}**, JeffCrunch se lo indica y le pide un nombre, y el archivo que abrió nunca se modifica.

Una hoja de cálculo OpenDocument se lee con sus hojas, valores, fórmulas (con los resultados que guardaba el archivo), formatos de número, fuentes, rellenos, bordes, celdas combinadas, anchos de columna y altos de fila, filas y columnas ocultas, paneles inmovilizados y nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior le indica cuáles de ellos tiene el archivo, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña no se abre: ábrala en el programa con el que se creó y guarde una copia sin la contraseña.

Un archivo de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, puntos y coma, tabulaciones o barras verticales) y cómo están codificadas sus letras (UTF-8 o UTF-16, o bien la página de códigos de su idioma), y lee cada valor tal como lo escribiría en su idioma: un número como 1.234,5 en alemán, un porcentaje, un importe de dinero, una fecha con su año (como 3/15/2024, o 15.03.2024 en alemán), una hora, VERDADERO o FALSO (en su idioma, como WAHR y FALSCH en alemán, y en inglés). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos se mantienen como texto, para que no se modifique un código postal o un número de cuenta, y lo mismo ocurre con un día y un mes sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, March 1 o Dec 25), porque habría que inventarse el año, y con textos como el nombre de gen MARCH1. Una hoja tiene 1.048.576 filas y 16.384 columnas; si el archivo tiene más, JeffCrunch abre lo que cabe y se lo indica.

Cuando abre un archivo de texto, primero aparece una ventana que muestra sus primeras filas y le pregunta cómo debe leerse: qué separa los campos (cualquiera de estos: coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba, y, si lo desea, varios seguidos se cuentan como uno solo), qué carácter delimita un campo que contiene un separador, la codificación de texto (Unicode, las páginas de códigos de Windows, las páginas ISO 8859, Mac Roman y las de Asia oriental) y la fila en la que empezar. Encima de cada columna elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar tal cual un número de pieza como 007 o un número de cuenta largo), **Fecha** en el orden que indique, u **Omitir**. La vista previa refleja cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero se abre el archivo completo. **Archivo > Abrir > Abrir un archivo de texto** pregunta incluso cuando el nombre del archivo es poco habitual.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo**, luego en **Guardar como**, y elija el tipo de archivo **.csv** (o **.tsv** y **.txt**, delimitados por tabulaciones). Antes de escribir el archivo, JeffCrunch le indica lo que tiene su libro que el archivo no podrá contener (las demás hojas, las fórmulas, las imágenes, los gráficos, las notas y las macros, por ejemplo), le pregunta cómo escribirlo y si desea **Guardar solo valores**, **Guardar como .xlsx** en su lugar para conservarlo todo, o **Cancelar**:

- **Separador.** Coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba. Un archivo **.csv** empieza con punto y coma si su idioma usa la coma en los números, y con coma en caso contrario; un **.tsv** o **.txt**, con tabulación.
- **Codificación de texto.** UTF-8 con marca (para que otros programas interpreten bien las letras acentuadas), UTF-8 sin ella, UTF-16 o una página de códigos como Europa occidental (1252), las páginas ISO 8859 o Mac Roman. Una letra para la que la página de códigos no tiene byte se escribe como signo de interrogación, y la barra de estado indica cuántas hay.
- **Fin de línea.** Windows (retorno de carro y avance de línea), Linux y Mac (avance de línea) o Mac antiguo (retorno de carro).
- **Celdas.** **Como se muestran en las celdas** escribe lo que ve. **Como se almacenan** escribe todos los dígitos de un número (0,1 y no 10 %), y una fecha como texto ISO, por ejemplo 2026-03-04.

El texto se escribe entre comillas cuando contiene el separador, unas comillas, un salto de línea o un espacio en cualquiera de sus extremos. Las demás hojas, las fórmulas, los formatos y la configuración no se incluyen en el archivo, y escribirlo no modifica su libro.

Un libro procedente de un archivo de texto se vuelve a escribir con la forma que tenía el archivo: el mismo separador, el mismo fin de línea y la misma codificación. Si lo guardó como copia sin cambiar ninguna celda, la copia es el mismo archivo, byte a byte, y no se pregunta nada.

## Guardar una copia como .xls u .ods

Para crear una copia que puedan abrir programas antiguos o programas que lean OpenDocument, haga clic en **Archivo**, luego en **Guardar como**, y elija el tipo de archivo **.xls** u **.ods**. La copia se crea a partir del libro tal como está ahora, con los cambios que aún no ha guardado, y contiene:

- los valores, con sus textos, números, valores lógicos y errores;
- las fórmulas, con los resultados que tienen ahora (una fórmula matricial sigue siendo matricial);
- los formatos de número, las fuentes, los rellenos, los bordes, la alineación y si una celda está bloqueada;
- las celdas combinadas, los anchos de columna y altos de fila, las filas y columnas ocultas, los esquemas de filas y columnas, y los paneles inmovilizados;
- las hojas, en su orden, con sus nombres y cuáles están ocultas, y los nombres que haya definido (y las áreas de impresión);
- los vínculos y la forma de imprimir cada hoja (el papel, los márgenes, la orientación, la escala, los encabezados y pies de página, y los saltos de página).

Un archivo **.xls** también contiene las tablas de datos y conserva la configuración de cálculo del libro. Una copia no puede contenerlo todo. Antes de escribir el archivo, JeffCrunch le indica lo que tiene su libro que el archivo no podrá contener: gráficos, imágenes y otros dibujos, notas, tablas dinámicas, tablas, formatos condicionales, validación de datos, filtros, protección de hoja, controles de formulario, macros y vínculos a otros libros (las fórmulas que los usan conservan los valores que tenían). Una fórmula que el formato antiguo no puede expresar (una función que no tiene, o una celda más allá de la columna IV o de la fila 65.536) se escribe como el valor que daba, y el mensaje indica cuántas hay. El formato antiguo también se limita a 65.536 filas, 256 columnas, 255 hojas y 32.767 caracteres por celda, y a unos 4.000 formatos de celda y 56 colores: lo que no cabe se recorta, y el mensaje indica cuánto. Después puede elegir **Guardar copia**, **Guardar como .xlsx** para conservarlo todo, o **Cancelar**.

La copia no modifica su libro, y este no se marca como guardado. El archivo que abrió nunca se sobrescribe: asigne otro nombre a la copia. Una copia de un archivo **.xls** u **.ods** que haya abierto puede guardarse con el otro tipo.

## Libros grandes

JeffCrunch lee los libros en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi al instante, aunque el libro sea muy grande. La barra de estado, en la parte inferior izquierda, le indica lo que está haciendo: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta el momento, **Calculando** con un porcentaje y, por último, **Listo**.

Mientras lee, puede desplazarse, seleccionar y copiar. Las pestañas de las demás hojas se activan a medida que se leen. Un libro dañado se abre hasta donde se puede leer, y un mensaje indica lo que se ha omitido.

## Si un archivo no se abre

Cuando JeffCrunch no puede abrir un archivo, explica el motivo con palabras sencillas y deja tal como están los libros que tenga abiertos. Distingue un archivo vacío de uno que no es un libro en absoluto (un documento PDF, una imagen, un documento de texto o una presentación, por ejemplo); un paquete zip incompleto o dañado, lo que suele significar que se interrumpió una descarga o una copia, por lo que debe volver a copiarlo desde el original; un libro cifrado con contraseña, que JeffCrunch todavía no puede abrir; y un archivo compuesto que no contiene ningún libro.

## Libros recientes

La página **Abrir** de la pantalla Archivo muestra los libros que ha abierto últimamente. Haga clic con el botón derecho en uno para **anclarlo** al principio de la lista o para quitarlo de la lista.

## Guardar

Presione **{key:Save}**, o haga clic en **Archivo** y luego en **Guardar**, para guardar los cambios en el libro que abrió. Para conservar el original y crear un archivo nuevo, haga clic en **Archivo** y luego en **Guardar como**, y elija un nombre. Un libro con macros se guarda como archivo **.xlsm**, una plantilla como archivo **.xltx** o **.xltm**, y cualquier otro como archivo **.xlsx**. No se acepta un nombre que termine con la extensión de otro tipo de libro, y el mensaje indica qué extensión debe usar; los nombres que terminan en **.xls**, **.ods**, **.csv**, **.tsv** y **.txt** crean copias, como se ha descrito más arriba.

JeffCrunch solo guarda lo que ha cambiado. Todo lo demás del archivo, como las otras hojas, los gráficos, las imágenes, las macros y la configuración que JeffCrunch no muestra, se copia tal como estaba, byte a byte. En una hoja que ha modificado, solo se vuelven a escribir las celdas que ha cambiado, junto con las fórmulas que dependen de ellas, que reciben sus nuevos valores. Si no ha cambiado ninguna celda, el archivo no se toca.

Si otro programa modificó el archivo después de que usted lo abriera, JeffCrunch no lo sobrescribe sin preguntar, porque se perderían los cambios del otro programa. Le pregunta si desea **Guardar una copia**, **Sobrescribir** el archivo o **Cancelar**.

El archivo nuevo se escribe primero junto al antiguo, y solo ocupa su lugar cuando está completo. Si algo sale mal, aparece un mensaje, y el archivo que había queda como estaba.

Si las fórmulas todavía se están calculando cuando guarda, JeffCrunch espera a que terminen, para que el archivo contenga sus resultados. Si mientras tanto presiona **Esc** para detener el cálculo, el guardado se cancela y la barra de estado lo indica; vuelva a guardar cuando quiera. JeffCrunch omite una cosa en el archivo: la lista del orden en que un programa calculó las fórmulas por última vez (la cadena de cálculo). El programa que abra el libro a continuación la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el archivo pide a ese programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro con cambios sin guardar, JeffCrunch le pregunta si desea **Guardar** los cambios, **No guardar** los cambios o **Cancelar** y seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.
