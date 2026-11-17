# Abrir, guardar, imprimir y exportar
<!-- keywords: libro electrónico, ebook, epub, lector electrónico, capítulos, contenido, página web, html, guardar como página web, página web filtrada, correo electrónico, eml, marcadores, marcadores de PDF, mayús+f5, última edición, dónde se quedó, plantillas integradas, galería, carta, informe, memorando, manuscrito, boletín, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, archivos antiguos, plantilla, dotx, recientes, anclar, nuevo, cerrar, imprimir, pdf, exportar, recuperar, recuperación automática, sin guardar, bloqueo, pantalla de archivo, formatos de archivo, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ott, reparar, abrir y reparar, dañado, contraseña, cifrar, proteger con contraseña, abrir documento, crear documento, ficheros antiguos, pantalla de fichero, formatos de fichero -->

Haga clic en **Archivo**, en la esquina superior izquierda, para ver la pantalla Archivo. Tiene páginas para Nuevo, Abrir, Imprimir y Exportar, y comandos para Guardar y Guardar como. Pulse **Esc** para volver al documento.

## Iniciar un documento nuevo

Pulse **Ctrl+N**, o haga clic en **Archivo > Nuevo** y luego en **Documento en blanco**. El documento nuevo se abre en su propia ventana. Si la ventana en la que está está vacía y nunca se ha guardado, el documento nuevo ocupa su lugar.

## Abrir un documento

Pulse **Ctrl+O**, o haga clic en **Archivo > Abrir**. Haga clic en **Examinar** para elegir un fichero. JeffWrite abre estos tipos de fichero:

- **.docx** es un documento.
- **.dotx** es una plantilla. Una plantilla se abre como un documento nuevo creado a partir de ella, de modo que la plantilla permanece como está.
- **.docm** y **.dotm** son un documento y una plantilla que pueden contener **macros**, pequeños programas que acompañan a un fichero. JeffWrite nunca las ejecuta. Las conserva tal como estaban al guardar como .docm o .dotm. Si guarda como .docx o .odt, las macros se omiten y JeffWrite se lo indica.
- **.txt** es texto sin formato, solo las palabras. Al abrirlo, JeffWrite lo lee de inmediato si es ASCII o Unicode simple. Si no lo es, JeffWrite le pregunta con qué codificación leerlo y muestra las primeras líneas, para que compruebe que las letras se ven bien. Una **codificación** es la forma en que un fichero almacena las letras, como UTF-8 o Windows 1252. Al guardar como .txt, elige la codificación y los finales de línea, y el fichero conserva solo las palabras: nada de fuentes, imágenes ni líneas de tabla.
- **.html** y **.htm** son páginas web. JeffWrite lee las palabras, los encabezados, la negrita, la cursiva, el subrayado, los colores, las listas y las tablas. Las viñetas y los números de las listas se importan como marcas escritas, y las imágenes y los vínculos se omiten. Nunca descarga nada de internet e ignora los scripts. Una página web nunca se sobrescribe: **Guardar** pide un nombre nuevo y guarda un fichero .docx. Para crear una página web, vea **Guardar como página web** más abajo.
- **.doc** y **.dot** son los tipos antiguos de documento y plantilla. JeffWrite los lee y guarda lo que haga con ellos como un fichero .docx nuevo.
- **.odt**, **.ott** y **.fodt** son ficheros OpenDocument. JeffWrite lee los tres. Guarda ficheros .odt, y guarda lo que haga con un fichero .ott o .fodt como un fichero .odt nuevo. Vea [Abrir y guardar ficheros OpenDocument](open-odt).
- **.rtf** es formato de texto enriquecido. JeffWrite lo lee y lo guarda. Vea [Abrir ficheros RTF](open-rtf).

La página **Abrir** también muestra los documentos que abrió o guardó recientemente. Haga clic en uno para abrirlo. Haga clic con el botón derecho en uno para anclarlo al principio de la lista o quitarlo de ella.

