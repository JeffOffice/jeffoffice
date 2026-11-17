# Publikationen öffnen und speichern
<!-- keywords: öffnen, speichern, speichern unter, Kopie speichern, schließen, zuletzt verwendet, angeheftet, Dateitypen, jpub, umbenennen, Sicherung, Sicherungskopie, Eigenschaften, Info, als final markieren, prüfen, Versionsverlauf, neuer Name, kopieren, Strg+S, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, Meine Vorlagen, als Vorlage speichern, reparieren, öffnen und reparieren, beschädigt, Kompatibilität prüfen, trotzdem speichern, nicht mehr fragen -->

JeffPub speichert Ihre Arbeit in .jpub-Dateien. Das Programm kann außerdem .pub-Dateien öffnen und speichern. Siehe [Mit .pub-Dateien arbeiten](pub-files).

## Speichern

Drücken Sie Strg+S, klicken Sie in der Schnellsymbolleiste auf die Schaltfläche **Speichern**, oder klicken Sie auf **Datei > Speichern**. Beim ersten Speichern fragt JeffPub nach einem Namen und einem Speicherort. Danach ersetzt **Speichern** die Datei durch Ihre neueste Fassung.

Ein Stern (*) nach dem Namen in der Titelleiste bedeutet, dass Sie Änderungen haben, die nicht gespeichert sind. Speichern Sie häufig. Wenn JeffPub unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Um die zuletzt gespeicherte Fassung einer Datei zu behalten, aktivieren Sie **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffPub zuerst die alte Datei in eine Datei mit dem Namen „Sicherung von“ plus dem bisherigen Dateinamen, im selben Ordner. Lässt sich die Kopie nicht anlegen, fragt das Programm, ob ohne Kopie gespeichert werden soll. Die Antwort lautet Nein, sofern Sie nicht Ja wählen, und die Datei bleibt unverändert. Diese Option finden Sie unter **Datei > Optionen** auf der Registerkarte **Allgemein**. Jede JeffOffice-App hat sie.

## Mit neuem Namen speichern

Klicken Sie auf **Datei > Speichern unter**, oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie unter **Dateityp** das gewünschte Format, und klicken Sie auf **Speichern**. Zur Auswahl stehen:

- **JeffPub-Publikation (*.jpub)** ist die übliche Wahl.
- **.pub-Publikationsdateien (*.pub)** erstellt eine .pub-Datei.
- **PDF (*.pdf)** erstellt ein PDF und lässt Ihre Publikation unverändert.
- **JeffPub-Vorlage (*.jpub)** fügt die Publikation unter **Meine Vorlagen** hinzu.

Nach dem Speichern als .jpub- oder .pub-Datei zeigt die Titelleiste den neuen Namen, und Sie arbeiten in der neuen Datei weiter.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern** und wählen Sie einen Namen. JeffPub schreibt die Publikation in diese Datei, und Sie arbeiten in der bisherigen Datei weiter. Deren Name und nicht gespeicherte Änderungen bleiben wie zuvor.

## Öffnen

1. Drücken Sie Strg+O, oder klicken Sie auf **Datei > Öffnen**.
2. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. Oder klicken Sie auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Publikationen**.

Sie können eine .jpub- oder .pub-Datei auch auf die Seite ziehen. Hat Ihre aktuelle Publikation nicht gespeicherte Änderungen, fragt JeffPub zuerst danach.

Klicken Sie mit der rechten Maustaste auf einen Namen in der Liste, um ihn **An Liste anheften** (er bleibt oben) oder **Aus Liste entfernen**. Ein grau dargestellter Name bedeutet, dass JeffPub die Datei derzeit nicht finden kann. Wie Sie die Anzahl der angezeigten zuletzt verwendeten Namen ändern, lesen Sie unter [JeffPub-Einstellungen ändern](options).

## Andere Möglichkeiten zum Öffnen

Auf der Seite **Öffnen** klicken Sie auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken.

- **Schreibgeschützt öffnen** öffnet die Publikation nur zum Lesen. Sie können sie ansehen und blättern, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“. Wenn Sie **Speichern** drücken, fragt JeffPub nach einem neuen Namen, damit die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet die Publikation ohne eigene Datei. Beim **Speichern** fragt JeffPub, wo sie abgelegt werden soll.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster.
- **Öffnen und reparieren** ist für eine beschädigte .jpub-Datei gedacht, zum Beispiel für eine, die beim Kopieren abgeschnitten wurde. JeffPub liest jeden lesbaren Teil der Datei und lässt einen nicht lesbaren Teil weg. Die Publikation wird als Kopie mit dem Namen „Repariert“ plus dem Dateinamen geöffnet, und eine Meldung nennt die weggelassenen Teile. Die beschädigte Datei wird nicht verändert.

