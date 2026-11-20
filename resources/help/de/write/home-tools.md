# Groß-/Kleinschreibung ändern, sortieren und auf andere Weise einfügen
<!-- keywords: Einfügen Schaltfläche, intelligentes Ausschneiden und Einfügen, Leerzeichen beim Einfügen, horizontale Linie, Linie, Formatierung anzeigen, Formatvorlagen-Inspektor, Umschalt+F1, Tabstopps, Tabulator, Balkentabstopp, Balken, Standardtabstopps, Tabstoppabstand, Füllzeichen, Mit nächstem Absatz, Zeilen zusammenhalten, Seitenwechsel davor, Hurenkind, Schusterjunge, Gliederungsebene, Zeilen- und Seitenumbrüche, Groß-/Kleinschreibung, Großbuchstaben, Kleinbuchstaben, Satzbeginn groß, Großschreiben, Umschalt+F3, sortieren, Absätze sortieren, alphabetisch, Reihenfolge, einfügen, Inhalte einfügen, Liste fortsetzen, neue Liste, Liste zusammenführen, Tabelle verschachteln, Tabellenkalkulationszellen, nur Text übernehmen, Formatierung zusammenführen, RTF, Rich Text, formatierter Text, ziehen, Text ziehen, unformatierter Text, Bild, Zwischenablage -->

Diese Werkzeuge befinden sich auf der Registerkarte **Start**. Sie wirken auf den markierten Text.

## Groß- und Kleinschreibung von Buchstaben ändern

Die **Schreibweise** eines Buchstabens gibt an, ob er ein Großbuchstabe (A) oder ein Kleinbuchstabe (a) ist.

1. Markieren Sie den Text. Ist nichts markiert, ändert JeffWrite das Wort, in dem sich der Cursor befindet.
2. Klicken Sie in der Gruppe **Schriftart** auf **Groß-/Kleinschreibung**.
3. Wählen Sie eine Option:
   - **Satzbeginn groß** schreibt den ersten Buchstaben jedes Satzes groß. Der Rest bleibt klein.
   - **kleinbuchstaben** schreibt alle Buchstaben klein.
   - **GROSSBUCHSTABEN** schreibt alle Buchstaben groß.
   - **Wörter groß schreiben** schreibt den ersten Buchstaben jedes Wortes groß.
   - **gROSS-/kLEINSCHREIBUNG uMKEHREN** macht aus jedem Großbuchstaben einen Kleinbuchstaben und aus jedem Kleinbuchstaben einen Großbuchstaben.

Mit **Umschalt+F3** durchlaufen Sie drei dieser Optionen nacheinander: **Wörter groß schreiben**, dann **GROSSBUCHSTABEN**, dann **kleinbuchstaben**.

Das Aussehen der Buchstaben bleibt gleich. Fett bleibt fett. Mit **Strg+Z** machen Sie die Änderung in einem Schritt rückgängig. Ist **Änderungen nachverfolgen** aktiviert, wird die Änderung als Löschung und Einfügung angezeigt.

## Absätze sortieren

Sortieren bringt Absätze in eine Reihenfolge, etwa von A bis Z. Verwenden Sie es für eine Liste von Namen oder für Zeilen einer Tabelle, die mit Tabstopps oder Kommas gegliedert ist.

1. Markieren Sie zwei oder mehr Absätze.
2. Klicken Sie in der Gruppe **Absatz** auf **Sortieren**.
3. Wählen Sie, wonach sortiert werden soll. Jede Auswahl hat ein **Feld**, einen **Typ** und eine **Reihenfolge**.
   - Das **Feld** ist ein Teil des Absatzes. Feld 1 ist der erste Teil. Sind die Teile durch Kommas oder Tabstopps getrennt, geben Sie das Trennzeichen im Feld unter den Sortierschlüsseln an. Belassen Sie es bei **Absätze**, ist der gesamte Absatz das eine Feld.
   - Der **Typ** ist **Text**, **Zahl** oder **Datum**. Eine Zahl wird nach ihrem Wert sortiert, sodass 9 vor 10 kommt.
   - Die **Reihenfolge** ist **Aufsteigend** (A bis Z, klein bis groß, alt bis neu) oder **Absteigend**.
4. Verwenden Sie **Dann nach**, um Gleichstände aufzulösen.
5. Aktivieren Sie **Die erste Zeile enthält Überschriften**, damit sie an ihrem Platz bleibt.
6. Klicken Sie auf **OK**.

Jeder Absatz behält sein eigenes Absatzformat. Sortieren ist nicht möglich, solange **Änderungen nachverfolgen** aktiviert ist, denn ein Sortieren ist keine Änderung, die sich anzeigen lässt. Schalten Sie **Änderungen nachverfolgen** aus, sortieren Sie und schalten Sie es danach wieder ein.

## Auf andere Weise einfügen

Klicken Sie auf den kleinen Pfeil unter **Einfügen**, um weitere Möglichkeiten zu sehen:

- Stammt der Text aus einem anderen Programm, das formatierten Text (RTF) in die Zwischenablage legt, behält **Strg+V** sein Aussehen: Schriftarten, Größen, Farben, Fett, Listen, Tabellen, Links und Bilder. Hat das Programm keinen formatierten Text geliefert, verwendet JeffWrite die Webseitenform (HTML) und danach reine Wörter. Formatierter Text über 16 MB wird nicht gelesen; das Einfügen greift dann auf die nächste Form zurück. Das Ziehen von formatiertem Text aus einem anderen Programm auf die Seite funktioniert genauso.
- **Formatierung zusammenführen** fügt den Text im Aussehen der Stelle ein, an die er kommt. Nur Fett, Kursiv, Unterstreichung und Durchstreichung bleiben erhalten.
- **Nur Text übernehmen** fügt die Wörter ohne jede Formatierung ein.
- **Bild** fügt ein Bild aus der Zwischenablage ein. Die Auswahl ist abgeblendet, wenn die Zwischenablage kein Bild enthält.
- **Inhalte einfügen** (**Strg+Alt+V**) listet alle Arten auf, in denen die Zwischenablage jetzt eingefügt werden kann, und lässt Sie eine auswählen, etwa **Formatierter Text (RTF)**, **Unformatierter Text** oder **HTML**.

Eine Auswahl ist abgeblendet, wenn die Zwischenablage nichts enthält, was sie verwenden kann.

Direkt nach dem Einfügen erscheint neben dem Cursor eine kleine Schaltfläche **Einfügen**. Klicken Sie darauf, um **Quellformatierung beibehalten**, **Formatierung zusammenführen** oder **Nur Text übernehmen** zu wählen. JeffWrite nimmt das Einfügen zurück und führt es auf diese Weise erneut aus. Die Änderung ist ein einziger Schritt: Ein einzelner Befehl **Rückgängig** nimmt das gesamte Einfügen zurück, egal wie oft Sie es sich anders überlegt haben. Über die Tastatur tippen Sie nach dem Einfügen auf die Taste **Strg** (auf dem Mac die Taste **Control**), um das Menü der Schaltfläche zu öffnen, und wählen dann mit den Pfeiltasten und **Eingabe** aus. Die Schaltfläche verschwindet, sobald Sie etwas eingeben, den Cursor bewegen oder **Esc** drücken, und sie wird in einem Dokument, das Sie nicht ändern dürfen, nicht angeboten.

Hat das Einfügen Listenabsätze neben einer Liste gebracht, bietet die Schaltfläche außerdem **Liste fortsetzen** (die Nummern laufen von der danebenliegenden Liste weiter), **Neue Liste** (die Nummerierung beginnt wieder bei 1) und **Liste zusammenführen** (die Elemente werden an die danebenliegende Liste angefügt und übernehmen deren Aussehen). Normales **Strg+V** fügt die Elemente so ein, wie sie kamen; eine Auswahl ändert das. Dieselben Möglichkeiten stehen als Befehle im Menü **Einfügen** auf der Registerkarte **Start** und in **Inhalte einfügen** zur Verfügung, sodass Sie sie auch über die Tastatur erreichen.

Enthält die Zwischenablage Tabellenkalkulationszellen (aus JeffCrunch oder einem anderen Programm kopiert, als Tabelle oder als Zeilen mit Tabstopps zwischen den Zellen), fügt normales **Strg+V** sie als die Wörter ein, die sie sind. Um sie als Zellen einzufügen, wählen Sie **Tabelle** über die Schaltfläche, das Menü **Einfügen** oder **Inhalte einfügen**. Steht der Cursor in einer Tabelle, bietet die Schaltfläche **Tabelle verschachteln** (eine Tabelle in der Zelle), **Tabelle zusammenführen** (die Zellen werden an die Tabelle angefügt, die dafür die nötigen Spalten erhält), **Als neue Zeilen einfügen**, **Als neue Spalten einfügen** (die Zellen kommen in neue Spalten hinter der Zelle des Cursors, ab dessen Zeile nach unten; eine Tabelle kann höchstens 256 Spalten und 50.000 Zellen haben, und ist das nicht möglich, weil eine verbundene Zelle über die Stelle für die Spalten reicht, wird das Einfügen mit einer Meldung abgelehnt) und **Zellen überschreiben** (die Zellen ersetzen die der Tabelle, ab der Zelle des Cursors). Eine eingefügte Tabelle hat höchstens 500 Zeilen und 63 Spalten, und eine Zelle enthält höchstens 4.000 Zeichen; was darüber hinausgeht, wird weggelassen, und eine Meldung weist darauf hin. Eine Tabelle in einer Tabelle kann höchstens 8 Ebenen tief sein; eine verschachtelte Tabelle, deren Spalten für die Zelle zu schmal würden, wird abgelehnt. Jedes Einfügen ist ein Schritt: **Rückgängig** nimmt alles zurück, und **Wiederholen** stellt es wieder her.

