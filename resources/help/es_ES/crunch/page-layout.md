# Configurar las páginas
<!-- keywords: diseño de página, márgenes, orientación, vertical, horizontal, papel, tamaño, área de impresión, salto de página, saltos, títulos de impresión, títulos, repetir filas, ajustar a la página, ajustar escala, líneas de cuadrícula, encabezados, opciones de la hoja, de derecha a izquierda, reflejar -->

La pestaña **Diseño de página** determina cómo se divide una hoja en páginas al imprimirla o al crear un PDF. Cada hoja conserva sus propias opciones, que se guardan en el libro. Cada cambio es un paso que puede deshacer.

## Márgenes, orientación y tamaño

- **Márgenes** establece el espacio en blanco alrededor de las celdas. **Estándar**, **Ancho** y **Estrecho** son conjuntos predefinidos. **Márgenes personalizados** abre el cuadro de diálogo Configurar página, donde puede escribir cada uno y indicar a qué distancia del borde del papel se sitúan el encabezado y el pie de página.
- **Orientación** es **Vertical** (el papel es más alto que ancho) u **Horizontal** (más ancho que alto).
- **Tamaño** es el papel: A4, Carta, Oficio y otros. **Más tamaños de papel** muestra todos.

## Área de impresión

De forma predeterminada, una hoja imprime todo lo que tenga contenido: valores, fórmulas, celdas con borde o relleno visibles, y cualquier gráfico, imagen o forma. Para imprimir solo una parte:

1. Seleccione las celdas que desea.
2. Elija **Área de impresión > Establecer área de impresión**.

**Agregar selección al área de impresión** añade otro bloque de celdas al área de impresión. Cada bloque se imprime en sus propias páginas. **Borrar área de impresión** vuelve a imprimir todo. Las filas o columnas que inserte o elimine dentro del área de impresión la modifican junto con las celdas.

## Saltos de página

El lugar donde termina una página y empieza la siguiente se calcula según el papel, los márgenes y el tamaño de las filas y columnas: nunca se corta una fila o columna por la mitad. Para terminar una página antes, seleccione la celda situada debajo y a la derecha de donde debe ir el salto, y elija **Saltos > Insertar salto de página**. Si selecciona una fila completa, el salto va encima de ella; si selecciona una columna completa, va a su izquierda. **Quitar salto de página** elimina el salto situado encima o a la izquierda de la celda seleccionada, y **Restablecer todos los saltos de página** elimina todos los que haya establecido. Los saltos de página que establezca se ignoran mientras la hoja se reduce para ajustarse.

## La vista Saltos de página

**Vista > Saltos de página** (o el botón de la esquina inferior derecha de la ventana) muestra la hoja dividida en las páginas en que se imprimirá. Las celdas que se imprimen conservan su aspecto, el resto se sombrea y cada página tiene su número sobre sus celdas. Una línea discontinua es un salto que JeffCrunch ha calculado, y una línea continua es uno que usted ha establecido.

- **Mover un salto.** Arrastre una línea hasta otra fila o columna. Se convierte en un salto que usted ha establecido en ese lugar, y las páginas posteriores se vuelven a dividir.
- **Quitar un salto.** Arrastre una línea fuera de las celdas impresas, hasta los números de fila o las letras de columna.
- **Cambiar el área de impresión.** Arrastre el borde azul de las celdas impresas. Si no ha establecido un área de impresión, las celdas que se imprimen por sí solas pasan a formar una, con ese borde desplazado.

Cada arrastre es un paso de Deshacer. Mientras la hoja se reduce para ajustarse, sus saltos se ignoran, por lo que las líneas no se pueden arrastrar. **Normal** devuelve las celdas a su aspecto original. La vista en la que se dejó cada hoja se guarda con el libro.

## La vista Diseño de página

**Vista > Diseño de página** (o el botón central de la esquina inferior derecha de la ventana) muestra la hoja como las páginas en que se imprimirá, una debajo de otra, cada una como una hoja de papel con sus márgenes y un espacio entre páginas. Las reglas se extienden por la parte superior y por la izquierda. El encabezado y el pie de página se dibujan donde se imprimen, al igual que las imágenes que contengan.

