# Arbeitsmappen öffnen und speichern
<!-- keywords: wiederherstellen, nicht gespeicherte Arbeit wiederherstellen, automatische Wiederherstellung, Absturz, kopieren, älteres Format, opendocument, kompatibilität, neu, leer, Mappe1, unbenannt, öffnen, speichern, speichern unter, datei, arbeitsmappe, xlsx, xlsm, xls, ods, csv, tsv, textdatei, getrennt, importieren, exportieren, zuletzt verwendet, angeheftet, ziehen, ablegen, laden, groß, makros, nicht gespeichert, änderungen, schließen, kopie, berechnungskette, vorlage, xltx, xltm, lässt sich nicht öffnen, beschädigt, verschlüsselt, kennwort, kopie speichern, schreibgeschützt, nur lesen, als kopie öffnen, neues fenster, sicherungskopie, meine vorlagen, als vorlage speichern, ots, reparieren, öffnen und reparieren, integrierte vorlagen, budget, rechnung, stundenzettel, kredit, kalender, notenbuch, feste breite, feste spalten, dezimaltrennzeichen, tausendertrennzeichen, nachgestelltes minuszeichen, html, webseite, htm, tabellen aus einer seite</keywords>

Wenn JeffCrunch ohne eine zu öffnende Datei startet, enthält das Fenster eine neue, leere Arbeitsmappe mit einem Blatt, das Mappe1 heißt. Jede neue Arbeitsmappe erhält die nächste Nummer (Mappe2, Mappe3 und so weiter), und eine Nummer wird nach dem Schließen des Fensters nicht wieder verwendet. Um eine weitere zu beginnen, drücken Sie **{key:New}**, oder klicken Sie auf **Datei** und dann auf **Neu**, und wählen Sie **Leere Arbeitsmappe**; sie wird in einem eigenen Fenster geöffnet. Eine neue Arbeitsmappe, die Sie nicht geändert haben, weicht, wenn Sie in ihrem Fenster eine Datei öffnen; eine geänderte bleibt erhalten, und die Datei wird in einem anderen Fenster geöffnet. **Speichern** fragt bei einer neuen Arbeitsmappe nach einem Namen, ebenso wie **Speichern unter**, und speichert sie als .xlsx-Datei. Beim Schließen einer neuen Arbeitsmappe, die Sie nicht geändert haben, erfolgt keine Nachfrage.

Um eine Arbeitsmappe zu öffnen, drücken Sie **{key:Open}**, oder klicken Sie auf **Datei** und dann auf **Öffnen**, und wählen Sie **Durchsuchen**. Sie können eine Arbeitsmappe auch aus dem Dateimanager auf das Fenster ziehen. Eine Arbeitsmappe, die bereits in einem Fenster geöffnet ist, wird nicht ein zweites Mal geöffnet: Dieses Fenster wird nach vorne geholt.

JeffCrunch öffnet **.xlsx**-Arbeitsmappen und **.xlsm**-Arbeitsmappen (die Makros enthalten) sowie die Vorlagen **.xltx** und **.xltm** (eine Vorlage, die Makros enthält). Es führt keine Makros aus und lässt sie unverändert: Das Makroprojekt, seine Signatur und die Teile, die sie benennen, werden genau so zurückgeschrieben, wie sie waren. Eine Vorlage wird als Vorlage gespeichert und eine Arbeitsmappe mit Makros als Arbeitsmappe mit Makros, weil die Art zur Datei gehört.

## Nicht gespeicherte Arbeit wiederherstellen

Wenn JeffCrunch unerwartet beendet wird, etwa bei einem Stromausfall, zeigt der nächste Start eine Liste der Kopien, die JeffCrunch von Ihrer nicht gespeicherten Arbeit angelegt hat. Wählen Sie aus, welche Sie öffnen möchten, und klicken Sie auf **Öffnen**; jede wird als neue Arbeitsmappe ohne Datei geöffnet, daher klicken Sie auf **Speichern** und wählen einen Namen. **Löschen** verwirft die markierten Kopien, und **Nicht jetzt** lässt sie für später bestehen. Die Liste können Sie jederzeit über **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** aufrufen. Wie oft die Kopien angelegt werden, erfahren Sie unter [JeffCrunch-Einstellungen ändern](options).

