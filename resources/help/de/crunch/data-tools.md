# Datentools
<!-- keywords: Prognoseblatt, Prognose, zukünftige Werte, Konfidenzintervall, Saisonalität, Trend, Glättung, Formular, Datenmaske, Dateneingabe, Datensatz, neuer Datensatz, weitersuchen, vorheriges suchen, Kriterien, Szenario-Manager, Szenarien, Was-wäre-wenn, Szenarioübersicht, Daten, Registerkarte Daten, Text in Spalten, aufteilen, Trennzeichen, Komma, Tabstopp, feste Breite, Duplikate entfernen, Zielwertsuche, Teilergebnis, Summen, erweiterter Filter, Bedingungen, Datenüberprüfung, ungültige Daten einkreisen, rote Ellipse, Überprüfungskreise entfernen, konsolidieren, Bereiche zusammenfassen, zulassen, Liste, Dropdown, Eingabemeldung, Fehlermeldung, wiederholte Zeilen, eindeutig, Textdatei, CSV, aus Text, Spalten, Trennzeichen, Anführungszeichen, Verknüpfungen bearbeiten, Werte aktualisieren, Quelle ändern, Verknüpfungen, extern, Quelle, Verknüpfung aufheben, Status prüfen, Quelle öffnen -->

Auf der Registerkarte **Daten** finden Sie die Werkzeuge, die mit einer Liste von Werten arbeiten: Text aus einer Datei holen, eine Liste sortieren, Text in Spalten aufteilen und wiederholte Zeilen entfernen.

## Aus Text/CSV

**Aus Text/CSV** öffnet eine Textdatei (.csv, .tsv oder .txt). Der Befehl entspricht **Textdatei öffnen** auf der Registerkarte Datei. JeffCrunch fragt, wie die Datei in Spalten aufgeteilt ist und was jede Spalte enthält, und zeigt sie dann als neue Arbeitsmappe an. Dasselbe Fenster erscheint, wenn Sie eine Textdatei öffnen oder **Textimport einfügen** verwenden.

Das Fenster schätzt anhand der ersten 200 Zeilen der Datei, was jede Spalte enthält, und zeigt die Schätzung im Feld über der Spalte an. Ändern Sie vor dem Öffnen der Datei jede Spalte, die nicht stimmt. Eine Art, die Sie ändern, wird nie durch eine spätere Schätzung ersetzt, auch wenn Sie danach das Trennzeichen ändern. Eine Schätzung wirkt nur auf drei Arten auf eine Spalte: Sie lässt die Spalte auf **Wie eingegeben**, also so, wie eine Datei gelesen wird, wenn nichts geschätzt wird; sie macht die Spalte zu **Text**, um sie zu schützen; oder sie macht die Spalte zu Datumswerten in einer Reihenfolge, die die Werte belegen. Dafür gelten diese Regeln:

- Eine Spalte bleibt **Wie eingegeben**, wenn „Wie eingegeben“ ihre Werte richtig liest: Zahlen, Wahr und Falsch, Uhrzeiten, Datumswerte mit Monatsnamen, Zahlen mit Vorzeichen wie -5 und +3, Wörter und Versionsnummern wie 1.13.2. Eine Änderung würde diese Werte nur verschlechtern.
- Eine Spalte ist ein **Datum** nur dann, wenn jeder Wert eine vierstellige Jahreszahl hat (oder ein vollständiges Datum wie 2024-05-13 ist) und in genau einer Reihenfolge als Datum gelesen wird (Monat, Tag, Jahr; Tag, Monat, Jahr; oder Jahr, Monat, Tag). Ihre Datumswerte werden so angezeigt, wie Sie Datumswerte eingeben. Passen mehrere Reihenfolgen zu allen Werten, etwa bei 05.06.2024, bleibt die Spalte als **Text** erhalten, das Fenster führt sie auf, und Sie wählen die Reihenfolge in ihrem Feld aus.
- Werte mit drei Zahlen und ohne vierstellige Jahreszahl, etwa 1.13.2 oder 13.05.24, macht die Schätzung nie zu Datumswerten. Würde „Wie eingegeben“ sie als Text belassen, bleibt die Spalte **Wie eingegeben**; würde „Wie eingegeben“ sie in Datumswerte umwandeln, wird die Spalte **Text**. In beiden Fällen führt das Fenster eine Spalte auf, die Datumswerte mit zweistelligen Jahreszahlen sein könnte, damit Sie eine Reihenfolge wählen können.
- Eine Spalte ist **Text**, wenn ein Wert mit einer Null beginnt, der weitere Ziffern folgen, wenn ein Wert mehr als 15 Ziffern hat, wenn ein Pluszeichen vor sieben oder mehr Ziffern steht (eine Telefonnummer) oder wenn eine Exponentschreibweise wie 1E5 neben anderen Werten steht.
- Ein Wert, der mit einem Gleichheitszeichen beginnt, wird dem Feld **Formeln lesen** überlassen. Ist das Feld aktiviert, bleibt die Spalte **Wie eingegeben**, damit der Wert als Formel gelesen wird. Ist es nicht aktiviert, wird die Spalte zu **Text**.
- Aktivieren Sie **Die erste Zeile enthält Spaltennamen**, dann wird die erste Zeile bei der Schätzung nicht berücksichtigt. Das Fenster aktiviert es, wenn die erste Zeile über Zahlen oder Datumswerten Text enthält.
- Die Spalten nach den ersten 100 werden als Text übernommen.

