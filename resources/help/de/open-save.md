# Publikationen öffnen und speichern
<!-- keywords: öffnen, speichern, speichern unter, Kopie speichern, schließen, zuletzt verwendete Dateien, angeheftet, Dateitypen, jpub, umbenennen, Sicherungskopie, Eigenschaften, Info, als endgültig markieren, prüfen, Versionsverlauf, neuer Name, kopieren, Strg+S, schreibgeschützt, nur lesen, als Kopie öffnen, neues Fenster, meine Vorlagen, als Vorlage speichern, reparieren, öffnen und reparieren, beschädigt, Kompatibilität prüfen, trotzdem speichern, nicht erneut fragen -->

JeffPub speichert Ihre Arbeit in .jpub-Dateien. Das Programm öffnet und speichert außerdem .pub-Dateien. Siehe [Mit .pub-Dateien arbeiten](pub-files).

## Speichern

Drücken Sie Strg+S, klicken Sie in der Schnellsymbolleiste auf die Schaltfläche **Speichern**, oder klicken Sie auf **Datei > Speichern**. Beim ersten Mal fragt JeffPub nach einem Namen und einem Speicherort. Danach ersetzt „Speichern“ die Datei durch Ihren neuesten Stand.

Ein Stern (*) hinter dem Namen in der Titelleiste bedeutet, dass Sie Änderungen haben, die noch nicht gespeichert sind. Speichern Sie häufig. Wenn JeffPub unerwartet beendet wird, lesen Sie [Nicht gespeicherte Arbeit wiederherstellen](recover).

Um die letzte gespeicherte Fassung einer Datei aufzubewahren, aktivieren Sie **Immer Sicherungskopie erstellen**. Bei jedem Speichern kopiert JeffPub zuerst die alte Datei in eine Datei mit dem Präfix „Sicherungskopie von“ und dem bisherigen Namen, im selben Ordner. Kann die Kopie nicht erstellt werden, fragt das Programm, ob ohne Sicherungskopie gespeichert werden soll. Die Antwort ist Nein, es sei denn, Sie sagen Ja, und die Datei bleibt dann unverändert. Diese Option finden Sie unter **Datei > Optionen** auf der Registerkarte **Allgemein**. Jede JeffOffice-App hat sie.

## Mit neuem Namen speichern

Klicken Sie auf **Datei > Speichern unter**, oder drücken Sie F12. Geben Sie einen Namen ein, wählen Sie unter **Dateityp** das gewünschte Format, und klicken Sie auf **Speichern**. Zur Auswahl stehen:

- **JeffPub-Publikation (*.jpub)** ist die übliche Wahl.
- **.pub-Publikationsdateien (*.pub)** erstellt eine .pub-Datei.
- **PDF (*.pdf)** erstellt ein PDF und lässt Ihre Publikation unverändert.
- **JeffPub-Vorlage (*.jpub)** fügt die Publikation zu **Meine Vorlagen** hinzu.

Nach dem Speichern als .jpub- oder .pub-Datei zeigt die Titelleiste den neuen Namen an, und Sie arbeiten in der neuen Datei weiter.

## Kopie speichern

Klicken Sie auf **Datei > Kopie speichern**, und wählen Sie einen Namen. JeffPub schreibt die Publikation in diese Datei, und Sie arbeiten in der bisherigen weiter. Ihr Name und die noch nicht gespeicherten Änderungen bleiben unverändert.

## Öffnen

1. Drücken Sie Strg+O, oder klicken Sie auf **Datei > Öffnen**.
2. Klicken Sie auf **Durchsuchen**, um eine Datei auszuwählen. Oder klicken Sie auf einen Namen unter **Angeheftet** oder **Zuletzt verwendete Publikationen**.

Sie können eine .jpub- oder .pub-Datei auch auf die Seite ziehen. Hat Ihre aktuelle Publikation nicht gespeicherte Änderungen, fragt JeffPub zuerst danach.

Klicken Sie mit der rechten Maustaste auf einen Namen in der Liste, um **An Liste anheften** (er bleibt oben) oder **Aus Liste entfernen** zu wählen. Ein grau dargestellter Name bedeutet, dass JeffPub die Datei derzeit nicht finden kann. Wie Sie die Anzahl der angezeigten zuletzt verwendeten Namen ändern, lesen Sie unter [JeffPub-Einstellungen ändern](options).

## Weitere Möglichkeiten zum Öffnen

Klicken Sie auf der Seite **Öffnen** auf **Weitere Möglichkeiten zum Öffnen**. Außerdem können Sie mit der rechten Maustaste auf einen Namen in der Liste der zuletzt verwendeten Dateien klicken.

- **Schreibgeschützt öffnen** öffnet die Publikation nur zum Lesen. Sie können blättern und scrollen, aber nichts ändern. Die Titelleiste zeigt „Schreibgeschützt“ an. Wenn Sie auf **Speichern** klicken, fragt JeffPub nach einem neuen Namen, sodass die geöffnete Datei unverändert bleibt.
- **Als Kopie öffnen** öffnet die Publikation ohne eigene Datei. **Speichern** fragt, wohin sie gelegt werden soll.
- **In neuem Fenster öffnen** öffnet die Datei in einem eigenen Fenster.
- **Öffnen und reparieren** ist für eine beschädigte .jpub-Datei gedacht, etwa für eine, die beim Kopieren abgeschnitten wurde. JeffPub liest jeden lesbaren Teil der Datei und lässt einen nicht lesbaren Teil weg. Die Publikation öffnet sich als Kopie mit dem Namen „Repariert“ plus dem bisherigen Namen, und eine Meldung nennt die weggelassenen Teile. Die beschädigte Datei bleibt unverändert.

