# Abrir, guardar, imprimir y exportar

<!-- keywords: fuentes en el archivo, fuentes incrustadas, fuente que falta, libro electrónico, ebook, epub, lector electrónico, capítulos, contenido, página web, html, guardar como página web, página web filtrada, correo electrónico, eml, marcadores, marcadores de PDF, mayús+f5, última edición, dónde lo dejó, plantillas integradas, galería, carta, informe, memorando, manuscrito, boletín, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, archivos antiguos, plantilla, dotx, recientes, anclar, nuevo, cerrar, imprimir, pdf, exportar, recuperar, recuperación automática, sin guardar, bloqueo, pantalla de archivo, formatos de archivo, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ott, reparar, abrir y reparar, dañado, contraseña, cifrar, proteger con contraseña -->

Haga clic en **Archivo**, arriba a la izquierda, para ver la pantalla Archivo. Tiene páginas para Nuevo, Abrir, Imprimir y Exportar, y comandos para Guardar y Guardar como. Pulse **Esc** para volver al documento.

## Iniciar un documento nuevo

Pulse **Ctrl+N**, o haga clic en **Archivo > Nuevo** y luego en **Documento en blanco**. El documento nuevo se abre en su propia ventana. Si la ventana en la que está está vacía y nunca se ha guardado, el documento nuevo ocupa su lugar.

## Abrir un documento

Pulse **Ctrl+O**, o haga clic en **Archivo > Abrir**. Haga clic en **Examinar** para elegir un archivo. JeffWrite abre estos tipos de archivo:

- **.docx** es un documento.
- **.dotx** es una plantilla. Una plantilla se abre como un documento nuevo creado a partir de ella, así que la plantilla no cambia.
- **.docm** y **.dotm** son un documento y una plantilla que pueden contener **macros**, pequeños programas que acompañan a un archivo. JeffWrite nunca las ejecuta. Las conserva tal como estaban cuando guarda como .docm o .dotm. Si guarda como .docx u .odt, las macros se quitan y JeffWrite se lo indica.
- **.txt** es texto sin formato, solo las palabras. Al abrirlo, JeffWrite lo lee de inmediato si es ASCII o Unicode simple. Si no lo es, JeffWrite le pregunta con qué codificación leerlo y muestra las primeras líneas, para que vea si las letras se ven bien. Una **codificación** es la forma en que un archivo guarda las letras, como UTF-8 o Windows 1252. Al guardar como .txt, elige la codificación y los finales de línea, y el archivo conserva solo las palabras: ni fuentes, ni imágenes, ni líneas de tabla.
- **.html** y **.htm** son páginas web. JeffWrite lee las palabras, los encabezados, la negrita, la cursiva, el subrayado, los colores, las listas y las tablas. Las viñetas y los números de las listas se importan como marcas escritas, y las imágenes y los vínculos se omiten. Nunca descarga nada de internet y 
ignora los scripts. Una página web nunca se sobrescribe: **Guardar** pide un nombre nuevo y guarda un archivo .docx. Para crear una página web, vea **Guardar como página web** más abajo.
- **.doc** y **.dot** son los tipos antiguos de documento y plantilla. JeffWrite los lee y guarda lo que haga con ellos como un archivo .docx nuevo.
- **.odt**, **.ott** y **.fodt** son archivos OpenDocument. JeffWrite lee los tres. Guarda archivos .odt, y guarda lo que haga con un archivo .ott o .fodt como un archivo .odt nuevo. Vea [Abrir y guardar archivos OpenDocument](open-odt).
- **.rtf** es formato de texto enriquecido. JeffWrite lo lee y lo guarda. Vea [Abrir archivos RTF](open-rtf).

La página **Abrir** también muestra los documentos que abrió o guardó recientemente. Haga clic en uno para abrirlo. Haga clic con el botón derecho en uno para anclarlo al principio de la lista, o quitarlo de la lista.

Si un archivo tiene partes que JeffWrite todavía no puede editar, como el diseño de una tabla o un encabezado, las conserva. Vea [Bienvenido a JeffWrite](start). Si una parte del archivo no se puede leer en absoluto, JeffWrite le indica qué ha omitido. Entonces no guarda sobre ese archivo: Guardar pide un nombre nuevo, para que el original conserve lo que la copia perdió.

## Guardar un documento

