# Abrir, guardar, imprimir y exportar

<!-- keywords: página web, html, guardar como página web, página web filtrada, correo electrónico, eml, marcadores, marcadores de PDF, mayús+f5, última edición, donde lo dejó, plantillas integradas, galería, carta, informe, memorando, manuscrito, boletín, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, archivos antiguos, plantilla, dotx, recientes, anclar, nuevo, cerrar, imprimir, pdf, exportar, recuperar, recuperación automática, sin guardar, bloqueo, cierre inesperado, pantalla Archivo, formatos de archivo, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ott, reparar, abrir y reparar, dañado -->

Haga clic en **Archivo**, arriba a la izquierda, para ver la pantalla Archivo. Tiene las páginas Nuevo, Abrir, Imprimir y Exportar, y los comandos Guardar y Guardar como. Presione **Esc** para volver al documento.

## Crear un documento nuevo

Presione **Ctrl+N** o haga clic en **Archivo > Nuevo** y luego en **Documento en blanco**. El documento nuevo se abre en su propia ventana. Si la ventana en la que se encuentra está vacía y nunca se ha guardado, el documento nuevo ocupa su lugar.

## Abrir un documento

Presione **Ctrl+O** o haga clic en **Archivo > Abrir**. Haga clic en **Examinar** para elegir un archivo. JeffWrite abre estos tipos de archivo:

- **.docx** es un documento.
- **.dotx** es una plantilla. Una plantilla se abre como un documento nuevo creado a partir de ella, de modo que la plantilla no cambia.
- **.docm** y **.dotm** son un documento y una plantilla que pueden contener **macros**, pequeños programas que acompañan a un archivo. JeffWrite nunca las ejecuta y las conserva tal como estaban cuando guarda como .docm o .dotm. Si guarda como .docx o .odt, las macros se omiten y JeffWrite se lo indica.
- **.txt** es texto sin formato, solo las palabras. Al abrirlo, JeffWrite lo lee de inmediato si es ASCII o Unicode simple. Si no lo es, JeffWrite le pregunta en qué codificación leerlo y muestra las primeras líneas para que compruebe que las letras se ven bien. Una **codificación** es la forma en que un archivo guarda las letras, como UTF-8 o Windows 1252. Al guardar como .txt, usted elige la codificación y los finales de línea, y el archivo conserva solo las palabras: sin fuentes, imágenes ni líneas de tabla.
- **.html** y **.htm** son páginas web. JeffWrite lee las palabras, los encabezados, la negrita, la cursiva, el subrayado, los colores, las listas y las tablas. Las viñetas y los números de las listas se importan como marcas escritas, y las imágenes y los vínculos se omiten. Nunca descarga nada de internet e ignora los scripts. Una página web nunca se sobrescribe: **Guardar** pide un nombre nuevo y guarda un archivo .docx. Para crear una página web, vea **Guardar como página web** más abajo.
- **.doc** y **.dot** son los tipos antiguos de documento y plantilla. JeffWrite los lee y guarda lo que haga con ellos como un archivo .docx nuevo.
- **.odt**, **.ott** y **.fodt** son archivos OpenDocument. JeffWrite los lee todos. Guarda archivos .odt y guarda lo que haga con un archivo .ott o .fodt como un archivo .odt nuevo. Vea [Abrir y guardar archivos OpenDocument](open-odt).
- **.rtf** es formato de texto enriquecido. JeffWrite lo lee y lo guarda. Vea [Abrir archivos RTF](open-rtf).

La página **Abrir** también muestra los documentos que abrió o guardó recientemente. Haga clic en uno para abrirlo. Haga clic con el botón derecho en uno para anclarlo al principio de la lista o quitarlo de ella.

Si un archivo tiene partes que JeffWrite todavía no puede editar, como el diseño de una tabla o un encabezado, las conserva. Vea [Bienvenido a JeffWrite](start). Si una parte del archivo no se puede leer en absoluto, JeffWrite le indica qué ha omitido. Entonces no sobrescribe ese archivo: Guardar pide un nombre nuevo, para que el original conserve lo que la copia perdió.

## Guardar un documento

Presione **Ctrl+S** para guardar. La primera vez, JeffWrite pide un nombre. **Guardar como** (F12) guarda una copia con un nombre nuevo. Elija **Plantilla (*.dotx)** para guardar una plantilla, **Documento habilitado para macros (*.docm)** para conservar las macros, **Texto sin formato (*.txt)** para guardar solo las palabras, o **Formato de texto enriquecido (*.rtf)** para un archivo que otros programas puedan leer. Un archivo procedente de un tipo antiguo siempre se guarda con un nombre nuevo, de modo que el archivo antiguo nunca se sobrescribe.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia**. Elija un nombre. JeffWrite escribe el documento en ese archivo y usted sigue trabajando en el documento que tenía. Su nombre y sus cambios sin guardar permanecen como estaban. Resulta útil cuando quiere enviar una versión a alguien y seguir escribiendo.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la página **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffWrite copia primero el archivo antiguo en otro llamado "Copia de" seguido de su nombre, en la misma carpeta. Si no se puede hacer la copia, JeffWrite pregunta si desea guardar sin ella. La respuesta es No, salvo que diga Sí, y el archivo sigue como estaba. Así puede volver a la última versión guardada.

## Guardar como plantilla

Una plantilla es un documento que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla se guarda en **Mis plantillas** y el documento en el que escribe no cambia. En **Guardar como** también puede elegir **Plantilla (*.dotx)** o **Plantilla de texto de OpenDocument (*.ott)** para guardar un archivo de plantilla donde prefiera.

