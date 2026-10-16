# Präsentationen öffnen und speichern
<!-- keywords: öffnen, speichern, speichern unter, otp, libreoffice, ausgelassen, nicht übernommen, schließen, neu, neue Präsentation, zuletzt verwendete Dateien, angeheftet, Dateitypen, Dateiformate, pptx, potx, ppsx, ppt, odp, Vorlage, Bildschirmpräsentation, umbenennen, Strg+S, Sicherungskopie, Backup, alte Dateien, OpenDocument, beibehalten, nicht angezeigt, Schriftarten, Schriftarten einbetten, eingebettete Schriftarten, fehlende Schriftart -->

JeffDeck speichert Ihre Arbeit in PPTX-Dateien und in ODP-Dateien und öffnet verschiedene Arten von Präsentationsdateien.

## Eine neue Präsentation beginnen

Drücken Sie Strg+N. Ein neues Fenster mit einer leeren Präsentation wird geöffnet. Sie können auch auf **Datei > Neu** und dann auf **Leere Präsentation** klicken. Enthält das aktuelle Fenster eine neue Präsentation, die noch niemand bearbeitet hat, tritt die leere Präsentation an ihre Stelle. Andernfalls wird sie in einem eigenen Fenster geöffnet.

## Speichern

Drücken Sie Strg+S, oder klicken Sie auf **Datei > Speichern**. Beim ersten Mal fragt JeffDeck nach einem Namen und einem Speicherort. Danach ersetzt „Speichern“ die Datei durch Ihren aktuellen Stand. Unten im Fenster erscheint die Meldung „Gespeichert“ mit dem Namen der Datei.

Ein Sternchen (*) nach dem Namen in der Titelleiste bedeutet, dass Sie Änderungen haben, die noch nicht gespeichert sind. Speichern Sie häufig. Falls JeffDeck unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Wenn eine Meldung besagt, dass einige Objekte „in dieser Art von Datei noch nicht gespeichert werden können“, wurde die Datei gespeichert, aber JeffDeck konnte diese Objekte nicht schreiben. Bewahren Sie auch die Originaldatei auf. Wenn Sie eine ODP-Datei speichern, führt die Meldung auf, was diese Art von Datei nicht aufnehmen kann. Was das ist, steht unten im Abschnitt „Als ODP-Datei speichern“.

## Unter einem neuen Namen oder in einem anderen Format speichern

Klicken Sie auf **Datei > Speichern unter**, oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie das Format, und klicken Sie auf **Speichern**. Zur Auswahl stehen:

- **Präsentation (*.pptx)** ist die übliche Wahl.
- **Vorlage (*.potx)** speichert die Folien als Muster, aus dem Sie neue Präsentationen erstellen.
- **Bildschirmpräsentation (*.ppsx)** speichert eine Präsentation zum Vorführen.
- **OpenDocument-Präsentation (*.odp)** speichert die Folien in dem Dateiformat, das LibreOffice und andere kostenlose Programme verwenden.
- **OpenDocument-Präsentationsvorlage (*.otp)** ist die Vorlagenform dieses Formats.

Wenn Sie die Dateiendung weglassen, ergänzt JeffDeck die zum Format passende. Nach dem Speichern zeigt die Titelleiste den neuen Namen, und Sie arbeiten in der neuen Datei weiter.

Eine Präsentation, die Sie aus einer Vorlage geöffnet haben, wird als neue Präsentation gespeichert: **Speichern** fragt nach einem Namen, damit die Vorlage unverändert bleibt. Dasselbe gilt für eine Präsentation, die aus einer älteren PPT-, PPS- oder POT-Datei geöffnet wurde. Eine aus einer ODP-Datei geöffnete Präsentation wird wieder als ODP-Datei gespeichert, und zwar in dieselbe Datei.

## Öffnen

Drücken Sie Strg+O, um direkt eine Datei auszuwählen. Oder klicken Sie auf **Datei > Öffnen** und dann auf **Durchsuchen** oder auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Präsentationen**.

