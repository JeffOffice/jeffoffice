# Gleichungen
<!-- keywords: Gleichungen einfügen, Formeln, Gleichungstab, Strukturen, Symbole, konvertieren, Gleichung, Gleichungen, Mathematik, Formel, Bruch, Quadratwurzel, Hochstellen, Tiefstellen, Exponent, Summe, Integral, griechisch, Symbol, Gleichung einfügen, Alt+=, Akzent, Dach, Vektor, Balken, Matrix, Klammern, integrierte Gleichungen, Galerie, Gleichung ändern, Gleichung bearbeiten -->

Eine **Gleichung** ist eine mathematische Zeile, zum Beispiel ein Bruch oder eine Quadratwurzel. Dieses Thema zeigt, wie Sie eine Gleichung eingeben und in ein Dokument einfügen.

## Gleichung einfügen

1. Setzen Sie die Einfügemarke an die Stelle, an der die Gleichung stehen soll.
2. Klicken Sie auf **Einfügen > Gleichung**, oder drücken Sie **Alt+=**.
3. Geben Sie die Gleichung in einer Zeile ein. Unter dem Feld sehen Sie die Gleichung so gezeichnet, wie sie aussehen wird, und die Textzeile, die die Seite anzeigt. Um von einer bekannten Gleichung auszugehen, wählen Sie eine aus der Liste **Integrierte Gleichungen** im Feld. Klicken Sie auf **OK**.

Die Gleichung wird im Dokument in der Standardform für Mathematik gespeichert, damit andere Programme sie öffnen können.

## So geben Sie Gleichungen ein

