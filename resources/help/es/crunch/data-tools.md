# Herramientas de datos
<!-- keywords: administrador de escenarios, escenarios, qué pasa si, hipótesis, resumen de escenarios, datos, pestaña datos, texto en columnas, dividir, separador, coma, tabulación, ancho fijo, quitar duplicados, buscar objetivo, subtotal, totales, filtro avanzado, condiciones, validación de datos, marcar datos no válidos, óvalo rojo, borrar círculos de validación, consolidar, combinar rangos, permitir, lista, desplegable, mensaje de entrada, alerta de error, filas repetidas, único, archivo de texto, csv, desde texto, columnas, delimitador, comillas, editar vínculos, vínculos, externo, origen, romper vínculo, comprobar estado, abrir origen -->

La pestaña **Datos** tiene las herramientas que trabajan con una lista de valores: obtener texto de un archivo, ordenar una lista, dividir un texto en columnas y quitar filas repetidas.

## Desde texto/CSV

**Desde texto/CSV** abre un archivo de texto (.csv, .tsv o .txt). Es lo mismo que **Abrir archivo de texto** en la pestaña Archivo. JeffCrunch le pregunta cómo está dividido el archivo en columnas y qué tipo de texto contiene, y después lo muestra como un libro nuevo.

## Editar vínculos

Si un libro tiene fórmulas que leen otro libro (como ='[Precios.xlsx]Hoja1'!A1), **Editar vínculos**, en el grupo **Obtener datos**, muestra esos otros libros, que se llaman orígenes. Cada fila indica dónde está el origen y su estado. Hasta que el libro se vuelva a abrir con sus orígenes, muestra los números que su archivo guardó de ellos.

- **Comprobar estado** busca cada origen en este equipo e indica **Encontrado** o que no se encontró. Un origen en la web no se descarga, así que su estado indica que no se comprobó.
- **Abrir origen** abre el origen elegido en una ventana nueva, si se puede encontrar.
- **Romper vínculo** convierte en el valor que muestran ahora todas las celdas que leen el origen elegido, y un nombre que representaba un lugar de ese origen en **#¡REF!**. Esta acción no se puede deshacer, así que JeffCrunch pide confirmación primero. Cuando no queda ningún vínculo, el archivo se guarda sin las partes que los contenían.

## Ordenar y filtrar

El grupo **Ordenar y filtrar** tiene los mismos comandos que el menú **Ordenar y filtrar** de la pestaña Inicio. Consulte [Ordenar y filtrar](sort-filter).

## Texto en columnas

Utilícelo cuando una columna contiene varios fragmentos de texto que quiere tener en columnas separadas, como "Smith, Ann, 31".

1. Seleccione las celdas de **una** columna.
2. Elija **Texto en columnas** en la pestaña **Datos**.
3. Elija cómo dividir el texto:
   - **Dividir por un separador**: marque los signos que hay entre los fragmentos: **Tabulación**, **Punto y coma**, **Coma**, **Espacio** u **Otro**, donde escribe su propio signo. **Tratar separadores consecutivos como uno** evita que dos comas seguidas generen un fragmento vacío. **Texto entre comillas** indica el signo que encierra un fragmento que no debe dividirse, como "Lee, Bo". Dentro de las comillas, dos signos seguidos equivalen a uno.
   - **Dividir en posiciones fijas**: escriba las posiciones donde empieza una columna nueva, contando los caracteres desde 1. Por ejemplo, 5, 12 inicia una columna nueva en el quinto carácter y en el duodécimo.
4. Elija qué es cada fragmento: **General** lee los números y las fechas como si los hubiera escrito, y **Texto** conserva cada fragmento tal como es, de modo que "007" sigue siendo "007".
5. En **Colocar las partes desde**, escriba la primera celda para las partes. Al principio es la primera celda que seleccionó.
6. El cuadro debajo de las opciones muestra las primeras filas divididas como eligió. Pulse **Aceptar**.

Las partes se colocan en las celdas a la derecha de la primera celda y sustituyen lo que había en ellas. Texto en columnas es un solo paso que puede deshacer.

## Quitar duplicados

Utilícelo para conservar solo la primera fila de cada grupo de filas repetidas.

1. Seleccione las celdas de la lista, o una celda de ella. JeffCrunch toma todo el bloque de datos alrededor de una celda.
2. Elija **Quitar duplicados**.
3. Marque **Mis datos tienen encabezados** si la primera fila contiene nombres. Esa fila se conserva y no se compara.
4. Marque las columnas que deben coincidir para que dos filas cuenten como repetidas. Dos filas se repiten solo cuando todas las columnas marcadas son iguales. Las mayúsculas y las minúsculas cuentan como iguales, y una fórmula cuenta como el valor que devuelve.
5. Pulse **Aceptar**. El cuadro le indica cuántas filas se quitaron y cuántas quedan.

