# Gráficos, minigráficos, imágenes y formas

<!-- keywords: botones de campo, botones de gráfico dinámico, agrupar, desagrupar, alinear, alinear a la izquierda, distribuir, seleccionar objetos, varias formas, elegir varios, cuadro de selección, filtros de gráfico, ocultar series, gráficos recomendados, sugerencias de gráfico, gráfico, columna, barra, línea, circular, área, dispersión, combinado, leyenda, título, líneas de cuadrícula, etiquetas de datos, datos del gráfico, tipo de gráfico, aspecto, diseños, mover, cambiar tamaño, imagen, forma, objeto, serie, línea de tendencia, ecuación, R cuadrado, segundo eje, minigráfico, minigráficos, eje de fechas, ganancia/pérdida, minigráfico, marcadores, nombre, texto alternativo, logarítmico, escala logarítmica, guardar como imagen, plantilla, crtx, texto del título, clave de leyenda, panel de selección, relleno de forma, contorno de forma, girar, voltear, recortar, editar texto, borde de imagen, insertar imagen, iconos, icono, wmf, emf, estilos de forma, estilos, insertar forma, cuadro de texto, formas, flecha, óvalo, rectángulo, estrella, diagrama, diagramas, imágenes en línea, imágenes gratuitas, openverse, wikimedia commons, licencia, crédito, texto artístico, wordart, deformar, comprimir imágenes, reducir imágenes, tamaño de archivo, ppp, eliminar áreas recortadas, código de barras, código QR -->

Un gráfico es una imagen de números que se mantiene vinculada a ellos: si cambia un número, el gráfico cambia con él. Las imágenes y las formas se colocan sobre las celdas de la misma manera, pero no están vinculadas a ningún número.

## Crear un gráfico

1. Seleccione los números que desea representar. Si la primera fila tiene nombres y la primera columna tiene etiquetas, seleccione también esas celdas. JeffCrunch examina la primera fila y la primera columna y las usa como nombres y como etiquetas cuando son palabras (o fechas, en el caso de las etiquetas).
2. En la ficha **Insertar**, en **Gráficos**, elija un tipo: **Columna**, **Barra**, **Línea**, **Circular**, **Área**, **Dispersión** o **Combinado**. La pequeña flecha junto a un tipo muestra sus variantes, como columnas apiladas, una línea con marcadores o un gráfico de dispersión de puntos, de puntos unidos o de líneas suavizadas.
3. Para ver antes todos los tipos, elija **Más gráficos**. La lista de la izquierda muestra cada tipo dibujado con sus celdas. **Cambiar filas y columnas** invierte el sentido de las series.

¿No sabe qué tipo elegir? Seleccione sus celdas y elija **Gráficos recomendados** en la ficha **Insertar**. JeffCrunch examina las celdas y muestra algunos tipos que les convienen, del mejor al peor. Por ejemplo, si las fechas están en una columna, sugiere una línea, y una sola columna de unos pocos números positivos con etiquetas sugiere un gráfico circular. Cada tipo se dibuja con sus celdas. Elija uno y pulse **Aceptar**.

Si selecciona una sola celda, JeffCrunch representa el bloque de celdas que la rodea y que contiene datos.

Las series se disponen en columnas cuando la selección tiene al menos tantas filas como columnas, y en filas cuando tiene más columnas que filas. Un gráfico circular contiene una sola serie. Un gráfico de dispersión toma sus valores horizontales de la primera columna.

Ejemplo: selecciona A1:C4, donde A2:A4 contienen Q1, Q2 y Q3, B1 y C1 contienen Este y Oeste, y el resto son números. El gráfico tiene dos series, Este y Oeste, y tres grupos de columnas, de Q1 a Q3.

## Ocultar una serie por ahora

Elija un gráfico. Junto a él aparecen tres pequeños botones. El que tiene forma de embudo es **Filtros de gráfico**. Muestra las series del gráfico, cada una con una casilla. Desmarque una casilla para quitar esa serie del gráfico. Vuelva a marcarla para que regrese a su lugar. **Deshacer** también funciona. Un gráfico siempre conserva al menos una serie. Una serie que ha quitado sigue en el archivo al guardarlo y permanece oculta al abrirlo, así que no se pierde nada. Marque la casilla para volver a mostrarla. (Un gráfico de anillo muestra todas las series al abrirse).

## Mover y cambiar el tamaño

