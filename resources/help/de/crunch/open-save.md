# Arbeitsmappen öffnen und speichern
<!-- keywords: kopieren, älteres Format, OpenDocument, Kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, getrennt, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, laden, groß, umfangreich, Makros, nicht gespeichert, Änderungen, schließen, Kopie, Berechnungskette, Vorlage, xltx, xltm, kann nicht geöffnet werden, beschädigt, verschlüsselt, Kennwort, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ots, reparieren, öffnen und reparieren, integrierte Vorlagen, Budget, Rechnung, Stundenzettel, Darlehen, Kalender, Notenbuch, feste Breite, feste Spalten, Dezimaltrennzeichen, Tausendertrennzeichen, nachgestelltes Minus, HTML, Webseite, htm, Tabellen aus einer Seite, Lesen, Zelle, Tabelle</keywords -->

Wenn JeffCrunch ohne eine zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 und so weiter). Eine Nummer wird nach dem Schließen des Fensters nicht wieder vergeben. Um eine weitere Arbeitsmappe zu beginnen, drücken Sie **{key:New}** oder klicken auf **Datei** und dann auf **Neu** und wählen **Leere Arbeitsmappe**. Sie wird in einem eigenen Fenster geöffnet. Eine neue Arbeitsmappe, die Sie nicht geändert haben, wird ersetzt, wenn Sie in ihrem Fenster eine Datei öffnen. Haben Sie sie dagegen geändert, bleibt sie erhalten, und die Datei wird in einem anderen Fenster geöffnet. **Speichern** fragt bei einer neuen Arbeitsmappe nach einem Namen, ebenso wie **Speichern unter**, und speichert sie als .xlsx-Datei. Beim Schließen einer neuen Arbeitsmappe, die Sie nicht geändert haben, erscheint keine Abfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Open}** oder klicken auf **Datei** und dann auf **Öffnen** und wählen **Durchsuchen**. Sie können eine Arbeitsmappe auch aus dem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht noch einmal geöffnet: Dieses Fenster wird in den Vordergrund gebracht.

JeffCrunch öffnet .xlsx-Arbeitsmappen und .xlsm-Arbeitsmappen (mit Makros) sowie die Vorlagen .xltx und .xltm (eine Vorlage mit Makros). Makros werden nicht ausgeführt, sondern unverändert belassen: Das Makroprojekt, seine Signatur und die Teile, die auf sie verweisen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, weil die Art zur Datei gehört.

## Andere Dateiarten

JeffCrunch öffnet auch ältere .xls-Arbeitsmappen, OpenDocument-Tabellen (.ods und die flache Form .fods) sowie Textdateien mit Werten (.csv, .tsv und .txt). Eine Arbeitsmappe, die aus einer dieser Dateien geöffnet wurde, wird als neue .xlsx-Datei gespeichert (oder als Kopie im Format .xls oder .ods, siehe unten). Wenn Sie **{key:Save}** drücken, weist JeffCrunch darauf hin und fragt nach einem Namen. Die geöffnete Datei wird nie verändert.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den Ergebnissen, die die Datei gespeichert hat), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Bereichen und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen. Eine Meldung oben zeigt an, welche davon die Datei enthält, und eine Kopie, die von hier aus gespeichert wird, enthält sie nicht. Eine Tabelle, die mit einem Kennwort geschützt ist, fragt beim Öffnen danach. Siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei ist ein einziges Blatt. JeffCrunch ermittelt, wie ihre Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche), und wie ihre Zeichen codiert sind (UTF-8 oder UTF-16, sonst die Codepage Ihrer Sprache). Jeder Wert wird so gelesen, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 in Deutsch, ein Prozentwert, ein Geldbetrag, ein Datum mit Jahr (etwa 3/15/2024 oder 15.03.2024 in Deutsch), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache, etwa WAHR und FALSCH in Deutsch sowie TRUE und FALSE in Englisch). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Ziffern bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Das gilt ebenso für einen Tag und einen Monat ohne Jahr, in beliebiger Reihenfolge und Sprache (etwa 3/4, 12.5, 1-2, 1. März oder 25. Dez.), weil das Jahr dann erfunden werden müsste, sowie für Texte wie das Gen MARCH1. Ein Blatt umfasst 1.048.576 Zeilen und 16.384 Spalten. Enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und teilt Ihnen das mit.

