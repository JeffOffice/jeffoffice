# In Zellen eingeben
<!-- keywords: Symbol, Symbolschrift, Sonderzeichen, Tastenkombination für ein Symbol, AutoKorrektur, typografische Anführungszeichen, Gedankenstrich, Bruch, Link, Adresse, fettes Wort, Teil einer Zelle, Rich Text, Teil des Textes formatieren, Wort färben, Eingabe, eingeben, tippen, Eingabetaste, bearbeiten, ändern, Zelle, Eintrag, Bearbeitungsleiste, F2, Doppelklick, Tab, Escape, abbrechen, rückgängig, wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Grenze, Zeichen -->

Um etwas in eine Zelle einzugeben, wählen Sie die Zelle aus und beginnen Sie zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um den Eintrag zu übernehmen, oder **Esc**, um ihn zu verwerfen und die Zelle unverändert zu lassen.

## Drei Wege, mit der Eingabe zu beginnen

- **Direkt in die Zelle schreiben.** Wählen Sie eine Zelle aus und tippen Sie. Das erste Zeichen beginnt den Eintrag, und die Zelle zeigt beim Tippen mit an, was Sie eingeben.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2** oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste schreiben.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld und tippen Sie dort. Die Zelle zeigt auch dabei an, was Sie eingeben.

Wenn Sie direkt in eine Zelle schreiben, beendet eine Pfeiltaste den Eintrag und verschiebt die Auswahl. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Verhalten.

## Eintrag abschließen

| Tasten | Was sie tun |
| --- | --- |
| Eingabe | Eintrag übernehmen und nach unten wechseln (oder wohin **Datei > Optionen > Bearbeiten** es festlegt) |
| Umschalt+Eingabe | Eintrag übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eintrag in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eintrag übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eintrag übernehmen und nach links wechseln |
| Pfeiltasten | Eintrag übernehmen und eine Zelle weiterwechseln (nur, wenn Sie direkt in die Zelle geschrieben haben) |
| Esc | Eintrag verwerfen |
| Alt+Eingabe | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (im Eintrag, wenn Sie gerade einen eingeben) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Die Wörter auflisten, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** bei mehreren markierten Zellen schreibt das Eingegebene in alle. Eine Formel wird für die Zelle geschrieben, in der Sie tippen, und in den übrigen Zellen wird sie so gelesen, wie es eine Kopie täte: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Das ist ein Schritt für **Rückgängig**.

**Alt+Pfeil nach unten** listet in einer Zelle die Wörter der Zellen darüber und darunter auf, in der Reihenfolge des Vorkommens und jedes nur einmal, soweit sie nicht durch eine leere Zelle getrennt sind. Die Auswahl fügt das Wort ein. Eine Zelle mit einer eigenen Liste zeigt stattdessen diese Liste an.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob **Eingabe** nach unten, nach rechts, nach oben oder nach links wechselt oder die Auswahl nicht verschiebt.

Wenn Sie mit **Tab** nacheinander in mehrere Zellen einer Zeile schreiben und dann **Eingabe** drücken, springt die Auswahl in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie vorher einen Zellbereich markieren, bewegen **Eingabe** und **Tab** die Auswahl innerhalb dieses Bereichs.

## AutoKorrektur beim Schreiben

Wenn Sie nach einem Wort ein Leerzeichen oder ein Satzzeichen wie einen Punkt eingeben, kann JeffCrunch das Wort beim Abschließen korrigieren. Es behebt häufige Tippfehler (etwa „teh“ zu „the“), zwei große Anfangsbuchstaben am Wortanfang, einen kleinen Anfangsbuchstaben nach einem Satzanfang oder bei einem Wochentag sowie gerade Anführungszeichen, die dann zu typografischen werden. Außerdem wird ein doppelter Bindestrich zwischen Wörtern zu einem langen Gedankenstrich, und 1/2, 1/4 und 3/4 werden zu ½, ¼ und ¾, wenn der Eintrag Wörter enthält. Eine Zahl wie 1/2, die allein steht, bleibt wie immer ein Datum oder ein Bruch.

