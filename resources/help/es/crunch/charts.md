# Gráficos, minigráficos, imágenes y formas

<!-- keywords: objeto incrustado, insertar objeto, objeto desde un rango, objeto desde un gráfico, objeto desde una página, pegar como objeto, lista de objetos, editar objeto, botones de campo, botones de gráfico dinámico, agrupar, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, seleccionar varios, cuadro de selección, filtros de gráfico, ocultar serie, gráficos recomendados, sugerencias de gráfico, gráfico, columna, barra, línea, circular, áreas, dispersión, combinado, radar, radar relleno, gráfico de araña, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, estilos, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, minigráfico, marcadores, nombre, texto alternativo, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella, diagrama, diagramas, imágenes en línea, imágenes libres, openverse, wikimedia commons, licencia, crédito, texto artístico, wordart, deformar, comprimir imágenes, reducir imágenes, tamaño de archivo, ppi, eliminar áreas recortadas, código de barras, código QR, vincular una forma a una celda, la forma muestra una celda, el cuadro de texto muestra una celda, valor de celda en una forma -->

Un gráfico es una imagen de números que sigue vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que quiere representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, selecciónelas también. JeffCrunch mira la primera fila y la primera columna y las usa como nombres y etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la pestaña **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Áreas**, **Dispersión**, **Combinado**, **Radar** o **Bolsa**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores, un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas, o un gráfico de radar simple, con marcadores o relleno. Un gráfico de radar dibuja cada etiqueta como un radio que sale del centro y cada serie como una forma cerrada que pasa por sus números en los radios, de modo que muestra varias medidas de una misma cosa una al lado de la otra.
   Un gráfico de **bolsa** muestra precios: una línea para cada categoría desde su precio más alto hasta el más bajo, con un pequeño punto para el cierre o, cuando hay apertura, una barra entre la apertura y el cierre (clara si sube y oscura si baja). El gráfico se hace con sus columnas tal como están, de izquierda a derecha, y nada comprueba que los números correspondan a su posición, así que colóquelos en este orden: máximo, mínimo y cierre; apertura, máximo, mínimo y cierre; volumen, máximo, mínimo y cierre; o volumen, apertura, máximo, mínimo y cierre. Si las columnas están en otro orden, el gráfico se dibuja igualmente, pero las marcas significan otra cosa (el punto de cierre va en la tercera columna de precios, y la caja va desde la primera columna de precios hasta la última). Con volumen, el volumen se lee en el eje izquierdo y los precios en un segundo eje, a la derecha. Si el número de columnas no corresponde al tipo, JeffOffice indica cuántas necesita y no crea ningún gráfico.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar entre filas y columnas** invierte la orientación de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la pestaña **Insertar**. JeffCrunch analiza las celdas y muestra algunos tipos adecuados, del mejor al menos adecuado. Por ejemplo, las fechas en la columna de la izquierda sugieren una línea, y una columna con unos pocos números positivos y etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que tiene contenido.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie por ahora

Seleccione un gráfico. Junto a él aparecen tres pequeños botones. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desactive una casilla para quitar esa serie del gráfico. Actívela de nuevo para volver a mostrarla en el mismo lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha quitado sigue en el archivo al guardarlo y permanece oculta al abrirlo, así que no se pierde nada. Active su casilla para volver a mostrarla. (Un gráfico de anillo muestra todas las series al abrirlo).

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiarle el tamaño.

