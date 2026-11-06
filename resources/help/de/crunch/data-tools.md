# Datentools
<!-- keywords: Szenario-Manager, Szenarien, Was-wäre-wenn, Was wäre wenn, Szenarioübersicht, Daten, Datenregister, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Summen, Spezialfilter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, rote Ellipse, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenführen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Die Registerkarte **Daten** enthält die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte **Datei**. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art von Text sie enthält, und zeigt sie danach als neue Arbeitsmappe an.

## Verknüpfungen bearbeiten

Wenn eine Arbeitsmappe Formeln enthält, die eine andere Arbeitsmappe auslesen (etwa ='[Preise.xlsx]Tabelle1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Externe Daten abrufen** diese anderen Arbeitsmappen auf. Sie heißen Quellen. Jede Zeile zeigt, wo sich die Quelle befindet, und ihren Status. Bis die Arbeitsmappe zusammen mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei aus diesen Quellen gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen; ihr Status lautet dann, dass sie nicht geprüft wurde.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, sofern sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die die gewählte Quelleausliest, in den Wert um, den sie jetzt anzeigt. Ein Name, der dafür stand, wird zu **#BEZUG!**. Dies lässt sich nicht rückgängig machen, daher fragt JeffCrunch vorher nach. Sind keine Verknüpfungen mehr vorhanden, wird die Datei ohne die Teile gespeichert, die sie enthielten.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte **Start**. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie dies, wenn eine Spalte mehrere Textteile enthält, die in getrennte Spalten sollen, etwa „Schmidt, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **An Trennzeichen aufteilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wobei Sie ein eigenes Zeichen eintippen. **Aufeinanderfolgende Trennzeichen als ein Zeichen behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. Mit **Text in Anführungszeichen** legen Sie das Zeichen fest, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen stehen zwei Zeichen hintereinander für eines.
   - **An festen Positionen aufteilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei die Zeichen ab 1 gezählt werden. Bei 5, 12 beginnt eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Legen Sie fest, was jeder Teil ist: **Standard** erkennt Zahlen und Datumsangaben so, als hätten Sie sie eingegeben. **Text** behält jeden Teil unverändert bei, sodass „007“ als „007“ erhalten bleibt.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für die Teile ein. Sie ist zunächst die erste markierte Zelle.
6. Das Feld unter den Auswahlmöglichkeiten zeigt die ersten Zeilen so, wie Sie sie aufgeteilt haben. Klicken Sie auf **OK**.

Die Teile werden in die Zellen rechts neben der ersten Zelle geschrieben und ersetzen dort vorhandenen Inhalt. **Text in Spalten** ist ein Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie dies, um von jeder Gruppe gleicher Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Wiederholungen gelten. Zwei Zeilen gelten nur dann als Wiederholung, wenn jede aktivierte Spalte gleich ist. Groß- und Kleinschreibung werden gleich behandelt, und eine Formel zählt als der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Das Dialogfeld meldet, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, zusammen mit ihren Formaten und Formeln. Die Zellen unterhalb der Markierung werden nicht verschoben. Die leeren Zeilen am Ende der Markierung werden gelöscht. **Duplikate entfernen** ist ein Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie dies, wenn Sie das gewünschte Ergebnis kennen, aber nicht die Zahl, die es ergibt. Beispiel: Eine Zelle für eine Zahlung enthält eine Formel mit einem Zinssatz, und Sie möchten, dass die Zahlung 500 beträgt. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Zahlung 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie ist zunächst die markierte Zelle.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Zu ändernde Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der zu ändernden Zelle, und die Statusleiste nennt sie. Findet sie keine, bleibt alles unverändert, und Sie erhalten eine Meldung. Versuchen Sie einen anderen Startwert in der zu ändernden Zelle, und prüfen Sie, ob die Formel tatsächlich diese Zelle verwendet. Die **Zielwertsuche** ist ein Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist eine benannte Gruppe von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt Was-wäre-wenn-Fälle durchzuspielen. Beispiel: Behalten Sie für dieselben Eingabezellen ein Szenario „Geringe Verkäufe“ und ein Szenario „Hohe Verkäufe“ und zeigen Sie jeweils das an, das Sie betrachten möchten.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Szenario-Manager**.
2. Klicken Sie auf **Hinzufügen**. Geben Sie einen Namen ein, etwa Hohe Verkäufe. Tragen Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario festlegt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die markierten Zellen werden für Sie eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt enthält. Geben Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und klicken Sie auf **OK**.
3. Wählen Sie ein Szenario in der Liste und klicken Sie auf **Anzeigen**, um seine Werte in die Zellen zu übernehmen. Ein Doppelklick auf den Namen hat dieselbe Wirkung. Mit **Bearbeiten** ändern Sie Name, Kommentar oder Werte. Mit **Löschen** entfernen Sie das Szenario.

**Anzeigen**, **Hinzufügen**, **Bearbeiten** und **Löschen** sind jeweils ein Schritt, den Sie rückgängig machen können.

Mit **Übersicht** werden alle Szenarien nebeneinander auf einem neuen Blatt namens Szenarioübersicht dargestellt. Es fragt nach den Ergebniszellen, also den Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Übersicht die Werte der veränderbaren Zellen und das Ergebnis der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte „Aktuelle Werte“ zeigt, wie die Zellen beim Erstellen der Übersicht waren. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine von einem anderen Programm erstellte Arbeitsmappe behält ihre Szenarien, und Sie sehen sie hier. Eine als ältere .xls-Datei oder als .ods gespeicherte Kopie enthält sie nicht; beim Speichern wird darauf hingewiesen.

## Teilergebnis

Verwenden Sie dies, um unter jede Gruppe von Zeilen mit demselben Eintrag in einer Spalte eine Zeile mit Summen zu setzen und am Ende eine Gesamtsumme. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der Sie gruppieren, damit gleiche Einträge nebeneinander stehen.

1. Wählen Sie eine Zelle in der Liste aus.
2. Wählen Sie auf der Registerkarte **Daten** in der Gruppe **Gliederung** den Befehl **Teilergebnis**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden**, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen. Klicken Sie auf **OK**.

Jede Summenzeile trägt die Bezeichnung „Ost Gesamt“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, sodass andere Summenzeilen ausgeschlossen werden. Die Zeilen jeder Gruppe werden gegliedert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Summen anzeigen können. Um die Teilergebnisse zu entfernen, öffnen Sie **Teilergebnis** erneut und klicken Sie auf **Alle entfernen**. Teilergebnisse hinzufügen oder entfernen ist ein Schritt, den Sie rückgängig machen können.

## Spezialfilter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. Mit **Erweitert** (in der Gruppe **Sortieren und Filtern**) schreiben Sie die Bedingungen auf das Blatt, sodass sie beliebig lang sein können.

1. Kopieren Sie neben der Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile leerer Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle zutreffen (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, dass **Listenbereich** die Liste mit ihrer Kopfzeile ist, und tragen Sie unter **Bedingungsbereich** die Zellen Ihrer Überschriften und Bedingungen ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Die Zeilen an einen anderen Ort kopieren** und geben Sie die erste Zelle des Zielbereichs ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um von jeder Zeile nur eine zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges einzelnes Zeichen und ***** für eine beliebige Zeichenfolge.
- Eine Zahl wählt Zellen aus, die ihr entsprechen.
- **=Anna** wählt Text, der genau Anna lautet. **=** allein wählt leere Zellen aus.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen aus, die nicht leer sind.

Eine leere Bedingungszeile wählt alle Zeilen aus. Um ausgeblendete Zeilen wieder anzuzeigen, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. **Erweitert** ist ein Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie dies, um festzulegen, was in bestimmte Zellen eingegeben werden darf, damit eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie in der Gruppe **Datentools** der Registerkarte **Daten** den Befehl **Datenüberprüfung**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge wählen Sie unter **Daten** (**zwischen**, **nicht zwischen**, **gleich**, und so weiter) und geben die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei **Liste** geben Sie die Auswahlmöglichkeiten durch Kommas getrennt ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich ein, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown-Liste in Zelle**, um den Pfeil auszublenden.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. **Leere Zellen ignorieren** lässt eine leere Zelle durch.

Auf der Registerkarte **Eingabemeldung** aktivieren Sie das Kästchen und schreiben einen Titel und eine Meldung. Sie erscheinen neben der Zelle, sobald sie ausgewählt ist.

Auf der Registerkarte **Fehlermeldung** legen Sie fest, was mit einer unzulässigen Eingabe geschieht. **Anhalten** verweigert sie. **Warnung** fragt, ob sie behalten werden soll. **Information** teilt es der Person mit und behält die Eingabe. Schreiben Sie einen Titel und eine Meldung für die Fehlermeldung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) zeichnet eine rote Ellipse um jede Zelle des Blatts, deren Wert ihre Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel vorhanden war, oder das Ergebnis einer Formel. Eine Meldung teilt Ihnen mit, wenn alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihre Ellipse. **Überprüfungskreise entfernen** nimmt alle Ellipsen weg.

**Alle löschen** entfernt die Regeln von den markierten Zellen. Die neue Regel ersetzt jede Regel, die die markierten Zellen hatten; andere Zellen behalten ihre Regeln. Das Ändern von Regeln ist ein Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie dies, um ähnlich aufgebaute Bereiche, etwa dieselbe Tabelle aus mehreren Blättern, in einer Tabelle zusammenzuführen.

1. Wählen Sie in der Gruppe **Datentools** der Registerkarte **Daten** den Befehl **Konsolidieren**.
2. Wählen Sie unter **Funktion** die Funktion, mit der kombiniert werden soll, etwa **Summe** oder **Mittelwert**.
3. Geben Sie einen Bereich in das Feld ein und klicken Sie auf **Hinzufügen**. Für einen Bereich auf einem anderen Blatt verwenden Sie etwa **Tabelle2!A1:C10**. Fügen Sie jeden Bereich hinzu, den Sie kombinieren möchten. Markieren Sie einen Bereich in der Liste und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Wenn die Bereiche in ihrer ersten Zeile Namen haben, aktivieren Sie **Obere Zeile enthält Spaltenbeschriftungen**. Wenn sie in ihrer ersten Spalte Namen haben, aktivieren Sie **Linke Spalte enthält Zeilenbeschriftungen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen mit den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die seine Quellzellen ausliest und sich mit ihnen ändert. Ohne diese Option sind die Ergebnisse einfache Zahlen.
7. Klicken Sie auf **OK**.

Ist keine Namensoption aktiviert, werden die Zellen an derselben Stelle jedes Bereichs kombiniert (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Sind Namen aktiviert, werden Zellen mit gleichen Namen kombiniert, unabhängig davon, wo sie in jedem Bereich stehen. Ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. **Konsolidieren** ist ein Schritt, den Sie rückgängig machen können.
