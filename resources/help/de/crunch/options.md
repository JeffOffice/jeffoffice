# Einstellungen von JeffCrunch ändern
<!-- keywords: Formeloptionen, Berechnungseinstellungen, maximale Änderung, Iteration, Wiederherstellung, nicht gespeicherte Arbeit wiederherstellen, automatische Wiederherstellung, Dokumentprüfung, Bearbeiten, AutoVervollständigen, Ausfüllkästchen, Ziehen und Ablegen, Nachkommastellen, Optionen, Einstellungen, Voreinstellungen, Nutzungsstatistiken, Installationsnummer, Datenschutz, Datenschutzhinweis, JP_NO_TELEMETRY, Statistiken löschen, Updates, Info, direktes Bearbeiten in Zellen, Gitternetzlinienfarbe, Standardzoom, Standardbreite, persönliche Informationen, Tastatur anpassen, Menüband anpassen, Schnellsymbolleiste anpassen, Schnellsymbolleiste, Befehl zur Schnellsymbolleiste hinzufügen, Registerkarte ausblenden, Gruppe ausblenden, Registerkarte hinzufügen, Gruppe hinzufügen, Registerkarte umbenennen, Menüband zurücksetzen, Tastenkombinationen ändern, Tasten neu zuweisen, Tasten zurücksetzen -->

Ihre Einstellungen finden Sie im Fenster **JeffCrunch-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Allgemein

**Minisymbolleiste bei Auswahl anzeigen** ist standardmäßig aktiviert. Die Option legt fest, ob die kleine Formatierungsleiste neben dem markierten Text und oben im Kontextmenü von Zellen erscheint (siehe [Text in Zellen eingeben](edit-cells)). Die Einstellung gilt für alle JeffOffice-Apps gemeinsam.

## Bearbeiten

Die Seite **Bearbeiten** ändert, wie die Eingabe in Zellen funktioniert.

- **Nach Drücken der Eingabetaste Markierung verschieben** legt fest, wohin die Markierung nach dem Abschluss einer Eingabe springt: nach unten, nach rechts, nach oben, nach links oder gar nicht.
- **Text aus den Zellen darüber und darunter automatisch vervollständigen** heißt AutoVervollständigen. Wenn Sie die ersten Buchstaben eines Textes eingeben und nur ein Text in dieser Spalte mit ihnen beginnt, zeigt JeffCrunch den Rest markiert an. Drücken Sie **Eingabe**, um ihn zu übernehmen. Wenn Sie weiterschreiben, wird er verworfen, mit **Rück**taste entfernen Sie ihn. Berücksichtigt werden nur der Zellbereich neben der Zelle, in die Sie schreiben, und nur Texte, keine Zahlen.
- **Ausfüllkästchen verwenden und Zellen zum Verschieben oder Kopieren ziehen** schaltet das kleine Quadrat an der Ecke der Markierung und das Ziehen am Markierungsrand ein. Schalten Sie es aus, wenn Sie Zellen versehentlich verschieben. Kopieren und Einfügen funktionieren weiterhin.
- **Eine Null in Zellen mit dem Wert null anzeigen (dieses Blatt)** blendet Nullen für das gerade angezeigte Blatt ein oder aus. Ist die Option ausgeschaltet, sieht eine Zelle mit 0 leer aus, enthält aber weiterhin die 0 und wird als 0 addiert. Die Einstellung wird mit dem Blatt gespeichert.
- **Dezimalstellen bei eingegebenen ganzen Zahlen automatisch setzen** heißt automatische Nachkommastellen. Stellen Sie **Stellen** auf 2 ein, dann ergibt die Eingabe 1234 den Wert 12,34. Stellen Sie den Wert auf -2 ein, dann ergibt die Eingabe 5 den Wert 500. Eine Zahl, die Sie mit Dezimalkomma eingeben, wird so übernommen, wie Sie sie schreiben.
- **Direktes Bearbeiten in Zellen zulassen** ist standardmäßig aktiviert. Wenn Sie die Option ausschalten, wird eine Eingabe oder **F2** an die Bearbeitungsleiste weitergeleitet, und die Zelle zeigt das Ergebnis erst nach dem Abschluss der Eingabe an.

