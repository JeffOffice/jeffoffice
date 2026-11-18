# Funções estatísticas
<!-- keywords: funções, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Estas são as funções estatísticas que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos outros quantos precisar, do último tipo. Para experimentar uma destas funções, veja [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece por baixo da célula uma pequena sugestão com os respetivos argumentos; se clicar no nome da função nessa sugestão, abre-se aqui a respetiva entrada.

## AVEDEV

`AVEDEV(number1, ...)`

Devolve a média das distâncias dos números em relação à sua média.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## AVERAGE

`AVERAGE(number1, ...)`

Devolve a média dos números.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## AVERAGEA

`AVERAGEA(value1, ...)`

Devolve a média dos valores, contando o texto como zero e o verdadeiro como um.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Calcula a média das células que cumprem uma condição.

- `range`: O bloco de células a testar.
- `criteria`: O que procurar: um número, um texto ou um teste, como ">5".
- `average_range` (opcional): As células a calcular a média (as células a testar, se for omitido).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Calcula a média das células que cumprem várias condições.

- `average_range`: As células a calcular a média (as células a testar, se for omitido).
- `criteria_range1`: O primeiro bloco de células a testar.
- `criteria1`: O que procurar no primeiro bloco de células: um número, um texto ou um teste, como ">5".
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Devolve o valor ou a percentagem acumulada da curva beta em x.

- `x`: O valor de x a utilizar.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.
- `A` (opcional): O limite inferior de x (0 se for omitido).
- `B` (opcional): O limite superior de x (1 se for omitido).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Devolve o valor de x correspondente a uma percentagem de uma curva beta.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0 se for omitido).
- `B` (opcional): O limite superior de x (1 se for omitido).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Devolve a percentagem acumulada da curva beta em x.

- `x`: O valor de x a utilizar.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0 se for omitido).
- `B` (opcional): O limite superior de x (1 se for omitido).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Devolve o valor de x correspondente a uma percentagem de uma curva beta.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0 se for omitido).
- `B` (opcional): O limite superior de x (1 se for omitido).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Devolve a probabilidade de um número de sucessos num número de tentativas.

- `number_s`: O número de sucessos.
- `trials`: O número de tentativas.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Devolve a probabilidade de os sucessos caírem num intervalo.

- `trials`: O número de tentativas.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.
- `number_s`: O número de sucessos.
- `number_s2` (opcional): O número máximo de sucessos a contar (igual ao mínimo se for omitido).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Devolve o menor número de sucessos cuja probabilidade acumulada atinge um valor.

- `trials`: O número de tentativas.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Devolve a probabilidade de um número de sucessos num número de tentativas.

- `number_s`: O número de sucessos.
- `trials`: O número de tentativas.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Devolve a percentagem da curva qui-quadrado acima de x.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## CHIINV

`CHIINV(probability, deg_freedom)`

Devolve o valor de x correspondente a uma percentagem da curva qui-quadrado, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Devolve o valor ou a percentagem acumulada da curva qui-quadrado em x.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Devolve a percentagem da curva qui-quadrado acima de x.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Devolve o valor de x correspondente a uma percentagem da curva qui-quadrado, contada a partir da esquerda.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Devolve o valor de x correspondente a uma percentagem da curva qui-quadrado, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Devolve a probabilidade de as contagens diferirem das contagens esperadas apenas por acaso.

- `actual_range`: O bloco de células com as contagens obtidas.
- `expected_range`: O bloco de células com as contagens esperadas, do mesmo tamanho das contagens obtidas.

## CHITEST

`CHITEST(actual_range, expected_range)`

Devolve a probabilidade de as contagens diferirem das contagens esperadas apenas por acaso.

- `actual_range`: O bloco de células com as contagens obtidas.
- `expected_range`: O bloco de células com as contagens esperadas, do mesmo tamanho das contagens obtidas.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Devolve a margem de um intervalo de confiança para uma média, usando a curva normal.

- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `standard_dev`: O desvio-padrão da distribuição.
- `size`: A dimensão da amostra, um número inteiro igual ou superior a 1.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Devolve a margem de um intervalo de confiança para uma média, usando a curva normal.

- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `standard_dev`: O desvio-padrão da distribuição.
- `size`: A dimensão da amostra, um número inteiro igual ou superior a 1.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Devolve a margem de um intervalo de confiança para uma média, usando a curva t de Student.

- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `standard_dev`: O desvio-padrão da distribuição.
- `size`: A dimensão da amostra, um número inteiro igual ou superior a 1.

## CORREL

`CORREL(array1, array2)`

Devolve o grau em que dois conjuntos de números variam em conjunto, de -1 a 1.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## COUNT

`COUNT(value1, ...)`

Conta os números numa lista ou num bloco de células.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## COUNTA

`COUNTA(value1, ...)`

Conta as células que não estão vazias.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## COUNTBLANK

`COUNTBLANK(range)`

Conta as células vazias num bloco.

- `range`: O bloco de células a testar.

## COUNTIF

`COUNTIF(range, criteria)`

Conta as células que cumprem uma condição.

- `range`: O bloco de células a testar.
- `criteria`: O que procurar: um número, um texto ou um teste, como ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Conta as linhas em que todas as condições se verificam.

- `criteria_range1`: O primeiro bloco de células a testar.
- `criteria1`: O que procurar no primeiro bloco de células: um número, um texto ou um teste, como ">5".
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## COVAR

`COVAR(array1, array2)`

Devolve o grau em que dois conjuntos de números variam em conjunto.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Devolve o grau em que dois conjuntos de números variam em conjunto, para uma população.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Devolve o grau em que dois conjuntos de números variam em conjunto, para uma amostra.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Devolve o menor número de sucessos cuja probabilidade acumulada atinge um valor.

- `trials`: O número de tentativas.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.

## DEVSQ

`DEVSQ(number1, ...)`

Devolve a soma dos quadrados das distâncias dos números em relação à sua média.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Devolve o valor ou a percentagem acumulada da curva exponencial em x.

- `x`: O valor de x a utilizar.
- `lambda`: A taxa da curva, superior a 0.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Devolve o valor ou a percentagem acumulada da curva exponencial em x.

- `x`: O valor de x a utilizar.
- `lambda`: A taxa da curva, superior a 0.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Devolve o valor ou a percentagem acumulada da curva F em x.

- `x`: O valor de x a utilizar.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro igual ou superior a 1.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro igual ou superior a 1.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Devolve a percentagem da curva F acima de x.

- `x`: O valor de x a utilizar.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro igual ou superior a 1.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro igual ou superior a 1.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Devolve o valor de x correspondente a uma percentagem da curva F, contada a partir da esquerda.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro igual ou superior a 1.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro igual ou superior a 1.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Devolve o valor de x correspondente a uma percentagem da curva F, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro igual ou superior a 1.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro igual ou superior a 1.

## F.TEST

`F.TEST(array1, array2)`

Devolve a probabilidade de dois conjuntos de números terem a mesma variância.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Devolve a percentagem da curva F acima de x.

- `x`: O valor de x a utilizar.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro igual ou superior a 1.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro igual ou superior a 1.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Devolve o valor de x correspondente a uma percentagem da curva F, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro igual ou superior a 1.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro igual ou superior a 1.

## FISHER

`FISHER(x)`

Devolve a transformação de Fisher de um número entre -1 e 1.

- `x`: O valor de x a utilizar.

## FISHERINV

`FISHERINV(y)`

Devolve a inversa da transformação de Fisher.

- `y`: O número cuja inversa da transformação de Fisher pretende obter.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Prevê um valor ao longo da reta que melhor se ajusta.

- `x`: O valor de x para o qual prever um valor de y.
- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's`: O bloco de células que contém os valores de x que já tem.

## FORECAST.ETS

`FORECAST.ETS(target_date, values, timeline, [seasonality], [data_completion], [aggregation])`

Prevê um valor futuro de uma série que tem uma tendência e uma época, por alisamento exponencial.

- `target_date`: O momento para o qual prever um valor.
- `values`: Os valores que já tem.
- `timeline`: As datas ou números dos valores, espaçados de forma igual.
- `seasonality`: 1 para determinar a época a partir dos valores, 0 para nenhuma, ou o comprimento da época.
- `data_completion`: 1 para preencher um ponto em falta com uma reta entre os vizinhos, 0 para o preencher com zero.
- `aggregation`: Como se combinam os valores no mesmo momento: 1 média, 2 contagem, 3 counta, 4 máximo, 5 mediana, 6 mínimo, 7 soma.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(target_date, values, timeline, [confidence_level], [seasonality], [data_completion], [aggregation])`

Devolve a distância acima e abaixo de uma previsão suavizada a que o valor provavelmente estará.

- `confidence_level`: Um número superior a 0 e inferior a 1; 0,95 se for omitido.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(values, timeline, [data_completion], [aggregation])`

