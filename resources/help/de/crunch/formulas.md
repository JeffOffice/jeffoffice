# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, Zeigen, auf Zelle klicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen -->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, jeweils einige wenige, damit das Fenster flüssig bleibt. Während dieser Berechnung zeigt die Statusleiste **Berechnung** mit einem Prozentwert an. Ist sie abgeschlossen, enthalten die Zellen die eigenen Ergebnisse von JeffCrunch. Um alles neu zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es werden laufend mehr. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei dafür gespeichert hat. Das gilt auch für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von diesen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen, zu denen einige Hinweise gehören

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen die Tage so, wie es die **Basis** angibt, die Sie angeben (0 bedeutet 30 Tage je Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage usw.). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS finden ihr Ergebnis, indem sie Werte ausprobieren, bis Kurs oder Zahlung passen. Sie brechen ab, wenn das Ergebnis auf etwa 15 Stellen genau ist, daher können sich die letzten Stellen von denen eines anderen Programms unterscheiden.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiter funktionieren. Die *.INV-Funktionen werden ebenfalls durch Ausprobieren von Werten gefunden.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINV und RGP liefern einen Block von Werten. Geben Sie sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen sollen. Dass eine Formel in die danebenliegenden Zellen überläuft, ist noch nicht umgesetzt.
- **Texte in anderen Schriften.** LENB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eins, so wie es auf einem Computer geschieht, der nicht für Doppelbyte-Text eingerichtet ist. PHONETIC gibt den Text der Zelle aus, da JeffCrunch die Lesungen von japanischem Text nicht speichert.
- **UMRECHNEN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eines annimmt, etwa „km“ oder „mL“.

## REST: der Rest einer Division

**REST(n; d)** gibt an, was von **n** übrig bleibt, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Es ist n - d * GANZZAHL(n / d). Beispiel: **=REST(10; 3)** ergibt 1, und **=REST(-10; 3)** ergibt 2, denn GANZZAHL(-10 / 3) ist -4, und -10 - 3 * -4 ergibt 2. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+^** (auf dem Mac die Control-Taste, nicht die Befehlstaste), um statt der Werte in jeder Zelle die Formel zu sehen. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite, und die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken Sie **F2**, bearbeiten Sie die Formel und drücken Sie **Eingabe**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen bearbeiten](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, mit einem Satz zu jeder. Drücken Sie **Tab**, um die markierte zu übernehmen. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere und übernehmen sie dann mit **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion stehen, werden ihre Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett erscheint. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Semikolons getrennt; in JeffCrunch wird das Semikolon verwendet. Wenn Ihre Sprache das Komma als Dezimaltrennzeichen verwendet, ist das Semikolon das übliche Trennzeichen.

## Auf Zellen zeigen

Statt die Adresse einer Zelle einzutippen, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor an eine Stelle, an der eine Zelle stehen kann (direkt nach **=**, hinter einem Operator wie **+**, nach einer öffnenden Klammer oder einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie eingetragen. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die gerade eingetragene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A einzutragen, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 einzutragen, und ziehen Sie über die Buchstaben oder Zahlen für B:D oder 2:5.

Auch die Pfeiltasten funktionieren. Drücken Sie eine Pfeiltaste, bewegt sich ein Zeiger von der Zelle weg, in der Sie gerade eingeben, und seine Adresse wird eingetragen. Drücken Sie weitere Pfeiltasten, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe**, während Sie tippen, und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Drücken Sie **F2**, um zwischen Zeigen und Bearbeiten des Textes umzuschalten.

Um auf eine Zelle auf einem anderen Blatt zu zeigen, klicken Sie auf die Registerkarte dieses Blatts, während der Cursor an einer Stelle steht, an der eine Zelle stehen kann, und klicken Sie dann auf die Zelle. Der Blattname wird mitgeschrieben, bei Bedarf in Anführungszeichen, etwa als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zum Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rand eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder das Eckquadrat ziehen, um sie zu erweitern; die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in einer Adresse oder direkt dahinter steht, um Teile davon mit einem Dollarzeichen festzusetzen: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und dann wieder zu A1.

Jede Adresse in der Formel ist im Text farbig markiert, und die Zellen, auf die sie verweist, sind auf dem Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick nach einer vollständigen Adresse, etwa =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, was eine Formel für viele Werte einer oder zweier Zellen ergibt. Öffnen Sie eine Arbeitsmappe, die eine solche Tabelle enthält, dann berechnet JeffCrunch deren Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrecht stehenden in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte senkrecht in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und jeder Formel, die von der Eingabezelle abhängt, wobei die Eingabe auf diesen Wert gesetzt ist; sonst wird nichts in der Arbeitsmappe verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die die Zellen einer Tabelle lesen, folgen. Eine neue Datentabelle zu erstellen, wird noch nicht angeboten. Eine Tabelle, die nicht berechnet werden kann (weil eine Formel, die von der Eingabe abhängt, eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** tut dasselbe, lässt aber die Zellen von Datentabellen unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Die Statusleiste zeigt nach einer Änderung **Veraltet** an. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere, was wartet, zu berechnen. Eine auf Manuell eingestellte Arbeitsmappe wird mit den Ergebnissen geöffnet, die ihre Datei enthält.

Die Auswahl wird mit der Arbeitsmappe gespeichert, und sie lässt sich rückgängig machen.

## Formeln, die sich selbst lesen

Eine Formel, die ihr eigenes Ergebnis liest, direkt oder über andere Zellen, lässt sich nicht auf die übliche Weise berechnen. JeffCrunch meldet dies einmal, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie in der Liste auf eine Zelle, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkelbezug absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Eintrag **Iterative Berechnung aktivieren**, dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die festgelegte Anzahl an Durchläufen erreicht ist.

## Anzeigen, was eine Formel liest

Markieren Sie eine Zelle und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet; die Statusleiste zeigt an, wenn es mehr gab. **Pfeile entfernen** löscht sie, ebenso jede Änderung an der Arbeitsmappe.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Prüfen** der Registerkarte Formeln die Option **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die fehlerhaft sein könnten. Sie sucht nach zwei Dingen: einer Formel, die einen Fehler ergibt (etwa #DIV/0!), und einer Zahl, die als Text gespeichert ist und die Formeln möglicherweise übergehen.

Das Dialogfeld zeigt jeweils eine Zelle, springt auf dem Blatt zu dieser Zelle und erklärt in einem Satz, was nicht stimmt. **Weiter** und **Zurück** wechseln zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. **Fehler ignorieren** nimmt die Zelle aus der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt Rückgängig holt sie zurück. Ist nichts fehlerhaft, wird Ihnen das mitgeteilt.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Wählen Sie eine Zelle mit einer Formel und klicken Sie auf **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert nimmt dessen Stelle ein. Die Teile werden in dieser Reihenfolge abgearbeitet: zuerst jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, und dann der Rest. **Neu starten** beginnt von vorn. Lässt sich ein Teil nicht berechnen, nennt das Dialogfeld den Grund und lässt die Formel unverändert.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das beim Arbeiten sichtbar bleibt. Es listet Zellen auf, die Sie im Auge behalten möchten, mit Blatt, Zelle, Wert und Formel, auch wenn sie auf einem anderen Blatt liegen. Markieren Sie Zellen und drücken Sie **Überwachung hinzufügen**. Markieren Sie Zeilen in der Liste und drücken Sie **Überwachung löschen**. Doppelklicken Sie auf eine Zeile, um zur Zelle zu gelangen. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.
