# Bedingte Formatierung
<!-- keywords: bedingte Formatierung, bedingte Formatierungen, Regel, hervorheben, Hervorhebung, Farbskala, Datenbalken, Symbolsatz, Symbole, Icons, Top, Bottom, überdurchschnittlich, unterdurchschnittlich, Duplikat, doppelt, eindeutig, Formel, Stopp wenn zutreffend, Priorität, Balken, Pfeile, Ampeln, Verlauf, Differenzformat, Schaltfläche Bedingte Formatierung, Regeln löschen, größer als, kleiner als, zwischen, Text enthält, doppelte Werte, Top 10 -->

Eine **bedingte Formatierung** ist eine Regel, die ändert, wie eine Zelle aussieht, wenn ihr Inhalt eine Bedingung erfüllt. Beispiel: Eine Regel lautet „Zelle rot färben, wenn der Wert unter 100 liegt“. Eine Zelle mit 80 ist dann rot, eine Zelle mit 120 nicht. Ändern Sie 80 auf 150, verschwindet das Rot von selbst. Die eigentliche Formatierung der Zelle bleibt unverändert: Die Regel bestimmt nur, was angezeigt wird.

## Was JeffCrunch anzeigt

JeffCrunch zeigt die Regeln einer Arbeitsmappe an:

- **Regeln zum Hervorheben**: Eine Regel, die Schrift, Füllung, Rahmen oder Zahlenformat der Zellen ändert, auf die sie zutrifft. Die Bedingung kann ein Vergleich mit einem Wert sein (größer als, zwischen, gleich usw.), eine Formel, ein Text, der bestimmte Wörter enthält, beginnt oder endet, ein Datum wie „letzte Woche“, leere Zellen oder Zellen mit Fehlern, Werte unter den höchsten oder niedrigsten (eine Zahl oder ein Prozentsatz), Werte über oder unter dem Durchschnitt sowie doppelte oder eindeutige Werte.
- **Farbskalen**: Jede Zelle wird mit einer Farbe gefüllt, die zwischen zwei oder drei Farben wechselt, je nachdem, wie groß die Zahl im Verhältnis zu den anderen ist. Eine Skala von Weiß nach Grün macht die größten Zahlen am grünsten.
- **Datenbalken**: Ein Balken in der Zelle, der umso länger ist, je größer die Zahl ist. Balken können einfarbig sein oder verblassen, einen Rahmen und eine Achse haben. Bei negativen Zahlen können sie in die andere Richtung und in einer anderen Farbe laufen.
- **Symbolsätze**: Ein kleines Symbol in der Zelle, das zeigt, in welchem Teil des Bereichs die Zahl liegt, etwa ein Pfeil nach oben, ein Pfeil zur Seite oder ein Pfeil nach unten, oder eine rote, gelbe oder grüne Ampel. Es gibt zwanzig Sätze.

Wenn zwei Regeln dieselbe Zelle betreffen, gewinnt bei allem, was beide festlegen, die Regel, die in der Reihenfolge der Arbeitsmappe zuerst kommt. Eine Regel, die als „Stopp, wenn zutreffend“ markiert ist, beendet die Suche. Die Werte, auf deren Grundlage eine Farbskala, ein Datenbalken oder ein Symbolsatz berechnet wird, beziehen sich auf die Zellen der jeweiligen Regel und werden neu berechnet, wenn sich eine dieser Zellen ändert.

## Was gleich bleibt

- Regeln folgen den Zellen, zu denen sie gehören. Wenn Sie Zeilen oder Spalten einfügen oder löschen oder Zellen ausschneiden und einfügen, decken die Regeln weiterhin die richtigen Zellen ab. Mit **Rückgängig** stellen Sie den vorherigen Zustand wieder her.
- Eine Regel, die JeffCrunch nicht darstellt, bleibt unverändert in der Datei erhalten.
- Beim Speichern werden die Regeln der Arbeitsmappe geschrieben. Wenn Sie auf einem Blatt Regeln erstellen oder entfernen, werden die Regeln des Blatts mit Ihren Änderungen neu geschrieben.

