# Einstellungen von JeffCrunch ändern
<!-- keywords: Bearbeiten, AutoVervollständigen, Ausfüllkästchen, Ziehen und Ablegen, Dezimalstellen, Optionen, Einstellungen, Voreinstellungen, Nutzungsstatistik, Installationsnummer, Datenschutz, Datenschutzhinweis, JP_NO_TELEMETRY, Statistik löschen, Updates, Info, direkt in Zellen bearbeiten, Gitternetzlinienfarbe, Standardzoom, Standardbreite, Datenschutz, persönliche Informationen -->

Ihre Einstellungen finden Sie im Fenster **JeffCrunch-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Bearbeiten

Auf der Seite **Bearbeiten** legen Sie fest, wie die Eingabe in Zellen funktioniert.

- **Markierung nach Drücken der Eingabetaste verschieben** legt fest, wohin die Markierung wandert, wenn Sie eine Eingabe abschließen: nach unten, nach rechts, nach oben, nach links oder gar nicht.
- **Text aus den Zellen darüber und darunter vervollständigen** heißt AutoVervollständigen. Wenn Sie die ersten Buchstaben eines Textes eingeben und nur ein Text in dieser Spalte mit ihnen beginnt, zeigt JeffCrunch den Rest markiert an. Drücken Sie **Eingabe**, um ihn zu übernehmen. Tippen Sie weiter, um ihn zu ignorieren, oder drücken Sie **Rücktaste**, um ihn zu entfernen. Es werden nur der Zellbereich neben der Zelle, in die Sie eingeben, und nur Texte berücksichtigt, keine Zahlen.
- **Ausfüllkästchen verwenden und Zellen zum Verschieben oder Kopieren ziehen** blendet das kleine Quadrat an der Ecke der Markierung und das Ziehen am Rand der Markierung ein. Schalten Sie es aus, wenn Sie Zellen versehentlich verschieben. Kopieren und Einfügen funktionieren weiterhin.
- **Null in Zellen mit dem Wert null anzeigen (dieses Blatt)** blendet Nullen für das gerade angezeigte Blatt ein oder aus. Ist die Option aus, wirkt eine Zelle mit 0 leer, enthält aber weiterhin die 0 und wird als 0 addiert. Die Einstellung wird mit dem Blatt gespeichert.
- **Dezimalpunkt bei eingegebenen ganzen Zahlen setzen** heißt automatische Dezimalstellen. Setzen Sie **Stellen** auf 2, wird bei der Eingabe von 1234 der Wert 12,34 eingetragen. Setzen Sie den Wert auf -2, wird bei der Eingabe von 5 der Wert 500 eingetragen. Eine Zahl, die Sie mit Dezimalzeichen eingeben, wird so übernommen, wie Sie sie eingegeben haben.

- **Direktes Bearbeiten in Zellen zulassen** ist standardmäßig aktiviert. Wenn Sie es ausschalten, wird eine Eingabe oder F2 in die Bearbeitungsleiste geleitet, und die Zelle zeigt das Ergebnis erst an, wenn Sie die Eingabe abschließen.

## Anzeigeoptionen

- **Gitternetzlinienfarbe** ist die Farbe der Linien zwischen den Zellen auf Ihrem Bildschirm. Diese Einstellung gilt nur für Ihren Bildschirm und wird nicht in der Arbeitsmappe gespeichert.
- **Zoom einer neuen Arbeitsmappe** legt fest, wie groß die Blätter einer neuen Arbeitsmappe angezeigt werden, von 10 % bis 400 %.
- **Standardspaltenbreite (dieses Blatt)** ist die Breite jeder Spalte dieses Blatts, die keine eigene Breite hat. Sie entspricht der Option **Standardbreite** auf der Registerkarte Start.

## Datenschutzoptionen

- **Name und Firma beim Speichern aus den Eigenschaften einer Arbeitsmappe entfernen** löscht beim Speichern den Autor, den Bearbeiter der letzten Speicherung, die Firma und den Manager aus den Eigenschaften der Arbeitsmappe. Die Eigenschaften sind die Angaben, die eine Datei über sich selbst speichert.

## Nutzungsstatistik

**Anonyme Nutzungsstatistiken senden** ermöglicht es Ihnen, Angaben zur Nutzung von JeffCrunch zu teilen. Anonym bedeutet, dass die Angaben nichts über Sie als Person verraten. Ist die Option eingeschaltet, sendet JeffCrunch diese Angaben einmal am Tag sowie am Tag einer Aktualisierung: dass es sich um JeffCrunch handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft das Programm gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben. Wenn JeffCrunch einmal unerwartet beendet wird, sendet es außerdem beim nächsten Start einen Absturzbericht. JeffCrunch sendet niemals Ihre Dateien, deren Namen oder deren Inhalt und nichts, was Sie eingegeben haben. Ist die Option ausgeschaltet, wird auch kein Absturzbericht gesendet.

Die Auswahl gilt für alle JeffOffice-Apps: Ändern Sie sie hier, wird sie in allen übernommen. JeffCrunch fragt beim ersten Start danach, im Fenster **Helfen Sie, JeffCrunch zu verbessern**.

- **Datenschutzhinweis** neben dem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, welche Daten gesendet werden, wohin sie gehen und wie lange wir sie aufbewahren. Wir speichern Statistiken 24 Monate lang.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Sie ist eine zufällige Nummer für Ihre Kopie von JeffOffice und in allen JeffOffice-Apps gleich. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie in die Zwischenablage zu kopieren. Ist die Statistik ausgeschaltet oder wurde noch keine Nummer erstellt, steht dort **keine**.
- Wenn die bereits bei uns gespeicherten Statistiken gelöscht werden sollen, senden Sie Ihre Installationsnummer **bevor** Sie die Statistik ausschalten an info@jeffoffice.com. Beim Ausschalten wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihnen gehören.
- Um die Statistik auszuschalten, ohne JeffCrunch zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffCrunch stellt die Frage beim ersten Start nicht.

## Siehe auch

- [Willkommen bei JeffCrunch](start)
