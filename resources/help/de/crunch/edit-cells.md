# Zellen bearbeiten
<!-- keywords: fetter Begriff, Teil einer Zelle, Rich Text, Textteil formatieren, Wort färben, Eingabe, eingeben, Zelle, Eintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Esc, abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Begrenzung, Zeichen, schreiben, Eingabe bearbeiten, ändern, Zelleninhalt, Tabelle eingeben -->

Um einen Wert in eine Zelle einzugeben, markieren Sie die Zelle und beginnen Sie zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle so zu belassen, wie sie war.

## Drei Arten, die Eingabe zu beginnen

- **Direkt in die Zelle schreiben.** Markieren Sie eine Zelle, und tippen Sie. Das erste Zeichen beginnt die Eingabe, und die Zelle zeigt während des Tippens an, was Sie eingeben.
- **Inhalt der Zelle bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste schreiben.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und tippen Sie dort. Die Zelle zeigt ebenfalls an, was Sie eingeben.

Wenn Sie direkt in eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Möglichkeiten.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** es festlegt) |
| Umschalt+Eingabe | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie direkt in der Zelle eingegeben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine eingeben) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** schreibt bei mehreren markierten Zellen die Eingabe in alle. Eine Formel wird für die Zelle geschrieben, in die Sie eingegeben haben; in den übrigen Zellen wird sie so gelesen, wie es eine Kopie wäre: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Das ist ein Schritt für Rückgängig.

**Alt+Pfeil nach unten** listet in einer Zelle in der Reihenfolge und jeweils einmal die Wörter aus den Zellen darüber und darunter auf, die nicht durch eine leere Zelle getrennt sind. Wenn Sie eines auswählen, wird es eingegeben. Hat die Zelle eine eigene Liste, zeigt sie stattdessen diese Liste an.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob **Eingabe** nach unten, rechts, oben oder links wechselt oder die Markierung nicht verschiebt.

Wenn Sie mit **Tab** nacheinander mehrere Zellen in einer Zeile ausfüllen und dann **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie vorher einen Zellbereich markieren, bewegen sich **Eingabe** und **Tab** innerhalb dieses Bereichs.

## Teil des Textes formatieren

Einige Wörter in einer Zelle können anders aussehen als der Rest, zum Beispiel ein fettes oder rotes Wort.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie **Fett**, **Kursiv**, **Unterstreichung**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe** auf der Registerkarte **Start**, oder Strg+B, Strg+I und Strg+U.
4. Drücken Sie Eingabe.

Nur die markierten Wörter ändern sich. Der Rest der Zelle behält das Aussehen der Zelle. Während Sie bearbeiten, zeigt das Eingabefeld die Wörter in einem einheitlichen Aussehen; nach **Eingabe** zeigt die Zelle jeden Teil in seinem eigenen Aussehen. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern. Wählen Sie eine Schriftart oder Schriftgröße, ohne Wörter zu markieren, ändert sich wie gewohnt die ganze Zelle.

Zellen mit Formatierung innerhalb des Textes aus einem anderen Programm zeigen diese Formatierung an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Formatierung innerhalb eines Textes gilt für Text, nicht für Zahlen oder Formeln. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, behält die Wörter; beim Speichern weist JeffCrunch darauf hin, dass die Formatierung innerhalb der Wörter nicht erhalten bleibt.

## Was Sie eingeben, wird zu

JeffCrunch liest Ihre Eingaben so, wie es die Einstellungen Ihres Computers vorgeben. Das Dezimalzeichen, das Tausendertrennzeichen, das Währungssymbol und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 04.03.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | der logische Wert WAHR | WAHR |
| #NV | der Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | der Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, mit Ausnahme einer Formel.

Wenn etwas wie eine Zahl aussehen soll, aber als Text gespeichert werden muss, etwa ein Code **007**, geben Sie vorher einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Zum Beispiel verdoppelt **=B2*2** die Zahl in B2. Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind immer die englischen, in jeder Sprache: Geben Sie **SUMME** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Zeichen zwischen den Teilen richten sich nach den Einstellungen Ihres Computers. Ist das Dezimalzeichen ein Komma, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in einer Zahl ein Komma: **=SUMME(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln ebenso an. Ist das Dezimalzeichen ein Punkt, trennen Sie die Argumente mit einem Komma: **=SUMME(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen, damit sie überall gleich geöffnet wird.

Hat eine Formel einen Fehler, etwa eine nicht geschlossene Klammer, sagt JeffCrunch, was nicht stimmt. Die Eingabe bleibt, wie Sie sie geschrieben haben, und die Einfügemarke steht in der Nähe des Fehlers, damit Sie ihn korrigieren können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192. Ist eine Eingabe länger, zeigt JeffCrunch dies an.

## Zellen leeren

Markieren Sie die Zellen, und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihr Format bleibt erhalten, und der Vorgang lässt sich rückgängig machen. JeffCrunch leert auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** entfernt Inhalt, Format, Notizen und Verknüpfungen der Zellen. **Formate löschen** behält den Inhalt und stellt das schlichte Format wieder her. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, der Text der Zellen bleibt. Ein Befehl ist abgeblendet, wenn die markierten Zellen dafür nichts zu löschen haben.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, und **Strg+Y** wiederholt sie. Die Befehle finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in einer Zelle ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so wieder her, wie sie war: Wert, Formel, Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Arbeitsspeicher nicht überlastet wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch die Formeln, die von ihr abhängen, im Hintergrund. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die übrigen Zellen werden kurz danach aktualisiert. Solange zeigt die Statusleiste **Berechnung läuft** an. Sie können inzwischen weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern.
- In einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, außer die Arbeitsmappe legt etwas anderes fest. JeffCrunch meldet, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den eine andere Spalte schon hat, erhält eine Zahl dahinter. Ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, aber nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und addieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden dadurch nicht geändert.
