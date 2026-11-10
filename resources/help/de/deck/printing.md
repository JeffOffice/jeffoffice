# Drucken, PDF und Folienbilder

<!-- keywords: drucken, Drucker, Druck, Kopien, Folien, Notizenseiten, Handzettel, Gliederung, PDF, exportieren, Bild, PNG, JPEG, JPG, Grafik, Bereich, ausgeblendete Folien, Rahmen, Graustufen, Schwarzweiß, Druckvorschau, Papier, A4, A5, Kennwort, verschlüsseln, mit Kennwort schützen -->

Sie können Ihre Präsentation drucken, als PDF speichern oder eine Folie als Bild speichern. Alle drei Funktionen finden Sie in der Ansicht „Datei“.

## Drucken

1. Drücken Sie Strg+P, oder klicken Sie auf **Datei > Drucken**.
2. Wählen Sie unter **Drucken** aus, was ausgegeben wird: **Folien**, **Notizenseiten**, **Gliederung** oder **Handzettel** mit 1, 2, 3, 4, 6 oder 9 Folien pro Seite.
3. Bei Handzetteln mit 4 oder mehr Folien legt **Reihenfolge** fest, ob die Folien zuerst waagerecht über die Seite und dann senkrecht angeordnet werden (**Horizontal**) oder zuerst senkrecht und dann waagerecht (**Vertikal**).
4. Wählen Sie unter **Bereich** entweder **Alle Folien** oder **Aktuelle Folie**, oder wählen Sie **Folien** und geben Sie die Nummern in das Feld ein: Foliennummern und Bereiche, durch Kommas getrennt, zum Beispiel 1-3, 5.
5. Aktivieren Sie bei Bedarf **Ausgeblendete Folien drucken**, **Folien mit Rahmen** (ein dünner Rahmen um jede Folie) und **Graustufen**.
6. Wählen Sie unter **Seiten** entweder **Einseitig drucken**, **Beidseitig drucken, an der langen Kante wenden** (Seiten, die wie ein Buch umgeblättert werden), **Beidseitig drucken, an der kurzen Kante wenden** (Seiten, die wie ein Notizblock umgeblättert werden) oder **Beidseitig manuell drucken**. Die ersten beiden Optionen sind abgeblendet, wenn Ihr Drucker nicht beidseitig drucken kann. Zeigen Sie mit der Maus darauf, um den Grund zu sehen. **Manuell** ist für solche Drucker gedacht: JeffDeck druckt zuerst die erste, dritte, fünfte usw. Seite. Danach fordert eine Meldung Sie auf, den Stapel umzudrehen und wieder in den Drucker zu legen. Wenn Sie auf **OK** klicken, druckt JeffDeck die übrigen Seiten, beginnend mit der letzten. Bei nur einer Seite muss nichts umgedreht werden. JeffDeck merkt sich Ihre Auswahl.
7. Klicken Sie auf **Drucken**. JeffDeck öffnet das Druckfenster Ihres Systems, in dem Sie den Drucker und die Anzahl der Kopien wählen. Das Fenster beginnt mit Ihrer Auswahl bei den Seiten. Eine PDF-Datei, die aus diesem Fenster erstellt wird, hat keine Seiten.

Die Vorschau rechts zeigt jede Seite, die ausgegeben wird, mit Pfeilen zum Blättern. Sie wird genauso dargestellt, wie die Seite gedruckt wird. JeffDeck merkt sich Ihre Einstellungen für das nächste Mal.

## Was die einzelnen Optionen drucken

- **Folien** druckt eine Folie auf jede Seite, in der Form der Folie. Bei einer breiten Folie wird das Papier quer eingelegt.
- **Notizenseiten** zeigt oben auf der Seite eine Folie, darunter die Sprechernotizen.
- **Handzettel** legen mehrere verkleinerte Folien in einem Raster auf jede Seite, dazu eine Kopf- und eine Fußzeile mit Titel, Datum und Seitenzahl. Bei 3 Folien pro Seite gibt es neben jeder Folie Linien zum Schreiben.
- **Gliederung** druckt die Nummer und den Titel jeder Folie, danach den Text der Folie, nach Ebenen eingerückt.

