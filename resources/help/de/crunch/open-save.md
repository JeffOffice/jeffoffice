# Arbeitsmappen öffnen und speichern

<!-- keywords: kopieren, älteres Format, OpenDocument, Kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, Trennzeichen, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, laden, groß, umfangreich, Makros, nicht gespeichert, Änderungen, schließen, Kopie, Berechnungskette, Vorlage, xltx, xltm, kann nicht geöffnet werden, beschädigt, verschlüsselt, Kennwort, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ots, reparieren, öffnen und reparieren -->

Wenn JeffCrunch ohne eine zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 und so weiter). Eine Nummer wird nach dem Schließen des Fensters nicht wieder verwendet. Um eine weitere Arbeitsmappe zu beginnen, drücken Sie **{key:Neu}** oder klicken auf **Datei** und dann auf **Neu**, und wählen Sie **Leere Arbeitsmappe**. Sie wird in einem eigenen Fenster geöffnet. Eine neue Arbeitsmappe, die Sie nicht geändert haben, wird ersetzt, wenn Sie in ihrem Fenster eine Datei öffnen. Eine Arbeitsmappe, die Sie geändert haben, bleibt erhalten, und die Datei wird in einem anderen Fenster geöffnet. **Speichern** fragt bei einer neuen Arbeitsmappe nach einem Namen, ebenso wie **Speichern unter**, und speichert sie als .xlsx-Datei. Beim Schließen einer neuen Arbeitsmappe, die Sie nicht geändert haben, erfolgt keine Rückfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Öffnen}** oder klicken auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus dem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Dieses Fenster wird stattdessen in den Vordergrund gebracht.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (die Makros enthalten) sowie die Vorlagen **.xltx** und **.xltm** (eine Vorlage mit Makros). Makros werden nicht ausgeführt, und JeffCrunch lässt sie unverändert: Das Makroprojekt, seine Signatur und die Teile, die sich auf sie beziehen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, denn die Art ist Teil der Datei.

## Andere Dateiarten

JeffCrunch öffnet außerdem ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und das flache **.fods**) sowie Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine Arbeitsmappe, die aus einer dieser Dateien geöffnet wurde, wird als neue **.xlsx**-Datei gespeichert (oder als Kopie im Format **.xls** oder **.ods**, siehe unten). Wenn Sie **{key:Speichern}** drücken, teilt JeffCrunch Ihnen dies mit und fragt nach einem Namen. Die geöffnete Datei wird niemals verändert.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den Ergebnissen, die die Datei gespeichert hat), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Bereichen und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen. Eine Meldung oben im Fenster zeigt an, welche davon die Datei enthält, und eine von hier gespeicherte Kopie wird sie nicht enthalten. Eine Tabelle, die mit einem Kennwort geschützt ist, fragt beim Öffnen danach: siehe [Passwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei besteht aus einem Blatt. JeffCrunch ermittelt, wie ihre Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche), und wie ihre Zeichen codiert sind (UTF-8 oder UTF-16 oder sonst die Codepage Ihrer Sprache). Jeden Wert liest es so, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 in Deutsch, einen Prozentwert, einen Geldbetrag, ein Datum mit Jahr (etwa 3/15/2024 oder 15.03.2024 auf Deutsch), eine Uhrzeit, WAHR oder FALSCH (in Deutsch WAHR und FALSCH, in Englisch TRUE und FALSE). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Ziffern bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Ebenso bleiben Tag und Monat ohne Jahr, in beliebiger Reihenfolge und Sprache (etwa 3/4, 12.5, 1-2, 1. März oder 25. Dez.), Text, denn das Jahr müsste erfunden werden, und ebenso Texte wie das Gen MARCH1. Ein Blatt umfasst 1.048.576 Zeilen und 16.384 Spalten. Enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und teilt es Ihnen mit.

Beim Öffnen einer Textdatei zeigt zuerst ein Fenster die ersten Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben; mehrere Trennzeichen hintereinander können Sie als eines zählen lassen), was ein Feld mit einem Trennzeichen einschließt, die Textcodierung (Unicode, die Windows-Codepages, die Seiten ISO 8859, Mac Roman und die ostasiatischen) und die Zeile, ab der gelesen wird. Über jeder Spalte wählen Sie, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu behalten), ein **Datum** in der von Ihnen angegebenen Reihenfolge oder **Auslassen**. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, die gesamte Datei wird jedoch geöffnet. **Datei > Öffnen > Textdatei öffnen** fragt auch dann, wenn der Dateiname ungewöhnlich ist.

Um nur die Werte des gerade angezeigten Blatts zu speichern, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie den Dateityp **.csv** (oder **.tsv** und **.txt** für tabstoppgetrennte Dateien). Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann (etwa die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros), und fragt, wie geschrieben werden soll: mit **Nur Werte speichern**, mit **Als .xlsx speichern**, um alles beizubehalten, oder mit **Abbrechen**.

