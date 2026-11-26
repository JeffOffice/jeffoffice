# Gráficos, minigráficos, imágenes y formas

<!-- keywords: objeto incrustado, insertar objeto, objeto desde un rango, objeto desde un gráfico, objeto desde una página, pegar como objeto, lista de objetos, editar objeto, botones de campo, botones de gráfico dinámico, grupo, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, seleccionar varios, cuadro de selección, filtros de gráfico, ocultar serie, gráficos recomendados, sugerencias de gráfico, gráfico, diagrama, columna, barra, línea, circular, áreas, dispersión, combinado, radar, radar relleno, gráfico de araña, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, estilos, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, victorias y derrotas, minigráfico pequeño, marcadores, nombre, texto alternativo, logarítmica, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, estilos, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella, diagrama, diagramas, imágenes en línea, imágenes gratuitas, openverse, wikimedia commons, licencia, crédito, texto artístico, wordart, deformar, comprimir imágenes, reducir imágenes, tamaño de archivo, ppp, eliminar áreas recortadas, código de barras, código QR, vincular una forma a una celda, forma que muestra una celda, cuadro de texto que muestra una celda, valor de celda en una forma, tamaño de fichero -->

Un gráfico es una imagen de números que sigue vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, selecciónelas también. JeffCrunch mira la primera fila y la primera columna y las usa como nombres y como etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Áreas**, **Dispersión**, **Combinado** o **Radar**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores, un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas, o un gráfico de radar simple, con marcadores o relleno. Un gráfico de radar dibuja cada etiqueta como un radio que sale del centro y cada serie como una forma cerrada que pasa por sus números en los radios, así que muestra varias medidas de una misma cosa una al lado de otra.
3. Para ver antes cada tipo, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Intercambiar filas y columnas** invierte el sentido de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la ficha **Insertar**. JeffCrunch analiza las celdas y muestra algunos tipos que les convienen, del mejor al menos adecuado. Por ejemplo, si las fechas están en vertical, sugiere una línea; y una sola columna de unos pocos números positivos con etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que tiene contenido.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie por ahora

Elija un gráfico. Junto a él aparecen tres pequeños botones. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para quitar esa serie del gráfico. Vuelva a marcarla para que regrese al mismo lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha quitado sigue en el fichero al guardarlo y permanece oculta al abrirlo, así que no se pierde nada. Vuelva a marcarla para mostrarla. (Un gráfico de anillo muestra todas las series al abrirse).

## Mover y cambiar el tamaño

Haga clic en un gráfico, imagen o forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiar su tamaño.

- Mantenga pulsada **Alt** mientras arrastra para que un borde quede alineado con el borde de una celda.
- **Ajustar a la cuadrícula** y **Ajustar a la forma** están en la ficha **Formato de forma**, y se conservan para usted en todos los libros. Con **Ajustar a la cuadrícula** activado, un borde que arrastra va al borde más cercano de una celda. Con **Ajustar a la forma** activado, un borde va al borde de otro objeto cuando está a menos de 6 píxeles. Solo se tienen en cuenta los primeros 2000 objetos de la hoja para **Ajustar a la forma**. Con ambas opciones activadas, prevalecen los bordes de las celdas, porque un objeto está anclado a las celdas, y no se miran los bordes de los demás objetos. Un cambio llega a todas las ventanas abiertas a la vez.
- **Alt** invierte el ajuste activo para un arrastre: con **Ajustar a la cuadrícula** o **Ajustar a la forma** activados, mantener **Alt** desactiva el ajuste para ese arrastre (nunca cambia al otro tipo), y con ambos desactivados, **Alt** hace que los bordes vayan a las celdas, como se ha explicado.
- Un objeto girado se ajusta según el contorno que se ve (el recuadro más pequeño que lo contiene), no según su marco antes de girarlo, y lo mismo ocurre con los objetos a los que se ajusta.
- Mantenga pulsada **Mayús** mientras arrastra una esquina para conservar las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo desplaza un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva hasta el siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su sitio.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase breve que explique a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué ocurre cuando inserta, elimina o cambia el tamaño de celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o quedarse donde está. La elección se guarda en el fichero. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el fichero también guarda esa elección.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y aparece una ficha **Gráfico** en la cinta de opciones.

