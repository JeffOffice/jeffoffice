# Funciones estadísticas
<!-- keywords: functions, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Estas son las funciones estadísticas que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y cuál es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y «...» significa que puede indicar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, una pequeña sugerencia debajo de la celda muestra sus argumentos, y si hace clic en el nombre de la función en la sugerencia, se abrirá aquí su entrada.

## AVEDEV

`AVEDEV(número1, ...)`

Devuelve el promedio de las distancias de los números respecto a su media.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## AVERAGE

`AVERAGE(número1, ...)`

Devuelve el promedio de los números.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## AVERAGEA

`AVERAGEA(valor1, ...)`

Devuelve el promedio de los valores, contando el texto como cero y el verdadero como uno.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## AVERAGEIF

`AVERAGEIF(rango, criterio, [rango_promedio])`

Calcula el promedio de las celdas que cumplen una condición.

- `rango`: El rango de celdas que se evalúa.
- `criterio`: Lo que se busca: un número, un texto o una prueba como ">5".
- `rango_promedio` (opcional): Las celdas que se promedian (las celdas evaluadas si se omite).

## AVERAGEIFS

`AVERAGEIFS(rango_promedio, rango_criterios1, criterio1, ...)`

Calcula el promedio de las celdas que cumplen varias condiciones.

- `rango_promedio`: Las celdas que se promedian (las celdas evaluadas si se omite).
- `rango_criterios1`: El primer rango de celdas que se evalúa.
- `criterio1`: Lo que se busca en el primer rango de celdas: un número, un texto o una prueba como ">5".
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## BETA.DIST

`BETA.DIST(x, alfa, beta, acumulado, [A], [B])`

Devuelve el valor de la curva beta en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.
- `A` (opcional): El límite inferior de x (0 si se omite).
- `B` (opcional): El límite superior de x (1 si se omite).

## BETA.INV

`BETA.INV(probabilidad, alfa, beta, [A], [B])`

Devuelve el valor de x para una proporción de una curva beta.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `A` (opcional): El límite inferior de x (0 si se omite).
- `B` (opcional): El límite superior de x (1 si se omite).

## BETADIST

`BETADIST(x, alfa, beta, [A], [B])`

Devuelve la proporción acumulada de la curva beta en x.

- `x`: El valor de x con el que se trabaja.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `A` (opcional): El límite inferior de x (0 si se omite).
- `B` (opcional): El límite superior de x (1 si se omite).

## BETAINV

`BETAINV(probabilidad, alfa, beta, [A], [B])`

Devuelve el valor de x para una proporción de una curva beta.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `A` (opcional): El límite inferior de x (0 si se omite).
- `B` (opcional): El límite superior de x (1 si se omite).

## BINOM.DIST

`BINOM.DIST(num_éxito, ensayos, prob_éxito, acumulado)`

Devuelve la probabilidad de un número de éxitos en un número de ensayos.

- `num_éxito`: El número de éxitos.
- `ensayos`: El número de ensayos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(ensayos, prob_éxito, num_éxito, [num_éxito2])`

Devuelve la probabilidad de que los éxitos caigan en un intervalo.

- `ensayos`: El número de ensayos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.
- `num_éxito`: El número de éxitos.
- `num_éxito2` (opcional): El máximo de éxitos que se cuentan (el mismo que el mínimo si se omite).

## BINOM.INV

`BINOM.INV(ensayos, prob_éxito, alfa)`

Devuelve el menor número de éxitos cuya probabilidad acumulada alcanza un valor.

- `ensayos`: El número de ensayos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.

## BINOMDIST

`BINOMDIST(num_éxito, ensayos, prob_éxito, acumulado)`

Devuelve la probabilidad de un número de éxitos en un número de ensayos.

- `num_éxito`: El número de éxitos.
- `ensayos`: El número de ensayos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## CHIDIST

`CHIDIST(x, grados_libertad)`

Devuelve la proporción de la curva chi cuadrado por encima de x.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## CHIINV

`CHIINV(probabilidad, grados_libertad)`

Devuelve el valor de x para una proporción de la curva chi cuadrado, contada desde la derecha.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## CHISQ.DIST

`CHISQ.DIST(x, grados_libertad, acumulado)`

Devuelve el valor de la curva chi cuadrado en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, grados_libertad)`

Devuelve la proporción de la curva chi cuadrado por encima de x.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## CHISQ.INV

`CHISQ.INV(probabilidad, grados_libertad)`

