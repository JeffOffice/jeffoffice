# Wie Formeln angezeigt und berechnet werden
<!-- keywords: Überlauf, übergelaufen, #ÜBERLAUF!, dynamische Matrix, Überlaufbereich, A1#, nächster Fehler, vorheriger Fehler, Fehler durchgehen, Fehler verfolgen, vor dem Speichern berechnen, Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, auf Zelle klicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Namen für einen Wert vergeben, CELL-Funktion, INFO-Funktion, FILTERXML, XML, XPath, Genauigkeit wie angezeigt, runden, Rundung, Dezimalstellen, Stellen-->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen berechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei den zuletzt berechneten Wert jeder Formel, sodass das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, jeweils einige wenige, damit das Fenster flüssig bleibt. Die Statusleiste zeigt dabei **Berechnung** mit einem Prozentwert an. Ist die Berechnung abgeschlossen, enthalten die Zellen die Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alle neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es werden laufend mehr. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei für sie gespeichert hat. Das gilt auch für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die von diesen abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen, zu denen einige Hinweise gehören

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen die Tage so, wie es die **Basis** angibt, die Sie angeben (0 bedeutet 30 Tage pro Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage usw.). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS finden ihr Ergebnis, indem sie so lange Werte ausprobieren, bis Kurs oder Rate passen. Sie brechen ab, wenn das Ergebnis auf etwa 15 Stellen genau ist, deshalb können sich die letzten Stellen von denen eines anderen Programms unterscheiden.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT, liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiterhin funktionieren. Die *.INV-Funktionen werden ebenfalls durch Ausprobieren von Werten gefunden.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINV und RGP liefern einen Block von Werten. Geben Sie eine solche Funktion in einer einzelnen Zelle ein, dann füllt der Block die Zellen darunter und daneben (siehe **Formeln, die die Zellen daneben füllen**), oder Sie geben sie als Matrixformel ein (**Strg+Umschalt+Eingabe**) über den Zellen, die sie füllen soll.
- **Texte in anderen Schriftsystemen.** LÄNGEB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eins, wie es auf einem Computer ohne Einstellung für Doppelbytezeichen geschieht. PHONETIC liefert den Text der Zelle, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **UMWANDELN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion angegeben: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eins annimmt, etwa „km“ oder „ml“.

## MOD: der Rest

**REST(n; d)** liefert den Rest von **n**, nachdem so viele ganze Vielfache von **d** abgezogen wurden, wie möglich sind: Er ist n - d * GANZZAHL(n / d). Beispiel: **=REST(10; 3)** ergibt 1, und **=REST(-10; 3)** ergibt 2, weil GANZZAHL(-10 / 3) gleich -4 ist und -10 - 3 * -4 gleich 2 ergibt. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** gleich 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+^** (auf dem Mac die Control-Taste, nicht die Befehlstaste), um anstelle des Werts jeder Zelle deren Formel zu sehen. Die Spalten, die Formeln enthalten, werden bis zu einer Grenze verbreitert, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite, und die Spaltenbreiten in Ihrer Datei bleiben unverändert.

## Formeln ändern

