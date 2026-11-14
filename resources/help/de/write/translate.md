# Text übersetzen
<!-- keywords: übersetzen, Übersetzung, Übersetzer, Sprache, Sprachen, Spanisch, Französisch, Deutsch, Italienisch, Portugiesisch, Niederländisch, Russisch, Ukrainisch, Indonesisch, offline, auf diesem Computer, herunterladen, Modelle, ersetzen, darunter einfügen, Änderungen nachverfolgen -->

**Überprüfen > Sprache > Übersetzen** öffnet den Aufgabenbereich **Übersetzen** auf der rechten Seite. Er übersetzt Ihren Text auf Ihrem eigenen Computer. Ihre Wörter werden nirgendwohin gesendet, und es ist keine Internetverbindung nötig, sobald die Übersetzungsdateien für eine Sprache auf Ihrem Computer liegen.

## Text übersetzen

1. Markieren Sie den Text, den Sie übersetzen möchten. Ist nichts markiert, verwendet der Aufgabenbereich den Absatz, in dem sich der Cursor befindet.
2. Klicken Sie in der Gruppe **Sprache** auf **Übersetzen**. Der Aufgabenbereich zeigt Ihren Text unter **Original** an.
3. Wählen Sie unter **Von** die Sprache, in der Ihr Text geschrieben ist. JeffWrite beginnt mit der Sprache, als die der Text markiert ist. Wählen Sie unter **Nach** die gewünschte Sprache. Mit der Schaltfläche **Tauschen** werden beide vertauscht.
4. Klicken Sie auf **Übersetzen**. Die Übersetzung erscheint unter **Übersetzung**. Fett, kursiv und unterstrichen formatierte Wörter bleiben fett, kursiv und unterstrichen, und eine Farbe oder eine Schriftgröße bleibt erhalten, wenn die Wörter vorher das gleiche Aussehen hatten.
5. Klicken Sie auf **Ersetzen**, um die Übersetzung an die Stelle Ihres Textes zu setzen, oder auf **Darunter einfügen**, um sie in neuen Absätzen unter Ihrem Text einzufügen. Beide Schritte lassen sich mit **Rückgängig** in einem Schritt zurücknehmen. Die übersetzten Wörter werden mit ihrer Sprache gekennzeichnet, sodass die Rechtschreibprüfung sie in dieser Sprache prüft.

Wenn Sie den Cursor bewegen oder den Text ändern, nachdem Sie auf **Übersetzen** geklickt haben, klicken Sie auf **Auswahl verwenden**, um den Text erneut einzulesen, und dann noch einmal auf **Übersetzen**. Der Aufgabenbereich setzt keine Übersetzung über einen Text, der nicht mehr der übersetzte Text ist.

Wenn **Änderungen nachverfolgen** aktiviert ist, werden **Ersetzen** und **Darunter einfügen** wie Ihre Eingaben als nachverfolgte Änderungen erfasst, damit ein Prüfer sieht, was vorher dort stand. Die übersetzten Wörter erhalten dann das Aussehen, das Ihre Eingaben hätten. Wörter, die eine nachverfolgte Änderung gelöscht hat, bleiben unberührt: Sie werden weder übersetzt noch von **Darunter einfügen** kopiert.

**Abbrechen** stoppt eine Übersetzung, die zu lange dauert, und das Schließen des Aufgabenbereichs stoppt sie ebenfalls: Das Fenster wartet nie darauf. Lange Texte brauchen mehr Zeit: Eine Seite Text braucht auf den meisten Computern einige Sekunden. Der Aufgabenbereich übersetzt auf einmal höchstens 1.000.000 Buchstaben (etwa 200 Seiten); wenn mehr markiert ist, teilt er das mit und bittet Sie, weniger auszuwählen. Lange Absätze werden in Stücke von etwa 1.500 Buchstaben zerlegt, möglichst am Ende eines Satzes, und danach wieder zusammengesetzt.

## Beim ersten Gebrauch einer Sprache

Für jede Übersetzungsrichtung wird ein Satz von Dateien benötigt, ein sogenanntes Modell: eines für Englisch nach Deutsch und ein weiteres für Deutsch nach Englisch. Jedes Modell ist etwa 37 MB groß. Die Dateien gehören nicht zu JeffWrite. Wenn Sie zum ersten Mal mit einer Sprache übersetzen, teilt der Aufgabenbereich mit, was benötigt wird und wie groß es ist, zum Beispiel „Deutsch (74 MB) herunterladen?“. Es wird erst etwas heruntergeladen, wenn Sie auf diese Schaltfläche klicken. Der Aufgabenbereich zeigt den Fortschritt des Downloads an, und **Abbrechen** stoppt ihn und hinterlässt nichts.

JeffWrite prüft jede Datei anhand einer Prüfsumme (SHA-256-Prüfsumme), bevor sie verwendet wird. Eine Datei, die nicht übereinstimmt, wird verworfen, und die Sprache wird nicht installiert.

Die Dateien liegen in einem Ordner, den alle JeffOffice-Apps gemeinsam nutzen (ein Ordner namens JeffOffice in Ihrem Benutzerdatenordner, unter Translation/models). Eine Sprache, die Sie in einer App herunterladen, steht daher in den anderen Apps bereit und wird nur einmal heruntergeladen. Zwei Sprachen ohne eigenes Modell, etwa Deutsch und Französisch, werden über Englisch übersetzt, daher benötigt der Aufgabenbereich beide Modelle.

## Wenn eine Sprache kein Modell hat

Wenn für ein Sprachpaar kein Modell vorhanden ist, teilt der Aufgabenbereich das mit und bietet **Stattdessen im Browser öffnen** an. Damit wird eine kostenlose Übersetzungswebsite geöffnet, LibreTranslate (oder der Dienst, dessen Adresse Sie in der Einstellung dafür gewählt haben), mit Ihrem Text. Erst dann verlässt Ihr Text den Computer, denn er wird an diese Website gesendet, und der Aufgabenbereich sagt nicht mehr, dass er auf Ihrem Computer bleibt. Es wird erst etwas gesendet, wenn Sie auf diese Schaltfläche klicken.

## Zur Qualität

Die Modelle sind klein und laufen daher auf einem normalen Computer schnell. Sie übersetzen Sätze gut und kurze Beschriftungen weniger gut: Ein einzelnes Wort oder der Name einer Schaltfläche kann falsch übersetzt werden. Lesen Sie die Übersetzung, bevor Sie sie verwenden. Namen, Zahlen und Tabstopps bleiben unverändert.

Bilder, Tabstopps und Zeilenumbrüche innerhalb eines Absatzes bleiben unangetastet. Die Übersetzung füllt nur die Wörter zwischen ihnen aus.

## Siehe auch

- [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools)
