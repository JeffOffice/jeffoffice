# Arbeitsmappen öffnen und speichern
<!-- keywords: öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, Trennzeichen, getrennt, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, Drag & Drop, laden, große Dateien, groß, Makros, nicht gespeichert, Änderungen, schließen, Kopie, Berechnungskette -->

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Open}**, oder klicken Sie auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus Ihrem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Stattdessen wird dieses Fenster in den Vordergrund geholt.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (solche mit Makros). Makros werden nicht ausgeführt, aber unverändert beibehalten.

## Andere Dateitypen

JeffCrunch öffnet außerdem ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und die einfache XML-Variante **.fods**) sowie Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine aus einer solchen Datei geöffnete Arbeitsmappe wird als neue **.xlsx**-Datei gespeichert: Wenn Sie **{key:Save}** drücken, weist JeffCrunch Sie darauf hin und fragt nach einem Namen. Die geöffnete Datei selbst wird nie verändert.

Aus einer OpenDocument-Tabelle werden die Blätter, Werte, Formeln (mit den in der Datei gespeicherten Ergebnissen), Zahlenformate, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Fensterbereiche und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, PivotTables, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen. Eine Meldung oben im Fenster zeigt an, welche davon die Datei enthält; eine von hier gespeicherte Kopie enthält sie nicht. Eine kennwortgeschützte Tabelle wird nicht geöffnet: Öffnen Sie sie in dem Programm, mit dem sie erstellt wurde, und speichern Sie eine Kopie ohne Kennwort.

Eine Textdatei ergibt ein Blatt. JeffCrunch erkennt, wodurch die Werte getrennt sind (Kommas, Semikolons, Tabulatoren oder senkrechte Striche) und wie die Zeichen codiert sind (UTF-8 oder UTF-16, andernfalls die Codepage Ihrer Sprache). Jeder Wert wird so gelesen, als hätten Sie ihn in Ihrer Sprache eingegeben: eine Zahl wie 1.234,5 im Deutschen, ein Prozentwert, ein Geldbetrag, ein Datum mit Jahr (etwa 15.03.2024 im Deutschen oder 3/15/2024), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache und auf Englisch, also auch TRUE und FALSE). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen mit führender Null (etwa 007) und Zahlen mit mehr als 15 Stellen bleiben Text, damit eine Postleitzahl oder Kontonummer nicht verändert wird. Ebenso bleiben Tag und Monat ohne Jahr Text, in jeder Reihenfolge und Sprache (etwa 3/4, 12.5, 1-2, March 1 oder Dec 25), weil das Jahr sonst erfunden werden müsste, und auch Texte wie der Genname MARCH1. Ein Blatt umfasst 1.048.576 Zeilen und 16.384 Spalten; enthält die Datei mehr, öffnet JeffCrunch den Teil, der hineinpasst, und weist Sie darauf hin.

Wenn Sie eine Textdatei öffnen, zeigt ein Fenster zunächst die ersten Zeilen an und fragt, wie die Datei gelesen werden soll: wodurch die Felder getrennt sind (beliebig kombiniert aus Komma, Semikolon, Tabulator, Leerzeichen, senkrechtem Strich oder einem selbst eingegebenen Zeichen, wobei aufeinanderfolgende Trennzeichen auf Wunsch als eines gelten), welches Zeichen ein Feld mit Trennzeichen einschließt, die Textcodierung und die Zeile, mit der begonnen wird. Über jeder Spalte legen Sie fest, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu lassen), ein **Datum** in der angegebenen Reihenfolge oder **Überspringen**. Die Vorschau zeigt jede Änderung sofort und umfasst höchstens die ersten 200 Zeilen und 100 Spalten, geöffnet wird aber die ganze Datei. **Datei > Öffnen > Textdatei öffnen** fragt auch dann nach, wenn der Dateiname ungewöhnlich ist.

Um nur die Werte des angezeigten Blatts zu speichern, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie den Dateityp **.csv**. JeffCrunch schreibt die Werte so, wie die Zellen sie anzeigen, in UTF-8, getrennt durch ein Semikolon, wenn Ihre Sprache in Zahlen ein Komma verwendet (wie das Deutsche), sonst durch ein Komma, und setzt Text bei Bedarf in Anführungszeichen. Die anderen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in einer CSV-Datei enthalten, und Ihre Arbeitsmappe wird dadurch nicht verändert.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern geöffnet war, erscheinen fast sofort, selbst bei sehr großen Arbeitsmappen. Die Statusleiste unten links zeigt an, was gerade geschieht: **Öffnen**, **Lesen** eines Blatts mit der Anzahl der bisher gelesenen Zellen, **Berechnung** mit einer Prozentangabe und schließlich **Bereit**.

Während des Lesens können Sie bereits scrollen, markieren und kopieren. Die Registerkarten der anderen Blätter werden aktiv, sobald sie gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie lesbar ist, und eine Meldung nennt, was ausgelassen wurde.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Menü „Datei“ listet die zuletzt geöffneten Arbeitsmappen auf. Klicken Sie mit der rechten Maustaste auf einen Eintrag, um ihn oben in der Liste **anzuheften** oder ihn aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Save}**, oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei zu erstellen, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine ohne Makros als **.xlsx**-Datei.

JeffCrunch speichert nur, was Sie geändert haben. Alles andere in der Datei, etwa die anderen Blätter, Diagramme, Bilder, Makros und Einstellungen, die JeffCrunch nicht anzeigt, wird byteweise unverändert übernommen. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den davon abhängigen Formeln, die ihre neuen Werte erhalten. Wenn Sie keine Zelle geändert haben, bleibt die Datei unangetastet.

Hat ein anderes Programm die Datei geändert, nachdem Sie sie geöffnet haben, überschreibt JeffCrunch sie nicht ohne Rückfrage, da sonst die Änderungen des anderen Programms verloren gingen. Sie können wählen zwischen **Kopie speichern**, **Überschreiben** und **Abbrechen**.

Die neue Datei wird zunächst neben die alte geschrieben und ersetzt diese erst, wenn sie vollständig ist. Tritt ein Fehler auf, erhalten Sie eine Meldung, und die vorhandene Datei bleibt unverändert.

Werden die Formeln beim Speichern noch berechnet, wartet JeffCrunch, bis die Berechnung abgeschlossen ist, damit die Datei die Ergebnisse enthält. Wenn Sie währenddessen **Esc** drücken, um die Berechnung anzuhalten, wird das Speichern abgebrochen, und die Statusleiste zeigt dies an; speichern Sie dann erneut, wann Sie möchten. Eines lässt JeffCrunch in der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Das nächste Programm, das die Arbeitsmappe öffnet, erstellt sie neu. Konnte JeffCrunch eine Formel nicht berechnen oder haben Sie die Berechnung angehalten, fordert die Datei dieses Programm auf, beim Öffnen alle Formeln neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie sie **Speichern**, **Nicht speichern** oder mit **Abbrechen** weiterarbeiten möchten. Ein Sternchen (*) im Fenstertitel zeigt an, dass nicht gespeicherte Änderungen vorhanden sind.
