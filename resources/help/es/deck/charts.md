# Gráficos

<!-- keywords: gráfico, gráficos, diagrama, gráfico de columnas, gráfico de barras, gráfico de líneas, gráfico circular, gráfico de anillo, área, dispersión, insertar gráfico, datos del gráfico, editar datos, título del gráfico, leyenda, etiquetas de datos, tipo de gráfico, formato de gráfico, ejes, título del eje, líneas de cuadrícula, estilo de gráfico, elementos del gráfico, diseños, colores, aspectos, datos del gráfico, intercambiar filas y columnas, tipo de gráfico, dar formato al eje, barras de error, línea de tendencia, filtros de gráfico, elementos del gráfico, gráfico de cotizaciones, bursátil -->

Un **gráfico** convierte números en una imagen, como columnas, una línea o porciones de un círculo. Este tema explica cómo insertar un gráfico en una diapositiva, cómo escribir sus números y cómo cambiar su aspecto.

## Insertar un gráfico en una diapositiva

1. Vaya a la diapositiva donde debe ir el gráfico.
2. Haga clic en **Insertar > Gráfico** y elija un tipo: columna, columna apilada, columna 100 % apilada, barra, barra apilada, línea, línea con marcadores, circular, anillo, área, área apilada, dispersión, radar, radar con marcadores, radar relleno, máximo-mínimo-cierre, apertura-máximo-mínimo-cierre, volumen-máximo-mínimo-cierre o volumen-apertura-máximo-mínimo-cierre.

Un gráfico **bursátil** muestra precios: una línea por cada categoría, desde su precio más alto hasta el más bajo, con un pequeño punto para el cierre o, si hay apertura, una barra entre la apertura y el cierre (clara si sube y oscura si baja). El gráfico se crea con sus columnas tal como están, de izquierda a derecha, y nada comprueba que los números correspondan a su lugar, así que colóquelos en este orden: máximo, mínimo y cierre; apertura, máximo, mínimo y cierre; volumen, máximo, mínimo y cierre; o volumen, apertura, máximo, mínimo y cierre. Si las columnas están en otro orden, el gráfico se dibuja igualmente, pero las marcas significan otra cosa (el punto de cierre va en la tercera columna de precios, y la caja va de la primera columna de precios a la última). Con volumen, el volumen se lee en el eje izquierdo como columnas y los precios se leen en un segundo eje, a la derecha. Si el número de columnas no corresponde al tipo, JeffDeck indica cuántas necesita y no crea el gráfico.

Un gráfico **apilado** apila las series unas sobre otras. Un gráfico **100 % apilado** hace que cada pila tenga la misma altura, para que vea proporciones. Un gráfico de **dispersión** coloca cada punto donde indican sus dos números.

El gráfico aparece con números de ejemplo, en el centro de la diapositiva, y se abre la ficha **Formato de gráfico**. Un gráfico es un objeto, como una imagen o una forma: arrástrelo para moverlo, arrastre un controlador para cambiar su tamaño (el gráfico se vuelve a dibujar para ajustarse, de modo que sus textos conservan la forma correcta) y gírelo con **Girar** en el grupo **Dibujo** de la ficha **Inicio**.

## Escribir sus propios números

Seleccione el gráfico y haga clic en **Formato de gráfico > Editar datos**. Se abre una tabla.

- La primera columna contiene las palabras bajo los puntos (las **categorías**), como los meses.
- La primera fila contiene el nombre de cada **serie**, que es un conjunto de números, como un producto.
- El resto de las celdas contienen los números.

Haga clic en **Agregar fila** o **Agregar columna** para añadir más, o en **Quitar última fila** o **Quitar última columna** para quitar. Haga clic en **Aceptar**. Si una celda no es un número, el gráfico no cambia y la barra de estado indica en qué fila está el problema. Un gráfico puede tener hasta 200 categorías y 24 series.

En un gráfico de dispersión, la primera columna contiene los números que colocan los puntos de izquierda a derecha.

## Gráficos guardados por otro programa

Algunos gráficos guardados por otros programas conservan sus números en un pequeño **libro de cálculo** (un archivo de hoja de cálculo) almacenado dentro de su archivo. Seleccione un gráfico de este tipo y haga clic en **Formato de gráfico > Editar datos**. La tabla muestra las celdas de ese libro que lee el gráfico. Cambie un número, un nombre bajo un punto o el nombre de una serie, y haga clic en **Aceptar**. El cambio pasa al libro y al gráfico a la vez, como un único paso que puede deshacer, de modo que otros programas también muestran los nuevos números.

No puede agregar ni quitar filas o columnas de un gráfico de este tipo, y **Datos del gráfico** e **Intercambiar filas y columnas** aparecen atenuados, porque el gráfico está vinculado a esas celdas. Lo mismo ocurre cuando el gráfico lee celdas de otro archivo, en un bloque que no es una sola columna o una sola fila, o en celdas que contienen una fórmula, una fecha o palabras donde debería haber un número. El comando aparece atenuado y su información indica el motivo.

## Cambiar su aspecto

Estos comandos están en la ficha **Formato de gráfico**. Cada uno es un solo paso que puede deshacer, y el gráfico conserva sus colores y el resto de su aspecto.

