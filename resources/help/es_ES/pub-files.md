# Trabajar con ficheros .pub
<!-- keywords: pub, .pub, archivo pub, abrir pub, guardar pub, convertir, conversión, compatibilidad, importar, publicaciones antiguas, otros programas, jpub, formato, faltan fuentes, fuentes que faltan, copia de seguridad, original, formato de archivo, fichero pub, formato de fichero -->

Un fichero .pub es una publicación creada con otros programas de autoedición. JeffPub abre ficheros .pub creados desde 1998. También puede guardar su trabajo como fichero .pub. El formato propio de JeffPub es .jpub.

## Abrir un fichero .pub

1. Pulse Ctrl+O o haga clic en **Archivo > Abrir**.
2. Haga clic en **Examinar**, elija el fichero .pub y haga clic en **Abrir**.

También puede arrastrar el fichero a la página. Se importan el texto, las imágenes, las formas y las tablas, junto con los cuadros de texto vinculados. La mayoría de las páginas se parecen mucho al original, pero todavía no coinciden todos los detalles. Es posible que algunos cuadros de texto terminen una o dos palabras antes o después que en el original.

Si una fuente del fichero no está en su equipo, JeffPub usa en su lugar una fuente abierta similar. Las fuentes guardadas dentro del fichero .pub no se importan. Si JeffPub tiene algo que comunicarle después de abrir el fichero, aparece un aviso en la barra de estado, en la parte inferior de la ventana.

Si el fichero no se abre, JeffPub le indica que puede estar dañado o que usa características que todavía no puede leer.

## Comprobar el resultado

Abrir el fichero implica una conversión, así que revise todas las páginas. Compruebe los saltos de línea, las fuentes y las imágenes. Use el [Comprobador de diseño](design-checker) para encontrar texto que no cabe.

## Guardar como fichero .pub

- Haga clic en **Archivo > Exportar** y, a continuación, en **Guardar como fichero .pub**.
- O bien haga clic en **Archivo > Guardar como** y elija **Ficheros de publicación .pub (*.pub)** en **Tipo**.

Al guardar un fichero .pub, JeffPub puede escribir páginas, cuadros de texto y cuadros de texto vinculados, formato de texto, formas, líneas, imágenes, tablas, rellenos, sombras, texto artístico, estilos, folletos, tarjetas plegadas, sobres, números de página y vínculos. Si algo no tiene equivalente en .pub, JeffPub le indica cuántos objetos ha omitido. Esos objetos se conservan en su publicación y se mantienen al guardarla como fichero .jpub.

## Proteger el original

Si abre un fichero .pub y pulsa Ctrl+S, JeffPub guarda los cambios en ese mismo fichero .pub. Para mantener seguro el original:

- Active **Crear siempre una copia de seguridad** en **Archivo > Opciones**, en la pestaña **Guardar**. JeffPub conserva entonces el fichero anterior con el nombre "Copia de seguridad de" seguido de su nombre.
- O bien empiece con **Archivo > Guardar como** y guarde un fichero .jpub. Un fichero .jpub lo conserva todo.

Conserve los ficheros originales hasta que esté seguro de que los nuevos son correctos.

## Temas relacionados

- [Abrir y guardar publicaciones](open-save)
- [Guardar como imagen, página web o libro electrónico](export)
- [Cambiar la configuración de JeffPub](options)
