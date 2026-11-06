# Diagramme, Sparklines, Bilder und Formen
<!-- keywords: gruppieren, Gruppierung aufheben, ausrichten, links ausrichten, verteilen, Objekte auswählen, mehrere Formen, mehrere auswählen, Auswahlrahmen, Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Streudiagramm, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Reihe, Trendlinie, Trendlinie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skalierung, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Symbole, Symbol, wmf, emf, Formformatvorlagen, Formatvorlagen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber nicht an Zahlen gebunden.

## Ein Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch prüft die erste Zeile und Spalte und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ öffnet dessen Varianten, etwa gestapelte Säulen, eine Linie mit Datenpunkten oder ein Punkt-(XY)-Diagramm mit reinen Punkten, verbundenen Punkten oder interpolierten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ, gezeichnet aus Ihren Zellen. Mit **Zeilen und Spalten tauschen** drehen Sie die Reihen um.

Sie wissen nicht, welchen Typ Sie wählen sollen? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch betrachtet die Zellen und listet einige passende Typen auf, der beste zuerst. Datumswerte untereinander ergeben zum Beispiel eine Linie, und eine Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Wenn Sie nur eine Zelle markieren, stellt JeffCrunch den Zellbereich um diese Zelle dar, der Inhalt hat.

Reihen verlaufen in den Spalten, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und in den Zeilen, wenn sie mehr Spalten als Zeilen hat. Ein Kreis enthält eine Reihe. Ein Punkt-(XY)-Diagramm übernimmt die waagerechten Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4, wobei A2:A4 Q1, Q2 und Q3 enthalten, B1 und C1 Ost und West enthalten und der Rest Zahlen sind. Das Diagramm hat zwei Reihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Reihe vorübergehend ausblenden

Wählen Sie ein Diagramm aus. Neben ihm erscheinen drei kleine Schaltflächen. Die Schaltfläche mit dem Trichter ist **Diagrammfilter**. Sie zeigt die Reihen des Diagramms, jede mit einem Häkchen. Entfernen Sie das Häkchen, um eine Reihe aus dem Diagramm herauszunehmen. Setzen Sie es wieder, um sie an derselben Stelle zurückzuholen. **Rückgängig** funktioniert ebenfalls. Ein Diagramm behält immer mindestens eine Reihe. Eine ausgeblendete Reihe bleibt beim Speichern in der Datei erhalten und bleibt beim Öffnen der Datei ausgeblendet, sodass nichts verloren geht. Setzen Sie das Häkchen wieder, um sie zurückzuholen. (Ein Ringdiagramm zeigt beim Öffnen alle Reihen an.)

## Verschieben und Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie **Alt** gedrückt, während Sie ziehen, damit eine Kante an der Kante einer Zelle ausgerichtet wird.
- Halten Sie beim Ziehen einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt um wenige Pixel. **Strg** mit einer Pfeiltaste verschiebt es um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens stellt den ursprünglichen Zustand wieder her.

Jedes Verschieben oder Ändern der Größe ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an und wählen Sie **Eigenschaften**, um den **Namen** (jedes Objekt auf einem Blatt braucht einen eigenen) und den **Alternativtext** (ein kurzer Satz, der einer Person, die das Objekt nicht sehen kann, erklärt, was es zeigt) einzugeben, die Größe festzulegen und zu wählen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, aber die Größe beibehalten, oder an der Stelle bleiben. Die Auswahl wird in der Datei gespeichert. **Dieses Objekt drucken** in demselben Feld schaltet das Objekt auf den gedruckten Seiten ab (es bleibt auf dem Bildschirm sichtbar); auch diese Einstellung speichert die Datei.

