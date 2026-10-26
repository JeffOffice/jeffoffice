# Tablas
<!-- keywords: tabla, tablas, referencia estructurada, fila de encabezado, encabezados, fila de totales, totales, columna calculada, con bandas, filas alternas, estilo, apariencia de tabla, formato de tabla, botones de filtro, filtro, Ctrl+T, convertir, rango, celdas normales, cambiar tamaño, nombre de columna, [@Columna], Tabla1[Columna] -->

Una tabla es un bloque de celdas con una fila de nombres sobre sus columnas que se trata como un único elemento: tiene un nombre, una apariencia y botones para filtrar y ordenar sus filas, y crece cuando se le agregan datos. Las fórmulas pueden hacer referencia a sus columnas por su nombre en lugar de por la dirección de las celdas.

## Crear una tabla

Seleccione las celdas, o haga clic en una celda de los datos, y elija **Tabla** en la pestaña **Insertar** (**Ctrl+T**). JeffCrunch pregunta qué celdas abarca la tabla y si la primera fila contiene los nombres de las columnas. Los nombres que son números, fechas o fórmulas se convierten en el texto que muestran; los nombres vacíos pasan a ser Columna1, Columna2, etc., y si dos columnas tienen el mismo nombre, se numeran. Si no hay nombres, la tabla no tiene fila de encabezado y, en las fórmulas, sus columnas se llaman Columna1, Columna2, etc.

La galería **Apariencia de tabla** (en la pestaña **Inicio** o en la pestaña **Tabla**) muestra las apariencias disponibles: claras, medias y oscuras, cada una en negro y en los seis colores del tema del libro. Si elige una para celdas que no forman una tabla, se convierten en tabla. La apariencia se dibuja por debajo del formato propio de cada celda: se conservan el relleno, los bordes, el color del texto o la negrita que haya aplicado a una celda.

## La pestaña Tabla

La pestaña **Tabla** se muestra mientras el cursor está en una tabla.

- **Nombre** es el nombre con el que las fórmulas hacen referencia a la tabla. Todas las fórmulas que usan el nombre anterior se actualizan. Un nombre contiene letras, dígitos, guiones bajos y puntos, empieza por una letra o un guion bajo, y no puede coincidir con el de otra tabla, con un nombre del libro ni con una dirección de celda como A1.
- **Fila de encabezado**, **Fila de totales**, **Filas con bandas**, **Columnas con bandas**, **Primera columna**, **Última columna** y **Botones de filtro** se activan o desactivan para la tabla.
- **Cambiar tamaño de tabla…** pide las celdas que abarca la tabla. Se mantiene su primera celda; se pueden agregar o quitar filas y columnas por abajo y por la derecha.
- **Convertir en celdas normales** convierte la tabla en celdas normales, que conservan su contenido. Las fórmulas que hacían referencia a la tabla o a sus columnas pasan a hacer referencia a las celdas.

## El menú de las celdas

Haga clic con el botón derecho en una celda de una tabla y elija **Tabla**: **Insertar fila encima**, **Insertar fila debajo**, **Insertar columna a la izquierda**, **Insertar columna a la derecha**, **Eliminar filas de la tabla**, **Eliminar columnas de la tabla**, **Fila de totales** y **Convertir en celdas normales**. Estos comandos insertan o eliminan filas y columnas enteras de la hoja, como Insertar y Eliminar; una fila o columna insertada justo después del final de la tabla pasa a formar parte de ella.

## Fila de totales

Active **Fila de totales** y la fila situada bajo la tabla mostrará la palabra Total en su primera columna y un total en la última. Cada celda de esa fila tiene un botón para elegir lo que muestra el total de la columna (el promedio, un recuento, cuántos números hay, el máximo, el mínimo, la suma, la desviación estándar, la varianza o **Fórmula personalizada**). Un total es un SUBTOTALES de su columna, por lo que excluye las filas que oculta un filtro.

## Agregar datos a una tabla

- Escriba en la fila situada bajo una tabla o en la columna contigua, y la tabla crecerá para incluir esa celda. **Deshacer** anula el crecimiento y conserva lo que escribió.
- Una fórmula escrita en una celda de una columna que, por lo demás, está vacía se coloca en todas las celdas de esa columna: es una columna calculada. Si usa **Deshacer** justo después, la fórmula queda solo en esa celda. Una fórmula escrita sobre una columna calculada cambia toda la columna si las demás celdas contienen la fórmula anterior; si alguna no la contiene, la celda queda como excepción.
- Las filas insertadas dentro de una tabla reciben las fórmulas de las columnas calculadas. Las columnas insertadas dentro de ella reciben un nombre en la fila de encabezado. Al eliminar filas o columnas dentro de la tabla, esta se reduce; no se puede eliminar la fila de encabezado ni todas las columnas.

## Filtrar y ordenar una tabla

Los botones de los encabezados filtran y ordenan solo las filas de la tabla. Funcionan igual que los botones del filtro de una hoja (consulte [Ordenar y filtrar](sort-filter)). Una tabla no se puede filtrar mientras un filtro de la hoja, o de otra tabla, abarque las mismas filas, ya que cada uno necesitaría saber qué filas le pertenecen.

## Fórmulas que hacen referencia a una tabla

- `Ventas[Importe]` son las celdas de la columna Importe, sin su nombre ni su total.
- `[@Importe]` es la celda de la propia fila de la fórmula, y en el archivo se escribe `Ventas[[#This Row],[Importe]]`.
- `Ventas[#Headers]`, `Ventas[#Data]`, `Ventas[#Totals]` y `Ventas[#All]` son los encabezados, los datos, los totales y toda la tabla, y `Ventas[[#Headers],[Importe]]` es una sola columna de ellos.
- `Ventas[[Región]:[Importe]]` son las columnas desde Región hasta Importe.

Escriba `[` después del nombre de una tabla y aparecerá una lista de sus columnas. Mientras escribe una fórmula, al hacer clic en una celda de una tabla se escribe una referencia como `Ventas[@Importe]` o `Ventas[Importe]` (si hace clic en toda una columna de sus datos). Al cambiar el nombre de una columna en su encabezado, cambia en todas las fórmulas; al insertar y eliminar columnas, las fórmulas siguen apuntando a las columnas que nombraban, y una columna eliminada deja #¡REF! en las fórmulas que la nombraban.
