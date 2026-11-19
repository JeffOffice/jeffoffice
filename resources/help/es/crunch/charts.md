# Gráficos, minigráficos, imágenes y formas

<!-- keywords: objeto incrustado, insertar objeto, objeto desde un rango, objeto desde un gráfico, objeto desde una página, pegar como objeto, lista de objetos, editar objeto, botones de campo, botones de gráfico dinámico, grupo, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, elegir varios, cuadro de selección, filtros de gráfico, ocultar series, gráficos recomendados, sugerencias de gráfico, gráfico, diagrama, columna, barra, línea, circular, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, estilos, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, victoria/derrota, minigráfico pequeño, marcadores, nombre, texto alternativo, logarítmico, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, estilos, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella, diagrama, diagramas, imágenes en línea, imágenes gratuitas, openverse, wikimedia commons, licencia, crédito, texto artístico, wordart, deformar, comprimir imágenes, reducir imágenes, tamaño de archivo, ppp, eliminar áreas recortadas, código de barras, código QR, vincular una forma a una celda, forma que muestra una celda, cuadro de texto que muestra una celda, valor de celda en una forma -->

Un gráfico es una imagen de números que sigue vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, selecciónelas también. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y como etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Área**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte el sentido de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la ficha **Insertar**. JeffCrunch examina las celdas y enumera algunos tipos adecuados, del mejor al peor. Por ejemplo, las fechas en la columna lateral sugieren una línea, y una columna con unos pocos números positivos y etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que tiene contenido.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie por ahora

Elija un gráfico. Junto a él aparecen tres botones pequeños. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para dejar esa serie fuera del gráfico. Vuelva a marcarla para que regrese al mismo lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha dejado fuera sigue en el archivo al guardarlo y permanece oculta al abrirlo, así que no se pierde nada. Márquela de nuevo para recuperarla. (Un gráfico de anillo muestra todas las series al abrirlo).

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiar su tamaño.

- Mantenga pulsada **Alt** mientras arrastra para que un borde se alinee con el borde de una celda.
- Mantenga pulsada **Mayús** mientras arrastra una esquina para conservar las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición original.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase breve que explica a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué hace al insertar, eliminar o cambiar el tamaño de celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o permanecer en su sitio. La elección se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el archivo también guarda esa elección.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y aparece en la cinta una ficha **Gráfico**.

