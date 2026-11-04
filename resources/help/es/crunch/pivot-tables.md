# Tablas dinámicas

<!-- keywords: tabla dinámica, tablas dinámicas, expandir detalle, contraer detalle, insertar tabla dinámica, crear, hacer, seleccionar, mover tabla dinámica, GETPIVOTDATA, obtener datos de tabla dinámica, generar GETPIVOTDATA, actualizar, actualizar todo, cambiar origen de datos, lista de campos, campos, filas, columnas, valores, filtros, filtro de informe, total general, subtotal, diseño, compacto, esquema, tabular, borrar, configuración de campo de valor, suma, recuento, promedio, mostrar valores como, porcentaje del total, diferencia, total acumulado, formato de número, configuración de campo, ordenar, filtrar, superior, agrupar, desagrupar, fechas, meses, años, expandir, contraer, opciones, estilo, filas con bandas, filas en blanco, repetir etiquetas, actualizar al abrir el archivo, gráfico dinámico, segmentación de datos, escala de tiempo, mostrar detalles, expandir, contraer, filas detrás de un valor -->

Una tabla dinámica es un pequeño informe creado a partir de una lista larga. La lista tiene una fila por cada venta, por ejemplo; el informe suma las ventas por región y por mes. No necesita escribir ninguna fórmula. Usted indica qué columnas de la lista van en el lateral, cuáles van en la parte superior y cuál se suma, y el informe se rellena solo.

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

Este en enero es 15 porque se suman las dos filas de Este de enero, 10 y 5. La columna Total general suma cada fila, la fila Total general suma cada columna, y la celda de la esquina suma todo.

Los términos usados aquí:

- **Campo** es una columna de la lista. Región, Mes e Importe son campos.
- **Filas**, **Columnas** y **Valores** son los tres lugares donde puede ir un campo. Un campo en Filas da una línea por cada uno de sus valores distintos; un campo en Columnas da una columna por cada uno; un campo en Valores es el que se suma. Un cuarto lugar, **Filtros**, contiene un campo que deja fuera las filas que no elija.
- Una **caché** (la caché de tabla dinámica) es una copia de su lista que la tabla guarda dentro del archivo, de modo que puede mostrar totales sin leer las celdas. El informe se calcula a partir de la caché, no de las celdas, así que no cambia cuando cambian las celdas. **Actualizar** vuelve a copiar las celdas en la caché.

## Crear una tabla dinámica

1. Haga clic en una celda de la lista o seleccione las celdas que desea usar. La lista necesita una fila con los nombres de las columnas encima de los datos.
2. Elija **Insertar** y, después, **Tabla dinámica**.
3. El cuadro muestra las celdas en las que están los datos. Cámbielas si no son correctas. También puede escribir el nombre de una tabla.
4. Elija dónde va la tabla dinámica: **Nueva hoja** (la opción predeterminada) o una celda de esta hoja u otra. Elija un lugar donde no haya nada, porque la tabla vacía ocupa unas dieciocho filas y tres columnas.
5. Elija **Aceptar**. La tabla empieza vacía y se abre el panel **Campos de tabla dinámica**. Marque los campos que desee: un campo de números se suma, y cualquier otro se convierte en una fila. Después, mueva los campos a los cuadros como indican las secciones siguientes.

Una hoja nueva es un paso propio de **Deshacer**, y también lo es la tabla. Al guardar como .xlsx, la tabla se escribe del modo en que lo hace el programa original, de modo que se abre allí como tabla dinámica. Una copia guardada como .xls o .ods conserva en sus celdas los números que muestra la tabla, pero no la tabla en sí, y el guardado se lo indica.

## GETPIVOTDATA: leer un número de una tabla dinámica

`GETPIVOTDATA` devuelve el número que muestra una tabla dinámica para los valores, filas y columnas que indique. Para el ejemplo anterior:

`=GETPIVOTDATA("Suma de Importe"; $E$3; "Región"; "Este"; "Mes"; "Ene")` devuelve 15.

