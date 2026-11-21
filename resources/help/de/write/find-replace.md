# Wörter suchen und ersetzen
<!-- keywords: erweiterte Suche, klingt ähnlich, Soundex, Wortformen, alle Formen eines Wortes, Satzzeichen ignorieren, Leerzeichen ignorieren, Sonderzeichen, Absatzmarke, Tabstopp, nach Format suchen, Format, fett, Schriftart, Formatvorlage, Platzhalter, Platzhalterzeichen, Muster, regulärer Ausdruck, Gruppen, Trefferanzahl, alle Treffer hervorheben, ersetzen, alle ersetzen, suchen, finden, Wörter ändern, Groß-/Kleinschreibung beachten, nur ganze Wörter, weitersuchen, Strg+F, Strg+H, Bearbeiten, Bereich, ausgeblendeter Text, rückgängig -->

Mit „Suchen“ springen Sie zu Wörtern in Ihrem Dokument. Mit „Ersetzen“ ändern Sie sie. Beide Funktionen arbeiten in einem Bereich rechts im Fenster.

## Wörter suchen

1. Drücken Sie **Strg+F**, oder klicken Sie auf **Start > Bearbeiten > Suchen**. Der Bereich **Suchen und Ersetzen** wird geöffnet, und die Einfügemarke steht in **Suchen nach**.
2. Geben Sie die gesuchten Wörter ein. Drücken Sie dann **Eingabe**, oder klicken Sie auf **Weitersuchen**.

JeffWrite beginnt an der Stelle der Einfügemarke. Es markiert die nächste Stelle, an der Ihre Wörter gefunden werden, und blättert die Seiten so weit, dass Sie sie sehen. Diese Stelle heißt **Treffer**. Klicken Sie erneut auf **Weitersuchen**, um zum nächsten Treffer zu gelangen. Nach dem letzten Treffer beginnt die Suche wieder am Anfang des Dokuments.

Während Sie tippen, zählt der Bereich die Treffer, etwa „9 Treffer“, und jeder Treffer erhält auf den Seiten eine gelbe Markierung. Die Markierungen erscheinen auch in Tabellenzellen, Kopfzeilen, Fußzeilen und Fußnoten. Nach **Weitersuchen** zeigt der Bereich an, bei welchem Treffer Sie sind, etwa „Treffer 2 von 4“. Die Anzahl und die Markierungen ändern sich, wenn Sie **Groß-/Kleinschreibung beachten** oder **Nur ganze Wörter suchen** anklicken oder den Text ändern. Beim Schließen des Bereichs verschwinden sie.

## Mit Platzhaltern suchen

Aktivieren Sie **Platzhalter verwenden**, um nach einem Muster statt nur nach Wörtern zu suchen. Ein **Platzhalterzeichen** steht für andere Zeichen. Diese Zeichen gibt es:

- **?** steht für einen beliebigen Buchstaben. **c?t** findet „cat“ und „cot“.
- **\*** steht für eine beliebige Folge von Buchstaben. **c\*t** findet „cat“ und „cart“.
- **<** ist der Wortanfang, und **>** ist das Wortende. **<der>** findet „der“, aber nicht „oder“.
- **[abc]** steht für einen dieser Buchstaben, **[a-c]** für einen Buchstaben von a bis c und **[!abc]** für jeden Buchstaben außer diesen.
- **{2}** wiederholt den davorstehenden Buchstaben zweimal. **{1,3}** wiederholt ihn ein- bis dreimal.
- **@** steht für einen oder mehrere Buchstaben des davorstehenden Zeichens.
- **\\** vor einem Zeichen sucht das Zeichen selbst. **\\?** findet ein Fragezeichen.
- **^p** ist eine Absatzmarke, **^t** ein Tabstopp, **^#** eine Ziffer, **^$** ein Buchstabe und **^w** ein Leerraum.

Setzen Sie einen Teil des Musters in **( )**, um eine **Gruppe** zu bilden. In **Ersetzen durch** steht **\1** für die erste Gruppe, **\2** für die zweite und so weiter bis **\9**. **^&** steht für alles, was gefunden wurde. Wenn Sie zum Beispiel **(<[A-Z]@>), (<[A-Z]@>)** suchen und durch **\2 \1** ersetzen, wird „Müller, Hans“ zu „Hans Müller“. **Nur ganze Wörter suchen** ist ausgegraut, solange Platzhalter aktiv sind, weil **<** und **>** diese Aufgabe übernehmen.

