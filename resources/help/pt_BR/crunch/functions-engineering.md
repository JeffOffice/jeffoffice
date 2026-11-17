# Funções de engenharia
<!-- keywords: funções, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX, funções de engenharia, funções engenharia, função, fórmulas, números complexos, conversão de base, bits, binário, hexadecimal, octal, Bessel, erro -->

Estas são as funções de engenharia que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros forem necessários após o último. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra os argumentos dela, e um clique no nome da função na dica abre o seu verbete aqui.

## BESSELI

`BESSELI(x, n)`

Retorna a função de Bessel modificada de primeira espécie.

- `x`: O valor x a ser usado.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou maior que 0.

## BESSELJ

`BESSELJ(x, n)`

Retorna a função de Bessel de primeira espécie.

- `x`: O valor x a ser usado.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou maior que 0.

## BESSELK

`BESSELK(x, n)`

Retorna a função de Bessel modificada de segunda espécie.

- `x`: O valor x a ser usado.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou maior que 0.

## BESSELY

`BESSELY(x, n)`

Retorna a função de Bessel de segunda espécie.

- `x`: O valor x a ser usado.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou maior que 0.

## BIN2DEC

`BIN2DEC(número)`

Converte um número binário em um número decimal.

- `número`: O número a ser processado.

## BIN2HEX

`BIN2HEX(número, [casas])`

Converte um número binário em um número hexadecimal.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## BIN2OCT

`BIN2OCT(número, [casas])`

Converte um número binário em um número octal.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## BITAND

`BITAND(número1, número2)`

Retorna os bits que estão ativados em ambos os dois números inteiros.

- `número1`: O primeiro número, ou intervalo de células.
- `número2`: O segundo número.

## BITLSHIFT

`BITLSHIFT(número, quantidade_deslocamento)`

Desloca os bits de um número inteiro para a esquerda.

- `número`: O número a ser processado.
- `quantidade_deslocamento`: Quantos bits deslocar: números positivos e negativos seguem sentidos opostos.

## BITOR

`BITOR(número1, número2)`

Retorna os bits que estão ativados em um ou no outro dos dois números inteiros.

- `número1`: O primeiro número, ou intervalo de células.
- `número2`: O segundo número.

## BITRSHIFT

`BITRSHIFT(número, quantidade_deslocamento)`

Desloca os bits de um número inteiro para a direita.

- `número`: O número a ser processado.
- `quantidade_deslocamento`: Quantos bits deslocar: números positivos e negativos seguem sentidos opostos.

## BITXOR

`BITXOR(número1, número2)`

Retorna os bits que estão ativados em apenas um dos dois números inteiros, e não em ambos.

- `número1`: O primeiro número, ou intervalo de células.
- `número2`: O segundo número.

## COMPLEX

`COMPLEX(núm_real, núm_i, [sufixo])`

Cria um número complexo a partir de suas partes real e imaginária.

- `núm_real`: A parte real do número complexo.
- `núm_i`: A parte imaginária do número complexo.
- `sufixo` (opcional): i ou j, para marcar a parte imaginária (i, se omitido).

## CONVERT

`CONVERT(número, unidade_de, unidade_para)`

Converte um número de uma unidade de medida para outra.

- `número`: O número a ser processado.
- `unidade_de`: A unidade em que o número está, como texto, como "mi" ou "lbm".
- `unidade_para`: A unidade para a qual converter, como texto, como "km" ou "kg".

## DEC2BIN

`DEC2BIN(número, [casas])`

Converte um número decimal em um número binário.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## DEC2HEX

`DEC2HEX(número, [casas])`

Converte um número decimal em um número hexadecimal.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## DEC2OCT

`DEC2OCT(número, [casas])`

Converte um número decimal em um número octal.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## DELTA

`DELTA(número1, [número2])`

Informa se dois números são iguais, com 1 para sim e 0 para não.

- `número1`: O primeiro número, ou intervalo de células.
- `número2` (opcional): O segundo número.

## ERF

`ERF(limite_inferior, [limite_superior])`

