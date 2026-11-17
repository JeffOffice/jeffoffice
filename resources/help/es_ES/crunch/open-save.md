# Abrir y guardar libros

<!-- keywords: recuperar trabajo no guardado, recuperación automática, bloqueo, fallo, copia, formato antiguo, opendocument, compatibilidad, nuevo, en blanco, libro1, sin título, abrir, guardar, guardar como, archivo, libro, xlsx, xlsm, xls, ods, csv, tsv, archivo de texto, delimitado, importar, exportar, recientes, anclado, arrastrar, soltar, carga, grande, macros, sin guardar, cambios, cerrar, copia, cadena de cálculo, plantilla, xltx, xltm, no se puede abrir, dañado, cifrado, contraseña, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ots, reparar, abrir y reparar, plantillas integradas, presupuesto, factura, hoja de horas, préstamo, calendario, libro de calificaciones, ancho fijo, columnas fijas, separador decimal, separador de miles, signo menos final, línea de comandos, opción de separador, opción de codificación, opción de decimal, html, página web, htm, tablas de una página, fichero, fichero de texto -->

Cuando JeffCrunch se inicia sin ningún fichero que abrir, la ventana muestra un libro nuevo y vacío con una hoja, llamada Libro1. Cada libro nuevo toma el siguiente número (Libro2, Libro3, y así sucesivamente), y un número no se vuelve a usar después de cerrar su ventana. Para empezar otro, pulse **{key:New}**, o haga clic en **Archivo** y después en **Nuevo**, y elija **Libro en blanco**; se abre en una ventana propia. Un libro nuevo que no ha modificado cede su lugar cuando abre un fichero en su ventana; uno que sí ha modificado se mantiene, y el fichero se abre en otra ventana. **Guardar** en un libro nuevo pide un nombre, igual que **Guardar como**, y lo guarda como fichero .xlsx. Al cerrar un libro nuevo que no ha modificado, no se pregunta nada.

Para abrir un libro, pulse **{key:Open}**, o haga clic en **Archivo** y después en **Abrir**, y elija **Examinar**. También puede arrastrar un libro desde el administrador de ficheros hasta la ventana. Un libro que ya está abierto en una ventana no se vuelve a abrir: esa ventana pasa al frente.

JeffCrunch abre libros **.xlsx** y libros **.xlsm** (los que contienen macros), y las plantillas **.xltx** y **.xltm** (una plantilla que contiene macros). No ejecuta las macros y las conserva intactas: el proyecto de macros, su firma y las partes que los nombran se escriben de nuevo exactamente como estaban. Una plantilla se guarda como plantilla, y un libro con macros como libro con macros, porque el tipo forma parte del fichero.

## Recuperar trabajo no guardado

Si JeffCrunch se cierra inesperadamente, por ejemplo por un corte de luz, al siguiente inicio muestra una lista de las copias que guardó de su trabajo sin guardar. Elija cuáles abrir y haga clic en **Abrir**; cada una se abre como un libro nuevo que todavía no tiene fichero, así que haga clic en **Guardar** y elija un nombre. **Eliminar** descarta las copias marcadas, y **Ahora no** las deja para más tarde. Puede pedir la lista en cualquier momento con **Archivo > Abrir > Recuperar trabajo no guardado**. Consulte [Cambiar la configuración de JeffCrunch](options) para saber con qué frecuencia se hacen las copias.

## Otros tipos de ficheros

JeffCrunch también abre libros **.xls** antiguos, hojas de cálculo de OpenDocument (**.ods** y la versión plana **.fods**) y ficheros de texto con valores (**.csv**, **.tsv** y **.txt**). Un libro abierto desde cualquiera de ellos se guarda como un fichero **.xlsx** nuevo (o como copia **.xls** o **.ods**: véase más abajo): al pulsar **{key:Save}**, JeffCrunch se lo indica y pide un nombre, y el fichero que abrió nunca se modifica. Las imágenes, formas, cuadros de texto, grupos, líneas y texto artístico de un fichero **.ods** pasan al fichero nuevo con sus posiciones, tamaños, giros, orden, nombres, texto alternativo, rellenos y contornos; sus gráficos pasan con ellos, dibujados a partir de las celdas que leen.

