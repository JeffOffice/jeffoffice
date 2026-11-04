# Abrir y guardar libros

<!-- keywords: copiar, formato antiguo, opendocument, compatibilidad, nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, importar, exportar, reciente, anclado, arrastrar, soltar, cargando, grande, macros, sin guardar, cambios, cerrar, cadena de cálculo, plantilla, xltx, xltm, no se puede abrir, dañado, cifrado, contraseña, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ots, reparar, abrir y reparar, cadena de cálculo, guardar copia -->

Cuando JeffCrunch se inicia sin ningún archivo que abrir, la ventana contiene un libro nuevo y vacío, con una hoja llamada Libro1. Cada libro nuevo toma el siguiente número (Libro2, Libro3, y así sucesivamente), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, pulse **{key:New}** o haga clic en **Archivo** y luego en **Nuevo**, y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado cede su lugar cuando abre un archivo en su ventana; uno que sí ha modificado se conserva, y el archivo se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, igual que **Guardar como**, y lo guarda como archivo .xlsx. Cerrar un libro nuevo que no ha modificado no pide nada.

Para abrir un libro, pulse **{key:Open}** o haga clic en **Archivo** y luego en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde su administrador de archivos hasta la ventana. Un libro que ya está abierto en una ventana no se abre por segunda vez: esa ventana pasa al frente.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros), y las plantillas **.xltx** y **.xltm** (una plantilla que contiene macros). No ejecuta las macros y las conserva sin cambios: el proyecto de macros, su firma y las partes que los nombran se vuelven a escribir exactamente como estaban. Una plantilla se guarda como plantilla, y un libro con macros como libro con macros, porque el tipo forma parte del archivo.

## Otros tipos de archivos

JeffCrunch también abre libros .xls antiguos, hojas de cálculo OpenDocument (**.ods** y el formato plano **.fods**) y archivos de texto con valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde uno de ellos se guarda como un nuevo archivo **.xlsx** (o como copia .xls o .ods: véase más abajo). Cuando pulsa **{key:Save}**, JeffCrunch se lo indica y pide un nombre, y el archivo que abrió nunca se modifica.

Una hoja de cálculo OpenDocument se lee con sus hojas, valores, fórmulas (con los resultados que guardó el archivo), formatos de número, tipos de letra, rellenos, bordes, celdas combinadas, anchos de columna y alturas de fila, filas y columnas ocultas, paneles inmovilizados y nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior indica cuáles de ellos tiene el archivo, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña pide la contraseña al abrirla: véase [Abrir libros protegidos con contraseña](open-password).

Un archivo de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, punto y coma, tabulaciones o barras verticales) y cómo están codificadas sus letras (UTF-8 o UTF-16, o bien la página de códigos de su idioma), y lee cada valor como lo escribiría en su idioma: un número como 1.234,5 en alemán, un porcentaje, una cantidad de dinero, una fecha con su año (como 15/03/2024, o 15.03.2024 en alemán), una hora, VERDADERO o FALSO (en su idioma, como WAHR y FALSCH en alemán, y en inglés). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos se mantienen como texto, para que un código postal o un número de cuenta no se modifique; lo mismo ocurre con un día y un mes sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, 1 de marzo o 25 dic), porque el año tendría que inventarse, y con textos como el nombre del gen MARCH1. Una hoja contiene 1.048.576 filas y 16.384 columnas; si el archivo tiene más, JeffCrunch abre lo que cabe y se lo indica.

Al abrir un archivo de texto, primero aparece una ventana con sus primeras filas y pregunta cómo debe leerse: qué separa los campos (una coma, un punto y coma, una tabulación, un espacio, una barra vertical o un carácter que usted escriba, con la opción de considerar varios separadores seguidos como uno), qué entrecomilla un campo que contiene un separador, la codificación del texto (Unicode, las páginas de códigos de Windows, las páginas ISO 8859, Mac Roman y las de Asia oriental) y la fila en la que empezar. Encima de cada columna elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar un código de pieza como 007 o un número de cuenta largo tal como está), una **Fecha** en el orden que indique, o **Omitir**. La vista previa sigue cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero se abre el archivo completo. **Archivo > Abrir > Abrir archivo de texto** pregunta incluso cuando el nombre del archivo es inusual.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo** y luego en **Guardar como**, y elija el tipo de archivo **.csv** (o **.tsv** y **.txt**, separados por tabulaciones). Antes de escribir el archivo, JeffCrunch indica lo que tiene su libro que el archivo no podrá contener (las demás hojas, fórmulas, imágenes, gráficos, notas y macros, por ejemplo), y pregunta cómo escribirlo y si desea **Guardar solo valores**, **Guardar como .xlsx** para conservar todo, o **Cancelar**:

