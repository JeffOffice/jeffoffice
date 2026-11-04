# Gráficos, minigráficos, imágenes y formas

<!-- keywords: gráfico, diagrama, columna, barra, línea, circular, áreas, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, aspecto, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, gráfico diminuto, marcadores -->

Un gráfico es una imagen de números que permanece vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, seleccione también esas celdas. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Áreas**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte el sentido de las series.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que contiene datos.

Las series se disponen en las columnas cuando la selección tiene al menos tantas filas como columnas, y en las filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. En un gráfico de dispersión, los valores horizontales se toman de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiarle el tamaño.

- Mantenga pulsada la tecla **Alt** mientras arrastra para alinear un borde con el borde de una celda.
- Mantenga pulsada la tecla **Mayús** mientras arrastra una esquina para conservar las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición original.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su tamaño, y para elegir qué hace al insertar, eliminar o cambiar el tamaño de celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o permanecer en su sitio. La elección se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el archivo también conserva esta opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y una ficha **Gráfico** en la cinta de opciones.

- El botón **+** (y **Elementos de gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, e indica dónde van la leyenda y las etiquetas.
- El botón de pincel (y **Colores** y **Estilos** en la ficha) aplica al gráfico uno de los conjuntos de colores y uno de los estilos de JeffCrunch. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregar, editar, quitar y reordenar series, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para ver hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben deprisa y luego se estabilizan, **Polinómica** para una curva con curvaturas, **Potencial**, o **Promedio móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe en el gráfico la fórmula de la línea, y **Mostrar R cuadrado** escribe lo bien que se ajusta la línea (1 es un ajuste perfecto; 0, ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja frente a un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de al lado.
- **Etiquetas de datos** también incluye **Contenido de la etiqueta** (el valor, el nombre de la categoría, el nombre de la serie y el porcentaje, en cualquier combinación), **Separador** (lo que va entre ellos: coma, punto y coma, punto, nueva línea o espacio) y **Formato de número…** (un código de formato, como 0,0, para los números de las etiquetas; déjelo vacío para mostrar los números tal como son). Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Dar formato al eje horizontal…** y **Dar formato al eje vertical…** en la ficha **Gráfico** abren un cuadro para un solo eje. Permite establecer los valores mínimo y máximo, y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores se muestran en orden inverso, en qué punto cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto se giran. Solo un eje numérico tiene límites y unidades.
- **Tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o una columna de números justo al lado de ellos.

1. Seleccione los números. Para un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. **Línea** une los números con una línea. **Columna** dibuja una barra por cada número. **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor que cero, todas del mismo tamaño, lo que resulta adecuado para resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, compruebe el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas en las que se colocan). Elija una celda de ubicación por cada fila de datos, o por cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten el mismo aspecto. Haga clic en una celda con un minigráfico para que aparezca la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que seleccionó. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se leen las celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** asigna al grupo un conjunto de colores. **Color del minigráfico** establece el color de la línea o de las barras, y el **Grosor** (cuán gruesa es la línea, en puntos). **Color de marcador de minigráfico** establece el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, representa los números de derecha a izquierda y establece los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** asigna a todo el grupo una sola escala, para poder compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se situará más a la derecha. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** coloca los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** elimina los minigráficos seleccionados o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, seguirá leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copian y pegan de la misma manera que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la original. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Dar nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Entrar**. Deshacer restaura el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminar

Pulse **Supr** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo descrito arriba se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, de modo que un gráfico creado aquí se abre allí de la misma manera. Un gráfico procedente de otro programa con algo que JeffCrunch no puede conservar al reescribirlo (barras de error, un gráfico tridimensional) se puede mover, cambiar de tamaño y eliminar, pero sus elementos y datos no se modifican aquí.
