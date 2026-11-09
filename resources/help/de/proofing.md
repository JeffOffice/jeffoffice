# Rechtschreibung, Thesaurus, Recherche und Silbentrennung
<!-- keywords: Rechtschreibprüfung, Rechtschreibung, Rechtschreibung prüfen, Thesaurus, Synonyme, Recherche, Wörterbuch, Dokumentprüfungssprache, Silbentrennung, trennen, übersetzen, AutoKorrektur, falsch geschrieben, rote Wellenlinie, Sprache -->

Auf der Registerkarte **Überprüfen** finden Sie Werkzeuge, mit denen Sie Ihre Wörter richtig schreiben.

## Rechtschreibung während der Eingabe prüfen

JeffPub setzt eine rote Wellenlinie unter ein Wort, das es nicht kennt. Klicken Sie mit der rechten Maustaste auf das Wort, um einen Vorschlag auszuwählen, **Alle ignorieren** (für diese Sitzung) oder **Zum Wörterbuch hinzufügen** zu wählen.

Um diese Funktion auszuschalten, klicken Sie auf den Pfeil neben **Überprüfen > Dokumentprüfung > Rechtschreibung** und anschließend auf **Rechtschreibung während der Eingabe prüfen**.

## Die gesamte Publikation prüfen

1. Klicken Sie auf **Überprüfen > Dokumentprüfung > Rechtschreibung** oder drücken Sie F7.
2. Das Dialogfeld **Rechtschreibung** zeigt das erste problematische Wort unter **Nicht im Wörterbuch** an, darunter die **Vorschläge**.
3. Wählen Sie einen Vorschlag aus oder geben Sie unter **Ersetzen durch** Ihre eigene Schreibweise ein.
4. Klicken Sie auf **Ändern**, um dieses Wort zu ersetzen, oder auf **Alle ändern**, um alle Treffer zu ersetzen. Mit **Ignorieren** überspringen Sie das Wort, mit **Alle ignorieren** überspringen Sie es für diese Sitzung, und mit **Hinzufügen** speichern Sie es in Ihrer eigenen Wortliste.

JeffPub prüft jedes Textfeld, jede Form und jede Tabelle, beginnend auf der ersten Seite. Am Ende meldet das Programm, dass die Prüfung abgeschlossen ist. Wörter in GROSSBUCHSTABEN und Wörter mit Zahlen werden übersprungen. Sie können dies auf der Registerkarte **Dokumentprüfung** unter **Datei > Optionen** (das Fenster **JeffPub-Optionen**) ändern.

## Sprache festlegen

Rechtschreibung und automatische Silbentrennung richten sich nach der Sprache des Textes. Text ohne eingestellte Sprache wird als Englisch (USA) geprüft. JeffPub hat Wörterbücher für Englisch (USA, Vereinigtes Königreich, Kanada und Australien), Spanisch (21 Regionen, von Argentinien bis Venezuela, einschließlich Mexiko und Spanien), Französisch, Deutsch, Italienisch, Niederländisch sowie Portugiesisch (Brasilien und Portugal).

1. Markieren Sie den Text.
2. Klicken Sie auf **Überprüfen > Sprache > Sprache für die Dokumentprüfung festlegen…**
3. Wählen Sie eine Sprache. Für Text wie Namen oder Codes wählen Sie **Rechtschreibung nicht prüfen**.

## Ein besseres Wort finden

- **Thesaurus:** Klicken Sie in ein Wort und dann auf **Überprüfen > Dokumentprüfung > Thesaurus** (Umschalt+F7). Wählen Sie ein Synonym, also ein Wort mit ähnlicher Bedeutung. Klicken Sie auf **Einfügen**, um Ihr Wort zu ersetzen. Mit **Nachschlagen** suchen Sie nach dem ausgewählten Synonym.
- **Recherche:** Klicken Sie auf **Überprüfen > Dokumentprüfung > Recherche** (Alt+F7), um den Bereich **Recherche** zu öffnen. Geben Sie ein Wort unter **Suchen nach** ein und klicken Sie auf **Nachschlagen**.

Beide nutzen einen in JeffPub eingebauten Thesaurus, nur auf Englisch. Was Sie eingeben, wird nicht online übertragen.

## Silbentrennung

Die Silbentrennung teilt lange Wörter am Zeilenende mit einem Bindestrich, damit die Zeilen gleichmäßig aussehen. Neue Textfelder werden standardmäßig automatisch getrennt. Um das für neue Textfelder zu ändern, lesen Sie den Eintrag **Automatische Silbentrennung in neuen Textfeldern** auf der Registerkarte **Erweitert** unter **Datei > Optionen** (das Fenster **JeffPub-Optionen**).

Um einen einzelnen Textabschnitt zu ändern, klicken Sie in sein Textfeld und dann auf **Überprüfen > Prüfen > Silbentrennung…** (Strg+Umschalt+H). Alternativ können Sie **Textfeld > Text > Silbentrennung** verwenden. Im Dialogfeld **Silbentrennung** gilt:

- **Automatische Silbentrennung für diesen Textabschnitt** schaltet sie ein oder aus.
- **Silbentrennzone** legt fest, wie viel leerer Platz am Zeilenende höchstens bleiben darf, bevor JeffPub stattdessen ein Wort trennt.
- **Bedingten Trennstrich einfügen** setzt einen Trennstrich an die Einfügemarke. Er wird nur angezeigt, wenn das Wort dort tatsächlich getrennt wird.
- **Bedingte Trennstriche hinzufügen** setzt an jeder Stelle, an der ein langes Wort getrennt werden kann, einen Trennstrich.

## Text übersetzen

Markieren Sie einen Text und klicken Sie auf **Überprüfen > Sprache > Übersetzen**. Wählen Sie eine Sprache und klicken Sie auf **OK**. JeffPub öffnet dann Ihren Webbrowser mit LibreTranslate, einer kostenlosen Übersetzungsseite mit offenem Quellcode, und übergibt den Text. Da der Text an diese Seite gesendet wird, verwenden Sie die Funktion nicht für private Inhalte.

## Siehe auch

- [Text suchen und ersetzen](find-replace)
- [Die Registerkarte Überprüfen](tab-review)
- [Textfelder hinzufügen und formatieren](text-boxes)
