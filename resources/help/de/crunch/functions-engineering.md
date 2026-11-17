# Technische Funktionen
<!-- keywords: functions, BESSELI, BESSELJ, BESSELK, BESSELY, BIN2DEC, BIN2HEX, BIN2OCT, BITAND, BITLSHIFT, BITOR, BITRSHIFT, BITXOR, COMPLEX, CONVERT, DEC2BIN, DEC2HEX, DEC2OCT, DELTA, ERF, ERF.PRECISE, ERFC, ERFC.PRECISE, GESTEP, HEX2BIN, HEX2DEC, HEX2OCT, IMABS, IMAGINARY, IMARGUMENT, IMCONJUGATE, IMCOS, IMCOSH, IMCOT, IMCSC, IMCSCH, IMDIV, IMEXP, IMLN, IMLOG10, IMLOG2, IMPOWER, IMPRODUCT, IMREAL, IMSEC, IMSECH, IMSIN, IMSINH, IMSQRT, IMSUB, IMSUM, IMTAN, OCT2BIN, OCT2DEC, OCT2HEX -->

Dies sind die technischen Funktionen, die JeffCrunch berechnen kann. Jede Funktion zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie in einer Zelle eine Funktion eingeben, zeigt ein kleiner Hinweis unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Hinweis öffnet ihren Eintrag hier.

## BESSELI

`BESSELI(x; n)`

Gibt die modifizierte Besselfunktion erster Art zurück.

- `x`: Der zu verwendende x-Wert.
- `n`: Die erste Potenz von x bzw. die Ordnung der Funktion, eine ganze Zahl größer oder gleich 0.

## BESSELJ

`BESSELJ(x; n)`

Gibt die Besselfunktion erster Art zurück.

- `x`: Der zu verwendende x-Wert.
- `n`: Die erste Potenz von x bzw. die Ordnung der Funktion, eine ganze Zahl größer oder gleich 0.

## BESSELK

`BESSELK(x; n)`

Gibt die modifizierte Besselfunktion zweiter Art zurück.

- `x`: Der zu verwendende x-Wert.
- `n`: Die erste Potenz von x bzw. die Ordnung der Funktion, eine ganze Zahl größer oder gleich 0.

## BESSELY

`BESSELY(x; n)`

Gibt die Besselfunktion zweiter Art zurück.

- `x`: Der zu verwendende x-Wert.
- `n`: Die erste Potenz von x bzw. die Ordnung der Funktion, eine ganze Zahl größer oder gleich 0.

## BIN2DEC

`BIN2DEC(Zahl)`

Wandelt eine Binärzahl in eine Dezimalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.

## BIN2HEX

`BIN2HEX(Zahl; [Stellen])`

Wandelt eine Binärzahl in eine Hexadezimalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## BIN2OCT

`BIN2OCT(Zahl; [Stellen])`

Wandelt eine Binärzahl in eine Oktalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## BITAND

`BITAND(Zahl1; Zahl2)`

Gibt die Bits zurück, die in beiden von zwei ganzen Zahlen gesetzt sind.

- `Zahl1`: Die erste Zahl bzw. der Zellbereich.
- `Zahl2`: Die zweite Zahl.

## BITLSHIFT

`BITLSHIFT(Zahl; Verschiebung)`

Verschiebt die Bits einer ganzen Zahl nach links.

- `Zahl`: Die zu verarbeitende Zahl.
- `Verschiebung`: Um wie viele Bits verschoben wird: Positive und negative Zahlen verschieben in entgegengesetzte Richtungen.

## BITOR

`BITOR(Zahl1; Zahl2)`

Gibt die Bits zurück, die in einer der beiden ganzen Zahlen gesetzt sind.

- `Zahl1`: Die erste Zahl bzw. der Zellbereich.
- `Zahl2`: Die zweite Zahl.

## BITRSHIFT

`BITRSHIFT(Zahl; Verschiebung)`

Verschiebt die Bits einer ganzen Zahl nach rechts.

- `Zahl`: Die zu verarbeitende Zahl.
- `Verschiebung`: Um wie viele Bits verschoben wird: Positive und negative Zahlen verschieben in entgegengesetzte Richtungen.

