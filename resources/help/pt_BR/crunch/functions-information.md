# Funções de informação

<!-- keywords: funções, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE, funções de informação, informações, célula, erro, tipo de erro, verificar, é vazio, é número, é texto, é par, é ímpar, fórmula, planilha, planilhas -->

Estas são as funções de informação que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros forem necessários após o último. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra seus argumentos, e um clique no nome da função na dica abre a sua entrada aqui.

## CELL

`CELL(tipo_info, [referência])`

Informa algo sobre uma célula, como seu endereço ou seu formato.

- `tipo_info`: O que informar sobre a célula, como texto, por exemplo "endereço", "linha", "coluna" ou "formato".
- `referência` (opcional): A célula sobre a qual informar (a célula com o cursor, se omitido).

## ERROR.TYPE

`ERROR.TYPE(valor_erro)`

Retorna o número que representa um tipo de erro.

- `valor_erro`: O erro a consultar, ou uma célula que o contenha.

## INFO

`INFO(tipo_info)`

Informa algo sobre o programa ou a pasta de trabalho, como sua pasta ou a forma como ela calcula.

- `tipo_info`: O que informar sobre a célula, como texto, por exemplo "endereço", "linha", "coluna" ou "formato".

## ISBLANK

`ISBLANK(valor)`

Verdadeiro quando a célula está vazia.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISERR

`ISERR(valor)`

Verdadeiro quando o valor é um erro diferente de não disponível.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISERROR

`ISERROR(valor)`

Verdadeiro quando o valor é qualquer erro.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISEVEN

`ISEVEN(número)`

Verdadeiro quando um número inteiro é par.

- `número`: O número a ser verificado.

## ISFORMULA

`ISFORMULA(referência)`

Verdadeiro quando a célula contém uma fórmula.

- `referência`: Uma célula ou um intervalo de células.

## ISLOGICAL

`ISLOGICAL(valor)`

Verdadeiro quando o valor é verdadeiro ou falso.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISNA

`ISNA(valor)`

Verdadeiro quando o valor é o erro de não disponível.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISNONTEXT

`ISNONTEXT(valor)`

Verdadeiro quando o valor é qualquer coisa exceto texto.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISNUMBER

`ISNUMBER(valor)`

Verdadeiro quando o valor é um número.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISODD

`ISODD(número)`

Verdadeiro quando um número inteiro é ímpar.

- `número`: O número a ser verificado.

## ISREF

`ISREF(valor)`

Verdadeiro quando o valor é uma referência a células.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## ISTEXT

`ISTEXT(valor)`

Verdadeiro quando o valor é texto.

- `valor`: O valor a ser verificado, ou uma célula que o contenha.

## N

`N(valor)`

Converte um valor em número: datas e valores verdadeiros se tornam números, e texto se torna zero.

- `valor`: O valor a ser usado, ou uma célula que o contenha.

## NA

`NA()`

Retorna o erro de não disponível, para marcar um valor que está faltando.

## SHEET

`SHEET([valor])`

Retorna o número de uma planilha, contando a primeira planilha como 1.

- `valor` (opcional): O valor a ser usado, ou uma célula que o contenha.

## SHEETS

`SHEETS([referência])`

Retorna o número de planilhas em uma referência, ou na pasta de trabalho.

- `referência` (opcional): Uma célula ou um intervalo de células.

## TYPE

`TYPE(valor)`

Retorna um número que indica o tipo de valor: número, texto, lógico, erro ou matriz.

- `valor`: O valor a ser usado, ou uma célula que o contenha.
