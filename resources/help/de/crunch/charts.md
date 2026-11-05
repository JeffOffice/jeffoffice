# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Punkt, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Trendlinie, Trendlinie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skala, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber nicht an Zahlen gebunden.

## Ein Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Haben die erste Zeile Namen und die erste Spalte Beschriftungen, markieren Sie diese mit. JeffCrunch sieht sich die erste Zeile und Spalte an und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt dessen Varianten, etwa gestapelte Säulen, eine Linie mit Markierungen oder ein Punktdiagramm mit einzelnen Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ mit Ihren Zellen. Mit **Zeilen und Spalten tauschen** drehen Sie die Datenreihen um.

Sie wissen nicht, welcher Typ passt? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch prüft die Zellen und schlägt einige passende Typen vor, der beste zuerst. Stehen beispielsweise Datumswerte untereinander, wird eine Linie vorgeschlagen; eine Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Wenn Sie nur eine Zelle markieren, zeichnet JeffCrunch den Zellbereich um sie herum, der Inhalt hat.

Datenreihen verlaufen in den Spalten, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und in den Zeilen, wenn sie mehr Spalten als Zeilen hat. Ein Kreis enthält genau eine Datenreihe. Ein Punktdiagramm übernimmt die horizontalen Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, der Rest sind Zahlen. Das Diagramm hat zwei Datenreihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Datenreihe vorübergehend ausblenden

Wählen Sie ein Diagramm aus. Daneben erscheinen drei kleine Schaltflächen. Die Schaltfläche mit dem Trichter ist **Diagrammfilter**. Sie zeigt die Datenreihen des Diagramms, jede mit einem Häkchen. Entfernen Sie das Häkchen, um eine Datenreihe aus dem Diagramm herauszunehmen. Setzen Sie es wieder, um sie an derselben Stelle zurückzuholen. Auch **Rückgängig** funktioniert. Ein Diagramm behält immer mindestens eine Datenreihe. Eine ausgeblendete Datenreihe wird beim Speichern und Schließen der Datei nicht übernommen. Setzen Sie das Häkchen also vorher wieder, wenn Sie sie behalten möchten.

## Verschieben und in der Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie an einem der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie beim Ziehen **Alt** gedrückt, damit eine Kante an der Kante einer Zelle ausgerichtet wird.
- Halten Sie beim Ziehen an einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt einige Pixel auf einmal. **Strg** mit einer Pfeiltaste verschiebt es um ein Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jede Verschiebung und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an und wählen Sie **Eigenschaften**. Dort geben Sie den **Namen** ein (jedes Objekt auf einem Blatt braucht einen eigenen Namen) und den **Alternativtext** (ein kurzer Satz, der Personen, die das Objekt nicht sehen können, sagt, was es zeigt). Außerdem legen Sie dort die Größe fest und bestimmen, was mit dem Objekt geschieht, wenn Sie Zellen einfügen, löschen oder in der Größe ändern: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, die Größe aber beibehalten, oder an der Stelle bleiben. Die Auswahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Feld schaltet das Objekt auf den gedruckten Seiten aus (auf dem Bildschirm bleibt es sichtbar); auch diese Einstellung speichert die Datei.

