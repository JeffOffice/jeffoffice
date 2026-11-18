# Pack and Go: Präsentation mitnehmen
<!-- keywords: pack and go, packen, paket, senden, teilen, anderer computer, zip, ordner, schriftarten, verknüpfte dateien, verknüpfte bilder, fehlende bilder, kopieren, mitnehmen, USB-Stick, lizenz, film, video, ton, audio, präsentation, weitergeben -->

Eine Präsentation kann einen Film oder Ton aus einer Datei abspielen, die nicht in der Präsentation enthalten ist. Auf einem anderen Computer fehlt diese Datei, und die Bildschirmpräsentation hat eine Lücke. Mit **Pack and Go** speichern Sie die Präsentation zusammen mit ihren verknüpften Dateien und ihren Schriftarten in einer ZIP-Datei oder in einem Ordner.

## So verwenden Sie die Funktion

1. Klicken Sie auf **Datei** und dann auf **Exportieren**.
2. Klicken Sie auf **Pack and Go: Für einen anderen Computer speichern**, um eine ZIP-Datei zu erstellen, oder auf **Pack and Go: In einem Ordner speichern**, um einen neuen Ordner zu erstellen.
3. Wählen Sie den Speicherort. Für eine ZIP-Datei geben Sie einen Namen ein. Für einen Ordner wählen Sie den Ordner aus, in dem der neue Ordner angelegt werden soll.
4. Lesen Sie die Meldung, die danach erscheint. Sie zeigt, wo sich das Paket befindet, wie viele verknüpfte Dateien und Schriftartdateien hinzugefügt wurden und welche Dateien nicht gepackt werden konnten.

Eine **ZIP-Datei** ist eine einzelne Datei, die andere Dateien enthält, und lässt sich deshalb leicht versenden. Die andere Person öffnet sie, um die Dateien herauszuholen. Ein Ordner eignet sich besser, wenn Sie alles auf einen Datenträger kopieren möchten.

## Was ins Paket kommt

- **Die Präsentation selbst** mit dem Namen, den sie jetzt hat. Es handelt sich um eine Kopie. Ihre eigene Datei wird nicht verändert.
- **Verknüpfte Filme und Töne.** Ein Film oder Ton kann verknüpft sein, sodass die Präsentation ihn aus seiner Datei abspielt, statt ihn zu enthalten. Das Paket enthält eine Kopie jeder dieser Dateien, damit die Bildschirmpräsentation auf dem anderen Computer genauso abläuft. Wenn Sie die Datei nicht in die Präsentation eingefügt haben, nehmen Sie sie auf diese Weise mit.
- **Verknüpfte Bilder.** Das Paket enthält eine Kopie jedes Bildes, das mit seiner Datei verknüpft ist.
- **Schriftarten.** Eine Schriftart bestimmt das Aussehen der Buchstaben. Das Paket enthält die Schriftartdateien, die zum Programm gehören und die die Präsentation verwendet, in einem Ordner namens **fonts**. Pack and Go packt niemals eine Schriftart, deren Lizenz die Weitergabe untersagt. Die Meldung nennt jede Schriftart, die ausgelassen wurde. Von Ihnen auf dem Computer installierte Schriftarten werden nicht gepackt. Die andere Person benötigt diese unter Umständen selbst.

## Wie sich die Verknüpfungen ändern

In der Kopie zeigt jede Verknüpfung, die gepackt wurde, auf die Datei innerhalb des Pakets, und zwar über einen kurzen Namen, zum Beispiel **linked/photo.png**. Die Verknüpfungen verwenden nicht mehr den Pfad auf Ihrem Computer und funktionieren daher überall, wo das Paket geöffnet wird. Haben zwei Dateien denselben Namen, wird der zweiten eine Nummer angehängt.

Eine Verknüpfung, der Pack and Go nicht folgen kann, bleibt unverändert, und die Meldung führt sie auf. Pack and Go packt nicht:

- eine Datei, die nicht gefunden werden kann.
- eine Webadresse. Diese bleiben unverändert.
- eine Datei auf einem anderen Computer in Ihrem Netzwerk.
- eine Verknüpfung, die aus dem Ordner der Präsentation heraus nach oben führt, etwa **../../notizen.png**.
- ein Dokument, eine Arbeitsmappe oder ein anderes Objekt, das nicht im eigenen Ordner der Präsentation liegt (oder in einem Ordner darin), auch wenn die Verknüpfung den vollständigen Pfad angibt. Nur Bilder, Filme und Töne werden von überall her gepackt.
- eine Verknüpfung, die über eine Verknüpfung auf einen anderen Ordner (eine symbolische Verknüpfung) aus dem Ordner der Präsentation herausführt.
- eine Verknüpfung auf die Präsentation selbst.
- eine Datei, die kein Bild, Film, Ton oder Dokument ist. Ein Programm wird niemals gepackt.

Pack and Go durchsucht die verknüpften Dateien nicht nach weiteren Verknüpfungen. Eine Datei, die auf diese Präsentation zurückverweist, kann daher keine Endlosschleife auslösen.

## Grenzwerte

Damit ein Paket nicht zu groß wird, fragt Pack and Go zuerst nach, wenn mehr als 200 verknüpfte Dateien oder mehr als 256 MB davon vorhanden sind. Wählen Sie **Nein**, wird nichts gespeichert. Pack and Go packt niemals mehr als 2.000 Dateien oder mehr als 1 GB, unabhängig von Ihrer Antwort. Die darüber hinausgehenden Dateien werden ausgelassen und in der Meldung aufgeführt.

Wenn Sie für die Präsentation ein Kennwort verwenden, hat die Kopie im Paket dasselbe Kennwort.
