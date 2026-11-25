# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, inhalte einfügen, Werte, Formeln, Formate, transponieren, Leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, Vorgang, verbundene Zellen, Text, Tabstopp, HTML, Webseite, anderes Programm, Esc, laufende Ameisen, Rahmen, Spaltenbreiten, Notizen, Kommentare, Zahlenformate, alle außer Rahmen, Bild, Bitmap, als Bild kopieren, gedrucktes Bild, als gedruckt, Verknüpfung einfügen, Hyperlink, Textimport, getrennter Text, Text ablegen, Komma, Semikolon, CSV, Trennzeichen, Standardeinfügen festlegen, Standard einfügen, feste Breite, Einfügeoptionen, Zwischenablage-Bereich, Kopie, Tastenkürzel, Strg+C, Strg+V, Strg+X</!-->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in der Zwischenablage ab, einem Speicher, den der Computer für einen Inhalt nach dem anderen vorhält. **Einfügen** (**{key:Paste}**) fügt den Inhalt der Zwischenablage in die Zellen ein, beginnend mit der markierten Zelle. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, aber beim Einfügen werden die Zellen verschoben, und der alte Platz wird geleert.

Ein wanderndes gestricheltes Rahmenlinie zeigt die kopierten oder ausgeschnittenen Zellen an. Drücken Sie **Esc**, um den Rahmen zu entfernen; die Zellen sind dann nicht mehr zum Einfügen bereit. Ausgeschnittene Zellen werden außerdem freigegeben, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig). Schneiden Sie sie in diesem Fall erneut aus, wenn Sie sie noch verschieben möchten.

## Einfügen innerhalb von JeffCrunch

Alles wird übernommen: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel verweist weiterhin auf dieselben Stellen *relativ zu ihrer Position*. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit einem Dollarzeichen, etwa **$A$1**, bleibt unverändert. Markieren Sie einen Bereich, der ein ganzzahliges Vielfaches der Größe des Kopierten ist, wird die Kopie wiederholt, um ihn zu füllen: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen, fügen Sie ein, und alle zwanzig erhalten den Inhalt.

Eine eingefügte Zahl, die für eine Spalte mit noch ursprünglicher Breite zu breit ist, verbreitert die Spalte.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden die Zellen tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, verweist nun auf ihre neue Position. Beispiel: B1 enthält **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie sie in E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zellen stand, ist verloren (**Rückgängig** holt es zurück), und eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr da ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer solchen Zelle überschreiben, wird nichts verschoben: Markieren Sie die ganze verbundene Zelle. Wenn Sie Zellen auf ein anderes Blatt ausschneiden, lesen die Formeln weiterhin die Zellen, die sie vorher gelesen haben (sie nennen das Blatt). Die Zwischenablage wird nach dem Ausschneiden und Einfügen geleert.

## Inhalte einfügen

**Strg+Alt+V** oder der Pfeil unter **Einfügen** auf der Registerkarte **Start** lässt Sie wählen, was eingefügt wird:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* wandelt eine Formel in die Zahl oder den Text um, den sie anzeigt.
- **Alle außer Rahmen**: alles, aber die Zellen behalten ihre bisherigen Rahmen.
- **Formeln und Zahlenformate** oder **Werte und Zahlenformate**: Die Formeln (bzw. Werte) werden mit der Anzeige der Zahlen übernommen, etwa Prozent oder Datum, aber nicht mit Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Erscheinungsbild.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden in die Zellen übernommen, in die Sie einfügen. Kein Zellwert ändert sich.
- Ein **Vorgang**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Es ergibt sich 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Zeilen werden zu Spalten und Spalten zu Zeilen.

## Die Schaltfläche Einfügeoptionen

