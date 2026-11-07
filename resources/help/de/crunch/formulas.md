# Wie Formeln angezeigt und berechnet werden
<!-- keywords: nächster Fehler, vorheriger Fehler, Fehler durchgehen, Fehler verfolgen, vor dem Speichern berechnen, Formel, Formeln, berechnen, Berechnung, neu berechnen, Funktion, Funktionen, Wert, Ergebnis, gespeichert, F9, zirkulär, Zirkelbezug, Formeln anzeigen, Bearbeitungsleiste, Fehler, zeigen, Zelle anklicken, Bezug, F4, Fehlerüberprüfung, Fehler ignorieren, Formel auswerten, Schritt, Überwachungsfenster, überwachen, LET, Wert benennen, Zellfunktion, Informationsfunktion, FILTERXML, XML, XPath-->

Eine **Formel** ist eine Zelle, die ihren Wert aus anderen Zellen errechnet, zum Beispiel =SUMME(B3:B8). Die Zelle zeigt den Wert an, und die **Bearbeitungsleiste** zeigt die Formel.

## Was JeffCrunch mit Formeln macht

Wenn Sie eine Arbeitsmappe öffnen, enthält die Datei für jede Formel den zuletzt gespeicherten Wert, damit das Blatt sofort angezeigt werden kann. Danach berechnet JeffCrunch jede Formel erneut, jeweils einige wenige, damit das Fenster flüssig bleibt. Die Statusleiste zeigt dabei **Berechnung läuft** mit einer Prozentangabe. Wenn die Berechnung abgeschlossen ist, enthalten die Zellen die eigenen Ergebnisse von JeffCrunch. Um alles erneut zu berechnen, klicken Sie auf der Registerkarte **Formeln** auf **Alles neu berechnen** oder drücken Sie **F9**.

## Wenn JeffCrunch eine Formel nicht berechnen kann

JeffCrunch hat mehr als 400 Funktionen, und es kommen laufend neue hinzu. Eine Formel, die eine Funktion verwendet, die JeffCrunch noch nicht kennt, behält den Wert, den die Datei dafür gespeichert hat. Das Gleiche gilt für eine Formel, die auf eine andere Arbeitsmappe verweist. Zellen, die davon abhängen, werden aus den gespeicherten Werten berechnet.

## Funktionen mit einigen Hinweisen

- **Anleihen und Darlehen.** PREIS, RENDITE, DURATION, ODDFPREIS und die anderen Anleihefunktionen zählen die Tage so, wie es die **Basis** angibt, die Sie angeben (0 bedeutet 30 Tage je Monat in einem Jahr mit 360 Tagen, 1 bedeutet die tatsächlichen Tage und so weiter). RENDITE, ODDFRENDITE, ODDLRENDITE und ZINS ermitteln ihr Ergebnis, indem sie so lange Werte ausprobieren, bis Kurs oder Zahlung passt. Sie hören auf, wenn das Ergebnis auf etwa 15 Stellen genau ist, daher können die letzten Stellen von denen eines anderen Programms abweichen.
- **Verteilungen und Tests.** Die Funktionen, die auf .VERT, .INV, .TEST enden, sowie die älteren Namen wie NORMVERT und TVERT liefern dieselben Ergebnisse: Ein älterer Name bleibt erhalten, damit alte Dateien weiterhin funktionieren. Die *.INV-Funktionen werden ebenfalls durch Ausprobieren von Werten gefunden.
- **Listen, die mehrere Zellen füllen.** SORTIEREN, SORTIERENNACH, FILTER, EINDEUTIG, FOLGE, ZUFALLSMATRIX, XVERWEIS, MMULT, MINV und RGP liefern einen Block von Werten. Geben Sie sie als Matrixformel (**Strg+Umschalt+Eingabe**) über den Zellen ein, die sie füllen sollen. Dass eine Formel in die danebenliegenden Zellen überläuft, wird noch nicht unterstützt.
- **Texte in anderen Schriften.** LENGTHB, LINKSB, TEILB, RECHTSB, FINDENB, SUCHENB und ERSETZENB zählen jedes Zeichen als eines, so wie auf einem Computer, der nicht für Doppelbyte-Text eingerichtet ist. PHONETIC liefert den Text der Zelle, weil JeffCrunch die Lesungen japanischer Texte nicht speichert.
- **UMRECHNEN.** Einheiten werden als Text geschrieben, mit Groß- und Kleinbuchstaben wie in der Liste der Funktion angegeben: „kg“, „lbm“, „mi“, „F“. Ein metrisches Präfix kann vor einer Einheit stehen, die eines verträgt, etwa „km“ oder „mL“.

