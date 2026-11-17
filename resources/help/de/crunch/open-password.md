# Arbeitsmappen mit Kennwort öffnen

<!-- keywords: Kennwort, Passwort, geschützt, verschlüsselt, Verschlüsselung, öffnen, gesperrt, falsches Kennwort, abbrechen, xlsx, xls, ods, Dateischutz, Standardkennwort, privat -->

Manche Arbeitsmappen sind beim Speichern mit einem Kennwort geschützt: Ohne Kennwort kann kein Programm lesen, was in der Datei steht. Das ist etwas anderes als **Blatt schützen** und **Arbeitsmappe schützen** (siehe [Blatt oder Arbeitsmappe schützen](protect)), die nur Änderungen verhindern. Eine Arbeitsmappe, die mit einem Kennwort geschützt ist, lässt sich erst öffnen, wenn das Kennwort eingegeben wurde. Diese Seite beschreibt, wie Sie eine solche Arbeitsmappe öffnen und wie Sie eine erstellen.

## Eine Arbeitsmappe öffnen

Öffnen Sie die Datei wie jede andere: Drücken Sie **{key:Open}**, wählen Sie die Datei aus, oder ziehen Sie sie in das Fenster. JeffCrunch erkennt, dass die Datei geschützt ist, und fragt nach dem Kennwort, bevor etwas gelesen wird:

1. Geben Sie das Kennwort in das Feld ein, das mit **Diese Datei ist mit einem Kennwort geschützt.** beginnt. Die Zeichen werden als Punkte angezeigt.
2. Klicken Sie auf **Öffnen**.

Ist das Kennwort falsch, meldet JeffCrunch das und fragt erneut nach, so oft Sie möchten. Klicken Sie auf **Abbrechen**, um aufzuhören: Es wird nichts geöffnet, und die bereits geöffneten Arbeitsmappen bleiben unverändert. Kennwörter müssen genau stimmen: Groß- und Kleinschreibung, Leerzeichen und Akzente zählen.

JeffCrunch speichert das Kennwort nirgendwo. Solange die Arbeitsmappe geöffnet ist, bleibt es im Arbeitsspeicher, damit **Speichern** die Datei wieder schützen kann, und es wird gelöscht, wenn Sie die Arbeitsmappe schließen. Beim nächsten Öffnen der Datei wird erneut danach gefragt.

## Was sich öffnen lässt

- **.xlsx- und .xlsm-Arbeitsmappen**, die auf eine der beiden Arten geschützt sind, die das ursprüngliche Programm seit 2007 verwendet: die neuere Art (der Schlüssel hat 128, 192 oder 256 Bit; JeffCrunch prüft außerdem, ob die Datei nach dem Schutz unverändert blieb) und die ältere Art (AES mit einem Schlüssel von 128, 192 oder 256 Bit).
- **.xls-Arbeitsmappen**, die auf eine der drei Arten dieses Formats geschützt sind: die einfache Verschleierung der ältesten Dateien, die Art mit 40-Bit-Schlüssel und die stärkere Art, die den Schlüsseldienst des Computers nutzt.
- **.ods-Tabellen**, die mit einem Kennwort geschützt sind, in allen drei Arten, in denen LibreOffice und OpenOffice sie geschrieben haben: der neuesten (LibreOffice 25 und neuer sperren die ganze Datei mit einem 256-Bit-Schlüssel und erzeugen den Schlüssel aus dem Kennwort mit einem Verfahren, das absichtlich langsam ist und viel Arbeitsspeicher benötigt; deshalb dauert das Öffnen solcher Dateien etwas länger), der 256-Bit-Art der Vorversionen und der ältesten Art.

Bei älteren .xls-Dateien gibt es Grenzen für das Kennwort selbst. Die einfache Verschleierung der ältesten Dateien verwendet nur die ersten 15 Zeichen eines Kennworts und kann nur Buchstaben, Ziffern und Zeichen des westeuropäischen Zeichensatzes (Latin-1) enthalten: Ein Kennwort mit anderen Buchstaben (etwa griechischen oder chinesischen) wird dort nie als richtig erkannt. Die 40-Bit-Art nimmt beliebige Buchstaben, aber nur die ersten 16 Zeichen. JeffCrunch speichert .xls-Dateien nicht mit Kennwort.

