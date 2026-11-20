# Sicherheit: worauf eine Datei zugreifen darf
<!-- keywords: Vertrauensstellungscenter, Sicherheit, Makros, verknüpfte Dateien, Verknüpfungen, verknüpfte Bilder, verknüpfte Medien, Internet, heruntergeladen, schreibgeschützt, geschützt, externe Verknüpfungen, Programme, sicher -->

Eine Datei kann auf andere Dinge verweisen: auf Dateien, Webseiten, Filme und sogar Programme. Auf der Seite **Sicherheit** legen Sie fest, wie JeffPub damit umgeht. Klicken Sie auf **Datei > Optionen** und dann auf die Registerkarte **Sicherheit**. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

Diese Einstellungen gelten für alle JeffOffice-Apps: Ändern Sie eine in einer App, ändert sie sich in allen. Eine Zeile für etwas, das eine App nicht besitzt, zeigt **Trifft nicht zu** und ist ausgegraut.

## Makros

Makros sind kleine Programme, die in einer Datei gespeichert sind. JeffPub **führt sie niemals aus**, und es gibt keine Einstellung, die sie ausführt. JeffPub behält auch die Makros einer Publikation nicht: Wenn Sie eine geöffnete `.pub`-Datei speichern, enthält die Kopie keine Makros. Bewahren Sie das Original auf, falls Sie die Makros noch brauchen.

## Verknüpfte Dateien und Arbeitsmappenverknüpfungen

Hier geht es um Bilder, die mit einer Datei **verknüpft** sind, statt in der Publikation gespeichert zu sein. Die Auswahl bestimmt, was JeffPub beim Öffnen einer Publikation mit solchen Bildern tut.

- **Aktualisieren** ist die übliche Wahl. Beim Öffnen einer Publikation liest JeffPub die Datei jedes verknüpften Bilds erneut ein, wenn sie im eigenen Ordner der Publikation liegt, damit das Bild aktuell ist. Es wird nie nachgefragt.
- **Nie aktualisieren** lässt jedes Bild so, wie es gespeichert wurde. Jedes zeigt **Nicht aktualisiert**, bis Sie **Verknüpfung aktualisieren** dafür verwenden. **Als PDF exportieren** und **Drucken** melden dann in der Statusleiste, dass für Bilder Vorschauen verwendet wurden, die in der Publikation keine vollständige Kopie haben.

JeffPub liest nur eine Datei, die sich im eigenen Ordner der Publikation befindet (oder in einem Ordner, den Sie als unbedenklich festgelegt haben). Jede andere verknüpfte Datei wartet, bis Sie sie anfordern.

## Verknüpfte Filme und Töne

Dies trifft in JeffPub nicht zu: Das Programm hat keine verknüpften Filme oder Töne abzuspielen. Die Zeile ist ausgegraut.

## Dateien aus dem Internet oder von anderen Computern

Eine Datei, die Sie heruntergeladen haben oder die auf einem anderen Computer in Ihrem Netzwerk liegt, ist möglicherweise nicht sicher. Diese Auswahl legt fest, wie JeffPub sie öffnet.

- **Zuerst schreibgeschützt öffnen** ist die übliche Wahl. Die Datei lässt sich zum Lesen öffnen, aber nicht ändern. Eine Leiste unter der Datei weist darauf hin. Klicken Sie in der Leiste auf **Bearbeitung aktivieren**, um nur diese eine Datei zu bearbeiten (bis Sie das Programm schließen). Wenn Sie sie danach am selben Ort speichern, entfernt JeffPub die Download-Markierung von der Datei. Oder verwenden Sie **Speichern unter**, um eine Kopie zu erstellen, oder ändern Sie diese Einstellung auf die nächste Option.
- **Normal öffnen** öffnet jede Datei auf die gleiche Weise.

Eine Datei, die Sie über **Als Kopie öffnen**, **Öffnen und reparieren** oder **Schreibgeschützt öffnen** im Bereich **Datei** öffnen, wird unabhängig von dieser Einstellung auf diese Weise geöffnet.

## Verknüpfungen, die ein Programm starten

Eine Verknüpfung, die ein Programm oder ein Skript starten würde (eine Datei mit der Endung .exe, .bat, .sh und so weiter), wird **immer abgelehnt**. JeffPub meldet das, und nichts wird ausgeführt. Eine Verknüpfung zu einer Datei auf einem anderen Computer wird niemals ohne Ihre vorherige Rückfrage aufgerufen. Keine Einstellung ändert das.

## Siehe auch

- [JeffPub-Einstellungen ändern](options)
- [Bilder](pictures)
