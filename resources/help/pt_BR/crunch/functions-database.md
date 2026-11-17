# Funções de banco de dados
<!-- keywords: funções, DMÉDIA, DCONT.NÚM, DCONTAR, DObter, DMÁX, DMÍN, DMULT, DDESVPAD, DDESVPADP, DSOMA, DVAR, DVARP -->

Estas são as funções de banco de dados que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." indica que você pode informar quantos outros forem necessários após o último. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra seus argumentos, e um clique no nome da função na dica abre a sua entrada aqui.

## DMÉDIA

`DMÉDIA(banco_de_dados; campo; critérios)`

Calcula a média dos números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DCONT.NÚM

`DCONT.NÚM(banco_de_dados; campo; critérios)`

Conta os números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DCONTAR

`DCONTAR(banco_de_dados; campo; critérios)`

Conta as células que não estão vazias em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DObter

`DObter(banco_de_dados; campo; critérios)`

Retorna o único valor de uma coluna de uma lista para a única linha que atende aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DMÁX

`DMÁX(banco_de_dados; campo; critérios)`

Retorna o maior número em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DMÍN

`DMÍN(banco_de_dados; campo; critérios)`

Retorna o menor número em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DMULT

`DMULT(banco_de_dados; campo; critérios)`

Multiplica os números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DDESVPAD

`DDESVPAD(banco_de_dados; campo; critérios)`

Estima a dispersão dos números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DDESVPADP

`DDESVPADP(banco_de_dados; campo; critérios)`

Retorna a dispersão dos números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DSOMA

`DSOMA(banco_de_dados; campo; critérios)`

Soma os números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DVAR

`DVAR(banco_de_dados; campo; critérios)`

Estima a variância dos números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".

## DVARP

`DVARP(banco_de_dados; campo; critérios)`

Retorna a variância dos números em uma coluna de uma lista para as linhas que atendem aos critérios.

- `banco_de_dados`: O intervalo de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a usar: o nome dela entre aspas ou seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste, como ">5".
