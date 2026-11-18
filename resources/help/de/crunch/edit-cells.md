# In Zellen eingeben
<!-- keywords: Symbol, Symbolschrift, Sonderzeichen, Tastenkombination für ein Symbol, AutoKorrektur, typografische Anführungszeichen, Strich, Bruch, Link, Adresse, fettes Wort, Teil einer Zelle, Rich Text, Teil eines Textes formatieren, Wort färben, Eingabe, eingeben, Eingabemodus, schreiben, bearbeiten, ändern, Zelle, Eintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Esc, Abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Begrenzung, Zeichen -->

Um etwas in eine Zelle zu schreiben, markieren Sie die Zelle und beginnen Sie zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um den Eintrag zu übernehmen, oder **Esc**, um ihn zu verwerfen und die Zelle unverändert zu lassen.

## Drei Möglichkeiten, die Eingabe zu beginnen

- **Über die Zelle schreiben.** Markieren Sie eine Zelle und tippen Sie. Das erste Zeichen beginnt den Eintrag, und die Zelle zeigt den Text bereits während der Eingabe an.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2** oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste schreiben.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld und tippen Sie dort. Auch dann zeigt die Zelle den Text während der Eingabe an.

Wenn Sie über eine Zelle schreiben, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Verhalten.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** angibt) |
| Umschalt+Eingabe | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie über die Zelle geschrieben haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine schreiben) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Wörter aus der Spalte auflisten, die schon vorhanden sind, und eines auswählen |

**Strg+Eingabe** mit mehreren markierten Zellen schreibt Ihre Eingabe in alle von ihnen. Eine Formel wird für die Zelle geschrieben, in der Sie tippen, und in den anderen Zellen wird sie so übernommen, als wäre sie dorthin kopiert worden: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Das ist ein einzelner Schritt für Rückgängig.

**Alt+Pfeil nach unten** listet in einer Zelle die Wörter aus den Zellen oberhalb und unterhalb auf, sofern sie nicht durch eine leere Zelle getrennt sind. Jedes Wort erscheint nur einmal, in der Reihenfolge des Vorkommens. Wählen Sie ein Wort aus, wird es eingegeben. Hat eine Zelle eine eigene Liste, zeigt Alt+Pfeil nach unten stattdessen diese Liste.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob die **Eingabe**-Taste nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung stehen lässt.

Wenn Sie mit **Tab** nacheinander mehrere Zellen einer Zeile beschreiben und dann **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Markieren Sie zuvor einen Zellbereich, bewegen sich **Eingabe** und **Tab** innerhalb dieses Bereichs.

## AutoKorrektur bei der Eingabe

Wenn Sie nach einem Wort ein Leerzeichen oder ein Satzzeichen wie einen Punkt eingeben, kann JeffCrunch das Wort während der Eingabe korrigieren. Dabei werden typische Tippfehler (etwa „teh“ statt „the“) behoben, zwei große Anfangsbuchstaben am Wortanfang verkleinert, ein kleiner Anfangsbuchstabe am Satzanfang oder bei einem Wochentag großgeschrieben und gerade Anführungszeichen in typografische umgewandelt. Außerdem werden zwei Bindestriche zwischen Wörtern in einen langen Strich umgewandelt sowie 1/2, 1/4 und 3/4 in ½, ¼ und ¾, wenn der Eintrag Wörter enthält. Eine Zahl wie 1/2, die allein eingegeben wird, bleibt wie gewohnt ein Datum oder ein Bruch.

