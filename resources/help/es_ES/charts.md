# Gráficos
<!-- keywords: gráfico, gráficos, diagrama, gráfico de columnas, gráfico de barras, gráfico de líneas, gráfico circular, anillo, áreas, dispersión, insertar gráfico, datos del gráfico, editar datos, título del gráfico, leyenda, etiquetas de datos, tipo de gráfico, formato de gráfico, ejes, título del eje, líneas de cuadrícula, estilo de gráfico, línea de tendencia, barras de error, colores del gráfico, diseño rápido, dar formato al elemento del gráfico -->

Un **gráfico** convierte los números en una imagen, como columnas, una línea o porciones de un círculo. Este tema explica cómo colocar un gráfico en una página, cómo escribir sus números y cómo cambiar su aspecto.

## Colocar un gráfico en una página

1. Vaya a la página donde debe ir el gráfico.
2. Haga clic en **Insertar > Gráfico** y elija un tipo: columnas, columnas apiladas, columnas apiladas al 100 %, barras, barras apiladas, líneas, líneas con marcadores, circular, anillo, áreas, áreas apiladas, dispersión, radar, radar con marcadores, radar relleno, máximo-mínimo-cierre, apertura-máximo-mínimo-cierre, volumen-máximo-mínimo-cierre o volumen-apertura-máximo-mínimo-cierre.

Un gráfico **bursátil** muestra precios: una línea para cada categoría desde su precio más alto hasta el más bajo, con un punto pequeño para el cierre o, si hay apertura, una barra entre la apertura y el cierre (clara si sube y oscura si baja). El gráfico se crea con sus columnas tal como están, de izquierda a derecha, y nada comprueba que los números correspondan a cada posición, así que colóquelos en este orden: máximo, mínimo y cierre; apertura, máximo, mínimo y cierre; volumen, máximo, mínimo y cierre; o volumen, apertura, máximo, mínimo y cierre. Si las columnas están en otro orden, el gráfico se dibuja igualmente, pero las marcas significan otra cosa (el punto de cierre va en la tercera columna de precios, y la caja va de la primera a la última columna de precios). Con volumen, el volumen se lee en el eje izquierdo y los precios, en un segundo eje a la derecha. Si el número de columnas no corresponde al tipo, JeffPub indica cuántas necesita y no crea ningún gráfico.

Un gráfico **apilado** pone una serie encima de otra. Un gráfico **apilado al 100 %** hace que cada pila tenga la misma altura, para que vea proporciones. Un gráfico de **dispersión** coloca cada punto donde indican sus dos números.

El gráfico aparece con números de ejemplo, en el centro de la página, y se abre la pestaña **Formato de gráfico**. Un gráfico es un objeto como una imagen o una forma: arrástrelo para moverlo, arrastre un controlador para cambiar su tamaño (el gráfico se vuelve a dibujar para ajustarse, de modo que sus textos mantienen su forma) y gírelo con **Girar** en el grupo **Organizar**.

## Escribir sus propios números

Seleccione el gráfico y haga clic en **Formato de gráfico > Editar datos**. Se abre una tabla.

- La primera columna contiene las palabras que aparecen debajo de los puntos (las **categorías**), como los meses.
- La primera fila contiene el nombre de cada **serie**, que es un conjunto de números, como un producto.
- El resto de celdas contiene los números.

Haga clic en **Agregar fila** o **Agregar columna** para añadir más, o en **Quitar última fila** o **Quitar última columna** para quitar. Haga clic en **Aceptar**. Si una celda no es un número, el gráfico no cambia y la barra de estado indica en qué fila está el problema. Un gráfico puede tener hasta 200 categorías y 24 series.

En un gráfico de dispersión, la primera columna contiene los números que colocan los puntos de izquierda a derecha.

## Cambiar su aspecto

Estos comandos están en la pestaña **Formato de gráfico**. Cada uno se puede deshacer en un solo paso, y el gráfico conserva sus colores y el resto de su aspecto.

