# Namen für Zellen und Werte
<!-- keywords: Name, Namen, definierter Name, neuer Name, benannter Bereich, Gültigkeitsbereich, Kommentar, Namen aus Zellen, Name einfügen, Strg+F3, verweist auf, Namen sortieren, Fehlername, Warnung -->

Ein **Name** ist ein Wort, das für eine Zelle, einen Zellbereich, eine Zahl oder eine Formel steht. So kann =Preis*Menge anstelle von =B2*C2 geschrieben werden. Ein Name gilt für die gesamte Arbeitsmappe oder für ein einzelnes Blatt. Ein Name auf einem Blatt verdeckt auf diesem Blatt einen gleich geschriebenen Namen der Arbeitsmappe.

## Die Regeln für einen Namen

- Er beginnt mit einem Buchstaben, einem Unterstrich oder einem Backslash und enthält danach nur Buchstaben, Ziffern, Unterstriche und Punkte. Er enthält keine Leerzeichen.
- Er ist höchstens 255 Zeichen lang.
- Er ist keine Zelladresse wie **A1** oder **Z2S3**.
- Kein anderer Name desselben Blatts (oder der Arbeitsmappe) ist gleich geschrieben, unabhängig von Groß- und Kleinschreibung.

Während Sie tippen, zeigt das Dialogfeld an, was nicht stimmt, und die Schaltfläche **OK** bleibt deaktiviert.

## Einen Namen erstellen

1. Markieren Sie die Zellen, und klicken Sie auf der Registerkarte **Formeln** auf **Neuer Name**.
2. Geben Sie den Namen ein, wählen Sie, wo er gelten soll, und legen Sie fest, wofür er steht. Die markierten Zellen werden für Sie eingetragen.

Für einen Namen der gesamten Arbeitsmappe gibt es einen schnelleren Weg. Markieren Sie die Zellen, klicken Sie auf das **Namenfeld** (links neben der Bearbeitungsleiste), geben Sie den Namen ein, und drücken Sie **Eingabe**. Ist noch kein Name mit dieser Schreibweise vorhanden, wird er für die markierten Zellen angelegt. Gibt es ihn bereits, springt **Eingabe** zu seinen Zellen.

**Namen aus Zellen** erstellt auf einmal viele Namen aus den Beschriftungen um einen Bereich herum: Stehen die Überschriften in der obersten Zeile und die Zahlen darunter, benennt jede Überschrift die Zellen darunter. Wählen Sie, wo sich die Beschriftungen befinden (oberste Zeile, linke Spalte, unterste Zeile oder rechte Spalte). Eine Beschriftung mit Leerzeichen oder mit Zeichen, die in einem Namen nicht erlaubt sind, erhält an deren Stelle Unterstriche. Eine Beschriftung, die leer ist, doppelt vorkommt oder bereits als Name der Arbeitsmappe vorhanden ist, wird ausgelassen. Die Statusleiste zeigt an, wie viele es waren.

## Alle Namen ansehen

Klicken Sie auf der Registerkarte **Formeln** auf **Namen**, oder drücken Sie **Strg+F3**. Die Liste zeigt jeden Namen, den Wert, den er gerade hat, wofür er steht, wo er gilt und seinen Kommentar. Geben Sie im Filter ein, um nur die passenden Namen anzuzeigen. Mit **Neu**, **Ändern** und **Löschen** erstellen, ändern und entfernen Sie einen Namen. Klicken Sie auf eine Überschrift, um die Liste danach zu sortieren, und erneut, um die Reihenfolge umzukehren. Ein Name, der auf einen Fehler verweist, etwa auf eine gelöschte Stelle, trägt daneben ein kleines Warnzeichen. Enthält eine Arbeitsmappe mehr als 20.000 Namen, werden nur die ersten 20.000 aufgelistet.

Wenn Sie den Namen eines Namens ändern, wird er in jeder Formel, die ihn verwendet, auf allen Blättern in einem Schritt geändert. Entfernen Sie einen Namen, zeigen die Formeln, die ihn verwendet haben, **#NAME?** an. Jede Änderung lässt sich rückgängig machen.

## Einen Namen in eine Formel einfügen

Während Sie eine Formel eingeben, werden die passenden Namen in der Liste unter der Zelle angezeigt. Die Schaltfläche **Name einfügen** auf der Registerkarte **Formeln** zeigt die Namen zur Auswahl an.
