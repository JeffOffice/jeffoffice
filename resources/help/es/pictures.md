# Agregar y ajustar imágenes
<!-- keywords: foto, fotografía, imagen, insertar imagen, recortar, imágenes prediseñadas, página de PDF, jpg, png, svg, volver a colorear, texto alternativo, título, pie de foto, marcador de posición, administrador de gráficos, imágenes en línea, vincular al archivo, imagen vinculada, insertar y vincular, incrustar, imagen no encontrada, falta imagen, volver a vincular -->

Una imagen se encuentra dentro de un marco. Puede recortar la imagen y mover o cambiar el tamaño del marco.

## Insertar una imagen desde un archivo

1. Haga clic en **Insertar > Ilustraciones > Imágenes**.
2. En la ventana **Insertar imagen**, elija un archivo. JeffPub abre archivos PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO y PDF.
3. Elija **Insertar**, **Vincular al archivo** o **Insertar y vincular** en el cuadro **Insertar como**, que se encuentra junto a la lista de archivos y al que se puede llegar con la tecla Tab. Cada vez que se abre la ventana, aparece seleccionado **Insertar**. Consulte "Vincular una imagen a su archivo", más abajo.
4. Haga clic en **Abrir**.

La imagen aparece en el centro de la página. También puede arrastrar un archivo de imagen a la página, lo que siempre usa **Insertar**.

JeffPub lee archivos de imagen de hasta 256 MB. Un archivo más grande se rechaza, al igual que un archivo vacío o cualquier cosa que no sea un archivo normal del disco, como un dispositivo o una canalización. El mensaje indica que JeffPub no puede leerlo como imagen.

Si elige varias imágenes a la vez, se alinean como miniaturas en el área de trabajo, el espacio gris que hay junto a la página. Arrastre a la página las que quiera usar.

Si un PDF tiene más de una página, la ventana **Insertar página de PDF** muestra cada página. Elija una y haga clic en **Aceptar**.

## Vincular una imagen a su archivo

**Insertar** coloca una copia de la imagen dentro de la publicación. Esa copia va con la publicación a todas partes, pero una publicación con muchas fotos grandes también se vuelve grande. Un vínculo, en cambio, mantiene las fotos en sus propios archivos.

- **Insertar** copia la imagen en la publicación. Es la opción habitual.
- **Vincular al archivo** guarda solo la ubicación del archivo y una pequeña vista previa, de 512 píxeles como máximo en su lado más largo. La imagen de la página, y la de una impresión, un PDF o un libro electrónico, procede del archivo con calidad completa. Si el archivo se mueve o se elimina, la página muestra la pequeña vista previa.
- **Insertar y vincular** hace ambas cosas. La publicación guarda la imagen completa y el vínculo. Cuando abre la publicación y el archivo ha cambiado, JeffPub vuelve a leerlo y reemplaza su copia.

JeffPub recuerda la ubicación completa del archivo y también dónde está en relación con la publicación, como "la carpeta Fotos que está a su lado". Si mueve la publicación y sus imágenes juntas, prevalece la segunda, de modo que las imágenes se vuelven a encontrar. Mantenga las carpetas tal como estaban.

JeffPub abre por sí solo el archivo de una imagen vinculada únicamente cuando el archivo está en la carpeta de la publicación o en una carpeta dentro de ella. Una imagen vinculada a un archivo situado en cualquier otro lugar muestra su vista previa guardada, borrosa, y se marca como **No actualizada** hasta que la selecciona y hace clic en **Actualizar vínculo** en el Administrador de gráficos. Así se evita que una publicación que alguien le envíe acceda a su equipo. Para que las imágenes se carguen solas, guárdelas en la carpeta de la publicación.

El **Administrador de gráficos** muestra si cada imagen está **Incrustada**, **Vinculada**, **No encontrada**, **Modificado** o **No actualizada**, y tiene botones para actualizar un vínculo, dirigirlo a otro archivo o convertirlo en una copia. Consulte [Administrador de gráficos](graphics-manager).

Cuando guarda una publicación como archivo `.pub`, crea un archivo con Empaquetar o la envía por correo electrónico, JeffPub incluye completa en el archivo cada imagen vinculada, para que no dependa de archivos que se quedan atrás. Si en ese momento falta un archivo vinculado, o la imagen está **No actualizada**, se incluye su pequeña vista previa. Un archivo `.pub` que abra nunca tiene vínculos: sus imágenes entran como copias.

## Agregar un marcador de posición de imagen

Un marcador de posición es un marco vacío que reserva un sitio para una imagen. Haga clic en **Insertar > Ilustraciones > Marcador de posición de imagen**, haga doble clic en el marco y elija un archivo. La imagen rellena el marco y los bordes que sobresalen quedan ocultos. Un marcador de posición vacío no se imprime.

## Buscar imágenes gratuitas en línea

