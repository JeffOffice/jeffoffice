# Pivot-Tabellen
<!-- keywords: Pivot-Tabelle, Details anzeigen, Details einblenden, Detailebene, übergeordnete Ebene, Pivot-Tabelle einfügen, erstellen, Pivot-Tabelle verschieben, GETPIVOTDATA, Daten aus Pivot-Tabelle abrufen, aktualisieren, Alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Zwischensumme, Layout, Kompaktform, Gliederungsform, Tabellenform, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent vom Gesamtergebnis, Differenz, Laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datumswerte, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Zeilen hinter einem Wert -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste enthält beispielsweise für jeden Verkauf eine Zeile; der Bericht addiert die Umsätze nach Region und Monat. Sie schreiben dafür keine Formeln. Sie legen fest, welche Spalten der Liste an der Seite stehen, welche oben und welche addiert wird, und der Bericht füllt sich selbst.

## Ein Beispiel

Hier sehen Sie eine Liste in den Zellen A1 bis C7. Jede Zeile ist ein Verkauf.

| Region | Monat | Betrag |
|---|---|---|
| Ost | Jan | 10 |
| Ost | Jan | 5 |
| Ost | Feb | 20 |
| West | Jan | 7 |
| West | Feb | 3 |
| West | Feb | 4 |

Eine Pivot-Tabelle mit **Region** an der Seite, **Monat** oben und der Summe von **Betrag** in der Mitte sieht so aus.

| Summe von Betrag | Monat | | |
|---|---|---|---|
| Region | Jan | Feb | Gesamtergebnis |
| Ost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Gesamtergebnis | 22 | 27 | 49 |

Ost im Januar ergibt 15, weil die beiden Zeilen für Ost im Januar, 10 und 5, addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die hier verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind die Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in denen ein Feld stehen kann. Ein Feld in „Zeilen“ ergibt eine Zeile für jeden seiner verschiedenen Einträge; ein Feld in „Spalten“ ergibt eine Spalte für jeden Eintrag; ein Feld in „Werte“ ist das Feld, das addiert wird. Ein vierter Bereich, **Filter**, enthält ein Feld, das die Zeilen ausblendet, die Sie nicht auswählen.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei aufbewahrt, damit sie Summen anzeigen kann, ohne die Zellen anzusehen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen; er ändert sich also nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Pivot-Tabelle erstellen

1. Klicken Sie auf eine Zelle in der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Über den Daten muss eine Zeile mit Namen stehen.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, wenn sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben.
4. Legen Sie fest, wo die Pivot-Tabelle stehen soll: **Ein neues Blatt** (Standard) oder eine Zelle in diesem oder einem anderen Blatt. Wählen Sie eine Stelle, an der sonst nichts steht, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle ist zunächst leer, und der Bereich **Pivot-Tabellen-Felder** wird geöffnet. Aktivieren Sie die gewünschten Felder: Ein Feld mit Zahlen wird addiert, jedes andere Feld wird zu einer Zeile. Verschieben Sie die Felder anschließend wie in den nächsten Abschnitten beschrieben in die Bereiche.

Ein neues Blatt ist ein eigener Schritt für „Rückgängig“, ebenso die Tabelle. Wenn Sie als .xlsx speichern, wird die Tabelle so geschrieben, wie das Originalprogramm sie schreibt, und lässt sich dort als Pivot-Tabelle öffnen. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle in ihren Zellen anzeigt, aber nicht die Tabelle selbst; das Speichern weist Sie darauf hin.

## GETPIVOTDATA: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` liefert die Zahl, die eine Pivot-Tabelle für den von Ihnen genannten Wert sowie die genannten Zeilen und Spalten anzeigt. Für das obige Beispiel:

`=GETPIVOTDATA("Sum of Amount", $E$3, "Region", "East", "Month", "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, wie ihn die Tabelle anzeigt (zum Beispiel „Summe von Betrag“). Der Name des Felds, das addiert wird (Betrag), funktioniert ebenfalls.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Wenn Sie nur Region und Ost nennen, erhalten Sie die Summe für Ost, 35. Wenn Sie nichts nennen, erhalten Sie das Gesamtergebnis, 49. Die genannten Felder müssen in dieser Reihenfolge die ersten Felder der Zeilen oder die ersten Felder der Spalten sein.
- Wenn die Tabelle die Zahl nicht anzeigt, zeigt die Zelle `#REF!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach einer Umordnung der Tabelle die richtige Zahl liest. Um dies auszuschalten, wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie nichts daran ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, trägt die neuen Zahlen in die Zellen ein und schreibt die Pivot-Tabelle zurück, sodass das Programm, das sie erstellt hat, sie ohne Beanstandung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden und so weiter, die Datumswerte und Zahlen gruppieren und die Werte als Anteil, Differenz oder laufende Summe anzeigen. Eine Pivot-Tabelle mit einem berechneten Feld oder Element, mit einem Wert, der als Rang oder als Anteil eines übergeordneten Elements angezeigt wird, mit von Hand erstellten Gruppen von Elementen oder mit einem Datenmodell wird so angezeigt und beibehalten, wie sie gespeichert wurde; die Befehle teilen Ihnen mit, dass sie sie nicht ändern können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** wird angezeigt.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wurde, und berechnet den Bericht neu. Neue Einträge werden hinter den vorhandenen eingefügt; nicht mehr vorhandene Einträge werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle** tut dasselbe für jede Pivot-Tabelle der Arbeitsmappe.

