# Funções matemáticas e trigonométricas
<!-- keywords: funções, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC -->

Estas são as funções matemáticas e trigonométricas que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos outros do último tipo quantos precisar. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece uma pequena sugestão por baixo da célula com os respetivos argumentos; se clicar no nome da função nessa sugestão, abre-se aqui a sua entrada.

## ABS

`ABS(número)`

Devolve um número sem o respetivo sinal.

- `número`: O número a processar.

## ACOS

`ACOS(número)`

Devolve o ângulo, em radianos, cujo cosseno é um determinado valor.

- `número`: O número a processar.

## ACOSH

`ACOSH(número)`

Devolve o cosseno hiperbólico inverso de um número.

- `número`: O número a processar.

## ACOT

`ACOT(número)`

Devolve a cotangente inversa de um número, como um ângulo em radianos.

- `número`: O número a processar.

## ACOTH

`ACOTH(número)`

Devolve a cotangente hiperbólica inversa de um número.

- `número`: O número a processar.

## AGGREGATE

`AGGREGATE(núm_função, opções, ref1, ...)`

Efetua um cálculo sobre uma lista, como uma soma ou uma mediana, e pode ignorar linhas ocultas e erros.

- `núm_função`: Que cálculo efetuar: 1 média, 2 contagem, 3 contagem de não vazios, 4 máximo, 5 mínimo, 6 produto, 7 e 8 desvio padrão, 9 soma, 10 e 11 variância.
- `opções`: O que ignorar: 0 totais aninhados, 1 também linhas ocultas, 2 também erros, 3 ambos, 4 a 7 para manter os totais aninhados.
- `ref1`: A primeira célula ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## ARABIC

`ARABIC(texto)`

Lê numerais romanos num texto como um número.

- `texto`: O texto a processar, ou uma célula que o contenha.

## ASIN

`ASIN(número)`

Devolve o ângulo, em radianos, cujo seno é um determinado valor.

- `número`: O número a processar.

## ASINH

`ASINH(número)`

Devolve o seno hiperbólico inverso de um número.

- `número`: O número a processar.

## ATAN

`ATAN(número)`

Devolve o ângulo, em radianos, cuja tangente é um determinado valor.

- `número`: O número a processar.

## ATAN2

`ATAN2(x_núm, y_núm)`

Devolve o ângulo, em radianos, do ponto de coordenadas x e y.

- `x_núm`: O valor de x.
- `y_núm`: O valor de y.

## ATANH

`ATANH(número)`

Devolve a tangente hiperbólica inversa de um número.

- `número`: O número a processar.

## BASE

`BASE(número, base, [comprimento_mín])`

Escreve um número noutra base, de 2 a 36, como texto.

- `número`: O número a processar.
- `base`: A base do número: de 2 a 36.
- `comprimento_mín` (opcional): O menor número de carateres do resultado; os resultados mais curtos são preenchidos com zeros à esquerda.

## CEILING

`CEILING(número, [significado])`

Arredonda um número por excesso para o múltiplo mais próximo.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.

## CEILING.MATH

`CEILING.MATH(número, [significado], [modo])`

Arredonda um número por excesso para o múltiplo mais próximo, com uma opção para números negativos.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.
- `modo` (opcional): Como arredondar os números negativos: 0 ou omitido é a forma habitual, e qualquer outro número inverte-a.

## CEILING.PRECISE

`CEILING.PRECISE(número, [significado])`

Arredonda um número por excesso para o múltiplo mais próximo, qualquer que seja o sinal do múltiplo.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.

## COMBIN

`COMBIN(número, número_escolhido)`

Conta as formas de escolher itens de um grupo quando a ordem não importa.

- `número`: O número a processar.
- `número_escolhido`: Quantos itens escolher em cada grupo.

## COMBINA

`COMBINA(número, número_escolhido)`

Devolve o número de formas de escolher itens quando um item pode ser escolhido mais do que uma vez.

- `número`: O número a processar.
- `número_escolhido`: Quantos itens escolher em cada grupo.

## COS

`COS(número)`

Devolve o cosseno de um ângulo em radianos.