Haga clic en un gráfico, una imagen o una forma para seleccionarlo. Arrástrelo para moverlo, o arrastre uno de los ocho cuadrados de su borde para cambiarle el tamaño.

- Mantenga pulsada **Alt** mientras arrastra para alinear un borde con el borde de una celda.
- Mantenga pulsada **Mayús** mientras arrastra una esquina para mantener las proporciones.
- Las teclas de flecha lo desplazan unos pocos píxeles cada vez. **Ctrl** con una flecha lo mueve un píxel, **Mayús** con una flecha lo hace más grande o más pequeño, y **Alt** con una flecha lo lleva al siguiente borde de una celda.
- **Esc** mientras arrastra lo devuelve a su posición original.

Cada movimiento o cambio de tamaño es un paso que puede deshacer. Haga clic con el botón derecho en el objeto y elija **Propiedades** para escribir su **Nombre** (cada objeto de una hoja necesita el suyo) y su **Texto alternativo** (una frase breve que explica a una persona que no puede ver el objeto qué muestra), para escribir su tamaño y para elegir qué hace al insertar, eliminar o cambiar el tamaño de celdas: moverse y cambiar de tamaño con las celdas, moverse con ellas pero conservar su tamaño, o permanecer donde está. La elección se guarda en el archivo. **Imprimir este objeto**, en el mismo cuadro, desactiva el objeto en las páginas impresas (sigue viéndose en pantalla); el archivo también guarda esta opción.

## Cambiar lo que muestra un gráfico

Cuando un gráfico está seleccionado, aparecen dos botones junto a él y en la cinta de opciones aparece la ficha **Gráfico**.

