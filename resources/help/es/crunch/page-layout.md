# Configurar las páginas
<!-- keywords: tema, temas, colores del tema, fuentes del tema, colores, fuentes, énfasis, fuente de encabezado, fuente del cuerpo, fondo, imagen detrás de las celdas, mosaico, papel tapiz, diseño de página, márgenes, orientación, vertical, horizontal, papel, tamaño, área de impresión, salto de página, saltos, títulos de impresión, repetir filas, ajustar a página, líneas de cuadrícula, encabezados, opciones de hoja, de derecha a izquierda, reflejar -->

La pestaña **Diseño de página** decide cómo se divide una hoja en páginas al imprimirla o al crear un PDF. Cada hoja conserva sus propias opciones, que se guardan en el libro. Cada cambio es un paso que puede deshacer.

## Temas, colores y fuentes

Un libro tiene un **tema**: doce colores y dos fuentes (una para los encabezados y otra para el texto del cuerpo) que sus celdas, estilos de celda, estilos de tabla, formas y gráficos pueden usar en lugar de copiarlos. Al cambiar el tema, todo lo que lo usa cambia con él: una celda rellena con «Énfasis 1, más claro 40 %» pasa al nuevo Énfasis 1, más claro en la misma proporción.

- **Temas** muestra temas predefinidos que establecen colores y fuentes a la vez. **Buscar temas** toma el tema de un archivo de tema, de un libro, de un documento o de una presentación. **Guardar tema actual** guarda el tema de este libro en un archivo, y **Restablecer el tema del libro** devuelve el tema que tenía el libro al abrirlo.
- **Colores** cambia solo los doce colores. **Crear nuevos colores del tema** le permite definir los suyos y darles nombre; a partir de entonces, todas las aplicaciones de JeffOffice los muestran.
- **Fuentes** cambia solo las fuentes de encabezado y de cuerpo. Las celdas que usan las fuentes del tema las siguen; las celdas con una fuente indicada por nombre la conservan.

Cada elección es un paso que puede deshacer. Los botones de color (Color de relleno, Color de fuente, Bordes y Color de pestaña) muestran los colores del tema en una fila, con versiones más claras y más oscuras debajo de cada uno; un color que elija allí sigue siendo un color del tema, y cambia con él. Un color de **Colores estándar** o de **Más colores** se conserva tal como lo eligió.

Un libro nuevo empieza con el tema propio de JeffOffice. Una forma o un gráfico que cree aquí conserva los colores que tenía; uno que llegó en el archivo y usa un color del tema sigue al tema. Un libro abierto desde un archivo binario antiguo o desde un archivo OpenDocument no tiene tema que cambiar, así que estos botones no ofrecen nada en ese caso. Al guardar un libro como archivo OpenDocument, que no tiene temas, los colores se escriben tal como se ven.

## Márgenes, orientación y tamaño

- **Márgenes** establece el espacio en blanco alrededor de las celdas. **Estándar**, **Ancho** y **Estrecho** son conjuntos predefinidos. **Márgenes personalizados** abre el cuadro de diálogo Configurar página, donde puede escribir cada uno y indicar a qué distancia del borde del papel se sitúan el encabezado y el pie de página.
- **Orientación** es **Vertical** (el papel es más alto que ancho) u **Horizontal** (más ancho que alto).
- **Tamaño** es el papel: A4, Carta, Oficio y otros. **Más tamaños de papel** los muestra todos.

## Área de impresión

Por defecto, una hoja imprime todo lo que tenga contenido: valores, fórmulas, y celdas con un borde o un relleno visible, además de cualquier gráfico, imagen o forma. Para imprimir solo una parte:

1. Seleccione las celdas que desee.
2. Elija **Área de impresión > Establecer área de impresión**.

**Agregar selección al área de impresión** añade otro bloque de celdas al área de impresión. Cada bloque se imprime en páginas propias. **Borrar área de impresión** vuelve a imprimir todo. Las filas o columnas que inserte o elimine dentro del área de impresión la modifican junto con las celdas.

## Saltos de página

El lugar donde termina una página y empieza la siguiente se calcula a partir del papel, los márgenes y el tamaño de las filas y columnas: una fila o columna nunca se corta por la mitad. Para terminar una página antes, seleccione la celda situada debajo y a la derecha de donde debe ir el salto, y elija **Saltos > Insertar salto de página**. Si selecciona una fila completa, el salto va encima de ella, y si selecciona una columna completa, va a su izquierda. **Quitar salto de página** elimina el salto situado encima o a la izquierda de la celda seleccionada, y **Restablecer todos los saltos de página** elimina todos los que haya establecido. Los saltos de página que establezca se ignoran mientras una hoja se reduce para ajustarse.

## La vista Saltos de página

**Vista > Saltos de página** (o el botón de la esquina inferior derecha de la ventana) muestra la hoja dividida en las páginas en que se imprime. Las celdas que se imprimen se mantienen igual, el resto aparece sombreado y cada página tiene su número encima de sus celdas. Una línea discontinua es un salto que calculó JeffCrunch, y una línea continua es uno que usted estableció.

- **Mover un salto.** Arrastre una línea hasta otra fila o columna. Se convierte en un salto que usted establece allí, y las páginas siguientes se vuelven a dividir.
- **Quitar un salto.** Arrastre una línea fuera de las celdas impresas, hacia los números de fila o las letras de columna.
- **Cambiar el área de impresión.** Arrastre el borde azul de las celdas impresas. Si no ha establecido un área de impresión, las celdas que se imprimen por sí solas pasan a formar una, con ese borde movido.

