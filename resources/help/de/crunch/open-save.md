# Arbeitsmappen öffnen und speichern
<!-- keywords: nicht gespeicherte Arbeit wiederherstellen, automatische Wiederherstellung, Absturz, kopieren, älteres Format, OpenDocument, Kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, Datei, Arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, Textdatei, getrennt, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, laden, groß, Makros, ungespeichert, Änderungen, schließen, Kopie, Berechnungskette, Vorlage, xltx, xltm, lässt sich nicht öffnen, beschädigt, verschlüsselt, Kennwort, Kopie speichern, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Sicherungskopie, Meine Vorlagen, als Vorlage speichern, ots, reparieren, Öffnen und reparieren, integrierte Vorlagen, Budget, Rechnung, Stundenzettel, Darlehen, Kalender, Notenbuch, feste Breite, feste Spalten, Dezimaltrennzeichen, Tausendertrennzeichen, nachgestelltes Minuszeichen, html, Webseite, htm, Tabellen aus einer Seite -->

Wenn JeffCrunch ohne eine zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt namens Mappe1. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 und so weiter), und eine Nummer wird nach dem Schließen des Fensters nicht wieder verwendet. Um eine weitere zu beginnen, drücken Sie **{key:Neu}** oder klicken auf **Datei** und dann auf **Neu**, und wählen Sie **Leere Arbeitsmappe**; sie öffnet sich in einem eigenen Fenster. Eine neue Arbeitsmappe, die Sie nicht geändert haben, weicht einer Datei, die Sie in ihrem Fenster öffnen; eine, die Sie geändert haben, bleibt bestehen, und die Datei öffnet sich in einem anderen Fenster. **Speichern** fragt bei einer neuen Arbeitsmappe nach einem Namen, ebenso wie **Speichern unter**, und speichert sie als .xlsx-Datei. Das Schließen einer neuen Arbeitsmappe, die Sie nicht geändert haben, fragt nicht nach.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Öffnen}** oder klicken auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus dem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Dieses Fenster wird in den Vordergrund gebracht.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (die Makros enthalten) sowie die Vorlagen **.xltx** und **.xltm** (eine Vorlage, die Makros enthält). Es führt keine Makros aus und lässt sie unverändert: Das Makroprojekt, seine Signatur und die Teile, die sie benennen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, da die Art zur Datei gehört.

## Nicht gespeicherte Arbeit wiederherstellen

Wenn JeffCrunch unerwartet beendet wird, etwa bei einem Stromausfall, zeigt der nächste Start eine Liste der Kopien, die das Programm von Ihrer nicht gespeicherten Arbeit angelegt hat. Wählen Sie aus, welche Sie öffnen möchten, und klicken Sie auf **Öffnen**; jede öffnet sich als neue Arbeitsmappe ohne Datei. Klicken Sie daher auf **Speichern** und wählen Sie einen Namen. **Löschen** verwirft die markierten Kopien, und **Nicht jetzt** lässt sie für später. Die Liste können Sie jederzeit über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** aufrufen. Wie oft die Kopien angelegt werden, erfahren Sie unter [JeffCrunch-Einstellungen ändern](options).

## Andere Dateiarten

JeffCrunch öffnet außerdem ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und die einteilige Form **.fods**) sowie Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine aus einer dieser Dateien geöffnete Arbeitsmappe wird als neue **.xlsx**-Datei gespeichert (oder als Kopie im Format .xls oder .ods; siehe unten). Wenn Sie **{key:Speichern}** drücken, weist JeffCrunch darauf hin und fragt nach einem Namen; die geöffnete Datei wird nie verändert. Bilder, Formen, Textfelder, Gruppen, Linien und Textkunst einer **.ods**-Datei werden mit ihrer Position, Größe, Drehung, Reihenfolge, ihren Namen, ihrem Alternativtext, ihren Füllungen und Konturen in die neue Datei übernommen. Ihre Diagramme kommen mit, und zwar aus den Zellen gezeichnet, die sie lesen.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den von der Datei gespeicherten Ergebnissen), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Bereichen und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formate und Datenüberprüfungen werden noch nicht gelesen; eine Meldung oben zeigt an, welche davon die Datei enthält, und eine Kopie, die von hier aus gespeichert wird, enthält sie nicht. Eine mit einem Kennwort geschützte Tabelle fragt beim Öffnen danach; siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei ist ein Blatt. JeffCrunch ermittelt, wie ihre Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche), und wie ihre Buchstaben codiert sind (UTF-8 oder UTF-16, sonst die Codepage Ihrer Sprache). Jeder Wert wird so gelesen, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 auf Deutsch, ein Prozentwert, ein Geldbetrag, ein Datum mit Jahr (etwa 3/15/2024 oder 15.03.2024 auf Deutsch), eine Uhrzeit, WAHR oder FALSCH (auf Deutsch, in Englisch TRUE oder FALSE). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Ziffern bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Ebenso bleiben ein Tag und ein Monat ohne Jahr in beliebiger Reihenfolge und Sprache Text (etwa 3/4, 12.5, 1-2, 1. März oder 25. Dez.), weil das Jahr erfunden werden müsste, sowie Texte wie das Gen MARCH1. Ein Blatt hat 1.048.576 Zeilen und 16.384 Spalten; enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und teilt Ihnen das mit.