Wenn Sie eine Textdatei öffnen, zeigt ein Fenster zunächst die ersten Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben; mehrere aufeinanderfolgende Trennzeichen können Sie als eines zählen lassen), welche Anführungszeichen ein Feld einschließen, das ein Trennzeichen enthält, die Textcodierung (Unicode, die Windows-Codepages, die Seiten ISO 8859, Mac Roman und die ostasiatischen Codepages) und die Zeile, bei der das Lesen beginnt. Über jeder Spalte legen Sie fest, wie sie gelesen wird: **Wie eingegeben**, **Text** (damit etwa eine Artikelnummer wie 007 oder eine lange Kontonummer unverändert bleibt), **Datum** in der von Ihnen angegebenen Reihenfolge oder **Nicht übernehmen**.

Manche Dateien richten ihre Werte in Spalten gleicher Breite aus, mit Leerzeichen dazwischen und ohne Trennzeichen. Aktivieren Sie dann **Die Spalten haben eine feste Breite**. Die ersten Zeilen der Datei erscheinen in einem Feld. Klicken Sie zwischen zwei Zeichen, um dort eine Spalte beginnen zu lassen, und klicken Sie erneut, um die Trennung zu entfernen. Alternativ tippen Sie die Stellen unter **Spalten beginnen bei** ein, etwa 10, 25, 40. Jeder Teil wird an diesen Stellen abgeschnitten, und die Leerzeichen um ihn herum werden entfernt. Mit **Dezimaltrennzeichen** und **Tausendertrennzeichen** teilen Sie JeffCrunch mit, wie Zahlen in dieser Datei geschrieben sind, unabhängig von der Sprache Ihres Programms. So wird 1.234,50 als eintausendzweihundertvierunddreißig und eine halbe Einheit gelesen, wenn die Datei ein Komma als Dezimaltrennzeichen verwendet. Aktivieren Sie **Ein Minuszeichen kann nach der Zahl stehen** für Dateien, die -12 als 12- schreiben. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, die gesamte Datei wird aber geöffnet. **Datei > Öffnen > Textdatei öffnen** fragt auch dann, wenn der Dateiname ungewöhnlich ist.

JeffCrunch öffnet auch **Webseiten** (.html und .htm) sowie eine Seite, die mit der Endung einer Arbeitsmappe gespeichert wurde. Jede Tabelle der Seite wird zu einem Blatt, das **Tabelle1**, **Tabelle2** und so weiter heißt. Die Zellen werden gelesen, als hätten Sie sie eingegeben: Eine Zahl ist eine Zahl und ein Datum ein Datum. Eine Zelle, die über mehrere Spalten oder Zeilen verbunden ist, lässt leere Zellen zurück, damit die übrigen Zellen in ihren Spalten bleiben. Der Text außerhalb der Tabellen, die Bilder und das Aussehen der Seite werden nicht gelesen. Eine Seite ohne Tabelle lässt sich nicht öffnen. Was die Seite benennt, wird nicht aus dem Internet geladen.

Um nur die Werte des gerade angezeigten Blatts zu speichern, klicken Sie auf **Datei**, dann auf **Speichern unter** und wählen die Dateiart **.csv** (oder **.tsv** und **.txt** mit Tabstopps als Trennzeichen). Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann (etwa die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros). Anschließend fragt es, wie geschrieben werden soll, und ob Sie **Nur Werte speichern**, **Als .xlsx speichern** (und alles behalten) oder **Abbrechen** wählen:

- **Trennzeichen.** Ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben. Eine **.csv**-Datei beginnt mit einem Semikolon, wenn Ihre Sprache in Zahlen ein Komma schreibt, sonst mit einem Komma. Eine **.tsv**- oder **.txt**-Datei verwendet einen Tabstopp.
- **Textcodierung.** UTF-8 mit Markierung (damit andere Programme Buchstaben mit Akzent richtig übernehmen), UTF-8 ohne Markierung, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die Seiten ISO 8859, Mac Roman sowie die Codepages für Japanisch, Chinesisch, Koreanisch, Hebräisch, Arabisch, Baltisch, Vietnamesisch und Thai. Ein Zeichen, für das die Codepage kein Byte hat, wird als Fragezeichen geschrieben. Die Statusleiste zeigt an, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder alter Mac (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt jede Ziffer einer Zahl (0,1 und nicht 10 %) und ein Datum als ISO-Text wie 2026-03-04.

Text wird in Anführungszeichen gesetzt, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die übrigen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in der Datei enthalten, und Ihre Arbeitsmappe wird durch das Schreiben einer Datei nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form zurückgeschrieben, die die Datei hatte: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Wenn Sie sie ohne Änderung einer Zelle als Kopie speichern, ist die Kopie dieselbe Datei, Byte für Byte, und es wird nicht nachgefragt.

## Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme für OpenDocument öffnen können, klicken Sie auf **Datei**, dann auf **Speichern unter** und wählen die Dateiart **.xls** oder **.ods**. Die Kopie wird aus Ihrer Arbeitsmappe in ihrem jetzigen Zustand erstellt, einschließlich der Änderungen, die Sie noch nicht gespeichert haben. Sie enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit den Ergebnissen, die sie jetzt haben (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, Ausrichtungen und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Gliederungen von Zeilen und Spalten sowie fixierte Bereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und der Angabe, welche ausgeblendet sind, sowie die definierten Namen (und die Druckbereiche);
- die Verknüpfungen und die Druckeinstellungen jedes Blatts (Papier, Ränder, Ausrichtung, Skalierung, Kopf- und Fußzeilen sowie Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe. Eine Kopie kann aber nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht speichern kann: Diagramme, Bilder und andere Zeichnungen, Notizen, Pivot-Tabellen, Tabellen, bedingte Formatierungen, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Verknüpfungen zu anderen Arbeitsmappen (die Formeln, die sie verwenden, behalten die Werte, die sie hatten). Eine Formel, die das ältere Format nicht ausdrücken kann (eine Funktion, die es nicht kennt, oder eine Zelle hinter Spalte IV oder Zeile 65.536), wird als der Wert geschrieben, den sie ergab, und die Meldung zählt sie. Das ältere Format endet außerdem bei 65.536 Zeilen, 256 Spalten, 255 Blättern und 32.767 Zeichen in einer Zelle sowie bei etwa 4.000 Zellformaten und 56 Farben. Was nicht hineinpasst, wird abgeschnitten, und die Meldung sagt, wie viel. Anschließend können Sie **Kopie speichern**, **Als .xlsx speichern** (und alles behalten) oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die geöffnete Datei wird nie geschrieben: Geben Sie der Kopie einen anderen Namen. Eine Kopie einer geöffneten .xls- oder .ods-Datei kann in der jeweils anderen Art gespeichert werden.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffCrunch schreibt die Arbeitsmappe in diese Datei, einschließlich der Änderungen, die Sie noch nicht gespeichert haben, und Sie arbeiten in der bisherigen Arbeitsmappe weiter. Deren Name und nicht gespeicherte Änderungen bleiben unverändert. Sie können jede Dateiart wählen, die **Speichern unter** anbietet. Eine Kopie wird nie über die eigene Datei der Arbeitsmappe geschrieben.

## Sicherungskopie erstellen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffCrunch zuerst die alte Datei in eine Datei mit dem Namen „Sicherungskopie von“ und dem Namen der Datei im selben Ordner. Lässt sich die Kopie nicht erstellen, fragt das Programm, ob ohne sie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja sagen, und die Datei bleibt unverändert.

## Als Vorlage speichern

Eine Vorlage ist eine Arbeitsmappe, die Sie als Ausgangspunkt verwenden. Sie kann Formeln, Formatvorlagen und Blätter enthalten. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage (eine .xltx-Datei) wird unter **Meine Vorlagen** gespeichert, Ihre Arbeitsmappe bleibt unverändert. Im Dialog **Speichern unter** können Sie auch **Vorlagen (*.xltx)** oder **OpenDocument-Tabellenvorlagen (*.ots)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um eine Arbeitsmappe aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen stehen unter **Meine Vorlagen**. Klicken Sie eine davon an. Eine neue Arbeitsmappe mit den Zellen und Blättern der Vorlage wird geöffnet. Sie hat noch keine eigene Datei, daher fragt **Speichern** nach einem Namen, und die Vorlage bleibt unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Weitere Möglichkeiten zum Öffnen

Auf der Seite **Öffnen** klicken Sie auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken.

- **Schreibgeschützt öffnen** öffnet die Arbeitsmappe nur zum Lesen. Sie können sie ansehen, blättern, suchen und kopieren, aber keine Zelle ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffCrunch nach einem neuen Namen, damit die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Arbeitsmappe ohne eigene Datei. **Speichern** fragt, wo sie abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Arbeitsmappe in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .xlsx- oder .xls-Datei. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .xlsx-Datei beschädigt, etwa weil ein Download auf halber Strecke abbrach oder eine Festplatte Fehler gemacht hat. Eine .xlsx-Datei ist ein Bündel kleinerer Teile, etwa der Blätter, der Formatvorlagen und des gemeinsamen Texts. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Lesen abbrach, wird bis zu der Stelle übernommen, an der er abbrach. So erhalten Sie möglicherweise nur die ersten Zeilen eines Blatts. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, erstellt JeffCrunch sie neu.

Die reparierte Arbeitsmappe wird ohne eigene Datei geöffnet, mit einem Namen wie „Reparierte Budget“. Eine Meldung informiert Sie, was bis zu einem Abbruch übernommen und was weggelassen wurde. Die beschädigte Datei bleibt unverändert. Speichern Sie die reparierte Arbeitsmappe unter einem neuen Namen und prüfen Sie die Zahlen, bevor Sie ihnen vertrauen.

### Eine .xls-Datei

Auch eine .xls-Datei kann beschädigt sein. Ist ihre äußere Hülle abgeschnitten, sucht **Öffnen und reparieren** in der Datei nach dem Anfang der Arbeitsmappe und liest von dort an. Ein abgeschnittenes Blatt behält die Zellen vor dem Abbruch. Was nach dem Abbruch kam, geht verloren, und Sie werden darüber informiert. Die reparierte Arbeitsmappe wird als Kopie geöffnet. Speichern Sie sie daher unter einem neuen Namen.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern der Arbeitsmappe geöffnet war, erscheinen fast sofort, auch wenn die Arbeitsmappe sehr groß ist. Die Statusleiste unten links zeigt, was das Programm gerade tut: **Öffnen**, **Lesen** eines Blatts mit der bisher gelesenen Anzahl an Zellen, **Berechnen** mit einem Prozentwert und dann **Bereit**.

Während des Lesens können Sie blättern, auswählen und kopieren. Die Registerkarten der übrigen Blätter werden aktiviert, sobald sie gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie sich lesen lässt, und eine Meldung sagt, was weggelassen wurde.

## Wenn sich eine Datei nicht öffnen lässt

Kann JeffCrunch eine Datei nicht öffnen, nennt es den Grund in einfachen Worten und lässt die geöffneten Arbeitsmappen unverändert. Es unterscheidet, ob die Datei leer ist oder keine Arbeitsmappe enthält (etwa ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); ein Zip-Paket, das abgeschnitten oder beschädigt ist, was meist bedeutet, dass ein Download oder Kopiervorgang unterbrochen wurde, sodass Sie die Datei vom Original erneut kopieren sollten; eine Arbeitsmappe, die mit einem Kennwort geschützt ist, nach dem das Programm fragt (siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password)); und eine zusammengesetzte Datei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Dateibereich listet die Arbeitsmappen auf, die Sie zuletzt geöffnet haben. Klicken Sie mit der rechten Maustaste auf eine, um sie oben in der Liste **anzuheften** oder aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Save}** oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei anzulegen, klicken Sie auf **Datei** und dann auf **Speichern unter** und wählen einen Namen. Eine Arbeitsmappe mit Makros wird als .xlsm-Datei gespeichert, eine Vorlage als .xltx- oder .xltm-Datei und jede andere Arbeitsmappe als .xlsx-Datei. Ein Name, der auf eine andere Art von Arbeitsmappe endet, wird nicht akzeptiert, und die Meldung nennt die passende Endung. Die Endungen .xls, .ods, .csv, .tsv und .txt erstellen Kopien, wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles Übrige in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird unverändert übernommen, Byte für Byte. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den Formeln, die von ihnen abhängen und deren neue Werte erhalten. Haben Sie keine Zelle geändert, bleibt die Datei unberührt.

