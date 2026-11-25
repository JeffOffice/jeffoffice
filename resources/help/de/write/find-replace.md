# Suchen und Ersetzen
<!-- keywords: erweiterte Suche, klingt ähnlich, Soundex, Wortformen, alle Formen eines Wortes, Satzzeichen ignorieren, Leerzeichen ignorieren, Sonderzeichen, Absatzmarke, Tabstopp, nach Format suchen, Format, fett, Schriftart, Formatvorlage, Platzhalter, Platzhalter-Zeichen, Muster, regulärer Ausdruck, Gruppen, Trefferanzahl, alle Treffer markieren, ersetzen, alle ersetzen, suchen, finden, Wörter ändern, Groß-/Kleinschreibung, ganze Wörter, Weitersuchen, Strg+F, Strg+H, bearbeiten, Bereich, ausgeblendeter Text, rückgängig, Suchen, Suche, Textersetzung, Ersetzen, Wörter suchen -->

Mit der Suche springen Sie zu Wörtern in Ihrem Dokument. Mit dem Ersetzen ändern Sie sie. Beide Funktionen arbeiten im gleichen Bereich rechts im Fenster.

## Wörter suchen

1. Drücken Sie **Strg+F**, oder klicken Sie auf **Start > Bearbeiten > Suchen**. Der Bereich **Suchen und Ersetzen** öffnet sich, und die Einfügemarke steht im Feld **Suchen nach**.
2. Geben Sie die gesuchten Wörter ein. Drücken Sie dann **Eingabe**, oder klicken Sie auf **Weitersuchen**.

JeffWrite beginnt an der Stelle, an der Ihre Einfügemarke steht. Es markiert die nächste Stelle, an der Ihre Wörter gefunden werden, und blättert die Seiten so, dass Sie sie sehen. Diese Stelle heißt **Treffer**. Klicken Sie erneut auf **Weitersuchen**, um zum nächsten Treffer zu gelangen. Nach dem letzten Treffer springt die Suche zurück zum Anfang des Dokuments.

Während Sie tippen, zählt der Bereich die Treffer, etwa „9 Treffer“, und jeder Treffer erhält auf den Seiten eine gelbe Markierung. Die Markierungen erscheinen auch in Tabellenzellen, Kopf- und Fußzeilen sowie Fußnoten. Nach **Weitersuchen** zeigt der Bereich an, bei welchem Treffer Sie sind, etwa „Treffer 2 von 4“. Die Anzahl und die Markierungen ändern sich, wenn Sie **Groß-/Kleinschreibung beachten** oder **Nur ganze Wörter suchen** anklicken oder den Text ändern. Sie verschwinden, wenn Sie den Bereich schließen.

## Mit Platzhaltern suchen

Aktivieren Sie **Platzhalter verwenden**, um nach einem Muster statt nur nach Wörtern zu suchen. Ein **Platzhalter** ist ein Zeichen, das für andere Buchstaben steht. Diese Zeichen gibt es:

- **?** steht für einen beliebigen Buchstaben. **K?t** findet „Kat“ und „Kot“.
- **\*** steht für eine beliebige Folge von Buchstaben. **K\*t** findet „Kat“ und „Kart“.
- **<** ist der Anfang eines Wortes, und **>** ist das Ende eines Wortes. **<der>** findet „der“, aber nicht „oder“.
- **[abc]** ist einer dieser Buchstaben, **[a-c]** ist ein beliebiger Buchstabe von a bis c, und **[!abc]** ist ein beliebiger Buchstabe außer diesen.
- **{2}** wiederholt den davorstehenden Buchstaben zweimal. **{1,3}** wiederholt ihn ein- bis dreimal.
- **@** bedeutet einen oder mehrere Buchstaben des davorstehenden Zeichens.
- **\\** vor einem Zeichen findet das Zeichen selbst. **\\?** findet ein Fragezeichen.
- **^p** ist eine Absatzmarke, **^t** ist ein Tabstopp, **^#** ist eine Ziffer, **^$** ist ein Buchstabe, und **^w** ist Leerraum.

Setzen Sie einen Teil des Musters in **( )**, um eine **Gruppe** zu bilden. In **Ersetzen durch** steht **\1** für die erste Gruppe, **\2** für die zweite und so weiter bis **\9**. **^&** steht für alles, was gefunden wurde. Wenn Sie zum Beispiel **(<[A-Z]@>), (<[A-Z]@>)** suchen und durch **\2 \1** ersetzen, wird aus „Müller, Anna“ „Anna Müller“. **Nur ganze Wörter suchen** ist abgeblendet, solange Platzhalter aktiv sind, weil **<** und **>** diese Aufgabe übernehmen.

