# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, Inhalte einfügen, Werte, Formeln, Formate, transponieren, Leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, Vorgang, verbundene Zellen, Text, Tabstopp, HTML, Webseite, anderes Programm, Esc, Laufrahmen, Rahmen, Spaltenbreiten, Notizen, Kommentare, Zahlenformate, alles außer Rahmen, Bild, Bitmap, als Bild kopieren, gedrucktes Bild, als gedruckt, Verknüpfung einfügen, Hyperlink, Textimport, getrennter Text, Text ablegen, Komma, Semikolon, CSV, Trennzeichen, Standardeinfügen festlegen, Standard einfügen, feste Breite, Zellen einfügen, Formel, Feste Spaltenbreite -->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in der Zwischenablage ab, einem Speicher, den der Computer für jeweils eine Sache vorhält. **Einfügen** (**{key:Paste}**) fügt den Inhalt der Zwischenablage in die Zellen ein, beginnend bei der markierten Zelle. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, doch beim Einfügen werden die Zellen verschoben, und der alte Platz wird geleert.

Ein wanderndes gestricheltes Rechteck zeigt die Zellen an, die kopiert oder ausgeschnitten wurden. Drücken Sie **Esc**, um das Rechteck zu entfernen; die Zellen können dann nicht mehr eingefügt werden. Ausgeschnittene Zellen werden auch dann freigegeben, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig). Schneiden Sie die Zellen in diesem Fall erneut aus, wenn Sie sie noch verschieben möchten.

## Innerhalb von JeffCrunch einfügen

Es wird alles übernommen: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel zeigt weiterhin auf dieselben Stellen *relativ zu ihrem Ort*. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit einem Dollarzeichen, etwa **$A$1**, bleibt unverändert. Markieren Sie einen Bereich, der ein ganzzahliges Vielfaches der Größe des Kopierten ist, wird die Kopie wiederholt, um ihn zu füllen: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen und fügen Sie ein, dann erhalten alle zwanzig den Inhalt.

Ist eine eingefügte Zahl zu breit für eine Spalte, die noch ihre erste Breite hat, wird die Spalte verbreitert.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden sie tatsächlich verschoben. Jede Formel, die auf sie zeigte, verweist nun auf ihren neuen Ort. Beispiel: B1 enthält **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie sie bei E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zellen stand, ist weg (**Rückgängig** holt es zurück). Eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr vorhanden ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer solchen Zelle überschreiben, wird nichts verschoben: Markieren Sie die ganze verbundene Zelle. Schneiden Sie Zellen auf ein anderes Blatt aus, behalten die Formeln die Zellen bei, die sie vorher gelesen haben (dabei wird das Blatt angegeben). Nach dem Ausschneiden und Einfügen ist die Zwischenablage leer.

## Inhalte einfügen

Mit **Strg+Alt+V** oder dem Pfeil unter **Einfügen** auf der Registerkarte **Start** wählen Sie aus, was eingefügt werden soll:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* macht aus einer Formel die Zahl oder den Text, den sie anzeigt.
- **Alles außer Rahmen**: alles, aber die Zellen behalten ihre Rahmen.
- **Formeln und Zahlenformate** oder **Werte und Zahlenformate**: Die Formeln (bzw. Werte) werden mit der Anzeige der Zahlen übernommen, etwa Prozent oder Datum, aber nicht die Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Aussehen.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden in die Zellen übernommen, in die Sie einfügen. Kein Zellwert ändert sich.
- Ein **Vorgang**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Daraus werden 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Zeilen werden zu Spalten, Spalten zu Zeilen.

## Verknüpfung einfügen, Als Hyperlink einfügen und Textimport einfügen

Drei weitere Möglichkeiten finden Sie im Pfeil unter **Einfügen**. Sie gelten für Zellen, die Sie in derselben Arbeitsmappe kopiert haben, außer der letzten.

- **Verknüpfung einfügen** setzt in jede Zelle eine Formel, die auf die kopierte Zelle zeigt. Beispiel: Kopieren Sie A1:B2, markieren Sie D4 und wählen Sie **Verknüpfung einfügen**: D4 wird zu **=Tabelle1!$A$1**. Ändert sich A1, ändert sich D4 mit. Eine leere kopierte Zelle zeigt 0 an.
- **Als Hyperlink einfügen** setzt in jede Zelle dieselben Angaben, die die kopierte Zelle anzeigt, und macht jede davon zu einem Link auf die kopierte Zelle. Halten Sie **Strg** gedrückt und klicken Sie darauf, um zu dieser Zelle zu gelangen. Es funktioniert für bis zu 2.000 Zellen auf einmal.
- **Textimport einfügen** dient für Text aus einem anderen Programm. Es öffnet dasselbe Fenster wie das Öffnen einer Textdatei: Sie legen fest, was die Teile trennt (ein Komma, ein Tabstopp, ein Leerzeichen, ein Semikolon, ein senkrechter Strich oder etwas anderes), womit ein Teil in Anführungszeichen steht, ab welcher Zeile gelesen wird und wie jede Spalte gelesen wird (wie eingegeben, als Text, als Datum oder ausgelassen). Manche Texte richten ihre Teile in gleich breiten Spalten aus, mit Leerzeichen dazwischen. Aktivieren Sie in diesem Fall **Die Spalten haben eine feste Breite** und geben Sie an, wo jede Spalte beginnt: Jede Zeile wird an diesen Stellen getrennt, und die Leerzeichen um jedes Teil werden entfernt. Wählen Sie *Text* für eine Spalte mit Codes wie 007, damit die Nullen erhalten bleiben. Die Teile werden, beginnend bei der markierten Zelle, als ein Schritt in die Zellen eingefügt, den Sie rückgängig machen können.

