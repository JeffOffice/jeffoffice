# Kennwortgeschützte Arbeitsmappen öffnen
<!-- keywords: Kennwort, Passwort, geschützt, kennwortgeschützt, verschlüsselt, Verschlüsselung, öffnen, gesperrt, falsches Kennwort, abbrechen, xlsx, xls, ods, Dateischutz, Standardkennwort, privat -->

Manche Arbeitsmappen werden beim Speichern mit einem Kennwort geschützt: Ohne das Kennwort kann kein Programm den Inhalt der Datei lesen. Das ist etwas anderes als **Blatt schützen** und **Arbeitsmappe schützen** (siehe [Ein Blatt oder die Arbeitsmappe schützen](protect)), die nur Änderungen verhindern. Eine Arbeitsmappe, die mit einem Kennwort geschützt ist, lässt sich überhaupt nicht öffnen, bis das Kennwort eingegeben wird. Auf dieser Seite erfahren Sie, wie Sie eine solche Arbeitsmappe öffnen und wie Sie eine erstellen.

## Eine geschützte Arbeitsmappe öffnen

Öffnen Sie die Datei wie jede andere: Drücken Sie **{key:Open}**, wählen Sie die Datei aus, oder ziehen Sie sie in das Fenster. JeffCrunch erkennt, dass die Datei geschützt ist, und fragt nach dem Kennwort, bevor es etwas liest:

1. Geben Sie das Kennwort in das Feld mit dem Text **Diese Datei ist kennwortgeschützt. Kennwort:** ein. Die Zeichen werden als Punkte angezeigt.
2. Klicken Sie auf **Öffnen**.

Ist das Kennwort falsch, meldet JeffCrunch dies und fragt erneut, so oft Sie möchten. Klicken Sie auf **Abbrechen**, um aufzuhören: Es wird nichts geöffnet, und bereits geöffnete Arbeitsmappen bleiben unverändert. Kennwörter müssen exakt stimmen: Großbuchstaben, Leerzeichen und Akzente zählen.

JeffCrunch speichert das Kennwort nirgends. Es hält es im Arbeitsspeicher, solange die Arbeitsmappe geöffnet ist, damit **Speichern** die Datei wieder schützen kann, und löscht es, wenn Sie die Arbeitsmappe schließen. Beim nächsten Öffnen der Datei wird erneut nach dem Kennwort gefragt.

## Was geöffnet werden kann

- **.xlsx- und .xlsm-Arbeitsmappen**, die auf eine der beiden Arten geschützt sind, die das ursprüngliche Programm seit 2007 verwendet: die neuere (der Schlüssel hat 128, 192 oder 256 Bit, und JeffCrunch prüft außerdem, dass niemand die Datei nach dem Schützen geändert hat) und die ältere (AES mit einem Schlüssel von 128, 192 oder 256 Bit).
- **.xls-Arbeitsmappen**, die auf eine der drei Arten dieses Formats geschützt sind: die einfache Verschleierung der ältesten Dateien, die 40-Bit-Verschlüsselung und die stärkere Verschlüsselung, die den Kryptografiedienst des Computers nutzt.
- **.ods-Tabellen**, die mit einem Kennwort geschützt sind, in allen drei Arten, die LibreOffice und OpenOffice geschrieben haben: die neueste (LibreOffice ab Version 25 verschlüsselt die ganze Datei mit einem 256-Bit-Schlüssel und leitet den Schlüssel mit einem absichtlich langsamen und speicherintensiven Verfahren aus dem Kennwort ab, sodass das Öffnen einer solchen Datei etwas länger dauert), die 256-Bit-Verschlüsselung der vorherigen Versionen und die älteste Art.

Ältere .xls-Dateien haben Einschränkungen beim Kennwort selbst. Die einfache Verschleierung der ältesten Dateien verwendet nur die ersten 15 Zeichen eines Kennworts und kann nur Buchstaben, Ziffern und Zeichen des westeuropäischen Zeichensatzes (Latin-1) enthalten: Ein Kennwort mit anderen Schriftzeichen (etwa griechischen oder chinesischen) wird dort nie als richtig erkannt. Die 40-Bit-Verschlüsselung akzeptiert beliebige Zeichen, aber nur die ersten 16. JeffCrunch schreibt keine .xls-Dateien mit Kennwort.

Manche Dateien sind mit einem allgemein bekannten Kennwort gesperrt, damit jeder sie öffnen, aber niemand sie versehentlich ändern kann. JeffCrunch öffnet solche Dateien, ohne nachzufragen.

## Wenn sich eine Datei nicht öffnen lässt

- **„Das Kennwort ist nicht korrekt.“** Das eingegebene Kennwort ist nicht das, mit dem die Datei geschützt wurde. Versuchen Sie es erneut, und achten Sie auf Großbuchstaben und Leerzeichen. JeffCrunch kann ein verlorenes Kennwort nicht wiederherstellen.
- **„Diese Datei ist auf eine Weise geschützt, die dieses Programm nicht öffnen kann.“** Die Datei ist nicht mit einem Kennwort gesperrt, sondern auf andere Weise (zum Beispiel mit einem Schlüssel aus einem Zertifikat), oder in einem Format, das JeffCrunch nicht liest.
- **„Der Kennwortschutz dieser Datei erfordert mehr Arbeitsspeicher oder Zeit, als dieses Programm zulässt. Die Datei wird daher nicht geöffnet.“** Eine Datei aus LibreOffice ab Version 25 legt fest, wie viel Arbeitsspeicher und Zeit die Prüfung des Kennworts beansprucht. JeffCrunch akzeptiert bis zum Zehnfachen dessen, was LibreOffice selbst festlegt (640 MiB Arbeitsspeicher), damit eine Datei, die Ihren Computer lahmlegen soll, dies nicht kann. Eine von LibreOffice selbst geschriebene Datei liegt weit innerhalb dieser Grenze.
- **„Die Datei ist beschädigt oder wurde nach dem Schützen geändert.“** Das Kennwort war richtig, aber der Inhalt der Datei entspricht nicht dem, was gesperrt wurde: Sie wurde abgeschnitten, beschädigt oder von einem anderen Programm geändert. Kopieren Sie sie erneut vom Original.

## Mit Kennwort speichern

Klicken Sie auf **Datei** und dann auf **Mit Kennwort schützen** (oder **Überprüfen > Mit Kennwort schützen**), geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Arbeitsmappe wird beim nächsten Speichern geschützt geschrieben: Wer die Datei öffnet, muss das Kennwort eingeben. Um das Kennwort zu ändern, wählen Sie denselben Befehl und geben ein neues ein. Um es zu entfernen, klicken Sie auf **Kennwort entfernen** und speichern.

Eine Arbeitsmappe, die Sie mit einem Kennwort geöffnet haben, behält es, solange sie geöffnet ist. **Speichern** schreibt die Datei also wieder mit demselben Kennwort geschützt. JeffCrunch hält das Kennwort nur bis zum Schließen der Arbeitsmappe im Arbeitsspeicher und löscht es dann. Die mit dem Kennwort geöffnete Datei wird nicht versehentlich ungeschützt gespeichert: Wenn Sie eine Kopie als .xls- oder .ods-Datei speichern, die JeffCrunch nicht schützen kann, weist es Sie vorher darauf hin, dass die Kopie nicht geschützt ist, und Sie können abbrechen.

Verwenden Sie ein langes Kennwort, und bewahren Sie es sicher auf: Niemand, auch wir nicht, kann die Datei ohne das Kennwort öffnen.
