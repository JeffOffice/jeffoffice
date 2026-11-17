# Abrir y guardar libros

<!-- keywords: recuperar trabajo no guardado, recuperación automática, fallo, bloqueo, copia, formato antiguo, opendocument, compatibilidad, nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, importar, exportar, reciente, anclado, arrastrar, soltar, carga, grande, macros, sin guardar, cambios, cerrar, copia, cadena de cálculo, plantilla, xltx, xltm, no se puede abrir, dañado, cifrado, contraseña, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ots, reparar, abrir y reparar, plantillas integradas, presupuesto, factura, hoja de horas, préstamo, calendario, libro de calificaciones, ancho fijo, columnas fijas, separador decimal, separador de miles, signo menos final, línea de comandos, opción de separador, opción de codificación, opción decimal, html, página web, htm, tablas de una página -->

Cuando JeffCrunch se inicia sin archivo que abrir, la ventana muestra un libro nuevo y vacío con una hoja, llamada Libro1. Cada libro nuevo toma el siguiente número (Libro2, Libro3, y así sucesivamente), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, pulse **{key:New}**, o haga clic en **Archivo** y luego en **Nuevo**, y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado cede su lugar cuando abre un archivo en su ventana; uno que sí ha modificado se mantiene, y el archivo se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, igual que **Guardar como**, y lo guarda como archivo .xlsx. Cerrar un libro nuevo que no ha modificado no pregunta nada.

Para abrir un libro, pulse **{key:Open}**, o haga clic en **Archivo** y luego en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde el administrador de archivos hasta la ventana. Un libro que ya está abierto en una ventana no se abre por segunda vez: esa ventana pasa al frente.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros), y las plantillas **.xltx** y **.xltm** (una plantilla que contiene macros). No ejecuta las macros y las conserva intactas: el proyecto de macros, su firma y las partes que los nombran se vuelven a escribir exactamente como estaban. Una plantilla se guarda como plantilla, y un libro con macros como libro con macros, porque el tipo forma parte del archivo.

## Recuperar trabajo no guardado

Si JeffCrunch se cierra de forma inesperada, por ejemplo por un corte de luz, al iniciarse de nuevo muestra una lista de las copias que guardó de su trabajo sin guardar. Elija cuáles abrir y haga clic en **Abrir**; cada una se abre como un libro nuevo que todavía no tiene archivo, así que haga clic en **Guardar** y elija un nombre. **Eliminar** descarta las copias marcadas, y **Ahora no** las deja para más tarde. Puede pedir la lista en cualquier momento con **Archivo > Abrir > Recuperar trabajo no guardado**. Consulte [Cambiar la configuración de JeffCrunch](options) para saber con qué frecuencia se hacen las copias.

## Otros tipos de archivo

JeffCrunch también abre libros **.xls** antiguos, hojas de cálculo OpenDocument (**.ods** y la versión plana **.fods**), y archivos de texto con valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde cualquiera de ellos se guarda como un nuevo archivo **.xlsx** (o como copia **.xls** o **.ods**: vea más abajo): cuando pulsa **{key:Save}**, JeffCrunch se lo indica y pide un nombre, y el archivo que abrió nunca se modifica. Las imágenes, las formas, los cuadros de texto, los grupos, las líneas y el texto artístico de un archivo **.ods** pasan al nuevo archivo con su posición, tamaño, giro, orden, nombres, texto alternativo, rellenos y contornos; sus gráficos pasan con ellos, dibujados a partir de las celdas que leen.

Una hoja de cálculo OpenDocument se lee con sus hojas, valores, fórmulas (con los resultados que guardó el archivo), formatos de número, fuentes, rellenos, bordes, celdas combinadas, anchos de columna y alturas de fila, filas y columnas ocultas, paneles inmovilizados y nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior le indica cuáles tiene el archivo, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña pide la contraseña al abrirla: consulte [Abrir libros protegidos con contraseña](open-password).