- El botón **+** (y **Elementos del gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, yele indica dónde van la leyenda y las etiquetas.
- El botón del pincel (y **Colores** y **Estilos** en la ficha) asigna al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** enumera las series del gráfico. Puede agregar, editar, quitar y reordenar series, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Botones de campo** (en el menú **+**, solo para un gráfico dinámico) muestra pequeños botones con los nombres de los campos de la tabla dinámica que sigue el gráfico: los filtros de informe bajo el título, y las filas, los valores y las columnas a lo largo de la parte inferior. Active o desactive cada tipo, o elija **Mostrar todo** u **Ocultar todo**. Un gráfico dinámico nuevo los muestra.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas, áreas o dispersión) dibuja una pequeña línea encima y debajo de cada punto para mostrar cuánto podría diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Elija **Personalizado** para usar importes propios: escriba las celdas que contienen los importes sobre cada punto (por ejemplo `=Hoja1!$D$2:$D$4`) y, después, las celdas de los importes por debajo. Deje una vacía para dibujar las barras de un solo lado. Las barras cambian cuando cambian esas celdas. Un archivo que tenga sus propios importes para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) incluye **Líneas de caída**, que van desde el punto más alto de cada categoría hasta el eje, **Líneas de máximos y mínimos**, que unen el punto más alto y el más bajo de cada categoría, y **Barras de subida/bajada**, que son cuadros entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximos y mínimos y las barras de subida/bajada necesitan dos series o más. Los gráficos de área tienen líneas de caída.
- **Tabla de datos** (en el menú **+**, para un gráfico de columnas, líneas o áreas) coloca una cuadrícula con los números bajo el gráfico, con una columna para cada categoría y una fila para cada serie. Su lista activa y desactiva la tabla, y establece las líneas entre filas, las líneas entre columnas, el contorno y las claves de leyenda (el color de cada serie junto a su nombre).
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para ver hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben deprisa y luego se estabilizan, **Polinómica** para una curva con inflexiones, **Potencial**, o **Media móvil**, que dibuja la media de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R cuadrado** escribe qué tan bien se ajusta la línea (1 es un ajuste perfecto; 0, ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja frente a un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Superpuesta al gráfico**, que coloca la leyenda sobre el área de trazado en lugar de al lado.
- **Etiquetas de datos** también tiene **Etiqueta contiene** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: una coma, un punto y coma, un punto, un salto de línea o un espacio), y **Formato de número…** (un código de formato como 0,0 para los números de las etiquetas; déjelo vacío para mostrar los números tal como son). **Valor desde celdas…** muestra el texto de algunas celdas como etiquetas, una celda para cada punto. Escriba las celdas, como =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Desactive todos los contenidos y las etiquetas desaparecen.
- **Dar formato al eje horizontal…** y **Dar formato al eje vertical…** en la ficha **Gráfico** abren un cuadro para un solo eje. Establece los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1 000 para miles, de modo que 30 000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, junto al título del eje), si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que conviene a números que difieren mucho en tamaño y deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto se giran. Solo un eje numérico tiene límites y unidades.
- **Mover gráfico…** en la ficha **Gráfico** pregunta dónde debe ir el gráfico. Elija **Hoja nueva** y escriba un nombre, y el gráfico obtiene una hoja de gráfico propia: una hoja que solo contiene el gráfico, sin celdas. Elija **Objeto en** y seleccione una hoja de cálculo, y el gráfico se colocará en esa hoja. Un gráfico en una hoja de gráfico solo puede pasar a una hoja de cálculo, y la hoja de gráfico que deja se elimina. El gráfico sigue leyendo las celdas que leía. En una hoja de cálculo conserva su tamaño y su lugar en la hoja, incluso donde las filas y columnas son más altas o más anchas, y se mueve y cambia de tamaño con las celdas de allí como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar, salvo que la operación haya eliminado una hoja. Un archivo en formato `.ods` no puede contener una hoja de gráfico, así que guarde una hoja de gráfico en formato `.xlsx`.
- **Título del gráfico** y **Títulos de ejes** (en el menú **+**) también tienen **Texto del título…**. Escriba las palabras que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contenga esa celda. Cuando la celda cambie, también cambiará el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la ficha **Gráfico** (y en el menú del botón derecho) guarda el gráfico seleccionado como un archivo de imagen **PNG** o **JPEG**, con el doble de tamaño que en la hoja para que se vea nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la ficha **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y de la leyenda, los ejes y las fuentes. El gráfico al que se aplica conserva sus propios números y las celdas que lee. Un solo **Deshacer** quita de nuevo el aspecto.
- **Tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o de una columna de números, justo al lado de ellos.

1. Seleccione los números. Para un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Victoria/derrota**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra por cada número. **Victoria/derrota** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor, todas del mismo tamaño, lo que conviene a resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, marque el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas donde van). Elija una celda de ubicación por cada fila de datos, o por cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten un mismo aspecto. Haga clic en una celda con un minigráfico y aparecerá la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que seleccionó. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se deben leer celdas en filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y victoria/derrota.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** asigna al grupo un conjunto de colores. **Color del minigráfico** establece el color de la línea o de las barras, y el **Grosor** (cuánto de gruesa es la línea, en puntos). **Color del marcador** establece el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y establece los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da a todo el grupo una sola escala, para poder compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se sitúa más adelante. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** quita los minigráficos seleccionados, o sus grupos enteros.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la ficha **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y aparecerá en la hoja, en la celda seleccionada. Una imagen de más de 480 puntos de ancho se reduce para encajar, y conserva su forma. El libro guarda la imagen en sí. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen sencilla formada por puntos. Un archivo que no sea una imagen, o que pese más de 16 MB, se rechaza con un mensaje. Un archivo WMF o EMF (los tipos de imagen que algunos programas antiguos crean a partir de líneas y formas) también se dibuja y se guarda como PNG.

**Iconos** muestra la misma ventana de pequeñas imágenes de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlas por nombre. Elija uno o varios, y cada uno aparecerá en la hoja como una imagen de 48 puntos de ancho y de alto. Se dibuja en gris oscuro, con un tamaño que se mantiene nítido al ampliarlo. Cada uno es un solo paso que puede deshacer.

