# Wörter suchen und ersetzen
<!-- keywords: erweiterte Suche, klingt ähnlich, Soundex, Wortformen, alle Formen eines Wortes, Satzzeichen ignorieren, Leerzeichen ignorieren, Sonderzeichen, Absatzmarke, Tabstopp, nach Format suchen, Format, fett, Schriftart, Formatvorlage, Platzhalter, Platzhalterzeichen, Muster, regulärer Ausdruck, Gruppen, Trefferanzahl, alle Treffer markieren, ersetzen, alle ersetzen, suchen, Wörter ändern, Groß-/Kleinschreibung, ganze Wörter, weitersuchen, strg+f, strg+h, bearbeiten, Bereich, ausgeblendeter Text, rückgängig -->

Mit **Suchen** springen Sie in Ihrem Dokument zu bestimmten Wörtern. Mit **Ersetzen** ändern Sie sie. Beide Funktionen arbeiten in einem Bereich rechts im Fenster.

## Wörter suchen

1. Drücken Sie **Strg+F**, oder klicken Sie auf **Start > Bearbeiten > Suchen**. Der Bereich **Suchen und Ersetzen** öffnet sich, und die Einfügemarke steht im Feld **Suchen nach**.
2. Geben Sie die gewünschten Wörter ein. Drücken Sie dann **Eingabe**, oder klicken Sie auf **Weitersuchen**.

JeffWrite beginnt an der Stelle, an der sich Ihre Einfügemarke befindet. Es markiert die nächste Stelle, an der Ihre Wörter gefunden werden, und blättert die Seiten so weit, dass Sie sie sehen. Diese Stelle heißt **Treffer**. Klicken Sie erneut auf **Weitersuchen**, um zum nächsten Treffer zu gelangen. Nach dem letzten Treffer springt das Programm wieder zum Anfang des Dokuments.

Während Sie tippen, zählt der Bereich die Treffer, etwa „9 Treffer“, und jeder Treffer erhält auf den Seiten einen gelben Streifen. Die Streifen erscheinen auch in Tabellenzellen, Kopf- und Fußzeilen sowie Fußnoten. Nach **Weitersuchen** zeigt der Bereich an, bei welchem Treffer Sie sind, etwa „Treffer 2 von 4“. Die Anzahl und die Streifen ändern sich, wenn Sie **Groß-/Kleinschreibung beachten** oder **Nur ganze Wörter suchen** anklicken oder den Text ändern. Die Streifen bleiben auf den Seiten, wenn Sie den Bereich schließen und wenn Sie **Lesemodus** anklicken, damit Sie mit allen Treffern im Blick lesen können. Am Dokument ändert sich dabei nichts, und die Streifen werden nie gedruckt oder gespeichert. Sie verschwinden, wenn Sie das Dokument ändern, wenn Sie den Bereich erneut öffnen oder wenn Sie auf **Start > Bearbeiten > Suchmarkierungen löschen** klicken.

## Mit Platzhalterzeichen suchen

Aktivieren Sie **Platzhalterzeichen verwenden**, um nach einem Muster statt nach einzelnen Wörtern zu suchen. Ein **Platzhalterzeichen** steht für andere Buchstaben. Diese Zeichen gibt es:

- **?** steht für einen beliebigen Buchstaben. **c?t** findet „cat“ und „cot“.
- **\*** steht für eine beliebige Folge von Buchstaben. **c\*t** findet „cat“ und „cart“.
- **<** ist der Wortanfang, und **>** ist das Wortende. **<the>** findet „the“, aber nicht „other“.
- **[abc]** steht für einen dieser Buchstaben, **[a-c]** für einen Buchstaben von a bis c, und **[!abc]** für einen Buchstaben außer diesen.
- **{2}** wiederholt den davorstehenden Buchstaben zweimal. **{1,3}** wiederholt ihn ein- bis dreimal.
- **@** steht für einen oder mehrere Buchstaben des davorstehenden Zeichens.
- **\\** vor einem Zeichen sucht das Zeichen selbst. **\\?** findet ein Fragezeichen.
- **^p** ist eine Absatzmarke, **^t** ein Tabstopp, **^#** eine Ziffer, **^$** ein Buchstabe und **^w** ein Leerraum.

Setzen Sie einen Teil des Musters in **( )**, um eine **Gruppe** zu bilden. Im Feld **Ersetzen durch** steht **\1** für die erste Gruppe, **\2** für die zweite und so weiter bis **\9**. **^&** steht für alles, was gefunden wurde. Wenn Sie zum Beispiel **(<[A-Z]@>), (<[A-Z]@>)** suchen und durch **\2 \1** ersetzen, wird „Müller, Hans“ zu „Hans Müller“. **Nur ganze Wörter suchen** ist abgeblendet, solange Platzhalter aktiv sind, weil **<** und **>** diese Aufgabe übernehmen.