- **Separador.** Una coma, un punto y coma, una tabulación, un espacio, una barra vertical o un carácter que usted escriba. Un **.csv** empieza con punto y coma donde su idioma escribe la coma en los números, y con coma en los demás casos; un **.tsv** o **.txt**, con tabulación.
- **Codificación del texto.** UTF-8 con marca (para que otros programas reconozcan bien las letras acentuadas), UTF-8 sin marca, UTF-16, o una página de códigos como Europa occidental (1252), las páginas ISO 8859 o Mac Roman. Una letra que la página de códigos no tiene se escribe como un signo de interrogación, y la barra de estado indica cuántas son.
- **Finales de línea.** Windows (retorno de carro y avance de línea), Linux y Mac (avance de línea), o Mac antiguo (retorno de carro).
- **Celdas.** **Como se muestran en las celdas** escribe lo que ve. **Como se almacenan** escribe todos los dígitos de un número (0,1 y no 10 %), y una fecha como texto ISO, como 2026-03-04.

El texto se pone entre comillas cuando contiene el separador, una comilla, un salto de línea o un espacio al principio o al final. Las demás hojas, las fórmulas, los formatos y los ajustes no están en el archivo, y el libro no se modifica al escribirlo.

Un libro que procede de un archivo de texto se vuelve a escribir con la forma que tenía el archivo: el mismo separador, los mismos finales de línea y la misma codificación. Si lo guarda como copia sin cambiar ninguna celda, la copia es el mismo archivo, byte a byte, y no se pregunta nada.

## Guardar una copia como .xls o .ods

Para hacer una copia que puedan abrir programas más antiguos, o programas que leen OpenDocument, haga clic en **Archivo** y luego en **Guardar como**, y elija el tipo de archivo **.xls** u **.ods**. La copia se hace a partir de su libro tal como está ahora, con los cambios que todavía no ha guardado, y contiene:

- los valores, con sus textos, números, valores lógicos y errores;
- las fórmulas, con los resultados que tienen ahora (una fórmula matricial sigue siendo una fórmula matricial);
- los formatos de número, tipos de letra, rellenos, bordes, alineación, y si una celda está bloqueada;
- las celdas combinadas, los anchos de columna y las alturas de fila, las filas y columnas ocultas, los esquemas de filas y columnas, y los paneles inmovilizados;
- las hojas, en su orden, con sus nombres y cuáles están ocultas, y los nombres que definió (y las áreas de impresión);
- los vínculos, y cómo se imprime cada hoja (el papel, los márgenes, la orientación, la escala, los encabezados y pies de página, y los saltos de página).

Un archivo **.xls** también contiene tablas de datos, y conserva los ajustes de cálculo del libro. Una copia no puede contener todo. Antes de que JeffCrunch escriba el archivo, indica lo que su libro tiene y el archivo no podrá contener: gráficos, imágenes y otros dibujos, notas, tablas dinámicas, tablas, formatos condicionales, validación de datos, filtros, protección de hoja, controles de formulario, macros y vínculos a otros libros (las fórmulas que los usan conservan los valores que tenían). Una fórmula que la forma antigua no puede expresar (una función que no tiene, o una celda más allá de la columna IV o de la fila 65.536) se escribe como el valor al que llegaba, y el mensaje cuenta cuántas son. La forma antigua también se limita a 65.536 filas, 256 columnas, 255 hojas y 32.767 caracteres en una celda, y a unos 4.000 formatos de celda y 56 colores: lo que no cabe se recorta, y el mensaje indica cuánto. Después puede elegir **Guardar copia**, **Guardar como .xlsx** para conservar todo, o **Cancelar**.

La copia no modifica su libro, y el libro no se marca como guardado. El archivo que abrió no se escribe nunca: dé a la copia otro nombre. Una copia de un archivo **.xls** u **.ods** que abrió puede guardarse en el otro tipo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffCrunch escribe el libro en ese archivo, con los cambios que no ha guardado, y usted sigue trabajando en el libro que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban. Puede elegir cualquier tipo de archivo que ofrece **Guardar como**. Una copia nunca se escribe sobre el archivo propio del libro.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones**, y en la página **General** active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffCrunch copia primero el archivo antiguo en otro llamado "Copia de" seguido de su nombre, en la misma carpeta. Si no se puede hacer la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo se mantiene como estaba.

## Guardar como plantilla

Una plantilla es un libro que usa como punto de partida. Puede contener fórmulas, estilos y hojas. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un archivo **.xltx**) se guarda en **Mis plantillas**, y su libro no se modifica. En **Guardar como** también puede elegir **Plantillas (*.xltx)** u **Plantillas de hoja de cálculo de OpenDocument (*.ots)** para guardar un archivo de plantilla donde desee.

Para empezar un libro a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un libro nuevo con las celdas y hojas de la plantilla. No tiene todavía un archivo propio, así que **Guardar** pide un nombre, y la plantilla no se modifica. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista Reciente.