Una hoja de cálculo de OpenDocument se lee con sus hojas, valores, fórmulas (con los resultados que guardó el fichero), formatos de número, fuentes, rellenos, bordes, celdas combinadas, anchos de columna y alturas de fila, filas y columnas ocultas, paneles inmovilizados y nombres. Los gráficos, las imágenes y otros dibujos, los comentarios, las tablas dinámicas, las macros, los formatos condicionales y la validación de datos todavía no se leen; un mensaje en la parte superior indica cuáles tiene el fichero, y una copia guardada desde aquí no los tendrá. Una hoja de cálculo protegida con contraseña pide la contraseña al abrirla: consulte [Abrir libros protegidos con contraseña](open-password).

Un fichero de texto es una sola hoja. JeffCrunch averigua cómo están separados sus valores (por comas, punto y coma, tabulaciones o barras verticales) y cómo están codificadas sus letras (UTF-8 o UTF-16, o bien la página de códigos de su idioma), y lee cada valor como lo escribiría en su idioma: un número como 1.234,5 en alemán, un porcentaje, una cantidad de dinero, una fecha con su año (como 15/03/2024, o 15.03.2024 en alemán), una hora, VERDADERO o FALSO (en su idioma, como WAHR y FALSCH en alemán, y en inglés). Un valor que empieza por un signo igual es una fórmula. Los dígitos que empiezan por cero (como 007) y los números de más de 15 dígitos siguen siendo texto, para que un código postal o un número de cuenta no se modifique; también lo son un día y un mes sin año, en cualquier orden y en cualquier idioma (como 3/4, 12.5, 1-2, 1 de marzo o 25 dic), porque el año tendría que inventarse, y textos como el nombre del gen MARCH1. Una hoja tiene 1.048.576 filas y 16.384 columnas; si el fichero tiene más, JeffCrunch abre lo que cabe y se lo indica.

Al abrir un fichero de texto, una ventana muestra primero sus primeras filas y pregunta cómo se lee: qué separa los campos (cualquiera de una coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba, pudiendo contar varios seguidos como uno), qué encierra en comillas un campo que contiene un separador, la codificación del texto (Unicode, las páginas de códigos de Windows, las páginas ISO 8859, Mac Roman y las de Asia oriental) y la fila por la que empezar. Encima de cada columna elige cómo se lee: **Tal como se escribió**, **Texto** (para conservar un número de pieza como 007 o un número de cuenta largo tal cual), **Fecha** en el orden que indique, u **Omitir**. Algunos ficheros alinean sus valores en columnas de igual ancho, separadas por espacios y sin ningún separador. Marque **Las columnas tienen un ancho fijo**, y las primeras líneas del fichero se muestran en un cuadro. Haga clic entre dos caracteres para empezar allí una columna, y vuelva a hacer clic para quitarla; o escriba las posiciones en **Las columnas comienzan en**, como 10, 25, 40. Cada parte se corta en esas posiciones, y se quitan los espacios que la rodean. **Separador decimal** y **Separador de miles** indican a JeffCrunch cómo se escriben los números en este fichero, sea cual sea el idioma de usted, de modo que 1.234,50 es mil doscientos treinta y cuatro con cincuenta cuando el fichero usa la coma como separador decimal. Marque **Un signo menos puede ir después del número** en los ficheros que escriben -12 como 12-. La vista previa sigue cada cambio y muestra como máximo las primeras 200 filas y 100 columnas, pero se abre el fichero completo. **Archivo > Abrir > Abrir un fichero de texto** pregunta incluso cuando el nombre del fichero es poco habitual.

También puede indicar cómo se lee un fichero de texto al iniciar JeffCrunch desde una línea de comandos, y entonces no aparece ninguna ventana. **--separator** indica qué separa los campos: **comma**, **semicolon**, **tab**, **space**, **bar**, o un carácter. **--encoding** indica cómo están codificadas las letras: **utf-8**, **utf-8-bom**, **utf-16le**, **utf-16be**, **latin1**, **mac-roman**, **windows-1252** (u otra página, como **windows-1250**), o **iso-8859-N**. **--decimal** indica el signo de los números: **point** o **comma**. Si omite una opción, JeffCrunch averigua esa parte como de costumbre. Ejemplo: JeffCrunch --separator semicolon --decimal comma --encoding windows-1252 prices.csv. Un valor que JeffCrunch no entiende (un separador vacío, un fin de línea como separador, o el mismo carácter para el separador y para el separador decimal) se indica en pantalla, y no se abre nada. Las opciones solo se aplican a los ficheros de texto nombrados en esa línea de comandos. Un fichero que abra después con **Archivo > Abrir**, o que suelte en la ventana, pregunta como de costumbre.

