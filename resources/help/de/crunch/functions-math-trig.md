# Mathematische und trigonometrische Funktionen
<!-- keywords: Funktionen, ABS, ACOS, ACOSH, ACOT, ACOTH, AGGREGATE, ARABIC, ASIN, ASINH, ATAN, ATAN2, ATANH, BASE, CEILING, CEILING.MATH, CEILING.PRECISE, COMBIN, COMBINA, COS, COSH, COT, COTH, CSC, CSCH, DECIMAL, DEGREES, EVEN, EXP, FACT, FACTDOUBLE, FLOOR, FLOOR.MATH, FLOOR.PRECISE, GCD, INT, ISO.CEILING, LCM, LN, LOG, LOG10, MAX, MDETERM, MIN, MINVERSE, MMULT, MOD, MROUND, MULTINOMIAL, MUNIT, ODD, PERMUT, PI, POWER, PRODUCT, QUOTIENT, RADIANS, RAND, RANDARRAY, RANDBETWEEN, ROMAN, ROUND, ROUNDDOWN, ROUNDUP, SEC, SECH, SEQUENCE, SERIESSUM, SIGN, SIN, SINH, SQRT, SQRTPI, SUBTOTAL, SUM, SUMPRODUCT, SUMSQ, SUMX2MY2, SUMX2PY2, SUMXMY2, TAN, TANH, TRUNC, Formeln, Funktionen, Mathematik, Trigonometrie, Rechnen, Winkel, Runden, Summe, Potenz, Logarithmus, Zufallszahl -->

Diese Funktionen für Mathematik und Trigonometrie kann JeffCrunch berechnen. Bei jeder Funktion sehen Sie, wie sie geschrieben wird, was sie tut und was ihre Argumente bedeuten. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Zum Ausprobieren siehe [Funktion auswählen](insert-function). Während Sie eine Funktion in eine Zelle eingeben, zeigt ein kleiner Tipp unter der Zelle deren Argumente an. Ein Klick auf den Namen der Funktion im Tipp öffnet ihren Eintrag hier.

## ABS

`ABS(Zahl)`

Gibt eine Zahl ohne ihr Vorzeichen zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ACOS

`ACOS(Zahl)`

Gibt den Winkel im Bogenmaß zurück, dessen Kosinus die angegebene Zahl ist.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ACOSH

`ACOSH(Zahl)`

Gibt den Areakosinus Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ACOT

`ACOT(Zahl)`

Gibt den Arkuskotangens einer Zahl zurück, als Winkel im Bogenmaß.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ACOTH

`ACOTH(Zahl)`

Gibt den Areakotangens Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## AGGREGATE

`AGGREGATE(Funktion_Nr, Optionen, Bezug1, ...)`

Führt eine Berechnung für eine Liste aus, etwa eine Summe oder einen Median, und kann ausgeblendete Zeilen und Fehler ausschließen.

- `Funktion_Nr`: Welche Berechnung ausgeführt wird: 1 Mittelwert, 2 Anzahl, 3 Anzahl ohne Leerzellen, 4 Maximum, 5 Minimum, 6 Produkt, 7 und 8 Standardabweichung, 9 Summe, 10 und 11 Varianz.
- `Optionen`: Was ausgeschlossen wird: 0 verschachtelte Teilergebnisse, 1 zusätzlich ausgeblendete Zeilen, 2 zusätzlich Fehler, 3 beides, 4 bis 7 verschachtelte Teilergebnisse werden berücksichtigt.
- `Bezug1`: Die erste Zelle oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## ARABIC

`ARABIC(Text)`

Liest römische Ziffern in einem Text als Zahl ein.

- `Text`: Der Text, mit dem gerechnet wird, oder eine Zelle, die ihn enthält.

## ASIN

`ASIN(Zahl)`

Gibt den Winkel im Bogenmaß zurück, dessen Sinus die angegebene Zahl ist.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ASINH

`ASINH(Zahl)`

Gibt den Areasinus Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ATAN

`ATAN(Zahl)`

Gibt den Winkel im Bogenmaß zurück, dessen Tangens die angegebene Zahl ist.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ATAN2

