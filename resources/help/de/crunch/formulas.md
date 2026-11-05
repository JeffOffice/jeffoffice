# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, Zirkelbezug, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, Zeigen, auf Zelle klicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Name für Wert, Zellfunktion, Infofunktion, FILTERXML, XML, XPath-->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen ermittelt, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Beim Öffnen einer Arbeitsmappe enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch alle Formeln erneut, einige auf einmal, damit das Fenster flüssig bleibt. In der Statusleiste steht während der Berechnung **Berechnung** mit einer Prozentangabe. Ist sie abgeschlossen, enthalten die Zellen die eigenen Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es kommen laufend weitere hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei dafür gespeichert hat. Das gilt auch für Formeln, die auf eine andere Arbeitsmappe verweisen. Zellen, die von diesen abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen, zu denen einige Hinweise gehören

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen die Tage so, wie es die **Basis** angibt, die Sie angeben (0 bedeutet 30 Tage je Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage usw.). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS ermitteln ihr Ergebnis, indem sie Werte ausprobieren, bis Kurs oder Zahlung passen. Sie brechen ab, wenn das Ergebnis auf etwa 15 Stellen genau ist, deshalb können sich die letzten Stellen von denen eines anderen Programms unterscheiden.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT, liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiter funktionieren. Die Funktionen *.INV werden ebenfalls durch Ausprobieren von Werten ermittelt.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINV und RGP liefern einen Block von Werten. Geben Sie sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen sollen. Dass eine Formel in die daneben liegenden Zellen überläuft, ist noch nicht umgesetzt.
- **Texte in anderen Schriften.** LÄNGEB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eines, so wie auf einem Computer, der nicht für Doppelbyte-Text eingerichtet ist. PHONETIC liefert den Text der Zelle, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **UMWANDELN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion angegeben: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor eine Einheit gesetzt werden, die eines verlangt, etwa „km“ oder „mL“.

## MREST: der Rest

