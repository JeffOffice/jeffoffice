# Datentools
<!-- keywords: Szenario-Manager, Szenarien, Was-wäre-wenn, was wäre wenn, Szenario-Zusammenfassung, Daten, Datenregisterkarte, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Summen, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, rote Ellipse, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenführen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Werte aktualisieren, Quelle ändern, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Die Registerkarte **Daten** enthält die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte „Datei“. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art von Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Verknüpfungen bearbeiten

Wenn eine Arbeitsmappe Formeln enthält, die eine andere Arbeitsmappe lesen (etwa ='[Preise.xlsx]Tabelle1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Externe Daten abrufen** diese anderen Arbeitsmappen auf, die Quellen heißen. Jede Zeile zeigt, wo sich die Quelle befindet, und ihren Status. Bis die Arbeitsmappe mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei aus diesen Quellen gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen, deshalb heißt es bei ihr, sie sei nicht geprüft.
- **Werte aktualisieren** liest die gewählte Quelle von diesem Computer und gibt jeder Zelle, die nur ein Verweis auf eine ihrer Zellen ist, den Wert, den diese Zelle jetzt hat. Eine Zelle, die mit dem Wert mehr macht, etwa `=[1]Preise!A1*2`, behält die Zahl, die ihre Datei gespeichert hat. Die Meldung unter der Liste nennt, wie viele Zellen aktualisiert wurden und wie viele nicht. Werte aktualisieren ist ein einziger Schritt, deshalb stellt **Rückgängig** die alten Werte wieder her. Die neuen Werte werden beim Speichern außerdem in den eigenen Teil der Verknüpfung in der Datei geschrieben, sodass die Arbeitsmappe sie beim nächsten Öffnen zeigt, auch wenn die Quelle nicht vorhanden ist.
- **Quelle ändern** lässt Sie für die gewählte Verknüpfung eine andere Datei auswählen, etwa wenn die Arbeitsmappe verschoben wurde. Die Formeln bleiben unverändert. Der neue Speicherort wird beim Speichern in die Datei geschrieben, und **Rückgängig** nimmt die Änderung zurück.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, sofern sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die auf die gewählte Quelle verweist, in den Wert um, den sie jetzt anzeigt, und einen Namen, der darin für eine Stelle stand, in **#BEZUG!**. JeffCrunch fragt vorher nach. Verknüpfung aufheben ist ein einziger Schritt, deshalb bringt **Rückgängig** die Formeln, die Namen und die Verknüpfungen zurück, bis Sie die Arbeitsmappe speichern. Ist keine Verknüpfung mehr vorhanden, wird die Datei ohne die Teile gespeichert, die sie enthielten.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte „Start“. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie dies, wenn eine Spalte mehrere Textteile enthält, die Sie in getrennten Spalten haben möchten, etwa „Schmidt, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **Bei Trennzeichen aufteilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Anderes**, wo Sie ein eigenes Zeichen eingeben. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. **Text in Anführungszeichen** legt das Zeichen fest, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen stehen zwei gleiche Zeichen hintereinander für eines.
   - **Bei festen Positionen aufteilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei die Zeichen ab 1 gezählt werden. Zum Beispiel beginnt bei 5, 12 eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Legen Sie fest, was jeder Teil ist: **Allgemein** erkennt Zahlen und Datumswerte so, als hätten Sie sie eingegeben, und **Text** behält jeden Teil unverändert bei, sodass „007“ als „007“ bleibt.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für die Teile ein. Sie ist zunächst die erste Zelle, die Sie markiert haben.
6. Das Feld unter den Optionen zeigt die ersten Zeilen so geteilt, wie Sie es gewählt haben. Klicken Sie auf **OK**.

Die Teile werden in die Zellen rechts neben der ersten Zelle geschrieben und ersetzen, was dort stand. Text in Spalten ist ein einziger Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie dies, um von jeder Gruppe wiederholter Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Wiederholungen gelten. Zwei Zeilen sind nur dann Wiederholungen, wenn jede aktivierte Spalte gleich ist. Groß- und Kleinschreibung gelten als gleich, und eine Formel zählt als der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Das Dialogfeld meldet, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, mit ihren Formaten und Formeln. Die Zellen unter der Markierung bleiben unverändert. Die leeren Zeilen, die am Ende der Markierung übrig bleiben, werden geleert. Duplikate entfernen ist ein einziger Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie dies, wenn Sie das gewünschte Ergebnis kennen, aber nicht die Zahl, die es ergibt. Zum Beispiel enthält eine Zelle für eine Zahlung eine Formel, die einen Zinssatz verwendet, und Sie möchten, dass die Zahlung 500 beträgt. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Zahlung 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie ist zunächst die Zelle, die Sie markiert haben.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Veränderbare Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der veränderbaren Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Kann sie keine finden, ändert sich nichts, und Sie erhalten eine Meldung. Probieren Sie in der veränderbaren Zelle eine andere Startzahl aus, und prüfen Sie, ob die Formel diese Zelle wirklich verwendet. Die Zielwertsuche ist ein einziger Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist eine benannte Gruppe von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt „Was wäre wenn“ zu prüfen. Sie können zum Beispiel ein Szenario „Geringer Umsatz“ und ein Szenario „Hoher Umsatz“ für dieselben Eingabezellen behalten und das anzeigen, das Sie sich ansehen möchten.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Szenario-Manager**.
2. Klicken Sie auf **Hinzufügen**. Geben Sie einen Namen ein, etwa Hoher Umsatz. Geben Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario festlegt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die markierten Zellen werden für Sie eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt hat. Geben Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und klicken Sie auf **OK**.
3. Wählen Sie in der Liste ein Szenario und klicken Sie auf **Anzeigen**, um seine Werte in die Zellen zu übernehmen. Ein Doppelklick auf einen Namen bewirkt dasselbe. Klicken Sie auf **Bearbeiten**, um Namen, Kommentar oder Werte zu ändern. Klicken Sie auf **Löschen**, um es zu entfernen.

Anzeigen, Hinzufügen, Bearbeiten und Löschen sind jeweils ein Schritt, den Sie rückgängig machen können.

Mit **Zusammenfassung** werden alle Szenarien nebeneinander auf einem neuen Blatt namens Szenario-Zusammenfassung dargestellt. Dafür werden die Ergebniszellen abgefragt, also die Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Zusammenfassung die Werte der veränderbaren Zellen und die Ergebnisse der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte mit dem Namen Aktuelle Werte zeigt, wie die Zellen beim Erstellen der Zusammenfassung waren. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine Arbeitsmappe, die mit einem anderen Programm erstellt wurde, behält ihre Szenarien, und Sie sehen sie hier. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, enthält sie nicht, und beim Speichern erhalten Sie einen Hinweis darauf.

## Teilergebnis

Verwenden Sie dies, um unter jede Gruppe von Zeilen mit demselben Eintrag in einer Spalte eine Zeile mit Summen zu setzen und unten eine Gesamtsumme. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der Sie gruppieren, damit gleiche Einträge nebeneinander stehen.

1. Markieren Sie eine Zelle in der Liste.
2. Wählen Sie in der Gruppe **Gliederung** auf der Registerkarte **Daten** den Befehl **Teilergebnis**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden** aus, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen. Klicken Sie auf **OK**.

Jede Summenzeile lautet „Ost Total“ (oder der Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, sodass andere Summenzeilen nicht mitgezählt werden. Die Zeilen jeder Gruppe werden gegliedert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Summen anzeigen können. Um die Teilergebnisse zu entfernen, öffnen Sie **Teilergebnis** erneut und klicken Sie auf **Alle entfernen**. Teilergebnisse hinzufügen oder entfernen ist ein einziger Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. Mit **Erweitert** (in der Gruppe **Sortieren und Filtern**) schreiben Sie die Bedingungen auf das Blatt, sodass sie so umfangreich sein können, wie Sie es brauchen.

1. Kopieren Sie neben die Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile leerer Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle wahr sein (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, dass **Listenbereich** die Liste mit ihrer Kopfzeile ist, und geben Sie die Zellen Ihrer Überschriften und Bedingungen unter **Bedingungsbereich** ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Ergebnisse an eine andere Stelle kopieren** und geben Sie die erste Zelle des Ziels ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je eine Zeile zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges Zeichen und ***** für eine beliebige Folge von Zeichen.
- Eine Zahl wählt Zellen aus, die ihr entsprechen.
- **=Ann** wählt Text, der genau Ann lautet. **=** allein wählt leere Zellen aus.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen aus, die nicht leer sind.

Eine Zeile mit Bedingungen, in der nichts steht, wählt jede Zeile aus. Um die ausgeblendeten Zeilen wieder anzuzeigen, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. Erweiterter Filter ist ein einziger Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie dies, um festzulegen, was in bestimmte Zellen eingegeben werden darf, damit eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie in der Gruppe **Datentools** auf der Registerkarte **Daten** den Befehl **Datenüberprüfung**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Wählen Sie bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge unter **Daten** (**zwischen**, **nicht zwischen**, **gleich** und so weiter) und geben Sie die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei einer **Liste** geben Sie die Auswahlmöglichkeiten mit Kommas getrennt ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich ein, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown-Liste in Zelle**, um den Pfeil auszublenden.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. **Leere Zellen ignorieren** lässt eine leere Zelle durch.

Aktivieren Sie auf der Registerkarte **Eingabemeldung** das Kontrollkästchen und schreiben Sie einen Titel und eine Meldung. Sie werden neben der Zelle angezeigt, sobald sie ausgewählt ist.

Auf der Registerkarte **Fehlermeldung** wählen Sie, was mit einer unzulässigen Eingabe geschehen soll. **Beenden** lehnt sie ab. **Warnung** fragt, ob sie beibehalten werden soll. **Information** teilt es der Person mit und behält die Eingabe. Schreiben Sie einen Titel und eine Meldung für die Fehlermeldung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) setzt eine rote Ellipse um jede Zelle des Blatts, deren Wert ihre Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel dort stand, oder das Ergebnis einer Formel. Eine Meldung sagt Ihnen, wenn alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihre Ellipse. **Überprüfungskreise entfernen** nimmt alle Ellipsen weg.

**Alle löschen** entfernt die Regeln von den markierten Zellen. Die neue Regel ersetzt alle Regeln, die die markierten Zellen hatten; andere Zellen behalten ihre. Das Ändern von Regeln ist ein einziger Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie dies, um Bereiche, die einander ähnlich sind, etwa dieselbe Tabelle auf mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie in der Gruppe **Datentools** auf der Registerkarte **Daten** den Befehl **Konsolidieren**.
2. Wählen Sie unter **Funktion** die Funktion, mit der zusammengefasst werden soll, etwa **Summe** oder **Mittelwert**.
3. Geben Sie einen Bereich in das Feld ein und klicken Sie auf **Hinzufügen**. Verwenden Sie **Tabelle2!A1:C10** für einen Bereich auf einem anderen Blatt. Fügen Sie jeden Bereich hinzu, den Sie zusammenfassen möchten. Markieren Sie einen Bereich in der Liste und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Wenn die Bereiche in ihrer ersten Zeile Namen haben, aktivieren Sie **Oberste Zeile enthält Spaltenüberschriften**. Wenn sie in ihrer ersten Spalte Namen haben, aktivieren Sie **Linke Spalte enthält Zeilennamen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die seine Quellzellen liest und sich mit ihnen ändert. Ohne dieses Kontrollkästchen sind die Ergebnisse einfache Zahlen.
7. Klicken Sie auf **OK**.

Ohne aktivierte Namen wird die Zelle an derselben Stelle in jedem Bereich zusammengefasst (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Namen werden Zellen mit denselben Namen zusammengefasst, unabhängig davon, wo sie in jedem Bereich stehen, und ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. Konsolidieren ist ein einziger Schritt, den Sie rückgängig machen können.
