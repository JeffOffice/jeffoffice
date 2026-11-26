# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Überlauf, übergelaufen, #ÜBERLAUF!, dynamisches Array, Überlaufbereich, A1#, nächster Fehler, vorheriger Fehler, Fehler durchgehen, Fehler nachverfolgen, vor dem Speichern berechnen, Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Zirkelbezug, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, Zelle anklicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Wert benennen, Zellfunktion, Informationsfunktion, FILTERXML, XML, XPath, Genauigkeit wie angezeigt, runden, Rundung, Dezimalstellen, Stellen-->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, etwa =SUMME(B3:B8). Die Zelle zeigt den Wert, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, nach und nach, damit das Fenster flüssig bleibt. Die Statusleiste zeigt dabei **Berechnung läuft…** mit einem Prozentwert an. Wenn die Berechnung abgeschlossen ist, enthalten die Zellen die eigenen Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es kommen laufend weitere hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei für sie gespeichert hat. Das gilt auch für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von diesen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen, zu denen einige Hinweise gehören

- **Anleihen und Darlehen.** PRICE, YIELD, DURATION, ODDFPRICE und die anderen Anleihefunktionen zählen Tage so, wie es die angegebene **Basis** vorgibt (0 bedeutet 30 Tage pro Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage usw.). YIELD, ODDFYIELD, ODDLYIELD und RATE ermitteln ihr Ergebnis, indem sie Werte ausprobieren, bis Kurs oder Zahlung passen. Sie brechen ab, wenn das Ergebnis auf etwa 15 Stellen genau ist, daher können die letzten Stellen von denen eines anderen Programms abweichen.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT, liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiter funktionieren. Die Funktionen *.INV werden ebenfalls durch Ausprobieren von Werten ermittelt.
- **Listen, die mehrere Zellen füllen.** SORT, SORTIERENNACH (SORTBY), FILTER, EINDEUTIG (UNIQUE), SEQUENZ (SEQUENCE), ZUFALLSMATRIX (RANDARRAY), XVERWEIS (XLOOKUP), MMULT, MINVERS (MINVERSE) und RGP (LINEST) liefern einen Block von Werten. Geben Sie eine solche Funktion in einer einzelnen Zelle ein, und der Block läuft in die Zellen darunter und daneben über (siehe **Formeln, die die Zellen daneben füllen**). Alternativ geben Sie sie als Matrixformel ein (**Strg+Umschalt+Eingabe**) über den Zellen, die sie füllen soll.
- **Texte in anderen Schriften.** LENB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eines, so wie auf einem Computer, der nicht für Doppelbytetext eingerichtet ist. PHONETIC liefert den Text der Zelle, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **UMWANDELN (CONVERT).** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eines annimmt, etwa „km“ oder „mL“.

## MOD: der Rest

