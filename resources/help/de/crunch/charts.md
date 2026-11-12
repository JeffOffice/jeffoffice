# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Feldschaltflächen, Schaltflächen für Pivot-Diagramme, Gruppieren, Gruppierung aufheben, ausrichten, linksbündig ausrichten, verteilen, Objekte auswählen, mehrere Formen, mehrere auswählen, Auswahlrahmen, Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Streudiagramm, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Diagrammdarstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Trendlinie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skalierung, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Symbole, Icon, wmf, emf, Formformatvorlagen, Formatvorlagen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern, Diagramm (Schema), Diagramme, Online-Bilder, freie Bilder, Openverse, Wikimedia Commons, Lizenz, Quellenangabe, Textkunst, Fontwork, Verzerrung, Bilder komprimieren, Bilder verkleinern, Dateigröße, ppi, zugeschnittene Bereiche löschen, Barcode, QR-Code -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Ändern Sie eine Zahl, ändert sich das Diagramm mit. Bilder und Formen liegen ähnlich über den Zellen, sind aber nicht an Zahlen gebunden.

## Ein Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Enthalten die erste Zeile Namen und die erste Spalte Beschriftungen, markieren Sie diese ebenfalls. JeffCrunch betrachtet die erste Zeile und Spalte und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ aus: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt (XY)** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt dessen Varianten, etwa gestapelte Säulen, eine Linie mit Markierungen oder ein Punkt-Diagramm mit Punkten, verbundenen Punkten oder interpolierten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ, gezeichnet aus Ihren Zellen. Mit **Zeilen und Spalten tauschen** drehen Sie die Datenreihen um.

Sie wissen nicht, welcher Typ passt? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch prüft die Zellen und schlägt einige passende Typen vor, der beste zuerst. Stehen beispielsweise Datumswerte untereinander, wird eine Linie vorgeschlagen; eine Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Markieren Sie nur eine Zelle, verwendet JeffCrunch den Zellbereich um sie herum, der Inhalte enthält.

Datenreihen verlaufen in Spalten, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und in Zeilen, wenn sie mehr Spalten als Zeilen hat. Ein Kreis enthält eine Datenreihe. Ein Punkt-Diagramm übernimmt die waagerechten Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, der Rest sind Zahlen. Das Diagramm hat zwei Datenreihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Datenreihe vorübergehend ausblenden

Wählen Sie ein Diagramm aus. Daneben erscheinen drei kleine Schaltflächen. Die Schaltfläche mit dem Trichter ist **Diagrammfilter**. Sie zeigt die Datenreihen des Diagramms, jede mit einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um die Reihe aus dem Diagramm zu nehmen. Aktivieren Sie es erneut, kehrt sie an der gleichen Stelle zurück. **Rückgängig** funktioniert ebenfalls. Ein Diagramm behält immer mindestens eine Datenreihe. Eine ausgeblendete Reihe bleibt beim Speichern in der Datei erhalten und bleibt beim Öffnen ausgeblendet, es geht also nichts verloren. Aktivieren Sie das Kontrollkästchen, um sie wieder einzublenden. (Ein Ringdiagramm zeigt beim Öffnen alle Datenreihen.)

## Verschieben und Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie beim Ziehen **Alt** gedrückt, damit eine Kante an der Kante einer Zelle ausgerichtet wird.
- Halten Sie beim Ziehen einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt einige Pixel auf einmal. **Strg** mit einer Pfeiltaste verschiebt es um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an und wählen Sie **Eigenschaften**, um seinen **Namen** einzugeben (jedes Objekt auf einem Blatt braucht einen eigenen) und seinen **Alternativtext** (ein kurzer Satz, der Personen, die das Objekt nicht sehen können, erklärt, was es zeigt). Dort legen Sie außerdem die Größe fest und bestimmen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mit den Zellen verschieben und in der Größe anpassen, mit den Zellen verschieben, aber Größe beibehalten, oder an Ort und Stelle bleiben. Die Wahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Fenster schaltet das Objekt auf den gedruckten Seiten aus (auf dem Bildschirm bleibt es sichtbar); die Datei speichert auch diese Einstellung.

