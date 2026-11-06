# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, Inhalte einfügen, Werte, Formeln, Formate, transponieren, Leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, Operation, verbundene Zellen, Text, Tabstopp, HTML, Webseite, anderes Programm, Escape, laufende Ameisen, Rahmen, Spaltenbreiten, Notizen, Kommentare, Zahlenformate, alle außer Rahmen, Bild, Bitmap, als Bild kopieren, gedrucktes Bild, wie gedruckt, als Verknüpfung einfügen, Hyperlink, Textimport, Text teilen, Trennzeichen, Standardeinfügen festlegen, Standard einfügen, feste Breite -->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in der Zwischenablage ab, einem Speicher, den der Computer für jeweils einen Inhalt vorhält. **Einfügen** (**{key:Paste}**) setzt den Inhalt der Zwischenablage in die Zellen ab der markierten Zelle. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, aber beim Einfügen werden die Zellen verschoben, und der alte Platz wird geleert.

Ein wanderndes gestricheltes Rahmenlinien zeigt die Zellen an, die kopiert oder ausgeschnitten wurden. Drücken Sie **Esc**, um die Rahmenlinie zu entfernen; die Zellen sind dann nicht mehr zum Einfügen bereit. Ausgeschnittene Zellen werden außerdem freigegeben, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen, Rückgängig). Schneiden Sie sie dann erneut aus, wenn Sie sie weiterhin verschieben möchten.

## Einfügen innerhalb von JeffCrunch

Es wird alles übernommen: die Werte, die Formeln, die Formate und die verbundenen Zellen. Eine Formel verweist weiterhin auf dieselben Stellen, und zwar *relativ zu ihrer Position*. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit einem Dollarzeichen, etwa **$A$1**, bleibt unverändert. Markieren Sie einen Bereich, der ein ganzzahliges Vielfaches der Größe des Kopierten ist, wird die Kopie zum Füllen wiederholt: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen, fügen Sie ein, und alle zwanzig erhalten den Inhalt.

Eine eingefügte Zahl, die für eine Spalte zu breit ist, die noch ihre erste Breite hat, vergrößert die Spalte.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden sie tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, zeigt nun auf ihren neuen Platz. Beispiel: B1 enthält **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie in E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zellen stand, ist weg (**Rückgängig** holt es zurück), und eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!**, weil die gelesene Zelle nicht mehr vorhanden ist. Wenn die ausgeschnittenen Zellen nur einen Teil einer verbundenen Zelle umfassen oder Sie nur einen Teil einer solchen überschreiben, wird nichts verschoben: Markieren Sie die ganze verbundene Zelle. Wenn Sie Zellen auf ein anderes Blatt ausschneiden, beziehen sich die Formeln weiterhin auf die Zellen, die sie zuvor gelesen haben (dabei wird das Blatt angegeben). Die Zwischenablage wird nach einem Ausschneiden und Einfügen geleert.

## Inhalte einfügen

**Strg+Alt+V** oder der Pfeil unter **Einfügen** auf der Registerkarte Start öffnet die Auswahl, was eingefügt werden soll:

- **Alles**, **Formeln**, **Werte** oder **Formate**. *Werte* wandelt eine Formel in die Zahl oder den Text um, den sie anzeigt.
- **Alles außer Rahmen**: alles, aber die Zellen behalten ihre Rahmen.
- **Formeln und Zahlenformate** oder **Werte und Zahlenformate**: Die Formeln (bzw. Werte) werden mit der Anzeige der Zahlen übernommen, etwa Prozent oder Datum, aber nicht die Farben, Schriftarten oder Rahmen. Die Zellen behalten ihr bisheriges Aussehen.
- **Spaltenbreiten**: Die Spalten, in die Sie einfügen, erhalten die Breiten der kopierten Spalten.
- **Kommentare und Notizen**: Die Notizen der kopierten Zellen werden in die Zellen übernommen, in die Sie einfügen. Kein Zellwert ändert sich.
- Eine **Operation**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Es ergibt sich 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, in die sie eingefügt werden, unverändert.
- **Transponieren**: Zeilen werden zu Spalten und Spalten zu Zeilen.

## Als Verknüpfung einfügen, Als Hyperlink einfügen und Textimport einfügen

Drei weitere Auswahlmöglichkeiten finden Sie im Pfeil unter **Einfügen**. Sie funktionieren mit Zellen, die Sie in derselben Arbeitsmappe kopiert haben, außer der letzten.