`ATAN2(x_Zahl, y_Zahl)`

Gibt den Winkel im Bogenmaß des Punkts mit den Koordinaten x und y zurück.

- `x_Zahl`: Der x-Wert.
- `y_Zahl`: Der y-Wert.

## ATANH

`ATANH(Zahl)`

Gibt den Areatangens Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## BASE

`BASE(Zahl, Basis, [Mindestlänge])`

Schreibt eine Zahl als Text in einem anderen Zahlensystem mit der Basis 2 bis 36.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Basis`: Die Basis der Zahl: von 2 bis 36.
- `Mindestlänge` (optional): Die geringste Anzahl von Zeichen des Ergebnisses; kürzere Ergebnisse werden links mit Nullen aufgefüllt.

## CEILING

`CEILING(Zahl, [Genauigkeit])`

Rundet eine Zahl auf das nächste Vielfache auf.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## CEILING.MATH

`CEILING.MATH(Zahl, [Genauigkeit], [Modus])`

Rundet eine Zahl auf das nächste Vielfache auf, mit einer Wahl für negative Zahlen.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.
- `Modus` (optional): Wie negative Zahlen gerundet werden: 0 oder weggelassen ist die übliche Weise, jede andere Zahl kehrt sie um.

## CEILING.PRECISE

`CEILING.PRECISE(Zahl, [Genauigkeit])`

Rundet eine Zahl auf das nächste Vielfache auf, unabhängig vom Vorzeichen des Vielfachen.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## COMBIN

`COMBIN(Anzahl, Anzahl_gewählt)`

Zählt die Möglichkeiten, aus einer Gruppe Elemente auszuwählen, wenn die Reihenfolge keine Rolle spielt.

- `Anzahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_gewählt`: Wie viele Elemente in jeder Gruppe ausgewählt werden.

## COMBINA

`COMBINA(Anzahl, Anzahl_gewählt)`

Gibt die Anzahl der Möglichkeiten an, Elemente auszuwählen, wenn ein Element mehrfach gewählt werden darf.

- `Anzahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_gewählt`: Wie viele Elemente in jeder Gruppe ausgewählt werden.

## COS

`COS(Zahl)`

Gibt den Kosinus eines Winkels im Bogenmaß zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## COSH

`COSH(Zahl)`

Gibt den Kosinus Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## COT

`COT(Winkel)`

Gibt den Kotangens eines Winkels im Bogenmaß zurück.

- `Winkel`: Der Winkel im Bogenmaß.

## COTH

`COTH(Zahl)`

Gibt den Kotangens Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## CSC

`CSC(Winkel)`

Gibt den Kosekans eines Winkels im Bogenmaß zurück.

- `Winkel`: Der Winkel im Bogenmaß.

## CSCH

`CSCH(Zahl)`

Gibt den Kosekans Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## DECIMAL

`DECIMAL(Text, Basis)`

Liest einen Text, der in einem Zahlensystem mit der Basis 2 bis 36 geschrieben ist, als Zahl ein.

- `Text`: Der Text, mit dem gerechnet wird, oder eine Zelle, die ihn enthält.
- `Basis`: Die Basis der Zahl: von 2 bis 36.

## DEGREES

`DEGREES(Winkel)`

Wandelt Bogenmaß in Grad um.

- `Winkel`: Der Winkel im Bogenmaß.

## EVEN

`EVEN(Zahl)`

Rundet eine Zahl auf die nächste gerade ganze Zahl auf.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## EXP

`EXP(Zahl)`

Gibt e hoch eine Potenz zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## FACT

`FACT(Zahl)`

Gibt die Fakultät einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## FACTDOUBLE

`FACTDOUBLE(Zahl)`

Gibt die Doppelfakultät einer Zahl zurück: das Produkt aller zweiten Zahlen bis hinunter zu 1 oder 2.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## FLOOR

`FLOOR(Zahl, [Genauigkeit])`

Rundet eine Zahl auf das nächste Vielfache ab.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## FLOOR.MATH

`FLOOR.MATH(Zahl, [Genauigkeit], [Modus])`

Rundet eine Zahl auf das nächste Vielfache ab, mit einer Wahl für negative Zahlen.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.
- `Modus` (optional): Wie negative Zahlen gerundet werden: 0 oder weggelassen ist die übliche Weise, jede andere Zahl kehrt sie um.

## FLOOR.PRECISE

`FLOOR.PRECISE(Zahl, [Genauigkeit])`

Rundet eine Zahl auf das nächste Vielfache ab, unabhängig vom Vorzeichen des Vielfachen.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## GCD

`GCD(Zahl1, ...)`

Gibt die größte ganze Zahl zurück, durch die alle Zahlen teilbar sind.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## INT

`INT(Zahl)`

Rundet eine Zahl auf die nächstkleinere ganze Zahl ab.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## ISO.CEILING

`ISO.CEILING(Zahl, [Genauigkeit])`

Rundet eine Zahl auf das nächste Vielfache auf, unabhängig vom Vorzeichen des Vielfachen.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Genauigkeit` (optional): Die Zahl, auf deren Vielfache gerundet wird.

