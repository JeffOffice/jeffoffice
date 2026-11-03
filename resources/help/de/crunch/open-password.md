# Kennwortgeschützte Arbeitsmappen öffnen
<!-- keywords: kennwort, passwort, geschützt, verschlüsselt, verschlüsselung, öffnen, gesperrt, falsches kennwort, abbrechen, xlsx, xls, ods, dateischutz, standardkennwort, privat -->

Manche Arbeitsmappen werden beim Speichern mit einem Kennwort geschützt: Ohne das Kennwort kann kein Programm lesen, was in der Datei steht. Das ist etwas anderes als **Blatt schützen** und **Arbeitsmappe schützen** (siehe [Blatt oder Arbeitsmappe schützen](protect)), die nur Änderungen verhindern. Eine Arbeitsmappe mit Kennwortschutz lässt sich überhaupt nicht öffnen, bevor das Kennwort eingegeben wurde. Diese Seite sagt, wie Sie eine solche Arbeitsmappe öffnen und wie Sie eine erstellen.

## Eine Arbeitsmappe öffnen

Öffnen Sie die Datei wie jede andere: Drücken Sie **{key:Open}**, wählen Sie sie aus, oder ziehen Sie sie auf das Fenster. JeffCrunch erkennt, dass die Datei geschützt ist, und fragt nach dem Kennwort, bevor es etwas liest:

1. Geben Sie das Kennwort in das Feld ein, das mit **Diese Datei ist durch ein Kennwort geschützt.** beschriftet ist. Die Zeichen erscheinen als Punkte.
2. Klicken Sie auf **Öffnen**.

Ist das Kennwort nicht richtig, sagt JeffCrunch das und fragt erneut, beliebig oft. Mit **Abbrechen** hören Sie auf: Es wird nichts geöffnet, und die Arbeitsmappen, die Sie schon geöffnet haben, bleiben unverändert. Kennwörter müssen genau stimmen: Groß- und Kleinschreibung, Leerzeichen und Akzente zählen.

JeffCrunch schreibt das Kennwort nirgendwohin. Es hält es im Arbeitsspeicher, solange die Arbeitsmappe geöffnet ist, damit **Speichern** die Datei wieder schützen kann, und löscht es, wenn Sie die Arbeitsmappe schließen. Beim nächsten Öffnen der Datei fragt es erneut.

## Was sich öffnen lässt

- **.xlsx- und .xlsm-Arbeitsmappen**, die auf eine der beiden Arten geschützt sind, die das Originalprogramm seit 2007 verwendet: die neuere (der Schlüssel hat 128, 192 oder 256 Bit, und JeffCrunch prüft außerdem, dass niemand die Datei nach dem Schützen verändert hat) und die ältere (AES mit einem Schlüssel von 128, 192 oder 256 Bit).
- **.xls-Arbeitsmappen**, die auf eine der drei Arten dieses Formats geschützt sind: die einfache Verschleierung der ältesten Dateien, die 40-Bit-Art und die stärkere Art, die den Schlüsseldienst des Computers verwendet.
- **.ods-Tabellen** mit Kennwortschutz, auf allen drei Arten, auf die LibreOffice und OpenOffice sie geschrieben haben: die neueste (LibreOffice 25 und neuer sperren die ganze Datei mit einem 256-Bit-Schlüssel und machen den Schlüssel aus dem Kennwort mit einem Verfahren, das absichtlich langsam ist und viel Arbeitsspeicher braucht; das Öffnen einer solchen Datei dauert daher einen Moment länger), die 256-Bit-Art der Versionen davor und die älteste Art.

Für ältere .xls-Dateien gelten Grenzen für das Kennwort selbst. Die einfache Verschleierung der ältesten Dateien verwendet nur die ersten 15 Zeichen eines Kennworts und kann nur Buchstaben, Ziffern und Zeichen des westlichen Zeichensatzes (Latin-1) enthalten: Ein Kennwort mit anderen Buchstaben (etwa griechischen oder chinesischen) wird dort nie als das richtige erkannt. Die 40-Bit-Art akzeptiert beliebige Buchstaben, aber nur die ersten 16 Zeichen. JeffCrunch schreibt keine .xls-Dateien mit Kennwort.

Manche Dateien sind mit einem Kennwort gesperrt, das jeder kennt, damit sie jeder öffnen, aber niemand versehentlich ändern kann. JeffCrunch öffnet diese, ohne zu fragen.

## Wenn sich eine Datei nicht öffnen lässt

- **„Das Kennwort ist nicht richtig.“** Das eingegebene Kennwort ist nicht das, mit dem die Datei geschützt wurde. Versuchen Sie es noch einmal und achten Sie auf Groß- und Kleinschreibung und Leerzeichen; JeffCrunch kann ein vergessenes Kennwort nicht wiederherstellen.
- **„Diese Datei ist auf eine Weise geschützt, die dieses Programm nicht öffnen kann.“** Die Datei ist durch etwas anderes als ein Kennwort gesperrt (zum Beispiel durch einen Schlüssel, der in einem Zertifikat liegt) oder in einer Form, die JeffCrunch nicht liest.
- **„Der Kennwortschutz dieser Datei verlangt mehr Arbeitsspeicher oder Zeit, als dieses Programm zulässt; sie wird nicht geöffnet.“** Eine Datei von LibreOffice 25 oder neuer legt fest, wie viel Arbeitsspeicher und Zeit die Prüfung des Kennworts braucht. JeffCrunch akzeptiert bis zum Zehnfachen dessen, was LibreOffice selbst festlegt (640 MiB Arbeitsspeicher), damit eine Datei, die Ihren Computer lahmlegen soll, das nicht kann. Eine Datei, die LibreOffice selbst geschrieben hat, liegt weit darunter.
- **„Die Datei ist beschädigt oder wurde nach dem Schützen verändert.“** Das Kennwort war richtig, aber der Inhalt der Datei entspricht nicht dem, was gesperrt wurde: Sie wurde abgeschnitten, beschädigt oder von einem anderen Programm verändert. Kopieren Sie sie erneut vom Original.

## Mit Kennwort speichern

Klicken Sie auf **Datei** und dann auf **Mit Kennwort schützen** (oder auf **Überprüfen > Mit Kennwort schützen**), geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Arbeitsmappe wird beim nächsten Speichern geschützt geschrieben: Wer die Datei öffnet, muss das Kennwort eingeben. Um das Kennwort zu ändern, wählen Sie denselben Befehl und geben ein neues ein; um es zu entfernen, klicken Sie auf **Kennwort entfernen** und speichern.

Eine Arbeitsmappe, die Sie mit einem Kennwort geöffnet haben, behält es, solange sie geöffnet ist. **Speichern** schreibt die Datei daher wieder mit demselben Kennwort geschützt. JeffCrunch hält das Kennwort nur im Arbeitsspeicher, bis Sie die Arbeitsmappe schließen, und löscht es dann. Die Datei wird nicht versehentlich ungeschützt gespeichert: Wenn Sie eine Kopie als .xls- oder .ods-Datei speichern, die JeffCrunch nicht schützen kann, sagt es Ihnen vorher, dass die Kopie nicht geschützt ist, und Sie können abbrechen.

Verwenden Sie ein langes Kennwort, und bewahren Sie es an einem sicheren Ort auf: Niemand, auch wir nicht, kann die Datei ohne das Kennwort öffnen.
