# Herramientas de datos
<!-- keywords: hoja de pronóstico, pronóstico, valores futuros, intervalo de confianza, estacionalidad, tendencia, suavizado, formulario, formulario de datos, introducción de datos, registro, registros, nuevo registro, buscar siguiente, buscar anterior, criterios, administrador de escenarios, escenarios, qué pasaría si, hipótesis, resumen de escenarios, datos, pestaña datos, texto en columnas, dividir, separador, coma, tabulación, ancho fijo, quitar duplicados, buscar objetivo, subtotal, totales, filtro avanzado, condiciones, validación de datos, marcar datos no válidos, óvalo rojo, borrar círculos de validación, consolidar, combinar rangos, permitir, lista, desplegable, mensaje de entrada, alerta de error, filas repetidas, único, archivo de texto, csv, desde texto, columnas, delimitador, comillas, editar vínculos, actualizar valores, cambiar origen, vínculos, externo, origen, romper vínculo, comprobar estado, abrir origen, fichero de texto -->

La pestaña **Datos** tiene las herramientas que trabajan con una lista de valores: obtener texto de un fichero, ordenar una lista, dividir un texto en columnas y eliminar filas repetidas.

## Desde texto/CSV

**Desde texto/CSV** abre un fichero de texto (.csv, .tsv o .txt). Es lo mismo que **Abrir fichero de texto** en la pestaña Archivo. JeffCrunch pregunta cómo se divide el fichero en columnas y qué contiene cada columna, y después lo muestra como un libro nuevo. La misma ventana se abre al abrir un fichero de texto o al usar **Pegar importación de texto**.

La ventana adivina qué contiene cada columna a partir de las primeras 200 filas del fichero y muestra la suposición en el cuadro situado encima de la columna. Cambie cualquier columna antes de abrir el fichero. Un tipo que cambie usted nunca se sustituye por una suposición posterior, aunque luego cambie el separador. La suposición solo hace tres cosas con una columna: la deja en **Tal como se escribió**, que es como se lee un fichero cuando no se adivina nada; la convierte en **Texto** para protegerla; o convierte las fechas de la columna a un orden que demuestran los valores. Estas son las reglas:

- Una columna se queda en **Tal como se escribió** cuando este formato lee bien sus valores: números, verdadero y falso, horas, fechas escritas con nombres de mes, números con signo como -5 y +3, palabras y números de versión como 1.13.2. Cambiarlos solo empeoraría las cosas.
- Una columna es de **fecha** solo cuando todos sus valores tienen un año de cuatro cifras (o son una fecha completa como 2024-05-13) y se leen como fecha en un único orden (mes, día, año; día, mes, año; o año, mes, día). Sus fechas se muestran como se muestran las fechas que usted escribe. Cuando más de un orden encaja con todos los valores, como 05/06/2024, la columna se mantiene como **Texto**, la ventana la indica y usted elige el orden en su cuadro.
- Los valores con tres números y sin año de cuatro cifras, como 1.13.2 o 13.05.24, nunca los convierte la suposición en fechas. Si Tal como se escribió los mantendría como texto, la columna se queda en Tal como se escribió; si Tal como se escribió los convertiría en fechas, la columna pasa a **Texto**. En cualquier caso, la ventana indica una columna que podría contener fechas con años de dos cifras, para que usted elija un orden.
- Una columna es **Texto** cuando un valor empieza por cero seguido de más cifras, cuando tiene más de 15 cifras, cuando un signo más va delante de siete o más cifras (un número de teléfono) o cuando aparece un exponente como 1E5 junto a otros valores.
- Un valor que empieza por un signo igual se deja para el cuadro **Leer fórmulas**. Si el cuadro está marcado, la columna se queda en Tal como se escribió para que el valor se lea como fórmula. Si no está marcado, la columna es **Texto**.
- Marque **La primera fila tiene nombres de columna** y la primera fila no se usa para la suposición. La ventana la marca cuando la primera fila es texto sobre números o fechas.
- Las columnas posteriores a las primeras 100 se importan como texto.

Solo se miran las primeras 200 filas, así que un valor más abajo puede resultar sorprendente. En una columna de fechas, un valor más abajo que no sea una fecha en el orden elegido se importa como texto, nunca como una fecha incorrecta. En una columna de Texto, cada valor se importa tal como está escrito. En una columna de Tal como se escribió, un valor más abajo se lee como lo leería Tal como se escribió en cualquier otro lugar.

