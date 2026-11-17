# Textfunktionen
<!-- keywords: Funktionen, ASC, BAHTTEXT, CHAR, CLEAN, CODE, CONCAT, CONCATENATE, DBCS, DOLLAR, ENCODEURL, EXACT, FILTERXML, FIND, FINDB, FIXED, LEFT, LEFTB, LEN, LENB, LOWER, MID, MIDB, NUMBERVALUE, PHONETIC, PROPER, REPLACE, REPLACEB, REPT, RIGHT, RIGHTB, SEARCH, SEARCHB, SUBSTITUTE, T, TEXT, TEXTJOIN, TRIM, UNICHAR, UNICODE, UPPER, VALUE -->

Dies sind die Textfunktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Zum Ausprobieren siehe [Funktion auswählen](insert-function). Während Sie in einer Zelle eine Funktion eingeben, zeigt ein kleiner Hinweis unter der Zelle deren Argumente an. Ein Klick auf den Namen der Funktion im Hinweis öffnet ihren Eintrag hier.

## ASC

`ASC(text)`

Wandelt vollbreite Buchstaben und Ziffern im Text in halbbreite um.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## BAHTTEXT

`BAHTTEXT(zahl)`

Schreibt eine Zahl als thailändische Wörter mit der Bezeichnung für Baht.

- `zahl`: Die Zahl, mit der gearbeitet wird.

## CHAR

`CHAR(zahl)`

Gibt das Zeichen zurück, das zu einer Codenummer gehört.

- `zahl`: Die Zahl, mit der gearbeitet wird.

## CLEAN

`CLEAN(text)`

Entfernt aus einem Text Zeichen, die nicht gedruckt werden.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## CODE

`CODE(text)`

Gibt die Codenummer des ersten Zeichens eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## CONCAT

`CONCAT(text1, ...)`

Verbindet Texte und die Texte in Zellbereichen zu einem.

- `text1`: Der erste Text oder Zellbereich mit Text.
- `...`: Beliebig viele weitere derselben Art wie das Argument davor.

## CONCATENATE

`CONCATENATE(text1, ...)`

Verbindet Texte zu einem.

- `text1`: Der erste Text oder Zellbereich mit Text.
- `...`: Beliebig viele weitere derselben Art wie das Argument davor.

## DBCS

`DBCS(text)`

Wandelt halbbreite Buchstaben und Ziffern im Text in vollbreite um.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## DOLLAR

`DOLLAR(zahl, [dezimalstellen])`

Wandelt eine Zahl in Text als Geldbetrag um.

- `zahl`: Die Zahl, mit der gearbeitet wird.
- `dezimalstellen` (optional): Wie viele Stellen nach dem Dezimaltrennzeichen beibehalten werden.

## ENCODEURL

`ENCODEURL(text)`

Ändert Text so, dass er in einer Webadresse verwendet werden kann.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## EXACT

`EXACT(text1, text2)`

WAHR, wenn zwei Texte genau gleich sind, einschließlich Groß- und Kleinschreibung.

- `text1`: Der erste Text oder Zellbereich mit Text.
- `text2`: Der zweite Text.

## FILTERXML

`FILTERXML(xml, xpath)`

Holt Werte aus einem Text, der XML enthält, mithilfe eines XPath-Ausdrucks heraus.

- `xml`: Der Text, der das XML enthält.
- `xpath`: Der XPath, der angibt, welche Teile ausgegeben werden, z. B. //artikel/preis.

## FIND

`FIND(suchtext, text_innerhalb, [erste_stelle])`

Gibt an, an welcher Stelle ein Text innerhalb eines anderen beginnt, und unterscheidet dabei Groß- und Kleinbuchstaben.

- `suchtext`: Der Text, nach dem gesucht wird.
- `text_innerhalb`: Der Text, in dem gesucht wird.
- `erste_stelle` (optional): Die Stelle im Text, an der die Suche beginnt. Das erste Zeichen zählt als 1.

## FINDB

`FINDB(suchtext, text_innerhalb, [erste_stelle])`

