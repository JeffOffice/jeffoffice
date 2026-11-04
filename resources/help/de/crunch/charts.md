# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Diagramm, Grafik, Säule, Balken, Linie, Kreis, Kreisdiagramm, Fläche, Streudiagramm, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Trendlinie, Trend, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Minidiagramm, Markierungen -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber nicht mit Zahlen verbunden.

## Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch prüft die erste Zeile und Spalte und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumsangaben) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ aus: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ öffnet dessen Varianten, etwa gestapelte Säulen, eine Linie mit Datenpunkten oder ein Punkt-(XY-)Diagramm mit reinen Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ mit Ihren Zellen. Mit **Zeilen und Spalten tauschen** werden die Datenreihen umgekehrt.

Wenn Sie nur eine Zelle markieren, stellt JeffCrunch den Zellbereich rund um sie dar, der Inhalte enthält.

Datenreihen verlaufen spaltenweise, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und zeilenweise, wenn sie mehr Spalten als Zeilen hat. Ein Kreisdiagramm enthält eine Datenreihe. Ein Punkt-(XY-)Diagramm übernimmt die waagerechten Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4, wobei A2:A4 Q1, Q2 und Q3 enthalten, B1 und C1 Ost und West stehen und der Rest Zahlen sind. Das Diagramm hat zwei Datenreihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Verschieben und in der Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie **Alt** beim Ziehen gedrückt, damit sich eine Kante an einer Zellkante ausrichtet.
- Halten Sie beim Ziehen an einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt jeweils um wenige Pixel. **Strg** mit einer Pfeiltaste verschiebt es um ein Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Zellkante.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben oder Ändern der Größe ist ein Schritt, den Sie rückgängig machen können. Klicken Sie mit der rechten Maustaste auf das Objekt, und wählen Sie **Eigenschaften**, um die Größe einzugeben. Dort legen Sie außerdem fest, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, aber die Größe beibehalten, oder an der Stelle bleiben. Die Auswahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Feld schaltet das Objekt auf den gedruckten Seiten aus; auf dem Bildschirm bleibt es sichtbar. Auch diese Auswahl speichert die Datei.

## Anzeige eines Diagramms ändern

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband wird die Registerkarte **Diagramm** angezeigt.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Diagrammtitel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen ein oder aus und legt fest, wohin die Legende und die Beschriftungen kommen.
- Die Pinselschaltfläche (und **Farben** und **Designs** auf der Registerkarte) gibt dem Diagramm eine der Farbfolgen und eine der Darstellungen von JeffCrunch. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Datenreihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten vertauschen und festlegen, wie leere Zellen sowie ausgeblendete Zeilen und Spalten dargestellt werden.
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Datenreihe angepasst ist, damit Sie sehen, wohin sie sich entwickeln. Wählen Sie **Linear** für eine Gerade, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 bedeutet perfekte Übereinstimmung, 0 keine Übereinstimmung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wenn eine Datenreihe an einer zweiten senkrechten Achse dargestellt wird, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite Horizontale** und **Zweite Vertikale** für sie. Im Menü **Legende** gibt es **Über dem Diagramm**, das die Legende über die Darstellungsfläche legt statt daneben.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (der Wert, der Name der Kategorie, der Name der Datenreihe und der Prozentsatz, in beliebiger Kombination), **Trennzeichen** (was zwischen ihnen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (ein Formatcode wie 0,0 für die Zahlen in den Beschriftungen; leer lassen, um die Zahlen unverändert anzuzeigen). Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Feld für eine Achse. Dort legen Sie den kleinsten und größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge laufen, wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagrammtyp** ändert den Typ für das ganze Diagramm oder für jede Datenreihe einzeln: Säulen mit einer Linie darüber oder eine Datenreihe an einer zweiten senkrechten Achse.

Ein Bezug auf Zellen wird etwa so geschrieben: **Tabelle1!$B$2:$B$9**. Enthält der Blattname Leerzeichen, steht er in einfachen Anführungszeichen.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzige Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben diesen Zahlen.

1. Markieren Sie die Zahlen. Für eine Sparkline pro Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen durch eine Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sind.
3. Prüfen Sie im Feld **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben Sie den **Positionsbereich** ein (die Zellen, in die sie kommen). Wählen Sie für jede Datenzeile oder jede Datenspalte eine Zelle. Wählen Sie eine einzige Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und haben ein gemeinsames Aussehen. Klicken Sie auf eine Zelle mit einer Sparkline, erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und die Position der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschieht (als Lücke darstellen, als null behandeln oder die Punkte über sie hinweg verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **Erster Punkt**, den **Letzter Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe eine Farbfolge. **Sparkline-Farbe** legt die Farbe der Linie oder der Balken fest, und **Stärke** legt fest, wie dick die Linie ist (in Punkt). **Markierungsfarbe** legt die Farbe der einzelnen Arten markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und größten Wert der senkrechten Achse fest. **Automatisch für jede Sparkline** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine gemeinsame Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit späterem Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ein eigenes Aussehen. **Löschen** entfernt die ausgewählten Sparklines oder ganze Gruppen.

Eine Sparkline folgt ihren Zellen: Wenn Sie eine Zeile über den Daten einfügen, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einer Form gespeichert, die auch andere Tabellenkalkulationsprogramme lesen.

## Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden; JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe auf einem beliebigen Blatt ein, liest die Kopie dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest aber keine Zellen mehr, weil die Zellen dort nicht dieselben sind, aus denen es erstellt wurde.

## Objekt benennen

Wenn ein Objekt ausgewählt ist, zeigt das Namenfeld seinen Namen an. Geben Sie dort einen neuen Namen ein und drücken Sie **Eingabe**. Mit **Rückgängig** wird der alte Name wiederhergestellt; der neue Name wird in der Datei gespeichert.

## Objekt löschen

Drücken Sie bei ausgewähltem Objekt **Entf**. Mit **Rückgängig** holen Sie es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einer Form gespeichert, die auch andere Tabellenkalkulationsprogramme lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm aus einem anderen Programm, das Elemente enthält, die JeffCrunch beim Neuschreiben eines Diagramms nicht beibehalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich weiterhin verschieben, in der Größe ändern und löschen. Seine Elemente und Daten werden hier aber nicht verändert.
