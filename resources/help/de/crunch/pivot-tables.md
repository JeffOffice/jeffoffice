# Pivot-Tabellen

<!-- keywords: andere Datei, andere Arbeitsmappe, externe Quelle, Formeln auflisten, berechnetes Feld, berechnetes Element, Berechnungsreihenfolge, Formelfeld, Berichtsfilterseiten, Pivot-Tabelle, Details einblenden, Details ausblenden, Pivot-Tabelle einfügen, erstellen, machen, auswählen, Pivot-Tabelle verschieben, GETPIVOTDATA, GETPIVOTDATENZUORDNEN, aktualisieren, alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, kompakt, Gliederung, tabellarisch, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent der Gesamtsumme, Differenz, laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datumswerte, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen der Datei aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Details anzeigen, Details einblenden, Zeilen hinter einem Wert -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste hat zum Beispiel für jeden Verkauf eine Zeile; der Bericht addiert die Verkäufe nach Region und Monat. Sie schreiben dafür keine Formeln. Sie legen fest, welche Spalten der Liste links untereinander stehen, welche oben quer stehen und welche addiert wird, und der Bericht füllt sich selbst aus.

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

Ost im Januar ergibt 15, weil die beiden Zeilen von Ost für Januar, 10 und 5, addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in die ein Feld gelegt werden kann. Ein Feld in „Zeilen“ ergibt für jeden seiner verschiedenen Einträge eine Zeile; ein Feld in „Spalten“ ergibt für jeden eine Spalte; ein Feld in „Werte“ ist das, was addiert wird. Ein vierter Bereich, **Filter**, enthält ein Feld, mit dem Sie die Zeilen auswählen, die nicht angezeigt werden sollen.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei aufbewahrt, damit sie Summen anzeigen kann, ohne auf die Zellen zu schauen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen, und ändert sich daher nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Eine Pivot-Tabelle erstellen

1. Klicken Sie auf eine Zelle der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste braucht eine Zeile mit Namen über ihren Daten.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, wenn sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben. Um eine Liste aus einer anderen Datei zu verwenden, wählen Sie **Ein Bereich in einer anderen Datei**, suchen die Datei mit **Durchsuchen** aus und geben den Bereich ein, oder den Namen einer Tabelle oder eines Namens, den Sie in dieser Datei angelegt haben, zum Beispiel `Tabelle1!A1:D100`. JeffCrunch liest Arbeitsmappen (`.xlsx`, `.xlsm`, `.xls` und `.ods`) und `.csv`-Dateien.
4. Legen Sie fest, wo die Pivot-Tabelle erscheinen soll: **Neues Blatt** (Standard) oder eine Zelle auf diesem oder einem anderen Blatt. Wählen Sie einen Platz, an dem nichts steht, denn die leere Tabelle belegt etwa 18 Zeilen und 3 Spalten.
5. Wählen Sie **OK**. Die Tabelle beginnt leer, und der Bereich **Felder der Pivot-Tabelle** wird geöffnet. Aktivieren Sie die Felder, die Sie möchten: Ein Feld mit Zahlen wird addiert, jedes andere wird zu einer Zeile. Verschieben Sie dann die Felder in die Bereiche, wie in den folgenden Abschnitten beschrieben.

Wenn sich die Daten in einer anderen Datei befinden, speichert die Pivot-Tabelle eine Kopie davon, sodass die Tabelle auch ohne diese Datei funktioniert und gespeichert werden kann. Siehe Daten in einer anderen Datei weiter unten.

Ein neues Blatt ist ein eigener Schritt von **Rückgängig**, ebenso die Tabelle. Wenn Sie als .xlsx speichern, wird die Tabelle so geschrieben, wie das Originalprogramm sie schreibt, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle anzeigt, in ihren Zellen, aber nicht die Tabelle selbst; beim Speichern wird darauf hingewiesen.

## GETPIVOTDATENZUORDNEN: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATENZUORDNEN` liefert die Zahl, die eine Pivot-Tabelle für die angegebenen Werte sowie Zeilen und Spalten anzeigt. Für das Beispiel oben:

`=GETPIVOTDATENZUORDNEN("Summe von Betrag"; $E$3; "Region"; "Ost"; "Monat"; "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, so wie die Tabelle ihn anzeigt (zum Beispiel Summe von Betrag). Auch der Name des Feldes, das addiert wird (Betrag), funktioniert.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Nennen Sie nur Region und Ost, erhalten Sie die Summe von Ost, 35. Nennen Sie nichts, erhalten Sie das Gesamtergebnis, 49. Die Felder, die Sie nennen, müssen in dieser Reihenfolge die ersten Felder in den Zeilen oder die ersten Felder in den Spalten sein.
- Zeigt die Tabelle die Zahl nicht an, zeigt die Zelle `#BEZUG!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach dem Umordnen der Tabelle die richtige Zahl liest. Um das abzuschalten, wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATENZUORDNEN.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie nichts daran ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, schreibt die neuen Zahlen in die Zellen und speichert die Pivot-Tabelle so, dass das Programm, das sie erstellt hat, sie ohne Fehlermeldung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden und Ähnliches, die Datums- und Zahlenwerte gruppieren und die Werte als Anteil, als Differenz oder als laufende Summe anzeigen. Eine Pivot-Tabelle, die einen Wert als Rang oder als Anteil an einem übergeordneten Element anzeigt, von Hand erstellte Gruppen von Elementen enthält oder ein Datenmodell verwendet, wird so angezeigt und beibehalten, wie sie gespeichert wurde; die Befehle weisen darauf hin, dass sie nicht geändert werden können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** erscheint.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wurde, und berechnet den Bericht neu. Neue Einträge kommen hinter die bereits vorhandenen; Einträge, die nicht mehr existieren, werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren**, auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle**, tut dasselbe für alle Pivot-Tabellen der Arbeitsmappe.

Würde der Bericht über Zellen mit Inhalt wachsen, aktualisiert JeffCrunch nicht. Es teilt Ihnen mit, welche Zellen im Weg sind. Leeren Sie diese Zellen oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie dann erneut.

**Rückgängig** macht eine Aktualisierung mit einem Schritt rückgängig.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich ihrer ersten Zeile mit Namen. Geben Sie sie ein, etwa `Tabelle1!A1:D100`, oder den Namen einer Tabelle oder eines Namens, den Sie angelegt haben. JeffCrunch liest sie und berechnet den Bericht neu. Hat ein Feld der Liste in den neuen Zellen denselben Namen, bleibt seine Position im Bericht erhalten.

Sie können die Tabelle auch auf einen Bereich in einer anderen Datei richten. Wählen Sie im Feld **Ein Bereich in einer anderen Datei**, wählen Sie die Datei aus und geben Sie den Bereich darin ein. Mit **Ein Bereich in dieser Arbeitsmappe** zeigt die Tabelle wieder auf Zellen hier. Jede dieser Änderungen ist ein Schritt von **Rückgängig**.

## Daten in einer anderen Datei

Eine Pivot-Tabelle, die aus einer anderen Datei erstellt wurde, behält die Daten, aus denen sie entstanden ist, in dieser Arbeitsmappe. So können Sie die Arbeitsmappe öffnen, ändern und speichern, auch wenn die andere Datei nicht mehr vorhanden ist. **Aktualisieren** liest die andere Datei erneut. Wurde die Datei verschoben, fehlt sie, ist sie zu groß, ist sie keine Arbeitsmappe oder enthält sie den Bereich nicht mehr, behält die Tabelle die Daten, die sie hatte, und eine Meldung teilt mit, dass die Datei nicht gelesen werden konnte. Eine Datei, die sich auf einem anderen Computer im Netzwerk befindet, wird erst gelesen, wenn Sie sie selbst unter **Datenquelle ändern** auswählen. Eine Aktualisierung liest jede Datei einmal, insgesamt höchstens 32 Dateien, 1 GB und 30 Sekunden; eine Tabelle, die darüber hinausgeht, behält ihre Daten.

