# Date and time functions
<!-- keywords: functions, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC -->

These are the date and time functions that JeffCrunch can work out. Each one shows how it is written, what it does, and what each of its arguments is. An argument in brackets is optional, and "..." means you can give as many more of the last one as you need. To try one, see [Pick a function](insert-function). While you type a function in a cell, a small tip under the cell shows its arguments, and a click on the function's name in the tip opens its entry here.

## DATE

`DATE(year, month, day)`

Makes a date from a year, a month, and a day.

- `year`: The year, such as 2027.
- `month`: The month, from 1 to 12.
- `day`: The day of the month, from 1 to 31.

## DATEDIF

`DATEDIF(start_date, end_date, unit)`

Gives the years, months, or days between two dates.

- `start_date`: The date the period starts.
- `end_date`: The date the period ends.
- `unit`: What to count in: "Y" years, "M" months, "D" days, "MD" days ignoring months and years, "YM" months ignoring years, "YD" days ignoring years.

## DATEVALUE

`DATEVALUE(date_text)`

Turns a date written as text into a date.

- `date_text`: A date written as text, such as "3/15/2027".

## DAY

`DAY(serial_number)`

Gives the day of the month of a date.

- `serial_number`: A date or time, or a cell that holds one.

## DAYS

`DAYS(end_date, start_date)`

Counts the days between two dates.

- `end_date`: The date the period ends.
- `start_date`: The date the period starts.

## DAYS360

`DAYS360(start_date, end_date, [method])`

Counts the days between two dates in a year of twelve 30-day months.

- `start_date`: The date the period starts.
- `end_date`: The date the period ends.
- `method` (optional): How months are counted: false or left out for the US way, true for the European way.

## EDATE

`EDATE(start_date, months)`

Gives the date a number of months before or after a date.

- `start_date`: The date the period starts.
- `months`: How many months to add (negative to go back).

## EOMONTH

`EOMONTH(start_date, months)`

Gives the last day of the month a number of months before or after a date.

- `start_date`: The date the period starts.
- `months`: How many months to add (negative to go back).

## HOUR

`HOUR(serial_number)`

Gives the hour of a time.

- `serial_number`: A date or time, or a cell that holds one.

## ISOWEEKNUM

`ISOWEEKNUM(date)`

Gives the international week number of a date.

- `date`: A date, or a cell that holds one.

## MINUTE

`MINUTE(serial_number)`

Gives the minutes of a time.

- `serial_number`: A date or time, or a cell that holds one.

## MONTH

`MONTH(serial_number)`

Gives the month of a date, from 1 to 12.

- `serial_number`: A date or time, or a cell that holds one.

## NETWORKDAYS

`NETWORKDAYS(start_date, end_date, [holidays])`

Counts the working days between two dates.

- `start_date`: The date the period starts.
- `end_date`: The date the period ends.
- `holidays` (optional): A block of cells with dates that are not working days.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(start_date, end_date, [weekend], [holidays])`

Counts the working days between two dates, with the days of the weekend of your choice.

- `start_date`: The date the period starts.
- `end_date`: The date the period ends.
- `weekend` (optional): Which days are the weekend: a code from 1 to 7 or 11 to 17, or seven characters of 0 and 1 from Monday on, with 1 for a day off.
- `holidays` (optional): A block of cells with dates that are not working days.

## NOW

`NOW()`

Gives the date and time right now.

## SECOND

`SECOND(serial_number)`

Gives the seconds of a time.

- `serial_number`: A date or time, or a cell that holds one.

## TIME

`TIME(hour, minute, second)`

Makes a time from an hour, minutes, and seconds.

- `hour`: The hour, from 0 to 23.
- `minute`: The minute, from 0 to 59.
- `second`: The second, from 0 to 59.

## TIMEVALUE

`TIMEVALUE(time_text)`

Turns a time written as text into a time.

- `time_text`: A time written as text, such as "2:30 PM".

## TODAY

`TODAY()`

Gives today's date.

## WEEKDAY

`WEEKDAY(serial_number, [return_type])`

Gives the day of the week of a date as a number.

- `serial_number`: A date or time, or a cell that holds one.
- `return_type` (optional): Which day the week starts on: 1 or left out for Sunday, 2 for Monday.

## WEEKNUM

`WEEKNUM(serial_number, [return_type])`

Gives the number of the week of the year that a date falls in.

- `serial_number`: A date or time, or a cell that holds one.
- `return_type` (optional): Which day the week starts on: 1 or left out for Sunday, 2 for Monday.

## WORKDAY

`WORKDAY(start_date, days, [holidays])`

Gives the date a number of working days before or after a date.

- `start_date`: The date the period starts.
- `days`: The number of days to add (negative to go back).
- `holidays` (optional): A block of cells with dates that are not working days.

## WORKDAY.INTL

`WORKDAY.INTL(start_date, days, [weekend], [holidays])`

Gives the date a number of working days from a start date, with the days of the weekend of your choice.

- `start_date`: The date the period starts.
- `days`: The number of days to add (negative to go back).
- `weekend` (optional): Which days are the weekend: a code from 1 to 7 or 11 to 17, or seven characters of 0 and 1 from Monday on, with 1 for a day off.
- `holidays` (optional): A block of cells with dates that are not working days.

## YEAR

`YEAR(serial_number)`

Gives the year of a date.

- `serial_number`: A date or time, or a cell that holds one.

## YEARFRAC

`YEARFRAC(start_date, end_date, [basis])`

Gives the part of a year between two dates.

- `start_date`: The date the period starts.
- `end_date`: The date the period ends.
- `basis` (optional): How the days of a year are counted: 0 or left out for 30/360, 1 actual days, 2 actual days in a 360-day year, 3 actual days in a 365-day year, 4 European 30/360.
