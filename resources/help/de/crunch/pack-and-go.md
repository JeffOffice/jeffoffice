# Pack and Go: Arbeitsmappe mit allen benötigten Dateien senden
<!-- keywords: Pack and Go, Paket, packen, senden, teilen, anderer Computer, ZIP, Ordner, Schriftarten, verknüpfte Dateien, verknüpfte Bilder, fehlende Bilder, kopieren, mitnehmen, USB-Stick, Lizenz, externer Link, verknüpfte Arbeitsmappe, Zellen, Diagramme -->

Wenn Sie eine Arbeitsmappe an jemand anderen senden, können die Bilder und anderen Arbeitsmappen, mit denen sie verknüpft ist, auf dem Weg verloren gehen. **Pack and Go** speichert die Arbeitsmappe zusammen mit diesen Dateien und ihren Schriftarten in einer ZIP-Datei oder einem Ordner, sodass sie auf einem anderen Computer genauso aussieht.

## So verwenden Sie die Funktion

1. Klicken Sie auf **Datei** und dann auf **Exportieren**.
2. Klicken Sie auf **Pack and Go: Für einen anderen Computer speichern**, um eine ZIP-Datei zu erstellen, oder auf **Pack and Go: In einem Ordner speichern**, um einen neuen Ordner anzulegen.
3. Wählen Sie aus, wo das Paket gespeichert werden soll. Bei einer ZIP-Datei geben Sie einen Namen ein. Bei einem Ordner wählen Sie den Ordner aus, in dem der neue Ordner angelegt werden soll.
4. Lesen Sie die Meldung, die angezeigt wird. Sie nennt den Speicherort des Pakets, die Anzahl der verknüpften Dateien und Schriftartdateien, die aufgenommen wurden, und alles, was nicht gepackt werden konnte.

Eine **ZIP-Datei** ist eine einzelne Datei, die andere Dateien enthält, und lässt sich dadurch leicht versenden. Die andere Person öffnet sie, um die Dateien herauszuholen. Ein Ordner eignet sich besser, wenn Sie alles auf ein Laufwerk kopieren möchten.

## Was in das Paket kommt

- **Die Arbeitsmappe selbst** mit dem Namen, den sie jetzt hat. Es ist eine Kopie. Ihre eigene Datei wird nicht verändert.
- **Verknüpfte Bilder.** Ein Bild in der Arbeitsmappe kann mit seiner Datei verknüpft sein. Das Paket enthält eine Kopie jeder dieser Dateien. Bilder, die direkt in die Arbeitsmappe eingefügt wurden, sind bereits darin enthalten.
- **Verknüpfte Arbeitsmappen.** Eine Formel kann Zahlen aus einer anderen Arbeitsmappe beziehen. Das Paket enthält auch eine Kopie dieser Arbeitsmappe, und die Verknüpfung verweist nun auf die Kopie. Es werden die Schriftarten verwendet, die in Zellen, Diagrammen und Formen eingesetzt werden.
- **Schriftarten.** Eine Schriftart ist das Erscheinungsbild der Buchstaben. Das Paket enthält die Schriftartdateien, die mit dem Programm mitgeliefert werden und die die Arbeitsmappe verwendet, in einem Ordner namens **fonts**. Pack and Go packt nie eine Schriftart, deren Lizenz die Weitergabe untersagt. Die Meldung nennt jede Schriftart, die ausgelassen wurde. Von Ihnen auf Ihrem Computer installierte Schriftarten werden nicht gepackt, sodass die andere Person diese möglicherweise benötigt.

## Wie sich die Verknüpfungen ändern

In der Kopie verweist jede Verknüpfung, die gepackt wurde, auf die Datei im Paket, und zwar über einen kurzen Namen, etwa **linked/photo.png**. Die Verknüpfungen verwenden nicht mehr den Pfad auf Ihrem Computer und funktionieren daher überall, wo das Paket geöffnet wird. Haben zwei Dateien denselben Namen, wird der zweiten eine Zahl angehängt.

Eine Verknüpfung, der Pack and Go nicht folgen kann, bleibt unverändert, und die Meldung listet sie auf. Pack and Go packt nicht:

- Eine Datei, die nicht gefunden werden kann.
- Eine Webadresse. Diese bleiben unverändert.
- Eine Datei auf einem anderen Computer in Ihrem Netzwerk.
- Eine Verknüpfung, die aus dem Ordner der Arbeitsmappe nach oben und hinaus führt, etwa **../../notes.png**.
- Ein Dokument, eine Arbeitsmappe oder ein anderes Objekt, das nicht im Ordner der Arbeitsmappe (oder in einem Unterordner davon) liegt, auch wenn die Verknüpfung den vollständigen Pfad angibt. Nur Bilder, Filme und Töne werden von überall her gepackt.
- Eine Verknüpfung, die über eine Verknüpfung auf einen anderen Ordner (eine symbolische Verknüpfung) aus dem Ordner der Arbeitsmappe herausführt.
- Eine Verknüpfung auf die Arbeitsmappe selbst.
- Eine Datei, die kein Bild, Film, Ton oder Dokument ist. Ein Programm wird nie gepackt.

Pack and Go sucht in den verknüpften Dateien nicht nach weiteren Verknüpfungen. Daher kann eine Datei, die auf diese Arbeitsmappe zurückverweist, keine Endlosschleife auslösen.

## Grenzwerte

Damit ein Paket nicht zu groß wird, fragt Pack and Go zuerst nach, wenn mehr als 200 verknüpfte Dateien oder mehr als 256 MB davon vorhanden sind. Wenn Sie **Nein** wählen, wird nichts gespeichert. Pack and Go packt unabhängig von Ihrer Antwort niemals mehr als 2.000 Dateien oder mehr als 1 GB. Die darüber hinausgehenden Dateien werden ausgelassen und in der Meldung aufgeführt.

Wenn Sie die Arbeitsmappe mit einem Kennwort geschützt haben, hat die Kopie im Paket dasselbe Kennwort.
