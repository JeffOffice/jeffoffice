# Einstellungen von JeffDeck ändern
<!-- keywords: Optionen, Einstellungen, Voreinstellungen, Design, dunkler Modus, Sprache, Benutzername, Initialen, Lineal, einrasten, Rechtschreibung, AutoKorrektur, Anführungszeichen, Updates, Beta, Nutzungsstatistik, Info, speichern, Schriftarten einbetten, Schriftarten, Tastatur anpassen, Menüband anpassen, Schnellsymbolleiste anpassen, Schnellsymbolleiste, Befehl zur Schnellsymbolleiste hinzufügen, Registerkarte ausblenden, Gruppe ausblenden, Registerkarte hinzufügen, Gruppe hinzufügen, Registerkarte umbenennen, Menüband zurücksetzen, Tastenkombination ändern, Tasten neu zuweisen, Tasten zurücksetzen -->

Ihre Einstellungen finden Sie im Fenster **JeffDeck-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Es hat sieben Registerkarten. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Allgemein

- **Benutzername** und **Initialen** werden bei den Kommentaren angezeigt, die Sie schreiben. Siehe [Kommentare](comments).
- **Design** ist **Systemeinstellung verwenden**, **Hell** oder **Dunkel**. Die Änderung wird sofort angezeigt.
- **Anzeigesprache** ist die Sprache der Oberfläche von JeffDeck. **Systemsprache verwenden** übernimmt die Sprache Ihres Computers. Weitere Sprachen erscheinen in dieser Liste, sobald Übersetzungen verfügbar sind. Eine neue Sprache wird beim nächsten Start von JeffDeck wirksam.
- **Beim Start von JeffDeck nach Updates suchen** sorgt dafür, dass JeffDeck bei jedem Start nach einer neueren Version sucht. Gibt es eine, zeigt ein Fenster **Update verfügbar**, was neu ist. Wählen Sie **Jetzt aktualisieren**, **Später** oder **Diese Version überspringen**. Unter Windows lädt **Jetzt aktualisieren** das Installationsprogramm herunter. Es installiert die neue Version aller JeffOffice-Apps und fordert Sie auf, geöffnete Apps zu schließen. Auf anderen Computern heißt die Schaltfläche **Downloadseite öffnen**. Sie öffnet die Seite, auf der Sie die neue Version erhalten. Ist auch JeffPub geöffnet, meldet pro Tag nur eines der beiden Programme eine neue Version.
- **Betaversionen erhalten (frühe Builds zum Testen)** sorgt dafür, dass JeffDeck auch Betaversionen anbietet. Diese erscheinen vor den regulären Versionen und können noch Fehler enthalten.
- **Anonyme Nutzungsstatistiken senden** erlaubt es Ihnen, Angaben darüber zu teilen, wie JeffDeck verwendet wird. Anonym heißt, dass die Angaben nichts darüber verraten, wer Sie sind. Ist diese Option aktiv, sendet JeffDeck diese Angaben einmal am Tag und am Tag, an dem Sie aktualisieren: dass es sich um JeffDeck handelt, seine Version, Ihr Betriebssystem und Ihre Sprache, wie oft es gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben (zum Beispiel .pptx). Schließt sich JeffDeck einmal unerwartet, sendet es beim nächsten Start außerdem einen Absturzbericht. Der Bericht sagt, an welcher Stelle im Programm es abbrach, damit das Problem gefunden und behoben werden kann. JeffDeck sendet nie Ihre Dateien, deren Namen oder deren Inhalt und nichts, was Sie eingegeben haben. Ist die Option aus, wird auch kein Absturzbericht gesendet. JeffDeck fragt beim ersten Start danach, im Fenster **Helfen Sie, JeffDeck zu verbessern**. Die Frage entfällt, wenn Sie bereits in JeffPub oder im Setup eine Wahl getroffen haben.
- **Datenschutzhinweis** neben diesem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Statistiken bewahren wir 24 Monate auf.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Sie ist eine Zufallszahl für Ihre Kopie von JeffOffice und in jeder JeffOffice-App gleich. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind die Statistiken ausgeschaltet oder wurde noch keine Nummer erzeugt, steht dort **keine**.
- Wenn die bisher bei uns gespeicherten Statistiken gelöscht werden sollen, senden Sie Ihre Installationsnummer **bevor** Sie die Statistiken ausschalten an info@jeffoffice.com. Beim Ausschalten wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihnen gehören.
- Um die Statistiken auszuschalten, ohne JeffDeck zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffDeck stellt die Frage beim ersten Start nicht.

