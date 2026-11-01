# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Neuberechnung, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, Zirkelbezug, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler -->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, die **Bearbeitungsleiste** die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Anschließend berechnet JeffCrunch alle Formeln neu, jeweils einige auf einmal, damit das Fenster schnell reagiert. Währenddessen zeigt die Statusleiste **Berechnung läuft…** mit einer Prozentangabe an. Danach enthalten die Zellen die von JeffCrunch berechneten Ergebnisse. Um alles neu zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen**, oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch bietet mehr als 200 Funktionen, und es werden laufend mehr. Eine Formel mit einer Funktion, die JeffCrunch noch nicht kennt, behält den Wert, der in der Datei für sie gespeichert ist. Dasselbe gilt für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von solchen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen**, oder drücken Sie **Strg+`** (auf dem Mac die Control-Taste, nicht die Befehlstaste), um in jeder Zelle anstelle des Werts die Formel zu sehen. Spalten mit Formeln werden bis zu einer bestimmten Grenze verbreitert, damit die Formeln vollständig sichtbar sind. Wenn Sie die Anzeige wieder ausschalten, erhalten die Spalten ihre ursprüngliche Breite zurück; die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, wählen Sie ihre Zelle aus, drücken **F2**, bearbeiten die Formel und drücken die **Eingabetaste**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die davon abhängigen Formeln im Hintergrund neu berechnet. Siehe [In Zellen eingeben](edit-cells).

## Formeln schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names) an, die so beginnen, jeweils mit einer kurzen Beschreibung. Drücken Sie die **Tabulatortaste**, um den markierten Eintrag zu übernehmen. Mit den Pfeiltasten **Nach-unten** und **Nach-oben** markieren Sie einen anderen Eintrag und übernehmen ihn dann mit der **Tabulatortaste** oder der **Eingabetaste**. Mit **Esc** schließen Sie die Liste. Sobald Sie sich innerhalb der Klammern einer Funktion befinden, werden ihre Argumente unter der Zelle angezeigt; das Argument, das Sie gerade eingeben, ist fett dargestellt. Die Liste und die Argumente sind dieselben, die auch **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt oder durch Semikolons, wenn in Ihrer Sprache das Komma als Dezimaltrennzeichen dient.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, welches Ergebnis eine Formel für viele verschiedene Werte von einer oder zwei Zellen liefert. Wenn Sie eine Arbeitsmappe mit einer Datentabelle öffnen, berechnet JeffCrunch deren Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle in der linken oberen Ecke der Tabelle; die horizontal angeordneten Werte werden in die Zeileneingabezelle eingesetzt, die vertikal angeordneten in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte entweder untereinander in der Spalte links (und die Formeln in der Zeile darüber) oder nebeneinander in der Zeile darüber (und die Formeln in der Spalte links). Jede Zelle der Tabelle ist das Ergebnis der Formel – und aller Formeln, die von der Eingabezelle abhängen –, wenn die Eingabe auf den jeweiligen Wert gesetzt ist; der Rest der Arbeitsmappe bleibt unverändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und Formeln, die Zellen einer Tabelle lesen, werden anschließend berechnet. Neue Datentabellen können noch nicht erstellt werden. Eine Tabelle, die nicht berechnet werden kann (weil eine von der Eingabe abhängige Formel eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, wie ZUFALLSZAHL oder JETZT), behält die in der Datei gespeicherten Werte.

## Manuell berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet die Formeln, während Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** arbeitet ebenso, lässt die Zellen von Datentabellen aber unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es veranlassen. Nach einer Änderung zeigt die Statusleiste **Veraltet** an. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles, was sonst noch aussteht, zu berechnen. Eine Arbeitsmappe im Modus „Manuell“ wird mit den in der Datei gespeicherten Ergebnissen geöffnet.

Die Einstellung wird mit der Arbeitsmappe gespeichert und lässt sich in einem Schritt rückgängig machen.

## Formeln, die sich selbst lesen

Eine Formel, die direkt oder über andere Zellen ihr eigenes Ergebnis liest, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die betroffenen Zellen und umrandet sie auf dem Blatt; sie behalten ihre bisherigen Werte. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste anzuzeigen, und doppelklicken Sie auf eine Zelle in der Liste, um zu ihr zu wechseln.

Manche Modelle benötigen einen solchen Zirkelbezug absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Befehl **Iterative Berechnung aktivieren**. Die Formeln werden dann wiederholt berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die zulässige Anzahl an Iterationen erreicht ist.

## Anzeigen, was eine Formel liest

Wählen Sie eine Zelle aus, und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen weiteren Schritt zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen; jeder Klick geht einen Schritt weiter. Ein kleines Seitensymbol steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet; die Statusleiste weist darauf hin, wenn es mehr gab. **Pfeile entfernen** entfernt die Pfeile, ebenso jede Änderung an der Arbeitsmappe.
