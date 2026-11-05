# El cuadro de diálogo Configurar página

<!-- keywords: configurar página, cuadro de diálogo, encabezado, pie de página, números de página, fecha, primera página, páginas pares e impares, blanco y negro, borrador, notas, errores, orden de las páginas, centrar en la página, iniciar numeración de páginas, códigos de encabezado y pie de página, calidad de impresión, ppp, puntos por pulgada -->

Ábralo desde la pequeña flecha de la esquina del grupo **Configurar página** de la pestaña **Diseño de página**. Tiene cuatro pestañas. **Aceptar** cambia la hoja en la que se encuentra, como un solo paso que puede deshacer.

## Página

- **Orientación del papel**: vertical o horizontal.
- **Cómo se imprimen las celdas**: un porcentaje del tamaño real, o **Reducir hasta que quepa**, con cuántas páginas de ancho y de alto. «Cualquier número de» permite que esa dirección ocupe tantas páginas como necesite.
- **Papel**: el tamaño del papel.
- **Calidad de impresión**: cuántos puntos por pulgada se piden a la impresora. Más puntos dan una impresión más fina, y algunas impresoras tardan más. Si un libro pide un número que no está en la lista, JeffCrunch lo conserva.
- **Iniciar la numeración de páginas en**: el número que recibe la primera página. Si no se indica, la primera página es la 1, y un libro impreso completo numera sus páginas de forma consecutiva de una hoja a la siguiente.

## Márgenes

Escriba el espacio alrededor de las celdas y la distancia a la que se sitúan el encabezado y el pie de página de los bordes superior e inferior del papel. Los cuadros usan centímetros si su equipo está configurado con una región métrica, y pulgadas en los demás casos. **Horizontalmente** y **Verticalmente** colocan las celdas en el centro de la página.

## Encabezado y pie de página

Un **encabezado** es una línea de texto en la parte superior de cada página, y un **pie de página** es una línea en la parte inferior. Elija uno de la lista, o seleccione **Editar encabezado** o **Editar pie de página** para escribir el suyo. Cada uno tiene una parte izquierda, una central y una derecha, y botones que insertan el número de página, el número de páginas, la fecha, la hora, el nombre del archivo, la carpeta y el nombre de la hoja. Estos datos cambian de una página a otra, así que se calculan al imprimir cada página. Para poner una imagen en una parte, haga clic en esa parte y elija **Imagen…**, y luego seleccione un archivo PNG, JPEG, GIF o BMP. La imagen se reduce a un máximo de 5,1 cm en su lado más largo, y se imprime donde está el código `&G`. **Quitar imagen** la elimina. Cada parte admite una imagen, y la primera página y las páginas pares pueden tener imágenes propias.

También puede definir un encabezado y un pie de página distintos para la primera página y para las páginas pares e impares, y elegir si las letras crecen y se reducen con las celdas y si el encabezado y el pie de página se alinean con los márgenes.

Detrás de los botones hay códigos cortos que el archivo guarda, y puede escribirlos usted mismo: **&P** es el número de página, **&N** el número de páginas, **&D** la fecha, **&T** la hora, **&F** el nombre del archivo, **&A** el nombre de la hoja, y **&&** es un signo de y comercial. **&B**, **&I** y **&U** activan y desactivan la negrita, la cursiva y el subrayado, **&12** fija el tamaño en 12 puntos, y **&"Arial,Bold"** elige una fuente. **&L**, **&C** y **&R** inician las partes izquierda, central y derecha.

## Hoja

- **Celdas que se imprimirán**: el área de impresión, como bloques del tipo **A1:D20, F1:G9**. Déjelo vacío para imprimir todo.
- **Repetir estas filas arriba** y **Repetir estas columnas a la izquierda**: los títulos de impresión, como **1:3** y **A:B**.
- **Imprimir también**: las líneas entre celdas, los números de fila y las letras de columna, solo en blanco y negro (sin colores ni rellenos), y un borrador rápido (sin líneas entre celdas, y sin imágenes, gráficos ni formas).
- **Notas** y **Celdas que muestran un error**: qué hacer con ellas. El ajuste de errores cambia lo que muestra la página: tal como lo muestra la celda, nada, dos guiones o #N/A. Notas: **Omitirlas** no imprime ninguna nota. **Enumerarlas tras la última página** imprime cada nota después de la última página de la hoja, en páginas propias que llevan los siguientes números de página y el encabezado y pie de página de la hoja, como «Celda A2: el autor: el texto de la nota», en el orden de las celdas. **Mostrarlas donde están** se guarda en el archivo para otros programas, pero JeffCrunch todavía no imprime las notas en las celdas.
- **Orden de las páginas**: hacia abajo en la hoja y luego a la derecha, o a la derecha y luego hacia abajo.
