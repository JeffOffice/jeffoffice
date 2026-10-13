# Präsentationen öffnen und speichern
<!-- keywords: öffnen, speichern, speichern unter, otp, libreoffice, ausgelassen, weggelassen, schließen, neu, zuletzt verwendete Dateien, angeheftet, Dateitypen, Dateiformate, pptx, potx, ppsx, ppt, odp, Vorlage, Bildschirmpräsentation, umbenennen, Strg+S, Sicherung, Sicherungskopie, alte Dateien, OpenDocument, beibehalten, nicht angezeigt, Schriftarten, Schriftarten einbetten, eingebettete Schriftarten, fehlende Schriftart -->

JeffDeck speichert Ihre Arbeit in PPTX-Dateien und in ODP-Dateien und öffnet verschiedene Arten von Präsentationsdateien.

## Eine neue Präsentation beginnen

Drücken Sie Strg+N. Ein neues Fenster mit einer leeren Präsentation wird geöffnet. Oder klicken Sie auf **Datei > Neu** und dann auf **Leere Präsentation**. Enthält das aktuelle Fenster eine neue Präsentation, die noch niemand bearbeitet hat, tritt die leere Präsentation an ihre Stelle. Andernfalls wird sie in einem eigenen Fenster geöffnet.

## Speichern

Drücken Sie Strg+S, oder klicken Sie auf **Datei > Speichern**. Beim ersten Mal fragt JeffDeck nach einem Namen und einem Speicherort. Danach ersetzt **Speichern** die Datei durch Ihren aktuellen Stand. Unten im Fenster erscheint die Meldung „Gespeichert“ mit dem Dateinamen.

Ein Stern (*) hinter dem Namen in der Titelleiste bedeutet, dass Sie Änderungen noch nicht gespeichert haben. Speichern Sie oft. Falls JeffDeck unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Meldet JeffDeck, dass einige Objekte „in dieser Dateiart noch nicht gespeichert werden können“, wurde die Datei gespeichert, aber JeffDeck konnte diese Objekte nicht schreiben. Bewahren Sie auch die Originaldatei auf. Wenn Sie eine ODP-Datei speichern, nennt die Meldung, was diese Dateiart nicht aufnehmen kann. Was das ist, steht unten im Abschnitt „Als ODP-Datei speichern“.

## Unter einem neuen Namen oder in einer anderen Dateiart speichern

Klicken Sie auf **Datei > Speichern unter**, oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie die Dateiart aus, und klicken Sie auf **Speichern**. Zur Auswahl stehen:

- **Präsentation (*.pptx)** ist die übliche Wahl.
- **Vorlage (*.potx)** speichert die Folien als Muster, aus dem Sie neue Präsentationen beginnen.
- **Bildschirmpräsentation (*.ppsx)** speichert eine Präsentation zum Vorführen.
- **OpenDocument-Präsentation (*.odp)** speichert die Folien in der Dateiart, die LibreOffice und andere freie Programme verwenden.
- **OpenDocument-Präsentationsvorlage (*.otp)** ist die Vorlagenform dieser Dateiart.

Wenn Sie die Dateiendung weglassen, ergänzt JeffDeck die passende. Nach dem Speichern zeigt die Titelleiste den neuen Namen, und Sie arbeiten in der neuen Datei weiter.

Eine Präsentation, die Sie aus einer Vorlage geöffnet haben, wird als neue Präsentation gespeichert: **Speichern** fragt nach einem Namen, damit die Vorlage unverändert bleibt. Dasselbe gilt für Präsentationen aus älteren PPT-, PPS- oder POT-Dateien. Eine aus einer ODP-Datei geöffnete Präsentation wird wieder als ODP-Datei in dieselbe Datei gespeichert.

## Öffnen

Drücken Sie Strg+O, um direkt eine Datei auszuwählen. Oder klicken Sie auf **Datei > Öffnen** und dann auf **Durchsuchen** oder auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Präsentationen**.

