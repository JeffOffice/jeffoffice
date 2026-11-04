# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, Zirkelbezug, Formeln anzeigen, Bearbeitungsleiste, Fehler, Zeigen, Zeigemodus, auf Zelle klicken, Bezug, F4 -->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, einige nach der anderen, damit das Fenster flüssig bleibt. Die Statusleiste zeigt dabei **Berechnung** mit einem Prozentwert an. Ist die Berechnung abgeschlossen, enthalten die Zellen die eigenen Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es kommen laufend weitere hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei für sie gespeichert hat. Das gilt auch für Formeln, die auf eine andere Arbeitsmappe verweisen. Zellen, die davon abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen, zu denen einige Hinweise nötig sind

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen Tage so, wie es die **Basis** angibt, die Sie angeben (0 steht für 30 Tage je Monat in einem Jahr mit 360 Tagen, 1 für die tatsächlichen Tage und so weiter). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS ermitteln ihr Ergebnis, indem sie Werte ausprobieren, bis Kurs oder Zahlung passt. Sie hören auf, wenn das Ergebnis auf etwa 15 Stellen genau ist, daher können die letzten Stellen von denen eines anderen Programms abweichen.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, und die älteren Namen wie NORMVERT und TVERT liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiter funktionieren. Die Funktionen *.INV werden ebenfalls durch Ausprobieren von Werten ermittelt.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINVERS und RGP liefern einen Block von Werten. Geben Sie sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen sollen. Dass eine Formel in die daneben liegenden Zellen überläuft, ist noch nicht umgesetzt.
- **Texte in anderen Schriften.** LÄNGEB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eins, so wie auf einem Computer, der nicht für Doppelbyte-Text eingerichtet ist. PHONETIC gibt den Text der Zelle aus, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **UMWANDELN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion angegeben: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor eine Einheit gesetzt werden, die ein solches verträgt, etwa „km“ oder „mL“.

## MOD: der Rest

**MOD(n; d)** ergibt den Rest von **n**, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Er ist n - d * GANZZAHL(n / d). Beispiel: **=MOD(10; 3)** ergibt 1, und **=MOD(-10; 3)** ergibt 2, denn GANZZAHL(-10 / 3) ist -4, und -10 - 3 * -4 ergibt 2. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+#** (auf dem Mac die Ctrl-Taste statt der Befehlstaste), um anstelle des Werts die Formel jeder Zelle zu sehen. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite, und die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken Sie **F2**, bearbeiten Sie die Formel und drücken Sie **Eingabe**. Sie können sie auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen bearbeiten](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, jeweils mit einem Satz Erläuterung. Drücken Sie **Tab**, um die markierte Funktion zu übernehmen. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere und übernehmen sie dann mit **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion stehen, werden deren Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett dargestellt wird. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt, in Sprachen, die als Dezimaltrennzeichen ein Komma verwenden, durch Semikolons.

## Auf Zellen zeigen

Anstatt die Adresse einer Zelle einzugeben, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie die Einfügemarke dorthin, wo eine Zelle stehen kann (direkt nach **=**, hinter einem Operator wie **+**, einer öffnenden Klammer oder einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie eingetragen. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, und die neue Zelle ersetzt die gerade eingetragene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A zu schreiben, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 zu schreiben, und ziehen Sie über die Buchstaben oder Zahlen für B:D oder 2:5.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, und ein Zeiger wandert von der Zelle weg, in die Sie gerade schreiben, und seine Adresse wird eingetragen. Drücken Sie weitere Pfeile, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe**, während Sie tippen, und **Bearbeiten**, wenn die Pfeiltasten die Einfügemarke im Text bewegen. Drücken Sie **F2**, um zwischen Zeigen und Bearbeiten des Texts umzuschalten.

Um auf eine Zelle auf einem anderen Blatt zu zeigen, klicken Sie auf den Registerreiter dieses Blatts, während die Einfügemarke an einer Stelle steht, wo eine Zelle stehen kann, und klicken Sie dann auf die Zelle. Der Blattname wird mit angegeben, bei Bedarf in Anführungszeichen, wie in 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zu dem Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rand eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder an dessen Eckquadrat ziehen, um ihn zu vergrößern; die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während die Einfügemarke in oder direkt hinter einer Adresse steht, um Teile davon mit einem Dollarzeichen zu fixieren: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und dann wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden auf dem Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick hinter einer vollständigen Adresse, wie in =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text auch.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, was eine Formel für viele Werte einer oder zweier Zellen ergibt. Öffnen Sie eine Arbeitsmappe mit einer solchen Tabelle, berechnet JeffCrunch ihre Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrecht ausprobierten Werte in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte senkrecht in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und aller Formeln, die von der Eingabezelle abhängen, wobei die Eingabe auf diesen Wert gesetzt ist; sonst wird nichts in der Arbeitsmappe verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die die Zellen einer Tabelle lesen, werden nachgezogen. Das Erstellen einer neuen Datentabelle wird noch nicht angeboten. Eine Tabelle, die nicht berechnet werden kann (eine Formel, die von der Eingabe abhängt, verwendet eine Funktion, die JeffCrunch nicht kennt, oder ändert sich von selbst, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** arbeitet genauso, lässt die Zellen von Datentabellen aber unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Die Statusleiste zeigt nach einer Änderung **Veraltet** an. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere, was auf Berechnung wartet, zu berechnen. Eine Arbeitsmappe, die auf Manuell eingestellt ist, wird mit den Ergebnissen geöffnet, die die Datei enthält.

Die Auswahl wird mit der Arbeitsmappe gespeichert, und sie ist ein Schritt, den Sie rückgängig machen können.

## Formeln, die sich selbst lesen

Eine Formel, die ihr eigenes Ergebnis liest, direkt oder über andere Zellen, lässt sich nicht auf die übliche Weise berechnen. JeffCrunch meldet das einmal, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie in der Liste auf eine Zelle, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkelbezug absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Eintrag **Iterative Berechnung aktivieren**, dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den von Ihnen angegebenen Betrag ändert oder die erlaubte Anzahl an Durchläufen erreicht ist.

## Anzeigen, was eine Formel liest

Markieren Sie eine Zelle und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Ein weiterer Klick geht einen Schritt weiter zurück. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste zeigt an, wenn es mehr gab. **Pfeile entfernen** löscht sie, und jede Änderung an der Arbeitsmappe ebenfalls.
