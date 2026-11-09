# Entwurf prüfen
<!-- keywords: Entwurfsprüfung, prüfen, Fehler, Probleme, niedrige Auflösung, Überlauf, Text passt nicht, leeres Textfeld, außerhalb der Seite, Preflight, Endprüfung, Korrekturabzug, verknüpftes Bild, fehlende Schriftart, Druckprüfung -->

Die **Entwurfsprüfung** durchsucht Ihre Publikation nach Stellen, an denen häufig etwas schiefgeht. Sie findet Text, der nicht passt, leere Felder, Objekte, die über die Seite hinausragen, und Bilder, die schlecht gedruckt werden. Führen Sie sie aus, bevor Sie drucken oder eine Datei an eine Druckerei senden.

## Entwurfsprüfung ausführen

Klicken Sie auf **Überprüfen > Prüfen > Entwurfsprüfung ausführen**. Der Bereich **Entwurfsprüfung** wird rechts geöffnet. Sie können auch auf **Datei > Info** und dann auf **Entwurfsprüfung ausführen** klicken.

Der Bereich enthält zwei Kontrollkästchen, die zu Beginn beide aktiviert sind:

- **Allgemeine Entwurfsprüfungen ausführen** sucht nach Layoutproblemen.
- **Abschließende Veröffentlichungsprüfungen ausführen** sucht nach Dingen, die beim Drucken wichtig sind.

Lassen Sie beide aktiviert, um alle Arten von Problemen zu sehen. Die Liste wird während der Arbeit aktualisiert. Wenn nichts zu korrigieren ist, steht dort „Keine Probleme gefunden.“

## Ein Problem beheben

1. Klicken Sie in der Liste unter **Wählen Sie ein Element zum Korrigieren aus** auf ein Element.
2. Klicken Sie auf **Gehe zu Element**. Oder doppelklicken Sie auf das Element.

JeffPub springt zur Seite und markiert das Objekt. Korrigieren Sie es, dann verschwindet das Element aus der Liste.

## Was gefunden wird

| Meldung | Bedeutung | Was zu tun ist |
|---|---|---|
| Seite hat keinen Inhalt | Die Seite ist leer. | Fügen Sie etwas hinzu, oder löschen Sie die Seite. |
| Objekt liegt teilweise außerhalb der Seite | Ein Teil des Objekts liegt außerhalb der Seite. | Verschieben Sie es, oder lassen Sie es, wenn es ein Anschnitt ist. |
| Textfeld ist leer | Ein Textfeld enthält keinen Text. | Geben Sie Text ein, oder löschen Sie das Feld. |
| Textabschnitt mit Text im Überlauf | Ein Textabschnitt ist der Text, der durch verknüpfte Felder läuft. Dieser hier hat mehr Text, als passt. | Vergrößern Sie ein Feld, kürzen Sie den Text, oder [verknüpfen Sie ein weiteres Feld](linked-text). |
| Text wurde zum Anpassen stark verkleinert | Der Text wurde so stark verkleinert, dass er weniger als halb so groß ist. | Vergrößern Sie das Feld, oder verwenden Sie weniger Text. |
| Bildplatzhalter ist leer | Ein Bildrahmen enthält kein Bild. | Klicken Sie darauf, um ein Bild einzufügen, oder löschen Sie den Rahmen. |
| Bild ist nicht proportional skaliert | Das Bild ist gestreckt oder gestaucht. | Ändern Sie die Größe, damit das Bild seine Form zurückerhält. |
| Bild ist verknüpft, nicht eingebettet | Das Bild ist an eine Datei gebunden, und in der Publikation ist nur eine kleine Vorschau gespeichert. Ein mit **Einfügen und verknüpfen** eingefügtes Bild wird vollständig gespeichert und wird nicht aufgeführt. | Bewahren Sie die Originaldatei zusammen mit Ihrer Publikation auf, oder klicken Sie im [Grafik-Manager](graphics-manager) auf **Bild einbetten**. |
| Verknüpftes Bild fehlt | Die Datei, auf die ein verknüpftes Bild verweist, ist nicht mehr vorhanden. Die Seite zeigt daher nur die kleine Vorschau, und genau die würde gedruckt. | Klicken Sie im Grafik-Manager auf **Verknüpfung ändern…**, um die Datei zu suchen. Oder stellen Sie die Datei wieder her und klicken Sie auf **Verknüpfung aktualisieren**. |
| Verknüpftes Bild zeigt bis zur Aktualisierung der Verknüpfung nur die Vorschau | Das Bild ist mit einer Datei außerhalb des Ordners der Publikation verknüpft, die JeffPub nicht von selbst öffnet. Die Seite zeigt die kleine Vorschau, und genau die würde gedruckt. | Wählen Sie im Grafik-Manager das Bild aus, und klicken Sie auf **Verknüpfung aktualisieren**. |
| Bild hat niedrige Auflösung | Das Bild hat in seiner gedruckten Größe weniger als 150 ppi (Pixel pro Zoll). | Verwenden Sie ein größeres Bild, oder drucken Sie es kleiner. |
| Objekt hat Transparenz oder Effekte (wird möglicherweise anders gedruckt) | Transparente Bereiche und Effekte können auf Papier anders aussehen. | Drucken Sie eine Testseite. |
| Schriftart „Name“ ist nicht installiert | Eine Schriftart in Ihrem Text ist auf diesem Computer nicht vorhanden, daher wird eine andere verwendet. | Installieren Sie die Schriftart, oder wählen Sie eine andere. |

Jede Meldung endet mit der Seitenzahl, z. B. „(Seite 2)“.

## Vor einer Datei für die Druckerei

Wenn Sie eine PDF/X-Datei erstellen, warnt JeffPub Sie vor Bildern mit niedriger Auflösung und fehlenden Schriftarten und bietet an, die Entwurfsprüfung zu öffnen. Siehe [PDF-Dateien und Druckereien](pdf).

## Siehe auch

- [PDF-Dateien und Druckereien](pdf)
- [Publikation drucken](printing)
- [Text zwischen Textfeldern fortlaufend anordnen](linked-text)
- [Bilder einfügen und anpassen](pictures)
- [Grafik-Manager](graphics-manager)
