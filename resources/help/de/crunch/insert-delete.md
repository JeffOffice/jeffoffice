# Zeilen, Spalten und Zellen einfügen und löschen
<!-- keywords: einfügen, ausgeschnittene Zellen einfügen, kopierte Zellen einfügen, nach rechts verschieben, nach unten verschieben, einfügen, löschen, entfernen, Zeile, Spalte, Zelle, verschieben, hinzufügen, leer, Strg+Plus, Strg+Minus, Rechtsklick, Kopfzeile, rückgängig, Bezug, #BEZUG!, Formel, Name, verbunden, Tabelle, Matrix, Rand -->

Sie können leere Zeilen, Spalten oder Zellen mitten in Ihre Daten einfügen und Zeilen, Spalten oder Zellen entfernen. Alles darunter rückt dann nach, um Platz zu schaffen oder die Lücke zu schließen. Formeln, die auf verschobene Zellen verweisen, zeigen weiterhin auf dieselben Zellen.

## Einfügen

1. Markieren Sie die Zeilennummern, die Spaltenbuchstaben oder die Zellen, an deren Stelle die neuen eingefügt werden sollen. Um drei Zeilen hinzuzufügen, markieren Sie drei Zeilen.
2. Drücken Sie **Strg und die Plustaste** (**Strg und die Minustaste** löscht), verwenden Sie **Start > Einfügen** oder klicken Sie die Markierung mit der rechten Maustaste an und wählen Sie **Einfügen**.

Sind ganze Zeilen oder Spalten markiert, werden die neuen sofort eingefügt: die Zeilen oberhalb der markierten Zeilen, die Spalten links von den markierten Spalten. Sind Zellen markiert, fragt ein kleines Fenster, wie die nachfolgenden Zellen verschoben werden sollen:

- **Zellen nach rechts verschieben** oder **Zellen nach unten verschieben** verschiebt nur die Zellen in den Spalten (bzw. Zeilen) Ihrer Markierung. Der übrige Teil des Blatts bleibt, wo er ist.
- **Gesamte Zeile** oder **Gesamte Spalte** fügt ganze Zeilen bzw. Spalten ein.

## Ausgeschnittene oder kopierte Zellen einfügen

Um Zellen, die Sie ausgeschnitten oder kopiert haben, zwischen andere Zellen statt über sie zu setzen, schneiden Sie sie zuerst aus oder kopieren Sie sie (**Strg+X** bzw. **Strg+C**). Markieren Sie dann die Zelle, an der sie stehen sollen, und verwenden Sie **Start > Einfügen > Ausgeschnittene oder kopierte Zellen einfügen** (der Befehl heißt **Ausgeschnittene Zellen einfügen** oder **Kopierte Zellen einfügen**, sobald etwas zum Einfügen bereitsteht). Alternativ klicken Sie die Zelle mit der rechten Maustaste an und wählen den Befehl. Ein kleines Fenster fragt, ob die vorhandenen Zellen **nach rechts** oder **nach unten** verschoben werden sollen, um Platz zu schaffen.

Die Schaltfläche **Einfügen** und **Strg und die Plustaste** (auch **Strg+Umschalt und die Plustaste**) tun dies ebenfalls, aber nur, wenn eindeutig ist, was Sie meinen:

- wenn Zellen **ausgeschnitten** wurden und der Ausschnitt noch wartet (das erkennen Sie an dem gestrichelten Rahmen) auf dem Blatt, aus dem sie stammen, und
- wenn Zellen **kopiert** wurden und Sie seitdem nichts geändert haben (keine Eingabe, keine andere Bearbeitung, kein Rückgängig oder Wiederholen).

In allen anderen Fällen fügen die Schaltfläche und die Tasten wie bisher leere Zellen ein. Nach einer Änderung bleibt eine Kopie für **Einfügen** und den Befehl im Menü erhalten. Mit **Esc** beenden Sie das Ausschneiden oder Kopieren: Der Rahmen verschwindet, und die Schaltfläche und die Tasten fügen wieder leere Zellen ein. Wenn Sie Zellen auf einem Blatt ausschneiden und die Tasten auf einem anderen Blatt drücken, werden leere Zellen eingefügt. Der Befehl im Menü zeigt an, dass ausgeschnittene Zellen nur auf ihrem eigenen Blatt zwischen andere Zellen gesetzt werden können. Um sie auf ein anderes Blatt zu verschieben, verwenden Sie **Einfügen**. Sind ganze Zeilen oder Spalten markiert, ist der Befehl nicht verfügbar, und **Einfügen** fügt wie bisher Zeilen oder Spalten ein.

