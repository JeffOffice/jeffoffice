# Pivot-Tabellen
<!-- keywords: Pivot-Tabelle, Liste, Formeln auflisten, berechnetes Feld, Berichtsfilterseiten, Detailebene anzeigen, Details einblenden, Details ausblenden, Pivot-Tabelle einfügen, erstellen, verschieben, GETPIVOTDATA, PIVOTDATENZUORDNEN, aktualisieren, alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, kompakt, Gliederung, tabellarisch, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent vom Gesamtergebnis, Differenz, laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datum, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Details anzeigen, Zeilen hinter einem Wert -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste enthält beispielsweise für jeden Verkauf eine Zeile; der Bericht summiert die Umsätze nach Region und Monat. Sie schreiben keine Formeln. Sie legen fest, welche Spalten der Liste an der Seite stehen, welche oben und welche summiert wird, und der Bericht füllt sich von selbst.

## Ein Beispiel

Hier ist eine Liste in den Zellen A1 bis C7. Jede Zeile ist ein Verkauf.

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

Ost im Januar ergibt 15, weil die beiden Zeilen für Ost im Januar, 10 und 5, addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ jede Spalte, und die Zelle in der Ecke addiert alles.

Die hier verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind die Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in die ein Feld gelegt werden kann. Ein Feld in „Zeilen“ ergibt eine Zeile für jeden seiner verschiedenen Einträge, ein Feld in „Spalten“ ergibt jeweils eine Spalte, und ein Feld in „Werte“ wird summiert. Ein vierter Bereich, **Filter**, enthält ein Feld, das die Zeilen ausblendet, die Sie nicht auswählen.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei speichert, damit sie Ergebnisse anzeigen kann, ohne die Zellen zu lesen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen; er ändert sich also nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Pivot-Tabelle erstellen

1. Klicken Sie auf eine Zelle in der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste benötigt oberhalb der Daten eine Zeile mit Namen.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, falls sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben.
4. Wählen Sie, wohin die Pivot-Tabelle kommt: **Neues Blatt** (Standard) oder eine Zelle auf diesem oder einem anderen Blatt. Wählen Sie eine Stelle, an der nichts anderes steht, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle ist zunächst leer, und der Bereich **Pivot-Tabellen-Felder** wird geöffnet. Aktivieren Sie die gewünschten Felder: Ein Feld mit Zahlen wird summiert, jedes andere Feld wird zu einer Zeile. Verschieben Sie die Felder dann, wie in den nächsten Abschnitten beschrieben, in die Bereiche.

Ein neues Blatt und die Tabelle sind jeweils ein eigener Schritt von „Rückgängig“. Beim Speichern als .xlsx wird die Tabelle so geschrieben wie vom ursprünglichen Programm, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle anzeigt, in ihren Zellen, aber nicht die Tabelle selbst; das Speichern weist Sie darauf hin.

## GETPIVOTDATA: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` gibt die Zahl zurück, die eine Pivot-Tabelle für den Wert und die von Ihnen genannten Zeilen und Spalten anzeigt. Für das obige Beispiel:

`=GETPIVOTDATA("Summe von Betrag", $E$3, "Region", "Ost", "Monat", "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, wie ihn die Tabelle anzeigt (zum Beispiel Summe von Betrag). Auch der Name des summierten Felds (Betrag) funktioniert.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Nennen Sie nur Region, Ost, erhalten Sie die Summe für Ost, 35. Nennen Sie nichts, erhalten Sie das Gesamtergebnis, 49. Die genannten Felder müssen die ersten Felder in den Zeilen oder die ersten Felder in den Spalten sein, in dieser Reihenfolge.
- Wenn die Tabelle die Zahl nicht anzeigt, zeigt die Zelle `#REF!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach einer Umordnung der Tabelle die richtige Zahl liest. Zum Ausschalten wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie nichts daran ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, schreibt die neuen Zahlen in die Zellen und schreibt die Pivot-Tabelle zurück, sodass das Programm, das sie erstellt hat, sie ohne Beanstandung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die summieren, zählen, Mittelwerte bilden usw., die Datums- und Zahlenwerte gruppieren und die Werte als Anteil, Differenz oder laufende Summe anzeigen. Eine Pivot-Tabelle mit einem berechneten Feld oder Element, mit einem Wert als Rang oder als Anteil eines übergeordneten Elements, mit von Hand erstellten Elementgruppen oder mit einem Datenmodell wird so angezeigt und beibehalten, wie sie gespeichert wurde, und die Befehle teilen Ihnen mit, dass sie sie nicht ändern können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** wird angezeigt.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wurde, und berechnet den Bericht neu. Neue Einträge folgen auf die vorhandenen; Einträge, die es nicht mehr gibt, werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle** führt dasselbe für alle Pivot-Tabellen der Arbeitsmappe aus.

