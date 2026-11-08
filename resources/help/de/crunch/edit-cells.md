# Zellen bearbeiten

<!-- keywords: fett, Wort in Zelle, Teil einer Zelle, Rich Text, Teil des Textes formatieren, Wort färben, Eingabe, Text eingeben, eingeben, Zelle, Eintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Esc, abbrechen, rückgängig, wiederholen, löschen, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Begrenzung, Zeichen, Eingabe bearbeiten, Zellinhalt, Inhalte, Tippen, überschreiben, Autovervollständigen, Liste, Eingeben, Enter, Ausfüllen -->

Um etwas in eine Zelle einzugeben, markieren Sie die Zelle und beginnen zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Enter**, um die Eingabe zu übernehmen, oder **Esc**, um sie abzubrechen. Dann bleibt die Zelle so, wie sie vorher war.

## Drei Möglichkeiten zu beginnen

- **Über die Zelle schreiben.** Markieren Sie eine Zelle, und tippen Sie. Das erste Zeichen, das Sie eingeben, beginnt die Eingabe, und die Zelle zeigt während des Schreibens an, was Sie eingeben.
- **Inhalt der Zelle bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und tippen Sie dort. Auch dann zeigt die Zelle an, was Sie eingeben.

Wenn Sie über eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Verhalten.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Enter | Eingabe übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** festlegt) |
| Umschalt+Enter | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Enter | Eingabe in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie über die Zelle geschrieben haben) |
| Esc | Eingabe abbrechen |
| Alt+Enter | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine eingeben) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Enter** mit mehreren markierten Zellen schreibt Ihre Eingabe in alle. Eine Formel wird für die Zelle geschrieben, in der Sie tippen; in den anderen Zellen wird sie so behandelt, als wäre sie dorthin kopiert worden. Wenn Sie etwa **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 die Formel **=B2**. Das ist ein Schritt für Rückgängig.

**Alt+Pfeil nach unten** listet in einer Zelle die Wörter der Zellen darüber und darunter auf, sortiert in der Reihenfolge des Auftretens und jedes Wort nur einmal. Dabei werden nur Zellen berücksichtigt, die nicht durch eine leere Zelle getrennt sind. Wenn Sie ein Wort auswählen, wird es eingegeben. Hat eine Zelle eine eigene Liste, zeigt Alt+Pfeil nach unten stattdessen diese Liste.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob **Enter** nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung stehen lässt.

Wenn Sie mit **Tab** nacheinander mehrere Zellen in einer Zeile ausfüllen und dann **Enter** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie zuvor einen Zellbereich markieren, durchlaufen **Enter** und **Tab** diesen Bereich.

## Teil des Textes formatieren

Einzelne Wörter in einer Zelle können anders aussehen als der Rest. Zum Beispiel kann ein Wort fett oder rot sein.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie auf der Registerkarte **Start** die Befehle **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe**, oder drücken Sie Strg+B, Strg+I und Strg+U.
4. Drücken Sie Enter.

Nur die markierten Wörter ändern sich. Der Rest der Zelle behält das eigene Aussehen der Zelle. Während Sie bearbeiten, zeigt das Eingabefeld die Wörter in einem einheitlichen Aussehen; nach Enter zeigt die Zelle jeden Teil in seiner eigenen Formatierung. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder einen Schriftgrad, ohne Wörter zu markieren, wird wie gewohnt die ganze Zelle geändert.

Zellen, die Formatierungen innerhalb des Textes aus einem anderen Programm enthalten, zeigen sie an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Formatierungen innerhalb eines Textes gelten für Text, nicht für Zahlen oder Formeln. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, behält die Wörter, aber die Formatierung innerhalb der Wörter geht verloren; beim Speichern erhalten Sie dazu einen Hinweis.

## Was Sie eingeben, wird zu

JeffCrunch liest Ihre Eingaben so, wie es die Einstellungen Ihres Computers vorsehen. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

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

Wenn Sie etwas, das wie eine Zahl aussieht, als Text behalten möchten, etwa einen Code **007**, geben Sie vorher einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Zum Beispiel verdoppelt **=B2*2** die Zahl in B2. Die Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind in jeder Sprache immer die englischen. Geben Sie also **SUMME** nicht ein, sondern die englische Form **SUM**. Die Trennzeichen dazwischen richten sich nach den Einstellungen Ihres Computers. Ist das Dezimaltrennzeichen ein Komma, wie im Deutschen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in einer Zahl ein Komma: **=SUM(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln auf dieselbe Weise an. Ist das Dezimaltrennzeichen ein Punkt, trennen Sie die Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert die Formel immer in der Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen. So öffnet sie sich überall gleich.

Hat eine Formel einen Fehler, etwa eine Klammer, die nicht geschlossen wird, teilt JeffCrunch mit, was nicht stimmt. Die Eingabe bleibt, wie Sie sie getippt haben, und die Einfügemarke steht in der Nähe des Fehlers, damit Sie ihn korrigieren können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192. Ist eine Eingabe länger, zeigt JeffCrunch einen Hinweis an.

## Zellen löschen

Markieren Sie die Zellen, und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten, und der Vorgang lässt sich rückgängig machen. JeffCrunch löscht auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** leert die Zellen, ihre Formate, ihre Notizen, ihre Kommentare (die Verläufe mit Antworten) und ihre Links. Ein Kommentarverlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch weist darauf hin, weil Rückgängig die Kommentare nicht wiederherstellen könnte, die es nicht gelesen hat. **Formate löschen** behält den Inhalt der Zellen und setzt das einfache Standardformat zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und Kommentare aus den markierten Zellen. **Hyperlinks entfernen** entfernt nur die Links, und die Zellen behalten ihren Text. Ein Befehl ist abgeblendet, wenn die markierten Zellen nichts enthalten, das er entfernen könnte.

## Rückgängig und Wiederholen

**{key:Undo}** macht die letzte Änderung rückgängig, und **{key:Redo}** führt sie erneut aus. Beide finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, und ebenso jedes Löschen. Rückgängig setzt die Zelle in den vorherigen Zustand zurück: Wert, Formel, Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Arbeitsspeicher nicht zu stark belastet wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die davon abhängen. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die übrigen Zellen werden kurz danach aktualisiert. In der Statusleiste steht **Berechnung läuft**, bis sie fertig sind. Sie können in der Zwischenzeit weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, sofern die Arbeitsmappe es nicht anders vorsieht. JeffCrunch zeigt einen Hinweis, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den eine andere Spalte bereits hat, erhält eine Zahl dahinter, ein Name darf nicht leer sein, und **Entf** löscht diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und addieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie die Schriftart, den Schriftgrad und die Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen bleiben unverändert.
