# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, Inhalte einfügen, Werte, Formeln, Formate, transponieren, Leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, Operation, verbundene Zellen, Text, Tabstopp, HTML, Webseite, anderes Programm, Escape, Laufrahmen, Rahmen, Spaltenbreiten, Notizen, Kommentare, Zahlenformate, alles außer Rahmen, Bild, Bitmap, als Bild kopieren, gedrucktes Bild, wie gedruckt, Verknüpfung einfügen, Hyperlink, Textimport, getrennter Text, Text ziehen, Komma, Semikolon, CSV, Trennzeichen, Standardeinfügen festlegen, Standard einfügen, feste Breite -->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in der Zwischenablage ab, einem Speicher, den der Computer für jeweils einen Inhalt vorhält. **Einfügen** (**{key:Paste}**) fügt den Inhalt der Zwischenablage ab der markierten Zelle in die Zellen ein. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, aber beim Einfügen werden die Zellen verschoben, und der alte Platz wird geleert.

Ein wanderndes gestricheltes Rechteck zeigt die kopierten oder ausgeschnittenen Zellen an. Drücken Sie **Esc**, um das Rechteck zu entfernen; die Zellen sind dann nicht mehr zum Einfügen bereit. Ausgeschnittene Zellen werden außerdem verworfen, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig). Wenn Sie sie noch verschieben möchten, schneiden Sie sie erneut aus.

## Einfügen innerhalb von JeffCrunch

Es wird alles übernommen: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel verweist weiterhin auf dieselben Stellen, relativ zu ihrer Position. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit einem Dollarzeichen, etwa **$A$1**, bleibt unverändert. Markieren Sie einen Bereich, der ein ganzzahliges Vielfaches der Größe des Kopierten ist, wird die Kopie zur Füllung wiederholt: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen, fügen Sie ein, und alle zwanzig erhalten den Inhalt.

Eine eingefügte Zahl, die zu breit für eine Spalte ist, die noch ihre ursprüngliche Breite hat, verbreitert die Spalte.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden die Zellen tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, verweist nun auf ihre neue Position. Beispiel: In B1 steht **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie sie in E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zellen stand, ist verloren (**Rückgängig** holt es zurück). Eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr da ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer verbundenen Zelle überschreiben, wird nichts verschoben: Markieren Sie dann die ganze verbundene Zelle. Schneiden Sie Zellen auf ein anderes Blatt aus, lesen die Formeln weiterhin die Zellen, die sie vorher gelesen haben (dabei wird das Blatt angegeben). Nach dem Ausschneiden und Einfügen ist die Zwischenablage leer.

## Inhalte einfügen

**Strg+Alt+V** oder der Pfeil unter **Einfügen** auf der Registerkarte Start öffnet die Auswahl, was eingefügt werden soll:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* macht aus einer Formel die Zahl oder den Text, den sie anzeigt.
- **Alles außer Rahmen**: alles, aber die Zellen behalten ihre bisherigen Rahmen.
- **Formeln und Zahlenformate** bzw. **Werte und Zahlenformate**: Die Formeln (bzw. Werte) kommen mit der Anzeige der Zahlen, etwa Prozent oder Datum, aber ohne Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Aussehen.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden an die Zellen gesetzt, in die Sie einfügen. Kein Zellwert ändert sich.
- Eine **Operation**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Es ergibt sich 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Zeilen werden zu Spalten und Spalten zu Zeilen.

## Die Schaltfläche „Einfügeoptionen“

Direkt nach dem Einfügen von in JeffCrunch kopierten Zellen erscheint neben der zuletzt eingefügten Zelle eine kleine Schaltfläche **Einfügeoptionen**. Klicken Sie darauf, um eine andere Art zu wählen, dieselben Zellen einzufügen: **Quellformatierung beibehalten** (alles), **Werte**, **Formeln**, **Formatierung**, **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** oder **Transponieren**. JeffCrunch nimmt das Einfügen zurück und führt es auf diese Weise erneut aus. Die Änderung ist ein einziger Schritt: Ein einziges **Rückgängig** nimmt das ganze Einfügen zurück, egal wie oft Sie es umgestellt haben. Beispiel: Kopieren Sie A1:B2, wobei B1 **=A1*2** enthält, und fügen Sie in F5 ein. Wählen Sie **Werte**, enthält G5 die Zahl 2 statt **=F5*2**; wählen Sie **Quellformatierung beibehalten**, ist die Formel wieder da.