Devolve o comprimento da época que se repete numa série, ou 1 se não existir nenhuma.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(values, timeline, statistic_type, [seasonality], [data_completion], [aggregation])`

Devolve uma das constantes de alisamento ou medidas de ajuste de uma previsão suavizada.

- `statistic_type`: 1 Alfa, 2 Beta, 3 Gama, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 o passo da linha temporal.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Prevê um valor ao longo da reta que melhor se ajusta.

- `x`: O valor de x a utilizar.
- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's`: O bloco de células que contém os valores de x que já tem.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Conta quantos valores caem em cada intervalo e devolve as contagens como uma coluna.

- `data_array`: O bloco de células com os valores a contar.
- `bins_array`: O bloco de células com os limites dos intervalos em que contar.

## FTEST

`FTEST(array1, array2)`

Devolve a probabilidade de dois conjuntos de números terem a mesma variância.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## GAMMA

`GAMMA(x)`

Devolve a função gama de um número.

- `x`: O valor de x a utilizar.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Devolve o valor ou a percentagem acumulada da curva gama em x.

- `x`: O valor de x a utilizar.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Devolve o valor de x correspondente a uma percentagem de uma curva gama.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Devolve o valor ou a percentagem acumulada da curva gama em x.

- `x`: O valor de x a utilizar.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Devolve o valor de x correspondente a uma percentagem de uma curva gama.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).

## GAMMALN

`GAMMALN(x)`

Devolve o logaritmo natural da função gama.

- `x`: O valor de x a utilizar.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Devolve o logaritmo natural da função gama.

- `x`: O valor de x a utilizar.

## GAUSS

`GAUSS(z)`

Devolve a percentagem da curva normal padrão entre a média e z.

- `z`: O valor de z.

## GEOMEAN

`GEOMEAN(number1, ...)`

Devolve a média geométrica de números positivos.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Devolve os valores ao longo da curva exponencial que melhor se ajusta, para novos valores de x.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's` (opcional): O bloco de células que contém os valores de x que já tem.
- `new_x's` (opcional): Os valores de x para os quais devolver valores de y (os valores de x conhecidos, se for omitido).
- `const` (opcional): Verdadeiro ou omitido para ajustar uma curva que não tem de passar por zero; falso para que passe por zero.

## HARMEAN

`HARMEAN(number1, ...)`

Devolve a média harmónica de números positivos.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Devolve a probabilidade de extrair um número de sucessos sem repor os itens.

- `sample_s`: O número de sucessos na amostra.
- `number_sample`: O número de itens extraídos.
- `population_s`: O número de sucessos em toda a população.
- `number_pop`: O número de itens em toda a população.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Devolve a probabilidade de extrair um número de sucessos sem repor os itens.

- `sample_s`: O número de sucessos na amostra.
- `number_sample`: O número de itens extraídos.
- `population_s`: O número de sucessos em toda a população.
- `number_pop`: O número de itens em toda a população.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Devolve o ponto em que a reta que melhor se ajusta cruza o eixo vertical.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's`: O bloco de células que contém os valores de x que já tem.

## KURT

`KURT(number1, ...)`

Devolve o grau de achatamento ou de pico de um conjunto de números, em comparação com uma curva normal.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## LARGE

`LARGE(array, k)`

Devolve o k-ésimo maior número de um conjunto.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `k`: Que posição devolver, contando a partir do maior: 1 é o maior.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Ajusta uma reta ou um plano aos dados pelo método dos mínimos quadrados, e devolve os seus números como uma linha.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's` (opcional): O bloco de células que contém os valores de x que já tem.
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não tem de passar por zero; falso para que passe por zero.
- `stats` (opcional): Verdadeiro para devolver também mais números sobre o ajuste; falso ou omitido para devolver apenas a reta.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Ajusta uma curva exponencial aos dados, e devolve os seus números como uma linha.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's` (opcional): O bloco de células que contém os valores de x que já tem.
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não tem de passar por zero; falso para que passe por zero.
- `stats` (opcional): Verdadeiro para devolver também mais números sobre o ajuste; falso ou omitido para devolver apenas a curva.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Devolve o valor de x correspondente a uma percentagem de uma curva log-normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Devolve o valor ou a percentagem acumulada da curva log-normal em x.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Devolve o valor de x correspondente a uma percentagem de uma curva log-normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Devolve a percentagem acumulada da curva log-normal em x.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.

