# Zellen ausfüllen und addieren
<!-- keywords: ausfüllen, Datenreihe, Reihe, ziehen, Ausfüllkästchen, AutoAusfüllen, Strg+D, Strg+R, nach unten, nach rechts, nach oben, nach links, Zahl, Datum, Wochentag, Monat, Jahr, Inkrement, Schrittweite, Endwert, kopieren, Summe, Summe, Mittelwert, Durchschnitt, Anzahl, Max, Min, Maximum, Minimum, Alt+Gleichheitszeichen, Ergebnis -->

## Ausfüllen

Beim Ausfüllen wird fortgesetzt, was Sie begonnen haben. Markieren Sie die bereits ausgefüllten Zellen, ziehen Sie dann das kleine Quadrat an der Ecke der Markierung (das **Ausfüllkästchen**) entlang einer Spalte oder Zeile, und lassen Sie los. Ein gestrichelter Rahmen zeigt, wie weit ausgefüllt wird.

| Sie beginnen mit | Das Ausfüllen ergibt |
| --- | --- |
| 1, 2 | 3, 4, 5 usw.: Der Abstand zwischen den Werten wird fortgesetzt |
| Einer einzelnen Zahl wie 5 | 5, 5, 5: Kopien (halten Sie beim Ziehen Strg gedrückt, um das Gegenteil zu erhalten) |
| Einem Datum | Die folgenden Tage (verwenden Sie die Schaltfläche **Ausfülloptionen** oder **Ausfüllen > Datenreihe** für Wochentage, Monate oder Jahre) |
| Mo oder Januar | Di, Mi, ... oder Februar, März, ... in Ihrer Sprache |
| Artikel 1 | Artikel 2, Artikel 3, ... |
| Einer Formel | Die kopierte Formel, deren Bezüge wie beim Einfügen angepasst werden |

**Ausfüllen > Nach unten** (**Strg+D**) und **Ausfüllen > Nach rechts** (**Strg+R**) kopieren die erste Zeile bzw. Spalte der Markierung in die übrigen Zellen. **Ausfüllen > Nach oben** und **Ausfüllen > Nach links** gehen von der letzten aus. **Ausfüllen > Datenreihe** fragt nach dem Typ (linear, geometrisch, Datum), der Zeiteinheit eines Datums, dem Inkrement und dem Endwert. Jede ausgefüllte Zelle erhält das Format der Zelle, auf die sie folgt. Eine Zelle, die nicht geändert werden kann (Teil einer Matrix, eine gesperrte Zelle oder die Kopfzeile einer Tabelle), bricht das Ausfüllen ab, und es wird nichts geändert.

Nachdem Sie das Ausfüllkästchen gezogen haben, erscheint an der Ecke des ausgefüllten Bereichs die kleine Schaltfläche **Ausfülloptionen**. Über ihr Menü wird das Ausfüllen auf andere Weise wiederholt, und es bleibt ein einziger Schritt für „Rückgängig“: **Zellen kopieren**, **Datenreihe ausfüllen**, **Nur Formate ausfüllen** (die Zellen behalten ihren Inhalt und übernehmen das Format), **Ohne Formatierung ausfüllen** (die Zellen behalten ihr eigenes Format) und bei Datumsangaben **Tage ausfüllen**, **Wochentage ausfüllen**, **Monate ausfüllen** und **Jahre ausfüllen**. Die Schaltfläche verschwindet, sobald Sie etwas anderes tun.

Das Ausfüllen erfolgt nicht in verbundene Zellen hinein oder aus ihnen heraus, weil sonst Werte in Zellen stehen würden, die der verbundene Bereich verdeckt. Heben Sie den Zellverbund zuerst auf.

**Ausfüllen > Blocksatz** verteilt den Text der markierten Zellen einer Spalte auf Zeilen, die so breit wie die Spalte sind, beginnend mit der ersten Zelle nach unten; übrige Zellen der Markierung werden geleert. Benötigt der Text mehr Zellen als markiert, fragt JeffCrunch, ob die Zellen darunter verwendet werden sollen. Dies funktioniert nur mit Text.

**Ausfüllen > Über Arbeitsblätter** kopiert die markierten Zellen in dieselben Zellen der Blätter, die Sie in einer Liste aktivieren: alles, nur die Inhalte oder nur die Formate. Dies ist ein einziger Schritt für „Rückgängig“.

## Summe

**Start > Summe** (**Alt+Gleichheitszeichen**) fügt in die markierte Zelle eine Summe ein. Dabei werden die Zahlen oberhalb der Zelle berücksichtigt, und falls dort keine stehen, die Zahlen links davon. Die Formel wird zur Kontrolle in die Zelle eingefügt; mit der **Eingabetaste** übernehmen Sie sie. Über den Pfeil neben „Summe“ erhalten Sie die anderen Ergebnisse: **Mittelwert**, **Anzahl Zahlen**, **Max** und **Min**.

Markieren Sie vorher einen Bereich, erhält jede Spalte des Bereichs eine eigene Formel in den leeren Zellen darunter (bei einer einzelnen Zeile rechts davon).
