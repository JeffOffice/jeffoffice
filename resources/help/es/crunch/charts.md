# Gráficos, minigráficos, imágenes y formas

<!-- keywords: objeto incrustado, insertar objeto, objeto desde un rango, objeto desde un gráfico, objeto desde una página, pegar como objeto, lista de objetos, editar objeto, botones de campo, botones de gráfico dinámico, grupo, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, seleccionar varios, cuadro de selección, filtros de gráfico, ocultar serie, gráficos recomendados, sugerencias de gráfico, gráfico, diagrama, columna, barra, línea, circular, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, estilos, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, minigráfico, marcadores, nombre, texto alternativo, logarítmico, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, estilos, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella, diagrama, diagramas, imágenes en línea, imágenes gratuitas, openverse, wikimedia commons, licencia, crédito, texto artístico, wordart, deformar, comprimir imágenes, reducir imágenes, tamaño de archivo, ppp, eliminar áreas recortadas, código de barras, código QR, vincular una forma a una celda, la forma muestra una celda, el cuadro de texto muestra una celda, valor de celda en una forma -->

Un gráfico es una imagen de números que permanece vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea graficar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, selecciónelas también. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y como etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Área**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte la forma en que se dibujan las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la ficha **Insertar**. JeffCrunch analiza las celdas y muestra algunos tipos adecuados, del mejor al menos adecuado. Por ejemplo, si las fechas están en la columna lateral, se sugiere una línea, y una sola columna de pocos números positivos con etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch grafica el bloque de celdas que la rodea y que contiene datos.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie por ahora

Seleccione un gráfico. Junto a él aparecen tres pequeños botones. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para dejar esa serie fuera del gráfico. Vuelva a marcarla para que regrese en el mismo lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha dejado fuera sigue en el archivo al guardarlo y permanece oculta al abrirlo, así que no se pierde nada. Vuelva a marcarla para mostrarla. (Un gráfico de anillo muestra todas las series al abrirse).

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadros de su borde para cambiar su tamaño.

- Mantenga pulsada **Alt** mientras arrastra para que un borde quede alineado con el borde de una celda.
- **Ajustar a la cuadrícula** y **Ajustar a la forma** están en la ficha **Formato de forma**, y se conservan para usted en todos los libros. Con **Ajustar a la cuadrícula** activado, un borde que arrastra va al borde más cercano de una celda. Con **Ajustar a la forma** activado, un borde va al borde de otro objeto cuando está a menos de 6 píxeles. Solo se tienen en cuenta los primeros 2000 objetos de la hoja para **Ajustar a la forma**. Con ambas opciones activadas, prevalecen los bordes de las celdas, porque un objeto está anclado a las celdas, y no se miran los bordes de los demás objetos. Un cambio llega a todas las ventanas abiertas a la vez.
- **Alt** invierte el ajuste activo durante un arrastre: con **Ajustar a la cuadrícula** o **Ajustar a la forma** activados, mantener **Alt** desactiva el ajuste solo para ese arrastre (nunca cambia al otro tipo), y con ambos desactivados, **Alt** hace que los bordes vayan a las celdas, como se indicó antes.
- Un objeto girado se ajusta según el contorno que se ve (el recuadro más pequeño que lo contiene), no según su marco antes de girarlo, y lo mismo ocurre con los objetos a los que se ajusta.
- Mantenga pulsada **Mayús** mientras arrastra una esquina para conservar las proporciones.
- Las teclas de dirección lo desplazan unos pocos píxeles. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase breve que explica a una persona que no puede ver el objeto lo que muestra), para escribir su tamaño y para elegir qué hace al insertar, eliminar o cambiar el tamaño de celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o quedarse donde está. La opción se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el archivo también conserva esta opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y en la cinta aparece la ficha **Gráfico**.

