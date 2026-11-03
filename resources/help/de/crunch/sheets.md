# Mit Blättern arbeiten
<!-- keywords: Blatt, Blätter, Tabellenblatt, Arbeitsblatt, Register, Registerkarte, hinzufügen, einfügen, neues Blatt, löschen, entfernen, umbenennen, Name, verschieben, kopieren, duplizieren, neu anordnen, ziehen, ausblenden, einblenden, anzeigen, Registerfarbe, Farbe, mehrere Blätter auswählen, Umschalt+F11, #BEZUG!, 3D-Bezug, Verschieben oder kopieren -->

Eine Arbeitsmappe besteht aus **Blättern**, und die **Registerkarten** unten im Fenster zeigen ihre Namen. Klicken Sie auf eine Registerkarte, um das Blatt anzuzeigen. Jede der folgenden Änderungen ist ein Schritt, den Sie rückgängig machen können, und wird in der Arbeitsmappe gespeichert.

## Blatt hinzufügen

- Klicken Sie auf die Schaltfläche **+** hinter der letzten Registerkarte, um am Ende ein Blatt hinzuzufügen.
- Drücken Sie **Umschalt+F11**, oder wählen Sie **Start > Einfügen > Blatt einfügen**, um vor dem gerade angezeigten Blatt ein Blatt hinzuzufügen.

Das neue Blatt ist leer, erhält den nächsten freien Namen (Tabelle4, Tabelle5 usw.) und wird sofort geöffnet. Eine Arbeitsmappe kann bis zu 1.000 Blätter enthalten; darüber hinaus meldet JeffCrunch dies und fügt keines hinzu, da es eine Datei mit mehr Blättern nicht öffnen kann.

## Blatt umbenennen

**Doppelklicken** Sie auf die Registerkarte, geben Sie den neuen Namen ein, und drücken Sie die **Eingabetaste** (mit **Esc** brechen Sie ab). Sie können auch **Start > Format > Blatt umbenennen** verwenden.

Ein Name darf höchstens 31 Zeichen lang sein. Er darf nicht leer sein, keines der Zeichen **: \ / ? * [ ]** enthalten, nicht mit einem Apostroph beginnen oder enden und nicht mit dem Namen eines anderen Blatts übereinstimmen (Groß- und Kleinschreibung gilt dabei nicht als Unterschied). Der Name „History“ ist für das Programm reserviert. Wird der Name abgelehnt, nennt JeffCrunch den Grund, und das Blatt behält seinen Namen.

Jede Formel, die das Blatt nennt, übernimmt den neuen Namen. Beispiel: Aus **=Budget!B2** wird **='Q1 Budget'!B2**, wenn Sie Budget in Q1 Budget umbenennen. Auch Namen, die auf das Blatt verweisen, die Regeln von bedingten Formatierungen und Datenüberprüfungen sowie Diagramme übernehmen den neuen Namen.

## Blatt löschen

Klicken Sie mit der rechten Maustaste auf die Registerkarte, und wählen Sie **Blatt löschen**, oder verwenden Sie **Start > Löschen > Blatt löschen**. Enthält das Blatt etwas, fragt JeffCrunch zuerst nach. Formeln auf anderen Blättern, die das gelöschte Blatt lesen, zeigen **#BEZUG!** an; das bedeutet, dass das, worauf sie verweisen, nicht mehr vorhanden ist. Mit **Rückgängig** stellen Sie das Blatt und diese Formeln wieder her.

Eine Arbeitsmappe behält immer mindestens ein angezeigtes Blatt.

## Blatt verschieben oder kopieren

- **Ziehen** Sie eine Registerkarte an eine neue Stelle, um das Blatt zu verschieben.
- Oder verwenden Sie **Start > Format > Blatt verschieben oder kopieren**. Wählen Sie das Blatt aus, vor dem das gewählte Blatt eingefügt werden soll (oder **(nach dem letzten Blatt)**), und aktivieren Sie **Kopie erstellen und Originale an ihrer Stelle lassen**, um das Blatt zu kopieren statt zu verschieben. Eine Kopie erhält einen Namen wie **Budget (2)** und wird geöffnet.

