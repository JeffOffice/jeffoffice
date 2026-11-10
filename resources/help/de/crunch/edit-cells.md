# In Zellen eingeben
<!-- keywords: Autokorrektur, typografische Anführungszeichen, Gedankenstrich, Bruch, Link, Adresse, fett, Teil einer Zelle, Rich Text, Text teilweise formatieren, Wort färben, Eingabe, eingeben, Eingabe bearbeiten, ändern, Zelle, Eintrag, Bearbeitungsleiste, F2, Doppelklick, Tabulator, Escape, abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Begrenzung, Zeichen -->

Um einen Inhalt in eine Zelle einzugeben, markieren Sie die Zelle und beginnen zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle unverändert zu lassen.

## Drei Möglichkeiten zum Beginnen

- **Direkt in die Zelle schreiben.** Markieren Sie eine Zelle, und tippen Sie. Das erste Zeichen, das Sie eingeben, beginnt die Eingabe, und die Zelle zeigt Ihren Text während des Schreibens an.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2**, oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **Die Bearbeitungsleiste verwenden.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld, und schreiben Sie dort. Auch dann zeigt die Zelle Ihre Eingabe an.

Wenn Sie direkt in eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie von der einen in die andere Betriebsart.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** es festlegt) |
| Umschalt+Eingabe | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der aktuellen Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie direkt in die Zelle geschrieben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Einen neuen Zeilenumbruch in der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine schreiben) |
| Strg+Umschalt+; | Die aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Die Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** schreibt bei mehreren markierten Zellen die Eingabe in alle. Eine Formel wird für die Zelle geschrieben, in der Sie tippen, und in den übrigen Zellen wird sie so behandelt, als wäre sie dorthin kopiert worden: Wenn Sie bei markiertem Bereich E6:F7 **=A1** eingeben, ergibt sich in F7 **=B2**. Das ist ein einzelner Schritt für Rückgängig.

**Alt+Pfeil nach unten** listet in einer Zelle, in der Reihenfolge und jeweils einmal, die Wörter aus den Zellen oberhalb und unterhalb auf, die nicht durch eine leere Zelle getrennt sind. Ein Wort, das Sie auswählen, wird eingetippt. Hat eine Zelle eine eigene Liste, zeigt **Alt+Pfeil nach unten** stattdessen diese Liste.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob **Eingabe** nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung nicht verschiebt.

Wenn Sie mit **Tab** nacheinander mehrere Zellen in einer Zeile füllen und dann **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie vorher einen Zellbereich markieren, durchlaufen **Eingabe** und **Tab** diesen Bereich.

## Autokorrektur bei der Eingabe

Wenn Sie nach einem Wort ein Leerzeichen oder ein Zeichen wie einen Punkt eingeben, kann JeffCrunch das Wort korrigieren, sobald Sie es abschließen. Dabei werden typische Tippfehler behoben (etwa „teh“ zu „the“), zwei große Anfangsbuchstaben am Wortanfang korrigiert, ein kleiner Anfangsbuchstabe am Satzanfang oder bei einem Wochentag großgeschrieben und gerade Anführungszeichen in typografische umgewandelt. Außerdem werden zwei Bindestriche zwischen Wörtern in einen langen Gedankenstrich verwandelt, und 1/2, 1/4 und 3/4 werden zu ½, ¼ und ¾, wenn die Eingabe Wörter enthält. Eine Zahl wie 1/2, die allein eingegeben wird, bleibt wie immer ein Datum oder ein Bruch.

