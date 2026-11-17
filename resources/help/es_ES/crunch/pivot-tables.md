# Tablas dinámicas
<!-- keywords: otro archivo, otro libro, origen externo, fórmulas de lista, campo calculado, elemento calculado, orden de resolución, campo de fórmula, páginas de filtro de informe, tabla dinámica, profundizar, subir nivel, insertar tabla dinámica, crear, hacer, seleccionar, mover tabla dinámica, obtenerdatosdinamicos, generar obtenerdatosdinamicos, actualizar, actualizar todo, cambiar origen de datos, lista de campos, campos, filas, columnas, valores, filtros, filtro de informe, total general, subtotal, diseño, compacto, esquema, tabular, borrar, configuración de campo de valor, suma, recuento, promedio, mostrar valores como, porcentaje del total, diferencia, total acumulado, formato de número, configuración de campo, ordenar, filtrar, superiores, agrupar, desagrupar, fechas, meses, años, expandir, contraer, opciones, estilo, filas con bandas, filas en blanco, repetir etiquetas, actualizar al abrir el archivo, gráfico dinámico, segmentación de datos, escala de tiempo, mostrar detalles, profundizar, filas detrás de un valor, otro fichero, actualizar al abrir el fichero -->

Una tabla dinámica es un informe pequeño creado a partir de una lista larga. La lista tiene una fila por cada venta, por ejemplo; el informe suma las ventas por región y por mes. No necesita escribir ninguna fórmula. Usted indica qué columnas de la lista van a la izquierda, cuáles van arriba y cuál se suma, y el informe se rellena solo.

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

Una tabla dinámica con **Región** a la izquierda, **Mes** arriba y la suma de **Importe** en el centro tiene este aspecto.

| Suma de Importe | Mes | | |
|---|---|---|---|
| Región | Ene | Feb | Total general |
| Este | 15 | 20 | 35 |
| Oeste | 7 | 7 | 14 |
| Total general | 22 | 27 | 49 |

El Este en enero es 15 porque se suman las dos filas del Este de enero, 10 y 5. La columna Total general suma cada fila, la fila Total general suma cada columna, y la celda de la esquina suma todo.

Las palabras que se usan aquí:

- **Campo** es una columna de la lista. Región, Mes e Importe son los campos.
- **Filas**, **Columnas** y **Valores** son los tres lugares donde puede ir un campo. Un campo en Filas da una línea por cada uno de sus valores distintos; un campo en Columnas da una columna por cada uno; un campo en Valores es el que se suma. Un cuarto lugar, **Filtros**, contiene un campo que deja fuera las filas que usted no elige.
- Una **caché** (la caché de tabla dinámica) es una copia de su lista que la tabla dinámica guarda dentro del fichero, para poder mostrar totales sin leer las celdas. El informe se calcula a partir de la caché, no de las celdas, así que no cambia cuando cambian las celdas. **Actualizar** copia de nuevo las celdas en la caché.

## Crear una tabla dinámica

1. Haga clic en una celda de la lista, o seleccione las celdas que desea usar. La lista necesita una fila de nombres encima de sus datos.
2. Elija **Insertar** y, después, **Tabla dinámica**.
3. El cuadro muestra las celdas donde están los datos. Cámbielas si no son correctas. También puede escribir el nombre de una tabla. Para usar una lista que está en otro fichero, elija **Un rango en otro fichero**, seleccione el fichero con **Examinar** y escriba el rango, o el nombre de una tabla o de un nombre que haya creado en ese fichero, por ejemplo `Hoja1!A1:D100`. JeffCrunch lee libros (`.xlsx`, `.xlsm`, `.xls` y `.ods`) y ficheros `.csv`.
4. Elija dónde va la tabla dinámica: **Una hoja nueva** (la opción predeterminada), o una celda de esta hoja o de otra. Elija un lugar donde no haya nada más, porque la tabla vacía ocupa unas dieciocho filas y tres columnas.
5. Elija **Aceptar**. La tabla empieza vacía y se abre el panel **Campos de tabla dinámica**. Marque los campos que desea: un campo de números se suma, y cualquier otro se convierte en una fila. Después, mueva los campos a los cuadros como indican las secciones siguientes.