Enthält das Muster einen Fehler, nennt der Bereich die Art des Fehlers.

Sind Ihre Wörter nicht im Dokument enthalten, meldet der Bereich das, und nichts wird verschoben.

Sie können im Dokument weiterarbeiten, während der Bereich geöffnet ist. Klicken Sie auf die Seite, um zu schreiben. Klicken Sie oben im Bereich auf das **X**, um ihn zu schließen.

## Erweiterte Suche

Diese Optionen finden Sie im Bereich unter **Platzhalter verwenden**. Jede davon ändert, wie JeffWrite die Wörter in **Suchen nach** liest.

- **Klingt ähnlich** findet Wörter, die so klingen wie Ihr Wort. Jedes Wort erhält einen Code aus seinen Lauten, und gefunden werden Wörter mit demselben Code. Geben Sie **Schmidt** ein, findet das Programm auch „Schmitt“.
- **Alle Wortformen suchen** findet alle Formen des Wortes. Geben Sie **laufen** ein, findet es auch „läuft“, „lief“ und „gelaufen“. Die Option arbeitet mit ganzen Wörtern. Geben Sie zwei Wörter ein, etwa **großer Hund**, werden sie nebeneinander in allen ihren Formen gefunden.
- **Satzzeichen ignorieren** überspringt Zeichen wie Bindestriche und Punkte zwischen den Buchstaben. Geben Sie **E-Mail** ein, findet es auch „E Mail“ und „Email“.
- **Leerzeichen ignorieren** überspringt Leerzeichen und Tabstopps zwischen den Buchstaben. Geben Sie **E Mail** ein, findet es „EMail“.

Von **Klingt ähnlich**, **Alle Wortformen suchen** und **Platzhalter verwenden** kann jeweils nur eine Option gleichzeitig aktiv sein. **Klingt ähnlich** und **Alle Wortformen suchen** finden immer ganze Wörter, daher sind **Nur ganze Wörter suchen** und **Groß-/Kleinschreibung beachten** dann deaktiviert.

### Sonderzeichen