Jede Korrektur ist ein eigener Schritt. Drücken Sie direkt nach einer Korrektur **Strg+Z**, um nur diese Korrektur zurückzunehmen und Ihre Eingabe zu behalten. In Formeln wird nichts verändert: Eine Eingabe, die mit einem Gleichheitszeichen beginnt oder mit einem Plus- oder Minuszeichen und danach eine Formel enthält, bleibt so, wie Sie sie eingegeben haben. Eine Internetadresse (etwa https://example.org) oder eine E-Mail-Adresse, die die ganze Eingabe bildet, wird beim Abschließen zu einem Link in ihrer Zelle, wenn **Internetadressen beim Eingeben in Links umwandeln** aktiviert ist.

Diese Optionen schalten Sie unter **Datei > Optionen > Dokumentprüfung** ein oder aus. Ein falsch geschriebenes Wort wird schon während der Eingabe markiert: siehe [Rechtschreibung](notes-links).

## Teil des Textes formatieren

Einzelne Wörter in einer Zelle können anders aussehen als der Rest. Zum Beispiel kann ein Wort fett oder rot sein.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe** auf der Registerkarte **Start**, oder drücken Sie Strg+B, Strg+I und Strg+U.
4. Drücken Sie Eingabe.

Nur die markierten Wörter ändern sich. Der übrige Teil der Zelle behält das Aussehen der Zelle. Beim Bearbeiten zeigt das Eingabefeld die Wörter in einheitlichem Aussehen, und nach **Eingabe** zeigt die Zelle jeden Teil in seinem eigenen Aussehen. Wenn Sie später Wörter einfügen oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder Schriftgröße ohne markierte Wörter aus, ändert sich wie immer die ganze Zelle.

Zellen, die Formatierungen innerhalb des Textes aus einem anderen Programm enthalten, zeigen sie an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Formatierungen innerhalb eines Textes gelten für Text, nicht für Zahlen oder Formeln. Eine als ältere .xls-Datei oder als .ods gespeicherte Kopie behält die Wörter, und beim Speichern weist JeffCrunch darauf hin, dass die Formatierungen innerhalb der Wörter nicht erhalten bleiben.

## Was Sie eingeben, wird zu

JeffCrunch liest Ihre Eingaben entsprechend den Einstellungen Ihres Computers. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungssymbol sowie die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234.5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5 % |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 4.3.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | den Wahrheitswert WAHR | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer Formeln.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text behalten möchten, etwa einen Code **007**, tippen Sie zuerst einen Apostroph: **'007**. Der Apostroph gehört nicht zum Text. Beim Bearbeiten der Zelle sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. **=B2*2** verdoppelt zum Beispiel die Zahl in B2. Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie in Großbuchstaben.

Die Namen der Funktionen sind immer die englischen, in jeder Sprache: Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Zeichen zwischen den Teilen richten sich nach den Einstellungen Ihres Computers. Ist das Dezimaltrennzeichen ein Komma, wie in Deutsch und Französisch, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in einer Zahl ein Komma: **=SUM(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln genauso an. Ist das Dezimaltrennzeichen ein Punkt, trennen Sie die Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen, sodass sie überall gleich geöffnet wird.

Wenn eine Formel einen Fehler hat, etwa eine Klammer, die nie geschlossen wird, teilt JeffCrunch Ihnen mit, was nicht stimmt. Die Eingabe bleibt, wie Sie sie eingegeben haben, und die Einfügemarke steht in der Nähe des Fehlers, damit Sie ihn beheben können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192. JeffCrunch meldet Ihnen, wenn eine Eingabe länger ist.

## Zellen leeren

Markieren Sie die Zellen, und drücken Sie **Entf**. Dabei wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten, und der Vorgang lässt sich rückgängig machen. JeffCrunch leert jeweils bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** leert die Zellen samt ihren Formaten, Notizen, Kommentaren (den Verläufen mit Antworten) und Links. Ein Kommentarverlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch sagt Ihnen das, weil Rückgängig die Kommentare, die nicht gelesen wurden, nicht wiederherstellen könnte. **Formate löschen** behält den Inhalt der Zellen und setzt das schlichte Format zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und Kommentare aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, und die Zellen behalten ihren Text. Eine Auswahl ist abgeblendet, wenn die markierten Zellen dafür nichts zu löschen haben.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, und **Strg+Y** wiederholt sie. Beide Befehle finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so wieder her, wie sie war: ihren Wert, ihre Formel, ihr Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Speicherbedarf gering bleibt.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die davon abhängen. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die übrigen Zellen werden kurz darauf aktualisiert. Solange zeigt die Statusleiste **Berechnung läuft** an. Sie können inzwischen weiter schreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern.
- In einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Alle Zellen sind gesperrt, sofern die Arbeitsmappe es nicht anders festlegt. JeffCrunch weist Sie darauf hin, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Einen Namen, den bereits eine andere Spalte hat, erhält eine Zahl dahinter. Ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich ansehen, aber nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet dann das Programm, das sie öffnet, die Formel zu berechnen.
- Formatierung, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes).

## Sonderzeichen einfügen

Klicken Sie auf **Einfügen > Symbole > Symbol**, um das Dialogfeld „Symbol“ zu öffnen. Wählen Sie ein Zeichen aus, und klicken Sie auf **Einfügen**, oder doppelklicken Sie darauf. Es wird an der Einfügemarke der Eingabe eingefügt, die Sie gerade schreiben. Schreiben Sie nicht, öffnet sich der vorhandene Inhalt der Zelle, und das Symbol wird an seinem Ende angefügt; **Eingabe** schreibt es dann in die Zelle. Beispiel: Enthält eine Zelle das Wort *Gesamt* und Sie wählen das Eurozeichen, wird die Eingabe zu *Gesamt€*. Die zuletzt verwendeten Symbole stehen zuerst in der Liste. Die Schriftart der Zelle zeichnet das Symbol, und ein Symbol, das die Schriftart nicht enthält, wird als Kästchen angezeigt.

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden dadurch nicht geändert.
