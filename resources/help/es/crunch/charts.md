# Gráficos, minigráficos, imágenes y formas

<!-- keywords: botones de campo, botones de campo de gráfico dinámico, agrupar, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, seleccionar varios, cuadro de selección, filtros de gráfico, ocultar series, gráficos recomendados, sugerencias de gráfico, gráfico, diagrama, columna, barra, línea, circular, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, aspecto, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, mini gráfico, marcadores, nombre, texto alternativo, logarítmica, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, estilos, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella -->

Un gráfico es una imagen de números que permanece vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, seleccione también esas celdas. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Área**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores, o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte la disposición de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la ficha **Insertar**. JeffCrunch examina las celdas y muestra algunos tipos adecuados, del mejor al menos adecuado. Por ejemplo, si las fechas aparecen en una columna lateral, sugiere una línea, y una sola columna con unos pocos números positivos y etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y seleccione **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que tiene contenido.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie por ahora

Elija un gráfico. Junto a él aparecen tres pequeños botones. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para quitar esa serie del gráfico. Vuelva a marcarla para que regrese a su lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha quitado sigue en el archivo al guardarlo, y permanece oculta al abrirlo, así que no se pierde nada. Vuelva a marcarla para mostrarla. (Un gráfico de anillo muestra todas las series al abrirse).

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiarle el tamaño.

- Mantenga pulsada **Alt** mientras arrastra para que un borde se alinee con el borde de una celda.
- Mantenga pulsada **Mayús** mientras arrastra una esquina para conservar las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo hace más grande o más pequeño, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición original.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase breve que indica a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué ocurre al insertar, eliminar o cambiar el tamaño de las celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o quedarse donde está. La elección se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue visible en pantalla); el archivo también conserva esta opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y en la cinta de opciones aparece la ficha **Gráfico**.

- El botón **+** (y **Elementos de gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y permite indicar dónde van la leyenda y las etiquetas.
- El botón del pincel (y **Colores** y **Estilos** en la ficha) asigna al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, indicar las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Botones de campo** (en el menú **+**, solo para un gráfico dinámico) muestra pequeños botones con los nombres de los campos de la tabla dinámica que sigue el gráfico: los filtros del informe bajo el título, y las filas, los valores y las columnas en la parte inferior. Active o desactive cada tipo, o elija **Mostrar todo** u **Ocultar todo**. Un gráfico dinámico nuevo los muestra.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas, áreas o dispersión) dibuja una pequeña línea por encima y por debajo de cada punto para mostrar cuánto podría diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Elija **Personalizado** para usar importes propios: escriba las celdas que contienen los importes por encima de cada punto (por ejemplo, `=Hoja1!$D$2:$D$4`) y luego las celdas de los importes por debajo. Deje una vacía para dibujar las barras de un solo lado. Las barras cambian cuando cambian esas celdas. Un archivo que tiene sus propios importes para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) incluye **Líneas de caída**, que van desde el punto más alto de cada categoría hasta el eje; **Líneas de máximo y mínimo**, que unen el punto más alto y el más bajo de cada categoría; y **Barras de subida/bajada**, que son recuadros entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximo y mínimo y las barras de subida/bajada necesitan dos series o más. Los gráficos de área tienen líneas de caída.
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para ver hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben rápido y luego se estabilizan, **Polinómica** para una curva con curvaturas, **Potencial** o **Media móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R cuadrado** escribe lo bien que se ajusta la línea (1 es un ajuste perfecto; 0 es ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja frente a un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Segundo horizontal** y **Segundo vertical** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de a su lado.
- **Etiquetas de datos** también tiene **Contenido de la etiqueta** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: coma, punto y coma, punto, nueva línea o espacio) y **Formato de número…** (un código de formato como 0,0 para los números de las etiquetas; déjelo vacío para mostrar los números tal como están). **Valor de celdas…** muestra las palabras de algunas celdas como etiquetas, una celda por cada punto. Escriba las celdas, por ejemplo =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Desactive todos los contenidos y las etiquetas desaparecen.
- **Formato del eje horizontal…** y **Formato del eje vertical…** en la ficha **Gráfico** abren un cuadro para un eje. Permite definir los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1000 para miles, de modo que 30 000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, junto al título del eje), si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que resulta adecuado para números de tamaños muy distintos que deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto se giran. Solo un eje numérico tiene límites y unidades.
- **Mover gráfico…** en la ficha **Gráfico** pide otra hoja de cálculo y coloca allí el gráfico, en las mismas celdas. Sigue leyendo las celdas que leía y conserva su tamaño y su posición en la hoja, incluso donde las filas y columnas son más altas o más anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar. Todavía no se puede crear una hoja de gráfico (una hoja que solo contiene un gráfico), y tampoco se puede mover un gráfico que esté en una de ellas.
- **Título del gráfico** y **Títulos de ejes** (en el menú **+**) tienen también **Texto del título…**. Escriba las palabras que desee. O escriba un signo igual seguido de una celda, como **=Hoja1!A1**, y el título mostrará lo que contiene esa celda. Cuando la celda cambia, también cambia el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la ficha **Gráfico** (y en el menú contextual del botón derecho) guarda el gráfico seleccionado como un archivo de imagen **PNG** o **JPEG**, con el doble de tamaño que en la hoja para que se mantenga nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la ficha **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y de la leyenda, los ejes y las fuentes. El gráfico al que se aplica conserva sus propios números y las celdas que lee. Un **Deshacer** quita de nuevo el aspecto.
- **Cambiar tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe, por ejemplo, como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando contiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o de una columna de números junto a ellos.

