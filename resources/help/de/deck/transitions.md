# Übergänge zwischen Folien
<!-- keywords: Übergang, Übergänge, Folienübergang, Morphen, Morph, Verblassen, Überblenden, Schieben, Wischen, Teilen, Enthüllen, Bedecken, Aufdecken, Zoom, Schnitt, Effektoptionen, Objekte, Wörter, Zeichen, zuordnen, Paar, Name, Richtung, Dauer, Folienwechsel, bei Mausklick, nach, automatisch, Anzeigedauer, für alle übernehmen, Vorschau, Stern -->

Ein **Übergang** ist der Effekt, der abgespielt wird, wenn die Bildschirmpräsentation von einer Folie zur nächsten wechselt. Ein Übergang gehört zu der Folie, zu der gewechselt wird. Die Bildschirmpräsentation spielt ihn ab, wenn sie vorwärts zu dieser Folie wechselt.

## Übergang hinzufügen

1. Klicken Sie im Folienbereich auf die Folie.
2. Klicken Sie auf die Registerkarte **Übergänge**. Klicken Sie im Katalog **Übergang zu dieser Folie** auf einen Effekt: **Keine**, **Morphen**, **Verblassen**, **Schieben**, **Wischen**, **Teilen**, **Enthüllen**, **Bedecken**, **Aufdecken**, **Zoom** oder **Schnitt**. Die Miniatur der Folie im Folienbereich erhält einen kleinen Stern, der anzeigt, dass sie einen Übergang hat.
3. Klicken Sie auf **Übergänge > Vorschau > Vorschau**, um den Übergang zur Folie über dem Folienbereich abzuspielen. Drücken Sie Esc, um ihn anzuhalten.

Wählen Sie **Keine**, um einen Übergang zu entfernen.

## Ablauf festlegen

Klicken Sie auf **Effektoptionen**, um die Optionen des Effekts anzuzeigen:

- **Schieben**, **Wischen**, **Enthüllen**, **Bedecken** und **Aufdecken** haben eine Richtung: **Von rechts**, **Von links**, **Von unten** oder **Von oben**.
- **Teilen** und **Zoom** gehen **Heraus** oder **Hinein**.
- **Morphen** bewegt **Objekte**, **Wörter** oder **Zeichen**. Siehe unten.
- **Verblassen** und **Schnitt** haben keine Optionen. Ein Schnitt verläuft über Schwarz.

## Morphen

**Morphen** bewegt, was zwei Folien gemeinsam haben. Ein **Objekt** ist ein einzelnes Element auf einer Folie: eine Form, ein Bild, ein Textfeld, eine Linie oder eine Tabelle. Die Präsentation wechselt von einer Folie zur nächsten. Jedes Objekt, das auf beiden Folien vorhanden ist, gleitet von seiner Position auf der ersten Folie zu seiner Position auf der zweiten Folie. Dabei wird es größer oder kleiner, dreht sich und ändert seine Farbe. Ein Objekt, das nur auf der ersten Folie vorhanden ist, wird ausgeblendet. Ein Objekt, das nur auf der zweiten Folie vorhanden ist, wird eingeblendet.

So erstellen Sie einen Morph-Übergang:

1. Klicken Sie im Folienbereich mit der rechten Maustaste auf eine Folie, und wählen Sie **Folie duplizieren**. Die Kopie enthält dieselben Objekte.
2. Verschieben Sie auf der Kopie ein Objekt, vergrößern oder drehen Sie es, oder ändern Sie seine Farbe. Sie können auch seinen Text ändern oder Objekte hinzufügen und löschen.
3. Klicken Sie auf die Kopie und dann auf **Übergänge > Übergang zu dieser Folie > Morphen**. Ein Morph-Übergang dauert 2 Sekunden. Sie können das mit **Dauer** ändern.
4. Klicken Sie auf **Vorschau**, um ihn anzusehen, oder starten Sie die Bildschirmpräsentation.

### Welche Objekte zusammengehören

JeffDeck ordnet einem Objekt auf der ersten Folie ein Objekt auf der zweiten Folie zu. Dabei werden die folgenden Möglichkeiten in dieser Reihenfolge geprüft. Jedes Objekt gehört höchstens zu einem Paar.

