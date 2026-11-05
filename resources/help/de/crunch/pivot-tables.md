# Pivot-Tabellen
<!-- keywords: Berichtsfilterseiten, Pivot-Tabelle, Details einblenden, Details ausblenden, Pivot-Tabelle einfügen, erstellen, auswählen, Pivot-Tabelle verschieben, GETPIVOTDATA, PIVOTDATENZUORDNEN, aktualisieren, alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, Kompaktform, Gliederungsform, Tabellenform, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent des Gesamtergebnisses, Differenz, laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datum, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen der Datei aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Details anzeigen, Zeilen hinter einem Wert -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste enthält beispielsweise für jeden Verkauf eine Zeile; der Bericht addiert die Verkäufe nach Region und Monat. Sie müssen keine Formeln schreiben. Sie legen fest, welche Spalten der Liste an der Seite stehen, welche oben und welche addiert wird, und der Bericht füllt sich von selbst.

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

Ost im Januar ergibt 15, weil die beiden Ost-Zeilen für Januar, 10 und 5, addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die hier verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind die Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in die ein Feld gelegt werden kann. Ein Feld in „Zeilen“ ergibt eine Zeile für jeden seiner verschiedenen Einträge; ein Feld in „Spalten“ ergibt jeweils eine Spalte; ein Feld in „Werte“ ist das Feld, das addiert wird. Ein vierter Bereich, **Filter**, enthält ein Feld, das die nicht gewählten Zeilen ausschließt.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei aufbewahrt, damit sie Summen anzeigen kann, ohne die Zellen anzusehen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen; er ändert sich also nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Pivot-Tabelle erstellen

1. Klicken Sie in eine Zelle der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste benötigt über den Daten eine Zeile mit Namen.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, falls sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben.
4. Legen Sie fest, wohin die Pivot-Tabelle kommt: **Ein neues Blatt** (Standard) oder eine Zelle in diesem oder einem anderen Blatt. Wählen Sie eine Stelle, an der sonst nichts steht, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle ist zunächst leer, und der Bereich **Pivot-Tabellen-Felder** wird geöffnet. Aktivieren Sie die gewünschten Felder: Ein Feld mit Zahlen wird addiert, jedes andere Feld wird zu einer Zeile. Verschieben Sie die Felder anschließend wie in den folgenden Abschnitten beschrieben in die Bereiche.

Ein neues Blatt ist ein eigener Schritt von „Rückgängig“, ebenso die Tabelle. Beim Speichern als .xlsx wird die Tabelle so geschrieben, wie es das ursprüngliche Programm tut, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle anzeigt, in ihren Zellen, aber nicht die Tabelle selbst; das Speichern weist Sie darauf hin.

## GETPIVOTDATA: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` gibt die Zahl zurück, die eine Pivot-Tabelle für die angegebenen Werte sowie Zeilen und Spalten anzeigt. Für das obige Beispiel:

`=GETPIVOTDATA("Summe von Betrag", $E$3, "Region", "Ost", "Monat", "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, wie ihn die Tabelle anzeigt (zum Beispiel Summe von Betrag). Der Name des Felds, das addiert wird (Betrag), funktioniert ebenfalls.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Geben Sie nur Region, Ost an, erhalten Sie die Summe für Ost, 35. Geben Sie nichts an, erhalten Sie das Gesamtergebnis, 49. Die angegebenen Felder müssen in der Reihenfolge die ersten Felder der Zeilen oder die ersten Felder der Spalten sein.
- Zeigt die Tabelle die Zahl nicht an, steht in der Zelle `#REF!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach einer Umordnung der Tabelle noch die richtige Zahl liest. Um dies auszuschalten, wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie nichts daran ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, trägt die neuen Zahlen in die Zellen ein und schreibt die Pivot-Tabelle so zurück, dass das Programm, das sie erstellt hat, sie ohne Beanstandung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden usw., die Datumswerte und Zahlen gruppieren und die Werte als Anteil, Differenz oder laufende Summe anzeigen. Eine Pivot-Tabelle mit einem berechneten Feld oder Element, mit einem als Rang oder als Anteil eines übergeordneten Elements angezeigten Wert, mit von Hand erstellten Gruppen von Elementen oder mit einem Datenmodell wird so angezeigt und beibehalten, wie sie gespeichert wurde, und die Befehle melden, dass sie nicht geändert werden kann.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** wird angezeigt.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wurde, und berechnet den Bericht neu. Neue Einträge werden nach den vorhandenen angefügt; Einträge, die nicht mehr vorhanden sind, entfallen. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle** macht dasselbe für jede Pivot-Tabelle der Arbeitsmappe.

