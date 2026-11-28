# Überprüfen-Registerkarte
<!-- keywords: Überprüfen-Registerkarte, vergleichen, Vergleich, Überprüfungsbereich, annehmen, ablehnen, Menüband, Dokumentprüfung, Rechtschreibung, Thesaurus, Wortanzahl, Wörter zählen, Kommentare, neuer Kommentar, Kommentar löschen, vorheriger, nächster, Kommentare anzeigen, Sprache, Sprache der Dokumentprüfung, Sprache festlegen, Rechtschreibung nicht prüfen, übersetzen, Übersetzung, LibreTranslate, Grammatik, Grammatikprüfung, Stil, blaue Linie, gepunktete Linie, Grammatik während der Eingabe prüfen -->

Die Registerkarte **Überprüfen** prüft Ihre Wörter, übersetzt sie und verwaltet Kommentare.

- **Dokumentprüfung** enthält **Rechtschreibung** (F7), **Thesaurus** (Umschalt+F7), **Wörter zählen** und **Barrierefreiheit prüfen**. Siehe [Text formatieren](text).
- **Sprache** enthält **Übersetzen**, **Sprache der Dokumentprüfung festlegen** und **Rechtschreibung nicht prüfen**. Siehe unten.
- **Vergleichen** enthält **Vergleichen**, **Annehmen**, **Ablehnen**, **Vorherige**, **Nächste** und **Überprüfungsbereich**. Siehe [Präsentationen vergleichen](compare).
- **Kommentare** enthält **Neuer Kommentar** (Strg+Alt+M), **Löschen** mit seinem Menü, **Vorheriger**, **Nächster** und **Kommentare anzeigen**. Siehe [Kommentare](comments).

## Grammatik während der Eingabe prüfen

JeffDeck sucht in Texten auch nach eindeutigen Grammatik- und Stilfehlern, auf Folien und in den Sprechernotizen. Ein Problem erhält eine blaue gepunktete Linie, und ein falsch geschriebenes Wort behält seine rote Wellenlinie. Klicken Sie das Problem mit der rechten Maustaste an, um zu sehen, was falsch ist, und einen Vorschlag auszuwählen, oder wählen Sie **Dieses Problem ignorieren** oder **Alle ignorieren**. **Rechtschreibung** (F7) prüft nach der Rechtschreibung auch die Grammatikprobleme auf den Folien. Text in einer Sprache, die nicht geprüft wird, und Text, der als „nicht prüfen“ markiert ist, wird übersprungen. Um die Prüfung auszuschalten, klicken Sie auf **Datei > Optionen > Dokumentprüfung** und deaktivieren Sie **Grammatik während der Eingabe prüfen**. Diese Einstellung gilt für alle JeffOffice-Apps.

**Welche Sprachen geprüft werden.** Grammatik wird nur für Englisch geprüft, und zwar in allen regionalen Varianten (amerikanisch, britisch, kanadisch, australisch und weitere). Text in anderen Sprachen wird nur auf Rechtschreibung geprüft und erhält keine Grammatikmarkierungen. Ein Absatz wird in der Sprache geprüft, als die er markiert ist, und Text ohne Sprachangabe wird als Englisch geprüft. In einem Absatz mit mehreren Sprachen wird nur das Englische geprüft, der Rest wird übersprungen. Text, den eine Datei als „nicht prüfen“ markiert, wird ebenfalls übersprungen.

**Wählen Sie, wonach gesucht wird.** Klicken Sie auf **Datei > Optionen > Dokumentprüfung > Grammatikeinstellungen**, um die Arten von Problemen festzulegen: **Grammatik** (nicht übereinstimmende Verben, falsche Verbform, „a“ und „an“), **Zeichensetzung** (überflüssige oder fehlende Leerzeichen, Kommas und Bindestriche), **Groß- und Kleinschreibung**, **Verwechselte Wörter** (Wörter, die gleich klingen), **Wiederholung** und **Stil**. **Stil** ist ausgeschaltet, bis Sie es einschalten. Er macht Vorschläge und betrifft keine Fehler: Sätze mit mehr als 40 Wörtern, das Passiv, drei aufeinanderfolgende Sätze, die mit demselben Wort beginnen, und umständliche Wendungen („in order to“, „due to the fact that“). Die Auswahl bleibt erhalten, wenn Sie in den Optionen auf **OK** klicken. Sie wirkt sofort auf die blauen Linien und den Bereich Rechtschreibung (F7) in den geöffneten Fenstern dieser App und bei den übrigen JeffOffice-Apps beim nächsten Start.

## Sprache eines Textes festlegen