- Mantenga pulsada la tecla **Alt** mientras arrastra para que un borde quede alineado con el borde de una celda.
- **Ajustar a la cuadrícula** y **Ajustar a la forma** están en la pestaña **Formato de forma**, y se conservan para usted en todos los libros. Con **Ajustar a la cuadrícula** activado, un borde que arrastra va al borde más cercano de una celda. Con **Ajustar a la forma** activado, un borde va al borde de otro objeto cuando está a menos de 6 píxeles. Solo se tienen en cuenta los primeros 2.000 objetos de una hoja para **Ajustar a la forma**. Con ambas opciones activadas, prevalecen los bordes de las celdas, porque un objeto está anclado a las celdas, y no se miran los bordes de los demás objetos. Un cambio se aplica a todas las ventanas abiertas a la vez.
- **Alt** invierte el ajuste activo en un solo arrastre: con **Ajustar a la cuadrícula** o **Ajustar a la forma** activados, mantener **Alt** desactiva el ajuste en ese arrastre (nunca cambia al otro tipo), y con ambas opciones desactivadas, **Alt** hace que los bordes vayan a las celdas, como se ha descrito antes.
- Un objeto girado se ajusta según el contorno que se ve (el cuadro más pequeño que lo contiene), y no según su marco antes de girarlo. Lo mismo ocurre con los objetos a los que se ajusta.
- Mantenga pulsada la tecla **Mayús** mientras arrastra una esquina para conservar las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo agranda o lo reduce, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición.

Cada movimiento o cambio de tamaño es un paso que se puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase corta que explica a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué ocurre cuando inserta, elimina o cambia el tamaño de celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o quedarse donde está. La opción se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el archivo también guarda esa opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y aparece la pestaña **Diseño de gráfico** en la cinta de opciones.