## Text mit Kommas, Semikolons oder Spalten

Wenn der Text, den Sie aus einem anderen Programm einfügen, eindeutig eine Tabelle ist, öffnet JeffCrunch dasselbe Fenster wie beim Öffnen einer Textdatei. So können Sie prüfen, wie der Text geteilt wird, bevor etwas in die Zellen gelangt. Das geschieht nur beim normalen **Einfügen** (**{key:Paste}**), nie bei **Inhalte einfügen** oder einer seiner Auswahlmöglichkeiten. Es tritt auf, wenn der Text mindestens zwei Zeilen und keinen Tabstopp enthält und entweder jede Zeile gleich viele Teile hat, mindestens drei, durch Kommas, Semikolons oder senkrechte Striche getrennt, wobei in jeder Spalte etwas steht (ein Teil in Anführungszeichen darf ein Komma enthalten); oder die Teile sind in gleich breiten Spalten ausgerichtet, mit denselben Leerzeichenlücken in jeder Zeile (mindestens drei Zeilen und mindestens zwei Lücken). Zahlen mit Tausenderkommas, etwa 1,234, sowie Codezeilen oder Listen, die mit einem Komma oder Semikolon enden, werden nicht als Tabelle erkannt. Das Fenster beginnt mit der Trennweise, die für den Text erkannt wurde. Enthalten die Zahlen Kommas und sind die Teile durch Semikolons getrennt, ist das Komma das Dezimalzeichen. Klicken Sie auf **OK**, um die Teile einzufügen, oder auf **Abbrechen**, um nichts einzufügen.

Beispiel: Kopieren Sie diese Zeilen aus einer E-Mail.

    name,menge,preis
    Apfel,12,1.50
    Birne,3,0.25

Fügen Sie sie mit einer dritten Spalte ein, etwa einem Preis, zeigt das Fenster drei Spalten, und **Komma** ist aktiviert. Eine einzelne Satzzeile oder Zeilen mit unterschiedlich vielen Kommas oder nur zwei Teilen pro Zeile werden als Wörter eingefügt, ohne dass ein Fenster erscheint. Text mit Tabstopps gelangt wie bisher direkt in die Zellen. Um das Fenster für beliebigen Text zu öffnen, wählen Sie **Textimport einfügen** (siehe oben).

Dasselbe geschieht, wenn Sie Text aus einem anderen Programm ziehen und auf dem Raster ablegen: Er wird bei der markierten Zelle eingefügt. Ein Ablegen an einer anderen Stelle des Fensters bewirkt nichts.

## Standardeinfügen festlegen

**Standardeinfügen festlegen** ist die letzte Möglichkeit im Pfeil unter **Einfügen**. Sie legt fest, was die Schaltfläche **Einfügen** und **{key:Paste}** einfügen, wenn Sie Zellen kopiert haben: zunächst alles, oder nur die Werte, nur die Formeln, nur die Formatierung und so weiter. Beispiel: Wählen Sie *Werte*, dann erscheint eine kopierte Formel als die Zahl, die sie anzeigt. Die Auswahl bleibt für das nächste Mal erhalten. Sie ändert nicht **Inhalte einfügen** oder die Auswahlmöglichkeiten im Menü, und Text aus anderen Programmen wird unverändert eingefügt. Wählen Sie *Alles*, um zum Ausgangszustand zurückzukehren.

## Das Menü „Einfügen“ und Bilder

Der Pfeil unter **Einfügen** zeigt die üblichen Möglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Auswahlmöglichkeiten unter Inhalte einfügen, mit je einem Klick.

Der Pfeil neben **Kopieren** enthält **Als Bild kopieren** und **Als Bitmap kopieren**. Beide legen ein Bild der markierten Zellen in der Zwischenablage ab, so gezeichnet, wie Sie es sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte und bleibt daher auch vergrößert scharf. **Als gedrucktes Bild kopieren** zeichnet die Zellen so, wie das Papier sie zeigen würde: Die Gitternetzlinien erscheinen nur, wenn die Seiteneinrichtung sie druckt. Fügen Sie es in JeffWrite oder JeffDeck als Bild ein. Passen die Zellen nicht in ein Bild, markieren Sie weniger.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen auch als Text (Zellen durch Tabstopps getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle aus einer Webseite oder einer anderen Tabellenkalkulation, kommt jedes Teil in eine eigene Zelle und wird so gelesen, als hätten Sie es eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Beim Kopieren aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe werden Werte, Formeln und Zahlenformate übernommen, aber nicht die übrige Formatierung. Fügen Sie sie in JeffDeck oder JeffPub ein, erscheinen sie als Tabelle, die Schriftarten, Schattierungen, Rahmen und die Zahlen so behält, wie sie angezeigt werden (siehe **Zellen als Tabelle einfügen** in deren Hilfe).

## Was nicht eingefügt werden kann

JeffCrunch meldet es und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel**, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen reichen würde. Eine ganze **Matrixformel** kann ausgeschnitten und verschoben werden (markieren Sie alle ihre Zellen). Ein Teil davon, eine Datentabelle oder die Namenszeile einer Tabelle nicht.