Cuando los datos están en otro fichero, la tabla dinámica guarda una copia de ellos, de modo que la tabla funciona y se guarda sin ese fichero. Consulte Datos en otro fichero más abajo.

Una hoja nueva es un paso propio de Deshacer, y también lo es la tabla. Al guardar como .xlsx, la tabla se escribe de la manera en que la escribe el programa original, así que se abre allí como tabla dinámica. Una copia guardada como .xls o .ods conserva en sus celdas los números que muestra la tabla, pero no la tabla en sí, y el programa se lo indica al guardar.

## OBTENERDATOSDINAMICOS: leer un número de una tabla dinámica

`OBTENERDATOSDINAMICOS` devuelve el número que muestra una tabla dinámica para los valores, filas y columnas que usted indique. Para el ejemplo anterior:

`=OBTENERDATOSDINAMICOS("Suma de Importe"; $E$3; "Región"; "Este"; "Mes"; "Ene")` devuelve 15.

- El primer argumento es el nombre del valor, tal como lo muestra la tabla (por ejemplo, Suma de Importe). También vale el nombre del campo que se suma (Importe).
- El segundo argumento es cualquier celda de la tabla dinámica.
- Después van pares de un campo y uno de sus elementos. Si indica solo Región y Este, obtiene el total del Este, 35. Si no indica nada, obtiene el total general, 49. Los campos que indique deben ser los primeros campos de las filas, o los primeros campos de las columnas, en ese orden.
- Si la tabla no muestra el número, la celda muestra `#REF!`.

Cuando hace clic en un valor de una tabla dinámica mientras escribe una fórmula, JeffCrunch escribe esta fórmula por usted, para que siga leyendo el número correcto aunque se reorganice la tabla. Para desactivarlo, elija **Opciones** en la pestaña **Tabla dinámica** y desmarque la casilla sobre OBTENERDATOSDINAMICOS.

## Qué hace JeffCrunch con una tabla dinámica

Una tabla dinámica de un libro se muestra tal como se guardó. Si no cambia nada en ella, el fichero se guarda con la tabla dinámica exactamente igual que estaba. Cuando la actualiza o la cambia, JeffCrunch vuelve a calcular el informe, escribe los números nuevos en las celdas y vuelve a escribir la tabla dinámica para que el programa que la creó la abra sin problemas.

JeffCrunch calcula tablas dinámicas que suman, cuentan, promedian, etc., que agrupan fechas y números, y que muestran valores como porcentaje, diferencia o total acumulado. Una tabla dinámica que muestra un valor como clasificación o como porcentaje de un elemento superior, que tiene grupos de elementos creados a mano o que usa un modelo de datos se muestra y se conserva tal como se guardó, y los comandos le indican que no se pueden cambiar.

Haga clic en cualquier celda de una tabla dinámica. Aparece la pestaña **Tabla dinámica**.

## Actualizar

**Actualizar** lee las celdas de las que se crea la tabla dinámica y vuelve a calcular el informe. Los elementos nuevos se añaden después de los que ya había; los que han desaparecido se eliminan. El ancho de las columnas y el diseño que eligió se conservan. **Actualizar todo**, en la pestaña **Datos** y en la pestaña **Tabla dinámica**, hace lo mismo con todas las tablas dinámicas del libro.

Si el informe fuera a crecer sobre celdas que tienen contenido, JeffCrunch no actualiza. Le indica qué celdas estorban. Bórrelas o mueva la tabla dinámica, y actualice de nuevo.

**Deshacer** revierte una actualización en un solo paso.

## Cambiar origen de datos

**Cambiar origen de datos** pide las celdas donde está la lista, incluida su primera fila de nombres. Escríbalas como `Hoja1!A1:D100`, o escriba el nombre de una tabla o de un nombre que haya creado. JeffCrunch las lee y vuelve a calcular el informe. Si un campo de la lista tiene el mismo nombre en las celdas nuevas, conserva su lugar en el informe.

