# Abrir libros protegidos con contraseña
<!-- keywords: contraseña, protegido, cifrado, cifrar, abrir, bloqueado, contraseña incorrecta, cancelar, xlsx, xls, ods, protección de archivos, contraseña predeterminada, privado -->

Algunos libros se protegen con una contraseña al guardarlos: sin ella, ningún programa puede leer lo que contiene el archivo. Esto es distinto de **Proteger hoja** y **Proteger libro** (véase [Proteger una hoja o el libro](protect)), que solo impiden hacer cambios. Un libro protegido con contraseña no se puede abrir en absoluto hasta que se escribe la contraseña. Esta página explica cómo abrir uno y cómo crear uno.

## Abrir un libro

Abra el archivo como abre cualquier otro: pulse **{key:Open}**, elíjalo o arrástrelo a la ventana. JeffCrunch detecta que el archivo está protegido y pide la contraseña antes de leer nada:

1. Escriba la contraseña en el cuadro que dice **Este archivo está protegido con contraseña.** Las letras aparecen como puntos.
2. Haga clic en **Abrir**.

Si la contraseña no es la correcta, JeffCrunch lo indica y vuelve a preguntar, tantas veces como quiera. Haga clic en **Cancelar** para dejar de intentarlo: no se abre nada y los libros que ya tiene abiertos siguen como estaban. Las contraseñas son exactas: cuentan las mayúsculas, los espacios y los acentos.

JeffCrunch no guarda la contraseña en ningún sitio. La mantiene en memoria mientras el libro está abierto, para que **Guardar** pueda volver a protegerlo, y la borra al cerrar el libro. La próxima vez que abra el archivo, volverá a preguntarla.

## Qué se puede abrir

- **Libros .xlsx y .xlsm** protegidos de cualquiera de las dos formas que el programa original ha usado desde 2007: la más reciente (la clave tiene 128, 192 o 256 bits, y JeffCrunch también comprueba que nadie haya modificado el archivo después de protegerlo) y la anterior (AES con una clave de 128, 192 o 256 bits).
- **Libros .xls** protegidos de cualquiera de las tres formas de ese formato: el cifrado sencillo de los archivos más antiguos, el de 40 bits y el más robusto, que usa el servicio de claves del propio equipo.
- **Hojas de cálculo .ods** protegidas con contraseña, en las tres formas en que LibreOffice y OpenOffice las han escrito: la más reciente (LibreOffice 25 y posteriores bloquean todo el archivo con una clave de 256 bits y generan la clave a partir de la contraseña con un método deliberadamente lento y que usa mucha memoria, por lo que abrir uno de estos archivos tarda un poco más), la de 256 bits de las versiones anteriores y la más antigua.

Los archivos .xls más antiguos tienen límites en la propia contraseña. El cifrado sencillo de los archivos más antiguos usa solo los primeros 15 caracteres de la contraseña y solo admite letras, cifras y signos del juego de caracteres occidental (Latin-1): una contraseña con otras letras (como griegas o chinas) nunca se acepta como correcta en ese caso. El de 40 bits admite cualquier letra, pero solo los primeros 16 caracteres. JeffCrunch escribe las copias .xls con contraseña en la forma más robusta (la que usa el servicio de claves del equipo, con una clave de 128 bits), porque las otras dos son más débiles. Esa forma usa los primeros 255 caracteres de la contraseña, que pueden ser de cualquier letra.

Algunos archivos están bloqueados con una contraseña que todo el mundo conoce, para que cualquiera pueda abrirlos pero no se modifiquen por error. JeffCrunch los abre sin preguntar.

## Cuando no se puede abrir un archivo

- **«La contraseña no es correcta.»** La contraseña escrita no es la que se usó para proteger el archivo. Vuelva a intentarlo con cuidado con las mayúsculas y los espacios; JeffCrunch no puede recuperar una contraseña perdida.
- **«Este archivo está protegido de una forma que este programa no puede abrir.»** El archivo está bloqueado por algo distinto de una contraseña (por ejemplo, una clave guardada en un certificado) o en un formato que JeffCrunch no lee.
- **«La protección con contraseña de este archivo exige más memoria o tiempo del que este programa permite, así que no se abre.»** Un archivo de LibreOffice 25 o posterior fija cuánta memoria y tiempo se necesitan para comprobar la contraseña. JeffCrunch acepta hasta diez veces lo que fija el propio LibreOffice (640 MiB de memoria), para que un archivo hecho para saturar el equipo no pueda hacerlo. Un archivo que haya guardado LibreOffice está muy por debajo de ese límite.
- **«El archivo está dañado o se modificó después de protegerlo.»** La contraseña era correcta, pero el contenido del archivo no es el que se bloqueó: está incompleto, dañado o lo cambió otro programa. Cópielo de nuevo desde el original.

## Guardar con contraseña

Haga clic en **Archivo** y, después, en **Proteger con contraseña** (o **Revisar > Proteger con contraseña**), escriba la contraseña dos veces y haga clic en **Proteger**. El libro se guardará protegido la próxima vez que lo guarde: quien abra el archivo deberá escribir la contraseña. Para cambiarla, elija el mismo comando y escriba una nueva; para quitarla, haga clic en **Quitar contraseña** y guarde.

Un libro que abrió con contraseña la conserva mientras está abierto, así que **Guardar** vuelve a escribir el archivo protegido con la misma contraseña. JeffCrunch mantiene la contraseña en memoria solo hasta que cierra el libro, y entonces la borra. El archivo que abrió con contraseña no queda sin protección por descuido: una copia guardada como archivo .xls se protege con la misma contraseña en la forma más robusta de .xls, y una copia guardada como .ods o .ots se protege con ella según describe el estándar OpenDocument, de modo que otros programas que sigan esos estándares pueden abrir la copia con la contraseña. Una copia en texto no se puede proteger, así que JeffCrunch le avisa antes de que la copia no quedará protegida, y puede cancelar.

Use una contraseña larga y guárdela en un lugar seguro: nadie, tampoco nosotros, puede abrir el archivo sin ella.
