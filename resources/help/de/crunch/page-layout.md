# Seiten einrichten
<!-- keywords: Design, Designs, Designfarben, Designschriftarten, Farben, Schriftarten, Akzent, Überschriftenschriftart, Textschriftart, Hintergrund, Bild hinter Zellen, kacheln, Hintergrundbild, Seitenlayout, Seitenränder, Ausrichtung, Hochformat, Querformat, Papier, Größe, Druckbereich, Seitenumbruch, Umbrüche, Drucktitel, Titel, Zeilen wiederholen, an Format anpassen, auf Seite anpassen, Gitternetzlinien, Überschriften, Blattoptionen, von rechts nach links, spiegeln -->

Die Registerkarte **Seitenlayout** legt fest, wie ein Blatt beim Drucken oder beim Erstellen einer PDF-Datei auf Seiten aufgeteilt wird. Jedes Blatt behält seine eigenen Einstellungen, und sie werden in der Arbeitsmappe gespeichert. Jede Änderung ist ein Schritt, den Sie rückgängig machen können.

## Designs, Farben und Schriftarten

Eine Arbeitsmappe hat ein **Design**: zwölf Farben und zwei Schriftarten (eine für Überschriften und eine für Fließtext), die ihre Zellen, Zellenformatvorlagen, Tabellenformatvorlagen, Formen und Diagramme benennen können, statt sie zu kopieren. Wenn Sie das Design ändern, ändert sich alles, was es benennt: Eine Zelle, die mit „Akzent 1, heller 40 %“ gefüllt ist, wechselt zum neuen Akzent 1, um denselben Wert heller.

- **Designs** enthält fertige Designs, die Farben und Schriftarten gemeinsam festlegen. Mit **Nach Designs suchen** übernehmen Sie das Design aus einer Designdatei, einer Arbeitsmappe, einem Dokument oder einer Präsentation. **Aktuelles Design speichern** sichert das Design dieser Arbeitsmappe in einer Datei, und **Auf das Design der Arbeitsmappe zurücksetzen** stellt das Design wieder her, das die Arbeitsmappe beim Öffnen hatte.
- **Farben** ändert nur die zwölf Farben. Mit **Neue Designfarben erstellen** legen Sie eigene Farben fest und benennen sie; alle JeffOffice-Apps zeigen sie danach in ihren Listen an.
- **Schriftarten** ändert nur die Schriftarten für Überschriften und Fließtext. Zellen, die die Designschriftarten verwenden, folgen der Änderung; Zellen, deren Schriftart mit Namen festgelegt ist, behalten sie.

Jede Auswahl ist ein Schritt, den Sie rückgängig machen können. Die Farbschaltflächen (Füllfarbe, Schriftfarbe, Rahmen und Registerfarbe) zeigen die Designfarben in einer Reihe, darunter jeweils hellere und dunklere Varianten. Eine Farbe, die Sie dort auswählen, bleibt eine Designfarbe und ändert sich mit dem Design. Eine Farbe aus **Standardfarben** oder **Weitere Farben** bleibt so, wie Sie sie gewählt haben.

Eine neue Arbeitsmappe beginnt mit dem eigenen Design von JeffOffice. Eine Form oder ein Diagramm, die Sie hier erstellen, behält die Farben, die sie erhalten hat; eine Form oder ein Diagramm aus der Datei, die eine Designfarbe benennt, folgt dem Design. Eine Arbeitsmappe, die aus einer älteren Binärdatei oder einer OpenDocument-Datei geöffnet wurde, hat kein Design, das sich ändern ließe, daher bieten diese Schaltflächen dafür nichts an. Wenn Sie eine Arbeitsmappe als OpenDocument-Datei speichern, die keine Designs kennt, werden die Farben so geschrieben, wie sie aussehen.

## Seitenränder, Ausrichtung und Größe

- **Seitenränder** legt den freien Raum um die Zellen fest. **Standard**, **Breit** und **Schmal** sind fertige Einstellungen. **Benutzerdefinierte Seitenränder** öffnet das Dialogfeld „Seite einrichten“, in dem Sie jeden Rand eingeben und festlegen, wie weit Kopf- und Fußzeile vom Papierrand entfernt stehen.
- **Ausrichtung** ist **Hochformat** (das Papier ist höher als breit) oder **Querformat** (breiter als hoch).
- **Größe** ist das Papierformat: A4, A5, Letter, Legal und andere. **Weitere Papierformate** listet alle auf.

## Druckbereich

Standardmäßig druckt ein Blatt alles, was Inhalte hat: Werte, Formeln sowie Zellen mit sichtbarem Rahmen oder sichtbarer Füllung, außerdem jedes Diagramm, jedes Bild und jede Form. Um nur einen Teil zu drucken:

1. Markieren Sie die gewünschten Zellen.
2. Wählen Sie **Druckbereich > Druckbereich festlegen**.