También puede apuntar la tabla a un rango de otro fichero. En el cuadro, elija **Un rango en otro fichero**, seleccione el fichero y escriba el rango en él. Si elige **Un rango de este libro**, la tabla vuelve a apuntar a celdas de aquí. Cada uno de estos cambios es un paso de Deshacer.

## Datos en otro fichero

Una tabla dinámica creada a partir de otro fichero guarda dentro de este libro los datos de los que se creó, de modo que puede abrir, cambiar y guardar el libro aunque el otro fichero ya no esté. **Actualizar** vuelve a leer el otro fichero. Si el fichero se ha movido, no existe, es demasiado grande, no es un libro o ya no tiene el rango, la tabla conserva los datos que tenía y un mensaje indica que no se pudo leer el fichero. Un fichero que está en otro equipo de su red no se lee hasta que usted lo elija en **Cambiar origen de datos**. Una actualización lee cada fichero una sola vez, y en total como máximo 32 ficheros, 1 GB y 30 segundos; una tabla que supere ese límite conserva sus datos.

Si un libro indica que su tabla debe volver a leerse al abrirse, y los datos de la tabla están en otro fichero, JeffCrunch pregunta antes, del mismo modo que lo hace con los vínculos a otros libros. Hasta que usted diga que sí, no se lee ningún fichero.

Un libro guardado como .xlsx almacena dónde está el fichero. Si elige el fichero en el cuadro, se almacena su dirección completa. Un libro creado por otro programa, cuya tabla apunta a un fichero mediante una ruta corta como `Precios.xlsx`, conserva esa ruta, y JeffCrunch busca el fichero en la misma carpeta que el libro. Cuando usa **Guardar como** para guardar el libro en otra carpeta, una ruta así se reescribe para que siga nombrando el mismo fichero. Actualizar no sigue los vínculos que el otro fichero tenga con otros ficheros.

## Borrar

**Borrar** tiene dos opciones. **Borrar filtros** quita todos los filtros y vuelve a mostrar todos los elementos. **Borrar todo** quita todos los campos del informe y lo deja vacío, listo para volver a construirlo.

## Profundizar y subir nivel

## Páginas de filtro de informe

Un filtro está encima de una tabla dinámica y le permite ver un elemento cada vez, por ejemplo un mes. **Mostrar páginas de filtro de informe** en la pestaña **Tabla dinámica** crea una página para cada elemento a la vez. Cada página es una hoja nueva, con el nombre del elemento y con su propia copia de la tabla. El filtro de cada copia se fija en ese elemento. Si la tabla tiene más de un filtro, elige cuál usar. Un solo **Deshacer** elimina todas las páginas.

Cuando un informe tiene más de un campo en sus filas, un elemento del primer campo puede abrirse o cerrarse. Haga clic en la celda con el nombre del elemento y elija **Subir nivel** para cerrarlo: las líneas del campo siguiente que hay debajo se ocultan y solo queda su total. **Profundizar** lo vuelve a abrir. Un elemento del último campo no tiene nada debajo. Cada una de estas acciones es un **Deshacer**.

## Seleccionar y mover

**Seleccionar** en la pestaña **Tabla dinámica** selecciona una parte del informe: **Toda la tabla dinámica**, **Valores** (los números) o **Etiquetas** (los nombres de la izquierda; si el informe no tiene nombres a la izquierda, los nombres de arriba). **Mover tabla dinámica…** pide una celda de la misma hoja y vuelve a dibujar el informe con su primera celda allí. Las celdas que deja quedan vacías. Las fórmulas que apuntan a celdas del informe las siguen, como ocurre al cortar y pegar. Si el lugar nuevo tiene algo, JeffCrunch no mueve el informe y se lo indica. Un solo **Deshacer** lo devuelve a su sitio.

## El panel de campos

**Lista de campos** en la pestaña **Tabla dinámica** abre el panel **Campos de tabla dinámica**. Muestra los campos de la lista con un cuadro junto a cada uno, y debajo los cuatro lugares: **Filtros**, **Columnas**, **Filas** y **Valores**.

