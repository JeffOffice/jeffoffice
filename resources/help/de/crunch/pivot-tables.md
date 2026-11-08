# Pivot-Tabellen
<!-- keywords: Listenformeln, berechnetes Feld, berechnetes Element, Berechnungsreihenfolge, Formelfeld, Berichtsfilterseiten, Pivot-Tabelle, Details einblenden, Details ausblenden, Pivot-Tabelle einfügen, erstellen, machen, auswählen, Pivot-Tabelle verschieben, GETPIVOTDATA, GETPIVOTDATA generieren, aktualisieren, alles aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, kompakt, Gliederung, tabellarisch, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent der Gesamtsumme, Differenz, laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datumswerte, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen der Datei aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Details anzeigen, Details einblenden, Zeilen hinter einem Wert -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste hat zum Beispiel für jeden Verkauf eine Zeile; der Bericht addiert die Verkäufe nach Region und Monat. Sie schreiben keine Formeln. Sie legen fest, welche Spalten der Liste links untereinander stehen, welche oben nebeneinander stehen und welche summiert wird, und der Bericht füllt sich selbst aus.

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

Eine Pivot-Tabelle mit **Region** links, **Monat** oben und der Summe von **Betrag** in der Mitte sieht so aus:

| Summe von Betrag | Monat | | |
|---|---|---|---|
| Region | Jan | Feb | Gesamtergebnis |
| Ost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Gesamtergebnis | 22 | 27 | 49 |

Ost im Januar ergibt 15, weil die beiden Zeilen Ost/Jan mit 10 und 5 addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die hier verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in die ein Feld gesetzt werden kann. Ein Feld in „Zeilen“ ergibt für jeden seiner verschiedenen Einträge eine Zeile; ein Feld in „Spalten“ ergibt für jeden Eintrag eine Spalte; ein Feld in „Werte“ wird summiert. Ein vierter Bereich, **Filter**, enthält ein Feld, mit dem Sie die nicht gewählten Zeilen ausblenden.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei speichert, damit sie Summen anzeigen kann, ohne die Zellen zu lesen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen, und ändert sich daher nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Eine Pivot-Tabelle erstellen

1. Klicken Sie in eine Zelle der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste benötigt oberhalb ihrer Daten eine Zeile mit Namen.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, wenn sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben.
4. Legen Sie fest, wo die Pivot-Tabelle erscheint: **Neues Blatt** (Standard) oder eine Zelle auf diesem oder einem anderen Blatt. Wählen Sie eine Stelle, an der nichts anderes steht, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle beginnt leer, und der Bereich **Pivot-Tabellenfelder** öffnet sich. Aktivieren Sie die gewünschten Felder: Ein Feld mit Zahlen wird summiert, jedes andere wird zu einer Zeile. Verschieben Sie die Felder danach in die Bereiche, wie in den folgenden Abschnitten beschrieben.

Ein neues Blatt ist ein eigener Schritt für „Rückgängig“, und die Tabelle ebenso. Beim Speichern als .xlsx wird die Tabelle so geschrieben, wie das ursprüngliche Programm sie schreibt, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle anzeigt, in ihren Zellen, nicht aber die Tabelle selbst; beim Speichern wird Sie darauf hingewiesen.

## GETPIVOTDATA: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` liefert die Zahl, die eine Pivot-Tabelle für die angegebenen Werte sowie Zeilen- und Spaltenbezeichnungen anzeigt. Für das Beispiel oben:

`=GETPIVOTDATA("Summe von Betrag"; $E$3; "Region"; "Ost"; "Monat"; "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, so wie die Tabelle ihn anzeigt (zum Beispiel Summe von Betrag). Auch der Name des Feldes, das summiert wird (Betrag), funktioniert.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Wenn Sie nur Region und Ost angeben, erhalten Sie die Summe für Ost, 35. Wenn Sie nichts angeben, erhalten Sie das Gesamtergebnis, 49. Die angegebenen Felder müssen die ersten Felder der Zeilen oder die ersten Felder der Spalten sein, in dieser Reihenfolge.
- Zeigt die Tabelle die Zahl nicht an, zeigt die Zelle `#BEZUG!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach dem Umordnen der Tabelle die richtige Zahl liest. Zum Ausschalten wählen Sie auf der Registerkarte **Pivot-Tabellenanalyse** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie daran nichts ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, schreibt die neuen Zahlen in die Zellen und speichert die Pivot-Tabelle so, dass das Programm, das sie erstellt hat, sie ohne Fehlermeldung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden und so weiter, die Datums- und Zahlenwerte gruppieren und die Werte als Anteil, als Differenz oder als laufende Summe anzeigen. Eine Pivot-Tabelle, die einen Wert als Rang oder als Anteil eines übergeordneten Elements anzeigt, von Hand erstellte Gruppen von Elementen enthält oder ein Datenmodell verwendet, wird so angezeigt und beibehalten, wie sie gespeichert wurde; die Befehle weisen darauf hin, dass sie sie nicht ändern können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabellenanalyse** erscheint.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wurde, und berechnet den Bericht neu. Neue Einträge werden nach den bereits vorhandenen eingefügt; Einträge, die nicht mehr existieren, werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabellenanalyse** tut dasselbe für alle Pivot-Tabellen der Arbeitsmappe.

Wenn der Bericht über Zellen wachsen würde, in denen etwas steht, aktualisiert JeffCrunch nicht. Es teilt Ihnen mit, welche Zellen im Weg sind. Leeren Sie diese Zellen oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie dann erneut.

**Rückgängig** macht eine Aktualisierung in einem Schritt rückgängig.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich ihrer ersten Zeile mit Namen. Geben Sie sie wie `Tabelle1!A1:D100` ein, oder geben Sie den Namen einer Tabelle oder eines selbst definierten Namens ein. JeffCrunch liest sie und berechnet den Bericht neu. Hat ein Feld der Liste in den neuen Zellen denselben Namen, bleibt seine Position im Bericht erhalten.

## Löschen

**Löschen** bietet zwei Möglichkeiten. **Filter löschen** entfernt alle Filter und zeigt wieder alle Einträge an. **Alles löschen** nimmt alle Felder aus dem Bericht heraus und lässt ihn leer, bereit für einen neuen Aufbau.

## Details einblenden und ausblenden

## Berichtsfilterseiten

Ein Filter steht über einer Pivot-Tabelle und lässt Sie jeweils ein Element betrachten, etwa einen Monat. **Berichtsfilterseiten anzeigen** auf der Registerkarte **Pivot-Tabellenanalyse** erstellt für jedes Element gleichzeitig eine Seite. Jede Seite ist ein neues Blatt, das nach dem Element benannt ist und eine eigene Kopie der Tabelle enthält. Der Filter jeder Kopie ist auf dieses Element eingestellt. Hat die Tabelle mehr als einen Filter, wählen Sie den zu verwendenden aus. Ein einziges **Rückgängig** entfernt alle Seiten.

Wenn ein Bericht mehr als ein Feld in den Zeilen hat, lässt sich ein Element des ersten Feldes öffnen und schließen. Klicken Sie auf die Zelle mit dem Namen des Elements und wählen Sie **Details ausblenden**, um es zu schließen: Die Zeilen des nächsten Feldes darunter werden ausgeblendet, und nur seine Summe bleibt sichtbar. **Details einblenden** öffnet es wieder. Ein Element des letzten Feldes hat nichts darunter. Jeder Schritt ist ein **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabellenanalyse** wählt einen Teil des Berichts aus: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen links; hat der Bericht links keine Namen, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht mit seiner ersten Zelle dort neu. Die Zellen, die er verlässt, werden geleert. Formeln, die auf die Zellen des Berichts zeigen, folgen ihnen, so wie beim Ausschneiden und Einfügen. Steht am neuen Ort etwas, verschiebt JeffCrunch den Bericht nicht und teilt Ihnen dies mit. Ein **Rückgängig** verschiebt ihn zurück.

## Der Bereich Felder

