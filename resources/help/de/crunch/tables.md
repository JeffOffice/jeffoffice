# Tabellen
<!-- keywords: Tabelle, Tabellen, strukturierter Verweis, Kopfzeile, Überschriftszeile, Ergebniszeile, berechnete Spalte, gebändert, gestreift, Formatvorlage, Tabellendarstellungen, Filterschaltflächen, Strg+T, konvertieren, normale Zellen, Größe ändern, Spaltenname, [@Spalte], Tabelle1[Spalte] -->

Eine Tabelle ist ein Zellbereich mit einer Zeile von Namen über den Spalten, der als Einheit behandelt wird: Sie hat einen Namen, eine Darstellung und Schaltflächen zum Filtern und Sortieren ihrer Zeilen, und sie wächst, wenn Sie etwas hinzufügen. Formeln können ihre Spalten über den Namen statt über die Zelladresse ansprechen.

## Tabelle erstellen

Markieren Sie die Zellen oder klicken Sie in eine Zelle der Daten, und wählen Sie auf der Registerkarte **Einfügen** den Befehl **Tabelle** (**Strg+T**). JeffCrunch fragt, welche Zellen die Tabelle umfasst und ob die erste Zeile die Namen der Spalten enthält. Namen, die Zahlen, Datumsangaben oder Formeln sind, werden zu dem Text, den sie anzeigen; leere Namen werden zu Spalte1, Spalte2 usw., und zwei Spalten mit demselben Namen werden nummeriert. Gibt es keine Namen, hat die Tabelle keine Überschriftszeile, und ihre Spalten heißen in Formeln Spalte1, Spalte2 usw.

Der Katalog **Tabellendarstellungen** (Registerkarte **Start** oder Registerkarte **Tabelle**) zeigt die Darstellungen: hell, mittel und dunkel, jeweils in Schwarz und in den sechs Farben des Designs der Arbeitsmappe. Wählen Sie eine Darstellung für Zellen, die keine Tabelle sind, werden diese zu einer Tabelle. Die Darstellung liegt unter den eigenen Formaten einer Zelle: Eine Füllung, Rahmen, eine Textfarbe oder Fettdruck, die Sie für eine Zelle festgelegt haben, bleiben erhalten.

## Die Registerkarte „Tabelle“

Die Registerkarte **Tabelle** wird angezeigt, solange sich der Cursor in einer Tabelle befindet.

- **Name** ist der Name, über den Formeln die Tabelle ansprechen. Alle Formeln, die den alten Namen verwenden, werden angepasst. Ein Name besteht aus Buchstaben, Ziffern, Unterstrichen und Punkten, beginnt mit einem Buchstaben oder einem Unterstrich und darf weder der Name einer anderen Tabelle noch ein Name in der Arbeitsmappe noch eine Zelladresse wie A1 sein.
- **Überschriftszeile**, **Ergebniszeile**, **Gebänderte Zeilen**, **Gebänderte Spalten**, **Erste Spalte**, **Letzte Spalte** und **Filterschaltflächen** werden für die Tabelle aktiviert oder deaktiviert.
- **Tabellengröße ändern…** fragt nach den Zellen, die die Tabelle umfasst. Die erste Zelle bleibt erhalten; unten und rechts können Zeilen und Spalten hinzugefügt oder entfernt werden.
- **In normale Zellen konvertieren** wandelt die Tabelle in gewöhnliche Zellen um. Deren Inhalt bleibt erhalten. Eine Formel, die die Tabelle oder ihre Spalten genannt hat, verweist stattdessen auf die Zellen.

## Das Kontextmenü der Zellen

Klicken Sie mit der rechten Maustaste auf eine Zelle einer Tabelle, und wählen Sie **Tabelle**: **Zeile oberhalb einfügen**, **Zeile unterhalb einfügen**, **Spalte links einfügen**, **Spalte rechts einfügen**, **Tabellenzeilen löschen**, **Tabellenspalten löschen**, **Ergebniszeile** und **In normale Zellen konvertieren**. Diese Befehle fügen ganze Zeilen und Spalten des Blatts ein oder entfernen sie, wie „Einfügen“ und „Löschen“; eine Zeile oder Spalte, die direkt hinter dem Ende der Tabelle eingefügt wird, wird in die Tabelle aufgenommen.

