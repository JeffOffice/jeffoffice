# Formato condicional

<!-- keywords: formato condicional, conditional formatting, regla, resaltar, escala de colores, barra de datos, conjunto de iconos, iconos, superior, inferior, superior al promedio, inferior al promedio, duplicado, único, fórmula, detener si es verdadero, prioridad, barras, flechas, semáforos, degradado, formato diferencial, botón Formato condicional, borrar reglas, mayor que, menor que, entre, texto que contiene, valores duplicados, top 10 -->

Un **formato condicional** es una regla que cambia el aspecto de una celda cuando su contenido cumple una condición. Por ejemplo, una regla dice «colorear una celda de rojo si es menor que 100». Una celda con 80 se ve roja, una celda con 120 no, y si cambia 80 por 150, el rojo desaparece por sí solo. El formato propio de la celda no se modifica: la regla solo decide qué se dibuja.

## Qué muestra JeffCrunch

JeffCrunch muestra las reglas que tiene un libro:

- **Reglas de resaltado**: una regla que cambia la fuente, el relleno, el borde o el formato de número de las celdas que cumplen la condición. La condición puede ser una comparación con un valor (mayor que, entre, igual a, etc.), una fórmula, un texto que contiene, empieza por o termina en determinadas palabras, una fecha como «la semana pasada», celdas en blanco o con error, valores que están entre los más altos o más bajos (un número o un porcentaje), valores superiores o inferiores al promedio, y valores duplicados o únicos.
- **Escalas de colores**: cada celda se rellena con un color que pasa entre dos o tres colores según su número vaya de bajo a alto. Una escala de blanco a verde hace que los números más grandes sean los más verdes.
- **Barras de datos**: una barra dentro de la celda que es tanto más larga cuanto mayor es el número. Las barras pueden ser de color sólido o degradarse, pueden tener borde, pueden tener eje y pueden ir en sentido contrario, con otro color, para los números negativos.
- **Conjuntos de iconos**: un pequeño dibujo en la celda que indica en qué parte del intervalo está el número, como una flecha hacia arriba, una flecha horizontal o una flecha hacia abajo; o una luz roja, amarilla o verde. Hay veinte conjuntos.

Cuando dos reglas afectan a la misma celda, la que está antes en el orden del libro gana en lo que ambas establecen, y una regla marcada como «detener si es verdadero» termina la búsqueda. Los números de una escala de colores, una barra de datos o un conjunto de iconos se calculan sobre las celdas propias de la regla, y se vuelven a calcular cuando cambia una de esas celdas.

## Lo que se mantiene igual

- Las reglas siguen a las celdas a las que pertenecen. Si inserta o elimina filas o columnas, corta y pega, las reglas siguen cubriendo las celdas correctas; **Deshacer** las devuelve a su sitio.
- Una regla que JeffCrunch no dibuja se conserva en el archivo tal como estaba.
- Al guardar se escriben las reglas que tenía el libro. Cuando crea o borra reglas en una hoja, las reglas de la hoja se escriben de nuevo con sus cambios.

## Crear una regla

1. Seleccione las celdas.
2. Haga clic en **Inicio > Formato condicional**.
3. En **Reglas para resaltar celdas**, elija **Mayor que**, **Menor que**, **Entre**, **Igual a**, **Texto que contiene** o **Valores duplicados**. En **Reglas superiores e inferiores**, elija **10 elementos superiores**, **10 elementos inferiores**, **10 % superior**, **10 % inferior**, **Superior al promedio** o **Inferior al promedio**.
4. Escriba el valor (o los dos valores para **Entre**, el texto para **Texto que contiene**, o cuántos para las reglas superiores e inferiores). En **10 elementos superiores**, «10» es el número de celdas que recibirán el formato. Puede cambiarlo por cualquier número.
5. Elija el aspecto de las celdas, por ejemplo **Relleno rojo claro con texto rojo oscuro**, y haga clic en **Aceptar**.

La regla se dibuja en el momento. Cada regla nueva va en primer lugar, así que, si dos reglas quieren cambiar lo mismo, gana la más reciente. Crear una regla es un paso que puede deshacer.

## Borrar reglas

**Quitar reglas de las celdas seleccionadas** elimina todas las reglas que haya en cualquiera de las celdas seleccionadas. **Quitar reglas de esta hoja** elimina todas las reglas de la hoja. Cada una es un paso que puede deshacer. Si no hay ninguna regla que borrar, la opción no hace nada.

## Lo que todavía no está hecho

- Las barras de datos, las escalas de colores y los conjuntos de iconos se pueden mostrar, pero todavía no se pueden crear. La ventana que enumera, ordena y edita todas las reglas (**Administrar reglas**) todavía no está disponible. Una regla que usa una fórmula no se puede crear desde el menú todavía.
- Una hoja que tiene barras de datos de una versión más reciente del archivo las conserva tal como están, y JeffCrunch no crea ni borra reglas en esa hoja. Haga esos cambios en el programa que creó el libro.
- Un libro abierto desde un tipo de archivo más antiguo conserva sus reglas al guardarlo como libro .xlsx, y después puede añadir reglas.
