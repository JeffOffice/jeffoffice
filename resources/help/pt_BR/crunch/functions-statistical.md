# Funções estatísticas
<!-- keywords: funções, AVEDEV, AVERAGE, AVERAGEA, AVERAGEIF, AVERAGEIFS, BETA.DIST, BETA.INV, BETADIST, BETAINV, BINOM.DIST, BINOM.DIST.RANGE, BINOM.INV, BINOMDIST, CHIDIST, CHIINV, CHISQ.DIST, CHISQ.DIST.RT, CHISQ.INV, CHISQ.INV.RT, CHISQ.TEST, CHITEST, CONFIDENCE, CONFIDENCE.NORM, CONFIDENCE.T, CORREL, COUNT, COUNTA, COUNTBLANK, COUNTIF, COUNTIFS, COVAR, COVARIANCE.P, COVARIANCE.S, CRITBINOM, DEVSQ, EXPON.DIST, EXPONDIST, F.DIST, F.DIST.RT, F.INV, F.INV.RT, F.TEST, FDIST, FINV, FISHER, FISHERINV, FORECAST, FORECAST.LINEAR, FREQUENCY, FTEST, GAMMA, GAMMA.DIST, GAMMA.INV, GAMMADIST, GAMMAINV, GAMMALN, GAMMALN.PRECISE, GAUSS, GEOMEAN, GROWTH, HARMEAN, HYPGEOM.DIST, HYPGEOMDIST, INTERCEPT, KURT, LARGE, LINEST, LOGEST, LOGINV, LOGNORM.DIST, LOGNORM.INV, LOGNORMDIST, MAXA, MAXIFS, MEDIAN, MINA, MINIFS, MODE, MODE.MULT, MODE.SNGL, NEGBINOM.DIST, NEGBINOMDIST, NORM.DIST, NORM.INV, NORM.S.DIST, NORM.S.INV, NORMDIST, NORMINV, NORMSDIST, NORMSINV, PEARSON, PERCENTILE, PERCENTILE.EXC, PERCENTILE.INC, PERCENTRANK, PERCENTRANK.EXC, PERCENTRANK.INC, PERMUTATIONA, PHI, POISSON, POISSON.DIST, PROB, QUARTILE, QUARTILE.EXC, QUARTILE.INC, RANK, RANK.AVG, RANK.EQ, RSQ, SKEW, SKEW.P, SLOPE, SMALL, STANDARDIZE, STDEV, STDEV.P, STDEV.S, STDEVA, STDEVP, STDEVPA, STEYX, SUMIF, SUMIFS, T.DIST, T.DIST.2T, T.DIST.RT, T.INV, T.INV.2T, T.TEST, TDIST, TINV, TREND, TRIMMEAN, TTEST, VAR, VAR.P, VAR.S, VARA, VARP, VARPA, WEIBULL, WEIBULL.DIST, Z.TEST, ZTEST, funções estatísticas, média, desvio padrão, variância, distribuição, probabilidade, regressão -->

Estas são as funções estatísticas que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros do último tipo forem necessários. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra os argumentos dela, e um clique no nome da função na dica abre a sua entrada aqui.

## AVEDEV

`AVEDEV(número1, ...)`

Retorna a média dos desvios absolutos dos números em relação à média deles.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## AVERAGE

`AVERAGE(número1, ...)`

Retorna a média dos números.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## AVERAGEA

`AVERAGEA(valor1, ...)`

Retorna a média dos valores, contando texto como zero e verdadeiro como um.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## AVERAGEIF

`AVERAGEIF(intervalo; critérios; [intervalo_média])`

Calcula a média das células que atendem a uma condição.

- `intervalo`: O intervalo de células a testar.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".
- `intervalo_média` (opcional): As células cuja média será calculada (as células testadas, se omitido).

## AVERAGEIFS

`AVERAGEIFS(intervalo_média; intervalo_critérios1; critérios1; ...)`

Calcula a média das células que atendem a várias condições.