Formeln lesen weiterhin dieselben Blätter, egal wo sich diese befinden. Eine Kopie hat dieselben Zellen, Formeln, Formate und Größen wie das Ursprungsblatt. Ein Bezug ohne Blattnamen (z. B. **A1**) liest die entsprechende Zelle der Kopie; ein Bezug mit Blattnamen (z. B. **Tabelle1!A1**) verweist weiterhin auf dieses Blatt, auch wenn es das Ursprungsblatt der Kopie ist.

Eine Kopie übernimmt außerdem Blattschutz, Filter, Links, Seiteneinrichtung und Seitenumbrüche, die Gruppierungen von Zeilen und Spalten sowie das Aussehen der Datenbalken; Links folgen den eingefügten oder gelöschten Zeilen, im Original vor dem Kopieren und in der Kopie danach. Nicht übernommen werden Bilder, Diagramme, Notizen, Pivot-Tabellen, Sparklines und Namen, die nur zum Original gehören, da JeffCrunch diese noch nicht kopiert; Sie erfahren in diesem Fall, welche Elemente betroffen sind. Ein Blatt mit Tabellen wird mit Kopien dieser Tabellen kopiert (jede mit eigenem Namen, eigener ID und eigenem Teil, und die Formeln der Kopie verweisen auf deren eigene Tabellen). Beim Löschen eines Blatts werden seine Tabellen mitgelöscht; eine Formel auf einem anderen Blatt, die eine davon nannte, zeigt dann #BEZUG! an, und mit Rückgängig wird sie wiederhergestellt.

## Blatt aus- und einblenden

Klicken Sie mit der rechten Maustaste auf die Registerkarte, und wählen Sie **Blatt ausblenden** (oder **Start > Format > Blatt ausblenden**). Das Blatt wird nicht angezeigt, ist aber weiterhin in der Arbeitsmappe vorhanden, und Formeln lesen es weiterhin. Um es wieder anzuzeigen, klicken Sie mit der rechten Maustaste auf eine beliebige Registerkarte, wählen **Blatt einblenden** und klicken dann in der Liste auf seinen Namen. Die Liste ist leer, wenn kein Blatt ausgeblendet ist.

## Registerkarte einfärben

Klicken Sie mit der rechten Maustaste auf die Registerkarte, wählen Sie **Registerfarbe**, und wählen Sie dann eine Farbe oder **Keine Farbe**. Die Farbe erscheint als Linie unter dem Namen. Eine Farbe aus der Palette des Designs wird als die feste Farbe gespeichert, die sie gerade hat.

## Mehrere Blätter gleichzeitig

Klicken Sie mit gedrückter **Strg**-Taste auf Registerkarten, um mehrere auszuwählen, oder klicken Sie auf eine Registerkarte und mit gedrückter **Umschalttaste** auf eine andere, um alle dazwischenliegenden auszuwählen. Die ausgewählten Registerkarten sind schattiert. **Löschen**, **Ausblenden**, **Registerfarbe** und **Verschieben oder kopieren** wirken dann in einem Schritt auf alle. Klicken Sie auf eine nicht ausgewählte Registerkarte, um nur diese auszuwählen.

Was Sie eingeben oder formatieren, während mehrere Blätter ausgewählt sind, ändert nur das gerade angezeigte Blatt.

## Was nicht möglich ist

- In einer Arbeitsmappe mit einer Formel über einen Bereich von Blättern, z. B. **=SUMME(Tabelle1:Tabelle3!A1)**, können noch keine Blätter hinzugefügt, gelöscht, umbenannt oder verschoben werden, da sich sonst ändern würde, welche Blätter die Formel umfasst.
- Nach dem Speichern kann **Rückgängig** nicht hinter ein Blatt zurückgehen, das Sie vor dem Speichern hinzugefügt, kopiert, gelöscht, umbenannt oder verschoben haben. Die gespeicherte Datei enthält die Blätter im aktuellen Zustand. Ausblenden, Einblenden und die Registerfarbe lassen sich weiterhin rückgängig machen.