## MAXA

`MAXA(value1, ...)`

Devolve o maior valor, contando o texto como zero e o verdadeiro como um.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Devolve o maior número entre as células que cumprem várias condições.

- `max_range`: As células das quais obter o maior valor.
- `criteria_range1`: O primeiro bloco de células a testar.
- `criteria1`: O que procurar no primeiro bloco de células: um número, um texto ou um teste, como ">5".
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MEDIAN

`MEDIAN(number1, ...)`

Devolve o número central de um conjunto.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MINA

`MINA(value1, ...)`

Devolve o menor valor, contando o texto como zero e o verdadeiro como um.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Devolve o menor número entre as células que cumprem várias condições.

- `min_range`: As células das quais obter o menor valor.
- `criteria_range1`: O primeiro bloco de células a testar.
- `criteria1`: O que procurar no primeiro bloco de células: um número, um texto ou um teste, como ">5".
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MODE

`MODE(number1, ...)`

Devolve o número que ocorre com mais frequência.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MODE.MULT

`MODE.MULT(number1, ...)`

Devolve uma coluna com os números que surgem com mais frequência num conjunto.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Devolve o número que surge com mais frequência num conjunto.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Devolve a probabilidade de um número de insucessos antes de um número de sucessos.

- `number_f`: O número de insucessos.
- `number_s`: O número de sucessos.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Devolve a probabilidade de um número de insucessos antes de um número de sucessos.

- `number_f`: O número de insucessos.
- `number_s`: O número de sucessos.
- `probability_s`: A probabilidade de sucesso em cada tentativa, de 0 a 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Devolve o valor ou a percentagem acumulada da curva normal em x.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Devolve o valor de x correspondente a uma percentagem de uma curva normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Devolve o valor ou a percentagem acumulada da curva normal padrão em z.

- `z`: O valor de z.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## NORM.S.INV

`NORM.S.INV(probability)`

Devolve o valor de z correspondente a uma percentagem da curva normal padrão.

- `probability`: Uma probabilidade, de 0 a 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Devolve o valor ou a percentagem acumulada da curva normal em x.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Devolve o valor de x correspondente a uma percentagem de uma curva normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.

## NORMSDIST

`NORMSDIST(z)`

Devolve a percentagem acumulada da curva normal padrão até z.

- `z`: O valor de z.

## NORMSINV

`NORMSINV(probability)`

Devolve o valor de z correspondente a uma percentagem da curva normal padrão.

- `probability`: Uma probabilidade, de 0 a 1.

## PEARSON

`PEARSON(array1, array2)`

Devolve o grau em que dois conjuntos de números variam em conjunto, de -1 a 1.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.

## PERCENTILE

`PERCENTILE(array, k)`

Devolve o valor abaixo do qual cai uma percentagem k dos números.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `k`: A percentagem dos valores, de 0 a 1, para a qual devolver o valor.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Devolve o valor abaixo do qual cai uma percentagem k dos números, excluindo 0 e 1.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `k`: A posição na ordem, contando a partir de 1.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Devolve o valor abaixo do qual cai uma percentagem k dos números, de 0 a 1.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `k`: A posição na ordem, contando a partir de 1.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Devolve a posição de um valor num conjunto de números, como uma percentagem de 0 a 1.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `x`: O valor de x a utilizar.
- `significance` (opcional): O número cujos múltiplos arredondar.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Devolve a posição de um valor num conjunto de números, como uma percentagem, excluindo 0 e 1.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `x`: O valor de x a utilizar.
- `significance` (opcional): O número cujos múltiplos arredondar.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Devolve a posição de um valor num conjunto de números, como uma percentagem de 0 a 1.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `x`: O valor de x a utilizar.
- `significance` (opcional): O número cujos múltiplos arredondar.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Devolve o número de escolhas ordenadas quando um item pode ser escolhido mais do que uma vez.

