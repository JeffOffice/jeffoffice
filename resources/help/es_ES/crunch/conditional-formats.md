# Formatos condicionales
<!-- keywords: formato condicional, formatos condicionales, regla, resaltar, escala de colores, barra de datos, conjunto de iconos, iconos, superior, inferior, superior al promedio, inferior al promedio, duplicado, único, fórmula, detener si es verdadero, prioridad, barras, flechas, semáforos, degradado, formato diferencial, botón Formato condicional, borrar reglas, mayor que, menor que, entre, texto que contiene, valores duplicados, top 10 -->

Un **formato condicional** es una regla que cambia el aspecto de una celda cuando su contenido cumple una condición. Por ejemplo, una regla dice «colorear una celda de rojo si su valor es inferior a 100». Una celda con 80 se ve roja, una celda con 120 no, y si cambia 80 por 150, el rojo desaparece por sí solo. El formato propio de la celda no se modifica: la regla solo decide cómo se dibuja.

## Qué muestra JeffCrunch

JeffCrunch muestra las reglas que tiene un libro:

- **Reglas de resaltado**: una regla que cambia la fuente, el relleno, el borde o el formato de número de las celdas que la cumplen. La condición puede ser una comparación con un valor (mayor que, entre, igual a, etc.), una fórmula, un texto que contiene, empieza o termina por determinadas palabras, una fecha como «la semana pasada», celdas vacías o con error, valores entre los más altos o más bajos (un número o un porcentaje), valores superiores o inferiores al promedio, y valores duplicados o únicos.
- **Escalas de colores**: cada celda se rellena con un color que pasa entre dos o tres colores según su número vaya de menor a mayor. Una escala de blanco a verde hace que los números más grandes sean los más verdes.
- **Barras de datos**: una barra dentro de la celda, tanto más larga cuanto mayor sea el número. Las barras pueden ser de color sólido o degradado, pueden tener borde y eje, y pueden ir en sentido contrario, con otro color, para los números negativos.
- **Conjuntos de iconos**: un pequeño dibujo en la celda que indica en qué parte del rango está el número, como una flecha hacia arriba, una flecha lateral o una flecha hacia abajo; o un semáforo en rojo, amarillo o verde. Hay veinte conjuntos.

Cuando dos reglas dicen algo sobre la misma celda, gana la que está primero en el orden del libro en lo que ambas establecen, y una regla marcada como «detener si es verdadero» detiene la búsqueda. Los números de una escala de colores, una barra de datos o un conjunto de iconos se calculan sobre las celdas propias de la regla, y se vuelven a calcular cuando cambia alguna de esas celdas.

## Qué se mantiene igual

- Las reglas siguen a las celdas a las que pertenecen. Si inserta o elimina filas o columnas, o si corta y pega, las reglas siguen cubriendo las celdas correctas; **Deshacer** las devuelve a su sitio.
- Una regla que JeffCrunch no dibuja se conserva en el fichero tal como estaba.
- Al guardar se escriben las reglas que tenía el libro. Cuando crea o borra reglas en una hoja, las reglas de esa hoja se vuelven a escribir con sus cambios.

## Crear una regla

1. Seleccione las celdas. Puede seleccionar columnas enteras o toda la hoja; la regla se guarda para el rango, no para cada celda.
2. Haga clic en **Inicio > Formato condicional**.
3. En **Reglas para resaltar celdas**, elija **Mayor que**, **Menor que**, **Entre**, **Igual a**, **Texto que contiene**, **Una fecha que se produzca** o **Valores duplicados**. En **Reglas superiores e inferiores**, elija **10 elementos superiores**, **10 elementos inferiores**, **10 % superior**, **10 % inferior**, **Superior al promedio** o **Inferior al promedio**.
4. Escriba el valor (o los dos valores para **Entre**, el texto para **Texto que contiene**, o cuántos para las reglas superiores e inferiores), o elija las fechas. En **10 elementos superiores**, «10» es el número de celdas que recibirán el formato. Puede cambiarlo por cualquier número.
5. Elija el aspecto de las celdas, por ejemplo **Relleno rojo claro con texto rojo oscuro**, y haga clic en **Aceptar**. La muestra junto a la lista muestra el aspecto.

Para crear un aspecto propio, elija **Formato personalizado...** al final de la lista. Se abre un cuadro con cuatro pestañas: **Número**, **Fuente** (estilo, subrayado, tachado y color), **Borde** (solo tipos de línea finos) y **Relleno** (un color, o una trama con dos colores). El cuadro parte del aspecto que había elegido, y lo que no cambie se mantiene como estaba. Haga clic en **Aceptar** y la muestra mostrará su aspecto.