## Formeloptionen

Die Seite **Formeln** legt fest, wie die geöffnete Arbeitsmappe ihre Formeln berechnet: **Berechnung** (automatisch, automatisch außer Datentabellen oder manuell), ob die Formeln bei manueller Berechnung vor dem Speichern der Arbeitsmappe berechnet werden, ob eine Formel, die auf sich selbst verweist, durch wiederholtes Durchlaufen berechnet wird, die maximale Anzahl der Durchläufe und die Änderung, bei der die Durchläufe abbrechen. Diese Einstellungen gehören zur Arbeitsmappe, nicht zu Ihnen: Sie werden in ihrer Datei gespeichert und nicht für andere Arbeitsmappen übernommen. Ist keine Arbeitsmappe geöffnet, sind die Optionen ausgegraut. Die Änderung wird mit allen Stellen angezeigt, Sie können sie aber auch als Zahl eingeben, etwa 0,0000001 oder 1e-7. Nur eine geänderte Einstellung wird gespeichert: **OK** ohne Änderungen erzeugt keinen Schritt für **Rückgängig** und lässt die Arbeitsmappe unverändert. Eine Änderung ist ein Schritt von **Rückgängig**. Das neuere Arbeitsmappenformat und die ältere .xls-Datei speichern alle Einstellungen. Eine .ods-Kopie behält die Einstellungen für die iterative Berechnung, hat aber keinen Platz für die manuelle Berechnung. Daher berechnet die Kopie die Formeln laufend, und der Bericht vor dem Speichern der Kopie weist darauf hin. Dieselben Einstellungen finden Sie auch auf der Registerkarte **Formeln**.

## Speicheroptionen

**Informationen zur automatischen Wiederherstellung speichern alle** legt fest, wie oft JeffCrunch eine Kopie einer Arbeitsmappe mit nicht gespeicherten Änderungen anlegt, und zwar von 1 bis 120 Minuten. Die Kopie ist eine .xlsx-Datei in einem Ordner, den JeffCrunch dafür verwaltet. Sie wird im Hintergrund erstellt, sodass Sie weiterschreiben können. Sie wird gelöscht, wenn Sie die Arbeitsmappe speichern oder schließen. Wenn JeffCrunch unerwartet beendet wird, bietet es die Kopien beim nächsten Start an. Sie können auch **Datei > Öffnen > Nicht gespeicherte Arbeit wiederherstellen** wählen. Eine Arbeitsmappe, die mit einem Kennwort geschützt ist, wird nicht kopiert, weil die Kopie nicht geschützt wäre.

## Dokumentprüfungsoptionen

Die Seite **Dokumentprüfung** wird von den JeffOffice-Apps gemeinsam genutzt. Sie legt fest, ob die Rechtschreibung während der Eingabe in einer Zelle geprüft wird (schalten Sie **Rechtschreibung während der Eingabe prüfen** aus, um die roten Wellenlinien in allen geöffneten Fenstern und den anderen Apps zu entfernen; **Rechtschreibung** auf der Registerkarte **Überprüfen** funktioniert trotzdem weiter), ob Wörter in Großbuchstaben und Wörter mit Zahlen bei der Rechtschreibprüfung ignoriert werden, ob die AutoKorrektur Wörter während der Eingabe korrigiert, ob gerade Anführungszeichen in typografische umgewandelt werden, ob Striche, Brüche und Ordnungszahlen geändert werden und ob eine Webadresse zu einem Link wird. Siehe [Rechtschreibung](notes-links) und [Text in Zellen eingeben](edit-cells).

## Anzeigeoptionen