- `intervalo_média`: As células cuja média será calculada (as células testadas, se omitido).
- `intervalo_critérios1`: O primeiro intervalo de células a testar.
- `critérios1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## BETA.DIST

`BETA.DIST(x; alfa; beta; cumulativo; [A]; [B])`

Retorna o valor da curva beta em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BETA.INV

`BETA.INV(probabilidade; alfa; beta; [A]; [B])`

Retorna o valor de x correspondente a uma proporção de uma curva beta.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BETADIST

`BETADIST(x; alfa; beta; [A]; [B])`

Retorna a proporção acumulada da curva beta até x.

- `x`: O valor de x a usar.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BETAINV

`BETAINV(probabilidade; alfa; beta; [A]; [B])`

Retorna o valor de x correspondente a uma proporção de uma curva beta.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `A` (opcional): O limite inferior de x (0, se omitido).
- `B` (opcional): O limite superior de x (1, se omitido).

## BINOM.DIST

`BINOM.DIST(núm_sucessos; tentativas; probabilidade_sucesso; cumulativo)`

Retorna a chance de um número de sucessos em um número de tentativas.

- `núm_sucessos`: O número de sucessos.
- `tentativas`: O número de tentativas.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## BINOM.DIST.RANGE

`BINOM.DIST.RANGE(tentativas; probabilidade_sucesso; núm_sucessos; [núm_sucessos2])`

Retorna a chance de os sucessos caírem em um intervalo.

- `tentativas`: O número de tentativas.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.
- `núm_sucessos`: O número de sucessos.
- `núm_sucessos2` (opcional): O maior número de sucessos a contar (igual ao menor, se omitido).

## BINOM.INV

`BINOM.INV(tentativas; probabilidade_sucesso; alfa)`

Retorna o menor número de sucessos cuja chance acumulada atinge um valor.

- `tentativas`: O número de tentativas.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.

## BINOMDIST

`BINOMDIST(núm_sucessos; tentativas; probabilidade_sucesso; cumulativo)`

Retorna a chance de um número de sucessos em um número de tentativas.

- `núm_sucessos`: O número de sucessos.
- `tentativas`: O número de tentativas.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## CHIDIST

`CHIDIST(x; gl)`

Retorna a proporção da curva qui-quadrado acima de x.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## CHIINV

`CHIINV(probabilidade; gl)`

Retorna o valor de x correspondente a uma proporção da curva qui-quadrado, contada a partir da direita.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## CHISQ.DIST

`CHISQ.DIST(x; gl; cumulativo)`

Retorna o valor da curva qui-quadrado em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## CHISQ.DIST.RT

`CHISQ.DIST.RT(x; gl)`

Retorna a proporção da curva qui-quadrado acima de x.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## CHISQ.INV

`CHISQ.INV(probabilidade; gl)`

Retorna o valor de x correspondente a uma proporção da curva qui-quadrado, contada a partir da esquerda.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## CHISQ.INV.RT

`CHISQ.INV.RT(probabilidade; gl)`

Retorna o valor de x correspondente a uma proporção da curva qui-quadrado, contada a partir da direita.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## CHISQ.TEST

`CHISQ.TEST(intervalo_real; intervalo_esperado)`

Retorna a chance de as contagens diferirem das contagens esperadas apenas por acaso.

- `intervalo_real`: O intervalo de células com as contagens obtidas.
- `intervalo_esperado`: O intervalo de células com as contagens esperadas, do mesmo tamanho das contagens obtidas.

## CHITEST

`CHITEST(intervalo_real; intervalo_esperado)`

Retorna a chance de as contagens diferirem das contagens esperadas apenas por acaso.

- `intervalo_real`: O intervalo de células com as contagens obtidas.
- `intervalo_esperado`: O intervalo de células com as contagens esperadas, do mesmo tamanho das contagens obtidas.

## CONFIDENCE

`CONFIDENCE(alfa; desv_padrão; tamanho)`

Retorna a margem de um intervalo de confiança para uma média, usando a curva normal.

- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `desv_padrão`: O desvio padrão da distribuição.
- `tamanho`: O tamanho da amostra, um número inteiro igual ou maior que 1.

## CONFIDENCE.NORM

`CONFIDENCE.NORM(alfa; desv_padrão; tamanho)`

Retorna a margem de um intervalo de confiança para uma média, usando a curva normal.

- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `desv_padrão`: O desvio padrão da distribuição.
- `tamanho`: O tamanho da amostra, um número inteiro igual ou maior que 1.

## CONFIDENCE.T

`CONFIDENCE.T(alfa; desv_padrão; tamanho)`

Retorna a margem de um intervalo de confiança para uma média, usando a curva t de Student.

- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `desv_padrão`: O desvio padrão da distribuição.
- `tamanho`: O tamanho da amostra, um número inteiro igual ou maior que 1.

## CORREL

`CORREL(matriz1; matriz2)`

Retorna o quanto dois conjuntos de números variam juntos, de -1 a 1.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## COUNT

`COUNT(valor1; ...)`

Conta os números em uma lista ou em um intervalo de células.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## COUNTA

`COUNTA(valor1; ...)`

Conta as células que não estão vazias.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## COUNTBLANK

`COUNTBLANK(intervalo)`

Conta as células vazias de um intervalo.

- `intervalo`: O intervalo de células a testar.

## COUNTIF

`COUNTIF(intervalo; critérios)`

Conta as células que atendem a uma condição.

- `intervalo`: O intervalo de células a testar.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## COUNTIFS

`COUNTIFS(intervalo_critérios1; critérios1; ...)`

Conta as linhas em que várias condições são todas atendidas.

- `intervalo_critérios1`: O primeiro intervalo de células a testar.
- `critérios1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## COVAR

