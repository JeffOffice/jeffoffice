# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Feldschaltflächen, Schaltflächen für Pivot-Diagramme, gruppieren, Gruppierung aufheben, ausrichten, linksbündig ausrichten, verteilen, Objekte auswählen, mehrere Formen, mehrere auswählen, Auswahlrahmen, Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Punkt, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Looks, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Trendlinie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skala, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Symbole, Symbol, wmf, emf, Formenarten, Formatvorlagen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern, Diagramm, Diagramme, Schema -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber mit keinen Zahlen verbunden.

## Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese mit. JeffCrunch sieht sich die erste Zeile und Spalte an und verwendet sie als Namen und Beschriftungen, wenn es Wörter (oder bei den Beschriftungen Datumswerte) sind.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt seine Varianten, etwa gestapelte Säulen, eine Linie mit Datenpunkten oder ein Punktdiagramm mit einzelnen Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zunächst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ mit Ihren Zellen. **Zeilen und Spalten wechseln** dreht die Datenreihen um.

Sie wissen nicht, welcher Typ passt? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch prüft die Zellen und schlägt einige passende Typen vor, den besten zuerst. Stehen beispielsweise Datumswerte untereinander, schlägt JeffCrunch eine Linie vor; eine einzelne Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Wenn Sie nur eine Zelle markieren, stellt JeffCrunch den Zellbereich um sie herum dar, der Inhalt hat.

Datenreihen verlaufen in den Spalten, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und in den Zeilen, wenn sie mehr Spalten als Zeilen hat. Ein Kreisdiagramm enthält eine Datenreihe. Ein Punktdiagramm übernimmt die horizontalen Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, der Rest sind Zahlen. Das Diagramm hat zwei Datenreihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Datenreihe vorübergehend ausblenden

Klicken Sie ein Diagramm an. Neben ihm erscheinen drei kleine Schaltflächen. Die Schaltfläche mit dem Trichter ist **Diagrammfilter**. Sie zeigt die Datenreihen des Diagramms, jede mit einem Häkchen. Entfernen Sie das Häkchen, um die Datenreihe aus dem Diagramm auszublenden. Setzen Sie es wieder, um sie an derselben Stelle zurückzuholen. **Rückgängig** funktioniert ebenfalls. Ein Diagramm behält immer mindestens eine Datenreihe. Eine ausgeblendete Datenreihe bleibt beim Speichern in der Datei erhalten und bleibt beim Öffnen der Datei ausgeblendet, damit nichts verloren geht. Setzen Sie das Häkchen wieder, um sie zurückzuholen. (Ein Ringdiagramm zeigt beim Öffnen alle Datenreihen an.)

## Verschieben und in der Größe ändern

Klicken Sie ein Diagramm, ein Bild oder eine Form an, um sie auszuwählen. Ziehen Sie sie, um sie zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie beim Ziehen die **Alt**-Taste gedrückt, damit eine Kante an der Kante einer Zelle ausgerichtet wird.
- Halten Sie beim Ziehen an einer Ecke die **Umschalttaste** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt in kleinen Schritten von wenigen Pixeln. **Strg** mit einer Pfeiltaste verschiebt um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an und wählen Sie **Eigenschaften**. Dort geben Sie den **Namen** ein (jedes Objekt auf einem Blatt braucht einen eigenen) sowie den **Alternativtext** (einen kurzen Satz, der Menschen, die das Objekt nicht sehen können, sagt, was es zeigt). Außerdem legen Sie die Größe fest und bestimmen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mitverschieben und mit skalieren, mitverschieben, aber die Größe behalten, oder an der Stelle bleiben. Die Wahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Feld schaltet das Objekt auf den gedruckten Seiten ab (auf dem Bildschirm bleibt es sichtbar); auch diese Wahl speichert die Datei.