- `número`: O número a processar.

## COSH

`COSH(número)`

Devolve o cosseno hiperbólico de um número.

- `número`: O número a processar.

## COT

`COT(ângulo)`

Devolve a cotangente de um ângulo em radianos.

- `ângulo`: O ângulo, em radianos.

## COTH

`COTH(número)`

Devolve a cotangente hiperbólica de um número.

- `número`: O número a processar.

## CSC

`CSC(ângulo)`

Devolve a cossecante de um ângulo em radianos.

- `ângulo`: O ângulo, em radianos.

## CSCH

`CSCH(número)`

Devolve a cossecante hiperbólica de um número.

- `número`: O número a processar.

## DECIMAL

`DECIMAL(texto, base)`

Lê um texto escrito numa base de 2 a 36 como um número.

- `texto`: O texto a processar, ou uma célula que o contenha.
- `base`: A base do número: de 2 a 36.

## DEGREES

`DEGREES(ângulo)`

Converte radianos em graus.

- `ângulo`: O ângulo, em radianos.

## EVEN

`EVEN(número)`

Arredonda um número por excesso para o número inteiro par mais próximo.

- `número`: O número a processar.

## EXP

`EXP(número)`

Devolve e elevado a uma potência.

- `número`: O número a processar.

## FACT

`FACT(número)`

Devolve o fatorial de um número.

- `número`: O número a processar.

## FACTDOUBLE

`FACTDOUBLE(número)`

Devolve o fatorial duplo de um número: o produto de cada segundo número, descendo até 1 ou 2.

- `número`: O número a processar.

## FLOOR

`FLOOR(número, [significado])`

Arredonda um número por defeito para o múltiplo mais próximo.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.

## FLOOR.MATH

`FLOOR.MATH(número, [significado], [modo])`

Arredonda um número por defeito para o múltiplo mais próximo, com uma opção para números negativos.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.
- `modo` (opcional): Como arredondar os números negativos: 0 ou omitido é a forma habitual, e qualquer outro número inverte-a.

## FLOOR.PRECISE

`FLOOR.PRECISE(número, [significado])`

Arredonda um número por defeito para o múltiplo mais próximo, qualquer que seja o sinal do múltiplo.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.

## GCD

`GCD(número1, ...)`

Devolve o maior número inteiro que divide todos os números.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## INT

`INT(número)`

Arredonda um número por defeito para o número inteiro inferior mais próximo.

- `número`: O número a processar.

## ISO.CEILING

`ISO.CEILING(número, [significado])`

Arredonda um número por excesso para o múltiplo mais próximo, qualquer que seja o sinal do múltiplo.

- `número`: O número a processar.
- `significado` (opcional): O número cujos múltiplos servem de referência para o arredondamento.

## LCM

`LCM(número1, ...)`

Devolve o menor número inteiro que todos os números dividem.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## LN

`LN(número)`

Devolve o logaritmo natural de um número.

- `número`: O número a processar.

## LOG

`LOG(número, [base])`

Devolve o logaritmo de um número numa base (10, se não for indicada nenhuma).

- `número`: O número a processar.
- `base` (opcional): A base do logaritmo (10, se for omitida).

## LOG10

`LOG10(número)`

Devolve o logaritmo de um número na base 10.

- `número`: O número a processar.

## MAX

`MAX(número1, ...)`

Devolve o maior número.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MDETERM

`MDETERM(matriz)`

Devolve o determinante de um bloco quadrado de números.

- `matriz`: Um bloco de células, ou uma lista de valores escrita entre chavetas.

## MIN

`MIN(número1, ...)`

Devolve o menor número.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MINVERSE

`MINVERSE(matriz)`

Devolve a inversa de um bloco quadrado de números.

- `matriz`: Um bloco de células, ou uma lista de valores escrita entre chavetas.

## MMULT

`MMULT(matriz1, matriz2)`

Devolve o produto de dois blocos de números, como matrizes.

- `matriz1`: O primeiro bloco de células ou lista de valores.
- `matriz2`: O segundo bloco de células ou lista de valores, com o mesmo tamanho do primeiro.