## Schließen

Klicken Sie auf **Datei > Schließen**, oder drücken Sie Strg+F4. JeffPub fragt, ob Änderungen gespeichert werden sollen, und zeigt danach eine leere Seite an.

## Info: Eigenschaften, Als endgültig markieren, Prüfen und Versionen

**Datei > Info** zeigt den Speicherort der Datei mit **Im Ordner anzeigen** und **Pfad kopieren**. Außerdem werden Angaben zur Publikation angezeigt: Seiten, Seitengröße, Bilder, Wörter und Datumsangaben. Geben Sie in den Feldern **Titel**, **Autor**, **Thema**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager** ein, oder klicken Sie auf **Benutzerdefinierte Eigenschaften** für eigene Eigenschaften. Die Änderung wird mit der Publikation gespeichert, und **Strg+Z** macht sie rückgängig. Die Seite öffnet außerdem Ihre Unternehmensinformationen, die **Einstellungen für professionellen Druck** (siehe [PDF-Dateien und Druckereien](pdf)) und die [Entwurfsprüfung](design-checker).

- **Kompatibilität prüfen** listet auf, was eine .pub-Datei von Ihrer Publikation nicht speichern kann, ohne etwas zu speichern: ausgeblendete Objekte, einige Listenformate und alles andere, was der .pub-Schreibvorgang auslässt. Wenn Sie als .pub-Datei speichern, zeigt JeffPub dieselbe Liste **vor** dem Schreiben an. Wählen Sie **Trotzdem speichern**, **Als .jpub speichern** (die .jpub-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen**. Aktivieren Sie **Für diese Datei nicht erneut fragen**, dann fragt JeffPub bei dieser Datei nicht mehr, bis Sie das Programm schließen.
- **Immer schreibgeschützt öffnen** sorgt dafür, dass die Publikation bei jedem Öffnen nur zum Lesen geöffnet wird. Die Einstellung wird mit einer .jpub-Datei gespeichert, und die Statusleiste zeigt es an, wenn jemand die Datei öffnet. Zum Bearbeiten wählen Sie auf derselben Seite **Wird immer schreibgeschützt geöffnet** ab. Das ist eine Erinnerung und keine Sperre. Eine .pub-Datei kann diese Einstellung nicht speichern (der .pub-Schreibvorgang behält keine Eigenschaften dieser Art, und JeffPub liest sie aus einer .pub-Datei auch nicht aus). Speichern Sie daher zuerst als .jpub-Datei. Die Kompatibilitätsprüfung listet sie für eine .pub-Datei auf.
- **Als endgültig markieren** speichert die Publikation und hält sie danach nur zum Lesen bereit. Wenn jemand sie öffnet, zeigt die Titelleiste an, dass sie schreibgeschützt ist, und ein Hinweis teilt mit, dass sie als endgültig markiert wurde. Das ist keine Sperre: Wählen Sie erneut **Als endgültig markiert**, um sie zu bearbeiten. Eine .pub-Datei kann diese Markierung nicht speichern, speichern Sie daher zuerst als .jpub-Datei.
- **Publikation prüfen** listet auf, was die Datei enthält und was Sie vielleicht nicht weitergeben möchten, und entfernt es: die Kommentare und die Namen der Personen, die sie geschrieben haben, den Autor und andere Namen in den Eigenschaften, die Unternehmensinformationen, die Seriendruckliste und die Objekte im Arbeitsbereich neben den Seiten.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten entfernt bei jedem Speichern den Autor, die Person, die die Datei zuletzt gespeichert hat, die Firma und den Manager aus der Datei, für eine .jpub- oder eine .pub-Datei. Es ist dasselbe Kontrollkästchen wie unter **Optionen** und in jeder JeffOffice-App.
- **Versionsverlauf** bewahrt benannte Kopien der Publikation auf Ihrem Computer auf. Speichern Sie die Publikation zuerst. Klicken Sie auf **Version speichern…**, und wählen Sie später eine Version aus, um **Kopie öffnen**, **Zu dieser Version zurückkehren** (die aktuelle Publikation wird vorher als Version gespeichert) oder **Löschen** zu wählen.

Eine .jpub-Datei hat keine Form für ein Kennwort, daher hat JeffPub keine Karte für ein Kennwort. JeffPub hat noch keine Prüfung der Barrierefreiheit, daher gibt es auf der Seite dafür keine Karte.

## Wenn Windows JeffPub das Speichern nicht erlaubt

Die Windows-Sicherheit kann Programmen das Speichern in Ordnern wie Dokumente verbieten. JeffPub zeigt dann eine Meldung mit Schritten, um es zu erlauben. Oder speichern Sie in einem anderen Ordner.

## Siehe auch

- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Mit .pub-Dateien arbeiten](pub-files)
- [Neue Publikation beginnen](create)
- [JeffPub-Einstellungen ändern](options)
