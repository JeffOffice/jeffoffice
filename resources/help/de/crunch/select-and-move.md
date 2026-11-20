# Zellen auswählen und navigieren
<!-- keywords: auswählen, Auswahl, Zelle, Bereich, Zeile, Spalte, klicken, ziehen, Umschalt, Strg, Pfeil, Pos1, Ende, Bildauf, Bild ab, Tab, Eingabe, ganze Zeile, ganze Spalte, Namenfeld, gehe zu, Zellen verschieben, Rahmen ziehen, scrollen, Statusleiste, Summe, Mittelwert, Anzahl -->

**Auswählen** von Zellen bedeutet, die Zellen festzulegen, auf die der nächste Befehl angewendet wird. Die markierten Zellen sind eingefärbt und von einem grünen Rahmen umgeben. Die **aktive Zelle** ist die Zelle mit der Einfügemarke; sie bleibt innerhalb der Auswahl weiß.

## Mit der Maus

- **Klicken** Sie auf eine Zelle, um sie auszuwählen. Klicken Sie in eine verbundene Zelle, um die gesamte verbundene Zelle auszuwählen.
- **Ziehen** Sie über Zellen, um einen Block auszuwählen. Ziehen Sie über den Fensterrand hinaus, scrollt das Blatt.
- **Umschalt+Klicken** Sie auf eine andere Zelle, um von der aktiven Zelle bis dorthin auszuwählen.
- Klicken Sie auf einen **Spaltenbuchstaben** oder eine **Zeilennummer**, um die ganze Spalte bzw. Zeile auszuwählen. Ziehen Sie über die Buchstaben oder Nummern oder verwenden Sie Umschalt+Klicken, um mehrere auszuwählen.
- Klicken Sie auf die **Ecke**, in der die Buchstaben und Nummern zusammentreffen, um alles auszuwählen.
- Halten Sie **Strg** beim Klicken oder Ziehen gedrückt, um der Auswahl **einen Bereich hinzuzufügen**. Die hinzugefügten Bereiche sind eingefärbt, und der Bereich mit der Einfügemarke hat den grünen Rahmen. Mit **Entf** werden alle Bereiche in einem Schritt gelöscht. Ein Befehl, der immer nur auf einen Bereich angewendet werden kann, etwa Kopieren, Ausfüllen, Sortieren, Suchen in der Auswahl, Alle ersetzen oder eine Formatierung, weist Sie darauf hin und ändert nichts. Klicken Sie einmal, um wieder einen einzelnen Bereich zu haben. Wechseln Sie das Blatt oder fügen Sie Zeilen oder Spalten ein bzw. entfernen Sie sie, bleibt ebenfalls nur ein Bereich übrig.
- **Ziehen Sie den Rahmen** der Auswahl, um die Zellen an eine andere Stelle zu verschieben. Zeigen Sie auf den Rand des grünen Rahmens, bis der Zeiger vier Pfeile anzeigt. Ein gestrichelter Umriss zeigt, wo die Zellen landen. Alles, was auf die Zellen verweist, etwa eine Formel, folgt ihnen. Halten Sie beim Ziehen **Strg** gedrückt, um die Zellen stattdessen zu kopieren. Halten Sie beim Ziehen **Umschalt** gedrückt, um die Zellen zwischen andere einzufügen statt über sie zu legen. Dasselbe ohne Maus erreichen Sie, indem Sie die Zellen ausschneiden oder kopieren und **Ausgeschnittene oder kopierte Zellen einfügen** verwenden (siehe [Einfügen und Löschen](insert-delete)). Klicken Sie nur auf den Rahmen, ohne ihn zu verschieben, wird die Zelle wie gewohnt ausgewählt.
- Drehen Sie das **Mausrad**, um drei Zeilen weit zu scrollen. Halten Sie **Umschalt** gedrückt, um seitlich zu scrollen, oder **Strg**, um zu zoomen.

## Mit der Tastatur

Die Pfeiltasten bewegen die aktive Zelle und überspringen ausgeblendete Zeilen und Spalten. Halten Sie **Umschalt** gedrückt, um die Auswahl zu erweitern. **Strg** mit einer Pfeiltaste springt zum Rand der Daten: zum Ende eines Bereichs mit Inhalten oder, vom Ende eines Bereichs aus, zur nächsten Zelle mit Inhalt. Alle Tasten finden Sie unter [JeffCrunch mit der Tastatur bedienen](keyboard).

**Strg+A** wählt den Zellblock um die aktive Zelle aus, begrenzt durch leere Zeilen und Spalten; drücken Sie die Taste erneut, um das ganze Blatt auszuwählen. Ist die Auswahl bereits größer als dieser Block, wählt Strg+A sofort das ganze Blatt aus. **Strg+Umschalt+\*** wählt nur den Block aus. Drücken Sie **Ende** und danach eine Pfeiltaste, um wie mit **Strg** und der Pfeiltaste zum Datenrand zu gelangen. Die Statusleiste zeigt **Ende**, bis Sie die Pfeiltaste, eine andere Taste drücken oder klicken.

## Zu einer Zelle wechseln

Geben Sie eine Adresse im **Namenfeld** ein, etwa **C5**, **A1:D10**, **B:D** oder **3:7**, und drücken Sie **Eingabe**. Mit **F5** (oder Strg+G) öffnen Sie das Dialogfeld **Gehe zu**, in das Sie die Adresse eintragen. Trennen Sie mehrere Adressen durch Kommas, etwa **A1:B2,D4**, um mehrere Bereiche gleichzeitig auszuwählen.

## Die Zahlen der Auswahl

Wenn Sie zwei oder mehr Zellen auswählen, zeigt die Statusleiste den **Mittelwert**, die **Anzahl** und die **Summe** der Zellen an. Die Anzahl zählt die Zellen, die einen Inhalt haben; Mittelwert und Summe berücksichtigen nur die Zahlen. Ausgeblendete Zeilen und Spalten bleiben unberücksichtigt. Um die Auswahl als Text zu kopieren, drücken Sie **Strg+C**.

## Was die Statusleiste anzeigt

Links in der Statusleiste steht **Bereit**, wenn Sie arbeiten können, **Eingabe**, während Sie in einer Zelle schreiben, **Bearbeiten** nach Drücken von F2, **Zeigen**, wenn Sie in eine Zelle klicken oder mit den Pfeiltasten dorthin wechseln können, um ihre Adresse in eine Formel einzufügen, und **Ende** nach Drücken der Ende-Taste. Ist ein Filter aktiv, steht dort außerdem, wie viele Zeilen der Filter übrig lässt, etwa **3 von 8 Datensätzen gefunden**.
