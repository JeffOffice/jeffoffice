# In Zellen eingeben
<!-- keywords: eingeben, Eingabe, Eingabetaste, bearbeiten, ändern, Zelle, Zelleneintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Tabulator, Esc, abbrechen, verwerfen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, überschreiben, Grenze, Zeichen -->

Um etwas in eine Zelle einzugeben, markieren Sie die Zelle und beginnen Sie mit der Eingabe. Ihre Eingabe ersetzt den bisherigen Inhalt der Zelle. Drücken Sie die **Eingabetaste**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle unverändert zu lassen.

## Drei Möglichkeiten, die Eingabe zu beginnen

- **Zelle überschreiben.** Markieren Sie eine Zelle, und tippen Sie los. Das erste eingegebene Zeichen beginnt die Eingabe, und die Zelle zeigt fortlaufend an, was Sie eingeben.
- **Zelleninhalt bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle wird mit ihrem Inhalt geöffnet, und der Cursor steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **Bearbeitungsleiste verwenden.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und geben Sie dort ein. Die Zelle zeigt Ihre Eingabe ebenfalls an.

Wenn Sie eine Zelle überschreiben, schließt eine Pfeiltaste die Eingabe ab und verschiebt die Markierung. Wenn Sie den Zelleninhalt bearbeiten, bewegen die Pfeiltasten den Cursor im Text. Mit **F2** wechseln Sie zwischen beiden Modi.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabetaste | Eingabe übernehmen und nach unten wechseln (oder wie unter **Datei > Optionen > Bearbeiten** festgelegt) |
| Umschalt+Eingabetaste | Eingabe übernehmen und in die Gegenrichtung wechseln |
| Strg+Eingabetaste | Eingabe in alle markierten Zellen übernehmen und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiter wechseln (nur beim Überschreiben der Zelle) |
| Esc | Eingabe verwerfen |
| Alt+Eingabetaste | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (während einer Eingabe in die Eingabe) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Nach-unten | Die bereits in der Spalte vorhandenen Wörter auflisten und eines auswählen |

**Strg+Eingabetaste** übernimmt Ihre Eingabe in alle markierten Zellen, wenn mehrere Zellen markiert sind. Eine Formel gilt für die Zelle, in die Sie eingegeben haben, und lautet in jeder anderen Zelle so, wie eine Kopie davon lauten würde: Wenn Sie bei markiertem Bereich E6:F7 **=A1** eingeben, steht in F7 **=B2**. Dies ist ein einziger Schritt für „Rückgängig“.

**Alt+Nach-unten** listet in einer Zelle die Wörter aus den Zellen darüber und darunter auf, die nicht durch eine leere Zelle getrennt sind – sortiert und jedes nur einmal. Wenn Sie eines auswählen, wird es eingegeben. Hat eine Zelle eine eigene Liste, wird stattdessen diese angezeigt.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob die **Eingabetaste** die Markierung nach unten, rechts, oben oder links verschiebt oder ob sie an der Stelle bleibt.

Wenn Sie in einer Zeile mehrere Zellen mit **Tab** ausfüllen und dann die **Eingabetaste** drücken, wechselt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie zuerst einen Zellbereich markieren, bewegen sich **Eingabetaste** und **Tab** innerhalb dieses Bereichs.

## Was aus Ihrer Eingabe wird

JeffCrunch interpretiert Ihre Eingabe gemäß den Einstellungen Ihres Computers. Dezimaltrennzeichen, Tausendertrennzeichen, Währungssymbol und die Reihenfolge von Tag und Monat in einem Datum stammen alle aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Eingabe | Inhalt der Zelle | Anzeige |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 3.4.2025 | das Datum als Anzahl von Tagen | 03.04.2025 |
| 22:30 | die Uhrzeit als Bruchteil eines Tages | 22:30 |
| WAHR | der Wahrheitswert WAHR | WAHR |
| #NV | der Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** speichert alle Eingaben als Text, außer Formeln.

Um etwas, das wie eine Zahl aussieht, als Text zu speichern, etwa einen Code **007**, geben Sie zuerst einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. So verdoppelt beispielsweise **=B2*2** die Zahl in B2. Funktionsnamen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie in Großbuchstaben.

Die Funktionsnamen sind in jeder Sprache die englischen: Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Trennzeichen richten sich nach den Einstellungen Ihres Computers. Wo das Dezimaltrennzeichen ein Komma ist, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in Zahlen ein Komma: **=SUM(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln ebenso an. Wo das Dezimaltrennzeichen ein Punkt ist, trennen Sie Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm lesen kann – mit Kommas zwischen Argumenten und Punkten in Zahlen –, sodass sie sich überall gleich öffnet.

Enthält eine Formel einen Fehler, etwa eine nicht geschlossene Klammer, teilt JeffCrunch Ihnen mit, was falsch ist. Die Eingabe bleibt so, wie Sie sie eingegeben haben, und der Cursor steht in der Nähe des Fehlers, damit Sie ihn korrigieren können. Erst wenn die Formel richtig ist, wird etwas geändert.

Eine Zelle fasst bis zu 32.767 Zeichen, eine Formel bis zu 8.192. JeffCrunch weist Sie darauf hin, wenn eine Eingabe länger ist.

## Zellen leeren

Markieren Sie die Zellen, und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten. Dies ist ein einziger Schritt, den Sie rückgängig machen können. JeffCrunch leert bis zu 100.000 Zellen auf einmal.

## Rückgängig und Wiederholen

**{key:Undo}** macht die letzte Änderung rückgängig, **{key:Redo}** wiederholt sie. Beide Befehle finden Sie auch oben in der Schnellsymbolleiste. Jede Zelle, in die Sie eingeben, ist ein Schritt, ebenso jedes Leeren. „Rückgängig“ stellt die Zelle wieder her, wie sie war: ihren Wert, ihre Formel, ihr Format und die davon abhängigen Formeln. JeffCrunch merkt sich bis zu 1.000 Schritte, weniger, wenn sehr viele Zellen geändert wurden, damit nie viel Arbeitsspeicher belegt wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch die davon abhängigen Formeln im Hintergrund. Die geänderte Zelle zeigt ihren neuen Inhalt sofort an, die anderen Zellen werden einen Moment später aktualisiert; bis dahin zeigt die Statusleiste **Berechnung läuft** an. Sie können währenddessen weiter eingeben.

## Was nicht geändert werden kann

- Eine Zelle, die zu einer **Matrixformel** oder einer Datentabelle gehört, kann nicht einzeln geändert werden.
- Auf einem **geschützten Blatt** können gesperrte Zellen nicht geändert werden. Jede Zelle ist gesperrt, sofern die Arbeitsmappe nichts anderes festlegt. JeffCrunch weist Sie darauf hin, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte „Überprüfen“ den Befehl „Blattschutz aufheben“.
- Die erste Zeile einer **Tabelle** enthält die Spaltennamen. Geben Sie in eine dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle bleibt in der Datei gültig. Ein Name, den bereits eine andere Spalte hat, erhält eine angehängte Zahl, ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, kann angezeigt, aber weder geändert noch gespeichert werden.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel in der Zelle ohne Wert; die Arbeitsmappe fordert das Programm, das sie als Nächstes öffnet, auf, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten werden in [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und addieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes) beschrieben.
