# Mit Blättern arbeiten
<!-- keywords: Blatt, Blätter, Registerkarte, Registerkarten, hinzufügen, einfügen, neues Blatt, löschen, entfernen, umbenennen, Name, verschieben, kopieren, duplizieren, neu anordnen, ziehen, ausblenden, einblenden, anzeigen, Registerfarbe, Farbe, mehrere Blätter auswählen, Umschalt+F11, #BEZUG!, 3D-Bezug, Verschieben oder Kopieren -->

Eine Arbeitsmappe besteht aus **Blättern**, und die **Registerkarten** am unteren Rand des Fensters zeigen deren Namen. Klicken Sie auf eine Registerkarte, um dieses Blatt anzuzeigen. Jede der folgenden Änderungen ist ein Schritt, den Sie rückgängig machen können, und sie wird in der Arbeitsmappe gespeichert.

## Ein Blatt hinzufügen

- Klicken Sie auf die Schaltfläche **+** nach der letzten Registerkarte, um ein Blatt am Ende hinzuzufügen.
- Drücken Sie **Umschalt+F11** oder verwenden Sie **Start > Einfügen > Blatt einfügen**, um ein Blatt vor dem aktuell angezeigten Blatt einzufügen.

Das neue Blatt ist leer, erhält den nächsten freien Namen (Sheet4, Sheet5 usw.) und wird sofort geöffnet. Eine Arbeitsmappe kann bis zu 1.000 Blätter haben. Darüber hinaus zeigt JeffCrunch eine Meldung an und fügt keines mehr hinzu, denn eine Datei mit mehr Blättern kann JeffCrunch nicht öffnen.

## Ein Blatt umbenennen

**Doppelklicken** Sie auf die Registerkarte, geben Sie den neuen Namen ein und drücken Sie **Eingabe** (mit **Esc** verwerfen Sie die Eingabe). Alternativ verwenden Sie **Start > Format > Blatt umbenennen**.

Ein Name darf höchstens 31 Zeichen lang sein. Er darf nicht leer sein und keines der Zeichen **: \ / ? * [ ]** enthalten. Er darf nicht mit einem Apostroph beginnen oder enden und nicht mit dem Namen eines anderen Blatts übereinstimmen (Groß- und Kleinschreibung spielt dabei keine Rolle). Der Name „History“ ist für das Programm reserviert. Wird ein Name abgelehnt, teilt JeffCrunch den Grund mit, und das Blatt behält seinen bisherigen Namen.

Jede Formel, die das Blatt beim Namen nennt, übernimmt den neuen Namen. Beispiel: Wenn Sie Budget in Quartal 1 Budget umbenennen, wird **=Budget!B2** zu **='Quartal 1 Budget'!B2**. Auch Namen, die auf das Blatt verweisen, die Regeln bedingter Formatierungen und der Datenüberprüfung sowie Diagramme passen sich an.

## Ein Blatt löschen

Klicken Sie mit der rechten Maustaste auf die Registerkarte und wählen Sie **Blatt löschen**, oder verwenden Sie **Start > Löschen > Blatt löschen**. Enthält das Blatt Daten, fragt JeffCrunch zuerst nach. Formeln auf anderen Blättern, die das gelöschte Blatt lesen, zeigen **#BEZUG!** an. Das bedeutet, dass die gelesenen Daten nicht mehr vorhanden sind. Mit **Rückgängig** holen Sie das Blatt und diese Formeln zurück.

Eine Arbeitsmappe behält immer mindestens ein angezeigtes Blatt.

## Ein Blatt verschieben oder kopieren

- **Ziehen** Sie eine Registerkarte an eine neue Stelle, um das Blatt zu verschieben.
- Oder verwenden Sie **Start > Format > Blatt verschieben oder kopieren**. Wählen Sie das Blatt, vor dem das ausgewählte Blatt stehen soll (oder **(nach dem letzten Blatt)**). Aktivieren Sie **Kopie erstellen, Originale bleiben an ihrem Platz**, um das Blatt zu kopieren statt zu verschieben. Eine Kopie erhält einen Namen wie **Budget (2)** und wird sofort geöffnet.

Formeln lesen weiterhin die Blätter, die sie lesen, unabhängig davon, wo die Blätter liegen. Eine Kopie hat dieselben Zellen, Formeln, Formate und Größen wie das Blatt, von dem sie stammt. Ein Bezug ohne Blattnamen (etwa **A1**) liest die eigene Zelle der Kopie. Ein Bezug, der ein Blatt nennt (etwa **Tabelle1!A1**), verweist weiterhin auf dieses Blatt, auch wenn es das Blatt ist, von dem die Kopie stammt.

Eine Kopie übernimmt außerdem den Blattschutz, den Filter, die Verknüpfungen, das Seitenlayout und die Seitenumbrüche, die Gruppen von Zeilen und Spalten sowie das Aussehen der Datenbalken. Verknüpfungen folgen den Zeilen, die Sie eingefügt oder gelöscht haben: im Original vor der Kopie und in der Kopie danach. Nicht übernommen werden Bilder, Diagramme, Notizen, Pivot-Tabellen, Sparklines und Namen, die nur zum Original gehören. JeffCrunch kann das derzeit noch nicht kopieren und teilt Ihnen beim Kopieren mit, welche Elemente fehlen. Ein Blatt mit Tabellen wird mit Kopien dieser Tabellen kopiert. Jede Tabelle erhält einen eigenen Namen, eine eigene Kennung und einen eigenen Teil in der Datei, und die Formeln der Kopie verweisen auf die eigenen Tabellen. Beim Löschen eines Blatts werden auch seine Tabellen gelöscht. Eine Formel auf einem anderen Blatt, die eine dieser Tabellen nennt, zeigt danach **#BEZUG!** an. Mit **Rückgängig** kehrt sie zurück.

