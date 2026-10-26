# Zeilen, Spalten und Zellen einfügen und löschen
<!-- keywords: einfügen, löschen, entfernen, Zeile, Spalte, Zelle, verschieben, hinzufügen, leer, Leerzeile, Strg+Plus, Strg+Minus, Rechtsklick, Kontextmenü, Kopfzeile, rückgängig, Bezug, #BEZUG!, Formel, Name, verbundene Zellen, Tabelle, Matrix, Array, Rand -->

Sie können mitten in Ihren Daten leere Zeilen, Spalten oder Zellen hinzufügen und Zeilen, Spalten oder Zellen entfernen. Alles dahinter rückt nach, um Platz zu schaffen oder die Lücke zu schließen, und Formeln, die auf die verschobenen Zellen verweisen, verweisen weiterhin auf diese.

## Einfügen

1. Markieren Sie die Zeilennummern, die Spaltenbuchstaben oder die Zellen an der Stelle, an der die neuen eingefügt werden sollen. Um drei Zeilen hinzuzufügen, markieren Sie drei Zeilen.
2. Drücken Sie **Strg und die Plustaste** (**Strg und die Minustaste** löscht), oder wählen Sie **Start > Einfügen**, oder klicken Sie mit der rechten Maustaste auf die Markierung, und wählen Sie **Einfügen**.

Sind ganze Zeilen oder Spalten markiert, werden die neuen sofort eingefügt, oberhalb der Zeilen bzw. links von den Spalten. Sind Zellen markiert, fragt ein kleines Fenster, wie die nachfolgenden Zellen verschoben werden sollen:

- **Zellen nach rechts verschieben** oder **Zellen nach unten verschieben** verschiebt nur die Zellen in den Spalten (bzw. Zeilen) Ihrer Markierung; der Rest des Blatts bleibt unverändert.
- **Ganze Zeile** oder **Ganze Spalte** fügt ganze Zeilen oder Spalten hinzu.

## Löschen

Markieren Sie, was Sie entfernen möchten, und drücken Sie **Strg und die Minustaste**, oder wählen Sie **Start > Löschen**. Sind Zellen markiert, fragt das Fenster, ob die Zellen **nach links verschoben** oder **nach oben verschoben** werden sollen oder ob die **ganze Zeile** bzw. **ganze Spalte** gelöscht werden soll. Das Löschen ist ein einziger Schritt, und **Rückgängig** stellt alles wieder her, was entfernt wurde, einschließlich der Formeln.

## Auswirkungen

Eine Formel, die auf verschobene Zellen verweist, verweist auf diese an ihrer neuen Position. Beispiel: In B1 steht **=SUMME(A1:A5)**. Fügen Sie oberhalb von Zeile 3 eine Zeile ein, wird daraus **=SUMME(A1:A6)**, weil der Bereich um die neue Zeile gewachsen ist. Löschen Sie Zeile 3, wird daraus **=SUMME(A1:A4)**. Eine Formel, die auf eine gelöschte Zelle verwiesen hat, zeigt **#BEZUG!** an: Die benötigte Zelle ist nicht mehr vorhanden. **Rückgängig** stellt Zelle und Formel wieder her.

Folgendes wird ebenfalls mit den Zellen angepasst: Formeln auf allen Blättern, Namen (etwa Druckbereiche und Drucktitel), verbundene Zellen, die Zellen, für die Regeln der Datenüberprüfung und bedingte Formatierungen gelten, Tabellen, Diagramme, Bilder, Notizen, Links und Seitenumbrüche.

## Was nicht möglich ist

- Zellen können nicht durch einen Teil einer **Matrixformel** oder einer Datentabelle hindurch eingefügt oder gelöscht werden. Verschieben Sie diese als Ganzes, oder lassen Sie sie unverändert. Eine Zelle, die eine Datentabelle als Eingabe liest, kann ebenfalls nicht gelöscht werden.
- Eine **PivotTable** kann derzeit nicht verschoben oder zerschnitten werden: Zeilen, Spalten oder Zellen können nicht eingefügt oder gelöscht werden, wenn sie dadurch verschoben oder verändert würde, und Zellen können nicht aus ihr heraus- oder in sie hineingeschnitten werden. Der Rest des Blatts lässt sich wie gewohnt ändern.
- Eine **Tabelle** kann nicht zerschnitten werden: Sie können keine Spalte innerhalb der Tabelle einfügen und weder ihre Kopfzeile noch alle ihre Zeilen löschen. Fügen Sie Zeilen innerhalb der Tabelle ein oder löschen Sie sie, oder ändern Sie die Zellen um sie herum.
- In einem **geschützten Blatt** können keine Zeilen, Spalten oder Zellen eingefügt oder gelöscht werden. Wählen Sie dazu auf der Registerkarte „Überprüfen“ den Befehl „Blattschutz aufheben“.
- Zellen am Ende des Blatts, die Inhalt haben, können nicht über den Rand hinausgeschoben werden. Löschen Sie sie zuerst.
- Auch nach dem Speichern macht **Rückgängig** weiterhin frühere Schritte rückgängig, auch eingefügte Zeilen und Spalten (beim nächsten Speichern werden sie wieder entfernt). Nur **gelöschte** Zeilen, Spalten oder Zellen lassen sich nicht wiederherstellen: Die gespeicherte Datei enthält ihren Inhalt nicht mehr. **Rückgängig** endet daher beim letzten gespeicherten Löschvorgang, und die Zeile am unteren Fensterrand weist beim Speichern darauf hin.
- Ein Schritt, der so viele Zellen verschiebt oder löscht, dass **Rückgängig** ihn nicht speichern könnte (mehr als etwa 128 MB), fragt vorher nach, wie man es von anderen Tabellenkalkulationen kennt: „Dies kann nicht rückgängig gemacht werden.“ Wenn Sie fortfahren, wird der Schritt ausgeführt, und **Rückgängig** kann weder ihn noch vorherige Schritte rückgängig machen.