**Pegar importación de texto** introduce el texto como si lo hubiera escrito usted, con sus propios separadores decimal y de miles, así que su ventana no tiene cuadros para esos separadores, y la suposición lee el texto del mismo modo que lo hace el pegado. La ventana de un fichero sí los tiene.

## Desde fichero

**Desde fichero**, en el grupo **Obtener datos**, carga las filas de un fichero JSON, de un fichero XML u otro libro como una tabla. Solo se puede cargar un fichero de este equipo. Se rechaza una dirección web o un fichero de otro equipo.

1. Haga clic en **Desde fichero**, después en **Examinar** y elija el fichero. Las primeras filas se muestran al instante, con el tipo que JeffCrunch encontró para cada columna a partir de esas filas: número, fecha, verdadero o falso, o texto. El fichero completo se comprueba al cargarlo, así que una columna puede acabar como texto. No cambia nada en la hoja hasta que haga clic en **Cargar**.
2. Cuando el fichero contiene más de una lista, elija la que desea cargar. En un fichero JSON es una lista de elementos con valores con nombre, como `$.items`. En un fichero XML es un elemento que se repite, como `/feed/entry`. En un libro es una hoja, una tabla o un rango con nombre.
3. Un elemento JSON dentro de otro elemento se convierte en una columna llamada `externo.interno`, hasta cuatro niveles. Una lista de valores sencillos se une en una sola celda (los primeros 10.000 valores) o se reparte en columnas (las primeras 16), según elija. En XML, los atributos y los elementos secundarios se convierten en columnas. Dos valores distintos nunca comparten columna: cuando dos recibirían el mismo nombre, al segundo se le añade un número o, en XML, su prefijo, y una nota lo indica. Un nombre repetido dentro de un mismo elemento JSON conserva su último valor, y una nota lo indica. Un número demasiado grande o demasiado pequeño para una celda se mantiene como texto, tal como está escrito, y un carácter que no se puede leer aparece como un signo de sustitución; ambos casos se anotan.
4. Marque **Cargar todas las columnas como texto** para conservar cada valor tal como está escrito. Una columna de números con un cero delante, como 007, siempre se mantiene como texto.
5. Escriba la celda donde irá la tabla y haga clic en **Cargar**. Si la tabla cubriera celdas que contienen algo, JeffCrunch pregunta antes de reemplazarlas. Si no hay espacio, o la hoja está protegida, se le avisa y no cambia nada.

La tabla es un paso de **Deshacer**. Recuerda el fichero y sus opciones. Para leer de nuevo el fichero, haga clic en una celda de la tabla y en **Actualizar** de la pestaña Datos. **Actualizar todo** es el botón grande junto a él: lee todas las tablas creadas con Obtener datos y, después, actualiza todas las tablas dinámicas, todo como un solo paso de Deshacer. La tabla crece o se reduce según las filas del fichero. Las columnas que haya añadido junto a la tabla se conservan: una fórmula que haya rellenado hacia abajo se rellena en las filas nuevas, y las filas que ya no están en el fichero también desaparecen de ella. Si el fichero ya no existe o no se puede leer, se le avisa y no cambia nada. Si cambiaron las columnas propias del fichero, se le indica cuáles faltan o son nuevas, y no cambia nada. Vuelva a crear la tabla con Obtener datos, o restaure las columnas en el fichero.

El vínculo con el fichero se guarda en la tabla, dentro de un libro `.xlsx` guardado, en una parte que otros programas ignoran. El vínculo contiene la ruta completa del fichero en este equipo, así que cualquiera que reciba el libro puede leer esa ruta. Para eliminarla antes de compartir el libro, use **Inspeccionar**, después **Vínculos a ficheros de datos** y, por último, **Quitar**. Una copia guardada como otro tipo de fichero no conserva el vínculo, y **Comprobar compatibilidad** lo indica. **Inspeccionar** enumera los vínculos en **Vínculos a ficheros de datos** y puede quitarlos. Las tablas y sus celdas se conservan.

Los ficheros se leen con límites: un fichero de hasta 64 MB, 200.000 filas, 256 columnas y 32.767 caracteres en una celda. Se rechaza un XML con declaración de tipo de documento o entidades. Cualquier parte recortada por un límite se indica.

## Editar vínculos