## Ergebniszeile

Wenn Sie **Ergebniszeile** aktivieren, enthält die Zeile unter der Tabelle in der ersten Spalte das Wort „Ergebnis“ und in der letzten Spalte ein Ergebnis. Jede Zelle dieser Zeile hat eine Schaltfläche: Wählen Sie, was das Ergebnis der Spalte anzeigt (Mittelwert, Anzahl, Anzahl der Zahlen, Maximum, Minimum, Summe, Standardabweichung, Varianz oder **Eigene Formel…**). Ein Ergebnis ist ein TEILERGEBNIS seiner Spalte und lässt daher Zeilen aus, die ein Filter ausblendet.

## Eine Tabelle erweitern

- Geben Sie etwas in die Zeile unter einer Tabelle oder in die Spalte daneben ein, wächst die Tabelle und nimmt die Zelle auf. **Rückgängig** macht die Erweiterung rückgängig und behält Ihre Eingabe.
- Eine Formel, die Sie in eine Zelle einer ansonsten leeren Spalte eingeben, wird in jede Zelle dieser Spalte übernommen: eine berechnete Spalte. Wählen Sie direkt danach **Rückgängig**, bleibt die Formel nur in dieser einen Zelle. Eine Formel, die Sie in einer berechneten Spalte eingeben, ändert die ganze Spalte, wenn die übrigen Zellen die alte Formel enthalten; ist das bei einigen nicht der Fall, bleibt die Zelle eine Ausnahme.
- Innerhalb einer Tabelle eingefügte Zeilen erhalten die Formeln der berechneten Spalten. Innerhalb der Tabelle eingefügte Spalten erhalten einen Namen in der Überschriftszeile. Das Löschen von Zeilen oder Spalten innerhalb der Tabelle verkleinert sie; die Überschriftszeile und alle Spalten zugleich lassen sich nicht löschen.

## Tabelle filtern und sortieren

Die Schaltflächen an den Namen filtern und sortieren nur die Zeilen der Tabelle. Sie funktionieren wie die Schaltflächen eines Blattfilters (siehe [Sortieren und filtern](sort-filter)). Eine Tabelle kann nicht gefiltert werden, solange ein Filter des Blatts oder einer anderen Tabelle dieselben Zeilen umfasst, da jeder Filter wissen müsste, welche Zeilen zu ihm gehören.

## Formeln, die eine Tabelle nennen

- `Sales[Amount]` sind die Zellen der Spalte „Amount“, ohne deren Namen und Ergebnis.
- `[@Amount]` ist die Zelle in der Zeile der Formel selbst und wird in der Datei als `Sales[[#This Row],[Amount]]` geschrieben.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]` und `Sales[#All]` sind die Namen, die Daten, die Ergebnisse und die gesamte Tabelle, und `Sales[[#Headers],[Amount]]` ist eine Spalte davon.
- `Sales[[Region]:[Amount]]` sind die Spalten von „Region“ bis „Amount“.

Geben Sie nach dem Namen einer Tabelle `[` ein, wird eine Liste ihrer Spalten angezeigt. Wenn Sie beim Eingeben einer Formel auf eine Zelle einer Tabelle klicken, wird ein Bezug wie `Sales[@Amount]` oder `Sales[Amount]` (beim Klicken auf eine ganze Datenspalte) geschrieben. Wenn Sie eine Spalte in ihrer Überschrift umbenennen, ändert sich der Name in jeder Formel; beim Einfügen und Löschen von Spalten verweisen Formeln weiterhin auf die genannten Spalten, und eine gelöschte Spalte hinterlässt #BEZUG! in den Formeln, die sie genannt haben.