Haga clic en **Insertar > Ilustraciones > Imágenes en línea**. El panel **Imágenes en línea** busca en dos bibliotecas gratuitas de imágenes con licencia abierta: Openverse, que abarca muchas colecciones, y Wikimedia Commons, la biblioteca de imágenes de Wikipedia.

1. Elija una biblioteca, escriba lo que busca, como "granero rojo", y presione Entrar.
2. Haga clic en una imagen para ver quién la creó y su licencia. Una licencia es el conjunto de normas para usar la imagen. La mayoría exigen citar al autor.
3. Haga clic en **Insertar**. JeffPub coloca la imagen en la página y usa su título como texto alternativo. Con **Agregar un crédito debajo de la imagen** activado, debajo aparece una línea como "Red barn" by Pat Lee, CC BY 2.0.

Las palabras de búsqueda se envían a la biblioteca que haya elegido. Compruebe que la licencia de cada imagen se ajusta al uso que le vaya a dar.

Una búsqueda muestra hasta 24 imágenes. JeffPub solo usa direcciones web seguras (https) de una biblioteca y omite las imágenes cuya dirección no lo sea. En Wikimedia Commons, una imagen de más de 1920 píxeles de ancho llega como una copia de ese ancho, no como el archivo original. Una descarga de más de 50 MB, o que tarde demasiado, se detiene, y el panel indica que no se pudo descargar o cargar la imagen.

## Recortar una imagen

Al recortar se ocultan los bordes de una imagen.

1. Haga clic en la imagen y, a continuación, en **Formato de imagen > Recortar > Recortar**.
2. Arrastre un controlador para ocultar parte de la imagen. Arrastre la imagen para desplazarla dentro del marco.
3. Vuelva a hacer clic en **Recortar** o presione **Esc**.

También en el grupo **Recortar**: **Ajustar** muestra la imagen completa en su marco, **Relleno** rellena el marco, **Recortar a la forma** recorta según una forma, como un óvalo, y **Borrar recorte** vuelve a mostrar la imagen completa.

## Cambiar el aspecto de una imagen

En la pestaña **Formato de imagen**, use el grupo **Ajustar**:

- **Correcciones**: elija una combinación de brillo y contraste, de menos 40 a más 40 por ciento. **Formato de objeto…**, al final del menú, tiene controles deslizantes.
- **Volver a colorear**: aplique a la imagen Escala de grises, Sepia, Decolorar, Blanco y negro o un tinte de énfasis. **Establecer color transparente** vuelve transparente el color de la esquina superior izquierda de la imagen.
- **Transparencia…**: atenúa toda la imagen.
- **Restablecer imagen**: deshace estos cambios.
- **Comprimir imágenes…**: reduce el archivo bajando el detalle a 300, 220 o 96 ppp (píxeles por pulgada). De forma predeterminada, cambia todas las imágenes y elimina las áreas recortadas.
- **Convertir en formas**: convierte una imagen SVG (un dibujo hecho de líneas y formas) en formas que puede modificar, cortadas donde el dibujo las recorta. El texto y las imágenes que contiene se omiten.

## Título, texto alternativo e intercambio

- **Formato de imagen > Estilos de imagen > Título** agrega un cuadro de texto debajo de la imagen.
- El texto alternativo es una breve descripción para quienes no pueden ver la imagen, como los usuarios de lectores de pantalla. Haga clic con el botón derecho en la imagen, elija **Formato de objeto…**, abra la pestaña **Texto alternativo** y escriba en **Texto alternativo:**.
- **Formato de imagen > Insertar > Cambiar imagen** coloca otro archivo en el mismo marco.
- Para intercambiar dos imágenes, arrastre una sobre la otra. Cuando aparezca la sugerencia **Suelte para intercambiar las imágenes**, suelte: las imágenes cambian de marco y el marco que arrastró vuelve a su sitio. También puede mantener presionada Mayús, hacer clic en ambas y hacer clic en **Intercambiar**. Mantenga presionada Ctrl mientras arrastra para copiar una imagen. Una copia que suelte sobre otra imagen se queda donde la coloque y no se intercambia nada.

## Ver todas las imágenes

Haga clic en **Vista > Mostrar > Administrador de gráficos** para ver la lista de todas las imágenes en el panel **Administrador de gráficos**. Seleccione una y haga clic en **Ir a** para ir a ella, en **Guardar como…** para guardar una copia o en **Reemplazar…** para elegir otro archivo. Para las imágenes vinculadas, también tiene **Actualizar vínculo**, **Cambiar vínculo…** e **Incrustar imagen**. Consulte [Administrador de gráficos](graphics-manager).

## Temas relacionados

- [Administrador de gráficos](graphics-manager)
- [Dibujar formas y líneas](shapes)
- [Mover, alinear, agrupar y organizar objetos en capas](arrange)
- [La pestaña Formato de imagen](tab-picture-format)
