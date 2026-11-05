# JeffCrunch-Einstellungen ändern
<!-- keywords: Bearbeiten, AutoVervollständigen, Ausfülltool, Ziehen und Ablegen, Dezimalstellen, Optionen, Einstellungen, Voreinstellungen, Nutzungsstatistik, Installationsnummer, Datenschutz, Datenschutzhinweis, JP_NO_TELEMETRY, Statistik löschen, Updates, Info -->

Ihre Einstellungen finden Sie im Fenster **JeffCrunch-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Bearbeiten

Die Seite **Bearbeiten** ändert, wie die Eingabe in Zellen funktioniert.

- **Markierung nach Drücken der Eingabetaste verschieben** legt fest, wohin die Markierung springt, wenn Sie eine Eingabe abschließen: nach unten, nach rechts, nach oben, nach links oder gar nicht.
- **Text aus den Zellen darüber und darunter vervollständigen** heißt AutoVervollständigen. Wenn Sie die ersten Buchstaben eines Textes eingeben und nur ein Text in dieser Spalte mit ihnen beginnt, zeigt JeffCrunch den Rest an, markiert. Drücken Sie **Eingabe**, um ihn zu übernehmen. Geben Sie weiter ein, um ihn zu ignorieren, oder drücken Sie **Rücktaste**, um ihn zu entfernen. Es werden nur die Zellen im Bereich neben der Zelle berücksichtigt, in die Sie gerade eingeben, und nur Texte, keine Zahlen.
- **Ausfülltool verwenden und Zellen zum Verschieben oder Kopieren ziehen** aktiviert das kleine Quadrat an der Ecke der Markierung und das Ziehen am Rand der Markierung. Deaktivieren Sie es, wenn Sie Zellen versehentlich verschieben. Kopieren und Einfügen funktionieren weiterhin.
- **Dezimalstelle bei eingegebenen ganzen Zahlen setzen** heißt automatische Dezimalstellen. Setzen Sie **Stellen** auf 2, wird bei der Eingabe von 1234 der Wert 12,34 eingetragen. Setzen Sie den Wert auf -2, wird bei der Eingabe von 5 der Wert 500 eingetragen. Eine Zahl, die Sie mit Dezimalstelle eingeben, wird so übernommen, wie Sie sie eingegeben haben.

## Nutzungsstatistik

**Anonyme Nutzungsstatistiken senden** ermöglicht es Ihnen, Angaben darüber weiterzugeben, wie JeffCrunch verwendet wird. Anonym heißt, dass die Angaben nichts über Ihre Person verraten. Ist diese Option aktiviert, sendet JeffCrunch diese Angaben einmal am Tag und am Tag, an dem Sie aktualisieren: dass es sich um JeffCrunch handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft das Programm gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben. Wenn JeffCrunch einmal unerwartet beendet wird, sendet es außerdem beim nächsten Start einen Absturzbericht. JeffCrunch sendet niemals Ihre Dateien, deren Namen oder deren Inhalte und auch nichts, was Sie eingegeben haben. Ist diese Option deaktiviert, wird auch kein Absturzbericht gesendet.

Die Entscheidung gilt für alle JeffOffice-Apps: Ändern Sie sie hier, wird sie in jeder von ihnen geändert. JeffCrunch fragt beim ersten Start danach im Fenster **Helfen Sie, JeffCrunch zu verbessern**.

- **Datenschutzhinweis** neben dem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, welche Daten gesendet werden, wohin sie gehen und wie lange wir sie aufbewahren. Wir speichern Statistiken 24 Monate lang.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Es ist eine zufällige Nummer, die für Ihre Kopie von JeffOffice erzeugt wurde und in jeder JeffOffice-App gleich ist. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind Statistiken deaktiviert oder wurde noch keine Nummer erzeugt, steht dort **keine**.
- Wenn die bereits bei uns vorliegenden Statistiken gelöscht werden sollen, senden Sie Ihre Installationsnummer **bevor** Sie die Statistiken deaktivieren an info@jeffoffice.com. Beim Deaktivieren wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihnen gehören.
- Um die Statistiken zu deaktivieren, ohne JeffCrunch zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffCrunch stellt die Frage beim ersten Start nicht.

## Siehe auch

- [Willkommen bei JeffCrunch](start)