## Regel erstellen

1. Markieren Sie die Zellen. Sie können ganze Spalten oder das gesamte Blatt markieren. Eine Regel gilt für den Bereich und nicht für jede einzelne Zelle.
2. Klicken Sie auf **Start > Bedingte Formatierung**.
3. Wählen Sie unter **Regeln zum Hervorheben von Zellen** die Option **Größer als**, **Kleiner als**, **Zwischen**, **Gleich**, **Textinhalt**, **Datum auftritt** oder **Doppelte Werte**. Wählen Sie unter **Obere/untere Regeln** die Option **Top-10-Elemente**, **Bottom-10-Elemente**, **Top 10 %**, **Bottom 10 %**, **Überdurchschnittlich** oder **Unterdurchschnittlich**.
4. Geben Sie den Wert ein (bei **Zwischen** die beiden Werte, bei **Text enthält** den Text, bei den Top- und Bottom-Regeln die Anzahl) oder wählen Sie die Datumswerte aus. Bei **Top-10-Elemente** bestimmt „10“, wie viele Zellen die Formatierung erhalten. Sie können die Zahl beliebig ändern.
5. Wählen Sie aus, wie die Zellen aussehen sollen, zum Beispiel **Helle rote Füllung mit dunkelroter Schrift**, und klicken Sie auf **OK**. Das Beispiel neben der Liste zeigt die Formatierung.

Um eine eigene Formatierung zu erstellen, wählen Sie am Ende der Liste **Benutzerdefiniertes Format...**. Es öffnet sich ein Dialogfeld mit vier Registerkarten: **Zahl**, **Schriftart** (Schriftschnitt, Unterstreichung, Durchstreichung und Farbe), **Rahmen** (nur dünne Linienarten) und **Füllung** (eine Farbe oder ein Muster mit zwei Farben). Das Dialogfeld beginnt mit der zuvor gewählten Formatierung. Was Sie nicht ändern, bleibt unverändert. Klicken Sie auf **OK**, dann zeigt das Beispiel Ihre Formatierung.

Die Regel wird sofort angewendet. Jede neue Regel kommt an den Anfang der Reihenfolge, wenn also zwei Regeln dasselbe ändern wollen, gewinnt die neuere. Das Erstellen einer Regel ist ein Schritt, den Sie rückgängig machen können.

## Datenbalken, Farbskalen und Symbolsätze

Klicken Sie auf **Start > Bedingte Formatierung** und dann auf **Datenbalken**, **Farbskalen** oder **Symbolsätze** und wählen Sie eine Variante. Die Regel wird sofort auf die markierten Zellen angewendet, ohne dass ein Dialogfeld ausgefüllt werden muss. Der Vorgang lässt sich mit **Rückgängig** zurücknehmen.

- **Datenbalken** gibt es in sechs Farben, jeweils als Farbverlauf oder als einfarbige Füllung.
- **Farbskalen** bietet zwölf Varianten mit zwei oder drei Farben. Der Name nennt zuerst die Farbe der größten Zahlen.
- **Symbolsätze** bietet zwanzig Sätze: Richtungen (Pfeile und Dreiecke), Formen (Ampeln und Schilder), Indikatoren (Symbole und Flaggen) und Bewertungen (Sterne, Viertel und Kästchen). Die Symbole teilen den Bereich in gleich große Stufen auf.

**Neue Regel** am Ende des Menüs öffnet ein Dialogfeld für alle Arten von Regeln, einschließlich einer Regel mit eigener Formel sowie eines Datenbalkens, einer Farbskala oder eines Symbolsatzes mit eigenen Einstellungen.

## Regeln verwalten

**Start > Bedingte Formatierung > Regeln verwalten** zeigt die Regeln des Blatts in der Reihenfolge, in der sie wirken. Verwenden Sie **Neue Regel**, **Regel bearbeiten** und **Regel löschen**. Mit **Nach oben verschieben** und **Nach unten verschieben** ändern Sie die Reihenfolge. Unter **Anwenden auf** geben Sie andere Zellen ein, und mit dem Kontrollkästchen **Stopp, wenn zutreffend** legen Sie das Beenden fest. Mit **Regel bearbeiten** ändern Sie alles, was eine Regel festlegt:

- bei einem **Datenbalken**: wofür der kürzeste und der längste Balken stehen (niedrigster oder höchster Wert, eine Zahl, ein Prozentsatz, ein Perzentil oder eine Formel), Farbverlauf oder einfarbige Füllung, die Farben, ein Rahmen, die Farbe negativer Balken, die Position der Achse, die Laufrichtung der Balken und **Nur Balken anzeigen**;
- bei einer **Farbskala**: zwei oder drei Farben und wofür jede Farbe steht;
- bei einem **Symbolsatz**: der Symbolstil, der Wert, bei dem jedes Symbol beginnt, **Symbolreihenfolge umkehren** und **Nur Symbol anzeigen**;
- bei den übrigen Regeln: der Wert, Text, Datum oder die Formel sowie die Formatierung. Eine Regel, die eine Datei mit eigener Formatierung mitgebracht hat, wird mit der Formatierung **Benutzerdefiniertes Format** aufgeführt, und das Beispiel zeigt, wie sie tatsächlich aussieht. Ändern Sie etwas anderes an der Regel, bleibt die Formatierung genau so, wie die Datei sie enthielt. Wählen Sie **Benutzerdefiniertes Format...**, um die Formatierung zu ändern.

Mit **OK** wenden Sie alle Ihre Änderungen als einen Schritt an, den Sie rückgängig machen können. Eine Regel, die Sie nicht geändert haben, behält genau das, was die Datei enthielt, unabhängig von ihrer Position in der Liste. Eine geänderte Regel wird neu geschrieben. Die Formel einer Regel wird aus der oberen linken Zelle ihres Bereichs gelesen. Eine Formel, die Sie für eine neue Regel eingeben, wird so gelesen, als stünde sie in der Zelle mit dem Cursor. Ein geschütztes Blatt, auf dem das Formatieren von Zellen nicht zugelassen ist, übernimmt keine Regeln.

## Was andere Dateiformate behalten

Nur eine .xlsx-Arbeitsmappe behält bedingte Formatierungen. Wenn Sie in ein älteres Format (.xls oder .ods) speichern, gehen sie verloren. **Kompatibilität prüfen** weist darauf hin und nennt Datenbalken, Farbskalen und Symbolsätze, sofern das Blatt sie enthält. Eine .csv-Datei behält keinerlei Formatierungen. Bedingte Formatierungen werden so gedruckt, wie sie auf dem Bildschirm aussehen.

## Regeln löschen

**Regeln aus ausgewählten Zellen entfernen** entfernt alle Regeln, die auf einer der markierten Zellen liegen. **Regeln aus diesem Blatt entfernen** entfernt alle Regeln des Blatts. Jeder Vorgang ist ein Schritt, den Sie mit **Rückgängig** zurücknehmen können. Ist keine Regel zum Entfernen vorhanden, hat die Auswahl keine Wirkung.

## Was noch nicht unterstützt wird

- Ein Blatt mit Datenbalken aus einer neueren Dateiversion behält sie unverändert bei. JeffCrunch kann auf diesem Blatt keine Regeln erstellen, ändern oder entfernen. Nehmen Sie solche Änderungen in dem Programm vor, mit dem die Arbeitsmappe erstellt wurde.
- Eine Arbeitsmappe, die aus einem älteren Dateiformat geöffnet wurde, behält ihre Regeln, wenn Sie sie als .xlsx-Arbeitsmappe speichern, und Sie können danach Regeln hinzufügen.
- Eine eigene Formatierung kann keine mittlere, dicke, doppelte oder schräge Linie, keine diagonale Linie, keine Farbverlaufsfüllung und keine Schriftart oder Schriftgröße enthalten. Diese Merkmale können Regeln nicht ändern.
