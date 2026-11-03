# Arbeitsmappen öffnen und speichern
<!-- keywords: Kopie, älteres Format, OpenDocument, Kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, getrennt, Trennzeichen, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, Drag & Drop, laden, groß, umfangreich, Makros, nicht gespeichert, Änderungen, schließen, Berechnungskette, Vorlage, xltx, xltm, lässt sich nicht öffnen, beschädigt, verschlüsselt, Kennwort, Passwort -->

Wenn JeffCrunch ohne zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 usw.), und eine Nummer wird nicht erneut vergeben, nachdem ihr Fenster geschlossen wurde. Um eine weitere zu beginnen, drücken Sie **{key:New}**, oder klicken Sie auf **Datei**, dann auf **Neu**, und wählen Sie **Leere Arbeitsmappe**; sie wird in einem eigenen Fenster geöffnet. Eine neue Arbeitsmappe, die Sie nicht geändert haben, macht Platz, wenn Sie in ihrem Fenster eine Datei öffnen; eine geänderte bleibt erhalten, und die Datei wird in einem anderen Fenster geöffnet. **Speichern** fragt bei einer neuen Arbeitsmappe wie **Speichern unter** nach einem Namen und speichert sie als .xlsx-Datei. Beim Schließen einer neuen, nicht geänderten Arbeitsmappe erfolgt keine Rückfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Open}**, oder klicken Sie auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus dem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Stattdessen wird dieses Fenster in den Vordergrund geholt.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (solche mit Makros) sowie die Vorlagen **.xltx** und **.xltm** (eine Vorlage mit Makros). Makros werden nicht ausgeführt, aber unverändert beibehalten: Das Makroprojekt, seine Signatur und die Teile, die darauf verweisen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, da die Art Teil der Datei ist.

## Andere Dateiarten

JeffCrunch öffnet auch ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und das flache **.fods**) sowie Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine aus einer solchen Datei geöffnete Arbeitsmappe wird als neue **.xlsx**-Datei gespeichert (oder als **.xls**- oder **.ods**-Kopie, siehe unten): Wenn Sie **{key:Save}** drücken, weist JeffCrunch Sie darauf hin und fragt nach einem Namen; die geöffnete Datei wird nie geändert.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den in der Datei gespeicherten Ergebnissen), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Fensterbereichen und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen; eine Meldung oben gibt an, welche davon die Datei enthält, und eine von hier gespeicherte Kopie enthält sie nicht. Bei einer kennwortgeschützten Tabelle werden Sie beim Öffnen nach dem Kennwort gefragt: siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei ergibt ein Blatt. JeffCrunch erkennt, wie die Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche) und wie die Zeichen codiert sind (UTF-8 oder UTF-16, andernfalls die Codepage Ihrer Sprache), und liest jeden Wert so, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5, ein Prozentwert, ein Geldbetrag, ein Datum mit Jahr (z. B. 15.03.2024, oder 3/15/2024 im Englischen), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache und auf Englisch, also auch TRUE und FALSE). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen mit führender Null (z. B. 007) und Zahlen mit mehr als 15 Stellen bleiben Text, damit eine Postleitzahl oder Kontonummer nicht verändert wird. Das Gleiche gilt für Tag und Monat ohne Jahr, in beliebiger Reihenfolge und Sprache (z. B. 3/4, 12.5, 1-2, 1. März oder Dec 25), da das Jahr erfunden werden müsste, sowie für Texte wie den Gennamen MARCH1. Ein Blatt umfasst 1.048.576 Zeilen und 16.384 Spalten; enthält die Datei mehr, öffnet JeffCrunch, was passt, und weist Sie darauf hin.