Wenn der Bericht über Zellen hinauswachsen würde, die Inhalt haben, aktualisiert JeffCrunch nicht. Es teilt Ihnen mit, welche Zellen im Weg sind. Leeren Sie sie, oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie erneut.

**Rückgängig** macht eine Aktualisierung in einem Schritt rückgängig.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich ihrer ersten Zeile mit Namen. Geben Sie sie wie `Sheet1!A1:D100` ein, oder geben Sie den Namen einer Tabelle oder eines von Ihnen erstellten Namens ein. JeffCrunch liest sie und berechnet den Bericht neu. Wenn ein Feld der Liste in den neuen Zellen denselben Namen hat, bleibt seine Position im Bericht erhalten.

## Löschen

**Löschen** bietet zwei Möglichkeiten. **Filter löschen** entfernt jeden Filter und zeigt wieder jeden Eintrag an. **Alle löschen** entfernt jedes Feld aus dem Bericht und lässt ihn leer, bereit für einen neuen Aufbau.

## Detailebene anzeigen und übergeordnete Ebene anzeigen

Wenn ein Bericht mehr als ein Feld in den Zeilen hat, kann ein Element des ersten Felds geöffnet oder geschlossen werden. Klicken Sie auf die Zelle mit dem Namen des Elements, und wählen Sie **Übergeordnete Ebene anzeigen**, um es zu schließen: Die Zeilen des nächsten Felds darunter werden ausgeblendet, und nur seine Summe bleibt. **Detailebene anzeigen** öffnet es wieder. Ein Element des letzten Felds hat nichts unter sich. Jeder Vorgang ist ein eigener Schritt für **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabelle** markiert einen Teil des Berichts: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen an der Seite; wenn der Bericht an der Seite keine hat, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht neu, sodass seine erste Zelle dort liegt. Die Zellen, die er verlässt, werden geleert. Formeln, die auf die Zellen des Berichts verweisen, folgen ihnen, wie beim Ausschneiden und Einfügen. Wenn am neuen Ort etwas steht, verschiebt JeffCrunch den Bericht nicht und teilt Ihnen das mit. Ein **Rückgängig** verschiebt ihn zurück.

