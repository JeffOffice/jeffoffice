# Gleichung hinzufügen
<!-- keywords: Gleichungen-Registerkarte, Strukturen, Symbole, konvertieren, Gleichung, Gleichungen, Mathematik, Formel, Bruch, Quadratwurzel, Hochstellen, Tiefstellen, Exponent, Summe, Integral, griechisch, Symbol, Gleichung einfügen, Alt+=, Akzent, Matrix, Klammern, integrierte Gleichungen, Galerie, Gleichung ändern, Gleichung bearbeiten, Vektor, PDF, JPUB, PUB-Datei -->

Eine **Gleichung** ist eine Zeile Mathematik, etwa ein Bruch oder eine Quadratwurzel. In JeffPub ist eine Gleichung ein Objekt auf der Seite, ähnlich wie ein Bild: Sie können sie verschieben, in der Größe ändern, drehen und einfärben.

## Gleichung auf die Seite setzen

1. Klicken Sie auf **Einfügen > Text > Gleichung**, oder drücken Sie **Alt+=**.
2. Geben Sie die Gleichung in einer Zeile ein. Unter dem Feld sehen Sie, wie sie aussehen wird. Klicken Sie auf **OK**.

Die Gleichung erscheint in der Mitte der Seite und ist markiert. Um von einer bekannten Gleichung auszugehen, klicken Sie auf den Pfeil neben **Gleichung** und wählen eine integrierte Gleichung, etwa die quadratische Formel oder die Fläche eines Kreises.

## So geben Sie sie ein

- **Brüche:** Geben Sie einen Schrägstrich ein. `a/b` ist a über b. `(x+1)/(x-1)` fasst jede Gruppe in Klammern als einen Teil auf.
- **Potenzen und tiefgestellte Zeichen:** `x^2` ist x hoch 2, und `x_i` ist x mit einem kleinen i darunter. Sie können beides kombinieren, wie in `x_i^2`.
- **Wurzeln:** Geben Sie `sqrt(x+1)` für eine Quadratwurzel oder `sqrt[3]{x}` für eine dritte Wurzel ein.
- **Summen, Integrale und Produkte:** Geben Sie `\sum_(i=1)^n i`, `\int_0^1 x` oder `\prod_(k=1)^n k` ein. Der Teil nach dem Unterstrich steht unten, der Teil nach dem Dach­zeichen steht oben.
- **Symbole:** Geben Sie einen umgekehrten Schrägstrich und einen Namen ein. `\alpha` ist α, `\pi` ist π, `\infty` ist ∞, `\leq` ist ≤ und `\times` ist ×. Etwa 70 Namen sind möglich.
- **Akzente:** `\hat{x}`, `\vec{v}` und `\bar{x}` setzen ein Zeichen über den Buchstaben.
- **Klammern, die mitwachsen:** Geben Sie `\left( ... \right)` ein, dann wachsen die Klammern auf die Höhe des Inhalts zwischen ihnen.
- **Matrizen:** Eine **Matrix** ist ein Raster aus Zahlen. Geben Sie `\pmatrix{a & b \\ c & d}` ein. Setzen Sie ein `&` zwischen die Zellen einer Zeile und zwei umgekehrte Schrägstriche zwischen die Zeilen.

## In UnicodeMath oder LaTeX eingeben

Neben dem Feld steht eine Liste von Eingabeformaten: **Einfache Zeile** (die oben beschriebene), **UnicodeMath** und **LaTeX**. Das sind zwei Arten, Mathematik als reinen Text zu schreiben, die vielen Menschen schon bekannt sind. Wenn Sie ein anderes Format wählen, wird das Eingegebene in diesem Format neu geschrieben.

- **UnicodeMath:** `a/b` ist ein Bruch, `x^2_i` hat eine Hoch- und eine Tiefstellung, `√(x)` ist eine Wurzel und `∑_(i=1)^n i` ist eine Summe. Setzen Sie Wörter in Anführungszeichen, etwa `"wenn x"`, dann bleiben sie normaler Text statt Mathematikzeichen. `(a|b)` erzeugt Klammern mit zwei Teilen, und `\matrix(a&b@c&d)` erzeugt eine Matrix (`&` zwischen den Zellen, `@` zwischen den Zeilen).
- **LaTeX:** `\frac{a}{b}` ist ein Bruch, `\sqrt[3]{x}` ist eine dritte Wurzel, `\sum_{i=1}^n i` ist eine Summe, `\text{wenn x}` ist normaler Text, und `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` ist eine Matrix.