**Código de barras** abre un menú corto con dos opciones. **Código de barras** abre una ventana para crear un solo código de barras (**Códigos de barras desde celdas**, más abajo, crea muchos): ISBN de libro (con o sin precio), EAN-13, UPC-A, EAN-8, Code 128, Code 39 o un código QR, que puede contener cualquier texto, como una dirección web. Elija un **Tipo**, escriba el texto o los dígitos, y la ventana muestra el código de barras mientras escribe, y explica en rojo qué falla si no puede generarse. Pulse **Insertar** y aparecerá en la hoja como una imagen, en un paso que puede deshacer. Muévalo y cambie su tamaño como cualquier imagen. Para modificarlo, selecciónelo y pulse de nuevo **Código de barras**, cambie el texto y pulse **Actualizar**; el nuevo código ocupa el lugar y el ancho del anterior. Al guardar, se guarda como imagen con un SVG que se mantiene nítido, para que lo muestre cualquier programa, y JeffCrunch conserva sus ajustes en el archivo, de modo que sigue siendo un código de barras que puede editar al abrir el libro de nuevo en JeffCrunch.

**Código de barras > Códigos de barras desde celdas** (en el mismo menú **Código de barras**) crea un código de barras por cada celda con contenido de las celdas seleccionadas, para una hoja de etiquetas. Seleccione las celdas, pulse esta opción y, a continuación, elija el tipo y el aspecto en la misma ventana. Lo que escriba allí es solo una muestra, porque el texto de cada celda se convierte en su propio código de barras. Los códigos de barras se colocan en una columna justo a la derecha de las celdas seleccionadas, uno debajo de otro, como un solo paso que puede deshacer. Una celda cuyo texto el código no puede contener (una letra en un EAN-13, por ejemplo) se indica en un mensaje y no recibe código de barras. Se procesan como máximo 500 celdas a la vez; seleccione menos si indica que hay demasiadas. Cada código de barras recuerda la celda de la que procede y la sigue: cuando cambia esa celda, el código se vuelve a generar con el nuevo texto, en el mismo lugar y con el mismo tamaño, como un paso propio que puede deshacer. El vínculo se mueve con la celda al insertar o eliminar filas o columnas. Si elimina la celda, el código de barras conserva el último texto que tuvo. Al guardar, la imagen del archivo se genera a partir del texto actual de la celda, de modo que otros programas también muestran el valor actual.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y aparecerá en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, coloca un cuadro vacío sin relleno y sin borde. Pulse **Dibujar cuadro de texto** (el botón principal, o la primera opción de su menú) y, a continuación, arrastre sobre la hoja para dibujar el cuadro. Al arrastrar, el cuadro adopta el tamaño que arrastre, pero nunca es menor de 18 puntos (0,6 cm) por lado; si hace clic con menos de 4 puntos de movimiento, se crea el tamaño habitual. Mantenga pulsada la tecla **Mayús** para que el cuadro sea cuadrado. El puntero es una cruz hasta que suelte el botón; **Esc** o cualquier otro comando desactiva la herramienta. El cuadro es un paso que puede deshacer.

Para girar el texto de un cuadro, selecciónelo y pulse **Dirección del texto** en la ficha **Formato de forma**: **Horizontal**, **Girar todo el texto 90°** o **Girar todo el texto 270°**. El texto se escribe girado, tal como se muestra, y un libro guardado en formato .xlsx u .ods conserva la dirección.

Con una forma seleccionada, **Estilos** en la ficha **Formato de forma** le da un aspecto predefinido: un relleno y un contorno que combinan, como azul claro con borde azul oscuro. Las dos últimas opciones conservan solo el contorno o solo el relleno. Un clic es un paso que puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

## Buscar imágenes gratuitas en internet

**Imágenes en línea**, en el grupo **Ilustraciones** de la ficha **Insertar**, busca imágenes gratuitas. Proceden de dos bibliotecas abiertas: Openverse, que busca en muchas colecciones, y Wikimedia Commons, la biblioteca de las imágenes de Wikipedia.

1. Haga clic en **Imágenes en línea…**. Se abre un panel a la derecha.
2. Elija una biblioteca, escriba lo que busca y haga clic en **Buscar** (o pulse Intro).
3. Haga clic en una imagen para ver quién la hizo y qué **licencia** tiene. Una **licencia** indica cómo puede usar una imagen. Haga clic en el nombre de la licencia para leerla.
4. Haga clic en **Insertar** (o haga doble clic en la imagen).

