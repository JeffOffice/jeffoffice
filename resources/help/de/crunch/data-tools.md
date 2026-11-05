# Datentools
<!-- keywords: Daten, Datenregister, Text in Spalten, Aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Summen, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, roter Kreis, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenführen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen -->

Das Register **Daten** enthält die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** im Register **Datei**. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** im Register **Start**. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie diesen Befehl, wenn eine Spalte mehrere Textteile enthält, die in getrennte Spalten sollen, etwa „Schmidt, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie im Register **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **An Trennzeichen aufteilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wobei Sie ein eigenes Zeichen eintragen. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander eine leere Spalte erzeugen. Mit **Text in Anführungszeichen** legen Sie das Zeichen fest, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen stehen zwei Zeichen hintereinander für eines.
   - **An festen Positionen aufteilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei Zeichen ab 1 gezählt werden. Beispiel: Bei 5, 12 beginnt eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Wählen Sie, was jeder Teil ist: **Allgemein** erkennt Zahlen und Datumsangaben so, als hätten Sie sie eingegeben, und **Text** behält jeden Teil unverändert bei, sodass „007“ als „007“ erhalten bleibt.
5. Geben Sie unter **Teile einfügen ab** die erste Zelle für die Teile ein. Zunächst ist es die erste markierte Zelle.
6. Das Feld unter den Optionen zeigt die ersten Zeilen so geteilt, wie Sie es gewählt haben. Klicken Sie auf **OK**.

Die Teile werden in die Zellen rechts neben der ersten Zelle eingefügt und ersetzen dort vorhandenen Inhalt. **Text in Spalten** ist ein Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie diesen Befehl, um von jeder Gruppe sich wiederholender Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Wiederholung gelten. Zwei Zeilen gelten nur dann als Wiederholung, wenn alle aktivierten Spalten gleich sind. Groß- und Kleinschreibung werden nicht unterschieden, und bei einer Formel zählt der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Das Dialogfeld zeigt an, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, zusammen mit ihren Formaten und Formeln. Die Zellen unter der Markierung bleiben unverändert. Die leeren Zeilen am Ende der Markierung werden gelöscht. **Duplikate entfernen** ist ein Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie diesen Befehl, wenn Sie das gewünschte Ergebnis kennen, nicht aber die Zahl, die es ergibt. Beispiel: Eine Zelle mit der Zahlung enthält eine Formel mit einem Zinssatz, und die Zahlung soll 500 betragen. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Zahlung 500 ist.

1. Wählen Sie im Register **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Zunächst ist es die markierte Zelle.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Zu ändernde Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der zu ändernden Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Findet sie keine, bleibt alles unverändert, und Sie erhalten eine Meldung. Versuchen Sie eine andere Startzahl in der zu ändernden Zelle, und prüfen Sie, ob die Formel diese Zelle tatsächlich verwendet. Die **Zielwertsuche** ist ein Schritt, den Sie rückgängig machen können.

## Teilergebnisse

Verwenden Sie diesen Befehl, um unter jede Gruppe von Zeilen mit gleichem Eintrag in einer Spalte eine Zeile mit Summen zu setzen und am Ende eine Gesamtsumme. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der gruppiert wird, damit gleiche Einträge nebeneinander stehen.

1. Markieren Sie eine Zelle in der Liste.
2. Wählen Sie im Register **Daten** in der Gruppe **Gliederung** den Befehl **Teilergebnisse**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden**, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um vorhandene Teilergebnisse zu ersetzen. Klicken Sie auf **OK**.

Jede Ergebniszeile trägt die Bezeichnung „Ost Ergebnis“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, die die anderen Ergebniszeilen auslässt. Die Zeilen jeder Gruppe werden gegliedert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Ergebnisse anzeigen können. Um die Teilergebnisse zu entfernen, öffnen Sie **Teilergebnisse** erneut und klicken auf **Alle entfernen**. Das Hinzufügen und Entfernen von Teilergebnissen ist ein Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. Mit **Erweitert** (in der Gruppe **Sortieren und Filtern**) schreiben Sie die Bedingungen direkt in das Blatt, sodass sie beliebig lang sein können.

