# Diagramme, Bilder und Formen
<!-- keywords: Diagramm, Grafik, Schaubild, Säule, Säulendiagramm, Balken, Balkendiagramm, Linie, Liniendiagramm, Kreis, Kreisdiagramm, Torte, Fläche, Flächendiagramm, Punkt, Punktdiagramm, XY-Diagramm, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Reihe -->

Ein Diagramm ist ein Bild von Zahlen, das mit diesen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen auf dieselbe Weise über den Zellen, sind aber nicht mit Zahlen verbunden.

## Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie im Diagramm darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch prüft die erste Zeile und Spalte und verwendet sie als Namen und Beschriftungen, wenn sie Wörter enthalten (bei den Beschriftungen auch Datumsangaben).
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ aus: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ enthält seine Varianten, etwa gestapelte Säulen, eine Linie mit Datenpunkten oder ein Punktdiagramm mit Punkten, verbundenen Punkten oder interpolierten Linien.
3. Um zuerst alle Typen anzusehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ mit Ihren Zellen. **Zeilen und Spalten tauschen** dreht die Datenreihen in die andere Richtung.

Wenn Sie nur eine Zelle markieren, erstellt JeffCrunch das Diagramm aus dem Bereich um diese Zelle, der Inhalt hat.

Die Datenreihen verlaufen spaltenweise, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und zeilenweise, wenn sie mehr Spalten als Zeilen hat. Ein Kreisdiagramm enthält eine Datenreihe. Ein Punktdiagramm entnimmt seine horizontalen Werte der ersten Spalte.

Beispiel: Sie markieren A1:C4, wobei A2:A4 die Werte Q1, Q2 und Q3 enthalten, B1 und C1 die Werte Ost und West und die übrigen Zellen Zahlen. Das Diagramm hat zwei Datenreihen, Ost und West, und drei Säulengruppen, Q1 bis Q3.

## Verschieben und Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie beim Ziehen die **Alt**-Taste gedrückt, um eine Kante an der Kante einer Zelle auszurichten.
- Halten Sie beim Ziehen an einer Ecke die **Umschalttaste** gedrückt, um das Seitenverhältnis beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt jeweils um einige Pixel. **Strg** mit einer Pfeiltaste verschiebt es um ein Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste verschiebt es zur nächsten Zellkante.
- Mit **Esc** während des Ziehens setzen Sie es an seine Position zurück.

Jedes Verschieben und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie mit der rechten Maustaste auf das Objekt, und wählen Sie **Eigenschaften**, um seine Größe einzugeben und festzulegen, wie es sich beim Einfügen, Löschen oder Ändern der Größe von Zellen verhält: mit den Zellen verschieben und skalieren, mit den Zellen verschieben, aber die Größe beibehalten, oder die Position beibehalten. Die Auswahl wird in der Datei gespeichert.

## Ändern, was ein Diagramm anzeigt

Wenn ein Diagramm ausgewählt ist, werden daneben zwei Schaltflächen und im Menüband die Registerkarte **Diagramm** angezeigt.

- Mit der Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) blenden Sie Diagrammtitel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen ein oder aus und legen fest, wo Legende und Beschriftungen stehen.
- Die Pinsel-Schaltfläche (und **Farben** und **Darstellungen** auf der Registerkarte) weist dem Diagramm einen der Farbsätze und eine der Darstellungen von JeffCrunch zu. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Datenreihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen und festlegen, wie leere Zellen sowie ausgeblendete Zeilen und Spalten dargestellt werden.
- **Diagrammtyp** ändert den Typ für das gesamte Diagramm oder für jede Datenreihe: Säulen mit einer Linie darüber oder eine Datenreihe auf einer zweiten vertikalen Achse.

Ein Zellbezug wird zum Beispiel als **Tabelle1!$B$2:$B$9** geschrieben, wobei der Blattname in einfachen Anführungszeichen steht, wenn er Leerzeichen enthält.

## Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung mit einer Datei, kann nicht kopiert werden, und JeffCrunch teilt Ihnen den Grund mit.


Wenn Sie ein Diagramm kopieren und in dieselbe Arbeitsmappe einfügen, auf einem beliebigen Blatt, liest die Kopie dieselben Zellen wie das Original. Wenn Sie es in eine andere Arbeitsmappe einfügen, behält es die angezeigten Zahlen, liest aber keine Zellen mehr, da die Zellen dort nicht die sind, aus denen es erstellt wurde.

## Benennen

Wenn ein Objekt ausgewählt ist, zeigt das Namenfeld seinen Namen an. Geben Sie dort einen neuen Namen ein, und drücken Sie die **Eingabetaste**. Mit „Rückgängig“ stellen Sie den alten Namen wieder her; der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie **Entf**, während das Objekt ausgewählt ist. Mit „Rückgängig“ holen Sie es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in dem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen. Ein hier erstelltes Diagramm wird dort also genauso geöffnet. Ein Diagramm aus einem anderen Programm mit Elementen, die JeffCrunch beim Neuschreiben eines Diagramms nicht beibehalten kann (eine Farbverlaufsfüllung, eine Trendlinie, ein dreidimensionales Diagramm), lässt sich weiterhin verschieben, in der Größe ändern und löschen, seine Elemente und Daten werden hier aber nicht geändert.