Wenn der Bericht über Zellen hinauswachsen würde, die Inhalt haben, aktualisiert JeffCrunch nicht. Es teilt Ihnen mit, welche Zellen im Weg sind. Leeren Sie sie oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie erneut.

**Rückgängig** macht eine Aktualisierung in einem Schritt rückgängig.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich der ersten Zeile mit Namen. Geben Sie sie wie `Sheet1!A1:D100` ein, oder geben Sie den Namen einer Tabelle oder eines von Ihnen erstellten Namens ein. JeffCrunch liest sie und berechnet den Bericht neu. Wenn ein Feld der Liste in den neuen Zellen denselben Namen hat, bleibt seine Position im Bericht erhalten.

## Löschen

**Löschen** bietet zwei Möglichkeiten. **Filter löschen** entfernt alle Filter und zeigt wieder alle Einträge an. **Alle löschen** entfernt alle Felder aus dem Bericht und lässt ihn leer, bereit für einen Neuaufbau.

## Details einblenden und ausblenden

## Berichtsfilterseiten

Ein Filter steht über einer Pivot-Tabelle und lässt Sie jeweils ein Element betrachten, zum Beispiel einen Monat. **Berichtsfilterseiten anzeigen** auf der Registerkarte **Pivot-Tabelle** erstellt auf einmal eine Seite für jedes Element. Jede Seite ist ein neues Blatt, nach dem Element benannt, mit einer eigenen Kopie der Tabelle. Der Filter jeder Kopie ist auf dieses Element eingestellt. Wenn die Tabelle mehr als einen Filter hat, wählen Sie den aus, der verwendet werden soll. Ein einziges **Rückgängig** entfernt alle Seiten.

Wenn ein Bericht mehr als ein Feld in den Zeilen hat, kann ein Element des ersten Felds geöffnet oder geschlossen werden. Klicken Sie auf die Zelle mit dem Namen des Elements, und wählen Sie **Details ausblenden**, um es zu schließen: Die Zeilen des nächsten Felds darunter werden ausgeblendet, und nur seine Summe bleibt. **Details einblenden** öffnet es wieder. Ein Element des letzten Felds hat nichts darunter. Jede Aktion ist ein Schritt von **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabelle** markiert einen Teil des Berichts: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen an der Seite; wenn der Bericht keine an der Seite hat, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht neu, sodass seine erste Zelle dort liegt. Die verlassenen Zellen werden geleert. Formeln, die auf Zellen des Berichts verweisen, folgen diesen, wie beim Ausschneiden und Einfügen. Wenn am neuen Ort bereits etwas steht, verschiebt JeffCrunch den Bericht nicht und teilt Ihnen dies mit. Ein **Rückgängig** verschiebt ihn zurück.

