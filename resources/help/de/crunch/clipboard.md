# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, inhalte einfügen, werte, formeln, formate, transponieren, leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, operation, verbundene zellen, text, tabstopp, html, webseite, anderes programm, Esc, laufende ameisen, rahmen, spaltenbreiten, notizen, kommentare, zahlenformate, alles außer rahmen, bild, bitmap, als bild kopieren, gedrucktes bild, wie gedruckt, verknüpfung einfügen, hyperlink, textimport, getrennter text, text ablegen, komma, semikolon, csv, trennzeichen, standardeinfügen festlegen, standard einfügen, feste breite -->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in die Zwischenablage, einen Speicherbereich, den der Computer für jeweils eine Sache vorhält. **Einfügen** (**{key:Paste}**) fügt den Inhalt der Zwischenablage ab der markierten Zelle in die Zellen ein. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, aber beim Einfügen werden die Zellen verschoben und der alte Platz wird geleert.

Eine wandernde gestrichelte Rahmenlinie zeigt die kopierten oder ausgeschnittenen Zellen an. Drücken Sie **Esc**, um die Rahmenlinie zu entfernen; die Zellen können dann nicht mehr eingefügt werden. Ausgeschnittene Zellen werden auch dann freigegeben, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig). Wenn Sie sie trotzdem verschieben möchten, schneiden Sie sie erneut aus.

## Innerhalb von JeffCrunch einfügen

Es wird alles übernommen: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel verweist relativ zu ihrer Position weiterhin auf dieselben Stellen. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie sie in B5 ein, wird daraus **=A5*2**. Ein Teil mit Dollarzeichen, etwa **$A$1**, bleibt unverändert. Wenn Sie einen Bereich markieren, der ein ganzzahliges Vielfaches der Größe des Kopierten ist, wird die Kopie so oft wiederholt, bis der Bereich gefüllt ist. Kopieren Sie also eine Zelle, markieren Sie zwanzig Zellen und fügen Sie ein, erhalten alle zwanzig den Inhalt.

Eine eingefügte Zahl, die für eine Spalte mit noch unveränderter Ausgangsbreite zu breit ist, vergrößert die Spalte.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden sie tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, verweist danach auf ihre neue Position. Beispiel: B1 enthält **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie in E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zielzellen stand, ist weg (**Rückgängig** holt es zurück). Eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr vorhanden ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer verbundenen Zelle überschreiben, wird nichts verschoben: Markieren Sie die ganze verbundene Zelle. Schneiden Sie Zellen auf ein anderes Blatt aus, lesen die Formeln weiterhin die Zellen, die sie vorher gelesen haben (die Blattangabe steht dabei). Nach dem Ausschneiden und Einfügen wird die Zwischenablage geleert.

## Inhalte einfügen

Mit **{key:Ctrl}+Alt+V** oder dem Pfeil unter **Einfügen** auf der Registerkarte **Start** wählen Sie aus, was eingefügt werden soll:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* macht aus einer Formel die Zahl oder den Text, den sie anzeigt.
- **Alles außer Rahmen**: alles, aber die Zellen behalten ihre bisherigen Rahmen.
- **Formeln und Zahlenformate** oder **Werte und Zahlenformate**: Die Formeln (oder Werte) kommen mit der Anzeige der Zahlen, etwa Prozent oder Datum, aber ohne Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Aussehen.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden in die Zellen übernommen, in die Sie einfügen. Kein Zellwert ändert sich.
- Eine **Operation**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Es ergibt sich 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Zeilen werden zu Spalten und Spalten zu Zeilen.

## Verknüpfung einfügen, Als Hyperlink einfügen und Textimport einfügen

Drei weitere Möglichkeiten finden Sie im Pfeil unter **Einfügen**. Sie wirken auf Zellen, die Sie in derselben Arbeitsmappe kopiert haben, außer der letzten Option.

- **Verknüpfung einfügen** setzt in jede Zelle eine Formel, die auf die kopierte Zelle verweist. Beispiel: Kopieren Sie A1:B2, markieren Sie D4 und wählen Sie **Verknüpfung einfügen**: In D4 steht **=Tabelle1!$A$1**. Ändert sich A1, ändert sich D4 mit. Eine leere kopierte Zelle zeigt 0 an.
- **Als Hyperlink einfügen** setzt in jede Zelle denselben Inhalt, den die kopierte Zelle anzeigt, und macht jede davon zu einem Link auf die kopierte Zelle. Halten Sie **Strg** gedrückt und klicken Sie darauf, um zu dieser Zelle zu gelangen. Es funktioniert für bis zu 2.000 Zellen auf einmal.
- **Textimport einfügen** dient für Text aus anderen Programmen. Es öffnet dasselbe Fenster wie das Öffnen einer Textdatei: Sie legen fest, was die Teile trennt (ein Komma, ein Tabstopp, ein Leerzeichen, ein Semikolon, ein Strich oder ein anderes Zeichen), womit ein Teil in Anführungszeichen gesetzt ist, welche Zeile als erste verwendet wird und wie jede Spalte gelesen wird (wie eingegeben, als Text, als Datum oder ausgelassen). Manche Texte richten ihre Teile in Spalten gleicher Breite aus, mit Leerzeichen dazwischen. Aktivieren Sie in diesem Fall **Die Spalten haben eine feste Breite** und geben Sie an, wo jede Spalte beginnt: Jede Zeile wird an diesen Stellen getrennt, und die Leerzeichen um jeden Teil werden entfernt. Wählen Sie *Text* für eine Spalte mit Codes wie 007, damit die Nullen erhalten bleiben. Die Teile werden ab der markierten Zelle in die Zellen eingefügt, in einem einzigen Schritt, den Sie rückgängig machen können.

