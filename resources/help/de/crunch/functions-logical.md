# Logische Funktionen
<!-- keywords: Funktionen, UND, WAHL, FALSCH, WENN, WENNFEHLER, WENNNV, WENNS, LET, NICHT, ODER, ERGEBNIS, WAHR, XODER, Funktionsliste, Formeln -->

Dies sind die logischen Funktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie in einer Zelle eine Funktion eingeben, zeigt ein kleiner Tipp unter der Zelle ihre Argumente an. Wenn Sie im Tipp auf den Namen der Funktion klicken, öffnet sich ihr Eintrag hier.

## UND

`UND(logisch1; ...)`

Ist wahr, wenn alle Bedingungen wahr sind.

- `logisch1`: Der erste Wert oder Test, der wahr oder falsch ist.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## WAHL

`WAHL(index; wert1; ...)`

Wählt aus einer Liste den Wert an der Position, die der Index angibt.

- `index`: Welcher Wert ausgegeben wird, wobei der erste als 1 zählt.
- `wert1`: Der erste Wert oder Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## FALSCH

`FALSCH()`

Der logische Wert Falsch.

## WENN

`WENN(bedingung; wert_wenn_wahr; [wert_wenn_falsch])`

Gibt einen Wert aus, wenn eine Bedingung zutrifft, und einen anderen, wenn sie nicht zutrifft.

- `bedingung`: Ein Wert oder Test, der wahr oder falsch ist.
- `wert_wenn_wahr`: Was ausgegeben wird, wenn die Bedingung wahr ist.
- `wert_wenn_falsch` (optional): Was ausgegeben wird, wenn die Bedingung falsch ist (FALSCH, wenn das Argument fehlt).

## WENNFEHLER

`WENNFEHLER(wert; wert_bei_fehler)`

Gibt einen Wert aus oder einen Ersatz, wenn dieser Wert ein Fehler ist.

- `wert`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `wert_bei_fehler`: Was ausgegeben wird, wenn der Wert ein Fehler ist.

## WENNNV

`WENNNV(wert; wert_bei_nv)`

Gibt einen Wert aus oder einen Ersatz, wenn dieser Wert der Fehler „Nicht verfügbar“ ist.

- `wert`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `wert_bei_nv`: Was ausgegeben wird, wenn der Wert der Fehler „Nicht verfügbar“ ist.

## WENNS

`WENNS(bedingung1; wert1; ...)`

Prüft Bedingungen nacheinander und gibt den Wert aus, der zur ersten zutreffenden Bedingung gehört.

- `bedingung1`: Der erste Test, der wahr oder falsch ist.
- `wert1`: Der erste Wert oder Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## LET

`LET(name1; wert1; ...)`

Gibt Werten innerhalb einer Formel Namen und berechnet dann eine Rechnung, die diese Namen verwendet.

- `name1`: Ein Name für den folgenden Wert. Er kann in den nachfolgenden Werten und in der Berechnung verwendet werden.
- `wert1`: Der erste Wert oder Zellbereich.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## NICHT

`NICHT(logisch)`

Macht aus Wahr Falsch und aus Falsch Wahr.

- `logisch`: Ein Wert oder Test, der wahr oder falsch ist.

## ODER

`ODER(logisch1; ...)`

Ist wahr, wenn mindestens eine der Bedingungen wahr ist.

- `logisch1`: Der erste Wert oder Test, der wahr oder falsch ist.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## ERGEBNIS

`ERGEBNIS(ausdruck; wert1; ergebnis1; ...)`

Vergleicht einen Wert mit einer Liste von Werten und gibt das Ergebnis aus, das zur ersten Übereinstimmung gehört.

- `ausdruck`: Der Wert, der mit den folgenden Werten verglichen wird.
- `wert1`: Der erste Wert oder Zellbereich.
- `ergebnis1`: Das Ergebnis, das ausgegeben wird, wenn der davor stehende Wert übereinstimmt.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## WAHR

`WAHR()`

Der logische Wert Wahr.

## XODER

`XODER(logisch1; ...)`

Ist wahr, wenn eine ungerade Anzahl der Bedingungen wahr ist.

- `logisch1`: Der erste Wert oder Test, der wahr oder falsch ist.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.