Un archivo de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, punto y coma, tabulaciones o barras verticales) y cómo están codificadas sus letras (UTF-8 o UTF-16, o bien la página de códigos de su idioma), y lee cada valor como lo escribiría usted en su idioma: un número como 1.234,5 en alemán, un porcentaje, una cantidad de dinero, una fecha con su año (como 15/03/2024, o 15.03.2024 en alemán), una hora, VERDADERO o FALSO (en su idioma, como WAHR y FALSCH en alemán, y en inglés). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos se quedan como texto, para que un código postal o un número de cuenta no cambie; y también una fecha con día y mes pero sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, 1 de marzo o 25 dic), porque el año habría que inventarlo, y textos como el nombre del gen MARCH1. Una hoja admite 1.048.576 filas y 16.384 columnas; si el archivo tiene más, JeffCrunch abre lo que cabe y se lo indica.

Al abrir un archivo de texto, una ventana muestra primero sus primeras filas y pregunta cómo se lee: qué separa los campos (una coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba, con la opción de contar como uno varios separadores seguidos), qué encierra un campo que contiene un separador, la codificación del texto (Unicode, las páginas de códigos de Windows, las páginas ISO 8859, Mac Roman y las de Asia oriental), y la fila por la que empezar. Encima de cada columna elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar un número de pieza como 007 o un número de cuenta largo tal cual), una **Fecha** en el orden que indique, o **Omitir**. Algunos archivos alinean sus valores en columnas de igual ancho, con espacios entre ellas y sin ningún separador. Marque **Las columnas tienen un ancho fijo**, y las primeras líneas del archivo aparecen en un recuadro. Haga clic entre dos caracteres para empezar allí una columna, y vuelva a hacer clic para quitarla; o escriba las posiciones en **Las columnas comienzan en**, por ejemplo 10, 25, 40. Cada fragmento se corta en esas posiciones, y se quitan los espacios que lo rodean. **Separador decimal** y **Separador de miles** indican a JeffCrunch cómo están escritos los números en este archivo, sea cual sea el idioma que usa usted, de modo que 1.234,50 es mil doscientos treinta y cuatro y medio cuando el archivo usa la coma como separador decimal. Marque **Un signo menos puede ir después del número** en los archivos que escriben -12 como 12-. La vista previa sigue cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero el archivo completo se abre. **Archivo > Abrir > Abrir archivo de texto** pregunta aunque el nombre del archivo sea poco habitual.

También puede indicar cómo se lee un archivo de texto al iniciar JeffCrunch desde la línea de comandos, y entonces no aparece ninguna ventana. **--separator** indica qué separa los campos: **comma**, **semicolon**, **tab**, **space**, **bar**, o un carácter. **--encoding** indica cómo están codificadas las letras: **utf-8**, **utf-8-bom**, **utf-16le**, **utf-16be**, **latin1**, **mac-roman**, **windows-1252** (u otra página, como **windows-1250**), o **iso-8859-N**. **--decimal** indica el separador de los números: **point** o **comma**. Si omite una opción, JeffCrunch averigua esa parte como de costumbre. Ejemplo: JeffCrunch --separator semicolon --decimal comma --encoding windows-1252 precios.csv. Un valor que JeffCrunch no entiende (un separador vacío, un fin de línea como separador, o el mismo carácter para el separador y para el separador decimal) se indica en pantalla, y no se abre nada. Las opciones solo se aplican a los archivos de texto nombrados en esa línea de comandos. Un archivo que abra después con **Archivo > Abrir**, o que suelte sobre la ventana, pregunta como de costumbre.

JeffCrunch también abre **páginas web** (**.html** y **.htm**), y una página guardada con el nombre de un libro. Cada tabla de la página se convierte en una hoja, llamada **Tabla1**, **Tabla2**, y así sucesivamente. Las celdas se leen como si las hubiera escrito usted: un número es un número, y una fecha es una fecha. Una celda que está unida a varias columnas o filas deja celdas vacías, de modo que las demás celdas se mantienen en sus columnas. Las palabras fuera de las tablas, las imágenes y el aspecto de la página no se leen, y una página sin tabla no se abre. No se descarga de Internet nada de lo que nombra la página.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo** y luego en **Guardar como**, y elija el tipo de archivo **.csv** (o **.tsv** y **.txt**, separados por tabulaciones). Antes de escribir el archivo, JeffCrunch indica lo que tiene su libro y el archivo no conservará (por ejemplo, las demás hojas, fórmulas, imágenes, gráficos, notas y macros), y pregunta cómo escribirlo, y si desea **Guardar solo valores**, **Guardar como .xlsx** para conservar todo, o **Cancelar**:

- **Separador.** Una coma, punto y coma, tabulación, espacio, barra vertical, o un carácter que escriba. Un **.csv** empieza con punto y coma allí donde su idioma escribe una coma en los números, y con coma en los demás casos; un **.tsv** o **.txt**, con tabulación.
- **Codificación del texto.** UTF-8 con marca (para que otros programas reconozcan bien las letras acentuadas), UTF-8 sin ella, UTF-16, o una página de códigos como Europa occidental (1252), las páginas ISO 8859, Mac Roman, o las páginas japonesa, china, coreana, hebrea, árabe, báltica, vietnamita y tailandesa. Una letra para la que la página de códigos no tiene byte se escribe como signo de interrogación, y la barra de estado indica cuántas son.
- **Finales de línea.** Windows (retorno de carro y avance de línea), Linux y Mac (avance de línea), o Mac antiguo (retorno de carro).
- **Celdas.** **Como se muestran en las celdas** escribe lo que ve. **Como se almacenan** escribe todos los dígitos de un número (0,1 y no 10 %), y una fecha como texto ISO, como 2026-03-04.

El texto va entre comillas cuando contiene el separador, unas comillas, un salto de línea, o un espacio en cualquiera de sus extremos. Las otras hojas, las fórmulas, los formatos y la configuración no están en el archivo, y su libro no se modifica al escribir uno.

Un libro que procede de un archivo de texto se vuelve a escribir con la forma que tenía el archivo: el mismo separador, finales de línea y codificación. Si lo guardó como copia sin cambiar ninguna celda, la copia es el mismo archivo, byte a byte, y no se pregunta nada.

## Guardar una copia como .xls o .ods

Para hacer una copia que puedan abrir programas más antiguos, o programas que leen OpenDocument, haga clic en **Archivo** y luego en **Guardar como**, y elija el tipo de archivo **.xls** o **.ods**. La copia se hace a partir de su libro tal como está ahora, con los cambios que todavía no ha guardado, y contiene:

- los valores, con sus textos, números, valores lógicos y errores;
- las fórmulas, con los resultados que tienen ahora (una fórmula matricial sigue siendo una fórmula matricial);
- los formatos de número, fuentes, rellenos, bordes, alineación, y si una celda está bloqueada;
- las celdas combinadas, los anchos de columna y las alturas de fila, las filas y columnas ocultas, los esquemas de filas y columnas, y los paneles inmovilizados;
- las hojas, en su orden, con sus nombres y cuáles están ocultas, y los nombres que definió (y las áreas de impresión);
- los gráficos de cada hoja (solo en una copia **.ods**): columnas, barras, líneas, áreas, circulares, anillos y de dispersión, con sus series y las celdas que leen, categorías, títulos, leyenda, ejes (títulos, escala, formato de número), etiquetas de datos, colores y posición (lo que un gráfico **.ods** no tiene dónde guardar, como líneas de tendencia, barras de error y ejes secundarios, se nombra en el mensaje);
- las imágenes, formas, cuadros de texto, grupos, líneas y texto artístico de cada hoja (solo en una copia **.ods**), con su posición, tamaño, giro, orden, nombres, texto alternativo, rellenos, contornos, palabras, y las propias imágenes con sus recortes (el texto artístico se guarda como la forma de texto de efectos que tiene OpenDocument);
- los vínculos, y cómo se imprime cada hoja (el papel, los márgenes, la orientación, la escala, los encabezados y pies de página, y los saltos de página).

