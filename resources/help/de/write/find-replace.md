# Wörter suchen und ersetzen
<!-- keywords: erweiterte Suche, klingt ähnlich, Soundex, Wortformen, alle Formen eines Wortes, Satzzeichen ignorieren, Leerzeichen ignorieren, Sonderzeichen, Absatzmarke, Tabstopp, nach Format suchen, Format, fett, Schriftart, Formatvorlage, Platzhalter, Platzhalterzeichen, Muster, regulärer Ausdruck, Gruppen, Trefferanzahl, alle Treffer markieren, ersetzen, alle ersetzen, suchen, Groß- und Kleinschreibung, ganze Wörter, weitersuchen, Strg+F, Strg+H, Bearbeiten, Bereich, ausgeblendeter Text, rückgängig -->

Mit **Suchen** springen Sie zu Wörtern in Ihrem Dokument. Mit **Ersetzen** ändern Sie sie. Beide Funktionen arbeiten in einem gemeinsamen Bereich auf der rechten Seite des Fensters.

## Wörter suchen

1. Drücken Sie **Strg+F**, oder klicken Sie auf **Start > Bearbeiten > Suchen**. Der Bereich **Suchen und Ersetzen** wird geöffnet, und der Cursor steht in **Suchen nach**.
2. Geben Sie die gesuchten Wörter ein. Drücken Sie dann **Eingabe**, oder klicken Sie auf **Weitersuchen**.

JeffWrite beginnt an der Stelle, an der Ihr Cursor steht. Es markiert die nächste Stelle, an der es Ihre Wörter findet, und blättert die Seiten so, dass Sie sie sehen. Diese Stelle heißt **Treffer**. Klicken Sie erneut auf **Weitersuchen**, um zum nächsten Treffer zu gelangen. Nach dem letzten Treffer beginnt die Suche wieder am Anfang des Dokuments.

Während Sie tippen, zählt der Bereich die Treffer, etwa „9 Treffer“, und jeder Treffer erhält einen gelben Balken auf den Seiten. Die Balken erscheinen auch in Tabellenzellen, Kopf- und Fußzeilen sowie Fußnoten. Nach **Weitersuchen** zeigt der Bereich an, bei welchem Treffer Sie sind, etwa „Treffer 2 von 4“. Die Anzahl und die Balken ändern sich, wenn Sie **Groß-/Kleinschreibung beachten** oder **Nur ganze Wörter suchen** anklicken oder den Text ändern. Beim Schließen des Bereichs verschwinden sie.

## Mit Platzhaltern suchen

Aktivieren Sie **Platzhalter verwenden**, um nach einem Muster statt nur nach Wörtern zu suchen. Ein **Platzhalter** ist ein Zeichen, das für andere Buchstaben steht. Diese Zeichen gibt es:

- **?** steht für einen beliebigen Buchstaben. **c?t** findet „cat“ und „cot“.
- **\*** steht für eine beliebige Folge von Buchstaben. **c\*t** findet „cat“ und „cart“.
- **<** ist der Anfang eines Wortes, und **>** ist das Ende eines Wortes. **<the>** findet „the“, aber nicht „other“.
- **[abc]** steht für einen dieser Buchstaben, **[a-c]** für jeden Buchstaben von a bis c, und **[!abc]** für jeden Buchstaben außer diesen.
- **{2}** wiederholt den davorstehenden Buchstaben zweimal. **{1,3}** wiederholt ihn einmal bis dreimal.
- **@** steht für einen oder mehrere Buchstaben des davorstehenden Zeichens.
- **\\** vor einem Zeichen findet das Zeichen selbst. **\\?** findet ein Fragezeichen.
- **^p** ist eine Absatzmarke, **^t** ein Tabstopp, **^#** eine Ziffer, **^$** ein Buchstabe und **^w** ein Leerraum.