Jede Korrektur ist ein eigener Schritt. Drücken Sie direkt nach einer Korrektur **Strg+Z**, um nur diese Korrektur zurückzunehmen und das Eingegebene zu behalten. In Formeln wird nichts geändert: Ein Eintrag, der mit einem Gleichheitszeichen beginnt oder mit einem Plus- oder Minuszeichen und dann einer Formel, bleibt so, wie Sie ihn eingegeben haben. Eine Webadresse wie https://example.org oder eine E-Mail-Adresse, die den ganzen Eintrag bildet, wird beim Abschließen zu einem Link in der Zelle, sofern **Webadressen beim Eingeben automatisch in Links umwandeln** eingeschaltet ist.

Diese Optionen schalten Sie unter **Datei > Optionen > Dokumentprüfung** ein oder aus. Ein falsch geschriebenes Wort wird beim Schreiben markiert: siehe [Rechtschreibung](notes-links).

## Teil des Textes formatieren

Einzelne Wörter in einer Zelle können anders aussehen als der Rest. Zum Beispiel kann ein Wort fett oder rot sein.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie auf der Registerkarte **Start** die Befehle **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe**, oder Strg+B, Strg+I und Strg+U.
4. Drücken Sie Eingabe.

Nur die markierten Wörter ändern sich. Der Rest der Zelle behält das eigene Aussehen der Zelle. Beim Bearbeiten zeigt das Eingabefeld die Wörter einheitlich, und nach **Eingabe** zeigt die Zelle jeden Teil in seinem Aussehen. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder einen Schriftgrad, ohne dass Wörter markiert sind, ändert sich wie immer die ganze Zelle.

## Die Minisymbolleiste

Eine kleine Leiste mit den am häufigsten verwendeten Formatierungen erscheint dort, wo Sie arbeiten, sodass Sie nicht zum Menüband hinaufgehen müssen. Sie nimmt der Zelle nie die Tastatur weg: Was Sie gerade eingegeben haben, bleibt, wo es war.

- **Markierter Text in einer Zelle:** Wählen Sie beim Bearbeiten einer Zelle Wörter mit der Maus aus, erscheint die Leiste über ihnen. Sie enthält die Felder **Schriftart** und **Schriftgrad**, **Fett**, **Kursiv**, **Unterstrichen**, **Schriftgrad vergrößern**, **Schriftgrad verkleinern** und **Schriftfarbe**. Sie ändern nur die markierten Wörter, wie auf dem Menüband.
- **Markierte Zellen:** Klicken Sie mit der rechten Maustaste auf die Zellen, dann steht die Leiste oben im Kontextmenü. Sie enthält außerdem **Füllfarbe**, **Rahmen**, **Währung**, **Prozentformat**, **Tausendertrennzeichen**, **Dezimalstelle hinzufügen**, **Dezimalstelle entfernen**, **Verbinden und zentrieren** und **Formatierung kopieren**. Es sind die Befehle des Menübands, und jeder ist ein einzelner Schritt, den Sie rückgängig machen können.

Die Leiste über markiertem Text verblasst, wenn Sie den Zeiger davon wegbewegen, und verschwindet, wenn er weit weg ist. Sie verschwindet auch, wenn Sie etwas eingeben, Esc drücken oder scrollen. Um die Leiste abzuschalten, deaktivieren Sie unter **Datei > Optionen** auf der Seite **Allgemein** die Option **Minisymbolleiste bei Auswahl anzeigen**; die anderen JeffOffice-Apps übernehmen diese Einstellung ebenfalls. Die Schaltfläche Schnellanalyse, die unter einem markierten Bereich erscheint, gibt es in dieser Version nicht.

Zellen, die Formatierungen innerhalb des Textes aus einem anderen Programm enthalten, zeigen sie an, behalten sie und speichern sie wieder. Eine geänderte Zelle behält ihre Formatierung. Eine Formatierung innerhalb eines Textes gilt für Text, nicht für Zahlen oder Formeln. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, behält die Wörter; beim Speichern weist das Programm darauf hin, dass die Formatierung innerhalb der Wörter nicht erhalten bleibt.

## Was Sie eingeben, wird zu