Mit **Auswahl zum Druckbereich hinzufügen** nehmen Sie einen zweiten Zellbereich in den Druckbereich auf. Jeder Bereich wird auf eigenen Seiten gedruckt. Mit **Druckbereich aufheben** drucken Sie wieder alles. Zeilen oder Spalten, die Sie innerhalb des Druckbereichs einfügen oder löschen, ändern den Druckbereich mit den Zellen.

## Seitenumbrüche

Wo eine Seite endet und die nächste beginnt, wird aus Papierformat, Seitenrändern und den Größen Ihrer Zeilen und Spalten berechnet. Eine Zeile oder Spalte wird nie in der Mitte geteilt. Um eine Seite früher enden zu lassen, markieren Sie die Zelle unterhalb und rechts von der Stelle, an der der Umbruch stehen soll, und wählen Sie **Umbrüche > Seitenumbruch einfügen**. Ist eine ganze Zeile markiert, steht der Umbruch darüber; ist eine ganze Spalte markiert, steht er links davon. Mit **Seitenumbruch entfernen** nehmen Sie den Umbruch über oder links von der markierten Zelle weg, und mit **Alle Seitenumbrüche zurücksetzen** entfernen Sie alle Umbrüche, die Sie gesetzt haben. Gesetzte Seitenumbrüche werden ignoriert, solange ein Blatt auf das Format verkleinert wird.

## Die Ansicht Seitenumbrüche

**Ansicht > Seitenumbrüche** (oder die Schaltfläche unten rechts im Fenster) zeigt das Blatt in die Seiten zerlegt, auf denen es gedruckt wird. Die Zellen, die gedruckt werden, bleiben wie sie sind; alles andere ist schattiert, und über den Zellen jeder Seite steht ihre Nummer. Eine gestrichelte Linie ist ein Umbruch, den JeffCrunch berechnet hat, und eine durchgezogene Linie einer, den Sie gesetzt haben.

- **Einen Umbruch verschieben.** Ziehen Sie eine Linie zu einer anderen Zeile oder Spalte. Sie wird dort zu einem Umbruch, den Sie gesetzt haben, und die folgenden Seiten werden neu aufgeteilt.
- **Einen Umbruch entfernen.** Ziehen Sie eine Linie von den gedruckten Zellen weg, auf die Zeilennummern oder Spaltenbuchstaben.
- **Den Druckbereich ändern.** Ziehen Sie die blaue Kante der gedruckten Zellen. Wenn Sie keinen Druckbereich festgelegt haben, werden die Zellen, die ohnehin gedruckt würden, zu einem Druckbereich, dessen Kante Sie verschieben.

Jedes Ziehen ist ein Schritt des Rückgängig-Befehls. Solange ein Blatt auf das Format verkleinert wird, werden seine Umbrüche ignoriert, daher lassen sich die Linien nicht ziehen. **Normal** stellt die Zellen wieder so dar, wie sie waren. Die Ansicht, in der Sie ein Blatt verlassen haben, wird mit der Arbeitsmappe gespeichert.

## Die Ansicht Seitenlayout

**Ansicht > Seitenlayout** (oder die mittlere Schaltfläche unten rechts im Fenster) zeigt das Blatt als die Seiten, auf denen es gedruckt wird, untereinander, jede als Blatt Papier mit seinen Seitenrändern und mit einem Abstand zwischen den Seiten. Lineale laufen oben und links entlang. Kopf- und Fußzeile werden dort gezeichnet, wo sie gedruckt werden, und ebenso Bilder darin.

- **In den Zellen arbeiten.** Klicken Sie auf eine Zelle, um sie auszuwählen, ziehen Sie, um mehrere zu markieren, und verwenden Sie die Pfeiltasten, die Eingabe, F2 und einen Doppelklick wie in der Normalansicht. Was Sie eingeben, erscheint in einer Leiste über der Zelle auf ihrer Seite.
- **Über die Registerkarte Einfügen beginnen.** **Einfügen > Kopf- und Fußzeile** zeigt die Seiten (falls sie nicht angezeigt werden) und öffnet die Mitte der Kopfzeile der ersten Seite, bereit zum Schreiben. Klicken Sie auf einen anderen Teil der Kopf- oder Fußzeile, um dort zu schreiben.
- **Kopf- oder Fußzeile eingeben.** Bewegen Sie den Zeiger über den oberen oder unteren Rand einer Seite: Ihr linker, mittlerer und rechter Teil werden hervorgehoben. Klicken Sie auf einen Teil und schreiben Sie. Die Registerkarte **Kopf- und Fußzeile** erscheint mit Schaltflächen, die Seitenzahl, Seitenanzahl, Datum, Uhrzeit, Ordner und Name der Datei, Name des Blatts sowie ein Bild einfügen. Ihre Auswahlmöglichkeiten legen fest, ob die erste Seite sowie gerade und ungerade Seiten eigene Kopf- und Fußzeilen haben, und ob die Kopfzeile mit den Zellen wächst und sich an den Seitenrändern ausrichtet. **Kopfzeile** und **Fußzeile** auf dieser Registerkarte sind Galerien fertiger Vorlagen wie **Seite 1**, **Seite 1 von ?**, der Name des Blatts, der Name der Datei und das Datum. Wenn Sie eine davon wählen, wird die gesamte Kopf- oder Fußzeile der Seiten ersetzt, auf denen Sie gerade schreiben (Wörter, die Sie darin noch eingegeben haben, bleiben nicht erhalten). **(keine)** leert sie. Mit **Zu Kopfzeile wechseln** und **Zu Fußzeile wechseln** öffnen Sie die Mitte dieses Randes zum Schreiben. Drücken Sie **Strg+Eingabe** oder klicken Sie irgendwo anders, um den Vorgang abzuschließen; mit **Esc** verwerfen Sie ihn. Jede fertige Kopf- oder Fußzeile ist ein Schritt des Rückgängig-Befehls.
- **Zoomen** Sie mit dem Schieberegler oder halten Sie **Strg** gedrückt und drehen Sie das Mausrad.