Wenn Sie eine Textdatei öffnen, zeigt ein Fenster zuerst ihre ersten Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben, wobei sich mehrere aufeinanderfolgende Trennzeichen auf Wunsch als eines zählen), was ein Feld mit einem Trennzeichen in Anführungszeichen setzt, die Textcodierung (Unicode, die Windows-Codepages, die ISO-8859-Seiten, Mac Roman und die ostasiatischen) und die Zeile, bei der begonnen wird. Über jeder Spalte legen Sie fest, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu behalten), ein **Datum** in der von Ihnen gewählten Reihenfolge oder **Auslassen**. Manche Dateien richten ihre Werte in Spalten gleicher Breite aus, mit Leerzeichen dazwischen und ohne Trennzeichen. Aktivieren Sie dann **Die Spalten haben eine feste Breite**; die ersten Zeilen der Datei erscheinen in einem Feld. Klicken Sie zwischen zwei Zeichen, um dort eine Spalte beginnen zu lassen, und klicken Sie erneut, um sie wieder zu entfernen. Oder geben Sie die Stellen unter **Spalten beginnen bei** ein, etwa 10, 25, 40. Jedes Feld wird an diesen Stellen abgeschnitten, und die Leerzeichen um es herum werden entfernt. **Dezimaltrennzeichen** und **Tausendertrennzeichen** legen fest, wie Zahlen in dieser Datei geschrieben sind, unabhängig von Ihrer Sprache: 1.234,50 ist eintausendzweihundertvierunddreißig­einhalb, wenn die Datei ein Komma als Dezimaltrennzeichen verwendet. Aktivieren Sie **Ein Minuszeichen kann nach der Zahl stehen** für Dateien, die -12 als 12- schreiben. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, doch die ganze Datei wird geöffnet. **Datei > Öffnen > Textdatei öffnen** fragt auch dann, wenn der Dateiname ungewöhnlich ist.

JeffCrunch öffnet außerdem **Webseiten** (**.html** und **.htm**) sowie eine Seite, die unter dem Namen einer Arbeitsmappe gespeichert wurde. Jede Tabelle der Seite wird zu einem Blatt, benannt **Tabelle1**, **Tabelle2** und so weiter. Die Zellen werden so gelesen, als hätten Sie sie eingegeben: Eine Zahl ist eine Zahl, und ein Datum ist ein Datum. Eine Zelle, die über mehrere Spalten oder Zeilen verbunden ist, lässt leere Zellen zurück, damit die übrigen Zellen in ihren Spalten bleiben. Die Wörter außerhalb der Tabellen, die Bilder und das Layout der Seite werden nicht gelesen; eine Seite ohne Tabelle lässt sich nicht öffnen. Nichts, was die Seite benennt, wird aus dem Internet geladen.

Um nur die Werte des Blatts zu speichern, das Sie gerade ansehen, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie die Dateiart **.csv** (oder **.tsv** und **.txt** mit Tabstopps als Trennzeichen). Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht aufnehmen kann (etwa die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros), und fragt, wie geschrieben werden soll: ob **Nur Werte speichern**, ob stattdessen **Als .xlsx speichern** (um alles zu behalten) oder **Abbrechen**:

- **Trennzeichen.** Ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben. Eine **.csv**-Datei beginnt mit einem Semikolon, wo Ihre Sprache ein Komma in Zahlen schreibt, sonst mit einem Komma; eine **.tsv**- oder **.txt**-Datei mit einem Tabstopp.
- **Textcodierung.** UTF-8 mit Markierung (damit andere Programme Akzentbuchstaben richtig übernehmen), UTF-8 ohne Markierung, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die ISO-8859-Seiten, Mac Roman oder die japanische, chinesische, koreanische, hebräische, arabische, baltische, vietnamesische und thailändische Codepage. Ein Buchstabe, für den die Codepage kein Byte hat, wird als Fragezeichen geschrieben, und die Statusleiste zeigt an, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder älteres Mac-Format (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt jede Ziffer einer Zahl (0,1 und nicht 10 %) und ein Datum als ISO-Text wie 2026-03-04.

Text wird in Anführungszeichen gesetzt, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die übrigen Blätter, die Formeln, die Formate und die Einstellungen sind nicht in der Datei enthalten, und Ihre Arbeitsmappe wird durch das Schreiben einer solchen Datei nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form zurückgeschrieben, die die Datei hatte: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Wenn Sie sie ohne Änderung einer Zelle als Kopie speichern, ist die Kopie dieselbe Datei, Byte für Byte, und es wird nichts gefragt.

## Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme für OpenDocument öffnen können, klicken Sie auf **Datei**, dann auf **Speichern unter**, und wählen Sie die Dateiart **.xls** oder **.ods**. Die Kopie wird aus Ihrer Arbeitsmappe in ihrem jetzigen Zustand erstellt, mit den Änderungen, die Sie noch nicht gespeichert haben, und enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit den Ergebnissen, die sie jetzt haben (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, Ausrichtungen und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Gliederungen von Zeilen und Spalten sowie fixierte Bereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und der Angabe, welche ausgeblendet sind, die Namen, die Sie festgelegt haben, sowie die Druckbereiche;
- die Diagramme jedes Blatts (nur in einer **.ods**-Kopie): Säulen, Balken, Linien, Flächen, Kreise, Ringe und Punktdiagramme mit ihren Datenreihen und den Zellen, die sie lesen, Kategorien, Titeln, Legende, Achsen (Titel, Skalierung, Zahlenformat), Datenbeschriftungen, Farben und Platz (was ein **.ods**-Diagramm nicht abbilden kann, etwa Trendlinien, Fehlerindikatoren und Sekundärachsen, wird in der Meldung genannt);
- die Bilder, Formen, Textfelder, Gruppen, Linien und Textkunst jedes Blatts (nur in einer **.ods**-Kopie) mit ihrer Position, Größe, Drehung, Reihenfolge, ihren Namen, ihrem Alternativtext, ihren Füllungen, Konturen und Wörtern sowie den Bildern selbst mit ihren Zuschnitten (Textkunst wird als Fontwork-Form gespeichert, wie OpenDocument sie kennt);
- die Verknüpfungen und wie jedes Blatt gedruckt wird (Papier, Ränder, Ausrichtung, Skalierung, Kopf- und Fußzeilen sowie Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe. Eine Kopie kann nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht aufnehmen kann: Diagramme, Bilder und andere Zeichnungen (bei einer **.xls**-Kopie; eine **.ods**-Kopie behält sie, Diagramme eingeschlossen, außer nicht lesbaren Diagrammen), Notizen, Pivot-Tabellen, Tabellen, bedingte Formate, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Verknüpfungen zu anderen Arbeitsmappen (die Formeln, die sie verwenden, behalten ihre bisherigen Werte). Eine Formel, die die ältere Form nicht ausdrücken kann (eine Funktion, die sie nicht kennt, oder eine Zelle hinter Spalte IV oder Zeile 65.536), wird als der Wert geschrieben, auf den sie sich ergab, und die Meldung zählt sie. Die ältere Form endet außerdem bei 65.536 Zeilen, 256 Spalten, 255 Blättern und 32.767 Zeichen in einer Zelle sowie bei etwa 4.000 Zellformaten und 56 Farben: Was nicht hineinpasst, wird abgeschnitten, und die Meldung nennt, wie viel. Anschließend können Sie **Kopie speichern**, **Als .xlsx speichern** (um alles zu behalten) oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die geöffnete Datei wird nie geschrieben: Geben Sie der Kopie einen anderen Namen. Eine Kopie einer geöffneten .xls- oder .ods-Datei kann in der jeweils anderen Art gespeichert werden.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffCrunch schreibt die Arbeitsmappe in diese Datei, mit den Änderungen, die Sie noch nicht gespeichert haben, und Sie arbeiten in der bisherigen Arbeitsmappe weiter. Ihr Name und ihre nicht gespeicherten Änderungen bleiben unverändert. Sie können jede Dateiart wählen, die **Speichern unter** anbietet. Eine Kopie wird niemals über die eigene Datei der Arbeitsmappe geschrieben.

## Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen** und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffCrunch zuerst die alte Datei in eine Datei namens „Sicherungskopie von“ mit dem bisherigen Namen im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne sie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja wählen, und die Datei bleibt unverändert.

## Als Vorlage speichern

Eine Vorlage ist eine Arbeitsmappe, die Sie als Ausgangspunkt verwenden. Sie kann Formeln, Formatvorlagen und Blätter enthalten. Klicken Sie auf **Datei > Als Vorlage speichern** und geben Sie einen Namen ein. Die Vorlage (eine .xltx-Datei) wird in **Meine Vorlagen** abgelegt, Ihre Arbeitsmappe bleibt unverändert. Unter **Speichern unter** können Sie außerdem **Vorlagen (*.xltx)** oder **OpenDocument-Tabellenvorlagen (*.ots)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um eine Arbeitsmappe aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgeführt. Klicken Sie eine davon an. Eine neue Arbeitsmappe öffnet sich mit den Zellen und Blättern der Vorlage. Sie hat noch keine eigene Datei, daher fragt **Speichern** nach einem Namen, und die Vorlage bleibt unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie sie mit der rechten Maustaste an und wählen **Aus Meine Vorlagen entfernen**.

## Andere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste **Zuletzt verwendet** klicken.

- **Schreibgeschützt öffnen** öffnet die Arbeitsmappe nur zum Lesen. Sie können sie ansehen, blättern, suchen und kopieren, aber keine Zelle ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffCrunch nach einem neuen Namen, damit die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Arbeitsmappe ohne eigene Datei. **Speichern** fragt, wo sie abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Arbeitsmappe in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient dazu, eine beschädigte .xlsx- oder .xls-Datei zu retten. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .xlsx-Datei beschädigt. Ein Download wurde auf halbem Weg abgebrochen oder ein Datenträger hat Fehler gemacht. Eine .xlsx-Datei ist ein Bündel kleinerer Teile, etwa der Blätter, der Formatvorlagen und des gemeinsamen Textes. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, das mitten im Schreiben abbricht, wird bis zu der Stelle übernommen, an der es abbricht; so erhalten Sie möglicherweise nur die ersten Zeilen eines Blatts. Ein Teil, das sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, stellt JeffCrunch sie wieder her.

Die reparierte Arbeitsmappe öffnet sich ohne eigene Datei, mit einem Namen wie „Reparierte Haushaltsplanung“. Eine Meldung sagt Ihnen, was bis zu einem Bruch übernommen und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Arbeitsmappe unter einem neuen Namen, und prüfen Sie die Zahlen, bevor Sie ihnen vertrauen.

### Eine .xls-Datei

Auch eine .xls-Datei kann beschädigt sein. Ist ihre äußere Hülle abgeschnitten, sucht **Öffnen und reparieren** in der Datei nach dem Anfang der Arbeitsmappe und liest von dort an. Ein abgeschnittenes Blatt behält die Zellen vor dem Schnitt. Was nach dem Schnitt kam, geht verloren, und Sie werden darauf hingewiesen. Die reparierte Arbeitsmappe öffnet sich als Kopie, speichern Sie sie daher unter einem neuen Namen.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern der Arbeitsmappe geöffnet war, erscheinen fast sofort, auch wenn die Arbeitsmappe sehr groß ist. Die Statusleiste unten links zeigt, was gerade geschieht: **Wird geöffnet**, **Blatt wird gelesen** mit der Anzahl der bisher gelesenen Zellen, **Berechnung läuft** mit einem Prozentwert und danach **Bereit**.

Während des Lesens können Sie blättern, Zellen markieren und kopieren. Die Registerkarten der übrigen Blätter werden aktiviert, sobald diese gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie sich lesen lässt, und eine Meldung sagt, was weggelassen wurde.

## Wenn sich eine Datei nicht öffnen lässt

Wenn JeffCrunch eine Datei nicht öffnen kann, nennt es den Grund in einfachen Worten und lässt die geöffneten Arbeitsmappen unverändert. Das Programm unterscheidet eine leere Datei von einer, die gar keine Arbeitsmappe ist (etwa ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); ein ZIP-Paket, das abgeschnitten oder beschädigt ist, was meist bedeutet, dass ein Download oder Kopiervorgang unterbrochen wurde, sodass Sie sie vom Original erneut kopieren sollten; eine Arbeitsmappe, die mit einem Kennwort geschützt ist, wonach es fragt (siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password)); und eine zusammengesetzte Datei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Dateibereich listet die Arbeitsmappen auf, die Sie zuletzt geöffnet haben. Klicken Sie mit der rechten Maustaste auf eine, um sie oben in der Liste anzuheften oder aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Speichern}** oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu sichern. Um das Original zu behalten und eine neue Datei anzulegen, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie einen Namen. Eine Arbeitsmappe mit Makros wird als .xlsm-Datei gespeichert, eine Vorlage als .xltx- oder .xltm-Datei und jede andere Arbeitsmappe als .xlsx-Datei. Ein Name, der auf eine andere Art von Arbeitsmappe endet, wird nicht akzeptiert, und die Meldung sagt, welche Endung Sie verwenden sollen. Namen, die auf **.xls**, **.ods**, **.csv**, **.tsv** und **.txt** enden, erzeugen Kopien, wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles Übrige in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird Byte für Byte unverändert übernommen. In einem geänderten Blatt werden nur die geänderten Zellen neu geschrieben, zusammen mit den Formeln, die von ihnen abhängen und daher ihre neuen Werte erhalten. Haben Sie keine Zelle geändert, bleibt die Datei unberührt.

