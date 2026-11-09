# Abrir, guardar, imprimir y exportar

<!-- keywords: página web, html, guardar como página web, página web filtrada, correo electrónico, eml, marcadores, marcadores de PDF, mayús+f5, última edición, donde se quedó, plantillas integradas, galería, carta, informe, memorando, manuscrito, boletín, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, archivos antiguos, plantilla, dotx, recientes, anclar, nuevo, cerrar, imprimir, pdf, exportar, recuperar, recuperación automática, sin guardar, bloqueo, cierre inesperado, pantalla de archivo, formatos de archivo, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ott, reparar, abrir y reparar, dañado, contraseña, cifrar, proteger con contraseña -->

Haga clic en **Archivo**, arriba a la izquierda, para ver la pantalla Archivo. Tiene páginas para Nuevo, Abrir, Imprimir y Exportar, y comandos para Guardar y Guardar como. Presione **Esc** para volver al documento.

## Iniciar un documento nuevo

Presione **Ctrl+N**, o haga clic en **Archivo > Nuevo** y luego en **Documento en blanco**. Se abre un documento nuevo en su propia ventana. Si la ventana en la que está está vacía y nunca se ha guardado, el documento nuevo ocupa su lugar.

## Abrir un documento

Presione **Ctrl+O**, o haga clic en **Archivo > Abrir**. Haga clic en **Examinar** para elegir un archivo. JeffWrite abre estos tipos de archivo:

- **.docx** es un documento.
- **.dotx** es una plantilla. Se abre como un documento nuevo creado a partir de ella, de modo que la plantilla se mantiene como está.
- **.docm** y **.dotm** son un documento y una plantilla que pueden contener **macros**, pequeños programas que vienen con un archivo. JeffWrite nunca las ejecuta, y las conserva tal como estaban cuando guarda como .docm o .dotm. Si guarda como .docx o .odt, las macros se omiten y JeffWrite se lo indica.
- **.txt** es texto sin formato, solo las palabras. Al abrirlo, JeffWrite lo lee de inmediato si es ASCII o Unicode simple. Si no lo es, JeffWrite le pregunta en qué codificación leerlo y muestra las primeras líneas, para que compruebe que las letras se ven bien. Una **codificación** es la forma en que un archivo guarda las letras, como UTF-8 o Windows 1252. Al guardar como .txt, usted elige la codificación y los finales de línea, y el archivo conserva solo las palabras: sin fuentes, imágenes ni líneas de tabla.
- **.html** y **.htm** son páginas web. JeffWrite lee las palabras, los encabezados, la negrita, la cursiva, el subrayado, los colores, las listas y las tablas. Las viñetas y los números de las listas llegan como marcas escritas, y las imágenes y los vínculos se omiten. Nunca descarga nada de internet, e ignora los scripts. Una página web nunca se sobrescribe: **Guardar** pide un nombre nuevo y guarda un archivo .docx. Para crear una página web, consulte **Guardar como página web** más abajo.
- **.doc** y **.dot** son los tipos antiguos de documento y plantilla. JeffWrite los lee y guarda lo que haga con ellos como un archivo .docx nuevo.
- **.odt**, **.ott** y **.fodt** son archivos OpenDocument. JeffWrite los lee todos. Guarda archivos .odt, y guarda lo que haga con un archivo .ott o .fodt como un archivo .odt nuevo. Consulte [Abrir y guardar archivos OpenDocument](open-odt).
- **.rtf** es Formato de texto enriquecido. JeffWrite lo lee y lo guarda. Consulte [Abrir archivos RTF](open-rtf).

La página **Abrir** también muestra los documentos que abrió o guardó recientemente. Haga clic en uno para abrirlo. Haga clic con el botón derecho en uno para anclarlo al principio de la lista o quitarlo de ella.

Si un archivo tiene partes que JeffWrite todavía no puede editar, como el diseño de una tabla o un encabezado, las conserva. Consulte [Bienvenido a JeffWrite](start). Si una parte del archivo no se puede leer en absoluto, JeffWrite le indica lo que omitió. Entonces no guarda sobre ese archivo: Guardar pide un nombre nuevo, para que el original conserve lo que la copia perdió.

