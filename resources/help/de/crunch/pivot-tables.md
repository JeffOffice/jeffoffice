# Pivot-Tabellen
<!-- keywords: anderer Datei, andere Arbeitsmappe, externe Quelle, Formeln auflisten, berechnetes Feld, berechnetes Element, Berechnungsreihenfolge, Formelfeld, Berichtsfilterseiten, Pivot-Tabelle, Details einblenden, Details ausblenden, Pivot-Tabelle einfügen, erstellen, machen, auswählen, Pivot-Tabelle verschieben, GETPIVOTDATA, GETPIVOTDATA generieren, aktualisieren, alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, kompakt, Gliederung, tabellarisch, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent der Gesamtsumme, Differenz, laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datumswerte, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen der Datei aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Details anzeigen, Details einblenden, Zeilen hinter einem Wert -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste hat zum Beispiel für jeden Verkauf eine Zeile; der Bericht addiert die Verkäufe nach Region und Monat. Sie schreiben keine Formeln. Sie legen fest, welche Spalten der Liste links untereinander stehen, welche oben quer stehen und welche addiert wird, und der Bericht füllt sich selbst aus.

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

Eine Pivot-Tabelle mit **Region** links, **Monat** oben und der Summe von **Betrag** in der Mitte sieht so aus.

| Summe von Betrag | Monat | | |
|---|---|---|---|
| Region | Jan | Feb | Gesamtergebnis |
| Ost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Gesamtergebnis | 22 | 27 | 49 |

Ost im Januar ergibt 15, weil die beiden Zeilen Ost/Jan mit 10 und 5 addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in die ein Feld gelegt werden kann. Ein Feld in „Zeilen“ ergibt für jeden seiner verschiedenen Einträge eine Zeile; ein Feld in „Spalten“ ergibt für jeden eine Spalte; ein Feld in „Werte“ wird addiert. Ein vierter Bereich, **Filter**, enthält ein Feld, mit dem Sie die Zeilen ausblenden, die Sie nicht auswählen.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei speichert, damit sie Summen anzeigen kann, ohne die Zellen zu lesen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen, und ändert sich deshalb nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Eine Pivot-Tabelle erstellen

1. Klicken Sie in eine Zelle der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste braucht oberhalb ihrer Daten eine Zeile mit Namen.
2. Wählen Sie die Registerkarte **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, wenn sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben. Um eine Liste aus einer anderen Datei zu verwenden, wählen Sie **Ein Bereich in einer anderen Datei**, wählen die Datei mit **Durchsuchen** aus und geben den Bereich ein oder den Namen einer Tabelle bzw. eines Namens, den Sie in dieser Datei angelegt haben, z. B. `Tabelle1!A1:D100`. JeffCrunch liest Arbeitsmappen (`.xlsx`, `.xlsm`, `.xls` und `.ods`) und `.csv`-Dateien.
4. Legen Sie fest, wohin die Pivot-Tabelle kommt: **Neues Blatt** (der Standard) oder eine Zelle auf diesem oder einem anderen Blatt. Wählen Sie eine Stelle, an der nichts steht, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle beginnt leer, und der Bereich **Pivot-Tabellen-Felder** wird geöffnet. Aktivieren Sie die Felder, die Sie möchten: Ein Feld mit Zahlen wird addiert, jedes andere wird zu einer Zeile. Verschieben Sie die Felder dann in die Bereiche, wie es die folgenden Abschnitte beschreiben.

Wenn die Daten in einer anderen Datei stehen, speichert die Pivot-Tabelle eine Kopie davon, sodass die Tabelle auch ohne diese Datei funktioniert und gespeichert werden kann. Siehe unten: Daten in einer anderen Datei.

Ein neues Blatt ist ein eigener Schritt von **Rückgängig**, ebenso die Tabelle. Wenn Sie als .xlsx speichern, wird die Tabelle so geschrieben, wie das ursprüngliche Programm sie schreibt, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle anzeigt, in ihren Zellen, nicht aber die Tabelle selbst; das Speichern weist Sie darauf hin.