## Der Feldbereich

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Pivot-Tabellen-Felder**. Er listet die Felder der Liste mit je einem Kontrollkästchen auf und darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt in **Werte** und wird addiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche, und wählen Sie **Verschieben nach**, um es in einen anderen zu legen. **Nach oben** und **Nach unten** ändern seine Reihenfolge innerhalb seines Bereichs, und **Feld entfernen** entfernt es.
- Bei mehr als einem Wert erscheint in **Spalten** eine Zeile mit dem Namen **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte an der Seite anzuzeigen.
- **Wertfeldeinstellungen** (Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er zusammengefasst wird: **Summe**, **Anzahl**, **Anzahl Zahlen**, **Mittelwert**, **Max**, **Min**, **Produkt**, **Standardabweichung (Stichprobe)** und **Standardabweichung (Grundgesamtheit)** (die Standardabweichung einer Stichprobe oder aller Werte) sowie **Varianz (Stichprobe)** und **Varianz (Grundgesamtheit)** (die Varianz derselben). Außerdem legt es fest, wie der Wert angezeigt wird und welches Zahlenformat er hat. Siehe nächster Abschnitt.
- **Feldeinstellungen** (Doppelklick auf ein Feld in „Filter“, „Spalten“ oder „Zeilen“ oder die Schaltfläche) legt die Eigenschaften eines Felds fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt für **Rückgängig**.

## Werte anzeigen als und das Zahlenformat eines Werts

In den **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts anzeigt.

- **% des Gesamtergebnisses**: die Zelle geteilt durch die Summe des gesamten Berichts. Im Beispiel sind Ost im Januar 15 von 49, also 30,6 %.
- **% des Spaltenergebnisses** und **% des Zeilenergebnisses**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie sich die Zelle zu dem verhält, was ihre Zeile und Spalte ergäben, wenn die Daten gleichmäßig verteilt wären. Eine Zahl über 1 bedeutet mehr als den gleichmäßigen Anteil.
- **Differenz von**, **% von** und **% Differenz von**: die Zelle im Vergleich zu einem anderen Element eines Felds. Wählen Sie das **Basisfeld** (ein Feld in „Zeilen“ oder „Spalten“) und das **Basiselement** (eines seiner Elemente oder das vorherige bzw. nächste). Wenn zum Beispiel Monat das Basisfeld und Jan das Basiselement ist, zeigt Feb, wie viel mehr oder weniger es als Jan ist. Das Basiselement selbst bleibt bei einer Differenz leer und ist bei **% von** 100 %.
- **Laufende Summe in**: Jede Zelle addiert die Zellen davor entlang des Basisfelds.

**Zahlenformat** akzeptiert einen Formatcode wie `0.00`, `#,##0` oder `0.0%`. Lassen Sie das Feld leer, um das Format der Zellen beizubehalten. Ein als Anteil angezeigter Wert erhält `0.00%`, sofern Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** macht es zusammen mit den Zahlen rückgängig.

## Feldeinstellungen

Markieren Sie ein Feld im Feld **Filter**, **Spalten** oder **Zeilen**, und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feldeinstellungen** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld anzeigt.
- **Teilergebnisse**: **Automatisch** zeigt für jede Gruppe des Felds eine Summe an, **Keine** nicht.
- **Elementbeschriftungen anzeigen in**: Kompaktform, Gliederungsform oder Tabellenform, wie bei „Berichtslayout“.
- **Nach jedem Element leere Zeile einfügen** fügt nach jeder Gruppe eine leere Zeile ein.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe an (Gliederungs- und Tabellenform).
- **Sortieren**: die vorhandene Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z oder von Z bis A sortieren. **Kleinster bis größter nach einem Wert** und **Größter bis kleinster nach einem Wert** sortieren die Elemente nach den Zahlen in einem Wertfeld. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filter** blendet Elemente nach ihren Beschriftungen aus (beginnt mit, endet mit, enthält, ist gleich) oder nach einem Wert (größer als, kleiner als, zwischen und so weiter) oder behält nur die obersten oder untersten Elemente nach einem Wert. Der Filter wird berechnet, wenn Sie OK wählen. Er blendet die ausgeschlossenen Elemente aus; die Kontrollkästchen darunter zeigen, welche, und Sie können ein Element aktivieren, um es wieder anzuzeigen.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Felds mit je einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um das Element auszublenden; die Summen lassen es dann aus.

## Datumswerte und Zahlen gruppieren

Markieren Sie ein Feld mit Datumswerten oder Zahlen in **Zeilen** oder **Spalten**, und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feld gruppieren**. Gruppen werden aus den Daten gebildet; die Daten müssen in dieser Spalte also nur Zahlen oder Datumswerte enthalten, ohne Text und ohne leere Zellen.

- **Datumswerte**: Aktivieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste aktivierte Teil bleibt am Feld. Für jeden gröberen Teil fügt JeffCrunch ein Feld mit entsprechendem Namen hinzu (zum Beispiel **Jahre**) und stellt es im selben Bereich vor das Datumsfeld. **Beginnend bei** und **Endet bei** sind das erste und das letzte Datum; belassen Sie sie unverändert, um das kleinste und das größte Datum der Daten zu verwenden. Datumswerte vor dem Anfang oder nach dem Ende kommen in zwei zusätzliche Gruppen, die mit `<` und `>` beginnen.
- **Zahlen**: **Beginnend bei**, **Endet bei** und **Nach** bilden Gruppen wie 1-10, 11-20 und so weiter.

**Gruppierung aufheben** entfernt die Gruppen und die für sie hinzugefügten Felder. Beim Aktualisieren werden die Gruppen aus den neuen Daten erneut gebildet. Wenn zwei Pivot-Tabellen aus denselben Daten erstellt wurden, können keine Gruppen gebildet werden, weil eine Gruppe ein Teil der gemeinsamen Daten ist.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert einer Pivot-Tabelle, oder wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Details anzeigen**. JeffCrunch erstellt ein neues Blatt mit dem Namen Detail 1, 2 und so weiter, das die Zeilen der Liste enthält, aus denen sich die Zahl zusammensetzt. In der ersten Zeile stehen die Namen der Felder. Mit F2 bearbeiten Sie die Zelle weiterhin, wenn sie bearbeitet werden kann. **Rückgängig** entfernt die Zeilen, ein weiteres **Rückgängig** entfernt das Blatt.

## Erweitern und reduzieren

Ein Feld, dessen Elemente weitere Elemente unter sich haben, kann die Zeilen unter seinen Elementen ein- oder ausblenden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** tun dies für jedes Element des Felds, das Sie im Bereich markieren.

## Pivot-Tabellen-Optionen

**Optionen** auf der Registerkarte **Pivot-Tabelle** öffnet ein Feld mit den Einstellungen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn tragen.
- **Bei Fehlerwerten anzeigen** und **Bei leeren Zellen anzeigen** tragen einen eigenen Text in diese Zellen ein.
- **Zellen mit Beschriftungen verbinden und zentrieren**, **Spaltenbreiten bei Aktualisierung der Tabelle an Inhalt anpassen** und **Zellenformatierung bei Aktualisierung der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtsummen für Zeilen anzeigen** und **für Spalten** entsprechen den Auswahlmöglichkeiten unter „Gesamtergebnisse“.
- **Daten beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Das ist wie jede Aktualisierung ein Schritt für „Rückgängig“.

## Formatvorlage: Kopfzeilen, Bänder und der Katalog

Die **Formatvorlagenoptionen** auf der Registerkarte **Pivot-Tabelle** schalten die Kopfzeile, die erste Spalte (Zeilenüberschriften) und die Bänder von Zeilen und Spalten ein oder aus. **Formatvorlagen** enthält einen Katalog mit Darstellungen, und **Format löschen** entfernt die Darstellung. Diese Darstellungen werden auf dieselbe Weise gezeichnet wie die Darstellungen von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** legen die leere Zeile und die wiederholten Beschriftungen für alle Felder der Zeilen gleichzeitig fest.

## Entwurf: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** zeigt oder verbirgt die Summe jeder Gruppe, oben oder unten in der Gruppe. Wenn in „Zeilen“ mehr als ein Feld steht, ist die Summe jedes äußeren Felds ein Teilergebnis.
- **Gesamtergebnisse** bietet vier Möglichkeiten: aus für Zeilen und Spalten, ein für beide, ein nur für Zeilen (eine Spalte mit Summen rechts, eine für jede Zeile) und ein nur für Spalten (eine Zeile mit Summen unten).
- **Berichtslayout** ist die Form der Beschriftungen. In der **Kompaktform** teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas nach rechts eingerückt angezeigt. In der **Gliederungsform** hat jedes Feld eine eigene Spalte, und jede Gruppe hat über ihren Elementen eine eigene Zeile. In der **Tabellenform** hat jedes Feld eine eigene Spalte, der Name der Gruppe steht in ihrer ersten Zeile, und das Teilergebnis steht unter der Gruppe.

## Dieselbe Liste mit einem Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und eine Zeile darüber zeigt **Monat** und **(Alle)**. Die Zahlen sind nun die Summe jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie auf eine Zelle in einer Pivot-Tabelle, und wählen Sie **Einfügen** und dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm der Werte der Tabelle. Die Gesamtergebnisse lässt es weg. Das Diagramm ist in der Datei als Diagramm der Pivot-Tabelle markiert. Es liest die Zellen der Tabelle; seine Zahlen ändern sich also, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Wenn die Tabelle mehr Zeilen oder Spalten erhält, erstellen Sie das Diagramm neu, weil das Diagramm die Zellen behält, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert, und eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie auf eine Zelle in einer Pivot-Tabelle, und klicken Sie dann auf der Registerkarte „Pivot-Tabelle“ auf **Datenschnitte**. Der Bereich „Datenschnitte“ wird geöffnet und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat für jedes Element seines Felds eine Schaltfläche. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut darauf, um alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt, und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt für „Rückgängig“, und „Rückgängig“ stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts.

**Berichtsverbindungen** gibt an, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den von ihr gewählten Datumsbereich an. JeffCrunch zeigt die Datenschnitte in diesem Bereich an, nicht über den Zellen, und ändert eine Zeitachse noch nicht. Beide werden in der Datei so beibehalten, wie sie gespeichert wurden.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste bis zu fünf Millionen Zeilen und ein Bericht bis zu vier Millionen Zellen. Eine Datei, die darüber hinausgeht, wird so angezeigt, wie sie gespeichert wurde, und kann nicht geändert werden. Das erneute Lesen von hunderttausend Zeilen mit zwanzig Spalten und die Berechnung des Berichts dauerten auf dem Computer, auf dem dies gemessen wurde, etwa eine halbe Sekunde; ein langsamerer Computer braucht länger.
