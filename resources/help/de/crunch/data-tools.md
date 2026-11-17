# Datentools

<!-- keywords: Formular, Datenmaske, Dateneingabe, Datensatz, Datensätze, neuer Datensatz, weitersuchen, zurücksuchen, Kriterien, Szenario-Manager, Szenarien, Was-wäre-wenn, Szenarioübersicht, Daten, Datenregister, Text in Spalten, Teilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Summen, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, roter Kreis, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenfassen, zulassen, Liste, Dropdown-Liste, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Werte aktualisieren, Quelle ändern, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Die Registerkarte **Daten** enthält die Werkzeuge, die mit Werten in einer Liste arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte **Datei**. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Verknüpfungen bearbeiten

Wenn eine Arbeitsmappe Formeln enthält, die eine andere Arbeitsmappe lesen (etwa ='[Preise.xlsx]Tabelle1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Daten abrufen** diese anderen Arbeitsmappen auf, die Quellen heißen. Jede Zeile zeigt, wo sich die Quelle befindet, und ihren Status. Bis die Arbeitsmappe mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei aus ihnen gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen; ihr Status lautet dann, dass sie nicht geprüft wurde.
- **Werte aktualisieren** liest die gewählte Quelle von diesem Computer und gibt jeder Zelle, die nur auf eine ihrer Zellen verweist, den Wert, den diese Zelle jetzt hat. Eine Zelle, die mit dem Wert mehr macht, etwa `=[1]Preise!A1*2`, behält die Zahl, die ihre Datei gespeichert hat. Die Meldung unter der Liste sagt, wie viele Zellen aktualisiert und wie viele nicht aktualisiert wurden. Werte aktualisieren ist ein einzelner Schritt, daher stellt **Rückgängig** die alten Werte wieder her. Die neuen Werte werden beim Speichern außerdem in den eigenen Teil der Verknüpfung in der Datei geschrieben, sodass die Arbeitsmappe sie beim nächsten Öffnen zeigt, auch wenn die Quelle nicht vorhanden ist.
- **Quelle ändern** lässt Sie für die gewählte Verknüpfung eine andere Datei auswählen, etwa wenn die Arbeitsmappe verschoben wurde. Die Formeln bleiben unverändert. Der neue Ort wird beim Speichern in die Datei geschrieben, und **Rückgängig** nimmt die Änderung zurück.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, sofern sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die auf die gewählte Quelle verweist, in den Wert um, den sie jetzt anzeigt. Ein Name, der für einen Ort darin stand, wird zu **#BEZUG!**. JeffCrunch fragt vorher nach. Verknüpfung aufheben ist ein einzelner Schritt, daher bringt **Rückgängig** die Formeln, die Namen und die Verknüpfungen zurück, bis Sie die Arbeitsmappe speichern. Wenn keine Verknüpfung mehr übrig ist, wird die Datei ohne die Teile gespeichert, die sie enthielten.

## Formular

**Formular** (in der Gruppe **Datentools** und im Suchfeld) zeigt jeweils eine Zeile einer Liste als kleines Formular an. Eine Liste ist eine Tabelle oder ein Zellbereich mit einer Zeile mit Namen oben und einer Datenzeile unter jedem Namen. Klicken Sie zuerst auf eine beliebige Zelle der Liste.

- Jedes Feld ist eine Spalte und trägt die Bezeichnung aus der obersten Zeile. Tippen Sie in ein Feld, um den Wert zu ändern. Ein abgeblendetes Feld enthält eine Formel: Das Formular zeigt das Ergebnis, und Sie ändern die Formel im Blatt.
- **Weitersuchen** und **Zurücksuchen** gehen zur nächsten bzw. vorherigen Zeile. Wenn Sie zu einer anderen Zeile wechseln oder die **Eingabetaste** drücken, wird das Eingegebene in dieser Zeile übernommen.
- **Neu** zeigt ein leeres Formular. Füllen Sie es aus, und drücken Sie dann erneut **Neu**, **Weitersuchen** oder **Schließen**. Die Zeile wird unter der letzten Zeile der Liste angefügt. Eine Tabelle wächst dabei um eine Zeile. Ist eine Zelle unter der Liste nicht leer, meldet das Formular dies und fügt nichts hinzu.
- **Löschen** entfernt die angezeigte Zeile, nachdem Sie bestätigt haben. Die Zeilen darunter rücken nach oben. Eine Tabelle behält immer mindestens eine Datenzeile. Eine Zeile, die ausgeblendet ist oder von einem Filter ausgeblendet wird, wird im Formular nicht angezeigt, nicht mitgezählt und durch diesen Befehl nie gelöscht. Ist die Liste zu lang, um vollständig gefunden zu werden, meldet das Formular, dass nur die ersten Zeilen angezeigt werden.
- **Wiederherstellen** nimmt zurück, was Sie in dieser Zeile eingegeben haben, bevor Sie sie verlassen.
- **Kriterien** hilft beim Suchen von Zeilen. Geben Sie in einem oder mehreren Feldern eine Bedingung ein, und drücken Sie dann **Weitersuchen** oder **Zurücksuchen**. Eine Bedingung ist ein Wert, etwa **Ann** (Text beginnt mit dem Wort; **?** und **\*** stehen für Buchstaben), oder ein Wert nach einem Vergleichszeichen: **>100**, **<=5**, **<>0**, **=Oslo**. Eine Zeile muss alle eingegebenen Bedingungen erfüllen. Drücken Sie **Formular**, um zurückzukehren, oder **Löschen**, um neu zu beginnen. Solange Kriterien gesetzt sind, überspringen **Weitersuchen** und **Zurücksuchen** die Zeilen, die sie nicht erfüllen.
- **Schließen** schreibt die Zeile und schließt das Formular.

Jede Zeile, die Sie ändern, hinzufügen oder löschen, ist ein Schritt, daher nimmt **Rückgängig** im Blatt sie mit einem Mal zurück.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte **Start**. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie dies, wenn eine Spalte mehrere Textteile enthält, die in getrennten Spalten stehen sollen, etwa „Schmidt, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **An einem Trennzeichen teilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wo Sie Ihr eigenes Zeichen eingeben. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. **Text in Anführungszeichen** nennt das Zeichen, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen stehen zwei Zeichen hintereinander für eines.
   - **An festen Positionen teilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei die Zeichen ab 1 gezählt werden. Zum Beispiel beginnt bei 5, 12 eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Wählen Sie, welche Art Inhalt jeder Teil hat: **Allgemein** liest Zahlen und Datumsangaben so, als hätten Sie sie eingegeben, und **Text** behält jeden Teil unverändert bei, sodass „007“ als „007“ erhalten bleibt.
5. Geben Sie unter **Teile einfügen ab** die erste Zelle für die Teile ein. Sie entspricht zunächst der ersten markierten Zelle.
6. Das Feld unter den Auswahlmöglichkeiten zeigt die ersten Zeilen so, wie Sie sie aufteilen. Klicken Sie auf **OK**.

Die Teile werden in die Zellen rechts neben der ersten Zelle geschrieben und ersetzen dabei den vorhandenen Inhalt. Text in Spalten ist ein Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie dies, um von jeder Gruppe wiederholter Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Duplikate gelten. Zwei Zeilen sind nur dann Duplikate, wenn in jeder aktivierten Spalte derselbe Wert steht. Groß- und Kleinschreibung gelten als gleich, und eine Formel zählt als der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Das Dialogfeld meldet, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, mit ihren Formaten und Formeln. Die Zellen unter der Markierung werden nicht verschoben. Die leeren Zeilen, die am Ende der Markierung übrig bleiben, werden gelöscht. Duplikate entfernen ist ein Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie dies, wenn Sie das gewünschte Ergebnis kennen, aber nicht die Zahl, die es erzeugt. Zum Beispiel enthält eine Zelle für eine Zahlung eine Formel mit einem Zinssatz, und Sie möchten, dass die Zahlung 500 beträgt. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Zahlung 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie entspricht zunächst der markierten Zelle.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Zu ändernde Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Wenn die Zielwertsuche eine Zahl findet, bleibt sie in der zu ändernden Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Wenn sie keine findet, ändert sich nichts, und Sie erhalten eine Meldung. Probieren Sie in der zu ändernden Zelle einen anderen Startwert aus, und prüfen Sie, ob die Formel tatsächlich diese Zelle verwendet. Die Zielwertsuche ist ein Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist eine benannte Gruppe von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt Was-wäre-wenn-Fälle durchzuspielen. Behalten Sie zum Beispiel für dieselben Eingabezellen ein Szenario „Geringe Umsätze“ und ein Szenario „Hohe Umsätze“, und zeigen Sie das gewünschte an.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Szenario-Manager**.
2. Klicken Sie auf **Hinzufügen**. Geben Sie einen Namen ein, etwa Hohe Umsätze. Geben Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario festlegt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die markierten Zellen werden für Sie eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt enthält. Tragen Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und klicken Sie auf **OK**.
3. Wählen Sie ein Szenario in der Liste aus, und klicken Sie auf **Anzeigen**, um seine Werte in die Zellen zu setzen. Ein Doppelklick auf den Namen hat dieselbe Wirkung. Mit **Bearbeiten** ändern Sie Namen, Kommentar oder Werte. Mit **Löschen** entfernen Sie das Szenario.

Anzeigen, Hinzufügen, Bearbeiten und Löschen sind jeweils ein Schritt, den Sie rückgängig machen können.

**Übersicht** legt alle Szenarien nebeneinander auf einem neuen Blatt mit dem Namen Szenarioübersicht an. Der Befehl fragt nach den Ergebniszellen, also den Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Übersicht die Werte der veränderbaren Zellen und die Ergebnisse der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte mit dem Namen Aktuelle Werte zeigt, wie die Zellen beim Erstellen der Übersicht waren. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine Arbeitsmappe, die mit einem anderen Programm erstellt wurde, behält ihre Szenarien, und Sie sehen sie hier. Eine als älteres .xls-Format oder als .ods gespeicherte Kopie enthält sie nicht, und das Speichern weist Sie darauf hin.

## Teilergebnis

Verwenden Sie dies, um unter jede Gruppe von Zeilen, die in einer Spalte denselben Eintrag haben, eine Zeile mit Summen zu setzen, und unten eine Gesamtsumme. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der Sie gruppieren, damit gleiche Einträge nebeneinanderstehen.

1. Wählen Sie eine Zelle in der Liste aus.
2. Wählen Sie **Teilergebnis** in der Gruppe **Gliederung** auf der Registerkarte **Daten**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden**, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen. Klicken Sie auf **OK**.

Jede Zeile mit Teilergebnis trägt die Bezeichnung „Ost Ergebnis“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, die die anderen Ergebniszeilen auslässt. Die Zeilen jeder Gruppe werden gegliedert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Ergebnisse anzeigen können. Um die Teilergebnisse zu entfernen, öffnen Sie **Teilergebnis** erneut und klicken auf **Alle entfernen**. Das Hinzufügen oder Entfernen von Teilergebnissen ist ein Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. **Erweitert** (in der Gruppe **Sortieren und Filtern**) lässt Sie die Bedingungen im Blatt schreiben, sodass sie beliebig lang sein können.

1. Kopieren Sie neben die Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile mit leeren Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle zutreffen (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, ob **Listenbereich** die Liste mit ihrer Kopfzeile ist, und geben Sie die Zellen Ihrer Überschriften und Bedingungen unter **Kriterienbereich** ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Die Zeilen an einen anderen Ort kopieren** und geben Sie die erste Zelle des Ziels ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je eine Zeile zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text aus, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges einzelnes Zeichen und **\*** für eine beliebige Folge von Zeichen.
- Eine Zahl wählt Zellen aus, die ihr entsprechen.
- **=Ann** wählt Text aus, der genau Ann lautet. **=** allein wählt leere Zellen aus.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen aus, die nicht leer sind.

Eine Bedingungszeile ohne Eintrag wählt jede Zeile aus. Um die ausgeblendeten Zeilen wieder anzuzeigen, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. Der erweiterte Filter ist ein Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie dies, um festzulegen, was in bestimmte Zellen eingegeben werden darf, sodass eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie **Datenüberprüfung** in der Gruppe **Datentools** auf der Registerkarte **Daten**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganzzahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Wählen Sie bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge unter **Daten** (**zwischen**, **nicht zwischen**, **gleich**, und so weiter) und geben Sie die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei einer **Liste** geben Sie die Auswahlmöglichkeiten durch Kommas getrennt ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich ein, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown-Liste in Zelle**, um den Pfeil auszublenden.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. **Leere Zellen ignorieren** lässt eine leere Zelle durch.

Aktivieren Sie auf der Registerkarte **Eingabemeldung** das Kästchen, und schreiben Sie einen Titel und eine Meldung. Sie werden neben der Zelle angezeigt, sobald sie ausgewählt ist.

Legen Sie auf der Registerkarte **Fehlermeldung** fest, was mit einer unzulässigen Eingabe geschehen soll. **Stopp** lehnt sie ab. **Warnung** fragt, ob sie beibehalten werden soll. **Information** teilt es der Person mit und behält die Eingabe. Schreiben Sie einen Titel und eine Meldung für die Fehlermeldung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) setzt einen roten Kreis um jede Zelle des Blatts, deren Wert ihre Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel dort stand, oder das Ergebnis einer Formel. Eine Meldung sagt Ihnen, ob alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihren Kreis. **Überprüfungskreise entfernen** entfernt alle Kreise.

**Alle löschen** entfernt die Regeln aus den markierten Zellen. Die neue Regel ersetzt jede Regel, die die markierten Zellen bisher hatten; andere Zellen behalten ihre Regeln. Das Ändern von Regeln ist ein Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie dies, um ähnlich aufgebaute Bereiche, etwa dieselbe Tabelle aus mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie den Befehl **Konsolidieren** in der Gruppe **Datentools** auf der Registerkarte **Daten**.
2. Wählen Sie unter **Funktion** die Funktion, mit der kombiniert werden soll, etwa **Summe** oder **Mittelwert**.
3. Geben Sie einen Bereich in das Feld ein und klicken Sie auf **Hinzufügen**. Verwenden Sie **Blatt2!A1:C10** für einen Bereich auf einem anderen Blatt. Fügen Sie jeden Bereich hinzu, den Sie kombinieren möchten. Wählen Sie einen Bereich in der Liste aus und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Wenn die Bereiche in ihrer ersten Zeile Namen haben, aktivieren Sie **Obere Zeile enthält Spaltenüberschriften**. Wenn sie in ihrer ersten Spalte Namen haben, aktivieren Sie **Linke Spalte enthält Zeilenbeschriftungen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die auf seine Quellzellen verweist und sich mit ihnen ändert. Ohne diese Option sind die Ergebnisse einfache Zahlen.
7. Klicken Sie auf **OK**.

Ohne aktivierte Namen werden die Zellen an derselben Stelle in jedem Bereich kombiniert (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Namen werden Zellen mit gleichen Namen kombiniert, unabhängig davon, wo sie in jedem Bereich stehen, und ein Name, den nur manche Bereiche haben, erhält trotzdem eine Zeile oder Spalte. Konsolidieren ist ein Schritt, den Sie rückgängig machen können.
