# Datentools
<!-- keywords: Daten, Registerkarte Daten, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Begrenzungszeichen, Anführungszeichen -->

Auf der Registerkarte **Daten** finden Sie die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** im Menü **Datei**. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art von Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte **Start**. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie diesen Befehl, wenn eine Spalte mehrere Textteile enthält, die in getrennten Spalten stehen sollen, etwa „Schmidt, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Legen Sie fest, wie der Text aufgeteilt wird:
   - **An einem Trennzeichen aufteilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wobei Sie Ihr eigenes Zeichen eingeben. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. **Text in Anführungszeichen** gibt das Zeichen an, das einen Teil umschließt, der nicht getrennt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen steht für zwei Zeichen in einer Reihe nur ein Zeichen.
   - **An festen Positionen aufteilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei die Zeichen ab 1 gezählt werden. Bei 5, 12 beginnt zum Beispiel eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Legen Sie fest, wie die Teile formatiert werden: **Allgemein** erkennt Zahlen und Datumsangaben so, als hätten Sie sie eingegeben, und **Text** übernimmt jeden Teil unverändert, sodass „007“ als „007“ erhalten bleibt.
5. Geben Sie unter **Teile einfügen ab** die erste Zelle für die Teile ein. Zunächst ist es die erste markierte Zelle.
6. Das Feld unter den Auswahlmöglichkeiten zeigt die ersten Zeilen so, wie Sie sie aufgeteilt haben. Klicken Sie auf **OK**.

Die Teile werden in die Zellen rechts neben der ersten Zelle eingefügt und ersetzen dort vorhandenen Inhalt. Der Befehl **Text in Spalten** lässt sich mit einem Schritt rückgängig machen.

## Duplikate entfernen

Verwenden Sie diesen Befehl, um von jeder Gruppe gleicher Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie den Befehl **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Duplikate gelten. Zwei Zeilen gelten nur dann als wiederholt, wenn alle aktivierten Spalten gleich sind. Groß- und Kleinschreibung werden als gleich behandelt, und eine Formel zählt als der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Das Dialogfeld zeigt an, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, und zwar mit ihren Formaten und Formeln. Die Zellen unterhalb der Markierung werden nicht verschoben. Die leeren Zeilen, die am unteren Ende der Markierung übrig bleiben, werden gelöscht. Der Befehl **Duplikate entfernen** lässt sich mit einem Schritt rückgängig machen.