**MOD(n; d)** liefert den Rest von **n**, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Es ist n - d * GANZZAHL(n / d). Beispiel: **=REST(10; 3)** ergibt 1, und **=REST(-10; 3)** ergibt 2, weil GANZZAHL(-10 / 3) gleich -4 ist und -10 - 3 * -4 gleich 2 ergibt. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, ist das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+^** (auf dem Mac die Ctrl-Taste, nicht die Befehlstaste), um in jeder Zelle die Formel statt des Werts zu sehen. Die Spalten mit Formeln werden bis zu einer Grenze breiter, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite, und die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, wählen Sie ihre Zelle aus, drücken Sie **F2**, bearbeiten die Formel und drücken **Eingabe**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Eingeben in Zellen](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, jeweils mit einem Satz dazu. Drücken Sie **Tab**, um die markierte zu übernehmen. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere und übernehmen sie mit **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion stehen, werden deren Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett erscheint. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt, in Sprachen, die als Dezimalzeichen ein Komma verwenden, durch Semikolons.

## Funktionsnamen in Ihrer Sprache

Wenn JeffCrunch auf Deutsch, Französisch, Spanisch, Italienisch, Niederländisch, Portugiesisch oder Russisch angezeigt wird, können Sie eine Formel mit den Funktionsnamen Ihrer Sprache eingeben, etwa `=SUMME(A1;A2)` auf Deutsch oder `=SOMME(A1;A2)` auf Französisch. Die englischen Namen funktionieren ebenfalls, sodass eine Formel, die Sie aus dem Web kopieren, nicht geändert werden muss. Die Bearbeitungsleiste, die Zelle, die Funktionsliste und **Funktion auswählen** zeigen die Namen Ihrer Sprache.

Die Arbeitsmappe ändert sich beim Umschalten nicht. Eine Datei speichert immer die englischen Namen, sodass eine in einer Sprache erstellte Arbeitsmappe in einer anderen Sprache und in jedem anderen Programm korrekt geöffnet und gelesen wird. Lokale Namen gibt es für Deutsch, Französisch, Spanisch, Italienisch, Niederländisch, Portugiesisch (Brasilien und Portugal) und Russisch; die englischen Namen funktionieren immer, in jeder Sprache. Ukrainische und indonesische Programme behalten für fast alle Funktionen die englischen Namen. Hat eine Sprache zwei gebräuchliche Namen für eine Funktion, werden beide gelesen, und der gebräuchlichere wird angezeigt. Eine Funktion, die in Ihrer Sprache keinen üblichen Namen hat, behält den englischen. WAHR und FALSCH richten sich ebenfalls nach Ihrer Sprache. Die Fehlerwerte Ihrer Sprache, etwa `#NV` auf Deutsch, werden beim Eingeben einer Formel gelesen und in der Bearbeitungsleiste angezeigt. Eine Zelle mit einem Fehler zeigt aber immer die Standardform, etwa `#NV/A`.

Verwenden die Ländereinstellungen Ihres Computers das Komma als Dezimalzeichen, werden die Argumente einer Funktion durch ein Semikolon getrennt. Eine Matrix in geschweiften Klammern mit mehr als einer Zeile wird mit einem Semikolon zwischen den Werten einer Zeile und einem umgekehrten Schrägstrich zwischen den Zeilen angezeigt, etwa `{1;2\3;4}`, und Sie können sie so eingeben. Eine Matrix, die ohne umgekehrten Schrägstrich eingegeben wird, wird so gelesen, wie eine Datei sie schreibt: Kommas zwischen den Werten einer Zeile, Semikolons zwischen den Zeilen und ein Punkt als Dezimalzeichen.

Um nur englische Funktionsnamen zu verwenden, klicken Sie auf **Datei > Optionen > Formeln** und aktivieren **Englische Funktionsnamen verwenden**.

## Auf Zellen zeigen

Statt die Adresse einer Zelle einzutippen, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor dorthin, wo eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie geschrieben. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die zuletzt geschriebene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A zu schreiben, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 zu schreiben. Ziehen Sie über die Buchstaben oder Nummern, um etwa B:D oder 2:5 zu schreiben.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, bewegt sich ein Zeiger von der Zelle weg, in die Sie gerade schreiben, und seine Adresse wird geschrieben. Drücken Sie weitere Pfeile, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe**, während Sie schreiben, und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Mit **F2** wechseln Sie zwischen Zeigen und Bearbeiten des Textes.

Um auf eine Zelle eines anderen Blatts zu zeigen, klicken Sie auf den Registerreiter dieses Blatts, während der Cursor an einer Stelle steht, an der eine Zelle stehen kann, und klicken dann auf die Zelle. Der Name des Blatts wird mit geschrieben, bei Bedarf in Anführungszeichen, etwa als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück auf das Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rahmen eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder am Eckquadrat ziehen, um sie zu vergrößern; die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in einer Adresse oder direkt dahinter steht, um Teile davon mit einem Dollarzeichen festzulegen: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und dann wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden auf dem Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick hinter einer vollständigen Adresse, etwa =A1, beendet die Eingabe und wählt die angeklickte Zelle aus, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, was eine Formel für viele Werte einer oder zweier Zellen ergibt. Öffnen Sie eine Arbeitsmappe, die eine solche Tabelle enthält, und JeffCrunch berechnet ihre Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrecht stehenden in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte in der Spalte links davon (dann stehen die Formeln in der Zeile darüber) oder in der Zeile darüber (dann stehen die Formeln in der Spalte links). Jede Zelle der Tabelle ist das Ergebnis der Formel und aller Formeln, die von der Eingabezelle abhängen, wobei die Eingabe auf diesen Wert gesetzt ist. Sonst wird nichts in der Arbeitsmappe verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die die Zellen einer Tabelle lesen, folgen. Eine neue Datentabelle anzulegen wird noch nicht angeboten. Eine Tabelle, die nicht berechnet werden kann (weil eine Formel, die von der Eingabe abhängt, eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsart**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, Tabellen manuell** tut dasselbe, lässt aber die Zellen von Datentabellen unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Die Statusleiste zeigt nach einer Änderung **Nicht aktuell**. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Blatt neu berechnen**), um die Formeln des aktuellen Blatts zu berechnen, und alles andere, das noch wartet. Eine Arbeitsmappe, die auf Manuell eingestellt ist, öffnet sich mit den Ergebnissen, die ihre Datei enthält. **Vor dem Speichern berechnen** im Menü **Berechnungsart** ist anfangs aktiviert: Wenn Sie eine manuelle Arbeitsmappe speichern, die nicht aktuell ist, werden die Formeln vorher berechnet, damit die Datei aktuelle Zahlen enthält. Schalten Sie es aus, um die Zahlen so zu speichern, wie sie sind.

Die Auswahl wird mit der Arbeitsmappe gespeichert und lässt sich als ein Schritt rückgängig machen.

## Formeln, die sich selbst lesen

Eine Formel, die direkt oder über andere Zellen ihr eigenes Ergebnis liest, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie auf eine Zelle darin, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkel absichtlich. Wählen Sie im Menü **Berechnungsart** die Option **Iterative Berechnung aktivieren**. Dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die erlaubte Anzahl an Durchläufen erreicht ist.

## Zahlen so verwenden, wie die Zellen sie anzeigen

Eine Zelle kann weniger Stellen anzeigen, als sie speichert. Enthält eine Zelle 2,675 und zeigt 2,68 an, verwenden die Formeln, die sie lesen, dennoch 2,675. Manchmal soll aber das verwendet werden, was Sie sehen. Dann wählen Sie auf der Registerkarte **Formeln** im Menü **Berechnungsoptionen** die Option **Genauigkeit wie angezeigt festlegen**.

JeffCrunch warnt Sie vorher, denn die verborgenen Stellen gehen verloren. Wählen Sie **Abbrechen**, dann ändert sich nichts. Wählen Sie **OK**, wird jede Zahl in der Arbeitsmappe auf das gerundet, was ihre Zelle anzeigt. Eine Zelle, die zwei Dezimalstellen anzeigt, behält zwei. Eine Zelle, die einen Prozentwert anzeigt, behält den Prozentwert. Eine Zelle mit einem Datum behält den Tag, und eine Zelle mit einer Uhrzeit behält den kleinsten angezeigten Zeitanteil, etwa die Minute. Eine Zelle, die **Standard** anzeigt, behält alle ihre Stellen. Bei einer großen Arbeitsmappe wird während dieses Vorgangs ein Hinweis mit einer Schaltfläche **Abbrechen** angezeigt. Brechen Sie ab, solange der Hinweis angezeigt wird, dann wird nichts geändert.

Danach wird jede Zahl, die Sie eingeben oder einfügen, und jedes Ergebnis einer Formel auf die gleiche Weise gerundet. Die Formeln selbst werden nicht geändert. Die Auswahl wird mit der Arbeitsmappe gespeichert.

Das Runden lässt sich als ein Schritt rückgängig machen, und **Rückgängig** stellt die Zahlen wieder her. Wenn Sie **Genauigkeit wie angezeigt festlegen** erneut wählen, wird es ausgeschaltet. Das verhindert nur das Runden von da an. Die Zahlen, die bereits gerundet wurden, bleiben gerundet.

## Anzeigen, was eine Formel liest

Wählen Sie eine Zelle aus und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen; bei jedem Klick geht es einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste zeigt an, wenn es mehr gab. **Pfeile entfernen** löscht sie, ebenso jede Änderung an der Arbeitsmappe.

Zeigt eine Zelle einen Fehler wie #DIV/0!, wählen Sie sie aus und klicken auf **Fehler nachverfolgen**. Es werden Pfeile zu den Zellen gezeichnet, die sie liest und die ebenfalls einen Fehler anzeigen. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. Zeigt keine der gelesenen Zellen einen Fehler, meldet die Statusleiste, dass diese Zelle den Fehler verursacht, und genau diese müssen Sie beheben.

Um alle Fehler des Blatts durchzugehen, klicken Sie auf **Nächster Fehler** oder **Vorheriger Fehler**. Die Zellen mit einem Fehler werden zeilenweise besucht, und nach der letzten beginnt es wieder bei der ersten. Jede Zelle wird ausgewählt, und die Pfeile werden bis zu der Zelle gezeichnet, die den Fehler verursacht. Die Statusleiste zeigt an, um welchen Fehler es sich handelt und wie viele es insgesamt gibt.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Prüfen** der Registerkarte Formeln die Option **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die möglicherweise falsch sind. Sie sucht nach einer Formel, die einen Fehler ergibt (etwa #DIV/0!), nach einer Zahl, die als Text gespeichert ist (die Formeln möglicherweise überspringen), und nach mehr: einer Formel, die anders aufgebaut ist als die Formeln daneben, einem Datum als Text mit einer zweistelligen Jahreszahl, einer Formel in einer Zelle, die nicht gesperrt ist, einer Formel, die eine Zahl neben den verwendeten Zellen auslässt (nur bei einer Funktion, die einen Bereich addiert oder summiert, etwa **SUMME**, **MITTELWERT**, **ANZAHL**, **MIN** oder **MAX**, wenn der Bereich allein als Argument steht), und einer Formel, die eine leere Zelle verwendet.

Das Dialogfeld zeigt jeweils eine Zelle, springt zu ihr auf dem Blatt und beschreibt in einem Satz das Problem. Mit **Weiter** und **Zurück** wechseln Sie zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. Klicken Sie auf das Dreieck selbst, um neben der Zelle ein Menü zu öffnen: Es nennt das Problem, bietet die Korrektur an, wenn es eine gibt, und enthält **Fehler ignorieren** und **Fehlerüberprüfung…**. Kann die Zelle korrigiert werden, erscheint eine Schaltfläche mit der Korrektur (bei einer Formel, die eine Zahl auslässt, **Zelle einschließen**); sie ändert die Formel, und ein Schritt von Rückgängig stellt die alte Formel wieder her. **Fehler ignorieren** nimmt die Zelle von der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt von Rückgängig macht es wieder rückgängig. Ist nichts falsch, wird Ihnen das mitgeteilt. Um festzulegen, welche dieser Fehler markiert werden, öffnen Sie **Optionen** und die Seite **Formeln**: Dort gibt es für jeden ein Kontrollkästchen, und wenn Sie ein Kästchen deaktivieren, wird diese Art nicht mehr markiert. Die Auswahl gilt für das Programm und damit für alle geöffneten Fenster. Eine Zelle kann mehr als ein Problem haben, und **Fehler ignorieren** ignoriert nur das gerade angezeigte.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Wählen Sie eine Zelle mit einer Formel aus und wählen Sie **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert nimmt dessen Platz ein. Die Teile werden in dieser Reihenfolge abgearbeitet: zuerst jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, dann der Rest. **Neu starten** beginnt von vorn. Lässt sich ein Teil nicht berechnen, nennt das Dialogfeld den Grund und lässt die Formel unverändert.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das beim Arbeiten sichtbar bleibt. Es listet Zellen auf, die Sie im Auge behalten möchten, mit Blatt, Zelle, Wert und Formel, auch wenn sie auf einem anderen Blatt liegen. Wählen Sie Zellen aus und drücken Sie **Überwachung hinzufügen**. Wählen Sie Zeilen in der Liste aus und drücken Sie **Überwachung löschen**. Doppelklicken Sie auf eine Zeile, um zur Zelle zu springen. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Formeln, die die Zellen daneben füllen

Liefert eine Formel einen Block von Werten, füllt der Block die Zellen unter und neben der Zelle, in die Sie die Formel eingegeben haben. Man nennt das Überlauf. Versuchen Sie =SEQUENZ(3) in A1: A1 zeigt 1, und A2 und A3 zeigen 2 und 3. Eine Formel läuft über, wenn sie eine Funktion aufruft, die einen Block liefert (SORT, SORTIERENNACH, FILTER, EINDEUTIG, SEQUENZ, ZUFALLSMATRIX, TRANSPONIEREN, XVERWEIS oder MMULT), einen Block in geschweiften Klammern schreibt, einen Überlaufbereich (siehe unten) verwendet oder eine Rechnung mit einem Bereich ausführt, etwa =A1:A3*2.

- **Die Zellen, die er füllt, enthalten keine eigene Formel.** Sie zeigen die Werte. Wählen Sie eine davon aus, zeigt die Bearbeitungsleiste die Formel der Zelle, die sie füllt, in Grau an. Drücken Sie darauf **Eingabe**, ohne etwas zu ändern, passiert nichts. Um den Block zu ändern, ändern Sie zuerst diese erste Zelle.
- **Der Block folgt dem Ergebnis.** Liefert die Formel nach einer Änderung der gelesenen Werte mehr oder weniger Werte, wächst oder schrumpft der Block, und die Formeln, die diese Zellen lesen, werden neu berechnet.
- **Eine Zelle im Weg ergibt #ÜBERLAUF!.** Enthält eine Zelle, in die der Block käme, einen Wert oder eine Formel, zeigt die erste Zelle **#ÜBERLAUF!**, und nichts wird gefüllt. Leeren Sie die Zelle im Weg, dann füllt sich der Block sofort. Eingaben in eine Zelle, die der Block füllt, bewirken dasselbe: Die erste Zelle zeigt #ÜBERLAUF!.
- **Ein Block hat eine Größenbegrenzung.** Ein Block, der größer als das Blatt ist oder mehr als 200.000 Zellen umfasst, ergibt #ÜBERLAUF!.
- **A1# ist der ganze Block.** Schreiben Sie die Zelle, die die Formel enthält, gefolgt von einem **#**, etwa =SUMME(A1#), um den ganzen Block in seinem aktuellen Zustand zu verwenden, auf diesem Blatt oder einem anderen (=SUMME(Tabelle1!A1#)). Läuft A1 nicht über, ist A1# gleich #BEZUG!.

Ein Überlauf wird mit der Datei als dynamisches Array gespeichert, sodass das Programm, aus dem die Datei stammt, ihn ebenfalls als Überlauf anzeigt. Beim Speichern im älteren Format .xls oder .ods bleiben die Werte erhalten, und die Formel wird dort, wo das ältere Format die verwendeten Funktionen hat, als Matrixformel über dem Block geschrieben. Die Kompatibilitätsprüfung nennt diese Formeln.

## Werten in einer Formel Namen geben

**LET** benennt einen Wert mitten in einer Formel, sodass Sie ihn einmal schreiben und danach wieder verwenden können. Die Paare aus Name und Wert kommen zuerst, die Berechnung zuletzt. In =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beides. Ein Name kann die Namen davor verwenden, und ein LET kann bis zu 126 Paare enthalten. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf einen Namen erneut verwenden. Ein Name kann auch für einen Zellbereich stehen, etwa =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Eine Zelle oder die Arbeitsmappe abfragen

**ZELLE (CELL)** liefert Angaben über eine Zelle. Das erste Argument legt fest, welche: "adresse", "zeile", "spalte", "inhalt", "typ", "format" (wie Zahlen angezeigt werden, etwa "F2" für zwei Dezimalstellen, "P0" für einen Prozentwert oder "D1" für ein Datum), "farbe" (1, wenn negative Zahlen farbig angezeigt werden), "klammern" (1, wenn positive Zahlen in Klammern angezeigt werden), "präfix" (das Zeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), "schutz" (1, wenn die Zelle gesperrt ist), "breite" (die Breite der Spalte in Zeichen) und "dateiname" (Ordner, Datei und Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, bis die Arbeitsmappe gespeichert ist). **INFO** liefert Angaben über das Programm und die Arbeitsmappe: "verzeichnis", "numdatei" (wie viele Blätter angezeigt werden), "origin", "osversion", "recalc" (Automatisch oder Manuell), "release" und "system".

## Werte aus XML herauslesen

**FILTERXML** nimmt einen Text, der XML enthält, und einen XPath, eine kurze Schreibweise dafür, welche Teile Sie möchten, und liefert die gefundenen Werte. =FILTERXML(A1; "//artikel/preis") liefert jeden Preis im Text in A1, untereinander. Verwenden Sie "/laden/artikel[2]/name" für den zweiten Artikel, "//artikel[@id='b']/name" für den Artikel mit der id b oder "count(//artikel)" für die Anzahl. Ist der Text kein gültiges XML oder der Pfad kein gültiger Pfad, ist das Ergebnis #WERT!. Gibt es keine Treffer, ist es #NV. XML mit einem Dokumenttyp wird abgelehnt.
