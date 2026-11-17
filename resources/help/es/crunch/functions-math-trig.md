# Funciones matemáticas y trigonométricas
<!-- keywords: funciones, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC, fórmulas, funciones matemáticas, trigonometría, suma, redondear, potencia, raíz, logaritmo, seno, coseno, tangente, aleatorio, matriz -->

Estas son las funciones matemáticas y trigonométricas que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y cuáles son sus argumentos. Un argumento entre corchetes es opcional, y "..." significa que puede dar tantos más del último como necesite. Para probar una, vea [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece un pequeño consejo debajo de la celda con sus argumentos, y si hace clic en el nombre de la función en el consejo, se abre su entrada en esta página.

## ABS

`ABS(número)`

Devuelve un número sin su signo.

- `número`: El número con el que se opera.

## ACOS

`ACOS(número)`

Devuelve el ángulo, en radianes, que tiene un coseno dado.

- `número`: El número con el que se opera.

## ACOSH

`ACOSH(número)`

Devuelve el coseno hiperbólico inverso de un número.

- `número`: El número con el que se opera.

## ACOT

`ACOT(número)`

Devuelve la cotangente inversa de un número, como ángulo en radianes.

- `número`: El número con el que se opera.

## ACOTH

`ACOTH(número)`

Devuelve la cotangente hiperbólica inversa de un número.

- `número`: El número con el que se opera.

## AGREGAR

`AGREGAR(núm_función, opciones, ref1, ...)`

Realiza un cálculo sobre una lista, como una suma o una mediana, y puede omitir las filas ocultas y los errores.

- `núm_función`: Qué cálculo realizar: 1 promedio, 2 contar, 3 contar no vacías, 4 máximo, 5 mínimo, 6 producto, 7 y 8 desviación estándar, 9 suma, 10 y 11 varianza.
- `opciones`: Qué omitir: 0 totales anidados, 1 también filas ocultas, 2 también errores, 3 ambos, 4 a 7 para conservar los totales anidados.
- `ref1`: La primera celda o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## ARABIC

`ARABIC(texto)`

Lee como número los números romanos que aparecen en un texto.

- `texto`: El texto con el que se opera, o una celda que lo contiene.

## ASEN

`ASEN(número)`

Devuelve el ángulo, en radianes, que tiene un seno dado.

- `número`: El número con el que se opera.

## ASINH

`ASINH(número)`

Devuelve el seno hiperbólico inverso de un número.

- `número`: El número con el que se opera.

## ATAN

`ATAN(número)`

Devuelve el ángulo, en radianes, que tiene una tangente dada.

- `número`: El número con el que se opera.

## ATAN2

`ATAN2(x_núm, y_núm)`

Devuelve el ángulo, en radianes, del punto de coordenadas x e y.

- `x_núm`: El valor de x.
- `y_núm`: El valor de y.

## ATANH

`ATANH(número)`

Devuelve la tangente hiperbólica inversa de un número.

- `número`: El número con el que se opera.

## BASE

`BASE(número, base, [longitud_mín])`

Escribe un número en otra base, de 2 a 36, como texto.

- `número`: El número con el que se opera.
- `base`: La base del número: de 2 a 36.
- `longitud_mín` (opcional): El menor número de caracteres del resultado; los resultados más cortos se completan con ceros a la izquierda.

## CEILING

`CEILING(número, [cifra_significativa])`

Redondea un número hacia arriba al múltiplo más próximo.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.

## CEILING.MATH

`CEILING.MATH(número, [cifra_significativa], [modo])`

Redondea un número hacia arriba al múltiplo más próximo, con una opción para los números negativos.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.
- `modo` (opcional): Cómo se redondean los números negativos: 0 o si se omite es el modo habitual, y cualquier otro número lo invierte.

## CEILING.PRECISE

`CEILING.PRECISE(número, [cifra_significativa])`

Redondea un número hacia arriba al múltiplo más próximo, sea cual sea el signo del múltiplo.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.

## COMBIN

`COMBIN(número, número_elegido)`

Cuenta las formas de elegir elementos de un grupo cuando el orden no importa.

- `número`: El número con el que se opera.
- `número_elegido`: Cuántos elementos se eligen en cada grupo.

## COMBINA

`COMBINA(número, número_elegido)`

Devuelve el número de formas de elegir elementos cuando un elemento puede elegirse más de una vez.

- `número`: El número con el que se opera.
- `número_elegido`: Cuántos elementos se eligen en cada grupo.

## COS

`COS(número)`

Devuelve el coseno de un ángulo en radianes.

- `número`: El número con el que se opera.

## COSH

`COSH(número)`

Devuelve el coseno hiperbólico de un número.

- `número`: El número con el que se opera.

## COT

`COT(ángulo)`

Devuelve la cotangente de un ángulo en radianes.

- `ángulo`: El ángulo, en radianes.

## COTH

`COTH(número)`

Devuelve la cotangente hiperbólica de un número.

- `número`: El número con el que se opera.

## CSC

`CSC(ángulo)`

Devuelve la cosecante de un ángulo en radianes.

- `ángulo`: El ángulo, en radianes.

## CSCH

`CSCH(número)`

Devuelve la cosecante hiperbólica de un número.

- `número`: El número con el que se opera.

## DECIMAL

`DECIMAL(texto, base)`

Lee como número un texto escrito en una base de 2 a 36.

- `texto`: El texto con el que se opera, o una celda que lo contiene.
- `base`: La base del número: de 2 a 36.

## GRADOS

`GRADOS(ángulo)`

Convierte radianes en grados.

- `ángulo`: El ángulo, en radianes.

## EVEN

`EVEN(número)`

Redondea un número hacia arriba al entero par más próximo.

- `número`: El número con el que se opera.

## EXP

`EXP(número)`

Devuelve e elevado a una potencia.

- `número`: El número con el que se opera.

## FACT

`FACT(número)`

Devuelve el factorial de un número.

- `número`: El número con el que se opera.

## FACTDOBLE

`FACTDOBLE(número)`

Devuelve el factorial doble de un número: el producto de cada segundo número hasta llegar a 1 o 2.

- `número`: El número con el que se opera.

## FLOOR

`FLOOR(número, [cifra_significativa])`

Redondea un número hacia abajo al múltiplo más próximo.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.

## FLOOR.MATH

`FLOOR.MATH(número, [cifra_significativa], [modo])`

Redondea un número hacia abajo al múltiplo más próximo, con una opción para los números negativos.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.
- `modo` (opcional): Cómo se redondean los números negativos: 0 o si se omite es el modo habitual, y cualquier otro número lo invierte.

## FLOOR.PRECISE

`FLOOR.PRECISE(número, [cifra_significativa])`

Redondea un número hacia abajo al múltiplo más próximo, sea cual sea el signo del múltiplo.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.

## MCD

`MCD(número1, ...)`

Devuelve el mayor número entero que divide a todos los números.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## INT

`INT(número)`

Redondea un número hacia abajo al entero inferior más próximo.

- `número`: El número con el que se opera.

## ISO.CEILING

`ISO.CEILING(número, [cifra_significativa])`

Redondea un número hacia arriba al múltiplo más próximo, sea cual sea el signo del múltiplo.

- `número`: El número con el que se opera.
- `cifra_significativa` (opcional): El número cuyos múltiplos se usan para redondear.

## MCM

`MCM(número1, ...)`

Devuelve el menor número entero que es divisible por todos los números.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## LN

`LN(número)`

Devuelve el logaritmo natural de un número.

- `número`: El número con el que se opera.

## LOG

`LOG(número, [base])`

Devuelve el logaritmo de un número en una base (10 si no se indica ninguna).

- `número`: El número con el que se opera.
- `base` (opcional): La base del logaritmo (10 si se omite).

## LOG10

`LOG10(número)`

Devuelve el logaritmo en base 10 de un número.

- `número`: El número con el que se opera.

## MAX

`MAX(número1, ...)`

Devuelve el número mayor.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MDETERM

`MDETERM(matriz)`

Devuelve el determinante de un rango cuadrado de números.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.

## MIN

`MIN(número1, ...)`

Devuelve el número menor.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MINVERSA

`MINVERSA(matriz)`

Devuelve la matriz inversa de un rango cuadrado de números.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.

## MMULT

`MMULT(matriz1, matriz2)`

Devuelve el producto de dos rangos de números, como matrices.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## MOD

`MOD(número, divisor)`

Devuelve el resto de dividir un número entre otro.

- `número`: El número con el que se opera.
- `divisor`: El número por el que se divide.

## MREDONDEAR

`MREDONDEAR(número, múltiplo)`

Redondea un número al múltiplo más próximo.

- `número`: El número con el que se opera.
- `múltiplo`: El número cuyos múltiplos se usan para redondear.

## MULTINOMIAL

`MULTINOMIAL(número1, ...)`

Devuelve el factorial de la suma de los números dividido por el producto de sus factoriales.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MUNIT

`MUNIT(dimensión)`

Devuelve la matriz identidad de un tamaño.

- `dimensión`: El tamaño del rango cuadrado: cuántas filas y columnas tiene.

## ODD

`ODD(número)`

Redondea un número hacia arriba al entero impar más próximo.

- `número`: El número con el que se opera.

## PERMUT

`PERMUT(número, número_elegido)`

Cuenta las formas de elegir elementos de un grupo cuando el orden sí importa.

- `número`: El número con el que se opera.
- `número_elegido`: Cuántos elementos se eligen en cada grupo.

## PI

`PI()`

Devuelve el número pi.

## POTENCIA

`POTENCIA(número, potencia)`

Eleva un número a una potencia.

- `número`: El número con el que se opera.
- `potencia`: La potencia a la que se eleva el número.

## PRODUCTO

`PRODUCTO(número1, ...)`

Multiplica números entre sí.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## COCIENTE

`COCIENTE(numerador, denominador)`

Devuelve la parte entera de una división.

- `numerador`: El número que se divide.
- `denominador`: El número por el que se divide.

## RADIANES

`RADIANES(ángulo)`

Convierte grados en radianes.

- `ángulo`: El ángulo, en grados.

## ALEATORIO

`ALEATORIO()`

Devuelve un número aleatorio mayor o igual que 0 y menor que 1.

## ALEATORIO.MATRIZ

`ALEATORIO.MATRIZ([filas], [columnas], [mín], [máx], [número_entero])`

Devuelve un rango de números aleatorios.

- `filas` (opcional): Cuántas filas tiene el resultado.
- `columnas` (opcional): Cuántas columnas tiene el resultado (1 si se omite).
- `mín` (opcional): El menor número que puede tener el resultado (0 si se omite).
- `máx` (opcional): El mayor número que puede tener el resultado (1 si se omite).
- `número_entero` (opcional): Verdadero para números enteros; falso o si se omite, para números con decimales.

## ALEATORIO.ENTRE

`ALEATORIO.ENTRE(inferior, superior)`

Devuelve un número entero aleatorio entre dos números.

- `inferior`: El menor valor que puede tener el resultado.
- `superior`: El mayor valor que puede tener el resultado.

## ROMANO

`ROMANO(número, [forma])`

Escribe un número entero de 0 a 3999 como números romanos en texto.

- `número`: El número con el que se opera.
- `forma` (opcional): Lo corto que es el número romano: 0 o si se omite para la forma clásica, hasta 4 para la más corta.

## REDONDEAR

`REDONDEAR(número, núm_decimales)`

Redondea un número a un número de decimales.

- `número`: El número con el que se opera.
- `núm_decimales`: A cuántos dígitos se redondea: positivo para la derecha de la coma decimal, 0 para un número entero, negativo para la izquierda.

## REDONDEAR.MENOS

`REDONDEAR.MENOS(número, núm_decimales)`

Redondea un número hacia abajo, hacia cero, a un número de dígitos.

- `número`: El número con el que se opera.
- `núm_decimales`: A cuántos dígitos se redondea: positivo para la derecha de la coma decimal, 0 para un número entero, negativo para la izquierda.

## REDONDEAR.MAS

`REDONDEAR.MAS(número, núm_decimales)`

Redondea un número hacia arriba, alejándose de cero, a un número de dígitos.

- `número`: El número con el que se opera.
- `núm_decimales`: A cuántos dígitos se redondea: positivo para la derecha de la coma decimal, 0 para un número entero, negativo para la izquierda.

## SEC

`SEC(ángulo)`

Devuelve la secante de un ángulo en radianes.

- `ángulo`: El ángulo, en radianes.

## SECH

`SECH(número)`

Devuelve la secante hiperbólica de un número.

- `número`: El número con el que se opera.

## SECUENCIA

`SECUENCIA(filas, [columnas], [inicio], [paso])`

Devuelve un rango de números que cuentan desde un inicio, sumando un paso cada vez.

- `filas`: Cuántas filas tiene el resultado; positivo hacia abajo, negativo hacia arriba.
- `columnas` (opcional): Cuántas columnas tiene el resultado (1 si se omite).
- `inicio` (opcional): El primer número (1 si se omite).
- `paso` (opcional): Cuánto se suma en cada número (1 si se omite; en HEX2DEC, el número con el que comparar, 0 si se omite).

## SERIESSUM

`SERIESSUM(x, n, m, coeficientes)`

Devuelve la suma de una serie de potencias.

- `x`: El valor de x con el que trabajar.
- `n`: La primera potencia de x, o el orden de la función, un número entero de 0 o más.
- `m`: El paso que se suma a la potencia en cada coeficiente.
- `coeficientes`: Los números por los que se multiplica cada potencia de x.

## SIGNO

`SIGNO(número)`

Devuelve 1 para un número positivo, 0 para cero y -1 para uno negativo.

- `número`: El número con el que se opera.

## SEN

`SEN(número)`

Devuelve el seno de un ángulo en radianes.

- `número`: El número con el que se opera.

## SENH

`SENH(número)`

Devuelve el seno hiperbólico de un número.

- `número`: El número con el que se opera.

## RAIZ

`RAIZ(número)`

Devuelve la raíz cuadrada de un número.

- `número`: El número con el que se opera.

## RAIZPI

`RAIZPI(número)`

Devuelve la raíz cuadrada de un número multiplicado por pi.

- `número`: El número con el que se opera.

## SUBTOTALES

`SUBTOTALES(núm_función, ref1, ...)`

Devuelve un total, un promedio, un recuento u otra cifra para un rango de celdas.

- `núm_función`: Qué cálculo realizar: 1 promedio, 2 contar, 3 contar no vacías, 4 máximo, 5 mínimo, 6 producto, 7 y 8 desviación estándar, 9 suma, 10 y 11 varianza.
- `ref1`: La primera celda o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SUMA

`SUMA(número1, ...)`

Suma números y los números de los rangos de celdas.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SUMAPRODUCTO

`SUMAPRODUCTO(matriz1, ...)`

Multiplica los números de rangos del mismo tamaño, posición por posición, y suma los productos.

- `matriz1`: El primer rango de celdas o lista de valores.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SUMACUADRADOS

`SUMACUADRADOS(número1, ...)`

Suma los cuadrados de números.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SUMAX2MENOSY2

`SUMAX2MENOSY2(matriz_x, matriz_y)`

Suma las diferencias de los cuadrados de dos conjuntos de números.

- `matriz_x`: El primer rango de celdas o lista de valores.
- `matriz_y`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## SUMAX2MÁSY2

`SUMAX2MÁSY2(matriz_x, matriz_y)`

Suma las sumas de los cuadrados de dos conjuntos de números.

- `matriz_x`: El primer rango de celdas o lista de valores.
- `matriz_y`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## SUMAXMENOSY2

`SUMAXMENOSY2(matriz_x, matriz_y)`

Suma los cuadrados de las diferencias de dos conjuntos de números.

- `matriz_x`: El primer rango de celdas o lista de valores.
- `matriz_y`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## TAN

`TAN(número)`

Devuelve la tangente de un ángulo en radianes.

- `número`: El número con el que se opera.

## TANH

`TANH(número)`

Devuelve la tangente hiperbólica de un número.

- `número`: El número con el que se opera.

## TRUNC

`TRUNC(número, [núm_decimales])`

Corta un número después de un número de dígitos, sin redondear.

- `número`: El número con el que se opera.
- `núm_decimales` (opcional): A cuántos dígitos se trunca: positivo para la derecha de la coma decimal, 0 para un número entero, negativo para la izquierda.