Gibt an, an welcher Stelle ein Text zuerst in einem anderen Text vorkommt, und unterscheidet dabei Groß- und Kleinbuchstaben.

- `suchtext`: Der Text, nach dem gesucht wird.
- `text_innerhalb`: Der Text, in dem gesucht wird.
- `erste_stelle` (optional): Die Stelle im Text, an der die Suche beginnt. Das erste Zeichen zählt als 1.

## FIXED

`FIXED(zahl, [dezimalstellen], [ohne_kommas])`

Wandelt eine Zahl in Text mit einer festgelegten Anzahl Dezimalstellen um.

- `zahl`: Die Zahl, mit der gearbeitet wird.
- `dezimalstellen` (optional): Wie viele Stellen nach dem Dezimaltrennzeichen beibehalten werden.
- `ohne_kommas` (optional): WAHR, um die Trennzeichen zu entfernen, die Tausender gliedern.

## LEFT

`LEFT(text, [anzahl_zeichen])`

Gibt die ersten Zeichen eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `anzahl_zeichen` (optional): Wie viele Zeichen übernommen werden (1, wenn weggelassen).

## LEFTB

`LEFTB(text, [anzahl_zeichen])`

Gibt die ersten Zeichen eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `anzahl_zeichen` (optional): Wie viele Zeichen übernommen werden (1, wenn weggelassen).

## LEN

`LEN(text)`

Zählt die Zeichen in einem Text.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## LENB

`LENB(text)`

Gibt die Anzahl der Zeichen im Text zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## LOWER

`LOWER(text)`

Wandelt einen Text in Kleinbuchstaben um.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## MID

`MID(text, erste_stelle, anzahl_zeichen)`

Gibt Zeichen aus der Mitte eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `erste_stelle`: Die Stelle im Text, ab der die Zeichen übernommen werden. Das erste Zeichen zählt als 1.
- `anzahl_zeichen`: Wie viele Zeichen übernommen werden (1, wenn weggelassen).

## MIDB

`MIDB(text, erste_stelle, anzahl_zeichen)`

Gibt Zeichen aus der Mitte eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `erste_stelle`: Die Stelle im Text, ab der die Zeichen übernommen werden. Das erste Zeichen zählt als 1.
- `anzahl_zeichen`: Wie viele Zeichen übernommen werden (1, wenn weggelassen).

## NUMBERVALUE

`NUMBERVALUE(text, [dezimaltrennzeichen], [gruppierungszeichen])`

Liest Text als Zahl, mit den angegebenen Trennzeichen.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `dezimaltrennzeichen` (optional): Das Zeichen, das im Text die Dezimalstellen abtrennt (der Punkt, wenn weggelassen).
- `gruppierungszeichen` (optional): Das Zeichen, das im Text die Ziffern gruppiert (das Komma, wenn weggelassen).

## PHONETIC

`PHONETIC(bezug)`

Gibt den Text einer Zelle zurück, für die Lesungen japanischer Texte.

- `bezug`: Eine Zelle oder ein Zellbereich.

## PROPER

`PROPER(text)`

Schreibt jedes Wort eines Textes mit großem Anfangsbuchstaben.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## REPLACE

`REPLACE(alter_text, erste_stelle, anzahl_zeichen, neuer_text)`

Setzt neuen Text an die Stelle einiger Zeichen eines Textes.

- `alter_text`: Der Text, der geändert wird.
- `erste_stelle`: Die Stelle im Text, ab der ersetzt wird. Das erste Zeichen zählt als 1.
- `anzahl_zeichen`: Wie viele Zeichen ersetzt werden (1, wenn weggelassen).
- `neuer_text`: Der Text, der eingefügt wird.

## REPLACEB

`REPLACEB(alter_text, erste_stelle, anzahl_zeichen, neuer_text)`

Ersetzt einen Teil des Textes durch anderen Text.

- `alter_text`: Der Text, der geändert wird.
- `erste_stelle`: Die Stelle im Text, ab der ersetzt wird. Das erste Zeichen zählt als 1.
- `anzahl_zeichen`: Wie viele Zeichen ersetzt werden (1, wenn weggelassen).
- `neuer_text`: Der Text, der eingefügt wird.