Würde der Bericht über Zellen wachsen, die Inhalt haben, aktualisiert JeffCrunch nicht, sondern teilt Ihnen mit, welche Zellen im Weg sind. Löschen Sie sie, oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie erneut.

**Rückgängig** macht eine Aktualisierung in einem Schritt rückgängig.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich ihrer ersten Zeile mit den Namen. Geben Sie sie wie `Sheet1!A1:D100` ein, oder geben Sie den Namen einer Tabelle oder eines von Ihnen erstellten Namens ein. JeffCrunch liest sie und berechnet den Bericht neu. Hat ein Feld der Liste in den neuen Zellen denselben Namen, bleibt seine Position im Bericht erhalten.

## Löschen

**Löschen** bietet zwei Optionen. **Filter löschen** entfernt alle Filter und zeigt wieder alle Einträge an. **Alle löschen** entfernt jedes Feld aus dem Bericht und lässt ihn leer, bereit für einen neuen Aufbau.

## Details einblenden und ausblenden

## Berichtsfilterseiten

Ein Filter steht über einer Pivot-Tabelle und ermöglicht es, jeweils ein Element zu betrachten, etwa einen Monat. **Berichtsfilterseiten anzeigen** auf der Registerkarte **Pivot-Tabelle** erstellt für jedes Element auf einmal eine Seite. Jede Seite ist ein neues Blatt, nach dem Element benannt, mit einer eigenen Kopie der Tabelle. Der Filter jeder Kopie ist auf dieses Element festgelegt. Hat die Tabelle mehr als einen Filter, wählen Sie den aus, der verwendet werden soll. Mit einmal **Rückgängig** verschwinden alle Seiten.

Wenn ein Bericht mehr als ein Feld in den Zeilen hat, kann ein Element des ersten Felds geöffnet oder geschlossen werden. Klicken Sie auf die Zelle mit dem Namen des Elements, und wählen Sie **Details ausblenden**, um es zu schließen: Die Zeilen des nächsten Felds darunter werden ausgeblendet, und nur seine Summe bleibt. **Details einblenden** öffnet es wieder. Ein Element des letzten Felds hat darunter nichts. Jede Aktion ist ein Schritt von **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabelle** wählt einen Teil des Berichts aus: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen an der Seite; hat der Bericht an der Seite keine, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht neu, wobei seine erste Zelle dort liegt. Die verlassenen Zellen werden geleert. Formeln, die auf Zellen des Berichts verweisen, folgen ihnen, wie beim Ausschneiden und Einfügen. Steht am neuen Ort etwas, verschiebt JeffCrunch den Bericht nicht und teilt Ihnen dies mit. Mit einmal **Rückgängig** wird er zurückverschoben.

