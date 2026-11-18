# Funções estatísticas
<!-- keywords: funções, estatística, probabilidade, média, desvio padrão, variância, previsão, regressão, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.ETS, FORECAST.ETS.CONFINT, FORECAST.ETS.SEASONALITY, FORECAST.ETS.STAT, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST -->

Estas são as funções estatísticas que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros forem necessários após o último. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica aparece sob a célula mostrando os argumentos dela, e um clique no nome da função na dica abre a entrada correspondente aqui.

## AVEDEV

`AVEDEV(number1, ...)`

Retorna a média das distâncias dos números em relação à média deles.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## AVERAGE

`AVERAGE(number1, ...)`

Retorna a média dos números.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## AVERAGEA

`AVERAGEA(value1, ...)`

Retorna a média dos valores, contando texto como zero e verdadeiro como um.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## AVERAGEIF

`AVERAGEIF(range, criteria, [average_range])`

Calcula a média das células que atendem a uma condição.

- `range`: O intervalo de células a testar.
- `criteria`: O que procurar: um número, um texto ou um teste, como ">5".
- `average_range` (opcional): As células cuja média será calculada (as células testadas, se omitido).

## AVERAGEIFS

`AVERAGEIFS(average_range, criteria_range1, criteria1, ...)`

Calcula a média das células que atendem a várias condições.

- `average_range`: As células cuja média será calculada (as células testadas, se omitido).
- `criteria_range1`: O primeiro intervalo de células a testar.
- `criteria1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## BETA.DIST

`BETA.DIST(x, alpha, beta, cumulative, [A], [B])`

Retorna o valor da curva beta em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BETA.INV

`BETA.INV(probability, alpha, beta, [A], [B])`

Retorna o valor de x para uma proporção de uma curva beta.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BETADIST

`BETADIST(x, alpha, beta, [A], [B])`

Retorna a proporção acumulada da curva beta até x.

- `x`: O valor de x com o qual trabalhar.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BETAINV

`BETAINV(probability, alpha, beta, [A], [B])`

Retorna o valor de x para uma proporção de uma curva beta.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BINOM.DIST

`BINOM.DIST(number_s, trials, probability_s, cumulative)`

Retorna a chance de um número de sucessos em um número de tentativas.

- `number_s`: O número de sucessos.
- `trials`: O número de tentativas.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(trials, probability_s, number_s, [number_s2])`

Retorna a chance de os sucessos caírem em um intervalo.

- `trials`: O número de tentativas.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.
- `number_s`: O número de sucessos.
- `number_s2` (opcional): O maior número de sucessos a contar (o mesmo que o menor, se omitido).

## BINOM.INV

`BINOM.INV(trials, probability_s, alpha)`

Retorna o menor número de sucessos cuja chance acumulada atinge um valor.

- `trials`: O número de tentativas.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.

## BINOMDIST

`BINOMDIST(number_s, trials, probability_s, cumulative)`

Retorna a chance de um número de sucessos em um número de tentativas.

- `number_s`: O número de sucessos.
- `trials`: O número de tentativas.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## CHIDIST

`CHIDIST(x, deg_freedom)`

Retorna a proporção da curva qui-quadrado acima de x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## CHIINV

`CHIINV(probability, deg_freedom)`

Retorna o valor de x para uma proporção da curva qui-quadrado, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## CHISQ.DIST

`CHISQ.DIST(x, deg_freedom, cumulative)`

Retorna o valor da curva qui-quadrado em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x, deg_freedom)`

Retorna a proporção da curva qui-quadrado acima de x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## CHISQ.INV

`CHISQ.INV(probability, deg_freedom)`

Retorna o valor de x para uma proporção da curva qui-quadrado, contada a partir da esquerda.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## CHISQ.INV.RT

`CHISQ.INV.RT(probability, deg_freedom)`

Retorna o valor de x para uma proporção da curva qui-quadrado, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## CHISQ.TEST

`CHISQ.TEST(actual_range, expected_range)`

Retorna a chance de as contagens diferirem das contagens esperadas apenas por acaso.

- `actual_range`: O intervalo de células com as contagens obtidas.
- `expected_range`: O intervalo de células com as contagens esperadas, do mesmo tamanho das contagens obtidas.

## CHITEST

`CHITEST(actual_range, expected_range)`

Retorna a chance de as contagens diferirem das contagens esperadas apenas por acaso.

- `actual_range`: O intervalo de células com as contagens obtidas.
- `expected_range`: O intervalo de células com as contagens esperadas, do mesmo tamanho das contagens obtidas.

## CONFIDENCE

`CONFIDENCE(alpha, standard_dev, size)`

Retorna a margem de um intervalo de confiança para uma média, usando a curva normal.

- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `standard_dev`: O desvio padrão da distribuição.
- `size`: O tamanho da amostra, um número inteiro de 1 ou mais.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alpha, standard_dev, size)`