## Ändern, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen neben ihm zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Titel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen des Diagramms ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinselschaltfläche (und **Farben** sowie **Designs** auf der Registerkarte) gibt dem Diagramm einen der Farbsätze und eines der Designs von JeffCrunch. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Reihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen und festlegen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Fehlerindikatoren** (im Menü **+**, für ein Säulen-, Balken-, Linien- oder Punkt-(XY)-Diagramm) zeichnen über und unter jedem Punkt eine kleine Linie, die zeigt, wie weit der tatsächliche Wert abweichen kann. Wählen Sie **Standardfehler**, **Prozentsatz (5 %)**, **Standardabweichung** oder **Fester Wert (1)**. Eine Datei, die eigene Beträge für jeden Punkt enthält, behält sie.
- **Linien** (im Menü **+**, für ein Liniendiagramm) enthält **Tropflinien**, die vom höchsten Punkt jeder Kategorie bis zur Achse verlaufen, **Hoch-Tief-Linien**, die den höchsten und den niedrigsten Punkt jeder Kategorie verbinden, und **Aufwärts-/Abwärtsbalken**, das sind Kästen zwischen der ersten und der letzten Reihe jeder Kategorie. Ein Aufwärtsbalken zeigt, dass die letzte Reihe höher liegt. Hoch-Tief-Linien und Aufwärts-/Abwärtsbalken benötigen zwei oder mehr Reihen. Flächendiagramme haben Tropflinien.
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Reihe angepasst ist, damit Sie sehen, wohin sie sich entwickeln. Wählen Sie **Linear** für eine gerade Linie, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten paar Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung; 0 bedeutet gar keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wenn eine Reihe an einer zweiten senkrechten Achse dargestellt wird, enthalten die Menüs **Achsen** und **Achsentitel** dafür die Einträge **Zweite Waagerechte** und **Zweite Senkrechte**. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende über die Diagrammfläche legt statt daneben.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Reihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Reihe, in beliebiger Kombination), **Trennzeichen** (was dazwischensteht: ein Komma, Semikolon, Punkt, neuer Absatz oder Leerzeichen) und **Zahlenformat…** (ein Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen so anzuzeigen, wie sie sind). **Werte aus Zellen…** zeigt die Wörter einiger Zellen als Beschriftungen an, eine Zelle für jeden Punkt. Geben Sie die Zellen ein, etwa =Tabelle1!$D$2:$D$4. Die Beschriftungen ändern sich, wenn sich die Zellen ändern. Lassen Sie das Feld leer, um sie zu entfernen. Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Waagerechte Achse formatieren…** und **Senkrechte Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Feld für jeweils eine Achse. Es legt den kleinsten und den größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eine von Ihnen eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge verlaufen, die Anzeigeeinheiten (die Beschriftungen zeigen die Zahlen geteilt an, etwa durch 1.000 für Tausender, sodass 30.000 als 30 erscheint; aktivieren Sie **Name der Einheiten im Diagramm anzeigen**, um den Namen, etwa Tausender, zusammen mit dem Achsentitel zu drucken), ob die Skalierung logarithmisch ist (jede Markierung ist ein Vielfaches der vorherigen mit einer Basis, etwa 1, 10, 100 und 1.000; das eignet sich für Zahlen, die sich stark in ihrer Größe unterscheiden und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt nach einem anderen Arbeitsblatt und legt das Diagramm dort in denselben Zellen ab. Es liest weiterhin die Zellen, die es gelesen hat, und behält seine Größe und seine Position auf dem Blatt bei, auch wenn die Zeilen und Spalten dort höher oder breiter sind, und es verschiebt und ändert die Größe mit den Zellen dort wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Es ist ein einziger Schritt für **Rückgängig**, auch nachdem Sie gespeichert haben. Ein Diagrammblatt (ein Blatt, das nur aus einem Diagramm besteht) kann noch nicht erstellt werden, und ein Diagramm auf einem solchen Blatt kann nicht verschoben werden.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel an, was diese Zelle enthält. Ändert sich die Zelle, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) bewahren das Erscheinungsbild eines Diagramms in einer **.crtx**-Datei auf und übertragen es auf ein anderes Diagramm. Zum Erscheinungsbild gehören Typ, Farben, Einstellungen für Titel und Legende, die Achsen und die Schriftarten. Das Diagramm, auf das Sie es anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein Schritt **Rückgängig** entfernt das Erscheinungsbild wieder.
- **Diagrammtyp ändern** ändert den Typ für das ganze Diagramm oder für jede Reihe: Säulen mit einer Linie darüber oder eine Reihe an einer zweiten senkrechten Achse.

