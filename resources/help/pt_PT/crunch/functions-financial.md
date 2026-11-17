# Funções financeiras
<!-- keywords: funções, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT -->

Estas são as funções financeiras que o JeffCrunch consegue calcular. Cada uma mostra a forma como é escrita, o que faz e o que é cada um dos seus argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos outros quantos precisar, após o último. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, aparece por baixo da célula uma pequena sugestão com os respetivos argumentos; se clicar no nome da função nessa sugestão, abre-se aqui a respetiva entrada.

## ACCRINT

`ACCRINT(emissão, primeiro_juro, liquidação, taxa, valor_nominal, frequência, [base], [método_cálculo])`

Devolve os juros acumulados de um título que paga juros segundo um calendário.

- `emissão`: A data em que o título foi emitido.
- `primeiro_juro`: A data em que são pagos os primeiros juros.
- `liquidação`: A data em que compra o título.
- `taxa`: A taxa de juro de cada período.
- `valor_nominal`: O valor nominal do título (1000 se omitido).
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.
- `método_cálculo` (opcional): Verdadeiro ou omitido para contar os juros desde a data de emissão; falso para os contar desde a data do primeiro juro, quando a liquidação for posterior.

## ACCRINTM

`ACCRINTM(emissão, liquidação, taxa, [valor_nominal], [base])`

Devolve os juros acumulados de um título que paga juros no vencimento.

- `emissão`: A data em que o título foi emitido.
- `liquidação`: A data em que compra o título.
- `taxa`: A taxa de juro de cada período.
- `valor_nominal` (opcional): O valor nominal do título (1000 se omitido).
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## AMORDEGRC

`AMORDEGRC(custo, data_compra, primeiro_período, valor_residual, período, taxa, [base])`

Devolve a depreciação de um período pelo método francês de saldo decrescente.

- `custo`: O custo do ativo quando foi comprado.
- `data_compra`: A data em que o ativo foi comprado.
- `primeiro_período`: A data em que termina o primeiro período.
- `valor_residual`: O valor do ativo no final.
- `período`: O período a calcular, contando o primeiro como 1.
- `taxa`: A taxa de juro de cada período.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## AMORLINC

`AMORLINC(custo, data_compra, primeiro_período, valor_residual, período, taxa, [base])`

Devolve a depreciação de um período pelo método francês de linha reta.

- `custo`: O custo do ativo quando foi comprado.
- `data_compra`: A data em que o ativo foi comprado.
- `primeiro_período`: A data em que termina o primeiro período.
- `valor_residual`: O valor do ativo no final.
- `período`: O período a calcular, contando o primeiro como 1.
- `taxa`: A taxa de juro de cada período.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## COUPDAYBS

`COUPDAYBS(liquidação, vencimento, frequência, [base])`

Devolve os dias desde o início do período de cupão até à data de liquidação.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## COUPDAYS

`COUPDAYS(liquidação, vencimento, frequência, [base])`

Devolve os dias do período de cupão que contém a data de liquidação.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## COUPDAYSNC

`COUPDAYSNC(liquidação, vencimento, frequência, [base])`

Devolve os dias desde a data de liquidação até à data do cupão seguinte.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## COUPNCD

`COUPNCD(liquidação, vencimento, frequência, [base])`

Devolve a data do cupão seguinte à data de liquidação.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## COUPNUM

`COUPNUM(liquidação, vencimento, frequência, [base])`

Devolve o número de cupões a pagar entre as datas de liquidação e de vencimento.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## COUPPCD

`COUPPCD(liquidação, vencimento, frequência, [base])`

Devolve a data do cupão anterior à data de liquidação.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## CUMIPMT

`CUMIPMT(taxa, nper, va, período_inicial, período_final, tipo)`

Devolve os juros pagos num empréstimo entre dois períodos.

- `taxa`: A taxa de juro de cada período.
- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `período_inicial`: O período em que o intervalo começa.
- `período_final`: O período em que o intervalo termina.
- `tipo`: Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## CUMPRINC

`CUMPRINC(taxa, nper, va, período_inicial, período_final, tipo)`

Devolve o capital pago num empréstimo entre dois períodos.

- `taxa`: A taxa de juro de cada período.
- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `período_inicial`: O período em que o intervalo começa.
- `período_final`: O período em que o intervalo termina.
- `tipo`: Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## DB

`DB(custo, valor_residual, vida, período, [mês])`

Devolve a depreciação de um ativo num período, pelo saldo decrescente fixo.

