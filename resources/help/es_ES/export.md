# Guardar como imagen, página web o libro electrónico
<!-- keywords: exportar, guardar como imagen, png, jpeg, gif, tiff, svg, imagen, foto, página web, html, libro electrónico, ebook, e-book, epub, diseño fijo, página a página, libro ilustrado, empaquetar, impresora fotográfica, correo electrónico, email -->

Use **Archivo > Exportar** para convertir la publicación en otro tipo de fichero. La publicación en sí no cambia. Para los PDF, vea [Ficheros PDF e imprentas](pdf). Para los ficheros .pub, vea [Trabajar con ficheros .pub](pub-files).

## Guardar páginas como imágenes

1. Haga clic en **Archivo > Exportar** y, después, en **Guardar como imagen**.
2. Escriba un nombre. En **Tipo**, elija **PNG**, **JPEG**, **GIF**, **TIFF**, **Mapa de bits** o **Dibujo vectorial SVG**.
3. Haga clic en **Guardar**.
4. Para todos los tipos excepto SVG, elija una **Resolución (puntos por pulgada)**: 96, 150, 300 o 600. Más puntos dan una imagen más nítida y un fichero más grande. 300 es una buena opción para imprimir.

Cada página se guarda en su propio fichero. Si la publicación tiene más de una página, JeffPub agrega -1, -2, etc. al nombre.

Un fichero **SVG** es un dibujo formado por líneas y curvas, por lo que se mantiene nítido a cualquier tamaño. JeffPub convierte las letras en contornos, de modo que el fichero se ve igual en un equipo que no tenga sus fuentes.

## Guardar como página web

Haga clic en **Archivo > Exportar** y, después, en **Guardar como página web**. JeffPub crea un único fichero .html que se abre en cualquier navegador web. Cada página de la publicación aparece como una imagen, por lo que su texto no se puede seleccionar ni modificar. Cada imagen incluye una versión en texto de las palabras de la página para los lectores de pantalla, y los vínculos que haya agregado siguen funcionando.

## Guardar como libro electrónico

**EPUB** es el formato estándar de libro electrónico. Lo abren los lectores de libros electrónicos, las tabletas y los teléfonos. Puede elegir entre dos tipos:

- **Texto que fluye para ajustarse a la pantalla**: el texto se redistribuye según la pantalla y el tamaño de texto de cada lector, como en una novela. Las páginas no se corresponden una a una con su diseño.
- **Cada página exactamente como se diseñó**: cada página se ve como en JeffPub, como en un libro ilustrado, una revista o un cómic. Los lectores amplían para leer el texto pequeño. El texto se conserva debajo de cada página, de modo que se puede buscar y los lectores de pantalla pueden leerlo en voz alta.

1. Haga clic en **Archivo > Exportar** y, después, en **Guardar como libro electrónico (EPUB)**.
2. Elija el tipo y compruebe el **Título** y el **Autor**.
3. Para texto que fluye, deje activada la casilla **Usar la primera página como portada** o desactívela. (Un libro página a página siempre usa su primera página como portada).
4. Haga clic en **Aceptar**, escriba un nombre de fichero y haga clic en **Guardar**.

En un libro con texto que fluye, cada párrafo con el estilo Título 1 inicia un capítulo nuevo. Los párrafos con los estilos Título 1, 2 y 3 forman la tabla de contenido en ambos tipos; un libro página a página muestra cada título en la página en la que se encuentra. Las imágenes y las tablas se mantienen en su lugar. Las notas al pie aparecen en ventanas emergentes y las notas al final van al final del libro. Vea [estilos](styles) y [tablas de contenido, notas al pie y notas al final](references).

## Empaquetar e impresiones fotográficas

- **Empaquetar: guardar para una imprenta comercial** le pide una carpeta. Guarda un PDF para imprenta comercial (imágenes a resolución completa, con marcas de recorte y las demás marcas de imprenta fuera de la página), la publicación y un breve fichero de texto con el tamaño de página y el número de páginas. Si no se puede guardar alguno de los tres, por ejemplo porque la carpeta está llena o protegida, aparece un mensaje que lo indica en lugar de confirmar que se guardó correctamente.
- **Empaquetar: guardar para otro equipo** crea un único fichero ZIP con la publicación y las fuentes incluidas con JeffPub que usa. Si no se puede guardar el ZIP, aparece un mensaje que lo indica. Las imágenes vinculadas a sus ficheros se guardan completas en la publicación. Una imagen vinculada marcada como **No actualizada** incluye la pequeña vista previa que muestra, porque JeffPub no ha abierto su fichero.
- **Guardar para una impresora fotográfica** le pide una carpeta y guarda cada página como un fichero JPEG a 300 ppp.

## Compartir por correo electrónico

Haga clic en **Archivo > Compartir**. Elija **Enviar página actual por correo electrónico**, **Enviar como datos adjuntos** o **Enviar como PDF**. JeffPub no envía nada. Guarda un fichero .eml, que es un mensaje de correo electrónico que se abre en su programa de correo. Allí puede agregar los destinatarios.

## Guardar como plantilla

**Archivo > Exportar > Guardar como plantilla** agrega la publicación a **Mis plantillas**. Vea [Crear una publicación nueva](create).

## Temas relacionados

- [Ficheros PDF e imprentas](pdf)
- [Imprimir una publicación](printing)
- [Trabajar con ficheros .pub](pub-files)
- [Abrir y guardar publicaciones](open-save)