Direkt nach dem Einfügen von Zellen, die Sie in JeffCrunch kopiert haben, erscheint neben der zuletzt eingefügten Zelle eine kleine Schaltfläche **Einfügeoptionen**. Klicken Sie darauf, um eine andere Art zu wählen, dieselben Zellen einzufügen: **Quellformatierung beibehalten** (alles), **Werte**, **Formeln**, **Formatierung**, **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** oder **Transponieren**. JeffCrunch nimmt das Einfügen zurück und führt es auf diese Weise erneut aus. Die Änderung ist ein einziger Schritt: Ein einziges **Rückgängig** nimmt das gesamte Einfügen zurück, egal wie oft Sie es sich anders überlegt haben. Beispiel: Kopieren Sie A1:B2, wobei B1 **=A1*2** enthält, und fügen Sie in F5 ein. Wählen Sie **Werte**, enthält G5 die Zahl 2 statt **=F5*2**; wählen Sie **Quellformatierung beibehalten**, ist die Formel wieder da.

Über die Tastatur tippen Sie nach dem Einfügen auf die Taste **Strg** (auf dem Mac die Taste **Control**), um das Menü der Schaltfläche zu öffnen, und wählen dann mit den Pfeiltasten und **Eingabe**. Die Schaltfläche verschwindet, wenn Sie eine Zelle oder etwas anderes in der Arbeitsmappe ändern, etwas anderes kopieren, **Esc** drücken oder die zuletzt eingefügte Zelle aus dem Blickfeld scrollen (sie erscheint wieder, wenn Sie zurückscrollen). Nach einem Ausschneiden und Einfügen oder bei Text aus einem anderen Programm wird sie nicht angeboten.

## Verknüpfung einfügen, Als Hyperlink einfügen und Textimport einfügen

Drei weitere Möglichkeiten finden Sie im Pfeil unter **Einfügen**. Sie funktionieren mit Zellen, die Sie in derselben Arbeitsmappe kopiert haben, außer der letzten.

- **Verknüpfung einfügen** setzt in jede Zelle eine Formel, die auf die kopierte Zelle verweist. Beispiel: Kopieren Sie A1:B2, markieren Sie D4 und verwenden Sie **Verknüpfung einfügen**: D4 wird zu **=Sheet1!$A$1**. Wenn sich A1 ändert, ändert sich D4 mit. Eine kopierte leere Zelle zeigt 0 an.
- **Als Hyperlink einfügen** setzt in jede Zelle dieselben Wörter, die die kopierte Zelle anzeigt, und macht jede davon zu einem Link auf die kopierte Zelle. Halten Sie **Strg** gedrückt und klicken Sie darauf, um zu dieser Zelle zu gelangen. Es funktioniert mit bis zu 2.000 Zellen auf einmal.
- **Textimport einfügen** ist für Text aus einem anderen Programm gedacht. Es öffnet dasselbe Fenster wie das Öffnen einer Textdatei: Sie legen fest, was die Teile trennt (ein Komma, ein Tabstopp, ein Leerzeichen, ein Semikolon, ein Strich oder etwas anderes), womit ein Teil in Anführungszeichen steht, die erste zu verwendende Zeile und wie jede Spalte gelesen wird (wie eingegeben, als Text, als Datum oder ausgelassen). Manche Texte richten ihre Teile in Spalten gleicher Breite aus, mit Leerzeichen dazwischen. Aktivieren Sie **Die Spalten haben eine feste Breite** und geben Sie an, wo jede Spalte beginnt: Jede Zeile wird an diesen Stellen getrennt, und die Leerzeichen um jeden Teil werden entfernt. Wählen Sie *Text* für eine Spalte mit Codes wie 007, damit die Nullen erhalten bleiben. Die Teile werden beginnend mit der markierten Zelle in Zellen eingefügt, als ein einziger Schritt, den Sie rückgängig machen können.

## Text mit Kommas, Semikolons oder Spalten