- **Gitternetzlinienfarbe** ist die Farbe der Linien zwischen den Zellen auf Ihrem Bildschirm. Die Einstellung gilt nur für Ihren Bildschirm und wird nicht in der Arbeitsmappe gespeichert.
- **Zoom einer neuen Arbeitsmappe** legt fest, wie groß die Blätter einer neuen Arbeitsmappe angezeigt werden, von 10 % bis 400 %.
- **Standardspaltenbreite (dieses Blatt)** ist die Breite jeder Spalte dieses Blatts, die keine eigene Breite hat. Es ist dieselbe Einstellung wie **Standardbreite** auf der Registerkarte **Start**.

## Anpassen

Die Registerkarte **Anpassen** hat drei Seiten: **Menüband**, **Schnellsymbolleiste** und **Tastatur**. Auf der Seite **Tastatur** wählen Sie die Tastenkombinationen für die Befehle von JeffCrunch. Siehe „Eigene Tastenkombinationen wählen“ im Thema zur Tastatur.

### Menüband anpassen

Die Seite **Menüband** zeigt die Registerkarten des Menübands mit ihren Gruppen.

- **Registerkarte oder Gruppe ausblenden oder anzeigen.** Entfernen Sie das Häkchen neben einer Registerkarte oder Gruppe, um sie auszublenden, und setzen Sie es wieder, um sie anzuzeigen. Eine ausgeblendete Gruppe braucht keinen Platz, ihre Befehle funktionieren aber weiterhin über ihre Tastenkombinationen. Die Registerkarte **Start** hat kein Kästchen, weil sie immer angezeigt bleibt, ebenso die Schaltfläche **Datei**. Eine Registerkarte, die nur erscheint, wenn etwas ausgewählt ist, etwa ein Bild oder eine Tabelle, steht am Ende der Liste und hat kein Kästchen.
- **Eigene Registerkarte oder Gruppe erstellen.** Geben Sie einen Namen im Feld **Name** ein und klicken Sie auf **Neue Registerkarte**. Um eine Gruppe zu erstellen, wählen Sie eine Registerkarte und klicken Sie auf **Neue Gruppe**. Eine eigene Gruppe kann auf jeder Registerkarte stehen, auch auf einer Registerkarte, die nur bei Auswahl erscheint.
- **Befehle in Ihre Gruppe aufnehmen.** Wählen Sie Ihre Gruppe, suchen Sie in der Liste **Befehle** einen Befehl (geben Sie im Feld **Befehle suchen** einen Begriff ein, um die Liste einzugrenzen) und klicken Sie auf **Befehl hinzufügen**. Jeder Befehl mit einem Namen kann in eine Gruppe aufgenommen werden. Ein Befehl kann nur einmal in einer Gruppe stehen.
- **Umbenennen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl, geben Sie den neuen Namen ein und klicken Sie auf **Umbenennen**. Ein umbenannter Befehl macht weiterhin dasselbe, nur seine Schaltfläche zeigt den neuen Namen. Um einem Befehl seinen ursprünglichen Namen zurückzugeben, benennen Sie ihn mit einem leeren Namen um.
- **Reihenfolge ändern.** Wählen Sie eine Registerkarte und klicken Sie auf **Nach oben** oder **Nach unten**. Sie können die Registerkarten des Menübands und Ihre eigenen Registerkarten verschieben, nicht aber die Registerkarten, die nur bei Auswahl erscheinen. Ihre Gruppen und Befehle lassen sich innerhalb ihrer Registerkarte oder Gruppe auf dieselbe Weise verschieben. Die Gruppen des Menübands behalten ihre Reihenfolge.
- **Entfernen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl und klicken Sie auf **Entfernen**.