**Qué envía JeffCrunch.** Las palabras de su búsqueda se envían a la biblioteca que elija, y solo cuando hace clic en **Buscar** o pulsa Intro. JeffCrunch pide después a la biblioteca las vistas previas pequeñas de lo que ha encontrado, y la imagen que inserte. Como cualquier página web, la biblioteca puede ver la dirección de internet de su equipo, y se le informa de que JeffCrunch hace la petición. No se envía nada de su libro.

La imagen aparece en la hoja como una imagen procedente de un archivo, y el libro la conserva. Su título se guarda como su **texto alternativo**. La mayoría de las licencias piden que se nombre al autor. Cuando **Agregar un crédito debajo de la imagen** está marcado, JeffCrunch añade debajo de la imagen un pequeño cuadro de texto con su título, su autor y su licencia. Puede moverlo, cambiarlo o eliminarlo. La imagen y su crédito se insertan en un solo paso, así que **Deshacer** los quita a ambos. Compruebe que la licencia permite el uso que va a dar a la imagen.

## Agregar texto artístico

**Texto artístico** son palabras con adornos: letras con color, contorno, sombra o una forma curva.

1. Haga clic en **Insertar > Texto > Texto artístico**. Se abre una galería de estilos.
2. Haga clic en un estilo. Las palabras «Escriba aquí el texto» aparecen en la hoja con ese estilo.
3. Para cambiar las palabras, seleccione el texto artístico y haga clic en **Editar texto…** en la ficha **Formato de forma**. Un cuadro pide las palabras, la fuente, el tamaño, negrita y cursiva. Un tamaño mayor hace más grande el texto artístico.
4. En la ficha **Formato de forma**, **Estilos de texto artístico** da a las letras otro estilo, y **Deformar texto** curva las palabras en una forma, como una onda o un arco. «Sin deformar» las mantiene rectas.

El texto artístico es una forma del dibujo de la hoja, con su deformación, su relleno y su contorno, así que conserva su aspecto al guardar el libro en formato .xlsx y volver a abrirlo, en JeffCrunch o en otro programa. Cada cambio es un paso que puede deshacer. Un libro guardado en formato .ods conserva también sus imágenes, formas, cuadros de texto, grupos, líneas y texto artístico. El texto artístico se guarda como la forma de texto artístico que tiene OpenDocument, con sus palabras, fuente, relleno, contorno y deformación, y vuelve a ser texto artístico al abrir de nuevo el archivo .ods. Algunas deformaciones para las que OpenDocument no tiene forma (Onda 2, Onda 4, y las formas inferior y superior de Hinchar y Desinflar) se guardan como palabras rectas con las mismas letras y estilo. Los gráficos también se conservan en un archivo .ods: vea más abajo.

## Reducir el tamaño de las imágenes

Una imagen de una cámara puede ser mucho mayor que lo que se ve en la hoja, y hace grande el archivo. Haga clic en una imagen y, después, en **Comprimir imágenes…** en el grupo **Ajustar** de la ficha **Formato de imagen**. Elija dónde se usará el libro: **Impresión comercial (300 ppp)**, **Impresión de escritorio (220 ppp)** o **Web (96 ppp)**. Deje marcada la opción **Aplicar a todas las imágenes de esta hoja** para reducir todas las imágenes de la hoja, o desmárquela para reducir solo la imagen seleccionada. **Eliminar las áreas recortadas de las imágenes** quita también las partes ocultas que recortó, y el recorte desaparece. Una imagen solo se sustituye cuando la nueva es más pequeña. Su tamaño, su lugar, su nombre y su texto alternativo se mantienen. **Deshacer** (Ctrl+Z) devuelve las imágenes originales, y el libro guardado contiene solo las imágenes más pequeñas. Las imágenes de otras hojas no cambian: vaya a cada hoja y repita la operación. Una imagen que es un dibujo (SVG, EMF o WMF), o una animación, se deja como está.

Cada uno es un paso que puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiarle el tamaño, llevarlo adelante o atrás, darle un nombre y un texto alternativo en **Propiedades**, o pulsar **Supr**.

## Mostrar el texto de una celda en una forma o un cuadro de texto

Una forma o un cuadro de texto puede mostrar lo que muestra una celda, y seguirla. Seleccione la forma, haga clic en la **barra de fórmulas**, escriba un signo igual y una celda, y pulse **Intro**.

