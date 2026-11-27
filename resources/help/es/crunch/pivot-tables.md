# Tablas dinámicas

<!-- keywords: otro archivo, otro libro, origen externo, fórmulas de lista, campo calculado, elemento calculado, orden de resolución, campo de fórmula, páginas de filtro de informe, tabla dinámica, profundizar, subir nivel, insertar tabla dinámica, crear, hacer, seleccionar, mover tabla dinámica, obtener.datos.tabla.dinámica, GETPIVOTDATA, generar GETPIVOTDATA, actualizar, actualizar todo, cambiar origen de datos, lista de campos, campos, filas, columnas, valores, filtros, filtro de informe, total general, subtotal, diseño, compacto, esquema, tabular, borrar, configuración de campo de valor, suma, recuento, promedio, mostrar valores como, porcentaje del total, diferencia, total acumulado, formato de número, configuración de campo, ordenar, filtrar, superiores, agrupar, desagrupar, fechas, meses, años, expandir, contraer, opciones, estilo, filas con bandas, filas en blanco, repetir etiquetas, actualizar al abrir el archivo, gráfico dinámico, segmentación de datos, escala de tiempo, mostrar detalles, profundizar, filas detrás de un valor -->

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

Las palabras que se usan aquí:

- **Campo** es una columna de la lista. Región, Mes e Importe son campos.
- **Filas**, **Columnas** y **Valores** son los tres lugares donde puede ir un campo. Un campo en Filas da una línea por cada uno de sus valores distintos; un campo en Columnas da una columna por cada uno; un campo en Valores es el que se suma. Un cuarto lugar, **Filtros**, contiene un campo que deja fuera las filas que no elija.
- Una **caché** (la caché de tabla dinámica) es una copia de su lista que la tabla dinámica guarda dentro del archivo, de modo que puede mostrar totales sin leer las celdas. El informe se calcula a partir de la caché, no de las celdas, por lo que no cambia cuando cambian las celdas. **Actualizar** vuelve a copiar las celdas en la caché.

## Crear una tabla dinámica

1. Haga clic en una celda de la lista o seleccione las celdas que quiere usar. La lista necesita una fila de nombres encima de sus datos.
2. Elija **Insertar** y, después, **Tabla dinámica**.
3. El cuadro muestra las celdas donde están los datos. Cámbielas si no son correctas. También puede escribir el nombre de una tabla. Para usar una lista que está en otro archivo, elija **Un rango en otro archivo**, seleccione el archivo con **Examinar** y escriba el rango, o el nombre de una tabla o de un nombre que haya creado en ese archivo, como `Hoja1!A1:D100`. JeffCrunch lee libros (`.xlsx`, `.xlsm`, `.xls` y `.ods`) y archivos `.csv`.
4. Elija dónde va la tabla dinámica: **Una hoja nueva** (la opción predeterminada), o una celda de esta hoja o de otra. Elija un lugar donde no haya nada más, porque la tabla vacía ocupa unas dieciocho filas y tres columnas.
5. Elija **Aceptar**. La tabla empieza vacía y se abre el panel **Campos de tabla dinámica**. Marque los campos que quiera: un campo de números se suma, y cualquier otro se convierte en una fila. Después, mueva los campos a los cuadros como indican las secciones siguientes.

Cuando los datos están en otro archivo, la tabla dinámica guarda una copia de ellos, de modo que la tabla funciona y se guarda sin ese archivo. Consulte Datos en otro archivo más abajo.

Una hoja nueva es un paso propio de Deshacer, y también lo es la tabla. Al guardar como .xlsx, la tabla se escribe tal como lo haría el programa original, de modo que se abre allí como tabla dinámica. Una copia guardada como .xls o .ods conserva en sus celdas los números que muestra la tabla, pero no la tabla en sí, y al guardar se le avisa de ello.

## OBTENER.DATOS.TABLA.DINÁMICA: leer un número de una tabla dinámica

