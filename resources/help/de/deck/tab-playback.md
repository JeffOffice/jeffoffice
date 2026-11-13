# Registerkarte Wiedergabe
<!-- keywords: Wiedergabe, Registerkarte Wiedergabe, Menüband, Film, Video, Ton, Audio, abspielen, wiedergeben, Pause, Vorschau, zuschneiden, Beschriftungen, Untertitel, Beschriftungen einfügen, Beschriftungen entfernen, WebVTT, vtt, Lautstärke, stumm, Start, in Klickreihenfolge, automatisch, bei Klick, Vollbild, Ausblenden bei Nichtwiedergabe, bis zum Beenden wiederholen, zurückspulen nach Wiedergabe, Medientools -->

Die Registerkarte **Wiedergabe** legt fest, wie ein Film oder ein Ton abgespielt wird, und fügt einem Film Beschriftungen hinzu. Sie wird nur angezeigt, solange genau ein Film oder ein Ton ausgewählt ist, und verschwindet, sobald Sie etwas anderes auswählen.

- Unter **Vorschau** finden Sie **Wiedergabe**. Damit wird der Film oder der Ton auf der Folie abgespielt, sodass Sie ihn sehen und hören können. Während die Wiedergabe läuft, heißt die Schaltfläche **Anhalten**.
- Unter **Bearbeiten** finden Sie **Zuschneiden**. Es öffnet ein Feld mit **Startzeit** und **Endzeit**, angegeben in Sekunden. Der Film oder Ton wird nur von der Startzeit bis zur Endzeit abgespielt. Der Rest der Datei bleibt in der Präsentation erhalten. Um den Film wieder bis zum Dateiende abzuspielen, stellen Sie die Endzeit auf die volle Länge ein.
  - **Beschriftungen einfügen** fügt einem Film Beschriftungen aus einer WebVTT-Datei (einer Textdatei mit der Endung `.vtt`) hinzu. Sie werden zuerst nach der Datei und dann nach der Sprache der Beschriftungen gefragt, als Kurzcode wie `en`, `es` oder `pt-BR`. Beschriftungen können nur zu einem Film hinzugefügt werden, der in der Präsentation eingebettet ist; bei einem Film, der nur von außen verknüpft ist, ist die Schaltfläche abgeblendet. Ein Film behält für jede Sprache einen Satz Beschriftungen: Wenn Sie für dieselbe Sprache eine zweite Datei einfügen, ersetzt sie die erste. Eine Datei, die kein WebVTT ist, keine Beschriftungen enthält oder zu groß ist (höchstens 4 MB, 20.000 Beschriftungen und 2.000 Zeichen in einer Beschriftung), wird mit einer Meldung abgelehnt, und nichts ändert sich.
  - **Beschriftungen entfernen** nimmt die Beschriftungen vom Film. Hat der Film Beschriftungen in mehreren Sprachen, fragt JeffDeck, welche entfernt werden sollen, oder ob alle entfernt werden sollen.
- Unter **Optionen für Medien** legen Sie fest, wie der Film in der Bildschirmpräsentation abgespielt wird:
  - **Lautstärke** ist **Leise**, **Mittel**, **Laut** oder **Stumm**.
  - **Start** legt fest, was die Wiedergabe startet. **In Klickreihenfolge** spielt den Film bei einem Klick ab, zusammen mit den Animationen der Folie. **Automatisch** spielt ihn von selbst ab, wenn er an der Reihe ist. **Bei Klick** spielt ihn ab, wenn Sie auf den Film oder Ton klicken.
  - **Vollbild wiedergeben** lässt einen Film den Bildschirm ausfüllen, während er läuft. Das funktioniert nur bei Filmen.
  - **Ausblenden, wenn nicht wiedergegeben** blendet das Bild aus, bis der Film oder Ton abgespielt wird, und nach dem Ende wieder. Das Lautsprechersymbol eines Tons wird die ganze Zeit ausgeblendet.
  - **Bis zum Beenden wiederholen** spielt den Film oder Ton nach jedem Ende erneut ab, bis er angehalten wird oder die Folie wechselt.
  - **Nach Wiedergabe zurückspulen** setzt einen Film nach dem Ende auf sein erstes Bild zurück. Ohne diese Option bleibt der Film beim letzten Bild stehen.

Beschriftungen werden unten im Film angezeigt, als weiße Schrift auf einem dunklen Balken, während der Film in der Bildschirmpräsentation und im Editor läuft. Sie sind zeitlich auf den Film abgestimmt, und ihre Größe richtet sich nach der Größe des Films. Hat ein Film mehrere Sprachen, wird die erste angezeigt. Beschriftungen werden zusammen mit der Präsentation als Teil der `.pptx`-Datei gespeichert und stehen beim erneuten Öffnen wieder zur Verfügung. Eine Beschriftungsdatei in einem anderen Format (nicht WebVTT) bleibt erhalten und wird unverändert wieder gespeichert, wird aber nicht angezeigt. Sobald ein Film Beschriftungen hat, listet **Barrierefreiheit prüfen** ihn nicht mehr auf.

Jede Änderung ist ein einzelner Schritt. Mit **Strg+Z** machen Sie sie rückgängig.

Ein Ton oder Film, der über mehrere Folien läuft, was manche Dateien so festlegen, wird so lange abgespielt, wie die Datei es vorgibt. JeffDeck spielt ihn in der Bildschirmpräsentation so ab und behält die Einstellung beim Speichern bei.

Siehe [Formen, Bilder, Tabellen und Textkunst](objects) und [Bildschirmpräsentation abspielen und Referentenansicht verwenden](show).

## Verwandte Themen

- [Formen, Bilder, Tabellen und Textkunst](objects)
- [Registerkarte Einfügen](tab-insert)
- [Bildschirmpräsentation abspielen und Referentenansicht verwenden](show)