Retorna a margem de um intervalo de confiança para uma média, usando a curva normal.

- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `standard_dev`: O desvio padrão da distribuição.
- `size`: O tamanho da amostra, um número inteiro de 1 ou mais.

## CONFIDENCE.T

`CONFIDENCE.T(alpha, standard_dev, size)`

Retorna a margem de um intervalo de confiança para uma média, usando a curva t de Student.

- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `standard_dev`: O desvio padrão da distribuição.
- `size`: O tamanho da amostra, um número inteiro de 1 ou mais.

## CORREL

`CORREL(array1, array2)`

Retorna o quanto dois conjuntos de números variam juntos, de -1 a 1.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## COUNT

`COUNT(value1, ...)`

Conta os números em uma lista ou em um intervalo de células.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## COUNTA

`COUNTA(value1, ...)`

Conta as células que não estão vazias.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## COUNTBLANK

`COUNTBLANK(range)`

Conta as células vazias em um intervalo.

- `range`: O intervalo de células a testar.

## COUNTIF

`COUNTIF(range, criteria)`

Conta as células que atendem a uma condição.

- `range`: O intervalo de células a testar.
- `criteria`: O que procurar: um número, um texto ou um teste, como ">5".

## COUNTIFS

`COUNTIFS(criteria_range1, criteria1, ...)`

Conta as linhas em que várias condições são todas atendidas.

- `criteria_range1`: O primeiro intervalo de células a testar.
- `criteria1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## COVAR

`COVAR(array1, array2)`

Retorna o quanto dois conjuntos de números variam juntos.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## COVARIANCE.P

`COVARIANCE.P(array1, array2)`

Retorna o quanto dois conjuntos de números variam juntos, para uma população.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## COVARIANCE.S

`COVARIANCE.S(array1, array2)`

Retorna o quanto dois conjuntos de números variam juntos, para uma amostra.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## CRITBINOM

`CRITBINOM(trials, probability_s, alpha)`

Retorna o menor número de sucessos cuja chance acumulada atinge um valor.

- `trials`: O número de tentativas.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.

## DEVSQ

`DEVSQ(number1, ...)`

Retorna a soma dos quadrados das distâncias dos números em relação à média deles.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## EXPON.DIST

`EXPON.DIST(x, lambda, cumulative)`

Retorna o valor da curva exponencial em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `lambda`: A taxa da curva, maior que 0.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## EXPONDIST

`EXPONDIST(x, lambda, cumulative)`

Retorna o valor da curva exponencial em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `lambda`: A taxa da curva, maior que 0.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## F.DIST

`F.DIST(x, deg_freedom1, deg_freedom2, cumulative)`

Retorna o valor da curva F em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro de 1 ou mais.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro de 1 ou mais.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## F.DIST.RT

`F.DIST.RT(x, deg_freedom1, deg_freedom2)`

Retorna a proporção da curva F acima de x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro de 1 ou mais.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro de 1 ou mais.

## F.INV

`F.INV(probability, deg_freedom1, deg_freedom2)`

Retorna o valor de x para uma proporção da curva F, contada a partir da esquerda.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro de 1 ou mais.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro de 1 ou mais.

## F.INV.RT

`F.INV.RT(probability, deg_freedom1, deg_freedom2)`

Retorna o valor de x para uma proporção da curva F, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro de 1 ou mais.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro de 1 ou mais.

## F.TEST

`F.TEST(array1, array2)`

Retorna a chance de dois conjuntos de números terem a mesma variância.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## FDIST

`FDIST(x, deg_freedom1, deg_freedom2)`

Retorna a proporção da curva F acima de x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro de 1 ou mais.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro de 1 ou mais.

## FINV

`FINV(probability, deg_freedom1, deg_freedom2)`

Retorna o valor de x para uma proporção da curva F, contada a partir da direita.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom1`: Os graus de liberdade do numerador, um número inteiro de 1 ou mais.
- `deg_freedom2`: Os graus de liberdade do denominador, um número inteiro de 1 ou mais.

