# Gráficos, minigráficos, imágenes y formas

<!-- keywords: agrupar, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, elegir varios, cuadro de selección, filtros de gráfico, ocultar series, gráficos recomendados, sugerencias de gráfico, gráfico, gráfico de columnas, columna, barra, línea, circular, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, aspecto, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, tendencia, ecuación, R al cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, minigráfico pequeño, marcadores, nombre, texto alternativo, escala logarítmica, logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, estilos, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella -->

Un gráfico es una imagen de números que sigue vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, seleccione también esas celdas. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la pestaña **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Área**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver primero todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte la orientación de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la pestaña **Insertar**. JeffCrunch analiza las celdas y muestra algunos tipos adecuados, del mejor al menos adecuado. Por ejemplo, las fechas en la columna lateral sugieren una línea, y una columna con unos pocos números positivos y etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que contiene datos.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie temporalmente

Elija un gráfico. Junto a él aparecen tres botones pequeños. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para dejar esa serie fuera del gráfico. Vuelva a marcarla para que regrese en el mismo lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha quitado sigue en el archivo al guardarlo y permanece oculta al abrirlo, así que no se pierde nada. Vuelva a marcarla para mostrarla. (Un gráfico de anillo muestra todas las series al abrirlo).

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiar su tamaño.

- Mantenga pulsada **Alt** mientras arrastra para que un borde se alinee con el borde de una celda.
- Mantenga pulsada **Mayús** mientras arrastra una esquina para mantener las proporciones.
- Las teclas de flecha lo mueven unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición original.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase breve que explica a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué hace cuando inserta, elimina o cambia el tamaño de las celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o quedarse donde está. La elección se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el archivo también guarda esa opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y en la cinta de opciones aparece una pestaña **Gráfico**.

- El botón **+** (y **Elementos del gráfico** en la pestaña) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y permite indicar dónde van la leyenda y las etiquetas.
- El botón de pincel (y **Colores** y **Estilos** en la pestaña) aplica al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus aspectos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, indicar las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas o dispersión) dibuja una pequeña línea encima y debajo de cada punto para mostrar cuánto podría diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Un archivo que tenga sus propios importes para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) incluye **Líneas de caída**, que van desde el punto más alto de cada categoría hasta el eje, **Líneas de máximos y mínimos**, que unen los puntos más alto y más bajo de cada categoría, y **Barras de subida/bajada**, que son cuadros entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximos y mínimos y las barras de subida/bajada necesitan dos series o más. Los gráficos de área tienen líneas de caída.
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para ver hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben rápido y luego se estabilizan, **Polinómica** para una curva con inflexiones, **Potencial**, o **Media móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R al cuadrado** indica lo bien que se ajusta la línea (1 es un ajuste perfecto; 0 es ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja con un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Segundo horizontal** y **Segundo vertical** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de al lado.
- **Etiquetas de datos** también tiene **Contiene la etiqueta** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: coma, punto y coma, punto, salto de línea o espacio) y **Formato de número…** (un código de formato, como 0,0, para los números de las etiquetas; déjelo vacío para mostrar los números tal como están). **Valor desde celdas…** muestra el texto de algunas celdas como etiquetas, una celda por cada punto. Escriba las celdas, por ejemplo =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Formato del eje horizontal…** y **Formato del eje vertical…** en la pestaña **Gráfico** abren un cuadro para un solo eje. Permite establecer los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1 000 para miles, de modo que 30 000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, junto al título del eje), si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que resulta útil cuando los números difieren mucho de tamaño y todos deben ser mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, la posición de sus etiquetas y el ángulo de giro de las etiquetas. Solo un eje numérico tiene límites y unidades.
- **Mover gráfico…** en la pestaña **Gráfico** pide otra hoja de cálculo y coloca allí el gráfico, en las mismas celdas. Sigue leyendo las celdas que leía y conserva su tamaño y su posición en la hoja, incluso cuando las filas y columnas son más altas o anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar. Todavía no se puede crear una hoja de gráfico (una hoja formada solo por un gráfico), y un gráfico que esté en una no se puede mover.
- **Título del gráfico** y **Títulos de ejes** (en el menú **+**) también tienen **Texto del título…**. Escriba el texto que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contenga esa celda. Cuando la celda cambia, también cambia el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la pestaña **Gráfico** (y en el menú del botón derecho) guarda el gráfico seleccionado como un archivo de imagen **PNG** o **JPEG**, con el doble de tamaño que en la hoja para que se vea nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la pestaña **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y la leyenda, los ejes y las fuentes. El gráfico al que se aplica conserva sus propios números y las celdas que lee. Un solo **Deshacer** quita de nuevo el aspecto.
- **Cambiar tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o una columna de números justo al lado de ellos.

