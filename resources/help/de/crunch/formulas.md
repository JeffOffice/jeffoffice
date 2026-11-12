# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Überlauf, übergelaufen, #ÜBERLAUF!, dynamische Matrix, Überlaufbereich, A1#, nächster Fehler, vorheriger Fehler, Fehler durchgehen, Fehler nachverfolgen, vor dem Speichern berechnen, Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Zirkelbezug, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, Klicken auf Zelle, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Wert benennen, Zellfunktion, Informationsfunktion, FILTERXML, XML, XPath-->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei für jede Formel den zuletzt berechneten Wert, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch alle Formeln erneut, jeweils einige wenige, damit das Fenster flüssig bleibt. Während dieser Berechnung zeigt die Statusleiste **Berechnung läuft** mit einem Prozentwert an. Ist die Berechnung fertig, enthalten die Zellen JeffCrunch-eigene Ergebnisse. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alle neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es kommen laufend neue hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei dafür gespeichert hat. Das Gleiche gilt für Formeln, die auf eine andere Arbeitsmappe verweisen. Zellen, die von diesen Formeln abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen mit einigen Hinweisen

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen Tage so, wie es die **Basis** angibt, die Sie angeben (0 bedeutet 30 Tage pro Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage und so weiter). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS ermitteln ihr Ergebnis, indem sie so lange Werte ausprobieren, bis Kurs oder Rate passt. Sie brechen ab, wenn das Ergebnis auf etwa 15 Stellen genau ist, daher können die letzten Stellen von denen anderer Programme abweichen.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiter funktionieren. Die *.INV-Funktionen werden ebenfalls durch Ausprobieren von Werten ermittelt.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINVERS und DATENSCHÄTZUNG liefern einen Block von Werten. Geben Sie eine solche Funktion in einer einzelnen Zelle ein, dann läuft der Block in die Zellen darunter und daneben über (siehe **Formeln, die die Zellen daneben füllen**), oder Sie geben sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen soll.
- **Texte in anderen Schriften.** LÄNGEB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eines, so wie auf einem Computer, der nicht für Doppelbyte-Text eingerichtet ist. PHONETIC gibt den Text der Zelle aus, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **UMRECHNEN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion angegeben: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eines annimmt, etwa „km“ oder „mL“.

## MOD: der Rest

**MOD(n; d)** gibt an, was von **n** übrig bleibt, nachdem so viele ganze Vielfache von **d** wie möglich abgezogen wurden: Es ist n - d * GANZZAHL(n / d). Beispiel: **=MOD(10; 3)** ergibt 1, und **=MOD(-10; 3)** ergibt 2, weil GANZZAHL(-10 / 3) gleich -4 ist und -10 - 3 * -4 gleich 2 ergibt. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, ist das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+#** (auf dem Mac die Ctrl-Taste, nicht die Befehlstaste), um statt der Werte in jeder Zelle die Formel zu sehen. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite; die Spaltenbreiten in Ihrer Datei werden nicht geändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken Sie **F2**, bearbeiten die Formel und drücken **Eingabe**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen bearbeiten](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, jeweils mit einem Satz Erläuterung. Drücken Sie **Tab**, um die markierte zu übernehmen. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere, dann **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion sind, werden ihre Argumente unter der Zelle angezeigt, das gerade eingegebene fett. Liste und Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt, in Sprachen mit Komma als Dezimalzeichen durch Semikolons.

## Auf Zellen zeigen

Statt die Adresse einer Zelle zu tippen, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor dorthin, wo eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder einem Komma), und klicken auf die Zelle. Ihre Adresse wird für Sie eingetragen. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die zuletzt eingetragene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A zu schreiben, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 zu schreiben. Ziehen Sie über die Buchstaben oder Zahlen, um B:D oder 2:5 zu erhalten.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, bewegt sich ein Zeiger von der Zelle weg, in der Sie gerade schreiben, und seine Adresse wird eingetragen. Drücken Sie weitere Pfeile, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, solange die Pfeiltasten zeigen, **Eingabe**, während Sie schreiben, und **Bearbeiten**, wenn die Pfeiltasten im Text den Cursor bewegen. Drücken Sie **F2**, um zwischen Zeigen und Bearbeiten des Textes umzuschalten.