Die Rechtschreibprüfung prüft jeden Textabschnitt in seiner eigenen Sprache. Um die Sprache einiger Wörter zu ändern, markieren Sie sie oder markieren Sie ein ganzes Textfeld oder einen Platzhalter, und klicken Sie in der Gruppe **Sprache** auf **Sprache der Dokumentprüfung festlegen**. Wählen Sie die Sprache in der Liste aus, und klicken Sie dann auf **OK**. Wörter in dieser Sprache werden mit dem Wörterbuch dieser Sprache geprüft, und die Datei behält diese Auswahl, wenn Sie sie als Präsentation oder als OpenDocument-Präsentation speichern. Text, den Sie nicht markiert haben, wird in amerikanischem Englisch geprüft.

## Einen Text nicht prüfen

Manche Texte sollen nicht geprüft werden, etwa Code, ein Name oder eine Zeile in einer erfundenen Sprache. Markieren Sie ihn und klicken Sie in der Gruppe **Sprache** auf **Rechtschreibung nicht prüfen**. Die roten Wellenlinien verschwinden. Klicken Sie erneut darauf, um den Text wieder zu prüfen. Das Notizfeld enthält nur einfachen Text und hat daher keine eigene Sprache.

## Text übersetzen

1. Markieren Sie die Wörter in einem Textfeld oder in den Notizen. Um den gesamten Text eines Textfelds zu übersetzen, markieren Sie das Feld.
2. Klicken Sie in der Gruppe **Sprache** auf **Übersetzen**.
3. Wählen Sie die Sprache aus, in die übersetzt werden soll.
4. Klicken Sie auf **OK**.

Ihr Webbrowser öffnet dann einen Übersetzungsdienst namens LibreTranslate mit Ihrem Text. Gesendet wird erst, wenn Sie auf **OK** klicken. Sie können die Adresse des Dienstes im Feld ändern, zum Beispiel auf einen Dienst, den Sie selbst betreiben. Es werden nur sichere Webadressen (https) verwendet, für diesen Computer auch http-Adressen. Es werden höchstens 5.000 Buchstaben gesendet.

## Barrierefreiheit prüfen

**Barrierefreiheit prüfen** öffnet ein kleines Fenster, das auflistet, was manche Menschen nur schwer nutzen können: ein Bild oder eine Form ohne Alternativtext, eine Folie ohne Titel, eine Tabelle ohne Kopfzeile, eine Tabelle mit verbundenen Zellen, ein Video ohne Untertitel, eine Folie, deren Titel eine andere Folie ebenfalls hat (ein Tipp), ein Link, dessen Text seine eigene Adresse ist oder nichts über das Ziel aussagt (etwa „hier“ oder „weiterlesen“), Text, der vor seinem Hintergrund schlecht lesbar ist, und Objekte, die ein Bildschirmleseprogramm in einer anderen Reihenfolge vorliest, als ihre Positionen vermuten lassen. Text braucht zum Hintergrund einen Kontrast von 4,5 zu 1 (3 zu 1 für Text ab 18 Punkt oder ab 14 Punkt fett). Liegt Text über einem Bild, weist ein Tipp darauf hin, den Kontrast mit dem Auge zu prüfen. Ein Bildschirmleseprogramm liest die Objekte einer Folie vom Ende des Stapels zum Anfang. Klicken Sie auf **Lesereihenfolge**, um sie in dieser Reihenfolge zu sehen, und verwenden Sie **Früher lesen** und **Später lesen** (oder ziehen Sie sie), um sie zu ändern. Jeder Schritt lässt sich mit Rückgängig zurücknehmen. Ein Bildschirmleseprogramm verwendet die erste Zeile einer Tabelle, um deren Spalten zu benennen. Wählen Sie ein Tabellenproblem aus und klicken Sie auf **Erste Zeile als Kopfzeile verwenden**, um die Kopfzeile zu aktivieren (ein Schritt von Rückgängig). Ein Video ohne Untertitel wird als Tipp aufgeführt, denn Videos brauchen Untertitel für Menschen, die nicht hören können. Es gibt keine Korrektur mit einem Klick: Wählen Sie das Video aus und verwenden Sie **Beschriftungen einfügen** auf der Registerkarte **Wiedergabe**. Danach verschwindet der Tipp. Klicken Sie auf ein Problem, um zu seiner Folie zu gelangen und das Objekt auszuwählen. Geben Sie einem Bild einen Alternativtext im Bereich **Bild formatieren**, oder markieren Sie es als dekorativ, und klicken Sie dann auf **Erneut prüfen**. Die Prüfung können Sie auch über **Datei > Info** starten.

## Siehe auch

- [Präsentationen vergleichen](compare)
- [Kommentare](comments)
- [Text formatieren](text)
