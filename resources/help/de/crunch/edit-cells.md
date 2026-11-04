# Zellen bearbeiten
<!-- keywords: eingeben, eingabe, bearbeiten, ändern, zelle, zellen, eintrag, bearbeitungsleiste, F2, Doppelklick, Tab, Esc, abbrechen, rückgängig, wiederholen, wiederherstellen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Grenze, Zeichen, Zeichenanzahl, eingeben in Zelle, Zelleneingabe -->

Um etwas in eine Zelle zu schreiben, markieren Sie die Zelle und beginnen Sie mit der Eingabe. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle unverändert zu lassen.

## Drei Möglichkeiten, die Eingabe zu beginnen

- **Direkt in die Zelle schreiben.** Markieren Sie eine Zelle, und tippen Sie los. Das erste Zeichen beginnt die Eingabe, und die Zelle zeigt an, was Sie während der Eingabe tippen.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste schreiben.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und geben Sie dort ein. Auch dann zeigt die Zelle an, was Sie eingeben.

Wenn Sie direkt in eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Verhalten.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** es festlegt) |
| Umschalt+Eingabe | Eingabe übernehmen und in die entgegengesetzte Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der aktuellen Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie direkt in die Zelle geschrieben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Innerhalb der Zelle eine neue Zeile beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine Eingabe schreiben) |
| Strg+Umschalt+; | Die aktuelle Uhrzeit einfügen |
| Alt+Nach-unten | Die Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** bei mehreren markierten Zellen schreibt Ihre Eingabe in alle von ihnen. Eine Formel wird für die Zelle geschrieben, in der Sie tippen, und in den übrigen Zellen wirkt sie so, als wäre sie dorthin kopiert worden: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Das ist ein einziger Schritt für **Rückgängig**.

**Alt+Nach-unten** in einer Zelle listet in der Reihenfolge der Zellen oberhalb und unterhalb auf, jedes Wort nur einmal, sofern diese nicht durch eine leere Zelle getrennt sind. Ein Wort, das Sie auswählen, wird eingegeben. Hat eine Zelle eine eigene Liste, zeigt Alt+Nach-unten stattdessen diese Liste an.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob die **Eingabe**-Taste nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung nicht bewegt.

Wenn Sie mit **Tab** nacheinander mehrere Zellen in einer Zeile ausfüllen und dann **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Markieren Sie zuerst einen Zellbereich, bewegen **Eingabe** und **Tab** die Markierung innerhalb dieses Bereichs.

## Was aus Ihrer Eingabe wird

JeffCrunch liest Ihre Eingaben so, wie es die Einstellungen Ihres Computers vorgeben. Dezimaltrennzeichen, Tausendertrennzeichen, Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält ein Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 04.03.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | den logischen Wert WAHR | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text speichern möchten, etwa einen Code **007**, geben Sie vorab einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Zum Beispiel verdoppelt **=B2*2** die Zahl in B2. Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind immer die englischen, in allen Sprachen. Geben Sie also **SUM** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Zeichen zwischen den Teilen einer Formel richten sich nach den Einstellungen Ihres Computers. Ist das Dezimaltrennzeichen ein Komma, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion durch ein Semikolon und schreiben in einer Zahl ein Komma: **=SUMME(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln ebenso an. Ist das Dezimaltrennzeichen ein Punkt, trennen Sie die Argumente durch ein Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert die Formeln immer in der Form, die jedes Programm lesen kann, mit Kommas zwischen den Argumenten und Punkten in Zahlen. So öffnet sie sich überall gleich.

Enthält eine Formel einen Fehler, etwa eine Klammer, die nie geschlossen wird, teilt JeffCrunch Ihnen mit, was nicht stimmt. Die Eingabe bleibt, wie Sie sie geschrieben haben, und die Einfügemarke steht in der Nähe des Fehlers, damit Sie ihn beheben können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle nimmt bis zu 32.767 Zeichen auf, eine Formel bis zu 8.192. Ist eine Eingabe länger, weist JeffCrunch Sie darauf hin.

## Zellen leeren

Markieren Sie die Zellen, und drücken Sie **Entf**. Dabei wird der Inhalt gelöscht, die Formate bleiben erhalten, und Sie können den Schritt rückgängig machen. JeffCrunch leert auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** entfernt Inhalt, Formate, Notizen und Links der Zellen. **Formate löschen** behält den Inhalt und setzt das einfache Format zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links; der Text der Zellen bleibt. Ein Befehl ist abgeblendet, wenn die markierten Zellen nichts enthalten, was er entfernen könnte.

## Rückgängig und Wiederholen

**{key:Undo}** macht die letzte Änderung rückgängig, und **{key:Redo}** führt sie erneut aus. Beide Befehle finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so wieder her, wie sie war: Wert, Formel, Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Speicherbedarf gering bleibt.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die von ihr abhängen. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die übrigen Zellen werden einen Moment später aktualisiert. Die Statusleiste zeigt **Berechnung läuft** an, bis alles fertig ist. Sie können in der Zwischenzeit weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht einzeln ändern.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Alle Zellen sind gesperrt, außer die Arbeitsmappe legt etwas anderes fest. JeffCrunch meldet, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Hat schon eine andere Spalte diesen Namen, wird eine Zahl angehängt. Ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet dann das Programm, das sie öffnet, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes).
