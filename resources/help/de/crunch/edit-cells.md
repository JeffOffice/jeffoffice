# In Zellen eingeben
<!-- keywords: Symbol, Symbolschrift, Sonderzeichen, Tastenkombination für ein Symbol, AutoKorrektur, typografische Anführungszeichen, Gedankenstrich, Bruch, Link, Adresse, fettes Wort, Teil einer Zelle, Rich Text, Teil eines Textes formatieren, Wort färben, Eingabe, eingeben, Bearbeiten, ändern, Zelle, Eingabe, Bearbeitungsleiste, F2, Doppelklick, Tab, Esc, abbrechen, Rückgängig, Wiederholen, löschen, leeren, Prozent, Währung, Geld, Datum, Uhrzeit, Zahl, Text, Apostroph, Formel, Gleichheitszeichen, ersetzen, Begrenzung, Zeichen, schreiben, tippen -->

Um etwas in eine Zelle einzugeben, markieren Sie die Zelle und beginnen Sie zu tippen. Was Sie eingeben, ersetzt den bisherigen Inhalt der Zelle. Drücken Sie **Eingabe**, um die Eingabe zu übernehmen, oder **Esc**, um sie zu verwerfen und die Zelle unverändert zu lassen.

## Drei Arten, die Eingabe zu beginnen

- **Direkt in die Zelle tippen.** Markieren Sie eine Zelle und tippen Sie. Das erste Zeichen beginnt die Eingabe, und die Zelle zeigt an, was Sie eingeben.
- **Den Inhalt der Zelle bearbeiten.** Drücken Sie **F2** oder doppelklicken Sie auf die Zelle. Die Zelle öffnet sich mit ihrem Inhalt, und die Einfügemarke steht am Ende. Mit den Pfeiltasten bewegen Sie sich im Text.
- **In der Bearbeitungsleiste schreiben.** Klicken Sie in die Bearbeitungsleiste neben dem Namenfeld und tippen Sie dort. Auch dann zeigt die Zelle an, was Sie eingeben.

Wenn Sie direkt in eine Zelle tippen, beendet eine Pfeiltaste die Eingabe und verschiebt die Markierung. Wenn Sie den Inhalt der Zelle bearbeiten, bewegen die Pfeiltasten die Einfügemarke im Text. Mit **F2** wechseln Sie zwischen beiden Arten.

## Eingabe abschließen

| Tasten | Wirkung |
| --- | --- |
| Eingabe | Eingabe übernehmen und nach unten wechseln (oder dorthin, was **Datei > Optionen > Bearbeiten** festlegt) |
| Umschalt+Eingabe | Eingabe übernehmen und in die andere Richtung wechseln |
| Strg+Eingabe | Eingabe in alle markierten Zellen schreiben und an der Stelle bleiben |
| Tab | Eingabe übernehmen und nach rechts wechseln |
| Umschalt+Tab | Eingabe übernehmen und nach links wechseln |
| Pfeiltasten | Eingabe übernehmen und eine Zelle weiterwechseln (nur, wenn Sie direkt in die Zelle getippt haben) |
| Esc | Eingabe verwerfen |
| Alt+Eingabe | Neue Zeile innerhalb der Zelle beginnen |
| Strg+; | Heutiges Datum einfügen (in der Eingabe, wenn Sie gerade eine tippen) |
| Strg+Umschalt+; | Aktuelle Uhrzeit einfügen |
| Alt+Pfeil nach unten | Wörter aufzählen, die bereits in der Spalte stehen, und eines auswählen |

**Strg+Eingabe** schreibt bei mehreren markierten Zellen Ihre Eingabe in alle. Eine Formel ist für die Zelle geschrieben, in der Sie tippen; in den anderen Zellen verhält sie sich so, wie es eine Kopie täte: Wenn Sie **=A1** eingeben, während der Bereich E6:F7 markiert ist, ergibt sich in F7 **=B2**. Das ist ein einziger Schritt für Rückgängig.

**Alt+Pfeil nach unten** listet in einer Zelle die Wörter der Zellen darüber und darunter auf, die nicht durch eine leere Zelle getrennt sind, in der Reihenfolge des Vorkommens und jedes nur einmal. Wenn Sie eines auswählen, wird es eingegeben. Hat eine Zelle eine eigene Liste, zeigt sie stattdessen diese Liste.

Unter **Datei > Optionen > Bearbeiten** legen Sie fest, ob die **Eingabetaste** nach unten, nach rechts, nach oben oder nach links wechselt oder die Markierung nicht bewegt.

