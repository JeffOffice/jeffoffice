# Diagramme

<!-- keywords: Diagramm, Diagramme, Grafik, Säulendiagramm, Balkendiagramm, Liniendiagramm, Kreisdiagramm, Ringdiagramm, Fläche, Streudiagramm, Diagramm einfügen, Diagrammdaten, Daten bearbeiten, Diagrammtitel, Legende, Datenbeschriftungen, Diagrammtyp, Diagrammformat, Achsen, Achsentitel, Gitternetzlinien, Diagrammformatvorlage, Diagrammelemente, Layouts, Farben, Darstellungen, Daten tauschen, Zeilen und Spalten tauschen, Diagrammart, Achse formatieren, Fehlerindikatoren, Trendlinie, Diagrammfilter, Diagrammteile -->

Ein **Diagramm** macht aus Zahlen ein Bild, etwa Säulen, eine Linie oder Teile eines Kreises. Dieses Thema zeigt, wie Sie ein Diagramm in ein Dokument einfügen, eigene Zahlen eingeben und das Aussehen ändern.

## Diagramm einfügen

1. Setzen Sie die Einfügemarke an die Stelle, an der das Diagramm stehen soll.
2. Klicken Sie auf **Einfügen > Diagramm**, und wählen Sie eine Art: Säule, gestapelte Säule, 100 % gestapelte Säule, Balken, gestapelter Balken, Linie, Linie mit Markierungen, Kreis, Ring, Fläche, gestapelte Fläche, Punkt (XY), Netz, Netz mit Markierungen, gefülltes Netz, Hoch-Tief-Schluss, Eröffnung-Hoch-Tief-Schluss, Volumen-Hoch-Tief-Schluss oder Volumen-Eröffnung-Hoch-Tief-Schluss.

Ein **Börsendiagramm** zeigt Kurse: Für jede Kategorie erscheint eine Linie vom höchsten bis zum niedrigsten Kurs, mit einem kleinen Punkt für den Schlusskurs. Ist ein Eröffnungskurs vorhanden, wird zwischen Eröffnung und Schluss ein Balken gezeichnet (hell bei einem Anstieg, dunkel bei einem Rückgang). Das Diagramm entsteht aus Ihren Spalten genau in der Reihenfolge, in der sie stehen, von links nach rechts. Es wird nicht geprüft, ob die Zahlen zu ihrer Stelle passen. Ordnen Sie die Spalten daher so: Hoch, Tief und Schluss; Eröffnung, Hoch, Tief und Schluss; Volumen, Hoch, Tief und Schluss; oder Volumen, Eröffnung, Hoch, Tief und Schluss. Stehen die Spalten in einer anderen Reihenfolge, wird das Diagramm trotzdem gezeichnet, die Markierungen bedeuten dann aber etwas anderes (der Punkt für den Schluss steht auf der dritten Kursspalte, und der Kasten reicht von der ersten bis zur letzten Kursspalte). Bei einem Volumen werden die Volumenwerte als Säulen an der linken Achse gelesen, die Kurse an einer zweiten Achse rechts. Passt die Anzahl der Spalten nicht zur Art, nennt JeffWrite die benötigte Anzahl und erstellt kein Diagramm.

Ein **gestapeltes** Diagramm legt die Datenreihen übereinander. Ein **100 % gestapeltes** Diagramm macht jeden Stapel gleich hoch, sodass Sie Anteile sehen. Ein **Punkt-(XY-)Diagramm** setzt jeden Punkt dorthin, wo seine beiden Zahlen es angeben.

Das Diagramm erscheint mit Beispielzahlen, im Textfluss eingefügt wie ein großer Buchstabe. Die Registerkarte **Diagrammformat** wird geöffnet.

## Eigene Zahlen eingeben

Markieren Sie das Diagramm, und klicken Sie auf **Diagrammformat > Daten bearbeiten**. Eine Tabelle wird geöffnet.

- Die erste Spalte enthält die Wörter unter den Punkten (die **Kategorien**), etwa die Monate.
- Die erste Zeile enthält den Namen jeder **Datenreihe**, also eines Zahlensatzes, etwa eines Produkts.
- Die übrigen Zellen enthalten die Zahlen.

Klicken Sie auf **Zeile hinzufügen** oder **Spalte hinzufügen**, um mehr anzulegen, oder auf **Letzte Zeile entfernen** bzw. **Letzte Spalte entfernen**, um weniger zu haben. Klicken Sie auf **OK**. Ist eine Zelle keine Zahl, bleibt das Diagramm unverändert, und die Statusleiste zeigt an, in welcher Zeile das Problem liegt. Ein Diagramm kann bis zu 200 Kategorien und 24 Datenreihen haben.

Bei einem Punktdiagramm enthält die erste Spalte die Zahlen, die die Punkte von links nach rechts einordnen.

## Diagramme aus anderen Programmen