Es werden nur die ersten 200 Zeilen geprüft; ein Wert weiter unten kann daher überraschen. In einer Datumsspalte kommt ein weiter unten stehender Wert, der in der gewählten Reihenfolge kein Datum ist, als Text und nie als falsches Datum. In einer Textspalte wird jeder Wert genau so übernommen, wie er geschrieben ist. In einer Spalte **Wie eingegeben** wird ein weiter unten stehender Wert so gelesen, wie „Wie eingegeben“ ihn überall sonst liest.

**Textimport einfügen** fügt den Text so ein, als hätten Sie ihn selbst eingegeben, mit Ihrem Dezimal- und Tausendertrennzeichen. Deshalb hat das Fenster dafür keine Felder für diese Zeichen, und die Schätzung liest den Text genauso wie das Einfügen. Das Fenster für eine Datei hat diese Felder.

## Aus Datei

**Aus Datei** in der Gruppe **Daten abrufen** lädt die Zeilen einer JSON-Datei, einer XML-Datei oder einer anderen Arbeitsmappe als Tabelle. Es kann nur eine Datei auf diesem Computer geladen werden. Eine Webadresse oder eine Datei auf einem anderen Computer wird abgelehnt.

1. Klicken Sie auf **Aus Datei**, dann auf **Durchsuchen**, und wählen Sie die Datei aus. Die ersten Zeilen werden sofort angezeigt, mit der Art, die JeffCrunch für jede Spalte aus diesen ersten Zeilen erkannt hat: Zahl, Datum, Wahr oder Falsch, oder Text. Beim Laden wird die ganze Datei geprüft, daher kann eine Spalte am Ende Text sein. Auf dem Blatt ändert sich nichts, bis Sie auf **Laden** klicken.
2. Enthält die Datei mehr als eine Liste, wählen Sie die zu ladende aus. In einer JSON-Datei ist es eine Liste von Elementen mit benannten Werten, etwa `$.items`. In einer XML-Datei ist es ein Element, das sich wiederholt, etwa `/feed/entry`. In einer Arbeitsmappe ist es ein Blatt, eine Tabelle oder ein benannter Bereich.
3. Ein JSON-Element innerhalb eines Elements wird zu einer Spalte namens `äußer.innen`, bis zu vier Ebenen tief. Eine Liste einfacher Werte wird in eine Zelle zusammengefügt (die ersten 10.000 Werte) oder auf Spalten verteilt (die ersten 16), je nachdem, was Sie wählen. In XML werden die Attribute und die untergeordneten Elemente zu Spalten. Zwei verschiedene Werte teilen sich nie eine Spalte: Würden zwei denselben Namen erhalten, wird dem zweiten eine Zahl angehängt oder, in XML, sein Präfix vorangestellt, und eine Notiz weist darauf hin. Ein Name, der sich innerhalb eines JSON-Elements wiederholt, behält seinen letzten Wert, und auch dazu gibt es eine Notiz. Eine Zahl, die für eine Zelle zu groß oder zu klein ist, bleibt als Text erhalten, so wie sie geschrieben ist, und ein Zeichen, das nicht gelesen werden kann, wird durch ein Ersatzzeichen angezeigt; beides wird vermerkt.
4. Aktivieren Sie **Alle Spalten als Text laden**, um jeden Wert so zu behalten, wie er geschrieben ist. Eine Spalte mit Zahlen mit führender Null, etwa 007, bleibt immer Text.
5. Geben Sie die Zelle ein, in die die Tabelle kommt, und klicken Sie dann auf **Laden**. Würde die Tabelle Zellen überdecken, die etwas enthalten, fragt JeffCrunch, bevor es sie ersetzt. Gibt es keinen Platz oder ist das Blatt geschützt, erhalten Sie eine Meldung und nichts ändert sich.

Die Tabelle ist ein Schritt von **Rückgängig**. Sie merkt sich die Datei und Ihre Auswahl. Um die Datei erneut zu lesen, klicken Sie in der Tabelle auf eine Zelle und dann auf der Registerkarte Daten auf **Aktualisieren**. **Alle aktualisieren** ist die große Schaltfläche daneben: Sie liest jede Tabelle, die mit „Daten abrufen“ erstellt wurde, und aktualisiert danach jede Pivot-Tabelle, alles als ein Schritt von Rückgängig. Die Tabelle wächst oder schrumpft mit den Zeilen der Datei. Spalten, die Sie neben der Tabelle hinzugefügt haben, bleiben erhalten: Eine Formel, die Sie nach unten kopiert haben, wird in neue Zeilen übernommen, und Zeilen, die aus der Datei verschwunden sind, verschwinden auch aus der Tabelle. Ist die Datei nicht mehr vorhanden oder kann sie nicht gelesen werden, erhalten Sie eine Meldung und nichts ändert sich. Haben sich die Spalten der Datei selbst geändert, erfahren Sie, welche fehlen oder neu sind, und nichts ändert sich. Erstellen Sie die Tabelle dann mit „Daten abrufen“ neu, oder stellen Sie die Spalten in der Datei wieder her.