- El botón **+** (y **Elementos del gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y dice dónde van la leyenda y las etiquetas.
- El botón del pincel (y **Colores** y **Estilos** en la ficha) da al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Botones de campo** (en el menú **+**, solo para un gráfico dinámico) muestra pequeños botones con los nombres de los campos de la tabla dinámica que sigue el gráfico: los filtros del informe bajo el título, y las filas, los valores y las columnas a lo largo de la parte inferior. Active o desactive cada tipo, o elija **Mostrar todo** u **Ocultar todo**. Un gráfico dinámico nuevo los muestra.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas, áreas o dispersión) dibuja una pequeña línea encima y debajo de cada punto para mostrar cuánto podría diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Elija **Personalizado** para usar importes propios: escriba las celdas que contienen los importes encima de cada punto (por ejemplo `=Hoja1!$D$2:$D$4`) y después las celdas de los importes debajo. Deje una vacía para dibujar las barras de un solo lado. Las barras cambian cuando cambian esas celdas. Un fichero que tiene sus propios importes para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) tiene **Líneas de caída**, que bajan desde el punto más alto de cada categoría hasta el eje; **Líneas de máximos y mínimos**, que unen el punto más alto y el más bajo de cada categoría; y **Barras de subida/bajada**, que son recuadros entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximos y mínimos y las barras de subida/bajada necesitan dos series o más. Los gráficos de áreas tienen líneas de caída.
- **Tabla de datos** (en el menú **+**, para un gráfico de columnas, líneas o áreas) pone una cuadrícula con los números debajo del gráfico, con una columna para cada categoría y una fila para cada serie. Su lista activa y desactiva la tabla, y define las líneas entre filas, las líneas entre columnas, el contorno y las claves de leyenda (el color de cada serie junto a su nombre).
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para ver hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben deprisa y luego se estabilizan, **Polinómica** para una curva con curvas, **Potencial**, o **Media móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R cuadrado** escribe qué tan bien se ajusta la línea (1 es un ajuste perfecto; 0 es ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja frente a un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de al lado.
- **Etiquetas de datos** también tiene **Contiene la etiqueta** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: una coma, un punto y coma, un punto, un salto de línea o un espacio) y **Formato de número…** (un código de formato como 0,0 para los números de las etiquetas; déjelo vacío para mostrar los números tal como son). **Valor desde celdas…** muestra el texto de algunas celdas como etiquetas, una celda para cada punto. Escriba las celdas, por ejemplo =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Formato del eje horizontal…** y **Formato del eje vertical…** en la ficha **Gráfico** abren un cuadro para un solo eje. Define los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1000 para miles, de modo que 30 000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, con el título del eje), si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que conviene a números de tamaños muy distintos que deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto están giradas. Solo un eje numérico tiene límites y unidades.
- **Formato del elemento del gráfico…** en la ficha **Gráfico** (y en el menú del botón derecho) define el relleno y la línea de la página del gráfico, del área de trazado (el recuadro en el que se sitúan las marcas) o de una serie. En una línea, también define si es suavizada y qué marca aparece en cada punto. El menú del botón derecho de un gráfico tiene además **Intercambiar filas y columnas** y los dos comandos de ejes.
- **Selección actual** enumera las partes del gráfico: el área del gráfico, el área de trazado y cada serie. Elija una y **Formato de selección** abre la ventana de formato en esa parte. **Restablecer para igualar al estilo** quita lo que se configuró en la parte elegida y devuelve lo que le da el estilo del gráfico. Para el área del gráfico, eso significa su relleno, su línea y el color del texto que haya fijado un estilo (el texto claro del estilo oscuro vuelve al color de texto del tema). Para el área de trazado, son su relleno y su línea. Para una serie, son su color (el que le da la opción Colores o el estilo del gráfico), su contorno, sus marcas tal como las tiene ese tipo de gráfico y los colores de sus puntos; una línea suavizada se conserva si el tipo de gráfico es suavizado. No cambia títulos, leyenda, etiquetas, ejes, líneas de cuadrícula ni datos, y no hace nada con una parte que ya coincide. Es un paso de Deshacer.
- **Mover gráfico…** en la ficha **Gráfico** pregunta dónde debe ir el gráfico. Elija **Hoja nueva** y escriba un nombre, y el gráfico tendrá una hoja de gráfico propia: una hoja formada solo por el gráfico, sin celdas. Elija **Objeto en** y seleccione una hoja de cálculo, y el gráfico irá a esa hoja. Un gráfico en una hoja de gráfico solo puede pasar a una hoja de cálculo, y la hoja de gráfico que deja se elimina. El gráfico sigue leyendo las celdas que leía. En una hoja de cálculo conserva su tamaño y su lugar en la hoja, incluso donde las filas y columnas son más altas o más anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar, salvo que el cambio haya eliminado una hoja. Un fichero en formato `.ods` no puede contener una hoja de gráfico, así que guarde una hoja de gráfico en formato `.xlsx`.
- **Título del gráfico** y **Títulos de ejes** (en el menú **+**) también tienen **Texto del título…**. Escriba las palabras que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contiene esa celda. Cuando la celda cambia, también cambia el título. El fichero conserva el vínculo.
- **Guardar como imagen…** en la ficha **Gráfico** (y en el menú del botón derecho) guarda el gráfico seleccionado como un fichero de imagen **PNG** o **JPEG**, con el doble del tamaño que tiene en la hoja para que quede nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la ficha **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un fichero **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y de la leyenda, los ejes y las fuentes. El gráfico al que se aplica conserva sus propios números y las celdas que lee. Un **Deshacer** quita de nuevo el aspecto.
- **Tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o de una columna de números, justo al lado de ellos.

