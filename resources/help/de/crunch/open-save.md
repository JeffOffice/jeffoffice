# Arbeitsmappen öffnen und speichern
<!-- keywords: neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, getrennt, Trennzeichen, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, Drag & Drop, laden, groß, große Dateien, Makros, nicht gespeichert, Änderungen, schließen, Kopie, Berechnungskette -->

Wenn JeffCrunch ohne zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 usw.), und eine Nummer wird nicht wieder vergeben, nachdem ihr Fenster geschlossen wurde. Um eine weitere Arbeitsmappe zu beginnen, drücken Sie **{key:New}**, oder klicken Sie auf **Datei** und dann auf **Neu**, und wählen Sie **Leere Arbeitsmappe**. Sie wird in einem eigenen Fenster geöffnet. Eine neue Arbeitsmappe, die Sie nicht geändert haben, wird ersetzt, wenn Sie in ihrem Fenster eine Datei öffnen. Eine geänderte bleibt erhalten, und die Datei wird in einem anderen Fenster geöffnet. **Speichern** fragt bei einer neuen Arbeitsmappe wie **Speichern unter** nach einem Namen und speichert sie als .xlsx-Datei. Beim Schließen einer neuen Arbeitsmappe, die Sie nicht geändert haben, erscheint keine Rückfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Open}**, oder klicken Sie auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus Ihrem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Stattdessen wird dieses Fenster in den Vordergrund geholt.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (solche mit Makros). Makros werden nicht ausgeführt, bleiben aber unverändert erhalten.

## Andere Dateiarten

JeffCrunch öffnet auch ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und das flache **.fods**) sowie Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine Arbeitsmappe, die aus einer dieser Dateien geöffnet wurde, wird als neue **.xlsx**-Datei gespeichert: Wenn Sie **{key:Save}** drücken, weist JeffCrunch Sie darauf hin und fragt nach einem Namen. Die geöffnete Datei wird nie verändert.

Aus einer OpenDocument-Tabelle werden Blätter, Werte, Formeln (mit den in der Datei gespeicherten Ergebnissen), Zahlenformate, Schriftarten, Füllungen, Rahmen, verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, fixierte Fensterbereiche und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, PivotTables, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen. Eine Meldung oben im Fenster zeigt an, welche davon die Datei enthält; eine von hier aus gespeicherte Kopie enthält sie nicht. Eine kennwortgeschützte Tabelle wird nicht geöffnet: Öffnen Sie sie in dem Programm, mit dem sie erstellt wurde, und speichern Sie eine Kopie ohne Kennwort.

Eine Textdatei ergibt ein Blatt. JeffCrunch erkennt, wie die Werte getrennt sind (durch Kommas, Semikolons, Tabulatoren oder senkrechte Striche) und wie die Zeichen codiert sind (UTF-8 oder UTF-16, andernfalls die Codepage Ihrer Sprache), und liest jeden Wert so, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 im Deutschen, einen Prozentwert, einen Geldbetrag, ein Datum mit Jahr (etwa 15.03.2024 im Deutschen oder 3/15/2024 im Englischen), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache, im Deutschen also WAHR und FALSCH, und auf Englisch TRUE und FALSE). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Stellen bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Ebenso bleiben Tag und Monat ohne Jahr Text, in jeder Reihenfolge und in jeder Sprache (etwa 3/4, 12.5, 1-2, March 1 oder Dec 25), weil das Jahr sonst erfunden werden müsste, und auch Texte wie der Genname MARCH1. Ein Blatt umfasst 1.048.576 Zeilen und 16.384 Spalten. Enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und weist Sie darauf hin.

Wenn Sie eine Textdatei öffnen, zeigt ein Fenster zunächst die ersten Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (Komma, Semikolon, Tabulator, Leerzeichen, senkrechter Strich oder ein selbst eingegebenes Zeichen, auf Wunsch mit mehreren aufeinanderfolgenden Trennzeichen als eines gezählt), womit ein Feld, das ein Trennzeichen enthält, in Anführungszeichen gesetzt ist, die Textcodierung und die Zeile, ab der importiert wird. Über jeder Spalte wählen Sie, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu lassen), **Datum** in der von Ihnen angegebenen Reihenfolge oder **Überspringen**. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, geöffnet wird aber die ganze Datei. **Datei > Öffnen > Textdatei öffnen** fragt auch dann nach, wenn der Dateiname ungewöhnlich ist.

Um nur die Werte des angezeigten Blatts zu speichern, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie den Dateityp **.csv**. JeffCrunch schreibt die Werte so, wie die Zellen sie anzeigen, in UTF-8, mit einem Semikolon dazwischen, wenn Ihre Sprache in Zahlen ein Komma schreibt (wie im Deutschen), sonst mit einem Komma, und setzt Text bei Bedarf in Anführungszeichen. Die anderen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in einer .csv-Datei enthalten, und Ihre Arbeitsmappe wird durch das Schreiben nicht verändert.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern geöffnet war, erscheinen fast sofort, auch bei sehr großen Arbeitsmappen. Die Statusleiste unten links zeigt an, was gerade geschieht: **Öffnen**, **Lesen** eines Blatts mit der Anzahl der bisher gelesenen Zellen, **Berechnung** mit einer Prozentangabe und schließlich **Bereit**.

Während des Lesens können Sie scrollen, auswählen und kopieren. Die Registerkarten der anderen Blätter werden aktiv, sobald diese gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie sich lesen lässt, und eine Meldung nennt, was ausgelassen wurde.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Menü „Datei“ listet die zuletzt geöffneten Arbeitsmappen auf. Klicken Sie mit der rechten Maustaste auf einen Eintrag, um ihn oben in der Liste **anzuheften** oder ihn aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Save}**, oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei zu erstellen, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine ohne Makros als **.xlsx**-Datei.

JeffCrunch speichert nur, was Sie geändert haben. Alles andere in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird Byte für Byte unverändert übernommen. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den davon abhängigen Formeln, die ihre neuen Werte erhalten. Wenn Sie keine Zelle geändert haben, bleibt die Datei unberührt.

Wenn ein anderes Programm die Datei geändert hat, nachdem Sie sie geöffnet haben, überschreibt JeffCrunch sie nicht ohne Rückfrage, weil sonst die Änderungen des anderen Programms verloren gingen. Sie werden gefragt, ob Sie **Kopie speichern**, die Datei **Überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zunächst neben der alten geschrieben und ersetzt diese erst, wenn sie vollständig ist. Geht dabei etwas schief, erhalten Sie eine Meldung, und die vorhandene Datei bleibt unverändert.

Wenn die Formeln beim Speichern noch berechnet werden, wartet JeffCrunch, bis die Berechnung abgeschlossen ist, damit die Datei die Ergebnisse enthält. Wenn Sie währenddessen **Esc** drücken, um die Berechnung anzuhalten, wird das Speichern abgebrochen, und die Statusleiste zeigt dies an; speichern Sie dann erneut, wenn Sie möchten. Eines lässt JeffCrunch in der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Das Programm, das die Arbeitsmappe als Nächstes öffnet, erstellt sie neu. Wenn JeffCrunch eine Formel nicht berechnen konnte oder Sie die Berechnung angehalten haben, fordert die Datei dieses Programm auf, beim Öffnen alle Formeln neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie die Änderungen **Speichern**, **Nicht speichern** oder mit **Abbrechen** weiterarbeiten möchten. Ein Sternchen (*) im Fenstertitel zeigt an, dass es nicht gespeicherte Änderungen gibt.