1. Kopieren Sie neben die Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile mit leeren Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle zutreffen (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, ob unter **Listenbereich** die Liste mit ihrer Kopfzeile steht, und geben Sie unter **Bedingungsbereich** die Zellen Ihrer Überschriften und Bedingungen ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Die Zeilen an einen anderen Ort kopieren** und geben Sie die erste Zelle des Ziels ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je ein Exemplar zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text, der damit beginnt. Groß- und Kleinschreibung werden nicht unterschieden. **?** steht für ein beliebiges Zeichen und ***** für eine beliebige Folge von Zeichen.
- Eine Zahl wählt Zellen, die ihr entsprechen.
- **=Ann** wählt Text, der genau Ann lautet. **=** allein wählt leere Zellen.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen, die nicht leer sind.

Eine Zeile mit Bedingungen, in der nichts steht, wählt alle Zeilen aus. Um die ausgeblendeten Zeilen wieder anzuzeigen, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. **Erweiterter Filter** ist ein Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie diesen Befehl, um festzulegen, was in bestimmte Zellen eingegeben werden darf, damit eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie in der Gruppe **Datentools** des Registers **Daten** den Befehl **Datenüberprüfung**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen** eine Option: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge wählen Sie unter **Daten** (**zwischen**, **nicht zwischen**, **gleich** und so weiter) und geben die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei **Liste** tippen Sie die Auswahlmöglichkeiten mit Kommas dazwischen ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich ein, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown-Liste in Zelle**, um den Pfeil auszublenden.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. Mit **Leere Zellen ignorieren** dürfen leere Zellen passieren.

Auf der Registerkarte **Eingabemeldung** aktivieren Sie das Kästchen und schreiben einen Titel und eine Meldung. Sie werden neben der Zelle angezeigt, sobald sie ausgewählt ist.

Auf der Registerkarte **Fehlermeldung** wählen Sie, was mit einer unzulässigen Eingabe geschehen soll. **Stopp** lehnt sie ab. **Warnung** fragt, ob sie behalten werden soll. **Information** informiert die Person und behält die Eingabe bei. Schreiben Sie einen Titel und eine Meldung für die Fehlermeldung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) setzt einen roten Kreis um jede Zelle im Blatt, deren Wert die Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel vorhanden war, oder das Ergebnis einer Formel. Eine Meldung teilt Ihnen mit, wenn alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihren Kreis. **Überprüfungskreise entfernen** entfernt alle Kreise.

**Alle löschen** entfernt die Regeln aus den markierten Zellen. Die neue Regel ersetzt alle Regeln, die die markierten Zellen zuvor hatten; andere Zellen behalten ihre Regeln. Das Ändern von Regeln ist ein Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie diesen Befehl, um gleichartige Bereiche, etwa dieselbe Tabelle aus mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie im Register **Daten** in der Gruppe **Datentools** den Befehl **Konsolidieren**.
2. Wählen Sie unter **Funktion** die Funktion, mit der zusammengefasst werden soll, etwa **Summe** oder **Mittelwert**.
3. Geben Sie im Feld einen Bereich ein und klicken Sie auf **Hinzufügen**. Für einen Bereich auf einem anderen Blatt verwenden Sie **Blatt2!A1:C10**. Fügen Sie jeden Bereich hinzu, den Sie zusammenfassen möchten. Markieren Sie einen Bereich in der Liste und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Wenn die Bereiche in ihrer ersten Zeile Namen haben, aktivieren Sie **Obere Zeile enthält Spaltenbeschriftungen**. Wenn sie in ihrer ersten Spalte Namen haben, aktivieren Sie **Linke Spalte enthält Zeilenbeschriftungen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die ihre Quellzellen liest und sich so ändert, wenn diese sich ändern. Ohne dieses Kästchen sind die Ergebnisse reine Zahlen.
7. Klicken Sie auf **OK**.

Ohne aktivierte Namen werden die Zellen an derselben Position in jedem Bereich zusammengefasst (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Namen werden Zellen mit gleichen Namen zusammengefasst, unabhängig davon, wo sie in den einzelnen Bereichen stehen. Ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. **Konsolidieren** ist ein Schritt, den Sie rückgängig machen können.