**Feldliste** auf der Registerkarte **Pivot-Tabellenanalyse** öffnet den Bereich **Pivot-Tabellenfelder**. Er listet die Felder der Liste mit einem Kontrollkästchen daneben auf, darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, wird in **Werte** gesetzt und summiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche und wählen Sie **Verschieben nach**, um es in einen anderen zu setzen. Die Befehle **Nach oben verschieben** und **Nach unten verschieben** ändern seine Reihenfolge innerhalb des Bereichs, und **Feld entfernen** nimmt es heraus.
- Bei mehr als einem Wert erscheint in **Spalten** eine Zeile mit der Bezeichnung **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte links untereinander anzuordnen.
- **Wertfeldeinstellungen** (ein Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er berechnet wird: **Summe**, **Anzahl**, **Anzahl (Zahlen)**, **Mittelwert**, **Max**, **Min**, **Produkt**, **StdAbw** und **StdAbwN** (die Standardabweichung einer Stichprobe bzw. aller Werte) sowie **Varianz** und **VarianzN** (die Varianz derselben). Außerdem legt sie fest, wie der Wert angezeigt wird, und sein Zahlenformat. Siehe den nächsten Abschnitt.
- **Feldeinstellungen** (ein Doppelklick auf ein Feld in Filter, Spalten oder Zeilen oder die Schaltfläche) legt ein einzelnes Feld fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt von **Rückgängig**.

## Werte anzeigen als und das Zahlenformat eines Werts

In **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts anzeigt.

- **% der Gesamtsumme**: die Zelle geteilt durch die Summe des gesamten Berichts. Im Beispiel ist Ost im Januar 15 von 49, also 30,6 %.
- **% der Spaltensumme** und **% der Zeilensumme**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie die Zelle im Vergleich zu dem Wert steht, den ihre Zeile und Spalte bei gleichmäßiger Verteilung der Daten ergeben würden. Eine Zahl über 1 bedeutet mehr als der gleichmäßige Anteil.
- **Differenz von**, **% von** und **% Differenz von**: die Zelle im Vergleich zu einem anderen Element eines Feldes. Wählen Sie das **Basisfeld** (ein Feld in Zeilen oder Spalten) und das **Basiselement** (eines seiner Elemente oder das davor oder danach). Wenn Sie zum Beispiel Monat als Basisfeld und Jan als Basiselement wählen, zeigt Februar, um wie viel er höher oder niedriger als Januar ist. Das Basiselement selbst bleibt bei einer Differenz leer und ist bei **% von** 100 %.
- **Laufende Summe in**: jede Zelle addiert die Zellen davor entlang des Basisfeldes.
- **Rang vom Kleinsten zum Größten** und **Rang vom Größten zum Kleinsten**: jede Zelle zeigt ihren Platz, 1, 2, 3 und so weiter, unter den Zellen des Basisfeldes in derselben Zeile oder Spalte. Rang 1 ist die kleinste bzw. größte Zahl.
- **% der übergeordneten Zeilensumme** und **% der übergeordneten Spaltensumme**: jede Zelle zeigt, welchen Anteil sie an ihrem übergeordneten Element hat. Das übergeordnete Element eines Elements ist das Element eine Ebene darüber, etwa das Jahr, das einen Monat enthält. Ein Element ohne übergeordnetes Element wird als Anteil am Gesamtergebnis angezeigt.

**Zahlenformat** nimmt einen Formatcode, etwa `0,00`, `#.##0` oder `0,0%`. Lassen Sie das Feld leer, um das Format der Zellen beizubehalten. Ein als Anteil angezeigter Wert erhält `0,00%`, wenn Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** nimmt es zusammen mit den Zahlen zurück.

## Feldeinstellungen

Markieren Sie ein Feld im Bereich **Filter**, **Spalten** oder **Zeilen** und wählen Sie auf der Registerkarte **Pivot-Tabellenanalyse** **Feldeinstellungen** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld anzeigt.
- **Teilergebnisse**: **Automatisch** zeigt eine Summe für jede Gruppe des Feldes, **Keine** zeigt keine.
- **Elementbeschriftungen anzeigen in**: kompakte, gegliederte oder tabellarische Form, wie unter Berichtslayout.
- **Nach jedem Element eine Leerzeile einfügen** setzt nach jeder Gruppe eine leere Zeile.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe (gegliederte und tabellarische Form).
- **Sortieren**: die Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z bzw. von Z bis A sortieren. **Vom kleinsten zum größten nach einem Wert** und **Vom größten zum kleinsten nach einem Wert** sortieren die Elemente nach den Zahlen eines Wertfeldes. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filter** blendet Elemente anhand ihrer Beschriftungen aus (beginnt mit, endet mit, enthält, ist gleich) oder anhand eines Wertes (größer als, kleiner als, zwischen und so weiter) oder zeigt nur die obersten oder untersten Elemente nach einem Wert an. Der Filter wird beim Klick auf OK berechnet. Er blendet die ausgelassenen Elemente aus; die Kontrollkästchen darunter zeigen, welche das sind, und Sie können ein Element durch Aktivieren wieder einblenden.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Feldes mit je einem Kontrollkästchen. Deaktivieren Sie ein Kästchen, um das Element auszublenden; die Summen lassen es dann aus.

## Datums- und Zahlenwerte gruppieren

Markieren Sie ein Feld mit Datums- oder Zahlenwerten in **Zeilen** oder **Spalten** und wählen Sie auf der Registerkarte **Pivot-Tabellenanalyse** die Option **Feld gruppieren**. Gruppen werden aus den Daten gebildet, daher dürfen in dieser Spalte nur Zahlen oder Datumswerte stehen, ohne Text und ohne leere Zellen.

- **Datumswerte**: Aktivieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste aktivierte Teil bleibt im Feld. Für jeden gröberen Teil fügt JeffCrunch ein nach ihm benanntes Feld hinzu (zum Beispiel **Jahre**) und setzt es vor das Datumsfeld an dieselbe Stelle. **Beginnt bei** und **Endet bei** sind das erste und das letzte Datum; lassen Sie sie unverändert, um das früheste und späteste Datum der Daten zu verwenden. Datumswerte vor dem Anfang oder nach dem Ende kommen in zwei zusätzliche Gruppen, die mit `<` bzw. `>` beginnen.
- **Zahlen**: **Beginnt bei**, **Endet bei** und **Schrittweite** bilden Gruppen wie 1–10, 11–20 und so weiter.

**Gruppierung aufheben** entfernt die Gruppen und die Felder, die für sie hinzugefügt wurden. Beim Aktualisieren werden die Gruppen aus den neuen Daten neu gebildet. Werden zwei Pivot-Tabellen aus denselben Daten erstellt, können keine Gruppen gebildet werden, weil eine Gruppe ein Teil der gemeinsamen Daten ist.

## Berechnete Felder

Ein berechnetes Feld ist ein Feld, das Sie aus einer Formel erstellen. Es verwendet andere Felder der Liste. Ein Bonus von einem Zehntel des Betrags ist zum Beispiel `=Betrag*0,1`.

Wählen Sie auf der Registerkarte **Pivot-Tabellenanalyse** die Option **Berechnetes Feld**. Geben Sie einen Namen und eine Formel ein. Schreiben Sie den Namen eines Feldes in die Formel oder doppelklicken Sie ihn in der Liste **Felder**, um ihn dorthin zu setzen. Einen Namen mit Leerzeichen setzen Sie in einfache Anführungszeichen, wie in `='Stückpreis'*Menge`. Sie können Zahlen, `+ - * / ^ %`, Vergleiche und einige Funktionen wie `WENN`, `RUNDEN`, `SUMME`, `MIN`, `MAX`, `MITTELWERT` und `ABS` verwenden.

Wählen Sie **Hinzufügen**. Das neue Feld wird in den Bereich **Werte** gesetzt. Es erscheint auch in der Feldliste, wo Sie es wie jedes andere Feld aktivieren oder deaktivieren können. Es kann nicht in Zeilen, Spalten oder Filter gesetzt werden.

Ein berechnetes Feld wird in jeder Zelle des Berichts berechnet. Der Name eines Feldes steht für die Summe dieses Feldes in der Zelle, daher quadriert `=Betrag*Betrag` die Summe der Zelle. Es quadriert nicht jede Zeile und addiert dann. Ein Gesamtergebnis verwendet die Summen der gesamten Liste.

**Listenformeln** (neben **Berechnetes Feld**) erstellt ein neues Blatt, auf dem jedes berechnete Feld mit seiner Formel sowie jedes berechnete Element mit seiner Formel, seinem Feld und seiner Berechnungsreihenfolge aufgeführt wird. **Rückgängig** entfernt die Zellen, und ein weiteres **Rückgängig** entfernt das Blatt.

Um ein Feld zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Feld und seinen Wert. Jeder Schritt ist ein Schritt von **Rückgängig**. Ein berechnetes Feld wird mit der Datei gespeichert und bleibt bei einer Aktualisierung des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können keines haben, weil ein berechnetes Feld ein Teil der gemeinsamen Daten ist.

## Berechnete Elemente

Ein berechnetes Element ist ein Element, das Sie aus anderen Elementen desselben Feldes bilden. Das Feld Region hat zum Beispiel die Elemente Ost und West. Ein neues Element mit dem Namen Beide kann `=Ost+West` sein.

Klicken Sie in der Pivot-Tabelle auf eine Zelle und wählen Sie auf der Registerkarte **Pivot-Tabellenanalyse** die Option **Berechnetes Element**. Wählen Sie das **Feld** aus. Es muss in den Zeilen oder Spalten stehen. Geben Sie einen **Namen** und eine **Formel** ein. Schreiben Sie den Namen eines Elements in die Formel oder doppelklicken Sie ihn in der Liste **Elemente**, um ihn dorthin zu setzen. Einen Namen mit Leerzeichen setzen Sie in einfache Anführungszeichen, wie in `='Großer Ost'+West`. Sie können Zahlen, `+ - * / ^ %` und Klammern verwenden.

Das neue Element erhält eine Zeile im Bericht. In jeder Spalte zeigt es die Formel, berechnet aus den Zeilen der Elemente, die es nennt. Das neue Element zählt in die Summen mit. Eine Summe, etwa ein Gesamtergebnis oder ein Teilergebnis, addiert die Zeile des neuen Elements zu den Zeilen der anderen Elemente. Bei Ost mit 35, West mit 14 und Beide (Ost plus West) mit 49 ist das Gesamtergebnis also 98. Ist ein Element, das die Formel nennt, ausgeblendet, zeigt die Zelle **#BEZUG!**. Nennt die Formel ein Element, das nicht vorhanden ist, zeigt die Zelle **#NAME?**.

Um ein Element zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Element und seine Zeile. Jeder Schritt ist ein Schritt von **Rückgängig**. Ein berechnetes Element wird mit der Datei gespeichert und bleibt bei einer Aktualisierung des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können keines haben, weil ein berechnetes Element ein Teil der gemeinsamen Daten ist.

### Berechnungsreihenfolge

Manchmal hat eine Zelle zwei berechnete Elemente. Eines steht in ihrer Zeile, das andere in ihrer Spalte. Die Reihenfolge, in der sie berechnet werden, kann das Ergebnis verändern. Angenommen, das Zeilenelement ist `=Ost-West` und das Spaltenelement ist `=Jan+1`. Berechnet in einer Reihenfolge, ist die Zelle (Ost - West) + 1. In der anderen Reihenfolge ist sie (Ost + 1) - (West + 1), was eine andere Zahl ergibt.

**Berechnungsreihenfolge** auf der Registerkarte **Pivot-Tabellenanalyse** listet die berechneten Elemente in der Reihenfolge auf, in der sie berechnet werden. Das weiter unten stehende Element wird zuletzt berechnet. Wählen Sie ein Element und klicken Sie auf **Nach oben verschieben** oder **Nach unten verschieben**. Die Reihenfolge wird als Reihenfolge der Elemente mit der Datei gespeichert, und andere Programme lesen sie genauso. **Listenformeln** zeigt die Stelle jedes Elements darin.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert der Pivot-Tabelle oder wählen Sie auf der Registerkarte **Pivot-Tabellenanalyse** die Option **Details anzeigen**. JeffCrunch erstellt ein neues Blatt mit dem Namen Detail 1, Detail 2 und so weiter, mit den Zeilen der Liste, aus denen die Zahl besteht. Die Feldnamen stehen in der ersten Zeile. F2 bearbeitet die Zelle weiterhin, wenn sie bearbeitet werden kann. **Rückgängig** entfernt die Zeilen, und ein weiteres **Rückgängig** entfernt das Blatt.

## Erweitern und reduzieren

Ein Feld, dessen Elemente weitere Elemente darunter haben, kann die Zeilen unter seinen Elementen ein- oder ausblenden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** tun dies für jedes Element des Feldes, das Sie im Bereich ausgewählt haben.

## Pivot-Tabellen-Optionen

**Optionen** auf der Registerkarte **Pivot-Tabellenanalyse** öffnet ein Feld mit den Einstellungen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn tragen.
- **Für Fehlerwerte anzeigen** und **Für leere Zellen anzeigen** setzen einen eigenen Text in diese Zellen.
- **Zellen mit Beschriftungen verbinden und zentrieren**, **Spaltenbreiten beim Aktualisieren der Tabelle anpassen** und **Formatierung der Zellen beim Aktualisieren der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtergebnisse für Zeilen anzeigen** und **für Spalten anzeigen** entsprechen den Optionen für Gesamtergebnisse.
- **Daten beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Es ist ein Schritt von „Rückgängig“, wie jede Aktualisierung.

## Formatvorlagen: Kopfzeilen, Bänder und Katalog

Die **Formatvorlagenoptionen** auf der Registerkarte **Pivot-Tabellenanalyse** schalten die Kopfzeile, die erste Spalte (Zeilenüberschriften) sowie die Bänder der Zeilen und Spalten ein oder aus. **Formatvorlagen** bietet einen Katalog mit Looks, und **Formatvorlage löschen** nimmt den Look weg. Diese Looks werden genauso gezeichnet wie die Looks von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** legen die leere Zeile und die wiederholten Beschriftungen für alle Felder der Zeilen gleichzeitig fest.

## Gestaltung: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** blendet die Summe jeder Gruppe ein oder aus, oben oder unten in der Gruppe. Hat Zeilen mehr als ein Feld, ist die Summe jedes äußeren Feldes ein Teilergebnis.
- **Gesamtergebnisse** hat vier Möglichkeiten: aus für Zeilen und Spalten, ein für beide, nur für Zeilen (eine Spalte mit Summen rechts, eine für jede Zeile) und nur für Spalten (eine Zeile mit Summen unten).
- **Berichtslayout** bestimmt die Form der Beschriftungen. In **kompakter** Form teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas weiter rechts angezeigt. In **gegliederter** Form hat jedes Feld seine eigene Spalte, und jede Gruppe hat eine eigene Zeile über ihren Elementen. In **tabellarischer** Form hat jedes Feld seine eigene Spalte, und der Name der Gruppe steht in ihrer ersten Zeile, mit dem Teilergebnis unter der Gruppe.

## Dieselbe Liste, ein Filter

Verschieben Sie **Monat** aus **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und über ihm erscheint eine Zeile mit **Monat** und **(Alle)**. Die Zahlen sind nun die Summen jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie in einer Pivot-Tabelle auf eine Zelle und wählen Sie **Einfügen** und dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm aus den Werten der Tabelle. Es lässt die Gesamtergebnisse weg. Das Diagramm wird in der Datei als Diagramm der Pivot-Tabelle gekennzeichnet. Es liest die Zellen der Tabelle, daher ändern sich seine Zahlen, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Wenn die Tabelle mehr Zeilen oder Spalten bekommt, erstellen Sie das Diagramm neu, da es die Zellen behält, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert, und eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie in einer Pivot-Tabelle auf eine Zelle und dann auf der Registerkarte Pivot-Tabellenanalyse auf **Datenschnitte**. Der Bereich Datenschnitte öffnet sich und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat eine Schaltfläche für jedes Element seines Feldes. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut darauf, um alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt von „Rückgängig“, und „Rückgängig“ stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts bei.

**Berichtsverbindungen** zeigt, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den Zeitraum, den sie gewählt hat. JeffCrunch zeigt die Datenschnitte in diesem Bereich an, nicht über den Zellen, und ändert eine Zeitachse derzeit noch nicht. Beides wird so in der Datei behalten, wie es gespeichert wurde.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste mit bis zu fünf Millionen Zeilen und einen Bericht mit bis zu vier Millionen Zellen. Eine Datei, die diese Grenzen überschreitet, wird so angezeigt, wie sie gespeichert wurde, und kann nicht geändert werden. Das erneute Lesen von hunderttausend Zeilen mit zwanzig Spalten und das Berechnen des Berichts dauerten auf dem Computer, mit dem dies gemessen wurde, etwa eine halbe Sekunde; auf einem langsameren Computer dauert es länger.