Devuelve el valor de x para una proporción de la curva chi cuadrado, contada desde la izquierda.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## CHISQ.INV.RT

`CHISQ.INV.RT(probabilidad, grados_libertad)`

Devuelve el valor de x para una proporción de la curva chi cuadrado, contada desde la derecha.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## CHISQ.TEST

`CHISQ.TEST(rango_real, rango_esperado)`

Devuelve la probabilidad de que los recuentos difieran de los recuentos esperados solo por azar.

- `rango_real`: El rango de celdas con los recuentos obtenidos.
- `rango_esperado`: El rango de celdas con los recuentos esperados, del mismo tamaño que los recuentos obtenidos.

## CHITEST

`CHITEST(rango_real, rango_esperado)`

Devuelve la probabilidad de que los recuentos difieran de los recuentos esperados solo por azar.

- `rango_real`: El rango de celdas con los recuentos obtenidos.
- `rango_esperado`: El rango de celdas con los recuentos esperados, del mismo tamaño que los recuentos obtenidos.

## CONFIDENCE

`CONFIDENCE(alfa, desv_estándar, tamaño)`

Devuelve el margen de un intervalo de confianza para una media, usando la curva normal.

- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `desv_estándar`: La desviación estándar de la distribución.
- `tamaño`: El tamaño de la muestra, un número entero de 1 o más.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alfa, desv_estándar, tamaño)`

Devuelve el margen de un intervalo de confianza para una media, usando la curva normal.

- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `desv_estándar`: La desviación estándar de la distribución.
- `tamaño`: El tamaño de la muestra, un número entero de 1 o más.

## CONFIDENCE.T

`CONFIDENCE.T(alfa, desv_estándar, tamaño)`

Devuelve el margen de un intervalo de confianza para una media, usando la curva t de Student.

- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `desv_estándar`: La desviación estándar de la distribución.
- `tamaño`: El tamaño de la muestra, un número entero de 1 o más.

## CORREL

`CORREL(matriz1, matriz2)`

Devuelve lo unidos que se mueven dos conjuntos de números, de -1 a 1.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## COUNT

`COUNT(valor1, ...)`

Cuenta los números de una lista o de un rango de celdas.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## COUNTA

`COUNTA(valor1, ...)`

Cuenta las celdas que no están vacías.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## COUNTBLANK

`COUNTBLANK(rango)`

Cuenta las celdas vacías de un rango.

- `rango`: El rango de celdas que se evalúa.

## COUNTIF

`COUNTIF(rango, criterio)`

Cuenta las celdas que cumplen una condición.

- `rango`: El rango de celdas que se evalúa.
- `criterio`: Lo que se busca: un número, un texto o una prueba como ">5".

## COUNTIFS

`COUNTIFS(rango_criterios1, criterio1, ...)`

Cuenta las filas en las que se cumplen todas las condiciones.

- `rango_criterios1`: El primer rango de celdas que se evalúa.
- `criterio1`: Lo que se busca en el primer rango de celdas: un número, un texto o una prueba como ">5".
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## COVAR

`COVAR(matriz1, matriz2)`

Devuelve cuánto varían juntos dos conjuntos de números.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## COVARIANCE.P

`COVARIANCE.P(matriz1, matriz2)`

Devuelve cuánto varían juntos dos conjuntos de números, para una población.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## COVARIANCE.S

`COVARIANCE.S(matriz1, matriz2)`

Devuelve cuánto varían juntos dos conjuntos de números, para una muestra.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## CRITBINOM

`CRITBINOM(ensayos, prob_éxito, alfa)`

Devuelve el menor número de éxitos cuya probabilidad acumulada alcanza un valor.

- `ensayos`: El número de ensayos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.

## DEVSQ

`DEVSQ(número1, ...)`

Devuelve la suma de los cuadrados de las distancias de los números respecto a su media.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## EXPON.DIST

`EXPON.DIST(x, lambda, acumulado)`

Devuelve el valor de la curva exponencial en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `lambda`: La tasa de la curva, mayor que 0.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## EXPONDIST

`EXPONDIST(x, lambda, acumulado)`

Devuelve el valor de la curva exponencial en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `lambda`: La tasa de la curva, mayor que 0.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## F.DIST

`F.DIST(x, grados_libertad1, grados_libertad2, acumulado)`

Devuelve el valor de la curva F en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad1`: Los grados de libertad del numerador, un número entero de 1 o más.
- `grados_libertad2`: Los grados de libertad del denominador, un número entero de 1 o más.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## F.DIST.RT