Pulse **Ctrl+S** para guardar. La primera vez, JeffWrite pide un nombre. **Guardar como** (F12) guarda una copia con un nombre nuevo. Elija **Plantilla (*.dotx)** para guardar una plantilla, **Documento habilitado para macros (*.docm)** para conservar las macros, **Texto sin formato (*.txt)** para guardar solo las palabras, o **Formato de texto enriquecido (*.rtf)** para un archivo que otros programas puedan leer. Un archivo que procede de un tipo de archivo antiguo siempre se guarda con un nombre nuevo, para que el archivo antiguo nunca se sobrescriba.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia**. Elija un nombre. JeffWrite escribe el documento en ese archivo, y usted sigue trabajando en el documento que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban. Esto resulta útil cuando quiere enviar una versión a alguien y seguir escribiendo.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la página **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffWrite copia primero el archivo antiguo en otro llamado «Copia de» más su nombre, en la misma carpeta. Si no se puede hacer la copia, JeffWrite pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo se queda como estaba. Así puede volver a la última versión guardada.

## Guardar como plantilla

Una plantilla es un documento que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla se guarda en **Mis plantillas**, y el documento en el que escribe no cambia. En **Guardar como** también puede elegir **Plantilla (*.dotx)** u **Plantilla de texto de OpenDocument (*.ott)** para guardar un archivo de plantilla donde prefiera.

Para empezar un documento nuevo a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un documento nuevo con el texto y los estilos de la plantilla. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de Recientes. Cada forma pide un archivo, o usa el que haya elegido:

- **Abrir como solo lectura** abre el archivo solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no cambiar nada. La barra de título dice «Solo lectura». Si pulsa **Guardar**, JeffWrite pide un nombre nuevo, así que el archivo que abrió no cambia.
- **Abrir como copia** abre un documento nuevo con el texto del archivo. No tiene archivo propio, así que **Guardar** pregunta dónde colocarlo. El archivo que abrió nunca cambia.
- **Abrir en nueva ventana** abre el archivo en una ventana propia, aunque la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un archivo .docx o .doc dañado, por ejemplo uno que se cortó al copiarlo. Vea más abajo.

## Abrir y reparar

A veces un archivo .docx o .doc está dañado. Una copia se detuvo a medio camino, o un disco cometió errores. Un archivo .docx es un conjunto de partes más pequeñas, como el texto, los estilos y las imágenes. **Abrir y reparar** lee cada parte que puede. Una parte que se detuvo a mitad se conserva hasta el punto donde se detuvo. Una parte que no se puede leer se omite. Si la lista de partes o los vínculos entre ellas se han perdido, JeffWrite los vuelve a crear.

Un archivo .doc antiguo es un contenedor grande con unos pocos flujos de datos dentro. Uno contiene el texto, y otro, la tabla que indica dónde está cada fragmento de texto y cómo se ve. Si el contenedor se corta o sus listas están dañadas, **Abrir y reparar** localiza esos flujos según lo que contienen y recupera todo lo que puede. Cuando las listas que indican cómo encajan los fragmentos están dañadas, el programa toma cada fragmento para seguir al anterior, y muestra lo que encuentra allí. Ese texto podría pertenecer a una copia antigua del documento o a otra parte del archivo, así que el mensaje indica el párrafo donde empieza y le pide que lo compare con una copia de seguridad si tiene una. Si la suposición llega antes del texto, el mensaje indica que se recompuso todo el documento, y si llega después, que se recompuso el formato. Si las imágenes no se pudieron recuperar por completo, el mensaje lo indica. Si se pierde la tabla, el texto se conserva como párrafos sin formato, sin estilos, listas, tablas ni imágenes. Un archivo .doc con contraseña no se puede reparar.

El documento reparado se abre como una copia con un nombre como «Carta reparada». Un mensaje le indica qué se conservó hasta una interrupción y qué se omitió. El archivo dañado no cambia. Guarde la copia reparada con un nombre propio y revísela antes de fiarse de ella.

## Recuperar trabajo sin guardar

JeffWrite guarda una copia de repuesto del trabajo sin guardar cada pocos minutos, como un archivo en su propia carpeta. Si el programa se cierra inesperadamente, ofrece esas copias la próxima vez que lo inicie. Haga clic en **Archivo > Abrir > Recuperar trabajo sin guardar** para verlas más tarde. Puede elegir cuáles abrir, y guardar cada una donde prefiera.

## Empezar desde una plantilla

**Archivo > Nuevo** muestra el **Documento en blanco** y un conjunto de plantillas que vienen con JeffWrite, agrupadas por tipo: una carta, un informe, un memorando, un manuscrito y un boletín. Cada imagen es la primera página del documento que obtendrá. Una **plantilla** es un documento ya preparado, con sus estilos, márgenes y textos de muestra. Haga clic en una y sustituya los textos de muestra por los suyos. Las plantillas que usted mismo ha guardado aparecen en **Mis plantillas**.