## FISHER

`FISHER(x)`

Retorna a transformação de Fisher de um número entre -1 e 1.

- `x`: O valor de x com o qual trabalhar.

## FISHERINV

`FISHERINV(y)`

Retorna a inversa da transformação de Fisher.

- `y`: O número cuja inversa da transformação de Fisher será calculada.

## FORECAST

`FORECAST(x, known_y's, known_x's)`

Prevê um valor ao longo da reta que melhor se ajusta aos dados.

- `x`: O valor de x para o qual prever um valor de y.
- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's`: O intervalo de células que contém os valores de x que você já tem.

## FORECAST.ETS

`FORECAST.ETS(target_date, values, timeline, [seasonality], [data_completion], [aggregation])`

Prevê um valor futuro de uma série que tem tendência e sazonalidade, por suavização exponencial.

- `target_date`: O momento para o qual prever um valor.
- `values`: Os valores que você já tem.
- `timeline`: As datas ou números dos valores, espaçados igualmente.
- `seasonality`: 1 para descobrir a sazonalidade a partir dos valores, 0 para nenhuma, ou a duração da sazonalidade.
- `data_completion`: 1 para preencher um ponto ausente com uma reta entre os vizinhos, 0 para preenchê-lo com zero.
- `aggregation`: Como os valores no mesmo momento são combinados: 1 média, 2 contagem, 3 CONTARA, 4 máx, 5 mediana, 6 mín, 7 soma.

## FORECAST.ETS.CONFINT

`FORECAST.ETS.CONFINT(target_date, values, timeline, [confidence_level], [seasonality], [data_completion], [aggregation])`

Retorna o quanto acima e abaixo de uma previsão suavizada o valor provavelmente estará.

- `confidence_level`: Um número maior que 0 e menor que 1; 0,95, se omitido.

## FORECAST.ETS.SEASONALITY

`FORECAST.ETS.SEASONALITY(values, timeline, [data_completion], [aggregation])`

Retorna a duração da sazonalidade que se repete em uma série, ou 1 se não houver nenhuma.

## FORECAST.ETS.STAT

`FORECAST.ETS.STAT(values, timeline, statistic_type, [seasonality], [data_completion], [aggregation])`

Retorna uma das constantes de suavização ou medidas de ajuste de uma previsão suavizada.

- `statistic_type`: 1 Alfa, 2 Beta, 3 Gama, 4 MASE, 5 SMAPE, 6 MAE, 7 RMSE, 8 o passo da linha do tempo.

## FORECAST.LINEAR

`FORECAST.LINEAR(x, known_y's, known_x's)`

Prevê um valor ao longo da reta que melhor se ajusta aos dados.

- `x`: O valor de x com o qual trabalhar.
- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's`: O intervalo de células que contém os valores de x que você já tem.

## FREQUENCY

`FREQUENCY(data_array, bins_array)`

Conta quantos valores caem em cada intervalo e retorna as contagens como uma coluna.

- `data_array`: O intervalo de células com os valores a contar.
- `bins_array`: O intervalo de células com os limites dos intervalos em que contar.

## FTEST

`FTEST(array1, array2)`

Retorna a chance de dois conjuntos de números terem a mesma variância.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## GAMMA

`GAMMA(x)`

Retorna a função gama de um número.

- `x`: O valor de x com o qual trabalhar.

## GAMMA.DIST

`GAMMA.DIST(x, alpha, beta, cumulative)`

Retorna o valor da curva gama em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## GAMMA.INV

`GAMMA.INV(probability, alpha, beta)`

Retorna o valor de x para uma proporção de uma curva gama.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).

## GAMMADIST

`GAMMADIST(x, alpha, beta, cumulative)`

Retorna o valor da curva gama em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## GAMMAINV

`GAMMAINV(probability, alpha, beta)`

Retorna o valor de x para uma proporção de uma curva gama.

- `probability`: Uma probabilidade, de 0 a 1.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).

## GAMMALN

`GAMMALN(x)`

Retorna o logaritmo natural da função gama.

- `x`: O valor de x com o qual trabalhar.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Retorna o logaritmo natural da função gama.

- `x`: O valor de x com o qual trabalhar.

## GAUSS

`GAUSS(z)`

Retorna a proporção da curva normal padrão entre a média e z.

- `z`: O valor de z.

## GEOMEAN