Un archivo **.xls** también contiene tablas de datos, y conserva la configuración de cálculo del libro. Una copia no puede contener todo. Antes de que JeffCrunch escriba el archivo, indica lo que tiene su libro y el archivo no conservará: gráficos, imágenes y otros dibujos (en una copia **.xls**; una copia **.ods** los conserva, con los gráficos, salvo los diagramas), notas, tablas dinámicas, tablas, formatos condicionales, validación de datos, filtros, protección de hoja, controles de formulario, macros, y vínculos a otros libros (las fórmulas que los usan conservan los valores que tenían). Una fórmula que la forma antigua no puede expresar (una función que no tiene, o una celda más allá de la columna IV o de la fila 65.536) se escribe como el valor que resultó, y el mensaje cuenta cuántas son. La forma antigua también se detiene en 65.536 filas, 256 columnas, 255 hojas y 32.767 caracteres en una celda, y en unos 4.000 formatos de celda y 56 colores: lo que no cabe se recorta, y el mensaje indica cuánto. Entonces puede elegir **Guardar copia**, **Guardar como .xlsx** para conservar todo, o **Cancelar**.

La copia no modifica su libro, y este no se marca como guardado. El archivo que abrió nunca se escribe: dé a la copia otro nombre. Una copia de un archivo **.xls** o **.ods** que abrió puede guardarse en el otro tipo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffCrunch escribe el libro en ese archivo, con los cambios que todavía no ha guardado, y usted sigue trabajando en el libro que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban. Puede elegir cualquier tipo de archivo que ofrece **Guardar como**. Una copia nunca se escribe sobre el archivo propio del libro.

## Mantener una copia de seguridad

Haga clic en **Archivo > Opciones**, y en la página **General** active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffCrunch copia primero el archivo anterior en otro llamado «Copia de seguridad de» seguido de su nombre, en la misma carpeta. Si no se puede hacer la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, a menos que diga Sí, y el archivo se queda como estaba.

## Guardar como plantilla

Una plantilla es un libro que usa como punto de partida. Puede contener fórmulas, estilos y hojas. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un archivo **.xltx**) se guarda en **Mis plantillas**, y su libro no se modifica. En **Guardar como** también puede elegir **Plantillas (*.xltx)**, o **Plantillas de hoja de cálculo de OpenDocument (*.ots)**, para guardar un archivo de plantilla donde prefiera.

Para empezar un libro a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un libro nuevo con las celdas y hojas de la plantilla. Todavía no tiene archivo propio, así que **Guardar** pide un nombre, y la plantilla no se modifica. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes.

- **Abrir como solo lectura** abre el libro solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no puede cambiar ninguna celda. La barra de título muestra «Solo lectura». Si pulsa **Guardar**, JeffCrunch pide un nombre nuevo, de modo que el archivo que abrió no se modifica.
- **Abrir como copia** abre el libro sin archivo propio. **Guardar** pregunta dónde colocarlo. El archivo que abrió nunca se modifica.
- **Abrir en una nueva ventana** abre el libro en una ventana propia, aunque la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un archivo .xlsx o .xls dañado. Vea más abajo.

## Abrir y reparar

A veces un archivo .xlsx está dañado. Una descarga se interrumpió a medias, o un disco cometió errores. Un archivo .xlsx es un conjunto de partes más pequeñas, como las hojas, los estilos y el texto compartido. **Abrir y reparar** lee cada parte que puede. Una parte que se cortó a mitad se conserva hasta el punto en que se cortó, de modo que puede obtener las primeras filas de una hoja. Una parte que no se puede leer se omite. Si se han perdido la lista de partes o los vínculos entre ellas, JeffCrunch los vuelve a crear.

El libro reparado se abre sin archivo propio, con un nombre como «Presupuesto reparado». Un mensaje indica lo que se conservó hasta una rotura y lo que se omitió. El archivo dañado no se modifica. Guarde el libro reparado con un nombre nuevo, y compruebe los números antes de fiarse de ellos.

### Un archivo .xls

Un archivo .xls también puede estar dañado. Si su envoltura exterior está cortada o sus listas están dañadas, **Abrir y reparar** sigue la envoltura todo lo que puede leer, y cuando eso falla, busca en el archivo el inicio del libro y lee desde allí. Una hoja que se cortó conserva las celdas anteriores al corte. Lo que venía después del corte se pierde, y así se le indica. El libro reparado se abre como copia, así que guárdelo con un nombre nuevo.

## Libros grandes

