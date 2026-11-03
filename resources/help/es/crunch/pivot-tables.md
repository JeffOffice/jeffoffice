# Tablas dinámicas
<!-- keywords: tabla dinámica, insertar tabla dinámica, crear, generar, informe, resumen, getpivotdata, importardatosdinamicos, generar getpivotdata, actualizar, actualizar todo, cambiar origen de datos, lista de campos, campos, filas, columnas, valores, filtros, filtro de informe, total general, subtotal, diseño, compacto, esquema, tabular, borrar, configuración de campo de valor, suma, recuento, promedio -->

Una tabla dinámica es un pequeño informe creado a partir de una lista larga. Por ejemplo, la lista tiene una fila por cada venta y el informe suma las ventas por región y por mes. No hace falta escribir fórmulas. Basta con indicar qué columnas de la lista van en el lateral, cuáles van en la parte superior y cuál se suma; el informe se rellena solo.

## Un ejemplo

Esta es una lista en las celdas A1 a C7. Cada fila es una venta.

| Región | Mes | Importe |
|---|---|---|
| Este | Ene | 10 |
| Este | Ene | 5 |
| Este | Feb | 20 |
| Oeste | Ene | 7 |
| Oeste | Feb | 3 |
| Oeste | Feb | 4 |

Una tabla dinámica con **Región** en el lateral, **Mes** en la parte superior y la suma de **Importe** en el centro tiene este aspecto.

| Suma de Importe | Mes | | |
|---|---|---|---|
| Región | Ene | Feb | Total general |
| Este | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total general | 22 | 27 | 49 |

Este en enero es 15 porque se suman las dos filas de Este de enero, 10 y 5. La columna Total general suma cada fila, la fila Total general suma cada columna y la celda de la esquina lo suma todo.

Términos que se usan aquí:

- Un **campo** es una columna de la lista. Región, Mes e Importe son los campos.
- **Filas**, **Columnas** y **Valores** son los tres lugares en los que se puede colocar un campo. Un campo en Filas genera una línea por cada uno de sus valores distintos; un campo en Columnas genera una columna por cada uno; un campo en Valores es el que se suma. Un cuarto lugar, **Filtros**, contiene un campo que excluye las filas que no elija.
- La **caché** (caché de la tabla dinámica) es una copia de la lista que la tabla dinámica guarda dentro del archivo, para poder mostrar los totales sin consultar las celdas. El informe se calcula a partir de la caché, no de las celdas, por lo que no cambia cuando cambian las celdas. **Actualizar** vuelve a copiar las celdas en la caché.

## Crear una tabla dinámica

1. Haga clic en una celda de la lista o seleccione las celdas que desea usar. La lista necesita una fila de nombres encima de los datos.
2. Elija **Insertar** y, a continuación, **Tabla dinámica**.
3. El cuadro muestra las celdas que contienen los datos. Cámbielas si no son correctas. También puede escribir el nombre de una tabla.
4. Elija dónde se colocará la tabla dinámica: **Una hoja nueva** (la opción predeterminada) o una celda de esta hoja u otra. Elija un lugar vacío, porque la tabla vacía ocupa unas dieciocho filas y tres columnas.
5. Elija **Aceptar**. La tabla empieza vacía y se abre el panel **Campos de tabla dinámica**. Active los campos que desee: un campo de números se suma y cualquier otro campo se convierte en una fila. Después, mueva los campos a los cuadros como se explica en las secciones siguientes.

La hoja nueva es un paso independiente de Deshacer, igual que la tabla. Al guardar como .xlsx, la tabla se escribe tal como la escribe el programa original, por lo que allí se abre como tabla dinámica. Una copia guardada como .xls u .ods conserva los números que la tabla muestra en sus celdas, pero no la tabla en sí, y el guardado se lo indica.

## GETPIVOTDATA: leer un número de una tabla dinámica

`GETPIVOTDATA` devuelve el número que muestra una tabla dinámica para los valores y las filas y columnas que indique. Para el ejemplo anterior:

`=GETPIVOTDATA("Suma de Importe", $E$3, "Región", "Este", "Mes", "Ene")` devuelve 15.

- El primer argumento es el nombre del valor, tal como lo muestra la tabla (por ejemplo, Suma de Importe). También sirve el nombre del campo que se suma (Importe).
- El segundo argumento es cualquier celda de la tabla dinámica.
- A continuación vienen pares formados por un campo y uno de sus elementos. Si indica solo Región, Este, obtiene el total de Este, 35. Si no indica nada, obtiene el total general, 49. Los campos que indique deben ser los primeros campos de las filas, o los primeros de las columnas, en orden.
- Si la tabla no muestra el número, la celda muestra `#REF!`.

Si hace clic en un valor de una tabla dinámica mientras escribe una fórmula, JeffCrunch escribe esta fórmula automáticamente, para que siga leyendo el número correcto aunque se reorganice la tabla. Para desactivarlo, elija **Opciones** en la pestaña **Tabla dinámica** y desactive la casilla relativa a GETPIVOTDATA.