## MOD

`MOD(número, divisor)`

Devolve o resto da divisão de um número por outro.

- `número`: O número a processar.
- `divisor`: O número pelo qual dividir.

## MROUND

`MROUND(número, múltiplo)`

Arredonda um número para o múltiplo mais próximo.

- `número`: O número a processar.
- `múltiplo`: O número cujos múltiplos servem de referência para o arredondamento.

## MULTINOMIAL

`MULTINOMIAL(número1, ...)`

Devolve o fatorial da soma dos números a dividir pelo produto dos respetivos fatoriais.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## MUNIT

`MUNIT(dimensão)`

Devolve a matriz identidade de um tamanho.

- `dimensão`: O tamanho do bloco quadrado: quantas linhas e colunas tem.

## ODD

`ODD(número)`

Arredonda um número por excesso para o número inteiro ímpar mais próximo.

- `número`: O número a processar.

## PERMUT

`PERMUT(número, número_escolhido)`

Conta as formas de escolher itens de um grupo quando a ordem importa.

- `número`: O número a processar.
- `número_escolhido`: Quantos itens escolher em cada grupo.

## PI

`PI()`

Devolve o número pi.

## POWER

`POWER(número, potência)`

Eleva um número a uma potência.

- `número`: O número a processar.
- `potência`: A potência a que elevar o número.

## PRODUCT

`PRODUCT(número1, ...)`

Multiplica números entre si.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## QUOTIENT

`QUOTIENT(numerador, denominador)`

Devolve a parte inteira de uma divisão.

- `numerador`: O número a dividir.
- `denominador`: O número pelo qual dividir.

## RADIANS

`RADIANS(ângulo)`

Converte graus em radianos.

- `ângulo`: O ângulo, em graus.

## RAND

`RAND()`

Devolve um número aleatório de 0 até, mas sem incluir, 1.

## RANDARRAY

`RANDARRAY([linhas], [colunas], [mín], [máx], [número_inteiro])`

Devolve um bloco de números aleatórios.

- `linhas` (opcional): Quantas linhas deslocar: positivo para baixo, negativo para cima.
- `colunas` (opcional): Quantas colunas tem o resultado (1, se for omitido).
- `mín` (opcional): O menor número que o resultado pode ter (0, se for omitido).
- `máx` (opcional): O maior número que o resultado pode ter (1, se for omitido).
- `número_inteiro` (opcional): Verdadeiro para números inteiros; falso ou omitido para números com casas decimais.

## RANDBETWEEN

`RANDBETWEEN(inferior, superior)`

Devolve um número inteiro aleatório entre dois números.

- `inferior`: O menor valor que o resultado pode ter.
- `superior`: O maior valor que o resultado pode ter.

## ROMAN

`ROMAN(número, [forma])`

Escreve um número inteiro de 0 a 3999 como numerais romanos num texto.

- `número`: O número a processar.
- `forma` (opcional): O grau de abreviação dos numerais romanos: 0 ou omitido para a forma clássica, até 4 para a forma mais curta.

## ROUND

`ROUND(número, núm_casas)`

Arredonda um número para um número de casas decimais.

- `número`: O número a processar.
- `núm_casas`: Para quantas casas arredondar: positivo para a direita da vírgula decimal, 0 para um número inteiro, negativo para a esquerda.

## ROUNDDOWN

`ROUNDDOWN(número, núm_casas)`

Arredonda um número por defeito, em direção a zero, para um número de casas decimais.

- `número`: O número a processar.
- `núm_casas`: Para quantas casas arredondar: positivo para a direita da vírgula decimal, 0 para um número inteiro, negativo para a esquerda.

## ROUNDUP

`ROUNDUP(número, núm_casas)`

Arredonda um número por excesso, afastando-se de zero, para um número de casas decimais.

- `número`: O número a processar.
- `núm_casas`: Para quantas casas arredondar: positivo para a direita da vírgula decimal, 0 para um número inteiro, negativo para a esquerda.

## SEC

`SEC(ângulo)`

Devolve a secante de um ângulo em radianos.