`F.DIST.RT(x, grados_libertad1, grados_libertad2)`

Devuelve la proporción de la curva F por encima de x.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad1`: Los grados de libertad del numerador, un número entero de 1 o más.
- `grados_libertad2`: Los grados de libertad del denominador, un número entero de 1 o más.

## F.INV

`F.INV(probabilidad, grados_libertad1, grados_libertad2)`

Devuelve el valor de x para una proporción de la curva F, contada desde la izquierda.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad1`: Los grados de libertad del numerador, un número entero de 1 o más.
- `grados_libertad2`: Los grados de libertad del denominador, un número entero de 1 o más.

## F.INV.RT

`F.INV.RT(probabilidad, grados_libertad1, grados_libertad2)`

Devuelve el valor de x para una proporción de la curva F, contada desde la derecha.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad1`: Los grados de libertad del numerador, un número entero de 1 o más.
- `grados_libertad2`: Los grados de libertad del denominador, un número entero de 1 o más.

## F.TEST

`F.TEST(matriz1, matriz2)`

Devuelve la probabilidad de que dos conjuntos de números tengan la misma varianza.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## FDIST

`FDIST(x, grados_libertad1, grados_libertad2)`

Devuelve la proporción de la curva F por encima de x.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad1`: Los grados de libertad del numerador, un número entero de 1 o más.
- `grados_libertad2`: Los grados de libertad del denominador, un número entero de 1 o más.

## FINV

`FINV(probabilidad, grados_libertad1, grados_libertad2)`

Devuelve el valor de x para una proporción de la curva F, contada desde la derecha.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad1`: Los grados de libertad del numerador, un número entero de 1 o más.
- `grados_libertad2`: Los grados de libertad del denominador, un número entero de 1 o más.

## FISHER

`FISHER(x)`

Devuelve la transformación de Fisher de un número entre -1 y 1.

- `x`: El valor de x con el que se trabaja.

## FISHERINV

`FISHERINV(y)`

Devuelve la inversa de la transformación de Fisher.

- `y`: El número cuya inversa de la transformación de Fisher se busca.

## FORECAST

`FORECAST(x, y_conocidos, x_conocidos)`

Predice un valor a lo largo de la línea recta de mejor ajuste.

- `x`: El valor de x para el que se predice un valor de y.
- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos`: El rango de celdas que contiene los valores de x que ya tiene.

## FORECAST.ETS

`FORECAST.ETS(fecha_destino, valores, cronología, [estacionalidad], [completar_datos], [agregación])`

Predice un valor futuro de una serie que tiene tendencia y estacionalidad, mediante suavizado exponencial.

- `fecha_destino`: El momento para el que se predice un valor.
- `valores`: Los valores que ya tiene.
- `cronología`: Las fechas o números de los valores, separados por la misma distancia.
- `estacionalidad`: 1 para hallar la estacionalidad a partir de los valores, 0 para ninguna, o la longitud de la temporada.
- `completar_datos`: 1 para completar un punto que falta con una línea entre sus vecinos, 0 para completarlo con cero.
- `agregación`: Cómo se combinan los valores en el mismo momento: 1 promedio, 2 contar, 3 contarv, 4 máx, 5 mediana, 6 mín, 7 suma.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(fecha_destino, valores, cronología, [nivel_confianza], [estacionalidad], [completar_datos], [agregación])`

Devuelve cuánto por encima y por debajo de un pronóstico suavizado es probable que esté el valor.

- `nivel_confianza`: Un número mayor que 0 y menor que 1; 0,95 si se omite.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(valores, cronología, [completar_datos], [agregación])`

Devuelve la longitud de la temporada repetitiva que se encuentra en una serie, o 1 si no hay ninguna.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(valores, cronología, tipo_estadístico, [estacionalidad], [completar_datos], [agregación])`

Devuelve una de las constantes de suavizado o medidas de ajuste de un pronóstico suavizado.

- `tipo_estadístico`: 1 Alfa, 2 Beta, 3 Gamma, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 el paso de la cronología.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, y_conocidos, x_conocidos)`

Predice un valor a lo largo de la línea recta de mejor ajuste.

