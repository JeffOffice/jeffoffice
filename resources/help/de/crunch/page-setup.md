# Das Dialogfeld „Seite einrichten“
<!-- keywords: Seite einrichten, Dialogfeld, Kopfzeile, Fußzeile, Seitenzahlen, Datum, erste Seite, gerade ungerade Seiten, schwarzweiß, Entwurf, Notizen, Fehler, Seitenreihenfolge, auf Seite zentrieren, Seitennummerierung beginnen, Kopf- und Fußzeilencodes -->

Sie öffnen es über den kleinen Pfeil in der Ecke der Gruppe **Seite einrichten** auf der Registerkarte **Seitenlayout**. Es hat vier Registerkarten. **OK** ändert das aktuelle Blatt in einem einzigen Schritt, den Sie rückgängig machen können.

## Seite

- **Ausrichtung des Papiers**: Hochformat oder Querformat.
- **Druckgröße der Zellen**: ein Prozentsatz der Originalgröße oder **Verkleinern, bis es passt** mit der Anzahl der Seiten in der Breite und in der Höhe. „Beliebig viele“ lässt in dieser Richtung so viele Seiten zu wie nötig.
- **Papier**: das Papierformat.
- **Seitennummerierung beginnen bei**: die Nummer der ersten Seite. Ohne diese Angabe ist die erste Seite 1, und wenn Sie eine ganze Arbeitsmappe drucken, werden die Seiten von einem Blatt zum nächsten fortlaufend nummeriert.

## Seitenränder

Geben Sie den Abstand um die Zellen ein und wie weit Kopf- und Fußzeile vom oberen und unteren Papierrand entfernt sind. Die Felder verwenden Zentimeter, wenn Ihr Computer auf eine Region mit metrischem System eingestellt ist, andernfalls Zoll. **Horizontal** und **Vertikal** zentrieren die Zellen auf der Seite.

## Kopf- und Fußzeile

Eine **Kopfzeile** ist eine Textzeile oben auf jeder Seite, eine **Fußzeile** eine Textzeile unten. Wählen Sie eine aus der Liste aus, oder wählen Sie **Kopfzeile bearbeiten** bzw. **Fußzeile bearbeiten**, um eine eigene zu schreiben. Jede hat einen linken, einen mittleren und einen rechten Bereich sowie Schaltflächen, mit denen Sie die Seitenzahl, die Gesamtzahl der Seiten, das Datum, die Uhrzeit, den Dateinamen, den Ordner und den Blattnamen einfügen. Diese Angaben ändern sich von Seite zu Seite und werden daher erst beim Drucken der Seite ermittelt. Um ein Bild in einen Bereich einzufügen, klicken Sie in diesen Bereich, wählen Sie **Bild…** und dann eine PNG-, JPEG-, GIF- oder BMP-Datei aus. Das Bild wird an seiner längsten Seite auf höchstens 5 cm verkleinert und an der Stelle des Codes `&G` gedruckt. **Bild entfernen** nimmt es wieder heraus. Jeder Bereich enthält ein Bild, und die erste Seite und die geraden Seiten können jeweils eigene Bilder haben.

Sie können außerdem eine andere Kopf- und Fußzeile für die erste Seite sowie für gerade und ungerade Seiten festlegen und wählen, ob die Schrift mit den Zellen vergrößert und verkleinert wird und ob Kopf- und Fußzeile an den Seitenrändern ausgerichtet werden.

Hinter den Schaltflächen stehen kurze Codes, die in der Datei gespeichert werden und die Sie auch selbst eingeben können: **&P** ist die Seitenzahl, **&N** die Gesamtzahl der Seiten, **&D** das Datum, **&T** die Uhrzeit, **&F** der Dateiname, **&A** der Blattname und **&&** ein kaufmännisches Und-Zeichen. **&B**, **&I** und **&U** schalten Fett, Kursiv und Unterstrichen ein und aus, **&12** setzt den Schriftgrad auf 12 Punkt, und **&"Arial,Bold"** wählt eine Schriftart. **&L**, **&C** und **&R** leiten den linken, mittleren und rechten Bereich ein.

## Blatt

- **Zu druckende Zellen**: der Druckbereich als Zellbereiche wie **A1:D20, F1:G9**. Lassen Sie das Feld leer, um alles zu drucken.
- **Diese Zeilen oben wiederholen** und **Diese Spalten links wiederholen**: die Drucktitel, z. B. **1:3** und **A:B**.
- **Außerdem drucken**: die Linien zwischen den Zellen, die Zeilennummern und Spaltenbuchstaben, nur schwarzweiß (keine Farben oder Füllungen) und ein schneller Entwurf (keine Linien zwischen den Zellen und keine Bilder, Diagramme oder Formen).
- **Notizen** und **Zellen mit Fehleranzeige**: wie diese behandelt werden. Die Einstellung für Fehler ändert, was auf der Seite erscheint: wie in der Zelle angezeigt, nichts, zwei Bindestriche oder #NV. Notizen: **Weglassen** druckt keine Notizen. **Nach der letzten Seite auflisten** druckt jede Notiz nach der letzten Seite des Blatts auf eigenen Seiten, die die folgenden Seitenzahlen sowie die Kopf- und Fußzeile des Blatts tragen, in der Form „Zelle A2: Autor: Text der Notiz“ und in der Reihenfolge der Zellen. **An ihrer Position anzeigen** wird für andere Programme in der Datei gespeichert, JeffCrunch druckt Notizen aber noch nicht an den Zellen.
- **Welche Seite als Nächstes kommt**: erst nach unten und dann nach rechts oder erst nach rechts und dann nach unten.