Die Verknüpfung zur Datei wird in der Tabelle in einer gespeicherten `.xlsx`-Arbeitsmappe gehalten, in einem Teil, den andere Programme ignorieren. Die Verknüpfung enthält den vollständigen Pfad der Datei auf diesem Computer, daher kann jeder, der die Arbeitsmappe erhält, diesen Pfad lesen. Um ihn vor der Weitergabe der Arbeitsmappe zu entfernen, verwenden Sie **Prüfen**, dann **Links zu Datendateien** und dann **Entfernen**. Wird eine Kopie als eine andere Dateiart gespeichert, bleibt die Verknüpfung nicht erhalten; **Kompatibilität prüfen** weist darauf hin. **Prüfen** führt die Verknüpfungen unter **Links zu Datendateien** auf und kann sie entfernen. Die Tabellen und ihre Zellen bleiben erhalten.

Dateien werden mit Grenzen gelesen: eine Datei bis 64 MB, 200.000 Zeilen, 256 Spalten und 32.767 Zeichen in einer Zelle. XML mit einem Dokumenttyp oder mit Entitäten wird abgelehnt. Alles, was durch eine Grenze abgeschnitten wird, wird angegeben.

## Verknüpfungen bearbeiten

Enthält eine Arbeitsmappe Formeln, die eine andere Arbeitsmappe lesen (etwa ='[Preise.xlsx]Tabelle1'!A1), listet **Verknüpfungen bearbeiten** in der Gruppe **Daten abrufen** diese anderen Arbeitsmappen auf, die Quellen heißen. Jede Zeile zeigt, wo die Quelle liegt, und ihren Status. Bis die Arbeitsmappe mit ihren Quellen erneut geöffnet wird, zeigt sie die Zahlen, die ihre Datei von dort gespeichert hat.

- **Status prüfen** sucht jede Quelle auf diesem Computer und meldet **Gefunden** oder dass sie nicht gefunden wurde. Eine Quelle im Web wird nicht abgerufen; ihr Status lautet dann, dass sie nicht geprüft wurde.
- **Werte aktualisieren** liest die gewählte Quelle von diesem Computer und gibt jeder Zelle, die nur auf eine ihrer Zellen verweist, den Wert, den diese Zelle jetzt hat. Eine Zelle, die mit dem Wert mehr macht, etwa `=[1]Preise!A1*2`, behält die Zahl, die ihre Datei gespeichert hat. Die Meldung unter der Liste sagt, wie viele Zellen aktualisiert wurden und wie viele nicht. Werte aktualisieren ist ein Schritt, daher stellt **Rückgängig** die alten Werte wieder her. Die neuen Werte werden beim Speichern auch in den eigenen Teil der Verknüpfung in der Datei geschrieben, sodass die Arbeitsmappe sie beim nächsten Öffnen zeigt, selbst wenn die Quelle nicht vorhanden ist.
- **Quelle ändern** lässt Sie eine andere Datei für die gewählte Verknüpfung auswählen, etwa wenn die Arbeitsmappe verschoben wurde. Die Formeln bleiben unverändert. Der neue Ort wird beim Speichern in die Datei geschrieben, und **Rückgängig** nimmt die Änderung zurück.
- **Quelle öffnen** öffnet die gewählte Quelle in einem neuen Fenster, wenn sie gefunden werden kann.
- **Verknüpfung aufheben** wandelt jede Zelle, die auf die gewählte Quelle verweist, in den Wert um, den sie jetzt anzeigt, und ein Name, der darin für eine Stelle stand, in **#BEZUG!**. JeffCrunch fragt vorher nach. Verknüpfung aufheben ist ein Schritt, daher bringt **Rückgängig** die Formeln, die Namen und die Verknüpfungen zurück, bis Sie die Arbeitsmappe speichern. Ist keine Verknüpfung mehr vorhanden, wird die Datei ohne die Teile gespeichert, die sie enthielten.

## Formular

**Formular** (in der Gruppe **Datentools** und im Suchfeld) zeigt jeweils eine Zeile einer Liste als kleines Formular an. Eine Liste ist eine Tabelle oder ein Zellbereich mit einer Zeile mit Namen oben und einer Datenzeile unter jedem Namen. Klicken Sie zuerst in eine beliebige Zelle der Liste.

