# Der Dialog „Seite einrichten“

<!-- keywords: Seite einrichten, Dialog, Kopfzeile, Fußzeile, Seitenzahlen, Datum, erste Seite, gerade ungerade, schwarzweiß, Entwurf, Notizen, Fehler, Seitenreihenfolge, auf Seite zentrieren, Seitenzahlen beginnen, Kopf- und Fußzeilencodes, Druckqualität, dpi, Punkte pro Zoll -->

Öffnen Sie ihn über den kleinen Pfeil in der Ecke der Gruppe **Seite einrichten** auf der Registerkarte **Seitenlayout**. Er hat vier Registerkarten. Mit **OK** wird das Blatt, auf dem Sie gerade arbeiten, geändert, und zwar als ein Schritt, den Sie rückgängig machen können.

## Seite

- **Papierausrichtung**: Hochformat oder Querformat.
- **Druckgröße der Zellen**: ein Prozentsatz der Originalgröße oder **Verkleinern, bis es passt**, mit der Anzahl der Seiten in der Breite und in der Höhe. Mit „Beliebig viele“ kann diese Richtung so viele Seiten nutzen, wie nötig sind.
- **Papierformat**: das Format des Papiers.
- **Druckqualität**: wie viele Punkte pro Zoll der Drucker angefordert bekommt. Mehr Punkte ergeben feinere Ausdrucke, manche Drucker brauchen dafür aber länger. Wenn eine Arbeitsmappe eine Zahl verlangt, die nicht in der Liste steht, behält JeffCrunch sie bei.
- **Seitenzahlen beginnen bei**: die Nummer, die die erste Seite erhält. Ohne diese Angabe ist die erste Seite die 1, und eine Arbeitsmappe, die vollständig gedruckt wird, nummeriert ihre Seiten von Blatt zu Blatt fortlaufend weiter.

## Ränder

Geben Sie den Abstand um die Zellen ein sowie den Abstand der Kopf- und Fußzeile von der oberen bzw. unteren Papierkante. Die Felder verwenden Zentimeter, wenn Ihr Computer auf eine metrische Region eingestellt ist, sonst Zoll. Mit **Horizontal auf der Seite zentrieren** und **Vertikal auf der Seite zentrieren** werden die Zellen in der Mitte der Seite platziert.

## Kopf- und Fußzeile

Eine **Kopfzeile** ist eine Textzeile oben auf jeder Seite, eine **Fußzeile** eine Zeile unten. Wählen Sie eine aus der Liste, oder klicken Sie auf **Kopfzeile bearbeiten** bzw. **Fußzeile bearbeiten**, um eigene zu schreiben. Jede besteht aus einem linken, einem mittleren und einem rechten Bereich sowie Schaltflächen, mit denen Sie Seitenzahl, Seitenanzahl, Datum, Uhrzeit, Dateiname, Ordner und Blattname einfügen. Diese Angaben ändern sich von Seite zu Seite und werden daher beim Drucken jeder Seite ermittelt. Um ein Bild in einen Bereich einzufügen, klicken Sie in diesen Bereich und wählen Sie **Bild…**, dann eine Datei im Format PNG, JPEG, GIF oder BMP. Das Bild wird an seiner längsten Seite auf höchstens 5,1 cm (2 Zoll) verkleinert und dort gedruckt, wo der Code `&G` steht. Mit **Bild entfernen** wird es wieder herausgenommen. Ein Bereich enthält nur ein Bild, und die erste Seite sowie die geraden Seiten können jeweils eigene Bilder haben.

Sie können außerdem eine andere Kopf- und Fußzeile für die erste Seite sowie für gerade und ungerade Seiten festlegen. Außerdem bestimmen Sie, ob sich die Schrift mit den Zellen vergrößert und verkleinert und ob Kopf- und Fußzeile an den Seitenrändern ausgerichtet werden.

Hinter den Schaltflächen stehen kurze Codes, die die Datei speichert. Sie können sie auch selbst eintippen: **&P** steht für die Seitenzahl, **&N** für die Anzahl der Seiten, **&D** für das Datum, **&T** für die Uhrzeit, **&F** für den Dateinamen, **&A** für den Blattnamen, und **&&** ist ein kaufmännisches Und (&). **&B**, **&I** und **&U** schalten Fettdruck, Kursivschrift und Unterstreichung ein und aus, **&12** legt die Schriftgröße auf 12 Punkt fest, und mit **&"Arial,Fett"** wählen Sie eine Schriftart. **&L**, **&C** und **&R** leiten den linken, den mittleren bzw. den rechten Bereich ein.

## Blatt

- **Zu druckende Zellen**: der Druckbereich als Bereiche wie **A1:D20, F1:G9**. Lassen Sie das Feld leer, um alles zu drucken.
- **Oben zu wiederholende Zeilen** und **Links zu wiederholende Spalten**: die Drucktitel, etwa **1:3** und **A:B**.
- **Außerdem drucken**: die Linien zwischen den Zellen, die Zeilennummern und Spaltenbuchstaben, nur schwarzweiß (ohne Farben und Füllungen) sowie der Entwurfsdruck (ohne Linien zwischen den Zellen und ohne Bilder, Diagramme oder Formen).
- **Notizen** und **Zellen mit Fehleranzeige**: Hier legen Sie fest, was mit ihnen geschieht. Die Einstellung für Fehler bestimmt, was die Seite anzeigt: wie die Zelle sie zeigt, nichts, zwei Bindestriche oder #NV. Bei Notizen gilt: **Weglassen** druckt keine Notizen. **Nach der letzten Seite auflisten** druckt jede Notiz nach der letzten Seite des Blatts, und zwar auf eigenen Seiten, die die folgenden Seitenzahlen sowie Kopf- und Fußzeile des Blatts tragen, in der Form „Zelle A2: der Autor: der Wortlaut der Notiz“, in der Reihenfolge der Zellen. **An ihrer Position anzeigen** wird für andere Programme in der Datei gespeichert, aber JeffCrunch druckt Notizen an den Zellen noch nicht.
- **Reihenfolge der Seiten**: zuerst nach unten und dann nach rechts oder zuerst nach rechts und dann nach unten.
