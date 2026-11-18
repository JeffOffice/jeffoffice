# Herramientas de datos

<!-- keywords: previsión, hoja de previsión, pronóstico, valores futuros, intervalo de confianza, estacionalidad, tendencia, suavizado, formulario, formulario de datos, introducción de datos, registro, registros, nuevo registro, buscar siguiente, buscar anterior, criterios, administrador de escenarios, escenarios, qué pasaría si, resumen de escenarios, datos, pestaña datos, texto en columnas, dividir, separador, coma, tabulación, ancho fijo, quitar duplicados, buscar objetivo, subtotal, totales, filtro avanzado, condiciones, validación de datos, marcar datos no válidos, óvalo rojo, borrar círculos de validación, consolidar, combinar rangos, permitir, lista, lista desplegable, mensaje de entrada, alerta de error, filas repetidas, único, archivo de texto, csv, desde texto, columnas, delimitador, comillas, editar vínculos, actualizar valores, cambiar origen, vínculos, externo, origen, romper vínculo, comprobar estado, abrir origen -->

La pestaña **Datos** tiene las herramientas que trabajan con una lista de valores: obtener texto de un archivo, ordenar una lista, dividir texto en columnas y quitar las filas repetidas.

## Desde texto/CSV

**Desde texto/CSV** abre un archivo de texto (.csv, .tsv o .txt). Es lo mismo que **Abrir archivo de texto** en la pestaña Archivo. JeffCrunch pregunta cómo está dividido el archivo en columnas y qué tipo de texto contiene, y luego lo muestra como un libro nuevo.

## Editar vínculos

Si un libro tiene fórmulas que leen otro libro (como ='[Precios.xlsx]Hoja1'!A1), **Editar vínculos**, en el grupo **Obtener datos**, muestra la lista de esos otros libros, que se llaman orígenes. Cada fila indica dónde está el origen y su estado. Hasta que el libro se vuelva a abrir con sus orígenes, muestra los números que su archivo guardó de ellos.

- **Comprobar estado** busca cada origen en este equipo e indica **Encontrado**, o que no se encontró. Un origen en la web no se descarga, así que su estado indica que no se comprobó.
- **Actualizar valores** lee el origen elegido desde este equipo y da a cada celda que solo es una referencia a una de sus celdas el valor que tiene esa celda ahora. Una celda que hace más con el valor, como `=[1]Precios!A1*2`, conserva el número que guardó su archivo, y el mensaje debajo de la lista indica cuántas celdas se actualizaron y cuántas no. Actualizar valores es un solo paso, así que **Deshacer** devuelve los valores anteriores. Los valores nuevos también se escriben en la parte del vínculo dentro del archivo al guardar, de modo que el libro los muestra la próxima vez que se abra, aunque el origen no esté disponible.
- **Cambiar origen** permite elegir otro archivo para el vínculo seleccionado, por ejemplo cuando el libro se ha movido. Las fórmulas siguen igual. La nueva ubicación se escribe en el archivo al guardar, y **Deshacer** anula el cambio.
- **Abrir origen** abre el origen elegido en una ventana nueva, si se puede encontrar.
- **Romper vínculo** convierte cada celda que lee el origen elegido en el valor que muestra ahora, y un nombre que representaba un lugar de ese origen en **#¡REF!**. JeffCrunch pregunta antes. Romper vínculo es un solo paso, así que **Deshacer** restaura las fórmulas, los nombres y los vínculos, hasta que guarde el libro. Cuando no queda ningún vínculo, el archivo se guarda sin las partes que los contenían.

## Formulario

**Formulario** (en el grupo **Herramientas de datos** y en el cuadro de búsqueda) muestra una fila de una lista cada vez, como un pequeño formulario. Una lista es una tabla, o un bloque de celdas con una fila de nombres arriba y una fila de datos debajo de cada nombre. Haga clic primero en cualquier celda de la lista.