- El primer argumento es el nombre del valor, tal como lo muestra la tabla (por ejemplo, Suma de Importe). También vale el nombre del campo que se suma (Importe).
- El segundo argumento es cualquier celda de la tabla dinámica.
- Después vienen pares formados por un campo y uno de sus elementos. Si indica solo Región y Este, obtiene el total de Este, 35. Si no indica nada, obtiene el total general, 49. Los campos que indique deben ser los primeros campos de las filas, o los primeros campos de las columnas, en ese orden.
- Si la tabla no muestra el número, la celda muestra `#REF!`.

Cuando hace clic en un valor de una tabla dinámica mientras escribe una fórmula, JeffCrunch escribe esta fórmula por usted, de modo que siga leyendo el número correcto aunque la tabla cambie de disposición. Para desactivarlo, elija **Opciones** en la ficha **Tabla dinámica** y desmarque la casilla sobre GETPIVOTDATA.

## Qué hace JeffCrunch con una tabla dinámica

Una tabla dinámica de un libro se muestra tal como se guardó. Si no cambia nada, el archivo se guarda con la tabla dinámica exactamente como estaba. Cuando la actualiza o la cambia, JeffCrunch vuelve a calcular el informe, escribe los nuevos números en las celdas y vuelve a escribir la tabla dinámica para que el programa que la creó la abra sin problemas.

JeffCrunch calcula tablas dinámicas que suman, cuentan, promedian, etc., que agrupan fechas y números, y que muestran valores como porcentaje, diferencia o total acumulado. Una tabla dinámica que tiene un campo o elemento calculado, muestra un valor como clasificación o como porcentaje de un elemento principal, tiene grupos de elementos creados a mano o usa un modelo de datos, se muestra y se conserva tal como se guardó, y los comandos le indican que no pueden modificarla.

Haga clic en cualquier celda de una tabla dinámica. Aparece la ficha **Tabla dinámica**.

## Actualizar

**Actualizar** lee las celdas de las que se crea la tabla dinámica y vuelve a calcular el informe. Los elementos nuevos aparecen después de los que ya están; los que han desaparecido se eliminan. El ancho de las columnas y la disposición que eligió se mantienen. **Actualizar todo**, en la ficha **Datos** y en la ficha **Tabla dinámica**, hace lo mismo con todas las tablas dinámicas del libro.

Si el informe fuera a crecer sobre celdas que tienen contenido, JeffCrunch no actualiza y le indica qué celdas estorban. Bórrelas o mueva la tabla dinámica, y actualice de nuevo.

**Deshacer** revierte una actualización en un solo paso.

## Cambiar origen de datos

**Cambiar origen de datos** pide las celdas en las que está la lista, incluida su primera fila de nombres. Escríbalas así: `Hoja1!A1:D100`, o escriba el nombre de una tabla o de un nombre que haya creado. JeffCrunch las lee y vuelve a calcular el informe. Si un campo de la lista tiene el mismo nombre en las nuevas celdas, conserva su lugar en el informe.

## Borrar

**Borrar** tiene dos opciones. **Borrar filtros** quita todos los filtros y vuelve a mostrar todos los elementos. **Borrar todo** quita todos los campos del informe y lo deja vacío, listo para volver a crearlo.

## Expandir y contraer el detalle

Cuando un informe tiene más de un campo en sus filas, un elemento del primer campo se puede expandir o contraer. Haga clic en la celda con el nombre del elemento y elija **Contraer** para cerrarlo: las líneas del siguiente campo que están debajo se ocultan y solo queda su total. **Expandir** lo vuelve a abrir. Un elemento del último campo no tiene nada debajo. Cada una de estas acciones se puede deshacer con **Deshacer**.

## Seleccionar y mover

