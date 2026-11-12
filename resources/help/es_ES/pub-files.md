# Trabajar con ficheros .pub
<!-- keywords: pub, .pub, archivo pub, abrir pub, guardar pub, convertir, compatibilidad, importar, publicaciones antiguas, otros programas, jpub, formato, fuentes que faltan, copia de seguridad del original, formato de archivo, fichero pub, formato de fichero -->

Un fichero .pub es una publicación creada con otros programas de autoedición. JeffPub abre ficheros .pub creados desde 1998. También puede guardar su trabajo como fichero .pub. El formato propio de JeffPub es .jpub.

## Abrir un fichero .pub

1. Pulse Ctrl+O o haga clic en **Archivo > Abrir**.
2. Haga clic en **Examinar**, elija el fichero .pub y haga clic en **Abrir**.

También puede arrastrar el fichero a la página. Se importan el texto, las imágenes, las formas y las tablas, junto con los cuadros de texto vinculados. La mayoría de las páginas se parecen mucho al original, pero no todos los detalles coinciden todavía. Es posible que algunos cuadros de texto terminen una palabra antes o después de como lo hacían antes.

Si una fuente del fichero no está en su equipo, JeffPub usa una fuente abierta parecida. Las fuentes guardadas dentro del fichero .pub no se importan. Si JeffPub tiene algo que comunicarle tras abrir el fichero, aparece una nota en la barra de estado, en la parte inferior de la ventana.

Si el fichero no se abre, JeffPub indica que puede estar dañado o que usa características que todavía no puede leer.

## Comprobar el resultado

Abrir un fichero implica una conversión, así que revise cada página. Compruebe los saltos de línea, las fuentes y las imágenes. Use el [Comprobador de diseño](design-checker) para encontrar el texto que no cabe.

## Guardar como fichero .pub

- Haga clic en **Archivo > Exportar** y, después, en **Guardar como fichero .pub**.
- O haga clic en **Archivo > Guardar como** y elija **Ficheros de publicación .pub (*.pub)** en **Tipo**.

Al guardar un fichero .pub, JeffPub puede escribir páginas, cuadros de texto y cuadros de texto vinculados, formato de texto, formas, líneas, imágenes, tablas, rellenos, sombras, texto artístico, estilos, folletos, tarjetas plegadas, sobres, números de página y vínculos. Si algún elemento no tiene equivalente en formato .pub, JeffPub le indica cuántos objetos ha omitido. Esos objetos se mantienen en su publicación y se conservan si guarda como fichero .jpub. Los comentarios tampoco tienen equivalente en .pub: JeffPub indica que los ha omitido y solo se conservan en ficheros .jpub. El tema Comentarios de esta Ayuda da más información.

## Proteger el original

Cuando abre un fichero .pub y pulsa Ctrl+S, JeffPub escribe los cambios en ese mismo fichero .pub. Para conservar el original a salvo:

- Active **Crear siempre una copia de seguridad** en **Archivo > Opciones**, en la pestaña **Guardar**. JeffPub conservará entonces el fichero anterior con el nombre «Backup of» seguido del nombre original.
- O empiece con **Archivo > Guardar como** y guarde un fichero .jpub. Un fichero .jpub conserva todo.

Conserve los ficheros originales hasta estar seguro de que los nuevos son correctos.

## Temas relacionados

- [Abrir y guardar publicaciones](open-save)
- [Guardar como imagen, página web o libro electrónico](export)
- [Cambiar la configuración de JeffPub](options)