- Cada cuadro corresponde a una columna y lleva como etiqueta el nombre de la fila superior. Escriba en un cuadro para cambiar el valor. Un cuadro en gris contiene una fórmula: el formulario muestra el resultado, y la fórmula se cambia en la hoja.
- **Buscar siguiente** y **Buscar anterior** van a la fila siguiente y a la anterior. Al pasar a otra fila, o al presionar **Entrar**, se escribe en esta fila lo que ha introducido.
- **Nuevo** muestra un formulario vacío. Rellénelo y luego presione **Nuevo** de nuevo, **Buscar siguiente** o **Cerrar**. La fila se agrega debajo de la última fila de la lista. Una tabla crece una fila. Si una celda debajo de la lista no está vacía, el formulario se lo indica y no agrega nada.
- **Eliminar** quita la fila que está viendo, después de preguntar. Las filas de debajo suben. Una tabla siempre conserva al menos una fila de datos. Una fila oculta, o que un filtro oculta, no se muestra en el formulario, no se cuenta y nunca la elimina el formulario. Si la lista es demasiado larga para recorrerla completa, el formulario indica que solo muestra las primeras filas.
- **Restaurar** deshace lo que escribió en esta fila, antes de salir de ella.
- **Criterios** permite buscar filas. Escriba una condición en uno o más cuadros y luego presione **Buscar siguiente** o **Buscar anterior**. Una condición es un valor, como **Ann** (el texto coincide con el inicio de la palabra, y **?** y ***** representan letras), o un valor precedido de un signo: **>100**, **<=5**, **<>0**, **=Oslo**. Una fila debe cumplir todas las condiciones que escriba. Presione **Formulario** para volver, o **Borrar** para empezar de nuevo. Mientras haya criterios, **Buscar siguiente** y **Buscar anterior** omiten las filas que no los cumplen.
- **Cerrar** escribe la fila y cierra el formulario.

Cada fila que cambie, agregue o elimine es un solo paso, así que **Deshacer** en la hoja la revierte de una vez.

## Ordenar y filtrar

El grupo **Ordenar y filtrar** tiene los mismos comandos que el menú **Ordenar y filtrar** de la pestaña Inicio. Vea [Ordenar y filtrar](sort-filter).

## Texto en columnas

Use esta opción cuando una columna contiene varios fragmentos de texto que desea separar en columnas distintas, como «García, Ana, 31».

1. Seleccione las celdas de **una** columna.
2. Elija **Texto en columnas** en la pestaña **Datos**.
3. Elija cómo dividir el texto:
   - **Dividir con un separador**: marque los signos que hay entre los fragmentos: **Tabulación**, **Punto y coma**, **Coma**, **Espacio** u **Otro**, donde escribe su propio signo. **Considerar separadores consecutivos como uno solo** evita que dos comas seguidas creen un fragmento vacío. **Comillas de texto** indica el signo que encierra un fragmento que no debe dividirse, como «Lee, Bo». Dentro de las comillas, dos signos seguidos representan uno solo.
   - **Dividir en posiciones fijas**: escriba las posiciones donde empieza una columna nueva, contando los caracteres desde 1. Por ejemplo, 5, 12 inicia una columna nueva en el quinto y en el duodécimo carácter.
4. Elija el tipo de cada fragmento: **General** lee números y fechas como si los hubiera escrito, y **Texto** conserva cada fragmento tal cual, de modo que «007» sigue siendo «007».
5. En **Colocar las partes desde**, escriba la primera celda para los fragmentos. Al principio es la primera celda que seleccionó.
6. El cuadro debajo de las opciones muestra las primeras filas divididas como eligió. Presione **Aceptar**.

Los fragmentos se colocan en las celdas a la derecha de la primera celda y reemplazan lo que hubiera en ellas. Texto en columnas es un solo paso que puede deshacer.

## Quitar duplicados

Use esta opción para conservar solo la primera de cada grupo de filas repetidas.

1. Seleccione las celdas de la lista, o una celda de ella. JeffCrunch toma todo el bloque de datos que rodea una celda.
2. Elija **Quitar duplicados**.
3. Marque **Mis datos tienen encabezados** si la primera fila contiene nombres. Esa fila se conserva y no se compara.
4. Marque las columnas que deben coincidir para que dos filas se consideren repetidas. Dos filas se repiten solo cuando todas las columnas marcadas son iguales. Las mayúsculas y minúsculas se consideran iguales, y una fórmula cuenta como el valor que da.
5. Presione **Aceptar**. El cuadro indica cuántas filas se quitaron y cuántas quedan.

Las filas debajo de una fila eliminada suben dentro de las celdas seleccionadas, con sus formatos y fórmulas. Las celdas debajo de la selección no se mueven. Las filas vacías que quedan al final de la selección se borran. Quitar duplicados es un solo paso que puede deshacer.

