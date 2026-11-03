# Felder
<!-- keywords: Feld, Felder, Feld aktualisieren, F9, Verknüpfung aufheben, Feldverknüpfung aufheben, sperren, Feld sperren, Feldfunktionen, Feldcode, Seitenzahl, Datum, NUMPAGES, SEQ, REF, PAGEREF, Formel, Feld einfügen -->

Ein **Feld** ist ein Text, den das Programm für Sie berechnet, z. B. eine Seitenzahl, das heutige Datum oder die Nummer einer Abbildung. Ein Feld besteht aus zwei Teilen: einer **Feldfunktion**, die angibt, was berechnet wird, und einem **Ergebnis**, dem Text, den Sie sehen. Die Feldfunktion für eine Seitenzahl lautet `PAGE`, ihr Ergebnis auf der dritten Seite ist `3`.

## Ein Feld in den Text einfügen

1. Klicken Sie an die Stelle, an der das Feld stehen soll.
2. Klicken Sie auf **Einfügen > Feld**.
3. Wählen Sie ein Feld aus der Liste aus. Seine Feldfunktion wird im Feld darunter angezeigt und kann dort geändert werden.
4. Klicken Sie auf **OK**.

Wählen Sie beispielsweise **Datum**, ändern Sie die Feldfunktion in `DATE \@ "d. MMMM yyyy"`, und klicken Sie auf **OK**. Im Text wird das Datum dann als `2. November 2026` angezeigt.

## Felder aktualisieren

Ein Feld zeigt das Ergebnis seiner letzten Berechnung an. Drücken Sie **F9**, um das Feld, in dem sich die Einfügemarke befindet, neu zu berechnen. Um mehrere Felder auf einmal zu aktualisieren, markieren Sie den Text, der sie enthält, und drücken Sie **F9**. Drücken Sie **Strg+A** und dann **F9**, um alle Felder im Dokument zu aktualisieren.

Das Aktualisieren ist ein einziger Schritt: **Rückgängig** stellt alle Ergebnisse wieder her. Das Aktualisieren wird nicht als Änderung nachverfolgt, auch wenn „Änderungen nachverfolgen“ aktiviert ist, denn das Ergebnis haben Sie nicht selbst eingegeben.

Ein Feld in einer Kopf- oder Fußzeile, das die Seitenzahl, die Seitenanzahl oder eine Überschrift anzeigt, ist immer aktuell und muss nicht aktualisiert werden.

## Feldfunktionen anzeigen

Drücken Sie **Umschalt+F9**, um statt des Ergebnisses die Feldfunktion des Felds, in dem sich die Einfügemarke befindet, in geschweiften Klammern anzuzeigen, z. B. `{ PAGE }`. Drücken Sie **Alt+F9**, um die Feldfunktionen aller Felder anzuzeigen, und drücken Sie die Tastenkombination erneut, um wieder die Ergebnisse anzuzeigen. Dadurch ändert sich nur die Anzeige: Die Datei wird unverändert gespeichert, und das Dokument wird nicht als geändert gekennzeichnet.

## Feldverknüpfung aufheben oder Feld sperren

- **Strg+Umschalt+F9** wandelt das Feld in normalen Text um. Das Ergebnis bleibt erhalten, ist aber kein Feld mehr.
- **Strg+F11** sperrt ein Feld, sodass **F9** es nicht verändert. **Strg+Umschalt+F11** hebt die Sperre auf.

## Formate

Fügen Sie der Feldfunktion Schalter hinzu, um die Darstellung des Ergebnisses zu ändern:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` und `\* Caps` ändern die Groß-/Kleinschreibung oder die Zahlendarstellung. `{ = 21 \* CardText }` zeigt `einundzwanzig` an.
- `\# "0,00"` legt fest, wie eine Zahl dargestellt wird. `{ = 1234,5 \# "#.##0,00" }` zeigt `1.234,50` an.
- `\@ "dddd, d. MMMM"` legt fest, wie ein Datum dargestellt wird. `M` steht für den Monat, `m` für die Minute.

## Felder, die JeffWrite berechnet

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, eine Formel, die mit `=` beginnt, und TOC. Ein Feld eines anderen Typs behält sein Ergebnis und wird unverändert gespeichert.

DATE, TIME, FILENAME und AUTHOR hängen vom Computer und vom Zeitpunkt ab und können sich daher bei jeder Aktualisierung ändern.
