# Informatiefuncties
<!-- keywords: functions, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE -->

Dit zijn de informatiefuncties die JeffCrunch kan berekenen. Elke functie laat zien hoe ze wordt geschreven, wat ze doet en wat elk van haar argumenten is. Een argument tussen haakjes is optioneel, en "..." betekent dat u zoveel extra argumenten van het laatste type kunt opgeven als nodig is. Om er een uit te proberen, zie [Een functie kiezen](insert-function). Terwijl u in een cel een functie typt, toont een kleine tip onder de cel de argumenten ervan, en een klik op de naam van de functie in de tip opent het bijbehorende onderdeel op deze pagina.

## CELL

`CELL(info_type, [reference])`

Geeft informatie over een cel, zoals het adres of de opmaak.

- `info_type`: De informatie die u over de cel wilt weergeven, als tekst zoals "address", "row", "col" of "format".
- `reference` (optioneel): De cel waarover informatie wordt gegeven (de cel met de invoegpositie als dit wordt weggelaten).

## ERROR.TYPE

`ERROR.TYPE(error_value)`

Geeft het getal dat voor een soort fout staat.

- `error_value`: De fout die moet worden opgezocht, of een cel die deze bevat.

## INFO

`INFO(info_type)`

Geeft informatie over het programma of de werkmap, zoals de map of de manier waarop wordt berekend.

- `info_type`: De informatie die u over de cel wilt weergeven, als tekst zoals "address", "row", "col" of "format".

## ISBLANK

`ISBLANK(value)`

Waar als de cel leeg is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISERR

`ISERR(value)`

Waar als de waarde een fout is, anders dan 'niet beschikbaar'.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISERROR

`ISERROR(value)`

Waar als de waarde een fout is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISEVEN

`ISEVEN(number)`

Waar als een geheel getal even is.

- `number`: Het getal waarmee wordt gewerkt.

## ISFORMULA

`ISFORMULA(reference)`

Waar wanneer de cel een formule bevat.

- `reference`: Een cel of celbereik.

## ISLOGICAL

`ISLOGICAL(value)`

Waar als de waarde waar of onwaar is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISNA

`ISNA(value)`

Waar als de waarde de fout 'niet beschikbaar' is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISNONTEXT

`ISNONTEXT(value)`

Waar als de waarde geen tekst is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISNUMBER

`ISNUMBER(value)`

Waar als de waarde een getal is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISODD

`ISODD(number)`

Waar als een geheel getal oneven is.

- `number`: Het getal waarmee wordt gewerkt.

## ISREF

`ISREF(value)`

Waar als de waarde een verwijzing naar cellen is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## ISTEXT

`ISTEXT(value)`

Waar als de waarde tekst is.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## N

`N(value)`

Zet een waarde om in een getal: datums en de waarde waar worden getallen, tekst wordt nul.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## NA

`NA()`

Geeft de fout 'niet beschikbaar', om een ontbrekende waarde aan te geven.

## SHEET

`SHEET([value])`

Geeft het nummer van een blad, waarbij het eerste blad als 1 telt.

- `value` (optioneel): De waarde waarmee wordt gewerkt, of een cel die deze bevat.

## SHEETS

`SHEETS([reference])`

Geeft het aantal bladen in een verwijzing, of in de werkmap.

- `reference` (optioneel): Een cel of celbereik.

## TYPE

`TYPE(value)`

Geeft een getal dat aangeeft wat voor soort waarde het is: getal, tekst, logische waarde, fout of matrix.

- `value`: De waarde waarmee wordt gewerkt, of een cel die deze bevat.