## MOD: der Rest

**REST(n; d)** liefert den Rest von **n**, nachdem so viele ganze Vielfache von **d** abgezogen wurden, wie möglich sind: Er ist n - d * GANZZAHL(n / d). Beispiel: **=REST(10; 3)** ergibt 1, und **=REST(-10; 3)** ergibt 2, denn GANZZAHL(-10 / 3) ist -4, und -10 - 3 * -4 ergibt 2. Das Ergebnis hat das Vorzeichen von **d**. Ist **d** 0, lautet das Ergebnis #DIV/0!.

## Formeln anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Formeln anzeigen** oder drücken Sie **Strg+Akzent gravis** (auf dem Mac die Control-Taste, nicht die Befehlstaste), um in jeder Zelle die Formel statt des Werts zu sehen. Die Spalten mit Formeln werden breiter, bis zu einer Grenze, damit die Formeln vollständig angezeigt werden. Beim Ausschalten erhalten sie wieder ihre ursprüngliche Breite, und die Spaltenbreiten in Ihrer Datei bleiben unverändert.

## Formeln ändern

Um eine Formel zu ändern, markieren Sie ihre Zelle, drücken Sie **F2**, bearbeiten die Formel und drücken **Eingabe**. Sie können sie auch in der Bearbeitungsleiste bearbeiten. Wenn Sie eine Zelle ändern, werden die Formeln, die von ihr abhängen, im Hintergrund neu berechnet. Siehe [Zellen beschreiben](edit-cells).

## Eine Formel schreiben

Geben Sie **=** und die ersten Buchstaben einer Funktion ein. Eine kurze Liste zeigt die Funktionen und [Namen](names), die so beginnen, mit einem Satz zu jeder. Drücken Sie **Tab**, um die markierte zu übernehmen. Mit den Pfeiltasten **Nach unten** und **Nach oben** markieren Sie eine andere und übernehmen sie dann mit **Tab** oder **Eingabe**. **Esc** schließt die Liste. Sobald Sie innerhalb der Klammern einer Funktion stehen, werden deren Argumente unter der Zelle angezeigt, wobei das gerade eingegebene fett dargestellt wird. Die Liste und die Argumente sind dieselben, die **Funktion auswählen** anzeigt. Argumente werden durch Kommas getrennt oder, wo Ihre Sprache das Komma als Dezimaltrennzeichen verwendet, durch Semikola.

## Auf Zellen zeigen

Statt die Adresse einer Zelle einzutippen, können Sie auf sie zeigen. Während Sie eine Formel eingeben, setzen Sie den Cursor dorthin, wo eine Zelle stehen kann (direkt nach **=**, nach einem Operator wie **+**, nach einer öffnenden Klammer oder nach einem Komma), und klicken Sie auf die Zelle. Ihre Adresse wird für Sie eingetragen. Ziehen Sie über Zellen, um einen Bereich wie B2:D5 zu schreiben. Klicken Sie erneut, bevor Sie etwas anderes eingeben, und die neue Zelle ersetzt die gerade eingetragene. Halten Sie **Strg** gedrückt und klicken Sie, um nach einem Komma eine weitere Adresse hinzuzufügen. Klicken Sie auf einen Spaltenbuchstaben, um eine ganze Spalte wie A:A einzutragen, oder auf eine Zeilennummer, um eine ganze Zeile wie 3:3 einzutragen. Ziehen Sie über die Buchstaben oder Zahlen, um B:D oder 2:5 einzutragen.

Auch die Pfeiltasten funktionieren. Drücken Sie einen Pfeil, und ein Zeiger bewegt sich von der Zelle weg, in der Sie gerade schreiben, und seine Adresse wird eingetragen. Drücken Sie weitere Pfeile, um weiterzugehen, oder halten Sie **Umschalt** gedrückt, um den Zeiger über einen Bereich zu ziehen. Die Statusleiste zeigt **Zeigen**, während die Pfeiltasten zeigen, **Eingabe**, während Sie schreiben, und **Bearbeiten**, während die Pfeiltasten den Cursor im Text bewegen. Drücken Sie **F2**, um zwischen Zeigen und Bearbeiten des Textes umzuschalten.

