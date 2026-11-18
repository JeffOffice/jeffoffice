# Diagramme, Sparklines, Bilder und Formen

<!-- keywords: Feldschaltflächen, Schaltflächen für Pivot-Diagramme, gruppieren, Gruppierung aufheben, ausrichten, linksbündig ausrichten, verteilen, Objekte auswählen, mehrere Formen, mehrere auswählen, Auswahlrahmen, Diagrammfilter, Reihen ausblenden, empfohlene Diagramme, Diagrammvorschläge, Diagramm, Grafik, Säule, Balken, Linie, Kreis, Fläche, Streudiagramm, Kombination, Legende, Titel, Gitternetzlinien, Datenbeschriftungen, Diagrammdaten, Diagrammtyp, Formatvorlagen, Layouts, verschieben, Größe ändern, Bild, Form, Objekt, Datenreihe, Trendlinie, Gleichung, R-Quadrat, zweite Achse, Sparkline, Sparklines, Datumsachse, Gewinn/Verlust, Mini-Diagramm, Markierungen, Name, Alternativtext, logarithmisch, logarithmische Skalierung, als Bild speichern, Vorlage, crtx, Titeltext, Legendenschlüssel, Auswahlbereich, Formfüllung, Formkontur, drehen, spiegeln, zuschneiden, Text bearbeiten, Bildrahmen, Bild einfügen, Symbole, Icon, WMF, EMF, Formenarten, Formatvorlagen, Form einfügen, Textfeld, Formen, Pfeil, Oval, Rechteck, Stern, Diagramm (Schema), Schemadiagramm, Onlinebilder, freie Bilder, Openverse, Wikimedia Commons, Lizenz, Quellenangabe, Textkunst, Fontwork, Verzerrung, Bilder komprimieren, Bilder verkleinern, Dateigröße, ppi, gekürzte Bereiche löschen, Barcode, QR-Code, Form mit Zelle verknüpfen, Form zeigt Zelle, Textfeld zeigt Zelle, Zellwert in Form -->

Ein Diagramm ist ein Bild von Zahlen, das mit ihnen verbunden bleibt: Ändern Sie eine Zahl, passt sich das Diagramm an. Bilder und Formen liegen ebenso über den Zellen, sind aber nicht an Zahlen gebunden.

## Ein Diagramm erstellen

1. Markieren Sie die Zahlen, die Sie darstellen möchten. Wenn die erste Zeile Namen und die erste Spalte Beschriftungen enthält, markieren Sie diese ebenfalls. JeffCrunch prüft die erste Zeile und Spalte und verwendet sie als Namen und Beschriftungen, wenn es sich um Wörter (oder bei den Beschriftungen um Datumswerte) handelt.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Diagramme** einen Typ: **Säule**, **Balken**, **Linie**, **Kreis**, **Fläche**, **Streudiagramm** oder **Kombination**. Der kleine Pfeil neben einem Typ zeigt seine Varianten, etwa gestapelte Säulen, eine Linie mit Markierungen oder ein Streudiagramm mit Punkten, verbundenen Punkten oder geglätteten Linien.
3. Um zuerst alle Typen zu sehen, wählen Sie **Weitere Diagramme**. Die Liste links zeigt jeden Typ mit Ihren Zellen. **Zeilen und Spalten wechseln** dreht die Datenreihen um.

Sie wissen nicht, welchen Typ Sie wählen sollen? Markieren Sie Ihre Zellen und wählen Sie auf der Registerkarte **Einfügen** die Option **Empfohlene Diagramme**. JeffCrunch prüft die Zellen und listet einige passende Typen auf, der beste zuerst. Stehen zum Beispiel Datumswerte untereinander, schlägt JeffCrunch eine Linie vor; eine einzelne Spalte mit wenigen positiven Zahlen und Beschriftungen ergibt eher einen Kreis. Jeder Typ wird aus Ihren Zellen gezeichnet. Wählen Sie einen aus und klicken Sie auf **OK**.

Wenn Sie nur eine Zelle markieren, stellt JeffCrunch den Zellbereich um sie herum dar, der Inhalt hat.

Datenreihen verlaufen in den Spalten, wenn die Markierung mindestens so viele Zeilen wie Spalten hat, und in den Zeilen, wenn sie mehr Spalten als Zeilen hat. Ein Kreis enthält eine Datenreihe. Ein Streudiagramm übernimmt die horizontalen Werte aus der ersten Spalte.

Beispiel: Sie markieren A1:C4. A2:A4 enthalten Q1, Q2 und Q3, B1 und C1 enthalten Ost und West, der Rest sind Zahlen. Das Diagramm hat zwei Datenreihen, Ost und West, und drei Gruppen von Säulen, Q1 bis Q3.

## Eine Reihe vorübergehend ausblenden

Wählen Sie ein Diagramm aus. Daneben erscheinen drei kleine Schaltflächen. Die mit dem Trichter ist **Diagrammfilter**. Sie listet die Datenreihen des Diagramms mit je einem Kontrollkästchen auf. Deaktivieren Sie ein Kontrollkästchen, um die Reihe aus dem Diagramm zu nehmen. Aktivieren Sie es erneut, um sie an derselben Stelle wieder einzublenden. **Rückgängig** funktioniert ebenfalls. Ein Diagramm behält immer mindestens eine Datenreihe. Eine ausgeblendete Reihe bleibt beim Speichern in der Datei erhalten und bleibt beim Öffnen ausgeblendet, damit nichts verloren geht. Aktivieren Sie das Kontrollkästchen, um sie wieder einzublenden. (Ein Ringdiagramm zeigt beim Öffnen alle Datenreihen an.)