- El botón **+** (y **Elementos de gráfico** en la pestaña) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y permite indicar dónde van la leyenda y las etiquetas.
- El botón de pincel (y **Colores** y **Estilos** en la pestaña) aplica al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede añadirlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Botones de campo** (en el menú **+**, solo para un gráfico dinámico) muestra pequeños botones con los nombres de los campos de la tabla dinámica que sigue el gráfico: los filtros del informe bajo el título, y las filas, los valores y las columnas en la parte inferior. Active o desactive cada tipo, o elija **Mostrar todo** u **Ocultar todo**. Un nuevo gráfico dinámico los muestra.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas, áreas o dispersión) dibuja una pequeña línea encima y debajo de cada punto para mostrar cuánto podría diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Elija **Personalizado** para usar importes propios: escriba las celdas que contienen los importes sobre cada punto (por ejemplo, `=Hoja1!$D$2:$D$4`) y, después, las celdas de los importes por debajo. Deje una vacía para dibujar las barras solo a un lado. Las barras cambian cuando cambian esas celdas. Un archivo que tiene sus propios importes para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) incluye **Líneas de caída**, que bajan desde el punto más alto de cada categoría hasta el eje; **Líneas de máximos y mínimos**, que unen el punto más alto y el más bajo de cada categoría; y **Barras de subida/bajada**, que son cajas entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximos y mínimos y las barras de subida/bajada necesitan dos series o más. Los gráficos de áreas tienen líneas de caída.
- **Tabla de datos** (en el menú **+**, para un gráfico de columnas, líneas o áreas) coloca una cuadrícula con los números bajo el gráfico, con una columna para cada categoría y una fila para cada serie. Su lista activa y desactiva la tabla, y establece las líneas entre filas, las líneas entre columnas, el contorno y las claves de leyenda (el color de cada serie junto a su nombre).
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para que vea hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben deprisa y luego se estabilizan, **Polinómica** para una curva con inflexiones, **Potencia**, o **Media móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R al cuadrado** escribe lo bien que se ajusta la línea (1 es un ajuste perfecto; 0 es ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja contra un segundo eje vertical, los menús **Ejes** y **Títulos de los ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Sobre el gráfico**, que coloca la leyenda sobre el área de trazado en lugar de al lado.
- **Etiquetas de datos** también tiene **La etiqueta contiene** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: coma, punto y coma, punto, salto de línea o espacio) y **Formato de número…** (un código de formato como 0,0 para los números de las etiquetas; déjelo vacío para mostrar los números tal como están). **Valor desde celdas…** muestra el texto de algunas celdas como etiquetas, una celda para cada punto. Escriba las celdas, por ejemplo =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Dar formato al eje horizontal…** y **Dar formato al eje vertical…** en la pestaña **Diseño de gráfico** abren un cuadro para un eje. Permite establecer los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1.000 para miles, de modo que 30.000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, junto al título del eje), si la escala es logarítmica (cada marca es una base multiplicada por la anterior, como 1, 10, 100 y 1000, lo que conviene a números de tamaños muy distintos que deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde se sitúan sus etiquetas y cuánto se giran. Solo un eje numérico tiene límites y unidades.
- **Dar formato a la parte del gráfico…** en la pestaña **Diseño de gráfico** (y en el menú del botón derecho) establece el relleno y la línea de la página del gráfico, del área de trazado (el recuadro donde están las marcas) o de una serie. En el caso de una línea, también establece si es suavizada y qué marca aparece en cada punto. El menú del botón derecho de un gráfico tiene también **Cambiar entre filas y columnas** y los dos comandos de los ejes.
- **Selección actual** muestra las partes del gráfico: el área del gráfico, el área de trazado y cada serie. Elija una y **Dar formato a la selección** abre la ventana de formato en esa parte. **Restablecer para igualar al estilo** quita lo que se estableció en la parte elegida y devuelve lo que le da el estilo del gráfico. En el caso del área del gráfico, es su relleno, su línea y el color del texto que haya fijado un estilo (el texto claro del estilo oscuro vuelve al color de texto del tema). En el caso del área de trazado, son su relleno y su línea. Para una serie, es su color (el que le da la opción Colores o el estilo del gráfico), su contorno, sus marcas tal como las tiene ese tipo de gráfico, y los colores de sus puntos; una línea suavizada se mantiene si el tipo de gráfico es suavizado. No cambia títulos, leyenda, etiquetas, ejes, líneas de cuadrícula ni datos, y no hace nada con una parte que ya coincide. Es un paso de **Deshacer**.
- **Mover gráfico…** en la pestaña **Diseño de gráfico** pregunta dónde debe ir el gráfico. Elija **Nueva hoja** y escriba un nombre, y el gráfico tendrá una hoja de gráfico propia: una hoja que solo contiene el gráfico, sin celdas. Elija **Objeto en** y seleccione una hoja de cálculo, y el gráfico se colocará en ella. Un gráfico en una hoja de gráfico solo puede pasar a una hoja de cálculo, y la hoja de gráfico que deja se elimina. El gráfico sigue leyendo las celdas que leía. En una hoja de cálculo mantiene su tamaño y su posición en la hoja, aunque las filas y columnas sean más altas o más anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar, salvo que una hoja haya sido eliminada por esta acción. Un archivo en formato `.ods` no puede contener una hoja de gráfico, así que guarde una hoja de gráfico en formato `.xlsx`.
- **Título del gráfico** y **Títulos de los ejes** (en el menú **+**) también tienen **Texto del título…**. Escriba las palabras que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contenga esa celda. Cuando la celda cambia, también cambia el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la pestaña **Diseño de gráfico** (y en el menú del botón derecho) guarda el gráfico seleccionado como un archivo de imagen **PNG** o **JPEG**, con el doble de tamaño que en la hoja para que se vea nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la pestaña **Diseño de gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y de la leyenda, los ejes y las fuentes. El gráfico al que lo aplica conserva sus propios números y las celdas que lee. Un **Deshacer** quita de nuevo el aspecto.
- **Tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando contiene espacios.

## Minigráficos

Un minigráfico es un gráfico muy pequeño que cabe dentro de una celda. Muestra la tendencia de una fila o una columna de números justo al lado de ellos.

1. Seleccione los números. Para tener un minigráfico por fila, seleccione varias filas de números.
2. En la pestaña **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra para cada número. **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor, todas del mismo tamaño, lo que conviene a resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, compruebe el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas donde van). Elija una celda de ubicación para cada fila de datos, o para cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten un mismo aspecto. Haga clic en una celda con un minigráfico, y aparece la pestaña **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que ha seleccionado. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se leen las celdas de las filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en el caso de una línea, los **Marcadores** en cada punto.
- **Estilos** da al grupo un conjunto de colores. **Color del minigráfico** establece el color de la línea o las barras, y el **Grosor** (lo gruesa que es la línea, en puntos). **Color de marcador** establece el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y establece los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da al grupo completo una sola escala, para poder compararlos. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior queda más lejos. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un solo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** quita los minigráficos seleccionados, o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila sobre los datos, sigue leyendo los mismos números. Cada cambio es un paso que se puede deshacer. Los minigráficos se guardan en el archivo en un formato que leen otros programas de hojas de cálculo.

