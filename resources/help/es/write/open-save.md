# Abrir, guardar, imprimir y exportar
<!-- keywords: abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, archivos antiguos, plantilla, dotx, recientes, anclar, nuevo, cerrar, imprimir, pdf, exportar, recuperar, recuperación automática, sin guardar, bloqueo, cierre inesperado, pantalla Archivo, formatos de archivo, guardar una copia, solo lectura, abrir como copia, nueva ventana, copia de seguridad, mis plantillas, guardar como plantilla, ott, reparar, abrir y reparar, dañado -->

Haga clic en **Archivo** en la parte superior izquierda para ver la pantalla Archivo. Tiene páginas para Nuevo, Abrir, Imprimir y Exportar, y comandos para Guardar y Guardar como. Presione **Esc** para volver al documento.

## Iniciar un documento nuevo

Presione **Ctrl+N** o haga clic en **Archivo > Nuevo** y, después, en **Documento en blanco**. El documento nuevo se abre en su propia ventana. Si la ventana en la que se encuentra está vacía y nunca se ha guardado, el documento nuevo ocupa su lugar.

## Abrir un documento

Presione **Ctrl+A** o haga clic en **Archivo > Abrir**. Haga clic en **Examinar** para elegir un archivo. JeffWrite abre estos tipos de archivo:

- **.docx** es un documento.
- **.dotx** es una plantilla. Una plantilla se abre como un documento nuevo creado a partir de ella, de modo que la plantilla no cambia.
- **.docm** y **.dotm** también se abren. JeffWrite no conserva sus macros. Se lo indica la primera vez que guarda un documento así, y guarda un archivo .docx.
- **.doc** y **.dot** son los formatos antiguos de documento y plantilla. JeffWrite los lee y guarda lo que haga con ellos como un archivo .docx nuevo.
- **.odt**, **.ott** y **.fodt** son archivos OpenDocument. JeffWrite lee los tres. Guarda los archivos .odt y guarda lo que haga con un archivo .ott o .fodt como un archivo .odt nuevo. Consulte [Abrir y guardar archivos OpenDocument](open-odt).
- **.rtf** es formato de texto enriquecido. JeffWrite lo lee y guarda lo que haga con él como un archivo .docx nuevo. Consulte [Abrir archivos RTF](open-rtf).

La página **Abrir** también muestra los documentos que abrió o guardó recientemente. Haga clic en uno para abrirlo. Haga clic con el botón derecho en uno para anclarlo a la parte superior de la lista o quitarlo de ella.

Si un archivo tiene partes que JeffWrite todavía no puede editar, como el diseño de una tabla o un encabezado, las conserva. Consulte [Bienvenido a JeffWrite](start). Si una parte del archivo no se puede leer en absoluto, JeffWrite le indica qué ha omitido. Entonces no guarda sobre ese archivo: Guardar pide un nombre nuevo, para que el original conserve lo que la copia perdería.

## Guardar un documento

Presione **Ctrl+S** para guardar. La primera vez, JeffWrite pide un nombre. **Guardar como** (F12) guarda una copia con un nombre nuevo. Elija **Plantilla (*.dotx)** para guardar una plantilla. Un archivo que procede de un formato antiguo siempre se guarda con un nombre nuevo, para que el archivo antiguo nunca se sobrescriba.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia**. Elija un nombre. JeffWrite escribe el documento en ese archivo, y usted sigue trabajando en el documento que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban. Resulta útil cuando quiere enviar una versión a alguien y seguir escribiendo.

## Conservar una copia de seguridad

Haga clic en **Archivo > Opciones** y, en la página **General**, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffWrite primero copia el archivo anterior en otro llamado "Copia de seguridad de" seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, JeffWrite pregunta si desea guardar sin ella. La respuesta predeterminada es No, a menos que diga Sí, y el archivo se queda como estaba. Así puede volver a la última versión guardada.

## Guardar como plantilla

Una plantilla es un documento que usa como punto de partida. Haga clic en **Archivo > Guardar como plantilla** y escriba un nombre. La plantilla se guarda en **Mis plantillas**, y el documento en el que está escribiendo no cambia. En **Guardar como** también puede elegir **Plantilla (*.dotx)** o **Plantilla de texto de OpenDocument (*.ott)** para guardar un archivo de plantilla en la ubicación que prefiera.

Para empezar un documento nuevo a partir de su plantilla, haga clic en **Archivo > Nuevo**. Sus plantillas aparecen en **Mis plantillas**. Haga clic en una. Se abre un documento nuevo con el texto y los estilos de la plantilla. Para quitar una plantilla de la lista, haga clic con el botón derecho en ella y elija **Quitar de Mis plantillas**.

## Otras formas de abrir un archivo

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista Recientes. Cada forma pide un archivo o usa el que haya elegido:

- **Abrir como solo lectura** abre el archivo solo para leerlo. Puede mirarlo, desplazarse por él, buscar y copiar, pero no cambiar nada. La barra de título dice "Solo lectura". Si presiona **Guardar**, JeffWrite pide un nombre nuevo, de modo que el archivo que abrió no se modifica.
- **Abrir como copia** abre un documento nuevo con el texto del archivo. No tiene archivo propio, así que **Guardar** pregunta dónde guardarlo. El archivo que abrió nunca se modifica.
- **Abrir en nueva ventana** abre el archivo en una ventana propia, incluso cuando la ventana en la que se encuentra está vacía.
- **Abrir y reparar** sirve para un archivo .docx dañado, por ejemplo uno que se cortó al copiarlo. Consulte más abajo.

## Abrir y reparar

A veces un archivo .docx está dañado. Una copia se detuvo a medio camino, o un disco cometió errores. Un archivo .docx es un conjunto de partes más pequeñas, como el texto, los estilos y las imágenes. **Abrir y reparar** lee todas las partes que puede. Una parte que se interrumpió se conserva hasta el punto donde se cortó. Una parte que no se puede leer se omite. Si se han perdido la lista de partes o los vínculos entre ellas, JeffWrite los vuelve a crear.

El documento reparado se abre como copia con un nombre como "Carta reparada". Un mensaje le indica qué se conservó hasta una interrupción y qué se omitió. El archivo dañado no se modifica. Guarde la copia reparada con un nombre propio y revísela antes de fiarse de ella.

## Recuperar trabajo sin guardar

JeffWrite guarda una copia de respaldo del trabajo sin guardar cada pocos minutos, como un archivo en su propia carpeta. Si el programa se cierra inesperadamente, ofrece esas copias la próxima vez que lo inicie. Haga clic en **Archivo > Abrir > Recuperar trabajo sin guardar** para verlas más tarde. Puede elegir cuáles abrir y guardar cada una donde prefiera.

## Imprimir

Presione **Ctrl+P** o haga clic en **Archivo > Imprimir**. Elija una impresora, el número de copias y las páginas, revise cada página en la vista previa y haga clic en **Imprimir**. Consulte [Imprimir](print).

## Crear un PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Elija un nombre. El PDF se ve igual en cualquier equipo, porque las fuentes van incluidas en el archivo.

## Relacionado

- [Bienvenido a JeffWrite](start)
- [Imprimir](print)
- [Usar JeffWrite con el teclado](keyboard)
