# Sicherheit: worauf eine Datei zugreifen darf

<!-- keywords: Trust Center, Sicherheit, Makros, verknüpfte Dateien, Verknüpfungen, verknüpfte Bilder, verknüpfte Medien, Internet, heruntergeladen, schreibgeschützt, geschützt, externe Verknüpfungen, Programme, sicher -->

Eine Datei kann auf andere Dinge verweisen: auf Dateien, Webseiten, Filme und sogar auf Programme. Auf der Seite **Sicherheit** legen Sie fest, was JeffPub dabei tut. Klicken Sie auf **Datei > Optionen** und dann auf die Registerkarte **Sicherheit**. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

Diese Einstellungen gelten für alle JeffOffice-Apps: Ändern Sie eine Einstellung in einer App, wird sie in allen geändert. Eine Zeile für etwas, das eine App nicht bietet, zeigt **Trifft nicht zu** und ist ausgegraut.

## Makros

Makros sind kleine Programme, die in einer Datei gespeichert sind. JeffPub bewahrt sie in der Datei auf, sodass sie beim Speichern erhalten bleiben, und **führt sie niemals aus**. Es gibt keine Einstellung, die sie ausführt.

## Verknüpfte Dateien und Verknüpfungen

Hier geht es um Bilder, die mit einer Datei **verknüpft** sind, statt in der Publikation gespeichert zu werden. Die Auswahl legt fest, was JeffPub tut, wenn es eine Publikation öffnet, die solche Bilder enthält.

- **Aktualisieren** ist die übliche Wahl. Beim Öffnen einer Publikation liest JeffPub die Datei jedes verknüpften Bilds erneut ein, wenn sich die Datei im eigenen Ordner der Publikation befindet, damit das Bild auf dem neuesten Stand ist. Es wird nie nachgefragt.
- **Nie aktualisieren** lässt jedes Bild so, wie es gespeichert wurde. Jedes zeigt **Nicht aktualisiert**, bis Sie bei ihm **Verknüpfung aktualisieren** verwenden. **Als PDF exportieren** und **Drucken** melden dann in der Statusleiste, dass für Bilder Vorschauen verwendet wurden, die in der Publikation keine vollständige Kopie haben.

JeffPub liest nur eine Datei, die im eigenen Ordner der Publikation liegt (oder in einem Ordner, den Sie als unbedenklich festgelegt haben). Jede andere verknüpfte Datei wartet, bis Sie sie anfordern.

## Verknüpfte Filme und Töne

Dies trifft in JeffPub nicht zu: Das Programm hat keine verknüpften Filme oder Töne abzuspielen. Die Zeile ist ausgegraut.

## Dateien aus dem Internet oder von anderen Computern

Eine Datei, die Sie heruntergeladen haben oder die auf einem anderen Computer in Ihrem Netzwerk liegt, ist unter Umständen nicht sicher. Diese Auswahl legt fest, wie JeffPub sie öffnet.

- **Zuerst schreibgeschützt öffnen** ist die übliche Wahl. Die Datei wird zum Lesen geöffnet, lässt sich aber nicht ändern. Eine Leiste unter der Datei weist darauf hin. Klicken Sie auf der Leiste auf **Bearbeitung aktivieren**, um diese eine Datei zu ändern (bis Sie das Programm schließen). Wenn Sie sie danach an derselben Stelle speichern, entfernt JeffPub die Download-Markierung von der Datei. Alternativ verwenden Sie **Speichern unter**, um eine Kopie anzulegen, oder wechseln Sie zur nächsten Auswahl dieser Einstellung.
- **Normal öffnen** öffnet jede Datei auf die gleiche Weise.

Eine Datei, die Sie über den Bereich **Datei** mit **Als Kopie öffnen**, **Öffnen und reparieren** oder **Schreibgeschützt öffnen** öffnen, wird unabhängig von dieser Einstellung so geöffnet.

## Verknüpfungen, die ein Programm starten

Eine Verknüpfung, die ein Programm oder ein Skript starten würde (eine Datei mit der Endung .exe, .bat, .sh und so weiter), wird **immer abgelehnt**. JeffPub teilt Ihnen das mit, und nichts wird ausgeführt. Ein Link zu einer Datei auf einem anderen Computer wird nie ohne Rückfrage geöffnet. Keine Einstellung ändert das.

## Siehe auch

- [JeffPub-Einstellungen ändern](options)
- [Bilder](pictures)
