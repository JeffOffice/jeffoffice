# Imprimir, PDF e imágenes de diapositivas

<!-- keywords: imprimir, impresora, impresión, copias, intercalar, intercalada, selección, última página primero, invertir, zoom, diapositivas, páginas de notas, documentos, esquema, pdf, exportar, imagen, png, jpeg, jpg, gif, tiff, tif, bmp, svg, vectorial, imagen, patrón de notas, patrón de documento, intervalo, diapositivas ocultas, marco, escala de grises, blanco y negro, vista previa de impresión, papel, carta, a4, contraseña, cifrar, proteger con contraseña, a5 -->

Puede imprimir la presentación, guardarla como PDF o guardar una diapositiva como imagen. Las tres opciones están en la pantalla Archivo.

## Imprimir

1. Pulse Ctrl+P o haga clic en **Archivo > Imprimir**.
2. Junto al botón **Imprimir**, configure **Copias**. Elija una **Impresora**: la lista termina con **Guardar como PDF**, que crea un fichero PDF en lugar de papel. Con más de una copia, **Intercalar** imprime el conjunto completo otra vez por cada copia (1, 2, 3, y luego 1, 2, 3), y con la opción desactivada las copias de cada página salen juntas (1, 1, 2, 2, 3, 3). Un fichero PDF se crea una sola vez, sea cual sea el número de copias. **Propiedades de la impresora…** abre la ventana de impresión del sistema para esa impresora, con sus propias opciones (la bandeja, la calidad); lo que elija allí se guarda para el trabajo, y las copias, la intercalación, el papel y la escala de grises vuelven a esta página. **Papel** define el tamaño de la hoja; **Como indican las diapositivas** es la opción habitual.
3. En **Imprimir**, elija qué sale: **Diapositivas**, **Páginas de notas**, **Esquema** o **Documentos** con 1, 2, 3, 4, 6 o 9 diapositivas por página.
4. En los documentos de 4 o más diapositivas por página, **Orden** indica si las diapositivas recorren la página de izquierda a derecha y luego hacia abajo (**Horizontal**), o hacia abajo y luego a la derecha (**Vertical**).
5. En **Intervalo**, elija **Todas las diapositivas**, **Diapositiva actual** o **Selección** (las diapositivas seleccionadas en el clasificador de diapositivas, o la diapositiva en la que está si no está en el clasificador), o elija **Diapositivas** y escriba los números en el cuadro: números de diapositiva e intervalos separados por comas, por ejemplo 1-3, 5.
6. Marque lo que necesite: **Última página primero** (las hojas salen en orden inverso, así que una pila sale en orden; a doble cara, cada anverso queda con su reverso, de modo que las diapositivas 1 a 4 salen como 3 y 4, y luego 1 y 2; la opción aparece atenuada cuando imprime a doble cara a mano), **Imprimir diapositivas ocultas**, **Enmarcar diapositivas** (una línea fina alrededor de cada diapositiva) y **Escala de grises**.
7. En **Caras**, elija **Imprimir a una cara**, **Imprimir a doble cara, voltear por el borde largo** (páginas que se pasan como un libro), **Imprimir a doble cara, voltear por el borde corto** (páginas que se pasan como un bloc), o **Imprimir a doble cara manualmente**. Las dos primeras opciones están atenuadas si su impresora no puede imprimir a doble cara; apunte a una para ver el motivo. **Manualmente** es para ese tipo de impresora: JeffDeck imprime las páginas primera, tercera, quinta, y así sucesivamente; luego un mensaje le pide que dé la vuelta a la pila y la vuelva a colocar en la impresora. Al hacer clic en **Aceptar**, imprime las demás, de la última página a la primera. Con una sola página no hay nada que voltear. JeffDeck recuerda su elección.
8. Haga clic en **Imprimir**. Para un fichero PDF, JeffDeck le pide un nombre. Un fichero PDF no tiene caras.

La vista previa de la derecha muestra cada página que saldrá, con flechas para pasar de una a otra. **Zoom** sobre ella amplía la página más allá del tamaño que cabe; desplácese para ver el resto. Se dibuja de la misma forma en que se imprime la página. JeffDeck recuerda sus opciones para la próxima vez.

## Qué imprime cada opción

- **Diapositivas** imprime una diapositiva por página, con la forma propia de la diapositiva. El papel se gira en horizontal para una diapositiva ancha.
- **Páginas de notas** coloca una diapositiva en la parte superior de la página, con sus notas del orador debajo.
- **Documentos** colocan una cuadrícula de diapositivas en cada página, con un encabezado y un pie de página que contienen el título, la fecha y el número de página. Con 3 por página, hay líneas junto a cada diapositiva para escribir.
- **Esquema** imprime el número y el título de cada diapositiva, seguidos de su texto, con sangría según el nivel.