Retorna a função de erro, entre dois limites ou a partir de 0.

- `limite_inferior`: O limite inferior do intervalo.
- `limite_superior` (opcional): O limite superior do intervalo (somente o limite inferior, se omitido).

## ERF.PRECISE

`ERF.PRECISE(x)`

Retorna a função de erro de 0 até um limite.

- `x`: O valor x a ser usado.

## ERFC

`ERFC(x)`

Retorna a função de erro complementar de um limite até o infinito.

- `x`: O valor x a ser usado.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Retorna a função de erro complementar de um limite até o infinito.

- `x`: O valor x a ser usado.

## GESTEP

`GESTEP(número, [etapa])`

Informa se um número é pelo menos igual a uma etapa, com 1 para sim e 0 para não.

- `número`: O número a ser processado.
- `etapa` (opcional): Quanto somar a cada número (1, se omitido; para GESTEP, o número com o qual comparar, 0 se omitido).

## HEX2BIN

`HEX2BIN(número, [casas])`

Converte um número hexadecimal em um número binário.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## HEX2DEC

`HEX2DEC(número)`

Converte um número hexadecimal em um número decimal.

- `número`: O número a ser processado.

## HEX2OCT

`HEX2OCT(número, [casas])`

Converte um número hexadecimal em um número octal.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## IMABS

`IMABS(inúmero)`

Retorna o módulo de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMAGINARY

`IMAGINARY(inúmero)`

Retorna a parte imaginária de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMARGUMENT

`IMARGUMENT(inúmero)`

Retorna o ângulo de um número complexo, em radianos.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCONJUGATE

`IMCONJUGATE(inúmero)`

Retorna o conjugado de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCOS

`IMCOS(inúmero)`

Retorna o cosseno de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCOSH

`IMCOSH(inúmero)`

Retorna o cosseno hiperbólico de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCOT

`IMCOT(inúmero)`

Retorna a cotangente de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCSC

`IMCSC(inúmero)`

Retorna a cossecante de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCSCH

`IMCSCH(inúmero)`

Retorna a cossecante hiperbólica de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMDIV

`IMDIV(inúmero1, inúmero2)`

Retorna o quociente de dois números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `inúmero2`: O segundo número complexo, escrito como texto.

## IMEXP

`IMEXP(inúmero)`

Retorna e elevado à potência de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMLN

`IMLN(inúmero)`

Retorna o logaritmo natural de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMLOG10

`IMLOG10(inúmero)`

Retorna o logaritmo de base 10 de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMLOG2

`IMLOG2(inúmero)`

Retorna o logaritmo de base 2 de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMPOWER

`IMPOWER(inúmero, número)`

Retorna um número complexo elevado a uma potência.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".
- `número`: O número a ser processado.

## IMPRODUCT

`IMPRODUCT(inúmero1, ...)`

Retorna o produto de números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## IMREAL

`IMREAL(inúmero)`

Retorna a parte real de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSEC

`IMSEC(inúmero)`

Retorna a secante de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSECH

`IMSECH(inúmero)`

Retorna a secante hiperbólica de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSIN

`IMSIN(inúmero)`

Retorna o seno de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSINH

`IMSINH(inúmero)`

Retorna o seno hiperbólico de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSQRT

`IMSQRT(inúmero)`

Retorna a raiz quadrada de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSUB

`IMSUB(inúmero1, inúmero2)`

Retorna a diferença entre dois números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `inúmero2`: O segundo número complexo, escrito como texto.

## IMSUM

`IMSUM(inúmero1, ...)`

Retorna a soma de números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## IMTAN

`IMTAN(inúmero)`

Retorna a tangente de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## OCT2BIN

`OCT2BIN(número, [casas])`

Converte um número octal em um número binário.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.

## OCT2DEC

`OCT2DEC(número)`

Converte um número octal em um número decimal.

- `número`: O número a ser processado.

## OCT2HEX

`OCT2HEX(número, [casas])`

Converte um número octal em um número hexadecimal.

- `número`: O número a ser processado.
- `casas` (opcional): Quantos caracteres completar no resultado, com zeros à esquerda.