## Insertar una imagen, una forma o un cuadro de texto

En la pestaña **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y se colocará en la hoja, en la celda seleccionada. Una imagen de más de 480 píxeles de ancho se reduce para que quepa, y conserva su forma. El libro guarda la propia imagen. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen simple formada por puntos. Un archivo que no es una imagen, o que pesa más de 16 MB, se rechaza con un mensaje. Un archivo WMF o EMF (los tipos de imagen que algunos programas antiguos crean a partir de líneas y formas) se dibuja y también se guarda como PNG.

**Iconos** muestra la misma ventana de pequeñas imágenes de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlas por nombre. Elija uno o varios, y cada uno se colocará en la hoja como una imagen de 48 puntos de ancho y de alto. Se dibuja en gris oscuro, con un tamaño que se mantiene nítido cuando lo amplía. Cada uno es un solo paso que se puede deshacer.

**Código de barras** abre un menú corto con dos opciones. **Código de barras** abre una ventana para crear un solo código de barras (**Códigos de barras desde celdas**, más abajo, crea muchos): ISBN de libros (con o sin precio), EAN-13, UPC-A, EAN-8, Code 128, Code 39 o un código QR, que puede contener cualquier texto, como una dirección web. Elija un **Tipo**, escriba el texto o los dígitos, y la ventana muestra el código de barras a medida que escribe, y, si no se puede crear, indica en rojo qué falla. Pulse **Insertar** y se colocará en la hoja como una imagen, en un solo paso que se puede deshacer. Muévalo y cambie su tamaño como cualquier imagen. Para cambiarlo, selecciónelo y pulse de nuevo **Código de barras**, cambie el texto y pulse **Actualizar**; el nuevo código ocupa el lugar y el ancho del anterior. Al guardar, se guarda como una imagen con un SVG que se mantiene nítido, para que lo muestre cualquier programa, y JeffCrunch conserva sus ajustes en el archivo, de modo que sigue siendo un código de barras que puede editar cuando vuelva a abrir el libro en JeffCrunch.

**Código de barras > Códigos de barras desde celdas** (en el mismo menú **Código de barras**) crea un código de barras para cada celda con contenido de las celdas seleccionadas, para una hoja de etiquetas. Seleccione las celdas, haga clic en esta opción y después elija el tipo y el aspecto en la misma ventana. Lo que escriba allí es solo un ejemplo, porque el texto de cada celda se convierte en su código de barras. Los códigos se colocan en una columna justo a la derecha de las celdas seleccionadas, uno debajo de otro, como un solo paso que se puede deshacer. Una celda cuyo texto el código no admite (una letra en un EAN-13, por ejemplo) se indica en un mensaje y no recibe código de barras. Se procesan como máximo 500 celdas a la vez; seleccione menos si indica que hay demasiadas. Cada código de barras recuerda la celda de la que procede y la sigue: cuando cambia esa celda, el código se vuelve a crear con el nuevo texto, en el mismo lugar y con el mismo tamaño, como un paso propio que puede deshacer. El vínculo se mueve con la celda cuando inserta o elimina filas o columnas. Si elimina la celda, el código conserva el último texto que tenía. Al guardar, la imagen del archivo se crea con el texto de la celda tal como está en ese momento, para que otros programas muestren también el valor actual.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y aparecerá en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, coloca un cuadro vacío sin relleno ni borde. Haga clic en **Dibujar cuadro de texto** (el botón principal, o la primera opción de su menú) y arrastre sobre la hoja para dibujar el cuadro. Al arrastrar se crea un cuadro del tamaño que arrastre, pero nunca menor de 18 puntos (unos 0,6 cm) por lado, y un clic de menos de 4 puntos da el tamaño habitual. Mantenga pulsada la tecla **Mayús** para que el cuadro sea cuadrado. El puntero es una cruz hasta que suelta el botón; **Esc**, o cualquier otro comando, desactiva la herramienta. El cuadro es un paso que se puede deshacer.