Wenn Sie mit **Tab** nacheinander in mehrere Zellen einer Zeile eingeben und dann **Eingabe** drücken, springt die Markierung in die nächste Zeile, und zwar in die Spalte, in der Sie begonnen haben. Wenn Sie vorher einen Zellbereich markieren, bewegen **Eingabe** und **Tab** die Markierung innerhalb dieses Bereichs.

## AutoKorrektur bei der Eingabe

Wenn Sie nach einem Wort ein Leerzeichen oder ein Satzzeichen wie einen Punkt eingeben, kann JeffCrunch das Wort korrigieren, sobald Sie es abgeschlossen haben. Die Korrektur behebt übliche Tippfehler (etwa „teh“ zu „the“), zwei große Anfangsbuchstaben am Wortanfang, einen kleinen Anfangsbuchstaben nach einem Satzende oder bei einem Wochentag sowie gerade Anführungszeichen, die dann zu typografischen werden. Außerdem wandelt sie zwei Bindestriche zwischen Wörtern in einen Gedankenstrich um und 1/2, 1/4 und 3/4 in ½, ¼ und ¾, wenn die Eingabe Wörter enthält. Eine Zahl wie 1/2, die allein steht, bleibt wie gewohnt ein Datum oder ein Bruch.

Jede Korrektur ist ein eigener Schritt. Drücken Sie direkt nach einer Korrektur **Strg+Z**, um nur diese Korrektur zurückzunehmen und Ihre Eingabe zu behalten. In einer Formel wird nichts verändert: Eine Eingabe, die mit einem Gleichheitszeichen beginnt oder mit einem Plus- oder Minuszeichen gefolgt von einer Formel, bleibt so, wie Sie sie getippt haben. Eine Webadresse wie https://example.org oder eine E-Mail-Adresse, die die ganze Eingabe bildet, wird beim Abschluss der Eingabe zu einem Link in ihrer Zelle, wenn **Webadressen beim Eingeben automatisch in Links umwandeln** aktiviert ist.

Diese Optionen schalten Sie unter **Datei > Optionen > Dokumentprüfung** ein oder aus. Ein falsch geschriebenes Wort wird während der Eingabe markiert: siehe [Rechtschreibung](notes-links).

## Teil des Textes formatieren

Einzelne Wörter in einer Zelle können anders aussehen als der Rest, zum Beispiel fett oder rot.

1. Drücken Sie F2, um die Zelle zu bearbeiten (oder doppelklicken Sie darauf).
2. Markieren Sie die Wörter, die Sie ändern möchten, mit Umschalt und den Pfeiltasten oder mit der Maus.
3. Verwenden Sie auf der Registerkarte **Start** die Befehle **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Schriftart**, **Schriftgrad** oder **Schriftfarbe** oder die Tasten Strg+B, Strg+I und Strg+U.
4. Drücken Sie Eingabe.

Nur die markierten Wörter ändern sich. Der Rest der Zelle behält das eigene Aussehen der Zelle. Während Sie bearbeiten, zeigt das Eingabefeld die Wörter einheitlich an; in der Zelle erscheint jeder Teil in seinem Aussehen, sobald Sie Eingabe drücken. Wenn Sie später Wörter eingeben oder löschen, wandert die Formatierung mit den Wörtern mit. Wählen Sie eine Schriftart oder einen Schriftgrad, ohne dass Wörter markiert sind, ändert sich wie gewohnt die ganze Zelle.

## Die Minisymbolleiste

Eine kleine Leiste mit den am häufigsten benötigten Formatierungen erscheint dort, wo Sie arbeiten, sodass Sie nicht zum Menüband hinaufgehen müssen. Sie nimmt der Zelle nie die Tastatur weg: Was Sie gerade eingeben, bleibt an seiner Stelle.

- **Markierter Text in einer Zelle:** Wenn Sie beim Bearbeiten einer Zelle Wörter mit der Maus markieren, erscheint die Leiste über ihnen. Sie enthält die Felder **Schriftart** und **Schriftgrad**, **Fett**, **Kursiv**, **Unterstrichen**, **Schriftgrad vergrößern**, **Schriftgrad verkleinern** und **Schriftfarbe**. Sie ändern nur die markierten Wörter, wie auf dem Menüband.
- **Markierte Zellen:** Klicken Sie mit der rechten Maustaste auf die Zellen; die Leiste steht oben im Kontextmenü. Sie enthält außerdem **Füllfarbe**, **Rahmen**, **Währung**, **Prozentformat**, **Tausendertrennzeichen**, **Dezimalstelle hinzufügen**, **Dezimalstelle entfernen**, **Zellen verbinden und zentrieren** und **Formatierung übertragen**. Es sind die Befehle des Menübands, und jeder davon ist ein einzelner Schritt, den Sie rückgängig machen können.