`OBTENER.DATOS.TABLA.DINÁMICA` devuelve el número que muestra una tabla dinámica para los valores, las filas y las columnas que indique. Para el ejemplo anterior:

`=OBTENER.DATOS.TABLA.DINÁMICO("Suma de Importe"; $E$3; "Región"; "Este"; "Mes"; "Ene")` devuelve 15.

- El primer argumento es el nombre del valor, tal como lo muestra la tabla (por ejemplo, Suma de Importe). También vale el nombre del campo que se suma (Importe).
- El segundo argumento es cualquier celda de la tabla dinámica.
- Después van pares de un campo y uno de sus elementos. Si indica solo Región y Este, obtiene el total de Este, 35. Si no indica nada, obtiene el total general, 49. Los campos que indique deben ser los primeros campos de las filas, o los primeros campos de las columnas, en ese orden.
- Si la tabla no muestra el número, la celda muestra `#¡REF!`.

Cuando hace clic en un valor de una tabla dinámica mientras escribe una fórmula, JeffCrunch escribe esta fórmula por usted, para que siga leyendo el número correcto aunque la tabla se reorganice. Para desactivarlo, elija **Opciones** en la pestaña **Herramientas de tabla dinámica** y desmarque la casilla sobre GETPIVOTDATA.

## Qué hace JeffCrunch con una tabla dinámica

Una tabla dinámica de un libro se muestra tal como se guardó. Si no cambia nada, el archivo se guarda con la tabla dinámica exactamente igual que estaba. Cuando la actualiza o la modifica, JeffCrunch vuelve a calcular el informe, escribe los nuevos números en las celdas y vuelve a escribir la tabla dinámica para que el programa que la creó la abra sin problemas.

JeffCrunch calcula tablas dinámicas que suman, cuentan, promedian, etc., que agrupan fechas y números, y que muestran valores como porcentaje, diferencia o total acumulado. Una tabla dinámica que muestra un valor como clasificación o como porcentaje de un elemento principal, que tiene grupos de elementos creados a mano o que usa un modelo de datos, se muestra y se conserva tal como se guardó, y los comandos le indican que no puede modificarla.

Haga clic en cualquier celda de una tabla dinámica. Aparece la pestaña **Herramientas de tabla dinámica**.

## Actualizar

**Actualizar** lee las celdas de las que se crea la tabla dinámica y vuelve a calcular el informe. Los elementos nuevos se añaden después de los que ya están; los elementos que ya no existen se eliminan. El ancho de las columnas y el diseño que eligió se mantienen. **Actualizar todo**, en la pestaña **Datos** y en la pestaña **Herramientas de tabla dinámica**, hace lo mismo con todas las tablas dinámicas del libro.

Si el informe fuera a crecer sobre celdas que contienen algo, JeffCrunch no actualiza. Le indica qué celdas estorban. Bórrelas o mueva la tabla dinámica y actualice de nuevo.

**Deshacer** deshace una actualización en un solo paso.

## Cambiar origen de datos

**Cambiar origen de datos** pide las celdas donde está la lista, incluida su primera fila de nombres. Escríbalas como `Hoja1!A1:D100`, o escriba el nombre de una tabla o de un nombre que haya creado. JeffCrunch las lee y vuelve a calcular el informe. Si un campo de la lista tiene el mismo nombre en las nuevas celdas, conserva su lugar en el informe.

También puede apuntar la tabla a un rango de otro archivo. En el cuadro, elija **Un rango en otro archivo**, seleccione el archivo y escriba el rango en él. Si elige **Un rango en este libro**, la tabla vuelve a apuntar a celdas de aquí. Cada uno de estos cambios es un paso de Deshacer.

## Datos en otro archivo

