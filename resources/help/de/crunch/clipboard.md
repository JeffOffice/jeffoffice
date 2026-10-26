# Ausschneiden, Kopieren und Einfügen
<!-- keywords: ausschneiden, kopieren, einfügen, Zwischenablage, verschieben, duplizieren, Inhalte einfügen, Werte, Formeln, Formate, transponieren, Leerzellen überspringen, addieren, subtrahieren, multiplizieren, dividieren, Vorgang, Rechenoperation, verbundene Zellen, Text, Tabulator, HTML, Webseite, anderes Programm, Esc, Laufrahmen, gestrichelter Rahmen, Rahmen -->

**Kopieren** (**{key:Copy}**) legt eine Kopie der markierten Zellen in der Zwischenablage ab, einem Speicherplatz, den der Computer jeweils für ein Element bereithält. **Einfügen** (**{key:Paste}**) fügt den Inhalt der Zwischenablage in die Zellen ab der markierten Zelle ein. **Ausschneiden** (**{key:Cut}**) funktioniert wie Kopieren, beim Einfügen werden die Zellen jedoch verschoben und die ursprüngliche Stelle wird geleert.

Ein bewegter gestrichelter Rahmen zeigt die kopierten oder ausgeschnittenen Zellen an. Drücken Sie **Esc**, um den Rahmen zu entfernen; die Zellen stehen dann nicht mehr zum Einfügen bereit. Ausgeschnittene Zellen werden auch freigegeben, wenn Sie vor dem Einfügen eine andere Änderung vornehmen (Eingeben, Einfügen von Zellen, Rückgängigmachen): Schneiden Sie sie erneut aus, wenn Sie sie weiterhin verschieben möchten.

## Einfügen innerhalb von JeffCrunch

Alles wird übernommen: Werte, Formeln, Formate und verbundene Zellen. Eine Formel verweist weiterhin auf dieselben Stellen *relativ zu ihrer Position*. Beispiel: Kopieren Sie **=A1*2** aus B1 und fügen Sie es in B5 ein, wird daraus **=A5*2**. Ein Teil mit Dollarzeichen, etwa **$A$1**, bleibt unverändert. Wenn Sie einen Bereich markieren, der ein ganzzahliges Vielfaches der Größe des Kopierten ist, wird die Kopie wiederholt, bis er gefüllt ist: Kopieren Sie eine Zelle, markieren Sie zwanzig Zellen und fügen Sie ein, erhalten alle zwanzig den Inhalt.

Ist eine eingefügte Zahl zu breit für eine Spalte, die noch ihre ursprüngliche Breite hat, wird die Spalte verbreitert.

## Ausschneiden verschiebt Zellen

Wenn Sie Zellen ausschneiden und einfügen, werden die Zellen tatsächlich verschoben. Jede Formel, die auf sie verwiesen hat, verweist nun auf ihre neue Position. Beispiel: B1 enthält **=A1+A2**. Schneiden Sie A1:A2 aus und fügen Sie es in E4 ein, wird B1 zu **=E4+E5**. Was in den überschriebenen Zellen stand, ist weg (**Rückgängig** stellt es wieder her), und eine Formel, die eine dieser Zellen gelesen hat, zeigt **#BEZUG!** an, weil die gelesene Zelle nicht mehr vorhanden ist. Sind die ausgeschnittenen Zellen nur ein Teil einer verbundenen Zelle oder überschreiben Sie nur einen Teil davon, wird nichts verschoben: Markieren Sie die gesamte verbundene Zelle. Wenn Sie Zellen auf ein anderes Blatt verschieben, lesen die Formeln weiterhin dieselben Zellen wie zuvor (mit Angabe des Blatts). Nach dem Ausschneiden und Einfügen wird die Zwischenablage geleert.

## Inhalte einfügen

Mit **Strg+Alt+V** oder dem Pfeil unter **Einfügen** auf der Registerkarte **Start** wählen Sie aus, was eingefügt werden soll:

- **Alle**, **Formeln**, **Werte** oder **Formate**. *Werte* wandelt eine Formel in die Zahl oder den Text um, den sie anzeigt.
- Ein **Vorgang**: **Addieren**, **Subtrahieren**, **Multiplizieren** oder **Dividieren**. Die kopierten Zahlen werden mit den Zahlen verrechnet, die bereits in den Zellen stehen. Beispiel: Die Zellen enthalten 10 und 20, Sie haben 5 kopiert und *Multiplizieren* gewählt: Daraus werden 50 und 100.
- **Leerzellen überspringen**: Leere Zellen in der Kopie lassen die Zellen, auf die sie treffen, unverändert.
- **Transponieren**: Aus Zeilen werden Spalten und aus Spalten Zeilen.

## Zwischen Programmen

Beim Kopieren legt JeffCrunch die Zellen zusätzlich als Text (Zellen durch Tabulatoren getrennt, eine Zeile pro Zeile) und als Tabelle für Webseiten in der Zwischenablage ab, damit andere Programme sie übernehmen können. Wenn Sie etwas einfügen, das nicht aus JeffCrunch stammt, etwa Text mit Tabulatoren oder eine Tabelle aus einer Webseite oder einer anderen Tabellenkalkulation, kommt jedes Element in eine eigene Zelle und wird so gelesen, als hätten Sie es eingegeben: Zahlen, Datumsangaben und Formeln, die mit einem Gleichheitszeichen beginnen. Beim Kopieren aus einer anderen in JeffCrunch geöffneten Arbeitsmappe werden Werte, Formeln und Zahlenformate übernommen, aber nicht die übrige Formatierung.

## Was nicht eingefügt werden kann

JeffCrunch weist Sie darauf hin und ändert nichts, wenn das Einfügen einen Teil einer **Matrixformel** betreffen würde, die Zeile mit den Spaltennamen einer **Tabelle** überschreiben würde, gesperrte Zellen eines **geschützten Blatts** betreffen würde, über das Ende des Blatts hinausginge oder mehr als 100.000 Zellen umfassen würde. Eine ganze **Matrixformel** kann ausgeschnitten und verschoben werden (markieren Sie alle ihre Zellen); ein Teil davon, eine Datentabelle oder die Namenszeile einer Tabelle nicht.
