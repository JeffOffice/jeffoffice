# Datentools
<!-- keywords: Prognoseblatt, Prognose, zukünftige Werte, Konfidenzintervall, Saisonalität, Trend, Glättung, Formular, Datenformular, Dateneingabe, Datensatz, Datensätze, neuer Datensatz, weitersuchen, zurücksuchen, Kriterien, Szenario-Manager, Szenarien, Was-wäre-wenn, Szenarioübersicht, Daten, Registerkarte Daten, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Summen, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, rotes Oval, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenfassen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Werte aktualisieren, Quelle ändern, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Die Registerkarte **Daten** enthält die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte Datei. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und welche Art Text sie enthält, und zeigt sie dann als neue Arbeitsmappe an.

## Aus Datei

**Aus Datei** in der Gruppe **Daten abrufen** lädt die Zeilen einer JSON-Datei, einer XML-Datei oder einer anderen Arbeitsmappe als Tabelle. Es können nur Dateien auf diesem Computer geladen werden. Eine Webadresse oder eine Datei auf einem anderen Computer wird abgewiesen.

1. Klicken Sie auf **Aus Datei**, dann auf **Durchsuchen**, und wählen Sie die Datei aus. Die ersten Zeilen werden sofort angezeigt, zusammen mit der Art, die JeffCrunch für jede Spalte aus diesen ersten Zeilen erkannt hat: Zahl, Datum, Wahr/Falsch oder Text. Die gesamte Datei wird beim Laden geprüft, daher kann eine Spalte am Ende als Text gelten. Auf dem Blatt ändert sich nichts, bis Sie auf **Laden** klicken.
2. Enthält die Datei mehr als eine Liste, wählen Sie die zu ladende Liste aus. In einer JSON-Datei ist es eine Liste von Elementen mit benannten Werten, etwa `$.items`. In einer XML-Datei ist es ein Element, das sich wiederholt, etwa `/feed/entry`. In einer Arbeitsmappe ist es ein Blatt, eine Tabelle oder ein benannter Bereich.
3. Ein JSON-Element innerhalb eines Elements wird zu einer Spalte namens `äußer.innen`, bis zu vier Ebenen tief. Eine Liste einfacher Werte wird entweder in eine Zelle zusammengefügt (die ersten 10.000 Werte) oder auf Spalten verteilt (die ersten 16), je nach Ihrer Wahl. In XML werden die Attribute und die untergeordneten Elemente zu Spalten. Zwei verschiedene Werte teilen sich nie eine Spalte: Würden zwei denselben Namen erhalten, wird dem zweiten eine Nummer angehängt oder, bei XML, sein Präfix vorangestellt, und ein Hinweis weist darauf hin. Ein Name, der innerhalb eines JSON-Elements wiederholt wird, behält seinen letzten Wert, und auch darauf weist ein Hinweis hin. Eine Zahl, die zu groß oder zu klein für eine Zelle ist, wird als Text so übernommen, wie sie geschrieben ist, und ein Zeichen, das nicht gelesen werden kann, wird durch ein Ersatzzeichen dargestellt; beides wird vermerkt.
4. Aktivieren Sie **Alle Spalten als Text laden**, um jeden Wert so zu behalten, wie er geschrieben ist. Eine Zahlenspalte mit führender Null, etwa 007, wird immer als Text übernommen.
5. Geben Sie die Zelle ein, in der die Tabelle beginnen soll, und klicken Sie dann auf **Laden**. Würde die Tabelle Zellen überdecken, die Inhalte haben, fragt JeffCrunch, bevor es sie ersetzt. Ist kein Platz vorhanden oder das Blatt geschützt, erhalten Sie eine Meldung, und es ändert sich nichts.

