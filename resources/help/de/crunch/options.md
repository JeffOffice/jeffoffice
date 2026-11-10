# Einstellungen von JeffCrunch ändern

<!-- keywords: Wiederherstellung, nicht gespeicherte Arbeit wiederherstellen, automatische Wiederherstellung, Dokumentprüfung, Bearbeiten, AutoVervollständigen, Ausfüllkästchen, Ziehen und Ablegen, Dezimalstellen, Optionen, Einstellungen, Voreinstellungen, Nutzungsstatistiken, Installationsnummer, Datenschutz, Datenschutzhinweis, JP_NO_TELEMETRY, Statistiken löschen, Updates, Info, direkt in Zellen bearbeiten, Gitternetzlinienfarbe, Standardzoom, Standardbreite, Datenschutz, persönliche Informationen -->

Ihre Einstellungen finden Sie im Fenster **JeffCrunch-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Bearbeiten

Die Seite **Bearbeiten** ändert, wie das Eingeben in Zellen funktioniert.

- **Markierung nach Drücken der Eingabetaste verschieben** legt fest, wohin die Markierung wandert, wenn Sie eine Eingabe abschließen: nach unten, nach rechts, nach oben, nach links oder gar nicht.
- **Text aus den Zellen darüber und darunter vervollständigen** heißt AutoVervollständigen. Wenn Sie die ersten Buchstaben eines Textes eingeben und in dieser Spalte nur ein Text mit diesen Buchstaben beginnt, zeigt JeffCrunch den Rest an, markiert. Drücken Sie **Eingabe**, um ihn zu übernehmen. Tippen Sie weiter, um ihn zu ignorieren, oder drücken Sie **Rücktaste**, um ihn zu entfernen. Es werden nur die Zellen im Block neben der Zelle berücksichtigt, in die Sie gerade schreiben, und nur Texte, keine Zahlen.
- **Ausfüllkästchen verwenden und Zellen durch Ziehen verschieben oder kopieren** schaltet das kleine Quadrat an der Ecke der Markierung und das Ziehen am Rahmen der Markierung ein. Schalten Sie es aus, wenn Sie Zellen versehentlich verschieben. Kopieren und Einfügen funktionieren weiterhin.
- **Eine Null in Zellen mit dem Wert null anzeigen (dieses Blatt)** schaltet Nullen für das gerade angezeigte Blatt ein oder aus. Ist die Option aus, sieht eine Zelle mit 0 leer aus, enthält aber weiterhin die 0 und wird als 0 addiert. Die Einstellung wird mit dem Blatt gespeichert.
- **Dezimalpunkt bei eingegebenen ganzen Zahlen setzen** heißt automatische Dezimalstellen. Setzen Sie **Stellen** auf 2, wird bei der Eingabe von 1234 der Wert 12,34 übernommen. Setzen Sie den Wert auf -2, wird bei der Eingabe von 5 der Wert 500 übernommen. Eine Zahl, die Sie mit Dezimalzeichen eingeben, wird so übernommen, wie Sie sie eingegeben haben.
- **Direktes Bearbeiten in Zellen zulassen** ist standardmäßig eingeschaltet. Wenn Sie es ausschalten, gehen eine eingegebene Eingabe oder F2 in die Bearbeitungsleiste, und die Zelle zeigt das Ergebnis erst an, wenn Sie die Eingabe abschließen.

## Speicheroptionen

**Informationen zur automatischen Wiederherstellung speichern alle** legt fest, wie oft JeffCrunch eine Kopie einer Arbeitsmappe mit nicht gespeicherten Änderungen anlegt, und zwar im Bereich von 1 bis 120 Minuten. Die Kopie ist eine .xlsx-Datei in einem Ordner, den JeffCrunch dafür verwaltet. Sie wird im Hintergrund erstellt, sodass Sie weiterarbeiten können. Beim Speichern oder Schließen der Arbeitsmappe wird sie entfernt. Wenn JeffCrunch unerwartet beendet wird, bietet es beim nächsten Start diese Kopien an. Außerdem können Sie auf **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** klicken. Eine mit einem Kennwort geschützte Arbeitsmappe wird nicht kopiert, weil die Kopie nicht geschützt wäre.