Jede Korrektur ist ein eigener Schritt. Drücken Sie direkt nach einer Korrektur **Strg+Z**, um nur diese Korrektur zurückzunehmen und Ihre Eingabe zu behalten. In Formeln wird nichts verändert: Ein Eintrag, der mit einem Gleichheitszeichen beginnt oder mit einem Plus- oder Minuszeichen gefolgt von einer Formel, bleibt so, wie Sie ihn eingegeben haben. Eine Internetadresse (etwa https://example.org) oder eine E-Mail-Adresse, die den gesamten Eintrag bildet, wird beim Abschließen der Eingabe zu einem Link in der Zelle, sofern **Webadressen beim Eingeben automatisch in Links umwandeln** aktiviert ist.

Diese Optionen schalten Sie unter **Datei > Optionen > Dokumentprüfung** ein oder aus. Ein falsch geschriebenes Wort wird bereits während der Eingabe markiert: siehe [Rechtschreibung](notes-links).

## Teil des Textes formatieren

Einzelne Wörter in einer Zelle können anders aussehen als der Rest. Zum Beispiel kann ein Wort fett oder rot sein.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie auf der Registerkarte **Start** die Befehle **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe** oder die Tasten Strg+B, Strg+I und Strg+U.
4. Drücken Sie die Eingabetaste.

Nur die markierten Wörter ändern sich. Der übrige Text der Zelle behält das Aussehen der Zelle. Während Sie bearbeiten, zeigt das Eingabefeld die Wörter einheitlich an; nach **Eingabe** zeigt die Zelle jeden Teil in seinem eigenen Aussehen. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder einen Schriftgrad, ohne dass Wörter markiert sind, ändert sich wie gewohnt die ganze Zelle.

## Die Minisymbolleiste

Eine kleine Leiste mit den am häufigsten benötigten Formatierungen erscheint direkt bei Ihrer Arbeit, damit Sie nicht bis zum Menüband hochgehen müssen. Sie nimmt der Zelle nie den Tastaturfokus: Was Sie gerade eingeben, bleibt an seiner Stelle.

- **Markierter Text in einer Zelle:** Wenn Sie beim Bearbeiten einer Zelle Wörter mit der Maus markieren, schwebt die Leiste über ihnen. Sie enthält die Felder **Schriftart** und **Schriftgrad** sowie **Fett**, **Kursiv**, **Unterstrichen**, **Schriftgrad vergrößern**, **Schriftgrad verkleinern** und **Schriftfarbe**. Sie ändern nur die markierten Wörter, wie im Menüband.
- **Markierte Zellen:** Klicken Sie mit der rechten Maustaste auf die Zellen. Die Leiste steht dann oben im Kontextmenü. Sie enthält außerdem **Füllfarbe**, **Rahmen**, **Währung**, **Prozentformat**, **1.000er-Trennzeichen**, **Dezimalstelle hinzufügen**, **Dezimalstelle entfernen**, **Verbinden und zentrieren** und **Formatierung kopieren**. Das sind die Befehle des Menübands, und jeder davon ist ein einzelner Schritt, den Sie rückgängig machen können.

Die Leiste über markiertem Text verblasst, wenn Sie den Zeiger davon wegbewegen, und verschwindet, wenn der Zeiger weit entfernt ist. Sie verschwindet auch, wenn Sie schreiben, Esc drücken oder scrollen. Um die Leiste abzuschalten, deaktivieren Sie unter **Datei > Optionen** auf der Seite **Allgemein** die Option **Minisymbolleiste bei Auswahl anzeigen**; die anderen JeffOffice-Apps übernehmen diese Einstellung ebenfalls. Die Schaltfläche Schnellanalyse, die unter einem markierten Bereich erscheint, gibt es in dieser Version nicht.

Zellen mit Formatierungen innerhalb des Textes, die aus einem anderen Programm stammen, zeigen diese Formatierung an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Formatierungen innerhalb eines Textes gelten für Text, nicht für Zahlen oder Formeln. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, behält die Wörter; beim Speichern erhalten Sie dann den Hinweis, dass die Formatierung innerhalb der Wörter nicht erhalten bleibt.

## Was aus Ihrer Eingabe wird

JeffCrunch liest Ihre Eingaben so, wie es die Einstellungen Ihres Computers vorgeben. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

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

Eine Zelle, die bereits ein eigenes Format hat, etwa ein Prozent- oder Datumsformat, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text behalten möchten, etwa den Code **007**, geben Sie zuerst einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Beim Bearbeiten der Zelle wird er wieder angezeigt.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. Zum Beispiel verdoppelt **=B2*2** die Zahl in B2. Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind immer die englischen, in allen Sprachen. Geben Sie also **SUM** ein und nicht den Namen, den Ihre Sprache dafür hat. Die Zeichen zwischen den Teilen einer Formel richten sich nach den Einstellungen Ihres Computers. Wo das Dezimaltrennzeichen ein Komma ist, wie im Deutschen, trennen Sie die Argumente einer Funktion mit einem Semikolon und schreiben in einer Zahl ein Komma: **=SUMME(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln ebenso an. Wo das Dezimaltrennzeichen ein Punkt ist, trennen Sie die Argumente mit einem Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert die Formel immer in der Form, die jedes Programm lesen kann, mit Kommas zwischen den Argumenten und Punkten in Zahlen, damit sie überall gleich geöffnet wird.

Wenn eine Formel einen Fehler hat, etwa eine Klammer, die nie geschlossen wird, sagt JeffCrunch, was nicht stimmt. Der Eintrag bleibt so, wie Sie ihn eingegeben haben, und die Einfügemarke steht in der Nähe des Fehlers, damit Sie ihn beheben können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle enthält bis zu 32.767 Zeichen, eine Formel bis zu 8.192 Zeichen. Ist ein Eintrag länger, weist JeffCrunch Sie darauf hin.

## Zellen leeren

Markieren Sie die Zellen und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten, und das ist ein einzelner Schritt, den Sie rückgängig machen können. JeffCrunch löscht bis zu 100.000 Zellen auf einmal.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** leert die Zellen samt ihren Formaten, Notizen, Kommentaren (den Verläufen mit Antworten) und Links. Ein Kommentarverlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch weist darauf hin, weil ein Rückgängig die Kommentare, die es nicht gelesen hat, nicht wiederherstellen könnte. **Formate löschen** behält den Inhalt der Zellen und setzt das Standardformat zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und Kommentare aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, und die Zellen behalten ihren Text. Ein Befehl ist abgeblendet, wenn die markierten Zellen nichts enthalten, was er entfernen könnte.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, und **Strg+Y** führt sie erneut aus. Beide Befehle finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Löschen. Rückgängig stellt die Zelle so wieder her, wie sie war: ihren Wert, ihre Formel, ihr Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei Änderungen an sehr vielen Zellen weniger, damit der Arbeitsspeicher nicht übermäßig belastet wird.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch die Formeln, die von ihr abhängen, im Hintergrund. Die geänderte Zelle zeigt ihren neuen Inhalt sofort, die übrigen Zellen werden kurz danach aktualisiert. Die Statusleiste zeigt **Berechnung** an, bis sie fertig sind. Sie können in der Zwischenzeit weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht einzeln ändern.
- In einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Alle Zellen sind gesperrt, sofern die Arbeitsmappe nichts anderes angibt. JeffCrunch weist Sie darauf hin, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** den Befehl **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den eine andere Spalte bereits hat, erhält eine Nummer dahinter; ein Name darf nicht leer sein, und **Entf** löscht diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich nur ansehen, aber nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatierung, Sortierung sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen und die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Spaltenbreite und Zeilenhöhe ändern](sizes).

## Ein Symbol einfügen

Klicken Sie auf **Einfügen > Symbole > Symbol**, um das Dialogfeld Symbol zu öffnen. Wählen Sie ein Zeichen und klicken Sie auf **Einfügen** oder doppelklicken Sie darauf. Es wird an der Einfügemarke der Eingabe eingefügt, die Sie gerade schreiben. Schreiben Sie gerade nicht, öffnet sich der Eintrag der Zelle mit dem Symbol am Ende, und **Eingabe** übernimmt es in die Zelle. Beispiel: Steht in einer Zelle das Wort *Gesamt*, ergibt die Wahl des Eurozeichens die Eingabe *Gesamt€*. Die zuletzt verwendeten Symbole stehen zuerst in der Liste.

**Das Zeichen aus einer Schriftart übernehmen.** Wählen Sie im Feld **Schriftart** eine Schrift und dann das Zeichen. Das Zeichen wird in einem eigenen Abschnitt eingefügt, der in dieser Schrift gesetzt ist, während der übrige Text der Zelle seine Schriftart behält. Beispiel: In einer Zelle mit *Gesamt* ergibt ein Häkchen aus einer Symbolschrift *Gesamt✓*, wobei nur das Häkchen in dieser Schrift steht. Ist **(normaler Text)** gewählt, zeichnet die Schriftart der Zelle das Symbol; fehlt es dort, erscheint ein Kästchen. In einer Formel wird nur das Zeichen eingefügt.

**Sonderzeichen.** Die Registerkarte **Sonderzeichen** listet das geschützte Leerzeichen und den geschützten Bindestrich, den bedingten Trennstrich, den Halbgeviertstrich und den Geviertstrich, das Halbgeviertleerzeichen und das Geviertleerzeichen, die Anführungszeichen sowie den bedingten Zeilenumbruch ohne Breite auf, jeweils mit Namen. Doppelklicken Sie auf eines, um es einzufügen.

**Einem Symbol eine Tastenkombination geben.** Wählen Sie ein Symbol oder ein Sonderzeichen, klicken Sie auf **Tastenkombination**, drücken Sie die gewünschten Tasten, etwa **Strg+Alt+Umschalt+J**, und klicken Sie auf **OK**. Die Tasten fügen dann dieses Symbol ein, genauso wie das Dialogfeld. Lassen Sie die Tasten leer und klicken Sie auf **OK**, um die Kombination zu entfernen. JeffCrunch meldet, wenn ein anderer Befehl diese Tasten bereits belegt. Das Symbol wird dadurch zu einem Befehl und ist daher auch unter **Datei > Optionen > Anpassen > Tastatur** aufgeführt, wo Sie die Tasten ändern können (siehe [Tastenkombinationen](keyboard)).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und die Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden dadurch nicht geändert.
