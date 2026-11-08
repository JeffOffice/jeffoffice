# Abrir y guardar ficheros OpenDocument
<!-- keywords: odt, fodt, ott, opendocument, open document, documento abierto, abrir, guardar, guardar como, formato, tipo de archivo, otros programas, intercambiar documentos, compartir, compatibilidad, control de cambios, cambios realizados, omitido, contenido perdido, plantilla, tipo de fichero -->

JeffWrite abre y guarda texto OpenDocument, el tipo de fichero cuyo nombre termina en **.odt**. Muchos programas gratuitos lo leen y lo escriben, así que es una buena forma de intercambiar un documento con alguien que no usa JeffWrite.

## Abrir un fichero OpenDocument

Pulse **Ctrl+A**, o haga clic en **Archivo > Abrir**, y elija un fichero:

- **.odt** es un documento.
- **.ott** es una plantilla. Se abre como un documento nuevo creado a partir de ella, de modo que la plantilla no cambia.
- **.fodt** es el mismo tipo de documento, guardado en un único fichero XML sin formato. JeffWrite no escribe ficheros .fodt: al usar **Guardar** en uno, se le pide un nombre nuevo y se guarda como fichero .odt.

JeffWrite averigua qué es un fichero por su contenido, no por su nombre, así que también se abre un fichero con una extensión incorrecta. También puede indicar el nombre de un fichero al iniciar JeffWrite, o arrastrarlo desde el explorador de ficheros hasta el programa, igual que con un fichero .docx.

JeffWrite lee el texto y su formato, los títulos, los estilos y los estilos en los que se basan, las listas con viñetas y numeradas, las tablas (con celdas combinadas, anchos de columna, bordes, sombreado y tablas dentro de tablas), las secciones y columnas, el tamaño, los márgenes y la dirección de la página, los encabezados y pies de página (primera página, izquierda y derecha), las notas al pie y las notas al final, los campos (números de página, número total de páginas, título, autor, nombre de fichero, referencias cruzadas y números de secuencia), los marcadores, los vínculos, la tabla de contenido, las imágenes, los cuadros de texto, las tabulaciones, los comentarios y los cambios controlados, que aparecen como marcas que puede aceptar o rechazar.

Hay cosas que JeffWrite todavía no puede mostrar: formas dibujadas que no contienen texto, fórmulas, gráficos y otros objetos insertados en el texto. Si un fichero contiene alguna, JeffWrite le indica cuáles al abrirlo. Después, no guarda encima de ese fichero: **Guardar** le pide un nombre nuevo, para que el original conserve lo que la copia perdió. Los documentos protegidos con contraseña no se abren. Ábralo en el programa con el que se creó y guarde una copia sin la contraseña.

## Guardar un fichero OpenDocument

Haga clic en **Archivo > Guardar como** y elija **Texto OpenDocument (*.odt)** en la lista de tipos de fichero. A partir de entonces, **Ctrl+G** guarda de nuevo en el mismo tipo.

El fichero contiene todo lo que se indica arriba, con sus comentarios y sus cambios controlados: texto agregado y eliminado, cambios de formato y saltos de párrafo agregados o eliminados. Para algunas cosas el formato no tiene sitio, y JeffWrite le indica cuáles son después de guardar: se guardan tal como están en ese momento o se omiten. Son los cambios controlados en tablas, columnas y configuración de página, y los cambios controlados en encabezados, pies de página y notas (se guardan tal como están); los campos de formulario (se guardan como el texto que muestran); las ecuaciones (se guardan como texto sin formato); y los dibujos, formas, gráficos y objetos incrustados que no se pueden conservar. Si vuelve a guardar después de un cambio que omite algo nuevo, JeffWrite también se lo indica, y no repite lo que ya le había dicho.

Otros programas que leen OpenDocument abren el fichero sin pedir que se repare.

## Temas relacionados

- [Abrir, guardar, imprimir y exportar](open-save)
- [Controlar cambios y aceptarlos o rechazarlos](track-changes)
- [Le damos la bienvenida a JeffWrite](start)
