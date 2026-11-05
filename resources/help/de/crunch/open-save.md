# Arbeitsmappen öffnen und speichern
<!-- keywords: kopieren, älteres Format, OpenDocument, Kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, getrennt, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, laden, groß, umfangreich, Makros, nicht gespeichert, Änderungen, schließen, Kopie, Berechnungskette, Vorlage, xltx, xltm, kann nicht geöffnet werden, beschädigt, verschlüsselt, Kennwort, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ots, reparieren, öffnen und reparieren, integrierte Vorlagen, Haushaltsplan, Rechnung, Stundenzettel, Darlehen, Kalender, Notenbuch -->

Wenn JeffCrunch ohne eine zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 und so weiter), und eine Nummer wird nicht wieder verwendet, nachdem ihr Fenster geschlossen wurde. Um eine weitere zu beginnen, drücken Sie **{key:Neu}** oder klicken auf **Datei** und dann auf **Neu** und wählen **Leere Arbeitsmappe**; sie öffnet sich in einem eigenen Fenster. Eine neue Arbeitsmappe, die Sie nicht geändert haben, weicht, wenn Sie in ihrem Fenster eine Datei öffnen; eine geänderte bleibt bestehen, und die Datei wird in einem anderen Fenster geöffnet. **Speichern** fragt bei einer neuen Arbeitsmappe nach einem Namen, ebenso wie **Speichern unter**, und speichert sie als .xlsx-Datei. Beim Schließen einer neuen Arbeitsmappe, die Sie nicht geändert haben, erfolgt keine Nachfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Öffnen}** oder klicken auf **Datei** und dann auf **Öffnen** und wählen **Durchsuchen**. Sie können eine Arbeitsmappe auch aus dem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht noch einmal geöffnet: Dieses Fenster wird in den Vordergrund gebracht.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (die Makros enthalten) sowie die Vorlagen **.xltx** und **.xltm** (eine Vorlage mit Makros). Makros werden nicht ausgeführt, und sie bleiben unverändert: Das Makroprojekt, seine Signatur und die Teile, die sie benennen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, weil die Art zur Datei gehört.

## Andere Dateiarten

JeffCrunch öffnet auch ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und die flache Form **.fods**) sowie Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine Arbeitsmappe, die aus einer dieser Dateien geöffnet wurde, wird als neue **.xlsx**-Datei gespeichert (oder als Kopie im Format **.xls** oder **.ods**, siehe unten): Wenn Sie **{key:Speichern}** drücken, weist JeffCrunch darauf hin und fragt nach einem Namen. Die geöffnete Datei wird nie geändert.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den Ergebnissen, die die Datei gespeichert hat), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Bereichen und Namen eingelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht eingelesen; eine Meldung oben zeigt an, welche davon die Datei enthält, und eine von hier gespeicherte Kopie enthält sie nicht. Eine Tabelle, die mit einem Kennwort geschützt ist, fragt beim Öffnen danach: siehe [Passwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei ist ein Blatt. JeffCrunch ermittelt, wie ihre Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche), und wie ihre Zeichen codiert sind (UTF-8 oder UTF-16, sonst die Codepage Ihrer Sprache). Jeder Wert wird so gelesen, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 in Deutsch, ein Prozentwert, ein Geldbetrag, ein Datum mit Jahr (etwa 3/15/2024 oder 15.03.2024 in deutscher Schreibweise), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache, also WAHR und FALSCH in Deutsch, und TRUE und FALSE in Englisch). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Stellen bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Ebenso bleiben ein Tag und ein Monat ohne Jahr, in beliebiger Reihenfolge und in jeder Sprache (etwa 3/4, 12.5, 1-2, 1. März oder 25. Dez.), Text, weil das Jahr erfunden werden müsste, sowie Texte wie das Gen MARCH1. Ein Blatt hat 1.048.576 Zeilen und 16.384 Spalten; enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und teilt Ihnen das mit.