Die Datei wird in diesem Fenster geöffnet, wenn es eine neue Präsentation enthält, die noch niemand bearbeitet hat. Andernfalls wird sie in einem eigenen Fenster geöffnet.

Klicken Sie mit der rechten Maustaste auf einen Namen in der Liste, um **An Liste anheften** (er bleibt dann oben), **Von Liste lösen** oder **Aus Liste entfernen** zu wählen. Ein grau dargestellter Name bedeutet, dass JeffDeck diese Datei zurzeit nicht finden kann.

## Schriftarten in der Datei

Eine Schriftart bestimmt die Form der Buchstaben. Nicht jeder Computer hat jede Schriftart. Fehlt einem Computer eine Schriftart, die Ihre Folien verwenden, zeichnet er die Buchstaben in einer anderen Schriftart. Dann können die Folien anders aussehen.

JeffDeck kann die Schriftarten, die Ihre Folien verwenden, in die Datei aufnehmen. Dann sehen die Folien auf jedem Computer gleich aus. Das nennt man Einbetten. JeffDeck tut dies bei jedem Speichern. Zum Ausschalten klicken Sie auf **Datei > Optionen**, öffnen die Registerkarte **Speichern** und deaktivieren **Schriftarten in der Datei einbetten**.

- JeffDeck bettet nur die Schriftarten ein, die mit JeffDeck geliefert werden, etwa Fraunces, Lato und Open Sans. Andere Computer haben diese nicht. Eine Schriftart, die jeder Computer hat, etwa Calibri oder Arial, wird nie eingebettet. Eine Schriftart, deren Lizenz das Kopieren in eine Datei nicht erlaubt, wird ausgelassen.
- JeffDeck bettet jeden Schriftschnitt ein, den die Folien verwenden: Standard, Fett, Kursiv und Fett Kursiv. Verwenden die Folien nur den Standardschnitt, wird nur dieser eingebettet.
- JeffDeck bettet alle Zeichen jeder Schriftart ein, nicht nur die verwendeten. So kann jemand, der die Datei öffnet, den Text weiterhin ändern.
- Die Datei wird größer. Jeder Schriftschnitt fügt etwa 40 bis 700 KB hinzu (ein KB sind tausend Byte). Die Datei wird beim Speichern komprimiert, daher wächst sie nur um etwa die Hälfte davon.

Wenn Sie eine Datei mit eingebetteten Schriftarten öffnen, verwendet JeffDeck diese, solange die Präsentation geöffnet ist. Die Folien sehen so aus wie dort, wo sie erstellt wurden. JeffDeck installiert die Schriftarten nicht auf Ihrem Computer und kopiert sie nirgendwohin. Sie gehören nur zu dieser Präsentation: Sie erscheinen nicht in Ihren Schriftartenlisten, und keine andere Präsentation verwendet sie. Beim Schließen der Präsentation verschwinden sie wieder. Ist eine Schriftart bereits auf Ihrem Computer vorhanden, verwendet JeffDeck Ihre eigene Kopie.

JeffDeck liest Schriftarten aus PPTX-Dateien anderer Präsentationsprogramme, auch wenn das Programm sie komprimiert hat. Auch aus ODP-Dateien liest JeffDeck Schriftarten.

Eine Datei kann eine Schriftart enthalten, die JeffDeck nicht hat. Beim Speichern schreibt JeffDeck diese Schriftart wieder in die Datei, sofern die Folien sie noch verwenden. Das geschieht unabhängig davon, ob **Schriftarten in der Datei einbetten** aktiviert ist. Ist eine Schriftart in einer Datei beschädigt oder nicht lesbar, weist die Meldung beim Öffnen der Datei darauf hin.

## Dateiformate, die JeffDeck liest