## LCM

`LCM(Zahl1, ...)`

Gibt die kleinste ganze Zahl zurück, die alle Zahlen als Teiler hat.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## LN

`LN(Zahl)`

Gibt den natürlichen Logarithmus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## LOG

`LOG(Zahl, [Basis])`

Gibt den Logarithmus einer Zahl zur angegebenen Basis zurück (ohne Angabe zur Basis 10).

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Basis` (optional): Die Basis des Logarithmus (10, wenn weggelassen).

## LOG10

`LOG10(Zahl)`

Gibt den Logarithmus einer Zahl zur Basis 10 zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## MAX

`MAX(Zahl1, ...)`

Gibt die größte Zahl zurück.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## MDETERM

`MDETERM(Matrix)`

Gibt die Determinante eines quadratischen Zahlenbereichs zurück.

- `Matrix`: Ein Zellbereich oder eine Werteliste in geschweiften Klammern.

## MIN

`MIN(Zahl1, ...)`

Gibt die kleinste Zahl zurück.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## MINVERSE

`MINVERSE(Matrix)`

Gibt die Inverse eines quadratischen Zahlenbereichs zurück.

- `Matrix`: Ein Zellbereich oder eine Werteliste in geschweiften Klammern.

## MMULT

`MMULT(Matrix1, Matrix2)`

Gibt das Produkt zweier Zahlenbereiche als Matrizen zurück.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix2`: Der zweite Zellbereich oder die zweite Werteliste, gleich groß wie die erste.

## MOD

`MOD(Zahl, Teiler)`

Gibt den Rest zurück, der bleibt, wenn eine Zahl durch eine andere geteilt wird.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Teiler`: Die Zahl, durch die geteilt wird.

## MROUND

`MROUND(Zahl, Vielfaches)`

Rundet eine Zahl auf das nächste Vielfache.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Vielfaches`: Die Zahl, auf deren Vielfache gerundet wird.

## MULTINOMIAL

`MULTINOMIAL(Zahl1, ...)`

Gibt die Fakultät der Summe der Zahlen geteilt durch das Produkt ihrer Fakultäten zurück.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## MUNIT

`MUNIT(Dimension)`

Gibt die Einheitsmatrix einer Größe zurück.

- `Dimension`: Die Größe des quadratischen Bereichs: wie viele Zeilen und Spalten er hat.

## ODD

`ODD(Zahl)`

Rundet eine Zahl auf die nächste ungerade ganze Zahl auf.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## PERMUT

`PERMUT(Anzahl, Anzahl_gewählt)`

Zählt die Möglichkeiten, aus einer Gruppe Elemente auszuwählen, wenn die Reihenfolge eine Rolle spielt.

- `Anzahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_gewählt`: Wie viele Elemente in jeder Gruppe ausgewählt werden.

## PI

`PI()`

Gibt die Zahl Pi zurück.

## POWER

`POWER(Zahl, Potenz)`