- El botón **+** (y **Elementos de gráfico** en la ficha) activa o desactiva el título del gráfico, los ejes, los títulos de los ejes, las líneas de cuadrícula, la leyenda y las etiquetas de datos, y muestra dónde van la leyenda y las etiquetas.
- El botón de pincel (y **Colores** y **Estilos** en la ficha) da al gráfico uno de los conjuntos de colores de JeffCrunch y uno de sus estilos. **Diseños** configura varios elementos a la vez.
- **Datos del gráfico** muestra las series del gráfico. Puede agregarlas, editarlas, quitarlas y reordenarlas, cambiar las celdas que lee cada una, establecer las celdas con las etiquetas, intercambiar filas y columnas, y elegir cómo se representan las celdas vacías y las filas y columnas ocultas.
- **Botones de campo** (en el menú **+**, solo para un gráfico dinámico) muestra pequeños botones con los nombres de los campos de la tabla dinámica que sigue el gráfico: los filtros del informe bajo el título, y las filas, los valores y las columnas en la parte inferior. Active o desactive cada tipo, o elija **Mostrar todo** u **Ocultar todo**. Un gráfico dinámico nuevo los muestra.
- **Barras de error** (en el menú **+**, para un gráfico de columnas, barras, líneas, áreas o dispersión) dibuja una pequeña línea por encima y por debajo de cada punto para mostrar cuánto puede diferir el valor real. Elija **Error estándar**, **Porcentaje (5 %)**, **Desviación estándar** o **Valor fijo (1)**. Elija **Personalizado** para usar importes propios: escriba las celdas que contienen los importes por encima de cada punto (por ejemplo `=Hoja1!$D$2:$D$4`) y luego las celdas de los importes por debajo. Deje una vacía para dibujar las barras en un solo lado. Las barras cambian cuando cambian esas celdas. Un archivo que tiene importes propios para cada punto los conserva.
- **Líneas** (en el menú **+**, para un gráfico de líneas) incluye **Líneas de caída**, que van desde el punto más alto de cada categoría hasta el eje; **Líneas de máximos y mínimos**, que unen el punto más alto y el más bajo de cada categoría, y **Barras de subida/bajada**, que son cuadros entre la primera y la última serie de cada categoría. Una barra de subida indica que la última serie es más alta. Las líneas de máximos y mínimos y las barras de subida/bajada requieren dos series o más. Los gráficos de área tienen líneas de caída.
- **Línea de tendencia** (en el menú **+**) dibuja una línea ajustada a los números de cada serie, para ver hacia dónde se dirigen. Elija **Lineal** para una línea recta, **Exponencial** para números que crecen en un porcentaje constante, **Logarítmica** para números que suben rápido y luego se estabilizan, **Polinómica** para una curva con inflexiones, **Potencial** o **Media móvil**, que dibuja el promedio de los últimos números. **Mostrar ecuación** escribe la fórmula de la línea en el gráfico, y **Mostrar R cuadrado** escribe qué tan bien se ajusta la línea (1 es un ajuste perfecto; 0, ningún ajuste). La línea de tendencia tiene su propia entrada en la leyenda.
- Cuando una serie se dibuja con un segundo eje vertical, los menús **Ejes** y **Títulos de ejes** tienen las entradas **Horizontal secundario** y **Vertical secundario** para ella. El menú **Leyenda** tiene **Encima del gráfico**, que coloca la leyenda sobre el área de trazado en lugar de a su lado.
- **Etiquetas de datos** también tiene **Contenido de la etiqueta** (el valor, el nombre de la categoría, el nombre de la serie, el porcentaje y la clave de leyenda, que es un pequeño cuadrado del color de la serie, en cualquier combinación), **Separador** (lo que va entre ellos: coma, punto y coma, punto, nueva línea o espacio) y **Formato de número…** (un código de formato, como 0,0, para los números de las etiquetas; déjelo vacío para mostrar los números tal como están). **Valor de celdas…** muestra el texto de algunas celdas como etiquetas, una celda por punto. Escriba las celdas, por ejemplo =Hoja1!$D$2:$D$4. Las etiquetas cambian cuando cambian las celdas. Déjelo vacío para quitarlas. Si desactiva todos los contenidos, las etiquetas desaparecen.
- **Formato del eje horizontal…** y **Formato del eje vertical…** en la ficha **Gráfico** abren un cuadro para un eje. Establece los valores mínimo y máximo y las unidades principal y secundaria (cada una automática o un número que escriba), si los valores van en orden inverso, las unidades de visualización (las etiquetas muestran los números divididos, por ejemplo entre 1000 para miles, de modo que 30 000 se lee como 30; marque **Mostrar el nombre de las unidades en el gráfico** para imprimir el nombre, como Miles, junto al título del eje), si la escala es logarítmica (cada marca es una base por la anterior, como 1, 10, 100 y 1000, lo que resulta adecuado para números de tamaños muy distintos que deben ser todos mayores que 0), dónde cruza el otro eje, el formato de número de sus etiquetas, sus marcas de graduación, dónde están sus etiquetas y cuánto están giradas. Solo un eje numérico tiene límites y unidades.
- **Mover gráfico…** en la ficha **Gráfico** pregunta dónde debe ir el gráfico. Elija **Hoja nueva** y escriba un nombre, y el gráfico obtendrá una hoja de gráfico propia: una hoja que solo contiene un gráfico, sin celdas. Elija **Objeto en** y seleccione una hoja de cálculo, y el gráfico se colocará en ella. Un gráfico de una hoja de gráfico solo puede pasar a una hoja de cálculo, y la hoja de gráfico que deja se elimina. El gráfico sigue leyendo las mismas celdas. En una hoja de cálculo conserva su tamaño y su posición en la hoja, aunque las filas y las columnas sean más altas o más anchas, y se mueve y cambia de tamaño con las celdas como antes. No se ofrecen las hojas ocultas. Es un solo **Deshacer**, incluso después de guardar, salvo que la operación haya eliminado una hoja. Un archivo en formato `.ods` no puede contener una hoja de gráfico, así que guarde una hoja de gráfico en formato `.xlsx`.
- **Título del gráfico** y **Títulos de ejes** (en el menú **+**) también tienen **Texto del título…**. Escriba las palabras que desee. O escriba un signo igual y una celda, como **=Hoja1!A1**, y el título mostrará lo que contenga esa celda. Cuando la celda cambia, también cambia el título. El archivo conserva el vínculo.
- **Guardar como imagen…** en la ficha **Gráfico** (y en el menú del botón derecho) guarda el gráfico seleccionado como archivo de imagen **PNG** o **JPEG**, con el doble de tamaño que en la hoja para que se vea nítido.
- **Guardar como plantilla…** y **Aplicar plantilla…** (en la ficha **Gráfico**, en el grupo **Tipo**) guardan el aspecto de un gráfico en un archivo **.crtx** y lo aplican a otro gráfico. El aspecto incluye el tipo, los colores, la configuración del título y la leyenda, los ejes y las fuentes. El gráfico al que se aplica conserva sus propios números y las celdas que lee. Un **Deshacer** quita de nuevo el aspecto.
- **Cambiar tipo de gráfico** cambia el tipo de todo el gráfico, o de cada serie: columnas con una línea encima, o una serie frente a un segundo eje vertical.

