# Zellen bearbeiten
<!-- keywords: Symbol, Symbolschrift, Sonderzeichen, Tastenkürzel für ein Symbol, AutoKorrektur, typografische Anführungszeichen, Strich, Bruch, Link, Adresse, fetter Text, Teil einer Zelle, Rich Text, Teil eines Textes formatieren, Wort färben, eingeben, Eingabe, Tippen, Schreiben, bearbeiten, ändern, Zelle, Eingabe, Bearbeitungsleiste, F2, Doppelklick, Tab, Escape, Abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Grenze, Zeichen, Zelleninhalt, Texteingabe, Zelleneingabe -->

Um etwas in eine Zelle zu schreiben, markieren Sie die Zelle und beginnen Sie mit der Eingabe. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle so zu lassen, wie sie war.

## Drei Wege, um zu beginnen

- **Über die Zelle schreiben.** Markieren Sie eine Zelle und geben Sie etwas ein. Das erste Zeichen beginnt die Eingabe, und die Zelle zeigt Ihren Text während der Eingabe an.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2** oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **Die Bearbeitungsleiste verwenden.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld und geben Sie dort ein. Auch dann zeigt die Zelle Ihre Eingabe an.

Wenn Sie über eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Arten.

## Eingabe abschließen

| Tasten | Was sie bewirken |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder dorthin, wie es unter **Datei > Optionen > Bearbeiten** festgelegt ist) |
| Umschalt+Eingabe | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie über die Zelle geschrieben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Innerhalb der Zelle eine neue Zeile beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine schreiben) |
| Strg+Umschalt+; | Die aktuelle Uhrzeit einfügen |
| Alt+Nach-unten | Die Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** bei mehreren markierten Zellen schreibt Ihre Eingabe in alle. Eine Formel wird für die Zelle geschrieben, in der Sie getippt haben; in den anderen Zellen verhält sie sich so, wie es eine Kopie davon täte: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Es ist ein Schritt für **Rückgängig**.

**Alt+Nach-unten** auf einer Zelle listet in der Reihenfolge, in der sie stehen, und jedes Wort nur einmal, die Wörter in den Zellen oberhalb und unterhalb auf, die nicht durch eine leere Zelle getrennt sind. Wenn Sie eines auswählen, wird es eingegeben. Hat die Zelle eine eigene Liste, zeigt sie stattdessen diese Liste an.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob die **Eingabe**-Taste nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung nicht bewegt.

Wenn Sie mit **Tab** mehrere Zellen in einer Zeile beschreiben und dann **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie vorher einen Zellbereich markieren, bewegen sich **Eingabe** und **Tab** durch diesen Bereich.

## AutoKorrektur bei der Eingabe

Wenn Sie nach einem Wort ein Leerzeichen oder ein Satzzeichen wie einen Punkt eingeben, kann JeffCrunch das Wort korrigieren, sobald Sie es abgeschlossen haben. Die Korrektur behebt übliche Tippfehler (etwa „teh“ statt „the“), zwei große Buchstaben am Wortanfang, einen kleinen Buchstaben am Satzanfang oder bei einem Wochentag sowie gerade Anführungszeichen (sie werden zu typografischen). Außerdem wird ein Doppelbindestrich zwischen Wörtern zu einem langen Strich, und 1/2, 1/4 und 3/4 werden zu ½, ¼ und ¾, wenn die Eingabe Wörter enthält. Eine Zahl wie 1/2, die allein eingegeben wird, bleibt wie immer ein Datum oder ein Bruch.

