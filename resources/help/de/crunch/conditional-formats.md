# Bedingte Formatierungen
<!-- keywords: bedingte Formatierung, bedingte Formatierungen, Regel, Regeln, hervorheben, Hervorhebung, Farbskala, Datenbalken, Symbolsatz, Symbole, Icons, oberste, unterste, überdurchschnittlich, unterdurchschnittlich, Duplikate, doppelte Werte, eindeutig, Formel, Stopp, wenn zutreffend, Priorität, Balken, Pfeile, Ampeln, Verlauf, Differenzformat, Schaltfläche Bedingte Formatierung, Regeln löschen, größer als, kleiner als, zwischen, Text, der enthält, Top 10 -->

Eine **bedingte Formatierung** ist eine Regel, die ändert, wie eine Zelle aussieht, wenn ihr Inhalt eine Bedingung erfüllt. Beispiel: Eine Regel legt fest: „Färbe eine Zelle rot, wenn sie einen Wert unter 100 enthält.“ Eine Zelle mit 80 ist dann rot, eine Zelle mit 120 nicht. Ändern Sie 80 in 150, verschwindet das Rot von selbst. Das eigentliche Zellformat wird nicht verändert: Die Regel bestimmt nur, was angezeigt wird.

## Was JeffCrunch anzeigt

JeffCrunch zeigt die Regeln an, die eine Arbeitsmappe enthält:

- **Regeln zum Hervorheben**: Eine Regel, die die Schrift, die Füllung, den Rahmen oder das Zahlenformat der Zellen ändert, auf die sie zutrifft. Die Bedingung kann ein Vergleich mit einem Wert sein (größer als, zwischen, gleich und so weiter), eine Formel, Text, der einige Wörter enthält, mit ihnen beginnt oder endet, ein Datum wie „letzte Woche“, leere Zellen oder Zellen mit einem Fehler, Werte, die zu den höchsten oder niedrigsten gehören (eine Zahl oder ein Prozentsatz), Werte über oder unter dem Durchschnitt sowie doppelte oder eindeutige Werte.
- **Farbskalen**: Jede Zelle wird mit einer Farbe gefüllt, die zwischen zwei oder drei Farben wechselt, je nachdem, wie groß ihre Zahl im Verhältnis zu den anderen ist. Eine Skala von Weiß nach Grün macht die größten Zahlen am grünsten.
- **Datenbalken**: Ein Balken in der Zelle, der umso länger ist, je größer die Zahl ist. Balken können einfarbig oder mit Verlauf dargestellt werden, können einen Rahmen und eine Achse haben und bei negativen Zahlen in eine andere Richtung und in einer anderen Farbe verlaufen.
- **Symbolsätze**: Ein kleines Symbol in der Zelle, das angibt, in welchem Bereich der Wert liegt, etwa ein Pfeil nach oben, ein seitlich zeigender Pfeil oder ein Pfeil nach unten, oder eine rote, gelbe oder grüne Ampel. Es gibt zwanzig Sätze.

Beziehen sich zwei Regeln auf dieselbe Zelle, gewinnt für das, was beide festlegen, die Regel, die in der Reihenfolge der Arbeitsmappe zuerst kommt. Eine Regel, die als „Stopp, wenn zutreffend“ markiert ist, beendet die Suche. Die Werte einer Farbskala, eines Datenbalkens oder eines Symbolsatzes werden über die Zellen der Regel gezählt und neu berechnet, sobald sich eine dieser Zellen ändert.

## Was gleich bleibt

- Regeln folgen den Zellen, zu denen sie gehören. Fügen Sie Zeilen oder Spalten ein oder löschen Sie sie, schneiden Sie aus oder fügen Sie ein: Die Regeln decken weiterhin die richtigen Zellen ab. **Rückgängig** stellt sie wieder her.
- Eine Regel, die JeffCrunch nicht darstellt, bleibt unverändert in der Datei erhalten.
- Beim Speichern werden die Regeln der Arbeitsmappe geschrieben. Wenn Sie auf einem Blatt Regeln erstellen oder löschen, werden die Regeln dieses Blatts mit Ihren Änderungen neu geschrieben.

## Regel erstellen

