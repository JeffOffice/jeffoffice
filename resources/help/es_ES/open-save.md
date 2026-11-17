# Guardar y abrir publicaciones
<!-- keywords: abrir, guardar, guardar como, guardar una copia, cerrar, archivos recientes, anclado, tipos de archivo, jpub, cambiar nombre, copia de seguridad, propiedades, información, marcar como final, inspeccionar, historial de versiones, nuevo nombre, copiar, Ctrl+S, solo lectura, abrir como copia, nueva ventana, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado, comprobar compatibilidad, guardar de todos modos, no volver a preguntar, ficheros recientes, tipos de fichero -->

JeffPub guarda su trabajo en ficheros .jpub. También abre y guarda ficheros .pub. Consulte [Trabajar con ficheros .pub](pub-files).

## Guardar

Pulse Ctrl+S, haga clic en el botón **Guardar** de la barra de herramientas rápida o haga clic en **Archivo > Guardar**. La primera vez, JeffPub le pide un nombre y una ubicación. Después, **Guardar** reemplaza el fichero con su trabajo más reciente.

Un asterisco (*) después del nombre en la barra de título indica que tiene cambios sin guardar. Guarde con frecuencia. Si JeffPub se cierra de forma inesperada, consulte [Recuperar trabajo sin guardar](recover).

Para conservar la última copia guardada de un fichero, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffPub copia primero el fichero anterior en otro llamado «Copia de» seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación pregunta si desea guardar sin ella. La respuesta predeterminada es No, a menos que diga Sí, y el fichero se mantiene como estaba. Encontrará esta opción en **Archivo > Opciones**, en la pestaña **General**. Todas las aplicaciones de JeffOffice la tienen.

## Guardar con un nuevo nombre

Haga clic en **Archivo > Guardar como**, o pulse F12. Escriba un nombre, elija **Guardar como tipo** y haga clic en **Guardar**. Las opciones son:

- **Publicación de JeffPub (*.jpub)** es la opción habitual.
- **Ficheros de publicación .pub (*.pub)** crea un fichero .pub.
- **PDF (*.pdf)** crea un PDF y deja su publicación como estaba.
- **Plantilla de JeffPub (*.jpub)** añade la publicación a **Mis plantillas**.

Después de guardar como .jpub o .pub, la barra de título muestra el nuevo nombre y usted sigue trabajando en el fichero nuevo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffPub escribe la publicación en ese fichero, y usted sigue trabajando en la que tenía. Su nombre y sus cambios sin guardar se mantienen como estaban.

## Abrir

1. Pulse Ctrl+O, o haga clic en **Archivo > Abrir**.
2. Haga clic en **Examinar** para elegir un fichero. También puede hacer clic en un nombre de **Anclados** o de **Publicaciones recientes**.

También puede arrastrar un fichero .jpub o .pub a la página. Si su publicación actual tiene cambios sin guardar, JeffPub le preguntará por ellos antes.

Haga clic con el botón derecho en un nombre de la lista para **Anclar a la lista** (se mantiene arriba) o **Quitar de la lista**. Un nombre en gris indica que JeffPub no puede encontrar ese fichero en este momento. Para cambiar cuántos nombres recientes se muestran, consulte [Cambiar la configuración de JeffPub](options).

## Otras formas de abrir un fichero

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista de recientes.

- **Abrir como solo lectura** abre la publicación solo para leerla. Puede mirar y desplazarse, pero no cambiar nada. La barra de título indica «Solo lectura». Si pulsa **Guardar**, JeffPub le pide un nombre nuevo, de modo que el fichero que abrió no se modifica.
- **Abrir como copia** abre la publicación sin fichero propio. **Guardar** pregunta dónde colocarla.
- **Abrir en nueva ventana** abre el fichero en una ventana propia.
- **Abrir y reparar** sirve para un fichero .jpub dañado, por ejemplo uno que se cortó al copiarlo. JeffPub lee todas las partes del fichero que puede y omite las que no puede leer. La publicación se abre como una copia llamada «Reparado» seguido del nombre, y un mensaje le indica lo que se omitió. El fichero dañado no se modifica.

