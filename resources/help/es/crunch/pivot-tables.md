# Tablas dinámicas

<!-- keywords: páginas de filtro de informe, tabla dinámica, profundizar, subir nivel, insertar tabla dinámica, crear, hacer, seleccionar, mover tabla dinámica, GETPIVOTDATA, generar GETPIVOTDATA, actualizar, actualizar todo, cambiar origen de datos, lista de campos, campos, filas, columnas, valores, filtros, filtro de informe, total general, subtotal, diseño, compacto, esquema, tabular, borrar, configuración de campo de valor, suma, recuento, promedio, mostrar valores como, porcentaje del total, diferencia, total acumulado, formato de número, configuración de campo, ordenar, filtrar, superiores, agrupar, desagrupar, fechas, meses, años, expandir, contraer, opciones, estilo, filas con bandas, filas en blanco, repetir etiquetas, actualizar al abrir el archivo, gráfico dinámico, segmentación, escala de tiempo, mostrar detalles, profundizar, filas detrás de un valor -->

Una tabla dinámica es un pequeño informe creado a partir de una lista larga. La lista tiene una fila por cada venta, por ejemplo; el informe suma las ventas por región y por mes. No necesita escribir ninguna fórmula. Indica qué columnas de la lista van en el lateral izquierdo, cuáles en la parte superior y cuál se suma, y el informe se rellena solo.

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

El Este en enero es 15 porque se suman las dos filas del Este de enero, 10 y 5. La columna Total general suma cada fila, la fila Total general suma cada columna, y la celda de la esquina suma todo.

Los términos utilizados aquí son:

- **Campo** es una columna de la lista. Región, Mes e Importe son los campos.
- **Filas**, **Columnas** y **Valores** son los tres lugares en los que puede colocarse un campo. Un campo en Filas da una línea por cada uno de sus valores distintos; un campo en Columnas da una columna por cada uno; un campo en Valores es el que se suma. Un cuarto lugar, **Filtros**, contiene un campo que excluye las filas que no elija.
- Una **caché** (la caché de la tabla dinámica) es una copia de la lista que la tabla guarda dentro del archivo, para poder mostrar totales sin consultar las celdas. El informe se calcula a partir de la caché, no de las celdas, así que no cambia cuando cambian las celdas. **Actualizar** vuelve a copiar las celdas en la caché.

## Crear una tabla dinámica

1. Haga clic en una celda de la lista, o seleccione las celdas que desea usar. La lista necesita una fila con nombres encima de sus datos.
2. Elija **Insertar** y, después, **Tabla dinámica**.
3. El cuadro muestra las celdas en las que están los datos. Cámbielas si no son correctas. También puede escribir el nombre de una tabla.
4. Elija dónde va la tabla dinámica: **Nueva hoja** (la opción predeterminada), o una celda de esta hoja o de otra. Elija un lugar donde no haya nada, porque la tabla vacía ocupa unas dieciocho filas y tres columnas.
5. Elija **Aceptar**. La tabla empieza vacía y se abre el panel **Campos de tabla dinámica**. Marque los campos que desee: un campo de números se suma, y cualquier otro se convierte en una fila. Después, mueva los campos a los cuadros como indican las secciones siguientes.

Una hoja nueva es un paso propio de **Deshacer**, y también lo es la tabla. Al guardar como .xlsx, la tabla se escribe de la forma en que lo hace el programa original, así que se abre allí como tabla dinámica. Una copia guardada como .xls o .ods conserva en sus celdas los números que muestra la tabla, pero no la tabla en sí, y al guardar se le indica.

## GETPIVOTDATA: leer un número de una tabla dinámica

`GETPIVOTDATA` devuelve el número que muestra una tabla dinámica para los valores, las filas y las columnas que indique. Para el ejemplo anterior:

`=GETPIVOTDATA("Suma de Importe"; $E$3; "Región"; "Este"; "Mes"; "Ene")` devuelve 15.