**Benutzername**, **Initialen**, **Design** und **Anzeigesprache** sowie die Entscheidungen zu Updates und Statistiken gelten für alle JeffOffice-Apps: Ändern Sie eine davon hier, wird sie in jeder App geändert.

## Dokumentprüfung

Diese Kontrollkästchen steuern die Rechtschreibung und Hilfen beim Schreiben. Siehe [Text eingeben und formatieren](text).

- **Rechtschreibung während der Eingabe prüfen**
- **Grammatik während der Eingabe prüfen** markiert eindeutige Grammatik- und Stilfehler in englischem, französischem, deutschem und spanischem Text mit einer blauen gepunkteten Linie.
- **Grammatikeinstellungen** öffnet eine Liste der Fehlerarten, nach denen gesucht wird: Grammatik, Zeichensetzung, Groß- und Kleinschreibung, verwechselte Wörter, Wiederholungen und Stil (der ist aus, bis Sie ihn einschalten).
- **Wörter in GROSSBUCHSTABEN ignorieren**
- **Wörter mit Zahlen ignorieren**
- **Text während der Eingabe ersetzen (AutoKorrektur)** korrigiert häufige Tippfehler. Unter **AutoKorrektur-Optionen** gibt es auf der Registerkarte **Mathematische Symbole** einen Schalter **Namen mathematischer Symbole während der Eingabe ersetzen**, der zunächst aus ist. Ist er an, wird aus einem Backslash und einem Namen, etwa \alpha, das Symbol (α), sobald Sie danach ein Leerzeichen oder ein Satzzeichen eingeben. Der Name muss am Wortanfang stehen, daher bleibt ein Ordnerpfad wie C:\temp\pi so, wie Sie ihn eingegeben haben. Die Registerkarte listet alle Namen auf. JeffDeck, JeffWrite und JeffPub ersetzen sie; JeffCrunch ändert in einer Zelle nichts.
- **Gerade Anführungszeichen durch typografische ersetzen** wandelt " und ' in typografische Anführungszeichen um.
- **Automatisch formatieren während der Eingabe (Striche, Brüche, Ordnungszahlen, automatische Listen)**
- **Webadressen beim Eingeben automatisch in Links umwandeln** macht eine Web- oder E-Mail-Adresse zu einem Link, sobald Sie danach die Leertaste drücken. Mit Rückgängig wird nur der Link zurückgenommen.

## Ansicht

- **Lineal anzeigen** blendet die Lineale am Rand der Folie ein. Das entspricht **Ansicht > Anzeigen > Lineal** und gilt für alle Fenster, die geöffnet sind, und für alle, die Sie später öffnen.
- **Objekte an anderen Objekten einrasten lassen** sorgt dafür, dass sich ein Objekt, das Sie ziehen, an den anderen Objekten auf der Folie ausrichtet.

## Erweitert

- **Einfügen-Taste zum Ein- und Ausschalten des Überschreibmodus verwenden** ist anfangs aus. Ist sie an, wechselt die Taste **Einfügen** zwischen zwei Arten der Texteingabe in einem Textfeld. Normalerweise schiebt jeder eingegebene Buchstabe die folgenden Buchstaben weiter. Im Modus **Überschreiben** ersetzt jeder eingegebene Buchstabe den Buchstaben hinter der Einfügemarke, bis zum Ende des Absatzes. Die Statusleiste zeigt **Einfügen** oder **Überschreiben** an, damit Sie wissen, in welchem Modus Sie sich befinden. Drücken Sie erneut **Einfügen**, um zurückzuwechseln.

## Speichern

- **Schriftarten in der Datei einbetten** speichert beim Speichern die Schriftarten der Folien in der Datei, damit die Folien auch auf einem Computer ohne diese Schriftarten gleich aussehen. Die Option ist aktiv, bis Sie sie deaktivieren. Es werden nur Schriftarten eingebettet, die mit JeffDeck mitgeliefert werden, und die Datei wird dadurch größer. Im Thema Präsentationen öffnen und speichern finden Sie die Einzelheiten unter „Schriftarten in der Datei“.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** entfernt beim geschriebenen Datei (eine .pptx- oder .odp-Datei, eine Vorlage und eine per E-Mail gesendete Präsentation) den Verfasser, die Person, die die Datei zuletzt gespeichert hat, das Unternehmen, den Vorgesetzten sowie die Namen, Initialen und Zeiten der Kommentare. Was in der Datei steht, bleibt unverändert. Die benutzerdefinierten Eigenschaften der Datei bleiben, wie sie sind, und das geöffnete Dokument wird nicht verändert. Diese Einstellung gilt in allen JeffOffice-Apps: Ändern Sie sie in einer, ändert sie sich in allen. Dasselbe Kontrollkästchen befindet sich auf der Seite **Info**.

