# Funciones de ingeniería
<!-- keywords: funciones, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX, fórmulas, funciones de ingeniería, números complejos, binario, hexadecimal, octal, conversión -->

Estas son las funciones de ingeniería que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y cuál es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y «...» significa que puede dar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña sugerencia debajo de la celda con sus argumentos, y si hace clic en el nombre de la función en la sugerencia, se abre su entrada en esta página.

## BESSELI

`BESSELI(x, n)`

Devuelve la función de Bessel modificada de primera especie.

- `x`: El valor x con el que trabajar.
- `n`: La primera potencia de x, o el orden de la función, un número entero igual o mayor que 0.

## BESSELJ

`BESSELJ(x, n)`

Devuelve la función de Bessel de primera especie.

- `x`: El valor x con el que trabajar.
- `n`: La primera potencia de x, o el orden de la función, un número entero igual o mayor que 0.

## BESSELK

`BESSELK(x, n)`

Devuelve la función de Bessel modificada de segunda especie.

- `x`: El valor x con el que trabajar.
- `n`: La primera potencia de x, o el orden de la función, un número entero igual o mayor que 0.

## BESSELY

`BESSELY(x, n)`

Devuelve la función de Bessel de segunda especie.

- `x`: El valor x con el que trabajar.
- `n`: La primera potencia de x, o el orden de la función, un número entero igual o mayor que 0.

## BIN2DEC

`BIN2DEC(número)`

Convierte un número binario en un número decimal.

- `número`: El número con el que se opera.

## BIN2HEX

`BIN2HEX(número, [posiciones])`

Convierte un número binario en un número hexadecimal.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## BIN2OCT

`BIN2OCT(número, [posiciones])`

Convierte un número binario en un número octal.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## BITAND

`BITAND(número1, número2)`

Devuelve los bits que están activados en ambos de dos números enteros.

- `número1`: El primer número, o rango de celdas.
- `número2`: El segundo número.

## BITLSHIFT

`BITLSHIFT(número, cantidad_desplazamiento)`

Desplaza los bits de un número entero hacia la izquierda.

- `número`: El número con el que se opera.
- `cantidad_desplazamiento`: Cuántos bits se desplazan: los números positivos y negativos van en sentidos opuestos.

## BITOR

`BITOR(número1, número2)`

Devuelve los bits que están activados en cualquiera de dos números enteros.

- `número1`: El primer número, o rango de celdas.
- `número2`: El segundo número.

## BITRSHIFT

`BITRSHIFT(número, cantidad_desplazamiento)`

Desplaza los bits de un número entero hacia la derecha.

- `número`: El número con el que se opera.
- `cantidad_desplazamiento`: Cuántos bits se desplazan: los números positivos y negativos van en sentidos opuestos.

## BITXOR

`BITXOR(número1, número2)`

Devuelve los bits que están activados en uno de dos números enteros, pero no en ambos.

- `número1`: El primer número, o rango de celdas.
- `número2`: El segundo número.

## COMPLEX

`COMPLEX(núm_real, núm_imaginario, [sufijo])`

Crea un número complejo a partir de sus partes real e imaginaria.

- `núm_real`: La parte real del número complejo.
- `núm_imaginario`: La parte imaginaria del número complejo.
- `sufijo` (opcional): i o j, para marcar la parte imaginaria (i si se omite).

## CONVERT

`CONVERT(número, unidad_origen, unidad_destino)`

Cambia un número de una unidad de medida a otra.

- `número`: El número con el que se opera.
- `unidad_origen`: La unidad en la que está el número, como texto, por ejemplo «mi» o «lbm».
- `unidad_destino`: La unidad a la que se cambia, como texto, por ejemplo «km» o «kg».

## DEC2BIN

`DEC2BIN(número, [posiciones])`

Convierte un número decimal en un número binario.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## DEC2HEX

`DEC2HEX(número, [posiciones])`

Convierte un número decimal en un número hexadecimal.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## DEC2OCT

`DEC2OCT(número, [posiciones])`

Convierte un número decimal en un número octal.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## DELTA

`DELTA(número1, [número2])`

Indica si dos números son iguales, con 1 para sí y 0 para no.

- `número1`: El primer número, o rango de celdas.
- `número2` (opcional): El segundo número.

## ERF

`ERF(límite_inferior, [límite_superior])`

Devuelve la función de error entre dos límites o desde 0.

- `límite_inferior`: El límite inferior del intervalo.
- `límite_superior` (opcional): El límite superior del intervalo (si se omite, se usa el límite inferior).

## ERF.PRECISE

`ERF.PRECISE(x)`

Devuelve la función de error desde 0 hasta un límite.