- `x`: El valor de x con el que se trabaja.
- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos`: El rango de celdas que contiene los valores de x que ya tiene.

## FREQUENCY

`FREQUENCY(matriz_datos, matriz_intervalos)`

Cuenta cuántos valores caen en cada intervalo y devuelve los recuentos como una columna.

- `matriz_datos`: El rango de celdas con los valores que se cuentan.
- `matriz_intervalos`: El rango de celdas con los límites de los intervalos en los que se cuenta.

## FTEST

`FTEST(matriz1, matriz2)`

Devuelve la probabilidad de que dos conjuntos de números tengan la misma varianza.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## GAMMA

`GAMMA(x)`

Devuelve la función gamma de un número.

- `x`: El valor de x con el que se trabaja.

## GAMMA.DIST

`GAMMA.DIST(x, alfa, beta, acumulado)`

Devuelve el valor de la curva gamma en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## GAMMA.INV

`GAMMA.INV(probabilidad, alfa, beta)`

Devuelve el valor de x para una proporción de una curva gamma.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).

## GAMMADIST

`GAMMADIST(x, alfa, beta, acumulado)`

Devuelve el valor de la curva gamma en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## GAMMAINV

`GAMMAINV(probabilidad, alfa, beta)`

Devuelve el valor de x para una proporción de una curva gamma.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).

## GAMMALN

`GAMMALN(x)`

Devuelve el logaritmo natural de la función gamma.

- `x`: El valor de x con el que se trabaja.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Devuelve el logaritmo natural de la función gamma.

- `x`: El valor de x con el que se trabaja.

## GAUSS

`GAUSS(z)`

Devuelve la proporción de la curva normal estándar entre la media y z.

- `z`: El valor de z.

## GEOMEAN

`GEOMEAN(número1, ...)`

Devuelve la media geométrica de números positivos.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## GROWTH

`GROWTH(y_conocidos, [x_conocidos], [nuevos_x], [constante])`

Devuelve los valores a lo largo de la curva exponencial de mejor ajuste para nuevos valores de x.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos` (opcional): El rango de celdas que contiene los valores de x que ya tiene.
- `nuevos_x` (opcional): Los valores de x para los que se devuelven los valores de y (los valores de x conocidos si se omite).
- `constante` (opcional): Verdadero o se omite para ajustar una línea que no tiene que pasar por cero; falso para que pase por cero.

## HARMEAN

`HARMEAN(número1, ...)`

Devuelve la media armónica de números positivos.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## HYPGEOM.DIST

`HYPGEOM.DIST(muestra_éxito, núm_muestra, población_éxito, núm_población, acumulado)`

Devuelve la probabilidad de extraer un número de éxitos sin devolver los elementos.

- `muestra_éxito`: El número de éxitos en la muestra.
- `núm_muestra`: El número de elementos extraídos.
- `población_éxito`: El número de éxitos en toda la población.
- `núm_población`: El número de elementos de toda la población.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## HYPGEOMDIST

`HYPGEOMDIST(muestra_éxito, núm_muestra, población_éxito, núm_población)`

Devuelve la probabilidad de extraer un número de éxitos sin devolver los elementos.

- `muestra_éxito`: El número de éxitos en la muestra.
- `núm_muestra`: El número de elementos extraídos.
- `población_éxito`: El número de éxitos en toda la población.
- `núm_población`: El número de elementos de toda la población.

## INTERCEPT

`INTERCEPT(y_conocidos, x_conocidos)`

Devuelve el punto en el que la línea recta de mejor ajuste corta el eje vertical.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos`: El rango de celdas que contiene los valores de x que ya tiene.

## KURT

`KURT(número1, ...)`

Devuelve lo apuntado o aplanado que es un conjunto de números en comparación con una curva normal.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## LARGE

`LARGE(matriz, k)`

Devuelve el k-ésimo mayor número de un conjunto.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `k`: Qué posición devolver, contando desde el mayor: 1 es el mayor.

## LINEST

`LINEST(y_conocidos, [x_conocidos], [constante], [estadísticas])`

Ajusta una línea recta o un plano a los datos por mínimos cuadrados, y devuelve sus valores como una fila.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos` (opcional): El rango de celdas que contiene los valores de x que ya tiene.
- `constante` (opcional): Verdadero o se omite para ajustar una línea que no tiene que pasar por cero; falso para que pase por cero.
- `estadísticas` (opcional): Verdadero para devolver además más valores sobre el ajuste; falso o se omite para devolver solo la línea.

## LOGEST

`LOGEST(y_conocidos, [x_conocidos], [constante], [estadísticas])`