## Andere Dateiarten

JeffCrunch öffnet außerdem ältere **.xls**-Arbeitsmappen, OpenDocument-Tabellen (**.ods** und die einfache Form **.fods**) und Textdateien mit Werten (**.csv**, **.tsv** und **.txt**). Eine Arbeitsmappe, die aus einer dieser Dateien geöffnet wurde, wird als neue **.xlsx**-Datei gespeichert (oder als Kopie im Format **.xls** oder **.ods**: siehe unten). Wenn Sie **{key:Save}** drücken, teilt JeffCrunch dies mit und fragt nach einem Namen; die Datei, die Sie geöffnet haben, wird nie geändert.

Eine OpenDocument-Tabelle wird mit ihren Blättern, Werten, Formeln (mit den Ergebnissen, die die Datei gespeichert hat), Zahlenformaten, Schriftarten, Füllungen, Rahmen, verbundenen Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendeten Zeilen und Spalten, fixierten Bereichen und Namen gelesen. Diagramme, Bilder und andere Zeichnungen, Kommentare, Pivot-Tabellen, Makros, bedingte Formatierungen und Datenüberprüfungen werden noch nicht gelesen; eine Meldung oben zeigt an, welche davon die Datei enthält, und eine von hier gespeicherte Kopie wird sie nicht enthalten. Eine Tabelle, die mit einem Kennwort geschützt ist, fragt beim Öffnen danach: siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password).

Eine Textdatei besteht aus einem Blatt. JeffCrunch ermittelt, wodurch die Werte getrennt sind (durch Kommas, Semikolons, Tabstopps oder senkrechte Striche), und wie die Zeichen codiert sind (UTF-8 oder UTF-16, sonst die Codepage Ihrer Sprache), und liest jeden Wert so, wie Sie ihn in Ihrer Sprache eingeben würden: eine Zahl wie 1.234,5 in Deutsch, einen Prozentwert, einen Geldbetrag, ein Datum mit Jahr (etwa 3/15/2024 oder 15.03.2024 auf Deutsch), eine Uhrzeit, WAHR oder FALSCH (in Ihrer Sprache, also WAHR und FALSCH auf Deutsch, und TRUE oder FALSE auf Englisch). Ein Wert, der mit einem Gleichheitszeichen beginnt, ist eine Formel. Ziffernfolgen, die mit einer Null beginnen (etwa 007), und Zahlen mit mehr als 15 Ziffern bleiben Text, damit eine Postleitzahl oder eine Kontonummer nicht verändert wird. Das gilt ebenso für einen Tag und einen Monat ohne Jahr, in beliebiger Reihenfolge und in jeder Sprache (etwa 3/4, 12.5, 1-2, 1. März oder 25. Dez), weil das Jahr erfunden werden müsste, und für Texte wie den Gennamen MARCH1. Ein Blatt umfasst 1.048.576 Zeilen und 16.384 Spalten; enthält die Datei mehr, öffnet JeffCrunch, was hineinpasst, und teilt dies mit.