## Qué hace JeffCrunch con una tabla dinámica

Una tabla dinámica de un libro se muestra tal como se guardó. Si no cambia nada en ella, el archivo se guarda con la tabla dinámica exactamente igual. Cuando la actualiza o la modifica, JeffCrunch vuelve a calcular el informe, coloca los números nuevos en las celdas y vuelve a escribir la tabla dinámica para que el programa que la creó la abra sin problemas.

JeffCrunch calcula tablas dinámicas que suman, cuentan, calculan promedios, etc. Una tabla dinámica que agrupa fechas o números, tiene un campo o elemento calculado, muestra valores como porcentaje, diferencia o clasificación, o usa un modelo de datos se muestra y se conserva tal como se guardó, y los comandos le indican que no pueden modificarla.

Haga clic en cualquier celda de una tabla dinámica. Aparece la pestaña **Tabla dinámica**.

## Actualizar

**Actualizar** lee las celdas a partir de las que se creó la tabla dinámica y vuelve a calcular el informe. Los valores nuevos se colocan detrás de los existentes; los que ya no están se quitan. Los anchos de columna y el diseño elegido se mantienen. **Actualizar todo**, en la pestaña **Datos** y en la pestaña **Tabla dinámica**, hace lo mismo con todas las tablas dinámicas del libro.

Si el informe fuera a extenderse sobre celdas con contenido, JeffCrunch no lo actualiza y le indica qué celdas lo impiden. Bórrelas o mueva la tabla dinámica y vuelva a actualizar.

**Deshacer** revierte una actualización en un solo paso.

## Cambiar origen de datos

**Cambiar origen de datos** pide las celdas que contienen la lista, incluida su primera fila de nombres. Escríbalas como `Hoja1!A1:D100`, o escriba el nombre de una tabla o de un nombre que haya creado. JeffCrunch las lee y vuelve a calcular el informe. Si un campo de la lista tiene el mismo nombre en las celdas nuevas, conserva su lugar en el informe.

## Borrar

**Borrar** tiene dos opciones. **Borrar filtros** quita todos los filtros y vuelve a mostrar todos los valores. **Borrar todo** quita todos los campos del informe y lo deja vacío, listo para crearlo de nuevo.

## El panel de campos

**Lista de campos** en la pestaña **Tabla dinámica** abre el panel **Campos de tabla dinámica**. Muestra los campos de la lista con una casilla junto a cada uno y, debajo, los cuatro lugares: **Filtros**, **Columnas**, **Filas** y **Valores**.

- Active un campo para agregarlo. Un campo que solo contiene números va a **Valores** y se suma; cualquier otro va a **Filas**.
- Desactive un campo para quitarlo de todas partes.
- Haga clic en un campo de uno de los cuatro lugares y elija **Mover a** para colocarlo en otro. **Subir** y **Bajar** cambian su orden dentro de su lugar, y **Quitar campo** lo quita.
- Con más de un valor, aparece en **Columnas** una línea llamada **Valores**. Muévala a **Filas** para colocar los valores en el lateral.
- **Configuración de campo de valor** (doble clic en un valor, o el botón) establece el nombre que muestra el informe para el valor y cómo se totaliza: **Suma**, **Recuento**, **Contar números**, **Promedio**, **Máx.**, **Mín.**, **Producto**, **DesvEst** y **DesvEstP** (la desviación estándar de una muestra o de todos los valores) y **Var** y **VarP** (la varianza de lo mismo).

Cada cambio es un paso de **Deshacer**.

## Diseño: subtotales, totales generales y diseño de informe

- **Subtotales** muestra u oculta el total de cada grupo, en la parte superior o inferior del grupo. Si hay más de un campo en Filas, el total de cada campo exterior es un subtotal.
- **Totales generales** tiene cuatro opciones: **Desactivado para filas y columnas**, **Activado para filas y columnas**, **Activado solo para filas** (una columna de totales a la derecha, uno por cada fila) y **Activado solo para columnas** (una fila de totales en la parte inferior).
- **Diseño de informe** es la forma de las etiquetas. En **forma compacta**, todos los campos de Filas comparten una columna y cada grupo nuevo se muestra un poco desplazado a la derecha. En **forma de esquema**, cada campo tiene su propia columna y cada grupo tiene una línea propia encima de sus elementos. En **formato tabular**, cada campo tiene su propia columna y el nombre del grupo aparece en su primera línea, con el subtotal debajo del grupo.

## La misma lista, con un filtro

Mueva **Mes** de **Columnas** a **Filtros**. El informe pierde sus columnas y una línea encima muestra **Mes** y **(Todo)**. Los números son ahora el total de cada región en todos los meses.

## Límites

Una tabla dinámica puede tener hasta 2000 campos, una lista de hasta cinco millones de filas y un informe de hasta cuatro millones de celdas. Un archivo que supera estos límites se muestra tal como se guardó y no se puede modificar. Volver a leer cien mil filas de veinte columnas y calcular el informe tardó aproximadamente medio segundo en el equipo en el que se midió; un equipo más lento tarda más.