## Buscar objetivo

Use esta opción cuando conoce el resultado que quiere, pero no el número que lo produce. Por ejemplo, una celda de pago contiene una fórmula que usa un tipo de interés, y desea que el pago sea 500. Buscar objetivo prueba números en la celda del tipo de interés hasta que el pago sea 500.

1. Elija **Buscar objetivo** en la pestaña **Datos**.
2. En **Celda con la fórmula**, escriba la celda que contiene la fórmula, como B3. Al principio es la celda que seleccionó.
3. En **Valor deseado**, escriba el resultado que quiere que dé la fórmula.
4. En **Celda que se va a cambiar**, escriba la celda que Buscar objetivo puede modificar, como B1. Debe contener un número, no una fórmula.
5. Presione **Aceptar**.

Si Buscar objetivo encuentra un número, se queda en la celda que se va a cambiar, y la barra de estado indica cuál es. Si no puede, no cambia nada y se le avisa. Pruebe con otro número inicial en la celda que se va a cambiar, y compruebe que la fórmula realmente usa esa celda. Buscar objetivo es un solo paso que puede deshacer.

## Administrador de escenarios

Un escenario es un conjunto con nombre de valores para algunas celdas. Use escenarios para probar «qué pasaría si» en una hoja. Por ejemplo, guarde un escenario «Ventas bajas» y otro «Ventas altas» para las mismas celdas de entrada, y muestre el que quiera revisar.

1. Elija **Administrador de escenarios** en la pestaña **Datos**.
2. Presione **Agregar**. Escriba un nombre, como Ventas altas. En **Celdas que cambian**, escriba las celdas que establece el escenario, como B1:B3 o D5. Un escenario puede cambiar hasta 32 celdas. Las celdas que seleccionó se rellenan automáticamente. Cada celda empieza con el valor que tiene ahora. Escriba en la tabla el valor que debe usar este escenario y presione **Aceptar**.
3. Seleccione un escenario en la lista y presione **Mostrar** para poner sus valores en las celdas. Hacer doble clic en un nombre tiene el mismo efecto. Presione **Editar** para cambiar su nombre, su comentario o sus valores. Presione **Eliminar** para quitarlo.

Mostrar, Agregar, Editar y Eliminar son pasos que puede deshacer uno a uno.

**Resumen** coloca todos los escenarios uno al lado del otro en una hoja nueva llamada Resumen de escenarios. Pide las celdas de resultado, que son las celdas que desea comparar, como el total en B10. Para cada escenario, el resumen muestra los valores de las celdas que cambian y el resultado de las celdas de resultado, calculado con los valores de ese escenario. Una columna llamada Valores actuales muestra cómo estaban las celdas cuando creó el resumen. Sus propias celdas no cambian.

Los escenarios se guardan en el libro. Un libro creado con otro programa conserva sus escenarios, y aquí puede verlos. Una copia guardada como archivo .xls antiguo o como .ods no los conserva, y al guardar se le indica.

## Subtotal

Use esta opción para colocar una fila de totales debajo de cada grupo de filas que tienen la misma entrada en una columna, y un total general al final. La lista necesita una fila de encabezado, y conviene ordenarla por la columna que agrupa, para que las entradas iguales queden juntas.

1. Seleccione una celda de la lista.
2. Elija **Subtotal** en el grupo **Esquema** de la pestaña **Datos**.
3. En **En cada cambio en**, elija la columna que agrupa las filas, como Región.
4. En **Usar función**, elija lo que se calcula: **Suma**, **Cuenta**, **Promedio**, **Máx**, **Mín** y otras.
5. En **Agregar subtotal a**, marque cada columna que quiera totalizar.
6. Deje marcada **Reemplazar subtotales actuales** para sustituir los subtotales que ya existan. Presione **Aceptar**.

Cada fila de total dice «Total Este» (o la entrada a la que corresponde) y contiene una fórmula SUBTOTALES, de modo que no tiene en cuenta las otras filas de total. Las filas de cada grupo quedan agrupadas, así que puede usar los botones 1, 2 y 3 junto a las filas para mostrar solo los totales. Para quitar los totales, abra **Subtotal** de nuevo y presione **Quitar todos**. Agregar o quitar subtotales es un solo paso que puede deshacer.

## Filtro avanzado

