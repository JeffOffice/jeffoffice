# Text übersetzen

<!-- keywords: übersetzen, Übersetzung, Übersetzer, Sprache, Sprachen, Spanisch, Französisch, Deutsch, Italienisch, Portugiesisch, Niederländisch, Russisch, Ukrainisch, Indonesisch, offline, auf diesem Computer, herunterladen, Modelle, ersetzen, darunter einfügen, Änderungen nachverfolgen, Übersetzungsmodelle, Sprachmodell -->

**Überprüfen > Sprache > Übersetzen** öffnet den Bereich **Übersetzen** auf der rechten Seite. Der Text wird auf Ihrem eigenen Computer übersetzt. Ihre Wörter werden nirgendwohin gesendet, und es ist keine Internetverbindung nötig, sobald die Übersetzungsdateien für eine Sprache auf Ihrem Computer liegen.

## Text übersetzen

1. Markieren Sie den Text, den Sie übersetzen möchten. Ist nichts markiert, verwendet der Bereich den Absatz, in dem sich die Einfügemarke befindet.
2. Klicken Sie in der Gruppe **Sprache** auf **Übersetzen**. Der Bereich zeigt Ihren Text unter **Original** an.
3. Wählen Sie unter **Von** die Sprache, in der Ihr Text verfasst ist. JeffWrite beginnt mit der Sprache, die dem Text zugewiesen ist. Wählen Sie unter **Nach** die gewünschte Sprache. Mit der Schaltfläche **Tauschen** werden die beiden Sprachen vertauscht.
4. Klicken Sie auf **Übersetzen**. Die Übersetzung erscheint unter **Übersetzung**. Fett, kursiv und unterstrichen formatierte Wörter bleiben fett, kursiv und unterstrichen, und eine Farbe oder eine Schriftgröße bleibt erhalten, wo die Wörter vorher das gleiche Aussehen hatten.
5. Klicken Sie auf **Ersetzen**, um die Übersetzung an die Stelle Ihres Textes zu setzen, oder auf **Darunter einfügen**, um sie als neue Absätze unter Ihren Text einzufügen. Beide Befehle sind ein einzelner Schritt, sodass Sie sie mit **Rückgängig** zurücknehmen können. Die übersetzten Wörter werden mit ihrer Sprache gekennzeichnet, sodass die Rechtschreibprüfung sie in dieser Sprache prüft.

Wenn Sie nach dem Klick auf **Übersetzen** die Einfügemarke verschieben oder den Text ändern, klicken Sie auf **Auswahl übernehmen**, um den Text erneut einzulesen, und dann noch einmal auf **Übersetzen**. Der Bereich setzt keine Übersetzung über einen Text, der nicht mehr der übersetzte Text ist.

Ist **Änderungen nachverfolgen** aktiviert, sind **Ersetzen** und **Darunter einfügen** nachverfolgte Änderungen wie Ihre Eingaben, sodass ein Prüfer sieht, was vorher dort stand. Die übersetzten Wörter erhalten dann das Aussehen, das Ihre Eingaben hätten. Wörter, die eine nachverfolgte Änderung gelöscht hat, bleiben unberührt: Sie werden weder übersetzt noch von **Darunter einfügen** kopiert.

Mit **Abbrechen** beenden Sie eine Übersetzung, die zu lange dauert, und das Schließen des Bereichs bewirkt dasselbe: Das Fenster wartet nie darauf. Lange Texte brauchen mehr Zeit: Eine Seite Text dauert auf den meisten Computern einige Sekunden. Der Bereich übersetzt höchstens 1.000.000 Buchstaben auf einmal (etwa 200 Seiten). Sind mehr markiert, weist er darauf hin und bittet Sie, weniger auszuwählen. Lange Absätze teilt er in Stücke von etwa 1.500 Buchstaben, möglichst am Satzende, und setzt sie danach wieder zusammen.

## Beim ersten Gebrauch einer Sprache

Für eine Übersetzung wird je Richtung ein Satz Dateien benötigt, ein sogenanntes Modell: eines für Englisch nach Deutsch und ein weiteres für Deutsch nach Englisch. Jedes Modell ist etwa 37 MB groß. Die Dateien gehören nicht zu JeffWrite. Wenn Sie zum ersten Mal mit einer Sprache übersetzen, teilt Ihnen der Bereich mit, was benötigt wird und wie groß es ist, zum Beispiel „Deutsch (74 MB) herunterladen?“. Es wird erst etwas heruntergeladen, wenn Sie auf diese Schaltfläche klicken. Der Bereich zeigt an, wie weit der Download fortgeschritten ist, und mit **Abbrechen** wird er gestoppt, ohne Reste zu hinterlassen.

JeffWrite prüft jede Datei anhand einer Prüfsumme (SHA-256-Fingerabdruck), bevor sie verwendet wird. Eine Datei, die nicht übereinstimmt, wird verworfen, und die Sprache wird nicht installiert.

Die Dateien liegen in einem Ordner, den alle JeffOffice-Apps gemeinsam nutzen (ein Ordner namens JeffOffice im Benutzerdatenordner, unter Translation/models). Eine Sprache, die Sie in einer App herunterladen, steht daher auch in den anderen bereit, und sie wird nur einmal geladen. Zwei Sprachen ohne eigenes Modell, etwa Deutsch und Französisch, werden über Englisch übersetzt, daher benötigt der Bereich beide Modelle.

## Wenn für eine Sprache kein Modell vorhanden ist

Gibt es für ein Sprachpaar kein Modell, teilt der Bereich das mit und bietet **Stattdessen im Browser öffnen** an. Damit wird eine kostenlose Übersetzungswebsite, LibreTranslate (oder der Dienst, dessen Adresse Sie in den Einstellungen dafür gewählt haben), mit Ihrem Text geöffnet. Erst dann verlässt Ihr Text den Computer, denn er wird an diese Website gesendet, und der Bereich sagt nicht mehr, dass er auf Ihrem Computer bleibt. Es wird erst etwas gesendet, wenn Sie auf diese Schaltfläche klicken.

## Zur Qualität

Die Modelle sind klein und laufen daher auf einem gewöhnlichen Computer schnell. Sie übersetzen Sätze gut, kurze Beschriftungen dagegen weniger gut: Ein einzelnes Wort oder der Name einer Schaltfläche kann falsch übersetzt werden. Lesen Sie die Übersetzung, bevor Sie sie verwenden. Namen, Zahlen und Tabulatoren bleiben unverändert.

Bilder, Tabstopps und Zeilenumbrüche innerhalb eines Absatzes bleiben unberührt. Die Übersetzung füllt nur die Wörter dazwischen aus.

## Siehe auch

- [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools)
