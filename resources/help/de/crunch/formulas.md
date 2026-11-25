# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Überlauf, übergelaufen, #ÜBERLAUF!, dynamische Matrix, Überlaufbereich, A1#, nächster Fehler, vorheriger Fehler, Fehler durchgehen, Fehler verfolgen, vor dem Speichern berechnen, Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, auf Zelle klicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Wert benennen, Zellfunktion, Informationsfunktion, FILTERXML, XML, XPath, Genauigkeit wie angezeigt, runden, Rundung, Dezimalstellen, Stellen

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, etwa =SUMME(B3:B8). Die Zelle zeigt den Wert, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, einige nach dem anderen, damit das Fenster schnell bleibt. Während dieser Berechnung zeigt die Statusleiste **Berechnung** mit einem Prozentwert an. Ist sie fertig, enthalten die Zellen die eigenen Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alle neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es werden laufend mehr. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei für sie gespeichert hat. Das gilt auch für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von diesen abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen, zu denen einige Hinweise gehören

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen die Tage so, wie es die **Basis** angibt, die Sie angeben (0 bedeutet 30 Tage pro Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage usw.). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS ermitteln ihr Ergebnis, indem sie Werte ausprobieren, bis Kurs oder Zahlung passt. Sie hören auf, wenn das Ergebnis auf etwa 15 Stellen genau ist, deshalb können die letzten Stellen von denen eines anderen Programms abweichen.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiterhin funktionieren. Die *.INV-Funktionen werden ebenfalls durch Ausprobieren von Werten ermittelt.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINV und RGP liefern einen Block von Werten. Geben Sie eine davon in einer einzelnen Zelle ein, dann füllt der Block die Zellen darunter und daneben (siehe **Formeln, die die Zellen daneben füllen**), oder geben Sie sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen soll.
- **Texte in anderen Schriften.** LÄNGEB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eines, wie auf einem Computer, der nicht für Doppelbyte-Text eingerichtet ist. PHONETIC liefert den Text der Zelle, weil JeffCrunch die Lesungen von japanischem Text nicht speichert.
- **UMWANDELN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eines annimmt, etwa „km“ oder „mL“.

## MOD: der Rest

**REST(n; d)** gibt an, was von **n** übrig bleibt, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Es ist n - d * GANZZAHL(n / d). Beispiel: **=REST(10; 3)** ergibt 1, und **=REST(-10; 3)** ergibt 2, weil GANZZAHL(-10 / 3) gleich -4 ist und -10 - 3 * -4 gleich 2 ergibt. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+#** (auf einem Mac die Ctrl-Taste, nicht die Befehlstaste), um die Formel jeder Zelle anstelle ihres Werts zu sehen. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten werden sie wieder auf ihre Breite zurückgesetzt, und die Spaltenbreiten in Ihrer Datei bleiben unverändert.

## Formeln ändern