Wenn Sie eine Textdatei öffnen, zeigt ein Fenster zuerst deren erste Zeilen und fragt, wie sie gelesen werden soll: was die Felder trennt (eines von Komma, Semikolon, Tabstopp, Leerzeichen, senkrechtem Strich oder ein Zeichen, das Sie eingeben; mehrere aufeinanderfolgende Trennzeichen können Sie als eines zählen lassen), was ein Feld in Anführungszeichen setzt, das ein Trennzeichen enthält, die Textcodierung (Unicode, die Windows-Codepages, die ISO-8859-Codepages, Mac Roman und die ostasiatischen), und die Zeile, bei der das Lesen beginnt. Über jeder Spalte wählen Sie, wie sie gelesen wird: **Wie eingegeben**, **Text** (um eine Teilenummer wie 007 oder eine lange Kontonummer unverändert zu behalten), **Datum** in der von Ihnen genannten Reihenfolge oder **Überspringen**. Manche Dateien richten ihre Werte in Spalten gleicher Breite aus, mit Leerzeichen dazwischen und ohne Trennzeichen. Aktivieren Sie dann **Die Spalten haben eine feste Breite**; die ersten Zeilen der Datei erscheinen in einem Feld. Klicken Sie zwischen zwei Zeichen, um dort eine Spalte beginnen zu lassen, und erneut, um sie wieder zu entfernen; oder geben Sie die Stellen unter **Spalten beginnen bei** ein, etwa 10, 25, 40. Jedes Stück wird an diesen Stellen abgeschnitten, und die Leerzeichen um sie herum werden entfernt. Mit **Dezimaltrennzeichen** und **Tausendertrennzeichen** teilen Sie JeffCrunch mit, wie Zahlen in dieser Datei geschrieben sind, unabhängig von Ihrer Sprache; so ist 1.234,50 eintausendzweihundertvierunddreißigeinhalb, wenn die Datei das Komma als Dezimaltrennzeichen verwendet. Aktivieren Sie **Ein Minuszeichen kann nach der Zahl stehen** für Dateien, die -12 als 12- schreiben. Die Vorschau folgt jeder Änderung und zeigt höchstens die ersten 200 Zeilen und 100 Spalten, die gesamte Datei wird aber geöffnet. **Datei > Öffnen > Textdatei öffnen** fragt auch dann, wenn der Dateiname ungewöhnlich ist.

JeffCrunch öffnet außerdem **Webseiten** (**.html** und **.htm**) sowie eine Seite, die mit der Endung einer Arbeitsmappe gespeichert wurde. Jede Tabelle der Seite wird zu einem Blatt, benannt **Tabelle1**, **Tabelle2** und so weiter. Die Zellen werden so gelesen, als hätten Sie sie eingegeben: Eine Zahl ist eine Zahl, und ein Datum ist ein Datum. Eine Zelle, die über mehrere Spalten oder Zeilen verbunden ist, lässt leere Zellen zurück, sodass die anderen Zellen in ihren Spalten bleiben. Die Wörter außerhalb der Tabellen, die Bilder und das Aussehen der Seite werden nicht gelesen; eine Seite ohne Tabelle lässt sich nicht öffnen. Nichts, worauf die Seite verweist, wird aus dem Internet geladen.

Um nur die Werte des Blatts zu speichern, das Sie gerade betrachten, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie die Dateiart **.csv** (oder **.tsv** und **.txt** mit Tabstopps als Trennzeichen). Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht aufnehmen kann (etwa die anderen Blätter, Formeln, Bilder, Diagramme, Notizen und Makros), und fragt, wie geschrieben werden soll: ob **Nur Werte speichern**, ob **Als .xlsx speichern** stattdessen, um alles zu behalten, oder ob **Abbrechen**:

- **Trennzeichen.** Ein Komma, Semikolon, Tabstopp, Leerzeichen, senkrechter Strich oder ein Zeichen, das Sie eingeben. Eine **.csv**-Datei beginnt mit einem Semikolon, wo Ihre Sprache in Zahlen ein Komma schreibt, sonst mit einem Komma; eine **.tsv**- oder **.txt**-Datei mit einem Tabstopp.
- **Textcodierung.** UTF-8 mit Markierung (damit andere Programme Akzentbuchstaben richtig übernehmen), UTF-8 ohne Markierung, UTF-16 oder eine Codepage wie Westeuropäisch (1252), die ISO-8859-Codepages, Mac Roman sowie die japanischen, chinesischen, koreanischen, hebräischen, arabischen, baltischen, vietnamesischen und thailändischen Codepages. Ein Zeichen, für das die Codepage kein Byte hat, wird als Fragezeichen geschrieben, und die Statusleiste zeigt an, wie viele es sind.
- **Zeilenenden.** Windows (Wagenrücklauf und Zeilenvorschub), Linux und Mac (Zeilenvorschub) oder alter Mac (Wagenrücklauf).
- **Zellen.** **Wie in den Zellen angezeigt** schreibt, was Sie sehen. **Wie gespeichert** schreibt jede Ziffer einer Zahl (0,1 und nicht 10 %) und ein Datum als ISO-Text wie 2026-03-04.