- **.pptx, .potx und .ppsx** sind ein Format in drei Formen: Präsentation, Vorlage und Bildschirmpräsentation. JeffDeck liest und schreibt alle drei.
- **.ppt, .pps und .pot** sind das ältere Format. JeffDeck liest es, kann es aber nicht schreiben. Daher fragt **Speichern** nach einem neuen Namen und speichert eine PPTX-Datei (bei einer PPS-Datei eine PPSX-Datei). Das Original wird nie überschrieben.
- **.odp und .otp** sind OpenDocument-Präsentationen und deren Vorlagen. JeffDeck liest und schreibt beide. Eine hier geöffnete ODP-Datei wird als ODP-Datei gespeichert. Eine OTP-Datei ist eine Vorlage, daher fragt **Speichern** nach einem neuen Namen und schlägt eine ODP-Datei vor.

Kann ein Teil einer Datei nicht gelesen werden, öffnet JeffDeck den Rest und führt auf, was ausgelassen wurde. Kann die ganze Datei nicht gelesen werden, nennt JeffDeck den Grund.

## Als ODP-Datei speichern

Eine ODP-Datei wird in LibreOffice Impress mit Ihren Folien, Mastern, Bildern, Notizen und Kommentaren geöffnet, und zwar so, wie JeffDeck sie anzeigt. Sie enthält außerdem Animationen, Folienübergänge, zielgruppenorientierte Präsentationen und die Einstellungen für die Bildschirmpräsentation. Die Schriftart jedes Worts wird mit ihrem Namen gespeichert. Ein Computer, der die Schriftart nicht hat, zeigt daher eine andere an.

JeffDeck selbst liest Animationen, Kommentare, zielgruppenorientierte Präsentationen und die Präsentationseinstellungen noch nicht aus einer ODP-Datei zurück. Sie sind in der Datei enthalten, und LibreOffice gibt sie wieder. Wenn Sie die ODP-Datei aber in JeffDeck öffnen, werden sie als ausgelassen aufgeführt. Bewahren Sie Ihre PPTX-Datei auf, wenn Sie diese Elemente hier weiter bearbeiten möchten.

Für einige Dinge hat eine ODP-Datei keinen Platz. JeffDeck führt sie nach dem Speichern in der Meldung unten im Fenster auf:

- **Diagramme, SmartArt-Grafiken und Formeln** werden als Bilder gespeichert. Sie sind sichtbar, aber ihre Zahlen oder Bestandteile lassen sich in der ODP-Datei nicht bearbeiten.
- **Abschnitte** werden nicht gespeichert.
- **Spiegelungen, Abschrägungen und innere Schatten** werden ausgelassen, ebenso Leuchten und Spiegelungen bei Text. Ein Schatten bei Text bleibt erhalten.
- **Animationseffekte**, die beim Klicken auf ein Objekt starten, und Effekte, die eine Farbe ändern, werden ausgelassen. Ein Klick auf ein Video, um es anzuhalten oder erneut wiederzugeben, bleibt erhalten.
- **Ein verknüpftes Video oder Audio**, das in einer anderen Datei liegt und nicht in der Präsentation gespeichert ist, behält in der ODP-Datei seine Adresse und sein Bild. LibreOffice gibt es wieder, wenn es die Datei findet. Videos und Audiodateien, die in der Präsentation enthalten sind, werden in der ODP-Datei gespeichert und wiedergegeben.

Bewahren Sie auch Ihre PPTX-Datei auf, wenn Sie diese Dinge wieder benötigen.

## Was beibehalten, aber nicht angezeigt wird

Manche Teile einer Datei kann JeffDeck noch nicht anzeigen oder ändern. JeffDeck behält sie bei ihren Folien und schreibt sie beim Speichern zurück, damit sie nicht verloren gehen. Dazu gehören Animationen, die in der Bildschirmpräsentation wiedergegeben werden, Videos und Audiodateien, die noch nicht wiedergegeben werden, sowie Übergangseffekte, für die JeffDeck keine Form hat und die als Verblassen wiedergegeben werden. Diagramme und SmartArt-Grafiken werden so angezeigt, wie sie gespeichert wurden. Siehe [Willkommen bei JeffDeck](start).

## Verwandte Themen

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Der Bildschirm „Datei“](file-screen)
- [Folien aus einer anderen Präsentation wiederverwenden](reuse)
- [Drucken, PDF und Folienbilder](printing)
