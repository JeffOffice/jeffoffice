# Diagramme, Sparklines, Bilder und Formen
<!-- keywords: Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Streudiagramm, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Reihe, Trendlinie, Trendlinie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, Alt-Text, logarithmisch, logarithmische Skala, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber mit keinen Zahlen verbunden.

## Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch sieht sich die erste Zeile und Spalte an und verwendet sie als Namen und Beschriftungen, wenn es Wörter (bzw. bei Beschriftungen Datumsangaben) sind.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt dessen Varianten, etwa gestapelte Säulen, eine Linie mit Datenpunkten oder ein Streudiagramm aus Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ mit Ihren Zellen. Mit **Zeilen und Spalten tauschen** werden die Reihen umgekehrt.

Wenn Sie nur eine Zelle markieren, zeichnet JeffCrunch den Zellbereich um sie herum, in dem etwas steht.

Reihen verlaufen spaltenweise, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und zeilenweise, wenn sie mehr Spalten als Zeilen hat. Ein Kreisdiagramm enthält eine Reihe. Ein Streudiagramm übernimmt die waagerechten Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, und der Rest sind Zahlen. Das Diagramm hat zwei Reihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Verschieben und in der Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie beim Ziehen **Alt** gedrückt, damit sich eine Kante an der Kante einer Zelle ausrichtet.
- Halten Sie beim Ziehen an einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt jeweils um einige Pixel. **Strg** mit einer Pfeiltaste verschiebt es um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an, und wählen Sie **Eigenschaften**, um seinen **Namen** einzugeben (jedes Objekt auf einem Blatt braucht einen eigenen) und seinen **Alternativtext** (ein kurzer Satz, der Personen, die das Objekt nicht sehen können, erklärt, was es zeigt), um seine Größe festzulegen und um zu bestimmen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, aber die Größe beibehalten, oder an der Stelle bleiben. Die Auswahl wird in der Datei gespeichert. Mit **Dieses Objekt drucken** im selben Dialogfeld wird das Objekt auf den gedruckten Seiten ausgeblendet (auf dem Bildschirm bleibt es sichtbar); auch diese Einstellung speichert die Datei.

## Anzeige eines Diagramms ändern

Wenn ein Diagramm ausgewählt ist, erscheinen neben ihm zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Titel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen des Diagramms ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinselschaltfläche (und **Farben** sowie **Darstellung** auf der Registerkarte) gibt dem Diagramm eine der Farbgruppen und eine der Darstellungen von JeffCrunch. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Reihen des Diagramms auf. Sie können Reihen hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen und festlegen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Trendlinie** (im Menü **+**) zeichnet für die Zahlen jeder Reihe eine angepasste Linie, sodass Sie sehen, wohin sie tendieren. Wählen Sie **Linear** für eine Gerade, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die zuerst schnell steigen und sich dann einpendeln, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 bedeutet eine perfekte Anpassung, 0 gar keine). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wenn eine Reihe an einer zweiten vertikalen Achse dargestellt wird, haben die Menüs **Achsen** und **Achsentitel** dafür die Einträge **Zweite horizontale** und **Zweite vertikale**. Im Menü **Legende** gibt es **Über dem Diagramm**; damit liegt die Legende über der Zeichenfläche statt daneben.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Reihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Reihe, in beliebiger Kombination), **Trennzeichen** (was dazwischensteht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (ein Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen unverändert anzuzeigen). Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Dialogfeld für eine Achse. Es legt den kleinsten und größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder eine Zahl, die Sie eintippen), ob die Werte in umgekehrter Reihenfolge laufen, ob die Skala logarithmisch ist (jede Markierung ist ein Vielfaches der vorherigen, etwa 1, 10, 100 und 1000; das eignet sich für Zahlen, die sich stark in der Größe unterscheiden, und alle müssen größer als 0 sein), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt nach einem anderen Arbeitsblatt und legt das Diagramm dort in denselben Zellen ab. Es liest weiterhin die Zellen, die es gelesen hat, und behält seine Größe und seinen Platz auf dem Blatt bei, auch wenn die Zeilen und Spalten dort höher oder breiter sind, und es verschiebt und skaliert sich dort mit den Zellen wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Es ist ein einziger Schritt für **Rückgängig**, auch nachdem Sie gespeichert haben. Ein Diagrammblatt (ein Blatt, das nur aus einem Diagramm besteht) kann noch nicht erstellt werden, und ein Diagramm, das auf einem solchen Blatt liegt, lässt sich nicht verschieben.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder tippen Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle. Ändert sich die Zelle, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, zweimal so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) bewahren die Darstellung eines Diagramms in einer **.crtx**-Datei auf und übertragen sie auf ein anderes Diagramm. Zur Darstellung gehören Typ, Farben, Einstellungen für Titel und Legende, Achsen und Schriftarten. Das Diagramm, auf das Sie sie anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein Schritt **Rückgängig** nimmt die Darstellung wieder zurück.
- **Diagrammtyp** ändert den Typ für das ganze Diagramm oder für jede Reihe: Säulen mit einer Linie darüber oder eine Reihe an einer zweiten vertikalen Achse.

