# Notizen und Links
<!-- keywords: Notiz, Notizen, Kommentar, Kommentare, Zeigen, roter Punkt, Ecke, neue Notiz, Notiz bearbeiten, Notiz löschen, Notiz anzeigen, Notiz ausblenden, alle Notizen anzeigen, Link, Links, Hyperlink, zuletzt verwendete Dateien, Adresse, Webseite, E-Mail, Stelle in dieser Arbeitsmappe, Strg+K, Strg+Klicken, QuickInfo, Link öffnen, Link entfernen, Unterhaltung, Arbeitsmappenstatistik, zählen, Zellen, Formeln, Diagramme, Pivot-Tabellen, Barrierefreiheit, Alternativtext, verbundene Zellen

## Notizen

Eine **Notiz** ist ein kurzer Text, der an einer Zelle hängt. Eine Zelle, die eine Notiz hat, zeigt in ihrer oberen rechten Ecke einen kleinen roten Punkt. Zeigen Sie mit der Maus auf die Zelle, erscheint die Notiz neben ihr; bewegen Sie den Mauszeiger weg, verschwindet sie wieder.

Um eine Notiz an einer Zelle anzubringen, markieren Sie die Zelle und verwenden **Überprüfen > Neue Notiz** (oder drücken Sie **Umschalt+F2**). Geben Sie den Text ein und klicken Sie auf **OK**. Bei einer Zelle, die bereits eine Notiz hat, heißt derselbe Befehl **Notiz bearbeiten**. Wenn Sie den Text löschen, wird die Notiz entfernt.

- **Notiz löschen** entfernt die Notizen der markierten Zellen.
- **Notiz ein-/ausblenden** zeigt die Notiz der markierten Zellen dauerhaft im Blatt an, in einem Feld neben jeder Zelle, oder blendet sie wieder aus.
- **Alle Notizen anzeigen** tut dasselbe für alle Notizen im Blatt; die Schaltfläche heißt dann **Alle Notizen ausblenden**.
- **Vorherige Notiz** und **Nächste Notiz** springen von einer Zelle mit Notiz zur nächsten.

Jeder dieser Schritte lässt sich mit **Rückgängig** zurücknehmen. Eine Notiz wandert mit ihrer Zelle, wenn Zeilen oder Spalten eingefügt oder gelöscht werden und wenn Zellen sortiert werden. **Alles löschen** entfernt die Notizen der Zellen, die gelöscht werden.

Eine Arbeitsmappe kann eine Unterhaltung aus Kommentaren mit Antworten enthalten. JeffCrunch zeigt die gesamte Unterhaltung als Notiz an und behält sie beim Speichern bei, verändert sie aber nicht: Die Notiz ist nur lesbar.

Notizen können in einer .xlsx-Arbeitsmappe gespeichert werden. Eine Arbeitsmappe, die aus einem älteren Dateityp oder aus einer Textdatei geöffnet wurde, behält ihre Notizen erst, nachdem Sie sie als .xlsx-Arbeitsmappe speichern.

## Links

Ein **Link** führt Sie beim Anklicken an ein Ziel: zu einer Webseite, zu einer Datei, zu einer E-Mail-Adresse oder zu einer Stelle in dieser Arbeitsmappe (einer Zelle, einem Zellbereich oder einem Namen).

Um einen Link an einer Zelle anzubringen, markieren Sie die Zelle und verwenden **Einfügen > Link** (oder drücken Sie **Strg+K**). Wählen Sie das Ziel, geben Sie die Adresse oder die Stelle ein (oder wählen Sie unter **Oder eine zuletzt verwendete Datei** eine der Dateien aus, die Sie zuletzt geöffnet haben) und gegebenenfalls den Text, den die Zelle anzeigen soll, sowie eine **QuickInfo**. Bei einer Zelle, die bereits einen Link hat, heißt derselbe Befehl **Link bearbeiten**. **Link entfernen** entfernt die Links der markierten Zellen.

Zeigen Sie mit der Maus auf eine Zelle mit Link, um zu sehen, wohin er führt. Um ihm zu folgen, halten Sie **Strg** gedrückt und klicken Sie auf die Zelle, oder markieren Sie die Zelle und wählen Sie **Link öffnen** aus dem Kontextmenü. Ein einfacher Klick markiert nur die Zelle.

JeffCrunch öffnet nie von selbst etwas. Eine Webseite oder eine E-Mail-Adresse öffnet sich in Ihrem üblichen Programm, wenn Sie dem Link folgen. Ein Link auf eine Datei oder auf eine andere Art von Adresse fragt Sie zuvor nach. Ein Link, der ein Skript ausführen würde, wird nie geöffnet.

Ein Link wandert mit seiner Zelle, wenn Zeilen oder Spalten eingefügt oder gelöscht werden und wenn Zellen sortiert werden. Er wird zusammen mit der Arbeitsmappe gespeichert.

## Arbeitsmappenstatistik

Die **Arbeitsmappenstatistik** (Registerkarte **Überprüfen**) zählt, was die Arbeitsmappe enthält: wie viele **Blätter** sie hat, wie viele **Zellen mit Daten** (Zellen mit einer Zahl, einem Text oder einer Formel), wie viele **Formeln**, wie viele **Diagramme** und wie viele **Pivot-Tabellen**. Gelöschte Blätter werden nicht mitgezählt. Dieselben Angaben finden Sie auch auf der Seite **Info** der Registerkarte **Datei**.

## Barrierefreiheit prüfen

**Barrierefreiheit prüfen** (Registerkarte **Überprüfen**) öffnet ein kleines Fenster, das auflistet, was in der Arbeitsmappe für manche Leserinnen und Leser schwer zu nutzen ist oder was ein Screenreader (ein Programm, das den Bildschirminhalt vorliest) nicht gut wiedergeben kann. Jede Zeile beginnt mit **Fehler**, **Warnung** oder **Tipp**. Klicken Sie auf eine Zeile, um zur Zelle zu gelangen.

Es prüft auf Folgendes:

- Ein Diagramm oder Bild ohne **Alternativtext** (die Worte, die beschreiben, was es zeigt). Das ist ein Fehler.
- Eine Tabelle ohne Kopfzeile, sodass ihre Spalten keine Namen haben. Das ist ein Fehler.
- **Verbundene Zellen**, die sich mit der Tastatur nur schwer durchlaufen lassen. Das ist eine Warnung.
- Text, der sich vor der Hintergrundfarbe schlecht lesen lässt, etwa hellgrauer Text auf Weiß. JeffCrunch misst, wie stark sich der Text von der Füllung abhebt. Kleiner Text braucht ein Verhältnis von mindestens 4,5 zu 1, großer Text (18 Punkt oder fett 14 Punkt) braucht 3 zu 1. Es nennt die erste Zelle mit jeder Kombination aus Schriftart und Füllung. Das ist eine Warnung.
- Ein Blatt, das noch den Namen trägt, den es beim Anlegen erhalten hat, etwa Tabelle1. Das ist ein Tipp.

Drücken Sie nach Änderungen auf **Erneut prüfen**.