Die Tabelle ist ein Schritt von **Rückgängig**. Sie merkt sich die Datei und Ihre Auswahl. Um die Datei erneut zu lesen, klicken Sie in der Tabelle auf eine Zelle und dann auf der Registerkarte Daten auf **Aktualisieren**. **Alle aktualisieren** ist die große Schaltfläche daneben: Sie liest jede Tabelle, die mit „Daten abrufen“ erstellt wurde, und aktualisiert danach jede Pivot-Tabelle, alles als ein Schritt von Rückgängig. Die Tabelle wächst oder schrumpft entsprechend den Zeilen der Datei. Spalten, die Sie neben der Tabelle hinzugefügt haben, bleiben erhalten: Eine Formel, die Sie nach unten kopiert haben, wird in neue Zeilen übernommen, und die Zeilen, die in der Datei nicht mehr vorkommen, werden ebenfalls aus der Tabelle entfernt. Ist die Datei verschwunden oder kann sie nicht gelesen werden, erhalten Sie eine Meldung, und es ändert sich nichts. Haben sich die Spalten der Datei geändert, erfahren Sie, welche fehlen oder neu sind, und nichts ändert sich. Erstellen Sie die Tabelle dann mit „Daten abrufen“ neu, oder stellen Sie die Spalten in der Datei wieder her.

Die Verknüpfung zur Datei wird in der Tabelle in einer gespeicherten `.xlsx`-Arbeitsmappe gespeichert, und zwar in einem Teil, den andere Programme ignorieren. Die Verknüpfung enthält den vollständigen Pfad der Datei auf diesem Computer, daher kann jeder, der die Arbeitsmappe erhält, diesen Pfad lesen. Um ihn zu entfernen, bevor Sie die Arbeitsmappe weitergeben, verwenden Sie **Dokument prüfen**, dann **Verknüpfungen zu Datendateien** und dann **Entfernen**. Eine Kopie, die als anderer Dateityp gespeichert wird, behält sie nicht, und **Kompatibilität prüfen** weist darauf hin. **Dokument prüfen** listet die Verknüpfungen unter **Verknüpfungen zu Datendateien** auf und kann sie entfernen. Die Tabellen und ihre Zellen bleiben erhalten.

Dateien werden mit Grenzen gelesen: eine Datei bis 64 MB, 200.000 Zeilen, 256 Spalten und 32.767 Zeichen in einer Zelle. XML mit einem Dokumenttyp oder mit Entitäten wird abgewiesen. Alles, was durch eine Grenze abgeschnitten wird, wird angegeben.

## Verknüpfungen bearbeiten