`GEOMEAN(number1, ...)`

Retorna a média geométrica de números positivos.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## GROWTH

`GROWTH(known_y's, [known_x's], [new_x's], [const])`

Retorna os valores ao longo da curva exponencial que melhor se ajusta aos dados, para novos valores de x.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `new_x's` (opcional): Os valores de x para os quais retornar os valores de y (os valores de x conhecidos, se omitido).
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.

## HARMEAN

`HARMEAN(number1, ...)`

Retorna a média harmônica de números positivos.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## HYPGEOM.DIST

`HYPGEOM.DIST(sample_s, number_sample, population_s, number_pop, cumulative)`

Retorna a chance de, ao sortear itens sem devolvê-los, obter um número de sucessos.

- `sample_s`: O número de sucessos na amostra.
- `number_sample`: O número de itens sorteados.
- `population_s`: O número de sucessos em toda a população.
- `number_pop`: O número de itens em toda a população.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## HYPGEOMDIST

`HYPGEOMDIST(sample_s, number_sample, population_s, number_pop)`

Retorna a chance de, ao sortear itens sem devolvê-los, obter um número de sucessos.

- `sample_s`: O número de sucessos na amostra.
- `number_sample`: O número de itens sorteados.
- `population_s`: O número de sucessos em toda a população.
- `number_pop`: O número de itens em toda a população.

## INTERCEPT

`INTERCEPT(known_y's, known_x's)`

Retorna o ponto em que a reta que melhor se ajusta aos dados cruza o eixo vertical.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's`: O intervalo de células que contém os valores de x que você já tem.

## KURT

`KURT(number1, ...)`

Retorna o quanto um conjunto de números é pontudo ou achatado em comparação com uma curva normal.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## LARGE

`LARGE(array, k)`

Retorna o k-ésimo maior número de um conjunto.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `k`: Qual posição retornar, contando a partir do maior: 1 é o maior.

## LINEST

`LINEST(known_y's, [known_x's], [const], [stats])`

Ajusta uma reta ou um plano aos dados pelo método dos mínimos quadrados e retorna os seus números como uma linha.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.
- `stats` (opcional): Verdadeiro para retornar também mais números sobre o ajuste; falso ou omitido para retornar somente a reta.

## LOGEST

`LOGEST(known_y's, [known_x's], [const], [stats])`

Ajusta uma curva exponencial aos dados e retorna os seus números como uma linha.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.
- `stats` (opcional): Verdadeiro para retornar também mais números sobre o ajuste; falso ou omitido para retornar somente a linha.

## LOGINV

`LOGINV(probability, mean, standard_dev)`

Retorna o valor de x para uma proporção de uma curva log-normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.

## LOGNORM.DIST

`LOGNORM.DIST(x, mean, standard_dev, cumulative)`

Retorna o valor da curva log-normal em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## LOGNORM.INV

`LOGNORM.INV(probability, mean, standard_dev)`

Retorna o valor de x para uma proporção de uma curva log-normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.

## LOGNORMDIST

`LOGNORMDIST(x, mean, standard_dev)`

Retorna a proporção acumulada da curva log-normal até x.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.

## MAXA

`MAXA(value1, ...)`

Retorna o maior valor, contando texto como zero e verdadeiro como um.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MAXIFS

`MAXIFS(max_range, criteria_range1, criteria1, ...)`

Retorna o maior número entre as células que atendem a várias condições.

- `max_range`: As células das quais obter o maior valor.
- `criteria_range1`: O primeiro intervalo de células a testar.
- `criteria1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MEDIAN

`MEDIAN(number1, ...)`

Retorna o número central de um conjunto.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MINA

`MINA(value1, ...)`

Retorna o menor valor, contando texto como zero e verdadeiro como um.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MINIFS

`MINIFS(min_range, criteria_range1, criteria1, ...)`

Retorna o menor número entre as células que atendem a várias condições.

- `min_range`: As células das quais obter o menor valor.
- `criteria_range1`: O primeiro intervalo de células a testar.
- `criteria1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MODE

`MODE(number1, ...)`

Retorna o número que mais se repete.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MODE.MULT

`MODE.MULT(number1, ...)`

Retorna uma coluna com os números que mais se repetem em um conjunto.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MODE.SNGL

`MODE.SNGL(number1, ...)`

Retorna o número que mais se repete em um conjunto.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## NEGBINOM.DIST