## Verschieben und die Größe ändern

Klicken Sie auf ein Diagramm, Bild oder eine Form, um es auszuwählen. Ziehen Sie es, um es zu verschieben, oder ziehen Sie eines der acht Quadrate am Rand, um die Größe zu ändern.

- Halten Sie **Alt** beim Ziehen gedrückt, damit eine Kante an der Kante einer Zelle ausgerichtet wird.
- Halten Sie **Umschalt** beim Ziehen an einer Ecke gedrückt, damit die Proportionen erhalten bleiben.
- Die Pfeiltasten verschieben das Objekt jeweils um einige Pixel. **Strg** mit einer Pfeiltaste verschiebt es um einen Pixel, **Umschalt** mit einer Pfeiltaste vergrößert oder verkleinert es, und **Alt** mit einer Pfeiltaste springt zur nächsten Kante einer Zelle.
- **Esc** während des Ziehens setzt das Objekt zurück.

Jedes Verschieben oder Ändern der Größe ist ein Schritt, den Sie rückgängig machen können. Klicken Sie mit der rechten Maustaste auf das Objekt und wählen Sie **Eigenschaften**, um seinen **Namen** (jedes Objekt auf einem Blatt braucht einen eigenen) und seinen **Alternativtext** (ein kurzer Satz, der einer Person, die das Objekt nicht sehen kann, erklärt, was es zeigt) einzugeben, die Größe festzulegen und zu bestimmen, was beim Einfügen, Löschen oder Ändern der Größe von Zellen geschieht: mit den Zellen verschieben und in der Größe anpassen, mit ihnen verschieben, aber die Größe beibehalten, oder an der Stelle bleiben. Die Auswahl wird in der Datei gespeichert. **Dieses Objekt drucken** im selben Dialogfeld schaltet das Objekt auf den gedruckten Seiten aus (auf dem Bildschirm bleibt es sichtbar); auch diese Einstellung speichert die Datei.

## Festlegen, was ein Diagramm zeigt

Wenn ein Diagramm ausgewählt ist, erscheinen daneben zwei Schaltflächen, und auf dem Menüband erscheint die Registerkarte **Diagramm**.

