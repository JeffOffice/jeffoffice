# El cuadro de diálogo Configurar página
<!-- keywords: configurar página, configuración de página, cuadro de diálogo, encabezado, pie de página, números de página, numeración, fecha, primera página, pares impares, blanco y negro, borrador, notas, comentarios, errores, orden de las páginas, centrar en la página, número inicial de página, códigos de encabezado y pie de página -->

Ábralo desde la pequeña flecha de la esquina del grupo **Configurar página** de la pestaña **Diseño de página**. Tiene cuatro pestañas. **Aceptar** cambia la hoja en la que se encuentra, como un solo paso que puede deshacer.

## Página

- **Orientación del papel**: vertical u horizontal.
- **Tamaño de impresión de las celdas**: un porcentaje del tamaño real, o **Reducir hasta que quepa**, con cuántas páginas de ancho y de alto. "Cualquier número de" permite que esa dirección ocupe tantas páginas como necesite.
- **Papel**: el tamaño del papel.
- **Empezar la numeración de páginas en**: el número que recibe la primera página. Si no lo indica, la primera página es la 1, y al imprimir un libro completo sus páginas se numeran de forma continua de una hoja a la siguiente.

## Márgenes

Escriba el espacio alrededor de las celdas y la distancia del encabezado y el pie de página a los bordes superior e inferior del papel. Los cuadros usan centímetros si el equipo está configurado con una región métrica, y pulgadas en caso contrario. **Horizontalmente** y **Verticalmente** colocan las celdas en el centro de la página.

## Encabezado y pie de página

Un **encabezado** es una línea de texto en la parte superior de cada página, y un **pie de página** es una en la parte inferior. Elija uno de la lista, o elija **Editar encabezado** o **Editar pie de página** para escribir el suyo. Cada uno tiene una parte izquierda, una central y una derecha, y botones que insertan el número de página, el número total de páginas, la fecha, la hora, el nombre del archivo, la carpeta y el nombre de la hoja. Estos datos cambian de una página a otra, por lo que se calculan al imprimir la página. Para insertar una imagen en una parte, haga clic en esa parte y elija **Imagen…**; después, elija un archivo PNG, JPEG, GIF o BMP. La imagen se reduce a un máximo de unos 5 cm en su lado más largo y se imprime donde aparece el código `&G`. **Quitar imagen** la elimina. Cada parte admite una imagen, y la primera página y las páginas pares pueden tener sus propias imágenes.

También puede definir un encabezado y un pie de página diferentes para la primera página y para las páginas pares e impares, y elegir si las letras crecen y se reducen con las celdas y si el encabezado y el pie de página se alinean con los márgenes.

Detrás de los botones hay códigos cortos que el archivo conserva, y puede escribirlos usted mismo: **&P** es el número de página, **&N** el número total de páginas, **&D** la fecha, **&T** la hora, **&F** el nombre del archivo, **&A** el nombre de la hoja y **&&** es una y comercial. **&B**, **&I** y **&U** activan y desactivan la negrita, la cursiva y el subrayado, **&12** establece el tamaño en 12 puntos y **&"Arial,Bold"** elige una fuente. **&L**, **&C** y **&R** inician las partes izquierda, central y derecha.

## Hoja

- **Celdas que se imprimirán**: el área de impresión, como bloques del tipo **A1:D20, F1:G9**. Déjelo vacío para imprimirlo todo.
- **Repetir estas filas arriba** y **Repetir estas columnas a la izquierda**: los títulos de impresión, como **1:3** y **A:B**.
- **Imprimir también**: las líneas entre celdas, los números de fila y las letras de columna, solo en blanco y negro (sin colores ni rellenos) y como borrador rápido (sin líneas entre celdas y sin imágenes, gráficos ni formas).
- **Notas** y **Celdas que muestran un error**: qué hacer con ellas. La configuración de errores cambia lo que muestra la página: como se muestra en la celda, nada, dos guiones o #N/A. Notas: **Omitirlas** no imprime ninguna nota. **Enumerarlas tras la última página** imprime cada nota después de la última página de la hoja, en páginas propias que llevan los números de página siguientes y el encabezado y pie de página de la hoja, con el formato "Celda A2: el autor: el texto de la nota", en el orden de las celdas. **Mostrarlas donde están** se conserva en el archivo para otros programas, pero JeffCrunch aún no imprime las notas sobre las celdas.
- **Qué página va a continuación**: hacia abajo en la hoja y luego horizontalmente, o horizontalmente y luego hacia abajo.
