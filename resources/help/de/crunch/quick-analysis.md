# Schnellanalyse

<!-- keywords: Schnellanalyse, analysieren, Analyse, Schnelldiagramm, Diagramm schnell, Schnellsummen, Summen, laufende Summe, Prozent vom Gesamtwert, Datenbalken, Farbskala, Symbolsatz, Sparklines, Tabelle, Pivot-Tabelle, Strg+Q, Vorschau -->

**Schnellanalyse** stellt die gängigsten Möglichkeiten, einen Zahlenbereich zu betrachten, in einem kleinen Feld zusammen: Zellen färben, als Diagramm darstellen, summieren, als Tabelle formatieren oder jeder Zeile ein kleines Diagramm geben. Sie wählen aus, was Sie möchten, und sehen das Ergebnis in Ihrer Tabelle, bevor Sie sich entscheiden.

## Öffnen

1. Markieren Sie zwei oder mehr Zellen, die Inhalte enthalten. Knapp unterhalb und rechts neben der zuletzt markierten Zelle erscheint eine kleine Schaltfläche.
2. Klicken Sie auf die Schaltfläche, drücken Sie **Strg+Q** (auf einem Mac **Control+Q**), oder klicken Sie auf **Start > Bearbeiten > Schnellanalyse**.
3. Wählen Sie eine Registerkarte und dann eine Option.

Die Schaltfläche nimmt Ihnen die Tastatur nie aus der Hand, die Eingabe bleibt im Tabellenblatt. Sie verschwindet, während Sie eine Zelle bearbeiten, wenn Sie nur eine Zelle markieren oder Zellen ohne Inhalt, und wenn das Blatt so geschützt ist, dass Sie nichts von dem tun können, was das Feld anbietet.

## Vorschau ansehen, bevor Sie wählen

Zeigen Sie mit der Maus auf eine Option, oder drücken Sie die Pfeiltasten **Links** und **Rechts**, um die Optionen einer Registerkarte durchzugehen (mit den Pfeiltasten **Oben** und **Unten** wechseln Sie zwischen den Registerkarten). Das Blatt zeigt, was die Option bewirken würde, aber es wird noch nichts geändert. Bei Summen über sehr viele Zellen weist das Feld darauf hin, dass zu viele Zellen für eine Vorschau vorhanden sind; die Option funktioniert trotzdem, wenn Sie sie anklicken. Es gibt keinen Schritt zum Rückgängigmachen, und eine Datei, die in diesem Moment gespeichert wird, enthält davon nichts. Drücken Sie **Esc** oder klicken Sie daneben, dann ist die Vorschau verschwunden. Klicken Sie auf eine Option oder drücken Sie **Eingabe**, um sie anzuwenden. Jede Auswahl ist ein Schritt von **Rückgängig**.

## Die Registerkarten

- **Formatierung**: **Datenbalken**, **Farbskala** und **Symbolsatz** zeigen, wie groß jede Zahl ist. **Größer als** färbt die Zellen, deren Zahl größer ist als der Durchschnitt aller Zahlen. **Obere 10 %** färbt das höchste Zehntel. **Format löschen** entfernt die bedingte Formatierung aus den markierten Zellen. Dies sind dieselben Regeln wie unter **Start > Bedingte Formatierung**, und Sie können sie dort ändern.
- **Diagramme**: bis zu vier Diagrammtypen, die zur Form Ihrer Zellen passen, dieselben, die **Einfügen > Empfohlene Diagramme** auswählt. Eine Vorschau zeigt das Diagramm in der Ecke des Blatts. Wenn Sie eine Option wählen, wird das Diagramm in das Blatt eingefügt.
- **Summen**: **Summe**, **Mittelwert**, **Anzahl**, **% vom Gesamtwert** und **Laufende Summe**, geschrieben als normale Formeln. Die erste Reihe der Optionen setzt unter jede Spalte mit Zahlen eine Summe. Die zweite Reihe (**Zeilensumme** und so weiter) setzt rechts neben jede Zeile eine Summe. Eine erste Zeile oder erste Spalte, die nur Wörter enthält, wird als Beschriftung behandelt und nicht berücksichtigt. Eine erste Zeile mit Zahlen, etwa Jahreszahlen, wird wie die übrigen Zeilen addiert. Ausgeblendete und gefilterte Zeilen werden mitgezählt. **% vom Gesamtwert** zeigt den Anteil jeder Spalte (oder Zeile) an allen Zahlen als Prozentwert. **Laufende Summe** addiert fortlaufend und schreibt die Ergebnisse in neue Zellen neben (oder unter) Ihren Zellen. Enthalten bereits Zellen, in die die Summen geschrieben werden sollen, Inhalte, werden Sie gefragt, bevor sie ersetzt werden. Summen werden nicht über Zellen geschrieben, die ein übergelaufenes Ergebnis füllt, und auch nicht über verbundene Zellen. Die Schnellanalyse weist Sie darauf hin und ändert nichts.
- **Tabellen**: **Tabelle** macht aus den Zellen eine Tabelle mit Schaltflächen zum Sortieren und Filtern. **Pivot-Tabelle** erstellt eine Pivot-Tabelle, die die Daten summiert, auf einem neuen Blatt. Die erste Zeile sollte die Namen der Spalten enthalten.
- **Sparklines**: **Linie**, **Säule** und **Gewinn/Verlust** zeichnen ein kleines Diagramm für jede Zeile in der Zelle rechts neben ihr.

## Grenzen

Die Schnellanalyse betrachtet für Summen und Sparklines bis zu 250.000 Zellen. Wenn Sie mehr markieren, weist das Programm darauf hin und ändert nichts. Eine Option, die nicht passt (zum Beispiel weil unter den Zellen kein Platz ist), wird ebenfalls gemeldet.