Ajusta una curva exponencial a los datos, y devuelve sus valores como una fila.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos` (opcional): El rango de celdas que contiene los valores de x que ya tiene.
- `constante` (opcional): Verdadero o se omite para ajustar una línea que no tiene que pasar por cero; falso para que pase por cero.
- `estadísticas` (opcional): Verdadero para devolver además más valores sobre el ajuste; falso o se omite para devolver solo la curva.

## LOGINV

`LOGINV(probabilidad, media, desv_estándar)`

Devuelve el valor de x para una proporción de una curva log-normal.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.

## LOGNORM.DIST

`LOGNORM.DIST(x, media, desv_estándar, acumulado)`

Devuelve el valor de la curva log-normal en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## LOGNORM.INV

`LOGNORM.INV(probabilidad, media, desv_estándar)`

Devuelve el valor de x para una proporción de una curva log-normal.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.

## LOGNORMDIST

`LOGNORMDIST(x, media, desv_estándar)`

Devuelve la proporción acumulada de la curva log-normal en x.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.

## MAXA

`MAXA(valor1, ...)`

Devuelve el valor más grande, contando el texto como cero y el verdadero como uno.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MAXIFS

`MAXIFS(rango_máximo, rango_criterios1, criterio1, ...)`

Devuelve el número más grande entre las celdas que cumplen varias condiciones.

- `rango_máximo`: Las celdas de las que se toma el valor mayor.
- `rango_criterios1`: El primer rango de celdas que se evalúa.
- `criterio1`: Lo que se busca en el primer rango de celdas: un número, un texto o una prueba como ">5".
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MEDIAN

`MEDIAN(número1, ...)`

Devuelve el número central de un conjunto.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MINA

`MINA(valor1, ...)`

Devuelve el valor más pequeño, contando el texto como cero y el verdadero como uno.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MINIFS

`MINIFS(rango_mínimo, rango_criterios1, criterio1, ...)`

Devuelve el número más pequeño entre las celdas que cumplen varias condiciones.

- `rango_mínimo`: Las celdas de las que se toma el valor menor.
- `rango_criterios1`: El primer rango de celdas que se evalúa.
- `criterio1`: Lo que se busca en el primer rango de celdas: un número, un texto o una prueba como ">5".
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MODE

`MODE(número1, ...)`

Devuelve el número que aparece con más frecuencia.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MODE.MULT

`MODE.MULT(número1, ...)`

Devuelve una columna con los números que aparecen con más frecuencia en un conjunto.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## MODE.SNGL

`MODE.SNGL(número1, ...)`

Devuelve el número que aparece con más frecuencia en un conjunto.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## NEGBINOM.DIST

`NEGBINOM.DIST(núm_fracaso, núm_éxito, prob_éxito, acumulado)`

Devuelve la probabilidad de un número de fracasos antes de un número de éxitos.

- `núm_fracaso`: El número de fracasos.
- `núm_éxito`: El número de éxitos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## NEGBINOMDIST

`NEGBINOMDIST(núm_fracaso, núm_éxito, prob_éxito)`

Devuelve la probabilidad de un número de fracasos antes de un número de éxitos.

- `núm_fracaso`: El número de fracasos.
- `núm_éxito`: El número de éxitos.
- `prob_éxito`: La probabilidad de éxito en cada ensayo, de 0 a 1.

## NORM.DIST

`NORM.DIST(x, media, desv_estándar, acumulado)`

Devuelve el valor de la curva normal en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## NORM.INV

`NORM.INV(probabilidad, media, desv_estándar)`

Devuelve el valor de x para una proporción de una curva normal.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.

## NORM.S.DIST

`NORM.S.DIST(z, acumulado)`

Devuelve el valor de la curva normal estándar en z, o su proporción acumulada.

- `z`: El valor de z.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## NORM.S.INV

`NORM.S.INV(probabilidad)`

Devuelve el valor de z para una proporción de la curva normal estándar.

- `probabilidad`: Una probabilidad, de 0 a 1.

## NORMDIST

`NORMDIST(x, media, desv_estándar, acumulado)`

Devuelve el valor de la curva normal en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## NORMINV

`NORMINV(probabilidad, media, desv_estándar)`

Devuelve el valor de x para una proporción de una curva normal.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.

## NORMSDIST

`NORMSDIST(z)`

Devuelve la proporción acumulada de la curva normal estándar hasta z.

- `z`: El valor de z.

## NORMSINV

`NORMSINV(probabilidad)`

Devuelve el valor de z para una proporción de la curva normal estándar.

- `probabilidad`: Una probabilidad, de 0 a 1.

## PEARSON

`PEARSON(matriz1, matriz2)`

Devuelve lo unidos que se mueven dos conjuntos de números, de -1 a 1.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.

## PERCENTILE

`PERCENTILE(matriz, k)`

Devuelve el valor por debajo del cual cae una proporción k de los números.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `k`: La proporción de los valores, de 0 a 1, en la que se devuelve el valor.

## PERCENTILE.EXC

`PERCENTILE.EXC(matriz, k)`

Devuelve el valor por debajo del cual cae una proporción k de los números, sin incluir 0 ni 1.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `k`: La posición en el orden, contando desde 1.

## PERCENTILE.INC

`PERCENTILE.INC(matriz, k)`

Devuelve el valor por debajo del cual cae una proporción k de los números, de 0 a 1.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `k`: La posición en el orden, contando desde 1.

## PERCENTRANK

`PERCENTRANK(matriz, x, [cifras_significativas])`

Devuelve la posición de un valor en un conjunto de números como una proporción de 0 a 1.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `x`: El valor de x con el que se trabaja.
- `cifras_significativas` (opcional): El número a cuyos múltiplos se redondea.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(matriz, x, [cifras_significativas])`

