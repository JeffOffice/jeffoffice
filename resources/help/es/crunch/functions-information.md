# Funciones de información
<!-- keywords: funciones, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE, funciones de información, información, comprobar celda, ISERROR, tipo de valor, hojas, celda vacía -->

Estas son las funciones de información que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y qué es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y "..." significa que puede dar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña sugerencia debajo de la celda con sus argumentos; si hace clic en el nombre de la función en la sugerencia, se abre su entrada en esta página.

## CELL

`CELL(tipo_info, [referencia])`

Devuelve información sobre una celda, como su dirección o su formato.

- `tipo_info`: Qué información se quiere obtener, como texto: "address" (dirección), "row" (fila), "col" (columna) o "format" (formato).
- `referencia` (opcional): La celda sobre la que se quiere información (la celda con el cursor si se omite).

## ERROR.TYPE

`ERROR.TYPE(valor_error)`

Devuelve el número que representa un tipo de error.

- `valor_error`: El error que se quiere consultar, o una celda que lo contiene.

## INFO

`INFO(tipo_info)`

Devuelve información sobre el programa o el libro, como su carpeta o cómo calcula.

- `tipo_info`: Qué información se quiere obtener, como texto: "address" (dirección), "row" (fila), "col" (columna) o "format" (formato).

## ISBLANK

`ISBLANK(valor)`

Es verdadero cuando la celda está vacía.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISERR

`ISERR(valor)`

Es verdadero cuando el valor es un error distinto de valor no disponible.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISERROR

`ISERROR(valor)`

Es verdadero cuando el valor es cualquier error.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISEVEN

`ISEVEN(número)`

Es verdadero cuando un número entero es par.

- `número`: El número con el que trabajar.

## ISFORMULA

`ISFORMULA(referencia)`

Es verdadero cuando la celda contiene una fórmula.

- `referencia`: Una celda o un rango de celdas.

## ISLOGICAL

`ISLOGICAL(valor)`

Es verdadero cuando el valor es verdadero o falso.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISNA

`ISNA(valor)`

Es verdadero cuando el valor es el error de valor no disponible.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISNONTEXT

`ISNONTEXT(valor)`

Es verdadero cuando el valor es cualquier cosa excepto texto.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISNUMBER

`ISNUMBER(valor)`

Es verdadero cuando el valor es un número.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISODD

`ISODD(número)`

Es verdadero cuando un número entero es impar.

- `número`: El número con el que trabajar.

## ISREF

`ISREF(valor)`

Es verdadero cuando el valor es una referencia a celdas.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## ISTEXT

`ISTEXT(valor)`

Es verdadero cuando el valor es texto.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## N

`N(valor)`

Convierte un valor en un número: las fechas y los valores verdadero se convierten en números, y el texto, en cero.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.

## NA

`NA()`

Devuelve el error de valor no disponible, para marcar un valor que falta.

## SHEET

`SHEET([valor])`

Devuelve el número de una hoja, contando la primera hoja como 1.

- `valor` (opcional): El valor con el que trabajar, o una celda que lo contiene.

## SHEETS

`SHEETS([referencia])`

Devuelve el número de hojas de una referencia, o del libro.

- `referencia` (opcional): Una celda o un rango de celdas.

## TYPE

`TYPE(valor)`

Devuelve un número que indica qué tipo de valor es: número, texto, lógico, error o matriz.

- `valor`: El valor con el que trabajar, o una celda que lo contiene.