Text steht in Anführungszeichen, wenn er das Trennzeichen, ein Anführungszeichen, einen Zeilenumbruch oder ein Leerzeichen am Anfang oder Ende enthält. Die anderen Blätter, die Formeln, die Formate und die Einstellungen stehen nicht in der Datei, und Ihre Arbeitsmappe wird durch das Schreiben einer solchen Datei nicht verändert.

Eine Arbeitsmappe, die aus einer Textdatei stammt, wird in der Form zurückgeschrieben, die die Datei hatte: mit demselben Trennzeichen, denselben Zeilenenden und derselben Codierung. Wenn Sie sie als Kopie speichern, ohne eine Zelle zu ändern, ist die Kopie dieselbe Datei, Byte für Byte, und es erfolgt keine Nachfrage.

## Kopie als .xls oder .ods speichern

Um eine Kopie zu erstellen, die ältere Programme oder Programme für OpenDocument öffnen können, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie die Dateiart **.xls** oder **.ods**. Die Kopie wird aus Ihrer Arbeitsmappe in ihrem jetzigen Zustand erstellt, mit den Änderungen, die Sie noch nicht gespeichert haben, und enthält:

- die Werte mit ihren Texten, Zahlen, Wahrheitswerten und Fehlern;
- die Formeln mit den Ergebnissen, die sie jetzt haben (eine Matrixformel bleibt eine Matrixformel);
- die Zahlenformate, Schriftarten, Füllungen, Rahmen, Ausrichtungen und ob eine Zelle gesperrt ist;
- verbundene Zellen, Spaltenbreiten und Zeilenhöhen, ausgeblendete Zeilen und Spalten, Gliederungen von Zeilen und Spalten sowie fixierte Bereiche;
- die Blätter in ihrer Reihenfolge mit ihren Namen und welche ausgeblendet sind, die Namen, die Sie festgelegt haben (und die Druckbereiche);
- die Verknüpfungen und wie jedes Blatt gedruckt wird (das Papier, die Ränder, die Ausrichtung, die Skalierung, Kopf- und Fußzeilen sowie die Seitenumbrüche).

Eine **.xls**-Datei enthält außerdem Datentabellen und behält die Berechnungseinstellungen der Arbeitsmappe. Eine Kopie kann aber nicht alles enthalten. Bevor JeffCrunch die Datei schreibt, nennt es, was Ihre Arbeitsmappe enthält, das die Datei nicht aufnehmen kann: Diagramme, Bilder und andere Zeichnungen, Notizen, Pivot-Tabellen, Tabellen, bedingte Formatierungen, Datenüberprüfungen, Filter, Blattschutz, Formularsteuerelemente, Makros und Verknüpfungen zu anderen Arbeitsmappen (die Formeln, die sie verwenden, behalten die Werte, die sie hatten). Eine Formel, die das ältere Format nicht darstellen kann (eine Funktion, die es nicht kennt, oder eine Zelle hinter Spalte IV oder Zeile 65.536), wird als der Wert geschrieben, den sie ergab, und die Meldung zählt sie. Das ältere Format endet außerdem bei 65.536 Zeilen, 256 Spalten, 255 Blättern und 32.767 Zeichen in einer Zelle sowie bei etwa 4.000 Zellformaten und 56 Farben: Was nicht hineinpasst, wird abgeschnitten, und die Meldung sagt, wie viel. Anschließend können Sie **Kopie speichern**, **Als .xlsx speichern** (um alles zu behalten) oder **Abbrechen** wählen.