Enthält eine Arbeitsmappe Formeln, die eine andere Arbeitsmappe lesen (etwa ='[Preise.xlsx]Tabelle1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Daten abrufen** diese anderen Arbeitsmappen auf, die Quellen heißen. Jede Zeile zeigt, wo die Quelle liegt, und ihren Status. Bis die Arbeitsmappe mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei aus den Quellen gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen, daher heißt ihr Status, dass sie nicht geprüft wurde.
- **Werte aktualisieren** liest die gewählte Quelle von diesem Computer und gibt jeder Zelle, die nur auf eine ihrer Zellen verweist, den Wert, den diese Zelle jetzt hat. Eine Zelle, die mit dem Wert mehr macht, etwa `=[1]Preise!A1*2`, behält die Zahl, die ihre Datei gespeichert hat. Die Meldung unter der Liste sagt, wie viele Zellen aktualisiert wurden und wie viele nicht. **Werte aktualisieren** ist ein Schritt, daher bringt **Rückgängig** die alten Werte zurück. Die neuen Werte werden beim Speichern auch in den eigenen Teil der Verknüpfung in der Datei geschrieben, sodass die Arbeitsmappe sie beim nächsten Öffnen anzeigt, auch wenn die Quelle nicht vorhanden ist.
- **Quelle ändern** lässt Sie für die gewählte Verknüpfung eine andere Datei auswählen, etwa wenn die Arbeitsmappe verschoben wurde. Die Formeln bleiben unverändert. Der neue Speicherort wird beim Speichern in die Datei geschrieben, und **Rückgängig** nimmt die Änderung zurück.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, sofern sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die auf die gewählte Quelle verweist, in den Wert um, den sie jetzt anzeigt, und einen Namen, der darin für eine Stelle stand, in **#BEZUG!**. JeffCrunch fragt vorher nach. **Verknüpfung aufheben** ist ein Schritt, daher bringt **Rückgängig** die Formeln, die Namen und die Verknüpfungen zurück, bis Sie die Arbeitsmappe speichern. Ist keine Verknüpfung mehr vorhanden, wird die Datei ohne die Teile gespeichert, die sie enthalten haben.

## Formular

**Formular** (in der Gruppe **Datentools** und im Suchfeld) zeigt jeweils eine Zeile einer Liste als kleines Formular. Eine Liste ist eine Tabelle oder ein Zellbereich mit einer Zeile mit Namen oben und einer Datenzeile unter jedem Namen. Klicken Sie zuerst auf eine beliebige Zelle der Liste.

- Jedes Feld ist eine Spalte, beschriftet mit dem Namen aus der obersten Zeile. Geben Sie in einem Feld ein, um den Wert zu ändern. Ein ausgegrautes Feld enthält eine Formel: Das Formular zeigt das Ergebnis an, und Sie ändern die Formel im Blatt.
- **Weitersuchen** und **Zurücksuchen** gehen zur nächsten und zur vorherigen Zeile. Wenn Sie zu einer anderen Zeile wechseln oder die Eingabetaste drücken, wird das, was Sie in dieser Zeile eingegeben haben, geschrieben.
- **Neu** zeigt ein leeres Formular. Füllen Sie es aus, und drücken Sie dann erneut **Neu**, **Weitersuchen** oder **Schließen**. Die Zeile wird unter der letzten Zeile der Liste hinzugefügt. Eine Tabelle wächst um eine Zeile. Ist eine Zelle unter der Liste nicht leer, meldet das Formular das und fügt nichts hinzu.
- **Löschen** entfernt die Zeile, die Sie gerade ansehen, nachdem Sie gefragt wurden. Die Zeilen darunter rücken nach oben. Eine Tabelle behält immer mindestens eine Datenzeile. Eine Zeile, die ausgeblendet ist oder die ein Filter ausblendet, wird im Formular nicht angezeigt, nicht gezählt und durch das Formular nie gelöscht. Ist die Liste zu lang, um ganz gefunden zu werden, meldet das Formular, dass nur die ersten Zeilen angezeigt werden.
- **Wiederherstellen** nimmt zurück, was Sie in dieser Zeile eingegeben haben, bevor Sie sie verlassen.
- **Kriterien** lässt Sie nach Zeilen suchen. Geben Sie eine Bedingung in einem oder mehreren Feldern ein, und drücken Sie dann **Weitersuchen** oder **Zurücksuchen**. Eine Bedingung ist ein Wert, etwa **Ann** (Text stimmt mit dem Wortanfang überein, und **?** sowie ***** stehen für Buchstaben), oder ein Wert nach einem Zeichen: **>100**, **<=5**, **<>0**, **=Oslo**. Eine Zeile muss jede Bedingung erfüllen, die Sie eingeben. Drücken Sie **Formular**, um zurückzukehren, oder **Leeren**, um neu zu beginnen. Solange Kriterien gesetzt sind, überspringen **Weitersuchen** und **Zurücksuchen** die Zeilen, die sie nicht erfüllen.
- **Schließen** schreibt die Zeile und schließt das Formular.

Jede Zeile, die Sie ändern, hinzufügen oder löschen, ist ein Schritt, daher nimmt **Rückgängig** im Blatt sie mit einem Mal zurück.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte Start. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie dies, wenn eine Spalte mehrere Textteile enthält, die Sie in getrennten Spalten haben möchten, etwa „Schmidt, Anna, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **An einem Trennzeichen aufteilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wobei Sie in dem Feld Ihr eigenes Zeichen eingeben. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. **Text in Anführungszeichen** legt das Zeichen fest, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen steht ein doppeltes Zeichen für eines.
   - **An festen Positionen aufteilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei Zeichen ab 1 gezählt werden. Zum Beispiel beginnt bei 5, 12 eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Wählen Sie, was jeder Teil ist: **Standard** liest Zahlen und Datumsangaben so, wie Sie sie eingegeben haben, und **Text** behält jeden Teil unverändert, sodass „007“ als „007“ bleibt.
   Um einen Teil anders zu behandeln, geben Sie seine Nummer unter **Teil** ein und wählen Sie, wie er gelesen wird: **Wie oben**, **Wie eingegeben**, **Text**, ein Datum in der Reihenfolge **Monat, Tag, Jahr**, **Tag, Monat, Jahr** oder **Jahr, Monat, Tag** oder **Auslassen**, wodurch dieser Teil nirgendwohin kommt. Der erste Teil ist 1. Ein Datumsteil darf den Monat als Namen enthalten (**5. März 2024**, **März 5, 2024**) auf Englisch oder in Ihrer Sprache, darf durch /, -, einen Punkt, ein Komma oder Leerzeichen getrennt sein und darf von einer Uhrzeit gefolgt werden (**10:30**, **10:30:45** oder **10:30 PM**). Ein Teil, der in der gewählten Reihenfolge kein Datum ist (ein Monat 13, ein Tag, den es nicht gibt, ein Jahr, das nicht zwei oder vier Ziffern hat, oder eine Uhrzeit, die keinen Sinn ergibt), bleibt als Text erhalten, so wie er eingegeben wurde; er wird niemals in einer anderen Reihenfolge gelesen.
5. Geben Sie unter **Teile einfügen ab** die erste Zelle für die Teile ein. Sie beginnt mit der ersten Zelle, die Sie markiert haben.
6. Das Feld unter den Optionen zeigt die ersten Zeilen so geteilt, wie Sie es gewählt haben. Drücken Sie **OK**.

Die Teile werden in die Zellen rechts neben der ersten Zelle geschrieben und ersetzen dabei, was dort stand. Text in Spalten ist ein Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie dies, um von jeder Gruppe sich wiederholender Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben eine Kopfzeile**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Wiederholungen gelten. Zwei Zeilen sind nur dann Wiederholungen, wenn jede aktivierte Spalte gleich ist. Groß- und Kleinschreibung gelten als gleich, und eine Formel zählt als der Wert, den sie ergibt.
5. Drücken Sie **OK**. Das Dialogfeld meldet, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, mit ihren Formaten und Formeln. Die Zellen unter der Auswahl bewegen sich nicht. Die leeren Zeilen am Ende der Auswahl werden geleert. Duplikate entfernen ist ein Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie dies, wenn Sie das gewünschte Ergebnis kennen, aber nicht die Zahl, die es erzeugt. Zum Beispiel enthält eine Zelle für die Rate eine Formel mit einem Zinssatz, und Sie möchten, dass die Rate 500 beträgt. Die Zielwertsuche probiert Zahlen in der Zinssatzzelle aus, bis die Rate 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie beginnt mit der Zelle, die Sie markiert haben.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Veränderbare Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Drücken Sie **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der veränderbaren Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Findet sie keine, ändert sich nichts, und Sie erhalten eine Meldung. Versuchen Sie einen anderen Startwert in der veränderbaren Zelle, und prüfen Sie, ob die Formel tatsächlich diese Zelle verwendet. Die Zielwertsuche ist ein Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist eine benannte Gruppe von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt Was-wäre-wenn-Fälle durchzuspielen. Behalten Sie zum Beispiel ein Szenario „Geringe Umsätze“ und ein Szenario „Hohe Umsätze“ für dieselben Eingabezellen, und zeigen Sie das an, das Sie sich ansehen möchten.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Szenario-Manager**.
2. Drücken Sie **Hinzufügen**. Geben Sie einen Namen ein, etwa Hohe Umsätze. Geben Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario festlegt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die Zellen, die Sie markiert haben, werden automatisch eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt enthält. Geben Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und drücken Sie **OK**.
3. Wählen Sie in der Liste ein Szenario aus, und drücken Sie **Anzeigen**, um seine Werte in die Zellen zu schreiben. Ein Doppelklick auf einen Namen bewirkt dasselbe. Drücken Sie **Bearbeiten**, um Namen, Kommentar oder Werte zu ändern. Drücken Sie **Löschen**, um es zu entfernen.

Anzeigen, Hinzufügen, Bearbeiten und Löschen sind jeweils ein Schritt, den Sie rückgängig machen können.

**Übersicht** stellt alle Szenarien nebeneinander auf einem neuen Blatt namens Szenarioübersicht dar. Das Verfahren fragt nach den Ergebniszellen, also den Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Übersicht die Werte der veränderbaren Zellen und die Ergebnisse der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte namens Aktuelle Werte zeigt, wie die Zellen beim Erstellen der Übersicht waren. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine Arbeitsmappe, die mit einem anderen Programm erstellt wurde, behält ihre Szenarien, und Sie sehen sie hier. Eine Kopie, die als älteres .xls-Format oder als .ods gespeichert wird, enthält sie nicht, und beim Speichern erhalten Sie einen Hinweis darauf.

## Teilergebnis

Verwenden Sie dies, um unter jede Gruppe von Zeilen, die in einer Spalte denselben Eintrag haben, eine Zeile mit Summen zu setzen, und unten eine Gesamtsumme. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der Sie gruppieren, damit gleiche Einträge nebeneinanderstehen.

1. Wählen Sie eine Zelle in der Liste.
2. Wählen Sie **Teilergebnis** in der Gruppe **Gliederung** auf der Registerkarte **Daten**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden** aus, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen.
7. **Übersicht unter Daten** übernimmt die Einstellung der Gliederung des Blatts. Lassen Sie sie aktiviert, damit jede Summenzeile unter ihrer Gruppe steht, oder deaktivieren Sie sie, damit jede Summenzeile über ihrer Gruppe steht, mit der Gesamtsumme oben. Drücken Sie **OK**.

Jede Summenzeile trägt die Beschriftung „Ost Summe“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, die die anderen Summenzeilen ausschließt. Die Zeilen jeder Gruppe sind gruppiert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Summen anzeigen können. Um die Summen zu entfernen, öffnen Sie **Teilergebnis** erneut und drücken Sie **Alle entfernen**. Wurden die Summen auf die andere Seite eines Blatts gesetzt, das bereits eigene Gruppen hatte, meldet die Statusleiste, dass diese Gruppen ihre Übersichtszeilen nun auf dieser Seite haben. **Alle entfernen** stellt die Seite dann so wieder her, wie sie vor dem ersten Teilergebnis war, sofern Sie sie seitdem nicht selbst geändert haben. Stehen die Summen oben, befinden sich die Gliederungsschaltflächen bei den Summenzeilen, sodass eine Plus- oder Minusschaltfläche neben „Ost Summe“ die Ost-Zeilen darunter ein- oder ausblendet. Deaktivieren Sie **Aktuelle Teilergebnisse ersetzen**, muss die neue Summe auf derselben Seite stehen wie die vorhandenen. Andernfalls wird nichts geändert, und eine Meldung weist darauf hin. Das Hinzufügen oder Entfernen von Teilergebnissen ist ein Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. **Erweitert** (in der Gruppe **Sortieren und Filtern**) lässt Sie die Bedingungen auf dem Blatt schreiben, sodass sie so lang sein können, wie Sie brauchen.

1. Kopieren Sie neben die Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile leerer Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle wahr sein (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, ob **Listenbereich** die Liste mit ihrer Kopfzeile ist, und geben Sie die Zellen Ihrer Überschriften und Bedingungen unter **Bedingungsbereich** ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Die Zeilen an einen anderen Ort kopieren**, und geben Sie die erste Zelle des Zielorts ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je eine Zeile zu kopieren.
5. Drücken Sie **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges Zeichen und ***** für eine beliebige Folge von Zeichen.
- Eine Zahl wählt Zellen, die ihr entsprechen.
- **=Ann** wählt Text, der genau Ann ist. **=** allein wählt leere Zellen.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen, die nicht leer sind.

Eine Zeile mit Bedingungen, in der nichts steht, wählt alle Zeilen aus. Um die ausgeblendeten Zeilen wieder einzublenden, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. Der erweiterte Filter ist ein Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie dies, um festzulegen, was in bestimmte Zellen eingegeben werden darf, damit eine falsche Eingabe verhindert oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie **Datenüberprüfung** in der Gruppe **Datentools** auf der Registerkarte **Daten**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Wählen Sie bei einer Zahl, einem Datum, einer Uhrzeit oder einer Textlänge unter **Daten** (**zwischen**, **nicht zwischen**, **gleich**, und so weiter) und geben Sie die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Bei einer **Liste** geben Sie die Auswahlmöglichkeiten mit Semikolons getrennt ein, etwa Rot;Grün;Blau, oder einen Zellbereich nach =, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown in Zelle**, um den Pfeil zu verbergen.
6. Bei **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. **Leere Zellen ignorieren** lässt eine leere Zelle durch.

Auf der Registerkarte **Eingabemeldung** aktivieren Sie das Kästchen und schreiben einen Titel und eine Meldung. Sie werden neben der Zelle angezeigt, sobald sie ausgewählt ist.

Auf der Registerkarte **Fehlermeldung** legen Sie fest, was mit einer unzulässigen Eingabe geschieht. **Stopp** lehnt sie ab. **Warnung** fragt, ob sie behalten werden soll. **Information** teilt es der Person mit und behält die Eingabe. Schreiben Sie für die Meldung einen Titel und einen Text.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) setzt um jede Zelle im Blatt, deren Wert ihre Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel dort stand, oder das Ergebnis einer Formel, ein rotes Oval. Eine Meldung sagt Ihnen, ob alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihr Oval. **Überprüfungskreise entfernen** nimmt alle Ovale weg.

**Alle löschen** entfernt die Regeln aus den markierten Zellen. Die neue Regel ersetzt alle Regeln, die die markierten Zellen hatten; andere Zellen behalten ihre. Das Ändern von Regeln ist ein Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie dies, um ähnliche Bereiche, etwa dieselbe Tabelle aus mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie in der Gruppe **Datentools** auf der Registerkarte **Daten** den Befehl **Konsolidieren**.
2. Wählen Sie unter **Funktion** die Funktion für die Zusammenfassung, etwa **Summe** oder **Mittelwert**.
3. Geben Sie einen Bereich in das Feld ein und drücken Sie **Hinzufügen**. Verwenden Sie **Tabelle2!A1:C10** für einen Bereich auf einem anderen Blatt. Fügen Sie jeden Bereich hinzu, den Sie zusammenfassen möchten. Markieren Sie in der Liste einen Bereich und drücken Sie **Löschen**, um ihn zu entfernen.
4. Haben die Bereiche Namen in ihrer ersten Zeile, aktivieren Sie **Oberste Zeile enthält die Spaltenbeschriftungen**. Haben sie Namen in ihrer ersten Spalte, aktivieren Sie **Linke Spalte enthält die Zeilenbeschriftungen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die auf seine Quellzellen verweist, sodass es sich ändert, wenn diese sich ändern. Ohne dieses Kästchen sind die Ergebnisse einfache Zahlen.
7. Drücken Sie **OK**.

Ohne aktivierte Beschriftungen wird die Zelle an derselben Stelle in jedem Bereich zusammengefasst (zuerst die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Beschriftungen werden Zellen mit denselben Namen zusammengefasst, unabhängig davon, wo sie im jeweiligen Bereich stehen, und ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. Konsolidieren ist ein Schritt, den Sie rückgängig machen können.

## Prognoseblatt

Das Prognoseblatt erstellt ein neues Blatt mit einer Tabelle und einem Diagramm zukünftiger Werte, die aus einer Zeitachse und einer Reihe von Werten berechnet werden. Es ermittelt den Trend und die sich wiederholende Saison der Werte und fügt ein Band hinzu, in dem die künftigen Werte voraussichtlich bleiben.

1. Markieren Sie die Zeitachse und die Werte, zwei nebeneinanderliegende Spalten (die Zeitachse zuerst). Die Zeitachse besteht aus Datumsangaben oder Zahlen mit gleichem Abstand, etwa jeden Monat oder alle 5 Einheiten. Eine erste Zeile mit Namen wird nicht berücksichtigt.
2. Wählen Sie **Prognoseblatt** in der Gruppe **Prognose** auf der Registerkarte **Daten**.
3. Ein Bild zeigt die Prognose und ändert sich mit Ihren Einstellungen. Wählen Sie **Liniendiagramm** oder **Säulendiagramm**.
4. Legen Sie die Optionen fest:
   - **Zeitachsenbereich** und **Wertebereich** sind die Zellen, die gelesen werden. Schreiben Sie hier andere Zellen, falls die Auswahl nicht stimmte.
   - **Prognosebeginn** bleibt leer, um nach dem letzten Punkt zu beginnen. Ein Datum oder eine Zahl vor dem letzten Punkt beginnt dort und verwendet nur die Punkte davor, sodass Sie die Prognose mit dem tatsächlichen Verlauf vergleichen können.
   - **Prognoseende** ist der letzte Zeitpunkt der Prognose. Es werden höchstens 10.000 Punkte prognostiziert.
   - **Konfidenzintervall** gibt an, wie sicher das Band ist, anfangs 95 %.
   - **Saisonalität** wird aus den Werten ermittelt. Sie können aber auch die Länge der Saison in Punkten festlegen oder Keine wählen. Die Werte müssen zwei vollständige Saisons umfassen.
   - **Fehlende Punkte auffüllen mit** legt fest, was ein Zeitpunkt ohne Wert erhält: eine gerade Linie zwischen seinen Nachbarn oder null.
   - **Doppelte Werte zusammenfassen mit** legt fest, wie Werte zum selben Zeitpunkt kombiniert werden: Mittelwert, Anzahl, Anzahl (Werte), Max, Median, Min oder Summe.
   - **Prognosestatistik einbeziehen** fügt eine Tabelle mit Alpha, Beta, Gamma, MASE, SMAPE, MAE und RMSE hinzu, die zeigt, wie gut die Prognose zu den Werten passt.
5. Ist etwas falsch, beschreibt das Dialogfeld in einfachen Worten, was (Schritte, die nicht gleich sind, zu wenige Punkte, Text in den Werten), und **Erstellen** wartet.
6. Drücken Sie **Erstellen**.

Das neue Blatt wird vor dem Blatt eingefügt, auf dem Sie gerade waren. Seine Tabelle hat die Spalten Zeitachse, Werte, Prognose, Untere Konfidenzgrenze und Obere Konfidenzgrenze. Die Prognose und die Grenzen sind Formeln, die `PROGNOSE.ETS` und `PROGNOSE.ETS.KONFINT` auf den eigenen Spalten Zeitachse und Werte der Tabelle verwenden, sodass sie neu berechnet werden, wenn Sie einen Wert ändern. Das Diagramm zeigt die Werte, die Prognose und das Band. Das Prognoseblatt ist ein Schritt, den Sie rückgängig machen können.

Die Zahlen können um einige Prozent von denen anderer Tabellenkalkulationsprogramme abweichen, die ihre eigene Methode zur Anpassung der Glättung verwenden. Eine Prognose, die eine Datei bereits enthält, behält die Werte, die die Datei gespeichert hat, bis sich die Daten, die sie liest, ändern.

Eine Datei mit mehr als 1.000.000 Punkten wird nicht verwendet.
