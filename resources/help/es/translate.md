# Traducir texto

<!-- keywords: traducir, traducción, traductor, idioma, idiomas, español, francés, alemán, italiano, portugués, neerlandés, ruso, ucraniano, indonesio, sin conexión, en este equipo, descargar, modelos, reemplazar, insertar debajo -->

**Revisar > Idioma > Traducir** abre el panel **Traducir**. Traduce el texto en su propio equipo. Sus palabras no se envían a ninguna parte, y no hace falta conexión a internet una vez que los archivos de traducción de un idioma están en el equipo.

## Traducir un texto

1. Seleccione el texto que desea traducir. Si no hay nada seleccionado, el panel usa el párrafo en el que está el cursor. En un cuadro de texto seleccionado pero que no se está editando, usa todo el texto del cuadro.
2. Haga clic en **Revisar > Idioma > Traducir**. El panel muestra su texto en **Original**.
3. En **Desde**, elija el idioma en que está su texto. En **A**, elija el idioma al que desea traducirlo. El botón **Intercambiar** invierte los dos.
4. Haga clic en **Traducir**. La traducción aparece en **Traducción**. Las palabras en negrita, cursiva y subrayado siguen en negrita, cursiva y subrayado.
5. Haga clic en **Reemplazar** para poner la traducción donde estaba su texto, o en **Insertar debajo** para añadirla en párrafos nuevos debajo de su texto. Cualquiera de las dos es un solo paso, así que **Deshacer** la revierte.

Si mueve el cursor o cambia el texto después de hacer clic en **Traducir**, haga clic en **Usar selección** para volver a leer el texto y, después, en **Traducir** otra vez. El panel no colocará una traducción sobre un texto que ya no es el que se tradujo.

**Cancelar** detiene una traducción que tarda demasiado, y también lo hace cerrar el panel: la ventana nunca espera a que termine. Los textos largos tardan más: una página de texto tarda unos segundos en la mayoría de los equipos. El panel traduce como máximo 1 000 000 de letras a la vez (unas 200 páginas); si selecciona más, lo indica y le pide que seleccione menos. Los párrafos largos se dividen en fragmentos de unas 1500 letras, al final de una oración siempre que sea posible, y después se vuelven a unir.

## La primera vez que usa un idioma

Una traducción necesita un conjunto de archivos, llamado modelo, para cada dirección: uno para del inglés al alemán y otro para del alemán al inglés. Cada modelo ocupa unos 37 MB. Los archivos no forman parte de JeffPub. La primera vez que traduce con un idioma, el panel le indica qué necesita y de qué tamaño es, por ejemplo «¿Descargar alemán (74 MB)?». No se descarga nada hasta que hace clic en ese botón. El panel muestra el avance de la descarga, y **Cancelar** la detiene y no deja nada a medio hacer.

JeffPub comprueba cada archivo con una huella digital (una suma de comprobación SHA-256) antes de usarlo. Un archivo que no coincide se descarta y el idioma no se instala.

Los archivos se guardan en una sola carpeta que comparten todas las aplicaciones de JeffOffice (una carpeta llamada JeffOffice dentro de la carpeta de datos de usuario, en Translation/models), de modo que un idioma que descargue en una aplicación estará listo en las demás, y solo se descarga una vez. Dos idiomas que no tienen modelo propio, como el alemán y el francés, pasan por el inglés, así que el panel necesita los dos modelos.

## Cuando un idioma no tiene modelo

Si una pareja de idiomas no tiene modelo, el panel lo indica y ofrece **Abrir en su navegador**. Eso abre un sitio web de traducción gratuito, LibreTranslate (o el servicio cuya dirección haya elegido en la configuración correspondiente), con su texto. Solo entonces su texto sale de su equipo, porque se envía a ese sitio web, y el panel ya no indica que se queda en su equipo. No se envía nada hasta que hace clic en ese botón.

## Sobre la calidad

Los modelos son pequeños, así que funcionan con rapidez en un equipo corriente. Traducen bien las oraciones y peor las etiquetas cortas: una sola palabra o el nombre de un botón puede salir mal. Lea la traducción antes de usarla. Los nombres, los números y las pestañas se conservan tal como estaban.

Las imágenes, las pestañas y los saltos de línea dentro de un párrafo no se modifican. La traducción rellena las palabras que hay entre ellos.

## Temas relacionados

- [Ortografía, sinónimos, investigación y división de palabras](proofing)
