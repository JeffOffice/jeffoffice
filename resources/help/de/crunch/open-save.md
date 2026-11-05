# Arbeitsmappen öffnen und speichern
<!-- keywords: Kopie, älteres Format, OpenDocument, Kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, getrennt, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, laden, groß, umfangreich, Makros, nicht gespeichert, Änderungen, schließen, Kopie, Berechnungskette, Vorlage, xltx, xltm, kann nicht geöffnet werden, beschädigt, verschlüsselt, Kennwort, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ots, reparieren, öffnen und reparieren, integrierte Vorlagen, Budget, Rechnung, Stundenzettel, Kredit, Kalender, Notenbuch, feste Breite, feste Spalten, Dezimaltrennzeichen, Tausendertrennzeichen, nachgestelltes Minus -->

Wenn JeffCrunch ohne eine zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 und so weiter), und eine Nummer wird nach dem Schließen des Fensters nicht wieder vergeben. Um eine weitere zu beginnen, drücken Sie **{key:New}** oder klicken auf **Datei** und dann auf **Neu** und wählen **Leere Arbeitsmappe**; sie öffnet sich in einem eigenen Fenster. Eine neue Arbeitsmappe, die Sie nicht geändert haben, weicht, wenn Sie in ihrem Fenster eine Datei öffnen; eine geänderte bleibt, und die Datei öffnet sich in einem anderen Fenster. **Speichern** fragt bei einer neuen Arbeitsmappe nach einem Namen, ebenso **Speichern unter**, und speichert sie als .xlsx-Datei. Beim Schließen einer neuen, nicht geänderten Arbeitsmappe erscheint keine Abfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Open}** oder klicken auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus Ihrem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Dieses Fenster wird in den Vordergrund gebracht.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (mit Makros) sowie die Vorlagen **.xltx** und **.xltm** (eine Vorlage mit Makros). Makros führt es nicht aus, und es lässt sie unverändert: Das Makroprojekt, seine Signatur und die Teile, die sie benennen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, denn die Art gehört zur Datei.

## Andere Dateiarten

JeffCrunch öffnet außerdem ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und die flache Form **.fods**) und Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine Arbeitsmappe, die aus einer dieser Dateien geöffnet wurde, wird als neue **.xlsx**-Datei gespeichert (oder als **.xls**- oder **.ods**-Kopie, siehe unten): Wenn Sie **{key:Save}** drücken, teilt JeffCrunch Ihnen das mit und fragt nach einem Namen. Die geöffnete Datei wird nie verändert.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den Ergebnissen, die die Datei gespeichert hat), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Bereichen und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen; eine Meldung oben zeigt an, welche davon die Datei enthält, und eine von hier gespeicherte Kopie wird sie nicht enthalten. Eine mit einem Kennwort geschützte Tabelle fragt beim Öffnen danach: siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei ist ein Blatt. JeffCrunch ermittelt, wie ihre Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche), und wie ihre Zeichen codiert sind (UTF-8 oder UTF-16, sonst die Codepage Ihrer Sprache). Es liest jeden Wert so, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 auf Deutsch, einen Prozentwert, einen Geldbetrag, ein Datum mit Jahr (etwa 3/15/2024 oder 15.03.2024 auf Deutsch), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache, also WAHR und FALSCH auf Deutsch, und TRUE und FALSE auf Englisch). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Ziffern bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Ebenso bleiben ein Tag und ein Monat ohne Jahr, in beliebiger Reihenfolge und in jeder Sprache (etwa 3/4, 12.5, 1-2, 1. März oder 25. Dez.), sowie Texte wie das Gen MARCH1 Text, weil das Jahr erfunden werden müsste. Ein Blatt hat 1.048.576 Zeilen und 16.384 Spalten; enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und teilt es Ihnen mit.