Una tabla dinámica creada a partir de otro archivo conserva dentro de este libro los datos de los que se creó, de modo que puede abrir, cambiar y guardar el libro cuando el otro archivo ya no está. **Actualizar** vuelve a leer el otro archivo. Si el archivo se ha movido, no existe, es demasiado grande, no es un libro o ya no tiene el rango, la tabla conserva los datos que tenía y aparece un mensaje que indica que no se pudo leer el archivo. Un archivo que está en otro equipo de su red no se lee hasta que lo elija usted mismo en **Cambiar origen de datos**. Una actualización lee cada archivo una sola vez, y en total como máximo 32 archivos, 1 GB y 30 segundos; una tabla que supere ese límite conserva sus datos.

Si un libro indica que su tabla debe volver a leerse al abrirse, y los datos de la tabla están en otro archivo, JeffCrunch pregunta antes, del mismo modo que lo hace con los vínculos a otros libros. Hasta que responda que sí, no se lee ningún archivo.

Un libro guardado como .xlsx almacena dónde está el archivo. Si elige el archivo en el cuadro, se guarda su dirección completa. Un libro creado por otro programa, cuya tabla apunta a un archivo mediante una ruta corta como `Precios.xlsx`, conserva esa ruta, y JeffCrunch busca el archivo en la misma carpeta que el libro. Cuando usa **Guardar como** para guardar el libro en otra carpeta, una ruta de ese tipo se reescribe para que siga nombrando el mismo archivo. Actualizar no sigue los vínculos que el otro archivo tenga con otros archivos.

## Borrar

**Borrar** tiene dos opciones. **Borrar filtros** quita todos los filtros y vuelve a mostrar todos los elementos. **Borrar todo** quita todos los campos del informe y lo deja vacío, listo para volver a construirlo.

## Profundizar y subir nivel

## Páginas de filtro de informe

Un filtro se sitúa encima de una tabla dinámica y permite ver un elemento cada vez, como un mes. **Mostrar páginas de filtro de informe** en la pestaña **Herramientas de tabla dinámica** crea una página para cada elemento a la vez. Cada página es una hoja nueva, con el nombre del elemento y con su propia copia de la tabla. El filtro de cada copia se fija en ese elemento. Si la tabla tiene más de un filtro, elige cuál usar. Un solo **Deshacer** elimina todas las páginas.

Cuando un informe tiene más de un campo en sus filas, un elemento del primer campo puede abrirse o cerrarse. Haga clic en la celda con el nombre del elemento y elija **Subir nivel** para cerrarlo: las líneas del siguiente campo que están debajo se ocultan y solo queda su total. **Profundizar** lo vuelve a abrir. Un elemento del último campo no tiene nada debajo. Cada acción es un solo **Deshacer**.

## Seleccionar y mover

**Seleccionar** en la pestaña **Herramientas de tabla dinámica** selecciona una parte del informe: **Tabla dinámica completa**, **Valores** (los números) o **Etiquetas** (los nombres del lateral; si el informe no tiene nombres en el lateral, los nombres de la parte superior). **Mover tabla dinámica…** pide una celda de la misma hoja y vuelve a dibujar el informe con su primera celda allí. Las celdas que deja quedan vacías. Las fórmulas que apuntan a las celdas del informe las siguen, como ocurre al cortar y pegar. Si el nuevo lugar tiene algo, JeffCrunch no mueve el informe y se lo indica. Un solo **Deshacer** lo devuelve a su sitio.

## El panel de campos

**Lista de campos** en la pestaña **Herramientas de tabla dinámica** abre el panel **Campos de tabla dinámica**. Muestra los campos de la lista con un cuadro al lado de cada uno y, debajo, los cuatro lugares: **Filtros**, **Columnas**, **Filas** y **Valores**.

