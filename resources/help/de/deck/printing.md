# Drucken, PDF und Folienbilder

<!-- keywords: drucken, Drucker, Druck, Kopien, Folien, Notizenseiten, Handzettel, Gliederung, PDF, exportieren, Bild, PNG, JPEG, JPG, Bilddatei, Bereich, ausgeblendete Folien, Rahmen, Graustufen, Schwarzweiß, Druckvorschau, Papier, A4, Kennwort, verschlüsseln, mit Kennwort schützen, Passwort -->

Sie können Ihre Präsentation drucken, als PDF speichern oder eine Folie als Bild speichern. Alle drei Funktionen finden Sie in der Ansicht „Datei“.

## Drucken

1. Drücken Sie Strg+P, oder klicken Sie auf **Datei > Drucken**.
2. Wählen Sie unter **Drucken**, was ausgegeben werden soll: **Folien**, **Notizenseiten**, **Gliederung** oder **Handzettel** mit 1, 2, 3, 4, 6 oder 9 Folien pro Seite.
3. Bei Handzetteln mit 4 oder mehr Folien legt **Reihenfolge** fest, ob die Folien zuerst quer über die Seite und dann nach unten (**Horizontal**) oder zuerst nach unten und dann quer (**Vertikal**) angeordnet werden.
4. Wählen Sie unter **Bereich** entweder **Alle Folien** oder **Aktuelle Folie**, oder wählen Sie **Folien** und geben Sie die Nummern im Feld ein: Foliennummern und Bereiche, durch Kommas getrennt, zum Beispiel 1-3, 5.
5. Aktivieren Sie bei Bedarf **Ausgeblendete Folien drucken**, **Folien mit Rahmen** (eine dünne Linie um jede Folie) und **Graustufen**.
6. Wählen Sie unter **Seiten** entweder **Einseitig drucken**, **Beidseitig drucken, an der langen Kante wenden** (Seiten, die wie ein Buch umgeblättert werden), **Beidseitig drucken, an der kurzen Kante wenden** (Seiten, die wie ein Notizblock umgeblättert werden) oder **Beidseitig manuell drucken**. Die ersten beiden Optionen sind abgeblendet, wenn Ihr Drucker nicht beidseitig drucken kann. Zeigen Sie mit der Maus darauf, um den Grund zu sehen. **Manuell** ist für solche Drucker gedacht: JeffDeck druckt zuerst die erste, dritte, fünfte usw. Seite. Danach fordert eine Meldung Sie auf, den Stapel umzudrehen und wieder in den Drucker zu legen. Wenn Sie auf **OK** klicken, druckt JeffDeck die übrigen Seiten, beginnend mit der letzten. Bei einer einzigen Seite gibt es nichts umzudrehen. JeffDeck merkt sich Ihre Auswahl.
7. Klicken Sie auf **Drucken**. JeffDeck öffnet das Druckfenster Ihres Systems, in dem Sie den Drucker und die Anzahl der Exemplare wählen. Das Fenster übernimmt zunächst Ihre Auswahl der Seitenausgabe. Eine PDF-Datei, die über dieses Fenster erstellt wird, hat keine Seitenausgabe.

Die Vorschau rechts zeigt jede Seite, die ausgegeben wird, mit Pfeilen zum Blättern. Sie wird genauso gezeichnet, wie die Seite gedruckt wird. JeffDeck merkt sich Ihre Auswahl für das nächste Mal.

## Was die einzelnen Auswahlen drucken

- **Folien** druckt eine Folie auf jede Seite, in der Form der Folie selbst. Bei einer breiten Folie wird das Papier quer eingelegt.
- **Notizenseiten** zeigt oben auf der Seite eine Folie, darunter die Sprechernotizen.
- **Handzettel** setzen auf jede Seite ein Raster aus Folien, mit einer Kopf- und Fußzeile, die Titel, Datum und Seitenzahl enthalten. Bei 3 Folien pro Seite gibt es neben jeder Folie Linien zum Schreiben.
- **Gliederung** druckt die Nummer und den Titel jeder Folie, danach den Textkörper, nach Ebenen eingerückt.

Notizenseiten, Handzettel und die Gliederung werden im Hochformat auf Letter- oder A4-Papier gedruckt, je nachdem, was in Ihrer Region üblich ist. Ausgeblendete Folien werden nur gedruckt, wenn Sie **Ausgeblendete Folien drucken** aktivieren.

## Als PDF speichern

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Ein Fenster fragt, was enthalten sein soll: dieselben Optionen **Drucken**, **Reihenfolge** und **Bereich** wie beim Drucken sowie dieselben drei Kontrollkästchen. Es zeigt an, wie viele Seiten die Datei haben wird. Klicken Sie auf **Exportieren…**, und wählen Sie dann einen Namen und einen Speicherort.

Das PDF enthält echten Text mit den eingebetteten Schriftarten und sieht daher auf jedem Computer gleich aus. Sein Titel ist der Titel der Präsentation oder, falls keiner vorhanden ist, der Dateiname.

## PDF mit einem Kennwort schützen

Klicken Sie im Fenster **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer das PDF öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei tatsächlich: Die Seiten werden mit einer starken Verschlüsselung (AES-256) verschlüsselt, und niemand kann sie ohne Kennwort lesen.

Sie können außerdem **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Programme halten sich daran, aber sie hindern niemanden, der sie umgehen möchte. Verlassen Sie sich daher nicht darauf, um eine Datei vertraulich zu halten.

Ein vergessenes Kennwort kann nicht wiederhergestellt werden. Bewahren Sie es daher an einem sicheren Ort auf. Das Kennwort gilt nur für dieses eine PDF und wird nirgends gespeichert.

Wenn Sie ein PDF mit Kennwort als Bild einfügen, fragt JeffDeck nach dem Kennwort. Ist es falsch, fragt es erneut; klicken Sie auf **Abbrechen**, um den Vorgang zu beenden. Die Folie erhält dann eine Kopie der Seite, für die kein Kennwort nötig ist. Wenn der Besitzer des PDFs verlangt, dass es nicht kopiert wird, fügt JeffDeck es trotzdem ein und zeigt einen entsprechenden Hinweis; diese Bitte ist keine Sperre.

## Eine Folie als Bild speichern

Klicken Sie auf **Datei > Exportieren > Folie als Bild exportieren**. JeffDeck speichert die aktuelle Folie als PNG- oder JPEG-Bild mit 1920 Pixeln Breite auf weißem Hintergrund. Als Dateinamen schlägt es den Namen der Präsentation und die Nummer der Folie vor.

## Siehe auch

- [Die Ansicht „Datei“](file-screen)
- [Sprechernotizen](notes)
- [Folien hinzufügen, ändern und löschen](slides)