Beim Öffnen einer Textdatei zeigt zuerst ein Fenster ihre ersten Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben; mehrere aufeinanderfolgende Trennzeichen können Sie als eines zählen lassen), was ein Feld umschließt, das ein Trennzeichen enthält, die Textcodierung (Unicode, die Windows-Codepages, die Seiten ISO 8859, Mac Roman und die ostasiatischen) und die Zeile, in der begonnen wird. Über jeder Spalte wählen Sie, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu behalten), ein **Datum** in der von Ihnen angegebenen Reihenfolge oder **Auslassen**. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, aber die ganze Datei wird geöffnet. **Datei > Öffnen > Textdatei öffnen** fragt auch dann, wenn der Dateiname ungewöhnlich ist.

Um nur die Werte des Blatts zu speichern, das Sie gerade betrachten, klicken Sie auf **Datei** und dann auf **Speichern unter** und wählen den Dateityp **.csv** (oder **.tsv** und **.txt** für tabstoppgetrennte Werte). Bevor die Datei geschrieben wird, sagt JeffCrunch, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann (etwa die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros), und fragt, wie geschrieben werden soll: mit **Nur Werte speichern**, mit **Als .xlsx speichern** statt dessen, wodurch alles erhalten bleibt, oder mit **Abbrechen**:

- **Trennzeichen.** Ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben. Eine **.csv**-Datei beginnt mit einem Semikolon, wo Ihre Sprache in Zahlen ein Komma schreibt, sonst mit einem Komma; eine **.tsv**- oder **.txt**-Datei mit einem Tabstopp.
- **Textcodierung.** UTF-8 mit Markierung (damit andere Programme Akzentbuchstaben richtig übernehmen), UTF-8 ohne Markierung, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die Seiten ISO 8859, Mac Roman oder die japanischen, chinesischen, koreanischen, hebräischen, arabischen, baltischen, vietnamesischen und thailändischen Seiten. Ein Zeichen, für das die Codepage kein Byte hat, wird als Fragezeichen geschrieben, und die Statusleiste zeigt an, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder älteres Mac-Format (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt jede Stelle einer Zahl (0,1 und nicht 10 %) und ein Datum als ISO-Text wie 2026-03-04.

Text wird in Anführungszeichen gesetzt, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die anderen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in der Datei enthalten, und Ihre Arbeitsmappe wird durch das Schreiben einer solchen Datei nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form zurückgeschrieben, die die Datei hatte: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Wenn Sie sie als Kopie speichern, ohne eine Zelle zu ändern, ist die Kopie dieselbe Datei, Byte für Byte, und es erfolgt keine Nachfrage.

## Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme für OpenDocument öffnen können, klicken Sie auf **Datei** und dann auf **Speichern unter** und wählen den Dateityp **.xls** oder **.ods**. Die Kopie wird aus Ihrer Arbeitsmappe im aktuellen Zustand erstellt, mit den Änderungen, die Sie noch nicht gespeichert haben, und enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit den Ergebnissen, die sie jetzt haben (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, Ausrichtungen und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Gliederungen von Zeilen und Spalten sowie fixierte Bereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und welche ausgeblendet sind, die Namen, die Sie definiert haben (und die Druckbereiche);
- die Verknüpfungen und wie jedes Blatt gedruckt wird (Papier, Ränder, Ausrichtung, Skalierung, Kopf- und Fußzeilen und Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe. Eine Kopie kann aber nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, sagt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann: Diagramme, Bilder und andere Zeichnungen, Notizen, Pivot-Tabellen, Tabellen, bedingte Formatierungen, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Verknüpfungen zu anderen Arbeitsmappen (die Formeln, die sie verwenden, behalten die Werte, die sie hatten). Eine Formel, die die ältere Form nicht ausdrücken kann (eine Funktion, die sie nicht kennt, oder eine Zelle jenseits der Spalte IV oder der Zeile 65.536), wird als der Wert geschrieben, den sie ergab; die Meldung zählt sie. Die ältere Form endet außerdem bei 65.536 Zeilen, 256 Spalten, 255 Blättern und 32.767 Zeichen in einer Zelle sowie bei etwa 4.000 Zellformaten und 56 Farben: Was nicht hineinpasst, wird abgeschnitten, und die Meldung sagt, wie viel. Sie können dann **Kopie speichern**, **Als .xlsx speichern** (wodurch alles erhalten bleibt) oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die Datei, die Sie geöffnet haben, wird nie geschrieben: Geben Sie der Kopie einen anderen Namen. Eine Kopie einer geöffneten **.xls**- oder **.ods**-Datei kann in der jeweils anderen Art gespeichert werden.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffCrunch schreibt die Arbeitsmappe in diese Datei, mit den Änderungen, die Sie noch nicht gespeichert haben, und Sie arbeiten in der Arbeitsmappe weiter, die Sie hatten. Ihr Name und ihre nicht gespeicherten Änderungen bleiben unverändert. Sie können jede Dateiart wählen, die **Speichern unter** anbietet. Eine Kopie wird nie über die eigene Datei der Arbeitsmappe geschrieben.

## Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und schalten Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen** ein. Bei jedem Speichern kopiert JeffCrunch zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ und dem bisherigen Namen im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne sie gespeichert werden soll. Die Antwort lautet Nein, außer Sie sagen Ja, und die Datei bleibt, wie sie war.

## Als Vorlage speichern

Eine Vorlage ist eine Arbeitsmappe, die Sie als Ausgangspunkt verwenden. Sie kann Formeln, Formatvorlagen und Blätter enthalten. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage (eine **.xltx**-Datei) wird unter **Meine Vorlagen** abgelegt, und Ihre Arbeitsmappe wird nicht verändert. Im Dialog **Speichern unter** können Sie auch **Vorlagen (*.xltx)** oder **OpenDocument-Tabellenvorlagen (*.ots)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um eine Arbeitsmappe aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgeführt. Klicken Sie auf eine. Eine neue Arbeitsmappe öffnet sich mit den Zellen und Blättern der Vorlage. Sie hat noch keine eigene Datei, daher fragt **Speichern** nach einem Namen, und die Vorlage wird nicht verändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste auf einen Namen in der Liste Zuletzt verwendet klicken.

- **Schreibgeschützt öffnen** öffnet die Arbeitsmappe nur zum Lesen. Sie können sie betrachten, durchblättern, durchsuchen und kopieren, aber keine Zelle ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie Speichern drücken, fragt JeffCrunch nach einem neuen Namen, damit die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Arbeitsmappe ohne eigene Datei. Speichern fragt, wohin sie gelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Arbeitsmappe in einem eigenen Fenster, auch dann, wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient dazu, eine beschädigte .xlsx- oder .xls-Datei zu öffnen. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .xlsx-Datei beschädigt, etwa wenn ein Download auf halbem Weg abbrach oder ein Datenträger Fehler gemacht hat. Eine .xlsx-Datei ist ein Paket aus kleineren Teilen, etwa den Blättern, den Formatvorlagen und dem gemeinsamen Text. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbrach, wird bis zu der Stelle übernommen, an der er abbrach, sodass Sie unter Umständen nur die ersten Zeilen eines Blatts erhalten. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, stellt JeffCrunch sie wieder her.

Die reparierte Arbeitsmappe wird ohne eigene Datei geöffnet, mit einem Namen wie „Reparierte Haushaltsplanung“. Eine Meldung teilt mit, was bis zu einem Abbruch übernommen und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Arbeitsmappe unter einem neuen Namen, und prüfen Sie die Zahlen, bevor Sie ihnen vertrauen.

### Eine .xls-Datei

Auch eine .xls-Datei kann beschädigt sein. Ist ihre äußere Hülle abgeschnitten, sucht **Öffnen und reparieren** in der Datei nach dem Beginn der Arbeitsmappe und liest von dort an. Ein Blatt, das abgeschnitten wurde, behält die Zellen vor dem Schnitt. Was nach dem Schnitt kam, geht verloren, und Sie werden darauf hingewiesen. Die reparierte Arbeitsmappe wird als Kopie geöffnet, speichern Sie sie daher unter einem neuen Namen.

## Umfangreiche Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern der Arbeitsmappe geöffnet war, erscheinen fast sofort, auch wenn die Arbeitsmappe sehr groß ist. Die Statusleiste unten links zeigt, was gerade geschieht: **Wird geöffnet**, **Blatt wird gelesen** mit der Anzahl der bisher gelesenen Zellen, **Berechnung** mit einem Prozentwert und dann **Bereit**.

Sie können während des Lesens blättern, auswählen und kopieren. Die Registerkarten der anderen Blätter werden aktiv, sobald sie gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie sich lesen lässt, und eine Meldung sagt, was weggelassen wurde.

## Wenn sich eine Datei nicht öffnen lässt

Wenn JeffCrunch eine Datei nicht öffnen kann, nennt es den Grund in einfachen Worten und lässt die geöffneten Arbeitsmappen, wie sie sind. Es unterscheidet eine leere Datei von einer, die gar keine Arbeitsmappe ist (etwa ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); ein Zip-Paket, das abgeschnitten oder beschädigt ist, was meist bedeutet, dass ein Download oder Kopievorgang unterbrochen wurde, sodass Sie es erneut vom Original kopieren sollten; eine Arbeitsmappe, die mit einem Kennwort geschützt ist, wonach es fragt (siehe [Passwortgeschützte Arbeitsmappen öffnen](open-password)); und eine zusammengesetzte Datei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Dateibereich listet die Arbeitsmappen auf, die Sie zuletzt geöffnet haben. Klicken Sie mit der rechten Maustaste auf eine, um sie oben in der Liste **anzuheften** oder aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Speichern}** oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei anzulegen, klicken Sie auf **Datei** und dann auf **Speichern unter** und wählen einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine Vorlage als **.xltx**- oder **.xltm**-Datei und jede andere als **.xlsx**-Datei. Ein Name, der auf eine andere Art von Arbeitsmappe endet, wird nicht akzeptiert; die Meldung sagt, welche Endung zu verwenden ist. Die Namen mit den Endungen **.xls**, **.ods**, **.csv**, **.tsv** und **.txt** erzeugen Kopien, wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles Übrige in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird so kopiert, wie es war, Byte für Byte. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den Formeln, die von ihnen abhängen und deshalb ihre neuen Werte erhalten. Haben Sie keine Zelle geändert, bleibt die Datei unberührt.

Hat ein anderes Programm die Datei geändert, nachdem Sie sie geöffnet haben, speichert JeffCrunch nicht ohne Nachfrage darüber, weil dabei die Änderungen des anderen Programms verloren gingen. Es fragt, ob Sie **Kopie speichern**, die Datei **Überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zuerst neben der alten geschrieben und ersetzt diese erst, wenn sie vollständig ist. Tritt ein Fehler auf, erhalten Sie eine Meldung, und die vorhandene Datei bleibt unverändert.

Werden die Formeln beim Speichern noch berechnet, wartet JeffCrunch, bis sie fertig sind, damit die Datei ihre Ergebnisse enthält. Drücken Sie in der Zwischenzeit **Esc**, um die Berechnung anzuhalten, wird das Speichern abgebrochen, und die Statusleiste meldet es; speichern Sie erneut, wenn Sie möchten. JeffCrunch lässt eine Sache aus der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Ein Programm, das die Arbeitsmappe danach öffnet, erstellt sie neu. Konnte JeffCrunch manche Formeln nicht berechnen oder haben Sie die Berechnung angehalten, bittet die Datei das Programm, beim Öffnen alle Formeln neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie diese **speichern**, **nicht speichern** oder ob Sie **Abbrechen** und weiterarbeiten möchten. Ein Sternchen (*) im Fenstertitel zeigt an, dass es nicht gespeicherte Änderungen gibt.

## Integrierte Vorlagen

Unter **Datei > Neu** finden Sie unter der leeren Arbeitsmappe fertige Arbeitsmappen in Gruppen: **Finanzen** (ein monatliches Budget, ein Tilgungsplan und ein Kassenbuch), **Geschäft** (eine Rechnung, eine Spesenabrechnung, ein Stundenzettel, eine Bestandsliste, ein Fahrtenbuch und ein Verkaufsbericht), **Planung** (ein Kalender und ein Projektplan) und **Bildung** (ein Notenbuch). Jede zeigt ein kleines Bild ihres Kopfbereichs. Klicken Sie auf eine, um eine neue Arbeitsmappe daraus zu beginnen. Tragen Sie Werte in die gelben Felder ein; die übrigen Zahlen werden für Sie berechnet. Die Arbeitsmappe hat keine eigene Datei, bis Sie sie speichern.