Die Datei wird in diesem Fenster geöffnet, wenn es eine neue Präsentation enthält, die noch niemand bearbeitet hat. Andernfalls wird sie in einem eigenen Fenster geöffnet.

Klicken Sie mit der rechten Maustaste auf einen Namen in der Liste, um **An Liste anheften** (er bleibt oben), **Von Liste lösen** oder **Aus Liste entfernen** zu wählen. Ein grauer Name bedeutet, dass JeffDeck diese Datei derzeit nicht findet.

## Schriftarten in der Datei

Eine Schriftart bestimmt die Form der Buchstaben. Nicht jeder Computer hat jede Schriftart. Fehlt einem Computer eine Schriftart, die Ihre Folien verwenden, stellt er die Buchstaben in einer anderen Schriftart dar. Dann können die Folien anders aussehen.

JeffDeck kann die Schriftarten, die Ihre Folien verwenden, in die Datei aufnehmen. Dann sehen die Folien auf jedem Computer gleich aus. Das nennt man Einbetten. JeffDeck tut dies bei jedem Speichern. Zum Ausschalten klicken Sie auf **Datei > Optionen**, öffnen die Registerkarte **Speichern** und deaktivieren **Schriftarten in der Datei einbetten**.

- JeffDeck bettet nur die Schriftarten ein, die mit JeffDeck geliefert werden, etwa Fraunces, Lato und Open Sans. Andere Computer haben diese nicht. Schriftarten, die jeder Computer hat, wie Calibri oder Arial, bettet JeffDeck nie ein. Schriftarten, deren Lizenz das Kopieren in eine Datei verbietet, lässt es aus.
- JeffDeck bettet jeden Schriftschnitt ein, den die Folien verwenden: Standard, Fett, Kursiv und Fett Kursiv. Verwenden die Folien nur den Standardschnitt, wird nur dieser eingebettet.
- JeffDeck bettet alle Zeichen jeder Schriftart ein, nicht nur die verwendeten. So kann jemand, der die Datei öffnet, den Text weiterhin ändern.
- Die Datei wird größer. Jeder Schriftschnitt fügt etwa 40 bis 700 KB hinzu (ein KB sind tausend Byte). Die Datei wird beim Speichern komprimiert, daher wächst sie nur um etwa die Hälfte davon.

Wenn Sie eine Datei mit eingebetteten Schriftarten öffnen, verwendet JeffDeck diese, solange die Präsentation geöffnet ist. Die Folien sehen so aus wie dort, wo sie erstellt wurden. JeffDeck installiert die Schriftarten nicht auf Ihrem Computer und kopiert sie nirgendwohin. Sie gehören nur zu dieser Präsentation: Sie erscheinen nicht in Ihren Schriftartlisten, und keine andere Präsentation verwendet sie. Beim Schließen der Präsentation werden sie wieder entfernt. Ist eine Schriftart bereits auf Ihrem Computer vorhanden, verwendet JeffDeck Ihre eigene Kopie.

JeffDeck liest Schriftarten aus PPTX-Dateien anderer Präsentationsprogramme, auch wenn das Programm sie komprimiert hat. Auch aus ODP-Dateien liest es Schriftarten.

Eine Datei kann eine Schriftart enthalten, die JeffDeck nicht hat. Beim Speichern schreibt JeffDeck diese Schriftart wieder in die Datei, solange die Folien sie noch verwenden. Das geschieht unabhängig davon, ob **Schriftarten in der Datei einbetten** aktiviert ist. Ist eine Schriftart in einer Datei beschädigt oder nicht lesbar, weist die Meldung beim Öffnen der Datei darauf hin.

## Dateiarten, die JeffDeck liest

