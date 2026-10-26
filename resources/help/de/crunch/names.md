# Namen für Zellen und Werte
<!-- keywords: Name, Namen, definierter Name, neuer Name, benannter Bereich, Bereichsname, Gültigkeitsbereich, Kommentar, Namen aus Zellen, Namen einfügen, Namens-Manager, Strg+F3, bezieht sich auf -->

Ein **Name** ist ein Wort, das für eine Zelle, einen Zellbereich, eine Zahl oder eine Formel steht. So können Sie =Preis*Menge statt =B2*C2 schreiben. Ein Name gilt für die gesamte Arbeitsmappe oder für ein einzelnes Blatt. Ein Name auf einem Blatt verdeckt auf diesem Blatt einen gleich geschriebenen Namen der Arbeitsmappe.

## Regeln für Namen

- Ein Name beginnt mit einem Buchstaben, einem Unterstrich oder einem umgekehrten Schrägstrich und enthält danach nur Buchstaben, Ziffern, Unterstriche und Punkte. Leerzeichen sind nicht erlaubt.
- Ein Name darf höchstens 255 Zeichen lang sein.
- Ein Name darf keine Zelladresse sein, etwa **A1** oder **R2C3**.
- Kein anderer Name desselben Blatts (oder der Arbeitsmappe) darf gleich geschrieben sein, unabhängig von Groß- und Kleinschreibung.

Während der Eingabe zeigt das Dialogfeld an, was nicht stimmt, und die Schaltfläche **OK** bleibt so lange deaktiviert.

## Namen erstellen

1. Markieren Sie die Zellen, und klicken Sie auf der Registerkarte **Formeln** auf **Neuer Name**.
2. Geben Sie den Namen ein, wählen Sie den Gültigkeitsbereich aus, und legen Sie fest, wofür der Name steht. Die markierten Zellen sind bereits eingetragen.

Mit **Namen aus Zellen** erstellen Sie viele Namen auf einmal aus den Beschriftungen rund um einen Bereich: Stehen die Überschriften in der obersten Zeile und die Zahlen darunter, benennt jede Überschrift die Zellen darunter. Wählen Sie aus, wo sich die Beschriftungen befinden (oberste Zeile, linke Spalte, unterste Zeile oder rechte Spalte). Leerzeichen und Zeichen, die in Namen nicht erlaubt sind, werden durch Unterstriche ersetzt. Beschriftungen, die leer sind, doppelt vorkommen oder bereits als Name in der Arbeitsmappe vorhanden sind, werden übersprungen. Die Statusleiste zeigt an, wie viele das sind.

## Alle Namen anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Namen**, oder drücken Sie **Strg+F3**. Die Liste zeigt zu jedem Namen den aktuellen Wert, wofür er steht, seinen Gültigkeitsbereich und seinen Kommentar. Geben Sie etwas in das Filterfeld ein, um nur passende Namen anzuzeigen. Mit **Neu**, **Ändern** und **Löschen** erstellen, ändern und entfernen Sie Namen. Enthält eine Arbeitsmappe mehr als 20.000 Namen, werden die ersten 20.000 aufgelistet.

Wenn Sie einen Namen umbenennen, wird er in einem Schritt in allen Formeln auf allen Blättern geändert, die ihn verwenden. Wenn Sie einen Namen löschen, zeigen die Formeln, die ihn verwendet haben, **#NAME?** an. Jede Änderung lässt sich rückgängig machen.

## Namen in eine Formel einfügen

Während Sie eine Formel eingeben, werden passende Namen in der Liste unter der Zelle angezeigt. **Namen einfügen** auf der Registerkarte **Formeln** listet die Namen zur Auswahl auf.
