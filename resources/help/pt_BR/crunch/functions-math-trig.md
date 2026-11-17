# Funções matemáticas e trigonométricas
<!-- keywords: funções, ABS, ACOS, ACOSH, ACOT, ACOTH, AGREGAR, ARÁBICO, ASEN, ASENH, ATAN, ATAN2, ATANH, BASE, TETO, TETO.MAT, TETO.PRECISO, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, GRAUS, PAR, EXP, FATORIAL, FATDUPLO, PISO, PISO.MAT, PISO.PRECISO, MDC, INT, TETO.ISO, MMC, LN, LOG, LOG10, MÁX, MDETERM, MÍN, MINVERSA, MMULT, MOD, MARRED, MULTINOMIAL, MUNIT, ÍMPAR, PERMUT, PI, POTÊNCIA, MULT, QUOCIENTE, RADIANOS, ALEATÓRIO, ALEATÓRIOENTRE, ROMANO, ARRED, ARREDONDAR.PARA.BAIXO, ARREDONDAR.PARA.CIMA, SEC, SECH, SEQUÊNCIA, SOMASÉRIE, SINAL, SEN, SENH, RAIZ, RAIZPI, SUBTOTAL, SOMA, SOMARPRODUTO, SOMAQUAD, SOMAX2M2, SOMAX2S2, SOMAXMY2, TAN, TANH, TRUNCAR -->

Estas são as funções matemáticas e trigonométricas que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros forem necessários após o último. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica sob a célula mostra os argumentos dela, e um clique no nome da função na dica abre a entrada correspondente aqui.

## ABS

`ABS(número)`

Retorna um número sem o sinal.

- `número`: O número a ser processado.

## ACOS

`ACOS(número)`

Retorna o ângulo, em radianos, cujo cosseno é o número informado.

- `número`: O número a ser processado.

## ACOSH

`ACOSH(número)`

Retorna o cosseno hiperbólico inverso de um número.

- `número`: O número a ser processado.

## ACOT

`ACOT(número)`

Retorna a cotangente inversa de um número, como um ângulo em radianos.

- `número`: O número a ser processado.

## ACOTH

`ACOTH(número)`

Retorna a cotangente hiperbólica inversa de um número.

- `número`: O número a ser processado.

## AGREGAR

`AGREGAR(núm_função, opções, ref1, ...)`

Faz um cálculo sobre uma lista, como uma soma ou uma mediana, e pode ignorar linhas ocultas e erros.

- `núm_função`: Qual cálculo fazer: 1 média, 2 contagem, 3 contagem de não vazias, 4 máximo, 5 mínimo, 6 produto, 7 e 8 desvio padrão, 9 soma, 10 e 11 variância.
- `opções`: O que ignorar: 0 totais aninhados, 1 também linhas ocultas, 2 também erros, 3 ambos, 4 a 7 para manter os totais aninhados.
- `ref1`: A primeira célula ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## ARÁBICO

`ARÁBICO(texto)`

Lê algarismos romanos em texto como um número.

- `texto`: O texto a ser processado, ou uma célula que o contenha.

## ASEN

`ASEN(número)`

Retorna o ângulo, em radianos, cujo seno é o número informado.

- `número`: O número a ser processado.

## ASENH

`ASENH(número)`

Retorna o seno hiperbólico inverso de um número.

- `número`: O número a ser processado.

## ATAN

`ATAN(número)`

Retorna o ângulo, em radianos, cuja tangente é o número informado.

- `número`: O número a ser processado.

## ATAN2

`ATAN2(núm_x, núm_y)`

Retorna o ângulo, em radianos, do ponto de coordenadas x e y.

- `núm_x`: O valor de x.
- `núm_y`: O valor de y.

## ATANH

`ATANH(número)`

Retorna a tangente hiperbólica inversa de um número.

- `número`: O número a ser processado.

## BASE

`BASE(número, base, [comprimento_mínimo])`

Escreve um número em outra base, de 2 a 36, como texto.

- `número`: O número a ser processado.
- `base`: A base do número: de 2 a 36.
- `comprimento_mínimo` (opcional): O menor número de caracteres do resultado; resultados menores são completados com zeros à esquerda.

## TETO

`TETO(número, [significância])`

Arredonda um número para cima até o múltiplo mais próximo.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.

## TETO.MAT

`TETO.MAT(número, [significância], [modo])`

