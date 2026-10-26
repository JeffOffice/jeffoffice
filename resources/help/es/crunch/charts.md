# Gráficos, imágenes y formas
<!-- keywords: gráfico, gráfica, diagrama, columna, barra, línea, circular, tarta, quesito, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, estilos, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie -->

Un gráfico es una imagen de unos números que sigue vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que quiere representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, selecciónelas también. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y como etiquetas cuando contienen palabras (o fechas, en el caso de las etiquetas).
2. En la pestaña **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Líneas**, **Circular**, **Áreas**, **Dispersión** o **Combinado**. La flecha pequeña junto a cada tipo contiene sus variantes, como columnas apiladas o líneas con marcadores.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado a partir de sus celdas. **Intercambiar filas y columnas** cambia la orientación de las series.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas con contenido que la rodea.

Las series se toman por columnas cuando la selección tiene al menos tantas filas como columnas, y por filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma los valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen T1, T2 y T3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de T1 a T3.

## Moverlo y cambiar su tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiar su tamaño.

- Mantenga presionada **Alt** mientras arrastra para alinear un borde con el borde de una celda.
- Mantenga presionada **Mayús** mientras arrastra una esquina para mantener las proporciones.
- Las teclas de dirección lo mueven unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva al siguiente borde de celda.
- **Esc** mientras arrastra lo devuelve a su lugar.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su tamaño y para elegir qué hace cuando inserta, elimina o cambia el tamaño de las celdas: mover y cambiar tamaño con sus celdas, mover con sus celdas pero mantener su tamaño, o no mover. La opción se guarda en el archivo.

## Cambiar lo que muestra un gráfico

Cuando se selecciona un gráfico, aparecen dos botones a su lado y una pestaña **Gráfico** en la cinta de opciones.

- El botón **+** (y **Elementos de gráfico** en la pestaña) activa o desactiva el título del gráfico, los ejes, los títulos de ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, e indica dónde se colocan la leyenda y las etiquetas.
- El botón del pincel (y **Colores** y **Apariencia** en la pestaña) aplica al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** establece varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, modificarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Tipo de gráfico** cambia el tipo de todo el gráfico o de cada serie: columnas con una línea encima, o una serie en un eje vertical secundario.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando contiene espacios.

## Cortar, copiar y pegar un objeto

Las imágenes y formas se copian y pegan igual que los gráficos. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.


Si copia un gráfico y lo pega en el mismo libro, en cualquier hoja, la copia lee las mismas celdas que el original. Si lo pega en otro libro, conserva los números que mostraba pero deja de leer celdas, porque las celdas que leería allí no son aquellas a partir de las que se creó.

## Asignarle un nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba ahí un nombre nuevo y presione **Entrar**. Deshacer recupera el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Presione **Supr** con el objeto seleccionado. Deshacer lo recupera.

## Qué se guarda

Todo lo anterior se guarda en el archivo en el formato que leen otros programas de hojas de cálculo, de modo que un gráfico creado aquí se abre allí de la misma manera. Un gráfico procedente de otro programa con algo que JeffCrunch no puede conservar al reescribir un gráfico (un relleno degradado, una línea de tendencia, un gráfico tridimensional) se puede mover, cambiar de tamaño y eliminar, pero sus elementos y datos no se modifican aquí.