## Der Feldbereich

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Pivot-Tabellen-Felder**. Er listet die Felder der Liste mit je einem Kontrollkästchen auf, darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt in **Werte** und wird summiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche, und wählen Sie **Verschieben nach**, um es in einen anderen zu legen. **Nach oben** und **Nach unten** ändern seine Reihenfolge im Bereich, und **Feld entfernen** entfernt es.
- Bei mehr als einem Wert erscheint in **Spalten** eine Zeile namens **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte an der Seite anzuzeigen.
- **Wertfeldeinstellungen** (Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er zusammengefasst wird: **Summe**, **Anzahl**, **Anzahl Zahlen**, **Mittelwert**, **Max**, **Min**, **Produkt**, **Standardabw.** und **Standardabw. (Grundgesamtheit)** (die Standardabweichung einer Stichprobe bzw. aller Werte) sowie **Varianz** und **Varianz (Grundgesamtheit)** (die Varianz derselben). Außerdem legt es fest, wie der Wert angezeigt wird, und sein Zahlenformat. Siehe nächster Abschnitt.
- **Feldeinstellungen** (Doppelklick auf ein Feld in „Filter“, „Spalten“ oder „Zeilen“ oder die Schaltfläche) legt die Einstellungen eines Felds fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt von **Rückgängig**.

## Werte anzeigen als und das Zahlenformat eines Werts

In den **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts anzeigt.

- **% des Gesamtergebnisses**: die Zelle geteilt durch die Summe des gesamten Berichts. Im Beispiel sind Ost im Januar 15 von 49, das sind 30,6 %.
- **% des Spaltenergebnisses** und **% des Zeilenergebnisses**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie die Zelle im Vergleich zu dem steht, was ihre Zeile und Spalte bei gleichmäßiger Verteilung der Daten ergäben. Eine Zahl über 1 bedeutet mehr als den gleichmäßigen Anteil.
- **Differenz von**, **% von** und **% Differenz von**: die Zelle im Vergleich zu einem anderen Element eines Felds. Wählen Sie das **Basisfeld** (ein Feld in Zeilen oder Spalten) und das **Basiselement** (eines seiner Elemente oder das vorherige bzw. nächste). Wenn zum Beispiel Monat das Basisfeld und Jan das Basiselement ist, zeigt Februar, um wie viel er über oder unter dem Januar liegt. Das Basiselement selbst bleibt bei einer Differenz leer und beträgt bei **% von** 100 %.
- **Laufende Summe in**: Jede Zelle addiert die vorherigen Zellen entlang des Basisfelds.
- **Rang vom Kleinsten zum Größten** und **Rang vom Größten zum Kleinsten**: Jede Zelle zeigt ihren Platz, 1, 2, 3 usw., unter den Zellen des Basisfelds in derselben Zeile oder Spalte. Rang 1 ist die kleinste bzw. die größte Zahl.
- **% des übergeordneten Zeilenergebnisses** und **% des übergeordneten Spaltenergebnisses**: Jede Zelle zeigt, welchen Anteil sie an ihrem übergeordneten Element hat. Das übergeordnete Element eines Elements ist das Element eine Ebene höher, etwa das Jahr, das einen Monat enthält. Ein Element ohne übergeordnetes Element wird als Teil des Gesamtergebnisses angezeigt.

**Zahlenformat** nimmt einen Formatcode wie `0.00`, `#,##0` oder `0.0%` entgegen. Lassen Sie das Feld leer, um das Format der Zellen beizubehalten. Ein als Anteil angezeigter Wert erhält `0.00%`, sofern Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** macht es zusammen mit den Zahlen rückgängig.

## Feldeinstellungen

Wählen Sie ein Feld im Feld **Filter**, **Spalten** oder **Zeilen** aus, und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feldeinstellungen** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld anzeigt.
- **Teilergebnisse**: **Automatisch** zeigt eine Summe für jede Gruppe des Felds, **Ohne** nicht.
- **Elementbeschriftungen anzeigen in**: kompakte Form, Gliederungsform oder tabellarische Form, wie bei „Berichtslayout“.
- **Nach jedem Element eine Leerzeile einfügen** fügt nach jeder Gruppe eine leere Zeile ein.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe (Gliederungs- und tabellarische Form).
- **Sortieren**: die Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z bzw. Z bis A sortieren. **Kleinster bis größter nach einem Wert** und **Größter bis kleinster nach einem Wert** sortieren die Elemente nach den Zahlen eines Wertfelds. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filter** schließt Elemente nach ihren Beschriftungen aus (beginnt mit, endet mit, enthält, ist gleich) oder nach einem Wert (größer als, kleiner als, zwischen usw.) oder behält nur die obersten oder untersten Elemente nach einem Wert. Der Filter wird berechnet, wenn Sie auf „OK“ klicken. Er blendet die ausgeschlossenen Elemente aus; die Kontrollkästchen darunter zeigen, welche, und Sie können ein Element aktivieren, um es wieder anzuzeigen.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Felds mit je einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um das Element auszublenden, und die Summen lassen es aus.

## Datums- und Zahlenwerte gruppieren

Wählen Sie ein Feld mit Datums- oder Zahlenwerten in **Zeilen** oder **Spalten** aus, und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feld gruppieren**. Gruppen werden aus den Daten gebildet; die Daten dürfen in dieser Spalte also nur Zahlen oder Datumswerte enthalten, keinen Text und keine leeren Zellen.

- **Datum**: Aktivieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste aktivierte Teil bleibt am Feld. Für jeden gröberen Teil fügt JeffCrunch ein nach ihm benanntes Feld hinzu (zum Beispiel **Jahre**) und setzt es im selben Bereich vor das Datumsfeld. **Beginnend bei** und **Endet bei** sind das erste und das letzte Datum; belassen Sie sie unverändert, um das kleinste und das größte Datum der Daten zu verwenden. Datumswerte vor dem Beginn oder nach dem Ende kommen in zwei zusätzliche Gruppen, die mit `<` und `>` beginnen.
- **Zahlen**: **Beginnend bei**, **Endet bei** und **Nach** bilden Gruppen wie 1-10, 11-20 usw.

**Gruppierung aufheben** entfernt die Gruppen und die dafür hinzugefügten Felder. Beim Aktualisieren werden die Gruppen aus den neuen Daten erneut gebildet. Wenn zwei Pivot-Tabellen aus denselben Daten erstellt wurden, können keine Gruppen gebildet werden, da eine Gruppe ein Teil der gemeinsam genutzten Daten ist.

## Berechnete Felder

Ein berechnetes Feld ist ein Feld, das Sie aus einer Formel erstellen. Es verwendet andere Felder der Liste. Ein Bonus von einem Zehntel des Betrags ist zum Beispiel `=Betrag*0.1`.

Wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Berechnetes Feld**. Geben Sie einen Namen und eine Formel ein. Schreiben Sie den Namen eines Felds in die Formel, oder doppelklicken Sie in der Liste **Felder** darauf, um ihn dort einzufügen. Setzen Sie einen Namen mit Leerzeichen in einfache Anführungszeichen, wie in `='Stückpreis'*Menge`. Sie können Zahlen, `+ - * / ^ %`, Vergleiche und einige Funktionen wie `IF`, `ROUND`, `SUM`, `MIN`, `MAX`, `AVERAGE` und `ABS` verwenden.

Wählen Sie **Hinzufügen**. Das neue Feld wird in den Bereich **Werte** gelegt. Es erscheint auch in der Feldliste, wo Sie es wie jedes andere Feld aktivieren oder deaktivieren können. Es kann nicht in Zeilen, Spalten oder Filter gelegt werden.

Ein berechnetes Feld wird in jeder Zelle des Berichts berechnet. Der Name eines Felds steht für die Summe dieses Felds in der Zelle; `=Betrag*Betrag` quadriert also die Summe der Zelle. Es quadriert nicht jede Zeile und addiert sie dann. Ein Gesamtergebnis verwendet die Summen der gesamten Liste.

**Formeln auflisten** (neben **Berechnetes Feld**) erstellt ein neues Blatt, das jedes berechnete Feld und seine Formel nennt. **Rückgängig** entfernt die Zellen, und ein weiteres **Rückgängig** entfernt das Blatt.

Um ein Feld zu ändern, wählen Sie seinen Namen im Feld **Name** aus. Seine Formel wird angezeigt. Ändern Sie sie, und wählen Sie **Ändern**. **Löschen** entfernt das Feld und seinen Wert. Jede Aktion ist ein Schritt von **Rückgängig**. Ein berechnetes Feld wird mit der Datei gespeichert und beim Aktualisieren des Berichts beibehalten. Zwei aus denselben Daten erstellte Pivot-Tabellen können kein solches Feld haben, da ein berechnetes Feld ein Teil der gemeinsam genutzten Daten ist.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert einer Pivot-Tabelle, oder wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Details anzeigen**. JeffCrunch erstellt ein neues Blatt, benannt Detail 1, 2 usw., mit den Zeilen der Liste, aus denen sich die Zahl ergibt. Die Namen der Felder stehen in der ersten Zeile. Mit F2 bearbeiten Sie die Zelle weiterhin, sofern sie bearbeitet werden kann. **Rückgängig** entfernt die Zeilen, und ein weiteres **Rückgängig** entfernt das Blatt.

## Erweitern und reduzieren

Ein Feld, dessen Elemente weitere Elemente unter sich haben, kann die Zeilen unter seinen Elementen anzeigen oder ausblenden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** wirken auf jedes Element des im Bereich ausgewählten Felds.

## Pivot-Tabellen-Optionen

**Optionen** auf der Registerkarte **Pivot-Tabelle** öffnet ein Feld mit den Einstellungen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn haben.
- **Für Fehlerwerte anzeigen** und **Für leere Zellen anzeigen** setzen einen eigenen Text in diese Zellen.
- **Zellen mit Beschriftungen verbinden und zentrieren**, **Spaltenbreiten beim Aktualisieren der Tabelle anpassen** und **Zellenformatierung beim Aktualisieren der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtergebnisse für Zeilen anzeigen** und **für Spalten** entsprechen den Optionen unter „Gesamtergebnisse“.
- **Daten beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Wie jede Aktualisierung ist es ein Schritt von „Rückgängig“.

## Formatvorlage: Kopfzeilen, Bänder und der Katalog

Die **Formatvorlagenoptionen** auf der Registerkarte **Pivot-Tabelle** schalten die Kopfzeile, die erste Spalte (Zeilenüberschriften) und die Bänder von Zeilen und Spalten ein oder aus. **Formatvorlagen** enthält einen Katalog mit Darstellungen, und **Format löschen** entfernt die Darstellung. Diese Darstellungen werden auf dieselbe Weise gezeichnet wie die von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** legen die Leerzeile und die wiederholten Beschriftungen für alle Felder der Zeilen gleichzeitig fest.

## Entwurf: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** zeigt oder verbirgt die Summe jeder Gruppe, am Anfang oder am Ende der Gruppe. Wenn in „Zeilen“ mehr als ein Feld steht, ist die Summe jedes äußeren Felds ein Teilergebnis.
- **Gesamtergebnisse** bietet vier Möglichkeiten: aus für Zeilen und Spalten, ein für beide, ein nur für Zeilen (eine Spalte mit Summen rechts, eine für jede Zeile) und ein nur für Spalten (eine Zeile mit Summen unten).
- **Berichtslayout** ist die Form der Beschriftungen. In der **kompakten** Form teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas nach rechts versetzt angezeigt. In der **Gliederungsform** hat jedes Feld eine eigene Spalte, und jede Gruppe hat über ihren Elementen eine eigene Zeile. In der **tabellarischen** Form hat jedes Feld eine eigene Spalte, der Name der Gruppe steht in ihrer ersten Zeile, und das Teilergebnis steht unter der Gruppe.

## Dieselbe Liste, ein Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und eine Zeile darüber zeigt **Monat** und **(Alle)**. Die Zahlen sind jetzt die Summe jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie auf eine Zelle in einer Pivot-Tabelle, und wählen Sie **Einfügen** und dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm der Werte der Tabelle. Die Gesamtergebnisse lässt es aus. Das Diagramm ist in der Datei als Diagramm der Pivot-Tabelle gekennzeichnet. Es liest die Zellen der Tabelle, seine Zahlen ändern sich also, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Wenn die Tabelle mehr Zeilen oder Spalten erhält, erstellen Sie das Diagramm neu, da das Diagramm die Zellen beibehält, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert, und eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie auf eine Zelle in einer Pivot-Tabelle, und klicken Sie dann auf der Registerkarte „Pivot-Tabelle“ auf **Datenschnitte**. Der Bereich „Datenschnitte“ wird geöffnet und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat für jedes Element seines Felds eine Schaltfläche. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut darauf, um alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt, und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt von „Rückgängig“, und „Rückgängig“ stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts bei.

**Berichtsverbindungen** gibt an, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den von ihr gewählten Datumsbereich. JeffCrunch zeigt die Datenschnitte in diesem Bereich an, nicht über den Zellen, und ändert eine Zeitachse noch nicht. Beide werden in der Datei so beibehalten, wie sie gespeichert wurden.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste bis zu fünf Millionen Zeilen und ein Bericht bis zu vier Millionen Zellen. Eine Datei, die darüber hinausgeht, wird so angezeigt, wie sie gespeichert wurde, und kann nicht geändert werden. Das erneute Lesen von hunderttausend Zeilen mit zwanzig Spalten und die Berechnung des Berichts dauerten auf dem Computer, auf dem dies gemessen wurde, etwa eine halbe Sekunde; ein langsamerer Computer benötigt länger.
