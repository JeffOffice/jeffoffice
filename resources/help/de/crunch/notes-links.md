# Notizen und Links
<!-- keywords: Thesaurus, Synonyme, Wortliste, Notiz, Notizen, Kommentar, Kommentare, Zeiger, roter Punkt, Ecke, neue Notiz, Notiz bearbeiten, Notiz löschen, Notiz anzeigen, Notiz ausblenden, alle Notizen anzeigen, Link, Links, Hyperlink, Adresse, Webseite, E-Mail, Stelle in dieser Arbeitsmappe, Strg+K, Strg+Klick, QuickInfo, Link öffnen, Link entfernen, Unterhaltung, Arbeitsmappenstatistik, Anzahl, Zellen, Formeln, Diagramme, Pivot-Tabellen, Barrierefreiheit, Alternativtext, verbundene Zellen, zuletzt verwendete Dateien -->

## Notizen

Eine **Notiz** ist ein paar Wörter, die zu einer Zelle gespeichert werden. Eine Zelle, die eine Notiz hat, zeigt in ihrer oberen rechten Ecke einen kleinen roten Punkt. Wenn Sie mit dem Zeiger auf der Zelle verweilen, erscheint die Notiz daneben; bewegen Sie den Zeiger weg, verschwindet sie wieder.

Um eine Notiz zu einer Zelle hinzuzufügen, markieren Sie die Zelle und verwenden **Überprüfen > Neue Notiz** (oder drücken Sie **Umschalt+F2**). Geben Sie den Text ein und klicken Sie auf **OK**. Bei einer Zelle, die bereits eine Notiz hat, heißt derselbe Befehl **Notiz bearbeiten**. Wenn Sie den Text löschen, wird die Notiz entfernt.

- **Notiz löschen** entfernt die Notizen der markierten Zellen.
- **Notiz anzeigen oder ausblenden** zeigt die Notiz der markierten Zellen dauerhaft im Blatt an, in einem Feld neben jeder Zelle, oder blendet sie wieder aus.
- **Alle Notizen anzeigen** tut dies für alle Notizen im Blatt; die Schaltfläche heißt dann **Alle Notizen ausblenden**.
- **Vorherige Notiz** und **Nächste Notiz** springen von einer Zelle mit Notiz zur nächsten.

Jeden dieser Schritte können Sie mit **Rückgängig** zurücknehmen. Eine Notiz wandert mit ihrer Zelle mit, wenn Zeilen oder Spalten eingefügt oder gelöscht werden und wenn Zellen sortiert werden. **Alle löschen** entfernt die Notizen der Zellen, die es leert.

Eine Arbeitsmappe kann eine Unterhaltung aus Kommentaren mit Antworten enthalten. JeffCrunch zeigt die gesamte Unterhaltung als Notiz an und behält sie beim Speichern bei, verändert sie aber nicht: Die Notiz ist schreibgeschützt.

Notizen können in einer .xlsx-Arbeitsmappe gespeichert werden. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, behält ihre Notizen erst, nachdem Sie sie als .xlsx-Arbeitsmappe speichern.

## Links

Ein **Link** führt Sie beim Anklicken an einen anderen Ort: zu einer Webseite, zu einer Datei, zu einer E-Mail-Adresse oder zu einer Stelle in dieser Arbeitsmappe (einer Zelle, mehreren Zellen oder einem Namen).

Um einen Link zu einer Zelle hinzuzufügen, markieren Sie die Zelle und verwenden **Einfügen > Link** (oder drücken Sie **Strg+K**). Wählen Sie das Ziel, geben Sie die Adresse oder die Stelle ein (oder wählen Sie unter **Oder eine zuletzt verwendete Datei** eine der zuletzt geöffneten Dateien aus) und, falls gewünscht, den Text, den die Zelle anzeigen soll, sowie eine **QuickInfo**. Bei einer Zelle, die bereits einen Link hat, heißt derselbe Befehl **Link bearbeiten**. **Link entfernen** entfernt die Links der markierten Zellen.