Ihre Arbeitsmappe wird durch die Kopie nicht verändert und nicht als gespeichert markiert. Die Datei, die Sie geöffnet haben, wird nie geschrieben: Geben Sie der Kopie einen anderen Namen. Eine Kopie einer geöffneten **.xls**- oder **.ods**-Datei kann in der jeweils anderen Art gespeichert werden.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**, und wählen Sie einen Namen. JeffCrunch schreibt die Arbeitsmappe in diese Datei, mit den Änderungen, die Sie noch nicht gespeichert haben, und Sie arbeiten in der Arbeitsmappe weiter, die Sie hatten. Ihr Name und Ihre nicht gespeicherten Änderungen bleiben, wie sie waren. Sie können jede Dateiart wählen, die **Speichern unter** anbietet. Eine Kopie wird nie über die eigene Datei der Arbeitsmappe geschrieben.

## Sicherungskopie anlegen

Klicken Sie auf **Datei > Optionen**, und aktivieren Sie auf der Seite **Allgemein** die Option **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffCrunch zuerst die alte Datei in eine Datei mit dem Namen „Sicherungskopie von“ und dem ursprünglichen Namen im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne sie gespeichert werden soll. Die Antwort lautet Nein, sofern Sie nicht Ja sagen, und die Datei bleibt, wie sie war.

## Als Vorlage speichern

Eine Vorlage ist eine Arbeitsmappe, die Sie als Ausgangspunkt verwenden. Sie kann Formeln, Formatvorlagen und Blätter enthalten. Klicken Sie auf **Datei > Als Vorlage speichern**, und geben Sie einen Namen ein. Die Vorlage (eine **.xltx**-Datei) wird unter **Meine Vorlagen** abgelegt, und Ihre Arbeitsmappe bleibt unverändert. Im Dialog **Speichern unter** können Sie auch **Vorlagen (*.xltx)** oder **OpenDocument-Tabellenvorlagen (*.ots)** wählen, um eine Vorlagendatei an einem beliebigen Ort zu speichern.

Um eine Arbeitsmappe aus Ihrer Vorlage zu beginnen, klicken Sie auf **Datei > Neu**. Ihre Vorlagen sind unter **Meine Vorlagen** aufgelistet. Klicken Sie auf eine davon. Es öffnet sich eine neue Arbeitsmappe mit den Zellen und Blättern der Vorlage. Sie hat noch keine eigene Datei, daher fragt **Speichern** nach einem Namen, und die Vorlage bleibt unverändert. Um eine Vorlage aus der Liste zu entfernen, klicken Sie mit der rechten Maustaste darauf und wählen **Aus Meine Vorlagen entfernen**.

## Andere Möglichkeiten, eine Datei zu öffnen

Auf der Seite **Öffnen** klicken Sie auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste einen Namen in der Liste der zuletzt verwendeten Dateien anklicken.

- **Schreibgeschützt öffnen** öffnet die Arbeitsmappe nur zum Lesen. Sie können ansehen, blättern, suchen und kopieren, aber keine Zelle ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffCrunch nach einem neuen Namen, damit die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Arbeitsmappe ohne eigene Datei. **Speichern** fragt, wo sie abgelegt werden soll. Die geöffnete Datei wird nie verändert.
- **In neuem Fenster öffnen** öffnet die Arbeitsmappe in einem eigenen Fenster, auch wenn das aktuelle Fenster leer ist.
- **Öffnen und reparieren** dient für eine beschädigte .xlsx- oder .xls-Datei. Siehe unten.

## Öffnen und reparieren