- `number`: O número a processar.
- `number_chosen`: Quantos itens escolher em cada grupo.

## PHI

`PHI(x)`

Devolve a altura da curva normal padrão em x.

- `x`: O valor de x a utilizar.

## POISSON

`POISSON(x, mean, cumulative)`

Devolve a probabilidade de um número de eventos, a partir do número esperado.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Devolve a probabilidade de um número de eventos, a partir do número esperado.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Devolve a probabilidade de os valores caírem entre dois limites.

- `x_range`: O bloco de células com os valores de x.
- `prob_range`: As probabilidades correspondentes a cada valor de x, cada uma de 0 a 1, com soma igual a 1.
- `lower_limit`: O limite inferior do intervalo.
- `upper_limit` (opcional): O limite superior do intervalo (apenas o limite inferior, se for omitido).

## QUARTILE

`QUARTILE(array, quart)`

Devolve os pontos de quartil de um conjunto de números.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `quart`: Que quartil: 0 o menor, 1 o primeiro quartil, 2 a mediana, 3 o terceiro quartil, 4 o maior.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Devolve os pontos de quartil de um conjunto de números, excluindo os extremos.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `quart`: Que quartil: 0 o menor, 1 o primeiro quartil, 2 a mediana, 3 o terceiro quartil, 4 o maior.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Devolve os pontos de quartil de um conjunto de números, do menor para o maior.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `quart`: Que quartil: 0 o menor, 1 o primeiro quartil, 2 a mediana, 3 o terceiro quartil, 4 o maior.

## RANK

`RANK(number, ref, [order])`

Devolve a posição de um número numa lista quando a lista é ordenada.

- `number`: O número a processar.
- `ref`: Uma célula ou bloco de células.
- `order` (opcional): 0 ou omitido coloca o maior em primeiro; qualquer outro número coloca o menor em primeiro.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Devolve a posição de um número numa lista e a posição média quando há empates.

- `number`: O número a processar.
- `ref`: Uma célula ou bloco de células.
- `order` (opcional): 0 ou omitido coloca o maior em primeiro; qualquer outro número coloca o menor em primeiro.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Devolve a posição de um número numa lista e a posição mais alta quando há empates.

- `number`: O número a processar.
- `ref`: Uma célula ou bloco de células.
- `order` (opcional): 0 ou omitido coloca o maior em primeiro; qualquer outro número coloca o menor em primeiro.

## RSQ

`RSQ(known_y's, known_x's)`

Devolve o grau de ajuste de uma reta a dois conjuntos de números.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's`: O bloco de células que contém os valores de x que já tem.

## SKEW

`SKEW(number1, ...)`

Devolve o grau de assimetria de um conjunto de números em relação à sua média, para uma amostra.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SKEW.P

`SKEW.P(number1, ...)`

Devolve o grau de assimetria de um conjunto de números em relação à sua média, para uma população inteira.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SLOPE

`SLOPE(known_y's, known_x's)`

Devolve o declive da reta que melhor se ajusta a dois conjuntos de números.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's`: O bloco de células que contém os valores de x que já tem.

## SMALL

`SMALL(array, k)`

Devolve o k-ésimo menor número de um conjunto.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `k`: Que posição devolver, contando a partir do menor: 1 é o menor.

## STANDARDIZE

`STANDARDIZE(x, mean, standard_dev)`

Devolve a distância de um valor à média, em número de desvios-padrão.

- `x`: O valor de x a utilizar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio-padrão da distribuição.

## STDEV

`STDEV(number1, ...)`

Estima a dispersão de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## STDEV.P

`STDEV.P(number1, ...)`

Devolve a dispersão de um grupo inteiro.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## STDEV.S

`STDEV.S(number1, ...)`

Estima a dispersão de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## STDEVA

`STDEVA(value1, ...)`

Devolve o desvio-padrão de uma amostra, contando o texto como 0 e o verdadeiro como 1.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## STDEVP

`STDEVP(number1, ...)`

Devolve a dispersão de um grupo inteiro.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## STDEVPA

`STDEVPA(value1, ...)`

Devolve o desvio-padrão de uma população, contando o texto como 0 e o verdadeiro como 1.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## STEYX

`STEYX(known_y's, known_x's)`

