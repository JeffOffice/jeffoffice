# Abrir y guardar libros
<!-- keywords: nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, hoja de cálculo, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, separado por comas, importar, exportar, recientes, anclado, arrastrar, colocar, soltar, carga, cargando, grande, macros, sin guardar, cambios, cerrar, copia, cadena de cálculo, plantilla, xltx, xltm, no se puede abrir, dañado, cifrado, contraseña -->

Cuando JeffCrunch se inicia sin ningún archivo que abrir, la ventana contiene un libro nuevo y vacío con una hoja, llamado Libro1. Cada libro nuevo recibe el número siguiente (Libro2, Libro3, etc.), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, presione **{key:New}**, o haga clic en **Archivo**, después en **Nuevo** y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado deja su lugar cuando abre un archivo en su ventana; uno que ha modificado se mantiene, y el archivo se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, igual que **Guardar como**, y lo guarda como archivo .xlsx. Al cerrar un libro nuevo que no ha modificado no se pregunta nada.

Para abrir un libro, presione **{key:Open}**, o haga clic en **Archivo** y después en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde el explorador de archivos hasta la ventana. Un libro que ya está abierto en una ventana no se abre por segunda vez: esa ventana pasa a primer plano.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros), y las plantillas **.xltx** y **.xltm** (una plantilla que contiene macros). No ejecuta las macros y las conserva intactas: el proyecto de macros, su firma y las partes que las nombran se vuelven a escribir exactamente como estaban. Una plantilla se guarda como plantilla, y un libro con macros como libro con macros, porque el tipo forma parte del archivo.

## Otros tipos de archivos

JeffCrunch también abre libros **.xls** antiguos, hojas de cálculo OpenDocument (**.ods** y la versión plana **.fods**) y archivos de texto de valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde uno de ellos se guarda como un archivo **.xlsx** nuevo: cuando presiona **{key:Save}**, JeffCrunch se lo indica y le pide un nombre, y el archivo que abrió no se modifica nunca.

De una hoja de cálculo OpenDocument se leen sus hojas, valores, fórmulas (con los resultados que guardaba el archivo), formatos de número, fuentes, rellenos, bordes, celdas combinadas, anchos de columna y altos de fila, filas y columnas ocultas, paneles inmovilizados y nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior le indica cuáles de ellos tiene el archivo, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña no se abre: ábrala en el programa con el que se creó y guarde una copia sin contraseña.

Un archivo de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, puntos y comas, tabulaciones o barras verticales) y cómo están codificados sus caracteres (UTF-8 o UTF-16, o, si no, la página de códigos de su idioma), y lee cada valor tal como lo escribiría en su idioma: un número como 1.234,5 en español, un porcentaje, un importe de dinero, una fecha con su año (como 15/03/2024, o 3/15/2024 en inglés), una hora, VERDADERO o FALSO (en su idioma y en inglés, TRUE y FALSE). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos se mantienen como texto, para que no se modifique un código postal o un número de cuenta, y lo mismo ocurre con un día y un mes sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, 1 de marzo o 25 dic), porque habría que inventarse el año, y con textos como el nombre del gen MARCH1. Una hoja tiene 1.048.576 filas y 16.384 columnas; si el archivo tiene más, JeffCrunch abre lo que cabe y se lo indica.

Al abrir un archivo de texto, primero aparece una ventana que muestra sus primeras filas y le pregunta cómo debe leerse: qué separa los campos (cualquier combinación de coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba, y si lo desea, varios seguidos cuentan como uno solo), qué carácter entrecomilla un campo que contiene un separador, la codificación de texto y la fila en la que empezar. Encima de cada columna elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar tal cual un número de pieza como 007 o un número de cuenta largo), una **Fecha** en el orden que indique, u **Omitir**. La vista previa refleja cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero se abre el archivo completo. **Archivo > Abrir > Abrir un archivo de texto** muestra esta ventana incluso cuando el nombre del archivo tiene una extensión poco habitual.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo**, después en **Guardar como**, y elija el tipo de archivo **.csv**. JeffCrunch escribe los valores tal como los muestran las celdas, en UTF-8, con un punto y coma entre ellos si su idioma usa la coma en los números y con una coma en los demás casos, y pone el texto entre comillas cuando es necesario. Las demás hojas, las fórmulas, los formatos y la configuración no se incluyen en un archivo .csv, y el libro no se modifica al escribirlo. Antes de escribir el archivo, JeffCrunch le indica qué tiene el libro que el archivo no podrá contener (por ejemplo, las demás hojas, las fórmulas, las imágenes, los gráficos, las notas y las macros) y le pregunta si desea **Guardar solo valores**, **Guardar como .xlsx** en su lugar y conservarlo todo, o **Cancelar**.

