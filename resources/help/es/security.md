# Seguridad: a qué puede acceder un archivo

<!-- keywords: centro de confianza, seguridad, macros, archivos vinculados, vínculos, imágenes vinculadas, multimedia vinculada, internet, descargado, solo lectura, protegido, vínculos externos, programas, seguro, confianza, opciones de seguridad -->

Un archivo puede apuntar a otras cosas: archivos, páginas web, vídeos e incluso programas. La página **Seguridad** es donde decide qué hace JeffPub al respecto. Haga clic en **Archivo > Opciones** y, después, en la pestaña **Seguridad**. Haga clic en **Aceptar** para conservar los cambios o en **Cancelar** para descartarlos.

Estas opciones son comunes a todas las aplicaciones de JeffOffice: si cambia una en una aplicación, cambia en todas. Una fila correspondiente a algo que una aplicación no tiene aparece como **Esto no se aplica** y en gris.

## Macros

Las macros son pequeños programas guardados dentro de un archivo. JeffPub **nunca las ejecuta** y no hay ninguna opción que las ejecute. Tampoco conserva las macros de una publicación: al guardar un archivo `.pub` que abrió, la copia no tendrá ninguna, así que conserve el original si todavía las necesita.

## Archivos vinculados y vínculos de libros

Esto se refiere a las imágenes que están **vinculadas** a un archivo en lugar de guardarse dentro de la publicación. La opción determina qué hace JeffPub al abrir una publicación que tenga algunas.

- **Actualizarlas** es la opción habitual. Al abrir una publicación, JeffPub vuelve a leer el archivo de cada imagen vinculada si el archivo está en la carpeta propia de la publicación, así la imagen está al día. Nunca pregunta.
- **No actualizarlas nunca** deja cada imagen tal como se guardó. Cada una muestra **No actualizada** hasta que use **Actualizar vínculo** en ella. Entonces, **Exportar a PDF** e **Imprimir** le indican en la barra de estado que se usaron vistas previas para las imágenes que no tienen una copia completa en la publicación.

JeffPub solo lee un archivo que esté en la carpeta de la publicación (o en una carpeta que usted haya marcado como segura). Cualquier otro archivo vinculado espera hasta que lo pida.

## Vínculos a películas y sonidos

Esto no se aplica en JeffPub, que no tiene películas ni sonidos vinculados que reproducir. La fila aparece en gris.

## Archivos de internet u otros equipos

Un archivo que descargó, o que está en otro equipo de su red, puede no ser seguro. Esta opción indica cómo lo abre JeffPub.

- **Abrir primero como solo lectura** es la opción habitual. El archivo se abre para que pueda leerlo, pero no puede modificarlo. Una barra debajo del archivo lo indica. Haga clic en **Habilitar edición** en la barra para modificar ese archivo (hasta que cierre el programa). Cuando lo guarde en el mismo sitio, JeffPub quita al archivo la marca de descarga. También puede usar **Guardar como** para crear una copia, o cambiar esta opción a la siguiente.
- **Abrir normalmente** abre todos los archivos de la misma forma.

Un archivo que abra con **Abrir como copia**, **Abrir y reparar** o **Abrir como solo lectura** desde la pantalla **Archivo** se abre de esa forma, sea cual sea la opción anterior.

## Vínculos que inician un programa

Un vínculo que iniciaría un programa o un script (un archivo que termina en .exe, .bat, .sh, etc.) se **rechaza siempre**. JeffPub se lo indica y no se ejecuta nada. Un vínculo a un archivo de otro equipo nunca se sigue sin preguntarle antes. Ninguna opción cambia esto.

## Temas relacionados

- [Cambiar la configuración de JeffPub](options)
- [Imágenes](pictures)