- **Abrir como solo lectura** abre el libro solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no cambiar ninguna celda. La barra de título indica "Solo lectura". Si pulsa **Guardar**, JeffCrunch pide un nombre nuevo, para que el archivo que abrió no se modifique.
- **Abrir como copia** abre el libro sin archivo propio. **Guardar** pregunta dónde colocarlo. El archivo que abrió nunca se modifica.
- **Abrir en una nueva ventana** abre el libro en una ventana propia, aunque la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un archivo .xlsx dañado. Véase más abajo.

## Abrir y reparar

A veces un archivo .xlsx está dañado. Una descarga que se interrumpió a medias, o un disco con errores. Un archivo .xlsx es un conjunto de partes más pequeñas, como las hojas, los estilos y el texto compartido. **Abrir y reparar** lee todas las partes que puede. Una parte que se cortó a mitad se conserva hasta el punto en que se cortó, así que puede obtener las primeras filas de una hoja. Una parte que no se puede leer se omite. Si la lista de partes o los vínculos entre ellas se han perdido, JeffCrunch los vuelve a crear.

El libro reparado se abre sin archivo propio, y con un nombre como "Presupuesto reparado". Un mensaje le indica qué se conservó hasta un corte y qué se omitió. El archivo dañado no se modifica. Guarde el libro reparado con un nombre nuevo y compruebe los números antes de fiarse de ellos.

## Libros grandes

JeffCrunch lee un libro en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi de inmediato, incluso cuando el libro es muy grande. La barra de estado, en la parte inferior izquierda, indica lo que hace: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta el momento, **Calculando** con un porcentaje, y después **Listo**.

Puede desplazarse, seleccionar y copiar mientras se lee. Las pestañas de las demás hojas se activan cuando se leen. Un libro dañado se abre hasta donde puede leerse, y un mensaje indica qué se omitió.

## Si un archivo no se abre

Cuando JeffCrunch no puede abrir un archivo, lo explica con palabras sencillas y deja los libros que tiene abiertos como estaban. Distingue un archivo vacío de uno que no es un libro en absoluto (un documento PDF, una imagen, un documento de texto o una presentación, por ejemplo); un paquete zip cortado o dañado, lo que suele significar que una descarga o una copia se interrumpió, así que cópielo de nuevo desde el original; un libro protegido con contraseña, que la pide (véase [Abrir libros protegidos con contraseña](open-password)); y un archivo compuesto que no contiene ningún libro.

## Libros recientes

La página **Abrir** de la pantalla Archivo muestra los libros que abrió recientemente. Haga clic con el botón derecho en uno para **anclarlo** a la parte superior de la lista o para quitarlo de ella.

## Guardar

Pulse **{key:Save}**, o haga clic en **Archivo** y luego en **Guardar**, para guardar los cambios en el libro que abrió. Para conservar el original y crear un archivo nuevo, haga clic en **Archivo** y luego en **Guardar como**, y elija un nombre. Un libro con macros se guarda como archivo **.xlsm**, una plantilla como archivo **.xltx** o **.xltm**, y cualquier otro como archivo **.xlsx**. No se acepta un nombre que termine en otro tipo de libro, y el mensaje indica qué terminación usar; los nombres que terminan en **.xls**, **.ods**, **.csv**, **.tsv** y **.txt** crean copias, como se ha descrito antes.

JeffCrunch guarda solo lo que cambió. Todo lo demás del archivo, como las demás hojas, los gráficos, las imágenes, las macros y los ajustes que JeffCrunch no muestra, se copia tal como estaba, byte a byte. En una hoja que cambió, solo se vuelven a escribir las celdas que cambió, junto con las fórmulas que dependen de ellas, que obtienen sus nuevos valores. Si no cambió ninguna celda, el archivo no se toca.

Si otro programa modificó el archivo después de abrirlo, JeffCrunch no lo sobrescribe sin preguntar, porque se perderían los cambios del otro programa. Pregunta si desea **Guardar una copia**, **Sobrescribir** el archivo o **Cancelar**.

El archivo nuevo se escribe primero junto al antiguo, y ocupa el lugar de este solo cuando está completo. Si algo sale mal, aparece un mensaje, y el archivo que había se mantiene como estaba.

Si las fórmulas todavía se están calculando cuando guarda, JeffCrunch espera a que terminen, para que el archivo contenga sus resultados. Si pulsa **Esc** para detener el cálculo mientras tanto, el guardado se cancela, y la barra de estado lo indica; guarde de nuevo cuando lo desee. JeffCrunch deja fuera del archivo una cosa: la lista del orden en que un programa calculó por última vez las fórmulas (la cadena de cálculo). Un programa que abra el libro después la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el archivo pide a ese programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro que tiene cambios sin guardar, JeffCrunch pregunta si desea **Guardar** los cambios, **No guardar** o **Cancelar** y seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.
