# Einstellungen von JeffDeck ändern

<!-- keywords: Optionen, Einstellungen, Voreinstellungen, Design, Designs, dunkler Modus, Sprache, Anzeigesprache, Benutzername, Initialen, Lineal, Einrasten, Rechtschreibung, Autokorrektur, Anführungszeichen, Updates, Beta, Nutzungsstatistik, Info, Speichern, Schriftarten einbetten, Schriftarten, Tastatur anpassen, Menüband anpassen, Schnellsymbolleiste anpassen, Schnellsymbolleiste, Befehl zur Schnellsymbolleiste hinzufügen, Registerkarte ausblenden, Gruppe ausblenden, Registerkarte hinzufügen, Gruppe hinzufügen, Registerkarte umbenennen, Menüband zurücksetzen, Tastenkombinationen ändern, Tasten neu zuweisen, Tasten zurücksetzen -->

Ihre Einstellungen finden Sie im Fenster **JeffDeck-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Es hat fünf Registerkarten. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Allgemein

- **Benutzername** und **Initialen** werden bei den Kommentaren angezeigt, die Sie schreiben. Siehe [Kommentare](comments).
- **Design** ist **Systemeinstellung verwenden**, **Hell** oder **Dunkel**. Die Änderung wird sofort angezeigt.
- **Anzeigesprache** ist die Sprache der Oberfläche von JeffDeck. **Systemsprache verwenden** richtet sich nach Ihrem Computer. Weitere Sprachen erscheinen in dieser Liste, sobald Übersetzungen verfügbar sind. Eine neue Sprache wird beim nächsten Start von JeffDeck wirksam.
- **Beim Start von JeffDeck nach Updates suchen** bewirkt, dass JeffDeck bei jedem Start nach einer neueren Version sucht. Ist eine verfügbar, zeigt das Fenster **Update verfügbar**, was neu ist. Wählen Sie **Jetzt aktualisieren**, **Später** oder **Diese Version überspringen**. Unter Windows lädt **Jetzt aktualisieren** das Installationsprogramm herunter. Es installiert die neue Version aller JeffOffice-Apps und fordert Sie auf, geöffnete Apps zu schließen. Auf anderen Computern heißt die Schaltfläche **Downloadseite öffnen**. Sie öffnet die Seite, auf der Sie die neue Version erhalten. Ist auch JeffPub geöffnet, meldet täglich nur eines der beiden Programme eine neue Version.
- **Betaversionen erhalten (frühe Builds zum Testen)** sorgt dafür, dass JeffDeck auch Betaversionen anbietet. Diese erscheinen vor den regulären Versionen und können noch Fehler enthalten.
- **Anonyme Nutzungsstatistiken senden** erlaubt es, Angaben darüber weiterzugeben, wie JeffDeck verwendet wird. Anonym heißt, dass die Angaben nichts darüber verraten, wer Sie sind. Ist dies aktiviert, sendet JeffDeck diese Angaben einmal am Tag und am Tag einer Aktualisierung: dass es sich um JeffDeck handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft das Programm gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben (zum Beispiel .pptx). Schließt sich JeffDeck unerwartet, sendet es beim nächsten Start außerdem einen Absturzbericht. Der Bericht sagt, an welcher Stelle im Programm es abbrach, damit das Problem gefunden und behoben werden kann. JeffDeck sendet niemals Ihre Dateien, deren Namen oder deren Inhalte und nichts von dem, was Sie eingeben. Ist dies deaktiviert, wird auch kein Absturzbericht gesendet. JeffDeck fragt beim ersten Start nach, im Fenster **Helfen Sie, JeffDeck zu verbessern**. Die Frage entfällt, wenn Sie bereits in JeffPub oder im Setup entschieden haben.
- **Datenschutzhinweis** neben diesem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Statistiken speichern wir 24 Monate.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Sie ist eine Zufallszahl für Ihre Kopie von JeffOffice und in allen JeffOffice-Apps gleich. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie in die Zwischenablage zu kopieren. Sind die Statistiken deaktiviert oder wurde noch keine Nummer erzeugt, steht dort **keine**.
- Wenn die bei uns vorhandenen Statistiken gelöscht werden sollen, senden Sie Ihre Installationsnummer **bevor** Sie die Statistiken deaktivieren an info@jeffoffice.com. Beim Deaktivieren wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihnen gehören.
- Um die Statistiken abzuschalten, ohne JeffDeck zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts erfasst oder gesendet, und JeffDeck stellt die Frage beim ersten Start nicht.

