# Pivot-Tabellen
<!-- keywords: andere Datei, andere Arbeitsmappe, externe Quelle, Formeln auflisten, berechnetes Feld, berechnetes Element, Berechnungsreihenfolge, Formelfeld, Berichtsfilterseiten, Pivot-Tabelle, Details einblenden, Details ausblenden, Pivot-Tabelle einfügen, erstellen, machen, auswählen, Pivot-Tabelle verschieben, GETPIVOTDATA, GETPIVOTDATA erzeugen, aktualisieren, alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, kompakt, Gliederung, tabellarisch, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Werte anzeigen als, Prozent der Gesamtsumme, Differenz, laufende Summe, Zahlenformat, Feldeinstellungen, sortieren, filtern, Top, gruppieren, Gruppierung aufheben, Datum, Monate, Jahre, erweitern, reduzieren, Optionen, Formatvorlage, gebänderte Zeilen, leere Zeilen, Beschriftungen wiederholen, beim Öffnen der Datei aktualisieren, Pivot-Diagramm, Datenschnitt, Zeitachse, Details anzeigen, Details einblenden, Zeilen hinter einem Wert</keywords>

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste entsteht. Die Liste hat etwa eine Zeile für jeden Verkauf; der Bericht addiert die Verkäufe nach Region und Monat. Sie schreiben dafür keine Formeln. Sie legen fest, welche Spalten der Liste links untereinander stehen, welche oben quer stehen und welche addiert wird, und der Bericht füllt sich selbst aus.

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

