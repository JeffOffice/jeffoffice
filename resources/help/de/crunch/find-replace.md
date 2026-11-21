# Suchen, Ersetzen und Gehe zu
<!-- keywords: suchen, finden, ersetzen, ändern, Suchen nach, Ersetzen durch, alle ersetzen, weitersuchen, nach Text suchen, ändern in, alle ändern, gleiche Groß-/Kleinschreibung, Platzhalter, Stern, Fragezeichen, Tilde, Groß-/Kleinschreibung beachten, ganze Zelle, nach Format suchen, Format, in Notizen ersetzen, gehe zu, Bezug, Name, nach Typ auswählen, leere Zellen, Konstanten, Formeln, Fehler, aktueller Bereich, Matrixformel, Zeilenunterschiede, Spaltenunterschiede, letzte Zelle, sichtbare Zellen, bedingte Formatierung, Datenüberprüfung, Strg+F, Strg+H, F5, Notizen -->

## Suchen

Drücken Sie **Strg+F** (oder wählen Sie auf der Registerkarte **Start** die Option **Suchen** und dann noch einmal **Suchen**). Geben Sie den Text unter **Suchen nach** ein, und drücken Sie **Eingabe**, um zur nächsten Zelle mit diesem Text zu springen. Mit **Alle auflisten** erhalten Sie eine Liste aller Zellen, die den Suchbegriff enthalten. Klicken Sie auf eine Zeile der Liste, um zu dieser Zelle zu gelangen.

| Option | Wirkung |
| --- | --- |
| **Durchsuchen** | Durchsucht dieses Blatt oder alle sichtbaren Blätter. Sind mehrere Zellen markiert, beschränkt sich die Suche in einem Blatt auf diese Zellen. |
| **Reihenfolge** | Durchsucht die Zellen zeilenweise (von links nach rechts, dann nach unten) oder spaltenweise (von oben nach unten, dann nach rechts). **Weitersuchen** hält beim ersten Treffer in dieser Reihenfolge an. |
| **Suchen in** | **Formeln** durchsucht, was in jeder Zelle eingegeben ist, **Werte** durchsucht, was angezeigt wird, und **Notizen** durchsucht die Notizen an den Zellen. |
| **Format…** | Findet nur Zellen, die ein bestimmtes Format haben. **Format…** öffnet das Dialogfeld **Zellen formatieren**, das bei der Zelle mit dem Cursor beginnt: Was Sie dort ändern, wird gesucht (eine fette Schrift, eine Füllfarbe, ein Zahlenformat). **Format aus Zelle übernehmen** übernimmt alle Teile des Formats der Zelle mit dem Cursor, und **Format löschen** stellt die Suche auf reinen Text zurück. Das Dialogfeld zeigt an, wonach gesucht wird, und weist darauf hin, wenn das gewählte Format keine Angaben enthält. Die Darstellung, die eine bedingte Formatierung einer Zelle gibt, wird nicht durchsucht, sondern nur das Format, das die Zelle selbst hat. Ist bei **Suchen nach** kein Text eingegeben, finden **Alle auflisten** und **Weitersuchen** die Zellen mit dem Format; ist Text eingegeben, muss eine Zelle beides haben. |
| **Groß-/Kleinschreibung beachten** | Groß- und Kleinbuchstaben müssen übereinstimmen. |
| **Gesamten Zellinhalt vergleichen** | Die ganze Zelle muss dem Text entsprechen, nicht nur ein Teil davon. |

Im Text unter **Suchen nach** steht **?** für ein beliebiges einzelnes Zeichen und **\*** für eine beliebige Folge von Zeichen (auch keines). Geben Sie **~** vor ?, \* oder ~ ein, um das Zeichen selbst zu suchen: **a~\*b** findet a\*b.

Eine Suche läuft schrittweise ab, sodass Sie weiterarbeiten können. Mit **Esc** (oder **Abbrechen**) wird sie angehalten, ebenso durch das Ändern einer Zelle. Ein sehr langer Suchtext belastet jeden Schritt stärker als ein kurzer. Bei einem Blatt mit langen Texten bleibt das Fenster dennoch bedienbar.

Notizen an Zellen folgen den Zellen: Nach dem Sortieren oder nach dem Einfügen oder Löschen von Zeilen oder Spalten findet eine Suche in Notizen jede Notiz an der Zelle, an der sie jetzt steht. Eine Notiz an einer gelöschten Zelle ist für die Suche verloren (und steht nach **Rückgängig** wieder zur Verfügung). Wurde eine Arbeitsmappe so vielfach geändert, dass die Notizen nicht durchgehend nachverfolgt werden können, werden Sie gebeten, sie zuerst zu speichern. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, hat ihre Notizen in einer Form, die JeffCrunch nicht durchsucht. Sie erhalten dazu einen Hinweis.

