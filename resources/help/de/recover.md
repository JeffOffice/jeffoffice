# Nicht gespeicherte Arbeit wiederherstellen
<!-- keywords: wiederherstellen, Wiederherstellung, AutoWiederherstellen, automatisch wiederherstellen, automatisch speichern, Absturz, abgestürzt, verlorene Arbeit, nicht gespeichert, Stromausfall, zurückholen, eingefroren, hängt, versehentlich geschlossen, Sicherung, Sicherungskopie -->

Wenn JeffPub unerwartet beendet wird oder der Strom ausfällt, ist Ihre letzte Arbeit möglicherweise noch vorhanden. Während Sie arbeiten, hält JeffPub eine Ersatzkopie jeder Publikation bereit, die nicht gespeicherte Änderungen enthält. Diese Funktion heißt **AutoWiederherstellen**.

## So funktioniert AutoWiederherstellen

- Alle paar Minuten speichert JeffPub eine Ersatzkopie jeder Publikation mit nicht gespeicherten Änderungen. Standardmäßig geschieht das alle 10 Minuten.
- JeffPub legt die Kopien in einem AutoWiederherstellen-Ordner ab, der sich in dem Ordner für die eigenen Programmdaten befindet. Sie müssen dort nie selbst nachsehen.
- Die Ersatzkopie ersetzt nie Ihre eigentliche Datei.
- Die Kopie wird entfernt, wenn Sie die Publikation speichern, schließen oder JeffPub normal beenden.

AutoWiederherstellen ersetzt nicht das Speichern. Wenn JeffPub abbricht, kann die Arbeit der letzten Minuten verloren gehen. Drücken Sie oft Strg+S.

## Arbeit wiederherstellen

Wenn Sie JeffPub nach einem Absturz das nächste Mal starten, wird das Fenster **Nicht gespeicherte Arbeit wiederherstellen** automatisch geöffnet. Jede Zeile zeigt den Namen der Publikation, den Speicherort der Datei (oder „nie gespeichert“) und den Zeitpunkt, zu dem die Kopie erstellt wurde.

1. Lassen Sie das Häkchen bei jeder Publikation stehen, die Sie zurückhaben möchten. Entfernen Sie das Häkchen bei den anderen.
2. Klicken Sie auf **Öffnen**.
3. Prüfen Sie die Arbeit, und drücken Sie dann Strg+S.

Die wiederhergestellte Publikation zeigt in der Titelleiste „(Wiederhergestellt)“ an, bis Sie sie speichern. Bei Strg+S werden Sie nach einem Namen gefragt; dabei sind der Ordner der Originaldatei und der Originalname bereits vorgegeben. Wählen Sie den Originalnamen, um die Datei zu ersetzen, oder einen neuen Namen, um beide zu behalten.

Mit Dateien verknüpfte Bilder werden so gefunden, als wäre die Publikation im Ordner der Originaldatei geöffnet: Bilder in diesem Ordner oder in einem Unterordner davon werden wie gewohnt geladen. Ein Bild, das mit einer Datei an einem anderen Ort verknüpft ist, zeigt seine gespeicherte Vorschau und ist als **Nicht aktualisiert** gekennzeichnet. Wählen Sie es aus, und klicken Sie im [Grafik-Manager](graphics-manager) auf **Verknüpfung aktualisieren**. Eine nie gespeicherte Publikation hat keinen Ordner; daher warten alle ihre verknüpften Bilder auf **Verknüpfung aktualisieren**.

Die anderen Schaltflächen im Fenster:

- **Löschen** entfernt die markierten Kopien endgültig, nachdem Sie dies bestätigt haben.
- **Nicht jetzt** schließt das Fenster und behält die Kopien. JeffPub bietet sie beim nächsten Start erneut an.

JeffPub bietet nur Kopien aus einer Sitzung an, die unerwartet beendet wurde. Kopien aus einem noch geöffneten JeffPub-Fenster werden nicht angeboten.

## Wiederhergestellte Arbeit später finden

Wenn Sie das Fenster geschlossen haben und Ihre Kopien zurückhaben möchten, klicken Sie auf **Datei > Öffnen** und dann auf **Nicht gespeicherte Arbeit wiederherstellen**. Wenn nichts wiederherzustellen ist, teilt JeffPub Ihnen das mit.

## Speicherintervall ändern

Klicken Sie auf **Datei > Optionen** und dann auf die Registerkarte **Speichern**. Ändern Sie **AutoWiederherstellen-Informationen speichern alle** auf einen Wert zwischen 1 und 120 Minuten. Je kleiner der Wert, desto weniger kann verloren gehen. Die neue Zeit wird erst beim nächsten Start von JeffPub vollständig wirksam.

## Verwandte Themen

- [Publikationen öffnen und speichern](open-save)
- [Einstellungen von JeffPub ändern](options)
- [Hilfe erhalten und Probleme melden](support)