Über die Tastatur öffnen Sie nach dem Einfügen das Menü der Schaltfläche, indem Sie die Taste **Strg** (auf dem Mac die Taste **Control**) antippen; mit den Pfeiltasten und **Eingabe** wählen Sie aus. Die Schaltfläche verschwindet, wenn Sie eine Zelle oder etwas anderes in der Arbeitsmappe ändern, etwas anderes kopieren, **Esc** drücken oder die zuletzt eingefügte Zelle aus dem Blickfeld scrollen (sie erscheint wieder, wenn Sie zurückscrollen). Nach dem Ausschneiden und Einfügen oder bei Text aus einem anderen Programm wird sie nicht angeboten.

## Verknüpfung einfügen, Als Hyperlink einfügen und Textimport einfügen

Drei weitere Möglichkeiten finden Sie im Pfeil unter **Einfügen**. Sie funktionieren mit Zellen, die Sie in derselben Arbeitsmappe kopiert haben, außer der letzten.

- **Verknüpfung einfügen** setzt in jede Zelle eine Formel, die auf die kopierte Zelle verweist. Beispiel: Kopieren Sie A1:B2, markieren Sie D4 und wählen Sie **Verknüpfung einfügen**: D4 wird zu **=Tabelle1!$A$1**. Ändert sich A1, ändert sich D4 mit. Eine kopierte leere Zelle zeigt 0 an.
- **Als Hyperlink einfügen** setzt in jede Zelle dieselben Wörter, die die kopierte Zelle anzeigt, und macht jede davon zu einem Link auf die kopierte Zelle. Halten Sie **Strg** gedrückt und klicken Sie darauf, um zu dieser Zelle zu gelangen. Es funktioniert für bis zu 2.000 Zellen auf einmal.
- **Textimport einfügen** dient für Text aus einem anderen Programm. Es öffnet dasselbe Fenster wie das Öffnen einer Textdatei: Sie legen fest, was die Teile trennt (ein Komma, ein Tabstopp, ein Leerzeichen, ein Semikolon, ein senkrechter Strich oder etwas anderes), womit ein Teil in Anführungszeichen steht, ab welcher Zeile gelesen wird und wie jede Spalte gelesen wird (wie eingegeben, als Text, als Datum oder ausgelassen). Manche Texte richten ihre Teile in Spalten gleicher Breite aus, mit Leerzeichen dazwischen. Aktivieren Sie dann **Die Spalten haben eine feste Breite** und geben Sie an, wo jede Spalte beginnt: Jede Zeile wird an diesen Stellen getrennt, und die Leerzeichen um jeden Teil werden entfernt. Wählen Sie *Text* für eine Spalte mit Codes wie 007, damit die Nullen erhalten bleiben. Die Teile werden ab der markierten Zelle in die Zellen eingefügt, als ein einziger Schritt, den Sie rückgängig machen können.

## Text mit Kommas, Semikolons oder Spalten

Wenn der Text, den Sie aus einem anderen Programm einfügen, eindeutig eine Tabelle ist, öffnet JeffCrunch dasselbe Fenster wie beim Öffnen einer Textdatei, damit Sie prüfen können, wie der Text getrennt wird, bevor etwas in die Zellen gelangt. Das geschieht nur beim normalen **Einfügen** (**{key:Paste}**), niemals bei **Inhalte einfügen** oder einer seiner Möglichkeiten. Es tritt auf, wenn der Text mindestens zwei Zeilen und keinen Tabstopp hat und entweder jede Zeile gleich viele Teile hat, mindestens drei, getrennt durch Kommas, Semikolons oder senkrechte Striche, wobei in jeder Spalte etwas steht (ein Teil in Anführungszeichen darf ein Komma enthalten); oder die Teile in Spalten gleicher Breite ausgerichtet sind, mit denselben Leerzeichenlücken in jeder Zeile (mindestens drei Zeilen und mindestens zwei Lücken). Zahlen mit Tausenderkommas, etwa 1,234, sowie Codezeilen oder Listen, die mit einem Komma oder Semikolon enden, werden nicht als Tabelle erkannt. Das Fenster beginnt mit der Trennung, die für den Text erkannt wurde. Haben die Zahlen Kommas und sind die Teile durch Semikolons getrennt, ist das Komma das Dezimalzeichen. Klicken Sie auf **OK**, um die Teile einzufügen, oder auf **Abbrechen**, um nichts einzufügen.

