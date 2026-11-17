# Funções de informação
<!-- keywords: funções, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE, funções de informação, funções lógicas, verificar células, tipo de erro, número de folhas, valor vazio, valor par, valor ímpar, erro, texto, número -->

Estas são as funções de informação que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e o que é cada um dos respetivos argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos argumentos adicionais, do último tipo, quantos precisar. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, uma pequena sugestão por baixo da célula mostra os respetivos argumentos, e se clicar no nome da função nessa sugestão, abre-se a respetiva entrada aqui.

## CELL

`CELL(tipo_info, [referência])`

Indica algo sobre uma célula, como o seu endereço ou o respetivo formato.

- `tipo_info`: O que indicar sobre a célula, como texto do tipo "address", "row", "col" ou "format".
- `referência` (opcional): A célula sobre a qual indicar informação (a célula onde está o cursor, se for omitida).

## ERROR.TYPE

`ERROR.TYPE(valor_erro)`

Devolve o número que representa um tipo de erro.

- `valor_erro`: O erro a procurar, ou uma célula que o contenha.

## INFO

`INFO(tipo_info)`

Indica algo sobre o programa ou o livro, como a respetiva pasta ou a forma como calcula.

- `tipo_info`: O que indicar sobre a célula, como texto do tipo "address", "row", "col" ou "format".

## ISBLANK

`ISBLANK(valor)`

Verdadeiro quando a célula está vazia.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISERR

`ISERR(valor)`

Verdadeiro quando o valor é um erro diferente de não disponível.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISERROR

`ISERROR(valor)`

Verdadeiro quando o valor é qualquer erro.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISEVEN

`ISEVEN(número)`

Verdadeiro quando um número inteiro é par.

- `número`: O número a analisar.

## ISFORMULA

`ISFORMULA(referência)`

Verdadeiro quando a célula contém uma fórmula.

- `referência`: Uma célula ou bloco de células.

## ISLOGICAL

`ISLOGICAL(valor)`

Verdadeiro quando o valor é verdadeiro ou falso.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISNA

`ISNA(valor)`

Verdadeiro quando o valor é o erro de não disponível.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISNONTEXT

`ISNONTEXT(valor)`

Verdadeiro quando o valor é qualquer coisa exceto texto.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISNUMBER

`ISNUMBER(valor)`

Verdadeiro quando o valor é um número.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISODD

`ISODD(número)`

Verdadeiro quando um número inteiro é ímpar.

- `número`: O número a analisar.

## ISREF

`ISREF(valor)`

Verdadeiro quando o valor é uma referência a células.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## ISTEXT

`ISTEXT(valor)`

Verdadeiro quando o valor é texto.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## N

`N(valor)`

Converte um valor num número: datas e valores verdadeiros tornam-se números, e o texto torna-se zero.

- `valor`: O valor a analisar, ou uma célula que o contenha.

## NA

`NA()`

Devolve o erro de não disponível, para assinalar um valor em falta.

## SHEET

`SHEET([valor])`

Devolve o número de uma folha, contando a primeira folha como 1.

- `valor` (opcional): O valor a analisar, ou uma célula que o contenha.

## SHEETS

`SHEETS([referência])`

Devolve o número de folhas numa referência, ou no livro.

- `referência` (opcional): Uma célula ou bloco de células.

## TYPE

`TYPE(valor)`

Devolve um número que indica o tipo de valor: número, texto, lógico, erro ou matriz.

- `valor`: O valor a analisar, ou uma célula que o contenha.