- Die Schaltfläche **+** (und **Diagrammelemente** auf der Registerkarte) schaltet Titel, Achsen, Achsentitel, Gitternetzlinien, Legende und Datenbeschriftungen des Diagramms ein oder aus und legt fest, wo Legende und Beschriftungen stehen.
- Die Pinselschaltfläche (und **Farben** sowie **Diagrammlook** auf der Registerkarte) gibt dem Diagramm eine der Farbgestaltungen und eines der Erscheinungsbilder von JeffCrunch. **Diagrammlayouts** setzt mehrere Elemente auf einmal.
- **Diagrammdaten** listet die Datenreihen des Diagramms auf. Sie können sie hinzufügen, bearbeiten, entfernen und neu anordnen, die Zellen ändern, die jede liest, die Zellen mit den Beschriftungen festlegen, Zeilen und Spalten vertauschen und festlegen, wie leere Zellen und ausgeblendete Zeilen und Spalten dargestellt werden.
- **Feldschaltflächen** (im Menü **+**, nur bei einem Pivot-Diagramm) zeigen kleine Schaltflächen mit den Namen der Felder der Pivot-Tabelle, der das Diagramm folgt: die Berichtsfilter unter dem Titel sowie die Zeilen, Werte und Spalten am unteren Rand. Schalten Sie jede Art ein oder aus oder wählen Sie **Alle anzeigen** oder **Alle ausblenden**. Ein neues Pivot-Diagramm zeigt sie an.
- **Fehlerindikatoren** (im Menü **+**, für ein Säulen-, Balken-, Linien-, Flächen- oder Streudiagramm) zeichnen über und unter jedem Punkt eine kleine Linie, die zeigt, wie stark der echte Wert abweichen könnte. Wählen Sie **Standardfehler**, **Prozentsatz (5 %)**, **Standardabweichung** oder **Fester Wert (1)**. Wählen Sie **Benutzerdefiniert**, um eigene Beträge zu verwenden: Geben Sie die Zellen ein, die die Beträge über jedem Punkt enthalten (etwa `=Tabelle1!$D$2:$D$4`), danach die Zellen für die Beträge darunter. Lassen Sie eine Angabe leer, um die Fehlerindikatoren nur auf einer Seite zu zeichnen. Die Indikatoren ändern sich, wenn sich diese Zellen ändern. Eine Datei, die eigene Beträge für jeden Punkt enthält, behält sie.
- **Linien** (im Menü **+**, für ein Liniendiagramm) enthält **Tropflinien**, die vom höchsten Punkt jeder Kategorie bis zur Achse reichen, **Hoch-Tief-Linien**, die den höchsten und den niedrigsten Punkt jeder Kategorie verbinden, und **Aufwärts-/Abwärtsbalken**, Kästchen zwischen der ersten und der letzten Datenreihe jeder Kategorie. Ein Aufwärtsbalken zeigt an, dass die letzte Reihe höher liegt. Hoch-Tief-Linien und Aufwärts-/Abwärtsbalken brauchen mindestens zwei Datenreihen. Flächendiagramme haben Tropflinien.
- **Datentabelle** (im Menü **+**, für ein Säulen-, Linien- oder Flächendiagramm) setzt unter das Diagramm ein Raster mit den Zahlen, mit einer Spalte für jede Kategorie und einer Zeile für jede Datenreihe. Ihre Liste schaltet die Tabelle ein und aus und legt die Linien zwischen Zeilen, die Linien zwischen Spalten, den Rahmen und die Legendenschlüssel fest (die Farbe jeder Datenreihe neben ihrem Namen).
- **Trendlinie** (im Menü **+**) zeichnet eine Linie, die an die Zahlen jeder Datenreihe angepasst ist, damit Sie sehen, wohin sie tendieren. Wählen Sie **Linear** für eine gerade Linie, **Exponentiell** für Zahlen, die um einen gleichbleibenden Prozentsatz wachsen, **Logarithmisch** für Zahlen, die schnell steigen und dann abflachen, **Polynomisch** für eine Kurve mit Knicken, **Potenz** oder **Gleitender Durchschnitt**, der den Durchschnitt der letzten Zahlen zeichnet. **Gleichung anzeigen** schreibt die Formel der Linie in das Diagramm, und **R-Quadrat anzeigen** schreibt, wie gut die Linie passt (1 ist eine perfekte Anpassung, 0 bedeutet keine Anpassung). Die Trendlinie erhält einen eigenen Eintrag in der Legende.
- Wird eine Datenreihe an einer zweiten senkrechten Achse dargestellt, enthalten die Menüs **Achsen** und **Achsentitel** die Einträge **Zweite horizontale** und **Zweite vertikale** für sie. Das Menü **Legende** enthält **Über dem Diagramm**, das die Legende auf die Zeichenfläche legt statt daneben.
- **Datenbeschriftungen** enthält außerdem **Beschriftung enthält** (den Wert, den Namen der Kategorie, den Namen der Datenreihe, den Prozentsatz und den Legendenschlüssel, ein kleines Quadrat in der Farbe der Datenreihe, in beliebiger Kombination), **Trennzeichen** (was dazwischen steht: Komma, Semikolon, Punkt, neue Zeile oder Leerzeichen) und **Zahlenformat…** (ein Formatcode wie 0,0 für die Zahlen in den Beschriftungen; lassen Sie das Feld leer, um die Zahlen unverändert anzuzeigen). **Wert aus Zellen…** zeigt die Texte einiger Zellen als Beschriftungen, eine Zelle für jeden Punkt. Geben Sie die Zellen ein, etwa `=Tabelle1!$D$2:$D$4`. Die Beschriftungen ändern sich, wenn sich die Zellen ändern. Lassen Sie das Feld leer, um sie zu entfernen. Schalten Sie alle Inhalte aus, verschwinden die Beschriftungen.
- **Horizontale Achse formatieren…** und **Vertikale Achse formatieren…** auf der Registerkarte **Diagramm** öffnen ein Dialogfeld für eine Achse. Es legt den kleinsten und den größten Wert sowie die Haupt- und Hilfsintervalle fest (jeweils automatisch oder als eingegebene Zahl), ob die Werte in umgekehrter Reihenfolge verlaufen, die Anzeigeeinheiten (die Beschriftungen zeigen die Zahlen geteilt, etwa durch 1.000 bei Tausendern, sodass 30.000 als 30 erscheint; aktivieren Sie **Name der Einheiten im Diagramm anzeigen**, um den Namen, etwa Tausend, zusammen mit dem Achsentitel zu drucken), ob die Skala logarithmisch ist (jede Marke ist eine Basis mal die vorherige, etwa 1, 10, 100 und 1.000; das eignet sich für Zahlen, die sich stark in der Größe unterscheiden und alle größer als 0 sein müssen), wo die andere Achse sie schneidet, das Zahlenformat ihrer Beschriftungen, ihre Teilstriche, wo ihre Beschriftungen stehen und wie stark die Beschriftungen gedreht sind. Nur eine Zahlenachse hat Grenzen und Intervalle.
- **Diagramm verschieben…** auf der Registerkarte **Diagramm** fragt, wohin das Diagramm soll. Wählen Sie **Neues Blatt** und geben Sie einen Namen ein, dann erhält das Diagramm ein eigenes Diagrammblatt: ein Blatt, das nur aus dem Diagramm besteht und keine Zellen hat. Wählen Sie **Objekt in** und dann ein Arbeitsblatt, landet das Diagramm auf diesem Arbeitsblatt. Ein Diagramm auf einem Diagrammblatt kann nur auf ein Arbeitsblatt verschoben werden, und das dadurch verlassene Diagrammblatt wird gelöscht. Das Diagramm liest weiterhin dieselben Zellen. Auf einem Arbeitsblatt behält es seine Größe und seine Position auf dem Blatt, auch wenn die Zeilen und Spalten dort höher oder breiter sind, und es verschiebt und ändert die Größe mit den Zellen dort wie zuvor. Ausgeblendete Blätter werden nicht angeboten. Es ist ein einziger Schritt für **Rückgängig**, auch nach dem Speichern, außer wenn dadurch ein Blatt gelöscht wurde. Eine Datei im Format `.ods` kann kein Diagrammblatt speichern; speichern Sie ein Diagrammblatt daher im Format `.xlsx`.
- **Diagrammtitel** und **Achsentitel** (im Menü **+**) haben außerdem **Titeltext…**. Geben Sie den gewünschten Text ein. Oder geben Sie ein Gleichheitszeichen und eine Zelle ein, etwa **=Tabelle1!A1**, dann zeigt der Titel den Inhalt dieser Zelle. Ändert sich die Zelle, ändert sich der Titel. Die Datei speichert die Verknüpfung.
- **Als Bild speichern…** auf der Registerkarte **Diagramm** (und im Kontextmenü) speichert das ausgewählte Diagramm als Bilddatei im Format **PNG** oder **JPEG**, doppelt so groß wie auf dem Blatt, damit es scharf bleibt.
- **Als Vorlage speichern…** und **Vorlage anwenden…** (auf der Registerkarte **Diagramm** in der Gruppe **Typ**) speichern das Erscheinungsbild eines Diagramms in einer Datei **.crtx** und übertragen es auf ein anderes Diagramm. Zum Erscheinungsbild gehören Typ, Farben, Einstellungen für Titel und Legende, Achsen und Schriftarten. Das Diagramm, auf das Sie es anwenden, behält seine eigenen Zahlen und die Zellen, die es liest. Ein **Rückgängig** nimmt das Erscheinungsbild wieder weg.
- **Diagrammtyp ändern** ändert den Typ für das ganze Diagramm oder für jede Datenreihe: Säulen mit einer Linie darüber oder eine Datenreihe an einer zweiten senkrechten Achse.

