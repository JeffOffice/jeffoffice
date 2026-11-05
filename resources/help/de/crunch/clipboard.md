# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, inhalte einfügen, Werte, Formeln, Formate, transponieren, leere Zellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, Vorgang, verbunden, Text, Tabstopp, HTML, Webseite, anderes Programm, Esc, Laufrahmen, Rahmen, Spaltenbreiten, Notizen, Kommentare, Zahlenformate, alle außer Rahmen, Bild, Bitmap, als Bild kopieren, gedrucktes Bild, als gedruckt, Verknüpfung einfügen, Hyperlink, Textimport, Text aufteilen, Trennzeichen -->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in die Zwischenablage, einen Speicher, den der Computer jeweils für einen Inhalt bereithält. **Einfügen** (**{key:Paste}**) fügt den Inhalt der Zwischenablage ab der markierten Zelle in die Zellen ein. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, doch beim Einfügen werden die Zellen verschoben, und der alte Platz wird geleert.

Ein wanderender gestrichelter Rahmen zeigt die Zellen, die kopiert oder ausgeschnitten wurden. Drücken Sie **Esc**, um den Rahmen zu entfernen; die Zellen können dann nicht mehr eingefügt werden. Ausgeschnittene Zellen werden außerdem verworfen, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig). Schneiden Sie die Zellen in diesem Fall erneut aus, wenn Sie sie noch verschieben möchten.

## Innerhalb von JeffCrunch einfügen

Es wird alles übernommen: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel bezieht sich weiterhin auf dieselben Stellen *relativ zu ihrer Position*. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit Dollarzeichen, etwa **$A$1**, bleibt unverändert. Markieren Sie einen Bereich, der ein ganzzahliges Vielfaches der kopierten Größe hat, wird die Kopie wiederholt, bis der Bereich gefüllt ist: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen und fügen Sie ein, erhalten alle zwanzig den Inhalt.

Eine eingefügte Zahl, die für eine Spalte mit noch ursprünglicher Breite zu breit ist, verbreitert die Spalte.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden sie tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, verweist nun auf ihre neue Position. Beispiel: In B1 steht **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie sie in E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zielzellen stand, ist weg (**Rückgängig** holt es zurück). Eine Formel, die auf eine dieser Zellen zugegriffen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr vorhanden ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer solchen Zelle überschreiben, wird nichts verschoben: Markieren Sie die gesamte verbundene Zelle. Beim Ausschneiden auf ein anderes Blatt behalten die Formeln die Zellen bei, die sie vorher gelesen haben (dabei wird das Blatt angegeben). Nach einem Ausschneiden und Einfügen wird die Zwischenablage geleert.

## Inhalte einfügen

Mit **Strg+Alt+V** oder dem Pfeil unter **Einfügen** auf der Registerkarte **Start** wählen Sie aus, was eingefügt werden soll:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* wandelt eine Formel in die Zahl oder den Text um, die beziehungsweise den sie anzeigt.
- **Alles außer Rahmen**: alles, aber die Zellen behalten ihre bisherigen Rahmen.
- **Formeln und Zahlenformate** oder **Werte und Zahlenformate**: Die Formeln (oder die Werte) werden mit der Anzeige der Zahlen übernommen, etwa Prozent oder Datum, aber nicht die Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Aussehen.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden in die Zellen übernommen, in die Sie einfügen. Kein Zellwert ändert sich.
- Ein **Vorgang**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Daraus werden 50 und 100.
- **Leere Zellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Zeilen werden zu Spalten, und Spalten werden zu Zeilen.

## Verknüpfung einfügen, als Hyperlink einfügen und Textimport einfügen

Drei weitere Möglichkeiten finden Sie im Pfeil unter **Einfügen**. Sie funktionieren mit Zellen, die Sie in derselben Arbeitsmappe kopiert haben, außer der letzten.

- **Verknüpfung einfügen** fügt in jede Zelle eine Formel ein, die auf die kopierte Zelle verweist. Beispiel: Kopieren Sie A1:B2, markieren Sie D4 und wählen Sie **Verknüpfung einfügen**: D4 wird zu **=Tabelle1!$A$1**. Ändert sich A1, ändert sich auch D4. Eine leere kopierte Zelle zeigt 0 an.
- **Als Hyperlink einfügen** fügt in jede Zelle denselben Inhalt ein, den die kopierte Zelle anzeigt, und macht jede davon zu einem Link auf die kopierte Zelle. Halten Sie **Strg** gedrückt und klicken Sie darauf, um zu dieser Zelle zu gelangen. Es funktioniert mit bis zu 2.000 Zellen gleichzeitig.
- **Textimport einfügen** dient für Text aus einem anderen Programm. Es öffnet dasselbe Fenster wie das Öffnen einer Textdatei: Sie legen fest, was die Teile trennt (ein Komma, ein Tabstopp, ein Leerzeichen, ein Semikolon, ein senkrechter Strich oder ein anderes Zeichen), womit ein Teil eingefasst ist, die erste zu verwendende Zeile und wie jede Spalte gelesen werden soll (wie eingegeben, als Text, als Datum oder ausgelassen). Wählen Sie für eine Spalte mit Codes wie 007 die Option *Text*, damit die Nullen erhalten bleiben. Die Teile werden ab der markierten Zelle in die Zellen eingefügt, als ein einziger Schritt, den Sie rückgängig machen können.

## Das Einfügen-Menü und Bilder

Der Pfeil unter **Einfügen** enthält die üblichen Möglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Optionen unter Inhalte einfügen, jeweils mit einem Klick.

Der Pfeil neben **Kopieren** enthält **Als Bild kopieren** und **Als Bitmap kopieren**. Beide legen ein Bild der markierten Zellen in die Zwischenablage, so gezeichnet, wie Sie sie am Bildschirm sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte und bleibt daher beim Vergrößern scharf. **Als gedrucktes Bild kopieren** zeichnet die Zellen so, wie das Papier sie zeigen würde: Die Gitternetzlinien erscheinen nur, wenn die Seiteneinrichtung sie druckt. Fügen Sie das Bild in JeffWrite oder JeffDeck als Grafik ein. Passen die Zellen nicht in ein einziges Bild, markieren Sie weniger Zellen.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen außerdem als Text (Zellen durch Tabstopps getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle von einer Webseite oder aus einer anderen Tabellenkalkulation, kommt jeder Teil in eine eigene Zelle und wird so gelesen, als hätten Sie ihn eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Beim Kopieren aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe werden Werte, Formeln und Zahlenformate übernommen, aber nicht die übrige Formatierung.

## Was nicht eingefügt werden kann

JeffCrunch meldet es und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel**, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen reichen würde. Eine vollständige **Matrixformel** können Sie ausschneiden und verschieben (markieren Sie alle ihre Zellen); einen Teil davon, einen Datentabelle oder die Namenszeile einer Tabelle nicht.
