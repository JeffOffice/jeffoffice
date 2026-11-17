# Funções de data e hora
<!-- keywords: funções, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC -->

Estas são as funções de data e hora que o JeffCrunch pode calcular. Cada uma mostra como é escrita, o que faz e o que é cada um de seus argumentos. Um argumento entre colchetes é opcional, e "..." significa que você pode informar quantos outros do último tipo forem necessários. Para experimentar uma delas, veja [Escolher uma Função](insert-function). Enquanto você digita uma função em uma célula, uma pequena dica abaixo da célula mostra os argumentos dela, e um clique no nome da função na dica abre o item correspondente aqui.

## DATE

`DATE(year, month, day)`

Cria uma data a partir de um ano, um mês e um dia.

- `year`: O ano, como 2027.
- `month`: O mês, de 1 a 12.
- `day`: O dia do mês, de 1 a 31.

## DATEDIF

`DATEDIF(start_date, end_date, unit)`

Retorna os anos, meses ou dias entre duas datas.

- `start_date`: A data em que o período começa.
- `end_date`: A data em que o período termina.
- `unit`: O que contar: "Y" anos, "M" meses, "D" dias, "MD" dias ignorando meses e anos, "YM" meses ignorando anos, "YD" dias ignorando anos.

## DATEVALUE

`DATEVALUE(date_text)`

Converte uma data escrita como texto em uma data.

- `date_text`: Uma data escrita como texto, como "15/03/2027".

## DAY

`DAY(serial_number)`

Retorna o dia do mês de uma data.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.

## DAYS

`DAYS(end_date, start_date)`

Conta os dias entre duas datas.

- `end_date`: A data em que o período termina.
- `start_date`: A data em que o período começa.

## DAYS360

`DAYS360(start_date, end_date, [method])`

Conta os dias entre duas datas em um ano de doze meses de 30 dias.

- `start_date`: A data em que o período começa.
- `end_date`: A data em que o período termina.
- `method` (opcional): Como os meses são contados: falso ou omitido para o método dos EUA, verdadeiro para o método europeu.

## EDATE

`EDATE(start_date, months)`

Retorna a data de um número de meses antes ou depois de uma data.

- `start_date`: A data em que o período começa.
- `months`: Quantos meses somar (negativo para voltar).

## EOMONTH

`EOMONTH(start_date, months)`

Retorna o último dia do mês de um número de meses antes ou depois de uma data.

- `start_date`: A data em que o período começa.
- `months`: Quantos meses somar (negativo para voltar).

## HOUR

`HOUR(serial_number)`

Retorna a hora de um horário.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.

## ISOWEEKNUM

`ISOWEEKNUM(date)`

Retorna o número internacional da semana de uma data.

- `date`: Uma data, ou uma célula que contenha uma.

## MINUTE

`MINUTE(serial_number)`

Retorna os minutos de um horário.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.

## MONTH

`MONTH(serial_number)`

Retorna o mês de uma data, de 1 a 12.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.

## NETWORKDAYS

`NETWORKDAYS(start_date, end_date, [holidays])`

Conta os dias úteis entre duas datas.

- `start_date`: A data em que o período começa.
- `end_date`: A data em que o período termina.
- `holidays` (opcional): Um intervalo de células com datas que não são dias úteis.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(start_date, end_date, [weekend], [holidays])`

Conta os dias úteis entre duas datas, com os dias do fim de semana que você escolher.

- `start_date`: A data em que o período começa.
- `end_date`: A data em que o período termina.
- `weekend` (opcional): Quais dias são o fim de semana: um código de 1 a 7 ou de 11 a 17, ou sete caracteres de 0 e 1 a partir de segunda-feira, com 1 para um dia de folga.
- `holidays` (opcional): Um intervalo de células com datas que não são dias úteis.

## NOW

`NOW()`

Retorna a data e a hora atuais.

## SECOND

`SECOND(serial_number)`

Retorna os segundos de um horário.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.

## TIME

`TIME(hour, minute, second)`

Cria um horário a partir de uma hora, de minutos e de segundos.

- `hour`: A hora, de 0 a 23.
- `minute`: O minuto, de 0 a 59.
- `second`: O segundo, de 0 a 59.

## TIMEVALUE

`TIMEVALUE(time_text)`

Converte um horário escrito como texto em um horário.

- `time_text`: Um horário escrito como texto, como "2:30 PM".

## TODAY

`TODAY()`

Retorna a data de hoje.

## WEEKDAY

`WEEKDAY(serial_number, [return_type])`

Retorna o dia da semana de uma data como um número.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.
- `return_type` (opcional): Em que dia a semana começa: 1 ou omitido para domingo, 2 para segunda-feira.

## WEEKNUM

`WEEKNUM(serial_number, [return_type])`

Retorna o número da semana do ano em que uma data se encontra.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.
- `return_type` (opcional): Em que dia a semana começa: 1 ou omitido para domingo, 2 para segunda-feira.

## WORKDAY

`WORKDAY(start_date, days, [holidays])`

Retorna a data de um número de dias úteis antes ou depois de uma data.

- `start_date`: A data em que o período começa.
- `days`: O número de dias a somar (negativo para voltar).
- `holidays` (opcional): Um intervalo de células com datas que não são dias úteis.

## WORKDAY.INTL

`WORKDAY.INTL(start_date, days, [weekend], [holidays])`

Retorna a data de um número de dias úteis a partir de uma data inicial, com os dias do fim de semana que você escolher.

- `start_date`: A data em que o período começa.
- `days`: O número de dias a somar (negativo para voltar).
- `weekend` (opcional): Quais dias são o fim de semana: um código de 1 a 7 ou de 11 a 17, ou sete caracteres de 0 e 1 a partir de segunda-feira, com 1 para um dia de folga.
- `holidays` (opcional): Um intervalo de células com datas que não são dias úteis.

## YEAR

`YEAR(serial_number)`

Retorna o ano de uma data.

- `serial_number`: Uma data ou hora, ou uma célula que contenha uma.

## YEARFRAC

`YEARFRAC(start_date, end_date, [basis])`

Retorna a parte de um ano entre duas datas.

- `start_date`: A data em que o período começa.
- `end_date`: A data em que o período termina.
- `basis` (opcional): Como os dias de um ano são contados: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais em um ano de 360 dias, 3 dias reais em um ano de 365 dias, 4 método europeu 30/360.