1. **Ein Name, der mit !! beginnt.** Zwei Objekte mit demselben Namen, der mit zwei Ausrufezeichen beginnt, bilden immer ein Paar. Das können zwei beliebige Objekte sein, sogar ein Kreis und ein Stern.
2. **Derselbe Name.** Zwei Objekte derselben Art mit demselben Namen bilden ein Paar. Ein Textfeld und eine Form sind unterschiedliche Arten.
3. **Derselbe Text.** Zwei Textfelder mit denselben Wörtern bilden ein Paar, unabhängig von ihren Namen. Das gilt auch für zwei Formen mit denselben Wörtern.
4. **Dasselbe Bild.** Zwei Bilder mit derselben Bilddatei bilden ein Paar, auch wenn sie einen anderen Namen oder eine andere Größe haben.
5. **Dieselbe Formart.** Zwei Kreise bilden ein Paar. Das gilt auch für zwei Linien oder zwei Tabellen.

Wenn mehrere Objekte ein Paar bilden könnten, werden zuerst die Objekte zugeordnet, die am nächsten beieinanderliegen.

Die Namen der Objekte stammen aus der Präsentationsdatei. Eine in einem anderen Programm erstellte Präsentation behält die Namen ihrer Objekte, und JeffDeck verwendet sie.

### Wie sich ein Paar bewegt

- Zwei Objekte mit derselben Form verhalten sich wie ein einzelnes Objekt, das sich bewegt. Position, Größe, Drehung, Füllfarbe, Linienfarbe und Transparenz ändern sich schrittweise. Auch der Zuschnitt eines Bilds ändert sich.
- Zwei Objekte mit unterschiedlicher Form, etwa ein Kreis und ein Quadrat, bewegen sich zu denselben Positionen, während das eine in das andere überblendet.
- Wenn der Text eines Paars nicht übereinstimmt, wird der alte Text in den neuen überblendet.
- Objekte, die durch eine Animation ausgeblendet werden, werden nicht berücksichtigt. Ein Objekt, das durch eine Animation verschoben wurde, wird an seiner Endposition übernommen.

### Was sich bewegt: Objekte, Wörter oder Zeichen

Klicken Sie auf **Effektoptionen**, um die Option auszuwählen:

- **Objekte.** Der Text eines Paars bleibt in seinem Objekt. Der alte Text wird in den neuen überblendet.
- **Wörter.** Ein Wort, das in beiden Texten vorkommt, bewegt sich von seiner alten zu seiner neuen Position. Die übrigen Wörter werden aus- und eingeblendet. Ein Satzzeichen wie ein Komma zählt als eigenes Wort.
- **Zeichen.** Wie bei Wörtern, aber Buchstabe für Buchstabe bzw. Ziffer für Ziffer.

**Wörter** und **Zeichen** funktionieren bei Textfeldern und Formen. Ein Text mit mehr als 16.000 Zeichen, vertikal verlaufender Text und Text in einer gespiegelten Form werden wie **Objekte** abgespielt. Das gilt auch für ein Paar ohne gemeinsames Wort oder Zeichen.

Morphen gibt es auch als Übergang in anderen Präsentationsprogrammen (in den Versionen ab 2019). JeffDeck liest und speichert ihn auf dieselbe Weise. Ältere Programme spielen einen Morph-Übergang als Verblassen ab.

## Anzeigedauer festlegen

Die Gruppe **Anzeigedauer** legt fest, wie lange der Übergang dauert und wie die Präsentation die Folie verlässt.

- **Dauer** gibt an, wie viele Sekunden der Effekt dauert. Ein neuer Übergang dauert 0,7 Sekunden. Sie können Werte von 0,01 bis 59 eingeben.
- **Bei Mausklick** ist standardmäßig aktiviert. Die Präsentation wechselt von dieser Folie weiter, wenn der Referent klickt. Deaktivieren Sie die Option, um stattdessen mit einer Taste wie der Leertaste oder Nach-rechts weiterzuwechseln. Die Animationen der Folie werden weiterhin per Klick abgespielt.
- **Nach** lässt die Präsentation nach der eingegebenen Anzahl Sekunden automatisch weiterwechseln. Sie können **Nach** zusammen mit **Bei Mausklick** verwenden: Die Präsentation wechselt dann bei dem Ereignis weiter, das zuerst eintritt.

**Auf alle anwenden** überträgt den Übergang und die Anzeigedauer der aktuellen Folie auf alle Folien.

Jede dieser Änderungen ist ein Schritt, den Sie mit Strg+Z rückgängig machen können.

## Übergänge aus anderen Dateien

Eine in einem anderen Programm erstellte Präsentation kann einen Übergang enthalten, für den JeffDeck keinen Effekt hat. Er wird als Verblassen abgespielt und unverändert gespeichert, sodass er im ursprünglichen Programm weiterhin funktioniert.

## Verwandte Themen

- [Registerkarte „Übergänge“](tab-transitions)
- [Animationen](animations)
- [Bildschirmpräsentation vorführen und Referentenansicht verwenden](show)
