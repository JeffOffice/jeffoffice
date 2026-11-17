# Funções lógicas
<!-- keywords: funções, AND, CHOOSE, FALSE, IF, IFERROR, IFNA, IFS, LET, NOT, OR, SWITCH, TRUE, XOR, E, ESCOLHER, FALSO, SE, SEERRO, SENA, SES, VERDADEIRO, NÃO, OU, XOU -->

Estas são as funções lógicas que o JeffCrunch consegue calcular. Cada uma mostra a forma como é escrita, o que faz e o que é cada um dos respetivos argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos outros do último tipo quantos precisar. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece por baixo da célula uma pequena sugestão com os respetivos argumentos, e um clique no nome da função na sugestão abre a respetiva entrada aqui.

## AND

`AND(logical1, ...)`

Verdadeiro quando todas as condições são verdadeiras.

- `logical1`: O primeiro valor ou teste, que é verdadeiro ou falso.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## CHOOSE

`CHOOSE(index, value1, ...)`

Escolhe, numa lista, o valor que está na posição indicada pelo índice.

- `index`: Qual o valor a devolver, contando o primeiro como 1.
- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## FALSE

`FALSE()`

O valor lógico falso.

## IF

`IF(condition, value_if_true, [value_if_false])`

Devolve um valor quando uma condição se verifica e outro quando não se verifica.

- `condition`: Um valor ou teste que é verdadeiro ou falso.
- `value_if_true`: O que devolver se a condição for verdadeira.
- `value_if_false` (opcional): O que devolver se a condição for falsa (FALSE, se for omitido).

## IFERROR

`IFERROR(value, value_if_error)`

Devolve um valor ou um substituto, quando esse valor é um erro.

- `value`: O valor a processar, ou uma célula que o contenha.
- `value_if_error`: O que devolver se o valor for um erro.

## IFNA

`IFNA(value, value_if_na)`

Devolve um valor ou um substituto, quando esse valor é o erro de não disponível.

- `value`: O valor a processar, ou uma célula que o contenha.
- `value_if_na`: O que devolver se o valor for o erro de não disponível.

## IFS

`IFS(condition1, value1, ...)`

Testa as condições por ordem e devolve o valor associado à primeira que se verificar.

- `condition1`: O primeiro teste, que é verdadeiro ou falso.
- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## LET

`LET(name1, value1, ...)`

Atribui nomes a valores dentro de uma única fórmula e, em seguida, calcula um cálculo que os utiliza.

- `name1`: Um nome a atribuir ao valor que se segue. Pode ser usado nos valores seguintes e no cálculo.
- `value1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## NOT

`NOT(logical)`

Transforma o verdadeiro em falso e o falso em verdadeiro.

- `logical`: Um valor ou teste que é verdadeiro ou falso.

## OR

`OR(logical1, ...)`

Verdadeiro quando pelo menos uma das condições é verdadeira.

- `logical1`: O primeiro valor ou teste, que é verdadeiro ou falso.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## SWITCH

`SWITCH(expression, value1, result1, ...)`

Compara um valor com uma lista de valores e devolve o resultado associado à primeira correspondência.

- `expression`: O valor a comparar com os valores que se seguem.
- `value1`: O primeiro valor, ou bloco de células.
- `result1`: O resultado a devolver quando o valor anterior corresponder.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.

## TRUE

`TRUE()`

O valor lógico verdadeiro.

## XOR

`XOR(logical1, ...)`

Verdadeiro quando um número ímpar de condições é verdadeiro.

- `logical1`: O primeiro valor ou teste, que é verdadeiro ou falso.
- `...`: Tantos outros quantos precisar, do mesmo tipo do anterior.