- Jedes Feld ist eine Spalte und trägt den Namen aus der obersten Zeile als Beschriftung. Geben Sie im Feld einen Wert ein, um ihn zu ändern. Ein ausgegrautes Feld enthält eine Formel: Das Formular zeigt das Ergebnis an, und Sie ändern die Formel im Blatt.
- **Weitersuchen** und **Vorheriges suchen** gehen zur nächsten bzw. vorherigen Zeile. Wenn Sie zu einer anderen Zeile wechseln oder die **Eingabetaste** drücken, wird das, was Sie in dieser Zeile eingegeben haben, geschrieben.
- **Neu** zeigt ein leeres Formular. Füllen Sie es aus, und drücken Sie dann erneut **Neu**, **Weitersuchen** oder **Schließen**. Die Zeile wird unter der letzten Zeile der Liste angefügt. Eine Tabelle wächst um eine Zeile. Ist eine Zelle unter der Liste nicht leer, meldet das Formular es und fügt nichts hinzu.
- **Löschen** entfernt die angezeigte Zeile, nachdem Sie bestätigt haben. Die Zeilen darunter rücken nach oben. Eine Tabelle behält immer mindestens eine Datenzeile. Eine Zeile, die ausgeblendet ist oder die ein Filter ausblendet, wird im Formular nicht angezeigt, nicht gezählt und durch das Formular nie gelöscht. Ist die Liste zu lang, um ganz gefunden zu werden, meldet das Formular, dass nur die ersten Zeilen angezeigt werden.
- **Wiederherstellen** nimmt zurück, was Sie in dieser Zeile eingegeben haben, bevor Sie sie verlassen.
- **Kriterien** hilft beim Suchen von Zeilen. Geben Sie eine Bedingung in ein oder mehrere Felder ein, und drücken Sie dann **Weitersuchen** oder **Vorheriges suchen**. Eine Bedingung ist ein Wert, etwa **Ann** (Text beginnt mit dem Wert, und **?** und ***** stehen für Zeichen), oder ein Wert nach einem Vergleichszeichen: **>100**, **<=5**, **<>0**, **=Oslo**. Eine Zeile muss jede Bedingung erfüllen, die Sie eingeben. Drücken Sie **Formular**, um zurückzukehren, oder **Leeren**, um neu zu beginnen. Solange Kriterien gesetzt sind, überspringen **Weitersuchen** und **Vorheriges suchen** die Zeilen, die sie nicht erfüllen.
- **Schließen** schreibt die Zeile und schließt das Formular.

Jede Zeile, die Sie ändern, hinzufügen oder löschen, ist ein Schritt, daher nimmt **Rückgängig** im Blatt sie mit einem Schritt zurück.

## Sortieren und Filtern

Die Gruppe **Sortieren und Filtern** enthält dieselben Befehle wie das Menü **Sortieren und Filtern** auf der Registerkarte Start. Siehe [Sortieren und Filtern](sort-filter).

## Text in Spalten

Verwenden Sie dies, wenn eine Spalte mehrere Textteile enthält, die in getrennten Spalten stehen sollen, etwa „Schmidt, Ann, 31“.

1. Markieren Sie die Zellen **einer** Spalte.
2. Wählen Sie auf der Registerkarte **Daten** den Befehl **Text in Spalten**.
3. Wählen Sie, wie der Text aufgeteilt werden soll:
   - **Bei Trennzeichen teilen**: Aktivieren Sie die Zeichen, die zwischen den Teilen stehen: **Tabstopp**, **Semikolon**, **Komma**, **Leerzeichen** oder **Andere**, wo Sie Ihr eigenes Zeichen eingeben. **Aufeinanderfolgende Trennzeichen als eines behandeln** verhindert, dass zwei Kommas hintereinander einen leeren Teil erzeugen. **Text in Anführungszeichen** nennt das Zeichen, das einen Teil umschließt, der nicht geteilt werden darf, etwa „Lee, Bo“. Innerhalb der Anführungszeichen steht zweimal dasselbe Zeichen für eines.
   - **Bei festen Positionen teilen**: Geben Sie die Stellen ein, an denen eine neue Spalte beginnt, wobei die Zeichen ab 1 gezählt werden. Zum Beispiel beginnt bei 5, 12 eine neue Spalte beim fünften und beim zwölften Zeichen.
4. Wählen Sie, was jeder Teil ist: **Standard** liest Zahlen und Datumswerte so, als hätten Sie sie eingegeben, und **Text** behält jeden Teil so, wie er ist, sodass „007“ als „007“ bleibt.
   Um einen Teil anders zu behandeln, geben Sie seine Nummer unter **Teil** ein und wählen Sie, wie er gelesen wird: **Wie oben**, **Wie eingegeben**, **Text**, ein Datum im Format **Monat, Tag, Jahr**, **Tag, Monat, Jahr** oder **Jahr, Monat, Tag**, oder **Auslassen**, wodurch dieser Teil nirgends hingeschrieben wird. Der erste Teil ist 1. Ein Datumsteil darf den Monat als Namen enthalten (**5. März 2024**, **März 5, 2024**) auf Englisch oder in Ihrer Sprache, darf durch /, -, einen Punkt, ein Komma oder Leerzeichen getrennt sein und darf von einer Uhrzeit gefolgt sein (**10:30**, **10:30:45** oder **10:30 PM**). Ein Teil, der in der gewählten Reihenfolge kein Datum ist (ein Monat 13, ein Tag, den es nicht gibt, eine Jahreszahl mit anderer Stellenzahl als zwei oder vier, oder eine unsinnige Uhrzeit), bleibt als Text erhalten, genau so, wie er geschrieben wurde; er wird nie in einer anderen Reihenfolge gelesen.
