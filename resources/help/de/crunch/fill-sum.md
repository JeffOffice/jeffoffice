# Zellen ausfüllen und summieren
<!-- keywords: ausfüllen, Reihe, ziehen, Ausfüllkästchen, Doppelklick, benutzerdefinierte Liste, AutoAusfüllen, Strg+D, Strg+R, nach unten, nach rechts, nach oben, nach links, Zahl, Datum, Wochentag, Monat, Jahr, Schritt, Stopp, kopieren, Summe, summieren, Mittelwert, Anzahl, Max, Min, Alt+Gleichheitszeichen, Gesamtsumme, Blitzvorschau, Strg+E, Muster, Namen teilen -->

## Ausfüllen

Mit Ausfüllen setzen Sie fort, was Sie begonnen haben. Markieren Sie die ausgefüllten Zellen, und ziehen Sie dann das kleine Quadrat an der Ecke der Markierung (das **Ausfüllkästchen**) entlang einer Spalte oder einer Zeile. Lassen Sie anschließend die Maustaste los. Ein gestrichelter Rahmen zeigt, wie weit der Bereich ausgefüllt wird.

| Ausgangswert | Ergebnis des Ausfüllens |
| --- | --- |
| 1, 2 | 3, 4, 5 und so weiter: Der Abstand zwischen beiden Werten wird fortgesetzt |
| Eine einzelne Zahl wie 5 | 5, 5, 5: Die Zahl wird kopiert (halten Sie beim Ziehen Strg gedrückt, um die Gegenoption zu erhalten) |
| Ein Datum | Die folgenden Tage (verwenden Sie die Schaltfläche **Ausfülloptionen** oder **Start > Ausfüllen > Reihe**, für Wochentage, Monate oder Jahre) |
| Mo oder Januar | Di, Mi, ... oder Februar, März, ... in Ihrer Sprache |
| Element 1 | Element 2, Element 3, ... |
| Ein Wort aus einer benutzerdefinierten Liste, etwa Klein | Das nächste Wort der Liste, nach dem letzten wieder das erste |
| Eine Formel | Die Formel wird kopiert, und ihre Bezüge werden wie beim Einfügen verschoben |

Doppelklicken Sie auf das Ausfüllkästchen, um nach unten bis zu der Zeile auszufüllen, in der die Daten neben der Markierung enden. JeffCrunch sieht zuerst in die Spalte links von der Markierung und dann in die Spalte rechts davon. Es hört an der ersten leeren Zelle dieser Spalte auf.

## Benutzerdefinierte Listen

Eine **benutzerdefinierte Liste** ist eine Reihe von Wörtern, die das Ausfüllen durchläuft, ähnlich wie die Wochentage. Um eine Liste anzulegen, klicken Sie auf **Datei > Optionen > Benutzerdefinierte Listen**. Geben Sie jede Liste in eine eigene Zeile ein, und trennen Sie die Wörter durch Kommas, zum Beispiel `Klein, Mittel, Groß`. Klicken Sie auf **OK**. Geben Sie nun **Klein** in eine Zelle ein, und ziehen Sie am Ausfüllkästchen: Die Zellen werden mit Mittel, Groß, Klein und so weiter gefüllt. Bei der Suche nach einem Wort in einer Liste spielt die Groß- und Kleinschreibung keine Rolle. Die neuen Zellen übernehmen jedoch die Schreibweise der Zelle, mit der Sie begonnen haben. Eine Liste braucht mindestens zwei Wörter. Die Wochentage und die Monate sind bereits eingebaut.

**Start > Ausfüllen > Nach unten** (**Strg+D**) und **Start > Ausfüllen > Nach rechts** (**Strg+R**) kopieren die erste Zeile oder Spalte der Markierung über den Rest. **Nach oben** und **Nach links** arbeiten von der letzten Zeile oder Spalte aus. **Start > Ausfüllen > Reihe** fragt nach der Art (linear, geometrisch, Datum), der Zeiteinheit bei Datumswerten, dem Schritt und dem Endwert. Jede ausgefüllte Zelle übernimmt das Format der Zelle, auf die sie folgt. Eine Zelle, die sich nicht ändern lässt (etwa Teil einer Matrix, eine gesperrte Zelle oder die Kopfzeile einer Tabelle), beendet das Ausfüllen, und es ändert sich nichts.

