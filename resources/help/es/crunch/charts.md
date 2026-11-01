# Gráficos, imágenes y formas
<!-- keywords: gráfico, diagrama, columna, barra, línea, circular, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, apariencia, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie -->

Un gráfico es una imagen de números que permanece vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila contiene nombres y la primera columna contiene etiquetas, selecciónelas también. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y etiquetas cuando contienen palabras (o fechas, en el caso de las etiquetas).
2. En la pestaña **Insertar**, en el grupo **Gráficos**, elija un tipo: **Columna**, **Barra**, **Líneas**, **Circular**, **Áreas**, **Dispersión** o **Combinado**. La pequeña flecha situada junto a cada tipo contiene sus variantes, como columnas apiladas, líneas con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver antes todos los tipos, elija **Más gráficos…**. La lista de la izquierda muestra cada tipo dibujado a partir de sus celdas. **Intercambiar filas y columnas** orienta las series en el otro sentido.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas con contenido que la rodea.

Las series se organizan por columnas cuando la selección tiene al menos tantas filas como columnas, y por filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen T1, T2 y T3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de T1 a T3.

## Moverlo y cambiar su tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiar su tamaño.

- Mantenga presionada la tecla **Alt** mientras arrastra para alinear un borde con el borde de una celda.
- Mantenga presionada la tecla **Mayús** mientras arrastra una esquina para mantener las proporciones.
- Las teclas de dirección lo mueven unos pocos píxeles cada vez. **Ctrl** con una tecla de dirección lo mueve un píxel, **Mayús** con una tecla de dirección lo agranda o lo reduce, y **Alt** con una tecla de dirección lo lleva hasta el siguiente borde de una celda.
- Si presiona **Esc** mientras arrastra, vuelve a su posición.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su tamaño y para elegir qué hace al insertar, eliminar o cambiar el tamaño de las celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero mantener su tamaño, o no moverse. Esta opción se guarda en el archivo.

## Cambiar lo que muestra un gráfico

Cuando se selecciona un gráfico, aparecen dos botones junto a él y una pestaña **Gráfico** en la cinta de opciones.

- El botón **+** (y **Elementos de gráfico** en la pestaña) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, e indica dónde se colocan la leyenda y las etiquetas.
- El botón del pincel (y **Colores** y **Apariencia** en la pestaña) aplica al gráfico uno de los conjuntos de colores de JeffCrunch y una de sus apariencias. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** enumera las series del gráfico. Puede agregarlas, modificarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar las filas y las columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Tipo de gráfico** cambia el tipo de todo el gráfico o de cada serie: columnas con una línea superpuesta, o una serie en un eje vertical secundario.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples si contiene espacios.

## Cortar, copiar y pegar un objeto

Las imágenes y las formas se copian y se pegan del mismo modo que los gráficos. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.


Si copia un gráfico y lo pega en el mismo libro, en cualquier hoja, la copia lee las mismas celdas que el original. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son aquellas a partir de las que se creó.

## Asignarle un nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo en él y presione **Entrar**. Deshacer restaura el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Presione **Supr** con el objeto seleccionado. Deshacer lo recupera.

## Qué se guarda

Todo lo anterior se guarda en el archivo en el formato que leen otros programas de hojas de cálculo, de modo que un gráfico creado aquí se abre allí de la misma manera. Un gráfico procedente de otro programa con algo que JeffCrunch no puede conservar al reescribir un gráfico (un relleno degradado, una línea de tendencia, un gráfico tridimensional) se puede mover, cambiar de tamaño y eliminar, pero sus elementos y sus datos no se modifican aquí.
