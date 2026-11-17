# Funções de base de dados
<!-- keywords: funções, DAVERAGE, DCOUNT, DCOUNTA, DGET, DMAX, DMIN, DPRODUCT, DSTDEV, DSTDEVP, DSUM, DVAR, DVARP, base de dados, média, contar, máximo, mínimo, produto, desvio padrão, variância, soma -->

Estas são as funções de base de dados que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos outros quantos precisar do último. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, uma pequena sugestão por baixo da célula mostra os respetivos argumentos, e um clique no nome da função na sugestão abre a respetiva entrada aqui.

## DAVERAGE

`DAVERAGE(base_de_dados; campo; critérios)`

Calcula a média dos números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DCOUNT

`DCOUNT(base_de_dados; campo; critérios)`

Conta os números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DCOUNTA

`DCOUNTA(base_de_dados; campo; critérios)`

Conta as células que não estão vazias numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DGET

`DGET(base_de_dados; campo; critérios)`

Devolve o único valor de uma coluna de uma lista para a única linha que cumpre os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DMAX

`DMAX(base_de_dados; campo; critérios)`

Devolve o maior número numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DMIN

`DMIN(base_de_dados; campo; critérios)`

Devolve o menor número numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DPRODUCT

`DPRODUCT(base_de_dados; campo; critérios)`

Multiplica os números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DSTDEV

`DSTDEV(base_de_dados; campo; critérios)`

Estima a dispersão dos números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DSTDEVP

`DSTDEVP(base_de_dados; campo; critérios)`

Devolve a dispersão dos números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DSUM

`DSUM(base_de_dados; campo; critérios)`

Soma os números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DVAR

`DVAR(base_de_dados; campo; critérios)`

Estima a variância dos números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".

## DVARP

`DVARP(base_de_dados; campo; critérios)`

Devolve a variância dos números numa coluna de uma lista para as linhas que cumprem os critérios.

- `base_de_dados`: O bloco de células que contém a lista, com os nomes das colunas na primeira linha.
- `campo`: A coluna a utilizar: o seu nome entre aspas ou o seu número, contando a primeira coluna como 1.
- `critérios`: O que procurar: um número, um texto ou um teste como ">5".