- Marque un campo para añadirlo. Un campo que solo contiene números va a **Valores** y se suma; cualquier otro va a **Filas**.
- Desmarque un campo para quitarlo de todas partes.
- Haga clic en un campo de uno de los cuatro lugares y elija **Mover a** para ponerlo en otro. Las palabras **Subir** y **Bajar** cambian su orden dentro de su lugar, y **Quitar campo** lo elimina.
- Con más de un valor, aparece una línea llamada **Valores** en **Columnas**. Muévala a **Filas** para poner los valores en el lateral.
- **Configuración de campo de valor** (doble clic en un valor, o el botón) establece el nombre que muestra el informe para el valor y cómo se totaliza: **Suma**, **Cuenta**, **Cuenta de números**, **Promedio**, **Máx**, **Mín**, **Producto**, **DesvEst** y **DesvEstP** (la desviación estándar de una muestra, o de todos los valores), y **Var** y **Varp** (la varianza de lo mismo). También establece cómo se muestra el valor y su formato de número. Consulte la sección siguiente.
- **Configuración de campo** (doble clic en un campo de Filtros, Columnas o Filas, o el botón) configura un solo campo. Consulte «Configuración de campo».

Cada cambio es un paso de **Deshacer**.

## Mostrar valores como, y el formato de número de un valor

En **Configuración de campo de valor**, **Mostrar valores como** cambia lo que muestra una celda del informe.

- **% del total general**: la celda dividida entre el total de todo el informe. En el ejemplo, Este en enero es 15 de 49, que son 30,6 %.
- **% del total de columna** y **% del total de fila**: la celda dividida entre el total de su columna, o de su fila.
- **Índice**: cómo se compara la celda con lo que daría su fila y su columna si los datos estuvieran repartidos por igual. Un número superior a 1 significa más que la parte igualitaria.
- **Diferencia de**, **% de** y **% de diferencia de**: la celda comparada con otro elemento de un campo. Elija el **Campo base** (un campo de Filas o de Columnas) y el **Elemento base** (uno de sus elementos, o el anterior o el siguiente). Por ejemplo, con Mes como campo base y Ene como elemento base, febrero muestra cuánto es más o menos que enero. El propio elemento base queda vacío en una diferencia, y es 100 % en **% de**.
- **Total acumulado en**: cada celda suma las celdas anteriores a lo largo del campo base.
- **Clasificar de menor a mayor** y **Clasificar de mayor a menor**: cada celda muestra su puesto, 1, 2, 3 y así sucesivamente, entre las celdas del campo base de la misma fila o columna. El puesto 1 es el número más pequeño, o el más grande.
- **% del total de fila principal** y **% del total de columna principal**: cada celda muestra qué parte de su principal es. El principal de un elemento es el elemento un nivel por encima, como el año que contiene un mes. Un elemento sin principal se muestra como parte del total general.

**Formato de número** acepta un código de formato, como `0,00`, `#.##0` o `0,0%`. Déjelo vacío para mantener el formato que tienen las celdas. Un valor mostrado como porcentaje recibe `0,00%` salvo que elija un formato. Las celdas del informe adoptan el formato de inmediato. **Deshacer** lo quita junto con los números.

## Configuración de campo

Seleccione un campo en el cuadro **Filtros**, **Columnas** o **Filas** y elija **Configuración de campo** en la pestaña **Herramientas de tabla dinámica** (o haga doble clic en él).

- **Nombre personalizado** cambia el nombre que muestra el informe para el campo.
- **Subtotales**: **Automático** muestra un total para cada grupo del campo; **Ninguno** no lo muestra.
- **Mostrar etiquetas de elementos en**: forma compacta, de esquema o tabular, como en Diseño del informe.
- **Insertar una línea en blanco después de cada elemento** añade una línea vacía después de cada grupo.
- **Repetir etiquetas de elemento** muestra el nombre de un elemento externo en cada línea de su grupo (formas de esquema y tabular).
- **Ordenar**: mantener el orden en que están los elementos (manual), u ordenar de la A a la Z o de la Z a la A por las etiquetas. **De menor a mayor según un valor** y **De mayor a menor según un valor** ordenan los elementos por los números de un campo de valores. El informe mantiene la ordenación cuando cambian los datos.
- **Filtrar** deja fuera elementos según sus etiquetas (comienza por, termina en, contiene, es igual a) o según un valor (mayor que, menor que, entre, etc.), o conserva solo los elementos superiores o inferiores según un valor. El filtro se calcula al elegir Aceptar. Oculta los elementos que deja fuera; las casillas de abajo muestran cuáles son, y puede marcar un elemento para volver a mostrarlo.
- **Mostrar estos elementos** es una lista de los elementos del campo con una casilla cada uno. Desmarque una casilla para ocultar el elemento, y los totales lo dejarán fuera.