**Seleccionar** en la ficha **Tabla dinámica** selecciona una parte del informe: **Toda la tabla dinámica**, **Valores** (los números) o **Etiquetas** (los nombres del lateral; si el informe no tiene nombres en el lateral, los nombres de la parte superior). **Mover tabla dinámica…** pide una celda de la misma hoja y vuelve a dibujar el informe con su primera celda allí. Las celdas que deja quedan vacías. Las fórmulas que apuntan a las celdas del informe las siguen, como ocurre al cortar y pegar. Si el nuevo lugar tiene contenido, JeffCrunch no mueve el informe y se lo indica. Un solo **Deshacer** lo devuelve a su sitio.

## El panel de campos

**Lista de campos** en la ficha **Tabla dinámica** abre el panel **Campos de tabla dinámica**. Muestra los campos de la lista con un cuadro al lado de cada uno y, debajo, los cuatro lugares: **Filtros**, **Columnas**, **Filas** y **Valores**.

- Marque un campo para añadirlo. Un campo que solo tiene números va a **Valores** y se suma; cualquier otro va a **Filas**.
- Desmarque un campo para quitarlo de todas partes.
- Haga clic en un campo de uno de los cuatro lugares y elija **Mover a** para ponerlo en otro. Las opciones **Subir** y **Bajar** cambian su orden dentro de su lugar, y **Quitar campo** lo elimina.
- Con más de un valor, aparece una línea llamada **Valores** en **Columnas**. Muévala a **Filas** para poner los valores en el lateral.
- **Configuración de campo de valor** (doble clic en un valor, o el botón) establece el nombre que muestra el informe para el valor y cómo se totaliza: **Suma**, **Cuenta**, **Cuenta números**, **Promedio**, **Máx**, **Mín**, **Producto**, **DesvEst** y **DesvEstP** (la desviación estándar de una muestra o de todos los valores), y **Var** y **VarP** (la varianza de las mismas). También establece cómo se muestra el valor y su formato de número. Véase la sección siguiente.
- **Configuración de campo** (doble clic en un campo de Filtros, Columnas o Filas, o el botón) establece un solo campo. Véase «Configuración de campo».

Cada cambio es un paso de **Deshacer**.

## Mostrar valores como, y el formato de número de un valor

En **Configuración de campo de valor**, **Mostrar valores como** cambia lo que muestra una celda del informe.

- **% del total general**: la celda dividida entre el total de todo el informe. En el ejemplo, Este en enero es 15 de 49, que es un 30,6 %.
- **% del total de columna** y **% del total de fila**: la celda dividida entre el total de su columna, o de su fila.
- **Índice**: la comparación de la celda con lo que daría su fila y su columna si los datos estuvieran repartidos por igual. Un número superior a 1 significa más que la parte proporcional.
- **Diferencia de**, **% de** y **% de diferencia de**: la celda comparada con otro elemento de un campo. Elija el **Campo base** (un campo de Filas o de Columnas) y el **Elemento base** (uno de sus elementos, o el anterior o el siguiente). Por ejemplo, con Mes como campo base y Ene como elemento base, febrero muestra cuánto es más o menos que enero. El propio elemento base queda vacío en una diferencia, y es 100 % en **% de**.
- **Total acumulado en**: cada celda suma las celdas anteriores a lo largo del campo base.

**Formato de número** admite un código de formato, como `0,00`, `#.##0` o `0,0%`. Déjelo vacío para conservar el formato que tienen las celdas. Un valor mostrado como porcentaje recibe `0,00%` si no elige otro formato. Las celdas del informe adoptan el formato al momento. **Deshacer** lo revierte junto con los números.

## Configuración de campo

Seleccione un campo en el cuadro **Filtros**, **Columnas** o **Filas** y elija **Configuración de campo** en la ficha **Tabla dinámica** (o haga doble clic en él).