1. Seleccione los números. Para un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Victorias/derrotas**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra por cada número. **Victorias/derrotas** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor que cero, todas del mismo tamaño, lo que conviene a resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, marque el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas en las que van). Elija una celda de ubicación para cada fila de datos, o para cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten un mismo aspecto. Haga clic en una celda con un minigráfico y aparece la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que ha seleccionado. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se leen celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y victorias/derrotas.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** da al grupo un conjunto de colores. **Color del minigráfico** fija el color de la línea o de las barras, y el **Grosor** (qué grosa es la línea, en puntos). **Color de marcador** fija el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y fija los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da al grupo una sola escala, para compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se sitúa más adelante. Déjelo vacío para volver a espaciar los puntos por igual.
- **Agrupar** coloca los minigráficos seleccionados en un solo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** quita los minigráficos seleccionados, o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el fichero en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la ficha **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de ficheros. Elija un fichero de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y se colocará en la hoja, en la celda seleccionada. Una imagen de más de 480 píxeles de ancho se reduce para encajar y conserva su forma. El libro guarda la imagen en sí. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen simple formada por puntos. Un fichero que no es una imagen, o que pesa más de 16 MB, se rechaza con un mensaje. Un fichero WMF o EMF (los tipos de imagen que algunos programas antiguos crean con líneas y formas) se dibuja y también se guarda como PNG.

**Iconos** muestra la misma ventana de pequeños dibujos de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlos por nombre. Elija uno o varios, y cada uno se colocará en la hoja como una imagen de 48 puntos de ancho y alto. Se dibuja en gris oscuro, con un tamaño que se mantiene nítido al agrandarlo. Cada uno es un solo paso que puede deshacer.

**Código de barras** abre un menú corto con dos opciones. **Código de barras** abre una ventana para crear un solo código de barras (**Códigos de barras desde celdas**, más abajo, crea muchos): ISBN de libro (con o sin precio), EAN-13, UPC-A, EAN-8, Code 128, Code 39, o un código QR, que puede contener cualquier texto, como una dirección web. Elija un **Tipo**, escriba el texto o los dígitos, y la ventana muestra el código de barras mientras escribe, y dice en rojo qué falla si no puede crearse. Haga clic en **Insertar** y se colocará en la hoja como una imagen, en un paso que puede deshacer. Muévalo y cambie su tamaño como cualquier imagen. Para cambiarlo, selecciónelo y haga clic de nuevo en **Código de barras**, cambie el texto y haga clic en **Actualizar**; el nuevo código ocupa el lugar y el ancho del anterior. Al guardar, se guarda como una imagen con un SVG que se mantiene nítido, para que todos los programas lo muestren, y JeffCrunch conserva sus ajustes en el fichero, así que sigue siendo un código de barras que puede editar al volver a abrir el libro en JeffCrunch.

