# Das Dialogfeld „Seite einrichten“
<!-- keywords: seite einrichten, dialogfeld, kopfzeile, fußzeile, seitenzahlen, seitennummerierung, datum, erste seite, gerade ungerade seiten, schwarzweiß, entwurf, notizen, fehler, seitenreihenfolge, auf seite zentrieren, erste seitenzahl, codes für kopf- und fußzeile, druckeinstellungen -->

Sie öffnen es über den kleinen Pfeil in der Ecke der Gruppe **Seite einrichten** auf der Registerkarte **Seitenlayout**. Es hat vier Registerkarten. **OK** ändert das aktuelle Blatt in einem Schritt, den Sie rückgängig machen können.

## Seite

- **Ausrichtung des Papiers**: Hochformat oder Querformat.
- **Druckgröße der Zellen**: ein Prozentsatz der Originalgröße oder **Verkleinern, bis es passt**, mit der Anzahl der Seiten in der Breite und in der Höhe. Mit „beliebig viele“ kann diese Richtung so viele Seiten belegen wie nötig.
- **Papier**: das Papierformat.
- **Seitennummerierung beginnen bei**: die Nummer der ersten Seite. Ohne diese Angabe ist die erste Seite die 1, und beim Drucken der gesamten Arbeitsmappe werden die Seiten über die Blätter hinweg fortlaufend nummeriert.

## Seitenränder

Geben Sie den Abstand um die Zellen ein und wie weit Kopf- und Fußzeile vom oberen und unteren Papierrand entfernt sind. Die Felder verwenden Zentimeter, wenn Ihr Computer auf eine Region mit metrischem System eingestellt ist, sonst Zoll. **Horizontal** und **Vertikal** setzen die Zellen in die Mitte der Seite.

## Kopf- und Fußzeile

Eine **Kopfzeile** ist eine Textzeile oben auf jeder Seite, eine **Fußzeile** eine Textzeile unten. Wählen Sie eine aus der Liste aus, oder wählen Sie **Kopfzeile bearbeiten** oder **Fußzeile bearbeiten**, um eine eigene zu schreiben. Jede hat einen linken, einen mittleren und einen rechten Bereich sowie Schaltflächen, die die Seitenzahl, die Anzahl der Seiten, das Datum, die Uhrzeit, den Dateinamen, den Ordner und den Blattnamen einfügen. Diese ändern sich von Seite zu Seite und werden daher erst beim Drucken der Seite ermittelt. Um ein Bild in einen Bereich einzufügen, klicken Sie in diesen Bereich, wählen Sie **Bild…** und dann eine PNG-, JPEG-, GIF- oder BMP-Datei aus. Das Bild wird an seiner längsten Seite höchstens 5 cm groß und an der Stelle des Codes `&G` gedruckt. **Bild entfernen** entfernt es wieder. Ein Bereich enthält ein Bild, und die erste Seite sowie die geraden Seiten können jeweils eigene Bilder haben.

Sie können außerdem eine eigene Kopf- und Fußzeile für die erste Seite sowie für gerade und ungerade Seiten festlegen und bestimmen, ob die Schrift mit den Zellen vergrößert und verkleinert wird und ob Kopf- und Fußzeile an den Seitenrändern ausgerichtet werden.

Hinter den Schaltflächen stehen kurze Codes, die in der Datei gespeichert werden und die Sie auch selbst eingeben können: **&P** ist die Seitenzahl, **&N** die Anzahl der Seiten, **&D** das Datum, **&T** die Uhrzeit, **&F** der Dateiname, **&A** der Blattname und **&&** ein kaufmännisches Und-Zeichen. **&B**, **&I** und **&U** schalten Fett, Kursiv und Unterstrichen ein und aus, **&12** setzt den Schriftgrad auf 12 Punkt, und **&"Arial,Bold"** wählt eine Schriftart aus. **&L**, **&C** und **&R** leiten den linken, mittleren und rechten Bereich ein.

## Blatt

- **Zu druckende Zellen**: der Druckbereich, als Bereiche wie **A1:D20, F1:G9**. Lassen Sie das Feld leer, um alles zu drucken.
- **Diese Zeilen oben wiederholen** und **Diese Spalten links wiederholen**: die Drucktitel, z. B. **1:3** und **A:B**.
- **Außerdem drucken**: die Linien zwischen den Zellen, die Zeilennummern und Spaltenbuchstaben, nur schwarzweiß (keine Farben oder Füllungen) und ein schneller Entwurf (keine Linien zwischen den Zellen und keine Bilder, Diagramme oder Formen).
- **Notizen** und **Zellen mit Fehleranzeige**: wie mit ihnen verfahren wird. Die Einstellung für Fehler ändert, was auf der Seite erscheint: so, wie die Zelle es anzeigt, nichts, zwei Bindestriche oder #NV. Notizen bleiben für andere Programme in der Datei erhalten, JeffCrunch liest oder druckt sie aber noch nicht.
- **Welche Seite als Nächstes kommt**: erst nach unten und dann nach rechts oder erst nach rechts und dann nach unten.