Para girar el texto de un cuadro de texto, selecciónelo y haga clic en **Dirección del texto** en la pestaña **Formato de forma**: **Horizontal**, **Girar todo el texto 90°** o **Girar todo el texto 270°**. El texto se escribe girado, tal como se muestra, y un libro guardado en formato .xlsx o .ods conserva la dirección.

Con una forma seleccionada, **Estilos** en la pestaña **Formato de forma** le da un aspecto preestablecido: un relleno y un contorno que combinan, como azul claro con un borde azul más oscuro. Los dos últimos conservan solo el contorno o solo el relleno. Un clic es un paso que se puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

## Buscar imágenes libres en internet

**Imágenes en línea**, en el grupo **Ilustraciones** de la pestaña **Insertar**, busca imágenes libres. Proceden de dos bibliotecas abiertas: Openverse, que busca en muchas colecciones, y Wikimedia Commons, la biblioteca de las imágenes de Wikipedia.

1. Haga clic en **Imágenes en línea…**. Se abre un panel a la derecha.
2. Elija una biblioteca, escriba lo que busca y haga clic en **Buscar** (o pulse Intro).
3. Haga clic en una imagen para ver quién la hizo y con qué licencia. Una **licencia** indica cómo puede usar una imagen. Haga clic en el nombre de la licencia para leerla.
4. Haga clic en **Insertar** (o haga doble clic en la imagen).

**Qué envía JeffCrunch.** Las palabras de búsqueda se envían a la biblioteca que elija, y solo cuando hace clic en **Buscar** o pulsa Intro. Después, JeffCrunch pide a la biblioteca las vistas previas pequeñas de lo que ha encontrado y la imagen que inserta. Como cualquier página web, la biblioteca puede ver la dirección de internet de su equipo, y se le indica que la solicitud la hace JeffCrunch. No se envía nada de su libro.

La imagen se coloca en la hoja como una imagen de archivo, y el libro la conserva. Su título se guarda como **texto alternativo**. La mayoría de las licencias piden que se nombre al autor. Si está marcada la opción **Agregar un crédito debajo de la imagen**, JeffCrunch añade un pequeño cuadro de texto bajo la imagen con su título, su autor y su licencia. Puede moverlo, cambiarlo o eliminarlo. La imagen y su crédito se insertan en un solo paso, así que **Deshacer** quita ambos. Compruebe que la licencia se ajusta a la forma en que va a usar la imagen.

## Añadir texto artístico

El **texto artístico** son palabras con efectos: letras con color, contorno, sombra o una forma curva.

1. Haga clic en **Insertar > Texto > Texto artístico**. Se abre una galería de estilos.
2. Haga clic en un estilo. Las palabras "Escriba aquí el texto" aparecen en la hoja con ese estilo.
3. Para cambiar las palabras, seleccione el texto artístico y haga clic en **Editar texto…** en la pestaña **Formato de forma**. Un cuadro pide las palabras, la fuente, el tamaño, la negrita y la cursiva. Un tamaño mayor hace que el texto artístico sea más grande.
4. En la pestaña **Formato de forma**, **Estilos de texto artístico** da a las letras otro estilo, y **Deformar texto** las curva en una forma, como una onda o un arco. "Sin deformar" las mantiene rectas.

