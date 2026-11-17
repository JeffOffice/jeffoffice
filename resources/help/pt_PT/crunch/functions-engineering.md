# Funções de engenharia
<!-- keywords: funções, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX, função, engenharia, números complexos, conversão, bits, binário, hexadecimal, octal -->

Estas são as funções de engenharia que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos outros do último tipo quantos precisar. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece por baixo da célula uma pequena sugestão com os respetivos argumentos, e se clicar no nome da função na sugestão, abre-se aqui a respetiva entrada.

## BESSELI

`BESSELI(x; n)`

Devolve a função de Bessel modificada de primeira espécie.

- `x`: O valor x a utilizar.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou superior a 0.

## BESSELJ

`BESSELJ(x; n)`

Devolve a função de Bessel de primeira espécie.

- `x`: O valor x a utilizar.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou superior a 0.

## BESSELK

`BESSELK(x; n)`

Devolve a função de Bessel modificada de segunda espécie.

- `x`: O valor x a utilizar.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou superior a 0.

## BESSELY

`BESSELY(x; n)`

Devolve a função de Bessel de segunda espécie.

- `x`: O valor x a utilizar.
- `n`: A primeira potência de x, ou a ordem da função, um número inteiro igual ou superior a 0.

## BIN2DEC

`BIN2DEC(número)`

Converte um número binário num número decimal.

- `número`: O número a processar.

## BIN2HEX

`BIN2HEX(número; [casas])`

Converte um número binário num número hexadecimal.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## BIN2OCT

`BIN2OCT(número; [casas])`

Converte um número binário num número octal.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## BITAND

`BITAND(número1; número2)`

Devolve os bits que estão ativos em ambos os dois números inteiros.

- `número1`: O primeiro número, ou bloco de células.
- `número2`: O segundo número.

## BITLSHIFT

`BITLSHIFT(número; quantidade_deslocamento)`

Desloca os bits de um número inteiro para a esquerda.

- `número`: O número a processar.
- `quantidade_deslocamento`: Quantos bits deslocar: números positivos e negativos deslocam-se em sentidos opostos.

## BITOR

`BITOR(número1; número2)`

Devolve os bits que estão ativos em pelo menos um dos dois números inteiros.

- `número1`: O primeiro número, ou bloco de células.
- `número2`: O segundo número.

## BITRSHIFT

`BITRSHIFT(número; quantidade_deslocamento)`

Desloca os bits de um número inteiro para a direita.

- `número`: O número a processar.
- `quantidade_deslocamento`: Quantos bits deslocar: números positivos e negativos deslocam-se em sentidos opostos.

## BITXOR

`BITXOR(número1; número2)`

Devolve os bits que estão ativos num dos dois números inteiros, mas não em ambos.

- `número1`: O primeiro número, ou bloco de células.
- `número2`: O segundo número.

## COMPLEX

`COMPLEX(núm_real; núm_i; [sufixo])`

Cria um número complexo a partir das suas partes real e imaginária.

- `núm_real`: A parte real do número complexo.
- `núm_i`: A parte imaginária do número complexo.
- `sufixo` (opcional): i ou j, para marcar a parte imaginária (i se for omitido).

## CONVERT

`CONVERT(número; unidade_origem; unidade_destino)`

Converte um número de uma unidade de medida para outra.

- `número`: O número a processar.
- `unidade_origem`: A unidade em que o número está, como texto, como "mi" ou "lbm".
- `unidade_destino`: A unidade para a qual converter, como texto, como "km" ou "kg".

## DEC2BIN

`DEC2BIN(número; [casas])`

Converte um número decimal num número binário.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## DEC2HEX

`DEC2HEX(número; [casas])`

Converte um número decimal num número hexadecimal.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## DEC2OCT

`DEC2OCT(número; [casas])`

Converte um número decimal num número octal.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## DELTA

`DELTA(número1; [número2])`

Indica se dois números são iguais, com 1 para sim e 0 para não.

- `número1`: O primeiro número, ou bloco de células.
- `número2` (opcional): O segundo número.

## ERF

`ERF(limite_inferior; [limite_superior])`

Devolve a função de erro, entre dois limites ou a partir de 0.

- `limite_inferior`: O limite inferior do intervalo.
- `limite_superior` (opcional): O limite superior do intervalo (apenas o limite inferior se for omitido).