Enthält das Muster einen Fehler, nennt der Bereich die Ursache.

Sind Ihre Wörter nicht im Dokument enthalten, meldet der Bereich das, und nichts wird verschoben.

Sie können im Dokument weiterarbeiten, während der Bereich geöffnet ist. Klicken Sie auf die Seite, um zu schreiben. Klicken Sie auf das **X** oben im Bereich, um ihn zu schließen.

## Mit regulären Ausdrücken suchen

Aktivieren Sie **Reguläre Ausdrücke verwenden** für Muster, die über Platzhalter hinausgehen. Ein **regulärer Ausdruck** ist ein kurzes Muster aus Buchstaben und Zeichen. Er wird anstelle von **Platzhalter verwenden** eingeschaltet, denn nur eines von beiden kann aktiv sein. Diese Zeichen sind am häufigsten:

- **.** steht für ein beliebiges Zeichen, **\d** für eine Ziffer, **\w** für einen Buchstaben, eine Ziffer oder einen Unterstrich, und **\s** für ein Leerzeichen oder einen Tabstopp. **[a-c]** ist ein beliebiger Buchstabe von a bis c, und **[^abc]** ist ein beliebiges Zeichen außer diesen.
- **^** ist der Anfang eines Absatzes, **$** ist das Ende eines Absatzes, und **\b** ist die Grenze eines Wortes. **\bKatze\b** findet „Katze“, aber nicht „Katzen“.
- **\*** bedeutet null oder mehr des davorstehenden Zeichens, **+** bedeutet eines oder mehr, und **?** bedeutet null oder eines. **{2}** bedeutet genau zweimal, und **{2,5}** bedeutet zwei- bis fünfmal; eine Anzahl kann bis zu 200 betragen. Setzen Sie **?** hinter eine Wiederholung, wie bei **.+?**, um die kürzeste passende Folge zu finden.
- **|** bedeutet „oder“: **Katze|Hund** findet eines der beiden Wörter.
- **( )** bildet eine **Gruppe**. Verwenden Sie **(?: )** für eine Gruppe, die Sie in **Ersetzen durch** nicht brauchen.
- **\\** vor einem Zeichen findet das Zeichen selbst: **\.** findet einen Punkt. **\t** ist ein Tabstopp.

**Groß-/Kleinschreibung beachten** funktioniert wie bei Wörtern. Buchstaben mit Akzenten und Buchstaben anderer Alphabete zählen als Buchstaben.

Ein Muster wird jeweils absatzweise durchsucht, sodass **^** und **$** den Anfang und das Ende jedes Absatzes bedeuten. Um über ein Absatzende hinweg zu suchen, setzen Sie **\n** (eine Absatzmarke) in das Muster: **Heim\.\nMüller** findet „Heim.“ am Ende eines Absatzes und „Müller“ am Anfang des nächsten.

In **Ersetzen durch** steht **$1** oder **\1** für die erste Gruppe, **$2** oder **\2** für die zweite und so weiter bis zu neun Gruppen. **$0** oder **\0** steht für alles, was gefunden wurde. **$$** ist ein Dollarzeichen, **\\** ist ein Backslash, und **\t** ist ein Tabstopp. Wenn Sie zum Beispiel **(\w+), (\w+)** suchen und durch **$2 $1** ersetzen, wird aus „Müller, Anna“ „Anna Müller“.

**Alle ersetzen** ändert alle Treffer im Text, in Kopf- und Fußzeilen, Notizen und Textfeldern in einem Schritt, sodass ein einziges **Rückgängig** alle zurücknimmt. Ist **Änderungen nachverfolgen** aktiv, wird jede Änderung nachverfolgt, wie bei Wörtern. Ausgeblendeter Text bleibt unverändert.

