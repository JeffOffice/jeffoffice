# Tablas
<!-- keywords: tabla, tablas, quitar duplicados, duplicados, resumir, borrar estilo, referencia estructurada, fila de encabezados, fila de totales, columna calculada, con bandas, bandas, estilo, aspecto de tabla, botones de filtro, Ctrl+T, convertir, celdas normales, cambiar tamaño, nombre de columna, [@Columna], Tabla1[Columna] -->

Una tabla es un bloque de celdas con una fila de nombres encima de sus columnas, que se trata como una sola cosa: tiene un nombre, un aspecto, botones para filtrar y ordenar sus filas, y crece cuando le añade datos. Las fórmulas pueden llamar a sus columnas por su nombre en lugar de por la dirección de celda.

## Crear una tabla

Seleccione las celdas, o haga clic en una celda de los datos, y elija **Tabla** en la ficha **Insertar** (**Ctrl+T**). JeffCrunch le pregunta qué celdas abarca la tabla y si la primera fila contiene los nombres de las columnas. Los nombres que son números, fechas o fórmulas se convierten en el texto que muestran, los nombres vacíos pasan a ser Columna1, Columna2, y así sucesivamente, y dos columnas con el mismo nombre se numeran. Si no hay nombres, la tabla no tiene fila de nombres, y sus columnas se llaman Columna1, Columna2, y así sucesivamente en las fórmulas.

La galería **Aspecto de tabla** (ficha **Inicio**, o ficha **Tabla**) muestra los aspectos (**Borrar**, debajo de la galería en la ficha **Tabla**, quita el aspecto de la tabla): claro, medio y oscuro, cada uno en negro y en los seis colores del tema del libro. Si elige uno para celdas que no son una tabla, estas se convierten en una. El aspecto se dibuja debajo de lo que tenga cada celda por sí misma: un relleno, bordes, un color de texto o negrita que haya aplicado a una celda se conserva.

## La ficha Tabla

La ficha **Tabla** aparece mientras el cursor está en una tabla.

- **Nombre** es el nombre con el que las fórmulas llaman a la tabla. Todas las fórmulas que usan el nombre anterior se actualizan. Un nombre tiene letras, dígitos, guiones bajos y puntos, empieza por una letra o un guion bajo, y no es el de otra tabla, un nombre del libro ni una dirección de celda como A1.
- **Fila de nombres**, **Fila de totales**, **Filas con bandas**, **Columnas con bandas**, **Primera columna**, **Última columna** y **Botones de filtro** están activados o desactivados para la tabla.
- **Cambiar tamaño de tabla…** pide las celdas que abarca la tabla. Conserva su primera celda; se pueden añadir filas y columnas por abajo y a la derecha, o quitarlas.
- **Resumir con tabla dinámica** crea una tabla dinámica a partir de las celdas de la tabla (véase [Tablas dinámicas](pivot-tables)). La tabla dinámica lee la tabla por su nombre, así que la sigue cuando crece.
- **Quitar duplicados…** elimina las filas que se repiten. Una fila se repite cuando muestra lo mismo, en todas las columnas que marque, que una fila anterior. Las mayúsculas y las minúsculas se consideran iguales. Se conserva la primera de las filas, y las filas situadas debajo de una fila eliminada suben. La tabla se acorta, y su fila de totales sube con ella. Un **Deshacer** las restaura todas. Las celdas situadas debajo de la tabla no se mueven. Las fórmulas que apuntan a las celdas de la tabla, en cualquier hoja, siguen a las celdas como ocurre al eliminar filas; una fórmula que apuntaba a una fila eliminada muestra #¡REF!. No funciona en una tabla con un filtro activado.
- **Convertir en celdas normales** convierte la tabla en celdas corrientes. Conservan lo que contienen. Una fórmula que nombraba la tabla o sus columnas pasa a nombrar las celdas.

## El menú de las celdas

Haga clic con el botón derecho en una celda de una tabla y elija **Tabla**: **Insertar arriba**, **Insertar debajo**, **Insertar columnas a la izquierda**, **Insertar columnas a la derecha**, **Eliminar filas de la tabla**, **Eliminar columnas de la tabla**, **Fila de totales** y **Convertir en celdas normales**. Estas opciones añaden o quitan filas y columnas completas de la hoja, como hacen Insertar y Eliminar; una fila añadida justo después del final de la tabla pasa a formar parte de ella.

## Fila de totales

Active **Fila de totales** y la fila debajo de la tabla muestra la palabra Total en su primera columna y un total en la última. Cada celda de la fila tiene un botón: elija lo que muestra el total de la columna (el promedio, un recuento, cuántos números hay, el mayor, el menor, la suma, la desviación estándar, la varianza o **Fórmula personalizada**). Un total es un SUBTOTAL de su columna, así que no incluye las filas que oculta un filtro.

## Añadir datos a una tabla

- Escriba en la fila debajo de una tabla, o en la columna junto a ella, y la tabla crece para incluir la celda. **Deshacer** elimina el crecimiento y deja lo que escribió.
- Una fórmula escrita en una celda de una columna que de otro modo está vacía se copia en todas las celdas de esa columna: una columna calculada. Use **Deshacer** justo después, y la fórmula se queda solo en la celda. Una fórmula escrita sobre una columna calculada cambia toda la columna si las demás celdas tienen la fórmula anterior; si algunas no la tienen, la celda queda independiente.
- Las filas insertadas dentro de una tabla reciben las fórmulas de las columnas calculadas. Las columnas insertadas dentro de ella se nombran en la fila de nombres. Al eliminar filas o columnas dentro de la tabla, esta se hace más pequeña; no se pueden eliminar a la vez la fila de nombres y todas las columnas.

## Filtrar y ordenar una tabla

Los botones de los nombres filtran y ordenan solo las filas de la tabla. Funcionan igual que los botones del filtro de una hoja (véase [Ordenar y filtrar](sort-filter)). No se puede filtrar una tabla mientras un filtro de la hoja, o de otra tabla, afecte a las mismas filas, ya que cada uno necesitaría saber cuáles son sus propias filas.

## Fórmulas que nombran una tabla

- `Ventas[Importe]` son las celdas de la columna Importe, sin su nombre ni su total.
- `[@Importe]` es la celda de la fila propia de la fórmula, y se escribe `Ventas[[#Esta fila],[Importe]]` en el archivo.
- `Ventas[#Encabezados]`, `Ventas[#Datos]`, `Ventas[#Totales]` y `Ventas[#Todo]` son los nombres, los datos, los totales y la tabla completa, y `Ventas[[#Encabezados],[Importe]]` es una columna de ellos.
- `Ventas[[Región]:[Importe]]` son las columnas desde Región hasta Importe.

Escriba `[` después del nombre de una tabla y aparece una lista de sus columnas. Mientras escribe una fórmula, al hacer clic en una celda de una tabla se escribe una referencia como `Ventas[@Importe]` o `Ventas[Importe]` (si hace clic en una columna entera de sus datos). Si cambia el nombre de una columna en su encabezado, el cambio se aplica en todas las fórmulas; insertar y eliminar columnas mantiene las fórmulas apuntando a las columnas que nombraban, y una columna eliminada deja #¡REF! en las fórmulas que la nombraban.