`COVAR(matriz1; matriz2)`

Retorna o quanto dois conjuntos de números variam juntos.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## COVARIANCE.P

`COVARIANCE.P(matriz1; matriz2)`

Retorna o quanto dois conjuntos de números variam juntos, para uma população.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## COVARIANCE.S

`COVARIANCE.S(matriz1; matriz2)`

Retorna o quanto dois conjuntos de números variam juntos, para uma amostra.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## CRITBINOM

`CRITBINOM(tentativas; probabilidade_sucesso; alfa)`

Retorna o menor número de sucessos cuja chance acumulada atinge um valor.

- `tentativas`: O número de tentativas.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.

## DEVSQ

`DEVSQ(número1; ...)`

Retorna a soma dos quadrados dos desvios dos números em relação à média deles.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## EXPON.DIST

`EXPON.DIST(x; lambda; cumulativo)`

Retorna o valor da curva exponencial em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `lambda`: A taxa da curva, maior que 0.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## EXPONDIST

`EXPONDIST(x; lambda; cumulativo)`

Retorna o valor da curva exponencial em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `lambda`: A taxa da curva, maior que 0.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## F.DIST

`F.DIST(x; gl1; gl2; cumulativo)`

Retorna o valor da curva F em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `gl1`: Os graus de liberdade do numerador, um número inteiro igual ou maior que 1.
- `gl2`: Os graus de liberdade do denominador, um número inteiro igual ou maior que 1.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## F.DIST.RT

`F.DIST.RT(x; gl1; gl2)`

Retorna a proporção da curva F acima de x.

- `x`: O valor de x a usar.
- `gl1`: Os graus de liberdade do numerador, um número inteiro igual ou maior que 1.
- `gl2`: Os graus de liberdade do denominador, um número inteiro igual ou maior que 1.

## F.INV

`F.INV(probabilidade; gl1; gl2)`

Retorna o valor de x correspondente a uma proporção da curva F, contada a partir da esquerda.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl1`: Os graus de liberdade do numerador, um número inteiro igual ou maior que 1.
- `gl2`: Os graus de liberdade do denominador, um número inteiro igual ou maior que 1.

## F.INV.RT

`F.INV.RT(probabilidade; gl1; gl2)`

Retorna o valor de x correspondente a uma proporção da curva F, contada a partir da direita.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl1`: Os graus de liberdade do numerador, um número inteiro igual ou maior que 1.
- `gl2`: Os graus de liberdade do denominador, um número inteiro igual ou maior que 1.

## F.TEST

`F.TEST(matriz1; matriz2)`

Retorna a chance de dois conjuntos de números terem a mesma variância.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## FDIST

`FDIST(x; gl1; gl2)`

Retorna a proporção da curva F acima de x.

- `x`: O valor de x a usar.
- `gl1`: Os graus de liberdade do numerador, um número inteiro igual ou maior que 1.
- `gl2`: Os graus de liberdade do denominador, um número inteiro igual ou maior que 1.

## FINV

`FINV(probabilidade; gl1; gl2)`