## GETPIVOTDATA: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` liefert die Zahl, die eine Pivot-Tabelle für die angegebenen Werte und Zeilen- bzw. Spaltenbezeichnungen anzeigt. Für das Beispiel oben:

`=PIVOTDATENZUORDNEN("Summe von Betrag"; $E$3; "Region"; "Ost"; "Monat"; "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, so wie die Tabelle ihn anzeigt (z. B. Summe von Betrag). Auch der Name des Feldes, das addiert wird (Betrag), funktioniert.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Wenn Sie nur Region und Ost angeben, erhalten Sie die Summe für Ost, 35. Wenn Sie nichts angeben, erhalten Sie das Gesamtergebnis, 49. Die angegebenen Felder müssen die ersten Felder der Zeilen oder die ersten Felder der Spalten sein, und zwar in dieser Reihenfolge.
- Zeigt die Tabelle die Zahl nicht an, zeigt die Zelle `#BEZUG!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach dem Umordnen der Tabelle die richtige Zahl liest. Um dies auszuschalten, wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie daran nichts ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, schreibt die neuen Zahlen in die Zellen und speichert die Pivot-Tabelle so, dass das Programm, das sie erstellt hat, sie ohne Meldung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden und Ähnliches, die Datums- und Zahlenwerte gruppieren und die Werte als Anteil, als Differenz oder als laufende Summe anzeigen. Eine Pivot-Tabelle, die einen Wert als Rang oder als Anteil an einem übergeordneten Element anzeigt, von Hand erstellte Gruppen von Elementen enthält oder ein Datenmodell verwendet, wird so angezeigt und beibehalten, wie sie gespeichert wurde; die Befehle weisen darauf hin, dass sie sie nicht ändern können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** wird angezeigt.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wurde, und berechnet den Bericht neu. Neue Einträge kommen nach den bereits vorhandenen; Einträge, die nicht mehr da sind, werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle** macht dasselbe für alle Pivot-Tabellen der Arbeitsmappe.

Wenn der Bericht über Zellen wachsen würde, in denen etwas steht, aktualisiert JeffCrunch nicht. Es teilt Ihnen mit, welche Zellen im Weg sind. Leeren Sie diese Zellen oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie dann erneut.

**Rückgängig** macht eine Aktualisierung mit einem Schritt rückgängig.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich der ersten Zeile mit den Namen. Geben Sie sie wie `Tabelle1!A1:D100` ein oder geben Sie den Namen einer Tabelle oder eines Namens ein, den Sie angelegt haben. JeffCrunch liest sie und berechnet den Bericht neu. Hat ein Feld der Liste in den neuen Zellen denselben Namen, bleibt sein Platz im Bericht erhalten.

Sie können die Tabelle auch auf einen Bereich in einer anderen Datei richten. Wählen Sie im Feld **Ein Bereich in einer anderen Datei**, wählen Sie die Datei und geben Sie den Bereich darin ein. Mit **Ein Bereich in dieser Arbeitsmappe** zeigt die Tabelle wieder auf Zellen hier. Jede dieser Änderungen ist ein Schritt von **Rückgängig**.

## Daten in einer anderen Datei

Eine Pivot-Tabelle, die aus einer anderen Datei erstellt wurde, behält die Daten, aus denen sie entstanden ist, innerhalb dieser Arbeitsmappe. So können Sie die Arbeitsmappe öffnen, ändern und speichern, auch wenn die andere Datei nicht mehr vorhanden ist. **Aktualisieren** liest die andere Datei erneut. Wurde die Datei verschoben, fehlt sie, ist sie zu groß, ist sie keine Arbeitsmappe oder enthält sie den Bereich nicht mehr, behält die Tabelle die bisherigen Daten, und eine Meldung teilt mit, dass die Datei nicht gelesen werden konnte. Eine Datei, die sich auf einem anderen Computer im Netzwerk befindet, wird erst gelesen, wenn Sie sie selbst unter **Datenquelle ändern** auswählen. Eine Aktualisierung liest jede Datei einmal, und insgesamt höchstens 32 Dateien, 1 GB und 30 Sekunden; eine Tabelle, die darüber hinausgeht, behält ihre Daten.