Si un libro tiene fórmulas que leen otro libro (como ='[Precios.xlsx]Hoja1'!A1), **Editar vínculos**, en el grupo **Obtener datos**, enumera esos otros libros, que se llaman orígenes. Cada fila muestra dónde está el origen y su estado. Hasta que el libro se vuelva a abrir con sus orígenes, muestra los números que su fichero guardó de ellos.

- **Comprobar estado** busca cada origen en este equipo y muestra **Encontrado**, o indica que no se encontró. Un origen en la web no se descarga, así que su estado indica que no se comprobó.
- **Actualizar valores** lee de este equipo el origen elegido y da a cada celda que solo es una referencia a una de sus celdas el valor que tiene esa celda ahora. Una celda que hace algo más con el valor, como `=[1]Precios!A1*2`, conserva el número que guardó su fichero, y el mensaje bajo la lista indica cuántas celdas se actualizaron y cuántas no. Actualizar valores es un solo paso, así que **Deshacer** devuelve los valores antiguos. Los valores nuevos también se escriben en la parte del vínculo dentro del fichero al guardar, de modo que el libro los muestra la próxima vez que se abra, aunque el origen no esté.
- **Cambiar origen** permite elegir otro fichero para el vínculo elegido, por ejemplo cuando se movió el libro. Las fórmulas se mantienen como están. El nuevo lugar se escribe en el fichero al guardar, y **Deshacer** revierte el cambio.
- **Abrir origen** abre el origen elegido en una nueva ventana, cuando se puede encontrar.
- **Romper vínculo** convierte cada celda que lee el origen elegido en el valor que muestra ahora, y un nombre que representaba un lugar de ese origen en **#¡REF!**. JeffCrunch pregunta antes. Romper vínculo es un solo paso, así que **Deshacer** devuelve las fórmulas, los nombres y los vínculos, hasta que guarde el libro. Cuando no queda ningún vínculo, el fichero se guarda sin las partes que los contenían.

## Formulario

**Formulario** (en el grupo **Herramientas de datos** y en el cuadro de búsqueda) muestra una fila de una lista cada vez, como un pequeño formulario. Una lista es una tabla, o un rango de celdas con una fila de nombres arriba y una fila de datos debajo de cada nombre. Haga clic primero en cualquier celda de la lista.

- Cada cuadro es una columna, con la etiqueta del nombre de la fila superior. Escriba en un cuadro para cambiar el valor. Un cuadro en gris contiene una fórmula: el formulario muestra el resultado, y usted cambia la fórmula en la hoja.
- **Buscar siguiente** y **Buscar anterior** van a la fila siguiente y a la anterior. Al ir a otra fila, o al pulsar **Intro**, se escribe lo que ha introducido en esta fila.
- **Nuevo** muestra un formulario vacío. Rellénelo y después pulse de nuevo **Nuevo**, **Buscar siguiente** o **Cerrar**. La fila se añade debajo de la última fila de la lista. Una tabla crece una fila. Si una celda debajo de la lista no está vacía, el formulario lo indica y no añade nada.
- **Eliminar** quita la fila que está viendo, después de preguntarlo. Las filas de debajo suben. Una tabla conserva siempre al menos una fila de datos. Una fila oculta, o que oculta un filtro, no se muestra en el formulario, no se cuenta y nunca la elimina el formulario. Si la lista es demasiado larga para recorrerla entera, el formulario indica que solo muestra las primeras filas.
- **Restaurar** deshace lo que escribió en esta fila, antes de salir de ella.
- **Criterios** le permite buscar filas. Escriba una condición en uno o más cuadros y pulse **Buscar siguiente** o **Buscar anterior**. Una condición es un valor, como **Ana** (el texto coincide con el inicio de la palabra, y **?** y ***** representan letras), o un valor precedido de un signo: **>100**, **<=5**, **<>0**, **=Oslo**. Una fila debe cumplir todas las condiciones que escriba. Pulse **Formulario** para volver, o **Borrar** para empezar de nuevo. Mientras haya criterios, **Buscar siguiente** y **Buscar anterior** omiten las filas que no los cumplen.
- **Cerrar** escribe la fila y cierra el formulario.

Cada fila que cambie, añada o elimine es un solo paso, así que **Deshacer** en la hoja la revierte de una vez.

## Ordenar y filtrar

El grupo **Ordenar y filtrar** tiene los mismos comandos que el menú **Ordenar y filtrar** de la pestaña Inicio. Consulte [Ordenar y filtrar](sort-filter).