- **Cambiar tipo de gráfico** convierte el gráfico en otro tipo con los mismos números. Para cambiar a un tipo bursátil, las series del gráfico deben estar en el orden que pide ese tipo; si no lo están, se indica el orden necesario y el gráfico no cambia.
- **Título del gráfico** muestra u oculta el título. **Editar título del gráfico** cambia su texto.
- **Leyenda** es la clave que indica qué color corresponde a cada serie. Colóquela a la derecha, arriba, a la izquierda o abajo, o quítela.
- **Ejes** muestra u oculta la línea de palabras debajo del gráfico (las **categorías**) y la línea de números a su lado (los **valores**), y define un título para cada una. Un gráfico circular no tiene ejes.
- **Líneas de cuadrícula** dibuja líneas tenues a través del gráfico para seguir mejor los números. Las líneas **principales** coinciden con los números principales del lateral, y las **secundarias** quedan entre ellas.
- **Estilos de gráfico** permite elegir uno de ocho estilos generales.
- **Etiquetas de datos** escribe cada número sobre su columna, punto o porción.

## Añadir líneas de tendencia, barras de error y colores

Seleccione el gráfico. Junto a él aparecen tres botones pequeños, y los mismos comandos están en la pestaña **Formato de gráfico**.

- **Elementos del gráfico** (el botón con signo más) añade o quita una parte del gráfico: el título, las etiquetas, una tabla con los números y más. Su entrada **Línea de tendencia** dibuja una línea que pasa por los puntos y muestra hacia dónde se dirigen (lineal, exponencial y otros tipos), y puede escribir la ecuación de la línea en el gráfico. Su entrada **Barras de error** dibuja una pequeña barra en cada punto para indicar cuánto puede diferir el número real del que se muestra: una cantidad fija, un porcentaje o una dispersión de los números.
- **Diseños** elige una de varias disposiciones predefinidas del título, la leyenda y las etiquetas.
- **Colores** cambia los colores en los que se dibujan las series. **Estilos** (el botón del pincel) elige un aspecto general.
- **Filtros de gráfico** (el botón del embudo) oculta una serie sin borrar sus números.
- **Formato del gráfico** abre una ventana para el aspecto de una parte: todo el gráfico, el área de trazado o una serie. **Dar formato al eje horizontal** y **Dar formato al eje vertical** definen el rango, los intervalos y el formato de número de un eje.
- **Selección actual** enumera las partes del gráfico: el área del gráfico, el área de trazado y cada serie. Elija una y **Dar formato a la selección** abre la ventana de formato en esa parte. **Restablecer para coincidir con el estilo** quita lo que se haya definido en la parte elegida y devuelve lo que le asigna el estilo del gráfico. En el área del gráfico, eso significa su relleno, su línea y el color del texto que haya definido un aspecto (el texto claro del aspecto oscuro vuelve al color de texto del tema). En el área de trazado, son su relleno y su línea. En una serie, son su color (el que le asigna la opción Colores o el estilo del gráfico), su contorno, sus marcas según el tipo de gráfico y los colores de sus puntos; una línea suavizada se conserva si el tipo de gráfico es suavizado. No cambia títulos, leyenda, etiquetas, ejes, líneas de cuadrícula ni datos, y no hace nada con una parte que ya coincide con el estilo. Se puede deshacer en un paso.
- **Datos del gráfico** y **Intercambiar filas y columnas** cambian qué números son las series y cuáles son las categorías.

Cada uno de estos comandos se puede deshacer en un solo paso.

## Guardar, imprimir y PDF

- Un fichero **.jpub** guarda el gráfico como gráfico, de modo que puede abrirlo más tarde y cambiar sus números.
- Un fichero **.pub** no puede guardar un gráfico que otros programas abran como gráfico. JeffPub guarda el gráfico como una imagen y le indica cuántos gráficos ha guardado de esa forma. Conserve una copia en **.jpub** si quiere cambiar los números más adelante.
- **PDF** e impresión dibujan el gráfico como líneas, formas y texto real, no como una imagen, de modo que se mantiene nítido a cualquier tamaño.
