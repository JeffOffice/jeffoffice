# Pivot-Tabellen
<!-- keywords: Pivot-Tabelle, Pivot-Tabelle einfügen, erstellen, PIVOTDATENZUORDNEN, GETPIVOTDATA, PIVOTDATENZUORDNEN generieren, aktualisieren, alle aktualisieren, Datenquelle ändern, Feldliste, Felder, Zeilen, Spalten, Werte, Filter, Berichtsfilter, Gesamtergebnis, Teilergebnis, Layout, Kurzformat, Gliederungsformat, Tabellenformat, löschen, Wertfeldeinstellungen, Summe, Anzahl, Mittelwert, Kreuztabelle, Auswertung -->

Eine Pivot-Tabelle ist ein kleiner Bericht, der aus einer langen Liste erstellt wird. Die Liste enthält zum Beispiel eine Zeile für jeden Verkauf; der Bericht addiert die Verkäufe nach Region und Monat. Sie schreiben keine Formeln. Sie legen fest, welche Spalten der Liste an der Seite stehen, welche oben quer verlaufen und welche addiert wird, und der Bericht füllt sich von selbst aus.

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

Eine Pivot-Tabelle mit **Region** an der Seite, **Monat** oben und der Summe von **Betrag** in der Mitte sieht so aus:

| Summe von Betrag | Monat | | |
|---|---|---|---|
| Region | Jan | Feb | Gesamtergebnis |
| Ost | 15 | 20 | 35 |
| West | 7 | 7 | 14 |
| Gesamtergebnis | 22 | 27 | 49 |

Ost im Januar ergibt 15, weil die beiden Ost-Zeilen für Januar, 10 und 5, addiert werden. Die Spalte „Gesamtergebnis“ addiert jede Zeile, die Zeile „Gesamtergebnis“ addiert jede Spalte, und die Zelle in der Ecke addiert alles.

Die hier verwendeten Begriffe:

- **Feld** ist eine Spalte der Liste. Region, Monat und Betrag sind die Felder.
- **Zeilen**, **Spalten** und **Werte** sind die drei Bereiche, in die ein Feld gesetzt werden kann. Ein Feld in „Zeilen“ ergibt eine Zeile für jeden seiner unterschiedlichen Einträge; ein Feld in „Spalten“ ergibt eine Spalte für jeden; ein Feld in „Werte“ wird addiert. Ein vierter Bereich, **Filter**, nimmt ein Feld auf, das die nicht ausgewählten Zeilen ausschließt.
- Ein **Cache** (der Pivot-Tabellen-Cache) ist eine Kopie Ihrer Liste, die die Pivot-Tabelle in der Datei speichert, damit sie Summen anzeigen kann, ohne die Zellen zu lesen. Der Bericht wird aus dem Cache berechnet, nicht aus den Zellen, und ändert sich daher nicht, wenn sich die Zellen ändern. **Aktualisieren** kopiert die Zellen erneut in den Cache.

## Pivot-Tabelle erstellen

1. Klicken Sie in eine Zelle der Liste, oder markieren Sie die Zellen, die Sie verwenden möchten. Die Liste benötigt über den Daten eine Zeile mit Namen.
2. Wählen Sie **Einfügen** und dann **Pivot-Tabelle**.
3. Das Feld zeigt die Zellen, in denen die Daten stehen. Ändern Sie sie, falls sie nicht stimmen. Sie können auch den Namen einer Tabelle eingeben.
4. Wählen Sie, wo die Pivot-Tabelle eingefügt wird: **Ein neues Blatt** (die Standardeinstellung) oder eine Zelle auf diesem oder einem anderen Blatt. Wählen Sie eine Stelle, an der sich sonst nichts befindet, denn die leere Tabelle belegt etwa achtzehn Zeilen und drei Spalten.
5. Wählen Sie **OK**. Die Tabelle ist zunächst leer, und der Bereich **Pivot-Tabellen-Felder** wird geöffnet. Aktivieren Sie die gewünschten Felder: Ein Feld mit Zahlen wird addiert, jedes andere Feld wird zu einer Zeile. Verschieben Sie die Felder dann wie in den folgenden Abschnitten beschrieben in die Bereiche.