Ost im Januar ist 15, weil die beiden Zeilen von Ost im Januar, 10 und 5, addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Stellen, an die ein Feld gesetzt werden kann. Ein Feld in den Zeilen ergibt eine Zeile für jeden seiner verschiedenen Einträge; ein Feld in den Spalten ergibt eine Spalte für jeden; ein Feld in den Werten ist dasjenige, das addiert wird. Eine vierte Stelle, **Filter**, enthält ein Feld, mit dem Sie die Zeilen ausblenden, die Sie nicht wählen.
- Ein **Cache** (der Pivot-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei aufbewahrt, damit sie Summen zeigen kann, ohne auf die Zellen zu schauen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen, und ändert sich deshalb nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Eine Pivot-Tabelle erstellen

1. Klicken Sie in eine Zelle der Liste oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste benötigt oberhalb ihrer Daten eine Zeile mit Namen.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, wenn sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben. Um eine Liste aus einer anderen Datei zu verwenden, wählen Sie **Ein Bereich in einer anderen Datei**, wählen die Datei mit **Durchsuchen** aus und geben den Bereich ein oder den Namen einer Tabelle bzw. eines Namens, den Sie in dieser Datei angelegt haben, etwa `Tabelle1!A1:D100`. JeffCrunch liest Arbeitsmappen (`.xlsx`, `.xlsm`, `.xls` und `.ods`) und `.csv`-Dateien.
4. Legen Sie fest, wo die Pivot-Tabelle erscheint: **Neues Blatt** (Standard) oder eine Zelle auf diesem oder einem anderen Blatt. Wählen Sie einen Ort, an dem nichts steht, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle beginnt leer, und der Bereich **Felder der Pivot-Tabelle** öffnet sich. Aktivieren Sie die Felder, die Sie möchten: Ein Feld mit Zahlen wird addiert, jedes andere wird zu einer Zeile. Verschieben Sie die Felder dann wie in den folgenden Abschnitten beschrieben in die Kästchen.

Stammen die Daten aus einer anderen Datei, behält die Pivot-Tabelle eine Kopie davon. So funktioniert die Tabelle und lässt sich speichern, ohne dass die andere Datei vorhanden ist. Siehe dazu unten: Daten aus einer anderen Datei.

Ein neues Blatt ist ein eigener Schritt für **Rückgängig**, und die Tabelle ebenfalls. Beim Speichern als .xlsx wird die Tabelle so geschrieben, wie das Originalprogramm sie schreibt, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle zeigt, in ihren Zellen, nicht aber die Tabelle selbst; beim Speichern erhalten Sie einen Hinweis darauf.

## GETPIVOTDATA: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` liefert die Zahl, die eine Pivot-Tabelle für die Werte und die Zeilen und Spalten zeigt, die Sie nennen. Für das Beispiel oben:

`=GETPIVOTDATA("Summe von Betrag"; $E$3; "Region"; "Ost"; "Monat"; "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, so wie die Tabelle ihn zeigt (zum Beispiel Summe von Betrag). Der Name des Feldes, das addiert wird (Betrag), funktioniert ebenfalls.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Nennen Sie nur Region und Ost, erhalten Sie die Summe von Ost, 35. Nennen Sie nichts, erhalten Sie das Gesamtergebnis, 49. Die genannten Felder müssen in der Reihenfolge die ersten Felder der Zeilen oder die ersten Felder der Spalten sein.
- Zeigt die Tabelle die Zahl nicht an, zeigt die Zelle `#BEZUG!`.

Klicken Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach dem Umordnen der Tabelle die richtige Zahl liest. Um das abzuschalten, wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen zu GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Ändern Sie nichts daran, wird die Datei mit genau derselben Pivot-Tabelle gespeichert. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, schreibt die neuen Zahlen in die Zellen und speichert die Pivot-Tabelle so, dass das Programm, das sie erstellt hat, sie ohne Fehlermeldung öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden und Ähnliches, die Datums- und Zahlenwerte gruppieren und Werte als Anteil, als Differenz oder als laufende Summe anzeigen. Eine Pivot-Tabelle, die einen Wert als Rang oder als Anteil eines übergeordneten Elements anzeigt, selbst angelegte Gruppen von Elementen enthält oder ein Datenmodell verwendet, wird so angezeigt und gespeichert, wie sie gespeichert wurde. Die Befehle weisen darauf hin, dass sie sie nicht ändern können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** erscheint.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt ist, und berechnet den Bericht neu. Neue Einträge kommen nach den bereits vorhandenen; Einträge, die nicht mehr da sind, werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle** tut dasselbe für alle Pivot-Tabellen der Arbeitsmappe.

Würde der Bericht über Zellen wachsen, in denen etwas steht, aktualisiert JeffCrunch nicht. Es sagt Ihnen, welche Zellen im Weg sind. Leeren Sie diese Zellen oder verschieben Sie die Pivot-Tabelle und aktualisieren Sie dann erneut.

**Rückgängig** nimmt eine Aktualisierung mit einem Schritt zurück.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen, in denen die Liste steht, einschließlich ihrer ersten Zeile mit Namen. Geben Sie sie ein wie `Tabelle1!A1:D100` oder geben Sie den Namen einer Tabelle oder einen Namen ein, den Sie angelegt haben. JeffCrunch liest sie und berechnet den Bericht neu. Hat ein Feld der Liste in den neuen Zellen denselben Namen, bleibt seine Stelle im Bericht erhalten.

Sie können die Tabelle auch auf einen Bereich in einer anderen Datei richten. Wählen Sie im Feld **Ein Bereich in einer anderen Datei**, wählen Sie die Datei und geben Sie den Bereich darin ein. Wählen Sie **Ein Bereich in dieser Arbeitsmappe**, zeigt die Tabelle wieder auf Zellen hier. Jede dieser Änderungen ist ein Schritt für Rückgängig.

## Daten aus einer anderen Datei

Eine Pivot-Tabelle, die aus einer anderen Datei erstellt wurde, behält die Daten, aus denen sie entstanden ist, in dieser Arbeitsmappe. So können Sie die Arbeitsmappe öffnen, ändern und speichern, auch wenn die andere Datei nicht mehr da ist. **Aktualisieren** liest die andere Datei erneut. Wurde die Datei verschoben, fehlt sie, ist sie zu groß, ist sie keine Arbeitsmappe oder enthält sie den Bereich nicht mehr, behält die Tabelle die Daten, die sie hatte, und eine Meldung sagt, dass die Datei nicht gelesen werden konnte. Eine Datei, die auf einem anderen Computer im Netzwerk liegt, wird erst gelesen, wenn Sie sie selbst unter **Datenquelle ändern** wählen. Eine Aktualisierung liest jede Datei einmal, und insgesamt höchstens 32 Dateien, 1 GB und 30 Sekunden; eine Tabelle, die darüber hinausgeht, behält ihre Daten.

Sagt eine Arbeitsmappe, dass ihre Tabelle beim Öffnen erneut gelesen werden soll, und liegen die Daten der Tabelle in einer anderen Datei, fragt JeffCrunch zuerst nach, so wie es bei Verknüpfungen zu anderen Arbeitsmappen geschieht. Bis Sie zustimmen, wird keine Datei gelesen.

Eine als .xlsx gespeicherte Arbeitsmappe speichert, wo die Datei liegt. Wählen Sie die Datei im Feld, wird ihre vollständige Adresse gespeichert. Eine Arbeitsmappe, die ein anderes Programm erstellt hat und deren Tabelle mit einem kurzen Pfad wie `Preise.xlsx` auf eine Datei verweist, behält diesen Pfad, und JeffCrunch sucht die Datei im selben Ordner wie die Arbeitsmappe. Wenn Sie die Arbeitsmappe mit **Speichern unter** in einem anderen Ordner speichern, wird ein solcher Pfad so umgeschrieben, dass er weiterhin dieselbe Datei nennt. Eine Aktualisierung folgt keinen Verknüpfungen, die die andere Datei ihrerseits auf weitere Dateien hat.

## Löschen

**Löschen** hat zwei Möglichkeiten. **Filter löschen** entfernt alle Filter und zeigt alle Einträge wieder an. **Alle löschen** nimmt alle Felder aus dem Bericht heraus und lässt ihn leer, bereit für einen neuen Aufbau.

## Details einblenden und ausblenden

## Berichtsfilterseiten

Ein Berichtsfilter steht über einer Pivot-Tabelle und lässt Sie jeweils ein Element betrachten, etwa einen Monat. **Berichtsfilterseiten anzeigen** auf der Registerkarte **Pivot-Tabelle** erstellt für jedes Element gleichzeitig eine Seite. Jede Seite ist ein neues Blatt, nach dem Element benannt, mit einer eigenen Kopie der Tabelle. Der Filter jeder Kopie ist auf dieses Element gesetzt. Hat die Tabelle mehr als einen Filter, wählen Sie den aus, der verwendet werden soll. Ein einziger Schritt **Rückgängig** entfernt alle Seiten.

Hat ein Bericht mehr als ein Feld in den Zeilen, lässt sich ein Element des ersten Feldes öffnen und schließen. Klicken Sie auf die Zelle mit dem Namen des Elements und wählen Sie **Details ausblenden**, um es zu schließen: Die Zeilen des nächsten Feldes darunter werden ausgeblendet, und nur seine Summe bleibt stehen. **Details einblenden** öffnet es wieder. Ein Element des letzten Feldes hat nichts darunter. Jeder Schritt ist ein Schritt für **Rückgängig**.

## Auswählen und verschieben

**Auswählen** auf der Registerkarte **Pivot-Tabelle** wählt einen Teil des Berichts aus: **Gesamte Pivot-Tabelle**, **Werte** (die Zahlen) oder **Beschriftungen** (die Namen links; gibt es im Bericht keine Namen links, die Namen oben). **Pivot-Tabelle verschieben…** fragt nach einer Zelle auf demselben Blatt und zeichnet den Bericht mit seiner ersten Zelle dort neu. Die Zellen, die er verlässt, werden geleert. Formeln, die auf Zellen des Berichts zeigen, folgen ihnen, so wie beim Ausschneiden und Einfügen. Ist am neuen Ort etwas, verschiebt JeffCrunch den Bericht nicht und sagt es Ihnen. Ein Schritt **Rückgängig** verschiebt ihn zurück.

## Der Bereich mit den Feldern

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Felder der Pivot-Tabelle**. Er zeigt die Felder der Liste mit je einem Kästchen daneben und darunter die vier Stellen: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt in **Werte** und wird addiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einer der vier Stellen und wählen Sie **Verschieben nach**, um es an eine andere zu setzen. Die Schaltflächen **Nach oben verschieben** und **Nach unten verschieben** ändern seine Reihenfolge an seiner Stelle, und **Feld entfernen** nimmt es heraus.
- Gibt es mehr als einen Wert, erscheint in **Spalten** eine Zeile namens **Werte**. Verschieben Sie sie nach **Zeilen**, um die Werte links untereinander zu setzen.
- **Wertfeldeinstellungen** (ein Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert zeigt, und wie er summiert wird: **Summe**, **Anzahl**, **Anzahl (Zahlen)**, **Mittelwert**, **Max**, **Min**, **Produkt**, **StdAbw** und **StdAbwp** (die Standardabweichung einer Stichprobe bzw. aller Werte) sowie **Varianz** und **Varianzp** (die Varianz derselben). Außerdem legt es fest, wie der Wert angezeigt wird und sein Zahlenformat. Siehe den nächsten Abschnitt.
- **Feldeinstellungen** (ein Doppelklick auf ein Feld in Filter, Spalten oder Zeilen oder die Schaltfläche) legt ein einzelnes Feld fest. Siehe „Feldeinstellungen“.

Jede Änderung ist ein Schritt für **Rückgängig**.

## Werte anzeigen als und das Zahlenformat eines Werts

In **Wertfeldeinstellungen** ändert **Werte anzeigen als**, was eine Zelle des Berichts zeigt.

- **% der Gesamtsumme**: die Zelle geteilt durch die Summe des ganzen Berichts. Im Beispiel ist Ost im Januar 15 von 49, also 30,6 %.
- **% der Spaltensumme** und **% der Zeilensumme**: die Zelle geteilt durch die Summe ihrer Spalte bzw. ihrer Zeile.
- **Index**: wie die Zelle im Vergleich zu dem Wert steht, den ihre Zeile und Spalte bei gleichmäßiger Verteilung der Daten ergeben würden. Eine Zahl über 1 bedeutet mehr als der gleichmäßige Anteil.
- **Differenz zu**, **% von** und **% Differenz zu**: die Zelle im Vergleich zu einem anderen Element eines Feldes. Wählen Sie das **Basisfeld** (ein Feld in Zeilen oder Spalten) und das **Basiselement** (eines seiner Elemente oder das davor oder danach). Wählen Sie etwa Monat als Basisfeld und Jan als Basiselement, zeigt Februar, um wie viel es mehr oder weniger ist als Januar. Das Basiselement selbst bleibt bei einer Differenz leer und ist bei **% von** 100 %.
- **Laufende Summe in**: jede Zelle addiert die Zellen davor entlang des Basisfeldes.
- **Vom kleinsten zum größten Rang** und **Vom größten zum kleinsten Rang**: jede Zelle zeigt ihren Platz, 1, 2, 3 und so weiter, unter den Zellen des Basisfeldes in derselben Zeile oder Spalte. Rang 1 ist die kleinste bzw. größte Zahl.
- **% der übergeordneten Zeilensumme** und **% der übergeordneten Spaltensumme**: jede Zelle zeigt, welchen Teil ihres übergeordneten Elements sie ausmacht. Das übergeordnete Element eines Elements ist das eine Ebene höher, etwa das Jahr, das einen Monat enthält. Ein Element ohne übergeordnetes Element wird als Teil des Gesamtergebnisses angezeigt.

**Zahlenformat** nimmt einen Formatcode, etwa `0,00`, `#.##0` oder `0,0%`. Lassen Sie das Feld leer, bleibt das Format der Zellen erhalten. Ein Wert, der als Anteil angezeigt wird, erhält `0,00%`, wenn Sie kein Format wählen. Die Zellen des Berichts übernehmen das Format sofort. **Rückgängig** nimmt es zusammen mit den Zahlen zurück.

## Feldeinstellungen

Wählen Sie ein Feld im Kasten **Filter**, **Spalten** oder **Zeilen** und wählen Sie **Feldeinstellungen** auf der Registerkarte **Pivot-Tabelle** (oder doppelklicken Sie darauf).

- **Benutzerdefinierter Name** ändert den Namen, den der Bericht für das Feld zeigt.
- **Teilergebnisse**: **Automatisch** zeigt eine Summe für jede Gruppe des Feldes, **Keine** zeigt keine.
- **Elementbeschriftungen anzeigen in**: kompakt, Gliederung oder tabellarisch, wie im Berichtslayout.
- **Nach jedem Element eine Leerzeile einfügen** setzt eine leere Zeile nach jede Gruppe.
- **Elementbeschriftungen wiederholen** zeigt den Namen eines äußeren Elements in jeder Zeile seiner Gruppe (Gliederungs- und tabellarische Form).
- **Sortieren**: die Reihenfolge der Elemente beibehalten (manuell) oder nach den Beschriftungen von A bis Z bzw. von Z bis A sortieren. **Vom kleinsten zum größten nach einem Wert** und **Vom größten zum kleinsten nach einem Wert** sortieren die Elemente nach den Zahlen eines Wertfeldes. Der Bericht behält die Sortierung bei, wenn sich die Daten ändern.
- **Filtern** blendet Elemente anhand ihrer Beschriftungen aus (beginnt mit, endet mit, enthält, ist gleich) oder anhand eines Werts (größer als, kleiner als, zwischen und so weiter) oder behält nur die obersten oder untersten Elemente nach einem Wert. Der Filter wird beim Wählen von OK berechnet. Er blendet die Elemente aus, die er weglässt; die Kontrollkästchen darunter zeigen, welche das sind, und Sie können ein Element durch Aktivieren wieder einblenden.
- **Diese Elemente anzeigen** ist eine Liste der Elemente des Feldes mit je einem Kontrollkästchen. Deaktivieren Sie ein Kästchen, um das Element auszublenden; die Summen lassen es dann weg.

## Datums- und Zahlenwerte gruppieren

Wählen Sie ein Feld mit Datums- oder Zahlenwerten in **Zeilen** oder **Spalten** und wählen Sie **Feld gruppieren** auf der Registerkarte **Pivot-Tabelle**. Gruppen entstehen aus den Daten, deshalb dürfen in dieser Spalte nur Zahlen oder nur Datumswerte stehen, ohne Text und ohne leere Zellen.

- **Datumswerte**: Aktivieren Sie, wonach gruppiert werden soll: **Sekunden**, **Minuten**, **Stunden**, **Tage**, **Monate**, **Quartale** oder **Jahre**. Der feinste aktivierte Teil bleibt am Feld. Für jeden gröberen Teil fügt JeffCrunch ein Feld hinzu, das nach ihm benannt ist (etwa **Jahre**), und setzt es vor das Datumsfeld an dieselbe Stelle. **Beginnen bei** und **Enden bei** sind das erste und das letzte Datum; lassen Sie sie unverändert, um das früheste und das späteste Datum der Daten zu verwenden. Datumswerte vor dem Beginn oder nach dem Ende kommen in zwei zusätzliche Gruppen, die mit `<` bzw. `>` beginnen.
- **Zahlen**: **Beginnen bei**, **Enden bei** und **Schrittweite** erzeugen Gruppen wie 1-10, 11-20 und so weiter.

**Gruppierung aufheben** entfernt die Gruppen und die Felder, die für sie hinzugefügt wurden. Eine Aktualisierung erstellt die Gruppen aus den neuen Daten neu. Bei zwei Pivot-Tabellen aus denselben Daten können keine Gruppen erstellt werden, denn eine Gruppe ist ein Teil der Daten, die beide gemeinsam haben.

## Berechnete Felder

Ein berechnetes Feld ist ein Feld, das Sie aus einer Formel machen. Es verwendet andere Felder der Liste. Ein Bonus von einem Zehntel des Betrags ist zum Beispiel `=Betrag*0,1`.

Wählen Sie **Berechnetes Feld** auf der Registerkarte **Pivot-Tabelle**. Geben Sie einen Namen und eine Formel ein. Schreiben Sie den Namen eines Feldes in die Formel oder doppelklicken Sie ihn in der Liste **Felder**, um ihn einzufügen. Einen Namen mit Leerzeichen setzen Sie in einfache Anführungszeichen, wie in `='Stückpreis'*Menge`. Sie können Zahlen, `+ - * / ^ %`, Vergleiche und einige Funktionen wie `WENN`, `RUNDEN`, `SUMME`, `MIN`, `MAX`, `MITTELWERT` und `ABS` verwenden.

Wählen Sie **Hinzufügen**. Das neue Feld wird in das Kästchen **Werte** gesetzt. Es erscheint auch in der Feldliste, wo Sie es wie jedes andere Feld aktivieren oder deaktivieren können. Es kann nicht in die Zeilen, Spalten oder Filter.

Ein berechnetes Feld wird in jeder Zelle des Berichts berechnet. Der Name eines Feldes steht für die Summe dieses Feldes in der Zelle, daher quadriert `=Betrag*Betrag` die Summe der Zelle. Es quadriert nicht jede Zeile und addiert die Ergebnisse. Ein Gesamtergebnis verwendet die Summen der ganzen Liste.

**Formeln auflisten** (neben **Berechnetes Feld**) erstellt ein neues Blatt, das jedes berechnete Feld mit seiner Formel nennt sowie jedes berechnete Element mit seiner Formel, seinem Feld und seiner Berechnungsreihenfolge. **Rückgängig** entfernt die Zellen, ein weiterer **Rückgängig** entfernt das Blatt.

Um ein Feld zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Feld und seinen Wert. Jeder Schritt ist ein Schritt für **Rückgängig**. Ein berechnetes Feld wird mit der Datei gespeichert und bleibt beim Aktualisieren des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können keines haben, denn ein berechnetes Feld ist ein Teil der Daten, die sie gemeinsam haben.

## Berechnete Elemente

Ein berechnetes Element ist ein Element, das Sie aus anderen Elementen desselben Feldes machen. Das Feld Region hat zum Beispiel die Elemente Ost und West. Ein neues Element mit dem Namen Beide kann `=Ost+West` sein.

Klicken Sie auf eine Zelle der Pivot-Tabelle und wählen Sie **Berechnetes Element** auf der Registerkarte **Pivot-Tabelle**. Wählen Sie das **Feld**. Es muss in den Zeilen oder den Spalten stehen. Geben Sie einen **Namen** und eine **Formel** ein. Schreiben Sie den Namen eines Elements in die Formel oder doppelklicken Sie ihn in der Liste **Elemente**, um ihn einzufügen. Einen Namen mit Leerzeichen setzen Sie in einfache Anführungszeichen, wie in `='Großes Ost'+West`. Sie können Zahlen, `+ - * / ^ %` und Klammern verwenden.

Das neue Element erhält eine Zeile im Bericht. In jeder Spalte zeigt es die Formel, berechnet aus den Zeilen der Elemente, die es nennt. Das neue Element zählt in den Summen mit. Eine Summe, etwa ein Gesamtergebnis oder ein Teilergebnis, addiert die Zeile des neuen Elements zu den Zeilen der anderen Elemente. Bei Ost mit 35, West mit 14 und Beide (Ost plus West) mit 49 ist das Gesamtergebnis also 98. Ist ein Element, das die Formel nennt, ausgeblendet, zeigt die Zelle **#BEZUG!**. Nennt die Formel ein Element, das nicht vorhanden ist, zeigt die Zelle **#NAME?**.

Um ein Element zu ändern, wählen Sie seinen Namen im Feld **Name**. Seine Formel wird angezeigt. Ändern Sie sie und wählen Sie **Ändern**. **Löschen** entfernt das Element und seine Zeile. Jeder Schritt ist ein Schritt für **Rückgängig**. Ein berechnetes Element wird mit der Datei gespeichert und bleibt beim Aktualisieren des Berichts erhalten. Zwei Pivot-Tabellen aus denselben Daten können keines haben, denn ein berechnetes Element ist ein Teil der Daten, die sie gemeinsam haben.

### Berechnungsreihenfolge

Manchmal hat eine Zelle zwei berechnete Elemente. Eines steht in ihrer Zeile, das andere in ihrer Spalte. Die Reihenfolge, in der sie berechnet werden, kann das Ergebnis verändern. Angenommen, das Zeilenelement ist `=Ost-West` und das Spaltenelement ist `=Jan+1`. In einer Reihenfolge ist die Zelle (Ost - West) + 1. In der anderen ist sie (Ost + 1) - (West + 1), und das ist eine andere Zahl.

**Berechnungsreihenfolge** auf der Registerkarte **Pivot-Tabelle** listet die berechneten Elemente in der Reihenfolge auf, in der sie berechnet werden. Das Element weiter unten in der Liste wird zuletzt berechnet. Wählen Sie ein Element und klicken Sie auf **Nach oben verschieben** oder **Nach unten verschieben**. Die Reihenfolge wird mit der Datei als Reihenfolge der Elemente gespeichert, und andere Programme lesen sie genauso. **Formeln auflisten** zeigt den Platz jedes Elements darin.

## Die Zeilen hinter einem Wert anzeigen

Doppelklicken Sie auf einen Wert der Pivot-Tabelle oder wählen Sie **Details anzeigen** auf der Registerkarte **Pivot-Tabelle**. JeffCrunch erstellt ein neues Blatt mit dem Namen Detail 1, Detail 2 und so weiter, mit den Zeilen der Liste, aus denen die Zahl besteht. Die Namen der Felder stehen in der ersten Zeile. F2 bearbeitet die Zelle weiterhin, sofern sie bearbeitet werden kann. Ein Schritt **Rückgängig** entfernt die Zeilen, ein weiterer das Blatt.

## Erweitern und reduzieren

Ein Feld, dessen Elemente andere darunter haben, kann die Zeilen unter seinen Elementen ein- oder ausblenden. **Gesamtes Feld erweitern** und **Gesamtes Feld reduzieren** tun dies für jedes Element des Feldes, das Sie im Bereich ausgewählt haben.

## Pivot-Tabellen-Optionen

**Optionen** auf der Registerkarte **Pivot-Tabelle** öffnet ein Feld mit den Einstellungen der Tabelle.

- **Name** ist der Name der Pivot-Tabelle. Keine andere Pivot-Tabelle der Arbeitsmappe darf ihn tragen.
- **Für Fehlerwerte anzeigen** und **Für leere Zellen anzeigen** setzen einen eigenen Text in diese Zellen.
- **Zellen mit Beschriftungen verbinden und zentrieren**, **Spaltenbreiten beim Aktualisieren der Tabelle anpassen** und **Zellenformatierung beim Aktualisieren der Tabelle beibehalten** werden mit der Tabelle gespeichert.
- **Gesamtergebnisse für Zeilen anzeigen** und **für Spalten anzeigen** entsprechen den Optionen für Gesamtergebnisse.
- **Daten beim Öffnen der Datei aktualisieren** liest die Zellen bei jedem Öffnen der Arbeitsmappe erneut. Es ist ein Schritt für Rückgängig, wie jede Aktualisierung.

## Formatvorlage: Kopfzeilen, Bänder und Katalog

Die **Formatvorlagenoptionen** auf der Registerkarte **Pivot-Tabelle** schalten die Kopfzeile, die erste Spalte (Zeilenüberschriften) und die Bänder für Zeilen und Spalten ein oder aus. **Formatvorlagen** hat einen Katalog mit Looks, und **Formatierung löschen** nimmt den Look weg. Diese Looks werden genauso gezeichnet wie die Looks von Tabellen. **Leere Zeilen** und **Beschriftungen wiederholen** setzen die Leerzeile und die wiederholten Beschriftungen für alle Felder der Zeilen auf einmal.

## Gestaltung: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** zeigt oder verbirgt die Summe jeder Gruppe, oben oder unten in der Gruppe. Gibt es mehr als ein Feld in den Zeilen, ist die Summe jedes äußeren Feldes ein Teilergebnis.
- **Gesamtergebnisse** hat vier Möglichkeiten: aus für Zeilen und Spalten, ein für beides, nur für Zeilen (eine Spalte mit Summen rechts, eine für jede Zeile) und nur für Spalten (eine Zeile mit Summen unten).
- **Berichtslayout** ist die Form der Beschriftungen. In der **kompakten** Form teilen sich alle Felder der Zeilen eine Spalte, und eine neue Gruppe wird etwas weiter rechts gezeigt. In der **Gliederungsform** hat jedes Feld seine eigene Spalte, und jede Gruppe hat eine eigene Zeile über ihren Elementen. In der **tabellarischen Form** hat jedes Feld seine eigene Spalte, und der Name der Gruppe steht in ihrer ersten Zeile, das Teilergebnis darunter.

## Dieselbe Liste, ein Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und über ihm erscheint eine Zeile mit **Monat** und **(Alle)**. Die Zahlen sind jetzt die Summe jeder Region über alle Monate.

## Pivot-Diagramm

Klicken Sie auf eine Zelle einer Pivot-Tabelle und wählen Sie **Einfügen**, dann **Pivot-Diagramm**. JeffCrunch erstellt ein Säulendiagramm der Werte der Tabelle. Es lässt die Gesamtergebnisse weg. Das Diagramm ist in der Datei als Diagramm der Pivot-Tabelle gekennzeichnet. Es liest die Zellen der Tabelle, daher ändern sich seine Zahlen, wenn Sie die Tabelle aktualisieren oder ihr Layout ändern. Bekommt die Tabelle weitere Zeilen oder Spalten, erstellen Sie das Diagramm neu, denn es behält die Zellen, aus denen es erstellt wurde. Ein Pivot-Diagramm aus einer Datei wird so angezeigt, wie es gespeichert wurde.

## Datenschnitte und Zeitachsen

Ein Datenschnitt ist eine Gruppe von Schaltflächen, die eine Pivot-Tabelle filtert, und eine Zeitachse ist ein Schieberegler, der dasselbe für Datumswerte tut.

Klicken Sie auf eine Zelle einer Pivot-Tabelle und dann auf **Datenschnitte** auf der Registerkarte Pivot-Tabelle. Der Bereich Datenschnitte öffnet sich und listet die Datenschnitte und Zeitachsen der Arbeitsmappe auf. Ein Datenschnitt hat eine Schaltfläche für jedes Element seines Feldes. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt.

Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen. Klicken Sie erneut darauf, um wieder alle Elemente anzuzeigen. Halten Sie **Strg** gedrückt und klicken Sie, um ein Element zu den angezeigten hinzuzufügen oder eines zu entfernen. Jeder Klick ist ein Schritt für Rückgängig, und Rückgängig stellt die Tabelle wieder her. Beim Speichern behält die Datei die Auswahl des Datenschnitts.

Ein Datenschnitt, der mit der Datei kommt, wird außerdem über den Zellen gezeichnet, dort, wo die Datei ihn platziert. Er zeigt seine Beschriftung und eine Schaltfläche für jedes Element. Die blauen Schaltflächen sind die Elemente, die die Tabelle anzeigt. Klicken Sie auf eine Schaltfläche, um nur dieses Element anzuzeigen, halten Sie **Strg** gedrückt und klicken Sie, um ein Element hinzuzufügen oder zu entfernen, und klicken Sie oben rechts auf den Trichter, um wieder alle Elemente anzuzeigen. Ein Klick ist ein Schritt für Rückgängig. Wird die Tabelle auf andere Weise gefiltert oder machen Sie Rückgängig, folgen die Schaltflächen. Ist ein Datenschnitt zu klein für alle seine Schaltflächen, zeigt die letzte Zeile „Weitere Elemente“ und wie viele es sind (und wie viele davon gewählt sind); klicken Sie auf diese Zeile, um den Bereich Datenschnitte zu öffnen, der jedes Element auflistet. Ein Datenschnitt, der eine Tabelle statt einer Pivot-Tabelle filtert oder dessen Pivot-Tabelle nicht in der Datei liegt, wird mit seiner Beschriftung und einem Satz gezeichnet, der sagt, dass JeffCrunch ihn noch nicht verwenden kann; er bleibt in der Datei unverändert erhalten. Eine Arbeitsmappe, die nur zum Lesen geöffnet ist, und ein Blatt, dessen Pivot-Tabellen gegen Änderungen geschützt sind, lassen einen Klick die Tabelle nicht filtern. Ein Datenschnitt wird mit dem Blatt gedruckt, sofern die Datei das nicht ausschließt. Datenschnitte auf dem Blatt lassen sich noch nicht verschieben, in der Größe ändern oder löschen.

**Beziehungen zu Berichten** zeigt, welche Pivot-Tabellen ein Datenschnitt filtert. Eine Zeitachse zeigt den Zeitraum, den sie gewählt hat. JeffCrunch zeichnet eine Zeitachse noch nicht über den Zellen und ändert sie nicht. Beides bleibt in der Datei so erhalten, wie es gespeichert wurde.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder haben, eine Liste mit bis zu fünf Millionen Zeilen und einen Bericht mit bis zu vier Millionen Zellen. Eine Datei, die darüber hinausgeht, wird so angezeigt, wie sie gespeichert wurde, und lässt sich nicht ändern. Das erneute Einlesen von hunderttausend Zeilen mit zwanzig Spalten und die Berechnung des Berichts dauerten auf dem Computer, mit dem dies gemessen wurde, etwa eine halbe Sekunde; ein langsamerer Computer braucht länger.