Wenn eine Arbeitsmappe angibt, dass ihre Tabelle beim Öffnen erneut gelesen werden soll, und die Daten der Tabelle in einer anderen Datei stehen, fragt JeffCrunch zuerst nach, genauso wie bei Verknüpfungen zu anderen Arbeitsmappen. Solange Sie nicht zustimmen, wird keine Datei gelesen.

Eine als .xlsx gespeicherte Arbeitsmappe speichert, wo die Datei liegt. Wenn Sie die Datei im Feld auswählen, wird ihre vollständige Adresse gespeichert. Eine Arbeitsmappe, die ein anderes Programm erstellt hat und deren Tabelle mit einem kurzen Pfad wie `Preise.xlsx` auf eine Datei verweist, behält diesen Pfad, und JeffCrunch sucht die Datei im selben Ordner wie die Arbeitsmappe. Wenn Sie die Arbeitsmappe mit **Speichern unter** in einem anderen Ordner speichern, wird ein solcher Pfad so umgeschrieben, dass er weiterhin auf dieselbe Datei verweist. Beim Aktualisieren folgt JeffCrunch keinen Verknüpfungen, die die andere Datei selbst zu weiteren Dateien hat.

## Löschen

**Löschen** hat zwei Auswahlmöglichkeiten. **Filter löschen** entfernt alle Filter und zeigt alle Einträge wieder an. **Alles löschen** nimmt alle Felder aus dem Bericht heraus und lässt ihn leer, bereit zum Neuaufbau.

## Details einblenden und ausblenden

## Berichtsfilterseiten

Ein Filter steht über einer Pivot-Tabelle und lässt Sie jeweils ein Element betrachten, etwa einen Monat. **Berichtsfilterseiten anzeigen** auf der Registerkarte **Pivot-Tabelle** erstellt für jedes Element gleichzeitig eine Seite. Jede Seite ist ein neues Blatt, das nach dem Element benannt ist und seine eigene Kopie der Tabelle enthält. Der Filter jeder Kopie ist auf dieses Element eingestellt. Hat die Tabelle mehr als einen Filter, wählen Sie aus, welcher verwendet wird. Ein einziger Schritt **Rückgängig** entfernt alle Seiten.

Hat ein Bericht mehr als ein Feld in den Zeilen, lässt sich ein Element des ersten Feldes öffnen und schließen. Klicken Sie auf die Zelle mit dem Namen des Elements und wählen Sie **Details ausblenden**, um es zu schließen: Die Zeilen des nächsten Feldes darunter werden ausgeblendet, und nur seine Summe bleibt. **Details einblenden** öffnet es wieder. Ein Element des letzten Feldes hat nichts darunter. Jeder Schritt ist ein **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabelle** wählt einen Teil des Berichts aus: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen links; hat der Bericht links keine, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht mit seiner ersten Zelle dort neu. Die Zellen, die er verlässt, werden geleert. Formeln, die auf die Zellen des Berichts zeigen, folgen ihnen, wie beim Ausschneiden und Einfügen. Steht am neuen Ort etwas, verschiebt JeffCrunch den Bericht nicht und teilt Ihnen das mit. Ein Schritt **Rückgängig** verschiebt ihn zurück.