## Anpassen

Die Registerkarte **Anpassen** hat drei Seiten: **Menüband**, **Schnellsymbolleiste** und **Tastatur**. Auf der Seite **Tastatur** wählen Sie die Tastenkombinationen für die Befehle von JeffDeck. Siehe „Eigene Tastenkombinationen festlegen“ im Thema zur Tastatur.

### Menüband anpassen

Die Seite **Menüband** listet die Registerkarten des Menübands mit ihren Gruppen auf.

- **Registerkarte oder Gruppe ausblenden oder einblenden.** Entfernen Sie den Haken neben ihr, um sie auszublenden, und setzen Sie ihn, um sie wieder einzublenden. Eine ausgeblendete Gruppe braucht keinen Platz, und ihre Befehle funktionieren weiterhin über ihre Tastenkombinationen. **Start** hat kein Kontrollkästchen, weil die Registerkarte immer angezeigt wird, ebenso die Schaltfläche **Datei**. Eine Registerkarte, die nur erscheint, wenn etwas ausgewählt ist, etwa ein Bild oder eine Tabelle, steht am Ende der Liste und hat kein Kontrollkästchen.
- **Eigene Registerkarte oder Gruppe erstellen.** Geben Sie einen Namen im Feld **Name** ein und klicken Sie auf **Neue Registerkarte**. Für eine neue Gruppe wählen Sie eine Registerkarte aus und klicken auf **Neue Gruppe**. Eine eigene Gruppe kann auf jede Registerkarte, auch auf eine, die nur bei Auswahl erscheint.
- **Befehle in Ihre Gruppe aufnehmen.** Wählen Sie Ihre Gruppe, suchen Sie in der Liste **Befehle** einen Befehl (geben Sie im Feld **Befehle suchen** ein, um die Liste einzugrenzen) und klicken Sie auf **Befehl hinzufügen**. Jeder Befehl mit einem Namen kann aufgenommen werden. Ein Befehl kann nur einmal in einer Gruppe stehen.
- **Umbenennen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl, geben Sie den neuen Namen ein und klicken Sie auf **Umbenennen**. Ein umbenannter Befehl tut dasselbe, der neue Name steht nur auf seiner Schaltfläche. Um einem Befehl seinen ursprünglichen Namen zurückzugeben, benennen Sie ihn mit einem leeren Namen um.
- **Reihenfolge ändern.** Wählen Sie eine Registerkarte und klicken Sie auf **Nach oben** oder **Nach unten**. Sie können die Registerkarten des Menübands verschieben, aber auch Ihre eigenen. Nicht verschieben können Sie die Registerkarten, die nur bei Auswahl erscheinen. Ihre Gruppen und Befehle werden auf dieselbe Weise innerhalb ihrer Registerkarte oder Gruppe verschoben. Die Gruppen des Menübands bleiben in ihrer Reihenfolge.
- **Entfernen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl und klicken Sie auf **Entfernen**.

**Eine zurücksetzen** stellt die gewählte Registerkarte wieder her: Sie wird angezeigt, ihre Gruppen werden angezeigt, und die Gruppen, die Sie ihr hinzugefügt haben, sind weg. Eine eigene Registerkarte wird entfernt. **Alle zurücksetzen** stellt das gesamte Menüband wieder her, einschließlich der Reihenfolge der Registerkarten.

Erst wenn Sie auf **OK** klicken, wird etwas geändert. Dann zeigt jedes geöffnete Fenster von JeffDeck Ihr Menüband, ebenso jedes Fenster, das Sie später öffnen. JeffDeck speichert nur, was sich vom üblichen Menüband unterscheidet. Eine Registerkarte oder Gruppe, die eine spätere Version hinzufügt, erscheint daher von selbst. Ihre Registerkarten und Gruppen erhalten Tastenkombinationen (die Buchstaben, die beim Drücken von **Alt** angezeigt werden), die sich nicht mit anderen überschneiden. Sie können 20 eigene Registerkarten haben, auf einer Registerkarte 30 eigene Gruppen und in einer Gruppe 60 Befehle.

