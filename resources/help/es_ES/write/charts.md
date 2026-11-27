# Gráficos
<!-- keywords: gráfico, gráficos, diagrama, gráfico de columnas, gráfico de barras, gráfico de líneas, gráfico circular, gráfico de anillos, anillo, área, dispersión, insertar gráfico, datos del gráfico, editar datos, título del gráfico, leyenda, etiquetas de datos, tipo de gráfico, formato de gráfico, ejes, título del eje, líneas de cuadrícula, estilo de gráfico, elementos del gráfico, diseños, colores, estilos, datos del gráfico, intercambiar filas y columnas, tipo de gráfico, formato del eje, barras de error, línea de tendencia, filtros de gráfico, elementos del gráfico -->

Un **gráfico** convierte los números en una imagen, como columnas, una línea o porciones de un círculo. Este tema explica cómo insertar un gráfico en un documento, cómo escribir sus números y cómo cambiar su aspecto.

## Insertar un gráfico

1. Coloque el cursor donde debe ir el gráfico.
2. Haga clic en **Insertar > Gráfico** y elija un tipo: columnas, columnas apiladas, columnas apiladas al 100 %, barras, barras apiladas, líneas, líneas con marcadores, circular, anillo, áreas, áreas apiladas, dispersión, radar, radar con marcadores, radar relleno, máximo-mínimo-cierre, apertura-máximo-mínimo-cierre, volumen-máximo-mínimo-cierre o volumen-apertura-máximo-mínimo-cierre.

Un gráfico **bursátil** muestra precios: una línea para cada categoría, desde su precio más alto hasta el más bajo, con un pequeño punto para el cierre o, cuando hay apertura, una barra entre la apertura y el cierre (clara si sube y oscura si baja). El gráfico se crea con sus columnas tal como están, de izquierda a derecha, y no se comprueba que los números correspondan a cada lugar, así que colóquelos en este orden: máximo, mínimo y cierre; apertura, máximo, mínimo y cierre; volumen, máximo, mínimo y cierre; o volumen, apertura, máximo, mínimo y cierre. Si las columnas están en otro orden, el gráfico se dibuja igualmente, pero las marcas significan otra cosa (el punto de cierre va en la tercera columna de precios, y la caja va desde la primera columna de precios hasta la última). Con volumen, este se lee como columnas en el eje izquierdo y los precios se leen en un segundo eje a la derecha. Si el número de columnas no corresponde al tipo, JeffOffice indica cuántas necesita y no crea el gráfico.

Un gráfico **apilado** superpone las series unas sobre otras. Un gráfico **apilado al 100 %** hace que cada pila tenga la misma altura, para que vea las proporciones. Un gráfico de **dispersión** coloca cada punto donde indican sus dos números.

El gráfico se inserta con números de ejemplo, en línea con el texto, como una letra capital. Se abre la ficha **Formato de gráfico**.

## Escribir sus propios números

Seleccione el gráfico y haga clic en **Formato de gráfico > Editar datos**. Se abre una tabla.

- La primera columna contiene las palabras bajo los puntos (las **categorías**), como los meses.
- La primera fila contiene el nombre de cada **serie**, que es un conjunto de números, como un producto.
- Las demás celdas contienen los números.

Haga clic en **Agregar fila** o **Agregar columna** para añadir más, o en **Quitar última fila** o **Quitar última columna** para quitar. Haga clic en **Aceptar**. Si una celda no contiene un número, el gráfico no se modifica y la barra de estado indica en qué fila está el problema. Un gráfico puede tener hasta 200 categorías y 24 series.

En un gráfico de dispersión, la primera columna contiene los números que colocan los puntos de izquierda a derecha.

## Gráficos guardados por otro programa

Algunos gráficos guardados por otros programas conservan sus números en un pequeño **libro de cálculo** (un fichero de hoja de cálculo) almacenado dentro de su fichero. Seleccione un gráfico de este tipo y haga clic en **Formato de gráfico > Editar datos**. La tabla muestra las celdas de ese libro que lee el gráfico. Cambie un número, un nombre bajo un punto o el nombre de una serie, y haga clic en **Aceptar**. El cambio se escribe en el libro y en el gráfico a la vez, como un solo paso que puede deshacer, de modo que los demás programas también muestran los nuevos números.

No puede agregar ni quitar filas o columnas de un gráfico de este tipo, y **Datos del gráfico** e **Intercambiar filas y columnas** aparecen atenuados, porque el gráfico está vinculado a esas celdas. Lo mismo ocurre cuando el gráfico lee celdas de otro fichero, en un bloque que no es una sola columna ni una sola fila, o en celdas que contienen una fórmula, una fecha o texto donde debería haber un número. El comando aparece atenuado y su información en pantalla indica el motivo.

## Cambiar el aspecto

Estos comandos están en la ficha **Formato de gráfico**. Cada uno es un solo paso que puede deshacer, y el gráfico conserva sus colores y el resto de su aspecto.

- **Cambiar tipo de gráfico** convierte el gráfico en otro tipo con los mismos números. Para cambiar a un tipo bursátil, las series del gráfico deben estar en el orden que pide ese tipo; si no lo están, se indica el orden necesario y el gráfico no cambia.
- **Título del gráfico** muestra u oculta el título. **Editar título del gráfico** cambia su texto.
- **Leyenda** es la clave que indica qué color corresponde a cada serie. Colóquela a la derecha, arriba, a la izquierda o abajo, o quítela.
- **Ejes** muestra u oculta la línea de palabras bajo el gráfico (las **categorías**) y la línea de números junto a él (los **valores**), y define un título para cada una. Un gráfico circular no tiene ejes.
- **Líneas de cuadrícula** dibuja líneas tenues a través del gráfico para facilitar la lectura de los números. Las líneas **principales** se sitúan en los números principales del lateral, y las **secundarias** entre ellas.
- **Estilos de gráfico** elige uno de ocho estilos generales.
- **Etiquetas de datos** escribe cada número sobre su columna, punto o porción.
- **Ajustar texto**, **Texto alternativo**, y **Alto** y **Ancho** funcionan igual que con las imágenes. Consulte [Agregar y cambiar imágenes](pictures).