- El botón **+** (y **Elementos del gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y permite indicar dónde van la leyenda y las etiquetas.
- El botón de pincel (y **Colores** y **Estilos de gráfico** en la ficha) da al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, definir las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Botones de campo** (en el menú **+**, solo para un gráfico dinámico) muestra pequeños botones con los nombres de los campos de la tabla dinámica que sigue el gráfico: los filtros del informe bajo el título, y las filas, los valores y las columnas en la parte inferior. Active o desactive cada tipo, o elija **Mostrar todo** u **Ocultar todo**. Un gráfico dinámico nuevo los muestra.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas, áreas o dispersión) dibuja una pequeña línea encima y debajo de cada punto para mostrar cuánto podría diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Elija **Personalizado** para usar importes propios: escriba las celdas que contienen los importes sobre cada punto (por ejemplo, `=Hoja1!$D$2:$D$4`) y después las celdas de los importes por debajo. Deje una vacía para dibujar las barras solo en un lado. Las barras cambian cuando cambian esas celdas. Un archivo que tiene sus propios importes para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) tiene **Líneas de caída**, que van desde el punto más alto de cada categoría hasta el eje, **Líneas de máximos y mínimos**, que unen el punto más alto y el más bajo de cada categoría, y **Barras de subida/bajada**, que son recuadros entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximos y mínimos y las barras de subida/bajada requieren dos series o más. Los gráficos de área tienen líneas de caída.
- **Tabla de datos** (en el menú **+**, para un gráfico de columnas, líneas o áreas) coloca una cuadrícula con los números bajo el gráfico, con una columna por cada categoría y una fila por cada serie. Su lista activa y desactiva la tabla, y define las líneas entre filas, las líneas entre columnas, el contorno y las claves de leyenda (el color de cada serie junto a su nombre).
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para que vea hacia dónde se dirigen. Elija **Lineal** para una recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben rápido y luego se estabilizan, **Polinómica** para una curva con inflexiones, **Potencial** o **Media móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R cuadrado** escribe qué tan bien se ajusta la línea (1 es un ajuste perfecto; 0 es ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja frente a un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de a su lado.
- **Etiquetas de datos** también tiene **Etiqueta contiene** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: coma, punto y coma, punto, nueva línea o espacio) y **Formato de número…** (un código de formato, como 0,0, para los números de las etiquetas; déjelo vacío para mostrar los números tal como son). **Valor desde celdas…** muestra el texto de algunas celdas como etiquetas, una celda por cada punto. Escriba las celdas, por ejemplo =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Formato del eje horizontal…** y **Formato del eje vertical…** en la ficha **Gráfico** abren un cuadro para un eje. Permite definir los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1000 para miles, de modo que 30 000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, junto al título del eje), si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que conviene a números de tamaños muy distintos que deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto se giran. Solo un eje numérico tiene límites y unidades.
- **Formato de elemento del gráfico…** en la ficha **Gráfico** (y en el menú contextual del botón derecho) define el relleno y la línea de la página del gráfico, del área de trazado (el recuadro en el que se sitúan las marcas) o de una serie. En una línea, también define si es suavizada y qué marca aparece en cada punto. El menú contextual de un gráfico también tiene **Cambiar filas y columnas** y los dos comandos de ejes.
- **Selección actual** enumera las partes del gráfico: el área del gráfico, el área de trazado y cada serie. Seleccione una y **Formato de selección** abre la ventana de formato en esa parte. **Restablecer para igualar al estilo** quita lo que se configuró en la parte seleccionada y devuelve lo que le da el estilo del gráfico. En el área del gráfico, eso significa su relleno, su línea y el color de texto que establece un estilo (el texto claro del estilo oscuro vuelve al color de texto del tema). En el área de trazado, son su relleno y su línea. En una serie, son su color (el que le asigna la opción Colores o el estilo del gráfico), su contorno, sus marcas tal como las tiene ese tipo de gráfico, y los colores de sus puntos; una línea suavizada se mantiene si el tipo de gráfico es suavizado. No cambia títulos, leyenda, etiquetas, ejes, líneas de cuadrícula ni datos, y no hace nada en una parte que ya coincide. Es un paso de Deshacer.
- **Mover gráfico…** en la ficha **Gráfico** pregunta dónde debe ir el gráfico. Elija **Nueva hoja** y escriba un nombre, y el gráfico obtendrá una hoja de gráfico propia: una hoja que solo contiene el gráfico, sin celdas. Elija **Objeto en** y seleccione una hoja de cálculo, y el gráfico se colocará en esa hoja. Un gráfico de una hoja de gráfico solo puede pasar a una hoja de cálculo, y la hoja de gráfico que deja se elimina. El gráfico sigue leyendo las celdas que leía. En una hoja de cálculo conserva su tamaño y su lugar en la hoja, incluso donde las filas y columnas son más altas o más anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar, salvo que la operación haya eliminado una hoja. Un archivo con formato .ods no puede contener una hoja de gráfico, así que guarde una hoja de gráfico en formato .xlsx.
- **Título del gráfico** y **Títulos de ejes** (en el menú **+**) también tienen **Texto del título…**. Escriba las palabras que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contenga esa celda. Cuando la celda cambia, también cambia el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la ficha **Gráfico** (y en el menú contextual) guarda el gráfico seleccionado como archivo de imagen **PNG** o **JPEG**, con el doble del tamaño que tiene en la hoja para que se vea nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la ficha **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y la leyenda, los ejes y las fuentes. El gráfico al que lo aplica conserva sus propios números y las celdas que lee. Un **Deshacer** quita de nuevo el aspecto.
- **Tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o de una columna de números, justo al lado de ellos.