Arredonda um número para cima até o múltiplo mais próximo, com uma opção para números negativos.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.
- `modo` (opcional): Como os números negativos são arredondados: 0 ou omitido é o modo usual, e qualquer outro número inverte esse comportamento.

## TETO.PRECISO

`TETO.PRECISO(número, [significância])`

Arredonda um número para cima até o múltiplo mais próximo, qualquer que seja o sinal do múltiplo.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.

## COMBIN

`COMBIN(número, núm_escolhido)`

Conta as maneiras de escolher itens de um grupo quando a ordem não importa.

- `número`: O número a ser processado.
- `núm_escolhido`: Quantos itens escolher em cada grupo.

## COMBINA

`COMBINA(número, núm_escolhido)`

Retorna o número de maneiras de escolher itens quando um item pode ser escolhido mais de uma vez.

- `número`: O número a ser processado.
- `núm_escolhido`: Quantos itens escolher em cada grupo.

## COS

`COS(número)`

Retorna o cosseno de um ângulo em radianos.

- `número`: O número a ser processado.

## COSH

`COSH(número)`

Retorna o cosseno hiperbólico de um número.

- `número`: O número a ser processado.

## COT

`COT(ângulo)`

Retorna a cotangente de um ângulo em radianos.

- `ângulo`: O ângulo, em radianos.

## COTH

`COTH(número)`

Retorna a cotangente hiperbólica de um número.

- `número`: O número a ser processado.

## CSC

`CSC(ângulo)`

Retorna a cossecante de um ângulo em radianos.

- `ângulo`: O ângulo, em radianos.

## CSCH

`CSCH(número)`

Retorna a cossecante hiperbólica de um número.

- `número`: O número a ser processado.

## DECIMAL

`DECIMAL(texto, base)`

Lê um texto escrito em uma base de 2 a 36 como um número.

- `texto`: O texto a ser processado, ou uma célula que o contenha.
- `base`: A base do número: de 2 a 36.

## GRAUS

`GRAUS(ângulo)`

Converte radianos em graus.

- `ângulo`: O ângulo, em radianos.

## PAR

`PAR(número)`

Arredonda um número para cima até o número inteiro par mais próximo.

- `número`: O número a ser processado.

## EXP

`EXP(número)`

Retorna e elevado a uma potência.

- `número`: O número a ser processado.

## FATORIAL

`FATORIAL(número)`

Retorna o fatorial de um número.

- `número`: O número a ser processado.

## FATDUPLO

`FATDUPLO(número)`

Retorna o fatorial duplo de um número: o produto de todos os números de dois em dois, até 1 ou 2.

- `número`: O número a ser processado.

## PISO

`PISO(número, [significância])`

Arredonda um número para baixo até o múltiplo mais próximo.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.

## PISO.MAT

`PISO.MAT(número, [significância], [modo])`

Arredonda um número para baixo até o múltiplo mais próximo, com uma opção para números negativos.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.
- `modo` (opcional): Como os números negativos são arredondados: 0 ou omitido é o modo usual, e qualquer outro número inverte esse comportamento.

## PISO.PRECISO

`PISO.PRECISO(número, [significância])`

Arredonda um número para baixo até o múltiplo mais próximo, qualquer que seja o sinal do múltiplo.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.

## MDC

`MDC(número1, ...)`

Retorna o maior número inteiro que divide todos os números.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## INT

`INT(número)`

Arredonda um número para baixo, até o número inteiro abaixo dele.

- `número`: O número a ser processado.

## TETO.ISO

`TETO.ISO(número, [significância])`

Arredonda um número para cima até o múltiplo mais próximo, qualquer que seja o sinal do múltiplo.

- `número`: O número a ser processado.
- `significância` (opcional): O número cujos múltiplos serão usados no arredondamento.

## MMC

`MMC(número1, ...)`

Retorna o menor número inteiro que todos os números dividem.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## LN

`LN(número)`

Retorna o logaritmo natural de um número.

- `número`: O número a ser processado.

## LOG

`LOG(número, [base])`

Retorna o logaritmo de um número em uma base (10, se nenhuma for informada).

- `número`: O número a ser processado.
- `base` (opcional): A base do logaritmo (10, se omitida).

## LOG10

`LOG10(número)`

Retorna o logaritmo de um número na base 10.

- `número`: O número a ser processado.

## MÁX

`MÁX(número1, ...)`

Retorna o maior número.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## MDETERM

`MDETERM(matriz)`

Retorna o determinante de um intervalo quadrado de números.