Notizenseiten, Handzettel und die Gliederung werden im Hochformat auf Letter- oder A4-Papier gedruckt, je nachdem, welches Format in Ihrer Region üblich ist. Ausgeblendete Folien werden nur gedruckt, wenn Sie **Ausgeblendete Folien drucken** aktivieren.

## Als PDF speichern

Klicken Sie auf **Datei > Exportieren > Als PDF exportieren**. Ein Fenster fragt, was enthalten sein soll: dieselben Optionen **Drucken**, **Reihenfolge** und **Bereich** wie beim Drucken sowie dieselben drei Kontrollkästchen. Es zeigt an, wie viele Seiten die Datei haben wird. Klicken Sie auf **Exportieren…**, und wählen Sie dann einen Namen und einen Speicherort.

Das PDF enthält echten Text und die Schriftarten, sodass es auf jedem Computer gleich aussieht. Sein Titel ist der Titel der Präsentation oder der Dateiname.

Aktivieren Sie **PDF/A zur langfristigen Aufbewahrung**, um eine PDF/A-1b-Datei zu erstellen. Alle Schriftarten sind in der Datei enthalten, und transparente Effekte werden reduziert, damit die Folien auch in Jahren gleich aussehen. Die Datei ist nicht für Bildschirmleseprogramme gekennzeichnet. Ein PDF mit Kennwort kann kein PDF/A sein. Wenn Sie ein Kennwort wählen, wird PDF/A daher ausgeschaltet.

## Präsentation per E-Mail senden

Klicken Sie auf **Datei > Exportieren > E-Mail erstellen**. Wählen Sie, ob die Präsentation als Datei oder als PDF angehängt werden soll. JeffDeck erstellt damit eine Nachrichtendatei (.eml) und fragt, wo sie gespeichert werden soll. Es wird nichts gesendet. Öffnen Sie die Datei in Ihrem E-Mail-Programm, tragen Sie den Empfänger ein, und senden Sie die Nachricht. Eine Präsentation mit Kennwort wird mit dem Kennwort angehängt. Ein PDF kann ein eigenes Kennwort haben.

## PDF mit Kennwort schützen

Klicken Sie im Fenster **Als PDF exportieren** auf **Mit Kennwort verschlüsseln…**. Geben Sie das Kennwort zweimal ein. Wer die PDF-Datei öffnet, muss dann das Kennwort eingeben. Das Kennwort schützt die Datei wirklich: Die Seiten werden mit starker Verschlüsselung (AES-256) unlesbar gemacht, und niemand kann sie ohne Kennwort lesen.

Sie können auch **Drucken, Kopieren und Bearbeiten einschränken** aktivieren, ein zweites Kennwort vergeben und festlegen, was erlaubt ist. Diese Einschränkungen sind nur eine Bitte. Die meisten PDF-Programme halten sich daran, aber wer sie umgehen will, kann das. Verlassen Sie sich deshalb nicht darauf, eine Datei damit vertraulich zu halten.

Ein vergessenes Kennwort kann nicht wiederhergestellt werden. Bewahren Sie es daher an einem sicheren Ort auf. Das Kennwort gilt nur für dieses eine PDF und wird nirgends gespeichert.

Wenn Sie ein PDF mit Kennwort als Bild einfügen, fragt JeffDeck nach dem Kennwort. Ist es falsch, fragt es erneut; klicken Sie auf **Abbrechen**, um abzubrechen. Die Folie erhält dann eine Kopie der Seite, für die kein Kennwort nötig ist. Hat der Eigentümer des PDFs verlangt, dass es nicht kopiert wird, fügt JeffDeck es trotzdem ein und zeigt einen entsprechenden Hinweis an. Diese Bitte ist allerdings kein Kopierschutz.

## Eine Folie als Bild speichern

Klicken Sie auf **Datei > Exportieren > Folie als Bild exportieren**. JeffDeck speichert die aktuelle Folie als PNG- oder JPEG-Bild mit 1920 Pixeln Breite auf weißem Hintergrund. Als Dateiname schlägt es den Namen der Präsentation und die Nummer der Folie vor.

## Siehe auch

- [Die Ansicht „Datei“](file-screen)
- [Sprechernotizen](notes)
- [Folien hinzufügen, ändern und löschen](slides)
