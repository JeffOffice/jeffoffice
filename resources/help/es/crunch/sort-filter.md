# Ordenar y filtrar
<!-- keywords: ordenar, ordenación, filtrar, filtro, autofiltro, ascendente, descendente, A-Z, reglas, encabezado, color, lista, meses, días de la semana, mayúsculas, columnas, mayores, menores, promedio, entre, comienza por, contiene, fecha, borrar, volver a aplicar, lista desplegable, filas ocultas, tabla, Ctrl+Mayús+L -->

## Ordenar

Seleccione una celda de los datos y elija **Ordenar de A a Z** u **Ordenar de Z a A** (**Ordenar y filtrar** en la pestaña Inicio). JeffCrunch toma todo el bloque de datos que rodea la celda, conserva una fila de encabezado si la encuentra (texto sobre números) y ordena las filas según la columna en la que se encuentra. Si hay varias celdas seleccionadas, ordena solo esas; si hay datos junto a ellas, le pregunta si desea incluirlos.

En la ordenación, primero van los números, luego el texto, después VERDADERO y FALSO y, por último, los errores; las celdas vacías siempre van al final. Las mayúsculas y las minúsculas se consideran iguales, salvo que indique lo contrario. El texto se ordena de la misma forma en todas partes: según las letras del inglés, con cada letra acentuada junto a la letra sin acento, sea cual sea el idioma del equipo.

Elija **Ordenar…** para abrir el cuadro de diálogo Ordenar:

- Agregue una regla por cada columna por la que desee ordenar. Una regla posterior resuelve los empates de las anteriores. Use **Subir** y **Bajar** para cambiar el orden de las reglas, y **Duplicar regla** para partir de una existente.
- Ordene **Según** los valores, el **color de relleno** de las celdas o su **color del texto**. Si elige un color, indique si va **Al principio** o **Al final**; las demás filas conservan su orden. Un color es el color que resulta: uno elegido en la paleta y otro con los mismos valores de rojo, verde y azul son el mismo.
- Para los valores, el orden puede ser **A–Z**, **Z–A** o una lista: los días de la semana y los meses (completos y abreviados, en inglés y en su idioma), o una lista que escriba usted, con comas entre los valores. Los valores que no están en la lista se colocan después de los que sí están.
- **La primera fila contiene los nombres de las columnas** deja la primera fila fuera de la ordenación. **Distinguir mayúsculas y minúsculas** hace que las mayúsculas y las minúsculas sean distintas (primero las minúsculas). **Ordenar columnas, no filas** ordena las columnas según los valores de una fila.

Una fórmula se mueve con su fila, y sus referencias a celdas de la misma fila permanecen en esa fila. Una referencia a una celda de otra fila cambia como si se copiara la fórmula. Lo que está asociado a una celda se mueve con ella: el formato de la celda, su nota y su vínculo se trasladan al lugar al que va la celda. Las reglas que abarcan un rango de celdas (lo que se puede escribir y los formatos condicionales) permanecen donde están, ya que abarcan las posiciones y no las celdas. La ordenación es un solo paso que puede deshacer, y **Esc** detiene una ordenación grande; las ordenaciones grandes se aplican poco a poco.

Algunas celdas no se pueden ordenar, y se le indica el motivo: celdas bloqueadas en una hoja protegida, celdas que forman parte de una matriz, celdas combinadas que no tienen todas el mismo tamaño (o que abarcan más de una fila), celdas combinadas cortadas por el borde de la selección y la fila de encabezado de una tabla.

## Filtrar

Seleccione una celda de los datos y elija **Filtro** (**Ctrl+Mayús+L**). Aparece un botón en cada celda de la primera fila. Haga clic en uno para ver:

- **Ordenar de A a Z** y **Ordenar de Z a A**, y **Ordenar por color**.
- Una lista de los valores de la columna con casillas, un cuadro de búsqueda y **(Todo)**. Una columna de fechas se muestra por año, mes y día, y los valores que no son fechas aparecen debajo. Desactive lo que no desee y haga clic en **Aceptar**. La búsqueda muestra los valores que contienen todas las palabras escritas; **(Todos los resultados)** activa o desactiva solo esos valores. La lista abarca toda la columna. Si la columna tiene más valores distintos de los que caben en la lista, se indica y la lista se trunca; use una condición para el resto.
- **Filtrar por color**, por color de relleno o color del texto.
- **Condiciones de número**, **Condiciones de texto** o **Condiciones de fecha**: es igual a, mayor que, mayor o igual que, menor que, menor o igual que, entre, los mayores o menores (elementos o porcentaje), por encima o por debajo del promedio, comienza por, termina en, contiene, fechas de un período (hoy, esta semana, mes siguiente, trimestre anterior, en lo que va de año, un trimestre o un mes de cualquier año) y **Condición personalizada**, formada por dos comparaciones unidas con Y u O. En el texto de una condición, ? y \* representan un carácter cualquiera y cualquier serie de caracteres.
- **Mostrar todo de** la columna.

Un botón con un embudo indica una columna con filtro. Las filas que no coinciden se ocultan y conservan sus números de fila. **Borrar** (en el menú **Ordenar y filtrar**) quita todas las condiciones y muestra las filas; **Volver a aplicar** vuelve a evaluar las condiciones después de cambiar un valor. Si vuelve a elegir **Filtro**, se quitan los botones.

Las filas ocultas por un filtro se tratan aparte de las que ha ocultado usted: **Mostrar filas** no muestra una fila que oculta el filtro, y borrar el filtro no muestra una fila que ocultó usted. Al ordenar un bloque filtrado, solo se ordenan entre sí las filas visibles.

Un filtro funciona en toda una hoja, hasta 1.048.576 filas, poco a poco; **Borrar** y desactivar el filtro funcionan sea cual sea el tamaño. Lo que se le pide a un filtro está limitado a 40 millones de comprobaciones (las filas multiplicadas por las columnas que tienen una condición, más algunas por los valores que enumeran las condiciones); si un filtro pide más, se indica y no se crea; puede filtrar menos filas o usar menos condiciones. Un filtro de la hoja no abarca las celdas de una tabla: una tabla tiene sus propios botones, que funcionan del mismo modo (consulte [Tablas](tables)).

Una lista de valores de una condición se guarda tal como la escribe el archivo, sin idioma en los números ni en las fechas (1,234.50), y un valor coincide con una celda que lo muestra en su idioma o de esa forma; un archivo filtrado en un idioma filtra las mismas celdas en otro.

El filtro se guarda en el archivo con sus condiciones y las filas que oculta, por lo que el archivo se abre con las mismas filas visibles. Al volver a abrir un archivo, aparece el embudo en todas las columnas que tienen una condición; las filas ocultas dentro del rango del filtro se consideran del filtro.