## Texto en columnas

Use esta herramienta cuando una columna contenga varios fragmentos de texto que desee tener en columnas separadas, como «Smith, Ann, 31».

1. Seleccione las celdas de **una** columna.
2. Elija **Texto en columnas** en la pestaña **Datos**.
3. Elija cómo dividir el texto:
   - **Dividir con un separador**: marque los signos que hay entre los fragmentos: **Tabulación**, **Punto y coma**, **Coma**, **Espacio** u **Otro**, donde escribe su propio signo. **Tratar separadores consecutivos como uno** evita que dos comas seguidas produzcan un fragmento vacío. **Texto entre comillas** indica el signo que encierra un fragmento que no debe dividirse, como «Lee, Bo». Dentro de las comillas, dos signos seguidos equivalen a uno.
   - **Dividir en posiciones fijas**: escriba las posiciones en las que empieza una columna nueva, contando los caracteres desde 1. Por ejemplo, 5, 12 inicia una columna nueva en el quinto y en el duodécimo carácter.
4. Elija qué es cada fragmento: **General** lee los números y las fechas como si los hubiera escrito usted, y **Texto** mantiene cada fragmento tal como está, de modo que «007» sigue siendo «007».
   Para tratar un fragmento de forma distinta, escriba su número en **Fragmento** y elija cómo se lee: **Igual que arriba**, **Tal como se escribió**, **Texto**, una fecha escrita como **mes, día, año**, **día, mes, año** o **año, mes, día**, o **Omitir**, que no coloca ese fragmento en ningún sitio. El primer fragmento es el 1. Un fragmento de fecha puede tener el mes con nombre (**5 mar 2024**, **5 de marzo de 2024**) en inglés o en su idioma, puede estar separado por /, -, un punto, una coma o espacios, y puede ir seguido de una hora (**10:30**, **10:30:45** o **10:30 PM**). Un fragmento que no es una fecha en el orden elegido (un mes 13, un día que no existe, un año que no tiene dos ni cuatro cifras, o una hora que no tiene sentido) se mantiene como texto, tal como se escribió; nunca se lee en otro orden.
5. En **Colocar los fragmentos desde**, escriba la primera celda para los fragmentos. Al principio es la primera celda que seleccionó.
6. El cuadro bajo las opciones muestra las primeras filas divididas como eligió. Pulse **Aceptar**.

Los fragmentos se colocan en las celdas a la derecha de la primera celda, y reemplazan lo que hubiera en ellas. Texto en columnas es un paso que puede deshacer.

## Quitar duplicados

Use esta herramienta para conservar solo la primera de cada grupo de filas repetidas.

1. Seleccione las celdas de la lista, o una celda de ella. JeffCrunch toma todo el bloque de datos alrededor de una sola celda.
2. Elija **Quitar duplicados**.
3. Marque **Mis datos tienen fila de encabezado** si la primera fila contiene nombres. Esa fila se conserva y no se compara.
4. Marque las columnas que deben coincidir para que dos filas se consideren repetidas. Dos filas se repiten solo cuando todas las columnas marcadas son iguales. Las mayúsculas y las minúsculas cuentan como lo mismo, y una fórmula cuenta como el valor que da.
5. Pulse **Aceptar**. El cuadro le indica cuántas filas se eliminaron y cuántas quedan.

Las filas situadas debajo de una fila eliminada suben dentro de las celdas seleccionadas, con sus formatos y fórmulas. Las celdas situadas debajo de la selección no se mueven. Las filas vacías que quedan al final de la selección se borran. Quitar duplicados es un paso que puede deshacer.

## Buscar objetivo

Use esta herramienta cuando sepa el resultado que quiere, pero no el número que lo produce. Por ejemplo, una celda de pago contiene una fórmula que usa un tipo de interés, y usted quiere que el pago sea 500. Buscar objetivo prueba números en la celda del tipo hasta que el pago sea 500.

1. Elija **Buscar objetivo** en la pestaña **Datos**.
2. En **Celda con la fórmula**, escriba la celda que contiene la fórmula, como B3. Al principio es la celda que seleccionó.
3. En **Valor deseado**, escriba el resultado que quiere que dé la fórmula.
4. En **Celda que cambiar**, escriba la celda que Buscar objetivo puede modificar, como B1. Debe contener un número, no una fórmula.
5. Pulse **Aceptar**.