Beim Öffnen einer Textdatei zeigt zuerst ein Fenster ihre ersten Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben; mehrere aufeinanderfolgende Trennzeichen können Sie als eines zählen lassen), welches Zeichen ein Feld einschließt, das ein Trennzeichen enthält, die Textcodierung (Unicode, die Windows-Codepages, die ISO-8859-Seiten, Mac Roman und die ostasiatischen Codierungen) und die Zeile, ab der gelesen wird. Über jeder Spalte legen Sie fest, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Artikelnummer wie 007 oder eine lange Kontonummer unverändert zu behalten), ein **Datum** in der von Ihnen gewählten Reihenfolge oder **Auslassen**. Manche Dateien richten ihre Werte in gleich breiten Spalten aus, mit Leerzeichen dazwischen und ganz ohne Trennzeichen. Aktivieren Sie dann **Die Spalten haben eine feste Breite**; die ersten Zeilen der Datei erscheinen in einem Feld. Klicken Sie zwischen zwei Zeichen, um dort eine Spalte beginnen zu lassen, und klicken Sie erneut, um sie wieder zu entfernen; oder tragen Sie die Stellen unter **Spalten beginnen bei** ein, etwa 10, 25, 40. Jedes Stück wird an diesen Stellen abgeschnitten, und die Leerzeichen um den Wert werden entfernt. **Dezimaltrennzeichen** und **Tausendertrennzeichen** legen fest, wie Zahlen in dieser Datei geschrieben sind, unabhängig von Ihrer Sprache: 1.234,50 ist eintausendzweihundertvierunddreißig und ein halb, wenn die Datei ein Komma als Dezimaltrennzeichen verwendet. Aktivieren Sie **Ein Minuszeichen kann nach der Zahl stehen** für Dateien, die -12 als 12- schreiben. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, geöffnet wird aber die ganze Datei. **Datei > Öffnen > Textdatei öffnen** fragt auch dann, wenn der Dateiname ungewöhnlich ist.

Um nur die Werte des gerade angezeigten Blatts zu behalten, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen die Dateiart **.csv** (oder **.tsv** und **.txt**, mit Tabstopps getrennt). Bevor JeffCrunch die Datei schreibt, sagt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann (etwa die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros), und fragt, wie geschrieben werden soll: ob **Nur Werte speichern**, ob stattdessen **Als .xlsx speichern** und alles behalten, oder ob **Abbrechen**:

- **Trennzeichen.** Ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben. Eine **.csv** beginnt mit einem Semikolon, wo Ihre Sprache ein Komma in Zahlen schreibt, sonst mit einem Komma; eine **.tsv** oder **.txt** mit einem Tabstopp.
- **Textcodierung.** UTF-8 mit Markierung (damit andere Programme Akzentbuchstaben richtig übernehmen), UTF-8 ohne Markierung, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die ISO-8859-Seiten, Mac Roman oder die japanische, chinesische, koreanische, hebräische, arabische, baltische, vietnamesische und thailändische Codepage. Ein Zeichen, für das die Codepage kein Byte hat, wird als Fragezeichen geschrieben; die Statusleiste zeigt, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder älteres Mac-Format (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt jede Ziffer einer Zahl (0,1 und nicht 10 %) und ein Datum als ISO-Text wie 2026-03-04.

Text wird in Anführungszeichen gesetzt, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die anderen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in der Datei enthalten, und Ihre Arbeitsmappe wird durch das Schreiben einer solchen Datei nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form zurückgeschrieben, die die Datei hatte: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Haben Sie sie als Kopie gespeichert, ohne eine Zelle zu ändern, ist die Kopie dieselbe Datei, Byte für Byte, und es wird nicht nachgefragt.

## Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme, die OpenDocument lesen, öffnen können, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen die Dateiart **.xls** oder **.ods**. Die Kopie wird aus Ihrer Arbeitsmappe in ihrem jetzigen Zustand erstellt, mit den Änderungen, die Sie noch nicht gespeichert haben, und enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit den Ergebnissen, die sie jetzt haben (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, Ausrichtungen und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Gliederungen von Zeilen und Spalten sowie fixierte Bereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und davon, welche ausgeblendet sind, sowie die Namen, die Sie definiert haben (und die Druckbereiche);
- die Verknüpfungen und wie jedes Blatt gedruckt wird (Papier, Ränder, Ausrichtung, Skalierung, Kopf- und Fußzeilen und Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe. Eine Kopie kann jedoch nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, sagt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern wird: Diagramme, Bilder und andere Zeichnungen, Notizen, Pivot-Tabellen, Tabellen, bedingte Formatierungen, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Verknüpfungen zu anderen Arbeitsmappen (die Formeln, die sie verwenden, behalten die Werte, die sie hatten). Eine Formel, die die ältere Form nicht ausdrücken kann (eine Funktion, die sie nicht kennt, oder eine Zelle hinter Spalte IV oder Zeile 65.536), wird als der Wert geschrieben, auf den sie sich ergab, und die Meldung zählt sie. Die ältere Form endet außerdem bei 65.536 Zeilen, 256 Spalten, 255 Blättern und 32.767 Zeichen in einer Zelle sowie bei etwa 4.000 Zellformaten und 56 Farben: Was nicht hineinpasst, wird abgeschnitten, und die Meldung sagt, wie viel. Anschließend können Sie **Kopie speichern**, **Als .xlsx speichern** und alles behalten oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die Datei, die Sie geöffnet haben, wird nie geschrieben: Geben Sie der Kopie einen anderen Namen. Eine Kopie einer geöffneten **.xls**- oder **.ods**-Datei kann in der jeweils anderen Art gespeichert werden.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffCrunch schreibt die Arbeitsmappe in diese Datei, mit den Änderungen, die Sie noch nicht gespeichert haben, und Sie arbeiten in der bisherigen Arbeitsmappe weiter. Ihr Name und Ihre nicht gespeicherten Änderungen bleiben unverändert. Sie können jede Dateiart wählen, die auch **Speichern unter** anbietet. Eine Kopie wird nie über die eigene Datei der Arbeitsmappe geschrieben.

## Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffCrunch zuerst die alte Datei in eine Datei mit dem Namen „Sicherungskopie von“ plus dem Dateinamen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt das Programm, ob ohne sie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert.

## Als Vorlage speichern

Eine Vorlage ist eine Arbeitsmappe, die Sie als Ausgangspunkt verwenden. Sie kann Formeln, Formatvorlagen und Blätter enthalten. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage (eine **.xltx**-Datei) kommt in **Meine Vorlagen**, Ihre Arbeitsmappe wird dabei nicht verändert. Im Dialog **Speichern unter** können Sie außerdem **Vorlagen (*.xltx)** oder **OpenDocument-Tabellenvorlagen (*.ots)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um eine Arbeitsmappe aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgelistet. Klicken Sie auf eine davon. Eine neue Arbeitsmappe öffnet sich mit den Zellen und Blättern der Vorlage. Sie hat noch keine eigene Datei, daher fragt **Speichern** nach einem Namen, und die Vorlage bleibt unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Auf der Seite **Öffnen** klicken Sie auf **Weitere Möglichkeiten zum Öffnen**. Sie können außerdem mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken.

- **Schreibgeschützt öffnen** öffnet die Arbeitsmappe nur zum Lesen. Sie können ansehen, blättern, suchen und kopieren, aber keine Zelle ändern. Die Titelleiste zeigt „Schreibgeschützt“. Drücken Sie **Speichern**, fragt JeffCrunch nach einem neuen Namen, damit die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Arbeitsmappe ohne eigene Datei. **Speichern** fragt, wohin sie soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Arbeitsmappe in einem eigenen Fenster, auch wenn das Fenster, in dem Sie sich befinden, leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .xlsx- oder .xls-Datei. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .xlsx-Datei beschädigt, etwa wenn ein Download auf halbem Weg abbricht oder eine Festplatte Fehler gemacht hat. Eine .xlsx-Datei ist ein Bündel kleinerer Teile, etwa der Blätter, der Formatvorlagen und des gemeinsamen Texts. **Öffnen und reparieren** liest jeden Teil, den es lesen kann. Ein Teil, der mitten im Vorgang abbricht, wird bis zu der Stelle übernommen, an der er abbricht; so erhalten Sie möglicherweise nur die ersten Zeilen eines Blatts. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, stellt JeffCrunch sie neu her.

Die reparierte Arbeitsmappe öffnet sich ohne eigene Datei, mit einem Namen wie „Repariertes Budget“. Eine Meldung teilt Ihnen mit, was bis zu einem Bruch übernommen und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Arbeitsmappe unter einem neuen Namen, und prüfen Sie die Zahlen, bevor Sie ihnen vertrauen.

### Eine .xls-Datei

Auch eine .xls-Datei kann beschädigt sein. Ist ihre äußere Hülle abgeschnitten, sucht **Öffnen und reparieren** in der Datei nach dem Anfang der Arbeitsmappe und liest ab dort. Ein Blatt, das abgeschnitten wurde, behält die Zellen vor dem Schnitt. Was nach dem Schnitt kam, geht verloren, und Sie werden darüber informiert. Die reparierte Arbeitsmappe öffnet sich als Kopie; speichern Sie sie unter einem neuen Namen.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern der Arbeitsmappe geöffnet war, erscheinen fast sofort, auch wenn die Arbeitsmappe sehr groß ist. Die Statusleiste unten links zeigt, was gerade geschieht: **Öffnen**, **Lesen** eines Blatts mit der Anzahl der bisher gelesenen Zellen, **Berechnen** mit einem Prozentwert und danach **Bereit**.

Sie können während des Lesens blättern, auswählen und kopieren. Die Registerkarten der anderen Blätter werden aktiviert, sobald sie gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie lesbar ist, und eine Meldung sagt, was weggelassen wurde.

## Wenn sich eine Datei nicht öffnen lässt

Kann JeffCrunch eine Datei nicht öffnen, nennt es den Grund in verständlichen Worten und lässt die geöffneten Arbeitsmappen, wie sie sind. Es unterscheidet, ob eine Datei leer ist oder gar keine Arbeitsmappe darstellt (etwa ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); ein ZIP-Paket, das abgeschnitten oder beschädigt ist, was meist bedeutet, dass ein Download oder Kopiervorgang unterbrochen wurde, sodass Sie es vom Original erneut kopieren sollten; eine Arbeitsmappe, die mit einem Kennwort geschützt ist, wonach das Programm fragt (siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password)); und eine zusammengesetzte Datei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Dateibereich listet die Arbeitsmappen auf, die Sie zuletzt geöffnet haben. Klicken Sie eine mit der rechten Maustaste an, um sie **oben in der Liste anzuheften** oder aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Save}** oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei zu erstellen, klicken Sie auf **Datei** und dann auf **Speichern unter** und wählen einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine Vorlage als **.xltx**- oder **.xltm**-Datei und jede andere als **.xlsx**-Datei. Ein Name, der auf eine andere Art von Arbeitsmappe endet, wird nicht akzeptiert; die Meldung nennt die passende Endung. Die Namen mit den Endungen **.xls**, **.ods**, **.csv**, **.tsv** und **.txt** erstellen Kopien, wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles Übrige in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird unverändert übernommen, Byte für Byte. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den Formeln, die von ihnen abhängen und deren neue Werte erhalten. Haben Sie keine Zelle geändert, bleibt die Datei unberührt.