JeffCrunch también abre **páginas web** (**.html** y **.htm**), y una página guardada con el nombre de un libro. Cada tabla de la página se convierte en una hoja, llamada **Tabla1**, **Tabla2**, y así sucesivamente. Las celdas se leen como si las hubiera escrito usted: un número es un número, y una fecha es una fecha. Una celda que está combinada en varias columnas o filas deja celdas en blanco, de modo que las demás celdas se mantienen en sus columnas. Las palabras fuera de las tablas, las imágenes y el aspecto de la página no se leen, y una página sin tablas no se abre. No se descarga nada de internet de lo que la página nombra.

Para conservar solo los valores de la hoja que está viendo, haga clic en **Archivo**, después en **Guardar como**, y elija el tipo de fichero **.csv** (o **.tsv** y **.txt**, separados por tabulaciones). Antes de escribir el fichero, JeffCrunch indica lo que su libro tiene y el fichero no conservará (por ejemplo, las demás hojas, fórmulas, imágenes, gráficos, notas y macros), y pregunta cómo escribirlo, y si desea **Guardar solo los valores**, **Guardar como .xlsx** para conservar todo, o **Cancelar**:

- **Separador.** Una coma, punto y coma, tabulación, espacio, barra vertical o un carácter que escriba. Un **.csv** empieza con punto y coma allí donde su idioma escribe una coma en los números, y con coma en los demás casos; un **.tsv** o **.txt**, con tabulación.
- **Codificación del texto.** UTF-8 con marca (para que otros programas lean bien las letras acentuadas), UTF-8 sin ella, UTF-16, o una página de códigos como Europa occidental (1252), las páginas ISO 8859, Mac Roman, o las páginas japonesa, china, coreana, hebrea, árabe, báltica, vietnamita y tailandesa. Una letra para la que la página de códigos no tiene byte se escribe como un signo de interrogación, y la barra de estado indica cuántas son.
- **Finales de línea.** Windows (retorno de carro y avance de línea), Linux y Mac (avance de línea), o Mac antiguo (retorno de carro).
- **Celdas.** **Como se muestran en las celdas** escribe lo que ve. **Como se almacenan** escribe cada dígito de un número (0,1 y no 10 %), y una fecha como texto ISO, como 2026-03-04.

El texto va entre comillas cuando contiene el separador, una comilla, un salto de línea o un espacio al principio o al final. Las demás hojas, las fórmulas, los formatos y la configuración no están en el fichero, y su libro no se modifica al escribir uno.

Un libro que procede de un fichero de texto se escribe de nuevo con la forma que tenía el fichero: el mismo separador, los mismos finales de línea y la misma codificación. Si lo guardó como copia sin cambiar ninguna celda, la copia es el mismo fichero, byte por byte, y no se pregunta nada.

## Guardar una copia como .xls u .ods

Para crear una copia que puedan abrir programas antiguos, o programas que leen OpenDocument, haga clic en **Archivo**, después en **Guardar como**, y elija el tipo de fichero **.xls** u **.ods**. La copia se crea a partir de su libro tal como está ahora, con los cambios que todavía no ha guardado, y contiene:

- los valores, con sus textos, números, valores lógicos y errores;
- las fórmulas, con los resultados que tienen ahora (una fórmula matricial sigue siendo matricial);
- los formatos de número, fuentes, rellenos, bordes, alineación, y si una celda está bloqueada;
- las celdas combinadas, los anchos de columna y las alturas de fila, las filas y columnas ocultas, los esquemas de filas y columnas, y los paneles inmovilizados;
- las hojas, en su orden, con sus nombres y cuáles están ocultas, y los nombres que definió (y las áreas de impresión);
- los gráficos de cada hoja (solo en una copia **.ods**): columnas, barras, líneas, áreas, circulares, anillos y de dispersión, con sus series y las celdas que leen, categorías, títulos, leyenda, ejes (títulos, escala, formato de número), etiquetas de datos, colores y posición (lo que un gráfico **.ods** no tiene dónde guardar, como líneas de tendencia, barras de error y ejes secundarios, se nombra en el mensaje);
- las imágenes, formas, cuadros de texto, grupos, líneas y texto artístico de cada hoja (solo en una copia **.ods**), con sus posiciones, tamaños, giros, orden, nombres, texto alternativo, rellenos, contornos, textos, y las propias imágenes con sus recortes (el texto artístico se guarda como la forma de texto de efectos que tiene OpenDocument);
- los vínculos, y cómo se imprime cada hoja (el papel, los márgenes, la orientación, la escala, los encabezados y pies de página, y los saltos de página).

