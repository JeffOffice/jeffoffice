# Datentools
<!-- keywords: Szenario-Manager, Szenarien, Was-wäre-wenn, Was wäre wenn, Szenarioübersicht, Daten, Datenregister, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Ergebnisse, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, rote Ellipse, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenführen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Werte aktualisieren, Quelle ändern, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Die Registerkarte **Daten** enthält die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte Datei. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art von Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Verknüpfungen bearbeiten

Wenn eine Arbeitsmappe Formeln enthält, die eine andere Arbeitsmappe lesen (etwa ='[Prices.xlsx]Sheet1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Daten abrufen** diese anderen Arbeitsmappen auf, die Quellen heißen. Jede Zeile zeigt, wo die Quelle liegt, und ihren Status. Bis die Arbeitsmappe mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei aus diesen Quellen gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen, deshalb heißt ihr Status, dass sie nicht geprüft wurde.
- **Werte aktualisieren** liest die gewählte Quelle von diesem Computer und gibt jeder Zelle, die nur ein Verweis auf eine ihrer Zellen ist, den Wert, den diese Zelle jetzt hat. Eine Zelle, die mit dem Wert mehr macht, etwa `=[1]Prices!A1*2`, behält die Zahl, die ihre Datei gespeichert hat. Die Meldung unter der Liste sagt, wie viele Zellen aktualisiert wurden und wie viele nicht. Werte aktualisieren ist ein einziger Schritt, deshalb stellt **Rückgängig** die alten Werte wieder her.
- **Quelle ändern** lässt Sie für die gewählte Verknüpfung eine andere Datei auswählen, etwa wenn die Arbeitsmappe verschoben wurde. Die Formeln bleiben unverändert. Der neue Speicherort wird beim Speichern in die Datei geschrieben, und **Rückgängig** nimmt die Änderung zurück.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, sofern sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die auf die gewählte Quelle verweist, in den Wert um, den sie jetzt anzeigt, und einen Namen, der dort für eine Stelle stand, in **#BEZUG!**. Das lässt sich nicht rückgängig machen, deshalb fragt JeffCrunch vorher nach. Wenn keine Verknüpfung mehr übrig ist, wird die Datei ohne die Teile gespeichert, die sie enthielten.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte Start. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie dies, wenn eine Spalte mehrere Textstücke enthält, die in getrennten Spalten stehen sollen, etwa „Müller, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text geteilt werden soll:
   - **An einem Trennzeichen teilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wobei Sie in das Feld ein eigenes Zeichen eintippen. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander eine leere Spalte erzeugen. **Text in Anführungszeichen** legt das Zeichen fest, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen stehen zwei Zeichen hintereinander für eines.
   - **An festen Positionen teilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei Zeichen ab 1 gezählt werden. Beispiel: Bei 5, 12 beginnt eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Legen Sie fest, was jeder Teil ist: **Standard** erkennt Zahlen und Datumsangaben so, als hätten Sie sie eingegeben, und **Text** behält jeden Teil unverändert bei, sodass „007“ als „007“ bleibt.
5. Geben Sie unter **Teile einfügen ab** die erste Zelle für die Teile ein. Sie ist zunächst die erste markierte Zelle.
6. Das Feld unter den Auswahlmöglichkeiten zeigt die ersten Zeilen so, wie Sie sie aufgeteilt haben. Klicken Sie auf **OK**.

Die Teile kommen in die Zellen rechts neben der ersten Zelle und ersetzen dort vorhandenen Inhalt. Text in Spalten ist ein einziger Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie dies, um von jeder Gruppe gleicher Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Duplikate gelten. Zwei Zeilen sind nur dann Duplikate, wenn jede aktivierte Spalte gleich ist. Groß- und Kleinschreibung gelten als gleich, und eine Formel zählt als der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Der Dialog zeigt an, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, mit ihren Formaten und Formeln. Die Zellen unter der Auswahl bewegen sich nicht. Die leeren Zeilen am Ende der Auswahl werden gelöscht. Duplikate entfernen ist ein einziger Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie dies, wenn Sie das gewünschte Ergebnis kennen, aber nicht die Zahl, die es ergibt. Beispiel: Eine Zelle für eine Zahlung enthält eine Formel mit einem Zinssatz, und Sie möchten, dass die Zahlung 500 beträgt. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Zahlung 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie ist zunächst die markierte Zelle.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Zu ändernde Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der zu ändernden Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Findet sie keine, bleibt alles unverändert, und Sie erhalten eine Meldung. Versuchen Sie in der zu ändernden Zelle einen anderen Startwert, und prüfen Sie, ob die Formel tatsächlich diese Zelle verwendet. Die Zielwertsuche ist ein einziger Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist eine benannte Gruppe von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt Was-wäre-wenn-Fälle durchzuspielen. Beispiel: Behalten Sie für dieselben Eingabezellen ein Szenario „Geringe Umsätze“ und ein Szenario „Hohe Umsätze“, und zeigen Sie das gewünschte an.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Szenario-Manager**.
2. Klicken Sie auf **Hinzufügen**. Geben Sie einen Namen ein, etwa Hohe Umsätze. Geben Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario festlegt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die markierten Zellen werden für Sie eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt hat. Tragen Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und klicken Sie auf **OK**.
3. Wählen Sie ein Szenario in der Liste und klicken Sie auf **Anzeigen**, um seine Werte in die Zellen zu übernehmen. Ein Doppelklick auf den Namen hat dieselbe Wirkung. Mit **Bearbeiten** ändern Sie Namen, Kommentar oder Werte. Mit **Löschen** entfernen Sie das Szenario.

Anzeigen, Hinzufügen, Bearbeiten und Löschen sind jeweils ein einziger Schritt, den Sie rückgängig machen können.

Mit **Zusammenfassung** werden alle Szenarien nebeneinander auf einem neuen Blatt namens Szenarioübersicht dargestellt. Der Befehl fragt nach den Ergebniszellen, also den Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Übersicht die Werte der veränderbaren Zellen und das Ergebnis der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte mit der Bezeichnung Aktuelle Werte zeigt, wie die Zellen beim Erstellen der Übersicht aussahen. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine Arbeitsmappe, die mit einem anderen Programm erstellt wurde, behält ihre Szenarien, und Sie sehen sie hier. Eine Kopie, die als älteres .xls-Format oder als .ods gespeichert wird, enthält sie nicht, und beim Speichern wird Sie darauf hingewiesen.

## Teilergebnis

Verwenden Sie dies, um unter jede Gruppe von Zeilen mit demselben Eintrag in einer Spalte eine Ergebniszeile und am Ende eine Gesamtsumme zu setzen. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der Sie gruppieren, damit gleiche Einträge nebeneinander stehen.

1. Wählen Sie eine Zelle in der Liste.
2. Wählen Sie **Teilergebnis** in der Gruppe **Gliederung** auf der Registerkarte **Daten**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden**, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen. Klicken Sie auf **OK**.

Jede Ergebniszeile trägt die Bezeichnung „Ost Gesamt“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, sodass andere Ergebniszeilen nicht mitgezählt werden. Die Zeilen jeder Gruppe werden gegliedert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Ergebnisse anzeigen können. Um die Teilergebnisse zu entfernen, öffnen Sie **Teilergebnis** erneut und klicken auf **Alle entfernen**. Teilergebnisse hinzufügen oder entfernen ist ein einziger Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. Mit **Erweitert** (in der Gruppe **Sortieren und Filtern**) schreiben Sie die Bedingungen auf das Blatt, sodass sie so lang sein können, wie Sie benötigen.

1. Kopieren Sie neben die Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile mit leeren Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle erfüllt sein (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, dass **Listenbereich** die Liste mit ihrer Kopfzeile ist, und geben Sie die Zellen Ihrer Überschriften und Bedingungen unter **Bedingungsbereich** ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Ergebnisse an eine andere Stelle kopieren**, und geben Sie die erste Zelle des Ziels ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je eine Zeile zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges Zeichen und ***** für eine beliebige Zeichenfolge.
- Eine Zahl wählt Zellen aus, die ihr entsprechen.
- **=Ann** wählt Text, der genau Ann lautet. **=** allein wählt leere Zellen.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen, die nicht leer sind.

Eine Bedingungszeile ohne Inhalt wählt jede Zeile aus. Um die ausgeblendeten Zeilen wieder anzuzeigen, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. Erweiterter Filter ist ein einziger Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie dies, um festzulegen, was in bestimmten Zellen eingegeben werden darf, damit eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie **Datenüberprüfung** in der Gruppe **Datentools** auf der Registerkarte **Daten**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge wählen Sie unter **Daten** (**zwischen**, **nicht zwischen**, **gleich**, und so weiter) und geben die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei einer **Liste** geben Sie die Auswahlmöglichkeiten mit Kommas dazwischen ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich ein, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown-Liste in Zelle**, um den Pfeil auszublenden.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. **Leere Zellen ignorieren** lässt eine leere Zelle durch.

Aktivieren Sie auf der Registerkarte **Eingabemeldung** das Kästchen und schreiben Sie einen Titel und eine Meldung. Sie werden neben der Zelle angezeigt, sobald sie markiert ist.

Auf der Registerkarte **Fehlermeldung** legen Sie fest, was mit einer unzulässigen Eingabe geschieht. **Stopp** lehnt sie ab. **Warnung** fragt, ob die Eingabe beibehalten werden soll. **Information** teilt es der Person mit und behält die Eingabe bei. Schreiben Sie einen Titel und eine Meldung für die Fehlermeldung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) zieht eine rote Ellipse um jede Zelle auf dem Blatt, deren Wert die Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel dort stand, oder das Ergebnis einer Formel. Eine Meldung teilt Ihnen mit, wenn alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihre Ellipse. **Überprüfungskreise entfernen** nimmt alle Ellipsen weg.

**Alle löschen** entfernt die Regeln von den markierten Zellen. Die neue Regel ersetzt jede Regel, die die markierten Zellen hatten; andere Zellen behalten ihre eigenen. Das Ändern von Regeln ist ein einziger Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie dies, um gleich aufgebaute Bereiche, etwa dieselbe Tabelle aus mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie auf der Registerkarte **Daten** in der Gruppe **Datentools** den Befehl **Konsolidieren**.
2. Wählen Sie unter **Funktion** die Berechnung, etwa **Summe** oder **Mittelwert**.
3. Geben Sie in das Feld einen Bereich ein und klicken Sie auf **Hinzufügen**. Für einen Bereich auf einem anderen Blatt verwenden Sie **Tabelle2!A1:C10**. Fügen Sie jeden Bereich hinzu, den Sie zusammenfassen möchten. Wählen Sie einen Bereich in der Liste und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Haben die Bereiche in ihrer ersten Zeile Namen, aktivieren Sie **Obere Zeile enthält die Spaltennamen**. Haben sie Namen in ihrer ersten Spalte, aktivieren Sie **Linke Spalte enthält die Zeilennamen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die seine Quellzellen liest, und sich deshalb ändert, wenn diese sich ändern. Ohne das Kästchen sind die Ergebnisse einfache Zahlen.
7. Klicken Sie auf **OK**.

Ohne aktivierte Namen werden die Zellen an derselben Stelle in jedem Bereich zusammengefasst (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Namen werden Zellen mit gleichen Namen zusammengefasst, egal wo sie in jedem Bereich stehen, und ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. Konsolidieren ist ein einziger Schritt, den Sie rückgängig machen können.
