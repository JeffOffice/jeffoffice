# Tabellen
<!-- keywords: Tabelle, Tabellen, Duplikate entfernen, Duplikate, zusammenfassen, Format löschen, strukturierter Verweis, Kopfzeile, Ergebniszeile, berechnete Spalte, gebändert, gestreift, Formatvorlage, Tabellendarstellungen, neue Tabellenformatvorlage, benutzerdefinierte Tabellenformatvorlage, Filterschaltflächen, Strg+T, konvertieren, normale Zellen, Größe ändern, Spaltenname, [@Spalte], Tabelle1[Spalte], Tabellenformat, Tabellenlayout, Tabellenname umbenennen, Zeilen und Spalten -->

Eine Tabelle ist ein Zellbereich mit einer Zeile mit Namen über seinen Spalten, der als Einheit behandelt wird: Sie hat einen Namen, eine Darstellung, Schaltflächen zum Filtern und Sortieren ihrer Zeilen und wächst, wenn Sie etwas hinzufügen. Formeln können ihre Spalten über den Namen statt über die Zelladresse ansprechen.

## Tabelle erstellen

Markieren Sie die Zellen, oder klicken Sie in eine Zelle der Daten, und wählen Sie auf der Registerkarte **Einfügen** die Option **Tabelle** (**Strg+T**). JeffCrunch fragt, welche Zellen die Tabelle umfasst und ob die erste Zeile die Namen der Spalten enthält. Namen, die Zahlen, Datumswerte oder Formeln sind, werden zu dem Text, den sie anzeigen. Leere Namen werden zu Spalte1, Spalte2 und so weiter, und zwei Spalten mit demselben Namen werden nummeriert. Gibt es keine Namen, hat die Tabelle keine Zeile mit Namen, und ihre Spalten heißen in Formeln Spalte1, Spalte2 und so weiter.

Die Galerie **Tabellendarstellungen** (Registerkarte **Start** oder Registerkarte **Tabelle**) zeigt die Darstellungen: **Löschen** (unter der Galerie auf der Registerkarte **Tabelle**) entfernt die Darstellung von der Tabelle. Zur Auswahl stehen helle, mittlere und dunkle Darstellungen, jeweils in Schwarz und in den sechs Farben des Designs der Arbeitsmappe. Wenn Sie eine davon für Zellen wählen, die keine Tabelle sind, werden diese zu einer Tabelle. Die Darstellung wird unter die eigene Formatierung einer Zelle gelegt: Eine Füllung, Rahmen, eine Textfarbe oder Fettschrift, die Sie auf einer Zelle festgelegt haben, bleiben erhalten.

Um eine eigene Darstellung zu erstellen, wählen Sie unten in der Galerie **Neue Tabellenformatvorlage**. Geben Sie einen Namen ein. Wählen Sie dann für jeden Teil, den Sie ändern möchten, eine Füllfarbe, eine Textfarbe und Fettschrift: die Kopfzeile, die Streifen der Zeilen, die Ergebniszeile und die erste Spalte. Die neue Darstellung erscheint in einer Gruppe **Benutzerdefiniert** oben in der Galerie und wird auf die Tabelle angewendet, in der Sie sich gerade befinden. Sie wird in der Datei gespeichert und ist daher beim erneuten Öffnen der Datei vorhanden.

## Die Registerkarte „Tabelle“

Die Registerkarte **Tabelle** wird angezeigt, solange sich die Einfügemarke in einer Tabelle befindet.

- **Name** ist der Name, unter dem Formeln die Tabelle ansprechen. Jede Formel, die den alten Namen verwendet, wird angepasst. Ein Name besteht aus Buchstaben, Ziffern, Unterstrichen und Punkten, beginnt mit einem Buchstaben oder einem Unterstrich und ist weder der Name einer anderen Tabelle noch ein Name der Arbeitsmappe noch eine Zelladresse wie A1.
- **Zeile mit Namen**, **Ergebniszeile**, **Gestreifte Zeilen**, **Gestreifte Spalten**, **Erste Spalte**, **Letzte Spalte** und **Filterschaltflächen** werden für die Tabelle aktiviert oder deaktiviert.
- **Tabellengröße ändern…** fragt nach den Zellen, die die Tabelle umfasst. Die erste Zelle bleibt erhalten; Zeilen und Spalten können unten und rechts hinzugefügt oder entfernt werden.
- **Mit Pivot-Tabelle zusammenfassen** erstellt aus den Zellen der Tabelle eine Pivot-Tabelle (siehe [Pivot-Tabellen](pivot-tables)). Die Pivot-Tabelle spricht die Tabelle über ihren Namen an und folgt ihr, wenn sie wächst.
- **Duplikate entfernen…** entfernt die Zeilen, die sich wiederholen. Eine Zeile wiederholt sich, wenn sie in allen angekreuzten Spalten dieselben Werte zeigt wie eine Zeile darüber. Groß- und Kleinschreibung gelten als gleich. Die erste der Zeilen bleibt erhalten, und die Zeilen unter einer entfernten Zeile rücken nach oben. Die Tabelle wird kürzer, und ihre Ergebniszeile rückt mit nach oben. Ein Schritt **Rückgängig** stellt alle wieder her. Zellen unter der Tabelle bleiben, wo sie sind. Formeln, die auf Zellen der Tabelle zeigen, auf einem beliebigen Blatt, folgen den Zellen, so wie beim Löschen von Zeilen; eine Formel, die auf eine entfernte Zeile zeigte, zeigt #BEZUG!. Bei einer Tabelle mit aktiviertem Filter funktioniert dies nicht.
- **In normale Zellen konvertieren** wandelt die Tabelle in gewöhnliche Zellen um. Sie behalten ihren Inhalt. Eine Formel, die die Tabelle oder ihre Spalten benannt hat, nennt stattdessen die Zellen.