Um ein Blatt in **eine andere Arbeitsmappe** zu kopieren, die geöffnet ist, wählen Sie sie im selben Dialogfeld unter **Zu Arbeitsmappe**. Das Feld listet auf, was die Kopie nicht enthalten wird (derzeit Bilder, Diagramme, bedingte Formatierungen, Tabellen, Notizen und die Druckeinstellungen des Blatts), und weist vor dem Kopieren darauf hin. Formeln, die andere Blätter der ersten Arbeitsmappe lesen, werden zu Verknüpfungen mit deren Datei. Daher muss die erste Arbeitsmappe mindestens einmal als .xlsx-Datei gespeichert worden sein. Die Kopie zeigt die Werte, die die erste Arbeitsmappe aktuell enthält. Diese ändern sich, wenn die Verknüpfungen aktualisiert werden (**Werte aktualisieren** unter **Daten > Verknüpfungen bearbeiten**). Hat die erste Arbeitsmappe nicht gespeicherte Änderungen, lesen die Verknüpfungen beim Aktualisieren ihre gespeicherte Datei. Speichern Sie sie daher, wenn Sie diese Werte behalten möchten. Formeln, die das kopierte Blatt selbst lesen, bleiben auf der Kopie. Zellen, die Namen, Tabellen, ein Matrix- oder ein Überlaufergebnis verwenden oder aus einer dritten Arbeitsmappe lesen, werden als die Werte übernommen, die sie anzeigen. Das Feld nennt deren Anzahl. Die Anzeige des Blatts (fixierte Zeilen und Spalten, Zoom, Gitternetzlinien und Leserichtung) sowie die Standardzeilenhöhe und Standardspaltenbreite werden ebenfalls übernommen. Die Kopie hat dasselbe Aussehen: Formate, Schriftarten und Farben werden der anderen Arbeitsmappe hinzugefügt. Eine Farbe aus dem Design der ersten Arbeitsmappe bleibt als die Farbe erhalten, die sie anzeigt, wenn sich die beiden Designs unterscheiden. Beide Arbeitsmappen müssen Datumswerte ab demselben Jahr zählen (1900 oder 1904). Mit **Rückgängig** in der anderen Arbeitsmappe entfernen Sie die Kopie wieder. Die erste Arbeitsmappe wird dabei nicht verändert. Ein Blatt in eine andere Arbeitsmappe zu verschieben, ist noch nicht möglich.

## Ein Blatt ausblenden und einblenden

Klicken Sie mit der rechten Maustaste auf die Registerkarte und wählen Sie **Blatt ausblenden** (oder verwenden Sie **Start > Format > Blatt ausblenden**). Das Blatt wird nicht angezeigt, bleibt aber in der Arbeitsmappe, und Formeln lesen es weiterhin. Zum Einblenden klicken Sie mit der rechten Maustaste auf eine beliebige Registerkarte und wählen **Blatt einblenden**. Klicken Sie anschließend in der Liste auf den Namen des Blatts. Die Liste ist leer, wenn kein Blatt ausgeblendet ist.

## Eine Registerkarte farbig markieren

Klicken Sie mit der rechten Maustaste auf die Registerkarte und wählen Sie **Registerfarbe**. Wählen Sie dann eine Farbe oder **Ohne Farbe**. Die Farbe erscheint als Linie unter dem Namen. Eine Farbe aus der Designpalette wird als feste Farbe gespeichert, die sie zum jetzigen Zeitpunkt darstellt.

## Mehrere Blätter gleichzeitig

Klicken Sie mit **Strg+Klick** auf Registerkarten, um mehrere auszuwählen. Oder klicken Sie auf eine Registerkarte und dann mit **Umschalt+Klick** auf eine andere, um alle dazwischen liegenden auszuwählen. Die ausgewählten Registerkarten sind abgeschattet. **Löschen**, **Ausblenden**, **Registerfarbe** sowie **Verschieben oder Kopieren** wirken dann auf alle als ein einziger Schritt. Klicken Sie auf eine nicht ausgewählte Registerkarte, um nur diese auszuwählen.

Was Sie eingeben oder formatieren, während mehrere Blätter ausgewählt sind, ändert nur das Blatt, das Sie gerade anzeigen.

## Was nicht möglich ist

- Eine Arbeitsmappe mit einer Formel über einen Bereich von Blättern, etwa **=SUMME(Tabelle1:Tabelle3!A1)**, kann noch keine Blätter hinzugewinnen, löschen, umbenennen oder verschieben. Das würde nämlich ändern, welche Blätter die Formel umfasst.
- Nach dem Speichern kann **Rückgängig** keine Schritte mehr zurücknehmen, die vor dem Speichern ein Blatt hinzugefügt, kopiert, gelöscht, umbenannt oder verschoben haben. Die gespeicherte Datei enthält sie so, wie sie sind. Ausblenden, Einblenden und die Registerfarbe können weiterhin rückgängig gemacht werden.
