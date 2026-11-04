# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Punkt, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe, Bild, Form, Objekt, Reihe, Trendlinie, Trend-Linie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Minidiagramm, Markierungen -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verknüpft bleibt: Ändern Sie eine Zahl, ändert sich das Diagramm mit. Bilder und Formen liegen auf die gleiche Weise über den Zellen, sind aber nicht mit Zahlen verknüpft.

## Ein Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthalten, markieren Sie diese ebenfalls. JeffCrunch prüft die erste Zeile und Spalte und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt dessen Varianten, etwa gestapelte Säulen, eine Linie mit Markierungen oder ein Punktdiagramm mit Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ, gezeichnet aus Ihren Zellen. Mit **Zeilen und Spalten tauschen** werden die Reihen umgekehrt angeordnet.

Wenn Sie nur eine Zelle markieren, zeichnet JeffCrunch den Zellbereich um sie herum, in dem etwas steht.

Reihen verlaufen spaltenweise, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und zeilenweise, wenn sie mehr Spalten als Zeilen hat. Ein Kreisdiagramm enthält eine Reihe. Ein Punktdiagramm übernimmt die horizontalen Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, und der Rest sind Zahlen. Das Diagramm hat zwei Reihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Verschieben und in der Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie an einem der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie beim Ziehen **Alt** gedrückt, damit eine Kante an einer Zellkante ausgerichtet wird.
- Halten Sie beim Ziehen an einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt jeweils um einige Pixel. **Strg** mit einer Pfeiltaste verschiebt es um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** setzt das Objekt beim Ziehen an seine ursprüngliche Stelle zurück.

Jedes Verschieben und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an, und wählen Sie **Eigenschaften**, um seine Größe einzugeben und festzulegen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: Es wird mit den Zellen verschoben und in der Größe angepasst, mit ihnen verschoben, behält aber seine Größe, oder es bleibt an seiner Stelle. Die Auswahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Feld schaltet das Objekt auf den gedruckten Seiten aus (auf dem Bildschirm bleibt es sichtbar); auch diese Auswahl speichert die Datei.

## Festlegen, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagrammtools**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Diagrammtitel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen ein oder aus und legt fest, wohin Legende und Beschriftungen gehören.
- Die Pinselschaltfläche (und **Farben** und **Darstellung** auf der Registerkarte) gibt dem Diagramm eine der Farbfolgen und eine der Darstellungen von JeffCrunch. Mit **Layouts** legen Sie mehrere Elemente gleichzeitig fest.
- **Diagrammdaten** listet die Reihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, aus denen jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten vertauschen und festlegen, wie leere Zellen sowie ausgeblendete Zeilen und Spalten dargestellt werden.
- **Trendlinie** (im Menü **+**) zeichnet für die Werte jeder Reihe eine angepasste Linie, sodass Sie sehen, in welche Richtung sich die Werte entwickeln. Wählen Sie **Linear** für eine gerade Linie, **Exponentiell** für Werte, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Werte, die zunächst schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Krümmungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Werte zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie ins Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung, 0 bedeutet gar keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wenn eine Reihe an einer zweiten vertikalen Achse dargestellt wird, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite horizontale** und **Zweite vertikale** für diese Reihe. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende auf die Zeichnungsfläche legt, statt neben das Diagramm.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Reihe und den Prozentsatz, in beliebiger Kombination), **Trennzeichen** (was zwischen ihnen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (ein Formatcode wie 0,0 für die Zahlen in den Beschriftungen; leer lassen, um die Zahlen unverändert anzuzeigen). Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagrammtools** öffnen ein Feld für eine Achse. Dort legen Sie den kleinsten und größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge laufen, wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark sie gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagrammtools** fragt nach einem anderen Tabellenblatt und setzt das Diagramm dort in dieselben Zellen. Es liest weiterhin die Zellen, die es vorher gelesen hat, behält seine Größe und seine Position auf dem Blatt bei, auch wenn die Zeilen und Spalten dort höher oder breiter sind, und verschiebt und skaliert sich dort mit den Zellen wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Das Verschieben lässt sich mit einem einzigen Schritt **Rückgängig** zurücknehmen, auch nachdem Sie gespeichert haben. Ein Diagrammblatt (ein Blatt, das nur aus einem Diagramm besteht) kann noch nicht erstellt werden, und ein Diagramm auf einem solchen Blatt kann nicht verschoben werden.
- **Diagrammtyp** ändert den Typ für das ganze Diagramm oder für jede Reihe einzeln: Säulen mit einer Linie darüber oder eine Reihe an einer zweiten vertikalen Achse.

Ein Zellbezug wird geschrieben wie **Tabelle1!$B$2:$B$9**; der Name des Blatts steht in einfachen Anführungszeichen, wenn er Leerzeichen enthält.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben den Zahlen.

1. Markieren Sie die Zahlen. Für eine Sparkline pro Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. **Linie** verbindet die Zahlen durch eine Linie. **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sein können.
3. Prüfen Sie im Feld **Sparklines erstellen** den **Datenbereich** (die Zellen, aus denen die Sparklines lesen), und geben Sie den **Positionsbereich** ein (die Zellen, in die sie gesetzt werden). Wählen Sie für jede Datenzeile oder jede Datenspalte eine Zielzelle. Wenn Sie nur eine Zelle wählen, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und teilen sich ein Erscheinungsbild. Klicken Sie auf eine Zelle mit einer Sparkline, und die Registerkarte **Sparklinetools** erscheint.

- **Daten bearbeiten** ändert die Daten und die Position der ganzen Gruppe oder die Daten der einen Sparkline, auf die Sie geklickt haben. Außerdem legen Sie fest, was mit leeren Zellen geschieht (als Lücke darstellen, als null behandeln oder die Punkte über sie hinweg verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **Erster Punkt**, den **Letzter Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe eine Farbfolge. **Sparklinefarbe** legt die Farbe der Linie oder der Balken fest und die **Stärke** (wie dick die Linie in Punkt ist). **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, trägt die Zahlen von rechts nach links auf und legt den kleinsten und größten Wert der vertikalen Achse fest. **Für jede Sparkline automatisch** passt jede Sparkline an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine gemeinsame Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit einem späteren Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die markierten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ein eigenes Erscheinungsbild. **Löschen** entfernt die markierten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen.

## Ein Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden; JeffCrunch zeigt Ihnen den Grund an.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe auf einem beliebigen Blatt ein, liest die Kopie dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest aber keine Zellen mehr, denn die Zellen, die es dort lesen würde, sind nicht diejenigen, aus denen es erstellt wurde.

## Ein Objekt benennen

Wenn ein Objekt ausgewählt ist, zeigt das Namenfeld seinen Namen an. Geben Sie dort einen neuen Namen ein, und drücken Sie **Eingabe**. **Rückgängig** setzt den alten Namen wieder ein, und der neue Name wird in der Datei gespeichert.

## Ein Objekt löschen

Drücken Sie **Entf**, wenn das Objekt ausgewählt ist. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und Elemente enthält, die JeffCrunch beim Neuschreiben eines Diagramms nicht beibehalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich trotzdem verschieben, in der Größe ändern und löschen, aber seine Elemente und Daten werden hier nicht verändert.
