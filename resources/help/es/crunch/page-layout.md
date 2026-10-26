# Configurar las páginas
<!-- keywords: diseño de página, márgenes, orientación, vertical, horizontal, papel, tamaño, área de impresión, salto de página, saltos, imprimir títulos, títulos, repetir filas, ajustar área de impresión, ajustar a la página, líneas de cuadrícula, encabezados, opciones de la hoja -->

La pestaña **Diseño de página** determina cómo se divide una hoja en páginas al imprimirla o convertirla en PDF. Cada hoja conserva su propia configuración, que se guarda en el libro. Cada cambio es un paso que puede deshacer.

## Márgenes, orientación y tamaño

- **Márgenes** establece el espacio en blanco alrededor de las celdas. **Normal**, **Ancho** y **Estrecho** son conjuntos predefinidos. **Márgenes personalizados** abre el cuadro de diálogo Configurar página, donde puede escribir cada uno e indicar a qué distancia del borde del papel se sitúan el encabezado y el pie de página.
- **Orientación** es **Vertical** (el papel es más alto que ancho) u **Horizontal** (más ancho que alto).
- **Tamaño** es el papel: A4, Carta, Legal y otros. **Más tamaños de papel** los muestra todos.

## Área de impresión

De forma predeterminada, una hoja imprime todo lo que tenga contenido: valores, fórmulas, celdas con un borde o relleno visible, y cualquier gráfico, imagen o forma. Para imprimir solo una parte:

1. Seleccione las celdas que desee.
2. Elija **Área de impresión > Establecer área de impresión**.

**Agregar selección al área de impresión** añade un segundo bloque de celdas al área de impresión. Cada bloque se imprime en sus propias páginas. **Borrar área de impresión** vuelve a imprimir todo. Las filas o columnas que inserte o elimine dentro del área de impresión la modifican junto con las celdas.

## Saltos de página

El lugar donde termina una página y empieza la siguiente se calcula a partir del papel, los márgenes y el tamaño de las filas y columnas: una fila o columna nunca se corta por la mitad. Para terminar una página antes, seleccione la celda situada debajo y a la derecha de donde debe ir el salto y elija **Saltos > Insertar salto de página**. Si hay una fila entera seleccionada, el salto se coloca encima de ella; si hay una columna entera seleccionada, se coloca a su izquierda. **Quitar salto de página** quita el salto situado encima o a la izquierda de la celda seleccionada, y **Restablecer todos los saltos de página** quita todos los saltos que haya establecido. Los saltos de página establecidos se omiten mientras la hoja se reduce para ajustarse.

## La vista Saltos de página

**Vista > Saltos de página** (o el botón de la parte inferior derecha de la ventana) muestra la hoja dividida en las páginas en que se imprime. Las celdas que se imprimen se muestran como están, el resto aparece sombreado y cada página muestra su número sobre sus celdas. Una línea discontinua es un salto calculado por JeffCrunch, y una línea continua es un salto que ha establecido usted.

- **Mover un salto.** Arrastre una línea a otra fila o columna. Se convierte en un salto establecido por usted en esa posición, y las páginas siguientes se vuelven a dividir.
- **Quitar un salto.** Arrastre una línea fuera de las celdas impresas, sobre los números de fila o las letras de columna.
- **Cambiar el área de impresión.** Arrastre el borde azul de las celdas impresas. Si no ha establecido un área de impresión, las celdas que se imprimen por sí solas pasan a ser una, con ese borde desplazado.

Cada arrastre es un paso de Deshacer. Mientras la hoja se reduce para ajustarse, sus saltos se omiten, por lo que las líneas no se pueden arrastrar. **Normal** vuelve a mostrar las celdas como estaban. La vista en que quedó cada hoja se guarda con el libro.

## La vista Diseño de página

**Vista > Diseño de página** (o el botón central de la parte inferior derecha de la ventana) muestra la hoja como las páginas en que se imprime, una debajo de otra, cada una como una hoja de papel con sus márgenes y un espacio entre páginas. Hay reglas en la parte superior y en el lado izquierdo. El encabezado y el pie de página se muestran donde se imprimen, igual que las imágenes que contienen.

- **Trabajar en las celdas.** Haga clic en una celda para elegirla, arrastre para elegir varias y use las teclas de dirección, la escritura, F2 y el doble clic como en la Vista Normal. Lo que escribe aparece en una franja sobre la celda en su página.
- **Escribir un encabezado o pie de página.** Mueva el puntero sobre el margen superior o inferior de una página: sus partes izquierda, central y derecha se resaltan. Haga clic en una y escriba. Aparece la pestaña **Encabezado y pie de página**, con botones que insertan el número de página, el número total de páginas, la fecha, la hora, la carpeta y el nombre del archivo, el nombre de la hoja y una imagen; sus opciones permiten que la primera página y las páginas pares e impares tengan encabezados y pies de página propios, e indicar si el encabezado se ajusta al documento y se alinea con los márgenes. Presione **Ctrl+Entrar**, o haga clic en cualquier otro lugar, para terminar; **Esc** lo descarta. Cada encabezado o pie de página terminado es un paso de Deshacer.
- **Zoom** con el control deslizante, o mantenga presionada **Ctrl** y gire la rueda del mouse.

Solo se dibujan las páginas visibles, por lo que una hoja de miles de páginas se abre al instante. La vista en que quedó cada hoja se guarda con el libro.

## Imprimir títulos

**Imprimir títulos** repite filas en la parte superior y columnas a la izquierda de cada página posterior a aquella en la que se encuentran, de modo que una fila de encabezado aparezca en todas las páginas. Escriba las filas (por ejemplo, **1:3**) y las columnas (por ejemplo, **A:B**) en la pestaña Hoja del cuadro de diálogo Configurar página.

## Ajustar área de impresión

**Ancho** y **Alto** reducen las celdas hasta que la hoja no ocupe más de ese número de páginas de ancho o de alto. **Automático** permite que esa dirección ocupe tantas páginas como necesite. **Escala** imprime a un porcentaje del tamaño real, de 10 a 400, y funciona cuando Ancho y Alto están ambos en **Automático**. Si la reducción para ajustar dejara las letras muy pequeñas, la pantalla Imprimir lo indica.

## Opciones de la hoja

**Mostrar líneas de cuadrícula** y **Mostrar encabezados** solo afectan a la pantalla. **Imprimir líneas de cuadrícula** imprime las líneas entre celdas, e **Imprimir encabezados** imprime los números de fila y las letras de columna.

## Más opciones

La pequeña flecha de la esquina del grupo **Configurar página** abre el cuadro de diálogo Configurar página, que incluye todas estas opciones y otras más: [El cuadro de diálogo Configurar página](page-setup).