- Ausgeschnittene Zellen werden **verschoben**: Sie verlassen ihren alten Platz, und was auf sie verweist, folgt ihnen. Das entspricht dem Halten der **Umschalttaste** beim Ziehen am Rand der Markierung. Der Vorgang gilt ganz oder gar nicht: Wird ein Teil abgelehnt, ändert sich nichts.
- Kopierte Zellen werden als Kopie eingefügt, so wie **Einfügen** sie einfügen würde (wie es unter **Standardeinfügen festlegen** beschrieben ist, und mit an den neuen Platz angepassten Formeln). Die Kopie bleibt bereit, sodass Sie sie an anderer Stelle erneut einfügen können.
- Die Zellen werden einmal eingefügt, und zwar in der oberen linken Zelle Ihrer Markierung, auch wenn Sie mehr Zellen markiert haben, als Sie kopiert haben.
- Ist die Liste gefiltert, werden die Zellen in die sichtbaren Zeilen eingefügt, wie es auch beim Einfügen geschieht. Der dafür geschaffene Platz umfasst auch die dazwischenliegenden Zeilen, die der Filter ausblendet. Eine von Hand ausgeblendete Zeile zählt wie jede andere Zeile.
- Jeder dieser Vorgänge ist ein Schritt, den Sie rückgängig machen können. Ausgeschnittene Zellen können nur auf dem Blatt zwischen andere Zellen gesetzt werden, auf dem sie ausgeschnitten wurden.
- Eine Zelle kann nicht in ihren eigenen Weg eingefügt werden (Zellen, die aus A3:A4 ausgeschnitten wurden, können nicht zwischen A3 und A4 eingefügt werden). Ein geschütztes Blatt lehnt den Vorgang ab. JeffCrunch nennt den Grund und ändert nichts.

## Löschen

Markieren Sie, was Sie entfernen möchten, und drücken Sie **Strg und die Minustaste**, oder verwenden Sie **Start > Löschen**. Bei markierten Zellen fragt das Fenster, ob die Zellen **nach links verschoben**, **nach oben verschoben** oder die **gesamte Zeile** bzw. **gesamte Spalte** gelöscht werden sollen. Das Löschen ist ein einziger Schritt. Mit **Rückgängig** stellen Sie alles wieder her, was dabei entfernt wurde, einschließlich der Formeln.

## Was danach geschieht

Eine Formel, die auf Zellen verweist, die verschoben werden, verweist danach auf sie an ihrem neuen Platz. Beispiel: In B1 steht **=SUMME(A1:A5)**. Fügen Sie eine Zeile oberhalb von Zeile 3 ein, dann wird die Formel zu **=SUMME(A1:A6)**, weil der Bereich um die neue Zeile größer geworden ist. Löschen Sie Zeile 3, wird daraus **=SUMME(A1:A4)**. Eine Formel, die auf eine gelöschte Zelle verwiesen hat, zeigt **#BEZUG!** an. Das bedeutet, dass die benötigte Zelle nicht mehr vorhanden ist. Mit **Rückgängig** bringen Sie die Zelle und die Formel zurück.

Folgende Elemente werden ebenfalls mit den Zellen verschoben: Formeln auf allen Blättern, Namen (etwa Druckbereiche und Drucktitel), verbundene Zellen, die Zellen, für die Gültigkeitsregeln und bedingte Formate gelten, Tabellen, Diagramme, Bilder, Notizen, Hyperlinks und Seitenumbrüche.

## Was nicht möglich ist

- Zellen können nicht über einen Teil einer **Matrixformel** oder einer Datentabelle hinweg eingefügt oder gelöscht werden. Verschieben Sie sie vollständig oder lassen Sie sie unverändert. Eine Zelle, die eine Datentabelle als Eingabe liest, kann ebenfalls nicht gelöscht werden.
- Eine **Pivot-Tabelle** kann bislang nicht verschoben oder durchschnitten werden: Zeilen, Spalten oder Zellen können nicht dort eingefügt oder gelöscht werden, wo dadurch die Pivot-Tabelle verschoben oder verändert würde, und Zellen können weder aus ihr heraus noch in sie hinein ausgeschnitten werden. Der übrige Teil des Blatts lässt sich wie gewohnt bearbeiten. Eine Pivot-Tabelle ändern Sie mit den Befehlen der Registerkarte **Pivot-Tabelle** (Aktualisieren, Datenquelle ändern, Felder und Layout), nicht durch Eingabe in ihren Zellen.
- Eine **Tabelle** kann nicht quer durchschnitten werden: Sie können keine Spalte in ihr einfügen und keine Kopfzeile oder alle Zeilen der Tabelle löschen. Fügen oder löschen Sie Zeilen innerhalb der Tabelle oder ändern Sie die Zellen um sie herum.
- Bei einem **geschützten Blatt** können keine Zeilen, Spalten oder Zellen eingefügt oder gelöscht werden. Um das zu ermöglichen, wählen Sie auf der Registerkarte **Überprüfen** den Befehl **Blattschutz aufheben**.
- Zellen am Ende des Blatts, die Inhalte enthalten, können nicht über den Rand hinausgeschoben werden. Löschen Sie sie zuerst.
- Nach dem Speichern reicht **Rückgängig** weiterhin bis zu Vorgängen zurück, die Sie vorher ausgeführt haben, einschließlich der eingefügten Zeilen und Spalten (beim nächsten Speichern werden sie wieder entfernt). Nicht zurückholen lassen sich jedoch gelöschte Zeilen, Spalten oder Zellen: Die gespeicherte Datei hat deren Inhalt verloren. Deshalb reicht **Rückgängig** nur bis zum letzten gespeicherten Löschvorgang, und die Zeile am unteren Fensterrand weist beim Speichern darauf hin.
- Ein Vorgang, der so viele Zellen verschiebt oder löscht, dass **Rückgängig** ihn nicht speichern kann (mehr als etwa 128 MB), fragt vorher nach, wie es auch die bekannten Tabellenkalkulationen tun: „Dies kann nicht rückgängig gemacht werden.“ Wenn Sie fortfahren, wird der Vorgang ausgeführt, und **Rückgängig** geht weder über ihn noch über vorherige Vorgänge zurück.
