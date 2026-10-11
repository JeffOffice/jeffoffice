# Administrador de gráficos
<!-- keywords: administrador de gráficos, imágenes, fotos, imagen vinculada, vincular al archivo, actualizar vínculo, cambiar vínculo, incrustar imagen, incrustada, no encontrada, falta imagen, modificada, no actualizada, fuera de la carpeta, volver a vincular, buscar archivo de imagen, estado de la imagen, resolución -->

El **Administrador de gráficos** muestra todas las imágenes de la publicación e indica de dónde procede cada una. Es especialmente útil para las imágenes vinculadas a sus archivos, porque una imagen vinculada depende de un archivo que está fuera de la publicación. Para saber cómo vincular una imagen, consulte [Agregar y ajustar imágenes](pictures).

## Abrirlo

Haga clic en **Vista > Mostrar > Administrador de gráficos**. El panel se abre a la derecha. Para cerrarlo, vuelva a hacer clic en el mismo botón o en la **X** de la parte superior del panel.

## Leer la lista

Cada fila muestra una miniatura, el nombre del archivo, la página, el tipo de archivo, su tamaño en KB y su **estado**. El cuadro situado encima de la lista ordena las filas por página, nombre, tamaño o tipo. Debajo de la lista, los detalles de la imagen seleccionada indican su estado, su archivo, si la publicación conserva la imagen completa, su tamaño en píxeles y su **resolución efectiva**: cuántos píxeles de la imagen caben en cada pulgada de la página. Unos 300 es un buen valor para imprimir.

| Estado | Qué significa |
|---|---|
| Incrustada | La imagen se almacena dentro de la publicación. No se necesita nada externo. |
| Vinculada | La imagen está vinculada a un archivo, y el archivo está donde JeffPub lo espera, sin cambios. |
| No encontrada | La imagen está vinculada a un archivo que ya no existe. Si la publicación no conserva ninguna copia, la página muestra la vista previa pequeña, que se ve borrosa, y eso es lo que se imprime. |
| Modificada | El archivo ha cambiado desde que se creó el vínculo o desde la última actualización. Haga clic en **Actualizar vínculo** para incorporar los cambios. |
| No actualizada | La imagen está vinculada a un archivo que está fuera de la carpeta de la publicación, por lo que JeffPub no ha abierto ese archivo. La página muestra lo que almacenó la publicación. Haga clic en **Actualizar vínculo** para usar el archivo. Consulte "Imágenes de fuera de la carpeta de la publicación", más abajo. |

## Qué hacen los botones

- **Ir a** selecciona la imagen en su página.
- **Guardar como…** guarda una copia de la imagen en un archivo.
- **Reemplazar…** coloca un archivo diferente en el mismo marco.
- **Actualizar vínculo** vuelve a leer el archivo. Úselo con una imagen **Modificada**, después de devolver a su sitio un archivo **No encontrado** o con una imagen **No actualizada**. Se actualizan todas las imágenes que comparten el vínculo, y la imagen conserva su posición y su ancho.
- **Cambiar vínculo…** vincula la imagen a otro archivo, por ejemplo, si ha movido los archivos a otra carpeta.
- **Incrustar imagen** convierte el vínculo en una copia almacenada en la publicación. A partir de entonces, la imagen aparece como **Incrustada**. Si falta el archivo, solo se puede incrustar la vista previa pequeña, y la barra de estado se lo indica.

Los tres botones de vínculo aparecen atenuados si la imagen seleccionada no está vinculada. Cada uno es un solo paso, así que **Ctrl+Z** lo deshace.

## Imágenes de fuera de la carpeta de la publicación

Al abrir una publicación, JeffPub lee los archivos de sus imágenes vinculadas que están en la misma carpeta que la publicación o en una subcarpeta. No examina ningún archivo situado en otro lugar, ni en su equipo ni en la red.

El motivo es que una publicación que le envíe otra persona puede hacer referencia a cualquier archivo. Si JeffPub abriera cualquier archivo indicado por la publicación, la publicación de un desconocido podría colocar en sus páginas una de sus imágenes privadas, que usted podría guardar o devolver sin saberlo. En una red, el simple hecho de comprobar si un archivo existe también puede enviar sus credenciales de inicio de sesión de Windows a otro equipo.

Por eso, una imagen vinculada a un archivo situado en otro lugar muestra lo que almacenó la publicación: la imagen completa si la publicación la conserva o, si no, la vista previa pequeña, que se ve borrosa. Su estado es **No actualizada**, y los detalles indican que el archivo está fuera de la carpeta.

Para usar el archivo, seleccione la imagen y haga clic en **Actualizar vínculo**, o haga clic en **Cambiar vínculo…** y elija usted mismo el archivo. Con eso autoriza el uso de esa imagen concreta, y la autorización dura hasta que cierre la publicación. Las demás imágenes siguen como **No actualizada** hasta que haga lo mismo con cada una.

Al guardar, el vínculo se mantiene tal como estaba, así que la próxima vez que abra la publicación la imagen volverá a estar **No actualizada**. Si quiere que las imágenes se carguen solas, guárdelas en la carpeta de la publicación o en una subcarpeta.

Al imprimir, guardar un PDF o guardar un libro electrónico se usa lo que muestra la página. Si una imagen está **No actualizada**, haga clic en **Actualizar vínculo** antes de imprimir. El [Comprobador de diseño](design-checker) muestra estas imágenes.

## Corregir una imagen que falta

1. Seleccione la imagen **No encontrada** en la lista.
2. Haga clic en **Cambiar vínculo…** y elija el archivo. O bien devuelva el archivo a su ubicación original y haga clic en **Actualizar vínculo**.
3. Si el archivo ha desaparecido definitivamente, haga clic en **Incrustar imagen** para conservar la vista previa o en **Reemplazar…** para usar otra imagen.

El [Comprobador de diseño](design-checker) también muestra las imágenes vinculadas que faltan.

## Temas relacionados

- [Agregar y ajustar imágenes](pictures)
- [Comprobar el diseño](design-checker)
- [Guardar como imagen, página web o libro electrónico](export)