1. Seleccione los números. Para tener un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra por cada número. **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor que cero, todas del mismo tamaño, lo que conviene a resultados que solo pueden ser buenos o malos.
3. En el cuadro **Crear minigráficos**, marque el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas donde van). Elija una celda de ubicación para cada fila de datos, o para cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo, y comparten un mismo aspecto. Haga clic en una celda con un minigráfico, y aparece la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que seleccionó. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se deben leer las celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** da al grupo un conjunto de colores. **Color del minigráfico** define el color de la línea o de las barras, y el **Grosor** (qué tan gruesa es la línea, en puntos). **Color de marcador** define el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y define los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da a todo el grupo una misma escala, para que pueda compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se sitúa más a la derecha. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** quita los minigráficos seleccionados, o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila sobre los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la ficha **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y se coloca en la hoja, en la celda seleccionada. Una imagen de más de 480 puntos de ancho se reduce para caber y conserva su forma. El libro guarda la imagen tal cual. Una imagen de un tipo que el libro no puede conservar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen normal hecha de puntos. Un archivo que no es una imagen, o que pesa más de 16 MB, se rechaza con un mensaje. Un archivo WMF o EMF (los tipos de imagen que algunos programas antiguos crean a partir de líneas y formas) se dibuja y se guarda también como PNG.

**Iconos** muestra la misma ventana de pequeñas imágenes de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlas por nombre. Elija uno o varios, y cada uno se coloca en la hoja como una imagen de 48 puntos de ancho y de alto. Se dibuja en gris oscuro, con un tamaño que se mantiene nítido al agrandarlo. Cada uno es un solo paso que puede deshacer.

**Código de barras** abre un menú corto con dos opciones. **Código de barras** abre una ventana para crear un solo código de barras (**Códigos de barras desde celdas**, más abajo, crea muchos): ISBN de libro (con o sin precio), EAN-13, UPC-A, EAN-8, Código 128, Código 39 o un código QR, que puede contener cualquier texto, como una dirección web. Elija un **Tipo**, escriba el texto o los dígitos, y la ventana muestra el código de barras mientras escribe, y explica en rojo qué falla si no puede crearse. Pulse **Insertar** y se coloca en la hoja como una imagen, en un paso que puede deshacer. Muévala y cambie su tamaño como cualquier imagen. Para modificarlo, selecciónelo y pulse de nuevo **Código de barras**, cambie el texto y pulse **Actualizar**; el nuevo código ocupa el lugar y el ancho del anterior. Al guardar, se guarda como imagen con un SVG que se mantiene nítido, de modo que cualquier programa lo muestra, y JeffCrunch conserva sus ajustes en el archivo, así que sigue siendo un código de barras que puede editar cuando vuelve a abrir el libro en JeffCrunch.