**Einzeln zurücksetzen** stellt die gewählte Registerkarte wieder her: Sie wird angezeigt, ihre Gruppen werden angezeigt, und die Gruppen, die Sie ihr hinzugefügt haben, verschwinden. Eine eigene Registerkarte wird entfernt. **Alle zurücksetzen** stellt das gesamte Menüband wieder her, einschließlich der Reihenfolge der Registerkarten.

Erst mit **OK** wird etwas geändert. Danach zeigt jedes geöffnete JeffCrunch-Fenster Ihr Menüband, ebenso jedes später geöffnete Fenster. JeffCrunch speichert nur, was sich vom üblichen Menüband unterscheidet. Eine Registerkarte oder Gruppe, die eine spätere Version ergänzt, erscheint daher von selbst. Ihre Registerkarten und Gruppen erhalten Zugriffstasten (die Buchstaben, die erscheinen, wenn Sie **Alt** drücken), die sich nicht mit den anderen überschneiden. Sie können 20 eigene Registerkarten haben, auf einer Registerkarte 30 eigene Gruppen und in einer Gruppe 60 Befehle.

### Schnellsymbolleiste anpassen

Die Seite **Schnellsymbolleiste** betrifft die kleine Symbolleiste oben links im Fenster. Die linke Liste zeigt, was sich jetzt in der Symbolleiste befindet. Die Liste **Befehle** rechts enthält alle Befehle mit einem Namen (geben Sie im Feld **Befehle suchen** einen Begriff ein, um sie einzugrenzen, wie auf der Seite **Menüband**).

- **Befehl hinzufügen.** Wählen Sie ihn in der Liste **Befehle** und klicken Sie auf **Befehl hinzufügen**. Er wird ans Ende der Symbolleiste gesetzt. Ein Befehl, der ein Menü öffnet, behält sein Menü auf der Symbolleiste. Ein Befehl kann nur einmal in der Symbolleiste stehen.
- **Rechtsklick.** Sie können auch auf einen beliebigen Befehl im Menüband rechtsklicken und **Zur Schnellsymbolleiste hinzufügen** wählen oder auf eine Schaltfläche der Symbolleiste rechtsklicken und **Aus Schnellsymbolleiste entfernen** wählen. Im selben Menü stehen **Schnellsymbolleiste anpassen** und **Menüband anpassen**, die diese Seite bzw. die Seite **Menüband** öffnen.
- **Unter dem Menüband anzeigen.** Aktivieren Sie **Schnellsymbolleiste unter dem Menüband anzeigen**, damit die Symbolleiste in einer Zeile unter dem Menüband statt oben links steht. Auch die beiden Rechtsklick-Menüs enthalten **Schnellsymbolleiste unter dem Menüband anzeigen**. JeffCrunch merkt sich die Einstellung.
- **Trennlinie hinzufügen.** Klicken Sie auf **Trennlinie hinzufügen**, um eine dünne Linie am Ende einzufügen. Verschieben Sie sie anschließend an die gewünschte Stelle.
- **Reihenfolge ändern.** Wählen Sie einen Befehl oder eine Trennlinie in der linken Liste und klicken Sie auf **Nach oben** oder **Nach unten**.
- **Entfernen.** Wählen Sie den Eintrag und klicken Sie auf **Entfernen**.

**Einzeln zurücksetzen** stellt den gewählten Befehl wieder her: an seinem üblichen Platz oder außerhalb der Symbolleiste, wenn JeffCrunch ihn dort normalerweise nicht hat. **Alle zurücksetzen** stellt die Symbolleiste wieder her, die JeffCrunch mitbringt. Alle zurücksetzen auf dieser Seite ändert Ihr Menüband nicht, und Alle zurücksetzen auf der Seite **Menüband** ändert Ihre Symbolleiste nicht.