Retorna o valor de x correspondente a uma proporção da curva F, contada a partir da direita.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl1`: Os graus de liberdade do numerador, um número inteiro igual ou maior que 1.
- `gl2`: Os graus de liberdade do denominador, um número inteiro igual ou maior que 1.

## FISHER

`FISHER(x)`

Retorna a transformação de Fisher de um número entre -1 e 1.

- `x`: O valor de x a usar.

## FISHERINV

`FISHERINV(y)`

Retorna a inversa da transformação de Fisher.

- `y`: O número cuja inversa da transformação de Fisher será calculada.

## FORECAST

`FORECAST(x; valores_conhecidos_y; valores_conhecidos_x)`

Prevê um valor ao longo da reta de melhor ajuste.

- `x`: O valor de x para o qual será previsto um valor de y.
- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x`: O intervalo de células que contém os valores de x que você já tem.

## FORECAST.LINEAR

`FORECAST.LINEAR(x; valores_conhecidos_y; valores_conhecidos_x)`

Prevê um valor ao longo da reta de melhor ajuste.

- `x`: O valor de x a usar.
- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x`: O intervalo de células que contém os valores de x que você já tem.

## FREQUENCY

`FREQUENCY(matriz_dados; matriz_bins)`

Conta quantos valores caem em cada intervalo e retorna as contagens em uma coluna.

- `matriz_dados`: O intervalo de células com os valores a contar.
- `matriz_bins`: O intervalo de células com os limites dos intervalos em que a contagem será feita.

## FTEST

`FTEST(matriz1; matriz2)`

Retorna a chance de dois conjuntos de números terem a mesma variância.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## GAMMA

`GAMMA(x)`

Retorna a função gama de um número.

- `x`: O valor de x a usar.

## GAMMA.DIST

`GAMMA.DIST(x; alfa; beta; cumulativo)`

Retorna o valor da curva gama em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## GAMMA.INV

`GAMMA.INV(probabilidade; alfa; beta)`

Retorna o valor de x correspondente a uma proporção de uma curva gama.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).

## GAMMADIST

`GAMMADIST(x; alfa; beta; cumulativo)`

Retorna o valor da curva gama em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## GAMMAINV

`GAMMAINV(probabilidade; alfa; beta)`

Retorna o valor de x correspondente a uma proporção de uma curva gama.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).

## GAMMALN

`GAMMALN(x)`

Retorna o logaritmo natural da função gama.

- `x`: O valor de x a usar.

## GAMMALN.PRECISE

`GAMMALN.PRECISE(x)`

Retorna o logaritmo natural da função gama.

- `x`: O valor de x a usar.

## GAUSS

`GAUSS(z)`

Retorna a proporção da curva normal padrão entre a média e z.

- `z`: O valor de z.

## GEOMEAN

`GEOMEAN(número1; ...)`

Retorna a média geométrica de números positivos.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## GROWTH

`GROWTH(valores_conhecidos_y; [valores_conhecidos_x]; [novos_valores_x]; [const])`

Retorna os valores ao longo da curva exponencial de melhor ajuste para novos valores de x.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `novos_valores_x` (opcional): Os valores de x para os quais serão retornados os valores de y (os valores de x conhecidos, se omitido).
- `const` (opcional): Verdadeiro ou omitido para ajustar uma linha que não precisa passar por zero; falso para fazê-la passar por zero.

## HARMEAN

`HARMEAN(número1; ...)`

Retorna a média harmônica de números positivos.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## HYPGEOM.DIST

`HYPGEOM.DIST(amostra_sucessos; núm_amostra; população_sucessos; núm_população; cumulativo)`

Retorna a chance de tirar um número de sucessos sem devolver os itens.

- `amostra_sucessos`: O número de sucessos na amostra.
- `núm_amostra`: O número de itens sorteados.
- `população_sucessos`: O número de sucessos em toda a população.
- `núm_população`: O número de itens em toda a população.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## HYPGEOMDIST

`HYPGEOMDIST(amostra_sucessos; núm_amostra; população_sucessos; núm_população)`

Retorna a chance de tirar um número de sucessos sem devolver os itens.

- `amostra_sucessos`: O número de sucessos na amostra.
- `núm_amostra`: O número de itens sorteados.
- `população_sucessos`: O número de sucessos em toda a população.
- `núm_população`: O número de itens em toda a população.

## INTERCEPT

`INTERCEPT(valores_conhecidos_y; valores_conhecidos_x)`

Retorna o ponto em que a reta de melhor ajuste cruza o eixo vertical.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x`: O intervalo de células que contém os valores de x que você já tem.