Si un fichero tiene partes que JeffWrite todavía no puede editar, como el diseño de una tabla o un encabezado, las conserva. Vea [Bienvenido a JeffWrite](start). Si una parte del fichero no se puede leer en absoluto, JeffWrite le indica qué omitió. Entonces no guarda sobre ese fichero: Guardar pide un nombre nuevo, para que el original conserve lo que la copia perdió.

## Guardar un documento

Pulse **Ctrl+S** para guardar. La primera vez, JeffWrite pide un nombre. **Guardar como** (F12) guarda una copia con un nombre nuevo. Elija **Plantilla (*.dotx)** para guardar una plantilla, **Documento habilitado para macros (*.docm)** para conservar las macros, **Texto sin formato (*.txt)** para guardar solo las palabras, o **Formato de texto enriquecido (*.rtf)** para un fichero que otros programas puedan leer. Un fichero que procede de un tipo de fichero antiguo siempre se guarda con un nombre nuevo, de modo que el fichero antiguo nunca se sobrescribe.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia**. Elija un nombre. JeffWrite escribe el documento en ese fichero, y usted sigue trabajando en el documento que tenía. Su nombre y sus cambios sin guardar permanecen como estaban. Es útil cuando quiere enviar una versión a alguien y seguir escribiendo.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones**, y en la página **General** active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffWrite primero copia el fichero anterior en otro llamado «Copia de» seguido de su nombre, en la misma carpeta. Si no se puede hacer la copia, JeffWrite pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el fichero se queda como estaba. Así puede volver a la última versión guardada.

## Guardar como plantilla

Una plantilla es un documento que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla se guarda en **Mis plantillas**, y el documento en el que escribe no cambia. En **Guardar como** también puede elegir **Plantilla (*.dotx)** o **Plantilla de texto de OpenDocument (*.ott)** para guardar un fichero de plantilla donde prefiera.

Para iniciar un documento nuevo a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un documento nuevo con el texto y los estilos de la plantilla. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un fichero

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes. Cada forma pide un fichero, o usa el que haya elegido:

- **Abrir como solo lectura** abre el fichero solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no cambiar nada. La barra de título dice «Solo lectura». Si pulsa **Guardar**, JeffWrite pide un nombre nuevo, de modo que el fichero abierto no se modifica.
- **Abrir como copia** abre un documento nuevo con el texto del fichero. No tiene fichero propio, así que **Guardar** pregunta dónde ubicarlo. El fichero abierto nunca se modifica.
- **Abrir en nueva ventana** abre el fichero en una ventana propia, incluso cuando la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un fichero .docx o .doc dañado, por ejemplo uno que se cortó al copiarlo. Vea más abajo.

## Abrir y reparar

A veces un fichero .docx o .doc se daña. Una copia se detuvo a medio camino, o un disco cometió errores. Un fichero .docx es un conjunto de partes más pequeñas, como el texto, los estilos y las imágenes. **Abrir y reparar** lee cada parte que puede. Una parte que se cortó a mitad se conserva hasta el punto donde se cortó. Una parte que no se puede leer se omite. Si la lista de partes o los vínculos entre ellas se han perdido, JeffWrite los vuelve a crear.

Un fichero .doc antiguo es un contenedor grande con unos pocos flujos de datos dentro. Uno contiene el texto, y otro, la tabla que indica dónde está cada fragmento de texto y cómo se ve. Si el contenedor se corta o sus listas están dañadas, **Abrir y reparar** localiza esos flujos según su contenido y lee todo lo que puede. Si se pierde la tabla, el texto se conserva como párrafos sin formato, sin estilos, listas, tablas ni imágenes. Un fichero .doc que tiene contraseña no se puede reparar.

El documento reparado se abre como una copia con un nombre como «Carta reparada». Un mensaje le indica qué se conservó hasta una interrupción y qué se omitió. El fichero dañado no se modifica. Guarde la copia reparada con un nombre propio y compruébela antes de confiar en ella.

## Recuperar trabajo sin guardar

