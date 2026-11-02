# Entwurf prüfen
<!-- keywords: Entwurfsprüfung, prüfen, Fehler, Probleme, niedrige Auflösung, Überlauf, Text passt nicht, leeres Textfeld, außerhalb der Seite, Preflight, Druckvorstufe, abschließende Prüfung, Korrektur, verknüpftes Bild, fehlende Schriftart -->

Die **Entwurfsprüfung** durchsucht Ihre Publikation nach häufigen Fehlerquellen. Sie findet Text, der nicht passt, leere Felder, Objekte, die über die Seite hinausragen, und Bilder, die schlecht gedruckt werden. Führen Sie sie aus, bevor Sie drucken oder eine Datei an eine Druckerei senden.

## Entwurfsprüfung ausführen

Klicken Sie auf **Überprüfen > Prüfen > Entwurfsprüfung ausführen**. Rechts wird der Bereich **Entwurfsprüfung** geöffnet. Sie können auch auf **Datei > Eigenschaften** und dann auf **Entwurfsprüfung ausführen** klicken.

Der Bereich enthält zwei Kontrollkästchen, die anfangs beide aktiviert sind:

- **Allgemeine Entwurfsprüfungen ausführen** sucht nach Layoutproblemen.
- **Abschließende Veröffentlichungsprüfungen ausführen** sucht nach Dingen, die beim Drucken wichtig sind.

Lassen Sie beide aktiviert, um alle Arten von Problemen zu sehen. Die Liste wird während der Arbeit aktualisiert. Gibt es nichts zu korrigieren, wird „Keine Probleme gefunden.“ angezeigt.

## Problem beheben

1. Klicken Sie unter **Wählen Sie ein Element zum Korrigieren aus** auf ein Element in der Liste.
2. Klicken Sie auf **Gehe zu Element**. Oder doppelklicken Sie auf das Element.

JeffPub wechselt zur Seite und markiert das Objekt. Beheben Sie das Problem, und das Element verschwindet aus der Liste.

## Was gefunden wird

| Meldung | Bedeutung | Vorgehen |
|---|---|---|
| Seite hat keinen Inhalt | Die Seite ist leer. | Fügen Sie etwas hinzu, oder löschen Sie die Seite. |
| Objekt liegt teilweise außerhalb der Seite | Ein Teil eines Objekts liegt außerhalb der Seite. | Verschieben Sie es, oder lassen Sie es so, wenn es sich um einen Anschnitt handelt. |
| Textfeld ist leer | Ein Textfeld enthält keinen Text. | Geben Sie Text ein, oder löschen Sie es. |
| Textabschnitt mit Text im Überlauf | Ein Textabschnitt ist der Text, der durch verknüpfte Felder fließt. Dieser enthält mehr Text, als hineinpasst. | Vergrößern Sie ein Feld, kürzen Sie den Text, oder [verknüpfen Sie ein weiteres Feld](linked-text). |
| Text wurde zum Anpassen stark verkleinert | Durch das Verkleinern auf die Feldgröße ist der Text auf weniger als die Hälfte seiner Größe geschrumpft. | Vergrößern Sie das Feld, oder verwenden Sie weniger Text. |
| Bildplatzhalter ist leer | Ein Bildrahmen enthält kein Bild. | Klicken Sie darauf, um ein Bild hinzuzufügen, oder löschen Sie ihn. |
| Bild ist nicht proportional skaliert | Das Bild ist gestreckt oder gestaucht. | Ändern Sie die Größe, um die ursprüngliche Form wiederherzustellen. |
| Bild ist verknüpft, nicht eingebettet | Das Bild ist mit einer Datei verknüpft, und in der Publikation ist nur eine kleine Vorschau gespeichert. Ein mit **Einfügen und verknüpfen** eingefügtes Bild wird vollständig gespeichert und nicht aufgeführt. | Bewahren Sie die Originaldatei zusammen mit der Publikation auf, oder klicken Sie im [Grafik-Manager](graphics-manager) auf **Bild einbetten**. |
| Verknüpftes Bild fehlt | Die Datei, auf die ein verknüpftes Bild verweist, ist nicht mehr vorhanden. Die Seite zeigt daher nur die kleine Vorschau, und diese würde gedruckt. | Klicken Sie im Grafik-Manager auf **Verknüpfung ändern…**, um die Datei zu suchen, oder legen Sie sie zurück, und klicken Sie auf **Verknüpfung aktualisieren**. |
| Verknüpftes Bild zeigt bis „Verknüpfung aktualisieren“ nur seine Vorschau | Das Bild ist mit einer Datei außerhalb des Publikationsordners verknüpft, die JeffPub nicht selbstständig öffnet. Die Seite zeigt die kleine Vorschau, und diese würde gedruckt. | Wählen Sie im Grafik-Manager das Bild aus, und klicken Sie auf **Verknüpfung aktualisieren**. |
| Bild hat niedrige Auflösung | Das Bild hat in seiner Druckgröße weniger als 150 ppi (Pixel pro Zoll). | Verwenden Sie ein größeres Bild, oder drucken Sie es kleiner. |
| Objekt hat Transparenz oder Effekte (wird möglicherweise anders gedruckt) | Transparente Bereiche und Effekte können auf Papier anders aussehen. | Drucken Sie eine Testseite. |
| Schriftart „Name“ ist nicht installiert | Eine Schriftart in Ihrem Text ist auf diesem Computer nicht vorhanden, daher wird eine andere verwendet. | Installieren Sie die Schriftart, oder wählen Sie eine andere. |

Jede Meldung endet mit der Seitenzahl, z. B. „(Seite 2)“.

## Vor einer Datei für die Druckerei

Wenn Sie eine PDF/X-Datei erstellen, warnt JeffPub Sie vor Bildern mit niedriger Auflösung und fehlenden Schriftarten und bietet an, die Entwurfsprüfung zu öffnen. Siehe [PDF-Dateien und Druckereien](pdf).

## Verwandte Themen

- [PDF-Dateien und Druckereien](pdf)
- [Publikation drucken](printing)
- [Text zwischen Textfeldern fließen lassen](linked-text)
- [Bilder hinzufügen und anpassen](pictures)
- [Grafik-Manager](graphics-manager)