## Festlegen, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Titel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinselschaltfläche (und **Farben** sowie **Diagrammformatvorlagen** auf der Registerkarte) gibt dem Diagramm einen der Farbsätze und eines der Erscheinungsbilder von JeffCrunch. **Layouts** setzt mehrere Elemente auf einmal.
- **Diagrammdaten** listet die Datenreihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen und festlegen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Feldschaltflächen** (im Menü **+**, nur für ein Pivot-Diagramm) zeigen kleine Schaltflächen mit den Namen der Felder der Pivot-Tabelle, der das Diagramm folgt: die Berichtsfilter unter dem Titel sowie die Zeilen-, Werte- und Spaltenfelder am unteren Rand. Schalten Sie jede Art ein oder aus oder wählen Sie **Alle anzeigen** oder **Alle ausblenden**. Ein neues Pivot-Diagramm zeigt sie an.
- **Fehlerindikatoren** (im Menü **+**, für ein Säulen-, Balken-, Linien-, Flächen- oder Punkt-Diagramm) zeichnen über und unter jedem Punkt eine kleine Linie, die zeigt, wie stark der tatsächliche Wert abweichen könnte. Wählen Sie **Standardfehler**, **Prozentsatz (5 %)**, **Standardabweichung** oder **Fester Wert (1)**. Wählen Sie **Benutzerdefiniert**, um eigene Beträge zu verwenden: Geben Sie die Zellen ein, die die Beträge oberhalb jedes Punkts enthalten (etwa `=Tabelle1!$D$2:$D$4`), danach die Zellen für die Beträge unterhalb. Lassen Sie eine Angabe leer, um die Indikatoren nur auf einer Seite zu zeichnen. Die Indikatoren ändern sich, wenn sich diese Zellen ändern. Eine Datei, die eigene Beträge für jeden Punkt enthält, behält sie.
- **Linien** (im Menü **+**, für ein Liniendiagramm) enthält **Tropfenlinien**, die vom höchsten Punkt jeder Kategorie bis zur Achse reichen, **Hoch-Tief-Linien**, die den höchsten und den niedrigsten Punkt jeder Kategorie verbinden, und **Auf-/Ab-Balken**, Kästchen zwischen der ersten und der letzten Datenreihe jeder Kategorie. Ein Aufwärtsbalken zeigt an, dass die letzte Reihe höher liegt. Hoch-Tief-Linien und Auf-/Ab-Balken brauchen zwei oder mehr Datenreihen. Flächendiagramme haben Tropfenlinien.
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Datenreihe angepasst ist, damit Sie sehen, wohin sie sich entwickeln. Wählen Sie **Linear** für eine Gerade, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die zuerst schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung, 0 bedeutet gar keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wird eine Datenreihe an einer zweiten senkrechten Achse dargestellt, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite waagerechte** und **Zweite senkrechte** für diese Reihe. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende über die Zeichenfläche legt, statt sie daneben anzuordnen.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Datenreihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Datenreihe, in beliebiger Kombination), **Trennzeichen** (was dazwischen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (einen Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen unverändert anzuzeigen). Mit **Werte aus Zellen…** zeigen Sie den Text einiger Zellen als Beschriftungen an, eine Zelle für jeden Punkt. Geben Sie die Zellen ein, etwa `=Tabelle1!$D$2:$D$4`. Die Beschriftungen ändern sich, wenn sich die Zellen ändern. Lassen Sie das Feld leer, um sie zu entfernen. Schalten Sie alle Inhalte aus, verschwinden auch die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Fenster für jeweils eine Achse. Es legt den kleinsten und den größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eine von Ihnen eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge laufen, die Anzeigeeinheiten (die Beschriftungen zeigen die Zahlen geteilt, etwa durch 1.000 für Tausender, sodass 30.000 als 30 erscheint; aktivieren Sie **Name der Einheiten im Diagramm anzeigen**, damit der Name, etwa Tausend, zusammen mit dem Achsentitel gedruckt wird), ob die Skala logarithmisch ist (jede Markierung ist ein Vielfaches der vorherigen, etwa 1, 10, 100 und 1.000; das eignet sich für Zahlen, die sehr unterschiedlich groß sind und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt, wohin das Diagramm soll. Wählen Sie **Neues Blatt** und geben Sie einen Namen ein, dann erhält das Diagramm ein eigenes Diagrammblatt: ein Blatt, das nur aus dem Diagramm besteht und keine Zellen hat. Wählen Sie **Objekt in**, und wählen Sie ein Arbeitsblatt aus, dann kommt das Diagramm auf dieses Arbeitsblatt. Ein Diagramm auf einem Diagrammblatt kann nur auf ein Arbeitsblatt verschoben werden; das Diagrammblatt, das dabei frei wird, wird gelöscht. Das Diagramm liest weiterhin die Zellen, die es vorher gelesen hat. Auf einem Arbeitsblatt behält es seine Größe und seine Position auf dem Blatt, auch wenn Zeilen und Spalten dort höher oder breiter sind, und es verschiebt und verändert seine Größe mit den Zellen dort wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Das Verschieben ist ein Schritt für **Rückgängig**, auch nach dem Speichern, außer wenn dabei ein Blatt gelöscht wurde. Eine Datei im Format `.ods` kann kein Diagrammblatt enthalten. Speichern Sie ein Diagrammblatt daher im Format `.xlsx`.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle. Ändert sich die Zelle, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) bewahren das Erscheinungsbild eines Diagramms in einer **.crtx**-Datei auf und übertragen es auf ein anderes Diagramm. Zum Erscheinungsbild gehören Typ, Farben, Einstellungen für Titel und Legende, Achsen und Schriftarten. Das Diagramm, auf das Sie es anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein **Rückgängig** entfernt das Erscheinungsbild wieder.
- **Diagrammtyp ändern** ändert den Typ für das ganze Diagramm oder für jede Datenreihe: Säulen mit einer darüberliegenden Linie oder eine Datenreihe an einer zweiten senkrechten Achse.

Ein Zellbezug wird etwa als **Tabelle1!$B$2:$B$9** geschrieben; enthält der Blattname Leerzeichen, steht er in einfachen Anführungszeichen.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben ihnen.

1. Markieren Sie die Zahlen. Möchten Sie für jede Zeile eine Sparkline, markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl unter null einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sein können.
3. Prüfen Sie im Fenster **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben Sie den **Positionsbereich** ein (die Zellen, in denen sie stehen). Wählen Sie für jede Datenzeile oder jede Datenspalte eine Zelle. Wählen Sie eine einzelne Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und teilen sich ein Erscheinungsbild. Klicken Sie auf eine Zelle mit einer Sparkline, erscheint die Registerkarte **Sparklines**.

- **Daten bearbeiten** ändert die Daten und den Standort der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschieht (als Lücke darstellen, als null behandeln oder die Punkte darüber verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Höchstpunkt**, den **Tiefstpunkt**, **Negative Punkte**, den **ersten Punkt**, den **letzten Punkt** und bei einer Linie **Markierungen** auf jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Farbsatz. **Sparkline-Farbe** legt die Farbe der Linie oder der Balken fest, und **Stärke** (wie dick die Linie ist, in Punkt). **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und den größten Wert der senkrechten Achse fest. **Automatisch für jede Sparkline** passt jede an ihre eigenen Zahlen an. **Gleich für alle Sparklines** gibt der ganzen Gruppe eine gemeinsame Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit späterem Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ihr eigenes Erscheinungsbild. **Löschen** entfernt die ausgewählten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das auch andere Tabellenkalkulationen lesen.

## Ein Bild, eine Form oder ein Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet **Bild** in der Gruppe **Illustrationen** einen Dateiauswahldialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG), und sie landet auf dem Blatt in der ausgewählten Zelle. Ein Bild, das breiter als 480 Punkte ist, wird verkleinert, damit es passt, und behält seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert speichern kann (etwa SVG), wird als PNG gespeichert, ein einfaches Bild aus Punkten. Eine Datei, die kein Bild ist oder größer als 16 MB, wird mit einer Meldung abgelehnt. Eine WMF- oder EMF-Datei (Bildformate, die manche älteren Programme aus Linien und Formen erzeugen) wird ebenfalls gezeichnet und als PNG gespeichert.

**Symbole** zeigt dieselbe Auswahl kleiner Linienbilder, die JeffPub und JeffDeck haben, mit einem Feld zur Suche nach Namen. Wählen Sie eines oder mehrere aus; jedes landet als Bild mit 48 Punkt Breite und Höhe auf dem Blatt. Es wird in dunklem Grau gezeichnet, in einer Größe, die beim Vergrößern scharf bleibt. Jedes Symbol ist ein Schritt, den Sie rückgängig machen können.

**Barcode** öffnet ein Fenster, in dem Sie einen Barcode erstellen: Buch-ISBN (mit oder ohne Preis), EAN-13, UPC-A, EAN-8, Code 128, Code 39 oder ein QR-Code, der beliebigen Text enthält, etwa eine Webadresse. Wählen Sie einen **Typ**, geben Sie den Text oder die Ziffern ein; das Fenster zeigt den Barcode während der Eingabe an und nennt in Rot, was nicht stimmt, falls er nicht erstellt werden kann. Klicken Sie auf **Einfügen**, dann landet er als Bild auf dem Blatt; das ist ein Schritt, den Sie rückgängig machen können. Verschieben und skalieren Sie ihn wie jedes Bild. Zum Ändern markieren Sie ihn und klicken erneut auf **Barcode**, ändern den Text und klicken auf **Aktualisieren**; der neue Barcode nimmt den Platz und die Breite des alten ein. Beim Speichern wird er als Bild mit einer scharf bleibenden SVG-Grafik gespeichert, sodass ihn jedes Programm anzeigt. JeffCrunch bewahrt seine Einstellungen in der Datei auf, sodass Sie ihn beim erneuten Öffnen der Arbeitsmappe in JeffCrunch weiterhin bearbeiten können.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblasen-Legende und eine Linie. Wählen Sie eine aus, und sie landet hellblau mit dunklerem Rand auf dem Blatt. **Textfeld** in der Gruppe **Text** fügt ein leeres Kästchen ohne Füllung und ohne Rand ein.

Ist eine Form ausgewählt, gibt **Formatvorlagen** auf der Registerkarte **Formformat** ihr ein fertiges Erscheinungsbild: eine Füllung und eine Kontur, die zusammenpassen, etwa Hellblau mit dunkelblauem Rand. Die beiden letzten Einträge behalten nur die Kontur oder nur die Füllung. Ein Klick ist ein Schritt, den Sie rückgängig machen können. **Formfüllung** und **Formkontur** ändern jeden Teil für sich.

## Kostenlose Bilder im Internet finden

**Online-Bilder** in der Gruppe **Illustrationen** der Registerkarte **Einfügen** findet kostenlose Bilder. Sie stammen aus zwei offenen Bibliotheken: Openverse, das viele Sammlungen durchsucht, und Wikimedia Commons, der Bildbibliothek von Wikipedia.

1. Klicken Sie auf **Online-Bilder…**. Rechts öffnet sich ein Bereich.
2. Wählen Sie eine Bibliothek, geben Sie ein, wonach Sie suchen, und klicken Sie auf **Suchen** (oder drücken Sie die Eingabetaste).
3. Klicken Sie auf ein Bild, um zu sehen, wer es gemacht hat und welche Lizenz es hat. Eine **Lizenz** legt fest, wie Sie ein Bild verwenden dürfen. Klicken Sie auf den Namen der Lizenz, um sie zu lesen.
4. Klicken Sie auf **Einfügen** (oder doppelklicken Sie auf das Bild).

**Was JeffCrunch sendet.** Ihre Suchbegriffe gehen an die von Ihnen gewählte Bibliothek, und zwar nur, wenn Sie auf **Suchen** klicken oder die Eingabetaste drücken. JeffCrunch fordert danach von der Bibliothek die kleinen Vorschaubilder der Treffer und das eine Bild an, das Sie einfügen. Wie jede Webseite kann die Bibliothek die Internetadresse Ihres Computers sehen, und sie erfährt, dass JeffCrunch diese Anfrage stellt. Aus Ihrer Arbeitsmappe wird nichts gesendet.

Das Bild landet auf dem Blatt wie ein Bild aus einer Datei, und die Arbeitsmappe speichert es. Sein Titel wird als **Alternativtext** gespeichert. Die meisten Lizenzen verlangen, dass Sie den Urheber nennen. Ist **Quellenangabe unter dem Bild hinzufügen** aktiviert, fügt JeffCrunch unter dem Bild ein kleines Textfeld mit Titel, Urheber und Lizenz ein. Sie können es verschieben, ändern oder löschen. Bild und Quellenangabe werden in einem Schritt eingefügt, daher nimmt **Rückgängig** beides zurück. Prüfen Sie, ob die Lizenz zu Ihrer Verwendung passt.

## Textkunst hinzufügen

**Textkunst** sind kunstvoll gestaltete Wörter: Buchstaben mit Farbe, Kontur, Schatten oder einer gebogenen Form.

1. Klicken Sie auf **Einfügen > Text > Textkunst**. Eine Galerie mit Formatvorlagen öffnet sich.
2. Klicken Sie auf eine Formatvorlage. Der Text „Ihr Text hier“ landet im gewählten Stil auf dem Blatt.
3. Um die Wörter zu ändern, wählen Sie die Textkunst aus und klicken auf der Registerkarte **Formformat** auf **Text bearbeiten…**. Ein Fenster fragt nach den Wörtern, der Schriftart, der Größe, fett und kursiv. Eine größere Schrift macht die Textkunst größer.
4. Auf der Registerkarte **Formformat** gibt **Textkunst-Formatvorlagen** den Buchstaben einen anderen Stil, und **Textverzerrung** biegt die Wörter in eine Form, etwa eine Welle oder einen Bogen. „Keine“ lässt sie gerade.

Textkunst ist eine Form in der Zeichnung des Blatts, mit ihrer Verzerrung, Füllung und Kontur. Sie bleibt beim Speichern der Arbeitsmappe als .xlsx-Datei und beim erneuten Öffnen in JeffCrunch oder in einem anderen Programm erhalten. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Eine als .ods-Datei gespeicherte Arbeitsmappe behält ihre Bilder, Formen, Textfelder, Gruppen, Linien und Textkunst ebenfalls. Textkunst wird als Fontwork-Form gespeichert, die OpenDocument kennt, mit ihren Wörtern, der Schriftart, Füllung, Kontur und Verzerrung, und kehrt beim erneuten Öffnen der .ods-Datei als Textkunst zurück. Einige Verzerrungen, für die OpenDocument keine Form kennt (Welle 2, Welle 4 sowie die unteren und oberen Formen von Aufblähen und Zusammendrücken), werden als gerade Wörter mit denselben Buchstaben und demselben Stil gespeichert. Diagramme werden ebenfalls in einer .ods-Datei erhalten; siehe unten.

## Bilder verkleinern

Ein Foto aus einer Kamera kann viel größer sein, als es auf dem Blatt angezeigt wird, und macht die Datei groß. Klicken Sie auf ein Bild und dann in der Gruppe **Anpassen** der Registerkarte **Bildformat** auf **Bilder komprimieren…**. Wählen Sie, wo die Arbeitsmappe verwendet werden soll: **Professioneller Druck (300 ppi)**, **Desktop-Druck (220 ppi)** oder **Web (96 ppi)**. Lassen Sie **Auf alle Bilder auf diesem Blatt anwenden** aktiviert, um alle Bilder auf dem Blatt zu verkleinern, oder deaktivieren Sie es, um nur das ausgewählte Bild zu verkleinern. **Zugeschnittene Bereiche von Bildern löschen** entfernt außerdem die verborgenen Teile, die Sie zugeschnitten haben; der Zuschnitt entfällt dann. Ein Bild wird nur ersetzt, wenn das neue kleiner ist. Größe, Position, Name und Alternativtext bleiben unverändert. Mit Strg+Z (Rückgängig) erhalten Sie die Bilder zurück, und die gespeicherte Arbeitsmappe enthält nur die kleineren Bilder. Bilder auf anderen Blättern werden nicht geändert: Wechseln Sie auf jedes Blatt und wiederholen Sie den Vorgang. Ein Bild, das eine Zeichnung ist (SVG, EMF oder WMF), oder eine Animation bleibt unverändert.

Jede Aktion ist ein Schritt, den Sie rückgängig machen können. Das neue Objekt ist ausgewählt, sodass Sie es ziehen, in der Größe ändern, nach vorne oder hinten verschieben, unter **Eigenschaften** benennen und mit Alternativtext versehen oder **Entf** drücken können.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wählen Sie ein Bild aus, erscheint die Registerkarte **Bildformat**. Wählen Sie eine Form, eine Linie oder ein Textfeld aus, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können, und das Objekt behält seinen Namen, seinen Alternativtext, seine Position und seine Reihenfolge unter den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Strichstärke (wie dick die Linie ist) und die Striche fest (durchgezogen, gestrichelt oder gepunktet). Ein Bild hat **Bildrahmen**, das seine Kontur ist.
- **Drehen** dreht das Objekt um eine Viertelumdrehung nach rechts oder links oder spiegelt es horizontal oder vertikal.
- **Zuschneiden** schneidet ein Bild auf eine Fensterform zu: Quadrat, 4 breit mal 3 hoch, 3 breit mal 4 hoch, 16 mal 9 oder 9 mal 16. Der Zuschnitt erfolgt gleichmäßig von der Mitte aus, und das Bild wird so groß gemacht, wie der verbleibende Teil es zulässt. **Ganzes Bild anzeigen** holt den abgeschnittenen Teil zurück. Die Bilddatei wird nicht verändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie die Wörter ein und wählen Sie, wie sie waagerecht und senkrecht ausgerichtet sind, die Größe, fett oder normal, und ob sie sich an die Form anpassen oder in einer Zeile bleiben. Bei Textkunst fragt das Fenster stattdessen nach den Wörtern, der Schriftart, der Größe, fett und kursiv.
- **Auswahlbereich** öffnet eine Liste aller Objekte des Blatts mit ihren Namen, das vorderste zuerst. Klicken Sie auf einen Namen, um das Objekt auszuwählen; das hilft, wenn Objekte einander verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt Größe, Namen, Alternativtext und fest, was das Objekt beim Verschieben oder Vergrößern von Zellen tut.

## Ein Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird genauso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden; JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe ein, auf einem beliebigen Blatt: Die Kopie liest dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es angezeigt hat, liest aber keine Zellen mehr, weil die Zellen dort nicht die sind, aus denen es erstellt wurde.

## Mehrere Objekte auswählen, gruppieren und ausrichten

Klicken Sie auf ein Objekt und halten Sie dann Umschalt oder Strg gedrückt, während Sie ein weiteres anklicken, um es ebenfalls auszuwählen. Klicken Sie ein ausgewähltes Objekt erneut mit Umschalt oder Strg an, um es abzuwählen. Ausgewählte Objekte erhalten jeweils einen gestrichelten Rahmen. Sie können auch auf der Registerkarte **Formformat** **Objekte auswählen** wählen und einen Kasten über leere Zellen ziehen: Jedes Objekt, das vollständig im Kasten liegt, wird ausgewählt. Drücken Sie Esc oder wählen Sie **Objekte auswählen** erneut, um wieder Zellen auszuwählen. Drücken Sie Entf, um alle ausgewählten Objekte in einem Schritt zu entfernen.

Sind zwei oder mehr Objekte ausgewählt, enthält die Registerkarte **Formformat** in der Gruppe **Anordnen** folgende Befehle:

- **Gruppieren** fasst die ausgewählten Formen, Bilder und Linien zu einem Objekt zusammen. Es wird als Einheit verschoben und in der Größe verändert, und die Datei speichert es als Gruppe. Ein Diagramm kann nicht gruppiert werden.
- **Gruppierung aufheben** löst eine ausgewählte Gruppe auf. Jeder Teil kehrt an seine Stelle auf dem Blatt zurück.
- **Ausrichten** richtet die ausgewählten Objekte aus: **Linksbündig**, **Zentriert**, **Rechtsbündig**, **Oben ausrichten**, **Mitte ausrichten** und **Unten ausrichten**. Sie richten sich an der Kante oder Mitte der gesamten Auswahl aus. **Horizontal verteilen** und **Vertikal verteilen** verteilen drei oder mehr Objekte gleichmäßig zwischen den beiden äußersten.

Gruppieren, Gruppierung aufheben und jede Ausrichtungsoption sind jeweils ein Schritt, den Sie rückgängig machen können.

## Benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie die Eingabetaste. Rückgängig setzt den alten Namen wieder ein, und der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie Entf, während das Objekt ausgewählt ist. Rückgängig holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das auch andere Tabellenkalkulationen lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und Elemente enthält, die JeffCrunch beim Neuschreiben nicht behalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), kann weiterhin verschoben, in der Größe verändert und gelöscht werden, aber seine Elemente und Daten werden hier nicht verändert.

## Diagramme aus anderen Programmen

Ein **Schema** ist ein Bild aus Kästchen und Pfeilen, etwa ein Ablauf, das ein anderes Programm erstellt hat. JeffCrunch zeigt ein Schema anhand des Bilds an, das die Datei daneben speichert. Klicken Sie darauf, um es auszuwählen, und ziehen Sie es, um es zu verschieben. Beim Speichern der Datei wird das Schema unverändert zurückgeschrieben, sodass das Programm, das es erstellt hat, es weiterhin bearbeiten kann.

JeffCrunch kann den Inhalt eines Schemas noch nicht ändern. Ein Schema kann nicht kopiert oder in eine Gruppe aufgenommen werden. Enthält die Datei kein Bild eines Schemas, zeigt JeffCrunch ein einfaches graues Feld, das besagt, dass es sich um ein Schema handelt.

## Diagramme in .ods-Dateien

Eine als .ods-Datei gespeicherte Arbeitsmappe behält ihre Diagramme: Säulen-, Balken-, Linien-, Flächen-, Kreis-, Ring- und Punkt-Diagramme, jeweils mit ihren Datenreihen und den Zellen, die sie lesen (auf diesem oder einem anderen Blatt), Kategorien, Titel, Legende, Achsen (Titel, kleinster und größter Wert, Zahlenformat), Datenbeschriftungen, Farben und Position. Eine .ods-Datei, die ein anderes Programm erstellt hat, wird mit ihren Diagrammen geöffnet; sie werden anhand der Zellen gezeichnet, die sie lesen. Was ein .ods-Diagramm nicht unterbringen kann, wird beim Speichern weggelassen und benannt: Trendlinien, Fehlerindikatoren, zweite Achsen und die besonderen Darstellungen einzelner Punkte. Diagrammtypen, die JeffCrunch nicht zeichnet (Börsen- und Netzdiagramme), werden als Säulen ihrer Werte geöffnet.