## Text mit Kommas, Semikolons oder Spalten

Wenn der Text, den Sie aus einem anderen Programm einfügen, eindeutig eine Tabelle ist, öffnet JeffCrunch dasselbe Fenster wie beim Öffnen einer Textdatei. So können Sie prüfen, wie der Text aufgeteilt wird, bevor etwas in die Zellen gelangt. Das geschieht nur beim einfachen **Einfügen** (**{key:Paste}**), nie bei **Inhalte einfügen** oder einer seiner Auswahlmöglichkeiten. Es tritt auf, wenn der Text mindestens zwei Zeilen und keinen Tabstopp hat und entweder jede Zeile gleich viele Teile hat, mindestens drei, durch Kommas, Semikolons oder Striche getrennt, mit einem Inhalt in jeder Spalte (ein Teil in Anführungszeichen darf ein Komma enthalten); oder die Teile richten sich in Spalten gleicher Breite aus, mit denselben Leerzeichenlücken in jeder Zeile (mindestens drei Zeilen und mindestens zwei Lücken). Zahlen mit Tausenderkommas, etwa 1,234, sowie Codezeilen oder Listen, die mit einem Komma oder Semikolon enden, werden nicht als Tabelle erkannt. Das Fenster beginnt mit der Aufteilung, die für den Text erkannt wurde. Enthalten die Zahlen Kommas und sind die Teile durch Semikolons getrennt, ist das Komma das Dezimaltrennzeichen. Klicken Sie auf **OK**, um die Teile einzufügen, oder auf **Abbrechen**, um nichts einzufügen.

Beispiel: Kopieren Sie diese Zeilen aus einer E-Mail.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Fügen Sie sie mit einer dritten Spalte ein, etwa einem Preis, zeigt das Fenster drei Spalten, mit aktiviertem **Komma**. Ein Satz in einer Zeile oder Zeilen mit unterschiedlich vielen Kommas oder nur zwei Teilen pro Zeile werden als Wörter eingefügt, ohne Fenster. Text mit Tabstopps wird wie bisher direkt in die Zellen eingefügt. Um das Fenster für beliebigen Text zu öffnen, wählen Sie **Textimport einfügen** (siehe oben).

Dasselbe geschieht, wenn Sie Text aus einem anderen Programm auf das Raster ziehen und dort ablegen: Er wird an der markierten Zelle eingefügt. Ein Ablegen an einer anderen Stelle des Fensters bewirkt nichts.

## Standard einfügen festlegen

**Standardeinfügen festlegen** ist die letzte Auswahlmöglichkeit im Pfeil unter **Einfügen**. Sie bestimmt, was die Schaltfläche **Einfügen** und **{key:Paste}** beim Einfügen kopierter Zellen übernehmen: zunächst alles, oder nur die Werte, nur die Formeln, nur die Formatierung und so weiter. Beispiel: Wählen Sie *Werte*, dann erscheint eine kopierte Formel als die Zahl, die sie anzeigt. Die Auswahl bleibt für das nächste Mal erhalten. Sie ändert nichts an **Inhalte einfügen** oder den Auswahlmöglichkeiten im Menü, und Text aus anderen Programmen wird unverändert eingefügt. Wählen Sie *Alles*, um zum Ausgangszustand zurückzukehren.

## Das Einfügen-Menü und Bilder

Der Pfeil unter **Einfügen** zeigt die üblichen Auswahlmöglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Auswahlmöglichkeiten unter Inhalte einfügen, mit je einem Klick.

Der Pfeil neben **Kopieren** enthält **Als Bild kopieren** und **Als Bitmap kopieren**. Beide legen ein Bild der markierten Zellen in die Zwischenablage, so gezeichnet, wie Sie sie sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte, bleibt also beim Vergrößern scharf. **Als gedrucktes Bild kopieren** zeichnet die Zellen so, wie sie auf dem Papier erscheinen würden: Die Gitternetzlinien sind nur dann vorhanden, wenn die Seiteneinrichtung sie druckt. Fügen Sie es in JeffWrite oder JeffDeck als Grafik ein. Passen die Zellen nicht in ein Bild, markieren Sie weniger.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen auch als Text (Zellen durch Tabstopps getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle von einer Webseite oder aus einer anderen Tabellenkalkulation, kommt jeder Teil in eine eigene Zelle und wird so gelesen, als hätten Sie ihn eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Beim Kopieren aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe werden Werte, Formeln und Zahlenformate übernommen, aber nicht die übrige Formatierung.

## Was nicht eingefügt werden kann

JeffCrunch meldet es und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel**, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen reichen würde. Eine ganze **Matrixformel** kann ausgeschnitten und verschoben werden (markieren Sie alle ihre Zellen); ein Teil davon, eine Datentabelle oder die Namenszeile einer Tabelle nicht.