`NEGBINOM.DIST(number_f, number_s, probability_s, cumulative)`

Retorna a chance de um número de falhas ocorrer antes de um número de sucessos.

- `number_f`: O número de falhas.
- `number_s`: O número de sucessos.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## NEGBINOMDIST

`NEGBINOMDIST(number_f, number_s, probability_s)`

Retorna a chance de um número de falhas ocorrer antes de um número de sucessos.

- `number_f`: O número de falhas.
- `number_s`: O número de sucessos.
- `probability_s`: A chance de sucesso em cada tentativa, de 0 a 1.

## NORM.DIST

`NORM.DIST(x, mean, standard_dev, cumulative)`

Retorna o valor da curva normal em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## NORM.INV

`NORM.INV(probability, mean, standard_dev)`

Retorna o valor de x para uma proporção de uma curva normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.

## NORM.S.DIST

`NORM.S.DIST(z, cumulative)`

Retorna o valor da curva normal padrão em z, ou a proporção acumulada até z.

- `z`: O valor de z.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## NORM.S.INV

`NORM.S.INV(probability)`

Retorna o valor de z para uma proporção da curva normal padrão.

- `probability`: Uma probabilidade, de 0 a 1.

## NORMDIST

`NORMDIST(x, mean, standard_dev, cumulative)`

Retorna o valor da curva normal em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## NORMINV

`NORMINV(probability, mean, standard_dev)`

Retorna o valor de x para uma proporção de uma curva normal.

- `probability`: Uma probabilidade, de 0 a 1.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.

## NORMSDIST

`NORMSDIST(z)`

Retorna a proporção acumulada da curva normal padrão até z.

- `z`: O valor de z.

## NORMSINV

`NORMSINV(probability)`

Retorna o valor de z para uma proporção da curva normal padrão.

- `probability`: Uma probabilidade, de 0 a 1.

## PEARSON

`PEARSON(array1, array2)`

Retorna o quanto dois conjuntos de números variam juntos, de -1 a 1.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## PERCENTILE

`PERCENTILE(array, k)`

Retorna o valor abaixo do qual cai uma proporção k dos números.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `k`: A proporção dos valores, de 0 a 1, para a qual retornar o valor.

## PERCENTILE.EXC

`PERCENTILE.EXC(array, k)`

Retorna o valor abaixo do qual cai uma proporção k dos números, sem contar 0 e 1.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `k`: A posição na ordem, contando a partir de 1.

## PERCENTILE.INC

`PERCENTILE.INC(array, k)`

Retorna o valor abaixo do qual cai uma proporção k dos números, de 0 a 1.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `k`: A posição na ordem, contando a partir de 1.

## PERCENTRANK

`PERCENTRANK(array, x, [significance])`