- `=B2` vincula la forma a B2 de esta hoja. `=Hoja2!B3` la vincula a una celda de otra hoja. `=Total` la vincula a un nombre que representa una celda.
- La forma muestra entonces el texto de la celda tal como lo muestra la celda, con su formato de número. Cuando cambia la celda, su formato o una fórmula de la que depende, la forma también cambia. El texto más largo de lo que puede contener una celda (32 767 caracteres) se corta. Entre todas las formas vinculadas de un libro, se muestran como máximo 4 MB de texto: a partir de ahí, una forma muestra su texto cortado.
- Con la forma seleccionada, la barra de fórmulas muestra su vínculo, como `=Hoja1!$B$2`. Borre el contenido de la barra de fórmulas y pulse **Intro** para quitar el vínculo. La forma conserva entonces las palabras que muestra.
- Escribir palabras en la forma también quita el vínculo.
- Un vínculo es una sola celda. Un rango como `=A1:B2`, una lista, un cálculo o un nombre que no es una celda se rechaza con un mensaje, y la forma conserva su vínculo.
- Si se elimina la fila o la columna de la celda, o su hoja, la forma muestra `#REF!` y conserva el vínculo, como lo haría una fórmula. Deshacer devuelve la celda.
- Cada vínculo, y cada vez que se quita, es un paso que puede deshacer.

El vínculo se guarda en un archivo .xlsx junto con el texto que mostraba la celda, de modo que un programa que no sigue vínculos sigue mostrando las palabras, y JeffCrunch abre el archivo con el vínculo funcionando. OpenDocument no tiene forma de vincular un cuadro de texto a una celda, así que un libro guardado en formato .ods conserva las palabras que muestra la forma en ese momento, y no el vínculo.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparecerá la ficha **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparecerá la ficha **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su lugar y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** establece el grosor (cuánto de gruesa es la línea) y los guiones (continua, discontinua o punteada). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta con forma de ventana: cuadrada, de 4 de ancho por 3 de alto, de 3 de ancho por 4 de alto, de 16 por 9, o de 9 por 16. El recorte se hace de manera uniforme desde el centro de lo que se ve, y la imagen se amplía todo lo posible con lo que queda. **Mostrar la imagen completa** devuelve lo que se recortó. **Rellenar** mantiene el marco y recorta la imagen para que lo cubra por completo. **Ajustar** muestra la imagen entera, y el marco adopta la forma de la imagen dentro del anterior. **Recortar a forma** (la galería junto a **Recortar**) corta la imagen con una forma como un óvalo, una estrella o un rectángulo redondeado; la forma se guarda con la imagen. El archivo de la imagen no cambia: la parte recortada solo queda oculta.
- **Editar texto** sirve para formas y cuadros de texto. Escriba las palabras y elija cómo se sitúan a lo ancho y a lo alto, el tamaño, la negrita o la normal, y si se ajustan a la forma o se quedan en una sola línea. Para un texto artístico, pide en su lugar las palabras, la fuente, el tamaño, la negrita y la cursiva.
- **Panel de selección** abre una lista de todos los objetos de la hoja por su nombre, con el primero delante. Haga clic en un nombre para seleccionar ese objeto, lo que ayuda cuando unos objetos ocultan a otros. **Traer adelante** y **Enviar atrás** lo desplazan en la lista.
- **Propiedades** establece el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o se amplían las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la primera. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son aquellas de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y, a continuación, mantenga pulsada Mayús o Ctrl y haga clic en otro para seleccionarlo también. Haga clic de nuevo en uno seleccionado con Mayús o Ctrl para dejarlo. Los objetos seleccionados muestran un marco discontinuo. También puede elegir **Seleccionar objetos** en la ficha **Formato de forma** y arrastrar un cuadro sobre celdas vacías: se seleccionan todos los objetos que quepan por completo dentro del cuadro. Pulse Esc, o elija de nuevo **Seleccionar objetos**, para volver a seleccionar celdas. Pulse Supr para eliminar todos los objetos seleccionados en un solo paso.

Con dos o más objetos seleccionados, la ficha **Formato de forma** tiene estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el archivo lo conserva como grupo. Un gráfico no se puede poner en un grupo.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su sitio en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Centrar**, **Alinear a la derecha**, **Alinear arriba**, **Centrar verticalmente** y **Alinear abajo**. Se alinean con el borde o el centro de todo el conjunto. **Distribuir horizontalmente** y **Distribuir verticalmente** reparten tres o más objetos de manera uniforme entre los dos de los extremos.