Un fichero **.xls** también contiene tablas de datos, y conserva la configuración de cálculo del libro. Una copia no puede contenerlo todo. Antes de que JeffCrunch escriba el fichero, indica lo que su libro tiene y el fichero no conservará: gráficos, imágenes y otros dibujos (en una copia **.xls**; una copia **.ods** los conserva, incluidos los gráficos, salvo los diagramas), notas, tablas dinámicas, tablas, formatos condicionales, validación de datos, filtros, protección de hoja, controles de formulario, macros y vínculos a otros libros (las fórmulas que los usan conservan los valores que tenían). Una fórmula que la forma antigua no puede expresar (una función que no tiene, o una celda más allá de la columna IV o de la fila 65.536) se escribe como el valor que resultó, y el mensaje cuenta cuántas son. La forma antigua también se detiene en 65.536 filas, 256 columnas, 255 hojas y 32.767 caracteres en una celda, y en unos 4.000 formatos de celda y 56 colores: lo que no cabe se recorta, y el mensaje indica cuánto. Entonces puede elegir **Guardar copia**, **Guardar como .xlsx** para conservar todo, o **Cancelar**.

La copia no modifica su libro, y el libro no se marca como guardado. El fichero que abrió nunca se escribe: dé a la copia otro nombre. Una copia de un fichero **.xls** u **.ods** que abrió puede guardarse con el otro tipo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffCrunch escribe el libro en ese fichero, con los cambios que todavía no ha guardado, y usted sigue trabajando en el libro que tenía. Su nombre y sus cambios sin guardar siguen como estaban. Puede elegir cualquier tipo de fichero que ofrece **Guardar como**. Una copia nunca se escribe sobre el fichero propio del libro.

## Mantener una copia de seguridad

Haga clic en **Archivo > Opciones**, y en la página **General** active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffCrunch copia primero el fichero anterior a otro llamado «Copia de seguridad de» seguido de su nombre, en la misma carpeta. Si no se puede hacer la copia, la aplicación pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el fichero sigue como estaba.

## Guardar como plantilla

Una plantilla es un libro que usa como punto de partida. Puede contener fórmulas, estilos y hojas. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla (un fichero **.xltx**) va a **Mis plantillas**, y su libro no se modifica. En **Guardar como** también puede elegir **Plantillas (*.xltx)**, o **Plantillas de hoja de cálculo de OpenDocument (*.ots)**, para guardar un fichero de plantilla donde prefiera.

Para empezar un libro a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un libro nuevo con las celdas y hojas de la plantilla. Todavía no tiene fichero propio, así que **Guardar** pide un nombre, y la plantilla no se modifica. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un fichero

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de Recientes.

- **Abrir como solo lectura** abre el libro solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no puede cambiar ninguna celda. La barra de título dice «Solo lectura». Si pulsa **Guardar**, JeffCrunch pide un nombre nuevo, para que el fichero que abrió no se modifique.
- **Abrir como copia** abre el libro sin fichero propio. **Guardar** pregunta dónde colocarlo. El fichero que abrió nunca se modifica.
- **Abrir en nueva ventana** abre el libro en una ventana propia, incluso cuando la ventana en la que está está vacía.
- **Abrir y reparar** sirve para un fichero .xlsx o .xls dañado. Véase más abajo.

## Abrir y reparar

A veces un fichero .xlsx está dañado. Una descarga que se interrumpió a la mitad, o un disco que cometió errores. Un fichero .xlsx es un conjunto de partes más pequeñas, como las hojas, los estilos y el texto compartido. **Abrir y reparar** lee cada parte que puede. Una parte que se cortó a la mitad se conserva hasta el punto en que se cortó, así que puede obtener las primeras filas de una hoja. Una parte que no se puede leer se deja fuera. Si la lista de partes o los vínculos entre ellas han desaparecido, JeffCrunch los vuelve a crear.

El libro reparado se abre sin fichero propio, con un nombre como «Presupuesto reparado». Un mensaje le indica qué se conservó hasta una interrupción y qué se dejó fuera. El fichero dañado no se modifica. Guarde el libro reparado con un nombre nuevo, y compruebe los números antes de fiarse de ellos.

### Un fichero .xls

Un fichero .xls también puede estar dañado. Si su envoltura exterior está cortada, **Abrir y reparar** busca en el fichero el inicio del libro y lee desde allí. Una hoja que se cortó conserva las celdas anteriores al corte. Lo que venía después del corte se pierde, y así se le indica. El libro reparado se abre como copia, así que guárdelo con un nombre nuevo.

## Libros grandes