## Guardar un documento

Presione **Ctrl+S** para guardar. La primera vez, JeffWrite pide un nombre. **Guardar como** (F12) guarda una copia con un nombre nuevo. Elija **Plantilla (*.dotx)** para guardar una plantilla, **Documento habilitado para macros (*.docm)** para conservar las macros, **Texto sin formato (*.txt)** para guardar solo las palabras, o **Formato de texto enriquecido (*.rtf)** para un archivo que otros programas puedan leer. Un archivo que proviene de un tipo antiguo siempre se guarda con un nombre nuevo, para que el archivo antiguo nunca se sobrescriba.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia**. Elija un nombre. JeffWrite escribe el documento en ese archivo, y usted sigue trabajando en el documento que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban. Es útil cuando quiere enviar una versión a alguien y seguir escribiendo.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la página **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffWrite primero copia el archivo anterior en otro llamado "Copia de" seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, JeffWrite pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo se mantiene como estaba. Así puede volver a la última versión guardada.

## Guardar como plantilla

Una plantilla es un documento que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla se guarda en **Mis plantillas**, y el documento que está escribiendo no cambia. En **Guardar como** también puede elegir **Plantilla (*.dotx)** o **Plantilla de texto de OpenDocument (*.ott)** para guardar un archivo de plantilla donde prefiera.

Para empezar un documento nuevo a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un documento nuevo con el texto y los estilos de la plantilla. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes. Cada forma pide un archivo o usa el que haya elegido:

- **Abrir como solo lectura** abre el archivo solo para leerlo. Puede ver, desplazarse, buscar y copiar, pero no cambiar nada. La barra de título dice "Solo lectura". Si presiona **Guardar**, JeffWrite pide un nombre nuevo, así que el archivo que abrió no se cambia.
- **Abrir como copia** abre un documento nuevo con el texto del archivo. No tiene archivo propio, así que **Guardar** pregunta dónde colocarlo. El archivo que abrió nunca se cambia.
- **Abrir en una nueva ventana** abre el archivo en una ventana propia, aunque la ventana en la que está esté vacía.
- **Abrir y reparar** sirve para un archivo .docx dañado, por ejemplo uno que se cortó al copiarlo. Consulte más abajo.

## Abrir y reparar

A veces un archivo .docx se daña. Una copia se interrumpió a mitad, o un disco cometió errores. Un archivo .docx es un conjunto de partes más pequeñas, como el texto, los estilos y las imágenes. **Abrir y reparar** lee cada parte que puede. Una parte que se interrumpió a mitad se conserva hasta el punto donde se cortó. Una parte que no se puede leer se omite. Si se han perdido la lista de partes o los vínculos entre ellas, JeffWrite los vuelve a crear.

El documento reparado se abre como una copia con un nombre como "Carta reparada". Un mensaje le indica qué se conservó hasta una interrupción y qué se omitió. El archivo dañado no se cambia. Guarde la copia reparada con un nombre propio y revísela antes de confiar en ella.

## Recuperar trabajo sin guardar

JeffWrite guarda una copia de respaldo del trabajo sin guardar cada pocos minutos, como un archivo en su propia carpeta. Si el programa se cierra inesperadamente, las ofrece la próxima vez que lo inicie. Haga clic en **Archivo > Abrir > Recuperar trabajo sin guardar** para verlas más adelante. Puede elegir cuáles abrir, y guardar cada una donde prefiera.

## Empezar desde una plantilla

**Archivo > Nuevo** muestra **Documento en blanco** y un conjunto de plantillas que vienen con JeffWrite, agrupadas por tipo: carta, informe, memorando, manuscrito y boletín. Cada imagen es la primera página del documento que obtendrá. Una **plantilla** es un documento ya preparado, con sus estilos, márgenes y textos de muestra. Haga clic en una y reemplace los textos de muestra por los suyos. Las plantillas que usted mismo guardó aparecen en **Mis plantillas**.