- **Trabaje en las celdas.** Haga clic en una celda para seleccionarla, arrastre para seleccionar varias, y use las teclas de flecha, la escritura, F2 y el doble clic como en la vista Normal. Lo que escriba aparece en una franja sobre la celda, en su página.
- **Empiece desde la pestaña Insertar.** **Insertar > Encabezado y pie de página** muestra las páginas (si no se están mostrando) y abre la parte central del encabezado de la primera página, lista para escribir. Haga clic en otra parte del encabezado o del pie de página para escribir allí.
- **Escriba un encabezado o pie de página.** Mueva el puntero sobre el margen superior o inferior de una página: sus partes izquierda, central y derecha se iluminan. Haga clic en una y escriba. Aparece la pestaña **Encabezado y pie de página**, con botones que insertan el número de página, el número total de páginas, la fecha, la hora, la carpeta y el nombre del fichero, el nombre de la hoja y una imagen. Sus opciones permiten una primera página distinta, páginas pares e impares con encabezados y pies propios, e indicar si el encabezado crece con las celdas y se alinea con los márgenes. **Encabezado** y **Pie de página** de esa pestaña son galerías de formatos predefinidos, como **Página 1**, **Página 1 de ?**, el nombre de la hoja, el nombre del fichero y la fecha. Al elegir uno, se reemplaza todo el encabezado o pie de las páginas en las que está escribiendo (no se conservan las palabras que estuviera escribiendo en él). **(ninguno)** lo vacía. **Ir al encabezado** e **Ir al pie de página** abren la parte central de ese margen para escribir. Pulse **Ctrl+Entrar**, o haga clic en cualquier otro lugar, para terminar; **Esc** lo cancela. Cada encabezado o pie terminado es un paso de Deshacer.
- **Ampliar o reducir** con el control deslizante, o mantenga pulsado **Ctrl** y gire la rueda del ratón.

Solo se dibujan las páginas visibles, así que una hoja de miles de páginas se abre de inmediato. La vista en la que se dejó cada hoja se guarda con el libro.

## Títulos de impresión

**Títulos de impresión** repite filas en la parte superior y columnas a la izquierda en todas las páginas posteriores a la de su aparición, de modo que una fila de encabezados esté en cada página. Escriba las filas (por ejemplo, **1:3**) y las columnas (por ejemplo, **A:B**) en la pestaña Hoja del cuadro de diálogo Configurar página.

## Ajustar a la página

**Ancho** y **Alto** reducen las celdas hasta que la hoja no sea más ancha, o más alta, que ese número de páginas. **Automático** permite que esa dirección ocupe todas las páginas que necesite. **Escala** imprime a un porcentaje del tamaño real, de 10 a 400, y funciona mientras Ancho y Alto estén ambos en **Automático**. Si reducir para ajustar hiciera que las letras fueran muy pequeñas, la pantalla de Imprimir lo indica.

## Opciones de la hoja

**Mostrar líneas de cuadrícula** y **Mostrar encabezados** solo cambian la pantalla. **Imprimir líneas de cuadrícula** imprime en el papel las líneas entre celdas, e **Imprimir encabezados** imprime en el papel los números de fila y las letras de columna.

**Hoja de derecha a izquierda** hace que la hoja se lea desde la derecha. La columna A pasa al borde derecho, y las columnas se cuentan hacia la izquierda. Los números de fila también pasan al borde derecho. Úselo para idiomas que se escriben de derecha a izquierda, como el árabe y el hebreo. Cambia cómo se ve la hoja en pantalla, y la opción se guarda con la hoja, de modo que se conserva al abrir el libro. No es un paso que pueda deshacer: haga clic de nuevo para desactivarla.

## Más opciones

La pequeña flecha de la esquina del grupo **Configurar página** abre el cuadro de diálogo Configurar página, que incluye todas estas opciones y otras más: [El cuadro de diálogo Configurar página](page-setup).
