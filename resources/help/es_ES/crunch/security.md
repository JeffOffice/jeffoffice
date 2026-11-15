# Seguridad: a qué puede acceder un fichero
<!-- keywords: centro de confianza, seguridad, macros, archivos vinculados, vínculos, imágenes vinculadas, multimedia vinculada, internet, descargado, solo lectura, protegido, vínculos externos, programas, seguro, ficheros vinculados -->

Un fichero puede apuntar a otras cosas: ficheros, páginas web, vídeos e incluso programas. La página **Seguridad** es donde decide qué hace JeffCrunch con ellos. Haga clic en **Archivo > Opciones** y, después, en la pestaña **Seguridad**. Haga clic en **Aceptar** para conservar los cambios, o en **Cancelar** para descartarlos.

Estas opciones son comunes a todas las aplicaciones de JeffOffice: si cambia una en una aplicación, cambia en todas. Una fila de algo que una aplicación no tiene dice **No se aplica** y aparece atenuada.

## Macros

Las macros son pequeños programas guardados dentro de un fichero. JeffCrunch las conserva en el fichero, de modo que siguen ahí al guardar, y **nunca las ejecuta**. No hay ninguna opción que las ejecute.

## Ficheros vinculados y vínculos de libros

Esto se refiere a otros libros de los que un libro toma números, llamados **vínculos**. La elección determina qué hace JeffCrunch al abrir un libro que los tenga.

- **Preguntar primero** es la opción habitual. JeffCrunch pregunta si debe actualizar los números de los otros libros.
- **Actualizarlos** los actualiza sin preguntar.
- **No actualizarlos nunca** conserva los números que guardó el fichero.

Una elección que haga para un libro en **Datos > Editar vínculos** (**Preguntar al inicio**) prevalece para ese libro. Un libro que indique «nunca» se deja siempre tal cual. Un libro que esté en otro equipo nunca se lee al abrir un fichero, sea cual sea la opción elegida. Aun así, puede usar **Actualizar valores** en **Editar vínculos** en cualquier momento.

## Vídeos y sonidos vinculados

Esto no se aplica en JeffCrunch, ya que no tiene vídeos ni sonidos vinculados que reproducir. La fila aparece atenuada.

## Ficheros de internet u otros equipos

Un fichero que descargó, o que está en otro equipo de su red, puede no ser seguro. Esta opción indica cómo lo abre JeffCrunch.

- **Abrir primero como solo lectura** es la opción habitual. El fichero se abre para que pueda leerlo, pero no cambiarlo. Una barra bajo el fichero lo indica. Haga clic en **Habilitar edición** en la barra para cambiar solo ese fichero (hasta que cierre el programa). Cuando después lo guarde en el mismo sitio, se quita la marca de descarga del fichero. También puede usar **Guardar como** para crear una copia, o cambiar esta opción a la siguiente.
- **Abrir normalmente** abre todos los ficheros de la misma manera.

Un fichero que abra con **Abrir como copia**, **Abrir y reparar** o **Abrir como solo lectura** desde la pantalla **Archivo** se abre de ese modo, sea cual sea lo indicado aquí.

## Vínculos que inician un programa

Un vínculo que iniciaría un programa o un script (un fichero con extensión .exe, .bat, .sh, etc.) se **rechaza siempre**. JeffCrunch se lo indica y no se ejecuta nada. Un vínculo a un fichero de otro equipo nunca se sigue sin preguntarle antes. Ninguna opción cambia esto.

## Véase también

- [Cambiar la configuración de JeffCrunch](options)
- [Notas y vínculos](notes-links)
