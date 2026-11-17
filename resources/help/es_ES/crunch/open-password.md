# Abrir libros protegidos con contraseña
<!-- keywords: contraseña, protegido, cifrado, cifrar, abrir, bloqueado, contraseña incorrecta, cancelar, xlsx, xls, ods, protección de archivos, contraseña predeterminada, privado, proteger con contraseña, protección de ficheros -->

Algunos libros se protegen con una contraseña al guardarlos: sin ella, ningún programa puede leer lo que contiene el fichero. Esto es distinto de **Proteger hoja** y **Proteger libro** (véase [Proteger una hoja o el libro](protect)), que solo impiden hacer cambios. Un libro protegido con contraseña no se puede abrir en absoluto hasta que se escribe la contraseña. En esta página se explica cómo abrir uno y cómo crear uno.

## Abrir uno

Abra el fichero como abre cualquier otro: pulse **{key:Open}**, elíjalo o arrástrelo a la ventana. JeffCrunch detecta que el fichero está protegido y pide la contraseña antes de leer nada:

1. Escriba la contraseña en el cuadro que dice **Este fichero está protegido con contraseña.** Las letras se muestran como puntos.
2. Haga clic en **Abrir**.

Si la contraseña no es la correcta, JeffCrunch se lo indica y vuelve a preguntar, tantas veces como quiera. Haga clic en **Cancelar** para detenerse: no se abre nada y los libros que ya tenga abiertos siguen igual. Las contraseñas son exactas: las mayúsculas, los espacios y los acentos cuentan.

JeffCrunch no guarda la contraseña en ningún sitio. La mantiene en memoria mientras el libro está abierto, para que **Guardar** pueda volver a proteger el fichero, y la borra al cerrar el libro. La próxima vez que abra el fichero, volverá a preguntarla.

## Qué se puede abrir

- **Libros .xlsx y .xlsm** protegidos de una de las dos formas que el programa original ha usado desde 2007: la más reciente (la clave tiene 128, 192 o 256 bits, y JeffCrunch también comprueba que nadie cambió el fichero después de protegerlo) y la anterior (AES con una clave de 128, 192 o 256 bits).
- **Libros .xls** protegidos de cualquiera de las tres formas de ese formato: el cifrado sencillo de los ficheros más antiguos, el de 40 bits y el más robusto, que usa el servicio de claves del propio equipo.
- **Hojas de cálculo .ods** protegidas con contraseña, de las tres formas en que LibreOffice y OpenOffice las han guardado: la más reciente (LibreOffice 25 y posteriores bloquean todo el fichero con una clave de 256 bits y generan la clave a partir de la contraseña con un método deliberadamente lento y que consume mucha memoria, así que abrir estos ficheros tarda un poco más), la de 256 bits de las versiones anteriores y la más antigua.

Los ficheros .xls más antiguos tienen límites en la propia contraseña. El cifrado sencillo de los ficheros más antiguos usa solo los primeros 15 caracteres de una contraseña y solo puede contener letras, dígitos y signos del juego de caracteres occidental (Latín-1): una contraseña con otras letras (como griegas o chinas) nunca se acepta como correcta en ese caso. El de 40 bits admite cualquier letra, pero solo los primeros 16 caracteres. JeffCrunch no guarda ficheros .xls con contraseña.

Algunos ficheros están bloqueados con una contraseña que todo el mundo conoce, para que cualquiera pueda abrirlos pero no se modifiquen por accidente. JeffCrunch los abre sin preguntar.

## Cuando un fichero no se puede abrir

- **«La contraseña no es correcta.»** La contraseña que escribió no es la que se usó para proteger el fichero. Pruebe de nuevo con cuidado con las mayúsculas y los espacios; JeffCrunch no puede recuperar una contraseña perdida.
- **«Este fichero está protegido de una forma que este programa no puede abrir.»** El fichero está bloqueado por algo distinto de una contraseña (por ejemplo, una clave guardada en un certificado) o en un formato que JeffCrunch no lee.
- **«La protección con contraseña de este fichero exige más memoria o tiempo de los que este programa permite, así que no se abre.»** Un fichero de LibreOffice 25 o posterior define cuánta memoria y tiempo cuesta comprobar la contraseña. JeffCrunch admite hasta diez veces lo que define LibreOffice (640 MiB de memoria), para que un fichero diseñado para acaparar el equipo no pueda bloquearlo. Un fichero que haya guardado LibreOffice está muy por debajo de ese límite.
- **«El fichero está dañado o se cambió después de protegerlo.»** La contraseña era correcta, pero el contenido del fichero no es el que se protegió: está truncado, dañado o lo cambió otro programa. Cópielo de nuevo desde el original.

## Guardar con contraseña

Haga clic en **Archivo** y después en **Proteger con contraseña** (o **Revisar > Proteger con contraseña**), escriba la contraseña dos veces y haga clic en **Proteger**. El libro se guarda protegido la próxima vez que lo guarde: quien abra el fichero deberá escribir la contraseña. Para cambiarla, elija el mismo comando y escriba una nueva; para quitarla, haga clic en **Quitar contraseña** y guarde.

Un libro que abrió con contraseña la conserva mientras esté abierto, así que **Guardar** vuelve a escribir el fichero protegido con la misma contraseña. JeffCrunch mantiene la contraseña en memoria solo hasta que cierra el libro, y después la borra. El fichero abierto con contraseña no queda desprotegido por accidente: una copia guardada como fichero .ods u .ots se protege con la misma contraseña, de la forma que describe el estándar OpenDocument, de modo que otros programas que sigan el estándar pueden abrirla con la contraseña. Una copia .xls o de texto no puede protegerse, así que JeffCrunch le avisa antes de que la copia no quedará protegida, y puede cancelar.

Use una contraseña larga y guárdela en un lugar seguro: nadie, tampoco nosotros, puede abrir el fichero sin ella.
