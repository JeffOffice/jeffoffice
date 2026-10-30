# Abrir y guardar libros
<!-- keywords: nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, hoja de cálculo, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, separado por comas, importar, exportar, recientes, anclado, anclar, arrastrar, colocar, soltar, carga, cargando, grande, macros, sin guardar, cambios, cerrar, copia, cadena de cálculo -->

Cuando JeffCrunch se inicia sin ningún archivo que abrir, la ventana contiene un libro nuevo y vacío con una hoja, llamado Libro1. Cada libro nuevo recibe el número siguiente (Libro2, Libro3, etc.), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, presione **{key:New}**, o haga clic en **Archivo**, luego en **Nuevo**, y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado se sustituye cuando abre un archivo en su ventana; uno que ha modificado se mantiene, y el archivo se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, como hace **Guardar como**, y lo guarda como archivo .xlsx. Al cerrar un libro nuevo que no ha modificado, no se pregunta nada.

Para abrir un libro, presione **{key:Open}**, o haga clic en **Archivo** y luego en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde el administrador de archivos hasta la ventana. Un libro que ya está abierto en una ventana no se abre por segunda vez: esa ventana pasa al frente.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros). No ejecuta las macros y las conserva intactas.

## Otros tipos de archivo

JeffCrunch también abre libros **.xls** antiguos, hojas de cálculo OpenDocument (**.ods** y la versión plana **.fods**) y archivos de texto de valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde uno de ellos se guarda como un archivo **.xlsx** nuevo: cuando presiona **{key:Save}**, JeffCrunch se lo indica y le pide un nombre, y el archivo que abrió nunca se modifica.

De una hoja de cálculo OpenDocument se leen las hojas, los valores, las fórmulas (con los resultados que guardaba el archivo), los formatos de número, las fuentes, los rellenos, los bordes, las celdas combinadas, el ancho de las columnas y el alto de las filas, las filas y columnas ocultas, los paneles inmovilizados y los nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior le indica cuáles de ellos contiene el archivo, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña no se abre: ábrala en el programa que la creó y guarde una copia sin la contraseña.

Un archivo de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, puntos y comas, tabulaciones o barras verticales) y cómo están codificados sus caracteres (UTF-8 o UTF-16, o bien la página de códigos de su idioma), y lee cada valor tal como lo escribiría en su idioma: un número como 1.234,5 en español, un porcentaje, un importe de dinero, una fecha con su año (como 15/03/2024, o 3/15/2024 en inglés), una hora, VERDADERO o FALSO (en su idioma y en inglés, TRUE y FALSE). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos se quedan como texto, para que no se modifique un código postal o un número de cuenta, y lo mismo ocurre con un día y un mes sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, 1 de marzo o 25 dic), porque habría que inventarse el año, y con textos como el nombre del gen MARCH1. Una hoja admite 1.048.576 filas y 16.384 columnas; si el archivo tiene más, JeffCrunch abre lo que cabe y se lo indica.

Cuando abre un archivo de texto, primero aparece una ventana que muestra sus primeras filas y le pregunta cómo leerlo: qué separa los campos (cualquiera de estos: coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba, y, si lo desea, varios seguidos se consideran uno solo), qué carácter entrecomilla un campo que contiene un separador, la codificación de texto y la fila en la que empezar. Encima de cada columna elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar tal cual un número de pieza como 007 o un número de cuenta largo), **Fecha** en el orden que indique, u **Omitir**. La vista previa refleja cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero se abre el archivo completo. **Archivo > Abrir > Abrir un archivo de texto** muestra esta ventana incluso cuando el nombre del archivo es poco habitual.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo**, luego en **Guardar como**, y elija el tipo de archivo **.csv**. JeffCrunch escribe los valores tal como los muestran las celdas, en UTF-8, con un punto y coma entre ellos si su idioma usa la coma en los números y una coma en los demás casos, y con el texto entre comillas cuando lo necesita. Las demás hojas, las fórmulas, los formatos y la configuración no se incluyen en un archivo .csv, y el libro no se modifica al escribirlo.

## Libros grandes

JeffCrunch lee los libros en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi al instante, aunque el libro sea muy grande. La barra de estado, abajo a la izquierda, indica lo que está haciendo: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta el momento, **Calculando** con un porcentaje y, por último, **Listo**.

Mientras lee, puede desplazarse, seleccionar y copiar. Las pestañas de las demás hojas se activan a medida que se leen. Un libro dañado se abre hasta donde se puede leer, y un mensaje indica lo que se omitió.

## Libros recientes

La página **Abrir** de la pantalla Archivo muestra los libros que ha abierto recientemente. Haga clic con el botón derecho en uno para **anclarlo** en la parte superior de la lista o para quitarlo de la lista.

## Guardar

Presione **{key:Save}**, o haga clic en **Archivo** y luego en **Guardar**, para guardar los cambios en el libro que abrió. Para conservar el original y crear un archivo nuevo, haga clic en **Archivo** y luego en **Guardar como**, y elija un nombre. Un libro con macros se guarda como archivo **.xlsm**, y uno sin macros, como archivo **.xlsx**.

JeffCrunch guarda solo lo que ha cambiado. Todo lo demás del archivo, como las otras hojas, los gráficos, las imágenes, las macros y la configuración que JeffCrunch no muestra, se copia tal como estaba, byte a byte. En una hoja que ha modificado, solo se vuelven a escribir las celdas que ha cambiado, junto con las fórmulas que dependen de ellas, que reciben sus nuevos valores. Si no ha cambiado ninguna celda, el archivo no se toca.

Si otro programa modificó el archivo después de que usted lo abriera, JeffCrunch no lo sobrescribe sin preguntar, porque se perderían los cambios del otro programa. Le pregunta si desea **Guardar una copia**, **Sobrescribir** el archivo o **Cancelar**.

El archivo nuevo se escribe primero junto al anterior y solo lo sustituye cuando está completo. Si algo falla, verá un mensaje, y el archivo que había se mantiene como estaba.

Si las fórmulas aún se están calculando cuando guarda, JeffCrunch espera a que terminen, para que el archivo contenga sus resultados. Si mientras tanto presiona **Esc** para detener el cálculo, el guardado se cancela y la barra de estado lo indica; vuelva a guardar cuando quiera. JeffCrunch omite una cosa en el archivo: la lista del orden en que un programa calculó las fórmulas por última vez (la cadena de cálculo). El siguiente programa que abra el libro la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el archivo pide a ese programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro con cambios sin guardar, JeffCrunch le pregunta si desea **Guardar** los cambios, **No guardar** o **Cancelar** y seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.
