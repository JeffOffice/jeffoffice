# Herramientas de datos
<!-- keywords: datos, pestaña datos, texto en columnas, dividir, separador, coma, tabulación, ancho fijo, quitar duplicados, buscar objetivo, subtotal, totales, filtro avanzado, condiciones, validación de datos, marcar con círculo datos no válidos, óvalo rojo, borrar círculos de validación, consolidar, combinar rangos, permitir, lista, lista desplegable, mensaje de entrada, alerta de error, filas repetidas, únicos, archivo de texto, csv, desde texto, columnas, delimitador, comillas -->

La pestaña **Datos** tiene las herramientas que trabajan sobre una lista de valores: obtener texto de un archivo, ordenar una lista, dividir texto en columnas y quitar filas repetidas.

## Desde texto/CSV

**Desde texto/CSV** abre un archivo de texto (.csv, .tsv o .txt). Es lo mismo que **Abrir archivo de texto** en la pestaña Archivo. JeffCrunch le pregunta cómo está dividido el archivo en columnas y qué tipo de texto contiene, y luego lo muestra como un libro nuevo.

## Ordenar y filtrar

El grupo **Ordenar y filtrar** tiene los mismos comandos que el menú **Ordenar y filtrar** de la pestaña Inicio. Consulte [Ordenar y filtrar](sort-filter).

## Texto en columnas

Use esta herramienta cuando una columna contenga varios datos que desea tener en columnas separadas, como «Smith, Ann, 31».

1. Seleccione las celdas de **una** columna.
2. Elija **Texto en columnas** en la pestaña **Datos**.
3. Elija cómo dividir el texto:
   - **Dividir en un separador**: marque los signos que están entre los datos: **Tabulación**, **Punto y coma**, **Coma**, **Espacio** u **Otro**, donde escribe su propio signo. **Considerar separadores consecutivos como uno solo** evita que dos comas seguidas creen un dato vacío. **Texto entre comillas** indica el signo que encierra un dato que no debe dividirse, como «Lee, Bo». Dentro de las comillas, dos signos seguidos equivalen a uno.
   - **Dividir en posiciones fijas**: escriba las posiciones en las que empieza una columna nueva, contando los caracteres desde 1. Por ejemplo, 5, 12 inicia una columna nueva en el quinto y en el duodécimo carácter.
4. Elija el tipo de cada dato: **General** lee los números y las fechas como si los hubiera escrito, y **Texto** conserva cada dato tal cual, de modo que «007» sigue siendo «007».
5. En **Colocar las partes desde**, escriba la primera celda para los datos. Al principio es la primera celda que seleccionó.
6. El cuadro debajo de las opciones muestra las primeras filas divididas como eligió. Pulse **Aceptar**.

Los datos se colocan en las celdas a la derecha de la primera celda, y sustituyen lo que había en ellas. Texto en columnas es un paso que puede deshacer.

## Quitar duplicados

Use esta herramienta para conservar solo la primera de cada grupo de filas repetidas.

1. Seleccione las celdas de la lista o una celda de ella. JeffCrunch toma todo el bloque de datos que rodea a una sola celda.
2. Elija **Quitar duplicados**.
3. Marque **Mis datos tienen encabezados** si la primera fila contiene nombres. Esa fila se conserva y no se compara.
4. Marque las columnas que deben coincidir para que dos filas se consideren repetidas. Dos filas se repiten solo cuando todas las columnas marcadas son iguales. Las mayúsculas y las minúsculas se consideran iguales, y una fórmula cuenta como el valor que devuelve.
5. Pulse **Aceptar**. El cuadro le indica cuántas filas se han quitado y cuántas quedan.

Las filas situadas debajo de una fila eliminada suben dentro de las celdas seleccionadas, con sus formatos y fórmulas. Las celdas debajo de la selección no se mueven. Las filas vacías que quedan al final de la selección se borran. Quitar duplicados es un paso que puede deshacer.

## Buscar objetivo