- **.pptx, .potx und .ppsx** sind ein Format in drei Formen: Präsentation, Vorlage und Bildschirmpräsentation. JeffDeck liest und schreibt alle drei.
- **.ppt, .pps und .pot** sind die ältere Dateiart. JeffDeck liest sie, kann sie aber nicht schreiben. Daher fragt **Speichern** nach einem neuen Namen und speichert eine PPTX-Datei (bei .pps eine PPSX-Datei). Das Original wird nie überschrieben.
- **.odp und .otp** sind OpenDocument-Präsentationen und deren Vorlagen. JeffDeck liest und schreibt beide. Eine hier geöffnete ODP-Datei wird als ODP-Datei gespeichert. Eine OTP-Datei ist eine Vorlage, daher fragt **Speichern** nach einem neuen Namen und schlägt eine ODP-Datei vor.

Kann ein Teil einer Datei nicht gelesen werden, öffnet JeffDeck den Rest und listet auf, was ausgelassen wurde. Kann die ganze Datei nicht gelesen werden, nennt JeffDeck den Grund.

## Als ODP-Datei speichern

Eine ODP-Datei wird in LibreOffice Impress mit Ihren Folien, Mastern, Bildern, Notizen und Kommentaren geöffnet und sieht so aus, wie JeffDeck sie anzeigt. Sie enthält auch Animationen, Folienübergänge, zielgruppenorientierte Präsentationen und die Einstellungen für die Bildschirmpräsentation. Die Schriftart jedes Worts wird mit ihrem Namen gespeichert. Ein Computer, der die Schriftart nicht hat, zeigt daher eine andere an.

JeffDeck selbst liest Animationen, Kommentare, zielgruppenorientierte Präsentationen und die Präsentationseinstellungen noch nicht aus einer ODP-Datei zurück. Sie sind in der Datei enthalten, und LibreOffice gibt sie wieder, aber wenn Sie die ODP-Datei in JeffDeck öffnen, werden sie als ausgelassen aufgeführt. Behalten Sie Ihre PPTX-Datei, wenn Sie sie hier weiter bearbeiten möchten.

Für einige Dinge bietet eine ODP-Datei keinen Platz. JeffDeck nennt sie nach dem Speichern in der Meldung unten im Fenster:

- **Diagramme, SmartArt-Grafiken und Formeln** werden als Bilder gespeichert. Sie sind sichtbar, aber Zahlen oder Bestandteile lassen sich in der ODP-Datei nicht bearbeiten.
- **Abschnitte** werden nicht gespeichert.
- **Spiegelungen, Abschrägungen und innere Schatten** werden ausgelassen.
- **Animationseffekte**, die beim Klicken auf ein Objekt starten, und Effekte, die eine Farbe ändern, werden ausgelassen. Ein Klick auf ein Video, um es anzuhalten oder erneut abzuspielen, bleibt erhalten.
- **Ein verknüpftes Video oder Audio**, das in einer anderen Datei liegt und nicht in der Präsentation, behält in der ODP-Datei seine Adresse und sein Bild. LibreOffice gibt es wieder, wenn es die Datei findet. Videos und Audios, die in der Präsentation enthalten sind, bleiben in der ODP-Datei erhalten und werden wiedergegeben.

Behalten Sie auch Ihre PPTX-Datei, wenn Sie diese Dinge später wieder benötigen.

## Was erhalten bleibt, aber nicht angezeigt wird

Manche Teile einer Datei kann JeffDeck noch nicht anzeigen oder ändern. JeffDeck behält sie bei ihren Folien und schreibt sie beim Speichern zurück, sodass sie nicht verloren gehen. Dazu gehören Animationen, die in der Bildschirmpräsentation wiedergegeben werden, Videos und Audios, die noch nicht wiedergegeben werden, sowie Übergangseffekte, für die JeffDeck keine Entsprechung hat und die als Verblassen wiedergegeben werden. Diagramme und SmartArt-Grafiken werden so angezeigt, wie sie gespeichert wurden. Siehe [Willkommen bei JeffDeck](start).

## Verwandte Themen

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Der Bildschirm „Datei“](file-screen)
- [Folien aus einer anderen Präsentation wiederverwenden](reuse)
- [Drucken, PDF und Folienbilder](printing)