1. Seleccione los números. Para un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. **Línea** une los números con una línea. **Columna** dibuja una barra por cada número. **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor, todas del mismo tamaño, lo que resulta adecuado para resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, marque el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas en las que se colocan). Elija una celda de ubicación por cada fila de datos, o por cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten un mismo aspecto. Haga clic en una celda con un minigráfico y aparecerá la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que ha seleccionado. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se deben leer las celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: **Punto alto**, **Punto bajo**, **Puntos negativos**, **Primer punto**, **Último punto** y, para una línea, **Marcadores** en cada punto.
- **Estilos** asigna al grupo un conjunto de colores. **Color del minigráfico** define el color de la línea o de las barras, y el **Grosor** (cuán gruesa es la línea, en puntos). **Color de marcador** define el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y define los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** asigna a todo el grupo una sola escala, para que pueda compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según sus fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se coloca más allá. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** elimina los minigráficos seleccionados o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la ficha **Insertar**, en **Ilustraciones**, **Imagen** abre un explorador de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y aparecerá en la hoja, en la celda seleccionada. Una imagen de más de 480 puntos de ancho se reduce para ajustarse, y conserva su forma. El libro guarda la propia imagen. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se conserva como PNG, una imagen sencilla formada por puntos. Un archivo que no es una imagen, o que supera los 16 MB, se rechaza con un mensaje. Un archivo WMF o EMF (los tipos de imagen que algunos programas antiguos crean a partir de líneas y formas) también se dibuja y se conserva como PNG.

**Iconos** muestra la misma ventana de pequeñas imágenes de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlas por nombre. Elija uno o varios, y cada uno aparecerá en la hoja como una imagen de 48 puntos de ancho y de alto. Se dibuja en gris oscuro, con un tamaño que se mantiene nítido al agrandarlo. Cada uno es un paso que puede deshacer.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y aparecerá en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, inserta un cuadro vacío sin relleno y sin borde.

Con una forma seleccionada, **Estilos** en la ficha **Formato de forma** le da un aspecto predefinido: un relleno y un contorno que combinan, como azul claro con un borde azul más oscuro. Los dos últimos conservan solo el contorno o solo el relleno. Un clic es un paso que puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

Cada una de estas acciones es un paso que puede deshacer. El nuevo objeto queda seleccionado, de modo que puede arrastrarlo, cambiarle el tamaño, enviarlo hacia delante o hacia atrás, darle un nombre y texto alternativo en **Propiedades**, o pulsar **Supr**.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparecerá la ficha **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparecerá la ficha **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su posición y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** define el grosor (cuán gruesa es la línea) y los guiones (continua, discontinua o punteada). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta con forma de ventana: cuadrada, de 4 de ancho por 3 de alto, de 3 de ancho por 4 de alto, de 16 por 9 o de 9 por 16. El recorte se hace uniformemente desde el centro de lo que se ve, y la imagen se amplía lo máximo posible con lo que queda. **Mostrar la imagen completa** recupera lo recortado. El archivo de la imagen no cambia: la parte recortada solo se oculta.
- **Editar texto** es para formas y cuadros de texto. Escriba las palabras y elija cómo se sitúan horizontal y verticalmente, el tamaño, la negrita o el estilo normal, y si se ajustan a la forma o permanecen en una sola línea.
- **Panel de selección** abre una lista de todos los objetos de la hoja por su nombre, con el que está delante en primer lugar. Haga clic en un nombre para seleccionar ese objeto, lo que resulta útil cuando los objetos se tapan entre sí. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** define el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o amplían las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que el original. Si lo pega en otro libro, conserva los números que mostraba pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y, manteniendo pulsada Mayús o Ctrl, haga clic en otro para seleccionarlo también. Haga clic de nuevo en uno seleccionado con Mayús o Ctrl para deseleccionarlo. Los objetos seleccionados muestran un marco discontinuo. También puede elegir **Seleccionar objetos** en la ficha **Formato de forma** y arrastrar un cuadro sobre celdas vacías: se seleccionan todos los objetos que quepan completamente dentro del cuadro. Pulse Esc, o elija de nuevo **Seleccionar objetos**, para volver a seleccionar celdas. Pulse Supr para eliminar todos los objetos seleccionados en un solo paso.

Con dos o más objetos seleccionados, la ficha **Formato de forma** tiene estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el archivo lo conserva como grupo. Un gráfico no se puede poner en un grupo.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su lugar en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Centrar horizontalmente**, **Alinear a la derecha**, **Alinear arriba**, **Centrar verticalmente** y **Alinear abajo**. Se alinean con el borde o con el centro del conjunto completo. **Distribuir horizontalmente** y **Distribuir verticalmente** reparten de forma uniforme tres o más objetos entre los dos de los extremos.

Agrupar, Desagrupar y cada opción de Alinear son un paso que puede deshacer.

## Cambiarle el nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. Deshacer restaura el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Pulse **Supr** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo descrito arriba se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, de modo que un gráfico que cree aquí se abre allí de la misma manera. Un gráfico procedente de otro programa que contiene algo que JeffCrunch no puede conservar al reescribirlo (barras de error o un gráfico tridimensional) puede moverse, cambiar de tamaño y eliminarse, pero sus elementos y sus datos no se modifican aquí.
