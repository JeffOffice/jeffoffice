# Abrir y guardar libros
<!-- keywords: nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, hoja de cálculo, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, separado por comas, importar, exportar, recientes, anclado, arrastrar, colocar, soltar, carga, cargando, grande, macros, sin guardar, cambios, cerrar, copia, cadena de cálculo, plantilla, xltx, xltm, no se puede abrir, dañado, cifrado, contraseña -->

Cuando JeffCrunch se inicia sin ningún archivo que abrir, la ventana contiene un libro nuevo y vacío con una hoja, llamado Libro1. Cada libro nuevo recibe el número siguiente (Libro2, Libro3, etc.), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, presione **{key:New}**, o haga clic en **Archivo**, luego en **Nuevo**, y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado deja su sitio cuando abre un archivo en su ventana; uno que ha modificado se mantiene, y el archivo se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, como hace **Guardar como**, y lo guarda como archivo .xlsx. Al cerrar un libro nuevo que no ha modificado, no se pregunta nada.

Para abrir un libro, presione **{key:Open}**, o haga clic en **Archivo** y luego en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde el explorador de archivos hasta la ventana. Un libro que ya está abierto en una ventana no se abre por segunda vez: esa ventana pasa al frente.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros), y las plantillas **.xltx** y **.xltm** (una plantilla que contiene macros). No ejecuta macros y las mantiene intactas: el proyecto de macros, su firma y las partes que hacen referencia a ellas se vuelven a escribir exactamente como estaban. Una plantilla se guarda como plantilla, y un libro con macros como libro con macros, porque el tipo forma parte del archivo.

## Otros tipos de archivo

JeffCrunch también abre libros **.xls** antiguos, hojas de cálculo OpenDocument (**.ods** y el formato plano **.fods**) y archivos de texto de valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde uno de ellos se guarda como un archivo **.xlsx** nuevo: al presionar **{key:Save}**, JeffCrunch se lo indica y le pide un nombre, y el archivo que abrió nunca se modifica.

De una hoja de cálculo OpenDocument se leen sus hojas, valores, fórmulas (con los resultados que guardaba el archivo), formatos de número, fuentes, rellenos, bordes, celdas combinadas, anchos de columna y altos de fila, filas y columnas ocultas, paneles inmovilizados y nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior le indica cuáles de ellos contiene el archivo, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña no se abre: ábrala en el programa con el que se creó y guarde una copia sin la contraseña.

Un archivo de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, puntos y comas, tabulaciones o barras verticales) y cómo están codificadas sus letras (UTF-8 o UTF-16, o bien la página de códigos de su idioma), y lee cada valor tal como lo escribiría en su idioma: un número como 1.234,5 en español, un porcentaje, un importe de dinero, una fecha con su año (como 15/03/2024, o 3/15/2024 en inglés), una hora, VERDADERO o FALSO (en su idioma y en inglés, TRUE y FALSE). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos se quedan como texto, para que no se modifique un código postal o un número de cuenta, y lo mismo ocurre con un día y un mes sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, 1 de marzo o Dec 25), porque habría que inventar el año, y con textos como el nombre de gen MARCH1. Una hoja contiene 1.048.576 filas y 16.384 columnas; si el archivo tiene más, JeffCrunch abre lo que cabe y se lo indica.

Al abrir un archivo de texto, primero aparece una ventana que muestra sus primeras filas y pregunta cómo se debe leer: qué separa los campos (una coma, un punto y coma, una tabulación, un espacio, una barra o un carácter que escriba, con la opción de considerar varios seguidos como uno solo), qué delimita un campo que contiene un separador, la codificación de texto (Unicode, las páginas de códigos de Windows, las páginas ISO 8859, Mac Roman y las de Asia oriental) y la fila en la que empezar. Encima de cada columna se elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar tal cual una referencia como 007 o un número de cuenta largo), una **Fecha** en el orden que indique u **Omitir**. La vista previa refleja cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero se abre el archivo completo. **Archivo > Abrir > Abrir un archivo de texto** pregunta incluso cuando el nombre del archivo es poco habitual.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo**, luego en **Guardar como**, y elija el tipo de archivo **.csv** (o **.tsv** y **.txt**, separados por tabulaciones). Antes de escribir el archivo, JeffCrunch indica qué tiene el libro que el archivo no podrá contener (las demás hojas, fórmulas, imágenes, gráficos, notas y macros, por ejemplo), pregunta cómo escribirlo y si desea **Guardar solo valores**, **Guardar como .xlsx** en su lugar para conservarlo todo, o **Cancelar**:

- **Separador.** Una coma, un punto y coma, una tabulación, un espacio, una barra vertical o un carácter que escriba. Un archivo **.csv** usa de entrada un punto y coma si su idioma escribe una coma en los números, y una coma en los demás casos; un **.tsv** o **.txt**, una tabulación.
- **Codificación de texto.** UTF-8 con marca (para que otros programas interpreten bien las letras acentuadas), UTF-8 sin ella, UTF-16 o una página de códigos como Europa occidental (1252), las páginas ISO 8859 o Mac Roman. Una letra para la que la página de códigos no tiene byte se escribe como un signo de interrogación, y la barra de estado indica cuántas hay.
- **Fin de línea.** Windows (retorno de carro y avance de línea), Linux y Mac (avance de línea) o Mac antiguo (retorno de carro).
- **Celdas.** **Como se muestran en las celdas** escribe lo que ve. **Como se almacenan** escribe todos los dígitos de un número (0,1 y no 10 %), y una fecha como texto ISO, por ejemplo 2026-03-04.

El texto se escribe entre comillas cuando contiene el separador, una comilla, un salto de línea o un espacio en cualquiera de sus extremos. Las demás hojas, las fórmulas, los formatos y la configuración no se incluyen en el archivo, y escribirlo no modifica el libro.

Un libro que procede de un archivo de texto se vuelve a escribir con la forma que tenía el archivo: el mismo separador, los mismos finales de línea y la misma codificación. Si lo guardó como copia sin cambiar ninguna celda, la copia es el mismo archivo, byte a byte, y no se pregunta nada.

## Libros grandes

JeffCrunch lee los libros en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi al instante, incluso si el libro es muy grande. La barra de estado, en la parte inferior izquierda, indica lo que está haciendo: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta el momento, **Calculando** con un porcentaje y, por último, **Listo**.

Puede desplazarse, seleccionar y copiar mientras se lee. Las pestañas de las demás hojas se activan a medida que se leen. Un libro dañado se abre hasta donde se puede leer, y un mensaje indica qué se omitió.

## Si un archivo no se abre

Cuando JeffCrunch no puede abrir un archivo, explica el motivo con palabras sencillas y deja como están los libros que tenga abiertos. Distingue un archivo vacío de uno que no es un libro en absoluto (un documento PDF, una imagen, un documento de texto o una presentación, por ejemplo); un paquete zip incompleto o dañado, lo que suele indicar que se interrumpió una descarga o una copia, por lo que debe copiarlo de nuevo desde el original; un libro cifrado con contraseña, que JeffCrunch todavía no puede abrir; y un archivo compuesto que no contiene ningún libro.

## Libros recientes

La página **Abrir** de la pantalla Archivo muestra los libros que ha abierto últimamente. Haga clic con el botón derecho en uno para **anclarlo** al principio de la lista o para quitarlo de ella.

## Guardar

Presione **{key:Save}**, o haga clic en **Archivo** y luego en **Guardar**, para guardar los cambios en el libro que abrió. Para conservar el original y crear un archivo nuevo, haga clic en **Archivo** y luego en **Guardar como**, y elija un nombre. Un libro con macros se guarda como archivo **.xlsm**, una plantilla como archivo **.xltx** o **.xltm**, y cualquier otro como archivo **.xlsx**. No se acepta un nombre que termine con otra extensión, y el mensaje indica qué extensión usar.

JeffCrunch guarda solo lo que ha cambiado. Todo lo demás del archivo, como las otras hojas, los gráficos, las imágenes, las macros y la configuración que JeffCrunch no muestra, se copia tal como estaba, byte a byte. En una hoja modificada, solo se vuelven a escribir las celdas que cambió, junto con las fórmulas que dependen de ellas, que reciben sus nuevos valores. Si no cambió ninguna celda, el archivo no se toca.

Si otro programa modificó el archivo después de que usted lo abriera, JeffCrunch no lo sobrescribe sin preguntar, porque se perderían los cambios del otro programa. Pregunta si desea **Guardar una copia**, **Sobrescribir** el archivo o **Cancelar**.

El archivo nuevo se escribe primero junto al antiguo, y solo lo reemplaza cuando está completo. Si algo sale mal, aparece un mensaje y el archivo que había queda como estaba.

Si las fórmulas todavía se están calculando cuando guarda, JeffCrunch espera a que terminen, de modo que el archivo contenga sus resultados. Si mientras tanto presiona **Esc** para detener el cálculo, se cancela el guardado y la barra de estado lo indica; vuelva a guardar cuando lo desee. JeffCrunch omite una cosa en el archivo: la lista del orden en que un programa calculó las fórmulas por última vez (la cadena de cálculo). El programa que abra después el libro la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el archivo pide a ese programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro con cambios sin guardar, JeffCrunch pregunta si desea **Guardar** los cambios, **No guardar** o **Cancelar** para seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.