5. Geben Sie unter **Die Teile beginnen in** die erste Zelle für die Teile ein. Sie beginnt als die erste Zelle, die Sie markiert haben.
6. Das Feld unter den Auswahlmöglichkeiten zeigt die ersten Zeilen so aufgeteilt, wie Sie es gewählt haben. Klicken Sie auf **OK**.

Die Teile kommen in die Zellen rechts neben der ersten Zelle und ersetzen dort vorhandenen Inhalt. Text in Spalten ist ein Schritt, den Sie rückgängig machen können.

## Duplikate entfernen

Verwenden Sie dies, um von jeder Gruppe sich wiederholender Zeilen nur die erste zu behalten.

1. Markieren Sie die Zellen der Liste oder eine Zelle darin. JeffCrunch nimmt den gesamten Datenbereich um eine einzelne Zelle.
2. Wählen Sie **Duplikate entfernen**.
3. Aktivieren Sie **Meine Daten haben Kopfzeilen**, wenn die erste Zeile Namen enthält. Diese Zeile bleibt erhalten und wird nicht verglichen.
4. Aktivieren Sie die Spalten, die übereinstimmen müssen, damit zwei Zeilen als Wiederholungen gelten. Zwei Zeilen sind nur dann Wiederholungen, wenn jede aktivierte Spalte gleich ist. Groß- und Kleinschreibung gelten als gleich, und eine Formel zählt als der Wert, den sie liefert.
5. Klicken Sie auf **OK**. Der Dialog teilt Ihnen mit, wie viele Zeilen entfernt wurden und wie viele übrig sind.

Die Zeilen unter einer entfernten Zeile rücken innerhalb der markierten Zellen nach oben, mit ihren Formaten und Formeln. Die Zellen unter der Auswahl bewegen sich nicht. Die leeren Zeilen, die am Ende der Auswahl übrig bleiben, werden gelöscht. Duplikate entfernen ist ein Schritt, den Sie rückgängig machen können.

## Zielwertsuche

Verwenden Sie dies, wenn Sie das gewünschte Ergebnis kennen, nicht aber die Zahl, die es ergibt. Zum Beispiel enthält eine Zelle für eine Rate eine Formel, die einen Zinssatz verwendet, und Sie möchten, dass die Rate 500 ist. Die Zielwertsuche probiert Zahlen in der Zinszelle aus, bis die Rate 500 ist.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Zielwertsuche**.
2. Geben Sie unter **Zelle mit der Formel** die Zelle ein, die die Formel enthält, etwa B3. Sie beginnt als die Zelle, die Sie markiert haben.
3. Geben Sie unter **Gewünschter Wert** das Ergebnis ein, das die Formel liefern soll.
4. Geben Sie unter **Veränderbare Zelle** die Zelle ein, die die Zielwertsuche ändern darf, etwa B1. Sie muss eine Zahl enthalten, keine Formel.
5. Klicken Sie auf **OK**.

Findet die Zielwertsuche eine Zahl, bleibt sie in der veränderbaren Zelle, und die Statusleiste zeigt an, welche Zahl es ist. Findet sie keine, ändert sich nichts, und Sie erhalten eine Meldung. Probieren Sie einen anderen Startwert in der veränderbaren Zelle und prüfen Sie, ob die Formel tatsächlich diese Zelle verwendet. Die Zielwertsuche ist ein Schritt, den Sie rückgängig machen können.

## Szenario-Manager

Ein Szenario ist ein benannter Satz von Werten für einige Zellen. Verwenden Sie Szenarien, um auf einem Blatt „Was wäre wenn“ durchzuspielen. Zum Beispiel behalten Sie ein Szenario „Geringer Umsatz“ und ein Szenario „Hoher Umsatz“ für dieselben Eingabezellen und zeigen das gewünschte an.

1. Wählen Sie auf der Registerkarte **Daten** den Befehl **Szenario-Manager**.
2. Klicken Sie auf **Hinzufügen**. Geben Sie einen Namen ein, etwa Hoher Umsatz. Geben Sie unter **Veränderbare Zellen** die Zellen ein, die das Szenario setzt, etwa B1:B3 oder D5. Ein Szenario kann bis zu 32 Zellen ändern. Die Zellen, die Sie markiert haben, werden für Sie eingetragen. Jede Zelle beginnt mit dem Wert, den sie jetzt enthält. Geben Sie in der Tabelle den Wert ein, den dieses Szenario verwenden soll, und klicken Sie auf **OK**.
3. Wählen Sie in der Liste ein Szenario aus und klicken Sie auf **Anzeigen**, um seine Werte in die Zellen zu setzen. Ein Doppelklick auf einen Namen tut dasselbe. Klicken Sie auf **Bearbeiten**, um Name, Kommentar oder Werte zu ändern. Klicken Sie auf **Löschen**, um es zu entfernen.

Anzeigen, Hinzufügen, Bearbeiten und Löschen sind jeweils ein Schritt, den Sie rückgängig machen können.

