# Text übersetzen
<!-- keywords: übersetzen, übersetzung, übersetzer, sprache, sprachen, spanisch, französisch, deutsch, italienisch, portugiesisch, niederländisch, russisch, ukrainisch, indonesisch, offline, auf diesem computer, herunterladen, modelle, ersetzen, darunter einfügen, translate, translation -->

**Überprüfen > Sprache > Übersetzen** öffnet den Aufgabenbereich **Übersetzen**. Er übersetzt Ihren Text auf Ihrem eigenen Computer. Ihre Wörter werden nirgendwohin gesendet, und es ist keine Internetverbindung nötig, sobald die Übersetzungsdateien für eine Sprache auf Ihrem Computer liegen.

## Einen Text übersetzen

1. Markieren Sie den Text, den Sie übersetzen möchten. Ist nichts markiert, verwendet der Aufgabenbereich den Absatz, in dem sich die Einfügemarke befindet. In einem Textfeld, das markiert, aber nicht bearbeitet wird, verwendet er den gesamten Text des Felds.
2. Klicken Sie auf **Überprüfen > Sprache > Übersetzen**. Der Aufgabenbereich zeigt Ihren Text unter **Original** an.
3. Wählen Sie unter **Von** die Sprache aus, in der Ihr Text geschrieben ist. Wählen Sie unter **Nach** die gewünschte Sprache aus. Mit der Schaltfläche **Tauschen** werden beide Sprachen vertauscht.
4. Klicken Sie auf **Übersetzen**. Die Übersetzung erscheint unter **Übersetzung**. Fett, kursiv und unterstrichen formatierte Wörter bleiben fett, kursiv bzw. unterstrichen.
5. Klicken Sie auf **Ersetzen**, um die Übersetzung an die Stelle Ihres Textes zu setzen, oder auf **Darunter einfügen**, um sie als neue Absätze unter Ihrem Text einzufügen. Beides ist ein einzelner Schritt, sodass Sie es mit **Rückgängig** zurücknehmen können.

Wenn Sie die Einfügemarke verschieben oder den Text ändern, nachdem Sie auf **Übersetzen** geklickt haben, klicken Sie auf **Auswahl verwenden**, um den Text erneut einzulesen, und dann noch einmal auf **Übersetzen**. Der Aufgabenbereich setzt keine Übersetzung über einen Text, der nicht mehr der übersetzte Text ist.

Mit **Abbrechen** beenden Sie eine Übersetzung, die zu lange dauert; dasselbe geschieht, wenn Sie den Aufgabenbereich schließen. Das Fenster wartet nie darauf. Lange Texte brauchen mehr Zeit: Eine Seite Text dauert auf den meisten Computern einige Sekunden. Der Aufgabenbereich übersetzt auf einmal höchstens 1.000.000 Buchstaben (etwa 200 Seiten). Sind mehr markiert, weist er darauf hin und bittet Sie, weniger auszuwählen. Lange Absätze zerlegt er in Stücke von etwa 1.500 Buchstaben, möglichst am Satzende, und setzt sie danach wieder zusammen.

## Die erste Verwendung einer Sprache

Für jede Richtung braucht eine Übersetzung einen Satz Dateien, ein sogenanntes Modell: eines für Englisch nach Deutsch, ein weiteres für Deutsch nach Englisch. Jedes Modell ist etwa 37 MB groß. Die Dateien gehören nicht zu JeffPub. Wenn Sie zum ersten Mal mit einer Sprache übersetzen, teilt der Aufgabenbereich mit, was benötigt wird und wie groß es ist, zum Beispiel „Deutsch herunterladen (74 MB)?“. Es wird erst etwas heruntergeladen, wenn Sie auf diese Schaltfläche klicken. Der Aufgabenbereich zeigt an, wie weit der Download ist. Mit **Abbrechen** stoppen Sie ihn, und dabei bleibt nichts zurück.

JeffPub prüft jede Datei anhand einer Prüfsumme (einem SHA-256-Fingerabdruck), bevor sie verwendet wird. Eine Datei, die nicht übereinstimmt, wird verworfen, und die Sprache wird nicht installiert.

Die Dateien werden in einem Ordner gespeichert, den alle JeffOffice-Apps gemeinsam nutzen (einem Ordner namens JeffOffice in Ihrem Benutzerdatenordner, unter Translation/models). So ist eine Sprache, die Sie in einer App herunterladen, auch in den anderen verfügbar, und sie wird nur einmal heruntergeladen. Zwei Sprachen ohne eigenes Modell, etwa Deutsch und Französisch, werden über Englisch übersetzt; deshalb braucht der Aufgabenbereich beide Modelle.

## Wenn eine Sprache kein Modell hat

Gibt es für ein Sprachpaar kein Modell, weist der Aufgabenbereich darauf hin und bietet **Stattdessen im Browser öffnen** an. Dabei wird Ihr Text in einer kostenlosen Übersetzungswebsite geöffnet, LibreTranslate (oder dem Dienst, dessen Adresse Sie in den entsprechenden Einstellungen gewählt haben). Erst dann verlässt Ihr Text Ihren Computer, denn er wird an diese Website gesendet, und der Aufgabenbereich sagt nicht mehr, dass er auf Ihrem Computer bleibt. Es wird nichts gesendet, bevor Sie auf diese Schaltfläche klicken.

## Zur Qualität

Die Modelle sind klein und laufen deshalb auf einem gewöhnlichen Computer schnell. Sie übersetzen Sätze gut, kurze Beschriftungen dagegen weniger gut: Ein einzelnes Wort oder der Name einer Schaltfläche kann falsch übersetzt werden. Lesen Sie die Übersetzung, bevor Sie sie verwenden. Namen, Zahlen und Tabstopps bleiben unverändert.

Bilder, Tabstopps und Zeilenumbrüche innerhalb eines Absatzes bleiben unangetastet. Die Übersetzung füllt nur die Wörter dazwischen aus.

## Siehe auch

- [Rechtschreibung, Thesaurus, Recherche und Silbentrennung](proofing)