Potenziert eine Zahl.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Potenz`: Der Exponent, mit dem die Zahl potenziert wird.

## PRODUCT

`PRODUCT(Zahl1, ...)`

Multipliziert Zahlen miteinander.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## QUOTIENT

`QUOTIENT(Zähler, Nenner)`

Gibt den ganzzahligen Anteil einer Division zurück.

- `Zähler`: Die zu teilende Zahl.
- `Nenner`: Die Zahl, durch die geteilt wird.

## RADIANS

`RADIANS(Winkel)`

Wandelt Grad in Bogenmaß um.

- `Winkel`: Der Winkel in Grad.

## RAND

`RAND()`

Gibt eine Zufallszahl von 0 bis unter 1 zurück.

## RANDARRAY

`RANDARRAY([Zeilen], [Spalten], [Min], [Max], [ganze_Zahl])`

Gibt einen Zellbereich mit Zufallszahlen zurück.

- `Zeilen` (optional): Wie viele Zeilen der Bereich hat: positiv nach unten, negativ nach oben.
- `Spalten` (optional): Wie viele Spalten breit das Ergebnis ist (1, wenn weggelassen).
- `Min` (optional): Die kleinste Zahl, die das Ergebnis haben darf (0, wenn weggelassen).
- `Max` (optional): Die größte Zahl, die das Ergebnis haben darf (1, wenn weggelassen).
- `ganze_Zahl` (optional): WAHR für ganze Zahlen; FALSCH oder weggelassen für Zahlen mit Dezimalstellen.

## RANDBETWEEN

`RANDBETWEEN(Untere_Grenze, Obere_Grenze)`

Gibt eine zufällige ganze Zahl zwischen zwei Zahlen zurück.

- `Untere_Grenze`: Der kleinste Wert, den das Ergebnis haben darf.
- `Obere_Grenze`: Der größte Wert, den das Ergebnis haben darf.

## ROMAN

`ROMAN(Zahl, [Form])`

Schreibt eine ganze Zahl von 0 bis 3999 als römische Ziffern in einem Text.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Form` (optional): Wie kurz die römischen Ziffern sind: 0 oder weggelassen für die klassische Form, bis 4 für die kürzeste.

## ROUND

`ROUND(Zahl, Anzahl_Stellen)`

Rundet eine Zahl auf eine bestimmte Anzahl von Stellen.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_Stellen`: Auf wie viele Stellen gerundet wird: positiv für die Stellen rechts vom Dezimaltrennzeichen, 0 für eine ganze Zahl, negativ für die Stellen links davon.

## ROUNDDOWN

`ROUNDDOWN(Zahl, Anzahl_Stellen)`

Rundet eine Zahl auf eine bestimmte Anzahl von Stellen ab, in Richtung null.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_Stellen`: Auf wie viele Stellen gerundet wird: positiv für die Stellen rechts vom Dezimaltrennzeichen, 0 für eine ganze Zahl, negativ für die Stellen links davon.

## ROUNDUP

`ROUNDUP(Zahl, Anzahl_Stellen)`

