# Funktionen mit dem Dialogfeld „Funktionsargumente“ eingeben
<!-- keywords: Funktionsargumente, Argument, fx, Formelergebnis, Umschalt+F3, Zellen auswählen, Bereich, Zellbereich, optional, wiederholen, Funktion bearbeiten, verschachtelte Funktion, Fehler -->

Das Dialogfeld **Funktionsargumente** hilft Ihnen, eine Funktion Argument für Argument auszufüllen, und zeigt das Ergebnis der Formel an, bevor Sie sie in die Zelle übernehmen.

## Öffnen

- Wählen Sie auf der Registerkarte „Formeln“ **Funktion auswählen** (**Umschalt+F3**), oder klicken Sie neben der Bearbeitungsleiste auf **fx**, und wählen Sie dann eine Funktion aus. Das Dialogfeld wird für diese Funktion geöffnet.
- Um eine Funktion zu ändern, die bereits in einer Formel steht, setzen Sie den Cursor in ihre Klammern (in der Zelle oder in der Bearbeitungsleiste) und drücken **Umschalt+F3** oder klicken auf **fx**. Das Dialogfeld wird für diese Funktion geöffnet, und ihre Argumente stehen in den Feldern. In einer Formel wie =SUMME(A1; WENN(B1>2; 5; 6)) ändern Sie mit dem Cursor innerhalb von WENN die Funktion WENN, mit dem Cursor in SUMME, aber außerhalb von WENN, die Funktion SUMME.

## Ausfüllen

Jedes Argument hat eine eigene Zeile: seinen Namen, ein Feld zur Eingabe und rechts daneben seinen aktuellen Wert, z. B. 12,5, "Text", WAHR oder {1;2;3} für einen Zellbereich. Ein fett dargestellter Name muss ausgefüllt werden; ein nicht fett dargestellter Name ist optional und kann leer bleiben.

- Mit **Tab** gelangen Sie zum nächsten Feld, mit **Umschalt+Tab** zum vorherigen.
- Die Zeile unter den Feldern beschreibt, was die Funktion tut, und die Zeile darunter, was das aktuelle Argument bedeutet.
- Funktionen wie SUMME nehmen beliebig viele Argumente an: Sobald Sie etwas in das letzte Feld eingeben, erscheint ein neues.
- Mit der Schaltfläche rechts neben einem Feld können Sie auf Zellen zeigen. Das Dialogfeld wird zu einer schmalen Leiste verkleinert, und Sie markieren die Zellen im Blatt mit der Maus (das Feld zeigt beim Ziehen ihre Adresse an). Klicken Sie auf die Schaltfläche in der Leiste, oder drücken Sie die **Eingabetaste**, um zurückzukehren.

## Das Ergebnis

**Formelergebnis =** zeigt das Ergebnis der gesamten Funktion an, kurz nachdem Sie die Eingabe beendet haben. In das Blatt wird erst etwas geschrieben, wenn Sie auf **OK** klicken. Stimmt etwas nicht, beschreibt eine Zeile unter dem Ergebnis das Problem in Worten, zum Beispiel, dass ein Wert nicht gefunden wurde, dass durch null geteilt wird oder dass eine Zahl nicht zulässig ist. Eine Formel, deren Berechnung hier zu lange dauern würde, wird nicht angezeigt; klicken Sie auf **OK**, dann berechnet das Blatt sie.

**Hilfe für diese Funktion** öffnet diese Seite.

## Fertig stellen

Drücken Sie die **Eingabetaste**, oder klicken Sie auf **OK**, um die Funktion einzufügen. Wenn Sie in der Zelle gerade nichts eingeben, wird die Formel in die Zelle geschrieben. Geben Sie gerade eine Formel ein, wird die Funktion an der Cursorposition eingefügt oder ersetzt die geänderte Funktion, und Sie können weiterschreiben. Drücken Sie **Esc**, oder klicken Sie auf **Abbrechen**, bleibt alles unverändert.