Ein Zellbezug wird geschrieben wie **Tabelle1!$B$2:$B$9**. Der Name des Blatts steht in einfachen Anführungszeichen, wenn er Leerzeichen enthält.

## Sparklines

Eine Sparkline ist ein winziges Diagramm, das in eine einzelne Zelle passt. Sie zeigt den Verlauf einer Zeile oder Spalte mit Zahlen direkt neben ihnen.

1. Markieren Sie die Zahlen. Für eine Sparkline je Zeile markieren Sie mehrere Zeilen mit Zahlen.
2. Wählen Sie auf der Registerkarte **Einfügen** in der Gruppe **Sparklines** den Typ **Linie**, **Säule** oder **Gewinn/Verlust**. Eine **Linie** verbindet die Zahlen mit einer Linie. Eine **Säule** zeichnet für jede Zahl einen Balken. **Gewinn/Verlust** zeichnet für jede Zahl über null einen Balken nach oben und für jede Zahl darunter einen Balken nach unten, alle gleich groß; das eignet sich für Ergebnisse, die nur gut oder schlecht sind.
3. Prüfen Sie im Dialogfeld **Sparklines erstellen** den **Datenbereich** (die Zellen, die die Sparklines lesen) und geben Sie den **Positionsbereich** ein (die Zellen, in die sie kommen). Wählen Sie für jede Datenzeile oder jede Datenspalte eine Zielzelle. Wählen Sie eine einzige Zelle, liest die Sparkline alle Daten.

Die Sparklines, die Sie in einem Schritt erstellen, bilden eine Gruppe und teilen sich ein Erscheinungsbild. Klicken Sie auf eine Zelle mit einer Sparkline, erscheint die Registerkarte **Sparkline**.

- **Daten bearbeiten** ändert die Daten und die Position der ganzen Gruppe oder die Daten der einen Sparkline, auf die Sie geklickt haben. Außerdem legen Sie fest, was mit leeren Zellen geschieht (Lücke lassen, als null behandeln oder die Punkte darüber verbinden) und ob Zellen in ausgeblendeten Zeilen und Spalten gelesen werden.
- **Typ** wechselt zwischen Linie, Säule und Gewinn/Verlust.
- **Anzeigen** markiert Punkte: den **Hochpunkt**, den **Tiefpunkt**, **Negative Punkte**, den **ersten Punkt**, den **letzten Punkt** und bei einer Linie **Markierungen** an jedem Punkt.
- **Formatvorlagen** gibt der Gruppe einen Satz Farben. **Sparkline-Farbe** legt die Farbe der Linie oder der Balken und die **Stärke** (wie dick die Linie ist, in Punkt) fest. **Markierungsfarbe** legt die Farbe jeder Art markierter Punkte fest.
- **Achse** zeigt eine Linie bei null, stellt die Zahlen von rechts nach links dar und legt den kleinsten und den größten Wert der senkrechten Achse fest. **Automatisch für jede Sparkline** passt jede an ihre eigenen Zahlen an. **Für alle Sparklines gleich** gibt der ganzen Gruppe eine einheitliche Skala, damit Sie sie vergleichen können. **Benutzerdefinierter Wert** lässt Sie eine Zahl eingeben. **Datumsachse…** verteilt die Punkte nach Datum: Geben Sie die Zellen ein, die das Datum jedes Punkts enthalten; ein Punkt mit einem späteren Datum steht weiter rechts. Lassen Sie das Feld leer, um die Punkte wieder gleichmäßig zu verteilen.
- **Gruppieren** fasst die ausgewählten Sparklines in einer Gruppe zusammen. **Gruppierung aufheben** gibt jeder ein eigenes Erscheinungsbild. **Löschen** entfernt die ausgewählten Sparklines oder ihre ganzen Gruppen.

Eine Sparkline folgt ihren Zellen: Fügen Sie eine Zeile über den Daten ein, liest sie weiterhin dieselben Zahlen. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Sparklines werden in der Datei in einem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen.

## Ein Bild, eine Form oder ein Textfeld einfügen

Auf der Registerkarte **Einfügen** öffnet in der Gruppe **Illustrationen** der Befehl **Bild** einen Dateidialog. Wählen Sie eine Bilddatei (PNG, JPEG, GIF, BMP, TIFF oder SVG); sie landet auf dem Blatt an der markierten Zelle. Ein Bild, das breiter als 480 Punkte ist, wird verkleinert, damit es passt, und behält seine Form. Die Arbeitsmappe speichert das Bild selbst. Ein Bild in einem Format, das die Arbeitsmappe nicht unverändert speichern kann (zum Beispiel SVG), wird als PNG gespeichert, ein einfaches Bild aus Pixeln. Eine Datei, die kein Bild ist, oder größer als 16 MB ist, wird mit einer Meldung abgelehnt. Eine WMF- oder EMF-Datei (Bildformate, die ältere Programme aus Linien und Formen erstellen) wird ebenfalls gezeichnet und als PNG gespeichert.