## KURT

`KURT(número1; ...)`

Retorna o quanto um conjunto de números é pontiagudo ou achatado em comparação com uma curva normal.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## LARGE

`LARGE(matriz; k)`

Retorna o k-ésimo maior número de um conjunto.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `k`: A posição a retornar, contando a partir do maior: 1 é o maior.

## LINEST

`LINEST(valores_conhecidos_y; [valores_conhecidos_x]; [const]; [estatística])`

Ajusta uma reta ou um plano aos dados pelo método dos mínimos quadrados e retorna os números como uma linha.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.
- `estatística` (opcional): Verdadeiro para retornar também mais números sobre o ajuste; falso ou omitido para retornar somente a reta.

## LOGEST

`LOGEST(valores_conhecidos_y; [valores_conhecidos_x]; [const]; [estatística])`

Ajusta uma curva exponencial aos dados e retorna os números como uma linha.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.
- `estatística` (opcional): Verdadeiro para retornar também mais números sobre o ajuste; falso ou omitido para retornar somente a curva.

## LOGINV

`LOGINV(probabilidade; média; desv_padrão)`

Retorna o valor de x correspondente a uma proporção de uma curva lognormal.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.

## LOGNORM.DIST

`LOGNORM.DIST(x; média; desv_padrão; cumulativo)`

Retorna o valor da curva lognormal em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## LOGNORM.INV

`LOGNORM.INV(probabilidade; média; desv_padrão)`

Retorna o valor de x correspondente a uma proporção de uma curva lognormal.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.

## LOGNORMDIST

`LOGNORMDIST(x; média; desv_padrão)`

Retorna a proporção acumulada da curva lognormal até x.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.

## MAXA

`MAXA(valor1; ...)`

Retorna o maior valor, contando texto como zero e verdadeiro como um.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MAXIFS

`MAXIFS(intervalo_máximo; intervalo_critérios1; critérios1; ...)`

Retorna o maior número entre as células que atendem a várias condições.

- `intervalo_máximo`: As células das quais será obtido o maior valor.
- `intervalo_critérios1`: O primeiro intervalo de células a testar.
- `critérios1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MEDIAN

`MEDIAN(número1; ...)`

Retorna o número central de um conjunto.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MINA

`MINA(valor1; ...)`

Retorna o menor valor, contando texto como zero e verdadeiro como um.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MINIFS

`MINIFS(intervalo_mínimo; intervalo_critérios1; critérios1; ...)`

Retorna o menor número entre as células que atendem a várias condições.

- `intervalo_mínimo`: As células das quais será obtido o menor valor.
- `intervalo_critérios1`: O primeiro intervalo de células a testar.
- `critérios1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MODE

`MODE(número1; ...)`

Retorna o número que aparece com mais frequência.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MODE.MULT

`MODE.MULT(número1; ...)`

Retorna uma coluna com os números que aparecem com mais frequência em um conjunto.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## MODE.SNGL

`MODE.SNGL(número1; ...)`

Retorna o número que aparece com mais frequência em um conjunto.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## NEGBINOM.DIST

`NEGBINOM.DIST(núm_falhas; núm_sucessos; probabilidade_sucesso; cumulativo)`

Retorna a chance de um número de falhas antes de um número de sucessos.

- `núm_falhas`: O número de falhas.
- `núm_sucessos`: O número de sucessos.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## NEGBINOMDIST

`NEGBINOMDIST(núm_falhas; núm_sucessos; probabilidade_sucesso)`

Retorna a chance de um número de falhas antes de um número de sucessos.

- `núm_falhas`: O número de falhas.
- `núm_sucessos`: O número de sucessos.
- `probabilidade_sucesso`: A chance de sucesso em cada tentativa, de 0 a 1.

## NORM.DIST

`NORM.DIST(x; média; desv_padrão; cumulativo)`

Retorna o valor da curva normal em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## NORM.INV

`NORM.INV(probabilidade; média; desv_padrão)`

Retorna o valor de x correspondente a uma proporção de uma curva normal.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.

## NORM.S.DIST

`NORM.S.DIST(z; cumulativo)`

Retorna o valor da curva normal padrão em z ou a proporção acumulada até z.

- `z`: O valor de z.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## NORM.S.INV

`NORM.S.INV(probabilidade)`