## REPT

`REPT(text, anzahl_wiederholungen)`

Wiederholt einen Text mehrmals.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `anzahl_wiederholungen`: Wie oft der Text wiederholt wird.

## RIGHT

`RIGHT(text, [anzahl_zeichen])`

Gibt die letzten Zeichen eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `anzahl_zeichen` (optional): Wie viele Zeichen übernommen werden (1, wenn weggelassen).

## RIGHTB

`RIGHTB(text, [anzahl_zeichen])`

Gibt die letzten Zeichen eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `anzahl_zeichen` (optional): Wie viele Zeichen übernommen werden (1, wenn weggelassen).

## SEARCH

`SEARCH(suchtext, text_innerhalb, [erste_stelle])`

Gibt an, an welcher Stelle ein Text innerhalb eines anderen beginnt. Groß- und Kleinschreibung wird ignoriert, und im Suchtext sind ? und * als Platzhalter erlaubt.

- `suchtext`: Der Text, nach dem gesucht wird.
- `text_innerhalb`: Der Text, in dem gesucht wird.
- `erste_stelle` (optional): Die Stelle im Text, an der die Suche beginnt. Das erste Zeichen zählt als 1.

## SEARCHB

`SEARCHB(suchtext, text_innerhalb, [erste_stelle])`

Gibt an, an welcher Stelle ein Text zuerst in einem anderen Text vorkommt. Groß- und Kleinschreibung wird ignoriert.

- `suchtext`: Der Text, nach dem gesucht wird.
- `text_innerhalb`: Der Text, in dem gesucht wird.
- `erste_stelle` (optional): Die Stelle im Text, an der die Suche beginnt. Das erste Zeichen zählt als 1.

## SUBSTITUTE

`SUBSTITUTE(text, alter_text, neuer_text, [vorkommen])`

Tauscht ein Textstück gegen ein anderes aus, überall wo es vorkommt.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `alter_text`: Der Text, nach dem gesucht wird.
- `neuer_text`: Der Text, der eingefügt wird.
- `vorkommen` (optional): Welcher Treffer ersetzt wird (alle, wenn weggelassen).

## T

`T(wert)`

Gibt den Text selbst zurück und nichts für alles, was kein Text ist.

- `wert`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## TEXT

`TEXT(wert, format_text)`

Wandelt eine Zahl in Text in einem Format um, das Sie wählen.

- `wert`: Der Wert, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
- `format_text`: Das Zahlenformat, in dem der Wert angezeigt wird, z. B. "0,00" oder "TT.MM.JJJJ".

## TEXTJOIN

`TEXTJOIN(trennzeichen, leere_ignorieren, text1, ...)`

Verbindet Texte mit einem Trennzeichen dazwischen.

- `trennzeichen`: Der Text, der zwischen die Teile gesetzt wird.
- `leere_ignorieren`: WAHR, um leere Teile wegzulassen; FALSCH, um sie beizubehalten.
- `text1`: Der erste Text oder Zellbereich mit Text.
- `...`: Beliebig viele weitere derselben Art wie das Argument davor.

## TRIM

`TRIM(text)`

Entfernt überflüssige Leerzeichen aus einem Text und lässt zwischen Wörtern nur einzelne Leerzeichen stehen.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## UNICHAR

`UNICHAR(zahl)`

Gibt das Zeichen zurück, das zu einer Unicode-Zahl gehört.

- `zahl`: Die Zahl, mit der gearbeitet wird.

## UNICODE

`UNICODE(text)`

Gibt die Unicode-Zahl des ersten Zeichens eines Textes zurück.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## UPPER

`UPPER(text)`

Wandelt einen Text in Großbuchstaben um.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.

## VALUE

`VALUE(text)`

Wandelt einen Text, der wie eine Zahl aussieht, in eine Zahl um.

- `text`: Der Text, mit dem gearbeitet wird, oder eine Zelle, die ihn enthält.