## Der Bereich mit den Feldern

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Pivot-Tabellen-Felder**. Er zeigt die Felder der Liste mit je einem Kontrollkästchen daneben und darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt nach **Werte** und wird addiert; jedes andere kommt nach **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche und wählen Sie **Verschieben nach**, um es in einen anderen Bereich zu legen. Die Befehle **Nach oben verschieben** und **Nach unten verschieben** ändern seine Reihenfolge innerhalb des Bereichs, und **Feld entfernen** nimmt es heraus.
- Bei mehr als einem Wert erscheint in **Spalten** eine Zeile mit dem Namen **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte links untereinander zu stellen.
- **Wertfeldeinstellungen** (ein Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er summiert wird: **Summe**, **Anzahl**, **Anzahl (Zahlen)**, **Mittelwert**, **Max**, **Min**, **Produkt**, **StdAbw** und **StdAbwN** (die Standardabweichung einer Stichprobe bzw. aller Werte) sowie **Var** und **VarianzN** (die Varianz derselben Werte). Außerdem legt sie fest, wie der Wert angezeigt wird, und sein Zahlenformat. Siehe den nächsten Abschnitt.
- **Feldeinstellungen** (ein Doppelklick auf ein Feld in Filter, Spalten oder Zeilen oder die Schaltfläche) legt ein einzelnes Feld fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt von **Rückgängig**.

## Werte anzeigen als, und das Zahlenformat eines Werts

In **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts anzeigt.

- **% der Gesamtsumme**: die Zelle geteilt durch die Summe des ganzen Berichts. Im Beispiel ist Ost im Januar 15 von 49, also 30,6 %.
- **% der Spaltensumme** und **% der Zeilensumme**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie die Zelle im Vergleich zu dem Wert steht, den ihre Zeile und Spalte bei gleichmäßiger Verteilung der Daten ergeben würden. Eine Zahl über 1 bedeutet mehr als der gleichmäßige Anteil.
- **Differenz von**, **% von** und **% Differenz von**: die Zelle im Vergleich zu einem anderen Element eines Feldes. Wählen Sie das **Basisfeld** (ein Feld in Zeilen oder Spalten) und das **Basiselement** (eines seiner Elemente oder das vorherige bzw. nächste). Wenn etwa Monat das Basisfeld und Jan das Basiselement ist, zeigt Februar, um wie viel er über oder unter Januar liegt. Das Basiselement selbst bleibt bei einer Differenz leer und ist bei **% von** 100 %.
- **Laufende Summe in**: Jede Zelle addiert die Zellen davor entlang des Basisfeldes.
- **Rang vom Kleinsten zum Größten** und **Rang vom Größten zum Kleinsten**: Jede Zelle zeigt ihren Platz, 1, 2, 3 usw., unter den Zellen des Basisfeldes in derselben Zeile oder Spalte. Rang 1 ist die kleinste bzw. die größte Zahl.
- **% der übergeordneten Zeilensumme** und **% der übergeordneten Spaltensumme**: Jede Zelle zeigt, welchen Anteil sie an ihrem übergeordneten Element hat. Das übergeordnete Element eines Elements ist das Element eine Ebene darüber, etwa das Jahr, das einen Monat enthält. Ein Element ohne übergeordnetes Element wird als Anteil am Gesamtergebnis angezeigt.

**Zahlenformat** nimmt einen Formatcode, etwa `0,00`, `#.##0` oder `0,0%`. Lassen Sie das Feld leer, um das Format der Zellen beizubehalten. Ein Wert, der als Anteil angezeigt wird, erhält `0,00%`, wenn Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** nimmt es zusammen mit den Zahlen zurück.

## Feldeinstellungen

Markieren Sie ein Feld im Bereich **Filter**, **Spalten** oder **Zeilen** und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feldeinstellungen** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld anzeigt.
- **Teilergebnisse**: **Automatisch** zeigt eine Summe für jede Gruppe des Feldes, **Keine** zeigt keine.
- **Elementbeschriftungen anzeigen in**: Kompaktform, Gliederungsform oder tabellarische Form, wie im Berichtslayout.
- **Nach jedem Element eine Leerzeile einfügen** setzt nach jeder Gruppe eine leere Zeile.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe (Gliederungs- und tabellarische Form).
- **Sortieren**: Die Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z bzw. von Z bis A sortieren. **Vom kleinsten zum größten nach einem Wert** und **Vom größten zum kleinsten nach einem Wert** sortieren die Elemente nach den Zahlen eines Wertfeldes. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filtern** blendet Elemente anhand ihrer Beschriftungen (beginnt mit, endet mit, enthält, ist gleich) oder anhand eines Werts (größer als, kleiner als, zwischen usw.) aus oder behält nur die ersten oder letzten wenigen Elemente nach einem Wert. Der Filter wird berechnet, wenn Sie OK wählen. Er blendet die Elemente aus, die er ausschließt; die Kontrollkästchen unten zeigen, welche das sind, und Sie können ein Element durch Aktivieren wieder einblenden.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Feldes mit je einem Kontrollkästchen. Deaktivieren Sie ein Kontrollkästchen, um das Element auszublenden; die Summen lassen es dann weg.

## Datums- und Zahlenwerte gruppieren

Markieren Sie ein Feld mit Datums- oder Zahlenwerten in **Zeilen** oder **Spalten** und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feld gruppieren**. Gruppen entstehen aus den Daten, deshalb dürfen in dieser Spalte nur Zahlen oder nur Datumswerte stehen, ohne Text und ohne leere Zellen.

- **Datumswerte**: Markieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste markierte Teil bleibt im Feld. Für jeden gröberen Teil fügt JeffCrunch ein Feld hinzu, das nach ihm benannt ist (z. B. **Jahre**), und setzt es an dieselbe Stelle vor das Datumsfeld. **Beginnend bei** und **Endend bei** sind das erste und das letzte Datum; lassen Sie sie unverändert, um das früheste und das späteste Datum der Daten zu verwenden. Datumswerte vor dem Beginn oder nach dem Ende landen in zwei zusätzlichen Gruppen, deren Namen mit `<` bzw. `>` beginnen.
- **Zahlen**: **Beginnend bei**, **Endend bei** und **Schrittweite** erzeugen Gruppen wie 1–10, 11–20 usw.

**Gruppierung aufheben** entfernt die Gruppen und die Felder, die für sie hinzugefügt wurden. Eine Aktualisierung erstellt die Gruppen anhand der neuen Daten neu. Werden zwei Pivot-Tabellen aus denselben Daten erstellt, lassen sich keine Gruppen bilden, weil eine Gruppe ein Teil der gemeinsamen Daten ist.

## Berechnete Felder

Ein berechnetes Feld ist ein Feld, das Sie aus einer Formel erstellen. Es verwendet andere Felder der Liste. Ein Bonus von einem Zehntel des Betrags ist zum Beispiel `=Betrag*0,1`.

Wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Berechnetes Feld**. Geben Sie einen Namen und eine Formel ein. Schreiben Sie den Namen eines Feldes in die Formel oder doppelklicken Sie ihn in der Liste **Felder**, um ihn dort einzufügen. Setzen Sie einen Namen mit Leerzeichen in einfache Anführungszeichen, wie in `='Stückpreis'*Menge`. Sie können Zahlen, `+ - * / ^ %`, Vergleiche und einige Funktionen wie `WENN`, `RUNDEN`, `SUMME`, `MIN`, `MAX`, `MITTELWERT` und `ABS` verwenden.

Wählen Sie **Hinzufügen**. Das neue Feld wird in den Bereich **Werte** gesetzt. Es erscheint auch in der Feldliste, wo Sie es wie jedes andere Feld aktivieren oder deaktivieren können. Es kann nicht in Zeilen, Spalten oder Filter gelegt werden.

Ein berechnetes Feld wird in jeder Zelle des Berichts berechnet. Der Name eines Feldes steht für die Summe dieses Feldes in der Zelle, sodass `=Betrag*Betrag` die Summe der Zelle quadriert. Es quadriert nicht jede Zeile einzeln und addiert dann. Ein Gesamtergebnis verwendet die Summen der gesamten Liste.

**Formeln auflisten** (neben **Berechnetes Feld**) erstellt ein neues Blatt, das jedes berechnete Feld mit seiner Formel auflistet sowie jedes berechnete Element mit seiner Formel, seinem Feld und seiner Berechnungsreihenfolge. **Rückgängig** entfernt die Zellen, ein weiteres **Rückgängig** entfernt das Blatt.

Um ein Feld zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Feld und seinen Wert. Jeder Schritt ist ein Schritt von **Rückgängig**. Ein berechnetes Feld wird mit der Datei gespeichert und bleibt beim Aktualisieren des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können kein gemeinsames berechnetes Feld haben, weil es ein Teil der gemeinsamen Daten ist.

## Berechnete Elemente

Ein berechnetes Element ist ein Element, das Sie aus anderen Elementen desselben Feldes erstellen. Das Feld Region hat zum Beispiel die Elemente Ost und West. Ein neues Element mit dem Namen Beide kann `=Ost+West` sein.

Klicken Sie auf eine Zelle der Pivot-Tabelle und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Berechnetes Element**. Wählen Sie das **Feld**. Es muss in Zeilen oder Spalten stehen. Geben Sie einen **Namen** und eine **Formel** ein. Schreiben Sie den Namen eines Elements in die Formel oder doppelklicken Sie ihn in der Liste **Elemente**, um ihn dort einzufügen. Setzen Sie einen Namen mit Leerzeichen in einfache Anführungszeichen, wie in `='Großer Ost'+West`. Sie können Zahlen, `+ - * / ^ %` und Klammern verwenden.

Das neue Element erhält eine Zeile im Bericht. In jeder Spalte zeigt es die Formel, berechnet aus den Zeilen der Elemente, die es nennt. Das neue Element wird in die Summen eingerechnet. Eine Summe, etwa ein Gesamtergebnis oder ein Teilergebnis, addiert die Zeile des neuen Elements zu den Zeilen der anderen Elemente. Bei Ost mit 35, West mit 14 und Beide (Ost plus West) mit 49 ist das Gesamtergebnis also 98. Nennt die Formel ein Element, das ausgeblendet ist, zeigt die Zelle **#BEZUG!**. Nennt die Formel ein Element, das nicht existiert, zeigt die Zelle **#NAME?**.

Um ein Element zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Element und seine Zeile. Jeder Schritt ist ein Schritt von **Rückgängig**. Ein berechnetes Element wird mit der Datei gespeichert und bleibt beim Aktualisieren des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können kein gemeinsames berechnetes Element haben, weil es ein Teil der gemeinsamen Daten ist.

### Berechnungsreihenfolge

Manchmal hat eine Zelle zwei berechnete Elemente: eines in ihrer Zeile und eines in ihrer Spalte. Die Reihenfolge, in der sie berechnet werden, kann das Ergebnis verändern. Angenommen, das Zeilenelement ist `=Ost-West` und das Spaltenelement ist `=Jan+1`. In einer Reihenfolge ergibt die Zelle (Ost − West) + 1. In der anderen ergibt sie (Ost + 1) − (West + 1), eine andere Zahl.

**Berechnungsreihenfolge** auf der Registerkarte **Pivot-Tabelle** listet die berechneten Elemente in der Reihenfolge auf, in der sie berechnet werden. Das Element weiter unten in der Liste wird zuletzt berechnet. Wählen Sie ein Element und klicken Sie auf **Nach oben verschieben** oder **Nach unten verschieben**. Die Reihenfolge wird als Reihenfolge der Elemente mit der Datei gespeichert, und andere Programme lesen sie genauso. **Formeln auflisten** zeigt den Platz jedes Elements darin.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert der Pivot-Tabelle oder wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Details anzeigen**. JeffCrunch erstellt ein neues Blatt mit dem Namen Detail 1, Detail 2 usw. mit den Zeilen der Liste, aus denen die Zahl besteht. Die Namen der Felder stehen in der ersten Zeile. F2 bearbeitet die Zelle weiterhin, sofern sie bearbeitet werden kann. **Rückgängig** entfernt die Zeilen, ein weiteres **Rückgängig** entfernt das Blatt.

## Erweitern und reduzieren

Ein Feld, dessen Elemente weitere Elemente darunter haben, kann die Zeilen unter seinen Elementen ein- oder ausblenden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** tun dies für jedes Element des Feldes, das Sie im Bereich ausgewählt haben.

## Optionen der Pivot-Tabelle

**Optionen** auf der Registerkarte **Pivot-Tabelle** öffnet ein Feld mit den Einstellungen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn haben.
- **Für Fehlerwerte anzeigen** und **Für leere Zellen anzeigen** setzen einen eigenen Text in diese Zellen.
- **Zellen mit Beschriftungen verbinden und zentrieren**, **Spaltenbreiten beim Aktualisieren der Tabelle anpassen** und **Formatierung der Zellen beim Aktualisieren der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtergebnisse für Zeilen anzeigen** und **für Spalten anzeigen** entsprechen den Optionen für Gesamtergebnisse.
- **Datenquelle beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Es ist ein Schritt von Rückgängig, wie jede Aktualisierung.

## Format: Kopfzeilen, Bänder und Katalog

Die **Formatoptionen** auf der Registerkarte **Pivot-Tabelle** schalten die Kopfzeile, die erste Spalte (Zeilenköpfe) sowie die Bänder der Zeilen und der Spalten ein oder aus. **Formatvorlagen** enthält einen Katalog von Designs, und **Format löschen** entfernt das Design. Diese Designs werden genauso gezeichnet wie die Designs von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** legen die Leerzeile und die wiederholten Beschriftungen für alle Felder der Zeilen gleichzeitig fest.

## Gestaltung: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** blendet die Summe jeder Gruppe ein oder aus, oben oder unten in der Gruppe. Stehen mehr als ein Feld in Zeilen, ist die Summe jedes äußeren Feldes ein Teilergebnis.
- **Gesamtergebnisse** hat vier Auswahlmöglichkeiten: für Zeilen und Spalten aus, für beide ein, nur für Zeilen ein (eine Spalte mit Summen rechts, eine für jede Zeile) und nur für Spalten ein (eine Zeile mit Summen unten).
- **Berichtslayout** bestimmt die Form der Beschriftungen. In **Kompaktform** teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas weiter rechts angezeigt. In **Gliederungsform** hat jedes Feld seine eigene Spalte, und jede Gruppe hat eine eigene Zeile über ihren Elementen. In **Tabellarischer Form** hat jedes Feld seine eigene Spalte, der Name der Gruppe steht in ihrer ersten Zeile, und das Teilergebnis steht unter der Gruppe.

## Dieselbe Liste, ein Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und über ihm erscheint eine Zeile mit **Monat** und **(Alle)**. Die Zahlen sind jetzt die Summe jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie auf eine Zelle einer Pivot-Tabelle und wählen Sie **Einfügen**, dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm der Werte der Tabelle. Die Gesamtergebnisse bleiben weg. Das Diagramm wird in der Datei als Diagramm der Pivot-Tabelle gekennzeichnet. Es liest die Zellen der Tabelle, daher ändern sich seine Zahlen, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Kommen zur Tabelle weitere Zeilen oder Spalten hinzu, erstellen Sie das Diagramm neu, weil es die Zellen behält, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert; eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie auf eine Zelle einer Pivot-Tabelle und dann auf der Registerkarte Pivot-Tabelle auf **Datenschnitte**. Der Bereich Datenschnitte wird geöffnet und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat eine Schaltfläche für jedes Element seines Feldes. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut darauf, um alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt von Rückgängig, und Rückgängig stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts.

**Berichtsverbindungen** zeigt, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den Zeitraum, den Sie gewählt haben. JeffCrunch zeigt die Datenschnitte in diesem Bereich, nicht über den Zellen, und ändert eine Zeitachse noch nicht. Beide werden so in der Datei behalten, wie sie gespeichert wurden.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste mit bis zu fünf Millionen Zeilen und einen Bericht mit bis zu vier Millionen Zellen. Eine Datei, die diese Grenzen überschreitet, wird so angezeigt, wie sie gespeichert wurde, und kann nicht geändert werden. Das erneute Lesen von hunderttausend Zeilen mit zwanzig Spalten und das Berechnen des Berichts dauerten auf dem Computer, auf dem dies gemessen wurde, etwa eine halbe Sekunde; ein langsamerer Computer braucht länger.