- **Als Verknüpfung einfügen** setzt in jede Zelle eine Formel, die auf die kopierte Zelle verweist. Beispiel: Kopieren Sie A1:B2, markieren Sie D4 und verwenden Sie **Als Verknüpfung einfügen**: D4 wird zu **=Tabelle1!$A$1**. Wenn sich A1 ändert, ändert sich D4 mit. Eine leere kopierte Zelle zeigt 0 an.
- **Als Hyperlink einfügen** setzt in jede Zelle dieselben Angaben, die die kopierte Zelle anzeigt, und macht jede davon zu einem Link auf die kopierte Zelle. Halten Sie **Strg** gedrückt und klicken Sie darauf, um zu dieser Zelle zu gelangen. Es funktioniert mit bis zu 2.000 Zellen auf einmal.
- **Textimport einfügen** dient für Text aus einem anderen Programm. Es öffnet dasselbe Fenster wie das Öffnen einer Textdatei: Sie legen fest, was die Teile trennt (ein Komma, ein Tabstopp, ein Leerzeichen, ein Semikolon, ein senkrechter Strich oder etwas anderes), womit ein Teil eingeschlossen ist, die erste zu verwendende Zeile und wie jede Spalte gelesen wird (wie eingegeben, als Text, als Datum oder ausgelassen). Manche Texte richten ihre Teile in gleich breiten Spalten aus, durch Leerzeichen getrennt. Aktivieren Sie in diesem Fall **Die Spalten haben eine feste Breite** und geben Sie an, wo jede Spalte beginnt: Jede Zeile wird an diesen Stellen getrennt, und die Leerzeichen um jeden Teil werden entfernt. Wählen Sie *Text* für eine Spalte mit Codes wie 007, damit die Nullen erhalten bleiben. Die Teile werden ab der markierten Zelle in die Zellen eingefügt, als ein Schritt, den Sie rückgängig machen können.

## Standardeinfügen festlegen

**Standardeinfügen festlegen** ist die letzte Auswahl im Pfeil unter **Einfügen**. Sie legt fest, was die Schaltfläche **Einfügen** und **{key:Paste}** einfügen, wenn Sie Zellen kopiert haben: zunächst alles, oder nur die Werte, nur die Formeln, nur die Formatierung und so weiter. Beispiel: Wählen Sie *Werte*, dann erscheint eine kopierte Formel als die Zahl, die sie anzeigt. Die Auswahl bleibt für das nächste Mal erhalten. Sie ändert weder **Inhalte einfügen** noch die Auswahlmöglichkeiten im Menü, und Text aus anderen Programmen wird unverändert eingefügt. Wählen Sie *Alles*, um zurückzukehren.

## Das Einfügen-Menü und Bilder

Der Pfeil unter **Einfügen** enthält die üblichen Auswahlmöglichkeiten sowie **Ohne Rahmen**, **Formeln und Zahlenformatierung**, **Werte und Zahlenformatierung**, **Spaltenbreiten der Quelle beibehalten** und **Kommentare und Notizen**. Sie entsprechen den Auswahlmöglichkeiten unter Inhalte einfügen, mit je einem Klick.

Der Pfeil neben **Kopieren** enthält **Als Bild kopieren** und **Als Bitmap kopieren**. Beide legen ein Bild der markierten Zellen in der Zwischenablage ab, so gezeichnet, wie Sie sie sehen, mit den Gitternetzlinien. Eine Bitmap hat die Größe des Bildschirms. Ein Bild hat doppelt so viele Punkte und bleibt daher beim Vergrößern scharf. **Als gedrucktes Bild kopieren** zeichnet die Zellen so, wie das Papier sie zeigen würde: Die Gitternetzlinien erscheinen nur, wenn die Seiteneinrichtung sie druckt. Fügen Sie es in JeffWrite oder JeffDeck als Grafik ein. Passen die Zellen nicht in ein Bild, markieren Sie weniger.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen außerdem als Text (Zellen durch Tabstopps getrennt, eine Zeile je Zeile) und als Tabelle für Webseiten in die Zwischenablage, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabstopps oder eine Tabelle aus einer Webseite oder einer anderen Tabellenkalkulation, kommt jeder Teil in eine eigene Zelle und wird so gelesen, als hätten Sie ihn eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Wenn Sie aus einer anderen, in JeffCrunch geöffneten Arbeitsmappe kopieren, werden Werte, Formeln und Zahlenformate übernommen, nicht aber die übrige Formatierung.

## Was nicht eingefügt werden kann

JeffCrunch teilt Ihnen mit und ändert nichts, wenn das Einfügen in einen Teil einer **Matrixformel**, über die Zeile mit den Spaltennamen einer **Tabelle**, in gesperrte Zellen eines **geschützten Blatts**, über das Ende des Blatts hinaus oder in mehr als 100.000 Zellen reichen würde. Eine vollständige **Matrixformel** kann ausgeschnitten und verschoben werden (markieren Sie alle ihre Zellen); ein Teil davon, eine Datentabelle oder die Namenszeile einer Tabelle nicht.
