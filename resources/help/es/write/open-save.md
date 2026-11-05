# Abrir, guardar, imprimir y exportar
<!-- keywords: correo electrónico, eml, marcadores, marcadores de PDF, mayús+f5, última edición, dónde se quedó, plantillas integradas, galería, carta, informe, memorando, manuscrito, boletín, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, archivos antiguos, plantilla, dotx, recientes, anclar, nuevo, cerrar, imprimir, pdf, exportar, recuperar, recuperación automática, sin guardar, fallo, pantalla de archivo, formatos de archivo, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ott, reparar, abrir y reparar, dañado -->

Haga clic en **Archivo**, arriba a la izquierda, para ver la pantalla de Archivo. Tiene páginas para Nuevo, Abrir, Imprimir y Exportar, y comandos para Guardar y Guardar como. Presione **Esc** para volver al documento.

## Iniciar un documento nuevo

Presione **Ctrl+N** o haga clic en **Archivo > Nuevo** y luego en **Documento en blanco**. Se abre un documento nuevo en su propia ventana. Si la ventana en la que se encuentra está vacía y nunca se ha guardado, el documento nuevo ocupa su lugar.

## Abrir un documento

Presione **Ctrl+O** o haga clic en **Archivo > Abrir**. Haga clic en **Examinar** para elegir un archivo. JeffWrite abre estos tipos de archivo:

- **.docx** es un documento.
- **.dotx** es una plantilla. Una plantilla se abre como un documento nuevo creado a partir de ella, de modo que la plantilla no cambia.
- **.docm** y **.dotm** son un documento y una plantilla que pueden contener **macros**, pequeños programas que acompañan a un archivo. JeffWrite nunca las ejecuta, y las conserva tal como estaban al guardar como .docm o .dotm. Si guarda como .docx o .odt, las macros se omiten y JeffWrite se lo indica.
- **.txt** es texto sin formato, solo las palabras. Al abrirlo, JeffWrite lo lee de inmediato si es ASCII o Unicode simple. Si no lo es, JeffWrite le pregunta con qué **codificación** leerlo y muestra las primeras líneas, para que compruebe que las letras se ven bien. Una **codificación** es la forma en que un archivo guarda las letras, como UTF-8 o Windows 1252. Al guardar como .txt, elige la codificación y los finales de línea, y el archivo conserva solo las palabras: sin fuentes, imágenes ni líneas de tabla.
- **.doc** y **.dot** son los tipos antiguos de documento y plantilla. JeffWrite los lee y guarda lo que haga con ellos como un archivo .docx nuevo.
- **.odt**, **.ott** y **.fodt** son archivos OpenDocument. JeffWrite los lee todos. Guarda archivos .odt, y guarda lo que haga con un archivo .ott o .fodt como un archivo .odt nuevo. Consulte [Abrir y guardar archivos OpenDocument](open-odt).
- **.rtf** es formato de texto enriquecido. JeffWrite lo lee y lo guarda. Consulte [Abrir archivos RTF](open-rtf).

La página **Abrir** también muestra los documentos que abrió o guardó recientemente. Haga clic en uno para abrirlo. Haga clic con el botón derecho en uno para anclarlo al principio de la lista o quitarlo de ella.

Si un archivo tiene partes que JeffWrite todavía no puede editar, como el diseño de una tabla o un encabezado, las conserva. Consulte [Bienvenido a JeffWrite](start). Si una parte del archivo no se puede leer en absoluto, JeffWrite le indica lo que omitió. Entonces no sobrescribe ese archivo: Guardar pide un nombre nuevo, para que el original conserve lo que la copia perdió.

## Guardar un documento

Presione **Ctrl+S** para guardar. La primera vez, JeffWrite pide un nombre. **Guardar como** (F12) guarda una copia con un nombre nuevo. Elija **Plantilla (*.dotx)** para guardar una plantilla, **Documento habilitado para macros (*.docm)** para conservar las macros, **Texto sin formato (*.txt)** para guardar solo las palabras, o **Formato de texto enriquecido (*.rtf)** para un archivo que otros programas puedan leer. Un archivo que proviene de un tipo antiguo siempre se guarda con un nombre nuevo, para que nunca se sobrescriba el archivo original.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia**. Elija un nombre. JeffWrite escribe el documento en ese archivo, y usted sigue trabajando en el documento que tenía. Su nombre y sus cambios sin guardar siguen como estaban. Esto resulta útil cuando quiere enviar una versión a alguien y seguir escribiendo.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la página **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffWrite copia primero el archivo anterior en otro llamado "Copia de" seguido de su nombre, en la misma carpeta. Si no se puede hacer la copia, JeffWrite pregunta si desea guardar sin ella. La respuesta es No, a menos que diga Sí, y el archivo sigue como estaba. Así puede volver a la última versión guardada.

