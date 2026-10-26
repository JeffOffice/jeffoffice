# Sortieren und Filtern
<!-- keywords: sortieren, filtern, Filter, AutoFilter, aufsteigend, absteigend, A-Z, Regeln, Sortierregeln, Überschrift, Kopfzeile, Farbe, Liste, benutzerdefinierte Liste, Monate, Wochentage, Groß-/Kleinschreibung, Spalten, größte, kleinste, Durchschnitt, Mittelwert, zwischen, beginnt mit, enthält, Datum, löschen, erneut anwenden, Dropdown, ausgeblendete Zeilen, Tabelle, Strg+Umschalt+L -->

## Sortieren

Markieren Sie eine Zelle in Ihren Daten, und wählen Sie **Von A bis Z sortieren** oder **Von Z bis A sortieren** (**Sortieren und Filtern** auf der Registerkarte **Start**). JeffCrunch nimmt den gesamten Datenbereich um die Zelle, behält eine Überschriftszeile bei, sofern es eine findet (Text über Zahlen), und ordnet die Zeilen nach der Spalte, in der Sie sich befinden. Sind mehrere Zellen markiert, werden nur diese sortiert; grenzen weitere Daten an, fragt JeffCrunch, ob diese mitsortiert werden sollen.

In der Reihenfolge kommen zuerst Zahlen, dann Text, dann WAHR und FALSCH, dann Fehler; leere Zellen stehen immer am Ende. Groß- und Kleinbuchstaben gelten als gleich, sofern Sie nichts anderes festlegen. Text wird überall auf dieselbe Weise sortiert: nach dem englischen Alphabet, wobei ein Buchstabe mit Akzent beim zugehörigen Grundbuchstaben eingeordnet wird, unabhängig von der Sprache des Computers.

Wählen Sie **Sortieren…**, um das Dialogfeld **Sortieren** zu öffnen:

- Fügen Sie für jede Spalte, nach der sortiert werden soll, eine Regel hinzu. Eine spätere Regel entscheidet bei Gleichstand in den früheren. Mit **Nach oben** und **Nach unten** ändern Sie die Reihenfolge der Regeln, mit **Regel duplizieren** verwenden Sie eine vorhandene als Ausgangspunkt.
- Sortieren Sie unter **Sortieren nach** nach Werten, nach der **Füllfarbe** der Zellen oder nach ihrer **Schriftfarbe**. Legen Sie bei einer Farbe fest, ob sie **Oben** oder **Unten** stehen soll; die übrigen Zeilen behalten ihre Reihenfolge. Maßgeblich ist die tatsächlich resultierende Farbe: Eine Farbe aus der Palette und eine Farbe mit denselben Rot-, Grün- und Blauwerten gelten als gleich.
- Bei Werten kann die Reihenfolge **A–Z**, **Z–A** oder eine Liste sein: die Wochentage und die Monate (ausgeschrieben und abgekürzt, auf Englisch und in Ihrer Sprache) oder eine Liste, die Sie selbst eingeben, mit Kommas zwischen den Werten. Werte, die nicht in der Liste stehen, kommen nach denen, die darin stehen.
- **Erste Zeile enthält Spaltenüberschriften** nimmt die erste Zeile von der Sortierung aus. **Groß-/Kleinschreibung beachten** unterscheidet zwischen Groß- und Kleinbuchstaben (Kleinbuchstaben zuerst). **Spalten statt Zeilen sortieren** ordnet die Spalten nach den Werten in einer Zeile.

Eine Formel wandert mit ihrer Zeile, und ihre Bezüge auf Zellen derselben Zeile bleiben in der Zeile. Ein Bezug auf eine Zelle in einer anderen Zeile wird so angepasst, als wäre die Formel kopiert worden. Was zu einer Zelle gehört, wandert mit ihr: Format, Notiz und Link der Zelle gehen an die neue Position der Zelle. Regeln, die für einen Zellbereich gelten (welche Eingaben zulässig sind, und bedingte Formatierungen), bleiben an ihrem Platz, da sie für die Positionen und nicht für die Zellen gelten. Das Sortieren ist ein einzelner Schritt, den Sie rückgängig machen können, und mit **Esc** brechen Sie eine umfangreiche Sortierung ab; eine umfangreiche Sortierung wird schrittweise ausgeführt.

Manche Zellen lassen sich nicht sortieren, und JeffCrunch nennt Ihnen den Grund: gesperrte Zellen auf einem geschützten Blatt, Teile einer Matrix, verbundene Zellen unterschiedlicher Größe (oder solche, die sich über mehr als eine Zeile erstrecken), verbundene Zellen, die vom Rand der Markierung durchschnitten werden, und die Kopfzeile einer Tabelle.

