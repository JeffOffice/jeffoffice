# Suchen, Ersetzen und Gehe zu
<!-- keywords: suchen, ersetzen, ändern, finden, Platzhalter, Stern, Fragezeichen, Tilde, Groß-/Kleinschreibung, ganze Zelle, Gehe zu, Bezug, Name, nach Typ auswählen, Leerzellen, Konstanten, Formeln, Fehler, aktuelle Region, aktuelle Matrix, Zeilenunterschiede, Spaltenunterschiede, letzte Zelle, sichtbare Zellen, bedingte Formatierung, Datenüberprüfung, Strg+F, Strg+H, F5, Notizen -->

## Suchen

Drücken Sie **Strg+F** (oder wählen Sie auf der Registerkarte **Start** die Option **Suchen** und dann noch einmal **Suchen**). Geben Sie den Text ein, und drücken Sie **Eingabe**, um zur nächsten Zelle zu springen, die ihn enthält. Alternativ wählen Sie **Alle auflisten**, um eine Liste aller passenden Zellen zu erhalten. Klicken Sie auf eine Zeile der Liste, um zu dieser Zelle zu gelangen.

| Option | Funktion |
| --- | --- |
| **Suchbereich** | Durchsuchen Sie dieses Blatt oder alle sichtbaren Blätter. Sind mehrere Zellen markiert, durchsucht eine Suche in einem Blatt nur diese Zellen. |
| **Suchreihenfolge** | Die Zellen werden zeilenweise (von links nach rechts, dann nach unten) oder spaltenweise (von oben nach unten, dann nach rechts) durchsucht. **Weitersuchen** springt zum ersten Treffer in dieser Reihenfolge. |
| **Suchen in** | **Formeln** durchsucht, was in jeder Zelle eingegeben ist, **Werte** durchsucht, was sie anzeigt, und **Notizen** durchsucht die Notizen an den Zellen. |
| **Groß-/Kleinschreibung beachten** | Groß- und Kleinbuchstaben müssen genau übereinstimmen. |
| **Nur ganze Zelle suchen** | Die gesamte Zelle muss dem Text entsprechen, nicht nur ein Teil davon. |

Im Suchtext steht **?** für ein beliebiges einzelnes Zeichen und **\*** für eine beliebige Zeichenfolge (auch eine leere). Geben Sie **~** vor ?, \* oder ~ ein, um das Zeichen selbst zu suchen: **a~\*b** findet a\*b.

Eine Suche läuft schrittweise ab, sodass Sie weiterarbeiten können. Mit **Esc** (oder **Abbrechen**) beenden Sie sie; sie wird auch beendet, wenn Sie eine Zelle ändern. Ein sehr langer Suchtext benötigt bei jedem Schritt mehr Zeit als ein kurzer. Bei einem Blatt mit vielen langen Texten bleibt das Fenster dennoch bedienbar.

Notizen an Zellen folgen den Zellen: Nach einer Sortierung oder nach dem Einfügen oder Löschen von Zeilen oder Spalten findet eine Suche in Notizen jede Notiz an der Zelle, an der sie jetzt steht. Eine Notiz an einer gelöschten Zelle ist aus der Suche verschwunden (und kehrt mit **Rückgängig** zurück). Wurde eine Arbeitsmappe so vielfältig geändert, dass sich die Notizen nicht mehr durchgehend nachverfolgen lassen, werden Sie aufgefordert, sie zuerst zu speichern. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, hat ihre Notizen in einer Form, die JeffCrunch nicht durchsucht; darauf wird Sie ein Hinweis aufmerksam machen.

## Ersetzen

Drücken Sie **Strg+H**. Mit **Ändern** wird der neue Text in die Zelle mit der Einfügemarke eingetragen, sofern sie den alten Text enthält, und es geht zur nächsten Zelle weiter. **Alle ändern** führt die Ersetzung in allen Zellen durch, die den Text enthalten, und zwar als einen einzigen Schritt, den Sie mit **Rückgängig** zurücknehmen können. Eine umfangreiche Änderung wird nach und nach durchgeführt. Der neue Text wird so eingetragen, als hätten Sie ihn eingegeben: Eine Zahl bleibt eine Zahl, und eine geänderte Formel bleibt eine Formel.

Es wird nichts geändert, und Sie erhalten einen Hinweis auf den Grund, wenn eine Zelle, die geändert werden müsste, auf einem geschützten Blatt gesperrt ist, Teil einer Matrix oder einer Datentabelle ist, in der ersten Zeile einer Tabelle steht oder danach keine Formel mehr wäre.

## Gehe zu

Drücken Sie **F5** (oder **Strg+G**), oder geben Sie etwas in das **Namenfeld** ein. Sie können eine Zelle wie B7, einen Bereich wie B3:D9, ganze Spalten oder Zeilen (C:E, 4:9), eine Zelle eines anderen Blatts (Tabelle2!A1 oder 'Mein Blatt'!A1) oder den Namen eines Bereichs angeben.

## Nach Typ auswählen

Wählen Sie im Menü **Suchen** die Option **Nach Typ auswählen**, um Zellen nach ihrer Art zu markieren:

- **Leerzellen**, **Konstanten** (eingegeben), **Formeln** und **Zellen mit Fehleranzeige**. Bei Konstanten und Formeln können Sie Zahlen, Text, WAHR und FALSCH oder Fehler ausschließen.
- **Der Bereich um die Zelle mit der Einfügemarke** und **Die letzte verwendete Zelle**.
- **Die Matrixformel, in der sich die Einfügemarke befindet** markiert alle Zellen dieser Matrixformel.
- **Zellen, die sich in ihrer Zeile von der Zelle mit der Einfügemarke unterscheiden** und **in ihrer Spalte** finden die Zellen, die der Zelle mit der Einfügemarke nicht entsprechen. Markieren Sie zuerst einen Bereich, und setzen Sie die Einfügemarke auf die Zelle, der die anderen entsprechen sollen. In jeder Zeile (oder Spalte) des Bereichs wird eine Zelle erfasst, wenn sie eine andere Zahl oder einen anderen Text enthält oder wenn sie eine Formel enthält, die nach dem Verschieben ihrer Zellbezüge an die Position dieser Zelle nicht gleich ist. Eine Zelle mit Formel wird auch erfasst, wenn die Zelle mit der Einfügemarke keine Formel hat. Markieren Sie nur eine einzige Zelle, betrachtet JeffCrunch den Bereich um sie herum.
- **Zellen, die angezeigt werden** (nicht in ausgeblendeten Zeilen oder Spalten), **Zellen mit bedingter Formatierung** sowie **Zellen mit einer Regel für zulässige Eingaben** (alle oder nur diejenigen mit derselben Regel wie die Zelle mit der Einfügemarke).

Der erste gefundene Bereich wird markiert, alle weiteren werden farblich hervorgehoben. Die Hervorhebung verschwindet, sobald Sie auf eine Zelle klicken.