## Ersetzen

Drücken Sie **Strg+H**, geben Sie den neuen Text unter **Ersetzen durch** ein, und wählen Sie eine Schaltfläche. **Ersetzen** setzt den neuen Text in die Zelle mit dem Cursor, sofern sie den Suchtext enthält, und springt dann zur nächsten Zelle. **Alle ersetzen** ersetzt den Text in allen Zellen, die ihn enthalten, als einen einzigen Schritt, den Sie rückgängig machen können. Eine große Änderung wird schrittweise ausgeführt. Der neue Text wird so eingegeben, als hätten Sie ihn getippt: Eine Zahl wird zur Zahl, und eine geänderte Formel bleibt eine Formel.

Um in Notizen zu ersetzen, wählen Sie auf der Registerkarte **Ersetzen** unter **Suchen in** die Option **Notizen**. **Alle ersetzen** ändert dann die Wörter jeder Notiz, die den Text enthält, auf diesem Blatt oder auf allen Blättern, als einen einzigen Schritt, den Sie rückgängig machen können. **Ersetzen** ändert die Notiz an der Zelle mit dem Cursor und springt dann zur nächsten. Eine Notiz, die als Kommentarverlauf angelegt ist, bleibt unverändert (ändern Sie diese auf der Registerkarte **Überprüfen**). Sie erfahren, wie viele solcher Notizen übersprungen wurden. Es wird nichts geändert, und Sie erhalten eine Begründung, wenn ein Blatt mit einer zu ändernden Notiz geschützt ist oder eine Notiz danach keinen Text mehr hätte bzw. mehr als 32.767 Zeichen enthielte. Eine Notiz, deren Wörter ersetzt werden, verliert das Datum, an dem sie geschrieben wurde, so wie beim Bearbeiten ihrer Wörter. Die Notizen werden mit der Arbeitsmappe gespeichert, in jeder Form, die Notizen enthält. **Werte** können durchsucht, aber nicht ersetzt werden, weil ein Wert das Ergebnis einer Formel ist.

Es wird nichts geändert, und Sie erhalten eine Begründung, wenn eine Zelle, die geändert werden müsste, auf einem geschützten Blatt gesperrt ist, Teil einer Matrix oder einer Datentabelle ist, in der ersten Zeile einer Tabelle steht oder danach keine Formel mehr wäre.

## Gehe zu

Drücken Sie **F5** (oder **Strg+G**), oder geben Sie etwas in das **Namenfeld** ein. Sie können eine Zelle wie B7 angeben, einen Bereich wie B3:D9, ganze Spalten oder Zeilen (C:E, 4:9), eine Zelle eines anderen Blatts (Tabelle2!A1 oder 'Mein Blatt'!A1) oder den Namen eines Bereichs.

## Nach Typ auswählen

Wählen Sie im Menü **Suchen** den Befehl **Nach Typ auswählen**, um Zellen nach ihrer Art auszuwählen:

- **Leere Zellen**, **Konstanten** (eingegeben), **Formeln** und **Zellen mit Fehleranzeige**. Bei Konstanten und Formeln können Sie Zahlen, Text, WAHR und FALSCH oder Fehler ausschließen.
- **Der Bereich um die Zelle mit dem Cursor** und **Die letzte verwendete Zelle**.
- **Die Matrixformel, in der sich der Cursor befindet** wählt alle Zellen dieser Matrixformel aus.
- **Zellen, die sich in ihrer Zeile** bzw. **in ihrer Spalte** von der Zelle mit dem Cursor unterscheiden, finden die Zellen, die nicht der Zelle mit dem Cursor entsprechen. Markieren Sie zuerst einen Bereich, und setzen Sie den Cursor auf die Zelle, mit der die anderen übereinstimmen sollen. In jeder Zeile (oder Spalte) des Bereichs wird eine Zelle erfasst, wenn sie eine andere Zahl oder einen anderen Text enthält oder wenn sie eine Formel enthält, die nach dem Verschieben ihrer Zellbezüge an die Position dieser Zelle nicht dieselbe ist. Eine Zelle mit einer Formel wird auch erfasst, wenn die Zelle mit dem Cursor keine Formel hat. Wenn Sie nur eine Zelle markiert haben, betrachtet JeffCrunch den Bereich um sie herum.
- **Sichtbare Zellen** (nicht in ausgeblendeten Zeilen oder Spalten), **Zellen mit bedingter Formatierung** und **Zellen mit Datenüberprüfung** (alle oder nur die mit derselben Regel wie die Zelle mit dem Cursor).

Der erste gefundene Block wird markiert, alle weiteren werden eingefärbt. Die Färbung verschwindet, wenn Sie auf eine Zelle klicken.
