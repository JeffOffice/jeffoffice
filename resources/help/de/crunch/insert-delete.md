# Zeilen, Spalten und Zellen einfügen und löschen
<!-- keywords: einfügen, löschen, entfernen, Zeile, Spalte, Zelle, verschieben, hinzufügen, leer, Leerzeile, Strg+Plus, Strg+Minus, Rechtsklick, Kontextmenü, Überschrift, Kopfzeile, rückgängig, Bezug, #BEZUG!, Formel, Name, verbunden, verbundene Zellen, Tabelle, Matrix, Matrixformel, Rand -->

Sie können mitten in Ihren Daten leere Zeilen, Spalten oder Zellen hinzufügen und Zeilen, Spalten oder Zellen entfernen. Alles dahinter rückt nach, um Platz zu schaffen oder die Lücke zu schließen, und Formeln, die auf die verschobenen Zellen verweisen, verweisen weiterhin auf sie.

## Einfügen

1. Markieren Sie die Zeilennummern, die Spaltenbuchstaben oder die Zellen, an deren Stelle die neuen eingefügt werden sollen. Um drei Zeilen hinzuzufügen, markieren Sie drei Zeilen.
2. Drücken Sie **Strg und die Plustaste** (**Strg und die Minustaste** löscht), oder wählen Sie **Start > Einfügen**, oder klicken Sie mit der rechten Maustaste auf die Markierung, und wählen Sie **Einfügen**.

Wenn ganze Zeilen oder Spalten markiert sind, werden die neuen sofort eingefügt, oberhalb der Zeilen bzw. links von den Spalten. Wenn Zellen markiert sind, fragt ein kleines Fenster, wie die nachfolgenden Zellen verschoben werden sollen:

- **Zellen nach rechts verschieben** oder **Zellen nach unten verschieben** verschiebt nur die Zellen in den Spalten (bzw. Zeilen) Ihrer Markierung, der Rest des Blatts bleibt, wo er ist.
- **Ganze Zeile** oder **Ganze Spalte** fügt ganze Zeilen oder Spalten hinzu.

## Löschen

Markieren Sie, was Sie entfernen möchten, und drücken Sie **Strg und die Minustaste**, oder wählen Sie **Start > Löschen**. Wenn Zellen markiert sind, fragt das Fenster, ob **Zellen nach links verschieben**, **Zellen nach oben verschieben** oder die **Ganze Zeile** bzw. **Ganze Spalte** gelöscht werden soll. Das Löschen ist ein einziger Schritt, und **Rückgängig** stellt alles wieder her, was entfernt wurde, auch die Formeln.

## Was sich mit ändert

Eine Formel, die auf verschobene Zellen verweist, verweist auf sie an ihrer neuen Position. Beispiel: In B1 steht **=SUMME(A1:A5)**. Fügen Sie oberhalb von Zeile 3 eine Zeile ein, wird die Formel zu **=SUMME(A1:A6)**, weil der Bereich um die neue Zeile gewachsen ist. Löschen Sie Zeile 3, wird sie zu **=SUMME(A1:A4)**. Eine Formel, die auf eine gelöschte Zelle verwiesen hat, zeigt **#BEZUG!** an. Das bedeutet, dass die benötigte Zelle nicht mehr vorhanden ist. **Rückgängig** stellt die Zelle und die Formel wieder her.

Ebenfalls mit den Zellen verschoben werden: Formeln auf allen Blättern, Namen (etwa Druckbereiche und Drucktitel), verbundene Zellen, die Zellen, für die Regeln der Datenüberprüfung und bedingte Formatierungen gelten, Tabellen, Diagramme, Bilder, Notizen, Links und Seitenumbrüche.

## Was nicht möglich ist

- Zellen können nicht durch einen Teil einer **Matrixformel** oder einer Datentabelle hindurch eingefügt oder gelöscht werden. Verschieben Sie sie als Ganzes, oder lassen Sie sie unverändert. Auch eine Zelle, die eine Datentabelle als Eingabe liest, kann nicht gelöscht werden.
- Eine **Pivot-Tabelle** kann noch nicht verschoben oder durchschnitten werden: Zeilen, Spalten oder Zellen können nicht eingefügt oder gelöscht werden, wenn sie dadurch verschoben oder verändert würde, und Zellen können nicht aus ihr ausgeschnitten oder in sie eingefügt werden. Der Rest des Blatts lässt sich wie gewohnt ändern. Eine Pivot-Tabelle ändern Sie mit den Befehlen der Registerkarte **Pivot-Tabelle** (Aktualisieren, Datenquelle ändern, Felder und Layout), nicht durch Eingaben in ihre Zellen.
- Eine **Tabelle** kann nicht durchschnitten werden: Sie können keine Spalte in ihr einfügen und weder ihre Kopfzeile noch alle ihre Zeilen löschen. Fügen Sie Zeilen innerhalb der Tabelle ein oder löschen Sie sie, oder ändern Sie die Zellen um sie herum.
- In einem **geschützten Blatt** können keine Zeilen, Spalten oder Zellen eingefügt oder gelöscht werden. Wählen Sie dazu auf der Registerkarte „Überprüfen“ den Befehl „Blattschutz aufheben“.
- Zellen am Ende des Blatts, die Inhalt haben, können nicht über den Rand hinausgeschoben werden. Löschen Sie sie zuerst.
- Nach dem Speichern macht **Rückgängig** weiterhin frühere Schritte rückgängig, auch eingefügte Zeilen und Spalten (beim nächsten Speichern werden sie wieder entfernt). Nur gelöschte Zeilen, Spalten oder Zellen kann es nicht zurückholen: Die gespeicherte Datei enthält deren Inhalt nicht mehr. Deshalb endet **Rückgängig** beim zuletzt gespeicherten Löschvorgang, und die Zeile am unteren Fensterrand weist beim Speichern darauf hin.
- Ein Schritt, der so viele Zellen verschiebt oder löscht, dass **Rückgängig** ihn nicht speichern könnte (mehr als etwa 128 MB), fragt vorher nach, wie man es von Tabellenkalkulationen kennt: „Dies kann nicht rückgängig gemacht werden.“ Wenn Sie fortfahren, wird er ausgeführt, und **Rückgängig** kann weder ihn noch vorherige Schritte rückgängig machen.
