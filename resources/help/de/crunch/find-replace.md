# Suchen, Ersetzen und Gehe zu
<!-- keywords: suchen, ersetzen, ändern, finden, Suchbegriff, Ersetzen durch, alle ersetzen, weitersuchen, suchen nach, ändern in, alle ändern, Groß- und Kleinschreibung, Platzhalter, Stern, Fragezeichen, Tilde, Groß-/Kleinschreibung beachten, ganze Zelle, nach Format suchen, Format, Notizen ersetzen, gehe zu, Bezug, Name, nach Art auswählen, Leerzellen, Konstanten, Formeln, Fehler, aktueller Bereich, aktuelle Matrix, Zeilenunterschiede, Spaltenunterschiede, letzte Zelle, sichtbare Zellen, bedingte Formatierung, Gültigkeit, Strg+F, Strg+H, F5, Notizen -->

## Suchen

Drücken Sie **Strg+F** (oder wählen Sie auf der Registerkarte **Start** die Option **Suchen** und dann noch einmal **Suchen**). Geben Sie den Text unter **Suchen nach** ein, und drücken Sie **Eingabe**, um zur nächsten Zelle mit diesem Text zu springen. Alternativ wählen Sie **Alle auflisten**, um eine Liste aller passenden Zellen zu erhalten. Klicken Sie auf eine Zeile der Liste, um zu dieser Zelle zu gelangen.

| Option | Was sie bewirkt |
| --- | --- |
| **Suchbereich** | Durchsucht dieses Blatt oder alle sichtbaren Blätter. Sind mehr als eine Zelle markiert, beschränkt sich die Suche in einem Blatt auf diese Zellen. |
| **Reihenfolge** | Durchläuft die Zellen zeilenweise (von links nach rechts, dann nach unten) oder spaltenweise (von oben nach unten, dann nach rechts). **Weitersuchen** bleibt beim ersten Treffer in dieser Reihenfolge stehen. |
| **Suchen in** | **Formeln** durchsucht den Inhalt, den Sie in jeder Zelle eingegeben haben, **Werte** die angezeigten Ergebnisse und **Notizen** die Notizen an den Zellen. |
| **Format…** | Findet nur Zellen mit einem bestimmten Format. **Format…** öffnet das Dialogfeld **Zellen formatieren**, ausgehend von der Zelle mit der Einfügemarke: Was Sie dort ändern, wird gesucht (etwa eine fette Schrift, eine Füllfarbe oder ein Zahlenformat). **Format aus Zelle** übernimmt alle Teile des Formats der Zelle mit der Einfügemarke, und **Format löschen** sucht wieder nur nach dem Text. Das Dialogfeld zeigt an, wonach gesucht wird, und weist darauf hin, wenn das gewählte Format keine Angaben enthält. Das Aussehen, das eine bedingte Formatierung einer Zelle gibt, wird nicht gesucht, sondern nur das Format, das die Zelle selbst hat. Ohne Text unter **Suchen nach** finden **Alle auflisten** und **Weitersuchen** die Zellen mit dem Format; mit Text muss eine Zelle beides haben. |
| **Groß-/Kleinschreibung beachten** | Groß- und Kleinschreibung müssen übereinstimmen. |
| **Nur ganze Zelle suchen** | Die ganze Zelle muss dem Text entsprechen, nicht nur ein Teil davon. |

Im Text unter **Suchen nach** steht **?** für ein beliebiges einzelnes Zeichen und **\*** für eine beliebige Folge von Zeichen (auch keines). Geben Sie **~** vor ?, \* oder ~ ein, um das Zeichen selbst zu suchen: **a~\*b** findet a\*b.

Eine Suche läuft schrittweise ab, sodass Sie weiterarbeiten können. **Esc** (oder **Abbrechen**) beendet sie, ebenso das Ändern einer Zelle. Ein sehr langer Suchtext kostet bei jedem Schritt mehr Zeit als ein kurzer, daher bleibt das Fenster bei einem Blatt mit langen Texten dennoch bedienbar.

Notizen folgen den Zellen: Nach dem Sortieren oder nach dem Einfügen oder Löschen von Zeilen oder Spalten findet eine Suche in Notizen jede Notiz an der Zelle, an der sie sich jetzt befindet. Eine Notiz an einer gelöschten Zelle ist aus der Suche verschwunden (und ist nach **Rückgängig** wieder da). Wurde eine Arbeitsmappe so vielfach geändert, dass die Notizen nicht durch alle Änderungen verfolgt werden können, werden Sie gebeten, sie zuerst zu speichern. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, hat ihre Notizen in einer Form, die JeffCrunch nicht durchsucht; Sie erhalten dazu einen Hinweis.

## Ersetzen

Drücken Sie **Strg+H**, geben Sie den neuen Text unter **Ersetzen durch** ein, und wählen Sie eine Schaltfläche. **Ersetzen** setzt den neuen Text in die Zelle mit der Einfügemarke, wenn sie den Text enthält, und springt dann zur nächsten. **Alle ersetzen** ändert jede betroffene Zelle in einem einzigen Schritt, den Sie rückgängig machen können; eine große Änderung wird schrittweise ausgeführt. Der neue Text wird so eingegeben, als hätten Sie ihn getippt: Eine Zahl wird zur Zahl, und eine geänderte Formel bleibt eine Formel.