**Código de barras > Códigos de barras desde celdas** (en el mismo menú **Código de barras**) crea un código de barras para cada celda con contenido de las celdas seleccionadas, por ejemplo para una hoja de etiquetas. Seleccione las celdas, pulse esta opción y después elija el tipo y el aspecto en la misma ventana. Lo que escriba allí es solo una muestra, porque el texto de cada celda se convierte en su código de barras. Los códigos se colocan en una columna justo a la derecha de las celdas seleccionadas, uno debajo de otro, como un solo paso que puede deshacer. Una celda cuyo texto el código no admite (una letra en un EAN-13, por ejemplo) se indica en un mensaje y no recibe código de barras. Se procesan como máximo 500 celdas a la vez; seleccione menos si indica que hay demasiadas. Cada código de barras recuerda la celda de la que procede y la sigue: cuando cambia esa celda, el código se vuelve a crear a partir del nuevo texto, en el mismo lugar y con el mismo tamaño, como un paso propio que puede deshacer. El vínculo se mueve con la celda cuando inserta o elimina filas o columnas. Si elimina la celda, el código de barras conserva el último texto que tenía. Al guardar, la imagen del archivo se crea a partir del texto actual de la celda, para que otros programas también muestren el valor vigente.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y se coloca en la hoja con un azul claro y un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, inserta un cuadro vacío sin relleno ni borde. Pulse **Dibujar cuadro de texto** (el botón principal, o la primera opción de su menú) y arrastre sobre la hoja para dibujar el cuadro. Al arrastrar, el cuadro tiene el tamaño que arrastre, pero nunca menos de 18 puntos (0,6 cm) por lado, y un clic de menos de 4 puntos da el tamaño habitual. Mantenga pulsada **Mayús** para que el cuadro quede cuadrado. El puntero es una cruz hasta que suelte el botón; **Esc** u otro comando desactiva la herramienta. El cuadro es un paso que puede deshacer.

Para girar las palabras de un cuadro de texto, selecciónelo y pulse **Dirección del texto** en la ficha **Formato de forma**: **Horizontal**, **Girar todo el texto 90°** o **Girar todo el texto 270°**. Las palabras se escriben giradas, tal como se muestran, y un libro guardado en formato .xlsx u .ods conserva la dirección.

Con una forma seleccionada, **Estilos** en la ficha **Formato de forma** le da un aspecto predefinido: un relleno y un contorno que combinan, como azul claro con un borde azul más oscuro. Los dos últimos conservan solo el contorno o solo el relleno. Un clic es un paso que puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

## Buscar imágenes gratuitas en línea

**Imágenes en línea**, en el grupo **Ilustraciones** de la ficha **Insertar**, busca imágenes gratuitas. Proceden de dos bibliotecas abiertas: Openverse, que busca en muchas colecciones, y Wikimedia Commons, la biblioteca de las imágenes de Wikipedia.

1. Pulse **Imágenes en línea…**. Se abre un panel a la derecha.
2. Elija una biblioteca, escriba lo que busca y pulse **Buscar** (o pulse Intro).
3. Haga clic en una imagen para ver quién la hizo y qué **licencia** tiene. Una **licencia** indica cómo puede usar una imagen. Haga clic en el nombre de la licencia para leerla.
4. Pulse **Insertar** (o haga doble clic en la imagen).

**Qué envía JeffCrunch.** Las palabras de búsqueda se envían a la biblioteca que elija, y solo cuando pulsa **Buscar** o Intro. Después JeffCrunch pide a la biblioteca las vistas previas pequeñas de lo que encontró, y la imagen que inserte. Como cualquier página web, la biblioteca puede ver la dirección de internet de su equipo, y se le informa de que la pide JeffCrunch. No se envía nada de su libro.

La imagen se coloca en la hoja como una imagen de archivo, y el libro la conserva. Su título se guarda como **texto alternativo**. La mayoría de las licencias piden que se nombre al autor. Si está marcada la opción **Agregar un crédito debajo de la imagen**, JeffCrunch añade un pequeño cuadro de texto bajo la imagen con su título, su autor y su licencia. Puede moverlo, cambiarlo o eliminarlo. La imagen y su crédito se insertan en un solo paso, así que **Deshacer** los quita a ambos. Compruebe que la licencia se ajusta a cómo va a usar la imagen.

