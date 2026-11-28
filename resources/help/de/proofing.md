# Rechtschreibung, Thesaurus, Recherche und Silbentrennung
<!-- keywords: Rechtschreibprüfung, Rechtschreibung, Schreibprüfung, Thesaurus, Synonyme, Recherche, Wörterbuch, Dokumentprüfungssprache, Silbentrennung, trennen, übersetzen, AutoKorrektur, falsch geschrieben, rote Wellenlinie, Sprache, Grammatik, Grammatikprüfung, Stil, blaue Linie, gepunktete Linie, Grammatik während der Eingabe prüfen -->

Auf der Registerkarte **Überprüfen** finden Sie Werkzeuge, mit denen Sie Ihre Wörter richtig schreiben.

## Rechtschreibung während der Eingabe prüfen

JeffPub setzt eine rote Wellenlinie unter ein Wort, das es nicht kennt. Klicken Sie mit der rechten Maustaste auf das Wort, um einen Vorschlag auszuwählen, **Alle ignorieren** (für diese Sitzung) zu wählen oder **Zum Wörterbuch hinzufügen** zu wählen.

Zum Ausschalten klicken Sie auf den Pfeil neben **Überprüfen > Dokumentprüfung > Rechtschreibung** und dann auf **Rechtschreibung während der Eingabe prüfen**. Dieselbe Einstellung finden Sie unter **Datei > Optionen > Dokumentprüfung**. Sie gilt einheitlich für alle JeffOffice-Apps und alle Fenster, und JeffPub merkt sie sich für den nächsten Start.

## Eigene Wörter

Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Benutzerwörterbuch**, um die von Ihnen hinzugefügten Wörter zu sehen und sie hinzuzufügen, zu entfernen, zu suchen, zu importieren oder zu exportieren. Im selben Dialog gibt es eine Liste **Ausgeschlossene Wörter**: Wörter, die immer als falsch geschrieben markiert werden, auch wenn ein Wörterbuch sie akzeptiert, und nie als Vorschlag angeboten werden. Ausgeschlossen ist nur genau dieses Wort, nicht seine anderen Formen. Wenn Sie ein Wort zum Wörterbuch hinzufügen, wird es von der Liste der ausgeschlossenen Wörter entfernt. Für Import und Export verwenden Sie eine einfache Textdatei mit einem Wort pro Zeile (UTF-8). Ihre Listen sind in allen JeffOffice-Apps gleich und werden nie in einem Dokument gespeichert.

## Grammatik während der Eingabe prüfen

JeffPub sucht auch nach eindeutigen Grammatik- und Stilfehlern in Text, in Textfeldern, Formen und Tabellen. Ein Problem wird mit einer blauen gepunkteten Linie markiert, ein falsch geschriebenes Wort behält seine rote Wellenlinie. Klicken Sie mit der rechten Maustaste auf das Problem, um zu sehen, was falsch ist, und einen Vorschlag auszuwählen, oder wählen Sie **Dieses Problem ignorieren** oder **Alle ignorieren**. Text in einer nicht geprüften Sprache und Text, der als „nicht prüfen“ markiert ist, wird übersprungen. Zum Ausschalten klicken Sie auf **Datei > Optionen > Dokumentprüfung** und deaktivieren **Grammatik während der Eingabe prüfen**. Diese Einstellung gilt einheitlich für alle JeffOffice-Apps.

**Welche Sprachen geprüft werden.** Die Grammatik wird nur für Englisch geprüft, in allen regionalen Varianten (amerikanisch, britisch, kanadisch, australisch und die übrigen). Text in anderen Sprachen wird nur auf Rechtschreibung geprüft und erhält keine Grammatikmarkierungen. Ein Absatz wird in der Sprache geprüft, als die er markiert ist. Text ohne Sprachangabe wird als Englisch geprüft, es sei denn, die meisten seiner Wörter sind keine englischen Wörter, etwa bei einem nicht gekennzeichneten Abschnitt in einer anderen Sprache. Zusätzliche Leerzeichen und Leerzeichen vor Satzzeichen werden nur in Sätzen geprüft, nicht in Überschriften, Beschriftungen, Namenslisten, Tabellenzellen, die keine ganzen Sätze sind, oder Quellenangaben. In einem Absatz mit mehreren Sprachen wird nur das Englische geprüft, der Rest wird übersprungen. Text, den eine Datei als „nicht prüfen“ markiert, wird ebenfalls übersprungen.

**Auswählen, wonach gesucht wird.** Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Grammatikeinstellungen**, um die Problemarten auszuwählen: **Grammatik** (nicht übereinstimmende Verben, falsche Verbform, „a“ und „an“), **Satzzeichen** (zusätzliche oder fehlende Leerzeichen, Kommas und Bindestriche), **Groß- und Kleinschreibung**, **Verwechselte Wörter** (gleich klingende Wörter), **Wiederholung** und **Stil**. Die Option **Stil** ist ausgeschaltet, bis Sie sie einschalten. Sie macht Vorschläge und weist nicht auf Fehler hin: Sätze mit mehr als 40 Wörtern, das Passiv, drei aufeinanderfolgende Sätze, die mit demselben Wort beginnen, und umständliche Wendungen („in order to“, „due to the fact that“). Die Auswahl bleibt erhalten, wenn Sie in den Optionen auf **OK** klicken. Sie wirkt sofort auf die blauen Linien und den Bereich Rechtschreibung (F7) in den geöffneten Fenstern dieser App, und auf die anderen JeffOffice-Apps beim nächsten Start.

