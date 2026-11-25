# Añadir y ajustar imágenes
<!-- keywords: efectos de imagen, sombra, reflexión, iluminado, bordes suaves, bisel, giro 3D, restablecer imagen y tamaño, foto, imagen, insertar imagen, recortar, imagen prediseñada, página de PDF, jpg, png, svg, volver a colorear, texto alternativo, título, marcador de posición, administrador de gráficos, imágenes en línea, vincular a archivo, imagen vinculada, insertar y vincular, incrustar, imagen que falta, volver a vincular, foto, fotografía, imagen, cuadro, marco, vincular a fichero -->

Una imagen se encuentra dentro de un marco. Puede recortar la imagen y mover o cambiar el tamaño del marco.

## Insertar una imagen desde un fichero

1. Haga clic en **Insertar > Ilustraciones > Imágenes**.
2. En la ventana **Insertar imagen**, elija un fichero. JeffPub abre ficheros PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO y PDF.
3. Elija **Insertar**, **Vincular al fichero** o **Insertar y vincular** en el cuadro **Insertar como**, que está junto a la lista de ficheros y se puede alcanzar con la tecla Tab. Cada vez que se abre la ventana, se elige **Insertar**. Consulte «Vincular una imagen a su fichero», más abajo.
4. Haga clic en **Abrir**.

La imagen aparece en el centro de la página. También puede arrastrar un fichero de imagen a la página; en ese caso siempre se usa **Insertar**.

JeffPub lee ficheros de imagen de hasta 256 MB. Un fichero más grande se rechaza, y también un fichero vacío o cualquier cosa que no sea un fichero normal en el disco, como un dispositivo o una tubería. El mensaje indica que JeffPub no puede leerlo como imagen.

Si elige varias imágenes a la vez, se colocan como miniaturas en el área de trabajo, el espacio gris junto a la página. Arrastre a la página las que quiera.

Si un PDF tiene más de una página, la ventana **Insertar página de PDF** muestra cada una. Elija una y haga clic en **Aceptar**.

## Vincular una imagen a su fichero

**Insertar** pone una copia de la imagen dentro de la publicación. Esa copia viaja con la publicación, pero una publicación con muchas fotos grandes también crece mucho. Un vínculo mantiene las fotos en sus propios ficheros.

- **Insertar** copia la imagen en la publicación. Es la opción habitual.
- **Vincular al fichero** guarda solo la ubicación del fichero y una vista previa pequeña, de no más de 512 píxeles en su lado más largo. La imagen de la página, y la de una impresión, un PDF o un libro electrónico, procede del fichero con calidad completa. Si el fichero se mueve o se elimina, la página muestra la vista previa pequeña.
- **Insertar y vincular** hace ambas cosas. La publicación conserva la imagen completa y el vínculo. Al abrir la publicación, si el fichero ha cambiado, JeffPub lo vuelve a leer y reemplaza su copia.

JeffPub recuerda la ubicación completa del fichero y también su posición respecto a la publicación, por ejemplo «la carpeta Fotos que está junto a ella». Si mueve la publicación y sus imágenes juntas, se usa esta segunda referencia, así que las imágenes se vuelven a encontrar. Mantenga las carpetas como estaban.

JeffPub abre por sí solo el fichero de una imagen vinculada únicamente si el fichero está en la carpeta de la publicación o en una carpeta dentro de ella. Una imagen vinculada a un fichero situado en otro lugar muestra su vista previa guardada, borrosa, y aparece marcada como **No actualizada**, hasta que la seleccione y haga clic en **Actualizar vínculo** en el Administrador de gráficos. Así, una publicación que alguien le envíe no puede acceder a su equipo. Para que las imágenes se carguen solas, guárdelas en la carpeta de la publicación.

El **Administrador de gráficos** indica si cada imagen está **Incrustada**, **Vinculada**, **No encontrada**, **Modificada** o **No actualizada**, y tiene botones para actualizar un vínculo, apuntarlo a otro fichero o convertirlo en una copia. Consulte [Administrador de gráficos](graphics-manager).

Cuando guarde una publicación como fichero `.pub`, cree un fichero de Empaquetar y llevar o la envíe por correo electrónico, JeffPub incluye cada imagen vinculada completa en el fichero, para que no dependa de ficheros que se queden atrás. Si un fichero vinculado falta en ese momento, o la imagen está **No actualizada**, se incluye en su lugar la vista previa pequeña. Un fichero `.pub` que abre nunca tiene vínculos: sus imágenes llegan como copias.

## Añadir un marcador de posición de imagen

Un marcador de posición es un marco vacío que reserva un lugar para una imagen. Haga clic en **Insertar > Ilustraciones > Marcador de posición de imagen**, haga doble clic en el marco y elija un fichero. La imagen llena el marco y se ocultan los bordes que sobresalen. Un marcador vacío no se imprime.

## Buscar imágenes libres en línea

Haga clic en **Insertar > Ilustraciones > Imágenes en línea**. El panel **Imágenes en línea** busca en dos bibliotecas gratuitas de imágenes con licencia abierta: Openverse, que reúne muchas colecciones, y Wikimedia Commons, la biblioteca de imágenes de Wikipedia.