**Intelligentes Ausschneiden und Einfügen** achtet beim Verschieben von Wörtern auf die richtigen Leerzeichen. Schneiden Sie ganze Wörter zwischen zwei Leerzeichen aus, entfernt JeffWrite auch eines der Leerzeichen, sodass kein doppeltes Leerzeichen zurückbleibt. Fügen Sie ein Wort direkt neben einem anderen Wort ein, setzt JeffWrite ein Leerzeichen dazwischen. Die Mitte eines Wortes bleibt unverändert. Zum Ausschalten klicken Sie auf **Datei > Optionen > Erweitert** und deaktivieren **Intelligentes Ausschneiden und Einfügen**.

## Zeilen- und Seitenumbrüche sowie Tabstopps

Klicken Sie auf die kleine Schaltfläche in der Ecke der Gruppe **Absatz**, um das Feld **Absatz** zu öffnen. Unter den Abstandsoptionen finden Sie:

- **Hurenkinder und Schusterjungen vermeiden** verhindert, dass eine einzelne Zeile eines Absatzes allein oben oder unten auf einer Seite steht.
- **Mit nächstem Absatz** hält den Absatz auf derselben Seite wie den folgenden. Verwenden Sie das bei einer Überschrift.
- **Zeilen zusammenhalten** hält alle Zeilen des Absatzes auf einer Seite.
- **Zeilennummern unterdrücken** lässt diesen Absatz aus, wenn die Zeilen nummeriert werden (siehe Layout > Zeilennummern).
- **Seitenwechsel davor** beginnt den Absatz auf einer neuen Seite.
- **Nicht silbentrennen** verhindert, dass die Wörter des Absatzes am Zeilenende mit einem Bindestrich getrennt werden.
- **Kein Abstand zwischen Absätzen derselben Formatvorlage** entfernt den Abstand zwischen zwei Absätzen mit derselben Formatvorlage.
- **Gliederungsebene** legt fest, wo der Absatz in der Gliederung des Dokuments steht: **Fließtext** oder eine Ebene von 1 bis 9.

Klicken Sie im Feld auf **Tabstopps**, um Tabstopps festzulegen. Ein Tabstopp ist eine Stelle, zu der der Text springt, wenn Sie **Tab** drücken. Geben Sie seine Position ein, wählen Sie, wie der Text dort ausgerichtet wird (links, zentriert, rechts oder dezimal), und wählen Sie ein Füllzeichen. Ein Füllzeichen ist die Reihe aus Punkten, Strichen oder eine Linie, die den Raum vor dem Tabstopp ausfüllt. Klicken Sie auf **Festlegen**, um ihn hinzuzufügen. Zum Entfernen wählen Sie ihn in der Liste aus und klicken auf **Löschen**. **Alle löschen** entfernt alle.

Wählen Sie **Balken**, um einen Balkentabstopp festzulegen. Ein Balkentabstopp hält den Text nicht an, sondern zeichnet an dieser Stelle in jeder Zeile des Absatzes eine dünne senkrechte Linie. So setzen Sie eine Trennlinie zwischen Textspalten. Ein Balkentabstopp hat kein Füllzeichen. Das Lineal zeigt Balkentabstopps nicht an. Ein Absatz kann bis zu 64 davon haben. Noch nicht unterstützt: In einem Absatz, der von rechts nach links gelesen wird, werden Balken noch vom linken Rand aus gemessen. Festlegen und Löschen erfolgt im Feld **Tabstopps**.

**Standardtabstopps** ist der Abstand zwischen den Tabstopps, den ein Absatz erhält, wenn er keine eigenen hat, und der nach dem letzten eigenen Tabstopp gilt. Üblich ist ein Abstand von 1,3 cm. Diese Einstellung gilt für das ganze Dokument, nicht nur für den Absatz, in dem Sie gerade stehen. Der neue Abstand gilt für jeden Absatz, sobald Sie auf **OK** klicken.

## Horizontale Linie zeichnen

Klicken Sie in der Gruppe **Absatz** auf **Horizontale Linie**. JeffWrite fügt hinter dem Absatz, in dem der Cursor steht, einen eigenen Absatz mit einer dünnen Linie darunter ein. Der Cursor springt zu einem schlichten Absatz unter der Linie. Zum Entfernen der Linie markieren Sie ihren Absatz und löschen ihn.

## Festlegungen an der Cursorposition anzeigen

Drücken Sie **Umschalt+F1** oder verwenden Sie **Formatierung anzeigen**. Ein Feld listet die Formatvorlagen, die Schriftart, die Schriftgröße, das Aussehen der Buchstaben, die Farbe sowie die Ausrichtung, die Einzüge und die Abstände des Absatzes an der Cursorposition auf. Es zeigt die Festlegungen nur an. Zum Ändern verwenden Sie die Felder **Schriftart** und **Absatz**.
