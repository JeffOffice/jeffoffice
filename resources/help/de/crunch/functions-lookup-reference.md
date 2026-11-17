# Nachschlage- und Bezugsfunktionen
<!-- keywords: functions, ADRESSE, ADDRESS, AREAS, BEREICHE, COLUMN, SPALTE, COLUMNS, SPALTEN, FILTER, FORMELTEXT, FORMULATEXT, GETPIVOTDATA, PIVOTDATENZUORDNEN, HLOOKUP, WVERWEIS, HYPERLINK, HYPERLINK, INDEX, INDIRECT, INDIREKT, LOOKUP, VERWEIS, MATCH, VERGLEICH, OFFSET, BEREICH.VERSCHIEBEN, ROW, ZEILE, ROWS, ZEILEN, SORT, SORTIEREN, SORTBY, SORTIERENNACH, TRANSPOSE, MTRANS, UNIQUE, EINDEUTIG, VLOOKUP, SVERWEIS, XLOOKUP, XVERWEIS, XMATCH, XVERGLEICH -->

Dies sind die Nachschlage- und Bezugsfunktionen, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und „...“ bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie eine Funktion in eine Zelle eingeben, zeigt ein kleiner Tipp unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Tipp öffnet ihren Eintrag hier.

## ADDRESS

`ADDRESS(row_num, column_num, [abs_num], [a1], [sheet_text])`

Schreibt die Adresse einer Zelle als Text, aus ihrer Zeilen- und Spaltennummer.

- `row_num`: Die Nummer der Zeile im Bereich, wobei die erste Zeile als 1 zählt.
- `column_num`: Die Nummer der Spalte im Bereich, wobei die erste Spalte als 1 zählt.
- `abs_num` (optional): Wie viel der Adresse festgelegt wird: 1 alles, 2 nur die Zeile, 3 nur die Spalte, 4 nichts davon.
- `a1` (optional): WAHR oder weggelassen, um die Adresse als Spaltenbuchstabe und Zeilennummer zu lesen; FALSCH, um sie als Zeilen- und Spaltennummern zu lesen.
- `sheet_text` (optional): Der Name des Blatts, der vor die Adresse gesetzt wird.

## AREAS

`AREAS(reference)`

Gibt die Anzahl getrennter Zellbereiche in einem Bezug an.

- `reference`: Eine Zelle oder ein Zellbereich.

## COLUMN

`COLUMN([reference])`

Gibt die Spaltennummer einer Zelle an.

- `reference` (optional): Eine Zelle oder ein Zellbereich.

## COLUMNS

`COLUMNS(array)`

Zählt die Spalten in einem Zellbereich.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.

## FILTER

`FILTER(array, include, [if_empty])`

