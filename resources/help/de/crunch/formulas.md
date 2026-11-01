# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Neuberechnung, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, Zirkelbezug, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler -->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Anschließend berechnet JeffCrunch alle Formeln neu, jeweils einige auf einmal, damit das Fenster schnell reagiert. Währenddessen zeigt die Statusleiste **Berechnung läuft** mit einer Prozentangabe an. Danach enthalten die Zellen die von JeffCrunch selbst berechneten Ergebnisse. Um alles neu zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen**, oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch verfügt über mehr als 200 Funktionen, und es kommen weitere hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den in der Datei gespeicherten Wert. Dasselbe gilt für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von solchen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen**, oder drücken Sie **Strg+`** (auf einem Mac die Ctrl-Taste, nicht die Befehlstaste), um in jeder Zelle die Formel statt ihres Werts zu sehen. Spalten mit Formeln werden bis zu einer Höchstgrenze verbreitert, damit die Formeln vollständig sichtbar sind. Wenn Sie die Anzeige wieder ausschalten, erhalten die Spalten ihre ursprüngliche Breite zurück; die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken **F2**, bearbeiten die Formel und drücken die **Eingabetaste**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die davon abhängigen Formeln im Hintergrund neu berechnet. Siehe [In Zellen eingeben](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, jeweils mit einer kurzen Beschreibung. Drücken Sie die **Tabulatortaste**, um den markierten Eintrag zu übernehmen. Mit den Tasten **Nach-unten** und **Nach-oben** markieren Sie einen anderen Eintrag und übernehmen ihn dann mit der **Tabulatortaste** oder der **Eingabetaste**. Mit **Esc** schließen Sie die Liste. Sobald Sie sich innerhalb der Klammern einer Funktion befinden, werden unter der Zelle ihre Argumente angezeigt, wobei das Argument, das Sie gerade eingeben, fett dargestellt ist. Die Liste und die Argumente sind dieselben, die auch **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt oder durch Semikolons, wenn in Ihrer Sprache das Komma als Dezimaltrennzeichen verwendet wird.

## Manuell berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet die Formeln, sobald Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** verhält sich genauso und wird für Programme, die Datentabellen gesondert behandeln, in der Datei gespeichert.
- **Manuell** berechnet nichts, bis Sie es anfordern. Nach einer Änderung zeigt die Statusleiste **Veraltet** an. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere, was noch aussteht, zu berechnen. Eine auf „Manuell“ eingestellte Arbeitsmappe wird mit den Ergebnissen geöffnet, die in ihrer Datei gespeichert sind.

Die Einstellung wird mit der Arbeitsmappe gespeichert und lässt sich als ein Schritt rückgängig machen.

## Formeln, die auf sich selbst verweisen

Eine Formel, die direkt oder über andere Zellen auf ihr eigenes Ergebnis verweist, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die betroffenen Zellen und umrandet sie auf dem Blatt; sie behalten ihre bisherigen Werte. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste anzuzeigen, und doppelklicken Sie auf eine Zelle in der Liste, um zu ihr zu springen.

Manche Modelle benötigen einen solchen Zirkelbezug absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Befehl **Iterative Berechnung aktivieren**. Die Formeln werden dann immer wieder berechnet, wobei jede die neuesten Werte der anderen verwendet, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die zulässige Anzahl von Iterationen erreicht ist.

## Anzeigen, worauf eine Formel verweist

Markieren Sie eine Zelle, und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, auf die ihre Formel verweist. Mit jedem weiteren Klick gehen Sie einen Schritt weiter zurück. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die auf die Zelle verweisen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet; die Statusleiste meldet, wenn es mehr gewesen wären. **Pfeile entfernen** entfernt die Pfeile, ebenso jede Änderung an der Arbeitsmappe.
