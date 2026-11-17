# Informationsfunktionen
<!-- keywords: Funktionen, CELL, ERROR.TYPE, INFO, ISBLANK, ISERR, ISERROR, ISEVEN, ISFORMULA, ISLOGICAL, ISNA, ISNONTEXT, ISNUMBER, ISODD, ISREF, ISTEXT, N, NA, SHEET, SHEETS, TYPE -->

Dies sind die Informationsfunktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie eine Funktion in einer Zelle eingeben, zeigt ein kleiner Tipp unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Tipp öffnet ihren Eintrag hier.

## CELL

`CELL(info_type, [reference])`

Liefert Angaben über eine Zelle, etwa ihre Adresse oder ihr Format.

- `info_type`: Welche Angabe über die Zelle geliefert werden soll, als Text, etwa „address“, „row“, „col“ oder „format“.
- `reference` (optional): Die Zelle, über die Angaben geliefert werden sollen (ohne Angabe die Zelle mit der Einfügemarke).

## ERROR.TYPE

`ERROR.TYPE(error_value)`

Liefert die Zahl, die für eine Fehlerart steht.

- `error_value`: Der Fehler, der nachgeschlagen werden soll, oder eine Zelle, die ihn enthält.

## INFO

`INFO(info_type)`

Liefert Angaben über das Programm oder die Arbeitsmappe, etwa den Ordner oder die Berechnungsart.

- `info_type`: Welche Angabe über die Zelle geliefert werden soll, als Text, etwa „address“, „row“, „col“ oder „format“.

## ISBLANK

`ISBLANK(value)`

WAHR, wenn die Zelle leer ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISERR

`ISERR(value)`

WAHR, wenn der Wert ein Fehler ist, außer „Nicht verfügbar“.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISERROR

`ISERROR(value)`

WAHR, wenn der Wert ein beliebiger Fehler ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISEVEN

`ISEVEN(number)`

WAHR, wenn eine ganze Zahl gerade ist.

- `number`: Die Zahl, mit der gearbeitet wird.

## ISFORMULA

`ISFORMULA(reference)`

WAHR, wenn die Zelle eine Formel enthält.

- `reference`: Eine Zelle oder ein Zellbereich.

## ISLOGICAL

`ISLOGICAL(value)`

WAHR, wenn der Wert WAHR oder FALSCH ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISNA

`ISNA(value)`

WAHR, wenn der Wert der Fehler „Nicht verfügbar“ ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISNONTEXT

`ISNONTEXT(value)`

WAHR, wenn der Wert kein Text ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISNUMBER

`ISNUMBER(value)`

WAHR, wenn der Wert eine Zahl ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISODD

`ISODD(number)`

WAHR, wenn eine ganze Zahl ungerade ist.

- `number`: Die Zahl, mit der gearbeitet wird.

## ISREF

`ISREF(value)`

WAHR, wenn der Wert ein Verweis auf Zellen ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## ISTEXT

`ISTEXT(value)`

WAHR, wenn der Wert Text ist.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## N

`N(value)`

Wandelt einen Wert in eine Zahl um: Datumswerte und WAHR werden zu Zahlen, Text wird zu null.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## NA

`NA()`

Liefert den Fehler „Nicht verfügbar“, um einen fehlenden Wert zu kennzeichnen.

## SHEET

`SHEET([value])`

Liefert die Nummer eines Blatts, wobei das erste Blatt als 1 gezählt wird.

- `value` (optional): Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## SHEETS

`SHEETS([reference])`

Liefert die Anzahl der Blätter in einem Bezug oder in der Arbeitsmappe.

- `reference` (optional): Eine Zelle oder ein Zellbereich.

## TYPE

`TYPE(value)`

Liefert eine Zahl, die angibt, um welche Art von Wert es sich handelt: Zahl, Text, Wahrheitswert, Fehler oder Matrix.

- `value`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