Jede Korrektur ist ein eigener Schritt. Drücken Sie direkt nach einer Korrektur **Strg+Z**, um nur diese Korrektur zurückzunehmen und Ihre Eingabe zu behalten. In einer Formel wird nichts geändert: Eine Eingabe, die mit einem Gleichheitszeichen oder mit einem Plus- oder Minuszeichen und dann einer Formel beginnt, bleibt so, wie Sie sie eingegeben haben. Eine Webadresse (etwa https://example.org) oder eine E-Mail-Adresse, die die ganze Eingabe bildet, wird beim Abschließen der Eingabe zu einem Link in der Zelle, wenn **Webadressen beim Eingeben automatisch in Links umwandeln** aktiviert ist.

Diese Optionen schalten Sie unter **Datei > Optionen > Dokumentprüfung** ein oder aus. Ein falsch geschriebenes Wort wird während der Eingabe markiert: siehe [Rechtschreibung](notes-links).

## Einen Teil des Textes formatieren

Manche Wörter in einer Zelle können anders aussehen als der Rest. Zum Beispiel kann ein Wort fett oder rot sein.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe** auf der Registerkarte **Start** oder Strg+B, Strg+I und Strg+U.
4. Drücken Sie die Eingabetaste.

Nur die markierten Wörter ändern sich. Der übrige Text der Zelle behält das Aussehen der Zelle. Das Eingabefeld zeigt die Wörter während der Bearbeitung in einem Aussehen, und die Zelle zeigt jeden Teil in seinem eigenen Aussehen, sobald Sie die Eingabetaste drücken. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder einen Schriftgrad, ohne dass Wörter markiert sind, so ändert sich wie immer die ganze Zelle.

Zellen, die Formatierungen innerhalb des Textes aus einem anderen Programm enthalten, zeigen sie an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Formatierungen innerhalb eines Textes gelten für Text, nicht für Zahlen oder Formeln. Eine Kopie im älteren Format .xls oder als .ods behält die Wörter, und beim Speichern erhalten Sie den Hinweis, dass die Formatierung innerhalb der Wörter nicht erhalten bleibt.

## Was aus Ihrer Eingabe wird

JeffCrunch liest Ihre Eingabe so, wie es die Einstellungen Ihres Computers vorgeben. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 04.03.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | der logische Wert „wahr“ | WAHR |
| #NV | der Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | der Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text behalten möchten, etwa einen Code **007**, geben Sie zuerst einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Zum Beispiel verdoppelt **=B2*2** die Zahl in B2. Geben Sie die Namen der Funktionen in Groß- oder Kleinbuchstaben ein; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind immer die englischen, in jeder Sprache: Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür verwendet. Die Trennzeichen zwischen den Teilen folgen den Einstellungen Ihres Computers. Wo das Dezimaltrennzeichen ein Komma ist, wie im Deutschen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in einer Zahl ein Komma: **=SUMME(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln genauso an. Wo das Dezimaltrennzeichen ein Punkt ist, trennen Sie die Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert stets die Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen, damit sie überall gleich geöffnet wird.

Hat eine Formel einen Fehler, etwa eine Klammer, die nie geschlossen wird, teilt JeffCrunch Ihnen mit, was nicht stimmt. Die Eingabe bleibt so, wie Sie sie geschrieben haben, und die Einfügemarke steht in der Nähe des Fehlers, damit Sie ihn beheben können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192. Ist eine Eingabe länger, teilt JeffCrunch Ihnen das mit.

## Zellen leeren

Markieren Sie die Zellen und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten, und es ist ein Schritt, den Sie rückgängig machen können. JeffCrunch leert auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** leert die Zellen, ihre Formate, ihre Notizen, ihre Kommentare (die Verläufe mit Antworten) und ihre Links. Ein Verlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch sagt Ihnen das, weil ein Rückgängig die Kommentare nicht zurückholen könnte, die es nie gelesen hat. **Formate löschen** behält den Inhalt der Zellen und stellt das schlichte Format wieder her. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und die Kommentare von den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, die Zellen behalten ihren Text. Eine Auswahl ist abgeblendet, wenn die markierten Zellen nichts davon haben, was sie entfernen könnte.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, und **Strg+Y** wiederholt sie. Beide finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so wieder her, wie sie war: ihren Wert, ihre Formel, ihr Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Speicherbedarf gering bleibt.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die von ihr abhängen. Die geänderte Zelle zeigt sofort ihren neuen Inhalt, die übrigen Zellen werden einen Moment später aktualisiert; die Statusleiste zeigt **Berechnung läuft**, bis sie fertig sind. Sie können derweil weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, sofern die Arbeitsmappe nicht etwas anderes angibt. JeffCrunch teilt Ihnen mit, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den bereits eine andere Spalte hat, erhält eine Zahl dahinter, ein Name darf nicht leer sein, und **Entf** löscht diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatierung, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Breite von Spalten und Höhe von Zeilen ändern](sizes).

## Ein Symbol einfügen

Klicken Sie auf **Einfügen > Symbole > Symbol**, um das Dialogfeld Symbol zu öffnen. Wählen Sie ein Zeichen aus und klicken Sie auf **Einfügen**, oder doppelklicken Sie darauf. Es wird an der Einfügemarke der Eingabe eingefügt, die Sie gerade schreiben. Schreiben Sie nicht gerade, öffnet sich die Eingabe der Zelle mit dem Symbol am Ende, und **Eingabe** schreibt es in die Zelle. Beispiel: Steht in einer Zelle das Wort *Gesamt*, so wird durch Wahl des Euro-Zeichens die Eingabe *Gesamt€*. Die zuletzt verwendeten Symbole stehen zuerst in der Liste.

**Ein Zeichen aus einer Schriftart übernehmen.** Wählen Sie im Feld **Schriftart** eine Schrift aus und dann das Zeichen. Das Zeichen kommt in einen eigenen Abschnitt, der in dieser Schrift gesetzt wird, und der übrige Text der Zelle behält die Schriftart der Zelle. Beispiel: In einer Zelle mit *Gesamt* ergibt ein aus einer Symbolschrift gewähltes Häkchen *Gesamt✓*, wobei nur das Häkchen in dieser Schrift steht. Ist **(normaler Text)** gewählt, zeichnet die Schriftart der Zelle das Symbol, sodass ein Symbol, das die Schrift nicht enthält, als Kästchen erscheint. In einer Formel wird nur das Zeichen eingefügt.

**Sonderzeichen.** Die Registerkarte **Sonderzeichen** enthält das geschützte Leerzeichen und den geschützten Bindestrich, den bedingten Trennstrich, den Halbgeviertstrich und den Geviertstrich, den Halbgeviertabstand und den Geviertabstand, die Anführungszeichen sowie den bedingten Zeilenumbruch ohne Breite, jeweils mit Namen. Doppelklicken Sie auf eines, um es einzufügen.

**Einem Symbol eine Tastenkombination zuweisen.** Wählen Sie ein Symbol oder ein Sonderzeichen aus, klicken Sie auf **Tastenkürzel**, drücken Sie die gewünschten Tasten, etwa **Strg+Alt+Umschalt+J**, und klicken Sie auf **OK**. Die Tasten fügen dann dieses Symbol ein, genauso wie das Feld. Lassen Sie die Tasten leer und klicken Sie auf **OK**, um sie zu entfernen. JeffCrunch teilt Ihnen mit, wenn ein anderer Befehl diese Tasten bereits verwendet. Das Symbol wird zu einem Befehl und ist deshalb auch unter **Datei > Optionen > Anpassen > Tastatur** aufgeführt, wo Sie seine Tasten ändern können (siehe [Tastenkombinationen](keyboard)).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und die Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden dadurch nicht verändert.
