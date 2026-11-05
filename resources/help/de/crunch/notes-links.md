# Notizen und Links
<!-- keywords: Notiz, Notizen, Kommentar, Kommentare, Zeigen, roter Punkt, Ecke, neue Notiz, Notiz bearbeiten, Notiz löschen, Notiz anzeigen, Notiz ausblenden, alle Notizen anzeigen, Link, Links, Hyperlink, Adresse, Webseite, E-Mail, Stelle in dieser Arbeitsmappe, Strg+K, Strg+Klicken, QuickInfo, Link öffnen, Link entfernen, Unterhaltung, Arbeitsmappenstatistik, Anzahl, Zellen, Formeln, Diagramme, Pivot-Tabellen, Barrierefreiheit, Alternativtext, verbundene Zellen <!-- -->

## Notizen

Eine **Notiz** ist ein kurzer Text, der mit einer Zelle gespeichert wird. Eine Zelle, die eine Notiz hat, zeigt in ihrer oberen rechten Ecke einen kleinen roten Punkt. Zeigen Sie mit dem Mauszeiger auf die Zelle, erscheint die Notiz daneben; bewegen Sie den Zeiger weg, verschwindet sie wieder.

Um eine Notiz an eine Zelle zu setzen, markieren Sie die Zelle und verwenden Sie **Überprüfen > Neue Notiz** (oder drücken Sie **Umschalt+F2**). Geben Sie den Text ein und klicken Sie auf **OK**. Bei einer Zelle mit Notiz heißt derselbe Befehl **Notiz bearbeiten**. Wenn Sie den Text löschen, wird die Notiz entfernt.

- **Notiz löschen** entfernt die Notizen aus den markierten Zellen.
- **Notiz anzeigen oder ausblenden** zeigt die Notiz der markierten Zellen dauerhaft auf dem Blatt an, in einem Kasten neben jeder Zelle, oder blendet sie wieder aus.
- **Alle Notizen anzeigen** macht dasselbe für alle Notizen auf dem Blatt; die Schaltfläche heißt dann **Alle Notizen ausblenden**.
- **Vorherige Notiz** und **Nächste Notiz** springen von einer Zelle mit Notiz zur nächsten.

Jeden dieser Schritte können Sie mit **Rückgängig** zurücknehmen. Eine Notiz wandert mit ihrer Zelle, wenn Zeilen oder Spalten eingefügt oder gelöscht werden und wenn Zellen sortiert werden. **Alle löschen** entfernt die Notizen aus den Zellen, die dabei geleert werden.

Eine Arbeitsmappe kann eine Unterhaltung aus Kommentaren mit Antworten enthalten. JeffCrunch zeigt die gesamte Unterhaltung als Notiz an und behält sie beim Speichern bei, verändert sie aber nicht: Die Notiz ist schreibgeschützt.

Notizen können in einer .xlsx-Arbeitsmappe gespeichert werden. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, behält ihre Notizen erst, wenn Sie sie als .xlsx-Arbeitsmappe speichern.

## Links

Ein **Link** führt Sie beim Anklicken an ein Ziel: zu einer Webseite, zu einer Datei, zu einer E-Mail-Adresse oder zu einer Stelle in dieser Arbeitsmappe (einer Zelle, mehreren Zellen oder einem Namen).

Um einen Link an eine Zelle zu setzen, markieren Sie die Zelle und verwenden Sie **Einfügen > Link** (oder drücken Sie **Strg+K**). Wählen Sie das Ziel, geben Sie die Adresse oder die Stelle ein und, falls gewünscht, den Text, den die Zelle anzeigen soll, sowie eine **QuickInfo**. Bei einer Zelle mit Link heißt derselbe Befehl **Link bearbeiten**. **Link entfernen** nimmt die Links aus den markierten Zellen weg.

Zeigen Sie mit dem Mauszeiger auf eine Zelle mit Link, um zu sehen, wohin er führt. Um ihm zu folgen, halten Sie **Strg** gedrückt und klicken Sie auf die Zelle, oder markieren Sie die Zelle und verwenden Sie **Link öffnen** aus dem Kontextmenü (Rechtsklick). Ein einfacher Klick markiert nur die Zelle.

JeffCrunch öffnet nie von selbst etwas. Eine Webseite oder eine E-Mail-Adresse wird in Ihrem üblichen Programm geöffnet, wenn Sie ihrem Link folgen. Bei einem Link zu einer Datei oder zu einer anderen Art von Adresse fragt das Programm zuerst nach. Ein Link, der ein Skript ausführen würde, wird nie geöffnet.

Ein Link wandert mit seiner Zelle, wenn Zeilen oder Spalten eingefügt oder gelöscht werden und wenn Zellen sortiert werden. Er wird mit der Arbeitsmappe gespeichert.

## Arbeitsmappenstatistik

Die **Arbeitsmappenstatistik** (Registerkarte **Überprüfen**) zählt, was die Arbeitsmappe enthält: wie viele **Blätter** sie hat, wie viele **Zellen mit Daten** (Zellen mit einer Zahl, einem Text oder einer Formel), wie viele **Formeln**, wie viele **Diagramme** und wie viele **Pivot-Tabellen**. Gelöschte Blätter werden nicht mitgezählt. Dieselben Zahlen finden Sie auf der Seite **Info** der Registerkarte Datei.

## Barrierefreiheit prüfen

**Barrierefreiheit prüfen** (Registerkarte **Überprüfen**) öffnet ein kleines Fenster, das auflistet, was in der Arbeitsmappe für manche Leser schwer zu benutzen ist oder was ein Bildschirmleseprogramm (ein Programm, das den Bildschirm vorliest) nicht gut wiedergeben kann. Jede Zeile beginnt mit **Fehler**, **Warnung** oder **Tipp**. Klicken Sie auf eine Zeile, um zur Zelle zu springen.

Die Prüfung sucht nach:

- Einem Diagramm oder Bild ohne **Alternativtext** (den Worten, die beschreiben, was es zeigt). Das ist ein Fehler.
- Einer Tabelle ohne Kopfzeile, sodass ihre Spalten keine Namen haben. Das ist ein Fehler.
- **Verbundenen Zellen**, die sich mit der Tastatur nur schwer durchlaufen lassen. Das ist eine Warnung.
- Einem Blatt, das noch den Namen trägt, den es bekommen hat, etwa Tabelle1. Das ist ein Tipp.

Drücken Sie nach Änderungen auf **Erneut prüfen**.