### Schnellsymbolleiste anpassen

Die Seite **Schnellsymbolleiste** betrifft die kleine Symbolleiste oben links im Fenster. Die linke Liste zeigt, was sich derzeit auf der Symbolleiste befindet. Die Liste **Befehle** rechts enthält alle Befehle mit einem Namen (geben Sie im Feld **Befehle suchen** ein, um sie einzugrenzen, wie auf der Seite **Menüband**).

- **Befehl hinzufügen.** Wählen Sie ihn in der Liste **Befehle** und klicken Sie auf **Befehl hinzufügen**. Er wird am Ende der Symbolleiste eingefügt. Ein Befehl, der ein Menü öffnet, behält sein Menü auf der Symbolleiste. Ein Befehl kann nur einmal auf der Symbolleiste stehen.
- **Rechtsklick.** Sie können auch mit der rechten Maustaste auf einen beliebigen Befehl im Menüband klicken und **Zur Schnellsymbolleiste hinzufügen** wählen oder mit der rechten Maustaste auf eine Schaltfläche der Symbolleiste klicken und **Aus Schnellsymbolleiste entfernen** wählen. Im selben Menü stehen auch **Schnellsymbolleiste anpassen** und **Menüband anpassen**, die diese Seite beziehungsweise die Seite **Menüband** öffnen.
- **Unter dem Menüband anzeigen.** Aktivieren Sie **Schnellsymbolleiste unter dem Menüband anzeigen**, um die Symbolleiste in einer Zeile unter dem Menüband statt oben links anzuzeigen. Die beiden Rechtsklick-Menüs haben ebenfalls **Schnellsymbolleiste unter dem Menüband anzeigen**. JeffDeck merkt sich die Einstellung.
- **Trennlinie hinzufügen.** Klicken Sie auf **Trennlinie hinzufügen**, um am Ende eine dünne Linie einzufügen, und verschieben Sie sie dann an die gewünschte Stelle.
- **Reihenfolge ändern.** Wählen Sie in der linken Liste einen Befehl oder eine Trennlinie und klicken Sie auf **Nach oben** oder **Nach unten**.
- **Entfernen.** Wählen Sie ihn aus und klicken Sie auf **Entfernen**.

**Eine zurücksetzen** stellt den gewählten Befehl wieder her: an seinem üblichen Platz oder, falls JeffDeck ihn dort normalerweise nicht hat, außerhalb der Symbolleiste. **Alle zurücksetzen** stellt die Symbolleiste wieder her, mit der JeffDeck ausgeliefert wird. Alle zurücksetzen auf dieser Seite ändert Ihr Menüband nicht, und Alle zurücksetzen auf der Seite **Menüband** ändert Ihre Symbolleiste nicht.

Erst wenn Sie auf **OK** klicken, wird etwas geändert. Dann zeigt jedes geöffnete Fenster von JeffDeck Ihre Symbolleiste, ebenso jedes Fenster, das Sie später öffnen. Die Symbolleiste kann insgesamt 100 Befehle und Trennlinien enthalten. Passen mehr Schaltflächen nicht in ein Drittel der Fensterbreite, warten die übrigen hinter einer Schaltfläche **»** am Ende der Symbolleiste, damit der Fenstertitel nie vom Bildschirm gedrängt wird. JeffDeck behält einen Befehl in Ihrer Liste, wenn eine spätere Version ihn nicht mehr hat, und zeigt ihn wieder an, wenn er zurückkehrt. Die Schaltflächen erhalten Tastenkombinationen: die Zahlen 1 bis 9 und danach zwei Buchstaben, in der Reihenfolge, in der sie auf der Symbolleiste stehen. Die Symbolleiste befindet sich immer oben links neben der Schaltfläche **Datei**.

## Sicherheit

Die Registerkarte **Sicherheit** legt fest, was JeffDeck mit den Dingen tut, auf die eine Datei verweisen kann: verknüpfte Dateien, verknüpfte Filme und Töne sowie Dateien aus dem Internet. Makros werden in einer Datei gespeichert und nie ausgeführt, und ein Link, der ein Programm starten würde, wird immer abgelehnt. Siehe [Sicherheit](security).

## Siehe auch

- [Registerkarte Ansicht](tab-view)
- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Der Bereich Datei](file-screen)
