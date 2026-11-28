# Sicherheit: worauf eine Datei zugreifen darf
<!-- keywords: Vertrauensstellung, Sicherheit, Makros, verknüpfte Dateien, Verknüpfungen, verknüpfte Bilder, verknüpfte Medien, Internet, heruntergeladen, schreibgeschützt, geschützt, externe Verknüpfungen, Programme, sicher -->

Eine Datei kann auf andere Dinge verweisen: auf andere Dateien, auf Webseiten, auf Filme und sogar auf Programme. Auf der Seite **Sicherheit** legen Sie fest, wie JeffCrunch damit umgeht. Klicken Sie auf **Datei > Optionen** und dann auf die Registerkarte **Sicherheit**. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

Diese Einstellungen gelten für alle JeffOffice-Apps: Ändern Sie eine in einer App, ändert sie sich in allen. Eine Zeile für etwas, das eine App nicht besitzt, zeigt **Trifft nicht zu** und ist ausgegraut.

## Makros

Makros sind kleine Programme, die in einer Datei gespeichert sind. JeffCrunch bewahrt sie in der Datei auf, sodass sie beim Speichern erhalten bleiben, und führt sie **niemals aus**. Es gibt keine Einstellung, die sie ausführt.

## Verknüpfte Dateien und Arbeitsmappenverknüpfungen

Hier geht es um die anderen Dateien, die eine Arbeitsmappe liest, etwa die Arbeitsmappen, aus denen sie Zahlen bezieht und die als **Verknüpfungen** bezeichnet werden, die Dateien, die ihre Abfragen lesen, und die Dateien, aus denen ihre Pivot-Tabellen erstellt sind. Die Einstellung legt fest, was JeffCrunch tut, wenn es eine Arbeitsmappe öffnet, die solche Dateien liest, und wenn Sie bei ihren Abfragen und Pivot-Tabellen **Aktualisieren** oder **Alle aktualisieren** verwenden.

- **Fragen, bevor die Datei gelesen wird** ist die übliche Wahl. Bevor JeffCrunch eine dieser Dateien liest, zeigt es den vollständigen Pfad der Datei an und fragt nach. Klicken Sie nur dann auf **Datei lesen**, wenn Sie der Quelle der Arbeitsmappe vertrauen. **Nicht lesen** ist die Standardschaltfläche; sie liest nichts und lässt die Arbeitsmappe unverändert. Diese Einstellung fragt bei jedem Öffnen einer Arbeitsmappe nach, und danach einmal für jede Datei. Nachdem Sie für eine Datei auf **Datei lesen** geklickt haben, fragt JeffCrunch in diesem Fenster nicht erneut nach derselben Datei, bis Sie das Fenster schließen.
- **Werte aktualisieren** liest eine Datei nur dann ohne Nachfrage, wenn Sie bereits zugestimmt haben, dass genau diese Arbeitsmappe genau diese Datei liest. Bei jeder anderen Datei, auch bei einer Datei, die in einer Arbeitsmappe genannt wird, die Ihnen gerade jemand geschickt hat, fragt JeffCrunch vorher auf dieselbe Weise nach.
- **Niemals aktualisieren** liest beim Öffnen einer Arbeitsmappe nichts und zeigt keine Frage an. Die Zahlen, die die Datei gespeichert hat, bleiben unverändert. Wenn Sie selbst auf **Aktualisieren** klicken, ist das eine Aufforderung, und JeffCrunch zeigt dann die Frage an. Nachdem Sie auf **Datei lesen** geklickt haben, fragt es in diesem Fenster nicht erneut nach derselben Datei.

## Was gemerkt wird

Wenn Sie auf **Datei lesen** oder **Dateien lesen** klicken, merkt sich JeffCrunch dieses Paar: diese Arbeitsmappe und diese Datei, jeweils mit vollständigem Pfad. Nur das Paar wird gemerkt. Wird die Arbeitsmappe verschoben oder umbenannt, so bearbeitet, dass sie eine andere Datei liest, oder wird die Datei durch eine Verknüpfung auf eine andere Datei ersetzt, fragt JeffCrunch erneut. Eine Arbeitsmappe, die Sie noch nie gespeichert haben, hat keinen Pfad, daher wird nach dem Schließen nichts über sie gemerkt.

Die Paare werden nur in Ihren eigenen Einstellungen auf diesem Computer gespeichert, nie in einer Arbeitsmappe. Daher kann eine Arbeitsmappe kein Paar hinzufügen, und niemand, der eine Arbeitsmappe von Ihnen erhält, bekommt diese Paare. Die Einstellungen speichern eine verschlüsselte Form jedes Pfads, nicht den Pfad selbst. Es werden bis zu 2.000 Paare gespeichert; sind es mehr, wird zuerst das Paar vergessen, das am längsten zurückliegend verwendet wurde.

**Optionen > Sicherheit** zeigt, wie viele Paare gemerkt sind (**Gemerkte Zustimmungen**). Klicken Sie auf **Alle vergessen**, um alle auf einmal zu vergessen. Keine Arbeitsmappe wird dabei geändert, und bei jeder fragt JeffCrunch beim nächsten Mal erneut nach.

Eine Arbeitsmappe, die in ihrer eigenen **Startaufforderung** (**Daten > Verknüpfungen bearbeiten**) „niemals“ festlegt, wird immer in Ruhe gelassen. Eine Arbeitsmappe auf einem anderen Computer wird beim Öffnen einer Datei nie gelesen, egal welche Einstellung Sie gewählt haben. Sie können jederzeit selbst **Werte aktualisieren** in **Verknüpfungen bearbeiten** verwenden.

## Verknüpfte Filme und Töne

Dies trifft in JeffCrunch nicht zu: Das Programm hat keine verknüpften Filme oder Töne abzuspielen. Die Zeile ist ausgegraut.

## Dateien aus dem Internet oder von anderen Computern

Eine Datei, die Sie heruntergeladen haben oder die auf einem anderen Computer in Ihrem Netzwerk liegt, ist möglicherweise nicht sicher. Diese Einstellung legt fest, wie JeffCrunch sie öffnet.

- **Zuerst schreibgeschützt öffnen** ist die übliche Wahl. Die Datei öffnet sich, damit Sie sie lesen können, Sie können sie aber nicht ändern. Eine Leiste unter der Datei weist darauf hin. Klicken Sie in der Leiste auf **Bearbeitung aktivieren**, um nur diese Datei zu ändern (bis Sie das Programm schließen). Wenn Sie sie danach am selben Ort speichern, wird die Kennzeichnung als heruntergeladen von der Datei entfernt. Oder verwenden Sie **Speichern unter**, um eine Kopie anzulegen, oder wechseln Sie zu der nächsten Einstellung.
- **Normal öffnen** öffnet jede Datei auf dieselbe Weise.

Eine Datei, die Sie über den Bereich **Datei** mit **Als Kopie öffnen**, **Öffnen und reparieren** oder **Schreibgeschützt öffnen** öffnen, wird unabhängig von dieser Einstellung auf diese Weise geöffnet.

## Verknüpfungen, die ein Programm starten

Eine Verknüpfung, die ein Programm oder ein Skript starten würde (eine Datei mit der Endung .exe, .bat, .sh und so weiter), wird **immer abgelehnt**. JeffCrunch informiert Sie darüber, und nichts wird ausgeführt. Eine Verknüpfung auf eine Datei auf einem anderen Computer wird nie ohne Rückfrage verfolgt. Keine Einstellung ändert das.

## Siehe auch

- [JeffCrunch-Einstellungen ändern](options)
- [Notizen und Verknüpfungen](notes-links)
