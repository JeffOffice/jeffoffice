# Rechtschreibung prüfen
<!-- keywords: Rechtschreibung, Rechtschreibprüfung, Rechtschreibfehler, falsch geschrieben, rote Linie, rote Wellenlinie, Wellenlinie, Vorschläge, ignorieren, alle ignorieren, zum Wörterbuch hinzufügen, Benutzerwörterbuch, Wörterbuch, Sprache, Dokumentprüfung, F7, Überprüfen, Rechtschreibung während der Eingabe prüfen, Großbuchstaben, Zahlen, Grammatik, Grammatikprüfung, Stil, blaue Linie, gepunktete Linie, Grammatik während der Eingabe prüfen -->

JeffWrite prüft die Rechtschreibung während der Eingabe. Ein Wort, das nicht im Wörterbuch steht, erhält eine rote Wellenlinie. Die Prüfung läuft auf Ihrem eigenen Computer, ohne Internetverbindung, und kennt 31 Sprachen und regionale Varianten.

## Während der Eingabe

- Eine rote Wellenlinie bedeutet, dass das Wörterbuch das Wort nicht kennt. Das Wort kann trotzdem richtig sein. Namen und neue Wörter fehlen oft. Die Namen der JeffOffice-Apps (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail und JeffBase) sowie „LLC“ werden in keiner Sprache markiert, mit oder ohne „s“.
- Das Wort, das Sie gerade eingeben, bleibt unmarkiert, bis Sie weitergehen. So flackert ein halb getipptes Wort nicht.
- Die Linien erscheinen nur auf dem Bildschirm. Sie werden nie gedruckt und nie in eine PDF-Datei übernommen.

Um ein Wort zu korrigieren, klicken Sie es mit der **rechten Maustaste** an. Das Menü zeigt diese Optionen oberhalb der üblichen Befehle:

- Bis zu sechs **Vorschläge**, der beste zuerst, fett gedruckt. Klicken Sie einen an, um ihn an die Stelle des Wortes zu setzen. Das ist ein Schritt, daher nimmt **Strg+Z** ihn zurück.
- **Alle ignorieren** lässt dieses Wort überall unverändert, bis Sie JeffWrite schließen.
- **Zum Wörterbuch hinzufügen** lernt das Wort dauerhaft. Es wird in einer kleinen Datei im eigenen JeffOffice-Ordner gespeichert, und jede JeffOffice-App verwendet sie. Wie Sie Ihre Wortliste ansehen oder ändern, lesen Sie unter „Eigene Wörter“ weiter unten.

Hat das Wörterbuch keinen Vorschlag, meldet das Menü das.

## Rechtschreibfehler durchgehen

Drücken Sie **F7**, oder klicken Sie auf **Überprüfen > Rechtschreibung**. Der Bereich „Rechtschreibung“ öffnet sich rechts. Er zeigt das erste falsch geschriebene Wort nach der Einfügemarke, im Text markiert, mit der Zeile, in der es steht, und einer Liste von Vorschlägen.

- **Ändern** setzt den gewählten Vorschlag an die Stelle des Wortes. Ein Doppelklick auf einen Vorschlag tut dasselbe.
- **Alle ändern** tut das für jede Kopie des Wortes an den Stellen, die der Bereich durchläuft (siehe unten). Das ist ein Schritt, daher nimmt **Strg+Z** alle Änderungen zurück, egal wo sie waren. Ist der Vorgang beendet, nennt der Bereich die Zahl der geänderten Wörter und die Zahl der Stellen. Der Text, jedes Textfeld, jede Fußnote und jede Kopf- oder Fußzeile zählen je als eine Stelle.
- **Ignorieren** lässt dieses Wort unverändert. Seine rote Linie bleibt.
- **Alle ignorieren** und **Zum Wörterbuch hinzufügen** funktionieren wie im Kontextmenü der rechten Maustaste.