Retorna a posição de um valor em um conjunto de números como uma proporção de 0 a 1.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `x`: O valor de x com o qual trabalhar.
- `significance` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(array, x, [significance])`

Retorna a posição de um valor em um conjunto de números como uma proporção, sem contar 0 e 1.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `x`: O valor de x com o qual trabalhar.
- `significance` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PERCENTRANK.INC

`PERCENTRANK.INC(array, x, [significance])`

Retorna a posição de um valor em um conjunto de números como uma proporção de 0 a 1.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `x`: O valor de x com o qual trabalhar.
- `significance` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PERMUTATIONA

`PERMUTATIONA(number, number_chosen)`

Retorna o número de escolhas ordenadas quando um item pode ser escolhido mais de uma vez.

- `number`: O número com o qual trabalhar.
- `number_chosen`: Quantos itens escolher em cada grupo.

## PHI

`PHI(x)`

Retorna a altura da curva normal padrão em x.

- `x`: O valor de x com o qual trabalhar.

## POISSON

`POISSON(x, mean, cumulative)`

Retorna a chance de um número de eventos, a partir de quantos são esperados.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## POISSON.DIST

`POISSON.DIST(x, mean, cumulative)`

Retorna a chance de um número de eventos, a partir de quantos são esperados.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## PROB

`PROB(x_range, prob_range, lower_limit, [upper_limit])`

Retorna a chance de os valores caírem entre dois limites.

- `x_range`: O intervalo de células com os valores de x.
- `prob_range`: As chances correspondentes a cada valor de x, cada uma de 0 a 1, cuja soma é 1.
- `lower_limit`: O limite inferior do intervalo.
- `upper_limit` (opcional): O limite superior do intervalo (somente o limite inferior, se omitido).

## QUARTILE

`QUARTILE(array, quart)`

Retorna os pontos de quartil de um conjunto de números.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `quart`: Qual quartil: 0 o menor, 1 o primeiro quartil, 2 a mediana, 3 o terceiro quartil, 4 o maior.

## QUARTILE.EXC

`QUARTILE.EXC(array, quart)`

Retorna os pontos de quartil de um conjunto de números, sem contar as extremidades.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `quart`: Qual quartil: 0 o menor, 1 o primeiro quartil, 2 a mediana, 3 o terceiro quartil, 4 o maior.

## QUARTILE.INC

`QUARTILE.INC(array, quart)`

Retorna os pontos de quartil de um conjunto de números, do menor ao maior.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `quart`: Qual quartil: 0 o menor, 1 o primeiro quartil, 2 a mediana, 3 o terceiro quartil, 4 o maior.

## RANK

`RANK(number, ref, [order])`

Retorna a posição de um número em uma lista quando a lista é colocada em ordem.

- `number`: O número com o qual trabalhar.
- `ref`: Uma célula ou intervalo de células.
- `order` (opcional): 0 ou omitido coloca o maior primeiro; qualquer outro número coloca o menor primeiro.

## RANK.AVG

`RANK.AVG(number, ref, [order])`

Retorna a posição de um número em uma lista, e a posição média quando há empates.

- `number`: O número com o qual trabalhar.
- `ref`: Uma célula ou intervalo de células.
- `order` (opcional): 0 ou omitido coloca o maior primeiro; qualquer outro número coloca o menor primeiro.

## RANK.EQ

`RANK.EQ(number, ref, [order])`

Retorna a posição de um número em uma lista, e a posição mais alta quando há empates.

- `number`: O número com o qual trabalhar.
- `ref`: Uma célula ou intervalo de células.
- `order` (opcional): 0 ou omitido coloca o maior primeiro; qualquer outro número coloca o menor primeiro.

## RSQ

`RSQ(known_y's, known_x's)`

Retorna o quanto bem uma reta se ajusta a dois conjuntos de números.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's`: O intervalo de células que contém os valores de x que você já tem.

## SKEW

`SKEW(number1, ...)`

Retorna o quanto um conjunto de números é assimétrico em torno da média, para uma amostra.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## SKEW.P

`SKEW.P(number1, ...)`

Retorna o quanto um conjunto de números é assimétrico em torno da média, para uma população inteira.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## SLOPE

`SLOPE(known_y's, known_x's)`

Retorna a inclinação da reta que melhor se ajusta a dois conjuntos de números.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's`: O intervalo de células que contém os valores de x que você já tem.

## SMALL

`SMALL(array, k)`

Retorna o k-ésimo menor número de um conjunto.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `k`: Qual posição retornar, contando a partir do menor: 1 é o menor.

## STANDARDIZE

`STANDARDIZE(x, mean, standard_dev)`

Retorna a quantos desvios padrão um valor está da média.

- `x`: O valor de x com o qual trabalhar.
- `mean`: A média da distribuição.
- `standard_dev`: O desvio padrão da distribuição.

## STDEV

`STDEV(number1, ...)`

Estima a dispersão de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEV.P

`STDEV.P(number1, ...)`

Retorna a dispersão de um grupo inteiro.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEV.S

`STDEV.S(number1, ...)`

Estima a dispersão de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEVA

`STDEVA(value1, ...)`

Retorna o desvio padrão de uma amostra, contando texto como 0 e verdadeiro como 1.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEVP

`STDEVP(number1, ...)`

Retorna a dispersão de um grupo inteiro.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEVPA

`STDEVPA(value1, ...)`

Retorna o desvio padrão de uma população, contando texto como 0 e verdadeiro como 1.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STEYX

`STEYX(known_y's, known_x's)`

Retorna o erro padrão dos valores de y previstos por uma reta ajustada.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's`: O intervalo de células que contém os valores de x que você já tem.

## SUMIF

`SUMIF(range, criteria, [sum_range])`

Soma as células que atendem a uma condição.

- `range`: O intervalo de células a testar.
- `criteria`: O que procurar: um número, um texto ou um teste, como ">5".
- `sum_range` (opcional): As células a somar (as células testadas, se omitido).

## SUMIFS

`SUMIFS(sum_range, criteria_range1, criteria1, ...)`

Soma as células que atendem a várias condições.