## Agrupar fechas y números

Seleccione un campo de fechas o de números en **Filas** o **Columnas** y elija **Agrupar campo** en la pestaña **Herramientas de tabla dinámica**. Los grupos se crean a partir de los datos, así que en esa columna solo debe haber números o fechas, sin texto ni celdas vacías.

- **Fechas**: marque por qué agrupar: **Segundos**, **Minutos**, **Horas**, **Días**, **Meses**, **Trimestres** o **Años**. La parte más detallada que marque se queda en el campo. Por cada parte más general, JeffCrunch añade un campo con su nombre (por ejemplo, **Años**) y lo coloca antes del campo de fecha, en el mismo lugar. **Comenzar en** y **Terminar en** son la primera y la última fecha; déjelas como están para usar la fecha más antigua y la más reciente de los datos. Las fechas anteriores al inicio o posteriores al final van en dos grupos adicionales, que empiezan por `<` y `>`.
- **Números**: **Comenzar en**, **Terminar en** y **Por** crean grupos como 1-10, 11-20, etc.

**Desagrupar** elimina los grupos y los campos que se añadieron para ellos. Actualizar vuelve a crear los grupos con los datos nuevos. Cuando dos tablas dinámicas se crean a partir de los mismos datos, no se pueden crear grupos, porque un grupo forma parte de los datos que comparten.

## Campos calculados

Un campo calculado es un campo que crea a partir de una fórmula. Usa otros campos de la lista. Por ejemplo, una prima de una décima parte del importe es `=Importe*0,1`.

Elija **Campo calculado** en la pestaña **Herramientas de tabla dinámica**. Escriba un nombre y una fórmula. Escriba el nombre de un campo en la fórmula, o haga doble clic en él en la lista **Campos** para colocarlo allí. Ponga entre comillas simples un nombre que tenga espacios, como en `='Precio unitario'*Cantidad`. Puede usar números, `+ - * / ^ %`, comparaciones y algunas funciones, como `SI`, `REDONDEAR`, `SUMA`, `MIN`, `MAX`, `PROMEDIO` y `ABS`.

Elija **Agregar**. El nuevo campo se coloca en el cuadro **Valores**. También aparece en la lista de campos, donde puede marcarlo o desmarcarlo como cualquier otro campo. No puede ir a las filas, a las columnas ni a los filtros.

Un campo calculado se calcula en cada celda del informe. El nombre de un campo representa el total de ese campo en la celda, así que `=Importe*Importe` eleva al cuadrado el total de la celda. No eleva al cuadrado cada fila y después las suma. Un total general usa los totales de toda la lista.

**Fórmulas de lista** (junto a **Campo calculado**) crea una hoja nueva que indica cada campo calculado y su fórmula, y cada elemento calculado con su fórmula, su campo y su orden de resolución. **Deshacer** quita las celdas, y otro **Deshacer** quita la hoja.

Para cambiar un campo, elija su nombre en el cuadro **Nombre**. Aparece su fórmula. Modifíquela y elija **Modificar**. **Eliminar** quita el campo y su valor. Cada acción es un paso de **Deshacer**. Un campo calculado se guarda con el archivo y se conserva al actualizar el informe. Dos tablas dinámicas creadas a partir de los mismos datos no pueden tener uno, porque un campo calculado forma parte de los datos que comparten.