JeffWrite guarda una copia de reserva del trabajo sin guardar cada pocos minutos, como un fichero en su propia carpeta. Si el programa se cierra inesperadamente, ofrece esas copias la próxima vez que lo inicie. Haga clic en **Archivo > Abrir > Recuperar trabajo sin guardar** para verlas más tarde. Puede elegir cuáles abrir, y guardar cada una donde quiera.

## Empezar a partir de una plantilla

**Archivo > Nuevo** muestra el **Documento en blanco** y un conjunto de plantillas que vienen con JeffWrite, agrupadas por tipo: una carta, un informe, un memorando, un manuscrito y un boletín. Cada imagen es la primera página del documento que obtendrá. Una **plantilla** es un documento ya preparado, con sus estilos, márgenes y textos de muestra. Haga clic en una y reemplace los textos de muestra por los suyos. Las plantillas que usted guardó aparecen en **Mis plantillas**.

## Retomar donde lo dejó

JeffWrite recuerda dónde estaba el cursor cuando guardó o cerró un fichero. La próxima vez que lo abra, el cursor estará allí. Pulse **Mayús+F5** para ir a los lugares que editó por última vez. Cada pulsación lleva al anterior: los últimos tres lugares, y luego vuelve al primero.

## Guardar como .odt

Un fichero .odt no puede contener todo lo que puede contener un fichero .docx. Al guardar como .odt, JeffWrite primero enumera lo que se perdería, y le permite guardar un fichero .docx en su lugar. Vea [Información](info).

## Imprimir **Ctrl+P**, o haga clic en **Archivo > Imprimir**. Elija una impresora, las copias y las páginas, revise cada página en la vista previa y haga clic en **Imprimir**. Vea [Imprimir](print).

## Crear un PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Elija qué páginas incluir: todas, la página actual, o un intervalo como 1-3, 5 (p2s3 y s3 también funcionan aquí). Luego elija cómo se crea:

- **Optimizar para** tiene **Estándar**, que conserva las imágenes tal como están, y **Tamaño mínimo**, que hace que cada imagen no tenga más de 150 píxeles por pulgada, para que el fichero sea más pequeño. Use Tamaño mínimo para un PDF que vaya a publicar en una página web o enviar por correo electrónico.
- **Incluir propiedades del documento** pone el título, el autor, el asunto y las palabras clave del documento en el PDF.
- **Crear marcadores a partir de los encabezados** convierte cada encabezado en un **marcador**, una entrada de la lista que un lector de PDF muestra junto a las páginas. Haga clic en un marcador para ir a su página.
- **Etiquetas de estructura del documento para accesibilidad** etiqueta los encabezados, los párrafos, las listas, las tablas (con sus filas de encabezado) y las imágenes (con su texto alternativo), e indica el idioma, para que un lector de pantalla lea el PDF en orden.

Después elija un nombre. El PDF se ve igual en cualquier equipo, porque las fuentes van incluidas en él. El PDF que **Imprimir** crea al elegir un fichero PDF también tiene los marcadores y las etiquetas. Cuando coloca varias páginas en una hoja, cada marcador y cada vínculo lleva a la hoja que contiene su página, y las páginas de una hoja se leen en el orden en que están dispuestas: de izquierda a derecha, y luego de arriba abajo.

## Guardar como página web

Haga clic en **Archivo > Exportar** y elija uno de dos tipos de página web. Una **página web** es un fichero que puede abrir en cualquier navegador web.

- **Guardar como página web** crea un único fichero .html que contiene sus palabras, su aspecto y sus imágenes. Puede enviar ese fichero por sí solo. El aspecto que conserva es la fuente, el tamaño, el color, el resaltado, la negrita, la cursiva, el subrayado, la alineación y el espacio alrededor de los párrafos.
- **Guardar como página web filtrada** crea un fichero .html pequeño con solo la estructura: encabezados, negrita y cursiva, listas, tablas, vínculos e imágenes. No tiene fuentes, tamaños ni colores, de modo que después puede darle su propio aspecto.

