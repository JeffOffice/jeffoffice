# Formatos condicionales
<!-- keywords: formato condicional, formatos condicionales, regla, resaltar, escala de colores, barra de datos, conjunto de iconos, iconos, superior, inferior, promedio superior, promedio inferior, duplicado, único, fórmula, detener si es verdadero, prioridad, barras, flechas, semáforos, degradado, formato diferencial, botón Formato condicional, borrar reglas, mayor que, menor que, entre, texto que contiene, valores duplicados, top 10, 10 superiores -->

Un **formato condicional** es una regla que cambia el aspecto de una celda cuando su contenido cumple una condición. Por ejemplo, una regla dice: "colorear una celda de rojo si es menor que 100". Una celda con 80 se ve roja, una celda con 120 no, y si cambia 80 por 150, el rojo desaparece solo. El formato propio de la celda no se modifica: la regla solo decide cómo se dibuja.

## Qué muestra JeffCrunch

JeffCrunch muestra las reglas que tiene un libro:

- **Reglas para resaltar celdas**: una regla que cambia la fuente, el relleno, el borde o el formato de número de las celdas que la cumplen. La condición puede ser una comparación con un valor (mayor que, entre, igual a, etc.), una fórmula, un texto que contiene, comienza por o termina en determinadas palabras, una fecha como "la semana pasada", celdas en blanco o con error, valores que están entre los más altos o más bajos (un número o un porcentaje), valores superiores o inferiores al promedio, y valores duplicados o únicos.
- **Escalas de colores**: cada celda se rellena con un color que cambia entre dos o tres colores según su número vaya de bajo a alto. Una escala de blanco a verde hace que los números más grandes sean más verdes.
- **Barras de datos**: una barra dentro de la celda, tanto más larga cuanto mayor es el número. Las barras pueden ser de color sólido o degradado, pueden tener borde y eje, y pueden ir en sentido contrario, con otro color, para los números negativos.
- **Conjuntos de iconos**: un pequeño dibujo en la celda que indica en qué parte del intervalo está el número, como una flecha hacia arriba, una flecha lateral o una flecha hacia abajo; o una luz roja, amarilla o verde. Hay veinte conjuntos.

Cuando dos reglas afectan a la misma celda, gana la que aparece primero en el orden del libro para lo que ambas establecen, y una regla marcada como "detener si es verdadero" termina la búsqueda. Los números de una escala de colores, una barra de datos o un conjunto de iconos se calculan sobre las celdas propias de la regla, y se vuelven a calcular cuando cambia una de esas celdas.

## Lo que no cambia

- Las reglas siguen a las celdas a las que pertenecen. Si inserta o elimina filas o columnas, corta y pega, las reglas siguen cubriendo las celdas correctas; **Deshacer** las devuelve a su sitio.
- Una regla que JeffCrunch no dibuja se conserva en el fichero tal como estaba.
- Al guardar se escriben las reglas que tenía el libro. Cuando crea o borra reglas en una hoja, las reglas de la hoja se vuelven a escribir con sus cambios.

## Crear una regla

1. Seleccione las celdas. Puede seleccionar columnas completas o toda la hoja; la regla se guarda para el intervalo, no para cada celda.
2. Haga clic en **Inicio > Formato condicional**.
3. En **Reglas para resaltar celdas**, elija **Mayor que**, **Menor que**, **Entre**, **Igual a**, **Texto que contiene**, **Una fecha que se produzca** o **Valores duplicados**. En **Reglas superiores e inferiores**, elija **10 elementos superiores**, **10 elementos inferiores**, **10 % superior**, **10 % inferior**, **Superior al promedio** o **Inferior al promedio**.
4. Escriba el valor (o los dos valores para **Entre**, el texto para **Texto que contiene**, o cuántos elementos para las reglas superiores e inferiores), o elija las fechas. En **10 elementos superiores**, "10" es cuántas celdas recibirán el formato. Puede cambiarlo por cualquier número.
5. Elija el aspecto de las celdas, por ejemplo **Relleno rojo claro con texto rojo oscuro**, y haga clic en **Aceptar**. La muestra junto a la lista muestra el aspecto.

Para crear un formato propio, elija **Formato personalizado...** al final de la lista. Se abre un cuadro con cuatro pestañas: **Número**, **Fuente** (el estilo, el subrayado, el tachado y el color), **Borde** (solo tipos de línea fina) y **Relleno** (un color, o una trama con dos colores). El cuadro parte del aspecto que había elegido, y lo que no cambie se queda como estaba. Haga clic en **Aceptar** y la muestra mostrará su formato.

