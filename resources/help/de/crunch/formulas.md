# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Überlauf, #ÜBERLAUF!, übergelaufen, dynamische Matrix, Überlaufbereich, A1#, nächster Fehler, vorheriger Fehler, Fehler durchgehen, Fehler nachverfolgen, vor dem Speichern berechnen, Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Zirkelbezug, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, Zelle anklicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Name für Wert, Zellfunktion, Informationsfunktion, FILTERXML, XML, XPath, Genauigkeit wie angezeigt, runden, Rundung, Dezimalstellen, Stellen-->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, jeweils einige wenige, damit das Fenster flüssig bleibt. Die Statusleiste zeigt dabei **Berechnung** mit einer Prozentangabe an. Ist die Berechnung abgeschlossen, enthalten die Zellen die Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alle neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es kommen laufend weitere hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei dafür gespeichert hat. Das gilt auch für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von diesen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen mit einigen Hinweisen

- **Anleihen und Darlehen.** PRICE, YIELD, DURATION, ODDFPRICE und die anderen Anleihefunktionen zählen die Tage so, wie es die **Basis** angibt, die Sie eingeben (0 bedeutet 30 Tage pro Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage usw.). YIELD, ODDFYIELD, ODDLYIELD und RATE ermitteln ihr Ergebnis, indem sie so lange Werte ausprobieren, bis Kurs bzw. Zahlung passt. Sie hören auf, wenn das Ergebnis auf etwa 15 Stellen genau ist, daher können die letzten Stellen von denen eines anderen Programms abweichen.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiterhin funktionieren. Auch die Funktionen *.INV werden durch Ausprobieren von Werten ermittelt.
- **Listen, die mehrere Zellen füllen.** SORT, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINV und RGP liefern einen Block von Werten. Geben Sie eine solche Formel in einer einzelnen Zelle ein, dann läuft der Block in die Zellen darunter und daneben über (siehe **Formeln, die die Zellen daneben füllen**). Alternativ geben Sie sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen soll.
- **Texte in anderen Schriften.** LÄNGEB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eines, so wie es auf einem Computer ohne Einstellung für Doppelbytezeichen geschieht. PHONETIC gibt den Text der Zelle aus, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **CONVERT.** Die Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eines verträgt, etwa „km“ oder „mL“.

## MOD: der Rest