- `ângulo`: O ângulo, em radianos.

## SECH

`SECH(número)`

Devolve a secante hiperbólica de um número.

- `número`: O número a processar.

## SEQUENCE

`SEQUENCE(linhas, [colunas], [início], [passo])`

Devolve um bloco de números que contam a partir de um início, com um passo.

- `linhas`: Quantas linhas deslocar: positivo para baixo, negativo para cima.
- `colunas` (opcional): Quantas colunas tem o resultado (1, se for omitido).
- `início` (opcional): O primeiro número (1, se for omitido).
- `passo` (opcional): Quanto somar a cada número (1, se for omitido; no caso de DEC.PARA.BIN, o número com que comparar, 0, se for omitido).

## SERIESSUM

`SERIESSUM(x, n, m, coeficientes)`

Devolve a soma de uma série de potências.

- `x`: O valor de x a utilizar.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou superior a 0.
- `m`: O passo a somar à potência por cada coeficiente.
- `coeficientes`: Os números pelos quais multiplicar cada potência de x.

## SIGN

`SIGN(número)`

Devolve 1 para um número positivo, 0 para zero e -1 para um número negativo.

- `número`: O número a processar.

## SIN

`SIN(número)`

Devolve o seno de um ângulo em radianos.

- `número`: O número a processar.

## SINH

`SINH(número)`

Devolve o seno hiperbólico de um número.

- `número`: O número a processar.

## SQRT

`SQRT(número)`

Devolve a raiz quadrada de um número.

- `número`: O número a processar.

## SQRTPI

`SQRTPI(número)`

Devolve a raiz quadrada de um número multiplicado por pi.

- `número`: O número a processar.

## SUBTOTAL

`SUBTOTAL(núm_função, ref1, ...)`

Devolve um total, média, contagem ou outro valor para um bloco de células.

- `núm_função`: Que cálculo efetuar: 1 média, 2 contagem, 3 contagem de não vazios, 4 máximo, 5 mínimo, 6 produto, 7 e 8 desvio padrão, 9 soma, 10 e 11 variância.
- `ref1`: A primeira célula ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SUM

`SUM(número1, ...)`

Soma números e os números de blocos de células.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SUMPRODUCT

`SUMPRODUCT(matriz1, ...)`

Multiplica os números de blocos do mesmo tamanho, posição a posição, e soma os produtos.

- `matriz1`: O primeiro bloco de células ou lista de valores.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SUMSQ

`SUMSQ(número1, ...)`

Soma os quadrados dos números.

- `número1`: O primeiro número, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SUMX2MY2

`SUMX2MY2(matriz_x, matriz_y)`

Soma as diferenças dos quadrados de dois conjuntos de números.

- `matriz_x`: O primeiro bloco de células ou lista de valores.
- `matriz_y`: O segundo bloco de células ou lista de valores, com o mesmo tamanho do primeiro.

## SUMX2PY2

`SUMX2PY2(matriz_x, matriz_y)`

Soma as somas dos quadrados de dois conjuntos de números.

- `matriz_x`: O primeiro bloco de células ou lista de valores.
- `matriz_y`: O segundo bloco de células ou lista de valores, com o mesmo tamanho do primeiro.

## SUMXMY2

`SUMXMY2(matriz_x, matriz_y)`

Soma os quadrados das diferenças de dois conjuntos de números.

- `matriz_x`: O primeiro bloco de células ou lista de valores.
- `matriz_y`: O segundo bloco de células ou lista de valores, com o mesmo tamanho do primeiro.

## TAN

`TAN(número)`

Devolve a tangente de um ângulo em radianos.

- `número`: O número a processar.

## TANH

`TANH(número)`

Devolve a tangente hiperbólica de um número.

- `número`: O número a processar.

## TRUNC

`TRUNC(número, [núm_casas])`

Corta um número após um número de casas decimais, sem arredondar.

- `número`: O número a processar.
- `núm_casas` (opcional): Para quantas casas truncar: positivo para a direita da vírgula decimal, 0 para um número inteiro, negativo para a esquerda.
