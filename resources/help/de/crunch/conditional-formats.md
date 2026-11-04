# Bedingte Formatierungen
<!-- keywords: bedingte Formatierung, bedingte Formatierungen, Regel, hervorheben, Farbskala, Datenbalken, Symbolsatz, Symbole, oberste, unterste, über dem Durchschnitt, unter dem Durchschnitt, Duplikat, eindeutig, Formel, Stopp, wenn wahr, Priorität, Balken, Pfeile, Ampel, Verlauf, Differenzformat, Schaltfläche Bedingte Formatierung, Regeln löschen, größer als, kleiner als, zwischen, Text, der enthält, doppelte Werte, Top 10 -->

Eine **bedingte Formatierung** ist eine Regel, die ändert, wie eine Zelle aussieht, wenn ihr Inhalt eine Bedingung erfüllt. Beispiel: Eine Regel sagt „Zelle rot färben, wenn der Wert kleiner als 100 ist“. Eine Zelle mit 80 ist rot, eine Zelle mit 120 nicht, und wenn Sie 80 in 150 ändern, verschwindet die Rotfärbung von selbst. Die eigene Formatierung der Zelle bleibt unverändert: Die Regel bestimmt nur, was angezeigt wird.

## Was JeffCrunch anzeigt

JeffCrunch zeigt die Regeln an, die eine Arbeitsmappe enthält:

- **Regeln zum Hervorheben**: Eine Regel, die Schriftart, Füllung, Rahmen oder Zahlenformat der Zellen ändert, auf die sie zutrifft. Die Bedingung kann ein Vergleich mit einem Wert (größer als, zwischen, gleich usw.), eine Formel, ein Text, der bestimmte Wörter enthält, beginnt oder endet, ein Datum wie „letzte Woche“, leere Zellen oder Zellen mit Fehlern, Werte unter den höchsten oder niedrigsten (eine Zahl oder ein Prozentsatz), Werte über oder unter dem Durchschnitt sowie doppelte oder eindeutige Werte sein.
- **Farbskalen**: Jede Zelle wird mit einer Farbe gefüllt, die sich zwischen zwei oder drei Farben bewegt, je nachdem, wie ihre Zahl von niedrig nach hoch liegt. Eine Skala von Weiß nach Grün macht die größten Zahlen am grünsten.
- **Datenbalken**: Ein Balken in der Zelle, der umso länger ist, je größer die Zahl ist. Balken können einfarbig sein oder auslaufen, können einen Rahmen und eine Achse haben und laufen bei negativen Zahlen in die andere Richtung, in einer anderen Farbe.
- **Symbolsätze**: Ein kleines Bild in der Zelle, das angibt, in welchem Bereich die Zahl liegt, etwa ein Aufwärtspfeil, ein seitlicher Pfeil oder ein Abwärtspfeil, oder eine rote, gelbe oder grüne Ampel. Es gibt zwanzig Sätze.

Wenn zwei Regeln für dieselbe Zelle etwas festlegen, gewinnt bei gemeinsam festgelegten Eigenschaften die Regel, die in der Reihenfolge der Arbeitsmappe zuerst kommt. Eine Regel mit der Option „Stopp, wenn wahr“ beendet die Suche. Die Werte einer Farbskala, eines Datenbalkens oder eines Symbolsatzes werden über die Zellen der Regel ermittelt und neu berechnet, wenn sich eine dieser Zellen ändert.

## Was gleich bleibt

- Regeln folgen den Zellen, zu denen sie gehören. Wenn Sie Zeilen oder Spalten einfügen oder löschen, ausschneiden und einfügen, bleiben die Regeln bei den richtigen Zellen; mit **Rückgängig** werden sie wiederhergestellt.
- Eine Regel, die JeffCrunch nicht darstellt, bleibt unverändert in der Datei erhalten.
- Beim Speichern werden die Regeln der Arbeitsmappe geschrieben. Wenn Sie Regeln auf einem Blatt erstellen oder löschen, werden die Regeln dieses Blatts mit Ihren Änderungen erneut geschrieben.

## Regel erstellen

1. Markieren Sie die Zellen.
2. Klicken Sie auf **Start > Bedingte Formatierung**.
3. Wählen Sie unter **Regeln zum Hervorheben von Zellen** die Option **Größer als**, **Kleiner als**, **Zwischen**, **Gleich**, **Text, der enthält** oder **Doppelte Werte**. Wählen Sie unter **Oberste/Unterste-Regeln** die Option **Oberste 10 Elemente**, **Unterste 10 Elemente**, **Oberste 10 %**, **Unterste 10 %**, **Überdurchschnittlich** oder **Unterdurchschnittlich**.
4. Geben Sie den Wert ein (bei **Zwischen** die beiden Werte, bei **Text, der enthält** den Text, bei den Oberste- und Unterste-Regeln die Anzahl). Bei **Oberste 10 Elemente** gibt „10“ an, wie viele Zellen die Formatierung erhalten. Sie können die Zahl beliebig ändern.
5. Wählen Sie die gewünschte Darstellung, etwa **Helle rote Füllung mit dunkelroter Schrift**, und klicken Sie auf **OK**.

Die Regel wird sofort angezeigt. Jede neue Regel steht an erster Stelle; wenn zwei Regeln dieselbe Eigenschaft ändern wollen, gewinnt daher die neuere. Das Erstellen einer Regel ist ein Schritt, den Sie rückgängig machen können.

## Regeln löschen

**Regeln aus ausgewählten Zellen entfernen** löscht alle Regeln, die auf den markierten Zellen liegen. **Regeln aus diesem Blatt entfernen** löscht alle Regeln des Blatts. Jeder Befehl ist ein Schritt, den Sie rückgängig machen können. Ein Befehl hat keine Wirkung, wenn keine Regel zum Löschen vorhanden ist.

## Was noch nicht möglich ist

- Datenbalken, Farbskalen und Symbolsätze können angezeigt, aber noch nicht erstellt werden. Das Fenster, in dem alle Regeln aufgelistet, sortiert und bearbeitet werden (**Regeln verwalten**), ist noch nicht verfügbar. Eine Regel mit einer Formel kann noch nicht über das Menü erstellt werden.
- Ein Blatt mit Datenbalken aus einer neueren Dateiversion behält diese unverändert bei; JeffCrunch erstellt oder löscht auf diesem Blatt keine Regeln. Nehmen Sie diese Änderungen in dem Programm vor, mit dem die Arbeitsmappe erstellt wurde.
- Eine Arbeitsmappe, die aus einem älteren Dateiformat geöffnet wurde, behält ihre Regeln, wenn Sie sie als .xlsx-Arbeitsmappe speichern; danach können Sie Regeln hinzufügen.
