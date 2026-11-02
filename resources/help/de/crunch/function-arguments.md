# Eine Funktion mit dem Dialog „Funktionsargumente“ eingeben
<!-- keywords: Funktionsargumente, Argument, fx, Formelergebnis, Umschalt+F3, Zellen auswählen, Bereich, optional, wiederholen, Funktion ändern, verschachtelte Funktion, Fehler -->

Der Dialog **Funktionsargumente** hilft Ihnen, eine Funktion Argument für Argument auszufüllen, und zeigt Ihnen, was die Formel ergibt, bevor Sie sie in die Zelle übernehmen.

## Öffnen

- Wählen Sie auf der Registerkarte **Formeln** den Befehl **Funktion auswählen** (**Umschalt+F3**) oder klicken Sie neben der Bearbeitungsleiste auf **fx**, und wählen Sie dann eine Funktion. Der Dialog öffnet sich für diese Funktion.
- Um eine Funktion zu ändern, die schon in einer Formel steht, setzen Sie den Cursor zwischen ihre Klammern (in der Zelle oder in der Bearbeitungsleiste) und drücken **Umschalt+F3** oder klicken auf **fx**. Der Dialog öffnet sich für diese Funktion, mit ihren Argumenten in den Feldern. In einer Formel wie =SUM(A1, IF(B1>2, 5, 6)) ändert ein Cursor innerhalb von IF die Funktion IF, ein Cursor in SUM außerhalb von IF die Funktion SUM.

## Ausfüllen

Jedes Argument hat eine Zeile: seinen Namen, ein Feld zum Eingeben und rechts davon, was es im Moment ergibt, zum Beispiel 12,5, "Text", TRUE oder {1,2,3} für einen Zellbereich. Ein fett gedruckter Name muss ausgefüllt werden; ein Name, der nicht fett ist, ist optional und kann leer bleiben.

- Mit der **Tabulatortaste** gelangen Sie zum nächsten Feld, mit **Umschalt+Tab** zum vorherigen.
- Die Zeile unter den Feldern sagt, was die Funktion tut, und die Zeile danach, was das Argument ist, in dem Sie gerade stehen.
- Funktionen wie SUM nehmen so viele Argumente, wie Sie brauchen: Sobald Sie in das letzte Feld schreiben, erscheint ein neues.
- Mit der Schaltfläche rechts neben einem Feld zeigen Sie auf Zellen. Der Dialog weicht einer kleinen Leiste, und Sie wählen mit der Maus Zellen im Blatt aus (das Feld zeigt beim Ziehen ihre Adresse). Klicken Sie auf die Schaltfläche der Leiste oder drücken Sie die **Eingabetaste**, um zurückzukehren.

## Das Ergebnis

**Formelergebnis =** zeigt, was die ganze Funktion ergibt, einen Moment nachdem Sie aufgehört haben zu tippen. Bis Sie auf **OK** klicken, wird nichts in das Blatt geschrieben. Stimmt etwas nicht, sagt eine Zeile unter dem Ergebnis es in Worten: zum Beispiel, dass ein Wert nicht gefunden wurde, dass durch null geteilt wird oder dass eine Zahl nicht erlaubt ist. Eine Formel, deren Berechnung hier zu lange dauern würde, wird nicht angezeigt; klicken Sie auf **OK**, dann rechnet das Blatt sie aus.

**Hilfe zu dieser Funktion** öffnet diese Seite.

## Abschließen

Drücken Sie die **Eingabetaste** oder klicken Sie auf **OK**, um die Funktion einzusetzen. In einer Zelle ohne laufende Eingabe wird die Formel in die Zelle geschrieben. In einer Formel, die Sie gerade eingeben, steht die Funktion an der Cursorposition oder ersetzt die, die Sie geändert haben, und Sie können weiterschreiben. Mit **Esc** oder **Abbrechen** ändert sich nichts.
