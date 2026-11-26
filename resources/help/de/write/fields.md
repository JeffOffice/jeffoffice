# Felder

<!-- keywords: Feld, Felder, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, Feld aktualisieren, F9, Verknüpfung aufheben, sperren, Feldfunktionen, Seitenzahl, Datum, NUMPAGES, SEQ, REF, PAGEREF, Formel, Feld einfügen, Feld bearbeiten, Rechtsklick -->

Ein **Feld** ist ein Stück Text, das das Programm für Sie berechnet, zum Beispiel eine Seitenzahl, das heutige Datum oder die Nummer einer Abbildung. Es besteht aus zwei Teilen: einem **Code**, der festlegt, was berechnet werden soll, und einem **Ergebnis**, also dem Text, den Sie sehen. Der Code für eine Seitenzahl ist `PAGE`, und das Ergebnis auf der dritten Seite ist `3`.

## Ein Feld in den Text einfügen

1. Klicken Sie an die Stelle, an der das Feld stehen soll.
2. Klicken Sie auf **Einfügen > Feld**.
3. Wählen Sie ein Feld aus der Liste. Sein Code erscheint im Feld darunter, und Sie können ihn ändern.
4. Klicken Sie auf **OK**.

Wählen Sie zum Beispiel **Datum**, ändern Sie den Code in `DATE \@ "MMMM d, yyyy"`, und klicken Sie auf **OK**. Im Text wird das Datum als `2. November 2026` angezeigt.

## Den Code eines Felds ändern

Klicken Sie mit der rechten Maustaste auf ein Feld, und wählen Sie **Feld bearbeiten**. Ein Dialogfeld zeigt den Code an. Ändern Sie ihn, und klicken Sie auf **OK**. Das Feld wird dann neu berechnet. Mit **Rückgängig** stellen Sie das alte Feld wieder her. Ein gesperrtes Feld lässt sich erst bearbeiten, wenn Sie es entsperren. Das Kontextmenü enthält außerdem **Feld aktualisieren**, **Feldfunktion ein/aus**, **Feld sperren**, **Feldsperre aufheben** und **Feldverknüpfung aufheben**.

## Felder aktualisieren

Ein Feld zeigt das Ergebnis, das es beim letzten Berechnen hatte. Drücken Sie **F9**, um das Feld an der Einfügemarke erneut zu berechnen. Um viele Felder auf einmal zu aktualisieren, markieren Sie den Text, der sie enthält, und drücken Sie **F9**. Drücken Sie **Strg+A** und danach **F9**, um alle Felder im Dokument zu aktualisieren.

Das Aktualisieren ist ein einziger Schritt: Mit **Rückgängig** werden alle Ergebnisse wiederhergestellt. Das Aktualisieren gilt nicht als nachverfolgte Änderung, auch wenn „Änderungen nachverfolgen“ aktiviert ist, weil das Ergebnis nichts ist, was Sie eingegeben haben.

Ein Feld in einer Kopf- oder Fußzeile, das die Seitenzahl, die Seitenanzahl oder eine Überschrift anzeigt, ist immer richtig und muss nicht aktualisiert werden.

## Feldfunktionen anzeigen

Drücken Sie **Umschalt+F9**, um den Code des Felds an der Einfügemarke in geschweiften Klammern anzuzeigen, zum Beispiel `{ PAGE }`, anstelle des Ergebnisses. Drücken Sie **Alt+F9**, um die Codes aller Felder anzuzeigen, und drücken Sie die Tastenkombination erneut, um wieder die Ergebnisse zu sehen. Das ändert nur die Anzeige: Die Datei wird so gespeichert, wie sie war, und das Dokument wird nicht als geändert markiert.

## Felder schattieren

Um zu sehen, wo sich Felder befinden, öffnen Sie **Datei > Optionen > Anzeige** und wählen Sie **Feldschattierung**. **Nie** zeigt keine Schattierung an. **Immer** schattiert jedes Feld hellgrau. **Bei Auswahl** schattiert nur das Feld, in dem sich die Einfügemarke befindet. Die Schattierung wird auf dem Bildschirm angezeigt und niemals gedruckt oder in einer PDF-Datei gespeichert.

## Felder vor dem Drucken aktualisieren

Aktivieren Sie unter **Datei > Optionen > Anzeige** das Kontrollkästchen **Felder vor dem Drucken aktualisieren**. Dann werden alle Felder jedes Mal neu berechnet, wenn Sie drucken oder als PDF-Datei speichern. So stimmen Seitenverweise, Datumsangaben und Zählungen auch auf dem Papier. Dies ist ein Schritt, den Sie rückgängig machen können. Bei einem Dokument, das nur zum Lesen geöffnet ist, geschieht dies nicht.

## Feld aufheben oder sperren

- **Strg+Umschalt+F9** wandelt das Feld in normalen Text um. Das Ergebnis bleibt erhalten, ist aber kein Feld mehr.
- **Strg+F11** sperrt ein Feld, sodass **F9** es unverändert lässt. **Strg+Umschalt+F11** hebt die Sperre wieder auf.

## Formate

Fügen Sie dem Code Schalter hinzu, um zu ändern, wie ein Ergebnis aussieht:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` und `\* Caps` ändern die Groß- und Kleinschreibung oder die Art der Zahl. `{ = 21 \* CardText }` zeigt `einundzwanzig`.
- `\# "0.00"` legt fest, wie eine Zahl aussieht. `{ = 1234.5 \# "#,##0.00" }` zeigt `1.234,50`.
- `\@ "dddd, MMMM d"` legt fest, wie ein Datum aussieht. `M` steht für den Monat, und `m` für die Minute.

## Felder, die JeffWrite berechnet

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, eine Formel, die mit `=` beginnt, TOC (auch ein Abbildungsverzeichnis), INDEX, TOA, CITATION und BIBLIOGRAPHY. Die ausgeblendeten Markierungen TC, XE und TA haben kein Ergebnis, das berechnet werden müsste: Sie legen fest, was in einer Tabelle, einem Stichwortverzeichnis oder einem Verzeichnis der Rechtsquellen aufgeführt wird. Ein Feld jeder anderen Art behält sein Ergebnis und wird so gespeichert, wie es war.

DATE, TIME, FILENAME und AUTHOR hängen vom Computer und vom Zeitpunkt ab, sodass sie sich bei jeder Aktualisierung ändern können.