## La ficha Gráfico

Seleccione un gráfico y se abre la ficha **Gráfico** junto a la ficha **Formato de gráfico**. Todo lo que hay en ella cambia el gráfico en un paso que puede deshacer.

- **Elementos del gráfico** agrega, quita o coloca una parte del gráfico: **Ejes**, **Títulos de eje**, **Título del gráfico**, **Etiquetas de datos** (qué contienen, dónde se colocan y el separador entre ellos), **Líneas de cuadrícula**, **Leyenda**, **Barras de error** (barras cortas que muestran cuánto puede desviarse un número), **Líneas** (líneas de caída, líneas de máximos y mínimos, y barras de subida y bajada), **Tabla de datos** (una cuadrícula con los números bajo el gráfico, con una columna por categoría; su lista activa y desactiva la tabla, y define las líneas entre filas, las líneas entre columnas, el contorno y las claves de leyenda, que muestran el color de cada serie junto a su nombre; se ofrece para gráficos de columnas, de líneas y de áreas) y **Línea de tendencia** (una línea que muestra hacia dónde se dirigen los números, con su ecuación si lo desea).
- **Diseños** establece varias partes a la vez, como el título, la leyenda, las etiquetas y las líneas de cuadrícula.
- **Colores** asigna a las series un nuevo conjunto de colores. **Estilos** da a todo el gráfico un aspecto, como sencillo, limpio, oscuro o suave.
- **Intercambiar filas y columnas** invierte los números: las series pasan a ser las palabras bajo los puntos, y esas palabras pasan a ser las series.
- **Datos del gráfico** muestra la lista de series. Marque las que desea dibujar, cambie sus nombres y muévalas hacia arriba o hacia abajo.
- **Editar datos** abre la tabla de números.
- **Tipo de gráfico** muestra una imagen de cada tipo. Elija un tipo para todo el gráfico, o un tipo para cada serie, por ejemplo columnas con una línea encima.
- **Formato de elemento de gráfico** establece el relleno y el contorno del área del gráfico, del área de trazado (el recuadro donde se sitúan las marcas) o de una serie. En una línea, también define si es suave y qué marca aparece en cada punto.
- **Selección actual** muestra las partes del gráfico: el área del gráfico, el área de trazado y cada serie. Elija una, y **Formato de selección** abre la ventana de formato en esa parte. **Restablecer para coincidir con el estilo** quita lo que se definió en la parte elegida y devuelve lo que le asigna el estilo del gráfico. En el área del gráfico, eso significa su relleno, su contorno y el color de texto que haya aplicado un estilo (el texto claro del estilo oscuro vuelve al color de texto del tema). En el área de trazado, son su relleno y su contorno. En una serie, es su color (el que le asigna la opción Colores o el estilo del gráfico), su contorno, sus marcas tal como las tiene ese tipo de gráfico, y los colores de sus puntos; una línea suave se mantiene si el tipo de gráfico es suave. No cambia títulos, leyenda, etiquetas, ejes, líneas de cuadrícula ni datos, y no hace nada con una parte que ya coincide con el estilo. Es un paso de Deshacer.
- **Formato del eje horizontal** y **Formato del eje vertical** establecen los números más bajo y más alto, el tamaño de cada paso, el orden, el punto donde cruza el otro eje, el aspecto de los números y las marcas de graduación.

## Los botones junto a un gráfico

Tres pequeños botones aparecen a la derecha de un gráfico seleccionado.

- **Elementos del gráfico** (el signo más) abre el mismo menú que **Elementos del gráfico** de la ficha.
- **Colores y estilos del gráfico** (el pincel) muestra los conjuntos de colores y los estilos.
- **Filtros de gráfico** (el embudo) muestra las series. Desmarque una serie para dejarla fuera del gráfico. El fichero la conserva, así que vuelve a aparecer cuando la marque de nuevo. Un gráfico siempre mantiene visible al menos una serie.

## Guardar un gráfico en un fichero OpenDocument

Si guarda el documento como fichero de **texto OpenDocument** (`.odt`), cada gráfico se guarda como un objeto independiente y, al abrirlo de nuevo, sigue siendo un gráfico que puede modificar. Se conservan el tipo, los números, el título, la leyenda, las etiquetas, los colores, los ejes, las líneas de cuadrícula, hasta cuatro líneas de tendencia por serie y las barras de error. Algunas partes no encajan en ese formato, como la tabla de datos, las líneas de caída, las líneas de máximos y mínimos, las barras de subida y bajada, y las barras de error cuyos importes escribió usted. JeffWrite muestra una lista de lo que se omitió al guardar.

## Gráficos en otros documentos

JeffWrite muestra los gráficos que contiene un documento y los conserva tal como están. Puede cambiar el tipo, el título, la leyenda y las etiquetas de un gráfico que sabe escribir completo. Si un gráfico tiene partes que JeffWrite no modifica, como efectos especiales, lo indica y deja el gráfico sin cambios. Si un gráfico lee sus números de una hoja de cálculo guardada con él, puede cambiar todo excepto los números.