Um auf eine Zelle auf einem anderen Blatt zu zeigen, klicken Sie auf die Registerkarte dieses Blatts, während der Cursor an einer Stelle steht, wo eine Zelle stehen kann, und klicken dann auf die Zelle. Der Blattname wird mitgeschrieben, bei Bedarf in Anführungszeichen, wie in 'Mein Blatt'!B3. **Eingabe** bringt Sie zurück zu dem Blatt, auf dem Sie begonnen haben, und trägt die Formel in ihre Zelle ein. **Esc** verwirft alles, auch den Blattwechsel.

Sie können außerdem den Rand eines umrandeten Bereichs auf dem Blatt ziehen, um diese Adresse zu verschieben, oder das Eckquadrat ziehen, um sie zu erweitern. Die Formel ändert sich beim Ziehen.

Drücken Sie **F4**, während der Cursor in oder direkt hinter einer Adresse steht, um Teile davon mit einem Dollarzeichen zu fixieren: A1 wird zu $A$1, dann zu A$1, dann zu $A1 und dann wieder zu A1.

Jede Adresse in der Formel ist im Text farbig markiert, und die Zellen, auf die sie verweist, sind auf dem Blatt in derselben Farbe umrandet. So sehen Sie auf einen Blick, was die Formel liest. Ein Klick nach einer vollständigen Adresse, etwa =A1, beendet die Eingabe und markiert die angeklickte Zelle, wie bei jedem anderen Text.

## Datentabellen

Eine Datentabelle (Mehrfachoperation) zeigt, welches Ergebnis eine Formel für viele Werte einer oder zweier Zellen ergibt. Öffnen Sie eine Arbeitsmappe, die eine solche Tabelle enthält, und JeffCrunch berechnet ihre Zellen: Bei einer Tabelle mit zwei Eingaben steht die Formel in der Zelle oben links in der Tabelle, die Werte, die waagerecht ausprobiert werden, kommen in die Zeileneingabezelle, und die senkrecht ausprobierten Werte in die Spalteneingabezelle. Bei einer Tabelle mit einer Eingabe stehen die Werte senkrecht in der Spalte links davon (und die Formeln in der Zeile darüber) oder waagerecht in der Zeile darüber (und die Formeln in der Spalte links davon). Jede Zelle der Tabelle ist das Ergebnis der Formel und aller Formeln, die von der Eingabezelle abhängen, wobei die Eingabe auf diesen Wert gesetzt ist. Sonst wird nichts in der Arbeitsmappe verändert. Im Modus **Automatisch** werden die Tabellen zusammen mit den Formeln berechnet, und die Formeln, die die Zellen einer Tabelle lesen, folgen. Eine neue Datentabelle anzulegen wird noch nicht angeboten. Eine Tabelle, die sich nicht berechnen lässt (weil eine Formel, die von der Eingabe abhängt, eine Funktion verwendet, die JeffCrunch nicht kennt, oder sich selbst ändert, etwa bei ZUFALLSZAHL oder JETZT), behält die Werte, die in ihrer Datei stehen.

## Von Hand berechnen

Klicken Sie auf der Registerkarte **Formeln** auf **Berechnungsart**, um festzulegen, wie die Arbeitsmappe ihre Formeln berechnet:

- **Automatisch** berechnet sie, während Sie Zellen ändern.
- **Automatisch, Tabellen manuell** tut dasselbe, lässt aber die Zellen von Datentabellen unverändert, bis Sie **F9** drücken.
- **Manuell** berechnet nichts, auch keine Datentabellen, bis Sie es verlangen. Nach einer Änderung zeigt die Statusleiste **Veraltet**. Drücken Sie **F9** (**Alles neu berechnen**), um alles zu berechnen, oder **Umschalt+F9** (**Dieses Blatt neu berechnen**), um die Formeln des aktuellen Blatts und alles andere, was wartet, zu berechnen. Eine Arbeitsmappe, die auf Manuell eingestellt ist, öffnet sich mit den Ergebnissen, die ihre Datei enthält. **Vor dem Speichern berechnen** steht im Menü **Berechnungsart** zunächst aktiviert: Wenn Sie eine manuell berechnete Arbeitsmappe speichern, die veraltet ist, werden die Formeln vorher berechnet, damit die Datei aktuelle Zahlen enthält. Schalten Sie es aus, um die Zahlen so zu speichern, wie sie sind.