Die Leiste über markiertem Text wird blasser, wenn Sie den Zeiger von ihr wegbewegen, und verschwindet, wenn der Zeiger weit entfernt ist. Sie verschwindet auch, wenn Sie etwas eingeben, Esc drücken oder scrollen. Um die Leiste abzuschalten, deaktivieren Sie unter **Datei > Optionen** auf der Seite **Allgemein** die Option **Minisymbolleiste bei Auswahl anzeigen**; die anderen JeffOffice-Apps übernehmen diese Einstellung ebenfalls. Die Schaltfläche Schnellanalyse, die unter einem markierten Bereich erscheint, gibt es in dieser Version nicht.

Zellen mit Formatierung innerhalb des Textes aus einem anderen Programm zeigen sie an, behalten sie und speichern sie wieder. Eine Zelle, die Sie ändern, behält ihre Formatierung. Formatierung innerhalb eines Textes gilt für Text, nicht für Zahlen oder Formeln. Eine Kopie, die Sie als ältere .xls-Datei oder als .ods speichern, behält die Wörter; beim Speichern erhalten Sie den Hinweis, dass die Formatierung innerhalb der Wörter nicht erhalten bleibt.

## Was Sie eingeben, wird zu

JeffCrunch liest Ihre Eingabe so, wie es die Einstellungen Ihres Computers vorgeben. Das Dezimaltrennzeichen, das Tausendertrennzeichen, das Währungszeichen und die Reihenfolge von Tag und Monat in einem Datum stammen aus diesen Einstellungen. Eine Zahl, die Sie in eine Zelle ohne eigenes Format eingeben, erhält ein Format, das sie so anzeigt, wie Sie sie eingegeben haben.

| Sie geben ein | Die Zelle enthält | Und zeigt |
| --- | --- | --- |
| 1234,5 | die Zahl 1234,5 | 1234,5 |
| 5% | die Zahl 0,05 | 5% |
| 12,50 € | die Zahl 12,5 | 12,50 € |
| 1 1/2 | die Zahl 1,5 | 1 1/2 |
| 4.3.2025 | das Datum als Anzahl von Tagen | 04.03.2025 |
| 22:30 | die Uhrzeit als Teil eines Tages | 22:30 |
| WAHR | den logischen Wert Wahr | WAHR |
| #NV | den Fehler #NV | #NV |
| =B2*2 | eine Formel | das Ergebnis der Formel |
| alles andere | Text | den Text |

Eine Zelle, die bereits ein eigenes Format hat, etwa Prozent oder Datum, behält es. Eine Zelle mit dem Format **Text** behält alles, was Sie eingeben, als Text, außer einer Formel.

Wenn Sie etwas, das wie eine Zahl aussieht, als Text behalten möchten, etwa einen Code **007**, tippen Sie vorher einen Apostroph: **'007**. Der Apostroph gehört nicht zum Text. Wenn Sie die Zelle bearbeiten, sehen Sie ihn wieder.

## Formeln

Eine Formel beginnt mit einem Gleichheitszeichen. **=B2*2** verdoppelt zum Beispiel die Zahl in B2. Namen von Funktionen können Sie in Groß- oder Kleinbuchstaben eingeben; JeffCrunch schreibt sie groß.

Die Namen der Funktionen sind in allen Sprachen die englischen: Tippen Sie **SUM**, nicht den Namen, den Ihre Sprache dafür verwendet. Die Zeichen dazwischen richten sich nach den Einstellungen Ihres Computers. Wenn das Dezimaltrennzeichen ein Komma ist, wie im Deutschen, trennen Sie die Argumente einer Funktion durch ein Semikolon und schreiben in einer Zahl ein Komma: **=SUMME(A1;A2)*0,5**. Die Bearbeitungsleiste und **F2** zeigen Formeln auf dieselbe Weise. Ist das Dezimaltrennzeichen ein Punkt, trennen Sie die Argumente durch ein Komma: **=SUMME(A1,A2)*0.5**. Die Datei speichert immer die Form, die jedes Programm liest, mit Kommas zwischen den Argumenten und Punkten in Zahlen, damit sie überall gleich geöffnet wird.

