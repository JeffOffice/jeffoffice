# Publikationen öffnen und speichern
<!-- keywords: öffnen, speichern, speichern unter, Kopie speichern, schließen, zuletzt verwendete Dateien, angeheftet, Dateitypen, jpub, umbenennen, Sicherungskopie, Backup, Eigenschaften, Info, als final markieren, prüfen, Versionsverlauf, neuer Name, kopieren, Strg+S, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, meine Vorlagen, als Vorlage speichern, reparieren, öffnen und reparieren, beschädigt -->

JeffPub speichert Ihre Arbeit in .jpub-Dateien. Die Software kann auch .pub-Dateien öffnen und speichern. Siehe [Mit .pub-Dateien arbeiten](pub-files).

## Speichern

Drücken Sie Strg+S, klicken Sie in der Schnellsymbolleiste auf die Schaltfläche **Speichern**, oder klicken Sie auf **Datei > Speichern**. Beim ersten Mal fragt JeffPub nach einem Namen und einem Speicherort. Danach ersetzt **Speichern** die Datei durch Ihre neueste Arbeit.

Ein Stern (*) hinter dem Namen in der Titelleiste bedeutet, dass Sie Änderungen haben, die nicht gespeichert sind. Speichern Sie oft. Wenn JeffPub unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Um die zuletzt gespeicherte Fassung einer Datei zu behalten, aktivieren Sie **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffPub zuerst die alte Datei in eine Datei mit dem Namen „Sicherungskopie von“ plus dem Dateinamen, im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne Kopie gespeichert werden soll. Die Antwort ist Nein, sofern Sie nicht Ja wählen; dann bleibt die Datei unverändert. Sie finden diese Option unter **Datei > Optionen** auf der Registerkarte **Allgemein**. Jede JeffOffice-App hat sie.

## Mit neuem Namen speichern

Klicken Sie auf **Datei > Speichern unter**, oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie unter **Dateityp** das gewünschte Format, und klicken Sie auf **Speichern**. Zur Auswahl stehen:

- **JeffPub-Publikation (*.jpub)** ist die übliche Wahl.
- **.pub-Publikationsdateien (*.pub)** erstellt eine .pub-Datei.
- **PDF (*.pdf)** erstellt ein PDF, und Ihre Publikation bleibt unverändert.
- **JeffPub-Vorlage (*.jpub)** fügt die Publikation zu **Meine Vorlagen** hinzu.

Nach dem Speichern als .jpub- oder .pub-Datei zeigt die Titelleiste den neuen Namen, und Sie arbeiten in der neuen Datei weiter.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**, und wählen Sie einen Namen. JeffPub schreibt die Publikation in diese Datei, und Sie arbeiten in der bisherigen Datei weiter. Dabei bleiben der Name und die nicht gespeicherten Änderungen der bisherigen Datei unverändert.

## Öffnen

1. Drücken Sie Strg+O, oder klicken Sie auf **Datei > Öffnen**.
2. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. Oder klicken Sie auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Publikationen**.

Sie können auch eine .jpub- oder .pub-Datei auf die Seite ziehen. Hat Ihre aktuelle Publikation nicht gespeicherte Änderungen, fragt JeffPub zuerst danach.

Klicken Sie mit der rechten Maustaste auf einen Namen in der Liste, um ihn **An Liste anheften** (er bleibt dann oben) oder **Aus Liste entfernen**. Ein grau dargestellter Name bedeutet, dass JeffPub die Datei nicht finden kann. Wie Sie die Anzahl der angezeigten zuletzt verwendeten Namen ändern, lesen Sie unter [JeffPub-Einstellungen ändern](options).

## Andere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Sie können auch mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken.