Um eine Formel zu ändern, wählen Sie ihre Zelle aus, drücken Sie **F2**, bearbeiten die Formel und drücken **Eingabe**. Sie können sie auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen bearbeiten](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein, dann zeigt eine kurze Liste die Funktionen und [Namen](names) an, die so beginnen, mit je einem Satz dazu. Mit **Tab** übernehmen Sie die markierte. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere, dann **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion sind, werden ihre Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett dargestellt wird. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt, bei Sprachen, die für das Dezimaltrennzeichen ein Komma verwenden, durch Semikolons.

## Funktionsnamen in Ihrer Sprache

Wenn JeffCrunch auf Deutsch, Französisch, Spanisch, Italienisch, Niederländisch, Portugiesisch oder Russisch angezeigt wird, können Sie eine Formel mit den Funktionsnamen eingeben, die Ihre Sprache verwendet, etwa `=SUMME(A1;A2)` auf Deutsch oder `=SOMME(A1;A2)` auf Französisch. Die englischen Namen funktionieren ebenfalls, sodass eine Formel, die Sie aus dem Web kopieren, nicht geändert werden muss. Die Bearbeitungsleiste, die Zelle, die Funktionsliste und **Funktion auswählen** zeigen die Namen Ihrer Sprache an.

Die Arbeitsmappe ändert sich beim Wechsel der Sprache nicht. Eine Datei behält immer die englischen Namen, sodass eine in einer Sprache erstellte Arbeitsmappe in einer anderen und in jedem anderen Programm korrekt geöffnet und gelesen wird. Lokale Namen gibt es für Deutsch, Französisch, Spanisch, Italienisch, Niederländisch, Portugiesisch (Brasilien und Portugal) und Russisch; die englischen Namen funktionieren immer, in jeder Sprache. Ukrainische und indonesische Programme behalten für fast alle Funktionen die englischen Namen. Hat eine Sprache zwei gebräuchliche Namen für eine Funktion, werden beide gelesen, und der gebräuchlichere wird angezeigt. Eine Funktion, die in Ihrer Sprache keinen üblichen Namen hat, behält ihren englischen. WAHR und FALSCH richten sich ebenfalls nach Ihrer Sprache. Die Fehlerwerte Ihrer Sprache, etwa `#NV` auf Deutsch, werden beim Eingeben einer Formel gelesen und in der Bearbeitungsleiste angezeigt, aber eine Zelle mit einem Fehler zeigt immer die Standardform, etwa `#N/A`.

Wenn die regionalen Einstellungen Ihres Computers das Komma als Dezimaltrennzeichen verwenden, werden die Argumente einer Funktion durch ein Semikolon unterschieden. Eine Matrix in geschweiften Klammern mit mehr als einer Zeile wird mit einem Semikolon zwischen den Werten einer Zeile und einem umgekehrten Schrägstrich zwischen Zeilen angezeigt, etwa `{1;2\3;4}`, und Sie können sie so eingeben. Eine ohne umgekehrten Schrägstrich eingegebene Matrix wird so gelesen, wie eine Datei sie schreibt: Kommas zwischen den Werten einer Zeile, Semikolons zwischen Zeilen und ein Punkt als Dezimaltrennzeichen.

Um nur englische Funktionsnamen einzugeben, klicken Sie auf **Datei > Optionen > Formeln** und aktivieren **Englische Funktionsnamen verwenden**.

## Auf Zellen zeigen

Statt die Adresse einer Zelle einzugeben, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor dorthin, wo eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder nach einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie geschrieben. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die gerade geschriebene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A zu schreiben, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 zu schreiben, und ziehen Sie über die Buchstaben oder Zahlen für B:D oder 2:5.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, bewegt sich ein Zeiger von der Zelle weg, in der Sie schreiben, und seine Adresse wird geschrieben; drücken Sie weitere Pfeile, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe** beim Schreiben und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Drücken Sie **F2**, um zwischen Zeigen und Bearbeiten des Textes umzuschalten.

Um auf eine Zelle eines anderen Blatts zu zeigen, klicken Sie auf die Registerkarte dieses Blatts, während der Cursor dort steht, wo eine Zelle stehen kann, und klicken dann auf die Zelle. Der Name des Blatts wird mit geschrieben, in Anführungszeichen, wenn sie nötig sind, etwa als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zu dem Blatt, auf dem Sie begonnen haben, und setzt die Formel in ihre Zelle. **Esc** verwirft alles, auch den Wechsel des Blatts.

Sie können auch den Rand eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder an seinem Eckquadrat ziehen, um ihn zu erweitern; die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in einer Adresse oder direkt dahinter steht, um Teile davon mit einem Dollarzeichen festzulegen: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und dann wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden auf dem Blatt in derselben Farbe umrandet, sodass Sie auf einen Blick sehen, was die Formel liest. Ein Klick hinter einer vollständigen Adresse, etwa =A1, beendet die Eingabe und wählt die angeklickte Zelle aus, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, welches Ergebnis eine Formel für viele Werte einer oder zweier Zellen hat. Öffnen Sie eine Arbeitsmappe, die eine enthält, und JeffCrunch berechnet ihre Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrecht darunter in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte senkrecht in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und jeder Formel, die von der Eingabezelle abhängt, wobei die Eingabe auf diesen Wert gesetzt ist; nichts anderes in der Arbeitsmappe wird verändert. Im Modus **Automatisch** werden die Tabellen mit den Formeln berechnet, und die Formeln, die die Zellen einer Tabelle lesen, folgen. Eine neue Datentabelle anzulegen wird noch nicht angeboten. Eine Tabelle, die nicht berechnet werden kann (weil eine Formel, die von der Eingabe abhängt, eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich selbst ändert, etwa ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, Tabellen manuell** tut dasselbe, lässt die Zellen von Datentabellen aber so, wie sie sind, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Nach einer Änderung steht in der Statusleiste **Veraltet**. Drücken Sie **F9** (**Alle neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Dieses Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere, was wartet, zu berechnen. Eine Arbeitsmappe im Modus Manuell wird mit den Ergebnissen geöffnet, die ihre Datei enthält. **Vor dem Speichern berechnen** im Menü **Berechnungsmodus** ist zunächst aktiviert: Wenn Sie eine manuell berechnete Arbeitsmappe speichern, die veraltet ist, werden die Formeln vorher berechnet, damit die Datei aktuelle Zahlen enthält. Schalten Sie es aus, um die Zahlen so zu speichern, wie sie sind.

Die Auswahl wird mit der Arbeitsmappe gespeichert, und sie ist ein Schritt, den Sie rückgängig machen können.

## Formeln, die sich selbst lesen

Eine Formel, die direkt oder über andere Zellen ihr eigenes Ergebnis liest, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie in ihr auf eine Zelle, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkel absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Eintrag **Iterative Berechnung aktivieren**, dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die erlaubte Anzahl an Durchgängen erreicht ist.

## Zahlen so verwenden, wie die Zellen sie anzeigen

Eine Zelle kann weniger Stellen anzeigen, als sie enthält. Wenn eine Zelle 2,675 enthält und 2,68 anzeigt, verwenden die Formeln, die sie lesen, weiterhin 2,675. Manchmal soll aber das verwendet werden, was Sie sehen. Wählen Sie dann im Menü **Berechnungsoptionen** auf der Registerkarte **Formeln** den Eintrag **Genauigkeit wie angezeigt festlegen**.

JeffCrunch warnt Sie vorher, denn die verborgenen Stellen gehen verloren. Wählen Sie **Abbrechen**, dann ändert sich nichts. Wählen Sie **OK**, dann wird jede Zahl in der Arbeitsmappe auf das gerundet, was ihre Zelle anzeigt. Eine Zelle, die zwei Dezimalstellen anzeigt, behält zwei. Eine Zelle, die einen Prozentwert anzeigt, behält den Prozentwert. Eine Zelle mit einem Datum behält den Tag, und eine Zelle mit einer Uhrzeit behält den kleinsten angezeigten Zeitteil, etwa die Minute. Eine Zelle, die **Standard** anzeigt, behält alle Stellen. Bei einer großen Arbeitsmappe wird während dieses Vorgangs ein Hinweis mit einer Schaltfläche **Abbrechen** angezeigt. Brechen Sie ab, solange der Hinweis angezeigt wird, dann wird nichts verändert.

Danach wird jede Zahl, die Sie eingeben oder einfügen, und jedes Ergebnis einer Formel auf die gleiche Weise gerundet. Die Formeln selbst werden nicht verändert. Die Auswahl wird mit der Arbeitsmappe gespeichert.

Das Runden ist ein Schritt, den Sie rückgängig machen können; **Rückgängig** stellt die Zahlen wieder her. Wenn Sie **Genauigkeit wie angezeigt festlegen** erneut wählen, wird es ausgeschaltet. Das stoppt nur das Runden ab dann. Die bereits gerundeten Zahlen bleiben gerundet.

## Anzeigen, was eine Formel liest

Wählen Sie eine Zelle aus und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile zu den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste zeigt an, wenn es mehr gab. **Pfeile entfernen** löscht sie, und jede Änderung an der Arbeitsmappe ebenfalls.

Zeigt eine Zelle einen Fehler wie #DIV/0!, wählen Sie sie aus und klicken Sie auf **Fehler verfolgen**. Es werden Pfeile zu den Zellen gezeichnet, die sie liest und die ebenfalls einen Fehler anzeigen. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. Zeigt keine der gelesenen Zellen einen Fehler, steht in der Statusleiste, dass diese Zelle den Fehler verursacht, und das ist die Zelle, die Sie korrigieren sollten.

Um jeden Fehler auf dem Blatt durchzugehen, klicken Sie auf **Nächster Fehler** oder **Vorheriger Fehler**. Die Zellen, die einen Fehler anzeigen, werden zeilenweise besucht, und nach der letzten beginnt es wieder bei der ersten. Jede Zelle wird ausgewählt, und die Pfeile werden bis zu der Zelle gezeichnet, die den Fehler verursacht. Die Statusleiste zeigt an, welcher Fehler gerade angezeigt wird und wie viele es gibt.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Überprüfen** der Registerkarte Formeln den Eintrag **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die möglicherweise falsch sind. Gesucht wird nach einer Formel, die zu einem Fehler führt (etwa #DIV/0!), einer Zahl, die als Text gespeichert ist (was Formeln möglicherweise überspringen), und weiteren Fällen: einer Formel, die nicht wie die daneben aussieht, einem Datum, das als Text mit zweistelliger Jahreszahl gespeichert ist, einer Formel in einer nicht gesperrten Zelle, einer Formel, die eine Zahl neben den verwendeten Zellen auslässt (nur für eine Funktion, die einen Bereich addiert oder summiert, etwa **SUMME**, **MITTELWERT**, **ANZAHL**, **MIN** oder **MAX**, wenn der Bereich allein als Argument steht), und einer Formel, die eine leere Zelle verwendet.

Das Dialogfeld zeigt jeweils eine Zelle, springt zu dieser Zelle auf dem Blatt und erklärt in einem Satz, was nicht stimmt. Mit **Weiter** und **Zurück** wechseln Sie zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. Kann die Zelle korrigiert werden, erscheint eine Schaltfläche mit der Korrektur (bei einer Formel, die eine Zahl auslässt, **Zelle einbeziehen**); sie ändert die Formel, und ein Schritt von **Rückgängig** stellt die alte Formel wieder her. **Fehler ignorieren** entfernt die Zelle von der Liste, nimmt ihr Dreieck weg und wird in der Datei gespeichert. Ein Schritt von **Rückgängig** holt sie zurück. Ist nichts falsch, wird Ihnen das mitgeteilt. Um festzulegen, welche dieser Fälle markiert werden, öffnen Sie **Optionen** und die Seite **Formeln**: Dort gibt es für jeden ein Kontrollkästchen, und ein deaktiviertes Kästchen verhindert, dass diese Art markiert wird. Die Auswahl gilt für das Programm und somit für alle geöffneten Fenster. Eine Zelle kann mehr als ein Problem haben, und **Fehler ignorieren** ignoriert nur das angezeigte.

## Formel auswerten

Verwenden Sie dies, um zu sehen, wie eine Formel zu ihrem Wert kommt. Wählen Sie eine Zelle mit einer Formel aus und wählen Sie **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert nimmt seinen Platz ein. Die Teile kommen in dieser Reihenfolge: jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, dann der Rest. **Neu starten** beginnt von vorn. Kann ein Teil nicht berechnet werden, nennt das Dialogfeld den Grund und lässt die Formel, wie sie war.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das während der Arbeit sichtbar bleibt. Es listet Zellen auf, die Sie im Blick behalten wollen, mit Blatt, Zelle, Wert und Formel, auch wenn sie auf einem anderen Blatt liegen. Wählen Sie Zellen aus und drücken Sie **Überwachung hinzufügen**. Wählen Sie Zeilen in der Liste aus und drücken Sie **Überwachung löschen**. Ein Doppelklick auf eine Zeile springt zur Zelle. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Formeln, die die Zellen daneben füllen

Wenn eine Formel einen Block von Werten liefert, füllt der Block die Zellen unter und neben der Zelle, in die Sie die Formel eingegeben haben. Dies heißt Überlauf. Probieren Sie =FOLGE(3) in A1: A1 zeigt 1, und A2 und A3 zeigen 2 und 3. Eine Formel läuft über, wenn sie eine Funktion aufruft, die einen Block liefert (SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, MTRANS, XVERWEIS oder MMULT), einen Block in geschweiften Klammern schreibt, einen Überlaufbereich (siehe unten) verwendet oder mit einem Bereich rechnet, etwa =A1:A3*2.

- **Die Zellen, die sie füllt, enthalten keine eigene Formel.** Sie zeigen die Werte. Wenn Sie eine davon auswählen, zeigt die Bearbeitungsleiste die Formel der Zelle, die sie füllt, in Grau an. Drücken Sie darauf **Eingabe**, ohne etwas zu ändern, ändert sich nichts. Um den Block zu ändern, ändern Sie zuerst diese erste Zelle.
- **Der Block folgt dem Ergebnis.** Liefert die Formel nach einer Änderung der gelesenen Werte mehr oder weniger Werte, wird der Block größer oder kleiner, und Formeln, die diese Zellen lesen, werden neu berechnet.
- **Eine Zelle im Weg ergibt #ÜBERLAUF!.** Enthält eine Zelle, in die der Block käme, einen Wert oder eine Formel, zeigt die erste Zelle **#ÜBERLAUF!**, und nichts wird gefüllt. Leeren Sie die Zelle im Weg, dann füllt der Block sofort. Ein Eintrag in eine Zelle, die der Block füllt, bewirkt dasselbe: Die erste Zelle zeigt #ÜBERLAUF!.
- **Ein Block hat eine Größenbegrenzung.** Ein Block, der größer als das Blatt ist oder mehr als 200.000 Zellen umfasst, ergibt #ÜBERLAUF!.
- **A1# ist der ganze Block.** Schreiben Sie die Zelle, die die Formel enthält, und ein **#**, etwa =SUMME(A1#), um den ganzen Block in seinem aktuellen Stand zu verwenden, auf diesem Blatt oder einem anderen (=SUMME(Tabelle1!A1#)). Überläuft A1 nicht, ist A1# #BEZUG!.

Ein Überlauf wird mit der Datei als dynamische Matrix gespeichert, sodass das Programm, aus dem die Datei stammt, ihn ebenfalls als Überlauf anzeigt. Beim Speichern im älteren Format .xls oder .ods bleiben die Werte erhalten, und die Formel wird als Matrixformel über dem Block geschrieben, wenn das ältere Format die verwendeten Funktionen hat. Kompatibilität prüfen nennt diese Formeln.

## Werten innerhalb einer Formel Namen geben

**LET** benennt einen Wert in der Mitte einer Formel, sodass Sie ihn einmal schreiben und wieder verwenden können. Die Paare aus Name und Wert kommen zuerst, die Berechnung zuletzt. In =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beides. Ein Name kann die davor stehenden Namen verwenden, und ein LET kann bis zu 126 Paare enthalten. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf denselben Namen erneut verwenden. Ein Name kann auch für einen Zellbereich stehen, etwa =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Nach einer Zelle oder der Arbeitsmappe fragen

**ZELLE** liefert Angaben über eine Zelle. Das erste Argument legt fest, welche: "adresse", "zeile", "spalte", "inhalt", "typ", "format" (wie Zahlen angezeigt werden, etwa "F2" für zwei Dezimalstellen, "P0" für einen Prozentwert oder "D1" für ein Datum), "farbe" (1, wenn negative Zahlen farbig angezeigt werden), "klammern" (1, wenn positive Zahlen in Klammern angezeigt werden), "präfix" (das Zeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), "schutz" (1, wenn die Zelle gesperrt ist), "breite" (die Breite der Spalte in Zeichen) und "dateiname" (der Ordner, die Datei und das Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, bis die Arbeitsmappe gespeichert ist). **INFO** liefert Angaben über das Programm und die Arbeitsmappe: "verzeichnis", "numdatei" (wie viele Blätter angezeigt werden), "herkunft", "osversion", "neuberechnen" (Automatisch oder Manuell), "version" und "system".

## Werte aus XML herausholen

**XMLFILTERN** nimmt einen Text, der XML enthält, und einen XPath, eine kurze Schreibweise dafür, welche Teile Sie wollen, und liefert die gefundenen Werte. =XMLFILTERN(A1; "//artikel/preis") liefert jeden Preis im Text in A1, untereinander. Verwenden Sie "/laden/artikel[2]/name" für den zweiten Artikel, "//artikel[@id='b']/name" für den Artikel mit der id b oder "count(//artikel)" für die Anzahl. Ist der Text kein gültiges XML oder der Pfad keiner, lautet das Ergebnis #WERT!. Gibt es keine Treffer, lautet es #NV. XML mit einem Dokumenttyp wird abgelehnt.