Um in Notizen zu ersetzen, wählen Sie auf der Seite **Ersetzen** unter **Suchen in** die Option **Notizen**. **Alle ersetzen** ändert dann die Wörter jeder Notiz, die den Text enthält, auf diesem Blatt oder auf allen Blättern, in einem einzigen Schritt, den Sie rückgängig machen können. **Ersetzen** ändert die Notiz an der Zelle mit der Einfügemarke und springt zur nächsten. Eine Notiz, die ein Kommentarverlauf ist, bleibt unverändert (ändern Sie diese auf der Registerkarte **Überprüfen**); Sie erfahren, wie viele übersprungen wurden. Es wird nichts geändert, und Sie erfahren den Grund, wenn ein Blatt mit einer zu ändernden Notiz geschützt ist oder eine Notiz danach keinen Text mehr enthielte bzw. mehr als 32.767 Zeichen hätte. Eine Notiz, deren Wörter ersetzt werden, verliert das Datum, an dem sie geschrieben wurde, so wie beim Bearbeiten ihrer Wörter. Die Notizen werden mit der Arbeitsmappe gespeichert, in jeder Form, die Notizen unterstützt. **Werte** lassen sich durchsuchen, aber nicht ersetzen, weil ein Wert das ist, was eine Formel berechnet.

Es wird nichts geändert, und Sie erfahren den Grund, wenn eine zu ändernde Zelle auf einem geschützten Blatt gesperrt ist, Teil einer Matrix oder einer Datentabelle ist, in der ersten Zeile einer Tabelle steht oder danach keine Formel mehr wäre.

## Gehe zu

Drücken Sie **F5** (oder **Strg+G**), oder geben Sie etwas in das Namenfeld ein. Sie können eine Zelle wie B7 angeben, einen Bereich wie B3:D9, ganze Spalten oder Zeilen (C:E, 4:9), eine Zelle eines anderen Blatts (Tabelle2!A1 oder 'Mein Blatt'!A1) oder den Namen eines Bereichs. Ein Name, der für dieselben Zellen auf mehreren Blättern steht, etwa Jan:Dez!B2, führt Sie zum ersten Blatt dieses Bereichs.

## Nach Art auswählen

Wählen Sie im Menü **Suchen** den Befehl **Nach Art auswählen**, um Zellen nach ihrer Art zu markieren:

- **Leerzellen**, **Konstanten** (eingegeben), **Formeln** und **Zellen mit einem Fehler**. Bei Konstanten und Formeln können Sie Zahlen, Text, WAHR und FALSCH oder Fehler ausschließen.
- **Der aktuelle Bereich um die Zelle mit der Einfügemarke** und **Die letzte verwendete Zelle**.
- **Die Matrixformel mit der Einfügemarke** markiert alle Zellen dieser Matrixformel.
- **Zellen mit anderem Inhalt als die Zelle mit der Einfügemarke in ihrer Zeile** und **in ihrer Spalte** finden die Zellen, die der Zelle mit der Einfügemarke nicht gleichen. Markieren Sie zuerst einen Bereich, und setzen Sie die Einfügemarke auf die Zelle, der die anderen entsprechen sollen. In jeder Zeile (oder Spalte) des Bereichs wird eine Zelle ausgewählt, wenn sie eine andere Zahl oder einen anderen Text enthält oder wenn sie eine Formel enthält, die nicht dieselbe ist, sobald ihre Zellbezüge auf die Position dieser Zelle verschoben werden. Eine Zelle mit einer Formel wird auch ausgewählt, wenn die Zelle mit der Einfügemarke keine Formel hat. Wenn Sie nur eine Zelle markiert haben, betrachtet JeffCrunch den Bereich um sie herum.
- **Zellen, auf die die Formeln der Auswahl verweisen** und **Zellen mit Formeln, die auf die Auswahl verweisen** markieren die Zellen dieses Blatts, die von den Formeln der markierten Zellen verwendet werden, oder die Zellen, deren Formeln die markierten Zellen verwenden. Bei nur einer markierten Zelle beginnt die Suche bei dieser Zelle. Aktivieren Sie **Über alle Ebenen verfolgen**, um von jeder gefundenen Zelle aus weiterzusuchen (die Zellen, die diese verwenden, und so weiter). Ohne diese Option wird nur die erste Ebene markiert. Die Suche startet bei höchstens 1.000 der markierten Zellen, und die Statusleiste zeigt an, wenn eine Suche an einer Grenze abbricht.
- **Sichtbare Zellen** (nicht in ausgeblendeten Zeilen oder Spalten), **Zellen mit bedingter Formatierung** sowie **Zellen mit einer Regel für die Eingabe** (alle oder nur solche mit derselben Regel wie die Zelle mit der Einfügemarke).

Der erste gefundene Block wird markiert, alle weiteren werden farbig hinterlegt; die Hinterlegung verschwindet, sobald Sie auf eine Zelle klicken.