Wenn eine Arbeitsmappe angibt, dass ihre Tabelle beim Öffnen erneut gelesen werden soll, und die Daten der Tabelle in einer anderen Datei liegen, fragt JeffCrunch zuerst nach, so wie es das auch bei Verknüpfungen zu anderen Arbeitsmappen tut. Bis Sie zustimmen, wird keine Datei gelesen.

Eine als .xlsx gespeicherte Arbeitsmappe speichert, wo sich die Datei befindet. Wenn Sie die Datei im Feld auswählen, wird ihre vollständige Adresse gespeichert. Eine Arbeitsmappe, die ein anderes Programm erstellt hat und deren Tabelle mit einem kurzen Pfad wie `Preise.xlsx` auf eine Datei verweist, behält diesen Pfad, und JeffCrunch sucht die Datei im selben Ordner wie die Arbeitsmappe. Wenn Sie mit **Speichern unter** die Arbeitsmappe in einem anderen Ordner speichern, wird ein solcher Pfad so umgeschrieben, dass er weiterhin dieselbe Datei nennt. Beim Aktualisieren werden keine Verknüpfungen verfolgt, die die andere Datei zu weiteren Dateien hat.

## Löschen

**Löschen** bietet zwei Möglichkeiten. **Filter löschen** entfernt alle Filter und zeigt wieder alle Einträge an. **Alle löschen** nimmt jedes Feld aus dem Bericht heraus und lässt ihn leer, bereit für einen neuen Aufbau.

## Details einblenden und ausblenden

## Berichtsfilterseiten

Ein Filter steht über einer Pivot-Tabelle und lässt Sie jeweils ein Element betrachten, etwa einen Monat. **Berichtsfilterseiten anzeigen** auf der Registerkarte **Pivot-Tabelle** erstellt eine Seite für jedes Element auf einmal. Jede Seite ist ein neues Blatt, das nach dem Element benannt ist und eine eigene Kopie der Tabelle enthält. Der Filter jeder Kopie ist auf dieses Element eingestellt. Hat die Tabelle mehr als einen Filter, wählen Sie aus, welcher verwendet wird. Ein Schritt von **Rückgängig** entfernt alle Seiten.

Hat ein Bericht mehr als ein Feld in den Zeilen, lässt sich ein Element des ersten Feldes öffnen und schließen. Klicken Sie auf die Zelle mit dem Namen des Elements und wählen Sie **Details ausblenden**, um es zu schließen: Die Zeilen des nächsten Feldes darunter werden ausgeblendet, und nur seine Summe bleibt. **Details einblenden** öffnet es wieder. Ein Element des letzten Feldes hat nichts darunter. Jeder Vorgang ist ein Schritt von **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabelle** wählt einen Teil des Berichts aus: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen links; hat der Bericht links keine, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht mit seiner ersten Zelle dort neu. Die Zellen, die er verlässt, werden geleert. Formeln, die auf die Zellen des Berichts zeigen, folgen ihnen, wie beim Ausschneiden und Einfügen. Ist am neuen Ort etwas, verschiebt JeffCrunch den Bericht nicht und teilt es Ihnen mit. Ein Schritt von **Rückgängig** verschiebt ihn zurück.