**Symbole** zeigt dasselbe Fenster mit kleinen Strichgrafiken, das JeffPub und JeffDeck haben, mit einem Feld zur Suche nach dem Namen. Wählen Sie ein oder mehrere Symbole; jedes landet als Bild mit 48 Punkt Breite und Höhe auf dem Blatt. Es wird in dunklem Grau gezeichnet, in einer Größe, die beim Vergrößern scharf bleibt. Jedes Symbol ist ein einzelner Schritt, den Sie rückgängig machen können.

**Barcode** öffnet ein kurzes Menü mit zwei Möglichkeiten. **Barcode** öffnet ein Fenster, in dem Sie einen einzelnen Barcode erstellen (**Barcodes aus Zellen**, unten, erstellt viele): Buch-ISBNs (mit oder ohne Preis), EAN-13, UPC-A, EAN-8, Code 128, Code 39 oder ein QR-Code, der beliebigen Text enthält, etwa eine Webadresse. Wählen Sie einen **Typ**, geben Sie den Text oder die Ziffern ein; das Fenster zeigt den Barcode während der Eingabe an und nennt rot, was nicht stimmt, falls er nicht erstellt werden kann. Klicken Sie auf **Einfügen**; der Barcode landet als Bild auf dem Blatt, ein Schritt, den Sie rückgängig machen können. Sie verschieben und skalieren ihn wie jedes Bild. Zum Ändern markieren Sie ihn und klicken erneut auf **Barcode**, ändern den Text und klicken auf **Aktualisieren**; der neue Barcode nimmt den Platz und die Breite des alten ein. Beim Speichern wird er als Bild mit einer SVG-Datei gespeichert, die scharf bleibt, damit jedes Programm ihn anzeigt. JeffCrunch speichert seine Einstellungen in der Datei, sodass Sie ihn beim erneuten Öffnen der Arbeitsmappe in JeffCrunch weiterhin bearbeiten können.

**Barcode > Barcodes aus Zellen** (im selben Menü **Barcode**) erstellt für jede gefüllte Zelle der markierten Zellen einen Barcode, etwa für einen Bogen mit Etiketten. Markieren Sie die Zellen, klicken Sie darauf und wählen Sie dann Typ und Aussehen im selben Fenster. Was Sie dort eingeben, ist nur ein Beispiel, denn der eigene Text jeder Zelle wird zu ihrem Barcode. Die Barcodes stehen in einer Spalte direkt rechts neben den markierten Zellen, untereinander, als ein einzelner Schritt, den Sie rückgängig machen können. Eine Zelle, deren Text der Code nicht darstellen kann (zum Beispiel ein Buchstabe in einem EAN-13), wird in einer Meldung genannt und bekommt keinen Barcode. Auf einmal werden höchstens 500 Zellen verarbeitet; markieren Sie weniger, wenn eine Meldung meldet, dass es zu viele sind. Jeder Barcode merkt sich die Zelle, aus der er stammt, und folgt ihr: Ändern Sie diese Zelle, wird der Barcode aus dem neuen Text erneut erstellt, an derselben Stelle und in derselben Größe, als eigener Schritt, den Sie rückgängig machen können. Die Verknüpfung wandert mit der Zelle, wenn Sie Zeilen oder Spalten einfügen oder löschen. Löschen Sie die Zelle, behält der Barcode den letzten Text, den er hatte. Beim Speichern wird das Bild in der Datei aus dem Text der Zelle in ihrem aktuellen Zustand erstellt, damit auch andere Programme den aktuellen Wert anzeigen.

**Formen** öffnet eine Liste: Rechteck, abgerundetes Rechteck, Oval, Dreieck, Raute, Pfeile, ein Stern, ein Herz, eine Wolke, eine Sprechblase und eine Linie. Wählen Sie eine Form; sie landet hellblau mit dunklerem Rand auf dem Blatt. **Textfeld** in der Gruppe **Text** fügt ein leeres Feld ohne Füllung und ohne Rand ein.

Wenn eine Form ausgewählt ist, gibt **Formenarten** auf der Registerkarte **Formformat** ihr ein fertiges Erscheinungsbild: eine Füllung und eine Kontur, die zusammenpassen, etwa Hellblau mit dunkelblauem Rand. Die beiden letzten Optionen behalten nur die Kontur oder nur die Füllung. Ein Klick ist ein Schritt, den Sie rückgängig machen können. **Formfüllung** und **Formkontur** ändern jeden Teil für sich.

## Kostenlose Bilder im Internet finden

**Onlinebilder** in der Gruppe **Illustrationen** der Registerkarte **Einfügen** findet kostenlose Bilder. Sie stammen aus zwei offenen Bibliotheken: Openverse, das viele Sammlungen durchsucht, und Wikimedia Commons, die Bildbibliothek von Wikipedia.

1. Klicken Sie auf **Onlinebilder…**. Rechts öffnet sich ein Bereich.
2. Wählen Sie eine Bibliothek, geben Sie ein, wonach Sie suchen, und klicken Sie auf **Suchen** (oder drücken Sie die Eingabetaste).
3. Klicken Sie auf ein Bild, um zu sehen, wer es gemacht hat und welche Lizenz es hat. Eine **Lizenz** legt fest, wie Sie ein Bild verwenden dürfen. Klicken Sie auf den Namen der Lizenz, um sie zu lesen.
4. Klicken Sie auf **Einfügen** (oder doppelklicken Sie auf das Bild).