Um auf eine Zelle eines anderen Blatts zu zeigen, klicken Sie auf den Blattreiter dieses Blatts, während der Cursor dort steht, wo eine Zelle stehen kann, und klicken dann auf die Zelle. Der Blattname wird mit geschrieben, bei Bedarf in Anführungszeichen, zum Beispiel als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zum Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rand eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder an seinem Eckquadrat ziehen, um ihn zu vergrößern; die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in oder direkt hinter einer Adresse steht, um Teile davon mit einem Dollarzeichen zu fixieren: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und schließlich wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden auf dem Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick hinter einer vollständigen Adresse, etwa =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, was eine Formel für viele Werte einer oder zweier Zellen ergibt. Öffnen Sie eine Arbeitsmappe, die eine solche Tabelle enthält, dann berechnet JeffCrunch ihre Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrecht ausprobierten Werte in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte senkrecht in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und aller Formeln, die von der Eingabezelle abhängen, wobei die Eingabe auf diesen Wert gesetzt ist. Sonst bleibt die Arbeitsmappe unverändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die auf Zellen einer Tabelle verweisen, folgen. Das Anlegen einer neuen Datentabelle wird noch nicht angeboten. Eine Tabelle, die nicht berechnet werden kann (weil eine Formel von der Eingabe eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die ihre Datei enthält.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsmodus**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, außer bei Datentabellen** tut dasselbe, lässt aber die Zellen von Datentabellen unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Die Statusleiste zeigt nach einer Änderung **Veraltet** an. Drücken Sie **F9** (**Alle neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Dieses Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere Wartende zu berechnen. Eine Arbeitsmappe, die auf Manuell eingestellt ist, wird mit den Ergebnissen geöffnet, die ihre Datei enthält. **Vor dem Speichern berechnen** steht ebenfalls im Menü **Berechnungsmodus** und ist zunächst aktiviert: Wenn Sie eine manuell berechnete, veraltete Arbeitsmappe speichern, werden die Formeln vorher berechnet, damit die Datei aktuelle Zahlen enthält. Schalten Sie es aus, um die Zahlen so zu speichern, wie sie sind.

Die Wahl wird mit der Arbeitsmappe gespeichert, und sie lässt sich rückgängig machen.

## Formeln, die sich selbst lesen

Eine Formel, die direkt oder über andere Zellen ihr eigenes Ergebnis liest, lässt sich nicht auf die übliche Weise berechnen. JeffCrunch meldet das einmal, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten ihre bisherigen Werte. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie in der Liste auf eine Zelle, um dorthin zu springen.

Manche Modelle brauchen einen solchen Kreis absichtlich. Wählen Sie im Menü **Berechnungsmodus** den Eintrag **Iterative Berechnung aktivieren**, dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr als um den angegebenen Betrag ändert oder die erlaubte Anzahl an Durchgängen erreicht ist.

## Herausfinden, was eine Formel liest

Markieren Sie eine Zelle und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Ein weiterer Klick geht einen Schritt weiter zurück. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet; die Statusleiste meldet, wenn es mehr gewesen wären. **Pfeile entfernen** löscht sie, und jede Änderung an der Arbeitsmappe ebenfalls.

Zeigt eine Zelle einen Fehler wie #DIV/0!, markieren Sie sie und klicken auf **Fehler nachverfolgen**. Es werden Pfeile zu den Zellen gezeichnet, die sie liest und die ebenfalls einen Fehler zeigen. Ein weiterer Klick geht einen Schritt weiter zurück. Zeigt keine der gelesenen Zellen einen Fehler, meldet die Statusleiste, dass diese Zelle den Fehler verursacht, und sie ist die, die Sie korrigieren sollten.

Um alle Fehler des Blatts der Reihe nach durchzugehen, klicken Sie auf **Nächster Fehler** oder **Vorheriger Fehler**. Die Zellen, die einen Fehler zeigen, werden Zeile für Zeile besucht, und nach der letzten beginnt es wieder bei der ersten. Jede Zelle wird markiert, und die Pfeile werden bis zu der Zelle gezeichnet, die den Fehler verursacht. Die Statusleiste zeigt, um welchen Fehler es sich handelt und wie viele es gibt.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Prüfen** der Registerkarte Formeln die Option **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die falsch sein könnten. Sie sucht nach zwei Dingen: einer Formel, die einen Fehler ergibt (etwa #DIV/0!), und einer Zahl, die als Text gespeichert ist und von Formeln übersprungen werden könnte.

Das Dialogfeld zeigt jeweils eine Zelle, springt zu dieser Zelle auf dem Blatt und erklärt in einem Satz, was nicht stimmt. **Weiter** und **Zurück** wechseln zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. **Fehler ignorieren** nimmt die Zelle aus der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt Rückgängig holt sie zurück. Ist nichts falsch, wird Ihnen das mitgeteilt.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Markieren Sie eine Zelle mit einer Formel und wählen Sie **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert ersetzt ihn. Die Teile werden in dieser Reihenfolge abgearbeitet: zuerst jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, und zuletzt der Rest. **Neu starten** beginnt von vorn. Kann ein Teil nicht berechnet werden, nennt das Dialogfeld den Grund und lässt die Formel unverändert.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das während der Arbeit sichtbar bleibt. Es listet Zellen auf, die Sie im Blick behalten möchten, mit ihrem Blatt, ihrer Zelle, ihrem Wert und ihrer Formel, auch wenn sie auf einem anderen Blatt liegen. Markieren Sie Zellen und drücken Sie **Überwachung hinzufügen**. Markieren Sie Zeilen in der Liste und drücken Sie **Überwachung löschen**. Doppelklicken Sie auf eine Zeile, um zur Zelle zu springen. Die Werte ändern sich, wenn das Blatt erneut berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Formeln, die die Zellen daneben füllen

Liefert eine Formel einen Block von Werten, füllt dieser Block die Zellen unter und neben der Zelle, in die Sie die Formel eingegeben haben. Man nennt das einen Überlauf. Probieren Sie =FOLGE(3) in A1: A1 zeigt 1, und A2 und A3 zeigen 2 und 3. Eine Formel läuft über, wenn sie eine Funktion aufruft, die einen Block liefert (SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, MTRANS, XVERWEIS oder MMULT), einen Block in geschweiften Klammern schreibt, einen Überlaufbereich (siehe unten) verwendet oder eine Rechnung mit einem Bereich ausführt, etwa =A1:A3*2.

- **Die Zellen, die sie füllt, enthalten keine eigene Formel.** Sie zeigen die Werte. Wenn Sie eine davon markieren, zeigt die Bearbeitungsleiste die Formel der Zelle an, die sie füllt, in Grau. Wird darin unverändert mit **Eingabe** bestätigt, ändert sich nichts. Um den Block zu ändern, ändern Sie zuerst diese erste Zelle.
- **Der Block folgt dem Ergebnis.** Liefert die Formel nach einer Änderung der gelesenen Werte mehr oder weniger Werte, wird der Block größer oder kleiner, und die Formeln, die diese Zellen lesen, werden erneut berechnet.
- **Eine Zelle im Weg ergibt #ÜBERLAUF!.** Enthält eine Zelle, in die der Block käme, einen Wert oder eine Formel, zeigt die erste Zelle **#ÜBERLAUF!**, und nichts wird gefüllt. Leeren Sie die Zelle im Weg, dann füllt der Block sofort. Wenn Sie in eine Zelle schreiben, die der Block füllt, passiert dasselbe: Die erste Zelle zeigt #ÜBERLAUF!.
- **Ein Block hat eine Größenbegrenzung.** Ein Block, der größer als das Blatt ist oder mehr als 200.000 Zellen umfasst, ergibt #ÜBERLAUF!.
- **A1# ist der ganze Block.** Schreiben Sie die Zelle mit der Formel und ein **#**, etwa =SUMME(A1#), um den ganzen Block in seinem aktuellen Zustand zu verwenden, auf diesem Blatt oder einem anderen (=SUMME(Tabelle1!A1#)). Läuft A1 nicht über, ist A1# gleich #BEZUG!.

Ein Überlauf wird mit der Datei als dynamische Matrix gespeichert, daher zeigt ihn auch das Programm, aus dem die Datei stammt, als Überlauf an. Beim Speichern im älteren Format .xls oder .ods bleiben die Werte erhalten, und die Formel wird dort, wo das ältere Format die verwendeten Funktionen kennt, als Matrixformel über den Block geschrieben. **Kompatibilität prüfen** nennt diese Formeln.

## Werten Namen geben in einer Formel

**LET** gibt einem Wert mitten in einer Formel einen Namen, damit Sie ihn einmal schreiben und danach wieder verwenden können. Die Paare aus Name und Wert kommen zuerst, die Berechnung zuletzt. In =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beides. Ein Name kann die Namen davor verwenden, und ein LET kann bis zu 126 Paare enthalten. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf denselben Namen wieder verwenden. Ein Name kann auch für einen Zellbereich stehen, etwa in =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Angaben zu einer Zelle oder zur Arbeitsmappe abfragen

**ZELLE** liefert Angaben zu einer Zelle. Ihr erstes Argument legt fest, welche: „adresse“, „zeile“, „spalte“, „inhalt“, „typ“, „format“ (wie Zahlen angezeigt werden, etwa „F2“ für zwei Dezimalstellen, „P0“ für einen Prozentwert oder „D1“ für ein Datum), „farbe“ (1, wenn negative Zahlen farbig angezeigt werden), „klammern“ (1, wenn positive Zahlen in Klammern stehen), „präfix“ (das Zeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), „schutz“ (1, wenn die Zelle gesperrt ist), „breite“ (die Breite der Spalte in Zeichen) und „dateiname“ (der Ordner, die Datei und das Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, solange die Arbeitsmappe nicht gespeichert ist). **INFO** liefert Angaben zum Programm und zur Arbeitsmappe: „verzeichnis“, „numdatei“ (wie viele Blätter sichtbar sind), „ursprung“, „betriebssystem“, „neuberechnen“ (Automatisch oder Manuell), „version“ und „system“.

## Werte aus XML auslesen

**XMLFILTERN** nimmt einen Text, der XML enthält, und einen XPath, eine kurze Schreibweise dafür, welche Teile Sie möchten, und liefert die gefundenen Werte. =XMLFILTERN(A1; "//artikel/preis") liefert jeden Preis im Text in A1, untereinander. Mit "/laden/artikel[2]/name" erhalten Sie den zweiten Artikel, mit "//artikel[@id='b']/name" den Artikel mit der id b, und mit "count(//artikel)" die Anzahl. Ist der Text kein gültiges XML oder der Pfad keiner, lautet das Ergebnis #WERT!. Trifft nichts zu, ist es #NV. XML mit einer Dokumenttypdefinition wird abgelehnt.
