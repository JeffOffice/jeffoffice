# In Zellen eingeben
<!-- keywords: Autokorrektur, typografische Anführungszeichen, Gedankenstrich, Bruch, Link, Adresse, fettes Wort, Teil einer Zelle, Rich Text, Teil eines Textes formatieren, Wort färben, eingeben, Eingabe, Tippen, Eingeben, bearbeiten, ändern, Zelle, Eintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Escape, abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Begrenzung, Zeichen -->

Um einen Wert in eine Zelle zu schreiben, wählen Sie die Zelle aus und beginnen Sie zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle so zu lassen, wie sie war.

## Drei Wege zu Beginn

- **Über die Zelle schreiben.** Wählen Sie eine Zelle aus und tippen Sie. Das erste Zeichen beginnt die Eingabe, und die Zelle zeigt an, was Sie eingeben.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2** oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **Die Bearbeitungsleiste nutzen.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld und tippen Sie dort. Auch dann zeigt die Zelle an, was Sie eingeben.

Wenn Sie über eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Verhalten.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** es festlegt) |
| Umschalt+Eingabe | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur wenn Sie über die Zelle geschrieben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine tippen) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** mit mehreren markierten Zellen schreibt das Eingegebene in alle. Eine Formel ist für die Zelle geschrieben, in der Sie sie eingeben, und in den übrigen Zellen wirkt sie wie eine Kopie davon: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Das ist ein Schritt für Rückgängig.

**Alt+Pfeil nach unten** in einer Zelle listet in der Reihenfolge und jeweils einmal die Wörter der Zellen darüber und darunter auf, die nicht durch eine leere Zelle getrennt sind. Wenn Sie ein Wort auswählen, wird es eingegeben. Hat eine Zelle eine eigene Liste, zeigt sie stattdessen diese Liste.

In **Datei > Optionen > Bearbeiten** legen Sie fest, ob **Eingabe** nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung stehen lässt.

Wenn Sie mit **Tab** mehrere Zellen in einer Zeile ausfüllen und dann **Eingabe** drücken, springt die Markierung zur nächsten Zeile, in die Spalte, in der Sie begonnen haben. Wenn Sie zuerst einen Zellbereich markieren, bewegen **Eingabe** und **Tab** die Markierung innerhalb dieses Bereichs.

## Automatische Korrektur bei der Eingabe

Wenn Sie nach einem Wort ein Leerzeichen oder ein Satzzeichen wie einen Punkt eingeben, kann JeffCrunch das Wort korrigieren, sobald Sie es abschließen. Dabei werden übliche Tippfehler behoben (etwa „teh“ zu „the“), zwei große Buchstaben am Wortanfang korrigiert, ein kleiner Buchstabe am Satzanfang oder bei einem Wochentag groß geschrieben und ein gerades Anführungszeichen durch ein typografisches ersetzt. Außerdem wird ein doppelter Bindestrich zwischen Wörtern zu einem Gedankenstrich, und 1/2, 1/4 und 3/4 werden zu ½, ¼ und ¾, wenn die Eingabe Wörter enthält. Eine Zahl wie 1/2, die allein eingegeben wird, bleibt wie gewohnt ein Datum oder ein Bruch.