Die Auswahl wird mit der Arbeitsmappe gespeichert, und sie lässt sich rückgängig machen.

## Formeln, die sich selbst lesen

Eine Formel, die direkt oder über andere Zellen ihr eigenes Ergebnis liest, kann nicht auf die übliche Weise berechnet werden. JeffCrunch weist Sie einmal darauf hin, nennt die Zellen und umrandet sie auf dem Blatt. Sie behalten die Werte, die sie hatten. Klicken Sie auf der Registerkarte **Formeln** auf **Zirkelbezüge**, um die Liste zu sehen, und doppelklicken Sie auf eine Zelle darin, um dorthin zu gelangen.

Manche Modelle brauchen einen solchen Zirkel absichtlich. Wählen Sie im Menü **Berechnungsart** die Option **Iterative Berechnung aktivieren**. Dann werden die Formeln immer wieder berechnet, wobei jede die neuesten Werte der anderen liest, bis sich kein Wert mehr um mehr als den angegebenen Betrag ändert oder die festgelegte Anzahl an Durchgängen verbraucht ist.

## Sehen, was eine Formel liest

Markieren Sie eine Zelle und klicken Sie auf **Vorgänger dieser Zelle anzeigen**, um Pfeile von den Zellen zu zeichnen, die ihre Formel liest. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. **Nachfolger dieser Zelle anzeigen** zeichnet Pfeile zu den Formeln, die die Zelle lesen, und jeder Klick geht einen Schritt weiter. Eine kleine Seite steht für eine Zelle auf einem anderen Blatt. Es werden höchstens 2.000 Pfeile gezeichnet, und die Statusleiste meldet, wenn es mehr gab. **Pfeile entfernen** löscht sie, und jede Änderung an der Arbeitsmappe löscht sie ebenfalls.

Wenn eine Zelle einen Fehler wie #DIV/0! anzeigt, markieren Sie sie und klicken Sie auf **Fehler verfolgen**. Es werden Pfeile zu den Zellen gezeichnet, die sie liest und die ebenfalls einen Fehler anzeigen. Klicken Sie erneut, um einen Schritt weiter zurückzugehen. Wenn keine der gelesenen Zellen einen Fehler anzeigt, meldet die Statusleiste, dass diese Zelle den Fehler verursacht, und sie ist die, die Sie korrigieren sollten.

Um alle Fehler auf dem Blatt der Reihe nach durchzugehen, klicken Sie auf **Nächster Fehler** oder **Vorheriger Fehler**. Die Zellen, die einen Fehler anzeigen, werden Zeile für Zeile besucht, und nach der letzten beginnt es wieder bei der ersten. Jede Zelle wird markiert, und die Pfeile werden bis zu der Zelle gezeichnet, die den Fehler verursacht. Die Statusleiste zeigt an, um welchen Fehler es sich handelt und wie viele es insgesamt gibt.

## Fehlerüberprüfung