1. Elija una biblioteca, escriba lo que busca, por ejemplo «granero rojo», y pulse Intro.
2. Haga clic en una imagen para ver quién la creó y su licencia. La licencia es el conjunto de condiciones para usar la imagen. La mayoría le pide que cite al autor.
3. Haga clic en **Insertar**. JeffPub coloca la imagen en la página y usa su título como texto alternativo. Si está marcada la opción **Agregar un crédito debajo de la imagen**, aparece debajo una línea como «Granero rojo» de Pat Lee, CC BY 2.0.

Las palabras de búsqueda se envían a la biblioteca que eligió. Compruebe que la licencia de cada imagen se ajusta al uso que vaya a darle.

Una búsqueda muestra hasta 24 imágenes. JeffPub usa solo direcciones web seguras (https) de una biblioteca y omite una imagen cuya dirección no lo sea. Desde Wikimedia Commons, una imagen de más de 1920 píxeles de ancho se descarga como copia de ese ancho, no como el original subido. Una descarga de más de 50 MB, o que tarde demasiado, se detiene, y el panel indica que la imagen no se pudo descargar ni cargar.

## Recortar una imagen

Recortar oculta los bordes de una imagen.

1. Haga clic en la imagen y después en **Formato de imagen > Recortar > Recortar**.
2. Arrastre un controlador para ocultar parte de la imagen. Arrastre la imagen para desplazarla dentro del marco.
3. Haga clic de nuevo en **Recortar** o pulse **Esc**.

También en el grupo **Recortar**: **Relación de aspecto** ajusta el marco a una forma como cuadrado, 4 de ancho por 3 de alto, o 16 por 9, centrado en su punto medio (la imagen se queda donde está en la página); **Ajustar** muestra la imagen entera dentro de su marco; **Rellenar** llena el marco; **Recortar a la forma** ajusta la imagen a una forma como un óvalo; y **Quitar recorte** vuelve a mostrar la imagen completa.

## Cambiar el aspecto de una imagen

En la ficha **Formato de imagen**, use el grupo **Ajustar**:

- **Quitar fondo**: elimina el fondo de una imagen, como la pared lisa detrás de una persona o el blanco alrededor de un logotipo. Haga clic en la imagen y después en **Quitar fondo**. JeffPub localiza los colores de los bordes de la imagen y tiñe de rosa lo que parece fondo. Si ha quitado demasiado o muy poco, haga clic en **Marcar áreas para conservar** o **Marcar áreas para quitar** en la ficha **Quitar fondo** y arrastre sobre la imagen; donde se cruzan dos marcas, gana la que dibujó por último. **Conservar cambios** (o Intro) termina la operación y conserva el resultado, y **Descartar todos los cambios** (o Esc) devuelve todo a su estado original. Toda la sesión es un solo paso para deshacer. La imagen nunca se corta: haga clic de nuevo en **Quitar fondo** para seguir marcando, o en **Restablecer imagen** para recuperar la imagen completa. Una imagen muy grande muestra un cuadro con **Cancelar** mientras JeffPub trabaja. Los dibujos hechos con líneas y formas (SVG) y las imágenes animadas no pueden quitar el fondo. Un fichero .jpub conserva el resultado para que pueda editarlo de nuevo. **Restablecer imagen** quita el fondo eliminado aquí; **Cambiar imagen** no está disponible mientras marca. Ir a otra página, eliminar la imagen o guardar termina Quitar fondo y conserva lo que marcó. Un fichero .pub, un PDF, una página web o un libro electrónico contienen solo la imagen terminada, y Comprobar compatibilidad lo indica para .pub. La imagen terminada se guarda con el tamaño con que se muestra, así que puede verse borrosa si otro programa la amplía; JeffPub la vuelve a generar desde el original cada vez que guarda aquí.
- **Correcciones**: elija una combinación de brillo y contraste, de menos 40 a más 40 por ciento, o **Enfocar** o **Suavizar** (25 % y 50 % en cada sentido). **Dar formato al objeto…**, al final del menú, tiene deslizadores.
- **Volver a colorear** también ofrece cinco opciones de **Temperatura**: los valores negativos son más fríos y azulados; los positivos, más cálidos y rojizos.
- **Efectos artísticos**: muestra una pequeña muestra de su imagen con cada efecto (**Desenfocar**, **Lápiz en escala de grises**, **Bordes iluminados** y **Mosaico**). Elija uno para usarlo, o **Sin efecto artístico** para quitarlo. Cada elección es un paso para deshacer, y **Restablecer imagen** lo quita. La imagen en sí nunca cambia. En un fichero .pub se guarda la imagen terminada, y **Comprobar compatibilidad** indica que allí el efecto no se puede modificar. Un efecto no elimina la imagen original del fichero: **Desenfocar** y **Mosaico** no ocultan nada a quien reciba el fichero, porque el original sigue dentro. Para quitar el original, elija **Archivo**, luego **Información**, luego **Inspeccionar publicación**, y haga clic en **Quitar originales** junto a **Imágenes originales conservadas bajo efectos de imagen**. La imagen conserva entonces solo su aspecto final, y **Restablecer imagen** ya no puede recuperar el original. La misma lista tiene **Partes recortadas de imágenes**: una imagen recortada conserva las partes que recortó y que no se ven, y **Quitar partes recortadas** reduce cada imagen a lo que se muestra, de modo que la página se ve exactamente igual (una imagen con efecto se convierte primero en su imagen terminada, y una imagen que se dibuja más pequeña de lo que está almacenada se deja tal cual, lo que la lista indica). Mientras un efecto se sigue calculando, una imagen con **Desenfocar** o **Mosaico** se muestra como una mancha plana de su color medio, nunca como la imagen que oculta, y la presentación, la impresión y el guardado esperan a que termine.
- **Volver a colorear**: convierta la imagen a Escala de grises, Sepia, Lavado, Blanco y negro o un tinte de color de énfasis. **Establecer color transparente** hace transparente el color de la esquina superior izquierda de la imagen.
- **Transparencia…**: atenúa toda la imagen.
- **Restablecer imagen**: quita todos los cambios de esta lista, así como el borde, la forma y los efectos de la imagen. El recorte también se quita, de modo que el marco muestra la imagen completa. Haga clic en la flecha junto a él para **Restablecer imagen y tamaño**, que además devuelve la imagen al tamaño que tiene en su fichero.
- **Comprimir imágenes…**: reduce el fichero bajando el detalle a 300, 220 o 96 ppp (píxeles por pulgada). Por defecto cambia todas las imágenes y elimina las áreas recortadas.
- **Convertir en formas**: convierte una imagen SVG (un dibujo hecho de líneas y formas) en formas editables, recortadas donde el dibujo las recorta. El texto y las imágenes que contiene se omiten.