**MOD(n; d)** gibt an, was von **n** übrig bleibt, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Das Ergebnis ist n - d * GANZZAHL(n / d). Beispiel: **=REST(10;3)** ergibt 1, und **=REST(-10;3)** ergibt 2, denn GANZZAHL(-10 / 3) ist -4, und -10 - 3 * -4 ergibt 2. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+^** (auf dem Mac die Control-Taste, nicht die Befehlstaste). Anstelle der Werte sehen Sie dann in jeder Zelle die Formel. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite, und die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, wählen Sie ihre Zelle aus, drücken **F2**, bearbeiten die Formel und drücken **Eingabe**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen bearbeiten](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, jeweils mit einem Satz Erläuterung. Mit **Tab** übernehmen Sie die markierte Funktion. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere und drücken dann **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion stehen, werden ihre Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett erscheint. Die Liste und die Argumente sind dieselben wie bei **Funktion auswählen**. Argumente werden durch Kommas getrennt, bei Sprachen, die ein Komma als Dezimaltrennzeichen verwenden, durch Semikolons.

## Funktionsnamen in Ihrer Sprache

Wenn JeffCrunch auf Deutsch, Französisch, Spanisch, Italienisch, Niederländisch, Portugiesisch oder Russisch angezeigt wird, können Sie eine Formel mit den Funktionsnamen schreiben, die Ihre Sprache verwendet, zum Beispiel `=SUMME(A1;A2)` auf Deutsch oder `=SOMME(A1;A2)` auf Französisch. Die englischen Namen funktionieren ebenfalls, sodass eine Formel, die Sie aus dem Web kopieren, nicht geändert werden muss. Die Bearbeitungsleiste, die Zelle, die Funktionsliste und **Funktion auswählen** zeigen die Namen Ihrer Sprache an.

Die Arbeitsmappe ändert sich nicht, wenn Sie die Sprache wechseln. Eine Datei speichert immer die englischen Namen, sodass eine in einer Sprache erstellte Arbeitsmappe in einer anderen Sprache und in jedem anderen Programm richtig geöffnet und gelesen wird. Landessprachliche Namen gibt es für Deutsch, Französisch, Spanisch, Italienisch, Niederländisch, Portugiesisch (Brasilien und Portugal) und Russisch; die englischen Namen funktionieren in jeder Sprache. Ukrainische und indonesische Programme behalten für fast alle Funktionen die englischen Namen. Hat eine Funktion in einer Sprache zwei gebräuchliche Namen, werden beide gelesen, und der gebräuchlichere wird angezeigt. Eine Funktion, die in Ihrer Sprache keinen üblichen Namen hat, behält den englischen. WAHR und FALSCH richten sich ebenfalls nach Ihrer Sprache. Die Fehlerwerte Ihrer Sprache, etwa `#NV` auf Deutsch, werden beim Eingeben einer Formel gelesen und in der Bearbeitungsleiste angezeigt. Eine Zelle mit einem Fehler zeigt aber immer die Standardform, etwa `#NV/A`.

Verwendet die Regionaleinstellung Ihres Computers das Komma als Dezimaltrennzeichen, werden die Argumente einer Funktion durch ein Semikolon getrennt. Eine Matrix in geschweiften Klammern mit mehr als einer Zeile wird mit einem Semikolon zwischen den Werten einer Zeile und einem Backslash zwischen den Zeilen angezeigt, etwa `{1;2\3;4}`, und Sie können sie so eingeben. Eine Matrix, die ohne Backslash eingegeben wird, wird so gelesen, wie eine Datei sie schreibt: Kommas zwischen den Werten einer Zeile, Semikolons zwischen den Zeilen und ein Punkt als Dezimaltrennzeichen.

Wenn Sie nur englische Namen eingeben möchten, klicken Sie auf **Datei > Optionen > Formeln** und aktivieren **Englische Funktionsnamen verwenden**.

## Auf Zellen zeigen

Statt die Adresse einer Zelle einzutippen, können Sie auf die Zelle zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor dorthin, wo eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder nach einem Komma), und klicken auf die Zelle. Ihre Adresse wird für Sie geschrieben. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie noch einmal, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die zuvor geschriebene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A zu schreiben, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 zu schreiben. Ziehen Sie über die Buchstaben oder Zahlen, um B:D oder 2:5 zu schreiben.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, bewegt sich ein Zeiger von der Zelle weg, in der Sie gerade eingeben, und seine Adresse wird geschrieben. Drücken Sie weitere Pfeiltasten, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe**, während Sie eingeben, und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Mit **F2** wechseln Sie zwischen Zeigen und Bearbeiten des Textes.