Nur die Seiten im Anzeigebereich werden gezeichnet, daher öffnet sich ein Blatt mit Tausenden Seiten sofort. Die Ansicht, in der Sie ein Blatt verlassen haben, wird mit der Arbeitsmappe gespeichert.

## Drucktitel

Mit **Drucktitel** werden Zeilen oben und Spalten links auf jeder Seite nach der Seite wiederholt, auf der sie stehen, damit eine Überschriftenzeile auf jeder Seite erscheint. Tragen Sie die Zeilen (etwa **1:3**) und die Spalten (etwa **A:B**) auf der Registerkarte „Tabelle“ im Dialogfeld „Seite einrichten“ ein.

## An Format anpassen

**Breite** und **Höhe** verkleinern die Zellen, bis das Blatt nicht breiter oder nicht höher als so viele Seiten ist. **Automatisch** lässt diese Richtung so viele Seiten zu, wie nötig sind. **Skalierung** druckt in einem Prozentsatz der Originalgröße, von 10 bis 400. Sie funktioniert, solange Breite und Höhe beide auf **Automatisch** stehen. Würde das Verkleinern die Schrift sehr klein machen, weist das Fenster Drucken darauf hin.

## Blattoptionen

**Gitternetzlinien anzeigen** und **Überschriften anzeigen** ändern nur den Bildschirm. **Gitternetzlinien drucken** druckt die Linien zwischen den Zellen auf Papier, und **Überschriften drucken** druckt die Zeilennummern und Spaltenbuchstaben aus.

**Blatt von rechts nach links** lässt das Blatt von rechts aus lesen. Spalte A wandert an den rechten Rand, und die Spalten zählen nach links. Auch die Zeilennummern wandern an den rechten Rand. Verwenden Sie es für Sprachen, die von rechts nach links geschrieben werden, etwa Arabisch und Hebräisch. Es ändert nur, wie das Blatt auf dem Bildschirm aussieht, und die Auswahl wird mit dem Blatt gespeichert, sodass sie beim Öffnen der Arbeitsmappe wieder erscheint. Es ist kein Schritt, den Sie rückgängig machen können: Klicken Sie erneut darauf, um die Option auszuschalten.

## Hintergrund

Mit **Hintergrund** (in der Gruppe **Seite einrichten**) legen Sie ein Bild hinter die Zellen des Blatts. Wählen Sie ein Bild im Format PNG, JPEG, GIF oder BMP. Das Bild wird wie Kacheln wiederholt, um das Blatt zu füllen, und es wandert mit den Zellen, wenn Sie scrollen. Eine Zelle mit Füllfarbe verdeckt das Bild, daher geben Sie den Zellen, die gut lesbar sein sollen, eine Füllung. Das Bild wird nur auf dem Bildschirm angezeigt: Es wird nicht gedruckt und gelangt nicht in eine PDF-Datei.

**Hintergrund entfernen** nimmt das Bild wieder weg. Beides sind Schritte, die Sie rückgängig machen können. Das Bild wird mit dem Blatt gespeichert und erscheint beim Öffnen der Arbeitsmappe wieder. In einer .ods-Kopie wird es als Seitenhintergrund gespeichert, den andere Programme möglicherweise drucken. Eine Bilddatei, die größer als 8 MB ist oder zu viele Punkte hat, wird abgelehnt. Ebenso wird ein Bild abgelehnt, das zusammen mit allen Bildern der Arbeitsmappe (Hintergründe, Kopf- und Fußzeilen) mehr als 64 MB ergeben würde. Ein Bild mit mehr als 2.400 Punkten an einer Seite wird trotzdem in seiner tatsächlichen Größe gekachelt.

## Weitere Einstellungen

Der kleine Pfeil an der Ecke der Gruppe **Seite einrichten** öffnet das Dialogfeld „Seite einrichten“, das all diese Einstellungen und noch mehr enthält: [Das Dialogfeld „Seite einrichten“](page-setup).