- **Nombre personalizado** cambia el nombre que muestra el informe para el campo.
- **Subtotales**: **Automático** muestra un total por cada grupo del campo; **Ninguno** no lo muestra.
- **Mostrar etiquetas de elemento en**: forma compacta, de esquema o tabular, como en Diseño de informe.
- **Insertar una línea en blanco después de cada elemento** pone una línea vacía después de cada grupo.
- **Repetir etiquetas de elemento** muestra el nombre de un elemento externo en cada línea de su grupo (formatos de esquema y tabular).
- **Ordenar**: mantener el orden en que están los elementos (manual), u ordenar de A a Z o de Z a A por las etiquetas. **De menor a mayor por un valor** y **De mayor a menor por un valor** ordenan los elementos según los números de un campo de valor. El informe conserva la ordenación cuando cambian los datos.
- **Filtro** deja fuera elementos según sus etiquetas (comienza por, termina en, contiene, es igual a) o según un valor (mayor que, menor que, entre, etc.), o conserva solo los primeros o últimos elementos por un valor. El filtro se calcula al elegir Aceptar. Oculta los elementos que deja fuera; las casillas de abajo muestran cuáles son, y puede marcar un elemento para volver a mostrarlo.
- **Mostrar estos elementos** es una lista de los elementos del campo, cada uno con una casilla. Desmarque una casilla para ocultar el elemento, y los totales lo dejan fuera.

## Agrupar fechas y números

Seleccione un campo de fechas o de números en **Filas** o **Columnas** y elija **Agrupar campo** en la ficha **Tabla dinámica**. Los grupos se crean a partir de los datos, así que la columna solo debe tener números o fechas, sin texto ni celdas vacías.

- **Fechas**: marque por qué agrupar: **Segundos**, **Minutos**, **Horas**, **Días**, **Meses**, **Trimestres** o **Años**. La parte más detallada que marque se queda en el campo. Por cada parte más general, JeffCrunch añade un campo con su nombre (por ejemplo, **Años**) y lo coloca delante del campo de fecha, en el mismo lugar. **Desde** y **Hasta** son la primera y la última fecha; déjelas como están para usar la fecha más antigua y la más reciente de los datos. Las fechas anteriores al inicio o posteriores al final van en dos grupos adicionales, que empiezan por `<` y `>`.
- **Números**: **Desde**, **Hasta** y **Intervalo** crean grupos como 1-10, 11-20, etc.

**Desagrupar** elimina los grupos y los campos que se añadieron para ellos. Al actualizar, los grupos se vuelven a crear con los nuevos datos. Cuando dos tablas dinámicas se crean a partir de los mismos datos, no se pueden crear grupos, porque un grupo es una parte de los datos que comparten.

## Mostrar las filas detrás de un valor

Haga doble clic en un valor de una tabla dinámica, o elija **Mostrar detalles** en la ficha **Tabla dinámica**. JeffCrunch crea una hoja nueva, llamada Detalle 1, 2, etc., con las filas de la lista que forman ese número. Los nombres de los campos están en la primera fila. F2 sigue editando la celda, si la celda se puede editar. **Deshacer** quita las filas, y otro **Deshacer** quita la hoja.

## Expandir y contraer

Un campo cuyos elementos tienen otros debajo puede mostrar u ocultar las líneas de debajo de sus elementos. **Expandir todo el campo** y **Contraer todo el campo** lo hacen para todos los elementos del campo que seleccione en el panel.

## Opciones de tabla dinámica

**Opciones** en la ficha **Tabla dinámica** abre un cuadro con las opciones de la tabla.

- **Nombre** es el nombre de la tabla dinámica. Ninguna otra tabla dinámica del libro puede tener ese nombre.
- **Para valores de error, mostrar** y **Para celdas vacías, mostrar** ponen un texto propio en esas celdas.
- **Combinar y centrar celdas con etiquetas**, **Ajustar anchos de columna al actualizar la tabla** y **Conservar el formato de las celdas al actualizar la tabla** se guardan con la tabla.
- **Mostrar totales generales de filas** y **de columnas** son lo mismo que las opciones de Total general.
- **Actualizar los datos al abrir el archivo** lee las celdas de nuevo cada vez que se abre el libro. Es un paso de Deshacer, como cualquier actualización.

