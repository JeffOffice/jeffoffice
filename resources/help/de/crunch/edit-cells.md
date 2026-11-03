# In Zellen eingeben
<!-- keywords: eingeben, Eingabe, tippen, bearbeiten, ändern, Zelle, Zelleneintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Esc, verwerfen, abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Grenze, Zeichen -->

Um etwas in eine Zelle einzugeben, markieren Sie die Zelle und beginnen zu tippen. Ihre Eingabe ersetzt den bisherigen Inhalt der Zelle. Drücken Sie die **Eingabetaste**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle unverändert zu lassen.

## Drei Möglichkeiten zu beginnen

- **Zelle überschreiben.** Markieren Sie eine Zelle, und tippen Sie. Mit dem ersten Zeichen beginnt die Eingabe, und die Zelle zeigt fortlaufend an, was Sie tippen.
- **Zellinhalt bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle wird mit ihrem Inhalt geöffnet, und der Cursor steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **Bearbeitungsleiste verwenden.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und tippen Sie dort. Auch die Zelle zeigt an, was Sie tippen.

Wenn Sie eine Zelle überschreiben, schließt eine Pfeiltaste die Eingabe ab und verschiebt die Markierung. Wenn Sie den Zellinhalt bearbeiten, bewegen die Pfeiltasten den Cursor im Text. Mit **F2** wechseln Sie zwischen beiden Modi.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabetaste | Eingabe übernehmen und nach unten wechseln (oder wie unter **Datei > Optionen > Bearbeiten** festgelegt) |
| Umschalt+Eingabetaste | Eingabe übernehmen und in die Gegenrichtung wechseln |
| Strg+Eingabetaste | Eingabe in alle markierten Zellen übernehmen und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiter wechseln (nur, wenn Sie die Zelle überschrieben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabetaste | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in die Eingabe, wenn Sie gerade tippen) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Nach-unten | Die bereits in der Spalte vorhandenen Wörter auflisten und eines auswählen |

Sind mehrere Zellen markiert, übernimmt **Strg+Eingabetaste** Ihre Eingabe in alle diese Zellen. Eine Formel wird für die Zelle geschrieben, in die Sie getippt haben, und lautet in jeder anderen Zelle so wie eine Kopie davon: Wenn Sie bei markiertem Bereich E6:F7 **=A1** eingeben, steht in F7 **=B2**. Das Ganze ist ein einziger Schritt zum Rückgängigmachen.

**Alt+Nach-unten** listet für eine Zelle sortiert und ohne Wiederholungen die Wörter in den Zellen darüber und darunter auf, die nicht durch eine leere Zelle getrennt sind. Wenn Sie eines auswählen, wird es eingegeben. Hat eine Zelle eine eigene Liste, wird stattdessen diese angezeigt.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob die **Eingabetaste** die Markierung nach unten, rechts, oben oder links verschiebt oder nicht verschiebt.

Wenn Sie mit **Tab** in mehreren Zellen einer Zeile nacheinander tippen und dann die **Eingabetaste** drücken, wechselt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie zuerst einen Zellbereich markieren, bewegen **Eingabetaste** und **Tab** die Markierung innerhalb des Bereichs.

## Was aus Ihrer Eingabe wird

JeffCrunch liest Ihre Eingabe gemäß den Einstellungen Ihres Computers. Dezimaltrennzeichen, Tausendertrennzeichen, Währungssymbol und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Geben Sie eine Zahl in eine Zelle ohne eigenes Format ein, erhält die Zelle das Format, das die Zahl so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 03.04.2025 | das Datum als Anzahl von Tagen | 03.04.2025 |
| 22:30 | die Uhrzeit als Bruchteil eines Tages | 22:30 |
| WAHR | den Wahrheitswert WAHR | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** speichert alles, was Sie eingeben, als Text – außer einer Formel.

Um etwas, das wie eine Zahl aussieht, als Text zu behalten, etwa den Code **007**, geben Sie zuerst ein Apostroph ein: **'007**. Das Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie es wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. **=B2*2** zum Beispiel verdoppelt die Zahl in B2. Funktionsnamen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie in Großbuchstaben.

Die Funktionsnamen sind in jeder Sprache die englischen: Geben Sie **SUM** ein, nicht SUMME. Die Trennzeichen richten sich nach den Einstellungen Ihres Computers. Wo das Dezimaltrennzeichen ein Komma ist, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion durch ein Semikolon und schreiben in Zahlen ein Komma: **=SUM(A1;A2)*0,5**. Bearbeitungsleiste und **F2** zeigen Formeln ebenso an. Wo das Dezimaltrennzeichen ein Punkt ist, trennen Sie die Argumente durch ein Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen, sodass sie überall gleich geöffnet wird.

Enthält eine Formel einen Fehler, etwa eine nicht geschlossene Klammer, teilt JeffCrunch Ihnen mit, was falsch ist. Die Eingabe bleibt wie eingegeben erhalten, und der Cursor steht in der Nähe des Fehlers, damit Sie ihn korrigieren können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle fasst bis zu 32.767 Zeichen, eine Formel bis zu 8.192. JeffCrunch weist Sie darauf hin, wenn eine Eingabe länger ist.

## Zellen leeren

Markieren Sie die Zellen, und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten, und Sie können den Vorgang in einem Schritt rückgängig machen. JeffCrunch leert bis zu 100.000 Zellen auf einmal.

## Rückgängig und Wiederholen

**{key:Undo}** macht die letzte Änderung rückgängig, **{key:Redo}** stellt sie wieder her. Beide Befehle finden Sie auch oben in der Schnellsymbolleiste. Jede Zelle, in die Sie etwas eingeben, ist ein Schritt, ebenso jedes Leeren. „Rückgängig“ stellt die Zelle so wieder her, wie sie war: Wert, Formel, Format und die davon abhängigen Formeln. JeffCrunch merkt sich bis zu 1.000 Schritte, weniger, wenn diese sehr viele Zellen geändert haben, damit nie viel Arbeitsspeicher belegt wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch die davon abhängigen Formeln im Hintergrund. Die geänderte Zelle zeigt ihren neuen Inhalt sofort, die anderen Zellen werden einen Moment später aktualisiert; bis dahin zeigt die Statusleiste **Berechnung läuft…** an. Sie können währenddessen weitertippen.

## Was nicht geändert werden kann

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, kann nicht einzeln geändert werden.
- Auf einem **geschützten Blatt** kann eine gesperrte Zelle nicht geändert werden. Jede Zelle ist gesperrt, sofern die Arbeitsmappe nichts anderes festlegt. JeffCrunch weist Sie darauf hin, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte „Überprüfen“ den Befehl „Blattschutz aufheben“.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in eine dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den bereits eine andere Spalte hat, erhält eine angehängte Zahl, ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, kann angezeigt, aber weder geändert noch gespeichert werden.
- Eine Funktion, die JeffCrunch noch nicht unterstützt, lässt eine neue Formel in der Zelle ohne Wert; die Arbeitsmappe fordert das Programm, das sie als Nächstes öffnet, auf, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten werden unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und addieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes) beschrieben.