Devuelve la posición de un valor en un conjunto de números como una proporción, sin incluir 0 ni 1.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `x`: El valor de x con el que se trabaja.
- `cifras_significativas` (opcional): El número a cuyos múltiplos se redondea.

## PERCENTRANK.INC

`PERCENTRANK.INC(matriz, x, [cifras_significativas])`

Devuelve la posición de un valor en un conjunto de números como una proporción de 0 a 1.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `x`: El valor de x con el que se trabaja.
- `cifras_significativas` (opcional): El número a cuyos múltiplos se redondea.

## PERMUTATIONA

`PERMUTATIONA(número, número_elegido)`

Devuelve el número de opciones ordenadas cuando un elemento puede elegirse más de una vez.

- `número`: El número con el que se opera.
- `número_elegido`: Número de elementos que se eligen en cada grupo.

## PHI

`PHI(x)`

Devuelve la altura de la curva normal estándar en x.

- `x`: El valor de x con el que se trabaja.

## POISSON

`POISSON(x, media, acumulado)`

Devuelve la probabilidad de un número de sucesos, a partir de cuántos se esperan.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## POISSON.DIST

`POISSON.DIST(x, media, acumulado)`

Devuelve la probabilidad de un número de sucesos, a partir de cuántos se esperan.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## PROB

`PROB(rango_x, rango_probabilidad, límite_inferior, [límite_superior])`

Devuelve la probabilidad de que los valores caigan entre dos límites.

- `rango_x`: El rango de celdas con los valores de x.
- `rango_probabilidad`: Las probabilidades de cada valor de x, cada una de 0 a 1, que suman 1.
- `límite_inferior`: El límite inferior del intervalo.
- `límite_superior` (opcional): El límite superior del intervalo (solo el límite inferior si se omite).

## QUARTILE

`QUARTILE(matriz, cuartil)`

Devuelve los valores de los cuartiles de un conjunto de números.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `cuartil`: Qué cuartil: 0 el menor, 1 el primer cuartil, 2 la mediana, 3 el tercer cuartil, 4 el mayor.

## QUARTILE.EXC

`QUARTILE.EXC(matriz, cuartil)`

Devuelve los valores de los cuartiles de un conjunto de números, sin incluir los extremos.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `cuartil`: Qué cuartil: 0 el menor, 1 el primer cuartil, 2 la mediana, 3 el tercer cuartil, 4 el mayor.

## QUARTILE.INC

`QUARTILE.INC(matriz, cuartil)`

Devuelve los valores de los cuartiles de un conjunto de números, desde el menor hasta el mayor.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `cuartil`: Qué cuartil: 0 el menor, 1 el primer cuartil, 2 la mediana, 3 el tercer cuartil, 4 el mayor.

## RANK

`RANK(número, referencia, [orden])`

Devuelve la posición de un número en una lista cuando la lista se ordena.

- `número`: El número con el que se opera.
- `referencia`: Una celda o un rango de celdas.
- `orden` (opcional): 0 o se omite para poner primero el mayor; cualquier otro número pone primero el menor.

## RANK.AVG

`RANK.AVG(número, referencia, [orden])`