## Elementos calculados

Un elemento calculado es un elemento que crea a partir de otros elementos del mismo campo. Por ejemplo, el campo Región tiene los elementos Este y Oeste. Un nuevo elemento llamado Ambos puede ser `=Este+Oeste`.

Haga clic en una celda de la tabla dinámica y elija **Elemento calculado** en la pestaña **Herramientas de tabla dinámica**. Elija el **Campo**. Debe estar en las filas o en las columnas. Escriba un **Nombre** y una **Fórmula**. Escriba el nombre de un elemento en la fórmula, o haga doble clic en él en la lista **Elementos** para colocarlo allí. Ponga entre comillas simples un nombre que tenga espacios, como en `='Este grande'+Oeste`. Puede usar números, `+ - * / ^ %` y paréntesis.

El nuevo elemento tiene una línea en el informe. En cada columna, muestra la fórmula calculada a partir de las líneas de los elementos que nombra. El nuevo elemento cuenta en los totales. Un total de sumas, como un total general o un subtotal, suma la línea del nuevo elemento a las líneas de los demás elementos. Así, con Este en 35, Oeste en 14 y Ambos (Este más Oeste) en 49, el total general es 98. Si un elemento que nombra la fórmula está oculto, la celda muestra **#¡REF!**. Si la fórmula nombra un elemento que no existe, la celda muestra **#¿NOMBRE?**.

Para cambiar un elemento, elija su nombre en el cuadro **Nombre**. Aparece su fórmula. Modifíquela y elija **Modificar**. **Eliminar** quita el elemento y su línea. Cada acción es un paso de **Deshacer**. Un elemento calculado se guarda con el archivo y se conserva al actualizar el informe. Dos tablas dinámicas creadas a partir de los mismos datos no pueden tener uno, porque un elemento calculado forma parte de los datos que comparten.

### Orden de resolución

A veces una celda tiene dos elementos calculados: uno está en su fila y el otro en su columna. El orden en que se calculan puede cambiar el resultado. Supongamos que el elemento de fila es `=Este-Oeste` y el de columna es `=Ene+1`. Calculado en un orden, la celda es (Este - Oeste) + 1. En el otro orden, es (Este + 1) - (Oeste + 1), que es un número distinto.

**Orden de resolución** en la pestaña **Herramientas de tabla dinámica** muestra los elementos calculados en el orden en que se calculan. El elemento que está más abajo en la lista se calcula el último. Elija un elemento y haga clic en **Subir** o **Bajar**. El orden se guarda con el archivo como orden de los elementos, y otros programas lo leen igual. **Fórmulas de lista** muestra el lugar de cada elemento en él.

## Mostrar las filas detrás de un valor

Haga doble clic en un valor de una tabla dinámica, o elija **Mostrar detalles** en la pestaña **Herramientas de tabla dinámica**. JeffCrunch crea una hoja nueva, llamada Detalle 1, 2, etc., con las filas de la lista que forman ese número. Los nombres de los campos están en la primera fila. F2 sigue editando la celda, si la celda se puede editar. **Deshacer** quita las filas, y otro **Deshacer** quita la hoja.

## Expandir y contraer

Un campo cuyos elementos tienen otros debajo puede mostrar u ocultar las líneas que hay bajo sus elementos. **Expandir todo el campo** y **Contraer todo el campo** lo hacen para todos los elementos del campo que seleccione en el panel.

## Opciones de tabla dinámica

**Opciones** en la pestaña **Herramientas de tabla dinámica** abre un cuadro con las opciones de la tabla.