Erst mit **OK** wird etwas geändert. Danach zeigt jedes geöffnete JeffCrunch-Fenster Ihre Symbolleiste, ebenso jedes später geöffnete Fenster. Die Symbolleiste kann insgesamt 100 Befehle und Trennlinien enthalten. Passen nicht alle Schaltflächen in ein Drittel der Fensterbreite, warten die übrigen hinter einer Schaltfläche **»** am Ende der Symbolleiste. So wird der Fenstertitel nie vom Bildschirm gedrängt. JeffCrunch behält einen Befehl in Ihrer Liste, wenn eine spätere Version ihn nicht mehr hat, und zeigt ihn wieder an, wenn er zurückkommt. Die Schaltflächen erhalten Zugriffstasten: die Ziffern 1 bis 9 und danach zwei Buchstaben, in der Reihenfolge der Symbolleiste. Die Symbolleiste steht immer oben links neben der Schaltfläche **Datei**.

## Datenschutzoptionen

- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** entfernt beim Schreiben der Datei (einer Arbeitsmappe, einer Kopie und einer per E-Mail versendeten Arbeitsmappe) den Autor, die Person, die die Datei zuletzt gespeichert hat, die Firma und den Manager sowie die Namen und Zeitangaben bei Notizen und Kommentaren. Der Inhalt der Datei bleibt unverändert. Die benutzerdefinierten Eigenschaften der Datei bleiben, wie sie sind, und das gerade geöffnete Dokument wird nicht verändert. Die Option ist in allen JeffOffice-Apps dieselbe: Ändern Sie sie in einer App, ändert sie sich in allen. Um es nur einmal zu tun, verwenden Sie **Arbeitsmappe prüfen** unter [Informationen](info). Dasselbe Kontrollkästchen finden Sie auch auf der Seite **Informationen**.

## Nutzungsstatistiken

**Anonyme Nutzungsstatistiken senden** ermöglicht es Ihnen, Angaben darüber zu teilen, wie JeffCrunch verwendet wird. Anonym heißt, dass die Angaben nichts darüber verraten, wer Sie sind. Ist die Option aktiviert, sendet JeffCrunch diese Angaben einmal täglich und am Tag einer Aktualisierung: dass es sich um JeffCrunch handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft JeffCrunch gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben. Wenn JeffCrunch einmal unerwartet beendet wird, sendet es außerdem beim nächsten Start einen Absturzbericht. JeffCrunch sendet niemals Ihre Dateien, deren Namen oder deren Inhalt und nichts, was Sie eingetippt haben. Ist die Option ausgeschaltet, wird auch kein Absturzbericht gesendet.

Die Einstellung gilt für alle JeffOffice-Apps: Ändern Sie sie hier, ändert sie sich in jeder von ihnen. JeffCrunch fragt beim ersten Start im Fenster **JeffCrunch bei der Verbesserung helfen** danach.

- **Datenschutzhinweis** neben dem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Statistiken bewahren wir 24 Monate lang auf.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Sie ist eine zufällige Nummer für Ihre Kopie von JeffOffice und in allen JeffOffice-Apps dieselbe. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind die Statistiken ausgeschaltet oder wurde noch keine Nummer erzeugt, steht dort **keine**.
- Um die Statistiken, die wir bereits gespeichert haben, löschen zu lassen, senden Sie Ihre Installationsnummer **bevor** Sie die Statistiken ausschalten an info@jeffoffice.com. Beim Ausschalten wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht erkennen, welche Datensätze Ihnen gehören.
- Um die Statistiken auszuschalten, ohne JeffCrunch zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffCrunch stellt die Frage beim ersten Start nicht.

## Sicherheit

Auf der Registerkarte **Sicherheit** legen Sie fest, wie JeffCrunch mit den Dingen umgeht, auf die eine Datei verweisen kann: verknüpfte Dateien, verknüpfte Filme und Töne sowie Dateien aus dem Internet. Makros werden in einer Datei gespeichert und nie ausgeführt, und ein Link, der ein Programm starten würde, wird immer abgelehnt. Siehe [Sicherheit](security).

## Siehe auch

- [Willkommen bei JeffCrunch](start)
