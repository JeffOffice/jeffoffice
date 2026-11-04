# Tabellen
<!-- keywords: Tabelle, Tabellen, Duplikate entfernen, Duplikate, zusammenfassen, Formatvorlage löschen, strukturierter Verweis, Kopfzeile, Ergebniszeile, berechnete Spalte, gebändert, gestreift, Formatvorlage, Tabellendarstellungen, Filterschaltflächen, Strg+T, konvertieren, normale Zellen, Größe ändern, Spaltenname, [@Spalte], Tabelle1[Spalte] -->

Eine Tabelle ist ein Zellbereich mit einer Zeile von Überschriften über ihren Spalten, der als ein Ganzes behandelt wird: Sie hat einen Namen, eine Darstellung, Schaltflächen zum Filtern und Sortieren ihrer Zeilen und wächst, wenn Sie etwas hinzufügen. Formeln können ihre Spalten über den Namen statt über die Zelladresse ansprechen.

## Eine Tabelle erstellen

Markieren Sie die Zellen oder klicken Sie auf eine Zelle in den Daten, und wählen Sie auf der Registerkarte **Einfügen** die Option **Tabelle** (**Strg+T**). JeffCrunch fragt, welche Zellen die Tabelle umfasst und ob die erste Zeile die Spaltenüberschriften enthält. Überschriften, die Zahlen, Datumsangaben oder Formeln sind, werden zu dem Text, den sie anzeigen. Leere Überschriften werden zu Spalte1, Spalte2 und so weiter, und zwei Spalten mit demselben Namen werden nummeriert. Gibt es keine Überschriften, hat die Tabelle keine Überschriftszeile, und ihre Spalten heißen in Formeln Spalte1, Spalte2 und so weiter.

Die Galerie **Tabellendarstellungen** (Registerkarte **Start** oder Registerkarte **Tabelle**) zeigt die Darstellungen: **Löschen** (unter der Galerie auf der Registerkarte **Tabelle**) entfernt die Darstellung von der Tabelle. Zur Auswahl stehen helle, mittlere und dunkle Darstellungen, jeweils in Schwarz und in den sechs Farben des Designs der Arbeitsmappe. Wählen Sie eine Darstellung für Zellen, die keine Tabelle sind, werden diese Zellen zu einer Tabelle. Die Darstellung wird unter die eigenen Formatierungen einer Zelle gelegt: Eine Füllung, Rahmen, eine Schriftfarbe oder Fettformatierung, die Sie für eine Zelle festgelegt haben, bleiben erhalten.

## Die Registerkarte „Tabelle“

Die Registerkarte **Tabelle** wird angezeigt, solange sich die Einfügemarke in einer Tabelle befindet.

- **Name** ist der Name, unter dem Formeln auf die Tabelle verweisen. Jede Formel, die den alten Namen verwendet, wird angepasst. Ein Name besteht aus Buchstaben, Ziffern, Unterstrichen und Punkten, beginnt mit einem Buchstaben oder einem Unterstrich und ist weder der Name einer anderen Tabelle noch ein Name in der Arbeitsmappe noch eine Zelladresse wie A1.
- **Überschriftszeile**, **Ergebniszeile**, **Gebänderte Zeilen**, **Gebänderte Spalten**, **Erste Spalte**, **Letzte Spalte** und **Filterschaltflächen** werden für die Tabelle aktiviert oder deaktiviert.
- **Tabellengröße ändern…** fragt nach den Zellen, die die Tabelle umfasst. Die erste Zelle bleibt erhalten; Zeilen und Spalten können unten und rechts hinzugefügt oder entfernt werden.
- **Mit Pivot-Tabelle zusammenfassen** erstellt aus den Zellen der Tabelle eine Pivot-Tabelle (siehe [Pivot-Tabellen](pivot-tables)). Die Pivot-Tabelle liest die Tabelle über ihren Namen, sodass sie mitwächst.
- **Duplikate entfernen…** entfernt die Zeilen, die sich wiederholen. Eine Zeile wiederholt sich, wenn sie in allen markierten Spalten dieselben Werte zeigt wie eine Zeile darüber. Groß- und Kleinschreibung gelten als gleich. Die erste der Zeilen bleibt erhalten, und die Zeilen unter einer entfernten Zeile rücken nach oben. Die Tabelle wird kürzer, und ihre Ergebniszeile rückt mit nach oben. Ein Schritt **Rückgängig** stellt alles wieder her. Zellen unter der Tabelle bleiben an ihrer Stelle. Formeln, die auf Zellen der Tabelle verweisen, auf einem beliebigen Blatt, folgen den Zellen, wie beim Löschen von Zeilen; eine Formel, die auf eine entfernte Zeile verwiesen hat, zeigt #BEZUG!. Bei einer Tabelle mit aktiviertem Filter funktioniert das nicht.
- **In normale Zellen konvertieren** wandelt die Tabelle in gewöhnliche Zellen um. Sie behalten ihren Inhalt. Eine Formel, die die Tabelle oder ihre Spalten benannt hat, nennt stattdessen die Zellen.