Manchmal ist eine .xlsx-Datei beschädigt, etwa wenn ein Download auf halbem Weg abbricht oder eine Festplatte Fehler gemacht hat. Eine .xlsx-Datei ist ein Bündel aus kleineren Teilen, etwa den Blättern, den Formatvorlagen und dem gemeinsamen Text. **Öffnen und reparieren** liest jeden Teil, der sich lesen lässt. Ein Teil, der mitten im Vorgang abbrach, wird bis zu der Stelle übernommen, an der er endete; daher erhalten Sie möglicherweise nur die ersten Zeilen eines Blatts. Ein Teil, der sich nicht lesen lässt, wird weggelassen. Fehlen die Liste der Teile oder die Verbindungen zwischen ihnen, erstellt JeffCrunch sie neu.

Die reparierte Arbeitsmappe öffnet sich ohne eigene Datei und mit einem Namen wie „Reparierte Budget“. Eine Meldung nennt, was bis zu einem Bruch übernommen wurde und was weggelassen wurde. Die beschädigte Datei wird nicht verändert. Speichern Sie die reparierte Arbeitsmappe unter einem neuen Namen, und prüfen Sie die Zahlen, bevor Sie ihnen vertrauen.

### Eine .xls-Datei

Auch eine .xls-Datei kann beschädigt sein. Ist ihre äußere Hülle abgeschnitten, sucht **Öffnen und reparieren** in der Datei nach dem Anfang der Arbeitsmappe und liest von dort an. Ein Blatt, das abgeschnitten wurde, behält die Zellen vor dem Schnitt. Was nach dem Schnitt kam, ist verloren, und Sie werden darauf hingewiesen. Die reparierte Arbeitsmappe öffnet sich als Kopie, daher speichern Sie sie unter einem neuen Namen.

## Große Arbeitsmappen

JeffCrunch liest eine Arbeitsmappe im Hintergrund. Die ersten Zeilen des Blatts, das beim Speichern der Arbeitsmappe geöffnet war, erscheinen fast sofort, auch wenn die Arbeitsmappe sehr groß ist. Die Statusleiste unten links zeigt an, was das Programm tut: **Öffnen**, **Lesen** eines Blatts mit der Anzahl der bisher gelesenen Zellen, **Berechnen** mit einem Prozentwert und dann **Bereit**.

Während des Lesens können Sie blättern, auswählen und kopieren. Die Registerkarten der anderen Blätter werden aktiviert, sobald sie gelesen sind. Eine beschädigte Arbeitsmappe wird so weit geöffnet, wie sie sich lesen lässt, und eine Meldung sagt, was weggelassen wurde.

## Wenn sich eine Datei nicht öffnen lässt

Kann JeffCrunch eine Datei nicht öffnen, nennt es den Grund in einfachen Worten und lässt die geöffneten Arbeitsmappen, wie sie sind. Es unterscheidet eine leere Datei von einer, die keine Arbeitsmappe ist (etwa ein PDF-Dokument, ein Bild, ein Textdokument oder eine Präsentation); ein ZIP-Paket, das abgeschnitten oder beschädigt ist, was meist bedeutet, dass ein Download oder Kopiervorgang unterbrochen wurde, sodass Sie die Datei aus dem Original erneut kopieren sollten; eine Arbeitsmappe, die mit einem Kennwort geschützt ist, wonach es fragt (siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password)); und eine zusammengesetzte Datei, die keine Arbeitsmappe enthält.

## Zuletzt verwendete Arbeitsmappen

Die Seite **Öffnen** im Dateibereich listet die Arbeitsmappen auf, die Sie zuletzt geöffnet haben. Klicken Sie mit der rechten Maustaste auf eine davon, um sie **oben in der Liste anzuheften** oder aus der Liste zu entfernen.

## Speichern