- El primer argumento es el nombre del valor, tal como lo muestra la tabla (por ejemplo, Suma de Importe). El nombre del campo que se suma (Importe) también sirve.
- El segundo argumento es cualquier celda de la tabla dinámica.
- Después vienen pares formados por un campo y uno de sus elementos. Si indica solo Región y Este, obtiene el total del Este, 35. Si no indica nada, obtiene el total general, 49. Los campos que indique deben ser los primeros campos de las filas, o los primeros campos de las columnas, en ese orden.
- Si la tabla no muestra el número, la celda muestra `#REF!`.

Cuando hace clic en un valor de una tabla dinámica mientras escribe una fórmula, JeffCrunch escribe esta fórmula por usted, para que siga leyendo el número correcto aunque la tabla se reorganice. Para desactivarlo, elija **Opciones** en la pestaña **Tabla dinámica** y desmarque la casilla sobre GETPIVOTDATA.

## Qué hace JeffCrunch con una tabla dinámica

Una tabla dinámica de un libro se muestra tal como se guardó. Si no cambia nada en ella, el archivo se guarda con la tabla dinámica exactamente como estaba. Cuando la actualiza o la modifica, JeffCrunch vuelve a calcular el informe, escribe los nuevos números en las celdas y vuelve a guardar la tabla dinámica de forma que el programa que la creó la abra sin problemas.

JeffCrunch calcula tablas dinámicas que suman, cuentan, promedian, etc., que agrupan fechas y números, y que muestran valores como porcentaje, diferencia o total acumulado. Una tabla dinámica que tiene un campo o elemento calculado, muestra un valor como clasificación o como porcentaje de un elemento principal, tiene grupos de elementos creados a mano o usa un modelo de datos se muestra y se conserva tal como se guardó, y los comandos le indican que no pueden modificarla.

Haga clic en cualquier celda de una tabla dinámica. Aparece la pestaña **Tabla dinámica**.

## Actualizar

**Actualizar** lee las celdas de las que se genera la tabla dinámica y vuelve a calcular el informe. Los nuevos elementos aparecen después de los que ya había; los elementos que han desaparecido se eliminan. Los anchos de columna y el diseño que eligió se mantienen. **Actualizar todo**, en la pestaña **Datos** y en la pestaña **Tabla dinámica**, hace lo mismo con todas las tablas dinámicas del libro.

Si el informe fuera a crecer sobre celdas que tienen contenido, JeffCrunch no lo actualiza. Le indica qué celdas estorban. Bórrelas o mueva la tabla dinámica, y vuelva a actualizar.

**Deshacer** revierte una actualización en un solo paso.

## Cambiar origen de datos

**Cambiar origen de datos** pide las celdas en las que está la lista, incluida su primera fila de nombres. Escríbalas como `Hoja1!A1:D100`, o escriba el nombre de una tabla o de un nombre que haya creado. JeffCrunch las lee y vuelve a calcular el informe. Si un campo de la lista tiene el mismo nombre en las nuevas celdas, conserva su lugar en el informe.

## Borrar

**Borrar** tiene dos opciones. **Borrar filtros** quita todos los filtros y vuelve a mostrar todos los elementos. **Borrar todo** quita todos los campos del informe y lo deja vacío, listo para volver a crearlo.

## Profundizar y subir nivel

## Páginas de filtro de informe

Un filtro se sitúa encima de una tabla dinámica y permite ver un elemento cada vez, por ejemplo, un mes. **Mostrar páginas de filtro de informe**, en la pestaña **Tabla dinámica**, crea una página para cada elemento a la vez. Cada página es una hoja nueva, con el nombre del elemento y con su propia copia de la tabla. El filtro de cada copia está fijado en ese elemento. Si la tabla tiene más de un filtro, elige cuál usar. Un solo **Deshacer** elimina todas las páginas.

Cuando un informe tiene más de un campo en sus filas, un elemento del primer campo puede abrirse o cerrarse. Haga clic en la celda con el nombre del elemento y elija **Subir nivel** para cerrarlo: las líneas del siguiente campo bajo él se ocultan y solo queda visible su total. **Profundizar** lo vuelve a abrir. Un elemento del último campo no tiene nada debajo. Cada una de estas acciones es un **Deshacer**.

## Seleccionar y mover