- Marque un campo para añadirlo. Un campo que contiene solo números va a **Valores** y se suma; cualquier otro va a **Filas**.
- Desmarque un campo para quitarlo de todas partes.
- Haga clic en un campo de uno de los cuatro lugares y elija **Mover a** para ponerlo en otro. Las palabras **Subir** y **Bajar** cambian su orden dentro de su lugar, y **Quitar campo** lo saca.
- Con más de un valor, aparece una línea llamada **Valores** en **Columnas**. Muévala a **Filas** para poner los valores a la izquierda.
- **Configuración de campo de valor** (doble clic en un valor, o el botón) fija el nombre que muestra el informe para el valor y cómo se totaliza: **Suma**, **Cuenta**, **Contar números**, **Promedio**, **Máx**, **Mín**, **Producto**, **DesvEst** y **DesvEstP** (la desviación estándar de una muestra o de todos los valores), y **Var** y **VarP** (la varianza de los mismos). También fija cómo se muestra el valor y su formato de número. Consulte la sección siguiente.
- **Configuración de campo** (doble clic en un campo de Filtros, Columnas o Filas, o el botón) configura un campo. Consulte «Configuración de campo».

Cada cambio es un paso de **Deshacer**.

## Mostrar valores como, y el formato de número de un valor

En **Configuración de campo de valor**, **Mostrar valores como** cambia lo que muestra una celda del informe.

- **% del total general**: la celda dividida por el total de todo el informe. En el ejemplo, el Este en enero son 15 de 49, es decir, 30,6 %.
- **% del total de columna** y **% del total de fila**: la celda dividida por el total de su columna o de su fila.
- **Índice**: cómo se compara la celda con lo que daría su fila y su columna si los datos estuvieran repartidos por igual. Un número superior a 1 significa más que la parte proporcional.
- **Diferencia de**, **% de** y **% de diferencia de**: la celda comparada con otro elemento de un campo. Elija el **Campo base** (un campo de Filas o Columnas) y el **Elemento base** (uno de sus elementos, o el anterior o el siguiente). Por ejemplo, con Mes como campo base y Ene como elemento base, febrero muestra cuánto más o menos es que enero. El propio elemento base queda vacío para una diferencia, y es el 100 % para **% de**.
- **Total acumulado en**: cada celda suma las celdas anteriores a lo largo del campo base.
- **Clasificar de menor a mayor** y **Clasificar de mayor a menor**: cada celda muestra su lugar, 1, 2, 3 y así sucesivamente, entre las celdas del campo base de la misma fila o columna. El puesto 1 es el número menor, o el mayor.
- **% del total de la fila superior** y **% del total de la columna superior**: cada celda muestra qué parte de su elemento superior es. El elemento superior de un elemento es el que está un nivel por encima, como el año que contiene un mes. Un elemento sin superior se muestra como parte del total general.

**Formato de número** acepta un código de formato, como `0,00`, `#.##0` o `0,0%`. Déjelo vacío para mantener el formato que tienen las celdas. Un valor mostrado como porcentaje recibe `0,00%` si no elige un formato. Las celdas del informe toman el formato al instante. **Deshacer** lo revierte junto con los números.

## Configuración de campo

Seleccione un campo en el cuadro **Filtros**, **Columnas** o **Filas** y elija **Configuración de campo** en la pestaña **Tabla dinámica** (o haga doble clic en él).