## Retomar donde se quedó

JeffWrite recuerda dónde estaba el cursor al guardar o cerrar un archivo. La próxima vez que lo abra, el cursor estará allí. Presione **Mayús+F5** para ir a los lugares que editó por última vez. Cada pulsación lleva al anterior: los últimos tres lugares y, luego, de nuevo al primero.

## Guardar como .odt

Un archivo .odt no puede contener todo lo que puede contener un archivo .docx. Al guardar como .odt, JeffWrite primero muestra lo que se perdería y le permite guardar un archivo .docx en su lugar. Consulte [Información](info).

## Imprimir

Presione **Ctrl+P**, o haga clic en **Archivo > Imprimir**. Elija una impresora, las copias y las páginas, revise cada página en la vista previa y haga clic en **Imprimir**. Consulte [Imprimir](print).

## Crear un PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Elija qué páginas incluir: todas, la página actual o un intervalo como 1-3, 5 (p2s3 y s3 también funcionan aquí). Después, elija un nombre. El PDF se ve igual en cualquier equipo, porque las fuentes se incluyen dentro de él. El título, el autor, el asunto y las palabras clave del documento pasan al PDF. Cada encabezado se convierte en un **marcador**, una entrada de la lista que un lector de PDF muestra junto a las páginas. Haga clic en un marcador para ir a su página.

## Guardar como página web

Haga clic en **Archivo > Exportar** y elija uno de los dos tipos de página web. Una **página web** es un archivo que puede abrir en cualquier navegador web.

- **Guardar como página web** crea un único archivo .html que contiene sus palabras, su aspecto y sus imágenes. Puede enviar ese único archivo por separado. El aspecto que conserva es la fuente, el tamaño, el color, el resaltado, la negrita, la cursiva, el subrayado, la alineación y el espacio alrededor de los párrafos.
- **Guardar como página web filtrada** crea un archivo .html pequeño con solo la estructura: encabezados, negrita y cursiva, listas, tablas, vínculos e imágenes. No tiene fuentes, tamaños ni colores, de modo que después puede darle su propio aspecto.

Ambos conservan los encabezados, las listas con viñetas y numeradas, las tablas con celdas combinadas, los vínculos y las imágenes. El texto oculto, las palabras que un cambio registrado eliminó, los comentarios, los encabezados, los pies de página y las notas se omiten. Un vínculo que iniciaría un programa o ejecutaría un script se guarda como texto sin formato.

## Proteger un PDF con contraseña

En el cuadro **Exportar a PDF**, haga clic en **Cifrar con contraseña…**. Escriba la contraseña dos veces. Quien abra el PDF tendrá que escribirla. La contraseña protege de verdad el archivo: las páginas se cifran con un cifrado robusto (AES-256), y nadie puede leerlas sin ella.

También puede marcar **Restringir la impresión, la copia y la edición**, indicar una segunda contraseña y elegir qué se permite. Esas restricciones son solo una petición. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un archivo en privado.

Una contraseña olvidada no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda. El cuadro desactiva **PDF/A** mientras hay una contraseña establecida, porque un archivo PDF/A no puede tenerla.

## Crear un PDF/A

En el cuadro **Exportar a PDF**, marque **PDF/A, para conservar durante mucho tiempo**. Un archivo PDF/A incluye todas las fuentes que utiliza y sigue una norma pensada para conservar archivos durante muchos años. Todavía no está etiquetado para lectores de pantalla, y no puede tener contraseña.

## Crear un correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico** para crear un archivo de mensaje (.eml). No se envía nada. Elija qué incluir en el mensaje: el documento, el documento como PDF o las páginas mostradas como imágenes en el mensaje (las primeras 30 páginas). Abra el archivo en su programa de correo, añada a quién va dirigido y envíelo. Para un PDF, puede hacer clic en **Cifrar con contraseña…** en el cuadro; comunique la contraseña a la persona por otro medio que no sea el propio mensaje.

## Relacionados

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
- [Usar JeffWrite con el teclado](keyboard)