- `custo`: O custo do ativo quando foi comprado.
- `valor_residual`: O valor do ativo no final.
- `vida`: Quantos períodos dura o ativo.
- `período`: O período a calcular, contando o primeiro como 1.
- `mês` (opcional): O mês, de 1 a 12.

## DDB

`DDB(custo, valor_residual, vida, período, [fator])`

Devolve a depreciação de um período pelo método do saldo decrescente.

- `custo`: O custo do ativo quando foi comprado.
- `valor_residual`: O valor do ativo no final.
- `vida`: Quantos períodos dura o ativo.
- `período`: O período a calcular, contando o primeiro como 1.
- `fator` (opcional): A rapidez com que o saldo diminui (2 se omitido, que é o dobro do saldo decrescente).

## DISC

`DISC(liquidação, vencimento, pr, reembolso, [base])`

Devolve a taxa de desconto de um título.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `pr`: O preço do título por cada 100 de valor nominal.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## DOLLARDE

`DOLLARDE(dólar_fracionário, fração)`

Converte um preço escrito como fração, como 1,02, num número decimal.

- `dólar_fracionário`: Um preço escrito como fração, como 1,02 para um inteiro e dois dezasseis avos.
- `fração`: O número pelo qual dividir a unidade: 16 para dezasseis avos, por exemplo.

## DOLLARFR

`DOLLARFR(dólar_decimal, fração)`

Converte um preço decimal num preço escrito como fração, como 1,02.

- `dólar_decimal`: Um preço escrito como número decimal, como 1,125.
- `fração`: O número pelo qual dividir a unidade: 16 para dezasseis avos, por exemplo.

## DURATION

`DURATION(liquidação, vencimento, cupão, rendimento, frequência, [base])`

Devolve o tempo médio para recuperar o custo de uma obrigação, ponderado pelos pagamentos.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `cupão`: A taxa de cupão anual da obrigação.
- `rendimento`: O rendimento anual do título.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## EFFECT

`EFFECT(taxa_nominal, npery)`

Devolve a taxa anual efetiva correspondente a uma taxa indicada com vários períodos por ano.

- `taxa_nominal`: A taxa anual indicada.
- `npery`: Quantos períodos há num ano.

## FV

`FV(taxa, nper, pgto, [va], [tipo])`

Devolve o valor final de pagamentos regulares.

- `taxa`: A taxa de juro de cada período.
- `nper`: Quantos pagamentos há no total.
- `pgto`: O pagamento efetuado em cada período.
- `va` (opcional): O valor atual do empréstimo ou investimento (o montante total).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## FVSCHEDULE

`FVSCHEDULE(capital, calendário)`

Devolve o valor futuro de um montante após uma lista de taxas de juro.

- `capital`: O montante inicial.
- `calendário`: O bloco de células com as taxas de juro a aplicar, uma a seguir à outra.

## INTRATE

`INTRATE(liquidação, vencimento, investimento, reembolso, [base])`

Devolve a taxa de juro de um título totalmente investido.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `investimento`: O montante investido no título.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## IPMT

`IPMT(taxa, período, nper, va, [vf], [tipo])`

Devolve a parte de juros de um pagamento.

- `taxa`: A taxa de juro de cada período.
- `período`: O pagamento a calcular, contando o primeiro como 1.
- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## IRR

`IRR(valores, [estimativa])`

Devolve a taxa interna de rentabilidade de uma série de montantes que ocorrem em intervalos regulares.

- `valores`: O bloco de células que contém os montantes.
- `estimativa` (opcional): A sua estimativa do resultado, para iniciar a pesquisa a partir dela (0,1 se omitido).

## ISPMT

`ISPMT(taxa, período, nper, va)`

Devolve os juros pagos num período de um empréstimo que é reembolsado em partes iguais de capital.

- `taxa`: A taxa de juro de cada período.
- `período`: O pagamento a calcular, contando o primeiro como 1.
- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).

## MDURATION

`MDURATION(liquidação, vencimento, cupão, rendimento, frequência, [base])`

Devolve a duração modificada de uma obrigação, ajustada ao seu rendimento.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `cupão`: A taxa de cupão anual da obrigação.
- `rendimento`: O rendimento anual do título.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## MIRR

`MIRR(valores, taxa_financiamento, taxa_reinvestimento)`

Devolve a taxa de rentabilidade de fluxos de caixa, com uma taxa para pedir empréstimo e outra para reinvestir.

- `valores`: O bloco de células que contém os montantes.
- `taxa_financiamento`: A taxa que paga pelo dinheiro que pede emprestado.
- `taxa_reinvestimento`: A taxa que ganha pelo dinheiro que reinveste.

## NOMINAL

`NOMINAL(taxa_efetiva, npery)`

