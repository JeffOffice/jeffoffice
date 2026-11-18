# Ändern der Einstellungen von JeffDeck
<!-- keywords: Optionen, Einstellungen, Voreinstellungen, Design, dunkler Modus, Sprache, Benutzername, Initialen, Lineal, einrasten, Rechtschreibung, AutoKorrektur, Anführungszeichen, Updates, Beta, Nutzungsstatistiken, Info, speichern, Schriftarten einbetten, Schriftarten, Tastatur anpassen, Menüband anpassen, Schnellsymbolleiste anpassen, Schnellsymbolleiste, Befehl zur Schnellsymbolleiste hinzufügen, Registerkarte ausblenden, Gruppe ausblenden, Registerkarte hinzufügen, Gruppe hinzufügen, Registerkarte umbenennen, Menüband zurücksetzen, Tastenkombinationen ändern, Tastenkombinationen neu zuweisen, Tastenkombinationen zurücksetzen -->

Ihre Einstellungen finden Sie im Fenster **JeffDeck-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Es hat sieben Registerkarten. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Allgemein

- **Benutzername** und **Initialen** werden bei den Kommentaren angezeigt, die Sie schreiben. Siehe [Kommentare](comments).
- **Design** ist **Systemeinstellung verwenden**, **Hell** oder **Dunkel**. Die Änderung wird sofort angezeigt.
- **Anzeigesprache** ist die Sprache der Oberfläche von JeffDeck. **Systemsprache verwenden** richtet sich nach Ihrem Computer. Weitere Sprachen erscheinen in dieser Liste, sobald Übersetzungen verfügbar sind. Eine neue Sprache wird beim nächsten Start von JeffDeck wirksam.
- **Beim Start von JeffDeck nach Updates suchen** sorgt dafür, dass JeffDeck bei jedem Start nach einer neueren Version sucht. Gibt es eine, zeigt das Fenster **Update verfügbar**, was neu ist. Wählen Sie **Jetzt aktualisieren**, **Später** oder **Diese Version überspringen**. Unter Windows lädt **Jetzt aktualisieren** das Installationsprogramm herunter. Es installiert die neue Version aller JeffOffice-Apps und fordert Sie auf, alle geöffneten zu schließen. Auf anderen Computern heißt die Schaltfläche **Downloadseite öffnen**. Sie öffnet die Seite, auf der Sie die neue Version erhalten. Ist auch JeffPub geöffnet, benachrichtigt nur eines der beiden Programme Sie einmal täglich über eine neue Version.
- **Betaversionen erhalten (frühe Builds zum Testen)** sorgt dafür, dass JeffDeck auch Betaversionen anbietet. Diese erscheinen vor den regulären Versionen und können noch Fehler enthalten.
- **Anonyme Nutzungsstatistiken senden** ermöglicht es Ihnen, Angaben darüber zu teilen, wie JeffDeck verwendet wird. Anonym bedeutet, dass die Angaben nichts über Ihre Person verraten. Ist diese Option aktiviert, sendet JeffDeck diese Angaben einmal täglich sowie an dem Tag, an dem Sie aktualisieren: dass es sich um JeffDeck handelt, seine Version, Ihr Betriebssystem und Ihre Sprache, wie oft es gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie öffnen oder speichern (zum Beispiel .pptx). Schließt sich JeffDeck einmal unerwartet, sendet es außerdem beim nächsten Start einen Absturzbericht. Der Bericht gibt an, an welcher Stelle des Programms es angehalten hat, damit das Problem gefunden und behoben werden kann. JeffDeck sendet niemals Ihre Dateien, deren Namen oder deren Inhalt und nichts, was Sie eingegeben haben. Ist diese Option aus, wird auch kein Absturzbericht gesendet. JeffDeck fragt beim ersten Start in einem Fenster namens **Helfen Sie, JeffDeck zu verbessern**, danach. Die Frage erscheint nicht, wenn Sie das bereits in JeffPub oder im Installationsprogramm festgelegt haben.
- **Datenschutzhinweis** neben diesem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Statistiken bewahren wir 24 Monate auf.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Sie ist eine Zufallszahl für Ihre Kopie von JeffOffice und in allen JeffOffice-Apps gleich. Sie sagt nichts über Ihre Person aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind die Statistiken aus oder wurde noch keine Nummer erstellt, steht dort **keine**.
- Wenn die bereits bei uns gespeicherten Statistiken gelöscht werden sollen, senden Sie Ihre Installationsnummer **vor** dem Ausschalten der Statistiken an info@jeffoffice.com. Beim Ausschalten wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihnen gehören.
- Um die Statistiken auszuschalten, ohne JeffDeck zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffDeck stellt die Frage beim ersten Start nicht.