Retorna o valor de z correspondente a uma proporção da curva normal padrão.

- `probabilidade`: Uma probabilidade, de 0 a 1.

## NORMDIST

`NORMDIST(x; média; desv_padrão; cumulativo)`

Retorna o valor da curva normal em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## NORMINV

`NORMINV(probabilidade; média; desv_padrão)`

Retorna o valor de x correspondente a uma proporção de uma curva normal.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.

## NORMSDIST

`NORMSDIST(z)`

Retorna a proporção acumulada da curva normal padrão até z.

- `z`: O valor de z.

## NORMSINV

`NORMSINV(probabilidade)`

Retorna o valor de z correspondente a uma proporção da curva normal padrão.

- `probabilidade`: Uma probabilidade, de 0 a 1.

## PEARSON

`PEARSON(matriz1; matriz2)`

Retorna o quanto dois conjuntos de números variam juntos, de -1 a 1.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## PERCENTILE

`PERCENTILE(matriz; k)`

Retorna o valor abaixo do qual cai uma proporção k dos números.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `k`: A proporção dos valores, de 0 a 1, para a qual será retornado o valor.

## PERCENTILE.EXC

`PERCENTILE.EXC(matriz; k)`

Retorna o valor abaixo do qual cai uma proporção k dos números, excluindo 0 e 1.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `k`: A posição na ordem, contando a partir de 1.

## PERCENTILE.INC

`PERCENTILE.INC(matriz; k)`

Retorna o valor abaixo do qual cai uma proporção k dos números, de 0 a 1.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `k`: A posição na ordem, contando a partir de 1.

## PERCENTRANK

`PERCENTRANK(matriz; x; [precisão])`

Retorna a posição de um valor em um conjunto de números como uma proporção de 0 a 1.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `x`: O valor de x a usar.
- `precisão` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PERCENTRANK.EXC

`PERCENTRANK.EXC(matriz; x; [precisão])`

Retorna a posição de um valor em um conjunto de números como uma proporção, excluindo 0 e 1.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `x`: O valor de x a usar.
- `precisão` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PERCENTRANK.INC

`PERCENTRANK.INC(matriz; x; [precisão])`

Retorna a posição de um valor em um conjunto de números como uma proporção de 0 a 1.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `x`: O valor de x a usar.
- `precisão` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PERMUTATIONA

`PERMUTATIONA(número; número_escolhido)`

Retorna o número de escolhas ordenadas quando um item pode ser escolhido mais de uma vez.

- `número`: O número a ser processado.
- `número_escolhido`: Quantos itens escolher em cada grupo.

## PHI

`PHI(x)`

Retorna a altura da curva normal padrão em x.

- `x`: O valor de x a usar.

## POISSON

`POISSON(x; média; cumulativo)`

Retorna a chance de um número de eventos, a partir de quantos se espera.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## POISSON.DIST

`POISSON.DIST(x; média; cumulativo)`

Retorna a chance de um número de eventos, a partir de quantos se espera.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## PROB

`PROB(intervalo_x; intervalo_prob; limite_inferior; [limite_superior])`

Retorna a chance de os valores caírem entre dois limites.

- `intervalo_x`: O intervalo de células com os valores de x.
- `intervalo_prob`: As chances correspondentes a cada valor de x, cada uma de 0 a 1, com soma igual a 1.
- `limite_inferior`: O limite inferior do intervalo.
- `limite_superior` (opcional): O limite superior do intervalo (somente o limite inferior, se omitido).

## QUARTILE

`QUARTILE(matriz; quarto)`

Retorna os pontos de quartil de um conjunto de números.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `quarto`: Qual quartil: 0 o menor, 1 o primeiro quarto, 2 a mediana, 3 o terceiro quarto, 4 o maior.

## QUARTILE.EXC

`QUARTILE.EXC(matriz; quarto)`

Retorna os pontos de quartil de um conjunto de números, sem considerar as extremidades.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `quarto`: Qual quartil: 0 o menor, 1 o primeiro quarto, 2 a mediana, 3 o terceiro quarto, 4 o maior.

## QUARTILE.INC

`QUARTILE.INC(matriz; quarto)`

Retorna os pontos de quartil de um conjunto de números, do menor ao maior.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `quarto`: Qual quartil: 0 o menor, 1 o primeiro quarto, 2 a mediana, 3 o terceiro quarto, 4 o maior.