**Código de barras > Códigos de barras desde celdas** (en el mismo menú **Código de barras**) crea un código de barras para cada celda con contenido de las celdas seleccionadas, para una hoja de etiquetas. Seleccione las celdas, haga clic en él y luego elija el tipo y el aspecto en la misma ventana. Lo que escriba allí es solo una muestra, porque el texto de cada celda se convierte en su código de barras. Los códigos de barras van en una columna justo a la derecha de las celdas seleccionadas, uno debajo de otro, como un solo paso que puede deshacer. Una celda cuyo texto el código no puede contener (una letra en un EAN-13, por ejemplo) se indica en un mensaje y no recibe código de barras. Se procesan como máximo 500 celdas a la vez; seleccione menos si indica que hay demasiadas. Cada código de barras recuerda la celda de la que procede y la sigue: cuando cambia esa celda, el código de barras se vuelve a crear con el nuevo texto, en el mismo lugar y con el mismo tamaño, como un paso propio que puede deshacer. El vínculo se mueve con la celda cuando inserta o elimina filas o columnas. Si elimina la celda, el código de barras conserva el último texto que tenía. Al guardar, la imagen del fichero se crea a partir del texto de la celda tal como está en ese momento, para que otros programas también muestren el valor actual.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de texto y una línea. Elija una y se colocará en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, coloca un cuadro vacío sin relleno y sin borde. Haga clic en **Dibujar cuadro de texto** (el botón principal, o la primera opción de su menú) y luego arrastre sobre la hoja para dibujar el cuadro. Al arrastrar se crea un cuadro del tamaño que arrastre, pero nunca de menos de 18 puntos (0,6 cm) por lado, y un clic de menos de 4 puntos da el tamaño habitual. Mantenga pulsada **Mayús** para que el cuadro sea cuadrado. El puntero es una cruz hasta que suelte el botón; **Esc**, o cualquier otro comando, desactiva la herramienta. El cuadro es un paso que puede deshacer.

Para girar el texto de un cuadro de texto, selecciónelo y haga clic en **Dirección del texto** en la ficha **Formato de forma**: **Horizontal**, **Girar todo el texto 90°** o **Girar todo el texto 270°**. El texto se escribe girado, tal como se muestra, y un libro guardado como fichero .xlsx o .ods conserva la dirección.

Con una forma seleccionada, **Estilos** en la ficha **Formato de forma** le da un aspecto ya preparado: un relleno y un contorno que combinan, como azul claro con un borde azul más oscuro. Las dos últimas opciones conservan solo el contorno o solo el relleno. Un clic es un paso que puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

## Buscar imágenes gratuitas en línea

**Imágenes en línea**, en el grupo **Ilustraciones** de la ficha **Insertar**, encuentra imágenes gratuitas. Proceden de dos bibliotecas abiertas: Openverse, que busca en muchas colecciones, y Wikimedia Commons, la biblioteca de las imágenes de Wikipedia.

1. Haga clic en **Imágenes en línea…**. Se abre un panel a la derecha.
2. Elija una biblioteca, escriba lo que busca y haga clic en **Buscar** (o pulse Intro).
3. Haga clic en una imagen para ver quién la hizo y qué licencia tiene. Una **licencia** indica cómo puede usar una imagen. Haga clic en el nombre de la licencia para leerla.
4. Haga clic en **Insertar** (o haga doble clic en la imagen).

**Qué envía JeffCrunch.** Las palabras de su búsqueda se envían a la biblioteca que elija, y solo cuando hace clic en **Buscar** o pulsa Intro. JeffCrunch pide después a la biblioteca las vistas previas pequeñas de lo que ha encontrado, y la imagen que inserta. Como cualquier página web, la biblioteca puede ver la dirección de internet de su equipo, y se le informa de que JeffCrunch hace la petición. No se envía nada de su libro.