- **Nombre personalizado** cambia el nombre que muestra el informe para el campo.
- **Subtotales**: **Automáticos** muestra un total para cada grupo del campo; **Ninguno** no lo muestra.
- **Mostrar etiquetas de elementos en**: forma compacta, de esquema o tabular, como en Diseño de informe.
- **Insertar una línea en blanco después de cada elemento** pone una línea vacía después de cada grupo.
- **Repetir etiquetas de elementos** muestra el nombre de un elemento externo en cada línea de su grupo (formas de esquema y tabular).
- **Ordenar**: mantener el orden en que están los elementos (manual), u ordenar de la A a la Z o de la Z a la A por las etiquetas. **Ordenar de menor a mayor por un valor** y **Ordenar de mayor a menor por un valor** ordenan los elementos por los números de un campo de valores. El informe conserva la ordenación cuando cambian los datos.
- **Filtrar** deja fuera elementos según sus etiquetas (comienza por, termina por, contiene, es igual a) o según un valor (mayor que, menor que, entre, etc.), o conserva solo los elementos superiores o inferiores según un valor. El filtro se calcula al elegir Aceptar. Oculta los elementos que deja fuera; las casillas de abajo indican cuáles son, y puede marcar un elemento para volver a mostrarlo.
- **Mostrar estos elementos** es una lista de los elementos del campo, con una casilla cada uno. Desmarque una casilla para ocultar el elemento, y los totales lo dejarán fuera.

## Agrupar fechas y números

Seleccione un campo de fechas o de números en **Filas** o **Columnas** y elija **Agrupar campo** en la pestaña **Tabla dinámica**. Los grupos se crean a partir de los datos, así que la columna debe contener solo números o solo fechas, sin texto y sin celdas vacías.

- **Fechas**: marque por qué agrupar: **Segundos**, **Minutos**, **Horas**, **Días**, **Meses**, **Trimestres** o **Años**. La parte más detallada que marque se queda en el campo. Por cada parte más general, JeffCrunch añade un campo con su nombre (por ejemplo **Años**) y lo coloca antes del campo de fecha, en el mismo lugar. **Comenzar en** y **Terminar en** son la primera y la última fecha; déjelas como están para usar la fecha más antigua y la más reciente de los datos. Las fechas anteriores al inicio o posteriores al final van en dos grupos adicionales, que empiezan por `<` y `>`.
- **Números**: **Comenzar en**, **Terminar en** y **Con intervalos de** crean grupos como 1-10, 11-20, y así sucesivamente.

**Desagrupar** elimina los grupos y los campos que se añadieron para ellos. Actualizar vuelve a crear los grupos a partir de los datos nuevos. Cuando dos tablas dinámicas se crean a partir de los mismos datos, no se pueden crear grupos, porque un grupo es una parte de los datos que comparten.

## Campos calculados

Un campo calculado es un campo que usted crea con una fórmula. Usa otros campos de la lista. Por ejemplo, una prima de una décima parte del importe es `=Importe*0,1`.

Elija **Campo calculado** en la pestaña **Tabla dinámica**. Escriba un nombre y una fórmula. Escriba en la fórmula el nombre de un campo, o haga doble clic en él en la lista **Campos** para ponerlo allí. Escriba entre comillas simples un nombre que tenga espacios, como en `='Precio unitario'*Cantidad`. Puede usar números, `+ - * / ^ %`, comparaciones y algunas funciones como `SI`, `REDONDEAR`, `SUMA`, `MIN`, `MAX`, `PROMEDIO` y `ABS`.

Elija **Agregar**. El nuevo campo se coloca en el cuadro **Valores**. También aparece en la lista de campos, donde puede marcarlo o desmarcarlo como cualquier otro. No puede ir en las filas, las columnas ni los filtros.

Un campo calculado se calcula en cada celda del informe. El nombre de un campo representa el total de ese campo en la celda, así que `=Importe*Importe` eleva al cuadrado el total de la celda. No eleva al cuadrado cada fila y luego suma. El total general usa los totales de toda la lista.

**Fórmulas de lista** (junto a **Campo calculado**) crea una hoja nueva que indica el nombre de cada campo calculado y su fórmula, y cada elemento calculado con su fórmula, su campo y su orden de resolución. **Deshacer** elimina las celdas, y otro **Deshacer** elimina la hoja.

Para cambiar un campo, elija su nombre en el cuadro **Nombre**. Aparece su fórmula. Modifíquela y elija **Modificar**. **Eliminar** quita el campo y su valor. Cada una de estas acciones es un paso de **Deshacer**. Un campo calculado se guarda con el fichero y se conserva al actualizar el informe. Dos tablas dinámicas creadas a partir de los mismos datos no pueden tener uno, porque un campo calculado es una parte de los datos que comparten.