JeffCrunch lee un libro en segundo plano. Las primeras filas de la hoja que estaba abierta cuando se guardó el libro aparecen casi de inmediato, incluso cuando el libro es muy grande. La barra de estado, en la parte inferior izquierda, indica qué está haciendo: **Abriendo**, **Leyendo** una hoja con el número de celdas leídas hasta ese momento, **Calculando** con un porcentaje, y después **Listo**.

Puede desplazarse, seleccionar y copiar mientras se lee. Las pestañas de las demás hojas se activan cuando se leen. Un libro dañado se abre hasta donde se puede leer, y un mensaje indica lo que se ha dejado fuera.

## Si un fichero no se abre

Cuando JeffCrunch no puede abrir un fichero, lo dice con palabras sencillas y deja los libros que tiene abiertos como están. Distingue un fichero vacío de uno que no es un libro en absoluto (un documento PDF, una imagen, un documento de texto o una presentación, por ejemplo); un paquete zip cortado o dañado, lo que suele indicar que una descarga o una copia se interrumpió, así que cópielo de nuevo desde el original; un libro protegido con contraseña, que la pide (consulte [Abrir libros protegidos con contraseña](open-password)); y un fichero compuesto que no contiene ningún libro.

## Libros recientes

La página **Abrir** de la pantalla Archivo muestra los libros que abrió hace poco. Haga clic con el botón derecho en uno para **anclarlo** a la parte superior de la lista, o para quitarlo de la lista.

## Guardar

Pulse **{key:Save}**, o haga clic en **Archivo** y después en **Guardar**, para guardar sus cambios en el libro que abrió. Para conservar el original y crear un fichero nuevo, haga clic en **Archivo** y después en **Guardar como**, y elija un nombre. Un libro con macros se guarda como fichero **.xlsm**, una plantilla como fichero **.xltx** o **.xltm**, y cualquier otro como fichero **.xlsx**. Un nombre que termina en otro tipo de libro no se acepta, y el mensaje indica qué terminación usar; los nombres que terminan en **.xls**, **.ods**, **.csv**, **.tsv** y **.txt** crean copias, como se ha descrito antes.

JeffCrunch guarda solo lo que cambió. Todo lo demás del fichero, como las demás hojas, los gráficos, las imágenes, las macros y la configuración que JeffCrunch no muestra, se copia tal como estaba, byte por byte. En una hoja que cambió, solo se vuelven a escribir las celdas que cambió, junto con las fórmulas que dependen de ellas, que reciben sus nuevos valores. Si no cambió ninguna celda, el fichero se deja en paz.

Si otro programa cambió el fichero después de abrirlo, JeffCrunch no lo sobrescribe sin preguntar, porque se perderían los cambios del otro programa. Pregunta si desea **Guardar una copia**, **Sobrescribir** el fichero, o **Cancelar**.

El fichero nuevo se escribe primero junto al anterior, y ocupa el lugar del antiguo solo cuando está completo. Si algo sale mal, aparece un mensaje, y el fichero que había sigue como estaba.

Si las fórmulas todavía se están calculando cuando guarda, JeffCrunch espera a que terminen, para que el fichero contenga sus resultados. Si mientras tanto pulsa **Esc** para detener el cálculo, el guardado se cancela, y la barra de estado lo indica; guarde de nuevo cuando lo desee. JeffCrunch deja fuera del fichero una cosa: la lista del orden en que un programa calculó por última vez las fórmulas (la cadena de cálculo). Un programa que abra el libro después la vuelve a crear. Si JeffCrunch no pudo calcular alguna fórmula, o usted detuvo el cálculo, el fichero pide a ese programa que vuelva a calcular todas las fórmulas al abrir el libro.

## Cerrar con cambios

Si cierra un libro que tiene cambios sin guardar, JeffCrunch pregunta si desea **Guardar** los cambios, **No guardar** o **Cancelar** para seguir trabajando. Un asterisco (*) en el título de la ventana indica que hay cambios sin guardar.

## Plantillas integradas

En **Archivo > Nuevo**, debajo del libro en blanco, hay libros ya preparados agrupados por categorías: **Finanzas** (un presupuesto mensual, un plan de pagos de préstamo y un registro de chequera), **Negocios** (una factura, un informe de gastos, una hoja de horas, una lista de inventario, un registro de kilometraje y un informe de ventas), **Planificación** (un calendario y un plan de proyecto) y **Educación** (un libro de calificaciones). Cada uno muestra una pequeña imagen de su parte superior. Haga clic en uno para iniciar un libro nuevo a partir de él. Escriba en los recuadros amarillos. Los demás números se calculan por usted. El libro no tiene fichero propio hasta que lo guarde.