Wenn der Text, den Sie aus einem anderen Programm einfügen, eindeutig eine Tabelle ist, öffnet JeffCrunch dasselbe Fenster wie beim Öffnen einer Textdatei, damit Sie prüfen können, wie der Text aufgeteilt wird, bevor etwas in die Zellen gelangt. Das geschieht nur beim einfachen **Einfügen** (**{key:Paste}**), nie bei **Inhalte einfügen** oder einer seiner Möglichkeiten. Es geschieht, wenn der Text mindestens zwei Zeilen und keinen Tabstopp hat und entweder jede Zeile gleich viele Teile hat, mindestens drei, getrennt durch Kommas, Semikolons oder Striche, wobei in jeder Spalte etwas steht (ein Teil in Anführungszeichen darf ein Komma enthalten); oder die Teile stehen in Spalten gleicher Breite mit denselben Leerzeichenlücken in jeder Zeile (mindestens drei Zeilen und mindestens zwei Lücken). Zahlen mit Tausenderkommas, etwa 1.234, sowie Codezeilen oder Listen, die auf ein Komma oder Semikolon enden, werden nicht als Tabelle erkannt. Das Fenster beginnt mit der Aufteilung, die für den Text erkannt wurde. Enthalten die Zahlen Kommas und sind die Teile durch Semikolons getrennt, ist das Komma das Dezimalzeichen. Klicken Sie auf **OK**, um die Teile einzufügen, oder auf **Abbrechen**, um nichts einzufügen.

Beispiel: Kopieren Sie diese Zeilen aus einer E-Mail.

    name,qty,price
    Apple,12,1.50
    Pear,3,0.25

Fügen Sie sie mit einer dritten Spalte, etwa einem Preis, ein, zeigt das Fenster drei Spalten mit aktiviertem **Komma**. Ein Satz in einer Zeile oder Zeilen mit unterschiedlicher Anzahl von Kommas oder nur zwei Teilen pro Zeile werden als Wörter eingefügt, ohne Fenster. Text mit Tabstopps geht direkt in die Zellen, wie bisher. Um das Fenster für irgendeinen Text zu öffnen, wählen Sie **Textimport einfügen** (siehe oben).

Dasselbe geschieht, wenn Sie Text aus einem anderen Programm ziehen und auf dem Raster ablegen: Er wird in der markierten Zelle eingefügt. Ein Ablegen an einer anderen Stelle des Fensters bewirkt nichts.

## Standardeinfügen festlegen

**Standardeinfügen festlegen** ist die letzte Möglichkeit im Pfeil unter **Einfügen**. Sie bestimmt, was die Schaltfläche **Einfügen** und **{key:Paste}** beim Einfügen kopierter Zellen einfügen: zunächst alles, oder nur die Werte, nur die Formeln, nur die Formatierung und so weiter. Beispiel: Wählen Sie *Werte*, dann landet eine kopierte Formel als die Zahl, die sie anzeigt. Die Auswahl wird für das nächste Mal gespeichert. Sie ändert weder **Inhalte einfügen** noch die Möglichkeiten im Menü, und Text aus anderen Programmen wird unverändert eingefügt. Wählen Sie *Alles*, um zurückzukehren.

Dieselbe Auswahl finden Sie auf der Seite **Bearbeiten** unter **Optionen**, in zwei Feldern: eines für Zellen, die Sie in dieser Arbeitsmappe kopiert haben, und eines für Zellen, die Sie in einer anderen Arbeitsmappe kopiert haben. Solange Sie für eine andere Arbeitsmappe nichts wählen, folgt sie der ersten Einstellung.

## Das Menü Einfügen und Bilder

Der Pfeil unter **Einfügen** enthält die üblichen Möglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Quellspaltenbreiten beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Möglichkeiten von Inhalte einfügen, mit je einem Klick.