Hat eine Formel einen Fehler, etwa eine Klammer, die nicht geschlossen wird, teilt JeffCrunch Ihnen mit, was nicht stimmt. Die Eingabe bleibt so, wie Sie sie getippt haben, und die Einfügemarke steht nahe der Fehlerstelle, damit Sie sie korrigieren können. Erst wenn die Formel stimmt, wird etwas verändert.

Eine Zelle nimmt bis zu 32.767 Zeichen auf, eine Formel bis zu 8.192. Ist eine Eingabe länger, weist JeffCrunch Sie darauf hin.

## Zellen leeren

Markieren Sie die Zellen und drücken Sie **Entf**. Dadurch wird der Inhalt der Zellen gelöscht, ihre Formate bleiben erhalten, und der Vorgang lässt sich rückgängig machen. JeffCrunch leert höchstens 100.000 Zellen auf einmal.

Auf der Registerkarte **Start** hat die Schaltfläche **Löschen** ein Menü. **Alles löschen** entfernt Inhalt, Formate, Notizen, Kommentare (die Verläufe mit Antworten) und Links aus den Zellen. Ein Kommentarverlauf mit mehr als 500 Kommentaren bleibt erhalten, und JeffCrunch weist darauf hin, weil ein Rückgängig die Kommentare, die es nicht gelesen hat, nicht wiederherstellen könnte. **Formate löschen** behält den Inhalt der Zellen und setzt das einfache Format zurück. **Inhalte löschen** entspricht **Entf**. **Kommentare und Notizen löschen** entfernt nur die Notizen und Kommentare aus den markierten Zellen. **Hyperlinks löschen** entfernt nur die Links, und die Zellen behalten ihren Text. Eine Auswahl ist abgeblendet, wenn die markierten Zellen dafür nichts zu löschen haben.

## Rückgängig und Wiederholen

**Strg+Z** macht die letzte Änderung rückgängig, und **Strg+Y** führt sie erneut aus. Beide finden Sie auch in der Schnellsymbolleiste oben. Jede Eingabe in eine Zelle ist ein Schritt, ebenso jedes Leeren. Rückgängig stellt die Zelle so her, wie sie war: ihren Wert, ihre Formel, ihr Format und die Formeln, die von ihr abhängen. JeffCrunch merkt sich bis zu 1.000 Schritte, weniger, wenn sie sehr viele Zellen betroffen haben, damit der Speicherbedarf klein bleibt.

## Während JeffCrunch die Formeln berechnet

Wenn Sie eine Zelle ändern, berechnet JeffCrunch im Hintergrund die Formeln, die von ihr abhängen. Die geänderte Zelle zeigt ihren neuen Inhalt sofort, die anderen Zellen aktualisieren sich kurz danach; bis dahin zeigt die Statusleiste **Berechnung läuft** an. Sie können inzwischen weiter eingeben.

## Was sich nicht ändern lässt

- Eine Zelle, die zu einer **Matrixformel** oder einer Datentabelle gehört, lässt sich nicht einzeln ändern. Um eine Matrixformel zu erstellen, markieren Sie die Zellen, die sie füllen soll, geben die Formel ein und drücken **Strg+Umschalt+Eingabe**. Eine Formel füllt dann alle Zellen, und die Bearbeitungsleiste zeigt sie in geschweiften Klammern, etwa **{=A1:A3*2}**. Zum Ändern markieren Sie eine beliebige Zelle der Matrix, drücken **F2**, ändern die Formel und drücken erneut **Strg+Umschalt+Eingabe**. Zum Löschen markieren Sie alle ihre Zellen und drücken **Entf**. Beides wirkt auf die ganze Matrix und ist ein einziger Schritt für Rückgängig. **F2** in einer beliebigen Zelle der Matrix öffnet die Formel der Matrix; ein geschütztes Blatt oder ein Kennwortbereich verweigert den Zugriff oder fragt nach, wie bei jeder anderen Zelle. Kopieren Sie eine Matrix oder einen Teil davon, oder füllen Sie aus einer ihrer Zellen, erhalten Sie die Werte, die sie anzeigt, und keine Formel.
- Auf einem **geschützten Blatt** lässt sich eine gesperrte Zelle nicht ändern. Jede Zelle ist gesperrt, es sei denn, die Arbeitsmappe legt etwas anderes fest. JeffCrunch meldet es, wenn Sie es versuchen. Um die Zelle zu ändern, wählen Sie auf der Registerkarte **Überprüfen** die Option **Blattschutz aufheben**.
- Die erste Zeile einer **Tabelle** enthält die Namen ihrer Spalten. Tippen Sie in einer dieser Zellen einen neuen Namen, benennen Sie die Spalte um: Die Formeln, die den alten Namen verwenden, werden angepasst, und die Tabelle in der Datei bleibt gültig. Hat schon eine andere Spalte den Namen, erhält er eine Zahl dahinter; ein Name darf nicht leer sein, und **Entf** leert diese Zeile nicht.
- Eine Arbeitsmappe, die nicht vollständig gelesen werden konnte, lässt sich ansehen, aber nicht ändern oder speichern.
- Eine Funktion, die JeffCrunch noch nicht kennt, lässt eine neue Formel ohne Wert in der Zelle; die Arbeitsmappe bittet das Programm, das sie öffnet, die Formel zu berechnen.
- Formatieren, Sortieren sowie Suchen und Ersetzen folgen in einer späteren Version. Einfügen und Löschen, Ausschneiden und Einfügen, Ausfüllen sowie die Größe von Zeilen und Spalten finden Sie unter [Zeilen, Spalten und Zellen einfügen und löschen](insert-delete), [Ausschneiden, Kopieren und Einfügen](clipboard), [Zellen ausfüllen und summieren](fill-sum) und [Breite von Spalten und Höhe von Zeilen ändern](sizes).

