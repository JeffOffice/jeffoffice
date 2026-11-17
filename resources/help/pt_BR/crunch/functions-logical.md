# Funções lógicas
<!-- keywords: funções, E, ESCOLHER, FALSO, SE, SEERRO, SEND, SEFALTAR, SEÑS, LET, NÃO, OU, ALTERNAR, VERDADEIRO, XOU -->

Estas são as funções lógicas que o JeffCrunch consegue calcular. Cada uma mostra como é escrita, o que faz e o que é cada um dos seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros do último tipo forem necessários. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra os argumentos dela, e um clique no nome da função na dica abre a sua entrada aqui.

## E

`E(lógico1, ...)`

Verdadeiro quando todas as condições são verdadeiras.

- `lógico1`: O primeiro valor ou teste, que é verdadeiro ou falso.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## ESCOLHER

`ESCOLHER(índice, valor1, ...)`

Escolhe, em uma lista, o valor que está na posição indicada pelo índice.

- `índice`: Qual valor retornar, contando o primeiro como 1.
- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## FALSO

`FALSO()`

O valor lógico falso.

## SE

`SE(condição, valor_se_verdadeiro, [valor_se_falso])`

Retorna um valor quando uma condição é atendida e outro quando não é.

- `condição`: Um valor ou teste que é verdadeiro ou falso.
- `valor_se_verdadeiro`: O que retornar se a condição for verdadeira.
- `valor_se_falso` (opcional): O que retornar se a condição for falsa (FALSO, se omitido).

## SEERRO

`SEERRO(valor, valor_se_erro)`

Retorna um valor ou, se esse valor for um erro, um substituto.

- `valor`: O valor a processar, ou uma célula que o contenha.
- `valor_se_erro`: O que retornar se o valor for um erro.

## SEND

`SEND(valor, valor_se_nd)`

Retorna um valor ou, se esse valor for o erro de não disponível, um substituto.

- `valor`: O valor a processar, ou uma célula que o contenha.
- `valor_se_nd`: O que retornar se o valor for o erro de não disponível.

## SEFALTAR

`SEFALTAR(condição1, valor1, ...)`

Testa as condições em sequência e retorna o valor correspondente à primeira que for atendida.

- `condição1`: O primeiro teste, que é verdadeiro ou falso.
- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## LET

`LET(nome1, valor1, ...)`

Dá nomes a valores dentro de uma única fórmula e, em seguida, calcula um resultado que os utiliza.

- `nome1`: Um nome a dar ao valor que vem depois. Ele pode ser usado nos valores seguintes e no cálculo.
- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## NÃO

`NÃO(lógico)`

Transforma verdadeiro em falso e falso em verdadeiro.

- `lógico`: Um valor ou teste que é verdadeiro ou falso.

## OU

`OU(lógico1, ...)`

Verdadeiro quando pelo menos uma das condições é verdadeira.

- `lógico1`: O primeiro valor ou teste, que é verdadeiro ou falso.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## ALTERNAR

`ALTERNAR(expressão, valor1, resultado1, ...)`

Compara um valor com uma lista de valores e retorna o resultado correspondente à primeira coincidência.

- `expressão`: O valor a comparar com os valores que seguem.
- `valor1`: O primeiro valor ou intervalo de células.
- `resultado1`: O resultado a retornar quando o valor anterior a ele coincidir.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## VERDADEIRO

`VERDADEIRO()`

O valor lógico verdadeiro.

## XOU

`XOU(lógico1, ...)`

Verdadeiro quando um número ímpar de condições é verdadeiro.

- `lógico1`: O primeiro valor ou teste, que é verdadeiro ou falso.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.