**Benutzername**, **Initialen**, **Design** und **Anzeigesprache** sowie die Entscheidungen zu Updates und Statistiken gelten für alle JeffOffice-Apps: Ändern Sie sie hier, werden sie in jeder dieser Apps geändert.

## Dokumentprüfung

Diese Kontrollkästchen steuern die Rechtschreibprüfung und die Schreibhilfen. Siehe [Text eingeben und formatieren](text).

- **Rechtschreibung während der Eingabe prüfen**
- **Grammatik während der Eingabe prüfen** markiert eindeutige Grammatik- und Stilfehler in englischen, französischen, deutschen und spanischen Texten mit einer blauen gepunkteten Linie.
- **Grammatikeinstellungen** öffnet eine Liste der Fehlerarten, nach denen gesucht wird: Grammatik, Zeichensetzung, Groß- und Kleinschreibung, verwechselte Wörter, Wiederholungen und Stil (der ausgeschaltet ist, bis Sie ihn aktivieren).
- **Wörter in GROSSBUCHSTABEN ignorieren**
- **Wörter mit Zahlen ignorieren**
- **Text während der Eingabe ersetzen (AutoKorrektur)** korrigiert typische Tippfehler.
- **Gerade Anführungszeichen durch typografische ersetzen** wandelt " und ' in typografische Anführungszeichen um.
- **Während der Eingabe automatisch formatieren (Striche, Brüche, Ordnungszahlen, automatische Listen)**
- **Webadressen beim Eingeben automatisch in Links umwandeln** macht eine Web- oder E-Mail-Adresse zu einem Link, sobald Sie danach die Leertaste drücken. Mit **Rückgängig** wird nur der Link zurückgenommen.

## Ansicht

- **Lineal anzeigen** blendet die Lineale am Rand der Folie ein. Das entspricht **Ansicht > Anzeigen > Lineal** und gilt für alle Fenster, jetzt und für alle, die Sie später öffnen.
- **Objekte an anderen Objekten ausrichten** sorgt dafür, dass ein Objekt, das Sie verschieben, sich an den anderen Objekten der Folie ausrichtet.

## Speichern

- **Schriftarten in der Datei einbetten** speichert die Schriftarten, die Ihre Folien verwenden, beim Speichern in der Datei. So sehen die Folien auch auf einem Computer gleich aus, auf dem diese Schriftarten fehlen. Das ist bis zum Deaktivieren eingeschaltet. Es werden nur Schriftarten eingebettet, die mit JeffDeck geliefert werden, und die Datei wird dadurch größer. Details dazu finden Sie im Thema Präsentationen öffnen und speichern unter „Schriftarten in der Datei“.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** entfernt beim Schreiben der Datei (eine .pptx- oder .odp-Datei, eine Vorlage und eine per E-Mail gesendete Präsentation) den Autor, die Person, die die Datei zuletzt gespeichert hat, das Unternehmen, den Manager sowie die Namen, Initialen und Zeitangaben in Kommentaren. Der Inhalt der Datei bleibt unverändert. Die benutzerdefinierten Eigenschaften der Datei bleiben ebenfalls erhalten, und das geöffnete Dokument wird nicht verändert. Diese Option gilt für alle JeffOffice-Apps: Ändern Sie sie in einer App, wird sie in allen geändert. Dasselbe Kontrollkästchen finden Sie auf der Seite **Info**.

## Anpassen