## RANK

`RANK(número; ref; [ordem])`

Retorna a posição de um número em uma lista quando a lista é ordenada.

- `número`: O número a ser processado.
- `ref`: Uma célula ou intervalo de células.
- `ordem` (opcional): 0 ou omitido coloca o maior primeiro; qualquer outro número coloca o menor primeiro.

## RANK.AVG

`RANK.AVG(número; ref; [ordem])`

Retorna a posição de um número em uma lista e a média das posições quando há empates.

- `número`: O número a ser processado.
- `ref`: Uma célula ou intervalo de células.
- `ordem` (opcional): 0 ou omitido coloca o maior primeiro; qualquer outro número coloca o menor primeiro.

## RANK.EQ

`RANK.EQ(número; ref; [ordem])`

Retorna a posição de um número em uma lista e a melhor posição quando há empates.

- `número`: O número a ser processado.
- `ref`: Uma célula ou intervalo de células.
- `ordem` (opcional): 0 ou omitido coloca o maior primeiro; qualquer outro número coloca o menor primeiro.

## RSQ

`RSQ(valores_conhecidos_y; valores_conhecidos_x)`

Retorna o quanto uma reta se ajusta bem a dois conjuntos de números.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x`: O intervalo de células que contém os valores de x que você já tem.

## SKEW

`SKEW(número1; ...)`

Retorna o quanto um conjunto de números é assimétrico em torno da média, para uma amostra.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## SKEW.P

`SKEW.P(número1; ...)`

Retorna o quanto um conjunto de números é assimétrico em torno da média, para uma população inteira.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## SLOPE

`SLOPE(valores_conhecidos_y; valores_conhecidos_x)`

Retorna a inclinação da reta que melhor se ajusta a dois conjuntos de números.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x`: O intervalo de células que contém os valores de x que você já tem.

## SMALL

`SMALL(matriz; k)`

Retorna o k-ésimo menor número de um conjunto.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `k`: A posição a retornar, contando a partir do menor: 1 é o menor.

## STANDARDIZE

`STANDARDIZE(x; média; desv_padrão)`

Retorna a quantos desvios padrão um valor está da média.

- `x`: O valor de x a usar.
- `média`: A média da distribuição.
- `desv_padrão`: O desvio padrão da distribuição.

## STDEV

`STDEV(número1; ...)`

Estima a dispersão de um grupo inteiro a partir de uma amostra dele.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEV.P

`STDEV.P(número1; ...)`

Retorna a dispersão de um grupo inteiro.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEV.S

`STDEV.S(número1; ...)`

Estima a dispersão de um grupo inteiro a partir de uma amostra dele.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEVA

`STDEVA(valor1; ...)`

Retorna o desvio padrão de uma amostra, contando texto como 0 e verdadeiro como 1.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEVP

`STDEVP(número1; ...)`

Retorna a dispersão de um grupo inteiro.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STDEVPA

`STDEVPA(valor1; ...)`

Retorna o desvio padrão de uma população, contando texto como 0 e verdadeiro como 1.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## STEYX

`STEYX(valores_conhecidos_y; valores_conhecidos_x)`