**MREST(n; d)** liefert den Rest von **n**, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Er ist n - d * GANZZAHL(n / d). Beispiel: **=MREST(10; 3)** ergibt 1, und **=MREST(-10; 3)** ergibt 2, denn GANZZAHL(-10 / 3) ist -4, und -10 - 3 * -4 ergibt 2. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+^** (auf dem Mac die Control-Taste, nicht die Befehlstaste), um statt des Werts in jeder Zelle die Formel zu sehen. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre Breite, und die Spaltenbreiten in Ihrer Datei werden nicht verändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken **F2**, bearbeiten die Formel und drücken **Eingabe**. Sie können sie auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen beschreiben](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, mit einem Satz zu jeder. Drücken Sie **Tab**, um die markierte zu übernehmen. Mit den Pfeiltasten **Ab** und **Auf** markieren Sie eine andere, dann **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion stehen, werden deren Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett erscheint. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Semikolons getrennt, in Sprachen mit Dezimalkomma durch Semikolon, in den anderen durch Komma.

## Auf Zellen zeigen

Anstatt die Adresse einer Zelle einzutippen, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor dorthin, wo eine Zelle stehen kann (direkt nach **=**, hinter einem Operator wie **+**, nach einer öffnenden Klammer oder einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie eingetragen. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 einzutragen. Klicken Sie erneut, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die gerade eingetragene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A einzutragen, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 einzutragen. Ziehen Sie über die Buchstaben oder Zahlen, um B:D oder 2:5 einzutragen.

Auch die Pfeiltasten funktionieren. Drücken Sie eine Pfeiltaste, bewegt sich ein Zeiger von der Zelle weg, in die Sie schreiben, und seine Adresse wird eingetragen. Drücken Sie weitere Pfeiltasten, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich auszudehnen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe**, während Sie tippen, und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Mit **F2** wechseln Sie zwischen Zeigen und Bearbeiten des Textes.

Um auf eine Zelle eines anderen Blatts zu zeigen, klicken Sie auf den Registerreiter dieses Blatts, während der Cursor dort stehen kann, wo eine Zelle stehen kann, und klicken dann auf die Zelle. Der Blattname wird mit eingetragen, bei Bedarf in Anführungszeichen, etwa als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zum Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rahmen eines umrandeten Bereichs im Blatt ziehen, um diese Adresse zu verschieben, oder an seinem Eckquadrat ziehen, um ihn zu vergrößern. Die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in oder direkt hinter einer Adresse steht, um Teile davon mit einem Dollarzeichen festzusetzen: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und schließlich wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden im Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick hinter einer vollständigen Adresse, etwa =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, welches Ergebnis eine Formel für viele Werte einer oder zweier Zellen hat. Öffnen Sie eine Arbeitsmappe, die eine solche Tabelle enthält, und JeffCrunch berechnet deren Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrechten in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte senkrecht in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und aller Formeln, die von der Eingabezelle abhängen, wobei die Eingabe auf diesen Wert gesetzt ist. Sonst wird nichts in der Arbeitsmappe verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die auf Zellen einer Tabelle zugreifen, folgen. Eine neue Datentabelle anzulegen wird noch nicht angeboten. Eine Tabelle, die sich nicht berechnen lässt (weil eine Formel von der Eingabe abhängt und eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** tut dasselbe, lässt die Zellen von Datentabellen aber unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Die Statusleiste zeigt nach einer Änderung **Veraltet**. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Dieses Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere Ausstehende zu berechnen. Eine Arbeitsmappe, die auf Manuell eingestellt ist, öffnet sich mit den Ergebnissen, die ihre Datei enthält.

Die Auswahl wird mit der Arbeitsmappe gespeichert, und sie ist ein Schritt, den Sie rückgängig machen können.

## Formeln, die sich selbst lesen

Eine Formel, die ihr eigenes Ergebnis liest, direkt oder über andere Zellen, lässt sich nicht auf die übliche Weise berechnen. JeffCrunch meldet dies einmal, nennt die Zellen und umrandet sie im Blatt. Sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie in der Liste auf eine Zelle, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkel mit Absicht. Wählen Sie im Menü **Berechnungsmodus** den Eintrag **Iterative Berechnung aktivieren**, dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die erlaubte Anzahl an Durchläufen erreicht ist.

## Sehen, was eine Formel liest

Markieren Sie eine Zelle und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Ein weiterer Klick geht einen Schritt weiter zurück. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste meldet, wenn es mehr gewesen wären. **Pfeile entfernen** löscht sie, ebenso jede Änderung an der Arbeitsmappe.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Überprüfen** der Registerkarte Formeln die Option **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die fehlerhaft sein könnten. Sie sucht nach zwei Dingen: einer Formel, die einen Fehler ergibt (etwa #DIV/0!), und einer Zahl, die als Text gespeichert ist und die Formeln möglicherweise überspringen.

Das Dialogfeld zeigt jeweils eine Zelle, springt im Blatt zu dieser Zelle und erklärt in einem Satz, was nicht stimmt. Mit **Weiter** und **Zurück** bewegen Sie sich durch die Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie im Blatt. **Fehler ignorieren** nimmt die Zelle von der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt **Rückgängig** bringt sie zurück. Ist nichts falsch, wird Ihnen das mitgeteilt.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Markieren Sie eine Zelle mit einer Formel und wählen Sie **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert nimmt seinen Platz ein. Die Teile werden in dieser Reihenfolge abgearbeitet: jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, dann der Rest. **Neu starten** beginnt von vorn. Lässt sich ein Teil nicht berechnen, nennt das Dialogfeld den Grund und lässt die Formel, wie sie war.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das beim Arbeiten sichtbar bleibt. Es listet Zellen auf, die Sie im Auge behalten möchten, mit Blatt, Zelle, Wert und Formel, auch wenn sie auf einem anderen Blatt liegen. Markieren Sie Zellen und drücken Sie **Überwachung hinzufügen**. Markieren Sie Zeilen in der Liste und drücken Sie **Überwachung löschen**. Ein Doppelklick auf eine Zeile führt zur Zelle. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Werten innerhalb einer Formel Namen geben

**LET** gibt einem Wert mitten in einer Formel einen Namen, sodass Sie ihn einmal schreiben und danach wieder verwenden können. Die Paare aus Name und Wert kommen zuerst, die Berechnung zuletzt. In =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beides. Ein Name kann die davor stehenden Namen verwenden. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf einen Namen wiederverwenden. Ein Name kann auch für einen Zellbereich stehen, etwa in =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Etwas über eine Zelle oder die Arbeitsmappe abfragen

**ZELLE** liefert Angaben zu einer Zelle. Ihr erstes Argument legt fest, welche: "adresse", "zeile", "spalte", "inhalt", "typ", "format" (wie Zahlen angezeigt werden, etwa "F2" für zwei Dezimalstellen, "P0" für Prozent oder "D1" für ein Datum), "farbe" (1, wenn negative Zahlen farbig angezeigt werden), "klammern" (1, wenn positive Zahlen in Klammern stehen), "präfix" (das Kennzeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), "schutz" (1, wenn die Zelle gesperrt ist), "breite" (die Breite der Spalte in Zeichen) und "dateiname" (Ordner, Datei und Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, bis die Arbeitsmappe gespeichert ist). **INFO** liefert Angaben zum Programm und zur Arbeitsmappe: "verzeichnis", "numdatei" (wie viele Blätter angezeigt werden), "herkunft", "osversion", "neuberechnung" (Automatisch oder Manuell), "version" und "system".

## Werte aus XML auslesen

**FILTERXML** nimmt Text, der XML enthält, und einen XPath, eine kurze Angabe, welche Teile Sie möchten, und liefert die gefundenen Werte. =FILTERXML(A1; "//artikel/preis") liefert jeden Preis im Text in A1, untereinander. Verwenden Sie "/laden/artikel[2]/name" für den zweiten Artikel, "//artikel[@id='b']/name" für den Artikel mit der id b oder "count(//artikel)" für die Anzahl. Ist der Text kein gültiges XML oder der Pfad keiner, lautet das Ergebnis #WERT!. Trifft nichts zu, lautet es #NV. XML mit einem Dokumenttyp wird abgelehnt.
