# Configurar las páginas
<!-- keywords: fondo, imagen detrás de las celdas, mosaico, papel tapiz, diseño de página, márgenes, orientación, vertical, horizontal, papel, tamaño, área de impresión, salto de página, saltos, títulos de impresión, repetir filas, ajustar a la página, ajustar área de impresión, líneas de cuadrícula, encabezados, opciones de la hoja, de derecha a izquierda, reflejar -->

La ficha **Diseño de página** decide cómo se divide una hoja en páginas al imprimirla o al crear un PDF. Cada hoja conserva sus propias opciones, y se guardan en el libro. Cada cambio es un paso que puede deshacer.

## Márgenes, orientación y tamaño

- **Márgenes** establece el espacio en blanco alrededor de las celdas. **Normales**, **Anchos** y **Estrechos** son conjuntos predefinidos. **Márgenes personalizados** abre el cuadro de diálogo Configurar página, donde puede escribir cada uno e indicar a qué distancia del borde del papel se sitúan el encabezado y el pie de página.
- **Orientación** es **Vertical** (el papel es más alto que ancho) u **Horizontal** (más ancho que alto).
- **Tamaño** es el papel: Carta, Oficio, A4 y otros. **Más tamaños de papel** los muestra todos.

## Área de impresión

De forma predeterminada, una hoja imprime todo lo que tiene contenido: valores, fórmulas, celdas con borde o relleno visible, y cualquier gráfico, imagen o forma. Para imprimir solo una parte:

1. Seleccione las celdas que desea.
2. Elija **Área de impresión > Establecer área de impresión**.

**Agregar selección al área de impresión** añade otro bloque de celdas al área de impresión. Cada bloque se imprime en páginas propias. **Borrar área de impresión** vuelve a imprimir todo. Las filas o columnas que inserte o elimine dentro del área de impresión la cambian junto con las celdas.

## Saltos de página

El lugar donde termina una página y empieza la siguiente se calcula según el papel, los márgenes y el tamaño de las filas y columnas: nunca se corta una fila o columna por la mitad. Para terminar una página antes, seleccione la celda situada debajo y a la derecha de donde debe ir el salto, y elija **Saltos > Insertar salto de página**. Si selecciona una fila completa, el salto va encima de ella; si selecciona una columna completa, va a su izquierda. **Quitar salto de página** elimina el salto situado encima o a la izquierda de la celda seleccionada, y **Restablecer todos los saltos de página** elimina todos los que haya definido. Los saltos que defina se ignoran mientras una hoja se reduce para ajustarse.

## La vista Saltos de página

**Vista > Saltos de página** (o el botón de la esquina inferior derecha de la ventana) muestra la hoja dividida en las páginas en que se imprime. Las celdas que se imprimen se mantienen tal como están, el resto aparece sombreado, y cada página tiene su número encima de sus celdas. Una línea discontinua es un salto que calculó JeffCrunch, y una línea continua es uno que usted definió.

- **Mover un salto.** Arrastre una línea hasta otra fila o columna. Se convierte en un salto definido por usted en ese lugar, y las páginas posteriores se vuelven a dividir.
- **Quitar un salto.** Arrastre una línea fuera de las celdas impresas, hacia los números de fila o las letras de columna.
- **Cambiar el área de impresión.** Arrastre el borde azul de las celdas impresas. Si no ha establecido un área de impresión, las celdas que se imprimen por sí solas pasan a formar una, con ese borde desplazado.

Cada arrastre es un paso de Deshacer. Mientras una hoja se reduce para ajustarse, sus saltos se ignoran, así que las líneas no se pueden arrastrar. **Normal** devuelve las celdas tal como estaban. La vista en que se dejó cada hoja se guarda con el libro.

## La vista Diseño de página

**Vista > Diseño de página** (o el botón central de la esquina inferior derecha de la ventana) muestra la hoja como las páginas en que se imprime, una debajo de otra, cada una como una hoja de papel con sus márgenes y un espacio entre páginas. Las reglas aparecen arriba y a la izquierda. El encabezado y el pie de página se dibujan donde se imprimen, y también las imágenes que contienen.