Die beiden Schaltflächen **Professionell** und **Linear** ändern, wie das Feld Ihre Gleichung zeigt. Professionell stellt sie aufgebaut dar, so wie sie später aussehen wird. Linear zeigt nur die Zeile, die Sie eingegeben haben. Wenn eine Gleichung im gewählten Format nicht geschrieben werden kann, meldet das Feld dies und behält das Eingegebene bei.

## Gleichung ändern

Markieren Sie die Gleichung und drücken Sie die Eingabetaste, oder doppelklicken Sie darauf, oder klicken Sie auf **Einfügen > Text > Gleichung**. Das Feld öffnet sich mit der Gleichung in einer Zeile. Ändern Sie sie und klicken Sie auf **OK**. Rückgängig machen können Sie das mit einem Schritt.

Eine Gleichung, die aus einer Datei übernommen wurde, öffnet sich in UnicodeMath, oder in LaTeX, wenn UnicodeMath sie nicht schreiben kann. Wenn keines von beiden möglich ist, beginnt das Feld mit dem Text der Gleichung. Klicken Sie ohne Änderung auf **OK** oder auf **Abbrechen**, um die Gleichung unverändert zu lassen, oder geben Sie eine neue ein, um sie zu ersetzen.

## Die Registerkarte „Gleichung“

Wenn Sie eine Gleichung markieren, erscheint am Ende des Menübands eine Registerkarte mit dem Namen **Gleichung**. Sie verschwindet, wenn Sie etwas anderes markieren.

- **Tools:** **Gleichung bearbeiten** öffnet das Feld. **Professionell** und **Linear** ändern, wie die markierte Gleichung auf der Seite dargestellt wird: Professionell stellt sie aufgebaut dar, Linear zeigt die Zeile, in der sie eingegeben wurde. Die Auswahl wird zusammen mit der Gleichung in der Datei `.jpub` gespeichert, sodass sie beim erneuten Öffnen der Datei gleich aussieht. Eine `.pub`-Datei erhält das Bild der Gleichung, so wie sie auf der Seite angezeigt wird. **Normaler Text** öffnet das Feld mit einem Paar Anführungszeichen, für Wörter, die keine Mathematik sind.
- **Konvertieren:** Wählen Sie das Format, in dem das Feld öffnet: **Einfache Zeile**, **UnicodeMath** oder **LaTeX**. Beim nächsten Bearbeiten einer Gleichung wird sie in dem gewählten Format geschrieben. Eine Gleichung, die keine Zeile der einfachen Art schreiben kann, etwa eine Summe mit Grenzen oben und unten, öffnet sich stattdessen in UnicodeMath.
- **Symbole:** griechische Buchstaben, Operatoren, Relationen, Pfeile sowie Mengen und Logik. Wählen Sie ein Symbol, dann öffnet sich das Feld mit dem Symbol in der Zeile.
- **Strukturen:** Bruch, Index, Wurzel, Integral, großer Operator, Klammer, Funktion, Akzent, Grenzwert, Operator und Matrix. Klicken Sie auf eine Schaltfläche für die übliche Form oder auf den Pfeil daneben für weitere Formen, etwa eine Ableitung, eine Wurzel mit Grad oder eine Einheitsmatrix.

Eine Struktur wird am Ende der Zeile mit Beispielbuchstaben eingefügt, und die erste Stelle ist markiert, sodass Sie direkt darüber schreiben können. Im Feld stehen dieselben Strukturen als Reihe kleiner Schaltflächen über der Zeile. Dort wird eine Struktur an der Einfügemarke eingefügt, und wenn ein Teil der Zeile markiert ist, kommt dieser Teil an die erste Stelle der Struktur.

## Verschieben, Größe und Farbe

Ziehen Sie die Gleichung, um sie zu verschieben. Ziehen Sie an einer Ecke, um sie zu vergrößern oder zu verkleinern; die Gleichung behält dabei ihre Proportionen. Mit **Formfüllung** ändern Sie die Farbe der Buchstaben.

## Speichern

- **.jpub** speichert die Gleichung, sodass Sie sie später wieder bearbeiten können.
- **PDF und Drucken** zeichnen die Gleichung als Formen, sodass sie in jeder Größe scharf bleibt.
- **.pub** kennt keine Gleichungen. JeffPub speichert jede Gleichung als Bild an derselben Stelle und in derselben Größe und weist beim Speichern darauf hin.
- **EPUB** und **HTML** übernehmen die Gleichung als die eingegebene Zeile.