**Benutzername**, **Initialen**, **Design** und **Anzeigesprache** sowie die Einstellungen für Updates und Statistiken gelten für alle JeffOffice-Apps: Ändern Sie eine davon hier, wird sie in jeder App geändert.

## Dokumentprüfung

Diese Kontrollkästchen steuern die Rechtschreibprüfung und die Eingabehilfen. Siehe [Text eingeben und formatieren](text).

- **Rechtschreibung während der Eingabe prüfen**
- **Grammatik während der Eingabe prüfen** markiert eindeutige Grammatik- und Stilfehler in englischen, französischen, deutschen und spanischen Texten mit einer blauen gepunkteten Linie.
- **Grammatikeinstellungen** öffnet eine Liste der Fehlerarten, nach denen gesucht wird: Grammatik, Zeichensetzung, Groß- und Kleinschreibung, verwechselte Wörter, Wiederholungen und Stil (der ist aus, bis Sie ihn einschalten).
- **Wörter in GROSSBUCHSTABEN ignorieren**
- **Wörter mit Zahlen ignorieren**
- **Text während der Eingabe ersetzen (AutoKorrektur)** behebt häufige Tippfehler.
- **Gerade Anführungszeichen durch typografische ersetzen** wandelt " und ' in typografische Anführungszeichen um.
- **Während der Eingabe automatisch formatieren (Striche, Brüche, Ordnungszahlen, automatische Listen)**
- **Webadressen beim Eingeben automatisch in Links umwandeln** macht eine Web- oder E-Mail-Adresse zu einem Link, sobald Sie danach die Leertaste drücken. Mit **Rückgängig** wird nur der Link zurückgenommen.

## Ansicht

- **Lineal anzeigen** zeigt die Lineale am Rand der Folie. Das entspricht **Ansicht > Anzeigen > Lineal** und gilt für alle Fenster, jetzt und für alle, die Sie später öffnen.
- **Objekte an anderen Objekten einrasten lassen** richtet ein Objekt, das Sie verschieben, an den anderen Objekten auf der Folie aus.

## Erweitert

- **Mit der Einfügen-Taste Überschreibmodus ein- und ausschalten** ist zunächst aus. Ist sie eingeschaltet, wechselt die Taste **Einfügen** zwischen zwei Arten der Eingabe in einem Textfeld. Normalerweise schiebt jeder Buchstabe, den Sie eingeben, die folgenden Buchstaben weiter. Im **Überschreibmodus** ersetzt jeder Buchstabe den Buchstaben hinter der Einfügemarke, bis zum Ende des Absatzes. Die Statusleiste zeigt **Einfügen** oder **Überschreiben** an, damit Sie wissen, in welchem Modus Sie sich befinden. Drücken Sie erneut **Einfügen**, um zurückzuwechseln.

## Speichern

- **Schriftarten in der Datei einbetten** legt die Schriftarten, die Ihre Folien verwenden, beim Speichern in die Datei, damit die Folien auf einem Computer ohne diese Schriftarten gleich aussehen. Die Option ist eingeschaltet, bis Sie sie deaktivieren. Es werden nur die Schriftarten eingebettet, die mit JeffDeck geliefert werden, und die Datei wird dadurch größer. Details dazu finden Sie im Thema Präsentationen öffnen und speichern unter „Schriftarten in der Datei“.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** entfernt beim Schreiben der Datei (eine .pptx- oder .odp-Datei, eine Vorlage und eine per E-Mail gesendete Präsentation) den Autor, die Person, die die Datei zuletzt gespeichert hat, das Unternehmen, den Vorgesetzten sowie die Namen, Initialen und Zeitangaben bei Kommentaren. Der Inhalt der Datei bleibt unverändert. Die benutzerdefinierten Eigenschaften der Datei bleiben ebenfalls erhalten, und das geöffnete Dokument wird nicht verändert. Diese Auswahl gilt in allen JeffOffice-Apps: Ändern Sie sie in einer, ändert sie sich in allen. Dasselbe Kontrollkästchen befindet sich auf der Seite **Info**.