Hat ein anderes Programm die Datei geändert, nachdem Sie sie geöffnet haben, speichert JeffCrunch nicht ohne Nachfrage darüber, weil dabei die Änderungen des anderen Programms verloren gingen. Das Programm fragt, ob Sie **Kopie speichern**, die Datei **überschreiben** oder **Abbrechen** wählen möchten.

Die neue Datei wird zuerst neben der alten geschrieben und ersetzt die alte erst, wenn sie vollständig ist. Tritt ein Fehler auf, erhalten Sie eine Meldung, und die vorhandene Datei bleibt unverändert.

Sind die Formeln beim Speichern noch in Berechnung, wartet JeffCrunch, bis sie fertig sind, damit die Datei ihre Ergebnisse enthält. Drücken Sie in der Zwischenzeit **Esc**, um die Berechnung abzubrechen, wird das Speichern abgebrochen, und die Statusleiste meldet es. Speichern Sie dann erneut, wenn Sie so weit sind. JeffCrunch lässt eines aus der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Ein Programm, das die Arbeitsmappe danach öffnet, erstellt sie neu. Konnte JeffCrunch einige Formeln nicht berechnen oder haben Sie die Berechnung abgebrochen, bittet die Datei das öffnende Programm, beim Öffnen der Arbeitsmappe alle Formeln neu zu berechnen.

## Schließen mit ungespeicherten Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie diese **Speichern**, **Nicht speichern** oder **Abbrechen** wollen, um weiterzuarbeiten. Ein Sternchen (*) im Titel des Fensters zeigt an, dass es nicht gespeicherte Änderungen gibt.

## Integrierte Vorlagen

Unter **Datei > Neu** finden Sie unter der leeren Arbeitsmappe fertige Arbeitsmappen in Gruppen: **Finanzen** (ein Monatsbudget, ein Tilgungsplan und ein Kassenbuch), **Geschäft** (eine Rechnung, eine Spesenabrechnung, ein Stundenzettel, eine Bestandsliste, ein Fahrtenbuch und ein Verkaufsbericht), **Planung** (ein Kalender und ein Projektplan) und **Bildung** (ein Notenbuch). Jede zeigt ein kleines Bild ihres Kopfbereichs. Klicken Sie auf eine, um eine neue Arbeitsmappe daraus zu beginnen. Tragen Sie Ihre Angaben in die gelben Felder ein. Die übrigen Zahlen werden für Sie berechnet. Die Arbeitsmappe hat keine eigene Datei, bis Sie sie speichern.
