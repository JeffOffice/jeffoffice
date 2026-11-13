# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Feldschaltflächen, Schaltflächen für Pivot-Diagramme, Gruppieren, Gruppierung aufheben, ausrichten, linksbündig ausrichten, verteilen, Objekte auswählen, mehrere Formen, mehrere auswählen, Auswahlrahmen, Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Punkt, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Darstellung, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Trendlinie, Trend, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skala, als Bild speichern, Vorlage, crtx, Titeltext, Legendeneintrag, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Symbole, Symbol, wmf, emf, Formenarten, Formatvorlagen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern, Schaubild, Diagramme, Onlinebilder, freie Bilder, Openverse, Wikimedia Commons, Lizenz, Quellenangabe, Textkunst, Fontwork, Verzerrung, Bilder komprimieren, Bilder verkleinern, Dateigröße, ppi, gekürzte Bereiche löschen, Barcode, QR-Code -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Wenn Sie eine Zahl ändern, ändert sich das Diagramm mit. Bilder und Formen liegen ebenso über den Zellen, sind aber nicht an Zahlen gebunden.

## Ein Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch sieht sich die erste Zeile und Spalte an und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Punkt** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt dessen Varianten, etwa gestapelte Säulen, eine Linie mit Markierungen oder ein Punktdiagramm mit einzelnen Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ, gezeichnet aus Ihren Zellen. Mit **Zeilen und Spalten tauschen** drehen Sie die Reihen um.

Sie wissen nicht, welcher Typ passt? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch prüft die Zellen und schlägt einige passende Typen vor, der beste zuerst. Stehen etwa Datumswerte untereinander, wird eine Linie vorgeschlagen, und eine Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Wenn Sie nur eine Zelle markieren, stellt JeffCrunch den Zellbereich um sie herum dar, der Inhalt hat.

Reihen verlaufen spaltenweise, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und zeilenweise, wenn sie mehr Spalten als Zeilen hat. Ein Kreis enthält genau eine Reihe. Ein Punktdiagramm übernimmt die Werte für die horizontale Achse aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, der Rest sind Zahlen. Das Diagramm hat zwei Reihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Reihe vorübergehend ausblenden

Wählen Sie ein Diagramm aus. Daneben erscheinen drei kleine Schaltflächen. Die Schaltfläche mit dem Trichter ist **Diagrammfilter**. Sie zeigt die Reihen des Diagramms, jede mit einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um die Reihe aus dem Diagramm zu entfernen. Aktivieren Sie es erneut, um sie an derselben Stelle wieder einzublenden. **Rückgängig** funktioniert ebenfalls. Ein Diagramm behält immer mindestens eine Reihe. Eine ausgeblendete Reihe bleibt beim Speichern in der Datei erhalten und bleibt beim Öffnen ausgeblendet, es geht also nichts verloren. Zum Einblenden aktivieren Sie das Kontrollkästchen wieder. (Ein Ringdiagramm zeigt beim Öffnen immer alle Reihen.)

## Verschieben und in der Größe ändern

Klicken Sie auf ein Diagramm, ein Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie an einem der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie **Alt** beim Ziehen gedrückt, damit eine Kante an einer Zellkante ausgerichtet wird.
- Halten Sie beim Ziehen an einer Ecke **Umschalt** gedrückt, um die Proportionen beizubehalten.
- Mit den Pfeiltasten verschieben Sie das Objekt jeweils um einige Pixel. **Strg** mit einer Pfeiltaste verschiebt um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben und jede Größenänderung ist ein Schritt, den Sie rückgängig machen können. Klicken Sie das Objekt mit der rechten Maustaste an und wählen Sie **Eigenschaften**. Dort geben Sie den **Namen** ein (jedes Objekt auf einem Blatt braucht einen eigenen) und den **Alternativtext** (einen kurzen Satz, der Menschen, die das Objekt nicht sehen können, sagt, was es zeigt). Außerdem legen Sie die Größe fest und bestimmen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, aber die Größe beibehalten, oder an der Stelle bleiben. Die Wahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Feld schaltet das Objekt auf den gedruckten Seiten aus (auf dem Bildschirm bleibt es sichtbar); auch diese Wahl speichert die Datei.