La imagen se coloca en la hoja como una imagen de fichero, y el libro la conserva. Su título se guarda como **texto alternativo**. La mayoría de las licencias piden que se nombre al autor. Cuando está marcada la opción **Agregar un crédito debajo de la imagen**, JeffCrunch añade un pequeño cuadro de texto debajo de la imagen con su título, su autor y su licencia. Puede moverlo, cambiarlo o eliminarlo. La imagen y su crédito se insertan en un solo paso, así que **Deshacer** quita ambos. Compruebe que la licencia se ajusta al uso que va a dar a la imagen.

## Agregar texto artístico

**Texto artístico** son palabras con adornos: letras con color, contorno, sombra o una forma curva.

1. Haga clic en **Insertar > Texto > Texto artístico**. Se abre una galería de estilos.
2. Haga clic en un estilo. Las palabras «Escriba aquí el texto» se colocan en la hoja con ese estilo.
3. Para cambiar las palabras, seleccione el texto artístico y haga clic en **Editar texto…** en la ficha **Formato de forma**. Un cuadro pide las palabras, la fuente, el tamaño, la negrita y la cursiva. Un tamaño mayor hace más grande el texto artístico.
4. En la ficha **Formato de forma**, **Estilos de texto artístico** da a las letras otro estilo, y **Deformar texto** curva las palabras en una forma, como una onda o un arco. «Sin forma» las deja rectas.

El texto artístico es una forma del dibujo de la hoja, con su deformación, su relleno y su contorno, así que conserva su aspecto al guardar el libro como fichero .xlsx y volver a abrirlo, en JeffCrunch o en otro programa. Cada cambio es un paso que puede deshacer. Un libro guardado como fichero .ods conserva también sus imágenes, formas, cuadros de texto, grupos, líneas y texto artístico. El texto artístico se guarda como la forma de texto de ajuste que tiene OpenDocument, con sus palabras, fuente, relleno, contorno y deformación, y vuelve a ser texto artístico al abrir de nuevo el fichero .ods. Algunas deformaciones que OpenDocument no tiene como forma (Onda 2, Onda 4, y las formas inferior y superior de Inflar y Desinflar) se guardan como palabras rectas con las mismas letras y estilo. Los gráficos también se conservan en un fichero .ods: véase más abajo.

## Reducir el tamaño de las imágenes

Una imagen tomada con una cámara puede ser mucho mayor que lo que se ve en la hoja, y hace grande el fichero. Haga clic en una imagen y, después, en **Comprimir imágenes…** en el grupo **Ajustar** de la ficha **Formato de imagen**. Elija dónde se usará el libro: **Impresión comercial (300 ppp)**, **Impresión de escritorio (220 ppp)** o **Web (96 ppp)**. Deje marcada la opción **Aplicar a todas las imágenes de esta hoja** para reducir todas las imágenes de la hoja, o desmárquela para reducir solo la imagen seleccionada. **Eliminar las áreas recortadas de las imágenes** también quita las partes ocultas que recortó, y el recorte desaparece. Una imagen solo se sustituye cuando la nueva es más pequeña. Su tamaño, su lugar, su nombre y su texto alternativo se conservan. Deshacer (Ctrl+Z) devuelve las imágenes, y el libro guardado contiene solo las imágenes más pequeñas. Las imágenes de otras hojas no se cambian: vaya a cada hoja y repita la operación. Una imagen que es un dibujo (SVG, EMF o WMF), o una animación, se deja como está.

Cada operación es un solo paso que puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiar su tamaño, llevarlo delante o detrás, darle un nombre y un texto alternativo en **Propiedades**, o pulsar **Suprimir**.

## Mostrar el texto de una celda en una forma o en un cuadro de texto

Una forma o un cuadro de texto puede mostrar lo que muestra una celda, y seguirla. Seleccione la forma, haga clic en la **barra de fórmulas**, escriba un signo igual y una celda, y pulse **Intro**.