Si Buscar objetivo encuentra un número, este queda en la celda que cambiar, y la barra de estado indica cuál es. Si no lo encuentra, nada cambia y se le avisa. Pruebe con otro número inicial en la celda que cambiar, y compruebe que la fórmula usa realmente esa celda. Buscar objetivo es un paso que puede deshacer.

## Administrador de escenarios

Un escenario es un conjunto con nombre de valores para algunas celdas. Use escenarios para probar «qué pasaría si» en una misma hoja. Por ejemplo, conserve un escenario «Ventas bajas» y otro «Ventas altas» para las mismas celdas de entrada, y muestre el que quiera revisar.

1. Elija **Administrador de escenarios** en la pestaña **Datos**.
2. Pulse **Agregar**. Escriba un nombre, como Ventas altas. En **Celdas que cambian**, escriba las celdas que establece el escenario, como B1:B3 o D5. Un escenario puede cambiar hasta 32 celdas. Las celdas que seleccionó se rellenan automáticamente. Cada celda empieza con el valor que tiene ahora. Escriba en la tabla el valor que debe usar este escenario y pulse **Aceptar**.
3. Elija un escenario de la lista y pulse **Mostrar** para poner sus valores en las celdas. Hacer doble clic en un nombre hace lo mismo. Pulse **Editar** para cambiar su nombre, su comentario o sus valores. Pulse **Eliminar** para quitarlo.

Mostrar, Agregar, Editar y Eliminar son cada uno un paso que puede deshacer.

**Resumen** coloca todos los escenarios uno junto a otro en una hoja nueva llamada Resumen de escenarios. Pide las celdas de resultado, que son las celdas que quiere comparar, como el total en B10. Para cada escenario, el resumen muestra los valores de las celdas que cambian y el resultado de las celdas de resultado, calculado con los valores de ese escenario. Una columna llamada Valores actuales muestra cómo estaban las celdas cuando creó el resumen. Sus propias celdas no se modifican.

Los escenarios se guardan en el libro. Un libro creado con otro programa conserva sus escenarios, y aquí los verá. Una copia guardada como fichero .xls antiguo o como .ods no los conserva, y al guardar se le indica.

## Subtotal

Use esta herramienta para colocar una fila de totales bajo cada grupo de filas que tienen la misma entrada en una columna, y un total general al final. La lista necesita una fila de encabezado, y conviene ordenarla por la columna por la que agrupa, para que las entradas iguales queden juntas.

1. Seleccione una celda de la lista.
2. Elija **Subtotal** en el grupo **Esquema** de la pestaña **Datos**.
3. En **En cada cambio en**, elija la columna que agrupa las filas, como Región.
4. En **Usar función**, elija lo que desea calcular: **Suma**, **Cuenta**, **Promedio**, **Máx**, **Mín** y otras.
5. En **Agregar subtotal a**, marque cada columna que desea totalizar.
6. Deje marcado **Reemplazar subtotales actuales** para sustituir los subtotales que ya existan.
7. **Resumen debajo de los datos** toma el valor que tiene el esquema de la hoja. Déjelo marcado para colocar cada fila de totales debajo de su grupo, o desmárquelo para colocarla encima del grupo, con el total general arriba. Pulse **Aceptar**.

Cada fila de total dice «Total Este» (o la entrada a la que corresponde) y contiene una fórmula SUBTOTALES, de modo que no tiene en cuenta las otras filas de total. Las filas de cada grupo quedan agrupadas, para que pueda usar los botones 1, 2 y 3 junto a las filas y mostrar solo los totales. Para eliminar los totales, abra **Subtotal** de nuevo y pulse **Quitar todos**. Si los totales se pasaron al otro lado de una hoja que ya tenía sus propios grupos, la barra de estado indica que esos grupos tienen ahora sus filas de resumen en ese lado, y Quitar todos devuelve el lado a como estaba antes del primer subtotal, salvo que lo haya cambiado usted después. Cuando los totales están arriba, los botones del esquema se sitúan en las filas de total, de modo que un botón de más o de menos junto a «Total Este» muestra u oculta las filas del Este que están debajo. Si desmarca **Reemplazar subtotales actuales**, los nuevos totales deben ir en el mismo lado que los que ya existen. En caso contrario, no se cambia nada y un mensaje lo indica. Agregar o quitar subtotales es un paso que puede deshacer.

