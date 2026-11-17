# Funções de data e hora
<!-- keywords: funções, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC -->

Estas são as funções de data e hora que o JeffCrunch consegue calcular. Cada uma mostra como se escreve, o que faz e qual é o significado de cada um dos respetivos argumentos. Um argumento entre parênteses retos é opcional, e "..." significa que pode indicar tantos argumentos adicionais quantos precisar, a seguir ao último. Para experimentar uma delas, consulte [Escolher uma função](insert-function). Enquanto escreve uma função numa célula, uma pequena sugestão por baixo da célula mostra os respetivos argumentos, e se clicar no nome da função nessa sugestão, abre-se a respetiva entrada nesta página.

## DATE

`DATE(ano; mês; dia)`

Cria uma data a partir de um ano, um mês e um dia.

- `ano`: O ano, como 2027.
- `mês`: O mês, de 1 a 12.
- `dia`: O dia do mês, de 1 a 31.

## DATEDIF

`DATEDIF(data_início; data_fim; unidade)`

Devolve os anos, os meses ou os dias entre duas datas.

- `data_início`: A data em que o período começa.
- `data_fim`: A data em que o período termina.
- `unidade`: O que contar: "Y" anos, "M" meses, "D" dias, "MD" dias ignorando meses e anos, "YM" meses ignorando anos, "YD" dias ignorando anos.

## DATEVALUE

`DATEVALUE(texto_data)`

Converte uma data escrita como texto numa data.

- `texto_data`: Uma data escrita como texto, como "15/03/2027".

## DAY

`DAY(número_de_série)`

Devolve o dia do mês de uma data.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.

## DAYS

`DAYS(data_fim; data_início)`

Conta os dias entre duas datas.

- `data_fim`: A data em que o período termina.
- `data_início`: A data em que o período começa.

## DAYS360

`DAYS360(data_início; data_fim; [método])`

Conta os dias entre duas datas num ano de doze meses de 30 dias.

- `data_início`: A data em que o período começa.
- `data_fim`: A data em que o período termina.
- `método` (opcional): A forma como os meses são contados: falso ou omitido para o método norte-americano, verdadeiro para o método europeu.

## EDATE

`EDATE(data_início; meses)`

Devolve a data que fica um determinado número de meses antes ou depois de uma data.

- `data_início`: A data em que o período começa.
- `meses`: Quantos meses adicionar (negativo para recuar).

## EOMONTH

`EOMONTH(data_início; meses)`

Devolve o último dia do mês que fica um determinado número de meses antes ou depois de uma data.

- `data_início`: A data em que o período começa.
- `meses`: Quantos meses adicionar (negativo para recuar).

## HOUR

`HOUR(número_de_série)`

Devolve a hora de um valor de tempo.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.

## ISOWEEKNUM

`ISOWEEKNUM(data)`

Devolve o número da semana internacional de uma data.

- `data`: Uma data, ou uma célula que contenha uma data.

## MINUTE

`MINUTE(número_de_série)`

Devolve os minutos de uma hora.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.

## MONTH

`MONTH(número_de_série)`

Devolve o mês de uma data, de 1 a 12.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.

## NETWORKDAYS

`NETWORKDAYS(data_início; data_fim; [feriados])`

Conta os dias úteis entre duas datas.

- `data_início`: A data em que o período começa.
- `data_fim`: A data em que o período termina.
- `feriados` (opcional): Um intervalo de células com datas que não são dias úteis.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(data_início; data_fim; [fim_de_semana]; [feriados])`

Conta os dias úteis entre duas datas, com os dias do fim de semana à sua escolha.

- `data_início`: A data em que o período começa.
- `data_fim`: A data em que o período termina.
- `fim_de_semana` (opcional): Quais são os dias do fim de semana: um código de 1 a 7 ou de 11 a 17, ou sete carateres de 0 e 1 a começar na segunda-feira, em que 1 representa um dia de folga.
- `feriados` (opcional): Um intervalo de células com datas que não são dias úteis.

## NOW

`NOW()`

Devolve a data e a hora atuais.

## SECOND

`SECOND(número_de_série)`

Devolve os segundos de uma hora.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.

## TIME

`TIME(hora; minuto; segundo)`

Cria uma hora a partir de horas, minutos e segundos.

- `hora`: A hora, de 0 a 23.
- `minuto`: O minuto, de 0 a 59.
- `segundo`: O segundo, de 0 a 59.

## TIMEVALUE

`TIMEVALUE(texto_hora)`

Converte uma hora escrita como texto numa hora.

- `texto_hora`: Uma hora escrita como texto, como "14:30".

## TODAY

`TODAY()`

Devolve a data de hoje.

## WEEKDAY

`WEEKDAY(número_de_série; [tipo_de_retorno])`

Devolve o dia da semana de uma data sob a forma de número.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.
- `tipo_de_retorno` (opcional): Em que dia começa a semana: 1 ou omitido para domingo, 2 para segunda-feira.

## WEEKNUM

`WEEKNUM(número_de_série; [tipo_de_retorno])`

Devolve o número da semana do ano em que uma data se encontra.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.
- `tipo_de_retorno` (opcional): Em que dia começa a semana: 1 ou omitido para domingo, 2 para segunda-feira.

## WORKDAY

`WORKDAY(data_início; dias; [feriados])`

Devolve a data que fica um determinado número de dias úteis antes ou depois de uma data.

- `data_início`: A data em que o período começa.
- `dias`: O número de dias a adicionar (negativo para recuar).
- `feriados` (opcional): Um intervalo de células com datas que não são dias úteis.

## WORKDAY.INTL

`WORKDAY.INTL(data_início; dias; [fim_de_semana]; [feriados])`

Devolve a data que fica um determinado número de dias úteis a partir de uma data de início, com os dias do fim de semana à sua escolha.

- `data_início`: A data em que o período começa.
- `dias`: O número de dias a adicionar (negativo para recuar).
- `fim_de_semana` (opcional): Quais são os dias do fim de semana: um código de 1 a 7 ou de 11 a 17, ou sete carateres de 0 e 1 a começar na segunda-feira, em que 1 representa um dia de folga.
- `feriados` (opcional): Um intervalo de células com datas que não são dias úteis.

## YEAR

`YEAR(número_de_série)`

Devolve o ano de uma data.

- `número_de_série`: Uma data ou hora, ou uma célula que contenha uma delas.

## YEARFRAC

`YEARFRAC(data_início; data_fim; [base])`

Devolve a parte de um ano que existe entre duas datas.

- `data_início`: A data em que o período começa.
- `data_fim`: A data em que o período termina.
- `base` (opcional): A forma como são contados os dias de um ano: 0 ou omitido para 30/360, 1 dias reais, 2 dias reais num ano de 360 dias, 3 dias reais num ano de 365 dias, 4 30/360 europeu.