Gibt die Zeilen oder Spalten eines Bereichs zurück, die eine Prüfung bestehen.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.
- `include`: Eine Spalte oder Zeile mit WAHR- und FALSCH-Werten, je einer für jede Zeile oder Spalte des Bereichs: WAHR behält sie bei.
- `if_empty` (optional): Was zurückgegeben wird, wenn keine Zeile oder Spalte die Prüfung besteht (ohne Angabe der Fehler #KALK!).

## FORMULATEXT

`FORMULATEXT(reference)`

Gibt die Formel einer Zelle als Text an.

- `reference`: Eine Zelle oder ein Zellbereich.

## GETPIVOTDATA

`GETPIVOTDATA(data_field, pivot_table, [field1], [item1], ...)`

Gibt den Wert an, den eine Pivot-Tabelle für einen Wert und die Elemente ihrer Zeilen und Spalten anzeigt. Eine Zelle der Tabelle ist das zweite Argument; die angegebenen Felder müssen die ersten der Zeilen oder der Spalten sein.

- `data_field`: Der Name des Werts, in Anführungszeichen, so wie die Pivot-Tabelle ihn anzeigt („Summe von Betrag“).
- `pivot_table`: Eine beliebige Zelle der Pivot-Tabelle.
- `field1` (optional): Der Name eines Felds der Zeilen oder der Spalten, in Anführungszeichen (kann zusammen mit dem Element weggelassen werden).
- `item1` (optional): Das Element dieses Felds, für das der Wert angegeben wird, in Anführungszeichen.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## HLOOKUP

`HLOOKUP(lookup_value, table_array, row_index_num, [range_lookup])`

Sucht einen Wert in der ersten Zeile einer Tabelle und gibt einen Wert aus derselben Spalte zurück.

- `lookup_value`: Der Wert, nach dem gesucht wird.
- `table_array`: Der Zellbereich, in dem gesucht wird; der gesuchte Wert muss in seiner ersten Spalte stehen.
- `row_index_num`: Die Nummer der Zeile der Tabelle, aus der ein Wert zurückgegeben wird, wobei die erste Zeile als 1 zählt.
- `range_lookup` (optional): FALSCH findet eine genaue Übereinstimmung; WAHR oder weggelassen findet den nächstkleineren Wert (die erste Zeile muss aufsteigend sortiert sein).

## HYPERLINK

`HYPERLINK(link_location, [friendly_name])`

Erstellt eine Zelle, die beim Klicken einen Link öffnet.

- `link_location`: Das Ziel des Links: eine Webadresse, eine Datei oder eine Stelle in dieser Arbeitsmappe.
- `friendly_name` (optional): Der Text, der in der Zelle angezeigt wird (ohne Angabe wird der Link selbst angezeigt).

## INDEX

`INDEX(array, row_num, [column_num], [area_num])`

Gibt den Wert an einer Zeile und Spalte eines Zellbereichs zurück.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.
- `row_num`: Die Nummer der Zeile im Bereich, wobei die erste Zeile als 1 zählt.
- `column_num` (optional): Die Nummer der Spalte im Bereich, wobei die erste Spalte als 1 zählt.
- `area_num` (optional): Welcher Bereich verwendet wird, wenn das erste Argument mehrere benennt (1 für den ersten).

## INDIRECT

`INDIRECT(ref_text, [a1])`

Gibt die Zelle zurück, auf die eine als Text geschriebene Adresse verweist.

- `ref_text`: Eine als Text geschriebene Adresse, zum Beispiel „B5“.
- `a1` (optional): WAHR oder weggelassen, um die Adresse als Spaltenbuchstabe und Zeilennummer zu lesen; FALSCH, um sie als Zeilen- und Spaltennummern zu lesen.

## LOOKUP

`LOOKUP(lookup_value, lookup_vector, [result_vector])`

Sucht einen Wert in einer Zeile oder Spalte und gibt den Wert an derselben Stelle in einer anderen zurück.

- `lookup_value`: Der Wert, nach dem gesucht wird.
- `lookup_vector`: Die eine Zeile oder die eine Spalte mit Zellen, in der gesucht wird.
- `result_vector` (optional): Die eine Zeile oder die eine Spalte mit Zellen, aus der das Ergebnis zurückgegeben wird.

## MATCH

`MATCH(lookup_value, lookup_array, [match_type])`

Gibt die Position eines Werts in einer Liste an.

- `lookup_value`: Der Wert, nach dem gesucht wird.
- `lookup_array`: Der Zellbereich, eine Zeile oder eine Spalte, in dem gesucht wird.
- `match_type` (optional): 1 oder weggelassen findet den größten Wert, der nicht über dem gesuchten liegt (die Liste muss aufsteigend sortiert sein); 0 findet eine genaue Übereinstimmung; -1 findet den kleinsten Wert, der nicht unter dem gesuchten liegt (die Liste muss absteigend sortiert sein).

## OFFSET

`OFFSET(reference, rows, cols, [height], [width])`

Gibt die Zellen zurück, die eine bestimmte Anzahl Zeilen und Spalten von einer Ausgangszelle entfernt liegen.

- `reference`: Eine Zelle oder ein Zellbereich.
- `rows`: Um wie viele Zeilen verschoben wird: positiv nach unten, negativ nach oben.
- `cols`: Um wie viele Spalten verschoben wird: positiv nach rechts, negativ nach links.
- `height` (optional): Wie viele Zeilen hoch das Ergebnis ist.
- `width` (optional): Wie viele Spalten breit das Ergebnis ist.

## ROW

`ROW([reference])`

Gibt die Zeilennummer einer Zelle an.

- `reference` (optional): Eine Zelle oder ein Zellbereich.

## ROWS

`ROWS(array)`

Zählt die Zeilen in einem Zellbereich.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.

## SORT

`SORT(array, [sort_index], [sort_order], [by_col])`

Gibt einen Bereich zurück, dessen Zeilen oder Spalten sortiert sind.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.
- `sort_index` (optional): Die Nummer der Spalte (oder Zeile), nach der sortiert wird (1, wenn weggelassen).
- `sort_order` (optional): 1 oder weggelassen für aufsteigend; -1 für absteigend.
- `by_col` (optional): WAHR, um nach Spalten zu arbeiten; FALSCH oder weggelassen, um nach Zeilen zu arbeiten.

## SORTBY

`SORTBY(array, by_array1, [sort_order1], ...)`

Gibt einen Bereich zurück, der in der Reihenfolge eines oder mehrerer anderer Bereiche sortiert ist.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.
- `by_array1`: Der Zellbereich, nach dessen Reihenfolge der erste Bereich sortiert wird; er hat dieselbe Länge wie der erste Bereich.
- `sort_order1` (optional): 1 oder weggelassen für aufsteigend; -1 für absteigend.
- `...`: Beliebig viele weitere derselben Art wie das vorherige Argument.

## TRANSPOSE

`TRANSPOSE(array)`

Macht aus Zeilen Spalten und aus Spalten Zeilen.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.

## UNIQUE

`UNIQUE(array, [by_col], [exactly_once])`

Gibt die verschiedenen Zeilen oder Spalten eines Bereichs zurück.

- `array`: Ein Zellbereich oder eine Liste von Werten, die in geschweiften Klammern geschrieben wird.
- `by_col` (optional): WAHR, um nach Spalten zu arbeiten; FALSCH oder weggelassen, um nach Zeilen zu arbeiten.
- `exactly_once` (optional): WAHR, um nur die Einträge zurückzugeben, die genau einmal vorkommen; FALSCH oder weggelassen, um jeden verschiedenen Eintrag zurückzugeben.

## VLOOKUP

`VLOOKUP(lookup_value, table_array, col_index_num, [range_lookup])`

Sucht einen Wert in der ersten Spalte einer Tabelle und gibt einen Wert aus derselben Zeile zurück.

- `lookup_value`: Der Wert, nach dem gesucht wird.
- `table_array`: Der Zellbereich, in dem gesucht wird; der gesuchte Wert muss in seiner ersten Spalte stehen.
- `col_index_num`: Die Nummer der Spalte der Tabelle, aus der ein Wert zurückgegeben wird, wobei die erste Spalte als 1 zählt.
- `range_lookup` (optional): FALSCH findet eine genaue Übereinstimmung; WAHR oder weggelassen findet den nächstkleineren Wert (die erste Spalte muss aufsteigend sortiert sein).

## XLOOKUP

`XLOOKUP(lookup_value, lookup_array, return_array, [if_not_found], [match_mode], [search_mode])`

Sucht einen Wert in einer Liste und gibt das passende Element einer anderen Liste zurück.

- `lookup_value`: Der Wert, nach dem gesucht wird.
- `lookup_array`: Der Zellbereich, eine Zeile oder eine Spalte, in dem gesucht wird.
- `return_array`: Der Zellbereich, aus dem die Übereinstimmung zurückgegeben wird.
- `if_not_found` (optional): Was zurückgegeben wird, wenn es keine Übereinstimmung gibt (ohne Angabe der Fehler #NV).
- `match_mode` (optional): 0 oder weggelassen für eine genaue Übereinstimmung, -1 für den nächstkleineren Wert, 1 für den nächstgrößeren Wert, 2 für die Verwendung von Platzhaltern.
- `search_mode` (optional): 1 oder weggelassen, um vom ersten an zu suchen, -1 vom letzten an, 2 oder -2 für eine binäre Suche in einer sortierten Liste.

## XMATCH

`XMATCH(lookup_value, lookup_array, [match_mode], [search_mode])`

Gibt die Position eines Werts in einer Liste an, mit Auswahlmöglichkeiten dafür, wie verglichen und in welche Richtung gesucht wird.

- `lookup_value`: Der Wert, nach dem gesucht wird.
- `lookup_array`: Der Zellbereich, eine Zeile oder eine Spalte, in dem gesucht wird.
- `match_mode` (optional): 0 oder weggelassen für eine genaue Übereinstimmung, -1 für den nächstkleineren Wert, 1 für den nächstgrößeren Wert, 2 für die Verwendung von Platzhaltern.
- `search_mode` (optional): 1 oder weggelassen, um vom ersten an zu suchen, -1 vom letzten an, 2 oder -2 für eine binäre Suche in einer sortierten Liste.
