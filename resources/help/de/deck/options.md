# Einstellungen von JeffDeck ändern
<!-- keywords: Optionen, Einstellungen, Voreinstellungen, Design, Dunkelmodus, Sprache, Benutzername, Initialen, Lineal, Einrasten, Rechtschreibung, AutoKorrektur, Anführungszeichen, Updates, Beta, Nutzungsstatistik, Info, Speichern, Schriftarten einbetten, Schriftarten, Tastatur anpassen, Menüband anpassen, Schnellsymbolleiste anpassen, Schnellsymbolleiste, Befehl zur Schnellsymbolleiste hinzufügen, Registerkarte ausblenden, Gruppe ausblenden, Registerkarte hinzufügen, Gruppe hinzufügen, Registerkarte umbenennen, Menüband zurücksetzen, Tastenkombinationen ändern, Tastenkombinationen neu zuweisen, Tastenkombinationen zurücksetzen -->

Ihre Einstellungen finden Sie im Fenster **JeffDeck-Optionen**. Klicken Sie auf **Datei > Optionen**, um es zu öffnen. Es hat sieben Registerkarten. Klicken Sie auf **OK**, um Ihre Änderungen zu übernehmen, oder auf **Abbrechen**, um sie zu verwerfen.

## Allgemein

- **Benutzername** und **Initialen** werden bei den Kommentaren angezeigt, die Sie schreiben. Siehe [Kommentare](comments).
- **Design** ist **Systemeinstellung verwenden**, **Hell** oder **Dunkel**. Die Änderung wird sofort angezeigt.
- **Anzeigesprache** ist die Sprache der Oberfläche von JeffDeck. **Systemsprache verwenden** übernimmt die Sprache Ihres Computers. Weitere Sprachen erscheinen in dieser Liste, wenn Übersetzungen verfügbar sind. Eine neue Sprache wird beim nächsten Start von JeffDeck übernommen.
- **Beim Start von JeffDeck nach Updates suchen** lässt JeffDeck bei jedem Start prüfen, ob eine neuere Version verfügbar ist. Gibt es eine, zeigt das Fenster **Update verfügbar**, was neu ist. Wählen Sie **Jetzt aktualisieren**, **Später** oder **Diese Version überspringen**. Unter Windows lädt **Jetzt aktualisieren** das Installationsprogramm herunter. Es installiert die neue Version aller JeffOffice-Apps und fordert Sie auf, geöffnete Apps zu schließen. Auf anderen Computern heißt die Schaltfläche **Downloadseite öffnen**. Sie öffnet die Seite, auf der Sie die neue Version erhalten. Ist auch JeffPub geöffnet, meldet nur eines der beiden Programme pro Tag eine neue Version.
- **Betaversionen erhalten (frühe Builds zum Testen)** lässt JeffDeck auch Betaversionen anbieten. Sie erscheinen vor den regulären Versionen und können noch Fehler enthalten.
- **Anonyme Nutzungsstatistiken senden** erlaubt, Angaben zur Nutzung von JeffDeck zu teilen. Anonym heißt, dass die Angaben nichts darüber verraten, wer Sie sind. Ist dies aktiviert, sendet JeffDeck diese Angaben einmal am Tag sowie am Tag einer Aktualisierung: dass es sich um JeffDeck handelt, die Version, Ihr Betriebssystem und Ihre Sprache, wie oft das Programm gestartet wurde, wie oft jeder Befehl verwendet wird und welche Art von Datei Sie geöffnet oder gespeichert haben (zum Beispiel .pptx). Schließt sich JeffDeck einmal unerwartet, sendet es beim nächsten Start außerdem einen Absturzbericht. Der Bericht sagt, an welcher Stelle im Programm es abbrach, damit das Problem gefunden und behoben werden kann. JeffDeck sendet niemals Ihre Dateien, deren Namen oder deren Inhalt und nichts, was Sie eingegeben haben. Ist dies deaktiviert, werden auch keine Absturzberichte gesendet. JeffDeck fragt beim ersten Start danach, im Fenster **Helfen Sie, JeffDeck zu verbessern**. Die Frage erscheint nicht, wenn Sie bereits in JeffPub oder im Setup entschieden haben.
- **Datenschutzhinweis** neben diesem Kontrollkästchen öffnet unseren Datenschutzhinweis unter jeffoffice.com/privacy. Dort steht, was gesendet wird, wohin es geht und wie lange wir es aufbewahren. Wir speichern Statistiken 24 Monate lang.
- **Installationsnummer** wird unter dem Kontrollkästchen angezeigt. Es ist eine zufällige Nummer für Ihre Kopie von JeffOffice, in allen JeffOffice-Apps gleich. Sie sagt nichts über Sie aus. Klicken Sie auf **Kopieren**, um sie zu kopieren. Sind Statistiken deaktiviert oder wurde noch keine Nummer erstellt, steht dort **keine**.
- Wenn wir die bereits vorliegenden Statistiken löschen lassen möchten, senden Sie Ihre Installationsnummer **bevor** Sie die Statistiken deaktivieren an info@jeffoffice.com. Beim Deaktivieren wird die Nummer von Ihrem Computer gelöscht, und ohne sie können wir nicht feststellen, welche Datensätze Ihre sind.
- Um die Statistiken zu deaktivieren, ohne JeffDeck zu öffnen, setzen Sie vor dem Start die Umgebungsvariable `JP_NO_TELEMETRY` auf einen beliebigen Wert. Dann wird nichts gezählt oder gesendet, und JeffDeck stellt die Frage beim ersten Start nicht.

