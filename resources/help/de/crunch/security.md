# Sicherheit: worauf eine Datei zugreifen darf
<!-- keywords: Vertrauensstelle, Sicherheit, Makros, verknüpfte Dateien, Verknüpfungen, verknüpfte Bilder, verknüpfte Medien, Internet, heruntergeladen, schreibgeschützt, geschützt, externe Verknüpfungen, Programme, sicher -->

Eine Datei kann auf andere Dinge verweisen: auf Dateien, Webseiten, Filme und sogar Programme. Auf der Seite **Sicherheit** legen Sie fest, was JeffCrunch dabei tut. Klicken Sie auf **Datei > Optionen** und dann auf die Registerkarte **Sicherheit**. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

Diese Einstellungen gelten für alle JeffOffice-Apps: Wenn Sie eine davon in einer App ändern, ändert sie sich in allen. Eine Zeile für etwas, das eine App nicht hat, zeigt **Trifft nicht zu** und ist ausgegraut.

## Makros

Makros sind kleine Programme, die in einer Datei gespeichert sind. JeffCrunch behält sie in der Datei, sodass sie beim Speichern erhalten bleiben, und führt sie **niemals aus**. Es gibt keine Einstellung, die sie ausführt.

## Verknüpfte Dateien und Arbeitsmappenverknüpfungen

Hier geht es um andere Arbeitsmappen, von denen eine Arbeitsmappe Zahlen bezieht; diese heißen **Verknüpfungen**. Die Auswahl legt fest, was JeffCrunch tut, wenn es eine Arbeitsmappe mit solchen Verknüpfungen öffnet.

- **Vorher fragen** ist die übliche Wahl. JeffCrunch fragt, ob die Zahlen aus den anderen Arbeitsmappen aktualisiert werden sollen.
- **Aktualisieren** aktualisiert sie, ohne zu fragen.
- **Nie aktualisieren** behält die Zahlen, die in der Datei gespeichert sind.

Eine Auswahl, die Sie für eine Arbeitsmappe unter **Daten > Verknüpfungen bearbeiten** (**Startaufforderung**) treffen, gilt für diese Arbeitsmappe und hat Vorrang. Eine Arbeitsmappe, für die „nie“ gilt, bleibt immer unverändert. Eine Arbeitsmappe auf einem anderen Computer wird beim Öffnen einer Datei nie gelesen, egal welche Auswahl Sie getroffen haben. Sie können jederzeit selbst **Werte aktualisieren** in **Verknüpfungen bearbeiten** verwenden.

## Verknüpfte Filme und Töne

Dies trifft in JeffCrunch nicht zu: Das Programm hat keine verknüpften Filme oder Töne abzuspielen. Die Zeile ist ausgegraut.

## Dateien aus dem Internet oder von anderen Computern

Eine Datei, die Sie heruntergeladen haben oder die auf einem anderen Computer in Ihrem Netzwerk liegt, ist möglicherweise nicht sicher. Diese Auswahl legt fest, wie JeffCrunch sie öffnet.

- **Zuerst schreibgeschützt öffnen** ist die übliche Wahl. Die Datei wird zum Lesen geöffnet, lässt sich aber nicht ändern. Eine Leiste unter der Datei weist darauf hin. Klicken Sie auf der Leiste auf **Bearbeitung aktivieren**, um diese eine Datei zu ändern (bis Sie das Programm schließen). Wenn Sie sie danach am selben Ort speichern, wird die Kennzeichnung als Download von der Datei entfernt. Oder verwenden Sie **Speichern unter**, um eine Kopie zu erstellen, oder wechseln Sie zur nächsten Auswahl.
- **Normal öffnen** öffnet jede Datei auf die gleiche Weise.

Eine Datei, die Sie über **Als Kopie öffnen**, **Öffnen und reparieren** oder **Schreibgeschützt öffnen** im Bereich **Datei** öffnen, wird unabhängig von dieser Einstellung auf diese Weise geöffnet.

## Verknüpfungen, die ein Programm starten

Eine Verknüpfung, die ein Programm oder ein Skript starten würde (eine Datei mit der Endung .exe, .bat, .sh und so weiter), wird **immer abgelehnt**. JeffCrunch informiert Sie darüber, und es wird nichts ausgeführt. Eine Verknüpfung auf eine Datei auf einem anderen Computer wird nie ohne Rückfrage verfolgt. Keine Einstellung ändert das.

## Siehe auch

- [JeffCrunch-Einstellungen ändern](options)
- [Notizen und Verknüpfungen](notes-links)