Hat ein anderes Programm die Datei geändert, nachdem Sie sie geöffnet haben, speichert JeffCrunch nicht ohne Rückfrage darüber, weil die Änderungen des anderen Programms sonst verloren gingen. Das Programm fragt, ob Sie eine **Kopie speichern**, die Datei **überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zuerst neben der alten geschrieben und ersetzt die alte erst, wenn sie vollständig ist. Tritt ein Fehler auf, sehen Sie eine Meldung, und die vorhandene Datei bleibt, wie sie war.

Werden die Formeln noch berechnet, wenn Sie speichern, wartet JeffCrunch, bis sie fertig sind, damit die Datei ihre Ergebnisse enthält. Drücken Sie in der Zwischenzeit **Esc**, um die Berechnung abzubrechen, wird das Speichern abgebrochen, und die Statusleiste zeigt dies an; speichern Sie dann erneut, wenn Sie möchten. JeffCrunch lässt eines aus der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Ein Programm, das die Arbeitsmappe danach öffnet, erstellt sie neu. Konnte JeffCrunch einige Formeln nicht berechnen oder haben Sie die Berechnung angehalten, bittet die Datei das öffnende Programm, alle Formeln beim Öffnen der Arbeitsmappe neu zu berechnen.

## Schließen mit Änderungen

Schließen Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen, fragt JeffCrunch, ob Sie **Speichern**, **Nicht speichern** oder **Abbrechen** wählen möchten, um weiterzuarbeiten. Ein Sternchen (*) im Fenstertitel zeigt an, dass es nicht gespeicherte Änderungen gibt.

## Integrierte Vorlagen

Unter **Datei > Neu** finden Sie unter der leeren Arbeitsmappe fertige Arbeitsmappen in Gruppen: **Finanzen** (ein Monatsbudget, ein Kredittilgungsplan und ein Scheckbuch-Register), **Geschäft** (eine Rechnung, eine Spesenabrechnung, ein Stundenzettel, eine Bestandsliste, ein Fahrtenbuch und ein Verkaufsbericht), **Planung** (ein Kalender und ein Projektplan) und **Bildung** (ein Notenbuch). Jede zeigt ein kleines Bild ihres Kopfbereichs. Klicken Sie auf eine, um eine neue Arbeitsmappe daraus zu beginnen. Tragen Sie Ihre Angaben in die gelben Felder ein. Die übrigen Zahlen werden für Sie berechnet. Die Arbeitsmappe hat erst dann eine eigene Datei, wenn Sie sie speichern.