- `x`: El valor x con el que trabajar.

## ERFC

`ERFC(x)`

Devuelve la función de error complementaria desde un límite hasta el infinito.

- `x`: El valor x con el que trabajar.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Devuelve la función de error complementaria desde un límite hasta el infinito.

- `x`: El valor x con el que trabajar.

## GESTEP

`GESTEP(número, [paso])`

Indica si un número es al menos igual a un paso, con 1 para sí y 0 para no.

- `número`: El número con el que se opera.
- `paso` (opcional): Cuánto se suma por cada número (1 si se omite; en GESTEP, el número con el que se compara, 0 si se omite).

## HEX2BIN

`HEX2BIN(número, [posiciones])`

Convierte un número hexadecimal en un número binario.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## HEX2DEC

`HEX2DEC(número)`

Convierte un número hexadecimal en un número decimal.

- `número`: El número con el que se opera.

## HEX2OCT

`HEX2OCT(número, [posiciones])`

Convierte un número hexadecimal en un número octal.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## IMABS

`IMABS(núm_complejo)`

Devuelve el módulo de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMAGINARY

`IMAGINARY(núm_complejo)`

Devuelve la parte imaginaria de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMARGUMENT

`IMARGUMENT(núm_complejo)`

Devuelve el ángulo de un número complejo, en radianes.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMCONJUGATE

`IMCONJUGATE(núm_complejo)`

Devuelve el conjugado de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMCOS

`IMCOS(núm_complejo)`

Devuelve el coseno de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMCOSH

`IMCOSH(núm_complejo)`

Devuelve el coseno hiperbólico de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMCOT

`IMCOT(núm_complejo)`

Devuelve la cotangente de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMCSC

`IMCSC(núm_complejo)`

Devuelve la cosecante de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMCSCH

`IMCSCH(núm_complejo)`

Devuelve la cosecante hiperbólica de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMDIV

`IMDIV(núm_complejo1, núm_complejo2)`

Devuelve el cociente de dos números complejos.

- `núm_complejo1`: El primer número complejo, escrito como texto, por ejemplo «3+4i».
- `núm_complejo2`: El segundo número complejo, escrito como texto.

## IMEXP

`IMEXP(núm_complejo)`

Devuelve e elevado a la potencia de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMLN

`IMLN(núm_complejo)`

Devuelve el logaritmo natural de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMLOG10

`IMLOG10(núm_complejo)`

Devuelve el logaritmo en base 10 de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMLOG2

`IMLOG2(núm_complejo)`

Devuelve el logaritmo en base 2 de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMPOWER

`IMPOWER(núm_complejo, número)`

Devuelve un número complejo elevado a una potencia.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».
- `número`: El número con el que se opera.

## IMPRODUCT

`IMPRODUCT(núm_complejo1, ...)`

Devuelve el producto de números complejos.

- `núm_complejo1`: El primer número complejo, escrito como texto, por ejemplo «3+4i».
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## IMREAL

`IMREAL(núm_complejo)`

Devuelve la parte real de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMSEC

`IMSEC(núm_complejo)`

Devuelve la secante de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMSECH

`IMSECH(núm_complejo)`

Devuelve la secante hiperbólica de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMSIN

`IMSIN(núm_complejo)`

Devuelve el seno de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMSINH

`IMSINH(núm_complejo)`

Devuelve el seno hiperbólico de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMSQRT

`IMSQRT(núm_complejo)`

Devuelve la raíz cuadrada de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## IMSUB

`IMSUB(núm_complejo1, núm_complejo2)`

Devuelve la diferencia de dos números complejos.

- `núm_complejo1`: El primer número complejo, escrito como texto, por ejemplo «3+4i».
- `núm_complejo2`: El segundo número complejo, escrito como texto.

## IMSUM

`IMSUM(núm_complejo1, ...)`

Devuelve la suma de números complejos.

- `núm_complejo1`: El primer número complejo, escrito como texto, por ejemplo «3+4i».
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## IMTAN

`IMTAN(núm_complejo)`

Devuelve la tangente de un número complejo.

- `núm_complejo`: Un número complejo escrito como texto, por ejemplo «3+4i».

## OCT2BIN

`OCT2BIN(número, [posiciones])`

Convierte un número octal en un número binario.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.

## OCT2DEC

`OCT2DEC(número)`

Convierte un número octal en un número decimal.

- `número`: El número con el que se opera.

## OCT2HEX

`OCT2HEX(número, [posiciones])`

Convierte un número octal en un número hexadecimal.

- `número`: El número con el que se opera.
- `posiciones` (opcional): Cuántos caracteres ocupará el resultado, completándolos con ceros a la izquierda.