## Retomar donde lo dejó

JeffWrite recuerda dónde estaba el cursor cuando guardó o cerró un archivo. La próxima vez que abra el archivo, el cursor estará allí. Pulse **Mayús+F5** para ir a los lugares que editó por última vez. Cada pulsación va al anterior: los últimos tres lugares, y después vuelve al primero.

## Guardar como .odt

Un archivo .odt no puede contener todo lo que puede contener un archivo .docx. Al guardar como .odt, JeffWrite primero enumera lo que se perdería, y le permite guardar un archivo .docx en su lugar. Vea [Información](info).

## Imprimir

Pulse **Ctrl+P**, o haga clic en **Archivo > Imprimir**. Elija una impresora, las copias y las páginas, revise cada página en la vista previa y haga clic en **Imprimir**. Vea [Imprimir](print).

## Crear un PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. El PDF incluye el título, el autor, el asunto y las palabras clave del documento desde **Archivo > Información** (con **Incluir propiedades del documento** activado), la fecha de creación y JeffWrite como programa que lo creó; el autor y un título creado a partir del nombre del archivo no se incluyen cuando se elimina la información personal. Cada uno de estos datos se conserva completo hasta 4000 caracteres. Marque **Abrir el PDF después de guardarlo** para que el archivo se abra en el programa de PDF de su equipo al guardarse. Esta opción está desactivada hasta que la marque. Si su equipo no puede abrir el archivo, la barra de estado lo indica (el archivo se guarda igualmente). Elija qué páginas incluir: todas, la página actual o un intervalo como 1-3, 5 (p2s3 y s3 también funcionan aquí). Después elija cómo se crea:

- **Optimizar para** tiene **Estándar**, que conserva las imágenes tal como están, y **Tamaño mínimo**, que hace que cada imagen no tenga más de 150 píxeles por pulgada, para que el archivo sea más pequeño. Use Tamaño mínimo para un PDF que vaya a publicar en una página web o enviar por correo electrónico.
- **Incluir propiedades del documento** incorpora al PDF el título, el autor, el asunto y las palabras clave del documento.
- **Crear marcadores a partir de los encabezados** convierte cada encabezado en un **marcador**, una entrada de la lista que muestra el lector de PDF junto a las páginas. Haga clic en un marcador para ir a su página.
- **Etiquetas de estructura del documento para accesibilidad** etiqueta los encabezados, párrafos, listas, tablas (con sus filas de encabezado) e imágenes (con su texto alternativo), e indica el idioma, para que un lector de pantalla lea el PDF en orden.

Por último, elija un nombre. El PDF se ve igual en cualquier equipo, porque las fuentes van dentro del archivo. El PDF que crea **Imprimir** al elegir un archivo PDF también incluye los marcadores y las etiquetas. Cuando coloca varias páginas en una hoja, cada marcador y cada vínculo llevan a la hoja que contiene su página, y las páginas de una hoja se leen en el orden en que están colocadas: de izquierda a derecha, y luego de arriba abajo.

## Guardar como página web

Haga clic en **Archivo > Exportar** y elija uno de los dos tipos de página web. Una **página web** es un archivo que puede abrir en cualquier navegador web.

- **Guardar como página web** crea un archivo .html que contiene sus palabras, su aspecto y sus imágenes. Puede enviar ese único archivo por sí solo. El aspecto que conserva es la fuente, el tamaño, el color, el resaltado, la negrita, la cursiva, el subrayado, la alineación y el espacio entre párrafos.
- **Guardar como página web filtrada** crea un archivo .html pequeño con solo la estructura: encabezados, negrita y cursiva, listas, tablas, vínculos e imágenes. No tiene fuentes, tamaños ni colores, así que más adelante puede darle su propio aspecto.

Ambos conservan los encabezados, las listas con viñetas y numeradas, las tablas con celdas combinadas, los vínculos y las imágenes. El texto oculto, las palabras que un cambio registrado eliminó, los comentarios, los encabezados, los pies de página y las notas se omiten. Un vínculo que iniciaría un programa o ejecutaría un script se guarda como texto sin formato.

## Guardar como libro electrónico

Un **libro electrónico** es un libro que se lee en un lector electrónico, un teléfono o una tableta. El archivo es un **EPUB**. El texto se adapta a cualquier pantalla, así que el lector puede aumentar o reducir el tamaño de las letras.

