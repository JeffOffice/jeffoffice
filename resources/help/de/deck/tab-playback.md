# Registerkarte Wiedergabe
<!-- keywords: Wiedergabe-Registerkarte, Menüband, Film, Video, Ton, Audio, abspielen, wiedergeben, Pause, Vorschau, zuschneiden, Beschriftungen, Beschriftungen einfügen, Beschriftungen entfernen, Untertitel, WebVTT, VTT, Lautstärke, stumm, Start, in Klickreihenfolge, automatisch, bei Klick, Vollbild wiedergeben, Ausblenden bei Nichtwiedergabe, bis zum Stopp wiederholen, nach der Wiedergabe zurückspulen, über Folien wiedergeben, Alternativtext, ausrichten, Höhe, Breite, Größe, Videotools -->

Die Registerkarte **Wiedergabe** legt fest, wie ein Film oder ein Ton abgespielt wird, und fügt einem Film Beschriftungen hinzu. Sie wird nur angezeigt, während ein einzelner Film oder Ton ausgewählt ist, und verschwindet, sobald Sie etwas anderes auswählen.

- **Vorschau** enthält **Wiedergeben**. Damit spielen Sie den Film oder Ton auf der Folie ab, damit Sie ihn sehen und hören können. Während er läuft, heißt die Schaltfläche **Pause**.
- **Bearbeiten** enthält **Zuschneiden**. Es öffnet ein Feld mit **Startzeit** und **Endzeit** in Sekunden. Der Film oder Ton wird nur von der Startzeit bis zur Endzeit abgespielt. Der übrige Teil der Datei bleibt in der Präsentation erhalten. Um wieder bis zum Ende der Datei abzuspielen, stellen Sie die Endzeit auf die volle Länge ein.
  - **Beschriftungen einfügen** fügt einem Film Beschriftungen aus einer WebVTT-Datei hinzu (einer Textdatei mit der Endung `.vtt`). Das Programm fragt zuerst nach der Datei und danach nach der Sprache der Beschriftungen als Kurzcode, etwa `en`, `es` oder `pt-BR`. Beschriftungen gehören zu einem Film, der in der Präsentation eingebettet ist; bei einem Film, der nur von außen verknüpft ist, ist die Schaltfläche abgeblendet. Ein Film behält für jede Sprache einen Satz Beschriftungen: Wenn Sie für dieselbe Sprache eine zweite Datei einfügen, ersetzt sie die erste. Eine Datei, die nicht WebVTT ist, keine Beschriftungen enthält oder zu groß ist (höchstens 4 MB, 20.000 Beschriftungen und 2.000 Zeichen in einer Beschriftung), wird mit einer Meldung abgelehnt, und es ändert sich nichts.
  - **Beschriftungen entfernen** entfernt die Beschriftungen aus dem Film. Hat der Film Beschriftungen in mehr als einer Sprache, fragt das Programm, welche entfernt werden sollen, oder ob alle entfernt werden sollen.
- **Medienoptionen** legt fest, wie der Film in der Bildschirmpräsentation abgespielt wird:
  - **Lautstärke** ist **Niedrig**, **Mittel**, **Hoch** oder **Stumm**.
  - **Start** legt fest, was die Wiedergabe startet. **In Klickreihenfolge** spielt den Film bei einem Klick ab, zusammen mit den Animationen der Folie. **Automatisch** spielt ihn von selbst ab, sobald er an der Reihe ist. **Bei Klick** spielt ihn ab, wenn Sie auf den Film oder Ton klicken.
  - **Vollbild wiedergeben** lässt einen Film während der Wiedergabe den ganzen Bildschirm füllen. Das gilt nur für Filme.
  - **Ausblenden bei Nichtwiedergabe** blendet das Bild aus, bis der Film oder Ton abgespielt wird, und wieder, nachdem er gestoppt wurde. Das Lautsprechersymbol eines Tons ist die ganze Zeit ausgeblendet.
  - **Bis zum Stopp wiederholen** spielt ihn nach jedem Ende erneut ab, bis er gestoppt wird oder die Folie wechselt.
  - **Nach der Wiedergabe zurückspulen** setzt einen Film nach dem Ende auf sein erstes Bild zurück. Ohne diese Option bleibt der Film auf seinem letzten Bild stehen.
  - **Über Folien wiedergeben** (für einen Ton; ein Film wird nur auf seiner eigenen Folie gezeigt) lässt den Ton in der Bildschirmpräsentation weiterlaufen, wenn die Folie wechselt, bis er endet oder gestoppt wird, im Extremfall bis zum Ende der Präsentation. Eine Datei aus einem anderen Programm kann angeben, über wie viele Folien der Ton wiedergegeben wird: Diese Anzahl bleibt erhalten, solange das Kästchen aktiviert ist, und der Ton stoppt nach so vielen Folien, gezählt ab der Folie, auf der er begonnen hat (Hin- und Herblättern zwischen diesen Folien verbraucht keine davon). Ist das Kästchen deaktiviert, stoppt der Ton mit seiner Folie; ist es wieder aktiviert, läuft er bis zum Ende der Präsentation weiter. Zusammen mit **Bis zum Stopp wiederholen** beginnt er nach jedem Ende von vorn. Mit **Strg+Z** machen Sie die Änderung rückgängig; eine .pptx-Datei speichert sie.

Beschriftungen werden unten über dem Film angezeigt, als weiße Schrift auf einem dunklen Band, während der Film in der Bildschirmpräsentation und im Editor läuft. Sie sind auf den Film abgestimmt, und ihre Größe richtet sich nach der Größe des Films. Hat ein Film mehrere Sprachen, wird die erste angezeigt. Beschriftungen werden mit der Präsentation als Teil der `.pptx`-Datei gespeichert und stehen daher beim erneuten Öffnen wieder zur Verfügung. Eine Beschriftungsdatei in einem anderen Format (nicht WebVTT) bleibt unverändert erhalten und wird unverändert wieder gespeichert, wird aber nicht angezeigt. Sobald ein Film Beschriftungen hat, führt **Barrierefreiheit prüfen** ihn nicht mehr auf.

Die Gruppen **Barrierefreiheit**, **Anordnen** und **Größe** entsprechen denen der Registerkarte Bildformat: **Alternativtext**, **Ausrichten** sowie die Felder **Höhe** und **Breite** gelten für den ausgewählten Film oder Ton.

Jede Änderung ist ein Schritt. **Strg+Z** macht ihn rückgängig.

Ein Ton oder Film, der über Folien hinweg wiedergegeben wird, was manche Dateien so festlegen, läuft in der Bildschirmpräsentation über so viele Folien, wie die Datei angibt. JeffDeck spielt ihn so ab und behält diese Einstellung beim Speichern bei.

Siehe [Formen, Bilder, Tabellen und Textkunst](objects) und [Bildschirmpräsentation abspielen und Referentenansicht verwenden](show).

## Verwandte Themen

- [Formen, Bilder, Tabellen und Textkunst](objects)
- [Registerkarte Einfügen](tab-insert)
- [Bildschirmpräsentation abspielen und Referentenansicht verwenden](show)
