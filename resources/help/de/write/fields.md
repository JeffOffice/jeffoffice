# Felder

<!-- keywords: Feld, Felder, Feldfunktion, Feldfunktionen, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, Feld aktualisieren, F9, Verknüpfung aufheben, sperren, Feldfunktionen anzeigen, Seitenzahl, Datum, NUMPAGES, SEQ, REF, PAGEREF, Formel, Feld einfügen, Feld bearbeiten, Rechtsklick -->

Ein **Feld** ist ein Text, den das Programm für Sie ermittelt, zum Beispiel eine Seitenzahl, das heutige Datum oder die Nummer einer Abbildung. Es besteht aus zwei Teilen: einem **Code**, der festlegt, was ermittelt werden soll, und einem **Ergebnis**, also dem Text, den Sie sehen. Der Code für eine Seitenzahl ist `PAGE`, und sein Ergebnis auf der dritten Seite ist `3`.

## Ein Feld in den Text einfügen

1. Klicken Sie an die Stelle, an der das Feld stehen soll.
2. Klicken Sie auf **Einfügen > Feld**.
3. Wählen Sie in der Liste ein Feld aus. Sein Code erscheint im Eingabefeld darunter, und Sie können ihn ändern.
4. Klicken Sie auf **OK**.

Wählen Sie zum Beispiel **Datum**, ändern Sie den Code in `DATE \@ "MMMM d, yyyy"`, und klicken Sie auf **OK**. Der Text zeigt das Datum dann als `2. November 2026` an.

## Den Code eines Felds ändern

Klicken Sie mit der rechten Maustaste auf ein Feld, und wählen Sie **Feld bearbeiten**. Ein Dialogfeld zeigt den Code an. Ändern Sie ihn, und klicken Sie auf **OK**, dann wird das Feld neu ermittelt. Mit **Rückgängig** stellen Sie das alte Feld wieder her. Ein gesperrtes Feld lässt sich erst bearbeiten, wenn Sie es entsperren. Das Kontextmenü enthält außerdem **Feld aktualisieren**, **Feldfunktion ein/aus**, **Feld sperren**, **Feldsperre aufheben** und **Feldverknüpfung aufheben**.

## Felder aktualisieren

Ein Feld zeigt das Ergebnis, das es beim letzten Ermitteln hatte. Drücken Sie **F9**, um das Feld an der Einfügemarke neu zu ermitteln. Um viele Felder auf einmal zu aktualisieren, markieren Sie den Text, der sie enthält, und drücken Sie **F9**. Drücken Sie **Strg+A** und danach **F9**, um alle Felder im Dokument zu aktualisieren.

Das Aktualisieren ist ein einziger Schritt: Mit **Rückgängig** werden alle Ergebnisse wieder zurückgesetzt. Das Aktualisieren gilt nicht als nachverfolgte Änderung, auch wenn „Änderungen nachverfolgen“ aktiviert ist, weil das Ergebnis nichts ist, was Sie eingegeben haben.

Ein Feld in einer Kopf- oder Fußzeile, das die Seitenzahl, die Seitenanzahl oder eine Überschrift anzeigt, ist immer aktuell und muss nicht aktualisiert werden.

## Feldfunktionen anzeigen

Drücken Sie **Umschalt+F9**, um den Code des Felds an der Einfügemarke statt seines Ergebnisses in geschweiften Klammern anzuzeigen, zum Beispiel `{ PAGE }`. Drücken Sie **Alt+F9**, um die Codes aller Felder anzuzeigen, und drücken Sie die Tastenkombination erneut, um wieder die Ergebnisse anzuzeigen. Dies ändert nur die Anzeige: Die Datei wird so gespeichert, wie sie war, und das Dokument wird nicht als geändert markiert.

## Ein Feld von der Verknüpfung lösen oder sperren

- **Strg+Umschalt+F9** wandelt das Feld in normalen Text um. Das Ergebnis bleibt erhalten, ist aber kein Feld mehr.
- **Strg+F11** sperrt ein Feld, sodass **F9** es nicht verändert. **Strg+Umschalt+F11** hebt die Sperre wieder auf.

## Formate

Fügen Sie dem Code Schalter hinzu, um die Darstellung eines Ergebnisses zu ändern:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` und `\* Caps` ändern die Groß-/Kleinschreibung oder die Zahlenart. `{ = 21 \* CardText }` zeigt `twenty-one` an.
- `\# "0.00"` legt fest, wie eine Zahl aussieht. `{ = 1234.5 \# "#,##0.00" }` zeigt `1,234.50` an.
- `\@ "dddd, MMMM d"` legt fest, wie ein Datum aussieht. `M` steht für den Monat, `m` für die Minute.

## Felder, die JeffWrite ermittelt

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, eine Formel, die mit `=` beginnt, TOC (auch ein Abbildungsverzeichnis), INDEX, TOA, CITATION und BIBLIOGRAPHY. Die ausgeblendeten Markierungen TC, XE und TA haben kein Ergebnis, das ermittelt werden könnte: Sie legen fest, was in einer Tabelle, einem Stichwortverzeichnis oder einem Verzeichnis der Rechtsquellen aufgeführt wird. Ein Feld anderer Art behält sein Ergebnis und wird so gespeichert, wie es ist.

DATE, TIME, FILENAME und AUTHOR hängen vom Computer und vom Zeitpunkt ab, sodass sie sich bei jeder Aktualisierung ändern können.