Wenn Sie eine Textdatei öffnen, zeigt ein Fenster zunächst die ersten Zeilen an und fragt, wie sie gelesen werden soll: wodurch die Felder getrennt sind (beliebige Kombination aus Komma, Semikolon, Tabstopp, Leerzeichen, senkrechtem Strich oder einem selbst eingegebenen Zeichen, wobei aufeinanderfolgende Trennzeichen auf Wunsch als eines gelten), womit ein Feld eingeschlossen wird, das ein Trennzeichen enthält, die Textcodierung (Unicode, die Windows-Codepages, die ISO-8859-Codepages, Mac Roman und die ostasiatischen Codierungen) und die Zeile, ab der importiert wird. Über jeder Spalte wählen Sie, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu lassen), ein **Datum** in der angegebenen Reihenfolge oder **Überspringen**. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, geöffnet wird jedoch die ganze Datei. **Datei > Öffnen > Textdatei öffnen** fragt auch dann nach, wenn der Dateiname ungewöhnlich ist.

Um nur die Werte des angezeigten Blatts zu behalten, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie den Dateityp **.csv** (oder **.tsv** und **.txt**, tabstoppgetrennt). Bevor die Datei geschrieben wird, teilt JeffCrunch mit, was Ihre Arbeitsmappe enthält, das die Datei nicht aufnehmen kann (z. B. die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros), fragt, wie sie geschrieben werden soll, und ob Sie **Nur Werte speichern**, stattdessen **Als .xlsx speichern…** und alles behalten oder **Abbrechen** möchten:

- **Trennzeichen.** Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein selbst eingegebenes Zeichen. Eine **.csv**-Datei verwendet ein Semikolon, wenn Ihre Sprache in Zahlen ein Komma schreibt (wie im Deutschen), sonst ein Komma; eine **.tsv**- oder **.txt**-Datei einen Tabstopp.
- **Textcodierung.** UTF-8 mit BOM (damit andere Programme Umlaute und Akzente richtig erkennen), UTF-8 ohne BOM, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die ISO-8859-Codepages oder Mac Roman. Ein Zeichen, für das die Codepage kein Byte hat, wird als Fragezeichen geschrieben, und die Statusleiste gibt an, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder altes Mac-Format (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt alle Ziffern einer Zahl (0,1 statt 10 %) und ein Datum als ISO-Text, z. B. 2026-03-04.

Text wird in Anführungszeichen gesetzt, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die anderen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in der Datei enthalten, und Ihre Arbeitsmappe wird durch das Schreiben nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form der ursprünglichen Datei zurückgeschrieben: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Wenn Sie sie als Kopie speichern, ohne eine Zelle zu ändern, ist die Kopie Byte für Byte identisch, und es erfolgt keine Rückfrage.

## Eine Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme für OpenDocument öffnen können, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie den Dateityp **.xls** oder **.ods**. Die Kopie wird aus dem aktuellen Stand Ihrer Arbeitsmappe einschließlich der noch nicht gespeicherten Änderungen erstellt und enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit ihren aktuellen Ergebnissen (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, die Ausrichtung und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Zeilen- und Spaltengliederungen sowie fixierte Fensterbereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und der Angabe, welche ausgeblendet sind, sowie die definierten Namen (und die Druckbereiche);
- die Links und die Druckeinstellungen jedes Blatts (Papier, Seitenränder, Ausrichtung, Skalierung, Kopf- und Fußzeilen und Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe bei. Eine Kopie kann nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, teilt es mit, was Ihre Arbeitsmappe enthält, das die Datei nicht aufnehmen kann: Diagramme, Bilder und andere Zeichnungen, Notizen, Pivot-Tabellen, Tabellen, bedingte Formatierungen, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Links zu anderen Arbeitsmappen (Formeln, die sie verwenden, behalten ihre bisherigen Werte). Eine Formel, die das ältere Format nicht ausdrücken kann (eine dort fehlende Funktion oder eine Zelle jenseits von Spalte IV oder Zeile 65.536), wird als ihr Ergebnis geschrieben, und die Meldung zählt diese Fälle. Das ältere Format ist außerdem auf 65.536 Zeilen, 256 Spalten, 255 Blätter und 32.767 Zeichen pro Zelle sowie auf etwa 4.000 Zellformate und 56 Farben beschränkt: Was nicht passt, wird abgeschnitten, und die Meldung gibt an, wie viel. Anschließend können Sie **Kopie speichern**, **Als .xlsx speichern…** und alles behalten oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die geöffnete Datei wird nie überschrieben: Geben Sie der Kopie einen anderen Namen. Eine geöffnete **.xls**- oder **.ods**-Datei kann als Kopie im jeweils anderen Format gespeichert werden.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern geöffnet war, erscheinen fast sofort, auch bei sehr großen Arbeitsmappen. Die Statusleiste unten links zeigt an, was gerade geschieht: **Öffnen**, **Lesen** eines Blatts mit der Anzahl der bisher gelesenen Zellen, **Berechnung läuft…** mit einer Prozentangabe und dann **Bereit**.

Während des Lesens können Sie scrollen, markieren und kopieren. Die Registerkarten der anderen Blätter werden aktiviert, sobald diese gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie gelesen werden kann, und eine Meldung gibt an, was ausgelassen wurde.

## Wenn sich eine Datei nicht öffnen lässt

Wenn JeffCrunch eine Datei nicht öffnen kann, nennt es den Grund in verständlichen Worten und lässt Ihre geöffneten Arbeitsmappen unverändert. Es unterscheidet zwischen einer leeren Datei und einer Datei, die gar keine Arbeitsmappe ist (z. B. ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); einem abgeschnittenen oder beschädigten ZIP-Paket, was meist bedeutet, dass ein Download oder Kopiervorgang unterbrochen wurde – kopieren Sie die Datei dann erneut vom Original; einer kennwortgeschützten Arbeitsmappe, für die nach dem Kennwort gefragt wird (siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password)); und einer Verbunddatei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Menü „Datei“ listet die zuletzt geöffneten Arbeitsmappen auf. Klicken Sie mit der rechten Maustaste auf einen Eintrag, um ihn oben in der Liste **anzuheften** oder aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Save}**, oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei zu erstellen, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine Vorlage als **.xltx**- oder **.xltm**-Datei und jede andere als **.xlsx**-Datei. Ein Name mit der Endung einer anderen Arbeitsmappenart wird nicht akzeptiert, und die Meldung nennt die zu verwendende Endung; Namen mit den Endungen **.xls**, **.ods**, **.csv**, **.tsv** und **.txt** erstellen Kopien wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles andere in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird Byte für Byte unverändert übernommen. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den davon abhängigen Formeln, die ihre neuen Werte erhalten. Wenn Sie keine Zelle geändert haben, bleibt die Datei unangetastet.