Manche Diagramme, die andere Programme gespeichert haben, halten ihre Zahlen in einer kleinen **Arbeitsmappe** (einer Tabellenkalkulationsdatei), die in Ihrer Datei eingebettet ist. Markieren Sie ein solches Diagramm, und klicken Sie auf **Diagrammformat > Daten bearbeiten**. Die Tabelle zeigt die Zellen dieser Arbeitsmappe, die das Diagramm liest. Ändern Sie eine Zahl, eine Beschriftung unter einem Punkt oder den Namen einer Datenreihe, und klicken Sie auf **OK**. Die Änderung wird gleichzeitig in die Arbeitsmappe und in das Diagramm übernommen, als ein Schritt, der sich rückgängig machen lässt. So zeigen auch andere Programme die neuen Zahlen.

Bei einem solchen Diagramm können Sie keine Zeile und keine Spalte hinzufügen oder entfernen. **Diagrammdaten** und **Zeilen und Spalten tauschen** sind abgeblendet, weil das Diagramm an diese Zellen gebunden ist. Dasselbe gilt, wenn das Diagramm Zellen in einer anderen Datei liest, in einem Bereich, der weder eine einzelne Spalte noch eine einzelne Zeile ist, oder in Zellen, die eine Formel, ein Datum oder Wörter enthalten, wo eine Zahl stehen sollte. Der Befehl ist dann abgeblendet, und sein Hinweis sagt, warum.

## Aussehen ändern

Diese Befehle finden Sie auf der Registerkarte **Diagrammformat**. Jeder Befehl lässt sich mit einem Schritt rückgängig machen, und das Diagramm behält seine Farben und das übrige Aussehen.

- **Diagrammtyp ändern** wandelt das Diagramm in eine andere Art mit denselben Zahlen um. Beim Wechsel zu einer Börsenart müssen die Datenreihen in der von dieser Art verlangten Reihenfolge stehen. Sind sie es nicht, nennt JeffWrite die Reihenfolge und lässt das Diagramm unverändert.
- **Diagrammtitel** blendet den Titel ein oder aus. **Diagrammtitel bearbeiten** ändert seinen Wortlaut.
- **Legende** ist der Schlüssel, der zeigt, welche Farbe zu welcher Datenreihe gehört. Sie können ihn rechts, oben, links oder unten platzieren oder ausblenden.
- **Achsen** blendet die Linie mit den Wörtern unter dem Diagramm (die **Kategorien**) und die Linie mit den Zahlen daneben (die **Werte**) ein oder aus und legt für beide einen Titel fest. Ein Kreisdiagramm hat keine Achsen.
- **Gitternetzlinien** zieht blasse Linien quer durch das Diagramm, damit das Auge den Zahlen leichter folgen kann. Die **Haupt**linien liegen bei den großen Zahlen an der Seite, die **Hilfs**linien dazwischen.
- **Diagrammformatvorlagen** wählt eine von acht Gesamtformatvorlagen.
- **Datenbeschriftungen** schreibt jede Zahl auf die Säule, den Punkt oder das Kreissegment.
- **Textumbruch**, **Alternativtext** sowie **Höhe** und **Breite** funktionieren wie bei Bildern. Siehe [Bilder hinzufügen und ändern](pictures).

## Die Registerkarte „Diagramm“

Markieren Sie ein Diagramm, dann wird neben der Registerkarte **Diagrammformat** die Registerkarte **Diagramm** geöffnet. Alles darauf verändert das Diagramm in einem Schritt, den Sie rückgängig machen können.

- **Diagrammelemente** fügt einen Teil des Diagramms hinzu, entfernt ihn oder legt ihn fest: **Achsen**, **Achsentitel**, **Diagrammtitel**, **Datenbeschriftungen** (was sie enthalten, wo sie stehen und welches Zeichen zwischen ihnen steht), **Gitternetzlinien**, **Legende**, **Fehlerindikatoren** (kurze Balken, die zeigen, wie weit eine Zahl abweichen kann), **Linien** (Tropflinien, Hoch-Tief-Linien sowie Auf- und Abwärtsbalken), **Datentabelle** (ein Raster mit den Zahlen unter dem Diagramm, mit einer Spalte für jede Kategorie; über die Liste wird die Tabelle ein- und ausgeschaltet und werden die Linien zwischen Zeilen und Spalten, der Rahmen und die Legendenschlüssel festgelegt, die die Farbe jeder Datenreihe neben ihrem Namen zeigen; sie wird für Säulen-, Linien- und Flächendiagramme angeboten) und **Trendlinie** (eine Linie, die zeigt, wohin sich die Zahlen entwickeln, auf Wunsch mit ihrer Gleichung).
- **Layouts** legt mehrere Teile gleichzeitig fest, etwa Titel, Legende, Beschriftungen und Gitternetzlinien.
- **Farben** gibt den Datenreihen einen neuen Farbsatz. **Designs** gibt dem gesamten Diagramm ein Erscheinungsbild, etwa schlicht, klar, dunkel oder weich.

