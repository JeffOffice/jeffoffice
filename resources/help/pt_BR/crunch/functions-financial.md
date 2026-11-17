# Funções financeiras

<!-- keywords: funções, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT -->

Estas são as funções financeiras que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros do último tipo forem necessários. Para experimentar uma delas, veja [Escolher uma função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra os argumentos dela, e um clique no nome da função na dica abre o item correspondente aqui.

## ACCRINT

`ACCRINT(emissão, primeiro_juro, liquidação, taxa, valor_nominal, frequência, [base], [método_cálculo])`

Retorna os juros acumulados de um título que paga juros em datas programadas.

- `emissão`: A data em que o título foi emitido.
- `primeiro_juro`: A data em que o primeiro juro é pago.
- `liquidação`: A data em que você compra o título.
- `taxa`: A taxa de juros de cada período.
- `valor_nominal`: O valor de face do título (1.000 se omitido).
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.
- `método_cálculo` (opcional): Verdadeiro ou omitido para contar os juros a partir da data de emissão; falso para contá-los a partir da data do primeiro juro quando a liquidação for posterior.

## ACCRINTM

`ACCRINTM(emissão, liquidação, taxa, [valor_nominal], [base])`

Retorna os juros acumulados de um título que paga juros no vencimento.

- `emissão`: A data em que o título foi emitido.
- `liquidação`: A data em que você compra o título.
- `taxa`: A taxa de juros de cada período.
- `valor_nominal` (opcional): O valor de face do título (1.000 se omitido).
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## AMORDEGRC

`AMORDEGRC(custo, data_compra, primeiro_período, valor_residual, período, taxa, [base])`

Retorna a depreciação de um período pelo método francês de saldo decrescente.

- `custo`: Quanto o ativo custou quando foi comprado.
- `data_compra`: A data em que o ativo foi comprado.
- `primeiro_período`: A data em que o primeiro período termina.
- `valor_residual`: Quanto o ativo vale no final.
- `período`: Qual período calcular, contando o primeiro como 1.
- `taxa`: A taxa de juros de cada período.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## AMORLINC

`AMORLINC(custo, data_compra, primeiro_período, valor_residual, período, taxa, [base])`

Retorna a depreciação de um período pelo método francês de linha reta.

- `custo`: Quanto o ativo custou quando foi comprado.
- `data_compra`: A data em que o ativo foi comprado.
- `primeiro_período`: A data em que o primeiro período termina.
- `valor_residual`: Quanto o ativo vale no final.
- `período`: Qual período calcular, contando o primeiro como 1.
- `taxa`: A taxa de juros de cada período.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## COUPDAYBS

`COUPDAYBS(liquidação, vencimento, frequência, [base])`

Retorna os dias desde o início do período de cupom até a data de liquidação.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## COUPDAYS

`COUPDAYS(liquidação, vencimento, frequência, [base])`

Retorna os dias do período de cupom que contém a data de liquidação.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## COUPDAYSNC

`COUPDAYSNC(liquidação, vencimento, frequência, [base])`

Retorna os dias desde a data de liquidação até a próxima data de cupom.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## COUPNCD

`COUPNCD(liquidação, vencimento, frequência, [base])`

Retorna a próxima data de cupom após a data de liquidação.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## COUPNUM

`COUPNUM(liquidação, vencimento, frequência, [base])`

Retorna o número de cupons a serem pagos entre as datas de liquidação e de vencimento.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## COUPPCD

`COUPPCD(liquidação, vencimento, frequência, [base])`

Retorna a data de cupom anterior à data de liquidação.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## CUMIPMT

`CUMIPMT(taxa, nper, vp, período_inicial, período_final, tipo)`

Retorna os juros pagos em um empréstimo entre dois períodos.

- `taxa`: A taxa de juros de cada período.
- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `período_inicial`: O período em que o intervalo começa.
- `período_final`: O período em que o intervalo termina.
- `tipo`: Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## CUMPRINC

`CUMPRINC(taxa, nper, vp, período_inicial, período_final, tipo)`

Retorna o principal pago em um empréstimo entre dois períodos.

- `taxa`: A taxa de juros de cada período.
- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `período_inicial`: O período em que o intervalo começa.
- `período_final`: O período em que o intervalo termina.
- `tipo`: Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## DB

`DB(custo, valor_residual, vida_útil, período, [mês])`

Retorna a depreciação de um ativo em um período, pelo saldo decrescente fixo.

- `custo`: Quanto o ativo custou quando foi comprado.
- `valor_residual`: Quanto o ativo vale no final.
- `vida_útil`: Quantos períodos o ativo dura.
- `período`: Qual período calcular, contando o primeiro como 1.
- `mês` (opcional): O mês, de 1 a 12.

## DDB

`DDB(custo, valor_residual, vida_útil, período, [fator])`

Retorna a depreciação de um período pelo método do saldo decrescente.

- `custo`: Quanto o ativo custou quando foi comprado.
- `valor_residual`: Quanto o ativo vale no final.
- `vida_útil`: Quantos períodos o ativo dura.
- `período`: Qual período calcular, contando o primeiro como 1.
- `fator` (opcional): Com que rapidez o saldo diminui (2 se omitido, o que equivale ao saldo duplo decrescente).

## DISC

`DISC(liquidação, vencimento, preço, resgate, [base])`

Retorna a taxa de desconto de um título.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `preço`: O preço do título por 100 de valor de face.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## DOLLARDE

`DOLLARDE(preço_fracionário, fração)`

Converte um preço escrito como fração, como 1,02, em um número decimal.

- `preço_fracionário`: Um preço escrito como fração, como 1,02 para um inteiro e dois dezesseis avos.
- `fração`: O número pelo qual dividir a unidade: 16 para dezesseis avos, por exemplo.

## DOLLARFR

`DOLLARFR(preço_decimal, fração)`

Converte um preço decimal em um preço escrito como fração, como 1,02.

- `preço_decimal`: Um preço escrito como número decimal, como 1,125.
- `fração`: O número pelo qual dividir a unidade: 16 para dezesseis avos, por exemplo.

## DURATION

`DURATION(liquidação, vencimento, cupom, rendimento, frequência, [base])`

Retorna o tempo médio para recuperar o custo de um título, ponderado pelos pagamentos.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `cupom`: A taxa anual de cupom do título.
- `rendimento`: O rendimento anual do título.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## EFFECT

`EFFECT(taxa_nominal, nper_ano)`

Retorna a taxa anual equivalente a uma taxa nominal com vários períodos por ano.

- `taxa_nominal`: A taxa anual nominal.
- `nper_ano`: Quantos períodos há em um ano.

## FV

`FV(taxa, nper, pgto, [vp], [tipo])`

Retorna quanto pagamentos regulares valem no final.

- `taxa`: A taxa de juros de cada período.
- `nper`: Quantos pagamentos há no total.
- `pgto`: O pagamento feito em cada período.
- `vp` (opcional): Quanto os valores valem agora (o empréstimo ou investimento total).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## FVSCHEDULE

`FVSCHEDULE(principal, cronograma)`

Retorna o valor futuro de um montante após uma lista de taxas de juros.

- `principal`: O valor inicial.
- `cronograma`: O intervalo de células com as taxas de juros a aplicar, uma após a outra.

## INTRATE

`INTRATE(liquidação, vencimento, investimento, resgate, [base])`

Retorna a taxa de juros de um título totalmente investido.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `investimento`: O valor aplicado no título.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## IPMT

`IPMT(taxa, per, nper, vp, [vf], [tipo])`

Retorna a parte de juros de um pagamento.

- `taxa`: A taxa de juros de cada período.
- `per`: Qual pagamento calcular, contando o primeiro como 1.
- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## IRR

`IRR(valores, [estimativa])`

Retorna a taxa de retorno de uma série de valores que ocorrem em intervalos regulares.

- `valores`: O intervalo de células que contém os valores.
- `estimativa` (opcional): Sua estimativa do resultado, para iniciar a busca a partir dela (0,1 se omitido).

## ISPMT

`ISPMT(taxa, per, nper, vp)`

Retorna os juros pagos em um período de um empréstimo que é quitado em parcelas iguais de principal.

- `taxa`: A taxa de juros de cada período.
- `per`: Qual pagamento calcular, contando o primeiro como 1.
- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).