Setzen Sie einen Teil des Musters in **( )**, um eine **Gruppe** zu bilden. Im Feld **Ersetzen durch** steht **\1** für die erste Gruppe, **\2** für die zweite und so weiter bis **\9**. **^&** steht für den gesamten gefundenen Text. Wenn Sie zum Beispiel **(<[A-Z]@>), (<[A-Z]@>)** suchen und durch **\2 \1** ersetzen, wird „Smith, John“ zu „John Smith“. **Nur ganze Wörter suchen** ist abgeblendet, solange Platzhalter aktiviert sind, weil **<** und **>** diese Aufgabe übernehmen.

Hat das Muster einen Fehler, nennt der Bereich ihn.

Sind Ihre Wörter nicht im Dokument enthalten, zeigt der Bereich das an, und es wird nichts verschoben.

Sie können im Dokument weiterarbeiten, während der Bereich geöffnet ist. Klicken Sie auf die Seite, um zu schreiben. Klicken Sie auf das **X** oben im Bereich, um ihn zu schließen.

## Erweiterte Suche

Diese Optionen finden Sie im Bereich unter **Platzhalter verwenden**. Jede davon ändert, wie JeffWrite die Wörter in **Suchen nach** liest.

- **Klingt ähnlich** findet Wörter, die so ausgesprochen werden wie Ihr Wort. Jedes Wort erhält einen Code aus seinen Lauten, und das Programm findet Wörter mit demselben Code. Geben Sie **Schmidt** ein, findet das Programm auch „Schmitt“.
- **Alle Wortformen suchen** findet jede Form des Wortes. Geben Sie **laufen** ein, findet es „läuft“, „laufend“ und „lief“. Die Option arbeitet mit ganzen Wörtern. Wenn Sie zwei Wörter eingeben, etwa **großer Hund**, findet es sie nebeneinander in beliebigen Formen.
- **Satzzeichen ignorieren** überspringt Zeichen wie Bindestriche und Punkte zwischen den Buchstaben. Geben Sie **email** ein, findet es auch „e-mail“.
- **Leerzeichen ignorieren** überspringt Leerzeichen und Tabstopps zwischen den Buchstaben. Geben Sie **e mail** ein, findet es „email“.

Von **Klingt ähnlich**, **Alle Wortformen suchen** und **Platzhalter verwenden** kann immer nur eine Option aktiv sein. **Klingt ähnlich** und **Alle Wortformen suchen** finden immer ganze Wörter, daher sind **Nur ganze Wörter suchen** und **Groß-/Kleinschreibung beachten** abgeblendet.

### Sonderzeichen

