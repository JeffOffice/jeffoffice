# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Feldschaltflächen, Schaltflächen für Pivot-Diagramm, gruppieren, Gruppierung aufheben, ausrichten, linksbündig ausrichten, verteilen, Objekte auswählen, mehrere Formen, mehrere auswählen, Auswahlrahmen, Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Punkt, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Looks, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Reihe, Trendlinie, Trend, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Minidiagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skalierung, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Symbole, Symbol, wmf, emf, Formformatvorlagen, Formatvorlagen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber nicht an Zahlen gebunden.

## Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch prüft die erste Zeile und Spalte und verwendet sie als Namen bzw. Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt seine Varianten, etwa gestapelte Säulen, eine Linie mit Datenpunkten oder ein Punktdiagramm mit einzelnen Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zunächst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ, gezeichnet aus Ihren Zellen. Mit **Zeilen und Spalten tauschen** drehen Sie die Reihen um.

Sie sind nicht sicher, welchen Typ Sie wählen sollen? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch prüft die Zellen und zeigt einige passende Typen, der beste zuerst. Stehen beispielsweise Datumswerte untereinander, wird eine Linie vorgeschlagen; eine einzelne Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Wenn Sie nur eine Zelle markieren, zeichnet JeffCrunch den Zellbereich um sie herum, der Inhalt hat.

Reihen verlaufen in Spalten, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und in Zeilen, wenn sie mehr Spalten als Zeilen hat. Ein Kreisdiagramm enthält eine Reihe. Ein Punktdiagramm übernimmt die horizontalen Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, und der Rest sind Zahlen. Das Diagramm hat dann zwei Reihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Reihe vorübergehend ausblenden

Wählen Sie ein Diagramm aus. Neben ihm erscheinen drei kleine Schaltflächen. Die Schaltfläche mit dem Trichter ist **Diagrammfilter**. Sie zeigt die Reihen des Diagramms, jeweils mit einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um die Reihe aus dem Diagramm auszublenden. Aktivieren Sie es wieder, um sie an derselben Stelle zurückzuholen. **Rückgängig** funktioniert ebenfalls. Ein Diagramm behält immer mindestens eine Reihe. Eine ausgeblendete Reihe bleibt beim Speichern in der Datei erhalten und bleibt beim Öffnen der Datei ausgeblendet, sodass nichts verloren geht. Aktivieren Sie das Kontrollkästchen wieder, um sie zurückzuholen. (Ein Ringdiagramm zeigt beim Öffnen alle Reihen an.)

## Verschieben und in der Größe ändern

Klicken Sie auf ein Diagramm, Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie **Alt** beim Ziehen gedrückt, damit sich eine Kante an der Kante einer Zelle ausrichtet.
- Halten Sie **Umschalt** beim Ziehen an einer Ecke gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt einige Pixel auf einmal. **Strg** mit einer Pfeiltaste verschiebt es um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben oder Ändern der Größe ist ein Schritt, den Sie rückgängig machen können. Klicken Sie mit der rechten Maustaste auf das Objekt und wählen Sie **Eigenschaften**, um seinen **Namen** (jedes Objekt auf einem Blatt braucht einen eigenen) und seinen **Alternativtext** (ein kurzer Satz, der einer Person, die das Objekt nicht sehen kann, erklärt, was es zeigt) einzugeben, seine Größe festzulegen und zu wählen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen passiert: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, aber die Größe beibehalten, oder an der Stelle bleiben. Die Auswahl wird in der Datei gespeichert. **Dieses Objekt drucken** in demselben Feld schaltet das Objekt auf den gedruckten Seiten aus (auf dem Bildschirm bleibt es sichtbar); die Datei speichert auch diese Einstellung.

