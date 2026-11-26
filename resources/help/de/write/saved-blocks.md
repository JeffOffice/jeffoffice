# Einen Baustein speichern und wieder einfügen
<!-- keywords: gespeicherte Bausteine, Textbaustein, Standardtext, Unterschrift, Briefkopf, Textkürzel, Text wiederverwenden, Auswahl speichern, erneut einfügen, F3, Bibliothek -->

Ein **Baustein** ist ein Teil eines Dokuments, das Sie gespeichert haben, damit Sie es wieder einfügen können: eine Unterschrift, ein Absatz, den Sie in jedem Brief verwenden, ein Briefkopf oder ein Bild mit Bildunterschrift. Ein Baustein behält seinen Text mit Schriftarten, Farben, Formatvorlagen, Aufzählungszeichen und Nummerierungen, Bildern, Verknüpfungen, Feldern und Tabellen (Zeilen, Spalten, Breiten, Rahmen, Schattierungen, verbundene Zellen, eine Kopfzeile, die sich wiederholt, und die Tabellenformatvorlage). Kommentare, Fußnoten, Endnoten und Abschnittswechsel werden nicht übernommen (siehe Wichtige Hinweise).

## Baustein speichern

1. Markieren Sie, was Sie speichern möchten.
2. Wählen Sie **Einfügen > Gespeicherte Bausteine > Auswahl als Baustein speichern**.
3. Geben Sie einen **Namen** ein. Sie können außerdem eine **Kategorie** und eine **Beschreibung** angeben, die Ihnen das spätere Finden erleichtern.
4. Wählen Sie unter **Galerie** entweder **Gespeicherte Bausteine** (ein Baustein, den Sie aus einer Galerie auswählen) oder **Textkürzel** (ein Baustein, den Sie durch Eingabe seines Namens einfügen).
5. Wählen Sie unter **Einfügen als**, wie der Baustein in ein Dokument kommt: **Wie er ist, an der Einfügemarke**, **In einem eigenen Absatz** oder **Auf einer eigenen Seite**.
6. Klicken Sie auf **Speichern**.

Ist unter diesem Namen bereits ein Baustein gespeichert, fragt JeffWrite, ob er ersetzt werden soll.

## Baustein einfügen

1. Setzen Sie die Einfügemarke an die Stelle, an der der Baustein stehen soll. Wenn Sie vorher Wörter markieren, nimmt der Baustein deren Platz ein.
2. Wählen Sie **Einfügen > Gespeicherte Bausteine** und klicken Sie dann auf den Baustein. Geben Sie im Suchfeld einen Teil des Namens, der Kategorie oder der Beschreibung ein, um einen Baustein zu finden.

Das Einfügen eines Bausteins lässt sich mit einem Schritt rückgängig machen. Eine Formatvorlage, die der Baustein verwendet und Ihr Dokument nicht hat, wird hinzugefügt; eine Formatvorlage, die Ihr Dokument bereits unter demselben Namen hat, wird nie verändert. Ein Feld im Baustein, etwa die Seitenzahl, wird für das Dokument aktualisiert, in dem es steht. Der erste und der letzte Absatz eines Bausteins übernehmen das Erscheinungsbild der Absätze, an die sie anschließen, wie beim Einfügen.

## Eigene Deckblätter, Tabellen und Textfelder

Ein eigenes Deckblatt, eine eigene Tabelle oder ein eigenes Textfeld können Sie in einer eigenen Galerie unterhalb der integrierten ablegen.

- **Deckblatt.** Markieren Sie das Deckblatt und seinen Seitenumbruch (oder setzen Sie die Einfügemarke irgendwo und markieren Sie nichts, wenn das Dokument bereits ein Deckblatt hat), und wählen Sie **Einfügen > Deckblatt > Ihre Deckblätter > Auswahl in Deckblatt-Katalog speichern**.
- **Tabelle.** Markieren Sie die ganze Tabelle und wählen Sie **Einfügen > Tabelle > Fertige Tabellen > Ihre Tabellen > Auswahl in Tabellen-Katalog speichern**.
- **Textfeld.** Klicken Sie auf das Textfeld, sodass es markiert ist, und wählen Sie **Einfügen > Textfeld > Ihre Textfelder > Auswahl in Textfeld-Katalog speichern**.

