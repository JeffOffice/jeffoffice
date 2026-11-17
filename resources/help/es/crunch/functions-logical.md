# Funciones lógicas
<!-- keywords: funciones, Y, ELEGIR, FALSO, SI, SI.ERROR, SI.N.D, SI.CONJUNTO, LET, NO, O, CAMBIAR, VERDADERO, O.EXCLUSIVO -->

Estas son las funciones lógicas que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y cuál es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y «...» significa que puede añadir todos los siguientes que necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña sugerencia debajo de la celda con sus argumentos; si hace clic en el nombre de la función en la sugerencia, se abre su entrada en esta página.

## AND

`AND(logical1, ...)`

Es verdadero cuando todas las condiciones son verdaderas.

- `logical1`: El primer valor o prueba, que es verdadero o falso.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## CHOOSE

`CHOOSE(index, value1, ...)`

Toma el valor de una lista que está en la posición que indica el índice.

- `index`: Qué valor devolver, contando el primero como 1.
- `value1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## FALSE

`FALSE()`

El valor lógico falso.

## IF

`IF(condition, value_if_true, [value_if_false])`

Devuelve un valor cuando una condición se cumple y otro cuando no se cumple.

- `condition`: Un valor o prueba que es verdadero o falso.
- `value_if_true`: Lo que se devuelve si la condición es verdadera.
- `value_if_false` (opcional): Lo que se devuelve si la condición es falsa (FALSE si se omite).

## IFERROR

`IFERROR(value, value_if_error)`

Devuelve un valor o, si ese valor es un error, un valor de sustitución.

- `value`: El valor con el que se trabaja, o una celda que lo contiene.
- `value_if_error`: Lo que se devuelve si el valor es un error.

## IFNA

`IFNA(value, value_if_na)`

Devuelve un valor o, si ese valor es el error de valor no disponible, un valor de sustitución.

- `value`: El valor con el que se trabaja, o una celda que lo contiene.
- `value_if_na`: Lo que se devuelve si el valor es el error de valor no disponible.

## IFS

`IFS(condition1, value1, ...)`

Evalúa las condiciones una a una y devuelve el valor que corresponde a la primera que se cumple.

- `condition1`: La primera prueba, que es verdadera o falsa.
- `value1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## LET

`LET(name1, value1, ...)`

Asigna nombres a valores dentro de una misma fórmula y luego calcula una operación que los usa.

- `name1`: Un nombre que se asigna al valor que sigue. Puede usarse en los valores posteriores y en el cálculo.
- `value1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## NOT

`NOT(logical)`

Convierte verdadero en falso y falso en verdadero.

- `logical`: Un valor o prueba que es verdadero o falso.

## OR

`OR(logical1, ...)`

Es verdadero cuando al menos una de las condiciones es verdadera.

- `logical1`: El primer valor o prueba, que es verdadero o falso.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SWITCH

`SWITCH(expression, value1, result1, ...)`

Compara un valor con una lista de valores y devuelve el resultado que corresponde a la primera coincidencia.

- `expression`: El valor que se compara con los valores que siguen.
- `value1`: El primer valor o rango de celdas.
- `result1`: El resultado que se devuelve cuando el valor anterior coincide.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## TRUE

`TRUE()`

El valor lógico verdadero.

## XOR

`XOR(logical1, ...)`

Es verdadero cuando un número impar de las condiciones son verdaderas.

- `logical1`: El primer valor o prueba, que es verdadero o falso.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.
