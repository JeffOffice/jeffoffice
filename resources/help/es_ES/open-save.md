# Guardar y abrir publicaciones
<!-- keywords: abrir, guardar, guardar como, guardar una copia, cerrar, archivos recientes, anclado, tipos de archivo, jpub, cambiar nombre, copia de seguridad, propiedades, información, marcar como final, inspeccionar, historial de versiones, nuevo nombre, copiar, Ctrl+S, solo lectura, abrir como copia, nueva ventana, mis plantillas, guardar como plantilla, reparar, abrir y reparar, dañado, ficheros recientes, tipos de fichero -->

JeffPub guarda su trabajo en ficheros .jpub. También abre y guarda ficheros .pub. Consulte [Trabajar con ficheros .pub](pub-files).

## Guardar

Pulse Ctrl+S, haga clic en el botón **Guardar** de la barra de herramientas de acceso rápido o haga clic en **Archivo > Guardar**. La primera vez, JeffPub le pide un nombre y una ubicación. Después, Guardar reemplaza el fichero con su trabajo más reciente.

Un asterisco (*) después del nombre en la barra de título indica que tiene cambios sin guardar. Guarde con frecuencia. Si JeffPub se detiene inesperadamente, consulte [Recuperar trabajo sin guardar](recover).

Para conservar la última copia guardada de un fichero, active **Crear siempre una copia de seguridad**. Cada vez que guarde, JeffPub copia primero el fichero anterior a otro llamado «Copia de» seguido de su nombre, en la misma carpeta. Si no se puede crear la copia, la aplicación pregunta si desea guardar sin ella. La respuesta predeterminada es No, a menos que diga Sí, y el fichero permanece como estaba. Encontrará esta opción en **Archivo > Opciones**, en la pestaña **General**. Todas las aplicaciones de JeffOffice la tienen.

## Guardar con un nuevo nombre

Haga clic en **Archivo > Guardar como** o pulse F12. Escriba un nombre, elija **Guardar como tipo** y haga clic en **Guardar**. Las opciones son:

- **Publicación de JeffPub (*.jpub)** es la opción normal.
- **Ficheros de publicación .pub (*.pub)** crea un fichero .pub.
- **PDF (*.pdf)** crea un PDF y deja su publicación como estaba.
- **Plantilla de JeffPub (*.jpub)** añade la publicación a **Mis plantillas**.

Después de guardar como .jpub o .pub, la barra de título muestra el nuevo nombre y usted sigue trabajando en el fichero nuevo.

## Guardar una copia

Haga clic en **Archivo > Guardar una copia** y elija un nombre. JeffPub escribe la publicación en ese fichero, y usted sigue trabajando en la que tenía abierta. Su nombre y sus cambios sin guardar permanecen como estaban.

## Abrir

1. Pulse Ctrl+O o haga clic en **Archivo > Abrir**.
2. Haga clic en **Examinar** para elegir un fichero, o haga clic en un nombre en **Anclados** o en **Publicaciones recientes**.

También puede arrastrar un fichero .jpub o .pub a la página. Si su publicación actual tiene cambios sin guardar, JeffPub le pregunta primero por ellos.

Haga clic con el botón derecho en un nombre de la lista para **Anclar a la lista** (se mantiene arriba) o **Quitar de la lista**. Un nombre en gris indica que JeffPub no puede encontrar ese fichero en este momento. Para cambiar cuántos nombres recientes se muestran, consulte [Cambiar la configuración de JeffPub](options).

## Otras formas de abrir un fichero

En la página **Abrir**, haga clic en **Más formas de abrir**. También puede hacer clic con el botón derecho en un nombre de la lista Recientes.

- **Abrir como solo lectura** abre la publicación solo para leerla. Puede mirarla y desplazarse, pero no cambiar nada. La barra de título dice «Solo lectura». Si pulsa **Guardar**, JeffPub le pide un nuevo nombre, así que el fichero que abrió no se modifica.
- **Abrir como copia** abre la publicación sin fichero propio. **Guardar** pregunta dónde colocarla.
- **Abrir en nueva ventana** abre el fichero en una ventana propia.
- **Abrir y reparar** sirve para un fichero .jpub dañado, por ejemplo uno que quedó incompleto al copiarlo. JeffPub lee todas las partes del fichero que puede y omite las que no puede leer. La publicación se abre como una copia llamada «Reparada» seguido del nombre, y un mensaje le indica qué se omitió. El fichero dañado no se modifica.

## Cerrar

Haga clic en **Archivo > Cerrar** o pulse Ctrl+F4. JeffPub pregunta si desea guardar los cambios y después muestra una página en blanco.

## Información: propiedades, Marcar como final, Inspeccionar y versiones

**Archivo > Información** muestra dónde está el fichero, con **Mostrar en la carpeta** y **Copiar ruta**. Muestra datos de su publicación: páginas, tamaño de página, imágenes, palabras y fechas. Escriba en los cuadros para definir **Título**, **Autor**, **Asunto**, **Palabras clave**, **Categorías**, **Comentarios**, **Empresa** y **Administrador**, y haga clic en **Propiedades personalizadas** para crear propiedades propias. El cambio se guarda con la publicación, y **Ctrl+Z** lo deshace. La página también abre su información empresarial, la **Configuración de impresión comercial** (consulte [Ficheros PDF e imprentas](pdf)) y el [Comprobador de diseño](design-checker).

- **Marcar como final** guarda la publicación y después la deja solo para lectura. Cuando alguien la abre, la barra de título indica que es de solo lectura, y una nota le informa de que se marcó como final. No es un bloqueo: elija **Marcado como final** de nuevo para editarla. Un fichero .pub no puede guardar esta marca, así que guárdelo primero como fichero .jpub.
- **Inspeccionar publicación** muestra lo que contiene el fichero y que quizá no desee compartir, y lo elimina: el autor y otros nombres de las propiedades, la información empresarial, la lista de combinación de correspondencia y los objetos guardados en el área de trabajo junto a las páginas.
- **Historial de versiones** conserva copias con nombre de la publicación en su equipo. Guarde primero la publicación. Haga clic en **Guardar una versión…** y, más tarde, elija una versión para **Abrir una copia** de ella, **Volver a esta versión** (la publicación tal como está ahora se guarda antes como versión) o **Eliminar**.

Un fichero .jpub no tiene un formato para contraseña, así que JeffPub no tiene una ficha de contraseña. JeffPub todavía no incluye una comprobación de accesibilidad, por lo que la página no tiene una ficha para ello.

## Si Windows no permite que JeffPub guarde

El Seguridad de Windows puede impedir que los programas guarden en carpetas como Documentos. En ese caso, JeffPub muestra un mensaje con los pasos para permitirlo. También puede guardar en otra carpeta.

## Relacionado

- [Recuperar trabajo sin guardar](recover)
- [Trabajar con ficheros .pub](pub-files)
- [Crear una publicación nueva](create)
- [Cambiar la configuración de JeffPub](options)
