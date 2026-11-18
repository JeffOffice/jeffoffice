# Formeln

<!-- keywords: Formeln-Registerkarte, Strukturen, Symbole, konvertieren, Formel, Formeln, Mathematik, Gleichung, Bruch, Quadratwurzel, Hochstellen, Tiefstellen, Exponent, Summe, Integral, griechisch, Symbol, Formel einfügen, Akzent, Matrix, Klammern, integrierte Formeln, Galerie, Formel ändern, Formel bearbeiten, pptx, odp, Bild, PDF, Bildschirmpräsentation, drucken -->

Eine **Formel** ist eine Zeile Mathematik, zum Beispiel ein Bruch oder eine Quadratwurzel. Auf einer Folie ist eine Formel ein Objekt wie ein Bild oder ein Textfeld. Sie können sie verschieben, in der Größe ändern, drehen, kopieren und löschen.

## Formel auf der Folie einfügen

1. Klicken Sie auf **Einfügen > Symbole > Formel** (oder drücken Sie **Alt+=**).
2. Geben Sie die Formel in einer Zeile ein. Unter dem Feld sehen Sie, wie sie aussehen wird. Klicken Sie auf **OK**.

Die Formel wird in der Mitte der Folie eingefügt und ausgewählt. Ihre Buchstaben sind etwa so groß wie eine Überschrift. Eine sehr breite Formel wird verkleinert, damit sie auf die Folie passt. Um von einer bekannten Formel auszugehen, klicken Sie auf **Integrierte Formeln** und wählen Sie eine aus, zum Beispiel die quadratische Formel oder die Fläche eines Kreises.

## So geben Sie die Formel ein

- **Brüche:** Geben Sie einen Schrägstrich ein. `a/b` ist a über b. `(x+1)/(x-1)` fasst jede Gruppe in Klammern als ein Teil auf.
- **Potenzen und Indizes:** `x^2` ist x hoch 2, und `x_i` ist x mit einem kleinen i darunter.
- **Wurzeln:** Geben Sie `sqrt(x+1)` für eine Quadratwurzel ein.
- **Summen, Integrale und Produkte:** Geben Sie `\sum_(i=1)^n i`, `\int_0^1 x` oder `\prod_(k=1)^n k` ein.
- **Symbole:** Geben Sie einen Backslash und einen Namen ein. `\alpha` ist α, `\pi` ist π, `\infty` ist ∞ und `\leq` ist ≤.
- **Akzente, Klammern und Matrizen:** `\hat{x}`, `\left( ... \right)` und `\pmatrix{a & b \\ c & d}`.

## In UnicodeMath oder LaTeX eingeben

Neben dem Feld steht eine Liste der Eingabeformate: **Einfache Zeile** (die oben beschriebene), **UnicodeMath** und **LaTeX**. Das sind zwei Arten, Mathematik als Nur-Text zu schreiben, die viele Menschen bereits kennen. Wenn Sie ein anderes Format wählen, wird Ihre Eingabe in diesem Format neu geschrieben.

- **UnicodeMath:** `a/b` ist ein Bruch, `x^2_i` hat eine Potenz und einen Index, `√(x)` ist eine Wurzel und `∑_(i=1)^n i` ist eine Summe. Setzen Sie Wörter in Anführungszeichen, etwa `"wenn x"`, dann bleiben sie normaler Text und werden nicht als Mathematik-Buchstaben gesetzt. `(a|b)` erzeugt Klammern mit zwei Teilen, und `\matrix(a&b@c&d)` erzeugt eine Matrix (`&` zwischen Zellen, `@` zwischen Zeilen).
- **LaTeX:** `\frac{a}{b}` ist ein Bruch, `\sqrt[3]{x}` ist eine Kubikwurzel, `\sum_{i=1}^n i` ist eine Summe, `\text{wenn x}` ist normaler Text und `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` ist eine Matrix.

Die beiden Schaltflächen **Professionell** und **Linear** ändern, wie das Feld Ihre Formel anzeigt. Professionell zeigt die Formel aufgebaut, so wie sie aussehen wird. Linear zeigt nur die Zeile, die Sie eingegeben haben. Wenn eine Formel nicht im gewählten Format geschrieben werden kann, zeigt das Feld einen Hinweis an und behält Ihre Eingabe bei.

## Ändern, verschieben und in der Größe anpassen

Doppelklicken Sie auf die Formel oder wählen Sie sie aus und klicken Sie auf **Einfügen > Symbole > Formel**, um das Feld mit ihrer Zeile zu öffnen. Ändern Sie die Formel und klicken Sie auf **OK**. Sie bleibt an ihrer Stelle und behält ihre Größe, und die Änderung lässt sich mit einem Schritt rückgängig machen. Ziehen Sie die Formel, um sie zu verschieben. Ziehen Sie an einem Ziehpunkt, um sie in der Größe zu ändern; die Formel behält dabei ihre Proportionen innerhalb ihres Rahmens.