## Optionen für die Dokumentprüfung

Die Seite **Dokumentprüfung** wird von den JeffOffice-Apps gemeinsam genutzt. Hier wird festgelegt, ob die Rechtschreibung während der Eingabe in einer Zelle geprüft wird. Schalten Sie **Rechtschreibung während der Eingabe prüfen** aus, um die roten Wellenlinien in allen geöffneten Fenstern und den anderen Apps zu entfernen; **Rechtschreibung** auf der Registerkarte Überprüfen funktioniert weiterhin. Außerdem legen Sie hier fest, ob Wörter in Großbuchstaben und Wörter mit Zahlen bei der Rechtschreibprüfung ignoriert werden, ob die AutoKorrektur Wörter während der Eingabe korrigiert, ob gerade Anführungszeichen in typografische umgewandelt werden, ob Striche, Brüche und Ordinalzahlen geändert werden und ob eine Webadresse zu einem Link wird. Siehe [Rechtschreibung](notes-links) und [Text in Zellen eingeben](edit-cells).

## Anzeigeoptionen

- **Gitternetzlinienfarbe** ist die Farbe der Linien zwischen den Zellen auf Ihrem Bildschirm. Diese Auswahl gilt nur für Ihren Bildschirm und wird nicht in der Arbeitsmappe gespeichert.
- **Zoom einer neuen Arbeitsmappe** legt fest, wie groß die Blätter einer neuen Arbeitsmappe angezeigt werden, von 10 % bis 400 %.
- **Standardspaltenbreite (dieses Blatt)** ist die Breite aller Spalten dieses Blatts, die keine eigene Breite haben. Sie entspricht **Standardbreite** auf der Registerkarte Start.

## Datenschutzoptionen

- **Name und Firma beim Speichern aus den Dokumenteigenschaften einer Arbeitsmappe entfernen** löscht beim Speichern den Autor, den letzten Bearbeiter, die Firma und den Manager aus den Eigenschaften der Arbeitsmappe. Die Eigenschaften sind die Angaben, die eine Datei über sich selbst speichert.

## Nutzungsstatistiken

**Anonyme Nutzungsstatistiken senden** ermöglicht es Ihnen, Angaben darüber weiterzugeben, wie JeffCrunch verwendet wird. Anonym bedeutet, dass die Angaben nichts darüber aussagen, wer Sie sind. Ist diese Option eingeschaltet, sendet JeffCrunch diese Angaben einmal am Tag und am Tag einer Aktualisierung: dass es sich um JeffCrunch handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft es gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben. Wenn JeffCrunch einmal unerwartet beendet wird, sendet es beim nächsten Start außerdem einen Absturzbericht. JeffCrunch sendet niemals Ihre Dateien, deren Namen oder deren Inhalt und auch nichts, was Sie eingegeben haben. Ist diese Option aus, wird auch kein Absturzbericht gesendet.

Die Entscheidung gilt für alle JeffOffice-Apps: Wenn Sie sie hier ändern, ändert sie sich in jeder von ihnen. JeffCrunch fragt beim ersten Start danach, im Fenster **JeffCrunch verbessern helfen**.

- **Datenschutzhinweis** neben dem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Statistiken bewahren wir 24 Monate auf.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Es ist eine zufällig erzeugte Nummer für Ihre Kopie von JeffOffice, in allen JeffOffice-Apps dieselbe. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind die Statistiken aus oder wurde noch keine Nummer erzeugt, steht dort **keine**.
- Wenn die bei uns vorhandenen Statistiken gelöscht werden sollen, senden Sie Ihre Installationsnummer an info@jeffoffice.com, **bevor** Sie die Statistiken ausschalten. Das Ausschalten löscht die Nummer von Ihrem Computer, und ohne sie können wir nicht feststellen, welche Datensätze zu Ihnen gehören.
- Um die Statistiken auszuschalten, ohne JeffCrunch zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffCrunch stellt die Frage beim ersten Start nicht.

## Siehe auch

- [Willkommen bei JeffCrunch](start)