## Estilos, bordes y efectos de imagen

Use el grupo **Estilos de imagen** de la ficha **Formato de imagen**:

- La **galería** muestra unos dos docenas de aspectos predefinidos: marcos, sombras, reflexiones, bordes suaves, formas redondeadas y ovaladas, un giro en el espacio, un iluminado y biseles. Un bisel es un borde en pendiente que hace que la imagen parezca elevada. Cada miniatura muestra sus colores. Haga clic en una para dar ese aspecto a la imagen. Un estilo define a la vez la forma, el borde y los efectos, y sustituye a los que tenía la imagen. El recorte, el tamaño y los colores se mantienen.
- **Borde de imagen** define el color, el **Grosor** (el grosor de la línea) y los **Guiones** (una línea continua, punteada o discontinua). **Sin contorno** quita el borde.
- **Efectos de imagen** abre **Sombra**, **Reflexión**, **Iluminado**, **Bordes suaves**, **Bisel** y **Giro 3D**. Cada uno tiene opciones predefinidas. Elija una para añadirla. Los demás efectos se mantienen. Elija la primera entrada, como **Sin sombra**, para quitar uno. **Más opciones de efectos…** abre **Dar formato al objeto**.

Cada cambio es un paso para deshacer.

## Título, texto alternativo e intercambio

- **Formato de imagen > Estilos de imagen > Título** añade un cuadro de texto debajo de la imagen.
- El texto alternativo es una descripción breve para las personas que no pueden ver la imagen, como quienes usan un lector de pantalla. Haga clic con el botón derecho en la imagen, elija **Dar formato al objeto…**, abra la ficha **Texto alternativo** y escriba en **Texto alternativo**.
- **Formato de imagen > Insertar > Cambiar imagen** coloca otro fichero en el mismo marco. El marco conserva su tamaño, su lugar y su aspecto: el borde, la forma, los efectos y los cambios de color. Si la nueva imagen tiene la misma forma que la anterior, el recorte también se conserva. Si tiene otra forma, llena el marco.
- Para intercambiar dos imágenes, arrastre una sobre la otra. Cuando la indicación diga **Suelte para intercambiar las imágenes**, suelte: las imágenes cambian de marco, y el marco que arrastró vuelve a su sitio. También puede mantener pulsada Mayús, hacer clic en ambas y hacer clic en **Intercambiar**. Mantenga pulsada Ctrl mientras arrastra para copiar una imagen en lugar de moverla. Una copia que suelta sobre otra imagen se queda donde la dejó, y no se intercambia nada.

## Ver todas las imágenes

Haga clic en **Vista > Mostrar > Administrador de gráficos** para listar todas las imágenes en el panel **Administrador de gráficos**. Seleccione una y haga clic en **Ir a** para llegar a ella, en **Guardar como…** para guardar una copia, o en **Reemplazar…** para elegir otro fichero. Para las imágenes vinculadas también tiene **Actualizar vínculo**, **Cambiar vínculo…** e **Incrustar imagen**. Consulte [Administrador de gráficos](graphics-manager).

## Temas relacionados

- [Administrador de gráficos](graphics-manager)
- [Dibujar formas y líneas](shapes)
- [Mover, alinear, agrupar y ordenar objetos en capas](arrange)
- [La ficha Formato de imagen](tab-picture-format)