Devuelve la posición de un número en una lista, y la posición media cuando hay empates.

- `número`: El número con el que se opera.
- `referencia`: Una celda o un rango de celdas.
- `orden` (opcional): 0 o se omite para poner primero el mayor; cualquier otro número pone primero el menor.

## RANK.EQ

`RANK.EQ(número, referencia, [orden])`

Devuelve la posición de un número en una lista, y la posición más alta cuando hay empates.

- `número`: El número con el que se opera.
- `referencia`: Una celda o un rango de celdas.
- `orden` (opcional): 0 o se omite para poner primero el mayor; cualquier otro número pone primero el menor.

## RSQ

`RSQ(y_conocidos, x_conocidos)`

Devuelve lo bien que una línea recta se ajusta a dos conjuntos de números.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos`: El rango de celdas que contiene los valores de x que ya tiene.

## SKEW

`SKEW(número1, ...)`

Devuelve lo asimétrico que es un conjunto de números alrededor de su media, para una muestra.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SKEW.P

`SKEW.P(número1, ...)`

Devuelve lo asimétrico que es un conjunto de números alrededor de su media, para una población completa.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## SLOPE

`SLOPE(y_conocidos, x_conocidos)`

Devuelve la pendiente de la línea recta que mejor se ajusta a dos conjuntos de números.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos`: El rango de celdas que contiene los valores de x que ya tiene.

## SMALL

`SMALL(matriz, k)`

Devuelve el k-ésimo menor número de un conjunto.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `k`: Qué posición devolver, contando desde el menor: 1 es el menor.

## STANDARDIZE

`STANDARDIZE(x, media, desv_estándar)`

Devuelve a cuántas desviaciones estándar está un valor de la media.

- `x`: El valor de x con el que se trabaja.
- `media`: El promedio de la distribución.
- `desv_estándar`: La desviación estándar de la distribución.

## STDEV

`STDEV(número1, ...)`

Estima la dispersión de un grupo completo a partir de una muestra de él.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## STDEV.P

`STDEV.P(número1, ...)`

Devuelve la dispersión de un grupo completo.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## STDEV.S

`STDEV.S(número1, ...)`

Estima la dispersión de un grupo completo a partir de una muestra de él.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## STDEVA

`STDEVA(valor1, ...)`

Devuelve la desviación estándar de una muestra, contando el texto como 0 y el verdadero como 1.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## STDEVP

`STDEVP(número1, ...)`

Devuelve la dispersión de un grupo completo.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## STDEVPA

`STDEVPA(valor1, ...)`

Devuelve la desviación estándar de una población, contando el texto como 0 y el verdadero como 1.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## STEYX

`STEYX(y_conocidos, x_conocidos)`