Enthält das Muster einen Fehler, nennt der Bereich die Art des Fehlers.

Sind Ihre Wörter nicht im Dokument enthalten, meldet der Bereich das, und nichts wird verschoben.

Sie können im Dokument weiterarbeiten, während der Bereich geöffnet ist. Klicken Sie auf die Seite, um zu schreiben. Mit dem **X** oben im Bereich schließen Sie ihn.

## Mit regulären Ausdrücken suchen

Aktivieren Sie **Reguläre Ausdrücke verwenden** für Muster, die über Platzhalterzeichen hinausgehen. Ein **regulärer Ausdruck** ist ein kurzes Muster aus Buchstaben und Zeichen. Er wird anstelle von **Platzhalterzeichen verwenden** eingeschaltet; beide zusammen sind nicht möglich. Dies sind die gebräuchlichen Zeichen:

- **.** steht für ein beliebiges Zeichen, **\d** für eine Ziffer, **\w** für einen Buchstaben, eine Ziffer oder einen Unterstrich, und **\s** für ein Leerzeichen oder einen Tabstopp. **[a-c]** steht für einen Buchstaben von a bis c, und **[^abc]** für ein beliebiges Zeichen außer diesen.
- **^** ist der Anfang eines Absatzes, **$** das Ende eines Absatzes, und **\b** ist die Grenze eines Wortes. **\bKatze\b** findet „Katze“, aber nicht „Katzenfutter“.
- **\*** bedeutet keines oder beliebig viele des davorstehenden Elements, **+** bedeutet eines oder mehrere, und **?** bedeutet keines oder eines. **{2}** bedeutet genau zweimal, und **{2,5}** bedeutet zwei- bis fünfmal; eine Anzahl kann bis zu 200 betragen. Fügen Sie nach einer Wiederholung ein **?** an, wie in **.+?**, um die kürzeste passende Folge zu finden.
- **|** bedeutet „oder“: **Katze|Hund** findet beide Wörter.
- **( )** bildet eine **Gruppe**. Verwenden Sie **(?: )** für eine Gruppe, die Sie im Feld **Ersetzen durch** nicht brauchen.
- **\\** vor einem Zeichen sucht das Zeichen selbst: **\.** findet einen Punkt. **\t** ist ein Tabstopp.

**Groß-/Kleinschreibung beachten** funktioniert wie bei Wörtern. Buchstaben mit Akzenten und Buchstaben aus anderen Alphabeten zählen als Buchstaben.

Ein Muster wird jeweils absatzweise durchsucht, sodass **^** und **$** den Anfang und das Ende jedes Absatzes bedeuten. Um über ein Absatzende hinweg zu suchen, setzen Sie **\n** (eine Absatzmarke) in das Muster: **Heim\.\nLee** findet „Heim.“ am Ende eines Absatzes und „Lee“ am Anfang des nächsten.

Im Feld **Ersetzen durch** steht **$1** oder **\1** für die erste Gruppe, **$2** oder **\2** für die zweite und so weiter bis neun Gruppen. **$0** oder **\0** steht für alles, was gefunden wurde. **$$** ist ein Dollarzeichen, **\\** ein umgekehrter Schrägstrich und **\t** ein Tabstopp. Wenn Sie zum Beispiel **(\w+), (\w+)** suchen und durch **$2 $1** ersetzen, wird „Müller, Hans“ zu „Hans Müller“.

**Alle ersetzen** ändert alle Treffer im Text, in Kopf- und Fußzeilen, Notizen und Textfeldern in einem einzigen Schritt, sodass ein einziges **Rückgängig** alle zurücknimmt. Ist **Änderungen nachverfolgen** aktiv, wird jede Änderung nachverfolgt, wie bei Wörtern. Ausgeblendeter Text bleibt unverändert.