Ambas conservan los encabezados, las listas con viñetas y numeradas, las tablas con celdas combinadas, los vínculos y las imágenes. El texto oculto, las palabras que un cambio registrado eliminó, los comentarios, los encabezados, los pies de página y las notas se omiten. Un vínculo que iniciaría un programa o ejecutaría un script se guarda como texto sin formato.

## Guardar como libro electrónico

Un **libro electrónico** es un libro que se lee en un lector electrónico, un teléfono o una tableta. El fichero es un **EPUB**. El texto se adapta a cualquier pantalla, de modo que el lector puede aumentar o reducir las letras.

1. Haga clic en **Archivo > Exportar > Guardar como libro electrónico (EPUB)**.
2. Compruebe el **Título** y el **Autor**. Parten del título y del autor de las propiedades de su documento (**Archivo > Información**). Puede cambiarlos solo para este libro.
3. Active **Usar la primera imagen como portada** si quiere que la primera imagen del documento sea la portada. Sin esta opción, el libro no tiene portada.
4. Haga clic en **Aceptar** y luego elija dónde guardar el fichero.

Así se crea el libro:

- Cada **Título 1** inicia un capítulo nuevo. Las palabras anteriores al primer Título 1 forman un capítulo propio.
- La lista de **contenido** muestra cada Título 1, Título 2 y Título 3, y el lector puede saltar a cada uno. Los encabezados más pequeños están en el libro, pero no en el contenido.
- Las imágenes y las tablas permanecen donde las colocó. El texto alternativo de una imagen va con ella, para que un lector de pantalla pueda decir lo que muestra.
- Las notas al pie y las notas al final se colocan al final del capítulo en el que están. Cada nota tiene un vínculo de vuelta al lugar del texto, y cada número del texto es un vínculo a su nota.

Un libro tiene límites para que un fichero dañado no pueda generar uno enorme: hasta 500 capítulos, 4000 líneas en el contenido, 1000 imágenes (ninguna de más de 32 MB) y 20 000 notas. Lo que supere un límite se queda en el último capítulo o se convierte en texto sin formato, y sus palabras nunca se pierden: una imagen que supera el límite se sustituye por su texto alternativo, y una nota que lo supera no tiene formato. Una imagen que un lector no puede mostrar (un metarchivo, un mapa de bits o un TIFF) se convierte en PNG. Exportar de nuevo el mismo documento guardado crea el mismo libro, de modo que un lector conserva su lugar.

## Proteger un PDF con contraseña

En el cuadro **Exportar a PDF**, haga clic en **Cifrar con contraseña…**. Escriba la contraseña dos veces. Quien abra el PDF tendrá que escribirla. La contraseña protege realmente el fichero: las páginas se cifran con un cifrado robusto (AES-256), y nadie puede leerlas sin ella.

También puede marcar **Restringir la impresión, la copia y la edición**, establecer una segunda contraseña y elegir qué está permitido. Esas restricciones son solo una petición. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un fichero en privado.

Una contraseña olvidada no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda. El cuadro desactiva **PDF/A** mientras hay una contraseña establecida, porque un fichero PDF/A no puede tener contraseña.

## Crear un PDF/A

En el cuadro **Exportar a PDF**, marque **PDF/A, para conservar durante mucho tiempo**. Un fichero PDF/A (PDF/A-1b) incluye todas las fuentes que usa, nombra sus colores y sigue una norma creada para conservar ficheros durante muchos años. Puede tener etiquetas para lectores de pantalla, y no puede tener contraseña.

## Crear un correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico** para crear un fichero de mensaje (.eml). No se envía nada. Elige qué va en el mensaje: el documento, el documento como PDF, o las páginas mostradas como imágenes en el mensaje (las primeras 30 páginas). Abra el fichero en su programa de correo, añada a quién va dirigido y envíelo. Para un PDF, puede hacer clic en **Cifrar con contraseña…** en el cuadro; comunique la contraseña a la persona por otro medio distinto del mensaje.

## Relacionado

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
- [Usar JeffWrite con el teclado](keyboard)