Enthält das Muster einen Fehler, nennt der Bereich die Stelle des Zeichens, in deren Nähe er liegt. Ein Muster kann auch gar keine Zeichen treffen, etwa **^** (der Absatzanfang) oder **$** (das Absatzende). **Ersetzen durch** fügt dann seinen Text an dieser Stelle ein, sodass **^** mit **> ** diese Zeichen vor jedem Absatz einfügt. Ein Muster, das einen sehr langen Abschnitt eines sehr langen Absatzes (zehntausende Zeichen) trifft, kann mit der Meldung „Benötigte zu viel Aufwand“ angehalten werden; das ist ein Hinweis auf das Muster. Ein Muster, das so viel Rechenaufwand braucht, dass es für Ihren Text zu lange dauern würde, etwa **(a+)+$** in einer langen Zeile aus a-Buchstaben, wird von JeffWrite angehalten, und JeffWrite meldet das; nichts wird geändert. Eine lange Suche zeigt ein Fenster **Suche läuft** mit einer Schaltfläche **Abbrechen**; bei Abbruch bleibt Ihr Dokument unverändert, und Sie können erst im Dokument schreiben, wenn die Suche beendet ist. Muster mit Vorausschau oder Rückschau, die die geprüften Zeichen nicht verbrauchen, etwa **x(?=\d)**, sowie Muster im erweiterten Modus, etwa **(?x)**, werden nicht unterstützt; JeffWrite weist darauf hin. Ein sehr langer Absatz wird stückweise durchsucht, und jeder Treffer ist der, den eine Suche im ganzen Absatz finden würde. Ein Muster, das einen sehr langen Abschnitt eines sehr langen Absatzes (zehntausende Zeichen) trifft, kann mit der Meldung „Benötigte zu viel Aufwand“ angehalten werden, und ein einzelner Treffer von mehr als 200.000 Zeichen wird nicht ersetzt: JeffWrite hält an, meldet es und ändert nichts.

## Erweiterte Suche

Diese Optionen finden Sie im Bereich unter **Reguläre Ausdrücke verwenden**. Jede davon ändert, wie JeffWrite die Wörter in **Suchen nach** liest.

- **Klingt ähnlich** findet Wörter, die so ausgesprochen werden wie Ihr Wort. Es gibt jedem Wort einen Code aus seinen Lauten und findet die Wörter mit demselben Code. Geben Sie **Schmidt** ein, findet es auch „Schmitt“.
- **Alle Wortformen suchen** findet jede Form des Wortes. Geben Sie **laufen** ein, findet es „läuft“, „lief“ und „gelaufen“. Die Option arbeitet mit ganzen Wörtern. Geben Sie zwei Wörter ein, etwa **großer Hund**, findet sie beide nebeneinander, in beliebigen ihrer Formen.
- **Satzzeichen ignorieren** überspringt Zeichen wie Bindestriche und Punkte zwischen den Buchstaben. Geben Sie **Emailadresse** ein, findet es auch „E-Mail-Adresse“.
- **Leerzeichen ignorieren** überspringt Leerzeichen und Tabstopps zwischen den Buchstaben. Geben Sie **E Mail** ein, findet es „E-Mail“.

Nur eine der Optionen **Klingt ähnlich**, **Alle Wortformen suchen**, **Platzhalter verwenden** und **Reguläre Ausdrücke verwenden** kann gleichzeitig aktiv sein. **Klingt ähnlich** und **Alle Wortformen suchen** finden immer ganze Wörter, daher sind **Nur ganze Wörter suchen** und **Groß-/Kleinschreibung beachten** dann abgeblendet.

### Sonderzeichen