Enthält das Muster einen Fehler, nennt der Bereich das Zeichen, in dessen Nähe der Fehler liegt. Ein Muster kann auch keine Zeichen treffen, etwa **^** (der Absatzanfang) oder **$** (das Absatzende). Dann fügt **Ersetzen durch** seinen Text an dieser Stelle ein. So fügt **^** mit **> ** die Zeichen am Anfang jedes Absatzes ein. Ein Muster, das bei Ihrem Text viel zu lange dauern würde, etwa **(a+)+$** in einer langen Zeile aus a-Zeichen, wird von JeffWrite gestoppt, und das Programm teilt es mit; es wird nichts geändert. Eine lange Suche zeigt ein Fenster **Suche läuft** mit der Schaltfläche **Abbrechen**. Wenn Sie abbrechen, bleibt Ihr Dokument unverändert, und Sie können im Dokument erst weiterschreiben, wenn die Suche beendet ist. Muster, die vorausschauen oder zurückschauen, ohne die geprüften Zeichen zu verbrauchen, etwa **x(?=\d)**, sowie Muster im erweiterten Modus, etwa **(?x)**, werden nicht unterstützt; JeffWrite weist darauf hin. Ein sehr langer Absatz wird stückweise durchsucht, und jeder Treffer ist der, den eine Suche im gesamten Absatz finden würde. Ein Muster, das einen sehr langen Abschnitt eines sehr langen Absatzes (zehntausende Zeichen) trifft, kann mit der Meldung „Es wurde zu viel Aufwand benötigt“ gestoppt werden. Ein einzelner Treffer von mehr als 200.000 Zeichen wird nicht ersetzt: JeffWrite hält an, meldet das und ändert nichts.

## Erweiterte Suche

Diese Optionen finden Sie im Bereich unter **Reguläre Ausdrücke verwenden**. Jede ändert, wie JeffWrite die Wörter im Feld **Suchen nach** liest.

- **Klingt ähnlich** findet Wörter, die so ausgesprochen werden wie Ihr Wort. Das Programm gibt jedem Wort einen Code aus seinen Lauten und findet Wörter mit demselben Code. Geben Sie **Schmidt** ein, findet das Programm auch „Schmid“.
- **Alle Wortformen suchen** findet jede Form des Wortes. Geben Sie **laufen** ein, findet das Programm auch „läuft“, „lief“ und „gelaufen“. Die Option arbeitet mit ganzen Wörtern. Geben Sie zwei Wörter ein, etwa **großer Hund**, findet sie diese nebeneinander, in beliebigen Formen.
- **Satzzeichen ignorieren** überspringt Zeichen wie Bindestriche und Punkte zwischen den Buchstaben. Geben Sie **Emailadresse** ein, findet das Programm auch „E-Mail-Adresse“.
- **Leerzeichen ignorieren** überspringt Leerzeichen und Tabstopps zwischen den Buchstaben. Geben Sie **E Mail** ein, findet das Programm auch „Email“.

Von **Klingt ähnlich**, **Alle Wortformen suchen**, **Platzhalterzeichen verwenden** und **Reguläre Ausdrücke verwenden** kann immer nur eine Option aktiv sein. **Klingt ähnlich** und **Alle Wortformen suchen** finden immer ganze Wörter, daher sind **Nur ganze Wörter suchen** und **Groß-/Kleinschreibung beachten** abgeblendet.

### Sonderzeichen