La regla se aplica de inmediato. Cada regla nueva va en primer lugar, así que, si dos reglas quieren cambiar lo mismo, gana la más reciente. Crear una regla es un solo paso que puede deshacer.

## Barras de datos, escalas de colores y conjuntos de iconos

Haga clic en **Inicio > Formato condicional** y, después, en **Barras de datos**, **Escalas de color** o **Conjuntos de iconos**, y pulse una opción. La regla se aplica de inmediato a las celdas seleccionadas, sin ventana que rellenar, como un solo paso que puede deshacer.

- Las **barras de datos** tienen seis colores, cada uno con relleno degradado o relleno sólido.
- Las **escalas de colores** tienen doce opciones, con dos o tres colores. El nombre indica primero el color de los números más altos.
- Los **conjuntos de iconos** tienen veinte conjuntos: direccionales (flechas y triángulos), formas (semáforos y señales), indicadores (símbolos y banderas) y calificaciones (estrellas, cuartos y cuadros). Los iconos dividen el rango en intervalos iguales.

**Nueva regla**, al final del menú, abre una sola ventana para todo tipo de reglas, incluida una regla con su propia fórmula, y una barra de datos, escala de colores o conjunto de iconos con opciones propias.

## Administrar reglas

**Inicio > Formato condicional > Administrar reglas** muestra las reglas de la hoja en el orden en que se aplican. Use **Nueva regla**, **Editar regla** y **Eliminar regla**; **Subir** y **Bajar** cambian el orden; escriba otras celdas en **Se aplica a**, y marque **Detener si es verdadero**. Editar regla cambia todo lo que tiene una regla:

- en una **barra de datos**: qué representan la barra más corta y la más larga (valor más bajo o más alto, un número, un porcentaje, un percentil o una fórmula), el relleno degradado o sólido, los colores, un borde, el color de las barras negativas, dónde está el eje, en qué sentido van las barras, y **Mostrar solo la barra**;
- en una **escala de colores**: dos o tres colores, y qué representa cada color;
- en un **conjunto de iconos**: el estilo de iconos, el valor en el que empieza cada icono, **Invertir orden de iconos** y **Mostrar solo el icono**;
- en las demás reglas: el valor, el texto, las fechas o la fórmula, y el aspecto. Una regla que un fichero trajo con un aspecto propio aparece con el aspecto **Formato personalizado**, y la muestra enseña cómo se ve de verdad. Si cambia otra cosa de la regla, su aspecto se mantiene exactamente como lo tenía el fichero. Elija **Formato personalizado...** para cambiar el aspecto.

Al pulsar **Aceptar**, todos sus cambios se aplican como un solo paso que puede deshacer. Una regla que no ha cambiado conserva exactamente lo que tenía el fichero, sea cual sea su lugar en la lista; una regla que cambia se vuelve a escribir. La fórmula de una regla se lee desde la celda superior izquierda de su rango, y una fórmula que escriba para una regla nueva se lee como si estuviera escrita en la celda donde está el cursor. Una hoja protegida que no permite dar formato a las celdas no admite reglas.

## Qué conservan otros tipos de fichero

Solo un libro .xlsx conserva los formatos condicionales. Al guardar como un tipo de fichero más antiguo (.xls u .ods), se omiten, y la Comprobación de compatibilidad lo indica, mencionando las barras de datos, las escalas de colores y los conjuntos de iconos cuando la hoja los tiene. Un fichero .csv no conserva ningún formato. Los formatos condicionales se imprimen tal como se ven en pantalla.

## Borrar reglas

**Borrar reglas de las celdas seleccionadas** elimina todas las reglas que estén en cualquiera de las celdas seleccionadas. **Borrar reglas de esta hoja** elimina todas las reglas de la hoja. **Borrar reglas de esta tabla** y **Borrar reglas de esta tabla dinámica** eliminan las reglas que están en la tabla (o tabla dinámica) donde se encuentran las celdas seleccionadas, y solo están disponibles cuando la selección está dentro de una. Cada una es un solo paso que puede deshacer. Una opción no hace nada cuando no hay ninguna regla que borrar.

## Lo que todavía no está disponible

- Una hoja que tiene barras de datos en el formato de un fichero más reciente las conserva tal como están, y JeffCrunch no crea, cambia ni borra reglas en esa hoja. Haga esos cambios en el programa con el que se creó el libro.
- Un libro abierto desde un tipo de fichero más antiguo conserva sus reglas al guardarlo como libro .xlsx, y después puede añadir reglas.
- Un aspecto propio no puede tener una línea media, gruesa, doble o inclinada, una línea diagonal, un relleno degradado, ni un tipo o tamaño de letra: esas cosas no forman parte de lo que una regla puede cambiar.