## Schließen

Klicken Sie auf **Datei > Schließen**, oder drücken Sie Strg+F4. JeffPub fragt, ob Änderungen gespeichert werden sollen, und zeigt dann eine leere Seite an.

## Info: Eigenschaften, Als final markieren, Prüfen und Versionen

**Datei > Info** zeigt den Speicherort der Datei mit den Schaltflächen **Im Ordner anzeigen** und **Pfad kopieren**. Außerdem stehen dort Angaben zu Ihrer Publikation: Seiten, Seitengröße, Bilder, Wörter und Datumsangaben. Tragen Sie in die Felder **Titel**, **Autor**, **Thema**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager** ein, um die Eigenschaften festzulegen, oder klicken Sie auf **Benutzerdefinierte Eigenschaften** für eigene Eigenschaften. Die Änderung wird mit der Publikation gespeichert, und **Strg+Z** macht sie rückgängig. Auf der Seite finden Sie außerdem Ihre Unternehmensinformationen, die **Einstellungen für professionellen Druck** (siehe [PDF-Dateien und Druckereien](pdf)) und die [Entwurfsprüfung](design-checker).

- **Kompatibilität prüfen** listet auf, was eine .pub-Datei von Ihrer Publikation nicht speichern kann, ohne dabei etwas zu speichern: ausgeblendete Objekte, bestimmte Listenformate und alles andere, was der .pub-Schreiber auslässt. Wenn Sie als .pub-Datei speichern, zeigt JeffPub dieselbe Liste **vor** dem Schreiben an. Wählen Sie **Trotzdem speichern**, **Als .jpub speichern** (die .jpub-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen**. Aktivieren Sie **Für diese Datei nicht erneut fragen**, dann fragt JeffPub bei dieser Datei nicht mehr, bis Sie das Programm schließen.
- **Als final markieren** speichert die Publikation und hält sie danach nur zum Lesen bereit. Beim Öffnen zeigt die Titelleiste, dass die Datei schreibgeschützt ist, und ein Hinweis teilt mit, dass sie als final markiert wurde. Das ist keine Sperre: Wählen Sie **Als final markiert** erneut, um sie zu bearbeiten. Eine .pub-Datei kann die Markierung nicht speichern, speichern Sie deshalb zuerst als .jpub-Datei.
- **Publikation prüfen** listet auf, was die Datei enthält und was Sie vielleicht nicht weitergeben möchten, und entfernt es: die Kommentare und die Namen der Personen, die sie geschrieben haben, den Autor und andere Namen in den Eigenschaften, die Unternehmensinformationen, die Seriendruckliste und die Objekte, die auf dem Arbeitsbereich neben den Seiten abgelegt sind.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten entfernt bei jedem Speichern den Autor, die Person, die die Datei zuletzt gespeichert hat, die Firma und den Manager aus der Datei, für eine .jpub- oder eine .pub-Datei. Es ist dasselbe Kontrollkästchen wie unter **Optionen** und in jeder JeffOffice-App.
- **Versionsverlauf** speichert benannte Kopien der Publikation auf Ihrem Computer. Speichern Sie die Publikation zuerst. Klicken Sie auf **Version speichern…**, und wählen Sie später eine Version aus, um **Kopie öffnen**, **Zu dieser Version zurückkehren** (die aktuelle Publikation wird vorher als Version gespeichert) oder **Löschen** zu wählen.

Eine .jpub-Datei kann kein Kennwort speichern, deshalb gibt es in JeffPub keine Karte für Kennwörter. JeffPub hat noch keine Prüfung der Barrierefreiheit, daher gibt es auf der Seite dafür ebenfalls keine Karte.

## Wenn Windows JeffPub das Speichern verwehrt

Die Windows-Sicherheit kann Programmen das Speichern in Ordnern wie Dokumente untersagen. JeffPub zeigt dann eine Meldung mit Schritten, um das Speichern zu erlauben. Alternativ speichern Sie in einem anderen Ordner.

## Siehe auch

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Mit .pub-Dateien arbeiten](pub-files)
- [Neue Publikation beginnen](create)
- [JeffPub-Einstellungen ändern](options)