Drücken Sie **{key:Save}**, oder klicken Sie auf **Datei** und dann auf **Speichern**, um Ihre Änderungen in der geöffneten Arbeitsmappe zu speichern. Um das Original zu behalten und eine neue Datei anzulegen, klicken Sie auf **Datei** und dann auf **Speichern unter**, und wählen Sie einen Namen. Eine Arbeitsmappe mit Makros wird als **.xlsm**-Datei gespeichert, eine Vorlage als **.xltx**- oder **.xltm**-Datei und jede andere als **.xlsx**-Datei. Ein Name, der auf eine andere Art von Arbeitsmappe endet, wird nicht akzeptiert, und die Meldung sagt, welche Endung zu verwenden ist; die Endungen **.xls**, **.ods**, **.csv**, **.tsv** und **.txt** erstellen Kopien, wie oben beschrieben.

JeffCrunch speichert nur, was Sie geändert haben. Alles andere in der Datei, etwa die anderen Blätter, die Diagramme, die Bilder, die Makros und die Einstellungen, die JeffCrunch nicht anzeigt, wird Byte für Byte so kopiert, wie es war. In einem geänderten Blatt werden nur die Zellen neu geschrieben, die Sie geändert haben, zusammen mit den Formeln, die von ihnen abhängen und deren neue Werte erhalten. Haben Sie keine Zelle geändert, bleibt die Datei unberührt.

Hat ein anderes Programm die Datei verändert, nachdem Sie sie geöffnet haben, speichert JeffCrunch nicht ohne Nachfrage darüber, denn dabei gingen die Änderungen des anderen Programms verloren. Das Programm fragt, ob Sie **Kopie speichern**, die Datei **überschreiben** oder **Abbrechen** möchten.

Die neue Datei wird zuerst neben der alten geschrieben und ersetzt die alte erst, wenn sie vollständig ist. Tritt ein Fehler auf, erhalten Sie eine Meldung, und die vorhandene Datei bleibt, wie sie war.

Sind die Formeln noch in Berechnung, wenn Sie speichern, wartet JeffCrunch, bis sie fertig sind, damit die Datei ihre Ergebnisse enthält. Drücken Sie in der Zwischenzeit **Esc**, um die Berechnung anzuhalten, wird das Speichern abgebrochen, und die Statusleiste meldet es; speichern Sie erneut, wenn Sie so weit sind. JeffCrunch lässt eines aus der Datei weg: die Liste der Reihenfolge, in der ein Programm die Formeln zuletzt berechnet hat (die Berechnungskette). Ein Programm, das die Arbeitsmappe als Nächstes öffnet, erstellt sie neu. Konnte JeffCrunch manche Formeln nicht berechnen oder haben Sie die Berechnung angehalten, bittet die Datei das öffnende Programm, alle Formeln beim Öffnen der Arbeitsmappe neu zu berechnen.

## Schließen mit Änderungen

Wenn Sie eine Arbeitsmappe mit ungespeicherten Änderungen schließen, fragt JeffCrunch, ob Sie diese **Speichern**, **Nicht speichern** oder ob Sie **Abbrechen** und weiterarbeiten möchten. Ein Sternchen (*) im Titel des Fensters zeigt an, dass es ungespeicherte Änderungen gibt.

## Integrierte Vorlagen

Unter **Datei > Neu** finden Sie unterhalb der leeren Arbeitsmappe fertige Arbeitsmappen in Gruppen: **Finanzen** (ein monatliches Budget, ein Tilgungsplan für einen Kredit und ein Kassenbuch), **Geschäft** (eine Rechnung, eine Spesenabrechnung, ein Stundenzettel, eine Bestandsliste, ein Fahrtenbuch und ein Verkaufsbericht), **Planung** (ein Kalender und ein Projektplan) und **Bildung** (ein Notenbuch). Jede zeigt ein kleines Bild ihres Kopfbereichs. Klicken Sie auf eine, um eine neue Arbeitsmappe daraus zu beginnen. Tragen Sie Ihre Werte in die gelben Felder ein. Die übrigen Zahlen werden für Sie berechnet. Die Arbeitsmappe hat keine eigene Datei, bis Sie sie speichern.
