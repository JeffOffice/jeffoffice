# Gráficos, minigráficos, imágenes y formas
<!-- keywords: filtros de gráfico, ocultar series, gráficos recomendados, sugerencias de gráfico, gráfico, diagrama, columna, barra, línea, circular, áreas, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, aspecto, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, gráfico diminuto, marcadores, nombre, texto alternativo, logarítmica, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella -->

Un gráfico es una imagen de números que sigue unida a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están unidas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, seleccione también esas celdas. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Áreas**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como las columnas apiladas, una línea con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver primero todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte la dirección de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la ficha **Insertar**. JeffCrunch examina las celdas y muestra algunos tipos que se ajustan a ellas, del mejor al peor. Por ejemplo, las fechas en la columna lateral sugieren una línea, y una columna con unos pocos números positivos y etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que contiene datos.

Las series se colocan en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. En un gráfico de dispersión, los valores horizontales se toman de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie temporalmente

Elija un gráfico. Junto a él aparecen tres botones pequeños. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para dejar esa serie fuera del gráfico. Vuelva a marcarla para que regrese al mismo lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha dejado fuera no se guarda al guardar y cerrar el archivo, así que márquela de nuevo antes si la quiere conservar.

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiar su tamaño.

- Mantenga pulsada la tecla **Alt** mientras arrastra para que un borde se alinee con el borde de una celda.
- Mantenga pulsada la tecla **Mayús** mientras arrastra una esquina para mantener las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo hace más grande o más pequeño, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su sitio.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase corta que explica a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué hace cuando inserta, elimina o cambia el tamaño de celdas: se mueve y cambia de tamaño con las celdas, se mueve con ellas pero conserva su tamaño, o no se mueve. La opción se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue mostrándose en pantalla); el archivo también conserva esa opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y en la cinta de opciones aparece una ficha **Gráfico**.

- El botón **+** (y **Elementos de gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y permite indicar dónde van la leyenda y las etiquetas.
- El botón del pincel (y **Colores** y **Estilos** en la ficha) asigna al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus aspectos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, indicar las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para que vea hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben rápido y luego se estabilizan, **Polinómica** para una curva con curvaturas, **Potencial**, o **Media móvil**, que dibuja la media de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R cuadrado** escribe lo bien que se ajusta la línea (1 es un ajuste perfecto; 0 es ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja con un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de al lado.
- **Etiquetas de datos** también tiene **Contenido de la etiqueta** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: una coma, un punto y coma, un punto, un salto de línea o un espacio) y **Formato de número…** (un código de formato como 0,0 para los números de las etiquetas; déjelo vacío para mostrar los números tal como son). Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Dar formato al eje horizontal…** y **Dar formato al eje vertical…** en la ficha **Gráfico** abren un cuadro para un solo eje. Permite establecer los valores mínimo y máximo y las unidades principal y secundaria (automáticas o un número que escriba), si los valores van en orden inverso, si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que resulta útil para números de tamaños muy distintos que deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto se giran. Solo un eje numérico tiene límites y unidades.
- **Mover gráfico…** en la ficha **Gráfico** pide otra hoja de cálculo y coloca allí el gráfico, en las mismas celdas. Sigue leyendo las celdas que leía, y conserva su tamaño y su posición en la hoja, incluso cuando las filas y columnas son más altas o más anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar. No se puede crear todavía una hoja de gráfico (una hoja que sea solo un gráfico), y un gráfico que esté en una no se puede mover.
- **Texto del título…** también está disponible en **Título del gráfico** y **Títulos de ejes** (en el menú **+**). Escriba las palabras que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contenga esa celda. Cuando la celda cambia, también cambia el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la ficha **Gráfico** (y en el menú del botón derecho) guarda el gráfico seleccionado como archivo de imagen **PNG** o **JPEG**, con el doble de tamaño que en la hoja para que se vea nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la ficha **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y de la leyenda, los ejes y las fuentes. El gráfico al que lo aplica conserva sus propios números y las celdas que lee. Un solo **Deshacer** elimina de nuevo el aspecto.
- **Tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o de una columna de números justo al lado de ellos.

1. Seleccione los números. Para tener un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra para cada número. Un **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor que cero, todas del mismo tamaño, lo que resulta adecuado para resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, compruebe el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas donde se colocan). Elija una celda de ubicación por cada fila de datos, o por cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten el mismo aspecto. Haga clic en una celda con un minigráfico y aparecerá la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del minigráfico en el que hizo clic. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se leen las celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** asigna al grupo un conjunto de colores. **Color del minigráfico** establece el color de la línea o de las barras, y el **Grosor** (cuán gruesa es la línea, en puntos). **Color de marcador** establece el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y establece los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da al grupo completo una sola escala, para poder compararlos. **Valor personalizado** permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se sitúa más a la derecha. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** elimina los minigráficos seleccionados o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la ficha **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y se colocará en la hoja, en la celda seleccionada. Una imagen de más de 480 puntos de ancho se reduce para encajar y conserva su forma. El libro guarda la imagen en sí. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen normal formada por puntos. Un archivo que no es una imagen, o que pesa más de 16 MB, se rechaza con un mensaje.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y aparecerá en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, inserta un cuadro vacío sin relleno ni borde.

Cada uno es un solo paso que puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiar su tamaño, enviarlo hacia delante o hacia atrás, asignarle un nombre y un texto alternativo en **Propiedades**, o pulsar **Supr**.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Al elegir una imagen aparece la ficha **Formato de imagen**. Al elegir una forma, una línea o un cuadro de texto aparece la ficha **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su posición y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** establece el grosor (cuán gruesa es la línea) y los guiones (continua, discontinua o punteada). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, en una imagen, la corta con forma de ventana: cuadrada, de 4 de ancho por 3 de alto, de 3 de ancho por 4 de alto, de 16 por 9 o de 9 por 16. El recorte se hace de forma uniforme desde el centro de lo que se ve, y la imagen se agranda hasta ocupar lo que queda. **Mostrar la imagen completa** recupera lo recortado. El archivo de la imagen no se modifica: la parte recortada solo queda oculta.
- **Editar texto** sirve para formas y cuadros de texto. Escriba las palabras y elija cómo se sitúan horizontal y verticalmente, el tamaño, la negrita o normal, y si se ajustan a la forma o permanecen en una sola línea.
- **Panel de selección** abre una lista de todos los objetos de la hoja por nombre, con el que está delante en primer lugar. Haga clic en un nombre para seleccionar ese objeto, lo que resulta útil cuando los objetos se ocultan unos a otros. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** establece el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o agrandan las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega de la misma forma que un gráfico. Una forma que usa algo distinto a una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la primera. Si lo pega en otro libro, conserva los números que mostraba pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Asignar un nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. **Deshacer** devuelve el nombre anterior, y el nuevo nombre se guarda en el archivo.

## Eliminarlo

Pulse **Supr** con el objeto seleccionado. **Deshacer** lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, así que un gráfico que crea aquí se abre allí de la misma manera. Un gráfico procedente de otro programa con algo que JeffCrunch no puede conservar al volver a escribir un gráfico (barras de error, un gráfico tridimensional) se puede mover, cambiar de tamaño y eliminar, pero sus elementos y datos no se modifican aquí.
