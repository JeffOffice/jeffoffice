# Text übersetzen
<!-- keywords: übersetzen, Übersetzung, Übersetzer, Sprache, Sprachen, Spanisch, Französisch, Deutsch, Italienisch, Portugiesisch, Niederländisch, Russisch, Ukrainisch, Indonesisch, offline, auf diesem Computer, herunterladen, Modelle, ersetzen, darunter einfügen -->

**Überprüfen > Sprache > Übersetzen** öffnet den Bereich **Übersetzen**. Er übersetzt Ihren Text auf Ihrem eigenen Computer. Ihre Wörter werden nirgendwohin gesendet, und es ist keine Internetverbindung nötig, sobald die Übersetzungsdateien für eine Sprache auf Ihrem Computer liegen.

## Text übersetzen

1. Markieren Sie den Text, den Sie übersetzen möchten. Ist nichts markiert, verwendet der Bereich den Absatz, in dem sich die Einfügemarke befindet. In einem Textfeld, das markiert, aber nicht bearbeitet wird, verwendet er den gesamten Text des Felds.
2. Klicken Sie auf **Überprüfen > Sprache > Übersetzen**. Der Bereich zeigt Ihren Text unter **Original** an.
3. Wählen Sie unter **Von** die Sprache aus, in der Ihr Text geschrieben ist. Unter **Nach** wählen Sie die gewünschte Sprache. Mit der Schaltfläche **Tauschen** werden beide Sprachen vertauscht.
4. Klicken Sie auf **Übersetzen**. Die Übersetzung erscheint unter **Übersetzung**. Fett, kursiv und unterstrichen formatierte Wörter bleiben fett, kursiv und unterstrichen.
5. Klicken Sie auf **Ersetzen**, um die Übersetzung an die Stelle Ihres Textes zu setzen, oder auf **Darunter einfügen**, um sie als neue Absätze unter Ihren Text einzufügen. Beide Vorgänge sind ein einziger Schritt, sodass Sie sie mit **Rückgängig** zurücknehmen können.

Wenn Sie die Einfügemarke bewegen oder den Text ändern, nachdem Sie auf **Übersetzen** geklickt haben, klicken Sie auf **Auswahl übernehmen**, um den Text erneut einzulesen, und dann noch einmal auf **Übersetzen**. Der Bereich setzt keine Übersetzung über einen Text, der nicht mehr der Text ist, den er übersetzt hat.

Mit **Abbrechen** beenden Sie eine Übersetzung, die zu lange dauert; das Gleiche bewirkt das Schließen des Bereichs. Das Fenster wartet nie darauf. Lange Texte brauchen länger: Eine Seite Text benötigt auf den meisten Computern einige Sekunden. Der Bereich übersetzt höchstens 1.000.000 Buchstaben auf einmal (etwa 200 Seiten). Sind mehr markiert, weist er darauf hin und bittet Sie, weniger auszuwählen. Lange Absätze teilt er in Stücke von etwa 1.500 Buchstaben, möglichst am Ende eines Satzes, und setzt sie danach wieder zusammen.

## Die erste Verwendung einer Sprache

Für eine Übersetzung braucht man je Richtung einen Satz Dateien, ein sogenanntes Modell: eines für Englisch nach Deutsch und ein weiteres für Deutsch nach Englisch. Jedes Modell ist etwa 37 MB groß. Die Dateien sind nicht Bestandteil von JeffPub. Wenn Sie zum ersten Mal mit einer Sprache übersetzen, zeigt der Bereich an, was benötigt wird und wie groß es ist, zum Beispiel „Deutsch herunterladen (74 MB)?“. Es wird erst etwas heruntergeladen, wenn Sie diese Schaltfläche anklicken. Der Bereich zeigt den Fortschritt des Downloads an, und **Abbrechen** beendet ihn und hinterlässt keine Reste.

JeffPub prüft jede Datei anhand einer Prüfsumme (SHA-256-Fingerabdruck), bevor sie verwendet wird. Eine Datei, die nicht übereinstimmt, wird verworfen, und die Sprache wird nicht installiert.

Die Dateien liegen in einem Ordner, den alle JeffOffice-Apps gemeinsam nutzen (ein Ordner namens JeffOffice in Ihrem Benutzerdatenordner, im Unterordner Translation/models). Eine Sprache, die Sie in einer App herunterladen, steht daher auch in den anderen zur Verfügung und wird nur einmal heruntergeladen. Zwei Sprachen, für die es kein eigenes Modell gibt, etwa Deutsch und Französisch, werden über Englisch übersetzt, daher benötigt der Bereich beide Modelle.

## Wenn für eine Sprache kein Modell vorhanden ist

Gibt es für ein Sprachpaar kein Modell, teilt der Bereich dies mit und bietet die Schaltfläche **Stattdessen im Browser öffnen** an. Damit öffnet sich eine kostenlose Übersetzungswebsite, LibreTranslate (oder der Dienst, dessen Adresse Sie in den Einstellungen dafür gewählt haben), mit Ihrem Text. Erst dann verlässt Ihr Text Ihren Computer, denn er wird an diese Website übermittelt, und der Bereich behauptet nicht mehr, dass er auf Ihrem Computer bleibt. Es wird erst etwas gesendet, wenn Sie diese Schaltfläche anklicken.

## Zur Qualität

Die Modelle sind klein und laufen daher auf einem normalen Computer schnell. Sie übersetzen Sätze gut und kurze Beschriftungen weniger gut: Ein einzelnes Wort oder der Name einer Schaltfläche kann falsch übersetzt werden. Prüfen Sie die Übersetzung, bevor Sie sie verwenden. Namen, Zahlen und Tabstopps bleiben unverändert.

Bilder, Tabstopps und Zeilenumbrüche innerhalb eines Absatzes bleiben unangetastet. Die Übersetzung füllt nur die Wörter dazwischen aus.

## Siehe auch

- [Rechtschreibung, Thesaurus, Recherche und Silbentrennung](proofing)