1. Markieren Sie die Zellen. Sie können ganze Spalten oder das ganze Blatt markieren; eine Regel gilt für den Bereich, nicht für jede Zelle einzeln.
2. Klicken Sie auf **Start > Bedingte Formatierung**.
3. Wählen Sie unter **Regeln zum Hervorheben von Zellen** die Option **Größer als**, **Kleiner als**, **Zwischen**, **Gleich**, **Text, der enthält**, **Ein Datum, das auftritt** oder **Doppelte Werte**. Wählen Sie unter **Regeln für obere/untere Werte** die Option **Oberste 10 Elemente**, **Unterste 10 Elemente**, **Oberste 10 %**, **Unterste 10 %**, **Überdurchschnittlich** oder **Unterdurchschnittlich**.
4. Geben Sie den Wert ein (bei **Zwischen** die beiden Werte, bei **Text, der enthält** den Text, bei den Regeln für obere und untere Werte die Anzahl) oder wählen Sie die Datumsangaben aus. Bei **Oberste 10 Elemente** gibt „10“ an, wie viele Zellen das Aussehen erhalten. Sie können die Zahl auf einen beliebigen Wert ändern.
5. Wählen Sie das gewünschte Aussehen, etwa **Hellrote Füllung mit dunkelroter Schrift**, und klicken Sie auf **OK**. Das Beispiel neben der Liste zeigt das Aussehen.

Um ein eigenes Aussehen festzulegen, wählen Sie am Ende der Liste **Benutzerdefiniertes Format...**. Es öffnet sich ein Fenster mit vier Registerkarten: **Zahl**, **Schrift** (Schriftschnitt, Unterstreichung, Durchstreichung und Farbe), **Rahmen** (nur dünne Linienarten) und **Füllung** (eine Farbe oder ein Muster mit zwei Farben). Das Fenster beginnt mit dem Aussehen, das Sie zuvor gewählt haben; Teile, die Sie nicht ändern, bleiben unverändert. Klicken Sie auf **OK**, dann zeigt das Beispiel Ihr Aussehen.

Die Regel wird sofort angewendet. Jede neue Regel steht an erster Stelle, daher gewinnt bei zwei Regeln, die dasselbe ändern wollen, die neuere. Das Erstellen einer Regel ist ein Schritt, den Sie rückgängig machen können.

## Datenbalken, Farbskalen und Symbolsätze

Klicken Sie auf **Start > Bedingte Formatierung** und dann auf **Datenbalken**, **Farbskalen** oder **Symbolsätze** und wählen Sie eine Möglichkeit aus. Die Regel wird sofort auf die markierten Zellen angewendet, ohne dass ein Fenster ausgefüllt werden muss. Das ist ein Schritt, den Sie rückgängig machen können.

- **Datenbalken** gibt es in sechs Farben, jeweils als Farbverlauf oder als einfarbige Füllung.
- **Farbskalen** gibt es in zwölf Varianten mit zwei oder drei Farben. Der Name nennt zuerst die Farbe der höchsten Zahlen.
- **Symbolsätze** bietet zwanzig Sätze: Richtungen (Pfeile und Dreiecke), Formen (Ampeln und Zeichen), Indikatoren (Symbole und Flaggen) und Bewertungen (Sterne, Viertel und Kästchen). Die Symbole teilen den Bereich in gleich große Stufen auf.

Der Befehl **Neue Regel** am Ende des Menüs öffnet ein Fenster für alle Arten von Regeln, auch für eine Regel mit einer eigenen Formel, sowie für einen Datenbalken, eine Farbskala oder einen Symbolsatz mit eigenen Einstellungen.

## Regeln verwalten

**Start > Bedingte Formatierung > Regeln verwalten** listet die Regeln des Blatts in der Reihenfolge auf, in der sie wirken. Verwenden Sie **Neue Regel**, **Regel bearbeiten** und **Regel löschen**. Mit **Nach oben verschieben** und **Nach unten verschieben** ändern Sie die Reihenfolge. Unter **Gilt für** geben Sie andere Zellen ein, und mit **Stopp, wenn zutreffend** setzen Sie ein Häkchen. Mit **Regel bearbeiten** ändern Sie alles, was eine Regel festlegt:

- bei einem **Datenbalken**: wofür der kürzeste und der längste Balken stehen (niedrigster oder höchster Wert, eine Zahl, ein Prozentsatz, ein Perzentil oder eine Formel), Farbverlauf oder einfarbige Füllung, die Farben, ein Rahmen, die Farbe negativer Balken, die Lage der Achse, die Richtung der Balken und **Nur Balken anzeigen**;
- bei einer **Farbskala**: zwei oder drei Farben und wofür jede Farbe steht;
- bei einem **Symbolsatz**: der Symbolstil, der Wert, bei dem jedes Symbol beginnt, **Symbolreihenfolge umkehren** und **Nur Symbol anzeigen**;
- bei den übrigen Regeln: der Wert, Text, die Datumsangaben oder die Formel sowie das Aussehen. Eine Regel, die eine Datei mit einem eigenen Aussehen mitgebracht hat, wird mit dem Aussehen **Benutzerdefiniertes Format** aufgeführt, und das Beispiel zeigt, wie es tatsächlich aussieht. Wenn Sie an der Regel etwas anderes ändern, bleibt ihr Aussehen genau so, wie es in der Datei war. Wählen Sie **Benutzerdefiniertes Format...**, um das Aussehen zu ändern.

Mit **OK** übernehmen Sie alle Ihre Änderungen als einen Schritt, den Sie mit **Rückgängig** zurücknehmen können. Eine Regel, die Sie nicht geändert haben, behält genau das, was die Datei enthielt, unabhängig von ihrem Platz in der Liste. Eine Regel, die Sie geändert haben, wird neu geschrieben. Die Formel einer Regel wird aus der oberen linken Zelle ihres Bereichs gelesen. Eine Formel, die Sie für eine neue Regel eingeben, wird so gelesen, als stünde sie in der Zelle mit der Einfügemarke. Ein geschütztes Blatt, auf dem das Formatieren von Zellen nicht erlaubt ist, nimmt keine Regeln an.

## Was andere Dateiformate behalten

Nur eine .xlsx-Arbeitsmappe behält bedingte Formatierungen. Beim Speichern in ein älteres Dateiformat (.xls oder .ods) gehen sie verloren, und **Kompatibilität prüfen** weist darauf hin. Dabei werden Datenbalken, Farbskalen und Symbolsätze genannt, wenn das Blatt solche enthält. Eine .csv-Datei behält überhaupt keine Formate. Bedingte Formatierungen werden so gedruckt, wie sie auf dem Bildschirm aussehen.

## Regeln löschen

**Regeln aus ausgewählten Zellen entfernen** entfernt alle Regeln, die auf einer der markierten Zellen liegen. **Regeln aus diesem Blatt entfernen** entfernt alle Regeln des Blatts. **Regeln aus dieser Tabelle löschen** und **Regeln aus dieser Pivot-Tabelle entfernen** entfernen die Regeln, die auf der Tabelle (oder Pivot-Tabelle) liegen, in der sich die markierten Zellen befinden. Diese Befehle sind nur verfügbar, wenn die Auswahl in einer solchen Tabelle liegt. Jeder Befehl ist ein Schritt, den Sie rückgängig machen können. Ein Befehl tut nichts, wenn keine Regel zum Entfernen vorhanden ist.

## Was noch nicht unterstützt wird

- Ein Blatt mit Datenbalken aus einer neueren Dateiversion behält sie unverändert. JeffCrunch erstellt, ändert oder löscht Regeln auf diesem Blatt nicht. Nehmen Sie diese Änderungen in dem Programm vor, mit dem die Arbeitsmappe erstellt wurde.
- Eine Arbeitsmappe, die aus einem älteren Dateiformat geöffnet wurde, behält ihre Regeln, wenn Sie sie als .xlsx-Arbeitsmappe speichern. Danach können Sie Regeln hinzufügen.
- Ein eigenes Aussehen kann keine mittlere, dicke, doppelte oder schräge Linie haben, keine diagonale Linie, keine Farbverlaufsfüllung und keine Schriftart oder Schriftgröße. Diese Eigenschaften gehören nicht zu dem, was eine Regel ändern kann.