Manche Dateien sind mit einem Kennwort gesperrt, das alle kennen, damit sie von jedem geöffnet, aber nicht versehentlich geändert werden können. JeffCrunch öffnet diese Dateien, ohne nachzufragen.

## Wenn eine Datei nicht geöffnet werden kann

- **„Das Kennwort ist nicht korrekt.“** Das eingegebene Kennwort ist nicht das, mit dem die Datei geschützt wurde. Versuchen Sie es erneut und achten Sie auf Groß- und Kleinschreibung und Leerzeichen. JeffCrunch kann ein verlorenes Kennwort nicht wiederherstellen.
- **„Diese Datei ist auf eine Weise geschützt, die dieses Programm nicht öffnen kann.“** Die Datei ist durch etwas anderes als ein Kennwort gesperrt (zum Beispiel durch einen Schlüssel, der in einem Zertifikat liegt) oder in einer Form, die JeffCrunch nicht lesen kann.
- **„Der Kennwortschutz dieser Datei verlangt mehr Arbeitsspeicher oder Zeit, als dieses Programm zulässt, daher wird sie nicht geöffnet.“** Eine Datei von LibreOffice 25 oder neuer legt fest, wie viel Arbeitsspeicher und Zeit die Prüfung des Kennworts benötigt. JeffCrunch akzeptiert höchstens das Zehnfache dessen, was LibreOffice selbst vorgibt (640 MiB Arbeitsspeicher), damit eine Datei, die Ihren Computer lahmlegen soll, nicht geöffnet wird. Eine Datei, die LibreOffice selbst geschrieben hat, liegt deutlich darunter.
- **„Die Datei ist beschädigt oder wurde nach dem Schutz verändert.“** Das Kennwort war richtig, aber der Inhalt der Datei entspricht nicht dem, was gesperrt wurde: Sie wurde abgeschnitten, beschädigt oder von einem anderen Programm verändert. Kopieren Sie die Datei erneut aus dem Original.

## Mit Kennwort speichern

Klicken Sie auf **Datei** und dann auf **Mit Kennwort schützen** (oder **Überprüfen > Mit Kennwort schützen**), geben Sie das Kennwort zweimal ein und klicken Sie auf **Schützen**. Die Arbeitsmappe wird beim nächsten Speichern geschützt geschrieben: Wer die Datei öffnet, muss das Kennwort eingeben. Um das Kennwort zu ändern, wählen Sie denselben Befehl und geben ein neues ein. Um es zu entfernen, klicken Sie auf **Kennwort entfernen** und speichern.

Eine Arbeitsmappe, die Sie mit Kennwort geöffnet haben, behält es, solange sie geöffnet ist. **Speichern** schreibt die Datei daher wieder geschützt mit demselben Kennwort. JeffCrunch hält das Kennwort nur im Arbeitsspeicher, bis Sie die Arbeitsmappe schließen, und löscht es dann. Die Datei, die Sie mit Kennwort geöffnet haben, bleibt nicht aus Versehen ungeschützt: Eine Kopie, die als .ods- oder .ots-Datei gespeichert wird, ist mit demselben Kennwort geschützt, und zwar so, wie es der OpenDocument-Standard beschreibt. Andere Programme, die diesen Standard unterstützen, können sie daher mit dem Kennwort öffnen. Eine .xls-Kopie oder eine Textkopie lässt sich nicht schützen. JeffCrunch weist Sie deshalb vorher darauf hin, dass die Kopie nicht geschützt sein wird, und Sie können abbrechen.

Verwenden Sie ein langes Kennwort, und bewahren Sie es sicher auf: Ohne das Kennwort kann niemand die Datei öffnen, auch wir nicht.