**Was JeffCrunch sendet.** Ihre Suchbegriffe gehen an die von Ihnen gewählte Bibliothek, und zwar nur, wenn Sie auf **Suchen** klicken oder die Eingabetaste drücken. Danach fordert JeffCrunch von der Bibliothek die kleinen Vorschaubilder der Treffer an sowie das eine Bild, das Sie einfügen. Wie jede Webseite kann die Bibliothek die Internetadresse Ihres Computers sehen, und sie erfährt, dass JeffCrunch diese Anfrage stellt. Nichts aus Ihrer Arbeitsmappe wird gesendet.

Das Bild landet auf dem Blatt wie ein Bild aus einer Datei, und die Arbeitsmappe speichert es. Sein Titel wird als **Alternativtext** gespeichert. Die meisten Lizenzen verlangen, den Urheber zu nennen. Ist **Quellenangabe unter dem Bild hinzufügen** aktiviert, fügt JeffCrunch unter dem Bild ein kleines Textfeld mit Titel, Urheber und Lizenz ein. Sie können es verschieben, ändern oder löschen. Bild und Quellenangabe werden in einem Schritt eingefügt, sodass **Rückgängig** beides zurücknimmt. Prüfen Sie, ob die Lizenz zu Ihrer Verwendung des Bildes passt.

## Textkunst hinzufügen

**Textkunst** sind verzierte Wörter: Buchstaben mit Farbe, Kontur, Schatten oder einer gebogenen Form.

1. Klicken Sie auf **Einfügen > Text > Textkunst**. Eine Galerie mit Formatvorlagen öffnet sich.
2. Klicken Sie auf eine Vorlage. Der Text „Ihr Text hier“ landet im gewählten Stil auf dem Blatt.
3. Um den Text zu ändern, wählen Sie die Textkunst aus und klicken Sie auf der Registerkarte **Formformat** auf **Text bearbeiten…**. Ein Dialogfeld fragt nach Text, Schriftart, Größe, Fett und Kursiv. Eine größere Schriftgröße macht die Textkunst größer.
4. Auf der Registerkarte **Formformat** gibt **Textkunst-Formatvorlagen** den Buchstaben einen anderen Stil, und **Text in Form** biegt die Wörter in eine Form, etwa eine Welle oder einen Bogen. „Keine“ macht sie wieder gerade.

Textkunst ist eine Form in der Zeichnung des Blatts, mit ihrer Verzerrung, Füllung und Kontur. Sie behält ihr Aussehen, wenn Sie die Arbeitsmappe als .xlsx-Datei speichern und wieder öffnen, in JeffCrunch oder in einem anderen Programm. Jede Änderung ist ein Schritt, den Sie rückgängig machen können. Eine als .ods-Datei gespeicherte Arbeitsmappe behält ihre Bilder, Formen, Textfelder, Gruppen, Linien und Textkunst ebenfalls. Textkunst wird als Fontwork-Form gespeichert, die OpenDocument kennt, mit Text, Schriftart, Füllung, Kontur und Verzerrung, und sie wird beim erneuten Öffnen der .ods-Datei wieder als Textkunst angezeigt. Einige Verzerrungen, für die OpenDocument keine Form hat (Welle 2, Welle 4 sowie die unteren und oberen Formen von Aufblasen und Zusammendrücken), werden als gerade Wörter mit denselben Buchstaben und demselben Stil gespeichert. Diagramme werden auch in einer .ods-Datei behalten: siehe unten.

## Bilder verkleinern

Ein Foto aus einer Kamera kann viel größer sein, als es auf dem Blatt angezeigt wird, und es macht die Datei groß. Klicken Sie auf ein Bild und dann auf **Bilder komprimieren…** in der Gruppe **Anpassen** der Registerkarte **Bildformat**. Wählen Sie, wo die Arbeitsmappe verwendet wird: **Professioneller Druck (300 ppi)**, **Desktop-Druck (220 ppi)** oder **Web (96 ppi)**. Lassen Sie **Auf alle Bilder auf diesem Blatt anwenden** aktiviert, um alle Bilder auf dem Blatt zu verkleinern, oder deaktivieren Sie es, um nur das ausgewählte Bild zu verkleinern. **Zugeschnittene Bereiche von Bildern löschen** entfernt außerdem die ausgeblendeten Teile, die Sie zugeschnitten haben; der Zuschnitt ist dann weg. Ein Bild wird nur ersetzt, wenn das neue kleiner ist. Größe, Position, Name und Alternativtext bleiben unverändert. **Rückgängig** (Strg+Z) stellt die Bilder wieder her, und die gespeicherte Arbeitsmappe enthält nur die kleineren Bilder. Bilder auf anderen Blättern werden nicht verändert: Wechseln Sie zu jedem Blatt und führen Sie den Vorgang dort erneut aus. Ein Bild, das eine Zeichnung ist (SVG, EMF oder WMF), oder eine Animation bleibt unverändert.

Jede Anwendung ist ein einzelner Schritt, den Sie rückgängig machen können. Das neue Objekt ist danach ausgewählt, sodass Sie es ziehen, in der Größe ändern, nach vorne oder hinten verschieben, unter **Eigenschaften** einen Namen und Alternativtext vergeben oder die Taste **Entf** drücken können.

## Den Text einer Zelle in einer Form oder einem Textfeld anzeigen

Eine Form oder ein Textfeld kann anzeigen, was eine Zelle zeigt, und ihr folgen. Wählen Sie die Form, klicken Sie in die **Bearbeitungsleiste**, geben Sie ein Gleichheitszeichen und eine Zelle ein und drücken Sie **Eingabe**.