## Elementos calculados

Un elemento calculado es un elemento que usted crea a partir de otros elementos del mismo campo. Por ejemplo, el campo Región tiene los elementos Este y Oeste. Un nuevo elemento llamado Ambos puede ser `=Este+Oeste`.

Haga clic en una celda de la tabla dinámica y elija **Elemento calculado** en la pestaña **Tabla dinámica**. Elija el **Campo**. Debe estar en las filas o en las columnas. Escriba un **Nombre** y una **Fórmula**. Escriba en la fórmula el nombre de un elemento, o haga doble clic en él en la lista **Elementos** para ponerlo allí. Escriba entre comillas simples un nombre que tenga espacios, como en `='Este grande'+Oeste`. Puede usar números, `+ - * / ^ %` y paréntesis.

El nuevo elemento tiene una línea en el informe. En cada columna muestra la fórmula calculada a partir de las líneas de los elementos que nombra. El nuevo elemento cuenta en los totales. Un total de sumas, como el total general o un subtotal, añade la línea del nuevo elemento a las líneas de los demás elementos. Así, con Este en 35, Oeste en 14 y Ambos (Este más Oeste) en 49, el total general es 98. Si un elemento que nombra la fórmula está oculto, la celda muestra **#REF!**. Si la fórmula nombra un elemento que no existe, la celda muestra **#NOMBRE?**.

Para cambiar un elemento, elija su nombre en el cuadro **Nombre**. Aparece su fórmula. Modifíquela y elija **Modificar**. **Eliminar** quita el elemento y su línea. Cada una de estas acciones es un paso de **Deshacer**. Un elemento calculado se guarda con el fichero y se conserva al actualizar el informe. Dos tablas dinámicas creadas a partir de los mismos datos no pueden tener uno, porque un elemento calculado es una parte de los datos que comparten.

### Orden de resolución

A veces una celda tiene dos elementos calculados: uno en su fila y otro en su columna. El orden en que se calculan puede cambiar el resultado. Supongamos que el elemento de fila es `=Este-Oeste` y el de columna es `=Ene+1`. Calculado en un orden, la celda es (Este - Oeste) + 1. En el otro orden, es (Este + 1) - (Oeste + 1), que es otro número.

**Orden de resolución** en la pestaña **Tabla dinámica** muestra los elementos calculados en el orden en que se calculan. El elemento que está más abajo en la lista se calcula el último. Elija un elemento y haga clic en **Subir** o **Bajar**. El orden se guarda con el fichero como orden de los elementos, y otros programas lo leen igual. **Fórmulas de lista** muestra el lugar de cada elemento en él.

## Mostrar las filas detrás de un valor

Haga doble clic en un valor de una tabla dinámica, o elija **Mostrar detalles** en la pestaña **Tabla dinámica**. JeffCrunch crea una hoja nueva, llamada Detalle 1, 2, y así sucesivamente, con las filas de la lista que forman ese número. Los nombres de los campos están en la primera fila. F2 sigue editando la celda, si la celda se puede editar. **Deshacer** elimina las filas, y otro **Deshacer** elimina la hoja.

## Expandir y contraer

Un campo cuyos elementos tienen otros debajo puede mostrar u ocultar las líneas que hay bajo sus elementos. **Expandir todo el campo** y **Contraer todo el campo** lo hacen para todos los elementos del campo que seleccione en el panel.

## Opciones de tabla dinámica

**Opciones** en la pestaña **Tabla dinámica** abre un cuadro con las opciones de la tabla.

- **Nombre** es el nombre de la tabla dinámica. Ninguna otra tabla dinámica del libro puede tenerlo.
- **Para valores de error, mostrar** y **Para celdas vacías, mostrar** ponen un texto propio en esas celdas.
- **Combinar y centrar celdas con etiquetas**, **Ajustar anchos de columna al actualizar la tabla** y **Conservar el formato de las celdas al actualizar la tabla** se guardan con la tabla.
- **Mostrar totales generales de filas** y **de columnas** son lo mismo que las opciones de Totales generales.
- **Actualizar los datos al abrir el fichero** vuelve a leer las celdas cada vez que se abre el libro. Es un paso de Deshacer, como cualquier actualización.