- **Trennzeichen.** Ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben. Eine **.csv**-Datei beginnt mit einem Semikolon, wo Ihre Sprache in Zahlen ein Komma schreibt, sonst mit einem Komma. Eine **.tsv**- oder **.txt**-Datei verwendet einen Tabstopp.
- **Textcodierung.** UTF-8 mit Byte-Order-Mark (damit andere Programme Akzentbuchstaben richtig übernehmen), UTF-8 ohne sie, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die Seiten ISO 8859 oder Mac Roman. Ein Zeichen, für das die Codepage kein Byte hat, wird als Fragezeichen geschrieben, und die Statusleiste zeigt an, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder altes Mac-Format (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt jede Ziffer einer Zahl (0,1 und nicht 10 %) und ein Datum als ISO-Text wie 2026-03-04.

Text wird in Anführungszeichen gesetzt, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die übrigen Blätter, die Formeln, die Formate und die Einstellungen sind in der Datei nicht enthalten. Ihre Arbeitsmappe wird durch das Schreiben einer Textdatei nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form zurückgeschrieben, die die Datei hatte: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Wenn Sie sie ohne Änderung an einer Zelle als Kopie speichern, ist die Kopie dieselbe Datei, Byte für Byte, und es erfolgt keine Rückfrage.

## Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme für OpenDocument öffnen können, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie den Dateityp **.xls** oder **.ods**. Die Kopie entsteht aus Ihrer Arbeitsmappe in ihrem jetzigen Zustand, einschließlich der noch nicht gespeicherten Änderungen, und enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit den Ergebnissen, die sie jetzt haben (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, Ausrichtungen und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Gliederungen von Zeilen und Spalten sowie fixierte Bereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und der Angabe, welche ausgeblendet sind, sowie die definierten Namen (und die Druckbereiche);
- die Verknüpfungen und die Druckeinstellungen jedes Blatts (Papier, Ränder, Ausrichtung, Skalierung, Kopf- und Fußzeilen sowie Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe. Eine Kopie kann aber nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann: Diagramme, Bilder und andere Zeichnungen, Notizen, Pivot-Tabellen, Tabellen, bedingte Formatierungen, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Verknüpfungen zu anderen Arbeitsmappen (Formeln, die sie verwenden, behalten die Werte, die sie hatten). Eine Formel, die das ältere Format nicht darstellen kann (etwa eine Funktion, die es nicht kennt, oder eine Zelle hinter Spalte IV oder Zeile 65.536), wird als der Wert geschrieben, den sie ergab, und die Meldung zählt sie. Das ältere Format endet außerdem bei 65.536 Zeilen, 256 Spalten, 255 Blättern und 32.767 Zeichen in einer Zelle sowie bei etwa 4.000 Zellformaten und 56 Farben. Was nicht hineinpasst, wird abgeschnitten, und die Meldung nennt, wie viel. Anschließend können Sie **Kopie speichern**, **Als .xlsx speichern**, um alles beizubehalten, oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die Datei, die Sie geöffnet haben, wird nie geschrieben: Geben Sie der Kopie einen anderen Namen. Eine Kopie einer geöffneten **.xls**- oder **.ods**-Datei kann in der jeweils anderen Art gespeichert werden.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffCrunch schreibt die Arbeitsmappe in diese Datei, einschließlich der noch nicht gespeicherten Änderungen, und Sie arbeiten in der bisherigen Arbeitsmappe weiter. Ihr Name und ihre nicht gespeicherten Änderungen bleiben unverändert. Sie können jede Dateiart wählen, die **Speichern unter** anbietet. Eine Kopie wird niemals über die eigene Datei der Arbeitsmappe geschrieben.

## Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffCrunch zuerst die alte Datei in eine Datei namens „Sicherung von“ plus Dateiname, im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne sie gespeichert werden soll. Die Antwort ist Nein, außer Sie sagen Ja, und die Datei bleibt unverändert.

## Als Vorlage speichern

Eine Vorlage ist eine Arbeitsmappe, die Sie als Ausgangspunkt verwenden. Sie kann Formeln, Formatvorlagen und Blätter enthalten. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage (eine **.xltx**-Datei) wird unter **Meine Vorlagen** abgelegt, und Ihre Arbeitsmappe wird nicht verändert. Unter **Speichern unter** können Sie auch **Vorlagen (*.xltx)** oder **OpenDocument-Tabellenvorlagen (*.ots)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um eine Arbeitsmappe aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgeführt. Klicken Sie auf eine davon. Eine neue Arbeitsmappe mit den Zellen und Blättern der Vorlage wird geöffnet. Sie hat noch keine eigene Datei, daher fragt **Speichern** nach einem Namen, und die Vorlage bleibt unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen Sie **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste einen Namen in der Liste der zuletzt verwendeten Dateien anklicken.

- **Schreibgeschützt öffnen** öffnet die Arbeitsmappe nur zum Lesen. Sie können ansehen, blättern, suchen und kopieren, aber keine Zelle ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffCrunch nach einem neuen Namen, damit die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Arbeitsmappe ohne eigene Datei. **Speichern** fragt, wohin sie gelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Arbeitsmappe in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .xlsx-Datei. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .xlsx-Datei beschädigt, etwa wenn ein Download auf halbem Weg abbrach oder ein Datenträger Fehler gemacht hat. Eine .xlsx-Datei ist ein Bündel kleinerer Teile, etwa der Blätter, der Formatvorlagen und des gemeinsamen Textes. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbrach, wird bis zu der Stelle übernommen, an der er abbrach. So erhalten Sie möglicherweise nur die ersten Zeilen eines Blatts. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, erstellt JeffCrunch sie neu.

Die reparierte Arbeitsmappe wird ohne eigene Datei geöffnet und erhält einen Namen wie „Reparierte Budget“. Eine Meldung teilt Ihnen mit, was bis zu einem Bruch übernommen und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Arbeitsmappe unter einem neuen Namen, und prüfen Sie die Zahlen, bevor Sie ihnen vertrauen.

## Umfangreiche Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern der Arbeitsmappe geöffnet war, erscheinen fast sofort, auch wenn die Arbeitsmappe sehr groß ist. Die Statusleiste unten links zeigt an, was gerade geschieht: **Öffnen**, **Lesen** eines Blatts mit der Anzahl der bisher gelesenen Zellen, **Berechnen** mit einem Prozentwert und danach **Bereit**.

Während des Lesens können Sie blättern, markieren und kopieren. Die Registerkarten der anderen Blätter werden aktiv, sobald sie gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie sich lesen lässt, und eine Meldung nennt, was weggelassen wurde.

## Wenn eine Datei nicht geöffnet wird

Kann JeffCrunch eine Datei nicht öffnen, nennt es den Grund in einfachen Worten und lässt die geöffneten Arbeitsmappen unverändert. Das Programm unterscheidet, ob eine Datei leer ist oder keine Arbeitsmappe darstellt (etwa ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); ein ZIP-Paket, das abgeschnitten oder beschädigt ist, was meist bedeutet, dass ein Download oder Kopiervorgang unterbrochen wurde, sodass Sie die Datei erneut vom Original kopieren sollten; eine Arbeitsmappe, die mit einem Kennwort geschützt ist und nach dem Kennwort fragt (siehe [Passwortgeschützte Arbeitsmappen öffnen](open-password)); und eine zusammengesetzte Datei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Dateibereich listet die Arbeitsmappen auf, die Sie zuletzt geöffnet haben. Klicken Sie mit der rechten Maustaste auf eine davon, um sie **anzuheften**, sodass sie oben in der Liste bleibt, oder um sie aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Speichern}** oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu sichern. Um das Original zu behalten und eine neue Datei anzulegen, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine Vorlage als **.xltx**- oder **.xltm**-Datei und jede andere als **.xlsx**-Datei. Ein Name mit einer anderen Art von Arbeitsmappe wird nicht akzeptiert; die Meldung nennt, welche Endung Sie verwenden sollten. Die Endungen **.xls**, **.ods**, **.csv**, **.tsv** und **.txt** erzeugen Kopien, wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles Übrige in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird unverändert übernommen, Byte für Byte. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den Formeln, die von ihnen abhängen und deshalb ihre neuen Werte erhalten. Haben Sie keine Zelle geändert, bleibt die Datei unberührt.

Hat ein anderes Programm die Datei nach dem Öffnen geändert, speichert JeffCrunch nicht ohne Rückfrage darüber, weil sonst die Änderungen des anderen Programms verloren gingen. Das Programm fragt, ob Sie **Kopie speichern**, die Datei **überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zuerst neben der alten geschrieben und ersetzt die alte erst, wenn sie vollständig ist. Tritt ein Fehler auf, sehen Sie eine Meldung, und die vorhandene Datei bleibt unverändert.

Werden die Formeln beim Speichern noch berechnet, wartet JeffCrunch, bis sie fertig sind, damit die Datei ihre Ergebnisse enthält. Drücken Sie in der Zwischenzeit **Esc**, um die Berechnung anzuhalten, wird das Speichern abgebrochen, und die Statusleiste meldet dies. Speichern Sie dann erneut, wenn Sie so weit sind. JeffCrunch lässt eines nicht in der Datei: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Ein Programm, das die Arbeitsmappe danach öffnet, erstellt sie neu. Konnte JeffCrunch manche Formeln nicht berechnen oder haben Sie die Berechnung angehalten, bittet die Datei das andere Programm, beim Öffnen der Arbeitsmappe alle Formeln neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie diese **speichern**, **nicht speichern** oder **Abbrechen** möchten, um weiterzuarbeiten. Ein Sternchen (*) in der Titelleiste des Fensters zeigt an, dass es nicht gespeicherte Änderungen gibt.
