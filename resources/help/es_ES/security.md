# Seguridad: a qué puede acceder un fichero

<!-- keywords: centro de confianza, seguridad, macros, archivos vinculados, vínculos, imágenes vinculadas, multimedia vinculada, internet, descargado, solo lectura, protegido, vínculos externos, programas, seguro, ficheros vinculados -->

Un fichero puede apuntar a otras cosas: ficheros, páginas web, vídeos e incluso programas. La página **Seguridad** es donde decide qué hace JeffPub al respecto. Haga clic en **Archivo > Opciones** y, después, en la pestaña **Seguridad**. Haga clic en **Aceptar** para conservar los cambios o en **Cancelar** para descartarlos.

Estas opciones son comunes a todas las aplicaciones de JeffOffice: si cambia una en una aplicación, cambia en todas. Una fila de algo que una aplicación no tiene dice **Esto no se aplica** y aparece atenuada.

## Macros

Las macros son pequeños programas guardados dentro de un fichero. JeffPub las conserva en el fichero, de modo que siguen ahí al guardar, y **nunca las ejecuta**. No hay ninguna opción que las ejecute.

## Ficheros vinculados y vínculos de libros

Esto se refiere a imágenes que están **vinculadas** a un fichero en lugar de guardadas dentro de la publicación. La opción decide qué hace JeffPub al abrir una publicación que las tenga.

- **Actualizarlas** es la opción habitual. Al abrir una publicación, JeffPub vuelve a leer el fichero de cada imagen vinculada si está en la carpeta de la propia publicación, así que la imagen queda actualizada. Nunca pregunta.
- **No actualizarlas nunca** deja cada imagen tal como se guardó. Cada una muestra **No actualizada** hasta que use **Actualizar vínculo** en ella. **Exportar a PDF** e **Imprimir** le indican entonces en la barra de estado que se usaron vistas previas para las imágenes que no tienen una copia completa en la publicación.

JeffPub solo lee un fichero que esté en la carpeta de la publicación (o en una carpeta que haya indicado como segura). Cualquier otro fichero vinculado espera hasta que lo pida.

## Multimedia vinculada

Esto no se aplica en JeffPub, que no tiene vídeos ni sonidos vinculados que reproducir. La fila aparece atenuada.

## Ficheros de internet u otros equipos

Un fichero que descargó, o que está en otro equipo de su red, puede no ser seguro. Esta opción indica cómo lo abre JeffPub.

- **Abrir primero como solo lectura** es la opción habitual. El fichero se abre para que pueda leerlo, pero no cambiarlo. Una barra bajo el fichero lo indica. Haga clic en **Habilitar edición** en la barra para cambiar ese fichero (hasta que cierre el programa). Cuando lo guarde en su sitio después, JeffPub quita la marca de descarga del fichero. También puede usar **Guardar como** para hacer una copia, o cambiar esta opción a la siguiente.
- **Abrir normalmente** abre todos los ficheros de la misma forma.

Un fichero que abra con **Abrir como copia**, **Abrir y reparar** o **Abrir como solo lectura** desde la pantalla **Archivo** se abre de ese modo, sea cual sea la opción.

## Vínculos que inician un programa

Un vínculo que iniciaría un programa o un script (un fichero con extensión .exe, .bat, .sh, etc.) se **rechaza siempre**. JeffPub se lo indica y no se ejecuta nada. Un vínculo a un fichero de otro equipo nunca se sigue sin preguntarle antes. Ninguna opción cambia esto.

## Temas relacionados

- [Cambiar la configuración de JeffPub](options)
- [Imágenes](pictures)