Una referencia a celdas se escribe como **Hoja1!$B$2:$B$9**, con el nombre de la hoja entre comillas simples cuando tiene espacios.

## Minigráficos

Un minigráfico es un gráfico diminuto que cabe dentro de una sola celda. Muestra la tendencia de una fila o una columna de números justo al lado de ellos.

1. Seleccione los números. Para tener un minigráfico por fila, seleccione varias filas de números.
2. En la ficha **Insertar**, en **Minigráficos**, elija **Línea**, **Columna** o **Ganancia/pérdida**. Una **Línea** une los números con una línea. Una **Columna** dibuja una barra por cada número. Una **Ganancia/pérdida** dibuja una barra hacia arriba por cada número mayor que cero y una barra hacia abajo por cada número menor que cero, todas del mismo tamaño, lo que resulta adecuado para resultados que solo son buenos o malos.
3. En el cuadro **Crear minigráficos**, compruebe el **Rango de datos** (las celdas que leen los minigráficos) y escriba el **Rango de ubicación** (las celdas donde van). Elija una celda de ubicación para cada fila de datos, o para cada columna de datos. Si elige una sola celda, el minigráfico lee todos los datos.

Los minigráficos que crea en un solo paso forman un grupo y comparten el mismo estilo. Haga clic en una celda con un minigráfico y aparecerá la ficha **Minigráfico**.

- **Editar datos** cambia los datos y la ubicación de todo el grupo, o los datos del que seleccionó. También indica qué hacer con las celdas vacías (dejar un hueco, tratarlas como cero o unir los puntos a través de ellas) y si se leen las celdas de filas y columnas ocultas.
- **Tipo** cambia entre línea, columna y ganancia/pérdida.
- **Mostrar** marca puntos: el **Punto alto**, el **Punto bajo**, los **Puntos negativos**, el **Primer punto**, el **Último punto» y, en una línea, **Marcadores** en cada punto.
- **Estilos** da al grupo un conjunto de colores. **Color del minigráfico** establece el color de la línea o de las barras y el **Grosor** (qué tan gruesa es la línea, en puntos). **Color de marcador** establece el color de cada tipo de punto marcado.
- **Eje** muestra una línea en cero, traza los números de derecha a izquierda y establece los valores mínimo y máximo del eje vertical. **Automático para cada minigráfico** ajusta cada uno a sus propios números. **Igual para todos los minigráficos** da a todo el grupo una sola escala, para que puedan compararse. **Valor personalizado** le permite escribir un número. **Eje de fechas…** espacia los puntos según las fechas: escriba las celdas que contienen la fecha de cada punto, y un punto con una fecha posterior se coloca más adelante. Déjelo vacío para volver a espaciar los puntos de forma uniforme.
- **Agrupar** pone los minigráficos seleccionados en un mismo grupo. **Desagrupar** da a cada uno su propio aspecto. **Borrar** elimina los minigráficos seleccionados o sus grupos completos.

Un minigráfico sigue a sus celdas: si inserta una fila encima de los datos, sigue leyendo los mismos números. Cada cambio es un paso que puede deshacer. Los minigráficos se guardan en el archivo en un formato que otros programas de hojas de cálculo pueden leer.

## Insertar una imagen, una forma o un cuadro de texto

En la ficha **Insertar**, en **Ilustraciones**, **Imagen** abre un selector de archivos. Elija un archivo de imagen (PNG, JPEG, GIF, BMP, TIFF o SVG) y aparecerá en la hoja, en la celda seleccionada. Una imagen de más de 480 puntos de ancho se reduce para caber y conserva su forma. El libro guarda la imagen en sí. Una imagen de un tipo que el libro no puede guardar tal cual (un SVG, por ejemplo) se guarda como PNG, una imagen sencilla formada por puntos. Un archivo que no es una imagen, o que pesa más de 16 MB, se rechaza con un mensaje. Un archivo WMF o EMF (los tipos de imagen que algunos programas antiguos crean a partir de líneas y formas) también se dibuja y se guarda como PNG.

**Iconos** muestra la misma ventana de pequeñas imágenes de línea que tienen JeffPub y JeffDeck, con un cuadro para buscarlas por nombre. Elija uno o varios, y cada uno aparecerá en la hoja como una imagen de 48 puntos de ancho y alto. Se dibuja en gris oscuro, con un tamaño que se ve nítido al ampliarlo. Cada uno es un paso que puede deshacer.