Wenn Sie mit dem Zeiger auf einer Zelle mit Link verweilen, sehen Sie, wohin er führt. Um ihm zu folgen, halten Sie **Strg** gedrückt und klicken auf die Zelle, oder markieren Sie die Zelle und verwenden **Link öffnen** aus dem Kontextmenü. Ein einfacher Klick markiert nur die Zelle.

JeffCrunch öffnet nie von selbst etwas. Eine Webseite oder eine E-Mail-Adresse wird beim Folgen des Links in Ihrem üblichen Programm geöffnet. Bei einem Link zu einer Datei oder zu einer anderen Art von Adresse wird vorher nachgefragt. Ein Link, der ein Skript ausführen würde, wird nie geöffnet.

Ein Link wandert mit seiner Zelle mit, wenn Zeilen oder Spalten eingefügt oder gelöscht werden und wenn Zellen sortiert werden, und er wird mit der Arbeitsmappe gespeichert.

## Arbeitsmappenstatistik

Die **Arbeitsmappenstatistik** (Registerkarte **Überprüfen**) zählt, was die Arbeitsmappe enthält: wie viele **Blätter** sie hat, wie viele **Zellen mit Daten** (Zellen, die eine Zahl, einen Text oder eine Formel enthalten), wie viele **Formeln**, wie viele **Diagramme** und wie viele **Pivot-Tabellen**. Gelöschte Blätter werden nicht mitgezählt. Dieselben Zahlen finden Sie auf der Seite **Info** des Menüs Datei.

## Thesaurus

Der **Thesaurus** (Registerkarte **Überprüfen** oder **Umschalt+F7**) findet Wörter, die fast dasselbe bedeuten wie ein Wort in einer Zelle. Er sucht das erste Wort des Texts in der Zelle, auf der Sie gerade stehen. Geben Sie ein anderes Wort in **Nachschlagen** ein und drücken Sie **Eingabe**, um stattdessen dieses nachzuschlagen. Wählen Sie ein Wort in der Liste aus und klicken Sie auf **Ersetzen**, oder doppelklicken Sie darauf. Das Wort in der Zelle wird ersetzt; ein großer Anfangsbuchstabe bleibt groß. **Nachschlagen** schlägt das ausgewählte Wort nach, um einer Bedeutungskette zu folgen. Die Wortliste funktioniert auf Englisch.

## Barrierefreiheit prüfen

**Barrierefreiheit prüfen** (Registerkarte **Überprüfen**) öffnet ein kleines Fenster, das auflistet, was in der Arbeitsmappe für manche Leser schwer zu benutzen ist oder was ein Bildschirmleseprogramm (ein Programm, das den Bildschirm laut vorliest) nicht gut wiedergeben kann. Jede Zeile beginnt mit **Fehler**, **Warnung** oder **Tipp**. Klicken Sie auf eine Zeile, um zur Zelle zu gelangen.

Es sucht nach:

- einem Diagramm oder Bild ohne **Alternativtext** (den Worten, die beschreiben, was es zeigt). Das ist ein Fehler.
- einer Tabelle ohne Kopfzeile, sodass ihre Spalten keine Namen haben. Das ist ein Fehler.
- **verbundenen Zellen**, die sich mit der Tastatur schwer durchlaufen lassen. Das ist eine Warnung.
- Text, der sich vor der Hintergrundfarbe schlecht abhebt, etwa hellgrauer Text auf Weiß. JeffCrunch misst, wie stark sich der Text von der Füllung abhebt. Kleiner Text braucht ein Verhältnis von mindestens 4,5 zu 1, großer Text (18 Punkt oder fett 14 Punkt) braucht 3 zu 1. Es nennt die erste Zelle mit jeder Kombination aus Schriftart und Füllung. Das ist eine Warnung.
- einem Blatt, das noch den Namen trägt, den es bekommen hat, etwa Tabelle1. Das ist ein Tipp.

Klicken Sie nach Änderungen auf **Erneut prüfen**.
