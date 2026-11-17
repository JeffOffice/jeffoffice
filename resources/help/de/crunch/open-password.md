# Arbeitsmappen mit Kennwort öffnen
<!-- keywords: Kennwort, Passwort, geschützt, verschlüsselt, Verschlüsselung, öffnen, gesperrt, falsches Kennwort, abbrechen, xlsx, xls, ods, Dateischutz, Standardkennwort, privat -->

Manche Arbeitsmappen sind beim Speichern mit einem Kennwort geschützt: Ohne Kennwort kann kein Programm lesen, was in der Datei steht. Das ist etwas anderes als **Blatt schützen** und **Arbeitsmappe schützen** (siehe [Ein Blatt oder die Arbeitsmappe schützen](protect)), die nur Änderungen verhindern. Eine mit Kennwort geschützte Arbeitsmappe lässt sich erst öffnen, wenn das Kennwort eingegeben wurde. Diese Seite beschreibt, wie Sie eine solche Datei öffnen und wie Sie eine erstellen.

## Eine Datei öffnen

Öffnen Sie die Datei wie jede andere: Drücken Sie **{key:Open}**, wählen Sie die Datei aus, oder ziehen Sie sie in das Fenster. JeffCrunch erkennt, dass die Datei geschützt ist, und fragt nach dem Kennwort, bevor etwas gelesen wird:

1. Geben Sie das Kennwort in das Feld **Diese Datei ist mit einem Kennwort geschützt** ein. Die Zeichen werden als Punkte angezeigt.
2. Klicken Sie auf **Öffnen**.

Ist das Kennwort nicht richtig, teilt JeffCrunch das mit und fragt erneut, so oft Sie möchten. Klicken Sie auf **Abbrechen**, um den Vorgang zu beenden: Es wird nichts geöffnet, und die bereits geöffneten Arbeitsmappen bleiben unverändert. Kennwörter müssen genau eingegeben werden: Groß- und Kleinschreibung, Leerzeichen und Akzente zählen.

JeffCrunch speichert das Kennwort nirgendwo. Solange die Arbeitsmappe geöffnet ist, bleibt es im Arbeitsspeicher, damit **Speichern** die Datei erneut schützen kann, und es wird gelöscht, sobald Sie die Arbeitsmappe schließen. Beim nächsten Öffnen der Datei fragt das Programm erneut.

## Welche Dateien geöffnet werden können

- **.xlsx- und .xlsm-Arbeitsmappen**, die auf eine der beiden Arten geschützt sind, die das Vorgängerprogramm seit 2007 verwendet: die neuere Art (Schlüssel mit 128, 192 oder 256 Bit; JeffCrunch prüft außerdem, ob die Datei nach dem Schutz verändert wurde) und die ältere Art (AES mit einem Schlüssel von 128, 192 oder 256 Bit).
- **.xls-Arbeitsmappen**, die auf eine der drei Arten dieses Formats geschützt sind: die einfache Verschleierung der ältesten Dateien, die 40-Bit-Variante und die stärkere Variante, die den Schlüsseldienst des Computers nutzt.
- **.ods-Tabellen**, die mit einem Kennwort geschützt sind, in allen drei Arten, in denen LibreOffice und OpenOffice sie gespeichert haben: der neueste Typ (LibreOffice 25 und neuer sperrt die gesamte Datei mit einem 256-Bit-Schlüssel und erzeugt den Schlüssel aus dem Kennwort mit einem Verfahren, das absichtlich langsam ist und viel Speicher benötigt; deshalb dauert das Öffnen solcher Dateien etwas länger), der 256-Bit-Typ der Vorgängerversionen und der älteste Typ.

Bei älteren .xls-Dateien gibt es Einschränkungen für das Kennwort selbst. Die einfache Verschleierung der ältesten Dateien berücksichtigt nur die ersten 15 Zeichen eines Kennworts und kann nur Buchstaben, Ziffern und Zeichen des westeuropäischen (Latin-1) Zeichensatzes enthalten: Ein Kennwort mit anderen Buchstaben (etwa griechischen oder chinesischen) wird dort nie als richtig erkannt. Die 40-Bit-Variante akzeptiert beliebige Buchstaben, verwendet aber nur die ersten 16 Zeichen. JeffCrunch schreibt eine .xls-Kopie mit Kennwort in der stärkeren Variante (die den Schlüsseldienst des Computers nutzt, mit einem 128-Bit-Schlüssel), weil die beiden anderen schwächer sind. Diese Variante verwendet die ersten 255 Zeichen des Kennworts, das aus beliebigen Buchstaben bestehen darf.

