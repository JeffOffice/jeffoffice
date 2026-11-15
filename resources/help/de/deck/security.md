# Sicherheit: worauf eine Datei zugreifen darf
<!-- keywords: Vertrauensstellung, Sicherheit, Makros, verknüpfte Dateien, Verknüpfungen, verknüpfte Bilder, verknüpfte Medien, Internet, heruntergeladen, schreibgeschützt, geschützt, externe Verknüpfungen, Programme, sicher -->

Eine Datei kann auf andere Dinge verweisen: auf Dateien, Webseiten, Videos und sogar Programme. Auf der Seite **Sicherheit** legen Sie fest, wie JeffDeck damit umgeht. Klicken Sie auf **Datei > Optionen** und dann auf die Registerkarte **Sicherheit**. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

Diese Einstellungen gelten für alle JeffOffice-Apps: Wenn Sie eine davon in einer App ändern, wird sie in allen geändert. Eine Zeile für etwas, das eine App nicht besitzt, zeigt **Dies trifft nicht zu** und ist ausgegraut.

## Makros

Makros sind kleine Programme, die in einer Datei gespeichert sind. JeffDeck behält sie in der Datei, sodass sie beim Speichern erhalten bleiben, und **führt sie nie aus**. Es gibt keine Einstellung, die sie ausführt.

## Verknüpfte Dateien und Arbeitsmappenverknüpfungen

Dies trifft in JeffDeck nicht zu: Das Programm hat keine verknüpften Dateien zu aktualisieren. Die Zeile ist ausgegraut.

## Verknüpfte Videos und Töne

Hier geht es um ein Video oder einen Ton, auf den eine Folie **verweist**, statt ihn in der Präsentation zu speichern.

- **Nie abspielen** ist die übliche Wahl. Ein verknüpftes Video oder ein verknüpfter Ton wird als verknüpft gekennzeichnet und nicht abgespielt.
- **Vorher fragen** fragt beim ersten Abspielen einmal für jede verknüpfte Datei nach.
- **Abspielen** spielt die Datei ohne Nachfrage ab.

Selbst bei **Abspielen** wird nur eine Video- oder Tondatei abgespielt, deren Name auf diesem Computer liegt. Eine Webadresse, eine Datei auf einem anderen Computer, ein Programm, eine Verknüpfung oder eine Spezialdatei und eine Datei, die kein Video und kein Ton ist, werden nie abgespielt. Eine verknüpfte Datei darf bis zu 256 MB groß sein. Die Datei wird bis zu 20 Sekunden im Hintergrund gelesen, sodass ein langsamer Datenträger JeffDeck nicht einfrieren lässt. Ein Netzlaufwerk, das wie ein lokaler Datenträger aussieht (ein zugeordnetes Laufwerk oder ein Ordner unter /Volumes oder /mnt), wird wie eine lokale Datei gelesen und kann daher langsam sein. Bei **Vorher fragen** fragt eine Bildschirmpräsentation einmal beim Start nach allen darin enthaltenen verknüpften Dateien.

## Dateien aus dem Internet oder von anderen Computern

Eine Datei, die Sie heruntergeladen haben oder die sich in Ihrem Netzwerk auf einem anderen Computer befindet, ist unter Umständen nicht sicher. Diese Option legt fest, wie JeffDeck sie öffnet.

- **Zuerst schreibgeschützt öffnen** ist die übliche Wahl. Die Datei wird zum Lesen geöffnet, lässt sich aber nicht ändern. Eine Leiste unter der Datei weist darauf hin. Klicken Sie auf der Leiste auf **Bearbeitung aktivieren**, um nur diese Datei zu ändern (bis Sie das Programm schließen). Wenn Sie sie danach am selben Ort speichern, wird die Download-Markierung von der Datei entfernt. Oder verwenden Sie **Speichern unter**, um eine Kopie anzulegen, oder ändern Sie diese Einstellung auf die nächste Option.
- **Normal öffnen** öffnet jede Datei auf dieselbe Weise.

Eine Datei, die Sie über **Als Kopie öffnen**, **Öffnen und reparieren** oder **Schreibgeschützt öffnen** im Bereich **Datei** öffnen, wird unabhängig von dieser Einstellung auf diese Weise geöffnet.

## Verknüpfungen, die ein Programm starten

Eine Verknüpfung, die ein Programm oder ein Skript startet (eine Datei mit der Endung .exe, .bat, .sh und so weiter), wird **immer abgelehnt**. JeffDeck meldet das, und nichts wird ausgeführt. Ein Link zu einer Datei auf einem anderen Computer wird nie ohne Rückfrage geöffnet. Keine Einstellung ändert das.

## Siehe auch

- [JeffDeck-Einstellungen ändern](options)
- [Bildschirmpräsentation ausführen](show)