- **Nombre** es el nombre de la tabla dinámica. Ninguna otra tabla dinámica del libro puede tenerlo.
- **Para valores de error, mostrar** y **Para celdas vacías, mostrar** colocan en esas celdas un texto propio.
- **Combinar y centrar celdas con etiquetas**, **Ajustar anchos de columna al actualizar la tabla** y **Conservar el formato de las celdas al actualizar la tabla** se guardan con la tabla.
- **Mostrar totales generales de filas** y **de columnas** equivalen a las opciones de Totales generales.
- **Actualizar los datos al abrir el archivo** vuelve a leer las celdas cada vez que se abre el libro. Es un paso de Deshacer, como cualquier actualización.

## Estilo: encabezados, bandas y galería

Las **Opciones de estilo** de la pestaña **Herramientas de tabla dinámica** activan o desactivan la fila de encabezado, la primera columna (encabezados de fila) y las bandas de filas y de columnas. **Estilos** tiene una galería de aspectos, y **Borrar estilo** quita el aspecto. Estos aspectos se dibujan igual que los de las tablas. **Filas en blanco** y **Repetir etiquetas** establecen la línea en blanco y las etiquetas repetidas para todos los campos de las filas a la vez.

## Diseño: subtotales, totales generales y disposición

- **Subtotales** muestra u oculta el total de cada grupo, al principio o al final del grupo. Cuando hay más de un campo en Filas, el total de cada campo externo es un subtotal.
- **Totales generales** tiene cuatro opciones: desactivado para filas y columnas, activado para ambos, activado solo para filas (una columna de totales a la derecha, uno por cada fila) y activado solo para columnas (una fila de totales al final).
- **Diseño del informe** es la forma de las etiquetas. En forma **compacta**, todos los campos de Filas comparten una columna, y un grupo nuevo se muestra un poco más a la derecha. En forma de **esquema**, cada campo tiene su propia columna, y cada grupo tiene una línea propia encima de sus elementos. En forma **tabular**, cada campo tiene su propia columna y el nombre del grupo está en su primera línea, con el subtotal debajo del grupo.

## La misma lista, con un filtro

Mueva **Mes** de **Columnas** a **Filtros**. El informe pierde sus columnas, y una línea encima de él muestra **Mes** y **(Todos)**. Los números son ahora el total de cada región en todos los meses.

## Gráfico dinámico

Haga clic en una celda de una tabla dinámica y elija **Insertar** y, después, **Gráfico dinámico**. JeffCrunch crea un gráfico de columnas con los valores de la tabla. No incluye los totales generales. El gráfico se marca en el archivo como gráfico de la tabla dinámica. Lee las celdas de la tabla, así que sus números cambian cuando actualiza la tabla o cambia su diseño. Si la tabla gana filas o columnas, cree el gráfico de nuevo, porque el gráfico conserva las celdas con las que se creó. Un gráfico dinámico que procede de un archivo se muestra tal como se guardó.

## Segmentaciones de datos y escalas de tiempo

Una segmentación de datos es un conjunto de botones que filtra una tabla dinámica, y una escala de tiempo es un control deslizante que hace lo mismo con las fechas.

Haga clic en una celda de una tabla dinámica y, después, en **Segmentaciones** de la pestaña Herramientas de tabla dinámica. Se abre el panel Segmentaciones de datos, que enumera las segmentaciones y escalas de tiempo del libro. Una segmentación tiene un botón por cada elemento de su campo. Los botones azules son los elementos que muestra la tabla.

Haga clic en un botón para mostrar solo ese elemento. Haga clic de nuevo para mostrar todos los elementos. Mantenga pulsada **Ctrl** y haga clic para añadir un elemento a los que se muestran, o para quitar uno. Cada clic es un paso de Deshacer, y Deshacer devuelve la tabla a su estado anterior. Al guardar, el archivo conserva la elección de la segmentación.