## Anpassen

Die Registerkarte **Anpassen** hat drei Seiten: **Menüband**, **Schnellsymbolleiste** und **Tastatur**. Auf der Seite **Tastatur** wählen Sie die Tastenkombinationen für die Befehle von JeffDeck. Siehe „Eigene Tastenkombinationen festlegen“ im Thema zur Tastatur.

### Menüband anpassen

Die Seite **Menüband** listet die Registerkarten des Menübands mit ihren Gruppen auf.

- **Registerkarte oder Gruppe ausblenden oder anzeigen.** Deaktivieren Sie das Kontrollkästchen daneben, um sie auszublenden, und aktivieren Sie es, um sie wieder anzuzeigen. Eine ausgeblendete Gruppe benötigt keinen Platz, und ihre Befehle funktionieren weiterhin über ihre Tastenkombinationen. **Start** hat kein Kontrollkästchen, weil die Registerkarte immer angezeigt wird, ebenso die Schaltfläche **Datei**. Eine Registerkarte, die nur erscheint, wenn etwas ausgewählt ist, etwa ein Bild oder eine Tabelle, steht am Ende der Liste und hat kein Kontrollkästchen.
- **Eigene Registerkarte oder Gruppe erstellen.** Geben Sie im Feld **Name** einen Namen ein, und klicken Sie auf **Neue Registerkarte**. Um eine Gruppe zu erstellen, wählen Sie eine Registerkarte und klicken auf **Neue Gruppe**. Eine eigene Gruppe kann auf jeder Registerkarte stehen, auch auf einer, die nur bei Auswahl erscheint.
- **Befehle in Ihre Gruppe aufnehmen.** Wählen Sie Ihre Gruppe, suchen Sie einen Befehl in der Liste **Befehle** (geben Sie im Feld **Befehle suchen** einen Begriff ein, um die Liste einzugrenzen), und klicken Sie auf **Befehl hinzufügen**. Jeder Befehl mit einem Namen kann aufgenommen werden. Ein Befehl kann in einer Gruppe nur einmal vorkommen.
- **Umbenennen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl, geben Sie den neuen Namen ein und klicken Sie auf **Umbenennen**. Ein umbenannter Befehl funktioniert genauso, der neue Name steht nur auf seiner Schaltfläche. Um einem Befehl wieder seinen ursprünglichen Namen zu geben, benennen Sie ihn mit einem leeren Namen um.
- **Reihenfolge ändern.** Wählen Sie eine Registerkarte und klicken Sie auf **Nach oben** oder **Nach unten**. Sie können die Registerkarten des Menübands ebenso verschieben wie Ihre eigenen, nicht aber die Registerkarten, die nur bei Auswahl erscheinen. Ihre Gruppen und Befehle lassen sich innerhalb ihrer Registerkarte bzw. Gruppe auf dieselbe Weise verschieben. Die Gruppen des Menübands behalten ihre Reihenfolge.
- **Entfernen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl und klicken Sie auf **Entfernen**.

**Einzeln zurücksetzen** stellt die gewählte Registerkarte wieder her: Sie wird angezeigt, ihre Gruppen werden angezeigt, und die Gruppen, die Sie ihr hinzugefügt haben, sind verschwunden. Eine eigene Registerkarte wird entfernt. **Alle zurücksetzen** stellt das gesamte Menüband wieder her, einschließlich der Reihenfolge der Registerkarten.

Nichts ändert sich, bis Sie auf **OK** klicken. Dann zeigt jedes geöffnete Fenster von JeffDeck Ihr Menüband, ebenso jedes Fenster, das Sie später öffnen. JeffDeck speichert nur, was sich vom üblichen Menüband unterscheidet. Eine Registerkarte oder Gruppe, die eine spätere Version hinzufügt, erscheint daher von selbst. Ihre Registerkarten und Gruppen erhalten Zugriffstasten (die Buchstaben, die beim Drücken von **Alt** angezeigt werden), die sich nicht mit den übrigen überschneiden. Sie können 20 eigene Registerkarten haben, 30 eigene Gruppen auf einer Registerkarte und 60 Befehle in einer Gruppe.

