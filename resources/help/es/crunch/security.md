# Seguridad: a qué puede acceder un archivo
<!-- keywords: centro de confianza, seguridad, macros, archivos vinculados, vínculos, imágenes vinculadas, multimedia vinculada, internet, descargado, solo lectura, protegido, vínculos externos, programas, seguro, confianza, autorización, permisos -->

Un archivo puede apuntar a otras cosas: archivos, páginas web, vídeos e incluso programas. La página **Seguridad** es donde decide qué hace JeffCrunch al respecto. Haga clic en **Archivo > Opciones** y, después, en la pestaña **Seguridad**. Haga clic en **Aceptar** para conservar los cambios o en **Cancelar** para descartarlos.

Estas opciones son comunes a todas las aplicaciones de JeffOffice: si cambia una en una aplicación, cambia en todas. Una fila de algo que una aplicación no tiene dice **No aplicable** y aparece atenuada.

## Macros

Las macros son pequeños programas guardados dentro de un archivo. JeffCrunch las conserva en el archivo, de modo que siguen ahí al guardar, y **nunca las ejecuta**. No hay ninguna opción que las ejecute.

## Archivos vinculados y vínculos de libros

Se trata de los otros archivos que lee un libro de trabajo, como los libros de los que obtiene números, llamados **vínculos**, los archivos que leen sus consultas y los archivos en los que se basan sus tablas dinámicas. La opción indica qué hace JeffCrunch al abrir un libro que lee algunos de estos archivos y al usar **Actualizar** o **Actualizar todo** en sus consultas y tablas dinámicas.

- **Preguntar primero** es la opción habitual. Antes de que JeffCrunch lea uno de esos archivos, muestra la ruta completa del archivo y pregunta. Haga clic en **Leer el archivo** solo si confía en el origen del libro. **No leer**, el botón predeterminado, no lee nada y deja el libro como estaba. Esta opción pregunta cada vez que abre un libro y, después, una vez por cada archivo. Tras hacer clic en **Leer el archivo** para un archivo, JeffCrunch no vuelve a preguntar por ese mismo archivo en esa ventana, hasta que cierre la ventana.
- **Actualizarlos** lee un archivo sin preguntar solo si ya ha confirmado que ese mismo libro lee ese mismo archivo. Para cualquier otro archivo, incluido cualquier archivo mencionado en un libro que acaba de recibir, sigue preguntando primero, de la misma forma.
- **No actualizarlos nunca** no lee nada al abrir un libro y no muestra ninguna pregunta. Los números que guardó el archivo se mantienen tal cual. Cuando hace clic en **Actualizar** usted mismo, eso es una petición, así que JeffCrunch muestra la pregunta en ese momento. Tras hacer clic en **Leer el archivo**, no vuelve a preguntar por ese mismo archivo en esa ventana.

## Qué se recuerda

Cuando hace clic en **Leer el archivo** o **Leer los archivos**, JeffCrunch recuerda esa pareja: este libro y aquel archivo, cada uno por su ruta completa. Solo se recuerda la pareja. Si el libro se mueve o se renombra, se edita para leer otro archivo, o el archivo se sustituye por un acceso directo a otro archivo, JeffCrunch vuelve a preguntar. Un libro que nunca ha guardado no tiene ruta, así que no se recuerda nada sobre él después de cerrarlo.

Las parejas se guardan solo en la configuración de su propio equipo, nunca en un libro, de modo que un libro no puede añadir ninguna y quien reciba un libro de usted no recibe ninguna. La configuración guarda una forma codificada de cada ruta, no la ruta en sí. Se conservan hasta 2000 parejas; cuando hay más, se olvida primero la pareja que se usó hace más tiempo.

**Opciones > Seguridad** muestra cuántas parejas se recuerdan (**Aprobaciones recordadas**). Haga clic en **Olvidar todo** para olvidarlas todas a la vez. Ningún libro se modifica, y cada uno volverá a preguntar la próxima vez.

Un libro que indica «nunca» en su propio **Aviso de inicio** (**Datos > Editar vínculos**) se deja siempre en paz. Un libro que está en otro equipo nunca se lee al abrir un archivo, sea cual sea la opción. Aun así, puede usar **Actualizar valores** en **Editar vínculos** cuando quiera.

## Vídeos y sonidos vinculados

Esto no se aplica en JeffCrunch: no tiene vídeos ni sonidos vinculados que reproducir. La fila aparece atenuada.

## Archivos de internet u otros equipos

Un archivo que ha descargado, o que está en otro equipo de su red, puede no ser seguro. Esta opción indica cómo lo abre JeffCrunch.

- **Abrir primero como solo lectura** es la opción habitual. El archivo se abre para que pueda leerlo, pero no puede modificarlo. Una barra bajo el archivo lo indica. Haga clic en **Habilitar edición** en la barra para modificar ese archivo (hasta que cierre el programa). Al guardarlo después en el mismo lugar, se quita la marca de descarga del archivo. También puede usar **Guardar como** para crear una copia, o cambiar esta opción a la siguiente.
- **Abrir normalmente** abre todos los archivos de la misma forma.

Un archivo que abra con **Abrir como copia**, **Abrir y reparar** o **Abrir como solo lectura** desde la pantalla **Archivo** se abre de esa forma, sea cual sea esta opción.

## Vínculos que inician un programa

Un vínculo que iniciaría un programa o un script (un archivo con extensión .exe, .bat, .sh, etc.) se **rechaza siempre**. JeffCrunch se lo indica y no se ejecuta nada. Un vínculo a un archivo de otro equipo nunca se sigue sin preguntarle antes. Ninguna opción cambia esto.

## Véase también

- [Cambiar la configuración de JeffCrunch](options)
- [Notas y vínculos](notes-links)