## Filtro avanzado

Los botones de filtro seleccionan filas por una columna cada vez. **Avanzado** (en el grupo **Ordenar y filtrar**) le permite escribir las condiciones en la hoja, de modo que pueden ser tan largas como necesite.

1. Junto a la lista, copie los encabezados de las columnas que quiere probar en una fila de celdas vacías.
2. Debajo de cada encabezado, escriba una condición. Las condiciones de la **misma fila** deben cumplirse todas (es «y»). Las condiciones de **filas distintas** son alternativas: una fila de la lista debe cumplir solo una de esas filas (es «o»).
3. Elija **Avanzado**. Compruebe que **Rango de la lista** es la lista con su fila de encabezado, y escriba en **Rango de condiciones** las celdas de sus encabezados y condiciones.
4. Elija **Ocultar las demás filas de la lista**, o **Copiar los resultados a otra ubicación** y escriba la primera celda del lugar, como H1. Marque **Omitir filas duplicadas** para copiar solo una de cada una.
5. Pulse **Aceptar**.

Cómo escribir una condición:

- Una palabra, como **An**, selecciona el texto que empieza por ella. Las mayúsculas y las minúsculas cuentan como lo mismo. **?** representa cualquier carácter y ***** cualquier serie de caracteres.
- Un número selecciona las celdas que son iguales a él.
- **=Ana** selecciona el texto que es exactamente Ana. **=** solo selecciona las celdas vacías.
- **>**, **>=**, **<**, **<=** y **<>** (distinto de) delante de un número o una palabra la comparan, como **>25**. **<>** solo selecciona las celdas que no están vacías.

Una fila de condiciones sin nada escrito selecciona todas las filas. Para volver a mostrar las filas ocultas, elija **Borrar**. La copia se hace al momento y no cambia cuando cambia la lista. Filtro avanzado es un paso que puede deshacer.

## Validación de datos

Use esta herramienta para indicar qué se puede escribir en algunas celdas, de modo que una entrada incorrecta se detenga o se cuestione.

1. Seleccione las celdas.
2. Elija **Validación de datos** en el grupo **Herramientas de datos** de la pestaña **Datos**.
3. En la ficha **Configuración**, elija qué **Permitir**: **Cualquier valor** (sin regla), **Número entero**, **Decimal**, **Lista**, **Fecha**, **Hora**, **Longitud de texto** o **Personalizada**.
4. Para un número, una fecha, una hora o una longitud de texto, elija **Datos** (**entre**, **no entre**, **igual a**, etc.) y escriba los límites. Un límite puede ser un valor o una fórmula que empiece por =.
5. Para una **Lista**, escriba las opciones separadas por comas, como Rojo,Verde,Azul, o escriba un rango de celdas precedido de =, como =$A$1:$A$5. La celda muestra entonces una flecha con las opciones. Desactive **Desplegable en celda** para ocultar la flecha.
6. Para **Personalizada**, escriba una fórmula que empiece por =. La entrada se permite cuando la fórmula es verdadera.
7. **Omitir espacios en blanco** permite que una celda vacía pase la validación.

En la ficha **Mensaje de entrada**, marque la casilla y escriba un título y un mensaje. Se muestran junto a la celda siempre que esté seleccionada.

En la ficha **Alerta de error**, elija qué ocurre con una entrada no permitida. **Detener** la rechaza. **Advertencia** pregunta si debe conservarla. **Información** avisa a la persona y la conserva. Escriba un título y un mensaje para la alerta.

**Marcar datos no válidos** (junto a **Validación de datos**) coloca un óvalo rojo alrededor de cada celda de la hoja cuyo valor su regla no permite, como un número que ya estaba antes de la regla, o el resultado de una fórmula. Un mensaje le indica si todas las celdas son correctas. Una celda que corrija pierde su óvalo. **Borrar círculos de validación** quita todos los óvalos.

**Borrar todo** quita las reglas de las celdas seleccionadas. La nueva regla sustituye cualquier regla que tuvieran las celdas seleccionadas; las demás celdas conservan las suyas. Cambiar las reglas es un paso que puede deshacer, y las reglas se guardan en el fichero.

## Consolidar

Use esta herramienta para combinar en una sola tabla rangos parecidos, como la misma tabla de varias hojas.