- **Trabajar en las celdas.** Haga clic en una celda para seleccionarla, arrastre para seleccionar varias, y use las teclas de flecha, la escritura, F2 y el doble clic como en la vista Normal. Lo que escriba aparece en una franja sobre la celda, en su página.
- **Empezar desde la ficha Insertar.** **Insertar > Encabezado y pie de página** muestra las páginas (si no se están mostrando) y abre la parte central del encabezado de la primera página, listo para escribir. Haga clic en otra parte del encabezado o del pie de página para escribir allí.
- **Escribir un encabezado o pie de página.** Mueva el puntero sobre el margen superior o inferior de una página: sus partes izquierda, central y derecha se iluminan. Haga clic en una y escriba. Aparece la ficha **Encabezado y pie de página**, con botones que insertan el número de página, el número de páginas, la fecha, la hora, la carpeta y el nombre del archivo, el nombre de la hoja y una imagen. Sus opciones permiten usar una primera página, páginas pares e impares con encabezados y pies propios, e indicar si el encabezado crece con las celdas y se alinea con los márgenes. **Encabezado** y **Pie de página** en esa ficha son galerías de formatos listos, como **Página 1**, **Página 1 de ?**, el nombre de la hoja, el nombre del archivo y la fecha. Al elegir uno se reemplaza todo el encabezado o pie de página de las páginas en que escribe (las palabras que aún estaba escribiendo en él no se conservan). **(ninguno)** lo vacía. **Ir a encabezado** e **Ir a pie de página** abren la parte central de ese margen para escribir. Pulse **Ctrl+Intro**, o haga clic en cualquier otro lugar, para terminar; **Esc** lo cancela. Cada encabezado o pie de página terminado es un paso de Deshacer.
- **Ampliar o reducir** con el control deslizante, o mantenga pulsada **Ctrl** y gire la rueda del ratón.

Solo se dibujan las páginas visibles, así que una hoja de miles de páginas se abre de inmediato. La vista en que se dejó cada hoja se guarda con el libro.

## Títulos de impresión

**Títulos de impresión** repite filas arriba, y columnas a la izquierda, en cada página posterior a la de su ubicación, de modo que una fila de encabezados aparezca en todas las páginas. Escriba las filas (por ejemplo **1:3**) y las columnas (por ejemplo **A:B**) en la ficha Hoja del cuadro de diálogo Configurar página.

## Ajustar a la página

**Ancho** y **Alto** reducen las celdas hasta que la hoja no sea más ancha, o más alta, que ese número de páginas. **Automático** permite que esa dirección use todas las páginas que necesite. **Escala** imprime a un porcentaje del tamaño real, de 10 a 400, y funciona mientras Ancho y Alto estén ambos en **Automático**. Si reducir para ajustar hiciera las letras muy pequeñas, la pantalla de Imprimir lo indica.

## Opciones de la hoja

**Mostrar líneas de cuadrícula** y **Mostrar encabezados** cambian solo la pantalla. **Imprimir líneas de cuadrícula** pone en el papel las líneas entre celdas, y **Imprimir encabezados** pone en el papel los números de fila y las letras de columna.

**Hoja de derecha a izquierda** hace que la hoja se lea desde la derecha. La columna A pasa al borde derecho, y las columnas se cuentan hacia la izquierda. Los números de fila también pasan al borde derecho. Úselo para idiomas que se escriben de derecha a izquierda, como el árabe y el hebreo. Cambia el aspecto de la hoja en pantalla, y la opción se guarda con la hoja, de modo que vuelve a aparecer al abrir el libro. No es un paso que pueda deshacer: vuelva a hacer clic para desactivarlo.

## Fondo

**Fondo** (en el grupo **Configurar página**) coloca una imagen detrás de las celdas de la hoja. Elija una imagen PNG, JPEG, GIF o BMP. La imagen se repite como mosaico para llenar la hoja, y se mueve con las celdas al desplazarse. Una celda que tiene color de relleno oculta la imagen, así que dé un relleno a las celdas que desee leer con facilidad. La imagen solo se ve en pantalla: no se imprime ni entra en un PDF.

**Eliminar fondo** quita la imagen. Ambas acciones son pasos que puede deshacer. La imagen se guarda con la hoja y vuelve a aparecer al abrir el libro. En una copia .ods se guarda como imagen de fondo de la página, que otros programas pueden imprimir. Un archivo de imagen de más de 8 MB, o con demasiados puntos, se rechaza, y también una imagen que haga que todas las imágenes del libro (fondos, encabezados y pies de página) superen juntas los 64 MB. Una imagen de más de 2400 puntos por lado se sigue repitiendo en mosaico con su tamaño real.

## Más opciones

La pequeña flecha de la esquina del grupo **Configurar página** abre el cuadro de diálogo Configurar página, que incluye todas estas opciones y más: [El cuadro de diálogo Configurar página](page-setup).
