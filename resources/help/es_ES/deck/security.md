# Seguridad: a qué puede acceder un fichero

<!-- keywords: centro de confianza, seguridad, macros, archivos vinculados, vínculos, imágenes vinculadas, multimedia vinculada, internet, descargado, solo lectura, protegido, vínculos externos, programas, seguro, ficheros vinculados -->

Un fichero puede apuntar a otras cosas: ficheros, páginas web, vídeos e incluso programas. La página **Seguridad** es donde decide qué hace JeffDeck al respecto. Haga clic en **Archivo > Opciones** y, luego, en la pestaña **Seguridad**. Haga clic en **Aceptar** para conservar los cambios o en **Cancelar** para descartarlos.

Estas opciones son comunes a todas las aplicaciones de JeffOffice: si cambia una en una aplicación, cambia en todas. Una fila de algo que una aplicación no tiene dice **No aplica** y aparece atenuada.

## Macros

Las macros son pequeños programas guardados dentro de un fichero. JeffDeck las conserva en el fichero, de modo que siguen ahí al guardar, y **nunca las ejecuta**. No hay ninguna opción que las ejecute.

## Ficheros vinculados y vínculos de libros

Esto no se aplica en JeffDeck: no tiene ficheros vinculados que actualizar. La fila aparece atenuada.

## Vídeos y sonidos vinculados

Se trata de un vídeo o un sonido al que una diapositiva **vincula** en lugar de incluirlo dentro de la presentación.

- **Nunca reproducirlo** es la opción habitual. Un vídeo o sonido vinculado indica que está vinculado y no se reproduce.
- **Preguntar primero** pregunta una vez por cada fichero vinculado la primera vez que lo reproduce.
- **Reproducirlo** lo reproduce sin preguntar.

Incluso con **Reproducirlo**, solo se reproduce un fichero de vídeo o sonido cuyo nombre esté en este equipo. Una dirección web, un fichero de otro equipo, un programa, un vínculo o fichero especial, y un fichero que en realidad no es un vídeo o sonido, nunca se reproducen. Un fichero vinculado puede tener hasta 256 MB. El fichero se lee en segundo plano durante un máximo de 20 segundos, de modo que una unidad lenta no puede bloquear JeffDeck. Una unidad de red que parece local (una unidad asignada, o una carpeta en /Volumes o /mnt) se lee como un fichero local, por lo que puede ser lenta. Con **Preguntar primero**, una presentación pregunta una sola vez, al iniciarse, por todos los ficheros vinculados que contenga.

## Ficheros de internet u otros equipos

Un fichero que ha descargado, o que está en otro equipo de su red, puede no ser seguro. Esta opción indica cómo lo abre JeffDeck.

- **Abrir primero como solo lectura** es la opción habitual. El fichero se abre para que pueda leerlo, pero no modificarlo. Una barra debajo del fichero lo indica. Haga clic en **Habilitar edición** en la barra para modificar ese fichero (hasta que cierre el programa). Cuando después lo guarde en su ubicación, se quita la marca de descarga del fichero. También puede usar **Guardar como** para crear una copia, o cambiar esta opción a la siguiente.
- **Abrir normalmente** abre todos los ficheros de la misma manera.

Un fichero que abra con **Abrir como copia**, **Abrir y reparar** o **Abrir como solo lectura** desde la pantalla **Archivo** se abre de ese modo, sea cual sea esta configuración.

## Vínculos que inician un programa

Un vínculo que iniciaría un programa o un script (un fichero con extensión .exe, .bat, .sh, etc.) se **rechaza siempre**. JeffDeck se lo indica y no se ejecuta nada. Un vínculo a un fichero de otro equipo nunca se sigue sin preguntarle antes. Ninguna opción cambia esto.

## Temas relacionados

- [Cambiar la configuración de JeffDeck](options)
- [Ejecutar la presentación con diapositivas](show)