Las páginas de notas, los documentos y el esquema se imprimen en papel carta o A4, según el que use su región, en posición vertical. Un documento se imprime en horizontal si su patrón de documento indica Horizontal. La posición de la diapositiva, las notas, el encabezado, la fecha, el pie de página y el número de página se toman del patrón de notas y del patrón de documento, que se cambian en **Vista > Vistas patrón**. Las diapositivas ocultas no se imprimen a menos que active **Imprimir diapositivas ocultas**.

## Guardar como PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Una ventana pregunta qué incluir: las mismas opciones **Imprimir**, **Orden** e **Intervalo** que para imprimir, y las mismas tres casillas. Muestra cuántas páginas tendrá el fichero. Haga clic en **Exportar…** y, después, elija un nombre y una ubicación.

El PDF contiene texto real, con las fuentes incluidas, así que se ve igual en cualquier equipo. Su título es el título de la presentación o el nombre del fichero.

**Etiquetas de estructura del documento para la accesibilidad** (activada de forma predeterminada) crea un PDF *etiquetado*: el título de cada diapositiva es un encabezado, sus objetos están en el orden que lee un lector de pantalla (el orden de lectura de la diapositiva), las imágenes y las formas tienen texto alternativo, y una tabla es una tabla. **Crear marcadores** (activada de forma predeterminada) añade un marcador en el PDF por cada diapositiva, con el título de la diapositiva, o «Diapositiva 3» si no tiene título. El diseño de esquema no tiene ninguno de los dos.

Marque **PDF/A, para conservar durante mucho tiempo** para crear un fichero PDF/A-1b. Todas las fuentes van dentro del fichero, y los efectos transparentes se aplanan, de modo que las diapositivas se ven igual dentro de años. Las etiquetas y los marcadores se conservan. Un PDF con contraseña no puede ser un fichero PDF/A, así que al elegir una contraseña se desactiva PDF/A.

## Enviar una presentación por correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico**. Elija la presentación como fichero o como PDF. JeffDeck crea un fichero de mensaje (.eml) con ella adjunta y le pide dónde guardarlo. No se envía nada. Abra el fichero en su programa de correo, añada a quién va dirigido y envíelo. Una presentación que tiene contraseña se adjunta con la contraseña todavía puesta. Un PDF puede tener una contraseña propia.

## Proteger un PDF con contraseña

En la ventana **Exportar a PDF**, haga clic en **Cifrar con contraseña…**. Escriba la contraseña dos veces. Quien abra el PDF tendrá que escribirla. La contraseña protege realmente el fichero: las páginas se cifran con un cifrado fuerte (AES-256), y nadie puede leerlas sin ella.

También puede marcar **Restringir la impresión, la copia y la edición**, indicar una segunda contraseña y elegir qué se permite. Esas restricciones son solo una petición. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un fichero privado.

Una contraseña olvidada no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda.

Al insertar como imagen un PDF que tiene contraseña, JeffDeck la pide. Si es incorrecta, la vuelve a pedir; haga clic en **Cancelar** para detenerse. La diapositiva recibe una copia de la página que no necesita contraseña. Si el propietario del PDF pide que no se copie, JeffDeck lo inserta igualmente y muestra una nota que lo indica; esa petición no es un bloqueo.

## Guardar una diapositiva como imagen

Haga clic en **Archivo > Exportar > Exportar diapositiva como imagen**. JeffDeck guarda la diapositiva en la que está como imagen PNG, JPEG, GIF, TIFF, BMP o SVG. Los formatos con píxeles (todos excepto SVG) tienen 1920 píxeles de ancho, sobre fondo blanco. Un GIF tiene como máximo 256 colores, así que una fotografía en ese formato puede verse más pixelada.

Una imagen **SVG** es un dibujo, no píxeles. Se ve nítida a cualquier tamaño, las letras se guardan como formas para que cualquier equipo muestre las mismas, y las imágenes de la diapositiva van dentro del fichero. Una diapositiva que usa algunos efectos, como una sombra suave, puede verse un poco diferente en SVG.

JeffDeck sugiere como nombre de fichero el nombre de la presentación y el número de la diapositiva.

Para guardar todas las diapositivas, haga clic en **Guardar diapositivas como imágenes** junto a esa opción. Elija el tipo de imagen (PNG, JPEG, GIF, TIFF, BMP o SVG) y una carpeta. JeffDeck crea un fichero por cada diapositiva, con los nombres **Diapositiva 1**, **Diapositiva 2**, y así sucesivamente. El cuadro **Ancho** define los píxeles de ancho y no se usa para SVG.

## Temas relacionados

- [La pantalla Archivo](file-screen)
- [Notas del orador](notes)
- [Agregar, cambiar y eliminar diapositivas](slides)
