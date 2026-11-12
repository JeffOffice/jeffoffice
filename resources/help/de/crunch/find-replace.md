# Suchen, Ersetzen und Gehe zu
<!-- keywords: suchen, ersetzen, ändern, finden, Suchbegriff, ersetzen durch, alle ersetzen, weitersuchen, suchen nach, ändern in, alle ändern, Groß- und Kleinschreibung, Platzhalter, Stern, Fragezeichen, Tilde, Groß-/Kleinschreibung beachten, ganze Zelle, gehe zu, Verweis, Name, nach Typ auswählen, leere Zellen, Konstanten, Formeln, Fehler, aktueller Bereich, aktuelle Matrix, Zeilenunterschiede, Spaltenunterschiede, letzte Zelle, sichtbare Zellen, bedingte Formatierung, Datenüberprüfung, Strg+F, Strg+H, F5, Notizen -->

## Suchen

Drücken Sie **Strg+F** (oder wählen Sie auf der Registerkarte **Start** die Option **Suchen** und dann noch einmal **Suchen**). Geben Sie den Text in **Suchen nach** ein, und drücken Sie **Eingabe** für die nächste Zelle, die ihn enthält. Alternativ wählen Sie **Alle auflisten**, um eine Liste aller passenden Zellen zu erhalten. Klicken Sie auf eine Zeile der Liste, um zu dieser Zelle zu wechseln.

| Option | Funktion |
| --- | --- |
| **Suchen in** | Dieses Blatt oder alle sichtbaren Blätter durchsuchen. Sind mehrere Zellen markiert, durchsucht eine Suche in einem Blatt nur diese Zellen. |
| **Reihenfolge** | Die Zellen zeilenweise (von links nach rechts, dann nach unten) oder spaltenweise (von oben nach unten, dann nach rechts) durchlaufen. **Weitersuchen** hält beim ersten Treffer in dieser Reihenfolge an. |
| **Suchen in** | **Formeln** durchsucht die Eingaben in den Zellen, **Werte** die angezeigten Inhalte und **Notizen** die Notizen an den Zellen. |
| **Groß-/Kleinschreibung beachten** | Groß- und Kleinschreibung müssen übereinstimmen. |
| **Nur ganze Zelle suchen** | Die gesamte Zelle muss dem Text entsprechen, nicht nur ein Teil davon. |

Im Text, den Sie bei **Suchen nach** eingeben, steht **?** für ein beliebiges Zeichen und **\*** für eine beliebige Zeichenfolge (auch keine). Geben Sie **~** vor ?, \* oder ~ ein, um das Zeichen selbst zu suchen: **a~\*b** findet a\*b.

Eine Suche läuft schrittweise ab, sodass Sie weiterarbeiten können. **Esc** (oder **Abbrechen**) bricht sie ab, ebenso eine Änderung an einer Zelle. Sehr lange Texte beanspruchen bei jedem Schritt mehr Zeit als kurze. Ein Blatt mit vielen langen Texten bleibt dennoch bedienbar.

Notizen an Zellen folgen den Zellen: Nach dem Sortieren oder nach dem Einfügen oder Löschen von Zeilen oder Spalten findet eine Suche in Notizen jede Notiz an der Zelle, auf der sie jetzt steht. Eine Notiz an einer gelöschten Zelle wird von der Suche nicht mehr erfasst (sie ist nach **Rückgängig** wieder da). Wurde eine Arbeitsmappe so oft verändert, dass die Notizen nicht bei allen Änderungen verfolgt werden können, werden Sie aufgefordert, sie zuerst zu speichern. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, hat ihre Notizen in einer Form, die JeffCrunch nicht durchsucht. Sie erhalten dazu einen Hinweis.

## Ersetzen

Drücken Sie **Strg+H**, geben Sie den neuen Text bei **Ersetzen durch** ein, und wählen Sie eine Schaltfläche. **Ersetzen** setzt den neuen Text in die Zelle mit der Einfügemarke, sofern sie den Suchtext enthält, und springt dann zur nächsten Zelle. **Alle ersetzen** ändert alle passenden Zellen in einem Schritt, den Sie rückgängig machen können. Große Änderungen werden nach und nach durchgeführt. Der neue Text wird so eingegeben, als hätten Sie ihn getippt: Eine Zahl wird zur Zahl, und eine geänderte Formel bleibt eine Formel.

Es wird nichts geändert, und Sie erhalten einen Hinweis mit dem Grund, wenn eine Zelle, die geändert werden müsste, auf einem geschützten Blatt gesperrt ist, Teil einer Matrix oder einer Datentabelle ist, in der ersten Zeile einer Tabelle steht oder danach keine Formel mehr wäre.

## Gehe zu

Drücken Sie **F5** (oder **Strg+G**), oder geben Sie im Namenfeld etwas ein. Sie können eine Zelle wie B7, einen Bereich wie B3:D9, ganze Spalten oder Zeilen (C:E, 4:9), eine Zelle eines anderen Blatts (Tabelle2!A1 oder 'Mein Blatt'!A1) oder den Namen eines Bereichs angeben.

## Nach Typ auswählen

Wählen Sie im Menü **Suchen** die Option **Nach Typ auswählen**, um Zellen nach ihrer Art auszuwählen:

- **Leere Zellen**, **Konstanten** (eingegebene Werte), **Formeln** und **Zellen mit Fehleranzeige**. Bei Konstanten und Formeln können Sie Zahlen, Text, WAHR und FALSCH oder Fehler ausschließen.
- **Der Bereich um die Zelle mit der Einfügemarke** und **Die zuletzt verwendete Zelle**.
- **Die Matrixformel, in der sich die Einfügemarke befindet** wählt alle Zellen dieser Matrixformel aus.
- **Zellen, die sich von der Zelle mit der Einfügemarke in ihrer Zeile unterscheiden** und **in ihrer Spalte** finden die Zellen, die nicht wie die Zelle mit der Einfügemarke sind. Markieren Sie zuerst einen Bereich, und setzen Sie die Einfügemarke auf die Zelle, mit der die anderen übereinstimmen sollen. In jeder Zeile (oder Spalte) des Bereichs wird eine Zelle gewählt, wenn sie eine andere Zahl oder einen anderen Text enthält oder wenn sie eine Formel enthält, die nach dem Verschieben ihrer Zellbezüge an die Position dieser Zelle nicht gleich ist. Eine Zelle mit einer Formel wird auch gewählt, wenn die Zelle mit der Einfügemarke keine Formel enthält. Markieren Sie nur eine einzelne Zelle, betrachtet JeffCrunch den Bereich um sie herum.
- **Sichtbare Zellen** (nicht in ausgeblendeten Zeilen oder Spalten), **Zellen mit bedingter Formatierung** sowie **Zellen mit einer Regel für die Eingabe** (alle oder nur solche mit derselben Regel wie die Zelle mit der Einfügemarke).

Der erste gefundene Block wird markiert, alle weiteren werden getönt. Die Tönung verschwindet, sobald Sie eine Zelle anklicken.