1. Haga clic en **Archivo > Exportar > Guardar como libro electrónico (EPUB)**.
2. Revise el **Título** y el **Autor**. Al principio son el título y el autor de las propiedades del documento (**Archivo > Información**). Puede cambiarlos solo para este libro.
3. Active **Usar la primera imagen como portada** si quiere que la primera imagen del documento sea la portada. Si no la activa, el libro no tendrá portada.
4. Haga clic en **Aceptar** y elija dónde guardar el archivo.

Así se crea el libro:

- Cada **Título 1** inicia un capítulo nuevo. El texto que hay antes del primer Título 1 es un capítulo propio.
- La lista de **contenido** muestra todos los Título 1, Título 2 y Título 3, y el lector puede saltar a cada uno. Los encabezados más pequeños están en el libro, pero no en el contenido.
- Las imágenes y las tablas permanecen donde las colocó. El texto alternativo de una imagen la acompaña, para que un lector de pantalla pueda decir qué muestra.
- Las notas al pie y las notas al final se colocan al final del capítulo en el que están. Cada nota tiene un vínculo de vuelta al lugar del texto, y cada número del texto es un vínculo a su nota.

Un libro tiene límites para que un archivo dañado no genere uno enorme: hasta 500 capítulos, 4000 líneas en el contenido, 1000 imágenes (ninguna de más de 32 MB) y 20 000 notas. Lo que supere un límite se queda en el último capítulo o se convierte en texto sin formato, y sus palabras nunca se pierden: una imagen que supera el límite se sustituye por su texto alternativo, y una nota que lo supera no conserva su aspecto. Una imagen que un lector no puede mostrar (un metarchivo, un mapa de bits o un TIFF) se convierte en PNG. Exportar de nuevo el mismo documento guardado produce el mismo libro, así que un lector conserva su lugar.

## Proteger un PDF con contraseña

En el cuadro **Exportar a PDF**, haga clic en **Cifrar con contraseña…**. Escriba la contraseña dos veces. Quien abra el PDF deberá escribirla. La contraseña protege realmente el archivo: las páginas se cifran con un cifrado fuerte (AES-256), y nadie puede leerlas sin ella.

También puede marcar **Restringir la impresión, la copia y la edición**, escribir una segunda contraseña y elegir qué está permitido. Esas limitaciones son solo una solicitud. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un archivo privado.

Una contraseña olvidada no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda. El cuadro desactiva **PDF/A** mientras hay una contraseña establecida, porque un archivo PDF/A no puede tenerla.

## Crear un PDF/A

En el cuadro **Exportar a PDF**, marque **PDF/A, para conservar durante mucho tiempo**. Un archivo PDF/A (PDF/A-1b) incluye todas las fuentes que usa, nombra sus colores y sigue una norma pensada para conservar archivos durante muchos años. Puede tener etiquetas para lectores de pantalla, y no puede tener contraseña.

## Crear un correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico** para crear un archivo de mensaje (.eml). No se envía nada. Usted elige lo que va en el mensaje: el documento, el documento como PDF, o las páginas mostradas como imágenes en el mensaje (las primeras 30 páginas). Abra el archivo en su programa de correo, añada a quién va dirigido y envíelo. Para un PDF, puede hacer clic en **Cifrar con contraseña…** en el cuadro; comunique la contraseña a la persona por otro medio distinto del mensaje.

## Fuentes en el archivo

Un archivo .docx puede llevar consigo las fuentes con las que está compuesto su texto, para que el documento se vea como lo hizo su autor, incluso en un equipo que no tenga esas fuentes. JeffWrite las lee al abrir el archivo. Se usan solo para ese documento, mientras está abierto. Nunca se instalan en su equipo, ningún otro documento ni programa las ve, y desaparecen al cerrar el documento. Una fuente que lleva el archivo no cuenta como fuente que falta.

JeffWrite no usa una fuente cuya licencia del fabricante no permita incrustarla, una fuente dañada ni ninguna fuente más allá de las primeras 64 de un mismo archivo. Cuando ocurre, el mensaje que ve al abrir el archivo lo indica, y el texto se muestra en una fuente parecida. El archivo conserva sus fuentes tal como estaban al guardarlo.

Para incluir en los archivos que guarda las fuentes que usa su texto, haga clic en **Archivo > Opciones**, abra la página **Guardar** y active **Guardar fuentes en el archivo**. Al principio está desactivada. JeffWrite entonces incluye las fuentes que vienen con él y que usa el texto, en cada estilo que emplea el documento, cuando guarda un archivo .docx. Nunca incluye una fuente cuya licencia no lo permita, y cuando deja una fuera, el mensaje posterior al guardado la nombra. Hace el archivo más grande, y como máximo caben 64 fuentes y 256 MB en un archivo.

## Relacionado

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
- [Usar JeffWrite con el teclado](keyboard)