Die Registerkarte **Anpassen** hat drei Seiten: **Menüband**, **Schnellsymbolleiste** und **Tastatur**. Auf der Seite **Tastatur** wählen Sie die Tastenkombinationen für die Befehle von JeffDeck. Siehe „Eigene Tastenkombinationen festlegen“ im Thema zur Tastatur.

### Menüband anpassen

Die Seite **Menüband** listet die Registerkarten des Menübands mit ihren Gruppen auf.

- **Registerkarte oder Gruppe ausblenden oder anzeigen.** Deaktivieren Sie das Kontrollkästchen daneben, um sie auszublenden, und aktivieren Sie es, um sie wieder anzuzeigen. Eine ausgeblendete Gruppe braucht keinen Platz, und ihre Befehle funktionieren weiterhin über ihre Tastenkombinationen. **Start** hat kein Kontrollkästchen, weil die Registerkarte immer angezeigt wird, ebenso die Schaltfläche **Datei**. Eine Registerkarte, die nur erscheint, wenn etwas markiert ist, etwa ein Bild oder eine Tabelle, steht am Ende der Liste und hat kein Kontrollkästchen.
- **Eigene Registerkarte oder Gruppe erstellen.** Geben Sie im Feld **Name** einen Namen ein, und klicken Sie auf **Neue Registerkarte**. Um eine Gruppe zu erstellen, wählen Sie eine Registerkarte und klicken auf **Neue Gruppe**. Eine eigene Gruppe kann auf jeder Registerkarte stehen, auch auf einer Registerkarte, die nur bei Markierungen erscheint.
- **Befehle in Ihre Gruppe aufnehmen.** Wählen Sie Ihre Gruppe, suchen Sie einen Befehl in der Liste **Befehle** (geben Sie im Feld **Befehle suchen** einen Begriff ein, um die Liste einzugrenzen), und klicken Sie auf **Befehl hinzufügen**. Jeder Befehl mit Namen kann aufgenommen werden. Ein Befehl kann nur einmal in einer Gruppe stehen.
- **Umbenennen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl, geben Sie den neuen Namen ein, und klicken Sie auf **Umbenennen**. Ein umbenannter Befehl macht weiterhin dasselbe, der neue Name steht nur auf seiner Schaltfläche. Um einem Befehl seinen ursprünglichen Namen zurückzugeben, benennen Sie ihn mit einem leeren Namen um.
- **Reihenfolge ändern.** Wählen Sie eine Registerkarte und klicken Sie auf **Nach oben** oder **Nach unten**. Sie können die Registerkarten des Menübands ebenso verschieben wie Ihre eigenen, nicht aber die Registerkarten, die nur bei Markierungen erscheinen. Ihre Gruppen und Befehle werden innerhalb ihrer Registerkarte oder Gruppe auf dieselbe Weise verschoben. Die Gruppen des Menübands bleiben in ihrer Reihenfolge.
- **Entfernen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl und klicken Sie auf **Entfernen**.

**Eine zurücksetzen** stellt die gewählte Registerkarte wieder her: Sie wird angezeigt, ihre Gruppen werden angezeigt, und die Gruppen, die Sie ihr hinzugefügt haben, verschwinden. Eine eigene Registerkarte wird entfernt. **Alle zurücksetzen** stellt das gesamte Menüband wieder her, einschließlich der Reihenfolge der Registerkarten.

Erst wenn Sie auf **OK** klicken, wird etwas geändert. Dann zeigt jedes geöffnete Fenster von JeffDeck Ihr Menüband, ebenso jedes Fenster, das Sie später öffnen. JeffDeck speichert nur, was sich vom üblichen Menüband unterscheidet. Eine Registerkarte oder Gruppe, die eine spätere Version ergänzt, erscheint daher von selbst. Ihre Registerkarten und Gruppen erhalten Zugriffstasten (die Buchstaben, die erscheinen, wenn Sie **Alt** drücken), die sich nicht mit den anderen überschneiden. Sie können 20 eigene Registerkarten haben, 30 eigene Gruppen auf einer Registerkarte und 60 Befehle in einer Gruppe.