Ein Zellbezug wird etwa so geschrieben: **Tabelle1!$B$2:$B$9**. Der Name des Blatts steht in einfachen Anführungszeichen, wenn er Leerzeichen enthält.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben diesen Zahlen.

1. Markieren Sie die Zahlen. Für eine Sparkline pro Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sein können.
3. Prüfen Sie im Dialogfeld **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen), und geben Sie den **Positionsbereich** ein (die Zellen, in die sie kommen). Wählen Sie für jede Datenzeile oder für jede Datenspalte eine Positionszelle. Wenn Sie eine Zelle wählen, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und haben eine gemeinsame Darstellung. Klicken Sie auf eine Zelle mit einer Sparkline, und die Registerkarte **Sparkline** erscheint.

- **Daten bearbeiten** ändert die Daten und die Position der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschieht (Lücke lassen, als null behandeln oder die Punkte über sie hinweg verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **Erster Punkt**, den **Letzter Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Farbsatz. **Sparkline-Farbe** legt die Farbe der Linie oder der Balken fest, und **Stärke** (wie dick die Linie ist, in Punkt). **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, trägt die Zahlen von rechts nach links auf und legt den kleinsten und größten Wert der vertikalen Achse fest. **Für jede Sparkline automatisch** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine gemeinsame Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eintippen. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit späterem Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines zu einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder eine eigene Darstellung. **Löschen** entfernt die ausgewählten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das andere Tabellenkalkulationen lesen.

## Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden; JeffCrunch teilt Ihnen den Grund mit.

Wenn Sie ein Diagramm kopieren und in dieselbe Arbeitsmappe einfügen, auf einem beliebigen Blatt, liest die Kopie dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest dort aber keine Zellen mehr, denn die Zellen, die es dort lesen würde, sind nicht diejenigen, aus denen es erstellt wurde.

## Objekt benennen

Wenn ein Objekt ausgewählt ist, zeigt das Namenfeld seinen Namen an. Geben Sie dort einen neuen Namen ein, und drücken Sie **Eingabe**. **Rückgängig** setzt den alten Namen wieder ein, und der neue Name wird in der Datei gespeichert.

## Objekt löschen

Drücken Sie **Entf**, wenn das Objekt ausgewählt ist. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das andere Tabellenkalkulationen lesen, sodass ein hier erstelltes Diagramm dort ebenso geöffnet werden kann. Ein Diagramm aus einem anderen Programm, das etwas enthält, das JeffCrunch beim Neuschreiben eines Diagramms nicht übernehmen kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich weiterhin verschieben, in der Größe ändern und löschen, aber seine Elemente und Daten werden hier nicht verändert.