Klicken Sie auf **Sonderzeichen**, um einen Code in **Suchen nach** einzufügen: **Absatzmarke** (^p), **Tabstoppzeichen** (^t), **Beliebige Ziffer** (^#), **Beliebiger Buchstabe** (^$), **Leerraum** (^w), **Abschnittswechsel** (^b), **Feld** (^d), **Fußnotenzeichen** (^f), **Endnotenzeichen** (^e) oder **Einfügemarke** (^^). Die Codes funktionieren auch ohne **Platzhalter verwenden**. Ein **Abschnittswechsel** ist die Markierung, die einen Abschnitt beendet. Ein **Feld** ist ein Textteil, den das Programm berechnet, etwa eine Seitenzahl oder ein Datum. **^d** findet den Anfang jedes Feldes. Die Notenzeichen sind die kleinen Zahlen im Text, die auf eine Fußnote oder Endnote verweisen. Diese vier Zeichen werden gefunden, aber **Ersetzen** und **Alle ersetzen** lassen sie unverändert, weil eine Änderung die Bedeutung verlieren würde, für die sie stehen. Die übrigen Zeichen im Feld, etwa **?** und **\***, sind dann nur noch diese Zeichen selbst.

### Format

Klicken Sie auf **Format**, um nur Text mit einem bestimmten Aussehen zu finden: **Fett**, **Kursiv**, **Unterstreichung**, **Durchgestrichen**, **Hervorgehoben**, eine **Schriftart** oder eine **Absatzformatvorlage**. Der Bereich listet die gewählten Formate unter den Schaltflächen auf. Wählen Sie **Keine Formatierung**, um sie zu entfernen.

Sie können **Suchen nach** leer lassen. Dann ist jeder Textabschnitt mit diesem Format ein Treffer. Mit Wörtern in **Suchen nach** werden nur diese Wörter mit dem Format gefunden. **Alle ersetzen** ändert nur diese Wörter.

## Suche eingrenzen

- **Groß-/Kleinschreibung beachten** findet nur Wörter mit denselben Großbuchstaben. Ist die Option aktiv, findet „Plan“ nicht „plan“.
- **Nur ganze Wörter suchen** überspringt längere Wörter, die Ihr Wort enthalten. Ist die Option aktiv, findet „plan“ nicht „planet“.
- **Suche** legt fest, in welche Richtung **Weitersuchen** ab der Einfügemarke sucht. **Abwärts** geht bis zum Ende des Dokuments, **Aufwärts** bis zum Anfang. Dort hält die Suche an und meldet das. **Alle** läuft bis zum anderen Ende weiter und ist die übliche Wahl. **Alle ersetzen** ändert immer das gesamte Dokument.

## Wörter ersetzen

1. Drücken Sie **Strg+H**, oder klicken Sie auf **Start > Bearbeiten > Ersetzen**. Die Einfügemarke springt zu **Ersetzen durch**.
2. Geben Sie die neuen Wörter in **Ersetzen durch** ein. Wenn Sie die gefundenen Wörter entfernen möchten, lassen Sie das Feld leer.
3. Geben Sie die alten Wörter in **Suchen nach** ein.
4. Klicken Sie auf **Ersetzen**, um den ersten Treffer anzuzeigen. Klicken Sie erneut auf **Ersetzen**, um ihn zu ändern. JeffWrite wechselt dann zum nächsten Treffer. Jede Änderung ist ein einzelner Schritt, daher nimmt **Rückgängig** (Strg+Z) jeweils eine Änderung zurück.
5. Klicken Sie auf **Alle ersetzen**, um alle Treffer auf einmal zu ändern. Der Bereich teilt mit, wie viele geändert wurden. **Alle ersetzen** ändert auch Wörter in Textfeldern und Formen und ist ein einziger Schritt, daher stellt ein einziges **Rückgängig** alle Wörter wieder her. Ist **Änderungen nachverfolgen** aktiv, wird jede Ersetzung als nachverfolgte Änderung festgehalten, im Text und in den Feldern. Ein Feld, das zu groß zum Ändern ist, wird in der Meldung mitgezählt. **Alle ersetzen** erreicht keine Textfelder in Kopf- und Fußzeilen, gemeinsam gruppierte Formen oder miteinander verknüpfte Textfelder. **Weitersuchen** durchsucht Textfelder noch nicht.

Die neuen Wörter übernehmen den Anfangsbuchstaben der ersetzten Wörter. Ersetzen Sie ein fettes Wort, ist das neue Wort ebenfalls fett.

Beispiel: Um überall „color“ in „colour“ zu ändern, geben Sie **color** in **Suchen nach** ein, **colour** in **Ersetzen durch** und klicken auf **Alle ersetzen**.

## Was die Suche durchsucht

- Sie durchsucht den Text des Dokuments, einschließlich des Texts in Tabellen. Eine Textmarke mitten in einzelnen Wörtern, wie sie manche Dateien enthalten, verhindert nicht, dass diese Wörter gefunden werden. **Ersetzen** und **Alle ersetzen** behalten die Textmarke bei.
- **Weitersuchen** sucht zuerst im Text. Dann geht es weiter zu den Kopfzeilen, danach zu den Fußzeilen und den Fußnoten und schließlich zurück zum Anfang des Textes. Liegt ein Treffer in einem dieser Bereiche, öffnet JeffWrite ihn zum Bearbeiten und markiert den Treffer. Der Bereich zeigt an, wo er sich befindet, etwa „Gefunden in der Kopfzeile von Seite 3“. **Ersetzen** tauscht den aktuellen Treffer aus, im Text, in einer Kopfzeile, einer Fußzeile oder einer Fußnote. **Alle ersetzen** ändert jeden Treffer in all diesen Bereichen, und ein einziges **Rückgängig** stellt sie alle wieder her. Endnoten gehören zum Text und werden daher mit ihm durchsucht. Siehe [Kopf- und Fußzeilen](headers-footers).
- Text, der auf **Ausgeblendet** gesetzt ist, wird übersprungen, weil er nicht angezeigt wird. Siehe [Textaussehen ändern](font).

## Siehe auch

- [Textaussehen ändern](font)
- [JeffWrite mit der Tastatur bedienen](keyboard)