## BITXOR

`BITXOR(Zahl1; Zahl2)`

Gibt die Bits zurück, die in genau einer der beiden ganzen Zahlen gesetzt sind, aber nicht in beiden.

- `Zahl1`: Die erste Zahl bzw. der Zellbereich.
- `Zahl2`: Die zweite Zahl.

## COMPLEX

`COMPLEX(Realteil; Imaginärteil; [Suffix])`

Erstellt eine komplexe Zahl aus ihrem Real- und Imaginärteil.

- `Realteil`: Der Realteil der komplexen Zahl.
- `Imaginärteil`: Der Imaginärteil der komplexen Zahl.
- `Suffix` (optional): i oder j, um den Imaginärteil zu kennzeichnen (i, wenn weggelassen).

## CONVERT

`CONVERT(Zahl; Ausgangseinheit; Zieleinheit)`

Rechnet eine Zahl von einer Maßeinheit in eine andere um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Ausgangseinheit`: Die Einheit, in der die Zahl angegeben ist, als Text, z. B. "mi" oder "lbm".
- `Zieleinheit`: Die Einheit, in die umgerechnet werden soll, als Text, z. B. "km" oder "kg".

## DEC2BIN

`DEC2BIN(Zahl; [Stellen])`

Wandelt eine Dezimalzahl in eine Binärzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## DEC2HEX

`DEC2HEX(Zahl; [Stellen])`

Wandelt eine Dezimalzahl in eine Hexadezimalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## DEC2OCT

`DEC2OCT(Zahl; [Stellen])`

Wandelt eine Dezimalzahl in eine Oktalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## DELTA

`DELTA(Zahl1; [Zahl2])`

Prüft, ob zwei Zahlen gleich sind, mit 1 für Ja und 0 für Nein.

- `Zahl1`: Die erste Zahl bzw. der Zellbereich.
- `Zahl2` (optional): Die zweite Zahl.

## ERF

`ERF(untere_Grenze; [obere_Grenze])`

Gibt die Fehlerfunktion zwischen zwei Grenzen oder von 0 an zurück.

- `untere_Grenze`: Die untere Grenze des Bereichs.
- `obere_Grenze` (optional): Die obere Grenze des Bereichs (wenn weggelassen, wird die untere Grenze allein verwendet).

## ERF.PRECISE

`ERF.PRECISE(x)`

Gibt die Fehlerfunktion von 0 bis zu einer Grenze zurück.

- `x`: Der zu verwendende x-Wert.

## ERFC

`ERFC(x)`

Gibt die komplementäre Fehlerfunktion von einer Grenze bis unendlich zurück.

- `x`: Der zu verwendende x-Wert.

## ERFC.PRECISE

`ERFC.PRECISE(x)`

Gibt die komplementäre Fehlerfunktion von einer Grenze bis unendlich zurück.

- `x`: Der zu verwendende x-Wert.

## GESTEP

`GESTEP(Zahl; [Schritt])`

Prüft, ob eine Zahl mindestens so groß wie ein Schritt ist, mit 1 für Ja und 0 für Nein.

- `Zahl`: Die zu verarbeitende Zahl.
- `Schritt` (optional): Der Wert, mit dem verglichen wird (0, wenn weggelassen).

## HEX2BIN

`HEX2BIN(Zahl; [Stellen])`

Wandelt eine Hexadezimalzahl in eine Binärzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## HEX2DEC

`HEX2DEC(Zahl)`

Wandelt eine Hexadezimalzahl in eine Dezimalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.

## HEX2OCT

`HEX2OCT(Zahl; [Stellen])`

Wandelt eine Hexadezimalzahl in eine Oktalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## IMABS

`IMABS(komplexe_Zahl)`

Gibt den Betrag einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMAGINARY

`IMAGINARY(komplexe_Zahl)`

Gibt den Imaginärteil einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMARGUMENT

`IMARGUMENT(komplexe_Zahl)`

Gibt den Winkel einer komplexen Zahl im Bogenmaß zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMCONJUGATE

`IMCONJUGATE(komplexe_Zahl)`

Gibt die konjugiert komplexe Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMCOS

`IMCOS(komplexe_Zahl)`

Gibt den Kosinus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMCOSH

`IMCOSH(komplexe_Zahl)`

Gibt den Kosinus hyperbolicus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMCOT

`IMCOT(komplexe_Zahl)`

Gibt den Kotangens einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMCSC

`IMCSC(komplexe_Zahl)`

Gibt den Kosekans einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMCSCH

`IMCSCH(komplexe_Zahl)`

Gibt den Kosekans hyperbolicus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMDIV

`IMDIV(komplexe_Zahl1; komplexe_Zahl2)`

Gibt den Quotienten zweier komplexer Zahlen zurück.

- `komplexe_Zahl1`: Die erste komplexe Zahl als Text, z. B. "3+4i".
- `komplexe_Zahl2`: Die zweite komplexe Zahl als Text.

## IMEXP

`IMEXP(komplexe_Zahl)`

Gibt e hoch die komplexe Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMLN

`IMLN(komplexe_Zahl)`

Gibt den natürlichen Logarithmus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMLOG10

`IMLOG10(komplexe_Zahl)`

Gibt den dekadischen Logarithmus (Basis 10) einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMLOG2

`IMLOG2(komplexe_Zahl)`

Gibt den binären Logarithmus (Basis 2) einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMPOWER

`IMPOWER(komplexe_Zahl; Zahl)`

Gibt eine komplexe Zahl potenziert mit einem Exponenten zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".
- `Zahl`: Die zu verarbeitende Zahl.

## IMPRODUCT

`IMPRODUCT(komplexe_Zahl1; ...)`

Gibt das Produkt komplexer Zahlen zurück.

- `komplexe_Zahl1`: Die erste komplexe Zahl als Text, z. B. "3+4i".
- `...`: So viele weitere, wie Sie benötigen, von derselben Art wie die vorherige.

## IMREAL

`IMREAL(komplexe_Zahl)`

Gibt den Realteil einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMSEC

`IMSEC(komplexe_Zahl)`

Gibt den Sekans einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMSECH

`IMSECH(komplexe_Zahl)`

Gibt den Sekans hyperbolicus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMSIN

`IMSIN(komplexe_Zahl)`

Gibt den Sinus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMSINH

`IMSINH(komplexe_Zahl)`

Gibt den Sinus hyperbolicus einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMSQRT

`IMSQRT(komplexe_Zahl)`

Gibt die Quadratwurzel einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## IMSUB

`IMSUB(komplexe_Zahl1; komplexe_Zahl2)`

Gibt die Differenz zweier komplexer Zahlen zurück.

- `komplexe_Zahl1`: Die erste komplexe Zahl als Text, z. B. "3+4i".
- `komplexe_Zahl2`: Die zweite komplexe Zahl als Text.

## IMSUM

`IMSUM(komplexe_Zahl1; ...)`

Gibt die Summe komplexer Zahlen zurück.

- `komplexe_Zahl1`: Die erste komplexe Zahl als Text, z. B. "3+4i".
- `...`: So viele weitere, wie Sie benötigen, von derselben Art wie die vorherige.

## IMTAN

`IMTAN(komplexe_Zahl)`

Gibt den Tangens einer komplexen Zahl zurück.

- `komplexe_Zahl`: Eine komplexe Zahl als Text, z. B. "3+4i".

## OCT2BIN

`OCT2BIN(Zahl; [Stellen])`

Wandelt eine Oktalzahl in eine Binärzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.

## OCT2DEC

`OCT2DEC(Zahl)`

Wandelt eine Oktalzahl in eine Dezimalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.

## OCT2HEX

`OCT2HEX(Zahl; [Stellen])`

Wandelt eine Oktalzahl in eine Hexadezimalzahl um.

- `Zahl`: Die zu verarbeitende Zahl.
- `Stellen` (optional): Auf wie viele Zeichen das Ergebnis links mit Nullen aufgefüllt wird.
