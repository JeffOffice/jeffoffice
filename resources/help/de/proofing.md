# Rechtschreibung, Thesaurus, Recherche und Silbentrennung
<!-- keywords: Rechtschreibprüfung, Rechtschreibung, Rechtschreibung prüfen, Thesaurus, Synonyme, Recherche, Wörterbuch, Dokumentprüfungssprache, Dokumentprüfung, Silbentrennung, trennen, übersetzen, AutoKorrektur, falsch geschrieben, rote Wellenlinie, Sprache, Grammatik, Grammatikprüfung, Stil, blaue Linie, gepunktete Linie, Grammatik während der Eingabe prüfen -->

Auf der Registerkarte **Überprüfen** finden Sie Werkzeuge, mit denen Sie Ihre Wörter richtig schreiben.

## Rechtschreibung während der Eingabe prüfen

JeffPub setzt eine rote Wellenlinie unter ein Wort, das es nicht kennt. Klicken Sie mit der rechten Maustaste auf das Wort, um einen Vorschlag auszuwählen, **Alle ignorieren** (für diese Sitzung) oder **Zum Wörterbuch hinzufügen** zu wählen.

Um dies auszuschalten, klicken Sie auf den Pfeil neben **Überprüfen > Dokumentprüfung > Rechtschreibung** und klicken Sie auf **Rechtschreibung während der Eingabe prüfen**. Dieselbe Einstellung finden Sie unter **Datei > Optionen > Dokumentprüfung**. Sie gilt einheitlich für alle JeffOffice-Apps und alle Fenster, und JeffPub merkt sie sich für den nächsten Start.

## Eigene Wörter

Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Benutzerwörterbuch**, um die von Ihnen hinzugefügten Wörter zu sehen und sie hinzuzufügen, zu entfernen, zu suchen, zu importieren oder zu exportieren. Im selben Dialogfeld gibt es eine Liste **Ausgeschlossene Wörter**: Wörter, die immer als falsch geschrieben markiert werden, auch wenn ein Wörterbuch sie akzeptiert, und niemals als Vorschlag angeboten werden. Ein ausgeschlossenes Wort betrifft nur genau dieses Wort, nicht seine anderen Formen. Wenn Sie ein Wort zum Wörterbuch hinzufügen, wird es von der Liste der ausgeschlossenen Wörter entfernt. Für Import und Export wird eine reine Textdatei mit einem Wort pro Zeile (UTF-8) verwendet. Ihre Listen sind in allen JeffOffice-Apps gleich und werden nie in einem Dokument gespeichert.

## Grammatik während der Eingabe prüfen

JeffPub sucht auch nach eindeutigen Grammatik- und Stilfehlern in Text, in Textfeldern, Formen und Tabellen. Ein Problem erhält eine blaue gepunktete Linie, und ein falsch geschriebenes Wort behält seine rote Wellenlinie. Klicken Sie mit der rechten Maustaste auf das Problem, um zu sehen, was falsch ist, und einen Vorschlag auszuwählen, oder wählen Sie **Dieses Problem ignorieren** oder **Alle ignorieren**. Text in einer Sprache, die nicht geprüft wird, und Text, der als „nicht prüfen“ markiert ist, wird übersprungen. Um dies auszuschalten, klicken Sie auf **Datei > Optionen > Dokumentprüfung** und deaktivieren Sie **Grammatik während der Eingabe prüfen**. Diese Einstellung gilt einheitlich für alle JeffOffice-Apps.

**Welche Sprachen geprüft werden.** Englisch, Französisch, Deutsch und Spanisch, jeweils mit eigenen Regeln. Französisch achtet auf den geschützten Leerschritt vor ; : ! ? und innerhalb von « », auf den Apostroph in Wörtern wie „l'ami“ und „qu'il“ sowie auf Verwechslungen wie „a“ und „à“. Deutsch achtet auf „das“ und „dass“, „seid“ und „seit“, „wider“ und „wieder“, das Komma vor „dass“ und „weil“ sowie auf große Anfangsbuchstaben bei gängigen Substantiven. Spanisch sucht nach fehlendem ¿ oder ¡, Akzenten bei Fragewörtern, „a ver“ und „haber“ sowie „del“ und „al“. Ein Absatz wird in der Sprache geprüft, mit der er markiert ist; Text ohne Sprachangabe wird als Englisch geprüft. In einem Absatz mit mehreren Sprachen wird die erste geprüfte Sprache verwendet, der Rest wird übersprungen.

**Auswählen, wonach gesucht wird.** Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Grammatikeinstellungen**, um die Arten von Problemen auszuwählen: **Grammatik** (Verben, die nicht übereinstimmen, falsche Verbform, „a“ und „an“), **Zeichensetzung** (überflüssige oder fehlende Leerzeichen, Kommas und Bindestriche), **Groß- und Kleinschreibung**, **Verwechselte Wörter** (Wörter, die gleich klingen), **Wiederholungen** und **Stil**. **Stil** ist ausgeschaltet, bis Sie es einschalten. Es macht Vorschläge und betrifft keine Fehler: Sätze mit mehr als 40 Wörtern, das Passiv, drei aufeinanderfolgende Sätze, die mit demselben Wort beginnen, und umständliche Wendungen („in order to“, „due to the fact that“). Die Auswahl bleibt erhalten, wenn Sie in den Optionen auf **OK** klicken. Sie wirkt sofort auf die blauen Linien und auf den Bereich Rechtschreibung (F7) in den geöffneten Fenstern dieser App und bei den anderen JeffOffice-Apps beim nächsten Start.