Agrupar, Desagrupar y cada opción de Alinear son un paso que puede deshacer.

## Cambiarle el nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. Deshacer devuelve el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Pulse **Supr** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, de modo que un gráfico que crea aquí se abre allí de la misma manera. Un gráfico que procede de otro programa, con algo que JeffCrunch no puede conservar al reescribir un gráfico (barras de error, un gráfico tridimensional), se puede mover, cambiar de tamaño y eliminar, pero sus elementos y datos no se modifican aquí.

## Insertar un objeto

Un **objeto** guarda su propio archivo dentro del libro, de modo que la aplicación que lo creó puede volver a abrirlo. Muestra una imagen de sí mismo, pero no es una imagen: sigue siendo un objeto cuando lo mueve, cambia su tamaño, lo copia o guarda el libro.

1. Haga clic en **Insertar** y después en **Objeto**, y elija **Desde un rango de JeffCrunch**, **Desde un gráfico de JeffCrunch** o **Desde una página de JeffWrite**.
2. Elija el archivo, y el rango o el número de página si el cuadro lo pide. Si lo copió en la otra aplicación, el cuadro ofrece usarlo.
3. El objeto se coloca en la celda seleccionada. Arrástrelo para moverlo y arrastre un controlador para cambiar su tamaño. Un rango, gráfico o página al que cambia el tamaño se dibuja de nuevo con su nuevo tamaño, para que se vea nítido. Pulse **Supr** para quitarlo. Cada uno es un solo paso de Deshacer.

Para insertar lo que copió en otra aplicación, haga clic en la flecha situada bajo **Pegar** y elija **Como objeto**.

**Haga doble clic** en un objeto para abrirlo en la aplicación que lo creó. Guarde allí, y el objeto de aquí muestra el cambio como un paso de Deshacer. Una celda en la que estaba escribiendo se deja como está. Una copia que pegue es un objeto propio: cambiar una no cambia la otra. Lo mismo ocurre con una hoja que copie: sus objetos son copias, y la copia de la hoja los conserva.

**Archivo**, después **Información**, después **Objetos** enumera todos los objetos del libro. Seleccione uno para abrirlo, convertirlo en una imagen sencilla o eliminarlo.

Un objeto creado en otro programa aparece como la imagen que conserva su archivo, sea del tipo que sea. Puede seleccionarlo, moverlo, cambiarle el tamaño, alinearlo con otros objetos y eliminarlo, y se mueve con las celdas como sus propios objetos. No puede editarlo aquí, ponerlo en un grupo ni cambiar su lugar en el orden de los objetos. Se guarda tal como estaba en el archivo, con solo su lugar modificado si lo movió. Un objeto que contenía un archivo .ods funciona de la misma manera. Al guardar como .xlsx o .ods, sus objetos se guardan en el formato estándar que leen otros programas. Cuando se escribe un archivo .ods para un objeto cuya imagen no se puede dibujar, el objeto se conserva sin imagen.

## Diagramas de otros programas

Un **diagrama** es una imagen de cuadros y flechas, como un proceso, que creó otro programa. JeffCrunch muestra un diagrama a partir de la imagen que el archivo conserva junto a él. Haga clic para seleccionarlo y arrástrelo para moverlo. Al guardar el archivo, el diagrama se escribe de nuevo tal como estaba, para que el programa que lo creó pueda seguir modificándolo.

JeffCrunch todavía no puede cambiar lo que hay dentro de un diagrama, y un diagrama no se puede copiar ni poner en un grupo. Si el archivo no conserva ninguna imagen de un diagrama, JeffCrunch muestra un cuadro gris sencillo que indica que es un diagrama.

## Gráficos en archivos .ods

Un libro guardado en formato .ods conserva sus gráficos: de columnas, barras, líneas, áreas, circulares, de anillo y de dispersión, cada uno con sus series y las celdas que leen (en esta hoja u otra), categorías, título, leyenda, ejes (títulos, valor mínimo y máximo, formato de número), etiquetas de datos, colores y lugar. Un archivo .ods creado por otro programa se abre con sus gráficos, dibujados a partir de las celdas que leen. Lo que un gráfico .ods no tiene dónde guardar se omite y se indica al guardar: líneas de tendencia, barras de error, ejes secundarios y los aspectos especiales de un solo punto. Los gráficos de tipos que JeffCrunch no dibuja (bursátiles y de radar) se abren como columnas con sus valores.