- `=B2` verknüpft die Form mit B2 auf diesem Blatt. `=Tabelle2!B3` verknüpft sie mit einer Zelle auf einem anderen Blatt. `=Gesamt` verknüpft sie mit einem Namen, der für eine Zelle steht.
- Die Form zeigt dann den Text der Zelle so, wie die Zelle ihn zeigt, mit ihrem Zahlenformat. Ändern Sie die Zelle, ihr Format oder eine Formel, von der sie abhängt, ändert sich auch die Form. Text, der länger ist als eine Zelle fassen kann (32.767 Zeichen), wird abgeschnitten. Alle verknüpften Formen einer Arbeitsmappe zeigen zusammen höchstens 4 MB Text: Darüber hinaus zeigt eine Form ihren Text gekürzt an.
- Ist die Form ausgewählt, zeigt die Bearbeitungsleiste ihre Verknüpfung, etwa `=Tabelle1!$B$2`. Löschen Sie den Inhalt der Bearbeitungsleiste und drücken Sie **Eingabe**, um die Verknüpfung zu entfernen. Die Form behält dann den Text, den sie zeigt.
- Wenn Sie Text in die Form eintippen, wird die Verknüpfung ebenfalls entfernt.
- Eine Verknüpfung bezieht sich auf eine Zelle. Ein Bereich wie `=A1:B2`, eine Liste, eine Berechnung oder ein Name, der keine Zelle ist, wird mit einer Meldung abgelehnt, und die Form behält ihre Verknüpfung.
- Wird die Zeile oder Spalte der Zelle gelöscht oder ihr Blatt, zeigt die Form `#BEZUG!` und behält die Verknüpfung, wie es eine Formel tun würde. **Rückgängig** holt die Zelle zurück.
- Jede Verknüpfung und jedes Entfernen einer Verknüpfung ist ein Schritt, den Sie rückgängig machen können.

Die Verknüpfung wird in einer .xlsx-Datei zusammen mit dem Text gespeichert, den die Zelle zeigte. So zeigt ein Programm, das Verknüpfungen nicht auswertet, dennoch die Wörter an, und JeffCrunch öffnet die Datei mit funktionierender Verknüpfung. OpenDocument kennt keine Möglichkeit, ein Textfeld mit einer Zelle zu verknüpfen. Eine als .ods-Datei gespeicherte Arbeitsmappe behält deshalb den Text, den die Form in diesem Moment zeigt, aber nicht die Verknüpfung.

## Aussehen eines Bildes, einer Form oder eines Textfelds ändern

Wählen Sie ein Bild, erscheint die Registerkarte **Bildformat**. Wählen Sie eine Form, eine Linie oder ein Textfeld, erscheint die Registerkarte **Formformat**. Jede Änderung ist ein Schritt, den Sie rückgängig machen können, und das Objekt behält seinen Namen, seinen Alternativtext, seine Position und seine Reihenfolge unter den anderen Objekten.

- **Formfüllung** und **Formkontur** wählen eine Farbe oder keine. **Linienart** legt die Stärke (wie dick die Linie ist) und die Striche fest (durchgezogen, gestrichelt oder gepunktet). Ein Bild hat **Bildrahmen**, seine Kontur.
- **Drehen** dreht das Objekt um eine Viertelumdrehung nach rechts oder links oder spiegelt es horizontal oder vertikal.
- **Zuschneiden** schneidet ein Bild auf einen Fensterausschnitt zu: quadratisch, 4 breit mal 3 hoch, 3 breit mal 4 hoch, 16 mal 9 oder 9 mal 16. Der Zuschnitt erfolgt gleichmäßig von der Mitte des sichtbaren Bereichs aus, und das Bild wird so groß dargestellt, wie der Rest es erlaubt. **Gesamtes Bild anzeigen** holt das Abgeschnittene zurück. **Ausfüllen** behält den Rahmen und schneidet das Bild so zu, dass es ihn ganz bedeckt. **Einpassen** zeigt das ganze Bild, und der Rahmen nimmt die Form des Bildes innerhalb des alten Rahmens an. **Auf Form zuschneiden** (die Galerie neben **Zuschneiden**) schneidet das Bild auf eine Form zu, etwa ein Oval, einen Stern oder ein abgerundetes Rechteck; die Form wird mit dem Bild gespeichert. Die Bilddatei selbst wird nicht verändert: Der abgeschnittene Teil wird nur ausgeblendet.
- **Text bearbeiten** gilt für Formen und Textfelder. Geben Sie den Text ein und wählen Sie, wie er waagerecht und senkrecht sitzt, die Größe, fett oder normal und ob er sich an die Form anpasst oder in einer Zeile bleibt. Bei Textkunst fragt das Dialogfeld stattdessen nach Text, Schriftart, Größe, Fett und Kursiv.
- **Auswahlbereich** öffnet eine Liste aller Objekte auf dem Blatt mit ihren Namen, das vorderste zuerst. Klicken Sie auf einen Namen, um dieses Objekt auszuwählen; das hilft, wenn Objekte einander verdecken. **Eine Ebene nach vorne** und **Eine Ebene nach hinten** verschieben es in der Liste.
- **Eigenschaften** legt die Größe, den Namen, den Alternativtext und fest, was das Objekt beim Verschieben oder Vergrößern von Zellen tut.

## Ein Objekt ausschneiden, kopieren und einfügen

