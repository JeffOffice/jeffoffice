# Imprimir, PDF e imágenes de diapositivas
<!-- keywords: imprimir, impresora, impresión, copias, diapositivas, páginas de notas, documentos, esquema, pdf, exportar, imagen, png, jpeg, jpg, imagen, intervalo, diapositivas ocultas, marco, escala de grises, blanco y negro, vista previa de impresión, papel, carta, a4, contraseña, cifrar, proteger con contraseña -->

Puede imprimir la presentación, guardarla como PDF o guardar una diapositiva como imagen. Las tres opciones están en la pantalla Archivo.

## Imprimir

1. Pulse Ctrl+P o haga clic en **Archivo > Imprimir**.
2. En **Imprimir**, elija qué se imprime: **Diapositivas**, **Páginas de notas**, **Esquema** o **Documentos** con 1, 2, 3, 4, 6 o 9 diapositivas por página.
3. En el caso de los documentos de 4 o más diapositivas por página, **Orden** indica si las diapositivas se colocan de izquierda a derecha y luego hacia abajo (**Horizontal**) o de arriba abajo y luego hacia la derecha (**Vertical**).
4. En **Intervalo**, elija **Todas las diapositivas** o **Diapositiva actual**, o elija **Diapositivas** y escriba los números en el cuadro: números de diapositiva e intervalos separados por comas, por ejemplo 1-3, 5.
5. Marque lo que necesite: **Imprimir diapositivas ocultas**, **Enmarcar diapositivas** (una línea fina alrededor de cada diapositiva) y **Escala de grises**.
6. En **Caras**, elija **Imprimir a una cara**, **Imprimir a doble cara, voltear por el borde largo** (páginas que se pasan como un libro), **Imprimir a doble cara, voltear por el borde corto** (páginas que se pasan como un bloc) o **Imprimir a doble cara manualmente**. Las dos primeras aparecen atenuadas si su impresora no puede imprimir a doble cara; sitúe el puntero sobre una de ellas para ver el motivo. **Manualmente** es para esa impresora: JeffDeck imprime la primera, la tercera, la quinta y así sucesivamente, y luego un mensaje le pide que dé la vuelta a la pila y la vuelva a colocar en la impresora. Al hacer clic en **Aceptar**, imprime las demás, empezando por la última página. Con una sola página no hay que dar la vuelta a nada. JeffDeck recuerda su elección.
7. Haga clic en **Imprimir**. JeffDeck abre la ventana de impresión de su sistema, donde elige la impresora y el número de copias. Esa ventana empieza con la opción de caras que eligió. Un fichero PDF creado desde esa ventana no tiene caras.

La vista previa de la derecha muestra cada página que se imprimirá, con flechas para pasar de una a otra. Se dibuja igual que se imprime la página. JeffDeck recuerda sus opciones para la próxima vez.

## Qué imprime cada opción

- **Diapositivas** imprime una diapositiva por página, con la forma propia de la diapositiva. El papel se gira en horizontal para una diapositiva ancha.
- **Páginas de notas** coloca una diapositiva en la parte superior de la página, con sus notas del orador debajo.
- **Documentos** colocan una cuadrícula de diapositivas en cada página, con un encabezado y un pie de página que contienen el título, la fecha y el número de página. Con 3 por página, hay líneas junto a cada diapositiva para escribir.
- **Esquema** imprime el número y el título de cada diapositiva, seguidos de su texto, con sangría según el nivel.

Las páginas de notas, los documentos y el esquema se imprimen en papel carta o A4 vertical, según el que use su región. Las diapositivas ocultas no se imprimen a menos que marque **Imprimir diapositivas ocultas**.

## Guardar como PDF

Haga clic en **Archivo > Exportar > Exportar a PDF**. Una ventana pregunta qué incluir: las mismas opciones de **Imprimir**, **Orden** e **Intervalo** que al imprimir, y las mismas tres casillas. Muestra cuántas páginas tendrá el fichero. Haga clic en **Exportar…** y luego elija un nombre y una ubicación.

El PDF tiene texto real, con las fuentes incluidas, así que se ve igual en cualquier equipo. Su título es el de la presentación o, si no lo tiene, el nombre del fichero.

Marque **PDF/A, para conservar durante mucho tiempo** para crear un fichero PDF/A-1b. Todas las fuentes van incluidas en el fichero y los efectos de transparencia se aplanan, de modo que las diapositivas se vean igual dentro de años. No está etiquetado para lectores de pantalla. Un PDF con contraseña no puede ser un fichero PDF/A, así que elegir una contraseña desactiva PDF/A.

## Enviar una presentación por correo electrónico

Haga clic en **Archivo > Exportar > Crear un correo electrónico**. Elija la presentación como fichero o como PDF. JeffDeck crea un fichero de mensaje (.eml) con ella adjunta y pregunta dónde guardarlo. No se envía nada. Abra el fichero en su programa de correo, añada a los destinatarios y envíelo. Una presentación que tiene contraseña se adjunta con la contraseña todavía puesta. Un PDF puede tener una contraseña propia.

## Proteger un PDF con contraseña

En la ventana **Exportar a PDF**, haga clic en **Cifrar con contraseña…**. Escriba la contraseña dos veces. Quien abra el PDF deberá escribirla. La contraseña protege de verdad el fichero: las páginas se cifran con un cifrado fuerte (AES-256) y nadie puede leerlas sin ella.

También puede marcar **Restringir la impresión, la copia y la edición**, escribir una segunda contraseña y elegir qué se permite. Esas restricciones son solo una petición. La mayoría de los lectores de PDF las respetan, pero no impiden a quien quiera saltárselas, así que no confíe en ellas para mantener un fichero privado.

Una contraseña olvidada no se puede recuperar, así que guárdela en un lugar seguro. La contraseña se usa solo para ese PDF y nunca se guarda.

Al insertar como imagen un PDF con contraseña, JeffDeck le pide la contraseña. Si es incorrecta, vuelve a pedirla; haga clic en **Cancelar** para detenerse. La diapositiva recibe una copia de la página que no necesita contraseña. Si el propietario del PDF pide que no se copie, JeffDeck lo inserta igualmente y muestra una nota que lo indica; esa petición no es un bloqueo.

## Guardar una diapositiva como imagen

Haga clic en **Archivo > Exportar > Exportar diapositiva como imagen**. JeffDeck guarda la diapositiva en la que se encuentra como imagen PNG o JPEG, de 1920 píxeles de ancho, sobre fondo blanco. Sugiere como nombre de fichero el de la presentación y el número de la diapositiva.

## Temas relacionados

- [La pantalla Archivo](file-screen)
- [Notas del orador](notes)
- [Agregar, cambiar y eliminar diapositivas](slides)