Devuelve el error típico de los valores de y previstos de una línea ajustada.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos`: El rango de celdas que contiene los valores de x que ya tiene.

## SUMIF

`SUMIF(rango, criterio, [rango_suma])`

Suma las celdas que cumplen una condición.

- `rango`: El rango de celdas que se evalúa.
- `criterio`: Lo que se busca: un número, un texto o una prueba como ">5".
- `rango_suma` (opcional): Las celdas que se suman (las celdas evaluadas si se omite).

## SUMIFS

`SUMIFS(rango_suma, rango_criterios1, criterio1, ...)`

Suma las celdas que cumplen varias condiciones.

- `rango_suma`: Las celdas que se suman (las celdas evaluadas si se omite).
- `rango_criterios1`: El primer rango de celdas que se evalúa.
- `criterio1`: Lo que se busca en el primer rango de celdas: un número, un texto o una prueba como ">5".
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## T.DIST

`T.DIST(x, grados_libertad, acumulado)`

Devuelve el valor de la curva t de Student en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## T.DIST.2T

`T.DIST.2T(x, grados_libertad)`

Devuelve la proporción de la curva t de Student fuera de -x y x.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## T.DIST.RT

`T.DIST.RT(x, grados_libertad)`

Devuelve la proporción de la curva t de Student por encima de x.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## T.INV

`T.INV(probabilidad, grados_libertad)`

Devuelve el valor de x para una proporción de la curva t de Student, contada desde la izquierda.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## T.INV.2T

`T.INV.2T(probabilidad, grados_libertad)`

Devuelve el valor de x para una proporción de la curva t de Student, repartida entre ambas colas.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## T.TEST

`T.TEST(matriz1, matriz2, colas, tipo)`

Devuelve la probabilidad de que las medias de dos conjuntos de números sean iguales.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.
- `colas`: 1 para una cola de la curva, 2 para ambas.
- `tipo`: Cuándo se realizan los pagos: 0 o se omite al final de cada periodo, 1 al principio.

## TDIST

`TDIST(x, grados_libertad, colas)`

Devuelve la proporción de la curva t de Student en una cola o en ambas.

- `x`: El valor de x con el que se trabaja.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.
- `colas`: 1 para una cola de la curva, 2 para ambas.

## TINV

`TINV(probabilidad, grados_libertad)`

Devuelve el valor de x para una proporción de la curva t de Student, repartida entre ambas colas.

- `probabilidad`: Una probabilidad, de 0 a 1.
- `grados_libertad`: Los grados de libertad, un número entero de 1 o más.

## TREND

`TREND(y_conocidos, [x_conocidos], [nuevos_x], [constante])`

Devuelve los valores a lo largo de la línea recta de mejor ajuste para nuevos valores de x.

- `y_conocidos`: El rango de celdas que contiene los valores de y que ya tiene.
- `x_conocidos` (opcional): El rango de celdas que contiene los valores de x que ya tiene.
- `nuevos_x` (opcional): Los valores de x para los que se devuelven los valores de y (los valores de x conocidos si se omite).
- `constante` (opcional): Verdadero o se omite para ajustar una línea que no tiene que pasar por cero; falso para que pase por cero.

## TRIMMEAN

`TRIMMEAN(matriz, porcentaje)`

Devuelve la media de un conjunto de números después de descartar una proporción de los valores más altos y más bajos.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `porcentaje`: La proporción de los valores, de 0 hasta sin llegar a 1, que se descarta en los dos extremos.

## TTEST

`TTEST(matriz1, matriz2, colas, tipo)`

Devuelve la probabilidad de que las medias de dos conjuntos de números sean iguales.

- `matriz1`: El primer rango de celdas o lista de valores.
- `matriz2`: El segundo rango de celdas o lista de valores, del mismo tamaño que el primero.
- `colas`: 1 para una cola de la curva, 2 para ambas.
- `tipo`: Cuándo se realizan los pagos: 0 o se omite al final de cada periodo, 1 al principio.

## VAR

`VAR(número1, ...)`

Estima la varianza de un grupo completo a partir de una muestra de él.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## VAR.P

`VAR.P(número1, ...)`

Devuelve la varianza de un grupo completo.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## VAR.S

`VAR.S(número1, ...)`

Estima la varianza de un grupo completo a partir de una muestra de él.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## VARA

`VARA(valor1, ...)`

Devuelve la varianza de una muestra, contando el texto como 0 y el verdadero como 1.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## VARP

`VARP(número1, ...)`

Devuelve la varianza de un grupo completo.

- `número1`: El primer número o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## VARPA

`VARPA(valor1, ...)`

Devuelve la varianza de una población, contando el texto como 0 y el verdadero como 1.

- `valor1`: El primer valor o rango de celdas.
- `...`: Tantos más como necesite, del mismo tipo que el anterior.

## WEIBULL

`WEIBULL(x, alfa, beta, acumulado)`

Devuelve el valor de la curva de Weibull en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## WEIBULL.DIST

`WEIBULL.DIST(x, alfa, beta, acumulado)`

Devuelve el valor de la curva de Weibull en x, o su proporción acumulada.

- `x`: El valor de x con el que se trabaja.
- `alfa`: La probabilidad de error que acepta, como 0,05 para un nivel de confianza del 95 por ciento.
- `beta`: Un número que da forma a la curva (la escala de la curva en las curvas gamma y de Weibull).
- `acumulado`: Verdadero para devolver el total acumulado hasta este punto; falso para devolver solo el valor en este punto.

## Z.TEST

`Z.TEST(matriz, x, [sigma])`

Devuelve la probabilidad de una cola de que la media de una muestra esté por encima de un valor.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `x`: El valor de x con el que se trabaja.
- `sigma` (opcional): La desviación estándar de toda la población (la de la muestra si se omite).

## ZTEST

`ZTEST(matriz, x, [sigma])`

Devuelve la probabilidad de una cola de que la media de una muestra esté por encima de un valor.

- `matriz`: Un rango de celdas, o una lista de valores escrita entre llaves.
- `x`: El valor de x con el que se trabaja.
- `sigma` (opcional): La desviación estándar de toda la población (la de la muestra si se omite).