## MDURATION

`MDURATION(liquidação, vencimento, cupom, rendimento, frequência, [base])`

Retorna a duração de um título, ajustada pelo seu rendimento.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `cupom`: A taxa anual de cupom do título.
- `rendimento`: O rendimento anual do título.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## MIRR

`MIRR(valores, taxa_financiamento, taxa_reinvestimento)`

Retorna a taxa de retorno de fluxos de caixa, com uma taxa para tomar emprestado e outra para reinvestir.

- `valores`: O intervalo de células que contém os valores.
- `taxa_financiamento`: A taxa que você paga sobre o dinheiro que toma emprestado.
- `taxa_reinvestimento`: A taxa que você ganha sobre o dinheiro reinvestido.

## NOMINAL

`NOMINAL(taxa_efetiva, nper_ano)`

Retorna a taxa nominal correspondente a uma taxa anual e a um número de períodos por ano.

- `taxa_efetiva`: A taxa anual.
- `nper_ano`: Quantos períodos há em um ano.

## NPER

`NPER(taxa, pgto, vp, [vf], [tipo])`

Retorna quantos pagamentos são necessários para quitar um empréstimo.

- `taxa`: A taxa de juros de cada período.
- `pgto`: O pagamento feito em cada período.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## NPV

`NPV(taxa, valor1, ...)`