## Das Menü der Zellen

Klicken Sie mit der rechten Maustaste auf eine Zelle einer Tabelle, und wählen Sie **Tabelle**: **Zeile oberhalb einfügen**, **Zeile unterhalb einfügen**, **Spalte links einfügen**, **Spalte rechts einfügen**, **Tabellenzeilen löschen**, **Tabellenspalten löschen**, **Ergebniszeile** und **In normale Zellen konvertieren**. Sie fügen ganze Zeilen und Spalten des Blatts ein oder entfernen sie, wie es auch Einfügen und Löschen tun; eine Zeile, die direkt hinter das Ende der Tabelle eingefügt wird, wird von der Tabelle übernommen.

## Ergebniszeile

Aktivieren Sie **Ergebniszeile**, dann enthält die Zeile unter der Tabelle in ihrer ersten Spalte das Wort Gesamt und in ihrer letzten Spalte eine Summe. Jede Zelle der Zeile hat eine Schaltfläche: Wählen Sie, was das Ergebnis der Spalte anzeigt (den Mittelwert, eine Anzahl, wie viele Zahlen es gibt, den größten Wert, den kleinsten Wert, die Summe, die Standardabweichung, die Varianz oder **Eigene Formel**). Ein Ergebnis ist eine TEILERGEBNIS-Funktion ihrer Spalte und lässt daher Zeilen aus, die ein Filter ausblendet.

## Zeilen zu einer Tabelle hinzufügen

- Geben Sie in der Zeile unter einer Tabelle oder in der Spalte daneben etwas ein, dann wächst die Tabelle und nimmt die Zelle auf. **Rückgängig** nimmt das Wachstum zurück und lässt Ihre Eingabe stehen.
- Eine Formel, die in einer Zelle einer sonst leeren Spalte eingegeben wird, wird in jede Zelle dieser Spalte übernommen: eine berechnete Spalte. Direkt danach **Rückgängig** ausführen, dann bleibt die Formel nur in der einen Zelle. Eine Formel, die über einer berechneten Spalte eingegeben wird, ändert die ganze Spalte, wenn die anderen Zellen die alte Formel enthalten; enthalten einige sie nicht, steht die Zelle allein.
- Zeilen, die innerhalb einer Tabelle eingefügt werden, erhalten die Formeln der berechneten Spalten. Spalten, die innerhalb der Tabelle eingefügt werden, erhalten ihren Namen in der Zeile mit Namen. Beim Löschen von Zeilen oder Spalten innerhalb der Tabelle wird sie kleiner; die Zeile mit Namen und jede Spalte können nicht alle gelöscht werden.

## Tabelle filtern und sortieren

Die Schaltflächen an den Namen filtern und sortieren nur die Zeilen der Tabelle. Sie funktionieren so, wie die Schaltflächen des Filters eines Blatts funktionieren (siehe [Sortieren und Filtern](sort-filter)). Eine Tabelle kann nicht gefiltert werden, während ein Filter des Blatts oder einer anderen Tabelle dieselben Zeilen betrifft, da jeder wissen müsste, welche Zeilen seine eigenen sind.

## Formeln, die eine Tabelle benennen

- `Verkäufe[Betrag]` sind die Zellen der Spalte Betrag, ohne ihren Namen und ohne ihr Ergebnis.
- `[@Betrag]` ist die Zelle der eigenen Zeile der Formel und wird in der Datei als `Verkäufe[[#Diese Zeile],[Betrag]]` geschrieben.
- `Verkäufe[#Kopfzeilen]`, `Verkäufe[#Daten]`, `Verkäufe[#Ergebnisse]` und `Verkäufe[#Alle]` sind die Namen, die Daten, die Ergebnisse und die ganze Tabelle, und `Verkäufe[[#Kopfzeilen],[Betrag]]` ist eine Spalte davon.
- `Verkäufe[[Region]:[Betrag]]` sind die Spalten von Region bis Betrag.

Geben Sie nach dem Namen einer Tabelle `[` ein, dann erscheint eine Liste ihrer Spalten. Während Sie eine Formel eingeben, schreibt ein Klick auf eine Zelle der Tabelle einen Verweis wie `Verkäufe[@Betrag]` oder `Verkäufe[Betrag]` (ein Klick auf eine ganze Spalte ihrer Daten). Wenn Sie eine Spalte in ihrer Kopfzeile umbenennen, ändert sich der Name in jeder Formel; das Einfügen und Löschen von Spalten hält Formeln auf die Spalten gerichtet, die sie benannt haben, und eine gelöschte Spalte hinterlässt in den Formeln, die sie benannt haben, #BEZUG!.
