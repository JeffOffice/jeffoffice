# Pack and Go: Dokument mit allen benötigten Dateien senden
<!-- keywords: Pack and Go, Paket, packen, senden, teilen, anderer Computer, ZIP, ZIP-Datei, Ordner, Schriftarten, verknüpfte Dateien, verknüpfte Bilder, fehlende Bilder, kopieren, mitnehmen, USB-Stick, Lizenz, INCLUDEPICTURE, INCLUDETEXT, Feld -->

Wenn Sie ein Dokument an jemand anderen senden, können die Bilder und Schriftarten, die es verwendet, unterwegs verloren gehen. **Pack and Go** speichert das Dokument zusammen mit allen benötigten Dateien in einer ZIP-Datei oder einem Ordner, damit es auf einem anderen Computer genauso aussieht.

## So verwenden Sie die Funktion

1. Klicken Sie auf **Datei** und dann auf **Exportieren**.
2. Klicken Sie auf **Pack and Go: Für einen anderen Computer speichern**, um eine ZIP-Datei zu erstellen, oder auf **Pack and Go: In einem Ordner speichern**, um einen neuen Ordner zu erstellen.
3. Wählen Sie den Speicherort. Für eine ZIP-Datei geben Sie einen Namen ein. Für einen Ordner wählen Sie den Ordner aus, in dem der neue Ordner angelegt werden soll.
4. Lesen Sie die Meldung, die anschließend erscheint. Sie nennt den Speicherort des Pakets, die Anzahl der eingepackten verknüpften Dateien und Schriftartdateien sowie alles, was nicht gepackt werden konnte.

Eine **ZIP-Datei** ist eine einzelne Datei, die andere Dateien enthält. Dadurch lässt sie sich leicht versenden. Die andere Person öffnet sie, um die Dateien zu entpacken. Ein Ordner eignet sich besser, wenn Sie alles auf ein Laufwerk kopieren möchten.

## Was ins Paket kommt

- **Das Dokument selbst** mit dem Namen, den es jetzt hat. Es handelt sich um eine Kopie. Ihre eigene Datei bleibt unverändert.
- **Verknüpfte Bilder.** Ein verknüpftes Bild zeigt ein Bild aus einer Datei an, anstatt das Bild selbst zu enthalten. Das Paket enthält eine Kopie jedes dieser Bilder.
- **Dateien, die durch Felder angegeben werden.** Ein Feld ist ein Code, der sein Ergebnis selbst einfügt. Ein **INCLUDEPICTURE**-Feld zeigt ein Bild aus einer Datei an, ein **INCLUDETEXT**-Feld zeigt Text aus einem anderen Dokument an. Das Paket enthält eine Kopie jeder dieser Dateien, und das Feld verweist nun auf diese Kopie.
- **Schriftarten.** Eine Schriftart bestimmt das Erscheinungsbild der Buchstaben. Das Paket enthält die Schriftdateien, die mit dem Programm mitgeliefert werden und die das Dokument verwendet, und zwar in einem Ordner namens **fonts**. Pack and Go packt niemals eine Schriftart, deren Lizenz die Weitergabe untersagt. Die Meldung nennt jede Schriftart, die nicht übernommen wurde. Schriftarten, die Sie selbst auf Ihrem Computer installiert haben, werden nicht gepackt. Die andere Person benötigt diese unter Umständen selbst.

## Wie sich die Verknüpfungen ändern

In der Kopie zeigt jede Verknüpfung, die gepackt wurde, auf die Datei innerhalb des Pakets, und zwar über einen kurzen Namen, zum Beispiel **linked/photo.png**. Die Verknüpfungen verwenden nicht mehr den Pfad auf Ihrem Computer, sodass sie überall funktionieren, wo das Paket geöffnet wird. Haben zwei Dateien denselben Namen, wird der zweiten eine Nummer angehängt.

Eine Verknüpfung, der Pack and Go nicht folgen kann, bleibt unverändert, und die Meldung führt sie auf. Pack and Go packt nicht:

- Eine Datei, die nicht gefunden werden kann.
- Eine Webadresse. Diese bleiben unverändert.
- Eine Datei auf einem anderen Computer in Ihrem Netzwerk.
- Eine Verknüpfung, die aus dem Ordner des Dokuments hinausführt, etwa **../../notes.png**.
- Ein Dokument, eine Arbeitsmappe oder ein anderes Objekt, das sich nicht im Ordner des Dokuments befindet (oder in einem Ordner darin), auch wenn die Verknüpfung den vollständigen Pfad angibt. Nur Bilder, Filme und Töne werden von überall gepackt.
- Eine Verknüpfung, die über eine Verknüpfung zu einem anderen Ordner (eine symbolische Verknüpfung) aus dem Ordner des Dokuments herausführt.
- Eine Verknüpfung auf das Dokument selbst.
- Eine Datei, die kein Bild, kein Film, kein Ton und kein Dokument ist. Ein Programm wird niemals gepackt.

Pack and Go durchsucht die verknüpften Dateien nicht nach weiteren Verknüpfungen. Deshalb kann eine Datei, die auf dieses Dokument zurückverweist, keine Endlosschleife verursachen.

## Grenzwerte

Damit ein Paket nicht zu groß wird, fragt Pack and Go vorher nach, wenn mehr als 200 verknüpfte Dateien oder mehr als 256 MB davon vorhanden sind. Wählen Sie **Nein**, wird nichts gespeichert. Pack and Go packt niemals mehr als 2.000 Dateien oder mehr als 1 GB, unabhängig von Ihrer Antwort. Dateien darüber hinaus werden nicht übernommen und in der Meldung aufgeführt.

Wenn Sie für das Dokument ein Kennwort verwenden, hat die Kopie im Paket dasselbe Kennwort.
