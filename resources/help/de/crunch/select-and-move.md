# Zellen auswählen und navigieren
<!-- keywords: auswählen, Auswahl, Zelle, Bereich, Zeile, Spalte, klicken, ziehen, Umschalt, Strg, Pfeiltaste, Pos1, Ende, Bild auf, Bild ab, Tabulator, Eingabe, ganze Zeile, ganze Spalte, Namenfeld, gehe zu, Zellen verschieben, Rahmen ziehen, scrollen, Statusleiste, Summe, Mittelwert, Anzahl -->

**Auswählen** bedeutet, die Zellen zu markieren, auf die der nächste Befehl angewendet wird. Die ausgewählten Zellen sind farblich hinterlegt und von einem grünen Rahmen umgeben. Die **aktive Zelle** ist die Zelle mit dem Cursor; sie bleibt innerhalb der Auswahl weiß.

## Mit der Maus

- **Klicken** Sie auf eine Zelle, um sie auszuwählen. Klicken Sie in eine verbundene Zelle, um die gesamte verbundene Zelle auszuwählen.
- **Ziehen** Sie über Zellen, um einen Block auszuwählen. Ziehen Sie über den Rand des Fensters hinaus, scrollt das Blatt.
- **Umschalt+Klicken** Sie auf eine andere Zelle, um von der aktiven Zelle bis dorthin auszuwählen.
- Klicken Sie auf einen **Spaltenbuchstaben** oder eine **Zeilennummer**, um die ganze Spalte bzw. Zeile auszuwählen. Ziehen Sie über die Buchstaben oder Nummern oder verwenden Sie Umschalt+Klicken, um mehrere auszuwählen.
- Klicken Sie auf die **Ecke**, in der die Buchstaben und Nummern zusammentreffen, um alles auszuwählen.
- Halten Sie **Strg** beim Klicken oder Ziehen gedrückt, um der Auswahl **einen Bereich hinzuzufügen**. Die hinzugefügten Bereiche sind farblich hinterlegt, und der Bereich mit dem Cursor hat den grünen Rahmen. Mit **Entf** werden alle Bereiche in einem Schritt gelöscht. Ein Befehl, der nur auf einen Bereich gleichzeitig wirkt, etwa Kopieren, Ausfüllen, Sortieren, Suchen in der Auswahl, Alle ersetzen oder eine Formatierung, weist Sie darauf hin und ändert nichts; ein Klick genügt, um wieder einen einzigen Bereich auszuwählen. Wechseln Sie das Blatt oder fügen Sie Zeilen oder Spalten ein bzw. entfernen Sie sie, bleibt ebenfalls nur ein Bereich übrig.
- **Ziehen Sie am Rand** der Auswahl, um die Zellen an eine andere Stelle zu verschieben. Zeigen Sie auf den Rand des grünen Rahmens, bis der Mauszeiger vier Pfeile anzeigt. Eine gestrichelte Umrandung zeigt, wo die Zellen landen. Alles, was auf die Zellen verweist, etwa eine Formel, folgt ihnen. Halten Sie beim Ziehen **Strg** gedrückt, um die Zellen stattdessen zu kopieren. Um die Zellen auf ein anderes Blatt zu verschieben, beginnen Sie das Ziehen am Rand, halten dann **Alt** gedrückt und lassen über dem Blattreiter dieses Blatts los: Die Zellen landen in denselben Zellen dieses Blatts (mit zusätzlich gedrückter **Strg**-Taste werden sie dorthin kopiert). Wenn Sie **Alt** schon vor dem Drücken der Maustaste halten, beginnt auf diese Weise kein Ziehen. Zellen, die bereits an der Zielstelle stehen, werden überschrieben, wie beim Verschieben innerhalb eines Blatts; mit **Rückgängig** stellen Sie beide Blätter in einem Schritt wieder her. Das Blatt, auf dem Sie ablegen, muss dies zulassen: Ein geschütztes Blatt sowie Zellen, die verbundene Zellen, eine Tabelle oder eine Pivot-Tabelle durchschneiden würden, werden abgelehnt, und es wird nichts geändert. Ein ausgeblendetes Blatt hat keinen Reiter, und die Schaltfläche **Neues Blatt** ist kein Reiter; deshalb nehmen beide keine Zellen auf. Das Drücken und Loslassen von **Alt** während des Ziehens zeigt keine Zugriffstasten an. Um dasselbe über die Tastatur zu erreichen, schneiden Sie die Zellen aus, wechseln zum anderen Blatt und fügen sie dort ein. Halten Sie beim Ziehen **Umschalt** gedrückt, um die Zellen zwischen andere einzufügen, statt sie darüberzulegen. Ohne Maus erreichen Sie dasselbe, indem Sie die Zellen ausschneiden oder kopieren und **Ausgeschnittene oder kopierte Zellen einfügen** verwenden (siehe [Einfügen und Löschen](insert-delete)). Klicken Sie nur auf den Rand, ohne zu verschieben, wird die Zelle wie gewohnt ausgewählt.
- Drehen Sie das **Mausrad**, um drei Zeilen zu scrollen. Halten Sie **Umschalt** gedrückt, um seitwärts zu scrollen, oder **Strg**, um zu zoomen.