## Agregar texto artístico

El **texto artístico** son palabras decoradas: letras con color, contorno, sombra o una forma curvada.

1. Pulse **Insertar > Texto > Texto artístico**. Se abre una galería de estilos.
2. Pulse un estilo. Las palabras «Escriba aquí el texto» se colocan en la hoja con ese estilo.
3. Para cambiar las palabras, seleccione el texto artístico y pulse **Editar texto…** en la ficha **Formato de forma**. Un cuadro pide las palabras, la fuente, el tamaño, la negrita y la cursiva. Un tamaño mayor hace más grande el texto artístico.
4. En la ficha **Formato de forma**, **Estilos de texto artístico** da a las letras otro estilo, y **Deformar texto** las curva en una forma, como una onda o un arco. «Sin forma» las mantiene rectas.

El texto artístico es una forma del dibujo de la hoja, con su deformación, su relleno y su contorno, así que conserva su aspecto al guardar el libro en formato .xlsx y volver a abrirlo, en JeffCrunch o en otro programa. Cada cambio es un paso que puede deshacer. Un libro guardado en formato .ods conserva también sus imágenes, formas, cuadros de texto, grupos, líneas y texto artístico. El texto artístico se guarda como la forma de texto curvo que tiene OpenDocument, con sus palabras, fuente, relleno, contorno y deformación, y vuelve a ser texto artístico al abrir de nuevo el archivo .ods. Algunas deformaciones que OpenDocument no tiene (Onda 2, Onda 4 y las formas inferior y superior de Inflar y Desinflar) se guardan como palabras rectas con las mismas letras y el mismo estilo. Los gráficos también se conservan en un archivo .ods: vea más abajo.

## Reducir el tamaño de las imágenes

Una imagen tomada con una cámara puede ser mucho mayor de lo que se muestra en la hoja, y hace que el archivo pese mucho. Pulse una imagen y después **Comprimir imágenes…** en el grupo **Ajustar** de la ficha **Formato de imagen**. Elija dónde se usará el libro: **Impresión comercial (300 ppp)**, **Impresión de escritorio (220 ppp)** o **Web (96 ppp)**. Deje marcada **Aplicar a todas las imágenes de esta hoja** para reducir todas las imágenes de la hoja, o desmárquela para reducir solo la imagen seleccionada. **Eliminar las áreas recortadas de las imágenes** también quita las partes ocultas que recortó, y el recorte desaparece. Una imagen solo se reemplaza cuando la nueva es más pequeña. Su tamaño, su lugar, su nombre y su texto alternativo se mantienen. **Deshacer** (Ctrl+Z) devuelve las imágenes originales, y el libro guardado contiene solo las imágenes más pequeñas. Las imágenes de otras hojas no cambian: vaya a cada hoja y repita la operación. Una imagen que es un dibujo (SVG, EMF o WMF), o una animación, se deja tal cual.

Cada una es un solo paso que puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiarle el tamaño, llevarlo adelante o atrás, darle un nombre y texto alternativo en **Propiedades**, o pulsar **Suprimir**.

## Mostrar el texto de una celda en una forma o en un cuadro de texto

Una forma o un cuadro de texto puede mostrar lo que muestra una celda, y seguirla. Seleccione la forma, haga clic en la **barra de fórmulas**, escriba un signo igual y una celda, y pulse **Intro**.