Use esta herramienta cuando conoce el resultado que quiere, pero no el número que lo produce. Por ejemplo, una celda de pago contiene una fórmula que usa un tipo de interés, y quiere que el pago sea 500. Buscar objetivo prueba números en la celda del tipo de interés hasta que el pago sea 500.

1. Elija **Buscar objetivo** en la pestaña **Datos**.
2. En **Celda con la fórmula**, escriba la celda que contiene la fórmula, como B3. Al principio es la celda que seleccionó.
3. En **Valor deseado**, escriba el resultado que quiere que dé la fórmula.
4. En **Cambiar la celda**, escriba la celda que Buscar objetivo puede cambiar, como B1. Debe contener un número, no una fórmula.
5. Pulse **Aceptar**.

Si Buscar objetivo encuentra un número, este queda en la celda que se cambia, y la barra de estado indica cuál es. Si no lo encuentra, no cambia nada y se le avisa. Pruebe con otro número inicial en la celda que se cambia, y compruebe que la fórmula realmente usa esa celda. Buscar objetivo es un paso que puede deshacer.

## Subtotal

Use esta herramienta para poner una fila de totales debajo de cada grupo de filas que tienen el mismo valor en una columna, y un total general al final. La lista necesita una fila de encabezados y conviene ordenarla por la columna que agrupa, para que los valores iguales queden juntos.

1. Seleccione una celda de la lista.
2. Elija **Subtotal** en el grupo **Esquema** de la pestaña **Datos**.
3. En **En cada cambio en**, elija la columna que agrupa las filas, como Región.
4. En **Usar función**, elija lo que desea calcular: **Suma**, **Cuenta**, **Promedio**, **Máx**, **Mín** y otras.
5. En **Agregar subtotal a**, marque cada columna que desea totalizar.
6. Deje marcada la opción **Reemplazar subtotales actuales** para sustituir los subtotales que ya existan. Pulse **Aceptar**.

Cada fila de total dice «Total Este» (o el valor al que corresponde) y contiene una fórmula SUBTOTAL, de modo que no tiene en cuenta las otras filas de total. Las filas de cada grupo quedan agrupadas, así que puede usar los botones 1, 2 y 3 junto a las filas para mostrar solo los totales. Para quitar los totales, abra **Subtotal** de nuevo y pulse **Quitar todos**. Agregar o quitar subtotales es un paso que puede deshacer.

## Filtro avanzado

Los botones de filtro seleccionan las filas por una columna cada vez. **Avanzado** (en el grupo **Ordenar y filtrar**) le permite escribir las condiciones en la hoja, de modo que pueden ser tan largas como necesite.

1. Junto a la lista, copie los encabezados de las columnas que desea comprobar en una fila de celdas vacías.
2. Debajo de cada encabezado, escriba una condición. Las condiciones de la **misma fila** deben cumplirse todas (esto es «y»). Las condiciones de **filas distintas** son alternativas: una fila de la lista necesita cumplir solo una de esas filas (esto es «o»).
3. Elija **Avanzado**. Compruebe que **Rango de la lista** es la lista con su fila de encabezados, y escriba en **Rango de condiciones** las celdas de sus encabezados y condiciones.
4. Elija **Ocultar las demás filas de la lista**, o **Copiar las filas en otra ubicación** y escriba la primera celda del destino, como H1. Marque **Omitir las filas que repiten una anterior** para copiar solo una de cada una.
5. Pulse **Aceptar**.

Cómo escribir una condición:

- Una palabra, como **An**, selecciona el texto que empieza por ella. Las mayúsculas y las minúsculas se consideran iguales. **?** representa cualquier carácter y ***** cualquier secuencia de caracteres.
- Un número selecciona las celdas que son iguales a él.
- **=Ann** selecciona el texto que es exactamente Ann. **=** solo selecciona las celdas vacías.
- **>**, **>=**, **<**, **<=** y **<>** (distinto de) delante de un número o una palabra los comparan, como **>25**. **<>** solo selecciona las celdas que no están vacías.