Nach dem Ziehen am Ausfüllkästchen erscheint am Eck des ausgefüllten Bereichs die kleine Schaltfläche **Ausfülloptionen**. Ihr Menü führt das Ausfüllen auf andere Weise noch einmal aus. Das Ausfüllen bleibt dabei ein einziger Schritt von Rückgängig: **Zellen kopieren**, **Reihe ausfüllen**, **Nur Formate ausfüllen** (die Zellen behalten ihren Inhalt und übernehmen das Format), **Ohne Formatierung ausfüllen** (die Zellen behalten ihr eigenes Format) und bei Datumswerten **Tage ausfüllen**, **Wochentage ausfüllen**, **Monate ausfüllen** und **Jahre ausfüllen**. Die Schaltfläche verschwindet, sobald Sie etwas anderes tun.

Ein Ausfüllen geht nicht in verbundene Zellen hinein und nicht aus ihnen heraus, weil sonst Werte in Zellen stünden, die der verbundene Bereich verdeckt. Heben Sie die Verbindung vorher auf.

**Start > Ausfüllen > Justieren** verteilt den Text der markierten Zellen einer Spalte so in Zeilen, dass sie die Breite der Spalte ausfüllen, beginnend in der ersten Zelle nach unten; übrige Zellen der Markierung werden geleert. Braucht der Text mehr Zellen als markiert, fragt JeffCrunch, ob die Zellen darunter verwendet werden sollen. Der Befehl wirkt nur auf Text.

**Start > Ausfüllen > Blitzvorschau** (**Strg+E**) füllt eine Spalte anhand eines Musters, das Sie vorgeben. Geben Sie das gewünschte Ergebnis in den ersten einen oder zwei Zellen einer Spalte von Hand ein, neben den Zellen, aus denen es stammt. Setzen Sie dann den Cursor in diese Spalte, und wählen Sie Blitzvorschau. JeffCrunch ermittelt, wie Ihre Beispiele aus den Zellen daneben entstanden sind, und füllt die leeren Zellen auf die gleiche Weise aus. Beispiel: In Spalte A stehen Namen wie **Schmidt, Anna**. In B1 geben Sie **Anna Schmidt** ein. Blitzvorschau füllt B2 und die darunterliegenden Zellen mit den übrigen Namen, die auf dieselbe Weise umgedreht werden. Dabei kann ein Wort aus einer Zelle übernommen werden, der erste Buchstabe eines Wortes oder die ersten oder letzten Buchstaben, in Großbuchstaben, Kleinbuchstaben oder unverändert. Es kann auch Text hinzufügen, der in allen Beispielen gleich ist, etwa ein Leerzeichen oder einen Punkt. Findet es kein Muster, teilt es Ihnen das mit. Geben Sie dann ein oder zwei weitere Beispiele ein, und versuchen Sie es erneut. Auch dies ist ein Schritt von Rückgängig.

**Start > Ausfüllen > Über Arbeitsblätter** kopiert die markierten Zellen in dieselben Zellen der Blätter, die Sie in einer Liste ankreuzen: den gesamten Inhalt, nur die Werte oder nur die Formate. Auch dies ist ein Schritt von Rückgängig.

## Summe

**Start > Summe** (**Alt+Gleichheitszeichen**) setzt eine Summe in die markierte Zelle. Die Funktion betrachtet die Zahlen oberhalb der Zelle; gibt es dort keine, die Zahlen links davon. Die Formel wird in die Zelle eingetragen, damit Sie sie prüfen können, und mit **Eingabe** übernehmen Sie sie. Der Pfeil neben Summe bietet weitere Berechnungen an: **Mittelwert**, **Anzahl**, **Max** und **Min**.

Markieren Sie zuerst einen Block, dann erhält jede Spalte des Blocks in den leeren Zellen darunter eine eigene Formel (bei einer einzelnen Zeile in den Zellen rechts davon).