Devolve a taxa nominal correspondente a uma taxa anual e a um número de períodos por ano.

- `taxa_efetiva`: A taxa anual.
- `npery`: Quantos períodos há num ano.

## NPER

`NPER(taxa, pgto, va, [vf], [tipo])`

Devolve quantos pagamentos são necessários para liquidar um empréstimo.

- `taxa`: A taxa de juro de cada período.
- `pgto`: O pagamento efetuado em cada período.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## NPV

`NPV(taxa, valor1, ...)`

Devolve o valor atual de uma série de montantes futuros.

- `taxa`: A taxa de juro de cada período.
- `valor1`: O primeiro valor, ou bloco de células.
- `...`: Tantos outros quantos precisar, do mesmo tipo que o anterior.

## ODDFPRICE

`ODDFPRICE(liquidação, vencimento, emissão, primeiro_cupão, taxa, rendimento, reembolso, frequência, [base])`

Devolve o preço por 100 de uma obrigação com um primeiro período irregular.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `emissão`: A data em que o título foi emitido.
- `primeiro_cupão`: A data do primeiro cupão.
- `taxa`: A taxa de juro de cada período.
- `rendimento`: O rendimento anual do título.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## ODDFYIELD

`ODDFYIELD(liquidação, vencimento, emissão, primeiro_cupão, taxa, pr, reembolso, frequência, [base])`

Devolve o rendimento de uma obrigação com um primeiro período irregular.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `emissão`: A data em que o título foi emitido.
- `primeiro_cupão`: A data do primeiro cupão.
- `taxa`: A taxa de juro de cada período.
- `pr`: O preço do título por cada 100 de valor nominal.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## ODDLPRICE

`ODDLPRICE(liquidação, vencimento, último_juro, taxa, rendimento, reembolso, frequência, [base])`

Devolve o preço por 100 de uma obrigação com um último período irregular.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `último_juro`: A data em que foram pagos os últimos juros.
- `taxa`: A taxa de juro de cada período.
- `rendimento`: O rendimento anual do título.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## ODDLYIELD

`ODDLYIELD(liquidação, vencimento, último_juro, taxa, pr, reembolso, frequência, [base])`

Devolve o rendimento de uma obrigação com um último período irregular.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `último_juro`: A data em que foram pagos os últimos juros.
- `taxa`: A taxa de juro de cada período.
- `pr`: O preço do título por cada 100 de valor nominal.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## PDURATION

`PDURATION(taxa, va, vf)`

Devolve os períodos necessários para um investimento atingir um valor.