- `matriz`: Um intervalo de células, ou uma lista de valores escrita entre chaves.

## MÍN

`MÍN(número1, ...)`

Retorna o menor número.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## MINVERSA

`MINVERSA(matriz)`

Retorna a matriz inversa de um intervalo quadrado de números.

- `matriz`: Um intervalo de células, ou uma lista de valores escrita entre chaves.

## MMULT

`MMULT(matriz1, matriz2)`

Retorna o produto de dois intervalos de números, como matrizes.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `matriz2`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## MOD

`MOD(número, divisor)`

Retorna o resto da divisão de um número por outro.

- `número`: O número a ser processado.
- `divisor`: O número pelo qual dividir.

## MARRED

`MARRED(número, múltiplo)`

Arredonda um número até o múltiplo mais próximo.

- `número`: O número a ser processado.
- `múltiplo`: O número cujos múltiplos serão usados no arredondamento.

## MULTINOMIAL

`MULTINOMIAL(número1, ...)`

Retorna o fatorial da soma dos números dividido pelo produto dos fatoriais deles.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## MUNIT

`MUNIT(dimensão)`

Retorna a matriz identidade de um tamanho.

- `dimensão`: O tamanho do intervalo quadrado: quantas linhas e colunas ele tem.

## ÍMPAR

`ÍMPAR(número)`

Arredonda um número para cima até o número inteiro ímpar mais próximo.

- `número`: O número a ser processado.

## PERMUT

`PERMUT(número, núm_escolhido)`

Conta as maneiras de escolher itens de um grupo quando a ordem importa.

- `número`: O número a ser processado.
- `núm_escolhido`: Quantos itens escolher em cada grupo.

## PI

`PI()`

Retorna o número pi.

## POTÊNCIA

`POTÊNCIA(número, potência)`

Eleva um número a uma potência.

- `número`: O número a ser processado.
- `potência`: A potência à qual elevar o número.

## MULT

`MULT(número1, ...)`

Multiplica números.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## QUOCIENTE

`QUOCIENTE(numerador, denominador)`

Retorna a parte inteira de uma divisão.

- `numerador`: O número a ser dividido.
- `denominador`: O número pelo qual dividir.

## RADIANOS

`RADIANOS(ângulo)`

Converte graus em radianos.

- `ângulo`: O ângulo, em graus.

## ALEATÓRIO

`ALEATÓRIO()`

Retorna um número aleatório de 0 até, mas não incluindo, 1.

## ALEATÓRIOMATRIZ

`ALEATÓRIOMATRIZ([linhas], [colunas], [mín], [máx], [número_inteiro])`

Retorna um intervalo de números aleatórios.

- `linhas` (opcional): Quantas linhas o resultado terá: positivo para baixo, negativo para cima.
- `colunas` (opcional): Quantas colunas o resultado terá de largura (1, se omitido).
- `mín` (opcional): O menor número que o resultado pode ter (0, se omitido).
- `máx` (opcional): O maior número que o resultado pode ter (1, se omitido).
- `número_inteiro` (opcional): Verdadeiro para números inteiros; falso ou omitido para números com decimais.

## ALEATÓRIOENTRE

`ALEATÓRIOENTRE(inferior, superior)`

Retorna um número inteiro aleatório entre dois números.

- `inferior`: O menor valor que o resultado pode ter.
- `superior`: O maior valor que o resultado pode ter.

## ROMANO

`ROMANO(número, [forma])`

Escreve um número inteiro de 0 a 3999 como algarismos romanos em texto.

- `número`: O número a ser processado.
- `forma` (opcional): Quão curtos são os algarismos romanos: 0 ou omitido para a forma clássica, até 4 para a mais curta.

## ARRED

`ARRED(número, núm_dígitos)`

Arredonda um número para um número de dígitos.

- `número`: O número a ser processado.
- `núm_dígitos`: Quantas casas decimais arredondar: positivo para a direita da vírgula, 0 para um número inteiro, negativo para a esquerda.

## ARREDONDAR.PARA.BAIXO

`ARREDONDAR.PARA.BAIXO(número, núm_dígitos)`

Arredonda um número para baixo, em direção a zero, até um número de dígitos.

- `número`: O número a ser processado.
- `núm_dígitos`: Quantas casas decimais arredondar: positivo para a direita da vírgula, 0 para um número inteiro, negativo para a esquerda.

## ARREDONDAR.PARA.CIMA

`ARREDONDAR.PARA.CIMA(número, núm_dígitos)`