Una segmentación que venía con el archivo también se dibuja sobre las celdas, donde la coloca el archivo. Muestra su título y un botón por cada elemento. Los botones azules son los elementos que muestra la tabla. Haga clic en un botón para mostrar solo ese elemento; mantenga pulsada **Ctrl** y haga clic para añadir un elemento o quitarlo, y haga clic en el embudo de la parte superior derecha para volver a mostrar todos los elementos. Cada clic es un paso de Deshacer. Cuando la tabla se filtra de otra forma, o usted hace clic en Deshacer, los botones lo siguen. Cuando una segmentación es demasiado pequeña para mostrar todos sus botones, aparece una barra de desplazamiento a su derecha: gire la rueda del ratón sobre la segmentación, haga clic encima o debajo del cursor de la barra, o arrastre el cursor para ver el resto. Mantenga pulsada **Mayús** y haga clic en un segundo botón para elegir todo el tramo de elementos desde el primer botón pulsado hasta este; mantenga pulsadas **Mayús** y **Ctrl** para añadir el tramo a los elementos ya elegidos. Una segmentación impresa no puede desplazarse, así que imprime los botones que caben, y su última línea indica cuántos elementos más hay. Una segmentación que filtra una tabla que no es dinámica, o cuya tabla dinámica no está en el archivo, se dibuja con su título y una frase que indica que JeffCrunch todavía no puede usarla; se conserva en el archivo tal como estaba. Un libro abierto solo para lectura, y una hoja protegida contra cambios en las tablas dinámicas, no permiten que un clic filtre la tabla. Una segmentación se imprime con la hoja, salvo que el archivo indique lo contrario. Al hacer clic en un botón de una segmentación se filtra; al hacer clic en su título o en su borde se selecciona la segmentación como objeto, igual que una imagen. Una segmentación seleccionada tiene controladores: arrástrela para moverla, arrastre un controlador para cambiar su tamaño, o pulse una tecla de flecha para desplazarla. Mantenga pulsada **Ctrl** o **Mayús** y haga clic en el título para seleccionarla junto con otros objetos. Haga clic con el botón derecho en una segmentación para ver un menú con **Eliminar** y el panel **Segmentaciones de datos**. Pulse **Supr** para quitar una segmentación seleccionada: el archivo ya no tendrá esa segmentación, y el filtro que aplicaba a la tabla dinámica se mantiene hasta que lo cambie. Mover, cambiar el tamaño y eliminar son cada uno un paso de Deshacer, y un libro abierto solo para lectura, o una hoja protegida contra cambios en objetos, no los permite.

Para trabajar con una segmentación desde el teclado, pulse **F6** para ir a la primera segmentación de la hoja (**Mayús+F6** va a la última). Después use las teclas de flecha, **Inicio**, **Fin**, **Re Pág** y **Av Pág** para moverse entre sus botones, **Espacio** o **Intro** para elegir el botón, **Ctrl+Espacio** para añadirlo a los elementos elegidos o quitarlo, **Mayús+Espacio** para elegir el tramo desde el último botón elegido, y **Ctrl+Alt+C** para volver a mostrar todos los elementos. Pulse **Esc** para salir de los botones (las teclas de flecha mueven entonces la segmentación, y **Supr** la elimina; mientras esté entre los botones, **Supr** no hace nada), y **Esc** otra vez para volver a las celdas. **F6** pasa a la siguiente segmentación, y después de la última vuelve a las celdas.

**Conexiones de informe** indica qué tablas dinámicas filtra una segmentación. Una escala de tiempo muestra el intervalo de fechas que eligió. JeffCrunch todavía no dibuja ni modifica una escala de tiempo sobre las celdas. Ambas se conservan en el archivo tal como se guardaron.

## Límites

Una tabla dinámica puede tener hasta 2.000 campos, una lista de hasta cinco millones de filas y un informe de hasta cuatro millones de celdas. Un archivo que supere estos límites se muestra tal como se guardó y no se puede modificar. Volver a leer cien mil filas de veinte columnas y calcular el informe tardó unos medio segundo en el equipo donde se midió; un equipo más lento tarda más.
