# Trabajar con archivos .pub

<!-- keywords: pub, .pub, archivo pub, abrir pub, guardar pub, convertir, compatibilidad, importar, publicaciones antiguas, otros programas, jpub, formato, fuentes que faltan, copia de seguridad del original, formato de archivo -->

Un archivo .pub es una publicación creada con otros programas de autoedición. JeffPub abre archivos .pub creados desde 1998. También puede guardar su trabajo como archivo .pub. El formato propio de JeffPub es .jpub.

## Abrir un archivo .pub

1. Presione Ctrl+O o haga clic en **Archivo > Abrir**.
2. Haga clic en **Examinar**, elija el archivo .pub y haga clic en **Abrir**.

También puede arrastrar el archivo a la página. El texto, las imágenes, las formas y las tablas se incorporan, junto con los cuadros de texto vinculados. La mayoría de las páginas se parecen mucho al original, pero no todos los detalles coinciden todavía. Es posible que algunos cuadros de texto terminen una o dos palabras antes o después de como estaban antes.

Si una fuente del archivo no está en su equipo, JeffPub usa en su lugar una fuente abierta parecida. Las fuentes guardadas dentro del archivo .pub no se incorporan. Si JeffPub tiene algo que comunicarle después de abrir el archivo, aparecerá una nota en la barra de estado, en la parte inferior de la ventana.

Si el archivo no se abre, JeffPub indica que puede estar dañado o que usa funciones que todavía no puede leer. Lo mismo ocurre con un archivo que contiene texto o formas, pero ninguna que JeffPub pueda colocar en una página: no se abre como páginas en blanco, para que no sobrescriba el archivo por error, y el archivo se deja como estaba.

## Revisar el resultado

Como abrir el archivo implica una conversión, revise todas las páginas. Compruebe los saltos de línea, las fuentes y las imágenes. Use el [Comprobador de diseño](design-checker) para encontrar texto que no quepa.

## Guardar como archivo .pub

- Haga clic en **Archivo > Exportar** y, después, en **Guardar como archivo .pub**.
- O haga clic en **Archivo > Guardar como** y elija **Archivos de publicación .pub (*.pub)** en **Guardar como tipo**.

Al guardar un archivo .pub, JeffPub puede escribir páginas, cuadros de texto y cuadros de texto vinculados, formato de texto, formas, líneas, imágenes, tablas, rellenos, sombras, texto artístico, estilos, folletos, tarjetas plegadas, sobres, números de página y vínculos. Si algo no tiene forma en .pub, JeffPub le indica cuántos objetos ha omitido. Esos objetos permanecen en su publicación y se conservan al guardar como archivo .jpub. Los comentarios tampoco tienen forma en .pub: JeffPub indica que los ha omitido y solo se conservan en archivos .jpub. El tema Comentarios de esta Ayuda da más información.

## Proteger el original

Cuando abre un archivo .pub y presiona Ctrl+S, JeffPub escribe los cambios en ese mismo archivo .pub. Para conservar el original a salvo:

- Active **Crear siempre una copia de seguridad** en **Archivo > Opciones**, en la pestaña **Guardar**. JeffPub conservará entonces el archivo anterior con el nombre «Backup of» seguido de su nombre.
- O comience con **Archivo > Guardar como** y guarde un archivo .jpub. Un archivo .jpub conserva todo.

Conserve los archivos originales hasta estar seguro de que los nuevos son correctos.

## Temas relacionados

- [Abrir y guardar publicaciones](open-save)
- [Guardar como imagen, página web o libro electrónico](export)
- [Cambiar la configuración de JeffPub](options)