Der Pfeil neben **Kopieren** bietet **Als Bild kopieren** und **Als Bitmap kopieren**. Jede Option legt ein Bild der markierten Zellen in der Zwischenablage ab, so gezeichnet, wie Sie es auf dem Bildschirm sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte und bleibt daher auch beim Vergrößern scharf. **Als gedrucktes Bild kopieren** zeichnet die Zellen so, wie das Papier sie zeigen würde: Die Gitternetzlinien erscheinen nur, wenn die Seiteneinrichtung sie druckt. Fügen Sie es in JeffWrite oder JeffDeck als Bild ein. Passen die Zellen nicht in ein Bild, markieren Sie weniger.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen auch als Text (Zellen durch Tabstopps getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle aus einer Webseite oder einer anderen Tabellenkalkulation, kommt jedes Teil in eine eigene Zelle und wird so gelesen, als hätten Sie es eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Wenn Sie Zellen aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe kopieren, werden Werte, Formeln und Zahlenformate übernommen, aber nicht die übrige Formatierung. Fügen Sie sie in JeffDeck oder JeffPub ein, werden sie als Tabelle eingefügt, die Schriftarten, Schattierungen, Rahmen und die Zahlen in der angezeigten Form beibehält (siehe **Zellen als Tabelle einfügen** in deren Hilfe).

## Was nicht eingefügt werden kann

JeffCrunch meldet es und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel** reichen würde, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen. Eine ganze **Matrixformel** können Sie ausschneiden und verschieben (markieren Sie alle ihre Zellen); einen Teil davon, eine Datentabelle oder die Zeile mit den Spaltennamen einer Tabelle nicht.

## Der Bereich Zwischenablage

Der Bereich **Zwischenablage** merkt sich die letzten 24 Elemente, die Sie in diesem Programmfenster kopiert oder ausgeschnitten haben, damit Sie ein älteres ohne erneutes Kopieren einfügen können. Klicken Sie auf den kleinen Pfeil in der Ecke der Gruppe **Zwischenablage** auf der Registerkarte **Start** (oder drücken Sie **Alt**, dann **H**, dann **FO**), um ihn zu öffnen. Neben der Seite erscheint ein Feld.

Jedes Element zeigt eine kleine Vorschau und die Art des Inhalts: Text, formatierter Text, Zellen, ein Bild, Formen oder Folien. Das neueste steht oben. Klicken Sie auf ein Element, um es dort einzufügen, wo sich die Einfügemarke oder die Markierung befindet. Es wird wie jedes andere Einfügen eingefügt: Ein Schritt von **Rückgängig** nimmt es zurück, und die Schaltfläche Einfügeoptionen erscheint wie üblich.

- **Alle einfügen** fügt jedes Element ein, das älteste zuerst, sodass sie in der Reihenfolge stehen, in der Sie sie kopiert haben.
- **Löschen** entfernt das gewählte Element. **Alle löschen** entfernt alle.
- Der Bereich sammelt nur, während er geöffnet ist. Damit er auch nach dem Schließen weiter sammelt, aktivieren Sie **Auch nach dem Schließen dieses Bereichs weiter sammeln**. Die Auswahl wird gespeichert, und das Sammeln beginnt mit dem Start des Programms, noch bevor Sie den Bereich öffnen.
- Um auch das zu sammeln, was Sie in anderen Programmen kopieren, aktivieren Sie **Auch Inhalte sammeln, die in anderen Programmen kopiert werden**. Es werden nur reiner Text und Bilder behalten, und die Option ist bei jedem Start des Programms wieder ausgeschaltet. Ist sie aktiv, wird eine in einem anderen Programm kopierte Auswahl vollständig gelesen, bevor dieses Programm entscheidet, ob sie behalten wird. Eine sehr große Kopie kann das Programm daher kurz anhalten lassen und so viel Speicher belegen, wie die Kopie groß ist. Es wird nur eine Form gelesen: ihr Text, wenn sie Text enthält, sonst ein Bild. Ist die Option aus, wird nichts gelesen, was in anderen Programmen kopiert wurde.

Die Elemente werden nur im Speicher des Programms gehalten. Nichts wird in einer Datei gespeichert, und die Liste wird geleert, wenn Sie das letzte Fenster des Programms schließen. Ein Element bleibt in der Liste, auch wenn das Dokument, aus dem es stammt, geschlossen wird. Ein Element, das größer als 8 MB ist, wird nicht behalten; der Bereich meldet dies. Ein Bild zählt 4 Byte für jeden Punkt (Pixel), ein Bild mit mehr als etwa 2 Millionen Pixeln ist daher zu groß. Alle Elemente zusammen sind auf 64 MB begrenzt, und die ältesten werden zuerst entfernt. Alles, was ein Passwort-Manager als geheim kennzeichnet, wird nie behalten.
