# Funciones de fecha y hora
<!-- keywords: funciones, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC, fórmulas, fecha, hora, FECHA, FECHANUMERO, DIA, DIAS, DIAS360, FECHA.MES, FIN.MES, HORA, NUM.DE.SEMANA.ISO, MINUTO, MES, DIAS.LAB, DIAS.LAB.INTL, AHORA, SEGUNDO, HORA.VALOR, HOY, DIA.SEMANA, NUM.DE.SEMANA, DIA.LAB, DIA.LAB.INTL, AÑO, FRAC.AÑO -->

Estas son las funciones de fecha y hora que JeffCrunch puede calcular. Cada una muestra cómo se escribe, qué hace y qué es cada uno de sus argumentos. Un argumento entre corchetes es opcional, y «...» significa que puede dar tantos más del último como necesite. Para probar una, consulte [Elegir una función](insert-function). Mientras escribe una función en una celda, aparece una pequeña sugerencia debajo de la celda con sus argumentos, y si hace clic en el nombre de la función en la sugerencia, se abre su entrada en esta página.

## DATE

`DATE(year, month, day)`

Crea una fecha a partir de un año, un mes y un día.

- `year`: El año, por ejemplo 2027.
- `month`: El mes, de 1 a 12.
- `day`: El día del mes, de 1 a 31.

## DATEDIF

`DATEDIF(start_date, end_date, unit)`

Devuelve los años, meses o días que hay entre dos fechas.

- `start_date`: La fecha en que empieza el período.
- `end_date`: La fecha en que termina el período.
- `unit`: Lo que se cuenta: «Y» años, «M» meses, «D» días, «MD» días sin tener en cuenta meses ni años, «YM» meses sin tener en cuenta años, «YD» días sin tener en cuenta años.

## DATEVALUE

`DATEVALUE(date_text)`

Convierte una fecha escrita como texto en una fecha.

- `date_text`: Una fecha escrita como texto, por ejemplo «15/3/2027».

## DAY

`DAY(serial_number)`

Devuelve el día del mes de una fecha.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.

## DAYS

`DAYS(end_date, start_date)`

Cuenta los días que hay entre dos fechas.

- `end_date`: La fecha en que termina el período.
- `start_date`: La fecha en que empieza el período.

## DAYS360

`DAYS360(start_date, end_date, [method])`

Cuenta los días que hay entre dos fechas en un año de doce meses de 30 días.

- `start_date`: La fecha en que empieza el período.
- `end_date`: La fecha en que termina el período.
- `method` (opcional): Cómo se cuentan los meses: falso o se omite para el método de EE. UU., verdadero para el método europeo.

## EDATE

`EDATE(start_date, months)`

Devuelve la fecha que se encuentra un número de meses antes o después de una fecha.

- `start_date`: La fecha en que empieza el período.
- `months`: Cuántos meses sumar (negativo para retroceder).

## EOMONTH

`EOMONTH(start_date, months)`

Devuelve el último día del mes que se encuentra un número de meses antes o después de una fecha.

- `start_date`: La fecha en que empieza el período.
- `months`: Cuántos meses sumar (negativo para retroceder).

## HOUR

`HOUR(serial_number)`

Devuelve la hora de un valor de hora.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.

## ISOWEEKNUM

`ISOWEEKNUM(date)`

Devuelve el número de semana ISO de una fecha.

- `date`: Una fecha, o una celda que contenga una.

## MINUTE

`MINUTE(serial_number)`

Devuelve los minutos de un valor de hora.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.

## MONTH

`MONTH(serial_number)`

Devuelve el mes de una fecha, de 1 a 12.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.

## NETWORKDAYS

`NETWORKDAYS(start_date, end_date, [holidays])`

Cuenta los días laborables que hay entre dos fechas.

- `start_date`: La fecha en que empieza el período.
- `end_date`: La fecha en que termina el período.
- `holidays` (opcional): Un rango de celdas con fechas que no son días laborables.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(start_date, end_date, [weekend], [holidays])`

Cuenta los días laborables que hay entre dos fechas, con los días del fin de semana que elija.

- `start_date`: La fecha en que empieza el período.
- `end_date`: La fecha en que termina el período.
- `weekend` (opcional): Qué días son el fin de semana: un código del 1 al 7 o del 11 al 17, o siete caracteres formados por 0 y 1 a partir del lunes, con 1 para un día libre.
- `holidays` (opcional): Un rango de celdas con fechas que no son días laborables.

## NOW

`NOW()`

Devuelve la fecha y la hora actuales.

## SECOND

`SECOND(serial_number)`

Devuelve los segundos de un valor de hora.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.

## TIME

`TIME(hour, minute, second)`

Crea una hora a partir de una hora, minutos y segundos.

- `hour`: La hora, de 0 a 23.
- `minute`: El minuto, de 0 a 59.
- `second`: El segundo, de 0 a 59.

## TIMEVALUE

`TIMEVALUE(time_text)`

Convierte una hora escrita como texto en una hora.

- `time_text`: Una hora escrita como texto, por ejemplo «14:30».

## TODAY

`TODAY()`

Devuelve la fecha de hoy.

## WEEKDAY

`WEEKDAY(serial_number, [return_type])`

Devuelve el día de la semana de una fecha como número.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.
- `return_type` (opcional): Qué día empieza la semana: 1 o se omite para domingo, 2 para lunes.

## WEEKNUM

`WEEKNUM(serial_number, [return_type])`

Devuelve el número de la semana del año en la que cae una fecha.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.
- `return_type` (opcional): Qué día empieza la semana: 1 o se omite para domingo, 2 para lunes.

## WORKDAY

`WORKDAY(start_date, days, [holidays])`

Devuelve la fecha que se encuentra un número de días laborables antes o después de una fecha.

- `start_date`: La fecha en que empieza el período.
- `days`: El número de días que sumar (negativo para retroceder).
- `holidays` (opcional): Un rango de celdas con fechas que no son días laborables.

## WORKDAY.INTL

`WORKDAY.INTL(start_date, days, [weekend], [holidays])`

Devuelve la fecha que se encuentra un número de días laborables desde una fecha de inicio, con los días del fin de semana que elija.

- `start_date`: La fecha en que empieza el período.
- `days`: El número de días que sumar (negativo para retroceder).
- `weekend` (opcional): Qué días son el fin de semana: un código del 1 al 7 o del 11 al 17, o siete caracteres formados por 0 y 1 a partir del lunes, con 1 para un día libre.
- `holidays` (opcional): Un rango de celdas con fechas que no son días laborables.

## YEAR

`YEAR(serial_number)`

Devuelve el año de una fecha.

- `serial_number`: Una fecha o una hora, o una celda que contenga una.

## YEARFRAC

`YEARFRAC(start_date, end_date, [basis])`

Devuelve la parte de un año que hay entre dos fechas.

- `start_date`: La fecha en que empieza el período.
- `end_date`: La fecha en que termina el período.
- `basis` (opcional): Cómo se cuentan los días de un año: 0 o se omite para 30/360, 1 días reales, 2 días reales en un año de 360 días, 3 días reales en un año de 365 días, 4 30/360 europeo.
