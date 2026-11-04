# Felder

<!-- keywords: Feld, Felder, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, Feld aktualisieren, F9, Verknüpfung aufheben, sperren, Feldfunktionen, Seitenzahl, Datum, NUMPAGES, SEQ, REF, PAGEREF, Formel, Feld einfügen -->

Ein **Feld** ist ein Text, den das Programm für Sie berechnet, zum Beispiel eine Seitenzahl, das heutige Datum oder die Nummer einer Abbildung. Es besteht aus zwei Teilen: einem **Code**, der angibt, was berechnet werden soll, und einem **Ergebnis**, das Sie als Text sehen. Der Code für eine Seitenzahl ist `PAGE`, und sein Ergebnis auf der dritten Seite ist `3`.

## Ein Feld in den Text einfügen

1. Klicken Sie an die Stelle, an der das Feld stehen soll.
2. Klicken Sie auf **Einfügen > Feld**.
3. Wählen Sie ein Feld aus der Liste. Sein Code erscheint im Feld darunter, und Sie können ihn ändern.
4. Klicken Sie auf **OK**.

Wählen Sie zum Beispiel **Datum**, ändern Sie den Code in `DATE \@ "MMMM d, yyyy"`, und klicken Sie auf **OK**. Der Text zeigt das Datum als `2. November 2026` an.

## Felder aktualisieren

Ein Feld zeigt das Ergebnis, das es beim letzten Berechnen hatte. Drücken Sie **F9**, um das Feld, in dem sich die Einfügemarke befindet, erneut zu berechnen. Um viele Felder auf einmal zu aktualisieren, markieren Sie den Text, der sie enthält, und drücken Sie **F9**. Drücken Sie **Strg+A** und danach **F9**, um alle Felder im Dokument zu aktualisieren.

Das Aktualisieren ist ein einzelner Schritt: **Rückgängig** stellt alle Ergebnisse wieder her. Das Aktualisieren ist keine nachverfolgte Änderung, auch wenn „Änderungen nachverfolgen“ aktiviert ist, weil das Ergebnis nichts ist, was Sie eingegeben haben.

Ein Feld in einer Kopf- oder Fußzeile, das die Seitenzahl, die Seitenanzahl oder eine Überschrift anzeigt, ist immer richtig und muss nicht aktualisiert werden.

## Codes anzeigen

Drücken Sie **Umschalt+F9**, um den Code des Felds, in dem sich die Einfügemarke befindet, in geschweiften Klammern wie `{ PAGE }` anstelle seines Ergebnisses anzuzeigen. Drücken Sie **Alt+F9**, um die Codes aller Felder anzuzeigen, und drücken Sie die Taste erneut, um wieder die Ergebnisse anzuzeigen. Dies ändert nur, was Sie sehen: Die Datei wird so gespeichert, wie sie war, und das Dokument wird nicht als geändert markiert.

## Ein Feld aufheben oder sperren

- **Strg+Umschalt+F9** wandelt das Feld in normalen Text um. Das Ergebnis bleibt erhalten, ist aber kein Feld mehr.
- **Strg+F11** sperrt ein Feld, sodass **F9** es nicht verändert. **Strg+Umschalt+F11** hebt die Sperre wieder auf.

## Formate

Fügen Sie dem Code Schalter hinzu, um zu ändern, wie ein Ergebnis aussieht:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` und `\* Caps` ändern die Groß- und Kleinschreibung oder die Zahlenart. `{ = 21 \* CardText }` zeigt `einundzwanzig`.
- `\# "0.00"` legt fest, wie eine Zahl aussieht. `{ = 1234.5 \# "#,##0.00" }` zeigt `1.234,50`.
- `\@ "dddd, MMMM d"` legt fest, wie ein Datum aussieht. `M` steht für den Monat, und `m` für die Minute.

## Felder, die JeffWrite berechnet

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, eine Formel, die mit `=` beginnt, TOC (auch ein Abbildungsverzeichnis), INDEX, TOA, CITATION und BIBLIOGRAPHY. Die ausgeblendeten Markierungen TC, XE und TA haben kein Ergebnis, das berechnet werden müsste: Sie geben einer Tabelle, einem Stichwortverzeichnis oder einem Verzeichnis der Rechtsquellen an, was aufgelistet werden soll. Ein Feld jeder anderen Art behält sein Ergebnis und wird so gespeichert, wie es war.

DATE, TIME, FILENAME und AUTHOR hängen vom Computer und vom Zeitpunkt ab. Sie können sich daher bei jeder Aktualisierung ändern.
