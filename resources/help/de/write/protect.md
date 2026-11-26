# Dokument schützen und Nachverfolgung sperren
<!-- keywords: schützen, Schutz, Bearbeitung einschränken, Schreibschutz, schreibgeschützt, Kennwort, Passwort, Nachverfolgung sperren, Änderungen nachverfolgen, Optionen für Änderungsnachverfolgung, Benutzername, Initialen, nur nachverfolgte Änderungen, nur Kommentare, Schutz aufheben, Formular, Formularfelder, Dokumentschutz, Kennwortschutz -->

Mit dem Schutz legen Sie fest, was andere in einem Dokument tun dürfen. Diese Werkzeuge finden Sie auf der Registerkarte **Überprüfen**.

## Bearbeitung einschränken

1. Klicken Sie in der Gruppe **Schützen** auf **Bearbeitung einschränken**.
2. Wählen Sie, was andere tun dürfen:
   - **Keine Änderungen (schreibgeschützt)**: Der Text kann nicht geändert werden.
   - **Nachverfolgte Änderungen**: Andere können bearbeiten, aber jede Bearbeitung wird nachverfolgt, und „Änderungen nachverfolgen“ kann nicht ausgeschaltet werden.
   - **Kommentare**: Andere können Kommentare hinzufügen, aber den Text nicht ändern.
   - **Formularausfüllung**: Der Text ist schreibgeschützt, aber andere können die darin enthaltenen Formularfelder ausfüllen.
3. Geben Sie zweimal ein Kennwort ein. Sie können das Kennwort auch leer lassen. Dann kann jeder den Schutz aufheben.
4. Klicken Sie auf **Schutz aktivieren**.

Die Titelleiste zeigt dann „Geschützt“ an. Um den Schutz aufzuheben, klicken Sie erneut auf **Bearbeitung einschränken** und dann auf **Schutz beenden**. Geben Sie das Kennwort ein, falls eines vergeben wurde.

Ein Kennwort hält ehrliche Personen davon ab, das Dokument versehentlich zu ändern. Es ist keine starke Sicherheitsmaßnahme. Verwenden Sie den Schutz nicht, um vertrauliche Informationen zu verbergen.

Das Kennwort wird nicht im Klartext gespeichert, sondern als nicht umkehrbarer Hashwert in der Datei abgelegt. Das Kennwort selbst wird nie gespeichert. Wenn Sie es vergessen, kann JeffWrite es nicht wiederherstellen.

## Formular ausfüllen

Ein Dokument kann Formularfelder enthalten: Eingabefelder, Kontrollkästchen und Listen zur Auswahl. JeffWrite füllt sie aus, fügt aber keine neuen hinzu.

- **In ein Feld schreiben.** Klicken Sie in das Feld und geben Sie Text ein. Ein Feld kann nur eine begrenzte Anzahl Zeichen zulassen, Eingaben in Großbuchstaben umwandeln oder eine Zahl oder ein Datum im festgelegten Format erfordern. JeffWrite beachtet das beim Eingeben, Einfügen und bei Eingaben über ein Eingabeverfahren. Eingefügter Text, der zu lang ist, wird abgeschnitten, und die ersten Buchstaben bleiben erhalten. Ein Zeilenumbruch im eingefügten Text wird zu einem Leerzeichen. Ein Zeilenumbruch, der bereits im Feld steht, bleibt erhalten, außer Sie markieren ihn und überschreiben ihn; beim Eingeben wird nie ein Zeilenumbruch hinzugefügt. Eine Zahl oder ein Datum wird beim Verlassen des Feldes in das festgelegte Format gebracht: mit der Tab-Taste, mit der Eingabetaste, durch Klicken an eine andere Stelle, durch Verlassen mit den Pfeiltasten, beim Speichern oder beim Drucken.
- **Kontrollkästchen aktivieren.** Klicken Sie darauf, oder wechseln Sie zum Kästchen und drücken Sie die Leertaste.
- **Aus einer Liste wählen.** Klicken Sie darauf, oder wechseln Sie zur Liste und drücken Sie die Eingabetaste. Wählen Sie dann einen Eintrag. Wenn Sie einen Buchstaben drücken, wird der nächste Eintrag gewählt, der mit diesem Buchstaben beginnt.
- **Zwischen Feldern wechseln.** Wenn das Dokument zum Ausfüllen von Formularen geschützt ist, gelangen Sie mit der Tab-Taste oder der Eingabetaste zum nächsten Feld und mit Umschalt+Tab zurück. In jedem Dokument erzeugt die Eingabetaste innerhalb eines Feldes keinen neuen Absatz. Nur die Felder können geändert werden. Ein Teil des Dokuments, den die Person, die das Formular erstellt hat, vom Schutz ausgenommen hat, kann ebenfalls geändert werden; dort ist die Tab-Taste die übliche Taste, und **Alle ersetzen** ändert nur die Treffer in diesem Teil, meldet, wie viele übersprungen wurden, und weist darauf hin, dass Text in Kopf- und Fußzeilen, Notizen und Textfeldern unverändert bleibt. Was Sie eingeben, einfügen, ablegen oder korrigieren, wird danach geprüft, wohin es gelangt, und nicht danach, wo sich die Einfügemarke befindet. Bilder, Formen und Tabellen können in einem vom Schutz ausgenommenen Teil wie in jedem Dokument hinzugefügt und geändert werden und werden an keiner anderen Stelle verändert.
- **Rückgängig machen.** Klicken Sie auf **Rückgängig**. Die Eingaben, die Sie in einem Feld vornehmen, bilden einen Schritt. **Rückgängig** macht die Schritte seit Beginn des Schutzes rückgängig, also Ihre Eingaben und Ihre Änderungen in einem vom Schutz ausgenommenen Teil, aber nichts davor.

Ein Feld, das im Formular ausgeschaltet ist, oder ein Feld, das das Formular selbst berechnet, kann nicht ausgefüllt werden. In jedem Dokument werden die Anfangs-, End- und andere Markierungen eines Feldes nie Stück für Stück gelöscht; markieren Sie das ganze Feld, um es zu löschen. Ein Feld in einem Dokument, das schreibgeschützt ist oder nur für Kommentare geöffnet ist, kann ebenfalls nicht ausgefüllt werden. Wenn „Änderungen nachverfolgen“ in einem nicht geschützten Dokument eingeschaltet ist, ist das, was Sie in einem Formularfeld eingeben, eine nachverfolgte Änderung, und die maximale Länge und Großschreibung des Feldes werden nicht erzwungen.

## Nachverfolgung sperren

Klicken Sie auf **Optionen für Nachverfolgung**, dann auf **Nachverfolgung sperren**. Ab dann wird jede Änderung nachverfolgt. Ohne Kennwort kann niemand „Änderungen nachverfolgen“ ausschalten oder eine Änderung annehmen oder ablehnen. Das entspricht dem Schutz des Dokuments für nachverfolgte Änderungen.

## Ihren Namen bei Änderungen angeben

Klicken Sie auf **Optionen für Nachverfolgung**, dann auf **Optionen für Änderungsnachverfolgung**. Geben Sie Ihren Namen und Ihre Initialen ein. Sie werden ab dann bei den Änderungen und Kommentaren angegeben, die Sie vornehmen.