Ein Verweis auf Zellen wird etwa so geschrieben: **Tabelle1!$B$2:$B$9**. Der Name des Blatts steht in einfachen Anführungszeichen, wenn er Leerzeichen enthält.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt direkt neben den Zahlen den Verlauf einer Zeile oder Spalte mit Zahlen.

1. Markieren Sie die Zahlen. Für eine Sparkline pro Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sind.
3. Prüfen Sie im Feld **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben Sie den **Positionsbereich** ein (die Zellen, in die sie kommen). Wählen Sie für jede Zeile mit Daten oder für jede Spalte mit Daten eine Positionszelle. Wählen Sie eine einzige Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und teilen sich ein Erscheinungsbild. Klicken Sie auf eine Zelle mit einer Sparkline, dann erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und den Ort der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschehen soll (als Lücke darstellen, als null behandeln oder die Punkte über sie hinweg verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **ersten Punkt**, den **letzten Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Satz Farben. **Sparkline-Farbe** legt die Farbe der Linie oder der Balken fest, und die **Stärke** (wie dick die Linie ist, in Punkt). **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und den größten Wert der senkrechten Achse fest. **Automatisch für jede Sparkline** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine einheitliche Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten, dann steht ein Punkt mit einem späteren Datum weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die markierten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ein eigenes Erscheinungsbild. **Löschen** entfernt die markierten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie trotzdem dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in dem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen.

## Ein Bild, eine Form oder ein Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet **Bild** in der Gruppe **Illustrationen** einen Dateiauswahldialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG), und sie wird am ausgewählten Zellcursor auf das Blatt gesetzt. Ein Bild, das breiter als 480 Punkte ist, wird verkleinert, damit es passt, und behält seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert speichern kann (zum Beispiel SVG), wird als PNG gespeichert, ein einfaches Bild aus Punkten. Eine Datei, die kein Bild ist oder größer als 16 MB ist, wird mit einer Meldung abgelehnt. Eine WMF- oder EMF-Datei (Bildformate, die manche älteren Programme aus Linien und Formen erstellen) wird ebenfalls gezeichnet und als PNG gespeichert.

**Symbole** zeigt dieselbe Auswahl kleiner Linienbilder, die JeffPub und JeffDeck haben, mit einem Feld, um sie nach Namen zu suchen. Wählen Sie eines oder mehrere aus; jedes wird als Bild mit einer Breite und Höhe von 48 Punkt auf das Blatt gesetzt. Es wird dunkelgrau gezeichnet, in einer Größe, die beim Vergrößern scharf bleibt. Jedes Symbol ist ein einzelner Schritt, den Sie rückgängig machen können.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblase und eine Linie. Wählen Sie eine aus, dann wird sie hellblau mit einem dunkleren Rand auf das Blatt gesetzt. **Textfeld** in der Gruppe **Text** fügt ein leeres Feld ohne Füllung und ohne Rand ein.

Wenn eine Form ausgewählt ist, gibt **Formatvorlagen** auf der Registerkarte **Formformat** ihr ein fertiges Erscheinungsbild: eine Füllung und eine Kontur, die zusammenpassen, etwa Hellblau mit einem dunkleren blauen Rand. Die letzten beiden behalten nur die Kontur oder nur die Füllung. Ein Klick ist ein Schritt, den Sie rückgängig machen können. **Formfüllung** und **Formkontur** ändern jeden Teil für sich.

Jede dieser Aktionen ist ein einzelner Schritt, den Sie rückgängig machen können. Das neue Objekt ist ausgewählt, sodass Sie es ziehen, in der Größe ändern, eine Ebene nach vorne oder nach hinten verschieben, ihm unter **Eigenschaften** einen Namen und Alternativtext geben oder **Entf** drücken können.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wenn Sie ein Bild auswählen, erscheint die Registerkarte **Bildformat**. Wenn Sie eine Form, eine Linie oder ein Textfeld auswählen, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können, und das Objekt behält seinen Namen, seinen Alternativtext, seine Position und seine Reihenfolge unter den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Stärke (wie dick die Linie ist) und die Striche fest (durchgehend, gestrichelt oder gepunktet). Ein Bild hat **Bildrahmen**, das ist seine Kontur.
- **Drehen** dreht das Objekt um eine Vierteldrehung nach rechts oder links oder spiegelt es horizontal oder vertikal.
- **Zuschneiden** schneidet ein Bild auf eine Fensterform zu: Quadrat, 4 breit und 3 hoch, 3 breit und 4 hoch, 16 zu 9 oder 9 zu 16. Der Zuschnitt erfolgt gleichmäßig von der Mitte des sichtbaren Bereichs aus, und das Bild wird so groß gemacht, wie es der verbleibende Bereich erlaubt. **Ganzes Bild anzeigen** holt das Abgeschnittene zurück. Die Bilddatei wird nicht verändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie die Wörter ein und legen Sie fest, wie sie waagerecht und senkrecht ausgerichtet sind, die Größe, fett oder normal, und ob sie sich an die Form anpassen oder in einer Zeile bleiben.
- **Auswahlbereich** öffnet eine Liste aller Objekte auf dem Blatt mit ihrem Namen, das vorderste zuerst. Klicken Sie auf einen Namen, um dieses Objekt auszuwählen; das hilft, wenn sich Objekte gegenseitig verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt die Größe, den Namen, den Alternativtext und fest, was das Objekt beim Verschieben oder Vergrößern von Zellen tut.

## Ein Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird ebenso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden, und JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe ein, auf einem beliebigen Blatt: Die Kopie liest dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest aber keine Zellen mehr, weil die Zellen, die es dort lesen würde, nicht diejenigen sind, aus denen es erstellt wurde.

## Mehrere Objekte auswählen, gruppieren und ausrichten

Klicken Sie auf ein Objekt und halten Sie dann **Umschalt** oder **Strg** gedrückt, während Sie ein weiteres anklicken, um es ebenfalls auszuwählen. Klicken Sie ein ausgewähltes Objekt mit **Umschalt** oder **Strg** erneut an, um es abzuwählen. Ausgewählte Objekte erhalten jeweils einen gestrichelten Rahmen. Außerdem können Sie auf der Registerkarte **Formformat** die Option **Objekte auswählen** wählen und einen Rahmen über leere Zellen ziehen: Jedes Objekt, das ganz innerhalb des Rahmens liegt, wird ausgewählt. Drücken Sie **Esc** oder wählen Sie **Objekte auswählen** erneut, um wieder Zellen auszuwählen. Drücken Sie **Entf**, um alle ausgewählten Objekte in einem Schritt zu entfernen.

Sind zwei oder mehr Objekte ausgewählt, enthält die Registerkarte **Formformat** in der Gruppe **Anordnen** diese Befehle:

- **Gruppieren** fasst die ausgewählten Formen, Bilder und Linien zu einem Objekt zusammen. Es wird als Einheit verschoben und in der Größe geändert, und die Datei speichert es als Gruppe. Ein Diagramm kann nicht gruppiert werden.
- **Gruppierung aufheben** löst eine ausgewählte Gruppe auf. Jeder Teil kehrt an seine Stelle auf dem Blatt zurück.
- **Ausrichten** richtet die ausgewählten Objekte aus: **Links ausrichten**, **Zentriert ausrichten**, **Rechts ausrichten**, **Oben ausrichten**, **Mitte ausrichten** und **Unten ausrichten**. Sie richten sich an der Kante oder der Mitte der ganzen Gruppe aus. **Horizontal verteilen** und **Vertikal verteilen** verteilen drei oder mehr Objekte gleichmäßig zwischen den beiden äußersten.

Gruppieren, Gruppierung aufheben und jede Ausrichtungsoption sind jeweils ein Schritt, den Sie rückgängig machen können.

## Benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie **Enter**. **Rückgängig** stellt den alten Namen wieder her, und der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie **Entf**, während das Objekt ausgewählt ist. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in dem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und Elemente enthält, die JeffCrunch beim Neuschreiben eines Diagramms nicht beibehalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), kann trotzdem verschoben, in der Größe geändert und gelöscht werden, aber seine Elemente und Daten werden hier nicht verändert.
