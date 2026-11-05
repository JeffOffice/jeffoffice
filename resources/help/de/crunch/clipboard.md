# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, zwischenablage, verschieben, duplizieren, inhalte einfügen, werte, formeln, formate, transponieren, leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, operation, verbundene zellen, text, tabstopp, html, webseite, anderes programm, Esc, laufende Ameisen, rahmen, spaltenbreiten, notizen, kommentare, zahlenformate, alle außer rahmen, bild, bitmap, als bild kopieren -->

**Kopieren** (**{key:Strg+C}**) legt eine Kopie der markierten Zellen in die Zwischenablage, einen Speicher, den der Computer für jeweils eine Sache vorhält. **Einfügen** (**{key:Strg+V}**) setzt den Inhalt der Zwischenablage ab der markierten Zelle in die Zellen. **Ausschneiden** (**{key:Strg+X}**) funktioniert wie Kopieren, doch beim Einfügen werden die Zellen verschoben, und der alte Platz wird geleert.

Ein wanderndes gestricheltes Rahmenlinie zeigt die Zellen, die kopiert oder ausgeschnitten wurden. Drücken Sie **Esc**, um die Rahmenlinie zu entfernen; die Zellen sind dann nicht mehr zum Einfügen bereit. Ausgeschnittene Zellen werden auch dann freigegeben, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig): Schneiden Sie sie erneut aus, wenn Sie sie weiterhin verschieben möchten.

## Innerhalb von JeffCrunch einfügen

Alles wird mitkopiert: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel verweist weiterhin auf dieselben Stellen *relativ zu ihrer Position*. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit einem Dollarzeichen, etwa **$A$1**, bleibt unverändert. Markieren Sie einen Bereich, der ein ganzzahliges Vielfaches der kopierten Größe hat, wird die Kopie wiederholt, bis er gefüllt ist: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen, fügen Sie ein, und alle zwanzig erhalten den Inhalt.

Eine eingefügte Zahl, die für eine Spalte mit noch ursprünglicher Breite zu breit ist, verbreitert die Spalte.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden sie tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, verweist nun auf ihren neuen Platz. Beispiel: B1 enthält **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie sie in E4 ein, wird B1 zu **=E4+E5**. Was in den Zellen stand, über die Sie einfügen, ist weg (mit **Rückgängig** holen Sie es zurück). Eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr vorhanden ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer verbundenen Zelle überschreiben, wird nichts verschoben: Markieren Sie die gesamte verbundene Zelle. Wenn Sie Zellen auf ein anderes Blatt ausschneiden, lesen die Formeln weiterhin die Zellen, die sie vorher gelesen haben (die Blattangabe bleibt erhalten). Nach dem Ausschneiden und Einfügen wird die Zwischenablage geleert.

## Inhalte einfügen

**{key:Strg+Alt+V}** oder der Pfeil unter **Einfügen** auf der Registerkarte **Start** lässt Sie auswählen, was eingefügt wird:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* macht aus einer Formel die Zahl oder den Text, den sie anzeigt.
- **Alles außer Rahmen**: alles, doch die Zellen behalten die Rahmen, die sie hatten.
- **Formeln und Zahlenformate** oder **Werte und Zahlenformate**: Die Formeln (bzw. Werte) kommen mit der Anzeige der Zahlen, etwa Prozent oder Datum, aber ohne Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Aussehen.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden in die Zellen übernommen, in die Sie einfügen. Kein Zellwert ändert sich.
- Eine **Operation**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Daraus werden 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Zeilen werden zu Spalten und Spalten zu Zeilen.

## Das Menü „Einfügen“ und Bilder

Der Pfeil unter **Einfügen** enthält die üblichen Auswahlmöglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Auswahlmöglichkeiten unter „Inhalte einfügen“, jeweils mit einem Klick.

Der Pfeil neben **Kopieren** bietet **Als Bild kopieren** und **Als Bitmap kopieren**. Beide legen ein Bild der markierten Zellen in die Zwischenablage, gezeichnet so, wie Sie die Zellen sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte und bleibt daher auch beim Vergrößern scharf. Fügen Sie es in JeffWrite oder JeffDeck als Bild ein. Passen die Zellen nicht in ein Bild, markieren Sie weniger Zellen.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen zusätzlich als Text (Zellen durch Tabstopps getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle aus einer Webseite oder einer anderen Tabellenkalkulation, kommt jedes Stück in eine eigene Zelle und wird so gelesen, als hätten Sie es eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Beim Kopieren aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe werden Werte, Formeln und Zahlenformate übernommen, nicht aber die übrige Formatierung.

## Was nicht eingefügt werden kann

JeffCrunch meldet es und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel**, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen reichen würde. Eine vollständige **Matrixformel** können Sie ausschneiden und verschieben (markieren Sie alle ihre Zellen); einen Teil davon, eine Datentabelle oder die Namenszeile einer Tabelle nicht.