Ein Bild oder eine Form wird wie ein Diagramm kopiert und eingefügt. Eine Form, die etwas anderes als ein Bild aus der Arbeitsmappe verwendet, etwa eine Verknüpfung zu einer Datei, kann nicht kopiert werden; JeffCrunch nennt den Grund.

Kopieren Sie ein Diagramm und fügen Sie es in dieselbe Arbeitsmappe ein, auf einem beliebigen Blatt, liest die Kopie dieselben Zellen wie das Original. Fügen Sie es in eine andere Arbeitsmappe ein, behält es die Zahlen, die es zeigte, liest aber keine Zellen mehr, denn die Zellen, die es dort lesen würde, sind nicht dieselben, aus denen es erstellt wurde.

## Mehrere Objekte auswählen, gruppieren und ausrichten

Klicken Sie auf ein Objekt und halten Sie dann **Umschalt** oder **Strg** gedrückt, während Sie auf ein weiteres klicken, um es ebenfalls auszuwählen. Klicken Sie mit **Umschalt** oder **Strg** erneut auf ein ausgewähltes Objekt, um es abzuwählen. Ausgewählte Objekte erhalten jeweils einen gestrichelten Rahmen. Sie können auch auf der Registerkarte **Formformat** die Option **Objekte auswählen** wählen und ein Feld über leere Zellen ziehen: Jedes Objekt, das vollständig im Feld liegt, wird ausgewählt. Drücken Sie **Esc** oder wählen Sie **Objekte auswählen** erneut, um wieder Zellen auszuwählen. Drücken Sie **Entf**, um alle ausgewählten Objekte in einem Schritt zu entfernen.

Sind zwei oder mehr Objekte ausgewählt, enthält die Registerkarte **Formformat** in der Gruppe **Anordnen** diese Befehle:

- **Gruppieren** fasst die ausgewählten Formen, Bilder und Linien zu einem Objekt zusammen. Es wird als Einheit verschoben und in der Größe geändert, und die Datei speichert es als Gruppe. Ein Diagramm kann nicht gruppiert werden.
- **Gruppierung aufheben** löst eine ausgewählte Gruppe auf. Jeder Teil kehrt an seine Stelle auf dem Blatt zurück.
- **Ausrichten** richtet die ausgewählten Objekte aus: **Linksbündig**, **Zentriert**, **Rechtsbündig**, **Oben ausrichten**, **Mitte ausrichten** und **Unten ausrichten**. Sie richten sich nach der Kante oder der Mitte der gesamten Auswahl. **Horizontal verteilen** und **Vertikal verteilen** verteilen drei oder mehr Objekte gleichmäßig zwischen den beiden äußersten.

Gruppieren, Gruppierung aufheben und jede Ausrichtung sind ein Schritt, den Sie rückgängig machen können.

## Benennen

Ist ein Objekt ausgewählt, zeigt das Namenfeld seinen Namen. Geben Sie dort einen neuen Namen ein und drücken Sie **Eingabe**. **Rückgängig** setzt den alten Namen wieder ein, und der neue Name wird in der Datei gespeichert.

## Löschen

Drücken Sie **Entf**, wenn das Objekt ausgewählt ist. **Rückgängig** holt es zurück.

## Was gespeichert wird

Alles oben Beschriebene wird in der Datei in einem Format gespeichert, das andere Tabellenkalkulationsprogramme lesen, sodass ein hier erstelltes Diagramm dort genauso geöffnet wird. Ein Diagramm, das aus einem anderen Programm stammt und etwas enthält, das JeffCrunch beim Umschreiben nicht behalten kann (Fehlerindikatoren, ein dreidimensionales Diagramm), lässt sich weiterhin verschieben, in der Größe ändern und löschen, aber seine Elemente und Daten werden hier nicht verändert.

## Diagramme aus anderen Programmen

Ein **Schema** ist ein Bild aus Kästchen und Pfeilen, etwa einen Ablauf, das ein anderes Programm erstellt hat. JeffCrunch zeigt ein Schema anhand des Bildes an, das die Datei daneben speichert. Klicken Sie darauf, um es auszuwählen, und ziehen Sie es, um es zu verschieben. Beim Speichern der Datei wird das Schema genauso zurückgeschrieben, wie es war, sodass das Programm, das es erstellt hat, es weiterhin ändern kann.

JeffCrunch kann den Inhalt eines Schemas noch nicht ändern, und ein Schema kann weder kopiert noch in eine Gruppe aufgenommen werden. Enthält die Datei kein Bild eines Schemas, zeigt JeffCrunch ein schlichtes graues Feld, das angibt, dass es sich um ein Schema handelt.

## Diagramme in .ods-Dateien

Eine als .ods-Datei gespeicherte Arbeitsmappe behält ihre Diagramme: Säulen-, Balken-, Linien-, Flächen-, Kreis-, Ring- und Streudiagramme, jeweils mit ihren Datenreihen und den Zellen, die sie lesen (auf diesem Blatt oder einem anderen), Kategorien, Titel, Legende, Achsen (Titel, kleinster und größter Wert, Zahlenformat), Datenbeschriftungen, Farben und Position. Eine .ods-Datei, die ein anderes Programm erstellt hat, öffnet sich mit ihren Diagrammen, die aus den Zellen gezeichnet werden, die sie lesen. Was in einem .ods-Diagramm keinen Platz hat, bleibt beim Speichern weg, und es wird genannt: Trendlinien, Fehlerindikatoren, zweite Achsen und die besonderen Erscheinungsbilder einzelner Punkte. Diagramme eines Typs, den JeffCrunch nicht zeichnet (Börsen- und Netzdiagramme), öffnen sich als Säulen ihrer Werte.