## Ändern, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Diagrammtitel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinselschaltfläche (und **Farben** sowie **Darstellung** auf der Registerkarte) gibt dem Diagramm eines der Farbschemata und eine der Darstellungen von JeffCrunch. **Layouts** setzt mehrere Elemente auf einmal.
- **Diagrammdaten** listet die Reihen des Diagramms auf. Sie können Reihen hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede Reihe liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten tauschen und festlegen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Feldschaltflächen** (im Menü **+**, nur bei einem Pivot-Diagramm) zeigt kleine Schaltflächen mit den Namen der Felder der Pivot-Tabelle, der das Diagramm folgt: die Berichtsfilter unter dem Titel sowie die Zeilen, Werte und Spalten am unteren Rand. Schalten Sie jede Art ein oder aus oder wählen Sie **Alle anzeigen** oder **Alle ausblenden**. Ein neues Pivot-Diagramm zeigt sie an.
- **Fehlerindikatoren** (im Menü **+**, für Säulen-, Balken-, Linien-, Flächen- oder Punktdiagramme) zeichnen oberhalb und unterhalb jedes Punktes eine kleine Linie, die zeigt, wie stark der tatsächliche Wert abweichen könnte. Wählen Sie **Standardfehler**, **Prozentsatz (5 %)**, **Standardabweichung** oder **Fester Wert (1)**. Mit **Benutzerdefiniert** verwenden Sie eigene Beträge: Geben Sie die Zellen ein, die die Beträge oberhalb jedes Punktes enthalten (zum Beispiel `=Tabelle1!$D$2:$D$4`), danach die Zellen für die Beträge unterhalb. Lassen Sie eine Angabe leer, um die Balken nur auf einer Seite zu zeichnen. Die Balken ändern sich, wenn sich diese Zellen ändern. Eine Datei, die eigene Beträge für jeden Punkt enthält, behält sie.
- **Linien** (im Menü **+**, für ein Liniendiagramm) bietet **Tropfenlinien**, die vom höchsten Punkt jeder Kategorie bis zur Achse reichen, **Hoch-Tief-Linien**, die den höchsten und den niedrigsten Punkt jeder Kategorie verbinden, und **Aufwärts-/Abwärtsbalken**, Kästchen zwischen der ersten und der letzten Reihe jeder Kategorie. Ein Aufwärtsbalken zeigt, dass die letzte Reihe höher liegt. Hoch-Tief-Linien und Aufwärts-/Abwärtsbalken brauchen mindestens zwei Reihen. Flächendiagramme haben Tropfenlinien.
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Reihe angepasst ist, damit Sie sehen, wohin sie tendieren. Wählen Sie **Linear** für eine gerade Linie, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Biegungen, **Potenz** oder **Gleitender Durchschnitt**, der den Mittelwert der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung, 0 bedeutet keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wird eine Reihe an einer zweiten vertikalen Achse dargestellt, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite horizontale** und **Zweite vertikale** für diese Reihe. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende über die Zeichenfläche legt, statt sie daneben anzuordnen.
- **Datenbeschriftungen** bietet außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Reihe, den Prozentsatz und den Legendeneintrag, ein kleines Quadrat in der Farbe der Reihe, in beliebiger Kombination), **Trennzeichen** (was dazwischen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (einen Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen unverändert anzuzeigen). **Werte aus Zellen…** zeigt den Text einiger Zellen als Beschriftungen, eine Zelle für jeden Punkt. Geben Sie die Zellen ein, etwa `=Tabelle1!$D$2:$D$4`. Die Beschriftungen ändern sich, wenn sich die Zellen ändern. Lassen Sie das Feld leer, um sie zu entfernen. Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Feld für jeweils eine Achse. Dort legen Sie den kleinsten und den größten Wert fest sowie die Haupt- und Hilfsintervalle (jeweils automatisch oder eine Zahl, die Sie eingeben), ob die Werte in umgekehrter Reihenfolge verlaufen, die Anzeigeeinheiten (die Beschriftungen zeigen die Zahlen geteilt, etwa durch 1.000 für Tausender, sodass 30.000 als 30 erscheint; aktivieren Sie **Name der Einheiten im Diagramm anzeigen**, um den Namen, etwa Tausend, zusammen mit dem Achsentitel zu drucken), ob die Skala logarithmisch ist (jede Markierung ist ein Vielfaches der vorherigen, etwa 1, 10, 100 und 1.000; das eignet sich für Zahlen, die sich stark in der Größe unterscheiden und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt, wohin das Diagramm soll. Wählen Sie **Neues Blatt** und geben Sie einen Namen ein, dann erhält das Diagramm ein eigenes Diagrammblatt: ein Blatt, das nur aus dem Diagramm besteht und keine Zellen hat. Wählen Sie **Objekt in** und ein Tabellenblatt, dann kommt das Diagramm auf dieses Tabellenblatt. Ein Diagramm auf einem Diagrammblatt kann nur auf ein Tabellenblatt verschoben werden; das verlassene Diagrammblatt wird gelöscht. Das Diagramm liest weiterhin dieselben Zellen. Auf einem Tabellenblatt behält es seine Größe und seine Position auf dem Blatt, auch wenn Zeilen und Spalten dort höher oder breiter sind, und es verschiebt und ändert seine Größe mit den Zellen dort. Ausgeblendete Blätter werden nicht angeboten. Der Vorgang ist ein einziger Schritt für **Rückgängig**, auch nach dem Speichern, es sei denn, dadurch wurde ein Blatt gelöscht. Eine Datei im Format `.ods` kann kein Diagrammblatt enthalten. Speichern Sie ein Diagrammblatt daher im Format `.xlsx`.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle. Ändert sich die Zelle, ändert sich auch der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) bewahren die Darstellung eines Diagramms in einer **.crtx**-Datei auf und übertragen sie auf ein anderes Diagramm. Zur Darstellung gehören Typ, Farben, Einstellungen für Titel und Legende, die Achsen und die Schriftarten. Das Diagramm, auf das Sie die Vorlage anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein **Rückgängig** nimmt die Darstellung wieder zurück.
- **Diagrammtyp ändern** ändert den Typ für das ganze Diagramm oder für jede Reihe einzeln, etwa Säulen mit einer Linie darüber oder eine Reihe an einer zweiten vertikalen Achse.

Ein Zellbezug wird etwa so geschrieben: **Tabelle1!$B$2:$B$9**. Enthält der Blattname Leerzeichen, steht er in einfachen Anführungszeichen.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben den Zahlen.

1. Markieren Sie die Zahlen. Möchten Sie für jede Zeile eine Sparkline, markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** die Option **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl unter null einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sein können.
3. Im Feld **Sparklines erstellen** prüfen Sie den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben den **Positionsbereich** ein (die Zellen, in die sie kommen). Wählen Sie für jede Datenzeile oder für jede Datenspalte eine Zelle. Wählen Sie eine einzelne Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und teilen sich eine Darstellung. Klicken Sie auf eine Zelle mit einer Sparkline, erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und die Position der ganzen Gruppe oder nur die Daten der angeklickten Sparkline. Außerdem legen Sie fest, was mit leeren Zellen geschieht (als Lücke darstellen, als null behandeln oder die Punkte über sie hinweg verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Hochpunkt**, den **Tiefpunkt**, **Negative Punkte**, den **ersten Punkt**, den **letzten Punkt** und bei einer Linie **Markierungen** auf jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Farbsatz. **Sparkline-Farbe** legt die Farbe der Linie oder Balken fest und die **Stärke** (wie dick die Linie ist, in Punkt). **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und größten Wert der vertikalen Achse fest. **Für jede Sparkline automatisch** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine einheitliche Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** erlaubt die Eingabe einer Zahl. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punktes enthalten, dann steht ein Punkt mit einem späteren Datum weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ihre eigene Darstellung. **Löschen** entfernt die ausgewählten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das andere Tabellenkalkulationen lesen.

## Ein Bild, eine Form oder ein Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet **Bild** in der Gruppe **Illustrationen** einen Dateidialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG). Das Bild landet an der ausgewählten Zelle auf dem Blatt. Ein Bild, das breiter als 480 Punkte ist, wird verkleinert, damit es passt, und behält seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert aufnehmen kann (etwa SVG), wird als PNG gespeichert, ein einfaches Pixelbild. Eine Datei, die kein Bild ist oder größer als 16 MB, wird mit einer Meldung abgelehnt. Eine WMF- oder EMF-Datei (Bildformate, die manche älteren Programme aus Linien und Formen erzeugen) wird ebenfalls als PNG gezeichnet und gespeichert.

**Symbole** zeigt dasselbe Fenster mit kleinen Strichgrafiken, das JeffPub und JeffDeck haben, mit einem Feld, um sie nach Namen zu suchen. Wählen Sie ein oder mehrere Symbole aus. Jedes landet als Bild mit 48 Punkt Breite und Höhe auf dem Blatt. Es wird dunkelgrau gezeichnet, in einer Größe, die auch beim Vergrößern scharf bleibt. Jedes Symbol ist ein einzelner Schritt, den Sie rückgängig machen können.

**Barcode** öffnet ein kurzes Menü mit zwei Auswahlmöglichkeiten. **Barcode** öffnet ein Fenster, in dem Sie einen einzelnen Barcode erstellen (**Barcodes aus Zellen**, unten, erstellt viele): Buch-ISBNs (mit oder ohne Preis), EAN-13, UPC-A, EAN-8, Code 128, Code 39 oder ein QR-Code, der beliebigen Text enthält, etwa eine Webadresse. Wählen Sie einen **Typ**, geben Sie den Text oder die Ziffern ein. Das Fenster zeigt den Barcode während der Eingabe an und nennt in Rot, was falsch ist, falls er nicht erstellt werden kann. Klicken Sie auf **Einfügen**, dann landet der Barcode als Bild auf dem Blatt, als ein Schritt, den Sie rückgängig machen können. Sie verschieben und skalieren ihn wie jedes andere Bild. Zum Ändern markieren Sie ihn und klicken erneut auf **Barcode**, ändern den Text und klicken auf **Aktualisieren**; der neue Barcode ersetzt den alten und übernimmt dessen Breite. Beim Speichern wird er als Bild mit einer SVG-Datei gespeichert, die scharf bleibt, sodass jedes Programm ihn anzeigt. JeffCrunch behält die Einstellungen in der Datei, sodass Sie den Barcode beim erneuten Öffnen der Arbeitsmappe in JeffCrunch weiterhin bearbeiten können.

**Barcode > Barcodes aus Zellen** (im selben Menü **Barcode**) erstellt für jede gefüllte Zelle der markierten Zellen einen Barcode, etwa für einen Bogen mit Etiketten. Markieren Sie die Zellen, klicken Sie darauf und wählen Sie dann Typ und Aussehen im selben Fenster. Was Sie dort eingeben, ist nur ein Beispiel, denn der Text jeder Zelle wird zu ihrem eigenen Barcode. Die Barcodes stehen in einer Spalte direkt rechts neben den markierten Zellen, untereinander, als ein einzelner Schritt, den Sie rückgängig machen können. Eine Zelle, deren Text der Code nicht aufnehmen kann (etwa ein Buchstabe in einem EAN-13), wird in einer Meldung genannt und erhält keinen Barcode. Auf einmal werden höchstens 500 Zellen verarbeitet. Wählen Sie weniger, wenn die Meldung zu viele meldet. Jeder Barcode ist ein gewöhnliches Bild, das beim Klicken aus dem Text der Zelle gezeichnet wird. Ändern Sie die Zelle später, folgt der Barcode nicht. Markieren Sie die Zellen dann und erstellen Sie die Barcodes erneut.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblasen-Legende und eine Linie. Wählen Sie eine aus, dann landet sie hellblau mit dunklerem Rand auf dem Blatt. **Textfeld** in der Gruppe **Text** fügt ein leeres Feld ohne Füllung und ohne Rand ein.

Wenn eine Form ausgewählt ist, gibt **Formenarten** auf der Registerkarte **Formformat** ihr eine fertige Darstellung: eine Füllung und eine Kontur, die zusammenpassen, etwa Hellblau mit dunkelblauem Rand. Die beiden letzten Optionen behalten nur die Kontur oder nur die Füllung. Ein Klick ist ein Schritt, den Sie rückgängig machen können. **Formfüllung** und **Formkontur** ändern jeweils einen Teil für sich.

## Freie Bilder im Internet finden

**Onlinebilder** in der Gruppe **Illustrationen** der Registerkarte **Einfügen** findet freie Bilder. Sie stammen aus zwei offenen Bibliotheken: Openverse, das viele Sammlungen durchsucht, und Wikimedia Commons, der Bildbibliothek von Wikipedia.

1. Klicken Sie auf **Onlinebilder…**. Rechts öffnet sich ein Bereich.
2. Wählen Sie eine Bibliothek, geben Sie ein, wonach Sie suchen, und klicken Sie auf **Suchen** (oder drücken Sie die Eingabetaste).
3. Klicken Sie auf ein Bild, um zu sehen, wer es erstellt hat und welche Lizenz es hat. Eine **Lizenz** legt fest, wie Sie ein Bild verwenden dürfen. Klicken Sie auf den Namen der Lizenz, um sie zu lesen.
4. Klicken Sie auf **Einfügen** (oder doppelklicken Sie auf das Bild).

**Was JeffCrunch sendet.** Ihre Suchbegriffe gehen an die gewählte Bibliothek, und zwar erst, wenn Sie auf **Suchen** klicken oder die Eingabetaste drücken. Danach fordert JeffCrunch von der Bibliothek die kleinen Vorschaubilder der Treffer an und das eine Bild, das Sie einfügen. Wie bei jeder Webseite kann die Bibliothek die Internetadresse Ihres Computers sehen, und ihr wird mitgeteilt, dass JeffCrunch die Bilder anfordert. Inhalte Ihrer Arbeitsmappe werden nicht gesendet.

Das Bild landet wie ein Bild aus einer Datei auf dem Blatt, und die Arbeitsmappe speichert es. Sein Titel wird als **Alternativtext** gespeichert. Die meisten Lizenzen verlangen, dass Sie den Urheber nennen. Ist **Quellenangabe unter dem Bild hinzufügen** aktiviert, fügt JeffCrunch unter dem Bild ein kleines Textfeld mit Titel, Urheber und Lizenz ein. Sie können es verschieben, ändern oder löschen. Bild und Quellenangabe werden in einem Schritt eingefügt, daher nimmt **Rückgängig** beide zurück. Prüfen Sie, ob die Lizenz zu Ihrer Verwendung passt.

## Textkunst hinzufügen

**Textkunst** sind besondere Schriftzüge: Buchstaben mit Farbe, Kontur, Schatten oder einer gebogenen Form.

1. Klicken Sie auf **Einfügen > Text > Textkunst**. Eine Galerie mit Formaten öffnet sich.
2. Klicken Sie auf ein Format. Der Text „Ihr Text hier“ landet im gewählten Format auf dem Blatt.
3. Um den Text zu ändern, wählen Sie die Textkunst aus und klicken auf der Registerkarte **Formformat** auf **Text bearbeiten…**. Ein Feld fragt nach dem Text, der Schriftart, der Größe, Fett und Kursiv. Eine größere Schriftgröße macht die Textkunst größer.
4. Auf der Registerkarte **Formformat** gibt **Textkunst-Formatvorlagen** den Buchstaben ein anderes Format, und **Textverzerrung** biegt die Wörter in eine Form, etwa eine Welle oder einen Bogen. „Einfach“ hält sie gerade.

Textkunst ist eine Form in der Zeichnung des Blattes, mit ihrer Verzerrung, Füllung und Kontur. Sie behält ihr Aussehen, wenn Sie die Arbeitsmappe als .xlsx speichern und erneut öffnen, in JeffCrunch oder einem anderen Programm. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Eine als .ods-Datei gespeicherte Arbeitsmappe behält ihre Bilder, Formen, Textfelder, Gruppen, Linien und Textkunst ebenfalls. Textkunst wird als Fontwork-Form gespeichert, die OpenDocument kennt, mit Text, Schriftart, Füllung, Kontur und Verzerrung, und sie wird beim erneuten Öffnen der .ods-Datei wieder als Textkunst angezeigt. Einige Verzerrungen, für die OpenDocument keine Form hat (Welle 2, Welle 4 sowie die unteren und oberen Formen von Aufblähen und Zusammendrücken), werden als gerade Schrift mit denselben Buchstaben und Format gespeichert. Diagramme sind ebenfalls in einer .ods-Datei enthalten, siehe unten.

## Bilder verkleinern

Ein Foto aus der Kamera kann viel größer sein, als es auf dem Blatt angezeigt wird, und macht die Datei groß. Klicken Sie auf ein Bild und dann in der Gruppe **Anpassen** der Registerkarte **Bildformat** auf **Bilder komprimieren…**. Wählen Sie, wo die Arbeitsmappe verwendet wird: **Professioneller Druck (300 ppi)**, **Desktop-Druck (220 ppi)** oder **Web (96 ppi)**. Lassen Sie **Auf alle Bilder auf diesem Blatt anwenden** aktiviert, um alle Bilder auf dem Blatt zu verkleinern, oder deaktivieren Sie es, um nur das ausgewählte Bild zu verkleinern. **Zugeschnittene Bereiche von Bildern löschen** entfernt außerdem die verborgenen Teile, die Sie zugeschnitten haben; der Zuschnitt ist dann weg. Ein Bild wird nur ersetzt, wenn das neue kleiner ist. Größe, Position, Name und Alternativtext bleiben erhalten. **Rückgängig** (Strg+Z) stellt die Bilder wieder her, und die gespeicherte Arbeitsmappe enthält nur die kleineren Bilder. Bilder auf anderen Blättern werden nicht geändert: Wechseln Sie auf jedes Blatt und wiederholen Sie den Vorgang. Ein Bild, das eine Zeichnung ist (SVG, EMF oder WMF), oder eine Animation bleibt unverändert.

Jeder Vorgang ist ein einzelner Schritt, den Sie rückgängig machen können. Das neue Objekt ist danach ausgewählt, sodass Sie es ziehen, in der Größe ändern, nach vorne oder hinten verschieben, unter **Eigenschaften** einen Namen und Alternativtext vergeben oder die Taste **Entf** drücken können.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wählen Sie ein Bild aus, erscheint die Registerkarte **Bildformat**. Wählen Sie eine Form, eine Linie oder ein Textfeld aus, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Das Objekt behält seinen Namen, seinen Alternativtext, seine Position und seine Reihenfolge unter den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Stärke (wie dick die Linie ist) und die Striche fest (durchgezogen, gestrichelt oder gepunktet). Ein Bild hat den **Bildrahmen**, das ist seine Kontur.
- **Drehen** dreht das Objekt um eine Viertelumdrehung nach rechts oder links oder spiegelt es horizontal oder vertikal.
- **Zuschneiden** schneidet ein Bild auf ein Fenster in einer bestimmten Form zu: Quadrat, 4 breit mal 3 hoch, 3 breit mal 4 hoch, 16 mal 9 oder 9 mal 16. Der Schnitt erfolgt gleichmäßig aus der Mitte des sichtbaren Bereichs, und das Bild wird so groß gemacht, wie es der Rest erlaubt. **Gesamtes Bild anzeigen** holt das Abgeschnittene zurück. Die Bilddatei bleibt unverändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie den Text ein und wählen Sie, wie er waagerecht und senkrecht ausgerichtet wird, die Größe, Fett oder normal und ob er sich an die Form anpasst oder in einer Zeile bleibt. Bei Textkunst fragt das Feld stattdessen nach Text, Schriftart, Größe, Fett und Kursiv.
- **Auswahlbereich** öffnet eine Liste aller Objekte auf dem Blatt mit ihren Namen, das vorderste zuerst. Klicken Sie auf einen Namen, um dieses Objekt auszuwählen; das hilft, wenn Objekte einander verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt Größe, Name, Alternativtext und fest, was das Objekt tut, wenn Zellen verschoben oder vergrößert werden.

## Ausschneiden, Kopieren und Einfügen eines Objekts

Ein Bild oder eine Form wird ebenso kopiert und eingefügt wie ein Diagramm. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa einen Link zu einer Datei, kann nicht kopiert werden. JeffCrunch teilt Ihnen den Grund mit.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe ein, auf einem beliebigen Blatt, dann liest die Kopie dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es zeigte, liest aber keine Zellen mehr, weil die Zellen dort nicht dieselben sind, aus denen es erstellt wurde.

## Mehrere Objekte auswählen, gruppieren und ausrichten

Klicken Sie auf ein Objekt und halten Sie dann **Umschalt** oder **Strg** gedrückt, während Sie ein weiteres anklicken, um es ebenfalls auszuwählen. Klicken Sie ein ausgewähltes Objekt erneut mit **Umschalt** oder **Strg** an, um die Auswahl aufzuheben. Ausgewählte Objekte erhalten jeweils einen gestrichelten Rahmen. Sie können auch auf der Registerkarte **Formformat** die Option **Objekte auswählen** wählen und über leere Zellen ein Feld ziehen: Jedes Objekt, das ganz innerhalb des Feldes liegt, wird ausgewählt. Drücken Sie **Esc** oder wählen Sie **Objekte auswählen** erneut, um wieder Zellen auszuwählen. Drücken Sie **Entf**, um alle ausgewählten Objekte in einem Schritt zu entfernen.

Sind zwei oder mehr Objekte ausgewählt, enthält die Registerkarte **Formformat** in der Gruppe **Anordnen** folgende Befehle:

- **Gruppieren** verbindet die ausgewählten Formen, Bilder und Linien zu einem Objekt. Es wird als Einheit verschoben und in der Größe geändert, und die Datei speichert die Gruppe. Ein Diagramm kann nicht gruppiert werden.
- **Gruppierung aufheben** löst eine ausgewählte Gruppe auf. Jeder Teil bleibt an seiner Stelle auf dem Blatt.
- **Ausrichten** richtet die ausgewählten Objekte aus: **Linksbündig ausrichten**, **Zentriert ausrichten**, **Rechtsbündig ausrichten**, **Oben ausrichten**, **Mittig ausrichten** und **Unten ausrichten**. Sie richten sich an der Kante oder der Mitte der gesamten Auswahl aus. **Horizontal verteilen** und **Vertikal verteilen** verteilen drei oder mehr Objekte gleichmäßig zwischen den beiden äußersten.

Gruppieren, Gruppierung aufheben und jede Ausrichtungsoption sind jeweils ein Schritt, den Sie rückgängig machen können.

## Benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie **Eingabetaste**. **Rückgängig** setzt den alten Namen wieder ein, und der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie **Entf**, während das Objekt ausgewählt ist. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das andere Tabellenkalkulationen lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und Elemente enthält, die JeffCrunch beim Neuschreiben eines Diagramms nicht erhalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich weiterhin verschieben, in der Größe ändern und löschen. Seine Elemente und Daten werden hier jedoch nicht verändert.

## Diagramme aus anderen Programmen

Ein **Schaubild** ist ein Bild aus Kästchen und Pfeilen, etwa einen Ablauf, das ein anderes Programm erstellt hat. JeffCrunch zeigt ein solches Diagramm anhand des Bildes an, das die Datei daneben aufbewahrt. Klicken Sie darauf, um es auszuwählen, und ziehen Sie es, um es zu verschieben. Beim Speichern wird das Diagramm genau so zurückgeschrieben, wie es war, sodass das Programm, das es erstellt hat, es weiterhin ändern kann.

JeffCrunch kann den Inhalt eines Diagramms noch nicht ändern, und ein Diagramm kann weder kopiert noch gruppiert werden. Enthält die Datei kein Bild eines Diagramms, zeigt JeffCrunch einen einfachen grauen Kasten mit dem Hinweis, dass es sich um ein Diagramm handelt.

## Diagramme in .ods-Dateien

Eine als .ods-Datei gespeicherte Arbeitsmappe behält ihre Diagramme: Säulen, Balken, Linien, Flächen, Kreise, Ringe und Punktdiagramme, jeweils mit ihren Reihen und den Zellen, die sie lesen (auf diesem Blatt oder einem anderen), Kategorien, Titel, Legende, Achsen (Titel, kleinster und größter Wert, Zahlenformat), Datenbeschriftungen, Farben und Position. Eine .ods-Datei aus einem anderen Programm wird mit ihren Diagrammen geöffnet, gezeichnet aus den Zellen, die sie lesen. Was ein .ods-Diagramm nicht aufnehmen kann, wird beim Speichern weggelassen und benannt: Trendlinien, Fehlerindikatoren, zweite Achsen und die besonderen Darstellungen einzelner Punkte. Diagrammtypen, die JeffCrunch nicht zeichnet (Börsen- und Netzdiagramme), werden als Säulen ihrer Werte geöffnet.
