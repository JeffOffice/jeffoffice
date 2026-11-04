# Namen für Zellen und Werte
<!-- keywords: Name, Namen, definierter Name, neuer Name, benannter Bereich, Gültigkeitsbereich, Kommentar, Name aus Zellen, Name einfügen, Strg+F3, bezieht sich auf, Namensfeld, Definierter Name -->

Ein **Name** ist ein Wort, das für eine Zelle, einen Zellbereich, eine Zahl oder eine Formel steht. So kann =Preis*Menge anstelle von =B2*C2 geschrieben werden. Ein Name gilt für die ganze Arbeitsmappe oder nur für ein Blatt. Ein Name auf einem Blatt verdeckt auf diesem Blatt einen Namen der Arbeitsmappe, der genauso geschrieben wird.

## Die Regeln für einen Namen

- Er beginnt mit einem Buchstaben, einem Unterstrich oder einem umgekehrten Schrägstrich und enthält danach nur Buchstaben, Ziffern, Unterstriche und Punkte. Er enthält keine Leerzeichen.
- Er ist höchstens 255 Zeichen lang.
- Er ist nicht die Adresse einer Zelle, zum Beispiel **A1** oder **Z2S3**.
- Auf demselben Blatt (oder in der Arbeitsmappe) gibt es keinen anderen Namen, der genauso geschrieben wird, unabhängig von Groß- und Kleinschreibung.

Während Sie eingeben, zeigt das Dialogfeld an, was nicht stimmt, und die Schaltfläche **OK** bleibt deaktiviert.

## Einen Namen erstellen

1. Markieren Sie die Zellen, und klicken Sie auf der Registerkarte **Formeln** auf **Neuer Name**.
2. Geben Sie den Namen ein, wählen Sie, wo er gelten soll, und legen Sie fest, wofür er steht. Die markierten Zellen werden für Sie eingetragen.

Für einen Namen der ganzen Arbeitsmappe gibt es einen schnelleren Weg. Markieren Sie die Zellen, klicken Sie auf das **Namenfeld** (links neben der Bearbeitungsleiste), geben Sie den Namen ein, und drücken Sie die **Eingabetaste**. Gibt es noch keinen Namen, der so geschrieben wird, wird er für die markierten Zellen erstellt. Gibt es ihn schon, springt die **Eingabetaste** zu seinen Zellen.

**Namen aus Zellen** erstellt mit einem Schlag viele Namen aus den Beschriftungen um einen Bereich: Stehen die Überschriften in der obersten Zeile und die Zahlen darunter, benennt jede Überschrift die Zellen darunter. Wählen Sie, wo die Beschriftungen stehen (oberste Zeile, linke Spalte, unterste Zeile oder rechte Spalte). Eine Beschriftung mit Leerzeichen oder Zeichen, die in einem Namen nicht vorkommen dürfen, erhält an deren Stelle Unterstriche. Eine Beschriftung, die leer ist, mehrfach vorkommt oder bereits als Name der Arbeitsmappe existiert, wird übersprungen, und die Statusleiste zeigt an, wie viele es waren.

## Alle Namen anzeigen

Klicken Sie auf der Registerkarte **Formeln** auf **Namen**, oder drücken Sie **Strg+F3**. Die Liste zeigt jeden Namen, den Wert, den er aktuell hat, wofür er steht, wo er gilt, und seinen Kommentar. Geben Sie im Filter ein, um nur passende Namen anzuzeigen. Mit **Neu**, **Ändern** und **Löschen** erstellen, ändern bzw. entfernen Sie einen Namen. Eine Arbeitsmappe mit mehr als 20.000 Namen zeigt die ersten 20.000 an.

Wenn Sie den Namen eines Namens ändern, wird er in einem Schritt in jeder Formel geändert, die ihn verwendet, auf allen Blättern. Wenn Sie einen Namen löschen, zeigen die Formeln, die ihn verwendet haben, **#NAME?** an. Jede Änderung können Sie rückgängig machen.

## Einen Namen in eine Formel einsetzen

Während Sie eine Formel eingeben, werden die passenden Namen in der Liste unter der Zelle angezeigt. Mit **Name einfügen** auf der Registerkarte **Formeln** wird eine Liste der Namen angezeigt, aus der Sie einen auswählen können.