## Der Feldbereich

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Pivot-Tabellen-Felder**. Er listet die Felder der Liste mit einem Kontrollkästchen daneben auf und darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt in **Werte** und wird addiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche, und wählen Sie **Verschieben nach**, um es in einen anderen zu legen. **Nach oben** und **Nach unten** ändern seine Reihenfolge innerhalb seines Bereichs, und **Feld entfernen** entfernt es.
- Bei mehr als einem Wert erscheint in **Spalten** eine Zeile namens **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte an der Seite anzuzeigen.
- **Wertfeldeinstellungen** (Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er summiert wird: **Summe**, **Anzahl**, **Anzahl Zahlen**, **Mittelwert**, **Max**, **Min**, **Produkt**, **Standardabw.** und **Standardabw. (Grundgesamtheit)** (die Standardabweichung einer Stichprobe bzw. aller Werte) sowie **Varianz** und **Varianz (Grundgesamtheit)** (die Varianz derselben). Außerdem legt es fest, wie der Wert angezeigt wird, und sein Zahlenformat. Siehe den nächsten Abschnitt.
- **Feldeinstellungen** (Doppelklick auf ein Feld in „Filter“, „Spalten“ oder „Zeilen“ oder die Schaltfläche) legt ein einzelnes Feld fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt von **Rückgängig**.

## Werte anzeigen als und das Zahlenformat eines Werts

In den **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts anzeigt.

- **% des Gesamtergebnisses**: die Zelle geteilt durch die Summe des gesamten Berichts. Im Beispiel sind Ost im Januar 15 von 49, also 30,6 %.
- **% des Spaltenergebnisses** und **% des Zeilenergebnisses**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie sich die Zelle zu dem verhält, was ihre Zeile und Spalte bei gleichmäßiger Verteilung der Daten ergeben würden. Eine Zahl über 1 bedeutet mehr als den gleichmäßigen Anteil.
- **Differenz von**, **% von** und **% Differenz von**: die Zelle im Vergleich zu einem anderen Element eines Felds. Wählen Sie das **Basisfeld** (ein Feld in Zeilen oder Spalten) und das **Basiselement** (eines seiner Elemente oder das vorherige bzw. nächste). Wenn Sie zum Beispiel Monat als Basisfeld und Jan als Basiselement wählen, zeigt der Februar, wie viel mehr oder weniger er als der Januar beträgt. Das Basiselement selbst bleibt bei einer Differenz leer und ist bei **% von** 100 %.
- **Laufende Summe in**: Jede Zelle addiert die vorangehenden Zellen entlang des Basisfelds.
- **Rang vom Kleinsten zum Größten** und **Rang vom Größten zum Kleinsten**: Jede Zelle zeigt ihren Platz, 1, 2, 3 usw., unter den Zellen des Basisfelds in derselben Zeile oder Spalte. Rang 1 ist die kleinste bzw. die größte Zahl.
- **% des übergeordneten Zeilenergebnisses** und **% des übergeordneten Spaltenergebnisses**: Jede Zelle zeigt, welchen Anteil sie an ihrem übergeordneten Element hat. Das übergeordnete Element eines Elements ist das Element eine Ebene darüber, etwa das Jahr, das einen Monat enthält. Ein Element ohne übergeordnetes Element wird als Anteil am Gesamtergebnis angezeigt.

**Zahlenformat** nimmt einen Formatcode entgegen, etwa `0.00`, `#,##0` oder `0.0%`. Lassen Sie das Feld leer, um das Format der Zellen beizubehalten. Ein als Anteil angezeigter Wert erhält `0.00%`, sofern Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** nimmt es zusammen mit den Zahlen zurück.

## Feldeinstellungen

Wählen Sie ein Feld im Feld **Filter**, **Spalten** oder **Zeilen** aus, und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feldeinstellungen** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld anzeigt.
- **Teilergebnisse**: **Automatisch** zeigt für jede Gruppe des Felds eine Summe an, **Keine** nicht.
- **Elementbeschriftungen anzeigen in**: Kompakt-, Gliederungs- oder Tabellenform, wie bei „Berichtslayout“.
- **Nach jedem Element eine leere Zeile einfügen** setzt nach jeder Gruppe eine leere Zeile.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe an (Gliederungs- und Tabellenform).
- **Sortieren**: die vorhandene Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z oder von Z bis A sortieren. **Kleinster bis größter nach einem Wert** und **Größter bis kleinster nach einem Wert** sortieren die Elemente nach den Zahlen eines Wertfelds. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filter** schließt Elemente anhand ihrer Beschriftungen (beginnt mit, endet mit, enthält, ist gleich) oder anhand eines Werts (größer als, kleiner als, zwischen usw.) aus oder behält nur die obersten oder untersten Elemente nach einem Wert bei. Der Filter wird beim Wählen von OK berechnet. Er blendet die ausgeschlossenen Elemente aus; die Kontrollkästchen darunter zeigen, welche, und Sie können ein Element aktivieren, um es wieder anzuzeigen.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Felds mit je einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um das Element auszublenden; die Summen schließen es dann aus.

## Datumswerte und Zahlen gruppieren

Wählen Sie ein Feld mit Datumswerten oder Zahlen in **Zeilen** oder **Spalten** aus, und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feld gruppieren**. Gruppen werden aus den Daten gebildet; die Daten dürfen in dieser Spalte also nur Zahlen oder Datumswerte enthalten, keinen Text und keine leeren Zellen.

- **Datumswerte**: Aktivieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste aktivierte Teil bleibt am Feld. Für jeden gröberen Teil fügt JeffCrunch ein nach ihm benanntes Feld hinzu (zum Beispiel **Jahre**) und setzt es im selben Bereich vor das Datumsfeld. **Beginnend bei** und **Endet bei** sind das erste und das letzte Datum; lassen Sie sie unverändert, um das kleinste und das größte Datum der Daten zu verwenden. Datumswerte vor dem Beginn oder nach dem Ende kommen in zwei zusätzliche Gruppen, die mit `<` und `>` beginnen.
- **Zahlen**: **Beginnend bei**, **Endet bei** und **Nach** bilden Gruppen wie 1-10, 11-20 usw.

**Gruppierung aufheben** entfernt die Gruppen und die dafür hinzugefügten Felder. Beim Aktualisieren werden die Gruppen aus den neuen Daten erneut gebildet. Wenn zwei Pivot-Tabellen aus denselben Daten erstellt wurden, können keine Gruppen gebildet werden, da eine Gruppe ein Teil der von ihnen gemeinsam genutzten Daten ist.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert einer Pivot-Tabelle, oder wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Details anzeigen**. JeffCrunch erstellt ein neues Blatt namens Detail 1, 2 usw. mit den Zeilen der Liste, aus denen sich die Zahl zusammensetzt. Die Namen der Felder stehen in der ersten Zeile. Mit F2 bearbeiten Sie die Zelle weiterhin, sofern sie bearbeitet werden kann. **Rückgängig** entfernt die Zeilen, ein weiteres **Rückgängig** das Blatt.

## Erweitern und reduzieren

Bei einem Feld, dessen Elemente weitere Elemente unter sich haben, können die Zeilen unter den Elementen angezeigt oder ausgeblendet werden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** tun dies für jedes Element des im Bereich ausgewählten Felds.

## Pivot-Tabellen-Optionen

**Optionen** auf der Registerkarte **Pivot-Tabelle** öffnet ein Dialogfeld mit den Optionen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn haben.
- **Bei Fehlerwerten anzeigen** und **Bei leeren Zellen anzeigen** tragen einen eigenen Text in diese Zellen ein.
- **Zellen mit Etiketten verbinden und zentrieren**, **Spaltenbreiten bei Aktualisierung der Tabelle an Inhalt anpassen** und **Zellenformatierung bei Aktualisierung der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtsummen für Zeilen anzeigen** und **für Spalten** entsprechen den Optionen unter „Gesamtergebnisse“.
- **Daten beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Es ist wie jede Aktualisierung ein Schritt von „Rückgängig“.

## Formatvorlage: Kopfzeilen, Bänder und der Katalog

Die **Formatvorlagenoptionen** auf der Registerkarte **Pivot-Tabelle** schalten die Kopfzeile, die erste Spalte (Zeilenüberschriften) und die Bänder von Zeilen und Spalten ein oder aus. **Formatvorlagen** bietet einen Katalog mit Darstellungen, und **Format löschen** entfernt die Darstellung. Diese Darstellungen werden auf dieselbe Weise gezeichnet wie die Darstellungen von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** legen die leere Zeile und die wiederholten Beschriftungen für alle Felder der Zeilen auf einmal fest.

## Entwurf: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** zeigt die Summe jeder Gruppe an oder blendet sie aus, oben oder unten in der Gruppe. Gibt es mehr als ein Feld in „Zeilen“, ist die Summe jedes äußeren Felds ein Teilergebnis.
- **Gesamtergebnisse** bietet vier Optionen: aus für Zeilen und Spalten, ein für beide, ein nur für Zeilen (rechts eine Spalte mit Summen, eine für jede Zeile) und ein nur für Spalten (unten eine Zeile mit Summen).
- **Berichtslayout** bestimmt die Form der Beschriftungen. In der **Kompaktform** teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas nach rechts versetzt angezeigt. In der **Gliederungsform** hat jedes Feld eine eigene Spalte, und jede Gruppe hat über ihren Elementen eine eigene Zeile. In der **Tabellenform** hat jedes Feld eine eigene Spalte, und der Name der Gruppe steht in ihrer ersten Zeile, mit dem Teilergebnis unter der Gruppe.

## Dieselbe Liste, ein Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und eine Zeile darüber zeigt **Monat** und **(Alle)**. Die Zahlen sind nun die Summe jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie auf eine Zelle in einer Pivot-Tabelle, und wählen Sie **Einfügen** und dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm der Werte der Tabelle. Die Gesamtergebnisse lässt es weg. Das Diagramm wird in der Datei als Diagramm der Pivot-Tabelle gekennzeichnet. Es liest die Zellen der Tabelle, seine Zahlen ändern sich also, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Erhält die Tabelle mehr Zeilen oder Spalten, erstellen Sie das Diagramm neu, da das Diagramm die Zellen behält, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert, und eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie auf eine Zelle in einer Pivot-Tabelle, und klicken Sie dann auf der Registerkarte „Pivot-Tabelle“ auf **Datenschnitte**. Der Bereich „Datenschnitte“ wird geöffnet und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat für jedes Element seines Felds eine Schaltfläche. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut darauf, um alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt, und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt von „Rückgängig“, und „Rückgängig“ stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts.

**Berichtsverbindungen** gibt an, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den von ihr gewählten Datumsbereich an. JeffCrunch zeigt die Datenschnitte in diesem Bereich an, nicht über den Zellen, und ändert eine Zeitachse noch nicht. Beide werden in der Datei so beibehalten, wie sie gespeichert wurden.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste bis zu fünf Millionen Zeilen und ein Bericht bis zu vier Millionen Zellen. Eine Datei, die darüber hinausgeht, wird so angezeigt, wie sie gespeichert wurde, und kann nicht geändert werden. Das erneute Lesen von hunderttausend Zeilen mit zwanzig Spalten und die Berechnung des Berichts dauerten auf dem Computer, auf dem dies gemessen wurde, etwa eine halbe Sekunde; ein langsamerer Computer benötigt länger.