**Übersicht** stellt alle Szenarien nebeneinander auf einem neuen Blatt namens Szenarioübersicht dar. Es fragt nach den Ergebniszellen, also den Zellen, die Sie vergleichen möchten, etwa die Summe in B10. Für jedes Szenario zeigt die Übersicht die Werte der veränderbaren Zellen und das Ergebnis der Ergebniszellen, berechnet mit den Werten dieses Szenarios. Eine Spalte namens Aktuelle Werte zeigt, wie die Zellen waren, als Sie die Übersicht erstellt haben. Ihre eigenen Zellen bleiben unverändert.

Die Szenarien werden in der Arbeitsmappe gespeichert. Eine Arbeitsmappe, die mit einem anderen Programm erstellt wurde, behält ihre Szenarien, und Sie sehen sie hier. Eine Kopie, die als ältere .xls-Datei oder als .ods gespeichert wird, enthält sie nicht, und das Speichern weist Sie darauf hin.

## Teilergebnis

Verwenden Sie dies, um unter jeder Gruppe von Zeilen mit demselben Eintrag in einer Spalte eine Zeile mit Summen und am Ende eine Gesamtsumme einzufügen. Die Liste braucht eine Kopfzeile und sollte nach der Spalte sortiert sein, nach der Sie gruppieren, damit gleiche Einträge nebeneinander stehen.

1. Wählen Sie eine Zelle in der Liste aus.
2. Wählen Sie **Teilergebnis** in der Gruppe **Gliederung** auf der Registerkarte **Daten**.
3. Wählen Sie unter **Bei jeder Änderung in** die Spalte, die die Zeilen gruppiert, etwa Region.
4. Wählen Sie unter **Funktion verwenden** aus, was berechnet werden soll: **Summe**, **Anzahl**, **Mittelwert**, **Max**, **Min** und weitere.
5. Aktivieren Sie unter **Teilergebnis hinzufügen zu** jede Spalte, die summiert werden soll.
6. Lassen Sie **Aktuelle Teilergebnisse ersetzen** aktiviert, um bereits vorhandene Teilergebnisse zu ersetzen.
7. **Zusammenfassung unter Daten** entspricht zunächst der Gliederung des Blatts. Lassen Sie es aktiviert, um jede Summenzeile unter ihre Gruppe zu setzen, oder deaktivieren Sie es, um jede Summenzeile über ihre Gruppe zu setzen, mit der Gesamtsumme oben. Klicken Sie auf **OK**.

Jede Summenzeile trägt „Ost Gesamt“ (oder den Eintrag, zu dem sie gehört) und enthält eine TEILERGEBNIS-Formel, die die anderen Summenzeilen ausschließt. Die Zeilen jeder Gruppe sind gruppiert, sodass Sie mit den Schaltflächen 1, 2 und 3 neben den Zeilen nur die Summen anzeigen können. Um die Summen zu entfernen, öffnen Sie **Teilergebnis** erneut und klicken Sie auf **Alle entfernen**. Haben die Summen die andere Seite eines Blatts erreicht, das bereits eigene Gruppen hatte, meldet die Statusleiste, dass diese Gruppen ihre Zusammenfassungszeilen nun auf dieser Seite haben, und **Alle entfernen** stellt die Seite so wieder her, wie sie vor dem ersten Teilergebnis war, sofern Sie sie seitdem nicht selbst geändert haben. Stehen die Summen oben, sitzen die Schaltflächen der Gliederung an den Summenzeilen, sodass eine Plus- oder Minus-Schaltfläche neben „Ost Gesamt“ die Ostzeilen darunter ein- oder ausblendet. Deaktivieren Sie **Aktuelle Teilergebnisse ersetzen**, müssen die neuen Summen auf derselben Seite stehen wie die bereits vorhandenen. Andernfalls wird nichts geändert, und eine Meldung weist darauf hin. Teilergebnisse hinzuzufügen oder zu entfernen ist ein Schritt, den Sie rückgängig machen können.

## Erweiterter Filter

Die Filterschaltflächen wählen Zeilen jeweils nach einer Spalte aus. **Erweitert** (in der Gruppe **Sortieren und Filtern**) lässt Sie die Bedingungen auf dem Blatt schreiben, sodass sie so lang sein können, wie Sie brauchen.

1. Kopieren Sie neben die Liste die Überschriften der Spalten, die Sie prüfen möchten, in eine Zeile leerer Zellen.
2. Schreiben Sie unter jede Überschrift eine Bedingung. Bedingungen in **derselben Zeile** müssen alle zutreffen (das ist „und“). Bedingungen in **verschiedenen Zeilen** sind Alternativen: Eine Zeile der Liste muss nur eine dieser Zeilen erfüllen (das ist „oder“).
3. Wählen Sie **Erweitert**. Prüfen Sie, dass **Listenbereich** die Liste mit ihrer Kopfzeile ist, und geben Sie die Zellen Ihrer Überschriften und Bedingungen unter **Bedingungsbereich** ein.
4. Wählen Sie **Die übrigen Zeilen in der Liste ausblenden** oder **Die Zeilen an einen anderen Ort kopieren** und geben Sie die erste Zelle des Ziels ein, etwa H1. Aktivieren Sie **Zeilen auslassen, die einer früheren Zeile entsprechen**, um nur je eine Zeile zu kopieren.
5. Klicken Sie auf **OK**.