## Das Kontextmenü der Zellen

Klicken Sie mit der rechten Maustaste auf eine Zelle einer Tabelle, und wählen Sie **Tabelle**: **Zeile oberhalb einfügen**, **Zeile unterhalb einfügen**, **Spalte links einfügen**, **Spalte rechts einfügen**, **Tabellenzeilen löschen**, **Tabellenspalten löschen**, **Ergebniszeile** und **In normale Zellen konvertieren**. Sie fügen ganze Zeilen und Spalten des Blatts ein oder entfernen sie, wie es Einfügen und Löschen tun; eine Zeile, die direkt hinter dem Ende der Tabelle eingefügt wird, nimmt die Tabelle auf.

## Ergebniszeile

Aktivieren Sie **Ergebniszeile**, dann steht in der Zeile unter der Tabelle in ihrer ersten Spalte das Wort Gesamt und in ihrer letzten Spalte eine Summe. Jede Zelle der Zeile hat eine Schaltfläche: Wählen Sie aus, was die Summe der Spalte anzeigt (den Mittelwert, eine Anzahl, wie viele Zahlen es gibt, den größten Wert, den kleinsten Wert, die Summe, die Standardabweichung, die Varianz oder **Eigene Formel…**). Eine Summe ist eine TEILERGEBNIS-Funktion ihrer Spalte und lässt daher Zeilen aus, die ein Filter ausblendet.

## Eine Tabelle erweitern

- Geben Sie in der Zeile unter einer Tabelle oder in der Spalte daneben etwas ein, dann wächst die Tabelle und nimmt die Zelle auf. **Rückgängig** macht das Wachstum rückgängig und lässt Ihre Eingabe stehen.
- Eine Formel, die Sie in einer Zelle einer sonst leeren Spalte eingeben, wird in jede Zelle dieser Spalte übernommen: eine berechnete Spalte. Ein Schritt **Rückgängig** direkt danach lässt die Formel nur in der einen Zelle stehen. Eine Formel, die Sie über eine berechnete Spalte schreiben, ändert die ganze Spalte, wenn die übrigen Zellen die alte Formel enthalten; enthalten einige sie nicht, steht die Zelle für sich allein.
- Zeilen, die innerhalb einer Tabelle eingefügt werden, erhalten die Formeln der berechneten Spalten. Spalten, die innerhalb der Tabelle eingefügt werden, erhalten ihren Namen in der Überschriftszeile. Wenn Sie Zeilen oder Spalten innerhalb der Tabelle löschen, wird sie kleiner; die Überschriftszeile und jede Spalte können nicht alle gelöscht werden.

## Eine Tabelle filtern und sortieren

Die Schaltflächen in den Überschriften filtern und sortieren nur die Zeilen der Tabelle. Sie funktionieren wie die Schaltflächen des Filters eines Blatts (siehe [Sortieren und Filtern](sort-filter)). Eine Tabelle kann nicht gefiltert werden, solange ein Filter des Blatts oder einer anderen Tabelle dieselben Zeilen betrifft, da jede Tabelle wissen müsste, welche Zeilen ihre eigenen sind.

## Formeln, die eine Tabelle benennen

- `Umsatz[Betrag]` sind die Zellen der Spalte Betrag, ohne ihre Überschrift und ihre Summe.
- `[@Betrag]` ist die Zelle der eigenen Zeile der Formel und wird in der Datei als `Umsatz[[#Diese Zeile],[Betrag]]` geschrieben.
- `Umsatz[#Überschriften]`, `Umsatz[#Daten]`, `Umsatz[#Summe]` und `Umsatz[#Alle]` sind die Überschriften, die Daten, die Ergebniszeile und die ganze Tabelle, und `Umsatz[[#Überschriften],[Betrag]]` ist eine Spalte davon.
- `Umsatz[[Region]:[Betrag]]` sind die Spalten von Region bis Betrag.

Geben Sie nach dem Namen einer Tabelle `[` ein, dann erscheint eine Liste ihrer Spalten. Während Sie eine Formel eingeben, schreibt ein Klick auf eine Zelle der Tabelle einen Verweis wie `Umsatz[@Betrag]` oder `Umsatz[Betrag]` (ein Klick auf eine ganze Datenspalte). Wenn Sie eine Spalte in ihrer Überschrift umbenennen, ändert sich der Name in jeder Formel; beim Einfügen und Löschen von Spalten bleiben Formeln auf die Spalten gerichtet, die sie benannt haben, und eine gelöschte Spalte hinterlässt #BEZUG! in den Formeln, die sie benannt haben.