Klicken Sie auf **Sonderzeichen**, um einen Code in **Suchen nach** einzufügen: **Absatzmarke** (^p), **Tabstopp** (^t), **Beliebige Ziffer** (^#), **Beliebiger Buchstabe** (^$), **Leerraum** (^w), **Abschnittswechsel** (^b), **Feld** (^d), **Fußnotenzeichen** (^f), **Endnotenzeichen** (^e) oder **Einfügemarkenzeichen** (^^). Die Codes funktionieren auch ohne **Platzhalter verwenden**. Ein **Abschnittswechsel** ist die Markierung, die einen Abschnitt beendet. Ein **Feld** ist ein Textteil, den das Programm berechnet, etwa eine Seitenzahl oder ein Datum, und **^d** findet den Anfang jedes Feldes. Die Notenzeichen sind die kleinen Zahlen im Text, die auf eine Fußnote oder Endnote verweisen. Diese vier Zeichen werden gefunden, aber **Ersetzen** und **Alle ersetzen** lassen sie unverändert, weil eine Änderung ihre Bedeutung verlieren würde. Die übrigen Zeichen in der Liste, etwa **?** und **\***, sind dann nur noch diese Zeichen.

### Format

Klicken Sie auf **Format**, um nur Text zu finden, der ein bestimmtes Aussehen hat: **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Hervorgehoben**, eine **Schriftart** oder eine **Absatzformatvorlage**. Der Bereich listet die gewählten Formate unter den Schaltflächen auf. Wählen Sie **Keine Formatierung**, um sie zu entfernen.

Sie können **Suchen nach** leer lassen. Dann ist jeder Textabschnitt mit diesem Format ein Treffer. Mit Wörtern in **Suchen nach** werden nur die Wörter mit diesem Format gefunden. **Alle ersetzen** ändert nur diese Wörter.

## Suche eingrenzen

- **Groß-/Kleinschreibung beachten** findet nur Wörter mit denselben Großbuchstaben. Ist die Option aktiv, findet „Plan“ nicht „plan“.
- **Nur ganze Wörter suchen** überspringt längere Wörter, die Ihres enthalten. Ist die Option aktiv, findet „Plan“ nicht „Planet“.
- **Suchen** legt fest, in welche Richtung **Weitersuchen** ab der Einfügemarke sucht. **Abwärts** geht bis zum Ende des Dokuments, und **Aufwärts** bis zum Anfang. Die Suche hält dort an und meldet es. **Alle** sucht weiter bis zum anderen Ende und ist die übliche Wahl. **Alle ersetzen** ändert immer das gesamte Dokument.

## Wörter ersetzen

1. Drücken Sie **Strg+H**, oder klicken Sie auf **Start > Bearbeiten > Ersetzen**. Die Einfügemarke springt zu **Ersetzen durch**.
2. Geben Sie die neuen Wörter in **Ersetzen durch** ein. Um die gefundenen Wörter zu entfernen, lassen Sie das Feld leer.
3. Geben Sie die alten Wörter in **Suchen nach** ein.
4. Klicken Sie auf **Ersetzen**, um den ersten Treffer zu sehen. Klicken Sie erneut auf **Ersetzen**, um ihn zu ändern. JeffWrite springt dann zum nächsten Treffer. Jede Änderung ist ein Schritt, daher nimmt **Rückgängig** (Strg+Z) jeweils eine Änderung zurück.
5. Klicken Sie auf **Alle ersetzen**, um alle Treffer auf einmal zu ändern. Der Bereich teilt Ihnen mit, wie viele Treffer geändert wurden. **Alle ersetzen** ändert auch die Wörter in Textfeldern und Formen und ist ein einziger Schritt, daher stellt ein einziges **Rückgängig** alle Wörter wieder her. Ist **Änderungen nachverfolgen** aktiv, wird jede Ersetzung als nachverfolgte Änderung festgehalten, im Text und in den Feldern. Ein Feld, das zu groß zum Ändern ist, wird in der Meldung mitgezählt. **Alle ersetzen** erreicht keine Textfelder in Kopf- und Fußzeilen, gruppierte Formen oder miteinander verknüpfte Textfelder. **Weitersuchen** durchsucht Textfelder noch nicht.

Die neuen Wörter übernehmen den Anfangsbuchstaben der ersetzten Wörter. Wenn Sie ein fettes Wort ersetzen, ist das neue Wort ebenfalls fett.

Um zum Beispiel „Farbe“ überall in „Farbe“ zu ändern, geben Sie **Farbe** in **Suchen nach** ein, **Farbton** in **Ersetzen durch** und klicken Sie auf **Alle ersetzen**.

## Was die Suche durchsucht

- Sie durchsucht den Text des Dokuments einschließlich des Textes in Tabellen. Eine Textmarke mitten in einzelnen Wörtern, wie sie manche Dateien enthalten, verhindert nicht, dass diese Wörter gefunden werden. **Ersetzen** und **Alle ersetzen** behalten die Textmarke bei.
- **Weitersuchen** sucht zuerst im Text. Danach folgen die Kopfzeilen, dann die Fußzeilen, dann die Fußnoten, und dann geht es zurück zum Anfang des Textes. Liegt ein Treffer in einem dieser Bereiche, öffnet JeffWrite ihn zum Bearbeiten und markiert den Treffer. Der Bereich zeigt an, wo er sich befindet, etwa „Gefunden in der Kopfzeile von Seite 3“. **Ersetzen** tauscht den aktuellen Treffer aus, im Text oder in einer Kopf-, Fußzeile oder Fußnote. **Alle ersetzen** ändert alle Treffer in all diesen Bereichen, und ein einziges **Rückgängig** stellt sie alle wieder her. Die Endnoten gehören zum Text und werden mit ihm durchsucht. Siehe [Kopf- und Fußzeilen](headers-footers).
- Ausgeblendeter Text, also Text, der auf **Ausgeblendet** gesetzt ist, wird übersprungen, weil er nicht angezeigt wird. Siehe [Textdarstellung ändern](font).

## Siehe auch

- [Textdarstellung ändern](font)
- [JeffWrite mit der Tastatur verwenden](keyboard)