## Der Bereich mit den Feldern

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Felder der Pivot-Tabelle**. Er listet die Felder der Liste mit einem Kontrollkästchen daneben auf, darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt in **Werte** und wird addiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche und wählen Sie **Verschieben nach**, um es in einen anderen zu legen. Die Befehle **Nach oben verschieben** und **Nach unten verschieben** ändern seine Reihenfolge innerhalb des Bereichs, und **Feld entfernen** nimmt es heraus.
- Bei mehr als einem Wert erscheint in **Spalten** eine Zeile namens **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte links untereinander anzuzeigen.
- **Wertfeldeinstellungen** (ein Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er zusammengefasst wird: **Summe**, **Anzahl**, **Anzahl (Zahlen)**, **Mittelwert**, **Max**, **Min**, **Produkt**, **StdAbw** und **StdAbwN** (die Standardabweichung einer Stichprobe bzw. aller Werte) sowie **Var** und **VarianzN** (die Varianz derselben). Außerdem legt sie fest, wie der Wert angezeigt wird, und sein Zahlenformat. Siehe den nächsten Abschnitt.
- **Feldeinstellungen** (ein Doppelklick auf ein Feld in Filter, Spalten oder Zeilen oder die Schaltfläche) legt ein einzelnes Feld fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt von **Rückgängig**.

## Werte anzeigen als, und das Zahlenformat eines Werts

In **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts zeigt.

- **% der Gesamtsumme**: die Zelle geteilt durch die Summe des gesamten Berichts. Im Beispiel ist Ost im Januar 15 von 49, also 30,6 %.
- **% der Spaltensumme** und **% der Zeilensumme**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie die Zelle im Vergleich zu dem Wert steht, den ihre Zeile und Spalte bei gleichmäßiger Verteilung der Daten ergeben würden. Eine Zahl über 1 bedeutet mehr als der gleichmäßige Anteil.
- **Differenz von**, **% von** und **% Differenz von**: die Zelle im Vergleich zu einem anderen Element eines Feldes. Wählen Sie das **Basisfeld** (ein Feld in Zeilen oder Spalten) und das **Basiselement** (eines seiner Elemente oder das vorhergehende bzw. nachfolgende). Wählen Sie etwa Monat als Basisfeld und Jan als Basiselement, zeigt Februar, wie viel mehr oder weniger es als Januar ist. Das Basiselement selbst bleibt bei einer Differenz leer und ist bei **% von** 100 %.
- **Laufende Summe in**: jede Zelle addiert die Zellen davor entlang des Basisfeldes.
- **Rang vom Kleinsten zum Größten** und **Rang vom Größten zum Kleinsten**: jede Zelle zeigt ihren Platz, 1, 2, 3 und so weiter, unter den Zellen des Basisfeldes in derselben Zeile oder Spalte. Rang 1 ist die kleinste bzw. größte Zahl.
- **% der übergeordneten Zeilensumme** und **% der übergeordneten Spaltensumme**: jede Zelle zeigt, welchen Anteil sie an ihrem übergeordneten Element hat. Das übergeordnete Element eines Elements ist das Element eine Ebene höher, etwa das Jahr, zu dem ein Monat gehört. Ein Element ohne übergeordnetes Element wird als Anteil am Gesamtergebnis angezeigt.

**Zahlenformat** nimmt einen Formatcode, etwa `0,00`, `#.##0` oder `0,0%`. Lassen Sie das Feld leer, bleibt das Format der Zellen erhalten. Ein als Anteil angezeigter Wert erhält `0,00%`, sofern Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** nimmt es zusammen mit den Zahlen zurück.

## Feldeinstellungen

Wählen Sie ein Feld im Bereich **Filter**, **Spalten** oder **Zeilen** und wählen Sie auf der Registerkarte **Pivot-Tabelle** **Feldeinstellungen** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld anzeigt.
- **Teilergebnisse**: **Automatisch** zeigt eine Summe für jede Gruppe des Feldes, **Keine** zeigt keine.
- **Elementbeschriftungen anzeigen in**: Kurzform, Gliederungsform oder tabellarische Form, wie beim Berichtslayout.
- **Nach jedem Element eine Leerzeile einfügen** setzt nach jeder Gruppe eine leere Zeile.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe (Gliederungs- und tabellarische Form).
- **Sortieren**: die Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z bzw. von Z bis A sortieren. **Vom kleinsten zum größten nach einem Wert** und **Vom größten zum kleinsten nach einem Wert** sortieren die Elemente nach den Zahlen eines Wertfeldes. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filter** blendet Elemente anhand ihrer Beschriftungen aus (beginnt mit, endet mit, enthält, ist gleich) oder anhand eines Werts (größer als, kleiner als, zwischen und so weiter), oder behält nur die obersten oder untersten Elemente nach einem Wert. Der Filter wird beim Wählen von OK berechnet. Er blendet die ausgelassenen Elemente aus; die Kontrollkästchen darunter zeigen, welche es sind, und Sie können ein Element durch Aktivieren wieder einblenden.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Feldes mit jeweils einem Kontrollkästchen. Entfernen Sie den Haken, um ein Element auszublenden; die Summen lassen es dann aus.

## Datums- und Zahlenwerte gruppieren

Wählen Sie ein Feld mit Datums- oder Zahlenwerten in **Zeilen** oder **Spalten** und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Feld gruppieren**. Gruppen werden aus den Daten gebildet; die Spalte darf daher nur Zahlen oder Datumswerte enthalten, ohne Text und ohne leere Zellen.

- **Datumswerte**: Markieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste markierte Teil bleibt am Feld. Für jeden gröberen Teil fügt JeffCrunch ein Feld hinzu, das danach benannt ist (zum Beispiel **Jahre**), und setzt es an derselben Stelle vor das Datumsfeld. **Beginnen bei** und **Enden bei** sind das erste und das letzte Datum; lassen Sie sie unverändert, um das früheste und späteste Datum der Daten zu verwenden. Daten vor dem Beginn oder nach dem Ende kommen in zwei zusätzliche Gruppen, deren Namen mit `<` bzw. `>` beginnen.
- **Zahlen**: **Beginnen bei**, **Enden bei** und **Schrittweite** erstellen Gruppen wie 1-10, 11-20 und so weiter.

**Gruppierung aufheben** entfernt die Gruppen und die Felder, die dafür hinzugefügt wurden. Eine Aktualisierung bildet die Gruppen aus den neuen Daten neu. Wenn zwei Pivot-Tabellen aus denselben Daten erstellt wurden, können keine Gruppen gebildet werden, weil eine Gruppe ein Teil der gemeinsamen Daten ist.

## Berechnete Felder

Ein berechnetes Feld ist ein Feld, das Sie aus einer Formel erstellen. Es verwendet andere Felder der Liste. Ein Bonus von einem Zehntel des Betrags ist zum Beispiel `=Betrag*0,1`.

Wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Berechnetes Feld**. Geben Sie einen Namen und eine Formel ein. Schreiben Sie den Namen eines Feldes in die Formel oder doppelklicken Sie ihn in der Liste **Felder**, um ihn dorthin zu setzen. Einen Namen mit Leerzeichen setzen Sie in einfache Anführungszeichen, wie in `='Stückpreis'*Menge`. Sie können Zahlen, `+ - * / ^ %`, Vergleiche und einige Funktionen wie `WENN`, `RUNDEN`, `SUMME`, `MIN`, `MAX`, `MITTELWERT` und `ABS` verwenden.

Wählen Sie **Hinzufügen**. Das neue Feld wird in den Bereich **Werte** gesetzt. Es erscheint auch in der Feldliste, wo Sie es wie jedes andere Feld aktivieren oder deaktivieren können. Es kann nicht in Zeilen, Spalten oder Filter gesetzt werden.

Ein berechnetes Feld wird in jeder Zelle des Berichts berechnet. Der Name eines Feldes steht für die Summe dieses Feldes in der Zelle, sodass `=Betrag*Betrag` die Summe der Zelle quadriert. Es quadriert nicht jede Zeile und addiert die Ergebnisse. Ein Gesamtergebnis verwendet die Summen der gesamten Liste.

**Formeln auflisten** (neben **Berechnetes Feld**) erstellt ein neues Blatt, das jedes berechnete Feld mit seiner Formel auflistet sowie jedes berechnete Element mit seiner Formel, seinem Feld und seiner Berechnungsreihenfolge. **Rückgängig** entfernt die Zellen, ein weiterer Schritt von **Rückgängig** entfernt das Blatt.

Um ein Feld zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Feld und seinen Wert. Jeder Vorgang ist ein Schritt von **Rückgängig**. Ein berechnetes Feld wird mit der Datei gespeichert und bleibt bei einer Aktualisierung des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können kein gemeinsames berechnetes Feld haben, weil es ein Teil der gemeinsamen Daten ist.

## Berechnete Elemente

Ein berechnetes Element ist ein Element, das Sie aus anderen Elementen desselben Feldes bilden. Das Feld Region hat zum Beispiel die Elemente Ost und West. Ein neues Element namens Beide kann `=Ost+West` sein.

Klicken Sie auf eine Zelle der Pivot-Tabelle und wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Berechnetes Element**. Wählen Sie das **Feld**. Es muss in den Zeilen oder Spalten liegen. Geben Sie einen **Namen** und eine **Formel** ein. Schreiben Sie den Namen eines Elements in die Formel oder doppelklicken Sie ihn in der Liste **Elemente**, um ihn dorthin zu setzen. Einen Namen mit Leerzeichen setzen Sie in einfache Anführungszeichen, wie in `='Großes Ost'+West`. Sie können Zahlen, `+ - * / ^ %` und Klammern verwenden.

Das neue Element erhält eine Zeile im Bericht. In jeder Spalte zeigt es die Formel, berechnet aus den Zeilen der Elemente, die darin genannt werden. Das neue Element wird in die Summen einbezogen. Eine Summe wie das Gesamtergebnis oder ein Teilergebnis addiert die Zeile des neuen Elements zu den Zeilen der anderen Elemente. Bei Ost mit 35, West mit 14 und Beide (Ost plus West) mit 49 ergibt das Gesamtergebnis also 98. Ist ein Element, das die Formel nennt, ausgeblendet, zeigt die Zelle **#BEZUG!**. Nennt die Formel ein Element, das nicht vorhanden ist, zeigt die Zelle **#NAME?**.

Um ein Element zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Element und seine Zeile. Jeder Vorgang ist ein Schritt von **Rückgängig**. Ein berechnetes Element wird mit der Datei gespeichert und bleibt bei einer Aktualisierung des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können kein gemeinsames berechnetes Element haben, weil es ein Teil der gemeinsamen Daten ist.

### Berechnungsreihenfolge

Manchmal hat eine Zelle zwei berechnete Elemente: eines in ihrer Zeile und eines in ihrer Spalte. Die Reihenfolge, in der sie berechnet werden, kann das Ergebnis verändern. Angenommen, das Zeilenelement ist `=Ost-West` und das Spaltenelement ist `=Jan+1`. In einer Reihenfolge ergibt die Zelle (Ost - West) + 1. In der anderen ergibt sie (Ost + 1) - (West + 1), eine andere Zahl.

**Berechnungsreihenfolge** auf der Registerkarte **Pivot-Tabelle** listet die berechneten Elemente in der Reihenfolge auf, in der sie berechnet werden. Das Element weiter unten in der Liste wird zuletzt berechnet. Wählen Sie ein Element und klicken Sie auf **Nach oben verschieben** oder **Nach unten verschieben**. Die Reihenfolge wird mit der Datei als Reihenfolge der Elemente gespeichert, und andere Programme lesen sie genauso. **Formeln auflisten** zeigt den Platz jedes Elements darin.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert einer Pivot-Tabelle oder wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Details anzeigen**. JeffCrunch erstellt ein neues Blatt namens Detail 1, 2 und so weiter, mit den Zeilen der Liste, aus denen sich die Zahl zusammensetzt. Die Namen der Felder stehen in der ersten Zeile. F2 bearbeitet die Zelle weiterhin, sofern sie bearbeitet werden kann. **Rückgängig** entfernt die Zeilen, ein weiterer Schritt von **Rückgängig** entfernt das Blatt.

## Erweitern und reduzieren

Ein Feld, dessen Elemente andere darunter haben, kann die Zeilen unter seinen Elementen ein- oder ausblenden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** tun das für jedes Element des Feldes, das Sie im Bereich ausgewählt haben.

## Optionen der Pivot-Tabelle

**Optionen** auf der Registerkarte **Pivot-Tabelle** öffnet ein Feld mit den Einstellungen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn tragen.
- **Für Fehlerwerte anzeigen** und **Für leere Zellen anzeigen** setzen einen eigenen Text in diese Zellen.
- **Zellen mit Beschriftungen verbinden und zentrieren**, **Spaltenbreiten beim Aktualisieren der Tabelle anpassen** und **Zellformatierung beim Aktualisieren der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtergebnisse für Zeilen anzeigen** und **für Spalten anzeigen** entsprechen den Optionen für Gesamtergebnisse.
- **Daten beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Es ist ein Schritt von Rückgängig, wie jede Aktualisierung.

## Formatvorlage: Kopfzeilen, Bänder und Katalog

Die **Formatvorlagenoptionen** auf der Registerkarte **Pivot-Tabelle** schalten die Kopfzeile, die erste Spalte (Zeilenüberschriften) sowie die Bänder der Zeilen und Spalten ein oder aus. **Formatvorlagen** enthält einen Katalog mit Designs, und **Formatvorlage löschen** entfernt das Design. Diese Designs werden genauso gezeichnet wie die von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** legen die leere Zeile und die wiederholten Beschriftungen für alle Felder der Zeilen auf einmal fest.

## Gestaltung: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** blendet die Summe jeder Gruppe ein oder aus, oben oder unten in der Gruppe. Enthalten die Zeilen mehr als ein Feld, ist die Summe jedes äußeren Feldes ein Teilergebnis.
- **Gesamtergebnisse** hat vier Möglichkeiten: aus für Zeilen und Spalten, ein für beide, nur für Zeilen (eine Spalte mit Summen rechts, eine für jede Zeile) und nur für Spalten (eine Zeile mit Summen unten).
- **Berichtslayout** ist die Form der Beschriftungen. In der **Kurzform** teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas weiter rechts angezeigt. In der **Gliederungsform** hat jedes Feld seine eigene Spalte, und jede Gruppe hat eine eigene Zeile über ihren Elementen. In der **tabellarischen Form** hat jedes Feld seine eigene Spalte, der Name der Gruppe steht in ihrer ersten Zeile, und das Teilergebnis steht unter der Gruppe.

## Dieselbe Liste, mit einem Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und oben erscheint eine Zeile mit **Monat** und **(Alle)**. Die Zahlen sind nun die Summe jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie auf eine Zelle einer Pivot-Tabelle und wählen Sie **Einfügen** und dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm mit den Werten der Tabelle. Die Gesamtergebnisse bleiben weg. Das Diagramm ist in der Datei als Diagramm der Pivot-Tabelle gekennzeichnet. Es liest die Zellen der Tabelle, daher ändern sich seine Zahlen, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Wenn die Tabelle mehr Zeilen oder Spalten bekommt, erstellen Sie das Diagramm neu, weil es die Zellen behält, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert, und eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie auf eine Zelle einer Pivot-Tabelle, dann auf der Registerkarte Pivot-Tabelle auf **Datenschnitte**. Der Bereich Datenschnitte öffnet sich und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat eine Schaltfläche für jedes Element seines Feldes. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut, um alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt von Rückgängig, und Rückgängig stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts.

Ein Datenschnitt, der mit der Datei kam, wird auch über den Zellen gezeichnet, dort wo die Datei ihn platziert. Er zeigt seine Beschriftung und eine Schaltfläche für jedes Element. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt. Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen, halten Sie **Strg** gedrückt und klicken Sie, um ein Element hinzuzufügen oder zu entfernen, und klicken Sie auf den Trichter oben rechts, um wieder alle Elemente anzuzeigen. Ein Klick ist ein Schritt von Rückgängig. Wird die Tabelle auf andere Weise gefiltert oder wählen Sie Rückgängig, folgen die Schaltflächen dem. Ist ein Datenschnitt zu klein, um alle Schaltflächen zu zeigen, erscheint rechts eine Bildlaufleiste: Drehen Sie das Mausrad über dem Datenschnitt, klicken Sie über oder unter den Schieber oder ziehen Sie ihn, um den Rest zu erreichen. Halten Sie **Umschalt** gedrückt und klicken Sie auf eine zweite Schaltfläche, um den ganzen Bereich von der ersten angeklickten bis zu dieser zu wählen; mit **Umschalt** und **Strg** wird der Bereich zu den bereits gewählten Elementen hinzugefügt. Ein gedruckter Datenschnitt kann nicht scrollen, daher druckt er die Schaltflächen, die hineinpassen, und seine letzte Zeile sagt, wie viele weitere Elemente es gibt. Ein Datenschnitt, der eine Tabelle statt einer Pivot-Tabelle filtert oder dessen Pivot-Tabelle nicht in der Datei ist, wird mit seiner Beschriftung und einem Satz gezeichnet, der sagt, dass JeffCrunch ihn noch nicht verwenden kann; er bleibt so in der Datei, wie er war. In einer Arbeitsmappe, die nur zum Lesen geöffnet ist, und auf einem Blatt, das gegen Änderungen an Pivot-Tabellen geschützt ist, filtert ein Klick die Tabelle nicht. Ein Datenschnitt wird mit dem Blatt gedruckt, sofern die Datei es nicht anders angibt. Ein Klick auf eine Schaltfläche filtert; ein Klick auf die Beschriftung oder den Rand wählt den Datenschnitt als Objekt aus, wie ein Bild. Ein ausgewählter Datenschnitt hat Ziehpunkte: Ziehen Sie ihn, um ihn zu verschieben, ziehen Sie an einem Ziehpunkt, um die Größe zu ändern, oder drücken Sie eine Pfeiltaste, um ihn zu verschieben. Halten Sie **Strg** oder **Umschalt** gedrückt und klicken Sie auf die Beschriftung, um ihn zusammen mit anderen Objekten auszuwählen. Klicken Sie mit der rechten Maustaste auf einen Datenschnitt, um ein Menü mit **Löschen** und dem Bereich **Datenschnitte** zu öffnen. Drücken Sie **Entf**, um einen ausgewählten Datenschnitt zu entfernen: Die Datei hat dann keinen Datenschnitt mehr, und der Filter, den er auf die Pivot-Tabelle gelegt hat, bleibt, bis Sie ihn ändern. Verschieben, Größe ändern und Löschen sind jeweils ein Schritt von Rückgängig; in einer Arbeitsmappe, die nur zum Lesen geöffnet ist, oder auf einem Blatt, das gegen Änderungen an Objekten geschützt ist, sind sie nicht möglich.

Um einen Datenschnitt mit der Tastatur zu bedienen, drücken Sie **F6**, um zum ersten Datenschnitt des Blatts zu gelangen (**Umschalt+F6** zum letzten). Dann bewegen Sie sich mit den Pfeiltasten, **Pos1**, **Ende**, **Bild auf** und **Bild ab** zwischen seinen Schaltflächen, wählen mit **Leertaste** oder **Eingabe** eine Schaltfläche aus, fügen mit **Strg+Leertaste** ein Element zu den gewählten hinzu oder entfernen es, wählen mit **Umschalt+Leertaste** den Bereich von der zuletzt gewählten Schaltfläche bis zu dieser und zeigen mit **Strg+Alt+C** wieder alle Elemente an. Drücken Sie **Esc**, um die Schaltflächen zu verlassen (dann verschieben die Pfeiltasten den Datenschnitt, und **Entf** entfernt ihn; solange Sie sich zwischen den Schaltflächen bewegen, tut **Entf** nichts), und noch einmal **Esc**, um zu den Zellen zurückzukehren. **F6** führt zum nächsten Datenschnitt, und nach dem letzten zurück zu den Zellen.

**Berichtsverbindungen** zeigt, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den Zeitraum der Datumswerte, den sie ausgewählt hat. JeffCrunch zeichnet eine Zeitachse derzeit weder über die Zellen noch ändert es sie. Beide bleiben in der Datei so, wie sie gespeichert wurden.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste mit bis zu fünf Millionen Zeilen und einen Bericht mit bis zu vier Millionen Zellen. Eine Datei, die diese Grenzen überschreitet, wird so angezeigt, wie sie gespeichert wurde, und kann nicht geändert werden. Das erneute Lesen von hunderttausend Zeilen mit zwanzig Spalten und das Berechnen des Berichts dauerte auf dem Computer, mit dem dies gemessen wurde, etwa eine halbe Sekunde; ein langsamerer Computer braucht länger.