So schreiben Sie eine Bedingung:

- Ein Wort, etwa **An**, wählt Text aus, der damit beginnt. Groß- und Kleinschreibung gelten als gleich. **?** steht für ein beliebiges Zeichen und ***** für eine beliebige Zeichenfolge.
- Eine Zahl wählt Zellen aus, die ihr entsprechen.
- **=Ann** wählt Text aus, der genau Ann lautet. **=** allein wählt leere Zellen aus.
- **>**, **>=**, **<**, **<=** und **<>** (ungleich) vor einer Zahl oder einem Wort vergleichen damit, etwa **>25**. **<>** allein wählt Zellen aus, die nicht leer sind.

Eine Bedingungszeile ohne Inhalt wählt alle Zeilen aus. Um die ausgeblendeten Zeilen wieder anzuzeigen, wählen Sie **Löschen**. Eine Kopie wird sofort erstellt und ändert sich nicht, wenn sich die Liste ändert. Erweiterter Filter ist ein Schritt, den Sie rückgängig machen können.

## Datenüberprüfung

Verwenden Sie dies, um festzulegen, was in bestimmte Zellen eingegeben werden darf, damit eine falsche Eingabe gestoppt oder hinterfragt wird.

1. Markieren Sie die Zellen.
2. Wählen Sie **Datenüberprüfung** in der Gruppe **Datentools** auf der Registerkarte **Daten**.
3. Wählen Sie auf der Registerkarte **Einstellungen** unter **Zulassen**: **Beliebiger Wert** (keine Regel), **Ganze Zahl**, **Dezimalzahl**, **Liste**, **Datum**, **Uhrzeit**, **Textlänge** oder **Benutzerdefiniert**.
4. Für eine Zahl, ein Datum, eine Uhrzeit oder eine Textlänge wählen Sie unter **Daten** (**zwischen**, **nicht zwischen**, **gleich** und so weiter) und geben die Grenzen ein. Eine Grenze kann ein Wert oder eine Formel sein, die mit = beginnt.
5. Für eine **Liste** geben Sie die Auswahlmöglichkeiten mit Kommas dazwischen ein, etwa Rot,Grün,Blau, oder Sie geben nach = einen Zellbereich ein, etwa =$A$1:$A$5. Die Zelle zeigt dann einen Pfeil mit den Auswahlmöglichkeiten. Deaktivieren Sie **Dropdown in Zelle**, um den Pfeil auszublenden.
6. Für **Benutzerdefiniert** geben Sie eine Formel ein, die mit = beginnt. Die Eingabe ist zulässig, wenn die Formel wahr ist.
7. **Leere Zellen ignorieren** lässt eine leere Zelle durch.

Aktivieren Sie auf der Registerkarte **Eingabemeldung** das Kästchen und schreiben Sie einen Titel und eine Meldung. Sie erscheinen neben der Zelle, sobald sie ausgewählt ist.

Auf der Registerkarte **Fehlermeldung** wählen Sie, was mit einer nicht zulässigen Eingabe geschehen soll. **Anhalten** lehnt sie ab. **Warnung** fragt, ob sie beibehalten werden soll. **Information** teilt es der Person mit und behält die Eingabe. Schreiben Sie einen Titel und eine Meldung für die Fehlermeldung.

**Ungültige Daten einkreisen** (neben **Datenüberprüfung**) setzt eine rote Ellipse um jede Zelle des Blatts, deren Wert ihre Regel nicht zulässt, etwa eine Zahl, die schon vor der Regel dastand, oder das Ergebnis einer Formel. Eine Meldung sagt Ihnen, wenn alle Zellen in Ordnung sind. Eine Zelle, die Sie korrigieren, verliert ihre Ellipse. **Überprüfungskreise entfernen** nimmt alle Ellipsen weg.

**Alle löschen** entfernt die Regeln von den markierten Zellen. Die neue Regel ersetzt jede Regel, die die markierten Zellen hatten; andere Zellen behalten ihre Regeln. Das Ändern von Regeln ist ein Schritt, den Sie rückgängig machen können, und die Regeln werden in der Datei gespeichert.

## Konsolidieren

Verwenden Sie dies, um Bereiche, die einander ähnlich sind, etwa dieselbe Tabelle aus mehreren Blättern, zu einer Tabelle zusammenzufassen.

1. Wählen Sie **Konsolidieren** in der Gruppe **Datentools** auf der Registerkarte **Daten**.
2. Wählen Sie unter **Funktion** die Funktion, mit der kombiniert werden soll, etwa **Summe** oder **Mittelwert**.
3. Geben Sie einen Bereich in das Feld ein und klicken Sie auf **Hinzufügen**. Verwenden Sie **Tabelle2!A1:C10** für einen Bereich auf einem anderen Blatt. Fügen Sie jeden Bereich hinzu, den Sie kombinieren möchten. Wählen Sie einen Bereich in der Liste aus und klicken Sie auf **Löschen**, um ihn zu entfernen.
4. Haben die Bereiche Namen in ihrer ersten Zeile, aktivieren Sie **Obere Zeile enthält Spaltennamen**. Haben sie Namen in ihrer ersten Spalte, aktivieren Sie **Linke Spalte enthält Zeilennamen**.
5. Geben Sie unter **Ergebnis ausgeben in** die erste Zelle für das Ergebnis ein.
6. Aktivieren Sie **Verknüpfungen zu den Quelldaten erstellen**, wenn jedes Ergebnis eine Formel sein soll, die seine Quellzellen liest, sodass es sich ändert, wenn diese sich ändern. Ohne dieses Kästchen sind die Ergebnisse einfache Zahlen.
7. Klicken Sie auf **OK**.