1. Elija **Consolidar** en el grupo **Herramientas de datos** de la pestaña **Datos**.
2. Elija la **Función** con la que combinar, como **Suma** o **Promedio**.
3. Escriba un rango en el cuadro y pulse **Agregar**. Use **Hoja2!A1:C10** para un rango de otra hoja. Agregue cada rango que quiera combinar. Seleccione un rango de la lista y pulse **Eliminar** para quitarlo.
4. Si los rangos tienen nombres en su primera fila, marque **La fila superior contiene los nombres de las columnas**. Si tienen nombres en su primera columna, marque **La columna izquierda contiene los nombres de las filas**.
5. En **Colocar el resultado en**, escriba la primera celda del resultado.
6. Marque **Crear vínculos a los datos de origen** si cada resultado debe ser una fórmula que lee sus celdas de origen, de modo que cambie cuando ellas cambien. Sin esta opción, los resultados son números fijos.
7. Pulse **Aceptar**.

Sin nombres marcados, se combina la celda que ocupa el mismo lugar en cada rango (la primera celda de todos los rangos, después la segunda, y así sucesivamente). Con los nombres marcados, se combinan las celdas con los mismos nombres, estén donde estén en cada rango, y un nombre que solo tienen algunos rangos sigue obteniendo una fila o columna. Consolidar es un paso que puede deshacer.

## Hoja de pronóstico

La hoja de pronóstico crea una hoja nueva con una tabla y un gráfico de valores futuros, calculados a partir de una línea temporal y una serie de valores. Halla la tendencia y la estacionalidad que se repite en los valores, y añade una banda dentro de la cual es probable que se mantengan los valores futuros.

1. Seleccione la línea temporal y los valores, dos columnas juntas (primero la línea temporal). La línea temporal son fechas o números separados por la misma distancia, como cada mes o cada 5 unidades. Una primera fila de nombres no se tiene en cuenta.
2. Elija **Hoja de pronóstico** en el grupo **Pronóstico** de la pestaña **Datos**.
3. Una imagen muestra el pronóstico y cambia a medida que modifica las opciones. Elija **Gráfico de líneas** o **Gráfico de columnas**.
4. Configure las opciones:
   - **Rango de la línea temporal** y **Rango de valores** son las celdas que se leen. Escriba aquí otras celdas si la selección fue incorrecta.
   - **Inicio del pronóstico** está vacío para empezar después del último punto. Una fecha o número anterior al último punto empieza allí y usa solo los puntos anteriores a él, de modo que puede comparar el pronóstico con lo que ocurrió.
   - **Fin del pronóstico** es el último momento que se pronostica. Como máximo se pronostican 10.000 puntos.
   - **Intervalo de confianza** indica cuánta seguridad da la banda, al principio un 95 %.
   - **Estacionalidad** se halla observando los valores, o puede establecer la longitud de la temporada en puntos, o elegir ninguna. Los valores deben contener dos temporadas completas.
   - **Rellenar puntos que faltan usando** indica qué recibe un momento sin valor: una línea recta entre sus vecinos, o cero.
   - **Agregar duplicados usando** indica cómo se combinan los valores del mismo momento: Promedio, Cuenta, CuentaA, Máx, Mediana, Mín o Suma.
   - **Incluir estadísticas del pronóstico** agrega una tabla con Alfa, Beta, Gamma, MASE, SMAPE, MAE y RMSE, que indican lo bien que el pronóstico se ajusta a los valores.
5. Si algo no es correcto, el cuadro lo explica con palabras sencillas (pasos que no son iguales, muy pocos puntos, texto en los valores), y **Crear** espera.
6. Pulse **Crear**.

La hoja nueva se coloca antes de la que estaba usando. Su tabla tiene las columnas Línea temporal, Valores, Pronóstico, Límite inferior de confianza y Límite superior de confianza. El Pronóstico y los límites son fórmulas que usan `PRONOSTICO.ETS` y `PRONOSTICO.ETS.INT.CONF` sobre las propias columnas Línea temporal y Valores de la tabla, así que se vuelven a calcular cuando cambia un valor. El gráfico muestra los valores, el pronóstico y la banda. Crear la hoja de pronóstico es un paso que puede deshacer.

Los números pueden diferir en unos pocos por ciento de los de otros programas de hojas de cálculo, que usan su propio método para ajustar el suavizado; y un pronóstico que ya contenga un fichero conserva los valores guardados en él hasta que cambien los datos que lee.

No se utiliza un fichero con más de 1.000.000 de puntos.