## Estilo: encabezados, bandas y galería

Las **Opciones de estilo** de la ficha **Tabla dinámica** activan o desactivan la fila de encabezados, la primera columna (encabezados de fila) y las bandas de filas y de columnas. **Estilos** tiene una galería de aspectos, y **Borrar estilo** quita el aspecto. Estos aspectos se dibujan igual que los de las tablas. **Filas en blanco** y **Repetir etiquetas** establecen la línea en blanco y las etiquetas repetidas para todos los campos de las filas a la vez.

## Diseño: subtotales, totales generales y disposición

- **Subtotales** muestra u oculta el total de cada grupo, al principio o al final del grupo. Cuando hay más de un campo en Filas, el total de cada campo externo es un subtotal.
- **Totales generales** tiene cuatro opciones: desactivado para filas y columnas, activado para ambas, activado solo para filas (una columna de totales a la derecha, uno por cada fila) y activado solo para columnas (una fila de totales abajo).
- **Diseño de informe** es la forma de las etiquetas. En forma **compacta**, todos los campos de Filas comparten una columna, y un grupo nuevo se muestra un poco más a la derecha. En forma de **esquema**, cada campo tiene su propia columna, y cada grupo ocupa una línea propia encima de sus elementos. En forma **tabular**, cada campo tiene su propia columna y el nombre del grupo está en su primera línea, con el subtotal debajo del grupo.

## La misma lista, con un filtro

Mueva **Mes** de **Columnas** a **Filtros**. El informe pierde sus columnas, y encima de él aparece una línea con **Mes** y **(Todos)**. Los números son ahora el total de cada región para todos los meses.

## Gráfico dinámico

Haga clic en una celda de una tabla dinámica y elija **Insertar** y, después, **Gráfico dinámico**. JeffCrunch crea un gráfico de columnas con los valores de la tabla. No incluye los totales generales. El gráfico queda marcado en el archivo como gráfico de la tabla dinámica. Lee las celdas de la tabla, así que sus números cambian al actualizar la tabla o al cambiar su disposición. Si la tabla gana filas o columnas, vuelva a crear el gráfico, porque el gráfico conserva las celdas a partir de las que se creó. Un gráfico dinámico que procede de un archivo se muestra tal como se guardó.

## Segmentaciones de datos y escalas de tiempo

Una segmentación de datos es un conjunto de botones que filtra una tabla dinámica, y una escala de tiempo es un control deslizante que hace lo mismo con las fechas.

Haga clic en una celda de una tabla dinámica y, después, en **Segmentaciones** de la ficha Tabla dinámica. Se abre el panel Segmentaciones, que enumera las segmentaciones y escalas de tiempo del libro. Una segmentación tiene un botón por cada elemento de su campo. Los botones azules son los elementos que muestra la tabla.

Haga clic en un botón para mostrar solo ese elemento. Haga clic de nuevo para mostrar todos los elementos. Mantenga pulsada **Ctrl** y haga clic para añadir un elemento a los que se muestran, o para quitar uno. Cada clic es un paso de Deshacer, y Deshacer devuelve la tabla a su estado anterior. Al guardar, el archivo conserva la elección de la segmentación.

**Conexiones de informe** indica qué tablas dinámicas filtra una segmentación. Una escala de tiempo muestra el intervalo de fechas que eligió. JeffCrunch muestra las segmentaciones en este panel, no sobre las celdas, y todavía no modifica una escala de tiempo. Ambas se conservan en el archivo tal como se guardaron.

## Límites

Una tabla dinámica puede tener hasta 2.000 campos, una lista de hasta cinco millones de filas y un informe de hasta cuatro millones de celdas. Un archivo que supere estos límites se muestra tal como se guardó y no se puede modificar. Leer de nuevo cien mil filas de veinte columnas y calcular el informe tardó unos medio segundo en el equipo donde se midió; un equipo más lento tarda más.