Eine Formel, die aus einer Datei stammt, können Sie ebenfalls ändern. Solange Sie sie nicht ändern, zeigt JeffDeck das Bild, das die Datei enthält, und speichert es unverändert zurück. Doppelklicken Sie darauf (oder wählen Sie sie aus und klicken Sie auf **Formel**). Das Feld beginnt mit einer Zeile, die beschreibt, was die Formel ist. Wenn Sie diese Zeile ändern, wird das Bild zu einer Formel an derselben Stelle, und JeffDeck setzt die Mathematik anhand Ihrer Eingabe neu. Dabei gehen Sonderschriften der alten Formel verloren. Wenn Sie auf **OK** klicken, ohne die Zeile zu ändern, ändert sich nichts.

## Die Registerkarte „Formel“

Wählen Sie eine Formel aus, dann erscheint am Ende des Menübands eine Registerkarte namens **Formel**. Sie verschwindet, wenn Sie etwas anderes auswählen.

- **Tools:** **Formel bearbeiten** öffnet das Feld. **Professionell** und **Linear** ändern, wie die ausgewählte Formel auf der Folie angezeigt wird: Professionell zeigt sie aufgebaut, Linear zeigt die Zeile, in der sie eingegeben wurde. Die Auswahl bleibt nur bei der Formel, solange die Präsentation geöffnet ist: Eine gespeicherte `.pptx`- oder `.odp`-Datei zeigt die Formel immer aufgebaut, sodass sie beim Öffnen auch so erscheint. **Normaler Text** öffnet das Feld mit einem Paar Anführungszeichen, für Wörter, die keine Mathematik sind.
- **Konvertieren:** Wählen Sie das Format, in dem das Feld geöffnet wird: **Einfache Zeile**, **UnicodeMath** oder **LaTeX**. Wenn Sie das nächste Mal eine Formel bearbeiten, wird sie im gewählten Format geschrieben. Eine Formel, die keine Zeile der einfachen Art schreiben kann, etwa eine Summe mit Grenzen oben und unten, wird stattdessen in UnicodeMath geöffnet.
- **Symbole:** griechische Buchstaben, Operatoren, Relationen, Pfeile sowie Mengen und Logik. Wählen Sie ein Symbol aus, dann öffnet sich das Feld mit dem Symbol in der Zeile.
- **Strukturen:** Bruch, Index, Wurzel, Integral, großer Operator, Klammer, Funktion, Akzent, Grenzwert, Operator und Matrix. Klicken Sie auf eine Schaltfläche für die übliche Form oder auf den Pfeil daneben für weitere Formen, etwa eine Ableitung, eine Wurzel mit Grad oder eine Einheitsmatrix.

Eine Struktur wird am Ende der Zeile eingefügt, mit Beispielbuchstaben darin, und die erste Stelle wird ausgewählt, damit Sie sie überschreiben können. Im Feld stehen dieselben Strukturen in einer Reihe kleiner Schaltflächen über der Zeile. Dort wird eine Struktur an der Einfügemarke eingefügt, und wenn ein Teil der Zeile markiert ist, kommt dieser Teil an die erste Stelle der Struktur.

## Bildschirmpräsentation, Drucken und PDF

Bildschirmpräsentation, Drucken und PDF-Dateien zeichnen die Formel als Formen. Dadurch bleibt sie in jeder Größe scharf.

## Speichern

- **.pptx** speichert die Formel als Formel in der Standardform für Mathematik, mit einem Bild davon daneben für Programme, die keine Mathematik lesen können. Wenn Sie die Datei erneut öffnen, zeigt JeffDeck das Bild und behält die Mathematik bei, und Sie können die Formel wie oben beschrieben weiterhin ändern. Eine Präsentation enthält höchstens 2.000 Formeln und insgesamt 16 MB Mathematik. Darüber hinaus fügt JeffDeck keine weitere Formel hinzu.
- **.odp** enthält in der Kopie von JeffDeck keine Formeln. Jede Formel wird als Bild davon an derselben Stelle gespeichert, und die Meldung nach dem Speichern weist darauf hin.
- Ein Textfeld, das Wörter neben seiner Mathematik enthält, ist hier keine Formel. Es wird als das Bild angezeigt, das die Datei enthält, und unverändert zurückgespeichert.

## Siehe auch

- [Formen, Bilder, Tabellen und Textkunst](objects)
- [Registerkarte Einfügen](tab-insert)
