# Guardar y abrir publicaciones
<!-- keywords: abrir, guardar, guardar como, guardar una copia, cerrar, archivos recientes, anclado, tipos de archivo, jpub, cambiar nombre, copia de seguridad, propiedades, información, marcar como final, inspeccionar, historial de versiones, nuevo nombre, copiar, Ctrl+G, solo lectura, abrir como solo lectura, abrir como copia, nueva ventana, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado, comprobar compatibilidad, guardar de todos modos, no volver a preguntar, ficheros recientes, tipos de fichero -->

JeffPub guarda su trabajo en ficheros .jpub. También abre y guarda ficheros .pub. Consulte [Trabajar con ficheros .pub](pub-files).

## Guardar

Pulse Ctrl+G, haga clic en el botón **Guardar** de la barra de herramientas rápida o haga clic en **Archivo > Guardar**. La primera vez, JeffPub le pide un nombre y una ubicación. Después, Guardar reemplaza el fichero con su trabajo más reciente.

Un asterisco (*) después del nombre en la barra de título indica que hay cambios sin guardar. Guarde con frecuencia. Si JeffPub se detiene inesperadamente, consulte [Recuperar trabajo sin guardar](recover).

Para conservar la última copia guardada de un fichero, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffPub copia primero el fichero anterior en otro llamado «Copia de» seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación le pregunta si desea guardar sin ella. La respuesta es No, salvo que indique Sí, y el fichero permanece como estaba. Encontrará esta opción en **Archivo > Opciones**, en la ficha **General**. Todas las aplicaciones de JeffOffice la tienen.

## Guardar con un nuevo nombre

Haga clic en **Archivo > Guardar como** o pulse F12. Escriba un nombre, elija **Guardar como tipo** y haga clic en **Guardar**. Las opciones son:

- **Publicación de JeffPub (*.jpub)** es la opción habitual.
- **Ficheros de publicación .pub (*.pub)** crea un fichero .pub.
- **PDF (*.pdf)** crea un PDF y deja su publicación como estaba.
- **Plantilla de JeffPub (*.jpub)** agrega la publicación a **Mis plantillas**.

Después de guardar como .jpub o .pub, la barra de título muestra el nuevo nombre y usted sigue trabajando en el nuevo fichero.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffPub escribe la publicación en ese fichero, y usted sigue trabajando en el que tenía abierto. Su nombre y sus cambios sin guardar se mantienen como estaban.

## Abrir

1. Pulse Ctrl+A, o haga clic en **Archivo > Abrir**.
2. Haga clic en **Examinar** para elegir un fichero, o haga clic en un nombre en **Anclados** o en **Publicaciones recientes**.

También puede arrastrar un fichero .jpub o .pub a la página. Si su publicación actual tiene cambios sin guardar, JeffPub le pregunta por ellos antes de continuar.

Haga clic con el botón derecho en un nombre de la lista para **Anclar a la lista** (se mantiene en la parte superior) o **Quitar de la lista**. Un nombre en gris indica que JeffPub no encuentra ese fichero en este momento. Para cambiar cuántos nombres recientes se muestran, consulte [Cambiar la configuración de JeffPub](options).

## Otras formas de abrir un fichero

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista Recientes.

- **Abrir como solo lectura** abre la publicación solo para leerla. Puede mirarla y desplazarse, pero no cambiarla. La barra de título dice «Solo lectura». Si pulsa **Guardar**, JeffPub le pide un nombre nuevo, de modo que el fichero que abrió no se modifica.
- **Abrir como copia** abre la publicación sin fichero propio. **Guardar** pregunta dónde colocarla.
- **Abrir en una nueva ventana** abre el fichero en una ventana propia.
- **Abrir y reparar** sirve para un fichero .jpub dañado, por ejemplo uno que quedó incompleto al copiarlo. JeffPub lee todas las partes del fichero que puede y omite las que no puede leer. La publicación se abre como una copia llamada «Reparada» seguido del nombre, y un mensaje le indica qué se omitió. El fichero dañado no se modifica.

## Cerrar

Haga clic en **Archivo > Cerrar** o pulse Ctrl+F4. JeffPub le pregunta si desea guardar los cambios y luego le muestra una página en blanco.

## Información: propiedades, Marcar como final, Inspeccionar y versiones

**Archivo > Información** muestra dónde está el fichero, con **Mostrar en la carpeta** y **Copiar ruta**. Muestra datos de su publicación: páginas, tamaño de página, imágenes, palabras y fechas. Escriba en los cuadros para definir **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Organización** y **Administrador**, y haga clic en **Propiedades personalizadas** para crear propiedades propias. El cambio se guarda con la publicación, y **Ctrl+Z** lo deshace. La página también abre la información empresarial, la **Configuración de impresión comercial** (consulte [Ficheros PDF y imprentas](pdf)) y el [Comprobador de diseño](design-checker).

- **Comprobar compatibilidad** indica lo que un fichero .pub no puede conservar de su publicación, sin guardar nada: objetos ocultos, algunos estilos de lista y cualquier otra cosa que el escritor de .pub omite. Cuando guarda como fichero .pub, JeffPub muestra la misma lista **antes** de escribir nada. Elija **Guardar de todos modos**, **Guardar como .jpub** (el fichero .jpub se crea junto a él, y no se pierde nada) o **Cancelar**. Marque **No volver a preguntar para este fichero** y JeffPub deja de preguntar por ese fichero hasta que cierre el programa.
- **Marcar como final** guarda la publicación y luego la deja solo para lectura. Cuando alguien la abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. No es un bloqueo: para editarla, elija **Marcada como final** de nuevo. Un fichero .pub no puede conservar esa marca, así que guarde primero como fichero .jpub.
- **Inspeccionar publicación** muestra lo que el fichero contiene y que quizá no desee compartir, y lo elimina: los comentarios y los nombres de quienes los escribieron, el autor y otros nombres de las propiedades, la información empresarial, la lista de combinación de correspondencia y los objetos guardados en el área de trabajo junto a las páginas.
- **Quitar información personal de este fichero al guardarlo**, debajo de las tarjetas, elimina en cada guardado el autor, la persona que guardó el fichero por última vez, la organización y el administrador, tanto en un fichero .jpub como en uno .pub. Es la misma casilla que en **Opciones**, y aparece igual en todas las aplicaciones de JeffOffice.
- **Historial de versiones** guarda copias con nombre de la publicación en su equipo. Guarde primero la publicación. Haga clic en **Guardar una versión…** y, más adelante, elija una para **Abrir una copia**, **Volver a esta versión** (la publicación tal como está ahora se guarda antes como versión) o **Eliminar**.

Un fichero .jpub no tiene forma de contraseña, así que JeffPub no tiene una tarjeta de contraseña. JeffPub todavía no tiene un comprobador de accesibilidad, por lo que la página no tiene tarjeta para ello.

## Si Windows no permite que JeffPub guarde

La Seguridad de Windows puede impedir que los programas guarden en carpetas como Documentos. En ese caso, JeffPub muestra un mensaje con los pasos para permitirlo. También puede guardar en otra carpeta.

## Temas relacionados

- [Recuperar trabajo sin guardar](recover)
- [Trabajar con ficheros .pub](pub-files)
- [Crear una publicación nueva](create)
- [Cambiar la configuración de JeffPub](options)