Jede Korrektur ist ein eigener Schritt. Drücken Sie direkt nach einer Korrektur **Strg+Z**, um nur diese Korrektur zurückzunehmen und Ihre Eingabe zu behalten. In einer Formel wird nichts verändert: Eine Eingabe, die mit einem Gleichheitszeichen beginnt oder mit einem Plus- oder Minuszeichen und dann einer Formel, bleibt so, wie Sie sie eingegeben haben. Eine Internetadresse (etwa https://example.org) oder eine E-Mail-Adresse, die die ganze Eingabe ausmacht, wird beim Abschluss der Eingabe zu einem Link in der Zelle, sofern **Internetadressen beim Eingeben in Links umwandeln** aktiviert ist.

Sie schalten diese Funktionen in **Datei > Optionen > Dokumentprüfung** ein oder aus. Ein falsch geschriebenes Wort wird schon während der Eingabe markiert: siehe [Rechtschreibung](notes-links).

## Teil des Textes formatieren

Einzelne Wörter in einer Zelle können anders aussehen als der Rest. Zum Beispiel kann ein Wort fett oder rot sein.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie auf der Registerkarte **Start** die Befehle **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe** oder die Tasten Strg+B, Strg+I und Strg+U.
4. Drücken Sie Eingabe.

Nur die markierten Wörter ändern sich. Der übrige Text der Zelle behält das Aussehen der Zelle. Während der Bearbeitung zeigt das Eingabefeld die Wörter einheitlich, und nach dem Drücken von Eingabe zeigt die Zelle jeden Teil in seinem eigenen Aussehen. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder einen Schriftgrad ohne markierte Wörter aus, ändert sich wie gewohnt die ganze Zelle.

Zellen, die Formatierungen innerhalb des Textes aus einem anderen Programm enthalten, zeigen sie an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Eine Formatierung innerhalb eines Textes gilt für Text, nicht für Zahlen oder Formeln. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, behält die Wörter, beim Speichern erscheint jedoch ein Hinweis, dass die Formatierung innerhalb der Wörter nicht erhalten bleibt.

## Was aus Ihrer Eingabe wird

JeffCrunch liest Ihre Eingabe nach den Einstellungen Ihres Computers. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5 % |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 04.03.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | den logischen Wert WAHR | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Um etwas, das wie eine Zahl aussieht, als Text zu behalten, etwa einen Code **007**, tippen Sie vorher einen Apostroph: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Zum Beispiel verdoppelt **=B2*2** die Zahl in B2. Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind in allen Sprachen die englischen. Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür vorsieht. Die Zeichen zwischen den Argumenten richten sich nach den Einstellungen Ihres Computers. Ist das Dezimaltrennzeichen ein Komma, wie im Deutschen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in einer Zahl ein Komma: **=SUMME(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln ebenso an. Ist das Dezimaltrennzeichen ein Punkt, trennen Sie die Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen, sodass sie überall gleich geöffnet wird.

Wenn eine Formel einen Fehler hat, etwa eine Klammer, die nie geschlossen wird, teilt JeffCrunch Ihnen mit, was nicht stimmt. Die Eingabe bleibt dann so, wie Sie sie getippt haben, mit der Einfügemarke nahe der Stelle des Fehlers, damit Sie ihn beheben können. Erst wenn die Formel stimmt, wird etwas verändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192. JeffCrunch meldet Ihnen, wenn eine Eingabe länger ist.

## Zellen leeren

Markieren Sie die Zellen und drücken Sie **Entf**. Das leert den Inhalt der Zellen und behält ihre Formate; dieser Schritt lässt sich rückgängig machen. JeffCrunch leert auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** leert die Zellen samt Formaten, Notizen, Kommentaren (den Diskussionen mit Antworten) und Links. Ein Kommentarverlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch weist darauf hin, weil **Rückgängig** Kommentare nicht wiederherstellen könnte, die es nicht gelesen hat. **Formate löschen** behält den Inhalt der Zellen und setzt das einfache Format zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und Kommentare aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, die Zellen behalten ihren Text. Eine Auswahl ist abgeblendet, wenn die markierten Zellen nichts davon enthalten.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, **Strg+Y** holt sie wieder zurück. Beide Befehle finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so wieder her, wie sie war: Wert, Formel, Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Arbeitsspeicher nicht übermäßig belastet wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die von ihr abhängen. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die übrigen Zellen werden einen Moment später aktualisiert; die Statusleiste zeigt **Berechnung läuft** an, bis sie fertig sind. Sie können in der Zwischenzeit weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, sofern die Arbeitsmappe nicht etwas anderes angibt. JeffCrunch meldet es, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Tippen Sie in eine dieser Zellen einen neuen Namen, um die Spalte umzubenennen: Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Einen Namen, den eine andere Spalte bereits hat, erhält eine Zahl dahinter, ein Name kann nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatierung, Sortierung sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Breite von Spalten und Höhe von Zeilen ändern](sizes).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und die Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden nicht verändert.
