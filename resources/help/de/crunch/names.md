# Namen für Zellen und Werte
<!-- keywords: Name, Namen, definierter Name, neuer Name, benannter Bereich, Gültigkeitsbereich, Kommentar, Namen aus Zellen, Name einfügen, Namen anwenden, Strg+F3, bezieht sich auf, Namen sortieren, Fehlername, Warnung, Bezeichner, Bereichsname -->

Ein **Name** ist ein Wort, das für eine Zelle, einen Zellbereich, eine Zahl oder eine Formel steht. So kann =Preis*Menge an Stelle von =B2*C2 geschrieben werden. Ein Name gilt für die ganze Arbeitsmappe oder nur für ein Blatt. Ein Name auf einem Blatt verdeckt auf diesem Blatt einen Namen der Arbeitsmappe, der genauso geschrieben wird.

## Die Regeln für einen Namen

- Er beginnt mit einem Buchstaben, einem Unterstrich oder einem Backslash und enthält danach nur Buchstaben, Ziffern, Unterstriche und Punkte. Er enthält keine Leerzeichen.
- Er darf höchstens 255 Zeichen lang sein.
- Er ist keine Zelladresse, etwa **A1** oder **Z2S3**.
- Auf demselben Blatt (oder in der Arbeitsmappe) gibt es keinen anderen Namen, der genauso geschrieben wird, unabhängig von Groß- und Kleinschreibung.

Während Sie tippen, zeigt das Dialogfeld an, was nicht stimmt, und die Schaltfläche **OK** bleibt deaktiviert.

## Einen Namen erstellen

1. Markieren Sie die Zellen, und klicken Sie auf der Registerkarte **Formeln** auf **Neuer Name**.
2. Geben Sie den Namen ein, wählen Sie, wo er gelten soll, und legen Sie fest, wofür er steht. Die markierten Zellen werden für Sie eingetragen.

Für einen Namen der ganzen Arbeitsmappe gibt es einen schnelleren Weg. Markieren Sie die Zellen, klicken Sie auf das **Namenfeld** (links neben der Bearbeitungsleiste), geben Sie den Namen ein und drücken Sie **Eingabe**. Wenn noch kein Name so geschrieben wird, wird er für die markierten Zellen angelegt. Gibt es ihn bereits, springt **Eingabe** zu seinen Zellen.

**Namen aus Zellen** erstellt auf einmal viele Namen aus den Beschriftungen um einen Bereich: Stehen die Überschriften in der obersten Zeile und die Zahlen darunter, benennt jede Überschrift die Zellen darunter. Wählen Sie, wo die Beschriftungen stehen (oberste Zeile, linke Spalte, unterste Zeile oder rechte Spalte). Ein Beschriftungstext mit Leerzeichen oder Zeichen, die in einem Namen nicht erlaubt sind, erhält an deren Stelle Unterstriche. Eine Beschriftung, die leer ist, doppelt vorkommt oder bereits als Name der Arbeitsmappe vorhanden ist, wird übergangen; die Statusleiste zeigt an, wie viele das waren.

## Alle Namen ansehen

Klicken Sie auf der Registerkarte **Formeln** auf **Namen** oder drücken Sie **Strg+F3**. Die Liste zeigt jeden Namen, den Wert, den er jetzt hat, wofür er steht, wo er gilt und seinen Kommentar. Geben Sie im Filter ein, um nur die passenden Namen anzuzeigen. Mit **Neu**, **Ändern** und **Löschen** erstellen, ändern bzw. entfernen Sie einen Namen. Klicken Sie auf eine Überschrift, um die Liste danach zu sortieren, und erneut, um die Reihenfolge umzukehren. Ein Name, der auf einen Fehler verweist, etwa auf eine gelöschte Stelle, hat ein kleines Warnzeichen daneben. Enthält eine Arbeitsmappe mehr als 20.000 Namen, werden die ersten 20.000 aufgelistet.

Wenn Sie einen Namen umbenennen, wird er in jeder Formel geändert, die ihn verwendet, auf allen Blättern, in einem Schritt. Wenn Sie einen Namen löschen, zeigen die Formeln, die ihn verwendet haben, **#NAME?** an. Jede Änderung kann rückgängig gemacht werden.

## Einen Namen in eine Formel einfügen

Während Sie eine Formel eingeben, werden die passenden Namen in der Liste unter der Zelle angezeigt. Mit **Namen einfügen** auf der Registerkarte **Formeln** erhalten Sie eine Liste der Namen, aus denen Sie wählen können.

## Namen in bereits geschriebene Formeln einsetzen

Angenommen, Sie haben den Namen Zinssatz für $A$1 erstellt, nachdem schon viele Formeln $A$1 enthielten. Klicken Sie auf der Registerkarte **Formeln** auf **Namen anwenden**, um Zinssatz an deren Stelle zu schreiben. Markieren Sie vorher die zu prüfenden Zellen, oder markieren Sie eine einzelne Zelle, um das ganze Blatt zu prüfen. Wählen Sie die einzusetzenden Namen aus. Ein Bezug wird nur geändert, wenn er genau den Zellen des Namens entspricht, mit oder ohne Dollarzeichen: A1:B2 wird nicht in einen Namen für A1 umgewandelt. Angeboten werden nur Namen für einen Zellbereich auf einem Blatt und Namen der ganzen Arbeitsmappe. Die Änderung erfolgt in einem Schritt, sodass **Rückgängig** sie zurücknimmt.
