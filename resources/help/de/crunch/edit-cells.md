# Zellen bearbeiten
<!-- keywords: eingeben, eingabe, tippen, schreiben, eintragen, bearbeiten, ändern, ändern, zelle, zelleneingabe, bearbeitungsleiste, F2, doppelklick, Tabulator, Tab, Esc, abbrechen, rückgängig, wiederholen, löschen, entfernen, prozent, währung, geld, datum, uhrzeit, zahl, text, apostroph, formel, gleichheitszeichen, ersetzen, grenze, zeichen, zeichenanzahl, Eingabe, Wiederholen -->

Um einen Inhalt in eine Zelle zu schreiben, markieren Sie die Zelle und beginnen zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um den Eintrag zu übernehmen, oder **Esc**, um ihn zu verwerfen; die Zelle bleibt dann unverändert.

## Drei Wege zum Bearbeiten

- **Direkt in die Zelle tippen.** Markieren Sie eine Zelle, und tippen Sie. Das erste Zeichen beginnt den Eintrag, und die Zelle zeigt beim Tippen an, was Sie eingeben.
- **Inhalt der Zelle bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Schreibmarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste schreiben.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und tippen Sie dort. Auch dann zeigt die Zelle an, was Sie eingeben.

Wenn Sie direkt in eine Zelle tippen, beendet eine Pfeiltaste den Eintrag und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Schreibmarke im Text. Mit **F2** wechseln Sie zwischen beiden Verhalten.

## Eintrag abschließen

| Tasten | Was sie bewirken |
| --- | --- |
| Eingabe | Eintrag übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** es festlegt) |
| Umschalt+Eingabe | Eintrag übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eintrag in alle markierten Zellen schreiben und an der aktuellen Stelle bleiben |
| Tab | Eintrag übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eintrag übernehmen und nach links wechseln |
| Pfeiltasten | Eintrag übernehmen und eine Zelle weiterwechseln (nur, wenn Sie direkt in die Zelle getippt haben) |
| Esc | Eintrag verwerfen |
| Alt+Eingabe | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (im Eintrag, wenn Sie gerade einen eingeben) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Wörter anzeigen, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** bei mehreren markierten Zellen schreibt den Eintrag in alle. Eine Formel wird für die Zelle geschrieben, in der Sie tippen; in den anderen Zellen wird sie so behandelt, als wäre sie dorthin kopiert worden. Wenn Sie etwa **=A1** tippen, während der Bereich E6:F7 markiert ist, steht in F7 die Formel **=B2**. Das ist ein Schritt für **Rückgängig**.

**Alt+Pfeil nach unten** listet in einer Zelle die Wörter der Zellen darüber und darunter auf, sofern sie nicht durch eine leere Zelle getrennt sind. Jedes Wort erscheint in der Reihenfolge nur einmal. Wenn Sie eines auswählen, wird es eingetragen. Hat eine Zelle eine eigene Liste, zeigt **Alt+Pfeil nach unten** stattdessen diese Liste.

In **Datei > Optionen > Bearbeiten** legen Sie fest, ob **Eingabe** nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung stehen lässt.

Wenn Sie mit **Tab** mehrere Zellen in einer Zeile füllen und danach **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Markieren Sie zuerst einen Zellbereich, bewegen **Eingabe** und **Tab** die Markierung innerhalb dieses Bereichs.

## Was Sie eingeben, wird zu

JeffCrunch liest Ihre Eingaben so, wie es die Einstellungen Ihres Computers vorgeben. Dezimal- und Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält ein Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 4.3.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | den logischen Wert WAHR | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa ein Prozent- oder Datumsformat, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text speichern wollen, etwa einen Code **007**, tippen Sie vorher einen Apostroph: **'007**. Der Apostroph gehört nicht zum Text. Beim Bearbeiten der Zelle sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. **=B2*2** verdoppelt zum Beispiel die Zahl in B2. Namen von Funktionen dürfen Sie groß oder klein schreiben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind immer die englischen, in jeder Sprache: Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Trennzeichen dazwischen richten sich nach den Einstellungen Ihres Computers. Wo das Dezimalzeichen ein Komma ist, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion durch ein Semikolon und schreiben in einer Zahl ein Komma: **=SUM(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln genauso. Wo das Dezimalzeichen ein Punkt ist, trennen Sie die Argumente durch ein Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen Argumenten und Punkten in Zahlen, damit sie überall gleich geöffnet wird.

Wenn eine Formel einen Fehler hat, etwa eine Klammer, die nie geschlossen wird, nennt JeffCrunch den Fehler. Der Eintrag bleibt so, wie Sie ihn getippt haben, und die Schreibmarke steht in der Nähe des Fehlers, damit Sie ihn korrigieren können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192. Ist ein Eintrag länger, weist JeffCrunch Sie darauf hin.

## Zellen leeren

Markieren Sie die Zellen, und drücken Sie **Entf**. Das leert ihren Inhalt und behält ihre Formate; der Schritt lässt sich rückgängig machen. JeffCrunch leert auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** entfernt Inhalt, Formate, Notizen und Links der Zellen. **Formate löschen** behält den Inhalt und setzt das einfache Format zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, der Text der Zellen bleibt. Eine Auswahl ist abgeblendet, wenn die markierten Zellen dafür nichts zu entfernen haben.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, **Strg+Y** wiederholt sie. Beide Befehle finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Leeren. **Rückgängig** stellt die Zelle so wieder her, wie sie war: Wert, Formel und Format sowie die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Arbeitsspeicher nicht zu stark belastet wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die von ihr abhängen. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die anderen Zellen werden kurz danach aktualisiert; die Statusleiste zeigt **Berechnung** an, bis sie fertig sind. Sie können in der Zwischenzeit weitertippen.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, sofern die Arbeitsmappe es nicht anders vorsieht. JeffCrunch zeigt an, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Schreiben Sie einen neuen Namen in eine dieser Zellen, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Hat schon eine andere Spalte diesen Namen, erhält er eine Zahl angehängt; ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, aber nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle zurück; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen kommen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und addieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgröße und Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden dabei nicht geändert.