Ein neues Blatt ist ein eigener Schritt für **Rückgängig**, ebenso die Tabelle. Wenn Sie als .xlsx speichern, wird die Tabelle so geschrieben, wie das Originalprogramm sie schreibt, sodass sie dort als Pivot-Tabelle geöffnet wird. Eine als .xls oder .ods gespeicherte Kopie behält die Zahlen, die die Tabelle in ihren Zellen anzeigt, aber nicht die Tabelle selbst; beim Speichern werden Sie darauf hingewiesen.

## PIVOTDATENZUORDNEN: eine Zahl aus einer Pivot-Tabelle lesen

`GETPIVOTDATA` gibt die Zahl zurück, die eine Pivot-Tabelle für den angegebenen Wert und die angegebenen Zeilen und Spalten anzeigt. Für das Beispiel oben:

`=GETPIVOTDATA("Summe von Betrag"; $E$3; "Region"; "Ost"; "Monat"; "Jan")` ergibt 15.

- Das erste Argument ist der Name des Werts, wie die Tabelle ihn anzeigt (zum Beispiel „Summe von Betrag“). Der Name des Felds, das addiert wird (Betrag), funktioniert ebenfalls.
- Das zweite Argument ist eine beliebige Zelle der Pivot-Tabelle.
- Danach folgen Paare aus einem Feld und einem seiner Elemente. Geben Sie nur Region, Ost an, erhalten Sie die Summe für Ost, 35. Geben Sie nichts an, erhalten Sie das Gesamtergebnis, 49. Die angegebenen Felder müssen die ersten Felder in den Zeilen oder die ersten Felder in den Spalten sein, in dieser Reihenfolge.
- Zeigt die Tabelle die Zahl nicht an, zeigt die Zelle `#BEZUG!`.

Wenn Sie beim Eingeben einer Formel auf einen Wert einer Pivot-Tabelle klicken, schreibt JeffCrunch diese Formel für Sie, damit sie auch nach dem Umordnen der Tabelle die richtige Zahl liest. Um dies auszuschalten, wählen Sie auf der Registerkarte **Pivot-Tabelle** die Option **Optionen** und deaktivieren das Kontrollkästchen für GETPIVOTDATA.

## Was JeffCrunch mit einer Pivot-Tabelle macht

Eine Pivot-Tabelle in einer Arbeitsmappe wird so angezeigt, wie sie gespeichert wurde. Wenn Sie nichts daran ändern, wird die Datei mit der Pivot-Tabelle genau so gespeichert, wie sie war. Wenn Sie sie aktualisieren oder ändern, berechnet JeffCrunch den Bericht neu, setzt die neuen Zahlen in die Zellen und schreibt die Pivot-Tabelle so zurück, dass das Programm, mit dem sie erstellt wurde, sie problemlos öffnet.

JeffCrunch berechnet Pivot-Tabellen, die addieren, zählen, Mittelwerte bilden usw. Eine Pivot-Tabelle, die Datumsangaben oder Zahlen gruppiert, ein berechnetes Feld oder Element enthält, Werte als Prozent, Differenz oder Rang anzeigt oder ein Datenmodell verwendet, wird wie gespeichert angezeigt und beibehalten; die Befehle weisen darauf hin, dass sie sie nicht ändern können.

Klicken Sie auf eine beliebige Zelle einer Pivot-Tabelle. Die Registerkarte **Pivot-Tabelle** wird angezeigt.

## Aktualisieren

**Aktualisieren** liest die Zellen, aus denen die Pivot-Tabelle erstellt wird, und berechnet den Bericht neu. Neue Einträge werden nach den vorhandenen eingefügt; nicht mehr vorhandene Einträge werden entfernt. Spaltenbreiten und das gewählte Layout bleiben erhalten. **Alle aktualisieren** auf der Registerkarte **Daten** und auf der Registerkarte **Pivot-Tabelle** macht dasselbe für jede Pivot-Tabelle der Arbeitsmappe.

Würde der Bericht in Zellen hineinwachsen, die Inhalt haben, aktualisiert JeffCrunch nicht. Es zeigt an, welche Zellen im Weg sind. Leeren Sie sie oder verschieben Sie die Pivot-Tabelle, und aktualisieren Sie erneut.

**Rückgängig** nimmt eine Aktualisierung in einem Schritt zurück.

## Datenquelle ändern

