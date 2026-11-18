# Datentools
<!-- keywords: Prognoseblatt, Prognose, zukünftige Werte, Konfidenzintervall, Saisonalität, Trend, Glättung, Formular, Datenformular, Dateneingabe, Datensatz, Datensätze, neuer Datensatz, weitersuchen, vorheriges suchen, Kriterien, Szenario-Manager, Szenarien, Was-wäre-wenn, Szenarioübersicht, Daten, Registerkarte Daten, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Ergebnisse, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, rote Ellipse, Überprüfungskreise entfernen, Konsolidieren, Bereiche zusammenfassen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Werte aktualisieren, Quelle ändern, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Auf der Registerkarte **Daten** finden Sie die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte **Datei**. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art von Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Verknüpfungen bearbeiten

Wenn eine Arbeitsmappe Formeln enthält, die eine andere Arbeitsmappe lesen (etwa ='[Preise.xlsx]Tabelle1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Daten abrufen** diese anderen Arbeitsmappen auf, die Quellen heißen. Jede Zeile zeigt, wo die Quelle liegt, und ihren Status. Bis die Arbeitsmappe mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei von dort gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen, ihr Status lautet daher, dass sie nicht geprüft wurde.
- **Werte aktualisieren** liest die gewählte Quelle von diesem Computer und gibt jeder Zelle, die nur auf eine ihrer Zellen verweist, den Wert, den diese Zelle jetzt hat. Eine Zelle, die mit dem Wert mehr macht, etwa =[1]Preise!A1*2, behält die Zahl, die ihre Datei gespeichert hat. Die Meldung unter der Liste nennt, wie viele Zellen aktualisiert wurden und wie viele nicht. **Werte aktualisieren** ist ein einzelner Schritt, daher stellt **Rückgängig** die alten Werte wieder her. Die neuen Werte werden beim Speichern außerdem in den eigenen Teil der Verknüpfung in der Datei geschrieben. So zeigt die Arbeitsmappe sie beim nächsten Öffnen an, auch wenn die Quelle nicht vorhanden ist.
- **Quelle ändern** lässt Sie für die gewählte Verknüpfung eine andere Datei auswählen, etwa wenn die Arbeitsmappe verschoben wurde. Die Formeln bleiben unverändert. Der neue Ort wird beim Speichern in die Datei geschrieben, und **Rückgängig** nimmt die Änderung zurück.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, sofern sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die die gewählte Quelle liest, in den Wert um, den sie jetzt anzeigt. Ein Name, der dort auf eine Stelle verwies, wird in **#BEZUG!**. JeffCrunch fragt vorher nach. **Verknüpfung aufheben** ist ein einzelner Schritt, daher bringt **Rückgängig** Formeln, Namen und Verknüpfungen zurück, bis Sie die Arbeitsmappe speichern. Wenn keine Verknüpfung mehr übrig ist, wird die Datei ohne die Teile gespeichert, die sie enthielten.

## Formular

**Formular** (in der Gruppe **Datentools** und im Suchfeld) zeigt jeweils eine Zeile einer Liste als kleines Formular an. Eine Liste ist eine Tabelle oder ein Zellbereich mit einer Zeile Namen oben und einer Datenzeile unter jedem Namen. Klicken Sie zuerst auf eine beliebige Zelle der Liste.

- Jedes Feld ist eine Spalte und trägt den Namen aus der obersten Zeile als Beschriftung. Geben Sie in einem Feld einen Wert ein, um ihn zu ändern. Ein abgeblendetes Feld enthält eine Formel: Das Formular zeigt das Ergebnis an, und die Formel ändern Sie im Blatt.
- **Weitersuchen** und **Vorheriges suchen** wechseln zur nächsten bzw. vorherigen Zeile. Wenn Sie zu einer anderen Zeile wechseln oder die **Eingabetaste** drücken, wird das, was Sie in dieser Zeile eingegeben haben, geschrieben.
- **Neu** zeigt ein leeres Formular. Füllen Sie es aus, und drücken Sie dann erneut **Neu**, **Weitersuchen** oder **Schließen**. Die Zeile wird unter der letzten Zeile der Liste eingefügt. Eine Tabelle wächst dabei um eine Zeile. Ist eine Zelle unter der Liste nicht leer, meldet das Formular dies und fügt nichts hinzu.
- **Löschen** entfernt die angezeigte Zeile, nachdem nachgefragt wurde. Die Zeilen darunter rücken nach oben. Eine Tabelle behält immer mindestens eine Datenzeile. Eine ausgeblendete Zeile oder eine Zeile, die ein Filter ausblendet, wird im Formular nicht angezeigt, nicht gezählt und durch das Formular nie gelöscht. Ist die Liste zu lang, um sie ganz zu erfassen, meldet das Formular, dass nur die ersten Zeilen angezeigt werden.
- **Wiederherstellen** nimmt zurück, was Sie in dieser Zeile eingegeben haben, bevor Sie sie verlassen.
- **Kriterien** hilft Ihnen, Zeilen zu suchen. Geben Sie eine Bedingung in ein oder mehrere Felder ein, und drücken Sie dann **Weitersuchen** oder **Vorheriges suchen**. Eine Bedingung ist ein Wert, etwa **Ann** (Text beginnt mit dem Wort; **?** und **\*** stehen für Buchstaben), oder ein Wert nach einem Vergleichszeichen: **>100**, **<=5**, **<>0**, **=Oslo**. Eine Zeile muss jede eingegebene Bedingung erfüllen. Mit **Formular** kehren Sie zurück, mit **Leeren** beginnen Sie neu. Solange Kriterien gesetzt sind, überspringen **Weitersuchen** und **Vorheriges suchen** die Zeilen, die sie nicht erfüllen.
- **Schließen** schreibt die Zeile und schließt das Formular.

Jede Zeile, die Sie ändern, hinzufügen oder löschen, ist ein Schritt. **Rückgängig** im Blatt nimmt sie daher mit einem Klick zurück.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte **Start**. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie diesen Befehl, wenn eine Spalte mehrere Textteile enthält, die Sie in getrennten Spalten haben möchten, etwa „Müller, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **An Trennzeichen aufteilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wobei Sie in das Feld Ihr eigenes Zeichen eintragen. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. **Text in Anführungszeichen** nennt das Zeichen, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen stehen zwei Zeichen hintereinander für eines.
   - **An festen Positionen aufteilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt. Die Zeichen werden ab 1 gezählt. Zum Beispiel beginnt bei 5, 12 eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Wählen Sie, was jeder Teil ist: **Standard** liest Zahlen und Datumswerte so, als hätten Sie sie eingegeben, und **Text** behält jeden Teil unverändert bei, sodass „007“ als „007“ bleibt.
5. Geben Sie unter **Teile einfügen ab** die erste Zelle für die Teile ein. Sie ist zunächst die erste markierte Zelle.
6. Das Feld unter den Optionen zeigt die ersten Zeilen so, wie Sie sie aufgeteilt haben. Klicken Sie auf **OK**.

Die Teile kommen in die Zellen rechts neben der ersten Zelle und ersetzen dort vorhandene Inhalte. **Text in Spalten** ist ein Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie diesen Befehl, um von jeder Gruppe sich wiederholender Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Wiederholung gelten. Zwei Zeilen wiederholen sich nur, wenn jede aktivierte Spalte gleich ist. Groß- und Kleinschreibung gelten als gleich, und eine Formel zählt als der Wert, den sie ergibt.
5. Klicken Sie auf **OK**. Das Dialogfeld meldet, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, mit ihren Formaten und Formeln. Die Zellen unter der Markierung rücken nicht nach. Die leeren Zeilen, die unten in der Markierung übrig bleiben, werden geleert. **Duplikate entfernen** ist ein Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie diesen Befehl, wenn Sie das gewünschte Ergebnis kennen, aber nicht die Zahl, die es liefert. Beispiel: Eine Zelle für eine Rate enthält eine Formel mit einem Zinssatz, und Sie möchten, dass die Rate 500 beträgt. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Rate 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie ist zunächst die markierte Zelle.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Veränderbare Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der veränderbaren Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Findet sie keine, bleibt alles unverändert, und Sie erhalten eine Meldung. Versuchen Sie einen anderen Startwert in der veränderbaren Zelle, und prüfen Sie, ob die Formel tatsächlich diese Zelle verwendet. Die Zielwertsuche ist ein Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist eine benannte Gruppe von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt Was-wäre-wenn-Berechnungen durchzuspielen. Beispiel: Sie behalten ein Szenario „Niedrige Umsätze“ und ein Szenario „Hohe Umsätze“ für dieselben Eingabezellen und zeigen das gewünschte an.

1. Wählen Sie auf der Registerkarte **Daten** **Szenario-Manager**.
2. Klicken Sie auf **Hinzufügen**. Geben Sie einen Namen ein, etwa Hohe Umsätze. Geben Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario festlegt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die von Ihnen markierten Zellen werden für Sie eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt enthält. Geben Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und klicken Sie auf **OK**.
3. Wählen Sie in der Liste ein Szenario, und klicken Sie auf **Anzeigen**, um seine Werte in die Zellen zu setzen. Ein Doppelklick auf den Namen hat dieselbe Wirkung. Mit **Bearbeiten** ändern Sie Name, Kommentar oder Werte. Mit **Löschen** entfernen Sie das Szenario.

**Anzeigen**, **Hinzufügen**, **Bearbeiten** und **Löschen** sind jeweils ein Schritt, den Sie rückgängig machen können.

Mit **Übersicht** stellen Sie alle Szenarien nebeneinander auf einem neuen Blatt namens Szenarioübersicht dar. Der Befehl fragt nach den Ergebniszellen, also den Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Übersicht die Werte der veränderbaren Zellen und die Ergebnisse der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte Aktuelle Werte zeigt, wie die Zellen beim Erstellen der Übersicht aussahen. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine Arbeitsmappe, die mit einem anderen Programm erstellt wurde, behält ihre Szenarien, und Sie sehen sie hier. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, enthält sie nicht, und das Speichern weist Sie darauf hin.

## Teilergebnis

Verwenden Sie diesen Befehl, um unter jede Gruppe von Zeilen mit demselben Eintrag in einer Spalte eine Zeile mit Summen zu setzen und unten eine Gesamtsumme. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der gruppiert wird, damit gleiche Einträge nebeneinanderstehen.

1. Markieren Sie eine Zelle der Liste.
2. Wählen Sie in der Gruppe **Gliederung** der Registerkarte **Daten** **Teilergebnis**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden**, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen. Klicken Sie auf **OK**.

Jede Summenzeile trägt die Bezeichnung „Ost Ergebnis“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, die daher die anderen Summenzeilen ausschließt. Die Zeilen jeder Gruppe werden gegliedert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Summen anzeigen lassen können. Um die Teilergebnisse zu entfernen, öffnen Sie **Teilergebnis** erneut und klicken auf **Alle entfernen**. Das Hinzufügen oder Entfernen von Teilergebnissen ist ein Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. **Erweitert** (in der Gruppe **Sortieren und Filtern**) erlaubt es Ihnen, die Bedingungen im Blatt zu schreiben, sodass sie beliebig lang sein können.

1. Kopieren Sie neben der Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile leerer Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle zutreffen (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, dass **Listenbereich** die Liste mit ihrer Kopfzeile ist, und geben Sie die Zellen Ihrer Überschriften und Bedingungen unter **Kriterienbereich** ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Die Zeilen an einen anderen Ort kopieren** und geben Sie die erste Zelle des Ziels ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je eine Zeile zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges Zeichen und **\*** für eine beliebige Folge von Zeichen.
- Eine Zahl wählt Zellen, die ihr entsprechen.
- **=Ann** wählt Text, der genau Ann lautet. **=** allein wählt leere Zellen.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen, die nicht leer sind.

Eine Bedingungszeile ohne Inhalt wählt jede Zeile. Um die ausgeblendeten Zeilen wieder einzublenden, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. **Erweiterter Filter** ist ein Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie diesen Befehl, um festzulegen, was in bestimmte Zellen eingegeben werden darf, damit eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie in der Gruppe **Datentools** der Registerkarte **Daten** **Datenüberprüfung**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge wählen Sie unter **Daten** (**zwischen**, **nicht zwischen**, **gleich**, und so weiter) und geben die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei einer **Liste** geben Sie die Auswahlmöglichkeiten mit Kommas getrennt ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich an, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown-Liste in Zelle**, um den Pfeil auszublenden.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. Mit **Leere Zellen ignorieren** dürfen leere Zellen passieren.

Aktivieren Sie auf der Registerkarte **Eingabemeldung** das Kontrollkästchen und schreiben Sie einen Titel und eine Meldung. Sie erscheinen neben der Zelle, sobald sie markiert ist.

Auf der Registerkarte **Fehlermeldung** legen Sie fest, was mit einer unzulässigen Eingabe geschieht. **Beenden** lehnt sie ab. **Warnung** fragt, ob sie behalten werden soll. **Information** teilt es der Person mit und behält die Eingabe. Schreiben Sie einen Titel und eine Meldung für die Warnung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) zeichnet eine rote Ellipse um jede Zelle im Blatt, deren Wert ihre Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel dort stand, oder das Ergebnis einer Formel. Eine Meldung sagt Ihnen, ob alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihre Ellipse. **Überprüfungskreise entfernen** nimmt alle Ellipsen weg.

**Alle löschen** entfernt die Regeln von den markierten Zellen. Die neue Regel ersetzt jede Regel, die die markierten Zellen hatten; andere Zellen behalten ihre Regeln. Das Ändern von Regeln ist ein Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie diesen Befehl, um ähnliche Bereiche, etwa dieselbe Tabelle aus mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie in der Gruppe **Datentools** der Registerkarte **Daten** **Konsolidieren**.
2. Wählen Sie unter **Funktion** die Berechnung, etwa **Summe** oder **Mittelwert**.
3. Geben Sie in das Feld einen Bereich ein und klicken Sie auf **Hinzufügen**. Für einen Bereich auf einem anderen Blatt verwenden Sie **Tabelle2!A1:C10**. Fügen Sie jeden Bereich hinzu, den Sie zusammenfassen möchten. Markieren Sie einen Bereich in der Liste und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Haben die Bereiche Namen in ihrer ersten Zeile, aktivieren Sie **Oberste Zeile enthält Spaltenüberschriften**. Haben sie Namen in ihrer ersten Spalte, aktivieren Sie **Linke Spalte enthält Zeilenbeschriftungen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die auf seine Quellzellen verweist und sich damit mitändert. Ohne diese Option sind die Ergebnisse einfache Zahlen.
7. Klicken Sie auf **OK**.

Ohne aktivierte Namen wird die Zelle an derselben Stelle in jedem Bereich zusammengefasst (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Namen werden Zellen mit gleichen Namen zusammengefasst, unabhängig davon, wo sie in jedem Bereich stehen. Ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. **Konsolidieren** ist ein Schritt, den Sie rückgängig machen können.

## Prognoseblatt

Das Prognoseblatt erstellt ein neues Blatt mit einer Tabelle und einem Diagramm künftiger Werte, die aus einer Zeitachse und einer Reihe von Werten berechnet werden. Es ermittelt den Trend und die sich wiederholende Saison der Werte und fügt ein Band hinzu, in dem die künftigen Werte voraussichtlich bleiben.

1. Markieren Sie die Zeitachse und die Werte, zwei nebeneinanderliegende Spalten (die Zeitachse zuerst). Die Zeitachse besteht aus Datumsangaben oder Zahlen mit gleichen Abständen, etwa jeden Monat oder jede 5 Einheiten. Eine erste Zeile mit Namen wird ausgelassen.
2. Wählen Sie in der Gruppe **Prognose** der Registerkarte **Daten** **Prognoseblatt**.
3. Ein Bild zeigt die Prognose und ändert sich, wenn Sie die Optionen ändern. Wählen Sie **Liniendiagramm** oder **Säulendiagramm**.
4. Legen Sie die Optionen fest:
   - **Zeitachsenbereich** und **Wertebereich** sind die Zellen, die gelesen werden. Geben Sie hier andere Zellen ein, wenn die Markierung falsch war.
   - **Prognosebeginn** bleibt leer, um nach dem letzten Punkt zu beginnen. Ein Datum oder eine Zahl vor dem letzten Punkt beginnt dort und verwendet nur die Punkte davor, sodass Sie die Prognose mit dem tatsächlichen Verlauf vergleichen können.
   - **Prognoseende** ist der letzte Zeitpunkt der Prognose. Es werden höchstens 10.000 Punkte prognostiziert.
   - **Konfidenzintervall** gibt an, wie sicher das Band ist, anfangs 95 %.
   - **Saisonalität** wird durch Betrachten der Werte ermittelt. Sie können aber auch die Länge der Saison in Punkten festlegen oder **Keine** wählen. Die Werte müssen zwei volle Saisons umfassen.
   - **Fehlende Punkte auffüllen mit** legt fest, was ein Zeitpunkt ohne Wert erhält: eine gerade Linie zwischen seinen Nachbarn oder null.
   - **Doppelte Zeitpunkte zusammenfassen mit** legt fest, wie Werte zum selben Zeitpunkt kombiniert werden: Mittelwert, Anzahl, Anzahl (Werte), Max, Median, Min oder Summe.
   - **Prognosestatistik einbeziehen** fügt eine Tabelle mit Alpha, Beta, Gamma, MASE, SMAPE, MAE und RMSE hinzu, die zeigt, wie gut die Prognose zu den Werten passt.
5. Ist etwas falsch, nennt das Dialogfeld in einfachen Worten, was (Schritte, die nicht gleich sind, zu wenige Punkte, Text in den Werten), und **Erstellen** wartet.
6. Klicken Sie auf **Erstellen**.

Das neue Blatt wird vor dem Blatt eingefügt, auf dem Sie gerade waren. Seine Tabelle hat die Spalten Zeitachse, Werte, Prognose, Untere Konfidenzgrenze und Obere Konfidenzgrenze. Die Prognose und die Grenzen sind Formeln, die FORECAST.ETS und FORECAST.ETS.CONFINT auf den eigenen Spalten Zeitachse und Werte der Tabelle verwenden. Sie werden daher neu berechnet, wenn Sie einen Wert ändern. Das Diagramm zeigt die Werte, die Prognose und das Band. Das Prognoseblatt ist ein Schritt, den Sie rückgängig machen können.

Die Zahlen können um einige Prozent von denen anderer Tabellenkalkulationsprogramme abweichen, da diese ihre eigene Methode zur Anpassung der Glättung verwenden. Eine Prognose, die eine Datei bereits enthält, behält die gespeicherten Werte, bis sich die Daten ändern, die sie liest.

Eine Datei mit mehr als 1.000.000 Punkten wird nicht verwendet.