**Benutzername**, **Initialen**, **Design** und **Anzeigesprache** sowie die Entscheidungen zu Updates und Statistiken gelten für alle JeffOffice-Apps: Eine Änderung hier wirkt sich in allen aus.

## Korrektur

Diese Kontrollkästchen steuern die Rechtschreibung und die Eingabehilfen. Siehe [Text eingeben und formatieren](text).

- **Rechtschreibung während der Eingabe prüfen**
- **Grammatik während der Eingabe prüfen** markiert eindeutige Grammatik- und Stilfehler in englischem Text, in allen regionalen Varianten, mit einer blauen gepunkteten Linie. Text in anderen Sprachen wird nur auf Rechtschreibung geprüft.
- **Grammatikeinstellungen** öffnet eine Liste der Fehlerarten, nach denen gesucht wird: Grammatik, Zeichensetzung, Groß- und Kleinschreibung, verwechselte Wörter, Wiederholungen und Stil (der bis zu seiner Aktivierung ausgeschaltet ist).
- **Wörter in GROSSBUCHSTABEN ignorieren**
- **Wörter mit Zahlen ignorieren**
- **Text während der Eingabe ersetzen (AutoKorrektur)** korrigiert typische Tippfehler. In den **AutoKorrektur-Optionen** gibt es auf der Registerkarte **Mathematische Symbole** einen Schalter **Namen mathematischer Symbole während der Eingabe ersetzen**, der erst aktiviert werden muss. Ist er eingeschaltet, wird ein Backslash mit einem Namen, etwa \alpha, zum Symbol (α), sobald Sie danach ein Leerzeichen oder ein Satzzeichen eingeben. Der Name muss am Wortanfang stehen, sodass ein Ordnerpfad wie C:\temp\pi so bleibt, wie Sie ihn eingegeben haben. Die Registerkarte listet alle Namen auf. JeffDeck, JeffWrite und JeffPub ersetzen sie; JeffCrunch ändert nichts in einer Zelle.
- **Gerade Anführungszeichen durch typografische ersetzen** wandelt " und ' in typografische Anführungszeichen um.
- **Automatisch formatieren während der Eingabe (Striche, Brüche, Ordinalzahlen, automatische Listen)**
- **Webadressen beim Eingeben automatisch in Links umwandeln** macht eine Web- oder E-Mail-Adresse zu einem Link, wenn Sie danach die Leertaste drücken. Mit **Rückgängig** wird nur der Link zurückgenommen.

## Ansicht

- **Lineal anzeigen** blendet die Lineale am Rand der Folie ein. Das entspricht **Ansicht > Anzeigen > Lineal** und gilt für alle Fenster, jetzt und für neu geöffnete.
- **Objekte an anderen Objekten ausrichten** lässt ein Objekt, das Sie ziehen, an den anderen Objekten auf der Folie einrasten.

## Erweitert

- **Taste Einfügen zum Ein- und Ausschalten des Überschreibmodus verwenden** ist anfangs ausgeschaltet. Ist sie aktiviert, wechselt die Taste **Einfügen** in einem Textfeld zwischen zwei Eingabearten. Normalerweise verschiebt jeder eingegebene Buchstabe die folgenden Buchstaben nach rechts. Im **Überschreibmodus** ersetzt jeder eingegebene Buchstabe den Buchstaben hinter der Einfügemarke, bis zum Ende des Absatzes. Die Statusleiste zeigt **Einfügen** oder **Überschreiben**, damit Sie wissen, in welchem Modus Sie sich befinden. Drücken Sie **Einfügen** erneut, um zurückzuwechseln.

## Speichern

