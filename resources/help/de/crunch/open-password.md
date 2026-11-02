# Kennwortgeschützte Arbeitsmappen öffnen
<!-- keywords: kennwort, passwort, geschützt, verschlüsselt, verschlüsselung, öffnen, gesperrt, falsches kennwort, abbrechen, xlsx, xls, ods, dateischutz, standardkennwort, privat -->

Manche Arbeitsmappen werden beim Speichern mit einem Kennwort geschützt: Ohne das Kennwort kann kein Programm lesen, was in der Datei steht. Das ist etwas anderes als **Blatt schützen** und **Arbeitsmappe schützen** (siehe [Blatt oder Arbeitsmappe schützen](protect)), die nur Änderungen verhindern. Eine Arbeitsmappe mit Kennwortschutz lässt sich überhaupt nicht öffnen, bevor das Kennwort eingegeben wurde.

## Eine Arbeitsmappe öffnen

Öffnen Sie die Datei wie jede andere: Drücken Sie **{key:Open}**, wählen Sie sie aus, oder ziehen Sie sie auf das Fenster. JeffCrunch erkennt, dass die Datei geschützt ist, und fragt nach dem Kennwort, bevor es etwas liest:

1. Geben Sie das Kennwort in das Feld ein, das mit **Diese Datei ist durch ein Kennwort geschützt.** beschriftet ist. Die Zeichen erscheinen als Punkte.
2. Klicken Sie auf **Öffnen**.

Ist das Kennwort nicht richtig, sagt JeffCrunch das und fragt erneut, beliebig oft. Mit **Abbrechen** hören Sie auf: Es wird nichts geöffnet, und die Arbeitsmappen, die Sie schon geöffnet haben, bleiben unverändert. Kennwörter müssen genau stimmen: Groß- und Kleinschreibung, Leerzeichen und Akzente zählen.

JeffCrunch verwendet das Kennwort nur, um die Datei zu öffnen. Es speichert das Kennwort nicht, schreibt es nirgendwohin und vergisst es, sobald die Datei gelesen ist. Beim nächsten Öffnen derselben Datei fragt es deshalb wieder.

## Was sich öffnen lässt

- **.xlsx- und .xlsm-Arbeitsmappen**, die auf eine der beiden Arten geschützt sind, die das Originalprogramm seit 2007 verwendet: die neuere (der Schlüssel hat 128, 192 oder 256 Bit, und JeffCrunch prüft außerdem, dass niemand die Datei nach dem Schützen verändert hat) und die ältere (128 Bit).
- **.xls-Arbeitsmappen**, die auf eine der drei Arten dieses Formats geschützt sind: die einfache Verschleierung der ältesten Dateien, die 40-Bit-Art und die stärkere Art, die den Schlüsseldienst des Computers verwendet.
- **.ods-Tabellen** mit Kennwortschutz, sowohl die aktuelle Art (256 Bit) als auch die ältere Art, die frühere Versionen von LibreOffice und OpenOffice geschrieben haben.

Manche Dateien sind mit einem Kennwort gesperrt, das jeder kennt, damit sie jeder öffnen, aber niemand versehentlich ändern kann. JeffCrunch öffnet diese, ohne zu fragen.

## Wenn sich eine Datei nicht öffnen lässt

- **„Das Kennwort ist nicht richtig.“** Das eingegebene Kennwort ist nicht das, mit dem die Datei geschützt wurde. Versuchen Sie es noch einmal und achten Sie auf Groß- und Kleinschreibung und Leerzeichen; JeffCrunch kann ein vergessenes Kennwort nicht wiederherstellen.
- **„Diese Datei ist auf eine Weise geschützt, die dieses Programm nicht öffnen kann.“** Die Datei ist durch etwas anderes als ein Kennwort gesperrt (zum Beispiel durch einen Schlüssel, der in einem Zertifikat liegt) oder in einer Form, die JeffCrunch nicht liest.
- **„Die Datei ist beschädigt oder wurde nach dem Schützen verändert.“** Das Kennwort war richtig, aber der Inhalt der Datei entspricht nicht dem, was gesperrt wurde: Sie wurde abgeschnitten, beschädigt oder von einem anderen Programm verändert. Kopieren Sie sie erneut vom Original.

## Speichern

Eine auf diese Weise geöffnete Arbeitsmappe ist für JeffCrunch eine neue Arbeitsmappe: Wenn Sie sie speichern, ist die Kopie **nicht** durch ein Kennwort geschützt, und die geöffnete Datei wird nie verändert. Um die Kopie wieder zu schützen, verwenden Sie das Programm, das das Original erstellt hat.