Retorna o valor presente de uma série de valores futuros.

- `taxa`: A taxa de juros de cada período.
- `valor1`: O primeiro valor ou intervalo de células.
- `...`: Quantos outros forem necessários, do mesmo tipo do anterior.

## ODDFPRICE

`ODDFPRICE(liquidação, vencimento, emissão, primeiro_cupom, taxa, rendimento, resgate, frequência, [base])`

Retorna o preço por 100 de um título com primeiro período irregular.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `emissão`: A data em que o título foi emitido.
- `primeiro_cupom`: A data do primeiro cupom.
- `taxa`: A taxa de juros de cada período.
- `rendimento`: O rendimento anual do título.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## ODDFYIELD

`ODDFYIELD(liquidação, vencimento, emissão, primeiro_cupom, taxa, preço, resgate, frequência, [base])`

Retorna o rendimento de um título com primeiro período irregular.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `emissão`: A data em que o título foi emitido.
- `primeiro_cupom`: A data do primeiro cupom.
- `taxa`: A taxa de juros de cada período.
- `preço`: O preço do título por 100 de valor de face.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## ODDLPRICE

`ODDLPRICE(liquidação, vencimento, último_juro, taxa, rendimento, resgate, frequência, [base])`

Retorna o preço por 100 de um título com último período irregular.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `último_juro`: A data em que o último juro foi pago.
- `taxa`: A taxa de juros de cada período.
- `rendimento`: O rendimento anual do título.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## ODDLYIELD

`ODDLYIELD(liquidação, vencimento, último_juro, taxa, preço, resgate, frequência, [base])`

Retorna o rendimento de um título com último período irregular.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `último_juro`: A data em que o último juro foi pago.
- `taxa`: A taxa de juros de cada período.
- `preço`: O preço do título por 100 de valor de face.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## PDURATION

`PDURATION(taxa, vp, vf)`

Retorna os períodos necessários para que um investimento atinja um valor.

- `taxa`: A taxa de juros de cada período.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf`: O que resta após o último pagamento (0 se omitido).

## PMT

`PMT(taxa, nper, vp, [vf], [tipo])`

Retorna o pagamento periódico de um empréstimo.

- `taxa`: A taxa de juros de cada período.
- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## PPMT

`PPMT(taxa, per, nper, vp, [vf], [tipo])`

Retorna a parte de um pagamento que amortiza o próprio empréstimo.

- `taxa`: A taxa de juros de cada período.
- `per`: Qual pagamento calcular, contando o primeiro como 1.
- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## PRICE

`PRICE(liquidação, vencimento, taxa, rendimento, resgate, frequência, [base])`

Retorna o preço por 100 de um título que paga juros em datas programadas.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `taxa`: A taxa de juros de cada período.
- `rendimento`: O rendimento anual do título.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## PRICEDISC

`PRICEDISC(liquidação, vencimento, desconto, resgate, [base])`

Retorna o preço por 100 de um título comprado com desconto.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `desconto`: A taxa de desconto do título.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## PRICEMAT

`PRICEMAT(liquidação, vencimento, emissão, taxa, rendimento, [base])`

Retorna o preço por 100 de um título que paga juros no vencimento.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `emissão`: A data em que o título foi emitido.
- `taxa`: A taxa de juros de cada período.
- `rendimento`: O rendimento anual do título.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## PV

`PV(taxa, nper, pgto, [vf], [tipo])`

Retorna quanto uma série de pagamentos futuros vale hoje.

- `taxa`: A taxa de juros de cada período.
- `nper`: Quantos pagamentos há no total.
- `pgto`: O pagamento feito em cada período.
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.

## RATE

`RATE(nper, pgto, vp, [vf], [tipo], [estimativa])`

Retorna a taxa de juros de cada período de um empréstimo ou de um investimento.

- `nper`: Quantos pagamentos há no total.
- `pgto`: O pagamento feito em cada período.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf` (opcional): O que resta após o último pagamento (0 se omitido).
- `tipo` (opcional): Quando os pagamentos são feitos: 0 ou omitido no fim de cada período, 1 no início.
- `estimativa` (opcional): Sua estimativa do resultado, para iniciar a busca a partir dela (0,1 se omitido).

