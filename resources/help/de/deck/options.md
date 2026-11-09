# Einstellungen von JeffDeck ändern
<!-- keywords: Optionen, Einstellungen, Voreinstellungen, Design, Dunkelmodus, Sprache, Benutzername, Initialen, Lineal, einrasten, Rechtschreibung, Autokorrektur, Anführungszeichen, Updates, Beta, Nutzungsstatistik, Info, speichern, Schriftarten einbetten, Schriftarten, Dokumentprüfung -->

Ihre Einstellungen finden Sie im Dialogfeld **JeffDeck-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Es hat vier Registerkarten. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Allgemein

- **Benutzername** und **Initialen** werden bei den Kommentaren angezeigt, die Sie schreiben. Siehe [Kommentare](comments).
- **Design** ist **Systemeinstellung verwenden**, **Hell** oder **Dunkel**. Die Änderung wird sofort angezeigt.
- **Anzeigesprache** ist die Sprache der Oberfläche von JeffDeck. **Systemsprache verwenden** richtet sich nach Ihrem Computer. Weitere Sprachen erscheinen in dieser Liste, wenn Übersetzungen verfügbar sind. Eine neue Sprache wird beim nächsten Start von JeffDeck wirksam.
- **Beim Start von JeffDeck nach Updates suchen** lässt JeffDeck bei jedem Start prüfen, ob eine neuere Version verfügbar ist. Wenn ja, zeigt ein Fenster mit dem Titel **Update verfügbar**, was neu ist. Wählen Sie **Jetzt aktualisieren**, **Später** oder **Diese Version überspringen**. Unter Windows lädt **Jetzt aktualisieren** das Installationsprogramm herunter. Es installiert die neue Version aller JeffOffice-Apps und fordert Sie auf, geöffnete Apps zu schließen. Auf anderen Computern lautet die Schaltfläche **Downloadseite öffnen**. Sie öffnet die Seite, auf der Sie die neue Version erhalten. Ist auch JeffPub geöffnet, informiert Sie nur eines der beiden Programme pro Tag über eine neue Version.
- **Betaversionen erhalten (frühe Builds zum Testen)** sorgt dafür, dass JeffDeck auch Betaversionen anbietet. Sie erscheinen vor den regulären Versionen und können noch Macken haben.
- **Anonyme Nutzungsstatistiken senden** erlaubt es, Angaben darüber weiterzugeben, wie JeffDeck verwendet wird. Anonym heißt, dass die Angaben nichts über Ihre Person verraten. Ist dies aktiviert, sendet JeffDeck diese Angaben einmal am Tag und am Tag einer Aktualisierung: dass es sich um JeffDeck handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft das Programm gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben (zum Beispiel .pptx). Schließt sich JeffDeck unerwartet, sendet es beim nächsten Start außerdem einen Absturzbericht. Der Bericht enthält die Stelle im Programm, an der es abbrach, damit das Problem gefunden und behoben werden kann. JeffDeck sendet niemals Ihre Dateien, deren Namen oder deren Inhalt und nichts, was Sie eingegeben haben. Ist dies deaktiviert, wird auch kein Absturzbericht gesendet. JeffDeck fragt beim ersten Start in einem Dialogfeld mit dem Titel **Helfen Sie, JeffDeck zu verbessern** danach. Die Frage entfällt, wenn Sie bereits in JeffPub oder im Setup entschieden haben.
- **Datenschutzhinweis** neben diesem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Wir bewahren Statistiken 24 Monate lang auf.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Sie ist eine zufällige Nummer für Ihre Kopie von JeffOffice und in allen JeffOffice-Apps gleich. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind die Statistiken deaktiviert oder wurde noch keine Nummer erzeugt, steht dort **keine**.
- Wenn Sie die bereits bei uns vorhandenen Statistiken löschen lassen möchten, senden Sie Ihre Installationsnummer **vor** dem Deaktivieren der Statistiken an info@jeffoffice.com. Beim Deaktivieren wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihnen gehören.
- Um die Statistiken zu deaktivieren, ohne JeffDeck zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffDeck stellt die Frage beim ersten Start nicht.

**Benutzername**, **Initialen**, **Design** und **Anzeigesprache** sowie die Entscheidungen zu Updates und Statistiken gelten für alle JeffOffice-Apps: Wenn Sie etwas hier ändern, wird es in jeder dieser Apps geändert.

## Dokumentprüfung

Diese Kontrollkästchen steuern die Rechtschreibung und die Eingabehilfen. Siehe [Text eingeben und formatieren](text).

- **Rechtschreibung während der Eingabe prüfen**
- **Wörter in GROSSBUCHSTABEN ignorieren**
- **Wörter mit Zahlen ignorieren**
- **Text während der Eingabe ersetzen (AutoKorrektur)** korrigiert typische Tippfehler.
- **Gerade Anführungszeichen durch typografische ersetzen** wandelt " und ' in typografische Anführungszeichen um.
- **Während der Eingabe automatisch formatieren (Gedankenstriche, Brüche, Ordnungszahlen, automatische Listen)**
- **Webadressen beim Eingeben automatisch in Links umwandeln** macht eine Web- oder E-Mail-Adresse zu einem Link, wenn Sie danach die Leertaste drücken. Mit Rückgängig wird nur der Link entfernt.

## Ansicht

- **Lineal anzeigen** blendet die Lineale am Rand der Folie ein. Das entspricht **Ansicht > Anzeigen > Lineal**, und es gilt für jedes Fenster, jetzt und für neu geöffnete Fenster.
- **An anderen Objekten ausrichten** sorgt dafür, dass sich ein Objekt, das Sie ziehen, an den anderen Objekten auf der Folie ausrichtet.

## Speichern

- **Schriftarten in der Datei einbetten** speichert beim Speichern die Schriftarten, die Ihre Folien verwenden, in der Datei. So sehen die Folien auf einem Computer ohne diese Schriftarten gleich aus. Die Option ist aktiviert, bis Sie sie deaktivieren. Es werden nur die Schriftarten eingebettet, die mit JeffDeck mitgeliefert werden, und die Datei wird dadurch größer. Details finden Sie im Thema Präsentationen öffnen und speichern unter „Schriftarten in der Datei“.

## Siehe auch

- [Registerkarte Ansicht](tab-view)
- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Die Backstage-Ansicht „Datei“](file-screen)
