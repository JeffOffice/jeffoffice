# Suchen, ersetzen und gehe zu
<!-- keywords: suchen, ersetzen, ändern, Suche, Platzhalter, Platzhalterzeichen, Sternchen, Fragezeichen, Tilde, Groß-/Kleinschreibung, gesamter Zellinhalt, gehe zu, Bezug, Name, nach Typ auswählen, Inhalte auswählen, leere Zellen, Leerzellen, Konstanten, Formeln, Fehler, aktueller Bereich, letzte Zelle, sichtbare Zellen, bedingte Formatierung, Datenüberprüfung, Gültigkeit, Strg+F, Strg+H, F5, Notizen -->

## Suchen

Drücken Sie **Strg+F** (oder wählen Sie auf der Registerkarte „Start“ **Suchen** und dann erneut **Suchen**). Geben Sie den Text ein, und drücken Sie die **Eingabetaste**, um zur nächsten Zelle mit diesem Text zu gelangen, oder wählen Sie **Alle auflisten**, um eine Liste aller übereinstimmenden Zellen zu erhalten. Klicken Sie auf eine Zeile der Liste, um zu dieser Zelle zu wechseln.

| Option | Funktion |
| --- | --- |
| **Ort** | Durchsucht dieses Blatt oder alle eingeblendeten Blätter. Wenn mehr als eine Zelle markiert ist, durchsucht die Suche in einem Blatt nur diese Zellen. |
| **Reihenfolge** | Durchläuft die Zellen zeilenweise (von links nach rechts, dann nach unten) oder spaltenweise (von oben nach unten, dann nach rechts). **Weiter** hält beim ersten Treffer in dieser Reihenfolge an. |
| **Suchen in** | **Formeln** durchsucht den in jede Zelle eingegebenen Inhalt, **Werte** den angezeigten Inhalt und **Notizen** die Notizen zu den Zellen. |
| **Groß-/Kleinschreibung beachten** | Groß- und Kleinbuchstaben müssen übereinstimmen. |
| **Gesamten Zellinhalt vergleichen** | Der gesamte Zellinhalt muss dem Text entsprechen, nicht nur ein Teil davon. |

Im Suchtext steht **?** für ein beliebiges einzelnes Zeichen und **\*** für eine beliebige Zeichenfolge (auch für keine Zeichen). Geben Sie **~** vor ?, \* oder ~ ein, um nach dem Zeichen selbst zu suchen: **a~\*b** findet a\*b.

Eine Suche läuft schrittweise ab, sodass Sie weiterarbeiten können. Mit **Esc** (oder **Abbrechen**) beenden Sie sie; auch das Ändern einer Zelle beendet die Suche. Ein sehr langer Text beansprucht mehr von jedem kleinen Schritt als ein kurzer, sodass das Fenster auch bei einem Blatt mit langen Texten bedienbar bleibt.

Notizen bleiben bei ihren Zellen: Nach einem Sortiervorgang oder nach dem Einfügen oder Löschen von Zeilen oder Spalten findet eine Suche in Notizen jede Notiz bei der Zelle, an der sie sich jetzt befindet. Eine Notiz zu einer gelöschten Zelle wird nicht mehr gefunden (nach „Rückgängig“ ist sie wieder da). Wurde eine Arbeitsmappe auf so viele Arten geändert, dass sich die Notizen nicht durch alle Änderungen verfolgen lassen, werden Sie aufgefordert, die Arbeitsmappe zuerst zu speichern. Bei einer Arbeitsmappe, die aus einem älteren Dateiformat oder aus einer Textdatei geöffnet wurde, liegen die Notizen in einer Form vor, die JeffCrunch nicht durchsucht; Sie erhalten dann einen entsprechenden Hinweis.

## Ersetzen

Drücken Sie **Strg+H**. **Ersetzen** fügt den neuen Text in die Zelle mit dem Cursor ein, wenn sie den gesuchten Text enthält, und geht zur nächsten Zelle weiter. **Alle ersetzen** ersetzt den Text in jeder Zelle, die ihn enthält, und zwar in einem Schritt, den Sie rückgängig machen können; umfangreiche Änderungen werden schrittweise vorgenommen. Der neue Text wird so übernommen, als hätten Sie ihn eingegeben: Eine Zahl wird zu einer Zahl, und eine geänderte Formel bleibt eine Formel.

Es wird nichts geändert, und Sie erhalten eine Begründung, wenn eine zu ändernde Zelle auf einem geschützten Blatt gesperrt ist, zu einer Matrix oder einer Datentabelle gehört, in der ersten Zeile einer Tabelle steht oder danach keine Formel mehr wäre.

## Gehe zu

Drücken Sie **F5** (oder **Strg+G**), oder geben Sie etwas in das Namenfeld ein. Sie können eine Zelle wie B7, einen Bereich wie B3:D9, ganze Spalten oder Zeilen (C:E, 4:9), eine Zelle eines anderen Blatts (Tabelle2!A1 oder 'Mein Blatt'!A1) oder den Namen eines Bereichs angeben.

## Nach Typ auswählen

Wählen Sie im Menü **Suchen** den Befehl **Nach Typ auswählen**, um Zellen nach ihrer Art auszuwählen:

- **Leere Zellen**, **Konstanten** (eingegeben), **Formeln** und **Zellen mit Fehleranzeige**. Bei Konstanten und Formeln können Sie Zahlen, Text, WAHR und FALSCH oder Fehler ausschließen.
- **Der Bereich um die Zelle mit dem Cursor** und **Die letzte verwendete Zelle**.
- **Sichtbare Zellen** (nicht in ausgeblendeten Zeilen oder Spalten), **Zellen mit bedingter Formatierung** und **Zellen mit Datenüberprüfung** (alle oder nur die mit derselben Regel wie die Zelle mit dem Cursor).

Der erste gefundene Bereich wird markiert, alle weiteren werden farbig hinterlegt; die Hervorhebung verschwindet, wenn Sie auf eine Zelle klicken.