## Estilo: encabezados, bandas y galería

Las **Opciones de estilo** de la pestaña **Tabla dinámica** activan o desactivan la fila de encabezados, la primera columna (encabezados de fila) y las bandas de filas y de columnas. **Estilos** tiene una galería de aspectos, y **Borrar estilo** quita el aspecto. Estos aspectos se dibujan igual que los de las tablas. **Filas en blanco** y **Repetir etiquetas** fijan la línea en blanco y las etiquetas repetidas para todos los campos de las filas a la vez.

## Diseño: subtotales, totales generales y formato

- **Subtotales** muestra u oculta el total de cada grupo, al principio o al final del grupo. Cuando hay más de un campo en Filas, el total de cada campo externo es un subtotal.
- **Totales generales** tiene cuatro opciones: desactivado para filas y columnas, activado para ambas, activado solo para filas (una columna de totales a la derecha, uno por cada fila) y activado solo para columnas (una fila de totales abajo).
- **Diseño de informe** es la forma de las etiquetas. En forma **compacta**, todos los campos de Filas comparten una columna, y un grupo nuevo se muestra un poco a la derecha. En forma de **esquema**, cada campo tiene su propia columna, y cada grupo tiene una línea propia encima de sus elementos. En forma **tabular**, cada campo tiene su propia columna y el nombre del grupo está en su primera línea, con el subtotal debajo del grupo.

## La misma lista, con un filtro

Mueva **Mes** de **Columnas** a **Filtros**. El informe pierde sus columnas, y una línea encima de él muestra **Mes** y **(Todos)**. Los números son ahora el total de cada región en todos los meses.

## Gráfico dinámico

Haga clic en una celda de una tabla dinámica y elija **Insertar** y, después, **Gráfico dinámico**. JeffCrunch crea un gráfico de columnas con los valores de la tabla. Deja fuera los totales generales. El gráfico queda marcado en el fichero como gráfico de la tabla dinámica. Lee las celdas de la tabla, así que sus números cambian cuando actualiza la tabla o cambia su diseño. Si la tabla gana filas o columnas, vuelva a crear el gráfico, porque el gráfico conserva las celdas a partir de las que se creó. Un gráfico dinámico que llega desde un fichero se muestra tal como se guardó.

## Segmentaciones y escalas de tiempo

Una segmentación es un conjunto de botones que filtra una tabla dinámica, y una escala de tiempo es un control deslizante que hace lo mismo con las fechas.

Haga clic en una celda de una tabla dinámica y, después, en **Segmentaciones** de la pestaña Tabla dinámica. Se abre el panel Segmentaciones y muestra las segmentaciones y escalas de tiempo del libro. Una segmentación tiene un botón por cada elemento de su campo. Los botones azules son los elementos que muestra la tabla.

Haga clic en un botón para mostrar solo ese elemento. Haga clic de nuevo para mostrar todos los elementos. Mantenga pulsada **Ctrl** y haga clic para añadir un elemento a los mostrados, o para quitarlo. Cada clic es un paso de Deshacer, y Deshacer devuelve la tabla a su estado anterior. Al guardar, el fichero conserva la elección de la segmentación.

**Conexiones de informe** indica qué tablas dinámicas filtra una segmentación. Una escala de tiempo muestra el intervalo de fechas que eligió. JeffCrunch muestra las segmentaciones en este panel, no sobre las celdas, y todavía no cambia una escala de tiempo. Ambas cosas se conservan en el fichero tal como se guardaron.

## Límites

Una tabla dinámica puede tener hasta 2.000 campos, una lista de hasta cinco millones de filas y un informe de hasta cuatro millones de celdas. Un fichero que supere estos valores se muestra tal como se guardó y no se puede cambiar. Leer de nuevo cien mil filas de veinte columnas y calcular el informe tardó unos medio segundo en el equipo donde se midió; un equipo más lento tarda más.
