# Datenbankfunktionen
<!-- keywords: Funktionen, Datenbankfunktionen, DMITTELWERT, DANZAHL, DANZAHL2, DAUSZUG, DMAX, DMIN, DPRODUKT, DSTABW, DSTABWN, DSUMME, DVARIANZ, DVARIANZEN -->

Dies sind die Datenbankfunktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie bewirkt und was jedes ihrer Argumente ist. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie eine Funktion in eine Zelle eintippen, zeigt ein kleiner Tipp unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Tipp öffnet ihren Eintrag hier.

## DMITTELWERT

`DMITTELWERT(Datenbank; Feld; Kriterien)`

Bildet den Mittelwert der Zahlen in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DANZAHL

`DANZAHL(Datenbank; Feld; Kriterien)`

Zählt die Zahlen in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DANZAHL2

`DANZAHL2(Datenbank; Feld; Kriterien)`

Zählt die Zellen, die nicht leer sind, in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DAUSZUG

`DAUSZUG(Datenbank; Feld; Kriterien)`

Gibt den einzelnen Wert einer Spalte einer Liste für die einzige Zeile zurück, die die Kriterien erfüllt.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DMAX

`DMAX(Datenbank; Feld; Kriterien)`

Gibt die größte Zahl in einer Spalte einer Liste für die Zeilen zurück, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DMIN

`DMIN(Datenbank; Feld; Kriterien)`

Gibt die kleinste Zahl in einer Spalte einer Liste für die Zeilen zurück, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DPRODUKT

`DPRODUKT(Datenbank; Feld; Kriterien)`

Multipliziert die Zahlen in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DSTABW

`DSTABW(Datenbank; Feld; Kriterien)`

Schätzt die Streuung der Zahlen in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DSTABWN

`DSTABWN(Datenbank; Feld; Kriterien)`

Gibt die Streuung der Zahlen in einer Spalte einer Liste für die Zeilen zurück, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DSUMME

`DSUMME(Datenbank; Feld; Kriterien)`

Addiert die Zahlen in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DVARIANZ

`DVARIANZ(Datenbank; Feld; Kriterien)`

Schätzt die Varianz der Zahlen in einer Spalte einer Liste für die Zeilen, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".

## DVARIANZEN

`DVARIANZEN(Datenbank; Feld; Kriterien)`

Gibt die Varianz der Zahlen in einer Spalte einer Liste für die Zeilen zurück, die die Kriterien erfüllen.

- `Datenbank`: Der Zellbereich, der die Liste enthält, mit den Spaltennamen in der ersten Zeile.
- `Feld`: Die zu verwendende Spalte: ihr Name in Anführungszeichen oder ihre Nummer, wobei die erste Spalte als 1 zählt.
- `Kriterien`: Was gesucht wird: eine Zahl, ein Text oder ein Vergleich wie ">5".
