# Gleichungen
<!-- keywords: Gleichungs-Registerkarte, Strukturen, Symbole, konvertieren, Gleichung, Gleichungen, Mathematik, Formel, Bruch, Quadratwurzel, Hochstellung, Tiefstellung, Exponent, Summe, Integral, griechisch, Symbol, Gleichung einfügen, Akzent, Matrix, Klammern, integrierte Gleichungen, Galerie, Gleichung ändern, Gleichung bearbeiten, ODS, Bild, PDF -->

Eine **Gleichung** ist eine mathematische Zeile, zum Beispiel ein Bruch oder eine Quadratwurzel. In JeffCrunch ist eine Gleichung ein Objekt über den Zellen, ähnlich wie ein Bild oder ein Textfeld. Sie ist keine Zelle: Sie ändert keine Zahl im Blatt.

## Eine Gleichung in das Blatt einfügen

1. Klicken Sie auf **Einfügen > Symbole > Gleichung**.
2. Geben Sie die Gleichung in einer Zeile ein. Unter dem Feld sehen Sie, wie sie aussehen wird. Klicken Sie auf **OK**.

Die Gleichung wird an der Zelle eingefügt, in der Sie sich gerade befinden, und ist markiert. Um von einer bekannten Gleichung auszugehen, klicken Sie auf **Integrierte Gleichungen** und wählen Sie eine aus, zum Beispiel die quadratische Formel oder die Fläche eines Kreises.

## So geben Sie sie ein

- **Brüche:** Geben Sie einen Schrägstrich ein. `a/b` ist a über b. `(x+1)/(x-1)` fasst jede Gruppe in Klammern als einen Teil auf.
- **Potenzen und Indizes:** `x^2` ist x hoch 2, und `x_i` ist x mit einem kleinen i darunter.
- **Wurzeln:** Geben Sie `sqrt(x+1)` für eine Quadratwurzel ein.
- **Summen, Integrale und Produkte:** Geben Sie `\sum_(i=1)^n i`, `\int_0^1 x` oder `\prod_(k=1)^n k` ein.
- **Symbole:** Geben Sie einen Backslash und einen Namen ein. `\alpha` ist α, `\pi` ist π, `\infty` ist ∞ und `\leq` ist ≤.
- **Akzente, Klammern und Matrizen:** `\hat{x}`, `\left( ... \right)` und `\pmatrix{a & b \\ c & d}`.

## In UnicodeMath oder LaTeX eingeben

Neben dem Feld steht eine Liste mit Eingabeformaten: **Einfache Zeile** (die oben beschriebene), **UnicodeMath** und **LaTeX**. Das sind zwei Arten, Mathematik als reinen Text zu schreiben, die viele Menschen schon kennen. Wenn Sie ein anderes Format wählen, wird das Eingegebene in diesem Format neu geschrieben.

- **UnicodeMath:** `a/b` ist ein Bruch, `x^2_i` hat eine Hochstellung und eine Tiefstellung, `√(x)` ist eine Wurzel und `∑_(i=1)^n i` ist eine Summe. Setzen Sie Wörter in Anführungszeichen, etwa `"wenn x"`, dann bleiben sie normaler Text und werden nicht als Mathematikzeichen dargestellt. `(a|b)` erzeugt eine Klammer mit zwei Teilen, und `\matrix(a&b@c&d)` erzeugt eine Matrix (`&` zwischen Zellen, `@` zwischen Zeilen).
- **LaTeX:** `\frac{a}{b}` ist ein Bruch, `\sqrt[3]{x}` ist eine dritte Wurzel, `\sum_{i=1}^n i` ist eine Summe, `\text{wenn x}` ist normaler Text und `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` ist eine Matrix.

Mit den Schaltflächen **Professionell** und **Linear** legen Sie fest, wie das Feld Ihre Gleichung anzeigt. Professionell zeigt sie aufgebaut, so wie sie später aussehen wird. Linear zeigt nur die eingegebene Zeile. Lässt sich eine Gleichung im gewählten Format nicht schreiben, meldet das Feld das und behält Ihre Eingabe bei.

## Die Registerkarte „Gleichung“

Wenn Sie eine Gleichung auswählen, erscheint am Ende des Menübands die Registerkarte **Gleichung**. Sie verschwindet, wenn Sie etwas anderes auswählen.

- **Tools:** **Gleichung bearbeiten** öffnet das Feld. **Professionell** und **Linear** legen fest, wie das Feld beginnt: aufgebaut oder nur als Zeile. **Normaler Text** öffnet das Feld mit einem Paar Anführungszeichen, für Wörter, die keine Mathematik sind.
- **Konvertieren:** Wählen Sie das Format, in dem das Feld geöffnet wird: **Einfache Zeile**, **UnicodeMath** oder **LaTeX**. Beim nächsten Bearbeiten einer Gleichung wird sie in dem gewählten Format geschrieben. Eine Gleichung, die keine einfache Zeile schreiben kann, etwa eine Summe mit Grenzen oben und unten, wird stattdessen in UnicodeMath geöffnet.
- **Symbole:** griechische Buchstaben, Operatoren, Relationen, Pfeile sowie Mengen und Logik. Wählen Sie ein Symbol aus, dann öffnet sich das Feld mit dem Symbol in der Zeile.
- **Strukturen:** Bruch, Hoch-/Tiefstellung, Wurzel, Integral, großer Operator, Klammer, Funktion, Akzent, Grenzwert, Operator und Matrix. Klicken Sie auf eine Schaltfläche für ihre übliche Form oder auf den Pfeil daneben für weitere Formen, etwa eine Ableitung, eine Wurzel mit Grad oder eine Einheitsmatrix.

Eine Struktur wird am Ende der Zeile eingefügt, mit Beispielzeichen darin, und die erste Stelle ist markiert, sodass Sie darüber schreiben können. Im Feld gibt es dieselben Strukturen in einer Reihe kleiner Schaltflächen über der Zeile. Dort wird eine Struktur an der Einfügemarke eingefügt. Ist ein Teil der Zeile markiert, kommt dieser Teil in die erste Stelle der Struktur.

## Ändern, verschieben und in der Größe anpassen

Doppelklicken Sie auf die Gleichung oder markieren Sie sie und klicken Sie auf **Einfügen > Symbole > Gleichung**, um das Feld mit ihrer Zeile zu öffnen. Ändern Sie die Gleichung und klicken Sie auf **OK**. Das Rückgängigmachen dauert einen Schritt. Ziehen Sie die Gleichung, um sie zu verschieben, und ziehen Sie an einem Griff, um sie in der Größe zu ändern; die Gleichung behält dabei ihre Proportionen innerhalb des Feldes.

## Speichern

- **.xlsx** speichert die Gleichung als Gleichung, in der Standardform für Mathematik, mit einem Bild daneben für Programme, die keine Mathematik lesen können. Wenn Sie die Datei erneut öffnen, wird die Gleichung angezeigt. Eine in einem anderen Programm erstellte Gleichung wird genauso angezeigt; wenn ihre Mathematik nicht gelesen werden kann, sehen Sie ihr Bild.
- **PDF und Drucken** zeichnen die Gleichung als Formen, sodass sie in jeder Größe scharf bleibt.
- **.ods-Kopien** enthalten keine Gleichungen. JeffCrunch speichert jede Gleichung als Bild an derselben Stelle und nennt die Gleichungen, wenn es Ihnen mitteilt, was die Kopie weglässt.
