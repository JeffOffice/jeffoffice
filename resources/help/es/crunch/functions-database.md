# Funciones de base de datos
<!-- keywords: funciones, DPROMEDIO, DCONTAR, DCONTARA, DEXTRAER, DMAX, DMIN, DPRODUCTO, DESVEST, DESVESTP, DSUMA, DVAR, DVARP, DAVERAGE, funciones de base de datos, base de datos, lista, criterios, promedio, suma -->

Estas son las funciones de base de datos que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y cuál es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y «...» indica que puede dar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña ayuda debajo de la celda con sus argumentos; si hace clic en el nombre de la función en esa ayuda, se abre su entrada en este documento.

## DAVERAGE

`DAVERAGE(database, field, criteria)`

Calcula el promedio de los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DCOUNT

`DCOUNT(database, field, criteria)`

Cuenta los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DCOUNTA

`DCOUNTA(database, field, criteria)`

Cuenta las celdas que no están vacías en una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DGET

`DGET(database, field, criteria)`

Devuelve el único valor de una columna de una lista para la única fila que cumple los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DMAX

`DMAX(database, field, criteria)`

Devuelve el número mayor de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DMIN

`DMIN(database, field, criteria)`

Devuelve el número menor de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DPRODUCT

`DPRODUCT(database, field, criteria)`

Multiplica los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DSTDEV

`DSTDEV(database, field, criteria)`

Estima la dispersión de los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DSTDEVP

`DSTDEVP(database, field, criteria)`

Calcula la dispersión de los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DSUM

`DSUM(database, field, criteria)`

Suma los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DVAR

`DVAR(database, field, criteria)`

Estima la varianza de los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".

## DVARP

`DVARP(database, field, criteria)`

Calcula la varianza de los números de una columna de una lista para las filas que cumplen los criterios.

- `database`: El rango de celdas que contiene la lista, con los nombres de sus columnas en la primera fila.
- `field`: La columna que se va a usar: su nombre entre comillas, o su número contando la primera columna como 1.
- `criteria`: Lo que se busca: un número, un texto o una prueba como ">5".
