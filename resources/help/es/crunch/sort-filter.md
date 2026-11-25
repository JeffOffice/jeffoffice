# Ordenar y filtrar
<!-- keywords: ordenar, filtrar, filtro automático, ascendente, descendente, A-Z, reglas, encabezado, color, lista, meses, días de la semana, mayúsculas, columnas, mayor, menor, promedio, entre, comienza por, contiene, fecha, borrar, volver a aplicar, lista desplegable, filas ocultas, tabla, Ctrl+Mayús+L -->

## Ordenar

Seleccione una celda de sus datos y elija **Ordenar de A a Z** u **Ordenar de Z a A** (**Ordenar y filtrar** en la ficha Inicio). JeffCrunch toma todo el bloque de datos alrededor de la celda, conserva una fila de encabezados donde la encuentra (texto sobre números) y pone las filas en orden según la columna en la que se encuentra. Con varias celdas seleccionadas, ordena solo esas; si hay datos junto a ellas, pregunta si deben incluirse también.

En el orden, los números van primero, después el texto, luego VERDADERO y FALSO, y después los errores; las celdas vacías siempre van al final. Las mayúsculas y las minúsculas se consideran iguales, salvo que indique lo contrario. El texto se ordena de la misma forma en todas partes: según las letras del alfabeto inglés, colocando una letra con acento junto a su versión sin acento, sea cual sea el idioma del equipo.

Elija **Ordenar…** para abrir el cuadro de diálogo Ordenar:

- Agregue una regla por cada columna por la que ordenar. Una regla posterior desempata las anteriores. Use **Subir** y **Bajar** para cambiar el orden de las reglas, y **Duplicar regla** para partir de una existente.
- Ordene **Según** los valores, el **color de relleno** de las celdas o su **color de fuente**. Para un color, elija si va **Primero** o **Último**; las demás filas conservan su orden. Un color es el color que resulta: uno elegido de la paleta y otro con los mismos valores de rojo, verde y azul son el mismo.
- Para los valores, el orden puede ser **De A a Z**, **De Z a A** o una lista: los días de la semana y los meses (completos y abreviados, en inglés y en su idioma), o una lista que escriba, con comas entre los valores. Los valores que no están en la lista van después de los que sí lo están.
- **La primera fila contiene los nombres de las columnas** deja la primera fila fuera de la ordenación. **Distinguir mayúsculas y minúsculas** hace que las mayúsculas y las minúsculas sean diferentes (primero las minúsculas). **Ordenar columnas, no filas** ordena las columnas según los valores de una fila. **Los números en el texto se cuentan por valor** coloca «Elemento 2» antes que «Elemento 10» (sin esta opción, las letras y los dígitos se comparan uno a uno, y «Elemento 10» va primero). La opción se aplica a la ordenación que está haciendo, y la ventana la recuerda para la siguiente ordenación y para **Volver a aplicar**; el archivo no la guarda, así que **Volver a aplicar** después de abrir el archivo de nuevo ordena de la forma habitual, y **Comprobar compatibilidad** lo indica cuando una ordenación guardada de un filtro o de una tabla se hizo de esta manera (una ordenación posterior normal la quita). Los números de cualquier longitud se cuentan por valor.

Una fórmula se desplaza con su fila, y sus referencias a celdas de la misma fila permanecen en la fila. Una referencia a una celda de otra fila se cambia como si la fórmula se hubiera copiado. Lo que está vinculado a una celda va con ella: el formato de la celda, su nota y su vínculo se mueven al lugar al que va la celda. Las reglas que cubren un intervalo de celdas (qué puede escribirse y los formatos condicionales) permanecen donde están, porque cubren los lugares y no las celdas. Ordenar es un paso que puede deshacer, y **Esc** detiene una ordenación grande; una ordenación grande se realiza poco a poco.

Algunas celdas no se pueden ordenar, y se le indica el motivo: celdas bloqueadas en una hoja protegida, parte de una matriz, celdas combinadas que no tienen todas el mismo tamaño (o que abarcan más de una fila), celdas combinadas cortadas por el borde de la selección y la fila de encabezados de una tabla.

## Filtrar

Seleccione una celda de sus datos y elija **Filtro** (**Ctrl+Mayús+L**). Aparece un botón en cada celda de la primera fila. Haga clic en uno para ver:

- **Ordenar de A a Z** y **Ordenar de Z a A**, y **Ordenar por color**.
- Una lista de los valores de la columna con casillas de verificación, un cuadro de búsqueda y **(Todo)**. Una columna de fechas se muestra por año, mes y día, y los valores que no son fechas aparecen debajo de ellas. Desmarque lo que no quiera y luego **Aceptar**. Al buscar se muestran los valores que contienen todas sus palabras; **(Todos los resultados)** marca o desmarca solo esos. La lista abarca toda la columna. Si la columna tiene más valores distintos de los que cabe en la lista, se indica y la lista se corta; use una condición para el resto.
- **Filtrar por color**, por color de relleno o color de fuente.
- **Condiciones de número**, **Condiciones de texto** o **Condiciones de fecha**: igual a, mayor que, mayor o igual que, menor que, menor o igual que, entre, los mayores o menores (elementos o porcentaje), por encima o por debajo del promedio, comienza por, termina en, contiene, fechas de un período (hoy, semana actual, mes siguiente, trimestre anterior, en lo que va de año, un trimestre o un mes de cualquier año), y **Condición personalizada** de dos comparaciones unidas por Y u O. En el texto de una condición, ? y \* representan cualquier carácter y cualquier secuencia de caracteres, respectivamente.
- **Mostrar todo** de la columna.

Un botón con un embudo indica una columna que tiene un filtro. Las filas que no coinciden quedan ocultas y conservan su número de fila. **Borrar** (en el menú **Ordenar y filtrar**) quita todas las condiciones y muestra las filas; **Volver a aplicar** calcula de nuevo las condiciones después de cambiar un valor. **Filtro** vuelve a desactivar los botones.

Las filas ocultas por un filtro se distinguen de las que oculta usted: **Mostrar filas** no muestra una fila que oculta el filtro, y borrar el filtro no muestra una fila que usted ocultó. Al ordenar un bloque filtrado, solo se ordenan entre sí las filas visibles.

Un filtro funciona sobre una hoja completa, de hasta 1.048.576 filas, poco a poco; **Borrar** y desactivar el filtro funcionan con cualquier tamaño. Lo que se pide a un filtro está limitado a 40 millones de comprobaciones (las filas por las columnas que tienen una condición, más algunas para los valores que listan las condiciones); un filtro que pide más lo indica y no se aplica, y puede filtrar menos filas o usar menos condiciones. Un filtro de la hoja no abarca las celdas de una tabla: una tabla tiene sus propios botones, que funcionan igual (véase [Tablas](tables)).

Una lista de valores en una condición se guarda como la escribe un archivo, sin idioma en los números y las fechas (1,234.50), y un valor coincide con una celda que lo muestra en su idioma o de esa forma; un archivo filtrado en un idioma filtra las mismas celdas en otro.

El filtro se guarda en el archivo con sus condiciones y las filas que oculta, de modo que el archivo se abre con las mismas filas visibles. Al volver a abrir un archivo aparece el embudo en cada columna que tiene una condición; las filas ocultas en el intervalo del filtro se cuentan como del filtro.