Um auf eine Zelle eines anderen Blatts zu zeigen, klicken Sie auf den Reiter dieses Blatts, während der Cursor an einer Stelle steht, wo eine Zelle stehen kann, und klicken dann auf die Zelle. Der Name des Blatts wird mit geschrieben, bei Bedarf in Anführungszeichen, etwa als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zu dem Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rand eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder an dem Eckquadrat ziehen, um sie zu vergrößern. Die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in einer Adresse oder direkt dahinter steht, um Teile davon mit einem Dollarzeichen festzulegen: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und dann wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden im Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick hinter einer vollständigen Adresse wie =A1 beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, welches Ergebnis eine Formel für viele Werte einer oder zweier Zellen hat. Öffnen Sie eine Arbeitsmappe mit einer Datentabelle, berechnet JeffCrunch deren Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht eingesetzt werden, kommen in die Zeileneingabezelle, und die senkrecht eingesetzten Werte in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und aller Formeln, die von der Eingabezelle abhängen, wobei die Eingabe auf diesen Wert gesetzt ist. Sonst wird nichts in der Arbeitsmappe verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die die Zellen einer Tabelle lesen, folgen. Das Anlegen einer neuen Datentabelle wird noch nicht angeboten. Eine Tabelle, die nicht berechnet werden kann (weil eine Formel, die von der Eingabe abhängt, eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsart**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, Tabellen manuell** tut dasselbe, lässt die Zellen von Datentabellen aber so, wie sie sind, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, Datentabellen eingeschlossen, bis Sie es verlangen. Nach einer Änderung zeigt die Statusleiste **Veraltet**. Drücken Sie **F9** (**Alle neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Dieses Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere Ausstehende zu berechnen. Eine Arbeitsmappe im Modus Manuell öffnet sich mit den Ergebnissen, die ihre Datei enthält. **Vor dem Speichern berechnen** im Menü **Berechnungsart** ist anfangs aktiviert: Wenn Sie eine manuelle Arbeitsmappe speichern, die veraltet ist, werden die Formeln zuerst berechnet, sodass die Datei aktuelle Zahlen enthält. Deaktivieren Sie die Option, um die Zahlen so zu speichern, wie sie sind.

Die Einstellung wird mit der Arbeitsmappe gespeichert, und sie lässt sich rückgängig machen.

## Formeln, die sich selbst lesen

Eine Formel, die direkt oder über andere Zellen ihr eigenes Ergebnis liest, lässt sich nicht auf die übliche Weise berechnen. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie auf dem Blatt. Sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie auf eine Zelle darin, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkel absichtlich. Wählen Sie im Menü **Berechnungsart** den Eintrag **Iterative Berechnung aktivieren**. Dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den von Ihnen angegebenen Betrag ändert oder die erlaubte Anzahl an Durchläufen erreicht ist.

## Die Zahlen so verwenden, wie die Zellen sie anzeigen

Eine Zelle kann weniger Stellen anzeigen, als sie enthält. Enthält eine Zelle 2,675 und zeigt 2,68 an, verwenden die Formeln, die sie lesen, weiterhin 2,675. Manchmal soll aber das verwendet werden, was Sie sehen. Wählen Sie dann auf der Registerkarte **Formeln** im Menü **Berechnungsoptionen** den Eintrag **Genauigkeit wie angezeigt festlegen**.

JeffCrunch warnt Sie vorher, denn die verborgenen Stellen gehen verloren. Wählen Sie **Abbrechen**, ändert sich nichts. Wählen Sie **OK**, wird jede Zahl in der Arbeitsmappe auf das gerundet, was ihre Zelle anzeigt. Eine Zelle mit zwei Dezimalstellen behält zwei. Eine Zelle, die einen Prozentwert zeigt, behält den Prozentwert. Eine Zelle mit einem Datum behält den Tag, und eine Zelle mit einer Uhrzeit behält den kleinsten angezeigten Teil der Uhrzeit, etwa die Minute. Eine Zelle, die **Standard** anzeigt, behält alle ihre Stellen. Bei einer großen Arbeitsmappe erscheint während dieses Vorgangs ein Hinweis mit einer Schaltfläche **Abbrechen**. Brechen Sie ab, solange der Hinweis angezeigt wird, dann wird nichts geändert.

Danach wird jede Zahl, die Sie eingeben oder einfügen, und jedes Ergebnis einer Formel auf dieselbe Weise gerundet. Die Formeln selbst werden nicht geändert. Die Einstellung wird mit der Arbeitsmappe gespeichert.

Das Runden lässt sich rückgängig machen, und **Rückgängig** stellt die Zahlen wieder her. Wenn Sie **Genauigkeit wie angezeigt festlegen** erneut wählen, wird die Option ausgeschaltet. Dadurch wird lediglich das weitere Runden beendet. Die Zahlen, die bereits gerundet wurden, bleiben gerundet.

## Anzeigen, was eine Formel liest

Wählen Sie eine Zelle aus und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und bei jedem Klick geht es einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste meldet, wenn es mehr gewesen wären. **Pfeile entfernen** löscht sie, und jede Änderung an der Arbeitsmappe löscht sie ebenfalls.

Zeigt eine Zelle einen Fehler wie #DIV/0!, wählen Sie sie aus und klicken auf **Fehler nachverfolgen**. Es werden Pfeile zu den Zellen gezeichnet, die sie liest und die ebenfalls einen Fehler zeigen. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. Zeigt keine der gelesenen Zellen einen Fehler, meldet die Statusleiste, dass diese Zelle den Fehler verursacht, und das ist die Zelle, die Sie korrigieren sollten.

Um alle Fehler des Blatts der Reihe nach durchzugehen, klicken Sie auf **Nächster Fehler** oder **Vorheriger Fehler**. Die Zellen mit einem Fehler werden zeilenweise besucht, und nach der letzten beginnt es wieder bei der ersten. Jede Zelle wird ausgewählt, und die Pfeile werden bis zur Zelle gezeichnet, die den Fehler verursacht. Die Statusleiste zeigt an, um welchen Fehler es sich handelt und wie viele es insgesamt gibt.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Überprüfen** der Registerkarte Formeln die Option **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die möglicherweise falsch sind. Sie sucht nach zwei Dingen: einer Formel, die einen Fehler ergibt (etwa #DIV/0!), und einer Zahl, die als Text gespeichert ist und die Formeln möglicherweise überspringen.

Das Dialogfeld zeigt jeweils eine Zelle, wechselt auf dem Blatt zu dieser Zelle und erklärt in einem Satz, was nicht stimmt. Mit **Weiter** und **Zurück** wechseln Sie zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. **Fehler ignorieren** nimmt die Zelle von der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt Rückgängig holt sie zurück. Ist nichts falsch, wird Ihnen das mitgeteilt.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Wählen Sie eine Zelle mit einer Formel aus und klicken Sie auf **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Mit **Auswerten** berechnen Sie diesen Teil; sein Wert ersetzt ihn. Die Teile werden in dieser Reihenfolge bearbeitet: zuerst die einzelnen Zellen von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, und danach der Rest. Mit **Neu starten** beginnen Sie von vorn. Lässt sich ein Teil nicht berechnen, nennt das Dialogfeld den Grund und lässt die Formel unverändert.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das beim Arbeiten sichtbar bleibt. Es listet Zellen auf, die Sie im Blick behalten möchten, mit ihrem Blatt, ihrer Zelle, ihrem Wert und ihrer Formel, auch wenn sie auf einem anderen Blatt liegen. Wählen Sie Zellen aus und drücken Sie **Überwachung hinzufügen**. Wählen Sie Zeilen in der Liste aus und drücken Sie **Überwachung löschen**. Ein Doppelklick auf eine Zeile bringt Sie zur Zelle. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Formeln, die die Zellen daneben füllen

Gibt eine Formel einen Block von Werten zurück, füllt der Block die Zellen unter und neben der Zelle, in die Sie die Formel eingegeben haben. Man nennt das einen Überlauf. Versuchen Sie =FOLGE(3) in A1: A1 zeigt 1, und A2 und A3 zeigen 2 und 3. Eine Formel läuft über, wenn sie eine Funktion aufruft, die einen Block liefert (SORT, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, TRANSPONIEREN, XVERWEIS oder MMULT), einen Block in geschweifte Klammern schreibt, einen Überlaufbereich (siehe unten) verwendet oder eine Rechenoperation auf einen Bereich anwendet, etwa in =A1:A3*2.

- **Die Zellen, die der Block füllt, enthalten keine eigene Formel.** Sie zeigen nur die Werte. Wählen Sie eine davon aus, zeigt die Bearbeitungsleiste die Formel der Zelle an, die sie füllt, in Grau. Drücken Sie darin auf **Eingabe**, ohne etwas zu ändern, geschieht nichts. Um den Block zu ändern, ändern Sie zuerst diese erste Zelle.
- **Der Block folgt dem Ergebnis.** Liefert die Formel nach einer Änderung der gelesenen Werte mehr oder weniger Werte, wird der Block größer oder kleiner, und die Formeln, die diese Zellen lesen, werden neu berechnet.
- **Eine Zelle im Weg ergibt #ÜBERLAUF!.** Enthält eine Zelle, in die der Block käme, einen Wert oder eine Formel, zeigt die erste Zelle **#ÜBERLAUF!**, und nichts wird gefüllt. Leeren Sie die Zelle im Weg, dann füllt sich der Block sofort. Schreiben Sie in eine Zelle, die der Block füllt, geschieht dasselbe: Die erste Zelle zeigt #ÜBERLAUF!.
- **Ein Block hat eine Größenbegrenzung.** Ein Block, der größer als das Blatt ist oder mehr als 200.000 Zellen umfasst, ergibt #ÜBERLAUF!.
- **A1# ist der ganze Block.** Schreiben Sie die Zelle mit der Formel und ein **#**, etwa in =SUMME(A1#), um den ganzen Block in seinem aktuellen Zustand zu verwenden, auf diesem Blatt oder einem anderen (=SUMME(Tabelle1!A1#)). Überläuft A1 nicht, ergibt A1# den Fehler #BEZUG!.

Ein Überlauf wird mit der Datei als dynamische Matrix gespeichert, sodass das Programm, aus dem die Datei stammt, ihn ebenfalls als Überlauf anzeigt. Beim Speichern im älteren Format .xls oder .ods bleiben die Werte erhalten, und die Formel wird als Matrixformel über den Block geschrieben, sofern das ältere Format die verwendeten Funktionen kennt. **Kompatibilität prüfen** führt diese Formeln auf.

## Namen für Werte innerhalb einer Formel

**LET** gibt einem Wert mitten in einer Formel einen Namen, sodass Sie ihn einmal schreiben und danach wieder verwenden können. Zuerst kommen die Paare aus Name und Wert, zuletzt die Berechnung. In =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beide. Ein Name darf die Namen davor verwenden, und ein LET kann bis zu 126 Paare enthalten. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf denselben Namen wieder verwenden. Ein Name kann auch für einen Zellbereich stehen, etwa in =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Nach einer Zelle oder der Arbeitsmappe fragen

**ZELLE** liefert Angaben zu einer Zelle. Ihr erstes Argument legt fest, welche: „adresse“, „zeile“, „spalte“, „inhalt“, „typ“, „format“ (wie Zahlen angezeigt werden, etwa „F2“ für zwei Dezimalstellen, „P0“ für einen Prozentwert oder „D1“ für ein Datum), „farbe“ (1, wenn negative Zahlen farbig angezeigt werden), „klammern“ (1, wenn positive Zahlen in Klammern stehen), „präfix“ (das Zeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), „schutz“ (1, wenn die Zelle gesperrt ist), „breite“ (die Breite der Spalte in Zeichen) und „dateiname“ (der Ordner, die Datei und das Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, solange die Arbeitsmappe nicht gespeichert ist). **INFO** liefert Angaben zum Programm und zur Arbeitsmappe: „verzeichnis“, „numdatei“ (wie viele Blätter angezeigt werden), „ursprung“, „osversion“, „neuberechnen“ (Automatisch oder Manuell), „version“ und „system“.

## Werte aus XML auslesen

**FILTERXML** nimmt einen Text mit XML und einen XPath, eine kurze Angabe, welche Teile Sie möchten, und liefert die gefundenen Werte. =FILTERXML(A1; "//item/price") liefert jeden Preis im Text in A1, untereinander. Mit "/shop/item[2]/name" erhalten Sie den Namen des zweiten Artikels, mit "//item[@id='b']/name" den Namen des Artikels, dessen ID b ist, und mit "count(//item)" die Anzahl der Artikel. Ist der Text kein gültiges XML oder der Pfad keiner, lautet das Ergebnis #WERT!. Findet sich nichts, lautet es #NV. XML mit einer Dokumenttypdefinition wird abgelehnt.
