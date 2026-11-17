# Datum- en tijdfuncties
<!-- keywords: functies, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC -->

Dit zijn de datum- en tijdfuncties die JeffCrunch kan berekenen. Bij elke functie ziet u hoe deze wordt geschreven, wat de functie doet en wat elk van de argumenten betekent. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste type kunt opgeven als nodig is. Om een functie te proberen, zie [Een functie kiezen](insert-function). Terwijl u een functie in een cel typt, toont een kleine tip onder de cel de argumenten ervan. Als u op de naam van de functie in de tip klikt, opent u het bijbehorende onderdeel op deze pagina.

## DATE

`DATE(jaar; maand; dag)`

Maakt een datum van een jaar, een maand en een dag.

- `jaar`: Het jaar, bijvoorbeeld 2027.
- `maand`: De maand, van 1 tot 12.
- `dag`: De dag van de maand, van 1 tot 31.

## DATEDIF

`DATEDIF(startdatum; einddatum; eenheid)`

Geeft het aantal jaren, maanden of dagen tussen twee datums.

- `startdatum`: De datum waarop de periode begint.
- `einddatum`: De datum waarop de periode eindigt.
- `eenheid`: Wat er wordt geteld: "Y" jaren, "M" maanden, "D" dagen, "MD" dagen zonder maanden en jaren, "YM" maanden zonder jaren, "YD" dagen zonder jaren.

## DATEVALUE

`DATEVALUE(datumtekst)`

Zet een datum die als tekst is geschreven om in een datum.

- `datumtekst`: Een datum als tekst, zoals "15-3-2027".

## DAY

`DAY(getal)`

Geeft de dag van de maand van een datum.

- `getal`: Een datum of tijd, of een cel die er een bevat.

## DAYS

`DAYS(einddatum; startdatum)`

Telt de dagen tussen twee datums.

- `einddatum`: De datum waarop de periode eindigt.
- `startdatum`: De datum waarop de periode begint.

## DAYS360

`DAYS360(startdatum; einddatum; [methode])`

Telt de dagen tussen twee datums in een jaar van twaalf maanden van 30 dagen.

- `startdatum`: De datum waarop de periode begint.
- `einddatum`: De datum waarop de periode eindigt.
- `methode` (optioneel): Hoe maanden worden geteld: onwaar of weggelaten voor de Amerikaanse methode, waar voor de Europese methode.

## EDATE

`EDATE(startdatum; maanden)`

Geeft de datum een aantal maanden vóór of na een datum.

- `startdatum`: De datum waarop de periode begint.
- `maanden`: Het aantal maanden dat wordt opgeteld (negatief om terug te gaan).

## EOMONTH

`EOMONTH(startdatum; maanden)`

Geeft de laatste dag van de maand een aantal maanden vóór of na een datum.

- `startdatum`: De datum waarop de periode begint.
- `maanden`: Het aantal maanden dat wordt opgeteld (negatief om terug te gaan).

## HOUR

`HOUR(getal)`

Geeft het uur van een tijd.

- `getal`: Een datum of tijd, of een cel die er een bevat.

## ISOWEEKNUM

`ISOWEEKNUM(datum)`

Geeft het internationale weeknummer van een datum.

- `datum`: Een datum, of een cel die er een bevat.

## MINUTE

`MINUTE(getal)`

Geeft de minuten van een tijd.

- `getal`: Een datum of tijd, of een cel die er een bevat.

## MONTH

`MONTH(getal)`

Geeft de maand van een datum, van 1 tot 12.

- `getal`: Een datum of tijd, of een cel die er een bevat.

## NETWORKDAYS

`NETWORKDAYS(startdatum; einddatum; [feestdagen])`

Telt de werkdagen tussen twee datums.

- `startdatum`: De datum waarop de periode begint.
- `einddatum`: De datum waarop de periode eindigt.
- `feestdagen` (optioneel): Een celbereik met datums die geen werkdagen zijn.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(startdatum; einddatum; [weekend]; [feestdagen])`

Telt de werkdagen tussen twee datums, met de weekenddagen naar keuze.

- `startdatum`: De datum waarop de periode begint.
- `einddatum`: De datum waarop de periode eindigt.
- `weekend` (optioneel): Welke dagen het weekend zijn: een code van 1 tot 7 of 11 tot 17, of zeven tekens van 0 en 1 vanaf maandag, waarbij 1 een vrije dag is.
- `feestdagen` (optioneel): Een celbereik met datums die geen werkdagen zijn.

## NOW

`NOW()`

Geeft de huidige datum en tijd.

## SECOND

`SECOND(getal)`

Geeft de seconden van een tijd.

- `getal`: Een datum of tijd, of een cel die er een bevat.

## TIME

`TIME(uur; minuut; seconde)`

Maakt een tijd van een uur, minuten en seconden.

- `uur`: Het uur, van 0 tot 23.
- `minuut`: De minuut, van 0 tot 59.
- `seconde`: De seconde, van 0 tot 59.

## TIMEVALUE

`TIMEVALUE(tijdtekst)`

Zet een tijd die als tekst is geschreven om in een tijd.

- `tijdtekst`: Een tijd als tekst, zoals "14:30".

## TODAY

`TODAY()`

Geeft de datum van vandaag.

## WEEKDAY

`WEEKDAY(getal; [type_retour])`

Geeft de dag van de week van een datum als getal.

- `getal`: Een datum of tijd, of een cel die er een bevat.
- `type_retour` (optioneel): Op welke dag de week begint: 1 of weggelaten voor zondag, 2 voor maandag.

## WEEKNUM

`WEEKNUM(getal; [type_retour])`

Geeft het nummer van de week van het jaar waarin een datum valt.

- `getal`: Een datum of tijd, of een cel die er een bevat.
- `type_retour` (optioneel): Op welke dag de week begint: 1 of weggelaten voor zondag, 2 voor maandag.

## WORKDAY

`WORKDAY(startdatum; dagen; [feestdagen])`

Geeft de datum een aantal werkdagen vóór of na een datum.

- `startdatum`: De datum waarop de periode begint.
- `dagen`: Het aantal dagen dat wordt opgeteld (negatief om terug te gaan).
- `feestdagen` (optioneel): Een celbereik met datums die geen werkdagen zijn.

## WORKDAY.INTL

`WORKDAY.INTL(startdatum; dagen; [weekend]; [feestdagen])`

Geeft de datum een aantal werkdagen vóór of na een startdatum, met de weekenddagen naar keuze.

- `startdatum`: De datum waarop de periode begint.
- `dagen`: Het aantal dagen dat wordt opgeteld (negatief om terug te gaan).
- `weekend` (optioneel): Welke dagen het weekend zijn: een code van 1 tot 7 of 11 tot 17, of zeven tekens van 0 en 1 vanaf maandag, waarbij 1 een vrije dag is.
- `feestdagen` (optioneel): Een celbereik met datums die geen werkdagen zijn.

## YEAR

`YEAR(getal)`

Geeft het jaar van een datum.

- `getal`: Een datum of tijd, of een cel die er een bevat.

## YEARFRAC

`YEARFRAC(startdatum; einddatum; [basis])`

Geeft het deel van een jaar tussen twee datums.

- `startdatum`: De datum waarop de periode begint.
- `einddatum`: De datum waarop de periode eindigt.
- `basis` (optioneel): Hoe de dagen van een jaar worden geteld: 0 of weggelaten voor 30/360, 1 werkelijke dagen, 2 werkelijke dagen in een jaar van 360 dagen, 3 werkelijke dagen in een jaar van 365 dagen, 4 Europese 30/360.