## Libros grandes

JeffCrunch lee los libros en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi de inmediato, aunque el libro sea muy grande. La barra de estado, en la parte inferior izquierda, le indica lo que está haciendo: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta el momento, **Calculando** con un porcentaje y, por último, **Listo**.

Puede desplazarse, seleccionar y copiar mientras se lee. Las pestañas de las demás hojas se activan a medida que se leen. Un libro dañado se abre hasta donde se puede leer, y un mensaje le indica qué se ha omitido.

## Si un archivo no se abre

Cuando JeffCrunch no puede abrir un archivo, le explica el motivo con palabras sencillas y deja los libros que tiene abiertos tal como están. Distingue un archivo vacío de uno que no es un libro (por ejemplo, un documento PDF, una imagen, un documento de texto o una presentación); un paquete zip incompleto o dañado, lo que suele indicar que se interrumpió una descarga o una copia, por lo que debe volver a copiarlo desde el original; un libro cifrado con contraseña, que JeffCrunch todavía no puede abrir; y un archivo compuesto que no contiene ningún libro.

## Libros recientes

La página **Abrir** de la pantalla Archivo muestra los libros que ha abierto recientemente. Haga clic con el botón derecho en uno para **anclarlo** a la parte superior de la lista o para quitarlo de la lista.

## Guardar

Presione **{key:Save}**, o haga clic en **Archivo** y después en **Guardar**, para guardar los cambios en el libro que abrió. Para conservar el original y crear un archivo nuevo, haga clic en **Archivo** y después en **Guardar como**, y elija un nombre. Un libro con macros se guarda como archivo **.xlsm**, una plantilla como archivo **.xltx** o **.xltm**, y cualquier otro como archivo **.xlsx**. No se acepta un nombre que termine en otra extensión, y el mensaje le indica qué extensión debe usar.

JeffCrunch guarda solo lo que ha cambiado. Todo lo demás del archivo, como las demás hojas, los gráficos, las imágenes, las macros y la configuración que JeffCrunch no muestra, se copia tal como estaba, byte a byte. En una hoja que haya modificado, solo se vuelven a escribir las celdas que cambió, junto con las fórmulas que dependen de ellas, que reciben sus nuevos valores. Si no cambió ninguna celda, el archivo no se toca.

Si otro programa modificó el archivo después de que usted lo abriera, JeffCrunch no lo sobrescribe sin preguntar, porque se perderían los cambios del otro programa. Le pregunta si desea **Guardar una copia**, **Sobrescribir** el archivo o **Cancelar**.

El archivo nuevo se escribe primero junto al anterior, y solo lo sustituye cuando está completo. Si algo sale mal, verá un mensaje y el archivo que había seguirá como estaba.

Si las fórmulas todavía se están calculando cuando guarda, JeffCrunch espera a que terminen, para que el archivo contenga sus resultados. Si mientras tanto presiona **Esc** para detener el cálculo, se cancela el guardado y la barra de estado lo indica; vuelva a guardar cuando quiera. JeffCrunch deja una sola cosa fuera del archivo: la lista del orden en que un programa calculó por última vez las fórmulas (la cadena de cálculo). El siguiente programa que abra el libro la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el archivo pide a ese programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro que tiene cambios sin guardar, JeffCrunch le pregunta si desea **Guardar** los cambios, **No guardar** o **Cancelar** para seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.