Manche Dateien sind mit einem allgemein bekannten Kennwort gesperrt, damit sie von jedem geöffnet, aber nicht versehentlich verändert werden können. JeffCrunch öffnet solche Dateien ohne Nachfrage.

## Wenn eine Datei nicht geöffnet werden kann

- **„Das Kennwort ist nicht korrekt.“** Das eingegebene Kennwort ist nicht das, mit dem die Datei geschützt wurde. Versuchen Sie es erneut und achten Sie auf Groß- und Kleinschreibung sowie Leerzeichen. JeffCrunch kann ein verlorenes Kennwort nicht wiederherstellen.
- **„Diese Datei ist auf eine Weise geschützt, die dieses Programm nicht öffnen kann.“** Die Datei ist durch etwas anderes als ein Kennwort gesperrt (zum Beispiel durch einen Schlüssel aus einem Zertifikat) oder in einem Format, das JeffCrunch nicht lesen kann.
- **„Der Kennwortschutz dieser Datei verlangt mehr Arbeitsspeicher oder Zeit, als dieses Programm zulässt, daher wird sie nicht geöffnet.“** Eine Datei von LibreOffice 25 oder neuer legt fest, wie viel Speicher und Zeit die Kennwortprüfung benötigt. JeffCrunch akzeptiert höchstens das Zehnfache der Werte, die LibreOffice selbst festlegt (640 MiB Arbeitsspeicher), damit eine Datei, die den Computer lahmlegen soll, nicht geöffnet wird. Eine von LibreOffice selbst geschriebene Datei liegt deutlich darunter.
- **„Die Datei ist beschädigt oder wurde nach dem Schutz verändert.“** Das Kennwort war richtig, der Inhalt der Datei entspricht aber nicht dem, was geschützt wurde: Sie wurde abgeschnitten, beschädigt oder von einem anderen Programm verändert. Kopieren Sie die Datei erneut aus dem Original.

## Mit Kennwort speichern

Klicken Sie auf **Datei** und dann auf **Mit Kennwort schützen** (oder auf **Überprüfen > Mit Kennwort schützen**), geben Sie das Kennwort zweimal ein und klicken Sie auf **Schützen**. Die Arbeitsmappe wird beim nächsten Speichern geschützt geschrieben: Wer die Datei öffnet, muss das Kennwort eingeben. Um das Kennwort zu ändern, wählen Sie denselben Befehl und geben ein neues Kennwort ein. Um es zu entfernen, klicken Sie auf **Kennwort entfernen** und speichern Sie.

Eine Arbeitsmappe, die Sie mit Kennwort geöffnet haben, behält es, solange sie geöffnet ist. **Speichern** schreibt die Datei daher wieder geschützt mit demselben Kennwort. JeffCrunch hält das Kennwort nur so lange im Arbeitsspeicher, bis Sie die Arbeitsmappe schließen, und löscht es dann. Die Datei wird nicht versehentlich ungeschützt: Eine als .xls-Datei gespeicherte Kopie ist mit demselben Kennwort in der stärkeren .xls-Variante geschützt, und eine als .ods- oder .ots-Datei gespeicherte Kopie ist auf die Weise geschützt, die der OpenDocument-Standard beschreibt. So können andere Programme, die diese Standards unterstützen, die Kopie mit dem Kennwort öffnen. Eine Textkopie lässt sich nicht schützen. JeffCrunch weist Sie deshalb vorher darauf hin, dass die Kopie nicht geschützt wird, und Sie können den Vorgang abbrechen.

Verwenden Sie ein langes Kennwort und bewahren Sie es sicher auf: Niemand, auch wir nicht, kann die Datei ohne Kennwort öffnen.