## Guardar como plantilla

Una plantilla es un documento que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla se guarda en **Mis plantillas**, y el documento en el que escribe no cambia. En **Guardar como** también puede elegir **Plantilla (*.dotx)** o **Plantilla de texto de OpenDocument (*.ott)** para guardar un archivo de plantilla donde prefiera.

Para iniciar un documento nuevo a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un documento nuevo con el texto y los estilos de la plantilla. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de Recientes. Cada forma pide un archivo, o usa el que haya seleccionado:

- **Abrir como solo lectura** abre el archivo solo para leerlo. Puede mirar, desplazarse, buscar y copiar, pero no cambiar nada. La barra de título dice "Solo lectura". Si presiona **Guardar**, JeffWrite pide un nombre nuevo, de modo que el archivo que abrió no cambia.
- **Abrir como copia** abre un documento nuevo con el texto del archivo. No tiene archivo propio, así que **Guardar** pregunta dónde colocarlo. El archivo que abrió nunca se cambia.
- **Abrir en nueva ventana** abre el archivo en una ventana propia, aunque la ventana en la que se encuentra esté vacía.
- **Abrir y reparar** sirve para un archivo .docx dañado, por ejemplo uno que se cortó al copiarlo. Consulte más abajo.

## Abrir y reparar

A veces un archivo .docx se daña. Una copia que se detuvo a medias, o un disco que cometió errores. Un archivo .docx es un conjunto de partes más pequeñas, como el texto, los estilos y las imágenes. **Abrir y reparar** lee cada parte que puede. Una parte que se cortó a mitad se conserva hasta el punto en que se cortó. Una parte que no se puede leer se omite. Si la lista de partes o los vínculos entre ellas han desaparecido, JeffWrite los vuelve a crear.

El documento reparado se abre como copia con un nombre como "Carta reparada". Un mensaje le indica qué se conservó hasta un corte y qué se omitió. El archivo dañado no cambia. Guarde la copia reparada con un nombre propio y compruébela antes de fiarse de ella.

## Recuperar trabajo no guardado

JeffWrite guarda una copia de respaldo del trabajo no guardado cada pocos minutos, como un archivo en su propia carpeta. Si el programa se cierra inesperadamente, ofrece esas copias la próxima vez que lo inicie. Haga clic en **Archivo > Abrir > Recuperar trabajo no guardado** para verlas más adelante. Puede elegir cuáles abrir, y puede guardar cada una donde prefiera.

## Empezar desde una plantilla

**Archivo > Nuevo** muestra el **Documento en blanco** y un conjunto de plantillas que vienen con JeffWrite, agrupadas por tipo: una carta, un informe, un memorando, un manuscrito y un boletín. Cada imagen es la primera página del documento que obtiene. Una **plantilla** es un documento ya preparado, con sus estilos, márgenes y textos de ejemplo. Haga clic en una y reemplace los textos de ejemplo por los suyos. Las plantillas que usted mismo guardó aparecen en **Mis plantillas**.

## Retomar donde se quedó

JeffWrite recuerda dónde estaba el cursor cuando guardó o cerró un archivo. La próxima vez que lo abra, el cursor estará allí. Presione **Mayús+F5** para ir a los lugares que editó por última vez. Cada pulsación va al anterior: los últimos tres lugares y luego vuelve al primero.

## Guardar como .odt

Un archivo .odt no puede contener todo lo que puede contener un archivo .docx. Al guardar como .odt, JeffWrite primero enumera lo que se perdería y le permite guardar un archivo .docx en su lugar. Consulte [Información](info).

## Imprimir

Presione **Ctrl+P** o haga clic en **Archivo > Imprimir**. Elija una impresora, las copias y las páginas, revise cada página en la vista previa y haga clic en **Imprimir**. Consulte [Imprimir](print).

## Crear un PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Elija qué páginas incluir: todas, la página actual o un intervalo como 1-3, 5 (p2s3 y s3 también funcionan aquí). Luego elija un nombre. El PDF se ve igual en cualquier equipo, porque las fuentes van incluidas en él. El título, el autor, el asunto y las palabras clave del documento se incorporan al PDF. Cada título se convierte en un **marcador**, una entrada de la lista que un lector de PDF muestra junto a las páginas. Haga clic en un marcador para ir a su página.

## Crear un correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico** para crear un archivo de mensaje (.eml). No se envía nada. Elija lo que va en el mensaje: el documento, el documento como PDF, o las páginas mostradas como imágenes en el mensaje (las primeras 30 páginas). Abra el archivo en su programa de correo, añada a quién va dirigido y envíelo.

## Relacionados

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
- [Usar JeffWrite con el teclado](keyboard)
