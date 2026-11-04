# Zellen ausfüllen und addieren
<!-- keywords: ausfüllen, Reihe, ziehen, Ausfüllkästchen, Doppelklick, benutzerdefinierte Liste, AutoAusfüllen, Strg+D, Strg+R, nach unten, nach rechts, nach oben, nach links, Zahl, Datum, Wochentag, Monat, Jahr, Schrittweite, Stopp, kopieren, Summe, summieren, Mittelwert, Anzahl, Maximum, Minimum, Alt+Gleichheitszeichen, Gesamtsumme -->

## Ausfüllen

Mit Ausfüllen setzen Sie fort, was Sie begonnen haben. Markieren Sie die ausgefüllten Zellen, und ziehen Sie dann das kleine Quadrat an der Ecke der Markierung (das **Ausfüllkästchen**) entlang einer Spalte oder einer Zeile. Lassen Sie danach die Maustaste los. Ein gestrichelter Rahmen zeigt, wie weit das Ausfüllen reicht.

| Sie beginnen mit | Das Ausfüllen ergibt |
| --- | --- |
| 1, 2 | 3, 4, 5 und so weiter: Der Abstand zwischen ihnen wird fortgesetzt |
| Eine einzelne Zahl wie 5 | 5, 5, 5: Kopien (halten Sie beim Ziehen **Strg** gedrückt, um die gegenteilige Wirkung zu erzielen) |
| Ein Datum | Die folgenden Tage (verwenden Sie die Schaltfläche **Ausfülloptionen** oder **Start > Ausfüllen > Reihe ausfüllen** für Wochentage, Monate oder Jahre) |
| Mo oder Januar | Di, Mi, ... bzw. Februar, März, ... in Ihrer Sprache |
| Element 1 | Element 2, Element 3, ... |
| Ein Wort aus einer benutzerdefinierten Liste, etwa Klein | Das nächste Wort der Liste, nach dem letzten wieder das erste |
| Eine Formel | Die kopierte Formel, wobei sich ihre Bezüge wie beim Einfügen verschieben |

Doppelklicken Sie auf das Ausfüllkästchen, um nach unten bis zu der Zeile auszufüllen, in der die Daten neben der Markierung enden. JeffCrunch sieht zuerst in die Spalte links und danach in die Spalte rechts. Es hört an der ersten leeren Zelle dieser Spalte auf.

## Benutzerdefinierte Listen

Eine **benutzerdefinierte Liste** ist eine Reihe von Wörtern, die das Ausfüllen durchläuft, wie die Wochentage. Um eine Liste anzulegen, klicken Sie auf **Datei > Optionen > Benutzerdefinierte Listen**. Geben Sie jede Liste in eine eigene Zeile ein, und trennen Sie die Wörter durch Kommas, zum Beispiel `Klein, Mittel, Groß`. Klicken Sie auf **OK**. Geben Sie nun **Klein** in eine Zelle ein, und ziehen Sie das Ausfüllkästchen: Die Zellen werden mit Mittel, Groß, Klein und so weiter gefüllt. Groß- und Kleinschreibung spielen beim Suchen eines Worts in einer Liste keine Rolle, und die neuen Zellen übernehmen die Schreibweise der Zelle, mit der Sie begonnen haben. Eine Liste braucht mindestens zwei Wörter. Die Wochentage und die Monate sind bereits eingebaut.

**Start > Ausfüllen > Nach unten** (**Strg+D**) und **Start > Ausfüllen > Nach rechts** (**Strg+R**) kopieren die erste Zeile bzw. Spalte der Markierung über den Rest. **Nach oben** und **Nach links** arbeiten von der letzten aus. **Start > Ausfüllen > Reihe ausfüllen** fragt nach der Art (linear, geometrisch, Datum), der Einheit eines Datums, der Schrittweite und dem Endwert. Jede ausgefüllte Zelle übernimmt das Format der Zelle, auf die sie folgt. Eine Zelle, die sich nicht ändern lässt (etwa Teil einer Matrix, eine gesperrte Zelle oder eine Tabellenüberschrift), bricht das Ausfüllen ab, und es wird nichts geändert.

Nachdem Sie das Ausfüllkästchen gezogen haben, erscheint an der Ecke des ausgefüllten Bereichs die kleine Schaltfläche **Ausfülloptionen**. Ihr Menü füllt auf eine andere Weise noch einmal aus, und das Ausfüllen bleibt ein Schritt für **Rückgängig**: **Zellen kopieren**, **Reihe ausfüllen**, **Nur Formate ausfüllen** (die Zellen behalten ihren Inhalt und übernehmen das Format), **Ohne Formatierung ausfüllen** (die Zellen behalten ihr eigenes Format) und bei Datumswerten **Tage ausfüllen**, **Wochentage ausfüllen**, **Monate ausfüllen** und **Jahre ausfüllen**. Die Schaltfläche verschwindet, sobald Sie etwas anderes tun.

Ein Ausfüllen erfolgt nicht in verbundene Zellen hinein und nicht aus ihnen heraus, weil dabei Werte in Zellen bleiben würden, die der verbundene Bereich verdeckt. Heben Sie zuerst die Verbindung auf.

**Start > Ausfüllen > Blocksatz** verteilt den Text der markierten Zellen einer Spalte auf Zeilen, die so breit wie die Spalte sind, und zwar ab der ersten Zelle nach unten. Überzählige Zellen der Markierung werden geleert. Braucht der Text mehr Zellen als markiert, fragt JeffCrunch, ob die darunterliegenden Zellen verwendet werden sollen. Der Befehl funktioniert nur bei Text.

**Start > Ausfüllen > Über Arbeitsblätter ausfüllen** kopiert die markierten Zellen in dieselben Zellen der Blätter, die Sie in einer Liste ankreuzen: alles, nur den Inhalt oder nur die Formate. Der Vorgang ist ein Schritt für **Rückgängig**.

## Summe

**Start > Summe** (**Alt und Gleichheitszeichen**) setzt eine Summe in die markierte Zelle. Berücksichtigt werden zuerst die Zahlen darüber; gibt es keine, die Zahlen links davon. Die Formel wird zur Kontrolle in die Zelle eingetragen, und **Eingabe** übernimmt sie. Der Pfeil neben Summe bietet die weiteren Berechnungen an: **Mittelwert**, **Anzahl Zahlen**, **Maximum** und **Minimum**.

Markieren Sie zuerst einen Block, dann erhält jede Spalte des Blocks in den leeren Zellen darunter eine eigene Formel (bei einer einzelnen Zeile rechts davon).