Una fila de condiciones sin nada escrito selecciona todas las filas. Para volver a mostrar las filas ocultas, elija **Borrar**. La copia se hace al momento y no cambia cuando cambia la lista. Filtro avanzado es un paso que puede deshacer.

## Validación de datos

Use esta herramienta para indicar qué puede escribirse en algunas celdas, de modo que una entrada incorrecta se impida o se cuestione.

1. Seleccione las celdas.
2. Elija **Validación de datos** en el grupo **Herramientas de datos** de la pestaña **Datos**.
3. En la ficha **Configuración**, elija qué **Permitir**: **Cualquier valor** (sin regla), **Número entero**, **Decimal**, **Lista**, **Fecha**, **Hora**, **Longitud del texto** o **Personalizada**.
4. Para un número, una fecha, una hora o una longitud de texto, elija **Datos** (**entre**, **no entre**, **igual a**, etc.) y escriba los límites. Un límite puede ser un valor o una fórmula que empiece por =.
5. Para una **Lista**, escriba las opciones separadas por comas, como Rojo,Verde,Azul, o escriba un rango de celdas después de =, como =$A$1:$A$5. La celda mostrará entonces una flecha con las opciones. Desactive **Lista desplegable en celda** para ocultar la flecha.
6. Para **Personalizada**, escriba una fórmula que empiece por =. La entrada se permite cuando la fórmula es verdadera.
7. **Omitir espacios en blanco** permite que una celda vacía sea válida.

En la ficha **Mensaje de entrada**, marque la casilla y escriba un título y un mensaje. Se muestran junto a la celda siempre que esté seleccionada.

En la ficha **Alerta de error**, elija qué ocurre con una entrada que no está permitida. **Detener** la rechaza. **Advertencia** le pregunta si desea conservarla. **Información** avisa a la persona y la conserva. Escriba un título y un mensaje para la alerta.

**Marcar con círculo datos no válidos** (junto a **Validación de datos**) dibuja un óvalo rojo alrededor de cada celda de la hoja cuyo valor no permite su regla, como un número que ya estaba antes de la regla o el resultado de una fórmula. Un mensaje le indica si todas las celdas son correctas. Una celda que corrija pierde su óvalo. **Borrar círculos de validación** quita todos los óvalos.

**Borrar todo** quita las reglas de las celdas seleccionadas. La nueva regla sustituye cualquier regla que tuvieran las celdas seleccionadas; las demás celdas conservan la suya. Cambiar las reglas es un paso que puede deshacer, y las reglas se guardan en el archivo.

## Consolidar

Use esta herramienta para combinar rangos parecidos, como la misma tabla de varias hojas, en una sola tabla.

1. Elija **Consolidar** en el grupo **Herramientas de datos** de la pestaña **Datos**.
2. Elija la **Función** con la que combinar, como **Suma** o **Promedio**.
3. Escriba un rango en el cuadro y pulse **Agregar**. Use **Hoja2!A1:C10** para un rango de otra hoja. Agregue cada rango que desea combinar. Seleccione un rango de la lista y pulse **Eliminar** para quitarlo.
4. Si los rangos tienen nombres en su primera fila, marque **La fila superior contiene los nombres de las columnas**. Si tienen nombres en su primera columna, marque **La columna izquierda contiene los nombres de las filas**.
5. En **Colocar el resultado en**, escriba la primera celda del resultado.
6. Marque **Crear vínculos a los datos de origen** si cada resultado debe ser una fórmula que lea sus celdas de origen, de modo que cambie cuando cambien ellas. Si no lo marca, los resultados son números fijos.
7. Pulse **Aceptar**.

Si no marca ningún nombre, se combina la celda que ocupa el mismo lugar en cada rango (la primera celda de cada rango, después la segunda, y así sucesivamente). Si marca los nombres, se combinan las celdas con los mismos nombres, estén donde estén dentro de cada rango, y un nombre que solo tengan algunos rangos sigue generando una fila o una columna. Consolidar es un paso que puede deshacer.
