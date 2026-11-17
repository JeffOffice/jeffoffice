# Agregar y ajustar imágenes
<!-- keywords: efectos de imagen, sombra, reflexión, iluminado, bordes suaves, bisel, giro 3D, restablecer imagen y tamaño, foto, fotografía, imagen, insertar imagen, recortar, imagen prediseñada, página de PDF, jpg, png, svg, volver a colorear, texto alternativo, título, marcador de posición, administrador de gráficos, imágenes en línea, vincular al archivo, imagen vinculada, insertar y vincular, incrustar, imagen que falta, volver a vincular, cambiar imagen, recorte, formato de imagen, estilos de imagen, comprimir imágenes, transparencia, borde de imagen, vincular al fichero -->

Una imagen está dentro de un marco. Puede recortar la imagen, y mover o cambiar el tamaño del marco.

## Insertar una imagen desde un fichero

1. Haga clic en **Insertar > Ilustraciones > Imágenes**.
2. En la ventana **Insertar imagen**, elija un fichero. JeffPub abre ficheros PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO y PDF.
3. Elija **Insertar**, **Vincular al fichero** o **Insertar y vincular** en el cuadro **Insertar como**, que está junto a la lista de ficheros y se puede alcanzar con la tecla Tab. Cada vez que se abre la ventana, se elige **Insertar**. Vea «Vincular una imagen a su fichero», más abajo.
4. Haga clic en **Abrir**.

La imagen aparece en el centro de la página. También puede arrastrar un fichero de imagen a la página, lo que siempre usa **Insertar**.

JeffPub lee ficheros de imagen de hasta 256 MB. Si el fichero es mayor, se rechaza. También se rechaza un fichero vacío o cualquier elemento que no sea un fichero normal del disco, como un dispositivo o una tubería. El mensaje indica que JeffPub no puede leerlo como imagen.

Si elige varias imágenes a la vez, aparecen en miniatura en el área de trabajo, el espacio gris junto a la página. Arrastre a la página las que desee.

Si un PDF tiene más de una página, la ventana **Insertar página de PDF** muestra cada una. Elija una y haga clic en **Aceptar**.

## Vincular una imagen a su fichero

**Insertar** pone una copia de la imagen dentro de la publicación. Esa copia viaja con la publicación, pero una publicación con muchas fotos grandes también crece mucho. Un vínculo mantiene las fotos en sus propios ficheros.

- **Insertar** copia la imagen dentro de la publicación. Es la opción habitual.
- **Vincular al fichero** guarda solo la ubicación del fichero y una vista previa pequeña, de no más de 512 píxeles en su lado más largo. La imagen de la página, y la de una impresión, un PDF o un libro electrónico, se toma del fichero con calidad completa. Si el fichero se mueve o se elimina, la página muestra la vista previa pequeña.
- **Insertar y vincular** hace ambas cosas. La publicación conserva la imagen completa y el vínculo. Al abrir la publicación, si el fichero ha cambiado, JeffPub lo lee de nuevo y reemplaza su copia.

JeffPub recuerda la ubicación completa del fichero, y también dónde está en relación con la publicación, por ejemplo «la carpeta Fotos junto a ella». Si mueve la publicación y sus imágenes juntas, se usa esta segunda referencia, y así las imágenes se vuelven a encontrar. Conserve las carpetas como estaban.

JeffPub abre por sí solo el fichero de una imagen vinculada únicamente si el fichero está en la carpeta de la publicación o en una carpeta dentro de ella. Una imagen vinculada a un fichero situado en otro lugar muestra su vista previa guardada, borrosa, y aparece marcada como **No actualizada**, hasta que la seleccione y haga clic en **Actualizar vínculo** en el Administrador de gráficos. Así, una publicación que alguien le envíe no puede acceder a su equipo. Para que las imágenes se carguen solas, guárdelas en la carpeta de la publicación.