Hat ein anderes Programm die Datei geändert, nachdem Sie sie geöffnet haben, speichert JeffCrunch nicht ohne Nachfrage darüber, weil dabei die Änderungen des anderen Programms verloren gingen. Das Programm fragt, ob Sie **Kopie speichern**, die Datei **Überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zuerst neben der alten geschrieben und ersetzt die alte erst, wenn sie vollständig ist. Geht etwas schief, erscheint eine Meldung, und die vorhandene Datei bleibt, wie sie war.

Werden die Formeln beim Speichern noch berechnet, wartet JeffCrunch, bis sie fertig sind, damit die Datei ihre Ergebnisse enthält. Drücken Sie in der Zwischenzeit **Esc**, um die Berechnung abzubrechen, wird das Speichern abgebrochen, und die Statusleiste zeigt es an; speichern Sie dann erneut, wenn Sie möchten. JeffCrunch lässt eines aus der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Ein Programm, das die Arbeitsmappe danach öffnet, legt sie neu an. Konnte JeffCrunch eine Formel nicht berechnen oder haben Sie die Berechnung abgebrochen, bittet die Datei das öffnende Programm, alle Formeln beim Öffnen der Arbeitsmappe neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit nicht gespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie **Speichern**, **Nicht speichern** oder **Abbrechen** wählen und weiterarbeiten möchten. Ein Sternchen (*) im Fenstertitel zeigt an, dass nicht gespeicherte Änderungen vorhanden sind.

## Integrierte Vorlagen

Unter **Datei > Neu** finden Sie unter der leeren Arbeitsmappe fertige Arbeitsmappen in Gruppen: **Finanzen** (ein monatliches Budget, ein Kredittilgungsplan und ein Kassenbuch), **Geschäft** (eine Rechnung, eine Spesenabrechnung, ein Stundenzettel, eine Bestandsliste, ein Fahrtenbuch und ein Verkaufsbericht), **Planung** (ein Kalender und ein Projektplan) sowie **Bildung** (ein Notenbuch). Jede zeigt ein kleines Bild ihres Kopfbereichs. Klicken Sie auf eine, um eine neue Arbeitsmappe daraus zu beginnen. Tragen Sie Ihre Angaben in die gelben Felder ein. Die übrigen Zahlen werden für Sie berechnet. Die Arbeitsmappe hat keine eigene Datei, bis Sie sie speichern.