Der Bereich beginnt an der Einfügemarke: im Text oder im Textfeld, in der Fußnote, Kopf- oder Fußzeile, die Sie gerade bearbeiten. Er geht zuerst den Text des Dokuments durch, Tabellen eingeschlossen, dann die Textfelder, dann die Fußnoten und zuletzt die Kopf- und Fußzeilen. Steht ein Wort in einem Textfeld, einer Fußnote, Kopf- oder Fußzeile, öffnet JeffWrite sie zur Bearbeitung, wie bei „Suchen“, damit Sie das Wort an seiner Stelle sehen. Eine Kopf- oder Fußzeile, die auf keiner Seite erscheint, wird übersprungen, und **Alle ändern** lässt sie ebenfalls unverändert. Von der Einfügemarke aus geht der Bereich bis zum Ende dieser Stelle, dann weiter durch die übrigen in dieser Reihenfolge und beginnt danach wieder am Anfang, bis zu der Stelle, an der er gestartet ist. **Ändern** ist ein Schritt, der rückgängig gemacht werden kann, und **Alle ändern** durchsucht jede Stelle, die der Bereich durchläuft, ebenfalls als ein Schritt. Ein Wort in einer Endnote können Sie ignorieren oder hinzufügen, aber nicht ändern. Sind keine weiteren Fehler vorhanden, meldet der Bereich, dass die Rechtschreibprüfung abgeschlossen ist. Ist die Rechtschreibung fertig, geht er zu den Grammatik- und Stilhinweisen über (siehe „Grammatik während der Eingabe prüfen“ weiter unten).

## Fehler in der Statusleiste zählen

Die Statusleiste zeigt, wie viele Rechtschreib- und Grammatikfehler das Dokument hat, zum Beispiel **3 Dokumentprüfungsfehler**. Klicken Sie darauf, um den nächsten Fehler nach der Einfügemarke auszuwählen. Beim letzten Fehler springt ein Klick zurück zum ersten.

JeffWrite zählt während der Arbeit, nach und nach, und prüft erneut nur die Absätze, die Sie ändern. In einem langen Dokument zeigt die Zahl ein Plus, zum Beispiel **120+ Dokumentprüfungsfehler**, bis jeder Absatz geprüft wurde. Ein Wort, das sowohl die Rechtschreibprüfung als auch die Grammatikprüfung markiert, wird zweimal gezählt. Grammatikfehler werden für die Absätze gezählt, die die Grammatikprüfung bereits untersucht hat, daher kann die Zahl beim Scrollen wachsen. Die Zahl ist ausgeblendet, wenn Rechtschreib- und Grammatikprüfung beide ausgeschaltet sind.

## Eigene Wörter

Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Benutzerdefiniertes Wörterbuch**, um Ihre Wörter anzusehen und zu ändern. Das Dialogfeld hat zwei Listen, jede mit einem Feld **Suchen**, mit dem Sie die Liste eingrenzen können:

- **Hinzugefügte Wörter** sind Wörter, die Sie JeffWrite beigebracht haben. Sie werden in jeder Sprache akzeptiert.
- **Ausgeschlossene Wörter** sind Wörter, die als falsch geschrieben markiert werden sollen, obwohl ein Wörterbuch sie akzeptiert, etwa ein Wort, das Sie immer falsch tippen. Sie werden überall markiert, in jeder Großschreibung, und nie als Vorschlag angeboten. Ausgeschlossen ist genau dieses Wort, nicht seine anderen Formen: Schließen Sie „Haus“ aus, wird „Häuser“ nicht markiert. Ein Wort zum Wörterbuch hinzuzufügen, im Bereich, im Kontextmenü oder im Dialogfeld, nimmt es von der Liste der ausgeschlossenen Wörter. **Alle ignorieren** wirkt bei einem ausgeschlossenen Wort weiterhin, bis Sie das Programm schließen.