## Das ganze Dokument prüfen

1. Klicken Sie auf **Überprüfen > Dokumentprüfung > Rechtschreibung** oder drücken Sie F7.
2. Das Dialogfeld **Rechtschreibung** zeigt das erste problematische Wort unter **Nicht im Wörterbuch**, darunter die **Vorschläge**.
3. Wählen Sie einen Vorschlag aus oder geben Sie unter **Ändern in** Ihren eigenen Text ein.
4. Klicken Sie auf **Ändern** für dieses Wort oder auf **Alle ändern** für alle Treffer. Mit **Ignorieren** überspringen Sie das Wort, mit **Alle ignorieren** überspringen Sie es für diese Sitzung, mit **Hinzufügen** speichern Sie es in Ihrer eigenen Wortliste.

JeffPub prüft jedes Textfeld, jede Form und jede Tabelle, beginnend auf der ersten Seite. Ist **Grammatik während der Eingabe prüfen** eingeschaltet, geht die Prüfung nach der Rechtschreibung weiter zu den Grammatikproblemen und zeigt statt **Nicht im Wörterbuch** an, was falsch ist. Am Ende wird eine Meldung angezeigt. Wörter in GROSSBUCHSTABEN und Wörter mit Zahlen werden übersprungen. Sie können dies auf der Registerkarte **Dokumentprüfung** unter **Datei > Optionen** (Dialogfeld **JeffPub-Optionen**) ändern.

## Sprache festlegen

Rechtschreibung und automatische Silbentrennung richten sich nach der Sprache des Textes. Text ohne eingestellte Sprache wird als Englisch (USA) geprüft. JeffPub hat Wörterbücher für Englisch (USA, Vereinigtes Königreich, Kanada und Australien), Spanisch (21 Regionen, von Argentinien bis Venezuela, einschließlich Mexiko und Spanien), Französisch, Deutsch, Italienisch, Niederländisch und Portugiesisch (Brasilien und Portugal).

1. Markieren Sie den Text.
2. Klicken Sie auf **Überprüfen > Sprache > Sprache für die Dokumentprüfung festlegen…**
3. Wählen Sie eine Sprache aus. Wählen Sie **Rechtschreibung nicht prüfen** für Text wie Namen oder Codes.

## Ein besseres Wort finden

- **Thesaurus:** Klicken Sie in ein Wort und klicken Sie auf **Überprüfen > Dokumentprüfung > Thesaurus** (Umschalt+F7). Wählen Sie ein Synonym aus, also ein Wort mit ähnlicher Bedeutung. Klicken Sie auf **Einfügen**, um Ihr Wort zu ersetzen. **Nachschlagen** sucht nach dem Synonym, das Sie ausgewählt haben.
- **Recherche:** Klicken Sie auf **Überprüfen > Dokumentprüfung > Recherche** (Alt+F7), um den Bereich **Recherche** zu öffnen. Geben Sie ein Wort unter **Suchen nach** ein und klicken Sie auf **Nachschlagen**.

Beide nutzen einen in JeffPub eingebauten Thesaurus, der nur Englisch unterstützt. Nichts, was Sie eingeben, wird online gesendet.

## Silbentrennung

Die Silbentrennung teilt lange Wörter am Zeilenende mit einem Bindestrich, damit die Zeilen gleichmäßig aussehen. Neue Textfelder trennen standardmäßig Silben. Um das für neue Felder zu ändern, siehe **Automatisch trennen in neuen Textfeldern** auf der Registerkarte **Erweitert** unter **Datei > Optionen** (Dialogfeld **JeffPub-Optionen**).

Um einen einzelnen Textabschnitt zu ändern, klicken Sie in sein Textfeld und klicken Sie auf **Überprüfen > Prüfen > Silbentrennung…** (Strg+Umschalt+H). Sie können auch **Textfeld > Text > Silbentrennung** verwenden. Im Dialogfeld **Silbentrennung**:

- **Diesen Textabschnitt automatisch trennen** schaltet die Silbentrennung ein oder aus.
- **Silbentrennzone** legt fest, wie viel freier Platz am Zeilenende höchstens bleiben darf, bevor JeffPub stattdessen ein Wort trennt.
- **Bedingten Trennstrich einfügen** fügt an der Einfügemarke einen Trennstrich ein. Er wird nur angezeigt, wenn das Wort dort getrennt wird.
- **Bedingte Trennstriche hinzufügen** fügt an jeder Stelle, an der ein langes Wort getrennt werden kann, einen ein.

## Text übersetzen

Markieren Sie Text und klicken Sie auf **Überprüfen > Sprache > Übersetzen**. Der Bereich Übersetzen übersetzt ihn auf Ihrem Computer und kann den Text ersetzen oder die Übersetzung darunter einfügen. Siehe [Text übersetzen](translate).

## Siehe auch

- [Text suchen und ersetzen](find-replace)
- [Die Registerkarte Überprüfen](tab-review)
- [Textfelder hinzufügen und formatieren](text-boxes)