### Schnellsymbolleiste anpassen

Die Seite **Schnellsymbolleiste** betrifft die kleine Symbolleiste oben links im Fenster. Ihre linke Liste zeigt, was aktuell auf der Symbolleiste steht. Die Liste **Befehle** rechts enthält alle Befehle mit Namen (geben Sie im Feld **Befehle suchen** einen Begriff ein, um sie einzugrenzen, wie auf der Seite **Menüband**).

- **Befehl hinzufügen.** Wählen Sie ihn in der Liste **Befehle** und klicken Sie auf **Befehl hinzufügen**. Er wird am Ende der Symbolleiste eingefügt. Ein Befehl, der ein Menü öffnet, behält sein Menü auf der Symbolleiste. Ein Befehl kann nur einmal auf der Symbolleiste stehen.
- **Rechtsklick.** Sie können auch auf einen beliebigen Befehl im Menüband rechtsklicken und **Zur Schnellsymbolleiste hinzufügen** wählen oder auf eine Schaltfläche der Symbolleiste rechtsklicken und **Aus Schnellsymbolleiste entfernen** wählen. Im selben Menü stehen auch **Schnellsymbolleiste anpassen** und **Menüband anpassen**, die diese Seite bzw. die Seite **Menüband** öffnen.
- **Unterhalb des Menübands anzeigen.** Aktivieren Sie **Schnellsymbolleiste unterhalb des Menübands anzeigen**, damit die Symbolleiste in einer Zeile unter dem Menüband statt oben links steht. Auch in den beiden Rechtsklick-Menüs gibt es **Schnellsymbolleiste unterhalb des Menübands anzeigen**. JeffDeck merkt sich diese Einstellung.
- **Trennlinie hinzufügen.** Klicken Sie auf **Trennzeichen hinzufügen**, um am Ende einen dünnen Strich einzufügen, und verschieben Sie ihn anschließend an die gewünschte Stelle.
- **Reihenfolge ändern.** Wählen Sie in der linken Liste einen Befehl oder ein Trennzeichen und klicken Sie auf **Nach oben** oder **Nach unten**.
- **Entfernen.** Wählen Sie ihn aus und klicken Sie auf **Entfernen**.

**Eine zurücksetzen** stellt den gewählten Befehl wieder her: an seinen üblichen Platz oder außerhalb der Symbolleiste, wenn er dort normalerweise nicht steht. **Alle zurücksetzen** stellt die Symbolleiste wieder her, mit der JeffDeck ausgeliefert wird. Alle zurücksetzen auf dieser Seite ändert Ihr Menüband nicht, und Alle zurücksetzen auf der Seite **Menüband** ändert Ihre Symbolleiste nicht.

Erst wenn Sie auf **OK** klicken, wird etwas geändert. Dann zeigt jedes geöffnete Fenster von JeffDeck Ihre Symbolleiste, ebenso jedes Fenster, das Sie später öffnen. Die Symbolleiste kann insgesamt 100 Befehle und Trennzeichen enthalten. Passen nicht alle Schaltflächen in ein Drittel der Fensterbreite, warten die übrigen hinter einer Schaltfläche **»** am Ende der Symbolleiste, damit der Fenstertitel nie vom Bildschirm gedrängt wird. Ein Befehl bleibt in Ihrer Liste, wenn eine spätere Version ihn nicht mehr hat, und er wird wieder angezeigt, sobald er zurückkehrt. Die Schaltflächen erhalten Zugriffstasten, die Ziffern 1 bis 9 und danach zwei Buchstaben, in der Reihenfolge, in der sie auf der Symbolleiste stehen. Die Symbolleiste befindet sich immer oben links neben der Schaltfläche **Datei**.

## Siehe auch

- [Registerkarte Ansicht](tab-view)
- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Der Bereich Datei](file-screen)