Las filas situadas debajo de una fila eliminada suben dentro de las celdas seleccionadas, con sus formatos y fórmulas. Las celdas situadas debajo de la selección no se mueven. Las filas vacías que quedan al final de la selección se borran. Quitar duplicados es un solo paso que puede deshacer.

## Buscar objetivo

Utilícelo cuando conoce el resultado que quiere, pero no el número que lo produce. Por ejemplo, una celda de pago contiene una fórmula que usa un tipo de interés, y quiere que el pago sea 500. Buscar objetivo prueba números en la celda del tipo hasta que el pago sea 500.

1. Elija **Buscar objetivo** en la pestaña **Datos**.
2. En **Celda con la fórmula**, escriba la celda que contiene la fórmula, como B3. Al principio es la celda que seleccionó.
3. En **Valor deseado**, escriba el resultado que quiere que dé la fórmula.
4. En **Celdas cambiantes**, escriba la celda que Buscar objetivo puede cambiar, como B1. Debe contener un número, no una fórmula.
5. Pulse **Aceptar**.

Si Buscar objetivo encuentra un número, este queda en la celda que cambia, y la barra de estado indica cuál es. Si no lo consigue, no cambia nada y se le avisa. Pruebe con otro número inicial en la celda que cambia y compruebe que la fórmula realmente usa esa celda. Buscar objetivo es un solo paso que puede deshacer.

## Administrador de escenarios

Un escenario es un conjunto con nombre de valores para algunas celdas. Use los escenarios para probar "qué pasa si" en una misma hoja. Por ejemplo, guarde un escenario "Ventas bajas" y otro "Ventas altas" para las mismas celdas de entrada, y muestre el que quiera revisar.

1. Elija **Administrador de escenarios** en la pestaña **Datos**.
2. Pulse **Agregar**. Escriba un nombre, como Ventas altas. En **Celdas cambiantes**, escriba las celdas que el escenario establece, como B1:B3 o D5. Un escenario puede cambiar hasta 32 celdas. Las celdas que seleccionó se rellenan automáticamente. Cada celda empieza con el valor que tiene ahora. Escriba en la tabla el valor que debe usar este escenario y pulse **Aceptar**.
3. Elija un escenario de la lista y pulse **Mostrar** para poner sus valores en las celdas. Hacer doble clic en un nombre hace lo mismo. Pulse **Editar** para cambiar su nombre, su comentario o sus valores. Pulse **Eliminar** para quitarlo.

Mostrar, Agregar, Editar y Eliminar son cada uno un paso que puede deshacer.

**Resumen** coloca todos los escenarios uno al lado del otro en una hoja nueva llamada Scenario Summary. Le pide las celdas de resultado, que son las celdas que quiere comparar, como el total en B10. Para cada escenario, el resumen muestra los valores de las celdas cambiantes y lo que suman las celdas de resultado, calculado con los valores de ese escenario. Una columna llamada Valores actuales muestra cómo estaban las celdas cuando creó el resumen. Sus celdas no se modifican.

Los escenarios se guardan en el libro. Un libro creado con otro programa conserva sus escenarios, y aquí los verá. Una copia guardada como archivo .xls antiguo o como .ods no los conserva, y el guardado se lo indica.

## Subtotal

Utilícelo para poner una fila de totales debajo de cada grupo de filas que tienen la misma entrada en una columna, y un total general al final. La lista necesita una fila de encabezados, y conviene ordenarla por la columna por la que agrupa, para que las entradas iguales queden juntas.

1. Seleccione una celda de la lista.
2. Elija **Subtotal** en el grupo **Esquema** de la pestaña **Datos**.
3. En **En cada cambio en**, elija la columna que agrupa las filas, como Región.
4. En **Usar función**, elija lo que quiere calcular: **Suma**, **Cuenta**, **Promedio**, **Máx**, **Mín** y otras.
5. En **Agregar subtotal a**, marque cada columna que quiere totalizar.
6. Deje marcada la opción **Reemplazar subtotales actuales** para sustituir los subtotales que ya existan. Pulse **Aceptar**.

Cada fila de total dice "Total Este" (o la entrada a la que corresponde) y contiene una fórmula SUBTOTAL, de modo que no incluye las otras filas de total. Las filas de cada grupo quedan agrupadas, así que puede usar los botones 1, 2 y 3 junto a las filas para mostrar solo los totales. Para quitar los totales, abra **Subtotal** de nuevo y pulse **Quitar todos**. Agregar o quitar subtotales es un solo paso que puede deshacer.

## Filtro avanzado

Los botones de filtro eligen filas por una columna cada vez. **Avanzado** (en el grupo **Ordenar y filtrar**) le permite escribir las condiciones en la hoja, de modo que pueden ser tan largas como necesite.