Los botones de filtro eligen filas según una columna cada vez. **Avanzado** (en el grupo **Ordenar y filtrar**) le permite escribir las condiciones en la hoja, de modo que pueden ser tan largas como necesite.

1. Junto a la lista, copie los encabezados de las columnas que quiere evaluar en una fila de celdas vacías.
2. Debajo de cada encabezado, escriba una condición. Las condiciones de la **misma fila** deben cumplirse todas (esto es «y»). Las condiciones en **filas distintas** son alternativas: una fila de la lista necesita cumplir solo una de esas filas (esto es «o»).
3. Elija **Avanzado**. Compruebe que **Intervalo de la lista** es la lista con su fila de encabezado, y escriba en **Intervalo de condiciones** las celdas de sus encabezados y condiciones.
4. Elija **Ocultar las demás filas de la lista**, o **Copiar las filas en otra ubicación** y escriba la primera celda del destino, como H1. Marque **Omitir las filas que repiten una anterior** para copiar solo una de cada una.
5. Presione **Aceptar**.

Cómo escribir una condición:

- Una palabra, como **An**, selecciona el texto que empieza por ella. Las mayúsculas y minúsculas se consideran iguales. **?** representa cualquier carácter y ***** cualquier secuencia de caracteres.
- Un número selecciona las celdas que son iguales a él.
- **=Ana** selecciona el texto que es exactamente Ana. **=** solo selecciona las celdas vacías.
- **>**, **>=**, **<**, **<=** y **<>** (distinto de) delante de un número o una palabra la comparan, como **>25**. **<>** solo selecciona las celdas que no están vacías.

Una fila de condiciones sin nada escrito selecciona todas las filas. Para volver a mostrar las filas ocultas, elija **Borrar**. Una copia se hace de inmediato y no cambia cuando cambia la lista. Filtro avanzado es un solo paso que puede deshacer.

## Validación de datos

Use esta opción para indicar qué puede escribirse en algunas celdas, de modo que una entrada incorrecta se impida o se cuestione.

1. Seleccione las celdas.
2. Elija **Validación de datos** en el grupo **Herramientas de datos** de la pestaña **Datos**.
3. En la ficha **Configuración**, elija qué **Permitir**: **Cualquier valor** (sin regla), **Número entero**, **Decimal**, **Lista**, **Fecha**, **Hora**, **Longitud de texto** o **Personalizada**.
4. Para un número, una fecha, una hora o una longitud de texto, elija **Datos** (**entre**, **no entre**, **igual a**, etc.) y escriba los límites. Un límite puede ser un valor o una fórmula que empiece por =.
5. Para una **Lista**, escriba las opciones separadas por comas, como Rojo,Verde,Azul, o escriba un rango de celdas después de =, como =$A$1:$A$5. La celda muestra entonces una flecha con las opciones. Desactive **Lista desplegable en celda** para ocultar la flecha.
6. Para **Personalizada**, escriba una fórmula que empiece por =. La entrada se permite cuando la fórmula es verdadera.
7. **Omitir espacios en blanco** permite pasar una celda vacía.

En la ficha **Mensaje de entrada**, marque la casilla y escriba un título y un mensaje. Se muestran junto a la celda cada vez que se selecciona.

En la ficha **Alerta de error**, elija qué ocurre con una entrada que no está permitida. **Detener** la rechaza. **Advertencia** pregunta si debe conservarla. **Información** avisa a la persona y la conserva. Escriba un título y un mensaje para la alerta.

**Marcar con círculo datos no válidos** (junto a **Validación de datos**) dibuja un óvalo rojo alrededor de cada celda de la hoja cuyo valor no permite su regla, como un número que ya estaba antes de la regla, o el resultado de una fórmula. Un mensaje le indica si todas las celdas están bien. Una celda que corrija pierde su óvalo. **Borrar círculos de validación** quita todos los óvalos.

**Borrar todo** quita las reglas de las celdas seleccionadas. La nueva regla reemplaza cualquier regla que tuvieran las celdas seleccionadas; las demás celdas conservan la suya. Cambiar reglas es un solo paso que puede deshacer, y las reglas se guardan en el archivo.

## Consolidar

Use esta opción para combinar rangos parecidos, como la misma tabla de varias hojas, en una sola tabla.