## ERF.PRECISE

`ERF.PRECISE(x)`

Devolve a função de erro de 0 até a um limite.

- `x`: O valor x a utilizar.

## ERFC

`ERFC(x)`

Devolve a função de erro complementar de um limite até ao infinito.

- `x`: O valor x a utilizar.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Devolve a função de erro complementar de um limite até ao infinito.

- `x`: O valor x a utilizar.

## GESTEP

`GESTEP(número; [passo])`

Indica se um número é pelo menos igual a um passo, com 1 para sim e 0 para não.

- `número`: O número a processar.
- `passo` (opcional): Quanto somar a cada número (1 se for omitido; para GESTEP, o número com que comparar, 0 se for omitido).

## HEX2BIN

`HEX2BIN(número; [casas])`

Converte um número hexadecimal num número binário.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## HEX2DEC

`HEX2DEC(número)`

Converte um número hexadecimal num número decimal.

- `número`: O número a processar.

## HEX2OCT

`HEX2OCT(número; [casas])`

Converte um número hexadecimal num número octal.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## IMABS

`IMABS(inúmero)`

Devolve o módulo de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMAGINARY

`IMAGINARY(inúmero)`

Devolve a parte imaginária de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMARGUMENT

`IMARGUMENT(inúmero)`

Devolve o ângulo de um número complexo, em radianos.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCONJUGATE

`IMCONJUGATE(inúmero)`

Devolve o conjugado de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCOS

`IMCOS(inúmero)`

Devolve o cosseno de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCOSH

`IMCOSH(inúmero)`

Devolve o cosseno hiperbólico de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCOT

`IMCOT(inúmero)`

Devolve a cotangente de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCSC

`IMCSC(inúmero)`

Devolve a cossecante de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMCSCH

`IMCSCH(inúmero)`

Devolve a cossecante hiperbólica de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMDIV

`IMDIV(inúmero1; inúmero2)`

Devolve o quociente de dois números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `inúmero2`: O segundo número complexo, escrito como texto.

## IMEXP

`IMEXP(inúmero)`

Devolve e elevado à potência de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMLN

`IMLN(inúmero)`

Devolve o logaritmo natural de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMLOG10

`IMLOG10(inúmero)`

Devolve o logaritmo de base 10 de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMLOG2

`IMLOG2(inúmero)`

Devolve o logaritmo de base 2 de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMPOWER

`IMPOWER(inúmero; número)`

Devolve um número complexo elevado a uma potência.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".
- `número`: O número a processar.

## IMPRODUCT

`IMPRODUCT(inúmero1; ...)`

Devolve o produto de números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `...`: Tantos outros quantos precisar, do mesmo tipo que o anterior.

## IMREAL

`IMREAL(inúmero)`

Devolve a parte real de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSEC

`IMSEC(inúmero)`

Devolve a secante de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSECH

`IMSECH(inúmero)`

Devolve a secante hiperbólica de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSIN

`IMSIN(inúmero)`

Devolve o seno de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSINH

`IMSINH(inúmero)`

Devolve o seno hiperbólico de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSQRT

`IMSQRT(inúmero)`

Devolve a raiz quadrada de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## IMSUB

`IMSUB(inúmero1; inúmero2)`

Devolve a diferença entre dois números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `inúmero2`: O segundo número complexo, escrito como texto.

## IMSUM

`IMSUM(inúmero1; ...)`

Devolve a soma de números complexos.

- `inúmero1`: O primeiro número complexo, escrito como texto, como "3+4i".
- `...`: Tantos outros quantos precisar, do mesmo tipo que o anterior.

## IMTAN

`IMTAN(inúmero)`

Devolve a tangente de um número complexo.

- `inúmero`: Um número complexo escrito como texto, como "3+4i".

## OCT2BIN

`OCT2BIN(número; [casas])`

Converte um número octal num número binário.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.

## OCT2DEC

`OCT2DEC(número)`

Converte um número octal num número decimal.

- `número`: O número a processar.

## OCT2HEX

`OCT2HEX(número; [casas])`

Converte um número octal num número hexadecimal.

- `número`: O número a processar.
- `casas` (opcional): Quantos carateres ocupar no resultado, preenchendo com zeros à esquerda.