Klicken Sie auf **Sonderzeichen**, um einen Code in **Suchen nach** einzufügen: **Absatzmarke** (^p), **Tabstoppzeichen** (^t), **Beliebige Ziffer** (^#), **Beliebiger Buchstabe** (^$), **Leerraum** (^w), **Abschnittswechsel** (^b), **Feld** (^d), **Fußnotenzeichen** (^f), **Endnotenzeichen** (^e) oder **Caret-Zeichen** (^^). Die Codes funktionieren auch ohne **Platzhalter verwenden**. Ein **Abschnittswechsel** ist die Markierung, die einen Abschnitt beendet. Ein **Feld** ist ein Textstück, das das Programm berechnet, etwa eine Seitenzahl oder ein Datum, und **^d** findet jeden Feldanfang. Die Notenzeichen sind die kleinen Zahlen im Text, die auf eine Fußnote oder Endnote verweisen. Diese vier Codes werden gefunden, aber **Ersetzen** und **Alle ersetzen** lassen sie unverändert, weil eine Änderung ihre Bedeutung zerstören würde. Die übrigen Zeichen in der Liste, etwa **?** und **\***, sind dann einfach diese Zeichen.

### Format

Klicken Sie auf **Format**, um nur Text mit einer bestimmten Darstellung zu finden: **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Hervorgehoben**, eine **Schriftart** oder eine **Formatvorlage** für Absätze. Der Bereich listet die gewählten Formate unter den Schaltflächen auf. Wählen Sie **Keine Formatierung**, um sie zu entfernen.

Sie können **Suchen nach** leer lassen. Dann ist jeder Textabschnitt mit dieser Darstellung ein Treffer. Mit Wörtern in **Suchen nach** werden nur diese Wörter mit der Darstellung gefunden. **Alle ersetzen** ändert dann nur diese Wörter.

## Suche eingrenzen

- **Groß-/Kleinschreibung beachten** findet nur Wörter mit denselben Großbuchstaben. Ist die Option aktiv, findet „Plan“ nicht „plan“.
- **Nur ganze Wörter suchen** überspringt längere Wörter, die Ihr Wort enthalten. Ist die Option aktiv, findet „plan“ nicht „planet“.

## Wörter ersetzen

1. Drücken Sie **Strg+H**, oder klicken Sie auf **Start > Bearbeiten > Ersetzen**. Der Cursor springt in **Ersetzen durch**.
2. Geben Sie die neuen Wörter in **Ersetzen durch** ein. Um die gefundenen Wörter zu entfernen, lassen Sie das Feld leer.
3. Geben Sie die alten Wörter in **Suchen nach** ein.
4. Klicken Sie auf **Ersetzen**, um den ersten Treffer anzuzeigen. Klicken Sie erneut auf **Ersetzen**, um ihn zu ändern. JeffWrite springt dann zum nächsten Treffer. Jede Änderung ist ein Schritt, daher nimmt **Rückgängig** (Strg+Z) jeweils eine Änderung zurück.
5. Klicken Sie auf **Alle ersetzen**, um alle Treffer auf einmal zu ändern. Der Bereich teilt Ihnen mit, wie viele Treffer geändert wurden. **Alle ersetzen** ist ein einziger Schritt, daher stellt ein **Rückgängig** alle Wörter wieder her.

Die neuen Wörter übernehmen die Schreibweise der Wörter, die sie ersetzen. Ersetzen Sie ein fettes Wort, ist das neue Wort ebenfalls fett.

Beispiel: Um „color“ überall durch „colour“ zu ersetzen, geben Sie **color** in **Suchen nach** ein, **colour** in **Ersetzen durch** und klicken auf **Alle ersetzen**.

## Was die Suche berücksichtigt

- Sie durchsucht den Text des Dokuments, einschließlich des Texts in Tabellen. Eine Textmarke mitten in manchen Wörtern, wie sie in einigen Dateien vorkommt, verhindert nicht, dass diese Wörter gefunden werden. **Ersetzen** und **Alle ersetzen** behalten die Textmarke bei.
- **Weitersuchen** durchsucht zuerst den Text. Danach folgen die Kopfzeilen, dann die Fußzeilen, dann die Fußnoten und schließlich wieder der Anfang des Texts. Befindet sich ein Treffer in einem dieser Bereiche, öffnet JeffWrite ihn zum Bearbeiten und markiert den Treffer. Der Bereich zeigt an, wo er sich befindet, etwa „Gefunden in der Kopfzeile von Seite 3.“ **Ersetzen** tauscht den aktuellen Treffer aus, im Text, in einer Kopf- oder Fußzeile oder in einer Fußnote. **Alle ersetzen** ändert alle Treffer in all diesen Bereichen, und ein **Rückgängig** stellt sie alle wieder her. Die Endnoten gehören zum Text und werden mit ihm durchsucht. Siehe [Kopf- und Fußzeilen](headers-footers).
- Die Suche überspringt Text, der auf **Ausgeblendet** gesetzt ist, weil er nicht angezeigt wird. Siehe [Textdarstellung ändern](font).

## Siehe auch

- [Textdarstellung ändern](font)
- [JeffWrite mit der Tastatur verwenden](keyboard)