Geben Sie einen Namen ein und klicken Sie auf **Speichern**. Der Eintrag erscheint dann unter **Ihre Deckblätter**, **Ihre Tabellen** oder **Ihre Textfelder**. Klicken Sie darauf, um ihn einzufügen. Ein Deckblatt wird am Anfang des Dokuments auf einer eigenen Seite eingefügt und ersetzt das vorhandene Deckblatt. Eine Tabelle oder ein Textfeld wird an der Einfügemarke eingefügt. **Gespeicherte Bausteine verwalten** listet diese Galerien ebenfalls auf, sodass Sie sie umbenennen, ändern, löschen, exportieren und importieren können.

## Textkürzel

Ein Textkürzel ist ein Baustein mit einem kurzen Namen. Geben Sie den Namen ein und drücken Sie dann **F3** (oder wählen Sie **Einfügen > Gespeicherte Bausteine > Textkürzel erweitern**). Der Name wird durch den Baustein ersetzt. Der Name muss am Anfang eines Wortes stehen: „sig“ funktioniert nach einem Leerzeichen, aber nicht innerhalb von „Design“.

## Gespeicherte Bausteine verwalten

Wählen Sie **Einfügen > Gespeicherte Bausteine > Gespeicherte Bausteine verwalten**. Sie sehen jeden Baustein und können seinen Namen, seine Kategorie, seine Beschreibung oder die Art des Einfügens über **Eigenschaften bearbeiten** ändern sowie ihn über **Löschen** entfernen. **Löschen rückgängig machen** stellt das Gelöschte wieder her, solange das Dialogfeld geöffnet ist. Mit **Exportieren** speichern Sie Ihre Bausteine in einer Datei, mit **Importieren** fügen Sie die Bausteine einer solchen Datei hinzu. Ein Baustein, dessen Name bereits vorhanden ist, bleibt unverändert.

## Wichtige Hinweise

- Ihre gespeicherten Bausteine werden in einer einzigen Datei auf diesem Computer abgelegt, nur für Ihr eigenes Konto: Sie stehen nicht in Ihren Dokumenten und werden nirgendwohin gesendet. Die Datei heißt `library.jblocks` und liegt im Ordner `Saved Blocks` innerhalb Ihres Datenordners: `AppData\Local\JeffOffice` unter Windows, `Library/Application Support/JeffOffice` auf dem Mac und `.local/share/JeffOffice` unter Linux. Alle JeffOffice-Programme auf Ihrem Computer nutzen sie gemeinsam, und zwei Fenster können gleichzeitig Bausteine hinzufügen, ohne dass einer den anderen überschreibt.
- Ein Baustein darf bis zu 8 MB groß sein, die Bibliothek fasst bis zu 5.000 Bausteine.
- Eine Tabelle wird als Tabelle gespeichert, wenn die Auswahl die ganze Tabelle enthält. Enthält die Auswahl nur einen Teil einer Tabelle, wird dieser Teil als Text gespeichert, mit einem Absatz für jede Zelle, und JeffWrite weist Sie vor dem Speichern darauf hin.
- Eine Textmarke in einem Baustein erhält beim Einfügen einen eigenen Namen: Hat Ihr Dokument bereits eine Textmarke mit diesem Namen, wird die neue mit „_2“, „_3“ usw. benannt.
- Kommentare werden nicht übernommen. Fußnoten und Endnoten werden nicht übernommen. Abschnittswechsel bleiben nicht erhalten. Formen und Textfelder bleiben erhalten und werden mit dem Baustein wieder eingefügt.
- Nachverfolgte Änderungen werden beim Speichern des Bausteins übernommen: Eingefügte Wörter bleiben erhalten, gelöschte Wörter sind nicht im Baustein enthalten. Ein Baustein enthält daher nie nachverfolgte Wörter einer anderen Person.
- Ein Dokument fügt nie Bausteine zu Ihrer Bibliothek hinzu. Das tun nur Sie.