## RECEIVED

`RECEIVED(liquidação, vencimento, investimento, desconto, [base])`

Retorna o valor recebido no vencimento de um título totalmente investido.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `investimento`: O valor aplicado no título.
- `desconto`: A taxa de desconto do título.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## RRI

`RRI(nper, vp, vf)`

Retorna a taxa de juros que faz um investimento crescer até um valor.

- `nper`: Quantos pagamentos há no total.
- `vp`: Quanto os valores valem agora (o empréstimo ou investimento total).
- `vf`: O que resta após o último pagamento (0 se omitido).

## SLN

`SLN(custo, valor_residual, vida_útil)`

Retorna a depreciação anual pelo método da linha reta.

- `custo`: Quanto o ativo custou quando foi comprado.
- `valor_residual`: Quanto o ativo vale no final.
- `vida_útil`: Quantos períodos o ativo dura.

## SYD

`SYD(custo, valor_residual, vida_útil, período)`

Retorna a depreciação de um ano pelo método da soma dos dígitos dos anos.

- `custo`: Quanto o ativo custou quando foi comprado.
- `valor_residual`: Quanto o ativo vale no final.
- `vida_útil`: Quantos períodos o ativo dura.
- `período`: Qual pagamento calcular, contando o primeiro como 1.

## TBILLEQ

`TBILLEQ(liquidação, vencimento, desconto)`

Retorna o rendimento equivalente a título de uma Letra do Tesouro.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `desconto`: A taxa de desconto do título.

## TBILLPRICE

`TBILLPRICE(liquidação, vencimento, desconto)`

Retorna o preço por 100 de uma Letra do Tesouro.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `desconto`: A taxa de desconto do título.

## TBILLYIELD

`TBILLYIELD(liquidação, vencimento, preço)`

Retorna o rendimento de uma Letra do Tesouro.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `preço`: O preço do título por 100 de valor de face.

## VDB

`VDB(custo, valor_residual, vida_útil, período_inicial, período_final, [fator], [sem_troca])`

Retorna a depreciação de qualquer intervalo de períodos pelo método do saldo decrescente.

- `custo`: Quanto o ativo custou quando foi comprado.
- `valor_residual`: Quanto o ativo vale no final.
- `vida_útil`: Quantos períodos o ativo dura.
- `período_inicial`: O período em que o intervalo começa.
- `período_final`: O período em que o intervalo termina.
- `fator` (opcional): Com que rapidez o saldo diminui (2 se omitido, o que equivale ao saldo duplo decrescente).
- `sem_troca` (opcional): Verdadeiro para continuar usando o saldo decrescente mesmo quando a linha reta daria mais.

## XIRR

`XIRR(valores, datas, [estimativa])`

Retorna a taxa de retorno de valores que ocorrem em datas que você escolhe.

- `valores`: O intervalo de células que contém os valores.
- `datas`: As datas dos valores, na mesma ordem dos valores.
- `estimativa` (opcional): Sua estimativa do resultado, para iniciar a busca a partir dela (0,1 se omitido).

## XNPV

`XNPV(taxa, valores, datas)`

Retorna o valor presente de valores que ocorrem em datas que você escolhe.

- `taxa`: A taxa de juros de cada período.
- `valores`: O intervalo de células que contém os valores.
- `datas`: As datas dos valores, na mesma ordem dos valores.

## YIELD

`YIELD(liquidação, vencimento, taxa, preço, resgate, frequência, [base])`

Retorna o rendimento de um título que paga juros em datas programadas.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `taxa`: A taxa de juros de cada período.
- `preço`: O preço do título por 100 de valor de face.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `frequência`: Quantos cupons são pagos em um ano: 1, 2 ou 4.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## YIELDDISC

`YIELDDISC(liquidação, vencimento, preço, resgate, [base])`

Retorna o rendimento anual de um título comprado com desconto.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `preço`: O preço do título por 100 de valor de face.
- `resgate`: O valor do título no vencimento, por 100 de valor de face.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.

## YIELDMAT

`YIELDMAT(liquidação, vencimento, emissão, taxa, preço, [base])`

Retorna o rendimento anual de um título que paga juros no vencimento.

- `liquidação`: A data em que você compra o título.
- `vencimento`: A data em que o título termina e é resgatado.
- `emissão`: A data em que o título foi emitido.
- `taxa`: A taxa de juros de cada período.
- `preço`: O preço do título por 100 de valor de face.
- `base` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 30/360 europeu.