## Filtern

Markieren Sie eine Zelle in Ihren Daten, und wählen Sie **Filter** (**Strg+Umschalt+L**). In jeder Zelle der ersten Zeile erscheint eine Schaltfläche. Klicken Sie darauf, um Folgendes anzuzeigen:

- **Von A bis Z sortieren** und **Von Z bis A sortieren** sowie **Nach Farbe sortieren**.
- Eine Liste der Werte in der Spalte mit Kontrollkästchen, ein Suchfeld und **(Alles)**. Eine Spalte mit Datumsangaben wird nach Jahr, Monat und Tag aufgelistet, und Werte darin, die keine Datumsangaben sind, stehen darunter. Deaktivieren Sie, was Sie nicht sehen möchten, und klicken Sie dann auf **OK**. Die Suche zeigt die Werte an, die alle Ihre Wörter enthalten; **(Alle Suchergebnisse)** aktiviert oder deaktiviert dann nur diese. Die Liste umfasst die gesamte Spalte. Enthält die Spalte mehr unterschiedliche Werte, als die Liste aufnehmen kann, wird darauf hingewiesen und die Liste gekürzt; verwenden Sie für die übrigen eine Bedingung.
- **Nach Farbe filtern**, nach Füllfarbe oder Schriftfarbe.
- **Zahlenbedingungen**, **Textbedingungen** oder **Datumsbedingungen**: gleich, größer als, größer oder gleich, kleiner als, kleiner oder gleich, zwischen, die größten oder kleinsten Werte (Elemente oder Prozent), über oder unter dem Durchschnitt, beginnt mit, endet mit, enthält, Datumsangaben in einem Zeitraum (heute, diese Woche, nächster Monat, letztes Quartal, bisher in diesem Jahr, ein Quartal oder ein Monat eines beliebigen Jahres) sowie **Benutzerdefinierte Bedingung** aus zwei Vergleichen, verknüpft mit Und oder Oder. Im Text einer Bedingung steht ? für ein beliebiges einzelnes Zeichen und \* für eine beliebige Zeichenfolge.
- **Alle anzeigen** für die Spalte.

Eine Schaltfläche mit einem Trichter kennzeichnet eine gefilterte Spalte. Die nicht passenden Zeilen werden ausgeblendet und behalten ihre Zeilennummern. **Löschen** (im Menü **Sortieren und Filtern**) entfernt alle Bedingungen und zeigt die Zeilen wieder an; **Erneut anwenden** wertet die Bedingungen neu aus, nachdem Sie einen Wert geändert haben. Erneutes Wählen von **Filter** schaltet die Schaltflächen aus.

Durch einen Filter ausgeblendete Zeilen werden getrennt von Zeilen behandelt, die Sie selbst ausgeblendet haben: **Zeilen einblenden** zeigt keine Zeile an, die der Filter ausblendet, und das Löschen des Filters zeigt keine Zeile an, die Sie ausgeblendet haben. Beim Sortieren eines gefilterten Bereichs werden nur die sichtbaren Zeilen untereinander sortiert.

Ein Filter wirkt auf ein ganzes Blatt mit bis zu 1.048.576 Zeilen und wird schrittweise ausgeführt; **Löschen** und das Ausschalten des Filters funktionieren unabhängig von der Größe. Der Umfang eines Filters ist auf 40 Millionen Prüfungen begrenzt (Zeilen mal Spalten mit einer Bedingung, zuzüglich eines Anteils für die in den Bedingungen aufgeführten Werte); ein Filter, der mehr erfordert, wird mit einem Hinweis abgelehnt, und Sie können weniger Zeilen filtern oder weniger Bedingungen verwenden. Ein Filter des Blatts gilt nicht für Zellen einer Tabelle: Eine Tabelle hat eigene Schaltflächen, die genauso funktionieren (siehe [Tabellen](tables)).

Eine Werteliste in einer Bedingung wird so gespeichert, wie sie in der Datei steht, ohne sprachspezifisches Zahlen- und Datumsformat (1,234.50), und ein Wert passt zu einer Zelle, die ihn in Ihrer Sprache oder in dieser Form anzeigt; eine Datei, die in einer Sprache gefiltert wurde, filtert in einer anderen dieselben Zellen.

Der Filter wird mit seinen Bedingungen und den ausgeblendeten Zeilen in der Datei gespeichert, sodass beim Öffnen dieselben Zeilen angezeigt werden. Beim erneuten Öffnen einer Datei erscheint der Trichter auf jeder Spalte mit einer Bedingung; im Bereich des Filters ausgeblendete Zeilen gelten als vom Filter ausgeblendet.