- `=B2` vincula la forma a B2 de esta hoja. `=Hoja2!B3` la vincula a una celda de otra hoja. `=Total` la vincula a un nombre que representa una celda.
- La forma muestra entonces el texto de la celda tal como lo muestra la celda, con su formato numérico. Cuando cambia la celda, su formato o una fórmula de la que depende, la forma cambia también. El texto más largo de lo que puede contener una celda (32 767 caracteres) se corta. Todas las formas vinculadas de un libro juntas muestran como máximo 4 MB de texto: más allá de eso, una forma muestra su texto cortado.
- Con la forma seleccionada, la barra de fórmulas muestra su vínculo, como `=Hoja1!$B$2`. Borre la barra de fórmulas y pulse **Intro** para quitar el vínculo. La forma conserva entonces las palabras que muestra.
- Escribir palabras en la forma también quita el vínculo.
- Un vínculo es una sola celda. Un rango como `=A1:B2`, una lista, un cálculo o un nombre que no sea una celda se rechazan con un mensaje, y la forma conserva su vínculo.
- Si se elimina la fila o la columna de la celda, o su hoja, la forma muestra `#REF!` y conserva el vínculo, como lo haría una fórmula. Deshacer devuelve la celda.
- Cada vínculo, y cada vez que quita uno, es un paso que puede deshacer.

El vínculo se guarda en un fichero .xlsx junto con el texto que mostraba la celda, así que un programa que no sigue vínculos sigue mostrando las palabras, y JeffCrunch abre el fichero con el vínculo funcionando. OpenDocument no tiene forma de vincular un cuadro de texto a una celda, así que un libro guardado como fichero .ods conserva las palabras que muestra la forma en ese momento, y no el vínculo.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparece la ficha **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparece la ficha **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su lugar y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** fija el grosor (qué grueso es el trazo) y los guiones (continua, discontinua o punteada). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta a una forma de ventana: cuadrada, 4 de ancho por 3 de alto, 3 de ancho por 4 de alto, 16 por 9 o 9 por 16. El recorte se hace por igual desde el centro de lo que se ve, y la imagen se agranda todo lo posible con lo que queda. **Mostrar la imagen completa** recupera lo que se recortó. **Rellenar** conserva el marco y recorta la imagen para que lo cubra por completo. **Ajustar** muestra la imagen entera, y el marco adopta la forma de la imagen dentro del anterior. **Recortar a forma** (la galería junto a **Recortar**) corta la imagen a una forma como un óvalo, una estrella o un rectángulo redondeado; la forma se guarda con la imagen. El fichero de la imagen no cambia: la parte recortada solo se oculta.
- **Editar texto** es para formas y cuadros de texto. Escriba las palabras y elija cómo se sitúan a lo ancho y a lo alto, el tamaño, la negrita o no, y si se ajustan a la forma o quedan en una sola línea. Para el texto artístico pide las palabras, la fuente, el tamaño, la negrita y la cursiva.
- **Panel de selección** abre una lista de todos los objetos de la hoja por nombre, con el primero delante. Haga clic en un nombre para seleccionar ese objeto, lo que ayuda cuando los objetos se ocultan unos a otros. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** fija el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o agrandan las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un fichero, no puede copiarse, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que el original. Si lo pega en otro libro, conserva los números que mostraba pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y, después, mantenga pulsada Mayús o Ctrl y haga clic en otro para seleccionarlo también. Haga clic de nuevo en un objeto seleccionado con Mayús o Ctrl para deseleccionarlo. Los objetos seleccionados tienen cada uno un marco discontinuo. También puede elegir **Seleccionar objetos** en la ficha **Formato de forma** y arrastrar un cuadro sobre celdas vacías: se seleccionan todos los objetos que quedan por completo dentro del cuadro. Pulse Esc, o elija **Seleccionar objetos** otra vez, para volver a seleccionar celdas. Pulse Suprimir para eliminar todos los objetos seleccionados en un solo paso.

Con dos o más seleccionados, la ficha **Formato de forma** tiene estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el fichero lo conserva como grupo. Un gráfico no puede agruparse.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su lugar en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Centrar**, **Alinear a la derecha**, **Alinear arriba**, **Alinear al medio** y **Alinear abajo**. Se alinean con el borde o con el centro del conjunto completo. **Distribuir horizontalmente** y **Distribuir verticalmente** reparten tres o más objetos por igual entre los dos de los extremos.

Agrupar, Desagrupar y cada opción de Alinear son un paso que puede deshacer.

## Ponerle nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. Deshacer devuelve el nombre anterior, y el nombre nuevo se guarda en el fichero.

## Eliminarlo