### Schnellsymbolleiste anpassen

Die Seite **Schnellsymbolleiste** betrifft die kleine Symbolleiste oben links im Fenster. Ihre linke Liste zeigt, was aktuell in der Symbolleiste steht. Die Liste **Befehle** auf der rechten Seite enthält alle Befehle mit einem Namen (geben Sie im Feld **Befehle suchen** einen Begriff ein, um die Liste einzugrenzen, wie auf der Seite **Menüband**).

- **Befehl hinzufügen.** Wählen Sie ihn in der Liste **Befehle** und klicken Sie auf **Befehl hinzufügen**. Er wird ans Ende der Symbolleiste gesetzt. Ein Befehl, der ein Menü öffnet, behält sein Menü in der Symbolleiste. Ein Befehl kann nur einmal in der Symbolleiste stehen.
- **Rechtsklick.** Sie können auch auf einen beliebigen Befehl im Menüband rechtsklicken und **Zur Schnellsymbolleiste hinzufügen** wählen oder auf eine Schaltfläche der Symbolleiste rechtsklicken und **Aus Schnellsymbolleiste entfernen** wählen. Im selben Menü finden Sie auch **Schnellsymbolleiste anpassen** und **Menüband anpassen**, die diese Seite beziehungsweise die Seite zum Menüband öffnen.
- **Unterhalb des Menübands anzeigen.** Aktivieren Sie **Schnellsymbolleiste unter dem Menüband anzeigen**, um die Symbolleiste als Zeile unter dem Menüband statt oben links anzuzeigen. Auch die beiden Rechtsklick-Menüs enthalten **Schnellsymbolleiste unterhalb des Menübands anzeigen**. JeffDeck merkt sich die Auswahl.
- **Trennlinie hinzufügen.** Klicken Sie auf **Trennlinie hinzufügen**, um am Ende eine dünne Linie einzufügen, und verschieben Sie sie anschließend an die gewünschte Stelle.
- **Reihenfolge ändern.** Wählen Sie in der linken Liste einen Befehl oder eine Trennlinie und klicken Sie auf **Nach oben** oder **Nach unten**.
- **Entfernen.** Wählen Sie den Eintrag und klicken Sie auf **Entfernen**.

**Einzeln zurücksetzen** stellt den gewählten Befehl wieder her: an seinem üblichen Platz oder außerhalb der Symbolleiste, wenn JeffDeck ihn dort normalerweise nicht hat. **Alle zurücksetzen** stellt die Symbolleiste wieder her, wie JeffDeck sie mitbringt. Alle zurücksetzen auf dieser Seite ändert Ihr Menüband nicht, und Alle zurücksetzen auf der Seite **Menüband** ändert Ihre Symbolleiste nicht.

Nichts ändert sich, bis Sie auf **OK** klicken. Dann zeigt jedes geöffnete Fenster von JeffDeck Ihre Symbolleiste, ebenso jedes Fenster, das Sie später öffnen. Die Symbolleiste kann insgesamt 100 Befehle und Trennlinien enthalten. Passen mehr Schaltflächen nicht in ein Drittel der Fensterbreite, warten die restlichen hinter einer Schaltfläche **»** am Ende der Symbolleiste, sodass der Fenstertitel nie vom Bildschirm verdrängt wird. JeffDeck behält einen Befehl in Ihrer Liste, wenn eine spätere Version ihn nicht mehr hat, und zeigt ihn wieder an, sobald er zurückkommt. Die Schaltflächen erhalten Zugriffstasten: die Ziffern 1 bis 9 und danach zwei Buchstaben, in der Reihenfolge, in der sie in der Symbolleiste stehen. Die Symbolleiste steht immer oben links neben der Schaltfläche **Datei**.

## Sicherheit

Die Registerkarte **Sicherheit** legt fest, wie JeffDeck mit den Dingen umgeht, auf die eine Datei verweisen kann: verknüpfte Dateien, verknüpfte Filme und Sounds sowie Dateien aus dem Internet. Makros werden in einer Datei gespeichert und nie ausgeführt, und ein Link, der ein Programm starten würde, wird immer abgelehnt. Siehe [Sicherheit](security).

## Siehe auch

- [Registerkarte Ansicht](tab-view)
- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Die Seite Datei](file-screen)