Devolve o erro-padrão dos valores de y previstos por uma reta ajustada.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's`: O bloco de células que contém os valores de x que já tem.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Soma as células que cumprem uma condição.

- `range`: O bloco de células a testar.
- `criteria`: O que procurar: um número, um texto ou um teste, como ">5".
- `sum_range` (opcional): As células a somar (as células testadas, se for omitido).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Soma as células que cumprem várias condições.

- `sum_range`: As células a somar (as células testadas, se for omitido).
- `criteria_range1`: O primeiro bloco de células a testar.
- `criteria1`: O que procurar no primeiro bloco de células: um número, um texto ou um teste, como ">5".
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Devolve o valor ou a percentagem acumulada da curva t de Student em x.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Devolve a percentagem da curva t de Student fora de menos x e de x.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Devolve a percentagem da curva t de Student acima de x.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## T.INV

`T.INV(probability, deg_freedom)`

Devolve o valor de x correspondente a uma percentagem da curva t de Student, contada a partir da esquerda.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Devolve o valor de x correspondente a uma percentagem da curva t de Student, repartida pelas duas caudas.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Devolve a probabilidade de as médias de dois conjuntos de números serem iguais.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.
- `tails`: 1 para uma cauda da curva, 2 para ambas.
- `type`: Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## TDIST

`TDIST(x, deg_freedom, tails)`

Devolve a percentagem da curva t de Student numa cauda ou em ambas.

- `x`: O valor de x a utilizar.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.
- `tails`: 1 para uma cauda da curva, 2 para ambas.

## TINV

`TINV(probability, deg_freedom)`

Devolve o valor de x correspondente a uma percentagem da curva t de Student, repartida pelas duas caudas.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro igual ou superior a 1.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Devolve os valores ao longo da reta que melhor se ajusta, para novos valores de x.

- `known_y's`: O bloco de células que contém os valores de y que já tem.
- `known_x's` (opcional): O bloco de células que contém os valores de x que já tem.
- `new_x's` (opcional): Os valores de x para os quais devolver valores de y (os valores de x conhecidos, se for omitido).
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não tem de passar por zero; falso para que passe por zero.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Devolve a média de um conjunto de números depois de retirar uma percentagem dos valores mais altos e mais baixos.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `percent`: A percentagem dos valores, de 0 a menos de 1, a excluir nos dois extremos.

## TTEST

`TTEST(array1, array2, tails, type)`

Devolve a probabilidade de as médias de dois conjuntos de números serem iguais.

- `array1`: O primeiro bloco de células ou lista de valores.
- `array2`: O segundo bloco de células ou lista de valores, do mesmo tamanho do primeiro.
- `tails`: 1 para uma cauda da curva, 2 para ambas.
- `type`: Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## VAR

`VAR(number1, ...)`

Estima a variância de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## VAR.P

`VAR.P(number1, ...)`

Devolve a variância de um grupo inteiro.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## VAR.S

`VAR.S(number1, ...)`

Estima a variância de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## VARA

`VARA(value1, ...)`

Devolve a variância de uma amostra, contando o texto como 0 e o verdadeiro como 1.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## VARP

`VARP(number1, ...)`

Devolve a variância de um grupo inteiro.

- `number1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## VARPA

`VARPA(value1, ...)`

Devolve a variância de uma população, contando o texto como 0 e o verdadeiro como 1.

- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Devolve o valor ou a percentagem acumulada da curva de Weibull em x.

- `x`: O valor de x a utilizar.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Devolve o valor ou a percentagem acumulada da curva de Weibull em x.

- `x`: O valor de x a utilizar.
- `alpha`: A probabilidade de erro que aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para devolver o total acumulado até este ponto; falso para devolver apenas o valor neste ponto.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Devolve a probabilidade, num só sentido, de a média de uma amostra ser superior a um valor.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `x`: O valor de x a utilizar.
- `sigma` (opcional): O desvio-padrão de toda a população (o da amostra, se for omitido).

## ZTEST

`ZTEST(array, x, [sigma])`

Devolve a probabilidade, num só sentido, de a média de uma amostra ser superior a um valor.

- `array`: Um bloco de células, ou uma lista de valores escrita entre chavetas.
- `x`: O valor de x a utilizar.
- `sigma` (opcional): O desvio-padrão de toda a população (o da amostra, se for omitido).