Hinweis: Die Befehle **Layouts**, **Farben** und **Designs** sind dieselben wie in der Gruppe der Diagrammformatvorlagen. Ihr Name in der Oberfläche richtet sich nach dem Programmtext.
- **Zeilen und Spalten tauschen** dreht die Zahlen um: Die Datenreihen werden zu den Wörtern unter den Punkten, und diese Wörter werden zu Datenreihen.
- **Diagrammdaten** listet die Datenreihen auf. Aktivieren Sie die zu zeichnenden, ändern Sie ihre Namen, und verschieben Sie sie nach oben oder unten.
- **Daten bearbeiten** öffnet die Tabelle mit den Zahlen.
- **Diagrammart** zeigt eine Vorschau jeder Art. Sie können eine Art für das gesamte Diagramm wählen oder für jede Datenreihe eine eigene, etwa Säulen mit einer Linie darüber.
- **Diagrammteil formatieren** legt Füllung und Linie der Diagrammfläche, der Zeichnungsfläche (des Rahmens, in dem die Markierungen stehen) oder einer einzelnen Datenreihe fest. Bei einer Linie legen Sie außerdem fest, ob sie geglättet wird und welche Markierung an jedem Punkt steht.
- **Aktuelle Auswahl** listet die Teile des Diagramms auf: die Diagrammfläche, die Zeichnungsfläche und jede Datenreihe. Wählen Sie einen Teil aus, dann öffnet **Auswahl formatieren** das Formatierungsfenster für genau diesen Teil. **Auf Formatvorlage zurücksetzen** entfernt, was für den gewählten Teil festgelegt wurde, und stellt die Werte wieder her, die die Formatvorlage des Diagramms vorgibt. Bei der Diagrammfläche sind das Füllung, Linie und die Textfarbe, die ein Erscheinungsbild gesetzt hat (die helle Schrift des dunklen Erscheinungsbilds kehrt zur Textfarbe des Designs zurück). Bei der Zeichnungsfläche sind es Füllung und Linie. Bei einer Datenreihe ist es ihre Farbe (die die Auswahl unter Farben oder die Formatvorlage ihr gibt), ihre Kontur, ihre Markierungen in der Form, die diese Diagrammart vorsieht, und die Farben ihrer Punkte. Eine geglättete Linie bleibt erhalten, wenn die Diagrammart eine geglättete ist. Titel, Legende, Beschriftungen, Achsen, Gitternetzlinien und Daten bleiben unverändert, und ein Teil, der bereits der Formatvorlage entspricht, wird nicht verändert. Der Befehl lässt sich mit **Rückgängig** als ein Schritt zurücknehmen.
- **Horizontale Achse formatieren** und **Vertikale Achse formatieren** legen die kleinsten und größten Zahlen, die Größe jedes Schritts, die Reihenfolge, die Stelle, an der die andere Achse schneidet, das Aussehen der Zahlen und die Teilstriche fest.

## Die Schaltflächen neben einem Diagramm

Drei kleine Schaltflächen stehen rechts neben einem markierten Diagramm.

- **Diagrammelemente** (das Plus) öffnet dasselbe Menü wie der Befehl **Diagrammelemente** auf der Registerkarte Diagramm.
- **Diagrammfarben und -formatvorlagen** (der Pinsel) listet die Farbsätze und Darstellungen auf.
- **Diagrammfilter** (der Trichter) listet die Datenreihen auf. Deaktivieren Sie eine Datenreihe, um sie aus dem Bild zu nehmen. Die Datei behält sie trotzdem, sodass sie wieder erscheint, wenn Sie sie erneut aktivieren. Ein Diagramm zeigt immer mindestens eine Datenreihe an.

## Diagramm in einer OpenDocument-Datei speichern

Speichern Sie das Dokument als **OpenDocument-Text** (`.odt`), wird jedes Diagramm als eigenes Objekt abgelegt und beim Öffnen wieder als Diagramm angezeigt, das Sie ändern können. Art, Zahlen, Titel, Legende, Beschriftungen, Farben, Achsen, Gitternetzlinien, bis zu vier Trendlinien je Datenreihe und Fehlerindikatoren bleiben erhalten. Einige Teile passen nicht in dieses Format, etwa eine Datentabelle, Tropflinien, Hoch-Tief-Linien, Auf- und Abwärtsbalken sowie Fehlerindikatoren mit eingegebenen Beträgen. JeffWrite zeigt beim Speichern an, was ausgelassen wurde.

## Diagramme in anderen Dokumenten

JeffWrite zeigt die Diagramme, die ein Dokument enthält, und behält sie unverändert bei. Es kann Art, Titel, Legende und Beschriftungen eines Diagramms ändern, das es vollständig schreiben kann. Enthält ein Diagramm Teile, die JeffWrite nicht ändert, etwa Spezialeffekte, weist das Programm darauf hin und lässt das Diagramm unberührt. Liest ein Diagramm seine Zahlen aus einem Tabellenblatt, das bei ihm gespeichert ist, können Sie alles ändern außer den Zahlen.