- **Schreibgeschützt öffnen** öffnet die Publikation nur zum Lesen. Sie können sie ansehen und durchblättern, aber nichts ändern. In der Titelleiste steht „Schreibgeschützt“. Wenn Sie auf **Speichern** klicken, fragt JeffPub nach einem neuen Namen, sodass die geöffnete Datei nicht verändert wird.
- **Als Kopie öffnen** öffnet die Publikation ohne eigene Datei. Beim **Speichern** fragt JeffPub, wohin sie gespeichert werden soll.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster.
- **Öffnen und reparieren** ist für eine beschädigte .jpub-Datei gedacht, beispielsweise eine, die beim Kopieren abgeschnitten wurde. JeffPub liest jeden lesbaren Teil der Datei und lässt einen nicht lesbaren Teil weg. Die Publikation wird als Kopie mit dem Namen „Repariert“ plus dem Dateinamen geöffnet, und eine Meldung nennt, was weggelassen wurde. Die beschädigte Datei wird nicht verändert.

## Schließen

Klicken Sie auf **Datei > Schließen**, oder drücken Sie Strg+F4. JeffPub fragt, ob Änderungen gespeichert werden sollen, und zeigt danach eine leere Seite.

## Info: Eigenschaften, Als final markieren, Prüfen und Versionen

**Datei > Info** zeigt, wo die Datei liegt, mit **Im Ordner anzeigen** und **Pfad kopieren**. Die Seite zeigt außerdem Angaben zu Ihrer Publikation: Seiten, Seitengröße, Bilder, Wörter und Datumsangaben. Geben Sie in den Feldern den **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager** ein, oder klicken Sie auf **Benutzerdefinierte Eigenschaften** für eigene Eigenschaften. Die Änderung wird mit der Publikation gespeichert; mit Strg+Z machen Sie sie rückgängig. Die Seite öffnet außerdem Ihre Unternehmensinformationen, die **Einstellungen für professionellen Druck** (siehe [PDF-Dateien und Druckereien](pdf)) und die [Entwurfsprüfung](design-checker).

- **Als final markieren** speichert die Publikation und setzt sie anschließend auf Nur-Lesen. Wer sie öffnet, sieht in der Titelleiste, dass sie schreibgeschützt ist, und ein Hinweis teilt mit, dass sie als final markiert wurde. Das ist kein Schloss: Wählen Sie **Als final markiert** erneut, um sie zu bearbeiten. Eine .pub-Datei kann diese Markierung nicht speichern; speichern Sie deshalb zuerst als .jpub-Datei.
- **Publikation prüfen** listet auf, was die Datei enthält und das Sie vielleicht nicht weitergeben möchten, und entfernt es: die Kommentare und die Namen ihrer Verfasser, den Autor und andere Namen in den Eigenschaften, die Unternehmensinformationen, die Seriendruckliste und die Objekte im Arbeitsbereich neben den Seiten.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten entfernt bei jedem Speichern den Autor, die Person, die die Datei zuletzt gespeichert hat, die Firma und den Manager aus der Datei, für eine .jpub- oder eine .pub-Datei. Es ist dasselbe Kontrollkästchen wie in **Optionen** und in jeder JeffOffice-App.
- **Versionsverlauf** bewahrt benannte Kopien der Publikation auf Ihrem Computer auf. Speichern Sie die Publikation zuerst. Klicken Sie auf **Version speichern…**, und wählen Sie später eine Version aus, um **Kopie öffnen**, **Zu dieser Version zurückkehren** (die aktuelle Publikation wird vorher als Version gespeichert) oder **Löschen** zu wählen.

Eine .jpub-Datei hat keine Form für ein Kennwort, daher gibt es in JeffPub keine Kennwortkarte. JeffPub hat noch keine Prüfung der Barrierefreiheit, daher gibt es für sie auf der Seite keine Karte.

## Wenn Windows JeffPub das Speichern nicht erlaubt

Die Windows-Sicherheit kann Programmen das Speichern in Ordnern wie Dokumente blockieren. JeffPub zeigt dann eine Meldung mit Schritten, wie Sie das Speichern erlauben. Alternativ speichern Sie in einem anderen Ordner.

## Siehe auch

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Mit .pub-Dateien arbeiten](pub-files)
- [Neue Publikation beginnen](create)
- [JeffPub-Einstellungen ändern](options)