Pulse **Suprimir** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el fichero en un formato que leen otros programas de hojas de cálculo, así que un gráfico creado aquí se abre allí de la misma manera. Un gráfico procedente de otro programa con algo que JeffCrunch no puede conservar al reescribirlo (barras de error, un gráfico tridimensional) puede moverse, cambiar de tamaño y eliminarse, pero sus elementos y sus datos no se modifican aquí.

## Insertar un objeto

Un **objeto** guarda su propio fichero dentro del libro, para que la aplicación que lo creó pueda volver a abrirlo. Muestra una imagen de sí mismo, pero no es una imagen: sigue siendo un objeto cuando lo mueve, cambia su tamaño, lo copia o guarda el libro.

1. Haga clic en **Insertar** y después en **Objeto**, y elija **Desde un rango de JeffCrunch**, **Desde un gráfico de JeffCrunch** o **Desde una página de JeffWrite**.
2. Elija el fichero y el rango o el número de página si el cuadro lo pide. Si lo copió en la otra aplicación, el cuadro ofrece usarlo.
3. El objeto se coloca en la celda seleccionada. Arrástrelo para moverlo, y arrastre un controlador para cambiar su tamaño. Un rango, gráfico o página al que cambie el tamaño se vuelve a dibujar con su nuevo tamaño, para que quede nítido. Pulse **Suprimir** para quitarlo. Cada uno es un solo paso de Deshacer.

Para insertar lo que copió en otra aplicación, haga clic en la flecha situada debajo de **Pegar** y elija **Como objeto**.

**Haga doble clic** en un objeto para abrirlo en la aplicación que lo creó. Guárdelo allí, y el objeto de aquí muestra el cambio como un paso de Deshacer. Una celda en la que esté escribiendo se deja como está. Una copia que pegue es un objeto independiente: cambiar una no cambia la otra. Lo mismo ocurre con una hoja que copie: sus objetos son copias, y la copia de la hoja los conserva.

**Archivo**, después **Información**, después **Objetos** enumera todos los objetos del libro. Seleccione uno para abrirlo, convertirlo en una imagen simple o eliminarlo.

Un objeto creado en otro programa se muestra como la imagen que conserva su fichero, sea cual sea el tipo de imagen. Puede seleccionarlo, moverlo, cambiar su tamaño, alinearlo con otros objetos y eliminarlo, y se mueve con las celdas como sus propios objetos. No puede editarlo aquí, ponerlo en un grupo ni cambiar su lugar en el orden de los objetos. Se guarda tal como estaba en el fichero, solo con su posición cambiada si lo movió. Un objeto que contenía un fichero .ods funciona de la misma manera. Guardado como fichero .xlsx o .ods, sus objetos están en el formato estándar que leen otros programas. Cuando se escribe un fichero .ods para un objeto cuya imagen no puede dibujarse, el objeto se conserva sin imagen.

## Diagramas de otros programas

Un **diagrama** es una imagen de cuadros y flechas, como un proceso, que creó otro programa. JeffCrunch muestra un diagrama a partir de la imagen que el fichero conserva junto a él. Haga clic en él para seleccionarlo, y arrástrelo para moverlo. Al guardar el fichero, el diagrama se escribe de nuevo tal como estaba, para que el programa que lo creó pueda seguir cambiándolo.

JeffCrunch todavía no puede cambiar lo que hay dentro de un diagrama, y un diagrama no puede copiarse ni ponerse en un grupo. Si el fichero no conserva ninguna imagen de un diagrama, JeffCrunch muestra un cuadro gris simple que indica que es un diagrama.

## Gráficos en ficheros .ods

Un libro guardado como fichero .ods conserva sus gráficos: de columnas, barras, líneas, áreas, circulares, de anillo, de dispersión y de radar, cada uno con sus series y las celdas que leen (en esta hoja o en otra), categorías, título, leyenda, ejes (títulos, valor mínimo y máximo, formato de número), etiquetas de datos, colores y lugar. Un fichero .ods creado por otro programa se abre con sus gráficos, dibujados a partir de las celdas que leen. Lo que un gráfico .ods no tiene dónde guardar se omite y se indica al guardar: líneas de tendencia, barras de error, ejes secundarios y los aspectos adicionales de un solo punto. Los gráficos de tipos que JeffCrunch no dibuja (bursátiles y de radar) se abren como columnas de sus valores.
