# In Zellen eingeben
<!-- keywords: eingeben, Eingabe, tippen, bearbeiten, ändern, Zelle, Zelleneintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Tabulator, Esc, abbrechen, verwerfen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, überschreiben, Grenze, Zeichen -->

Um etwas in eine Zelle einzutragen, markieren Sie die Zelle und beginnen Sie zu tippen. Ihre Eingabe ersetzt den bisherigen Inhalt der Zelle. Drücken Sie die **Eingabetaste**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle unverändert zu lassen.

## Drei Möglichkeiten zu beginnen

- **Zelle überschreiben.** Markieren Sie eine Zelle und tippen Sie. Das erste Zeichen beginnt die Eingabe, und die Zelle zeigt während der Eingabe an, was Sie tippen.
- **Zelleninhalt bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und der Cursor steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **Bearbeitungsleiste verwenden.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld und tippen Sie dort. Die Zelle zeigt Ihre Eingabe ebenfalls an.

Wenn Sie eine Zelle überschreiben, schließt eine Pfeiltaste die Eingabe ab und verschiebt die Markierung. Wenn Sie den Zelleninhalt bearbeiten, bewegen die Pfeiltasten den Cursor im Text. Mit **F2** wechseln Sie zwischen beiden Modi.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabetaste | Eingabe übernehmen und nach unten wechseln |
| Umschalt+Eingabetaste | Eingabe übernehmen und nach oben wechseln |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiter wechseln (nur beim Überschreiben der Zelle) |
| Esc | Eingabe verwerfen |
| Alt+Eingabetaste | Neue Zeile innerhalb der Zelle beginnen |

Wenn Sie mit **Tab** mehrere Zellen in einer Zeile ausfüllen und dann die **Eingabetaste** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie zuerst einen Zellbereich markieren, bewegen **Eingabetaste** und **Tab** die Markierung innerhalb dieses Bereichs.

## Was aus Ihrer Eingabe wird

JeffCrunch liest Ihre Eingabe so, wie es die Einstellungen Ihres Computers vorgeben. Dezimaltrennzeichen, Tausendertrennzeichen, Währungssymbol und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt an |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 3.4.2025 | das Datum, als Anzahl von Tagen | 03.04.2025 |
| 22:30 | die Uhrzeit, als Bruchteil eines Tages | 22:30 |
| WAHR | den Wahrheitswert WAHR | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** speichert alles, was Sie eingeben, als Text – außer einer Formel.

Um etwas, das wie eine Zahl aussieht, als Text zu speichern, etwa den Code **007**, geben Sie zuerst ein Apostroph ein: **'007**. Das Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie es wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Beispielsweise verdoppelt **=B2*2** die Zahl in B2. Sie können Funktionsnamen groß oder klein schreiben; JeffCrunch schreibt sie in Großbuchstaben.

Funktionsnamen sind in jeder Sprache immer die englischen: Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Trennzeichen richten sich nach den Einstellungen Ihres Computers. Wo das Dezimaltrennzeichen ein Komma ist, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in Zahlen ein Komma: **=SUM(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln auf dieselbe Weise an. Wo das Dezimaltrennzeichen ein Punkt ist, trennen Sie Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. In der Datei wird immer die Form gespeichert, die jedes Programm lesen kann, mit Kommas zwischen den Argumenten und Punkten in Zahlen. So wird sie überall gleich geöffnet.

Enthält eine Formel einen Fehler, etwa eine nicht geschlossene Klammer, teilt JeffCrunch Ihnen mit, was falsch ist. Die Eingabe bleibt so, wie Sie sie getippt haben, und der Cursor steht in der Nähe des Fehlers, damit Sie ihn korrigieren können. Erst wenn die Formel richtig ist, wird etwas geändert.

Eine Zelle fasst bis zu 32.767 Zeichen, eine Formel bis zu 8.192. JeffCrunch weist Sie darauf hin, wenn eine Eingabe länger ist.

## Zellen leeren

Markieren Sie die Zellen und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten. Dies ist ein einzelner Schritt, den Sie rückgängig machen können. JeffCrunch leert bis zu 100.000 Zellen auf einmal.

## Rückgängig und Wiederholen

**{key:Undo}** macht die letzte Änderung rückgängig, **{key:Redo}** stellt sie wieder her. Beide Befehle finden Sie auch in der Symbolleiste für den Schnellzugriff oben. Jede Zelle, in die Sie etwas eingeben, ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so wieder her, wie sie war: ihren Wert, ihre Formel, ihr Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, weniger, wenn sehr viele Zellen geändert wurden, damit nie viel Arbeitsspeicher belegt wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch die davon abhängigen Formeln im Hintergrund. Die geänderte Zelle zeigt ihren neuen Inhalt sofort an, die anderen Zellen werden kurz darauf aktualisiert; bis dahin zeigt die Statusleiste **Berechnung läuft…** an. Sie können währenddessen weitertippen.

## Was nicht geändert werden kann

- Eine Zelle, die zu einer **Matrixformel** oder einer Datentabelle gehört, kann nicht einzeln geändert werden.
- Auf einem **geschützten Blatt** können gesperrte Zellen nicht geändert werden. Jede Zelle ist gesperrt, sofern die Arbeitsmappe nichts anderes festlegt. JeffCrunch weist Sie darauf hin, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte „Überprüfen“ den Befehl „Blattschutz aufheben“.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in eine dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den bereits eine andere Spalte trägt, erhält eine angehängte Zahl. Ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, kann angezeigt, aber nicht geändert oder gespeichert werden.
- Bei einer Funktion, die JeffCrunch noch nicht kennt, erhält eine neue Formel in der Zelle keinen Wert; die Arbeitsmappe fordert das Programm, das sie als Nächstes öffnet, auf, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten werden unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und addieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes) beschrieben.