Ohne aktivierte Namen wird die Zelle an derselben Stelle in jedem Bereich kombiniert (die erste Zelle jedes Bereichs, dann die zweite und so weiter). Mit aktivierten Namen werden Zellen mit gleichen Namen kombiniert, unabhängig davon, wo sie in jedem Bereich stehen, und ein Name, den nur einige Bereiche haben, erhält trotzdem eine Zeile oder Spalte. Konsolidieren ist ein Schritt, den Sie rückgängig machen können.

## Prognoseblatt

Das Prognoseblatt erstellt ein neues Blatt mit einer Tabelle und einem Diagramm zukünftiger Werte, berechnet aus einer Zeitachse und einer Reihe von Werten. Es ermittelt den Trend und die sich wiederholende Saison der Werte und fügt ein Band hinzu, in dem die künftigen Werte voraussichtlich liegen.

1. Markieren Sie die Zeitachse und die Werte, zwei Spalten nebeneinander (zuerst die Zeitachse). Die Zeitachse besteht aus Datumswerten oder Zahlen mit gleichen Abständen, etwa jeden Monat oder alle 5 Einheiten. Eine erste Zeile mit Namen wird nicht berücksichtigt.
2. Wählen Sie **Prognoseblatt** in der Gruppe **Prognose** auf der Registerkarte **Daten**.
3. Ein Bild zeigt die Prognose und ändert sich, wenn Sie die Optionen ändern. Wählen Sie **Liniendiagramm** oder **Säulendiagramm**.
4. Legen Sie die Optionen fest:
   - **Zeitachsenbereich** und **Wertebereich** sind die Zellen, die gelesen werden. Schreiben Sie hier andere Zellen, falls die Auswahl falsch war.
   - **Prognosebeginn** ist leer, um nach dem letzten Punkt zu beginnen. Ein Datum oder eine Zahl vor dem letzten Punkt beginnt dort und verwendet nur die Punkte davor, sodass Sie die Prognose mit dem vergleichen können, was tatsächlich geschehen ist.
   - **Prognoseende** ist der letzte Zeitpunkt, für den eine Prognose erstellt wird. Es werden höchstens 10.000 Punkte prognostiziert.
   - **Konfidenzintervall** gibt an, wie sicher das Band ist, anfangs 95 %.
   - **Saisonalität** wird aus den Werten ermittelt, oder Sie legen die Länge der Saison in Punkten fest oder wählen **Keine**. Die Werte müssen zwei volle Saisons umfassen.
   - **Fehlende Punkte auffüllen mit** legt fest, was ein Zeitpunkt ohne Wert erhält: eine gerade Linie zwischen seinen Nachbarn oder null.
   - **Doppelte Werte zusammenfassen mit** legt fest, wie Werte zum selben Zeitpunkt kombiniert werden: Mittelwert, Anzahl, AnzahlA, Max, Median, Min oder Summe.
   - **Prognosestatistik einbeziehen** fügt eine Tabelle mit Alpha, Beta, Gamma, MASE, SMAPE, MAE und RMSE hinzu, die angeben, wie gut die Prognose zu den Werten passt.
5. Ist etwas falsch, nennt der Dialog in einfachen Worten, was es ist (Schritte, die nicht gleich sind, zu wenige Punkte, Text in den Werten), und **Erstellen** wartet.
6. Klicken Sie auf **Erstellen**.

Das neue Blatt wird vor dem Blatt eingefügt, auf dem Sie sich befanden. Seine Tabelle hat die Spalten Zeitachse, Werte, Prognose, Untere Konfidenzgrenze und Obere Konfidenzgrenze. Die Prognose und die Grenzen sind Formeln, die `FORECAST.ETS` und `FORECAST.ETS.CONFINT` auf den eigenen Spalten Zeitachse und Werte der Tabelle verwenden, sodass sie neu berechnet werden, wenn Sie einen Wert ändern. Das Diagramm zeigt die Werte, die Prognose und das Band. Das Prognoseblatt ist ein Schritt, den Sie rückgängig machen können.

Die Zahlen können sich um einige Prozent von denen anderer Tabellenkalkulationsprogramme unterscheiden, die ihre eigene Methode zur Anpassung der Glättung verwenden. Eine Prognose, die eine Datei bereits enthält, behält die Werte, die die Datei gespeichert hat, bis sich die Daten ändern, die sie liest.

Eine Datei mit mehr als 1.000.000 Punkten wird nicht verwendet.