Wenn ein anderes Programm die Datei geändert hat, nachdem Sie sie geöffnet haben, überschreibt JeffCrunch sie nicht ohne Rückfrage, da sonst die Änderungen des anderen Programms verloren gingen. Es fragt, ob Sie **Kopie speichern**, die Datei **Überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zuerst neben die alte geschrieben und ersetzt diese erst, wenn sie vollständig ist. Geht dabei etwas schief, erhalten Sie eine Meldung, und die vorhandene Datei bleibt unverändert.

Wenn die Formeln beim Speichern noch berechnet werden, wartet JeffCrunch, bis die Berechnung abgeschlossen ist, damit die Datei die Ergebnisse enthält. Wenn Sie währenddessen **Esc** drücken, um die Berechnung anzuhalten, wird das Speichern abgebrochen, und die Statusleiste weist darauf hin; speichern Sie erneut, wenn Sie möchten. Eines lässt JeffCrunch in der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Das Programm, das die Arbeitsmappe als Nächstes öffnet, erstellt sie neu. Konnte JeffCrunch eine Formel nicht berechnen oder haben Sie die Berechnung angehalten, fordert die Datei dieses Programm auf, beim Öffnen alle Formeln neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie **Speichern**, **Nicht speichern** oder **Abbrechen** und weiterarbeiten möchten. Ein Sternchen (*) im Fenstertitel zeigt an, dass nicht gespeicherte Änderungen vorhanden sind.