**Seleccionar**, en la pestaña **Tabla dinámica**, selecciona una parte del informe: **Toda la tabla dinámica**, **Valores** (los números) o **Etiquetas** (los nombres del lateral izquierdo; si el informe no tiene nombres en el lateral, los nombres de la parte superior). **Mover tabla dinámica…** pide una celda de la misma hoja y vuelve a dibujar el informe con su primera celda en ella. Las celdas que deja quedan vacías. Las fórmulas que apuntan a las celdas del informe las siguen, como ocurre al cortar y pegar. Si el nuevo lugar tiene contenido, JeffCrunch no mueve el informe y se lo indica. Un solo **Deshacer** lo devuelve a su sitio.

## El panel de campos

**Lista de campos**, en la pestaña **Tabla dinámica**, abre el panel **Campos de tabla dinámica**. Muestra los campos de la lista con un cuadro al lado de cada uno y, debajo, los cuatro lugares: **Filtros**, **Columnas**, **Filas** y **Valores**.

- Marque un campo para añadirlo. Un campo que contiene solo números va a **Valores** y se suma; cualquier otro va a **Filas**.
- Desmarque un campo para quitarlo de todas partes.
- Haga clic en un campo de uno de los cuatro lugares y elija **Mover a** para colocarlo en otro. Las palabras **Subir** y **Bajar** cambian su orden dentro de su lugar, y **Quitar campo** lo elimina.
- Con más de un valor, aparece una línea llamada **Valores** en **Columnas**. Muévala a **Filas** para colocar los valores en el lateral izquierdo.
- **Configuración de campo de valor** (doble clic en un valor, o el botón) establece el nombre que muestra el informe para el valor y cómo se totaliza: **Suma**, **Cuenta**, **Cuenta números**, **Promedio**, **Máx**, **Mín**, **Producto**, **DesvEst** y **DesvEstP** (la desviación estándar de una muestra, o de todos los valores), y **Var** y **VarP** (la varianza de los mismos). También establece cómo se muestra el valor y su formato de número. Véase la sección siguiente.
- **Configuración de campo** (doble clic en un campo de Filtros, Columnas o Filas, o el botón) configura un solo campo. Véase «Configuración de campo».

Cada cambio es un paso de **Deshacer**.

## Mostrar valores como, y el formato de número de un valor

En **Configuración de campo de valor**, **Mostrar valores como** cambia lo que muestra una celda del informe.

- **% del total general**: la celda dividida entre el total de todo el informe. En el ejemplo, el Este en enero es 15 de 49, es decir, 30,6 %.
- **% del total de columna** y **% del total de fila**: la celda dividida entre el total de su columna o de su fila.
- **Índice**: compara la celda con lo que daría su fila y su columna si los datos estuvieran repartidos por igual. Un número superior a 1 significa más que la parte proporcional.
- **Diferencia de**, **% de** y **% de diferencia de**: la celda comparada con otro elemento de un campo. Elija el **Campo base** (un campo de Filas o de Columnas) y el **Elemento base** (uno de sus elementos, o el anterior o el siguiente). Por ejemplo, con Mes como campo base y Ene como elemento base, febrero muestra cuánto más o cuánto menos es que enero. El propio elemento base queda vacío en una diferencia, y es 100 % en **% de**.
- **Total acumulado en**: cada celda suma las celdas anteriores a lo largo del campo base.
- **Clasificar de menor a mayor** y **Clasificar de mayor a menor**: cada celda muestra su posición, 1, 2, 3 y así sucesivamente, entre las celdas del campo base de la misma fila o columna. El puesto 1 es el número menor, o el mayor.
- **% del total de la fila superior** y **% del total de la columna superior**: cada celda muestra qué parte de su elemento superior es. El elemento superior de un elemento es el elemento un nivel por encima, como el año que contiene un mes. Un elemento sin superior se muestra como parte del total general.

**Formato de número** admite un código de formato, como `0,00`, `#.##0` o `0,0%`. Déjelo vacío para conservar el formato que tienen las celdas. Un valor mostrado como porcentaje recibe `0,00%` salvo que elija otro formato. Las celdas del informe adoptan el formato de inmediato. **Deshacer** lo revierte junto con los números.

## Configuración de campo

