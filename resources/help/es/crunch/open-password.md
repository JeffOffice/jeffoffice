# Abrir libros protegidos con contraseña
<!-- keywords: contraseña, clave, protegido, cifrado, encriptado, cifrar, abrir, bloqueado, contraseña incorrecta, cancelar, xlsx, xls, ods, protección de archivo, contraseña predeterminada, privado -->

Algunos libros se protegen con una contraseña al guardarlos: sin ella, ningún programa puede leer lo que contiene el archivo. Esto es distinto de **Proteger hoja** y **Proteger libro** (vea [Proteger una hoja o el libro](protect)), que solo impiden los cambios. Un libro protegido con contraseña no se puede abrir de ninguna forma hasta que se escribe la contraseña. En esta página se explica cómo abrir uno y cómo crearlo.

## Abrir un libro protegido

Abra el archivo como cualquier otro: presione **{key:Open}**, elíjalo o arrástrelo a la ventana. JeffCrunch detecta que el archivo está protegido y pide la contraseña antes de leer nada:

1. Escriba la contraseña en el cuadro que dice **Este archivo está protegido con contraseña.** Los caracteres se muestran como puntos.
2. Haga clic en **Abrir**.

Si la contraseña no es la correcta, JeffCrunch lo indica y vuelve a pedirla, tantas veces como quiera. Haga clic en **Cancelar** para detenerse: no se abre nada y los libros que ya tenga abiertos se quedan como están. Las contraseñas deben ser exactas: cuentan las mayúsculas, los espacios y los acentos.

JeffCrunch no escribe la contraseña en ningún sitio. La conserva en memoria mientras el libro está abierto, para que **Guardar** pueda volver a proteger el archivo, y la borra al cerrar el libro. La próxima vez que abra el archivo, se la volverá a pedir.

## Qué se puede abrir

- **Libros .xlsx y .xlsm** protegidos de cualquiera de las dos formas que usa el programa original desde 2007: la más reciente (con una clave de 128, 192 o 256 bits; JeffCrunch comprueba además que nadie haya modificado el archivo después de protegerlo) y la anterior (AES con una clave de 128, 192 o 256 bits).
- **Libros .xls** protegidos de cualquiera de las tres formas de ese formato: la codificación sencilla de los archivos más antiguos, la de 40 bits y la más robusta, que usa el servicio de claves del propio equipo.
- **Hojas de cálculo .ods** protegidas con contraseña, de las tres formas en que las han guardado LibreOffice y OpenOffice: la más reciente (LibreOffice 25 y posteriores cifran todo el archivo con una clave de 256 bits y obtienen la clave a partir de la contraseña con un método que es lento a propósito y usa mucha memoria, por lo que abrir uno de estos archivos tarda un poco más), la de 256 bits de las versiones anteriores y la más antigua.

Los archivos .xls antiguos tienen limitaciones en la propia contraseña. La codificación sencilla de los archivos más antiguos solo usa los primeros 15 caracteres de la contraseña y solo admite letras, dígitos y signos del juego de caracteres occidental (Latin-1): una contraseña con otras letras (como griegas o chinas) nunca se acepta como correcta en ellos. La de 40 bits admite cualquier letra, pero solo los primeros 16 caracteres. JeffCrunch no guarda archivos .xls con contraseña.

Algunos archivos están bloqueados con una contraseña que todo el mundo conoce, para que cualquiera pueda abrirlos pero nadie los modifique por accidente. JeffCrunch los abre sin preguntar.

## Cuando no se puede abrir un archivo

- **"La contraseña no es correcta."** La contraseña que escribió no es con la que se protegió el archivo. Vuelva a intentarlo prestando atención a las mayúsculas y los espacios; JeffCrunch no puede recuperar una contraseña perdida.
- **"Este archivo está protegido de una forma que este programa no puede abrir."** El archivo está bloqueado con algo que no es una contraseña (por ejemplo, una clave contenida en un certificado) o en un formato que JeffCrunch no lee.
- **"La protección con contraseña de este archivo requiere más memoria o tiempo de lo que permite este programa, por lo que no se abre."** Un archivo de LibreOffice 25 o posterior establece cuánta memoria y tiempo se necesitan para comprobar la contraseña. JeffCrunch admite hasta diez veces lo que establece el propio LibreOffice (640 MiB de memoria), para que un archivo creado con el fin de bloquear su equipo no pueda hacerlo. Un archivo guardado por el propio LibreOffice queda muy por debajo de ese límite.
- **"El archivo está dañado o se modificó después de protegerlo."** La contraseña era correcta, pero el contenido del archivo no es el que se bloqueó: está truncado, dañado o lo modificó otro programa. Vuelva a copiarlo desde el original.

## Guardar con contraseña

Haga clic en **Archivo** y luego en **Proteger con contraseña** (o en **Revisar > Proteger con contraseña**), escriba la contraseña dos veces y haga clic en **Proteger**. El libro se guardará protegido la próxima vez que lo guarde: quien abra el archivo tendrá que escribir la contraseña. Para cambiar la contraseña, elija el mismo comando y escriba una nueva; para quitarla, haga clic en **Quitar contraseña** y guarde.

Un libro que abrió con contraseña la conserva mientras está abierto, de modo que **Guardar** vuelve a guardar el archivo protegido con la misma contraseña. JeffCrunch conserva la contraseña en memoria solo hasta que cierra el libro y luego la borra. El archivo que abrió con contraseña no queda desprotegido por accidente: si guarda una copia como archivo .xls u .ods, que JeffCrunch no puede proteger, le avisa antes de que la copia no estará protegida, y puede cancelar.

Use una contraseña larga y guárdela en un lugar seguro: nadie, ni siquiera nosotros, puede abrir el archivo sin ella.