1. Seleccione los números. Para tener un minigráfico por fila, seleccione varias filas de números.
2. En la pestaña **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra por cada número. **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor que cero, todas del mismo tamaño, lo que resulta útil para resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, marque el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas donde se colocan). Elija una celda de ubicación por cada fila de datos, o por cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten el mismo aspecto. Haga clic en una celda con un minigráfico y aparecerá la pestaña **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del minigráfico en el que hizo clic. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se deben leer las celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** da al grupo un conjunto de colores. **Color del minigráfico** establece el color de la línea o de las barras, y el **Grosor** (cuán gruesa es la línea, en puntos). **Color de marcador** establece el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y establece los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da a todo el grupo una sola escala, para poder compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se coloca más adelante. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** elimina los minigráficos seleccionados o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la pestaña **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y aparecerá en la hoja, en la celda seleccionada. Una imagen más ancha de 480 puntos se reduce para caber y conserva su forma. El libro guarda la imagen en sí. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen simple formada por puntos. Un archivo que no es una imagen, o que supera los 16 MB, se rechaza con un mensaje. Un archivo WMF o EMF (los tipos de imagen que algunos programas antiguos crean con líneas y formas) también se dibuja y se guarda como PNG.

**Iconos** muestra la misma ventana de pequeñas imágenes de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlas por nombre. Elija uno o varios, y cada uno aparecerá en la hoja como una imagen de 48 puntos de ancho y de alto. Se dibuja en gris oscuro, con un tamaño que se ve nítido al ampliarlo. Cada uno es un solo paso que puede deshacer.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y aparecerá en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, inserta un cuadro vacío sin relleno y sin borde.

Con una forma seleccionada, **Estilos** en la pestaña **Formato de forma** le da un aspecto predefinido: un relleno y un contorno que combinan, como azul claro con un borde azul más oscuro. Los dos últimos conservan solo el contorno o solo el relleno. Un clic es un paso que puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

Cada uno es un solo paso que puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiar su tamaño, enviarlo hacia delante o hacia atrás, darle un nombre y un texto alternativo en **Propiedades**, o pulsar **Supr**.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparecerá la pestaña **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparecerá la pestaña **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su posición y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** establece el grosor (cuán gruesa es la línea) y los guiones (continua, discontinua o punteada). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta con forma de ventana: cuadrada, de 4 de ancho por 3 de alto, de 3 de ancho por 4 de alto, de 16 por 9 o de 9 por 16. El recorte se hace de manera uniforme desde el centro de lo que se ve, y la imagen se amplía hasta ocupar lo que queda. **Mostrar la imagen completa** recupera lo que se recortó. El archivo de la imagen no cambia: la parte recortada solo queda oculta.
- **Editar texto** es para formas y cuadros de texto. Escriba las palabras y elija cómo se colocan horizontal y verticalmente, el tamaño, la negrita o la fuente normal, y si se ajustan a la forma o permanecen en una sola línea.
- **Panel de selección** abre una lista de todos los objetos de la hoja por nombre, con el que está delante en primer lugar. Haga clic en un nombre para seleccionar ese objeto, lo que resulta útil cuando los objetos se tapan entre sí. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** establece el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o amplían las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copian y pegan igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la original. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y, después, mantenga pulsada Mayús o Ctrl y haga clic en otro para seleccionarlo también. Haga clic de nuevo en uno seleccionado con Mayús o Ctrl para dejarlo. Cada objeto seleccionado muestra un marco discontinuo. También puede elegir **Seleccionar objetos** en la pestaña **Formato de forma** y arrastrar un cuadro sobre celdas vacías: se seleccionan todos los objetos que quepan por completo dentro del cuadro. Pulse Esc, o elija de nuevo **Seleccionar objetos**, para volver a seleccionar celdas. Pulse Supr para eliminar todos los objetos seleccionados en un solo paso.

Con dos o más objetos seleccionados, la pestaña **Formato de forma** tiene estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el archivo lo guarda como grupo. Un gráfico no se puede poner en un grupo.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su lugar en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Centrar**, **Alinear a la derecha**, **Alinear arriba**, **Centrar verticalmente** y **Alinear abajo**. Se alinean con el borde o el centro del conjunto completo. **Distribuir horizontalmente** y **Distribuir verticalmente** reparten de manera uniforme tres o más objetos entre los dos de los extremos.

Agrupar, Desagrupar y cada opción de alineación son un paso que puede deshacer.

## Cambiarle el nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba allí un nombre nuevo y pulse **Entrar**. **Deshacer** restaura el nombre anterior, y el nuevo se guarda en el archivo.

## Eliminarlo

Pulse **Supr** con el objeto seleccionado. **Deshacer** lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, de modo que un gráfico creado aquí se abre allí de la misma manera. Un gráfico procedente de otro programa que contenga algo que JeffCrunch no puede conservar al reescribirlo (barras de error o un gráfico tridimensional) se puede mover, cambiar de tamaño y eliminar, pero sus elementos y datos no se modifican aquí.