## Symbol einfügen

Klicken Sie auf **Einfügen > Symbole > Symbol**, um das Dialogfeld Symbol zu öffnen. Wählen Sie ein Zeichen und klicken Sie auf **Einfügen** oder doppelklicken Sie darauf. Es wird an der Einfügemarke der Eingabe eingefügt, die Sie gerade tippen. Tippen Sie nicht, öffnet sich der Eintrag der Zelle mit dem Symbol am Ende, und **Eingabe** schreibt es in die Zelle. Beispiel: Steht in einer Zelle das Wort *Gesamt*, ergibt die Wahl des Euro-Zeichens den Eintrag *Gesamt€*. Die zuletzt verwendeten Symbole stehen zuerst in der Liste.

**Das Zeichen aus einer Schriftart nehmen.** Wählen Sie im Feld **Schriftart** eine Schrift und dann das Zeichen. Das Zeichen wird in einen eigenen Abschnitt gesetzt, der in dieser Schrift erscheint; der übrige Text der Zelle behält seine Schrift. Beispiel: In einer Zelle mit *Gesamt* ergibt ein Häkchen aus einer Symbolschrift *Gesamt✓*, wobei nur das Häkchen in dieser Schrift steht. Ist **(normaler Text)** gewählt, zeichnet die Schrift der Zelle das Symbol, und ein Symbol, das die Schrift nicht enthält, erscheint als Kästchen. In einer Formel wird nur das Zeichen eingefügt.

**Sonderzeichen.** Die Registerkarte **Sonderzeichen** listet den geschützten Leerschritt und den geschützten Bindestrich, den bedingten Trennstrich, den Halbgeviertstrich und den Geviertstrich, den Halbgeviertabstand und den Geviertabstand, die Anführungszeichen und den bedingten Zeilenumbruch ohne Breite, jeweils mit ihrem Namen. Doppelklicken Sie auf eines, um es einzufügen.

**Einem Symbol eine Tastenkombination geben.** Wählen Sie ein Symbol oder ein Sonderzeichen, klicken Sie auf **Tastenkombination**, drücken Sie die gewünschten Tasten, etwa **Strg+Alt+Umschalt+J**, und klicken Sie auf **OK**. Die Tasten fügen dann dieses Symbol ein, genauso wie das Dialogfeld. Lassen Sie die Tasten leer und klicken Sie auf **OK**, um sie zu entfernen. JeffCrunch meldet, wenn ein anderer Befehl die Tasten bereits verwendet. Das Symbol wird zu einem Befehl und ist deshalb auch unter **Datei > Optionen > Anpassen > Tastatur** aufgeführt, wo Sie seine Tasten ändern können (siehe [Tastenkombinationen](keyboard)).

## Neue Arbeitsmappen

Unter **Datei > Optionen > Neue Arbeitsmappen** legen Sie Schriftart, Schriftgrad und die Anzahl der Blätter fest, mit denen jede neue Arbeitsmappe beginnt. Vorhandene Arbeitsmappen werden dadurch nicht verändert.