- **Schriftarten in der Datei einbetten** legt die Schriftarten, die Ihre Folien verwenden, beim Speichern in die Datei, damit die Folien auf einem Computer ohne diese Schriftarten gleich aussehen. Das ist bis zu Ihrer Abwahl aktiviert. Es werden nur die Schriftarten eingebettet, die mit JeffDeck geliefert werden, und die Datei wird dadurch größer. Das Thema Präsentationen öffnen und speichern enthält die Einzelheiten unter „Schriftarten in der Datei“.
- **Persönliche Informationen beim Speichern aus dieser Datei entfernen** entfernt beim Schreiben der Datei (eine .pptx- oder .odp-Datei, eine Vorlage und eine per E-Mail gesendete Präsentation) den Autor, die Person, die die Datei zuletzt gespeichert hat, das Unternehmen, den Vorgesetzten sowie die Namen, Initialen und Zeiten der Kommentare. Was in der Datei steht, bleibt unverändert. Die benutzerdefinierten Eigenschaften der Datei bleiben ebenfalls erhalten, und das geöffnete Dokument wird nicht verändert. Es ist in allen JeffOffice-Apps dieselbe Einstellung: Eine Änderung in einer App gilt in allen. Dasselbe Kontrollkästchen finden Sie auf der Seite **Informationen**.

## Anpassen

Die Registerkarte **Anpassen** hat drei Seiten: **Menüband**, **Schnellsymbolleiste** und **Tastatur**. Auf der Seite **Tastatur** wählen Sie die Tastenkombinationen für die Befehle von JeffDeck. Siehe „Eigene Tastenkombinationen festlegen“ im Thema zur Tastatur.

### Menüband anpassen

Die Seite **Menüband** listet die Registerkarten des Menübands mit ihren Gruppen auf.

- **Registerkarte oder Gruppe ausblenden oder anzeigen.** Entfernen Sie das Häkchen daneben, um sie auszublenden, und setzen Sie es, um sie wieder anzuzeigen. Eine ausgeblendete Gruppe braucht keinen Platz, und ihre Befehle funktionieren weiterhin über ihre Tastenkombinationen. **Start** hat kein Kontrollkästchen, weil die Registerkarte immer angezeigt wird, ebenso die Schaltfläche **Datei**. Eine Registerkarte, die nur erscheint, wenn etwas ausgewählt ist, etwa ein Bild oder eine Tabelle, steht am Ende der Liste und hat kein Kontrollkästchen.
- **Eigene Registerkarte oder Gruppe erstellen.** Geben Sie einen Namen in das Feld **Name** ein und klicken Sie auf **Neue Registerkarte**. Um eine Gruppe zu erstellen, wählen Sie eine Registerkarte und klicken auf **Neue Gruppe**. Eine eigene Gruppe kann auf jede Registerkarte, auch auf eine, die nur bei einer Auswahl erscheint.
- **Befehle in Ihre Gruppe aufnehmen.** Wählen Sie Ihre Gruppe, suchen Sie einen Befehl in der Liste **Befehle** (geben Sie im Feld **Befehle suchen** ein, um die Liste einzugrenzen) und klicken Sie auf **Befehl hinzufügen**. Jeder Befehl mit einem Namen kann in eine Gruppe. Ein Befehl kann nur einmal in einer Gruppe stehen.
- **Umbenennen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl, geben Sie den neuen Namen ein und klicken Sie auf **Umbenennen**. Ein umbenannter Befehl tut dasselbe, der neue Name ist nur die Beschriftung seiner Schaltfläche. Um einem Befehl seinen ursprünglichen Namen zurückzugeben, benennen Sie ihn mit einem leeren Namen um.
- **Reihenfolge ändern.** Wählen Sie eine Registerkarte und klicken Sie auf **Nach oben** oder **Nach unten**. Sie können die Registerkarten des Menübands ebenso verschieben wie Ihre eigenen, nicht aber die Registerkarten, die nur bei einer Auswahl erscheinen. Ihre eigenen Gruppen und Befehle werden auf dieselbe Weise innerhalb ihrer Registerkarte oder Gruppe verschoben. Die Gruppen des Menübands behalten ihre Reihenfolge.
- **Entfernen.** Wählen Sie eine eigene Registerkarte, Gruppe oder einen eigenen Befehl und klicken Sie auf **Entfernen**.

**Eine zurücksetzen** stellt die gewählte Registerkarte wieder her: Sie wird angezeigt, ihre Gruppen werden angezeigt, und die von Ihnen hinzugefügten Gruppen verschwinden. Eine eigene Registerkarte wird entfernt. **Alle zurücksetzen** stellt das gesamte Menüband wieder her, einschließlich der Reihenfolge der Registerkarten.