El texto artístico es una forma del dibujo de la hoja, con su deformación, su relleno y su contorno, de modo que conserva su aspecto al guardar el libro como archivo .xlsx y volver a abrirlo, en JeffCrunch o en otro programa. Cada cambio es un paso que se puede deshacer. Un libro guardado como archivo .ods conserva también sus imágenes, formas, cuadros de texto, grupos, líneas y texto artístico. El texto artístico se guarda como la forma de texto con efecto que tiene OpenDocument, con sus palabras, su fuente, su relleno, su contorno y su deformación, y vuelve a ser texto artístico al abrir de nuevo el archivo .ods. Algunas deformaciones que OpenDocument no tiene como forma (Onda 2, Onda 4, y las formas inferior y superior de Inflar y Desinflar) se guardan como palabras rectas con las mismas letras y estilo. Los gráficos también se conservan en un archivo .ods: vea más abajo.

## Reducir el tamaño de las imágenes

Una imagen hecha con una cámara puede ser mucho más grande de lo que se ve en la hoja, y hace grande el archivo. Haga clic en una imagen y después en **Comprimir imágenes…** en el grupo **Ajustar** de la pestaña **Formato de imagen**. Elija dónde se usará el libro: **Impresión comercial (300 ppi)**, **Impresión de escritorio (220 ppi)** o **Web (96 ppi)**. Deje marcada la opción **Aplicar a todas las imágenes de esta hoja** para reducir todas las imágenes de la hoja, o desmárquela para reducir solo la imagen seleccionada. **Eliminar las áreas recortadas de las imágenes** quita también las partes ocultas que recortó, y el recorte desaparece. Una imagen solo se reemplaza cuando la nueva es más pequeña. Su tamaño, su posición, su nombre y su texto alternativo se mantienen. Deshacer (Ctrl+Z) devuelve las imágenes originales, y el libro guardado solo contiene las imágenes más pequeñas. Las imágenes de otras hojas no cambian: vaya a cada hoja y repita la operación. Una imagen que es un dibujo (SVG, EMF o WMF), o una animación, se deja como está.

Cada una es un solo paso que se puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiarle el tamaño, llevarlo hacia delante o atrás, darle un nombre y un texto alternativo en **Propiedades**, o pulsar **Supr**.

## Mostrar el texto de una celda en una forma o un cuadro de texto

Una forma o un cuadro de texto puede mostrar lo que muestra una celda, y seguirla. Seleccione la forma, haga clic en la **barra de fórmulas**, escriba un signo igual y una celda, y pulse **Intro**.

- `=B2` vincula la forma a B2 de esta hoja. `=Hoja2!B3` la vincula a una celda de otra hoja. `=Total` la vincula a un nombre que representa una sola celda.
- La forma muestra entonces el texto de la celda tal como lo muestra la celda, con su formato de número. Cuando cambia la celda, su formato o una fórmula de la que depende, la forma también cambia. El texto más largo de lo que cabe en una celda (32.767 caracteres) se corta. Todas las formas vinculadas de un libro muestran en conjunto como máximo 4 MB de texto: más allá de eso, una forma muestra su texto cortado.
- Con la forma seleccionada, la barra de fórmulas muestra su vínculo, como `=Hoja1!$B$2`. Borre la barra de fórmulas y pulse **Intro** para quitar el vínculo. La forma conserva entonces las palabras que muestra.
- Escribir palabras en la forma también quita el vínculo.
- Un vínculo es una sola celda. Un rango como `=A1:B2`, una lista, un cálculo o un nombre que no es una celda se rechazan con un mensaje, y la forma conserva su vínculo.
- Si se elimina la fila o la columna de la celda, o su hoja, la forma muestra `#REF!` y conserva el vínculo, como lo haría una fórmula. Deshacer devuelve la celda.
- Cada vínculo, y cada vez que se quita uno, es un paso que se puede deshacer.

