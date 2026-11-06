# Blatt oder Arbeitsmappe schützen
<!-- keywords: schützen, Schutz, Kennwort, sperren, gesperrte Zellen, entsperren, Schutz aufheben, Struktur, Blätter, schreibgeschützt, zulassen, Zellen formatieren, Zeilen einfügen, sortieren, filtern, Hash, Bearbeitungsbereiche zulassen, bearbeitbarer Bereich, Bereichskennwort -->

## Blatt schützen

**Überprüfen > Blatt schützen** verhindert, dass Blätter versehentlich geändert werden. In einem geschützten Blatt lassen sich die Zellen, die **gesperrt** sind, nicht ändern. Alle Zellen sind so lange gesperrt, bis Sie etwas anderes festlegen: Markieren Sie die Zellen, die geändert werden dürfen, öffnen Sie **Zellen formatieren > Schutz**, und deaktivieren Sie **Gesperrt**. (Das ist nur wichtig, sobald das Blatt geschützt ist.)

Das Dialogfeld fragt, was alle auf dem Blatt weiterhin tun dürfen: gesperrte oder nicht gesperrte Zellen auswählen, Zellen formatieren, Spalten oder Zeilen ändern, Zeilen und Spalten einfügen oder löschen, Hyperlinks einfügen, sortieren, Filter verwenden sowie Zeichnungsobjekte und Notizen ändern. Was Sie nicht aktivieren, wird abgelehnt, und es erscheint eine Meldung, die sagt, was abgelehnt wurde.

Ein **Kennwort** ist optional. Wenn Sie eines vergeben, lässt sich der Schutz des Blatts nur mit diesem Kennwort aufheben. JeffCrunch speichert eine verschlüsselte Form des Kennworts (einen Hash mit Salt, der 100.000-mal wiederholt wird) und nie das Kennwort selbst. Deshalb kann das Programm Ihnen das Kennwort später nicht anzeigen: Bewahren Sie es an einem sicheren Ort auf. Kennwörter, die andere Programme erstellt haben, werden in der älteren oder neueren Form auf dieselbe Weise geprüft.

Das Schützen und Aufheben des Schutzes lassen sich rückgängig machen. Um den Schutz aufzuheben, verwenden Sie **Blattschutz aufheben** und geben das Kennwort ein, falls eines vergeben wurde.

Einige Dinge sind auf einem geschützten Blatt unabhängig von den Erlaubnissen nicht möglich: Zellen verbinden sowie Zeilen oder Spalten löschen, die gesperrte Zellen enthalten.

## Bestimmte Zellen änderbar lassen: Bearbeitungsbereiche zulassen

**Überprüfen > Bearbeitungsbereiche zulassen** listet die **Bereiche** auf, die auch bei geschütztem Blatt geändert werden dürfen. Ein Bereich ist ein Zellbereich, etwa A1:C5. Klicken Sie auf **Neu**, um einen hinzuzufügen. Geben Sie ihm einen **Titel**, tragen Sie die Zellen ein, die er umfasst (zwischen mehreren Blöcken steht ein Leerzeichen), und vergeben Sie bei Bedarf ein **Kennwort**. Ein Bereich ohne Kennwort kann von jedem geändert werden. Bei einem Bereich mit Kennwort muss beim ersten Änderungsversuch in einer seiner Zellen das Kennwort eingegeben werden. Danach bleibt der Bereich geöffnet, bis die Arbeitsmappe geschlossen wird.

Mit **Ändern** bearbeiten Sie einen Bereich, mit **Löschen** entfernen Sie ihn. Sie können Bereiche einrichten, bevor Sie das Blatt schützen. Sie werden zusammen mit der Arbeitsmappe gespeichert, und die Kennwörter liegen in derselben verschlüsselten Form vor wie das Kennwort des Blatts. Das Zulassen von Bearbeitungsbereichen lässt sich rückgängig machen. Wenn Sie Zeilen, Spalten oder Zellen einfügen oder löschen, wandern die Bereiche mit ihren Zellen mit, werden kleiner oder größer oder verschwinden, wenn alle ihre Zellen gelöscht werden. Mit **Rückgängig** kehren sie zurück.

## Arbeitsmappe schützen

**Überprüfen > Arbeitsmappe schützen** sperrt die **Struktur**: Es lassen sich keine Blätter hinzufügen, löschen, umbenennen, verschieben, ausblenden oder einblenden. Außerdem kann verlangt werden, dass die Fenster unverändert bleiben. Auch hierfür kann ein Kennwort vergeben werden. **Schutz der Arbeitsmappe aufheben** hebt den Schutz wieder auf.