Klicken Sie auf **Sonderzeichen**, um einen Code in das Feld **Suchen nach** einzufügen: **Absatzmarke** (^p), **Tabstoppzeichen** (^t), **Beliebige Ziffer** (^#), **Beliebiger Buchstabe** (^$), **Leerraum** (^w), **Abschnittswechsel** (^b), **Feld** (^d), **Fußnotenzeichen** (^f), **Endnotenzeichen** (^e) oder **Einfügemarkenzeichen** (^^). Die Codes funktionieren auch ohne **Platzhalterzeichen verwenden**. Ein **Abschnittswechsel** ist die Markierung, die einen Abschnitt beendet. Ein **Feld** ist ein Textstück, das das Programm berechnet, etwa eine Seitenzahl oder ein Datum, und **^d** findet den Anfang jedes Feldes. Die Notizzeichen sind die kleinen Zahlen im Text, die auf eine Fußnote oder eine Endnote verweisen. Diese vier Zeichen werden gefunden, aber **Ersetzen** und **Alle ersetzen** lassen sie unverändert, weil eine Änderung verlieren würde, wofür sie stehen. Die übrigen Zeichen in der Liste, etwa **?** und **\***, sind dann nur diese Zeichen selbst.

### Format

Klicken Sie auf **Format**, um nur Text mit einem bestimmten Aussehen zu finden: **Fett**, **Kursiv**, **Unterstrichen**, **Durchgestrichen**, **Hervorgehoben**, eine **Schriftart** oder eine **Absatzformatvorlage**. Der Bereich listet die gewählten Merkmale unter den Schaltflächen auf. Wählen Sie **Keine Formatierung**, um sie zu entfernen.

Sie können das Feld **Suchen nach** leer lassen. Dann ist jeder Textabschnitt mit diesem Aussehen ein Treffer. Mit Wörtern im Feld **Suchen nach** werden nur die Wörter mit diesem Aussehen gefunden. **Alle ersetzen** ändert nur diese Wörter.

## Suche eingrenzen

- **Groß-/Kleinschreibung beachten** findet nur Wörter mit denselben Großbuchstaben. Ist die Option aktiv, findet „Plan“ nicht „plan“.
- **Nur ganze Wörter suchen** überspringt längere Wörter, die Ihr Wort enthalten. Ist die Option aktiv, findet „plan“ nicht „planet“.
- **Suchen** legt fest, in welche Richtung **Weitersuchen** von der Einfügemarke aus geht. **Abwärts** geht bis zum Ende des Dokuments, **Aufwärts** bis zum Anfang. Dort hält die Suche an und meldet es. **Alle** setzt die Suche am anderen Ende fort und ist die übliche Wahl. **Alle ersetzen** ändert immer das ganze Dokument.

## Wörter ersetzen

1. Drücken Sie **Strg+H**, oder klicken Sie auf **Start > Bearbeiten > Ersetzen**. Die Einfügemarke springt in das Feld **Ersetzen durch**.
2. Geben Sie die neuen Wörter in **Ersetzen durch** ein. Um die gefundenen Wörter zu entfernen, lassen Sie das Feld leer.
3. Geben Sie die alten Wörter in **Suchen nach** ein.
4. Klicken Sie auf **Ersetzen**, um den ersten Treffer zu sehen. Klicken Sie erneut auf **Ersetzen**, um ihn zu ändern. JeffWrite springt dann zum nächsten Treffer. Jede Änderung ist ein Schritt, daher nimmt **Rückgängig** (Strg+Z) jeweils einen Schritt zurück.
5. Klicken Sie auf **Alle ersetzen**, um alle Treffer auf einmal zu ändern. Der Bereich zeigt an, wie viele geändert wurden. **Alle ersetzen** ändert auch die Wörter in Textfeldern und Formen und bildet einen einzigen Schritt, sodass ein einziges **Rückgängig** alle Wörter wiederherstellt. Ist **Änderungen nachverfolgen** aktiv, wird jede Ersetzung im Text und in den Feldern als nachverfolgte Änderung festgehalten. Ein Feld, das zu groß zum Ändern ist, wird in der Meldung mitgezählt. **Alle ersetzen** erreicht keine Textfelder in Kopf- und Fußzeilen, gruppierte Formen oder miteinander verknüpfte Textfelder. **Weitersuchen** durchsucht Textfelder noch nicht.

Die neuen Wörter übernehmen den Anfangsbuchstaben der ersetzten Wörter. Ersetzen Sie ein fett formatiertes Wort, ist das neue Wort ebenfalls fett.

Um zum Beispiel „Farbe“ überall durch „Färbung“ zu ersetzen: Geben Sie **Farbe** in **Suchen nach** ein, geben Sie **Färbung** in **Ersetzen durch** ein, und klicken Sie auf **Alle ersetzen**.

## Was die Suche durchsucht

- Sie durchsucht den Text des Dokuments, einschließlich des Textes in Tabellen. Eine Textmarke mitten in einigen Wörtern, wie sie manche Dateien haben, verhindert nicht, dass diese Wörter gefunden werden. **Ersetzen** und **Alle ersetzen** erhalten die Textmarke.
- **Weitersuchen** durchsucht zuerst den Text. Danach folgen die Kopfzeilen, dann die Fußzeilen, dann die Fußnoten und schließlich wieder der Anfang des Textes. Liegt ein Treffer in einem dieser Bereiche, öffnet JeffWrite ihn zum Bearbeiten und markiert den Treffer. Der Bereich zeigt an, wo er sich befindet, etwa „Gefunden in der Kopfzeile von Seite 3“. **Ersetzen** tauscht den Treffer aus, an dem Sie gerade stehen, im Text oder in einer Kopf-, Fußzeile oder Fußnote. **Alle ersetzen** ändert alle Treffer in all diesen Bereichen, und ein einziges **Rückgängig** stellt sie alle wieder her. Die Endnoten gehören zum Text und werden mit ihm durchsucht. Siehe [Kopf- und Fußzeilen](headers-footers).
- Text, der als **Ausgeblendet** formatiert ist, wird übersprungen, weil er nicht angezeigt wird. Siehe [Textaussehen ändern](font).

## Siehe auch

- [Textaussehen ändern](font)
- [JeffWrite mit der Tastatur bedienen](keyboard)