El vínculo se guarda en un archivo .xlsx junto con el texto que mostraba la celda, de modo que un programa que no sigue vínculos sigue mostrando las palabras, y JeffCrunch abre el archivo con el vínculo funcionando. OpenDocument no tiene forma de vincular un cuadro de texto a una celda, así que un libro guardado como archivo .ods conserva las palabras que muestra la forma en ese momento, y no el vínculo.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparece la pestaña **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparece la pestaña **Formato de forma**. Cada cambio es un paso que se puede deshacer, y el objeto conserva su nombre, su texto alternativo, su lugar y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** establece el grosor (lo gruesa que es la línea) y los guiones (continua, discontinua o de puntos). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta a una forma de ventana: cuadrada, 4 de ancho por 3 de alto, 3 de ancho por 4 de alto, 16 por 9 o 9 por 16. El corte se hace por igual desde el centro de lo que se ve, y la imagen se agranda hasta el tamaño de lo que queda. **Mostrar la imagen completa** devuelve lo que se recortó. **Rellenar** conserva el marco y recorta la imagen para que lo cubra por completo. **Ajustar** muestra la imagen entera, y el marco toma la forma de la imagen dentro del anterior. **Recortar a forma** (la galería junto a **Recortar**) corta la imagen a una forma como un óvalo, una estrella o un rectángulo redondeado; la forma se guarda con la imagen. El archivo de la imagen no cambia: la parte recortada solo queda oculta.
- **Editar texto** es para formas y cuadros de texto. Escriba las palabras y elija cómo se colocan a lo ancho y a lo alto, el tamaño, la negrita o no, y si se ajustan a la forma o quedan en una sola línea. Para el texto artístico, pide las palabras, la fuente, el tamaño, la negrita y la cursiva.
- **Panel de selección** abre una lista de todos los objetos de la hoja por su nombre, con el que está delante en primer lugar. Haga clic en un nombre para seleccionar ese objeto, lo que ayuda cuando los objetos se ocultan entre sí. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** establece el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o se agrandan las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no puede copiarse, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la original. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y, después, mantenga pulsada la tecla Mayús o Ctrl y haga clic en otro para seleccionarlo también. Haga clic de nuevo en un objeto seleccionado con Mayús o Ctrl para deseleccionarlo. Los objetos seleccionados tienen cada uno un marco discontinuo. También puede elegir **Seleccionar objetos** en la pestaña **Formato de forma** y arrastrar un recuadro sobre celdas vacías: se seleccionan todos los objetos que quedan completamente dentro del recuadro. Pulse Esc, o elija **Seleccionar objetos** de nuevo, para volver a seleccionar celdas. Pulse Supr para eliminar todos los objetos seleccionados en un solo paso.

Con dos o más seleccionados, la pestaña **Formato de forma** tiene estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el archivo lo guarda como grupo. Un gráfico no puede agruparse.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su lugar en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Centrar**, **Alinear a la derecha**, **Alinear arriba**, **Alinear al medio** y **Alinear abajo**. Se alinean con el borde o el centro de todo el conjunto. **Distribuir horizontalmente** y **Distribuir verticalmente** reparten tres o más objetos por igual entre los dos de los extremos.

Agrupar, desagrupar y cada opción de alineación son un paso que se puede deshacer.

## Darle nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. Deshacer devuelve el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Pulse **Supr** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el archivo en un formato que leen otros programas de hojas de cálculo, de modo que un gráfico que crea aquí se abre allí de la misma manera. Un gráfico que procede de otro programa, con algo que JeffCrunch no puede conservar al reescribirlo (barras de error o un gráfico tridimensional), puede moverse, cambiar de tamaño y eliminarse, pero sus elementos y sus datos no se modifican aquí.

## Insertar un objeto

Un **objeto** guarda su propio archivo dentro del libro, de modo que la aplicación que lo creó puede volver a abrirlo. Muestra una imagen de sí mismo, pero no es una imagen: sigue siendo un objeto cuando lo mueve, cambia su tamaño, lo copia o guarda el libro.