JeffCrunch lee un libro en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi de inmediato, aunque el libro sea muy grande. La barra de estado, abajo a la izquierda, indica lo que está haciendo: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta el momento, **Calculando** con un porcentaje, y después **Listo**.

Puede desplazarse, seleccionar y copiar mientras se lee. Las pestañas de las demás hojas se activan cuando se leen. Un libro dañado se abre hasta donde se puede leer, y un mensaje indica lo que se omitió.

## Si un archivo no se abre

Cuando JeffCrunch no puede abrir un archivo, lo dice con palabras sencillas y deja como estaban los libros que tiene abiertos. Distingue un archivo vacío de uno que no es un libro en absoluto (un documento PDF, una imagen, un documento de texto o una presentación, por ejemplo); un paquete zip cortado o dañado, lo que normalmente indica que una descarga o una copia se interrumpió, así que cópielo de nuevo desde el original; un libro protegido con contraseña, que la pide (consulte [Abrir libros protegidos con contraseña](open-password)); y un archivo compuesto que no contiene ningún libro.

## Libros recientes

La página **Abrir** de la pantalla Archivo enumera los libros que abrió últimamente. Haga clic con el botón derecho en uno para **anclarlo** a la parte superior de la lista o para quitarlo de ella.

## Guardar

Pulse **{key:Save}**, o haga clic en **Archivo** y luego en **Guardar**, para guardar sus cambios en el libro que abrió. Para conservar el original y crear un archivo nuevo, haga clic en **Archivo** y luego en **Guardar como**, y elija un nombre. Un libro con macros se guarda como archivo **.xlsm**, una plantilla como archivo **.xltx** o **.xltm**, y cualquier otro como archivo **.xlsx**. Un nombre que termina en otro tipo de libro no se acepta, y el mensaje indica qué terminación usar; los nombres que terminan en **.xls**, **.ods**, **.csv**, **.tsv** y **.txt** crean copias, como se describe más arriba.

JeffCrunch guarda solo lo que cambió. Todo lo demás del archivo, como las otras hojas, los gráficos, las imágenes, las macros y la configuración que JeffCrunch no muestra, se copia tal como estaba, byte a byte. En una hoja que cambió, solo se vuelven a escribir las celdas que cambió, junto con las fórmulas que dependen de ellas, que obtienen sus nuevos valores. Si no cambió ninguna celda, el archivo se deja en paz.

Si otro programa modificó el archivo después de abrirlo, JeffCrunch no guarda encima sin preguntar, porque eso haría perder los cambios del otro programa. Pregunta si desea **Guardar una copia**, **Sobrescribir** el archivo, o **Cancelar**.

El archivo nuevo se escribe primero junto al antiguo, y ocupa el lugar del antiguo solo cuando está completo. Si algo sale mal, verá un mensaje, y el archivo que había sigue como estaba.

Si las fórmulas todavía se están calculando cuando guarda, JeffCrunch espera a que terminen, de modo que el archivo contenga sus resultados. Si mientras tanto pulsa **Esc** para detener el cálculo, el guardado se cancela, y la barra de estado lo indica; guarde de nuevo cuando lo desee. JeffCrunch deja fuera del archivo una cosa: la lista del orden en que un programa calculó por última vez las fórmulas (la cadena de cálculo). Un programa que abra el libro después la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el archivo pide al programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro que tiene cambios sin guardar, JeffCrunch pregunta si desea **Guardar** los cambios, **No guardar**, o **Cancelar** y seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.

## Plantillas integradas

En **Archivo > Nuevo**, debajo del libro en blanco, hay libros ya preparados agrupados por tema: **Finanzas** (un presupuesto mensual, un plan de pagos de préstamo y un registro de chequera), **Negocios** (una factura, un informe de gastos, una hoja de horas, una lista de inventario, un registro de kilometraje y un informe de ventas), **Planificación** (un calendario y un plan de proyecto) y **Educación** (un libro de calificaciones). Cada uno muestra una pequeña imagen de su parte superior. Haga clic en uno para empezar un libro nuevo a partir de él. Escriba en los recuadros amarillos. Los demás números se calculan por usted. El libro no tiene archivo propio hasta que lo guarde.