## Mit der Tastatur

Die Pfeiltasten verschieben die aktive Zelle und überspringen dabei ausgeblendete Zeilen und Spalten. Halten Sie **Umschalt** gedrückt, um die Auswahl zu erweitern. **Strg** zusammen mit einer Pfeiltaste springt zum Rand der Daten: zum Ende einer Folge gefüllter Zellen oder, vom Ende einer Folge aus, zur nächsten gefüllten Zelle. Alle Tasten finden Sie unter [JeffCrunch mit der Tastatur bedienen](keyboard).

**Strg+A** wählt den Zellblock rund um die aktive Zelle aus, begrenzt durch leere Zeilen und Spalten; drücken Sie erneut, um das ganze Blatt auszuwählen. Ist die Auswahl bereits größer als dieser Block, wählt Strg+A sofort das ganze Blatt aus. **Strg+Umschalt+\*** wählt nur den Block aus. Drücken Sie **Ende** und danach eine Pfeiltaste, um wie mit **Strg** und der Pfeiltaste zum Rand der Daten zu gelangen. Die Statusleiste zeigt **ENDE** an, bis Sie die Pfeiltaste, eine andere Taste drücken oder klicken.

## Zu einer Zelle gehen

Geben Sie eine Adresse im **Namenfeld** ein, zum Beispiel **C5**, **A1:D10**, **B:D** oder **3:7**, und drücken Sie **Eingabe**. Mit **F5** (oder Strg+G) öffnen Sie das Dialogfeld **Gehe zu**; geben Sie die Adresse dort ein. Trennen Sie mehrere Adressen durch Kommas, etwa **A1:B2,D4**, um mehrere Bereiche gleichzeitig auszuwählen.

## Die Zahlen der Auswahl

Wenn Sie zwei oder mehr Zellen auswählen, zeigt die Statusleiste den **Mittelwert**, die **Anzahl** und die **Summe** der Zellen an. Die Anzahl zählt die Zellen, die einen Inhalt haben; Mittelwert und Summe berücksichtigen nur die Zahlen. Ausgeblendete Zeilen und Spalten werden nicht berücksichtigt. Um die Auswahl als Text zu kopieren, drücken Sie **Strg+C**.

## Was die Statusleiste anzeigt

Links in der Statusleiste steht **Bereit**, wenn Sie arbeiten können, **Eingeben**, während Sie in einer Zelle schreiben, **Bearbeiten** nach F2, **Zeigen**, während Sie in eine Zelle klicken oder mit den Pfeiltasten zu einer Zelle gehen können, um deren Adresse in eine Formel einzufügen, und **ENDE** nach der Taste Ende. Ist ein Filter aktiv, zeigt die Statusleiste außerdem an, wie viele Zeilen der Filter übrig gelassen hat, etwa **3 von 8 Datensätzen gefunden**.
