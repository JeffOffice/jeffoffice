# Funciones de búsqueda y referencia
<!-- keywords: funciones, ADDRESS, AREAS, COLUMN, COLUMNS, FILTER, FORMULATEXT, GETPIVOTDATA, HLOOKUP, HYPERLINK, INDEX, INDIRECT, LOOKUP, MATCH, OFFSET, ROW, ROWS, SORT, SORTBY, TRANSPOSE, UNIQUE, VLOOKUP, XLOOKUP, XMATCH -->

Estas son las funciones de búsqueda y referencia que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y cuál es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y "..." significa que puede añadir todos los más que necesite del último. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece un pequeño consejo debajo de la celda con sus argumentos, y si hace clic en el nombre de la función dentro del consejo, se abre su entrada en esta página.

## ADDRESS

`ADDRESS(row_num, column_num, [abs_num], [a1], [sheet_text])`

Escribe la dirección de una celda como texto, a partir de sus números de fila y de columna.

- `row_num`: El número de la fila dentro del rango, contando la primera fila como 1.
- `column_num`: El número de la columna dentro del rango, contando la primera columna como 1.
- `abs_num` (opcional): Qué parte de la dirección queda fija: 1 toda, 2 solo la fila, 3 solo la columna, 4 nada.
- `a1` (opcional): Verdadero o se omite para leer la dirección como letra de columna y número de fila; falso para leerla como números de fila y columna.
- `sheet_text` (opcional): El nombre de la hoja que se antepone a la dirección.

## AREAS

`AREAS(reference)`

Devuelve el número de rangos de celdas separados que hay en una referencia.

- `reference`: Una celda o un rango de celdas.

## COLUMN

`COLUMN([reference])`

Devuelve el número de columna de una celda.

- `reference` (opcional): Una celda o un rango de celdas.

## COLUMNS

`COLUMNS(array)`

Cuenta las columnas de un rango de celdas.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.

## FILTER

`FILTER(array, include, [if_empty])`