Rundet eine Zahl auf eine bestimmte Anzahl von Stellen auf, von null weg.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_Stellen`: Auf wie viele Stellen gerundet wird: positiv für die Stellen rechts vom Dezimaltrennzeichen, 0 für eine ganze Zahl, negativ für die Stellen links davon.

## SEC

`SEC(Winkel)`

Gibt den Sekans eines Winkels im Bogenmaß zurück.

- `Winkel`: Der Winkel im Bogenmaß.

## SECH

`SECH(Zahl)`

Gibt den Sekans Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## SEQUENCE

`SEQUENCE(Zeilen, [Spalten], [Start], [Schritt])`

Gibt einen Zellbereich mit Zahlen zurück, die ab einem Startwert um einen Schritt steigen.

- `Zeilen`: Wie viele Zeilen der Bereich hat: positiv nach unten, negativ nach oben.
- `Spalten` (optional): Wie viele Spalten breit das Ergebnis ist (1, wenn weggelassen).
- `Start` (optional): Die erste Zahl (1, wenn weggelassen).
- `Schritt` (optional): Wie viel für jede Zahl addiert wird (1, wenn weggelassen; bei GESTEP die Zahl, mit der verglichen wird, 0, wenn weggelassen).

## SERIESSUM

`SERIESSUM(x, n, m, Koeffizienten)`

Gibt die Summe einer Potenzreihe zurück.

- `x`: Der zu verwendende x-Wert.
- `n`: Die erste Potenz von x bzw. die Ordnung der Funktion, eine ganze Zahl von 0 oder mehr.
- `m`: Der Schritt, der für jeden Koeffizienten zur Potenz addiert wird.
- `Koeffizienten`: Die Zahlen, mit denen jede Potenz von x multipliziert wird.

## SIGN

`SIGN(Zahl)`

Gibt 1 für eine positive Zahl, 0 für null und -1 für eine negative Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## SIN

`SIN(Zahl)`

Gibt den Sinus eines Winkels im Bogenmaß zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## SINH

`SINH(Zahl)`

Gibt den Sinus Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## SQRT

`SQRT(Zahl)`

Gibt die Quadratwurzel einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## SQRTPI

`SQRTPI(Zahl)`

Gibt die Quadratwurzel aus einer Zahl mal Pi zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## SUBTOTAL

`SUBTOTAL(Funktion_Nr, Bezug1, ...)`

Gibt eine Summe, einen Mittelwert, eine Anzahl oder eine andere Kennzahl für einen Zellbereich zurück.

- `Funktion_Nr`: Welche Berechnung ausgeführt wird: 1 Mittelwert, 2 Anzahl, 3 Anzahl ohne Leerzellen, 4 Maximum, 5 Minimum, 6 Produkt, 7 und 8 Standardabweichung, 9 Summe, 10 und 11 Varianz.
- `Bezug1`: Die erste Zelle oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## SUM

`SUM(Zahl1, ...)`

Addiert Zahlen und die Zahlen in Zellbereichen.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## SUMPRODUCT

`SUMPRODUCT(Matrix1, ...)`

Multipliziert die Zahlen gleich großer Zellbereiche Stelle für Stelle und addiert die Produkte.

- `Matrix1`: Der erste Zellbereich oder die erste Werteliste.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## SUMSQ

`SUMSQ(Zahl1, ...)`

Addiert die Quadrate von Zahlen.

- `Zahl1`: Die erste Zahl oder der erste Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## SUMX2MY2

`SUMX2MY2(Matrix_x, Matrix_y)`

Addiert die Differenzen der Quadrate zweier Zahlenreihen.

- `Matrix_x`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix_y`: Der zweite Zellbereich oder die zweite Werteliste, gleich groß wie die erste.

## SUMX2PY2

`SUMX2PY2(Matrix_x, Matrix_y)`

Addiert die Summen der Quadrate zweier Zahlenreihen.

- `Matrix_x`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix_y`: Der zweite Zellbereich oder die zweite Werteliste, gleich groß wie die erste.

## SUMXMY2

`SUMXMY2(Matrix_x, Matrix_y)`

Addiert die Quadrate der Differenzen zweier Zahlenreihen.

- `Matrix_x`: Der erste Zellbereich oder die erste Werteliste.
- `Matrix_y`: Der zweite Zellbereich oder die zweite Werteliste, gleich groß wie die erste.

## TAN

`TAN(Zahl)`

Gibt den Tangens eines Winkels im Bogenmaß zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## TANH

`TANH(Zahl)`

Gibt den Tangens Hyperbolicus einer Zahl zurück.

- `Zahl`: Die Zahl, mit der gerechnet wird.

## TRUNC

`TRUNC(Zahl, [Anzahl_Stellen])`

Schneidet eine Zahl nach einer bestimmten Anzahl von Stellen ab, ohne zu runden.

- `Zahl`: Die Zahl, mit der gerechnet wird.
- `Anzahl_Stellen` (optional): Auf wie viele Stellen gekürzt wird: positiv für die Stellen rechts vom Dezimaltrennzeichen, 0 für eine ganze Zahl, negativ für die Stellen links davon.