## Anzeige eines Diagramms ändern

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Titel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen des Diagramms ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinselschaltfläche (und **Farben** sowie **Looks** auf der Registerkarte) gibt dem Diagramm eine der Farbfolgen und eines der Erscheinungsbilder von JeffCrunch. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Reihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, aus denen jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen sowie festlegen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Feldschaltflächen** (im Menü **+**, nur bei einem Pivot-Diagramm) zeigen kleine Schaltflächen mit den Namen der Felder der Pivot-Tabelle, der das Diagramm folgt: die Berichtsfilter unter dem Titel sowie die Zeilen, Werte und Spalten am unteren Rand. Schalten Sie jede Art ein oder aus oder wählen Sie **Alle anzeigen** oder **Alle ausblenden**. Ein neues Pivot-Diagramm zeigt sie an.
- **Fehlerindikatoren** (im Menü **+**, für ein Säulen-, Balken-, Linien-, Flächen- oder Punktdiagramm) zeichnen über und unter jedem Punkt eine kleine Linie, die zeigt, wie weit der tatsächliche Wert abweichen könnte. Wählen Sie **Standardfehler**, **Prozentsatz (5 %)**, **Standardabweichung** oder **Fester Wert (1)**. Wählen Sie **Benutzerdefiniert**, um eigene Beträge zu verwenden: Geben Sie die Zellen ein, die die Beträge oberhalb jedes Punkts enthalten (etwa `=Tabelle1!$D$2:$D$4`), und danach die Zellen für die Beträge unterhalb. Lassen Sie eines der Felder leer, um die Balken nur auf einer Seite zu zeichnen. Die Balken ändern sich, wenn sich diese Zellen ändern. Eine Datei, die eigene Beträge für jeden Punkt enthält, behält sie.
- **Linien** (im Menü **+**, für ein Liniendiagramm) enthält **Tropfenlinien**, die vom höchsten Punkt jeder Kategorie bis zur Achse verlaufen, **Hoch-Tief-Linien**, die den höchsten und den niedrigsten Punkt jeder Kategorie verbinden, und **Auf-/Ab-Balken**, Kästchen zwischen der ersten und der letzten Reihe jeder Kategorie. Ein Aufwärtsbalken zeigt, dass die letzte Reihe höher liegt. Hoch-Tief-Linien und Auf-/Ab-Balken benötigen zwei oder mehr Reihen. Flächendiagramme haben Tropfenlinien.
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Reihe angepasst ist, damit Sie sehen, wohin sie tendieren. Wählen Sie **Linear** für eine Gerade, **Exponentiell** für Zahlen, die um einen gleichmäßigen Prozentsatz wachsen, **Logarithmisch** für Zahlen, die schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Mittelwert der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung, 0 bedeutet keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wenn eine Reihe an einer zweiten vertikalen Achse gezeichnet wird, haben die Menüs **Achsen** und **Achsentitel** dafür die Einträge **Zweite horizontale** und **Zweite vertikale**. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende über die Diagrammfläche legt statt daneben.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Reihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Reihe, in beliebiger Kombination), **Trennzeichen** (was dazwischen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (ein Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen so anzuzeigen, wie sie sind). **Werte aus Zellen…** zeigt den Text einiger Zellen als Beschriftungen, eine Zelle für jeden Punkt. Geben Sie die Zellen ein, etwa =Tabelle1!$D$2:$D$4. Die Beschriftungen ändern sich, wenn sich die Zellen ändern. Lassen Sie das Feld leer, um sie zu entfernen. Schalten Sie alle Inhalte aus, verschwinden auch die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Feld für eine Achse. Dort legen Sie den kleinsten und größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge laufen, die Anzeigeeinheiten (die Beschriftungen zeigen die Zahlen geteilt, etwa durch 1.000 bei Tausendern, sodass 30.000 als 30 erscheint; markieren Sie **Name der Einheiten im Diagramm anzeigen**, um den Namen, etwa Tausend, mit dem Achsentitel zu drucken), ob die Skala logarithmisch ist (jede Marke ist ein Vielfaches der Basis der vorherigen, etwa 1, 10, 100 und 1.000; das eignet sich für Zahlen, die sich stark in der Größe unterscheiden und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, die Position ihrer Beschriftungen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt nach einem anderen Tabellenblatt und setzt das Diagramm dort in dieselben Zellen. Es liest weiterhin die Zellen, die es gelesen hat, und behält seine Größe und seine Position auf dem Blatt, auch wenn die Zeilen und Spalten dort höher oder breiter sind; es verschiebt und ändert die Größe mit den Zellen dort, wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Es ist ein einziger Schritt für **Rückgängig**, auch nach dem Speichern. Ein Diagrammblatt (ein Blatt, das nur aus einem Diagramm besteht) kann noch nicht erstellt werden, und ein Diagramm, das auf einem solchen Blatt liegt, kann nicht verschoben werden.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle. Wenn sich die Zelle ändert, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) bewahren den Look eines Diagramms in einer **.crtx**-Datei auf und übertragen ihn auf ein anderes Diagramm. Zum Look gehören Typ, Farben, Einstellungen für Titel und Legende, die Achsen und die Schriftarten. Das Diagramm, auf das Sie ihn anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein **Rückgängig** entfernt den Look wieder.
- **Diagrammtyp** ändert den Typ für das ganze Diagramm oder für jede Reihe: Säulen mit einer Linie darüber oder eine Reihe an einer zweiten vertikalen Achse.

Ein Zellbezug wird etwa so geschrieben: **Tabelle1!$B$2:$B$9**. Enthält der Blattname Leerzeichen, steht er in einfachen Anführungszeichen.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben diesen Zahlen.

1. Markieren Sie die Zahlen. Für eine Sparkline pro Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sind.
3. Prüfen Sie im Feld **Sparklines erstellen** den **Datenbereich** (die Zellen, aus denen die Sparklines lesen), und geben Sie den **Standortbereich** ein (die Zellen, in denen sie stehen). Wählen Sie für jede Datenzeile oder für jede Datenspalte eine Zielzelle. Wählen Sie eine einzige Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und haben ein gemeinsames Aussehen. Klicken Sie auf eine Zelle mit einer Sparkline, erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und den Standort der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschehen soll (als Lücke darstellen, als Null behandeln oder die Punkte darüber verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Hochpunkt**, den **Tiefpunkt**, **Negative Punkte**, den **Ersten Punkt**, den **Letzten Punkt** und bei einer Linie **Markierungen** auf jedem Punkt.
- **Formatvorlagen** gibt der Gruppe eine Farbfolge. **Sparkline-Farbe** legt die Farbe der Linie oder der Balken und die **Stärke** fest (wie dick die Linie ist, in Punkt). **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und größten Wert der vertikalen Achse fest. **Für jede Sparkline automatisch** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine einheitliche Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit einem späteren Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines zu einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ihr eigenes Aussehen. **Löschen** entfernt die ausgewählten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in dem Format gespeichert, das andere Tabellenkalkulationen lesen.

## Bild, Form oder Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet **Bild** in der Gruppe **Illustrationen** einen Dateiauswahldialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG), und sie wird am ausgewählten Zellpunkt auf das Blatt gesetzt. Ein Bild, das breiter als 480 Punkte ist, wird verkleinert, damit es passt, und behält seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert speichern kann (etwa SVG), wird als PNG gespeichert, ein einfaches Bild aus Punkten. Eine Datei, die kein Bild ist, oder größer als 16 MB ist, wird mit einer Meldung abgelehnt. Eine WMF- oder EMF-Datei (Bildformate, die manche älteren Programme aus Linien und Formen erzeugen) wird ebenfalls als PNG gezeichnet und gespeichert.

**Symbole** zeigt dasselbe Fenster mit kleinen Strichbildern, das JeffPub und JeffDeck haben, mit einem Feld, um nach Namen zu suchen. Wählen Sie ein oder mehrere Symbole aus; jedes wird als Bild mit 48 Punkt Breite und Höhe auf das Blatt gesetzt. Es wird in dunklem Grau gezeichnet, in einer Größe, die beim Vergrößern scharf bleibt. Jedes Symbol ist ein einzelner Schritt, den Sie rückgängig machen können.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblase und eine Linie. Wählen Sie eine aus, und sie wird hellblau mit einem dunkleren Rand auf das Blatt gesetzt. **Textfeld** in der Gruppe **Text** fügt ein leeres Feld ohne Füllung und ohne Rand ein.

Ist eine Form ausgewählt, gibt **Formatvorlagen** auf der Registerkarte **Formformat** ihr ein fertiges Aussehen: eine Füllung und eine Kontur, die zusammenpassen, etwa Hellblau mit einem dunkelblauen Rand. Die letzten beiden Optionen behalten nur die Kontur oder nur die Füllung. Ein Klick ist ein Schritt, den Sie rückgängig machen können. **Formfüllung** und **Formkontur** ändern jeden Teil für sich.

Jede dieser Aktionen ist ein einzelner Schritt, den Sie rückgängig machen können. Das neue Objekt ist ausgewählt, sodass Sie es ziehen, in der Größe ändern, nach vorne oder hinten verschieben, unter **Eigenschaften** einen Namen und Alternativtext vergeben oder **Entf** drücken können.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wählen Sie ein Bild aus, erscheint die Registerkarte **Bildformat**. Wählen Sie eine Form, eine Linie oder ein Textfeld aus, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können, und das Objekt behält seinen Namen, seinen Alternativtext, seine Position und seine Reihenfolge unter den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Stärke (wie dick die Linie ist) und die Striche fest (durchgehend, gestrichelt oder gepunktet). Ein Bild hat **Bildrahmen**, seine Kontur.
- **Drehen** dreht das Objekt um eine viertel Drehung nach rechts oder links oder spiegelt es von links nach rechts oder von oben nach unten.
- **Zuschneiden** schneidet ein Bild auf eine Fensterform zu: quadratisch, 4 breit mal 3 hoch, 3 breit mal 4 hoch, 16 mal 9 oder 9 mal 16. Der Schnitt erfolgt gleichmäßig von der Mitte des sichtbaren Bereichs aus, und das Bild wird so groß wie der verbleibende Teil gemacht. **Ganzes Bild anzeigen** holt den abgeschnittenen Teil zurück. Die Bilddatei wird nicht verändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie den Text ein und wählen Sie, wie er waagerecht und senkrecht ausgerichtet ist, die Größe, fett oder normal, und ob er sich an die Form anpasst oder in einer Zeile bleibt.
- **Auswahlbereich** öffnet eine Liste aller Objekte auf dem Blatt mit ihren Namen, das vorderste zuerst. Klicken Sie auf einen Namen, um dieses Objekt auszuwählen; das hilft, wenn sich Objekte gegenseitig verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt Größe, Name, Alternativtext und fest, was das Objekt beim Verschieben oder Vergrößern von Zellen tut.

## Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung mit einer Datei, kann nicht kopiert werden; JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe auf einem beliebigen Blatt ein, liest die Kopie dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest aber keine Zellen mehr, da die dortigen Zellen nicht diejenigen sind, aus denen es erstellt wurde.

## Mehrere Objekte auswählen, gruppieren und ausrichten

Klicken Sie auf ein Objekt und halten Sie dann **Umschalt** oder **Strg** gedrückt, während Sie ein weiteres anklicken, um es ebenfalls auszuwählen. Klicken Sie mit **Umschalt** oder **Strg** erneut auf ein ausgewähltes Objekt, um es abzuwählen. Ausgewählte Objekte erhalten jeweils einen gestrichelten Rahmen. Außerdem können Sie auf der Registerkarte **Formformat** die Option **Objekte auswählen** wählen und über leere Zellen ein Feld ziehen: Jedes Objekt, das vollständig im Feld liegt, wird ausgewählt. Drücken Sie **Esc** oder wählen Sie **Objekte auswählen** erneut, um wieder Zellen auszuwählen. Drücken Sie **Entf**, um alle ausgewählten Objekte in einem Schritt zu entfernen.

Sind zwei oder mehr Objekte ausgewählt, enthält die Registerkarte **Formformat** in der Gruppe **Anordnen** folgende Befehle:

- **Gruppieren** fasst die ausgewählten Formen, Bilder und Linien zu einem Objekt zusammen. Es verschiebt und vergrößert sich als Einheit, und die Datei speichert es als Gruppe. Ein Diagramm kann nicht gruppiert werden.
- **Gruppierung aufheben** löst eine ausgewählte Gruppe auf. Jeder Teil kehrt an seine Position auf dem Blatt zurück.
- **Ausrichten** richtet die ausgewählten Objekte aus: **Linksbündig**, **Zentriert ausrichten**, **Rechtsbündig**, **Oben ausrichten**, **Mittig ausrichten** und **Unten ausrichten**. Sie richten sich am Rand oder an der Mitte der gesamten Gruppe aus. **Horizontal verteilen** und **Vertikal verteilen** verteilen drei oder mehr Objekte gleichmäßig zwischen den beiden äußersten.

**Gruppieren**, **Gruppierung aufheben** und jede Ausrichtungsoption sind jeweils ein Schritt, den Sie rückgängig machen können.

## Benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie **Eingabe**. **Rückgängig** stellt den alten Namen wieder her, und der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie **Entf**, während das Objekt ausgewählt ist. **Rückgängig** bringt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in dem Format gespeichert, das andere Tabellenkalkulationen lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und Elemente enthält, die JeffCrunch beim erneuten Schreiben eines Diagramms nicht beibehalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), kann weiterhin verschoben, in der Größe geändert und gelöscht werden, aber seine Elemente und Daten werden hier nicht verändert.