JeffCrunch liest Ihre Eingaben so, wie es die Einstellungen Ihres Computers vorgeben. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält das Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5 % |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 04.03.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | den logischen Wert Wahr | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa ein Prozent- oder Datumsformat, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text behalten möchten, etwa einen Code **007**, geben Sie zuerst einen Apostroph ein: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. **=B2*2** verdoppelt zum Beispiel die Zahl in B2. Funktionsnamen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Funktionsnamen sind immer die englischen, in allen Sprachen: Geben Sie **SUM** ein, nicht den Namen, den Ihre Sprache dafür vorsieht. Die Zeichen zwischen den Argumenten richten sich nach den Einstellungen Ihres Computers. Ist das Dezimaltrennzeichen ein Komma, wie im Deutschen und Französischen, trennen Sie die Argumente einer Funktion durch ein Semikolon und schreiben in einer Zahl ein Komma: **=SUM(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln genauso an. Ist das Dezimaltrennzeichen ein Punkt, trennen Sie die Argumente durch ein Komma: **=SUM(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen Argumenten und Punkten in Zahlen, damit sie überall gleich geöffnet wird.

Hat eine Formel einen Fehler, etwa eine Klammer, die nicht geschlossen wird, sagt JeffCrunch, was nicht stimmt. Der Eintrag bleibt so, wie Sie ihn eingegeben haben, und die Einfügemarke steht nahe der Stelle des Fehlers, damit Sie ihn beheben können. Erst wenn die Formel stimmt, wird etwas geändert.

Eine Zelle nimmt bis zu 32.767 Zeichen auf, eine Formel bis zu 8.192. JeffCrunch weist darauf hin, wenn ein Eintrag länger ist.

## Zellen leeren

Markieren Sie die Zellen und drücken Sie **Entf**. Das leert den Inhalt der Zellen, behält ihre Formate und ist ein Schritt, den Sie rückgängig machen können. JeffCrunch leert auf einmal bis zu 100.000 Zellen.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** entfernt Inhalt, Format, Notizen, Kommentare (die Verläufe mit Antworten) und Links aus den Zellen. Ein Kommentarverlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch weist darauf hin, weil sich diese Kommentare nicht mit **Rückgängig** wiederherstellen lassen, da das Programm sie nicht vollständig gelesen hat. **Formate löschen** behält den Inhalt und setzt das einfache Format zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und Kommentare aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, der Text bleibt. Ein Befehl ist abgeblendet, wenn die markierten Zellen nichts enthalten, was er löschen könnte.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, **Strg+Y** holt sie zurück. Beide stehen auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Löschen. **Rückgängig** stellt die Zelle so wieder her, wie sie war: Wert, Formel, Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, bei sehr vielen geänderten Zellen weniger, damit der Speicherbedarf gering bleibt.

**Mehrere Schritte auf einmal zurücknehmen.** In der Schnellsymbolleiste hat **Rückgängig** und **Wiederholen** je einen Pfeil daneben. Klicken Sie auf den Pfeil, um die Schritte zu sehen, die neuesten zuerst (bis zu 100 werden aufgelistet). Bewegen Sie den Zeiger in der Liste nach unten, um festzulegen, wie viele Schritte Sie meinen (die Zeile unter der Liste nennt zum Beispiel „3 Aktionen rückgängig machen“), und klicken Sie, um sie auszuführen. Wenn Sie den dritten Schritt wählen, werden die ersten drei rückgängig gemacht. Die Schritte bleiben in ihrer Reihenfolge, daher holt **Wiederholen** sie in derselben Weise zurück.

## Wiederholen

**F4** wiederholt den letzten Befehl auf den jetzt markierten Zellen. Beispiel: Machen Sie **A1** fett, markieren Sie **C4:C9** und drücken Sie **F4**. Die Zellen in C4:C9 werden fett, in einem Schritt, den Sie rückgängig machen können. Wiederholen funktioniert bei Formatierungen, Zellformatvorlagen, dem Löschen sowie dem Einfügen und Löschen von Zeilen und Spalten. Der QuickInfo der Schaltfläche oder der Befehl **Wiederholen** nennt, was er täte („Fett wiederholen“). **Strg+Y** wiederholt ebenfalls, aber nur, wenn kein Schritt zum Wiederholen vorliegt; solange einer vorliegt, ist **Strg+Y** Wiederholen. Während Sie eine Formel eingeben, wechselt **F4** wie bisher einen Bezug zwischen relativ und absolut. Befehle, die ein Dialogfeld öffnen, und Eingaben in Zellen werden nicht wiederholt.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch die Formeln, die von ihr abhängen, im Hintergrund. Die geänderte Zelle zeigt ihren neuen Inhalt sofort, die übrigen Zellen werden kurz danach aktualisiert; die Statusleiste zeigt **Berechnung** an, bis sie fertig sind. Sie können in der Zwischenzeit weiterschreiben.

## Was sich nicht ändern lässt

- Eine Zelle, die Teil einer **Matrixformel** oder einer Datentabelle ist, lässt sich nicht allein ändern. Um eine Matrixformel zu erstellen, markieren Sie die Zellen, die sie füllen soll, geben die Formel ein und drücken **Strg+Umschalt+Eingabe**. Eine Formel füllt dann alle Zellen, und die Bearbeitungsleiste zeigt sie in geschweiften Klammern, etwa **{=A1:A3*2}**. Zum Ändern markieren Sie eine beliebige Zelle der Matrix, drücken **F2**, ändern die Formel und drücken erneut **Strg+Umschalt+Eingabe**. Zum Löschen markieren Sie alle Zellen der Matrix und drücken **Entf**. Beides wirkt auf die ganze Matrix und ist ein Schritt für **Rückgängig**. **F2** auf einer beliebigen Zelle einer Matrix öffnet die Formel der Matrix; ein geschütztes Blatt oder ein Kennwortbereich verweigert den Zugriff oder fragt nach, wie bei jeder anderen Zelle. Das Kopieren einer Matrix oder eines Teils davon und das Ausfüllen aus einer ihrer Zellen ergeben die Werte, die sie anzeigt, nicht eine Formel.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, sofern die Arbeitsmappe es nicht anders vorsieht. JeffCrunch meldet es, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** den Befehl **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Geben Sie in einer dieser Zellen einen neuen Namen ein, um die Spalte umzubenennen: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Ein Name, den bereits eine andere Spalte hat, erhält eine Zahl dahinter, ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich ansehen, aber nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen und die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Breite von Spalten und Höhe von Zeilen ändern](sizes).

## Ein Symbol einfügen

Klicken Sie auf **Einfügen > Symbole > Symbol**, um das Dialogfeld Symbol zu öffnen. Wählen Sie ein Zeichen aus und klicken Sie auf **Einfügen** oder doppelklicken Sie darauf. Es wird an der Einfügemarke des Eintrags eingefügt, den Sie gerade schreiben. Schreiben Sie nicht gerade, öffnet sich der Eintrag der Zelle, und das Symbol wird an dessen Ende angefügt; **Eingabe** übernimmt es in die Zelle. Beispiel: Steht in einer Zelle das Wort *Gesamt*, macht das Euro-Zeichen daraus den Eintrag *Gesamt€*. Die zuletzt verwendeten Symbole stehen zuerst in der Liste.

**Ein Zeichen aus einer Schriftart nehmen.** Wählen Sie im Feld **Schriftart** eine Schrift und dann das Zeichen. Das Zeichen wird in einen eigenen Abschnitt in dieser Schrift eingefügt, der Rest des Zellentextes behält die Schriftart der Zelle. Beispiel: In einer Zelle mit *Gesamt* ergibt ein Häkchen aus einer Symbolschrift *Gesamt✓*, wobei nur das Häkchen in dieser Schrift steht. Mit **(normaler Text)** zeichnet die Schriftart der Zelle das Symbol; ein Symbol, das die Schrift nicht enthält, erscheint als Kästchen. In einer Formel wird nur das Zeichen eingefügt.

**Sonderzeichen.** Die Registerkarte **Sonderzeichen** listet geschütztes Leerzeichen und geschützten Bindestrich, den bedingten Trennstrich, den Halbgeviertstrich und den Geviertstrich, das Halbgeviertleerzeichen und das Geviertleerzeichen, die Anführungszeichen und den bedingten Umbruch ohne Breite auf, jeweils mit ihrem Namen. Doppelklicken Sie auf eines, um es einzufügen.

**Einem Symbol eine Tastenkombination geben.** Wählen Sie ein Symbol oder ein Sonderzeichen aus, klicken Sie auf **Tastenkombination**, drücken Sie die gewünschten Tasten, etwa **Strg+Alt+Umschalt+J**, und klicken Sie auf **OK**. Die Tasten fügen dann dieses Symbol ein, so wie es das Dialogfeld tut. Lassen Sie das Feld leer und klicken Sie auf **OK**, um die Tasten zu entfernen. JeffCrunch meldet, wenn ein anderer Befehl die Tasten bereits verwendet. Das Symbol wird zu einem Befehl und ist daher auch unter **Datei > Optionen > Anpassen > Tastatur** aufgeführt, wo Sie die Tasten ändern können (siehe [Tastenkombinationen](keyboard)).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und die Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden nicht geändert.