- **Brüche:** Geben Sie einen Schrägstrich ein. `a/b` ergibt a über b. `(x+1)/(x-1)` fasst jede Gruppe in Klammern als einen Teil auf. `\frac{1}{2}` funktioniert ebenfalls.
- **Potenzen:** Geben Sie ein Dachzeichen ein. `x^2` ergibt x hoch 2. `e^(i*pi)` fasst die Gruppe in Klammern zusammen.
- **Indizes:** Geben Sie einen Unterstrich ein. `x_i` ergibt x mit einem kleinen i darunter. Sie können beides verbinden, etwa `x_i^2`.
- **Wurzeln:** Geben Sie `sqrt(x+1)` für eine Quadratwurzel ein. Geben Sie `sqrt[3]{x}` für eine Kubikwurzel ein.
- **Summen, Integrale und Produkte:** Geben Sie `\sum_(i=1)^n i`, `\int_0^1 x` oder `\prod_(k=1)^n k` ein. Der Teil nach dem Unterstrich steht darunter (bzw. unten am Integralzeichen), der Teil nach dem Dachzeichen steht darüber.
- **Symbole:** Geben Sie einen umgekehrten Schrägstrich und einen Namen ein. `\alpha` ergibt α, `\pi` ergibt π, `\infty` ergibt ∞, `\leq` ergibt ≤, `\neq` ergibt ≠, `\times` ergibt ×, `\pm` ergibt ± und `\to` ergibt →. Etwa 70 Namen funktionieren.
- **Akzente:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` und `\vec{v}` setzen eine Markierung über den Buchstaben. `\overline{ab}` zieht eine Linie über die Gruppe, und `\underline{ab}` zieht eine Linie darunter.
- **Klammern, die mitwachsen:** Geben Sie `\left( ... \right)` ein. Die Klammern wachsen auf die Höhe dessen, was zwischen ihnen steht. Verwenden Sie nach `\left` und `\right` eine beliebige Klammer oder einen Punkt, um eine Seite wegzulassen, etwa in `\left[ x \right.`.
- **Matrizen:** Eine **Matrix** ist ein Raster aus Zahlen. Geben Sie `\matrix{a & b \\ c & d}` ein. Setzen Sie ein `&` zwischen die Zellen einer Zeile und zwei umgekehrte Schrägstriche zwischen die Zeilen. Verwenden Sie `\pmatrix` für runde Klammern um das Raster, `\bmatrix` für eckige und `\vmatrix` für senkrechte Striche.
- **Funktionen:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` und `max` behalten ihre Namen ganz.

Auf der Seite zeigt JeffWrite eine Gleichung als eine Textzeile. Die Gleichung in der Datei behält ihre Brüche und Wurzeln, damit andere Programme sie richtig darstellen.

## In UnicodeMath oder LaTeX eingeben

Neben dem Feld steht eine Liste mit Eingabeformaten: **Einfache Zeile** (das oben beschriebene), **UnicodeMath** und **LaTeX**. Das sind zwei Arten, Mathematik als Klartext zu schreiben, die viele Menschen bereits kennen. Wenn Sie ein anderes Format wählen, wird das Eingegebene in diesem Format neu geschrieben.

- **UnicodeMath:** `a/b` ist ein Bruch, `x^2_i` hat eine Potenz und einen Index, `√(x)` ist eine Wurzel und `∑_(i=1)^n i` ist eine Summe. Setzen Sie Wörter in Anführungszeichen, etwa `"wenn x"`, dann bleiben sie normaler Text statt Mathematikzeichen. `(a|b)` erzeugt Klammern mit zwei Teilen, und `\matrix(a&b@c&d)` erzeugt eine Matrix (`&` zwischen den Zellen, `@` zwischen den Zeilen).
- **LaTeX:** `\frac{a}{b}` ist ein Bruch, `\sqrt[3]{x}` ist eine Kubikwurzel, `\sum_{i=1}^n i` ist eine Summe, `\text{wenn x}` ist normaler Text und `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` ist eine Matrix.

Die beiden Schaltflächen **Professionell** und **Linear** ändern die Darstellung Ihrer Gleichung im Feld. Professionell zeigt sie aufgebaut, so wie sie aussehen wird. Linear zeigt nur die eingegebene Zeile. Wenn eine Gleichung im gewählten Format nicht geschrieben werden kann, meldet das Feld das und behält Ihre Eingabe.

## Der Reiter Gleichung

Wählen Sie eine Gleichung aus (klicken Sie darauf, oder bewegen Sie die Einfügemarke mit gedrückter Umschalttaste darüber). Am Ende des Menübands erscheint der Reiter **Gleichung**. Er verschwindet, sobald Sie etwas anderes auswählen.

- **Tools:** **Gleichung bearbeiten** öffnet das Feld. **Professionell** und **Linear** legen fest, wie das Feld beginnt: aufgebaut oder nur als Zeile. Im Dokument selbst wird eine Gleichung immer als Textzeile angezeigt. **Normaler Text** öffnet das Feld mit einem Paar Anführungszeichen, für Wörter, die keine Mathematik sind.
- **Konvertieren:** Wählen Sie das Format, in dem sich das Feld öffnet: **Einfache Zeile**, **UnicodeMath** oder **LaTeX**. Wenn Sie das nächste Mal eine Gleichung bearbeiten, wird sie in dem gewählten Format geschrieben. Eine Gleichung, die sich in einer einfachen Zeile nicht schreiben lässt, etwa eine Summe mit Grenzen oben und unten, öffnet sich stattdessen in UnicodeMath.
- **Symbole:** griechische Buchstaben, Operatoren, Relationen, Pfeile sowie Mengen und Logik. Wählen Sie ein Symbol aus, und das Feld öffnet sich mit dem Symbol in der Zeile.
- **Strukturen:** Bruch, Index, Wurzel, Integral, großer Operator, Klammer, Funktion, Akzent, Grenzwert, Operator und Matrix. Klicken Sie auf eine Schaltfläche für die übliche Form oder auf den Pfeil daneben für weitere Formen, etwa eine Ableitung, eine Wurzel mit Grad oder eine Einheitsmatrix.

Eine Struktur wird am Ende der Zeile eingefügt, mit Beispielbuchstaben darin, und die erste Stelle ist markiert, sodass Sie darüber schreiben können. Im Feld gibt es dieselben Strukturen als Reihe kleiner Schaltflächen über der Zeile. Dort wird eine Struktur an der Einfügemarke eingefügt, und wenn ein Teil der Zeile markiert ist, kommt dieser Teil an die erste Stelle der Struktur.

## Integrierte Gleichungen

Klicken Sie auf **Einfügen > Integrierte Gleichungen**, um eine Liste bekannter Gleichungen zu sehen, etwa die quadratische Formel und die Fläche eines Kreises. Klicken Sie auf eine davon, um sie an der Einfügemarke einzufügen.

## Gleichung ändern

Setzen Sie die Einfügemarke neben eine Gleichung, oder wählen Sie sie aus, und klicken Sie auf **Einfügen > Gleichung**. Das Feld öffnet sich mit der Gleichung in einer Zeile. Ändern Sie sie und klicken Sie auf **OK**. Rückgängig machen können Sie dies mit einem Schritt.

## Gleichungen in anderen Dokumenten

JeffWrite zeigt die Gleichungen an, die ein Dokument enthält, und bewahrt sie unverändert auf. Eine Gleichung, die ein anderes Programm erstellt hat, öffnet sich in UnicodeMath, oder in LaTeX, wenn UnicodeMath sie nicht schreiben kann, damit Sie sie ändern können. Wenn keines von beiden alle Teile schreiben kann, etwa Gruppierungszeichen, beginnt das Feld mit dem Text der Gleichung. Klicken Sie auf **Abbrechen**, um die Gleichung unverändert zu lassen, oder geben Sie eine neue ein, um sie zu ersetzen.