1. Elija **Consolidar** en el grupo **Herramientas de datos** de la pestaña **Datos**.
2. Elija la **Función** con la que combinar, como **Suma** o **Promedio**.
3. Escriba un rango en el cuadro y presione **Agregar**. Use **Hoja2!A1:C10** para un rango de otra hoja. Agregue cada rango que quiera combinar. Seleccione un rango en la lista y presione **Eliminar** para quitarlo.
4. Si los rangos tienen nombres en su primera fila, marque **La fila superior contiene los nombres de las columnas**. Si tienen nombres en su primera columna, marque **La columna izquierda contiene los nombres de las filas**.
5. En **Colocar el resultado en**, escriba la primera celda del resultado.
6. Marque **Crear vínculos a los datos de origen** si cada resultado debe ser una fórmula que lea sus celdas de origen, de modo que cambie cuando cambien ellas. Sin esta opción, los resultados son números fijos.
7. Presione **Aceptar**.

Sin nombres marcados, se combina la celda que ocupa el mismo lugar en cada rango (la primera celda de todos los rangos, luego la segunda, y así sucesivamente). Con los nombres marcados, se combinan las celdas con los mismos nombres, estén donde estén en cada rango, y un nombre que solo tengan algunos rangos sigue obteniendo una fila o columna. Consolidar es un solo paso que puede deshacer.

## Hoja de previsión

Hoja de previsión crea una hoja nueva con una tabla y un gráfico de valores futuros, calculados a partir de una línea temporal y una serie de valores. Detecta la tendencia y la estacionalidad repetitiva de los valores, y añade una banda dentro de la cual es probable que se mantengan los valores futuros.

1. Seleccione la línea temporal y los valores, dos columnas juntas (la línea temporal primero). La línea temporal son fechas o números separados a la misma distancia, como cada mes o cada 5 unidades. Una primera fila de nombres no se tiene en cuenta.
2. Elija **Hoja de previsión** en el grupo **Previsión** de la pestaña **Datos**.
3. Una imagen muestra la previsión, y cambia a medida que modifica las opciones. Elija **Gráfico de líneas** o **Gráfico de columnas**.
4. Configure las opciones:
   - **Intervalo de la línea temporal** e **Intervalo de valores** son las celdas que se leen. Escriba aquí otras celdas si la selección era incorrecta.
   - **Inicio de la previsión** está vacío para empezar después del último punto. Una fecha o número anterior al último punto empieza allí y usa solo los puntos anteriores a él, de modo que puede comparar la previsión con lo que ocurrió.
   - **Fin de la previsión** es el último momento que se prevé. Como máximo se prevén 10 000 puntos.
   - **Intervalo de confianza** indica cuánta seguridad ofrece la banda: 95 % al principio.
   - **Estacionalidad** se calcula observando los valores, o puede indicar la longitud de la temporada en puntos, o elegir ninguna. Los valores deben contener dos temporadas completas.
   - **Rellenar puntos que faltan usando** indica qué recibe un momento sin valor: una línea recta entre sus vecinos, o cero.
   - **Agregar duplicados usando** indica cómo se combinan los valores en el mismo momento: Promedio, Cuenta, CONTARA, Máx, Mediana, Mín o Suma.
   - **Incluir estadísticas de previsión** agrega una tabla con Alfa, Beta, Gamma, MASE, SMAPE, MAE y RMSE, que indican lo bien que la previsión se ajusta a los valores.
5. Si algo no es correcto, el cuadro lo explica con palabras sencillas (pasos que no son iguales, pocos puntos, texto en los valores), y **Crear** queda desactivado.
6. Presione **Crear**.

La hoja nueva se coloca antes de la que estaba usando. Su tabla tiene las columnas Línea temporal, Valores, Previsión, Límite inferior de confianza y Límite superior de confianza. La previsión y los límites son fórmulas que usan `PREDICCION.ETS` y `PREDICCION.ETS.CONFINT` sobre las columnas de línea temporal y de valores de la propia tabla, así que se vuelven a calcular cuando cambia un valor. El gráfico muestra los valores, la previsión y la banda. Hoja de previsión es un solo paso que puede deshacer.

Los números pueden diferir en unos pocos por ciento de los de otros programas de hojas de cálculo, que usan su propia forma de ajustar el suavizado; y una previsión que ya contiene un archivo conserva los valores guardados en él hasta que cambien los datos que lee.

No se usa un archivo con más de 1 000 000 de puntos.