- **Cambiar tipo de gráfico** convierte el gráfico en otro tipo con los mismos números. Para cambiar a un tipo bursátil, las series del gráfico deben estar en el orden que pide ese tipo; si no lo están, se indica el orden necesario y el gráfico se deja como está.
- **Título del gráfico** muestra u oculta el título. **Editar título del gráfico** cambia sus palabras.
- **Leyenda** es la clave que indica qué color corresponde a cada serie. Colóquela a la derecha, arriba, a la izquierda o abajo, o quítela.
- **Ejes** muestra u oculta la línea de palabras bajo el gráfico (las **categorías**) y la línea de números al lado (los **valores**), y asigna un título a cada una. Un gráfico circular no tiene ejes.
- **Líneas de cuadrícula** dibuja líneas tenues a lo largo del gráfico para ayudar a seguir los números. Las líneas **principales** se sitúan en los números principales del lateral, y las **secundarias** se sitúan entre ellas.
- **Estilos de gráfico** elige uno de ocho estilos generales.
- **Etiquetas de datos** escribe cada número sobre su columna, punto o porción.

## La ficha Gráfico

Seleccione un gráfico y se abre la ficha **Gráfico** junto a la ficha **Formato de gráfico**. Todo lo que hay en ella cambia el gráfico en un paso que puede deshacer.

- **Elementos del gráfico** agrega, quita o coloca una parte del gráfico: **Ejes**, **Títulos de los ejes**, **Título del gráfico**, **Etiquetas de datos** (qué contienen, dónde van y el separador entre ellas), **Líneas de cuadrícula**, **Leyenda**, **Barras de error** (barras cortas que muestran cuánto puede variar un número), **Líneas** (líneas de caída, líneas de máximos y mínimos, y barras de subida y bajada), **Tabla de datos** (una cuadrícula con los números bajo el gráfico, con una columna por categoría; su lista activa y desactiva la tabla y define las líneas entre filas, las líneas entre columnas, el contorno y las claves de la leyenda, que muestran junto a cada nombre el color de su serie; se ofrece para gráficos de columnas, de líneas y de áreas) y **Línea de tendencia** (una línea que muestra hacia dónde se dirigen los números, con su ecuación si lo desea).
- **Diseños** establece varias partes a la vez, como el título, la leyenda, las etiquetas y las líneas de cuadrícula.
- **Colores** asigna a las series un nuevo conjunto de colores. **Aspectos** da al gráfico completo un aspecto, como sencillo, limpio, oscuro o suave.
- **Intercambiar filas y columnas** invierte los números: las series pasan a ser las palabras bajo los puntos, y esas palabras pasan a ser las series.
- **Datos del gráfico** muestra la lista de series. Marque las que desea dibujar, cambie sus nombres y muévalas hacia arriba o hacia abajo.
- **Editar datos** abre la tabla de números.
- **Tipo de gráfico** muestra una imagen de cada tipo. Elija un tipo para todo el gráfico, o un tipo para cada serie, como columnas con una línea encima.
- **Dar formato a la parte del gráfico** establece el relleno y la línea de la página del gráfico, del área de trazado (el recuadro en el que se sitúan las marcas) o de una serie. En una línea, también define si es suave y qué marca aparece en cada punto.
- **Selección actual** muestra las partes del gráfico: el área del gráfico, el área de trazado y cada serie. Elija una, y **Dar formato a la selección** abre la ventana de formato en esa parte. **Restablecer para coincidir con el estilo** quita lo que se estableció en la parte elegida y devuelve lo que le asigna el estilo del gráfico. En el área del gráfico, eso significa su relleno, su línea y el color de texto que establece un aspecto (el texto claro del aspecto oscuro vuelve al color de texto del tema). En el área de trazado, son su relleno y su línea. En una serie, son su color (el que le asigna la opción Colores o el estilo del gráfico), su contorno, sus marcas tal como las tiene ese tipo de gráfico y los colores de sus puntos; una línea suave se mantiene si el tipo de gráfico es suave. No cambia los títulos, la leyenda, las etiquetas, los ejes, las líneas de cuadrícula ni los datos, y no hace nada en una parte que ya coincide con el estilo. Es un paso de Deshacer.
- **Dar formato al eje horizontal** y **Dar formato al eje vertical** establecen los números menor y mayor, el tamaño de cada paso, el orden, el punto en que el otro eje lo cruza, el aspecto de los números y las marcas de graduación.

## Los botones junto a un gráfico

Tres pequeños botones aparecen a la derecha de un gráfico seleccionado.

- **Elementos del gráfico** (el signo más) abre el mismo menú que **Elementos del gráfico** en la ficha **Gráfico**.
- **Colores y estilos del gráfico** (el pincel) muestra los conjuntos de colores y los aspectos.
- **Filtros de gráfico** (el embudo) muestra las series. Desmarque una serie para dejarla fuera del gráfico. El archivo sigue conteniéndola, así que vuelve a aparecer si la marca de nuevo. Un gráfico siempre mantiene al menos una serie visible.

## Gráficos en otras presentaciones

Un gráfico de un archivo **.pptx** se abre como un gráfico que puede modificar, cuando contiene sus números dentro del gráfico. Si un gráfico lee sus números de una hoja de cálculo guardada con él, puede modificar todo excepto los números. Un gráfico con partes que JeffDeck no modifica, como efectos especiales, se conserva tal como está.

## Guardar, imprimir y PDF

- Un archivo **.pptx** conserva el gráfico como gráfico, así que se abre como gráfico aquí y en otros programas.
- Un archivo **.odp** no tiene un lugar para un gráfico que se abra como gráfico aquí. JeffDeck guarda el gráfico como imagen, y el mensaje que aparece tras el guardado lo indica.
- El **PDF** y la impresión dibujan el gráfico como líneas, formas y texto real, no como imagen, de modo que se mantiene nítido a cualquier tamaño.