**Código de barras** abre una ventana para crear un código de barras: ISBN de libro (con o sin precio), EAN-13, UPC-A, EAN-8, Code 128, Code 39 o un código QR, que puede contener cualquier texto, como una dirección web. Elija un **Tipo**, escriba el texto o los dígitos, y la ventana muestra el código de barras mientras escribe y, si no puede crearse, indica en rojo qué está mal. Pulse **Insertar** y aparecerá en la hoja como una imagen, en un paso que puede deshacer. Muévalo y cambie su tamaño como cualquier imagen. Para cambiarlo, selecciónelo y pulse de nuevo **Código de barras**, modifique el texto y pulse **Actualizar**; el nuevo código ocupa el lugar y el ancho del anterior. Al guardar, se guarda como imagen con un SVG que se ve nítido, de modo que cualquier programa lo muestra, y JeffCrunch conserva sus ajustes en el archivo, así que sigue siendo un código de barras que puede editar cuando vuelve a abrir el libro en JeffCrunch.

**Formas** abre una lista: rectángulo, rectángulo redondeado, óvalo, triángulo, rombo, flechas, una estrella, un corazón, una nube, un globo de diálogo y una línea. Elija una y aparecerá en la hoja en azul claro con un borde más oscuro. **Cuadro de texto**, en el grupo **Texto**, inserta un cuadro vacío sin relleno ni borde.

Con una forma seleccionada, **Estilos** en la ficha **Formato de forma** le da un aspecto preparado: un relleno y un contorno que combinan, como azul claro con un borde azul oscuro. Los dos últimos conservan solo el contorno o solo el relleno. Un clic es un paso que puede deshacer. **Relleno de forma** y **Contorno de forma** cambian cada parte por separado.

## Buscar imágenes gratuitas en internet

**Imágenes en línea**, en el grupo **Ilustraciones** de la ficha **Insertar**, busca imágenes gratuitas. Proceden de dos bibliotecas abiertas: Openverse, que busca en muchas colecciones, y Wikimedia Commons, la biblioteca de imágenes de Wikipedia.

1. Haga clic en **Imágenes en línea…**. Se abre un panel a la derecha.
2. Elija una biblioteca, escriba lo que busca y haga clic en **Buscar** (o pulse Intro).
3. Haga clic en una imagen para ver quién la hizo y qué **licencia** tiene. Una **licencia** indica cómo puede usar una imagen. Haga clic en el nombre de la licencia para leerla.
4. Haga clic en **Insertar** (o haga doble clic en la imagen).

**Qué envía JeffCrunch.** Las palabras de búsqueda se envían a la biblioteca que elija, y solo cuando hace clic en **Buscar** o pulsa Intro. Después, JeffCrunch pide a la biblioteca las vistas previas pequeñas de lo que encontró y la imagen que inserta. Como cualquier página web, la biblioteca puede ver la dirección de internet de su equipo, y se le indica que la petición procede de JeffCrunch. No se envía nada de su libro.

La imagen aparece en la hoja como una imagen de archivo, y el libro la conserva. Su título se guarda como **texto alternativo**. La mayoría de las licencias piden que se nombre al autor. Si está marcada la opción **Agregar un crédito debajo de la imagen**, JeffCrunch añade un pequeño cuadro de texto bajo la imagen con su título, su autor y su licencia. Puede moverlo, cambiarlo o eliminarlo. La imagen y su crédito se insertan en un solo paso, así que **Deshacer** los quita a ambos. Compruebe que la licencia se ajusta al uso que va a dar a la imagen.

## Agregar texto artístico

El **texto artístico** son palabras decoradas: letras con color, contorno, sombra o una forma curva.

1. Haga clic en **Insertar > Texto > Texto artístico**. Se abre una galería de estilos.
2. Haga clic en un estilo. Las palabras «Escriba aquí el texto» aparecen en la hoja con ese estilo.
3. Para cambiar las palabras, seleccione el texto artístico y haga clic en **Editar texto…** en la ficha **Formato de forma**. Un cuadro pide las palabras, la fuente, el tamaño, la negrita y la cursiva. Un tamaño mayor hace más grande el texto artístico.
4. En la ficha **Formato de forma**, **Estilos de texto artístico** da a las letras otro estilo, y **Deformar texto** dobla las palabras para darles una forma, como una onda o un arco. «Sin forma» las deja rectas.