Devuelve las filas o columnas de un rango que cumplen una prueba.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `include`: Una columna o fila de valores verdadero y falso, uno por cada fila o columna del rango: verdadero la conserva.
- `if_empty` (opcional): Lo que se devuelve cuando ninguna fila o columna cumple la prueba (si se omite, el error #¡CALC!).

## FORMULATEXT

`FORMULATEXT(reference)`

Devuelve la fórmula de una celda como texto.

- `reference`: Una celda o un rango de celdas.

## GETPIVOTDATA

`GETPIVOTDATA(data_field, pivot_table, [field1], [item1], ...)`

Devuelve el valor que muestra una tabla dinámica para un valor y para los elementos de sus filas y columnas. Una celda de la tabla es el segundo argumento; los campos indicados deben ser los primeros de las filas o de las columnas.

- `data_field`: El nombre del valor que se devuelve, entre comillas, tal como lo muestra la tabla dinámica ("Suma de Importe").
- `pivot_table`: Cualquier celda de la tabla dinámica.
- `field1` (opcional): El nombre de un campo de las filas o de las columnas, entre comillas (puede omitirse junto con su elemento).
- `item1` (opcional): El elemento de ese campo para el que se devuelve el valor, entre comillas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## HLOOKUP

`HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])`

Busca un valor en la primera fila de una tabla y devuelve un valor de la misma columna.

- `lookup_value`: El valor que se busca.
- `table_array`: El rango de celdas en el que se busca; el valor buscado debe estar en su primera columna.
- `row_index_num`: El número de la fila de la tabla de la que se devuelve un valor, contando la primera fila como 1.
- `range_lookup` (opcional): Falso busca una coincidencia exacta; verdadero o se omite busca el valor más próximo por debajo (la primera columna debe estar ordenada de forma ascendente).

## HYPERLINK

`HYPERLINK(link_location, [friendly_name])`

Crea una celda que abre un vínculo al hacer clic en ella.

- `link_location`: Adónde lleva el vínculo: una dirección web, un fichero o un lugar de este libro.
- `friendly_name` (opcional): El texto que se muestra en la celda (si se omite, el propio vínculo).

## INDEX

`INDEX(array, row_num, [column_num], [area_num])`

Devuelve el valor de una fila y una columna de un rango de celdas.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `row_num`: El número de la fila dentro del rango, contando la primera fila como 1.
- `column_num` (opcional): El número de la columna dentro del rango, contando la primera columna como 1.
- `area_num` (opcional): Qué rango usar cuando el primer argumento indica varios (1 para el primero).

## INDIRECT

`INDIRECT(ref_text, [a1])`

Devuelve la celda a la que apunta una dirección escrita como texto.

- `ref_text`: Una dirección escrita como texto, como "B5".
- `a1` (opcional): Verdadero o se omite para leer la dirección como letra de columna y número de fila; falso para leerla como números de fila y columna.

## LOOKUP

`LOOKUP(lookup_value, lookup_vector, [result_vector])`

Busca un valor en una fila o columna y devuelve el valor de la misma posición en otra.

- `lookup_value`: El valor que se busca.
- `lookup_vector`: La única fila o columna de celdas en la que se busca.
- `result_vector` (opcional): La única fila o columna de celdas de la que se toma el resultado.

## MATCH

`MATCH(lookup_value, lookup_array, [match_type])`

Devuelve la posición de un valor en una lista.

- `lookup_value`: El valor que se busca.
- `lookup_array`: El rango de celdas, una fila o una columna, en el que se busca.
- `match_type` (opcional): 1 o se omite busca el mayor valor que no supere el buscado (la lista debe estar ordenada de forma ascendente); 0 busca una coincidencia exacta; -1 busca el menor valor que no sea inferior al buscado (la lista debe estar ordenada de forma descendente).

## OFFSET

`OFFSET(reference, rows, cols, [height], [width])`

Devuelve las celdas que están a un número determinado de filas y columnas de una celda inicial.

- `reference`: Una celda o un rango de celdas.
- `rows`: Cuántas filas se desplaza: positivo hacia abajo, negativo hacia arriba.
- `cols`: Cuántas columnas se desplaza: positivo hacia la derecha, negativo hacia la izquierda.
- `height` (opcional): Cuántas filas de alto tiene el resultado.
- `width` (opcional): Cuántas columnas de ancho tiene el resultado.

## ROW

`ROW([reference])`

Devuelve el número de fila de una celda.

- `reference` (opcional): Una celda o un rango de celdas.

## ROWS

`ROWS(array)`

Cuenta las filas de un rango de celdas.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.

## SORT

`SORT(array, [sort_index], [sort_order], [by_col])`

Devuelve un rango con sus filas o columnas ordenadas.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `sort_index` (opcional): El número de la columna (o fila) por la que se ordena (1 si se omite).
- `sort_order` (opcional): 1 o se omite para orden ascendente; -1 para descendente.
- `by_col` (opcional): Verdadero para trabajar con columnas; falso o se omite para trabajar con filas.

## SORTBY

`SORTBY(array, by_array1, [sort_order1], ...)`

Devuelve un rango ordenado según el orden de uno o más rangos.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `by_array1`: El rango de celdas según el cual se ordena el primer rango; debe tener la misma longitud que el primero.
- `sort_order1` (opcional): 1 o se omite para orden ascendente; -1 para descendente.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## TRANSPOSE

`TRANSPOSE(array)`

Convierte las filas en columnas y las columnas en filas.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.

## UNIQUE

`UNIQUE(array, [by_col], [exactly_once])`

Devuelve las filas o columnas distintas de un rango.

- `array`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `by_col` (opcional): Verdadero para trabajar con columnas; falso o se omite para trabajar con filas.
- `exactly_once` (opcional): Verdadero para devolver solo los elementos que aparecen una vez; falso o se omite para devolver cada elemento distinto.

## VLOOKUP

`VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])`

Busca un valor en la primera columna de una tabla y devuelve un valor de la misma fila.

- `lookup_value`: El valor que se busca.
- `table_array`: El rango de celdas en el que se busca; el valor buscado debe estar en su primera columna.
- `col_index_num`: El número de la columna de la tabla de la que se devuelve un valor, contando la primera columna como 1.
- `range_lookup` (opcional): Falso busca una coincidencia exacta; verdadero o se omite busca el valor más próximo por debajo (la primera columna debe estar ordenada de forma ascendente).

## XLOOKUP

`XLOOKUP(lookup_value, lookup_array, return_array, [if_not_found], [match_mode], [search_mode])`

Busca un valor en una lista y devuelve el elemento correspondiente de otra lista.

- `lookup_value`: El valor que se busca.
- `lookup_array`: El rango de celdas, una fila o una columna, en el que se busca.
- `return_array`: El rango de celdas del que se toma el resultado de la coincidencia.
- `if_not_found` (opcional): Lo que se devuelve cuando no hay coincidencia (si se omite, el error #N/A).
- `match_mode` (opcional): 0 o se omite para una coincidencia exacta; -1 para el siguiente menor; 1 para el siguiente mayor; 2 para usar comodines.
- `search_mode` (opcional): 1 o se omite para buscar desde el primero; -1 desde el último; 2 o -2 para una búsqueda binaria en una lista ordenada.

## XMATCH

`XMATCH(lookup_value, lookup_array, [match_mode], [search_mode])`

Devuelve la posición de un valor en una lista, con opciones para elegir cómo se hace la coincidencia y en qué sentido se busca.

- `lookup_value`: El valor que se busca.
- `lookup_array`: El rango de celdas, una fila o una columna, en el que se busca.
- `match_mode` (opcional): 0 o se omite para una coincidencia exacta; -1 para el siguiente menor; 1 para el siguiente mayor; 2 para usar comodines.
- `search_mode` (opcional): 1 o se omite para buscar desde el primero; -1 desde el último; 2 o -2 para una búsqueda binaria en una lista ordenada.