Retorna o erro padrão dos valores de y previstos pela reta ajustada.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x`: O intervalo de células que contém os valores de x que você já tem.

## SUMIF

`SUMIF(intervalo; critérios; [intervalo_soma])`

Soma as células que atendem a uma condição.

- `intervalo`: O intervalo de células a testar.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".
- `intervalo_soma` (opcional): As células a somar (as células testadas, se omitido).

## SUMIFS

`SUMIFS(intervalo_soma; intervalo_critérios1; critérios1; ...)`

Soma as células que atendem a várias condições.

- `intervalo_soma`: As células a somar (as células testadas, se omitido).
- `intervalo_critérios1`: O primeiro intervalo de células a testar.
- `critérios1`: O que procurar no primeiro intervalo de células: um número, um texto ou um teste, como ">5".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## T.DIST

`T.DIST(x; gl; cumulativo)`

Retorna o valor da curva t de Student em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## T.DIST.2T

`T.DIST.2T(x; gl)`

Retorna a proporção da curva t de Student fora do intervalo entre -x e x.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## T.DIST.RT

`T.DIST.RT(x; gl)`

Retorna a proporção da curva t de Student acima de x.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## T.INV

`T.INV(probabilidade; gl)`

Retorna o valor de x correspondente a uma proporção da curva t de Student, contada a partir da esquerda.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## T.INV.2T

`T.INV.2T(probabilidade; gl)`

Retorna o valor de x correspondente a uma proporção da curva t de Student, dividida entre as duas caudas.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## T.TEST

`T.TEST(matriz1; matriz2; caudas; tipo)`

Retorna a chance de as médias de dois conjuntos de números serem iguais.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.
- `caudas`: 1 para uma cauda da curva, 2 para as duas.
- `tipo`: Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## TDIST

`TDIST(x; gl; caudas)`

Retorna a proporção da curva t de Student em uma cauda ou nas duas.

- `x`: O valor de x a usar.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.
- `caudas`: 1 para uma cauda da curva, 2 para as duas.

## TINV

`TINV(probabilidade; gl)`

Retorna o valor de x correspondente a uma proporção da curva t de Student, dividida entre as duas caudas.

- `probabilidade`: Uma probabilidade, de 0 a 1.
- `gl`: Os graus de liberdade, um número inteiro igual ou maior que 1.

## TREND

`TREND(valores_conhecidos_y; [valores_conhecidos_x]; [novos_valores_x]; [const])`

Retorna os valores ao longo da reta de melhor ajuste para novos valores de x.

- `valores_conhecidos_y`: O intervalo de células que contém os valores de y que você já tem.
- `valores_conhecidos_x` (opcional): O intervalo de células que contém os valores de x que você já tem.
- `novos_valores_x` (opcional): Os valores de x para os quais serão retornados os valores de y (os valores de x conhecidos, se omitido).
- `const` (opcional): Verdadeiro ou omitido para ajustar uma reta que não precisa passar por zero; falso para fazê-la passar por zero.

## TRIMMEAN

`TRIMMEAN(matriz; percentual)`

Retorna a média de um conjunto de números depois de descartar uma proporção dos valores mais altos e mais baixos.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `percentual`: A proporção dos valores, de 0 até menos de 1, a excluir nas duas extremidades.

## TTEST

`TTEST(matriz1; matriz2; caudas; tipo)`

Retorna a chance de as médias de dois conjuntos de números serem iguais.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.
- `caudas`: 1 para uma cauda da curva, 2 para as duas.
- `tipo`: Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## VAR

`VAR(número1; ...)`

Estima a variância de um grupo inteiro a partir de uma amostra dele.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VAR.P

`VAR.P(número1; ...)`

Retorna a variância de um grupo inteiro.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VAR.S

`VAR.S(número1; ...)`

Estima a variância de um grupo inteiro a partir de uma amostra dele.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VARA

`VARA(valor1; ...)`

Retorna a variância de uma amostra, contando texto como 0 e verdadeiro como 1.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VARP

`VARP(número1; ...)`

Retorna a variância de um grupo inteiro.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VARPA

`VARPA(valor1; ...)`

Retorna a variância de uma população, contando texto como 0 e verdadeiro como 1.

- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## WEIBULL

`WEIBULL(x; alfa; beta; cumulativo)`

Retorna o valor da curva de Weibull em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## WEIBULL.DIST

`WEIBULL.DIST(x; alfa; beta; cumulativo)`

Retorna o valor da curva de Weibull em x ou a proporção acumulada até x.

- `x`: O valor de x a usar.
- `alfa`: A chance de erro que você aceita, como 0,05 para um nível de confiança de 95 por cento.
- `beta`: Um número que molda a curva (a escala da curva, nas curvas gama e Weibull).
- `cumulativo`: Verdadeiro para retornar o total acumulado até este ponto; falso para retornar o valor somente neste ponto.

## Z.TEST

`Z.TEST(matriz; x; [sigma])`

Retorna a chance unilateral de a média de uma amostra estar acima de um valor.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `x`: O valor de x a usar.
- `sigma` (opcional): O desvio padrão de toda a população (o da amostra, se omitido).

## ZTEST

`ZTEST(matriz; x; [sigma])`

Retorna a chance unilateral de a média de uma amostra estar acima de um valor.

- `matriz`: Um intervalo de células ou uma lista de valores entre chaves.
- `x`: O valor de x a usar.
- `sigma` (opcional): O desvio padrão de toda a população (o da amostra, se omitido).