Beispiel: Kopieren Sie diese Zeilen aus einer E-Mail.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Fügen Sie sie mit einer dritten Spalte ein, etwa einem Preis, zeigt das Fenster drei Spalten, mit aktiviertem **Komma**. Ein Satz in einer Zeile oder Zeilen mit unterschiedlich vielen Kommas oder nur zwei Teilen pro Zeile werden als Wörter eingefügt, ohne Fenster. Text mit Tabstopps geht wie bisher direkt in die Zellen. Um das Fenster für beliebigen Text zu öffnen, wählen Sie **Textimport einfügen** (siehe oben).

Dasselbe geschieht, wenn Sie Text aus einem anderen Programm auf das Raster ziehen und dort ablegen: Er wird ab der markierten Zelle eingefügt. Ein Ablegen an einer anderen Stelle des Fensters bewirkt nichts.

## Standardeinfügen festlegen

**Standardeinfügen festlegen** ist die letzte Möglichkeit im Pfeil unter **Einfügen**. Sie bestimmt, was die Schaltfläche **Einfügen** und **{key:Paste}** bei kopierten Zellen einfügen: alles (zunächst), oder nur die Werte, nur die Formeln, nur die Formatierung und so weiter. Beispiel: Wählen Sie *Werte*, wird eine kopierte Formel als die Zahl eingefügt, die sie anzeigt. Die Auswahl bleibt für das nächste Mal erhalten. Sie ändert nicht **Inhalte einfügen** oder die Möglichkeiten im Menü, und Text aus anderen Programmen wird unverändert eingefügt. Wählen Sie *Alles*, um zurückzukehren.

Dieselbe Auswahl finden Sie auf der Seite **Bearbeiten** unter **Optionen**, in zwei Feldern: eines für Zellen, die Sie in dieser Arbeitsmappe kopiert haben, und eines für Zellen, die Sie in einer anderen Arbeitsmappe kopiert haben. Solange Sie für eine andere Arbeitsmappe nichts wählen, gilt die Einstellung der ersten.

## Das Einfügen-Menü und Bilder

Der Pfeil unter **Einfügen** enthält die üblichen Möglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Möglichkeiten in **Inhalte einfügen**, mit einem Klick jeweils.

Der Pfeil neben **Kopieren** bietet **Als Bild kopieren** und **Als Bitmap kopieren**. Beide legen ein Bild der markierten Zellen in der Zwischenablage ab, so gezeichnet, wie Sie sie sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte und bleibt daher auch beim Vergrößern scharf. **Als gedrucktes Bild kopieren** zeichnet die Zellen so, wie das Papier sie zeigen würde: Die Gitternetzlinien sind nur dann vorhanden, wenn die Seiteneinrichtung sie druckt. Fügen Sie es in JeffWrite oder JeffDeck als Grafik ein. Passen die Zellen nicht in ein einziges Bild, markieren Sie weniger.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen außerdem als Text (Zellen durch Tabstopps getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle aus einer Webseite oder einer anderen Tabellenkalkulation, kommt jedes Teil in eine eigene Zelle und wird so gelesen, als hätten Sie es eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Beim Kopieren aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe werden Werte, Formeln und Zahlenformate übernommen, aber nicht die übrige Formatierung. Fügen Sie sie in JeffDeck oder JeffPub ein, landen sie als Tabelle, die Schriftarten, Schattierungen, Rahmen und die Zahlen so beibehält, wie sie angezeigt werden (siehe **Zellen als Tabelle einfügen** in deren Hilfe).

## Was nicht eingefügt werden kann

JeffCrunch meldet es und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel** reichen würde, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen. Eine ganze **Matrixformel** kann ausgeschnitten und verschoben werden (markieren Sie alle ihre Zellen); ein Teil davon, eine Datentabelle oder die Namenszeile einer Tabelle kann nicht verschoben werden.