- `=B2` vincula la forma a B2 de esta hoja. `=Hoja2!B3` la vincula a una celda de otra hoja. `=Total` la vincula a un nombre que representa una sola celda.
- La forma muestra entonces el texto de la celda tal como lo muestra la celda, con su formato de número. Cuando cambia la celda, su formato o una fórmula de la que depende, la forma también cambia. El texto más largo de lo que cabe en una celda (32 767 caracteres) se corta. Todas las formas vinculadas de un libro juntas muestran como máximo 4 MB de texto: a partir de ahí, una forma muestra su texto cortado.
- Con la forma seleccionada, la barra de fórmulas muestra su vínculo, como `=Hoja1!$B$2`. Borre la barra de fórmulas y pulse **Intro** para quitar el vínculo. La forma conserva entonces las palabras que muestra.
- Escribir palabras en la forma también quita el vínculo.
- Un vínculo es una sola celda. Un rango como `=A1:B2`, una lista, un cálculo o un nombre que no es una celda se rechazan con un mensaje, y la forma conserva su vínculo.
- Si se elimina la fila o la columna de la celda, o su hoja, la forma muestra `#¡REF!` y conserva el vínculo, como lo haría una fórmula. Deshacer devuelve la celda.
- Cada vínculo, y cada vez que quita uno, es un paso que puede deshacer.

El vínculo se guarda en un archivo .xlsx junto con el texto que mostraba la celda, así que un programa que no sigue vínculos sigue mostrando las palabras, y JeffCrunch abre el archivo con el vínculo funcionando. OpenDocument no tiene forma de vincular un cuadro de texto a una celda, así que un libro guardado en formato .ods conserva las palabras que la forma muestra en ese momento, y no el vínculo.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparece la ficha **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparece la ficha **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su lugar y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** define el grosor (qué tan gruesa es la línea) y los guiones (continua, discontinua o de puntos). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta con forma de ventana: cuadrada, 4 de ancho por 3 de alto, 3 de ancho por 4 de alto, 16 por 9 o 9 por 16. El recorte se hace de forma uniforme desde el centro de lo que se ve, y la imagen se agranda hasta el tamaño de lo que queda. **Mostrar la imagen completa** devuelve lo que se recortó. **Rellenar** conserva el marco y recorta la imagen para que cubra todo el marco. **Ajustar** muestra la imagen completa, y el marco adopta la forma de la imagen dentro del marco anterior. **Recortar a forma** (la galería junto a **Recortar**) corta la imagen con una forma como óvalo, estrella o rectángulo redondeado; la forma se guarda con la imagen. El archivo de imagen no cambia: la parte recortada solo queda oculta.
- **Editar texto** es para formas y cuadros de texto. Escriba las palabras y elija cómo se sitúan horizontal y verticalmente, el tamaño, la negrita o normal, y si se ajustan a la forma o se mantienen en una sola línea. Para un texto artístico, pide las palabras, la fuente, el tamaño, la negrita y la cursiva.
- **Panel de selección** abre una lista de todos los objetos de la hoja por su nombre, con el que está delante en primer lugar. Haga clic en un nombre para seleccionar ese objeto, lo que resulta útil cuando los objetos se tapan entre sí. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** define el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o agrandan las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la primera. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y después mantenga pulsada Mayús o Ctrl y haga clic en otro para seleccionarlo también. Haga clic de nuevo en un objeto seleccionado con Mayús o Ctrl para quitarlo de la selección. Los objetos seleccionados tienen cada uno un marco discontinuo. También puede elegir **Seleccionar objetos** en la ficha **Formato de forma** y arrastrar un cuadro sobre celdas vacías: se seleccionan todos los objetos que caben por completo dentro del cuadro. Pulse Esc, o elija **Seleccionar objetos** de nuevo, para volver a seleccionar celdas. Pulse Suprimir para eliminar de una vez todos los objetos seleccionados.

Con dos o más seleccionados, la ficha **Formato de forma** tiene estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el archivo lo conserva como grupo. Un gráfico no se puede agrupar.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su lugar en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Centrar**, **Alinear a la derecha**, **Alinear arriba**, **Centrar verticalmente** y **Alinear abajo**. Se alinean con el borde o con el centro del conjunto completo. **Distribuir horizontalmente** y **Distribuir verticalmente** espacian de manera uniforme tres objetos o más entre los dos de los extremos.

Agrupar, Desagrupar y cada opción de Alinear son un paso que puede deshacer.