**Datenquelle ändern** fragt nach den Zellen der Liste, einschließlich ihrer ersten Zeile mit Namen. Geben Sie sie wie `Tabelle1!A1:D100` ein, oder geben Sie den Namen einer Tabelle oder einen selbst definierten Namen ein. JeffCrunch liest sie und berechnet den Bericht neu. Hat ein Feld der Liste in den neuen Zellen denselben Namen, behält es seine Position im Bericht.

## Löschen

**Löschen** bietet zwei Optionen. **Filter löschen** entfernt alle Filter und zeigt wieder alle Einträge an. **Alle löschen** entfernt alle Felder aus dem Bericht und lässt ihn leer, bereit für einen Neuaufbau.

## Der Bereich „Pivot-Tabellen-Felder“

**Feldliste** auf der Registerkarte **Pivot-Tabelle** öffnet den Bereich **Pivot-Tabellen-Felder**. Er listet die Felder der Liste jeweils mit einem Kontrollkästchen auf und darunter die vier Bereiche: **Filter**, **Spalten**, **Zeilen** und **Werte**.

- Aktivieren Sie ein Feld, um es hinzuzufügen. Ein Feld, das nur Zahlen enthält, kommt in **Werte** und wird addiert; jedes andere kommt in **Zeilen**.
- Deaktivieren Sie ein Feld, um es überall zu entfernen.
- Klicken Sie auf ein Feld in einem der vier Bereiche, und wählen Sie **Verschieben nach**, um es in einen anderen zu setzen. **Nach oben** und **Nach unten** ändern seine Reihenfolge im Bereich, und **Feld entfernen** entfernt es.
- Bei mehr als einem Wert erscheint in **Spalten** ein Eintrag namens **Werte**. Verschieben Sie ihn in **Zeilen**, um die Werte an der Seite anzuzeigen.
- **Wertfeldeinstellungen** (Doppelklick auf einen Wert oder die Schaltfläche) legt den Namen fest, den der Bericht für den Wert anzeigt, und wie er zusammengefasst wird: **Summe**, **Anzahl**, **Anzahl Zahlen**, **Mittelwert**, **Max**, **Min**, **Produkt**, **Standardabweichung (Stichprobe)** und **Standardabweichung (Grundgesamtheit)** sowie **Varianz (Stichprobe)** und **Varianz (Grundgesamtheit)**.

Jede Änderung ist ein Schritt für **Rückgängig**.

## Entwurf: Teilergebnisse, Gesamtergebnisse und Layout

- **Teilergebnisse** zeigt die Summe jeder Gruppe oben oder unten in der Gruppe an oder blendet sie aus. Bei mehr als einem Feld in „Zeilen“ ist die Summe jedes äußeren Felds ein Teilergebnis.
- **Gesamtergebnisse** bietet vier Optionen: für Zeilen und Spalten deaktiviert, für beide aktiviert, nur für Zeilen aktiviert (eine Ergebnisspalte rechts, eine Summe pro Zeile) und nur für Spalten aktiviert (eine Ergebniszeile unten).
- **Berichtslayout** bestimmt die Form der Beschriftungen. Im **Kurzformat** teilen sich alle Felder in „Zeilen“ eine Spalte, und eine neue Gruppe wird etwas nach rechts eingerückt. Im **Gliederungsformat** hat jedes Feld eine eigene Spalte, und jede Gruppe hat über ihren Elementen eine eigene Zeile. Im **Tabellenformat** hat jedes Feld eine eigene Spalte, der Gruppenname steht in ihrer ersten Zeile, und das Teilergebnis steht unter der Gruppe.

## Dieselbe Liste mit Filter

Verschieben Sie **Monat** von **Spalten** nach **Filter**. Der Bericht verliert seine Spalten, und eine Zeile darüber zeigt **Monat** und **(Alles)**. Die Zahlen sind nun die Summe jeder Region über alle Monate.

## Grenzen

Eine Pivot-Tabelle kann bis zu 2.000 Felder, eine Liste mit bis zu fünf Millionen Zeilen und einen Bericht mit bis zu vier Millionen Zellen haben. Eine Datei, die diese Grenzen überschreitet, wird wie gespeichert angezeigt und kann nicht geändert werden. Das erneute Einlesen von hunderttausend Zeilen mit zwanzig Spalten und das Berechnen des Berichts dauerte auf dem Messcomputer etwa eine halbe Sekunde; ein langsamerer Computer benötigt länger.