Geben Sie ein Wort ein und klicken Sie auf **Hinzufügen**, oder wählen Sie Wörter aus und klicken Sie auf **Entfernen**. Ein Wort steht immer nur in einer Liste: Fügen Sie es einer Liste hinzu, verschwindet es aus der anderen. **Importieren** übernimmt die Wörter aus einer einfachen Textdatei mit einem Wort pro Zeile, **Exportieren** schreibt die Liste als Datei desselben Typs (UTF-8). Eine Zeile mit Leerzeichen oder Steuerzeichen oder mit mehr als 100 Buchstaben wird ausgelassen, und das Dialogfeld teilt Ihnen mit, wie viele es waren. Eine Liste darf bis zu 1 MB Text umfassen. Erst wenn Sie auf **OK** klicken, ändert sich etwas. Die Listen gehören Ihnen, sind in jeder JeffOffice-App gleich und werden nie in einem Dokument gespeichert.

## Wenn die AutoKorrektur ein Wort ändert

Ändert die AutoKorrektur beim Schreiben ein Wort, erscheint unter der Einfügemarke eine kleine Schaltfläche **AutoKorrektur-Optionen**. Sie bietet drei Möglichkeiten. Ein Bildschirmleser meldet beim Erscheinen der Schaltfläche, dass Optionen verfügbar sind, und die Beschreibung der Schaltfläche sagt, wie Sie sie öffnen. Sie können die Optionen auch mit **Alt+Umschalt+F10** öffnen. Es handelt sich um den Befehl **Menü der AutoKorrektur-Optionen**. Er steht auf der Seite „Tastatur“ der Optionen zusammen mit den anderen Befehlen, und dort können Sie ihm eine andere Taste zuweisen.

- **Automatische Korrektur rückgängig machen** stellt das Wort so wieder her, wie Sie es eingegeben haben. Das ist ein Schritt, daher macht **Wiederholen** die Änderung erneut.
- Eine Option, die die Änderung unterbindet. Was genau, hängt davon ab, was das Wort geändert hat: **Ersetzung beenden** nimmt den Eintrag aus Ihrer Ersetzungsliste, **Wie eingegeben beibehalten** fügt ein Wort mit zwei Großbuchstaben („THursday“) zu den Wörtern hinzu, die diese behalten, und **Großschreibung beenden** schaltet die Großschreibung für Wochentagsnamen, den ersten Buchstaben von Sätzen oder den ersten Buchstaben von Tabellenzellen überall aus, nicht nur für dieses Wort, und sagt das auch. Es ist derselbe Schalter wie im Dialogfeld „AutoKorrektur“. Außerdem wird das Wort so wiederhergestellt, wie Sie es eingegeben haben.
- **AutoKorrektur-Optionen...** öffnet das Dialogfeld „AutoKorrektur“.

Die Registerkarte **Mathematische Symbole** im Dialogfeld „AutoKorrektur“ hat einen Schalter, **Namen mathematischer Symbole beim Schreiben ersetzen**, der ausgeschaltet bleibt, bis Sie ihn einschalten. Ist er eingeschaltet, wird ein Backslash mit einem Namen, etwa \alpha, zum Symbol (α), sobald Sie danach ein Leerzeichen oder ein Zeichen eingeben. Der Name muss am Wortanfang stehen, daher bleibt ein Ordnerpfad wie C:\temp\pi so, wie Sie ihn eingegeben haben. Die Registerkarte listet alle Namen auf. Wurde ein Name ersetzt, lautet die Auswahl der Schaltfläche **Ersetzung mathematischer Namen beenden**, und sie schaltet den Schalter aus. JeffDeck und JeffPub ändern sie ebenfalls; JeffCrunch ändert in einer Zelle nichts.

Die Schaltfläche verschwindet, sobald Sie schreiben, bearbeiten oder die Einfügemarke verschieben.

## Rote Linien ein- oder ausschalten

