# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Neuberechnung, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, Zirkelbezug, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler, Zeigen, zeigen, Zelle anklicken, Bezug, Zellbezug, F4 -->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, z. B. =SUMME(B3:B8). Die Zelle zeigt den Wert an, die **Bearbeitungsleiste** die Formel.

## Was JeffCrunch mit Formeln macht

Beim Öffnen einer Arbeitsmappe enthält die Datei den letzten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Anschließend berechnet JeffCrunch alle Formeln neu, jeweils einige auf einmal, damit das Fenster schnell reagiert. Währenddessen zeigt die Statusleiste **Berechnung läuft** mit einer Prozentangabe an. Danach enthalten die Zellen die von JeffCrunch berechneten Ergebnisse. Um alles neu zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen**, oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch bietet mehr als 200 Funktionen, und es kommen weitere hinzu. Eine Formel mit einer Funktion, die JeffCrunch noch nicht kennt, behält den in der Datei gespeicherten Wert. Das gilt auch für Formeln, die auf eine andere Arbeitsmappe verweisen. Zellen, die von solchen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## REST: der Divisionsrest

**REST(n; d)** gibt zurück, was von **n** übrig bleibt, nachdem **d** so oft wie möglich vollständig abgezogen wurde: n - d * GANZZAHL(n / d). Beispiel: **=REST(10; 3)** ergibt 1, und **=REST(-10; 3)** ergibt 2, weil GANZZAHL(-10 / 3) gleich -4 ist und -10 - 3 * -4 gleich 2. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen**, oder drücken Sie **Strg+`** (auf dem Mac die Control-Taste, nicht die Befehlstaste), um in jeder Zelle statt des Werts die Formel anzuzeigen. Spalten mit Formeln werden bis zu einer Höchstgrenze verbreitert, damit die Formeln vollständig sichtbar sind. Wenn Sie die Anzeige ausschalten, erhalten die Spalten wieder ihre ursprüngliche Breite; die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken **F2**, bearbeiten die Formel und drücken die **Eingabetaste**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die davon abhängigen Formeln im Hintergrund neu berechnet. Siehe [In Zellen eingeben](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt dann die Funktionen und [Namen](names) an, die so beginnen, jeweils mit einer kurzen Beschreibung. Drücken Sie die **Tabulatortaste**, um den markierten Eintrag zu übernehmen. Mit den Tasten **Nach-unten** und **Nach-oben** markieren Sie einen anderen Eintrag und übernehmen ihn dann mit der **Tabulatortaste** oder der **Eingabetaste**. **Esc** schließt die Liste. Sobald Sie sich innerhalb der Klammern einer Funktion befinden, werden unter der Zelle ihre Argumente angezeigt, wobei das Argument, das Sie gerade eingeben, fett dargestellt wird. Die Liste und die Argumente sind dieselben wie unter **Funktion auswählen**. Argumente werden durch Kommas getrennt oder durch Semikolons, wenn in Ihrer Sprache das Komma als Dezimaltrennzeichen dient.

## Auf Zellen zeigen

Statt die Adresse einer Zelle einzugeben, können Sie auf die Zelle zeigen. Setzen Sie beim Eingeben einer Formel den Cursor an eine Stelle, an der eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder nach einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird dann für Sie eingefügt. Ziehen Sie über mehrere Zellen, um einen Bereich wie B2:D5 einzufügen. Wenn Sie erneut klicken, bevor Sie etwas anderes eingeben, ersetzt die neue Zelle die gerade eingefügte. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A einzufügen, oder auf eine Zeilennummer für eine ganze Zeile wie 3:3. Ziehen Sie über die Buchstaben oder Nummern, um B:D oder 2:5 einzufügen.

Auch die Pfeiltasten funktionieren. Wenn Sie eine Pfeiltaste drücken, bewegt sich ein Zeiger von der Zelle weg, in die Sie gerade schreiben, und seine Adresse wird eingefügt. Drücken Sie weitere Pfeiltasten, um den Zeiger weiterzubewegen, oder halten Sie die **Umschalttaste** gedrückt, um ihn über einen Bereich aufzuziehen. Die Statusleiste zeigt **Zeigen** an, während die Pfeiltasten auf Zellen zeigen, **Eingeben**, während Sie tippen, und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Mit **F2** wechseln Sie zwischen Zeigen und Bearbeiten des Texts.

Um auf eine Zelle in einem anderen Blatt zu zeigen, klicken Sie auf die Registerkarte dieses Blatts, während der Cursor an einer Stelle steht, an der eine Zelle stehen kann, und klicken dann auf die Zelle. Der Blattname wird mit eingefügt, bei Bedarf in Anführungszeichen, z. B. 'Mein Blatt'!B3. Mit der **Eingabetaste** kehren Sie zum Ausgangsblatt zurück, und die Formel wird in ihre Zelle übernommen. Mit **Esc** verwerfen Sie alles, auch den Blattwechsel.

Sie können auch den Rand eines umrandeten Bereichs im Blatt ziehen, um diese Adresse zu verschieben, oder das Quadrat an seiner Ecke ziehen, um den Bereich zu vergrößern; die Formel ändert sich dabei mit.

Drücken Sie **F4**, wenn der Cursor in oder direkt hinter einer Adresse steht, um Teile davon mit einem Dollarzeichen zu fixieren: Aus A1 wird $A$1, dann A$1, dann $A1 und dann wieder A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die bezeichneten Zellen werden im Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, auf welche Zellen die Formel zugreift. Ein Klick nach einer vollständigen Adresse, z. B. =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, was eine Formel für viele Werte einer oder zweier Zellen ergibt. Wenn Sie eine Arbeitsmappe mit einer Datentabelle öffnen, berechnet JeffCrunch deren Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle in der linken oberen Ecke der Tabelle; die Werte, die horizontal ausprobiert werden, kommen in die Zeileneingabezelle, die vertikalen in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte entweder in der Spalte links davon (und die Formeln in der Zeile darüber) oder in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel – und aller Formeln, die von der Eingabezelle abhängen –, wenn die Eingabe auf diesen Wert gesetzt wird; der Rest der Arbeitsmappe bleibt unverändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die auf Zellen einer Tabelle zugreifen, folgen. Neue Datentabellen können noch nicht erstellt werden. Eine Tabelle, die nicht berechnet werden kann (weil eine von der Eingabe abhängige Formel eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, wie ZUFALLSZAHL oder JETZT), behält die in ihrer Datei gespeicherten Werte.

## Manuell berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet die Formeln, sobald Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** macht dasselbe, lässt aber die Zellen von Datentabellen unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es veranlassen. Nach einer Änderung zeigt die Statusleiste **Veraltet** an. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere zu berechnen, was noch aussteht. Eine auf „Manuell“ eingestellte Arbeitsmappe wird mit den in ihrer Datei gespeicherten Ergebnissen geöffnet.

Die Einstellung wird mit der Arbeitsmappe gespeichert und lässt sich als ein Schritt rückgängig machen.

## Formeln, die auf sich selbst verweisen

Eine Formel, die direkt oder über andere Zellen auf ihr eigenes Ergebnis verweist, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie im Blatt; sie behalten ihre bisherigen Werte. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste anzuzeigen, und doppelklicken Sie darin auf eine Zelle, um zu ihr zu springen.

Manche Modelle benötigen einen solchen Zirkelbezug absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Befehl **Iterative Berechnung aktivieren**. Die Formeln werden dann wiederholt berechnet, wobei jede die neuesten Werte der anderen verwendet, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die zulässige Anzahl an Iterationen erreicht ist.

## Anzeigen, worauf eine Formel zugreift

Markieren Sie eine Zelle, und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, auf die ihre Formel zugreift. Klicken Sie erneut, um eine weitere Stufe zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die auf die Zelle zugreifen; jeder Klick geht eine Stufe weiter. Ein kleines Seitensymbol steht für eine Zelle in einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet; gibt es mehr, weist die Statusleiste darauf hin. **Pfeile entfernen** entfernt die Pfeile, ebenso jede Änderung an der Arbeitsmappe.