El texto artístico es una forma del dibujo de la hoja, con su deformación, su relleno y su contorno, así que conserva su aspecto al guardar el libro en formato .xlsx y volver a abrirlo, en JeffCrunch o en otro programa. Cada cambio es un paso que puede deshacer. Un libro guardado en formato .ods conserva también sus imágenes, formas, cuadros de texto, grupos, líneas y texto artístico. El texto artístico se guarda como la forma de texto con efectos que tiene OpenDocument, con sus palabras, fuente, relleno, contorno y deformación, y vuelve a aparecer como texto artístico al abrir de nuevo el archivo .ods. Algunas deformaciones que OpenDocument no tiene como forma (Onda 2, Onda 4, y las formas inferior y superior de Inflar y Desinflar) se guardan como palabras rectas con las mismas letras y estilo. Los gráficos también se conservan en un archivo .ods: véase más abajo.

## Reducir el tamaño de las imágenes

Una imagen de una cámara puede ser mucho más grande de lo que se ve en la hoja, y hace que el archivo pese mucho. Haga clic en una imagen y después en **Comprimir imágenes…** en el grupo **Ajustar** de la ficha **Formato de imagen**. Elija el uso que dará al libro: **Impresión comercial (300 ppp)**, **Impresión de escritorio (220 ppp)** o **Web (96 ppp)**. Deje marcada la opción **Aplicar a todas las imágenes de esta hoja** para reducir todas las imágenes de la hoja, o desmárquela para reducir solo la imagen seleccionada. **Eliminar las áreas recortadas de las imágenes** también quita las partes ocultas que recortó, y el recorte desaparece. Una imagen solo se reemplaza cuando la nueva es más pequeña. Su tamaño, su posición, su nombre y su texto alternativo se mantienen. Deshacer (Ctrl+Z) devuelve las imágenes originales, y el libro guardado solo contiene las imágenes reducidas. Las imágenes de otras hojas no cambian: vaya a cada hoja y repita la operación. Una imagen que es un dibujo (SVG, EMF o WMF), o una animación, no se modifica.

Cada una es un solo paso que puede deshacer. El nuevo objeto queda seleccionado, así que puede arrastrarlo, cambiarle el tamaño, llevarlo al frente o al fondo, darle nombre y texto alternativo en **Propiedades**, o pulsar **Supr**.

## Cambiar el aspecto de una imagen, una forma o un cuadro de texto

Seleccione una imagen y aparecerá la ficha **Formato de imagen**. Seleccione una forma, una línea o un cuadro de texto y aparecerá la ficha **Formato de forma**. Cada cambio es un paso que puede deshacer, y el objeto conserva su nombre, su texto alternativo, su posición y su orden entre los demás objetos.

- **Relleno de forma** y **Contorno de forma** eligen un color, o ninguno. **Estilo de línea** establece el grosor (qué tan gruesa es la línea) y los guiones (continua, discontinua o punteada). Una imagen tiene **Borde de imagen**, que es su contorno.
- **Girar** gira el objeto un cuarto de vuelta a la derecha o a la izquierda, o lo voltea de lado a lado o de arriba abajo.
- **Recortar**, para una imagen, la corta con forma de ventana: cuadrada, de 4 de ancho por 3 de alto, de 3 de ancho por 4 de alto, de 16 por 9 o de 9 por 16. El recorte se hace por igual desde el centro de lo que se ve, y la imagen se amplía lo máximo posible con lo que queda. **Mostrar la imagen completa** devuelve lo que se recortó. El archivo de la imagen no cambia: la parte recortada solo queda oculta.
- **Editar texto** sirve para formas y cuadros de texto. Escriba las palabras y elija cómo se colocan horizontal y verticalmente, el tamaño, la negrita o la normal, y si se ajustan a la forma o quedan en una sola línea. Para un texto artístico pide las palabras, la fuente, el tamaño, la negrita y la cursiva.
- **Panel de selección** abre una lista de todos los objetos de la hoja por su nombre, el de delante primero. Haga clic en un nombre para seleccionar ese objeto, lo que resulta útil cuando los objetos se ocultan unos a otros. **Traer adelante** y **Enviar atrás** lo mueven en la lista.
- **Propiedades** establece el tamaño, el nombre, el texto alternativo y lo que hace el objeto cuando se mueven o amplían las celdas.