Seleccione un campo en el cuadro **Filtros**, **Columnas** o **Filas** y elija **Configuración de campo** en la pestaña **Tabla dinámica** (o haga doble clic en él).

- **Nombre personalizado** cambia el nombre que muestra el informe para el campo.
- **Subtotales**: **Automático** muestra un total para cada grupo del campo; **Ninguno** no muestra ninguno.
- **Mostrar etiquetas de elemento en**: forma compacta, de esquema o tabular, como en Diseño de informe.
- **Insertar una línea en blanco después de cada elemento** pone una línea vacía después de cada grupo.
- **Repetir etiquetas de elemento** muestra el nombre de un elemento externo en cada línea de su grupo (formatos de esquema y tabular).
- **Ordenar**: mantener el orden en que están los elementos (manual), u ordenar de A a Z o de Z a A por las etiquetas. **De menor a mayor por un valor** y **De mayor a menor por un valor** ordenan los elementos según los números de un campo de valor. El informe conserva la ordenación cuando cambian los datos.
- **Filtro** excluye elementos por sus etiquetas (comienza por, termina en, contiene, es igual a) o por un valor (mayor que, menor que, entre, etc.), o conserva solo los elementos superiores o inferiores por un valor. El filtro se aplica al elegir Aceptar. Oculta los elementos que deja fuera; las casillas de abajo indican cuáles son, y puede marcar un elemento para volver a mostrarlo.
- **Mostrar estos elementos** es una lista de los elementos del campo, con una casilla cada uno. Desmarque una casilla para ocultar el elemento, y los totales lo dejarán fuera.

## Agrupar fechas y números

Seleccione un campo de fechas o de números en **Filas** o **Columnas** y elija **Agrupar campo** en la pestaña **Tabla dinámica**. Los grupos se crean a partir de los datos, así que esa columna solo debe contener números o fechas, sin texto ni celdas vacías.

- **Fechas**: marque por qué agrupar: **Segundos**, **Minutos**, **Horas**, **Días**, **Meses**, **Trimestres** o **Años**. La parte más detallada que marque se mantiene en el campo. Para cada parte más amplia, JeffCrunch añade un campo con su nombre (por ejemplo, **Años**) y lo coloca delante del campo de fecha, en el mismo lugar. **Comenzar en** y **Terminar en** son la primera y la última fecha; déjelos como están para usar las fechas menor y mayor de los datos. Las fechas anteriores al inicio o posteriores al final se agrupan en dos grupos adicionales, que empiezan por `<` y `>`.
- **Números**: **Comenzar en**, **Terminar en** y **Con intervalo de** crean grupos como 1-10, 11-20, etc.

**Desagrupar** elimina los grupos y los campos que se añadieron para ellos. Al actualizar, los grupos se vuelven a crear con los datos nuevos. Cuando dos tablas dinámicas se crean a partir de los mismos datos, no se pueden crear grupos, porque un grupo forma parte de los datos que comparten.

## Mostrar las filas detrás de un valor

Haga doble clic en un valor de una tabla dinámica, o elija **Mostrar detalles** en la pestaña **Tabla dinámica**. JeffCrunch crea una hoja nueva, llamada Detalle 1, 2, etc., con las filas de la lista que forman ese número. Los nombres de los campos están en la primera fila. F2 sigue editando la celda, si la celda se puede editar. **Deshacer** elimina las filas, y otro **Deshacer** elimina la hoja.

## Expandir y contraer

Un campo cuyos elementos tienen otros debajo puede mostrar u ocultar las líneas bajo sus elementos. **Expandir todo el campo** y **Contraer todo el campo** lo hacen para todos los elementos del campo que seleccione en el panel.

## Opciones de tabla dinámica

**Opciones**, en la pestaña **Tabla dinámica**, abre un cuadro con las opciones de la tabla.

- **Nombre** es el nombre de la tabla dinámica. Ninguna otra tabla dinámica del libro puede tener ese nombre.
- **Para valores de error, mostrar** y **Para celdas vacías, mostrar** introducen un texto propio en esas celdas.
- **Combinar y centrar celdas con etiquetas**, **Ajustar anchos de columna al actualizar la tabla** y **Conservar el formato de las celdas al actualizar la tabla** se guardan con la tabla.
- **Mostrar totales generales de filas** y **de columnas** son las mismas que las opciones de Totales generales.
- **Actualizar los datos al abrir el archivo** vuelve a leer las celdas cada vez que se abre el libro. Es un paso de Deshacer, como cualquier actualización.