Um eine Formel zu ändern, wählen Sie ihre Zelle aus, drücken Sie **F2**, bearbeiten Sie die Formel und drücken Sie **Eingabe**. Sie können die Formel auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen bearbeiten](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, jeweils mit einem Satz Erläuterung. Drücken Sie **Tab**, um die markierte Funktion zu übernehmen. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere und übernehmen sie mit **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion sind, werden deren Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett dargestellt wird. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt, bei Sprachen, die das Komma als Dezimaltrennzeichen verwenden, durch Semikolons.

## Auf Zellen zeigen

Statt die Adresse einer Zelle zu tippen, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie die Einfügemarke an eine Stelle, an der eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie eingetragen. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, dann ersetzt die neue Zelle die gerade geschriebene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A zu schreiben, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 zu schreiben, und ziehen Sie über die Buchstaben oder Zahlen, um B:D oder 2:5 zu erhalten.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, und ein Zeiger bewegt sich von der Zelle weg, in die Sie gerade schreiben, und seine Adresse wird eingetragen. Drücken Sie weitere Pfeile, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, solange die Pfeiltasten zeigen, **Eingabe** während Sie schreiben und **Bearbeiten**, wenn die Pfeiltasten die Einfügemarke im Text bewegen. Drücken Sie **F2**, um zwischen Zeigen und Bearbeiten des Textes umzuschalten.

Um auf eine Zelle auf einem anderen Blatt zu zeigen, klicken Sie auf die Registerkarte dieses Blatts, während die Einfügemarke an einer Stelle steht, an der eine Zelle stehen kann, und klicken Sie dann auf die Zelle. Der Name des Blatts wird mit eingetragen, bei Bedarf in Anführungszeichen, etwa als 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zum Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können auch den Rahmen eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder an dessen Eckquadrat ziehen, um ihn zu vergrößern; die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, wenn die Einfügemarke in einer Adresse oder direkt dahinter steht, um Teile davon mit einem Dollarzeichen festzuschreiben: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und schließlich wieder zu A1.

Jede Adresse in der Formel wird im Text farbig dargestellt, und die Zellen, auf die sie verweist, werden auf dem Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick hinter einer vollständigen Adresse, etwa =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Was-wäre-wenn-Tabelle) zeigt, welches Ergebnis eine Formel für viele Werte von einer oder zwei Zellen hat. Öffnen Sie eine Arbeitsmappe, die eine solche Tabelle enthält, dann berechnet JeffCrunch ihre Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links der Tabelle, die auszuprobierenden Werte für die Zeile kommen in die Zeileneingabezelle, und die Werte für die Spalte in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte untereinander in der Spalte links davon (die Formeln stehen in der Zeile darüber) oder nebeneinander in der Zeile darüber (die Formeln stehen in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und jeder Formel, die von der Eingabezelle abhängt, wenn die Eingabe auf diesen Wert gesetzt ist. Nichts anderes in der Arbeitsmappe wird verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die auf die Zellen einer Tabelle verweisen, werden entsprechend nachgezogen. Eine neue Datentabelle zu erstellen, ist noch nicht vorgesehen. Eine Tabelle, die nicht berechnet werden kann (weil eine Formel, die von der Eingabe abhängt, eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich von selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die in ihrer Datei gespeichert sind.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsart**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, Tabellen manuell** tut dasselbe, lässt aber die Zellen von Datentabellen unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Die Statusleiste zeigt nach einer Änderung **Veraltet** an. Drücken Sie **F9** (**Alle neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Dieses Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere, was noch wartet, zu berechnen. Eine auf Manuell eingestellte Arbeitsmappe wird mit den Ergebnissen geöffnet, die ihre Datei enthält. **Vor dem Speichern berechnen** im Menü **Berechnungsart** ist zunächst aktiviert: Wenn Sie eine manuell berechnete Arbeitsmappe speichern, die veraltet ist, werden die Formeln vorher berechnet, damit die Datei aktuelle Zahlen enthält. Deaktivieren Sie die Option, um die Zahlen so zu speichern, wie sie sind.

Die Auswahl wird mit der Arbeitsmappe gespeichert, und sie ist ein Schritt, den Sie rückgängig machen können.

## Formeln, die sich selbst lesen

Eine Formel, die ihr eigenes Ergebnis liest, direkt oder über andere Zellen, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie auf dem Blatt; sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie eine Zelle darin, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkel absichtlich. Wählen Sie im Menü **Berechnungsart** die Option **Iterative Berechnung aktivieren**, dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den von Ihnen angegebenen Betrag ändert oder die erlaubte Anzahl an Durchläufen erreicht ist.

## Die Zahlen so verwenden, wie die Zellen sie anzeigen

Eine Zelle kann weniger Stellen anzeigen, als sie tatsächlich enthält. Wenn eine Zelle 2,675 enthält und 2,68 anzeigt, verwenden die Formeln, die sie lesen, weiterhin 2,675. Manchmal soll aber mit dem gerechnet werden, was zu sehen ist. Wählen Sie dann im Menü **Berechnungsoptionen** auf der Registerkarte **Formeln** die Option **Genauigkeit wie angezeigt festlegen**.

JeffCrunch warnt Sie vorher, denn die verborgenen Stellen gehen verloren. Wählen Sie **Abbrechen**, dann ändert sich nichts. Wählen Sie **OK**, wird jede Zahl in der Arbeitsmappe auf das gerundet, was ihre Zelle anzeigt. Eine Zelle, die zwei Dezimalstellen anzeigt, behält zwei. Eine Zelle, die einen Prozentwert anzeigt, behält den Prozentwert. Eine Zelle mit einem Datum behält den Tag, und eine Zelle mit einer Uhrzeit behält den kleinsten angezeigten Zeitabschnitt, etwa die Minute. Eine Zelle, die **Standard** anzeigt, behält alle ihre Stellen. Bei einer großen Arbeitsmappe wird während dieses Vorgangs ein Hinweis mit einer Schaltfläche **Abbrechen** angezeigt. Brechen Sie ab, solange der Hinweis sichtbar ist, dann wird nichts geändert.

Danach wird jede Zahl, die Sie eingeben oder einfügen, und jedes Ergebnis einer Formel auf die gleiche Weise gerundet. Die Formeln selbst werden nicht verändert. Die Auswahl wird mit der Arbeitsmappe gespeichert.

Das Runden ist ein Schritt, den Sie rückgängig machen können: **Rückgängig** stellt die Zahlen wieder her. Wenn Sie **Genauigkeit wie angezeigt festlegen** erneut wählen, wird das Runden ausgeschaltet. Das verhindert nur das weitere Runden. Die bereits gerundeten Zahlen bleiben gerundet.

## Sehen, was eine Formel liest

Wählen Sie eine Zelle aus und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen; jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste zeigt an, wenn es mehr gewesen wären. **Pfeile entfernen** löscht sie, ebenso jede Änderung an der Arbeitsmappe.

Wenn eine Zelle einen Fehler wie #DIV/0! anzeigt, wählen Sie sie aus und klicken Sie auf **Fehler verfolgen**. Es werden Pfeile zu den Zellen gezeichnet, die sie liest und die ebenfalls einen Fehler anzeigen. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. Wenn keine der gelesenen Zellen einen Fehler anzeigt, meldet die Statusleiste, dass diese Zelle den Fehler verursacht, und sie ist diejenige, die Sie beheben müssen.

Um alle Fehler auf dem Blatt der Reihe nach durchzugehen, klicken Sie auf **Nächster Fehler** oder **Vorheriger Fehler**. Die Zellen mit einem Fehler werden zeilenweise besucht, und nach der letzten beginnt es wieder bei der ersten. Jede Zelle wird ausgewählt, und die Pfeile werden bis zu der Zelle gezeichnet, die den Fehler verursacht. Die Statusleiste zeigt an, welchen Fehler Sie gerade sehen, und wie viele es insgesamt gibt.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Prüfen** der Registerkarte Formeln die Option **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die möglicherweise falsch sind. Sie sucht nach zwei Dingen: einer Formel, die einen Fehler ergibt (etwa #DIV/0!), und einer Zahl, die als Text gespeichert ist und die Formeln möglicherweise übergehen.

Das Dialogfeld zeigt jeweils eine Zelle, springt auf dem Blatt zu dieser Zelle und erklärt in einem Satz, was nicht stimmt. Mit **Weiter** und **Zurück** wechseln Sie zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. **Fehler ignorieren** nimmt die Zelle von der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt **Rückgängig** holt sie zurück. Ist nichts falsch, wird Ihnen das mitgeteilt.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Wählen Sie eine Zelle mit einer Formel aus und wählen Sie **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert ersetzt ihn. Die Teile werden in dieser Reihenfolge abgearbeitet: zuerst jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, und zuletzt der Rest. **Neu starten** beginnt von vorn. Lässt sich ein Teil nicht berechnen, nennt das Dialogfeld den Grund und lässt die Formel unverändert.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das während der Arbeit im Blick bleibt. Es listet Zellen auf, die Sie im Auge behalten möchten, mit Blatt, Zelle, Wert und Formel, auch wenn sie auf einem anderen Blatt liegen. Wählen Sie Zellen aus und drücken Sie **Überwachung hinzufügen**. Wählen Sie Zeilen in der Liste aus und drücken Sie **Überwachung löschen**. Doppelklicken Sie auf eine Zeile, um zur Zelle zu gelangen. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Formeln, die die Zellen daneben füllen

Wenn eine Formel einen Block von Werten liefert, füllt der Block die Zellen unterhalb und neben der Zelle, in die Sie die Formel eingegeben haben. Dies heißt Überlauf. Probieren Sie =FOLGE(3) in A1: A1 zeigt 1, A2 und A3 zeigen 2 und 3. Eine Formel läuft über, wenn sie eine Funktion aufruft, die einen Block liefert (SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, MTRANS, XVERWEIS oder MMULT), einen Block in geschweiften Klammern schreibt, einen Überlaufbereich (siehe unten) verwendet oder eine Rechnung mit einem Bereich ausführt, etwa =A1:A3*2.

- **Die Zellen, die gefüllt werden, enthalten keine eigene Formel.** Sie zeigen nur die Werte. Wenn Sie eine davon auswählen, zeigt die Bearbeitungsleiste in Grau die Formel der Zelle, die sie füllt. Drücken Sie **Eingabe** darauf, ohne etwas zu ändern, dann passiert nichts. Um den Block zu ändern, ändern Sie zuerst diese erste Zelle.
- **Der Block folgt dem Ergebnis.** Wenn die Formel nach einer Änderung in den gelesenen Daten mehr oder weniger Werte liefert, wächst oder schrumpft der Block, und die Formeln, die auf diese Zellen verweisen, werden neu berechnet.
- **Eine Zelle im Weg ergibt #ÜBERLAUF!.** Enthält eine Zelle, in die der Block käme, einen Wert oder eine Formel, zeigt die erste Zelle **#ÜBERLAUF!**, und nichts wird gefüllt. Leeren Sie die Zelle im Weg, dann wird der Block sofort gefüllt. Dasselbe gilt, wenn Sie in eine Zelle schreiben, die der Block füllt: Die erste Zelle zeigt #ÜBERLAUF!.
- **Ein Block hat eine Größenbegrenzung.** Ein Block, der größer als das Blatt ist oder mehr als 200.000 Zellen umfasst, ergibt #ÜBERLAUF!.
- **A1# ist der ganze Block.** Schreiben Sie die Zelle, die die Formel enthält, gefolgt von einem **#**, etwa =SUMME(A1#), um den ganzen Block so zu verwenden, wie er jetzt ist, auf diesem Blatt oder einem anderen (=SUMME(Tabelle1!A1#)). Läuft A1 nicht über, ist A1# gleich #BEZUG!.

Ein Überlauf wird als dynamische Matrix mit der Datei gespeichert, sodass das Programm, mit dem die Datei erstellt wurde, ihn ebenfalls als Überlauf anzeigt. Beim Speichern im älteren Format .xls oder .ods bleiben die Werte erhalten, und die Formel wird dort als Matrixformel über den Block geschrieben, sofern das ältere Format die verwendeten Funktionen kennt. Die Kompatibilitätsprüfung nennt diese Formeln.

## Namen für Werte innerhalb einer Formel vergeben

**LET** gibt einem Wert mitten in einer Formel einen Namen, damit Sie ihn einmal schreiben und danach wieder verwenden können. Die Paare aus Name und Wert stehen zuerst, die Berechnung kommt zuletzt. In =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beide. Ein Name kann die davor stehenden Namen verwenden, und ein LET kann bis zu 126 Paare enthalten. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf denselben Namen erneut verwenden. Ein Name kann auch für einen Zellbereich stehen, etwa in =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Nach einer Zelle oder der Arbeitsmappe fragen

**ZELLE** liefert Angaben über eine Zelle. Das erste Argument legt fest, welche: "adresse", "zeile", "spalte", "inhalt", "typ", "format" (wie Zahlen angezeigt werden, etwa "F2" für zwei Dezimalstellen, "P0" für einen Prozentwert oder "D1" für ein Datum), "farbe" (1, wenn negative Zahlen farbig angezeigt werden), "klammern" (1, wenn positive Zahlen in Klammern stehen), "präfix" (das Kennzeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), "schutz" (1, wenn die Zelle gesperrt ist), "breite" (die Breite der Spalte in Zeichen) und "dateiname" (der Ordner, die Datei und das Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, solange die Arbeitsmappe nicht gespeichert ist). **INFO** liefert Angaben über das Programm und die Arbeitsmappe: "verzeichnis", "numdatei" (wie viele Blätter angezeigt werden), "ursprung", "osversion", "neuberechnung" (Automatisch oder Manuell), "version" und "system".

## Werte aus XML herauslesen

**FILTERXML** nimmt einen Text, der XML enthält, und einen XPath, eine kurze Schreibweise dafür, welche Teile Sie möchten, und liefert die gefundenen Werte. =FILTERXML(A1; "//artikel/preis") liefert jeden Preis im Text in A1, untereinander. Verwenden Sie "/laden/artikel[2]/name" für den zweiten Artikel, "//artikel[@id='b']/name" für den Artikel, dessen ID b ist, oder "count(//artikel)" für die Anzahl. Ist der Text kein gültiges XML oder der Pfad kein gültiger Pfad, lautet das Ergebnis #WERT!. Gibt es keine Treffer, ist das Ergebnis #NV. XML mit einer Dokumenttypdeklaration wird abgelehnt.