Nichts ändert sich, bis Sie auf **OK** klicken. Dann zeigt jedes geöffnete JeffDeck-Fenster Ihr Menüband, ebenso jedes später geöffnete Fenster. JeffDeck speichert nur, was vom üblichen Menüband abweicht. So erscheint eine Registerkarte oder Gruppe, die eine spätere Version hinzufügt, von selbst. Ihre Registerkarten und Gruppen erhalten Tastenhinweise (die Buchstaben, die bei Drücken von **Alt** angezeigt werden), die sich nicht mit den übrigen überschneiden. Sie können 20 eigene Registerkarten haben, 30 eigene Gruppen auf einer Registerkarte und 60 Befehle in einer Gruppe.

### Schnellsymbolleiste anpassen

Die Seite **Schnellsymbolleiste** betrifft die kleine Symbolleiste oben links im Fenster. Ihre linke Liste zeigt, was jetzt auf der Symbolleiste steht. Die Liste **Befehle** rechts enthält alle Befehle mit einem Namen (geben Sie im Feld **Befehle suchen** ein, um sie einzugrenzen, wie auf der Seite **Menüband**).

- **Befehl hinzufügen.** Wählen Sie ihn in der Liste **Befehle** und klicken Sie auf **Befehl hinzufügen**. Er wird ans Ende der Symbolleiste gesetzt. Ein Befehl, der ein Menü öffnet, behält sein Menü auf der Symbolleiste. Ein Befehl kann nur einmal auf der Symbolleiste stehen.
- **Rechtsklick.** Sie können auch auf einen beliebigen Befehl im Menüband rechtsklicken und **Zur Schnellsymbolleiste hinzufügen** wählen oder auf eine Schaltfläche der Symbolleiste rechtsklicken und **Aus Schnellsymbolleiste entfernen** wählen. Dasselbe Menü enthält **Schnellsymbolleiste anpassen** und **Menüband anpassen**, die diese Seite bzw. die Seite des Menübands öffnen.
- **Unter dem Menüband anzeigen.** Setzen Sie das Häkchen bei **Schnellsymbolleiste unter dem Menüband anzeigen**, damit die Symbolleiste in einer Zeile unter dem Menüband statt oben links steht. Die beiden Rechtsklick-Menüs enthalten ebenfalls **Schnellsymbolleiste unter dem Menüband anzeigen**. JeffDeck merkt sich die Wahl.
- **Trennlinie hinzufügen.** Klicken Sie auf **Trennlinie hinzufügen**, um am Ende eine dünne Linie einzufügen, und verschieben Sie sie dann an die gewünschte Stelle.
- **Reihenfolge ändern.** Wählen Sie in der linken Liste einen Befehl oder eine Trennlinie und klicken Sie auf **Nach oben** oder **Nach unten**.
- **Entfernen.** Wählen Sie ihn und klicken Sie auf **Entfernen**.

**Eine zurücksetzen** stellt den gewählten Befehl wieder her: an seinen üblichen Platz oder von der Symbolleiste weg, wenn JeffDeck ihn dort normalerweise nicht hat. **Alle zurücksetzen** stellt die Symbolleiste wieder her, wie JeffDeck sie mitbringt. Alle zurücksetzen auf dieser Seite ändert Ihr Menüband nicht, und Alle zurücksetzen auf der Seite **Menüband** ändert Ihre Symbolleiste nicht.

Nichts ändert sich, bis Sie auf **OK** klicken. Dann zeigt jedes geöffnete JeffDeck-Fenster Ihre Symbolleiste, ebenso jedes später geöffnete Fenster. Die Symbolleiste kann insgesamt 100 Befehle und Trennlinien enthalten. Passen nicht alle Schaltflächen in ein Drittel der Fensterbreite, warten die übrigen hinter einer Schaltfläche **»** am Ende der Symbolleiste, sodass der Fenstertitel nie vom Bildschirm gedrängt wird. JeffDeck behält einen Befehl in Ihrer Liste, wenn eine spätere Version ihn nicht mehr hat, und zeigt ihn wieder an, sobald er zurückkehrt. Die Schaltflächen erhalten Tastenhinweise, die Zahlen 1 bis 9 und danach zwei Buchstaben, in der Reihenfolge, in der sie auf der Symbolleiste stehen. Die Symbolleiste bleibt immer oben links neben der Schaltfläche **Datei**.

## Sicherheit

Die Registerkarte **Sicherheit** legt fest, was JeffDeck mit den Dingen tut, auf die eine Datei verweisen kann: verknüpfte Dateien, verknüpfte Filme und Töne sowie Dateien aus dem Internet. Makros werden in einer Datei gespeichert und nie ausgeführt, und ein Link, der ein Programm starten würde, wird immer abgelehnt. Siehe [Sicherheit](security).

## Siehe auch

- [Registerkarte Ansicht](tab-view)
- [Nicht gespeicherte Arbeit wiederherstellen](recover)
- [Die Datei-Ansicht](file-screen)