Arredonda um número para cima, para longe de zero, até um número de dígitos.

- `número`: O número a ser processado.
- `núm_dígitos`: Quantas casas decimais arredondar: positivo para a direita da vírgula, 0 para um número inteiro, negativo para a esquerda.

## SEC

`SEC(ângulo)`

Retorna a secante de um ângulo em radianos.

- `ângulo`: O ângulo, em radianos.

## SECH

`SECH(número)`

Retorna a secante hiperbólica de um número.

- `número`: O número a ser processado.

## SEQUÊNCIA

`SEQUÊNCIA(linhas, [colunas], [início], [passo])`

Retorna um intervalo de números que sobem a partir de um início, de um passo em um passo.

- `linhas`: Quantas linhas o resultado terá: positivo para baixo, negativo para cima.
- `colunas` (opcional): Quantas colunas o resultado terá de largura (1, se omitido).
- `início` (opcional): O primeiro número (1, se omitido).
- `passo` (opcional): Quanto somar a cada número (1, se omitido; para DEGRAU, o número de comparação, 0, se omitido).

## SOMASÉRIE

`SOMASÉRIE(x, n, m, coeficientes)`

Retorna a soma de uma série de potências.

- `x`: O valor de x a ser usado.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro de 0 ou mais.
- `m`: O incremento a somar à potência a cada coeficiente.
- `coeficientes`: Os números pelos quais multiplicar cada potência de x.

## SINAL

`SINAL(número)`

Retorna 1 para um número positivo, 0 para zero e -1 para um número negativo.

- `número`: O número a ser processado.

## SEN

`SEN(número)`

Retorna o seno de um ângulo em radianos.

- `número`: O número a ser processado.

## SENH

`SENH(número)`

Retorna o seno hiperbólico de um número.

- `número`: O número a ser processado.

## RAIZ

`RAIZ(número)`

Retorna a raiz quadrada de um número.

- `número`: O número a ser processado.

## RAIZPI

`RAIZPI(número)`

Retorna a raiz quadrada de um número multiplicado por pi.

- `número`: O número a ser processado.

## SUBTOTAL

`SUBTOTAL(núm_função, ref1, ...)`

Retorna um total, uma média, uma contagem ou outro valor para um intervalo de células.

- `núm_função`: Qual cálculo fazer: 1 média, 2 contagem, 3 contagem de não vazias, 4 máximo, 5 mínimo, 6 produto, 7 e 8 desvio padrão, 9 soma, 10 e 11 variância.
- `ref1`: A primeira célula ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## SOMA

`SOMA(número1, ...)`

Soma números e os números em intervalos de células.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## SOMARPRODUTO

`SOMARPRODUTO(matriz1, ...)`

Multiplica os números de intervalos de mesmo tamanho, posição por posição, e soma os produtos.

- `matriz1`: O primeiro intervalo de células ou lista de valores.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## SOMAQUAD

`SOMAQUAD(número1, ...)`

Soma os quadrados dos números.

- `número1`: O primeiro número ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do argumento anterior.

## SOMAX2M2

`SOMAX2M2(matriz_x, matriz_y)`

Soma as diferenças dos quadrados de dois conjuntos de números.

- `matriz_x`: O primeiro intervalo de células ou lista de valores.
- `matriz_y`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## SOMAX2S2

`SOMAX2S2(matriz_x, matriz_y)`

Soma as somas dos quadrados de dois conjuntos de números.

- `matriz_x`: O primeiro intervalo de células ou lista de valores.
- `matriz_y`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## SOMAXMY2

`SOMAXMY2(matriz_x, matriz_y)`

Soma os quadrados das diferenças de dois conjuntos de números.

- `matriz_x`: O primeiro intervalo de células ou lista de valores.
- `matriz_y`: O segundo intervalo de células ou lista de valores, do mesmo tamanho do primeiro.

## TAN

`TAN(número)`

Retorna a tangente de um ângulo em radianos.

- `número`: O número a ser processado.

## TANH

`TANH(número)`

Retorna a tangente hiperbólica de um número.

- `número`: O número a ser processado.

## TRUNCAR

`TRUNCAR(número, [núm_dígitos])`

Corta um número após um número de dígitos, sem arredondar.

- `número`: O número a ser processado.
- `núm_dígitos` (opcional): Quantas casas decimais manter: positivo para a direita da vírgula, 0 para um número inteiro, negativo para a esquerda.