La regla se dibuja de inmediato. Cada nueva regla va primero, así que si dos reglas quieren cambiar lo mismo, la más reciente gana. Crear una regla es un solo paso que puede deshacer.

## Barras de datos, escalas de colores y conjuntos de iconos

Haga clic en **Inicio > Formato condicional** y, después, en **Barras de datos**, **Escalas de color** o **Conjuntos de iconos**, y haga clic en una opción. La regla se aplica a las celdas seleccionadas de inmediato, sin ventana que rellenar, como un solo paso que puede deshacer.

- Las **barras de datos** tienen seis colores, cada uno como relleno degradado o como relleno sólido.
- Las **escalas de colores** tienen doce opciones, con dos o tres colores. El nombre indica primero el color de los números más altos.
- Los **conjuntos de iconos** tienen veinte conjuntos: direccionales (flechas y triángulos), formas (semáforos y señales), indicadores (símbolos y banderas) y calificaciones (estrellas, cuartos y cuadros). Los iconos dividen el intervalo en partes iguales.

**Nueva regla**, al final del menú, abre una sola ventana para todo tipo de reglas, incluida una regla con su propia fórmula, y una barra de datos, escala de colores o conjunto de iconos con ajustes propios.

## Administrar reglas

**Inicio > Formato condicional > Administrar reglas** muestra las reglas de la hoja en el orden en que se aplican. Use **Nueva regla**, **Editar regla** y **Eliminar regla**; **Subir** y **Bajar** cambian el orden; escriba otras celdas en **Se aplica a**, y marque **Detener si es verdadero**. **Editar regla** cambia todo lo que tenga una regla:

- para una **barra de datos**: lo que representan la barra más corta y la más larga (valor más bajo o más alto, un número, un porcentaje, un percentil o una fórmula), el relleno degradado o sólido, los colores, un borde, el color de las barras negativas, dónde está el eje, en qué sentido van las barras, y **Mostrar solo la barra**;
- para una **escala de colores**: dos o tres colores, y lo que representa cada uno;
- para un **conjunto de iconos**: el estilo de los iconos, el valor en el que empieza cada icono, **Invertir orden de iconos** y **Mostrar solo el icono**;
- para las demás reglas: el valor, el texto, las fechas o la fórmula, y el aspecto. Una regla que un fichero trajo con un formato propio aparece con el formato **Formato personalizado**, y la muestra indica cómo se ve de verdad. Si cambia otra cosa de la regla, su aspecto se queda exactamente como lo tenía el fichero. Elija **Formato personalizado...** para cambiarlo.

Al pulsar **Aceptar** se aplican todos sus cambios como un solo paso que puede deshacer. Una regla que no cambió conserva exactamente lo que tenía el fichero, sea cual sea su lugar en la lista; una regla que cambia se vuelve a escribir. La fórmula de una regla se lee desde la celda superior izquierda de su intervalo, y una fórmula que escriba para una nueva regla se lee como si estuviera escrita en la celda con el cursor. Una hoja protegida que no permite dar formato a las celdas no admite reglas.

## Qué conservan otros tipos de fichero

Solo un libro .xlsx conserva los formatos condicionales. Si guarda como un tipo de fichero más antiguo (.xls u .ods), estos se omiten, y **Comprobar compatibilidad** lo indica, mencionando las barras de datos, escalas de colores y conjuntos de iconos cuando la hoja los tiene. Un fichero .csv no conserva ningún formato. Los formatos condicionales se imprimen tal como se ven en pantalla.

## Borrar reglas

**Borrar reglas de las celdas seleccionadas** elimina todas las reglas que tengan las celdas seleccionadas. **Borrar reglas de esta hoja** elimina todas las reglas de la hoja. Cada una es un solo paso que puede deshacer. Si no hay ninguna regla que borrar, la opción no hace nada.

## Lo que todavía no está hecho

- Una hoja que tiene barras de datos en la forma de un fichero más reciente las conserva tal como están, y JeffCrunch no crea, cambia ni borra reglas en esa hoja. Haga esos cambios en el programa que creó el libro.
- Un libro abierto desde un tipo de fichero más antiguo conserva sus reglas al guardarlo como libro .xlsx, y después puede añadir reglas.
- Un formato propio no puede tener una línea media, gruesa, doble o inclinada, una línea diagonal, un relleno degradado, ni un tipo o tamaño de letra: esas cosas no forman parte de lo que una regla puede cambiar.