## Cerrar

Haga clic en **Archivo > Cerrar**, o pulse Ctrl+F4. JeffPub pregunta si desea guardar los cambios y luego muestra una página en blanco.

## Información: propiedades, Marcar como final, Inspeccionar y versiones

**Archivo > Información** muestra dónde está el fichero, con **Mostrar en la carpeta** y **Copiar ruta**. También muestra datos de su publicación: páginas, tamaño de página, imágenes, palabras y fechas. Escriba en los cuadros para definir **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Organización** y **Administrador**, y haga clic en **Propiedades personalizadas** para propiedades propias. El cambio se guarda con la publicación, y **Ctrl+Z** lo deshace. La página también abre su información empresarial, la **Configuración de impresión comercial** (consulte [Ficheros PDF e imprentas](pdf)) y el [Comprobador de diseño](design-checker).

- **Comprobar compatibilidad** enumera lo que un fichero .pub no puede contener de su publicación, sin guardar nada: objetos ocultos, algunos estilos de lista y cualquier otro elemento que el escritor de .pub no incluye. Al guardar como fichero .pub, JeffPub muestra la misma lista **antes** de escribir nada. Elija **Guardar de todos modos**, **Guardar como .jpub** (el fichero .jpub se crea junto a él, y no se pierde nada) o **Cancelar**. Marque **No volver a preguntar para este fichero** y JeffPub dejará de preguntar por ese fichero hasta que cierre el programa.
- **Siempre se abre como solo lectura** hace que la publicación se abra solo para lectura cada vez. La opción se guarda con un fichero .jpub, y la barra de estado lo indica cuando alguien la abre. Para editarla, elija **Siempre se abre como solo lectura** en la misma página. Es un recordatorio y no un bloqueo. Un fichero .pub no puede contenerla (el escritor de .pub no guarda propiedades de este tipo, y JeffPub no la lee de un fichero .pub), así que guarde primero como fichero .jpub. Comprobar compatibilidad la enumera para un fichero .pub.
- **Marcar como final** guarda la publicación y luego la deja solo para lectura. Cuando alguien la abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. No es un bloqueo: elija **Marcada como final** de nuevo para editarla. Un fichero .pub no puede contener la marca, así que guarde primero como fichero .jpub.
- **Inspeccionar publicación** enumera lo que el fichero contiene y que quizá no desee compartir, y lo elimina: los comentarios y los nombres de quienes los escribieron, el autor y otros nombres de las propiedades, la información empresarial, la lista de combinación de correspondencia y los objetos guardados en el área de trabajo junto a las páginas.
- **Quitar información personal de este fichero al guardarlo**, debajo de las tarjetas, quita el autor, la persona que guardó el fichero por última vez, la organización y el administrador del fichero cada vez que guarda, tanto en un fichero .jpub como en uno .pub. Es la misma casilla que en **Opciones**, y aparece igual en todas las aplicaciones de JeffOffice.
- **Historial de versiones** guarda copias con nombre de la publicación en su equipo. Guarde primero la publicación. Haga clic en **Guardar una versión…** y después elija una para **Abrir una copia** de ella, **Volver a esta versión** (la publicación tal como está ahora se guarda antes como versión) o **Eliminar**.

Un fichero .jpub no tiene forma de contraseña, así que JeffPub no tiene tarjeta de contraseña. JeffPub todavía no tiene una comprobación de accesibilidad, por lo que la página no tiene tarjeta para ella.

## Si Windows no permite que JeffPub guarde

El Seguridad de Windows puede impedir que los programas guarden en carpetas como Documentos. En ese caso, JeffPub muestra un mensaje con los pasos para permitirlo. También puede guardar en otra carpeta.

## Temas relacionados

- [Recuperar trabajo sin guardar](recover)
- [Trabajar con ficheros .pub](pub-files)
- [Crear una publicación nueva](create)
- [Cambiar la configuración de JeffPub](options)