## Ganze Publikation prüfen

1. Klicken Sie auf **Überprüfen > Dokumentprüfung > Rechtschreibung** oder drücken Sie F7.
2. Der Dialog **Rechtschreibung prüfen** zeigt das erste problematische Wort unter **Nicht im Wörterbuch**, darunter **Vorschläge**.
3. Wählen Sie einen Vorschlag aus oder geben Sie unter **Ändern in** Ihren eigenen Text ein.
4. Klicken Sie auf **Ändern** für dieses Wort oder auf **Alle ändern** für alle Treffer. Mit **Ignorieren** überspringen Sie das Wort, mit **Alle ignorieren** überspringen Sie es für diese Sitzung, und mit **Hinzufügen** speichern Sie es in Ihrer eigenen Wortliste.

JeffPub prüft jedes Textfeld, jede Form und jede Tabelle, beginnend auf der ersten Seite. Ist **Grammatik während der Eingabe prüfen** eingeschaltet, geht die Prüfung nach der Rechtschreibung weiter zu den Grammatikproblemen und zeigt statt **Nicht im Wörterbuch** an, was falsch ist. Am Ende meldet JeffPub, dass die Prüfung abgeschlossen ist. Wörter in GROSSBUCHSTABEN und Wörter mit Zahlen werden übersprungen. Sie können dies auf der Registerkarte **Dokumentprüfung** unter **Datei > Optionen** (Fenster **JeffPub-Optionen**) ändern.

## Sprache festlegen

Rechtschreibung und automatische Silbentrennung richten sich nach der Sprache des Textes. Text ohne festgelegte Sprache wird als Englisch (USA) geprüft. JeffPub hat Wörterbücher für Englisch (USA, Großbritannien, Kanada und Australien), Spanisch (21 Regionen, von Argentinien bis Venezuela, einschließlich Mexiko und Spanien), Französisch, Deutsch, Italienisch, Niederländisch und Portugiesisch (Brasilien und Portugal).

1. Markieren Sie den Text.
2. Klicken Sie auf **Überprüfen > Sprache > Dokumentprüfungssprache festlegen…**
3. Wählen Sie eine Sprache. Wählen Sie **Rechtschreibung nicht prüfen** für Text wie Namen oder Codes.

## Ein besseres Wort finden

- **Thesaurus:** Klicken Sie in ein Wort und dann auf **Überprüfen > Dokumentprüfung > Thesaurus** (Umschalt+F7). Wählen Sie ein Synonym, also ein Wort mit ähnlicher Bedeutung. Klicken Sie auf **Einfügen**, um Ihr Wort zu ersetzen. **Nachschlagen** sucht nach dem gewählten Synonym.
- **Recherche:** Klicken Sie auf **Überprüfen > Dokumentprüfung > Recherche** (Alt+F7), um den Bereich **Recherche** zu öffnen. Geben Sie ein Wort unter **Suchen nach** ein und klicken Sie auf **Nachschlagen**.

Beide nutzen einen in JeffPub eingebauten Thesaurus, nur für Englisch. Nichts, was Sie eingeben, wird online gesendet.

## Silbentrennung

Die Silbentrennung teilt lange Wörter am Zeilenende mit einem Bindestrich, damit die Zeilen gleichmäßig aussehen. Neue Textfelder trennen standardmäßig Silben. Um das für neue Textfelder zu ändern, siehe **Automatisch in neuen Textfeldern silbentrennen** auf der Registerkarte **Erweitert** unter **Datei > Optionen** (Fenster **JeffPub-Optionen**).

Um einen einzelnen Textabschnitt zu ändern, klicken Sie in sein Textfeld und dann auf **Überprüfen > Prüfen > Silbentrennung…** (Strg+Umschalt+H). Sie können auch **Textfeld > Text > Silbentrennung** verwenden. Im Dialog **Silbentrennung**:

- **Diesen Textabschnitt automatisch trennen** schaltet die Silbentrennung ein oder aus.
- **Trennzone** legt fest, wie viel freier Platz am Zeilenende höchstens bleiben darf, bevor JeffPub stattdessen ein Wort trennt.
- **Bedingten Trennstrich einfügen** fügt an der Einfügemarke einen Trennstrich ein. Er wird nur angezeigt, wenn das Wort dort umbricht.
- **Bedingte Trennstriche hinzufügen** fügt an jeder Stelle, an der ein langes Wort umbrechen kann, einen ein.

## Text übersetzen

Markieren Sie Text und klicken Sie auf **Überprüfen > Sprache > Übersetzen**. Der Bereich Übersetzen übersetzt ihn auf Ihrem Computer und kann den Text ersetzen oder die Übersetzung darunter einfügen. Siehe [Text übersetzen](translate).

## Siehe auch

- [Text suchen und ersetzen](find-replace)
- [Die Registerkarte Überprüfen](tab-review)
- [Textfelder hinzufügen und formatieren](text-boxes)