Cada arrastre es un paso de Deshacer. Mientras una hoja se reduce para ajustarse, sus saltos se ignoran, así que las líneas no se pueden arrastrar. **Normal** devuelve las celdas tal como estaban. La vista en que quedó cada hoja se guarda con el libro.

## La vista Diseño de página

**Vista > Diseño de página** (o el botón central de la esquina inferior derecha de la ventana) muestra la hoja como las páginas en que se imprime, una debajo de otra, cada una como una hoja de papel con sus márgenes y un espacio entre páginas. Hay reglas a lo largo de la parte superior y por el lado izquierdo. El encabezado y el pie de página se dibujan donde se imprimen, y también las imágenes que contengan.

- **Trabaje en las celdas.** Haga clic en una celda para seleccionarla, arrastre para seleccionar varias y use las teclas de dirección, escribir, F2 y el doble clic como en la vista Normal. Lo que escriba aparece en una franja sobre la celda, en su página.
- **Empiece desde la pestaña Insertar.** **Insertar > Encabezado y pie de página** muestra las páginas (si no se ven) y abre la parte central del encabezado de la primera página, lista para escribir. Haga clic en otra parte del encabezado o del pie de página para escribir allí.
- **Escriba un encabezado o un pie de página.** Mueva el puntero sobre el margen superior o inferior de una página: se iluminan sus partes izquierda, central y derecha. Haga clic en una y escriba. Aparece la pestaña **Encabezado y pie de página**, con botones que insertan el número de página, el número de páginas, la fecha, la hora, la carpeta y el nombre del archivo, el nombre de la hoja y una imagen; sus opciones permiten que la primera página, y las páginas pares e impares, tengan encabezados y pies de página propios, y indican si el encabezado crece con las celdas y se alinea con los márgenes. **Encabezado** y **Pie de página** en esa pestaña son galerías de formatos listos, como **Página 1**, **Página 1 de ?**, el nombre de la hoja, el nombre del archivo y la fecha. Al elegir uno, se reemplaza todo el encabezado o el pie de página de las páginas en que está escribiendo (lo que estuviera escribiendo en él y aún no había terminado no se conserva). **(ninguno)** lo vacía. **Ir a encabezado** e **Ir a pie de página** abren la parte central de ese margen para escribir. Pulse **Ctrl+Intro**, o haga clic en cualquier otro lugar, para terminar; **Esc** lo descarta. Cada encabezado o pie de página terminado es un paso de Deshacer.
- **Ampliar** con el control deslizante, o mantenga pulsado **Ctrl** y gire la rueda del ratón.

Solo se dibujan las páginas visibles, así que una hoja de miles de páginas se abre de inmediato. La vista en que quedó cada hoja se guarda con el libro.

## Títulos de impresión

**Títulos de impresión** repite filas en la parte superior, y columnas a la izquierda, en cada página posterior a aquella en que aparecen, de modo que una fila de encabezado esté en todas las páginas. Escriba las filas (por ejemplo **1:3**) y las columnas (por ejemplo **A:B**) en la pestaña Hoja del cuadro de diálogo Configurar página.

## Ajustar a la página

**Ancho** y **Alto** reducen las celdas hasta que la hoja no sea más ancha, o más alta, que ese número de páginas. **Automático** permite que esa dirección ocupe todas las páginas que necesite. **Escala** imprime a un porcentaje del tamaño real, de 10 a 400, y funciona mientras Ancho y Alto estén ambos en **Automático**. Si reducir para ajustar hiciera las letras muy pequeñas, la pantalla Imprimir lo indica.

## Opciones de hoja

**Mostrar líneas de cuadrícula** y **Mostrar encabezados** solo cambian la pantalla. **Imprimir líneas de cuadrícula** lleva al papel las líneas entre celdas, y **Imprimir encabezados** lleva los números de fila y las letras de columna.

**Hoja de derecha a izquierda** hace que la hoja se lea desde la derecha. La columna A pasa al borde derecho, y las columnas se cuentan hacia la izquierda. Los números de fila también pasan al borde derecho. Use esta opción para idiomas que se escriben de derecha a izquierda, como el árabe y el hebreo. Cambia el aspecto de la hoja en pantalla, y la elección se guarda con la hoja, así que vuelve al abrir el libro. No es un paso que pueda deshacer: haga clic de nuevo para desactivarla.

## Fondo

**Fondo** (en el grupo **Configurar página**) coloca una imagen detrás de las celdas de la hoja. Elija una imagen PNG, JPEG, GIF o BMP. La imagen se repite en mosaico para llenar la hoja y se mueve con las celdas al desplazarse. Una celda que tenga un color de relleno oculta la imagen, así que dé un relleno a las celdas que quiera leer con facilidad. La imagen solo se muestra en pantalla: no se imprime y no entra en un PDF.

**Eliminar fondo** quita la imagen. Ambas acciones son pasos que puede deshacer. La imagen se guarda con la hoja y vuelve al abrir el libro. En una copia .ods se guarda como imagen de fondo de la página, que otros programas podrían imprimir. Un archivo de imagen de más de 8 MB, o con demasiados puntos, se rechaza, y también una imagen que haría que todas las imágenes del libro (fondos, encabezados y pies de página) superaran juntas los 64 MB. Una imagen de más de 2400 puntos por lado se sigue repitiendo en mosaico a su tamaño real.

## Más opciones

El pequeño botón de flecha en la esquina del grupo **Configurar página** abre el cuadro de diálogo Configurar página, que contiene todas estas opciones y más: [El cuadro de diálogo Configurar página](page-setup).