## Estilo: encabezados, bandas y galería

Las **Opciones de estilo**, en la pestaña **Tabla dinámica**, activan o desactivan la fila de encabezados, la primera columna (encabezados de fila) y las bandas de filas y de columnas. **Estilos** tiene una galería de aspectos, y **Borrar estilo** quita el aspecto. Estos aspectos se dibujan igual que los de las tablas. **Filas en blanco** y **Repetir etiquetas** establecen la línea en blanco y las etiquetas repetidas para todos los campos de las filas a la vez.

## Diseño: subtotales, totales generales y disposición

- **Subtotales** muestra u oculta el total de cada grupo, al principio o al final del grupo. Cuando hay más de un campo en Filas, el total de cada campo externo es un subtotal.
- **Totales generales** tiene cuatro opciones: desactivado para filas y columnas, activado para ambas, activado solo para filas (una columna de totales a la derecha, uno por cada fila) y activado solo para columnas (una fila de totales en la parte inferior).
- **Diseño de informe** es la forma de las etiquetas. En forma **compacta**, todos los campos de Filas comparten una columna, y un nuevo grupo se muestra un poco a la derecha. En forma de **esquema**, cada campo tiene su propia columna, y cada grupo tiene una línea propia encima de sus elementos. En forma **tabular**, cada campo tiene su propia columna y el nombre del grupo está en su primera línea, con el subtotal debajo del grupo.

## La misma lista, con un filtro

Mueva **Mes** de **Columnas** a **Filtros**. El informe pierde sus columnas, y una línea sobre él muestra **Mes** y **(Todos)**. Los números son ahora el total de cada región en todos los meses.

## Gráfico dinámico

Haga clic en una celda de una tabla dinámica y elija **Insertar** y, después, **Gráfico dinámico**. JeffCrunch crea un gráfico de columnas con los valores de la tabla. No incluye los totales generales. El gráfico queda marcado en el archivo como gráfico de la tabla dinámica. Lee las celdas de la tabla, así que sus números cambian al actualizar la tabla o al cambiar su diseño. Si la tabla gana filas o columnas, cree el gráfico de nuevo, porque el gráfico conserva las celdas con las que se creó. Un gráfico dinámico que procede de un archivo se muestra tal como se guardó.

## Segmentaciones y escalas de tiempo

Una segmentación es un conjunto de botones que filtra una tabla dinámica, y una escala de tiempo es un control deslizante que hace lo mismo con las fechas.

Haga clic en una celda de una tabla dinámica y, después, en **Segmentación** en la pestaña Tabla dinámica. Se abre el panel Segmentaciones, que enumera las segmentaciones y escalas de tiempo del libro. Una segmentación tiene un botón por cada elemento de su campo. Los botones azules son los elementos que muestra la tabla.

Haga clic en un botón para mostrar solo ese elemento. Haga clic de nuevo para mostrar todos los elementos. Mantenga pulsada **Ctrl** y haga clic para añadir un elemento a los mostrados, o para quitarlo. Cada clic es un paso de Deshacer, y Deshacer devuelve la tabla a su estado anterior. Al guardar, el archivo conserva la selección de la segmentación.

**Conexiones de informe** indica qué tablas dinámicas filtra una segmentación. Una escala de tiempo muestra el intervalo de fechas que eligió. JeffCrunch muestra las segmentaciones en este panel, no sobre las celdas, y todavía no modifica una escala de tiempo. Ambas se conservan en el archivo tal como se guardaron.

## Límites

Una tabla dinámica puede tener hasta 2.000 campos, una lista de hasta cinco millones de filas y un informe de hasta cuatro millones de celdas. Un archivo que supere estos límites se muestra tal como se guardó y no se puede modificar. Leer de nuevo cien mil filas de veinte columnas y calcular el informe tardó unos medio segundo en el equipo en el que se midió; un equipo más lento tarda más.