Para empezar un documento nuevo a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un documento nuevo con el texto y los estilos de la plantilla. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes. Cada forma pide un archivo o usa el que haya elegido:

- **Abrir como solo lectura** abre el archivo solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no cambiar nada. La barra de título dice "Solo lectura". Si presiona **Guardar**, JeffWrite pide un nombre nuevo, de modo que el archivo abierto no se modifica.
- **Abrir como copia** abre un documento nuevo con el texto del archivo. No tiene archivo propio, de modo que **Guardar** pregunta dónde colocarlo. El archivo abierto nunca se modifica.
- **Abrir en una nueva ventana** abre el archivo en una ventana propia, incluso cuando la ventana en la que se encuentra está vacía.
- **Abrir y reparar** sirve para un archivo .docx dañado, por ejemplo uno que se cortó al copiarlo. Vea más abajo.

## Abrir y reparar

A veces un archivo .docx está dañado. Una copia se interrumpió a medias o un disco cometió errores. Un archivo .docx es un conjunto de partes más pequeñas, como el texto, los estilos y las imágenes. **Abrir y reparar** lee todas las partes que puede. Una parte que se cortó a mitad se conserva hasta el punto en que se interrumpió. Una parte que no se puede leer se omite. Si la lista de partes o los vínculos entre ellas han desaparecido, JeffWrite los vuelve a crear.

El documento reparado se abre como copia con un nombre como "Carta reparada". Un mensaje le indica qué se conservó hasta un corte y qué se omitió. El archivo dañado no se modifica. Guarde la copia reparada con un nombre propio y revísela antes de fiarse de ella.

## Recuperar trabajo sin guardar

JeffWrite guarda una copia de respaldo del trabajo sin guardar cada pocos minutos, como archivo en su propia carpeta. Si el programa se cierra inesperadamente, se las ofrece la próxima vez que lo inicie. Haga clic en **Archivo > Abrir > Recuperar trabajo sin guardar** para verlas más tarde. Puede elegir cuál abrir y guardar cada una donde prefiera.

## Empezar desde una plantilla

**Archivo > Nuevo** muestra el **Documento en blanco** y un conjunto de plantillas que vienen con JeffWrite, agrupadas por tipo: carta, informe, memorando, manuscrito y boletín. Cada imagen es la primera página del documento que obtendrá. Una **plantilla** es un documento ya preparado, con sus estilos, márgenes y texto de muestra. Haga clic en una y sustituya el texto de muestra por el suyo. Las plantillas que guardó usted mismo aparecen en **Mis plantillas**.

## Continuar donde lo dejó

JeffWrite recuerda dónde estaba el cursor cuando guardó o cerró un archivo. La próxima vez que lo abra, el cursor estará allí. Presione **Mayús+F5** para ir a los lugares que editó por última vez. Cada pulsación lleva al anterior: los últimos tres lugares y, después, vuelve al primero.

## Guardar como .odt

Un archivo .odt no puede contener todo lo que puede contener un archivo .docx. Al guardar como .odt, JeffWrite primero enumera lo que se perdería y le permite guardar un archivo .docx en su lugar. Vea [Información](info).

## Imprimir

Presione **Ctrl+P** o haga clic en **Archivo > Imprimir**. Elija una impresora, las copias y las páginas, revise cada página en la vista previa y haga clic en **Imprimir**. Vea [Imprimir](print).

## Crear un PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Elija qué páginas incluir: todas, la página actual o un intervalo como 1-3, 5 (p2s3 y s3 también funcionan aquí). Después elija un nombre. El PDF se ve igual en cualquier equipo, porque las fuentes van incluidas en él. El título, el autor, el asunto y las palabras clave del documento pasan al PDF. Cada encabezado se convierte en un **marcador**, una entrada de la lista que un lector de PDF muestra junto a las páginas. Haga clic en un marcador para ir a su página.

## Guardar como página web

Haga clic en **Archivo > Exportar** y elija uno de los dos tipos de página web. Una **página web** es un archivo que puede abrir en cualquier navegador web.

- **Guardar como página web** crea un único archivo .html que contiene sus palabras, su aspecto y sus imágenes. Puede enviar ese único archivo por separado. El aspecto que conserva es la fuente, el tamaño, el color, el resaltado, la negrita, la cursiva, el subrayado, la alineación y el espacio alrededor de los párrafos.
- **Guardar como página web filtrada** crea un archivo .html pequeño con solo la estructura: encabezados, negrita y cursiva, listas, tablas, vínculos e imágenes. No tiene fuentes, tamaños ni colores, de modo que después puede darle su propio aspecto.

Ambos conservan los encabezados, las listas con viñetas y numeradas, las tablas con celdas combinadas, los vínculos y las imágenes. El texto oculto, las palabras que un cambio registrado eliminó, los comentarios, los encabezados, los pies de página y las notas se omiten. Un vínculo que iniciaría un programa o ejecutaría un script se guarda como texto sin formato.

## Crear un PDF/A

En el cuadro **Exportar a PDF**, marque **PDF/A, para conservar durante mucho tiempo**. Un archivo PDF/A incluye todas las fuentes que utiliza y sigue una norma pensada para conservar archivos durante muchos años. Todavía no está etiquetado para lectores de pantalla y no tiene contraseña.

## Crear un correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico** para crear un archivo de mensaje (.eml). No se envía nada. Elija qué incluir en el mensaje: el documento, el documento como PDF o las páginas mostradas como imágenes dentro del mensaje (las primeras 30 páginas). Abra el archivo en su programa de correo, añada a quién va dirigido y envíelo.

## Temas relacionados

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
- [Usar JeffWrite con el teclado](keyboard)