- `taxa`: A taxa de juro de cada período.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf`: O que resta após o último pagamento (0 se omitido).

## PMT

`PMT(taxa, nper, va, [vf], [tipo])`

Devolve o pagamento regular de um empréstimo.

- `taxa`: A taxa de juro de cada período.
- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## PPMT

`PPMT(taxa, período, nper, va, [vf], [tipo])`

Devolve a parte de um pagamento que amortiza o próprio empréstimo.

- `taxa`: A taxa de juro de cada período.
- `período`: O pagamento a calcular, contando o primeiro como 1.
- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## PRICE

`PRICE(liquidação, vencimento, taxa, rendimento, reembolso, frequência, [base])`

Devolve o preço por 100 de uma obrigação que paga juros segundo um calendário.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `taxa`: A taxa de juro de cada período.
- `rendimento`: O rendimento anual do título.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## PRICEDISC

`PRICEDISC(liquidação, vencimento, desconto, reembolso, [base])`

Devolve o preço por 100 de um título comprado com desconto.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `desconto`: A taxa de desconto do título.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## PRICEMAT

`PRICEMAT(liquidação, vencimento, emissão, taxa, rendimento, [base])`

Devolve o preço por 100 de um título que paga juros no vencimento.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `emissão`: A data em que o título foi emitido.
- `taxa`: A taxa de juro de cada período.
- `rendimento`: O rendimento anual do título.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## PV

`PV(taxa, nper, pgto, [vf], [tipo])`

Devolve quanto vale hoje uma série de pagamentos futuros.

- `taxa`: A taxa de juro de cada período.
- `nper`: Quantos pagamentos há no total.
- `pgto`: O pagamento efetuado em cada período.
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.

## RATE

`RATE(nper, pgto, va, [vf], [tipo], [estimativa])`

Devolve a taxa de juro de cada período de um empréstimo ou de um investimento.

- `nper`: Quantos pagamentos há no total.
- `pgto`: O pagamento efetuado em cada período.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando são feitos os pagamentos: 0 ou omitido no fim de cada período, 1 no início.
- `estimativa` (opcional): A sua estimativa do resultado, para iniciar a pesquisa a partir dela (0,1 se omitido).

## RECEIVED

`RECEIVED(liquidação, vencimento, investimento, desconto, [base])`

Devolve o montante recebido no vencimento por um título totalmente investido.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `investimento`: O montante investido no título.
- `desconto`: A taxa de desconto do título.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## RRI

`RRI(nper, va, vf)`

Devolve a taxa de juro que faz um investimento crescer até um valor.

- `nper`: Quantos pagamentos há no total.
- `va`: O valor atual do empréstimo ou investimento (o montante total).
- `vf`: O que resta após o último pagamento (0 se omitido).

## SLN

`SLN(custo, valor_residual, vida)`

Devolve a depreciação anual pelo método da linha reta.

- `custo`: O custo do ativo quando foi comprado.
- `valor_residual`: O valor do ativo no final.
- `vida`: Quantos períodos dura o ativo.

## SYD

`SYD(custo, valor_residual, vida, período)`

Devolve a depreciação anual pelo método da soma dos dígitos dos anos.

- `custo`: O custo do ativo quando foi comprado.
- `valor_residual`: O valor do ativo no final.
- `vida`: Quantos períodos dura o ativo.
- `período`: O pagamento a calcular, contando o primeiro como 1.

## TBILLEQ

`TBILLEQ(liquidação, vencimento, desconto)`

Devolve o rendimento equivalente de obrigação de uma letra do Tesouro.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `desconto`: A taxa de desconto do título.

## TBILLPRICE

`TBILLPRICE(liquidação, vencimento, desconto)`

Devolve o preço por 100 de uma letra do Tesouro.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `desconto`: A taxa de desconto do título.

## TBILLYIELD

`TBILLYIELD(liquidação, vencimento, pr)`

Devolve o rendimento de uma letra do Tesouro.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `pr`: O preço do título por cada 100 de valor nominal.

## VDB

`VDB(custo, valor_residual, vida, período_inicial, período_final, [fator], [sem_mudança])`

Devolve a depreciação de qualquer conjunto de períodos pelo método do saldo decrescente.

- `custo`: O custo do ativo quando foi comprado.
- `valor_residual`: O valor do ativo no final.
- `vida`: Quantos períodos dura o ativo.
- `período_inicial`: O período em que o intervalo começa.
- `período_final`: O período em que o intervalo termina.
- `fator` (opcional): A rapidez com que o saldo diminui (2 se omitido, que é o dobro do saldo decrescente).
- `sem_mudança` (opcional): Verdadeiro para continuar a usar o saldo decrescente mesmo quando a linha reta daria um valor superior.

## XIRR

`XIRR(valores, datas, [estimativa])`

Devolve a taxa de rentabilidade de montantes que ocorrem em datas que escolhe.

- `valores`: O bloco de células que contém os montantes.
- `datas`: As datas dos montantes, pela mesma ordem que os montantes.
- `estimativa` (opcional): A sua estimativa do resultado, para iniciar a pesquisa a partir dela (0,1 se omitido).

## XNPV

`XNPV(taxa, valores, datas)`

Devolve o valor atual de montantes que ocorrem em datas que escolhe.

- `taxa`: A taxa de juro de cada período.
- `valores`: O bloco de células que contém os montantes.
- `datas`: As datas dos montantes, pela mesma ordem que os montantes.

## YIELD

`YIELD(liquidação, vencimento, taxa, pr, reembolso, frequência, [base])`

Devolve o rendimento de uma obrigação que paga juros segundo um calendário.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `taxa`: A taxa de juro de cada período.
- `pr`: O preço do título por cada 100 de valor nominal.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `frequência`: Quantos cupões são pagos por ano: 1, 2 ou 4.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## YIELDDISC

`YIELDDISC(liquidação, vencimento, pr, reembolso, [base])`

Devolve o rendimento anual de um título comprado com desconto.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `pr`: O preço do título por cada 100 de valor nominal.
- `reembolso`: O valor do título no vencimento, por cada 100 de valor nominal.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.

## YIELDMAT

`YIELDMAT(liquidação, vencimento, emissão, taxa, pr, [base])`

Devolve o rendimento anual de um título que paga juros no vencimento.

- `liquidação`: A data em que compra o título.
- `vencimento`: A data em que o título termina e é reembolsado.
- `emissão`: A data em que o título foi emitido.
- `taxa`: A taxa de juro de cada período.
- `pr`: O preço do título por cada 100 de valor nominal.
- `base` (opcional): Forma de contar os dias do ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.