El **Administrador de gráficos** muestra si cada imagen está **Incrustada**, **Vinculada**, **Falta**, **Modificada** o **No actualizada**, y tiene botones para actualizar un vínculo, apuntarlo a otro fichero o convertirlo en una copia. Vea [Administrador de gráficos](graphics-manager).

Al guardar una publicación como fichero `.pub`, crear un fichero de Empaquetar o enviarla por correo electrónico, JeffPub incluye cada imagen vinculada completa en el fichero, para que no dependa de ficheros que se quedan atrás. Si un fichero vinculado falta en ese momento, o la imagen está **No actualizada**, se incluye su vista previa pequeña. Un fichero `.pub` que abra nunca tiene vínculos: sus imágenes llegan como copias.

## Agregar un marcador de posición de imagen

Un marcador de posición es un marco vacío que reserva el lugar de una imagen. Haga clic en **Insertar > Ilustraciones > Marcador de posición de imagen**, haga doble clic en el marco y elija un fichero. La imagen llena el marco y se ocultan los bordes que sobresalen. Un marcador de posición vacío no se imprime.

## Buscar imágenes gratuitas en línea

Haga clic en **Insertar > Ilustraciones > Imágenes en línea**. El panel **Imágenes en línea** busca en dos bibliotecas gratuitas de imágenes con licencia abierta: Openverse, que reúne muchas colecciones, y Wikimedia Commons, la biblioteca de imágenes de Wikipedia.

1. Elija una biblioteca, escriba lo que busca, por ejemplo «granero rojo», y pulse Entrar.
2. Haga clic en una imagen para ver quién la hizo y su licencia. Una licencia es el conjunto de normas para usar la imagen. La mayoría pide que se nombre al autor.
3. Haga clic en **Insertar**. JeffPub pone la imagen en la página y usa su título como texto alternativo. Si marca **Agregar un crédito debajo de la imagen**, debajo aparece una línea como «Granero rojo», de Pat Lee, CC BY 2.0.

Las palabras de búsqueda se envían a la biblioteca que elija. Compruebe que la licencia de cada imagen le sirva para el uso que le va a dar.

Una búsqueda muestra hasta 24 imágenes. JeffPub solo usa direcciones web seguras (https) de una biblioteca, y deja fuera cualquier imagen cuya dirección no lo sea. Desde Wikimedia Commons, una imagen de más de 1920 píxeles de ancho se descarga con ese ancho, no como el original subido. Una descarga de más de 50 MB, o que tarda demasiado, se detiene, y el panel indica que la imagen no se pudo descargar ni cargar.

## Recortar una imagen

Recortar oculta los bordes de una imagen.

1. Haga clic en la imagen y después en **Formato de imagen > Recortar > Recortar**.
2. Arrastre un controlador para ocultar parte de la imagen. Arrastre la imagen para deslizarla dentro del marco.
3. Haga clic de nuevo en **Recortar**, o pulse **Esc**.

También en el grupo **Recortar**: **Relación de aspecto** recorta el marco a una forma como un cuadrado, 4 de ancho por 3 de alto, o 16 por 9, alrededor de su centro (la imagen sigue en el mismo lugar de la página). **Ajustar** muestra la imagen entera dentro de su marco. **Rellenar** llena el marco. **Recortar a la forma** recorta a una forma como un óvalo, y **Quitar recorte** vuelve a mostrar la imagen completa.

## Cambiar el aspecto de una imagen

En la pestaña **Formato de imagen**, use el grupo **Ajustar**:

- **Correcciones**: elija una combinación de brillo y contraste, de menos 40 a más 40 por ciento. **Formato de objeto…**, al final del menú, tiene controles deslizantes.
- **Volver a colorear**: convierte la imagen en Escala de grises, Sepia, Lavado, Blanco y negro, o en un tono de color de énfasis. **Establecer color transparente** hace transparente el color de la esquina superior izquierda de la imagen.
- **Transparencia…**: difumina la imagen completa.
- **Restablecer imagen**: quita todos los cambios de esta lista, y también el borde, la forma y los efectos de la imagen. El recorte también se quita, de modo que el marco muestra la imagen completa. Haga clic en la flecha junto a él para **Restablecer imagen y tamaño**, que además devuelve la imagen al tamaño que tiene en su fichero.
- **Comprimir imágenes…**: reduce el fichero bajando el detalle a 300, 220 o 96 ppp (píxeles por pulgada). Por defecto cambia todas las imágenes y elimina las áreas recortadas.
- **Convertir en formas**: convierte una imagen SVG (un dibujo hecho de líneas y formas) en formas que puede editar, recortadas donde el dibujo las corta. El texto y las imágenes que contiene se omiten.

## Estilos, bordes y efectos de imagen

Use el grupo **Estilos de imagen** de la pestaña **Formato de imagen**:

- La **galería** muestra unos dos docenas de aspectos predefinidos: marcos, sombras, reflexiones, bordes suaves, formas redondeadas y ovaladas, un giro en el espacio, un iluminado y biseles. Un bisel es un borde inclinado que hace que la imagen parezca en relieve. Cada miniatura muestra sus colores. Haga clic en una para dar ese aspecto a la imagen. Un estilo define a la vez la forma, el borde y los efectos, y reemplaza los que tenía la imagen. El recorte, el tamaño y los colores se conservan.
- **Borde de imagen** define el color, el **Grosor** (lo gruesa que es la línea) y los **Guiones** (una línea continua, punteada o discontinua). **Sin contorno** quita el borde.
- **Efectos de imagen** abre **Sombra**, **Reflexión**, **Iluminado**, **Bordes suaves**, **Bisel** y **Giro 3D**. Cada uno tiene opciones predefinidas. Elija una para agregarla. Los demás efectos se mantienen. Elija la primera entrada, como **Sin sombra**, para quitar uno. **Más opciones de efectos…** abre **Formato de objeto**.

Cada paso se puede deshacer por separado.

## Título, texto alternativo e intercambio

- **Formato de imagen > Estilos de imagen > Título** añade un cuadro de texto debajo de la imagen.
- El texto alternativo es una descripción breve para las personas que no pueden ver la imagen, como los usuarios de lectores de pantalla. Haga clic con el botón derecho en la imagen, elija **Formato de objeto…**, abra la pestaña **Texto alternativo** y escriba en **Texto alternativo**.
- **Formato de imagen > Insertar > Cambiar imagen** pone otro fichero en el mismo marco. El marco conserva su tamaño, su lugar y su aspecto: el borde, la forma, los efectos y los cambios de color. Si la nueva imagen tiene la misma forma que la anterior, el recorte también se conserva. Si tiene otra forma, llena el marco.
- Para intercambiar dos imágenes, arrastre una sobre la otra. Cuando la información indique **Suelte para intercambiar las imágenes**, suelte el botón: las imágenes cambian de marco, y el marco que arrastró vuelve a su sitio. También puede mantener pulsada la tecla Mayús, hacer clic en ambas y hacer clic en **Intercambiar**. Mantenga pulsada Ctrl mientras arrastra para copiar una imagen en lugar de moverla. Una copia que suelte sobre otra imagen se queda donde la dejó, y no se intercambia nada.

## Ver todas las imágenes

Haga clic en **Vista > Mostrar > Administrador de gráficos** para listar todas las imágenes en el panel **Administrador de gráficos**. Seleccione una y haga clic en **Ir a** para saltar hasta ella, en **Guardar como…** para guardar una copia, o en **Reemplazar…** para elegir otro fichero. En las imágenes vinculadas también hay **Actualizar vínculo**, **Cambiar vínculo…** e **Incrustar imagen**. Vea [Administrador de gráficos](graphics-manager).

## Temas relacionados

- [Administrador de gráficos](graphics-manager)
- [Dibujar formas y líneas](shapes)
- [Mover, alinear, agrupar y ordenar objetos en capas](arrange)
- [La pestaña Formato de imagen](tab-picture-format)