## Darle un nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. Deshacer restaura el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Pulse **Suprimir** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, así que un gráfico que cree aquí se abre allí de la misma manera. Un gráfico que procede de otro programa con algo que JeffCrunch no puede conservar al reescribirlo (barras de error, un gráfico tridimensional) se puede mover, cambiar de tamaño y eliminar, pero sus elementos y datos no se modifican aquí.

## Insertar un objeto

Un **objeto** guarda su propio archivo dentro del libro, para que la aplicación que lo creó pueda volver a abrirlo. Muestra una imagen de sí mismo, pero no es una imagen: sigue siendo un objeto cuando lo mueve, cambia su tamaño, lo copia o guarda el libro.

1. Pulse **Insertar**, después **Objeto**, y elija **Desde un rango de JeffCrunch**, **Desde un gráfico de JeffCrunch** o **Desde una página de JeffWrite**.
2. Elija el archivo y el rango o el número de página si el cuadro lo pide. Si lo copió en la otra aplicación, el cuadro ofrece usarlo.
3. El objeto se coloca en la celda seleccionada. Arrástrelo para moverlo, y arrastre un controlador para cambiar su tamaño. Un rango, gráfico o página al que cambie el tamaño se vuelve a dibujar con su nuevo tamaño, para que siga nítido. Pulse **Suprimir** para quitarlo. Cada uno es un solo paso de Deshacer.

Para insertar lo que copió en otra aplicación, pulse la flecha bajo **Pegar** y elija **Como objeto**.

**Haga doble clic** en un objeto para abrirlo en la aplicación que lo creó. Guárdelo allí, y el objeto de aquí muestra el cambio como un paso de Deshacer. Una celda en la que estaba escribiendo se deja como está. Una copia que pegue es un objeto propio: cambiar uno no cambia el otro. Lo mismo ocurre con una hoja que copie: sus objetos son copias, y la copia de la hoja los conserva.

**Archivo**, después **Información**, después **Objetos** enumera todos los objetos del libro. Seleccione uno para abrirlo, convertirlo en una imagen normal o eliminarlo.

Un objeto creado en otro programa se muestra como la imagen que conserva su archivo, sea cual sea el tipo de imagen. Puede seleccionarlo, moverlo, cambiarle el tamaño, alinearlo con otros objetos y eliminarlo, y se mueve con las celdas como sus propios objetos. No puede editarlo aquí, agruparlo ni cambiar su lugar en el orden de los objetos. Se guarda tal como estaba en el archivo, solo con el cambio de lugar si lo movió. Un objeto que contenía un archivo .ods funciona de la misma manera. Guardado en formato .xlsx o .ods, sus objetos están en el formato estándar que leen otros programas. Cuando se escribe un archivo .ods para un objeto cuya imagen no se puede dibujar, el objeto se conserva sin imagen.

## Diagramas de otros programas

Un **diagrama** es una imagen de cuadros y flechas, como un proceso, que creó otro programa. JeffCrunch muestra un diagrama a partir de la imagen que el archivo conserva junto a él. Haga clic en él para seleccionarlo, y arrástrelo para moverlo. Al guardar el archivo, el diagrama se escribe de nuevo tal como era, para que el programa que lo creó pueda seguir modificándolo.

JeffCrunch todavía no puede cambiar lo que hay dentro de un diagrama, y un diagrama no se puede copiar ni agrupar. Si el archivo no conserva una imagen de un diagrama, JeffCrunch muestra un cuadro gris sencillo que indica que es un diagrama.

## Gráficos en archivos .ods

Un libro guardado en formato .ods conserva sus gráficos: de columnas, barras, líneas, áreas, circulares, de anillo y de dispersión, cada uno con sus series y las celdas que leen (en esta hoja o en otra), categorías, título, leyenda, ejes (títulos, valor mínimo y máximo, formato de número), etiquetas de datos, colores y lugar. Un archivo .ods creado por otro programa se abre con sus gráficos, dibujados a partir de las celdas que leen. Lo que un gráfico .ods no tiene dónde guardar se omite y se indica al guardar: líneas de tendencia, barras de error, ejes secundarios y los aspectos especiales de un solo punto. Los gráficos de tipos que JeffCrunch no dibuja (bursátiles y de radar) se abren como columnas de sus valores.