## Ändern, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Diagrammtitel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen ein oder aus und legt fest, wohin Legende und Beschriftungen kommen.
- Die Pinselschaltfläche (und **Farben** sowie **Darstellung** auf der Registerkarte) gibt dem Diagramm eine der Farbfolgen und eine der Darstellungen von JeffCrunch. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Datenreihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen sowie bestimmen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Trendlinie** (im Menü **+**) zeichnet für jede Datenreihe eine Linie, die an die Zahlen angepasst ist, damit Sie sehen, wohin die Werte tendieren. Wählen Sie **Linear** für eine gerade Linie, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die zuerst schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung, 0 bedeutet gar keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wird eine Datenreihe an einer zweiten vertikalen Achse dargestellt, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite horizontale** und **Zweite vertikale** für diese Reihe. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende auf die Zeichenfläche legt statt daneben.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Datenreihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Datenreihe, in beliebiger Kombination), **Trennzeichen** (was zwischen ihnen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (einen Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen unverändert anzuzeigen). Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Feld für eine Achse. Dort legen Sie den kleinsten und größten Wert fest sowie die Haupt- und Hilfsintervalle (jeweils automatisch oder als eingegebene Zahl). Außerdem bestimmen Sie, ob die Werte in umgekehrter Reihenfolge laufen, ob die Skala logarithmisch ist (jede Markierung ist ein Vielfaches der vorherigen mit einer Basis, etwa 1, 10, 100 und 1000; das eignet sich für Zahlen, die sich stark in der Größe unterscheiden und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat der Beschriftungen, die Teilstriche, wo die Beschriftungen stehen und wie stark sie gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt nach einem anderen Tabellenblatt und setzt das Diagramm dorthin, in dieselben Zellen. Es liest weiterhin die Zellen, die es gelesen hat, und behält seine Größe und seinen Platz auf dem Blatt, auch wenn die Zeilen und Spalten dort höher oder breiter sind; dort verschiebt und skaliert es sich mit den Zellen wie bisher. Ausgeblendete Blätter werden nicht angeboten. Es ist ein einziger Schritt für **Rückgängig**, auch nachdem Sie gespeichert haben. Ein Diagrammblatt (ein Blatt, das nur aus einem Diagramm besteht) lässt sich noch nicht erstellen, und ein Diagramm auf einem solchen Blatt kann nicht verschoben werden.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle an. Ändert sich die Zelle, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) bewahren die Darstellung eines Diagramms in einer **.crtx**-Datei auf und übertragen sie auf ein anderes Diagramm. Zur Darstellung gehören Typ, Farben, Einstellungen für Titel und Legende, die Achsen und die Schriftarten. Das Diagramm, auf das Sie sie anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein Schritt **Rückgängig** entfernt die Darstellung wieder.
- **Diagrammtyp** ändert den Typ für das ganze Diagramm oder für jede Datenreihe einzeln: Säulen mit einer Linie darüber oder eine Datenreihe an einer zweiten vertikalen Achse.

Einen Zellbezug schreiben Sie etwa so: **Tabelle1!$B$2:$B$9**. Enthält der Blattname Leerzeichen, steht er in einfachen Anführungszeichen.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt daneben.

1. Markieren Sie die Zahlen. Möchten Sie für jede Zeile eine Sparkline, markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** den Typ **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen durch eine Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl unter null einen Balken nach unten, alle gleich groß. Das eignet sich für Ergebnisse, die nur gut oder schlecht sein können.
3. Prüfen Sie im Feld **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben Sie den **Positionsbereich** ein (die Zellen, in die die Sparklines gesetzt werden). Wählen Sie für jede Zeile der Daten oder für jede Spalte der Daten eine Zelle als Position. Wählen Sie eine einzige Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und haben eine gemeinsame Darstellung. Klicken Sie auf eine Zelle mit einer Sparkline, erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und die Position der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschieht (als Lücke darstellen, als null behandeln oder die Punkte über sie hinweg verbinden), und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **ersten Punkt**, den **letzten Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Farbsatz. **Sparklinefarbe** legt die Farbe der Linie oder der Balken fest, und **Stärke** die Dicke der Linie in Punkt. **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und größten Wert der vertikalen Achse fest. **Für jede Sparkline automatisch** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine gemeinsame Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit einem späteren Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ihre eigene Darstellung. **Löschen** entfernt die ausgewählten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das auch andere Tabellenkalkulationen lesen.

## Ein Bild, eine Form oder ein Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet **Bild** in der Gruppe **Illustrationen** einen Dateiauswahldialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG). Das Bild wird auf dem Blatt an der ausgewählten Zelle eingefügt. Ein Bild, das breiter als 480 Punkte ist, wird verkleinert, damit es passt, und behält dabei seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert speichern kann (zum Beispiel SVG), wird als PNG gespeichert, ein einfaches Bild aus Pixeln. Eine Datei, die kein Bild ist, oder größer als 16 MB ist, wird mit einer Meldung abgelehnt.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblase und eine Linie. Wählen Sie eine aus, und sie erscheint hellblau mit dunklerem Rand auf dem Blatt. **Textfeld** in der Gruppe **Text** fügt ein leeres Feld ohne Füllung und ohne Rand ein.

Jedes davon ist ein einzelner Schritt, den Sie rückgängig machen können. Das neue Objekt ist ausgewählt, sodass Sie es ziehen, in der Größe ändern, nach vorne oder hinten bringen, unter **Eigenschaften** einen Namen und Alternativtext vergeben oder die Taste **Entf** drücken können.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wählen Sie ein Bild aus, erscheint die Registerkarte **Bildformat**. Wählen Sie eine Form, eine Linie oder ein Textfeld aus, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Das Objekt behält dabei seinen Namen, seinen Alternativtext, seinen Platz und seine Reihenfolge unter den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Stärke (wie dick die Linie ist) und die Striche fest (durchgezogen, gestrichelt oder gepunktet). Ein Bild hat den **Bildrahmen**, das ist seine Kontur.
- **Drehen** dreht das Objekt um eine Viertelumdrehung nach rechts oder links oder spiegelt es horizontal oder vertikal.
- **Zuschneiden** schneidet ein Bild auf eine Fensterform zu: Quadrat, 4 breit und 3 hoch, 3 breit und 4 hoch, 16 zu 9 oder 9 zu 16. Der Zuschnitt erfolgt gleichmäßig von der Mitte des sichtbaren Bereichs aus, und das Bild wird so groß wie der verbleibende Bereich. **Gesamtes Bild anzeigen** holt das Abgeschnittene zurück. Die Bilddatei bleibt unverändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie den Text ein und legen Sie fest, wie er horizontal und vertikal sitzt, welche Schriftgröße er hat, ob er fett oder normal ist und ob er sich an die Form anpasst oder in einer Zeile bleibt.
- **Auswahlbereich** öffnet eine Liste aller Objekte auf dem Blatt mit ihrem Namen, das vorderste zuerst. Klicken Sie auf einen Namen, um dieses Objekt auszuwählen; das hilft, wenn Objekte einander verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt Größe, Namen, Alternativtext und fest, was das Objekt bewirkt, wenn Zellen verschoben oder vergrößert werden.

## Ein Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, lässt sich nicht kopieren; JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe ein, auf einem beliebigen Blatt: Die Kopie liest dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest aber keine Zellen mehr, weil die Zellen dort nicht diejenigen sind, aus denen es erstellt wurde.

## Ein Objekt benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie **Eingabe**. **Rückgängig** setzt den alten Namen wieder ein, und der neue Name wird in der Datei gespeichert.

## Ein Objekt löschen

Drücken Sie bei ausgewähltem Objekt die Taste **Entf**. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das auch andere Tabellenkalkulationen lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und Elemente enthält, die JeffCrunch beim Neuschreiben eines Diagramms nicht beibehalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich weiterhin verschieben, in der Größe ändern und löschen. Seine Elemente und Daten werden hier jedoch nicht verändert.