## Cortar, copiar y pegar un objeto

Una imagen o una forma se copia y se pega igual que un gráfico. Una forma que usa algo distinto de una imagen del libro, como un vínculo a un archivo, no se puede copiar, y JeffCrunch le indica el motivo.

Copie un gráfico y péguelo en el mismo libro, en cualquier hoja: la copia lee las mismas celdas que la original. Si lo pega en otro libro, conserva los números que mostraba, pero ya no lee celdas, porque las celdas que leería allí no son las de las que se creó.

## Seleccionar varios objetos, agruparlos y alinearlos

Haga clic en un objeto y después mantenga pulsada Mayús o Ctrl y haga clic en otro para seleccionarlo también. Haga clic de nuevo en un objeto seleccionado con Mayús o Ctrl para quitarlo de la selección. Cada objeto seleccionado muestra un marco discontinuo. También puede elegir **Seleccionar objetos** en la ficha **Formato de forma** y arrastrar un cuadro sobre celdas vacías: se seleccionan todos los objetos que quepan completamente dentro del cuadro. Pulse Esc, o elija **Seleccionar objetos** de nuevo, para volver a seleccionar celdas. Pulse Supr para eliminar todos los objetos seleccionados en un solo paso.

Con dos o más objetos seleccionados, la ficha **Formato de forma** muestra estas opciones en el grupo **Organizar**:

- **Agrupar** une las formas, imágenes y líneas seleccionadas en un solo objeto. Se mueve y cambia de tamaño como uno solo, y el archivo lo conserva como grupo. Un gráfico no puede agruparse.
- **Desagrupar** separa un grupo seleccionado. Cada parte vuelve a su lugar en la hoja.
- **Alinear** alinea los objetos seleccionados: **Alinear a la izquierda**, **Alinear al centro**, **Alinear a la derecha**, **Alinear arriba**, **Alinear en el medio** y **Alinear abajo**. Se alinean con el borde o con el centro de todo el conjunto. **Distribuir horizontalmente** y **Distribuir verticalmente** reparten de forma uniforme tres o más objetos entre los dos de los extremos.

Agrupar, desagrupar y cada opción de alineación son un solo paso que puede deshacer.

## Darle nombre

Con un objeto seleccionado, el cuadro de nombres muestra su nombre. Escriba un nombre nuevo allí y pulse **Intro**. Deshacer restaura el nombre anterior, y el nombre nuevo se guarda en el archivo.

## Eliminarlo

Pulse **Supr** con el objeto seleccionado. Deshacer lo devuelve.

## Qué se guarda

Todo lo anterior se guarda en el archivo en un formato que otros programas de hojas de cálculo pueden leer, así que un gráfico creado aquí se abre allí de la misma manera. Un gráfico que procede de otro programa con algo que JeffCrunch no puede conservar al reescribirlo (barras de error o un gráfico tridimensional) puede moverse, cambiar de tamaño y eliminarse, pero sus elementos y datos no se modifican aquí.

## Diagramas de otros programas

Un **diagrama** es una imagen de cuadros y flechas, como un proceso, que creó otro programa. JeffCrunch muestra un diagrama a partir de la imagen que el archivo guarda junto a él. Haga clic en él para seleccionarlo y arrástrelo para moverlo. Al guardar el archivo, el diagrama se escribe de nuevo tal como era, para que el programa que lo creó pueda seguir modificándolo.

JeffCrunch todavía no puede cambiar el contenido de un diagrama, y un diagrama no puede copiarse ni agruparse. Si el archivo no guarda una imagen de un diagrama, JeffCrunch muestra un cuadro gris sencillo que indica que es un diagrama.

## Gráficos en archivos .ods

Un libro guardado en formato .ods conserva sus gráficos: de columnas, barras, líneas, áreas, circulares, de anillo y de dispersión, cada uno con sus series y las celdas que leen (en esta hoja o en otra), categorías, título, leyenda, ejes (títulos, valor mínimo y máximo, formato de número), etiquetas de datos, colores y ubicación. Un archivo .ods creado por otro programa se abre con sus gráficos, dibujados a partir de las celdas que leen. Lo que un gráfico .ods no puede contener se omite y se indica al guardar: líneas de tendencia, barras de error, ejes secundarios y los aspectos especiales de un solo punto. Los gráficos de tipos que JeffCrunch no dibuja (bursátiles y de radar) se abren como columnas con sus valores.
