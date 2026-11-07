# Tablas
<!-- keywords: tabla, tablas, quitar duplicados, duplicados, resumir, borrar estilo, referencia estructurada, fila de encabezado, fila de totales, columna calculada, con bandas, rayas, estilo, apariencia de tabla, nuevo estilo de tabla, estilo de tabla personalizado, botones de filtro, Ctrl+T, convertir, celdas normales, cambiar tamaño, nombre de columna, [@Column], Table1[Column] -->

Una tabla es un bloque de celdas con una fila de nombres sobre sus columnas, que se trata como una sola cosa: tiene un nombre, una apariencia, botones para filtrar y ordenar sus filas, y crece cuando se le añade contenido. Las fórmulas pueden referirse a sus columnas por su nombre en lugar de por la dirección de celda.

## Crear una tabla

Seleccione las celdas, o haga clic en una celda de los datos, y elija **Tabla** en la ficha **Insertar** (**Ctrl+T**). JeffCrunch pregunta qué celdas abarca la tabla y si la primera fila contiene los nombres de las columnas. Los nombres que son números, fechas o fórmulas se convierten en el texto que muestran; los nombres vacíos pasan a ser Columna1, Columna2, etc., y las columnas con un mismo nombre se numeran. Si no hay nombres, la tabla no tiene fila de encabezado y sus columnas se llaman Columna1, Columna2, etc. en las fórmulas.

La galería **Apariencia de tabla** (ficha **Inicio** o ficha **Tabla**) muestra las apariencias (**Borrar**, debajo de la galería en la ficha **Tabla**, quita la apariencia de la tabla): clara, media y oscura, cada una en negro y en los seis colores del tema del libro. Si elige una para celdas que no son una tabla, estas se convierten en una. La apariencia se dibuja debajo de lo que una celda tenga propio: un relleno, bordes, un color de texto o negrita que usted haya aplicado a una celda se conserva.

Para crear una apariencia propia, elija **Nuevo estilo de tabla** al final de la galería. Escriba un nombre. Después, elija un color de relleno, un color de texto y negrita para cada parte que quiera cambiar: la fila de encabezado, las filas con bandas, la fila de totales y la primera columna. La nueva apariencia aparece en el grupo **Personalizado** en la parte superior de la galería, y se aplica a la tabla en la que se encuentra. Se guarda en el archivo, así que sigue disponible al volver a abrirlo.

## La ficha Tabla

La ficha **Tabla** aparece mientras el cursor está en una tabla.

- **Nombre** es el nombre con el que las fórmulas llaman a la tabla. Todas las fórmulas que usan el nombre anterior se actualizan. Un nombre tiene letras, dígitos, guiones bajos y puntos, empieza por una letra o un guion bajo, y no coincide con el de otra tabla, con un nombre del libro ni con una dirección de celda como A1.
- **Fila de encabezado**, **Fila de totales**, **Filas con bandas**, **Columnas con bandas**, **Primera columna**, **Última columna** y **Botones de filtro** se marcan o desmarcan para la tabla.
- **Cambiar tamaño de tabla…** pide las celdas que abarca la tabla. Conserva su primera celda; se pueden añadir filas y columnas al final y a la derecha, o quitarlas.
- **Resumir con tabla dinámica** crea una tabla dinámica a partir de las celdas de la tabla (vea [Tablas dinámicas](pivot-tables)). La tabla dinámica lee la tabla por su nombre, así que la sigue cuando crece.
- **Quitar duplicados…** elimina las filas que se repiten. Una fila se repite cuando muestra lo mismo, en todas las columnas que marque, que una fila situada encima. Las mayúsculas y las minúsculas se consideran iguales. Se conserva la primera de las filas, y las filas situadas debajo de una fila eliminada suben. La tabla se hace más corta, y su fila de totales sube con ella. Un **Deshacer** las restaura todas. Las celdas situadas debajo de la tabla no se mueven. Las fórmulas que apuntan a celdas de la tabla, en cualquier hoja, siguen a las celdas como ocurre al eliminar filas; una fórmula que apuntaba a una fila eliminada muestra #¡REF!. No funciona en una tabla con un filtro activo.
- **Convertir en celdas normales** convierte la tabla en celdas normales. Estas conservan lo que contienen. Una fórmula que nombraba la tabla o sus columnas pasa a nombrar las celdas.

## El menú de las celdas

Haga clic con el botón derecho en una celda de una tabla y elija **Tabla**: **Insertar fila encima**, **Insertar fila debajo**, **Insertar columna a la izquierda**, **Insertar columna a la derecha**, **Eliminar filas de la tabla**, **Eliminar columnas de la tabla**, **Fila de totales** y **Convertir en celdas normales**. Estas opciones añaden o quitan filas y columnas completas de la hoja, igual que Insertar y Eliminar; una fila añadida justo después del final de la tabla pasa a formar parte de ella.

## Fila de totales

Active **Fila de totales** y la fila situada debajo de la tabla muestra la palabra Total en su primera columna y un total en la última. Cada celda de la fila tiene un botón: elija lo que muestra el total de la columna (el promedio, un recuento, cuántos números hay, el mayor, el menor, la suma, la desviación estándar, la varianza o **Fórmula personalizada…**). Un total es un SUBTOTALES de su columna, así que no tiene en cuenta las filas que oculta un filtro.

## Añadir contenido a una tabla

- Escriba en la fila situada debajo de una tabla, o en la columna situada a su lado, y la tabla crece para incluir la celda. **Deshacer** quita el crecimiento y deja lo que escribió.
- Una fórmula escrita en una celda de una columna que por lo demás está vacía se copia en todas las celdas de esa columna: una columna calculada. **Deshacer** justo después hace que la fórmula quede solo en la celda. Una fórmula escrita sobre una columna calculada cambia la columna entera cuando las demás celdas contienen la fórmula anterior; si algunas no la contienen, la celda queda independiente.
- Las filas insertadas dentro de una tabla reciben las fórmulas de las columnas calculadas. Las columnas insertadas dentro de ella reciben su nombre en la fila de encabezado. Eliminar filas o columnas dentro de la tabla la hace más pequeña; la fila de encabezado y todas las columnas no se pueden eliminar a la vez.

## Filtrar y ordenar una tabla

Los botones de los nombres filtran y ordenan solo las filas de la tabla. Funcionan igual que los botones del filtro de una hoja (vea [Ordenar y filtrar](sort-filter)). Una tabla no se puede filtrar mientras un filtro de la hoja o de otra tabla afecte a las mismas filas, porque cada uno necesitaría saber cuáles son sus propias filas.

## Fórmulas que nombran una tabla

- `Sales[Amount]` son las celdas de la columna Amount, sin su nombre ni su total.
- `[@Amount]` es la celda de la fila propia de la fórmula, y se escribe `Sales[[#This Row],[Amount]]` en el archivo.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]` y `Sales[#All]` son los nombres, los datos, los totales y la tabla completa, y `Sales[[#Headers],[Amount]]` es una columna de ellos.
- `Sales[[Region]:[Amount]]` son las columnas desde Region hasta Amount.

Escriba `[` después del nombre de una tabla y aparece una lista de sus columnas. Mientras escribe una fórmula, al hacer clic en una celda de una tabla se escribe una referencia como `Sales[@Amount]` o `Sales[Amount]` (si hace clic en una columna completa de sus datos). Cambiar el nombre de una columna en su encabezado lo modifica en todas las fórmulas; insertar y eliminar columnas mantiene las fórmulas apuntando a las columnas que nombraban, y una columna eliminada deja #¡REF! en las fórmulas que la nombraban.