Wählen Sie in der Gruppe **Prüfen** der Registerkarte Formeln **Fehlerüberprüfung**, um das Blatt nach Zellen zu durchsuchen, die fehlerhaft sein könnten. Es sucht nach zwei Dingen: einer Formel, die einen Fehler ergibt (etwa #DIV/0!), und einer Zahl, die als Text gespeichert ist und die Formeln möglicherweise übergehen.

Das Dialogfeld zeigt jeweils eine Zelle, springt auf dem Blatt zu dieser Zelle und erklärt in einem Satz, was nicht stimmt. **Weiter** und **Zurück** wechseln zwischen den Zellen. Ein kleines grünes Dreieck in der oberen linken Ecke einer Zelle markiert sie auf dem Blatt. **Fehler ignorieren** nimmt die Zelle von der Liste, entfernt ihr Dreieck und wird in der Datei gespeichert. Ein Schritt Rückgängig holt sie zurück. Ist nichts fehlerhaft, erhalten Sie eine entsprechende Meldung.

## Formel auswerten

Damit sehen Sie, wie eine Formel zu ihrem Wert kommt. Markieren Sie eine Zelle mit einer Formel und wählen Sie **Formel auswerten**. Die Formel wird angezeigt, wobei der nächste Teil unterstrichen ist. Drücken Sie **Auswerten**, um diesen Teil zu berechnen; sein Wert nimmt dann dessen Platz ein. Die Teile werden in dieser Reihenfolge abgearbeitet: zuerst jede einzelne Zelle von links, dann jede Funktion mit ihren Klammern, beginnend mit der innersten, und zuletzt der Rest. **Neu starten** beginnt von vorn. Lässt sich ein Teil nicht berechnen, nennt das Dialogfeld den Grund und lässt die Formel unverändert.

## Überwachungsfenster

Das **Überwachungsfenster** ist ein kleines Fenster, das beim Arbeiten im Blick bleibt. Es listet Zellen auf, die Sie im Auge behalten möchten, mit ihrem Blatt, ihrer Zelle, ihrem Wert und ihrer Formel, auch wenn sie auf einem anderen Blatt liegen. Markieren Sie Zellen und drücken Sie **Überwachung hinzufügen**. Markieren Sie Zeilen in der Liste und drücken Sie **Überwachung löschen**. Doppelklicken Sie auf eine Zeile, um zur Zelle zu gelangen. Die Werte ändern sich, wenn das Blatt neu berechnet wird. Die Liste bleibt erhalten, solange die Arbeitsmappe geöffnet ist.

## Werte innerhalb einer Formel benennen

**LET** gibt einem Wert mitten in einer Formel einen Namen, sodass Sie ihn nur einmal schreiben und danach wieder verwenden können. Die Paare aus Name und Wert kommen zuerst, die Berechnung zuletzt. Bei =LET(preis; B2; steuer; preis*0,08; preis+steuer) steht der Name **preis** für den Wert in B2, **steuer** wird daraus berechnet, und der letzte Teil addiert beide. Ein Name kann die davor stehenden Namen verwenden. Ein Name gilt nur innerhalb seines eigenen LET, und ein LET innerhalb eines anderen darf einen Namen erneut verwenden. Ein Name kann auch für einen Zellbereich stehen, wie in =LET(zeilen; B2:B9; SUMME(zeilen)/ANZAHL(zeilen)).

## Nach einer Zelle oder der Arbeitsmappe fragen

**ZELLE** liefert Angaben zu einer Zelle. Ihr erstes Argument legt fest, welche: „adresse“, „zeile“, „spalte“, „inhalt“, „typ“, „format“ (wie Zahlen angezeigt werden, etwa „F2“ für zwei Dezimalstellen, „P0“ für Prozent oder „D1“ für ein Datum), „farbe“ (1, wenn negative Zahlen farbig angezeigt werden), „klammern“ (1, wenn positive Zahlen in Klammern angezeigt werden), „präfix“ (das Kennzeichen eines Textes, der links, rechts oder zentriert ausgerichtet ist), „schutz“ (1, wenn die Zelle gesperrt ist), „breite“ (die Breite der Spalte in Zeichen) und „dateiname“ (der Ordner, die Datei und das Blatt, etwa /home/ich/[Budget.xlsx]Tabelle1; leer, bis die Arbeitsmappe gespeichert ist). **INFO** liefert Angaben zum Programm und zur Arbeitsmappe: „verzeichnis“, „numdatei“ (wie viele Blätter angezeigt werden), „ursprung“, „betriebssystem“, „neuberechnen“ (Automatisch oder Manuell), „version“ und „system“.

## Werte aus XML herauslösen

**FILTERXML** nimmt Text, der XML enthält, und einen XPath, also eine kurze Angabe, welche Teile Sie möchten, und liefert die gefundenen Werte. =FILTERXML(A1; "//artikel/preis") liefert jeden Preis im Text in A1, untereinander. Verwenden Sie "/laden/artikel[2]/name" für den zweiten Artikel, "//artikel[@id='b']/name" für den Artikel mit der id b oder "count(//artikel)" für die Anzahl. Ist der Text kein gültiges XML oder der Pfad keiner, lautet das Ergebnis #WERT!. Trifft nichts zu, lautet es #NV. XML mit einem Dokumenttyp wird abgelehnt.
