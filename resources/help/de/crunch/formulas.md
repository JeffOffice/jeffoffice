# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Neuberechnung, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, Zirkelbezug, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler -->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, die **Bearbeitungsleiste** die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den letzten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Anschließend berechnet JeffCrunch alle Formeln neu, jeweils einige auf einmal, damit das Fenster schnell reagiert. Währenddessen zeigt die Statusleiste **Berechnung läuft…** mit einer Prozentangabe an. Danach enthalten die Zellen die von JeffCrunch berechneten Ergebnisse. Um alles neu zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen**, oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch bietet mehr als 200 Funktionen, und es kommen weitere hinzu. Eine Formel mit einer Funktion, die JeffCrunch noch nicht kennt, behält den in der Datei gespeicherten Wert. Das gilt auch für Formeln, die auf eine andere Arbeitsmappe verweisen. Zellen, die von solchen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen**, oder drücken Sie **Strg+`** (auf einem Mac die Taste Control, nicht die Befehlstaste), um in jeder Zelle statt des Werts die Formel zu sehen.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken **F2**, bearbeiten die Formel und drücken die **Eingabetaste**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die davon abhängigen Formeln im Hintergrund neu berechnet. Weitere Informationen finden Sie unter [Eingaben in Zellen](edit-cells).

## Eine Formel schreiben

Wenn Sie **=** und die ersten Buchstaben einer Funktion eingeben, zeigt eine kurze Liste die Funktionen und [Namen](names) an, die so beginnen, jeweils mit einer kurzen Beschreibung. Drücken Sie die **Tabulatortaste**, um den markierten Eintrag zu übernehmen. Mit den Pfeiltasten **Nach-unten** und **Nach-oben** markieren Sie einen anderen Eintrag und übernehmen ihn dann mit der **Tabulatortaste** oder der **Eingabetaste**. **Esc** schließt die Liste. Sobald Sie sich innerhalb der Klammern einer Funktion befinden, werden ihre Argumente unter der Zelle angezeigt; das Argument, das Sie gerade eingeben, ist fett dargestellt. Die Liste und die Argumente sind dieselben, die auch **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt oder durch Semikolons, wenn in Ihrer Sprache das Komma als Dezimaltrennzeichen dient.

## Manuell berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet die Formeln, während Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** verhält sich genauso und wird für Programme, die Datentabellen gesondert behandeln, in der Datei gespeichert.
- **Manuell** berechnet nichts, bis Sie es veranlassen. Nach einer Änderung zeigt die Statusleiste **Veraltet** an. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere zu berechnen, was noch aussteht. Eine auf „Manuell“ eingestellte Arbeitsmappe wird mit den in der Datei gespeicherten Ergebnissen geöffnet.

Die Einstellung wird mit der Arbeitsmappe gespeichert und lässt sich in einem Schritt rückgängig machen.

## Formeln, die auf sich selbst verweisen

Eine Formel, die direkt oder über andere Zellen auf ihr eigenes Ergebnis verweist, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten ihre bisherigen Werte. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste anzuzeigen, und doppelklicken Sie auf eine Zelle darin, um zu ihr zu wechseln.

Manche Modelle benötigen einen solchen Zirkelbezug absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Befehl **Iterative Berechnung aktivieren**. Dann werden die Formeln immer wieder neu berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die zulässige Anzahl an Iterationen erreicht ist.

## Anzeigen, worauf eine Formel verweist

Markieren Sie eine Zelle, und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Mit jedem weiteren Klick gehen Sie einen Schritt weiter zurück. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die auf die Zelle verweisen; jeder Klick geht einen Schritt weiter. Ein kleines Seitensymbol steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet; gibt es mehr, weist die Statusleiste darauf hin. **Pfeile entfernen** entfernt die Pfeile, ebenso jede Änderung an der Arbeitsmappe.