- `sum_range`: As células a somar (as células testadas, se omitido).
- `criteria_range1`: O primeiro intervalo de células a testar.
- `criteria1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## T.DIST

`T.DIST(x, deg_freedom, cumulative)`

Retorna o valor da curva t de Student em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## T.DIST.2T

`T.DIST.2T(x, deg_freedom)`

Retorna a proporção da curva t de Student fora de -x e x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## T.DIST.RT

`T.DIST.RT(x, deg_freedom)`

Retorna a proporção da curva t de Student acima de x.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## T.INV

`T.INV(probability, deg_freedom)`

Retorna o valor de x para uma proporção da curva t de Student, contada a partir da esquerda.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## T.INV.2T

`T.INV.2T(probability, deg_freedom)`

Retorna o valor de x para uma proporção da curva t de Student, dividida entre as duas caudas.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## T.TEST

`T.TEST(array1, array2, tails, type)`

Retorna a chance de as médias de dois conjuntos de números serem iguais.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.
- `tails`: 1 para uma cauda da curva, 2 para as duas.
- `type`: Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## TDIST

`TDIST(x, deg_freedom, tails)`

Retorna a proporção da curva t de Student em uma cauda ou nas duas.

- `x`: O valor de x com o qual trabalhar.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.
- `tails`: 1 para uma cauda da curva, 2 para as duas.

## TINV

`TINV(probability, deg_freedom)`

Retorna o valor de x para uma proporção da curva t de Student, dividida entre as duas caudas.

- `probability`: Uma probabilidade, de 0 a 1.
- `deg_freedom`: Os graus de liberdade, um número inteiro de 1 ou mais.

## TREND

`TREND(known_y's, [known_x's], [new_x's], [const])`

Retorna os valores ao longo da reta que melhor se ajusta aos dados, para novos valores de x.

- `known_y's`: O intervalo de células que contém os valores de y que você já tem.
- `known_x's` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `new_x's` (opcional): Os valores de x para os quais retornar os valores de y (os valores de x conhecidos, se omitido).
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.

## TRIMMEAN

`TRIMMEAN(array, percent)`

Retorna a média de um conjunto de números depois de descartar uma proporção dos valores mais altos e dos mais baixos.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `percent`: A proporção dos valores, de 0 até menos de 1, a deixar de fora nas duas extremidades.

## TTEST

`TTEST(array1, array2, tails, type)`

Retorna a chance de as médias de dois conjuntos de números serem iguais.

- `array1`: O primeiro intervalo de células ou lista de valores.
- `array2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.
- `tails`: 1 para uma cauda da curva, 2 para as duas.
- `type`: Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## VAR

`VAR(number1, ...)`

Estima a variância de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VAR.P

`VAR.P(number1, ...)`

Retorna a variância de um grupo inteiro.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VAR.S

`VAR.S(number1, ...)`

Estima a variância de um grupo inteiro a partir de uma amostra dele.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VARA

`VARA(value1, ...)`

Retorna a variância de uma amostra, contando texto como 0 e verdadeiro como 1.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VARP

`VARP(number1, ...)`

Retorna a variância de um grupo inteiro.

- `number1`: O primeiro número, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VARPA

`VARPA(value1, ...)`

Retorna a variância de uma população, contando texto como 0 e verdadeiro como 1.

- `value1`: O primeiro valor, ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## WEIBULL

`WEIBULL(x, alpha, beta, cumulative)`

Retorna o valor da curva de Weibull em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## WEIBULL.DIST

`WEIBULL.DIST(x, alpha, beta, cumulative)`

Retorna o valor da curva de Weibull em x, ou a proporção acumulada até x.

- `x`: O valor de x com o qual trabalhar.
- `alpha`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulative`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar apenas o valor neste ponto.

## Z.TEST

`Z.TEST(array, x, [sigma])`

Retorna a chance, em uma cauda, de a média de uma amostra estar acima de um valor.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `x`: O valor de x com o qual trabalhar.
- `sigma` (opcional): O desvio padrão de toda a população (o da amostra, se omitido).

## ZTEST

`ZTEST(array, x, [sigma])`

Retorna a chance, em uma cauda, de a média de uma amostra estar acima de um valor.

- `array`: Um intervalo de células, ou uma lista de valores escrita entre chaves.
- `x`: O valor de x com o qual trabalhar.
- `sigma` (opcional): O desvio padrão de toda a população (o da amostra, se omitido).