Klicken Sie auf **Überprüfen > Rechtschreibung während der Eingabe prüfen**. Dieselbe Auswahl finden Sie unter **Datei > Optionen > Dokumentprüfung**, und sie ist die eine Einstellung, die alle JeffOffice-Apps gemeinsam nutzen. Dort können Sie außerdem festlegen, dass Wörter in GROSSBUCHSTABEN und Wörter mit Zahlen wie 3rd übersprungen werden. Beide werden zunächst übersprungen.

## Grammatik während der Eingabe prüfen

JeffWrite sucht auch nach eindeutigen Grammatik- und Stilfehlern im Text. Ein Problem erhält eine blaue gepunktete Linie. Rechtschreibfehler behalten ihre rote Wellenlinie, und ein falsch geschriebenes Wort wird nie als Grammatikproblem markiert. Folgende Fehler sucht JeffWrite:

- ein wiederholtes Wort („the the“)
- „a“ und „an“ vor dem falschen Laut („an test“, „a hour“)
- ein Satz, der nach einem Punkt mit einem Kleinbuchstaben beginnt
- „its“ und „it's“, „their“ und „there“, „your“ und „you're“, „then“ und „than“ sowie „lose“ und „loose“, wenn die umgebenden Wörter den Fehler eindeutig machen
- „could of“ statt „could have“
- zwei Leerzeichen hintereinander innerhalb eines Satzes
- ein Leerzeichen vor einem Komma, einem Punkt oder einem anderen Zeichen, das zum vorangehenden Wort gehört
- kein Leerzeichen nach dem Satzende
- „alot“ sowie „everyday“ statt „every day“ („almost everyday“)
- Verben, die in eindeutigen Fällen nicht zum Subjekt passen („she don't“, „they was“, „it have“, „everyone are“, „there is many reasons“)
- die falsche Verbform („did went“, „has went“, „can't hardly“, „don't have no“)
- verwechselte Wörter („who's car“, „an affect on“, „less people“, „irregardless“, „can't breath“, „weather“ statt „whether“, „chose“ statt „choose“, „advise“ statt „advice“)
- „between you and I“, „Me and him went“, „more better“ und „these thing“
- ein Komma nach der Jahreszahl eines Datums („On June 5, 2020, we left“), ein Komma nach Ort und Bundesstaat („in Austin, Texas, the heat“) sowie ein Bindestrich in einem Compound vor einem Nomen („a well-known author“)
- ein kleines „i“ statt „I“ sowie Wochentage und Monate in Kleinschreibung

JeffWrite lässt lieber einen Fehler durchgehen, als richtigen Text zu markieren. Ein Satz ohne blaue Linie ist also kein Beleg dafür, dass er richtig ist.

Klicken Sie das Problem mit der rechten Maustaste an. Das Menü zeigt, was falsch ist, danach einen oder mehrere Vorschläge. Klicken Sie einen Vorschlag an, um ihn in den Text zu setzen, und drücken Sie Strg+Z, um ihn zurückzunehmen. **Dieses Problem ignorieren** entfernt die Linie nur an dieser einen Stelle, bis Sie JeffWrite schließen. Dieselben Wörter an einer anderen Stelle behalten ihre Linie, und Änderungen an anderer Stelle im Absatz bringen sie nicht zurück. **Alle ignorieren** entfernt die Linie bei jedem Problem mit dieser Meldung zu diesen Wörtern.

Die Prüfung läuft im Hintergrund auf Ihrem eigenen Computer, ohne Internetverbindung. Das Schreiben wird nie aufgehalten, und ein Absatz wird kurz nach dem Ende Ihrer Eingabe geprüft. Ein Absatz wird bis zu seinen ersten 100.000 Zeichen geprüft. Übersteigt ein Dokument dieses Limit, wird es nicht weiter geprüft, und die Statusleiste informiert Sie einmal darüber. Die Rechtschreibprüfung arbeitet weiter.

**Welche Sprachen geprüft werden.** Grammatik wird nur für Englisch geprüft, in allen regionalen Varianten (amerikanisch, britisch, kanadisch, australisch und die übrigen). Text in einer anderen Sprache wird nur auf Rechtschreibung geprüft und erhält keine Grammatikmarkierungen. Ein Absatz wird in der Sprache geprüft, als die er markiert ist; Text ohne Sprachangabe wird als Englisch geprüft. In einem Absatz mit mehreren Sprachen wird nur das Englische geprüft, der Rest wird übersprungen. Text, den eine Datei als „nicht prüfen“ kennzeichnet, wird ebenfalls übersprungen, ebenso Wörter, die eine nachverfolgte Änderung gelöscht hat, verborgener Text und die Ergebnisse von Feldern (etwa Seitenzahlen). Grammatik wird im Text des Dokuments geprüft, noch nicht in Tabellen, Kopf- und Fußzeilen oder Fußnoten.

Zum Ausschalten klicken Sie auf **Überprüfen > Grammatik während der Eingabe prüfen**. Dieselbe Auswahl finden Sie unter **Datei > Optionen > Dokumentprüfung**. Sie gilt für alle JeffOffice-Apps und in jedem Fenster gleichermaßen. Ist die Grammatikprüfung eingeschaltet, geht der Bereich „Rechtschreibung“ (F7) nach der Rechtschreibung auch die Grammatikprobleme durch.

**Auswählen, wonach gesucht wird.** Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Grammatikeinstellungen**, um die Arten von Problemen festzulegen: **Grammatik** (Verben, die nicht passen, falsche Verbformen, „a“ und „an“), **Interpunktion** (überflüssige oder fehlende Leerzeichen, Kommas und Bindestriche), **Groß- und Kleinschreibung**, **Verwechselte Wörter** (gleich klingende Wörter), **Wiederholungen** und **Stil**. **Stil** ist ausgeschaltet, bis Sie es einschalten. Es macht Vorschläge und betrifft keine Fehler: Sätze mit mehr als 40 Wörtern, das Passiv, drei Sätze in Folge, die mit demselben Wort beginnen, und umständliche Wendungen („in order to“, „due to the fact that“). Die Auswahl bleibt erhalten, wenn Sie in den Optionen auf **OK** klicken. Sie wirkt sofort auf die blauen Linien und den Bereich „Rechtschreibung“ (F7) in den geöffneten Fenstern dieser App und bei den anderen JeffOffice-Apps beim nächsten Start.

## Sprachen

JeffWrite prüft jeden Textabschnitt in seiner eigenen Sprache. Eine Datei kann angeben, in welcher Sprache ihr Text steht, bis hinunter zu einem einzelnen Wort, und JeffWrite liest das. Text ohne Angabe wird in der Sprache des Dokuments geprüft. Die Datei gibt diese Sprache für das ganze Dokument an, und ein neues Dokument verwendet amerikanisches Englisch.

Text in einer Sprache, für die JeffWrite kein Wörterbuch hat, wird nicht geprüft. Ebenso wenig Text, den eine Datei als „Rechtschreibung nicht prüfen“ kennzeichnet. Die Sprache von Text können Sie in JeffWrite noch nicht ändern, aber er behält die Angabe der Datei.

Die 31 Wörterbücher sind: Englisch (amerikanisch, britisch, kanadisch und australisch), Spanisch (21 Regionen, darunter Mexiko und Spanien), Französisch, Deutsch, Italienisch, Niederländisch sowie Portugiesisch (Brasilien und Portugal).

## Was noch nicht geprüft wird

Rote Linien erscheinen im Text, in den Zellen von Tabellen sowie in einer Kopf-, Fußzeile oder Fußnote, während Sie sie bearbeiten. In Textfeldern erscheinen sie noch nicht, aber der Bereich „Rechtschreibung“ geht sie durch.

## Siehe auch

- [Willkommen bei JeffWrite](start)
- [Drucken](print)
