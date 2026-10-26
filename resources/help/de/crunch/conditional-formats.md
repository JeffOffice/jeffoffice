# Bedingte Formatierungen
<!-- keywords: bedingte Formatierung, bedingtes Format, Regel, hervorheben, Hervorhebung, Farbskala, Datenbalken, Symbolsatz, Symbole, obere, untere, über dem Durchschnitt, unter dem Durchschnitt, doppelt, Duplikate, eindeutig, Formel, Anhalten, wenn wahr, Priorität, Balken, Pfeile, Ampel, Farbverlauf, differenzielles Format -->

Eine **bedingte Formatierung** ist eine Regel, die das Aussehen einer Zelle ändert, wenn ihr Inhalt eine Bedingung erfüllt. Beispiel: Eine Regel lautet „Zelle rot färben, wenn der Wert kleiner als 100 ist“. Eine Zelle mit 80 ist rot, eine Zelle mit 120 nicht, und wenn Sie 80 in 150 ändern, verschwindet das Rot von selbst. Das eigene Format der Zelle bleibt unverändert: Die Regel bestimmt nur, was angezeigt wird.

## Was JeffCrunch anzeigt

JeffCrunch zeigt die Regeln an, die eine Arbeitsmappe enthält:

- **Hervorhebungsregeln**: Eine Regel, die Schriftart, Füllung, Rahmen oder Zahlenformat der Zellen ändert, die sie erfüllen. Die Bedingung kann ein Vergleich mit einem Wert sein (größer als, zwischen, gleich usw.), eine Formel, Text, der bestimmte Wörter enthält, mit ihnen beginnt oder endet, ein Datum wie „letzte Woche“, leere Zellen oder Zellen mit Fehlern, Werte, die zu den größten oder kleinsten gehören (als Anzahl oder in Prozent), Werte über oder unter dem Durchschnitt sowie doppelte oder eindeutige Werte.
- **Farbskalen**: Jede Zelle wird mit einer Farbe gefüllt, die je nach Zahl von niedrig bis hoch zwischen zwei oder drei Farben wechselt. Bei einer Skala von Weiß nach Grün sind die größten Zahlen am grünsten.
- **Datenbalken**: Ein Balken in der Zelle, der umso länger ist, je größer die Zahl ist. Balken können einfarbig sein oder einen Farbverlauf haben, einen Rahmen und eine Achse besitzen und bei negativen Zahlen in die andere Richtung und in einer anderen Farbe verlaufen.
- **Symbolsätze**: Ein kleines Bild in der Zelle zeigt, in welchem Teil des Bereichs die Zahl liegt, zum Beispiel ein Pfeil nach oben, zur Seite oder nach unten oder eine rote, gelbe oder grüne Ampel. Es gibt zwanzig Sätze.

Wenn zwei Regeln dieselbe Zelle betreffen, setzt sich bei den Eigenschaften, die beide festlegen, die Regel durch, die in der Reihenfolge der Arbeitsmappe zuerst kommt. Eine Regel mit der Option „Anhalten, wenn wahr“ beendet die Suche. Die Zahlen einer Farbskala, eines Datenbalkens oder eines Symbolsatzes werden über die Zellen der jeweiligen Regel ermittelt und neu berechnet, sobald sich eine dieser Zellen ändert.

## Was erhalten bleibt

- Regeln bleiben bei den Zellen, zu denen sie gehören. Wenn Sie Zeilen oder Spalten einfügen oder löschen oder Inhalte ausschneiden und einfügen, gelten die Regeln weiterhin für die richtigen Zellen; mit **Rückgängig** werden sie wiederhergestellt.
- Eine Regel, die JeffCrunch nicht darstellt, bleibt unverändert in der Datei erhalten.
- Beim Speichern werden die Regeln der Arbeitsmappe unverändert geschrieben.

## Was noch nicht möglich ist

**Sie können noch keine Regeln erstellen, ändern oder löschen.** JeffCrunch hat keinen Befehl **Bedingte Formatierung**: Das Programm zeigt an, was die Arbeitsmappe bereits enthält, und erhält es bei Ihren Bearbeitungen korrekt. Das Bearbeiten von Regeln folgt in einer späteren Version. Bis dahin erstellen oder ändern Sie Regeln in dem Programm, mit dem die Arbeitsmappe erstellt wurde.