1. Junto a la lista, copie los encabezados de las columnas que quiere probar en una fila de celdas vacías.
2. Debajo de cada encabezado, escriba una condición. Las condiciones de la **misma fila** deben cumplirse todas (es decir, "y"). Las condiciones de **filas distintas** son alternativas: una fila de la lista debe cumplir solo una de esas filas (es decir, "o").
3. Elija **Avanzado**. Compruebe que **Rango de la lista** es la lista con su fila de encabezados, y escriba en **Rango de condiciones** las celdas de sus encabezados y condiciones.
4. Elija **Ocultar las demás filas de la lista**, o **Copiar las filas en otra ubicación** y escriba la primera celda del destino, como H1. Marque **Omitir las filas que repiten una anterior** para copiar solo una de cada una.
5. Pulse **Aceptar**.

Cómo escribir una condición:

- Una palabra, como **An**, elige el texto que empieza por ella. Las mayúsculas y las minúsculas cuentan como iguales. **?** representa cualquier carácter y ***** cualquier secuencia de caracteres.
- Un número elige las celdas que son iguales a él.
- **=Ana** elige el texto que es exactamente Ana. **=** solo elige las celdas vacías.
- **>**, **>=**, **<**, **<=** y **<>** (distinto de) delante de un número o una palabra la comparan con él, como **>25**. **<>** solo elige las celdas que no están vacías.

Una fila de condiciones sin nada en ella elige todas las filas. Para volver a mostrar las filas ocultas, elija **Borrar**. Una copia se hace en el momento y no cambia cuando cambia la lista. Filtro avanzado es un solo paso que puede deshacer.

## Validación de datos

Utilícela para indicar qué se puede escribir en algunas celdas, de modo que una entrada incorrecta se detenga o se cuestione.

1. Seleccione las celdas.
2. Elija **Validación de datos** en el grupo **Herramientas de datos** de la pestaña **Datos**.
3. En la ficha **Configuración**, elija qué **Permitir**: **Cualquier valor** (sin regla), **Número entero**, **Decimal**, **Lista**, **Fecha**, **Hora**, **Longitud del texto** o **Personalizada**.
4. Para un número, una fecha, una hora o una longitud de texto, elija **Datos** (**entre**, **no entre**, **igual a**, etc.) y escriba los límites. Un límite puede ser un valor o una fórmula que empiece por =.
5. Para una **Lista**, escriba las opciones separadas por comas, como Rojo,Verde,Azul, o escriba un rango de celdas después de =, como =$A$1:$A$5. La celda muestra entonces una flecha con las opciones. Desactive **Desplegable en celda** para ocultar la flecha.
6. Para **Personalizada**, escriba una fórmula que empiece por =. La entrada se permite cuando la fórmula es verdadera.
7. **Omitir blancos** deja pasar una celda vacía.

En la ficha **Mensaje de entrada**, marque la casilla y escriba un título y un mensaje. Se muestran junto a la celda cada vez que se selecciona.

En la ficha **Alerta de error**, elija qué ocurre con una entrada no permitida. **Detener** la rechaza. **Advertencia** pregunta si debe conservarla. **Información** avisa a la persona y la conserva. Escriba un título y un mensaje para la alerta.

**Marcar datos no válidos** (junto a **Validación de datos**) dibuja un óvalo rojo alrededor de cada celda de la hoja cuyo valor su regla no permite, como un número que ya estaba antes de la regla, o el resultado de una fórmula. Un mensaje le indica si todas las celdas son correctas. Una celda que corrija pierde su óvalo. **Borrar círculos de validación** quita todos los óvalos.

**Borrar todo** quita las reglas de las celdas seleccionadas. La nueva regla sustituye cualquier regla que tuvieran las celdas seleccionadas; las demás celdas conservan las suyas. Cambiar las reglas es un solo paso que puede deshacer, y las reglas se guardan en el archivo.

## Consolidar

Utilícelo para combinar rangos parecidos, como la misma tabla de varias hojas, en una sola tabla.

1. Elija **Consolidar** en el grupo **Herramientas de datos** de la pestaña **Datos**.
2. Elija la **Función** con la que combinar, como **Suma** o **Promedio**.
3. Escriba un rango en el cuadro y pulse **Agregar**. Use **Hoja2!A1:C10** para un rango de otra hoja. Agregue cada rango que quiera combinar. Seleccione un rango de la lista y pulse **Eliminar** para quitarlo.
4. Si los rangos tienen nombres en su primera fila, marque **La fila superior contiene los nombres de las columnas**. Si tienen nombres en su primera columna, marque **La columna izquierda contiene los nombres de las filas**.
5. En **Colocar el resultado en**, escriba la primera celda para el resultado.
6. Marque **Crear vínculos a los datos de origen** si cada resultado debe ser una fórmula que lea sus celdas de origen, de modo que cambie cuando cambien ellas. Sin esta opción, los resultados son números simples.
7. Pulse **Aceptar**.

Sin nombres marcados, se combina la celda que ocupa el mismo lugar en cada rango (la primera celda de todos los rangos, después la segunda, y así sucesivamente). Con los nombres marcados, se combinan las celdas con los mismos nombres, estén donde estén en cada rango, y un nombre que solo tienen algunos rangos sigue obteniendo una fila o una columna. Consolidar es un solo paso que puede deshacer.