## Festlegen, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf der Menüband-Registerkarte erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Titel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen des Diagramms ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinsel-Schaltfläche (und **Farben** und **Looks** auf der Registerkarte) gibt dem Diagramm eines der Farbsets und eines der Erscheinungsbilder von JeffCrunch. **Layouts** legt mehrere Elemente auf einmal fest.
- **Diagrammdaten** listet die Datenreihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Datenreihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten vertauschen und bestimmen, wie leere Zellen sowie ausgeblendete Zeilen und Spalten dargestellt werden.
- **Feldschaltflächen** (im Menü **+**, nur für ein Pivot-Diagramm) zeigen kleine Schaltflächen mit den Namen der Felder der Pivot-Tabelle, der das Diagramm folgt: die Berichtsfilter unter dem Titel sowie die Zeilen, Werte und Spalten entlang des unteren Rands. Schalten Sie jede Art ein oder aus oder wählen Sie **Alle anzeigen** oder **Alle ausblenden**. Ein neues Pivot-Diagramm zeigt sie an.
- **Fehlerindikatoren** (im Menü **+**, für ein Säulen-, Balken-, Linien-, Flächen- oder Punktdiagramm) zeichnen über und unter jedem Punkt eine kleine Linie, die zeigt, wie weit der tatsächliche Wert abweichen könnte. Wählen Sie **Standardfehler**, **Prozentsatz (5 %)**, **Standardabweichung** oder **Fester Wert (1)**. Wählen Sie **Benutzerdefiniert**, um eigene Beträge zu verwenden: Geben Sie die Zellen ein, die die Beträge über jedem Punkt enthalten (zum Beispiel `=Tabelle1!$D$2:$D$4`), danach die Zellen für die Beträge darunter. Lassen Sie eine Angabe leer, um die Balken nur auf einer Seite zu zeichnen. Die Balken ändern sich, wenn sich diese Zellen ändern. Eine Datei, die eigene Beträge für jeden Punkt enthält, behält sie.
- **Linien** (im Menü **+**, für ein Liniendiagramm) enthält **Abfallende Linien**, die von dem höchsten Punkt jeder Kategorie bis zur Achse verlaufen, **Hoch-Tief-Linien**, die den höchsten und den niedrigsten Punkt jeder Kategorie verbinden, und **Aufwärts-/Abwärtsbalken**, das sind Kästchen zwischen der ersten und der letzten Datenreihe jeder Kategorie. Ein Aufwärtsbalken zeigt an, dass die letzte Datenreihe höher liegt. Hoch-Tief-Linien und Aufwärts-/Abwärtsbalken brauchen zwei oder mehr Datenreihen. Flächendiagramme haben abfallende Linien.
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Datenreihe angepasst ist, damit Sie sehen, wohin sie sich entwickeln. Wählen Sie **Linear** für eine gerade Linie, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 bedeutet eine perfekte Anpassung, 0 gar keine). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wenn eine Datenreihe an einer zweiten vertikalen Achse gezeichnet wird, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite Horizontale** und **Zweite Vertikale** für sie. Im Menü **Legende** gibt es **Über dem Diagramm**, das die Legende auf die Zeichenfläche statt daneben legt.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Datenreihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Datenreihe, in beliebiger Mischung), **Trennzeichen** (was zwischen ihnen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (einen Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen unverändert anzuzeigen). **Werte aus Zellen…** zeigt die Texte einiger Zellen als Beschriftungen, eine Zelle für jeden Punkt. Geben Sie die Zellen ein, zum Beispiel =Tabelle1!$D$2:$D$4. Die Beschriftungen ändern sich, wenn sich die Zellen ändern. Lassen Sie das Feld leer, um sie zu entfernen. Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Feld für jeweils eine Achse. Dort legen Sie den kleinsten und den größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge verlaufen, die Anzeigeeinheiten (die Beschriftungen zeigen die Zahlen geteilt, etwa durch 1.000 bei Tausendern, sodass 30.000 als 30 erscheint; aktivieren Sie **Name der Einheiten im Diagramm anzeigen**, damit der Name, etwa Tausender, zusammen mit dem Achsentitel gedruckt wird), ob die Skala logarithmisch ist (jede Markierung ist ein Vielfaches der vorherigen, etwa 1, 10, 100 und 1.000; das eignet sich für Zahlen, die sich stark in ihrer Größe unterscheiden und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt, wohin das Diagramm soll. Wählen Sie **Neues Blatt** und geben Sie einen Namen ein, dann bekommt das Diagramm ein eigenes Diagrammblatt: ein Blatt, das nur aus einem Diagramm besteht und keine Zellen hat. Wählen Sie **Objekt in** und ein Tabellenblatt, dann landet das Diagramm auf diesem Tabellenblatt. Ein Diagramm auf einem Diagrammblatt kann nur auf ein Tabellenblatt verschoben werden; das Diagrammblatt, das dadurch frei wird, wird gelöscht. Das Diagramm liest weiterhin die Zellen, die es vorher gelesen hat. Auf einem Tabellenblatt behält es seine Größe und seine Position auf dem Blatt, auch wenn die Zeilen und Spalten dort höher oder breiter sind, und es verschiebt und skaliert sich mit den Zellen dort wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Das ist ein einziger Schritt für **Rückgängig**, selbst nach dem Speichern, es sei denn, ein Blatt wurde dabei gelöscht. Eine Datei im Format `.ods` kann kein Diagrammblatt enthalten. Speichern Sie ein Diagrammblatt daher im Format `.xlsx`.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle. Ändert sich die Zelle, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) speichern das Erscheinungsbild eines Diagramms in einer Datei **.crtx** und übertragen es auf ein anderes Diagramm. Zum Erscheinungsbild gehören der Typ, die Farben, die Einstellungen für Titel und Legende, die Achsen und die Schriftarten. Das Diagramm, auf das Sie es anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein **Rückgängig** nimmt das Erscheinungsbild wieder zurück.
- **Diagrammtyp ändern** ändert den Typ für das gesamte Diagramm oder für jede Datenreihe einzeln: Säulen mit einer Linie darüber oder eine Datenreihe an einer zweiten vertikalen Achse.

Einen Zellbezug schreiben Sie etwa als **Tabelle1!$B$2:$B$9**. Enthält der Name des Blatts Leerzeichen, steht er in einfachen Anführungszeichen.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben diesen Zahlen.

1. Markieren Sie die Zahlen. Für eine Sparkline je Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß. Das eignet sich für Ergebnisse, die nur gut oder schlecht sein können.
3. Prüfen Sie im Feld **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben Sie den **Positionsbereich** ein (die Zellen, in die die Sparklines kommen). Wählen Sie für jede Zeile mit Daten eine Positionszelle oder für jede Spalte mit Daten. Wählen Sie eine einzige Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und haben ein gemeinsames Erscheinungsbild. Klicken Sie auf eine Zelle mit einer Sparkline, dann erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und die Position der gesamten Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschieht (als Lücke lassen, als null behandeln oder die Punkte über sie hinweg verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **ersten Punkt**, den **letzten Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Satz Farben. **Sparkline-Farbe** legt die Farbe der Linie oder Balken fest, und **Stärke** legt die Dicke der Linie in Punkt fest. **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und den größten Wert der vertikalen Achse fest. **Automatisch für jede Sparkline** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine gemeinsame Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datumswerten: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten. Ein Punkt mit einem späteren Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die markierten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ein eigenes Erscheinungsbild. **Löschen** entfernt die markierten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das auch andere Tabellenkalkulationen lesen.

## Bild, Form oder Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet **Bild** in der Gruppe **Illustrationen** einen Dateiauswahldialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG). Sie landet auf dem Blatt an der ausgewählten Zelle. Ein Bild, das breiter als 480 Pixel ist, wird verkleinert, damit es passt, und behält seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert speichern kann (etwa ein SVG), wird als PNG gespeichert, also als einfaches Bild aus Pixeln. Eine Datei, die kein Bild ist, oder die größer als 16 MB ist, wird mit einer Meldung abgelehnt. Eine WMF- oder EMF-Datei (die Bildformate, die manche älteren Programme aus Linien und Formen erzeugen) wird ebenfalls gezeichnet und als PNG gespeichert.

**Symbole** zeigt dasselbe Fenster mit kleinen Strichgrafiken, das JeffPub und JeffDeck haben, mit einem Feld, in dem Sie nach Namen suchen können. Wählen Sie ein oder mehrere Symbole aus. Jedes landet als Bild mit 48 Punkt Breite und Höhe auf dem Blatt. Es ist dunkelgrau gezeichnet, in einer Größe, die beim Vergrößern scharf bleibt. Jedes Symbol ist ein einzelner Schritt, den Sie rückgängig machen können.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblase und eine Linie. Wählen Sie eine aus, dann landet sie hellblau mit einem dunkleren Rand auf dem Blatt. **Textfeld** in der Gruppe **Text** fügt ein leeres Feld ohne Füllung und ohne Rand ein.

Wenn eine Form ausgewählt ist, gibt **Formenarten** auf der Registerkarte **Formformat** ihr ein fertiges Aussehen: eine Füllung und eine Kontur, die zusammenpassen, etwa Hellblau mit einem dunkelblauen Rand. Die beiden letzten Einträge behalten nur die Kontur oder nur die Füllung bei. Ein Klick ist ein Schritt, den Sie rückgängig machen können. **Formfüllung** und **Formkontur** ändern jeweils einen Teil für sich.

Jede dieser Aktionen ist ein einzelner Schritt, den Sie rückgängig machen können. Das neue Objekt ist ausgewählt, sodass Sie es ziehen, in der Größe ändern, nach vorne oder hinten bringen, unter **Eigenschaften** einen Namen und Alternativtext vergeben oder die Taste **Entf** drücken können.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wenn Sie ein Bild auswählen, erscheint die Registerkarte **Bildformat**. Wenn Sie eine Form, eine Linie oder ein Textfeld auswählen, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Das Objekt behält dabei seinen Namen, seinen Alternativtext, seine Position und seine Reihenfolge zwischen den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Stärke (wie dick die Linie ist) und die Striche fest (durchgezogen, gestrichelt oder gepunktet). Ein Bild hat **Bildrahmen**, das ist seine Kontur.
- **Drehen** dreht das Objekt um eine Vierteldrehung nach rechts oder links oder spiegelt es horizontal oder vertikal.
- **Zuschneiden** schneidet ein Bild auf eine Fensterform zu: Quadrat, 4 breit und 3 hoch, 3 breit und 4 hoch, 16 zu 9 oder 9 zu 16. Der Ausschnitt wird gleichmäßig aus der Mitte des sichtbaren Bereichs gewählt, und das Bild wird so groß gemacht, wie der Rest es erlaubt. **Gesamtes Bild anzeigen** holt den abgeschnittenen Teil zurück. Die Bilddatei wird nicht verändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie den Text ein und wählen Sie, wie er waagerecht und senkrecht sitzt, die Größe, fett oder normal, und ob er in der Form umbricht oder in einer Zeile bleibt.
- **Auswahlbereich** öffnet eine Liste aller Objekte auf dem Blatt mit ihren Namen, vorne steht das oberste. Klicken Sie auf einen Namen, um dieses Objekt auszuwählen. Das hilft, wenn Objekte einander verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt Größe, Name, Alternativtext und fest, was das Objekt beim Verschieben oder Vergrößern von Zellen tut.

## Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden. JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe ein, auf einem beliebigen Blatt: Die Kopie liest dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest dort aber keine Zellen mehr, weil die Zellen dort nicht dieselben sind, aus denen es erstellt wurde.

## Mehrere Objekte auswählen, gruppieren und ausrichten

Klicken Sie ein Objekt an, halten Sie dann die Umschalttaste oder Strg gedrückt und klicken Sie ein weiteres an, um es ebenfalls auszuwählen. Klicken Sie ein ausgewähltes Objekt erneut bei gedrückter Umschalttaste oder Strg an, um es abzuwählen. Ausgewählte Objekte erhalten jeweils einen gestrichelten Rahmen. Sie können auch auf der Registerkarte **Formformat** **Objekte auswählen** wählen und ein Feld über leeren Zellen ziehen: Jedes Objekt, das vollständig in das Feld passt, wird ausgewählt. Drücken Sie Esc oder wählen Sie erneut **Objekte auswählen**, um wieder Zellen auszuwählen. Drücken Sie **Entf**, um alle ausgewählten Objekte mit einem Schritt zu entfernen.

Sind zwei oder mehr Objekte ausgewählt, enthält die Registerkarte **Formformat** in der Gruppe **Anordnen** folgende Befehle:

- **Gruppieren** fasst die ausgewählten Formen, Bilder und Linien zu einem Objekt zusammen. Es verschiebt und skaliert sich als Einheit, und die Datei speichert es als Gruppe. Ein Diagramm kann nicht gruppiert werden.
- **Gruppierung aufheben** löst eine ausgewählte Gruppe auf. Jeder Teil bleibt an seiner Stelle auf dem Blatt.
- **Ausrichten** richtet die ausgewählten Objekte aus: **Linksbündig ausrichten**, **Zentriert ausrichten**, **Rechtsbündig ausrichten**, **Oben ausrichten**, **Mitte ausrichten** und **Unten ausrichten**. Sie richten sich an der Kante oder der Mitte der gesamten Gruppe aus. **Horizontal verteilen** und **Vertikal verteilen** verteilen drei oder mehr Objekte gleichmäßig zwischen den beiden äußersten.

Gruppieren, Gruppierung aufheben und jede Ausrichtung sind je ein Schritt, den Sie rückgängig machen können.

## Benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie **Enter**. **Rückgängig** stellt den alten Namen wieder her, und der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie **Entf**, während das Objekt ausgewählt ist. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das auch andere Tabellenkalkulationen lesen, sodass ein Diagramm, das Sie hier erstellen, dort genauso geöffnet wird. Ein Diagramm aus einem anderen Programm, das Elemente enthält, die JeffCrunch beim Umschreiben nicht erhalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich zwar verschieben, in der Größe ändern und löschen. Seine Elemente und Daten werden hier aber nicht verändert.

## Diagramme aus anderen Programmen

Ein **Diagramm** im Sinne eines Schemas ist ein Bild aus Kästchen und Pfeilen, etwa ein Ablauf, das ein anderes Programm erstellt hat. JeffCrunch zeigt ein solches Diagramm anhand des Bildes an, das die Datei daneben speichert. Klicken Sie es an, um es auszuwählen, und ziehen Sie es, um es zu verschieben. Beim Speichern der Datei wird das Diagramm unverändert zurückgeschrieben, sodass das Programm, das es erstellt hat, es weiterhin bearbeiten kann.

JeffCrunch kann den Inhalt eines Diagramms derzeit nicht ändern, und ein solches Diagramm kann weder kopiert noch in einer Gruppe zusammengefasst werden. Speichert die Datei kein Bild des Diagramms, zeigt JeffCrunch ein schlichtes graues Feld, das angibt, dass es sich um ein Diagramm handelt.