1. Haga clic en **Insertar** y después en **Objeto**, y elija **Desde un rango de JeffCrunch**, **Desde un gráfico de JeffCrunch** o **Desde una página de JeffWrite**.
2. Elija el archivo y el rango o el número de página si el cuadro lo pide. Si lo copió en la otra aplicación, el cuadro ofrece usarlo.
3. El objeto se coloca en la celda seleccionada. Arrástrelo para moverlo, y arrastre un controlador para cambiarle el tamaño. Un rango, gráfico o página al que cambie el tamaño se vuelve a dibujar con su nuevo tamaño, para que siga nítido. Pulse **Supr** para quitarlo. Cada uno es un solo paso de Deshacer.

Para insertar lo que copió en otra aplicación, haga clic en la flecha bajo **Pegar** y elija **Como objeto**.

**Haga doble clic** en un objeto para abrirlo en la aplicación que lo creó. Guárdelo allí, y el objeto aquí muestra el cambio como un paso de Deshacer. Una celda en la que estaba escribiendo se deja como está. Una copia que pega es un objeto propio: cambiar una no cambia la otra. Lo mismo ocurre con una hoja que copia: sus objetos son copias, y la copia de la hoja los conserva.

**Archivo**, después **Información**, después **Objetos** enumera todos los objetos del libro. Seleccione uno para abrirlo, convertirlo en una imagen simple o eliminarlo.

Un objeto creado en otro programa se muestra como la imagen que guarda su archivo, sea cual sea el tipo de imagen. Puede seleccionarlo, moverlo, cambiarle el tamaño, alinearlo con otros objetos y eliminarlo, y se mueve con las celdas como sus propios objetos. No puede editarlo aquí, ponerlo en un grupo ni cambiar su lugar en el orden de los objetos. Se guarda tal como lo tenía el archivo, con solo su posición cambiada si lo movió. Un objeto que contenía un archivo .ods funciona igual. Guardados como archivo .xlsx o .ods, sus objetos están en el formato estándar que leen otros programas. Cuando se escribe un archivo .ods para un objeto cuya imagen no puede dibujarse, el objeto se conserva sin imagen.

## Diagramas de otros programas

Un **diagrama** es una imagen de cuadros y flechas, como un proceso, que creó otro programa. JeffCrunch muestra un diagrama a partir de la imagen que el archivo guarda junto a él. Haga clic en él para seleccionarlo, y arrástrelo para moverlo. Al guardar el archivo, el diagrama se escribe de nuevo tal como era, para que el programa que lo creó pueda seguir modificándolo.

JeffCrunch todavía no puede cambiar lo que hay dentro de un diagrama, y un diagrama no puede copiarse ni ponerse en un grupo. Si el archivo no guarda una imagen de un diagrama, JeffCrunch muestra un cuadro gris sencillo que indica que es un diagrama.

## Gráficos en archivos .ods

Un libro guardado como archivo .ods conserva sus gráficos: de columnas, barras, líneas, áreas, circulares, de anillo, de dispersión, de radar y de bolsa de máximo, mínimo y cierre, o de apertura, máximo, mínimo y cierre, cada uno con sus series y las celdas que leen (en esta hoja o en otra), categorías, título, leyenda, ejes (títulos, valor mínimo y máximo, formato de número), etiquetas de datos, colores y ubicación. Un archivo .ods creado por otro programa se abre con sus gráficos, dibujados a partir de las celdas que leen. Lo que un gráfico .ods no tiene dónde guardar se omite y se indica al guardar: líneas de tendencia, barras de error, ejes secundarios (así, un gráfico de bolsa con volumen se guarda con el volumen en el primer eje, y se abre como columnas) y los estilos especiales de un solo punto. Los gráficos de tipos que JeffCrunch no dibuja se abren como columnas de sus valores.
